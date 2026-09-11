import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from tools.gameplay_content.common import ContentError
from tools.gameplay_content.encounters import MODES, _discover_battle_callers, command_fingerprint, compile_encounters


class EncounterCompilerTests(unittest.TestCase):
    def _compile(self, root, product='wayfarer', trainers=None, compiled_modes=None):
        source = root / 'data/scripts/test.inc'
        blocks, label, body = {}, None, []
        compiled_modes = compiled_modes or {}
        def record(name, lines):
            command = next(line.strip() for line in lines if line.strip().startswith('trainerbattle'))
            macro = command.split(None, 1)[0]
            return {'path': 'data/scripts/test.inc', 'line': 1, 'body': '\n'.join(lines),
                    'resolvedBody': '\n'.join(lines), 'assemblerConditional': False,
                    'compiledBattle': {'mode': compiled_modes.get(name, MODES.get(macro, 0)), 'trainer': 0}}
        for line in source.read_text().splitlines():
            if line.endswith('::'):
                if label is not None:
                    blocks[label] = [record(label, body)]
                label, body = line[:-2], []
            else:
                body.append(line)
        blocks[label] = [record(label, body)]
        with patch('tools.gameplay_content.encounters.load_script_blocks', return_value=blocks):
            return compile_encounters(root, product, {}, [], trainers or {'TRAINER_TEST': {}})

    def _root(self, command='trainerbattle_single TRAINER_TEST, Intro, Defeat', product='wayfarer'):
        temporary = tempfile.TemporaryDirectory()
        root = Path(temporary.name)
        source = root / 'data/scripts/test.inc'
        source.parent.mkdir(parents=True)
        source.write_text('TestBattle::\n\t' + command + '\n\tend\n')
        body = '\t' + command + '\n\tend'
        declaration = {
            'schemaVersion': 1, 'services': [], 'legacyTrainers': [], 'encounters': [{
                'id': 'test_battle', 'products': [product],
                'caller': {'label': 'TestBattle', 'contextIndependent': True},
                'review': {'source': 'data/scripts/test.inc', 'commandFingerprint': command_fingerprint(body)},
                'profile': {'family': 'ordinary', 'scalingPolicy': 'ORDINARY', 'outcomePolicy': 'LOSS_RETURN',
                            'entry': 'trainerbattle', 'refusal': 'WayfarerStoryTryStartTrainerBattle',
                            'success': 'authored', 'nonVictory': 'EventScript_WayfarerStoryLossRetreat',
                            'dialogue': 'ORDINARY', 'stableKey': 7},
            }]}
        shared = root / 'data/gameplay/shared.json'
        shared.parent.mkdir(parents=True)
        shared.write_text(json.dumps(declaration))
        return temporary, root

    def test_compiles_actual_command_and_compact_registry(self):
        temporary, root = self._root()
        with temporary:
            result = self._compile(root)
        row = result['report']['encounters'][0]
        self.assertEqual(row['caller']['trainer'], 'TRAINER_TEST')
        self.assertEqual(row['caller']['mode'], 0)
        self.assertEqual(row['caller']['argumentOffset'], 1)
        self.assertIn('ORDINARY_ENTRY(TestBattle, 7, ORDINARY', result['outputs']['gameplay_encounters.inc'])

    def test_compiled_continuation_mode_overrides_the_basic_macro_category(self):
        temporary, root = self._root()
        with temporary:
            result = self._compile(root, compiled_modes={'TestBattle': 2})
        caller = result['report']['encounters'][0]['caller']
        self.assertEqual(caller['basicMode'], 0)
        self.assertEqual(caller['mode'], 2)

    def test_compiled_mode_outside_the_closed_macro_family_is_rejected(self):
        temporary, root = self._root()
        with temporary:
            with self.assertRaisesRegex(ContentError, 'compiled trainerbattle mode'):
                self._compile(root, compiled_modes={'TestBattle': 4})

    def test_changed_command_invalidates_review_before_authorization(self):
        temporary, root = self._root()
        with temporary:
            (root / 'data/scripts/test.inc').write_text(
                'TestBattle::\n\ttrainerbattle_double TRAINER_TEST, Intro, Defeat, NoParty, Return\n\tend\n')
            with self.assertRaisesRegex(ContentError, 'STALE_REVIEW'):
                self._compile(root)

    def test_unsupported_command_is_never_authorized(self):
        temporary, root = self._root('trainerbattle_continue_script TRAINER_TEST, Intro')
        with temporary:
            shared = root / 'data/gameplay/shared.json'
            document = json.loads(shared.read_text())
            document['encounters'][0]['review']['commandFingerprint'] = command_fingerprint('\ttrainerbattle_continue_script TRAINER_TEST, Intro\n\tend')
            shared.write_text(json.dumps(document))
            with self.assertRaisesRegex(ContentError, 'UNSUPPORTED_CONTEXT'):
                self._compile(root)

    def test_loss_return_cannot_name_an_unemitted_continuation(self):
        temporary, root = self._root()
        with temporary:
            shared = root / 'data/gameplay/shared.json'
            document = json.loads(shared.read_text())
            document['encounters'][0]['profile']['nonVictory'] = 'ArbitraryRetreat'
            shared.write_text(json.dumps(document))
            with self.assertRaisesRegex(ContentError, 'ordinary outcome continuation'):
                self._compile(root)

    def test_discovered_unreviewed_caller_stays_out_of_runtime_output(self):
        temporary, root = self._root()
        with temporary:
            source = root / 'data/scripts/test.inc'
            source.write_text(source.read_text() + '\nUnreviewedBattle::\n\ttrainerbattle_single TRAINER_TEST, Intro, Defeat\n\tend\n')
            result = self._compile(root)
        self.assertEqual([row['label'] for row in result['report']['unreviewedBattleCallers']], ['UnreviewedBattle'])
        self.assertNotIn('UnreviewedBattle', result['outputs']['gameplay_encounters.inc'])

    def test_removing_an_authored_ordinary_row_removes_its_compact_entry(self):
        temporary, root = self._root()
        with temporary:
            self.assertIn('ORDINARY_ENTRY(TestBattle, 7, ORDINARY',
                          self._compile(root)['outputs']['gameplay_encounters.inc'])
            shared = root / 'data/gameplay/shared.json'
            document = json.loads(shared.read_text())
            document['encounters'] = []
            shared.write_text(json.dumps(document))
            result = self._compile(root)
        self.assertNotIn('ORDINARY_ENTRY(TestBattle, 7, ORDINARY',
                         result['outputs']['gameplay_encounters.inc'])

    def test_deferred_scene_rejects_a_non_numeric_activation_rectangle(self):
        temporary, root = self._root()
        with temporary:
            shared = root / 'data/gameplay/shared.json'
            document = json.loads(shared.read_text())
            profile = document['encounters'][0]['profile']
            profile.update({'family': 'deferred_rival', 'scalingPolicy': 'EXCLUDED',
                            'nonVictory': 'Route110_EventScript_WayfarerRivalLossRetreat', 'dialogue': 'RIVAL',
                            'scene': {'sceneId': 'WAYFARER_STORY_SCENE_ROUTE110_RIVAL',
                                      'trigger': 'Route110_EventScript_RivalScene',
                                      'objectLifecycle': 'TRANSIENT_OBJECT', 'rearmRegion': 'MAP_ROUTE110',
                                      'eligibilityPredicate': 'WayfarerHoennRoute110RivalPending',
                                      'object': {'localId': 'LOCALID_ROUTE110_RIVAL', 'elevation': 3},
                                      'activationRect': {'x': '33', 'y': 56, 'width': 3, 'height': 1}}})
            shared.write_text(json.dumps(document))
            with self.assertRaisesRegex(ContentError, 'scene.activationRect'):
                self._compile(root)

    def test_standalone_product_keeps_discovery_without_wayfarer_scaling(self):
        temporary, root = self._root(product='hns')
        with temporary:
            result = self._compile(root, product='hns', trainers={'TRAINER_TEST': {'default': {'slots': [1]}}})
        self.assertEqual(result['report']['ordinaryEncounterCount'], 1)
        self.assertEqual(len(result['report']['discoveredBattleCallers']), 1)
        self.assertNotIn('scaling', result['report'])

    def test_raw_trainerbattle_extracts_both_opponents_by_signature(self):
        command = ('trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, LOCALID_A, TRAINER_ALPHA, Intro, Defeat, '
                   'Continue, LOCALID_B, TRAINER_BETA, NULL, NULL, NULL, NULL, NULL, FALSE, TRUE, FALSE, FALSE')
        blocks = {'RawBattle': [{'path': 'data/scripts/test.inc', 'line': 1, 'body': command,
                                 'resolvedBody': command, 'assemblerConditional': False}]}
        row = _discover_battle_callers(blocks)[0]
        self.assertEqual(row['trainerIds'], ['TRAINER_ALPHA', 'TRAINER_BETA'])
        self.assertEqual(row['unresolvedTrainerOperands'], [])

    def test_unknown_discovered_trainer_cannot_bypass_policy_coverage(self):
        temporary, root = self._root()
        with temporary:
            source = root / 'data/scripts/test.inc'
            source.write_text(source.read_text() + '\nDynamicBattle::\n\ttrainerbattle_single TRAINER_UNKNOWN, Intro, Defeat\n\tend\n')
            with self.assertRaisesRegex(ContentError, 'discovered callers'):
                self._compile(root)

    def test_standalone_foreign_roster_caller_stays_report_only(self):
        temporary, root = self._root(product='hns')
        with temporary:
            source = root / 'data/scripts/test.inc'
            source.write_text(source.read_text() + '\nForeignBattle::\n\ttrainerbattle_single TRAINER_FOREIGN, Intro, Defeat\n\tend\n')
            result = self._compile(root, product='hns', trainers={'TRAINER_TEST': {'default': {'slots': [1]}}})
        self.assertEqual(result['report']['unselectedTrainerIds'], ['TRAINER_FOREIGN'])
        self.assertEqual(result['report']['unreviewedBattleCallers'][0]['label'], 'ForeignBattle')

    def test_duplicate_active_caller_binding_is_rejected(self):
        temporary, root = self._root()
        with temporary:
            shared = root / 'data/gameplay/shared.json'
            document = json.loads(shared.read_text())
            duplicate = json.loads(json.dumps(document['encounters'][0]))
            duplicate['id'] = 'same_caller_other_id'
            document['encounters'].append(duplicate)
            shared.write_text(json.dumps(document))
            with self.assertRaisesRegex(ContentError, 'active caller already bound'):
                self._compile(root)

    def test_base_encounter_cannot_change_the_frozen_dialogue(self):
        temporary, root = self._root()
        with temporary:
            source = root / 'data/scripts/test.inc'
            source.write_text('BaseBattle::\n\ttrainerbattle_single TRAINER_TEST, Intro, Defeat\n\tend\n'
                              'RematchBattle::\n\ttrainerbattle_rematch TRAINER_TEST, Intro, Defeat\n\tend\n')
            shared = root / 'data/gameplay/shared.json'
            document = json.loads(shared.read_text())
            base = document['encounters'][0]
            base.update({'id': 'base', 'caller': {'label': 'BaseBattle', 'contextIndependent': True},
                         'review': {'source': 'data/scripts/test.inc',
                                    'commandFingerprint': command_fingerprint('\ttrainerbattle_single TRAINER_TEST, Intro, Defeat\n\tend')}})
            rematch = json.loads(json.dumps(base))
            rematch.update({'id': 'rematch', 'baseEncounter': 'base',
                            'caller': {'label': 'RematchBattle', 'contextIndependent': True},
                            'review': {'source': 'data/scripts/test.inc',
                                       'commandFingerprint': command_fingerprint('\ttrainerbattle_rematch TRAINER_TEST, Intro, Defeat\n\tend')}})
            rematch['profile']['dialogue'] = 'ROCKET_GUARD'
            document['encounters'] = [base, rematch]
            shared.write_text(json.dumps(document))
            with self.assertRaisesRegex(ContentError, 'rematch'):
                self._compile(root)


if __name__ == '__main__':
    unittest.main()
