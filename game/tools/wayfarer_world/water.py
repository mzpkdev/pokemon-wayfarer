"""Off-screen water links: land nodes joined across Surf-connected water.

The off-screen simulation may cross water ("arrives by boat" while unseen);
the on-screen walker never does. A water edge joins two land nodes, so a
record always rests on land and a walker always spawns on land.

Water is the surfable tiles (TILE_FLAG_SURFABLE, waterfalls included)
and the tiles under a bridge (MB_BRIDGE_OVER_*, elevation 15) without
collision bits or a New Game object; whirlpools (objects running
EventScript_Whirlpool, Johto's 2x2 blockers) don't count as objects here.
Off-screen, a notable trainer has the water HMs (Surf, Waterfall,
Whirlpool); land HMs (Cut, Strength, Rock Smash) still block. Two water
tiles are joined when a surfing object could step between them: the
directional walls and the elevation-mismatch rule as on land, and map
connections as lanes. A warp on water (Route 40's row into Route 41) joins
its tile to the landing water, or lands on a shore. Authored Dive links (water.json) join two water tiles
that Dive and emerge connect, such as Sootopolis's lake and Route 126.

A land node touches the water at its shore: a state at elevation 3 next to a
surfable-fishable tile it could Surf onto (IsPlayerFacingSurfableFishableWater:
the step is an elevation mismatch, the player stands at elevation 3, and no
collision bit or directional wall stops it).

A water body touching many land nodes (Hoenn's sea) would give a quadratic
number of pairs, so each water tile goes to its nearest shore (a multi-source
breadth-first flood) and two land nodes are linked when their water regions
meet: the region adjacency of a connected water body is connected, so every
pair of land nodes on one body stays mutually reachable. Each link is two
directed edges. Edge fields: a, b = the source node's shore tile (a land tile
on the source map), c = the reverse edge's position among the target node's
edges (filled in once edges are sorted).
"""

from collections import deque

import graph
from maps import load_json, TOOL_DIR

ELEVATION_DEFAULT = 3
WHIRLPOOL_SCRIPT = "EventScript_Whirlpool"


def load_dive(path=None):
    return load_json(path or TOOL_DIR / "water.json")["dive"]


class WaterFlood:
    """Water tiles of every in-scope map and their adjacency."""

    def __init__(self, builder):
        self.b = builder
        walk = builder.walk
        c = walk.consts
        self.surf = set(c.surfable)
        # A surfer keeps elevation 1 under a bridge at elevation 15.
        bridges = {v for k, v in c.mb.items()
                   if k.startswith("MB_BRIDGE_OVER_") and k != "MB_BRIDGE_OVER_ICE"}
        self.water = {}
        for info in builder.world.scope:
            grid = builder.floods[info.name].grid
            whirlpools = {o["y"] * grid.w + o["x"] for o in info.events["objects"]
                          if o["script"] == WHIRLPOOL_SCRIPT and grid.inside(o["x"], o["y"])}
            solid = builder.solid[info.name] - whirlpools
            self.water[info.name] = bytearray(
                1 if grid.col[i] == 0 and i not in solid and (
                    grid.mb[i] in self.surf or (grid.mb[i] in bridges and grid.elev[i] == 15))
                else 0 for i in range(grid.w * grid.h))
        self.extra = {}  # (map, tile) -> [(map, tile)]: lanes, water warps and Dive links
        self.extra_shores = {}  # (map, comp) -> [(shore tile, water tile)]: warps from water

    def is_water(self, name, x, y):
        grid = self.b.floods[name].grid
        return grid.inside(x, y) and self.water[name][y * grid.w + x] == 1

    @staticmethod
    def elevations_meet(a, b):
        return a == 0 or b == 0 or a == 15 or b == 15 or a == b

    def lanes(self):
        """Water lanes across map connections, both directions."""
        b = self.b
        for info in b.world.scope:
            ga = b.floods[info.name].grid
            for conn in info.connections:
                kind = graph.CONNECTION_KIND.get(conn["direction"])
                target = b.by_const.get(conn["map"])
                if kind is None or target is None:
                    continue
                gb = b.floods[target.name].grid
                off = conn["offset"]
                if kind == graph.KIND_NORTH:
                    pairs = [((x, 0), (x - off, gb.h - 1)) for x in range(ga.w)]
                    move = graph.NORTH
                elif kind == graph.KIND_SOUTH:
                    pairs = [((x, ga.h - 1), (x - off, 0)) for x in range(ga.w)]
                    move = graph.SOUTH
                elif kind == graph.KIND_WEST:
                    pairs = [((0, y), (gb.w - 1, y - off)) for y in range(ga.h)]
                    move = graph.WEST
                else:
                    pairs = [((ga.w - 1, y), (0, y - off)) for y in range(ga.h)]
                    move = graph.EAST
                leave, enter = b.walk.move_rules[move]
                for (sx, sy), (tx, ty) in pairs:
                    if not self.is_water(info.name, sx, sy) or not self.is_water(target.name, tx, ty):
                        continue
                    i, j = sy * ga.w + sx, ty * gb.w + tx
                    if ga.mb[i] in leave or gb.mb[j] in enter:
                        continue
                    if not self.elevations_meet(ga.elev[i], gb.elev[j]):
                        continue
                    self.link((info.name, i), (target.name, j))

    def link(self, u, v):
        self.extra.setdefault(u, []).append(v)
        self.extra.setdefault(v, []).append(u)

    def warps(self, report):
        """Warps on water (graph.py skips them for walkers): a water warp
        joins its tile to the landing tile's water, or makes the landing
        tile a shore of the land node it lands in. Route 40's bottom row
        of water warps is the way into Route 41."""
        b = self.b
        hole = b.walk.hole_warps
        count = 0
        for info in b.world.scope:
            grid = b.floods[info.name].grid
            for warp in info.events["warps"]:
                x, y = warp["x"], warp["y"]
                if not grid.inside(x, y):
                    continue
                i = y * grid.w + x
                if grid.mb[i] not in self.surf or grid.mb[i] in hole or grid.col[i] != 0:
                    continue
                target = b.by_const.get(warp["dest_map"])
                try:
                    land = int(warp["dest_warp"], 0)
                except ValueError:
                    continue
                if target is None or not 0 <= land < len(target.events["warps"]):
                    continue
                dest = target.events["warps"][land]
                tgrid = b.floods[target.name].grid
                tx, ty = dest["x"], dest["y"]
                if not tgrid.inside(tx, ty):
                    continue
                j = ty * tgrid.w + tx
                self.water[info.name][i] = 1
                if tgrid.mb[j] in self.surf and tgrid.col[j] == 0:
                    self.water[target.name][j] = 1
                    self.link((info.name, i), (target.name, j))
                    count += 1
                    continue
                comp = b.floods[target.name].tile_component(tx, ty, dest["elevation"])
                if comp is not None:
                    self.extra_shores.setdefault((target.name, comp), []).append(
                        ((tx, ty), (info.name, i)))
                    count += 1
        report["water_warps"] = count

    def dive(self, rows, problems):
        for row in rows:
            ends = []
            for name, (x, y) in ((row["from"], row["water"]), (row["to"], row["water_to"])):
                info = self.b.world.maps.get(name)
                if info is None or not info.in_scope:
                    problems.append("water.json %s: %s is not in scope" % (row["name"], name))
                    break
                if not self.is_water(name, x, y):
                    problems.append("water.json %s: %s (%d, %d) is not open water"
                                    % (row["name"], name, x, y))
                    break
                ends.append((name, y * self.b.floods[name].grid.w + x))
            if len(ends) == 2:
                self.link(ends[0], ends[1])

    def neighbours(self, u):
        name, i = u
        grid = self.b.floods[name].grid
        water = self.water[name]
        x, y = i % grid.w, i // grid.w
        for dx, dy in graph.N4:
            nx, ny = x + dx, y + dy
            if not grid.inside(nx, ny):
                continue
            j = ny * grid.w + nx
            if not water[j]:
                continue
            leave, enter = self.b.walk.move_rules[(dx, dy)]
            if grid.mb[i] in leave or grid.mb[j] in enter:
                continue
            if not self.elevations_meet(grid.elev[i], grid.elev[j]):
                continue
            yield (name, j)
        for v in self.extra.get(u, ()):
            yield v

    def shores(self, name, comp):
        """(shore tile, water tile) pairs of one land component, in order."""
        flood = self.b.floods[name]
        grid = flood.grid
        fishable = self.b.walk.fishable
        out = []
        for s in flood.components[comp]:
            i, e = divmod(s, 16)
            if e != ELEVATION_DEFAULT:
                continue
            x, y = i % grid.w, i // grid.w
            for dx, dy in graph.N4:
                nx, ny = x + dx, y + dy
                if not self.is_water(name, nx, ny):
                    continue
                j = ny * grid.w + nx
                m = grid.elev[j]
                if grid.mb[j] not in fishable or not graph.mismatch(ELEVATION_DEFAULT, m):
                    continue
                leave, enter = self.b.walk.move_rules[(dx, dy)]
                if grid.mb[i] in leave or grid.mb[j] in enter:
                    continue
                out.append(((x, y), (name, j)))
        return out + self.extra_shores.get((name, comp), [])


def water_links(builder, seeds, dive_rows, problems, report):
    """Raw links (as build.WorldGraph keeps them) between the land
    components in `seeds` (ordered (map, comp) pairs) that share water."""
    flood = WaterFlood(builder)
    flood.lanes()
    flood.dive(dive_rows, problems)
    flood.warps(report)
    label = {}
    queue = deque()
    pairs = {}

    def meet(a, b):
        (sa, shore_a), (sb, shore_b) = a, b
        if sa == sb:
            return
        key = (sa, sb) if sa < sb else (sb, sa)
        if key not in pairs:
            pairs[key] = {sa: shore_a, sb: shore_b}

    for k, (name, comp) in enumerate(seeds):
        for shore, w in flood.shores(name, comp):
            mine = (k, shore)
            if w in label:
                meet(label[w], mine)
                continue
            label[w] = mine
            queue.append(w)
    while queue:
        u = queue.popleft()
        mine = label[u]
        for v in flood.neighbours(u):
            if v in label:
                meet(mine, label[v])
            else:
                label[v] = mine
                queue.append(v)
    raw = []
    for (ka, kb), shore in sorted(pairs.items()):
        (ma, ca), (mb, cb) = seeds[ka], seeds[kb]
        xa, ya = shore[ka]
        xb, yb = shore[kb]
        raw.append((ma, ca, mb, cb, graph.KIND_WATER, xa, ya, 0, 0xFF, None))
        raw.append((mb, cb, ma, ca, graph.KIND_WATER, xb, yb, 0, 0xFF, None))
    report["water_tiles_reached"] = len(label)
    report["water_links"] = len(pairs)
    report["shore_nodes"] = len({k for key in pairs for k in key})
    return raw
