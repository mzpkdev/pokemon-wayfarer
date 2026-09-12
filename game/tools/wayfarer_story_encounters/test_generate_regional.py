#!/usr/bin/env python3
"""Equivalence checks for the compact regional registry source fixtures."""

import importlib.util
from pathlib import Path
import unittest


MODULE_PATH = Path(__file__).with_name("generate_regional.py")
SPEC = importlib.util.spec_from_file_location("wayfarer_story_encounters_generate_regional", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)


class RegionalRegistryGenerationTests(unittest.TestCase):
    def test_logical_source_keeps_original_counts_and_shared_descriptor_count(self):
        expected = {"johto": (41, 25), "hoenn": (49, 29)}
        for region, (row_count, descriptor_count) in expected.items():
            entries = (GENERATOR.DATA / f"wayfarer_story_encounter_{region}_entries.inc").read_text()
            rows = GENERATOR.logical_rows(region, entries)
            self.assertEqual(len(rows), row_count)
            self.assertEqual(len({descriptor for _, descriptor in rows}), descriptor_count)

    def test_generated_registry_round_trips_its_independent_logical_source(self):
        for region in GENERATOR.REGIONS:
            preamble = (GENERATOR.DATA / f"wayfarer_story_encounter_{region}_preamble.h").read_text()
            entries = (GENERATOR.DATA / f"wayfarer_story_encounter_{region}_entries.inc").read_text()
            generated, _, _ = GENERATOR.render(region, preamble, entries)
            current = (GENERATOR.DATA / f"wayfarer_story_encounter_{region}.h").read_text()
            self.assertEqual(current, generated)

    def test_callerless_hoenn_rows_keep_their_trigger_and_stable_key(self):
        entries = (GENERATOR.DATA / "wayfarer_story_encounter_hoenn_entries.inc").read_text()
        rows = GENERATOR.logical_rows("hoenn", entries)
        callerless = {descriptor[3]: descriptor for caller, descriptor in rows if caller == "NULL"}
        expected = {
            "WAYFARER_STORY_SCENE_RUSTBORO_RIVAL_CONVERSATION": ("NULL", "1"),
            "WAYFARER_STORY_SCENE_ROUTE110_RIVAL": ("NULL", "2"),
            "WAYFARER_STORY_SCENE_ROUTE119_RIVAL": ("NULL", "3"),
            "WAYFARER_STORY_SCENE_PETALBURG_WALLY_TUTORIAL": ("PetalburgCity_EventScript_WallyTutorial", "0"),
            "WAYFARER_STORY_SCENE_OCEANIC_MUSEUM_STERN": ("SlateportCity_OceanicMuseum_2F_EventScript_CaptStern", "0"),
            "WAYFARER_STORY_SCENE_SPACE_CENTER_GRUNTS": ("MossdeepCity_SpaceCenter_2F_EventScript_ThreeMagmaGrunts", "0"),
            "WAYFARER_STORY_SCENE_SPACE_CENTER_OFFER": ("MossdeepCity_SpaceCenter_2F_EventScript_Steven", "0"),
            "WAYFARER_STORY_SCENE_ROUTE120_KECLEON": ("Route120_EventScript_Steven", "0"),
        }
        for scene, (trigger, stable_key) in expected.items():
            descriptor = callerless[scene]
            self.assertEqual(descriptor[1], trigger, scene)
            self.assertEqual(descriptor[2], stable_key, scene)


if __name__ == "__main__":
    unittest.main()
