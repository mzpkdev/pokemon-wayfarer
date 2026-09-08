"""Static contract for Wayfarer's independent Fuchsia Safari objectives."""

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
MAPS = GAME_ROOT / "data/maps"


class FuchsiaSafariWayfarerTest(unittest.TestCase):
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
        cls.beach = json.loads(
            (MAPS / "FuchsiaCity_SafariZoneBeach_hns/map.json").read_text()
        )
        cls.house = json.loads(
            (MAPS / "FuchsiaCity_House2_hns/map.json").read_text()
        )
        cls.city = json.loads((MAPS / "FuchsiaCity_hns/map.json").read_text())
        cls.layouts = {
            layout["id"]: layout
            for layout in json.loads(
                (GAME_ROOT / "data/layouts/layouts.json").read_text()
            )["layouts"]
        }

    @classmethod
    def tearDownClass(cls):
        cls.build_dir.cleanup()

    @classmethod
    def generated_events(cls, map_name, version):
        output_dir = Path(cls.build_dir.name) / f"{map_name}-{version}"
        output_dir.mkdir()
        result = subprocess.run(
            [
                str(cls.mapjson),
                "map",
                version,
                f"data/maps/{map_name}/map.json",
                "data/layouts/layouts.json",
                str(output_dir),
            ],
            cwd=GAME_ROOT,
            text=True,
            capture_output=True,
        )
        if result.returncode:
            raise AssertionError(result.stderr)
        return (output_dir / "events.inc").read_text()

    @staticmethod
    def selected(events, version):
        return [
            event for event in events
            if not event.get("wayfarer_only") or version == "wayfarer"
        ]

    @staticmethod
    def block(source, label):
        match = re.search(
            rf"^{re.escape(label)}::\n(.*?)(?=^[A-Za-z0-9_]+::|\Z)",
            source,
            re.MULTILINE | re.DOTALL,
        )
        if match is None:
            raise AssertionError(f"missing script label {label}")
        return match.group(1)

    def grid(self, layout_id, x, y):
        layout = self.layouts[layout_id]
        blocks = (GAME_ROOT / layout["blockdata_filepath"]).read_bytes()
        self.assertEqual(len(blocks), layout["width"] * layout["height"] * 2)
        value = struct.unpack_from("<H", blocks, 2 * (y * layout["width"] + x))[0]
        return value & 0x3FF, (value >> 10) & 0x3, (value >> 12) & 0xF

    def assert_walkable_path(self, layout_id, path, blocked):
        self.assertGreater(len(path), 1)
        for current, following in zip(path, path[1:]):
            self.assertEqual(
                abs(current[0] - following[0]) + abs(current[1] - following[1]),
                1,
                (current, following),
            )
        for coordinate in path:
            self.assertNotIn(coordinate, blocked)
            _, collision, elevation = self.grid(layout_id, *coordinate)
            self.assertEqual((collision, elevation), (0, 3), coordinate)

    def test_wayfarer_selects_only_the_three_new_events(self):
        beach_hns = self.selected(self.beach["object_events"], "hns")
        beach_wayfarer = self.selected(self.beach["object_events"], "wayfarer")
        self.assertEqual(len(beach_hns), 8)
        self.assertEqual(
            [(event["x"], event["y"], event["script"]) for event in beach_hns],
            [
                (11, 32, "NULL"), (9, 33, "NULL"), (9, 42, "NULL"),
                (24, 19, "NULL"), (11, 18, "NULL"), (10, 40, "NULL"),
                (18, 18, "NULL"), (0, 12, "SafariZoneFuchsia_EventScript_Attendant2"),
            ],
        )
        self.assertEqual(
            [
                (event["x"], event["y"], event["graphics_id"], event["script"], event["flag"])
                for event in beach_wayfarer if event.get("wayfarer_only")
            ],
            [
                (39, 23, "OBJ_EVENT_GFX_ITEM_BALL", "WayfarerFuchsiaSafari_EventScript_GoldTeeth",
                 "FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS"),
                (4, 23, "OBJ_EVENT_GFX_ATTENDANT_M_HNS", "WayfarerFuchsiaSafari_EventScript_SurfAttendant", "0"),
            ],
        )
        self.assertEqual(len(self.selected(self.house["object_events"], "hns")), 1)
        self.assertEqual(
            [
                (event["x"], event["y"], event["graphics_id"], event["script"], event["flag"])
                for event in self.selected(self.house["object_events"], "hns")
            ],
            [
                (4, 4, "OBJ_EVENT_GFX_WOMAN_1_HNS",
                 "FuchsiaCity_House2_EventScript_Granddaughter", "0"),
            ],
        )
        self.assertEqual(
            [
                (event["x"], event["y"], event["graphics_id"], event["script"], event["flag"])
                for event in self.selected(self.house["object_events"], "wayfarer")
                if event.get("wayfarer_only")
            ],
            [(8, 2, "OBJ_EVENT_GFX_GENTLEMAN_HNS", "WayfarerFuchsiaSafari_EventScript_Baoba", "0")],
        )
        self.assertEqual(
            [(event["x"], event["y"], event["script"]) for event in self.house["bg_events"]],
            [
                (3, 1, "FuchsiaCity_House2_EventScript_Photo"),
                (4, 1, "FuchsiaCity_House2_EventScript_Photo"),
            ],
        )
        self.assertEqual(
            [(event["x"], event["y"], event["dest_map"], event["dest_warp_id"])
            for event in self.house["warp_events"]],
            [
                (4, 8, "MAP_FUCHSIA_CITY_HNS", "6"),
                (1, 1, "MAP_FUCHSIA_CITY_HNS", "7"),
            ],
        )
        self.assertEqual(
            [(event["x"], event["y"]) for event in self.beach["coord_events"]],
            [(1, 12), (1, 12)],
        )
        self.assertFalse(any(event.get("wayfarer_only") for event in self.beach["warp_events"]))

        # Selection is proven against emitted events, not only a duplicated
        # JSON predicate. HNS emits its old maps in event terms; Wayfarer
        # appends exactly the two Beach objects and Baoba.
        beach_hns_events = self.generated_events("FuchsiaCity_SafariZoneBeach_hns", "hns")
        beach_wayfarer_events = self.generated_events("FuchsiaCity_SafariZoneBeach_hns", "wayfarer")
        house_hns_events = self.generated_events("FuchsiaCity_House2_hns", "hns")
        house_wayfarer_events = self.generated_events("FuchsiaCity_House2_hns", "wayfarer")
        self.assertEqual(beach_hns_events.count("\tobject_event "), 8)
        self.assertEqual(beach_wayfarer_events.count("\tobject_event "), 10)
        self.assertEqual(house_hns_events.count("\tobject_event "), 1)
        self.assertEqual(house_wayfarer_events.count("\tobject_event "), 2)
        for label in (
            "WayfarerFuchsiaSafari_EventScript_GoldTeeth",
            "WayfarerFuchsiaSafari_EventScript_SurfAttendant",
            "WayfarerFuchsiaSafari_EventScript_Baoba",
        ):
            self.assertNotIn(label, beach_hns_events + house_hns_events)
        for flag in (
            "FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS",
            "FLAG_FUCHSIA_SAFARI_SURF_RECEIVED_HNS",
            "FLAG_FUCHSIA_SAFARI_STRENGTH_RECEIVED_HNS",
        ):
            self.assertNotIn(flag, beach_hns_events + house_hns_events)
        self.assertIn("WayfarerFuchsiaSafari_EventScript_GoldTeeth", beach_wayfarer_events)
        self.assertIn("WayfarerFuchsiaSafari_EventScript_SurfAttendant", beach_wayfarer_events)
        self.assertIn("WayfarerFuchsiaSafari_EventScript_Baoba", house_wayfarer_events)

        # Fuchsia City gains dialogue variants only; its map event inventory is
        # entirely HNS and remains free of Wayfarer event selection.
        for event_kind in ("object_events", "warp_events", "coord_events", "bg_events"):
            self.assertFalse(
                any(event.get("wayfarer_only") for event in self.city[event_kind]),
                event_kind,
            )
        self.assertIn(
            (23, 5, "FuchsiaCity_EventScript_SafariZone"),
            [(event["x"], event["y"], event["script"]) for event in self.city["bg_events"]],
        )

    def test_routes_avoid_fixed_actors_and_safari_level_trigger(self):
        # These are collision/elevation-valid legs from normal entry at (20, 38).
        # Roaming actors can temporarily occupy a tile at runtime, so the E2E
        # journey handles retry; static routing excludes all fixed actors.
        # This is the 34-step route used by the paid-admission E2E journey.
        surf_path = [
            (20, 38), (21, 38),
            *[(21, y) for y in range(37, 32, -1)],
            (22, 33), (22, 32), (22, 31), (21, 31), (21, 30),
            (20, 30), (19, 30),
            *[(19, y) for y in range(29, 23, -1)],
            *[(x, 24) for x in range(18, 3, -1)],
        ]
        teeth_path = [
            (20, 38), (21, 38), (22, 38), (23, 38), (24, 38),
            (24, 37), (24, 36), (24, 35), (24, 34), (25, 34),
            (25, 33), (25, 32), (26, 32), (26, 31), (26, 30),
            (27, 30), (27, 29), (28, 29), (29, 29), (30, 29),
            (30, 30), (30, 31), (31, 31), (32, 31), (33, 31),
            (34, 31), (35, 31), (36, 31), (37, 31), (38, 31),
            (39, 31), (39, 30), (39, 29), (39, 28), (39, 27),
            (39, 26), (39, 25), (39, 24),
        ]
        fixed_actors = {
            (event["x"], event["y"])
            for event in self.beach["object_events"]
            if not event.get("wayfarer_only")
            and (
                event["movement_type"] in {"MOVEMENT_TYPE_NONE", "MOVEMENT_TYPE_WALK_IN_PLACE_DOWN"}
                or (event["movement_type"] == "MOVEMENT_TYPE_WANDER_AROUND"
                    and event["movement_range_x"] == 0 and event["movement_range_y"] == 0)
            )
        }
        blocked = fixed_actors | {(1, 12)}
        self.assertEqual(fixed_actors, {(0, 12), (11, 18), (24, 19)})
        self.assertEqual(len(surf_path) - 1, 34)
        self.assertEqual(len(teeth_path) - 1, 37)
        self.assert_walkable_path("LAYOUT_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS", surf_path, blocked)
        self.assert_walkable_path("LAYOUT_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS", teeth_path, blocked)
        self.assertEqual(surf_path[-1], (4, 24))
        self.assertEqual(teeth_path[-1], (39, 24))
        for coordinate in ((4, 23), (39, 23)):
            _, collision, elevation = self.grid("LAYOUT_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS", *coordinate)
            self.assertEqual((collision, elevation), (0, 3), coordinate)

        house_path = [(4, 8), (5, 8), (6, 8), (7, 8), (8, 8), (8, 7), (8, 6), (8, 5), (8, 4), (8, 3)]
        self.assert_walkable_path(
            "LAYOUT_FUCHSIA_CITY_HOUSE2_HNS", house_path,
            {(4, 4), (3, 1), (4, 1), (8, 2)},
        )

    def test_flag_namespace_and_transaction_boundaries(self):
        source = (GAME_ROOT / "data/scripts/wayfarer_fuchsia_safari.inc").read_text()
        flags = (GAME_ROOT / "include/constants/flags_hns.h").read_text()
        self.assertEqual(
            {
                name: int(value, 16)
                for name, value in re.findall(
                    r"#define (FLAG_FUCHSIA_SAFARI_[A-Z_]+_HNS)\s+(0x[0-9A-F]+)", flags
                )
            },
            {
                "FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS": 0x4BC,
                "FLAG_FUCHSIA_SAFARI_SURF_RECEIVED_HNS": 0x4BD,
                "FLAG_FUCHSIA_SAFARI_STRENGTH_RECEIVED_HNS": 0x4BE,
            },
        )
        self.assertNotIn("FLAG_RECEIVED_HM_STRENGTH", source)
        self.assertNotIn("FLAG_GOT_HM03", source)
        self.assertNotIn("FLAG_GOT_HM04", source)

        # Compile the active Wayfarer namespace so these cannot degrade to
        # text-only aliases or collide with a source-map state allocation.
        assertion = """
#include \"global.h\"
#include \"constants/flags.h\"
_Static_assert(FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS == 0x4BC, \"Gold Teeth flag\");
_Static_assert(FLAG_FUCHSIA_SAFARI_SURF_RECEIVED_HNS == 0x4BD, \"Surf flag\");
_Static_assert(FLAG_FUCHSIA_SAFARI_STRENGTH_RECEIVED_HNS == 0x4BE, \"Strength flag\");
_Static_assert(FLAG_RECEIVED_HM_STRENGTH == 0x1F8, \"shared Strength state\");
_Static_assert(FLAG_GOT_HM03 == 0 && FLAG_GOT_HM04 == 0, \"source aliases remain zero\");
"""
        result = subprocess.run(
            ["gcc", "-std=c11", "-DPOKEMON_WAYFARER", "-Iinclude", "-x", "c", "-fsyntax-only", "-"],
            cwd=GAME_ROOT,
            text=True,
            input=assertion,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)

        gold = self.block(source, "WayfarerFuchsiaSafari_EventScript_GoldTeeth")
        self.assertLess(gold.index("checkitemspace ITEM_GOLD_TEETH"), gold.index("additem ITEM_GOLD_TEETH"))
        self.assertLess(gold.index("additem ITEM_GOLD_TEETH"), gold.index("checkitem ITEM_GOLD_TEETH", gold.index("additem ITEM_GOLD_TEETH")))
        self.assertLess(gold.index("msgreceiveditem"), gold.index("setflag FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS"))
        self.assertLess(gold.index("setflag FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS"), gold.index("removeobject VAR_LAST_TALKED"))
        self.assertNotIn("additem", self.block(source, "WayfarerFuchsiaSafari_EventScript_GoldTeethAlreadyOwned"))
        self.assertNotIn("setflag", self.block(source, "WayfarerFuchsiaSafari_EventScript_GoldTeethNoRoom"))

        surf = self.block(source, "WayfarerFuchsiaSafari_EventScript_SurfAttendant")
        self.assertLess(surf.index("additem ITEM_HM03"), surf.index("checkitem ITEM_HM03", surf.index("additem ITEM_HM03")))
        self.assertLess(surf.index("msgreceiveditem"), surf.index("setflag FLAG_FUCHSIA_SAFARI_SURF_RECEIVED_HNS"))
        self.assertIn("setflag FLAG_FUCHSIA_SAFARI_SURF_RECEIVED_HNS", self.block(source, "WayfarerFuchsiaSafari_EventScript_SurfAlreadyOwned"))
        self.assertNotIn("setflag", self.block(source, "WayfarerFuchsiaSafari_EventScript_SurfNoRoom"))
        self.assertNotIn("additem", self.block(source, "WayfarerFuchsiaSafari_EventScript_SurfComplete"))

        baoba = self.block(source, "WayfarerFuchsiaSafari_EventScript_Baoba")
        self.assertLess(baoba.index("checkitem ITEM_GOLD_TEETH"), baoba.index("checkitem ITEM_HM04"))
        self.assertLess(baoba.index("additem ITEM_HM04"), baoba.index("checkitem ITEM_HM04", baoba.index("additem ITEM_HM04")))
        commit = self.block(source, "WayfarerFuchsiaSafari_EventScript_BaobaCommit")
        self.assertLess(commit.index("removeitem ITEM_GOLD_TEETH"), commit.index("setflag FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS"))
        self.assertLess(commit.index("setflag FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS"), commit.index("setflag FLAG_FUCHSIA_SAFARI_STRENGTH_RECEIVED_HNS"))
        no_room = self.block(source, "WayfarerFuchsiaSafari_EventScript_BaobaNoRoom")
        self.assertNotIn("removeitem", no_room)
        self.assertNotIn("setflag", no_room)
        self.assertNotIn("additem", self.block(source, "WayfarerFuchsiaSafari_EventScript_BaobaComplete"))

    def test_standalone_hns_dialogue_and_admission_are_preserved(self):
        def preprocess(path, wayfarer):
            result = subprocess.run(
                ["cpp", "-traditional-cpp", "-P", f"-DIS_WAYFARER={int(wayfarer)}", "-DIS_HNS=1", str(path)],
                cwd=GAME_ROOT,
                text=True,
                capture_output=True,
                check=True,
            )
            return result.stdout

        city_hns = preprocess(MAPS / "FuchsiaCity_hns/scripts.inc", False)
        house_hns = preprocess(MAPS / "FuchsiaCity_House2_hns/scripts.inc", False)
        city_wayfarer = preprocess(MAPS / "FuchsiaCity_hns/scripts.inc", True)
        house_wayfarer = preprocess(MAPS / "FuchsiaCity_House2_hns/scripts.inc", True)
        self.assertIn("SAFARI ZONE OFFICE is closed until", city_hns)
        self.assertIn("The WARDEN is traveling abroad", city_hns)
        self.assertIn("took off overseas all by himself", house_hns)
        self.assertIn("DEVON SAFARI OFFICE", city_wayfarer)
        self.assertIn("WARDEN BAOBA is visiting", city_wayfarer)
        self.assertIn("He lost his GOLD TEETH", house_wayfarer)

        admission = (MAPS / "FuchsiaCity_SafariZoneEntrance_hns/scripts.inc").read_text()
        entry = self.block(admission, "FuchsiaCity_SafariZoneEntrance_EventScript_TryEnterSafariZone")
        for command in (
            "checkitem ITEM_POKEBLOCK_CASE", "checkmoney 500", "removemoney 500",
            "special EnterSafariMode", "setvar VAR_SAFARI_ZONE_STATE, 2",
            "warp MAP_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS, 1",
        ):
            self.assertIn(command, entry)
        self.assertNotIn("WayfarerFuchsiaSafari", admission)

    def test_blockdata_is_unchanged(self):
        # This feature adds schema-valid events only; these hashes make any
        # later direct map binary modification an explicit review decision.
        self.assertEqual(
            hashlib.sha256(
                (GAME_ROOT / "data/layouts/FuchsiaCity_SafariZoneBeach_hns/map.bin").read_bytes()
            ).hexdigest(),
            "66e4453d3106450bdf8bb22894f6cc8e9a9e5422d21d94248c96864acad706c9",
        )
        self.assertEqual(
            hashlib.sha256(
                (GAME_ROOT / "data/layouts/FuchsiaCity_House2_hns/map.bin").read_bytes()
            ).hexdigest(),
            "80891ece188a189ae2aa4366d2650a52e7fe7896dfe25f5876caab7b3df5b2df",
        )


if __name__ == "__main__":
    unittest.main()
