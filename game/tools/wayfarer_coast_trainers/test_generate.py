"""Regression gates for the selected Wayfarer Kanto coast Trainer roster."""

import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import generate


class CoastRosterTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rows = generate.build_inventory()

    def test_dense_collision_free_allocation(self):
        self.assertEqual(len(self.rows), 56)
        self.assertEqual([row["numeric_id"] for row in self.rows], list(range(1651, 1707)))
        self.assertEqual(sum(row["kind"] == "base" for row in self.rows), 45)
        self.assertEqual(sum(row["kind"] == "rematch" for row in self.rows), 11)
        self.assertEqual(sum(row["policy"] == "GYM_MEMBER" for row in self.rows), 7)
        self.assertEqual(sum(row["policy"] == "GYM_LEADER" for row in self.rows), 1)

    def test_compiled_roster_only_uses_runtime_ids(self):
        roster = generate.render_roster(self.rows)
        self.assertEqual(len(re.findall(r"\[DIFFICULTY_NORMAL\]\[TRAINER_WAYFARER_COAST_", roster)), 56)
        self.assertFalse(any(
            f"[DIFFICULTY_NORMAL][{row['source_trainer']}]" in roster
            for row in self.rows
        ))

    def test_staged_parties_share_their_base_defeat_identity(self):
        router = generate.render_defeat_router(self.rows)
        self.assertIn("    8,\n    13,\n    18,\n    18,", router)
        self.assertIn("    28,\n    28,", router)


if __name__ == "__main__":
    unittest.main()
