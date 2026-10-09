"""Reach of the maps that host ORDINARY Trainers: resolution rules, fail-closed cases and the checked-in outputs."""

import os
import unittest

import reach


def fake_map(const, name=None, map_type='MAP_TYPE_INDOOR', warps=(), value=0):
    info = reach.MapInfo()
    info.const, info.name = const, name or const.removeprefix('MAP_').title().replace('_', '')
    info.value, info.group, info.num = value, value >> 8, value & 0xFF
    info.map_type, info.warps = map_type, list(warps)
    info.events = {'objects': [], 'coords': [], 'bgs': [], 'warps': []}
    info.scripts_owner = info.name
    return info


class Fixture(unittest.TestCase):
    """A small world: a Road town, a Wilds route, a three-floor listed dungeon and what stands around them."""

    @classmethod
    def setUpClass(cls):
        _, cls.wild = reach.wild_places()

    def place(self, reach_name, intent='Mild', flat=False, floor=0, floors=0):
        return reach.place_record(self.wild, reach_name, intent, flat, floor, floors)

    def listed_entry(self, reach_name, name, intent=None, floor=0, floors=0, flat=False):
        place = self.place(reach_name, intent or 'Mild', flat, floor, floors)
        return {'place': place, 'name': name, 'reach': reach_name, 'intent_name': intent}

    def world(self, extra_maps=(), rules=None):
        maps = {m.const: m for m in [
            fake_map('MAP_TOWN', map_type='MAP_TYPE_TOWN', value=1),
            fake_map('MAP_ROUTE', map_type='MAP_TYPE_ROUTE', value=2),
            fake_map('MAP_CAVE_1F', map_type='MAP_TYPE_UNDERGROUND', value=3),
            fake_map('MAP_CAVE_2F', map_type='MAP_TYPE_UNDERGROUND', value=4),
            fake_map('MAP_CAVE_3F', map_type='MAP_TYPE_UNDERGROUND', value=5),
            *extra_maps]}
        listed = {
            'MAP_TOWN': self.listed_entry('Road', 'Town'),
            'MAP_ROUTE': self.listed_entry('Wilds', 'Route'),
            'MAP_CAVE_1F': self.listed_entry('Dungeon', 'Cave', 'Moderate', 0, 3),
            'MAP_CAVE_2F': self.listed_entry('Dungeon', 'Cave', 'Moderate', 1, 3),
            'MAP_CAVE_3F': self.listed_entry('Dungeon', 'Cave', 'Moderate', 2, 3),
        }
        data = {'version': 1, 'reach_maps': {}, 'dungeons': [], 'new_dungeons': []}
        data.update(rules or {})
        rule_object = reach.Rules.__new__(reach.Rules)
        rule_object.data = data
        rule_object.reach_maps = data['reach_maps']
        rule_object.dungeons = data['dungeons']
        rule_object.new_dungeons = data['new_dungeons']
        rule_object.interior_overrides = data.get('interior_overrides', {})
        return maps, listed, rule_object

    def resolver(self, extra_maps=(), rules=None):
        maps, listed, rule_object = self.world(extra_maps, rules)
        return reach.Resolver(maps, listed, self.wild, rule_object)


class ResolutionRuleTests(Fixture):
    def test_listed_maps_keep_the_wild_reach_and_floor(self):
        resolver = self.resolver()
        place, rule, _ = resolver.resolve('MAP_TOWN')
        self.assertEqual((rule, place['reach']), ('listed', 'WILD_REACH_ROAD'))
        place, rule, _ = resolver.resolve('MAP_CAVE_2F')
        self.assertEqual((rule, place['reach'], place['floor'], place['floorCount']), ('listed', 'WILD_REACH_DUNGEON', 1, 3))
        self.assertEqual(place['intent'], 'WILD_DUNGEON_MODERATE')

    def test_a_map_joins_a_reach_by_the_joining_table(self):
        resolver = self.resolver([fake_map('MAP_PLATEAU', map_type='MAP_TYPE_ROUTE', value=6)], {'reach_maps': {'MAP_PLATEAU': 'Outlands'}})
        place, rule, _ = resolver.resolve('MAP_PLATEAU')
        self.assertEqual((rule, place['reach']), ('join', 'WILD_REACH_OUTLANDS'))

    def test_extra_floors_join_their_dungeon_and_restep_it(self):
        basement = fake_map('MAP_CAVE_B1F', map_type='MAP_TYPE_UNDERGROUND', value=6)
        rules = {'dungeons': [{'name': 'Cave', 'intent': 'Hard', 'steps': [
            ['MAP_CAVE_B1F'], ['MAP_CAVE_1F'], ['MAP_CAVE_2F', 'MAP_CAVE_3F']]}]}
        resolver = self.resolver([basement], rules)
        new, rule, _ = resolver.resolve('MAP_CAVE_B1F')
        self.assertEqual((rule, new['floor'], new['floorCount'], new['intent']), ('join', 0, 3, 'WILD_DUNGEON_HARD'))
        listed, rule, _ = resolver.resolve('MAP_CAVE_1F')
        self.assertEqual((rule, listed['floor'], listed['floorCount']), ('join (listed map re-stepped)', 1, 3))
        shared = [resolver.resolve(c)[0]['floor'] for c in ('MAP_CAVE_2F', 'MAP_CAVE_3F')]
        self.assertEqual(shared, [2, 2])  # maps named at one step share a floor

    def test_a_join_keeps_the_dungeons_intent_unless_the_table_changes_it(self):
        basement = fake_map('MAP_CAVE_B1F', map_type='MAP_TYPE_UNDERGROUND', value=6)
        rules = {'dungeons': [{'name': 'Cave', 'steps': [['MAP_CAVE_B1F'], ['MAP_CAVE_1F'], ['MAP_CAVE_2F'], ['MAP_CAVE_3F']]}]}
        self.assertEqual(self.resolver([basement], rules).resolve('MAP_CAVE_B1F')[0]['intent'], 'WILD_DUNGEON_MODERATE')

    def test_a_join_must_follow_the_wild_floor_order(self):
        rules = {'dungeons': [{'name': 'Cave', 'steps': [['MAP_CAVE_3F'], ['MAP_CAVE_2F'], ['MAP_CAVE_1F']]}]}
        with self.assertRaisesRegex(reach.ReachError, 'floor order'):
            self.resolver(rules=rules)

    def test_a_join_of_a_non_dungeon_fails(self):
        with self.assertRaisesRegex(reach.ReachError, 'not a dungeon'):
            self.resolver(rules={'dungeons': [{'name': 'Town', 'steps': [['MAP_TOWN']]}]})

    def test_story_sites_become_dungeons_with_one_step_per_floor(self):
        tower = [fake_map(f'MAP_TOWER_{i}F', value=10 + i) for i in range(1, 4)]
        rules = {'new_dungeons': [{'name': 'Tower', 'intent': 'Moderate to hard', 'steps': [[m.const] for m in tower]}]}
        resolver = self.resolver(tower, rules)
        for index, info in enumerate(tower):
            place, rule, _ = resolver.resolve(info.const)
            self.assertEqual((rule, place['floor'], place['floorCount'], place['flat']), ('new-dungeon', index, 3, 0))
            self.assertEqual(place['intent'], 'WILD_DUNGEON_MODERATE_TO_HARD')

    def test_a_single_floor_story_site_is_flat(self):
        warehouse = fake_map('MAP_WAREHOUSE', value=10)
        rules = {'new_dungeons': [{'name': 'Warehouse', 'intent': 'Moderate', 'steps': [['MAP_WAREHOUSE']]}]}
        place, _, _ = self.resolver([warehouse], rules).resolve('MAP_WAREHOUSE')
        self.assertEqual((place['flat'], place['floor'], place['floorCount']), (1, 0, 1))

    def test_a_story_site_may_not_reuse_a_wild_listed_map(self):
        rules = {'new_dungeons': [{'name': 'Again', 'intent': 'Mild', 'steps': [['MAP_TOWN']]}]}
        with self.assertRaisesRegex(reach.ReachError, 'already lists'):
            self.resolver(rules=rules)

    def test_ferries_are_road(self):
        ferry = fake_map('MAP_SSTIDAL_ROOMS', name='SSTidalRooms', value=10)
        place, rule, _ = self.resolver([ferry]).resolve('MAP_SSTIDAL_ROOMS')
        self.assertEqual((rule, place['reach']), ('ferry', 'WILD_REACH_ROAD'))

    def test_interiors_take_the_map_they_open_onto(self):
        house = fake_map('MAP_HOUSE', warps=['MAP_ROUTE'], value=10)
        upstairs = fake_map('MAP_HOUSE_2F', warps=['MAP_HOUSE'], value=11)
        resolver = self.resolver([house, upstairs])
        for const in ('MAP_HOUSE', 'MAP_HOUSE_2F'):
            place, rule, _ = resolver.resolve(const)
            self.assertEqual((rule, place['reach']), ('interior', 'WILD_REACH_WILDS'))

    def test_an_interior_opening_onto_a_town_and_a_dungeon_takes_the_outdoor_map(self):
        lobby = fake_map('MAP_LOBBY', warps=['MAP_TOWN', 'MAP_CAVE_1F'], value=10)
        place, _, _ = self.resolver([lobby]).resolve('MAP_LOBBY')
        self.assertEqual(place['reach'], 'WILD_REACH_ROAD')

    def test_an_interior_that_opens_onto_two_different_reaches_needs_a_reviewed_override(self):
        gate = fake_map('MAP_GATE', warps=['MAP_TOWN', 'MAP_ROUTE'], value=10)
        with self.assertRaisesRegex(reach.ReachError, 'different reaches'):
            self.resolver([gate]).resolve('MAP_GATE')
        resolver = self.resolver([gate], {'interior_overrides': {'MAP_GATE': 'MAP_ROUTE'}})
        place, _, detail = resolver.resolve('MAP_GATE')
        self.assertEqual(place['reach'], 'WILD_REACH_WILDS')
        self.assertIn('reviewed override', detail)

    def test_gyms_have_no_reach_and_are_not_walked_through(self):
        gym = fake_map('MAP_TOWN_GYM', name='Town_Gym', warps=['MAP_TOWN'], value=10)
        backroom = fake_map('MAP_BACKROOM', warps=['MAP_TOWN_GYM'], value=11)
        resolver = self.resolver([gym, backroom])
        self.assertIsNone(resolver.resolve('MAP_TOWN_GYM'))
        self.assertIsNone(resolver.resolve('MAP_BACKROOM'))  # its only way out is the Gym

    def test_a_map_nothing_resolves_gets_no_entry(self):
        island = fake_map('MAP_ISLAND', map_type='MAP_TYPE_ROUTE', value=10)
        self.assertIsNone(self.resolver([island]).resolve('MAP_ISLAND'))

    def test_rule_data_naming_a_missing_map_fails(self):
        with self.assertRaisesRegex(reach.ReachError, 'MAP_NOT_THERE'):
            self.resolver(rules={'reach_maps': {'MAP_NOT_THERE': 'Wilds'}})
        with self.assertRaisesRegex(reach.ReachError, 'MAP_ALSO_NOT_THERE'):
            self.resolver(rules={'new_dungeons': [{'name': 'X', 'intent': 'Mild', 'steps': [['MAP_ALSO_NOT_THERE']]}]})

    def test_a_map_cannot_belong_to_two_dungeons(self):
        a, b = fake_map('MAP_A', value=10), fake_map('MAP_B', value=11)
        rules = {'new_dungeons': [{'name': 'One', 'intent': 'Mild', 'steps': [['MAP_A', 'MAP_B']]},
                                  {'name': 'Two', 'intent': 'Mild', 'steps': [['MAP_B']]}]}
        with self.assertRaisesRegex(reach.ReachError, 'two dungeons'):
            self.resolver([a, b], rules)

    def test_an_unknown_reach_name_fails(self):
        with self.assertRaisesRegex(reach.ReachError, 'unknown reach'):
            self.resolver([fake_map('MAP_X', value=10)], {'reach_maps': {'MAP_X': 'Moon'}})


class HostedMapTests(Fixture):
    def resolve(self, extra_maps, host, ordinary, rules=None):
        maps, listed, rule_object = self.world(extra_maps, rules)
        return reach.resolve_hosted(maps, set(ordinary), host, listed, self.wild, rule_object)

    def test_every_hosting_map_resolves_and_unhosted_trainers_are_reported(self):
        state = self.resolve([], {'TRAINER_A': {'MAP_TOWN'}, 'TRAINER_B': {'MAP_CAVE_2F', 'MAP_ROUTE'}}, ['TRAINER_A', 'TRAINER_B', 'TRAINER_C'])
        self.assertEqual(sorted(state['resolved']), ['MAP_CAVE_2F', 'MAP_ROUTE', 'MAP_TOWN'])
        self.assertEqual(state['unplaced'], ['TRAINER_C'])
        self.assertEqual(sorted(state['hosted']['MAP_ROUTE']), ['TRAINER_B'])

    def test_a_covered_trainer_on_a_map_with_no_entry_fails_generation(self):
        island = fake_map('MAP_ISLAND', map_type='MAP_TYPE_ROUTE', value=10)
        with self.assertRaisesRegex(reach.ReachError, r'MAP_ISLAND .* no rule resolves'):
            self.resolve([island], {'TRAINER_A': {'MAP_ISLAND'}}, ['TRAINER_A'])

    def test_an_ordinary_trainer_in_a_gym_fails_generation(self):
        gym = fake_map('MAP_TOWN_GYM', name='Town_Gym', warps=['MAP_TOWN'], value=10)
        with self.assertRaisesRegex(reach.ReachError, 'Gym or facility'):
            self.resolve([gym], {'TRAINER_A': {'MAP_TOWN_GYM'}}, ['TRAINER_A'])

    def test_every_failure_is_reported_together(self):
        a, b = fake_map('MAP_A', map_type='MAP_TYPE_ROUTE', value=10), fake_map('MAP_B', map_type='MAP_TYPE_ROUTE', value=11)
        with self.assertRaises(reach.ReachError) as caught:
            self.resolve([a, b], {'T1': {'MAP_A'}, 'T2': {'MAP_B'}}, ['T1', 'T2'])
        self.assertIn('MAP_A', str(caught.exception))
        self.assertIn('MAP_B', str(caught.exception))

    def test_the_table_lists_exactly_the_hosting_maps_in_map_order(self):
        state = self.resolve([], {'T1': {'MAP_ROUTE'}, 'T2': {'MAP_TOWN'}}, ['T1', 'T2'])
        table = reach.render_table(state)
        self.assertEqual(table.count('MAP_GROUP('), 2)
        self.assertLess(table.index('MAP_TOWN'), table.index('MAP_ROUTE'))  # map value order
        self.assertNotIn('MAP_CAVE', table)
        self.assertIn('.reach = WILD_REACH_ROAD', table)


class ScriptReadingTests(unittest.TestCase):
    def test_macro_arguments_split_on_commas_and_spaces_outside_parentheses(self):
        self.assertEqual(reach.macro_args('((201) + 638), Intro, Defeat'), ['((201) + 638)', 'Intro', 'Defeat'])
        self.assertEqual(reach.macro_args('476 Seen, Beaten'), ['476', 'Seen', 'Beaten'])

    def test_battled_trainers_come_from_the_first_argument_except_two_trainer_and_raw_forms(self):
        body = '''
 trainerbattle_single 85, Intro, Defeat
 trainerbattle_double (1 + 2) Intro, Defeat, NotEnough
 trainerbattle_two_trainers 11, LoseA, 12, LoseB
 trainerbattle 2, 2, 20, Intro, Lose, Script, OBJ_ID_NONE, 21, NULL
 goto_if_defeated 99, Somewhere
'''
        self.assertEqual(reach.body_trainers(body), {85, 3, 11, 12, 20, 21})

    def test_a_trainer_argument_that_is_not_a_number_fails(self):
        with self.assertRaisesRegex(reach.ReachError, 'not a number'):
            reach.body_trainers(' trainerbattle_single Oops, Intro, Defeat\n')

    def test_numeric_conditionals_drop_dead_branches_and_unknown_ones_keep_both(self):
        text = '\n'.join(['.if 0', 'dead', '.else', 'live', '.endif',
                          '.if 1', 'kept', '.endif',
                          '.if SOME_SYMBOL', 'maybe', '.else', 'also', '.endif',
                          '.if FALSE == FALSE', 'equal', '.endif'])
        kept = reach.apply_conditionals(text).split()
        self.assertEqual(kept, ['live', 'kept', 'maybe', 'also', 'equal'])

    def test_elseif_follows_the_first_true_branch(self):
        text = '\n'.join(['.if 0 == 1', 'a', '.elseif 1 == 1', 'b', '.else', 'c', '.endif'])
        self.assertEqual(reach.apply_conditionals(text).split(), ['b'])

    def test_a_script_runs_on_into_the_next_label_unless_it_ends(self):
        self.assertTrue(reach.ends_script('\n msgbox Text\n end\n'))
        self.assertTrue(reach.ends_script('\n goto Elsewhere\n'))
        self.assertFalse(reach.ends_script('\n goto_if_eq 0x800D, FALSE, Elsewhere\n'))
        self.assertTrue(reach.ends_script('\n .string "text"\n'))

    def test_hosting_follows_events_gotos_and_fallthrough_but_not_dead_labels(self):
        labels = {
            'Map_MapScripts': '\n .byte 0\n',
            'Map_Guard': '\n goto_if_defeated 5, Map_Done\n specialvar 0x800D, CanBattle\n',
            'Map_Guard_Battle': '\n trainerbattle_single 5, Intro, Defeat\n end\n',
            'Map_Sign': '\n goto Map_Shared\n',
            'Map_Shared': '\n trainerbattle_single 6, Intro, Defeat\n end\n',
            'Map_Dead': '\n trainerbattle_single 7, Intro, Defeat\n end\n',
        }
        reach.FALLTHROUGH.clear()
        reach.FALLTHROUGH['Map_Guard'] = 'Map_Guard_Battle'
        info = fake_map('MAP_X', name='Map')
        info.scripts_owner = 'Map'
        info.events['objects'] = [{'script': 'Map_Guard'}, {'script': 'NULL'}]
        info.events['bgs'] = [{'script': 'Map_Sign'}]
        host = reach.hosting({'MAP_X': info}, labels)
        self.assertEqual(dict(host), {5: {'MAP_X'}, 6: {'MAP_X'}})
        reach.FALLTHROUGH.clear()


class GeneratedOutputTests(unittest.TestCase):
    """The checked-in table and report against this build's maps (the Wayfarer mapjson outputs)."""

    @classmethod
    def setUpClass(cls):
        require = os.environ.get('TRAINER_REACH_REQUIRE_MAPS')
        present = (reach.ROOT / 'data/maps/groups.inc').is_file() and (reach.ROOT / 'include/constants/map_groups.h').is_file()
        if not present:
            if require:
                raise AssertionError('the Wayfarer mapjson outputs are missing; run `make BUILD=wayfarer generated`')
            raise unittest.SkipTest('needs the Wayfarer mapjson outputs (make BUILD=wayfarer generated)')
        cls.state = reach.resolve_all()

    def test_checked_in_table_and_report_are_current(self):
        self.assertEqual(reach.TABLE.read_text(), reach.render_table(self.state))
        self.assertEqual(reach.REPORT.read_text(), reach.render_report(self.state))

    def test_every_map_hosting_an_ordinary_trainer_has_a_table_row(self):
        table = reach.TABLE.read_text()
        for const in self.state['hosted']:
            self.assertIn(f'MAP_GROUP({const}), MAP_NUM({const})', table)
        self.assertEqual(table.count('MAP_GROUP('), len(self.state['hosted']))

    def test_no_table_row_is_a_gym_or_facility(self):
        for const in self.state['resolved']:
            name = self.state['maps'][const].name
            self.assertIsNone(reach.GYM_RE.search(name), name)
            self.assertIsNone(reach.FACILITY_RE.match(name), name)

    def test_the_rule_data_resolves_each_place_the_spec_names(self):
        resolved = self.state['resolved']
        expectations = {
            'MAP_ROUTE30_HNS': 'listed',
            'MAP_SPROUT_TOWER_1F_HNS': 'join',
            'MAP_SPROUT_TOWER_3F_HNS': 'join (listed map re-stepped)',
            'MAP_ROUTE26NORTH_HNS': 'join',
            'MAP_SILPH_CO_11F': 'new-dungeon',
            'MAP_SSANNE_DECK': 'new-dungeon',
            'MAP_SSAQUA_B1F_HNS': 'ferry',
            'MAP_ROUTE110_TRICK_HOUSE_PUZZLE1': 'interior',
        }
        for const, rule in expectations.items():
            self.assertEqual(resolved[const][1], rule, const)

    def test_ordinary_trainers_with_no_hosting_map_are_only_the_unreferenced_source_families(self):
        unplaced = set(self.state['unplaced'])
        # Placed by scripts or by their rematch family: none of these may be reported.
        for trainer in ('TRAINER_JOEY_2_HNS', 'TRAINER_SILPH_GRUNT_23_HNS', 'TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS',
                        'TRAINER_MT_MOON_ROCKET_GRUNT_1_HNS', 'TRAINER_WAYFARER_COAST_SWIMMER_MALE_JACK_2',
                        'TRAINER_WAYFARER_SS_ANNE_SAILOR_EDMOND'):
            self.assertNotIn(trainer, unplaced)
        for trainer in self.state['unplaced']:
            self.assertIn(trainer, self.state['ordinary'])


if __name__ == '__main__':
    unittest.main()
