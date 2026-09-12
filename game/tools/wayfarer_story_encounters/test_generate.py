#!/usr/bin/env python3
"""Unit tests for reviewed ordinary encounter manifest invariants."""

import importlib.util
import json
from pathlib import Path
import unittest


MODULE_PATH = Path(__file__).with_name("generate.py")
SPEC = importlib.util.spec_from_file_location("wayfarer_story_encounters_generate", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)


def row(caller, command, stable_key, dialogue):
    return {
        "caller": caller,
        "command": command,
        "baseTrainer": "TRAINER_JOEY_HNS",
        "stableKey": stable_key,
        "dialogue": dialogue,
    }


class OrdinaryManifestInvariantTests(unittest.TestCase):
    def test_rematch_reuses_the_frozen_base_identity(self):
        GENERATOR.validate_rematch_dialogue_identities([
            row("Route30_EventScript_Youngster_Joey", "trainerbattle_single TRAINER_JOEY_HNS", 14136, "ORDINARY"),
            row("Route30_EventScript_RematchJoey", "trainerbattle_rematch TRAINER_JOEY_HNS", 14136, "ORDINARY"),
        ])

    def test_rematch_cannot_rekey_or_change_its_dialogue(self):
        rows = [
            row("Route30_EventScript_Youngster_Joey", "trainerbattle_single TRAINER_JOEY_HNS", 14136, "ORDINARY"),
            row("Route30_EventScript_RematchJoey", "trainerbattle_rematch TRAINER_JOEY_HNS", 99999, "ROCKET_GUARD"),
        ]

        with self.assertRaisesRegex(ValueError, "rematch dialogue must match one stable base caller"):
            GENERATOR.validate_rematch_dialogue_identities(rows)

    def test_packed_metadata_round_trips_every_reviewed_runtime_field(self):
        rows = json.loads(Path(__file__).with_name("ordinary.json").read_text())["callers"]
        self.assertEqual(len(rows), 851)
        for source in rows:
            decoded = GENERATOR.unpack_metadata(GENERATOR.pack_metadata(source))
            self.assertEqual(decoded["dialogue"], source["dialogue"])
            self.assertEqual(decoded["stableKeyModulo4"], source["stableKey"] % 4)
            self.assertEqual(decoded["lossReturn"], source["lossReturn"])
            self.assertEqual(
                decoded["allowPostBattleText"],
                not source["command"].startswith("trainerbattle_rematch"),
            )

    def test_packing_rejects_unrepresentable_metadata(self):
        source = row("Route30_EventScript_Youngster_Joey", "trainerbattle_single TRAINER_JOEY_HNS", 1, "ORDINARY")
        source["lossReturn"] = True
        source["dialogue"] = "GYM"
        with self.assertRaisesRegex(ValueError, "unsupported ordinary dialogue"):
            GENERATOR.pack_metadata(source)
        source["dialogue"] = "ORDINARY"
        source["stableKey"] = 0x10000
        with self.assertRaisesRegex(ValueError, "outside u16 range"):
            GENERATOR.pack_metadata(source)

    def test_regional_logical_sources_reject_exact_caller_collisions_and_ignore_null(self):
        hoenn = Path(__file__).parents[2] / "src/data/wayfarer_story_encounter_hoenn_entries.inc"
        source = hoenn.read_text()
        with self.assertRaisesRegex(ValueError, "ordinary/hoenn caller ownership overlaps"):
            GENERATOR.validate_regional_caller_ownership(
                {"Route119_WeatherInstitute_2F_EventScript_Shelly"},
                {"hoenn": source},
            )
        GENERATOR.validate_regional_caller_ownership({"NULL"}, {"hoenn": source})


if __name__ == "__main__":
    unittest.main()
