"""Independent Phase-D comparison against f4f602d0dd's ordinary registry."""
import hashlib
import json
from pathlib import Path
import unittest

from tools.gameplay_content.configuration import numeric_defines
from tools.gameplay_content.encounters import compile_encounters
from tools.gameplay_content.maps import load_maps
from tools.gameplay_content.trainers import load_trainers


ROOT = Path(__file__).resolve().parents[3]
FLAGS = '-DPOKEMON_WAYFARER -iquote build/gameplay-content/wayfarer/current'
BASELINE = Path(__file__).with_name('encounters_baseline.json')
SCENE_ROWS = {
    'GAMEPLAY_ENCOUNTER_ROUTE110_RIVAL_ROW': (
        'HOENN_ENTRY(Route110_EventScript_MayBattleTreecko + 1, '
        'Route110_EventScript_WayfarerRivalLossRetreat, Route110_EventScript_RivalScene, 2, '
        'WAYFARER_STORY_SCENE_ROUTE110_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, '
        'WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN | '
        'WAYFARER_STORY_FLAG_TRANSIENT_OBJECT | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_ROUTE110, '
        'WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL, 3, 3, 1, 33, 56, WayfarerHoennRoute110RivalPending),'),
    'GAMEPLAY_ENCOUNTER_RUSTURF_AQUA_ROW': (
        'HOENN_ENTRY(RusturfTunnel_EventScript_GruntTrainerBattle + 1, '
        'RusturfTunnel_EventScript_WayfarerGruntLossRetreat, NULL, 20, '
        'WAYFARER_STORY_SCENE_RUSTURF_AQUA, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, '
        'WAYFARER_STORY_DIALOGUE_AQUA_GUARD, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_RUSTURF_TUNNEL, '
        '0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, '
        'WAYFARER_STORY_NO_COORD, WayfarerHoennRusturfRescuePending),'),
}


class EncounterBaselineTests(unittest.TestCase):
    @unittest.skipUnless((ROOT / 'tools/mapjson/mapjson').exists(), 'build mapjson before migration test')
    def test_ordinary_policy_assignments_match_unrefactored_baseline(self):
        baseline = json.loads(BASELINE.read_text())['callers']
        maps = load_maps(ROOT, 'wayfarer')
        trainers = load_trainers(ROOT, 'wayfarer', 'cpp', FLAGS)
        result = compile_encounters(ROOT, 'wayfarer', numeric_defines(ROOT, 'cpp', FLAGS), maps, trainers,
                                    cpp='cpp', cppflags=FLAGS)
        ordinary = sorted((row for row in result['report']['encounters'] if row['profile']['family'] == 'ordinary'),
                          key=lambda row: row['caller']['label'])
        expected = [(row['caller'], row['source'], row['baseTrainer'], row['stableKey'], row['dialogue'],
                     'LOSS_RETURN' if row['lossReturn'] else 'LEGACY',
                     {'trainerbattle_single': 0, 'trainerbattle_double': 4,
                      'trainerbattle_rematch': 5, 'trainerbattle_rematch_double': 7}[row['command'].split()[0]])
                    + (row['reviewedCommandsSha256'],)
                    for row in baseline]
        actual = [(row['caller']['label'], row['caller']['source'], row['caller']['trainerId'],
                   row['profile']['stableKey'], row['profile']['dialogue'],
                   row['profile']['outcomePolicy'], row['caller']['basicMode'],
                   row['review']['commandFingerprint']) for row in ordinary]
        self.assertEqual(actual, expected)
        self.assertEqual(sum(row[5] == 'LOSS_RETURN' for row in actual), 787)

    def test_regional_runtime_tables_remain_the_frozen_baseline(self):
        johto = ROOT / 'src/data/wayfarer_story_encounter_johto.h'
        self.assertEqual(hashlib.sha256(johto.read_bytes()).hexdigest(),
                         '08f40c0d21a0e49ede7a5ec6a4cc8d57a42c95724ce4f0b90c3ee5f677dfcdae')
        hoenn_path = ROOT / 'src/data/wayfarer_story_encounter_hoenn.h'
        hoenn = hoenn_path.read_text()
        self.assertIn('#include "gameplay_encounter_scenes.inc"', hoenn)
        reconstructed = hoenn.replace('\n#include "gameplay_encounter_scenes.inc"\n\n', '\n')
        reconstructed = reconstructed.replace('#undef GAMEPLAY_ENCOUNTER_RUSTURF_AQUA_ROW\n', '')
        reconstructed = reconstructed.replace('#undef GAMEPLAY_ENCOUNTER_ROUTE110_RIVAL_ROW\n', '')
        for macro, row in SCENE_ROWS.items():
            self.assertIn(macro, hoenn)
            reconstructed = reconstructed.replace('    ' + macro + '\n', '    ' + row + '\n')
        self.assertEqual(hashlib.sha256(reconstructed.encode()).hexdigest(),
                         '3034578b8001e0ac3bf29124d9fcacec665224e43d2d94a8f2e7a417913e4deb')

    @unittest.skipUnless((ROOT / 'tools/mapjson/mapjson').exists(), 'build mapjson before migration test')
    def test_expanded_scene_projection_matches_unrefactored_rows(self):
        maps = load_maps(ROOT, 'wayfarer')
        trainers = load_trainers(ROOT, 'wayfarer', 'cpp', FLAGS)
        result = compile_encounters(ROOT, 'wayfarer', numeric_defines(ROOT, 'cpp', FLAGS), maps, trainers,
                                    cpp='cpp', cppflags=FLAGS)
        output = result['outputs']['gameplay_encounter_scenes.inc']
        for macro, row in SCENE_ROWS.items():
            with self.subTest(macro=macro):
                self.assertIn(f'#define {macro} \\\n    {row}', output)


if __name__ == '__main__':
    unittest.main()
