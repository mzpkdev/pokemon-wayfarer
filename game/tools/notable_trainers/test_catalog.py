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

    def without_natures(self):
        catalog = copy.deepcopy(self.catalog)
        for trainer in catalog["trainers"]:
            for slot in trainer["roster"]:
                slot["nature"] = None
        return catalog

    def test_authored_nature_reaches_generated_roster(self):
        catalog = self.without_natures()
        slot = catalog["trainers"][0]["roster"][0]
        for nature in ("Adamant", "Hardy"):
            with self.subTest(nature=nature):
                slot["nature"] = nature
                generate.validate(catalog)
                _, table, _ = generate.generate(catalog)
                self.assertIn(f".nature = NATURE_{nature.upper()}", table)

    def test_unset_nature_preserves_constructor_default(self):
        catalog = self.without_natures()
        slot = catalog["trainers"][0]["roster"][0]
        slot["nature"] = None
        expected = generate.generate(catalog)
        del slot["nature"]
        generate.validate(catalog)
        self.assertEqual(generate.generate(catalog), expected)
        self.assertNotIn(".nature =", expected[1])

    def test_invalid_nature_is_rejected(self):
        for nature in ("Unknown", "Random", "May Synchronize", "", 0, True):
            with self.subTest(nature=nature), self.assertRaisesRegex(ValueError, "unknown nature"):
                catalog = copy.deepcopy(self.catalog)
                catalog["trainers"][0]["roster"][0]["nature"] = nature
                generate.validate(catalog)

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

    def with_voice(self, **changes):
        catalog = copy.deepcopy(self.catalog)
        catalog["trainers"][0]["voice"].update(changes)
        return catalog

    def test_every_trainer_has_the_six_wired_voice_bits(self):
        for trainer in self.catalog["trainers"]:
            self.assertEqual(set(trainer["voice"]), set(generate.VOICE_BITS), trainer["name"])
            self.assertIn("{PLAYER}", trainer["voice"]["heard"], trainer["name"])

    def test_voice_bits_are_complete_and_printable(self):
        with self.assertRaisesRegex(ValueError, "exactly"):
            catalog = copy.deepcopy(self.catalog)
            del catalog["trainers"][0]["voice"]["bye"]
            generate.validate(catalog)
        with self.assertRaisesRegex(ValueError, "must address"):
            generate.validate(self.with_voice(heard="Hello there."))
        with self.assertRaisesRegex(ValueError, "only"):
            generate.validate(self.with_voice(meet="I'm {ACE}!"))
        with self.assertRaisesRegex(ValueError, "charmap"):
            generate.validate(self.with_voice(meet="*Poof!*"))

    def test_voice_must_fit_one_message_box(self):
        with self.assertRaisesRegex(ValueError, "one message box"):
            generate.validate(self.with_voice(meet=" ".join(["word"] * 40)))

    def test_voice_breaks_keep_every_word(self):
        widths = generate.glyph_widths()
        for trainer in self.catalog["trainers"]:
            for bit, text in trainer["voice"].items():
                (box,) = generate.wrap_voice(text, widths)
                self.assertEqual(" ".join(box).split(), text.split(), f"{trainer['name']} {bit}")
                for line in box:
                    self.assertLessEqual(generate.text_pixels(line, widths), generate.MESSAGE_LINE_PIXELS)

    def test_generated_voices_use_the_buddy_variable_and_line_breaks(self):
        voices = generate.generate_voices(self.catalog)
        self.assertIn("{STR_VAR_1}", voices)
        self.assertNotIn("{BUDDY}", voices)
        self.assertIn("\\n", voices)
        self.assertEqual(voices.count("_(\""), 38 * len(generate.VOICE_BITS))

    def test_buddy_is_a_roster_slot(self):
        for value in (0, 7, True, "1", None):
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, "buddy"):
                catalog = copy.deepcopy(self.catalog)
                catalog["trainers"][0]["buddySlot"] = value
                generate.validate(catalog)


if __name__ == "__main__":
    unittest.main()
