"""Assemble the walker graph and spot table from the map model."""

from collections import Counter, defaultdict

import authored
import graph
import maps
import spots as sp
from maps import BuildError, INTERIOR_TYPES, REGIONS, TOWN_TYPES

REGION_INDEX = {r: i for i, r in enumerate(REGIONS)}
AREA_KINDS = {sp.TALL_GRASS, sp.SQUARE, sp.GYM}


class Node:
    __slots__ = ("id", "map", "comp", "x", "y", "flags", "edges", "region")


class Edge:
    __slots__ = ("source", "target", "kind", "a", "b", "c", "warp_id", "label")

    def key(self):
        return (self.kind, self.a, self.b, self.target, self.c, self.warp_id)


class WorldGraph:
    def __init__(self, world, named_rows=None, overrides=None, transit=None):
        self.world = world
        self.problems = []
        self.overrides = overrides or authored.Overrides()
        self.flags = maps.new_game_flags(world.root)
        self.builder = graph.GraphBuilder(world, self.flags, maps.flag_aliases(world.root),
                                          self.overrides.ignore_objects)
        b = self.builder
        for (name, local_id), why in sorted(self.overrides.ignore_objects.items()):
            info = world.maps.get(name)
            if info is None or not info.in_scope or not any(
                    o["local_id"] == local_id for o in info.events["objects"]):
                self.problems.append("ignore_objects: %s has no object %d in scope" % (name, local_id))
        b.build_incoming()
        # Raw links between (map, component) pairs.
        raw = []  # (src map, src comp, dst map, dst comp, kind, a, b, c, warp id, label)
        for info in world.scope:
            lanes = defaultdict(list)
            for kind, comp, target, tcomp, coord, off in b.connection_lanes(info):
                lanes[(kind, comp, target.name, tcomp, off)].append(coord)
            for (kind, comp, tname, tcomp, off), coords in sorted(lanes.items()):
                coords = sorted(set(coords))
                start = prev = coords[0]
                for c in coords[1:] + [None]:
                    if c is not None and c == prev + 1:
                        prev = c
                        continue
                    raw.append((info.name, comp, tname, tcomp, kind, start, prev,
                                start - off, 0xFF, None))
                    if c is not None:
                        start = prev = c
            for comp, k, target, land, tcomp in b.warp_links(info):
                w = info.events["warps"][k]
                raw.append((info.name, comp, target.name, tcomp, graph.KIND_WARP,
                            w["x"], w["y"], land, k, None))
        for link in (transit if transit is not None else authored.load_transit()):
            raw.append(self.transit_link(link))
        self.raw_links = [r for r in raw if r is not None]

        # Spots.
        self.detector = sp.Detector(world, b, self.overrides)
        det = self.detector
        detected = det.detect()
        rows = named_rows if named_rows is not None else authored.load_named()
        named = authored.build_named(rows, world, det, self.problems)
        named_tiles = {(s.map.name, s.x, s.y) for s in named}

        # A spot must be reachable: its component has an edge (in or out).
        # Components with a warp tile but no usable edge still become nodes
        # (the spec keeps them), but hold no spots.
        linked = set()
        for r in self.raw_links:
            linked.add((r[0], r[1]))
            linked.add((r[2], r[3]))
        self.linked = linked
        with_warp = set()
        for info in world.scope:
            for w in info.events["warps"]:
                for comp in b.warp_sources(info, w):
                    with_warp.add((info.name, comp))
        self.dropped = Counter()
        self.unreachable = defaultdict(Counter)
        kept = []
        for s in detected:
            if self.overrides.dropped(s):
                self.dropped["authored drop"] += 1
                continue
            if (s.map.name, s.x, s.y) in named_tiles:
                self.dropped["on a named spot's tile"] += 1
                det.findings["dropped_on_named_tile"].append(
                    [s.map.name, sp.KIND_KEYS[s.kind], s.x, s.y])
                continue
            if (s.map.name, s.comp) not in linked:
                self.dropped["unreachable (no edge)"] += 1
                self.unreachable[s.map.name][sp.KIND_KEYS[s.kind]] += 1
                continue
            if s.area is not None:
                s.area = [t for t in s.area if (s.map.name, t[0], t[1]) not in named_tiles]
            kept.append(s)
        for s in named:
            if (s.map.name, s.comp) not in linked:
                self.problems.append("named spot %r (%s): tile (%d, %d) is not reachable "
                                     "from the map's warps and edges" % (s.label, s.map.name, s.x, s.y))
        all_spots = kept + [s for s in named if (s.map.name, s.comp) in linked]
        used = linked | with_warp

        # Nodes in map order, then component order (first tile in scan order).
        self.nodes, self.node_of = [], {}
        self.dropped_components = Counter()
        for info in world.scope:
            flood = b.floods[info.name]
            for comp, states in enumerate(flood.components):
                if (info.name, comp) not in used:
                    self.dropped_components[info.name] += 1
                    continue
                n = Node()
                n.id, n.map, n.comp = len(self.nodes), info, comp
                first = states[0] // 16
                n.x, n.y = first % flood.grid.w, first // flood.grid.w
                n.region = info.region
                n.flags = (1 if info.map_type in INTERIOR_TYPES else 0) | \
                    (REGION_INDEX[info.region] << 1)
                n.edges = []
                self.node_of[(info.name, comp)] = n.id
                self.nodes.append(n)
        if len(self.nodes) > 4095:
            raise BuildError("%d nodes, over the record's 4,095" % len(self.nodes))

        # Edges per source node, deduplicated, in (kind, a, b) order.
        for r in self.raw_links:
            src = self.node_of.get((r[0], r[1]))
            dst = self.node_of.get((r[2], r[3]))
            if src is None or dst is None:
                raise BuildError("edge %s -> %s has no node" % (r[0], r[2]))
            e = Edge()
            e.source, e.target, e.kind = src, dst, r[4]
            e.a, e.b, e.c, e.warp_id, e.label = r[5], r[6], r[7], r[8], r[9]
            for v, what in ((e.a, "a"), (e.b, "b"), (e.c, "c")):
                if not 0 <= v <= 255:
                    raise BuildError("edge %s -> %s: %s=%d does not fit a u8"
                                     % (r[0], r[2], what, v))
            self.nodes[src].edges.append(e)
        self.edges = []
        for n in self.nodes:
            unique = {e.key(): e for e in n.edges}
            n.edges = [unique[k] for k in sorted(unique)]
            if len(n.edges) > 255:
                raise BuildError("node %d (%s) has %d edges" % (n.id, n.map.name, len(n.edges)))
            self.edges += n.edges
        if len(self.edges) > 0xFFFF:
            raise BuildError("too many edges")

        # Spots in map order, then (kind, y, x).
        for s in all_spots:
            if s.comp is not None:
                s.node = self.node_of[(s.map.name, s.comp)]
        self.spots = sorted(all_spots, key=lambda s: s.key())
        if len(self.spots) > 16383:
            raise BuildError("%d spots, over the record's 16,383" % len(self.spots))
        self.spot_id = {id(s): i for i, s in enumerate(self.spots)}
        self.public_interiors = self.town_interiors()
        for s in self.spots:
            if s.kind in sp.PUBLIC_KINDS:
                s.flags |= sp.FLAG_PUBLIC
            elif s.kind == sp.NAMED and (s.map.map_type in TOWN_TYPES
                                         or s.map.name in self.public_interiors):
                s.flags |= sp.FLAG_PUBLIC
            if s.kind == sp.NAMED:
                if s.capacity == 2:
                    s.flags |= sp.FLAG_CAPACITY_2
                s.flags |= s.template << sp.TEMPLATE_SHIFT
        self.layout_data()

    def transit_link(self, link):
        b = self.builder
        src = self.world.maps.get(link["from"])
        dst = self.world.maps.get(link["to"])
        tag = "transit %s %s -> %s" % (link["name"], link["from"], link["to"])
        if src is None or dst is None or not src.in_scope or not dst.in_scope:
            self.problems.append(tag + ": map not in scope")
            return None
        x, y = link["board"]
        comp = b.floods[src.name].tile_component(x, y)
        if comp is None:
            self.problems.append(tag + ": boarding tile (%d, %d) is in no node" % (x, y))
            return None
        land = link["landing_warp"]
        if not 0 <= land < len(dst.events["warps"]):
            self.problems.append(tag + ": landing warp %d does not exist" % land)
            return None
        tcomp = b.landing_component(dst, land)
        if tcomp is None:
            self.problems.append(tag + ": landing warp %d has no walkable tile" % land)
            return None
        return (src.name, comp, dst.name, tcomp, graph.KIND_TRANSIT, x, y, land, 0xFF,
                link["name"])

    def town_interiors(self):
        """Interiors entered from a town or city map, through interior warps."""
        by_const = self.builder.by_const
        seen = set()
        frontier = [m for m in self.world.scope if m.map_type in TOWN_TYPES]
        while frontier:
            nxt = []
            for info in frontier:
                for w in info.events["warps"]:
                    dest = by_const.get(w["dest_map"])
                    if dest is not None and dest.map_type in INTERIOR_TYPES \
                            and dest.name not in seen:
                        seen.add(dest.name)
                        nxt.append(dest)
            frontier = nxt
        return seen

    def layout_data(self):
        """Area tiles and store floors, in spot order."""
        self.area_tiles = []
        self.store_floors = []
        store_run = {}
        store_nodes = defaultdict(set)
        for s in self.spots:
            if s.kind == sp.STORE:
                store_nodes[s.store].add(s.node)
        for s in self.spots:
            if s.kind in AREA_KINDS:
                s_area = s.area or []
                start = len(self.area_tiles)
                self.area_tiles += s_area
                s.area_start, s.area_count = start, len(s_area)
            if s.kind == sp.STORE:
                if s.store not in store_run:
                    nodes = sorted(store_nodes[s.store])
                    store_run[s.store] = (len(self.store_floors), len(nodes))
                    self.store_floors += nodes
                s.area_start, s.area_count = store_run[s.store]
        if len(self.area_tiles) > 0xFFFF or len(self.store_floors) > 0xFFFF:
            raise BuildError("area or store-floor table over 65,535 entries")

    # Helpers ------------------------------------------------------------------

    def spot_data(self, s):
        """(dataStart, dataCount) as wayfarer_world_data.h defines them."""
        if s.kind in AREA_KINDS or s.kind == sp.STORE:
            return s.area_start, s.area_count
        if s.kind == sp.NPC_CHAT:
            return s.local_id, 0
        if s.kind == sp.NAMED:
            if s.second is not None:
                return s.second[0] | (s.second[1] << 8), 0
            return 0xFFFF, 0
        return 0, 0

    def map_nodes(self, name):
        return [n for n in self.nodes if n.map.name == name]

    def check(self):
        if self.problems:
            raise BuildError("\n".join(self.problems))
