"""Regression gates for the selected Wayfarer Sevii Trainer roster."""

import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import generate


class SelectedRosterTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.report = generate.build_report()

    def test_frozen_dense_runtime_allocation(self):
        allocation = self.report["allocation"]
        rows = allocation["allocations"]
        self.assertEqual((allocation["base"], allocation["next_id"], len(rows)), (1515, 1651, 136))
        self.assertEqual([row["slot"] for row in rows], list(range(136)))
        self.assertEqual([row["numeric_id"] for row in rows], list(range(1515, 1651)))
        self.assertEqual(sum(row["owner"] == "ordinary_trainer" for row in rows), 118)
        self.assertEqual(sum(row["owner"] == "story" for row in rows), 18)

    def test_runtime_roster_contains_only_generated_selected_ids(self):
        roster = generate.render_runtime_roster(self.report)
        rows = self.report["allocation"]["allocations"]
        self.assertEqual(len(re.findall(r"\[DIFFICULTY_NORMAL\]\[TRAINER_WAYFARER_SEVII_", roster)), 136)
        self.assertFalse(any(f"[DIFFICULTY_NORMAL][{row['source_trainer']}]" in roster for row in rows))
        self.assertTrue(all(row["source_hash"] == generate.source_hash(generate.party_blocks()[row["source_trainer"]]) for row in rows))

    def test_runtime_constants_match_roster_extent(self):
        constants = generate.render_runtime_constants(self.report)
        self.assertEqual(constants.count("#define TRAINER_WAYFARER_SEVII_"), 138)
        self.assertIn(
            "#if IS_WAYFARER\n#define TRAINERS_COUNT_WAYFARER      1651\n#endif",
            constants,
        )

    def test_defeat_router_aliases_rematches_to_their_base_slot(self):
        router = generate.render_defeat_router(self.report)
        rows = self.report["allocation"]["allocations"]
        slots_by_source = {row["source_trainer"]: row["slot"] for row in rows if row["kind"] in ("base", "planned")}
        slots = [slots_by_source[row["defeat_base"]] for row in rows]

        self.assertEqual(len(slots), 136)
        self.assertEqual(slots[0], 0)
        self.assertEqual(slots[-1], 135)
        sharon = next(index for index, row in enumerate(rows) if row["source_trainer"] == "TRAINER_CRUSH_GIRL_SHARON")
        self.assertEqual(slots[sharon + 1], slots[sharon])
        self.assertIn("WAYFARER_SEVII_TRAINER_DEFEAT_SLOT_NONE", router)
        self.assertIn("TRAINER_WAYFARER_SEVII_FIRST", router)


if __name__ == "__main__":
    unittest.main()
