#!/usr/bin/env python3
"""Generate authored wild encounters and deterministic Trainer Rating metadata."""

import argparse
from fractions import Fraction
import io
import json
import os
from pathlib import Path
import re
import stat
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_ENCOUNTERS = ROOT / "src/data/wild_encounters.json"
DEFAULT_SCALING = ROOT / "src/data/wild_encounter_scaling.json"
DEFAULT_SPECIES_METADATA = ROOT / "src/data/wild_encounter_species.json"
DEFAULT_OUTPUT = ROOT / "src/data/wild_encounters.h"
DEFAULT_AUDIT = ROOT / "build/wild-encounter-balance-audit.json"
DEFAULT_CONFIG = ROOT / "include/config/overworld.h"
DEFAULT_RTC = ROOT / "include/constants/rtc.h"
DEFAULT_SPECIES = ROOT / "include/constants/species.h"
DEFAULT_SPECIES_INFO = ROOT / "src/data/pokemon/species_info.h"
DEFAULT_SPECIES_CONFIG = ROOT / "include/config/pokemon.h"

MAX_LEVEL = 100
MAX_OFFSET = 5
MAX_U16 = 0xFFFF
SAMPLE_RATINGS = (0, 4, 8, 16, 30, 40, 55, 65, 80)
IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
SPECIES_IDENTIFIER = re.compile(r"^SPECIES_[A-Z0-9_]+$")
METHOD_AREAS = {
    "land_mons": "WILD_AREA_LAND",
    "water_mons": "WILD_AREA_WATER",
    "rock_smash_mons": "WILD_AREA_ROCKS",
    "fishing_mons": "WILD_AREA_FISHING",
}
RODS = {
    "NONE": "WILD_ENCOUNTER_FISHING_ROD_NONE",
    "OLD_ROD": "WILD_ENCOUNTER_FISHING_ROD_OLD",
    "GOOD_ROD": "WILD_ENCOUNTER_FISHING_ROD_GOOD",
    "SUPER_ROD": "WILD_ENCOUNTER_FISHING_ROD_SUPER",
}
PRODUCTS = (("EMERALD", "Emerald"), ("FIRERED", "FireRed"),
            ("LEAFGREEN", "LeafGreen"), ("POKEMON_HNS", "HNS"))
# The legacy generator detected any time word in a label before removing only a
# terminal ``_Time`` suffix. Mt. Silver's SnowNight map relies on that historic
# binding; retain it deliberately rather than make the general parser ambiguous.
REVIEWED_TIME_BINDINGS = {
    "gMtSilver_SnowNight_hns_Day": ("TIME_NIGHT", "gMtSilver_SnowNight_hns"),
}


class ValidationError(ValueError):
    pass


def load_json(path):
    try:
        return json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ValidationError(f"{path}: {error}") from error


def exact_keys(value, expected, location):
    if not isinstance(value, dict):
        raise ValidationError(f"{location}: expected object")
    missing, unexpected = set(expected) - set(value), set(value) - set(expected)
    if missing or unexpected:
        parts = []
        if missing:
            parts.append(f"missing {sorted(missing)}")
        if unexpected:
            parts.append(f"unexpected {sorted(unexpected)}")
        raise ValidationError(f"{location}: {'; '.join(parts)}")


def integer(value, location, minimum, maximum):
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise ValidationError(f"{location}: expected integer from {minimum} through {maximum}")
    return value


def identifier(value, location, pattern=IDENTIFIER):
    if not isinstance(value, str) or pattern.fullmatch(value) is None:
        raise ValidationError(f"{location}: invalid identifier {value!r}")
    return value


def round_half_up(value):
    return (2 * value.numerator + value.denominator) // (2 * value.denominator)


def divide_round_signed(numerator, denominator):
    if numerator >= 0:
        return (numerator + denominator // 2) // denominator
    return -((-numerator + denominator // 2) // denominator)


class Config:
    def __init__(self, config_path, rtc_path, encounters):
        try:
            rtc = Path(rtc_path).read_text(encoding="utf-8")
            config = Path(config_path).read_text(encoding="utf-8")
        except OSError as error:
            raise ValidationError(str(error)) from error
        match = re.search(r"enum\s+TimeOfDay\s*\{(?P<body>[\s*\w+,=\d]+)\}\s*;", rtc)
        if match is None:
            raise ValidationError(f"{rtc_path}: missing TimeOfDay enum")
        self.times = {}
        for name in re.findall(r"TIME_\w+", match.group("body")):
            self.times[name] = name.title().replace("Time_", "").replace("_", "")
        self.mon_types = []
        for group in encounters.get("wild_encounter_groups", []):
            for field in group.get("fields", []):
                value = field.get("type")
                if not isinstance(value, str):
                    raise ValidationError("wild encounter field has no type")
                if value not in self.mon_types:
                    self.mon_types.append(value)
        if not self.mon_types:
            raise ValidationError("wild encounters define no methods")

        def setting(name):
            found = re.search(rf"#define {name}\s+(\w+)", config)
            if found is None:
                raise ValidationError(f"{config_path}: {name} is not defined")
            return found.group(1)

        self.time_encounters = setting("OW_TIME_OF_DAY_ENCOUNTERS") == "TRUE"
        self.disable_time_fallback = setting("OW_TIME_OF_DAY_DISABLE_FALLBACK") == "TRUE"
        self.time_fallback = setting("OW_TIME_OF_DAY_FALLBACK")


def load_scaling(path):
    source = load_json(path)
    exact_keys(source, {"schemaVersion", "projectionCap", "levelAnchors", "zoneIdentity", "profileOffsets"}, path)
    if source["schemaVersion"] != 1 or isinstance(source["schemaVersion"], bool):
        raise ValidationError(f"{path}/schemaVersion: expected 1")
    cap = integer(source["projectionCap"], f"{path}/projectionCap", 1, 255)
    rows = source["levelAnchors"]
    if not isinstance(rows, list) or len(rows) < 2:
        raise ValidationError(f"{path}/levelAnchors: expected at least two rows")
    anchors, last_rating, last_level = [], None, None
    for index, row in enumerate(rows):
        location = f"{path}/levelAnchors/{index}"
        exact_keys(row, {"rating", "level"}, location)
        rating = integer(row["rating"], f"{location}/rating", 0, cap)
        level = integer(row["level"], f"{location}/level", 1, MAX_LEVEL)
        if last_rating is not None and rating <= last_rating:
            raise ValidationError(f"{location}/rating: anchors must be strictly ordered")
        if last_level is not None and level <= last_level:
            raise ValidationError(f"{location}/level: anchors must rise")
        anchors.append({"rating": rating, "level": level})
        last_rating, last_level = rating, level
    if anchors[0]["rating"] != 0 or anchors[-1]["rating"] != cap:
        raise ValidationError(f"{path}/levelAnchors: anchors must span 0 through the cap")

    identity = source["zoneIdentity"]
    exact_keys(identity, {"opening", "convergence"}, f"{path}/zoneIdentity")
    segments = []
    for name in ("opening", "convergence"):
        location = f"{path}/zoneIdentity/{name}"
        row = identity[name]
        exact_keys(row, {"startRating", "endRating", "startRetentionBasisPoints", "endRetentionBasisPoints", "shape"}, location)
        start = integer(row["startRating"], f"{location}/startRating", 0, cap)
        end = integer(row["endRating"], f"{location}/endRating", 0, cap)
        start_retention = integer(row["startRetentionBasisPoints"], f"{location}/startRetentionBasisPoints", 0, 10000)
        end_retention = integer(row["endRetentionBasisPoints"], f"{location}/endRetentionBasisPoints", 0, 10000)
        if end <= start or row["shape"] not in {"quadraticEaseOut", "quadraticEaseIn"}:
            raise ValidationError(f"{location}: invalid segment")
        segments.append((start, end, start_retention, end_retention, row["shape"]))
    if segments[0][0] != 0 or segments[0][1] != segments[1][0] or segments[1][1] != cap or segments[0][3] != segments[1][2]:
        raise ValidationError(f"{path}/zoneIdentity: segments must be contiguous and joined")

    points, anchor_index, segment_index = [], 0, 0
    for rating in range(cap + 1):
        while rating > anchors[anchor_index + 1]["rating"]:
            anchor_index += 1
        first, second = anchors[anchor_index], anchors[anchor_index + 1]
        progress = Fraction(rating - first["rating"], second["rating"] - first["rating"])
        anchor_level = round_half_up(Fraction(first["level"]) + (second["level"] - first["level"]) * progress)
        while rating > segments[segment_index][1]:
            segment_index += 1
        start, end, initial, final, shape = segments[segment_index]
        progress = Fraction(rating - start, end - start)
        eased = 1 - (1 - progress) ** 2 if shape == "quadraticEaseOut" else progress ** 2
        retention = Fraction(initial, 10000) + Fraction(final - initial, 10000) * eased
        if retention.numerator > MAX_U16 or retention.denominator > MAX_U16:
            raise ValidationError(f"{path}: retention point {rating} does not fit u16")
        points.append({"anchor_level": anchor_level, "retention_numerator": retention.numerator, "retention_denominator": retention.denominator})
    return {"projection_cap": cap, "anchors": anchors, "points": points, "profile_offsets": source["profileOffsets"]}


def product_for(label):
    if "FireRed" in label:
        return "FIRERED"
    if "LeafGreen" in label:
        return "LEAFGREEN"
    if "_Hns" in label or "_hns" in label:
        return "POKEMON_HNS"
    return "EMERALD"


def time_and_header(label, config):
    reviewed = REVIEWED_TIME_BINDINGS.get(label)
    if reviewed is not None:
        if reviewed[0] not in config.times:
            raise ValidationError(f"{label}: reviewed time is not configured")
        return reviewed
    for time, suffix in config.times.items():
        if label.endswith("_" + suffix):
            return time, label[: -len(suffix) - 1]
    return config.time_fallback, label


def standard_profiles(encounters, config):
    groups = encounters.get("wild_encounter_groups")
    if not isinstance(groups, list):
        raise ValidationError("wild_encounters.json: wild_encounter_groups must be a list")
    group = next((group for group in groups if group.get("label") == "gWildMonHeaders"), None)
    if group is None or group.get("for_maps") is not True:
        raise ValidationError("wild_encounters.json: gWildMonHeaders must be map-backed")
    header_ids = {product: {} for product, _ in PRODUCTS}
    profiles = []
    for index, encounter in enumerate(group.get("encounters", [])):
        location = f"wild_encounters.json/gWildMonHeaders/encounters/{index}"
        if not isinstance(encounter, dict):
            raise ValidationError(f"{location}: expected object")
        label = identifier(encounter.get("base_label"), f"{location}/base_label")
        map_name = identifier(encounter.get("map"), f"{location}/map")
        product = product_for(label)
        time, header = time_and_header(label, config)
        header_id = header_ids[product].setdefault(header, len(header_ids[product]))
        profiles.append({"label": label, "map": map_name, "product": product, "time": time, "header": header, "header_id": header_id, "encounter": encounter, "group": group})
    if not profiles:
        raise ValidationError("wild_encounters.json: no ordinary profiles")
    return profiles, header_ids


def species_ids(path):
    try:
        source = Path(path).read_text(encoding="utf-8")
    except OSError as error:
        raise ValidationError(f"{path}: {error}") from error
    expressions = {}
    for match in re.finditer(r"^\s*#define\s+(SPECIES_[A-Z0-9_]+)\s+(.+?)\s*$", source, re.MULTILINE):
        name, expression = match.groups()
        expressions[name] = expression.split("//", 1)[0].strip()
    if not expressions:
        raise ValidationError(f"{path}: missing species constants")
    values, resolving = {}, set()

    def resolve(name):
        if name in values:
            return values[name]
        if name in resolving:
            raise ValidationError(f"{path}: cyclic species alias at {name}")
        if name not in expressions:
            raise ValidationError(f"{path}: unresolved species alias {name}")
        resolving.add(name)
        expression = re.sub(r"\bSPECIES_[A-Z0-9_]+\b", lambda match: str(resolve(match.group())), expressions[name])
        if re.fullmatch(r"[0-9xXa-fA-F\s()+\-*/%<>&|~]+", expression) is None:
            raise ValidationError(f"{path}/{name}: unsupported numeric expression")
        try:
            value = eval(expression, {"__builtins__": {}}, {})
        except (ArithmeticError, SyntaxError) as error:
            raise ValidationError(f"{path}/{name}: invalid numeric expression") from error
        if isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= MAX_U16:
            raise ValidationError(f"{path}/{name}: numeric value does not fit u16")
        resolving.remove(name)
        values[name] = value
        return value

    for name in expressions:
        resolve(name)
    return values


def matching_delimiter(source, start, opening, closing, location):
    depth, quote, escaped = 0, None, False
    for index in range(start, len(source)):
        character = source[index]
        if quote:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == quote:
                quote = None
            continue
        if character in {"'", '"'}:
            quote = character
        elif character == opening:
            depth += 1
        elif character == closing:
            depth -= 1
            if depth == 0:
                return index
            if depth < 0:
                break
    raise ValidationError(f"{location}: unbalanced {opening}{closing}")


def split_top_level(value):
    parts, start, depths = [], 0, {"(": 0, "{": 0, "[": 0}
    closing, quote, escaped = {")": "(", "}": "{", "]": "["}, None, False
    for index, character in enumerate(value):
        if quote:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == quote:
                quote = None
            continue
        if character in {"'", '"'}:
            quote = character
        elif character in depths:
            depths[character] += 1
        elif character in closing:
            depths[closing[character]] -= 1
        elif character == "," and not any(depths.values()):
            parts.append(value[start:index].strip())
            start = index + 1
    if any(depths.values()):
        raise ValidationError("unbalanced nested expression")
    return parts + [value[start:].strip()]


def braced_items(source, location):
    result, depth, start, quote, escaped = [], 0, None, None, False
    for index, character in enumerate(source):
        if quote:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == quote:
                quote = None
            continue
        if character in {"'", '"'}:
            quote = character
        elif character == "{":
            if depth == 0:
                start = index
            depth += 1
        elif character == "}":
            depth -= 1
            if depth < 0:
                raise ValidationError(f"{location}: unbalanced braces")
            if depth == 0:
                result.append(source[start + 1:index])
    if depth:
        raise ValidationError(f"{location}: unbalanced braces")
    return result


def active_evolutions(path):
    command = [os.environ.get("CPP", "cpp"), "-P", "-DTRUE=1", "-DFALSE=0", "-I", str(ROOT / "include"), "-I", str(ROOT), "-include", str(DEFAULT_SPECIES_CONFIG), str(path)]
    try:
        result = subprocess.run(command, text=True, capture_output=True, check=False)
    except OSError as error:
        raise ValidationError(f"{path}: unable to run C preprocessor") from error
    if result.returncode:
        raise ValidationError(f"{path}: preprocessing failed: {result.stderr.strip()}")
    entries = {}
    for match in re.finditer(r"\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*\{", result.stdout):
        species, start = match.group(1), match.end() - 1
        end = matching_delimiter(result.stdout, start, "{", "}", str(path))
        body = result.stdout[start:end + 1]
        evolution = re.search(r"\.evolutions\s*=", body)
        rows = []
        if evolution:
            after = evolution.end()
            first_brace = body.find("{", after)
            if first_brace == -1:
                raise ValidationError(f"{path}/{species}: malformed evolutions")
            prefix = body[after:first_brace]
            if "EVOLUTION" in prefix:
                opening = body.find("(", after, first_brace)
                if opening == -1:
                    raise ValidationError(f"{path}/{species}: malformed EVOLUTION")
                closing = matching_delimiter(body, opening, "(", ")", f"{path}/{species}")
                rows = braced_items(body[opening + 1:closing], f"{path}/{species}")
            else:
                closing = matching_delimiter(body, first_brace, "{", "}", f"{path}/{species}")
                rows = braced_items(body[first_brace + 1:closing], f"{path}/{species}")
        parsed = []
        for row in rows:
            fields = split_top_level(row)
            if fields == ["EVOLUTIONS_END"]:
                continue
            if len(fields) < 3 or IDENTIFIER.fullmatch(fields[0]) is None or SPECIES_IDENTIFIER.fullmatch(fields[2]) is None:
                raise ValidationError(f"{path}/{species}: malformed evolution row")
            parsed.append({"method": fields[0], "parameter": fields[1], "target": fields[2]})
        if species in entries:
            raise ValidationError(f"{path}: duplicate active species {species}")
        entries[species] = parsed
    if not entries:
        raise ValidationError(f"{path}: no active species entries")
    return entries


def validate_encounters(encounters, known_species, config):
    profiles, header_ids = standard_profiles(encounters, config)
    fields = {field.get("type"): field for field in profiles[0]["group"].get("fields", [])}
    if set(fields) != set(config.mon_types):
        raise ValidationError("wild encounter method declarations drifted")
    for method, field in fields.items():
        weights = field.get("encounter_rates")
        if not isinstance(weights, list) or not weights or any(not isinstance(weight, int) or isinstance(weight, bool) or weight <= 0 for weight in weights):
            raise ValidationError(f"wild_encounters.json/{method}: invalid weights")
        if method == "fishing_mons":
            partitions = field.get("groups")
            if not isinstance(partitions, dict) or set(partitions) != {"old_rod", "good_rod", "super_rod"}:
                raise ValidationError("wild_encounters.json/fishing_mons: expected rod partitions")
            slots = [slot for partition in partitions.values() for slot in partition]
            if sorted(slots) != list(range(len(weights))):
                raise ValidationError("wild_encounters.json/fishing_mons: partitions must cover each slot once")
        for profile in profiles:
            entry = profile["encounter"].get(method)
            if entry is None:
                continue
            if not isinstance(entry, dict) or not isinstance(entry.get("mons"), list) or len(entry["mons"]) < len(weights):
                raise ValidationError(f"{profile['label']}/{method}: fewer slots than its runtime weight table")
            for index, mon in enumerate(entry["mons"]):
                location = f"{profile['label']}/{method}/mons/{index}"
                if not isinstance(mon, dict) or set(mon) - {"species", "min_level", "max_level"}:
                    raise ValidationError(f"{location}: malformed slot")
                species = identifier(mon.get("species"), f"{location}/species", SPECIES_IDENTIFIER)
                if species not in known_species:
                    raise ValidationError(f"{location}/species: unknown species")
                minimum = integer(mon.get("min_level", 2), f"{location}/min_level", 1, MAX_LEVEL)
                maximum = integer(mon.get("max_level", 100), f"{location}/max_level", 1, MAX_LEVEL)
    return profiles, header_ids


def build_species_metadata(document, evolutions, known_species, ordinary_species):
    exact_keys(document, {"schemaVersion", "minimumOrdinaryWildLevels", "predecessorResolutions"}, "wild_encounter_species.json")
    if document["schemaVersion"] != 1 or isinstance(document["schemaVersion"], bool):
        raise ValidationError("wild_encounter_species.json/schemaVersion: expected 1")
    floors = {}
    rows = document["minimumOrdinaryWildLevels"]
    if not isinstance(rows, list):
        raise ValidationError("wild_encounter_species.json/minimumOrdinaryWildLevels: expected list")
    for index, row in enumerate(rows):
        location = f"wild_encounter_species.json/minimumOrdinaryWildLevels/{index}"
        exact_keys(row, {"species", "minimumOrdinaryWildLevel"}, location)
        species = identifier(row["species"], f"{location}/species", SPECIES_IDENTIFIER)
        if species not in known_species or species in floors:
            raise ValidationError(f"{location}/species: unknown or duplicate floor")
        floors[species] = integer(row["minimumOrdinaryWildLevel"], f"{location}/minimumOrdinaryWildLevel", 1, MAX_LEVEL)

    candidates = {}
    for predecessor, rows in evolutions.items():
        for evolution in rows:
            if evolution["method"] != "EVO_LEVEL":
                continue
            if not evolution["parameter"].isdecimal():
                raise ValidationError(f"species_info/{predecessor}: EVO_LEVEL must be numeric")
            level = int(evolution["parameter"])
            if level == 0:
                continue
            if not 1 <= level <= MAX_LEVEL or evolution["target"] not in known_species or evolution["target"] not in evolutions:
                raise ValidationError(f"species_info/{predecessor}: malformed numeric evolution")
            candidates.setdefault(evolution["target"], set()).add((predecessor, level))

    resolutions = {}
    rows = document["predecessorResolutions"]
    if not isinstance(rows, list):
        raise ValidationError("wild_encounter_species.json/predecessorResolutions: expected list")
    for index, row in enumerate(rows):
        location = f"wild_encounter_species.json/predecessorResolutions/{index}"
        exact_keys(row, {"species", "predecessorSpecies", "predecessorLevel"}, location)
        species = identifier(row["species"], f"{location}/species", SPECIES_IDENTIFIER)
        predecessor = identifier(row["predecessorSpecies"], f"{location}/predecessorSpecies", SPECIES_IDENTIFIER)
        level = integer(row["predecessorLevel"], f"{location}/predecessorLevel", 1, MAX_LEVEL)
        if species in resolutions or len(candidates.get(species, ())) < 2 or (predecessor, level) not in candidates[species]:
            raise ValidationError(f"{location}: invalid predecessor resolution")
        resolutions[species] = (predecessor, level)

    predecessors = {}
    for species, choices in candidates.items():
        if len(choices) == 1:
            predecessors[species] = next(iter(choices))
        elif species in resolutions:
            predecessors[species] = resolutions[species]
        else:
            choices = ", ".join(f"{source}@{level}" for source, level in sorted(choices))
            raise ValidationError(f"species_info/{species}: ambiguous numeric predecessors {choices}; add a resolution")
    for species in predecessors:
        current, seen = species, set()
        while current in predecessors:
            if current in seen:
                raise ValidationError(f"species_info/{species}: numeric predecessor cycle at {current}")
            seen.add(current)
            current = predecessors[current][0]

    reachable = set()
    for species in ordinary_species:
        current = species
        while True:
            reachable.add(current)
            if current not in predecessors:
                break
            current = predecessors[current][0]
    if not set(floors) <= reachable:
        raise ValidationError("wild_encounter_species.json: reviewed floor is not reachable")
    metadata = []
    for species in sorted(reachable, key=known_species.__getitem__):
        predecessor, level = predecessors.get(species, ("SPECIES_NONE", 0))
        alternate = any(row["method"] != "EVO_LEVEL" and row["target"] == species for row in evolutions.get(predecessor, []))
        metadata.append({"species": species, "species_id": known_species[species], "minimum_level": floors.get(species, 1), "predecessor": predecessor, "predecessor_id": known_species.get(predecessor, 0), "predecessor_level": level, "has_alternate_non_level_route": alternate})
    return metadata


def load_species_metadata(path, species_info_path, known_species, ordinary_species):
    return build_species_metadata(load_json(path), active_evolutions(species_info_path), known_species, ordinary_species)


def load_offsets(rows, profiles, path):
    if not isinstance(rows, list):
        raise ValidationError(f"{path}/profileOffsets: expected list")
    profiles_by_label, result, identities = {profile["label"]: profile for profile in profiles}, [], set()
    for index, row in enumerate(rows):
        location = f"{path}/profileOffsets/{index}"
        exact_keys(row, {"label", "method", "fishingRod", "levelOffset"}, location)
        profile = profiles_by_label.get(identifier(row["label"], f"{location}/label"))
        if profile is None or row["method"] not in METHOD_AREAS or row["method"] not in profile["encounter"] or row["fishingRod"] not in RODS:
            raise ValidationError(f"{location}: unresolved ordinary profile offset")
        if (row["method"] == "fishing_mons") != (row["fishingRod"] != "NONE"):
            raise ValidationError(f"{location}: fishing offsets must name a rod only for fishing")
        level = integer(row["levelOffset"], f"{location}/levelOffset", -MAX_OFFSET, MAX_OFFSET)
        if level == 0:
            raise ValidationError(f"{location}/levelOffset: omit zero offsets")
        item = {"product": profile["product"], "header_id": profile["header_id"], "area": METHOD_AREAS[row["method"]], "time": profile["time"], "rod": RODS[row["fishingRod"]], "level_offset": level}
        key = tuple(item[key] for key in ("product", "header_id", "area", "time", "rod"))
        if key in identities:
            raise ValidationError(f"{location}: duplicate resolved profile offset")
        identities.add(key)
        result.append(item)
    return sorted(result, key=lambda item: (item["product"], item["header_id"], item["area"], item["time"], item["rod"]))


class Assembler:
    def __init__(self, output, data, config):
        self.output, self.data, self.config = output, data, config

    def line(self, value="", depth=0):
        self.output.write("    " * depth + value + "\n")

    def macro(self, key, value):
        self.output.write(f"#define {key} {value}\n")

    def write_macros(self):
        for group in self.data["wild_encounter_groups"]:
            for field in group.get("fields", []):
                base, rates = "ENCOUNTER_CHANCE_" + field["type"].upper(), field["encounter_rates"]
                suffixes = [""] * len(rates)
                for name, indices in field.get("groups", {}).items():
                    for index in indices:
                        suffixes[index] = "_" + name.upper()
                previous_group = previous_macro = None
                for index, rate in enumerate(rates):
                    name = f"{base}{suffixes[index]}_SLOT_{index}"
                    value = str(rate) if suffixes[index] != previous_group else f"({previous_macro} + {rate})"
                    if index and suffixes[index] != previous_group:
                        self.macro(f"{base}{suffixes[index - 1]}_TOTAL", f"({previous_macro})")
                    self.macro(name, value)
                    previous_group, previous_macro = suffixes[index], name
                    if index == len(rates) - 1:
                        self.macro(f"{base}{suffixes[index]}_TOTAL", f"({previous_macro})")
                self.line()

    def write_mons(self, name, entry):
        self.line(f"const struct WildPokemon {name}[] =")
        self.line("{")
        for mon in entry["mons"]:
            self.line(f"{{ {mon.get('min_level', 2)}, {mon.get('max_level', 100)}, {mon['species']} }},", 1)
        self.line("};\n")
        self.line(f"const struct WildPokemonInfo {name}Info = {{ {entry['encounter_rate']}, {name} }};\n")

    def write_terminator(self):
        self.line("{", 1); self.line(".mapGroup = MAP_GROUP(MAP_UNDEFINED),", 2); self.line(".mapNum = MAP_NUM(MAP_UNDEFINED),", 2); self.line(".encounterTypes =", 2); self.line("{", 2)
        for time in self.config.times:
            if not self.config.time_encounters and time != self.config.time_fallback:
                continue
            self.line(f"[{time}] =", 3); self.line("{", 3)
            for method in self.config.mon_types:
                member = method.title().replace("_", "")
                self.line(f".{member[0].lower() + member[1:]}Info = NULL,", 4)
            self.line("},", 3)
        self.line("},", 2); self.line("},", 1)

    def write_headers(self, headers):
        self.line(f"const struct WildPokemonHeader {headers['label']}[] ="); self.line("{")
        for label, data in headers["data"].items():
            self.line(); self.line(f"#ifdef {product_for(label)}"); self.line("{", 1)
            self.line(f".mapGroup = {data['mapGroup']},", 2); self.line(f".mapNum = {data['mapNum']},", 2); self.line(".encounterTypes =", 2); self.line("{", 2)
            for time in self.config.times:
                if not self.config.time_encounters and time != self.config.time_fallback:
                    continue
                self.line(f"[{time}] =", 4); self.line("{", 4)
                for method in self.config.mon_types:
                    member = method.title().replace("_", "")
                    value = data.get(time, {}).get(method, "NULL")
                    self.line(f".{member[0].lower() + member[1:]}Info = {'&' + value if value != 'NULL' else value},", 5)
                self.line("},", 3)
            self.line("},", 2); self.line("},", 1); self.line("#endif")
        self.write_terminator(); self.line("};")

    def write_encounters(self):
        for group in self.data["wild_encounter_groups"]:
            headers, counter = {"label": group["label"], "data": {}}, 1
            for encounter in group["encounters"]:
                map_group, map_num = "0", str(counter)
                if group.get("for_maps", False):
                    map_group, map_num = f"MAP_GROUP({encounter['map']})", f"MAP_NUM({encounter['map']})"
                counter += 1
                time, header = time_and_header(encounter["base_label"], self.config)
                data = headers["data"].setdefault(header, {"mapGroup": map_group, "mapNum": map_num})
                if data["mapGroup"] != map_group or data["mapNum"] != map_num:
                    raise ValidationError(f"{encounter['base_label']}: shared header spans maps")
                time_data = data.setdefault(time, {})
                self.line(f"#ifdef {product_for(header)}")
                for method in self.config.mon_types:
                    if method not in encounter:
                        continue
                    name = encounter["base_label"] + "_" + method.title().replace("_", "")
                    self.write_mons(name, encounter[method])
                    if method in time_data:
                        raise ValidationError(f"{encounter['base_label']}/{method}: duplicate time data")
                    time_data[method] = name + "Info"
                self.line("#endif")
            self.write_headers(headers)


def render_scaling(output, scaling, offsets, metadata):
    output.write("\nconst struct WildEncounterScalingConfig gWildEncounterScalingConfig =\n{\n")
    output.write(f"    .projectionCap = {scaling['projection_cap']},\n}};\n\n")
    output.write("const struct WildEncounterScalingAnchor gWildEncounterScalingAnchors[] =\n{\n")
    for anchor in scaling["anchors"]:
        output.write(f"    {{ {anchor['rating']}, {anchor['level']} }},\n")
    output.write("};\nconst u16 gWildEncounterScalingAnchorCount = ARRAY_COUNT(gWildEncounterScalingAnchors);\n\n")
    output.write("const struct WildEncounterScalingPoint gWildEncounterScalingPoints[] =\n{\n")
    for point in scaling["points"]:
        output.write(f"    {{ {point['anchor_level']}, {point['retention_numerator']}, {point['retention_denominator']} }},\n")
    output.write("};\nconst u16 gWildEncounterScalingPointCount = ARRAY_COUNT(gWildEncounterScalingPoints);\n\n")
    output.write("const struct WildEncounterProfileOffset gWildEncounterProfileOffsets[] =\n{\n")
    if offsets:
        for item in offsets:
            output.write(f"#ifdef {item['product']}\n    {{ {item['header_id']}, {item['area']}, {item['time']}, {item['rod']}, {item['level_offset']} }},\n#endif\n")
    else:
        output.write("    { 0 }, // Typed sentinel; count remains zero.\n")
    output.write("};\n")
    output.write("const u16 gWildEncounterProfileOffsetCount = " + ("ARRAY_COUNT(gWildEncounterProfileOffsets);\n\n" if offsets else "0;\n\n"))
    output.write("const struct WildEncounterSpeciesMetadata gWildEncounterSpeciesMetadata[] =\n{\n")
    for item in metadata:
        output.write(f"    {{ {item['species_id']}, {item['minimum_level']}, {item['predecessor_id']}, {item['predecessor_level']}, {'TRUE' if item['has_alternate_non_level_route'] else 'FALSE'} }},\n")
    output.write("};\nconst u16 gWildEncounterSpeciesMetadataCount = ARRAY_COUNT(gWildEncounterSpeciesMetadata);\n")


def render_header(encounters, config, scaling, offsets, metadata):
    output = io.StringIO()
    output.write("//\n// DO NOT MODIFY THIS FILE! It is auto-generated by tools/wild_encounters/wild_encounters_to_header.py\n//\n\n\n")
    assembler = Assembler(output, encounters, config)
    assembler.write_macros(); assembler.write_encounters(); render_scaling(output, scaling, offsets, metadata)
    return output.getvalue()


def project_level(scaling, vanilla, rating, offset):
    base, high_water = scaling["points"][0]["anchor_level"], 0
    for point in scaling["points"][:rating + 1]:
        raw = point["anchor_level"] + divide_round_signed((vanilla - base) * point["retention_numerator"], point["retention_denominator"])
        high_water = max(high_water, raw)
    return min(max(high_water + offset, 1), MAX_LEVEL)


def effective_species(species, vanilla, level, by_species):
    result, changes = species, []
    while True:
        metadata = by_species[result]
        predecessor = metadata["predecessor"]
        if predecessor == "SPECIES_NONE" or vanilla < metadata["predecessor_level"] or level >= metadata["predecessor_level"]:
            return result, changes
        changes.append((result, predecessor)); result = predecessor


def stage_rank(species, by_species):
    rank = 0
    while by_species[species]["predecessor"] != "SPECIES_NONE":
        rank += 1; species = by_species[species]["predecessor"]
    return rank


def slot_summary(slot, scaling, offset, by_species, failures, location):
    summaries = [{"locked": False, "outcomes": {}, "changes": set()} for _ in range(scaling["projection_cap"] + 1)]
    for vanilla in range(slot["minimumLevel"], slot["maximumLevel"] + 1):
        previous_level, previous_rank = None, None
        for rating in range(scaling["projection_cap"] + 1):
            level = project_level(scaling, vanilla, rating, offset)
            species, changes = effective_species(slot["species"], vanilla, level, by_species)
            outcome = summaries[rating]["outcomes"].setdefault(species, {"minimumLevel": level, "maximumLevel": level})
            outcome["minimumLevel"], outcome["maximumLevel"] = min(outcome["minimumLevel"], level), max(outcome["maximumLevel"], level)
            summaries[rating]["changes"].update(changes)
            if level < by_species[species]["minimum_level"]:
                summaries[rating]["locked"] = True
            rank = stage_rank(species, by_species)
            if previous_level is not None and level < previous_level:
                failures.append(f"{location}/vanilla {vanilla}: level decreases at rating {rating}")
            if previous_rank is not None and rank < previous_rank:
                failures.append(f"{location}/vanilla {vanilla}: evolution outcome regresses at rating {rating}")
            previous_level, previous_rank = level, rank
    unlock, unlocked = None, False
    for rating, summary in enumerate(summaries):
        eligible = not summary["locked"]
        if eligible and unlock is None:
            unlock = rating
        if unlocked and not eligible:
            failures.append(f"{location}: slot relocks at rating {rating}")
        unlocked |= eligible
    return summaries, unlock


def method_slots(profile, method, rod):
    field = next(field for field in profile["group"]["fields"] if field["type"] == method)
    # The existing engine owns selection counts through the field weight table.
    # Some HNS source entries carry inert trailing rows, so audit the same active
    # prefix rather than inventing weights for authored rows the engine cannot pick.
    indices = range(len(field["encounter_rates"])) if method != "fishing_mons" else field["groups"][rod.lower()]
    return [(index, profile["encounter"][method]["mons"][index], field["encounter_rates"][index]) for index in indices]


def audit_method(profile, method, rod, scaling, offset, by_species, failures):
    slots = []
    for index, mon, weight in method_slots(profile, method, rod):
        authored_minimum, authored_maximum = mon.get("min_level", 2), mon.get("max_level", 100)
        # Preserve the authored table verbatim in the generated header. The audit
        # uses its numeric envelope so legacy inverted ranges remain visible but
        # do not make the balance report impossible to produce.
        slot = {"species": mon["species"], "minimumLevel": min(authored_minimum, authored_maximum), "maximumLevel": max(authored_minimum, authored_maximum), "authoredMinimumLevel": authored_minimum, "authoredMaximumLevel": authored_maximum, "authoredRangeWasInverted": authored_minimum > authored_maximum}
        summaries, unlock = slot_summary(slot, scaling, offset, by_species, failures, f"{profile['product']}/{profile['label']}/{method}/{rod}/slot {index}")
        slots.append({"slot": index, "weight": weight, "original": slot, "summaries": summaries, "unlock": unlock})
    samples = []
    for rating in (value for value in SAMPLE_RATINGS if value <= scaling["projection_cap"]):
        locked = [slot for slot in slots if slot["summaries"][rating]["locked"]]
        eligible = [slot for slot in slots if slot not in locked]
        if not eligible:
            failures.append(f"{profile['product']}/{profile['label']}/{method}/{rod}: all slots are locked at rating {rating}")
        total = sum(slot["weight"] for slot in eligible)
        outcomes = []
        for slot in slots:
            summary = slot["summaries"][rating]
            outcomes.append({"slot": slot["slot"], "weight": slot["weight"], "locked": summary["locked"], "unlockRating": slot["unlock"], "effective": [{"species": species, **outcome} for species, outcome in sorted(summary["outcomes"].items())], "stageChanges": [{"fromSpecies": source, "toSpecies": target} for source, target in sorted(summary["changes"])], "renormalizedWeight": None if summary["locked"] else {"numerator": slot["weight"], "denominator": total}})
        samples.append({"rating": rating, "eligibleSlotCount": len(eligible), "lockedSlotCount": len(locked), "eligibleWeight": total, "lockedWeight": sum(slot["weight"] for slot in locked), "slotOutcomes": outcomes})
    return {"label": profile["label"], "map": profile["map"], "header": profile["header"], "headerId": profile["header_id"], "timeOfDay": profile["time"], "method": method, "fishingRod": rod, "encounterRate": profile["encounter"][method]["encounter_rate"], "authoredSlotCount": len(profile["encounter"][method]["mons"]), "runtimeSlotCount": len(slots), "levelOffset": offset, "samples": samples}


def build_wild_encounter_balance_audit(encounters_path=DEFAULT_ENCOUNTERS, scaling_path=DEFAULT_SCALING, config_path=DEFAULT_CONFIG, rtc_constants_path=DEFAULT_RTC, species_path=DEFAULT_SPECIES, wild_encounter_species_path=DEFAULT_SPECIES_METADATA, species_info_path=DEFAULT_SPECIES_INFO):
    encounters = load_json(encounters_path); config = Config(config_path, rtc_constants_path, encounters); scaling = load_scaling(scaling_path); known_species = species_ids(species_path)
    profiles, header_ids = validate_encounters(encounters, known_species, config)
    ordinary_species = {mon["species"] for profile in profiles for method in config.mon_types for mon in profile["encounter"].get(method, {}).get("mons", [])}
    metadata = load_species_metadata(wild_encounter_species_path, species_info_path, known_species, ordinary_species)
    by_species = {item["species"]: item for item in metadata}
    offsets = load_offsets(scaling["profile_offsets"], profiles, scaling_path)
    offset_map = {(item["product"], item["header_id"], item["area"], item["time"], item["rod"]): item["level_offset"] for item in offsets}
    failures, products = [], []
    for product, display in PRODUCTS:
        selected = [profile for profile in profiles if profile["product"] == product]
        if not selected:
            failures.append(f"{display}: no ordinary wild profiles")
        population = []
        for profile in selected:
            for method in config.mon_types:
                if method not in profile["encounter"]:
                    continue
                for rod in (("OLD_ROD", "GOOD_ROD", "SUPER_ROD") if method == "fishing_mons" else ("NONE",)):
                    key = (product, profile["header_id"], METHOD_AREAS[method], profile["time"], RODS[rod])
                    population.append(audit_method(profile, method, rod, scaling, offset_map.get(key, 0), by_species, failures))
        products.append({"product": display, "headerCount": len(header_ids[product]), "profileCount": len(selected), "population": population})
    return {"schemaVersion": 1, "sampleRatings": [rating for rating in SAMPLE_RATINGS if rating <= scaling["projection_cap"]], "projection": {"cap": scaling["projection_cap"], "anchors": scaling["anchors"], "retention": [{"numerator": point["retention_numerator"], "denominator": point["retention_denominator"]} for point in scaling["points"]]}, "products": products, "invariants": {"passed": not failures, "failures": failures}}


def atomic_write(path, content):
    path = Path(path); path.parent.mkdir(parents=True, exist_ok=True)
    try:
        mode = stat.S_IMODE(path.stat().st_mode)
    except FileNotFoundError:
        mode = 0o644
    temporary_name = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=path.parent, prefix=f".{path.name}.", delete=False) as temporary:
            temporary_name = temporary.name; temporary.write(content); temporary.flush(); os.fchmod(temporary.fileno(), mode); os.fsync(temporary.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        if temporary_name:
            try:
                os.unlink(temporary_name)
            except FileNotFoundError:
                pass
        raise


def generate(encounters_path=DEFAULT_ENCOUNTERS, scaling_path=DEFAULT_SCALING, output_path=DEFAULT_OUTPUT, config_path=DEFAULT_CONFIG, rtc_constants_path=DEFAULT_RTC, species_path=DEFAULT_SPECIES, wild_encounter_species_path=DEFAULT_SPECIES_METADATA, species_info_path=DEFAULT_SPECIES_INFO):
    encounters = load_json(encounters_path); config = Config(config_path, rtc_constants_path, encounters); scaling = load_scaling(scaling_path); known_species = species_ids(species_path)
    profiles, _ = validate_encounters(encounters, known_species, config)
    ordinary_species = {mon["species"] for profile in profiles for method in config.mon_types for mon in profile["encounter"].get(method, {}).get("mons", [])}
    metadata = load_species_metadata(wild_encounter_species_path, species_info_path, known_species, ordinary_species)
    atomic_write(output_path, render_header(encounters, config, scaling, load_offsets(scaling["profile_offsets"], profiles, scaling_path), metadata))


def generate_wild_encounter_balance_audit(output_path=DEFAULT_AUDIT, **kwargs):
    audit = build_wild_encounter_balance_audit(**kwargs)
    if audit["invariants"]["failures"]:
        raise ValidationError("wild encounter balance audit invariant failures: " + "; ".join(audit["invariants"]["failures"]))
    atomic_write(output_path, json.dumps(audit, ensure_ascii=True, indent=2, sort_keys=True) + "\n")
    return audit


def arguments():
    parser = argparse.ArgumentParser(description="Generate wild encounter scaling data")
    parser.add_argument("--encounters", type=Path, default=DEFAULT_ENCOUNTERS); parser.add_argument("--scaling", type=Path, default=DEFAULT_SCALING); parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG); parser.add_argument("--rtc-constants", type=Path, default=DEFAULT_RTC); parser.add_argument("--species", type=Path, default=DEFAULT_SPECIES)
    parser.add_argument("--wild-encounter-species", type=Path, default=DEFAULT_SPECIES_METADATA); parser.add_argument("--species-info", type=Path, default=DEFAULT_SPECIES_INFO)
    parser.add_argument("--balance-audit", type=Path, nargs="?", const=DEFAULT_AUDIT)
    return parser.parse_args()


def main():
    args = arguments()
    common = {"encounters_path": args.encounters, "scaling_path": args.scaling, "config_path": args.config, "rtc_constants_path": args.rtc_constants, "species_path": args.species, "wild_encounter_species_path": args.wild_encounter_species, "species_info_path": args.species_info}
    try:
        if args.balance_audit is None:
            generate(output_path=args.output, **common)
        else:
            audit = generate_wild_encounter_balance_audit(args.balance_audit, **common)
            count = sum(len(product["population"]) for product in audit["products"])
            print(f"wild encounter balance audit passed: {args.balance_audit} ({count} method rows)")
    except ValidationError as error:
        raise SystemExit(f"wild encounter generation failed: {error}") from error


if __name__ == "__main__":
    main()
