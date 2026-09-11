"""Phase-D fallback-policy ownership and caller-agreement contracts."""
import copy
import json
from pathlib import Path
import tempfile
import unittest

from tools.gameplay_content.common import ContentError
from tools.gameplay_content.trainers import compile_scaling_policies, load_legacy_trainers


class TrainerPolicyFixtures(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.trainers = {
            'TRAINER_TEST': {
                'DIFFICULTY_NORMAL': {
                    'source': 'src/data/trainers.party',
                    'trainerClass': 'TRAINER_CLASS_YOUNGSTER',
                    'slots': [{'species': 'SPECIES_RATTATA', 'lvl': 10}],
                },
            },
        }
        source = self.root / 'src/data/trainers.party'
        source.parent.mkdir(parents=True)
        source.write_text('TRAINER_TEST TRAINER_CLASS_YOUNGSTER\n')
        self.row = {
            'id': 'TRAINER_TEST',
            'products': ['wayfarer'],
            'sourceNamespace': 'emerald',
            'trainer': 'TRAINER_TEST',
            'scalingPolicy': 'ORDINARY',
            'review': {'sourceRefs': [
                {'path': 'src/data/trainers.party', 'symbol': 'TRAINER_TEST',
                 'role': 'TRAINER_CLASS_YOUNGSTER'},
            ]},
        }

    def write(self, rows):
        path = self.root / 'data/gameplay/shared.json'
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps({'schemaVersion': 1, 'services': [],
                                    'encounters': [], 'legacyTrainers': rows}))

    def test_reviewed_fallback_owns_unmigrated_roster_policy(self):
        self.write([self.row])
        result = compile_scaling_policies(self.root, 'wayfarer', self.trainers)
        self.assertEqual(result['policies'], {'TRAINER_TEST': 'ORDINARY'})
        self.assertEqual(result['legacyCoverage']['fallbackRows'], 1)
        self.assertEqual(result['legacyCoverage']['migratedTrainerRows'], 0)

    def test_complete_migrated_actor_can_remove_fallback(self):
        self.write([])
        result = compile_scaling_policies(
            self.root, 'wayfarer', self.trainers,
            [{'id': 'MAP_TEST/ordinary', 'trainer': 'TRAINER_TEST',
              'scalingPolicy': 'ORDINARY'}],
            discovered_callers=[{'id': 'MAP_TEST/ordinary', 'trainer': 'TRAINER_TEST'}])
        self.assertEqual(result['policies'], {'TRAINER_TEST': 'ORDINARY'})
        self.assertEqual(result['legacyCoverage']['fallbackRows'], 0)
        self.assertEqual(result['legacyCoverage']['migratedTrainerRows'], 1)

    def test_new_discovered_caller_invalidates_fallback_removal(self):
        self.write([])
        migrated = [{'id': 'MAP_TEST/ordinary', 'trainer': 'TRAINER_TEST',
                     'scalingPolicy': 'ORDINARY'}]
        discovered = [{'id': 'MAP_TEST/ordinary', 'trainer': 'TRAINER_TEST'},
                      {'id': 'MAP_TEST/newly_discovered', 'trainer': 'TRAINER_TEST'}]
        with self.assertRaisesRegex(ContentError, 'discovered caller lacks migrated declaration'):
            compile_scaling_policies(self.root, 'wayfarer', self.trainers,
                                     migrated, discovered_callers=discovered)

    def test_missing_fallback_and_migrated_caller_fails_coverage(self):
        self.write([])
        with self.assertRaisesRegex(ContentError, 'neither reviewed fallback nor migrated'):
            compile_scaling_policies(self.root, 'wayfarer', self.trainers)

    def test_migrated_callers_and_fallback_must_agree(self):
        self.write([self.row])
        with self.assertRaisesRegex(ContentError, 'reviewed fallback'):
            compile_scaling_policies(
                self.root, 'wayfarer', self.trainers,
                [{'id': 'MAP_TEST/gym', 'trainer': 'TRAINER_TEST',
                  'scalingPolicy': 'GYM_MEMBER'}])
        with self.assertRaisesRegex(ContentError, 'migrated caller scaling policies disagree'):
            compile_scaling_policies(
                self.root, 'wayfarer', self.trainers,
                [{'id': 'MAP_TEST/one', 'trainer': 'TRAINER_TEST', 'scalingPolicy': 'ORDINARY'},
                 {'id': 'MAP_TEST/two', 'trainer': 'TRAINER_TEST', 'scalingPolicy': 'GYM_MEMBER'}])

    def test_stale_review_and_wrong_source_namespace_fail(self):
        stale = copy.deepcopy(self.row)
        stale['review']['sourceRefs'][0]['symbol'] = 'TRAINER_OLD'
        self.write([stale])
        with self.assertRaisesRegex(ContentError, 'STALE_REVIEW'):
            load_legacy_trainers(self.root, 'wayfarer', self.trainers)
        wrong = copy.deepcopy(self.row)
        wrong['sourceNamespace'] = 'hns'
        self.write([wrong])
        with self.assertRaisesRegex(ContentError, 'CONFLICT'):
            load_legacy_trainers(self.root, 'wayfarer', self.trainers)

    def test_inactive_rows_still_receive_strict_schema_validation(self):
        inactive = copy.deepcopy(self.row)
        inactive['products'] = ['hns']
        inactive['sourceNamespace'] = 'not_a_namespace'
        inactive['review']['unexpected'] = True
        self.write([inactive])
        with self.assertRaisesRegex(ContentError, 'SCHEMA'):
            load_legacy_trainers(self.root, 'wayfarer', self.trainers)


if __name__ == '__main__':
    unittest.main()
