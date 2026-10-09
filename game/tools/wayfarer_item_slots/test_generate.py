import copy
import json
import os
import unittest

import generate as G
import spec_tables as S

C = G.C
HAVE_MAPS = (G.ROOT / 'data/maps/groups.inc').is_file() and (G.ROOT / 'include/constants/map_groups.h').is_file()
REQUIRE_MAPS = os.environ.get('WORLD_ITEMS_REQUIRE_MAPS') == '1'
POCKETS = {'ITEM_POTION': 'POCKET_ITEMS', 'ITEM_LEFTOVERS': 'POCKET_ITEMS', 'ITEM_MASTER_BALL': 'POCKET_POKE_BALLS',
           'ITEM_TM_TOXIC': 'POCKET_TM_HM', 'ITEM_TM_REST': 'POCKET_TM_HM', 'ITEM_TM_EARTHQUAKE': 'POCKET_TM_HM',
           'ITEM_OLD_SEA_MAP': 'POCKET_KEY_ITEMS', 'ITEM_ESCAPE_ROPE': 'POCKET_KEY_ITEMS', 'ITEM_HM_CUT': 'POCKET_TM_HM',
           'ITEM_CHOICE_BAND': 'POCKET_ITEMS'}


def spot(kind='ball', ident=1, x=1, y=1, map_name='MapA', region='Kanto', tier='Wilds', flag='FLAG_A', authored='ITEM_POTION', status='item'):
    return C.Spot(map_const='MAP_A', map_name=map_name, map_value=0x100, kind=kind, id=ident, x=x, y=y, authored=authored, flag=flag,
                  script='', region=region, tier=tier, status=status, gfx='')


def prize(item='ITEM_LEFTOVERS', tier='Find', region='Kanto', **kw):
    row = {'region': region, 'trail': 'T', 'n': 1, 'map': 'MapA', 'shown': 'Visible', 'item': item, 'tier': tier, 'spec_reach': 'Wilds',
           'kind': 'ball', 'local_id': 1, 'x': 1, 'y': 1}
    row.update(kw)
    return row


def run_prizes(prizes, spots, mart=('ITEM_TM_REST',)):
    data = {'prizes': prizes, 'mart_tms': list(mart), 'spec_totals': {}, 'spec_overall': len(prizes)}
    errors = []
    G.NOTES.clear()
    G.apply_prizes(spots, data, POCKETS, {}, errors)
    return errors


class PrizeValidation(unittest.TestCase):
    def test_valid_prize_marks_the_spot(self):
        spots = [spot()]
        self.assertEqual(run_prizes([prize()], spots), [])
        self.assertEqual((spots[0].status, spots[0].prize), ('prize', 'ITEM_LEFTOVERS'))

    def test_unknown_map_and_missing_spot_fail(self):
        self.assertTrue(any('not compiled' in e for e in run_prizes([prize(map='Nowhere')], [spot()])))
        self.assertTrue(any('has no item ball' in e for e in run_prizes([prize(local_id=9)], [spot()])))
        self.assertTrue(any('sits at' in e or 'no item ball' in e for e in run_prizes([prize(x=5)], [spot()])))

    def test_unknown_item_fails(self):
        self.assertTrue(any('not an item' in e for e in run_prizes([prize(item='ITEM_GEN_NINE')], [spot()])))

    def test_prize_spot_without_a_flag_fails(self):
        for flag in ('0', 'FLAG_NONE', ''):
            self.assertTrue(any('no flag' in e for e in run_prizes([prize()], [spot(flag=flag)])), flag)

    def test_visible_ball_on_road_fails_but_hidden_is_fine(self):
        self.assertTrue(any('on Road' in e for e in run_prizes([prize()], [spot(tier='Road')])))
        hidden = prize(shown='Hidden', kind='hidden')
        del hidden['local_id']
        self.assertEqual(run_prizes([hidden], [spot(kind='hidden', tier='Road')]), [])

    def test_shown_must_match_the_spot_kind(self):
        self.assertTrue(any('spec shows it as Hidden' in e for e in run_prizes([prize(shown='Hidden')], [spot()])))

    def test_mart_sold_tm_fails(self):
        self.assertTrue(any('mart sells' in e for e in run_prizes([prize(item='ITEM_TM_REST')], [spot()])))

    def test_legend_must_be_unique_game_wide(self):
        def fresh():
            return [spot(), spot(ident=2, x=2, map_name='MapA'), spot(ident=3, x=3, region='Johto')]
        rows = [prize(item='ITEM_MASTER_BALL', tier='Legend', local_id=1, x=1),
                prize(item='ITEM_MASTER_BALL', tier='Legend', local_id=3, x=3, region='Johto')]
        self.assertTrue(any('more than once' in e for e in run_prizes(rows, fresh())))
        rows = [prize(item='ITEM_MASTER_BALL', tier='Legend', local_id=1, x=1), prize(item='ITEM_MASTER_BALL', tier='Treasure', local_id=3, x=3, region='Johto')]
        self.assertTrue(any('unique game-wide' in e for e in run_prizes(rows, fresh())))

    def test_finds_and_treasures_may_repeat_across_regions_but_not_within_one(self):
        def fresh():
            return [spot(), spot(ident=2, x=2), spot(ident=3, x=3, region='Johto')]
        across = [prize(local_id=1, x=1), prize(local_id=3, x=3, region='Johto')]
        self.assertEqual(run_prizes(across, fresh()), [])
        within = [prize(local_id=1, x=1), prize(local_id=2, x=2)]
        self.assertTrue(any('twice in Kanto' in e for e in run_prizes(within, fresh())))

    def test_fixed_story_spot_cannot_be_a_prize_or_a_source(self):
        fixed = spot(status='fixed')
        fixed.reason = 'Old Sea Map'
        self.assertTrue(any('fixed story spot' in e for e in run_prizes([prize()], [fixed])))
        other = spot(ident=2, x=2)
        moved = prize(local_id=2, x=2, moved_from={'map': 'MapA', 'spot': {'kind': 'ball', 'local_id': 1, 'x': 1, 'y': 1}})
        self.assertTrue(any('moved from a fixed' in e for e in run_prizes([moved], [fixed, other])))

    def test_two_prizes_on_one_spot_fail(self):
        rows = [prize(), prize(item='ITEM_CHOICE_BAND')]
        self.assertTrue(any('also the prize' in e for e in run_prizes(rows, [spot()])))

    def test_region_of_prize_must_match_the_spot(self):
        self.assertTrue(any('not Johto' in e for e in run_prizes([prize(region='Johto')], [spot()])))

    def test_totals_must_match_the_spec(self):
        data = {'prizes': [prize()], 'mart_tms': [], 'spec_totals': {'Kanto': {'total': 2}}, 'spec_overall': 1}
        errors = []
        G.apply_prizes([spot()], data, POCKETS, {}, errors)
        self.assertTrue(any('spec totals say 2' in e for e in errors))


class Classification(unittest.TestCase):
    fixed = {'map_prefixes': [{'prefix': 'BattlePyramid', 'reason': 'out of scope'}],
             'spots': [{'map': 'MapA', 'ids': [7], 'hidden': [[4, 4]], 'reason': 'story'}]}

    def run_classify(self, spots, fixed=None):
        errors = []
        G.classify(spots, fixed or self.fixed, POCKETS, errors)
        return errors

    def test_unrecognised_ball_must_be_reviewed(self):
        ball = spot(ident=3, authored=None, status='unrecognised')
        self.assertTrue(any('fixed.json' in e for e in self.run_classify([ball, spot(ident=7)])))

    def test_listed_balls_and_hidden_are_fixed(self):
        spots = [spot(ident=7), spot(kind='hidden', ident=2, x=4, y=4)]
        self.assertEqual(self.run_classify(spots), [])
        self.assertEqual([s.status for s in spots], ['fixed', 'fixed'])

    def test_key_items_and_hms_are_fixed_but_escape_rope_is_not(self):
        spots = [spot(ident=7), spot(ident=8, authored='ITEM_OLD_SEA_MAP'), spot(ident=9, authored='ITEM_HM_CUT'), spot(ident=10, authored='ITEM_ESCAPE_ROPE')]
        self.run_classify(spots)
        self.assertEqual([s.status for s in spots], ['fixed', 'fixed', 'fixed', 'item'])

    def test_fixed_entry_that_matches_nothing_fails(self):
        self.assertTrue(any('not an item spot' in e for e in self.run_classify([spot(ident=1)])))

    def test_unknown_authored_item_fails(self):
        self.assertTrue(any('not an item of the Wayfarer build' in e for e in self.run_classify([spot(ident=7), spot(ident=1, authored='ITEM_GEN_NINE')])))

    def test_pyramid_prefix_is_excluded(self):
        pyramid = spot(ident=1, map_name='BattlePyramidSquare01', authored=None, status='unrecognised')
        self.assertEqual(self.run_classify([pyramid], {'map_prefixes': self.fixed['map_prefixes'], 'spots': []}), [])
        self.assertEqual(pyramid.status, 'fixed')


class Pools(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads(G.DATA.read_text())
        cls.pockets = G.item_symbols()

    def test_every_pool_item_exists_and_odds_sum(self):
        pools, stones, tms = G.pool_tables(self.data, self.pockets, set())
        self.assertEqual(len(stones), 10)
        for region, tiers in pools.items():
            for tier, rows in tiers.items():
                self.assertTrue(rows and all(weight > 0 for _, weight in rows), (region, tier))

    def test_legend_tms_leave_the_dynamic_list(self):
        _, _, everything = G.pool_tables(self.data, self.pockets, set())
        _, _, without = G.pool_tables(self.data, self.pockets, {'ITEM_TM_EARTHQUAKE'})
        self.assertIn('ITEM_TM_EARTHQUAKE', everything)
        self.assertNotIn('ITEM_TM_EARTHQUAKE', without)
        self.assertEqual(len(without), len(everything) - 1)

    def test_regional_flavour_moves_the_listed_items(self):
        pools, _, _ = G.pool_tables(self.data, self.pockets, set())
        def weight(region, tier, item):
            return dict(pools[region][tier]).get(item)
        self.assertEqual(weight('Kanto', 'Regular', 'ITEM_MOOMOO_MILK'), 4)
        self.assertIsNone(weight('Kanto', 'Better', 'ITEM_MOOMOO_MILK'))
        self.assertEqual(weight('Hoenn', 'Better', 'ITEM_HEART_SCALE'), 3)
        self.assertIsNone(weight('Hoenn', 'Special', 'ITEM_HEART_SCALE'))
        self.assertEqual(weight('Hoenn', 'Better', 'ITEM_RED_SHARD'), 2)
        self.assertEqual(weight('Sevii', 'Better', 'ITEM_PEARL'), 6)
        self.assertEqual(weight('Sevii', 'Better', 'ITEM_BIG_PEARL'), 2)
        self.assertEqual(weight('Johto', 'Regular', 'ITEM_RED_APRICORN'), 2)
        self.assertIsNone(weight('Kanto', 'Regular', 'ITEM_RED_APRICORN'))

    def test_unknown_pool_item_fails(self):
        data = copy.deepcopy(self.data)
        data['pools']['Regular'][0]['item'] = 'ITEM_GEN_NINE'
        with self.assertRaises(G.GenerateError):
            G.pool_tables(data, self.pockets, set())

    def test_odds_must_sum_to_one_hundred(self):
        data = copy.deepcopy(self.data)
        data['tier_odds']['Road'] = [70, 25, 6]
        with self.assertRaises(G.GenerateError):
            G.pool_tables(data, self.pockets, set())


class SpecData(unittest.TestCase):
    def test_data_json_matches_the_spec(self):
        self.assertEqual(G.DATA.read_text(), json.dumps(S.parse_all(S.SPEC.read_text(encoding='utf-8')), indent=1) + '\n')

    def test_prize_counts_match_the_spec_totals(self):
        data = json.loads(G.DATA.read_text())
        self.assertEqual(len(data['prizes']), 210)
        for region, expected in data['spec_totals'].items():
            rows = [p for p in data['prizes'] if p['region'] == region]
            self.assertEqual(len(rows), expected['total'])
            for tier in ('Find', 'Treasure', 'Legend'):
                self.assertEqual(sum(1 for p in rows if p['tier'] == tier), expected[tier], (region, tier))

    def test_mart_list_has_every_spec_entry(self):
        self.assertEqual(len(json.loads(G.DATA.read_text())['mart_tms']), 28)


@unittest.skipUnless(HAVE_MAPS or REQUIRE_MAPS, 'needs the Wayfarer mapjson outputs (make BUILD=wayfarer generated)')
class GeneratedOutput(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.state = G.build()

    def test_committed_output_is_current(self):
        for name, content in G.outputs(self.state).items():
            self.assertEqual((G.COUNT_HEADER if name == '@count' else G.OUTPUT / name).read_text(), content, name)

    def test_totals(self):
        rows = self.state['rows']
        self.assertEqual(sum(1 for s in rows if s.status == 'prize'), 210)
        counts = {}
        for s in rows:
            if s.status == 'item':
                counts[s.region] = counts.get(s.region, 0) + 1
        self.assertEqual((counts['Kanto'], counts['Sevii']), (128, 64))

    def test_slot_indices_are_dense_and_sorted(self):
        rows = self.state['rows']
        self.assertEqual([s.index for s in rows], list(range(len(rows))))
        self.assertEqual([s.key for s in rows], sorted(s.key for s in rows))
        self.assertEqual(len({s.key for s in rows}), len(rows))

    def test_prize_balls_keep_a_flag_and_none_is_visible_on_road(self):
        for s in self.state['rows']:
            if s.status == 'prize':
                self.assertFalse(C.NO_FLAG & {s.flag}, s.label)
                self.assertFalse(s.kind == 'ball' and s.tier == 'Road', s.label)

    def test_gym_spots_count_as_road(self):
        gyms = [s for s in self.state['rows'] if C.R.GYM_RE.search(s.map_name)]
        self.assertTrue(all(s.tier == 'Road' for s in gyms))

    def test_dungeon_entrance_is_wilds_and_deeper_floors_outlands(self):
        by_label = {s.label: s for s in self.state['rows']}
        self.assertEqual(by_label['SeafoamIslands_B1F_Frlg local id 3 (19,18)'].tier, 'Outlands')
        self.assertEqual(by_label['PokemonMansion_1F_Frlg hidden 2,21'].tier, 'Wilds')


if __name__ == '__main__':
    unittest.main()
