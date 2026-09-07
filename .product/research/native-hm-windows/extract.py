#!/usr/bin/env python3
"""Extract a research baseline; never modify game files.

Runs the C preprocessor on isolated learnset headers. Content switches are off
to remove the existing native-HM feature, including its successor reminder
entries, while restoring Chinchou's guarded Charge. Family/form switches are on
to inventory all species; actual encounter eligibility belongs to coverage.py.
"""
import json
import re
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
DATA = ROOT / 'game/src/data/pokemon'
UTILITIES = ['MOVE_' + move for move in
             ('CUT', 'FLASH', 'SURF', 'STRENGTH', 'ROCK_SMASH', 'WATERFALL', 'WHIRLPOOL', 'DIVE')]


def preprocess_learnsets(path):
    source = re.sub(r'^#define LEVEL_UP_.*$', '', path.read_text(), flags=re.M)
    directives = '\n'.join(line for line in source.splitlines() if line.startswith('#'))
    names = sorted(set(re.findall(r'\bP_[A-Z0-9_]+\b', directives)))
    prefix = '\n'.join(f'#define {name} 1' for name in names)
    prefix += '\n#define HAS_HNS_CONTENT 0\n#define HAS_EMERALD_CONTENT 0\n#define HAS_FRLG_CONTENT 0\n'
    result = subprocess.run(['cpp', '-P', '-undef', '-'], input=prefix + source,
                            text=True, capture_output=True, check=True)
    return {
        symbol: [[int(level), move] for level, move in
                 re.findall(r'LEVEL_UP_MOVE\(\s*(\d+)\s*,\s*(MOVE_\w+)\s*\)', body)]
        for symbol, body in re.findall(
            r'static const struct LevelUpMove (\w+)\[\]\s*=\s*\{(.*?)\};',
            result.stdout, re.S)
    }


def extract():
    modern = preprocess_learnsets(DATA / 'level_up_learnsets/gen_7.h')
    legacy = preprocess_learnsets(DATA / 'level_up_learnsets/gen_3.h')
    legacy_map = dict(re.findall(r'\[(SPECIES_\w+)\]\s*=\s*(s\w+LevelUpLearnset)',
                                (DATA / 'level_up_learnsets_gen3.c').read_text()))
    modern_map = {}
    teachable_map = {}
    for path in sorted((DATA / 'species_info').glob('gen_*_families.h')):
        chunks = re.split(r'\[(SPECIES_\w+)\]\s*=', path.read_text())
        for species, body in zip(chunks[1::2], chunks[2::2]):
            match = re.search(r'\.levelUpLearnset\s*=\s*(s\w+LevelUpLearnset)', body)
            if match:
                modern_map[species] = match.group(1)
            match = re.search(r'\.teachableLearnset\s*=\s*s(\w+)TeachableLearnset', body)
            if match:
                teachable_map[species] = re.sub(r'(?!^)([A-Z]+)', r'_\1', match.group(1)).upper()
    all_learnables = json.loads((DATA / 'all_learnables.json').read_text())
    records = {}
    for species in sorted(set(legacy_map) | set(modern_map)):
        # Shared form macros occasionally hide the pointer; the explicit Gen3
        # table uses the same symbol for those forms in both header families.
        msymbol = modern_map.get(species, legacy_map.get(species))
        lsymbol = legacy_map.get(species)
        if msymbol not in modern:
            continue
        learns = {'modern': modern[msymbol],
                  'legacy': legacy.get(lsymbol, modern[msymbol])}
        key = teachable_map.get(species, species.removeprefix('SPECIES_'))
        learnables = all_learnables.get(key, [])
        native = {mode: sorted({move for _, move in entries if move in UTILITIES})
                  for mode, entries in learns.items()}
        duplicates = {mode: {move: [level for level, m in entries if m == move]
                             for move in native[mode]
                             if sum(m == move for _, m in entries) > 1}
                      for mode, entries in learns.items()}
        records[species] = {
            **learns,
            'compatible': sorted(set(learnables).intersection(UTILITIES)
                                 | set(native['modern']) | set(native['legacy'])),
            'natural_utilities': native,
            'natural_utility_duplicates': duplicates,
            'modern_symbol': msymbol, 'legacy_symbol': lsymbol,
            'modern_pointer_inferred': species not in modern_map,
            'compatibility_key': key,
            'compatibility_source_present': key in all_learnables,
        }
    assert [50, 'MOVE_CHARGE'] in records['SPECIES_CHINCHOU']['modern']
    assert records['SPECIES_CHINCHOU']['natural_utilities']['modern'] == []
    assert [41, 'MOVE_DIVE'] in records['SPECIES_WAILMER']['modern']
    for anchor in ('AIPOM', 'CORPHISH', 'MAREEP', 'WOOPER', 'ELECTRIKE', 'ARON'):
        assert not records['SPECIES_' + anchor]['natural_utilities']['modern'], anchor
    return {
        'metadata': {
            'purpose': 'Original learnsets before native HM injections; candidate design input, not ROM output.',
            'modern_generation': 7, 'legacy_generation': 3,
            'content_switches': 'All HAS_*_CONTENT=0 restores baseline; all family/form switches=1 inventories candidates.',
            'compatibility': 'all_learnables.json any supported game/method plus original level-up utility moves; not a promise of active TM availability.',
            'species_mapping': 'Explicit species_info modern pointers; Gen3 table fallback for shared form macros (flagged).',
            'encounters': 'Use coverage.py; inventory itself includes unencountered and disabled species/forms.',
            'order': 'Learnset source order preserved, including original duplicate moves and level 0 evolution entries.',
        },
        'species': records,
    }


if __name__ == '__main__':
    baseline = extract()
    (HERE / 'baseline.json').write_text(json.dumps(baseline, indent=2) + '\n')
    print(f'Extracted {len(baseline["species"])} species into {HERE / "baseline.json"}')
