"""Authored inputs: named spots, overrides, transit links."""

from maps import BuildError, INTERIOR_TYPES, TOOL_DIR, load_json
from spots import DIR_NAMES, KIND_BY_KEY, KIND_KEYS, NAMED, Spot

ACTIVITIES = ["care", "shop", "gamble", "train", "relax", "fish", "visit",
              "study", "home", "sightsee", "lie low"]
ACTIVITY_INDEX = {a: i for i, a in enumerate(ACTIVITIES)}
ACTIVITY_NONE = 15
TEMPLATES = {"stand_and_face": 1, "sit_or_idle": 2}
# Named spot facing and second-trainer order (spots spec, named spots).
NAMED_ORDER = ((0, -1), (-1, 0), (1, 0), (0, 1))  # up, left, right, down


class Overrides:
    """overrides/<region>.json: adds (benches, Center seats) and drops."""

    def __init__(self, directory=None):
        directory = directory or (TOOL_DIR / "overrides")
        self.adds, self.drop_places, self.drop_tiles = [], set(), set()
        self.seats = {}
        self.ignore_objects = {}
        self.script_warps = {}
        self.used_drops = set()
        self.files = []
        for path in sorted(directory.glob("*.json")):
            self.files.append(path)
            data = load_json(path)
            for entry in data.get("add", []):
                entry = dict(entry, source=path.name)
                if entry.get("facing") not in DIR_NAMES[1:]:
                    raise BuildError("%s: add %s needs a DIR_* facing" % (path.name, entry))
                entry["facing"] = DIR_NAMES.index(entry["facing"])
                if entry.get("kind") not in ("bench", "center_side"):
                    raise BuildError("%s: add kind must be bench or center_side" % path.name)
                self.adds.append(entry)
                if entry["kind"] == "center_side":
                    self.seats.setdefault(entry["map"], []).append(entry)
            for entry in data.get("ignore_objects", []):
                self.ignore_objects[(entry["map"], entry["local_id"])] = entry.get("why", "")
            for entry in data.get("script_warps", []):
                ids = entry.get("warps")
                if not entry.get("map") or not ids or not all(isinstance(k, int) for k in ids):
                    raise BuildError("%s: script_warps %s needs a map and warp ids" % (path.name, entry))
                for k in ids:
                    self.script_warps[(entry["map"], k)] = entry.get("why", "")
            for entry in data.get("drop", []):
                kind = entry.get("kind")
                if kind is not None and kind not in KIND_BY_KEY:
                    raise BuildError("%s: unknown kind %r" % (path.name, kind))
                if "x" in entry:
                    self.drop_tiles.add((entry["map"], entry["x"], entry["y"],
                                         KIND_BY_KEY.get(kind)))
                elif kind is not None:
                    self.drop_places.add((entry["map"], KIND_BY_KEY[kind]))
                else:
                    raise BuildError("%s: a drop needs a kind or a tile" % path.name)

    def dropped(self, spot):
        name = spot.map.name
        hit = False
        # Every entry that matches counts as used, not just the first: a spot
        # covered by both a place drop and a tile drop uses both.
        for key, table in (((name, spot.kind), self.drop_places),
                           ((name, spot.x, spot.y, None), self.drop_tiles),
                           ((name, spot.x, spot.y, spot.kind), self.drop_tiles)):
            if key in table:
                self.used_drops.add(key)
                hit = True
        return hit

    def dropped_place(self, name, kind):
        """A whole detected place (a Mart, a Gym) dropped by `drop`."""
        if (name, kind) in self.drop_places:
            self.used_drops.add((name, kind))
            return True
        return False

    def unused_drops(self):
        """`drop` entries that matched no detected place or spot: a typo in
        a map, tile or kind would otherwise drop nothing, silently."""
        out = []
        for key in sorted(self.drop_places | self.drop_tiles, key=repr):
            if key not in self.used_drops:
                if len(key) == 2:
                    out.append("drop: %s has no detected %s place or spot" % (key[0], KIND_KEYS[key[1]]))
                else:
                    kind = "spot" if key[3] is None else KIND_KEYS[key[3]] + " spot"
                    out.append("drop: %s has no detected %s at (%d, %d)" % (key[0], kind, key[1], key[2]))
        return out


def load_named(path=None):
    return load_json(path or TOOL_DIR / "named_spots.json")["spots"]


def build_named(rows, world, detector, problems):
    """Validate named-spot rows (spots spec, named spots) and make spots.

    Reachability is checked later, once edges are known: some edge must lead
    into a named tile's component. That check is local (one edge back), so a
    named spot in a part of the graph no home reaches still passes; it is
    never chosen, because candidates and favourites come from searches out of
    each trainer's home."""
    out, seen = [], {}
    for row in rows:
        tag = "named spot %r (%s)" % (row.get("label"), row.get("map"))
        info = world.maps.get(row.get("map"))
        bad = []
        if info is None or getattr(info, "json", None) is None:
            problems.append(tag + ": map does not exist")
            continue
        if not info.in_scope:
            problems.append(tag + ": map is not in Wayfarer's spot scope")
            continue
        if info.region != row.get("region"):
            bad.append("map is in %s, the row is listed under %s" % (info.region, row.get("region")))
        acts = row.get("activities") or []
        if not 1 <= len(acts) <= 2 or any(a not in ACTIVITY_INDEX for a in acts):
            bad.append("activities %r: one or two from %s" % (acts, ", ".join(ACTIVITIES)))
        template = row.get("template")
        if template is not None and template not in TEMPLATES:
            bad.append("template %r: stand_and_face or sit_or_idle" % template)
        ctx = detector.context(info)
        x, y = row.get("x"), row.get("y")
        if not isinstance(x, int) or not ctx.grid.inside(x, y):
            bad.append("tile outside the map")
        else:
            if (x, y) in ctx.object_tiles or (x, y) in ctx.warp_tiles:
                bad.append("tile (%d, %d) holds an object or a warp" % (x, y))
            elif not ctx.flood.in_any(x, y):
                bad.append("tile (%d, %d) is not walkable land" % (x, y))
            if (info.name, x, y) in seen:
                bad.append("tile is also named spot %r" % seen[(info.name, x, y)])
        cap = row.get("capacity")
        indoor = info.map_type in INTERIOR_TYPES
        if cap not in (1, 2) or (indoor and cap != 1):
            bad.append("capacity %r over the %s cap" % (cap, "interior 1" if indoor else "outdoor 2"))
        if bad:
            problems += [tag + ": " + b for b in bad]
            continue
        seen[(info.name, x, y)] = row["label"]
        facing = 1  # DIR_SOUTH: faces down without a feature beside it
        g = ctx.grid
        features = detector.counter | ctx.shelf_kinds | detector.fishable
        for dx, dy in NAMED_ORDER:
            nx, ny = x + dx, y + dy
            if g.inside(nx, ny) and ((nx, ny) in ctx.object_tiles or (nx, ny) in ctx.signs
                                     or g.b(nx, ny) in features):
                facing = {(0, -1): 2, (-1, 0): 3, (1, 0): 4, (0, 1): 1}[(dx, dy)]
                break
        s = Spot(info, NAMED, x, y, facing, ctx.comp(x, y))
        s.label = row["label"]
        s.capacity = cap
        first = ACTIVITY_INDEX[acts[0]]
        second = ACTIVITY_INDEX[acts[1]] if len(acts) > 1 else ACTIVITY_NONE
        s.activities = first | (second << 4)
        s.template = TEMPLATES.get(template, 0)
        if cap == 2:
            s.second = next(((x + dx, y + dy) for dx, dy in NAMED_ORDER
                             if ctx.free(x + dx, y + dy)
                             and (info.name, x + dx, y + dy) not in seen), None)
            if s.second is None:
                problems.append(tag + ": capacity 2 but no free 4-neighbour for a second trainer")
                continue
        out.append(s)
    return out


def load_transit(path=None):
    return load_json(path or TOOL_DIR / "transit.json")["links"]
