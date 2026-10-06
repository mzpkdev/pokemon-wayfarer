"""Export the v2 native HM roster as roster_v2.json.

The v2 roster is the nearby-access roster (revisions/nearby-access/roster.json, modern mode) with the
hm_model.ROSTER_V2 changes applied: 12 added roles and 2 roles moved to new levels. A species new to the roster
takes its entries from the learnset hm_model parses (gen_7.h with IS_WAYFARER).

Usage: python3 export_roster.py     (rewrites roster_v2.json)
"""
import json, os
import hm_model as h

HERE = os.path.dirname(os.path.abspath(__file__))
BASE = json.load(open(os.path.join(HERE, '../nearby-access/roster.json')))
roster = {}
for r in BASE:
    m = r['modes']['modern']
    roster[r['species'][8:]] = dict(roles=list(r['roles']), entries=[list(e) for e in m['entries']],
                                    added={k: v for k, v in m['added'].items()}, change=None)

for sp, mv, lv, action in h.ROSTER_V2:
    move = 'MOVE_' + mv
    if sp not in roster:
        roster[sp] = dict(roles=[], entries=[[l, 'MOVE_' + m] for l, m in h.learnset_of(sp)], added={}, change=None)
    r = roster[sp]
    if action == 'move':
        assert move in r['added'], (sp, mv)  # only an added occurrence moves, never a native one
        r['entries'] = [e for e in r['entries'] if not (e[1] == move and e[0] == r['added'][move])]
    else:
        assert move not in r['roles'] and not any(e[1] == move for e in r['entries']), (sp, mv)
        r['roles'].append(move)
    i = 0
    while i < len(r['entries']) and r['entries'][i][0] <= lv: i += 1
    r['entries'].insert(i, [lv, move])
    r['added'][move] = lv
    r['change'] = {'action': action, 'move': move, 'level': lv}

out = []
for sp in sorted(roster):
    r = roster[sp]
    learn = {mv: sorted({l for l, m in r['entries'] if m == mv}) for mv in r['roles']}
    out.append(dict(species='SPECIES_' + sp, roles=r['roles'], entries=r['entries'], added=r['added'],
                    learn_levels=learn, change=r['change']))
doc = dict(revision='wild-encounters-v2', base='revisions/nearby-access/roster.json (modern mode)',
           species_count=len(out), role_count=sum(len(r['roles']) for r in out), roster=out)
json.dump(doc, open(os.path.join(HERE, 'roster_v2.json'), 'w'), indent=1)
print(doc['species_count'], 'species,', doc['role_count'], 'roles')
