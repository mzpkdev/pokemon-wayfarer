import json
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]


def read(relative_path: str) -> str:
    return (ROOT / relative_path).read_text(encoding="utf-8")


def label_body(source: str, label: str) -> str:
    match = re.search(rf"(?m)^{re.escape(label)}::?\s*$", source)
    if match is None:
        raise AssertionError(f"missing script label {label}")
    following = source[match.end() :]
    next_label = re.search(r"(?m)^\S[^\n:]*::?\s*$", following)
    return following if next_label is None else following[: next_label.start()]


def assert_order(test: unittest.TestCase, source: str, *needles: str) -> None:
    position = -1
    for needle in needles:
        next_position = source.find(needle, position + 1)
        test.assertNotEqual(next_position, -1, f"missing {needle!r}")
        test.assertGreater(next_position, position, f"{needle!r} is out of order")
        position = next_position


class EmeraldTraversalContractTests(unittest.TestCase):
    def test_new_game_initializes_only_traversal_hide_state(self):
        source = label_body(read("data/scripts/new_game.inc"), "EventScript_ResetAllMapFlags")
        for flag in (
            "FLAG_HIDE_ROUTE_110_TEAM_AQUA",
            "FLAG_HIDE_ROUTE_119_TEAM_AQUA",
            "FLAG_HIDE_MT_CHIMNEY_TEAM_AQUA",
            "FLAG_HIDE_MT_CHIMNEY_TEAM_MAGMA",
        ):
            self.assertIn(f"setflag {flag}", source)
        for forbidden in (
            "FLAG_DELIVERED_DEVON_GOODS",
            "FLAG_MET_ARCHIE_METEOR_FALLS",
            "FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY",
            "FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE",
        ):
            self.assertNotIn(forbidden, source)

    def test_public_ferry_is_three_stop_and_campaign_state_free(self):
        route104 = read("data/maps/Route104/scripts.inc")
        ferry = route104[
            route104.index("Route104_EventScript_PublicFerryDeckhand::") :
            route104.index("Route104_EventScript_ShowOrHideWhiteHerbFlorist::")
        ]
        for choice in range(4):
            self.assertIn(f"case {choice}, Route104_EventScript_PublicFerry", ferry)
        for warp in (
            "warp MAP_ROUTE104, 14, 52",
            "warp MAP_DEWFORD_TOWN, 13, 10",
            "warp MAP_ROUTE109, 22, 25",
        ):
            self.assertIn(warp, ferry)
        self.assertEqual(ferry.count("turnobject LOCALID_PLAYER, DIR_NORTH"), 3)
        self.assertEqual(ferry.count("setflag FLAG_VISITED_SLATEPORT_CITY"), 1)
        for forbidden in (
            "EventScript_BackupMrBrineyLocation",
            "VAR_BRINEY_LOCATION",
            "VAR_BOARD_BRINEY_BOAT_STATE",
            "FLAG_DELIVERED_STEVEN_LETTER",
            "FLAG_DELIVERED_DEVON_GOODS",
            "FLAG_RESCUED_MR_BRINEY",
        ):
            self.assertNotIn(forbidden, ferry)

        for path, current_stop in (
            ("data/maps/Route104/scripts.inc", 0),
            ("data/maps/DewfordTown/scripts.inc", 1),
            ("data/maps/Route109/scripts.inc", 2),
        ):
            source = read(path)
            self.assertIn(f"setvar VAR_0x8004, {current_stop}", source)
            self.assertIn("goto Route104_EventScript_PublicFerryMenu", source)

        common = read("data/event_scripts.s")
        presentation_begin = label_body(
            common, "Common_EventScript_BeginPublicFerrySailingPresentation"
        )
        presentation_end = label_body(
            common, "Common_EventScript_EndPublicFerrySailingPresentation"
        )
        assert_order(
            self,
            presentation_begin,
            "closemessage",
            "fadescreen FADE_TO_BLACK",
            "call Common_EventScript_PlayBrineysBoatMusic",
        )
        assert_order(
            self,
            presentation_end,
            "fadescreen FADE_TO_BLACK",
            "call Common_EventScript_StopBrineysBoatMusic",
        )

        presentations = (
            (
                "data/maps/Route104/scripts.inc",
                "Route104_EventScript_PublicFerrySailingPresentation",
                "LOCALID_ROUTE104_BOAT",
                "MAP_ROUTE104",
                "Route104_Movement_SailToDewford",
                "setobjectxy LOCALID_PLAYER, 12, 53",
                "MAP_DEWFORD_TOWN",
            ),
            (
                "data/maps/DewfordTown/scripts.inc",
                "DewfordTown_EventScript_PublicFerrySailingPresentationToRoute104",
                "LOCALID_DEWFORD_BOAT",
                "MAP_DEWFORD_TOWN",
                "DewfordTown_Movement_SailToPetalburg",
                "setobjectxy LOCALID_PLAYER, 12, 8",
                "MAP_ROUTE104",
            ),
            (
                "data/maps/DewfordTown/scripts.inc",
                "DewfordTown_EventScript_PublicFerrySailingPresentationToSlateport",
                "LOCALID_DEWFORD_BOAT",
                "MAP_DEWFORD_TOWN",
                "DewfordTown_Movement_SailToSlateport",
                "setobjectxy LOCALID_PLAYER, 12, 8",
                "MAP_ROUTE109",
            ),
            (
                "data/maps/Route109/scripts.inc",
                "Route109_EventScript_PublicFerrySailingPresentation",
                "LOCALID_ROUTE109_BOAT",
                "MAP_ROUTE109",
                "Route109_Movement_SailToDewford",
                "setobjectxy LOCALID_PLAYER, 21, 25",
                "MAP_DEWFORD_TOWN",
            ),
        )
        campaign_free = presentation_begin + presentation_end
        for path, label, boat, source_map, movement, player_position, endpoint_map in presentations:
            presentation = label_body(read(path), label)
            assert_order(
                self,
                presentation,
                "call Common_EventScript_BeginPublicFerrySailingPresentation",
                f"addobject {boat}",
                f"setobjectxy {boat}",
                player_position,
                f"hideobjectat LOCALID_PLAYER, {source_map}",
                "fadescreen FADE_FROM_BLACK",
                f"applymovement {boat}, {movement}",
                f"applymovement LOCALID_PLAYER, {movement}",
                "waitmovement 0",
                "call Common_EventScript_EndPublicFerrySailingPresentation",
                f"showobjectat LOCALID_PLAYER, {endpoint_map}",
            )
            campaign_free += presentation

        self.assertNotRegex(
            campaign_free,
            r"EventScript_BackupMrBrineyLocation|VAR_BRINEY_LOCATION|"
            r"VAR_BOARD_BRINEY_BOAT_STATE|FLAG_DELIVERED_STEVEN_LETTER|"
            r"FLAG_DELIVERED_DEVON_GOODS|FLAG_HIDE_.*(?:BRINEY|PEEKO)",
        )
        self.assertNotRegex(campaign_free, r"(?m)^\s*(?:setflag|clearflag|setvar|copyvar)\b")

    def test_lilycove_and_weather_institute_preserve_story_state(self):
        lilycove = read("data/maps/LilycoveCity/scripts.inc")
        self.assertNotIn("METATILE_Lilycove_Wailmer", lilycove)
        self.assertNotIn("LilycoveCity_OnLoad:", lilycove)

        route119 = label_body(read("data/maps/Route119/scripts.inc"), "Route119_OnTransition")
        self.assertIn(
            "call_if_eq VAR_WEATHER_INSTITUTE_STATE, 0, Route119_EventScript_HideBridgeAquaGrunts",
            route119,
        )
        for floor in ("1F", "2F"):
            source = read(f"data/maps/Route119_WeatherInstitute_{floor}/scripts.inc")
            transition = label_body(source, f"Route119_WeatherInstitute_{floor}_OnTransition")
            self.assertIn("VAR_WEATHER_INSTITUTE_STATE, 0", transition)
            self.assertIn("ShowTeamAqua", transition)
            self.assertIn("clearflag FLAG_HIDE_ROUTE_119_TEAM_AQUA", source)
        shelly = read("data/maps/Route119_WeatherInstitute_2F/scripts.inc")
        self.assertIn("setflag FLAG_HIDE_ROUTE_119_TEAM_AQUA", shelly)

    def test_mt_chimney_visibility_is_derived_from_local_story_flags(self):
        source = read("data/maps/MtChimney/scripts.inc")
        transition = label_body(source, "MtChimney_OnTransition")
        self.assertIn("call MtChimney_EventScript_DeriveTeamVisibility", transition)
        derive = label_body(source, "MtChimney_EventScript_DeriveTeamVisibility")
        assert_order(
            self,
            derive,
            "FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY",
            "MtChimney_EventScript_HideTeamConflict",
            "FLAG_MET_ARCHIE_METEOR_FALLS",
            "MtChimney_EventScript_ShowTeamConflict",
        )
        hide = label_body(source, "MtChimney_EventScript_HideTeamConflict")
        show = label_body(source, "MtChimney_EventScript_ShowTeamConflict")
        for team in ("AQUA", "MAGMA"):
            self.assertIn(f"setflag FLAG_HIDE_MT_CHIMNEY_TEAM_{team}", hide)
            self.assertIn(f"clearflag FLAG_HIDE_MT_CHIMNEY_TEAM_{team}", show)

        meteor_falls = label_body(
            read("data/maps/MeteorFalls_1F_1R/scripts.inc"),
            "MeteorFalls_1F_1R_EventScript_MagmaStealsMeteoriteScene",
        )
        assert_order(
            self,
            meteor_falls,
            "setflag FLAG_MET_ARCHIE_METEOR_FALLS",
            "clearflag FLAG_HIDE_MT_CHIMNEY_TEAM_AQUA",
            "clearflag FLAG_HIDE_MT_CHIMNEY_TEAM_MAGMA",
        )

    def test_survey_flags_are_emerald_only_and_keep_cross_build_meanings(self):
        emerald = read("include/constants/flags.h")
        for name, value in (
            ("FLAG_ROUTE_111_DESERT_SURVEY_STARTED", "0x264"),
            ("FLAG_ROUTE_111_DESERT_SURVEY_STAKE_1", "0x265"),
            ("FLAG_ROUTE_111_DESERT_SURVEY_STAKE_2", "0x266"),
            ("FLAG_ROUTE_111_DESERT_SURVEY_STAKE_3", "0x267"),
        ):
            self.assertRegex(emerald, rf"#define\s+{name}\s+{value}\b")

        frlg = read("include/constants/flags_frlg.h")
        for name, value in (
            ("FLAG_FOUND_BOTH_VERMILION_GYM_SWITCHES", "0x264"),
            ("FLAG_CINNABAR_GYM_QUIZ_1", "0x265"),
            ("FLAG_PENDING_DAYCARE_EGG", "0x266"),
            ("FLAG_CINNABAR_GYM_QUIZ_2", "0x267"),
        ):
            self.assertRegex(frlg, rf"#define\s+{name}\s+{value}\b")

        hns = read("include/constants/flags_hns.h")
        for name, value in (
            ("FLAG_NO_SHINY", "0x264"),
            ("FLAG_MOM_VISITED", "0x265"),
            ("FLAG_PENDING_DAYCARE_EGG", "0x266"),
            ("FLAG_SET_WALL_CLOCK", "0x267"),
        ):
            self.assertRegex(hns, rf"#define\s+{name}\s+{value}\b")

        survey = read("data/maps/Route111/scripts.inc")
        self.assertIn("#if !IS_FRLG && !IS_HNS", survey)
        self.assertIn("#else\nRoute111_EventScript_DesertSurveyor::", survey)

    def test_survey_and_lavaridge_deliver_go_goggles_before_commit(self):
        route111 = read("data/maps/Route111/scripts.inc")
        for label in (
            "Route111_EventScript_DesertSurveyGiveReward",
            "Route111_EventScript_DesertSurveyRestoreReward",
        ):
            body = label_body(route111, label)
            assert_order(
                self,
                body,
                "additem ITEM_GO_GOGGLES",
                "goto_if_eq VAR_RESULT, FALSE",
                "call EventScript_ObtainItemMessage",
                "setflag FLAG_RECEIVED_GO_GOGGLES",
            )
            self.assertNotIn("giveitem ITEM_GO_GOGGLES", body)

        surveyor = label_body(route111, "Route111_EventScript_DesertSurveyor")
        self.assertIn("checkitem ITEM_GO_GOGGLES", surveyor)
        self.assertIn("FLAG_RECEIVED_GO_GOGGLES", surveyor)
        for stake in range(1, 4):
            source = label_body(route111, f"Route111_EventScript_DesertSurveyStake{stake}")
            self.assertIn("FLAG_ROUTE_111_DESERT_SURVEY_STARTED", source)
            self.assertIn(f"FLAG_ROUTE_111_DESERT_SURVEY_STAKE_{stake}", source)

        lavaridge = read("data/maps/LavaridgeTown/scripts.inc")
        for label in (
            "LavaridgeTown_EventScript_MayGiveGoGoggles",
            "LavaridgeTown_EventScript_BrendanGiveGoGoggles",
        ):
            body = label_body(lavaridge, label)
            self.assertIn("checkitem ITEM_GO_GOGGLES", body)
            assert_order(
                self,
                body,
                "additem ITEM_GO_GOGGLES",
                "goto_if_eq VAR_RESULT, FALSE",
                "call EventScript_ObtainItemMessage",
                "setflag FLAG_RECEIVED_GO_GOGGLES",
            )
        failed = label_body(lavaridge, "LavaridgeTown_EventScript_GoGogglesBagFull")
        self.assertNotIn("FLAG_RECEIVED_GO_GOGGLES", failed)
        self.assertIn("setvar VAR_LAVARIDGE_TOWN_STATE, 3", failed)
        reset = label_body(lavaridge, "LavaridgeTown_EventScript_ResetPendingGoGoggles")
        self.assertIn("setvar VAR_LAVARIDGE_TOWN_STATE, 1", reset)

    def test_route120_routes_every_outcome_through_one_transaction(self):
        source = read("data/maps/Route120/scripts.inc")
        steven = label_body(source, "Route120_EventScript_Steven")
        self.assertIn("checkitem ITEM_DEVON_SCOPE", steven)
        self.assertIn("Route120_EventScript_RepairBridgeCompletion", steven)

        preflight = label_body(source, "Route120_EventScript_StevenTryBattleKecleon")
        self.assertIn("checkitemspace ITEM_DEVON_SCOPE", preflight)
        self.assertIn("Route120_EventScript_StevenNeedsBagSpace", preflight)

        battle = label_body(source, "Route120_EventScript_StevenBattleKecleon")
        assert_order(
            self,
            battle,
            "B_OUTCOME_LOST",
            "B_OUTCOME_DREW",
            "B_OUTCOME_FORFEITED",
            "goto Route120_EventScript_CommitBridgeCompletion",
        )
        for success in (
            "B_OUTCOME_WON",
            "B_OUTCOME_RAN",
            "B_OUTCOME_PLAYER_TELEPORTED",
            "B_OUTCOME_MON_FLED",
            "B_OUTCOME_CAUGHT",
            "B_OUTCOME_MON_TELEPORTED",
        ):
            self.assertNotIn(success, battle)
        self.assertNotIn("FLAG_SYS_CTRL_OBJ_DELETE", battle)

        commit = label_body(source, "Route120_EventScript_CommitBridgeCompletion")
        assert_order(
            self,
            commit,
            "additem ITEM_DEVON_SCOPE",
            "goto_if_eq VAR_RESULT, FALSE",
            "call EventScript_ObtainItemMessage",
            "setflag FLAG_RECEIVED_DEVON_SCOPE",
            "call Route120_EventScript_SetBridgeObjectHideFlags",
            "removeobject LOCALID_BRIDGE_KECLEON",
            "removeobject LOCALID_BRIDGE_KECLEON_SHADOW",
            "removeobject LOCALID_ROUTE120_STEVEN",
            "call Route120_EventScript_SetBridgeClearMetatiles",
        )
        self.assertNotIn("giveitem ITEM_DEVON_SCOPE", commit)

        failed = label_body(source, "Route120_EventScript_StevenRewardDeliveryFailed")
        self.assertNotIn("FLAG_RECEIVED_DEVON_SCOPE", failed)
        self.assertIn("Route120_EventScript_RestoreBridgeKecleon", failed)
        repaired = label_body(source, "Route120_EventScript_RepairBridgeCompletion")
        self.assertNotIn("additem ITEM_DEVON_SCOPE", repaired)
        assert_order(
            self,
            repaired,
            "setflag FLAG_RECEIVED_DEVON_SCOPE",
            "call Route120_EventScript_SetBridgeObjectHideFlags",
            "removeobject LOCALID_ROUTE120_STEVEN",
            "call Route120_EventScript_SetBridgeClearMetatiles",
        )

    def test_early_settlement_arrival_is_baseline_only(self):
        transitions = {
            "FortreeCity": "FLAG_VISITED_FORTREE_CITY",
            "LilycoveCity": "FLAG_VISITED_LILYCOVE_CITY",
            "MossdeepCity": "FLAG_VISITED_MOSSDEEP_CITY",
            "PacifidlogTown": "FLAG_VISITED_PACIFIDLOG_TOWN",
        }
        forbidden = re.compile(
            r"(?:set|clear)(?:flag|var) (?:FLAG|VAR)_(?:DEFEATED|BADGE|RECEIVED_HM|TEAM_|MAGMA|AQUA|RIVAL|KYOGRE|GROUDON|RAYQUAZA)"
        )
        for map_name, visited_flag in transitions.items():
            source = read(f"data/maps/{map_name}/scripts.inc")
            body = label_body(source, f"{map_name}_OnTransition")
            self.assertIn(f"setflag {visited_flag}", body)
            self.assertIsNone(forbidden.search(body), map_name)

    def test_native_surf_coverage_remains_on_the_stacked_standard_rod_contract(self):
        config = json.loads(read("src/data/standard_rod_fishing.json"))
        emerald = {
            row["baseLabel"]: (row["species"], row["expectedOldRodSuccessfulEncounterPercent"])
            for row in config["nativeSurfAccessibility"]
            if row["product"] == "EMERALD"
        }
        self.assertEqual(
            emerald,
            {
                "gLilycoveCity": ("SPECIES_WAILMER", 19),
                "gMossdeepCity": ("SPECIES_WAILMER", 18),
                "gPacifidlogTown": ("SPECIES_WAILMER", 18),
            },
        )


if __name__ == "__main__":
    unittest.main()
