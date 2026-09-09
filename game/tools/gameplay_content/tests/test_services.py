"""Independent baseline expectations and service authoring failures."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from tools.gameplay_content.common import ContentError
from tools.gameplay_content.maps import load_maps
from tools.gameplay_content.services import compile_services, validate_service

ROOT = Path(__file__).resolve().parents[3]


class ServiceFixtures(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        for path, text in {
            'src/data/wayfarer_marts.h': 'static const struct Stock sWayfarerMartProfiles[] = { [MART_PROFILE_TEST] = MART_PROFILE_EMPTY_FACILITY(MART_CATEGORY_TOWN), };',
            'include/constants/wayfarer_marts.h': '#define MART_PROFILE_TEST 1\n',
            'include/constants/global.h': '',
            'include/constants/flags.h': '#define FLAG_TEST 123\n#define FLAG_OTHER 124\n',
            'include/config/wayfarer_marts.h': '',
        }.items():
            target = self.root / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(text)
        self.service = {'id': 'giver', 'products': ['wayfarer'], 'kind': 'rod_contribution',
                        'binding': {'script': 'GiveRod'}, 'contribution': {'namespace': 'global', 'flag': 'FLAG_TEST'}}
        self.map = {'id': 'MAP_TEST', 'name': 'Test', 'sourceNamespace': 'emerald', 'physicalRegion': 'hoenn',
                    'objects': [{'script': 'GiveRod', 'runtimeId': 1, 'localId': None, 'x': 3, 'y': 4, 'flag': '0'}]}
        self.write([self.service], 'GiveRod::\n setvar VAR_0x8004, FLAG_TEST\n special Script_TryAwardStandardRod\n end\n')

    def write(self, services, script=None):
        path = self.root / 'data/maps/Test/gameplay.json'
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps({'schemaVersion': 1, 'services': services, 'encounters': []}))
        if script is not None:
            path.with_name('scripts.inc').write_text(script)

    def compile(self, **kwargs):
        return compile_services(self.root, 'wayfarer', {'IS_WAYFARER': 1, 'WAYFARER_TR_MARTS_ENABLED': 1}, [self.map], **kwargs)

    def test_added_and_removed_rod_changes_membership(self):
        result = self.compile()
        self.assertEqual(result['report']['rodCount'], 1)
        self.assertIn('X(FLAG_TEST)', result['outputs']['gameplay_services.h'])
        self.write([], 'GiveRod::\n end\n')
        self.assertEqual(self.compile()['report']['rodCount'], 0)

    def test_live_missing_giver_rejected(self):
        self.write([])
        with self.assertRaisesRegex(ContentError, 'live Standard Rod giver'):
            self.compile()

    def test_unrelated_handler_flag_cannot_validate_binding(self):
        self.write([self.service], 'GiveRod::\n setvar VAR_0x8004, FLAG_OTHER\n special Script_TryAwardStandardRod\n end\nUnrelated::\n setvar VAR_0x8004, FLAG_TEST\n end\n')
        with self.assertRaisesRegex(ContentError, 'rod script contribution'):
            self.compile()

    def add_alias(self, target='MAP_TEST/giver', flag='FLAG_TEST'):
        alias = dict(self.service, id='alias', binding={'script': 'OtherRod'},
                     contribution={'namespace': 'global', 'flag': flag, 'aliasOf': target})
        self.map['objects'].append(dict(self.map['objects'][0], script='OtherRod', runtimeId=2))
        self.write([self.service, alias], f'GiveRod::\n setvar VAR_0x8004, FLAG_TEST\n special Script_TryAwardStandardRod\n end\nOtherRod::\n setvar VAR_0x8004, {flag}\n special Script_TryAwardStandardRod\n end\n')
        return alias

    def test_explicit_shared_contribution_counts_once(self):
        self.add_alias()
        result = self.compile()
        self.assertEqual(result['report']['rodCount'], 1)
        self.assertEqual(len(result['report']['services']), 2)
        self.assertEqual(result['outputs']['gameplay_services.h'].count('X(FLAG_TEST)'), 1)

    def test_alias_requires_matching_identity_and_existing_target(self):
        self.add_alias(flag='FLAG_OTHER')
        with self.assertRaisesRegex(ContentError, 'alias identity'):
            self.compile()
        self.map['objects'].pop()
        self.add_alias(target='MAP_MISSING/giver')
        with self.assertRaisesRegex(ContentError, 'UNRESOLVED'):
            self.compile()

    def test_alias_cycles_rejected(self):
        alias = self.add_alias()
        self.service['contribution']['aliasOf'] = 'MAP_TEST/alias'
        self.write([self.service, alias])
        with self.assertRaisesRegex(ContentError, 'alias cycle'):
            self.compile()

    def test_undeclared_shared_identity_rejected(self):
        alias = self.add_alias()
        del alias['contribution']['aliasOf']
        self.write([self.service, alias])
        with self.assertRaisesRegex(ContentError, 'requires aliasOf'):
            self.compile()

    def test_comment_and_inactive_stock_profiles_are_not_live(self):
        mart = {'id': 'shop', 'products': ['wayfarer'], 'kind': 'mart', 'binding': {'script': 'GiveRod'}, 'profile': 'MART_PROFILE_BOGUS'}
        self.write([mart], 'GiveRod::\n end\n')
        path = self.root / 'src/data/wayfarer_marts.h'
        text = path.read_text().replace('};', '// [MART_PROFILE_BOGUS] = MART_PROFILE_EMPTY_FACILITY(MART_CATEGORY_TOWN),\n};')
        path.write_text(text)
        with self.assertRaisesRegex(ContentError, 'UNRESOLVED'):
            self.compile()
        path.write_text(text.replace('// [MART_PROFILE_BOGUS]', '\n#if 0\n[MART_PROFILE_BOGUS]').replace('\n};', '\n#endif\n};'))
        with self.assertRaisesRegex(ContentError, 'UNRESOLVED'):
            self.compile()
        path.write_text('#if 0\n' + text.replace('// ', '') + '\n#endif\n')
        with self.assertRaisesRegex(ContentError, 'UNRESOLVED'):
            self.compile()

    def test_stock_profile_requires_numeric_production_constant(self):
        path = self.root / 'include/constants/wayfarer_marts.h'
        path.write_text('')
        with self.assertRaisesRegex(ContentError, 'UNRESOLVED'):
            self.compile()

    def test_nonunique_script_rejected(self):
        self.map['objects'].append(dict(self.map['objects'][0], runtimeId=2))
        with self.assertRaisesRegex(ContentError, 'CONFLICT'):
            self.compile()

    def test_reordered_event_derives_new_id(self):
        self.map['objects'][0]['runtimeId'] = 5
        self.assertEqual(self.compile()['report']['services'][0]['event']['runtimeId'], 5)

    def test_numeric_local_id_and_wrong_bank_rejected(self):
        for field, value in [('binding', {'script': 'GiveRod', 'localId': '2'}),
                             ('contribution', {'namespace': 'hoenn', 'flag': 'FLAG_TEST'})]:
            service = dict(self.service, **{field: value})
            with self.assertRaises(ContentError):
                validate_service(service, 'fixture')

    def test_duplicate_binding_rejected(self):
        self.write([self.service, dict(self.service, id='another')])
        with self.assertRaisesRegex(ContentError, 'CONFLICT'):
            self.compile()

    def test_add_existing_mart_profile(self):
        mart = {'id': 'shop', 'products': ['wayfarer'], 'kind': 'mart', 'binding': {'script': 'Shop'}, 'profile': 'MART_PROFILE_TEST'}
        self.map['objects'][0]['script'] = 'Shop'
        self.write([mart], 'Shop::\n setvar VAR_0x8004, GAMEPLAY_MART_TEST_SHOP\n special WayfarerOpenMartProfile\n end\n')
        result = self.compile()
        self.assertEqual(result['report']['martCount'], 1)
        self.assertIn('.set GAMEPLAY_MART_TEST_SHOP, MART_PROFILE_TEST', result['outputs']['gameplay_mart_bindings.inc'])

    def test_disabled_mart_does_not_resolve_inactive_binding(self):
        mart = {'id': 'shop', 'products': ['wayfarer'], 'kind': 'mart', 'binding': {'script': 'Missing'}, 'profile': 'MART_PROFILE_TEST'}
        self.write([mart], '')
        result = compile_services(self.root, 'wayfarer', {'IS_WAYFARER': 1, 'WAYFARER_TR_MARTS_ENABLED': 0}, [])
        self.assertEqual(result['report']['martCount'], 0)
        self.assertEqual(len(result['report']['disabledServices']), 1)
        with self.assertRaisesRegex(ContentError, 'UNRESOLVED'):
            self.compile()

    def test_symbol_collision_rejected(self):
        first = {'id': 'a-b', 'products': ['wayfarer'], 'kind': 'mart', 'binding': {'script': 'One'}, 'profile': 'MART_PROFILE_TEST'}
        second = dict(first, id='a_b', binding={'script': 'Two'})
        self.map['objects'] = [dict(self.map['objects'][0], script=script, runtimeId=i) for i, script in enumerate(('One', 'Two'), 1)]
        self.write([first, second], '')
        with self.assertRaisesRegex(ContentError, 'symbol collision'):
            self.compile(validate_scripts=False)


class SelectedProductBaseline(unittest.TestCase):
    def test_independent_contributor_flags_and_mart_coverage(self):
        six = {'FLAG_STANDARD_ROD_ROUTE32_CONTRIBUTED', 'FLAG_STANDARD_ROD_OLIVINE_CONTRIBUTED',
               'FLAG_STANDARD_ROD_ROUTE12_CONTRIBUTED', 'FLAG_STANDARD_ROD_DEWFORD_CONTRIBUTED',
               'FLAG_STANDARD_ROD_ROUTE118_CONTRIBUTED', 'FLAG_STANDARD_ROD_MOSSDEEP_CONTRIBUTED'}
        for product, switches, expected in (
            ('wayfarer', ['-DPOKEMON_WAYFARER', '-DPOKEMON_HNS'], six),
            ('hns', ['-DPOKEMON_HNS'], six),
            ('emerald', ['-DEMERALD'], {'FLAG_RECEIVED_OLD_ROD', 'FLAG_RECEIVED_GOOD_ROD', 'FLAG_RECEIVED_SUPER_ROD'}),
            ('firered', ['-DFIRERED'], {'FLAG_GOT_OLD_ROD', 'FLAG_GOT_GOOD_ROD', 'FLAG_GOT_SUPER_ROD'}),
            ('leafgreen', ['-DLEAFGREEN'], {'FLAG_GOT_OLD_ROD', 'FLAG_GOT_GOOD_ROD', 'FLAG_GOT_SUPER_ROD'}),
        ):
            with self.subTest(product=product):
                result = compile_services(ROOT, product, {'IS_WAYFARER': int(product == 'wayfarer'), 'WAYFARER_TR_MARTS_ENABLED': 1}, load_maps(ROOT, product), cppflags=switches)
                report = result['report']
                contributors = [row for row in report['services'] if row['kind'] == 'rod_contribution'] + report['legacyRodContributors']
                self.assertEqual({row['contribution']['flag'] for row in contributors}, expected)
                self.assertEqual(report['martCount'], 35 if product == 'wayfarer' else 0)
                if product == 'hns':
                    self.assertEqual(len(report['legacyRodContributors']), 3)
                    self.assertTrue(all(not row['activeBinding'] for row in report['legacyRodContributors']))


if __name__ == '__main__':
    unittest.main()
