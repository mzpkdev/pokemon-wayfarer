import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock


GAME = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_content_audit", GAME / "tools/wayfarer_sevii_content/audit.py")
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)
GENERATOR_SPEC = importlib.util.spec_from_file_location(
    "wayfarer_sevii_trainer_content_generate", GAME / "tools/wayfarer_sevii_trainer_content/generate.py")
GENERATOR = importlib.util.module_from_spec(GENERATOR_SPEC)
GENERATOR_SPEC.loader.exec_module(GENERATOR)


class OrdinaryTrainerProjectionTests(unittest.TestCase):
    def test_frozen_ordinary_inventory_and_pair_aliases(self):
        report = AUDIT.build_report(GAME)["ordinary_trainers"]
        self.assertTrue(report["enabled"])
        self.assertEqual((report["objects"], report["base_identities"], report["single_objects"]), (87, 81, 75))
        self.assertEqual((len(report["pairs"]), report["sight_objects"], report["talk_objects"]), (6, 86, 1))
        self.assertEqual((report["rematch_objects"], report["single_stage_objects"]), (70, 17))
        self.assertEqual((report["party_allocations"], report["base_allocations"]), (118, 81))
        self.assertEqual(report["maps"]["SevenIsland_TrainerTower_Frlg"], 2)
        self.assertTrue(all(len(pair["members"]) == 2 for pair in report["pairs"]))
        self.assertTrue(all(len(pair["local_ids"]) == 2 and pair["local_ids"][0] > 0 for pair in report["pairs"]))

    def test_generator_preserves_foreign_contract_rows_on_write_and_check(self):
        manifest = json.loads((GAME / "src/data/wayfarer_sevii_maps.json").read_text(encoding="utf-8"))
        story_allocation = {"owner": "story", "slot": 900, "id": "TRAINER_WAYFARER_SEVII_STORY_FIXTURE"}
        tower_state = {"owner": "trainer_tower", "storage": "var", "slot": 31, "id": "SEVII_TOWER_FIXTURE"}
        manifest["contracts"]["trainer_ids"]["allocations"].append(story_allocation)
        manifest["contracts"]["states"].append(tower_state)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, output = root / "manifest.json", root / "ordinary.inc"
            source.write_text(json.dumps(manifest), encoding="utf-8")
            with mock.patch.object(GENERATOR, "MANIFEST", source), mock.patch.object(GENERATOR, "OUTPUT", output):
                with mock.patch.object(sys, "argv", ["generate.py"]):
                    self.assertEqual(GENERATOR.main(), 0)
                updated = json.loads(source.read_text(encoding="utf-8"))
                self.assertIn(story_allocation, updated["contracts"]["trainer_ids"]["allocations"])
                self.assertIn(tower_state, updated["contracts"]["states"])
                with mock.patch.object(sys, "argv", ["generate.py", "--check"]):
                    self.assertEqual(GENERATOR.main(), 0)


if __name__ == "__main__":
    unittest.main()
