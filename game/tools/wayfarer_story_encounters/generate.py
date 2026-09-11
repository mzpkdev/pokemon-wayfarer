#!/usr/bin/env python3
"""Compatibility audit for the Phase-D encounter declarations.

The editable ordinary allowlist moved to map gameplay sidecars. This command
remains as the Makefile's historical audit entry point, but consumes the
production encounter compiler instead of a frozen migration snapshot.
"""

import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from tools.gameplay_content.configuration import numeric_defines
from tools.gameplay_content.encounters import compile_encounters
from tools.gameplay_content.maps import load_maps
from tools.gameplay_content.trainers import load_trainers


def validate_rematch_dialogue_identities(rows):
    """Require each rematch to reuse the reviewed base caller identity."""
    base_dialogues = {}
    for row in rows:
        if row["command"].startswith("trainerbattle_rematch"):
            continue
        identity = (row["stableKey"], row["dialogue"])
        base_dialogues.setdefault(row["baseTrainer"], set()).add(identity)
    for row in rows:
        if not row["command"].startswith("trainerbattle_rematch"):
            continue
        identities = base_dialogues.get(row["baseTrainer"], set())
        if len(identities) != 1 or (row["stableKey"], row["dialogue"]) not in identities:
            raise ValueError(f"rematch dialogue must match one stable base caller: {row['caller']}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="retained Makefile compatibility flag")
    args = parser.parse_args()
    cppflags = ('-iquote include -Wno-trigraphs -DMODERN=1 -DTESTING=0 '
                '-DPOKEMON_WAYFARER -DPOKEMON_HNS -std=gnu17')
    asflags = ('-mcpu=arm7tdmi -march=armv4t -meabi=5 --defsym MODERN=1 '
               '--defsym POKEMON_WAYFARER=1 --defsym POKEMON_HNS=1')
    maps = load_maps(ROOT, 'wayfarer')
    trainers = load_trainers(ROOT, 'wayfarer', 'cpp', cppflags)
    result = compile_encounters(ROOT, 'wayfarer', numeric_defines(ROOT, 'cpp', cppflags), maps, trainers,
                                cpp='cpp', cppflags=cppflags,
                                assembler='arm-none-eabi-as', asflags=asflags)
    ordinary = [row for row in result['report']['encounters']
                if row['profile']['family'] == 'ordinary']
    if len(ordinary) != 851:
        raise ValueError(f'expected 851 reviewed ordinary callers, got {len(ordinary)}')
    redirects = sum(row['profile']['outcomePolicy'] == 'LOSS_RETURN' for row in ordinary)
    if redirects != 787:
        raise ValueError('ordinary loss-return projection drifted')
    if args.check and 'gameplay_encounters.inc' not in result['outputs']:
        raise ValueError('encounter runtime projection missing')
    print(f'Validated {len(ordinary)} reviewed ordinary callers; {redirects} field-loss redirects.')


if __name__ == '__main__':
    main()
