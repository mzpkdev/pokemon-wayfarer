import json
import os
from pathlib import Path
import re
import struct
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
    def add_map(root, name, map_id, source_version, warps=None, region=None,
                wayfarer_include=False):
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
        if wayfarer_include:
            data["wayfarer_include"] = True
        (map_dir / "map.json").write_text(json.dumps(data))
        return map_dir / "map.json"

    def run_groups(self, root, version, map_files):
        return subprocess.run(
            [
                str(self.mapjson),
                "groups",
                version,
                "data/maps/map_groups.json",
                *(str(path.relative_to(root)) for path in map_files),
                "data/maps",
                "include/constants",
            ],
            cwd=root,
            text=True,
            capture_output=True,
        )

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

    def test_wayfarer_selects_explicit_frlg_opt_in_and_warp_override_only_for_wayfarer(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(root, "HnsMap", "MAP_HNS", "hns")
        frlg = self.add_map(
            root,
            "AnneInterior",
            "MAP_ANNE",
            "frlg",
            [{
                "x": 1,
                "y": 2,
                "elevation": 0,
                "dest_warp_id": "0",
                "dest_map": "MAP_EXTERIOR",
                "wayfarer_dest_map": "MAP_DYNAMIC",
            }],
            wayfarer_include=True,
        )
        (root / "data/maps/map_groups.json").write_text(
            json.dumps({
                "group_order": ["gHns", "gAnne"],
                "gHns": ["HnsMap"],
                "gAnne": ["AnneInterior"],
                "connections_include_order": [],
            })
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps({"heal_locations": []})
        )

        result = self.run_groups(root, "wayfarer", [hns, frlg])

        self.assertEqual(result.returncode, 0, result.stderr)
        groups = (root / "data/maps/groups.inc").read_text()
        self.assertIn("gAnne::\n\t.4byte AnneInterior", groups)
        border = root / "data/layouts/Anne.border.bin"
        blockdata = root / "data/layouts/Anne.map.bin"
        border.touch()
        blockdata.touch()
        (root / "data/layouts/layouts.json").write_text(json.dumps({
            "layouts_table_label": "gMapLayouts",
            "layouts": [{
                "id": "LAYOUT_ANNE",
                "name": "gMapLayout_Anne",
                "game_version": "frlg",
                "wayfarer_include": True,
                "layout_version": "frlg",
                "width": 1,
                "height": 1,
                "border_filepath": str(border.relative_to(root)),
                "blockdata_filepath": str(blockdata.relative_to(root)),
                "primary_tileset": "gTileset_General",
                "secondary_tileset": "gTileset_Petalburg",
                "border_width": 2,
                "border_height": 2,
            }],
        }))
        anne_data = json.loads(frlg.read_text())
        anne_data.update({
            "layout": "LAYOUT_ANNE",
            "music": "MUS_NONE",
            "region_map_section": "MAPSEC_NONE",
            "requires_flash": False,
            "weather": "WEATHER_NONE",
            "map_type": "MAP_TYPE_TOWN",
            "allow_cycling": True,
            "allow_escaping": False,
            "allow_running": True,
            "show_map_name": True,
            "floor_number": 0,
            "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
            "object_events": [],
            "coord_events": [],
            "bg_events": [],
        })
        frlg.write_text(json.dumps(anne_data))
        def generate_map(version):
            return subprocess.run(
                [
                    str(self.mapjson), "map", version,
                    "data/maps/AnneInterior/map.json",
                    "data/layouts/layouts.json",
                    "data/maps/AnneInterior",
                ],
                cwd=root,
                text=True,
                capture_output=True,
            )

        result = generate_map("wayfarer")

        self.assertEqual(result.returncode, 0, result.stderr)
        events = (root / "data/maps/AnneInterior/events.inc").read_text()
        self.assertIn("warp_def 1, 2, 0, 0, MAP_DYNAMIC", events)

        result = generate_map("firered")

        self.assertEqual(result.returncode, 0, result.stderr)
        events = (root / "data/maps/AnneInterior/events.inc").read_text()
        self.assertIn("warp_def 1, 2, 0, 0, MAP_EXTERIOR", events)

    def test_persistent_anne_return_and_rewards_use_saved_wayfarer_state(self):
        """Keep the direct Anne return and every imported one-time item auditable."""
        maps_root = GAME_ROOT / "data/maps"
        corridor = json.loads(
            (maps_root / "SSAnne_1F_Corridor_Frlg/map.json").read_text()
        )
        dynamic_exits = {
            (warp["x"], warp["y"], warp["elevation"], warp["dest_warp_id"])
            for warp in corridor["warp_events"]
            if warp.get("wayfarer_dest_map") == "MAP_DYNAMIC"
        }
        self.assertEqual(dynamic_exits, {(19, 1, 3, "2"), (20, 0, 0, "3")})
        self.assertEqual(
            [
                warp["dest_map"]
                for warp in corridor["warp_events"]
                if warp.get("wayfarer_dest_map") == "MAP_DYNAMIC"
            ],
            ["MAP_SSANNE_EXTERIOR", "MAP_SSANNE_EXTERIOR"],
        )

        corridor_script = (
            maps_root / "SSAnne_1F_Corridor_Frlg/scripts.inc"
        ).read_text()
        self.assertIn(
            "setdynamicwarp MAP_VERMILION_CITY_PORT_INSIDE_HNS, 8, 9",
            corridor_script,
        )
        sailor_script = (
            maps_root / "VermilionCity_PortInside_hns/scripts.inc"
        ).read_text()
        self.assertIn("warp MAP_SSANNE_1F_CORRIDOR, 19, 2", sailor_script)

        layouts = {
            layout["id"]: layout
            for layout in json.loads(
                (GAME_ROOT / "data/layouts/layouts.json").read_text()
            )["layouts"]
        }

        def grid(layout_id, x, y):
            layout = layouts[layout_id]
            blocks = (GAME_ROOT / layout["blockdata_filepath"]).read_bytes()
            self.assertEqual(len(blocks), layout["width"] * layout["height"] * 2)
            value = struct.unpack_from("<H", blocks, 2 * (y * layout["width"] + x))[0]
            return (value & 0x3FF, (value >> 10) & 0x3, (value >> 12) & 0xF)

        # The existing north door remains a real S.S. Anne door metatile, so
        # the Wayfarer-only animation table is connected to the imported
        # corridor rather than merely compiled alongside it.
        door_metatile, door_collision, door_elevation = grid(
            "LAYOUT_SSANNE_1F_CORRIDOR", 19, 1
        )
        self.assertEqual(door_metatile, 0x281)  # METATILE_SSAnne_Door
        self.assertEqual((door_collision, door_elevation), (1, 0))

        # The boarded tile and the corridor route into the ship are public,
        # elevation-three walkable tiles. The return is likewise walkable and
        # has a clear line to Vermilion's existing public exit at (8, 2).
        for coordinate in [(19, 2), *( (20, y) for y in range(2, 11) )]:
            _, collision, elevation = grid("LAYOUT_SSANNE_1F_CORRIDOR", *coordinate)
            self.assertEqual((collision, elevation), (0, 3), coordinate)
        for coordinate in [(8, y) for y in range(2, 10)]:
            _, collision, elevation = grid("LAYOUT_VERMILION_CITY_PORT_INSIDE_HNS", *coordinate)
            self.assertEqual((collision, elevation), (0, 3), coordinate)
        port = json.loads(
            (maps_root / "VermilionCity_PortInside_hns/map.json").read_text()
        )
        self.assertIn(
            (8, 2, "MAP_VERMILION_CITY_PORT_OUTSIDE_HNS"),
            {(warp["x"], warp["y"], warp["dest_map"]) for warp in port["warp_events"]},
        )
        for events in ("coord_events", "warp_events", "object_events"):
            self.assertNotIn(
                (8, 9),
                {(event["x"], event["y"]) for event in port[events]},
                events,
            )
        self.assertNotIn(
            (19, 2),
            {
                (event["x"], event["y"])
                for events in ("coord_events", "warp_events", "object_events")
                for event in corridor[events]
            },
        )

        expected_item_flags = {
            "FLAG_HIDE_SSANNE_1F_ROOM2_TM31": 0x496,
            "FLAG_HIDE_SSANNE_2F_ROOM2_STARDUST": 0x497,
            "FLAG_HIDE_SSANNE_2F_ROOM4_X_ATTACK": 0x498,
            "FLAG_HIDE_SSANNE_B1F_ROOM2_TM44": 0x499,
            "FLAG_HIDE_SSANNE_B1F_ROOM3_ETHER": 0x49A,
            "FLAG_HIDE_SSANNE_B1F_ROOM5_SUPER_POTION": 0x49B,
            "FLAG_HIDE_SSANNE_KITCHEN_GREAT_BALL": 0x49C,
            "FLAG_HIDDEN_ITEM_SSANNE_B1F_CORRIDOR_HYPER_POTION": 0x49E,
            "FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_CHESTO_BERRY": 0x49F,
            "FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_PECHA_BERRY": 0x4A0,
            "FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_CHERI_BERRY": 0x4A1,
        }
        included_anne_maps = [
            json.loads(path.read_text())
            for path in maps_root.glob("SSAnne_*_Frlg/map.json")
            if json.loads(path.read_text()).get("wayfarer_include") is True
        ]
        item_flags = {
            event["flag"]
            for map_data in included_anne_maps
            for events in ("object_events", "bg_events")
            for event in map_data.get(events, [])
            if event.get("flag") in expected_item_flags
        }
        self.assertEqual(item_flags, set(expected_item_flags))

        flags_header = (GAME_ROOT / "include/constants/flags_hns.h").read_text()
        wayfarer_flag_blocks = re.findall(
            r"#if IS_WAYFARER\s*\n(.*?)#else", flags_header, re.DOTALL
        )
        for flag, value in expected_item_flags.items():
            self.assertTrue(any(
                re.search(rf"#define\s+{flag}\s+0x{value:03X}\b", block)
                for block in wayfarer_flag_blocks
            ), flag)
        self.assertTrue(any(
            re.search(r"#define\s+FLAG_SS_ANNE_CAPTAIN_REWARD_HNS\s+0x49D\b", block)
            for block in wayfarer_flag_blocks
        ))
        saved_flags = [*expected_item_flags.values(), 0x49D]
        self.assertEqual(len(saved_flags), len(set(saved_flags)))
        self.assertTrue(all(0x22 <= flag <= 0x4FF for flag in saved_flags))

        macro_output = subprocess.run(
            [
                "cpp", "-dM", "-DPOKEMON_WAYFARER",
                "-I", str(GAME_ROOT / "include"),
                "-include", "global.h",
                "-include", "constants/flags.h",
                "-include", "constants/vars.h",
                "-",
            ],
            cwd=GAME_ROOT,
            input="",
            text=True,
            capture_output=True,
            check=True,
        ).stdout
        macros = dict(re.findall(r"^#define\s+(\w+)\s+(.+)$", macro_output, re.MULTILINE))
        self.assertEqual(macros["VAR_MAP_SCENE_S_S_ANNE_2F_CORRIDOR"], "0x40D8")
        self.assertEqual(macros["FLAG_HIDE_SS_ANNE_RIVAL"], "0x4A2")
        self.assertEqual(macros["FLAG_SS_ANNE_BLUE_MET_HNS"], "0x4A3")
        # 0x405B is Cianwood's persisted map-state slot in the Wayfarer
        # source namespace. Anne must never retake that source value.
        self.assertNotEqual(macros["VAR_MAP_SCENE_S_S_ANNE_2F_CORRIDOR"], "0x405B")

    def test_persistent_anne_graphics_and_map_section_are_wayfarer_complete(self):
        """Every imported Anne NPC must have a concrete Wayfarer sprite entry."""
        maps_root = GAME_ROOT / "data/maps"
        included_anne_maps = [
            json.loads(path.read_text())
            for path in maps_root.glob("SSAnne_*_Frlg/map.json")
            if json.loads(path.read_text()).get("wayfarer_include") is True
        ]
        anne_graphics = {
            event["graphics_id"]
            for map_data in included_anne_maps
            for event in map_data["object_events"]
        }
        self.assertEqual(
            anne_graphics,
            {
                "OBJ_EVENT_GFX_SAILOR_FRLG",
                "OBJ_EVENT_GFX_WORKER_M",
                "OBJ_EVENT_GFX_GENTLEMAN_FRLG",
                "OBJ_EVENT_GFX_LASS_FRLG",
                "OBJ_EVENT_GFX_YOUNGSTER_FRLG",
                "OBJ_EVENT_GFX_WOMAN_2_FRLG",
                "OBJ_EVENT_GFX_ITEM_BALL",
                "OBJ_EVENT_GFX_LITTLE_GIRL_FRLG",
                "OBJ_EVENT_GFX_WIGGLYTUFF",
                "OBJ_EVENT_GFX_BALDING_MAN",
                "OBJ_EVENT_GFX_WOMAN_1_FRLG",
                "OBJ_EVENT_GFX_BLUE",
                "OBJ_EVENT_GFX_FISHER",
                "OBJ_EVENT_GFX_OLD_MAN_1",
                "OBJ_EVENT_GFX_LITTLE_BOY_FRLG",
                "OBJ_EVENT_GFX_WOMAN_3_FRLG",
                "OBJ_EVENT_GFX_BOY",
                "OBJ_EVENT_GFX_MACHOKE",
                "OBJ_EVENT_GFX_CAPTAIN",
                "OBJ_EVENT_GFX_CHEF",
            },
        )
        pointer_output = subprocess.run(
            [
                "cpp", "-P", "-DPOKEMON_WAYFARER",
                "-I", str(GAME_ROOT / "include"),
                "-include", "global.h",
                str(GAME_ROOT / "src/data/object_events/object_event_graphics_info_pointers.h"),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
            check=True,
        ).stdout
        object_output = subprocess.run(
            [
                "cpp", "-P", "-DPOKEMON_WAYFARER",
                "-I", str(GAME_ROOT / "include"),
                "-include", "global.h",
                str(GAME_ROOT / "src/event_object_movement.c"),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
            check=True,
        ).stdout
        for graphics_id in anne_graphics:
            pointer = re.search(
                rf"\[\s*{graphics_id}\s*\]\s*=\s*&(gObjectEventGraphicsInfo_\w+),",
                pointer_output,
            )
            self.assertIsNotNone(pointer, graphics_id)
            self.assertRegex(
                object_output,
                rf"const struct ObjectEventGraphicsInfo {pointer.group(1)}\b",
            )

        field_door_source = (GAME_ROOT / "src/field_door.c").read_text()
        door_output = subprocess.run(
            [
                "cpp", "-P", "-DPOKEMON_WAYFARER",
                "-I", str(GAME_ROOT / "include"),
                str(GAME_ROOT / "src/field_door.c"),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
            check=True,
        ).stdout
        self.assertIn("sDoorAnimTiles_SSAnne", door_output)
        self.assertIn("sDoorAnimPalettes_SSAnne", door_output)
        self.assertRegex(
            field_door_source,
            r"\{\s*METATILE_SSAnne_Door,\s*&gTileset_SSAnne,\s*DOOR_SOUND_NORMAL,\s*2,\s*sDoorAnimTiles_SSAnne,\s*sDoorAnimPalettes_SSAnne\s*\}",
        )
        self.assertRegex(
            door_output,
            r"\{\s*0x281,\s*&gTileset_SSAnne,\s*0,\s*2,\s*sDoorAnimTiles_SSAnne,\s*sDoorAnimPalettes_SSAnne\s*\}",
        )
        hns_door_output = subprocess.run(
            [
                "cpp", "-P", "-DPOKEMON_HNS",
                "-I", str(GAME_ROOT / "include"),
                str(GAME_ROOT / "src/field_door.c"),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
            check=True,
        ).stdout
        self.assertNotIn("sDoorAnimTiles_SSAnne", hns_door_output)
        self.assertNotIn("sDoorAnimPalettes_SSAnne", hns_door_output)

        sections_data = json.loads(
            (GAME_ROOT / "src/data/region_map/region_map_sections.json").read_text()
        )
        anne_section = next(
            section for section in sections_data["map_sections"]
            if section["id"] == "MAPSEC_S_S_ANNE"
        )
        self.assertEqual(
            anne_section,
            {
                "id": "MAPSEC_S_S_ANNE",
                "name": "S.S. ANNE",
                "x": 14,
                "y": 9,
                "width": 1,
                "height": 1,
                "wayfarer_hns": True,
            },
        )
        self.assertNotIn(
            "MAPSEC_S_S_ANNE",
            {section["id"] for section in sections_data["hns_map_sections"]},
        )
        entries = (GAME_ROOT / "src/data/region_map/region_map_entries.h").read_text()
        self.assertRegex(
            entries,
            r"(?s)#if IS_WAYFARER\s*\n\s*\[MAPSEC_S_S_ANNE\].*?S\.S\. ANNE",
        )
        region_map_source = (GAME_ROOT / "src/region_map.c").read_text()
        region_map_output = subprocess.run(
            [
                "cpp", "-P", "-DPOKEMON_WAYFARER",
                "-I", str(GAME_ROOT / "include"),
                str(GAME_ROOT / "src/region_map.c"),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
            check=True,
        ).stdout
        # Before Kanto is visited, the generated Johto table names Anne at its
        # source coordinates. Afterward the combined JK table keeps that name
        # and gives it Vermilion's visible harbor position.
        self.assertRegex(
            entries,
            r"(?s)\[MAPSEC_S_S_ANNE\].*?\.x = 14,.*?\.y = 9,.*?S\.S\. ANNE",
        )
        self.assertRegex(
            region_map_output,
            r"(?s)\[MAPSEC_S_S_ANNE\]\s*=\s*\{\s*24,\s*7,\s*1,\s*1,.*?S\.S\. ANNE.*?\}",
        )
        self.assertRegex(
            region_map_source,
            r"(?s)FlagGet\(FLAG_VISITED_KANTO\).*?sRegionMapEntries_JK.*?else.*?gRegionMapEntries",
        )
        wayfarer_regions = subprocess.run(
            [
                "cpp", "-P", "-DPOKEMON_WAYFARER",
                "-I", str(GAME_ROOT / "include"),
                str(GAME_ROOT / "include/regions.h"),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
            check=True,
        ).stdout
        self.assertRegex(
            wayfarer_regions,
            r"if \(sectionId == MAPSEC_S_S_ANNE\)\s*return REGION_KANTO;",
        )
        hns_regions = subprocess.run(
            [
                "cpp", "-P", "-DPOKEMON_HNS",
                "-I", str(GAME_ROOT / "include"),
                str(GAME_ROOT / "include/regions.h"),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
            check=True,
        ).stdout
        self.assertNotRegex(
            hns_regions,
            r"if \(sectionId == MAPSEC_S_S_ANNE\)\s*return REGION_KANTO;",
        )

        def assert_map_sections(build_define, expected):
            source = "\n".join(
                [
                    '#include "constants/region_map_sections.h"',
                    *(f'_Static_assert({section} == {value}, "{section}");'
                      for section, value in expected.items()),
                ]
            )
            with tempfile.TemporaryDirectory() as build_dir:
                source_path = Path(build_dir) / "map_sections.c"
                source_path.write_text(source)
                result = subprocess.run(
                    [
                        os.environ.get("CC", "cc"), "-std=c11",
                        f"-D{build_define}", "-I", str(GAME_ROOT / "include"),
                        "-fsyntax-only", str(source_path),
                    ],
                    cwd=GAME_ROOT,
                    text=True,
                    capture_output=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)

        # Anne follows the complete existing HNS range in Wayfarer. The real
        # map-section IDs remain stable; only its sentinel advances there.
        assert_map_sections(
            "POKEMON_WAYFARER",
            {
                "MAPSEC_ABANDONED_SHIP": 1,
                "MAPSEC_VERMILION_CITY": 25,
                "MAPSEC_WHIRL_ISLANDS": 124,
                "MAPSEC_S_S_ANNE": 125,
                "MAPSEC_NONE": 126,
            },
        )
        # Standalone HNS retains its original map-section universe exactly.
        assert_map_sections(
            "POKEMON_HNS",
            {
                "MAPSEC_VERMILION_CITY": 25,
                "MAPSEC_WHIRL_ISLANDS": 124,
                "MAPSEC_S_S_ANNE": 0,
                "MAPSEC_NONE": 125,
            },
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
        anne_interior_names = {
            "SSAnne_1F_Corridor_Frlg",
            "SSAnne_1F_Room1_Frlg", "SSAnne_1F_Room2_Frlg", "SSAnne_1F_Room3_Frlg",
            "SSAnne_1F_Room4_Frlg", "SSAnne_1F_Room5_Frlg", "SSAnne_1F_Room6_Frlg",
            "SSAnne_1F_Room7_Frlg", "SSAnne_2F_Corridor_Frlg", "SSAnne_2F_Room1_Frlg",
            "SSAnne_2F_Room2_Frlg", "SSAnne_2F_Room3_Frlg", "SSAnne_2F_Room4_Frlg",
            "SSAnne_2F_Room5_Frlg", "SSAnne_2F_Room6_Frlg", "SSAnne_3F_Corridor_Frlg",
            "SSAnne_B1F_Corridor_Frlg", "SSAnne_B1F_Room1_Frlg", "SSAnne_B1F_Room2_Frlg",
            "SSAnne_B1F_Room3_Frlg", "SSAnne_B1F_Room4_Frlg", "SSAnne_B1F_Room5_Frlg",
            "SSAnne_CaptainsOffice_Frlg", "SSAnne_Deck_Frlg", "SSAnne_Kitchen_Frlg",
        }
        anne_layout_ids = {
            "LAYOUT_SSANNE_1F_CORRIDOR", "LAYOUT_SSANNE_2F_CORRIDOR",
            "LAYOUT_SSANNE_3F_CORRIDOR", "LAYOUT_SSANNE_B1F_CORRIDOR",
            "LAYOUT_SSANNE_CAPTAINS_OFFICE", "LAYOUT_SSANNE_DECK",
            "LAYOUT_SSANNE_KITCHEN", "LAYOUT_SSANNE_ROOM1", "LAYOUT_SSANNE_ROOM2",
        }
        included = []
        for group_num, group_name in enumerate(groups["group_order"]):
            self.assertLessEqual(group_num, 127)
            for map_num, map_name in enumerate(groups[group_name]):
                self.assertLessEqual(map_num, 127)
                map_data = data[map_name]
                if (map_data.get("game_version", "emerald") in {"hns", "emerald"}
                        or map_data.get("wayfarer_include") is True):
                    included.append(map_data)

        included_ids = {map_data["id"] for map_data in included}
        self.assertEqual(len(included_ids), len(included))
        self.assertTrue(any(item.get("game_version") == "hns" for item in included))
        self.assertTrue(any(item.get("game_version", "emerald") == "emerald" for item in included))
        self.assertEqual(
            {item["name"] for item in included if item.get("game_version") == "frlg"},
            anne_interior_names,
        )
        self.assertNotIn("SSAnne_Exterior_Frlg", {item["name"] for item in included})
        layouts = json.loads((GAME_ROOT / "data/layouts/layouts.json").read_text())["layouts"]
        self.assertEqual(
            {layout["id"] for layout in layouts
             if layout.get("game_version") == "frlg" and layout.get("wayfarer_include") is True},
            anne_layout_ids,
        )
        dungeons = groups["gMapGroup_Dungeons_Frlg"]
        self.assertEqual(dungeons[5], "SSAnne_1F_Corridor_Frlg")
        self.assertEqual(dungeons[6], "SSAnne_2F_Corridor_Frlg")
        self.assertEqual(dungeons[8], "SSAnne_B1F_Corridor_Frlg")
        self.assertEqual(dungeons[11], "SSAnne_CaptainsOffice_Frlg")
        for map_data in included:
            for warp in map_data.get("warp_events", []):
                destination = warp.get("wayfarer_dest_map", warp["dest_map"])
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
