#!/usr/bin/env python3
"""Bounded coordinate search for Surf timing options, not a balance guarantee."""
import copy
import argparse
import json
from pathlib import Path
import sys
from collections import defaultdict

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import simulate as sim


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--early', choices=('CHINCHOU','HORSEA'))
    parser.add_argument('--output', type=Path, default=HERE/'level-options.json')
    parser.add_argument('--focus', default='cianwood_surf,olivine_surf,vermilion_surf,cinnabar_surf')
    args = parser.parse_args()
    baseline = json.loads((sim.RESEARCH / 'baseline.json').read_text())['species']
    proposal = json.loads((sim.RESEARCH / 'proposal.json').read_text())
    model = sim.CoverageModel()
    wanted = set(args.focus.split(','))
    scenarios = [s for s in json.loads((HERE / 'scenarios.json').read_text())['scenarios'] if s['id'] in wanted]
    profile_list = []
    groups = []
    for scenario in scenarios:
        for clock in ('DAY', 'NIGHT'):
            inds = []
            for p, _ in sim.select_profiles(model, scenario, clock, 'OLD_ROD'):
                if p not in profile_list:
                    profile_list.append(p)
                inds.append(profile_list.index(p))
            groups.append((scenario['id'], clock, inds))
    observations = [model.profile_outcomes(p, tr) for p in profile_list for tr in range(81)]
    present = set().union(*(set(s for s, _ in obs) for obs in observations))
    rows = sim.build_design(baseline, proposal)
    output = {'scope': 'Rejected exploratory timing options; heuristic search, not an impossibility proof or minimum-edit proof. Exact final simulator validates selected options.',
              'focused_scenarios': sorted(wanted), 'early_seed': args.early,
              'fixed_surf_species': ['TENTACOOL','STARYU','KRABBY','KINGLER'],
              'allowed_learning_levels': [1,60], 'modes': {}}
    revised = copy.deepcopy(proposal)
    for mode in sim.MODES:
        candidates = [s for s in rows if s in present and 'MOVE_SURF' in rows[s]['assignments']
                      and not any(m == 'MOVE_SURF' for _, m in baseline[s][mode])]
        defaults = {s: rows[s]['assignments']['MOVE_SURF'][mode] for s in candidates}
        data = {}
        for species in candidates:
            original = baseline[species][mode]
            weights = [[(level, float(chance)) for (s, level), chance in obs.items() if s == species] for obs in observations]
            data[species] = {}
            for level in range(1, 61):
                additions = [[level if move == 'MOVE_SURF' else assignment[mode], move]
                             for move, assignment in rows[species]['assignments'].items()
                             if not any(native_move == move for _, native_move in original)]
                entries = sorted(original + additions, key=lambda x:x[0])
                window = {lev for lev in range(1,101) if 'MOVE_SURF' in sim.moveset(entries,lev)}
                if level == defaults[species]:
                    assert window == set(rows[species]['modes'][mode]['windows']['MOVE_SURF']), (species,mode,window,rows[species]['modes'][mode]['windows']['MOVE_SURF'])
                data[species][level] = [sum(chance for lev, chance in obs if lev in window) for obs in weights]
        fixed = []
        for obs in observations:
            total = 0.0
            for (species, level), chance in obs.items():
                if species in candidates:
                    continue
                entries = rows[species]['modes'][mode]['entries'] if species in rows else baseline[species][mode]
                if 'MOVE_SURF' in sim.moveset(entries, level):
                    total += float(chance)
            fixed.append(total)
        def totals(levels):
            out = fixed[:]
            for species, level in levels.items():
                out = [a+b for a,b in zip(out,data[species][level])]
            return out
        def scores(values, levels):
            best = [max(values[i*81+tr] for i in inds) for _,_,inds in groups for tr in range(81)]
            return (sum(v >= 0.08-1e-12 for v in best), sum(min(v,0.08) for v in best),
                    -sum(levels[s] != defaults[s] for s in candidates),
                    -sum(abs(levels[s]-defaults[s]) for s in candidates))
        levels = defaults.copy()
        if mode == 'modern' and args.early:
            levels['SPECIES_'+args.early] = 5
        for sweep in range(12):
            changed = False
            for species in candidates:
                if species in ('SPECIES_TENTACOOL','SPECIES_STARYU','SPECIES_KRABBY','SPECIES_KINGLER') or (mode == 'modern' and species == 'SPECIES_'+str(args.early)):
                    continue
                current = totals(levels)
                removed = [a-b for a,b in zip(current,data[species][levels[species]])]
                bestlevel = levels[species]
                bestscore = scores(current, levels)
                for lev in sorted(range(1,61), key=lambda x:(x!=defaults[species],abs(x-defaults[species]))):
                    trial = dict(levels, **{species:lev})
                    values = [a+b for a,b in zip(removed,data[species][lev])]
                    score = scores(values,trial)
                    # Round cumulative floating arithmetic to avoid phantom improvements.
                    score = (score[0],round(score[1],10),*score[2:])
                    comparable = (bestscore[0],round(bestscore[1],10),*bestscore[2:])
                    if score > comparable:
                        bestlevel,bestscore = lev,score
                changed |= bestlevel != levels[species]
                levels[species] = bestlevel
            print(mode, 'sweep',sweep,scores(totals(levels),levels),flush=True)
            if not changed:
                break
        changes = {s.removeprefix('SPECIES_'):{'old':defaults[s],'new':levels[s]} for s in candidates if levels[s]!=defaults[s]}
        for s in candidates:
            key = s.removeprefix('SPECIES_')
            original = revised['moves']['SURF'][key]
            if not isinstance(original,dict):
                revised['moves']['SURF'][key] = {m:original for m in sim.MODES}
            revised['moves']['SURF'][key][mode] = levels[s]
        output['modes'][mode] = {'changes':changes,'score':scores(totals(levels),levels)}
    evidence = sim.evaluate(model,baseline,revised,scenarios)
    output['exact_old_rod_results'] = [{k:v for k,v in r.items() if k not in ('per_TR','profiles')}
                                       for r in evidence['results'] if r['rod']=='OLD_ROD']
    output['revised_moves'] = revised['moves']
    args.output.write_text(json.dumps(output,indent=2)+'\n')
    print(json.dumps(output['modes'],indent=2))
    for r in output['exact_old_rod_results']:
        print(r['scenario'],r['mode'],r['clock'],r['missing_TR'],r['below_minimum_TR'],r['minimum_best_chance_exact'])


if __name__ == '__main__':
    main()
