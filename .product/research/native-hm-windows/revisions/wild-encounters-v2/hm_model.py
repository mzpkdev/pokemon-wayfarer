"""Wild encounters v2 model for the native HM audit.

Levels follow specs/wild-level-scaling.md; movesets follow the production
initial-moveset algorithm over game/src/data/pokemon/level_up_learnsets/gen_7.h
with IS_WAYFARER enabled.
"""
import json, re, os, glob
from fractions import Fraction as F

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../../..')) + '/'
G = REPO + 'game/'
HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data')
UTILS = ['CUT', 'FLASH', 'STRENGTH', 'ROCK_SMASH', 'SURF', 'WATERFALL', 'WHIRLPOOL', 'DIVE']

# ---------- learnsets ----------
def parse_learnsets():
    text = open(G + 'src/data/pokemon/level_up_learnsets/gen_7.h').read().split('\n')
    sets, cur, stack = {}, None, []
    for line in text:
        s = line.strip()
        if s.startswith('#if'):
            cond = s[3:].strip()
            stack.append(not ('!IS_WAYFARER' in cond))
            continue
        if s.startswith('#endif'):
            stack.pop(); continue
        if s.startswith('#else') or s.startswith('#elif'):
            raise SystemExit('unexpected ' + s)
        if not all(stack):
            continue
        m = re.match(r'static const struct LevelUpMove (s\w+)\[\]', s)
        if m:
            cur = m.group(1); sets[cur] = []; continue
        m = re.match(r'LEVEL_UP_MOVE\(\s*(\d+),\s*MOVE_(\w+)\)', s)
        if m and cur:
            sets[cur].append((int(m.group(1)), m.group(2)))
    return sets

def parse_species_learnsets():
    out = {}
    for f in sorted(glob.glob(G + 'src/data/pokemon/species_info/gen_*_families.h')):
        cur = None
        for line in open(f):
            m = re.match(r'\s*\[SPECIES_(\w+)\]\s*=', line)
            if m: cur = m.group(1)
            m = re.search(r'\.levelUpLearnset = (s\w+)', line)
            if m and cur and cur not in out: out[cur] = m.group(1)
    return out

LEARNSETS = parse_learnsets()
SPECIES_LS = parse_species_learnsets()

def learnset_of(species):
    if species in SPECIES_LS: return LEARNSETS[SPECIES_LS[species]]
    for k in sorted(SPECIES_LS):
        if k.startswith(species + '_'): return LEARNSETS[SPECIES_LS[k]]
    parts = species.split('_')
    for n in range(len(parts), 0, -1):
        name = 's' + ''.join(x.capitalize() for x in parts[:n]) + 'LevelUpLearnset'
        if name in LEARNSETS: return LEARNSETS[name]
    raise KeyError(species)

def moveset(species, level):
    ls = learnset_of(species)
    cur = []
    for lv, mv in ls:
        if lv == 0 or lv > level: continue
        if mv in cur: continue
        cur.append(mv)
        if len(cur) > 4: cur.pop(0)
    return cur

# ---------- species / evolution ----------
SP = json.load(open(HERE + '/species.json'))
SPECIES = SP['species']
EVO = json.load(open(G + 'tools/notable_trainers/evolution.json'))
BABIES = set(SP['babies']) | {b.upper().replace(' ', '_').replace('.', '').replace("'", '').replace('-', '_') for b in EVO['babies']}
NONLEVEL = {(e['predecessor'][8:], e['successor'][8:]): e['level'] for e in EVO['nonLevelEdges']}
def _n(x):
    return {'Nidoran♀': 'NIDORAN_F', 'Nidoran♂': 'NIDORAN_M'}.get(x, x.upper().replace(' ', '_').replace('.', '').replace("'", '').replace('-', '_'))
for _chain in EVO['chains'].values():
    for _i in range(0, len(_chain) - 2, 2):
        NONLEVEL.setdefault((_n(_chain[_i]), _n(_chain[_i + 2])), _chain[_i + 1])
PRED, EVOLV, MISSING_EDGES = {}, {}, set()
for s, info in SPECIES.items():
    for e in info['evolves_to']:
        t = e['target']
        p = str(e['param'])
        # Conditional evolutions (friendship, a known move, a held item...) are stored as
        # LEVEL 0 with conditions; like other non-level edges they use the shared table.
        if e['method'].startswith('LEVEL') and p.isdigit() and int(p) > 0:
            lv = int(p)
        else:
            lv = NONLEVEL.get((s, t))
        EVOLV.setdefault(s, []).append((t, lv, e['method']))
        PRED.setdefault(t, []).append((s, lv, e['method']))

def evo_level(pred, succ):
    for t, lv, m in EVOLV.get(pred, []):
        if t == succ:
            if lv is None: MISSING_EDGES.add((pred, succ, m))
            return lv
    return None

def predecessor(s):
    p = [x for x in PRED.get(s, []) if x[0] not in BABIES]
    if not p: return None
    return p[0][0]

def young_limit(cap):
    if cap in BABIES: return 10
    lvls = []
    for t, lv, m in EVOLV.get(cap, []):
        if lv is None:
            MISSING_EDGES.add((cap, t, m)); continue
        lvls.append(lv)
    return min(lvls) - 1 if lvls else None

def stage_outcomes(cap, L):
    """[(species, prob)] for slot cap at level L: downward rule plus stage mix."""
    s = cap
    while True:
        p = predecessor(s)
        if p is None: break
        e = evo_level(p, s)
        if e is None or L >= e: break
        s = p
    p = predecessor(s)
    if p is None: return [(s, F(1))]
    e = evo_level(p, s)
    if e is None: return [(s, F(1))]
    keep = min(F(1), F(L - e + 1, 10))
    if keep >= 1: return [(s, F(1))]
    return [(s, keep), (p, 1 - keep)]

# ---------- prowlers ----------
def parse_prowlers():
    rows = {}
    for l in open(REPO + '.product/specs/prowlers.md'):
        m = re.match(r'\| (.+?) \| .+? \| (Harmless|Fierce|Dangerous) \| (\d+|—) \|', l)
        if not m: continue
        name, temp, bst = m.group(1), m.group(2), m.group(3)
        if temp == 'Harmless': mn = None
        elif temp == 'Dangerous': mn = 30
        else: mn = 25 if bst.isdigit() and int(bst) >= 485 else 20
        key = name.replace(' line', '')
        rows[key] = mn
    return rows
PROWLER_ROWS = parse_prowlers()

def norm(n):
    return n.upper().replace(' ', '_').replace('.', '').replace("'", '').replace('-', '_').replace('♀', '_F').replace('♂', '_M')

def root(s):
    while True:
        p = PRED.get(s)
        if not p: return s
        s = p[0][0]

PROWLER_MIN = {}
for name, mn in PROWLER_ROWS.items():
    m = re.match(r'(.+) \((Alolan|Galarian|Hisuian|White-Striped)\)', name)
    if m:
        suf = {'Alolan': '_ALOLA', 'Galarian': '_GALAR', 'Hisuian': '_HISUI', 'White-Striped': '_WHITE_STRIPED'}[m.group(2)]
        PROWLER_MIN[norm(m.group(1)) + suf] = mn
    else:
        PROWLER_MIN[norm(name)] = mn

def prowler_min(cap, region):
    # Kalos rewards in the Safari Zones and Sinjoh's residents have no minimum.
    r = root(cap)
    mn = PROWLER_MIN.get(cap, PROWLER_MIN.get(r))
    if mn is None: return None
    if region == 'Safari' and SPECIES.get(r, {}).get('nat', 0) and 650 <= SPECIES[r]['nat'] <= 721: return None
    if region == 'Sinjoh' and (cap.endswith('_HISUI') or r in ('STANTLER', 'SCYTHER', 'BASCULIN_WHITE_STRIPED') or r.endswith('_HISUI')): return None
    return mn

# ---------- levels ----------
def sc(a, tr):
    for (t0, v0), (t1, v1) in zip(a, a[1:]):
        if tr <= t1:
            d = t1 - t0
            return v0 + (2 * (tr - t0) * (v1 - v0) + d) // (2 * d)
    return a[-1][1]
ROAD = [(0, 5), (40, 20), (80, 38), (120, 56), (160, 74)]
WB = [(0, 4), (80, 6), (160, 11)]
OB = [(0, 8), (40, 8), (80, 12), (160, 24)]

def reach_levels(tr):
    R = sc(ROAD, tr)
    return R, R + sc(WB, tr), R + sc(OB, tr)

def parse_intents():
    out = {}
    for l in open(REPO + '.product/specs/reach-assignments.md'):
        cells = [c.strip() for c in l.split('|')]
        if len(cells) < 5 or cells[2] != 'Dungeon': continue
        notes = cells[4]
        for it in ['Mild to moderate', 'Moderate to hard', 'Brutal', 'Hard', 'Moderate', 'Mild']:
            if re.search(r'\b' + it + r'\b', notes, re.I):
                flat = bool(re.search(r'\bflat\b', notes, re.I)) or 'Single floor' in notes
                maps = re.findall(r'`(MAP_\w+)`', cells[3])
                for mp in maps: out[mp] = (it, flat)
                break
    return out
INTENTS = parse_intents()
META = json.load(open(HERE + '/meta.json'))
FLOOR_FIX = {'MAP_MT_SILVER_MOUNTAIN_SIDE_HNS': 2, 'MAP_MT_SILVER_1F_ITEM_ROOM_HNS': 2, 'MAP_MT_SILVER_1F_MOLTRES_ROOM_HNS': 2,
             'MAP_MT_SILVER_2F_HNS': 3, 'MAP_MT_SILVER_SNOW_HNS': 4, 'MAP_MT_SILVER_3F_HNS': 5}

def place_level(mp, tr):
    m = META[mp]; R, W, O = reach_levels(tr)
    if m['reach'] == 'Road': return R
    if m['reach'] == 'Wilds': return W
    if m['reach'] == 'Outlands': return O
    it, flat = INTENTS[mp]
    h = lambda a, b: (a + b) // 2
    first, last, floor = {
        'Mild': (h(R, W), W, None), 'Mild to moderate': (h(R, W), h(W, O), None),
        'Moderate': (W, h(W, O), None), 'Moderate to hard': (W, O, None),
        'Hard': (h(W, O), O + 3, None), 'Brutal': (O, O + 6, 50)}[it]
    i, n = m['floor'], m['floors']
    if mp in FLOOR_FIX: i, n = FLOOR_FIX[mp], 6
    if flat or not n or n == 1:
        lv = (first + last) // 2
    else:
        lv = first + (2 * (last - first) * i + (n - 1)) // (2 * (n - 1))
    if floor: lv = max(lv, floor)
    return min(lv, 100)

WEIGHTS = {'land': [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1], 'surf': [60, 30, 5, 4, 1], 'rock': [60, 30, 5, 4, 1]}
RODS = {'old': [38, 22, 10, 8, 8, 4, 3, 3, 2, 2], 'good': [25, 18, 12, 10, 9, 7, 6, 5, 4, 4], 'super': [12, 10, 11, 10, 10, 10, 10, 9, 9, 9]}
ROD_SHIFT = {'old': -4, 'good': -2, 'super': 0}

def slot_dist(cap, P, region):
    """[(species, level, prob)] for one slot at place level P (after rod shift)."""
    out = []
    pmin = prowler_min(cap, region)
    yl = young_limit(cap)
    for d in range(-2, 3):
        L = P + d
        if pmin: L = max(L, pmin)
        if yl is not None:
            L = min(L, yl)
            if pmin: L = max(L, pmin)
        L = max(1, min(100, L))
        for s, p in stage_outcomes(cap, L):
            out.append((s, L, p / 5))
    return out

def carrier_chance(slots, weights, P, region, move):
    tot = sum(weights); acc = F(0); best = None
    for cap, w in zip(slots, weights):
        if cap == 'NONE': continue
        for s, L, p in slot_dist(cap, P, region):
            if move in moveset(s, L):
                acc += F(w, tot) * p
    return acc

def witnesses(slots, weights, P, region, move):
    seen = {}
    for cap, w in zip(slots, weights):
        for s, L, p in slot_dist(cap, P, region):
            if move in moveset(s, L):
                seen.setdefault(s, set()).add(L)
    return {s: (min(v), max(v)) for s, v in seen.items()}

def load_tables():
    T = {}
    for f in ['kanto', 'johto', 'hoenn', 'alola', 'sevii', 'safari', 'sinjoh']:
        for mp, t in json.load(open(HERE + '/' + f + '.json')).items():
            T[mp] = t
    return T

# ---------- v2 roster revision ----------
ROSTER_V2 = [  # (species, move, level, action)
    ('LUVDISC', 'SURF', 11, 'add'),
    ('FRILLISH', 'SURF', 27, 'add'),
    ('JELLICENT', 'SURF', 45, 'add'),
    ('QUAGSIRE', 'SURF', 28, 'move'),
    # Sevii (Galar) and Alola
    ('ARROKUDA', 'SURF', 11, 'add'),
    ('BARRASKEWDA', 'SURF', 34, 'add'),
    ('DREDNAW', 'SURF', 22, 'add'),
    ('CHEWTLE', 'WATERFALL', 5, 'add'),
    ('WEEPINBELL', 'CUT', 20, 'move'),
    ('MAREANIE', 'SURF', 11, 'add'),
    ('WISHIWASHI', 'SURF', 22, 'add'),
    ('TOXAPEX', 'SURF', 38, 'add'),
    # Hoenn: leaving Sootopolis
    ('CARVANHA', 'DIVE', 12, 'add'),
    # Sinjoh: the Snowswept Cavern rocks
    ('GRAVELER', 'ROCK_SMASH', 26, 'add'),
]

if os.environ.get('EXTRA_ROSTER'):
    for _e in os.environ['EXTRA_ROSTER'].split(';'):
        _sp, _mv, _lv, _act = _e.split(':'); ROSTER_V2.append((_sp, _mv, int(_lv), _act))

def apply_roster_v2():
    for sp, mv, lv, action in ROSTER_V2:
        name = SPECIES_LS.get(sp) or 's' + ''.join(x.capitalize() for x in sp.split('_')) + 'LevelUpLearnset'
        ls = [e for e in LEARNSETS[name] if not (action == 'move' and e[1] == mv)]
        if action == 'add' and any(m == mv for _, m in ls):
            raise SystemExit(f'{sp} already knows {mv}')
        i = 0
        while i < len(ls) and ls[i][0] <= lv: i += 1
        ls.insert(i, (lv, mv))
        LEARNSETS[name] = ls
