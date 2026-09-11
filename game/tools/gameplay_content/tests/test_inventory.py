import copy
import json
from pathlib import Path
import tempfile
import unittest

from tools.gameplay_content.common import ContentError, assign_indexes, check_review, fingerprint, load_json
from tools.gameplay_content import maps
from tools.gameplay_content.maps import normalize_inventory, resolve_object
from tools.gameplay_content.__main__ import inputs, publish, compile_content
from unittest.mock import patch

class InventoryTests(unittest.TestCase):
    def fixture(self):
        raw = {'object_events': [
            {'script': 'Trainer', 'x': 1}, {'script': 'OldClerk', 'x': 2},
            {'script': 'OpaqueNurse', 'x': 3}], 'warp_events': [{'dest_map': 'REMOVED'}]}
        effective = {'object_events': [{'script': 'NewClerk', 'x': 2},
                                       {'script': 'OpaqueNurse', 'x': 3}], 'warp_events': []}
        return {'schemaVersion': 1, 'product': 'wayfarer', 'maps': [
            {'id': 'MAP_FIXTURE', 'name': 'Fixture', 'sourceNamespace': 'frlg',
             'physicalRegion': 'REGION_SEVII', 'sourcePath': 'fixture/map.json',
             'raw': raw, 'effective': effective}]}

    def test_effective_overlay_and_raw_provenance(self):
        row, = normalize_inventory(self.fixture(), 'wayfarer')
        self.assertEqual([x['script'] for x in row['objects']], ['NewClerk', 'OpaqueNurse'])
        self.assertEqual(row['effective']['warp_events'], [])
        self.assertEqual(row['raw']['object_events'][0]['script'], 'Trainer')
        self.assertEqual(row['sourceNamespace'], 'frlg')
        self.assertEqual(row['physicalRegion'], 'REGION_SEVII')
        self.assertEqual(resolve_object(row, {'script': 'NewClerk'}, 'fixture', 'clerk')['runtimeId'], 1)
        with self.assertRaisesRegex(ContentError, 'UNRESOLVED'):
            resolve_object(row, {'script': 'OldClerk'}, 'fixture', 'clerk')

    def test_review_drift(self):
        row = self.fixture()['maps'][0]
        reviewed = fingerprint(row['raw'])
        row['raw']['object_events'][0]['x'] = 4
        with self.assertRaisesRegex(ContentError, 'STALE_REVIEW'):
            check_review(row['raw'], reviewed, 'fixture', 'trainer')

    def test_ambiguous_handler(self):
        row, = normalize_inventory(self.fixture(), 'wayfarer')
        row['objects'].append(copy.deepcopy(row['objects'][0]))
        with self.assertRaisesRegex(ContentError, 'CONFLICT'):
            resolve_object(row, {'script': 'NewClerk'}, 'fixture', 'clerk')

    def test_wrong_product(self):
        with self.assertRaisesRegex(ContentError, 'INACTIVE'):
            normalize_inventory(self.fixture(), 'emerald')

    def test_configuration_rejects_wrong_standalone_product(self):
        with patch('tools.gameplay_content.__main__.numeric_defines', return_value={
                'IS_WAYFARER': 0, 'IS_HNS': 1, 'IS_FRLG': 0, 'GAMEPLAY_LEAFGREEN': 0}):
            with self.assertRaisesRegex(ContentError, 'INACTIVE'):
                compile_content(Path('.'), 'emerald', 'cpp', '', ('inventory',))

    def test_duplicate_json(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'bad.json'
            path.write_text('{"schemaVersion":1,"schemaVersion":1}')
            with self.assertRaisesRegex(ContentError, 'DUPLICATE'):
                load_json(path)

    def test_wayfarer_inventory_uses_manifest_when_present(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'data/maps/Fixture').mkdir(parents=True)
            (root / 'data/maps/Fixture/map.json').write_text('{}')
            (root / 'data/maps/map_groups.json').write_text('{}')
            manifest = root / 'src/data/wayfarer_sevii_maps.json'
            manifest.parent.mkdir(parents=True)
            manifest.write_text('{}')
            completed = type('Completed', (), {
                'returncode': 0,
                'stdout': json.dumps({'schemaVersion': 1, 'product': 'wayfarer', 'maps': []}),
            })()
            with patch.object(maps.subprocess, 'run', return_value=completed) as run:
                self.assertEqual(maps.load_maps(root, 'wayfarer'), [])
            self.assertEqual(
                run.call_args.args[0][-2:],
                ['--wayfarer-sevii-manifest', 'src/data/wayfarer_sevii_maps.json'],
            )

    def test_inventory_keeps_synthetic_and_standalone_catalogs_unextended(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'data/maps/Fixture').mkdir(parents=True)
            (root / 'data/maps/Fixture/map.json').write_text('{}')
            (root / 'data/maps/map_groups.json').write_text('{}')
            completed = type('Completed', (), {
                'returncode': 0,
                'stdout': json.dumps({'schemaVersion': 1, 'product': 'hns', 'maps': []}),
            })()
            with patch.object(maps.subprocess, 'run', return_value=completed) as run:
                self.assertEqual(maps.load_maps(root, 'hns'), [])
            self.assertNotIn('--wayfarer-sevii-manifest', run.call_args.args[0])

    def test_manifest_is_a_compiler_input(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = root / 'src/data/wayfarer_sevii_maps.json'
            manifest.parent.mkdir(parents=True)
            manifest.write_text('{"release_link_enabled":true}')
            first = inputs(root)
            manifest.write_text('{"release_link_enabled":false}')
            second = inputs(root)
            self.assertIn('src/data/wayfarer_sevii_maps.json', first)
            self.assertNotEqual(first['src/data/wayfarer_sevii_maps.json'], second['src/data/wayfarer_sevii_maps.json'])

    def test_indexes_capacity_and_duplicates(self):
        with self.assertRaisesRegex(ContentError, 'CAPACITY'):
            assign_indexes([{}] * 65536)
        with self.assertRaisesRegex(ContentError, 'DUPLICATE'):
            assign_indexes([{'key': ['x']}, {'key': ['x']}])
        self.assertEqual([r['key'] for r in assign_indexes([{'key':['b']}, {'key':['a']}])], [['a'], ['b']])

    def test_atomic_generations_and_check(self):
        with tempfile.TemporaryDirectory() as directory:
            first = publish(directory, 'wayfarer', 'a', {'one': 'first', 'two': 'second'})
            before = (first / 'one').stat().st_mtime_ns
            publish(directory, 'wayfarer', 'a', {'one': 'first', 'two': 'second'})
            self.assertEqual(before, (first / 'one').stat().st_mtime_ns)
            publish(directory, 'wayfarer', 'a', {'one': 'first', 'two': 'second'}, check=True)
            with self.assertRaisesRegex(ContentError, 'STALE_REVIEW'):
                publish(directory, 'wayfarer', 'b', {'one': 'third'}, check=True)
            self.assertEqual((Path(directory) / 'wayfarer/current/one').read_text(), 'first')
            with self.assertRaisesRegex(ContentError, 'CONFLICT'):
                publish(directory, 'wayfarer', 'a', {'one': 'corrupt'})
            self.assertEqual((Path(directory) / 'wayfarer/current/one').read_text(), 'first')

if __name__ == '__main__':
    unittest.main()
