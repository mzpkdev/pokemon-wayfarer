import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


GAME_ROOT = Path(__file__).resolve().parents[3]
RETIRED_HNS_MAP_NAMES = (
    "CinnabarIsland_hns",
    "CinnabarIsland_PokemonCenter_hns",
    "SeafoamIslands_1F_hns",
    "SeafoamIslands_B1F_hns",
    "SeafoamIslands_Gym_hns",
    "SeafoamIslands_SecretCave_hns",
    "Route21_hns",
)


class MapjsonWayfarerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build_dir = tempfile.TemporaryDirectory()
        cls.mapjson = Path(cls.build_dir.name) / "mapjson"
        subprocess.run(
            [
                os.environ.get("CXX", "g++"),
                "-std=c++17",
                str(GAME_ROOT / "tools/mapjson/mapjson.cpp"),
                str(GAME_ROOT / "tools/mapjson/json11.cpp"),
                "-o",
                str(cls.mapjson),
            ],
            check=True,
        )

    @classmethod
    def tearDownClass(cls):
        cls.build_dir.cleanup()

    def make_fixture(self):
        fixture = tempfile.TemporaryDirectory()
        root = Path(fixture.name)
        for directory in (
            "data/maps",
            "data/layouts",
            "include/constants",
            "src/data",
            "tools/mapjson",
        ):
            (root / directory).mkdir(parents=True, exist_ok=True)
        (root / "tools/mapjson/required_map_defines.json").write_text(
            json.dumps({"required_maps": [], "required_layouts": []})
        )
        return fixture, root

    @staticmethod
    def add_map(root, name, map_id, source_version, warps=None, region=None):
        map_dir = root / "data/maps" / name
        map_dir.mkdir()
        data = {
            "id": map_id,
            "name": name,
            "game_version": source_version,
            "warp_events": warps or [],
            "connections": [],
        }
        if region is not None:
            data["region"] = region
        (map_dir / "map.json").write_text(json.dumps(data))
        return map_dir / "map.json"

    def run_groups(self, root, version, map_files, manifest=None):
        command = [
            str(self.mapjson),
            "groups",
            version,
            "data/maps/map_groups.json",
            *(str(path.relative_to(root)) for path in map_files),
            "data/maps",
            "include/constants",
        ]
        if manifest is not None:
            command.extend(
                [
                    "--wayfarer-sevii-manifest",
                    str(manifest.relative_to(root)),
                ]
            )
        return subprocess.run(
            command,
            cwd=root,
            text=True,
            capture_output=True,
        )

    def run_map(self, root, version, map_file, layouts_file, manifest=None):
        command = [
            str(self.mapjson),
            "map",
            version,
            str(map_file.relative_to(root)),
            str(layouts_file.relative_to(root)),
            str(map_file.parent.relative_to(root)),
        ]
        if manifest is not None:
            command.extend(
                [
                    "--wayfarer-sevii-manifest",
                    str(manifest.relative_to(root)),
                ]
            )
        return subprocess.run(command, cwd=root, text=True, capture_output=True)

    def run_layouts(self, root, version):
        return subprocess.run(
            [
                str(self.mapjson), "layouts", version,
                "data/layouts/layouts.json", "data/layouts", "include/constants",
            ],
            cwd=root,
            text=True,
            capture_output=True,
        )

    def test_wayfarer_unlinks_replaced_and_orphaned_hns_maps_and_layouts(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        names = [*RETIRED_HNS_MAP_NAMES, "Route20_hns"]
        source_maps = {
            name: json.loads((GAME_ROOT / "data/maps" / name / "map.json").read_text())
            for name in names
        }
        map_files = [
            self.add_map(root, name, source_maps[name]["id"], "hns")
            for name in names
        ]
        (root / "data/maps/map_groups.json").write_text(json.dumps({
            "group_order": ["gHns"], "gHns": names, "connections_include_order": [],
        }))
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))

        layouts = []
        for name in names:
            layout_dir = root / "data/layouts" / name
            layout_dir.mkdir()
            (layout_dir / "border.bin").write_bytes(b"\0\0")
            (layout_dir / "map.bin").write_bytes(b"\0\0")
            layouts.append({
                "id": source_maps[name]["layout"], "name": f"{name}_Layout",
                "game_version": "hns", "width": 1, "height": 1,
                "border_filepath": f"data/layouts/{name}/border.bin",
                "blockdata_filepath": f"data/layouts/{name}/map.bin",
                "primary_tileset": "gTileset_Primary", "secondary_tileset": "gTileset_Secondary",
            })
        (root / "data/layouts/layouts.json").write_text(json.dumps({
            "layouts_table_label": "gMapLayouts", "layouts": layouts,
        }))

        result = self.run_groups(root, "wayfarer", map_files)
        self.assertEqual(result.returncode, 0, result.stderr)
        groups = (root / "data/maps/groups.inc").read_text()
        headers = (root / "data/maps/headers.inc").read_text()
        events = (root / "data/maps/events.inc").read_text()
        for name in RETIRED_HNS_MAP_NAMES:
            self.assertNotIn(f"\t.4byte {name}\n", groups)
            self.assertNotIn(f"/{name}/header.inc", headers)
            self.assertNotIn(f"/{name}/events.inc", events)
        for name in ("Route20_hns",):
            self.assertIn(f"\t.4byte {name}\n", groups)

        result = self.run_layouts(root, "wayfarer")
        self.assertEqual(result.returncode, 0, result.stderr)
        wayfarer_map_constants = (root / "include/constants/map_groups.h").read_text()
        wayfarer_layout_constants = (root / "include/constants/layouts.h").read_text()
        layout_headers = (root / "data/layouts/layouts.inc").read_text()
        layout_table = (root / "data/layouts/layouts_table.inc").read_text()
        for name in RETIRED_HNS_MAP_NAMES:
            self.assertNotIn(f"{name}_Layout::", layout_headers)
            self.assertNotIn(f"\t.4byte {name}_Layout\n", layout_table)
        for name in ("Route20_hns",):
            self.assertIn(f"{name}_Layout::", layout_headers)

        result = self.run_groups(root, "hns", map_files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_layouts(root, "hns")
        self.assertEqual(result.returncode, 0, result.stderr)
        hns_groups = (root / "data/maps/groups.inc").read_text()
        hns_layouts = (root / "data/layouts/layouts.inc").read_text()
        self.assertEqual((root / "include/constants/map_groups.h").read_text(), wayfarer_map_constants)
        self.assertEqual((root / "include/constants/layouts.h").read_text(), wayfarer_layout_constants)
        for name in RETIRED_HNS_MAP_NAMES:
            self.assertIn(f"\t.4byte {name}\n", hns_groups)
            self.assertIn(f"{name}_Layout::", hns_layouts)

    def test_route20_seafoam_warps_follow_selected_build(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        route_name = "Route20_hns"
        route_source = json.loads((GAME_ROOT / "data/maps" / route_name / "map.json").read_text())
        route_map = self.add_map(root, route_name, route_source["id"], "hns")
        route_map.write_text(json.dumps(route_source))
        self.assertEqual(
            [(warp["dest_map"], warp["dest_warp_id"]) for warp in route_source["warp_events"]],
            [("MAP_SEAFOAM_ISLANDS_1F_HNS", "0"), ("MAP_SEAFOAM_ISLANDS_1F_HNS", "3")],
        )
        old_seafoam = self.add_map(
            root, "SeafoamIslands_1F_hns", "MAP_SEAFOAM_ISLANDS_1F_HNS", "hns"
        )
        coast_seafoam = self.add_map(
            root, "SeafoamIslands_1F_CoastPoc", "MAP_SEAFOAM_ISLANDS_1F_COAST_POC", "hns"
        )
        route19 = self.add_map(root, "Route19_hns", "MAP_ROUTE19_HNS", "hns")
        cinnabar_seam = self.add_map(root, "CinnabarIsland_SeamPoc", "MAP_CINNABAR_SEAM_POC", "hns")
        (root / "data/maps/map_groups.json").write_text(json.dumps({
            "group_order": ["gHns"],
            "gHns": [route_name, "Route19_hns", "CinnabarIsland_SeamPoc", "SeafoamIslands_1F_hns", "SeafoamIslands_1F_CoastPoc"],
            "connections_include_order": [],
        }))
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        source_layouts = json.loads((GAME_ROOT / "data/layouts/layouts.json").read_text())
        route_layout = next(
            layout for layout in source_layouts["layouts"]
            if layout.get("id") == route_source["layout"]
        )
        for key in ("border_filepath", "blockdata_filepath"):
            path = root / route_layout[key]
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"\0\0")
        layouts_file = root / "data/layouts/layouts.json"
        layouts_file.write_text(json.dumps({"layouts": [route_layout]}))

        map_files = [route_map, route19, cinnabar_seam, old_seafoam, coast_seafoam]
        result = self.run_groups(root, "wayfarer", map_files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_map(root, "wayfarer", route_map, layouts_file)
        self.assertEqual(result.returncode, 0, result.stderr)
        wayfarer_events = (route_map.parent / "events.inc").read_text()
        self.assertIn("\twarp_def 60, 8, 0, 3, MAP_SEAFOAM_ISLANDS_1F_COAST_POC", wayfarer_events)
        self.assertIn("\twarp_def 72, 14, 0, 4, MAP_SEAFOAM_ISLANDS_1F_COAST_POC", wayfarer_events)

        result = self.run_groups(root, "hns", map_files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_map(root, "hns", route_map, layouts_file)
        self.assertEqual(result.returncode, 0, result.stderr)
        hns_events = (route_map.parent / "events.inc").read_text()
        self.assertIn("\twarp_def 60, 8, 0, 0, MAP_SEAFOAM_ISLANDS_1F_HNS", hns_events)
        self.assertIn("\twarp_def 72, 14, 0, 3, MAP_SEAFOAM_ISLANDS_1F_HNS", hns_events)

    def test_seafoam_exits_return_to_selected_route20_only_in_wayfarer(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        name = "SeafoamIslands_1F_CoastPoc"
        source = json.loads((GAME_ROOT / "data/maps" / name / "map.json").read_text())
        seafoam = self.add_map(root, name, source["id"], "hns")
        seafoam.write_text(json.dumps(source))
        route20_hns = self.add_map(root, "Route20_hns", "MAP_ROUTE20_HNS", "hns")
        route20_poc = self.add_map(root, "Route20_CoastPoc", "MAP_ROUTE20_COAST_POC", "hns")
        b1f = self.add_map(root, "SeafoamIslands_B1F_CoastPoc", "MAP_SEAFOAM_ISLANDS_B1F_COAST_POC", "hns")
        (root / "data/maps/map_groups.json").write_text(json.dumps({
            "group_order": ["gHns"],
            "gHns": [name, "Route20_hns", "Route20_CoastPoc", "SeafoamIslands_B1F_CoastPoc"],
            "connections_include_order": [],
        }))
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        source_layouts = json.loads((GAME_ROOT / "data/layouts/layouts.json").read_text())
        layout = next(row for row in source_layouts["layouts"] if row.get("id") == source["layout"])
        for key in ("border_filepath", "blockdata_filepath"):
            path = root / layout[key]
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"\0\0")
        layouts_file = root / "data/layouts/layouts.json"
        layouts_file.write_text(json.dumps({"layouts": [layout]}))
        files = [seafoam, route20_hns, route20_poc, b1f]

        result = self.run_groups(root, "wayfarer", files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_map(root, "wayfarer", seafoam, layouts_file)
        self.assertEqual(result.returncode, 0, result.stderr)
        events = (seafoam.parent / "events.inc").read_text()
        self.assertIn("\twarp_def 6, 21, 3, 0, MAP_ROUTE20_HNS", events)
        self.assertIn("\twarp_def 32, 21, 3, 1, MAP_ROUTE20_HNS", events)

        result = self.run_groups(root, "hns", files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_map(root, "hns", seafoam, layouts_file)
        self.assertEqual(result.returncode, 0, result.stderr)
        events = (seafoam.parent / "events.inc").read_text()
        self.assertIn("\twarp_def 6, 21, 15, 0, MAP_ROUTE20_COAST_POC", events)
        self.assertIn("\twarp_def 32, 21, 15, 1, MAP_ROUTE20_COAST_POC", events)

    @staticmethod
    def write_sevii_manifest(root, maps, release_link_enabled=False):
        inventory = []
        for map_record in maps:
            map_record.setdefault("retained_events", {})
            for event_kind in ("object_events", "warp_events", "coord_events", "bg_events"):
                map_record["retained_events"].setdefault(event_kind, [])
                for row in map_record["retained_events"][event_kind]:
                    if not isinstance(row, dict):
                        continue
                    content_id = row.setdefault(
                        "content_id",
                        f"exploration.{map_record['source_map'].lower()}.{event_kind}.{row['index']}",
                    )
                    row.setdefault("owner", "exploration")
                    row.setdefault("reason", "Fixture preserves a reviewed exploration event.")
                    inventory.append(content_id)
            map_record.setdefault("retained_map_scripts", [])
            map_record.setdefault("encounter_methods", [])
        path = root / "src/data/wayfarer_sevii_maps.json"
        path.write_text(
            json.dumps(
                {
                    "schema_version": 2,
                    "release_link_enabled": release_link_enabled,
                    "baseline": {
                        "projection_sha256": "0" * 64,
                        "wild_encounters_sha256": "0" * 64,
                        "event_island_baseline_sha256": "0" * 64,
                    },
                    "content_domains": {
                        "exploration": {"owner": "exploration", "enabled": True, "inventory": inventory},
                        "ordinary_trainers": {"owner": "ordinary_trainer", "enabled": False, "inventory": []},
                        "story": {"owner": "story", "enabled": False, "inventory": []},
                        "trainer_tower": {"owner": "trainer_tower", "enabled": False, "inventory": []},
                    },
                    "maps": maps,
                    "exclusions": [],
                }
            )
        )
        return path

    def test_wayfarer_selects_hns_and_emerald_without_mutating_heal_data(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(
            root, "HnsMap", "MAP_HNS", "hns", region="REGION_JOHTO"
        )
        emerald = self.add_map(root, "EmeraldMap", "MAP_EMERALD", "emerald")
        frlg = self.add_map(root, "FrlgMap", "MAP_FRLG", "frlg")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gEmerald", "gFrlg"],
                    "gHns": ["HnsMap"],
                    "gEmerald": ["EmeraldMap"],
                    "gFrlg": ["FrlgMap"],
                    "connections_include_order": [],
                }
            )
        )
        heal_data = json.dumps(
            {
                "heal_locations": [
                    {"id": "HEAL_HNS", "source": "HNS", "map": "MAP_HNS"},
                    {
                        "id": "HEAL_EMERALD",
                        "source": "EMERALD",
                        "map": "MAP_EMERALD",
                        "respawn_map": "MAP_EMERALD",
                        "respawn_npc": "LOCALID_NURSE",
                    },
                ]
            },
            indent=2,
        )
        heal_path = root / "src/data/heal_locations.json"
        heal_path.write_text(heal_data)

        result = self.run_groups(root, "wayfarer", [hns, emerald, frlg])

        self.assertEqual(result.returncode, 0, result.stderr)
        groups = (root / "data/maps/groups.inc").read_text()
        self.assertIn("\t.4byte HnsMap", groups)
        self.assertIn("\t.4byte EmeraldMap", groups)
        self.assertIn("gFrlg::\n\t.4byte NULL", groups)
        headers = (root / "data/maps/headers.inc").read_text()
        self.assertIn(
            '\t.include "data/wayfarer_hoenn_source_constants.inc"\n'
            '\t.include "data/maps/EmeraldMap/header.inc"',
            headers,
        )
        self.assertTrue(headers.rstrip().endswith(
            '\t.include "data/wayfarer_engine_source_constants.inc"'
        ))
        self.assertEqual(heal_path.read_text(), heal_data)
        map_sources = (root / "src/data/wayfarer_map_sources.h").read_text()
        self.assertIn("sWayfarerMapSourceOffsets[] = {0, 1, 2, 3, }", map_sources)
        self.assertIn("sWayfarerHoennMapSourceBits[] = {2, }", map_sources)
        self.assertIn(
            "sWayfarerMapRegionNibbles[] = "
            "{(REGION_JOHTO | (REGION_NONE << 4)), "
            "(REGION_NONE | (REGION_NONE << 4)), }",
            map_sources,
        )

    def test_wayfarer_rejects_unavailable_warp_destination(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(
            root,
            "HnsMap",
            "MAP_HNS",
            "hns",
            [{"dest_map": "MAP_FRLG"}],
        )
        frlg = self.add_map(root, "FrlgMap", "MAP_FRLG", "frlg")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gFrlg"],
                    "gHns": ["HnsMap"],
                    "gFrlg": ["FrlgMap"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps({"heal_locations": []})
        )

        result = self.run_groups(root, "wayfarer", [hns, frlg])

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("warp references unavailable map MAP_FRLG", result.stderr)

    def test_wayfarer_sevii_manifest_is_allowlist_based_and_release_gated(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(root, "HnsMap", "MAP_HNS", "hns")
        frlg = self.add_map(root, "OneIsland_Harbor", "MAP_SEVII", "frlg")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gFrlg"],
                    "gHns": ["HnsMap"],
                    "gFrlg": ["OneIsland_Harbor"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps({"heal_locations": []})
        )
        record = {
            "source_map": "OneIsland_Harbor",
            "map_id": "MAP_SEVII",
            "layout": "LAYOUT_SEVII",
        }
        manifest = self.write_sevii_manifest(root, [record])

        result = self.run_groups(root, "wayfarer", [hns, frlg], manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("gFrlg::\n\t.4byte NULL", (root / "data/maps/groups.inc").read_text())

        manifest = self.write_sevii_manifest(root, [record], release_link_enabled=True)
        result = self.run_groups(root, "wayfarer", [hns, frlg], manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("gFrlg::\n\t.4byte OneIsland_Harbor", (root / "data/maps/groups.inc").read_text())

        record["enabled"] = False
        manifest = self.write_sevii_manifest(root, [record], release_link_enabled=True)
        result = self.run_groups(root, "wayfarer", [hns, frlg], manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("gFrlg::\n\t.4byte NULL", (root / "data/maps/groups.inc").read_text())

    def test_wayfarer_sevii_events_strip_story_and_link_room_warps(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        map_dir = root / "data/maps/OneIsland_PokemonCenter_2F_Frlg"
        map_dir.mkdir()
        source = {
            "id": "MAP_SEVII_CENTER_2F",
            "name": "OneIsland_PokemonCenter_2F_Frlg",
            "game_version": "frlg",
            "layout": "LAYOUT_SEVII_CENTER_2F",
            "music": "MUS_NONE",
            "region_map_section": "MAPSEC_NONE",
            "requires_flash": False,
            "weather": "WEATHER_NONE",
            "map_type": "MAP_TYPE_INDOOR",
            "allow_cycling": False,
            "allow_escaping": False,
            "allow_running": False,
            "show_map_name": False,
            "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
            "object_events": [
                {"type": "object", "graphics_id": "OBJ_EVENT_GFX_TRAINER", "x": 1, "y": 1,
                 "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                 "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NORMAL",
                 "trainer_sight_or_berry_tree_id": "1", "script": "SevenIsland_SevaultCanyon_House_EventScript_ItemLuckyPunch", "flag": "0"},
                {"type": "object", "graphics_id": "OBJ_EVENT_GFX_NURSE", "x": 2, "y": 1,
                 "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                 "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NONE",
                 "trainer_sight_or_berry_tree_id": "0", "script": "SixIsland_WaterPath_House2_EventScript_Man", "flag": "0"},
            ],
            "warp_events": [
                {"x": 1, "y": 5, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_SEVII_1F"},
                {"x": 5, "y": 1, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_UNION_ROOM_FRLG"},
                {"x": 9, "y": 1, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_TRADE_CENTER_FRLG"},
            ],
            "coord_events": [{"type": "trigger", "x": 1, "y": 1, "elevation": 0,
                              "var": "VAR_TEMP_0", "var_value": "0", "script": "SevenIsland_SevaultCanyon_House_EventScript_ChanseyDanceMan"}],
            "bg_events": [{"type": "sign", "x": 1, "y": 2, "elevation": 0,
                           "player_facing_dir": "BG_EVENT_PLAYER_FACING_ANY", "script": "SevenIsland_SevaultCanyon_House_EventScript_Chansey"}],
            "connections": [{"direction": "up", "offset": 0, "map": "MAP_SEVII_1F"}],
        }
        map_file = map_dir / "map.json"
        map_file.write_text(json.dumps(source))
        (root / "include/constants/map_groups.h").write_text(
            "enum { MAP_SEVII_CENTER_2F = (0 | (0 << 8)), MAP_SEVII_1F = (1 | (0 << 8)), };\n"
        )
        layout_file = root / "data/layouts/layouts.json"
        (root / "data/layouts/center.border.bin").touch()
        (root / "data/layouts/center.map.bin").touch()
        layout_file.write_text(json.dumps({"layouts": [{
            "id": "LAYOUT_SEVII_CENTER_2F", "name": "gMapLayout_SeviiCenter2F",
            "game_version": "frlg", "layout_version": "frlg", "width": 1, "height": 1,
            "border_filepath": "data/layouts/center.border.bin", "blockdata_filepath": "data/layouts/center.map.bin",
            "primary_tileset": "gTileset_General", "secondary_tileset": "gTileset_Petalburg",
            "border_width": 2, "border_height": 2,
        }]}))
        manifest = self.write_sevii_manifest(root, [
            {"source_map": "OneIsland_PokemonCenter_2F_Frlg", "map_id": "MAP_SEVII_CENTER_2F",
             "layout": "LAYOUT_SEVII_CENTER_2F", "enabled": True,
             "retained_events": {"object_events": [{
                 "index": 1,
                 "source": source["object_events"][1],
                 "wayfarer_script": "WayfarerSevii_Nurse",
                 "overrides": {"script": "WayfarerSevii_Nurse", "flag": "0"},
             }], "coord_events": [], "bg_events": []}},
            {"source_map": "OneIsland_PokemonCenter_1F_Frlg", "map_id": "MAP_SEVII_1F",
             "layout": "LAYOUT_SEVII_CENTER_1F", "enabled": True},
        ], release_link_enabled=True)

        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        events = (map_dir / "events.inc").read_text()
        self.assertIn("ObjectEvents", events)
        self.assertIn("WayfarerSevii_Nurse", events)
        self.assertRegex(events, r"object_event [^\n]*, 0\n")
        self.assertNotIn("CoordEvents", events)
        self.assertNotIn("BGEvents", events)
        self.assertIn("MAP_SEVII_1F", events)
        self.assertNotIn("MAP_UNION_ROOM_FRLG", events)
        self.assertNotIn("MAP_TRADE_CENTER_FRLG", events)
        self.assertNotIn("SevenIsland_SevaultCanyon_House_EventScript_ItemLuckyPunch", events)
        self.assertNotIn("SixIsland_WaterPath_House2_EventScript_Man", events)
        self.assertNotIn("SevenIsland_SevaultCanyon_House_EventScript_ChanseyDanceMan", events)
        self.assertNotIn("SevenIsland_SevaultCanyon_House_EventScript_Chansey", events)
        self.assertNotIn("SevenIsland_SevaultCanyon_House_EventScript_ItemLuckyPunch", events)
        connections = (map_dir / "connections.inc").read_text()
        self.assertIn("MAP_SEVII_1F", connections)

    def test_wayfarer_sanitizes_only_registered_sevii_map_events(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)

        def map_source(name, map_id, game_version, script, flag):
            return {
                "id": map_id,
                "name": name,
                "game_version": game_version,
                "layout": f"LAYOUT_{map_id[4:]}",
                "music": "MUS_NONE",
                "region_map_section": "MAPSEC_NONE",
                "requires_flash": False,
                "weather": "WEATHER_NONE",
                "map_type": "MAP_TYPE_TOWN",
                "allow_cycling": True,
                "allow_escaping": False,
                "allow_running": True,
                "show_map_name": True,
                "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
                "object_events": [{
                    "type": "object", "graphics_id": "OBJ_EVENT_GFX_NURSE", "x": 1, "y": 1,
                    "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                    "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NONE",
                    "trainer_sight_or_berry_tree_id": "0", "script": script, "flag": "0",
                }],
                "warp_events": [],
                "coord_events": [],
                "bg_events": [{
                    "type": "hidden_item", "x": 2, "y": 2, "elevation": 0,
                    "item": "ITEM_POTION", "flag": flag,
                }],
                "connections": [],
            }

        layouts = []
        map_files = {}
        for name, map_id, source, script, flag in (
            ("OneIsland_Frlg", "MAP_SEVII", "frlg", "FrlgStoryEvent", "FLAG_SEVII_ITEM"),
            ("SafariZone_Southeast", "MAP_SAFARI", "emerald", "SafariLegacyEvent", "FLAG_SAFARI_ITEM"),
            ("NavelRock_Top", "MAP_NAVEL", "emerald", "NavelLegacyEvent", "FLAG_NAVEL_ITEM"),
        ):
            map_dir = root / "data/maps" / name
            map_dir.mkdir()
            source_data = map_source(name, map_id, source, script, flag)
            map_file = map_dir / "map.json"
            map_file.write_text(json.dumps(source_data))
            map_files[name] = map_file
            border = root / f"data/layouts/{name}.border.bin"
            blockdata = root / f"data/layouts/{name}.map.bin"
            border.touch()
            blockdata.touch()
            layouts.append({
                "id": source_data["layout"], "name": f"gMapLayout_{name}",
                "game_version": source, "layout_version": source, "width": 1, "height": 1,
                "border_filepath": str(border.relative_to(root)),
                "blockdata_filepath": str(blockdata.relative_to(root)),
                "primary_tileset": "gTileset_General", "secondary_tileset": "gTileset_Petalburg",
                "border_width": 2, "border_height": 2,
            })
        layouts_file = root / "data/layouts/layouts.json"
        layouts_file.write_text(json.dumps({"layouts": layouts}))
        (root / "include/constants/map_groups.h").write_text(
            "enum { MAP_SEVII = (0 | (0 << 8)), MAP_SAFARI = (1 | (0 << 8)), "
            "MAP_NAVEL = (2 | (0 << 8)), };\n"
        )
        manifest = self.write_sevii_manifest(root, [{
            "source_map": "OneIsland_Frlg", "map_id": "MAP_SEVII", "layout": "LAYOUT_SEVII",
            "enabled": True, "retained_events": {"object_events": [], "coord_events": [], "bg_events": []},
        }], release_link_enabled=True)

        result = self.run_map(root, "wayfarer", map_files["OneIsland_Frlg"], layouts_file, manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        sevii_events = map_files["OneIsland_Frlg"].parent.joinpath("events.inc").read_text()
        self.assertNotIn("FrlgStoryEvent", sevii_events)
        self.assertNotIn("FLAG_SEVII_ITEM", sevii_events)

        for legacy_name, legacy_script, legacy_flag in (
            ("SafariZone_Southeast", "SafariLegacyEvent", "FLAG_SAFARI_ITEM"),
            ("NavelRock_Top", "NavelLegacyEvent", "FLAG_NAVEL_ITEM"),
        ):
            result = self.run_map(root, "wayfarer", map_files[legacy_name], layouts_file, manifest)
            self.assertEqual(result.returncode, 0, result.stderr)
            wayfarer_events = map_files[legacy_name].parent.joinpath("events.inc").read_text()
            result = self.run_map(root, "emerald", map_files[legacy_name], layouts_file)
            self.assertEqual(result.returncode, 0, result.stderr)
            emerald_events = map_files[legacy_name].parent.joinpath("events.inc").read_text()
            self.assertEqual(wayfarer_events, emerald_events)
            self.assertIn(legacy_script, wayfarer_events)
            self.assertIn(legacy_flag, wayfarer_events)
            self.assertIn("bg_hidden_item_event ", wayfarer_events)
            self.assertNotIn("bg_hidden_item_event_hoenn ", wayfarer_events)

    def test_wayfarer_sevii_rejects_retained_trainer_identity(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        map_dir = root / "data/maps/OneIsland_Frlg"
        map_dir.mkdir()
        trainer = {"type": "object", "graphics_id": "OBJ_EVENT_GFX_TRAINER", "x": 1, "y": 1,
                   "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                   "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NORMAL",
                   "trainer_sight_or_berry_tree_id": "1", "script": "Frlg_Trainer", "flag": "0"}
        source = {"id": "MAP_SEVII", "name": "OneIsland_Frlg", "game_version": "frlg",
                  "layout": "LAYOUT_SEVII", "music": "MUS_NONE", "region_map_section": "MAPSEC_NONE",
                  "requires_flash": False, "weather": "WEATHER_NONE", "map_type": "MAP_TYPE_TOWN",
                  "allow_cycling": True, "allow_escaping": False, "allow_running": True,
                  "show_map_name": True, "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
                  "object_events": [trainer], "warp_events": [], "coord_events": [], "bg_events": [],
                  "connections": []}
        map_file = map_dir / "map.json"
        map_file.write_text(json.dumps(source))
        (root / "data/layouts/sevii.border.bin").touch()
        (root / "data/layouts/sevii.map.bin").touch()
        layout_file = root / "data/layouts/layouts.json"
        layout_file.write_text(json.dumps({"layouts": [{
            "id": "LAYOUT_SEVII", "name": "gMapLayout_Sevii", "game_version": "frlg", "layout_version": "frlg",
            "width": 1, "height": 1, "border_filepath": "data/layouts/sevii.border.bin",
            "blockdata_filepath": "data/layouts/sevii.map.bin", "primary_tileset": "gTileset_General",
            "secondary_tileset": "gTileset_Petalburg", "border_width": 2, "border_height": 2,
        }]}))
        manifest = self.write_sevii_manifest(root, [{
            "source_map": "OneIsland_Frlg", "map_id": "MAP_SEVII", "layout": "LAYOUT_SEVII", "enabled": True,
            "retained_events": {"object_events": [{"index": 0, "source": trainer,
                                                       "wayfarer_script": "WayfarerSevii_Test"}]},
        }], release_link_enabled=True)

        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("is a Trainer and cannot be retained", result.stderr)

    def test_wayfarer_sevii_accepts_tower_transient_event_state_only(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        map_dir = root / "data/maps/TrainerTower_1F_Frlg"
        map_dir.mkdir()
        tower_actor = {
            "type": "object", "graphics_id": "OBJ_EVENT_GFX_TRAINER_TOWER_DUDE",
            "x": 1, "y": 1, "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN",
            "movement_range_x": 0, "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NONE",
            "trainer_sight_or_berry_tree_id": "0", "script": "TowerActor", "flag": "FLAG_TEMP_6",
        }
        source = {
            "id": "MAP_TOWER", "name": "TrainerTower_1F_Frlg", "game_version": "frlg",
            "layout": "LAYOUT_TOWER", "music": "MUS_NONE", "region_map_section": "MAPSEC_NONE",
            "requires_flash": False, "weather": "WEATHER_NONE", "map_type": "MAP_TYPE_INDOOR",
            "allow_cycling": False, "allow_escaping": False, "allow_running": True,
            "show_map_name": False, "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
            "object_events": [tower_actor], "warp_events": [], "coord_events": [], "bg_events": [],
            "connections": [],
        }
        map_file = map_dir / "map.json"
        map_file.write_text(json.dumps(source))
        (root / "data/layouts/tower.border.bin").touch()
        (root / "data/layouts/tower.map.bin").touch()
        layout_file = root / "data/layouts/layouts.json"
        layout_file.write_text(json.dumps({"layouts": [{
            "id": "LAYOUT_TOWER", "name": "gMapLayout_Tower", "game_version": "frlg",
            "layout_version": "frlg", "width": 1, "height": 1,
            "border_filepath": "data/layouts/tower.border.bin",
            "blockdata_filepath": "data/layouts/tower.map.bin",
            "primary_tileset": "gTileset_General", "secondary_tileset": "gTileset_Petalburg",
            "border_width": 2, "border_height": 2,
        }]}))
        (root / "include/constants/map_groups.h").write_text(
            "enum { MAP_TOWER = (0 | (0 << 8)), };\n"
        )
        content_id = "trainer-tower.test.actor"
        row = {
            "index": 0, "source": tower_actor, "owner": "trainer_tower", "content_id": content_id,
            "reason": "Fixture preserves a transient Tower actor.",
            "wayfarer_script": "WayfarerSevii_TowerActor",
        }
        manifest = self.write_sevii_manifest(root, [{
            "source_map": "TrainerTower_1F_Frlg", "map_id": "MAP_TOWER", "layout": "LAYOUT_TOWER",
            "enabled": True, "retained_events": {"object_events": [row]},
        }], release_link_enabled=True)
        manifest_data = json.loads(manifest.read_text())
        manifest_data["content_domains"]["exploration"]["inventory"] = []
        manifest_data["content_domains"]["trainer_tower"] = {
            "owner": "trainer_tower", "enabled": True, "inventory": [content_id],
        }
        manifest.write_text(json.dumps(manifest_data))

        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertEqual(result.returncode, 0, result.stderr)

        manifest_data["content_domains"]["trainer_tower"] = {
            "owner": "trainer_tower", "enabled": False, "inventory": [],
        }
        manifest_data["content_domains"]["story"] = {
            "owner": "story", "enabled": True, "inventory": [content_id],
        }
        manifest_data["maps"][0]["retained_events"]["object_events"][0]["owner"] = "story"
        manifest.write_text(json.dumps(manifest_data))
        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("retains raw FRLG persistent state", result.stderr)

    def test_wayfarer_sevii_manifest_rejects_event_island_sources(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(root, "HnsMap", "MAP_HNS", "hns")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns"],
                    "gHns": ["HnsMap"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps({"heal_locations": []})
        )
        manifest = self.write_sevii_manifest(
            root,
            [
                {
                    "source_map": "BirthIsland_Harbor_Frlg",
                    "map_id": "MAP_BIRTH",
                    "layout": "LAYOUT_BIRTH",
                }
            ],
        )

        result = self.run_groups(root, "wayfarer", [hns], manifest)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("cannot select event-island map", result.stderr)

    def test_wayfarer_validates_selected_frlg_heal_locations(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(root, "HnsMap", "MAP_HNS", "hns")
        frlg = self.add_map(root, "OneIsland_PokemonCenter_1F", "MAP_SEVII", "frlg")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gFrlg"],
                    "gHns": ["HnsMap"],
                    "gFrlg": ["OneIsland_PokemonCenter_1F"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps(
                {
                    "heal_locations": [
                        {
                            "id": "HEAL_SEVII",
                            "source": "FRLG",
                            "map": "MAP_SEVII",
                            "respawn_map": "MAP_MISSING",
                            "respawn_npc": "LOCALID_NURSE",
                        }
                    ]
                }
            )
        )
        manifest = self.write_sevii_manifest(
            root,
            [
                {
                    "source_map": "OneIsland_PokemonCenter_1F",
                    "map_id": "MAP_SEVII",
                    "layout": "LAYOUT_SEVII",
                    "enabled": True,
                }
            ],
            release_link_enabled=True,
        )

        result = self.run_groups(root, "wayfarer", [hns, frlg], manifest)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("references unavailable respawn map MAP_MISSING", result.stderr)

    def test_wayfarer_layouts_include_both_sources_and_emit_map_flags(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        layouts = []
        for source, suffix in (("emerald", "Emerald"), ("hns", "Hns"), ("frlg", "Frlg")):
            border = root / f"data/layouts/{suffix}.border.bin"
            blockdata = root / f"data/layouts/{suffix}.map.bin"
            border.touch()
            blockdata.touch()
            layouts.append(
                {
                    "id": f"LAYOUT_{suffix.upper()}",
                    "name": f"gMapLayout_{suffix}",
                    "game_version": source,
                    "layout_version": source,
                    "width": 1,
                    "height": 1,
                    "border_filepath": str(border.relative_to(root)),
                    "blockdata_filepath": str(blockdata.relative_to(root)),
                    "primary_tileset": "gTileset_General",
                    "secondary_tileset": "gTileset_Petalburg",
                    "border_width": 2,
                    "border_height": 2,
                }
            )
        layouts_data = {
            "layouts_table_label": "gMapLayouts",
            "layouts": layouts,
        }
        layouts_path = root / "data/layouts/layouts.json"
        layouts_path.write_text(json.dumps(layouts_data))

        result = subprocess.run(
            [
                str(self.mapjson),
                "layouts",
                "wayfarer",
                "data/layouts/layouts.json",
                "data/layouts",
                "include/constants",
            ],
            cwd=root,
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        headers = (root / "data/layouts/layouts.inc").read_text()
        table = (root / "data/layouts/layouts_table.inc").read_text()
        self.assertIn("gMapLayout_Emerald::", headers)
        self.assertIn("gMapLayout_Hns::", headers)
        self.assertNotIn("gMapLayout_Frlg::", headers)
        self.assertIn("\t.4byte gMapLayout_Emerald", table)
        self.assertIn("\t.4byte gMapLayout_Hns", table)
        self.assertTrue(table.rstrip().endswith("\t.4byte NULL"))

        map_dir = root / "data/maps/HnsMap"
        map_dir.mkdir()
        map_data = {
            "id": "MAP_HNS",
            "name": "HnsMap",
            "game_version": "hns",
            "layout": "LAYOUT_HNS",
            "music": "MUS_NONE",
            "region_map_section": "MAPSEC_NONE",
            "requires_flash": False,
            "weather": "WEATHER_NONE",
            "map_type": "MAP_TYPE_TOWN",
            "allow_cycling": True,
            "allow_escaping": False,
            "allow_running": True,
            "show_map_name": True,
            "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
            "object_events": [],
            "warp_events": [],
            "coord_events": [],
            "bg_events": [],
            "connections": [],
        }
        (map_dir / "map.json").write_text(json.dumps(map_data))
        (root / "include/constants/map_groups.h").write_text(
            "enum { MAP_HNS = (0 | (0 << 8)), };\n"
        )
        result = subprocess.run(
            [
                str(self.mapjson),
                "map",
                "wayfarer",
                "data/maps/HnsMap/map.json",
                "data/layouts/layouts.json",
                "data/maps/HnsMap",
            ],
            cwd=root,
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("\tmap_header_flags ", (map_dir / "header.inc").read_text())

        first_mtimes = {
            path.name: path.stat().st_mtime_ns
            for path in map_dir.glob("*.inc")
        }
        result = subprocess.run(
            [
                str(self.mapjson),
                "map",
                "wayfarer",
                "data/maps/HnsMap/map.json",
                "data/layouts/layouts.json",
                "data/maps/HnsMap",
            ],
            cwd=root,
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(
            first_mtimes,
            {path.name: path.stat().st_mtime_ns for path in map_dir.glob("*.inc")},
        )

    def test_catalog_rejects_signed_byte_group_and_map_overflow(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        only_map = self.add_map(root, "OnlyMap", "MAP_ONLY", "emerald")
        group_names = [f"g{i}" for i in range(129)]
        groups = {
            "group_order": group_names,
            "connections_include_order": [],
            **{name: [] for name in group_names},
        }
        (root / "data/maps/map_groups.json").write_text(json.dumps(groups))

        result = self.run_groups(root, "emerald", [only_map])

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Map group 128 exceeds the signed-byte warp limit", result.stderr)

        map_names = []
        map_files = []
        for i in range(129):
            name = f"Map{i}"
            map_names.append(name)
            map_files.append(self.add_map(root, name, f"MAP_{i}", "emerald"))
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gMaps"],
                    "gMaps": map_names,
                    "connections_include_order": [],
                }
            )
        )

        result = self.run_groups(root, "emerald", map_files)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Map 128 in group gMaps exceeds the signed-byte warp limit", result.stderr)

    def test_repository_wayfarer_union_resolves_and_preserves_hns_positions(self):
        maps_root = GAME_ROOT / "data/maps"
        groups = json.loads((maps_root / "map_groups.json").read_text())
        expected_hns_groups = [
            "gMapGroup_TownsAndRoutes_Hns",
            "gMapGroup_IndoorNewBark_Hns",
            "gMapGroup_IndoorCherrygrove_Hns",
            "gMapGroup_IndoorViolet_Hns",
            "gMapGroup_IndoorAzalea_Hns",
            "gMapGroup_IndoorGoldenrod_Hns",
            "gMapGroup_IndoorEcruteak_Hns",
            "gMapGroup_IndoorOlivine_Hns",
            "gMapGroup_IndoorCianwood_Hns",
            "gMapGroup_IndoorMahogany_Hns",
            "gMapGroup_IndoorBlackthorn_Hns",
            "gMapGroup_IndoorPallet_Hns",
            "gMapGroup_IndoorViridian_Hns",
            "gMapGroup_IndoorPewter_Hns",
            "gMapGroup_IndoorCerulean_Hns",
            "gMapGroup_IndoorVermilion_Hns",
            "gMapGroup_IndoorLavender_Hns",
            "gMapGroup_IndoorCeladon_Hns",
            "gMapGroup_IndoorSaffron_Hns",
            "gMapGroup_IndoorFuchsia_Hns",
            "gMapGroup_IndoorCinnabar_Hns",
            "gMapGroup_IndoorIndigo_Hns",
            "gMapGroup_IndoorJohtoRoutes_Hns",
            "gMapGroup_IndoorKantoRoutes_Hns",
            "gMapGroup_Dungeons_Hns",
            "gMapGrouop_OutdoorAlola_Hns",
            "gMapGroup_IndoorAlola_Hns",
            "gMapGroup_IndoorDynamic_Hns",
            "gMapGroup_Sinjoh_Hns",
            "gMapGroup_IndoorSinjoh_Hns",
            "gMapGroup_SpecialArea_Hns",
        ]
        self.assertEqual(groups["group_order"][:31], expected_hns_groups)
        self.assertEqual(groups["group_order"][31], "gMapGroup_TownsAndRoutes")
        data = {
            path.parent.name: json.loads(path.read_text())
            for path in maps_root.glob("*/map.json")
        }
        included = []
        for group_num, group_name in enumerate(groups["group_order"]):
            self.assertLessEqual(group_num, 127)
            for map_num, map_name in enumerate(groups[group_name]):
                self.assertLessEqual(map_num, 127)
                map_data = data[map_name]
                if (map_data.get("game_version", "emerald") in {"hns", "emerald"}
                        and map_name not in RETIRED_HNS_MAP_NAMES):
                    included.append(map_data)

        included_ids = {map_data["id"] for map_data in included}
        self.assertEqual(len(included_ids), len(included))
        self.assertTrue(any(item.get("game_version") == "hns" for item in included))
        self.assertTrue(any(item.get("game_version", "emerald") == "emerald" for item in included))
        for map_data in included:
            for warp_index, warp in enumerate(map_data.get("warp_events", [])):
                destination = (
                    "MAP_SEAFOAM_ISLANDS_1F_COAST_POC"
                    if map_data["name"] == "Route20_hns" and warp_index < 2
                    else warp["dest_map"]
                )
                self.assertIn(
                    destination,
                    included_ids | {"MAP_DYNAMIC", "MAP_UNDEFINED"},
                )
            for connection in map_data.get("connections") or []:
                self.assertIn(connection["map"], included_ids)

        heal_locations = json.loads(
            (GAME_ROOT / "src/data/heal_locations.json").read_text()
        )["heal_locations"]
        self.assertEqual(
            [item["source"] for item in heal_locations],
            ["EMERALD"] * 22 + ["FRLG"] * 20 + ["HNS"] * 32,
        )
        for item in heal_locations:
            if item["source"] in {"EMERALD", "HNS"}:
                self.assertIn(item["map"], included_ids)
                if "respawn_map" in item:
                    self.assertIn(item["respawn_map"], included_ids)


if __name__ == "__main__":
    unittest.main()
