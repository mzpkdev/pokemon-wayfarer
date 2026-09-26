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


def block(source, label):
    body = source.split(label + "::\n", 1)[1]
    return re.split(r"(?m)^[A-Za-z_][A-Za-z0-9_]*::\s*$", body, maxsplit=1)[0]


class CircuitScriptTests(unittest.TestCase):
    def test_rollout_uses_the_production_johto_start_default(self):
        config = (GAME / "include/config/league_circuit.h").read_text()
        self.assertIn("#define WAYFARER_LEAGUE_CIRCUIT_ENABLED 1", config)
        self.assertIn("compile-time rollback", config)
        self.assertNotIn("regional openings and League travel/release acceptance", config)

    def test_all_24_initial_badges_and_deferred_awards_are_uncapped(self):
        self.assertEqual(len({(r, b) for _, _, r, b in GYMS}), 24)
        for name, label, region, badge in GYMS:
            with self.subTest(name=name, label=label):
                source = script(name)
                self.assertIn(label + "::", source)
                self.assertNotIn("LeagueCircuit_CanChallengeGym", source)
                self.assertNotIn("LeagueCircuit_EventScript_PostponeGym", source)

    def test_badge_commits_do_not_announce_circuit_status(self):
        award_count = 0
        for name in {row[0] for row in GYMS}:
            source = script(name)
            award_count += len(re.findall(r"setflag FLAG_BADGE\d+_GET\n", source))
            self.assertNotIn("LeagueCircuit_EventScript_CheckQualification", source)
        self.assertEqual(award_count, 24)

    def test_trainer_card_is_the_only_detailed_status_surface(self):
        sources = "\n".join(path.read_text() for path in (GAME / "data/maps").rglob("scripts.inc"))
        self.assertNotIn("LeagueCircuit_EventScript_ShowStatus", sources)
        self.assertNotIn("LeagueCircuit_EventScript_Itinerary", sources)
        self.assertNotIn("LeagueCircuit_EventScript_CheckQualification", sources)

        self.assertNotIn("LeagueCircuit_", block(script("NewBarkTown_Lab_hns"), "NewBarkTown_Lab_EventScript_ChoseStarter"))
        self.assertNotIn("LeagueCircuit_", block(script("IndigoPlateau_PokemonCenter_hns"), "IndigoPlateau_EventScript_Cooltrainer"))
        self.assertNotIn("LeagueCircuit_", block(script("ReceptionGate_hns"), "ReceptionGate_EventScript_Officer"))

        pallet = block(script("PalletTown_Lab_hns"), "PalletTown_Lab_EventScript_Oak16Badges")
        circuit_pallet = re.sub(
            r"#if !\(IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED\).*?#endif",
            "",
            pallet,
            flags=re.DOTALL,
        )
        self.assertNotIn("PalletTown_Lab_EventScript_OakKantoLeague", circuit_pallet)

    def test_rocket_takeover_recovers_from_mixed_badge_orders(self):
        source = script("MahoganyTown_Gym_hns")
        recovery = block(source, "MahoganyTown_Gym_EventScript_TryArmRocketTakeover")
        self.assertIn("goto_if_ne VAR_TRIGGER_ELM_ROCKET_CALL, 0", recovery)
        self.assertIn("goto_if_ge VAR_GOLDENROD_CITY_STATE, 6", recovery)
        self.assertIn("goto_if_ge VAR_MAHOGANY_TOWN_STATE, 16", recovery)
        self.assertIn("goto_if_lt VAR_MAHOGANY_TOWN_STATE, 15", recovery)
        for flag in ("FLAG_BADGE05_GET", "FLAG_BADGE06_GET", "FLAG_BADGE07_GET"):
            self.assertIn("goto_if_unset " + flag, recovery)
        self.assertIn("setvar VAR_TRIGGER_ELM_ROCKET_CALL, 1", recovery)
        self.assertNotIn("VAR_NUM_BADGES", recovery)

        for name, award in (
            ("CianwoodGym_hns", "CianwoodGym_EventScript_ChuckVictory"),
            ("OlivineCity_Gym_hns", "OlivineCity_Gym_EventScript_JasmineVictory"),
            ("MahoganyTown_Gym_hns", "MahoganyTown_Gym_EventScript_PryceVictory"),
        ):
            with self.subTest(name=name):
                self.assertIn("call MahoganyTown_Gym_EventScript_TryArmRocketTakeover", block(script(name), award))
        self.assertIn(
            "call MahoganyTown_Gym_EventScript_TryArmRocketTakeover",
            block(script("Mahoganytown_hns"), "Mahoganytown_OnLoad"),
        )

    def test_blue_invitation_and_wattson_relocation_are_order_independent(self):
        cinnabar = block(script("CinnabarIsland_hns"), "CinnabarIsland_EventScript_Blue")
        circuit_invitation = cinnabar.split("#else", 1)[0]
        self.assertNotIn("VAR_NUM_BADGES", circuit_invitation)
        active = block(script("CinnabarIsland_hns"), "CinnabarIsland_EventScript_BlueActive")
        self.assertIn("setflag FLAG_HIDE_CINNABAR_BLUE", active)
        self.assertIn("clearflag FLAG_HIDE_VIRIDIAN_BLUE", active)
        blue_intro = script("ViridianCity_Gym_hns").split("ViridianCity_Gym_Text_LeaderBlue_Before:\n", 1)[1]
        blue_intro = blue_intro.split("ViridianCity_Gym_Text_LeaderBlue_Win:", 1)[0]
        circuit_intro = blue_intro.split("#else", 1)[0]
        self.assertNotIn("JOHTO CHAMP", circuit_intro)
        self.assertNotIn("conquered all", circuit_intro)

        helper = block(script("MauvilleCity_Gym"), "MauvilleCity_Gym_EventScript_TryRelocateWattson")
        self.assertIn("goto_if_unset FLAG_DEFEATED_PETALBURG_GYM", helper)
        self.assertIn("goto_if_unset FLAG_BADGE03_GET", helper)
        self.assertIn("setflag FLAG_HIDE_MAUVILLE_GYM_WATTSON", helper)
        self.assertIn("clearflag FLAG_HIDE_MAUVILLE_CITY_WATTSON", helper)
        self.assertIn("call MauvilleCity_Gym_EventScript_TryRelocateWattson", block(script("MauvilleCity_Gym"), "MauvilleCity_Gym_EventScript_WattsonDefeated"))
        norman = script("PetalburgCity_Gym").split("PetalburgCity_Gym_EventScript_NormanBattle::\n", 1)[1].split("PetalburgCity_Gym_EventScript_GiveFacade::\n", 1)[0]
        self.assertIn("call MauvilleCity_Gym_EventScript_TryRelocateWattson", norman)
        self.assertNotIn("setflag FLAG_HIDE_MAUVILLE_GYM_WATTSON", norman.split("#else", 1)[0])

    def test_deferred_whitney_and_clair_awards_are_idempotent_and_monotonic(self):
        whitney = script("GoldenrodCity_Gym_hns")
        interaction = block(whitney, "GoldenrodCity_Gym_EventScript_Whitney")
        self.assertLess(interaction.index("FLAG_BADGE03_GET"), interaction.index("TRAINER_WHITNEY_1_HNS"))
        self.assertIn("goto_if_defeated TRAINER_WHITNEY_1_HNS", interaction)
        award = block(whitney, "GoldenrodCity_Gym_EventScript_WhitneyBadge")
        self.assertIn("goto_if_set FLAG_BADGE03_GET", award)
        self.assertIn("goto_if_defeated TRAINER_WHITNEY_1_HNS", award)
        self.assertIn("goto_if_ge VAR_GOLDENROD_CITY_STATE, 5", whitney)

        shrine = script("DragonsDen_Shrine_hns")
        shrine_on_load = block(shrine, "DragonsDen_Shrine_OnLoad")
        self.assertIn("goto_if_unset FLAG_BADGE08_GET", shrine_on_load)
        self.assertIn("setvar VAR_BLACKTHORN_CITY_STATE, 3", shrine_on_load)
        self.assertIn(
            "goto_if_set FLAG_BADGE08_GET, DragonsDen_Shrine_EventScript_RecoverAfterRisingBadge",
            block(shrine, "DragonsDen_Shrine_EventScript_ElderOfferQuiz"),
        )
        recovery = block(shrine, "DragonsDen_Shrine_EventScript_RecoverAfterRisingBadge")
        self.assertIn("goto_if_ge VAR_BLACKTHORN_CITY_STATE, 3", recovery)
        self.assertNotIn("addvar VAR_NUM_BADGES", recovery)

        clair = block(script("BlackthornCity_Gym_hns"), "BlackthornGym_EventScript_Clair")
        self.assertIn("goto BlackthornGym_EventScript_Clair_Defeated", clair.split("#endif", 1)[0])

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
        self.assertNotIn("call_if_set FLAG_BADGE15_GET, ViridianCity_Gym_EventScript_MoveDefeatedBlaine", source)
        self.assertIn("#if !IS_WAYFARER\n\tsetflag FLAG_HIDE_SEAFOAM_BLAINE", source)

    def test_fixed_room_selection_uses_saved_run_and_recovers_invalid_state(self):
        masters = (GAME / "data/scripts/wayfarer_masters_league.inc").read_text()
        for index, trainer in enumerate(("WILL", "KOGA", "BRUNO", "KAREN", "LANCE")):
            with self.subTest(stage="masters", trainer=trainer):
                self.assertIn(f"setvar VAR_0x8005, {index}", masters)
                self.assertIn(f"trainerbattle_no_intro TRAINER_{trainer}_2_HNS", masters)
        self.assertEqual(masters.count("LeagueCircuit_ValidateRoomBattle"), 5)
        self.assertEqual(masters.count("LeagueCircuit_RecordRoomVictory"), 5)

        indigo = (GAME / "data/scripts/wayfarer_indigo_league.inc").read_text()
        for index, trainer in enumerate(("LORELEI", "BRUNO", "AGATHA", "LANCE", "BLUE")):
            with self.subTest(stage="indigo", trainer=trainer):
                self.assertIn(f"setvar VAR_0x8005, {index}", indigo)
                self.assertIn(f"trainerbattle_no_intro TRAINER_WAYFARER_INDIGO_{trainer}", indigo)
        self.assertEqual(indigo.count("LeagueCircuit_ValidateRoomBattle"), 5)
        self.assertEqual(indigo.count("LeagueCircuit_RecordRoomVictory"), 5)

        graphics = (GAME / "src/data/object_events/object_event_graphics_info_pointers.h").read_text()
        wayfarer_closure = graphics.split("#if HAS_SEVII_CONTENT", 1)[1]
        for actor in ("LORELEI", "BRUNO", "AGATHA", "LANCE"):
            with self.subTest(graphics=actor):
                self.assertIn(f"[OBJ_EVENT_GFX_{actor}]", wayfarer_closure)

        for room in ("Sidneys", "Phoebes", "Glacias", "Drakes", "Champions"):
            source = script("EverGrandeCity_" + room + "Room")
            self.assertIn("LeagueCircuit_ValidateRoomBattle", source)
            self.assertIn("LeagueCircuit_RecordRoomVictory", source)
            self.assertNotIn("LeagueCircuit_IsEligible", source)
            self.assertIn("goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_HoennEntryDenied", source)

    def test_lobby_admission_remains_live_and_room_scripts_never_recapture(self):
        self.assertIn("LeagueCircuit_IsEligible", script("EverGrandeCity_PokemonLeague_1F"))
        for venue in ("PokemonLeague_*_hns", "EverGrandeCity_*"):
            for path in (GAME / "data/maps").glob(venue + "/scripts.inc"):
                self.assertNotIn("LeagueCircuit_BeginRun", path.read_text())

    def test_champion_completion_and_hall_of_fame_validate_before_effects(self):
        champion = script("EverGrandeCity_ChampionsRoom")
        victory = "specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory"
        self.assertLess(champion.index("trainerbattle_no_intro TRAINER_WALLACE"), champion.index(victory))
        self.assertLess(champion.index(victory), champion.index("goto EverGrandeCity_ChampionsRoom_EventScript_Defeated"))

        indigo = (GAME / "data/scripts/wayfarer_indigo_league.inc").read_text()
        self.assertLess(indigo.index("LeagueCircuit_CanCompleteRun"),
                        indigo.index("FLDEFF_HALL_OF_FAME_RECORD_FRLG"))
        self.assertLess(indigo.index("FLDEFF_HALL_OF_FAME_RECORD_FRLG"),
                        indigo.index("LeagueCircuit_CommitAndRegisterIndigo"))
        masters = (GAME / "data/scripts/wayfarer_masters_league.inc").read_text()
        self.assertLess(masters.index("LeagueCircuit_CanCompleteRun"),
                        masters.index("LeagueCircuit_CommitRunClear"))
        self.assertLess(masters.index("LeagueCircuit_CommitRunClear"),
                        masters.index("warp MAP_SEVEN_ISLAND_HOUSE_ROOM1", masters.index("WayfarerMasters_EventScript_Gallery")))
        hoenn = script("EverGrandeCity_HallOfFame")
        self.assertLess(hoenn.index("LeagueCircuit_CanCompleteRun"),
                        hoenn.index("dofieldeffect FLDEFF_HALL_OF_FAME_RECORD"))
        self.assertLess(hoenn.index("dofieldeffect FLDEFF_HALL_OF_FAME_RECORD"),
                        hoenn.index("LeagueCircuit_CommitRunClear"))

    def test_defeated_rooms_restore_exits_and_champions_do_not_refight_on_resume(self):
        wills = script("PokemonLeague_WillsRoom_hns")
        on_load = block(wills, "EliteFourRoom_OnLoad")
        self.assertIn("LeagueCircuit_IsCurrentRoomDefeated", on_load)
        self.assertIn("call_if_eq VAR_RESULT, TRUE, EliteFourRoom_OpenExit", on_load)
        exit_tiles = block(wills, "EliteFourRoom_OpenExit")
        self.assertIn("setmetatile 6, 1, 0x28E, FALSE", exit_tiles)
        self.assertIn("setmetatile 6, 2, 0x296, FALSE", exit_tiles)
        for name, prefix, entrance, battle in (
            ("PokemonLeague_ChampionsRoom_hns", "ChampionRoom", "ChampionRoom_CloseDoor", "PokemonLeague_WillsRoom_EventScript_Lance"),
            ("EverGrandeCity_ChampionsRoom", "EverGrandeCity_ChampionsRoom", "EverGrandeCity_ChampionsRoom_EventScript_EnterRoom", "EverGrandeCity_ChampionsRoom_EventScript_Wallace"),
        ):
            source = script(name)
            self.assertIn("map_script MAP_SCRIPT_ON_LOAD, " + prefix + "_OnLoad", source)
            self.assertIn("LeagueCircuit_IsCurrentRoomDefeated", block(source, prefix + "_OnLoad"))
            for label in (entrance, battle):
                guard = block(source, label)
                self.assertIn("LeagueCircuit_IsCurrentRoomDefeated", guard)
                self.assertIn("goto_if_eq VAR_RESULT, TRUE, " + prefix + "_ResumeCompleted", guard)
            self.assertIn("HALL_OF_FAME", block(source, prefix + "_ResumeCompleted"))

    def test_indigo_clear_uses_one_frlg_record_without_full_credits(self):
        source = (GAME / "data/scripts/wayfarer_indigo_league.inc").read_text()
        first_clear = block(source, "WayfarerIndigo_HallCeremony")
        replay = block(source, "WayfarerIndigo_HallReplay")
        self.assertIn("FLDEFF_HALL_OF_FAME_RECORD_FRLG", first_clear)
        self.assertIn("LeagueCircuit_CommitAndRegisterIndigo", first_clear)
        self.assertNotIn("GameClear", source)
        self.assertNotIn("GivePartyMonChampionRibbon", replay)
        self.assertNotIn("FLDEFF_HALL_OF_FAME_RECORD_FRLG", replay)
        self.assertLess(source.index("WayfarerIndigo_HallReplay::"),
                        source.index("WayfarerIndigo_HallCommit::"))
        self.assertIn("LeagueCircuit_CommitRunClear", block(source, "WayfarerIndigo_HallCommit"))

        commit = (GAME / "src/post_battle_event_funcs.c").read_text()
        commit = commit.split("u16 LeagueCircuit_CommitAndRegisterIndigo(void)", 1)[1].split("\nint GameClear(void)", 1)[0]
        self.assertLess(commit.index("sIndigoHallOfFameCommitPending = TRUE"),
                        commit.index("SetMainCallback2(CB2_DoHallOfFameScreenFrlg)"))
        self.assertIn("CommitCircuitRun(CIRCUIT_STAGE_INDIGO)", commit)
        self.assertIn("SetContinueGameWarpToHealLocation(HEAL_LOCATION_INDIGO_PLATEAU_HNS)", commit)
        hall = (GAME / "src/hall_of_fame_frlg.c").read_text()
        save = hall.split("static void Task_Hof_TrySaveData(u8 taskId)\n{", 1)[1].split("\n}", 1)[0]
        self.assertLess(save.index("CommitPendingIndigoHallOfFame"),
                        save.index("TrySavingData(SAVE_HALL_OF_FAME)"))
        self.assertIn("ResolveIndigoHallOfFameSaveAttempt(saveStatus == SAVE_STATUS_OK", save)
        self.assertIn("gDamagedSaveSectors != 0", save)
        self.assertGreaterEqual(save.count("gTasks[taskId].func = Task_Hof_DelayAfterSave"), 2)
        retry = (GAME / "src/save_failed_screen.c").read_text()
        self.assertIn("ResolveIndigoHallOfFameSaveAttempt(TRUE, FALSE)", retry)
        self.assertGreaterEqual(retry.count("ResolveIndigoHallOfFameSaveAttempt(FALSE, FALSE)"), 2)
        flash = (GAME / "src/save.c").read_text()
        transaction = flash.split("if (IsIndigoHallOfFameSaveTransactionActive()", 1)[1].split("else", 1)[0]
        self.assertLess(transaction.index("HandleWriteSectorNBytes(SECTOR_ID_HOF_1"),
                        transaction.index("WriteSaveSectorOrSlot(FULL_SAVE_SLOT"))
        self.assertIn("if (!gDamagedSaveSectors)", transaction)
        self.assertIn("TryIncrementIndigoHallOfFameSaveCount()", flash)

    def test_red_keeps_non_circuit_completion_and_indigo_hides_early_oak(self):
        red = block(script("MtSilver_SummitDay_hns"), "MtSilver_SummitDay_EventScript_RedOnDefeat")
        self.assertIn("WayfarerRedGameClear", red)
        self.assertIn("GameClear", red)
        clear = (GAME / "src/post_battle_event_funcs.c").read_text()
        self.assertIn("int WayfarerRedGameClear(void)", clear)
        self.assertIn("sUseNonCircuitGameClear = TRUE", clear)

        champion = json.loads((GAME / "data/maps/PokemonLeague_ChampionsRoom_Frlg/map.json").read_text())
        oak = next(event for event in champion["object_events"]
                   if event["local_id"] == "LOCALID_CHAMPIONS_ROOM_PROF_OAK")
        self.assertTrue(oak["wayfarer_exclude"])
        hall = json.loads((GAME / "data/maps/PokemonLeague_HallOfFame_Frlg/map.json").read_text())
        self.assertTrue(hall["object_events"][0]["wayfarer_exclude"])

    def test_corridor_bypass_leaves_story_unwritten(self):
        gate = script("ReceptionGate_hns").split("ReceptionGate_Trigger::\n", 1)[1].split("#endif", 1)[0]
        self.assertIn("\tend", gate)
        self.assertNotIn("setvar", gate)
        rival = block(script("VictoryRoadKanto_1F_hns"), "VictoryRoadKanto_1F_Trigger").split("\tlock\n", 1)[0]
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
        self.assertIn("specialvar VAR_RESULT, WayfarerCanUseRegularAqua", vermilion)
        self.assertIn("checkitem ITEM_SS_TICKET", vermilion)
        self.assertIn("WayfarerPrepareHoennEntry", vermilion)
        self.assertIn("MAP_SLATEPORT_CITY_HARBOR", vermilion)

        slateport = script("SlateportCity_Harbor").split("WayfarerHoennEntry_EventScript_SlateportAquaAttendant::", 1)[1]
        self.assertIn("specialvar VAR_RESULT, WayfarerCanUseRegularAqua", slateport)
        self.assertIn("checkitem ITEM_SS_TICKET", slateport)
        self.assertIn("MAP_OLIVINE_CITY_PORT_INSIDE_HNS", slateport)

    def test_league_clear_continuations_preserve_the_interregional_network(self):
        clear = (GAME / "src/post_battle_event_funcs.c").read_text()
        self.assertIn("SetContinueGameWarpToHealLocation(HEAL_LOCATION_INDIGO_PLATEAU_HNS)", clear)
        self.assertIn("SetContinueGameWarpToHealLocation(HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE)", clear)
        self.assertIn("LeagueCircuit_CommitAndRegisterIndigo", (GAME / "data/scripts/wayfarer_indigo_league.inc").read_text())
        self.assertIn("LeagueCircuit_CommitRunClear", (GAME / "data/scripts/wayfarer_masters_league.inc").read_text())
        self.assertIn("LeagueCircuit_CommitRunClear", script("EverGrandeCity_HallOfFame"))

        indigo = json.loads((GAME / "data/maps/IndigoPlateau_PokemonCenter_hns/map.json").read_text())
        self.assertIn("MAP_INDIGO_PLATEAU_HNS", {warp["dest_map"] for warp in indigo["warp_events"]})
        ever_grande = json.loads((GAME / "data/maps/EverGrandeCity_PokemonLeague_1F/map.json").read_text())
        self.assertIn("MAP_EVER_GRANDE_CITY", {warp["dest_map"] for warp in ever_grande["warp_events"]})


if __name__ == "__main__":
    unittest.main()
