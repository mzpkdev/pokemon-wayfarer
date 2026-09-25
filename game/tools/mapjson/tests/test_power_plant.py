"""Source contract for Wayfarer's imported Power Plant generating hall.

These checks inspect selected data and scripts; collision and battle outcomes
still require the emulator journey in the implementation specification.
"""

import json
from pathlib import Path
import re
import unittest


GAME_ROOT = Path(__file__).resolve().parents[3]


def read_json(relative_path):
    return json.loads((GAME_ROOT / relative_path).read_text())


def read_text(relative_path):
    return (GAME_ROOT / relative_path).read_text()


def script_block(source, label):
    match = re.search(
        rf"^{re.escape(label)}::\s*\n(.*?)(?=^\w+::|\Z)",
        source,
        re.MULTILINE | re.DOTALL,
    )
    if match is None:
        raise AssertionError(f"Missing script label: {label}")
    return match.group(1)


class PowerPlantSourceTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.hall = read_json("data/maps/PowerPlant_Frlg/map.json")
        cls.exterior = read_json("data/maps/Route10_hns/map.json")
        cls.lobby = read_json("data/maps/Route10_PowerPlantEntrance_hns/map.json")
        cls.back_room = read_json("data/maps/Route10_PowerPlantBackRoom_hns/map.json")
        cls.flags = read_text("include/constants/flags.h")
        cls.generator = read_text("tools/mapjson/mapjson.cpp")
        cls.hall_scripts = read_text("data/scripts/wayfarer_power_plant.inc")

    def test_hall_and_layout_are_selected_for_wayfarer(self):
        layouts = {
            entry["id"]: entry
            for entry in read_json("data/layouts/layouts.json")["layouts"]
        }
        groups = read_json("data/maps/map_groups.json")
        layout = layouts[self.hall["layout"]]
        self.assertEqual(self.hall["id"], "MAP_POWER_PLANT")
        self.assertIn(self.hall["name"], groups["gMapGroup_Dungeons_Frlg"])
        self.assertTrue(self.hall["wayfarer_include"])
        self.assertTrue(layout["wayfarer_include"])
        self.assertEqual((layout["width"], layout["height"]), (49, 40))
        self.assertEqual(layout["blockdata_filepath"],
                         "data/layouts/PowerPlant_Frlg/map.bin")
        self.assertTrue(self.hall["allow_escaping"])
        event_scripts = read_text("data/event_scripts.s")
        wayfarer_blocks = re.findall(
            r"^#if IS_WAYFARER\s*\n(.*?)^#endif", event_scripts,
            re.MULTILINE | re.DOTALL,
        )
        hall_blocks = [block for block in wayfarer_blocks
                       if '"data/scripts/wayfarer_power_plant.inc"' in block]
        self.assertEqual(len(hall_blocks), 1)
        self.assertNotIn('"data/maps/PowerPlant_Frlg/scripts.inc"', hall_blocks[0])

    def test_every_source_exit_returns_to_the_hns_lobby(self):
        warps = self.hall["warp_events"]
        self.assertEqual(len(warps), 5)
        self.assertEqual({(warp["x"], warp["y"]) for warp in warps},
                         {(4, 39), (5, 38), (6, 39), (1, 11), (0, 13)})
        self.assertEqual(
            {(warp["wayfarer_dest_map"], warp["wayfarer_dest_warp_id"])
             for warp in warps},
            {("MAP_ROUTE10_POWER_PLANT_ENTRANCE_HNS", "0")},
        )
        self.assertEqual(
            {(warp.get("wayfarer_x", warp["x"]),
              warp.get("wayfarer_y", warp["y"])) for warp in warps},
            {(4, 37), (5, 37), (6, 37), (1, 11), (1, 13)},
        )
        exit_triggers = [
            event for event in self.hall["coord_events"]
            if event["script"] == "PowerPlant_EventScript_ExitToLobby"
        ]
        self.assertEqual(
            {(event["x"], event["y"]) for event in exit_triggers},
            {(4, 37), (5, 37), (6, 37), (1, 11), (1, 13)},
        )
        self.assertTrue(all(event["wayfarer_only"] for event in exit_triggers))
        exit_script = script_block(
            self.hall_scripts, "PowerPlant_EventScript_ExitToLobby")
        self.assertIn("warp MAP_ROUTE10_POWER_PLANT_ENTRANCE_HNS, 4, 20",
                      exit_script)
        self.assertEqual(
            (self.lobby["warp_events"][0]["dest_map"],
             self.lobby["warp_events"][1]["dest_map"]),
            ("MAP_ROUTE10_HNS", "MAP_ROUTE10_POWER_PLANT_BACK_ROOM_HNS"),
        )
        self.assertEqual(self.back_room["warp_events"][0]["dest_map"],
                         "MAP_ROUTE10_POWER_PLANT_ENTRANCE_HNS")
        self.assertEqual(self.back_room["warp_events"][1]["dest_map"],
                         "MAP_ROUTE10_HNS")
        self.assertEqual(self.back_room["object_events"][0]["flag"],
                         "FLAG_HIDE_POWER_PLANT_ENGINEER")

    def test_source_content_has_distinct_wayfarer_state(self):
        objects = self.hall["object_events"]
        pickups = [event for event in objects
                   if event["script"].startswith("PowerPlant_EventScript_Item")]
        electrodes = [event for event in objects
                      if "Electrode" in event["script"]]
        hidden = self.hall["bg_events"]
        self.assertEqual(len(objects), 8)
        self.assertEqual(len(pickups), 5)
        self.assertEqual(len(electrodes), 2)
        self.assertEqual(len(hidden), 2)
        self.assertEqual({event["item"] for event in hidden},
                         {"ITEM_MAX_ELIXIR", "ITEM_THUNDER_STONE"})
        zapdos = next(event for event in objects
                      if event["script"] == "PowerPlant_EventScript_Zapdos")
        self.assertEqual(zapdos["wayfarer_graphics_id"],
                         "OBJ_EVENT_GFX_MON_BASE+SPECIES_ZAPDOS")

        expected_remap = {
            "FLAG_HIDE_POWER_PLANT_MAX_POTION": "FLAG_WAYFARER_POWER_PLANT_MAX_POTION",
            "FLAG_HIDE_POWER_PLANT_TM17": "FLAG_WAYFARER_POWER_PLANT_TM_PROTECT",
            "FLAG_HIDE_POWER_PLANT_TM25": "FLAG_WAYFARER_POWER_PLANT_TM_THUNDER",
            "FLAG_HIDE_POWER_PLANT_THUNDER_STONE": "FLAG_WAYFARER_POWER_PLANT_THUNDER_STONE",
            "FLAG_HIDE_POWER_PLANT_ELIXIR": "FLAG_WAYFARER_POWER_PLANT_ELIXIR",
            "FLAG_HIDE_POWER_PLANT_ELECTRODE_1": "FLAG_WAYFARER_POWER_PLANT_ELECTRODE_1",
            "FLAG_HIDE_POWER_PLANT_ELECTRODE_2": "FLAG_WAYFARER_POWER_PLANT_ELECTRODE_2",
            "FLAG_HIDE_ZAPDOS": "FLAG_WAYFARER_POWER_PLANT_HIDE_ZAPDOS",
            "FLAG_HIDDEN_ITEM_POWER_PLANT_MAX_ELIXIR": "FLAG_WAYFARER_POWER_PLANT_HIDDEN_MAX_ELIXIR",
            "FLAG_HIDDEN_ITEM_POWER_PLANT_THUNDER_STONE": "FLAG_WAYFARER_POWER_PLANT_HIDDEN_THUNDER_STONE",
        }
        self.assertEqual(
            {event["flag"] for event in objects + hidden},
            set(expected_remap),
        )
        remap_source = self.generator.split('if (name == "PowerPlant_Frlg") {', 1)[1]
        remap_source = remap_source.split("if (!wayfarer_coast_map_names.count(name))", 1)[0]
        actual_remap = dict(re.findall(r'\{\s*"(FLAG_\w+)"\s*,\s*"(FLAG_\w+)"\s*\}',
                                       remap_source))
        self.assertEqual(actual_remap, expected_remap)

        coast_slots = {}
        raw_slots = {}
        for name in expected_remap.values():
            match = re.search(rf"^#define\s+{name}\s+WAYFARER_COAST_FLAG_ID\((\d+)\)",
                              self.flags, re.MULTILINE)
            if match:
                coast_slots[name] = int(match.group(1))
            else:
                hidden_match = re.search(rf"^#define\s+{name}\s+(0x[0-9A-Fa-f]+)",
                                         self.flags, re.MULTILINE)
                self.assertIsNotNone(hidden_match, name)
                raw_slots[name] = int(hidden_match.group(1), 16)
        self.assertEqual(len(coast_slots), 8)
        self.assertEqual(len(set(coast_slots.values())), len(coast_slots))
        self.assertTrue(all(slot > 0 for slot in coast_slots.values()))
        self.assertEqual(set(raw_slots.values()), {0x95B, 0x95C})
        self.assertTrue(all(0x91A < slot < 0x960 for slot in raw_slots.values()))
        active_hns_flags = {
            int(value, 16)
            for name, value in re.findall(
                r"^#define\s+(FLAG_(?!UNUSED_)\w+_HNS)\s+(0x[0-9A-Fa-f]+)",
                read_text("include/constants/flags_hns.h"),
                re.MULTILINE,
            )
        }
        self.assertTrue(set(raw_slots.values()).isdisjoint(active_hns_flags))

    def test_semantic_tm_items_and_single_indoor_zapdos(self):
        item_scripts = read_text("data/scripts/item_ball_scripts_wayfarer.inc")
        expected_items = {
            "PowerPlant_EventScript_ItemMaxPotion": "ITEM_MAX_POTION",
            "PowerPlant_EventScript_ItemTM17": "ITEM_TM_PROTECT",
            "PowerPlant_EventScript_ItemTM25": "ITEM_TM_THUNDER",
            "PowerPlant_EventScript_ItemThunderStone": "ITEM_THUNDER_STONE",
            "PowerPlant_EventScript_ItemElixir": "ITEM_ELIXIR",
        }
        for label, item in expected_items.items():
            self.assertIn(f"finditem {item}", script_block(item_scripts, label))
        self.assertEqual(
            {event["script"] for event in self.hall["object_events"]
             if event["script"] in expected_items},
            set(expected_items),
        )

        outdoor = [event for event in self.exterior["object_events"]
                   if event["script"] == "Route10_EventScript_Zapdos"]
        indoor = [event for event in self.hall["object_events"]
                  if event["script"] == "PowerPlant_EventScript_Zapdos"]
        self.assertEqual(len(outdoor), 1)
        self.assertTrue(outdoor[0]["wayfarer_exclude"])
        self.assertEqual(len(indoor), 1)
        self.assertNotIn("wayfarer_exclude", indoor[0])
        self.assertEqual((indoor[0]["x"], indoor[0]["y"]), (5, 11))
        visibility = re.search(
            r"^#define\s+FLAG_WAYFARER_POWER_PLANT_HIDE_ZAPDOS\s+"
            r"WAYFARER_COAST_FLAG_ID\((\d+)\)", self.flags, re.MULTILINE,
        )
        resolved = re.search(
            r"^#define\s+FLAG_WAYFARER_POWER_PLANT_ZAPDOS_RESOLVED\s+"
            r"WAYFARER_COAST_FLAG_ID\((\d+)\)", self.flags, re.MULTILINE,
        )
        self.assertIsNotNone(visibility)
        self.assertIsNotNone(resolved)
        self.assertGreater(int(resolved.group(1)), 0)
        self.assertNotEqual(visibility.group(1), resolved.group(1))
        self.assertNotIn("FLAG_WAYFARER_POWER_PLANT_ZAPDOS_RESOLVED",
                         read_text("data/maps/PokemonLeague_HallOfFame_hns/scripts.inc"))
        self.assertNotIn("FLAG_WAYFARER_POWER_PLANT_HIDE_ZAPDOS",
                         read_text("data/maps/PokemonLeague_HallOfFame_hns/scripts.inc"))
        self.assertIn("setflag FLAG_WAYFARER_POWER_PLANT_ZAPDOS_RESOLVED",
                      script_block(self.hall_scripts, "PowerPlant_EventScript_ResolveZapdos"))
        self.assertNotIn("FLAG_WAYFARER_POWER_PLANT_ZAPDOS_RESOLVED",
                         script_block(self.hall_scripts,
                                      "PowerPlant_EventScript_ZapdosFlewAway"))

    def test_worker_handoff_and_map_encounters_are_registered(self):
        worker = read_text("data/maps/Route10_PowerPlantEntrance_hns/scripts.inc")
        before = script_block(worker, "Route10_PowerPlantEntrance_EventScript_Engineer2")
        after = script_block(worker, "Route10_PowerPlantEntrance_EventScript_Engineer2Done")
        offer = script_block(worker, "Route10_PowerPlantEntrance_EventScript_Engineer2OfferOldHall")
        decline = script_block(worker, "Route10_PowerPlantEntrance_EventScript_Engineer2DeclineOldHall")
        self.assertIn("Engineer2PowerPlantUpAndRunning", before)
        self.assertIn("Engineer2GeneratorIsRunningAgain", after)
        self.assertIn("MSGBOX_YESNO", offer)
        self.assertIn("warp MAP_POWER_PLANT", offer)
        self.assertIn("warp MAP_POWER_PLANT, 5, 36", offer)
        self.assertIn("release", decline)
        for block in (before, after, offer, decline):
            self.assertNotRegex(block, r"\b(?:setflag|clearflag|setvar)\b")
        self.assertIn(
            "ingame_trade INGAME_TRADE_MAGNETON",
            script_block(worker, "Route10_PowerPlantEntrance_EventScript_Lorenzo"),
        )
        trades = read_text("src/data/trade.h")
        magneton_trade = trades.split("[INGAME_TRADE_MAGNETON] =", 1)[1]
        magneton_trade = magneton_trade.split("[INGAME_TRADE_", 1)[0]
        self.assertIn(".species = SPECIES_MAGNETON", magneton_trade)
        self.assertIn(".requestedSpecies = SPECIES_DUGTRIO", magneton_trade)

        groups = read_json("src/data/wild_encounters.json")["wild_encounter_groups"]
        map_group = next(group for group in groups if group["label"] == "gWildMonHeaders")
        self.assertTrue(map_group["for_maps"])
        profiles = [entry for entry in map_group["encounters"]
                    if entry.get("map") == "MAP_POWER_PLANT"
                    and entry.get("base_label", "").startswith("sPowerPlant_Wayfarer_")]
        self.assertEqual({entry["base_label"] for entry in profiles},
                         {"sPowerPlant_Wayfarer_Day", "sPowerPlant_Wayfarer_Night"})
        encounter_generator = read_text(
            "tools/wild_encounters/wild_encounters_to_header.py")
        self.assertIn('WAYFARER_REPLACED_FRLG_WILD_MAPS = {\n    "MAP_POWER_PLANT",',
                      encounter_generator)
        self.assertIn("map_name in WAYFARER_REPLACED_FRLG_WILD_MAPS",
                      encounter_generator)
        for profile in profiles:
            self.assertEqual(profile["land_mons"]["encounter_rate"], 7)
            self.assertEqual(len(profile["land_mons"]["mons"]), 12)
            self.assertIn("SPECIES_ELECTABUZZ",
                          {mon["species"] for mon in profile["land_mons"]["mons"]})
        self.assertFalse(any(
            entry.get("base_label", "").startswith("sPowerPlant_Wayfarer_")
            for group in groups if not group["for_maps"]
            for entry in group["encounters"]
        ))


if __name__ == "__main__":
    unittest.main()
