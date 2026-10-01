#!/usr/bin/env python3
"""Draft inventory of the notable-spots pool, read-only.

Applies the detection rules in .product/specs/notable-spots.md to the real
map data under game/ and writes:

  per-map.csv     one row per scanned map, counts per spot kind
  inventory.json  scope, per-map detail, per-region totals, findings

next to this script, and prints the per-region tables as Markdown. Nothing
under game/ is written. Python 3 standard library only.

    python3 .product/research/notable-spots-inventory/inventory.py

This is a sanity check of the spec's rules, not the build-time tool: it
reimplements the Wayfarer map selection from game/tools/mapjson/mapjson.cpp
in a simplified form (build_scope below; the method is in
.product/research/notable-spots-inventory.md).
"""

import csv
import fnmatch
import json
import re
import struct
import sys
from collections import Counter, defaultdict, deque
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
GAME = REPO / "game"
MAPS = GAME / "data/maps"

REGIONS = ("Kanto", "Johto", "Hoenn", "Sevii")
OUTDOOR_TYPES = {"MAP_TYPE_TOWN", "MAP_TYPE_CITY", "MAP_TYPE_ROUTE",
                 "MAP_TYPE_OCEAN_ROUTE"}
SQUARE_TYPES = {"MAP_TYPE_TOWN", "MAP_TYPE_CITY"}

NURSE_GFX = {"OBJ_EVENT_GFX_NURSE", "OBJ_EVENT_GFX_NURSE_FRLG",
             "OBJ_EVENT_GFX_NURSE_HNS", "OBJ_EVENT_GFX_NURSE_CHANSEY_HNS"}
CLERK_GFX = {"OBJ_EVENT_GFX_MART_EMPLOYEE", "OBJ_EVENT_GFX_MART_EMPLOYEE_HNS",
             "OBJ_EVENT_GFX_CLERK"}
GYM_MUSIC = {"MUS_GYM", "MUS_HG_GYM", "MUS_RG_GYM"}
CORNER_MUSIC = {"MUS_GAME_CORNER", "MUS_HG_GAME_CORNER", "MUS_RG_GAME_CORNER"}
SLOT_SCRIPT = re.compile(r"_EventScript_SlotMachine\d*$")

PATCH_MIN = 6        # spec placeholder: smallest tall-grass patch
SQUARE_CLEARANCE = 3  # spec placeholder: tiles from any warp, sign, object

# Notable trainers (spec catalog) whose sprites mark cameo objects.
NOTABLES = ("BROCK MISTY SURGE LT_SURGE ERIKA JANINE KOGA SABRINA BLAINE "
            "GIOVANNI BLUE FALKNER BUGSY WHITNEY MORTY CHUCK JASMINE PRYCE "
            "CLAIR LANCE ROXANNE BRAWLY WATTSON FLANNERY NORMAN WINONA TATE "
            "LIZA JUAN WALLACE STEVEN LORELEI BRUNO AGATHA WILL KAREN SIDNEY "
            "PHOEBE GLACIA DRAKE").split()
NOTABLE_GFX = re.compile(r"^OBJ_EVENT_GFX_(%s)(_HNS|_FRLG)?$" % "|".join(NOTABLES))
# Sprites that are not people: items, field obstacles, props, vehicles.
PROP_GFX = re.compile(
    r"BALL|BERRY|MON_BASE|CUTTABLE|BREAKABLE|BOULDER|LIGHT|FOSSIL|TRUCK|"
    r"BOAT|FERRY|SS_TIDAL|SUBMARINE|DOLL|CUSHION|STATUE|MACHINE|_PC\b|"
    r"SIGN|ITEM|^OBJ_EVENT_GFX_VAR_|CABLE_CAR|METEORITE|MIRAGE|"
    r"MAIL|BOX|PROF_OAK_?POC|BIRTH_ISLAND|ORB|EGG|APRICORN|SHADOW|ARROW|"
    r"DEOXYS|UNKNOWN|INVISIBLE")
# Villain team members: story blockers even when always present.
VILLAIN_GFX = re.compile(r"ROCKET|AQUA_MEMBER|MAGMA_MEMBER|GRUNT|ARCHER|ARIANA|"
                         r"PROTON|PETREL|MAXIE|ARCHIE|TABITHA|SHELLY|MATT|COURTNEY")
STATIONARY = re.compile(r"^MOVEMENT_TYPE_(FACE_\w+|LOOK_AROUND)$")
ACTOR_CMDS = re.compile(
    r"^\s*(applymovement|removeobject|addobject|setobjectxy|setobjectxyperm|"
    r"copyobjectxytoperm|moveobjectoffscreen|showobjectat|hideobjectat)\s+"
    r"(\w+)", re.M)
WARP_CMDS = re.compile(r"^\s*warp\w*\s+(MAP_\w+)", re.M)

DIRS = {"up": (0, -1), "down": (0, 1), "left": (-1, 0), "right": (1, 0)}
N4 = ((0, -1), (0, 1), (-1, 0), (1, 0))


def load_json(path):
    return json.loads(Path(path).read_text(encoding="utf-8"))


# --------------------------------------------------------------------------
# Constants: metatile behaviours, FRLG normalisation, species.
# --------------------------------------------------------------------------

def behaviour_enum():
    text = (GAME / "include/constants/metatile_behaviors.h").read_text()
    body = text[text.index("enum {") + 6:text.index("};")]
    names = [re.sub(r"//.*", "", line).strip().rstrip(",")
             for line in body.splitlines()]
    return {name: index for index, name in enumerate(n for n in names if n)}


MB = behaviour_enum()


def frlg_normaliser():
    frlg = {m.group(1): int(m.group(2), 16) for m in re.finditer(
        r"#define (MB_FRLG_\w+) (0x[0-9A-Fa-f]+)",
        (GAME / "include/constants/metatile_behaviors_frlg.h").read_text())}
    text = (GAME / "src/fieldmap.c").read_text()
    body = text[text.index("NormalizeFrlgMetatileBehavior(u32"):]
    body = body[:body.index("default:")]
    table = {}
    for src, dst in re.findall(r"case (MB_FRLG_\w+):\s*return (MB_\w+);", body):
        table[frlg[src]] = MB[dst]
    return table


FRLG_NORMALISE = frlg_normaliser()

TALL_GRASS = {MB["MB_TALL_GRASS"], MB["MB_CYCLING_ROAD_PULL_DOWN_GRASS"],
              MB["MB_TALL_GRASS_IMPASSABLE_NORTH"]}
LONG_GRASS = {MB["MB_LONG_GRASS"]}
FISHABLE = {MB[n] for n in (
    "MB_POND_WATER", "MB_OCEAN_WATER", "MB_INTERIOR_DEEP_WATER",
    "MB_DEEP_WATER", "MB_SOOTOPOLIS_DEEP_WATER", "MB_EASTWARD_CURRENT",
    "MB_WESTWARD_CURRENT", "MB_NORTHWARD_CURRENT", "MB_SOUTHWARD_CURRENT")}
NOT_LAND = FISHABLE | {MB["MB_WATERFALL"]}
SHELF = {MB["MB_SHOP_SHELF"], MB["MB_SHOP_SHELF_DEPARTMENT"],
         MB["MB_SHOP_SHELF_DEPARTMENT_FORWARD"]}
COUNTER = {MB["MB_COUNTER"]}
SLOT_TILE = {MB["MB_SLOT_MACHINE"]}
DOOR = {MB["MB_ANIMATED_DOOR"], MB["MB_NON_ANIMATED_DOOR"],
        MB["MB_PETALBURG_GYM_DOOR"]}
LEDGE = {MB["MB_JUMP_EAST"]: (1, 0), MB["MB_JUMP_WEST"]: (-1, 0),
         MB["MB_JUMP_NORTH"]: (0, -1), MB["MB_JUMP_SOUTH"]: (0, 1)}
LEDGE |= {MB[k]: v for k, v in (("MB_JUMP_NORTHEAST", (1, -1)),
                                ("MB_JUMP_NORTHWEST", (-1, -1)),
                                ("MB_JUMP_SOUTHEAST", (1, 1)),
                                ("MB_JUMP_SOUTHWEST", (-1, 1))) if k in MB}

SPECIES = {m.group(1) for m in re.finditer(
    r"#define SPECIES_(\w+)\s+\d+",
    (GAME / "include/constants/species.h").read_text())} - {"NONE", "EGG"}


def is_pokemon_gfx(gfx):
    name = gfx.replace("OBJ_EVENT_GFX_", "")
    name = re.sub(r"(_\d+|_HNS|_FRLG|_FOLLOWER|_DOLL|_ASLEEP)+$", "", name)
    return name in SPECIES or "SPECIES" in gfx


# --------------------------------------------------------------------------
# Tilesets and layouts.
# --------------------------------------------------------------------------

HEADERS = (GAME / "src/data/tilesets/headers.h").read_text()
METATILES = (GAME / "src/data/tilesets/metatiles.h").read_text()
_attr_cache = {}


def tileset_attributes(tileset):
    if tileset in _attr_cache:
        return _attr_cache[tileset]
    match = re.search(
        rf"const struct Tileset {tileset} =\s*\{{.*?metatileAttributes = (\w+)",
        HEADERS, re.S)
    data = b""
    if match:
        path = re.search(rf'{match.group(1)}\[\] = INCBIN_U\d+\("([^"]+)"\)',
                         METATILES)
        if path:
            data = (GAME / path.group(1)).read_bytes()
    _attr_cache[tileset] = data
    return data


LAYOUTS = {layout["id"]: layout
           for layout in load_json(GAME / "data/layouts/layouts.json")["layouts"]
           if layout and "id" in layout}


def layout_family(layout):
    return layout.get("layout_version") or layout.get("game_version") or "emerald"


class Grid:
    """Behaviour, collision and elevation per tile, read as the engine does."""

    def __init__(self, layout):
        self.family = layout_family(layout)
        self.w, self.h = layout["width"], layout["height"]
        blocks = (GAME / layout["blockdata_filepath"]).read_bytes()
        primary = tileset_attributes(layout["primary_tileset"])
        secondary = tileset_attributes(layout["secondary_tileset"])
        n_primary = 512 if self.family == "emerald" else 640
        size = 4 if self.family == "frlg" else 2
        fmt = "<I" if size == 4 else "<H"
        self.mb, self.col, self.elev = [], [], []
        for i in range(self.w * self.h):
            block = struct.unpack_from("<H", blocks, 2 * i)[0]
            metatile = block & 0x3FF
            if metatile < n_primary:
                data, index = primary, metatile
            else:
                data, index = secondary, metatile - n_primary
            raw = (struct.unpack_from(fmt, data, index * size)[0]
                   if (index + 1) * size <= len(data) else 0)
            if self.family == "frlg":
                behaviour = FRLG_NORMALISE.get(raw & 0x1FF, raw & 0x1FF)
            else:
                behaviour = raw & 0xFF
            self.mb.append(behaviour)
            self.col.append((block >> 10) & 3)
            self.elev.append(block >> 12)

    def inside(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h

    def b(self, x, y):
        return self.mb[y * self.w + x]

    def land(self, x, y):
        """Walkable land: no collision bit, not water, not a ledge."""
        i = y * self.w + x
        return (self.col[i] == 0 and self.mb[i] not in NOT_LAND
                and self.mb[i] not in LEDGE and self.elev[i] != 1)


# --------------------------------------------------------------------------
# Wayfarer map selection (simplified port of mapjson.cpp).
# --------------------------------------------------------------------------

MAPJSON = (GAME / "tools/mapjson/mapjson.cpp").read_text()


def cpp_set(name):
    match = re.search(r"set<string> %s = \{(.*?)\};" % name, MAPJSON, re.S)
    return set(re.findall(r'"([^"]+)"', match.group(1)))


FRLG_CLOSED_SETS = set().union(*(cpp_set(n) for n in (
    "wayfarer_coast_map_names", "wayfarer_coast_layout_ids",
    "wayfarer_anne_map_names", "wayfarer_anne_layout_ids",
    "wayfarer_pokemon_tower_map_names", "wayfarer_pokemon_tower_layout_ids",
    "wayfarer_celadon_hideout_map_names",
    "wayfarer_celadon_hideout_layout_ids")))
HNS_DROPPED = cpp_set("retired_preview_ids") | cpp_set("replaced_hns_ids")

SEVII = load_json(GAME / "src/data/wayfarer_sevii_maps.json")
SEVII_DOMAINS = {v["owner"]: v["enabled"]
                 for v in SEVII["content_domains"].values()}
SEVII_RECORDS = {m["source_map"]: m for m in SEVII["maps"]
                 if m.get("enabled", SEVII["release_link_enabled"])}
SEVII_IDS = {m["map_id"] for m in SEVII_RECORDS.values()}

HOENN_POLICY = load_json(GAME / "tools/wayfarer_hoenn_content/classification.json")
# Hoenn systems kept in the ROM but left out of the spot pool: per-player
# secret bases and ticket-only event islands are not everyday places.
HOENN_SPOT_EXCLUDED_SYSTEMS = {"secret-bases", "event-islands"}


def hoenn_rule(name):
    for rule in HOENN_POLICY["rules"]:
        if any(fnmatch.fnmatchcase(name, g) for g in rule["globs"]):
            return rule
    return HOENN_POLICY["default"]


def wayfarer_selects(data):
    """Port of data_matches_version() for the wayfarer product."""
    version = data.get("game_version", "emerald")
    name, map_id = data["name"], data["id"]
    if version == "sinnoh":
        return True  # gated by its frozen manifest; tagged out of scope later
    if version == "hns":
        if map_id in HNS_DROPPED:
            return False
        if map_id.endswith("_PORT") and map_id.startswith(
                ("MAP_CINNABAR_ISLAND_", "MAP_POKEMON_MANSION_")):
            return False
        if name.endswith("_Port") and name.startswith(
                ("CinnabarIsland_", "PokemonMansion_")):
            return False
        return True
    if version == "frlg":
        if data.get("wayfarer_include"):
            return True
        if name in FRLG_CLOSED_SETS:
            return True
        return SEVII["release_link_enabled"] and name in SEVII_RECORDS
    return version == "emerald"


def wayfarer_events(data, all_maps):
    """Events as mapjson emits them for wayfarer (simplified)."""
    source = data
    if data.get("shared_events_map") in all_maps:
        source = all_maps[data["shared_events_map"]]
    events = {k: [dict(e) for e in source.get(k, [])]
              for k in ("object_events", "warp_events", "coord_events",
                        "bg_events")}
    record = SEVII_RECORDS.get(data["name"]) if data.get(
        "game_version") == "frlg" else None
    if record:
        retained = record["retained_events"]
        for kind in ("object_events", "coord_events", "bg_events"):
            kept = []
            for rule in retained.get(kind, []):
                if not SEVII_DOMAINS.get(rule.get("owner"), False):
                    continue
                event = dict(events[kind][rule["index"]])
                event.update(rule.get("overrides") or {})
                if rule.get("wayfarer_script"):
                    event["script"] = rule["wayfarer_script"]
                event["_sevii_owner"] = rule.get("owner")
                event["_sevii_role"] = rule.get("actor_role")
                kept.append(event)
            events[kind] = kept
        events["warp_events"] = [
            w for w in events["warp_events"]
            if w.get("dest_map") in SEVII_IDS or w.get("dest_map") in (
                "MAP_DYNAMIC", "MAP_UNDEFINED")]
    for kind, items in events.items():
        out = []
        for event in items:
            if event.get("wayfarer_exclude"):
                continue
            for field in ("graphics_id", "script", "flag", "dest_map",
                          "dest_warp_id", "var"):
                if event.get("wayfarer_" + field) not in (None, ""):
                    event[field] = event["wayfarer_" + field]
            out.append(event)
        events[kind] = out
    return events


def wayfarer_connections(data):
    connections = data.get("connections") or []
    if data.get("game_version") == "frlg" and data["name"] in SEVII_RECORDS:
        connections = [c for c in connections if c.get("map") in SEVII_IDS]
    return connections


# --------------------------------------------------------------------------
# Region tagging.
# --------------------------------------------------------------------------

MAPSEC_ORDER = [s["id"] for s in load_json(
    GAME / "src/data/region_map/region_map_sections.json")["map_sections"]]
MAPSEC_INDEX = {mapsec: i for i, mapsec in enumerate(MAPSEC_ORDER)}
SEVII_MAPSECS = set(MAPSEC_ORDER[MAPSEC_INDEX["MAPSEC_ONE_ISLAND"]:
                                 MAPSEC_INDEX["MAPSEC_EMBER_SPA"] + 1])

MAP_GROUPS = load_json(MAPS / "map_groups.json")
GROUP_OF = {name: group for group in MAP_GROUPS["group_order"]
            for name in MAP_GROUPS.get(group, [])}
JOHTO_GROUP = re.compile(r"Indoor(NewBark|Cherrygrove|Violet|Azalea|Goldenrod|"
                         r"Ecruteak|Olivine|Cianwood|Mahogany|Blackthorn|"
                         r"JohtoRoutes)_Hns")
KANTO_GROUP = re.compile(r"Indoor(Pallet|Viridian|Pewter|Cerulean|Vermilion|"
                         r"Lavender|Celadon|Saffron|Fuchsia|Cinnabar|Indigo|"
                         r"KantoRoutes)_Hns")


def region_of(data):
    """Kanto, Johto, Hoenn, Sevii, or None (out of scope)."""
    version = data.get("game_version", "emerald")
    if version == "emerald":
        return "Hoenn"
    if version == "frlg":
        return "Sevii" if data["region_map_section"] in SEVII_MAPSECS else "Kanto"
    if version == "hns":
        region = data.get("region")
        if region == "REGION_KANTO":
            return "Kanto"
        if region == "REGION_JOHTO":
            return "Johto"
        if region:
            return None  # Alola, Hisui
        group = GROUP_OF.get(data["name"], "")
        if JOHTO_GROUP.search(group):
            return "Johto"
        if KANTO_GROUP.search(group):
            return "Kanto"
        return None
    return None  # Sinnoh


# --------------------------------------------------------------------------
# Scope: selected, in map_groups, in a Wayfarer region, reachable.
# --------------------------------------------------------------------------

def load_maps():
    maps = {}
    for path in sorted(MAPS.glob("*/map.json")):
        data = load_json(path)
        maps[data["name"]] = data
    return maps


def scripts_text(data, all_maps):
    texts = []
    for name in {data["name"], data.get("shared_scripts_map")} - {None}:
        path = MAPS / name / "scripts.inc"
        if path.exists():
            texts.append(path.read_text(encoding="utf-8", errors="replace"))
    return "\n".join(texts)


def vendor_labels():
    """Script labels whose body opens a shop (pokemart*), from all .inc files."""
    labels = set()
    files = list(MAPS.glob("*/scripts.inc")) + list((GAME / "data/scripts").rglob("*.inc"))
    for path in files:
        current = []
        for line in path.read_text(errors="replace").splitlines():
            match = re.match(r"^(\w+)::?\s*$", line)
            if match:
                current.append(match.group(1))
                continue
            if re.match(r"^\s+pokemart", line):
                labels.update(current)
            elif re.match(r"^\s+(end|return|releaseall|release)\b", line):
                current = []
    return labels


VENDORS = vendor_labels()


def build_scope(all_maps):
    by_id, excluded = {}, Counter()
    excluded_names = defaultdict(list)
    for name, data in all_maps.items():
        reason = None
        if name not in GROUP_OF:
            reason = "not in map_groups.json"
        elif name.startswith("TestMap"):
            reason = "debug test map"
        elif not wayfarer_selects(data):
            reason = "not selected for wayfarer by mapjson"
        elif data.get("game_version") == "sinnoh":
            reason = "Sinnoh (outside the four spot regions)"
        elif region_of(data) is None:
            reason = ("HNS map outside Kanto and Johto (Alola, Sinjoh, and HNS "
                      "copies of Hoenn frontier, contest, event, link maps)")
        elif data.get("game_version", "emerald") == "emerald":
            rule = hoenn_rule(name)
            if rule["classification"] == "excluded":
                reason = "Hoenn policy: excluded"
            elif rule.get("system") in HOENN_SPOT_EXCLUDED_SYSTEMS:
                reason = "Hoenn policy: " + rule["system"]
        if reason:
            excluded[reason] += 1
            excluded_names[reason].append(name)
        else:
            by_id[data["id"]] = data

    events = {data["name"]: wayfarer_events(data, all_maps)
              for data in by_id.values()}
    edges = defaultdict(set)
    for data in by_id.values():
        name = data["name"]
        for warp in events[name]["warp_events"]:
            if warp.get("dest_map") in by_id:
                edges[name].add(by_id[warp["dest_map"]]["name"])
        for connection in wayfarer_connections(data):
            if connection.get("map") in by_id:
                edges[name].add(by_id[connection["map"]]["name"])
        if not (data.get("game_version") == "frlg" and name in SEVII_RECORDS):
            for dest in WARP_CMDS.findall(scripts_text(data, all_maps)):
                if dest in by_id:
                    edges[name].add(by_id[dest]["name"])

    seeds = set()
    for heal in load_json(GAME / "src/data/heal_locations.json")["heal_locations"]:
        for key in ("map", "respawn_map"):
            if heal.get(key) in by_id:
                seeds.add(by_id[heal[key]]["name"])
    # Shared scripts (data/scripts/*.inc) warp from maps we cannot tie to a
    # source statically, such as the Celadon hideout entry; their selected
    # destinations seed the search too.
    for path in sorted((GAME / "data/scripts").glob("*.inc")):
        for dest in WARP_CMDS.findall(path.read_text(errors="replace")):
            if dest in by_id:
                seeds.add(by_id[dest]["name"])
    reached, queue = set(seeds), deque(seeds)
    while queue:
        for nxt in edges[queue.popleft()]:
            if nxt not in reached:
                reached.add(nxt)
                queue.append(nxt)
    for data in by_id.values():
        if data["name"] not in reached:
            reason = "selected but unreachable by warps, connections, scripted warps"
            excluded[reason] += 1
            excluded_names[reason].append(data["name"])
    scope = {data["name"]: data for data in by_id.values()
             if data["name"] in reached}
    return scope, events, excluded, excluded_names, sorted(seeds)


# --------------------------------------------------------------------------
# Per-map analysis.
# --------------------------------------------------------------------------

def reachable_land(grid, data, ev, blocked):
    """Tiles reachable from warps and connection edges with walker rules."""
    starts = set()
    for warp in ev["warp_events"]:
        if grid.inside(warp["x"], warp["y"]):
            starts.add((warp["x"], warp["y"]))
    for connection in wayfarer_connections(data):
        direction = connection.get("direction")
        if direction == "up":
            starts |= {(x, 0) for x in range(grid.w)}
        elif direction == "down":
            starts |= {(x, grid.h - 1) for x in range(grid.w)}
        elif direction == "left":
            starts |= {(0, y) for y in range(grid.h)}
        elif direction == "right":
            starts |= {(grid.w - 1, y) for y in range(grid.h)}
    seen = set()
    queue = deque()
    for x, y in starts:
        if (grid.land(x, y) or (x, y) in ev["_warps"]) and (x, y) not in blocked:
            seen.add((x, y))
            queue.append((x, y))
    while queue:
        x, y = queue.popleft()
        for dx, dy in N4:
            nx, ny = x + dx, y + dy
            if not grid.inside(nx, ny):
                continue
            jump = LEDGE.get(grid.b(nx, ny))
            if jump:
                if jump != (dx, dy):
                    continue
                nx, ny = nx + dx, ny + dy
                if not grid.inside(nx, ny):
                    continue
            if (nx, ny) in seen or (nx, ny) in blocked:
                continue
            if grid.land(nx, ny) or (nx, ny) in ev["_warps"]:
                seen.add((nx, ny))
                queue.append((nx, ny))
    return seen


def components(tiles, neighbours=N4):
    tiles, out = set(tiles), []
    while tiles:
        start = tiles.pop()
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


N8 = N4 + ((1, 1), (1, -1), (-1, 1), (-1, -1))


def facing_tiles(grid, reach, free, kinds):
    """Free reachable tiles with a 4-neighbour whose behaviour is in kinds."""
    out = []
    for (x, y) in sorted(reach & free, key=lambda t: (t[1], t[0])):
        for dx, dy in N4:
            nx, ny = x + dx, y + dy
            if grid.inside(nx, ny) and grid.b(nx, ny) in kinds:
                out.append(((x, y), (nx, ny)))
                break
    return out


def analyse(data, ev, all_maps):
    layout = LAYOUTS[data["layout"]]
    grid = Grid(layout)
    name = data["name"]
    objects = ev["object_events"]
    ev["_warps"] = {(w["x"], w["y"]) for w in ev["warp_events"]}
    object_tiles = {(o.get("x"), o.get("y")) for o in objects}
    static_block = {(o.get("x"), o.get("y")) for o in objects
                    if str(o.get("flag", "0")) in ("0", "0x0")}
    reach = reachable_land(grid, data, ev, static_block)
    free = {t for t in reach if t not in object_tiles and t not in ev["_warps"]}
    info = {
        "name": name, "id": data["id"], "region": region_of(data),
        "family": layout_family(layout), "map_type": data["map_type"],
        "music": data["music"], "mapsec": data["region_map_section"],
        "w": grid.w, "h": grid.h,
    }
    gfx = [o.get("graphics_id", "") for o in objects]
    info["nurse"] = any(g in NURSE_GFX for g in gfx)
    info["nurse_like"] = sorted({g for g in gfx if "NURSE" in g or g.endswith(
        "CHANSEY")} - NURSE_GFX)
    info["clerk"] = any(g in CLERK_GFX for g in gfx)
    info["vendor"] = any(str(o.get("script", "")) in VENDORS for o in objects)
    count = Counter(grid.mb)
    info["shelf_tiles"] = sum(count[b] for b in SHELF)
    info["frlg_mart_shelf_tiles"] = (count[MB["MB_POKEMON_CENTER_BOOKSHELF"]]
                                     if grid.family == "frlg" else 0)
    info["counter_tiles"] = sum(count[b] for b in COUNTER)
    info["slot_tiles"] = sum(count[b] for b in SLOT_TILE)
    slot_signs = [b for b in ev["bg_events"]
                  if SLOT_SCRIPT.search(str(b.get("script", "")))]
    info["slot_signs"] = len(slot_signs)

    # Indoor standing tiles.
    nurses = [(o["x"], o["y"]) for o in objects
              if o.get("graphics_id") in NURSE_GFX]
    counters = facing_tiles(grid, reach, free, COUNTER)
    nurse_front = {(nx + 2 * dx, ny + 2 * dy) for nx, ny in nurses
                   for dx, dy in N4}
    info["center_counter_spots"] = len([t for t, c in counters
                                        if t not in nurse_front])
    info["mart_shelf_spots"] = len(facing_tiles(grid, reach, free, SHELF))
    stands = 0
    for sign in slot_signs:
        dx, dy = {"BG_EVENT_PLAYER_FACING_EAST": (1, 0),
                  "BG_EVENT_PLAYER_FACING_WEST": (-1, 0),
                  "BG_EVENT_PLAYER_FACING_NORTH": (0, -1),
                  "BG_EVENT_PLAYER_FACING_SOUTH": (0, 1)}.get(
                      sign.get("player_facing_dir"), (0, 0))
        tile = (sign["x"] - dx, sign["y"] - dy)
        if (dx, dy) != (0, 0) and tile in free:
            stands += 1
    info["slot_spots"] = stands

    # Tall grass.
    grass = {(x, y) for y in range(grid.h) for x in range(grid.w)
             if grid.b(x, y) in TALL_GRASS}
    info["tall_grass_tiles"] = len(grass)
    patches = components(grass)
    info["grass_patches"] = sorted(
        (len(p) for p in patches if len(p) >= PATCH_MIN and set(p) & reach),
        reverse=True)
    info["grass_small_patches"] = sum(1 for p in patches if len(p) < PATCH_MIN)
    info["grass_unreached_patches"] = sum(
        1 for p in patches if len(p) >= PATCH_MIN and not set(p) & reach)
    info["long_grass_tiles"] = sum(count[b] for b in LONG_GRASS)

    # Water's edge.
    edge = []
    for (x, y) in free:
        for dx, dy in N4:
            nx, ny = x + dx, y + dy
            if grid.inside(nx, ny) and grid.b(nx, ny) in FISHABLE:
                edge.append((x, y))
                break
    info["water_tiles"] = sum(count[b] for b in FISHABLE)
    info["water_edge_spots"] = len(edge)
    info["water_edge_stretches"] = len(components(edge, N8))

    # Town squares.
    info["square_tiles"] = 0
    info["squares"] = 0
    if data["map_type"] in SQUARE_TYPES:
        anchors = ev["_warps"] | object_tiles | {
            (b["x"], b["y"]) for b in ev["bg_events"]}
        open_land = {(x, y) for (x, y) in reach
                     if grid.b(x, y) not in TALL_GRASS | LONG_GRASS}
        centres = []
        for (x, y) in open_land:
            if all((x + dx, y + dy) in open_land
                   for dx in (-1, 0, 1) for dy in (-1, 0, 1)) and all(
                       max(abs(x - ax), abs(y - ay)) >= SQUARE_CLEARANCE
                       for ax, ay in anchors):
                centres.append((x, y))
        info["square_tiles"] = len(centres)
        info["squares"] = len(components(centres, N8))

    # NPC chats.
    script = scripts_text(data, all_maps)
    actors = {m.group(2) for m in ACTOR_CMDS.finditer(script)}
    chats, rejects = [], Counter()
    for index, o in enumerate(objects):
        g = o.get("graphics_id", "")
        movement = o.get("movement_type", "")
        reason = None
        if o.get("type", "object") != "object":
            reason = "clone or non-object"
        elif str(o.get("trainer_type", "TRAINER_TYPE_NONE")) not in (
                "TRAINER_TYPE_NONE", "0"):
            reason = "trainer"
        elif str(o.get("flag", "0")) not in ("0", "0x0"):
            reason = "flagged (story state)"
        elif g in NURSE_GFX or g in CLERK_GFX or "NURSE" in g:
            reason = "nurse or clerk"
        elif NOTABLE_GFX.match(g):
            reason = "notable cameo"
        elif VILLAIN_GFX.search(g):
            reason = "villain team (story)"
        elif is_pokemon_gfx(g):
            reason = "pokemon"
        elif PROP_GFX.search(g):
            reason = "item, berry tree, or prop"
        elif not STATIONARY.match(movement):
            if movement == "MOVEMENT_TYPE_NONE":
                reason = "MOVEMENT_TYPE_NONE"
            elif "IN_PLACE" in movement:
                reason = "walk in place"
            else:
                reason = "moves"
        elif o.get("local_id") and str(o["local_id"]) in actors:
            reason = "scripted actor"
        if reason is None:
            spots = [(o["x"] + dx, o["y"] + dy) for dx, dy in N4
                     if (o["x"] + dx, o["y"] + dy) in free]
            if not spots:
                reason = "no free adjacent tile"
            else:
                chats.append({"index": index, "graphics_id": g,
                              "x": o["x"], "y": o["y"],
                              "adjacent": len(spots)})
        if reason:
            rejects[reason] += 1
    info["npc_chats"] = chats
    info["npc_rejects"] = dict(rejects)
    info["warps"] = [{"x": w["x"], "y": w["y"], "dest": w.get("dest_map"),
                      "door": grid.inside(w["x"], w["y"])
                      and grid.b(w["x"], w["y"]) in DOOR,
                      "blocked": grid.inside(w["x"], w["y"])
                      and grid.col[w["y"] * grid.w + w["x"]] != 0}
                     for w in ev["warp_events"]]
    return info


# --------------------------------------------------------------------------
# Indoor kinds via doors.
# --------------------------------------------------------------------------

def classify(info):
    """Content classification of an interior, first match wins (spec)."""
    if info["nurse"]:
        return "center"
    if info["slot_signs"]:
        return "game_corner"
    if info["shelf_tiles"] and info["clerk"]:
        return "mart"
    if info["music"] in GYM_MUSIC:
        return "gym"
    return None


def main():
    all_maps = load_maps()
    scope, events, excluded, excluded_names, seeds = build_scope(all_maps)
    by_id = {d["id"]: d for d in scope.values()}
    infos = {name: analyse(scope[name], events[name], all_maps)
             for name in sorted(scope)}

    # Doors from outdoor maps into interiors.
    door_sites = defaultdict(set)      # interior name -> outdoor sources
    for name, info in infos.items():
        if info["map_type"] not in OUTDOOR_TYPES:
            continue
        for warp in info["warps"]:
            dest = by_id.get(warp["dest"])
            if dest and dest["map_type"] == "MAP_TYPE_INDOOR":
                door_sites[dest["name"]].add(name)

    sites = {}  # interior -> kind (door-entered)
    for interior in door_sites:
        kind = classify(infos[interior])
        if kind:
            sites[interior] = kind

    # Store floors: interiors reachable by interior warps with shelves.
    store_floors = {}
    for interior, kind in sites.items():
        if kind != "mart":
            continue
        seen, queue = {interior}, deque([interior])
        while queue:
            current = queue.popleft()
            for warp in infos[current]["warps"]:
                dest = by_id.get(warp["dest"])
                if (dest and dest["name"] not in seen
                        and dest["map_type"] == "MAP_TYPE_INDOOR"
                        and infos[dest["name"]]["shelf_tiles"]):
                    seen.add(dest["name"])
                    queue.append(dest["name"])
        for floor in seen - {interior}:
            store_floors[floor] = interior

    # Proposed store rule (not the spec's): a door interior is a store when
    # it, or a floor reached from it by interior warps, has shop shelves, and
    # some floor has a vendor (a clerk sprite or an object whose script opens
    # a shop). FRLG Mart tiles count MB_POKEMON_CENTER_BOOKSHELF as a shelf.
    def shelves(n):
        info = infos[n]
        return bool(info["shelf_tiles"] or info["frlg_mart_shelf_tiles"])

    def vendor(n):
        return infos[n]["clerk"] or infos[n]["vendor"]
    proposed_stores = {}
    for interior in door_sites:
        info = infos[interior]
        if info["nurse"] or info["slot_signs"]:
            continue
        seen, queue = {interior}, deque([interior])
        while queue:
            current = queue.popleft()
            for warp in infos[current]["warps"]:
                dest = by_id.get(warp["dest"])
                if (dest and dest["name"] not in seen
                        and dest["map_type"] == "MAP_TYPE_INDOOR"
                        and not infos[dest["name"]]["nurse"]
                        and (shelves(dest["name"]) or vendor(dest["name"]))):
                    seen.add(dest["name"])
                    queue.append(dest["name"])
        floors = sorted(n for n in seen if shelves(n))
        if floors and any(vendor(n) for n in seen):
            proposed_stores[interior] = floors
    # A rooftop door into a top floor finds the same store again: keep one
    # entry per floor set, the first by name (the ground floor).
    by_floors = {}
    for interior in sorted(proposed_stores):
        by_floors.setdefault(tuple(proposed_stores[interior]), interior)
    proposed_stores = {i: list(f) for f, i in by_floors.items()}
    proposed_store_totals = Counter()
    for interior, floors in proposed_stores.items():
        proposed_store_totals[(infos[interior]["region"], "stores")] += 1
        proposed_store_totals[(infos[interior]["region"], "floors")] += len(floors)

    # Content matches that no outdoor door reaches (detection misses).
    misses = defaultdict(list)
    for name, info in infos.items():
        if name in sites or name in store_floors:
            continue
        if info["nurse"]:
            misses["center"].append(name)
        elif info["slot_signs"]:
            misses["game_corner"].append(name)
        elif info["shelf_tiles"] and info["clerk"]:
            misses["mart"].append(name)
    named_gyms_without_music = sorted(
        n for n in door_sites if "Gym" in n and infos[n]["music"] not in GYM_MUSIC
        and sites.get(n) != "gym")
    gym_music_not_named = sorted(n for n, k in sites.items()
                                 if k == "gym" and "Gym" not in n)
    gym_music_not_door = sorted(
        n for n, i in infos.items() if i["music"] in GYM_MUSIC
        and sites.get(n) != "gym" and n not in store_floors)
    corner_music_no_slots = sorted(
        n for n, i in infos.items() if i["music"] in CORNER_MUSIC
        and not i["slot_signs"])
    shelves_no_clerk = sorted(
        n for n, i in infos.items() if i["shelf_tiles"] and not i["clerk"]
        and n not in store_floors and not i["nurse"] and not i["slot_signs"])
    clerk_no_shelves = sorted(
        n for n, i in infos.items() if i["clerk"] and not i["shelf_tiles"])
    nonindoor_dest = set()
    for n, i in infos.items():
        if i["map_type"] not in OUTDOOR_TYPES:
            continue
        for w in i["warps"]:
            dest = by_id.get(w["dest"])
            if dest and dest["map_type"] != "MAP_TYPE_INDOOR":
                kind = classify(infos[dest["name"]])
                if kind:
                    nonindoor_dest.add((dest["name"], dest["map_type"], kind))
    nondoor_entry = sorted({
        (dest_name, src) for dest_name, srcs in door_sites.items()
        if dest_name in sites for src in srcs
        if not any(w["door"] for w in infos[src]["warps"]
                   if by_id.get(w["dest"], {}).get("name") == dest_name)})

    # Per-map rows.
    rows = []
    for name, info in infos.items():
        kind = sites.get(name)
        floor_of = store_floors.get(name)
        row = {
            "region": info["region"], "map": name,
            "map_type": info["map_type"].replace("MAP_TYPE_", ""),
            "family": info["family"],
            "center": 1 if kind == "center" else 0,
            "center_counter_tiles": info["center_counter_spots"] if kind == "center" else 0,
            "mart_floor": 1 if kind == "mart" or floor_of else 0,
            "mart_shelf_tiles": info["mart_shelf_spots"] if kind == "mart" or floor_of else 0,
            "store_of": floor_of or (name if kind == "mart" else ""),
            "game_corner": 1 if kind == "game_corner" else 0,
            "slot_tiles": info["slot_spots"] if kind == "game_corner" else 0,
            "gym": 1 if kind == "gym" else 0,
            "tall_grass_tiles": info["tall_grass_tiles"],
            "grass_patches": len(info["grass_patches"]),
            "grass_patch_sizes": " ".join(map(str, info["grass_patches"])),
            "water_edge_tiles": info["water_edge_spots"],
            "water_edge_stretches": info["water_edge_stretches"],
            "square_tiles": info["square_tiles"],
            "squares": info["squares"],
            "npc_chats": len(info["npc_chats"]),
            "npc_chat_tiles": sum(c["adjacent"] for c in info["npc_chats"]),
        }
        rows.append(row)
    rows.sort(key=lambda r: (REGIONS.index(r["region"]), r["map"]))
    fields = list(rows[0].keys())
    with (HERE / "per-map.csv").open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)

    # Region totals.
    totals = {r: Counter() for r in REGIONS}
    maps_per_region = Counter()
    for row in rows:
        t = totals[row["region"]]
        maps_per_region[row["region"]] += 1
        for key in ("center", "center_counter_tiles", "mart_floor",
                    "mart_shelf_tiles", "game_corner", "slot_tiles", "gym",
                    "tall_grass_tiles", "grass_patches", "water_edge_tiles",
                    "water_edge_stretches", "square_tiles", "squares",
                    "npc_chats", "npc_chat_tiles"):
            t[key] += row[key]
        if row["store_of"] == row["map"]:
            t["stores"] += 1
        if row["square_tiles"]:
            t["square_maps"] += 1
        if row["map_type"] in ("TOWN", "CITY"):
            t["town_city_maps"] += 1
        if row["tall_grass_tiles"]:
            t["grass_maps"] += 1
        if row["water_edge_tiles"]:
            t["water_maps"] += 1
        if row["npc_chats"]:
            t["npc_maps"] += 1

    rejects = Counter()
    for info in infos.values():
        rejects.update(info["npc_rejects"])

    family_behaviour = defaultdict(Counter)
    for info in infos.values():
        family_behaviour[info["family"]]["maps"] += 1
        if info["shelf_tiles"]:
            family_behaviour[info["family"]]["maps_with_shelves"] += 1
        if info["counter_tiles"]:
            family_behaviour[info["family"]]["maps_with_counters"] += 1
        if info["slot_tiles"]:
            family_behaviour[info["family"]]["maps_with_slot_tiles"] += 1
        if info["tall_grass_tiles"]:
            family_behaviour[info["family"]]["maps_with_tall_grass"] += 1
        if info["long_grass_tiles"]:
            family_behaviour[info["family"]]["maps_with_long_grass"] += 1
        if info["water_tiles"]:
            family_behaviour[info["family"]]["maps_with_fishable_water"] += 1

    door_stats = defaultdict(Counter)
    for n, i in infos.items():
        if i["map_type"] not in OUTDOOR_TYPES:
            continue
        for w in i["warps"]:
            dest = by_id.get(w["dest"])
            if dest and dest["map_type"] == "MAP_TYPE_INDOOR":
                door_stats[i["family"]]["outdoor_to_indoor_warps"] += 1
                door_stats[i["family"]]["door_behaviour"] += w["door"]
                door_stats[i["family"]]["collision_bit_set"] += w["blocked"]
    wild = load_json(GAME / "src/data/wild_encounters.json")["wild_encounter_groups"][0]["encounters"]
    sevii_wild = load_json(GAME / "src/data/wayfarer_sevii_wild_encounters.json")["profiles"]
    land = {e["map"] for e in wild if "land_mons" in e} | {
        p["map"] for p in sevii_wild if p["method"] == "land_mons"}
    fishing = {e["map"] for e in wild if "fishing_mons" in e} | {
        p["map"] for p in sevii_wild if p["method"] == "fishing_mons"}
    grass_no_land = sorted(n for n, i in infos.items()
                           if i["grass_patches"] and i["id"] not in land)
    edge_no_fishing = sorted(n for n, i in infos.items()
                             if i["water_edge_spots"] and i["id"] not in fishing)
    slot_tile_maps = sorted((n, i["slot_tiles"]) for n, i in infos.items()
                            if i["slot_tiles"])
    centers_zero_counter = sorted(n for n, k in sites.items()
                                  if k == "center"
                                  and not infos[n]["center_counter_spots"])
    marts_zero_shelf = sorted(n for n, k in sites.items()
                              if k == "mart" and not infos[n]["mart_shelf_spots"])
    corners = {n: (infos[n]["slot_signs"], infos[n]["slot_spots"])
               for n, k in sites.items() if k == "game_corner"}
    towns_no_square = sorted(n for n, i in infos.items()
                             if i["map_type"] in SQUARE_TYPES and not i["square_tiles"])
    town_types = sorted((n, i["map_type"]) for n, i in infos.items()
                        if i["map_type"] in SQUARE_TYPES)
    unreached_grass = sorted((n, i["grass_unreached_patches"])
                             for n, i in infos.items() if i["grass_unreached_patches"])
    nurse_like = sorted((n, i["nurse_like"]) for n, i in infos.items()
                        if i["nurse_like"] and not i["nurse"])
    top = lambda key: sorted(((r[key], r["map"]) for r in rows), reverse=True)[:8]

    findings = {
        "content_matches_without_outdoor_door": {k: sorted(v) for k, v in misses.items()},
        "door_into_non_indoor_map_with_kind_content": sorted(nonindoor_dest),
        "gym_named_door_interiors_without_gym_music": named_gyms_without_music,
        "gym_music_interiors_not_named_gym": gym_music_not_named,
        "gym_music_maps_not_door_entered_gym": gym_music_not_door,
        "game_corner_music_without_slot_signs": corner_music_no_slots,
        "game_corners_signs_and_standing_tiles": corners,
        "shelf_tiles_without_clerk": shelves_no_clerk,
        "clerk_without_shelf_tiles": clerk_no_shelves,
        "centers_without_counter_spots": centers_zero_counter,
        "marts_without_shelf_spots": marts_zero_shelf,
        "door_kind_entered_without_door_behaviour": nondoor_entry,
        "town_city_maps_without_square": towns_no_square,
        "proposed_stores": dict(sorted(proposed_stores.items())),
        "proposed_store_totals": {r: {"stores": proposed_store_totals[(r, "stores")],
                                      "floors": proposed_store_totals[(r, "floors")]}
                                  for r in REGIONS},
        "spec_stores_missing_under_proposed": sorted(
            n for n, k in sites.items() if k == "mart" and n not in proposed_stores),
        "town_city_maps": town_types,
        "unreached_grass_patches": unreached_grass,
        "nurse_like_gfx_without_nurse": nurse_like,
        "npc_reject_reasons": dict(rejects.most_common()),
        "door_warp_tiles_by_family": {k: dict(v) for k, v in door_stats.items()},
        "slot_machine_tile_maps": slot_tile_maps,
        "grass_patch_maps_without_land_encounters": grass_no_land,
        "water_edge_maps_without_fishing_encounters": edge_no_fishing,
        "indoor_water_edge_maps": sorted(
            (n, i["water_edge_spots"]) for n, i in infos.items()
            if i["water_edge_spots"] and i["map_type"] == "MAP_TYPE_INDOOR"),
        "indoor_grass_maps": sorted(
            (n, i["tall_grass_tiles"]) for n, i in infos.items()
            if i["grass_patches"] and i["map_type"] == "MAP_TYPE_INDOOR"),
        "family_behaviour_presence": {k: dict(v) for k, v in family_behaviour.items()},
        "top_grass_tiles": top("tall_grass_tiles"),
        "top_water_edge_tiles": top("water_edge_tiles"),
        "top_npc_chats": top("npc_chats"),
        "top_square_tiles": top("square_tiles"),
    }
    out = {
        "scope": {
            "rule": __doc__.strip().splitlines()[0],
            "in_scope_maps": len(infos),
            "maps_per_region": dict(maps_per_region),
            "seeds": seeds,
            "excluded": dict(excluded.most_common()),
            "excluded_examples": {k: sorted(v)[:12] for k, v in excluded_names.items()},
            "unreachable_selected": sorted(excluded_names.get(
                "selected but unreachable by warps, connections, scripted warps", [])),
        },
        "region_totals": {r: dict(totals[r]) for r in REGIONS},
        "sites": {k: sorted(n for n, kk in sites.items() if kk == k)
                  for k in ("center", "mart", "game_corner", "gym")},
        "store_floors": dict(sorted(store_floors.items())),
        "findings": findings,
        # Per-map counts live in per-map.csv; keep only the lists here.
        "maps": {n: {"npc_chats": [[c["x"], c["y"], c["graphics_id"]]
                                   for c in i["npc_chats"]],
                     "npc_rejects": i["npc_rejects"]}
                 for n, i in infos.items()
                 if i["npc_chats"] or i["npc_rejects"]},
    }
    (HERE / "inventory.json").write_text(
        json.dumps(out, indent=1, default=list) + "\n")

    # Markdown summary on stdout.
    print("In-scope maps:", len(infos), dict(maps_per_region))
    print("Excluded:", dict(excluded.most_common()))
    header = "| Kind | " + " | ".join(REGIONS) + " | Total |"
    print(header)
    print("| --- |" + " ---: |" * (len(REGIONS) + 1))
    lines = [
        ("Maps scanned", lambda r: maps_per_region[r]),
        ("Pokémon Centers", lambda r: totals[r]["center"]),
        ("  counter tiles", lambda r: totals[r]["center_counter_tiles"]),
        ("Marts and stores", lambda r: totals[r]["stores"]),
        ("  floors", lambda r: totals[r]["mart_floor"]),
        ("  shelf tiles", lambda r: totals[r]["mart_shelf_tiles"]),
        ("Game Corners", lambda r: totals[r]["game_corner"]),
        ("  slot tiles", lambda r: totals[r]["slot_tiles"]),
        ("Gyms", lambda r: totals[r]["gym"]),
        ("Tall-grass maps", lambda r: totals[r]["grass_maps"]),
        ("  patches (>= %d)" % PATCH_MIN, lambda r: totals[r]["grass_patches"]),
        ("  tiles", lambda r: totals[r]["tall_grass_tiles"]),
        ("Water's-edge maps", lambda r: totals[r]["water_maps"]),
        ("  stretches", lambda r: totals[r]["water_edge_stretches"]),
        ("  tiles", lambda r: totals[r]["water_edge_tiles"]),
        ("Town and city maps", lambda r: totals[r]["town_city_maps"]),
        ("  with a square", lambda r: totals[r]["square_maps"]),
        ("  squares", lambda r: totals[r]["squares"]),
        ("  square tiles", lambda r: totals[r]["square_tiles"]),
        ("NPC-chat maps", lambda r: totals[r]["npc_maps"]),
        ("  NPCs", lambda r: totals[r]["npc_chats"]),
        ("  adjacent tiles", lambda r: totals[r]["npc_chat_tiles"]),
    ]
    for label, fn in lines:
        values = [fn(r) for r in REGIONS]
        print(f"| {label} | " + " | ".join(map(str, values)) + f" | {sum(values)} |")
    print()
    print(json.dumps(findings, indent=1, default=list))


if __name__ == "__main__":
    sys.exit(main())
