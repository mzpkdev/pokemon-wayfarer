#!/usr/bin/env python3
"""Static contracts for the audited Johto/HNS trainer-only story callers."""

import json
from pathlib import Path
import subprocess
import unittest


GAME = Path(__file__).resolve().parents[2]
MAPS = GAME / "data" / "maps"
MANIFEST = GAME / "src" / "data" / "wayfarer_story_encounter_johto.h"


def read(relative):
    return (MAPS / relative / "scripts.inc").read_text(encoding="utf-8")


class JohtoStoryContractTests(unittest.TestCase):
    def test_manifest_covers_only_audited_current_hns_scenes(self):
        manifest = MANIFEST.read_text(encoding="utf-8")
        for scene in (
            "SILVER_CHERRYGROVE", "SILVER_AZALEA", "SILVER_BURNED_TOWER",
            "SILVER_GOLDENROD_UNDERGROUND", "SILVER_VICTORY_ROAD",
            "SILVER_MT_MOON", "SILVER_INDIGO", "SLOWPOKE_WELL_PROTON",
            "MAHOGANY_PASSWORD_GRUNT_F", "MAHOGANY_PASSWORD_GRUNT_M",
            "MAHOGANY_PETREL", "RADIO_FAKE_DIRECTOR_PETREL",
            "RADIO_EXECUTIVE_PROTON", "RADIO_EXECUTIVE_ARIANA", "RADIO_ARCHER",
            "THEATER_ROCKET", "SPROUT_ELDER_LI", "ROUTE24_ROCKET",
            "VIRIDIAN_BLUE", "TOHJO_GIOVANNI_EXCEPTION", "SSAQUA_STANLY",
            "RADIO_1F_GRUNT",
        ):
            self.assertIn(f"WAYFARER_STORY_SCENE_{scene}", manifest)
        self.assertNotIn("MAP_PALLET_TOWN", manifest)
        self.assertNotIn("MAP_SEVII", manifest)

    def test_silver_stages_use_strict_allow_gates(self):
        for map_name, label in (
            ("CherrygroveCity_hns", "CherryGroveCity_EventScript_TriggerSilver"),
            ("AzaleaTown_hns", "AzaleaTown_EventScript_SilverTriggerTop"),
            ("GoldenrodCity_UndergroundSwitches_hns", "GoldenrodCity_UndergroundSwitches_EventScript_SilverTriggerTop"),
            ("MtMoon_Cave_hns", "MtMoon_Cave_EventScript_Silver"),
            ("IndigoPlateau_PokemonCenter_hns", "IndigoPlateau_EventScript_Silver"),
        ):
            script = read(map_name)
            start = script.index(f"{label}::")
            gate_end = script.index("Allowed::", start) + len("Allowed::")
            window = script[start:gate_end]
            allowed = script[gate_end:gate_end + 240]
            self.assertIn("WayfarerStoryCanStartScene", window)
            self.assertIn("WAYFARER_STORY_GATE_ALLOW", window)
            self.assertIn("lock", allowed)

    def test_azalea_retry_is_independent_of_host_progress_and_its_own_retirement(self):
        event_map = json.loads((MAPS / "AzaleaTown_hns" / "map.json").read_text(encoding="utf-8"))
        retry = [
            event for event in event_map["coord_events"]
            if event["script"] == "AzaleaTown_EventScript_SilverTriggerTop"
        ]
        self.assertEqual(
            retry,
            [{
                "type": "trigger", "x": 11, "y": 16, "elevation": 0,
                "var": "VAR_TEMP_C", "var_value": "0",
                "script": "AzaleaTown_EventScript_SilverTriggerTop",
            }],
        )
        script = read("AzaleaTown_hns")
        self.assertIn("setvar VAR_TEMP_C, 0", script)
        self.assertIn("FLAG_WAYFARER_SILVER_AZALEA_COMPLETE", script)
        self.assertIn("goto_if_ge VAR_AZALEA_TOWN_STATE, 5", script)
        self.assertIn("WayfarerSilverAzaleaIsPending", MANIFEST.read_text(encoding="utf-8"))
        native = subprocess.run(
            [
                "cpp", "-P", "-x", "assembler-with-cpp", f"-I{GAME / 'include'}",
                "-DIS_WAYFARER=0", "-DPOKEMON_WAYFARER=0", "-DPOKEMON_HNS=1",
                str(MAPS / "AzaleaTown_hns" / "scripts.inc"),
            ],
            check=True,
            text=True,
            capture_output=True,
        ).stdout
        native_trigger = native.split("AzaleaTown_EventScript_SilverTriggerTop:", 1)[1].split(
            "AzaleaTown_EventScript_SilverTriggerTopAllowed:", 1
        )[0]
        self.assertIn("goto_if_ne VAR_AZALEA_TOWN_STATE, 5", native_trigger)

    def test_standalone_hns_preprocessing_has_no_wayfarer_story_specials(self):
        for script_path in MAPS.glob("*_hns/scripts.inc"):
            source = subprocess.run(
                [
                    "cpp", "-P", "-x", "assembler-with-cpp", f"-I{GAME / 'include'}",
                    "-DIS_WAYFARER=0", "-DPOKEMON_WAYFARER=0", "-DPOKEMON_HNS=1",
                    str(script_path),
                ],
                check=True,
                text=True,
                capture_output=True,
            ).stdout
            self.assertNotIn("WayfarerStory", source, script_path)

    def test_mahogany_nonbattle_scene_retries_after_host_progress(self):
        event_map = json.loads((MAPS / "RocketHideout_B3F_hns" / "map.json").read_text(encoding="utf-8"))
        retry = [
            event for event in event_map["coord_events"]
            if event["script"] == "RocketHideout_B3F_Trigger_Silver"
        ]
        self.assertEqual(
            retry,
            [{
                "type": "trigger", "x": 8, "y": 13, "elevation": 0,
                "var": "VAR_TEMP_C", "var_value": "0",
                "script": "RocketHideout_B3F_Trigger_Silver",
            }],
        )
        script = read("RocketHideout_B3F_hns")
        scene = script.split("RocketHideout_B3F_Trigger_Silver::", 1)[1].split(
            "RocketHideout_B3F_Movement_SilverEnter:", 1
        )[0]
        self.assertIn("setvar VAR_TEMP_C, 0", script)
        self.assertIn("goto_if_set FLAG_WAYFARER_SILVER_MAHOGANY_SCENE_COMPLETE", scene)
        self.assertIn("goto_if_unset FLAG_JOHTO_STARTER_CHOICE_COMMITTED", scene)
        self.assertIn("goto_if_lt VAR_MAHOGANY_TOWN_STATE, 7", scene)
        self.assertNotIn("WayfarerStoryCanStartScene", scene)
        self.assertIn("goto_if_ge VAR_MAHOGANY_TOWN_STATE, 8", scene)
        native = subprocess.run(
            [
                "cpp", "-P", "-x", "assembler-with-cpp", f"-I{GAME / 'include'}",
                "-DIS_WAYFARER=0", "-DPOKEMON_WAYFARER=0", "-DPOKEMON_HNS=1",
                str(MAPS / "RocketHideout_B3F_hns" / "scripts.inc"),
            ],
            check=True,
            text=True,
            capture_output=True,
        ).stdout
        native_scene = native.split("RocketHideout_B3F_Trigger_Silver:", 1)[1].split(
            "RocketHideout_B3F_Movement_SilverEnter:", 1
        )[0]
        self.assertIn("goto_if_ne VAR_MAHOGANY_TOWN_STATE, 7", native_scene)
        self.assertNotIn("FLAG_WAYFARER", native_scene)

    def test_burned_tower_keeps_discovery_and_silver_completion_separate(self):
        script = read("BurnedTower_1F_hns")
        discovery = script.index("BurnedTower_1F_EventScript_LocalDiscovery::")
        drop = script.index("BurnedTower_1F_EventScript_DropToB1F::")
        self.assertIn("FLAG_WAYFARER_BURNED_TOWER_DISCOVERED", script[discovery:drop])
        self.assertNotIn("FLAG_WAYFARER_SILVER_BURNED_TOWER_COMPLETE", script[discovery:drop])

    def test_objective_loss_redirects_do_not_write_victory_state(self):
        script = read("RocketHideout_B3F_hns")
        retreat = script.index("RocketHideout_B3F_EventScript_PetrelRetreat::")
        body = script[retreat:retreat + 700]
        self.assertNotIn("VAR_ROCKET_PASSWORD", body)
        self.assertNotIn("VAR_MAHOGANY_TOWN_STATE", body)
        self.assertIn("WayfarerStoryShowRetreatDialogue", body)

    def test_completed_objective_callers_keep_earned_aftertext_without_rebattle(self):
        direct_callers = (
            "RocketHideout_B3F_EventScript_GruntF5",
            "RocketHideout_B3F_EventScript_Eto",
            "GoldenrodRaidoTower4_EventScript_Proton",
            "GoldenrodRaidoTower4_EventScript_ARIANA",
        )
        manifest = MANIFEST.read_text(encoding="utf-8")
        for caller in direct_callers:
            entry = next(line for line in manifest.splitlines() if caller in line and "WAYFARER_JOHTO_ENTRY" in line)
            self.assertIn("WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT", entry)

        completed_paths = (
            ("SlowpokeWell_B1F_hns", "SlowpokeWell_B1F_EventScript_Proton", "TRAINER_PROTON_1_HNS", "SlowpokeWell_B1F_EventScript_ProtonAlreadyDefeated"),
            ("RocketHideout_B3F_hns", "RocketHideout_B3F_EventScript_Giovanni", "TRAINER_PETREL_1_HNS", "RocketHideout_B3F_EventScript_GiovanniAlreadyDefeated"),
            ("GoldenrodCity_RadioTower_5F_hns", "GoldenrodCity_RadioTower_5F_EventScript_Petrel", "TRAINER_PETREL_2_HNS", "GoldenrodCity_RadioTower_5F_EventScript_Petrel2"),
            ("GoldenrodCity_RadioTower_5F_hns", "GoldenrodCity_RadioTower_5F_EventScript_Archer", "TRAINER_ARCHER_HNS", "GoldenrodCity_RadioTower_5F_EventScript_ArcherAlreadyDefeated"),
            ("EcruteakCity_Theater_hns", "EcruteakCity_Theater_EventScript_Rocket", "TRAINER_GRUNT_33_HNS", "EcruteakCity_Theater_EventScript_RocketAlreadyDefeated"),
            ("SproutTower_3F_hns", "SproutTower_3F_EventScript_SageLi", "TRAINER_LI_HNS", "SproutTower_3F_EventScript_SageLi_Beaten"),
            ("Route24_hns", "Route24_EventScript_Grunt", "TRAINER_GRUNT_31_HNS", "Route24_EventScript_GruntAlreadyDefeated"),
            ("SSAqua_RoomNW_hns", "SSAqua_RoomNW_EventScript_Stanly", "TRAINER_STANLY_HNS", "SSAqua_RoomNW_EventScript_StanlyAlreadyDefeated"),
            ("GoldenrodCity_RadioTower_1F_hns", "RadioTower1F_EventScript_Grunt", "TRAINER_GRUNT_2_HNS", "RadioTower1F_EventScript_GruntAlreadyDefeated"),
            ("ViridianCity_Gym_hns", "ViridianCity_Gym_EventScript_Blue", "TRAINER_BLUE_HNS", "ViridianCity_Gym_EventScript_Blue_Defeated"),
        )
        for map_name, caller, trainer, aftertext in completed_paths:
            script = read(map_name)
            caller_body = script.split(f"{caller}::", 1)[1].split("#endif", 1)[0]
            self.assertIn(f"goto_if_defeated {trainer}, {aftertext}", caller_body)
            self.assertNotIn("checktrainerflag", caller_body)
            after_body = script.split(f"{aftertext}::", 1)[1].split("\n\n", 1)[0]
            self.assertNotIn("trainerbattle", after_body, aftertext)

    def test_electrode_exception_has_no_trainer_only_gate(self):
        script = read("RocketHideout_B2F_hns")
        electrode = script.index("RocketHideout_B2F_Trigger_Electrode")
        self.assertNotIn("WAYFARER_STORY", script[electrode:electrode + 2200])


if __name__ == "__main__":
    unittest.main()
