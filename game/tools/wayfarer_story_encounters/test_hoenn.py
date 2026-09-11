#!/usr/bin/env python3
"""Static contract audit for the current Emerald trainer-only story integration."""

from __future__ import annotations

import json
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/data/wayfarer_story_encounter_hoenn.h"
MAPS = ROOT / "data/maps"


class HoennStoryEncounterAudit(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.text = HEADER.read_text()
        cls.entries = [
            line.strip()
            for line in cls.text.splitlines()
            if line.lstrip().startswith("HOENN_ENTRY(")
        ]

    def test_registry_is_wayfarer_only_and_nonempty(self) -> None:
        self.assertIn("#if IS_WAYFARER", self.text)
        self.assertIn("#else", self.text)
        self.assertGreater(len(self.entries), 40)
        self.assertIn("gWayfarerStoryHoennEncounterCount = 0", self.text)

    def test_every_registered_caller_is_an_immediate_trainerbattle_argument(self) -> None:
        callers = re.findall(r"HOENN_ENTRY\((\w+) \+ 1,", self.text)
        # Two expanded rows now come from the generated scene projection.
        self.assertGreaterEqual(len(callers), 39)
        self.assertIn("GAMEPLAY_ENCOUNTER_ROUTE110_RIVAL_ROW", self.text)
        self.assertIn("GAMEPLAY_ENCOUNTER_RUSTURF_AQUA_ROW", self.text)
        script_files = list(MAPS.glob("*/scripts.inc"))
        for caller in callers:
            matches = [
                path for path in script_files
                if f"{caller}::" in path.read_text()
            ]
            self.assertEqual(matches.__len__(), 1, caller)
            lines = matches[0].read_text().splitlines()
            line_no = next(i for i, line in enumerate(lines) if line == f"{caller}::")
            next_instruction = next(
                line.strip()
                for line in lines[line_no + 1:]
                if line.strip() and not line.lstrip().startswith("#")
            )
            self.assertTrue(next_instruction.startswith("trainerbattle"), caller)

    def test_private_transient_object_ids_match_map_event_ordinals(self) -> None:
        expected = {
            "ROUTE110_RIVAL": ("Route110", "LOCALID_ROUTE110_RIVAL"),
            "ROUTE110_RIVAL_ON_BIKE": ("Route110", "LOCALID_ROUTE110_RIVAL_ON_BIKE"),
            "ROUTE119_RIVAL": ("Route119", "LOCALID_ROUTE119_RIVAL"),
            "ROUTE119_RIVAL_ON_BIKE": ("Route119", "LOCALID_ROUTE119_RIVAL_ON_BIKE"),
            "LILYCOVE_RIVAL": ("LilycoveCity", "LilycoveCity_EventScript_Rival"),
            "MAUVILLE_WALLY": ("MauvilleCity", "LOCALID_MAUVILLE_WALLY"),
            "VICTORY_ROAD_EXIT_WALLY": ("VictoryRoad_1F", "VictoryRoad_1F_EventScript_ExitWally"),
        }
        for name, (map_name, identifier) in expected.items():
            value = int(re.search(
                rf"#define WAYFARER_HOENN_LOCALID_{name} (\d+)", self.text
            ).group(1))
            objects = json.loads((MAPS / map_name / "map.json").read_text())["object_events"]
            index = next(
                i for i, event in enumerate(objects, 1)
                if event.get("local_id") == identifier or event.get("script") == identifier
            )
            self.assertEqual(value, index, name)

    def test_rival_object_lifecycle_matches_native_staging(self) -> None:
        route110 = [entry for entry in self.entries if "ROUTE110_RIVAL" in entry]
        route119 = [entry for entry in self.entries if "ROUTE119_RIVAL" in entry]
        self.assertGreaterEqual(len(route110), 6)
        self.assertGreaterEqual(len(route119), 7)
        self.assertIn("GAMEPLAY_ENCOUNTER_ROUTE110_RIVAL_ROW", self.text)
        self.assertTrue(any(
            "WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL_ON_BIKE" in entry
            and "WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE" in entry
            for entry in route110
        ))
        self.assertTrue(any(
            "WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE" in entry
            for entry in route119
        ))
        self.assertFalse(any(
            "WAYFARER_STORY_FLAG_TRANSIENT_OBJECT" in entry
            for entry in route119
        ))
        self.assertFalse(any(
            "WAYFARER_STORY_FLAG_TRANSIENT_OBJECT" in entry
            and "WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE" in entry
            for entry in route110 + route119
        ))

    def test_predicates_use_banked_hoenn_source_state_and_retire_completed_chapters(self) -> None:
        source_constants = (ROOT / "data/wayfarer_hoenn_source_constants.inc").read_text()
        source_symbols = (
            "FLAG_DEFEATED_RIVAL_ROUTE103",
            "FLAG_DEFEATED_RIVAL_RUSTBORO",
            "FLAG_MET_RIVAL_LILYCOVE",
            "FLAG_DEFEATED_WALLY_MAUVILLE",
            "FLAG_DEFEATED_WALLY_VICTORY_ROAD",
            "FLAG_SYS_GAME_CLEAR",
            "FLAG_DELIVERED_DEVON_GOODS",
            "FLAG_RECOVERED_DEVON_GOODS",
            "FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY",
            "FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT",
            "FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE",
            "FLAG_DEFEATED_GRUNT_SPACE_CENTER_1F",
            "FLAG_DEFEATED_MAGMA_SPACE_CENTER",
            "FLAG_RECEIVED_DEVON_SCOPE",
            "VAR_ROUTE110_STATE",
            "VAR_ROUTE119_STATE",
            "VAR_WEATHER_INSTITUTE_STATE",
            "VAR_MOSSDEEP_CITY_STATE",
            "VAR_MOSSDEEP_SPACE_CENTER_STATE",
            "VAR_SEAFLOOR_CAVERN_STATE",
        )
        predicate_bodies = "\n".join(re.findall(
            r"static bool8 WayfarerHoenn\w+(?:Pending|Eligible)\(void\)\n\{(.*?)\n\}",
            self.text,
            re.DOTALL,
        ))
        for symbol in source_symbols:
            source_value = re.search(
                rf"^#define {symbol} (0x[0-9A-F]+)$", source_constants, re.MULTILINE
            ).group(1)
            local_symbol = f"WAYFARER_HOENN_SOURCE_{symbol}"
            local_value = re.search(
                rf"{local_symbol} = (0x[0-9A-F]+)", self.text
            ).group(1)
            self.assertEqual(local_value, source_value, symbol)
            if symbol.startswith("VAR_"):
                self.assertGreaterEqual(int(local_value, 16), 0x7000, symbol)
            else:
                self.assertGreaterEqual(int(local_value, 16), 0x6000, symbol)
            self.assertIn(local_symbol, predicate_bodies, symbol)

        # The collision that motivated this check was 0x4069: HNS Goldenrod
        # and Emerald Route 110 share the unbanked source number. The registry
        # must only read Emerald's banked chapter state.
        self.assertIn("#define VAR_ROUTE110_STATE                               0x4069", (ROOT / "include/constants/vars.h").read_text())
        self.assertIn("VarGet(WAYFARER_HOENN_SOURCE_VAR_ROUTE110_STATE) == 0", self.text)
        self.assertIn("!FlagGet(WAYFARER_HOENN_SOURCE_FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT)", self.text)
        for name in re.findall(r"static bool8 (WayfarerHoenn\w+(?:Pending|Eligible))\(void\)\n\{(.*?)\n\}", self.text, re.DOTALL):
            self.assertNotRegex(name[1], r"\b(?:FLAG|VAR)_[A-Z0-9_]+\b", name[0])

    def test_every_gate_requires_explicit_allowance(self) -> None:
        expected = {
            "Route103": ("WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL",),
            "PetalburgCity": ("WAYFARER_STORY_SCENE_PETALBURG_WALLY_TUTORIAL",),
            "RustboroCity": (
                "WAYFARER_STORY_SCENE_RUSTBORO_RIVAL",
                "WAYFARER_STORY_SCENE_RUSTBORO_RIVAL_CONVERSATION",
            ),
            "Route110": ("WAYFARER_STORY_SCENE_ROUTE110_RIVAL",),
            "Route119": ("WAYFARER_STORY_SCENE_ROUTE119_RIVAL",),
            "VictoryRoad_1F": ("WAYFARER_STORY_SCENE_VICTORY_ROAD_WALLY",),
            "SlateportCity_OceanicMuseum_2F": ("WAYFARER_STORY_SCENE_OCEANIC_MUSEUM_STERN",),
            "RusturfTunnel": ("WAYFARER_STORY_SCENE_RUSTURF_AQUA",),
            "MtChimney": ("WAYFARER_STORY_SCENE_MT_CHIMNEY_MAXIE",),
            "MagmaHideout_4F": ("WAYFARER_STORY_SCENE_MAGMA_HIDEOUT_MAXIE",),
            "AquaHideout_B2F": ("WAYFARER_STORY_SCENE_AQUA_HIDEOUT_MATT",),
            "MossdeepCity_SpaceCenter_1F": ("WAYFARER_STORY_SCENE_SPACE_CENTER_STAIR_GUARD",),
            "MossdeepCity_SpaceCenter_2F": (
                "WAYFARER_STORY_SCENE_SPACE_CENTER_GRUNTS",
                "WAYFARER_STORY_SCENE_SPACE_CENTER_OFFER",
            ),
            "SeafloorCavern_Room9": ("WAYFARER_STORY_SCENE_SEAFLOOR_ARCHIE",),
            "Route120": ("WAYFARER_STORY_SCENE_ROUTE120_KECLEON",),
        }
        for map_name, scenes in expected.items():
            text = (MAPS / map_name / "scripts.inc").read_text()
            for scene in scenes:
                self.assertIn(f"setvar VAR_0x8004, {scene}", text)
            self.assertIn("specialvar VAR_RESULT, WayfarerStoryCanStartScene", text)
            self.assertIn("WAYFARER_STORY_GATE_ALLOW", text)
            self.assertIn("goto Common_EventScript_ReleaseNoOp", text)

    def test_loss_returns_are_explicit_and_exceptions_remain_excluded(self) -> None:
        loss_entries = [entry for entry in self.entries if "WAYFARER_STORY_FLAG_LOSS_RETURN" in entry]
        self.assertGreaterEqual(len(loss_entries), 20)
        for entry in loss_entries:
            loss_redirect = re.match(r"HOENN_ENTRY\([^,]+, ([^,]+),", entry).group(1)
            self.assertNotEqual(loss_redirect, "NULL", entry)
        for scene in (
            "ROUTE103_TUTORIAL",
            "PETALBURG_WALLY_TUTORIAL",
            "VICTORY_ROAD_WALLY",
            "OCEANIC_MUSEUM_STERN",
            "SPACE_CENTER_GRUNTS",
            "SPACE_CENTER_OFFER",
            "ROUTE120_KECLEON",
        ):
            self.assertFalse(any(
                scene in entry and "WAYFARER_STORY_FLAG_LOSS_RETURN" in entry
                for entry in self.entries
            ), scene)

    def test_space_center_stair_guard_precedes_all_staging(self) -> None:
        text = (MAPS / "MossdeepCity_SpaceCenter_1F" / "scripts.inc").read_text()
        start = text.index("MossdeepCity_SpaceCenter_1F_EventScript_Grunt2::")
        battle = text.index("MossdeepCity_SpaceCenter_1F_EventScript_Grunt2TrainerBattle::", start)
        gated = text[start:battle]
        self.assertIn("WAYFARER_STORY_SCENE_SPACE_CENTER_STAIR_GUARD", gated)
        self.assertIn("WAYFARER_STORY_GATE_ALLOW", gated)
        self.assertIn("EventScript_WayfarerStoryNoPartyRefusal", gated)
        self.assertLess(gated.index("WAYFARER_STORY_GATE_ALLOW"), gated.index("\tlock"))
        entry = next(entry for entry in self.entries if "SpaceCenter_1F_EventScript_Grunt2TrainerBattle" in entry)
        self.assertNotIn("WAYFARER_STORY_FLAG_LOSS_RETURN", entry)

    def test_matt_notice_is_guarded_before_lock_or_progress_write(self) -> None:
        text = (MAPS / "AquaHideout_B2F" / "scripts.inc").read_text()
        start = text.index("AquaHideout_B2F_EventScript_MattNoticePlayer::")
        battle = text.index("AquaHideout_B2F_EventScript_Matt::", start)
        staged = text[start:battle]
        self.assertIn("WAYFARER_STORY_SCENE_AQUA_HIDEOUT_MATT", staged)
        self.assertIn("EventScript_WayfarerStoryNoPartyRefusal", staged)
        self.assertLess(staged.index("WAYFARER_STORY_GATE_ALLOW"), staged.index("\tlockall"))
        self.assertLess(staged.index("WAYFARER_STORY_GATE_ALLOW"), staged.index("\tsetvar VAR_TEMP_1, 1"))

    def test_rusturf_backup_is_guarded_before_lock_or_progress_write(self) -> None:
        text = (MAPS / "RusturfTunnel" / "scripts.inc").read_text()
        start = text.index("RusturfTunnel_EventScript_AquaGruntBackUp::")
        battle = text.index("RusturfTunnel_EventScript_Grunt::", start)
        staged = text[start:battle]
        self.assertIn("WAYFARER_STORY_SCENE_RUSTURF_AQUA", staged)
        self.assertIn("EventScript_WayfarerStoryNoPartyRefusal", staged)
        self.assertLess(staged.index("WAYFARER_STORY_GATE_ALLOW"), staged.index("\tlockall"))
        self.assertLess(staged.index("WAYFARER_STORY_GATE_ALLOW"), staged.index("\tsetflag FLAG_SAFE_FOLLOWER_MOVEMENT"))
        self.assertLess(staged.index("WAYFARER_STORY_GATE_ALLOW"), staged.index("\tsetvar VAR_RUSTURF_TUNNEL_STATE, 3"))

    def test_shelly_has_no_prebattle_staging_and_uses_her_audited_loss_return(self) -> None:
        script = (MAPS / "Route119_WeatherInstitute_2F" / "scripts.inc").read_text()
        start = script.index("Route119_WeatherInstitute_2F_EventScript_Shelly::")
        defeated = script.index("Route119_WeatherInstitute_2F_EventScript_ShellyDefeated::", start)
        self.assertTrue(script[start:defeated].lstrip().startswith("Route119_WeatherInstitute_2F_EventScript_Shelly::\n\ttrainerbattle"))
        entry = next(entry for entry in self.entries if "Route119_WeatherInstitute_2F_EventScript_Shelly" in entry)
        self.assertIn("WAYFARER_STORY_FLAG_LOSS_RETURN", entry)

    def test_rustboro_keeps_match_call_conversation_and_gates_only_battle_prompts(self) -> None:
        text = (MAPS / "RustboroCity" / "scripts.inc").read_text()
        direct = text[
            text.index("RustboroCity_EventScript_Rival::"):
            text.index("RustboroCity_EventScript_RivalTrigger0::")
        ]
        self.assertNotIn("WayfarerStoryCanStartScene", direct)
        for label, prompt in (
            ("RustboroCity_EventScript_MayEncounter::", "RustboroCity_Text_MayPassedBrineyWantToBattle"),
            ("RustboroCity_EventScript_MayAskToBattle::", "RustboroCity_Text_MayWantToBattle"),
            ("RustboroCity_EventScript_BrendanEncounter::", "RustboroCity_Text_BrendanPassedBrineyWantToBattle"),
            ("RustboroCity_EventScript_BrendanAskToBattle::", "RustboroCity_Text_BrendanWantToBattle"),
        ):
            prompt_block = text[text.index(label):text.index(prompt, text.index(label))]
            self.assertIn("WAYFARER_STORY_SCENE_RUSTBORO_RIVAL_CONVERSATION", prompt_block)
            self.assertIn("WAYFARER_STORY_GATE_ALLOW", prompt_block)
            self.assertIn("EventScript_WayfarerStoryNoPartyRefusal", prompt_block)

    def test_weather_security_callers_have_one_registry_owner(self) -> None:
        gameplay = json.loads((MAPS / "Route119_WeatherInstitute_1F" / "gameplay.json").read_text())
        second_floor = json.loads((MAPS / "Route119_WeatherInstitute_2F" / "gameplay.json").read_text())
        declarations = {entry["caller"]["label"]: entry for entry in gameplay["encounters"] + second_floor["encounters"]}
        for caller in (
            "Route119_WeatherInstitute_1F_EventScript_Grunt1",
            "Route119_WeatherInstitute_1F_EventScript_Grunt4",
            "Route119_WeatherInstitute_2F_EventScript_Grunt2",
            "Route119_WeatherInstitute_2F_EventScript_Grunt3",
            "Route119_WeatherInstitute_2F_EventScript_Grunt5",
        ):
            self.assertNotIn(caller, self.text)
            self.assertEqual(declarations[caller]["profile"]["family"], "ordinary")

    def test_route104_is_not_claimed_by_current_hoenn_scope(self) -> None:
        for map_name in ("Route104", "JaggedPass", "MeteorFalls_1F_1R", "LavaridgeTown"):
            self.assertNotIn("WayfarerStoryCanStartScene", (MAPS / map_name / "scripts.inc").read_text())
        self.assertNotIn("Route104_", self.text)


if __name__ == "__main__":
    unittest.main()
