import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


GAME_ROOT = Path(__file__).resolve().parents[3]


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

    @staticmethod
    def write_sevii_manifest(root, maps, release_link_enabled=False):
        path = root / "src/data/wayfarer_sevii_maps.json"
        path.write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "release_link_enabled": release_link_enabled,
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
                 "trainer_sight_or_berry_tree_id": "1", "script": "Frlg_Trainer", "flag": "0"},
                {"type": "object", "graphics_id": "OBJ_EVENT_GFX_NURSE", "x": 2, "y": 1,
                 "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                 "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NONE",
                 "trainer_sight_or_berry_tree_id": "0", "script": "Frlg_Story", "flag": "0"},
            ],
            "warp_events": [
                {"x": 1, "y": 5, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_SEVII_1F"},
                {"x": 5, "y": 1, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_UNION_ROOM_FRLG"},
                {"x": 9, "y": 1, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_TRADE_CENTER_FRLG"},
            ],
            "coord_events": [{"type": "trigger", "x": 1, "y": 1, "elevation": 0,
                              "var": "VAR_TEMP_0", "var_value": "0", "script": "Frlg_Cutscene"}],
            "bg_events": [{"type": "sign", "x": 1, "y": 2, "elevation": 0,
                           "player_facing_dir": "BG_EVENT_PLAYER_FACING_ANY", "script": "Frlg_Sign"}],
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
             "retained_events": {"object_events": [], "coord_events": [], "bg_events": []}},
            {"source_map": "OneIsland_PokemonCenter_1F_Frlg", "map_id": "MAP_SEVII_1F",
             "layout": "LAYOUT_SEVII_CENTER_1F", "enabled": True},
        ], release_link_enabled=True)

        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        events = (map_dir / "events.inc").read_text()
        self.assertNotIn("ObjectEvents", events)
        self.assertNotIn("CoordEvents", events)
        self.assertNotIn("BGEvents", events)
        self.assertIn("MAP_SEVII_1F", events)
        self.assertNotIn("MAP_UNION_ROOM_FRLG", events)
        self.assertNotIn("MAP_TRADE_CENTER_FRLG", events)
        self.assertNotIn("Frlg_Trainer", events)
        self.assertNotIn("Frlg_Story", events)
        self.assertNotIn("Frlg_Cutscene", events)
        self.assertNotIn("Frlg_Sign", events)
        connections = (map_dir / "connections.inc").read_text()
        self.assertIn("MAP_SEVII_1F", connections)

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
                if map_data.get("game_version", "emerald") in {"hns", "emerald"}:
                    included.append(map_data)

        included_ids = {map_data["id"] for map_data in included}
        self.assertEqual(len(included_ids), len(included))
        self.assertTrue(any(item.get("game_version") == "hns" for item in included))
        self.assertTrue(any(item.get("game_version", "emerald") == "emerald" for item in included))
        for map_data in included:
            for warp in map_data.get("warp_events", []):
                self.assertIn(
                    warp["dest_map"],
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
