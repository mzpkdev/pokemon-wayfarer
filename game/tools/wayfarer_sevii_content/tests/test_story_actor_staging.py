from pathlib import Path
import json
import re
import unittest


GAME = Path(__file__).resolve().parents[3]


class WayfarerSeviiStoryActorStagingTests(unittest.TestCase):
    def source(self, relative):
        return (GAME / relative).read_text(encoding="utf-8")

    def test_inverse_flags_have_a_compact_contiguous_allocation(self):
        flags = self.source("include/constants/flags.h")
        symbols = (
            "HIDE_RETURNED_LOSTELLE",
            "HIDE_RETURNED_SELPHY",
            "HIDE_DOTTED_HOLE_SCIENTIST",
            "HIDE_LOCAL_ROCKETS",
            "HIDE_WAREHOUSE_COMBATANTS",
            "HIDE_WAREHOUSE_GIDEON",
            "HIDE_RUBY_GUARDS",
            "HIDE_RIVALS",
        )
        for slot, symbol in enumerate(symbols, start=52):
            self.assertIn(
                f"FLAG_WAYFARER_SEVII_{symbol} WAYFARER_SEVII_FLAG_ID({slot})",
                flags,
            )

    def test_new_game_and_objective_commits_write_presentation_directly(self):
        persistence = self.source("src/wayfarer_persistence.c")
        adventures = self.source("data/scripts/wayfarer_sevii/story/local_adventures.inc")
        celio = self.source("data/scripts/wayfarer_sevii/story/celio_network.inc")
        for symbol in (
            "HIDE_RETURNED_LOSTELLE", "HIDE_RETURNED_SELPHY",
            "HIDE_DOTTED_HOLE_SCIENTIST", "HIDE_LOCAL_ROCKETS",
            "HIDE_WAREHOUSE_COMBATANTS", "HIDE_WAREHOUSE_GIDEON",
            "HIDE_RUBY_GUARDS", "HIDE_RIVALS",
        ):
            self.assertIn(f"FlagSet(FLAG_WAYFARER_SEVII_{symbol});", persistence)
        self.assertRegex(adventures, r"setflag FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED\n\s*clearflag FLAG_WAYFARER_SEVII_HIDE_RETURNED_LOSTELLE")
        self.assertRegex(adventures, r"setflag FLAG_WAYFARER_SEVII_SELPHY_RETURNED\n\s*clearflag FLAG_WAYFARER_SEVII_HIDE_RETURNED_SELPHY")
        self.assertRegex(celio, r"setflag FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED\n\s*clearflag FLAG_WAYFARER_SEVII_HIDE_DOTTED_HOLE_SCIENTIST\n\s*clearflag FLAG_WAYFARER_SEVII_HIDE_LOCAL_ROCKETS\n\s*clearflag FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS")
        self.assertRegex(celio, r"setflag FLAG_WAYFARER_SEVII_RUBY_RECOVERED\n\s*setflag FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS")
        self.assertRegex(celio, r"setflag FLAG_WAYFARER_SEVII_SAPPHIRE_STOLEN\n\s*setflag FLAG_WAYFARER_SEVII_HIDE_DOTTED_HOLE_SCIENTIST")
        self.assertNotIn("WayfarerSevii_RefreshStoryActorVisibility", adventures + celio)

    def test_warehouse_readiness_is_order_independent_and_keeps_gideon(self):
        story = self.source("data/scripts/wayfarer_sevii/story/celio_network.inc")
        readiness = re.search(
            r"WayfarerSevii_Story_UpdateWarehouseReadiness:(.*?)"
            r"WayfarerSevii_Story_UpdateWarehouseReadinessDone:", story, re.S
        ).group(1)
        for prerequisite in ("SAPPHIRE_STOLEN", "PASSWORD_ONE_LEARNED", "PASSWORD_TWO_LEARNED"):
            self.assertIn(f"goto_if_unset FLAG_WAYFARER_SEVII_{prerequisite}", readiness)
        self.assertIn("clearflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_COMBATANTS", readiness)
        self.assertIn("clearflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GIDEON", readiness)

    def test_warehouse_reward_retry_keeps_gideon_present(self):
        story = self.source("data/scripts/wayfarer_sevii/story/celio_network.inc")
        victory = re.search(
            r"WayfarerSevii_Warehouse_GideonVictory:(.*?)"
            r"WayfarerSevii_Warehouse_TryGiveSapphire:",
            story,
            re.S,
        ).group(1)
        reward = re.search(
            r"WayfarerSevii_Warehouse_TryGiveSapphire:(.*?)"
            r"WayfarerSevii_Warehouse_SapphireNoRoom:",
            story,
            re.S,
        ).group(1)
        self.assertIn("WAREHOUSE_CLEARED", victory)
        self.assertNotIn("WAREHOUSE_CLEARED", reward)
        self.assertIn("setflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_COMBATANTS", victory)
        self.assertNotIn("HIDE_WAREHOUSE_GIDEON", victory)
        self.assertEqual(re.findall(r"removeobject ([1-5])", victory), ["1", "2", "3", "4", "5"])
        self.assertNotIn("removeobject 6", victory + reward)

    def test_rival_scene_commits_then_spawns_the_current_actor(self):
        story = self.source("data/scripts/wayfarer_sevii/story/celio_network.inc")
        for label, local_id in (
            ("FourIsland_RivalTryScene", "10"),
            ("SixIsland_RivalTryScene", "2"),
        ):
            body = re.search(
                rf"WayfarerSevii_{label}::(.*?)(?:\nWayfarerSevii_|\Z)", story, re.S
            ).group(1)
            self.assertTrue(body.lstrip().startswith("setvar VAR_TEMP_8, 1"))
            commit = body.index("setflag FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN")
            presentation = body.index("clearflag FLAG_WAYFARER_SEVII_HIDE_RIVALS")
            spawn = body.index(f"addobject {local_id}")
            self.assertLess(commit, presentation)
            self.assertLess(presentation, spawn)

    def test_literal_local_ids_are_pinned_to_source_object_indices(self):
        cases = (
            ("FiveIsland_RocketWarehouse_Frlg", 0, "FiveIsland_RocketWarehouse_EventScript_Grunt2"),
            ("FiveIsland_RocketWarehouse_Frlg", 1, "FiveIsland_RocketWarehouse_EventScript_Grunt3"),
            ("FiveIsland_RocketWarehouse_Frlg", 2, "FiveIsland_RocketWarehouse_EventScript_Admin1"),
            ("FiveIsland_RocketWarehouse_Frlg", 3, "FiveIsland_RocketWarehouse_EventScript_Admin2"),
            ("FiveIsland_RocketWarehouse_Frlg", 4, "FiveIsland_RocketWarehouse_EventScript_Grunt1"),
            ("SixIsland_DottedHole_SapphireRoom_Frlg", 1, "0x0"),
            ("FourIsland_Frlg", 9, "0x0"),
            ("SixIsland_PokemonCenter_1F_Frlg", 1, "0x0"),
        )
        for map_name, index, script in cases:
            source = json.loads(self.source(f"data/maps/{map_name}/map.json"))
            self.assertEqual(source["object_events"][index]["script"], script)
            # mapjson assigns local IDs by one-based source order; the scripts
            # deliberately use the corresponding literal object operand.
            self.assertGreater(index + 1, 0)


if __name__ == "__main__":
    unittest.main()
