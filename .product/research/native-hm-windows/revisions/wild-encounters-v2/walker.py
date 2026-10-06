"""Walk the overworld on foot, as the engine allows, without field moves.

Tiles come from source layouts (collision, elevation, metatile behaviour);
movement follows the engine's rules closely enough for route evidence:
- a tile is passable when its collision bits are 0, it isn't surfable water or
  a waterfall, and no obstacle object (Cut tree, Rock Smash rock, Strength
  boulder, whirlpool) stands on it, unless the field move is allowed;
- elevation: a move is blocked when the walker's elevation and the tile's are
  both set (not 0 or 15) and differ; the walker takes the tile's elevation
  unless it is 15;
- ledges (MB_JUMP_*) are jumped in their direction only, landing two tiles on;
- map connections join edges with their offsets; warp tiles lead to their
  destination warp.
Other people are treated as passable: none of them permanently blocks a route
in the areas audited, and story blockers are recorded by hand.
"""
import json, re, glob, struct, collections, functools, os

G = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../../../game')) + '/'

def enum_values(text):
    val, out = -1, {}
    for line in text.split('\n'):
        line = re.sub(r'//.*', '', line).strip().rstrip(',')
        if not line or line.startswith('#') or line.startswith('enum') or line in ('{', '};'): continue
        if '=' in line:
            n, v = [x.strip() for x in line.split('=', 1)]
            try: val = int(v, 0)
            except ValueError: val = out[v]
        else:
            n = line; val += 1
        out[n] = val
    return out

mbh = open(G + 'include/constants/metatile_behaviors.h').read()
MB = enum_values(mbh[mbh.index('enum {'):mbh.index('};') + 2])
src = open(G + 'src/metatile_behavior.c').read()
flags = dict(re.findall(r'#define (TILE_FLAG_\w+)\s+\(1 << (\d+)\)', src))
ENC = 1 << int(flags['TILE_FLAG_HAS_ENCOUNTERS']); SURF = 1 << int(flags['TILE_FLAG_SURFABLE'])
tba = {}
for name, fl in re.findall(r'\[(MB_\w+)\]\s*=\s*([^,\n]+),', src[src.index('sTileBitAttributes'):src.index('};', src.index('sTileBitAttributes'))]):
    v = 0
    for f in fl.split('|'):
        f = f.strip(); v |= 1 << int(flags[f]) if f in flags else 0
    tba[MB[name]] = v
fm = open(G + 'src/fieldmap.c').read()
norm = {MB[a]: MB[b] for a, b in re.findall(r'case (MB_FRLG_\w+):\s*return (MB_\w+);', fm) if a in MB and b in MB}
hdr = open(G + 'src/data/tilesets/headers.h').read()
attr_sym = dict(re.findall(r'const struct Tileset (gTileset_\w+)\s*=\s*\{.*?\.metatileAttributes\s*=\s*(\w+)', hdr, re.S))
mt = open(G + 'src/data/tilesets/metatiles.h').read()
attr_path = {n: (p, w == '32') for n, w, p in re.findall(r'(gMetatileAttributes_\w+)\[\]\s*=\s*INCBIN_U(16|32)\("([^"]+)"\)', mt)}
LAYOUTS = {l['id']: l for l in json.load(open(G + 'data/layouts/layouts.json'))['layouts'] if 'id' in l}
MAPS = {}
for f in glob.glob(G + 'data/maps/*/map.json'):
    d = json.load(open(f)); MAPS[d['id']] = d

JUMP = {MB['MB_JUMP_EAST']: (1, 0), MB['MB_JUMP_WEST']: (-1, 0), MB['MB_JUMP_NORTH']: (0, -1), MB['MB_JUMP_SOUTH']: (0, 1)}
WATERFALL = MB['MB_WATERFALL']
OBSTACLE = {'CUT': ('CUT_TREE', 'EventScript_CutTree'), 'ROCK_SMASH': ('BREAKABLE_ROCK', 'EventScript_RockSmash'),
            'STRENGTH': ('PUSHABLE_BOULDER', 'EventScript_StrengthBoulder'), 'WHIRLPOOL': ('WHIRLPOOL', 'EventScript_Whirlpool')}

BAD_ATTRS = set()

@functools.lru_cache(maxsize=None)
def attrs(ts, frlg):
    # FRLG layouts read 32-bit attributes (fieldmap.c), whatever the INCBIN declares.
    path, _ = attr_path[attr_sym[ts]]
    data = open(G + path, 'rb').read()
    u32 = frlg and len(data) % 4 == 0
    if frlg and not u32: BAD_ATTRS.add(ts)
    return struct.unpack('<%d%s' % (len(data) // (4 if u32 else 2), 'I' if u32 else 'H'), data)

@functools.lru_cache(maxsize=None)
def grid(mid):
    d = MAPS[mid]; L = LAYOUTS[d['layout']]; ver = L.get('layout_version', 'emerald')
    frlg = ver == 'frlg'; nprim = 512 if ver == 'emerald' else 640
    prim, sec = attrs(L['primary_tileset'], frlg), attrs(L['secondary_tileset'], frlg)
    w, h = L['width'], L['height']; cells = []
    for (b,) in struct.iter_unpack('<H', open(G + L['blockdata_filepath'], 'rb').read()):
        m = b & 0x3FF; coll = (b >> 10) & 3; elev = b >> 12
        a = prim[m] if m < nprim else (sec[m - nprim] if m - nprim < len(sec) else 0)
        beh = a & (0x1FF if frlg else 0xFF)
        if frlg: beh = norm.get(beh, beh)
        f = tba.get(beh & 0xFF, 0) if beh < 256 else 0
        cells.append((coll, elev, beh, f))
    return w, h, cells

def obstacles(mid, allowed):
    out = set()
    for o in MAPS[mid].get('object_events', []):
        for mv, (gfx, script) in OBSTACLE.items():
            if mv in allowed: continue
            if gfx in o.get('graphics_id', '') or o.get('script') == script:
                out.add((o['x'], o['y']))
    return out

def warps(mid):
    out = {}
    for w in MAPS[mid].get('warp_events', []):
        dm, di = w['dest_map'], w['dest_warp_id']
        if dm not in MAPS or not str(di).isdigit(): continue
        dw = MAPS[dm].get('warp_events', [])
        if int(di) < len(dw): out[(w['x'], w['y'])] = (dm, dw[int(di)]['x'], dw[int(di)]['y'])
    return out

def neighbour(mid, x, y, dx, dy):
    """Step off an edge through a connection; None if there is none."""
    w, h, _ = grid(mid)
    if 0 <= x + dx < w and 0 <= y + dy < h: return mid, x + dx, y + dy
    for c in MAPS[mid].get('connections') or []:
        cm, off, d = c['map'], c['offset'], c['direction']
        if cm not in MAPS: continue
        cw, ch, _ = grid(cm)
        if d == 'up' and y + dy < 0: return cm, x - off, ch - 1
        if d == 'down' and y + dy >= h: return cm, x - off, 0
        if d == 'left' and x + dx < 0: return cm, cw - 1, y - off
        if d == 'right' and x + dx >= w: return cm, 0, y - off
    return None

def walk(starts, allowed=(), surf=False, region_maps=None):
    """BFS from [(map, x, y)]. Returns {map: set of (x, y)} reached on foot (or by Surf if allowed)."""
    seen = set(); q = collections.deque(); reached = collections.defaultdict(set)
    obst = {}; wcache = {}
    for m, x, y in starts:
        _, _, cells = grid(m); e = cells[y * grid(m)[0] + x][1]
        q.append((m, x, y, 0 if e == 15 else e, True))
    while q:
        m, x, y, e, via_warp = q.popleft()
        if (m, x, y, e, via_warp) in seen: continue
        seen.add((m, x, y, e, via_warp)); reached[m].add((x, y))
        if m not in wcache: wcache[m] = warps(m); obst[m] = obstacles(m, allowed)
        if (x, y) in wcache[m] and not via_warp:
            dm, dx_, dy_ = wcache[m][(x, y)]
            if region_maps is None or dm in region_maps:
                _, _, dc = grid(dm); de = dc[dy_ * grid(dm)[0] + dx_][1]
                q.append((dm, dx_, dy_, 0 if de == 15 else de, True))
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nb = neighbour(m, x, y, dx, dy)
            if not nb: continue
            nm, nx, ny = nb
            if region_maps is not None and nm not in region_maps: continue
            nw, nh, nc = grid(nm)
            if not (0 <= nx < nw and 0 <= ny < nh): continue
            if nm not in wcache: wcache[nm] = warps(nm); obst[nm] = obstacles(nm, allowed)
            coll, te, beh, f = nc[ny * nw + nx]
            if beh in JUMP and JUMP[beh] == (dx, dy):
                nb2 = neighbour(nm, nx, ny, dx, dy)
                if nb2:
                    lm, lx, ly = nb2; lw, lh, lc = grid(lm)
                    if 0 <= lx < lw and 0 <= ly < lh and lc[ly * lw + lx][0] == 0:
                        le = lc[ly * lw + lx][1]
                        q.append((lm, lx, ly, e if le == 15 else le, False))
                continue
            is_warp = (nx, ny) in wcache[nm]
            water = bool(f & SURF)
            if (nx, ny) in obst[nm]: continue
            if not is_warp:
                if coll != 0: continue
                if beh == WATERFALL: continue
                if water and not surf: continue
                if beh in JUMP: continue
            if e not in (0, 15) and te not in (0, 15) and te != e and not water: continue
            ne = e if te == 15 else te
            q.append((nm, nx, ny, ne, False))
    return reached

def sources(reached):
    """Land and shore-fishing sources reached: {map: {'land', 'fish'}}."""
    out = collections.defaultdict(set)
    for m, cells in reached.items():
        w, h, c = grid(m)
        for x, y in cells:
            coll, e, beh, f = c[y * w + x]
            if f & SURF: continue
            if f & ENC: out[m].add('land')
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nb = neighbour(m, x, y, dx, dy)
                if not nb: continue
                nm, nx, ny = nb; nw, nh, nc = grid(nm)
                if 0 <= nx < nw and 0 <= ny < nh and nc[ny * nw + nx][3] & SURF and nc[ny * nw + nx][3] & ENC:
                    out[nm].add('fish')
    return out
