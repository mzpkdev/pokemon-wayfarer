import copy
import json
import os
import unittest

import generate as G  # first: the shared reach modules put tools/trainer_scaling on the path, which has its own generate.py

A = G.A
C = G.C
R = G.R
HAVE_MAPS = (G.ROOT / 'data/maps/groups.inc').is_file() and (G.ROOT / 'include/constants/map_groups.h').is_file()
REQUIRE_MAPS = os.environ.get('WORLD_TRAINERS_REQUIRE_MAPS') == '1'
REVIEWED = json.loads(G.REVIEWED.read_text())


def obj(local_id=1, home='TRAINER_A', gfx='OBJ_EVENT_GFX_BOY_1', problems=(), pair=False, trainer_type='TRAINER_TYPE_NORMAL', hide='0',
        script='Script_A', texts=(), gift=False):
    return C.Slot(map_const='MAP_A', map_name='MapA', map_value=0x0100, local_id=local_id, x=3, y=4, gfx=gfx, script=script,
                  trainer_type=trainer_type, hide_flag=hide, sight='3', movement='MOVEMENT_TYPE_NONE', home=home, home_id=1, pair=pair,
                  klass='TRAINER_CLASS_HIKER_HNS', policy='ORDINARY', texts=list(texts), gift=gift, map_type='MAP_TYPE_ROUTE',
                  commands={'problems': list(problems)})


def units_of(*objects):
    errors = []
    units, loose = G.form_units(list(objects), errors)
    return units, errors


def classified(*objects, reviewed=None):
    if reviewed is None:
        reviewed = {**copy.deepcopy(REVIEWED), 'dialogue': [], 'not_daily_slots': [], 'not_daily_maps': [], 'not_daily_classes': []}
    units, errors = units_of(*objects)
    G.classify(units, reviewed, errors)
    return units, errors


class Units(unittest.TestCase):
    def test_a_talk_only_trainer_is_never_daily_and_other_exclusions_win(self):
        units, errors = classified(obj(trainer_type='TRAINER_TYPE_NONE'))
        self.assertEqual(units[0].status, 'not-daily')
        self.assertTrue(units[0].reason.startswith('talk-only'))
        # An exclusion that applies to any trainer is reported first.
        units, errors = classified(obj(trainer_type='TRAINER_TYPE_NONE', hide='FLAG_HIDE_X'))
        self.assertTrue(units[0].reason.startswith('hidden by a flag'))
        units, errors = classified(obj(trainer_type='TRAINER_TYPE_NONE', problems=('additem',)))
        self.assertTrue(units[0].reason.startswith('script does more than battle'))

    def test_a_plain_trainer_is_daily(self):
        units, errors = classified(obj())
        self.assertEqual(errors, [])
        self.assertEqual((units[0].status, units[0].family), ('daily', 'HIKER'))

    def test_class_copies_merge_into_one_family(self):
        self.assertEqual(G.class_family('TRAINER_CLASS_HIKER_HNS', {}), 'HIKER')
        self.assertEqual(G.class_family('TRAINER_CLASS_HIKER_FRLG', {}), 'HIKER')
        self.assertEqual(G.class_family('TRAINER_CLASS_HIKER', {}), 'HIKER')
        self.assertEqual(G.class_family('TRAINER_CLASS_COOLTRAINER_2', {'COOLTRAINER_2': 'COOLTRAINER'}), 'COOLTRAINER')
        self.assertNotEqual(G.class_family('TRAINER_CLASS_SWIMMER_F', {}), G.class_family('TRAINER_CLASS_SWIMMER_M', {}))

    def test_two_objects_of_a_double_battle_form_a_pair(self):
        units, _ = classified(obj(1, pair=True), obj(2, pair=True))
        self.assertEqual(len(units), 1)
        self.assertEqual((units[0].pair, len(units[0].objects), units[0].status), (True, 2, 'daily'))

    def test_a_pair_needs_exactly_two_objects(self):
        units, _ = classified(obj(1, pair=True))
        self.assertEqual(units[0].status, 'not-daily')
        self.assertIn('pair', units[0].reason)
        units, _ = classified(obj(1), obj(2))
        self.assertEqual(units[0].status, 'not-daily')

    def test_fixed_reasons(self):
        for kind, kwargs, text in (
                ('script', {'problems': ['applymovement']}, 'does more than battle: applymovement'),
                ('buried', {'trainer_type': 'TRAINER_TYPE_BURIED'}, 'buried'),
                ('hidden', {'hide': 'FLAG_HIDE_THEM'}, 'hidden by a flag (FLAG_HIDE_THEM)'),
                ('graphics', {'gfx': 'OBJ_EVENT_GFX_VAR_0'}, 'variable graphics')):
            units, _ = classified(obj(**kwargs))
            self.assertEqual(units[0].status, 'not-daily', kind)
            self.assertIn(text, units[0].reason, kind)

    def test_policy_other_than_ordinary_stays_fixed(self):
        slot = obj()
        slot.policy = 'GYM_MEMBER'
        units, _ = classified(slot)
        self.assertEqual(units[0].status, 'not-daily')
        self.assertIn('GYM_MEMBER', units[0].reason)

    def test_reviewed_map_class_and_slot_exclusions(self):
        reviewed = copy.deepcopy(REVIEWED)
        reviewed.update(not_daily_maps=[{'prefix': 'Map', 'reason': 'a story map'}], not_daily_classes=[], not_daily_slots=[], dialogue=[])
        units, errors = classified(obj(), reviewed=reviewed)
        self.assertEqual((units[0].status, units[0].reason), ('not-daily', 'a story map'))
        reviewed.update(not_daily_maps=[], not_daily_classes=[{'class': 'HIKER', 'reason': 'villains'}])
        units, errors = classified(obj(), reviewed=reviewed)
        self.assertEqual((units[0].status, units[0].reason), ('not-daily', 'villains'))
        reviewed.update(not_daily_classes=[], not_daily_slots=[{'map': 'MapA', 'ids': [1], 'reason': 'authored twice'}])
        units, errors = classified(obj(), reviewed=reviewed)
        self.assertEqual((units[0].status, units[0].reason), ('not-daily', 'authored twice'))

    def test_stale_or_unknown_reviewed_entries_fail(self):
        reviewed = copy.deepcopy(REVIEWED)
        reviewed.update(not_daily_maps=[{'prefix': 'Nowhere', 'reason': 'x'}], not_daily_classes=[{'class': 'NOBODY', 'reason': 'x'}],
                        not_daily_slots=[{'map': 'MapB', 'ids': [1], 'reason': 'x'}, {'map': 'MapA', 'ids': [9], 'reason': 'x'}], dialogue=[])
        _, errors = classified(obj(), reviewed=reviewed)
        joined = '\n'.join(errors)
        self.assertIn('prefix Nowhere matches no trainer slot map', joined)
        self.assertIn('NOBODY', joined)
        self.assertIn("MapB local id 1, which is not a map Wayfarer compiles with trainers", joined)
        self.assertIn('MapA local id 9', joined)


class Groups(unittest.TestCase):
    def daily(self, *specs):
        objects = []
        for number, (home, region_map, family, pair) in enumerate(specs, 1):
            slot = obj(number, home=home, pair=pair)
            slot.map_name, slot.map_const, slot.map_value = region_map, 'MAP_' + region_map, number << 8
            slot.klass = 'TRAINER_CLASS_' + family
            objects.append(slot)
            if pair:
                second = obj(number + 50, home=home, pair=True)
                second.map_name, second.map_const, second.map_value = region_map, 'MAP_' + region_map, number << 8
                second.klass = slot.klass
                objects.append(second)
        units, errors = classified(*objects)
        return units, errors

    def test_groups_split_by_family_region_and_pairs(self):
        old = G.region_of
        try:
            G.region_of = lambda name, root=G.ROOT: 'Hoenn' if name.startswith('H') else 'Kanto'
            units, errors = self.daily(('TRAINER_A', 'Hoenn1', 'HIKER', False), ('TRAINER_B', 'Hoenn2', 'HIKER', False),
                                          ('TRAINER_C', 'Kanto1', 'HIKER', False), ('TRAINER_D', 'Hoenn3', 'TWINS', True),
                                          ('TRAINER_E', 'Hoenn4', 'BIRD_KEEPER', False))
            self.assertEqual(errors, [])
            keys, members = G.make_groups(units, errors)
            self.assertEqual(errors, [])
            self.assertEqual(len(keys), 4)
            by_group = sorted(len(m) for m in members.values())
            self.assertEqual(by_group, [1, 1, 1, 2])
            twins = [u for u in units if u.pair][0]
            hikers = [u for u in units if u.family == 'HIKER' and u.region == 'Hoenn']
            self.assertNotEqual(twins.group, hikers[0].group)
            self.assertEqual(hikers[0].group, hikers[1].group)
        finally:
            G.region_of = old

    def test_a_trainer_in_two_slots_fails(self):
        old = G.region_of
        try:
            G.region_of = lambda name, root=G.ROOT: 'Hoenn'
            units, errors = self.daily(('TRAINER_A', 'M1', 'HIKER', False), ('TRAINER_A', 'M2', 'HIKER', False))
            G.make_groups(units, errors)
            self.assertTrue(any('is the home of 2 daily slots' in e for e in errors), errors)
        finally:
            G.region_of = old

    def test_a_region_without_groups_fails(self):
        old = G.region_of
        try:
            G.region_of = lambda name, root=G.ROOT: 'Other'
            units, errors = self.daily(('TRAINER_A', 'M1', 'HIKER', False))
            G.make_groups(units, errors)
            self.assertTrue(any('region Other has no trainer group' in e for e in errors), errors)
        finally:
            G.region_of = old


class Audit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.built = A.build_patterns(A.map_names(), A.load_words())

    def flags(self, text):
        return [(kind, found.lower()) for kind, found in A.flag_text(text, self.built)]

    def test_map_names_and_their_stems(self):
        self.assertIn(('map name', 'route 118'), self.flags('I walked here from ROUTE 118.'))
        self.assertIn(('map name', 'mt. pyre'), self.flags('MT. PYRE… There is a mysterious power at work there.'))
        self.assertIn(('map name', 'cianwood'), self.flags('Are you going to CIANWOOD?'))
        self.assertIn(('map name', 'pokémon league'), self.flags('The POKéMON LEAGUE is far.'))

    def test_names_written_without_a_space_or_with_several_words_still_match(self):
        self.assertIn(('map name', 'mt.moon'), self.flags('I heard CLEFAIRY appear at MT.MOON.'))
        self.assertIn(('map name', 'mt.mortar'), self.flags("I'm investigating MT.MORTAR."))
        self.assertIn(('map name', 'ever grande'), self.flags('There are tough TRAINERS galore around EVER GRANDE.'))

    def test_a_name_that_is_also_a_word_counts_only_in_capitals(self):
        self.assertIn(('map name', 'victory'), self.flags('This road lives up to its name of VICTORY.'))
        self.assertEqual(self.flags('A victory is a victory.'), [])

    def test_phrasing_that_ties_a_line_to_its_spot_is_flagged_without_a_name(self):
        for text in ('Until I improve, my teacher, who rests here, will never find peace.', 'They halted development here to protect POKéMON.',
                     'My dream ends here again…', 'This is where we DRAGON users do our training.', 'There is an odd place nearby.',
                     'A mystic POKéMON is said to sleep inside here.', 'I was exploring here by myself.', 'That giant rock may hold ruins.'):
            self.assertIn('spot phrase', [kind for kind, _ in self.flags(text)], text)
        for text in ('Here, you can have it!', 'It is a good distance from here.', 'We came here on a dare.', 'I like it here.'):
            self.assertNotIn('spot phrase', [kind for kind, _ in self.flags(text)], text)

    def test_landmarks_buildings_and_demonstrative_terrain(self):
        self.assertIn(('landmark', 'this cave'), self.flags('It is hot in this cave.'))
        self.assertIn(('landmark', 'the lighthouse'), self.flags('Up top is a POKéMON that keeps the LIGHTHOUSE lit.'))
        self.assertIn(('terrain', 'this beach'), self.flags('Isn\'t this beach great?'))
        self.assertIn(('place phrase', 'around here'), self.flags('Have you seen a SAILOR around here?'))

    def test_generic_scenery_and_play_words_are_not_flagged(self):
        for text in ('The sea is my backyard.', 'I found it in the grass.', 'Wow, look at all those BADGES!', 'I want to be a GYM LEADER.',
                     'Aiyiyi! No contest at all!', 'Am I out of my league?', 'GYM BADGES give you advantages.', 'The road ahead is long.',
                     'I am convinced that the sea keeps secrets.'):
            self.assertEqual(self.flags(text), [], text)

    def test_local_events_are_case_sensitive_proper_nouns(self):
        self.assertEqual([k for k, _ in self.flags('I won the Bug-Catching Contest.')], ['local event'])
        self.assertEqual(self.flags('I won at the contest.'), [])
        self.assertIn(('local event', 'gym'), self.flags('Meet me at the GYM later.'))

    def test_reviewed_allowed_phrases_cancel_only_their_span(self):
        self.assertEqual(self.flags('You should aim for the POKéMON LEAGUE.'), [])
        self.assertEqual(self.flags('Are you going to the GYM? Me too!'), [])
        self.assertEqual(self.flags('The GYM BADGES are pretty.'), [])
        self.assertNotEqual(self.flags('The POKéMON LEAGUE is right over there.'), [])
        self.assertNotEqual(self.flags('I am at the GYM now.'), [])

    def test_a_home_range_keeps_the_trainer_daily_with_no_group(self):
        unit, flagged, errors = self.run_audit([{'trainer': 'TRAINER_A', 'range': 'home', 'reason': 'about ROUTE 118'}])
        self.assertEqual(errors, [])
        self.assertEqual((unit.status, unit.range_kind, unit.reason), ('daily', 'home', 'about ROUTE 118'))
        self.assertIsNone(G.group_label(unit))

    def test_a_dialogue_row_may_set_the_absence_chance(self):
        unit, _, errors = self.run_audit([{'trainer': 'TRAINER_A', 'range': 'home', 'absence': 0, 'reason': 'keeps the spot'}])
        self.assertEqual((errors, unit.absence), ([], 0))

    def test_a_place_resolution_must_cover_every_flagged_phrase(self):
        row = {'trainer': 'TRAINER_A', 'range': 'place:cave', 'phrases': ['this cave'], 'reason': 'x'}
        unit, flagged, errors = self.run_audit([row], text='It is hot in this cave.')
        self.assertEqual((errors, unit.status, unit.place), ([], 'daily', 'cave'))
        _, _, errors = self.run_audit([row], text='It is hot in this cave near ROUTE 118.')
        self.assertTrue(any('does not cover' in e for e in errors), errors)
        _, _, errors = self.run_audit([row], text='Hello.')
        self.assertTrue(any('no line is flagged any more' in e for e in errors), errors)
        _, _, errors = self.run_audit([{**row, 'range': 'place:moon'}], text='It is hot in this cave.')
        self.assertTrue(any('range must be class, home or place' in e for e in errors), errors)

    def test_a_challenge_sequence_is_flagged(self):
        self.assertEqual([k for k, _ in self.flags("Here's No. 3! I won't be easy.")], ['sequence'])
        self.assertEqual([k for k, _ in self.flags("I'm second. Now it's serious!")], ['sequence'])
        self.assertEqual([k for k, _ in self.flags("Beat the six of us trainers to win a fabulous prize!")], ['sequence'])
        self.assertEqual(self.flags('The two of us together is much stronger!'), [])

    def run_audit(self, resolutions, text='I live near ROUTE 118.', status='daily', trainer='TRAINER_A'):
        unit, _ = units_of(obj(home=trainer, texts=[('say', 'Text_A')]))
        unit = unit[0]
        unit.status = status
        reviewed = {'dialogue': resolutions}
        errors = []
        flagged, found = G.run_audit([unit], {'Text_A': f'\t.string "{text}$"\n'}, errors, reviewed)
        return unit, flagged, errors

    def test_an_unresolved_flag_fails_generation(self):
        _, flagged, errors = self.run_audit([])
        self.assertIn('TRAINER_A', flagged)
        self.assertTrue(any('has no reviewed resolution' in e for e in errors), errors)

    def test_not_daily_takes_the_trainer_out_with_its_reason(self):
        unit, _, errors = self.run_audit([{'trainer': 'TRAINER_A', 'not_daily': True, 'reason': 'about ROUTE 118'}])
        self.assertEqual(errors, [])
        self.assertEqual(unit.status, 'not-daily')
        self.assertIn('about ROUTE 118', unit.reason)

    def test_a_reword_must_leave_nothing_flagged(self):
        row = {'trainer': 'TRAINER_A', 'range': 'class', 'was': ['I live near ROUTE 118.'], 'reason': 'edited'}
        _, _, errors = self.run_audit([row])
        self.assertTrue(any('still names a place' in e for e in errors), errors)
        _, _, errors = self.run_audit([{k: v for k, v in row.items() if k != 'was'}])
        self.assertTrue(any('keeps its class range but a line names a place' in e for e in errors), errors)
        unit, flagged, errors = self.run_audit([row], text='I live nearby.')
        self.assertEqual((errors, flagged, unit.status), ([], {}, 'daily'))

    def test_stale_resolutions_fail(self):
        _, _, errors = self.run_audit([{'trainer': 'TRAINER_GONE', 'not_daily': True, 'reason': 'x'}], text='Hello.')
        self.assertTrue(any('TRAINER_GONE' in e for e in errors), errors)
        _, _, errors = self.run_audit([{'trainer': 'TRAINER_A', 'resolution': 'keep', 'reason': 'x'}], text='Hello.')
        self.assertTrue(any('needs either not_daily or a range' in e for e in errors), errors)
        _, _, errors = self.run_audit([{'trainer': 'TRAINER_A', 'range': 'home', 'reason': ''}])
        self.assertTrue(any('needs a reason' in e for e in errors), errors)

    def test_only_daily_candidates_are_audited(self):
        _, flagged, errors = self.run_audit([], status='not-daily')
        self.assertEqual((flagged, errors), ({}, []))


class Script(unittest.TestCase):
    """Script analysis on synthetic scripts."""

    @classmethod
    def setUpClass(cls):
        if not HAVE_MAPS:
            raise unittest.SkipTest('needs the Wayfarer mapjson outputs (make BUILD=wayfarer generated)')

    def analyse(self, scripts, entry='Script_A', family=()):
        analyzer = C.Analyzer(scripts, {100: 'TRAINER_A', 101: 'TRAINER_B', 102: 'TRAINER_C'})
        R.FALLTHROUGH.clear()
        return analyzer.analyse(entry, list(family) or ['TRAINER_A'])

    def test_battle_then_speak_is_plain(self):
        facts = self.analyse({'Script_A': '\ttrainerbattle_single 100, Text_Intro, Text_Defeat\n\tmsgbox Text_After, MSGBOX_AUTOCLOSE\n\tend\n',
                              'Text_Intro': '', 'Text_Defeat': '', 'Text_After': ''})
        self.assertEqual((facts['home'], facts['problems'], facts['pair']), (100, [], False))
        self.assertEqual([kind for kind, _ in facts['texts']], ['intro', 'defeat', 'say'])

    def test_rematch_registration_and_a_double_are_plain(self):
        facts = self.analyse({
            'Script_A': '\ttrainerbattle_double 100, Text_Intro, Text_Defeat, Text_Cant, Script_Register\n\tspecialvar 0x800D, ShouldTryRematchBattle\n'
                        '\tgoto_if_eq 0x800D, TRUE, Script_Rematch\n\tmsgbox Text_After, MSGBOX_AUTOCLOSE\n\tend\n',
            'Script_Register': '\tspecial PlayerFaceTrainerAfterBattle\n\twaitmovement 0\n\tregister_matchcall 100\n\trelease\n\tend\n',
            'Script_Rematch': '\ttrainerbattle_rematch_double 100, Text_Intro, Text_Defeat, Text_Cant\n\tmsgbox Text_After, MSGBOX_AUTOCLOSE\n\tend\n',
            'Text_Intro': '', 'Text_Defeat': '', 'Text_Cant': '', 'Text_After': ''})
        self.assertEqual((facts['problems'], facts['pair']), ([], True))

    def test_more_than_a_battle_is_found(self):
        facts = self.analyse({'Script_A': '\ttrainerbattle_single 100, Text_Intro, Text_Defeat\n\tapplymovement 1, Move\n\tsetflag 0x10\n\tremoveobject 1\n\tend\n',
                              'Text_Intro': '', 'Text_Defeat': ''})
        self.assertEqual(sorted(set(facts['problems'])), ['applymovement', 'removeobject', 'setflag'])

    def test_a_second_trainer_is_found(self):
        facts = self.analyse({'Script_A': '\ttrainerbattle_single 100, Text_Intro, Text_Defeat\n\tgoto_if_not_defeated 101, Script_B\n\tend\n',
                              'Script_B': '\tend\n', 'Text_Intro': '', 'Text_Defeat': ''})
        self.assertTrue(any('another trainer' in p for p in facts['problems']), facts['problems'])
        facts = self.analyse({'Script_A': '\ttrainerbattle_two_trainers 100, Text_A, 101, Text_B\n\tend\n', 'Text_A': '', 'Text_B': ''})
        self.assertTrue(facts['two_trainers'])

    def test_a_family_members_defeat_check_is_plain(self):
        facts = self.analyse({'Script_A': '\ttrainerbattle_single 100, Text_Intro, Text_Defeat\n\tgoto_if_defeated 101, Script_B\n\tend\n',
                              'Script_B': '\tend\n', 'Text_Intro': '', 'Text_Defeat': ''}, family=['TRAINER_A', 'TRAINER_B'])
        self.assertEqual(facts['problems'], [])

    def test_a_gift_hand_off_is_plain_and_noted(self):
        gift = sorted(C.gift_flag_names())[0]
        number = C.symbol_numbers({gift}, ('constants/flags.h',))[gift]
        facts = self.analyse({'Script_A': f'\ttrainerbattle_single 100, Text_Intro, Text_Defeat\n\tgoto_if_set {number}, Script_Gift\n\tend\n',
                              'Script_Gift': f'\tmsgbox Text_Has, MSGBOX_DEFAULT\n\tgiveitem 0x4097\n\tclearflag {number}\n\tend\n',
                              'Text_Intro': '', 'Text_Defeat': '', 'Text_Has': ''})
        self.assertEqual(facts['problems'], [])
        self.assertTrue(facts['gift'])

    def test_a_story_flag_clear_is_found(self):
        facts = self.analyse({'Script_A': '\ttrainerbattle_single 100, Text_Intro, Text_Defeat\n\tclearflag 0x7FF\n\tend\n', 'Text_Intro': '', 'Text_Defeat': ''})
        self.assertTrue(any('clearflag' in p for p in facts['problems']), facts['problems'])


class Rendering(unittest.TestCase):
    def test_count_header_and_group_tables(self):
        unit_a, unit_b = [obj(1, 'TRAINER_A'), obj(2, 'TRAINER_B')]
        units, errors = classified(unit_a, unit_b)
        old = G.region_of
        try:
            G.region_of = lambda name, root=G.ROOT: 'Hoenn'
            keys, members = G.make_groups(units, errors)
        finally:
            G.region_of = old
        daily = sorted((u for u in units if u.status == 'daily'), key=lambda u: u.index)
        G.apply_absence(units, {'default_absence_percent': 25, 'absence': []}, errors)
        header = G.render_count(len(daily), len(keys), 0, 2, 2, 64)
        self.assertIn('#define WORLD_TRAINER_SLOT_COUNT        2', header)
        self.assertIn('#define WORLD_TRAINER_GROUP_COUNT       1', header)
        text = G.render_slots(daily, members, keys, [], [255] * 64, 2, 2, [25])
        self.assertIn('{ 0, 2 }, // 0: Hoenn HIKER', text)
        self.assertIn('TRAINER_A', text)
        self.assertEqual(text, G.render_slots(daily, members, keys, [], [255] * 64, 2, 2, [25]))
        self.assertIn('sWorldTrainerAbsence[] = { 25 }', text)
        self.assertIn('WORLD_TRAINER_ABSENCE(0)', text)


class PlaceTypes(unittest.TestCase):
    def placed(self, place, map_type='MAP_TYPE_ROUTE', map_name='MapA', **kw):
        unit = obj(**kw)
        unit.map_type, unit.map_name = map_type, map_name
        units, _ = units_of(unit)
        units[0].status, units[0].place = 'daily', place
        return units

    def test_a_slot_of_the_wrong_kind_fails_generation(self):
        errors = []
        G.check_places(self.placed('cave'), errors)
        self.assertTrue(any('assigned place type cave' in e and 'is not one' in e for e in errors), errors)
        errors = []
        G.check_places(self.placed('ship', map_name='SSAnne_Deck'), errors)
        self.assertTrue(any('place type ship' in e for e in errors), errors)

    def test_a_slot_of_the_right_kind_passes(self):
        for place, kwargs in (('cave', {'map_type': 'MAP_TYPE_UNDERGROUND'}), ('ship', {'map_name': 'AbandonedShip_Rooms_1F'}),
                              ('mansion', {'map_name': 'PokemonMansion_2F_Frlg'}), ('tower', {'map_name': 'SproutTower_1F_hns'})):
            errors = []
            G.check_places(self.placed(place, **kwargs), errors)
            self.assertEqual(errors, [], place)

    def test_an_unknown_place_type_fails(self):
        errors = []
        G.check_places(self.placed('moon'), errors)
        self.assertTrue(any('unknown place type' in e for e in errors), errors)

    def test_a_place_type_trainer_is_in_the_place_group_only(self):
        old = G.region_of
        try:
            G.region_of = lambda name, root=G.ROOT: 'Hoenn'
            a, b, c = obj(1, 'TRAINER_A'), obj(2, 'TRAINER_B'), obj(3, 'TRAINER_C')
            units, errors = classified(a, b, c)
            units[0].place = 'cave'
            units[1].place = 'cave'
            keys, members = G.make_groups(units, errors)
            self.assertEqual(errors, [])
            self.assertEqual(sorted(len(m) for m in members.values()), [1, 2])
            self.assertEqual(sum(len(m) for m in members.values()), 3)
            self.assertEqual(units[0].group, units[1].group)
            self.assertNotEqual(units[0].group, units[2].group)
            self.assertEqual(sorted(k[1] for k in keys), ['@cave', 'HIKER'])
        finally:
            G.region_of = old


class Absence(unittest.TestCase):
    def apply(self, rows, default=25):
        units, errors = classified(obj(1, 'TRAINER_A'), obj(2, 'TRAINER_B'))
        levels = G.apply_absence(units, {'default_absence_percent': default, 'absence': rows}, errors)
        return units, levels, errors

    def test_every_slot_gets_the_default_and_a_reviewed_value_overrides_it(self):
        units, levels, errors = self.apply([{'trainer': 'TRAINER_B', 'percent': 0, 'reason': 'the keeper is always there'}])
        self.assertEqual((errors, levels, [u.absence for u in units]), ([], [25, 0], [25, 0]))
        units, levels, errors = self.apply([])
        self.assertEqual((errors, levels, [u.absence for u in units]), ([], [25], [25, 25]))

    def test_bad_absence_rows_fail(self):
        for row, text in (({'trainer': 'TRAINER_B', 'percent': 101, 'reason': 'x'}, 'percent from 0 to 100'),
                          ({'trainer': 'TRAINER_B', 'percent': 0, 'reason': ''}, 'percent from 0 to 100'),
                          ({'trainer': 'TRAINER_GONE', 'percent': 0, 'reason': 'x'}, 'not a daily trainer slot')):
            _, _, errors = self.apply([row])
            self.assertTrue(any(text in e for e in errors), (row, errors))
        _, _, errors = self.apply([{'trainer': 'TRAINER_B', 'percent': 0, 'reason': 'x'}, {'trainer': 'TRAINER_B', 'percent': 5, 'reason': 'x'}])
        self.assertTrue(any('lists TRAINER_B twice' in e for e in errors), errors)

    def test_the_rendered_row_carries_the_absence_index(self):
        units, levels, errors = self.apply([{'trainer': 'TRAINER_B', 'percent': 0, 'reason': 'x'}])
        old = G.region_of
        try:
            G.region_of = lambda name, root=G.ROOT: 'Hoenn'
            keys, members = G.make_groups(units, errors)
        finally:
            G.region_of = old
        text = G.render_slots(units, members, keys, [], [255] * 64, 2, 2, levels)
        self.assertIn('sWorldTrainerAbsence[] = { 25, 0 }', text)
        self.assertEqual(text.count('WORLD_TRAINER_ABSENCE(0)'), 1)
        self.assertEqual(text.count('WORLD_TRAINER_ABSENCE(1)'), 1)

    def test_a_home_range_row_has_the_home_marker_and_no_group(self):
        units, levels, errors = self.apply([])
        units[0].range_kind = 'home'
        old = G.region_of
        try:
            G.region_of = lambda name, root=G.ROOT: 'Hoenn'
            keys, members = G.make_groups(units, errors)
        finally:
            G.region_of = old
        text = G.render_slots(units, members, keys, [], [255] * 64, 2, 2, levels)
        self.assertIn('WORLD_TRAINER_GROUP_HOME, WORLD_TRAINER_SLOT_HOME | WORLD_TRAINER_ABSENCE(0)', text)
        self.assertEqual(len(keys), 1)


class Reach(unittest.TestCase):
    def test_a_slot_map_without_a_reach_entry_fails(self):
        unit = obj()
        unit.map_const = 'MAP_NOWHERE_AT_ALL'
        units, _ = units_of(unit)
        errors = []
        G.check_reach(units, errors)
        self.assertTrue(any('no reach entry' in e for e in errors), errors)
        units[0].map_const = 'MAP_ROUTE31_HNS'
        errors = []
        G.check_reach(units, errors)
        self.assertEqual(errors, [])


@unittest.skipUnless(HAVE_MAPS or REQUIRE_MAPS, 'needs the Wayfarer mapjson outputs (make BUILD=wayfarer generated)')
class Repository(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.state = G.build()

    def test_committed_output_is_fresh(self):
        for name, content in G.outputs(self.state).items():
            path = G.COUNT_HEADER if name == '@count' else G.OUTPUT / name
            self.assertEqual(path.read_text(), content, f'{path} is stale: run make world-trainers-generate')

    def test_a_trainer_has_one_place(self):
        homes = [u.home for u in self.state['daily']]
        self.assertEqual(len(homes), len(set(homes)))
        fixed = {u.home for u in self.state['units'] if u.status == 'not-daily'}
        self.assertEqual(fixed & set(homes), set())

    def test_every_daily_trainer_is_ordinary_with_a_range_and_a_global_script(self):
        for unit in self.state['daily']:
            self.assertEqual(unit.policy, 'ORDINARY', unit.label)
            self.assertEqual(unit.group is None, unit.range_kind == 'home', unit.label)
            self.assertIn(unit.range_kind, ('class', 'place', 'home'), unit.label)
            self.assertEqual(len(unit.objects), 2 if unit.pair else 1, unit.label)
        members = self.state['members']
        homes = sum(1 for u in self.state['daily'] if u.range_kind == 'home')
        self.assertEqual(sum(len(m) for m in members.values()) + homes, len(self.state['daily']))

    def test_group_slots_are_listed_in_slot_order(self):
        for group in self.state['members'].values():
            indexes = [u.index for u in sorted(group, key=lambda u: u.index)]
            self.assertEqual(indexes, sorted(indexes))
        self.assertLessEqual(max(len(m) for m in self.state['members'].values()), self.state['group_max'])

    def test_home_ranges_use_no_group_and_the_route_25_sequence_is_not_daily(self):
        homes = [u for u in self.state['daily'] if u.range_kind == 'home']
        self.assertTrue(homes)
        for unit in homes:
            self.assertIsNone(unit.group, unit.label)
            self.assertEqual(self.state['resolutions'][unit.home]['range'], 'home')
        by_home = {u.home: u for u in self.state['units']}
        for name in ('DUDLEY', 'JOE', 'LAURA', 'LLOYD', 'ELLEN', 'SHANNON'):
            unit = by_home[f'TRAINER_{name}_HNS']
            self.assertEqual((unit.status, unit.range_kind), ('not-daily', None), name)
        self.assertEqual(sum(1 for r in self.state['resolutions'].values() if r.get('not_daily')), 6)
        self.assertLessEqual(len(self.state['keys']), G.GROUP_HOME)

    def test_every_slot_has_an_absence_from_the_table(self):
        for unit in self.state['daily']:
            self.assertIn(unit.absence, self.state['levels'], unit.label)
        self.assertLessEqual(len(self.state['levels']), 8)
        self.assertEqual(self.state['levels'][0], self.state['reviewed']['default_absence_percent'])

    def test_every_flagged_trainer_has_a_resolution_and_no_exclusion_is_stale(self):
        resolutions = self.state['resolutions']
        for trainer in self.state['flagged']:
            self.assertIn(trainer, resolutions)
        homes = {u.home: u for u in self.state['units']}
        for trainer, row in resolutions.items():
            if row.get('not_daily'):
                self.assertEqual(homes[trainer].status, 'not-daily')

    def test_a_talk_only_rematch_lead_stays_at_a_fixed_home(self):
        tully = [u for u in self.state['fixed_homes'] if u.home == 'TRAINER_TULLY_HNS']
        self.assertEqual(len(tully), 1)
        self.assertTrue(tully[0].reason.startswith('talk-only'))
        self.assertIn('talk-only', (G.ROOT / 'src/data/trainer_slots/report.md').read_text())

    def test_fixed_contacts_keep_a_home(self):
        homes = {u.home for u in self.state['fixed_homes']}
        for unit in self.state['fixed_homes']:
            self.assertEqual(unit.status, 'not-daily')
        # A rematch family's first trainer stands at its fixed home exactly when it is not a daily slot.
        self.assertEqual(homes & {u.home for u in self.state['daily']}, set())

    def test_every_slot_of_a_place_group_is_that_kind_of_place(self):
        specs = G.P.load_specs()
        placed = [u for u in self.state['daily'] if u.place]
        self.assertTrue(placed)
        world = G.P.load_world({u.map_name for u in placed})
        for unit in placed:
            self.assertTrue(G.P.holds(specs[unit.place], unit, world), unit.label)
        # A trainer is in one group only: its place group or its class group, never both.
        groups = [u.group for u in self.state['daily']]
        self.assertEqual(len(groups), len(self.state['daily']))
        for gid, members in self.state['members'].items():
            kinds = {G.group_label(u) for u in members}
            self.assertEqual(len(kinds), 1, gid)

    def test_the_sevii_contacts_fit_their_reserve(self):
        contacts = [c for c in self.state['sevii'] if c != 255]
        self.assertEqual(len(contacts), len(set(contacts)))
        self.assertTrue(all(G.SEVII_CONTACT_BASE <= c < G.COAST_CONTACT_BASE for c in contacts))

    def test_dialogue_inventory_lists_every_candidate(self):
        text = G.render_dialogue(self.state['daily'], self.state['flagged'])
        for unit in self.state['daily']:
            self.assertIn(f'| {unit.home} |', text, unit.label)


if __name__ == '__main__':
    unittest.main()
