#!/usr/bin/env python3
"""Regression checks for frame-bound and animation-contract validation."""
import unittest
from unittest.mock import patch

import check_manifest


class ManifestTests(unittest.TestCase):
    def test_all_registered_styles_have_all_states(self):
        rows = check_manifest.inventory()['states']
        self.assertEqual(len(rows), 36)
        for appearance in {r['appearance'] for r in rows}:
            self.assertEqual(len([r for r in rows if r['appearance'] == appearance]), 9)

    def test_rejects_animation_frame_outside_pic_table(self):
        original = check_manifest.arrays

        def arrays(path):
            result = original(path)
            if path.name == 'object_event_anims.h':
                result['sAnim_GoSouth'] = 'ANIMCMD_FRAME(999, 4), ANIMCMD_END,'
            return result

        with patch.object(check_manifest, 'arrays', side_effect=arrays):
            rows = check_manifest.inventory()['states']
        bike = next(r for r in rows if r['appearance'] == 'APPEARANCE_GOLD' and r['state'] == 'MACH_BIKE')
        self.assertTrue(any('references 999 outside' in e for e in bike['errors']))

    def test_rejects_pic_frame_outside_source_png(self):
        original = check_manifest.png_size

        def size(path):
            return (32, 32) if path.name == 'surfing_hns.png' else original(path)

        with patch.object(check_manifest, 'png_size', side_effect=size):
            rows = check_manifest.inventory()['states']
        gold = next(r for r in rows if r['appearance'] == 'APPEARANCE_GOLD' and r['state'] == 'SURFING')
        self.assertTrue(any('outside 1-frame sheet' in e for e in gold['errors']))

    def test_rejects_missing_acro_trick_animation_slot(self):
        original = check_manifest.arrays

        def arrays(path):
            result = original(path)
            if path.name == 'object_event_anims.h':
                result['sAnimTable_AcroBike'] = result['sAnimTable_AcroBike'].replace(
                    '[ANIM_MOVING_WHEELIE_EAST] = sAnim_MovingWheelieEast,', '')
            return result

        with patch.object(check_manifest, 'arrays', side_effect=arrays):
            rows = check_manifest.inventory()['states']
        bike = next(r for r in rows if r['appearance'] == 'APPEARANCE_GOLD' and r['state'] == 'ACRO_BIKE')
        self.assertIn('Missing required animation slots: ANIM_MOVING_WHEELIE_EAST', bike['errors'])

    def test_reflections_have_registered_character_palette(self):
        rows = check_manifest.inventory()['states']
        gold = next(r for r in rows if r['appearance'] == 'APPEARANCE_GOLD' and r['state'] == 'NORMAL')
        self.assertEqual(gold['palette_symbol'], 'gObjectEventPal_Gold_hns')
        self.assertEqual(gold['reflection_palette_symbols'], ['gObjectEventPal_GoldReflection_hns'] * 4)

    def test_registered_styles_exclude_deferred_characters(self):
        rows = check_manifest.inventory()['states']
        self.assertEqual({r['appearance'] for r in rows}, {
            'APPEARANCE_GOLD', 'APPEARANCE_KRIS', 'APPEARANCE_BRENDAN', 'APPEARANCE_MAY'})
        for row in rows:
            if row['state'] == 'NORMAL':
                self.assertEqual(row['animations']['ANIM_RUN_NORTH']['symbol'], 'sAnim_RunNorth')


if __name__ == '__main__':
    unittest.main()
