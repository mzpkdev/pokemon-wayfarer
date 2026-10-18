"""Place types of trainer groups: does a slot really stand in that kind of place?

The rules live in place_types.json. A rule holds when the slot's map type or map name matches, or a tile of the slot's
object neighbourhood has one of the rule's metatile behaviours (read with the walker generator's Grid, read-only).
"""
from __future__ import annotations

import json
from pathlib import Path
import sys

TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[1]
sys.path.insert(0, str(ROOT / 'tools/trainer_scaling'))
import reach_resolver as R  # noqa: E402

SPECS = TOOL / 'place_types.json'


def load_specs(path=SPECS):
    data = json.loads(Path(path).read_text())
    if data.get('version') != 1:
        raise ValueError('unsupported place_types.json version')
    return data['types']


def load_world(map_names, root=ROOT):
    module = R.load_module('place_world_maps', root / 'tools/wayfarer_world/maps.py')
    return module.World(root, only=set(map_names))


def needs_grid(spec):
    return 'near' in spec


def holds(spec, unit, world=None):
    """True when the unit's slot is that kind of place (every object of a pair must be)."""
    if unit.map_type in spec.get('map_types', ()):
        return True
    if any(unit.map_name.startswith(prefix) for prefix in spec.get('map_name_prefixes', ())):
        return True
    near = spec.get('near')
    if near is None or world is None:
        return False
    info = world.maps[unit.map_name]
    grid = world.grid(info)
    wanted = set()
    for name in near['behaviours']:
        wanted |= set(world.consts.surfable) if name == 'surfable' else {world.consts.mb[name]}
    radius = near['radius']
    for obj in unit.objects:
        found = False
        for dy in range(-radius, radius + 1):
            for dx in range(-radius, radius + 1):
                x, y = obj.x + dx, obj.y + dy
                if grid.inside(x, y) and grid.b(x, y) in wanted:
                    found = True
        if not found:
            return False
    return True
