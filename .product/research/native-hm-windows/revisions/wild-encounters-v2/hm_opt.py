"""Greedy search for the fewest crossing-table edits meeting the native HM floor.

Floor: one rank-allowed source (land, or Old Rod fishing) with an 8% chance of a
catch knowing the move, at every TR 20-160, day and night separately.
Writes hm_edits_new.json: [{map, method, time, index, from, to}].
"""
import json, functools, copy, os, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../../../game/tools/wild_encounters/v2'))  # check.py and hm_model.py moved there
import hm_model as h
h.apply_roster_v2()
h.ROD_SHIFT = {'old': 0, 'good': 0, 'super': 0}

FLOOR = 0.08
TRS = range(20, 161)
TIMES = ('day', 'night')
REPO = h.REPO
S = json.load(open(REPO + '.product/research/native-hm-windows/revisions/nearby-access/scenarios.json'))['scenarios']
S += json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'scenarios_regions.json')))['scenarios']
for sc in S:
    for m in sc['maps']:
        m['map'] = m['map'].replace('MAP_CINNABAR_ISLAND_HNS', 'MAP_CINNABAR_ISLAND')

FILES = {'Kanto': 'kanto.json', 'Johto': 'johto.json', 'Hoenn': 'hoenn.json', 'Sevii': 'sevii.json', 'Alola': 'alola.json', 'Sinjoh': 'sinjoh.json'}
DATA = {r: json.load(open(h.HERE + '/' + f)) for r, f in FILES.items()}
def table(mp):
    return DATA[h.META[mp]['region']][mp]

POOLS = {
    ('Kanto', 'sea'): ['TENTACRUEL', 'KINGLER', 'SEADRA', 'SHELLDER', 'SLOWBRO', 'LANTURN', 'GOLDUCK', 'QWILFISH', 'GYARADOS'],
    ('Johto', 'sea'): ['TENTACRUEL', 'KINGLER', 'SEADRA', 'SHELLDER', 'SLOWBRO', 'LANTURN', 'GOLDUCK', 'QWILFISH', 'GYARADOS'],
    ('Kanto', 'pond'): ['GOLDUCK', 'AZUMARILL', 'POLIWHIRL', 'QUAGSIRE', 'SLOWBRO', 'GYARADOS'],
    ('Johto', 'pond'): ['GOLDUCK', 'AZUMARILL', 'POLIWHIRL', 'QUAGSIRE', 'SLOWBRO', 'GYARADOS'],
    ('Johto', 'cave'): ['GOLDUCK', 'POLIWHIRL', 'QUAGSIRE', 'SLOWBRO', 'GYARADOS', 'AZUMARILL'],
    ('Hoenn', 'sea'): ['SHARPEDO', 'LUVDISC', 'JELLICENT', 'FRILLISH', 'CLAMPERL', 'WAILORD'],
    ('Hoenn', 'pond'): ['AZUMARILL', 'LOMBRE'],
    ('Sevii', 'sea'): ['DREDNAW', 'BARRASKEWDA', 'KINGLER', 'TENTACRUEL', 'SEADRA', 'SHELLDER'],
    ('Sevii', 'pond'): ['DREDNAW', 'BARRASKEWDA', 'GOLDUCK', 'AZUMARILL', 'POLIWHIRL', 'QUAGSIRE'],
    ('Sevii', 'cold'): ['BARRASKEWDA', 'SHELLDER', 'WALREIN'],
    ('Alola', 'sea'): ['TOXAPEX', 'WISHIWASHI', 'LUVDISC', 'SHELLDER', 'SHARPEDO'],
    ('Alola', 'cave'): ['WISHIWASHI', 'GOLDUCK'],
}
LAND_POOLS = {'MAP_SNOWSWEPT_CAVERN_HNS': ['GRAVELER', 'URSARING'], 'MAP_ROUTE49_HNS': ['URSARING', 'GRAVELER'], 'MAP_ROUTE50_HNS': ['URSARING', 'GRAVELER'], 'MAP_SINJOH_RUINS_HNS': ['GRAVELER'], 'MAP_NEWSINJOH_HOTSPRINGS_HNS': ['GRAVELER', 'URSARING'], 'MAP_MELEMELE_ISLE_HNS': ['PELIPPER'], 'MAP_AKALA_ISLE_HNS': ['PELIPPER'], 'MAP_ULAULA_ISLE_HNS': ['PELIPPER'], 'MAP_PONI_ISLE_HNS': ['PELIPPER'], 'MAP_ROUTE121': ['PELIPPER'], 'MAP_ROUTE118': ['PELIPPER'], 'MAP_ROUTE117': ['AZUMARILL', 'LOMBRE'],
              'MAP_ROUTE119': ['PELIPPER', 'LOMBRE'], 'MAP_CLIFF_EDGE_CAVE_HNS': ['KINGLER', 'SLOWBRO']}
LAND_W = h.WEIGHTS['land']; OLD_W = h.RODS['old']

@functools.lru_cache(maxsize=None)
def pk(cap, P, region, move):
    if cap == 'NONE': return 0.0
    return float(sum(p for s, L, p in h.slot_dist(cap, P, region) if move in h.moveset(s, L)))

def src_chance(mp, meth, tm, tr, move):
    t = table(mp); region = h.META[mp]['region']; P = h.place_level(mp, tr)
    if meth == 'land':
        slots, w = t['land'][tm], LAND_W
    else:
        slots, w = t['fish'][tm], OLD_W
    tot = sum(w)
    return sum(wi * pk(c, P, region, move) for c, wi in zip(slots, w)) / tot

def maxrank(sc):
    return 0 if sc['id'] in ('blackthorn_surf', 'den_whirlpool') else 1

def sources(sc):
    for m in sc['maps']:
        if m['rank'] > maxrank(sc) or m['map'] not in h.META: continue
        for meth in m['methods']:
            k = 'land' if meth == 'land_mons' else 'fish' if meth == 'fishing_mons' else None
            if k and k in table(m['map']): yield m['map'], k

def failures():
    out = set()
    for sc in S:
        srcs = list(sources(sc))
        for tm in TIMES:
            for tr in TRS:
                if max((src_chance(mp, k, tm, tr, sc['move']) for mp, k in srcs), default=0) < FLOOR:
                    out.add((sc['id'], tm, tr))
    return out

def candidates():
    seen = set()
    for sc in S:
        for mp, k in sources(sc):
            t = table(mp); region = h.META[mp]['region']
            pool = LAND_POOLS.get(mp, []) if k == 'land' else POOLS.get((region, t.get('water_type')), [])
            idxs = range(6) if k == 'land' else range(4)
            for tm in TIMES:
                for i in idxs:
                    for r in pool:
                        key = (mp, k, tm, i, r)
                        if key in seen or t[k][tm][i] == r: continue
                        seen.add(key); yield key

def main():
    global fails
    edits = []
    fails = failures()
    print('initial failures', len(fails))
    while fails:
        best = None
        for mp, k, tm, i, r in candidates():
            t = table(mp); old = t[k][tm][i]
            t[k][tm][i] = r
            f2 = failures()
            t[k][tm][i] = old
            gain = len(fails) - len(f2)
            if gain > 0 and (best is None or gain > best[0] or (gain == best[0] and i > best[1][3])):
                best = (gain, (mp, k, tm, i, r), f2)
        if not best:
            print('stuck with', len(fails), 'failures'); break
        gain, (mp, k, tm, i, r), fails = best
        t = table(mp)
        edits.append({'map': mp, 'method': k, 'time': tm, 'index': i, 'from': t[k][tm][i], 'to': r})
        t[k][tm][i] = r
        print(f'edit {mp} {k} {tm} [{i}] {edits[-1]["from"]} -> {r}  (+{gain}, left {len(fails)})', flush=True)

    json.dump(edits, open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'hm_edits_new.json'), 'w'), indent=1)
    by = {}
    for sc, tm, tr in sorted(fails):
        by.setdefault((sc, tm), []).append(tr)
    for k, v in by.items(): print('UNRESOLVED', k, v[0], '-', v[-1], len(v))


if __name__ == '__main__':
    main()
