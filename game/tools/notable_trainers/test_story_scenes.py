"""The story scenes keep notable trainers continuous (.product/specs/notable-story-continuity.md).

Static checks over the event scripts: each converted scene branches on the introduced bit through
`notable_introduce`, each history claim sits behind the flag that proves it, and each story beat is paid from
exactly one place. The mechanics behind them (the introduced bit, the greeting, a beat paying once) are tested
in test/notable_friendship.c.
"""

import importlib.util
import re
import unittest
from pathlib import Path

from wayfarer_view import wayfarer_view

GAME = Path(__file__).resolve().parents[2]
MAPS = GAME / "data/maps"
BEATS = GAME / "include/constants/notable_story_beats.h"
_spec = importlib.util.spec_from_file_location("johto_story_test_scripts", GAME / "tools/johto_story/test_scripts.py")
_johto = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_johto)
INTRODUCE = re.compile(r"^\s*notable_introduce\s+(NOTABLE_TRAINER_\w+)\s*,\s*(\w+)", re.M)


def script(name):
    return wayfarer_view((MAPS / name / "scripts.inc").read_text())


def label_body(text, label):
    """The lines from a script or text label up to the next label."""
    match = re.search(rf"^{re.escape(label)}::?\s*\n((?:(?!^[A-Za-z_]\w*::?\s*$).*\n?)*)", text, re.M)
    assert match, f"no label {label}"
    return match.group(1)


def introductions(text):
    return INTRODUCE.findall(text)


class IntroductionTests(unittest.TestCase):
    def test_lance_introduces_himself_at_the_lake_only_when_not_met(self):
        lake = script("LakeOfRage_hns")
        self.assertIn(("NOTABLE_TRAINER_LANCE", "LakeOfRage_Text_LanceIntro1"), introductions(lake))
        # The restored first-meeting text is the one that names him, and the scene's shared text follows it.
        self.assertIn("I'm LANCE", label_body(lake, "LakeOfRage_Text_LanceIntro1"))
        scene = label_body(lake, "LakeOfRage_EventScript_Lance")
        self.assertLess(scene.index("LakeOfRage_Text_LanceIntro1"), scene.index("LakeOfRage_Text_LanceIntro2"))

    def test_lance_at_indigo_does_not_assume_a_meeting(self):
        champions = script("PokemonLeague_ChampionsRoom_hns")
        self.assertIn(("NOTABLE_TRAINER_LANCE", "PokemonLeague_ChampionsRoom_Text_FirstMeetingIntro"),
                      introductions(champions))
        # Wayfarer plays the new first-meeting text; the original line survives only for non-Wayfarer builds.
        self.assertNotIn("eventually reach me here", label_body(champions, "PokemonLeague_ChampionsRoom_Text_BattleIntro"))

    def test_whitney_introduces_herself_once_across_tower_and_gym(self):
        tower = script("GoldenrodCity_RadioTower_1F_hns")
        gym = script("GoldenrodCity_Gym_hns")
        for text, label in ((tower, "RadioTower1F_EventScript_Whitney"), (gym, "GoldenrodCity_Gym_EventScript_Whitney")):
            self.assertIn("notable_introduce NOTABLE_TRAINER_WHITNEY", label_body(text, label))
        # Neither shared remainder names her again.
        for text, label in ((tower, "RadioTower1F_Text_Whitney1"), (gym, "GoldenrodCity_Gym_Text_WhitneyBefore")):
            self.assertNotRegex(label_body(text, label), r"(?i)my name is|I'm WHITNEY")

    def test_steven_names_himself_at_the_space_center_and_in_granite_cave(self):
        for map_name in ("MossdeepCity_SpaceCenter_1F", "MossdeepCity_SpaceCenter_2F", "GraniteCave_StevensRoom"):
            self.assertTrue(any(c == "NOTABLE_TRAINER_STEVEN" for c, _ in introductions(script(map_name))), map_name)

    def test_every_gym_and_league_self_introduction_is_an_introduction(self):
        for map_name, character in (
            ("AzaleaTown_Gym_hns", "BUGSY"), ("BlackthornCity_Gym_hns", "CLAIR"), ("VioletCity_Gym_hns", "FALKNER"),
            ("PewterCity_Gym_hns", "BROCK"), ("CeladonCity_Gym_hns", "ERIKA"), ("OlivineCity_Gym_hns", "JASMINE"),
            ("DewfordTown_Gym", "BRAWLY"), ("FortreeCity_Gym", "WINONA"), ("RustboroCity_Gym", "ROXANNE"),
            ("LavaridgeTown_Gym_1F", "FLANNERY"), ("PetalburgCity_Gym", "NORMAN"), ("Route25_hns", "MISTY"),
            ("VermilionCity_hns", "STEVEN"), ("PokemonTower_2F_Frlg", "BLUE"), ("SSAnne_2F_Corridor_Frlg", "BLUE"),
            ("CaveOfOrigin_B1F", "WALLACE"),
        ):
            self.assertIn(f"NOTABLE_TRAINER_{character}", [c for c, _ in introductions(script(map_name))], map_name)

    def test_the_league_circuit_host_is_introduced_by_the_room_host(self):
        circuit = (GAME / "data/scripts/league_circuit.inc").read_text()
        for script_name in ("RoomIntro", "ChampionIntro", "RoomOffer", "ChampionOffer"):
            self.assertIn("call LeagueEvent_EventScript_IntroduceHost", label_body(circuit, f"LeagueEvent_EventScript_{script_name}"))
        self.assertIn("LeagueEvent_GetRoomHostCharacter", label_body(circuit, "LeagueEvent_EventScript_IntroduceHost"))

    def test_blue_is_not_introduced_to_his_pallet_rival(self):
        talk = (GAME / "data/scripts/wayfarer_notable_talk.inc").read_text()
        body = label_body(talk, "NotableStory_EventScript_IntroduceBlue")
        self.assertIn("WayfarerKanto_IsPalletOrigin", body)
        ss_anne = script("SSAnne_2F_Corridor_Frlg")
        self.assertIn("WayfarerKanto_IsPalletOrigin", ss_anne)


class HistoryClaimTests(unittest.TestCase):
    def test_steven_recalls_the_cave_near_dewford_only_with_the_letter_delivered(self):
        route = script("Route118")
        trigger = label_body(route, "Route118_EventScript_StevenTrigger")
        self.assertLess(trigger.index("goto_if_set FLAG_DELIVERED_STEVEN_LETTER, Route118_EventScript_StevenRecallsCave"),
                        trigger.index("Route118_Text_StevenIntroducesHimself"))
        # The cave line lives in one text, reached only from the recall branch.
        self.assertIn("cave near DEWFORD", label_body(route, "Route118_Text_StevenMetInCave"))
        for label in ("Route118_Text_StevenIntroducesHimself", "Route118_Text_StevenQuestions"):
            self.assertNotIn("DEWFORD", label_body(route, label))
        self.assertEqual(route.count("Route118_Text_StevenMetInCave"), 2)  # the recall msgbox and the label

    def test_steven_in_meteor_falls_recalls_only_what_happened(self):
        cave = script("MeteorFalls_StevensCave")
        self.assertIn("call_if_ge VAR_MOSSDEEP_SPACE_CENTER_STATE, 3, MeteorFalls_StevensCave_EventScript_RecallSpaceCenter", cave)
        self.assertIn("goto_if_set FLAG_DELIVERED_STEVEN_LETTER", label_body(cave, "MeteorFalls_StevensCave_EventScript_MyPredictionCameTrue"))
        self.assertIn("SPACE CENTER", label_body(cave, "MeteorFalls_StevensCave_Text_BattledAtSpaceCenter"))
        # What he says of his own strength rides on the Space Center text, so it is told only with that flag.
        self.assertIn("how good I am", label_body(cave, "MeteorFalls_StevensCave_Text_BattledAtSpaceCenter"))
        self.assertNotIn("how good I am", label_body(cave, "MeteorFalls_StevensCave_Text_ExpectWorst"))
        self.assertNotIn("SPACE CENTER", label_body(cave, "MeteorFalls_StevensCave_Text_ExpectWorst"))
        self.assertIn("MOSSDEEP SPACE CENTER", label_body(cave, "MeteorFalls_StevensCave_Text_BattledAtSpaceCenter"))
        self.assertNotIn("GRANITE", label_body(cave, "MeteorFalls_StevensCave_Text_PredictionFromHearingOfYou"))

    def test_steven_at_vermilion_no_longer_calls_himself_the_hoenn_champion(self):
        self.assertNotIn("The Champion from the HOENN region", script("VermilionCity_hns"))

    def test_chucks_wife_says_he_lost_only_with_his_badge(self):
        city = script("CianwoodCity_hns")
        has_fly = label_body(city, "CianwoodCity_EventScript_ChucksWifeAlreadyHasFly")
        self.assertLess(has_fly.index("goto_if_unset FLAG_BADGE05_GET"), has_fly.index("CianwoodCity_Text_ChucksWifeChubby"))

    def test_norman_is_dad_only_to_his_own_child(self):
        route = script("Route104")
        calls = label_body(route, "Route104_EventScript_SailToDewfordDadCalls")
        self.assertLess(calls.index("WayfarerUsesNativeHoennHousehold"), calls.index("Route104_Text_DadPokenavCall"))
        self.assertIn("NORMAN", label_body(script("Route105"), "Route104_Text_RegisteredNormanInPokenav"))
        match_call = (GAME / "src/pokenav_match_call_data.c").read_text()
        self.assertIn("MatchCall_IsNormanVisitorCall", match_call)
        texts = (GAME / "data/text/match_call.inc").read_text()
        for number in range(1, 10):
            self.assertNotIn("DAD", label_body(texts, f"MatchCall_Text_NormanVisitor{number}"))
        self.assertNotIn("how much you've grown", script("PetalburgCity_Gym"))

    def test_clair_gives_no_johto_route_to_the_league(self):
        for map_name in ("BlackthornCity_Gym_hns", "DragonsDen_Cavern_hns"):
            self.assertNotIn("NEW BARK TOWN", script(map_name), map_name)

    def test_route_trainers_do_not_claim_a_badge(self):
        self.assertNotIn("You beat FALKNER", script("Route32_hns"))

    def test_misty_reads_johto_badges_only_with_all_eight(self):
        reads = label_body(script("Route25_hns"), "Route25_EventScript_MistyReadsBadges")
        for badge in range(1, 9):
            self.assertIn(f"goto_if_unset FLAG_BADGE0{badge}_GET", reads)


class PresenceTests(unittest.TestCase):
    def test_whitney_is_at_the_tower_only_in_the_morning_until_the_quiz_is_solved(self):
        tower = script("GoldenrodCity_RadioTower_1F_hns")
        check = label_body(tower, "RadioTower1F_EventScript_CheckWhitneyAtTower")
        self.assertIn("NotableStory_IsWhitneyAtTower", check)
        notable = (GAME / "src/notable_story.c").read_text()
        self.assertIn("!FlagGet(FLAG_ENABLE_RADIO) && GetTimeOfDay() == TIME_MORNING", notable)
        self.assertIn(".characterId = NOTABLE_TRAINER_WHITNEY, .predicate", notable)
        # The Tower copy and the Gym copy hide on the same temp flag, opposite conditions.
        self.assertIn("call_if_eq VAR_RESULT, FALSE, RadioTower1F_EventScript_HideWhitney", tower)
        self.assertIn("call_if_eq VAR_RESULT, TRUE, GoldenrodCity_Gym_EventScript_HideWhitney", script("GoldenrodCity_Gym_hns"))
        # The quiz still ends without her.
        self.assertIn("goto_if_set FLAG_TEMP_1, RadioTower1F_EventScript_QuizDoneWithoutWhitney", tower)
        beauty = script("GoldenrodCity_hns")
        self.assertIn("RadioTower1F_EventScript_CheckWhitneyAtTower", beauty)


class BeatTests(unittest.TestCase):
    def test_every_story_beat_is_paid_from_exactly_one_place(self):
        beats = re.findall(r"#define (NOTABLE_STORY_BEAT_\w+)\s+(\d+)", BEATS.read_text())
        names = [name for name, value in beats if name not in ("NOTABLE_STORY_BEAT_NONE", "NOTABLE_STORY_BEAT_COUNT")]
        count = int(re.search(r"#define NOTABLE_STORY_BEAT_COUNT\s+(\d+)", BEATS.read_text()).group(1))
        self.assertEqual(len(names), count)
        self.assertEqual(sorted(int(value) for name, value in beats if name in names), list(range(1, count + 1)))
        self.assertNotIn("TATE_LIZA", BEATS.read_text())
        sources = [p for p in (GAME / "data").rglob("*.inc")]
        for name in names:
            sites = []
            for path in sources:
                for line in path.read_text(errors="replace").splitlines():
                    if re.search(rf"^\s*(?:notable_beat\s+{name}\b|setvar VAR_0x8004, {name}\b)", line):
                        sites.append(path.name)
            self.assertEqual(len(sites), 1, f"{name} is called from {sites}")


LANCE = "NOTABLE_TRAINER_LANCE"
LANCE_ACCEPT = "NOTABLE_STORY_BEAT_LANCE_LAKE_REQUEST"


class SceneRunTests(unittest.TestCase):
    """Run the scenes' scripts in both orders: met first, and not met first."""

    def play(self, maps, label, items=None, **kwargs):
        interp = _johto.Interp(*maps if isinstance(maps, tuple) else (maps,), **kwargs)
        if items is not None:
            interp.items = set(items)
        interp.run(label)
        return interp

    def test_lance_at_the_lake_introduces_himself_to_a_stranger_only(self):
        scene = "LakeOfRage_EventScript_Lance"
        stranger = self.play("LakeOfRage_hns", scene)
        self.assertIn("LakeOfRage_Text_LanceIntro1", stranger.messages)
        self.assertEqual(stranger.introduced, {LANCE})
        # He knew them first (the INDIGO fight, a walker talk): his greeting stands in for the introduction.
        met = self.play("LakeOfRage_hns", scene, introduced={LANCE})
        self.assertNotIn("LakeOfRage_Text_LanceIntro1", met.messages)
        self.assertIn("greeting:" + LANCE, met.messages)
        # Either way the scene's own text follows.
        for run in (stranger, met):
            self.assertIn("LakeOfRage_Text_LanceIntro2", run.messages)

    def test_a_declined_lake_request_pays_nothing_and_an_accepted_one_pays_once(self):
        declined = self.play("LakeOfRage_hns", "LakeOfRage_EventScript_Lance", answers=0)
        self.assertEqual(declined.beats_paid, [])
        accepted = self.play("LakeOfRage_hns", "LakeOfRage_EventScript_Lance")
        self.assertEqual(accepted.beats_paid, [LANCE_ACCEPT])
        # Asking again after a decline, then accepting, still pays the one beat.
        again = _johto.Interp("LakeOfRage_hns", vars={"VAR_MAHOGANY_TOWN_STATE": 3}, answers=[0, 1])
        again.run("LakeOfRage_EventScript_Lance_Return")
        self.assertEqual(again.beats_paid, [])
        again.run("LakeOfRage_EventScript_Lance_Return")
        again.run("LakeOfRage_EventScript_LanceAccept")
        self.assertEqual(again.beats_paid, [LANCE_ACCEPT])

    def test_whitney_introduces_herself_once_whichever_place_comes_first(self):
        whitney = "NOTABLE_TRAINER_WHITNEY"
        tower_text, gym_text = "RadioTower1F_Text_WhitneyIntro", "GoldenrodCity_Gym_Text_WhitneyIntro"
        for first, second in (((("GoldenrodCity_RadioTower_1F_hns",), "RadioTower1F_EventScript_Whitney", tower_text),
                               (("GoldenrodCity_Gym_hns",), "GoldenrodCity_Gym_EventScript_Whitney", gym_text)),
                              ((("GoldenrodCity_Gym_hns",), "GoldenrodCity_Gym_EventScript_Whitney", gym_text),
                               (("GoldenrodCity_RadioTower_1F_hns",), "RadioTower1F_EventScript_Whitney", tower_text))):
            known = set()
            one = self.play(first[0], first[1], introduced=known)
            two = self.play(second[0], second[1], introduced=known)
            self.assertIn(first[2], one.messages)
            self.assertNotIn(second[2], two.messages)
            self.assertIn("greeting:" + whitney, two.messages)

    def test_steven_on_route_118_recalls_the_cave_only_with_the_letter(self):
        trigger = "Route118_EventScript_StevenTrigger"
        without = self.play("Route118", trigger, vars={"VAR_0x8008": 0})
        self.assertNotIn("Route118_Text_StevenMetInCave", without.messages)
        self.assertEqual(without.messages[0], "Route118_Text_StevenIntroducesHimself")
        self.assertEqual(without.messages[-1], "Route118_Text_StevenQuestions")
        # A player who delivered the letter has met him: the cave line, no introduction.
        delivered = self.play("Route118", trigger, vars={"VAR_0x8008": 0}, flags={"FLAG_DELIVERED_STEVEN_LETTER"})
        self.assertEqual(delivered.messages, ["Route118_Text_StevenMetInCave", "Route118_Text_StevenQuestions"])
        # Known some other way, a stranger-to-the-cave player still gets no cave line.
        known = self.play("Route118", trigger, vars={"VAR_0x8008": 0}, introduced={"NOTABLE_TRAINER_STEVEN"})
        self.assertEqual(known.messages, ["greeting:NOTABLE_TRAINER_STEVEN", "Route118_Text_StevenQuestions"])

    def test_chucks_wife_says_he_lost_only_when_he_did(self):
        wife = "CianwoodCity_EventScript_ChucksWife"
        # HM FLY from HOENN, no CIANWOOD badge: nothing about a battle with her husband.
        fly_only = self.play("CianwoodCity_hns", wife, items={"ITEM_HM_FLY"})
        self.assertEqual(fly_only.messages, ["CianwoodCity_Text_ChucksWifeAlreadyCanFly"])
        beaten = self.play("CianwoodCity_hns", wife, items={"ITEM_HM_FLY"}, flags={"FLAG_BADGE05_GET"})
        self.assertEqual(beaten.messages, ["CianwoodCity_Text_ChucksWifeChubby"])
        # Without FLY the plain gift stays as it was: offered after the badge only.
        gift = self.play("CianwoodCity_hns", wife, items=set(), flags={"FLAG_BADGE05_GET"})
        self.assertIn("CianwoodCity_Text_ChucksWifeGiveHM", gift.messages)
        self.assertIn("ITEM_HM_FLY", gift.given)

    def test_steven_in_the_cave_greets_a_stranger_without_expecting_them(self):
        steven = "NOTABLE_TRAINER_STEVEN"
        scene = "MeteorFalls_StevensCave_EventScript_Steven"
        stranger = self.play("MeteorFalls_StevensCave", scene)
        self.assertEqual(stranger.messages[:2], ["MeteorFalls_StevensCave_Text_StevenIntro",
                                                 "MeteorFalls_StevensCave_Text_RockManiacStranger"])
        self.assertNotIn("MeteorFalls_StevensCave_Text_ImAmazedYouFoundMe", stranger.messages)
        self.assertNotIn("MeteorFalls_StevensCave_Text_BattledAtSpaceCenter", stranger.messages)
        self.assertEqual(stranger.introduced, {steven})
        known = self.play("MeteorFalls_StevensCave", scene, introduced={steven})
        self.assertEqual(known.messages[:2], ["greeting:" + steven, "MeteorFalls_StevensCave_Text_ImAmazedYouFoundMe"])
        # The Space Center is recalled only by a player who fought beside him there.
        # It is keyed on the invasion's lasting state, not on the flag a later rival call clears.
        fought = self.play("MeteorFalls_StevensCave", scene, introduced={steven}, vars={"VAR_MOSSDEEP_SPACE_CENTER_STATE": 3})
        self.assertIn("MeteorFalls_StevensCave_Text_BattledAtSpaceCenter", fought.messages)
        unfinished = self.play("MeteorFalls_StevensCave", scene, introduced={steven}, vars={"VAR_MOSSDEEP_SPACE_CENTER_STATE": 2})
        self.assertNotIn("MeteorFalls_StevensCave_Text_BattledAtSpaceCenter", unfinished.messages)
        # His cave opens on HOENN's own FLAG_SYS_GAME_CLEAR, so the prediction of a CHAMPION always holds.
        self.assertIn("FLAG_SYS_GAME_CLEAR", (MAPS / "MeteorFalls_1F_1R/scripts.inc").read_text())

    def test_jasmine_gives_her_name_in_the_lighthouse_so_the_gym_greets(self):
        jasmine = "NOTABLE_TRAINER_JASMINE"
        first = self.play("OlivineCity_Lighthouse_hns", "OlivineLighthouse_EventScript_Jasmine")
        self.assertEqual(first.messages[0], "OlivineLighthouse_Text_JasmineName")
        self.assertEqual(first.introduced, {jasmine})
        # The medicine came before she asked: her name comes with the first words she says about AMPHY.
        medicine = self.play("OlivineCity_Lighthouse_hns", "OlivineLighthouse_EventScript_Jasmine",
                             vars={"VAR_OLIVINE_CITY_STATE": 4}, flags={"FLAG_WAYFARER_JOHTO_LIGHTHOUSE_MEDICINE_FIRST"})
        self.assertEqual(medicine.messages[0], "OlivineLighthouse_Text_JasmineName")
        # Met at the Lighthouse, the Gym greets in place of "allow me to introduce myself".
        gym = self.play("OlivineCity_Gym_hns", "OlivineCity_Gym_EventScript_Jasmine", introduced=first.introduced)
        self.assertNotIn("OlivineCity_Gym_Text_JasmineSteelTypeIntro", gym.messages)
        self.assertIn("greeting:" + jasmine, gym.messages)

    def test_blue_greets_a_player_on_the_islands_only_if_pallet_or_introduced(self):
        celio = (GAME / "data/scripts/wayfarer_sevii/story/celio_network.inc").read_text()
        for label in ("WayfarerSevii_FourIsland_RivalTryScene", "WayfarerSevii_SixIsland_RivalTryScene"):
            body = label_body(celio, label)
            self.assertLess(body.index("call WayfarerSevii_Rival_KnowsBlue"), body.index("lockall"), label)
        known = (label_body(celio, "WayfarerSevii_Rival_KnowsBlue")
                 + label_body(celio, "WayfarerSevii_Rival_KnowsBlueNotPallet"))
        self.assertIn("WayfarerKanto_IsPalletOrigin", known)
        self.assertIn("NotableStory_IsIntroduced", known)
        self.assertIn("NOTABLE_TRAINER_BLUE", known)

    def test_the_non_wayfarer_builds_keep_the_original_scenes(self):
        for name, label, kept, dropped in (
            ("Route32_hns", "Route32_Text_BirdKeeperPeter_Seen", "You beat FALKNER?", "TRAINER, aren't you"),
            ("BlackthornCity_Gym_hns", "BlackthornGym_Text_ClairText_League", "NEW BARK TOWN", "a long way"),
            ("DragonsDen_Cavern_hns", "DragonsDen_Cavern_Text_ClairWhatsTheMatter", "NEW BARK TOWN", "a long way"),
            ("VermilionCity_hns", "VermilionCity_Text_StevenIntro", "The Champion from the HOENN region", "For some time"),
            ("MeteorFalls_StevensCave", "MeteorFalls_StevensCave_Text_ShouldKnowHowGoodIAmExpectWorst",
             "SOOTOPOLIS SPACE CENTER", "MOSSDEEP"),
            ("Route25_hns", "Route25_Text_MistyDate", "JOHTO GYM BADGES", "TRAINER, right"),
            ("PewterCity_Gym_hns", "PewterCity_Gym_Text_Brock_Intro", "challenger from JOHTO", "real"),
            ("CeladonCity_Gym_hns", "CeladonCity_Gym_Text_ErikaBeforeBattle", "from JOHTO", "A traveler"),
        ):
            text = (MAPS / name / "scripts.inc").read_text()
            original = wayfarer_view(text, wayfarer=False)
            self.assertIn(kept, label_body(original, label), f"{name}: {label}")
            self.assertNotIn(dropped, label_body(original, label), f"{name}: {label}")
        # No notable_introduce, notable special or split intro text survives in the original arm of a shared map.
        for name in ("Route32_hns", "Route25_hns", "VermilionCity_hns", "MeteorFalls_StevensCave", "GraniteCave_StevensRoom",
                     "DewfordTown_Gym", "RustboroCity_Gym", "OlivineCity_Gym_hns", "BlackthornCity_Gym_hns"):
            original = wayfarer_view((MAPS / name / "scripts.inc").read_text(), wayfarer=False)
            self.assertNotIn("notable_introduce", original, name)

    def test_steven_at_granite_cave_does_not_reintroduce_a_known_steven(self):
        steven = "NOTABLE_TRAINER_STEVEN"
        first = self.play("GraniteCave_StevensRoom", "GraniteCave_StevensRoom_EventScript_Steven")
        self.assertEqual(first.messages[0], "GraniteCave_StevensRoom_Text_MyNameIsSteven")
        known = self.play("GraniteCave_StevensRoom", "GraniteCave_StevensRoom_EventScript_Steven", introduced={steven})
        self.assertEqual(known.messages[0], "greeting:" + steven)
        self.assertNotIn("GraniteCave_StevensRoom_Text_MyNameIsSteven", known.messages)
        # Delivering the letter is the beat; it pays on the handover.
        self.assertEqual(first.beats_paid, ["NOTABLE_STORY_BEAT_STEVEN_GRANITE_LETTER"])


if __name__ == "__main__":
    unittest.main()
