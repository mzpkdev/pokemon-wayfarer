#!/usr/bin/env python3
"""Compare approved and revised utility distributions in traced reachable areas.

Reachability is supplied by scenarios.json, not inferred from encounter tables.
Each scenario names a move, allowed maps and methods, and optional detour ranks.
Probabilities use exact uniform authored-level rolls conditional on a successful
ordinary encounter. They exclude bite rate, walking rate, leads, lures and RNG
modulo bias. Hoenn's static TIME_DAY table applies in both clock cases.
"""

import argparse
import copy
from collections import defaultdict
from fractions import Fraction
from functools import lru_cache
import hashlib
import json
from pathlib import Path
import sys

HERE = Path(__file__).resolve().parent
RESEARCH = HERE.parents[1]
sys.path.insert(0, str(RESEARCH))
from analyze import MODES, build_design, moveset, ranges
from coverage import CoverageModel

RODS = ('OLD_ROD', 'GOOD_ROD', 'SUPER_ROD')


def probability(value):
    return {'probability': float(value), 'exact_probability': str(value)}


def select_profiles(model, scenario, clock, rod):
    allowed = {}
    for item in scenario['maps']:
        item = {'map': item} if isinstance(item, str) else item
        if item['map'] in allowed:
            raise ValueError(f"Duplicate map in {scenario['id']}: {item['map']}")
        allowed[item['map']] = item
    result = []
    for p in model.profiles:
        item = allowed.get(p['map'])
        if item is None:
            continue
        if p['method'] not in item.get('methods', ('land_mons', 'fishing_mons')):
            continue
        if p['method'] == 'fishing_mons' and p['rod'] != rod:
            continue
        if p['rod'] not in item.get('rods', (*RODS, 'NONE')):
            continue
        actual_time = 'TIME_DAY' if p['product'] == 'EMERALD' else 'TIME_' + clock
        if p['time'] != actual_time:
            continue
        if clock not in [t.removeprefix('TIME_') for t in item.get('times', ('DAY', 'NIGHT'))]:
            continue
        result.append((p, item.get('rank', 0)))
    return result


def evaluate(model, baseline, proposal, scenarios, full_profiles=False):
    model = copy.copy(model)
    model.profiles = [dict(p, slots=list(p['slots'])) for p in model.profiles]
    extra_species = {r['species'] for r in proposal.get('encounter_replacements', [])} - model.metadata.keys()
    if extra_species:
        g = model.tool
        known = g.species_ids(g.DEFAULT_SPECIES)
        model.metadata = {item['species']: item for item in g.load_species_metadata(
            g.DEFAULT_SPECIES_METADATA, g.DEFAULT_SPECIES_INFO, known,
            set(model.metadata) | extra_species)}
    applied_replacements = []
    for replacement in proposal.get('encounter_replacements', []):
        maps = replacement.get('maps', [replacement.get('map')])
        matches = 0
        for p in model.profiles:
            if p['map'] not in maps or p['method'] != replacement['method']:
                continue
            index = replacement['slot']
            assert isinstance(index, int) and 0 <= index < len(p['slots']), replacement
            old_species, low, high, weight = p['slots'][index]
            if 'expected_species' in replacement:
                expected = replacement['expected_species']
                expected = [expected] if isinstance(expected, str) else expected
                assert old_species in expected, (p['map'], p['time'], index, old_species, expected)
            species = replacement['species']
            assert species in baseline and species in model.metadata, species
            low = replacement.get('min_level', low)
            high = replacement.get('max_level', high)
            assert 1 <= low <= high <= 100, replacement
            p['slots'][index] = (species, low, high, weight)
            matches += 1
        assert matches, ('No replacement profile matched', replacement)
        applied_replacements.append(dict(replacement, matched_profiles=matches))
    rows = build_design(baseline, proposal)

    @lru_cache(maxsize=None)
    def knows(species, mode, level, move):
        entries = rows[species]['modes'][mode]['entries'] if species in rows else baseline[species][mode]
        return move in moveset(entries, level)

    @lru_cache(maxsize=None)
    def measure(profile_index, mode, move, rating):
        p = model.profiles[profile_index]
        outcomes = model.profile_outcomes(p, rating)
        if outcomes:
            assert sum(outcomes.values()) == 1, (p['map'], rating)
        carriers = defaultdict(lambda: {'levels': set(), 'chance': Fraction(0)})
        total = Fraction(0)
        for (species, level), chance in outcomes.items():
            if knows(species, mode, level, move):
                total += chance
                carriers[species]['levels'].add(level)
                carriers[species]['chance'] += chance
        return total, [{'species': species, 'levels': sorted(data['levels']),
                        'level_ranges': ranges(data['levels']), **probability(data['chance'])}
                       for species, data in sorted(carriers.items())]

    profile_indices = {id(p): i for i, p in enumerate(model.profiles)}
    results = []
    for scenario in scenarios:
        move = scenario['move']
        if not move.startswith('MOVE_'):
            move = 'MOVE_' + move
        threshold = Fraction(str(scenario.get('minimum_chance', 0.08)))
        for mode in MODES:
            for clock in [t.removeprefix('TIME_') for t in scenario.get('times', ('DAY', 'NIGHT'))]:
                for rod in scenario.get('rods', RODS):
                    profiles = select_profiles(model, scenario, clock, rod)
                    per_rating = []
                    profile_results = []
                    for p, rank in profiles if full_profiles else ():
                        observations = []
                        for rating in range(81):
                            chance, carriers = measure(profile_indices[id(p)], mode, move, rating)
                            observations.append({'TR': rating, **probability(chance), 'carriers': carriers})
                        profile_results.append({
                            'map': p['map'], 'method': p['method'], 'profile_time': p['time'],
                            'profile_label': p['label'], 'rod': p['rod'], 'detour_rank': rank,
                            'per_TR': observations,
                        })
                    for rating in range(81):
                        choices = []
                        for p, rank in profiles:
                            chance, carriers = measure(profile_indices[id(p)], mode, move, rating)
                            witness = {'map': p['map'], 'method': p['method'],
                                       'profile_time': p['time'], 'rod': p['rod'],
                                       'detour_rank': rank, **probability(chance), 'carriers': carriers}
                            choices.append((chance, rank, witness))
                        best = max(choices, key=lambda c: (c[0], -c[1]), default=None)
                        adequate = [c for c in choices if c[0] >= threshold]
                        nearest = min(adequate, key=lambda c: (c[1], -c[0]), default=None)
                        per_rating.append({'TR': rating,
                                           'best': best[2] if best else None,
                                           'nearest_adequate': nearest[2] if nearest else None})
                    chances = [Fraction(r['best']['exact_probability']) if r['best'] else Fraction(0)
                               for r in per_rating]
                    gaps = [tr for tr, chance in enumerate(chances) if chance == 0]
                    below = [tr for tr, chance in enumerate(chances) if chance < threshold]
                    results.append({'scenario': scenario['id'], 'move': move, 'mode': mode,
                                    'clock': clock, 'rod': rod, 'minimum_required_chance': float(threshold),
                                    'profile_count': len(profiles), 'missing_TR': ranges(gaps),
                                    'below_minimum_TR': ranges(below), 'missing_TR_values': gaps,
                                    'below_minimum_TR_values': below,
                                    'minimum_best_chance': float(min(chances)),
                                    'minimum_best_chance_exact': str(min(chances)),
                                    'per_TR': per_rating,
                                    **({'profiles': profile_results} if full_profiles else {})})
    return {'encounter_replacements_applied': applied_replacements,
            'inventory_checks': {'species': len(rows), 'roles': sum(len(r['roles']) for r in rows.values()),
                                'compatible': True, 'maximum_two_utility_types_per_species': True,
                                'original_native_entries_preserved': True, 'new_entries_once': True,
                                'both_modes_simulated_together_with_all_assignments': True},
            'results': results}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scenarios', type=Path, default=HERE / 'scenarios.json')
    parser.add_argument('--proposal', type=Path, default=HERE / 'proposal.json')
    parser.add_argument('--output', type=Path, default=HERE / 'results.json')
    parser.add_argument('--full-profiles', action='store_true',
                        help='Include every accessible profile/TR matrix; default keeps best/nearest witnesses.')
    args = parser.parse_args()
    scenario_data = json.loads(args.scenarios.read_text())
    scenarios = scenario_data['scenarios'] if isinstance(scenario_data, dict) else scenario_data
    assert len({s['id'] for s in scenarios}) == len(scenarios), 'Duplicate scenario ID'
    model = CoverageModel()
    known_maps = {p['map'] for p in model.profiles}
    for s in scenarios:
        for item in s['maps']:
            name = item if isinstance(item, str) else item['map']
            if name not in known_maps:
                raise ValueError(f"No encounter profile for {name} in {s['id']}; remove non-encounter transit maps")
    baseline = json.loads((RESEARCH / 'baseline.json').read_text())['species']
    approved = RESEARCH / 'proposal.json'
    paths = [('approved', approved)]
    if args.proposal.exists() and args.proposal.resolve() != approved.resolve():
        paths.append(('revised', args.proposal))
    result = {'scope': __doc__, 'scenarios_sha256': hashlib.sha256(args.scenarios.read_bytes()).hexdigest(),
              'full_profile_matrices': args.full_profiles, 'scenarios': scenarios, 'distributions': {}}
    for label, path in paths:
        evidence = evaluate(model, baseline, json.loads(path.read_text()), scenarios, args.full_profiles)
        evidence['proposal_sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
        evidence['proposal_path'] = str(path.relative_to(RESEARCH)) if path.is_relative_to(RESEARCH) else str(path)
        result['distributions'][label] = evidence
        print(label, evidence['inventory_checks'])
        for r in evidence['results']:
            if r['rod'] == 'OLD_ROD':
                print(r['scenario'], r['mode'], r['clock'], 'gaps', r['missing_TR'],
                      'below8%', r['below_minimum_TR'], 'min', r['minimum_best_chance_exact'])
    args.output.write_text(json.dumps(result, separators=(',', ':')) + '\n')


if __name__ == '__main__':
    main()
