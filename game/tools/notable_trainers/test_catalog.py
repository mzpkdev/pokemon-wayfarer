"""Reject authored content that the runtime or pool rules cannot accept."""

import copy
import json
import unittest

import generate


class CatalogValidationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads(generate.CATALOG.read_text())

    def with_pool(self, pool):
        catalog = copy.deepcopy(self.catalog)
        catalog["trainers"][0]["movePool"] = pool
        return catalog

    def test_current_catalog(self):
        generate.validate(self.catalog)

    def test_runtime_pool_capacity(self):
        pool = [{"move": "Bind"}] * 32
        generate.validate(self.with_pool(pool))
        with self.assertRaisesRegex(ValueError, "1..32 entries"):
            generate.validate(self.with_pool(pool + [{"move": "Bind"}]))

    def test_from_level_is_explicit_integer_in_range(self):
        for value in (0, -1, 101, True, 1.5, "20", None):
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, "fromLevel"):
                generate.validate(self.with_pool([{"move": "Bind", "fromLevel": value}]))
        for entry in ({"move": "Bind"}, {"move": "Bind", "fromLevel": 1},
                      {"move": "Bind", "fromLevel": 100}):
            generate.validate(self.with_pool([entry]))

    def test_multiple_frustration_categories(self):
        with self.assertRaisesRegex(ValueError, "at most one frustration category"):
            generate.validate(self.with_pool([{"move": "Bind"}, {"move": "Hypnosis"}]))

    def test_evasion_cannot_pair_with_toxic(self):
        for move in ("Toxic", "Toxic Spikes"):
            with self.subTest(move=move), self.assertRaisesRegex(ValueError, "never pairs"):
                generate.validate(self.with_pool([{"move": "Double Team"}, {"move": move}]))

    def test_move_name_normalization_does_not_bypass_pool_rules(self):
        with self.assertRaisesRegex(ValueError, "never pairs"):
            generate.validate(self.with_pool([{"move": "Double-Team"}, {"move": "toxic"}]))

    def test_secondary_effects_and_same_category_are_allowed(self):
        generate.validate(self.with_pool([{"move": move} for move in
                          ("Hypnosis", "Yawn", "Hurricane", "Muddy Water", "Toxic")]))

    def test_none_is_not_a_move(self):
        with self.assertRaisesRegex(ValueError, "invalid move pool"):
            generate.validate(self.with_pool([{"move": "None"}]))


if __name__ == "__main__":
    unittest.main()
