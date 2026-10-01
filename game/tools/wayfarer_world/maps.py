"""Map model for the walker graph: scope, events, tile attributes, objects.

Every map is read as the Wayfarer ROM loads it. The compiled map list and
its constants come from mapjson's group outputs (data/maps/groups.inc and
include/constants/map_groups.h, written by `mapjson groups wayfarer`), and
each map's events, connections and header from the per-map files mapjson
writes with the Wayfarer detail version (events.inc, connections.inc,
header.inc), so `wayfarer_exclude`, the `wayfarer_*` field overrides, shared
event maps and the Sevii manifest are already applied. map.json is read only
for what mapjson does not emit: `game_version` and `region` (region tagging)
and nothing else.

Tile attributes follow game/src/fieldmap.c: ExtractMetatileAttribute (FRLG
32-bit attributes with a 9-bit behaviour then NormalizeFrlgMetatileBehavior;
Emerald and HNS 16-bit attributes with an 8-bit behaviour) and
GetNumMetatilesInPrimary (640 primary metatiles for the FRLG and HNS layout
versions, 512 for Emerald). Map grid entries follow global.fieldmap.h:
metatile id bits 0-9, collision bits 10-11, elevation bits 12-15.
"""

import fnmatch
import json
import re
import struct
from array import array
from collections import defaultdict, deque
from pathlib import Path

TOOL_DIR = Path(__file__).resolve().parent
DEFAULT_ROOT = TOOL_DIR.parents[1]

REGIONS = ("Kanto", "Johto", "Hoenn", "Sevii")
OUTDOOR_TYPES = {"MAP_TYPE_TOWN", "MAP_TYPE_CITY", "MAP_TYPE_ROUTE",
                 "MAP_TYPE_OCEAN_ROUTE"}
TOWN_TYPES = {"MAP_TYPE_TOWN", "MAP_TYPE_CITY"}
INTERIOR_TYPES = {"MAP_TYPE_INDOOR", "MAP_TYPE_NONE"}
GYM_MUSIC = {"MUS_GYM", "MUS_HG_GYM", "MUS_RG_GYM"}

# Hoenn systems kept in the ROM but left out of the spot pool (inventory
# scope step 4): per-player secret bases and ticket-only event islands.
HOENN_SPOT_EXCLUDED_SYSTEMS = {"secret-bases", "event-islands"}

WARP_CMDS = re.compile(r"^\s*warp\w*\s+(MAP_\w+)", re.M)

DIR_VEC = {"DIR_SOUTH": (0, 1), "DIR_NORTH": (0, -1),
           "DIR_WEST": (-1, 0), "DIR_EAST": (1, 0)}
DIR_VALUE = {"DIR_NONE": 0, "DIR_SOUTH": 1, "DIR_NORTH": 2,
             "DIR_WEST": 3, "DIR_EAST": 4}


def load_json(path):
    return json.loads(Path(path).read_text(encoding="utf-8"))


def read_text(path):
    return Path(path).read_text(encoding="utf-8", errors="replace")


class BuildError(Exception):
    """A validation failure: the generator reports it and exits non-zero."""


# --------------------------------------------------------------------------
# Constants read from the engine headers.
# --------------------------------------------------------------------------

class Constants:
    def __init__(self, root):
        self.root = root
        self.mb = self._behaviour_enum()
        self.mb_name = {v: k for k, v in self.mb.items()}
        self.frlg_normalise = self._frlg_normaliser()
        self.surfable = self._surfable()
        self._check_dirs()

    def _behaviour_enum(self):
        text = read_text(self.root / "include/constants/metatile_behaviors.h")
        body = text[text.index("enum {") + 6:text.index("};")]
        names = [re.sub(r"//.*", "", line).strip().rstrip(",")
                 for line in body.splitlines()]
        names = [n for n in names if n]
        if any("=" in n for n in names):
            raise BuildError("metatile_behaviors.h enum has explicit values; "
                             "update the reader")
        return {name: index for index, name in enumerate(names)}

    def _frlg_normaliser(self):
        frlg = {m.group(1): int(m.group(2), 16) for m in re.finditer(
            r"#define (MB_FRLG_\w+)\s+(0x[0-9A-Fa-f]+)",
            read_text(self.root / "include/constants/metatile_behaviors_frlg.h"))}
        text = read_text(self.root / "src/fieldmap.c")
        body = text[text.index("NormalizeFrlgMetatileBehavior(u32"):]
        body = body[:body.index("default:")]
        table = {}
        for src, dst in re.findall(r"case (MB_FRLG_\w+):\s*return (MB_\w+);", body):
            table[frlg[src]] = self.mb[dst]
        return table

    def _surfable(self):
        # MetatileBehavior_IsSurfableWaterOrUnderwater: TILE_FLAG_SURFABLE in
        # sTileBitAttributes (src/metatile_behavior.c).
        text = read_text(self.root / "src/metatile_behavior.c")
        body = text[text.index("sTileBitAttributes"):]
        body = body[:body.index("};")]
        return {self.mb[m.group(1)] for m in re.finditer(
            r"\[(MB_\w+)\]\s*=\s*[^,\n]*TILE_FLAG_SURFABLE", body)}

    def _check_dirs(self):
        text = read_text(self.root / "include/constants/global.h")
        match = re.search(r"DIR_NONE,\s*DIR_SOUTH,\s*DIR_NORTH,\s*DIR_WEST,\s*DIR_EAST",
                          text)
        if not match:
            raise BuildError("DIR_* order in constants/global.h changed")

    def set_of(self, *names):
        return {self.mb[n] for n in names if n in self.mb}


# --------------------------------------------------------------------------
# Tilesets, layouts and grids.
# --------------------------------------------------------------------------

class Tilesets:
    def __init__(self, root):
        self.root = root
        self.headers = read_text(root / "src/data/tilesets/headers.h")
        self.metatiles = read_text(root / "src/data/tilesets/metatiles.h")
        self._cache = {}
        self.paths = set()

    def attribute_path(self, tileset):
        match = re.search(
            rf"const struct Tileset {tileset} =\s*\{{.*?metatileAttributes = (\w+)",
            self.headers, re.S)
        if not match:
            return None
        path = re.search(rf'{match.group(1)}\[\] = INCBIN_U\d+\("([^"]+)"\)',
                         self.metatiles)
        return path.group(1) if path else None

    def attributes(self, tileset, wide):
        key = (tileset, wide)
        if key not in self._cache:
            rel = self.attribute_path(tileset)
            values = array("I" if wide else "H")
            if rel:
                self.paths.add(rel)
                data = (self.root / rel).read_bytes()
                usable = len(data) - len(data) % values.itemsize
                values.frombytes(data[:usable])
                if values.itemsize > 1 and struct.pack("=H", 1) != b"\x01\x00":
                    values.byteswap()
            self._cache[key] = values
        return self._cache[key]


def layout_family(layout):
    # mapjson defaults a missing layout_version to emerald.
    return layout.get("layout_version") or "emerald"


class Grid:
    """Behaviour, collision and elevation per tile, read as the engine does."""

    def __init__(self, root, layout, tilesets, consts):
        self.family = layout_family(layout)
        self.w, self.h = layout["width"], layout["height"]
        blocks = array("H")
        raw = (root / layout["blockdata_filepath"]).read_bytes()
        blocks.frombytes(raw[:2 * self.w * self.h])
        if struct.pack("=H", 1) != b"\x01\x00":
            blocks.byteswap()
        frlg = self.family == "frlg"
        n_primary = 512 if self.family == "emerald" else 640
        primary = tilesets.attributes(layout["primary_tileset"], frlg)
        secondary = tilesets.attributes(layout["secondary_tileset"], frlg)
        normalise = consts.frlg_normalise
        cache = {}
        mb = []
        for block in blocks:
            metatile = block & 0x3FF
            behaviour = cache.get(metatile)
            if behaviour is None:
                if metatile < n_primary:
                    data, index = primary, metatile
                else:
                    data, index = secondary, metatile - n_primary
                value = data[index] if index < len(data) else 0
                if frlg:
                    behaviour = value & 0x1FF
                    behaviour = normalise.get(behaviour, behaviour)
                else:
                    behaviour = value & 0xFF
                cache[metatile] = behaviour
            mb.append(behaviour)
        self.mb = mb
        self.col = [(b >> 10) & 3 for b in blocks]
        self.elev = [b >> 12 for b in blocks]

    def inside(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h

    def b(self, x, y):
        return self.mb[y * self.w + x]


# --------------------------------------------------------------------------
# mapjson outputs.
# --------------------------------------------------------------------------

def _args(line, macro):
    return [a.strip() for a in line.split(macro, 1)[1].split(",")]


def parse_events(text):
    events = {"objects": [], "warps": [], "coords": [], "bgs": []}
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("object_event "):
            a = _args(s, "object_event")
            events["objects"].append({
                "local_id": int(a[0], 0), "gfx": a[1], "x": int(a[2], 0),
                "y": int(a[3], 0), "elevation": int(a[4], 0),
                "movement": a[5], "range_x": int(a[6], 0),
                "range_y": int(a[7], 0), "trainer_type": a[8],
                "sight": a[9], "script": a[10], "flag": a[11],
                "kind": "object"})
        elif s.startswith("clone_event "):
            a = _args(s, "clone_event")
            events["objects"].append({
                "local_id": int(a[0], 0), "gfx": a[1], "x": int(a[2], 0),
                "y": int(a[3], 0), "elevation": 0, "movement": "",
                "trainer_type": "TRAINER_TYPE_NONE", "script": "",
                "flag": "0", "kind": "clone"})
        elif s.startswith("warp_def "):
            a = _args(s, "warp_def")
            events["warps"].append({
                "x": int(a[0], 0), "y": int(a[1], 0), "elevation": int(a[2], 0),
                "dest_warp": a[3], "dest_map": a[4]})
        elif s.startswith("coord_event ") or s.startswith("coord_weather_event "):
            macro = s.split()[0]
            a = _args(s, macro)
            events["coords"].append({"x": int(a[0], 0), "y": int(a[1], 0)})
        elif s.startswith("bg_sign_event "):
            a = _args(s, "bg_sign_event")
            events["bgs"].append({"x": int(a[0], 0), "y": int(a[1], 0),
                                  "kind": "sign", "facing": a[3],
                                  "script": a[4]})
        elif s.startswith("bg_") and "_event " in s:
            macro = s.split()[0]
            a = _args(s, macro)
            events["bgs"].append({"x": int(a[0], 0), "y": int(a[1], 0),
                                  "kind": macro, "facing": "", "script": ""})
    return events


def parse_connections(text):
    out = []
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("connection "):
            a = _args(s, "connection")
            out.append({"direction": a[0], "offset": int(a[1], 0), "map": a[2]})
    return out


def parse_header(text):
    words = [l.strip().split(None, 1) for l in text.splitlines()
             if l.strip().startswith((".4byte", ".2byte", ".byte"))]
    values = [w[1] for w in words if len(w) == 2]
    # .4byte layout, events, scripts, connections; .2byte music, layout id;
    # .byte mapsec, cave, weather, map type (mapjson.cpp, header writer).
    return {"events_label": values[1], "scripts_label": values[2],
            "music": values[4], "layout": values[5], "mapsec": values[6],
            "map_type": values[9]}


def parse_groups(root):
    """Compiled map order: (group name, [map names]) from groups.inc."""
    groups, current = [], None
    for line in read_text(root / "data/maps/groups.inc").splitlines():
        label = re.match(r"^(gMapGroup_\w+)::", line)
        if label:
            current = (label.group(1), [])
            groups.append(current)
            continue
        if line.startswith("gMapGroups::"):
            current = None
            continue
        item = re.match(r"^\s*\.4byte (\w+)", line)
        if item and current is not None:
            current[1].append(item.group(1))
    return groups


def parse_map_constants(root):
    """MAP_* constants in enum order with their values (map_groups.h)."""
    out = []
    for m in re.finditer(r"^\s*(MAP_\w+)\s*=\s*\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)",
                         read_text(root / "include/constants/map_groups.h"), re.M):
        out.append((m.group(1), int(m.group(2)) | (int(m.group(3)) << 8)))
    return out


class MapInfo:
    __slots__ = ("name", "const", "value", "order", "group", "header",
                 "events", "connections", "json", "layout", "region",
                 "map_type", "music", "scripts_owner", "grid", "family",
                 "in_scope")

    def __repr__(self):
        return "<Map %s>" % self.name


# --------------------------------------------------------------------------
# New-game flags: which flagged objects are present when a game starts.
# --------------------------------------------------------------------------

def _preprocess(text, defines):
    """Keep lines whose #if/#elif/#else blocks are active for `defines`."""
    out, stack = [], []
    for line in text.splitlines():
        s = line.strip()
        directive = re.match(r"^#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)", s)
        if not directive:
            if all(active for active, _ in stack):
                out.append(line)
            continue
        kind, rest = directive.group(1), directive.group(2).strip()

        def value(expr):
            expr = expr.strip()
            negate = expr.startswith("!")
            name = expr.lstrip("!").strip()
            result = bool(defines.get(name, 0))
            return not result if negate else result
        if kind == "if":
            v = value(rest)
            stack.append((v, v))
        elif kind == "ifdef":
            v = rest in defines
            stack.append((v, v))
        elif kind == "ifndef":
            v = rest not in defines
            stack.append((v, v))
        elif kind == "elif":
            _, taken = stack.pop()
            v = (not taken) and value(rest)
            stack.append((v, taken or v))
        elif kind == "else":
            _, taken = stack.pop()
            stack.append((not taken, True))
        elif kind == "endif":
            stack.pop()
    return "\n".join(out)


WAYFARER_DEFINES = {"IS_WAYFARER": 1, "IS_HNS": 1, "IS_FRLG": 0,
                    "POKEMON_WAYFARER": 1}


def _script_flags(text, label):
    on, _ = _script_effects(text, label)
    return on


def _script_effects(text, label):
    """(set, cleared) flags of one script label, in order, to `end`."""
    start = text.find("\n" + label + "::")
    if start < 0:
        raise BuildError("script label %s not found" % label)
    flags = {}
    for line in text[start:].splitlines()[1:]:
        s = line.split("@", 1)[0].strip()
        if re.match(r"^\w+::", s) and not flags:
            continue  # stacked labels
        cmd = s.split()
        if not cmd:
            continue
        if cmd[0] in ("end", "return"):
            break
        if cmd[0] == "setflag":
            flags[cmd[1]] = True
        elif cmd[0] == "clearflag":
            flags[cmd[1]] = False
    return ({f for f, on in flags.items() if on},
            {f for f, on in flags.items() if not on})


def _c_function_flags(text, function):
    match = re.search(r"\nvoid %s\(void\)\s*\{" % function, text)
    if not match:
        raise BuildError("function %s not found" % function)
    depth, i = 1, match.end()
    while depth and i < len(text):
        depth += {"{": 1, "}": -1}.get(text[i], 0)
        i += 1
    return text[match.end():i]


def new_game_flags(root):
    """Flags set when a Wayfarer game starts, per map family.

    NewGameInitData (src/new_game.c) runs EventScript_ResetAllMapFlagsHnS
    (Wayfarer builds with IS_HNS) and WayfarerInitPersistentState, which sets
    a few flags itself and through WayfarerSeviiInitPersistentState (both in
    src/wayfarer_persistence.c). Hoenn's objects read the Hoenn flag
    namespace, whose baseline is WayfarerHoennEntry_EventScript_InitializeBaseline
    (data/scripts/wayfarer_hoenn_entry.inc), run on the first entry into
    Hoenn, plus the HOENN_FLAG_ID FlagSet in WayfarerInitPersistentState.
    HNS also initialises story flags from first-visit triggers marked
    `@flagheap` in map scripts (a coord event on the way into Azalea,
    Goldenrod and Ecruteak, and the S.S. Aqua's first boarding); their
    setflag and clearflag effects are applied on top of the new-game script,
    since a player always crosses them before seeing the map's objects.
    Origin-specific initialisers are not read: a flag only some origins set
    leaves its object present, which is the conservative reading.
    """
    script = _preprocess(read_text(root / "data/scripts/new_game.inc"),
                         WAYFARER_DEFINES)
    hns = _script_flags(script, "EventScript_ResetAllMapFlagsHnS")
    for path in sorted((root / "data/maps").glob("*/scripts.inc")):
        text = read_text(path)
        if "@flagheap" not in text:
            continue
        text = _preprocess(text, WAYFARER_DEFINES)
        lines = text.splitlines()
        for n, line in enumerate(lines):
            if "@flagheap" not in line:
                continue
            label = re.match(r"^(\w+)::", line)
            if not label and n and re.match(r"^\w+::\s*$", lines[n - 1]) \
                    and not line.split("@")[0].strip():
                label = re.match(r"^(\w+)::", lines[n - 1])
            if label:
                on, off = _script_effects(text, label.group(1))
                hns = (hns | on) - off
            else:
                cmd = line.split("@", 1)[0].split()
                if len(cmd) == 2 and cmd[0] == "setflag":
                    hns.add(cmd[1])
    persistence = _preprocess(read_text(root / "src/wayfarer_persistence.c"),
                              WAYFARER_DEFINES)
    hoenn_numeric = []
    for function in ("WayfarerInitPersistentState",
                     "WayfarerSeviiInitPersistentState"):
        body = _c_function_flags(persistence, function)
        hns |= set(re.findall(r"FlagSet\((FLAG_\w+)\)", body))
        hoenn_numeric += re.findall(r"FlagSet\(HOENN_FLAG_ID\((\w+)\)\)", body)
    entry = _preprocess(read_text(root / "data/scripts/wayfarer_hoenn_entry.inc"),
                        WAYFARER_DEFINES)
    hoenn = _script_flags(entry, "WayfarerHoennEntry_EventScript_InitializeBaseline")
    if hoenn_numeric:
        defines = dict(re.findall(r"#define (\w+)\s+(0x[0-9A-Fa-f]+|\d+)",
                                  read_text(root / "include/constants/wayfarer_persistence.h")))
        emerald = defaultdict(set)
        for name, value in re.findall(r"#define (FLAG_\w+)\s+(0x[0-9A-Fa-f]+|\d+)\b",
                                      read_text(root / "include/constants/flags.h")):
            emerald[int(value, 0)].add(name)
        for name in hoenn_numeric:
            if name in defines:
                hoenn |= emerald.get(int(defines[name], 0), set())
    return {"hns": hns, "hoenn": hoenn}


def flag_aliases(root):
    """FLAG_A -> FLAG_B aliases (#define FLAG_A FLAG_B) in the flag headers."""
    alias = {}
    for path in sorted((root / "include/constants").glob("flags*.h")):
        for a, b in re.findall(r"#define (FLAG_\w+)\s+(FLAG_\w+)\s*$",
                               read_text(path), re.M):
            alias[a] = b

    def canonical(name):
        seen = set()
        while name in alias and name not in seen:
            seen.add(name)
            name = alias[name]
        return name
    return canonical


# --------------------------------------------------------------------------
# The world of maps.
# --------------------------------------------------------------------------

class World:
    """All compiled maps, the scope, and per-map grids (built on demand)."""

    def __init__(self, root=DEFAULT_ROOT, only=None):
        self.root = Path(root)
        self.consts = Constants(self.root)
        self.tilesets = Tilesets(self.root)
        self.layouts = {l["id"]: l for l in load_json(
            self.root / "data/layouts/layouts.json")["layouts"] if l and "id" in l}
        self.mapsec_index = {s["id"]: i for i, s in enumerate(load_json(
            self.root / "src/data/region_map/region_map_sections.json")["map_sections"])}
        self.groups = parse_groups(self.root)
        self.group_of = {}
        map_groups = load_json(self.root / "data/maps/map_groups.json")
        for group in map_groups["group_order"]:
            for name in map_groups.get(group, []):
                self.group_of[name] = group
        consts = parse_map_constants(self.root)
        names = [n for _, maps in self.groups for n in maps]
        # groups.inc keeps a NULL slot for each deselected map, so its
        # entries pair one to one with the enum in map_groups.h.
        if len(names) != len(consts):
            raise BuildError("groups.inc lists %d maps but map_groups.h %d"
                             % (len(names), len(consts)))
        self.maps, self.by_const = {}, {}
        self.order = []
        for name, (const, value) in zip(names, consts):
            if name == "NULL":
                continue  # a deselected map keeps its constant, no header
            order = len(self.order)
            info = MapInfo()
            info.name, info.const, info.value, info.order = name, const, value, order
            info.grid = None
            info.in_scope = False
            self.maps[name] = info
            self.by_const[const] = info
            self.order.append(info)
        self.only = set(only) if only else None
        for info in self.order:
            if self.only is not None and info.name not in self.only:
                continue
            self._load_map(info)
        self.excluded = defaultdict(list)
        self.scope = []

    # Per-map files ----------------------------------------------------------

    def map_dir(self, name):
        return self.root / "data/maps" / name

    def _load_map(self, info):
        d = self.map_dir(info.name)
        info.json = load_json(d / "map.json")
        if info.json.get("id") != info.const:
            raise BuildError("%s: map.json id %s does not match map_groups.h %s"
                             % (info.name, info.json.get("id"), info.const))
        info.header = parse_header(read_text(d / "header.inc"))
        owner = info.header["events_label"].rsplit("_MapEvents", 1)[0]
        info.events = parse_events(read_text(self.map_dir(owner) / "events.inc"))
        info.connections = parse_connections(read_text(d / "connections.inc"))
        info.layout = self.layouts[info.header["layout"]]
        info.family = layout_family(info.layout)
        info.map_type = info.header["map_type"]
        info.music = info.header["music"]
        info.scripts_owner = info.header["scripts_label"].rsplit("_MapScripts", 1)[0]
        info.region = self.region_of(info)

    def loaded(self):
        return [m for m in self.order if getattr(m, "header", None) is not None
                and hasattr(m, "json") and m.json is not None]

    def grid(self, info):
        if info.grid is None:
            info.grid = Grid(self.root, info.layout, self.tilesets, self.consts)
        return info.grid

    def scripts_text(self, info):
        path = self.map_dir(info.scripts_owner) / "scripts.inc"
        return read_text(path) if path.exists() else ""

    # Region tagging (inventory "Region tagging") ------------------------------

    def region_of(self, info):
        data = info.json
        version = data.get("game_version", "emerald")
        if version == "emerald":
            return "Hoenn"
        if version == "frlg":
            index = self.mapsec_index.get(info.header["mapsec"], -1)
            low = self.mapsec_index["MAPSEC_ONE_ISLAND"]
            high = self.mapsec_index["MAPSEC_EMBER_SPA"]
            return "Sevii" if low <= index <= high else "Kanto"
        if version == "hns":
            region = data.get("region")
            if region == "REGION_KANTO":
                return "Kanto"
            if region == "REGION_JOHTO":
                return "Johto"
            if region:
                return None
            group = self.group_of.get(info.name, "")
            if re.search(r"Indoor(NewBark|Cherrygrove|Violet|Azalea|Goldenrod|"
                         r"Ecruteak|Olivine|Cianwood|Mahogany|Blackthorn|"
                         r"JohtoRoutes)_Hns", group):
                return "Johto"
            if re.search(r"Indoor(Pallet|Viridian|Pewter|Cerulean|Vermilion|"
                         r"Lavender|Celadon|Saffron|Fuchsia|Cinnabar|Indigo|"
                         r"KantoRoutes)_Hns", group):
                return "Kanto"
            return None
        return None  # Sinnoh

    # Scope (inventory "Scope", steps 3-5; mapjson already did 1-2) -----------

    def build_scope(self, seed_all=False):
        policy = load_json(self.root / "tools/wayfarer_hoenn_content/classification.json")

        def hoenn_rule(name):
            for rule in policy["rules"]:
                if any(fnmatch.fnmatchcase(name, g) for g in rule["globs"]):
                    return rule
            return policy["default"]
        sevii = load_json(self.root / "src/data/wayfarer_sevii_maps.json")
        sevii_maps = {m["source_map"] for m in sevii["maps"]}
        candidates = {}
        for info in self.loaded():
            reason = None
            version = info.json.get("game_version", "emerald")
            if info.name.startswith("TestMap"):
                reason = "debug test map"
            elif version == "sinnoh":
                reason = "Sinnoh (outside the four spot regions)"
            elif info.region is None:
                reason = "HNS map outside Kanto and Johto"
            elif version == "emerald":
                rule = hoenn_rule(info.name)
                if rule["classification"] == "excluded":
                    reason = "Hoenn policy: excluded"
                elif rule.get("system") in HOENN_SPOT_EXCLUDED_SYSTEMS:
                    reason = "Hoenn policy: " + rule["system"]
            if reason:
                self.excluded[reason].append(info.name)
            else:
                candidates[info.const] = info
        links = defaultdict(set)
        for info in candidates.values():
            for warp in info.events["warps"]:
                if warp["dest_map"] in candidates:
                    links[info.name].add(candidates[warp["dest_map"]].name)
            for conn in info.connections:
                if conn["map"] in candidates:
                    links[info.name].add(candidates[conn["map"]].name)
            if not (info.json.get("game_version") == "frlg" and info.name in sevii_maps):
                for dest in WARP_CMDS.findall(self.scripts_text(info)):
                    if dest in candidates:
                        links[info.name].add(candidates[dest].name)
        if seed_all:
            seeds = {i.name for i in candidates.values()}
        else:
            seeds = set()
            for heal in load_json(self.root / "src/data/heal_locations.json")["heal_locations"]:
                for key in ("map", "respawn_map"):
                    if heal.get(key) in candidates:
                        seeds.add(candidates[heal[key]].name)
            for path in sorted((self.root / "data/scripts").glob("*.inc")):
                for dest in WARP_CMDS.findall(read_text(path)):
                    if dest in candidates:
                        seeds.add(candidates[dest].name)
        reached, queue = set(seeds), deque(sorted(seeds))
        while queue:
            for nxt in sorted(links[queue.popleft()]):
                if nxt not in reached:
                    reached.add(nxt)
                    queue.append(nxt)
        for info in candidates.values():
            if info.name not in reached:
                self.excluded["unreachable by warps, connections, scripted warps"].append(info.name)
        self.scope = [m for m in self.order if m.name in reached]
        for m in self.scope:
            m.in_scope = True
        for key in self.excluded:
            self.excluded[key].sort()
        return self.scope


def input_paths(root):
    """Static inputs the generator reads (make prerequisites).

    The mapjson outputs (events.inc, connections.inc, header.inc, groups.inc,
    map_groups.h, map_event_ids.h) are generated; the make fragment lists
    them through map_data_rules.mk's own variables so make builds them first.
    """
    root = Path(root)
    paths = set()
    paths |= {str(p.relative_to(root)) for p in (root / "data/maps").glob("*/map.json")}
    paths |= {str(p.relative_to(root)) for p in (root / "data/maps").glob("*/scripts.inc")}
    paths |= {str(p.relative_to(root)) for p in (root / "data/layouts").rglob("map.bin")}
    paths |= {str(p.relative_to(root)) for p in (root / "data/tilesets").rglob("metatile_attributes.bin")}
    paths |= {str(p.relative_to(root)) for p in (root / "data/scripts").rglob("*.inc")}
    paths |= {str(p.relative_to(root)) for p in
              (root / "graphics/object_events/pics/people").rglob("*.png")}
    paths |= {str(p.relative_to(root)) for p in TOOL_DIR.rglob("*.py")
              if not p.name.startswith("test_")}
    paths |= {str(p.relative_to(root)) for p in TOOL_DIR.rglob("*.json")}
    paths |= {str(p.relative_to(root)) for p in (root / "include/constants").glob("flags*.h")}
    for rel in ("data/layouts/layouts.json", "data/maps/map_groups.json",
                "src/data/region_map/region_map_sections.json",
                "src/data/heal_locations.json", "src/data/wayfarer_sevii_maps.json",
                "tools/wayfarer_hoenn_content/classification.json",
                "tools/notable_trainers/catalog.json",
                "src/data/tilesets/headers.h", "src/data/tilesets/metatiles.h",
                "include/constants/metatile_behaviors.h",
                "include/constants/metatile_behaviors_frlg.h",
                "include/constants/global.h", "include/constants/notable_trainers.h",
                "include/constants/species.h",
                "include/constants/wayfarer_persistence.h",
                "include/constants/wayfarer_world.h", "include/wayfarer_world_data.h",
                "src/fieldmap.c", "src/metatile_behavior.c",
                "src/wayfarer_persistence.c",
                "src/data/object_events/object_event_graphics_info_pointers.h",
                "src/data/object_events/object_event_graphics_info.h",
                "src/data/object_events/object_event_pic_tables.h",
                "src/data/object_events/object_event_graphics.h"):
        paths.add(rel)
    return sorted(paths)
