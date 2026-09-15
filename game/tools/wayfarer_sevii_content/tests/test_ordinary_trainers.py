import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock


GAME = Path(__file__).resolve().parents[3]
GENERATOR_SPEC = importlib.util.spec_from_file_location(
    "wayfarer_sevii_trainer_content_generate", GAME / "tools/wayfarer_sevii_trainer_content/generate.py")
GENERATOR = importlib.util.module_from_spec(GENERATOR_SPEC)
GENERATOR_SPEC.loader.exec_module(GENERATOR)


class OrdinaryTrainerProjectionTests(unittest.TestCase):
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
