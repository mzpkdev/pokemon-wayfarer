import json, functools, sys
from fractions import Fraction as F
import hm_model as h, os
h.ROD_SHIFT = {'old': 0, 'good': 0, 'super': 0}  # wild level scaling: rods use the place level
if not os.environ.get('OLD_ROSTER'): h.apply_roster_v2()

h.moveset = functools.lru_cache(maxsize=None)(h.moveset)
_sd = functools.lru_cache(maxsize=None)(lambda cap, P, region: tuple(h.slot_dist(cap, P, region)))

def chance(slots, weights, P, region, move):
    tot = sum(weights); acc = F(0)
    for cap, w in zip(slots, weights):
        if cap == 'NONE': continue
        for s, L, p in _sd(cap, P, region):
            if move in h.moveset(s, L):
                acc += F(w, tot) * p
    return acc

T = h.load_tables()
TRS = range(0, 161)

def sources(mp):
    """Yield (method_label, slots_by_time, weights, level_shift)."""
    t = T[mp]
    for meth in ('land', 'surf', 'rock'):
        if meth in t:
            yield meth, t[meth], h.WEIGHTS[meth], 0
    if 'fish' in t:
        for rod in ('old', 'good', 'super'):
            yield 'fish_' + rod, t['fish'], h.RODS[rod], h.ROD_SHIFT[rod]

def intervals(xs):
    xs = sorted(xs); out = []
    for x in xs:
        if out and x == out[-1][1] + 1: out[-1][1] = x
        else: out.append([x, x])
    return ', '.join(f'{a}' if a == b else f'{a}-{b}' for a, b in out)

mode = sys.argv[1] if len(sys.argv) > 1 else 'all'
report = {}

if mode in ('all', 'regional'):
    regions = {}
    for mp in T:
        regions.setdefault(h.META[mp]['region'], []).append(mp)
    reg_out = {}
    for region, maps in sorted(regions.items()):
        for u in h.UTILS:
            gaps = []; best_by_tr = {}
            for tr in TRS:
                best = (F(0), None)
                for mp in maps:
                    P0 = h.place_level(mp, tr)
                    for meth, bytime, w, shift in sources(mp):
                        for tm, slots in bytime.items():
                            c = chance(slots, w, P0 + shift, region, u)
                            if c > best[0]: best = (c, (mp, meth, tm))
                if best[0] == 0: gaps.append(tr)
                best_by_tr[tr] = (float(best[0]), best[1])
            reg_out[f'{region}/{u}'] = {'gaps': intervals(gaps), 'n_gaps': len(gaps),
                                        'sample': {tr: best_by_tr[tr] for tr in (0, 40, 80, 120, 160)}}
            print(f'{region:7} {u:10} gaps: {intervals(gaps) or "none"}', flush=True)
    report['regional'] = reg_out

if mode in ('all', 'scenarios'):
    S = json.load(open(h.REPO + '.product/research/native-hm-windows/revisions/nearby-access/scenarios.json'))['scenarios']
    sc_out = {}
    for sc in S:
        move = sc['move']
        SC_TRS = range(20, 161)  # the crossing guarantee starts at two badges
        maxrank = 0 if sc['id'] in ('blackthorn_surf', 'den_whirlpool') else 1
        fails = {'DAY': [], 'NIGHT': []}; lows = {}; detail = {}
        for tm in ('DAY', 'NIGHT'):
            for tr in SC_TRS:
                best = (F(0), None); best_any = (F(0), None); extra = {}
                for src in sc['maps']:
                    mp = src['map'].replace('MAP_CINNABAR_ISLAND_HNS','MAP_CINNABAR_ISLAND')
                    if mp not in T: continue
                    region = h.META[mp]['region']; P0 = h.place_level(mp, tr)
                    for meth, bytime, w, shift in sources(mp):
                        base = 'fishing_mons' if meth.startswith('fish') else meth + '_mons'
                        if base not in src['methods']: continue
                        slots = bytime.get(tm.lower(), bytime.get('day'))
                        c = chance(slots, w, P0 + shift, region, move)
                        qualifies = meth in ('land', 'fish_old')
                        if qualifies and c > best_any[0]: best_any = (c, (mp, meth, src['rank']))
                        if qualifies and src['rank'] <= maxrank and c > best[0]: best = (c, (mp, meth, src['rank']))
                        if meth in ('fish_good', 'fish_super'):
                            extra[meth] = max(extra.get(meth, 0), float(c))
                if best[0] < F(8, 100): fails[tm].append(tr)
                detail.setdefault(tm, {})[tr] = (float(best[0]), best[1], float(best_any[0]), best_any[1], extra)
        sc_out[sc['id']] = {'fails': {k: intervals(v) for k, v in fails.items()}, 'detail': detail}
        print(f"{sc['id']:20} {move:9} fail DAY: {intervals(fails['DAY']) or 'none'} | NIGHT: {intervals(fails['NIGHT']) or 'none'}", flush=True)
    report['scenarios'] = sc_out

print('missing evolution levels:', sorted(h.MISSING_EDGES))
json.dump(report, open(os.path.join(os.path.dirname(os.path.abspath(__file__)), f'audit_{mode}.json'), 'w'), default=str)
