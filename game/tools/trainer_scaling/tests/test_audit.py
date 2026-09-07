import sys
import unittest
from fractions import Fraction
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import audit


class ProjectionAuditTests(unittest.TestCase):
    def test_all_levels_ratings_roles_against_fraction_oracle(self):
        for rating in range(81):
            (r0, b0), (r1, b1) = next((a, b) for a, b in zip(audit.ANCHORS, audit.ANCHORS[1:]) if a[0] <= rating <= b[0])
            value = Fraction(b0) + Fraction((rating - r0) * (b1 - b0), r1 - r0)
            baseline = (value + Fraction(1, 2)).numerator // (value + Fraction(1, 2)).denominator
            for authored in range(1, 101):
                delta = Fraction(authored - 5, 5)
                magnitude = abs(delta) + Fraction(1, 2)
                adjustment = max(-1, min(8, (magnitude.numerator // magnitude.denominator) * (-1 if delta < 0 else 1)))
                for policy, bonus in (("ORDINARY", 0), ("GYM_MEMBER", 2)):
                    expected = min(100, max(1, baseline + adjustment + bonus))
                    self.assertEqual(audit.project(rating, authored, policy), expected)
                    if rating:
                        self.assertGreaterEqual(expected, audit.project(rating - 1, authored, policy))

    def test_moves_skip_evolution_entries_and_ignore_known_duplicates(self):
        schedule = [(0, "EVOLVE"), (1, "A"), (2, "B"), (3, "C"), (4, "D"), (5, "A"), (6, "E"), (7, "A")]
        self.assertEqual(audit.initial_moves(schedule, 5), ["A", "B", "C", "D"])
        self.assertEqual(audit.initial_moves(schedule, 7), ["C", "D", "E", "A"])

    def test_both_modes_exhaust_every_pool_slot_and_empty_moves_fail(self):
        data = {"SPECIES_TEST": {"abilities": ["ABILITY_ONE"], "gender": "MON_GENDERLESS", "base_exp": 10, "bst": 500}}
        schedules = {("SPECIES_TEST", "normal"): [(1, "MOVE_HIT")], ("SPECIES_TEST", "legacy"): []}
        records = {"TRAINER_TEST": {"0": {"partySize": 1, "poolSize": 2, "slots": [{"species": "SPECIES_TEST", "lvl": 5}, {"species": "SPECIES_TEST", "lvl": 50, "ability": "ABILITY_BAD", "gender": "TRAINER_MON_MALE"}]}}}
        metadata = [{"species": "SPECIES_TEST", "predecessor": "SPECIES_NONE", "predecessor_level": 0}]
        with patch.object(audit, "load_data", return_value=(data, schedules, {"MOVE_HIT": 10})), patch.object(audit.wild, "load_trainer_species_metadata", return_value=metadata):
            report = audit.build_audit(records, {"records": [{"id": "TRAINER_TEST", "policy": "ORDINARY"}], "move_exceptions": []})
        self.assertEqual(report["evaluated_slot_ratings_modes"], 324)
        self.assertEqual(len(report["structural_failures"]), 162)
        outcome = report["outcomes"][report["projections"][report["slots"][1]["projection"]]["normal"][0]["outcome"]]
        self.assertTrue(outcome["ability_fallback"])
        self.assertTrue(outcome["gender_adjustment"])
        self.assertTrue(outcome["high_bst_no_predecessor"])

    def test_active_data_covers_both_learnsets(self):
        species, schedules, moves = audit.load_data()
        self.assertGreater(len(species), 1000)
        self.assertEqual(species["SPECIES_BULBASAUR"]["bst"], 318)
        self.assertIn("ABILITY_OVERGROW", species["SPECIES_BULBASAUR"]["abilities"])
        self.assertNotEqual(schedules["SPECIES_BULBASAUR", "normal"], schedules["SPECIES_BULBASAUR", "legacy"])
        self.assertGreater(moves["MOVE_TACKLE"], 0)

    def test_reviewed_exception_requires_unchanged_species_and_active_level(self):
        data = {name: {"abilities": ["ABILITY_ONE"], "gender": "MON_MALE", "base_exp": 10, "bst": 100} for name in ("SPECIES_BASE", "SPECIES_EVOLVED")}
        schedules = {(name, mode): [(1, "MOVE_HIT"), (10 if mode == "normal" else 12, "MOVE_LATER")] for name in data for mode in ("normal", "legacy")}
        records = {"TRAINER_ALIAS": {"0": {"owner": "TRAINER_OWNER", "partySize": 1, "slots": [{"species": "SPECIES_EVOLVED", "lvl": 5, "moves": ["MOVE_LATER"]}]}}}
        metadata = [{"species": "SPECIES_BASE", "predecessor": "SPECIES_NONE", "predecessor_level": 0}, {"species": "SPECIES_EVOLVED", "predecessor": "SPECIES_BASE", "predecessor_level": 10}]
        manifest = {"records": [{"id": "TRAINER_ALIAS", "policy": "ORDINARY"}], "move_exceptions": [{"owner": "TRAINER_OWNER", "variant": "0", "slot": 0, "reason": "fixture"}]}
        with patch.object(audit, "load_data", return_value=(data, schedules, {"MOVE_HIT": 10, "MOVE_LATER": 10})), patch.object(audit.wild, "load_trainer_species_metadata", return_value=metadata):
            report = audit.build_audit(records, manifest)
        projection = report["projections"][0]
        at = lambda mode, rating: next(report["outcomes"][row["outcome"]] for row in projection[mode] if row["rating_start"] <= rating <= row["rating_end"])
        self.assertEqual(at("normal", 0)["species"], "SPECIES_BASE")
        self.assertFalse(at("normal", 0)["authored_moves_retained"])
        self.assertTrue(at("normal", 8)["authored_moves_retained"])
        self.assertFalse(at("legacy", 8)["authored_moves_retained"])
        self.assertTrue(at("legacy", 16)["authored_moves_retained"])


if __name__ == "__main__":
    unittest.main()
