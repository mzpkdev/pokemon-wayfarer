"""Wild encounters v2 data for the Wayfarer header generator.

Reads the committed v2 tables (game/src/data/wild_encounters_v2/) and returns the
structures wild_encounters_to_header.py renders into gWildMonHeaders (IS_WAYFARER),
gWildEncounterPlaces[] and gWildProwlerMinimums[].

The place and prowler rules here are an independent implementation of hm_model.py
(parse_intents, prowler_min, root, PROWLER_MIN); tests/test_v2_emit.py cross-checks
them against it for every map and every species.
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]  # game/
REPO = ROOT.parent
DATA_DIR = ROOT / "src/data/wild_encounters_v2"
RATES = DATA_DIR / "encounter_rates.json"
PROWLERS_SPEC = REPO / ".product/specs/prowlers.md"
REACH_SPEC = REPO / ".product/specs/reach-assignments.md"
EVOLUTION = ROOT / "tools/notable_trainers/evolution.json"

REGION_FILES = ("kanto", "johto", "hoenn", "alola", "sevii", "safari", "sinjoh")
# v2 method -> (WildPokemonHeader member suffix, slot count, array label)
METHODS = {
    "land": ("landMonsInfo", 12, "LandMons"),
    "surf": ("waterMonsInfo", 5, "WaterMons"),
    "rock": ("rockSmashMonsInfo", 5, "RockSmashMons"),
    "fish": ("fishingMonsInfo", 10, "FishingMons"),
}
# The v2 tables hold a day and a night table per method. Morning and day use the day table,
# evening and night the night table.
TIME_TO_TABLE = {"TIME_MORNING": "day", "TIME_DAY": "day", "TIME_EVENING": "night", "TIME_NIGHT": "night"}
# Slot levels are unused by v2 Wayfarer code (levels come from the place's level); every slot
# carries this valid placeholder level so a reader that misses the change shows level 1 instead of garbage.
PLACEHOLDER_LEVEL = 1

REACHES = {"Road": "WILD_REACH_ROAD", "Wilds": "WILD_REACH_WILDS", "Outlands": "WILD_REACH_OUTLANDS", "Dungeon": "WILD_REACH_DUNGEON"}
INTENTS = {
    "Mild": "WILD_DUNGEON_MILD", "Mild to moderate": "WILD_DUNGEON_MILD_TO_MODERATE", "Moderate": "WILD_DUNGEON_MODERATE",
    "Moderate to hard": "WILD_DUNGEON_MODERATE_TO_HARD", "Hard": "WILD_DUNGEON_HARD", "Brutal": "WILD_DUNGEON_BRUTAL",
}
REGIONS = {"Safari": "WILD_PLACE_REGION_SAFARI", "Sinjoh": "WILD_PLACE_REGION_SINJOH"}  # anything else is WILD_PLACE_REGION_OTHER


class V2Error(ValueError):
    pass


def _json(path):
    try:
        return json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, ValueError) as error:
        raise V2Error(f"{path}: {error}") from error


def normalize(name):
    return (name.upper().replace(" ", "_").replace(".", "").replace("'", "").replace("-", "_")
            .replace("♀", "_F").replace("♂", "_M"))


# ---------- prowlers (specs/prowlers.md) ----------
def parse_prowler_minimums(path=PROWLERS_SPEC):
    """{line key: minimum level} for every non-harmless prowler line, keyed like hm_model.PROWLER_MIN."""
    rows = {}
    for line in Path(path).read_text(encoding="utf-8").split("\n"):
        match = re.match(r"\| (.+?) \| .+? \| (Harmless|Fierce|Dangerous) \| (\d+|—) \|", line)
        if not match:
            continue
        name, temperament, bst = match.groups()
        if temperament == "Harmless":
            minimum = None
        elif temperament == "Dangerous":
            minimum = 30
        else:
            minimum = 25 if bst.isdigit() and int(bst) >= 485 else 20
        rows[name.replace(" line", "")] = minimum
    out = {}
    suffixes = {"Alolan": "_ALOLA", "Galarian": "_GALAR", "Hisuian": "_HISUI", "White-Striped": "_WHITE_STRIPED"}
    for name, minimum in rows.items():
        form = re.match(r"(.+) \((Alolan|Galarian|Hisuian|White-Striped)\)", name)
        out[normalize(form.group(1)) + suffixes[form.group(2)] if form else normalize(name)] = minimum
    return out


class Lines:
    """Evolution lines from species.json: the first non-baby stage of each species."""

    def __init__(self, species_doc, evolution_path=EVOLUTION):
        self.species = species_doc["species"]
        evolution = _json(evolution_path)
        self.babies = set(species_doc["babies"]) | {normalize(b) for b in evolution["babies"]}
        self.predecessors = {}
        for name, info in self.species.items():
            for edge in info["evolves_to"]:
                self.predecessors.setdefault(edge["target"], []).append(name)

    def root(self, name):
        while True:
            parents = [p for p in self.predecessors.get(name, []) if p not in self.babies]
            if not parents:
                return name
            name = parents[0]


# Regional forms are listed separately in prowlers.md ("(Alolan)", ...): a plain row never reaches them.
REGIONAL_TOKENS = ("ALOLA", "GALAR", "HISUI", "PALDEA", "WHITE_STRIPED")


def form_key(name, line_minimums):
    """The prowler row of the species a non-regional form constant belongs to (ORICORIO_PAU -> ORICORIO), or None."""
    for key, minimum in line_minimums.items():
        if minimum is not None and name.startswith(key + "_") and not name[len(key) + 1:].startswith(REGIONAL_TOKENS):
            return key
    return None


def prowler_minimums(species_ids, lines, line_minimums):
    """Sorted [(species id, constant, minimum, exempt in Safari, exempt in Sinjoh)] over every species of a prowler line."""
    out = {}
    for name in sorted(lines.species):
        root = lines.root(name)
        minimum = line_minimums.get(name, line_minimums.get(root))
        if minimum is None:
            # A row also covers every non-regional form constant of its species.
            root = form_key(name, line_minimums) or form_key(root, line_minimums)
            if root is None:
                continue
            minimum = line_minimums[root]
        nat = lines.species.get(root, {}).get("nat", 0)
        safari = bool(nat) and 650 <= nat <= 721  # Kalos rewards have no minimum in the Safari Zones
        sinjoh = name.endswith("_HISUI") or root in ("STANTLER", "SCYTHER", "BASCULIN_WHITE_STRIPED") or root.endswith("_HISUI")
        constant = "SPECIES_" + name
        if constant not in species_ids:
            raise V2Error(f"prowler species {constant} is not a species constant")
        row = (species_ids[constant], constant, minimum, safari, sinjoh)
        if out.get(row[0], row)[2:] != row[2:]:
            raise V2Error(f"{constant}: aliases of one species disagree on the prowler minimum")
        out.setdefault(row[0], row)
    unknown = sorted(key for key in line_minimums if key not in lines.species)
    if unknown:
        raise V2Error(f"prowlers.md names lines that are not v2 species: {unknown}")
    return [out[key] for key in sorted(out)]


# ---------- dungeon intents (specs/reach-assignments.md) ----------
def parse_intents(path=REACH_SPEC):
    """{map: (intent label, flat)} for every dungeon row, read from the Notes column."""
    out = {}
    for line in Path(path).read_text(encoding="utf-8").split("\n"):
        cells = [cell.strip() for cell in line.split("|")]
        if len(cells) < 5 or cells[2] != "Dungeon":
            continue
        notes = cells[4]
        for intent in ("Mild to moderate", "Moderate to hard", "Brutal", "Hard", "Moderate", "Mild"):
            if re.search(r"\b" + intent + r"\b", notes, re.I):
                flat = bool(re.search(r"\bflat\b", notes, re.I)) or "Single floor" in notes
                for map_name in re.findall(r"`(MAP_\w+)`", cells[3]):
                    out[map_name] = (intent, flat)
                break
    return out


# ---------- tables ----------
def load(data_dir=DATA_DIR):
    """(ordered keys, tables, meta, rates, default rates). Order is meta.json's, which keeps the
    three Bug Contest days (Tuesday, Thursday, Saturday) consecutive."""
    data_dir = Path(data_dir)
    meta = _json(data_dir / "meta.json")
    tables = {}
    for region in REGION_FILES:
        tables.update(_json(data_dir / f"{region}.json"))
    if set(tables) != set(meta):
        raise V2Error("v2 tables and meta.json list different maps")
    rates = _json(data_dir / "encounter_rates.json")
    return list(meta), tables, meta, rates["rates"], rates["defaults"]


def map_constant(key):
    """Bug Contest keys are MAP_...:TUESDAY; everything else is the map constant itself."""
    return key.split(":")[0]


def table_rate(key, method, rates, defaults):
    """(day rate, night rate, whether the default was used)."""
    found = rates.get(map_constant(key), {}).get(method)
    if found is None:
        return defaults[method], defaults[method], True
    return found[0], found[1], False


def label_for(key):
    """C identifier stem for a header key, e.g. MAP_ROUTE1_HNS -> gWildV2_Route1_Hns."""
    parts = re.sub(r"^MAP_", "", key).replace(":", "_").lower().split("_")
    return "gWildV2_" + "_".join(part.capitalize() for part in parts)


def place_of(key, meta, intents):
    """Fields of gWildEncounterPlaces[] for a header key."""
    entry = meta[key]
    reach = REACHES[entry["reach"]]
    intent, flat, floor, floor_count = "WILD_DUNGEON_MILD", False, 0, 0
    if entry["reach"] == "Dungeon":
        found = intents.get(map_constant(key))
        if found is None:
            raise V2Error(f"{key}: dungeon has no intent in reach-assignments.md")
        intent, flat = INTENTS[found[0]], found[1]
        floor, floor_count = entry["floor"] or 0, entry["floors"] or 0
        if floor_count and not 0 <= floor < floor_count:
            raise V2Error(f"{key}: floor {floor} outside {floor_count} floors")
    return {"reach": reach, "intent": intent, "flat": int(flat), "region": REGIONS.get(entry["region"], "WILD_PLACE_REGION_OTHER"),
            "floor": floor, "floorCount": floor_count}


def slot_lists(key, tables):
    """{method: {'day': [species], 'night': [species]}} for a header key."""
    out = {}
    for method, (_, count, _) in METHODS.items():
        table = tables[key].get(method)
        if table is None:
            continue
        for time in ("day", "night"):
            if len(table[time]) != count:
                raise V2Error(f"{key}/{method}/{time}: expected {count} slots, found {len(table[time])}")
        out[method] = table
    return out


def render(species_ids, line_minimums=None, data_dir=DATA_DIR):
    """The v2 half of wild_encounters.h: (arrays text, header entries text, trailer text, summary dict).

    arrays: every WildPokemon array and WildPokemonInfo, under #if IS_WAYFARER.
    headers: the gWildMonHeaders[] entries, in header order, to splice inside the array.
    trailer: gWildEncounterPlaces[], gWildProwlerMinimums[] and its count, under #if IS_WAYFARER.
    """
    keys, tables, meta, rates, defaults = load(data_dir)
    intents = parse_intents()
    species_doc = _json(Path(data_dir) / "species.json")
    lines = Lines(species_doc)
    line_minimums = parse_prowler_minimums() if line_minimums is None else line_minimums
    arrays, headers, places = [], [], []
    defaulted, total_arrays, slot_total = [], 0, 0

    for key in keys:
        stem, by_method = label_for(key), slot_lists(key, tables)
        infos = {"day": {}, "night": {}}
        for method, table in by_method.items():
            member, _, suffix = METHODS[method]
            day_rate, night_rate, used_default = table_rate(key, method, rates, defaults)
            if used_default:
                defaulted.append((key, method))
            for time, rate in (("day", day_rate), ("night", night_rate)):
                name = f"{stem}_{time.capitalize()}_{suffix}"
                for species in table[time]:
                    if "SPECIES_" + species not in species_ids:
                        raise V2Error(f"{key}/{method}/{time}: {species} is not a species constant")
                arrays.append(f"const struct WildPokemon {name}[] =\n{{\n"
                              + "".join(f"    {{ {PLACEHOLDER_LEVEL}, {PLACEHOLDER_LEVEL}, SPECIES_{species} }},\n" for species in table[time])
                              + f"}};\nconst struct WildPokemonInfo {name}Info = {{ {rate}, {name} }};\n")
                infos[time][method] = name + "Info"
                total_arrays += 1
                slot_total += len(table[time])
        map_name = map_constant(key)
        block = [f"    {{ // {key}", f"        .mapGroup = MAP_GROUP({map_name}),", f"        .mapNum = MAP_NUM({map_name}),",
                 "        .encounterTypes =", "        {"]
        for time, table in TIME_TO_TABLE.items():
            block.append(f"            [{time}] =")
            block.append("            {")
            for method, (member, _, _) in METHODS.items():
                value = infos[table].get(method)
                block.append(f"                .{member} = {'&' + value if value else 'NULL'},")
            block.append("            },")
        block += ["        },", "    },"]
        headers.append("\n".join(block))
        places.append((key, place_of(key, meta, intents)))

    place_rows = "".join(
        f"    {{ // {key}\n        .reach = {p['reach']}, .intent = {p['intent']}, .flat = {p['flat']}, .region = {p['region']},\n"
        f"        .floor = {p['floor']}, .floorCount = {p['floorCount']},\n    }},\n"
        for key, p in places)
    minimums = prowler_minimums(species_ids, lines, line_minimums)
    prowler_rows = "".join(
        f"    {{ {constant}, {minimum}, {int(safari)}, {int(sinjoh)} }},\n" for _, constant, minimum, safari, sinjoh in minimums)
    trailer = (
        "#if IS_WAYFARER\n"
        "// Parallel to gWildMonHeaders: index = header id. Dungeon fields come from reach-assignments.md and meta.json.\n"
        f"const struct WildEncounterPlace gWildEncounterPlaces[] =\n{{\n{place_rows}}};\n"
        "// The headers before the terminator, so no reader scans for it.\n"
        "const u16 gWildMonHeaderCount = ARRAY_COUNT(gWildMonHeaders) - 1;\n\n"
        "// Every species of a prowler line (spec: prowlers.md), sorted by species id for binary search.\n"
        f"const struct WildProwlerMinimum gWildProwlerMinimums[] =\n{{\n{prowler_rows}}};\n"
        "const u16 gWildProwlerMinimumCount = ARRAY_COUNT(gWildProwlerMinimums);\n"
        "#endif\n")
    arrays_text = ("// Wild encounters v2 (generated from src/data/wild_encounters_v2/). Slot levels are unused: the place's level\n"
                   f"// sets every wild level, so each slot carries the placeholder level {PLACEHOLDER_LEVEL}.\n"
                   + "\n".join(arrays))
    summary = {"headers": len(keys), "arrays": total_arrays, "slots": slot_total,
               "defaulted": defaulted, "prowlers": len(minimums)}
    return arrays_text, "\n".join(headers) + "\n", trailer, summary
