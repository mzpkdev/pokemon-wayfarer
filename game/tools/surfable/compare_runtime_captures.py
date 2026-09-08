#!/usr/bin/env python3
"""Compare separate baseline/optimized Surf smoke capture directories."""
import argparse
import hashlib
import json
from pathlib import Path

SPECIES = ('squirtle', 'raichuAlola', 'wartortle', 'qwilfishHisui', 'lugia', 'gyarados')
STAGES = ('entry', 'up', 'left', 'right', 'down', 'rod-bag', 'fishing', 'fishing-ended',
          'dismount', 'remount', 'before-transition', 'after-transition', 'bob-0', 'bob-8', 'bob-16')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def compare(root):
    pairs = []
    expected = {f'{species}-{variant}-{stage}.png' for species in SPECIES
                for variant in ('normal', 'shiny') for stage in STAGES}
    for build in ('baseline', 'optimized'):
        directory = root / f'runtime-final-{build}'
        require({p.name for p in directory.glob('*.png')} == expected,
                f'{directory}: missing, stale, or unexpected captures')
    for species in SPECIES:
        for variant in ('normal', 'shiny'):
            label = f'{species}-{variant}'
            for build in ('baseline', 'optimized'):
                path = root / f'runtime-final-{build}' / f'{label}.json'
                result = json.loads(path.read_text())
                require(not {'fishing', 'map-transition'} & set(result['pending']), f'{path}: incomplete full smoke case')
                state = result['state']
                require(state['map']['name'] == 'route-41' and state['player']['surfing'], f'{path}: transition not completed while mounted')
                require(state['fieldMove']['userSpecies'] == species and state['fieldMove']['user'] == 0,
                        f'{path}: wrong Surf user')
            for stage in STAGES:
                filename = f'{label}-{stage}.png'
                baseline = root / 'runtime-final-baseline' / filename
                optimized = root / 'runtime-final-optimized' / filename
                a, b = baseline.read_bytes(), optimized.read_bytes()
                require(a.startswith(b'\x89PNG\r\n\x1a\n') and a == b, f'{filename}: baseline/optimized capture mismatch')
                pairs.append({'capture': filename, 'sha256': hashlib.sha256(a).hexdigest()})
        a = root / 'runtime-final-optimized' / f'{species}-normal-left.png'
        b = root / 'runtime-final-optimized' / f'{species}-shiny-left.png'
        require(a.read_bytes() != b.read_bytes(), f'{species}: normal/shiny captures unexpectedly identical')
    return {'representative_variants_per_rom': 12, 'identical_required_capture_pairs': len(pairs),
            'note': 'PNG equality proves baseline parity at captured frames; silhouette, layering and bobbing also need visual review.',
            'captures': pairs}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('evidence_root', type=Path)
    args = parser.parse_args()
    try:
        print(json.dumps(compare(args.evidence_root), indent=2))
    except (ValueError, KeyError, OSError) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
