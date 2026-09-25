"""Check narrow FRLG source import and the collision-audited runtime IDs."""

import re
import unittest
import generate


class IndigoTrainerTests(unittest.TestCase):
    def test_dense_range_follows_existing_wayfarer_allocations(self):
        rows = generate.rows()
        tower = (generate.GAME / "include/constants/wayfarer_tower_trainers.h").read_text()
        self.assertIn("#define TRAINER_WAYFARER_TOWER_LAST 1794", tower)
        self.assertEqual([row["numeric_id"] for row in rows], list(range(1795, 1800)))
        self.assertEqual(len({row["id"] for row in rows}), 5)

    def test_compiled_roster_uses_only_wayfarer_ids_and_blastoise_default(self):
        roster = generate.render_roster()
        self.assertEqual(len(re.findall(r"\[TRAINER_WAYFARER_INDIGO_", roster)), 5)
        self.assertFalse(any(f"[{source}]" in roster for _, source in generate.SOURCES))
        blue = roster.split("[TRAINER_WAYFARER_INDIGO_BLUE] =", 1)[1]
        self.assertIn('.trainerName = _("BLUE")', blue)
        self.assertIn(".species = SPECIES_BLASTOISE", blue)
        self.assertNotIn("VAR_STARTER_MON", blue)

    def test_generated_files_match_source(self):
        self.assertEqual(generate.CONSTANTS.read_text(), generate.render_constants())
        self.assertEqual(generate.ROSTER.read_text(), generate.render_roster())


if __name__ == "__main__":
    unittest.main()
