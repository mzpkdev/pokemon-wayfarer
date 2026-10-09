"""Collect every item spot of the compiled Wayfarer maps: item balls and hidden items.

An item ball is an object event whose script gives exactly one item (a `finditem`, an
`additem`, or the shared `Common_EventScript_FindItem` reading the item from the event).
A hidden item is a `bg_hidden_item_event*` row; its id is its index among the map's bg events.
"""
from __future__ import annotations

import json
from pathlib import Path
import re
import sys

TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[1]
sys.path.insert(0, str(ROOT / 'tools/trainer_scaling'))
import reach_resolver as R  # noqa: E402

BALL_GFX = ('OBJ_EVENT_GFX_ITEM_BALL', 'OBJ_EVENT_GFX_ITEM_BALL_HNS', 'OBJ_EVENT_GFX_POKE_BALL')
COMMON_FIND_ITEM = 'Common_EventScript_FindItem'
GIVES = re.compile(r'^\s*(finditem|additem|giveitem)\s+(ITEM_\w+)', re.M)
NO_FLAG = {'', '0', '0x0', 'FLAG_NONE', 'FLAG_0'}


class Spot:
    __slots__ = ('map_const', 'map_name', 'map_value', 'kind', 'id', 'x', 'y', 'authored', 'flag', 'script',
                 'region', 'tier', 'reach_rule', 'reach_detail', 'status', 'reason', 'prize', 'prize_tier', 'index',
                 'sevii_row', 'gfx', 'underfoot', 'flag_c')

    def __init__(self, **fields):
        for slot in self.__slots__:
            setattr(self, slot, fields.get(slot))

    @property
    def label(self):
        return f'{self.map_name} ' + (f'local id {self.id} ({self.x},{self.y})' if self.kind == 'ball' else f'hidden {self.x},{self.y}')

    @property
    def key(self):
        return (self.map_value, 0 if self.kind == 'ball' else 1, self.id)


def hidden_events(text):
    """[(bg index, row)] of every hidden item among a map's bg events, in event order."""
    rows, index = [], 0
    for line in text.splitlines():
        stripped = line.strip()
        match = re.match(r'bg_(\w+?)_event(?:_(\w+))? (.*)$', stripped)
        if not match:
            continue
        if match.group(1) == 'hidden_item':
            args = [a.strip() for a in match.group(3).split(',')]
            rows.append((index, {'x': int(args[0], 0), 'y': int(args[1], 0), 'item': args[3], 'flag': args[4],
                                 'quantity': args[5] if len(args) > 5 else '1', 'underfoot': args[6] if len(args) > 6 else 'FALSE'}))
        index += 1
    return rows


def ball_item(obj, labels):
    """(authored item or None, script text) of a ball object."""
    script = obj['script']
    if script == COMMON_FIND_ITEM:
        return obj['sight'], 'common'
    body = labels.get(script)
    if not body:
        return None, 'none'
    found = set(item for _, item in GIVES.findall(re.sub(r'@.*', '', body)))
    if len(found) == 1 and 'trainerbattle' not in body and 'setwildbattle' not in body:
        return found.pop(), 'script'
    return None, 'other'


def sevii_rows(root=ROOT):
    """{map name: {source object index: (compiled local id, x, y)}} from the Sevii manifest's retained events."""
    manifest = json.loads((root / 'src/data/wayfarer_sevii_maps.json').read_text())
    result = {}
    for entry in manifest['maps']:
        rows = {}
        for position, event in enumerate(entry['retained_events']['object_events']):
            source = event['source']
            rows[event['index']] = (position + 1, source['x'], source['y'])
        result[entry['source_map']] = rows
    return result


def game_version(name, root=ROOT):
    return json.loads((root / 'data/maps' / name / 'map.json').read_text()).get('game_version', 'emerald')


def region_of(name, root=ROOT):
    path = root / 'data/maps' / name / 'map.json'
    data = json.loads(path.read_text())
    version, region = data.get('game_version', 'emerald'), data.get('region')
    if version == 'emerald':
        return 'Hoenn'
    if version == 'frlg':
        return {'REGION_KANTO': 'Kanto', 'REGION_GALAR': 'Sevii'}.get(region, 'Other')
    return {'REGION_KANTO': 'Kanto', 'REGION_JOHTO': 'Johto', 'REGION_ALOLA': 'Alola', 'REGION_HISUI': 'Sinjoh'}.get(region, 'Other')


def collect(maps, labels, root=ROOT):
    """Every ball-like object and hidden item of the compiled maps as Spot rows (unclassified)."""
    spots = []
    for const, info in maps.items():
        region = None
        events_text = (root / 'data/maps' / info.events_owner / 'events.inc').read_text()
        for obj in info.events['objects']:
            if obj.get('kind') != 'object' or obj['gfx'] not in BALL_GFX:
                continue
            authored, how = ball_item(obj, labels)
            spot = Spot(map_const=const, map_name=info.name, map_value=info.value, kind='ball', id=obj['local_id'],
                        x=obj['x'], y=obj['y'], authored=authored, flag=obj['flag'], script=obj['script'], gfx=obj['gfx'])
            spot.status = 'item' if authored else 'unrecognised'
            spots.append(spot)
        for index, row in hidden_events(events_text):
            spot = Spot(map_const=const, map_name=info.name, map_value=info.value, kind='hidden', id=index, x=row['x'], y=row['y'],
                        authored=row['item'], flag=row['flag'], script='', gfx='', underfoot=row['underfoot'] not in ('FALSE', '0'))
            spot.status = 'item' if row['item'] not in ('ITEM_NONE', '0') else 'empty'
            spots.append(spot)
    return spots
