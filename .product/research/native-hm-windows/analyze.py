#!/usr/bin/env python3
"""Generate reviewable design tables, without changing production game data."""
import csv
import hashlib
import json
from collections import defaultdict
from pathlib import Path
from coverage import CoverageModel

HERE = Path(__file__).resolve().parent
MODES = ('modern', 'legacy')
UTILITIES = tuple('MOVE_' + x for x in ('CUT', 'FLASH', 'SURF', 'STRENGTH', 'ROCK_SMASH', 'WATERFALL', 'WHIRLPOOL', 'DIVE'))


def label(value):
    return value.removeprefix('SPECIES_').removeprefix('MOVE_').replace('_', ' ').title()


def ranges(values):
    spans = []
    for value in sorted(set(values)):
        if spans and value == spans[-1][1] + 1:
            spans[-1][1] = value
        else:
            spans.append([value, value])
    return ', '.join(str(a) if a == b else f'{a}-{b}' for a, b in spans) or 'none'


def moveset(entries, level):
    moves = []
    for learn_level, move in entries:
        if learn_level == 0:
            continue
        if learn_level > level:
            break
        if move not in moves:
            moves.append(move)
            moves = moves[-4:]
    return moves


def build_design(baseline, proposal):
    rows = {}
    for move, assignments in proposal['moves'].items():
        move = 'MOVE_' + move
        for name, level in assignments.items():
            species = 'SPECIES_' + name
            source = baseline[species]
            assert move in source['compatible'], (species, move, 'incompatible')
            row = rows.setdefault(species, {'assignments': {}, 'source': source})
            row['assignments'][move] = level if isinstance(level, dict) else {m: level for m in MODES}
    for species, row in rows.items():
        native = set().union(*(set(row['source']['natural_utilities'][m]) for m in MODES))
        roles = native | set(row['assignments'])
        assert len(roles) <= 2, (species, roles)
        row['roles'] = sorted(roles)
        row['modes'] = {}
        for mode in MODES:
            original = row['source'][mode]
            additions = [(levels[mode], move) for move, levels in row['assignments'].items()
                         if not any(m == move for _, m in original)]
            # Stable ordering: new moves come after every original at that level.
            entries = sorted(original + [list(pair) for pair in additions], key=lambda pair: pair[0])
            assert [pair for pair in entries if tuple(pair) not in set(additions)] == original
            assert len(entries) <= 40, (species, mode, len(entries))
            for level, move in additions:
                assert sum(m == move for _, m in entries) == 1
                assert 1 <= level <= 100
            row['modes'][mode] = {
                'entries': entries,
                'added': dict((move, level) for level, move in additions),
                'windows': {move: [level for level in range(1, 101) if move in moveset(entries, level)]
                            for move in row['roles']},
                'learn_levels': {move: [level for level, m in entries if m == move]
                                 for move in row['roles']},
            }
    return rows


def regional_profile(p):
    # Conservative exclusion of known optional, unused and late special venues.
    # Remaining profiles still do not prove road/warp/obstacle reachability.
    excluded = ('SAFARI', 'ALTERING_CAVE', 'UNUSED', 'SKY_PILLAR', 'SEAFLOOR',
                'CAVE_OF_ORIGIN', 'VICTORY_ROAD', 'MT_SILVER', 'TIN_TOWER',
                'MAGMA_HIDEOUT', 'UNDERWATER', 'DESERT_UNDERPASS')
    return p['region'] in ('JOHTO', 'KANTO', 'HOENN') and not any(x in p['map'] for x in excluded)


def analyze(rows, model):
    species_stats = {s: {'locations': set(), 'ratings': {m: {move: defaultdict(set) for move in r['roles']} for m in MODES}}
                     for s, r in rows.items()}
    regional = defaultdict(dict)
    win = {(s, mode, move): set(r['modes'][mode]['windows'][move])
           for s, r in rows.items() for mode in MODES for move in r['roles']}
    profiles = [p for p in model.profiles if p['possible_species'] & rows.keys()]
    for p in profiles:
        for rating in range(81):
            outcomes = model.profile_outcomes(p, rating)
            sums = defaultdict(float)
            witnesses = defaultdict(set)
            for (s, level), chance in outcomes.items():
                if s not in rows:
                    continue
                stats = species_stats[s]
                stats['locations'].add((p['region'], p['map'], p['method'], p['time']))
                for mode in MODES:
                    for move in rows[s]['roles']:
                        if level in win[s, mode, move]:
                            stats['ratings'][mode][move][p['region']].add(rating)
                            if regional_profile(p) and (move != 'MOVE_SURF' or p['method'] in ('land_mons', 'fishing_mons')):
                                sums[mode, move] += float(chance)
                                witnesses[mode, move].add(s)
            for (mode, move), chance in sums.items():
                key = (p['region'], mode, move)
                current = regional[key].get(rating)
                if current is None or chance > current['chance']:
                    regional[key][rating] = {'chance': chance, 'map': p['map'], 'method': p['method'],
                                             'time': p['time'], 'rod': p['rod'], 'species': sorted(witnesses[mode, move])}
    return species_stats, regional


def export(rows, stats, regional):
    # Exact locality inventory is a separate CSV so the main roster remains readable.
    with (HERE / 'locations.csv').open('w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['species', 'region', 'map', 'method', 'time', 'qualification'])
        for s in sorted(stats):
            for location in sorted(stats[s]['locations']):
                writer.writerow([s, *location, 'table presence; access not verified'])
    csv_rows = []
    md = ['# Proposed native utility roster', '',
          'Research proposal, not implemented. Modern means Gen 7; legacy means Gen 3.',
          'A window is the inclusive caught-level range in which the generated four moves contain the utility. '
          'It is not a Trainer Rating range. An owned Pokemon can keep the move after this window.',
          'Original native move occurrences are retained at their original levels. New utility entries appear once. '
          'All native utility types count toward the two-type cap, including moves not added by this proposal.',
          'See [readable catching locations](locations.md), [all map/method/time records](locations.csv), and [measured TR evidence](coverage.json). '
          'Optional, inaccessible, and species-devolution cases can appear here and are not guaranteed acquisition routes.', '',
          '| Pokemon | Utility | Modern learn level -> catch window | Legacy learn level -> catch window | Encounter regions |',
          '| --- | --- | --- | --- | --- |']
    serial = []
    for s, row in sorted(rows.items()):
        regions = sorted({loc[0] for loc in stats[s]['locations']})
        for move in row['roles']:
            fields = {}
            for mode in MODES:
                data = row['modes'][mode]
                levels = ','.join(map(str, data['learn_levels'][move])) or 'absent'
                fields[mode] = f"{levels}{'' if move in data['added'] else ' (native)'} -> {ranges(data['windows'][move])}"
                csv_rows.append({'species': s, 'move': move, 'mode': mode, 'learn_levels': levels,
                                 'existing_native': move not in data['added'], 'catch_window': ranges(data['windows'][move]),
                                 'JOHTO_TR': ranges(stats[s]['ratings'][mode][move]['JOHTO']),
                                 'KANTO_TR': ranges(stats[s]['ratings'][mode][move]['KANTO']),
                                 'HOENN_TR': ranges(stats[s]['ratings'][mode][move]['HOENN'])})
            md.append(f"| {label(s)} | {label(move)} | {fields['modern']} | {fields['legacy']} | {', '.join(regions) or 'No ordinary catch: evolution/special only'} |")
        serial.append({'species': s, 'roles': row['roles'], 'modes': row['modes'],
                       'locations': [list(x) for x in sorted(stats[s]['locations'])]})
    (HERE / 'roster.md').write_text('\n'.join(md) + '\n')
    with (HERE / 'roster.csv').open('w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(csv_rows[0])); writer.writeheader(); writer.writerows(csv_rows)
    (HERE / 'roster.json').write_text(json.dumps(serial, indent=2) + '\n')
    evidence = []
    for region in ('JOHTO', 'KANTO', 'HOENN'):
        for mode in MODES:
            for move in UTILITIES:
                per_rating = regional[region, mode, move]
                covered = sorted(per_rating)
                gaps = sorted(set(range(81)) - set(covered))
                evidence.append({'region': region, 'mode': mode, 'move': move,
                                 'covered_TR': ranges(covered), 'missing_TR': ranges(gaps),
                                 'best_profile_minimum_chance': min((v['chance'] for v in per_rating.values()), default=0),
                                 'witnesses': per_rating})
    (HERE / 'coverage.json').write_text(json.dumps({
        'proposal_sha256': hashlib.sha256((HERE / 'proposal.json').read_bytes()).hexdigest(),
        'scope': 'Filtered regional table coverage; best available time/rod/profile, not every location or time. Surf excludes water/rocks; other roles may require prior traversal. Uniform level-roll model, no lure/lead modifiers. No runtime C test.',
        'results': evidence}, indent=2) + '\n')
    print(f'{len(rows)} species; {sum(len(r["roles"]) for r in rows.values())} utility roles; all compatibility/cap/native-order/single-addition checks passed.')
    print('Species without ordinary encounter outcome:', ', '.join(s for s in stats if not stats[s]['locations']) or 'none')
    for e in evidence:
        if e['missing_TR'] != 'none':
            print(e['region'], e['mode'], e['move'], 'GAPS', e['missing_TR'])


def main():
    baseline = json.loads((HERE / 'baseline.json').read_text())['species']
    proposal = json.loads((HERE / 'proposal.json').read_text())
    rows = build_design(baseline, proposal)
    model = CoverageModel()
    stats, regional = analyze(rows, model)
    export(rows, stats, regional)


if __name__ == '__main__':
    main()
