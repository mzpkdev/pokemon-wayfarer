"""Source-level contract checks for FRLG open-world regional traversal.

These tests intentionally inspect authored JSON and event scripts.  They do not
replace ROM or playthrough validation; they protect the exact static invariants
whose accidental regression would silently reintroduce a story roadblock or an
unsafe partial reward state.
"""

from __future__ import annotations

import json
import re
import unittest
from collections import deque
from pathlib import Path


GAME = Path(__file__).resolve().parents[3]
DATA = GAME / "data"
MAPS = DATA / "maps"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def load_map(name: str) -> dict:
    with (MAPS / name / "map.json").open(encoding="utf-8") as stream:
        return json.load(stream)


def at(events: list[dict], x: int, y: int, **fields: object) -> list[dict]:
    return [
        event
        for event in events
        if event.get("x") == x
        and event.get("y") == y
        and all(event.get(key) == value for key, value in fields.items())
    ]


def object_with_local_id(data: dict, local_id: str) -> dict:
    matches = [
        event
        for event in data["object_events"]
        if event.get("local_id") == local_id
    ]
    if len(matches) != 1:
        raise AssertionError(f"expected one {local_id} object, found {len(matches)}")
    return matches[0]


def uncommented(text: str) -> str:
    return "\n".join(line.split("@", 1)[0] for line in text.splitlines())


def frlg_guarded_lines(text: str) -> list[bool]:
    """Mark source lines inside the true arm of an ``#if IS_FRLG`` guard."""
    stack: list[tuple[bool, bool]] = []
    guarded: list[bool] = []
    for line in text.splitlines():
        stripped = line.strip()
        guarded.append(any(is_frlg and not in_else for is_frlg, in_else in stack))
        if re.fullmatch(r"#if\s+IS_FRLG", stripped):
            stack.append((True, False))
        elif stripped.startswith("#if"):
            stack.append((False, False))
        elif stripped.startswith("#else"):
            if not stack:
                raise AssertionError("unmatched #else in event scripts")
            is_frlg, in_else = stack.pop()
            stack.append((is_frlg, not in_else))
        elif stripped.startswith("#endif"):
            if not stack:
                raise AssertionError("unmatched #endif in event scripts")
            stack.pop()
    if stack:
        raise AssertionError("unterminated preprocessor guard in event scripts")
    return guarded


class ScriptIndex:
    """Index event-script labels and permit bounded reachability assertions."""

    LABEL = re.compile(r"(?m)^([A-Za-z_][A-Za-z0-9_]*)::\s*$")

    def __init__(self) -> None:
        self.blocks: dict[str, str] = {}
        self.paths: dict[str, Path] = {}
        for path in DATA.rglob("*.inc"):
            source = uncommented(read(path))
            matches = list(self.LABEL.finditer(source))
            for number, match in enumerate(matches):
                label = match.group(1)
                end = matches[number + 1].start() if number + 1 < len(matches) else len(source)
                self.blocks[label] = source[match.end() : end]
                self.paths[label] = path

    def block(self, label: str) -> str:
        if label not in self.blocks:
            raise AssertionError(f"script label {label} is not defined")
        return self.blocks[label]

    def reachable_labels(self, start: str, limit: int = 600) -> set[str]:
        if start not in self.blocks:
            raise AssertionError(f"script label {start} is not defined")
        found: set[str] = set()
        queue = deque([start])
        while queue:
            label = queue.popleft()
            if label in found:
                continue
            found.add(label)
            if len(found) > limit:
                raise AssertionError(f"script graph from {start} exceeded {limit} labels")
            for token in re.findall(r"\b[A-Za-z_][A-Za-z0-9_]*\b", self.blocks[label]):
                if token in self.blocks and token not in found:
                    queue.append(token)
        return found

    def reachable_text(self, start: str) -> str:
        return "\n".join(self.blocks[label] for label in self.reachable_labels(start))

    def has_ordered_path(self, start: str, patterns: list[str]) -> bool:
        """Return whether one label-graph path observes patterns in order."""
        compiled = [re.compile(pattern) for pattern in patterns]
        queue = deque([(start, 0)])
        seen: set[tuple[str, int]] = set()
        while queue:
            label, progress = queue.popleft()
            if (label, progress) in seen:
                continue
            seen.add((label, progress))
            body = self.block(label)
            cursor = 0
            while progress < len(compiled):
                match = compiled[progress].search(body, cursor)
                if match is None:
                    break
                cursor = match.end()
                progress += 1
            if progress == len(compiled):
                return True
            for token in re.findall(r"\b[A-Za-z_][A-Za-z0-9_]*\b", body):
                if token in self.blocks:
                    queue.append((token, progress))
        return False


class TraversalContractTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.scripts = ScriptIndex()

    def assertContains(self, text: str, pattern: str, message: str | None = None) -> None:
        self.assertRegex(text, re.compile(pattern, re.MULTILINE), message)

    def assertNotContains(self, text: str, pattern: str, message: str | None = None) -> None:
        self.assertNotRegex(text, re.compile(pattern, re.MULTILINE), message)

    def assertOrderedPath(self, start: str, patterns: list[str], message: str) -> None:
        self.assertTrue(self.scripts.has_ordered_path(start, patterns), message)

    def assertOrderedText(self, text: str, snippets: list[str], message: str) -> None:
        cursor = 0
        for snippet in snippets:
            position = text.find(snippet, cursor)
            self.assertNotEqual(position, -1, f"{message}: missing or out of order: {snippet}")
            cursor = position + len(snippet)

    def test_viridian_old_man_lane(self) -> None:
        data = load_map("ViridianCity_Frlg")
        triggers = data["coord_events"]
        self.assertFalse(at(triggers, 22, 11, var="VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN", var_value="0"))
        self.assertFalse(at(triggers, 22, 8, var="VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN", var_value="1"))
        kept = at(triggers, 20, 8, var="VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN", var_value="1")
        self.assertEqual([event["script"] for event in kept], ["ViridianCity_EventScript_TutorialTriggerLeft"])

        tutorial_man = object_with_local_id(data, "LOCALID_TUTORIAL_MAN")
        self.assertEqual(tutorial_man["script"], "ViridianCity_EventScript_TutorialOldMan")
        transition = self.scripts.reachable_text("ViridianCity_OnTransition")
        for value in (0, 1):
            self.assertContains(
                self.scripts.block("ViridianCity_OnTransition"),
                rf"call_if_eq\s+VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN,\s*{value},\s*([A-Za-z0-9_]+)",
            )
        self.assertGreaterEqual(transition.count("setobjectxyperm LOCALID_TUTORIAL_MAN, 21, 8"), 2)
        self.assertNotIn("setobjectxyperm LOCALID_TUTORIAL_MAN, 21, 11", transition)
        self.assertNotIn("OBJ_EVENT_GFX_OLD_MAN_LYING_DOWN", transition)
        self.assertGreaterEqual(transition.count("MOVEMENT_TYPE_LOOK_AROUND"), 2)

    def test_pewter_lower_lane_only_removes_two_triggers(self) -> None:
        data = load_map("PewterCity_Frlg")
        guide = object_with_local_id(data, "LOCALID_PEWTER_GYM_GUIDE")
        self.assertEqual((guide["x"], guide["y"]), (42, 20))
        triggers = data["coord_events"]
        retained = {
            (event["x"], event["y"], event["script"])
            for event in triggers
            if event.get("var") == "VAR_MAP_SCENE_PEWTER_CITY" and event.get("var_value") == "0"
        }
        self.assertIn((42, 21, "PewterCity_EventScript_GymGuideTriggerTop"), retained)
        self.assertIn((42, 22, "PewterCity_EventScript_GymGuideTriggerMid"), retained)
        self.assertFalse(at(triggers, 42, 23))
        self.assertFalse(at(triggers, 43, 23))

    def test_mt_moon_miguel_and_fossils_are_preserved_without_trigger(self) -> None:
        data = load_map("MtMoon_B2F_Frlg")
        self.assertFalse(at(data["coord_events"], 14, 11))
        expected = {
            "LOCALID_MIGUEL": (13, 11),
            "LOCALID_DOME_FOSSIL": (13, 7),
            "LOCALID_HELIX_FOSSIL": (14, 7),
        }
        for local_id, position in expected.items():
            obj = object_with_local_id(data, local_id)
            self.assertEqual((obj["x"], obj["y"]), position)
        miguel = self.scripts.reachable_text("MtMoon_B2F_EventScript_Miguel")
        self.assertIn("trainerbattle", miguel)
        self.assertIn("setvar VAR_MAP_SCENE_MT_MOON_B2F, 1", miguel)
        battle = self.scripts.block("MtMoon_B2F_EventScript_BattleMiguel")
        self.assertOrderedText(
            battle,
            ["trainerbattle_no_intro", "setvar VAR_MAP_SCENE_MT_MOON_B2F, 1"],
            "Miguel's scene may advance only after his original victory",
        )

    def test_cerulean_uses_base_nonblocking_positions(self) -> None:
        data = load_map("CeruleanCity_Frlg")
        positions = {
            "LOCALID_CERULEAN_POLICEMAN": (31, 12),
            "LOCALID_CERULEAN_SLOWBRO": (32, 29),
            "LOCALID_CERULEAN_LASS": (33, 29),
        }
        for local_id, expected in positions.items():
            obj = object_with_local_id(data, local_id)
            self.assertEqual((obj["x"], obj["y"]), expected)
        transition = self.scripts.block("CeruleanCity_OnTransition")
        self.assertNotIn("CeruleanCity_EventScript_BlockExits", transition)
        self.assertNotIn("FLAG_GOT_SS_TICKET", transition)

    def test_saffron_gates_allow_early_passage_and_late_tea_handoff(self) -> None:
        gates = {
            "Route5_SouthEntrance_Frlg": "Route5_SouthEntrance_EventScript_GuardTrigger",
            "Route6_NorthEntrance_Frlg": "Route6_NorthEntrance_EventScript_GuardTrigger",
            "Route7_EastEntrance_Frlg": "Route7_EastEntrance_EventScript_GuardTrigger",
            "Route8_WestEntrance_Frlg": "Route8_WestEntrance_EventScript_GuardTrigger",
        }
        for map_name, trigger in gates.items():
            with self.subTest(map=map_name):
                body = self.scripts.reachable_text(trigger)
                self.assertIn("checkitem ITEM_TEA", body)
                self.assertIn("removeitem ITEM_TEA", body)
                self.assertIn("setvar VAR_MAP_SCENE_ROUTE5_ROUTE6_ROUTE7_ROUTE8_GATES, 1", body)
                self.assertNotIn("setflag FLAG_GOT_TEA", body)
                self.assertNotIn("additem ITEM_TEA", body)
                direct = self.scripts.block(trigger)
                self.assertNotContains(direct, r"applymovement\s+LOCALID_PLAYER,\s*\w*BlockPlayerEntry")
                self.assertContains(direct, r"checkitem\s+ITEM_TEA")
                self.assertContains(direct, r"goto_if_eq\s+VAR_RESULT,\s*TRUE,")
                guard_script = load_map(map_name)["object_events"][0]["script"]
                guard = self.scripts.reachable_text(guard_script)
                self.assertIn("checkitem ITEM_TEA", guard)
                self.assertIn("removeitem ITEM_TEA", guard)

    def test_route12_snorlax_and_leftovers_move_together(self) -> None:
        data = load_map("Route12_Frlg")
        snorlax = [obj for obj in data["object_events"] if obj.get("flag") == "FLAG_HIDE_ROUTE_12_SNORLAX"]
        self.assertEqual(len(snorlax), 1)
        self.assertEqual((snorlax[0]["x"], snorlax[0]["y"]), (15, 70))
        self.assertIn("Route12_EventScript_Snorlax", snorlax[0]["script"])
        leftovers = [event for event in data["bg_events"] if event.get("flag") == "FLAG_HIDDEN_ITEM_ROUTE12_LEFTOVERS"]
        self.assertEqual(len(leftovers), 1)
        self.assertEqual((leftovers[0]["x"], leftovers[0]["y"]), (15, 70))
        self.assertEqual(leftovers[0]["item"], "ITEM_LEFTOVERS")
        self.assertTrue(leftovers[0]["underfoot"])
        snorlax_script = self.scripts.reachable_text(snorlax[0]["script"])
        self.assertIn("FLAG_GOT_POKE_FLUTE", snorlax_script)
        self.assertIn("FLAG_WOKE_UP_ROUTE_12_SNORLAX", snorlax_script)
        self.assertIn("FLAG_HIDE_ROUTE_12_SNORLAX", snorlax_script)

    def test_six_saved_flags_are_frlg_only_and_exact(self) -> None:
        assignments = {
            "FLAG_SEVII_SHAKEDOWN_STARTED": "0x4A7",
            "FLAG_SEVII_SHAKEDOWN_SPOT_1": "0x4A8",
            "FLAG_SEVII_SHAKEDOWN_SPOT_2": "0x4A9",
            "FLAG_SEVII_SHAKEDOWN_SPOT_3": "0x4AA",
            "FLAG_SEVII_SHAKEDOWN_COMPLETE": "0x4AB",
            "FLAG_SEVII_TRAVEL_INTRO_SEEN": "0x4AC",
        }
        frlg = read(GAME / "include/constants/flags_frlg.h")
        for name, value in assignments.items():
            self.assertContains(frlg, rf"^#define\s+{name}\s+{value}(?:\s|$)")
        for generic in ("flags.h", "flags_hns.h"):
            source = read(GAME / "include/constants" / generic)
            for name in assignments:
                self.assertNotIn(name, source, f"{name} must remain FRLG-only")

    def test_shakedown_map_and_retry_safe_reward(self) -> None:
        data = load_map("VermilionCity_Frlg")
        builder = at(data["object_events"], 36, 10)
        machop = at(data["object_events"], 35, 11)
        self.assertEqual(len(builder), 1)
        self.assertEqual(len(machop), 1)
        self.assertEqual(builder[0].get("local_id"), "LOCALID_VERMILION_PORT_BUILDER")
        self.assertEqual(machop[0].get("local_id"), "LOCALID_VERMILION_SHAKEDOWN_MACHOP")
        self.assertIn("Builder", builder[0]["script"])
        markers = {(event["x"], event["y"]): event for event in data["bg_events"] if (event["x"], event["y"]) in {(33, 9), (37, 9), (37, 13)}}
        self.assertEqual(set(markers), {(33, 9), (37, 9), (37, 13)})

        root = builder[0]["script"]
        graph = self.scripts.reachable_text(root)
        for flag in ("FLAG_SEVII_SHAKEDOWN_STARTED", "FLAG_SEVII_SHAKEDOWN_SPOT_1", "FLAG_SEVII_SHAKEDOWN_SPOT_2", "FLAG_SEVII_SHAKEDOWN_SPOT_3", "FLAG_SEVII_SHAKEDOWN_COMPLETE"):
            self.assertIn(flag, graph)
        self.assertIn("checkitem ITEM_RAINBOW_PASS", graph)
        self.assertIn("checkitemspace ITEM_RAINBOW_PASS", graph)
        self.assertIn("additem ITEM_RAINBOW_PASS", graph)
        self.assertNotContains(graph, r"\b(?:setfollower|createfollower|followobject)\b")
        self.assertOrderedPath(
            root,
            [
                r"checkitemspace\s+ITEM_RAINBOW_PASS",
                r"additem\s+ITEM_RAINBOW_PASS",
                r"checkitem\s+ITEM_RAINBOW_PASS",
                r"setflag\s+FLAG_SEVII_SHAKEDOWN_COMPLETE",
            ],
            "Rainbow Pass must be capacity-checked, delivered, verified, then committed",
        )
        for number in (1, 2, 3):
            marker_root = markers[((33, 9), (37, 9), (37, 13))[number - 1]]["script"]
            marker_graph = self.scripts.reachable_text(marker_root)
            self.assertIn(f"FLAG_SEVII_SHAKEDOWN_SPOT_{number}", marker_graph)
            self.assertOrderedPath(
                marker_root,
                [r"(?:applymovement|turnobject)", r"waitmovement", rf"setflag\s+FLAG_SEVII_SHAKEDOWN_SPOT_{number}"],
                f"spot {number} must commit only after Machop's presentation",
            )

        # Reconciliation requires both possession and completion to be tested,
        # including a delivery path even when completion was already present.
        self.assertContains(graph, r"goto_if_(?:set|unset)\s+FLAG_SEVII_SHAKEDOWN_COMPLETE,")
        self.assertContains(graph, r"goto_if_(?:eq|ne)\s+VAR_RESULT,\s*(?:TRUE|FALSE),")
        offer = self.scripts.block("VermilionCity_EventScript_OfferShakedown")
        self.assertEqual(offer.count("setflag"), 1)
        self.assertIn("setflag FLAG_SEVII_SHAKEDOWN_STARTED", offer)
        self.assertNotIn("additem", offer)
        has_pass = self.scripts.block("VermilionCity_EventScript_PortBuilderHasRainbowPass")
        self.assertIn("setflag FLAG_SEVII_SHAKEDOWN_COMPLETE", has_pass)
        self.assertNotIn("additem ITEM_RAINBOW_PASS", has_pass)
        reward_failure = self.scripts.block("VermilionCity_EventScript_ShakedownRewardNoRoom")
        self.assertNotIn("FLAG_SEVII_SHAKEDOWN_COMPLETE", reward_failure)
        builder_body = self.scripts.block("VermilionCity_EventScript_PortBuilderIncomplete")
        for number in (1, 2, 3):
            self.assertIn(f"call_if_set FLAG_SEVII_SHAKEDOWN_SPOT_{number}", builder_body)
        self.assertOrderedText(
            builder_body,
            [
                "call_if_set FLAG_SEVII_SHAKEDOWN_SPOT_1",
                "call_if_set FLAG_SEVII_SHAKEDOWN_SPOT_2",
                "call_if_set FLAG_SEVII_SHAKEDOWN_SPOT_3",
                "goto_if_ne VAR_0x8004, 0, VermilionCity_EventScript_ShakedownReportRemaining",
                "goto VermilionCity_EventScript_ShakedownReward",
            ],
            "the reward branch must remain behind completion of all three spots",
        )
        reconciliation = self.scripts.block("VermilionCity_EventScript_ReconcileSeviiIntroduction")
        self.assertContains(reconciliation, r"goto_if_lt\s+VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F,\s*1,")
        self.assertIn("setflag FLAG_SYS_SEVII_MAP_123", reconciliation)
        self.assertIn("setflag FLAG_SYS_SEVII_MAP_4567", reconciliation)
        self.assertIn("setflag FLAG_SEVII_TRAVEL_INTRO_SEEN", reconciliation)

    def test_shared_vermilion_dock_dispatch_is_story_isolated(self) -> None:
        data = load_map("VermilionCity_Frlg")
        sailor = object_with_local_id(data, "LOCALID_VERMILION_FERRY_SAILOR")
        trigger_scripts = {
            event["script"]
            for event in data["coord_events"]
            if event.get("script") in {"VermilionCity_EventScript_CheckTicketLeft", "VermilionCity_EventScript_CheckTicketRight"}
        }
        self.assertEqual(trigger_scripts, {"VermilionCity_EventScript_CheckTicketLeft", "VermilionCity_EventScript_CheckTicketRight"})
        for root in {sailor["script"], *trigger_scripts}:
            with self.subTest(root=root):
                graph = self.scripts.reachable_text(root)
                self.assertIn("FLAG_SEVII_SHAKEDOWN_COMPLETE", graph)
                self.assertIn("checkitem ITEM_RAINBOW_PASS", graph)
                self.assertIn("VAR_MAP_SCENE_VERMILION_CITY", graph)
                self.assertIn("FLAG_GOT_SS_TICKET", graph)
                self.assertIn("EventScript_SeviiDestinationsPage1", graph)
                self.assertIn("EventScript_CancelSail", graph)
        vermilion = uncommented(read(MAPS / "VermilionCity_Frlg/scripts.inc"))
        setters = re.findall(r"setvar\s+VAR_MAP_SCENE_VERMILION_CITY,\s*3", vermilion)
        self.assertEqual(len(setters), 1, "only the original S.S. Anne departure may set scene 3")
        self.assertIn("setvar VAR_MAP_SCENE_VERMILION_CITY, 3", self.scripts.block("VermilionCity_EventScript_ExitSSAnne"))
        dispatch = self.scripts.block("VermilionCity_EventScript_SharedDockDispatch")
        self.assertContains(dispatch, r"goto_if_eq\s+VAR_MAP_SCENE_VERMILION_CITY,\s*3,\s*VermilionCity_EventScript_SharedDockSeviiOnly")
        self.assertIn("MULTI_VERMILION_SHARED_DOCK", dispatch)
        self.assertContains(dispatch, r"case\s+0,\s*VermilionCity_EventScript_CheckSSTicket")
        self.assertContains(dispatch, r"case\s+1,\s*VermilionCity_EventScript_ChooseSeviiService")
        self.assertContains(dispatch, r"case\s+2,\s*EventScript_CancelSail")
        sevii_only = self.scripts.block("VermilionCity_EventScript_SharedDockSeviiOnly")
        self.assertIn("MULTI_VERMILION_SEVII_DOCK", sevii_only)
        self.assertNotIn("VermilionCity_EventScript_CheckTicket", sevii_only)
        sevii_choice = self.scripts.reachable_text("VermilionCity_EventScript_ChooseSeviiService")
        self.assertIn("FLAG_SEVII_TRAVEL_INTRO_SEEN", sevii_choice)
        self.assertNotContains(sevii_choice, r"setvar\s+VAR_MAP_SCENE_VERMILION_CITY,")

    def test_vermilion_state_5_return_preserves_pending_travel_intro(self) -> None:
        choice = self.scripts.block("VermilionCity_EventScript_ChooseSeviiService")
        pending = re.search(
            r"goto_if_eq\s+VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*5,\s*([A-Za-z0-9_]+)",
            choice,
        )
        self.assertIsNotNone(
            pending,
            "Vermilion Sevii selection must special-case a pending state-5 introduction",
        )
        unseen = re.search(
            r"goto_if_unset\s+FLAG_SEVII_TRAVEL_INTRO_SEEN,\s*([A-Za-z0-9_]+)",
            choice,
        )
        self.assertIsNotNone(unseen)
        self.assertLess(
            pending.start(),
            unseen.start(),
            "state 5 must dispatch before the generic unseen-introduction state-4 path",
        )

        pending_trip = self.scripts.reachable_text(pending.group(1))
        self.assertIn("SEAGALLOP_ONE_ISLAND", pending_trip)
        self.assertIn("EventScript_SailToDest", pending_trip)
        self.assertNotContains(
            pending_trip,
            r"setvar\s+VAR_MAP_SCENE_ONE_ISLAND_HARBOR,",
            "returning to retry Celio must preserve harbor state 5",
        )
        first_trip = self.scripts.block("VermilionCity_EventScript_FirstSeviiTrip")
        self.assertContains(
            first_trip,
            r"setvar\s+VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*4",
            "a true first trip must still enter the automatic travel introduction",
        )

    def test_full_service_is_reachable_from_every_island(self) -> None:
        selection_roots = (
            "EventScript_ChooseDestFromOneIsland",
            "EventScript_ChooseDestFromTwoIsland",
            "EventScript_ChooseDestFromIsland",
        )
        for root in selection_roots:
            with self.subTest(root=root):
                graph = self.scripts.reachable_text(root)
                self.assertIn("checkitem ITEM_RAINBOW_PASS", graph)
                self.assertIn("FLAG_SEVII_SHAKEDOWN_COMPLETE", graph)
                self.assertIn("FLAG_SEVII_TRAVEL_INTRO_SEEN", graph)
                self.assertIn("VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F", graph)
                self.assertIn("EventScript_SeviiDestinationsPage1", graph)
                self.assertIn("EventScript_CancelSail", graph)
        authorization = self.scripts.block("EventScript_CheckFullSeagallopService")
        for forbidden in ("LOSTELLE", "BIKER", "METEORITE", "RECOVERED_RUBY", "RECOVERED_SAPPHIRE"):
            self.assertNotIn(forbidden, authorization)
        pages = self.scripts.block("EventScript_SeviiDestinationsPage1") + self.scripts.block("EventScript_SeviiDestinationsPage2")
        for destination in ("VERMILION_CITY", "ONE_ISLAND", "TWO_ISLAND", "THREE_ISLAND", "FOUR_ISLAND", "FIVE_ISLAND", "SIX_ISLAND", "SEVEN_ISLAND"):
            self.assertIn(f"SEAGALLOP_{destination}", pages)
        self.assertIn("SEAGALLOP_MORE", pages)
        self.assertIn("MULTI_B_PRESSED", pages)

    def test_shared_seagallop_extensions_are_frlg_guarded(self) -> None:
        source = uncommented(read(DATA / "scripts/seagallop.inc"))
        guarded = frlg_guarded_lines(source)
        extension_tokens = (
            "EventScript_ChooseOnlyVermilionFromOneIsland",
            "EventScript_CheckFullSeagallopService",
            "EventScript_FullSeagallopServiceUnlocked",
            "EventScript_FullSeagallopServiceLocked",
            "FLAG_SEVII_SHAKEDOWN_COMPLETE",
            "FLAG_SEVII_TRAVEL_INTRO_SEEN",
        )
        occurrences = {token: 0 for token in extension_tokens}
        for line_number, line in enumerate(source.splitlines(), start=1):
            for token in extension_tokens:
                if token not in line:
                    continue
                occurrences[token] += 1
                self.assertTrue(
                    guarded[line_number - 1],
                    f"{token} at seagallop.inc:{line_number} must be inside #if IS_FRLG",
                )
        for token, count in occurrences.items():
            self.assertGreater(count, 0, f"expected shared Seagallop extension {token}")

        for root in (
            "EventScript_ChooseDestFromOneIsland",
            "EventScript_ChooseDestFromTwoIsland",
            "EventScript_ChooseDestFromIsland",
        ):
            with self.subTest(root=root):
                body = self.scripts.block(root)
                self.assertRegex(
                    body,
                    re.compile(
                        r"#if\s+IS_FRLG\b.*?"
                        r"EventScript_CheckFullSeagallopService.*?"
                        r"#else\b.*?"
                        r"goto_if_ge\s+VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F,\s*5,\s*EventScript_SeviiDestinationsPage1.*?"
                        r"#endif\b",
                        re.DOTALL,
                    ),
                    "non-FRLG builds must retain the original scene-5 menu branch",
                )

    def test_travel_only_intro_uses_harbor_states_and_commits_in_order(self) -> None:
        harbor_frame = self.scripts.block("OneIsland_Harbor_OnFrame")
        one_island_frame = self.scripts.block("OneIsland_OnFrame")
        self.assertContains(harbor_frame, r"map_script_2\s+VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*4,")
        self.assertContains(one_island_frame, r"map_script_2\s+VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*4,")

        center_frame = self.scripts.reachable_text("OneIsland_PokemonCenter_1F_OnFrame")
        self.assertIn("VAR_MAP_SCENE_ONE_ISLAND_HARBOR", center_frame)
        for value in (3, 4):
            self.assertContains(center_frame, rf"VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*{value}")
        self.assertIn("FLAG_SEVII_SHAKEDOWN_COMPLETE", center_frame)
        self.assertIn("FLAG_SEVII_TRAVEL_INTRO_SEEN", center_frame)

        intro = "OneIsland_PokemonCenter_1F_EventScript_TravelIntroduction"
        graph = self.scripts.reachable_text(intro)
        self.assertIn("checkitemspace ITEM_TOWN_MAP", graph)
        for forbidden in ("ITEM_METEORITE", "ITEM_TRI_PASS", "FLAG_SYS_PC_STORAGE_DISABLED"):
            self.assertNotIn(forbidden, graph)
        self.assertNotContains(graph, r"setvar\s+VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F,")
        prepare = self.scripts.block("OneIsland_PokemonCenter_1F_EventScript_TryPrepareTravelMap")
        self.assertContains(prepare, r"checkitemspace\s+ITEM_TOWN_MAP")
        self.assertContains(prepare, r"goto_if_eq\s+VAR_RESULT,\s*FALSE,\s*OneIsland_PokemonCenter_1F_EventScript_TravelMapNotReady")
        self.assertOrderedText(
            prepare,
            ["giveitem_msg", "ITEM_TOWN_MAP", "checkitem ITEM_TOWN_MAP", "FLAG_HIDE_TOWN_MAP"],
            "Town Map delivery must be verified before its associated state commits",
        )
        intro_body = self.scripts.block(intro)
        self.assertOrderedText(
            intro_body,
            [
                "call OneIsland_PokemonCenter_1F_EventScript_TryPrepareTravelMap",
                "goto_if_eq VAR_RESULT, FALSE",
                "call OneIsland_PokemonCenter_1F_EventScript_CommitTravelIntroduction",
            ],
            "the intro must abort on failed preparation before commit",
        )
        commit = self.scripts.block("OneIsland_PokemonCenter_1F_EventScript_CommitTravelIntroduction")
        self.assertOrderedText(
            commit,
            [
                "setflag FLAG_SYS_SEVII_MAP_123",
                "setflag FLAG_SYS_SEVII_MAP_4567",
                "setflag FLAG_SEVII_TRAVEL_INTRO_SEEN",
                "setvar VAR_MAP_SCENE_ONE_ISLAND_HARBOR, 0",
            ],
            "both map pages must precede intro completion and harbor reset",
        )
        no_room = self.scripts.block("OneIsland_PokemonCenter_1F_EventScript_TravelIntroductionNoRoom")
        self.assertContains(no_room, r"setvar\s+VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*5")
        self.assertIn("releaseall", no_room)

        celio_graph = self.scripts.reachable_text("OneIsland_PokemonCenter_1F_EventScript_Celio")
        self.assertContains(celio_graph, r"VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*5")
        harbor_sailor = self.scripts.reachable_text("OneIsland_Harbor_EventScript_Sailor")
        self.assertContains(harbor_sailor, r"VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*5")
        self.assertIn("EventScript_SailToVermilion2", harbor_sailor)
        self.assertIn("EventScript_CancelSail", harbor_sailor)

    def test_early_departure_hides_bill_and_cinnabar_preflights_restore_him(self) -> None:
        vermilion = uncommented(read(MAPS / "VermilionCity_Frlg/scripts.inc"))
        early_labels = [label for label, body in self.scripts.blocks.items() if self.scripts.paths[label] == MAPS / "VermilionCity_Frlg/scripts.inc" and "VAR_MAP_SCENE_ONE_ISLAND_HARBOR, 4" in body]
        self.assertTrue(early_labels, "Vermilion early-trip departure script was not found")
        self.assertTrue(any(self.scripts.has_ordered_path(label, [r"setflag\s+FLAG_HIDE_ONE_ISLAND_POKECENTER_BILL", r"setvar\s+VAR_MAP_SCENE_ONE_ISLAND_HARBOR,\s*4"]) for label in early_labels))

        cinnabar_root = "CinnabarIsland_EventScript_SailToOneIsland"
        graph = self.scripts.reachable_text(cinnabar_root)
        sail = self.scripts.block(cinnabar_root)
        self.assertOrderedText(
            sail,
            [
                "clearflag FLAG_HIDE_ONE_ISLAND_BILL",
                "clearflag FLAG_HIDE_ONE_ISLAND_POKECENTER_BILL",
                "setvar VAR_MAP_SCENE_ONE_ISLAND_HARBOR, 1",
            ],
            "Bill must be revealed immediately before the vanilla harbor arrival commits",
        )
        invitation_entries = (
            "CinnabarIsland_EventScript_AgreeSailToOneIsland",
            "CinnabarIsland_PokemonCenter_1F_EventScript_Bill",
        )
        for entry in invitation_entries:
            with self.subTest(invitation=entry):
                body = self.scripts.block(entry)
                self.assertOrderedText(
                    body,
                    [
                        "specialvar VAR_RESULT, CanStartOriginalSeviiTrip",
                        "goto_if_eq VAR_RESULT, FALSE",
                    ],
                    "each Bill invitation path must preflight before changing travel state",
                )
        preflight = uncommented(read(GAME / "src/seagallop.c"))
        self.assertContains(preflight, r"bool32\s+CanStartOriginalSeviiTrip\s*\(")
        for item in ("ITEM_TOWN_MAP", "ITEM_METEORITE", "ITEM_TRI_PASS", "ITEM_RAINBOW_PASS"):
            self.assertContains(preflight, rf"CheckBagHasItem\s*\(\s*{item},\s*1\s*\)")
        self.assertContains(preflight, r"emptySlots\s*>=\s*missingItems")
        hide_clear = re.search(r"clearflag\s+FLAG_HIDE_ONE_ISLAND_POKECENTER_BILL", graph)
        self.assertIsNotNone(hide_clear)
        clear_sites = {
            self.scripts.paths[label].relative_to(GAME).as_posix()
            for label, body in self.scripts.blocks.items()
            if "clearflag FLAG_HIDE_ONE_ISLAND_POKECENTER_BILL" in body
        }
        self.assertEqual(clear_sites, {"data/maps/CinnabarIsland_Frlg/scripts.inc"})
        self.assertIn("FLAG_SEVII_SHAKEDOWN_COMPLETE", self.scripts.reachable_text("OneIsland_PokemonCenter_1F_EventScript_MeetCelioScene"))

    def test_two_island_early_state_defers_without_starting_detour(self) -> None:
        transition = self.scripts.reachable_text("TwoIsland_JoyfulGameCorner_OnTransition")
        self.assertContains(transition, r"VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F,\s*1")
        self.assertContains(transition, r"setvar\s+VAR_MAP_SCENE_TWO_ISLAND_JOYFUL_GAME_CORNER,\s*5")
        self.assertContains(transition, r"VAR_MAP_SCENE_TWO_ISLAND_JOYFUL_GAME_CORNER,\s*5")
        self.assertContains(transition, r"setvar\s+VAR_MAP_SCENE_TWO_ISLAND_JOYFUL_GAME_CORNER,\s*0")
        on_transition = self.scripts.block("TwoIsland_JoyfulGameCorner_OnTransition")
        self.assertNotIn("FLAG_HIDE_THREE_ISLAND_LONE_BIKER", on_transition)
        self.assertNotIn("VAR_MAP_SCENE_THREE_ISLAND", on_transition)
        for npc in ("TwoIsland_JoyfulGameCorner_EventScript_InfoMan", "TwoIsland_JoyfulGameCorner_EventScript_LostellesDaddy"):
            self.assertContains(self.scripts.reachable_text(npc), r"VAR_MAP_SCENE_TWO_ISLAND_JOYFUL_GAME_CORNER,\s*5")

    def test_three_island_detour_finish_is_strictly_guarded(self) -> None:
        transition = self.scripts.reachable_text("ThreeIsland_Port_OnTransition")
        self.assertContains(transition, r"VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F,\s*1")
        self.assertIn("FLAG_SYS_PC_STORAGE_DISABLED", transition)
        self.assertIn("clearflag FLAG_SYS_PC_STORAGE_DISABLED", transition)
        self.assertIn("setflag FLAG_SEVII_DETOUR_FINISHED", transition)
        self.assertOrderedPath(
            "ThreeIsland_Port_OnTransition",
            [
                r"(?:goto|call)_if_(?:eq|ne)\s+VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F,\s*1,",
                r"(?:goto|call)_if_(?:set|unset)\s+FLAG_SYS_PC_STORAGE_DISABLED,",
                r"clearflag\s+FLAG_SYS_PC_STORAGE_DISABLED",
                r"setflag\s+FLAG_SEVII_DETOUR_FINISHED",
            ],
            "detour completion must require center scene 1 and disabled storage",
        )

    def test_four_and_six_rival_scene_is_deferred_and_synchronized(self) -> None:
        locations = (
            (
                "FourIsland",
                "FourIsland_OnTransition",
                "VAR_MAP_SCENE_FOUR_ISLAND",
                "VAR_MAP_SCENE_SIX_ISLAND_POKEMON_CENTER_1F",
                "FLAG_HIDE_FOUR_ISLAND_RIVAL",
            ),
            (
                "SixIsland_PokemonCenter_1F",
                "SixIsland_PokemonCenter_1F_OnTransition",
                "VAR_MAP_SCENE_SIX_ISLAND_POKEMON_CENTER_1F",
                "VAR_MAP_SCENE_FOUR_ISLAND",
                "FLAG_HIDE_SIX_ISLAND_POKECENTER_RIVAL",
            ),
        )
        for prefix, root, local_var, other_var, hide_flag in locations:
            with self.subTest(root=root):
                graph = self.scripts.reachable_text(root)
                self.assertIn("VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F", graph)
                self.assertContains(graph, rf"setvar\s+{local_var},\s*2")
                self.assertContains(graph, rf"setvar\s+{local_var},\s*0")
                self.assertIn("VAR_MAP_SCENE_FOUR_ISLAND", graph)
                self.assertIn("VAR_MAP_SCENE_SIX_ISLAND_POKEMON_CENTER_1F", graph)
                arbitration = self.scripts.block(f"{prefix}_EventScript_ArbitrateRivalScene")
                self.assertOrderedText(
                    arbitration,
                    [
                        f"goto_if_eq {local_var}, 1",
                        "goto_if_lt VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 5",
                        f"goto_if_eq {other_var}, 1",
                        f"goto_if_eq {local_var}, 2",
                    ],
                    "rival arbitration must defer before scene 5 and synchronize before restoring",
                )
                deferred = self.scripts.block(f"{prefix}_EventScript_DeferRivalScene")
                self.assertIn(f"setvar {local_var}, 2", deferred)
                self.assertIn(f"setflag {hide_flag}", deferred)
                self.assertNotIn(f"setvar {local_var}, 1", deferred)
                restored = self.scripts.block(f"{prefix}_EventScript_RestoreDeferredRivalScene")
                self.assertIn(f"setvar {local_var}, 0", restored)
                synchronized = self.scripts.block(f"{prefix}_EventScript_SynchronizeCompletedRivalScene")
                self.assertIn("setvar VAR_MAP_SCENE_FOUR_ISLAND, 1", synchronized)
                self.assertIn("setvar VAR_MAP_SCENE_SIX_ISLAND_POKEMON_CENTER_1F, 1", synchronized)
        for scene in ("FourIsland_EventScript_RivalScene", "SixIsland_PokemonCenter_1F_EventScript_RivalScene"):
            graph = self.scripts.reachable_text(scene)
            self.assertIn("setvar VAR_MAP_SCENE_FOUR_ISLAND, 1", graph)
            self.assertIn("setvar VAR_MAP_SCENE_SIX_ISLAND_POKEMON_CENTER_1F, 1", graph)


if __name__ == "__main__":
    unittest.main()
