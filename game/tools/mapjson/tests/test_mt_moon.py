"""Static contract for Wayfarer's local Mt. Moon fossil adventure."""

import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import unittest


GAME_ROOT = Path(__file__).resolve().parents[3]
MAP_PATH = GAME_ROOT / "data/maps/MtMoon_Cave_hns/map.json"


class MtMoonWayfarerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build_dir = tempfile.TemporaryDirectory()
        cls.mapjson = Path(cls.build_dir.name) / "mapjson"
        subprocess.run(
            [
                os.environ.get("CXX", "g++"), "-std=c++17",
                str(GAME_ROOT / "tools/mapjson/mapjson.cpp"),
                str(GAME_ROOT / "tools/mapjson/json11.cpp"),
                "-o", str(cls.mapjson),
            ],
            check=True,
        )
        cls.map_data = json.loads(MAP_PATH.read_text())
        cls.layouts = {
            entry["id"]: entry
            for entry in json.loads((GAME_ROOT / "data/layouts/layouts.json").read_text())["layouts"]
        }

    @classmethod
    def tearDownClass(cls):
        cls.build_dir.cleanup()

    def generated_events(self, version):
        output_dir = Path(self.build_dir.name) / version
        output_dir.mkdir()
        result = subprocess.run(
            [
                str(self.mapjson), "map", version,
                "data/maps/MtMoon_Cave_hns/map.json", "data/layouts/layouts.json", str(output_dir),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
        )
        if result.returncode:
            raise AssertionError(result.stderr)
        return (output_dir / "events.inc").read_text()

    def grid(self, x, y):
        layout = self.layouts["LAYOUT_MT_MOON_CAVE_HNS"]
        blocks = (GAME_ROOT / layout["blockdata_filepath"]).read_bytes()
        value = struct.unpack_from("<H", blocks, 2 * (y * layout["width"] + x))[0]
        return value & 0x3FF, (value >> 10) & 0x3, (value >> 12) & 0xF

    def test_selected_events_preserve_hns_and_append_the_local_adventure(self):
        objects = self.map_data["object_events"]
        hns = [event for event in objects if not event.get("wayfarer_only")]
        wayfarer = [event for event in objects if event.get("wayfarer_only")]
        self.assertEqual(len(hns), 15)
        self.assertEqual(
            [(event["x"], event["y"], event["graphics_id"], event["flag"])
            for event in hns],
            [
                (9, 11, "OBJ_EVENT_GFX_SILVER_HNS", "FLAG_HIDE_MTMOON_SILVER"),
                (14, 10, "OBJ_EVENT_GFX_MON_BASE+SPECIES_GEODUDE", "0"),
                (36, 12, "OBJ_EVENT_GFX_MON_BASE+SPECIES_SOLROCK", "FLAG_DAY_POKEMON"),
                (25, 31, "OBJ_EVENT_GFX_MON_BASE+SPECIES_CLEFAIRY", "FLAG_NIGHT_POKEMON"),
                (35, 12, "OBJ_EVENT_GFX_MON_BASE+SPECIES_LUNATONE", "FLAG_NIGHT_POKEMON"),
                (17, 4, "OBJ_EVENT_GFX_FOSSIL_HNS", "0"),
                (16, 4, "OBJ_EVENT_GFX_SCIENTIST_M_HNS", "0"),
                (15, 4, "OBJ_EVENT_GFX_MON_BASE+SPECIES_GEODUDE", "0"),
                (16, 5, "OBJ_EVENT_GFX_MON_BASE+SPECIES_GEODUDE", "0"),
                (16, 3, "OBJ_EVENT_GFX_MON_BASE+SPECIES_GEODUDE", "0"),
                (46, 19, "OBJ_EVENT_GFX_MON_BASE+SPECIES_SANDSHREW", "0"),
                (31, 32, "OBJ_EVENT_GFX_MON_BASE+SPECIES_CLEFAIRY", "0"),
                (14, 33, "OBJ_EVENT_GFX_MON_BASE+SPECIES_PARAS", "0"),
                (21, 18, "OBJ_EVENT_GFX_MON_BASE+SPECIES_PARAS", "0"),
                (48, 34, "OBJ_EVENT_GFX_MON_BASE+SPECIES_PARAS", "0"),
            ],
        )
        self.assertEqual(
            [
                (event["local_id"], event["graphics_id"], event["x"], event["y"],
                 event["movement_type"], event["trainer_sight_or_berry_tree_id"],
                 event["script"], event["flag"])
                for event in wayfarer
            ],
            [
                ("LOCALID_MTMOON_ROCKET_GRUNT_1", "OBJ_EVENT_GFX_ROCKET_M", 10, 14,
                 "MOVEMENT_TYPE_FACE_DOWN", "1", "WayfarerMtMoon_EventScript_Grunt1", "0"),
                ("LOCALID_MTMOON_ROCKET_GRUNT_2", "OBJ_EVENT_GFX_ROCKET_M", 38, 15,
                 "MOVEMENT_TYPE_FACE_DOWN", "1", "WayfarerMtMoon_EventScript_Grunt2", "0"),
                ("LOCALID_MTMOON_ROCKET_GRUNT_3", "OBJ_EVENT_GFX_ROCKET_M", 44, 27,
                 "MOVEMENT_TYPE_FACE_DOWN", "1", "WayfarerMtMoon_EventScript_Grunt3", "0"),
                ("LOCALID_MTMOON_ROCKET_GRUNT_4", "OBJ_EVENT_GFX_ROCKET_M", 33, 15,
                 "MOVEMENT_TYPE_FACE_DOWN", "1", "WayfarerMtMoon_EventScript_Grunt4", "0"),
                ("LOCALID_MTMOON_MIGUEL", "OBJ_EVENT_GFX_SCIENTIST_M_HNS", 25, 15,
                 "MOVEMENT_TYPE_FACE_DOWN", "0", "WayfarerMtMoon_EventScript_Miguel", "0"),
                ("LOCALID_MTMOON_DOME_FOSSIL", "OBJ_EVENT_GFX_FOSSIL_HNS", 24, 15,
                 "MOVEMENT_TYPE_FACE_DOWN", "0", "WayfarerMtMoon_EventScript_DomeFossil",
                 "FLAG_HIDE_MT_MOON_DOME_FOSSIL_HNS"),
                ("LOCALID_MTMOON_HELIX_FOSSIL", "OBJ_EVENT_GFX_FOSSIL_HNS", 26, 15,
                 "MOVEMENT_TYPE_FACE_DOWN", "0", "WayfarerMtMoon_EventScript_HelixFossil",
                 "FLAG_HIDE_MT_MOON_HELIX_FOSSIL_HNS"),
            ],
        )
        self.assertEqual(
            [(event["x"], event["y"], event["dest_map"], event["dest_warp_id"])
            for event in self.map_data["warp_events"]],
            [
                (46, 31, "MAP_ROUTE4_HNS", "0"), (4, 12, "MAP_ROUTE4_HNS", "1"),
                (24, 10, "MAP_MT_MOON_OUTSIDE_HNS", "1"),
                (32, 21, "MAP_MT_MOON_OUTSIDE_HNS", "2"),
            ],
        )
        self.assertEqual(
            [(event["x"], event["y"], event.get("script"), event.get("flag"))
            for event in self.map_data["bg_events"]],
            [(26, 21, None, "FLAG_HIDDEN_ITEM_MT_MOON_CAVE_REVIVE"),
             (15, 14, "MtMoon_Cave_EventScript_Sign", None)],
        )
        self.assertEqual(
            hashlib.sha256((GAME_ROOT / "data/layouts/MtMoon_Cave_hns/map.bin").read_bytes()).hexdigest(),
            "88c99af277ec9a1c18c8a4e8a1f7287f0c2347001f8ca34ce96a6e49cd7bfa9e",
        )

        # Every actor tile and its stated player approach remain passable on
        # the untouched HNS cave geometry. Object-event elevations are asserted
        # by the exact event fixture above; block elevation remains the HNS
        # layout's native value.
        for x, y in ((10, 14), (10, 15), (38, 15), (38, 16), (44, 27), (44, 28),
                     (33, 15), (33, 16), (25, 15), (25, 16), (24, 15), (24, 16),
                     (26, 15), (26, 16)):
            _, collision, _ = self.grid(x, y)
            self.assertEqual(collision, 0, (x, y))

        hns_events = self.generated_events("hns")
        wayfarer_events = self.generated_events("wayfarer")
        self.assertEqual(hns_events.count("\tobject_event "), 15)
        self.assertEqual(wayfarer_events.count("\tobject_event "), 22)
        for label in ("WayfarerMtMoon_EventScript_Grunt1", "WayfarerMtMoon_EventScript_Miguel",
                      "FLAG_HIDE_MT_MOON_DOME_FOSSIL_HNS"):
            self.assertNotIn(label, hns_events)
            self.assertIn(label, wayfarer_events)

        # TrySpawnObjectEvents considers a 20-by-17 template window around
        # the player (MAP_OFFSET_W + 4 by MAP_OFFSET_H + 3). Count every
        # template as visible, including day/night alternatives, so an event
        # or flag edit cannot rely on spawn order to exceed the 15 NPC slots.
        layout = self.layouts["LAYOUT_MT_MOON_CAVE_HNS"]
        viewport_counts = []
        for player_x in range(layout["width"]):
            for player_y in range(layout["height"]):
                viewport_counts.append(sum(
                    player_x - 2 <= event["x"] <= player_x + 17
                    and player_y <= event["y"] <= player_y + 16
                    for event in objects
                ))
        self.assertEqual(max(viewport_counts), 12)
        self.assertLessEqual(max(viewport_counts), 15)

    def test_local_state_and_dialogue_are_wayfarer_scoped(self):
        macros = {}
        for define in ("POKEMON_WAYFARER", "POKEMON_HNS"):
            output = subprocess.run(
                ["cpp", "-dM", f"-D{define}", "-I", str(GAME_ROOT / "include"),
                 "-include", "global.h", "-include", "constants/flags.h", "-"],
                cwd=GAME_ROOT, input="", text=True, capture_output=True, check=True,
            ).stdout
            macros[define] = dict(re.findall(r"^#define\s+(\w+)\s+(.+)$", output, re.MULTILINE))
        expected = {
            "FLAG_HIDE_MT_MOON_DOME_FOSSIL_HNS": "0x4BF",
            "FLAG_HIDE_MT_MOON_HELIX_FOSSIL_HNS": "0x4C0",
            "FLAG_GOT_FOSSIL_FROM_MT_MOON_HNS": "0x4C1",
        }
        for name, value in expected.items():
            self.assertEqual(macros["POKEMON_WAYFARER"][name], value)
            self.assertEqual(macros["POKEMON_HNS"][name], "0")

        script = (GAME_ROOT / "data/scripts/wayfarer_mt_moon.inc").read_text()
        source = (GAME_ROOT / "data/maps/MtMoon_B2F_Frlg/scripts.inc").read_text()
        for name in ("Grunt1Intro", "Grunt1Defeat", "Grunt1PostBattle", "Grunt2Intro",
                     "Grunt2Defeat", "Grunt2PostBattle", "Grunt3Intro", "Grunt3Defeat",
                     "Grunt3PostBattle", "Grunt4Intro", "Grunt4Defeat", "Grunt4PostBattle"):
            source_block = re.search(rf"(?s)MtMoon_B2F_Text_{name}::(.*?)(?=^\w+::|\Z)", source, re.MULTILINE)
            self.assertIsNotNone(source_block, name)
            target_name = name.replace("PostBattle", "After")
            target_block = re.search(rf"(?s)WayfarerMtMoon_Text_{target_name}::(.*?)(?=^\w+::|\Z)", script, re.MULTILINE)
            self.assertIsNotNone(target_block, target_name)
            self.assertEqual(target_block.group(1).strip(), source_block.group(1).strip(), name)

        self.assertIn("checkitemspace ITEM_HELIX_FOSSIL", script)
        self.assertIn("checkitemspace ITEM_DOME_FOSSIL", script)
        self.assertIn("goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoRoom", script)
        items = (GAME_ROOT / "src/data/items.h").read_text()
        self.assertEqual(items.count("#if I_KEY_FOSSILS >= GEN_4 || IS_WAYFARER"), 2)


if __name__ == "__main__":
    unittest.main()
