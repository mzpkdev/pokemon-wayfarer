import importlib.util
from pathlib import Path
import unittest


GAME = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_content_audit", GAME / "tools/wayfarer_sevii_content/audit.py")
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


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


if __name__ == "__main__":
    unittest.main()
