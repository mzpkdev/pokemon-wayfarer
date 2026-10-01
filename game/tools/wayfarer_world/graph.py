"""Walker graph: NPC-collision flood per map, nodes, and edges.

The flood mirrors an object's movement checks in
src/event_object_movement.c:

- GetVanillaCollision: MapGridGetCollisionAt (the tile's collision bits),
  IsMetatileDirectionallyImpassable, IsElevationMismatchAt, and
  DoesObjectCollideWithObjectAt.
- IsMetatileDirectionallyImpassable checks the leaving tile with
  gOppositeDirectionBlockedMetatileFuncs and the entering tile with
  gDirectionBlockedMetatileFuncs (MetatileBehavior_Is{North,South,East,West}Blocked,
  src/metatile_behavior.c). Moving south is blocked by a south-blocked
  leaving tile or a north-blocked entering tile, and moving north by the
  same pair, so a one-sided wall cuts both directions of one tile edge.
- IsElevationMismatchAt: an object at elevation 0 (transition) never
  mismatches; a tile at elevation 0 or 15 (multi-level) never mismatches;
  otherwise the tile's elevation must equal the object's.
- ObjectEventUpdateElevation: after a step the object takes the tile's
  elevation unless the tile is 15, where it keeps its own (a bridge).

So a walker's state is (x, y, elevation), and elevation only varies on
elevation-15 tiles. Walkers never Surf, use a waterfall or jump a ledge:
surfable tiles (TILE_FLAG_SURFABLE in sTileBitAttributes) and MB_JUMP_*
ledges are solid. Sideways stairs follow GetCollisionAtCoords's special
cases and GetSidewaysStairsCollision's diagonal steps (stair_move below).
Objects present at New Game are solid on their template tile.

A node is a connected component of states (an undirected union of the
allowed moves). The allowed-move relation is symmetric except across a
bridge's elevation memory; the generator counts asymmetric moves and
reports them.
"""

import re
from collections import defaultdict, deque

from maps import BuildError, read_text

KIND_NORTH, KIND_SOUTH, KIND_EAST, KIND_WEST, KIND_WARP, KIND_TRANSIT = 1, 2, 3, 4, 5, 6
EDGE_KIND_NAMES = {1: "WORLD_EDGE_NORTH", 2: "WORLD_EDGE_SOUTH", 3: "WORLD_EDGE_EAST",
                   4: "WORLD_EDGE_WEST", 5: "WORLD_EDGE_WARP", 6: "WORLD_EDGE_TRANSIT"}
CONNECTION_KIND = {"up": KIND_NORTH, "down": KIND_SOUTH, "right": KIND_EAST,
                   "left": KIND_WEST}

# Directions as (dx, dy, name) in the engine's DIR order.
SOUTH, NORTH, WEST, EAST = (0, 1), (0, -1), (-1, 0), (1, 0)
N4 = (NORTH, SOUTH, WEST, EAST)


def blocked_sets(root, consts):
    """Behaviours for which MetatileBehavior_Is{X}Blocked is true."""
    text = read_text(root / "src/metatile_behavior.c")
    out = {}
    for side in ("North", "South", "East", "West"):
        start = text.index("bool8 MetatileBehavior_Is%sBlocked(" % side)
        body = text[start:text.index("\n}", start)]
        out[side] = {consts.mb[n] for n in re.findall(r"== (MB_\w+)", body)}
    return out


class Walk:
    """Static walkability rules for one world."""

    def __init__(self, world):
        c = world.consts
        self.consts = c
        self.blocked = blocked_sets(world.root, c)
        # (leaving tile blocked set, entering tile blocked set) per move.
        self.move_rules = {
            SOUTH: (self.blocked["South"], self.blocked["North"]),
            NORTH: (self.blocked["North"], self.blocked["South"]),
            WEST: (self.blocked["West"], self.blocked["East"]),
            EAST: (self.blocked["East"], self.blocked["West"]),
        }
        ledges = {v for k, v in c.mb.items() if k.startswith("MB_JUMP_")}
        self.solid_behaviours = set(c.surfable) | ledges
        self.ledges = ledges
        mb = c.mb
        self.lt = {mb["MB_SIDEWAYS_STAIRS_LEFT_SIDE_TOP"]}
        self.rt = {mb["MB_SIDEWAYS_STAIRS_RIGHT_SIDE_TOP"]}
        self.lb = {mb["MB_SIDEWAYS_STAIRS_LEFT_SIDE_BOTTOM"]}
        self.rb = {mb["MB_SIDEWAYS_STAIRS_RIGHT_SIDE_BOTTOM"]}
        # MetatileBehavior_IsSidewaysStairs{Left,Right}Side and ...Any.
        self.left_side = {mb["MB_SIDEWAYS_STAIRS_LEFT_SIDE"]} | self.lb
        self.right_side = {mb["MB_SIDEWAYS_STAIRS_RIGHT_SIDE"]} | self.rb
        self.left_any = self.left_side | self.lt
        self.right_any = self.right_side | self.rt
        self.stairs = self.left_any | self.right_any
        self.fishable = c.set_of(
            "MB_POND_WATER", "MB_OCEAN_WATER", "MB_INTERIOR_DEEP_WATER",
            "MB_DEEP_WATER", "MB_SOOTOPOLIS_DEEP_WATER", "MB_EASTWARD_CURRENT",
            "MB_WESTWARD_CURRENT", "MB_NORTHWARD_CURRENT", "MB_SOUTHWARD_CURRENT")
        self.hole_warps = c.set_of("MB_MT_PYRE_HOLE", "MB_CRACKED_FLOOR_HOLE",
                                   "MB_FALL_WARP", "MB_CRACKED_FLOOR")


def stair_move(walk, grid, walkable, i, e, j, dx, dy):
    """Destination of a step from tile i towards tile j next to a sideways
    stair, or None: GetCollisionAtCoords's stair checks, the vanilla checks,
    then GetSidewaysStairsCollision's diagonal step (GetLeftSideStairsDirection
    and GetRightSideStairsDirection in field_player_avatar.c)."""
    mb, elev, w = grid.mb, grid.elev, grid.w
    cur, nxt = mb[i], mb[j]
    east, west, south, north = dx == 1, dx == -1, dy == 1, dy == -1
    if (nxt in walk.lt and east) or (nxt in walk.rt and west):
        return None
    if nxt in walk.rb and (east or south):
        return None
    if nxt in walk.lb and (west or south):
        return None
    tops, bottoms = walk.lt | walk.rt, walk.lb | walk.rb
    if cur in tops and north:
        return None
    if cur not in tops and south and nxt in tops:
        return None
    if cur not in bottoms and north and nxt in bottoms:
        return None
    leave, enter = walk.move_rules[(dx, dy)]
    plain = (walkable[j] and cur not in leave and nxt not in enter
             and not mismatch(e, elev[j]))
    plain = j if plain else None
    if dx == 0 or nxt in walk.fishable:
        return plain
    diagonal = None
    if nxt in walk.left_side:
        diagonal = None if (west and cur != nxt) else "left"
    elif nxt in walk.right_side:
        diagonal = None if (east and cur != nxt) else "right"
    elif cur in walk.left_any:
        diagonal = "left" if (west and nxt != cur) else None
    elif cur in walk.right_any:
        diagonal = "right" if (east and nxt != cur) else None
    if diagonal is None:
        return plain
    # Left stairs: west -> south-west, east -> north-east; right stairs:
    # west -> north-west, east -> south-east.
    ddy = (1 if west else -1) if diagonal == "left" else (-1 if west else 1)
    x, y = i % w + dx, i // w + ddy
    if not grid.inside(x, y) or not walkable[y * w + x]:
        return None
    return y * w + x


def mismatch(e, m):
    """IsElevationMismatchAt(e, tile at elevation m)."""
    return e != 0 and m != 0 and m != 15 and m != e


class MapFlood:
    """Flood result for one map: states, components, per-tile lookup."""

    def __init__(self, info, grid, solid_tiles, walk):
        self.info = info
        self.grid = grid
        w, h = grid.w, grid.h
        n = w * h
        mb, col, elev = grid.mb, grid.col, grid.elev
        solid_mb = walk.solid_behaviours
        walkable = bytearray(n)
        for i in range(n):
            if col[i] == 0 and mb[i] not in solid_mb and i not in solid_tiles:
                walkable[i] = 1
        self.walkable = walkable
        rules = walk.move_rules
        moves = []
        for (dx, dy) in N4:
            leave, enter = rules[(dx, dy)]
            moves.append((dx, dy, leave, enter))
        self.moves = moves

        parent = {}

        def find(a):
            root = a
            while parent[root] != root:
                root = parent[root]
            while parent[a] != root:
                parent[a], a = root, parent[a]
            return root

        def union(a, b):
            ra, rb = find(a), find(b)
            if ra != rb:
                if ra < rb:
                    parent[rb] = ra
                else:
                    parent[ra] = rb

        queue = deque()
        for i in range(n):
            if walkable[i] and elev[i] != 15:
                s = i * 16 + elev[i]
                parent[s] = s
                queue.append(s)
        # Elevation-15 tiles carry the walker's elevation; seed warp landings
        # on them with the warp's own elevation (the arrival elevation).
        for warp in info.events["warps"]:
            if grid.inside(warp["x"], warp["y"]):
                i = warp["y"] * w + warp["x"]
                if walkable[i] and elev[i] == 15:
                    s = i * 16 + warp["elevation"]
                    if s not in parent:
                        parent[s] = s
                        queue.append(s)
        stairs = walk.stairs
        edges_seen = set()
        while queue:
            s = queue.popleft()
            i, e = divmod(s, 16)
            x, y = i % w, i // w
            cur = mb[i]
            for dx, dy, leave, enter in moves:
                nx, ny = x + dx, y + dy
                if nx < 0 or ny < 0 or nx >= w or ny >= h:
                    continue
                j = ny * w + nx
                if cur in stairs or mb[j] in stairs:
                    j = stair_move(walk, grid, walkable, i, e, j, dx, dy)
                    if j is None:
                        continue
                    m = elev[j]
                else:
                    if not walkable[j] or cur in leave or mb[j] in enter:
                        continue
                    m = elev[j]
                    if e != 0 and m != 0 and m != 15 and m != e:
                        continue
                t = j * 16 + (e if m == 15 else m)
                if t not in parent:
                    parent[t] = t
                    queue.append(t)
                union(s, t)
                edges_seen.add((s, t))
        self.asymmetric = sum(1 for s, t in edges_seen if (t, s) not in edges_seen)
        comps = defaultdict(list)
        for s in parent:
            comps[find(s)].append(s)
        # Components ordered by their first tile in scan order (y, then x),
        # then by elevation for two bridge layers starting on one tile.
        ordered = sorted(comps.values(), key=lambda states: min(states))
        self.components = []
        self.comp_of = {}
        self.tile_comps = defaultdict(list)
        for k, states in enumerate(ordered):
            states.sort()
            self.components.append(states)
            for s in states:
                self.comp_of[s] = k
                self.tile_comps[s // 16].append(k)
        for i in self.tile_comps:
            self.tile_comps[i] = sorted(set(self.tile_comps[i]))

    def tile_component(self, x, y, e=None):
        """The component of tile (x, y); for a bridge tile, of state e."""
        if not self.grid.inside(x, y):
            return None
        i = y * self.grid.w + x
        if e is not None and i * 16 + e in self.comp_of:
            return self.comp_of[i * 16 + e]
        comps = self.tile_comps.get(i)
        return comps[0] if comps else None

    def in_any(self, x, y):
        return self.grid.inside(x, y) and (y * self.grid.w + x) in self.tile_comps

    def states_of(self, x, y):
        i = y * self.grid.w + x
        return [(s % 16, self.comp_of[s]) for s in range(i * 16, i * 16 + 16)
                if s in self.comp_of]


def solid_object_tiles(info, grid, new_game_flags, canonical, ignored=()):
    """Template tiles of objects present at New Game (walls for walkers),
    minus the authored passable story blockers (overrides `ignore_objects`)."""
    family = "hoenn" if info.json.get("game_version", "emerald") == "emerald" else "hns"
    hidden = {canonical(f) for f in new_game_flags[family]}
    tiles = set()
    for o in info.events["objects"]:
        flag = o["flag"]
        present = flag in ("0", "0x0") or canonical(flag) not in hidden
        if (info.name, o["local_id"]) in ignored:
            continue
        if present and grid.inside(o["x"], o["y"]):
            tiles.add(o["y"] * grid.w + o["x"])
    return tiles


class GraphBuilder:
    """Floods every in-scope map and builds connection and warp edges."""

    def __init__(self, world, new_game_flags, canonical, ignored=()):
        self.world = world
        self.walk = Walk(world)
        self.floods = {}
        self.report = {"dynamic_warps": defaultdict(int), "skipped_warps": defaultdict(int),
                       "asymmetric_moves": 0}
        self.solid = {}
        for info in world.scope:
            grid = world.grid(info)
            if grid.w > 255 or grid.h > 255:
                raise BuildError("%s is %dx%d; the record stores u8 crossings"
                                 % (info.name, grid.w, grid.h))
            solid = solid_object_tiles(info, grid, new_game_flags, canonical, ignored)
            self.solid[info.name] = solid
            flood = MapFlood(info, grid, solid, self.walk)
            self.floods[info.name] = flood
            self.report["asymmetric_moves"] += flood.asymmetric
        self.by_const = {m.const: m for m in world.scope}

    # Connections ------------------------------------------------------------

    def connection_lanes(self, info):
        """(kind, source comp, target map, target comp, coord, offset) tuples."""
        out = []
        fa = self.floods[info.name]
        ga = fa.grid
        for conn in info.connections:
            kind = CONNECTION_KIND.get(conn["direction"])
            target = self.by_const.get(conn["map"])
            if kind is None or target is None:
                continue
            fb = self.floods[target.name]
            gb = fb.grid
            off = conn["offset"]
            if kind == KIND_NORTH:
                pairs = [((x, 0), (x - off, gb.h - 1), x) for x in range(ga.w)]
                move = NORTH
            elif kind == KIND_SOUTH:
                pairs = [((x, ga.h - 1), (x - off, 0), x) for x in range(ga.w)]
                move = SOUTH
            elif kind == KIND_WEST:
                pairs = [((0, y), (gb.w - 1, y - off), y) for y in range(ga.h)]
                move = WEST
            else:
                pairs = [((ga.w - 1, y), (0, y - off), y) for y in range(ga.h)]
                move = EAST
            leave, enter = self.walk.move_rules[move]
            for (sx, sy), (tx, ty), coord in pairs:
                if not gb.inside(tx, ty) or not fa.in_any(sx, sy):
                    continue
                j = ty * gb.w + tx
                if not fb.walkable[j] or ga.b(sx, sy) in leave or gb.mb[j] in enter:
                    continue
                m = gb.elev[j]
                for e, comp in fa.states_of(sx, sy):
                    if mismatch(e, m):
                        continue
                    tcomp = fb.tile_component(tx, ty, e if m == 15 else m)
                    if tcomp is None:
                        continue
                    out.append((kind, comp, target, tcomp, coord, off))
        return out

    # Warps ------------------------------------------------------------------

    def warp_sources(self, info, warp):
        """Components a walker can use warp `warp` from."""
        flood = self.floods[info.name]
        x, y = warp["x"], warp["y"]
        if not flood.grid.inside(x, y):
            return []
        comps = [c for _, c in flood.states_of(x, y)]
        if not comps:
            i = y * flood.grid.w + x
            if not flood.walkable[i]:
                for dx, dy in N4:
                    nx, ny = x + dx, y + dy
                    if flood.in_any(nx, ny):
                        comps += [c for _, c in flood.states_of(nx, ny)]
        return sorted(set(comps))

    def landing_component(self, target, warp_index):
        """The component holding a destination warp's landing tile."""
        flood = self.floods[target.name]
        warp = target.events["warps"][warp_index]
        x, y = warp["x"], warp["y"]
        if flood.in_any(x, y):
            return flood.tile_component(x, y, warp["elevation"])
        for dx, dy in (SOUTH, NORTH, WEST, EAST):
            if flood.in_any(x + dx, y + dy):
                return flood.tile_component(x + dx, y + dy)
        return None

    def warp_links(self, info):
        """(source comp, warp index, target map, dest warp id, target comp)."""
        out = []
        flood = self.floods[info.name]
        grid = flood.grid
        rep = self.report
        for k, warp in enumerate(info.events["warps"]):
            if not grid.inside(warp["x"], warp["y"]):
                rep["skipped_warps"]["off map"] += 1
                continue
            beh = grid.b(warp["x"], warp["y"])
            if beh in self.walk.hole_warps:
                rep["skipped_warps"]["hole or fall warp"] += 1
                continue
            if beh in self.walk.consts.surfable:
                rep["skipped_warps"]["water warp (Surf or Dive)"] += 1
                continue
            sources = self.warp_sources(info, warp)
            if not sources:
                rep["skipped_warps"]["no walkable approach"] += 1
                continue
            links = []
            if warp["dest_map"] == "MAP_DYNAMIC":
                if warp["dest_warp"] == "WARP_ID_SECRET_BASE":
                    rep["dynamic_warps"]["secret base, skipped"] += 1
                    continue
                incoming = self.incoming[(info.const, k)]
                if not incoming:
                    rep["dynamic_warps"]["no incoming warp, skipped"] += 1
                    continue
                rep["dynamic_warps"]["resolved to incoming warps"] += 1
                for src, j in incoming:
                    land = j if warp["dest_warp"] == "WARP_ID_DYNAMIC" else int(warp["dest_warp"], 0)
                    links.append((src, land))
            else:
                target = self.by_const.get(warp["dest_map"])
                if target is None:
                    rep["skipped_warps"]["destination out of scope"] += 1
                    continue
                try:
                    land = int(warp["dest_warp"], 0)
                except ValueError:
                    rep["skipped_warps"]["symbolic destination warp id"] += 1
                    continue
                links.append((target, land))
            for target, land in links:
                if not 0 <= land < len(target.events["warps"]):
                    rep["skipped_warps"]["destination warp id out of range"] += 1
                    continue
                tcomp = self.landing_component(target, land)
                if tcomp is None:
                    rep["skipped_warps"]["landing tile not walkable"] += 1
                    continue
                for comp in sources:
                    out.append((comp, k, target, land, tcomp))
        return out

    def build_incoming(self):
        """Warps into each (map, warp id): resolves MAP_DYNAMIC returns."""
        self.incoming = defaultdict(list)
        for info in self.world.scope:
            for j, warp in enumerate(info.events["warps"]):
                try:
                    dest_id = int(warp["dest_warp"], 0)
                except ValueError:
                    continue
                if warp["dest_map"] in self.by_const:
                    self.incoming[(warp["dest_map"], dest_id)].append((info, j))

