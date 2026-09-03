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

    def test_norman_and_league_use_hoenn_badge_count(self):
        norman = (GAME / "data/maps/PetalburgCity_Gym/scripts.inc").read_text()
        league = (GAME / "data/maps/EverGrandeCity_PokemonLeague_1F/scripts.inc").read_text()

        self.assertIn("specialvar VAR_RESULT, WayfarerGetHoennBadgeCountForScript", norman)
        self.assertRegex(norman, r"case 3, PetalburgCity_Gym_EventScript_NormanThreeBadges\s+goto PetalburgCity_Gym_EventScript_NormanBattle")
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

    def test_local_starter_choice_and_delivery_commit_separately(self):
        lab = (GAME / "data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc").read_text()
        choice = lab.index("LittlerootTown_ProfessorBirchsLab_EventScript_GiveStarterEvent::")
        standalone = lab.index("#else", choice)
        wayfarer = lab[choice:standalone]
        self.assertIn("MULTI_HOENN_STARTERS", wayfarer)
        self.assertIn("setvar VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_TREECKO", wayfarer)
        self.assertIn("case MULTI_B_PRESSED", wayfarer)
        self.assertIn("goto_if_eq VAR_RESULT, MON_CANT_GIVE", wayfarer)
        self.assertLess(
            wayfarer.index("goto_if_eq VAR_RESULT, MON_CANT_GIVE"),
            wayfarer.index("setflag FLAG_HOENN_STARTER_RECEIVED"),
        )
        self.assertIn("goto_if_unset FLAG_HOENN_STARTER_RECEIVED", lab)

    def test_route_103_rival_waits_for_a_committed_local_starter_choice(self):
        persistence = (GAME / "src/wayfarer_persistence.c").read_text()
        lab = (GAME / "data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc").read_text()
        route = (GAME / "data/maps/Route103/scripts.inc").read_text()
        self.assertIn(
            "FlagSet(HOENN_FLAG_ID(WAYFARER_HOENN_HIDE_ROUTE_103_RIVAL_FLAG));",
            persistence,
        )
        self.assertEqual(lab.count("clearflag FLAG_HIDE_ROUTE_103_RIVAL"), 3)
        self.assertIn(
            "goto_if_gt VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_MUDKIP, Route103_EventScript_LocalStarterRequired",
            route,
        )
        self.assertRegex(
            route,
            r"Route103_EventScript_LocalStarterRequired::\s+lockall\s+msgbox .*?\s+releaseall\s+end",
        )


if __name__ == "__main__":
    unittest.main()
