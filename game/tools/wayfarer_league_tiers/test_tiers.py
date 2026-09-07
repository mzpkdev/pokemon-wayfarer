import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import tempfile
import unittest


GAME = Path(__file__).resolve().parents[2]
INVENTORY = json.loads(Path(__file__).with_name('inventory.json').read_text())['trainers']


def parties(wayfarer):
    source = '\n'.join((GAME / 'src/data' / name).read_text() for name in
                       ('trainers_hns.party', 'trainers.party'))
    processed = subprocess.run(
        ['cpp', '-P', '-traditional-cpp', '-DPOKEMON_WAYFARER' if wayfarer else '-DEMERALD', '-'],
        input=source, text=True, capture_output=True, check=True,
    ).stdout
    return dict(re.findall(r'=== (TRAINER_\w+) ===\n(.*?)(?=\n===|\Z)', processed, re.S))


class LeagueTiersTest(unittest.TestCase):
    @unittest.skipUnless((GAME / 'tools/trainerproc/trainerproc').is_file(), 'trainerproc must be built')
    def test_trainerproc_accepts_and_emits_both_product_level_sets(self):
        source = (GAME / 'src/data/trainers.party').read_text()
        with tempfile.TemporaryDirectory() as directory:
            for wayfarer in (False, True):
                processed = subprocess.run(
                    ['cpp', '-traditional-cpp', '-DPOKEMON_WAYFARER' if wayfarer else '-DEMERALD', '-'],
                    input=source, text=True, capture_output=True, check=True,
                ).stdout
                output = Path(directory) / 'trainers.h'
                subprocess.run(
                    [str(GAME / 'tools/trainerproc/trainerproc'), '-o', str(output), '-i', 'trainers.party', '-'],
                    input=processed, text=True, capture_output=True, check=True,
                )
                generated = output.read_text()
                for row in INVENTORY:
                    if row['tier'] != 3:
                        continue
                    body = re.search(r'\[DIFFICULTY_NORMAL\]\[' + row['trainer'] +
                                     r'\] =(.*?)(?=\[DIFFICULTY|\Z)', generated, re.S)[1]
                    expected = row['levels'] if wayfarer else row['emeraldLevels']
                    self.assertEqual(list(map(int, re.findall(r'\.lvl = (\d+)', body))), expected)

    def test_rom_fixture_selects_exact_production_records_without_mock_ids(self):
        source = subprocess.run(
            ['python3', str(Path(__file__).with_name('generate_fixture.py'))],
            text=True, capture_output=True, check=True,
        ).stdout
        processed = subprocess.run(
            ['cpp', '-P', '-traditional-cpp', '-DPOKEMON_WAYFARER', '-'],
            input=source, text=True, capture_output=True, check=True,
        ).stdout
        selected = dict(re.findall(r'=== (TRAINER_\w+) ===\n(.*?)(?=\n===|\Z)', processed, re.S))
        self.assertEqual(set(selected), {row['trainer'] for row in INVENTORY})
        production = parties(True)
        for trainer, body in selected.items():
            self.assertEqual(body.strip(), production[trainer].strip())

    def test_production_preprocessing_selects_fixed_tiers_and_preserves_rosters(self):
        for wayfarer in (False, True):
            generated = parties(wayfarer)
            for row in INVENTORY:
                with self.subTest(wayfarer=wayfarer, trainer=row['trainer']):
                    body = generated[row['trainer']]
                    levels = list(map(int, re.findall(r'^Level: (\d+)$', body, re.M)))
                    expected = row['levels'] if wayfarer else row.get('emeraldLevels', row['levels'])
                    self.assertEqual(levels, expected)
                    roster = re.sub(r'^Level: \d+\n', '', body, flags=re.M).strip()
                    # Preprocessor conditionals leave blank lines, which are not party fields.
                    roster = '\n'.join(line for line in roster.splitlines() if line.strip())
                    self.assertEqual(hashlib.sha256(roster.encode()).hexdigest(), row['rosterSha256'])
                    self.assertNotRegex(body, r'(?m)^Difficulty:', 'League parties must use the fixed Normal row')

    def test_tiers_increase_across_the_complete_sequence(self):
        ranges = {tier: [level for row in INVENTORY if row['tier'] == tier
                         for level in row['levels']] for tier in (1, 2, 3)}
        self.assertLess(max(ranges[1]), min(ranges[2]))
        self.assertLess(max(ranges[2]), min(ranges[3]))
        self.assertEqual(max(ranges[3]), 92)

    def test_all_existing_ids_remain_unchanged(self):
        hns = (GAME / 'include/constants/opponents_hns.h').read_text()
        emerald = (GAME / 'include/constants/opponents.h').read_text()
        offset = int(re.search(r'#define WAYFARER_HOENN_TRAINER_OFFSET\s+(\d+)', emerald)[1])
        for row in INVENTORY:
            if row['tier'] == 3:
                source = re.search(r'#define '+row['trainer']+r'\s+TRAINER_EMERALD_ID\((\d+)\)', emerald)
                actual = offset + int(source[1])
            else:
                actual = int(re.search(r'#define '+row['trainer']+r'\s+(\d+)', hns)[1])
            self.assertEqual(actual, row['id'])

    def test_content_audit_exception_is_only_league_levels(self):
        spec = importlib.util.spec_from_file_location('hoenn_content', GAME / 'tools/wayfarer_hoenn_content/generate.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        override = 'Level: WAYFARER_LEAGUE_LEVEL(86, 46)'
        self.assertEqual(module.emerald_trainer_party_source('TRAINER_SIDNEY', override), 'Level: 46')
        self.assertEqual(module.emerald_trainer_party_source('TRAINER_ROXANNE_1', override), override)
        other = '#if IS_WAYFARER\n- Surf\n#else\n- Water Gun\n#endif'
        self.assertEqual(module.emerald_trainer_party_source('TRAINER_SIDNEY', other), other)


if __name__ == '__main__':
    unittest.main()
