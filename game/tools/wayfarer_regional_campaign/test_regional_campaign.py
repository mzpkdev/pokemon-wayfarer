import re
import unittest
from pathlib import Path


GAME = Path(__file__).resolve().parents[2]


class WayfarerRegionalCampaignTests(unittest.TestCase):
    def test_each_hoenn_gym_awards_and_checks_its_source_badge(self):
        gyms = (
            ("RustboroCity_Gym", 1),
            ("DewfordTown_Gym", 2),
            ("MauvilleCity_Gym", 3),
            ("LavaridgeTown_Gym_1F", 4),
            ("PetalburgCity_Gym", 5),
            ("FortreeCity_Gym", 6),
            ("MossdeepCity_Gym", 7),
            ("SootopolisCity_Gym_1F", 8),
        )
        constants = (GAME / "data/wayfarer_hoenn_source_constants.inc").read_text()

        for map_name, badge in gyms:
            flag = f"FLAG_BADGE{badge:02}_GET"
            script = (GAME / f"data/maps/{map_name}/scripts.inc").read_text()
            self.assertRegex(script, rf"\bsetflag\s+{flag}\b", map_name)
            self.assertRegex(script, rf"\bgoto_if_(?:set|unset)\s+{flag}\b", map_name)
            self.assertIn(
                f"#define {flag} 0x{0x6000 + 0x866 + badge:X}",
                constants,
            )

    def test_league_uses_hoenn_badge_count(self):
        league = (GAME / "data/maps/EverGrandeCity_PokemonLeague_1F/scripts.inc").read_text()

        self.assertIn("specialvar VAR_RESULT, WayfarerGetHoennBadgeCountForScript", league)
        self.assertIn("goto_if_lt VAR_RESULT, 8, EverGrandeCity_PokemonLeague_1F_EventScript_NotAllBadges", league)

    def test_standalone_emerald_keeps_its_original_league_gate(self):
        league = (GAME / "data/maps/EverGrandeCity_PokemonLeague_1F/scripts.inc").read_text()
        wayfarer_gate = league.index("#if IS_WAYFARER")
        standalone_gate = league.index(
            "goto_if_unset FLAG_BADGE06_GET, EverGrandeCity_PokemonLeague_1F_EventScript_NotAllBadges",
            wayfarer_gate,
        )
        self.assertGreater(standalone_gate, league.index("#else", wayfarer_gate))

    def test_wayfarer_route_101_uses_the_existing_party(self):
        route = (GAME / "data/maps/Route101/scripts.inc").read_text()
        start = route.index("Route101_EventScript_BirchsBag::")
        wayfarer = route.index("#if IS_WAYFARER", start)
        standalone = route.index("#else", wayfarer)
        self.assertIn("setwildbattle SPECIES_ZIGZAGOON, 2", route[wayfarer:standalone])
        self.assertIn("dowildbattle", route[wayfarer:standalone])
        self.assertNotIn("special ChooseStarter", route[wayfarer:standalone])
        self.assertIn("special ChooseStarter", route[standalone:])

    def test_visitor_gets_thanks_and_a_rating_and_no_starter(self):
        lab = (GAME / "data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc").read_text()
        choice = lab.index("LittlerootTown_ProfessorBirchsLab_EventScript_GiveStarterEvent::")
        native = lab.index("LittlerootTown_ProfessorBirchsLab_EventScript_AcknowledgeNativeStarter::", choice)
        visitor = lab[choice:native]
        self.assertIn("LittlerootTown_ProfessorBirchsLab_Text_VisitorThanks", visitor)
        self.assertIn("goto ProfBirch_EventScript_AskRatePokedex", visitor)
        self.assertNotIn("MULTI_HOENN_STARTERS", lab)
        self.assertNotIn("SPECIES_TREECKO", lab)
        self.assertNotIn("VAR_HOENN_STARTER_CHOICE", lab)
        self.assertNotIn("FLAG_HOENN_STARTER_RECEIVED", lab)

    def test_birch_roams_by_the_hoenn_copy_of_his_state_once_his_opening_is_over(self):
        clock = (GAME / "src/time_events.c").read_text()
        self.assertNotIn("GetVarPointer(VAR_BIRCH_STATE)", clock)
        self.assertEqual(clock.count("GetVarPointer(HOENN_VAR_ID(VAR_BIRCH_STATE))"), 2)

        script = (GAME / "data/scripts/prof_birch.inc").read_text()
        update = script.split("ProfBirch_EventScript_UpdateLocation::", 1)[1]
        update = update.split("ProfBirch_EventScript_MoveToLab::", 1)[0]
        order = [
            "goto_if_set FLAG_SYS_GAME_CLEAR, ProfBirch_EventScript_MoveToLab",
            "call ProfBirch_EventScript_CheckOpeningOver",
            "goto_if_eq VAR_RESULT, FALSE, Common_EventScript_NopReturn",
            "goto_if_eq VAR_BIRCH_LAB_STATE, 2, ProfBirch_EventScript_MoveToLab",
            "call_if_eq VAR_BIRCH_STATE, 2, ProfBirch_EventScript_MoveToRoute101",
        ]
        self.assertEqual([update.index(line) for line in order], sorted(update.index(line) for line in order))

    def test_route_103_rival_belongs_to_the_hoenn_origin(self):
        persistence = (GAME / "src/wayfarer_persistence.c").read_text()
        lab = (GAME / "data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc").read_text()
        route = (GAME / "data/maps/Route103/scripts.inc").read_text()
        self.assertIn(
            "FlagSet(HOENN_FLAG_ID(WAYFARER_HOENN_HIDE_ROUTE_103_RIVAL_FLAG));",
            persistence,
        )
        self.assertEqual(lab.count("clearflag FLAG_HIDE_ROUTE_103_RIVAL"), 1)
        native = lab.split("LittlerootTown_ProfessorBirchsLab_EventScript_AcknowledgeNativeStarter::", 1)[1]
        native = native.split("LittlerootTown_ProfessorBirchsLab_EventScript_NicknameStarter::", 1)[0]
        self.assertIn("clearflag FLAG_HIDE_ROUTE_103_RIVAL", native)
        self.assertNotIn("givemon", native)
        self.assertIn("special WayfarerGrantSharedEquipment", native)
        self.assertRegex(
            route,
            r"specialvar VAR_RESULT, WayfarerIsHoennOrigin\s+"
            r"call_if_eq VAR_RESULT, FALSE, Route103_EventScript_HideRival",
        )

    def test_native_rescue_commits_after_delivery_and_keeps_loss_continuation(self):
        route = (GAME / "data/maps/Route101/scripts.inc").read_text()
        native = route.split("Route101_EventScript_NativeBirchRescue::", 1)[1]
        native = native.split("Route101_EventScript_BirchRescueAcknowledged::", 1)[0]
        self.assertIn("special ChooseStarter", native)
        self.assertNotIn("GetBattleOutcome", native)
        received = native.index("goto_if_unset FLAG_HOENN_STARTER_RECEIVED")
        self.assertIn("setflag FLAG_RESCUED_BIRCH", native[received:])
        self.assertIn("addobject LOCALID_ROUTE101_ZIGZAGOON", route)
        callback = (GAME / "src/battle_setup.c").read_text()
        native_callback = callback.split("static bool8 TryGiveNativeHoennStarter(u16 choice)\n{", 1)[1]
        native_callback = native_callback.split("#if TESTING", 1)[0]
        self.assertNotIn("VAR_STARTER_MON", native_callback)
        self.assertLess(native_callback.index("GiveScriptedMonToPlayer"),
                        native_callback.index("FlagSet(FLAG_HOENN_STARTER_RECEIVED)"))

    def test_hoenn_rewards_update_shared_ui_through_specials(self):
        lab = (GAME / "data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc").read_text()
        town = (GAME / "data/maps/LittlerootTown/scripts.inc").read_text()
        self.assertIn("specialvar VAR_RESULT, WayfarerHasSharedPokedex", lab)
        self.assertIn("special WayfarerGrantSharedPokedex", lab)
        self.assertIn("special WayfarerGrantRunningShoes", town)
        constants = (GAME / "data/wayfarer_hoenn_source_constants.inc").read_text()
        self.assertIn("#define VAR_HOENN_STARTER_CHOICE 0x7023", constants)
        self.assertIn("#define FLAG_SYS_POKEDEX_GET 0x6861", constants)


if __name__ == "__main__":
    unittest.main()
