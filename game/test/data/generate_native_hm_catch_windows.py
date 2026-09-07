#!/usr/bin/env python3
"""Export immutable approved roster expectations for the production C tests."""
import hashlib
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
REVISION = ROOT / '.product/research/native-hm-windows/revisions/nearby-access'
roster_path = REVISION / 'roster.json'
roster = json.loads(roster_path.read_text())
lines = [f'// Approved roster SHA256: {hashlib.sha256(roster_path.read_bytes()).hexdigest()}',
         '// Regenerate with python3 game/test/data/generate_native_hm_catch_windows.py']
rows = []
for species in roster:
    for modern, mode in enumerate(('legacy', 'modern')):
        data = species['modes'][mode]
        name = species['species'].removeprefix('SPECIES_') + '_' + mode
        lines.append(f'static const struct ExpectedCatchEntry sEntries_{name}[] = {{')
        lines += [f'    {{ {level}, {move} }},' for level, move in data['entries']]
        lines.append('};')
        lines.append(f'static const struct ExpectedCatchMoves sMoves_{name}[] = {{')
        prior = None
        for level in range(1, 101):
            moves = []
            for entry_level, move in data['entries']:
                if entry_level == 0 or entry_level > level or move in moves:
                    continue
                moves = (moves + [move])[-4:]
            for role, window in data['windows'].items():
                assert (role in moves) == (level in window), (name, level, role)
            moves += ['MOVE_NONE'] * (4 - len(moves))
            if moves != prior:
                lines.append(f'    {{ {level}, {{ {", ".join(moves)} }} }},')
                prior = moves
        lines.append('};')
        rows.append(f'    {{ {species["species"]}, {modern}, ARRAY_COUNT(sEntries_{name}), sEntries_{name}, ARRAY_COUNT(sMoves_{name}), sMoves_{name} }},')
lines += ['static const struct ExpectedCatchSpecies sCatchSpecies[] = {', *rows, '};', '']
output = Path(__file__).with_name('native_hm_catch_windows.h')
rendered = '\n'.join(lines)
if '--check' in sys.argv:
    if output.read_text() != rendered:
        raise SystemExit('Stale native HM catch-window fixture; regenerate with ' + __file__)
else:
    output.write_text(rendered)
