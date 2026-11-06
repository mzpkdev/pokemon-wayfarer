import json
import re
import unittest
from pathlib import Path


GAME = Path(__file__).resolve().parents[2]


def script(map_name):
    return (GAME / f"data/maps/{map_name}/scripts.inc").read_text()


def label_body(source, label):
    start = re.search(rf"^{re.escape(label)}:+\n", source, re.M)
    if start is None:
        raise AssertionError(f"missing label {label}")
    end = re.compile(r"^\s*(?:end|return|step_end)\b.*$", re.M).search(source, start.end())
    return source[start.end():end.start()]


class NormanTests(unittest.TestCase):
    def test_the_tutorial_readies_the_gym_and_reloads_it(self):
        body = label_body(script("PetalburgCity_Gym"), "PetalburgCity_Gym_EventScript_ReturnFromWallyTutorial")
        ordered = (
            "setvar VAR_PETALBURG_GYM_STATE, 6",
            "call Common_EventScript_ReadyPetalburgGymForBattle",
            "warp MAP_PETALBURG_CITY_GYM, 4, 108",
        )
        positions = [body.index(step) for step in ordered]
        self.assertEqual(positions, sorted(positions))

    def test_no_other_gym_changes_petalburg_gym_state(self):
        for map_name in ("RustboroCity_Gym", "DewfordTown_Gym", "MauvilleCity_Gym", "LavaridgeTown_Gym_1F"):
            self.assertNotIn("VAR_PETALBURG_GYM_STATE", script(map_name), map_name)
            self.assertNotIn("ReadyPetalburgGymForBattle", script(map_name), map_name)

    def test_norman_neither_counts_badges_nor_sends_the_player_away(self):
        source = script("PetalburgCity_Gym")
        self.assertNotIn("WayfarerGetHoennBadgeCountForScript", source)
        for gone in ("four GYM BADGES", "as I promised", "RUSTBORO CITY", "DEWFORD TOWN", "number of BADGES"):
            self.assertNotIn(gone, source)

    def test_an_unbeaten_norman_waits_at_the_back_for_a_champion_of_another_region(self):
        transition = label_body(script("PetalburgCity_Gym"), "PetalburgCity_Gym_OnTransition")
        self.assertLess(
            transition.index("goto_if_unset FLAG_DEFEATED_PETALBURG_GYM"),
            transition.index("PetalburgCity_Gym_EventScript_CheckNormanForRematch"),
        )

    def test_norman_leaves_briney_and_wattson_alone(self):
        victory = script("PetalburgCity_Gym")
        self.assertNotIn("EventScript_HideMrBriney", victory)
        self.assertNotIn("WATTSON", victory)
        common = (GAME / "data/event_scripts.s").read_text()
        self.assertNotIn("FLAG_DEFEATED_PETALBURG_GYM", label_body(common, "Common_EventScript_UpdateBrineyLocation"))

    def test_hall_of_fame_retires_briney_as_it_boards_him(self):
        clear = (GAME / "data/scripts/hall_of_fame.inc").read_text()
        self.assertLess(
            clear.index("clearflag FLAG_HIDE_SS_TIDAL_CORRIDOR_MR_BRINEY"),
            clear.index("call EventScript_HideMrBriney"),
        )

    def test_wallys_mother_says_he_left_only_once_he_has(self):
        mother = label_body(script("PetalburgCity_WallysHouse"), "PetalburgCity_WallysHouse_EventScript_WallysMom")
        self.assertNotIn("FLAG_RECEIVED_HM_SURF", mother)
        self.assertIn("goto_if_unset FLAG_DEFEATED_WALLY_MAUVILLE", mother)
        self.assertIn("goto_if_set FLAG_DEFEATED_LAVARIDGE_GYM", mother)

    def test_juan_counts_no_badges(self):
        source = script("SootopolisCity_Gym_1F")
        self.assertNotIn("FLAG_BADGE06_GET", source)
        self.assertNotIn("FORTREE", source)

    def test_petalburg_scott_raises_the_count(self):
        self.assertNotRegex(script("PetalburgCity"), r"setvar VAR_SCOTT_STATE\b")
        self.assertIn("addvar VAR_SCOTT_STATE, 1", script("PetalburgCity"))


class WallyTests(unittest.TestCase):
    def test_mauville_shows_wally_only_after_the_tutorial(self):
        source = script("MauvilleCity")
        self.assertIn(
            "call_if_unset FLAG_DEFEATED_WALLY_MAUVILLE, MauvilleCity_EventScript_PlaceWally",
            label_body(source, "MauvilleCity_OnTransition"),
        )
        place = label_body(source, "MauvilleCity_EventScript_PlaceWally")
        self.assertIn("goto_if_ge VAR_PETALBURG_GYM_STATE, 6", place)
        self.assertIn("setflag FLAG_HIDE_MAUVILLE_CITY_WALLY\n", place)
        self.assertIn("setflag FLAG_HIDE_MAUVILLE_CITY_WALLYS_UNCLE", place)

    def test_wally_stays_out_of_verdanturf_once_flannery_is_beaten(self):
        source = script("MauvilleCity")
        self.assertIn(
            "call_if_unset FLAG_DEFEATED_LAVARIDGE_GYM, MauvilleCity_EventScript_ShowVerdanturfWally",
            label_body(source, "MauvilleCity_EventScript_DefeatedWally"),
        )

    def test_victory_road_waits_for_the_tutorial_and_recalls_only_a_real_win(self):
        source = script("VictoryRoad_1F")
        for trigger in ("Trigger1", "Trigger2"):
            body = label_body(source, f"VictoryRoad_1F_EventScript_WallyBattle{trigger}")
            self.assertLess(body.index("goto_if_lt VAR_PETALBURG_GYM_STATE, 6"), body.index("lockall"))
        battle = label_body(source, "VictoryRoad_1F_EventScript_WallyEntranceBattle")
        self.assertIn("goto_if_unset FLAG_DEFEATED_WALLY_MAUVILLE, VictoryRoad_1F_EventScript_WallyFirstChallenge", battle)
        first = source[source.index("VictoryRoad_1F_Text_WallyFirstChallenge:"):source.index("VictoryRoad_1F_Text_WallyEntranceDefeat:")]
        self.assertNotIn("losing to you", first)
        self.assertNotIn("anymore", first)


class LavaridgeTests(unittest.TestCase):
    def test_only_a_door_arms_the_scene_for_a_player_with_a_starter(self):
        source = script("LavaridgeTown")
        arm = label_body(source, "LavaridgeTown_EventScript_TryArmRivalScene")
        self.assertIn("goto_if_gt VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_MUDKIP", arm)
        self.assertEqual(len(re.findall(r"setvar VAR_LAVARIDGE_TOWN_STATE, 1\b", source)), 2)
        for label, y in (("ArmAtGymDoor", 15), ("ArmAtPokemonCenterDoor", 6)):
            body = source[source.index(f"LavaridgeTown_EventScript_{label}::"):]
            self.assertIn(f"goto_if_ne VAR_0x8005, {y},", body[:200])
        self.assertIn("setvar VAR_LAVARIDGE_TOWN_STATE, 4", script("LavaridgeTown_Gym_1F"))

    def test_the_arming_tiles_are_the_two_front_doors(self):
        warps = json.loads((GAME / "data/maps/LavaridgeTown/map.json").read_text())["warp_events"]
        doors = {warp["dest_map"]: (warp["x"], warp["y"]) for warp in warps if warp["dest_warp_id"] == "0"}
        self.assertEqual(doors["MAP_LAVARIDGE_TOWN_GYM_1F"], (5, 15))
        self.assertEqual(doors["MAP_LAVARIDGE_TOWN_POKEMON_CENTER_1F"], (9, 6))


if __name__ == "__main__":
    unittest.main()
