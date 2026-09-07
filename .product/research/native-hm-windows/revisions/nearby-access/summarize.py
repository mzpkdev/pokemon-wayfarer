#!/usr/bin/env python3
"""Produce compact, reproducible nearby-access results and catch itineraries."""
import csv
import hashlib
import json
from fractions import Fraction
from pathlib import Path

HERE = Path(__file__).resolve().parent


def main():
    data = json.loads((HERE / 'results.json').read_text())
    revised = data['distributions']['revised']
    assert revised['proposal_sha256'] == hashlib.sha256((HERE / 'proposal.json').read_bytes()).hexdigest()
    assert data['scenarios_sha256'] == hashlib.sha256((HERE / 'scenarios.json').read_bytes()).hexdigest()
    results = revised['results']
    assert len(results) == 132
    assert all(not r['missing_TR_values'] for r in results)
    assert all(not r['below_minimum_TR_values'] for r in results if r['rod'] == 'OLD_ROD')
    for r in results:
        if r['rod'] == 'OLD_ROD' and r['scenario'] in ('blackthorn_surf', 'den_whirlpool'):
            assert all(p['nearest_adequate']['detour_rank'] == 0 for p in r['per_TR'])
    coverage = json.loads((HERE / 'coverage.json').read_text())
    assert coverage['proposal_sha256'] == revised['proposal_sha256']
    assert len(coverage['results']) == 48
    assert all(r['missing_TR'] == 'none' for r in coverage['results'])
    rows = []
    lines = ['# Nearby catch itineraries', '',
             'Generated from the selected proposal and results. All probabilities are conditional on an encounter.',
             'Fishing uses the Old Rod; land encounters have no bite-rate multiplier.',
             'Each row names an adequate single source. Do not sum probabilities across locations.', '',
             f"Proposal SHA-256: `{revised['proposal_sha256']}`.", '']
    for r in results:
        if r['rod'] != 'OLD_ROD':
            continue
        rows.append({'scenario': r['scenario'], 'mode': r['mode'], 'clock': r['clock'],
                     'minimum_best_chance_exact': r['minimum_best_chance_exact'],
                     'minimum_best_percent': round(100 * r['minimum_best_chance'], 6),
                     'maximum_required_detour_rank': max(p['nearest_adequate']['detour_rank'] for p in r['per_TR'])})
        lines += [f"## {r['scenario']}: {r['mode']}, {r['clock'].lower()}", '',
                  '| TR | Nearest adequate source | Method | Possible carriers | Minimum chance in interval |',
                  '| --- | --- | --- | --- | --- |']
        groups = []
        for p in r['per_TR']:
            w = p['nearest_adequate']
            key = (w['map'], w['method'], tuple(c['species'] for c in w['carriers']))
            chance = Fraction(w['exact_probability'])
            if groups and groups[-1][0] == key:
                groups[-1][2] = p['TR']
                groups[-1][3] = min(groups[-1][3], chance)
            else:
                groups.append([key, p['TR'], p['TR'], chance])
        for (map_name, method, species), first, last, chance in groups:
            interval = str(first) if first == last else f'{first}-{last}'
            carriers = ', '.join(s.removeprefix('SPECIES_').replace('_', ' ').title() for s in species)
            lines.append(f'| {interval} | `{map_name}` | {method} | {carriers} | {100 * float(chance):.3f}% |')
        lines.append('')
    with (HERE / 'summary.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    (HERE / 'itineraries.md').write_text('\n'.join(lines) + '\n')
    print(f'Validated {len(results) * 81} nonempty scenario cells, 3,564 Old Rod floor cases, '
          f'3,888 regional cells and local Blackthorn/Den coverage; wrote {len(rows)} Old Rod summaries.')


if __name__ == '__main__':
    main()
