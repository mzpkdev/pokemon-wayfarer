"""Source-level contract checks for HNS open-world regional traversal.

These checks deliberately inspect authored event JSON, event-script control flow,
and the small amount of C glue used by traversal.  They complement emulator
journeys by pinning failure/retry ordering and by proving that opening one route
does not synthesize unrelated campaign state.
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


def uncommented(text: str) -> str:
    return "\n".join(line.split("@", 1)[0] for line in text.splitlines())


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
    matches = [event for event in data["object_events"] if event.get("local_id") == local_id]
    if len(matches) != 1:
        raise AssertionError(f"expected one {local_id} object, found {len(matches)}")
    return matches[0]


def mutation_lines(text: str) -> list[str]:
    commands = r"(?:setvar|setflag|clearflag|additem|giveitem|removeitem)"
    return [line.strip() for line in text.splitlines() if re.match(rf"\s*{commands}\b", line)]


class ScriptIndex:
    """Index event labels and allow bounded reachability/order assertions."""

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

    def labels_in(self, path: Path) -> list[str]:
        return [label for label, source_path in self.paths.items() if source_path == path]

    def find_in(self, path: Path, *needles: str) -> list[str]:
        return [
            label
            for label in self.labels_in(path)
            if all(needle in self.blocks[label] for needle in needles)
        ]

    def reachable_labels(self, start: str, limit: int = 700) -> set[str]:
        found: set[str] = set()
        queue = deque([start])
        while queue:
            label = queue.popleft()
            if label in found:
                continue
            if label not in self.blocks:
                raise AssertionError(f"script label {label} is not defined")
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


class HnsTraversalContractTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.scripts = ScriptIndex()

    def assertContains(self, text: str, pattern: str, message: str | None = None) -> None:
        self.assertRegex(text, re.compile(pattern, re.MULTILINE | re.DOTALL), message)

    def assertNotContains(self, text: str, pattern: str, message: str | None = None) -> None:
        self.assertNotRegex(text, re.compile(pattern, re.MULTILINE | re.DOTALL), message)

    def assertOrderedText(self, text: str, snippets: list[str], message: str) -> None:
        cursor = 0
        for snippet in snippets:
            position = text.find(snippet, cursor)
            self.assertNotEqual(position, -1, f"{message}: missing or out of order: {snippet}")
            cursor = position + len(snippet)

    def assertOrderedPath(self, start: str, patterns: list[str], message: str) -> None:
        self.assertTrue(self.scripts.has_ordered_path(start, patterns), message)

    def test_new_bark_west_exit_removes_only_post_starter_turnbacks(self) -> None:
        triggers = load_map("NewBarkTown_hns")["coord_events"]
        for state in ("2", "4"):
            for y in (12, 13, 14):
                self.assertFalse(at(triggers, 0, y, var="VAR_NEWBARK_TOWN_STATE", var_value=state))
        lab = uncommented(read(MAPS / "NewBarkTown_Lab_hns/scripts.inc"))
        self.assertContains(lab, r"setvar\s+VAR_NEWBARK_TOWN_STATE,\s*3")
        self.assertContains(lab, r"setvar\s+VAR_NEWBARK_TOWN_STATE,\s*4")
        home = uncommented(read(MAPS / "NewBarkTown_PlayersHouse_1F_hns/scripts.inc"))
        self.assertContains(home, r"setvar\s+VAR_NEWBARK_TOWN_STATE,\s*5")

    def test_cherrygrove_and_azalea_keep_one_optional_silver_lane(self) -> None:
        cherry = load_map("CherrygroveCity_hns")["coord_events"]
        self.assertFalse(at(cherry, 56, 9, var="VAR_CHERRYGROVE_CITY_STATE", var_value="3"))
        for y in (10, 11):
            self.assertEqual(
                len(at(cherry, 56, y, var="VAR_CHERRYGROVE_CITY_STATE", var_value="3")),
                1,
            )
        cherry_battle = self.scripts.block("CherryGroveCity_Silver_AfterBattle")
        self.assertIn("setvar VAR_CHERRYGROVE_CITY_STATE, 4", cherry_battle)
        self.assertIn("setflag FLAG_HIDE_SILVER_CHERRYGROVE", cherry_battle)

        azalea = load_map("AzaleaTown_hns")["coord_events"]
        self.assertFalse(at(azalea, 11, 17, var="VAR_AZALEA_TOWN_STATE", var_value="5"))
        kept = at(azalea, 11, 16, var="VAR_AZALEA_TOWN_STATE", var_value="5")
        self.assertEqual([event["script"] for event in kept], ["AzaleaTown_EventScript_SilverTriggerTop"])
        battle = self.scripts.reachable_text("AzaleaTown_EventScript_SilverTriggerTop")
        self.assertIn("setvar VAR_AZALEA_TOWN_STATE, 6", battle)
        self.assertIn("setflag FLAG_HIDE_AZALEA_TOWN_SILVER", battle)

    def test_route_30_staging_and_lane_are_unchanged(self) -> None:
        data = load_map("Route30_hns")
        staged = {
            "LOCALID_ROUTE_30_JOEY": (23, 25, "Route30_EventScript_Youngster_Joey_Fighting"),
            "OBJ_EVENT_GFX_MON_BASE+SPECIES_PIDGEY": (23, 23, "NULL"),
            "OBJ_EVENT_GFX_MON_BASE+SPECIES_RATTATA": (23, 24, "NULL"),
        }
        for identity, expected in staged.items():
            matches = [
                obj
                for obj in data["object_events"]
                if obj.get("local_id") == identity or obj.get("graphics_id") == identity
            ]
            if identity.endswith("PIDGEY") or identity.endswith("RATTATA"):
                matches = [obj for obj in matches if obj.get("flag") == "FLAG_MOM_VISITED"]
            self.assertEqual(len(matches), 1)
            self.assertEqual((matches[0]["x"], matches[0]["y"], matches[0]["script"]), expected)
            self.assertEqual(matches[0]["flag"], "FLAG_MOM_VISITED")

    def test_route_32_turnbacks_are_deleted_and_reward_is_retry_safe(self) -> None:
        data = load_map("Route32_hns")
        for state in ("1", "2", "4"):
            for x in (27, 28, 29):
                self.assertFalse(at(data["coord_events"], x, 10, var="VAR_VIOLET_CITY_STATE", var_value=state))
        guard = at(data["object_events"], 26, 10)
        self.assertEqual(len(guard), 1)
        self.assertEqual(guard[0]["script"], "Route32_EventScript_BaldingMan")
        check = self.scripts.block("Route32_EventScript_BaldingManCheck")
        self.assertOrderedText(
            check,
            [
                "FLAG_HIDE_SPROUT_TOWER_SILVER",
                "FLAG_DEFEATED_VIOLET_GYM",
                "FLAG_RECEIVED_TOGEPI_EGG",
                "giveitem ITEM_MIRACLE_SEED",
                "goto_if_eq VAR_RESULT, FALSE",
                "setvar VAR_VIOLET_CITY_STATE, 5",
            ],
            "Miracle Seed delivery must succeed before state 5 commits",
        )
    def test_ilex_choke_tree_only_is_removed(self) -> None:
        data = load_map("IlexForest_hns")
        self.assertFalse(at(data["object_events"], 32, 40, script="EventScript_CutTree", flag="FLAG_TEMP_1"))
        self.assertFalse([obj for obj in data["object_events"] if obj.get("script") == "EventScript_CutTree"])
        quest = self.scripts.reachable_text("IlexForest_EventScript_Trigger")
        self.assertIn("ITEM_HM_CUT", quest)
        self.assertIn("VAR_ILEX_FOREST_FARFETCHD", quest)

    def test_sudowoodo_is_off_junction_and_retryable(self) -> None:
        obj = object_with_local_id(load_map("Route36_hns"), "LOCALID_ROUTE36_SUDOWOODO")
        self.assertEqual((obj["x"], obj["y"]), (37, 17))
        self.assertEqual(obj["script"], "Route36_EventScript_SudoWoodo")
        self.assertEqual(obj["flag"], "FLAG_HIDE_SUDOWOODO")
        root = self.scripts.reachable_text(obj["script"])
        self.assertIn("B_OUTCOME_WON", root)
        self.assertIn("B_OUTCOME_RAN", root)
        ran = self.scripts.block("Route36_EventScript_RanSudowoodo")
        self.assertNotIn("FLAG_HIDE_SUDOWOODO", ran)
        won = self.scripts.block("Route36_EventScript_Defeated_Sudowoodo")
        self.assertIn("setflag FLAG_HIDE_SUDOWOODO", won)
        self.assertIn("removeobject LOCALID_ROUTE36_SUDOWOODO", won)
        dialogue = "\n".join(
            self.scripts.block(label)
            for label in self.scripts.labels_in(MAPS / "Route36_hns/scripts.inc")
            if label.startswith("Route36_Text_")
        )
        self.assertNotContains(dialogue, r"(?i)(block(?:s|ing|ed)?|roadblock).{0,24}(road|route)|(?:road|route).{0,24}block")

    def test_changed_maps_preserve_ordinary_trainer_tuples(self) -> None:
        expected = {
            "Route30_hns": {
                ("Route30_EventScript_Youngster_Joey", 20, 28, "TRAINER_TYPE_NORMAL", "5"),
                ("Route30_EventScript_Bugcatcher_Don", 20, 8, "TRAINER_TYPE_NORMAL", "2"),
                ("Route30_EventScript_Youngster_Mikey", 23, 22, "TRAINER_TYPE_NORMAL", "2"),
            },
            "Route32_hns": {
                ("Route32_EventScript_YoungsterAlbert", 22, 23, "TRAINER_TYPE_NORMAL", "7"),
                ("Route32_EventScript_PicnickerLiz", 20, 31, "TRAINER_TYPE_NORMAL", "7"),
                ("Route32_EventScript_CamperRoland", 14, 45, "TRAINER_TYPE_NORMAL", "7"),
                ("Route32_EventScript_FisherRalph", 25, 57, "TRAINER_TYPE_NORMAL", "0"),
                ("Route32_EventScript_FisherHenry", 28, 63, "TRAINER_TYPE_NORMAL", "0"),
                ("Route32_EventScript_FisherJustin", 20, 56, "TRAINER_TYPE_NORMAL", "0"),
                ("Route32_EventScript_YoungsterGordon", 15, 63, "TRAINER_TYPE_NORMAL", "5"),
                ("Route32_EventScript_BirdkeeperPeter", 24, 95, "TRAINER_TYPE_NORMAL", "5"),
            },
            "IlexForest_hns": {
                ("IlexForest_EventScript_BugcatcherWayne", 39, 8, "TRAINER_TYPE_NORMAL", "6"),
            },
            "Route36_hns": {
                ("Route36_EventScript_Mark", 24, 24, "TRAINER_TYPE_NORMAL", "4"),
                ("Route36_EventScript_Schoolboy", 31, 23, "TRAINER_TYPE_NORMAL", "5"),
            },
            "SSAqua_B1F_hns": {
                ("SSAqua_B1F_EventScript_Wai", 10, 10, "TRAINER_TYPE_NORMAL", "0"),
                ("SSAqua_B1F_EventScript_Nate", 10, 11, "TRAINER_TYPE_NORMAL", "0"),
                ("SSAqua_B1F_EventScript_Shirley", 7, 11, "TRAINER_TYPE_NORMAL", "0"),
                ("SSAqua_B1F_EventScript_Ricky", 7, 12, "TRAINER_TYPE_NORMAL", "0"),
                ("SSAqua_B1F_EventScript_Debra", 16, 8, "TRAINER_TYPE_NORMAL", "4"),
                ("SSAqua_B1F_EventScript_Kenneth", 1, 7, "TRAINER_TYPE_NORMAL", "4"),
                ("SSAqua_B1F_EventScript_Jonah", 19, 12, "TRAINER_TYPE_NORMAL", "4"),
                ("SSAqua_B1F_EventScript_Fritz", 24, 8, "TRAINER_TYPE_NORMAL", "0"),
                ("SSAqua_B1F_EventScript_Garrett", 11, 3, "TRAINER_TYPE_NORMAL", "3"),
            },
            "SSAqua_RoomNW_hns": {
                ("SSAqua_RoomNW_EventScript_Stanly", 2, 6, "TRAINER_TYPE_NORMAL", "4"),
                ("SSAqua_RoomNW_EventScript_Edward", 4, -5, "TRAINER_TYPE_NORMAL", "3"),
                ("SSAqua_RoomNW_EventScript_Corey", 5, -5, "TRAINER_TYPE_NORMAL", "4"),
            },
            "CeruleanCity_Gym_hns": {
                ("CeruleanGym_EventScript_Parker", 9, 13, "TRAINER_TYPE_NORMAL", "1"),
                ("CeruleanGym_EventScript_Briana", 5, 8, "TRAINER_TYPE_NORMAL", "1"),
                ("CeruleanGym_EventScript_Diana", 4, 12, "TRAINER_TYPE_NORMAL", "1"),
            },
        }
        for map_name, wanted in expected.items():
            with self.subTest(map=map_name):
                actual = {
                    (
                        obj["script"],
                        obj["x"],
                        obj["y"],
                        obj["trainer_type"],
                        obj["trainer_sight_or_berry_tree_id"],
                    )
                    for obj in load_map(map_name)["object_events"]
                    if obj.get("trainer_type") != "TRAINER_TYPE_NONE"
                }
                self.assertEqual(actual, wanted)

    def test_new_game_owns_kanto_visibility_defaults_and_reuses_rocket_state(self) -> None:
        flags = read(GAME / "include/constants/flags_hns.h")
        self.assertContains(flags, r"^#define\s+FLAG_UNUSED_39\s+0x307(?:\s|$)")
        self.assertContains(flags, r"^#define\s+HNS_UNUSED_COUNT\s+40(?:\s|$)")
        self.assertNotIn("FLAG_HNS_MAGNET_TRAIN_RESTORATION_STARTED", flags)
        reset = self.scripts.block("EventScript_ResetAllMapFlagsHnS")
        set_defaults = (
            "FLAG_HIDE_COPYCAT_CLEFAIRY_DOLL",
            "FLAG_HIDE_CERULEAN_GYM_TRAINERS",
            "FLAG_HIDE_CERULEAN_CAPE_ROCKET",
            "FLAG_HIDE_CERULEAN_GYM_ROCKET",
            "FLAG_HIDDEN_ITEM_MACHINE_PART",
            "FLAG_HIDE_ROUTE25_MISTY",
            "FLAG_HIDE_ROUTE25_EUSINE",
            "FLAG_HIDE_ROUTE25_SUICUNE",
            "FLAG_HIDE_CELADON_EUSINE",
            "FLAG_HIDE_SEAFOAM_GYMGUY",
            "FLAG_HIDE_VIRIDIAN_BLUE",
            "FLAG_HIDE_ROUTE14_EUSINE",
            "FLAG_HIDE_ROUTE14_SUICUNE",
            "FLAG_MT_SILVER_1F_HIDE_SCIENTIST",
        )
        for flag in set_defaults:
            self.assertContains(reset, rf"^\s*setflag\s+{flag}\s*$")
        for flag in (
            "FLAG_HIDE_MTMOON_SILVER",
            "FLAG_HIDE_FAN_CLUB_CLEFAIRY_DOLL",
        ):
            self.assertContains(reset, rf"^\s*clearflag\s+{flag}\s*$")
        self.assertNotContains(reset, r"setvar\s+VAR_KANTO_ROCKET_STORY_STATE,")

    def test_ss_aqua_boarding_and_corridor_are_resumable(self) -> None:
        sailor = self.scripts.block("OlivinePort_EventScript_Sailor")
        self.assertContains(sailor, r"goto_if_ge\s+VAR_SSAQUA_STATE,\s*8,")
        self.assertContains(sailor, r"goto_if_ge\s+VAR_SSAQUA_STATE,\s*1,")
        repeat = self.scripts.reachable_text("OlivinePort_EventScript_Sailor_AfterKanto")
        self.assertIn("checkitem ITEM_SS_TICKET", repeat)
        self.assertNotIn("FLAG_RETURNED_MACHINE_PART", repeat)
        maiden = self.scripts.block("OlivinePort_EventScript_Sailor_MaidenVoyage")
        self.assertIn("setvar VAR_SSAQUA_STATE, 1", maiden)
        resume_labels = self.scripts.find_in(
            MAPS / "OlivineCity_PortInside_hns/scripts.inc", "MAP_SSAQUA_1F_HNS"
        )
        self.assertTrue(
            any("setvar VAR_SSAQUA_STATE, 1" not in self.scripts.block(label) for label in resume_labels),
            "states 1 through 7 need a reboard path that does not reinitialize actors",
        )

        b1f = load_map("SSAqua_B1F_hns")
        looking = object_with_local_id(b1f, "LOCALID_SSAQUA_B1F_SAILORLOOKING")
        self.assertNotIn((looking["x"], looking["y"]), {(28, 8), (29, 8)})
        self.assertFalse(at(b1f["coord_events"], 29, 8, var="VAR_SSAQUA_STATE", var_value="2"))
        stanley = self.scripts.reachable_text("SSAqua_RoomNW_EventScript_Stanly")
        self.assertNotContains(stanley, r"setvar\s+VAR_SSAQUA_STATE,")

    def test_ss_aqua_reunion_rewards_are_exact_once_and_retry_safe(self) -> None:
        captain = self.scripts.reachable_text("SSAqua_CaptainsRoom_EventScript_Granddaughter")
        self.assertContains(captain, r"setvar\s+VAR_SSAQUA_STATE,\s*3")
        on_frame = self.scripts.block("SSAqua_RoomSSE_OnFrame")
        self.assertContains(on_frame, r"map_script_2\s+VAR_SSAQUA_STATE,\s*3,")
        transition = self.scripts.block("SSAqua_RoomSSE_OnTransition")
        self.assertContains(transition, r"goto_if_ge\s+VAR_SSAQUA_STATE,\s*4,")
        reunion = self.scripts.block("SSAqua_RoomSSE_EventScript_ReunionScene")
        self.assertIn("setvar VAR_SSAQUA_STATE, 4", reunion)
        self.assertNotIn("ITEM_SS_TICKET", reunion)
        self.assertNotIn("ITEM_METAL_COAT", reunion)

        grandpa = self.scripts.reachable_text("SSAqua_RoomSSE_EventScript_Grandpa")
        self.assertIn("VAR_SSAQUA_STATE, 4", grandpa)
        self.assertIn("VAR_SSAQUA_STATE, 5", grandpa)
        self.assertIn("checkitem ITEM_SS_TICKET", grandpa)
        self.assertIn("checkitemspace ITEM_SS_TICKET", grandpa)
        self.assertIn("giveitem ITEM_SS_TICKET", grandpa)
        self.assertIn("checkitemspace ITEM_METAL_COAT", grandpa)
        self.assertIn("giveitem ITEM_METAL_COAT", grandpa)
        self.assertOrderedPath(
            "SSAqua_RoomSSE_EventScript_Grandpa",
            [
                r"checkitemspace\s+ITEM_SS_TICKET",
                r"giveitem\s+ITEM_SS_TICKET",
                r"goto_if_eq\s+VAR_RESULT,\s*FALSE,",
                r"setvar\s+VAR_SSAQUA_STATE,\s*5",
            ],
            "Ticket delivery must be capacity-checked, verified, then committed",
        )
        self.assertOrderedPath(
            "SSAqua_RoomSSE_EventScript_Grandpa",
            [
                r"checkitemspace\s+ITEM_METAL_COAT",
                r"giveitem\s+ITEM_METAL_COAT",
                r"goto_if_eq\s+VAR_RESULT,\s*FALSE,",
                r"setvar\s+VAR_SSAQUA_STATE,\s*6",
            ],
            "Metal Coat delivery must be capacity-checked, verified, then committed",
        )
        failure = self.scripts.block("SSAqua_RoomSSE_EventScript_MakeRoom")
        self.assertNotIn("setvar VAR_SSAQUA_STATE", failure)
        self.assertNotIn("giveitem", failure)

    def test_ticket_source_hall_of_fame_and_arrival_are_isolated(self) -> None:
        elm = uncommented(read(MAPS / "NewBarkTown_Lab_hns/scripts.inc"))
        self.assertNotIn("giveitem ITEM_SS_TICKET", elm)
        hof = self.scripts.reachable_text("PokemonLeague_HallOfFame_EventScript_SetFirstGameClearFlags")
        self.assertNotContains(hof, r"setvar\s+VAR_SSAQUA_STATE,")

        on_frame = self.scripts.block("SSAqua_1F_OnFrame")
        self.assertContains(on_frame, r"map_script_2\s+VAR_SSAQUA_STATE,\s*6,")
        arrived = self.scripts.block("SSAqua_1F_EventScript_Arrived")
        self.assertIn("setvar VAR_SSAQUA_STATE, 7", arrived)
        door = self.scripts.block("SSAqua_1F_EventScript_DoorSailor")
        self.assertContains(door, r"goto_if_ge\s+VAR_SSAQUA_STATE,\s*7,")
        leave = self.scripts.block("SSAqua_1F_EventScript_LeaveBoat")
        self.assertEqual(
            mutation_lines(leave),
            [
                "setflag FLAG_VISITED_KANTO",
                "setflag FLAG_VISITED_VERMILION_CITY",
                "setvar VAR_SSAQUA_STATE, 8",
            ],
        )
        self.assertIn("warp MAP_VERMILION_CITY_PORT_INSIDE_HNS", leave)

        vermilion = self.scripts.reachable_text("VermilionPort_EventScript_Sailor")
        self.assertIn("VAR_SSAQUA_STATE, 8", vermilion)
        self.assertIn("checkitem ITEM_SS_TICKET", vermilion)
        self.assertNotIn("FLAG_RETURNED_MACHINE_PART", vermilion)

    def test_region_map_selectors_share_one_kanto_flag(self) -> None:
        source = uncommented(read(GAME / "src/region_map.c"))
        active = re.search(
            r"const struct RegionMapLocation \*GetActiveRegionMapEntries\(void\)\s*\{(.*?)\n\}",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(active)
        self.assertContains(
            active.group(1),
            r"FlagGet\(FLAG_VISITED_KANTO\).*?sRegionMapEntries_JK.*?else.*?gRegionMapEntries",
        )
        map_type = re.search(r"enum RegionMapType GetRegionMapType\(.*?\)\s*\{(.*?)\n\}", source, re.DOTALL)
        self.assertIsNotNone(map_type)
        self.assertContains(
            map_type.group(1),
            r"FlagGet\(FLAG_VISITED_KANTO\).*?REGION_MAP_JK.*?REGION_MAP_JOHTO",
        )
        map_sec = re.search(
            r"static mapsec_u16_t GetMapSecIdAt\([^;{}]*\)\s*\{(.*?)\n\}",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(map_sec)
        self.assertContains(
            map_sec.group(1),
            r"FlagGet\(FLAG_VISITED_KANTO\).*?sRegionMapSections_JK.*?sRegionMapSections_Johto",
        )
        for consumer in (
            "cursorPosX = GetActiveRegionMapEntries()",
            "cursorPosY = GetActiveRegionMapEntries()",
            "StringCopy(dest, GetActiveRegionMapEntries()",
            "*x = GetActiveRegionMapEntries()",
            "*y = GetActiveRegionMapEntries()",
        ):
            self.assertIn(consumer, source)

    def test_mt_moon_silver_is_optional_and_local(self) -> None:
        data = load_map("MtMoon_Cave_hns")
        self.assertFalse(
            [
                event
                for event in data["coord_events"]
                if event.get("var") == "VAR_PEWTER_CITY_STATE" and event.get("var_value") in {"0", "1"}
            ]
        )
        silver = object_with_local_id(data, "LOCALID_MTMOON_SILVER")
        self.assertEqual((silver["x"], silver["y"]), (9, 11))
        self.assertEqual(silver["movement_type"], "MOVEMENT_TYPE_FACE_DOWN")
        self.assertEqual(silver["script"], "MtMoon_Cave_EventScript_Silver")
        source = uncommented(read(MAPS / "MtMoon_Cave_hns/scripts.inc"))
        transition = re.search(
            r"MtMoon_Cave_OnTransition:\s*(.*?)MtMoon_Cave_EventScript_EndTransition::",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(transition)
        self.assertEqual(mutation_lines(transition.group(1)), ["setvar VAR_PEWTER_CITY_STATE, 1"])
        interaction = self.scripts.reachable_text(silver["script"])
        self.assertIn("MSGBOX_YESNO", interaction)
        declined = self.scripts.block("MtMoon_Cave_EventScript_SilverDeclined")
        self.assertNotIn("VAR_PEWTER_CITY_STATE", declined)
        victory = self.scripts.block("MtMoon_Cave_EventScript_SilverAfterBattle")
        self.assertOrderedText(
            victory,
            [
                "fadescreen FADE_TO_BLACK",
                "removeobject LOCALID_MTMOON_SILVER",
                "setflag FLAG_HIDE_MTMOON_SILVER",
                "clearflag FLAG_HIDE_INDIGO_PLATEAU_SILVER",
                "setvar VAR_PEWTER_CITY_STATE, 2",
            ],
            "Silver's original local commits must follow his optional victory",
        )

    def test_cycling_road_loan_never_changes_owned_bicycle(self) -> None:
        source = uncommented(read(MAPS / "Gate_CeladonCity_Route16_hns/scripts.inc"))
        root = self.scripts.reachable_text("Gate_CeladonCity_Route16_TriggerScript")
        self.assertIn("FLAG_SYS_CYCLING_ROAD", root)
        self.assertIn("ForcePlayerOntoBike", root)
        self.assertIn("ITEM_BICYCLE", root)
        self.assertIn("ForcePlayerOffBike", root)
        self.assertOrderedPath(
            "Gate_CeladonCity_Route16_TriggerScript",
            [r"special\s+ForcePlayerOntoBike", r"setflag\s+FLAG_SYS_CYCLING_ROAD"],
            "loan entry must mount before committing cycling-road state",
        )
        self.assertOrderedPath(
            "Gate_CeladonCity_Route16_TriggerScript",
            [
                r"checkitem\s+ITEM_BICYCLE",
                r"special\s+ForcePlayerOffBike",
                r"clearflag\s+FLAG_SYS_CYCLING_ROAD",
            ],
            "a loaned exit must dismount and clear the system flag",
        )
        self.assertNotContains(source, r"(?:additem|giveitem|removeitem)\s+ITEM_BICYCLE")
        self.assertNotIn("FLAG_RECEIVED_BIKE", source)
        for map_name in ("Gate_CeladonCity_Route16_hns", "Gate_FuchsiaCity_Route18_hns"):
            triggers = load_map(map_name)["coord_events"]
            self.assertTrue(
                any(event.get("script") == "Gate_CeladonCity_Route16_TriggerScript" for event in triggers),
                f"{map_name} must retain the shared symmetric gate trigger",
            )

    def test_kanto_rocket_story_progresses_from_manager_to_route24(self) -> None:
        manager = self.scripts.block("Route10_PowerPlantBackRoom_EventScript_Manager")
        self.assertOrderedText(
            manager,
            [
                "VAR_KANTO_ROCKET_STORY_STATE, 0",
                "Route10_PowerPlantBackRoom_EventScript_ManagerInitial",
                "VAR_KANTO_ROCKET_STORY_STATE, 1",
                "VAR_KANTO_ROCKET_STORY_STATE, 2",
                "VAR_KANTO_ROCKET_STORY_STATE, 3",
                "VAR_KANTO_ROCKET_STORY_STATE, 4",
                "VAR_KANTO_ROCKET_STORY_STATE, 5",
                "VAR_KANTO_ROCKET_STORY_STATE, 6",
                "Route10_PowerPlantBackRoom_EventScript_ManagerCheckPart",
                "VAR_KANTO_ROCKET_STORY_STATE, 7",
                "Route10_PowerPlantBackRoom_EventScript_ManagerDone",
            ],
            "the manager must dispatch every Rocket state without another prerequisite",
        )
        self.assertOrderedPath(
            "Route10_PowerPlantBackRoom_EventScript_Manager",
            [
                r"goto_if_eq\s+VAR_KANTO_ROCKET_STORY_STATE,\s*0,",
                r"setvar\s+VAR_KANTO_ROCKET_STORY_STATE,\s*1",
            ],
            "speaking to the manager at state 0 must start the theft story",
        )

        entrance = load_map("Route10_PowerPlantEntrance_hns")
        self.assertEqual(
            at(
                entrance["coord_events"],
                6,
                15,
                var="VAR_KANTO_ROCKET_STORY_STATE",
                var_value="1",
            ),
            [
                {
                    "type": "trigger",
                    "x": 6,
                    "y": 15,
                    "elevation": 0,
                    "var": "VAR_KANTO_ROCKET_STORY_STATE",
                    "var_value": "1",
                    "script": "Route10_PowerPlantEntrance_Trigger_Cop",
                }
            ],
        )
        self.assertOrderedPath(
            "Route10_PowerPlantEntrance_Trigger_Cop",
            [
                r"setvar\s+VAR_KANTO_ROCKET_STORY_STATE,\s*2",
                r"clearflag\s+FLAG_HIDE_CERULEAN_GYM_ROCKET",
            ],
            "the officer report must expose the Gym grunt and enter state 2",
        )

        gym = load_map("CeruleanCity_Gym_hns")
        rocket = object_with_local_id(gym, "LOCALID_CERULEAN_GYM_ROCKET")
        self.assertEqual(rocket["flag"], "FLAG_HIDE_CERULEAN_GYM_ROCKET")
        on_frame = self.scripts.block("CeruleanCity_Gym_OnFrame")
        self.assertContains(
            on_frame,
            r"map_script_2\s+VAR_KANTO_ROCKET_STORY_STATE,\s*2,\s*CeruleanCity_Gym_EventScript_RocketGrunt",
        )
        self.assertOrderedPath(
            "CeruleanCity_Gym_EventScript_RocketGrunt",
            [
                r"setflag\s+FLAG_HIDE_CERULEAN_GYM_ROCKET",
                r"clearflag\s+FLAG_HIDE_CERULEAN_CAPE_ROCKET",
                r"setvar\s+VAR_KANTO_ROCKET_STORY_STATE,\s*3",
            ],
            "the Gym escape must move the same grunt to Route 24 and enter state 3",
        )
        gym_escape = self.scripts.reachable_text("CeruleanCity_Gym_EventScript_RocketGrunt")
        for unrelated in (
            "VAR_NUM_BADGES",
            "FLAG_DEFEATED_CERULEAN_GYM",
            "FLAG_KANTO_RADIO_GOT",
        ):
            self.assertNotIn(unrelated, gym_escape)

        route24 = load_map("Route24_hns")
        self.assertEqual(
            {
                (event["x"], event["y"], event["var_value"], event["script"])
                for event in route24["coord_events"]
                if event.get("var") == "VAR_KANTO_ROCKET_STORY_STATE"
            },
            {
                (x, 11, "3", "Route24_Trigger_GruntScene")
                for x in (16, 17, 18)
            },
        )
        staging = self.scripts.block("Route24_Trigger_GruntScene")
        self.assertIn("setvar VAR_KANTO_ROCKET_STORY_STATE, 4", staging)
        self.assertNotContains(staging, r"(?:setflag|clearflag)\s+FLAG_(?:HIDE_CERULEAN_CAPE_ROCKET|HIDDEN_ITEM_MACHINE_PART)")

        progression = "\n".join(
            self.scripts.reachable_text(root)
            for root in (
                "Route10_PowerPlantBackRoom_EventScript_Manager",
                "Route10_PowerPlantEntrance_Trigger_Cop",
                "CeruleanCity_Gym_EventScript_RocketGrunt",
                "Route24_Trigger_GruntScene",
                "Route24_EventScript_Grunt",
                "CeruleanCity_Gym_EventScript_MachinePartItem",
            )
        )
        for unrelated in (
            "VAR_NUM_BADGES",
            "FLAG_DEFEATED_CERULEAN_GYM",
            "FLAG_KANTO_RADIO_GOT",
            "VAR_FAN_CLUB_CLEFAIRY",
            "FLAG_HIDE_VERMILION_SNORLAX",
            "ITEM_HM_",
        ):
            self.assertNotIn(unrelated, progression)

    def test_route24_grunt_loss_is_retryable_and_victory_unlocks_part(self) -> None:
        route24 = load_map("Route24_hns")
        grunt = object_with_local_id(route24, "LOCALID_ROUTE24_GRUNT")
        self.assertEqual(grunt["script"], "Route24_EventScript_Grunt")
        self.assertEqual(grunt["flag"], "FLAG_HIDE_CERULEAN_CAPE_ROCKET")
        battle = self.scripts.block("Route24_EventScript_Grunt")
        self.assertOrderedText(
            battle,
            [
                "trainerbattle_no_intro TRAINER_GRUNT_31_HNS",
                "setflag FLAG_HIDE_CERULEAN_CAPE_ROCKET",
                "removeobject LOCALID_ROUTE24_GRUNT",
                "setvar VAR_KANTO_ROCKET_STORY_STATE, 5",
                "clearflag FLAG_HIDDEN_ITEM_MACHINE_PART",
            ],
            "only returning from a won grunt battle may commit state 5 and expose the part",
        )
        writers = self.scripts.find_in(
            MAPS / "Route24_hns/scripts.inc",
            "setvar VAR_KANTO_ROCKET_STORY_STATE, 5",
        )
        self.assertEqual(writers, ["Route24_EventScript_Grunt"])
        before_battle = battle.split("trainerbattle_no_intro TRAINER_GRUNT_31_HNS", 1)[0]
        self.assertNotContains(
            before_battle,
            r"(?:setvar\s+VAR_KANTO_ROCKET_STORY_STATE,\s*5|setflag\s+FLAG_HIDE_CERULEAN_CAPE_ROCKET|clearflag\s+FLAG_HIDDEN_ITEM_MACHINE_PART)",
            "loss must leave state 4, the grunt, and the hidden part unchanged for retry",
        )

    def test_machine_part_pickup_is_state_bounded_and_retry_safe(self) -> None:
        gym_path = MAPS / "CeruleanCity_Gym_hns/scripts.inc"
        gym = load_map("CeruleanCity_Gym_hns")
        pickup_objects = [
            obj
            for obj in gym["object_events"]
            if obj.get("script") == "CeruleanCity_Gym_EventScript_MachinePartItem"
        ]
        self.assertEqual(len(pickup_objects), 1)
        self.assertEqual(pickup_objects[0]["flag"], "FLAG_HIDDEN_ITEM_MACHINE_PART")
        self.assertEqual(
            at(
                gym["bg_events"],
                0,
                13,
                script="CeruleanCity_Gym_EventScript_MachinePartItem",
            ),
            [
                {
                    "type": "sign",
                    "x": 0,
                    "y": 13,
                    "elevation": 0,
                    "player_facing_dir": "BG_EVENT_PLAYER_FACING_ANY",
                    "script": "CeruleanCity_Gym_EventScript_MachinePartItem",
                }
            ],
        )
        sparkle = self.scripts.reachable_text("CeruleanCity_Gym_EventScript_TryMachinePart")
        for guard in (
            "VAR_KANTO_ROCKET_STORY_STATE, 5",
            "FLAG_HIDDEN_ITEM_MACHINE_PART",
            "checkitem ITEM_MACHINE_PART",
        ):
            self.assertIn(guard, sparkle)
        pickup = self.scripts.block("CeruleanCity_Gym_EventScript_MachinePartItem")
        self.assertOrderedText(
            pickup,
            [
                "VAR_KANTO_ROCKET_STORY_STATE, 5",
                "FLAG_HIDDEN_ITEM_MACHINE_PART",
                "checkitem ITEM_MACHINE_PART",
                "finditem ITEM_MACHINE_PART",
                "checkitem ITEM_MACHINE_PART",
                "goto_if_eq VAR_RESULT, FALSE, CeruleanCity_Gym_EventScript_DoNothing",
                "setflag FLAG_HIDDEN_ITEM_MACHINE_PART",
                "setvar VAR_KANTO_ROCKET_STORY_STATE, 6",
            ],
            "both pickup surfaces must verify delivery before hiding the part and entering state 6",
        )
        state6_writers = self.scripts.find_in(
            gym_path,
            "setvar VAR_KANTO_ROCKET_STORY_STATE, 6",
        )
        self.assertEqual(state6_writers, ["CeruleanCity_Gym_EventScript_MachinePartItem"])

    def test_manager_turn_in_is_atomic_and_restores_kanto_story(self) -> None:
        manager_path = MAPS / "Route10_PowerPlantBackRoom_hns/scripts.inc"
        manager_root = "Route10_PowerPlantBackRoom_EventScript_Manager"
        manager = self.scripts.reachable_text(manager_root)
        self.assertIn("checkitem ITEM_MACHINE_PART", manager)
        self.assertIn("checkitem ITEM_TM_THUNDER", manager)
        self.assertIn("checkitemspace ITEM_TM_THUNDER", manager)
        self.assertOrderedPath(
            manager_root,
            [
                r"checkitem\s+ITEM_TM_THUNDER",
                r"goto_if_eq\s+VAR_RESULT,\s*TRUE,\s*Route10_PowerPlantBackRoom_EventScript_ManagerCommitRepair",
                r"removeitem\s+ITEM_MACHINE_PART",
                r"goto_if_eq\s+VAR_RESULT,\s*FALSE,\s*Route10_PowerPlantBackRoom_EventScript_ManagerPartRemovalFailed",
                r"setvar\s+VAR_KANTO_ROCKET_STORY_STATE,\s*7",
            ],
            "an already-owned TM must satisfy the reward before the Machine Part is consumed",
        )
        self.assertOrderedPath(
            manager_root,
            [
                r"checkitemspace\s+ITEM_TM_THUNDER",
                r"goto_if_eq\s+VAR_RESULT,\s*FALSE,",
                r"giveitem\s+ITEM_TM_THUNDER",
                r"goto_if_eq\s+VAR_RESULT,\s*FALSE,",
                r"checkitem\s+ITEM_TM_THUNDER",
                r"goto_if_eq\s+VAR_RESULT,\s*FALSE,",
                r"removeitem\s+ITEM_MACHINE_PART",
                r"goto_if_eq\s+VAR_RESULT,\s*FALSE,",
                r"setvar\s+VAR_KANTO_ROCKET_STORY_STATE,\s*7",
            ],
            "new TM delivery and Machine Part removal must both succeed before completion",
        )

        preflight = self.scripts.block("Route10_PowerPlantBackRoom_EventScript_ManagerCheckPart")
        for forbidden in (
            "removeitem ITEM_MACHINE_PART",
            "setvar VAR_KANTO_ROCKET_STORY_STATE, 7",
            "FLAG_RETURNED_MACHINE_PART",
            "FLAG_HIDE_ROUTE25_MISTY",
            "FLAG_HIDE_POWER_PLANT_ENGINEER",
        ):
            self.assertNotIn(forbidden, preflight)
        failure = self.scripts.block("Route10_PowerPlantBackRoom_EventScript_ManagerPartRemovalFailed")
        self.assertEqual(mutation_lines(failure), [])

        commit = self.scripts.block("Route10_PowerPlantBackRoom_EventScript_ManagerCommitRepair")
        removal_gate = commit.find(
            "goto_if_eq VAR_RESULT, FALSE, Route10_PowerPlantBackRoom_EventScript_ManagerPartRemovalFailed"
        )
        self.assertNotEqual(removal_gate, -1)
        for consequence in (
            "setvar VAR_KANTO_ROCKET_STORY_STATE, 7",
            "setflag FLAG_HIDDEN_ITEM_MACHINE_PART",
            "setflag FLAG_RETURNED_MACHINE_PART",
            "clearflag FLAG_HIDE_ROUTE25_MISTY",
            "setvar VAR_CERULEAN_CITY_STATE, 2",
            "setflag FLAG_HIDE_POWER_PLANT_ENGINEER",
            "removeobject LOCALID_POWER_PLANT_BACK_ROOM_ENGINEER",
        ):
            with self.subTest(consequence=consequence):
                self.assertGreater(commit.find(consequence), removal_gate)

        back_room = load_map("Route10_PowerPlantBackRoom_hns")
        engineer = [
            obj
            for obj in back_room["object_events"]
            if obj.get("flag") == "FLAG_HIDE_POWER_PLANT_ENGINEER"
        ]
        self.assertEqual(len(engineer), 1)
        self.assertEqual(engineer[0]["local_id"], "LOCALID_POWER_PLANT_BACK_ROOM_ENGINEER")
        rear_warp = at(back_room["warp_events"], engineer[0]["x"], engineer[0]["y"])
        self.assertEqual(len(rear_warp), 1, "hiding the engineer must uncover the rear-exit warp")

        route25 = load_map("Route25_hns")
        misty_actors = [
            obj
            for obj in route25["object_events"]
            if obj.get("flag") == "FLAG_HIDE_ROUTE25_MISTY"
        ]
        self.assertEqual(len(misty_actors), 2)
        misty_scene = self.scripts.block("Route25_EventScript_Misty")
        self.assertOrderedText(
            misty_scene,
            [
                "removeobject LOCALID_ROUTE25_MISTY",
                "removeobject LOCALID_ROUTE25_MAN",
                "setflag FLAG_HIDE_ROUTE25_MISTY",
                "clearflag FLAG_HIDE_CERULEAN_GYM_TRAINERS",
                "setflag FLAG_HIDE_CERULEAN_GYM_POKEMON",
                "setvar VAR_CERULEAN_CITY_STATE, 3",
            ],
            "the state-2 cape scene must return Misty and her Trainers without completing the Gym",
        )
        self.assertNotIn("FLAG_DEFEATED_CERULEAN_GYM", misty_scene)

    def test_returned_part_consumers_and_ss_aqua_independence(self) -> None:
        returned_part_clears = []
        for path in DATA.rglob("*.inc"):
            if re.search(r"(?m)^\s*clearflag\s+FLAG_RETURNED_MACHINE_PART\b", uncommented(read(path))):
                returned_part_clears.append(path.relative_to(DATA).as_posix())
        self.assertEqual(
            returned_part_clears,
            ["scripts/new_game.inc"],
            "only new-game initialization may clear completed Power Plant restoration",
        )

        for root in (
            "Route10_PowerPlantEntrance_EventScript_Cop",
            "Route10_PowerPlantEntrance_EventScript_Engineer1",
            "Route10_PowerPlantEntrance_EventScript_Engineer2",
            "Route10_PowerPlantEntrance_EventScript_Cop2",
            "Route10_PowerPlantEntrance_EventScript_Engineer3",
        ):
            with self.subTest(power_plant=root):
                self.assertContains(
                    self.scripts.block(root),
                    r"goto_if_ge\s+VAR_KANTO_ROCKET_STORY_STATE,\s*7,",
                    "every visible Power Plant interaction must switch to terminal restored dialogue",
                )

        dialogue_roots = (
            "CeruleanCity_EventScript_CooltrainerM1",
            "Gate_SaffronCity_Route7_EventScript_Guard",
            "SaffronCity_EventScript_Lass1A",
            "SaffronCity_EventScript_PokefanM",
            "SaffronCity_EventScript_Fisher",
            "SaffronCity_PokemonCenter_EventScript_Fisher",
            "SaffronCity_CopyCatsHouse_1F_EventScript_Mom",
            "LavenderTown_PokemonCenter_EventScript_Youngster",
        )
        for root in dialogue_roots:
            with self.subTest(dialogue=root):
                self.assertIn("FLAG_RETURNED_MACHINE_PART", self.scripts.block(root))

        self.assertOrderedPath(
            "LavenderTown_RadioStation_EventScript_Director",
            [
                r"goto_if_set\s+FLAG_RETURNED_MACHINE_PART,",
                r"setflag\s+FLAG_KANTO_RADIO_GOT",
            ],
            "restored power must unlock the Kanto radio upgrade",
        )
        snorlax = self.scripts.block("VermilionCity_EventScript_Snorlax")
        self.assertIn("special CheckRadioStation", snorlax)
        self.assertIn("VermilionCity_EventScript_SnorlaxWakeUp", snorlax)

        copycat = self.scripts.block("SaffronCity_CopyCatsHouse_2F_EventScript_Copycat")
        self.assertIn("FLAG_RETURNED_MACHINE_PART", copycat)
        for map_name in ("Route5_hns", "Route6_hns"):
            blockers = [
                obj
                for obj in load_map(map_name)["object_events"]
                if obj.get("flag") == "FLAG_RETURNED_MACHINE_PART"
            ]
            self.assertTrue(blockers, f"{map_name} must hide its Underground Path blocker after repair")

        for root in (
            "SaffronStation_EventScript_Attendant",
            "GoldenrodCity_TrainStation_EventScript_Attendant",
        ):
            self.assertOrderedText(
                self.scripts.block(root),
                ["FLAG_RETURNED_MACHINE_PART", "checkitem ITEM_PASS"],
                "each station must check restored power before the Pass",
            )

        for ferry_root in (
            "OlivinePort_EventScript_Sailor",
            "VermilionPort_EventScript_Sailor",
        ):
            with self.subTest(ferry=ferry_root):
                ferry = self.scripts.reachable_text(ferry_root)
                self.assertIn("checkitem ITEM_SS_TICKET", ferry)
                self.assertNotIn("FLAG_RETURNED_MACHINE_PART", ferry)

    def test_copycat_handoffs_are_retry_safe(self) -> None:
        fan_path = MAPS / "VermilionCity_FanClub_hns/scripts.inc"
        fan_roots = self.scripts.find_in(fan_path, "ITEM_LOST_ITEM")
        self.assertTrue(fan_roots)
        fan = "\n".join(self.scripts.reachable_text(label) for label in fan_roots)
        self.assertIn("checkitemspace ITEM_LOST_ITEM", fan)
        self.assertOrderedPath(
            fan_roots[0],
            [
                r"checkitemspace\s+ITEM_LOST_ITEM",
                r"(?:giveitem|additem)\s+ITEM_LOST_ITEM",
                r"goto_if_eq\s+VAR_RESULT,\s*FALSE,",
                r"setflag\s+FLAG_HIDE_FAN_CLUB_CLEFAIRY_DOLL",
                r"setvar\s+VAR_FAN_CLUB_CLEFAIRY,\s*2",
            ],
            "the doll must remain visible until Lost Item delivery succeeds",
        )

        copycat_path = MAPS / "SaffronCity_CopyCatsHouse_2F_hns/scripts.inc"
        copycat_root = "SaffronCity_CopyCatsHouse_2F_EventScript_Copycat"
        copycat = self.scripts.reachable_text(copycat_root)
        self.assertIn("FLAG_RETURNED_MACHINE_PART", copycat)
        self.assertIn("VAR_FAN_CLUB_CLEFAIRY", copycat)
        self.assertContains(copycat, r"specialvar\s+VAR_RESULT,\s*[A-Za-z0-9_]+")
        self.assertOrderedPath(
            "SaffronCity_CopyCatsHouse_2F_EventScript_CopycatReturnDoll",
            [
                r"specialvar\s+VAR_RESULT,\s*[A-Za-z0-9_]+",
                r"goto_if_(?:eq|ne)\s+VAR_RESULT,\s*(?:FALSE|TRUE),",
                r"setvar\s+VAR_FAN_CLUB_CLEFAIRY,\s*3",
            ],
            "Copycat state 3 must commit only after the atomic Lost Item to Pass helper succeeds",
        )

    def test_both_train_stations_require_returned_part_then_pass(self) -> None:
        for map_name, root in (
            ("SaffronCity_TrainStation_hns", "SaffronStation_EventScript_Attendant"),
            ("GoldenrodCity_TrainStation_hns", "GoldenrodCity_TrainStation_EventScript_Attendant"),
        ):
            with self.subTest(map=map_name):
                body = self.scripts.block(root)
                self.assertOrderedText(
                    body,
                    ["FLAG_RETURNED_MACHINE_PART", "checkitem ITEM_PASS"],
                    "returned power must be checked before the Pass",
                )
                graph = self.scripts.reachable_text(root)
                self.assertIn("VAR_TRAIN", graph)
                self.assertNotIn("VAR_KANTO_ROCKET_STORY_STATE", graph)
                self.assertNotIn("FLAG_KANTO_RADIO_GOT", graph)

    def test_deferred_region_and_endgame_gates_are_unchanged(self) -> None:
        mahogany = load_map("Mahoganytown_hns")
        merchant = object_with_local_id(mahogany, "LOCALID_MAHOGANY_MERCHANT")
        self.assertEqual(
            (merchant["x"], merchant["y"], merchant["script"], merchant["flag"]),
            (30, 11, "MahoganyTown_EventScript_Merchant", "0"),
        )
        merchant_triggers = [
            event
            for event in mahogany["coord_events"]
            if event.get("script") == "MahoganyTown_EventScript_MerchantTrigger"
            and event.get("var") == "VAR_MAHOGANY_TOWN_STATE"
        ]
        self.assertEqual(
            {
                (event["x"], event["y"], event["var_value"], event["script"])
                for event in merchant_triggers
            },
            {
                (30, y, str(value), "MahoganyTown_EventScript_MerchantTrigger")
                for value in range(1, 17)
                for y in (12, 13, 14)
            },
        )
        self.assertIn(
            "setvar VAR_MAHOGANY_TOWN_STATE, 17",
            self.scripts.block("GoldenrodCity_RadioTower_5F_Fakeout"),
        )

        ice = load_map("IcePath_1F_hns")
        kimono = object_with_local_id(ice, "LOCALID_ICEPATH1_KIMONO")
        self.assertEqual(
            (kimono["x"], kimono["y"], kimono["script"], kimono["flag"]),
            (49, 25, "IcePath_1F_EventScript_KimonoGirl", "FLAG_HIDE_ICE_PATH_KIMONO"),
        )
        self.assertEqual(
            at(ice["coord_events"], 49, 29),
            [
                {
                    "type": "trigger",
                    "x": 49,
                    "y": 29,
                    "elevation": 0,
                    "var": "VAR_ICE_PATH_STATE",
                    "var_value": "0",
                    "script": "IcePath_1F_Trigger_KimonoGirl",
                }
            ],
        )
        self.assertIn(
            "goto_if_eq VAR_RESULT, FALSE, IcePath_1F_EventScript_KimonoGirlRefused",
            self.scripts.block("IcePath_1F_EventScript_KimonoGirlPushPrompt"),
        )
        refused = self.scripts.block("IcePath_1F_EventScript_KimonoGirlRefused")
        self.assertIn("goto_if_eq VAR_RESULT, TRUE, IcePath_1F_EventScript_KimonoGirlPush", refused)
        self.assertIn("goto_if_eq VAR_RESULT, FALSE, IcePath_1F_EventScript_KimonoGirlRefused", refused)
        self.assertOrderedText(
            self.scripts.block("IcePath_1F_EventScript_KimonoGirlPush"),
            ["setflag FLAG_HIDE_ICE_PATH_KIMONO", "setvar VAR_ICE_PATH_STATE, 1"],
            "accepting the Kimono scene must be what opens the Ice Path corridor",
        )

        blackthorn = load_map("BlackthornCity_hns")
        gym_boy = object_with_local_id(blackthorn, "LOCALID_BLACKTHORN_BOY")
        self.assertEqual(
            (gym_boy["x"], gym_boy["y"], gym_boy["script"]),
            (25, 27, "BlackthornCity_EventScript_GymBoy"),
        )
        self.assertEqual(
            at(blackthorn["warp_events"], 24, 26, dest_map="MAP_BLACKTHORN_CITY_GYM_HNS"),
            [
                {
                    "x": 24,
                    "y": 26,
                    "elevation": 0,
                    "dest_map": "MAP_BLACKTHORN_CITY_GYM_HNS",
                    "dest_warp_id": "0",
                }
            ],
        )
        self.assertOrderedText(
            self.scripts.block("BlackthornCity_OnLoad"),
            [
                "goto_if_unset FLAG_BADGE05_GET, BlackThornCity_EventScript_MoveGymBoy",
                "goto_if_unset FLAG_BADGE06_GET, BlackThornCity_EventScript_MoveGymBoy",
                "goto_if_unset FLAG_BADGE07_GET, BlackThornCity_EventScript_MoveGymBoy",
            ],
            "Blackthorn Gym admission must continue to require Badges 5, 6, and 7",
        )
        self.assertIn(
            "setobjectxyperm LOCALID_BLACKTHORN_BOY, 24, 27",
            self.scripts.block("BlackThornCity_EventScript_MoveGymBoy"),
        )
        self.assertNotIn("FLAG_BADGE08_GET", self.scripts.block("BlackthornGym_EventScript_Clair"))
        self.assertOrderedText(
            self.scripts.block("DragonsDen_Shrine_EventScript_ClairEnter"),
            ["setflag FLAG_BADGE08_GET", "addvar VAR_NUM_BADGES, 1"],
            "the Dragon's Den resolution, not Clair's Gym battle, must award Badge 8",
        )
        route13 = load_map("Route13_hns")
        captain = object_with_local_id(route13, "Route13_Alola_Captain")
        boat = object_with_local_id(route13, "LOCALID_ROUTE13_BOAT")
        self.assertEqual(
            (captain["x"], captain["y"], captain["script"]),
            (78, 26, "Route13_TravelToAlola"),
        )
        self.assertEqual((boat["x"], boat["y"], boat["flag"]), (89, 28, "FLAG_HIDE_ROUTE13_BOAT"))
        self.assertOrderedText(
            self.scripts.block("Route13_TravelToAlola"),
            [
                "checkitem ITEM_STRANGE_SOUVENIR",
                "compare VAR_RESULT, 0",
                "goto_if_ne Route13_TravelToAlola_1",
                "msgbox Route13_TravelToAlola_Text_4",
            ],
            "the Route 13 captain must refuse Alola travel without the Strange Souvenir",
        )

        snowswept = load_map("SnowsweptCavern_hns")
        machamp = object_with_local_id(snowswept, "LOCALID_SNOWSWEPT_CAVERN_MACHAMP")
        boulder = object_with_local_id(snowswept, "LOCALID_SNOWSWEPT_CAVERN_BOULDER")
        self.assertEqual(
            (machamp["x"], machamp["y"], machamp["script"]),
            (38, 49, "Snowswept_Cavern_Eventscript_Machamp"),
        )
        self.assertEqual((boulder["x"], boulder["y"], boulder["script"]), (38, 50, "NULL"))
        self.assertEqual(
            {
                (obj["x"], obj["y"], obj["script"])
                for obj in snowswept["object_events"]
                if (obj["x"], obj["y"]) in {(27, 23), (35, 31)}
            },
            {(27, 23, "EventScript_RockSmash"), (35, 31, "EventScript_RockSmash")},
        )
        self.assertOrderedText(
            self.scripts.block("Snowswept_Cavern_Eventscript_Machamp"),
            [
                "goto_if_ge VAR_SINJOH_STORYLINE, 3, Snowswept_Cavern_Eventscript_Machamp_AfterBoulder",
                "setwildbossbattle species=SPECIES_MACHAMP",
                "setobjectxyperm LOCALID_SNOWSWEPT_CAVERN_BOULDER, 38, 56",
                "setvar VAR_SINJOH_STORYLINE, 3",
            ],
            "the direct Snowswept corridor must remain gated by Machamp and its boulder",
        )

        sinjoh = load_map("NewSinjoh_hns")
        north_gate = [
            event
            for event in sinjoh["coord_events"]
            if event.get("script") == "NewSinjoh_EventScript_TriggerNorthGate"
        ]
        self.assertEqual(
            {(event["x"], event["y"], event["var"], event["var_value"]) for event in north_gate},
            {(x, 6, "VAR_SINJOH_STORYLINE", "4") for x in (32, 33, 34)},
        )
        self.assertIn(
            "applymovement LOCALID_PLAYER, Common_Movement_WalkDown1",
            self.scripts.block("NewSinjoh_EventScript_TriggerNorthGate"),
        )
        hideout = uncommented(read(MAPS / "NewSinjoh_KimonoHideout_hns/scripts.inc"))
        self.assertContains(
            hideout,
            r"map_script_2\s+VAR_SINJOH_STORYLINE,\s*4,\s*NewSinjoh_KimonoHideout_EventScript_PlayerMeetsKimonos",
        )
        self.assertContains(hideout, r"setvar\s+VAR_SINJOH_STORYLINE,\s*5")

        reception_map = load_map("ReceptionGate_hns")
        self.assertEqual(
            at(reception_map["coord_events"], 11, 14),
            [
                {
                    "type": "trigger",
                    "x": 11,
                    "y": 14,
                    "elevation": 0,
                    "var": "VAR_ROUTE27_STATE",
                    "var_value": "1",
                    "script": "ReceptionGate_Trigger",
                }
            ],
        )
        self.assertOrderedText(
            self.scripts.block("ReceptionGate_Trigger"),
            [
                "goto_if_lt VAR_ECRUTEAK_CITY_THEATER, 8, ReceptionGate_EventScript_Kimono",
                "goto_if_unset FLAG_BADGE08_GET, ReceptionGate_EventScript_NotEnough",
                "setvar VAR_ROUTE27_STATE, 2",
            ],
            "the League corridor must require the legendary story and Badge 8",
        )
        victory_road = load_map("VictoryRoadKanto_1F_hns")
        self.assertEqual(
            {
                (event["x"], event["y"], event["var"], event["var_value"], event["script"])
                for event in victory_road["coord_events"]
            },
            {(x, 7, "VAR_ROUTE27_STATE", "2", "VictoryRoadKanto_1F_Trigger") for x in (27, 28, 29)},
        )
        self.assertOrderedText(
            self.scripts.block("VictoryRoadKanto_1F_Trigger"),
            [
                "msgbox VictoryRoad_Text_RivalBefore",
                "msgbox VictoryRoad_Text_RivalAfter",
                "setvar VAR_ROUTE27_STATE, 3",
            ],
            "the final League corridor lane must retain the state-2 Silver battle",
        )

        silver_guard = [
            obj
            for obj in reception_map["object_events"]
            if obj.get("flag") == "FLAG_INDIGOJUNCTION_HIDE_SILVER_GUARD"
        ]
        self.assertEqual(
            [(obj["x"], obj["y"], obj["script"]) for obj in silver_guard],
            [(7, 7, "ReceptionGate_EventScript_SilverGuard")],
        )
        self.assertEqual(
            [
                (warp["x"], warp["y"])
                for warp in reception_map["warp_events"]
                if warp["dest_map"] == "MAP_ROUTE28_HNS"
            ],
            [(1, 9)],
        )
        self.assertIn(
            "goto_if_ge VAR_NUM_BADGES, 16, PalletTown_Lab_EventScript_Oak16Badges",
            self.scripts.block("PalletTown_Lab_EventScript_OakBadgeCheck"),
        )
        self.assertOrderedText(
            self.scripts.block("PalletTown_Lab_EventScript_Oak16Badges"),
            ["setflag FLAG_INDIGOJUNCTION_HIDE_SILVER_GUARD", "setvar VAR_PALLETTOWN_LABSTATE, 2"],
            "Oak must remain the 16-badge authority that opens Mt. Silver",
        )

        meara = self.scripts.reachable_text("MtSilver_PokemonCenter_EventScript_Meara")
        self.assertOrderedText(
            self.scripts.block("MtSilver_PokemonCenter_EventScript_Meara"),
            [
                "goto_if_ge VAR_SINJOH_STORYLINE, 1, MtSilver_PokemonCenter_EventScript_Meara_AfterUnlockSinjoh",
                "checkitem ITEM_AZURE_FLUTE",
                "goto_if_eq VAR_RESULT, TRUE, MtSilver_PokemonCenter_EventScript_Meara_BonusContentPrompt",
            ],
            "Meara must require the Azure Flute before initializing Sinjoh",
        )
        self.assertIn("setvar VAR_SINJOH_STORYLINE, 1", meara)

        snorlax = [
            obj
            for obj in load_map("VermilionCity_hns")["object_events"]
            if obj.get("flag") == "FLAG_HIDE_VERMILION_SNORLAX"
        ]
        self.assertEqual({(obj["x"], obj["y"]) for obj in snorlax}, {(61, 13), (62, 13), (60, 13), (60, 12), (62, 12)})
        self.assertIn("setwildbattle SPECIES_SNORLAX", self.scripts.reachable_text("VermilionCity_EventScript_Snorlax"))


if __name__ == "__main__":
    unittest.main()
