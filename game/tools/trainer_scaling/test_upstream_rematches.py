"""Protect trainer identity and route selection across HNS and Wayfarer builds."""
import json
import re
import unittest

import generate as gen


KANTO = ('BROCK', 'MISTY', 'LTSURGE', 'ERIKA', 'JANINE', 'SABRINA', 'BLAINE', 'BLUE')
JOHTO = ('FALKNER', 'BUGSY', 'WHITNEY', 'MORTY', 'CHUCK', 'JASMINE', 'PRYCE', 'CLAIR')
LEAGUE = {'Brunos': 'BRUNO', 'Kogas': 'KOGA', 'Karens': 'KAREN', 'Wills': 'WILL', 'Champions': 'LANCE'}
POSTOBC = ('FALKNER', 'BUGSY', 'WHITNEY', 'MORTY', 'CHUCK', 'JASMINE', 'PRYCE',
           'CLAIR', 'BROCK', 'LTSURGE', 'JANINE', 'SABRINA', 'BLAINE', 'BLUE',
           'WILL', 'KOGA', 'BRUNO', 'KAREN', 'LANCE', 'RED', 'MISTY', 'ERIKA')


def postobc_id(leader):
    return 'TRAINER_CLAIR_OBC_HNS' if leader == 'CLAIR' else f'TRAINER_{leader}_POSTOBC_HNS'


def script(map_name, wayfarer):
    return gen.command(['cpp', '-traditional-cpp', '-P', f'-DIS_WAYFARER={int(wayfarer)}',
                        '-DWAYFARER_LEAGUE_CIRCUIT_ENABLED=1',
                        str(gen.ROOT / 'data/maps' / map_name / 'scripts.inc')])


def battle_ids(source):
    return set(re.findall(r'\btrainerbattle_no_intro\s+(TRAINER_\w+)', source))


class UpstreamRematchTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw = gen.load_inventory()
        cls.ids = gen.trainer_ids(cls.raw)

    def test_all_compiled_trainers_have_distinct_ids_and_postobc_stays_after_hoenn(self):
        self.assertEqual(len(self.ids), len(set(self.ids.values())))
        self.assertEqual(self.ids['TRAINER_SAWYER_1'], 639)
        self.assertEqual(self.ids['TRAINER_MAY_PLACEHOLDER'], 1492)
        for i, leader in enumerate(KANTO):
            self.assertEqual(self.ids[f'TRAINER_{leader}_DOJO_HNS'], 631 + i)
        for i, leader in enumerate(POSTOBC):
            self.assertEqual(self.ids[postobc_id(leader)], 1493 + i)

    def test_hns_retains_compact_postobc_ids_and_capacity(self):
        names = [postobc_id(leader) for leader in POSTOBC]
        source = '#include "constants/opponents.h"\n' + '\n'.join(
            f'ID({name})' for name in names + ['TRAINERS_COUNT', 'MAX_TRAINERS_COUNT'])
        output = gen.command(['cpp', '-P', '-DIS_WAYFARER=0', '-DIS_HNS=1',
                              '-I', str(gen.ROOT / 'include'), '-'], input=source)
        values = [int(value) for value in re.findall(r'ID\(\(?(\d+)\)?\)', output)]
        self.assertEqual(values, list(range(639, 661)) + [661, 864])

    def test_postobc_rosters_are_complete_and_excluded_from_scaling(self):
        manifest = {row['id']: row for row in gen.legacy_manifest(
            gen.resolve_rosters(self.raw), gen.references())['records']}
        for leader in POSTOBC:
            trainer = postobc_id(leader)
            self.assertEqual(self.raw[trainer]['DIFFICULTY_NORMAL']['partySize'], 6)
            self.assertEqual(manifest[trainer]['policy'], 'EXCLUDED')
            self.assertTrue(manifest[trainer]['reason'])

    def test_wayfarer_dojo_keeps_authored_rematches_for_every_leader(self):
        source = script('SaffronCity_FightingDojoVIP_hns', True)
        self.assertNotIn('PostOBC', source)
        expected = {f'TRAINER_{leader}_DOJO_HNS' for leader in KANTO}
        expected.update(f'TRAINER_{leader}_2_HNS' for leader in JOHTO)
        self.assertEqual(battle_ids(source), expected)

    def test_hns_dojo_keeps_standard_and_postobc_routes(self):
        source = script('SaffronCity_FightingDojoVIP_hns', False)
        expected = {f'TRAINER_{leader}_HNS' for leader in KANTO}
        expected.update(f'TRAINER_{leader}_2_HNS' for leader in JOHTO)
        expected.update(postobc_id(leader) for leader in KANTO + JOHTO)
        self.assertEqual(battle_ids(source), expected)
        for leader in KANTO + JOHTO:
            self.assertIn(postobc_id(leader), source)
        self.assertEqual(source.count('goto_if_set FLAG_USED_TELEPORTER,'), 16)

    def test_postgame_flags_cannot_select_postobc_league_or_red_in_wayfarer(self):
        for room, leader in LEAGUE.items():
            map_name = f'PokemonLeague_{room}Room_hns'
            wayfarer = script(map_name, True)
            hns = script(map_name, False)
            self.assertNotIn('PostOBC', wayfarer)
            self.assertIn(f'TRAINER_{leader}_2_HNS', battle_ids(wayfarer))
            self.assertIn(postobc_id(leader), battle_ids(hns))
            self.assertIn('goto_if_set FLAG_USED_TELEPORTER,', hns)
        wayfarer = script('MtSilver_SummitDay_hns', True)
        self.assertEqual(battle_ids(wayfarer), {'TRAINER_RED_2_HNS'})
        self.assertNotIn('PostOBC', wayfarer)
        self.assertEqual(battle_ids(script('MtSilver_SummitDay_hns', False)),
                         {'TRAINER_RED_2_HNS', 'TRAINER_RED_POSTOBC_HNS'})


if __name__ == '__main__':
    unittest.main()
