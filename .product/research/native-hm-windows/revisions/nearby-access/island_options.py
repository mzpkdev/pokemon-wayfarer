#!/usr/bin/env python3
"""Bounded slot substitution search. No production files are changed."""
import hashlib
import itertools
import heapq
import json
from fractions import Fraction
from pathlib import Path
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT))
from analyze import build_design, ranges
from coverage import CoverageModel

CANDIDATES = ('TENTACOOL', 'TENTACRUEL', 'STARYU', 'CARVANHA', 'SHARPEDO',
              'CHINCHOU', 'LANTURN', 'SHELLDER')
FAMILY = {'TENTACRUEL': 'TENTACOOL', 'SHARPEDO': 'CARVANHA', 'LANTURN': 'CHINCHOU',
          'KINGLER': 'KRABBY', 'SEADRA': 'HORSEA', 'GYARADOS': 'MAGIKARP',
          'LOMBRE': 'LOTAD', 'POLIWHIRL': 'POLIWAG', 'GOLDUCK': 'PSYDUCK',
          'SLOWBRO': 'SLOWPOKE', 'QUAGSIRE': 'WOOPER'}


def main():
    model = CoverageModel()
    rows = build_design(json.loads((ROOT / 'baseline.json').read_text())['species'],
                        json.loads((ROOT / 'proposal.json').read_text()))
    known = {(s, mode): set(row['modes'][mode]['windows'].get('MOVE_SURF', []))
             for s, row in rows.items() for mode in ('modern', 'legacy')}
    contexts = [(mode, tr) for mode in ('modern', 'legacy') for tr in range(81)]

    def vector(profile, index, species):
        _, low, high, weight = profile['slots'][index]
        nums, dens = [], []
        for mode, tr in contexts:
            outcomes = model._slot_outcomes(species, low, high, profile['offset'], tr)
            nums.append(weight * sum(n for s, lv, n in outcomes if lv in known.get((s, mode), set())) / (high-low+1))
            dens.append(weight if outcomes else 0)
        return nums, dens

    def exact(profile, changes):
        p = dict(profile, slots=list(profile['slots']))
        for i, species in changes:
            _, low, high, weight = p['slots'][i]
            p['slots'][i] = (species, low, high, weight)
        values = []
        for mode, tr in contexts:
            values.append(sum((prob for (s, lv), prob in model.profile_outcomes(p, tr).items()
                               if lv in known.get((s, mode), set())), Fraction()))
        return {'minimum_probability': str(min(values)),
                'modes': {mode: {'minimum_probability': str(min(values[k*81:(k+1)*81])),
                                 'below_eight_percent_TR': ranges(tr for tr, v in enumerate(values[k*81:(k+1)*81]) if v < Fraction(2, 25)),
                                 'zero_probability_TR': ranges(tr for tr, v in enumerate(values[k*81:(k+1)*81]) if v == 0),
                                 'probability_by_TR': [str(v) for v in values[k*81:(k+1)*81]]}
                          for k, mode in enumerate(('modern', 'legacy'))}}

    results = {'input_sha256': {name: hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
                               for name in ('proposal.json', 'baseline.json', 'coverage.py', 'analyze.py')},
               'method': 'Coastal scope exhaustively searches one/two distinct slots and three if neither passes, within eight named candidates. All-approved scope exhaustively searches up to two slots. Aquatic beam scope uses fourteen candidates, width100, up to five slots, and is not an exhaustive impossibility/minimality proof. Preserve scope additionally requires at least one authored slot for each original species. Floating-point prefilter, exact Fraction verification of reported options. All slots keep authored levels and rod weights. No reachability claim.',
               'candidate_species': list(CANDIDATES), 'profiles': {}}
    results['script_sha256'] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    results['runtime_inputs_sha256'] = {str(path): hashlib.sha256(path.read_bytes()).hexdigest()
        for path in [Path(model.tool.__file__)] + [Path(getattr(model.tool, name)) for name in
            ('DEFAULT_ENCOUNTERS', 'DEFAULT_CONFIG', 'DEFAULT_RTC', 'DEFAULT_SCALING',
             'DEFAULT_SPECIES', 'DEFAULT_STANDARD_ROD_FISHING', 'DEFAULT_SPECIES_METADATA',
             'DEFAULT_SPECIES_INFO', 'DEFAULT_REGIONS', 'DEFAULT_TRAINER_RATING')]
        if path.is_file()}
    profiles = [p for p in model.profiles if p['method'] == 'fishing_mons' and p['rod'] == 'OLD_ROD'
                and p['map'] in ('MAP_MOSSDEEP_CITY', 'MAP_PACIFIDLOG_TOWN', 'MAP_CINNABAR_ISLAND_HNS')]
    all_surf = tuple(sorted(s[8:] for s, row in rows.items() if 'MOVE_SURF' in row['roles']))
    results['expanded_candidate_species'] = all_surf
    beam_candidates = CANDIDATES + ('HORSEA', 'SEADRA', 'REMORAID', 'KRABBY', 'KINGLER', 'GYARADOS')
    results['beam_candidate_species'] = beam_candidates
    for profile, candidates, scope in ([(p, CANDIDATES, 'coastal') for p in profiles]
            + [(p, all_surf, 'all_approved_surf') for p in profiles]
            + [(p, beam_candidates, 'aquatic_beam') for p in profiles]
            + [(p, beam_candidates, 'aquatic_beam_preserve') for p in profiles if p['map'] != 'MAP_CINNABAR_ISLAND_HNS']):
        key = profile['map'] + '/' + profile['time'] + '/' + scope
        originals = [slot[0] for slot in profile['slots']]
        families = {FAMILY.get(s[8:], s[8:]) for s in originals}
        base = [vector(profile, i, s) for i, s in enumerate(originals)]
        total_n = [sum(v[0][k] for v in base) for k in range(162)]
        total_d = [sum(v[1][k] for v in base) for k in range(162)]
        alternatives = []
        for i, original in enumerate(originals):
            for name in candidates:
                s = 'SPECIES_' + name
                if s == original:
                    continue
                n, d = vector(profile, i, s)
                alternatives.append((i, s, [n[k]-base[i][0][k] for k in range(162)],
                                     [d[k]-base[i][1][k] for k in range(162)]))
        passing, counts = [], {}
        for count in (() if scope.startswith('aquatic_beam') else (1, 2, 3) if scope == 'coastal' else (1, 2)):
            tested = 0
            for combination in itertools.combinations(alternatives, count):
                if len({c[0] for c in combination}) != count:
                    continue
                tested += 1
                minimum = 1.0
                for k in range(162):
                    denominator = total_d[k] + sum(c[3][k] for c in combination)
                    p = (total_n[k] + sum(c[2][k] for c in combination)) / denominator if denominator else 0
                    if p < .08 - 1e-12:
                        break
                    minimum = min(minimum, p)
                else:
                    changes = [(c[0], c[1]) for c in combination]
                    new_families = {FAMILY.get(s[8:], s[8:]) for _, s in changes} - families
                    passing.append((len(new_families), -minimum, changes))
            counts[str(count)] = tested
            if passing:
                break
        if scope.startswith('aquatic_beam'):
            beam = [()]
            for count in range(1, 6):
                scored, seen = [], set()
                for partial in beam:
                    occupied = {alternatives[a][0] for a in partial}
                    for a, alternative in enumerate(alternatives):
                        if alternative[0] in occupied:
                            continue
                        indexes = tuple(sorted(partial + (a,)))
                        if indexes in seen:
                            continue
                        seen.add(indexes)
                        combination = [alternatives[i] for i in indexes]
                        if scope.endswith('preserve'):
                            hypothetical_species = list(originals)
                            for c in combination:
                                hypothetical_species[c[0]] = c[1]
                            if not set(originals) <= set(hypothetical_species):
                                continue
                        probabilities = []
                        for k in range(162):
                            denominator = total_d[k] + sum(c[3][k] for c in combination)
                            probabilities.append((total_n[k] + sum(c[2][k] for c in combination)) / denominator if denominator else 0)
                        minimum = min(probabilities)
                        changes = [(c[0], c[1]) for c in combination]
                        new_families = {FAMILY.get(s[8:], s[8:]) for _, s in changes} - families
                        if minimum >= .08 - 1e-12:
                            passing.append((len(new_families), -minimum, changes))
                        score = (sum(p >= .08 - 1e-12 for p in probabilities),
                                 sum(min(p, .08) for p in probabilities), minimum, -len(new_families))
                        scored.append((score, indexes))
                counts[str(count)] = len(seen)
                if passing:
                    break
                beam = [v[1] for v in heapq.nlargest(100, scored)]
        passing.sort()
        # Retain best minimum for each novelty tier; up to four examples per tier.
        selected = []
        for novelty in sorted({v[0] for v in passing}):
            selected.extend([v for v in passing if v[0] == novelty][:4])
        entry = {'original_slots': profile['slots'], 'original': exact(profile, []),
                 'tested_combinations': counts, 'passing_options_at_minimum_change_count': len(passing), 'options': []}
        for novelty, _, changes in selected:
            option = {'new_family_count': novelty, 'changes': [{'slot_index_zero_based': i,
                      'from': originals[i], 'to': s, 'levels': list(profile['slots'][i][1:3])} for i, s in changes], 'rods': {}}
            for p in model.profiles:
                if p['map'] == profile['map'] and p['time'] == profile['time'] and p['method'] == 'fishing_mons':
                    option['rods'][p['rod']] = exact(p, changes)
            assert Fraction(option['rods']['OLD_ROD']['minimum_probability']) >= Fraction(2, 25)
            entry['options'].append(option)
        results['profiles'][key] = entry
        print(key, 'tested', counts, 'passing', len(passing), 'best', [(v[0], -v[1], v[2]) for v in selected], flush=True)
    (HERE / 'island-options.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
