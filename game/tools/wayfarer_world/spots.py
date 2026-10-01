"""Spot detection: the spots spec's revised rules, ported from the research
inventory (.product/research/notable-spots-inventory/inventory.py) onto the
walker graph's floods, so reachability is the walker's own.

Kinds and per-kind data follow include/wayfarer_world_data.h.
"""

import re
from collections import defaultdict, deque

from maps import (BuildError, GYM_MUSIC, INTERIOR_TYPES, OUTDOOR_TYPES,
                  TOWN_TYPES, read_text)

# enum WorldSpotKind
CENTER_COUNTER, CENTER_SIDE, STORE, GAME_CORNER, GYM, TALL_GRASS, WATER_EDGE, \
    SQUARE, BENCH, NPC_CHAT, NAMED = range(11)
KIND_NAMES = ["WORLD_SPOT_CENTER_COUNTER", "WORLD_SPOT_CENTER_SIDE",
              "WORLD_SPOT_STORE", "WORLD_SPOT_GAME_CORNER", "WORLD_SPOT_GYM",
              "WORLD_SPOT_TALL_GRASS", "WORLD_SPOT_WATER_EDGE",
              "WORLD_SPOT_SQUARE", "WORLD_SPOT_BENCH", "WORLD_SPOT_NPC_CHAT",
              "WORLD_SPOT_NAMED"]
KIND_KEYS = ["center_counter", "center_side", "store", "game_corner", "gym",
             "tall_grass", "water_edge", "square", "bench", "npc_chat", "named"]
KIND_BY_KEY = {k: i for i, k in enumerate(KIND_KEYS)}
PUBLIC_KINDS = {CENTER_COUNTER, CENTER_SIDE, STORE, GAME_CORNER, GYM, SQUARE,
                BENCH, NPC_CHAT}

FLAG_PUBLIC = 1 << 0
FLAG_CAPACITY_2 = 1 << 1
TEMPLATE_SHIFT = 2

DIR_NONE, DIR_SOUTH, DIR_NORTH, DIR_WEST, DIR_EAST = range(5)
DIR_NAMES = ["DIR_NONE", "DIR_SOUTH", "DIR_NORTH", "DIR_WEST", "DIR_EAST"]
VEC_DIR = {(0, 1): DIR_SOUTH, (0, -1): DIR_NORTH, (-1, 0): DIR_WEST, (1, 0): DIR_EAST}
N4 = ((0, -1), (0, 1), (-1, 0), (1, 0))  # up, down, left, right (inventory order)
N8 = N4 + ((1, 1), (1, -1), (-1, 1), (-1, -1))

PATCH_MIN = 6
SQUARE_CLEARANCE = 3
SQUARES_PER_MAP = 3
GYM_APPROACH_STEPS = 3
CENTER_SIDE_PER_WALL = 1

NURSE_GFX = {"OBJ_EVENT_GFX_NURSE", "OBJ_EVENT_GFX_NURSE_FRLG",
             "OBJ_EVENT_GFX_NURSE_HNS", "OBJ_EVENT_GFX_NURSE_CHANSEY_HNS"}
CLERK_GFX = {"OBJ_EVENT_GFX_MART_EMPLOYEE", "OBJ_EVENT_GFX_MART_EMPLOYEE_HNS",
             "OBJ_EVENT_GFX_CLERK"}
SLOT_SCRIPT = re.compile(r"_EventScript_SlotMachine\d*$")
FACING_VEC = {"BG_EVENT_PLAYER_FACING_EAST": (1, 0),
              "BG_EVENT_PLAYER_FACING_WEST": (-1, 0),
              "BG_EVENT_PLAYER_FACING_NORTH": (0, -1),
              "BG_EVENT_PLAYER_FACING_SOUTH": (0, 1)}

NOTABLES = ("BROCK MISTY SURGE LT_SURGE ERIKA JANINE KOGA SABRINA BLAINE "
            "GIOVANNI BLUE FALKNER BUGSY WHITNEY MORTY CHUCK JASMINE PRYCE "
            "CLAIR LANCE ROXANNE BRAWLY WATTSON FLANNERY NORMAN WINONA TATE "
            "LIZA JUAN WALLACE STEVEN LORELEI BRUNO AGATHA WILL KAREN SIDNEY "
            "PHOEBE GLACIA DRAKE").split()
NOTABLE_GFX = re.compile(r"^OBJ_EVENT_GFX_(%s)(_HNS|_FRLG)?$" % "|".join(NOTABLES))
PROP_GFX = re.compile(
    r"BALL|BERRY|MON_BASE|CUTTABLE|BREAKABLE|BOULDER|LIGHT|FOSSIL|TRUCK|"
    r"BOAT|FERRY|SS_TIDAL|SUBMARINE|DOLL|CUSHION|STATUE|MACHINE|_PC\b|"
    r"SIGN|ITEM|^OBJ_EVENT_GFX_VAR_|CABLE_CAR|METEORITE|MIRAGE|"
    r"MAIL|BOX|PROF_OAK_?POC|BIRTH_ISLAND|ORB|EGG|APRICORN|SHADOW|ARROW|"
    r"DEOXYS|UNKNOWN|INVISIBLE")
VILLAIN_GFX = re.compile(r"ROCKET|AQUA_MEMBER|MAGMA_MEMBER|GRUNT|ARCHER|ARIANA|"
                         r"PROTON|PETREL|MAXIE|ARCHIE|TABITHA|SHELLY|MATT|COURTNEY")
STATIONARY = re.compile(r"^MOVEMENT_TYPE_(FACE_\w+|LOOK_AROUND)$")
ACTOR_CMDS = re.compile(
    r"^\s*(applymovement|removeobject|addobject|setobjectxy|setobjectxyperm|"
    r"copyobjectxytoperm|moveobjectoffscreen|showobjectat|hideobjectat|"
    r"turnobject|setobjectmovementtype|setobjectsubpriority)\s+(\w+)", re.M)
MOVEMENT_FACE = {"MOVEMENT_TYPE_FACE_DOWN": (0, 1), "MOVEMENT_TYPE_FACE_UP": (0, -1),
                 "MOVEMENT_TYPE_FACE_LEFT": (-1, 0), "MOVEMENT_TYPE_FACE_RIGHT": (1, 0)}


class Spot:
    __slots__ = ("map", "kind", "x", "y", "facing", "flags", "activities",
                 "area", "store", "local_id", "second", "comp", "node", "label",
                 "template", "capacity", "area_start", "area_count")

    def __init__(self, info, kind, x, y, facing=DIR_NONE, comp=None):
        self.map, self.kind, self.x, self.y = info, kind, x, y
        self.facing, self.flags, self.activities = facing, 0, 0xFF
        self.area, self.store, self.local_id, self.second = None, None, None, None
        self.comp, self.node, self.label, self.template = comp, None, None, 0
        self.capacity = 1

    def key(self):
        return (self.map.order, self.kind, self.y, self.x)


def vendor_labels(root):
    """Script labels whose body opens a shop (pokemart*), from all .inc files."""
    labels = set()
    files = sorted((root / "data/maps").glob("*/scripts.inc")) + \
        sorted((root / "data/scripts").rglob("*.inc"))
    for path in files:
        current = []
        for line in read_text(path).splitlines():
            match = re.match(r"^(\w+)::?\s*$", line)
            if match:
                current.append(match.group(1))
                continue
            if re.match(r"^\s+pokemart", line):
                labels.update(current)
            elif re.match(r"^\s+(end|return|releaseall|release)\b", line):
                current = []
    return labels


def local_id_values(root):
    values = {}
    for name, value in re.findall(r"#define (\w+) (\d+)",
                                  read_text(root / "include/constants/map_event_ids.h")):
        values.setdefault(name, int(value))
    return values


def species_names(root):
    return {m.group(1) for m in re.finditer(
        r"#define SPECIES_(\w+)\s+\d+",
        read_text(root / "include/constants/species.h"))} - {"NONE", "EGG"}


class MapCtx:
    """Per-map lookup tables shared by the detectors."""

    def __init__(self, det, info):
        self.info = info
        self.flood = det.builder.floods[info.name]
        self.grid = self.flood.grid
        g = self.grid
        consts = det.world.consts
        self.objects = [o for o in info.events["objects"] if g.inside(o["x"], o["y"])]
        self.object_tiles = {(o["x"], o["y"]) for o in self.objects}
        self.warp_tiles = {(w["x"], w["y"]) for w in info.events["warps"]}
        self.signs = {(b["x"], b["y"]) for b in info.events["bgs"]}
        gfx = [o["gfx"] for o in self.objects]
        self.nurses = [(o["x"], o["y"]) for o in self.objects if o["gfx"] in NURSE_GFX]
        self.clerk = any(g_ in CLERK_GFX for g_ in gfx)
        self.vendor = any(o["script"] in det.vendors for o in self.objects)
        self.counter_tiles = [(x, y) for y in range(g.h) for x in range(g.w)
                              if g.b(x, y) in det.counter]
        shelf = set(det.shelf)
        if g.family == "frlg" and (self.clerk or self.vendor):
            shelf |= consts.set_of("MB_POKEMON_CENTER_BOOKSHELF")
        self.shelf_kinds = shelf
        self.shelf_count = sum(1 for b in g.mb if b in shelf)
        self.slot_signs = [b for b in info.events["bgs"]
                           if b["kind"] == "sign" and SLOT_SCRIPT.search(b["script"])]

    def free(self, x, y):
        return (self.flood.in_any(x, y) and (x, y) not in self.object_tiles
                and (x, y) not in self.warp_tiles)

    def comp(self, x, y):
        return self.flood.tile_component(x, y)

    def facing_tile(self, x, y, behaviours):
        for dx, dy in N4:
            nx, ny = x + dx, y + dy
            if self.grid.inside(nx, ny) and self.grid.b(nx, ny) in behaviours:
                return (dx, dy)
        return None


class Detector:
    def __init__(self, world, builder, overrides):
        self.world, self.builder = world, builder
        c = world.consts
        self.counter = c.set_of("MB_COUNTER")
        self.shelf = c.set_of("MB_SHOP_SHELF", "MB_SHOP_SHELF_DEPARTMENT",
                              "MB_SHOP_SHELF_DEPARTMENT_FORWARD")
        self.tall_grass = c.set_of("MB_TALL_GRASS", "MB_CYCLING_ROAD_PULL_DOWN_GRASS",
                                   "MB_TALL_GRASS_IMPASSABLE_NORTH")
        self.long_grass = c.set_of("MB_LONG_GRASS")
        self.fishable = builder.walk.fishable
        self.vendors = vendor_labels(world.root)
        self.local_ids = local_id_values(world.root)
        self.species = species_names(world.root)
        self.overrides = overrides
        self.ctx = {}
        self.report = defaultdict(lambda: defaultdict(int))
        self.findings = defaultdict(list)

    def context(self, info):
        if info.name not in self.ctx:
            self.ctx[info.name] = MapCtx(self, info)
        return self.ctx[info.name]

    # Places reached through doors -------------------------------------------

    def rooftops(self):
        """Outdoor-typed maps with no connections whose every warp leads to an
        interior that itself has no warp to an outdoor-typed map."""
        by_const = self.builder.by_const
        roofs = set()
        for info in self.world.scope:
            if info.map_type not in OUTDOOR_TYPES or info.connections:
                continue
            dests = [by_const.get(w["dest_map"]) for w in info.events["warps"]]
            if not dests or any(d is None or d.map_type not in INTERIOR_TYPES
                                for d in dests):
                continue
            if all(by_const.get(w2["dest_map"]) is None
                   or by_const[w2["dest_map"]].map_type not in OUTDOOR_TYPES
                   or by_const[w2["dest_map"]] is info
                   for d in dests for w2 in d.events["warps"]):
                roofs.add(info.name)
        return roofs

    def classify(self, info):
        """Center, Game Corner or Gym by content, first match wins."""
        ctx = self.context(info)
        if ctx.nurses and ctx.counter_tiles and info.music not in GYM_MUSIC:
            return CENTER_COUNTER
        if ctx.slot_signs:
            return GAME_CORNER
        if info.music in GYM_MUSIC:
            return GYM
        return None

    def dropped_place(self, info, kind):
        return (info.name, kind) in self.overrides.drop_places

    def find_places(self):
        by_const = self.builder.by_const
        self.roofs = self.rooftops()
        doors = defaultdict(set)  # interior -> outdoor door sources
        for info in self.world.scope:
            if info.map_type not in OUTDOOR_TYPES or info.name in self.roofs:
                continue
            for w in info.events["warps"]:
                dest = by_const.get(w["dest_map"])
                if dest is not None and dest.map_type in INTERIOR_TYPES:
                    doors[dest.name].add(info.name)
        self.doors = doors
        self.sites = {}
        for name in doors:
            info = self.world.maps[name]
            kind = self.classify(info)
            if kind is not None and not self.dropped_place(info, kind):
                self.sites[name] = kind
            elif kind is not None:
                self.findings["dropped_places"].append([name, KIND_KEYS[kind]])

        def shelves(name):
            return self.context(self.world.maps[name]).shelf_count > 0

        def vendor(name):
            c = self.context(self.world.maps[name])
            return c.clerk or c.vendor
        stores = {}
        for interior in sorted(doors, key=lambda n: self.world.maps[n].order):
            info = self.world.maps[interior]
            if self.classify(info) is not None:
                continue
            if self.dropped_place(info, STORE):
                self.findings["dropped_places"].append([interior, "store"])
                continue
            seen, queue = {interior}, deque([interior])
            while queue:
                current = self.world.maps[queue.popleft()]
                for w in current.events["warps"]:
                    dest = by_const.get(w["dest_map"])
                    if (dest is not None and dest.name not in seen
                            and dest.map_type in INTERIOR_TYPES
                            and self.classify(dest) is None
                            and (shelves(dest.name) or vendor(dest.name))):
                        seen.add(dest.name)
                        queue.append(dest.name)
            floors = tuple(sorted((n for n in seen if shelves(n)),
                                  key=lambda n: self.world.maps[n].order))
            if floors and any(vendor(n) for n in seen):
                stores.setdefault(floors, interior)
        self.store_of = {}
        self.stores = []
        for floors, interior in stores.items():
            kept = [f for f in floors if f not in self.store_of]
            if len(kept) != len(floors):
                self.findings["floors_in_two_stores"].append([interior, list(floors)])
            if not kept:
                continue
            store_index = len(self.stores)
            self.stores.append({"door": interior, "floors": kept})
            for f in kept:
                self.store_of[f] = store_index

    # Per-kind detection -------------------------------------------------------

    def detect(self):
        self.find_places()
        spots = []
        for info in self.world.scope:
            ctx = self.context(info)
            kind = self.sites.get(info.name)
            if kind == CENTER_COUNTER:
                spots += self.center_counter(ctx)
                spots += self.center_side(ctx)
            elif kind == GAME_CORNER:
                spots += self.game_corner(ctx)
            elif kind == GYM:
                spots += self.gym(ctx)
            if info.name in self.store_of:
                spots += self.store(ctx, self.store_of[info.name])
            spots += self.tall_grass_patches(ctx)
            spots += self.water_edge(ctx)
            if info.map_type in TOWN_TYPES and info.name not in self.roofs:
                spots += self.squares(ctx)
            spots += self.npc_chats(ctx)
        spots += self.authored(spots)
        return spots

    def center_counter(self, ctx):
        info, g = ctx.info, ctx.grid
        out = []
        # The tile in front of the nurse stays free for the player. Chansey
        # sprites count as nurses for detection, but only the nurse herself
        # keeps a tile free (Chansey's own, where both stand).
        nurses = [(o["x"], o["y"]) for o in ctx.objects
                  if o["gfx"] in NURSE_GFX and "CHANSEY" not in o["gfx"]] or ctx.nurses
        nurse_front = {(nx + 2 * dx, ny + 2 * dy) for nx, ny in nurses for dx, dy in N4}
        if len(ctx.counter_tiles) == 1 and ctx.nurses:
            # Emerald Centers mark one MB_COUNTER tile: the counter row is it
            # plus the unbroken run of collision tiles beside it; stand beyond
            # the row on the side away from the nurse.
            cx, cy = ctx.counter_tiles[0]
            side = 1 if ctx.nurses[0][1] < cy else -1
            row = [cx]
            for step in (-1, 1):
                x = cx + step
                while g.inside(x, cy) and g.col[cy * g.w + x] != 0:
                    row.append(x)
                    x += step
            ctx.counter_row = {(x, cy) for x in row}
            facing = DIR_NORTH if side == 1 else DIR_SOUTH
            for x in sorted(row):
                t = (x, cy + side)
                if ctx.free(*t) and t not in nurse_front:
                    out.append(Spot(info, CENTER_COUNTER, t[0], t[1], facing, ctx.comp(*t)))
            return out
        ctx.counter_row = set(ctx.counter_tiles)
        # A counter is a wall: an MB_COUNTER tile a walker can stand on (one
        # stray tile in CeruleanCity_PokemonCenter_hns) is not faced.
        solid_counter = {t for t in ctx.counter_tiles if not ctx.flood.in_any(*t)}
        for y in range(g.h):
            for x in range(g.w):
                if not ctx.free(x, y) or (x, y) in nurse_front:
                    continue
                d = next(((dx, dy) for dx, dy in N4 if (x + dx, y + dy) in solid_counter), None)
                if d:
                    out.append(Spot(info, CENTER_COUNTER, x, y, VEC_DIR[d], ctx.comp(x, y)))
        return out

    def center_side(self, ctx):
        """Authored seats, else the wall rule: the first free tile in scan
        order beside each side wall (west, then east), facing into the room,
        with a free tile in front, two or more tiles (Chebyshev) from the
        counter row, its standing tiles, and every warp, and not beside an
        object. At most one per wall."""
        info = ctx.info
        if self.overrides.seats.get(info.name):
            return []  # authored seats are added by authored()
        g = ctx.grid
        avoid = set(getattr(ctx, "counter_row", set())) | set(ctx.counter_tiles)
        for x, y in list(avoid):
            for dx, dy in N4:
                avoid.add((x + dx, y + dy))
        avoid |= ctx.warp_tiles

        def blocked(x, y):
            return not g.inside(x, y) or not ctx.flood.in_any(x, y)
        out = []
        for wall, face in (((-1, 0), (1, 0)), ((1, 0), (-1, 0))):
            found = None
            for y in range(g.h):
                for x in range(g.w):
                    if not ctx.free(x, y):
                        continue
                    if not blocked(x + wall[0], y) or blocked(x + face[0], y):
                        continue
                    if not ctx.free(x + face[0], y):
                        continue
                    if any(max(abs(x - ax), abs(y - ay)) < 2 for ax, ay in avoid):
                        continue
                    if any((x + dx, y + dy) in ctx.object_tiles for dx, dy in N4):
                        continue
                    found = (x, y)
                    break
                if found:
                    break
            if found:
                out.append(Spot(info, CENTER_SIDE, found[0], found[1],
                                VEC_DIR[face], ctx.comp(*found)))
        return out

    def store(self, ctx, store_index):
        info, g = ctx.info, ctx.grid
        out = []
        for y in range(g.h):
            for x in range(g.w):
                if not ctx.free(x, y):
                    continue
                d = ctx.facing_tile(x, y, ctx.shelf_kinds)
                if d:
                    s = Spot(info, STORE, x, y, VEC_DIR[d], ctx.comp(x, y))
                    s.store = store_index
                    out.append(s)
        return out

    def game_corner(self, ctx):
        out = []
        for sign in ctx.slot_signs:
            d = FACING_VEC.get(sign["facing"])
            if not d:
                continue
            t = (sign["x"] - d[0], sign["y"] - d[1])
            if ctx.free(*t):
                out.append(Spot(ctx.info, GAME_CORNER, t[0], t[1], VEC_DIR[d], ctx.comp(*t)))
        return out

    def gym(self, ctx):
        """The Gym's exit warp (the lowest-index warp back to a town door),
        and the free tiles within 3 steps of it, nearest first."""
        info = ctx.info
        sources = {self.world.maps[n].const for n in self.doors.get(info.name, ())}
        exits = [(k, w) for k, w in enumerate(info.events["warps"])
                 if w["dest_map"] in sources]
        if not exits:
            self.findings["gym_without_exit"].append(info.name)
            return []
        k, w = exits[0]
        x0, y0 = w["x"], w["y"]
        comps = self.builder.warp_sources(info, w)
        if not comps:
            self.findings["gym_exit_unreachable"].append(info.name)
            return []
        comp = comps[0]
        flood = ctx.flood
        dist = {}
        queue = deque()
        if flood.in_any(x0, y0):
            dist[(x0, y0)] = 0
            queue.append((x0, y0))
        else:
            for dx, dy in N4:
                t = (x0 + dx, y0 + dy)
                if comp in flood.tile_comps.get(t[1] * ctx.grid.w + t[0], ()) \
                        and ctx.grid.inside(*t):
                    dist[t] = 1
                    queue.append(t)
        while queue:
            x, y = queue.popleft()
            if dist[(x, y)] >= GYM_APPROACH_STEPS:
                continue
            for dx, dy in N4:
                t = (x + dx, y + dy)
                if t in dist or not ctx.grid.inside(*t):
                    continue
                if comp in flood.tile_comps.get(t[1] * ctx.grid.w + t[0], ()):
                    dist[t] = dist[(x, y)] + 1
                    queue.append(t)
        area = sorted((t for t in dist if ctx.free(*t) and t != (x0, y0)),
                      key=lambda t: (dist[t], t[1], t[0]))
        s = Spot(info, GYM, x0, y0, DIR_NONE, comp)
        s.area = area
        return [s]

    def tall_grass_patches(self, ctx):
        info, g, flood = ctx.info, ctx.grid, ctx.flood
        grass = defaultdict(set)
        for y in range(g.h):
            for x in range(g.w):
                if g.b(x, y) in self.tall_grass:
                    for c in flood.tile_comps.get(y * g.w + x, ()):
                        grass[c].add((x, y))
        out = []
        for comp, tiles in sorted(grass.items()):
            for patch in components(tiles, N4):
                if len(patch) < PATCH_MIN:
                    self.report["small_grass_patches"][info.name] += 1
                    continue
                s = Spot(info, TALL_GRASS, 0, 0, DIR_NONE, comp)
                s.area = sorted(patch, key=lambda t: (t[1], t[0]))
                s.x, s.y = anchor(s.area)
                out.append(s)
        return out

    def water_edge(self, ctx):
        info, g = ctx.info, ctx.grid
        if info.map_type in INTERIOR_TYPES or info.music in GYM_MUSIC:
            return []
        out = []
        for y in range(g.h):
            for x in range(g.w):
                if not ctx.free(x, y):
                    continue
                d = ctx.facing_tile(x, y, self.fishable)
                if d:
                    out.append(Spot(info, WATER_EDGE, x, y, VEC_DIR[d], ctx.comp(x, y)))
        return out

    def squares(self, ctx):
        info, g, flood = ctx.info, ctx.grid, ctx.flood
        anchors = ctx.warp_tiles | ctx.object_tiles | ctx.signs
        no_grass = self.tall_grass | self.long_grass
        centres = []
        for y in range(1, g.h - 1):
            for x in range(1, g.w - 1):
                comp = flood.tile_component(x, y)
                if comp is None:
                    continue
                ok = True
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        i = (y + dy) * g.w + x + dx
                        if comp not in flood.tile_comps.get(i, ()) or g.mb[i] in no_grass:
                            ok = False
                            break
                    if not ok:
                        break
                if ok and all(max(abs(x - ax), abs(y - ay)) >= SQUARE_CLEARANCE
                              for ax, ay in anchors):
                    centres.append((x, y))
        groups = components(centres, N8)
        ranked = sorted(groups, key=lambda grp: (-len(grp), min((t[1], t[0]) for t in grp)))
        out = []
        for group in ranked[:SQUARES_PER_MAP]:
            area = sorted(group, key=lambda t: (t[1], t[0]))
            x, y = anchor(area)
            s = Spot(info, SQUARE, x, y, DIR_NONE, ctx.comp(x, y))
            s.area = area
            out.append(s)
        if len(ranked) > SQUARES_PER_MAP:
            self.report["squares_over_cap"][info.name] = len(ranked)
        return out

    def is_pokemon_gfx(self, gfx):
        name = gfx.replace("OBJ_EVENT_GFX_", "")
        name = re.sub(r"(_\d+|_HNS|_FRLG|_FOLLOWER|_DOLL|_ASLEEP)+$", "", name)
        return name in self.species or "SPECIES" in gfx

    def scripted_actors(self, info):
        actors = set()
        for _, token in ACTOR_CMDS.findall(self.world.scripts_text(info)):
            if token.isdigit():
                actors.add(int(token))
            elif token in self.local_ids:
                actors.add(self.local_ids[token])
        return actors

    def npc_chats(self, ctx):
        info = ctx.info
        actors = self.scripted_actors(info)
        out = []
        rejects = self.report["npc_rejects"]
        for o in ctx.objects:
            g = o["gfx"]
            reason = None
            if o["kind"] != "object":
                reason = "clone"
            elif o["trainer_type"] not in ("TRAINER_TYPE_NONE", "0"):
                reason = "trainer"
            elif o["flag"] not in ("0", "0x0"):
                reason = "flagged"
            elif g in NURSE_GFX or g in CLERK_GFX or "NURSE" in g:
                reason = "nurse or clerk"
            elif NOTABLE_GFX.match(g):
                reason = "notable cameo"
            elif VILLAIN_GFX.search(g):
                reason = "villain team"
            elif self.is_pokemon_gfx(g):
                reason = "pokemon"
            elif PROP_GFX.search(g):
                reason = "item or prop"
            elif not STATIONARY.match(o["movement"]):
                reason = "not standing still"
            elif o["local_id"] in actors:
                reason = "scripted actor"
            if reason is None:
                order = []
                if o["movement"] in MOVEMENT_FACE:
                    order.append(MOVEMENT_FACE[o["movement"]])
                order += [d for d in ((0, 1), (0, -1), (-1, 0), (1, 0)) if d not in order]
                tile = next(((o["x"] + dx, o["y"] + dy) for dx, dy in order
                             if ctx.free(o["x"] + dx, o["y"] + dy)), None)
                if tile is None:
                    reason = "no free adjacent tile"
                else:
                    d = (o["x"] - tile[0], o["y"] - tile[1])
                    s = Spot(info, NPC_CHAT, tile[0], tile[1], VEC_DIR[d], ctx.comp(*tile))
                    s.local_id = o["local_id"]
                    out.append(s)
            if reason:
                rejects[reason] += 1
        return out

    def authored(self, detected):
        out = []
        for entry in self.overrides.adds:
            info = self.world.maps.get(entry["map"])
            if info is None or not info.in_scope:
                raise BuildError("override %s: map %s not in scope" % (entry["source"], entry["map"]))
            ctx = self.context(info)
            x, y = entry["x"], entry["y"]
            if not ctx.free(x, y):
                raise BuildError("override %s: %s (%d, %d) is not a free walkable tile"
                                 % (entry["source"], info.name, x, y))
            kind = KIND_BY_KEY[entry["kind"]]
            if kind not in (BENCH, CENTER_SIDE):
                raise BuildError("override %s: only bench and center_side can be added"
                                 % entry["source"])
            out.append(Spot(info, kind, x, y, entry["facing"], ctx.comp(x, y)))
        return out


def anchor(area):
    mx = sum(x for x, _ in area) / len(area)
    my = sum(y for _, y in area) / len(area)
    return min(area, key=lambda t: ((t[0] - mx) ** 2 + (t[1] - my) ** 2, t[1], t[0]))


def components(tiles, neighbours):
    """Connected groups, each sorted, in order of their first tile (y, x)."""
    tiles, out = set(tiles), []
    for start in sorted(tiles, key=lambda t: (t[1], t[0])):
        if start not in tiles:
            continue
        tiles.remove(start)
        group, queue = [start], deque([start])
        while queue:
            x, y = queue.popleft()
            for dx, dy in neighbours:
                n = (x + dx, y + dy)
                if n in tiles:
                    tiles.remove(n)
                    group.append(n)
                    queue.append(n)
        out.append(group)
    return out
