from pathlib import Path
import tempfile
import json
import subprocess
import unittest
from unittest.mock import patch

from tools.gameplay_content.__main__ import ROOT, compile_content, publish
from tools.gameplay_content.common import ContentError
from tools.gameplay_content.configuration import numeric_defines

FLAGS = {'wayfarer': '-DPOKEMON_WAYFARER -DPOKEMON_HNS', 'hns': '-DPOKEMON_HNS',
         'emerald': '-DPOKEMON_EMERALD', 'firered': '-DFIRERED', 'leafgreen': '-DLEAFGREEN'}

class IntegrationTests(unittest.TestCase):
    def test_domain_feature_defaults_and_numeric_zero(self):
        enabled = numeric_defines(ROOT, 'cpp', FLAGS['wayfarer'])
        self.assertEqual(enabled['WAYFARER_TR_MARTS_ENABLED'], 1)
        self.assertEqual(enabled['B_TRAINER_PARTY_SCALING'], 1)
        self.assertEqual(enabled['B_GYM_LEADER_SCALING'], 0)
        self.assertEqual(enabled['B_LEAGUE_SCALING'], 1)
        disabled = numeric_defines(ROOT, 'cpp', FLAGS['wayfarer'] +
                                  ' -DWAYFARER_TR_MARTS_ENABLED=0 -DB_TRAINER_PARTY_SCALING=0 -DB_LEAGUE_SCALING=0')
        self.assertEqual(disabled['WAYFARER_TR_MARTS_ENABLED'], 0)
        self.assertEqual(disabled['B_TRAINER_PARTY_SCALING'], 0)
        self.assertEqual(disabled['B_LEAGUE_SCALING'], 0)

    def test_real_preprocessor_rejects_every_wrong_product(self):
        for actual, flags in FLAGS.items():
            defines = numeric_defines(ROOT, 'cpp', flags)
            for requested in FLAGS:
                if actual == requested:
                    continue
                with self.subTest(actual=actual, requested=requested):
                    with patch('tools.gameplay_content.__main__.numeric_defines', return_value=defines):
                        with self.assertRaisesRegex(ContentError, 'INACTIVE'):
                            compile_content(ROOT, requested, 'cpp', flags, ('inventory',))

    @unittest.skipUnless((ROOT / 'tools/mapjson/mapjson').exists(), 'build mapjson before integration test')
    def test_real_inventory_is_deterministic(self):
        first = compile_content(ROOT, 'wayfarer', 'cpp', FLAGS['wayfarer'], ('inventory',))
        second = compile_content(ROOT, 'wayfarer', 'cpp', FLAGS['wayfarer'], ('inventory',))
        self.assertEqual(first, second)

    @unittest.skipUnless((ROOT / 'tools/mapjson/mapjson').exists(), 'build mapjson before integration test')
    def test_export_uses_catalog_connections_and_shared_event_owner(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            maps = root / 'data/maps'
            maps.mkdir(parents=True)
            (maps / 'map_groups.json').write_text(json.dumps({'group_order': ['g'], 'g': ['A', 'B']}))
            for name, record in {
                'A': {'id': 'MAP_A', 'name': 'A', 'game_version': 'hns', 'region': 'REGION_KANTO', 'shared_events_map': 'B',
                      'connections': [{'map': 'MAP_B'}, {'map': 'MAP_ABSENT'}]},
                'B': {'id': 'MAP_B', 'name': 'B', 'game_version': 'emerald',
                      'object_events': [{'script': 'OpaqueNurse'}],
                      'warp_events': [{'dest_map': 'MAP_ABSENT'}]}}.items():
                (maps / name).mkdir()
                (maps / name / 'map.json').write_text(json.dumps(record))
            output = subprocess.check_output([str(ROOT / 'tools/mapjson/mapjson'), 'inventory',
                                              'wayfarer', 'data/maps/map_groups.json'], cwd=root, text=True)
            a, b = json.loads(output)['maps']
            self.assertEqual(a['sourceNamespace'], 'hns')
            self.assertEqual(a['physicalRegion'], 'REGION_KANTO')
            self.assertEqual(b['sourceNamespace'], 'emerald')
            self.assertEqual(b['physicalRegion'], 'REGION_HOENN')
            self.assertEqual(b['physicalRegionResolution'], 'native_hoenn_catalog')
            self.assertEqual(a['effective']['connections'], [{'map': 'MAP_B'}])
            self.assertEqual(a['effective']['object_events'], [{'script': 'OpaqueNurse'}])
            self.assertEqual(a['eventSourcePath'], 'data/maps/B/map.json')
            self.assertEqual(a['raw']['shared_events_map'], 'B')
            # Current mapjson emits warps unconditionally; inventory cannot invent filtering.
            self.assertEqual(b['effective']['warp_events'], [{'dest_map': 'MAP_ABSENT'}])

    def test_authored_change_makes_check_stale_without_rewriting(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'data/gameplay/progression.json'
            source.parent.mkdir(parents=True)
            source.write_text('{"reviewed":1}')
            defines = {'IS_WAYFARER': 0, 'IS_HNS': 0, 'IS_FRLG': 0, 'GAMEPLAY_LEAFGREEN': 0}
            with patch('tools.gameplay_content.__main__.numeric_defines', return_value=defines), \
                 patch('tools.gameplay_content.__main__.load_maps', return_value=[]), \
                 patch('tools.gameplay_content.__main__.load_trainers', return_value={}):
                digest, outputs = compile_content(root, 'emerald', 'cpp', '', ('inventory',))
                publish(root / 'build', 'emerald', digest, outputs)
                source.write_text('{"reviewed":2}')
                changed_digest, changed_outputs = compile_content(root, 'emerald', 'cpp', '', ('inventory',))
                self.assertNotEqual(digest, changed_digest)
                with self.assertRaisesRegex(ContentError, 'STALE_REVIEW'):
                    publish(root / 'build', 'emerald', changed_digest, changed_outputs, check=True)
                self.assertEqual((root / 'build/emerald/current').resolve().name, digest)

if __name__ == '__main__':
    unittest.main()
