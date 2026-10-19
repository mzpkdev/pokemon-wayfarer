import re
import tempfile
import unittest
from pathlib import Path

import audit as A


class Parsing(unittest.TestCase):
    def test_a_text_is_its_string_lines_joined(self):
        source = 'MatchCall_X_Battle::\n\t.string "Hi, {PLAYER}!\\n"\n\t.string "Come to {STR_VAR_2}.$"\n\nMatchCall_X_Other::\n\t.string "Bye$"\n'
        self.assertEqual(A.parse_texts(source), {'MatchCall_X_Battle': 'Hi, {PLAYER}!\\nCome to {STR_VAR_2}.$', 'MatchCall_X_Other': 'Bye$'})

    def test_boxes_split_on_paragraphs_and_lines_on_newlines(self):
        boxes = A.split_boxes('One\\nTwo\\pThree\\lFour\\nFive$')
        self.assertEqual(boxes, [['One', 'Two'], ['Three', 'Four', 'Five']])

    def test_placeholders_take_the_width_the_game_gives_them(self):
        self.assertEqual(len(A.expand('{PLAYER} {STR_VAR_2} {STR_VAR_3}')), 7 + 1 + 16 + 1 + 10)


class Format(unittest.TestCase):
    def test_one_dollar_at_the_end(self):
        self.assertEqual(A.format_problems('T', ['a\\n', 'b$']), [])
        self.assertEqual(len(A.format_problems('T', ['a\\n', 'b'])), 1)
        self.assertEqual(len(A.format_problems('T', ['a$\\n', 'b$'])), 1)
        self.assertEqual(len(A.format_problems('T', ['a$'] * 2)), 2)

    def test_every_line_but_the_last_ends_in_a_break(self):
        self.assertEqual(A.format_problems('T', ['a\\l', 'b\\p', 'c$']), [])
        self.assertEqual(len(A.format_problems('T', ['a ', 'b$'])), 1)


class Fit(unittest.TestCase):
    def test_a_line_of_exactly_the_width_fits(self):
        self.assertEqual(A.fit_problems('T', 'x' * A.LINE_WIDTH + '$'), [])

    def test_a_line_one_too_wide_fails(self):
        self.assertEqual(len(A.fit_problems('T', 'x' * (A.LINE_WIDTH + 1) + '$')), 1)

    def test_the_place_name_counts_at_its_full_width(self):
        # 30 - 16 = 14 characters of text around {STR_VAR_2} fill a line; 15 do not.
        self.assertEqual(A.fit_problems('T', '{STR_VAR_2}' + 'x' * 14 + '$'), [])
        self.assertEqual(len(A.fit_problems('T', '{STR_VAR_2}' + 'x' * 15 + '$')), 1)

    def test_the_player_name_counts_as_seven(self):
        self.assertEqual(A.fit_problems('T', '{PLAYER}' + 'x' * 23 + '$'), [])
        self.assertEqual(len(A.fit_problems('T', '{PLAYER}' + 'x' * 24 + '$')), 1)

    def test_too_many_lines_in_a_box_fail(self):
        self.assertEqual(A.fit_problems('T', 'a\\nb\\nc$'), [])
        self.assertEqual(len(A.fit_problems('T', 'a\\nb\\nc\\nd$')), 1)
        self.assertEqual(len(A.fit_problems('T', 'a\\nb\\nc$', box_lines=2)), 1)

    def test_an_unknown_placeholder_is_reported(self):
        self.assertEqual(len(A.fit_problems('T', '{MYSTERY}$')), 1)


class Repository(unittest.TestCase):
    def test_every_contact_has_its_texts_and_they_fit(self):
        self.assertEqual(A.audit(), [])

    def test_the_texts_are_the_specs_texts(self):
        # The spec keeps the authored call texts; the game files are transcribed from it verbatim.
        spec = (A.ROOT.parent / '.product/specs/phone-rematches.md').read_text()
        authored = A.parse_texts('\n'.join(re.findall(r'```asm\n(.*?)```', spec, re.S)))
        compiled = {}
        for name in ('hns', 'hoenn', 'sevii_coast'):
            compiled.update(A.parse_texts((A.ROOT / f'data/text/match_call_{name}.inc').read_text()))
        self.assertGreater(len(authored), 400)
        for label, text in authored.items():
            self.assertEqual(compiled.get(label), text, label)
        # And the other way: every phone text of the three new groups and every rewritten HNS text is in the spec.
        extra = [label for label in compiled if label not in authored
                 and (label.startswith(('MatchCall_HOENN_', 'MatchCall_SEVII_', 'MatchCall_COAST_'))
                      or label.endswith(('_FoundItem', '_Battle')) and label.startswith('MatchCall_HNS_'))]
        self.assertEqual(extra, ['MatchCall_HNS_Nicole_Battle'])  # kept for the standalone HNS build, which compiles her

    def test_a_contact_without_a_place_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for relative in ('data/text/match_call_hns.inc', 'data/text/match_call_hoenn.inc', 'data/text/match_call_sevii_coast.inc',
                             'src/data/wayfarer_phone_hoenn.h', 'src/data/wayfarer_phone_sevii.h', 'src/data/wayfarer_phone_coast.h',
                             'src/match_call.c', 'src/battle_setup.c', 'src/data/wayfarer_sevii_rematches.h', 'src/wayfarer_coast_trainers.c',
                             'src/data/trainer_slots/slots.h'):
                (root / relative).parent.mkdir(parents=True, exist_ok=True)
                (root / relative).write_text((A.ROOT / relative).read_text())
            slots = root / 'src/data/trainer_slots/slots.h'
            slots.write_text(slots.read_text().replace('{ TRAINER_TULLY_HNS, MAP_GROUP', '{ TRAINER_NOT_TULLY, MAP_GROUP'))
            self.assertIn('TRAINER_TULLY_HNS is a phone contact with neither a daily slot nor a fixed home', A.audit(root))

    def test_a_missing_text_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for relative in ('data/text/match_call_hns.inc', 'data/text/match_call_hoenn.inc', 'data/text/match_call_sevii_coast.inc',
                             'src/data/wayfarer_phone_hoenn.h', 'src/data/wayfarer_phone_sevii.h', 'src/data/wayfarer_phone_coast.h',
                             'src/match_call.c', 'src/battle_setup.c', 'src/data/trainer_slots/slots.h', 'src/data/wayfarer_sevii_rematches.h', 'src/wayfarer_coast_trainers.c'):
                (root / relative).parent.mkdir(parents=True, exist_ok=True)
                (root / relative).write_text((A.ROOT / relative).read_text())
            hoenn = root / 'data/text/match_call_hoenn.inc'
            hoenn.write_text(hoenn.read_text().replace('MatchCall_HOENN_Rose_Battle::', 'MatchCall_HOENN_Rose_Renamed::'))
            self.assertIn('missing text MatchCall_HOENN_Rose_Battle', A.audit(root))

    def test_a_battle_text_without_the_place_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for relative in ('data/text/match_call_hns.inc', 'data/text/match_call_hoenn.inc', 'data/text/match_call_sevii_coast.inc',
                             'src/data/wayfarer_phone_hoenn.h', 'src/data/wayfarer_phone_sevii.h', 'src/data/wayfarer_phone_coast.h',
                             'src/match_call.c', 'src/battle_setup.c', 'src/data/trainer_slots/slots.h', 'src/data/wayfarer_sevii_rematches.h', 'src/wayfarer_coast_trainers.c'):
                (root / relative).parent.mkdir(parents=True, exist_ok=True)
                (root / relative).write_text((A.ROOT / relative).read_text())
            hns = root / 'data/text/match_call_hns.inc'
            hns.write_text(hns.read_text().replace('{STR_VAR_2}', 'nowhere'))
            errors = A.audit(root)
            self.assertIn("MatchCall_HNS_Joey_Battle does not name today's place with {STR_VAR_2}", errors)
            self.assertIn("MatchCall_HNS_Wade_FoundItem does not name today's place with {STR_VAR_2}", errors)


if __name__ == '__main__':
    unittest.main()
