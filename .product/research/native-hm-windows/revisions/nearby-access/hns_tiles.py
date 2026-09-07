"""Read-only HNS tile/component inspection, not a complete movement emulator."""
import json
import re
import struct
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[5] / 'game'
LAYOUTS = {x['id']: x for x in json.loads((ROOT/'data/layouts/layouts.json').read_text())['layouts']}
HEADERS = (ROOT/'src/data/tilesets/headers.h').read_text()
ATTRS = (ROOT/'src/data/tilesets/metatiles.h').read_text()
WATER = {16,17,18,19,20,21,80,81,82,83}
ENUM = {name: i for i,name in enumerate(re.findall(r'^\s*(MB_\w+)\s*[,=]',(ROOT/'include/constants/metatile_behaviors.h').read_text(), re.M))}
BEHAVIOR_SOURCE = (ROOT/'src/metatile_behavior.c').read_text()
BLOCKED = {}
for direction in ('North','South','East','West'):
    body = re.search(r'bool8 MetatileBehavior_Is'+direction+r'Blocked\(.*?\n\}',BEHAVIOR_SOURCE,re.S)[0]
    BLOCKED[direction] = {ENUM[n] for n in re.findall(r'== (MB_\w+)',body)}
DIRECTIONS = {(0,1):('South','North'),(0,-1):('North','South'),(1,0):('East','West'),(-1,0):('West','East')}

def load(name):
    m = json.loads((ROOT/'data/maps'/name/'map.json').read_text())
    l = LAYOUTS[m['layout']]
    tables = []
    for k in ('primary_tileset','secondary_tileset'):
        body = re.search(r'const struct Tileset '+l[k]+r'\s*=\s*\{(.*?)\};', HEADERS,re.S)[1]
        sym = re.search(r'\.metatileAttributes = (\w+)',body)[1]
        path = re.search(r'\b'+sym+r'\[\] = INCBIN_U16\("([^"]+)"\)',ATTRS)[1]
        b=(ROOT/path).read_bytes();tables.append(struct.unpack('<'+'H'*(len(b)//2),b))
    b=(ROOT/l['blockdata_filepath']).read_bytes();words=struct.unpack('<'+'H'*(len(b)//2),b)
    w=l['width'];h=l['height']; tiles={}
    for y in range(h):
        for x in range(w):
            a=words[y*w+x];i=a&1023
            tiles[x,y]={'collision':(a>>10)&3,'elevation':a>>12,'behavior':tables[i>=640][i-640 if i>=640 else i]&255}
    return m,l,tiles

def walk(name,start,blocked=()):
    m,l,t=load(name);blocked=set(blocked);seen={tuple(start)};q=deque(seen)
    while q:
        a=q.popleft()
        for d in ((0,1),(0,-1),(1,0),(-1,0)):
            p=(a[0]+d[0],a[1]+d[1]);v=t.get(p)
            if not v or p in seen or p in blocked or v['collision'] or v['behavior'] in WATER:continue
            forward,backward=DIRECTIONS[d]
            if t[a]['behavior'] in BLOCKED[forward] or v['behavior'] in BLOCKED[backward]:continue
            if v['elevation'] and t[a]['elevation'] and v['elevation']!=t[a]['elevation']:continue
            seen.add(p);q.append(p)
    banks=[]
    for a in seen:
        if t[a]['elevation']!=3:continue
        for dx,dy in ((0,1),(0,-1),(1,0),(-1,0)):
            p=a[0]+dx,a[1]+dy;v=t.get(p)
            forward,backward=DIRECTIONS[dx,dy]
            if v and v['behavior'] in WATER-{19} and not v['collision'] and v['elevation']!=3 and t[a]['behavior'] not in BLOCKED[forward] and v['behavior'] not in BLOCKED[backward]:banks.append((a,p))
    return {'start':start,'tiles':len(seen),'banks':sorted(banks),'warps':[(i,w['x'],w['y'])for i,w in enumerate(m['warp_events']) if (w['x'],w['y'])in seen], 'edges':{side:sorted(p for p in seen if p[axis]==value)for side,axis,value in [('left',0,0),('right',0,l['width']-1),('up',1,0),('down',1,l['height']-1)]},'behaviors':sorted({t[p]['behavior']for p in seen})}

if __name__=='__main__':
    import sys
    name=sys.argv[1]
    m,_,_=load(name)
    print(json.dumps(walk(name,(int(sys.argv[2]),int(sys.argv[3])),[(o['x'],o['y']) for o in m['object_events']]),indent=2))
