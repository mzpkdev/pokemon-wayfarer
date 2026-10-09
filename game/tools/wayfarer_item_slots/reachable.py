"""Static reachability of hidden items (daily world slots, generation and reports).

A hidden item is found by facing its tile (GetInteractedBackgroundEventScript), so some neighbouring tile
must be one the player can stand or surf on and whose elevation matches the event: GetBackgroundEventAtPosition
needs bg elevation == the player's elevation, or bg elevation 0 (ELEVATION_TRANSITION). The player's elevation
is the tile's, and 15 (a bridge or multi-level tile) matches any. This reads the layout's map.bin
collision and elevation read-only, through the walker generator's Grid.
"""
from __future__ import annotations

from pathlib import Path
import sys

TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[1]
sys.path.insert(0, str(ROOT / 'tools/trainer_scaling'))
import reach_resolver as R  # noqa: E402

NEIGHBOURS = ((0, -1), (0, 1), (-1, 0), (1, 0))


def load_world(names, root=ROOT):
    module = R.load_module('item_slots_world_maps', root / 'tools/wayfarer_world/maps.py')
    return module.World(root, only=set(names))


def faceable_from(grid, x, y, elevation):
    """The neighbouring tiles from which a player can face (x, y) at the event's elevation; empty means unreachable."""
    if not grid.inside(x, y):
        return []
    found = []
    for dx, dy in NEIGHBOURS:
        nx, ny = x + dx, y + dy
        if not grid.inside(nx, ny):
            continue
        index = ny * grid.w + nx
        if grid.col[index] != 0:  # impassable; surfable water has collision 0
            continue
        player = grid.elev[index]
        if elevation == 0 or player == 15 or player == elevation:
            found.append((nx, ny))
    return found


def unreachable(spots, root=ROOT):
    """[(spot, reason)] for hidden spots with no tile to face them from."""
    hidden = [s for s in spots if s.kind == 'hidden']
    world = load_world({s.map_name for s in hidden}, root)
    problems = []
    for spot in hidden:
        info = world.maps[spot.map_name]
        grid = world.grid(info)
        if not grid.inside(spot.x, spot.y):
            problems.append((spot, f'outside the {grid.w}x{grid.h} layout'))
        elif not faceable_from(grid, spot.x, spot.y, spot.elevation):
            problems.append((spot, f'no neighbouring tile to stand or surf on at elevation {spot.elevation}'))
    return problems
