#!/usr/bin/env python3
"""Source wiring checks; these do not replace emulator travel acceptance."""
from pathlib import Path
import json
import re
import unittest

GAME = Path(__file__).resolve().parents[2]
GYMS = [('VioletCity_Gym_hns', 'VioletCity_Gym_EventScript_Falkner', 2, 0), ('AzaleaTown_Gym_hns', 'AzaleaTown_Gym_EventScript_Bugsy', 2, 1), ('GoldenrodCity_Gym_hns', 'GoldenrodCity_Gym_EventScript_Whitney', 2, 2), ('EcruteakCity_Gym_hns', 'EcruteakCity_Gym_EventScript_Morty', 2, 3), ('CianwoodGym_hns', 'CianwoodGym_EventScript_Chuck', 2, 4), ('OlivineCity_Gym_hns', 'OlivineCity_Gym_EventScript_Jasmine', 2, 5), ('MahoganyTown_Gym_hns', 'MahoganyTown_Gym_EventScript_Pryce', 2, 6), ('BlackthornCity_Gym_hns', 'BlackthornGym_EventScript_Clair', 2, 7), ('PewterCity_Gym_hns', 'PewterCity_Gym_EventScript_Brock', 1, 0), ('CeruleanCity_Gym_hns', 'CeruleanCity_Gym_EventScript_Misty', 1, 1), ('VermilionCity_Gym_hns', 'VermilionCity_Gym_EventScript_Surge', 1, 2), ('CeladonCity_Gym_hns', 'CeladonCity_Gym_EventScript_Erika', 1, 3), ('SaffronCity_Gym_hns', 'SaffronCity_Gym_EventScript_Sabrina', 1, 4), ('FuchsiaCity_Gym_hns', 'FuchsiaCity_Gym_EventScript_Janine', 1, 5), ('SeafoamIslands_Gym_hns', 'SeafoamIslands_Gym_EventScript_Blaine', 1, 6), ('ViridianCity_Gym_hns', 'ViridianCity_Gym_EventScript_Blue', 1, 7), ('RustboroCity_Gym', 'RustboroCity_Gym_EventScript_Roxanne', 3, 0), ('DewfordTown_Gym', 'DewfordTown_Gym_EventScript_Brawly', 3, 1), ('MauvilleCity_Gym', 'MauvilleCity_Gym_EventScript_Wattson', 3, 2), ('LavaridgeTown_Gym_1F', 'LavaridgeTown_Gym_1F_EventScript_Flannery', 3, 3), ('PetalburgCity_Gym', 'PetalburgCity_Gym_EventScript_NormanBattle', 3, 4), ('FortreeCity_Gym', 'FortreeCity_Gym_EventScript_Winona', 3, 5), ('MossdeepCity_Gym', 'MossdeepCity_Gym_EventScript_TateAndLiza', 3, 6), ('SootopolisCity_Gym_1F', 'SootopolisCity_Gym_1F_EventScript_Juan', 3, 7), ('GoldenrodCity_Gym_hns', 'GoldenrodCity_Gym_EventScript_WhitneyBadge', 2, 2), ('DragonsDen_Shrine_hns', 'DragonsDen_Shrine_EventScript_ElderOfferQuiz', 2, 7)]


def script(name):
    return (GAME / "data/maps" / name / "scripts.inc").read_text()


class CircuitScriptTests(unittest.TestCase):
    def test_rollout_uses_the_production_johto_start_default(self):
        config = (GAME / "include/config/league_circuit.h").read_text()
        self.assertIn("#define WAYFARER_LEAGUE_CIRCUIT_ENABLED 1", config)
        self.assertIn("compile-time rollback", config)
        self.assertNotIn("regional openings and League travel/release acceptance", config)

    def test_all_24_badges_and_deferred_awards_guarded_before_state(self):
        self.assertEqual(len({(r, b) for _, _, r, b in GYMS}), 24)
        for name, label, region, badge in GYMS:
            with self.subTest(name=name, label=label):
                body = script(name).split(label + "::\n", 1)[1]
                prefix = body.split("#endif", 1)[0]
                self.assertTrue(prefix.startswith("#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED"))
                self.assertIn("setvar VAR_0x8005, " + str(badge), prefix)
                self.assertIn("LeagueCircuit_CanChallengeGym", prefix)
                self.assertIn("FALSE, LeagueCircuit_EventScript_PostponeGym", prefix)
                self.assertNotRegex(prefix, r"\b(?:trainerbattle\w*|setflag|clearflag|giveitem)\b")

    def test_each_badge_commit_announces_qualification(self):
        award_count = 0
        for name in {row[0] for row in GYMS}:
            source = script(name)
            for match in re.finditer(r"setflag FLAG_BADGE\d+_GET\n", source):
                award_count += 1
                self.assertTrue(source[match.end():].startswith("#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED\n\tcall LeagueCircuit_EventScript_CheckQualification\n#endif"))
        self.assertEqual(award_count, 24)

    def test_unearned_badges_skip_legacy_rematch_dispatch(self):
        cases = (
            ("CianwoodGym_hns", "FLAG_BADGE05_GET", "CianwoodGym_EventScript_ChuckOfficialChallenge", "FLAG_HIDE_DOJO_CHUCK", "TRAINER_CHUCK_1_HNS"),
            ("ViridianCity_Gym_hns", "FLAG_BADGE16_GET", "ViridianCity_Gym_EventScript_BlueOfficialChallenge", "FLAG_HIDE_DOJO_BLUE", "TRAINER_BLUE_HNS"),
            ("BlackthornCity_Gym_hns", "FLAG_BADGE08_GET", "BlackthornGym_EventScript_ClairOfficialChallenge", "FLAG_IS_CHAMPION", "TRAINER_CLAIR_1_HNS"),
        )
        for name, badge, target, legacy_gate, trainer in cases:
            with self.subTest(name=name):
                source = script(name)
                branch = "goto_if_unset " + badge + ", " + target
                self.assertLess(source.index(branch), source.index(legacy_gate))
                self.assertLess(source.index(legacy_gate), source.index(target + "::"))
                self.assertIn(trainer, source.split(target + "::", 1)[1])
        source = script("ViridianCity_Gym_hns")
        self.assertIn("call_if_set FLAG_BADGE15_GET, ViridianCity_Gym_EventScript_MoveDefeatedBlaine", source)

    def test_fixed_hns_selection_and_eligibility(self):
        for room in ("Wills", "Kogas", "Brunos", "Karens", "Champions"):
            source = script("PokemonLeague_" + room + "Room_hns")
            self.assertIn("LeagueCircuit_IsEligible", source)
            for pending in source.split("specialvar VAR_0x8004, LeagueCircuit_GetRequiredRegion")[1:]:
                gate = pending.split("specialvar VAR_RESULT, LeagueCircuit_IsEligible", 1)[0]
                self.assertIn("goto_if_eq VAR_0x8004, REGION_HOENN, LeagueCircuit_EventScript_", gate)
                self.assertIn("goto_if_eq VAR_0x8004, REGION_NONE, LeagueCircuit_EventScript_", gate)
            self.assertRegex(source, r"goto_if_eq VAR_0x8004, REGION_JOHTO, \w+Rematch")
        for room in ("Sidneys", "Phoebes", "Glacias", "Drakes", "Champions"):
            source = script("EverGrandeCity_" + room + "Room")
            self.assertIn("setvar VAR_0x8004, REGION_HOENN", source)
            self.assertIn("LeagueCircuit_IsEligible", source)

    def test_clear_bypasses_hns_crossregional_reset(self):
        source = (GAME / "data/scripts/league_circuit.inc").read_text()
        clear = source.split("LeagueCircuit_EventScript_IndigoHallOfFame::", 1)[1]
        self.assertLess(clear.index("LeagueCircuit_RecordClear"), clear.index("special GameClear"))
        self.assertNotIn("SetGameClearFlags", clear)
        self.assertNotIn("SetFirstGameClearFlags", clear)
        self.assertIn("setvar VAR_LEAGUE_STATE, 1", clear)
        self.assertLess(clear.index("LeagueCircuit_RecordClear"), clear.index("setvar VAR_LEAGUE_STATE, 1"))
        self.assertLess(clear.index("setvar VAR_LEAGUE_STATE, 1"), clear.index("special GameClear"))
        self.assertIn("HEAL_LOCATION_INDIGO_PLATEAU_HNS", clear)
        self.assertIn("LeagueCircuit_RecordClear", script("EverGrandeCity_HallOfFame"))

    def test_corridor_bypass_leaves_story_unwritten(self):
        gate = script("ReceptionGate_hns").split("ReceptionGate_Trigger::\n", 1)[1].split("#endif", 1)[0]
        self.assertIn("\tend", gate)
        self.assertNotIn("setvar", gate)
        rival = script("VictoryRoadKanto_1F_hns").split("VictoryRoadKanto_1F_Trigger::\n", 1)[1].split("#endif", 1)[0]
        self.assertIn("LeagueCircuit_IsEligible", rival)
        self.assertNotIn("setflag", rival)

    def test_johto_opening_establishes_the_complete_interregional_ferry_loop(self):
        olivine = script("OlivineCity_PortInside_hns")
        maiden = olivine.split("OlivinePort_EventScript_Sailor_MaidenVoyage::", 1)[1].split("OlivinePort_EventScript_Sailor_ResumeMaidenVoyage::", 1)[0]
        self.assertIn("setvar VAR_SSAQUA_STATE, 1", maiden)
        self.assertNotIn("checkitem ITEM_SS_TICKET", maiden)

        reunion = script("SSAqua_RoomSSE_hns")
        self.assertIn("giveitem ITEM_SS_TICKET", reunion)
        arrival = script("SSAqua_1F_hns")
        self.assertIn("setvar VAR_SSAQUA_STATE, 8", arrival)

        vermilion = script("VermilionCity_PortInside_hns").split("VermilionPort_EventScript_ChoseSlateport::", 1)[1]
        self.assertIn("goto_if_lt VAR_SSAQUA_STATE, 8", vermilion)
        self.assertIn("checkitem ITEM_SS_TICKET", vermilion)
        self.assertIn("WayfarerPrepareHoennEntry", vermilion)
        self.assertIn("MAP_SLATEPORT_CITY_HARBOR", vermilion)

        slateport = script("SlateportCity_Harbor").split("WayfarerHoennEntry_EventScript_SlateportAquaAttendant::", 1)[1]
        self.assertIn("goto_if_lt 0x408B, 8", slateport)
        self.assertIn("checkitem ITEM_SS_TICKET", slateport)
        self.assertIn("MAP_OLIVINE_CITY_PORT_INSIDE_HNS", slateport)

    def test_league_clear_continuations_preserve_the_interregional_network(self):
        clear = (GAME / "src/post_battle_event_funcs.c").read_text()
        self.assertIn("SetContinueGameWarpToHealLocation(HEAL_LOCATION_INDIGO_PLATEAU_HNS)", clear)
        self.assertIn("SetContinueGameWarpToHealLocation(HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE)", clear)
        self.assertIn("LeagueCircuit_EventScript_IndigoHallOfFame", script("PokemonLeague_HallOfFame_hns"))
        self.assertIn("LeagueCircuit_RecordClear", (GAME / "data/scripts/league_circuit.inc").read_text())
        self.assertIn("LeagueCircuit_RecordClear", script("EverGrandeCity_HallOfFame"))

        indigo = json.loads((GAME / "data/maps/IndigoPlateau_PokemonCenter_hns/map.json").read_text())
        self.assertIn("MAP_INDIGO_PLATEAU_HNS", {warp["dest_map"] for warp in indigo["warp_events"]})
        ever_grande = json.loads((GAME / "data/maps/EverGrandeCity_PokemonLeague_1F/map.json").read_text())
        self.assertIn("MAP_EVER_GRANDE_CITY", {warp["dest_map"] for warp in ever_grande["warp_events"]})


if __name__ == "__main__":
    unittest.main()
