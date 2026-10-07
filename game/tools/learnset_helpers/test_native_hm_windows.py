"""Check authored HM windows against the wild-encounters-v2 roster and standalone tables.

Run from the repository root:
    python3 -m unittest discover -s game/tools/learnset_helpers -p test_native_hm_windows.py
Production initial-moveset behavior is covered by game/test/native_hm_catch_windows.c.
"""

import hashlib
import importlib.util
import os
import json
import re
import subprocess
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
EVIDENCE = ROOT / '.product/research/native-hm-windows'
REVISION = EVIDENCE / 'revisions/nearby-access'
V2_REVISION = EVIDENCE / 'revisions/wild-encounters-v2'
BASE_COMMIT = '479b0c83aea4ad90feb0af649e83ccb1a5916770'
SOURCES = {
    'modern': 'game/src/data/pokemon/level_up_learnsets/gen_7.h',
}
UTILITIES = {'MOVE_' + move for move in (
    'CUT', 'FLASH', 'SURF', 'STRENGTH', 'ROCK_SMASH', 'WATERFALL', 'WHIRLPOOL', 'DIVE')}
UPSTREAM_DELTA = json.loads(Path(__file__).with_name('native_hm_upstream_delta.json').read_text())


def with_upstream_moves(entries, mode, symbol, wayfarer=True):
    """Apply the pinned upstream move changes to the historical HM design input."""
    result = [entry[:] for entry in entries]
    changes = UPSTREAM_DELTA.get(mode, {}).get(symbol, {})
    for entry in changes.get('removed', []):
        result.remove(entry)
    exclusions = (UPSTREAM_DELTA.get('wayfarer_exclusions', {}).get(mode, {}).get(symbol, {})
                  if wayfarer else {})
    for entry in changes.get('added', []):
        if entry in exclusions.get('added', []):
            continue
        index = next((i for i, existing in enumerate(result) if existing[0] > entry[0]),
                     len(result))
        result.insert(index, entry[:])
    return result


def preprocess(source, wayfarer=False, content=None):
    source = re.sub(r'^#define LEVEL_UP_.*$', '', source, flags=re.M)
    directives = '\n'.join(line for line in source.splitlines() if line.startswith('#'))
    names = sorted(set(re.findall(r'\bP_[A-Z0-9_]+\b', directives)))
    prefix = '\n'.join(f'#define {name} 1' for name in names) + '\n'
    prefix += f'#define IS_WAYFARER {int(wayfarer)}\n'
    for name in ('HNS', 'EMERALD', 'FRLG'):
        prefix += f'#define HAS_{name}_CONTENT {int(wayfarer or content == name)}\n'
    output = subprocess.run(['cpp', '-P', '-undef', '-'], input=prefix + source,
                            text=True, capture_output=True, check=True).stdout
    return {
        symbol: [[int(level), move] for level, move in
                 re.findall(r'LEVEL_UP_MOVE\(\s*(\d+)\s*,\s*(MOVE_\w+)\s*\)', body)]
        for symbol, body in re.findall(
            r'static const struct LevelUpMove (\w+)\[\]\s*=\s*\{(.*?)\};',
            output, re.S)
    }


def species_symbol(species, baseline):
    """Level-up learnset symbol; the baseline lacks species the v2 roster adds."""
    if species in baseline:
        return baseline[species]['modern_symbol']
    for path in sorted((ROOT / 'game/src/data/pokemon/species_info').glob('gen_*_families.h')):
        current = None
        for line in path.read_text().splitlines():
            match = re.match(r'\s*\[(SPECIES_\w+)\]\s*=', line)
            if match:
                current = match[1]
            match = re.search(r'\.levelUpLearnset = (s\w+)', line)
            if match and current and (current == species or current.startswith(species + '_')):
                return match[1]
    raise KeyError(species)


class NativeHmWindowsDataTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.baseline = json.loads((EVIDENCE / 'baseline.json').read_text())['species']
        cls.proposal = json.loads((REVISION / 'proposal.json').read_text())
        cls.roster_doc = json.loads((V2_REVISION / 'roster_v2.json').read_text())
        cls.roster = cls.roster_doc['roster']
        for row in cls.roster:
            row['symbol'] = species_symbol(row['species'], cls.baseline)
        # Assignments: the nearby-access proposal with the v2 roster changes applied.
        cls.assignments = {}
        for move, species in cls.proposal['moves'].items():
            for name, levels in species.items():
                cls.assignments.setdefault('SPECIES_' + name, {})['MOVE_' + move] = levels
        for row in cls.roster:
            if row['change']:
                cls.assignments.setdefault(row['species'], {})[row['change']['move']] = \
                    row['change']['level']
        cls.sources = {mode: (ROOT / path).read_text() for mode, path in SOURCES.items()}
        cls.production = {mode: preprocess(source, wayfarer=True)
                          for mode, source in cls.sources.items()}

    def test_approved_inventory_and_provenance(self):
        results = json.loads((REVISION / 'results.json').read_text())
        digest = hashlib.sha256((REVISION / 'proposal.json').read_bytes()).hexdigest()
        self.assertEqual(digest, results['distributions']['revised']['proposal_sha256'])
        self.assertEqual(self.roster_doc['revision'], 'wild-encounters-v2')
        self.assertEqual(len(self.roster), self.roster_doc['species_count'])
        self.assertEqual(len(self.roster), 130)
        self.assertEqual(sum(len(row['roles']) for row in self.roster), 166)
        self.assertEqual(sum(len(row['roles']) for row in self.roster),
                         self.roster_doc['role_count'])
        self.assertEqual(sum(len(row['added']) for row in self.roster), 149)
        self.assertEqual(sum(bool(row['change']) for row in self.roster), 14)

    def test_upstream_delta_preserves_utility_assignments(self):
        for symbol, changes in UPSTREAM_DELTA['modern'].items():
            for entry in changes['removed'] + changes['added']:
                self.assertNotIn(entry[1], UTILITIES, (symbol, entry))

    def test_complete_ordered_roster_and_native_preservation(self):
        assignments = self.assignments
        self.assertEqual(set(assignments), {row['species'] for row in self.roster})
        native = {mode: preprocess(source) for mode, source in self.sources.items()}
        for row in self.roster:
            species = row['species']
            symbol = row['symbol']
            roles = set(assignments[species])
            for mode in SOURCES:
                # Species the baseline lacks (Wishiwashi) take their native entries from source.
                original = (self.baseline[species][mode] if species in self.baseline
                            else native[mode][symbol])
                roles.update(move for _, move in original if move in UTILITIES)
                additions = []
                for move, level in assignments[species].items():
                    self.assertIn(move, UTILITIES)
                    if not any(native_move == move for _, native_move in original):
                        level = level[mode] if isinstance(level, dict) else level
                        self.assertTrue(1 <= level <= 100)
                        additions.append([level, move])
                expected = sorted(original + additions, key=lambda pair: pair[0])
                with self.subTest(species=species, mode=mode):
                    self.assertEqual(row['entries'], expected)
                    self.assertEqual(row['added'], {move: level for level, move in additions})
                    actual = self.production[mode][symbol]
                    self.assertEqual(actual, with_upstream_moves(expected, mode, symbol))
                    for _, move in additions:
                        self.assertEqual(sum(m == move for _, m in actual), 1)
            self.assertLessEqual(len(roles), 2, species)
            # Rows list every utility, except species new to the roster list only their additions.
            self.assertTrue(set(row['roles']) <= roles, species)
            self.assertEqual(roles, {move for _, move in self.production['modern'][symbol]
                                     if move in UTILITIES}, species)

    def test_both_configured_table_limits(self):
        limits = []
        for file, constant in [('pokemon.h', 'MAX_LEVEL_UP_MOVES'),
                               ('move_relearner.h', 'MAX_RELEARNER_MOVES')]:
            source = (ROOT / 'game/include/constants' / file).read_text()
            limits.append(int(re.search(r'#define\s+' + constant + r'\s+(\d+)', source)[1]))
        for row in self.roster:
            for mode in SOURCES:
                entries = self.production[mode][row['symbol']]
                for limit in limits:
                    self.assertLessEqual(len(entries), limit, (row['species'], mode))

    def test_unselected_tables_only_lose_old_feature_injections(self):
        for mode, source in self.sources.items():
            native = preprocess(source)
            selected = {row['symbol'] for row in self.roster}
            for symbol, entries in self.production[mode].items():
                if symbol not in selected:
                    self.assertEqual(entries, native[symbol], (mode, symbol))

    def test_standalone_schedules_equal_source_baseline(self):
        fixture = json.loads(Path(__file__).with_name('native_hm_standalone_baseline.json').read_text())
        self.assertEqual(fixture['base_commit'], BASE_COMMIT)
        for mode, path in SOURCES.items():
            record = fixture['modes'][mode]
            self.assertEqual(record['source'], path)
            for content in ('EMERALD', 'FRLG', 'HNS', None):
                with self.subTest(mode=mode, standalone=content):
                    tables = preprocess(self.sources[mode], content=content)
                    canonical = json.dumps(tables, sort_keys=True, separators=(',', ':')).encode()
                    self.assertEqual(hashlib.sha256(canonical).hexdigest(),
                                     UPSTREAM_DELTA['modern_standalone_sha256'][content or 'NONE'])

    def test_evolution_paths_use_at_most_two_utility_types(self):
        edges = {}
        for path in (ROOT / 'game/src/data/pokemon/species_info').glob('gen_*_families.h'):
            chunks = re.split(r'\[(SPECIES_\w+)\]\s*=', path.read_text())
            for species, body in zip(chunks[1::2], chunks[2::2]):
                if '.evolutions' not in body:
                    continue
                section = body.split('.evolutions', 1)[1]
                section = re.split(r'\n\s*\.\w+\s*=', section)[0]
                edges.setdefault(species, set()).update(re.findall(
                    r'\{EVO_\w+\s*,[^,]+,\s*(SPECIES_\w+)', section))
        selected = {row['species'] for row in self.roster}

        symbols = {row['species']: row['symbol'] for row in self.roster}

        def roles(species):
            record = self.baseline.get(species)
            if record is None and species not in symbols:
                return set()
            symbol = symbols.get(species) or record['modern_symbol']
            entries = self.production['modern'].get(symbol, [])
            return {move for _, move in entries if move in UTILITIES}

        def visit(path, utilities):
            if selected.intersection(path):
                self.assertLessEqual(len(utilities), 2, (path, utilities))
            for target in edges.get(path[-1], ()):
                self.assertNotIn(target, path)
                visit(path + [target], utilities | roles(target))

        for species in set(edges) | selected:
            visit([species], roles(species))

    def test_generated_hm_fallback_supports_every_assignment(self):
        def load(name):
            spec = importlib.util.spec_from_file_location(name, Path(__file__).with_name(name + '.py'))
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            return module

        generator = load('make_teachables')
        types = load('make_teaching_types')
        previous = Path.cwd()
        try:
            os.chdir(ROOT / 'game')
            tms = list(generator.extract_repo_tms('POKEMON_WAYFARER'))
            learnables = json.loads(Path('src/data/pokemon/all_learnables.json').read_text())
            special = json.loads(Path('src/data/pokemon/special_movesets.json').read_text())
            # HM entries come from the registry; tutors cannot establish HM fallback.
            header = generator.prepare_output(learnables, tms, [], special,
                                              types.extract_repo_species_data(), '')
        finally:
            os.chdir(previous)
        teachables = {
            generator.SNAKIFY_PAT.sub(r'_\1', symbol).upper(): set(re.findall(r'MOVE_\w+', body))
            for symbol, body in re.findall(
                r'static const u16 s(\w+)TeachableLearnset\[\]\s*=\s*\{(.*?)\};', header, re.S)
        }
        for species, moves in self.assignments.items():
            record = self.baseline.get(species)
            # Species the baseline lacks (Wishiwashi) key their family by name.
            key = record['compatibility_key'] if record else species.removeprefix('SPECIES_')
            for move in moves:
                self.assertIn(move, tms)
                self.assertIn(move, teachables[key], (species, move))

    def test_native_snapshot_has_not_drifted(self):
        for mode, source in self.sources.items():
            native = preprocess(source)
            for species, record in self.baseline.items():
                symbol = record[mode + '_symbol']
                if symbol in native:
                    self.assertEqual(native[symbol], with_upstream_moves(
                        record[mode], mode, symbol, wayfarer=False), (mode, species))


if __name__ == '__main__':
    unittest.main()
