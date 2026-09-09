"""Silph liberation keeps the FRLG floors isolated behind Wayfarer selection."""

import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest


GAME_ROOT = Path(__file__).resolve().parents[3]
SILPH_FLOORS = (
    ("2F", 36, 22), ("3F", 36, 22), ("4F", 36, 22), ("5F", 36, 22),
    ("6F", 31, 19), ("7F", 31, 19), ("8F", 31, 19), ("9F", 31, 19),
    ("10F", 17, 19), ("11F", 17, 20), ("Elevator", 5, 7),
)
SILPH_NAMES = tuple(f"SilphCo_{floor}_Frlg" for floor, _, _ in SILPH_FLOORS)
SILPH_LAYOUT_IDS = tuple(f"LAYOUT_SILPH_CO_{floor.upper()}" for floor, _, _ in SILPH_FLOORS)
GRAPHICS_OVERRIDES = {
    "OBJ_EVENT_GFX_SCIENTIST": "OBJ_EVENT_GFX_SCIENTIST_M_HNS",
    "OBJ_EVENT_GFX_WORKER_F": "OBJ_EVENT_GFX_WORKER_F_HNS",
    "OBJ_EVENT_GFX_ROCKER": "OBJ_EVENT_GFX_ROCKER_HNS",
    "OBJ_EVENT_GFX_OLD_MAN_2": "OBJ_EVENT_GFX_OLD_MAN_2_HNS",
}
SOURCE_BLOCK_HASHES = {
    "SilphCo_2F_Frlg": "652233e3c23fa74e45a09eebf9b6c52c74e3c7ab1d072686abdc4938a7c6fb37",
    "SilphCo_3F_Frlg": "6333b7e8ab0dee8a4690c04f941ef771d84616b245cf5c00cba4311ec18d7bf8",
    "SilphCo_4F_Frlg": "ce87635bbebcfdaa3251997be24f1f4ea649fb3ac01b84cfd97d7fe63406583f",
    "SilphCo_5F_Frlg": "c80d185f3c432dd9323d40b491bb1866c56bbd5b3f554e0f3fce0faa6c501939",
    "SilphCo_6F_Frlg": "5f40589bb71226816c1046176f4988b2435ccaab3e1f2f7537d01224a45d3763",
    "SilphCo_7F_Frlg": "f597b3db24f8f7a611ca8f750cbfaa17ea51fd839d587cbb90c9ef7ca02b72f6",
    "SilphCo_8F_Frlg": "5a2e82d8a8f0e70c6d99e110238066c1b1b39e7c33fac06a88d7b73ef7b81dfb",
    "SilphCo_9F_Frlg": "d12558d3563a58fd630f87914ecb58bcb2213e7515f42c7b354b2f439f16d040",
    "SilphCo_10F_Frlg": "bfe4521cb1b892d4383612c942b36ee6d61b842e41dcafde170719895702cd0c",
    "SilphCo_11F_Frlg": "5f0783f5f2acc9e6a89a93681ab9c7d50d6708fae7e1931f51399d760e9876a1",
    "SilphCo_Elevator_Frlg": "84557c1c0d7fec4344fd62a18b9c32fcdaab83cc8f63416012089328dc5344cd",
    "SaffronCity_SilphCo_hns": "4f36d5df3016c31d056482d81d60892c1ee775a38321c532d70f3fe70dedf4cc",
}


class SilphLiberationMapTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.root = Path(cls.temp.name)
        cls.tool = cls.root / "mapjson"
        subprocess.run(
            ["g++", "-std=c++17",
             str(GAME_ROOT / "tools/mapjson/mapjson.cpp"),
             str(GAME_ROOT / "tools/mapjson/json11.cpp"),
             "-o", str(cls.tool)],
            check=True,
        )
        cls.layouts = {
            layout["id"]: layout
            for layout in json.loads(
                (GAME_ROOT / "data/layouts/layouts.json").read_text()
            )["layouts"]
        }
        cls.maps = {
            name: json.loads((GAME_ROOT / f"data/maps/{name}/map.json").read_text())
            for name in SILPH_NAMES
        }

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def generated(self, name, version):
        output = self.root / f"{name}-{version}"
        output.mkdir(exist_ok=True)
        subprocess.run(
            [str(self.tool), "map", version, f"data/maps/{name}/map.json",
             "data/layouts/layouts.json", str(output)],
            cwd=GAME_ROOT,
            check=True,
            capture_output=True,
        )
        return (output / "events.inc").read_text()

    def grid(self, layout_id, x, y):
        layout = self.layouts[layout_id]
        self.assertTrue(0 <= x < layout["width"] and 0 <= y < layout["height"])
        data = (GAME_ROOT / layout["blockdata_filepath"]).read_bytes()
        value = struct.unpack_from("<H", data, 2 * (y * layout["width"] + x))[0]
        return value & 0x3FF, (value >> 10) & 3, value >> 12

    def test_catalog_keeps_exact_source_group_and_layout_registration(self):
        groups = json.loads((GAME_ROOT / "data/maps/map_groups.json").read_text())
        self.assertEqual(groups["group_order"][66], "gMapGroup_Dungeons_Frlg")
        self.assertEqual(groups["gMapGroup_Dungeons_Frlg"][48:59], list(SILPH_NAMES))
        self.assertEqual(groups["gMapGroup_Dungeons_Frlg"][47], "SilphCo_1F_Frlg")

        self.assertEqual(
            {name for name, data in self.maps.items()
             if data.get("wayfarer_include") is True},
            set(SILPH_NAMES),
        )
        for (floor, width, height), name, layout_id in zip(SILPH_FLOORS, SILPH_NAMES, SILPH_LAYOUT_IDS):
            data = self.maps[name]
            layout = self.layouts[layout_id]
            self.assertEqual(data["game_version"], "frlg")
            self.assertEqual(data["layout"], layout_id)
            self.assertEqual((layout["width"], layout["height"]), (width, height), floor)
            self.assertEqual(layout["game_version"], "frlg")
            self.assertEqual(layout["layout_version"], "frlg")
            self.assertTrue(layout["wayfarer_include"])
        self.assertNotIn("wayfarer_include", self.layouts["LAYOUT_SILPH_CO_1F"])

        for name, digest in SOURCE_BLOCK_HASHES.items():
            self.assertEqual(
                hashlib.sha256(
                    (GAME_ROOT / f"data/layouts/{name}/map.bin").read_bytes()
                ).hexdigest(),
                digest,
                name,
            )

    def test_generator_preserves_source_events_and_emits_wayfarer_overrides(self):
        source_events = {}
        wayfarer_events = {}
        for name in SILPH_NAMES:
            source_events[name] = self.generated(name, "firered")
            wayfarer_events[name] = self.generated(name, "wayfarer")

        source_occupations = sum(
            text.count("FLAG_HIDE_SILPH_ROCKETS") for text in source_events.values()
        )
        wayfarer_occupations = sum(
            text.count("FLAG_SILPH_LIBERATED_HNS") for text in wayfarer_events.values()
        )
        self.assertEqual(source_occupations, 31)
        self.assertEqual(wayfarer_occupations, 31)
        self.assertTrue(all(
            "FLAG_SILPH_LIBERATED_HNS" not in text for text in source_events.values()
        ))

        for source, override in GRAPHICS_OVERRIDES.items():
            self.assertGreater(
                sum(text.count(override) for text in wayfarer_events.values()), 0,
                override,
            )
            self.assertEqual(
                sum(text.count(override) for text in source_events.values()), 0,
                override,
            )
            self.assertGreater(
                sum(text.count(source) for text in source_events.values()), 0,
                source,
            )

        self.assertIn(
            "warp_def 30, 2, 3, 3, MAP_SILPH_CO_1F",
            source_events["SilphCo_2F_Frlg"],
        )
        self.assertIn(
            "warp_def 30, 2, 3, 1, MAP_SAFFRON_CITY_SILPH_CO_HNS",
            wayfarer_events["SilphCo_2F_Frlg"],
        )
        self.assertIn("VAR_MAP_SCENE_SILPH_CO_11F", source_events["SilphCo_11F_Frlg"])
        self.assertNotIn("VAR_MAP_SCENE_SILPH_CO_11F", wayfarer_events["SilphCo_11F_Frlg"])
        self.assertEqual(
            wayfarer_events["SilphCo_11F_Frlg"].count("VAR_SILPH_GIOVANNI_SCENE_HNS"),
            2,
        )

    def test_7f_excludes_blue_and_preserves_the_rest_of_the_floor(self):
        source = self.generated("SilphCo_7F_Frlg", "firered")
        wayfarer = self.generated("SilphCo_7F_Frlg", "wayfarer")
        source_data = self.maps["SilphCo_7F_Frlg"]

        blue = source_data["object_events"][0]
        self.assertEqual((blue["graphics_id"], blue["x"], blue["y"]),
                         ("OBJ_EVENT_GFX_BLUE", 2, 6))
        self.assertTrue(blue["wayfarer_exclude"])
        self.assertEqual(
            {(event["x"], event["y"]) for event in source_data["coord_events"]
             if event.get("wayfarer_exclude") is True},
            {(2, 4), (2, 5)},
        )
        self.assertIn("OBJ_EVENT_GFX_BLUE", source)
        self.assertNotIn("OBJ_EVENT_GFX_BLUE", wayfarer)
        self.assertIn("RivalTrigger", source)
        self.assertNotIn("RivalTrigger", wayfarer)
        self.assertEqual(source.count("\tcoord_event "), 2)
        self.assertEqual(wayfarer.count("\tcoord_event "), 0)
        self.assertIn("SilphCo_7F_EventScript_LaprasGuy", wayfarer)
        self.assertIn("SilphCo_7F_EventScript_Joshua", wayfarer)

    def test_selected_map_entry_labels_are_present_in_the_combined_wayfarer_unit(self):
        source = (
            (GAME_ROOT / "data/scripts/wayfarer_silph.inc").read_text()
            + (GAME_ROOT / "data/scripts/silphco_doors.inc").read_text()
        )
        labels = set()
        for name, data in self.maps.items():
            labels.add(f"{name}_MapScripts")
            for event_type in ("object_events", "coord_events", "bg_events"):
                for event in data[event_type]:
                    if event.get("wayfarer_exclude") is True:
                        continue
                    label = event.get("script")
                    if label and label not in {"0x0", "NULL"}:
                        labels.add(label)
        self.assertEqual(
            {label for label in labels if f"{label}::" not in source},
            set(),
        )

    def test_hns_lobby_access_is_wayfarer_only_and_uses_safe_arrival(self):
        lobby_name = "SaffronCity_SilphCo_hns"
        lobby = json.loads((GAME_ROOT / f"data/maps/{lobby_name}/map.json").read_text())
        wayfarer = self.generated(lobby_name, "wayfarer")
        hns = self.generated(lobby_name, "hns")

        self.assertEqual(lobby["warp_events"][0], {
            "x": 8, "y": 20, "elevation": 0,
            "dest_map": "MAP_SAFFRON_CITY_HNS", "dest_warp_id": "11",
        })
        arrival = lobby["warp_events"][1]
        self.assertEqual(
            (arrival["x"], arrival["y"], arrival["elevation"],
             arrival["dest_map"], arrival["dest_warp_id"]),
            (31, 3, 3, "MAP_SILPH_CO_2F", "3"),
        )
        self.assertTrue(arrival["wayfarer_only"])
        elevator = lobby["bg_events"][-1]
        self.assertEqual(
            (elevator["x"], elevator["y"], elevator["elevation"],
             elevator["player_facing_dir"], elevator["script"]),
            (22, 3, 3, "BG_EVENT_PLAYER_FACING_NORTH",
             "WayfarerSilph_EventScript_ElevatorEntry"),
        )
        self.assertTrue(elevator["wayfarer_only"])
        self.assertIn("warp_def 31, 3, 3, 3, MAP_SILPH_CO_2F", wayfarer)
        self.assertIn("WayfarerSilph_EventScript_ElevatorEntry", wayfarer)
        self.assertNotIn("warp_def 31, 3", hns)
        self.assertNotIn("WayfarerSilph_EventScript_ElevatorEntry", hns)

        for coordinate in ((22, 4), (30, 2), (31, 3)):
            self.assertEqual(self.grid("LAYOUT_SAFFRON_CITY_SILPH_CO_HNS", *coordinate)[1:],
                             (0, 3), coordinate)
        self.assertEqual(self.grid("LAYOUT_SAFFRON_CITY_SILPH_CO_HNS", 22, 3)[1], 1)
        generated_warps = [line for line in wayfarer.splitlines() if line.startswith("\twarp_def ")]
        self.assertFalse(any(line.startswith("\twarp_def 22, 3,") for line in generated_warps))
        self.assertFalse(any(line.startswith("\twarp_def 31, 2,") for line in generated_warps))


if __name__ == "__main__":
    unittest.main()
