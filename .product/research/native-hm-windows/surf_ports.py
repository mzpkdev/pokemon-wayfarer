#!/usr/bin/env python3
"""Optimize one Surf insertion per compatible shore species; research only.

Coordinate ascent is exploratory, not an infeasibility proof. The separate
Lilycove two-species exhaustive search is exhaustive for levels 1..100 only
with the existing fishing slots, fixed other assignments, and canonical order.
This uses table availability, with no assertion of bank accessibility.
"""
from collections import defaultdict
import json
from pathlib import Path
import random
import hashlib
from coverage import CoverageModel

HERE = Path(__file__).resolve().parent
MAPS = {'MAP_CIANWOOD_CITY_HNS', 'MAP_VERMILION_CITY_HNS',
        'MAP_VERMILION_CITY_PORT_OUTSIDE_HNS', 'MAP_CINNABAR_ISLAND_HNS',
        'MAP_LILYCOVE_CITY', 'MAP_MOSSDEEP_CITY', 'MAP_PACIFIDLOG_TOWN', 'MAP_ROUTE118'}


def has_surf(entries, learn, level):
    moves = []
    schedule = [(learn if lv is None else lv, move) for lv, move in entries]
    # Original entries precede additions; additions retain proposal JSON order.
    schedule.sort(key=lambda row: row[0])
    for lv, move in schedule:
        if not 1 <= lv <= level:
            continue
        if move not in moves:
            if len(moves) == 4:
                moves.pop(0)
            moves.append(move)
    return 'MOVE_SURF' in moves


def ranges(values):
    result = []
    for n in sorted(values):
        if result and result[-1][1] + 1 == n:
            result[-1][1] = n
        else:
            result.append([n, n])
    return result


def main():
    model = CoverageModel()
    baseline = json.loads((HERE / 'baseline.json').read_text())['species']
    proposal_bytes = (HERE / 'proposal.json').read_bytes()
    proposal = json.loads(proposal_bytes)['moves']
    profiles = [p for p in model.profiles if p['map'] in MAPS and p['method'] == 'fishing_mons'
                and p['rod'] == 'OLD_ROD']
    records = list(model.records(maps=MAPS, methods='fishing_mons', rods='OLD_ROD'))
    possible = {r['species'] for r in records}
    # Use the stricter modern union for both modes: species assignments should
    # never become three distinct native utilities under the other ruleset.
    def roles(s):
        native = set().union(*map(set, baseline[s]['natural_utilities'].values()))
        assigned = {'MOVE_'+move for move,assignments in proposal.items()
                    if s.removeprefix('SPECIES_') in assignments and assignments[s.removeprefix('SPECIES_')] is not None}
        return native | assigned | {'MOVE_SURF'}
    candidates = sorted(s for s in possible if 'MOVE_SURF' in baseline[s]['compatible'] and len(roles(s)) <= 2)
    cells = sorted({(p['map'], p['time'], tr) for p in profiles for tr in range(81)})
    cell_ids = {c: i for i, c in enumerate(cells)}
    full = (1 << len(cells)) - 1
    required = sum(1 << i for i, (m, t, _) in enumerate(cells)
                   if m.endswith('_HNS') and not (m == 'MAP_CIANWOOD_CITY_HNS' and t == 'TIME_NIGHT'))
    by_species = defaultdict(list)
    by_species_probability = defaultdict(list)
    for row in records:
        if row['species'] in candidates:
            by_species[row['species']].append((cell_ids[(row['map'], row['time'], row['TR'])], row['level']))
            by_species_probability[row['species']].append((cell_ids[(row['map'], row['time'], row['TR'])],
                                                          row['level'], float(row['probability'])))
    lilymask = sum(1 << i for i,c in enumerate(cells) if c[0] == 'MAP_LILYCOVE_CITY')
    result = {'assumptions': __doc__, 'status':'Exploratory alternative; fixed_ports.json validates actual proposal.',
              'proposal_sha256':hashlib.sha256(proposal_bytes).hexdigest(), 'candidates': candidates, 'modes': {}}
    for mode in ('modern', 'legacy'):
        schedules = {}
        for species in candidates:
            schedule = list(baseline[species][mode])
            for move, assignments in proposal.items():
                learn = assignments.get(species.removeprefix('SPECIES_'))
                if isinstance(learn, dict):
                    learn = learn.get(mode)
                if learn is not None and 'MOVE_' + move not in baseline[species]['natural_utilities'][mode]:
                    schedule.append([None if move == 'SURF' else learn, 'MOVE_' + move])
            if not any(move == 'MOVE_SURF' for _,move in schedule):
                schedule.append([None,'MOVE_SURF'])
            schedules[species] = schedule
        options = {}
        options_probability = {}
        for species in candidates:
            schedule = schedules[species]
            options[species] = {}
            options_probability[species] = {}
            levels = [None] if 'MOVE_SURF' in baseline[species]['natural_utilities'][mode] else range(1, 101)
            for learn in levels:
                active = {lv for lv in range(1, 101) if has_surf(schedule, learn, lv)}
                mask = 0
                for cell, level in by_species[species]:
                    if level in active:
                        mask |= 1 << cell
                options[species][learn] = mask
                probabilities = defaultdict(float)
                for cell, level, probability in by_species_probability[species]:
                    if level in active:
                        probabilities[cell] += probability
                options_probability[species][learn] = dict(probabilities)
        # Optimize required historic HNS ports first, then all other cells.
        def score(mask):
            return ((mask & required).bit_count(), mask.bit_count())
        rng = random.Random(413)
        best_score = (-1, -1)
        best = None
        for restart in range(24):
            choice = {s: rng.choice(list(options[s])) for s in candidates}
            for iteration in range(12):
                changed = False
                order = candidates.copy()
                rng.shuffle(order)
                for species in order:
                    other = 0
                    for s in candidates:
                        if s != species:
                            other |= options[s][choice[s]]
                    current = choice[species]
                    preferred = proposal['SURF'].get(species.removeprefix('SPECIES_'), 40)
                    if isinstance(preferred, dict):
                        preferred = preferred.get(mode, 40)
                    chosen = max(options[species], key=lambda lv: (score(other | options[species][lv]),
                                 -abs((lv or 1) - preferred), -(lv or 1)))
                    if chosen != current:
                        changed = True
                        choice[species] = chosen
                if not changed:
                    break
            mask = 0
            for s in candidates:
                mask |= options[s][choice[s]]
            if score(mask) > best_score:
                best_score, best = score(mask), (dict(choice), mask)
        choice, mask = best
        # Preserve maximum existence coverage, then improve the number of cells
        # at >=8% Old Rod conditional probability and capped probability mass.
        for _ in range(5):
            changed = False
            for species in candidates:
                other, probabilities = 0, defaultdict(float)
                for s in candidates:
                    if s != species:
                        other |= options[s][choice[s]]
                        for cell, probability in options_probability[s][choice[s]].items():
                            probabilities[cell] += probability
                preferred = proposal['SURF'].get(species.removeprefix('SPECIES_'), 40)
                if isinstance(preferred, dict):
                    preferred = preferred.get(mode, 40)
                def quality(lv):
                    threshold_gain, mass_gain = 0, 0.0
                    for cell,p in options_probability[species][lv].items():
                        old = probabilities[cell]
                        threshold_gain += int(old+p >= .08-1e-12) - int(old >= .08-1e-12)
                        mass_gain += min(.08,old+p)-min(.08,old)
                    return (score(other | options[species][lv]), threshold_gain, round(mass_gain,10),
                            -abs((lv or 1)-preferred), -(lv or 1))
                chosen = max(options[species], key=quality)
                changed |= chosen != choice[species]
                choice[species] = chosen
            if not changed:
                break
        mask = 0
        for species in candidates:
            mask |= options[species][choice[species]]
        # Exhaustive proof restricted to current Lilycove's eligible candidates.
        lily_species = [s for s in candidates if any(cells[c][0] == 'MAP_LILYCOVE_CITY' for c,_ in by_species[s])]
        assert set(lily_species) == {'SPECIES_STARYU', 'SPECIES_TENTACOOL'}, lily_species
        a,b = lily_species
        lily_best = max(((options[a][la] | options[b][lb]) & lilymask).bit_count()
                        for la in options[a] for lb in options[b])
        lily_choices = [(la,lb) for la in options[a] for lb in options[b]
                        if ((options[a][la] | options[b][lb]) & lilymask).bit_count() == lily_best]
        gaps = defaultdict(list)
        for i,(m,t,tr) in enumerate(cells):
            if not mask & (1 << i):
                gaps[m + '/' + t].append(tr)
        # Assess exact probability per successful encounter for all rod grades.
        probs = defaultdict(lambda: defaultdict(float))
        for r in model.records(species=set(candidates), maps=MAPS, methods='fishing_mons'):
            s = r['species']
            if has_surf(schedules[s], choice[s], r['level']):
                key = r['map'] + '/' + r['time'] + '/' + r['rod']
                probs[key][r['TR']] += float(r['probability'])
        result['modes'][mode] = {
            'assignments': choice,
            'covered_cells': mask.bit_count(), 'total_cells': len(cells),
            'required_covered': (mask & required).bit_count(), 'required_total': required.bit_count(),
            'gaps': {k:ranges(v) for k,v in gaps.items()},
            'minimum_probability_including_gaps': {k: min(v.get(tr, 0) for tr in range(81)) for k,v in probs.items()},
            'below_eight_percent_TR': {k:ranges(tr for tr in range(81) if v.get(tr,0)<.08-1e-12)
                                      for k,v in probs.items()},
            'lilycove_witnesses': {s: {'surf_level': choice[s],
                                      'wild_level_windows': ranges(lv for lv in range(1,101) if has_surf(schedules[s],choice[s],lv)),
                                      'TR_windows': ranges(cells[c][2] for c in range(len(cells))
                                                         if cells[c][0] == 'MAP_LILYCOVE_CITY' and options[s][choice[s]] & (1<<c))}
                                    for s in lily_species},
            'probability_by_TR': {k:[round(v.get(tr, 0),8) for tr in range(81)] for k,v in probs.items()},
            'lilycove_exhaustive': {'species': lily_species, 'max_covered': lily_best,
                                   'total': lilymask.bit_count(), 'best_level_pairs': lily_choices[:20],
                                   'number_best_pairs': len(lily_choices)},
        }
    (HERE / 'surf_ports.json').write_text(json.dumps(result, indent=2) + '\n')
    for mode, row in result['modes'].items():
        print(mode, json.dumps({k:v for k,v in row.items() if k != 'probability_by_TR'}, indent=2))


if __name__ == '__main__':
    main()
