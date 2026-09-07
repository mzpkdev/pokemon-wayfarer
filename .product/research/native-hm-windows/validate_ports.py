#!/usr/bin/env python3
"""Validate the fixed proposal with the canonical analyze.py move ordering.

Conditional ordinary encounter probabilities; no lure or lead modifiers.
Table-level availability does not certify map/bank reachability.
Hypothetical slot replacements are evaluated in memory, not implemented.
"""
from collections import defaultdict
from fractions import Fraction
import hashlib
import json
from pathlib import Path
from analyze import build_design, moveset, ranges
from coverage import CoverageModel
from surf_ports import MAPS

HERE = Path(__file__).resolve().parent


def main():
    baseline = json.loads((HERE / 'baseline.json').read_text())['species']
    proposal_bytes = (HERE / 'proposal.json').read_bytes()
    rows = build_design(baseline, json.loads(proposal_bytes))
    model = CoverageModel()
    profiles = [p for p in model.profiles if p['map'] in MAPS and p['method'] == 'fishing_mons']
    def surf_probability(p, tr, mode):
        return sum((prob for (s,lv),prob in model.profile_outcomes(p,tr).items()
                    if s in rows and 'MOVE_SURF' in moveset(rows[s]['modes'][mode]['entries'],lv)), Fraction())
    result = {'proposal_sha256': hashlib.sha256(proposal_bytes).hexdigest(),
              'assumptions': __doc__, 'modes': {}, 'lilycove_optional_slot_replacements': []}
    for mode in ('modern', 'legacy'):
        contexts = {}
        for p in profiles:
            values = [surf_probability(p,tr,mode) for tr in range(81)]
            contexts[p['map']+'/'+p['time']+'/'+p['rod']] = {
                'gaps': ranges(tr for tr,v in enumerate(values) if v == 0),
                'gap_count': sum(v == 0 for v in values),
                'below_eight_percent': ranges(tr for tr,v in enumerate(values) if v < Fraction(2,25)),
                'below_eight_percent_count': sum(v < Fraction(2,25) for v in values),
                'minimum_probability': str(min(values)),
                'probability_by_TR': [str(v) for v in values],
            }
        witnesses = {}
        for s in ('SPECIES_TENTACOOL','SPECIES_STARYU','SPECIES_CARVANHA','SPECIES_SHARPEDO'):
            observed = defaultdict(set)
            for record in model.records(species=s,maps=MAPS,methods='fishing_mons',rods='OLD_ROD'):
                if 'MOVE_SURF' in moveset(rows[s]['modes'][mode]['entries'],record['level']):
                    observed[record['map']+'/'+record['time']].add(record['TR'])
            witnesses[s] = {'learn_levels': rows[s]['modes'][mode]['learn_levels']['MOVE_SURF'],
                            'catch_level_window': ranges(rows[s]['modes'][mode]['windows']['MOVE_SURF']),
                            'TR_windows': {k:ranges(v) for k,v in observed.items()}}
        result['modes'][mode] = {'contexts':contexts,'handovers':witnesses,
                                 'gap_context_TR_count':sum(v['gap_count'] for v in contexts.values()),
                                 'total_context_TR_count':len(contexts)*81}
    lily_profiles = [p for p in profiles if p['map']=='MAP_LILYCOVE_CITY']
    for index,slot in enumerate(lily_profiles[0]['slots']):
        if slot[0] != 'SPECIES_MAGIKARP':
            continue
        for replacement in ('SPECIES_PSYDUCK','SPECIES_LOTAD','SPECIES_MARILL'):
            option = {'slot_index_zero_based':index, 'replace':'SPECIES_MAGIKARP',
                      'with':replacement, 'authored_level_range':list(slot[1:3]),'rods':{}}
            for p in lily_profiles:
                hypothetical = dict(p)
                hypothetical['slots'] = list(p['slots'])
                _,low,high,weight = hypothetical['slots'][index]
                hypothetical['slots'][index] = (replacement,low,high,weight)
                modes = {}
                for mode in ('modern','legacy'):
                    values = [surf_probability(hypothetical,tr,mode) for tr in range(81)]
                    modes[mode] = {'gaps':ranges(tr for tr,v in enumerate(values) if v==0),
                                   'gap_count':sum(v==0 for v in values),
                                   'TR0_probability':str(values[0]),'TR1_probability':str(values[1]),
                                   'minimum_probability':str(min(values)),
                                   'below_eight_percent':ranges(tr for tr,v in enumerate(values) if v<Fraction(2,25))}
                option['rods'][p['rod']] = {'raw_slot_weight':weight,'modes':modes}
            result['lilycove_optional_slot_replacements'].append(option)
    (HERE/'fixed_ports.json').write_text(json.dumps(result,indent=2)+'\n')
    print('sha256',result['proposal_sha256'])
    for mode,v in result['modes'].items():
        print(mode,'gap cases',v['gap_context_TR_count'],'/',v['total_context_TR_count'])
        print('gaps',{k:r['gaps'] for k,r in v['contexts'].items() if r['gap_count']})
        print('handovers',json.dumps(v['handovers']))
    print('patches',json.dumps(result['lilycove_optional_slot_replacements']))


if __name__=='__main__':
    main()
