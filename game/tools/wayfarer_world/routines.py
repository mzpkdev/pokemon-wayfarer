"""Simulated trainers: home base, cycle, favourites, candidates, search bound.

Reads routines.json (cycles and favourites, from the routines research
file) and the notable-trainer catalog (traveller and aloof traits, catalog
order and home region), and validates each routine against the generated
graph and spot table.
"""

import re
import struct
from collections import defaultdict, deque

import graph
import spots as sp
from authored import ACTIVITIES, ACTIVITY_INDEX
from maps import BuildError, TOOL_DIR, load_json, read_text

SIM_COUNT = 25
RADIUS_NARROW, RADIUS_TRAVELLER = 3, 8
NODE_NONE, SPOT_NONE = 0xFFF, 0x3FFF
REGION_NONE = 0xFF
REGION_INDEX = {"Kanto": 0, "Johto": 1, "Hoenn": 2, "Sevii": 3}
FLAG_TRAVELLER, FLAG_ALOOF, FLAG_GYM_LEADER = 1, 2, 4

# Activity -> detected kinds (notable spots spec, activities and kinds);
# every activity also takes the named spots that list it.
ACTIVITY_KINDS = {
    "care": {sp.CENTER_COUNTER, sp.CENTER_SIDE},
    "shop": {sp.STORE},
    "gamble": {sp.GAME_CORNER},
    "train": {sp.TALL_GRASS},
    "relax": {sp.SQUARE, sp.BENCH, sp.WATER_EDGE},
    "fish": {sp.WATER_EDGE},
    "visit": {sp.GYM, sp.NPC_CHAT},
}


def spot_offers(spot, activity):
    if spot.kind == sp.NAMED:
        index = ACTIVITY_INDEX[activity]
        return spot.activities & 0xF == index or spot.activities >> 4 == index
    return spot.kind in ACTIVITY_KINDS.get(activity, ())


def sprite_sheet(root, gfx):
    """(anim table, frame size, sheet size) of an OBJ_EVENT_GFX_* sprite.

    Follows object_event_graphics_info_pointers.h -> graphics_info.h ->
    pic_tables.h -> object_event_graphics.h -> the sheet's .png header. A
    pic table lists frames with overworld_frame, or points at the whole sheet
    with overworld_ascending_frames (relative frames), so the sheet's size
    is what tells a Full 9-frame sheet (144x32 at 16x32) from a face-only
    one (48x32)."""
    base = root / "src/data/object_events"
    ptr = re.search(r"\[%s\]\s*=\s*&(\w+)" % re.escape(gfx),
                    read_text(base / "object_event_graphics_info_pointers.h"))
    if not ptr:
        return None
    info = re.search(r"%s\s*=\s*\{([^;]*)\};" % ptr.group(1),
                     read_text(base / "object_event_graphics_info.h"), re.S)
    if not info:
        return None
    anims = re.search(r"\b(sAnimTable_\w+)", info.group(1))
    pics = re.search(r"\b(sPicTable_\w+)", info.group(1))
    if not anims or not pics:
        return None
    table = re.search(r"%s\[\]\s*=\s*\{(.*?)\};" % pics.group(1),
                      read_text(base / "object_event_pic_tables.h"), re.S)
    frames = re.findall(r"overworld_(?:frame|ascending_frames)\((\w+),\s*(\d+),\s*(\d+)",
                        table.group(1))
    sheets = {f[0] for f in frames}
    if len(sheets) != 1:
        return (anims.group(1), None, None)
    sheet = sheets.pop()
    path = re.search(r'%s\[\]\s*=\s*INCBIN_U(?:8|16|32)\("([^"]+)\.4bpp"' % sheet,
                     read_text(base / "object_event_graphics.h"))
    size = None
    if path:
        png = root / (path.group(1) + ".png")
        if png.exists():
            head = png.read_bytes()[:24]
            size = struct.unpack(">II", head[16:24])
    frame = (int(frames[0][1]) * 8, int(frames[0][2]) * 8)
    return (anims.group(1), frame, size)


FULL_SHEET = ("sAnimTable_Standard", (16, 32), (144, 32))


GFX_VARIANT_SUFFIXES = ("", "_HNS", "_FRLG")


def sprite_family(root, gfx):
    """Every OBJ_EVENT_GFX_* the same character is drawn with: the name with
    and without the _HNS / _FRLG suffixes (Viridian's Gym uses FireRed's
    OBJ_EVENT_GFX_GIOVANNI while the walker uses OBJ_EVENT_GFX_GIOVANNI_HNS)."""
    names = set(re.findall(r"#define (OBJ_EVENT_GFX_\w+)\s",
                           read_text(root / "include/constants/event_objects.h")))
    base = gfx
    for suffix in GFX_VARIANT_SUFFIXES[1:]:
        if base.endswith(suffix):
            base = base[:-len(suffix)]
    return {base + suffix for suffix in GFX_VARIANT_SUFFIXES if base + suffix in names} | {gfx}


class Trainer:
    pass


class Routines:
    def __init__(self, wg, routines_path=None, catalog_path=None):
        self.wg = wg
        self.root = wg.world.root
        routines_path = routines_path or TOOL_DIR / "routines.json"
        catalog_path = catalog_path or self.root / "tools/notable_trainers/catalog.json"
        self.entries = load_json(routines_path)["trainers"]
        self.catalog = {t["slug"]: t for t in load_json(catalog_path)["trainers"]}
        self.problems = []
        self.warnings = []
        self.adj = {}
        for n in wg.nodes:
            self.adj[n.id] = [(e.target, e.kind) for e in n.edges]
        self.trainers = []
        self.candidates, self.hops = [], []
        sim = [e for e in self.entries if e.get("simulated")]
        if len(sim) != SIM_COUNT:
            self.problems.append("routines.json has %d simulated trainers, expected %d"
                                 % (len(sim), SIM_COUNT))
        order = [t["slug"] for t in load_json(catalog_path)["trainers"]]
        if [e["slug"] for e in sim] != [s for s in order if s in {e["slug"] for e in sim}]:
            self.problems.append("routines.json simulated trainers are not in catalog order")
        for entry in sim:
            try:
                self.trainers.append(self.resolve(entry))
            except BuildError as err:
                self.problems.append(str(err))

    # Graph search ---------------------------------------------------------

    def allowed(self, t):
        nodes = self.wg.nodes

        def neighbours(nid):
            for target, kind in self.adj[nid]:
                if kind == graph.KIND_TRANSIT and not t.traveller:
                    continue
                if not t.traveller and nodes[target].region not in t.regions:
                    continue
                yield target
        return neighbours

    def bfs(self, start, neighbours, limit=None):
        dist = {start: 0}
        queue = deque([start])
        while queue:
            cur = queue.popleft()
            if limit is not None and dist[cur] >= limit:
                continue
            for nxt in neighbours(cur):
                if nxt not in dist:
                    dist[nxt] = dist[cur] + 1
                    queue.append(nxt)
        return dist

    # Resolution -----------------------------------------------------------

    def resolve(self, e):
        wg = self.wg
        tag = e.get("name", e.get("slug"))
        cat = self.catalog.get(e["slug"])
        if cat is None:
            raise BuildError("%s: not in the notable-trainer catalog" % tag)
        t = Trainer()
        t.entry, t.slug, t.name = e, e["slug"], cat["name"]
        t.character = e["character"]
        expect = "NOTABLE_TRAINER_" + e["slug"].upper().replace("-", "_")
        if t.character != expect:
            raise BuildError("%s: character %s, expected %s" % (tag, t.character, expect))
        t.traveller, t.aloof = bool(cat["traveller"]), bool(cat["aloof"])
        t.leader = cat["role"] == "Gym Leader"
        t.graphics = e["graphics"]
        sheet = sprite_sheet(self.root, t.graphics)
        t.sheet = sheet
        if sheet is None or tuple(sheet) != FULL_SHEET:
            what = ("%s: %s is not a Full 9-frame sheet (sAnimTable_Standard, 16x32 frames, "
                    "144x32 sheet); found %r" % (tag, t.graphics, sheet))
            if e.get("sprite_waiver"):
                self.warnings.append(what + "; waived: " + e["sprite_waiver"])
            else:
                raise BuildError(what)
        home = wg.world.maps.get(e["home"])
        if home is None or not home.in_scope:
            raise BuildError("%s: home map %s is not in scope" % (tag, e["home"]))
        t.home = home
        t.regions = {home.region}
        if cat.get("homeRegion") in REGION_INDEX:
            t.regions.add(cat["homeRegion"])
        t.radius = RADIUS_TRAVELLER if t.traveller else RADIUS_NARROW
        t.flags = (FLAG_TRAVELLER if t.traveller else 0) | (FLAG_ALOOF if t.aloof else 0) | \
            (FLAG_GYM_LEADER if t.leader else 0)
        home_nodes = wg.map_nodes(home.name)
        if not home_nodes:
            raise BuildError("%s: home map %s has no node" % (tag, home.name))
        per_node = {n.id: 0 for n in home_nodes}
        for s in wg.spots:
            if s.map is home:
                per_node[s.node] += 1
        t.home_spots = [i for i, s in enumerate(wg.spots) if s.map is home]
        neighbours = self.allowed(t)
        t.home_node = max(sorted(per_node), key=lambda nid: per_node[nid])

        # Gym (leaders): the node a door from the home map leads into.
        t.gym_node, t.own_gym_spot, t.gym_map = NODE_NONE, SPOT_NONE, None
        t.leader_local_id = 0
        t.alt_graphics = sorted(sprite_family(self.root, t.graphics) - {t.graphics})[:2]
        if t.leader:
            gym = wg.world.maps.get(e.get("gym", ""))
            if gym is None or not gym.in_scope:
                raise BuildError("%s: Gym map %r is not in scope" % (tag, e.get("gym")))
            t.gym_map = gym
            doors = sorted(edge.target for n in home_nodes for edge in n.edges
                           if edge.kind == graph.KIND_WARP and wg.nodes[edge.target].map is gym)
            if not doors:
                raise BuildError("%s: no walkable door from %s into %s" % (tag, home.name, gym.name))
            t.gym_node = doors[0]
            # A leader's home node must reach their Gym and back: where the
            # home map is split (Sootopolis's rings, Vermilion's Gym yard),
            # take the most-spots node among those that do.
            linked = [nid for nid in sorted(per_node)
                      if t.gym_node in self.bfs(nid, neighbours)
                      and nid in self.bfs(t.gym_node, neighbours)]
            if linked:
                t.home_node = max(linked, key=lambda nid: per_node[nid])
            own = [i for i, s in enumerate(wg.spots) if s.map is gym and s.kind == sp.GYM]
            if len(own) != 1:
                raise BuildError("%s: %s has %d Gym spots, expected 1" % (tag, gym.name, len(own)))
            t.own_gym_spot = own[0]
            # The Gym's own leader object (the badge battle's), by local id:
            # Fuchsia's Gym has four decoys with Janine's sprite, so the
            # sprite alone can't tell. Among the objects drawn with the
            # leader's sprite family, the one running "<map>_EventScript_<Name>".
            family = sprite_family(self.root, t.graphics)
            drawn = [o for o in gym.events["objects"] if o["gfx"] in family]
            key = re.sub(r"[^A-Za-z]", "", t.name)
            named = [o for o in drawn if o["script"].endswith("_EventScript_" + key)]
            pick = named or drawn
            if len(pick) != 1:
                raise BuildError("%s: %s has %d leader objects (%s), expected 1"
                                 % (tag, gym.name, len(pick), ", ".join(o["script"] for o in pick)))
            t.leader_local_id = pick[0]["local_id"]
            badge = e.get("badge") or {}
            if badge.get("region") not in ("Kanto", "Johto", "Hoenn") or \
                    not 0 <= badge.get("index", -1) <= 7:
                raise BuildError("%s: Gym Leader needs a badge region and index" % tag)
            t.badge_region, t.badge_index = REGION_INDEX[badge["region"]], badge["index"]
        else:
            t.badge_region, t.badge_index = REGION_NONE, 0

        # Home place (non-leaders), by spot order on the home map.
        t.home_place = SPOT_NONE
        if not t.leader:
            # Only spots on nodes mutually reachable with the home node, over
            # this trainer's edges (Sootopolis's rings are separate nodes).
            out_of_home = self.bfs(t.home_node, neighbours)
            mutual = {nid for nid in out_of_home
                      if t.home_node in self.bfs(nid, neighbours)}
            home_ok = [i for i in t.home_spots if wg.spots[i].node in mutual]
            named_home = [i for i in home_ok if wg.spots[i].kind == sp.NAMED
                          and spot_offers(wg.spots[i], "home")]
            squares = [i for i in home_ok if wg.spots[i].kind == sp.SQUARE]
            for pick in (named_home, squares, home_ok):
                if pick:
                    t.home_place = pick[0]
                    break

        # Cycle.
        cycle = e.get("cycle", [])
        if not 3 <= len(cycle) <= 4 or any(a not in ACTIVITY_INDEX for a in cycle):
            raise BuildError("%s: cycle %r needs 3-4 steps from %s" % (tag, cycle, ", ".join(ACTIVITIES)))
        t.cycle = [ACTIVITY_INDEX[a] for a in cycle]

        # Candidates: every spot on nodes within the radius, by (hops, id).
        dist = self.bfs(t.home_node, neighbours, t.radius)
        cands = sorted((dist[s.node], i) for i, s in enumerate(wg.spots) if s.node in dist)
        t.candidates = [i for _, i in cands]
        t.candidate_hops = [h for h, _ in cands]
        if not any(wg.spots[i].map is home for i in t.candidates):
            self.problems.append("%s: the home node can't reach a spot on %s" % (tag, home.name))

        # Favourites.
        t.favourites = []
        favs = e.get("favourites", [])
        if len(favs) > 3:
            raise BuildError("%s: more than 3 favourites" % tag)
        t.disabled = []
        for f in favs:
            if f.get("disabled"):
                t.disabled.append({"favourite": f.get("label") or f.get("note"),
                                   "map": f.get("map"), "why": f["disabled"]})
                continue
            fav = self.favourite(t, f, cycle)
            nodes = {wg.spots[i].node for i in range(fav[0], fav[0] + fav[1])}
            far = sorted(n for n in nodes if n not in self.bfs(t.home_node, neighbours))
            if far:
                n = wg.nodes[far[0]]
                raise BuildError("%s: favourite %s (%s) is unreachable from the home node "
                                 "over this trainer's edges (node %d at %d,%d); mark it "
                                 "disabled with a reason to keep it out of the table"
                                 % (tag, f.get("label") or f.get("note"), f.get("map"),
                                    n.id, n.x, n.y))
            t.favourites.append(fav)

        # Every cycle step has a candidate, unless the routine allows the
        # step to be skipped (the runtime skips a step with no candidate).
        allow_skip = e.get("allow_skip", {})
        t.skips = []
        for activity in cycle:
            if activity == "home":
                if t.leader or t.home_place != SPOT_NONE:
                    continue
                self.problems.append("%s: home step has no home place on %s" % (tag, home.name))
                continue
            if any(fav[3] == ACTIVITY_INDEX[activity] for fav in t.favourites):
                continue
            ok = any(spot_offers(wg.spots[i], activity)
                     and not (t.aloof and wg.spots[i].flags & sp.FLAG_PUBLIC)
                     and i != t.own_gym_spot for i in t.candidates)
            if not ok and activity in allow_skip:
                t.skips.append({"activity": activity, "why": allow_skip[activity]})
            elif not ok:
                self.problems.append("%s: %s step has no favourite and no candidate within %d hops"
                                     % (tag, activity, t.radius))

        # Search bound: the longest shortest path among the nodes the
        # trainer can be at or head for; at least twice the radius.
        targets = {t.home_node} | {wg.spots[i].node for i in t.candidates}
        if t.gym_node != NODE_NONE:
            targets.add(t.gym_node)
        if t.home_place != SPOT_NONE:
            targets.add(wg.spots[t.home_place].node)
        for first, count, _, _ in t.favourites:
            targets |= {wg.spots[i].node for i in range(first, first + count)}
        worst = 0
        stuck = []
        for a in sorted(targets):
            d = self.bfs(a, neighbours)
            for b in sorted(targets):
                if b not in d:
                    stuck.append((a, b))
                else:
                    worst = max(worst, d[b])
        if stuck:
            a, b = stuck[0]
            na, nb = wg.nodes[a], wg.nodes[b]
            self.problems.append(
                "%s: could get stuck: no path from node %d (%s %d,%d) to node %d (%s %d,%d)"
                " over this trainer's edges (%d unreachable pairs)"
                % (tag, a, na.map.name, na.x, na.y, b, nb.map.name, nb.x, nb.y, len(stuck)))
        t.search_bound = max(worst, 2 * t.radius)
        if t.search_bound > 255:
            self.problems.append("%s: search bound %d does not fit a u8" % (tag, t.search_bound))
        if not t.traveller and not stuck:
            self.check_rom_paths(t, tag, sorted(targets), neighbours)
        t.targets = len(targets)
        return t

    def check_rom_paths(self, t, tag, targets, neighbours):
        """The ROM searches without the region limit (SearchRun and the
        home-hop table only skip transit edges), so a non-traveller's
        in-ROM paths must be the region-limited ones this build measured:
        between any two of its targets the ROM's breadth-first path (edges
        in table order, first discovery wins) stays in the home regions and
        has the same length, and the home-hop table agrees on every node of
        those paths. A map change that opened a shorter way through another
        region fails here instead of letting the ROM leave the region."""
        nodes = self.wg.nodes
        # Reroutes (the ROM's search avoiding full maps) can take any simple
        # path, not just the shortest. A simple path that leaves the home
        # regions must come back in at a different region node, so it is
        # impossible while every out-of-region part of the graph touches the
        # regions at one node at most (today: Route 22 <-> the reception
        # gate). Checked over the undirected, transit-free graph.
        touching = defaultdict(set)
        for n in nodes:
            for e in n.edges:
                if e.kind != graph.KIND_TRANSIT:
                    touching[n.id].add(e.target)
                    touching[e.target].add(n.id)
        seen = set()
        for start in range(len(nodes)):
            if nodes[start].region in t.regions or start in seen:
                continue
            part, queue, portals = {start}, deque([start]), set()
            while queue:
                cur = queue.popleft()
                for nxt in touching[cur]:
                    if nodes[nxt].region in t.regions:
                        portals.add(nxt)
                    elif nxt not in part:
                        part.add(nxt)
                        queue.append(nxt)
            seen |= part
            if len(portals) > 1:
                names = ", ".join("%d (%s)" % (p, nodes[p].map.name) for p in sorted(portals)[:4])
                self.problems.append("%s: an out-of-region part of the graph (%d nodes) touches %s at "
                                     "%d nodes (%s): a reroute could leave the region and come back"
                                     % (tag, len(part), "/".join(sorted(t.regions)), len(portals), names))
                return
        free = {}
        for nid in range(len(nodes)):
            free[nid] = [tg for tg, kind in self.adj[nid] if kind != graph.KIND_TRANSIT]
        visited = set()
        for a in targets:
            limited = self.bfs(a, neighbours)
            parent, depth = {a: None}, {a: 0}
            queue = deque([a])
            while queue:
                cur = queue.popleft()
                for nxt in free[cur]:
                    if nxt not in parent:
                        parent[nxt], depth[nxt] = cur, depth[cur] + 1
                        queue.append(nxt)
            for b in targets:
                path, cur = [], b
                while cur is not None:
                    path.append(cur)
                    cur = parent.get(cur)
                outside = [n for n in path if nodes[n].region not in t.regions]
                if depth.get(b) != limited.get(b) or outside:
                    n = nodes[outside[0]] if outside else nodes[b]
                    self.problems.append(
                        "%s: the ROM's path from node %d to node %d (no region limit) %s "
                        "(node %d, %s %d,%d); the build measured %s hops in %s"
                        % (tag, a, b, "leaves the home regions" if outside else
                           "is %s hops" % depth.get(b), n.id, n.map.name, n.x, n.y,
                           limited.get(b), "/".join(sorted(t.regions))))
                    return
                visited.update(path)
        # emit.home_hops: the unrestricted reverse distance to home, against
        # the region-limited one (an edge counts if its target is in region).
        reverse = defaultdict(list)
        for n in nodes:
            for e in n.edges:
                if e.kind != graph.KIND_TRANSIT:
                    reverse[e.target].append(n.id)

        def hops_home(limit):
            dist = {t.home_node: 0}
            queue = deque([t.home_node])
            while queue:
                cur = queue.popleft()
                if limit and nodes[cur].region not in t.regions:
                    continue
                for prev in reverse[cur]:
                    if prev not in dist:
                        dist[prev] = dist[cur] + 1
                        queue.append(prev)
            return dist
        rom, limited = hops_home(False), hops_home(True)
        for n in sorted(visited):
            if limited.get(n) != rom.get(n):
                self.problems.append("%s: the home-hop table gives node %d (%s) %s hops home; "
                                     "the region-limited search %s"
                                     % (tag, n, nodes[n].map.name, rom.get(n), limited.get(n)))
                return

    def favourite(self, t, f, cycle):
        wg = self.wg
        tag = "%s: favourite %s (%s)" % (t.name, f.get("label") or f.get("note"), f.get("map"))
        info = wg.world.maps.get(f.get("map"))
        if info is None or not info.in_scope:
            raise BuildError(tag + ": map not in Wayfarer's spot scope")
        serves = f.get("serves")
        if serves not in ACTIVITY_INDEX:
            raise BuildError(tag + ": unknown activity %r" % serves)
        if serves not in cycle:
            raise BuildError(tag + ": serves %s, which is not a step of the cycle" % serves)
        kind = sp.KIND_BY_KEY.get(f.get("kind"))
        if kind is None:
            raise BuildError(tag + ": unknown kind %r" % f.get("kind"))
        if kind == sp.NAMED:
            run = [i for i, s in enumerate(wg.spots) if s.map is info and s.kind == sp.NAMED
                   and s.label == f.get("label")]
        else:
            rows = f.get("rows", [0, 255])
            run = [i for i, s in enumerate(wg.spots) if s.map is info and s.kind == kind
                   and ("x" not in f or (s.x, s.y) == (f["x"], f["y"]))
                   and rows[0] <= s.y <= rows[1]]
        if not run:
            raise BuildError(tag + ": resolves to no %s spot" % f.get("kind"))
        if run != list(range(run[0], run[0] + len(run))):
            raise BuildError(tag + ": spots are not contiguous")
        if len(run) > 255:
            raise BuildError(tag + ": %d spots, over the u8 spotCount" % len(run))
        for i in run:
            s = wg.spots[i]
            if not spot_offers(s, serves):
                raise BuildError(tag + ": the spot does not offer %s" % serves)
            if t.aloof and s.flags & sp.FLAG_PUBLIC:
                raise BuildError(tag + ": public spot for an aloof trainer")
            if i == t.own_gym_spot:
                raise BuildError(tag + ": the trainer's own Gym")
        if not t.traveller and info.region not in t.regions:
            raise BuildError(tag + ": in %s, outside the home region of a non-traveller" % info.region)
        return (run[0], len(run), info.name, ACTIVITY_INDEX[serves])

    def check(self):
        if self.problems:
            raise BuildError("\n".join(self.problems))
