#!/usr/bin/env python3
"""Unit tests for reviewed ordinary encounter manifest invariants."""

import importlib.util
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


if __name__ == "__main__":
    unittest.main()
