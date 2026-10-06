"""Count land-encounter, water-encounter and Headbutt-tree tiles and Rock Smash rocks on every encounter map,
from source layouts, following the engine: fieldmap.c attribute decoding and metatile_behavior.c sTileBitAttributes."""
import json, re, glob, struct, sys
import os
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
ENC, SURF = 1 << 0, None
flags = dict(re.findall(r'#define (TILE_FLAG_\w+)\s+\(1 << (\d+)\)', src))
SURF = 1 << int(flags['TILE_FLAG_SURFABLE'])
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
attr_path = dict(re.findall(r'(gMetatileAttributes_\w+)\[\]\s*=\s*INCBIN_U(?:16|32)\("([^"]+)"\)', mt))
layouts = {l['id']: l for l in json.load(open(G + 'data/layouts/layouts.json'))['layouts'] if 'id' in l}
maps, SCRIPTS = {}, {}
for f in glob.glob(G + 'data/maps/*/map.json'):
    d = json.load(open(f)); maps[d['id']] = d
    sp = f.replace('map.json', 'scripts.inc')
    try: SCRIPTS[d['id']] = open(sp).read()
    except FileNotFoundError: pass
def attrs(ts, u32):
    data = open(G + attr_path[attr_sym[ts]], 'rb').read()
    return struct.unpack('<%d%s' % (len(data) // (4 if u32 else 2), 'I' if u32 else 'H'), data)
out = {}
for mid in json.load(open(sys.argv[1])):
    d = maps[mid]; land = water = trees = 0
    # A map's scripts may swap in other layouts (tides, Mirage Island, ...): count the most of each across them
    alts = [d['layout']] + sorted(set(re.findall(r'setmaplayoutindex (LAYOUT_\w+)', SCRIPTS.get(mid, ''))) - {d['layout']})
    best = {}
    for lid in alts:
      L = layouts[lid]; ver = L.get('layout_version', 'emerald')
      frlg = ver == 'frlg'; nprim = 512 if ver == 'emerald' else 640
      prim, sec = attrs(L['primary_tileset'], frlg), attrs(L['secondary_tileset'], frlg)
      land = water = trees = 0
      for (b,) in struct.iter_unpack('<H', open(G + L['blockdata_filepath'], 'rb').read()):
        m = b & 0x3FF
        a = prim[m] if m < nprim else (sec[m - nprim] if m - nprim < len(sec) else 0)
        beh = a & (0x1FF if frlg else 0xFF)
        if frlg: beh = norm.get(beh, beh)
        f = tba.get(beh & 0xFF, 0) if beh < 256 else 0
        if f & ENC: water += bool(f & SURF); land += not (f & SURF)
        if ver == 'hns' and beh == MB['MB_HEADBUTT_TREE']: trees += 1
      for k, v in (('land', land), ('water', water), ('trees', trees)): best[k] = max(best.get(k, 0), v)
    land, water, trees = best['land'], best['water'], best['trees']
    rocks = sum(1 for o in d.get('object_events', []) if 'BREAKABLE_ROCK' in o.get('graphics_id', '') or o.get('script') == 'EventScript_RockSmash')
    out[mid] = dict(layouts=alts, ver=ver, land=land, water=water, trees=trees, rocks=rocks)
json.dump(out, open(sys.argv[2], 'w'), indent=1)
print(len(out), 'maps scanned')
