#!/usr/bin/env python3
"""Generate authored wild encounters and deterministic Trainer Rating metadata."""

import argparse
from fractions import Fraction
import hashlib
import io
import itertools
import json
import math
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
DEFAULT_STANDARD_ROD_FISHING = ROOT / "src/data/standard_rod_fishing.json"
DEFAULT_REGIONS = ROOT / "src/data/wild_encounter_regions.json"
DEFAULT_OUTPUT = ROOT / "src/data/wild_encounters.h"
DEFAULT_AUDIT = ROOT / "build/wild-encounter-balance-audit.json"
DEFAULT_CONFIG = ROOT / "include/config/overworld.h"
DEFAULT_RTC = ROOT / "include/constants/rtc.h"
DEFAULT_SPECIES = ROOT / "include/constants/species.h"
DEFAULT_POKEDEX = ROOT / "include/constants/pokedex.h"
DEFAULT_SPECIES_INFO = ROOT / "src/data/pokemon/species_info.h"
DEFAULT_SPECIES_CONFIG = ROOT / "include/config/pokemon.h"
DEFAULT_TRAINER_RATING = ROOT / "include/trainer_rating.h"

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
FISHING_QUALITIES = ("OLD_ROD", "GOOD_ROD", "SUPER_ROD")
FISHING_SLOT_COUNT = 10
FISHING_BASE_BITE_PERCENT = {"OLD_ROD": 25, "GOOD_ROD": 50, "SUPER_ROD": 75}
ACTIVE_SLOT_COUNTS = {"land_mons": 12, "water_mons": 5, "rock_smash_mons": 5, "fishing_mons": 10}
HABITATS = {"ROUTE_GRASS", "FOREST", "CAVE", "MOUNTAIN", "URBAN_EDGE", "POND", "COAST", "OFFSHORE", "FACILITY", "SAFARI"}
KANTO_REASONS = {"FRLG_SHARED", "FRLG_VERSION_COUNTERPART", "FRLG_DUPLICATE", "GEN2_LOCAL_ADDITION", "LATER_FAMILY_CONTINUITY", "NIGHT_REWEIGHT"}
KANTO_CHANGE_KINDS = {"MERGE", "REWEIGHT", "ADDITION", "FORBIDDEN_REMOVAL", "NIGHT_AUTHORING"}
KANTO_TOPOLOGY_RATE_SHA256 = "1255221864ef279c99f203aab5ea477a8fbb2272752ab5f1895c10a4e2f66280"
KANTO_BASELINE_LEDGER_SHA256 = "73a7fa085d5e54025001484f2c55179a0ed2fe9b61f8848014cfd8ea20dc215f"
KANTO_EQUIVALENT_SOURCES = {
    "MAP_ROUTE21_HNS": "sRoute21North",
    "MAP_VERMILION_CITY_PORT_OUTSIDE_HNS": "sSSAnneExterior",
    "MAP_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS": "sSafariZoneCenter",
    "MAP_FUCHSIA_CITY_SAFARI_ZONE_BRUSH_HNS": "sSafariZoneEast",
    "MAP_FUCHSIA_CITY_SAFARI_ZONE_CAVE_HNS": "sSafariZoneWest",
    "MAP_FUCHSIA_CITY_SAFARI_ZONE_MOUNTAIN_HNS": "sSafariZoneNorth",
    "MAP_MT_MOON_CAVE_HNS": "sMtMoon1F",
    "MAP_DIGLETTS_CAVE_TUNNEL_HNS": "sDiglettsCaveB1F",
    "MAP_CERULEAN_CAVE_1F_HNS": "sCeruleanCave1F",
    "MAP_CERULEAN_CAVE_B1F_HNS": "sCeruleanCave2F",
    "MAP_CERULEAN_CAVE_B2F_HNS": "sCeruleanCaveB1F",
    "MAP_VICTORY_ROAD_KANTO_1F_HNS": "sVictoryRoad1F",
    "MAP_VICTORY_ROAD_KANTO_B1F_HNS": "sVictoryRoad2F",
    "MAP_VICTORY_ROAD_KANTO_B2F_HNS": "sVictoryRoad3F",
}
KANTO_ANALOG_SOURCES = {
    **{(map_name, method): source for map_name, source in (
        ("MAP_ROUTE1_HNS", "sPalletTown"), ("MAP_ROUTE2_HNS", "sViridianCity"),
        ("MAP_ROUTE9_HNS", "sRoute10"), ("MAP_ROUTE14_HNS", "sFuchsiaCity"),
        ("MAP_ROUTE15_HNS", "sFuchsiaCity"),
    ) for method in ("water_mons", "fishing_mons")},
    ("MAP_VICTORY_ROAD_KANTO_1F_HNS", "water_mons"): "sCeruleanCave1F",
    ("MAP_VICTORY_ROAD_KANTO_1F_HNS", "fishing_mons"): "sCeruleanCave1F",
    ("MAP_VICTORY_ROAD_KANTO_B1F_HNS", "water_mons"): "sCeruleanCave1F",
    ("MAP_VICTORY_ROAD_KANTO_B1F_HNS", "fishing_mons"): "sCeruleanCave1F",
    ("MAP_CERULEAN_CAVE_B1F_HNS", "water_mons"): "sCeruleanCave1F",
    ("MAP_CERULEAN_CAVE_B1F_HNS", "fishing_mons"): "sCeruleanCave1F",
    ("MAP_CINNABAR_ISLAND_HNS", "land_mons"): "sRoute21North",
}
KANTO_MAPS = {
    *(f"MAP_ROUTE{number}_HNS" for number in range(1, 26)),
    "MAP_PALLET_TOWN_HNS", "MAP_VIRIDIAN_CITY_HNS", "MAP_PEWTER_CITY_HNS",
    "MAP_CERULEAN_CITY_HNS", "MAP_LAVENDER_TOWN_HNS", "MAP_VERMILION_CITY_HNS",
    "MAP_VERMILION_CITY_PORT_OUTSIDE_HNS", "MAP_CELADON_CITY_HNS",
    "MAP_FUCHSIA_CITY_HNS", "MAP_CINNABAR_ISLAND_HNS", "MAP_SAFFRON_CITY_HNS",
    "MAP_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS", "MAP_FUCHSIA_CITY_SAFARI_ZONE_BRUSH_HNS",
    "MAP_FUCHSIA_CITY_SAFARI_ZONE_CAVE_HNS", "MAP_FUCHSIA_CITY_SAFARI_ZONE_MOUNTAIN_HNS",
    "MAP_VIRIDIAN_FOREST_HNS", "MAP_MT_MOON_CAVE_HNS", "MAP_DIGLETTS_CAVE_TUNNEL_HNS",
    "MAP_ROCK_TUNNEL_1F_HNS", "MAP_ROCK_TUNNEL_B1F_HNS", "MAP_SEAFOAM_ISLANDS_1F_HNS",
    "MAP_SEAFOAM_ISLANDS_B1F_HNS", "MAP_CERULEAN_CAVE_1F_HNS", "MAP_CERULEAN_CAVE_B1F_HNS",
    "MAP_CERULEAN_CAVE_B2F_HNS", "MAP_VICTORY_ROAD_KANTO_1F_HNS",
    "MAP_VICTORY_ROAD_KANTO_B1F_HNS", "MAP_VICTORY_ROAD_KANTO_B2F_HNS",
}
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


def load_standard_rod_fishing(path):
    source = load_json(path)
    exact_keys(source, {"schemaVersion", "qualityWeights", "nativeSurfAccessibility"}, path)
    if source["schemaVersion"] != 1 or isinstance(source["schemaVersion"], bool):
        raise ValidationError(f"{path}/schemaVersion: expected 1")

    quality_weights = source["qualityWeights"]
    exact_keys(quality_weights, set(FISHING_QUALITIES), f"{path}/qualityWeights")
    for quality in FISHING_QUALITIES:
        weights = quality_weights[quality]
        location = f"{path}/qualityWeights/{quality}"
        if not isinstance(weights, list) or len(weights) != FISHING_SLOT_COUNT:
            raise ValidationError(f"{location}: expected exactly ten weights")
        for index, weight in enumerate(weights):
            integer(weight, f"{location}/{index}", 1, 0xFF)
        if sum(weights) != 100:
            raise ValidationError(f"{location}: weights must total 100")

    rows = source["nativeSurfAccessibility"]
    if not isinstance(rows, list) or len(rows) != 20:
        raise ValidationError(f"{path}/nativeSurfAccessibility: expected exactly 20 records")
    expected_fields = {
        "product", "baseLabel", "timeOfDay", "species",
        "expectedOldRodSuccessfulEncounterPercent",
        "minimumOldRodSuccessfulEncounterPercent",
        "minimumOldRodUnmodifiedCastPercent",
    }
    identities = set()
    for index, row in enumerate(rows):
        location = f"{path}/nativeSurfAccessibility/{index}"
        exact_keys(row, expected_fields, location)
        if row["product"] not in dict(PRODUCTS):
            raise ValidationError(f"{location}/product: unsupported product")
        identifier(row["baseLabel"], f"{location}/baseLabel")
        identifier(row["timeOfDay"], f"{location}/timeOfDay")
        identifier(row["species"], f"{location}/species", SPECIES_IDENTIFIER)
        integer(row["expectedOldRodSuccessfulEncounterPercent"], f"{location}/expectedOldRodSuccessfulEncounterPercent", 1, 100)
        integer(row["minimumOldRodSuccessfulEncounterPercent"], f"{location}/minimumOldRodSuccessfulEncounterPercent", 1, 100)
        integer(row["minimumOldRodUnmodifiedCastPercent"], f"{location}/minimumOldRodUnmodifiedCastPercent", 1, 100)
        identity = tuple(row[key] for key in ("product", "baseLabel", "timeOfDay", "species"))
        if identity in identities:
            raise ValidationError(f"{location}: duplicate accessibility record")
        identities.add(identity)
    return source


def trainer_rating_bounds(path, projection_cap):
    try:
        source = Path(path).read_text(encoding="utf-8")
    except OSError as error:
        raise ValidationError(f"{path}: {error}") from error
    values = {}
    for name in ("TRAINER_RATING_MIN", "TRAINER_RATING_MAX"):
        match = re.search(rf"^\s*#define\s+{name}\s+(\d+)\s*$", source, re.MULTILINE)
        if match is None:
            raise ValidationError(f"{path}: {name} is not defined as an integer")
        values[name] = int(match.group(1))
    minimum, maximum = values["TRAINER_RATING_MIN"], values["TRAINER_RATING_MAX"]
    if not 0 <= minimum <= maximum <= projection_cap:
        raise ValidationError(f"{path}: Trainer Rating bounds must fit projection cap {projection_cap}")
    return minimum, maximum


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


def national_dex_ids(path=DEFAULT_POKEDEX):
    try:
        result = subprocess.run(
            [os.environ.get("CPP", "cpp"), "-P", "-DTRUE=1", "-DFALSE=0", "-I", str(ROOT / "include"), "-I", str(ROOT), "-include", str(DEFAULT_SPECIES_CONFIG), str(path)],
            text=True, capture_output=True, check=False,
        )
    except OSError as error:
        raise ValidationError(f"{path}: unable to run C preprocessor") from error
    if result.returncode:
        raise ValidationError(f"{path}: preprocessing failed: {result.stderr.strip()}")
    source = result.stdout
    match = re.search(r"enum\s+NationalDexOrder\s*\{(?P<body>.*?)\};", source, re.DOTALL)
    if match is None:
        raise ValidationError(f"{path}: missing NationalDexOrder enum")
    values, current = {}, -1
    for raw in match.group("body").split(","):
        token = re.sub(r"//.*", "", raw).strip()
        if not token:
            continue
        assignment = re.fullmatch(r"(NATIONAL_DEX_[A-Z0-9_]+)(?:\s*=\s*(\d+))?", token)
        if assignment is None:
            raise ValidationError(f"{path}: malformed National Dex entry {token!r}")
        current = int(assignment.group(2)) if assignment.group(2) is not None else current + 1
        values[assignment.group(1)] = current
    if values.get("NATIONAL_DEX_BULBASAUR") != 1 or not values:
        raise ValidationError(f"{path}: invalid National Dex numbering")
    return values


def active_national_dex(path, dex_path=DEFAULT_POKEDEX):
    command = [os.environ.get("CPP", "cpp"), "-P", "-DTRUE=1", "-DFALSE=0", "-I", str(ROOT / "include"), "-I", str(ROOT), "-include", str(DEFAULT_SPECIES_CONFIG), str(path)]
    try:
        result = subprocess.run(command, text=True, capture_output=True, check=False)
    except OSError as error:
        raise ValidationError(f"{path}: unable to run C preprocessor") from error
    if result.returncode:
        raise ValidationError(f"{path}: preprocessing failed: {result.stderr.strip()}")
    dex_ids = national_dex_ids(dex_path)
    result_by_species = {}
    for match in re.finditer(r"\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*\{", result.stdout):
        species, start = match.group(1), match.end() - 1
        end = matching_delimiter(result.stdout, start, "{", "}", str(path))
        nat_dex = re.search(r"\.natDexNum\s*=\s*(NATIONAL_DEX_[A-Z0-9_]+)", result.stdout[start:end + 1])
        if nat_dex is None:
            continue
        nat_dex_name = nat_dex.group(1)
        for suffix in ("_ALOLA", "_GALAR", "_HISUI", "_PALDEA"):
            base_name = nat_dex_name.removesuffix(suffix)
            if base_name != nat_dex_name and base_name in dex_ids:
                nat_dex_name = base_name
                break
        if nat_dex_name not in dex_ids:
            raise ValidationError(f"{path}/{species}: unknown natDexNum")
        if species in result_by_species:
            raise ValidationError(f"{path}: duplicate active species {species}")
        result_by_species[species] = dex_ids[nat_dex_name]
    if not result_by_species:
        raise ValidationError(f"{path}: no active National Dex metadata")
    return result_by_species


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
            if len(weights) != FISHING_SLOT_COUNT:
                raise ValidationError("wild_encounters.json/fishing_mons: expected exactly ten source slots")
            slots = [slot for partition in partitions.values() for slot in partition]
            if sorted(slots) != list(range(FISHING_SLOT_COUNT)):
                raise ValidationError("wild_encounters.json/fishing_mons: rod partitions must cover slots 0 through 9 exactly once")
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


def _manifest_list(value, location):
    if not isinstance(value, list):
        raise ValidationError(f"{location}: expected list")
    return value


def _active_mons(profile, method):
    return profile["encounter"][method]["mons"][:ACTIVE_SLOT_COUNTS[method]]


def select_source_level_range(fire_red, leaf_green):
    candidates = [(fire_red.get("min_level", 2), fire_red.get("max_level", 100), "FIRERED"),
                  (leaf_green.get("min_level", 2), leaf_green.get("max_level", 100), "LEAFGREEN")]
    minimum, maximum, version = min(candidates, key=lambda row: (row[0] + row[1], row[1], 0 if row[2] == "FIRERED" else 1))
    return {"version": version, "minLevel": minimum, "maxLevel": maximum}


PROFILE_COUNTERPART_SOLVER_VERSION = 1
_PROFILE_COUNTERPART_SOLVER_CACHE = {}
PROTECTED_KANTO_CHINCHOU_SLOTS = {
    "MAP_VERMILION_CITY_HNS": (4, 6),
    "MAP_VERMILION_CITY_PORT_OUTSIDE_HNS": (4, 6),
    "MAP_CINNABAR_ISLAND_HNS": (4, 6),
}


def _profile_assignment_rank(assignment, groups, state_metrics):
    assigned = [(slot, value) for slot, value in enumerate(assignment) if value is not None]
    return (
        -state_metrics["distinct"], state_metrics["error"], state_metrics["imbalance"],
        tuple(slot for slot, _ in assigned),
        tuple(groups[group]["species"][species] for _, (group, species) in assigned),
        tuple(group for _, (group, _) in assigned),
    )


def _profile_assignment_tie_key(assignment, groups):
    assigned = [(slot, value) for slot, value in enumerate(assignment) if value is not None]
    return (
        tuple(slot for slot, _ in assigned),
        tuple(groups[group]["species"][species] for _, (group, species) in assigned),
        # This final key only distinguishes assignments equivalent under every
        # product-specified objective; it cannot change the selected winner.
        tuple(group for _, (group, _) in assigned),
    )


def _certificate_assignment(assignment):
    return [None if value is None else list(value) for value in assignment]


def _hash_certificate_row(digest, row):
    digest.update(json.dumps(row, sort_keys=True, separators=(",", ":")).encode("ascii"))
    digest.update(b"\n")


def _evaluate_profile_assignment(assignment, groups, qualities, weight_vectors):
    source_species = {species for group in groups for species in group["species"]}
    retained, group_presence = set(), []
    total_error, total_imbalance = Fraction(0), Fraction(0)
    budget_pass, ratio_pass, all_differing_members = True, True, True
    for group_index, group in enumerate(groups):
        present = []
        values_by_species = []
        for species_index, species in enumerate(group["species"]):
            slots = [slot for slot, value in enumerate(assignment) if value == (group_index, species_index)]
            present.append(bool(slots))
            if slots:
                retained.add(species)
            values_by_species.append([
                Fraction(sum(weight_vectors[quality][slot] for slot in slots), sum(weight_vectors[quality]))
                for quality in qualities
            ])
        group_presence.append(tuple(present))
        if not group["differing"]:
            continue
        all_differing_members &= all(present)
        for quality_index, _ in enumerate(qualities):
            first, second = values_by_species[0][quality_index], values_by_species[1][quality_index]
            error = abs(first + second - group["budgets"][quality_index])
            tolerance = max(Fraction(1, 50), group["budgets"][quality_index] / 5)
            total_error += error
            total_imbalance += abs(first - second)
            budget_pass &= error <= tolerance
            # The narrow omission exception waives the ratio only for an
            # actually omitted group member. A retained pair still obeys 2:1.
            if all(present):
                ratio_pass &= min(first, second) > 0 and max(first, second) / min(first, second) <= 2
    return {
        "distinct": len(retained),
        "error": total_error,
        "imbalance": total_imbalance,
        "budgetPass": budget_pass,
        "ratioPass": ratio_pass,
        "full": all_differing_members and retained == source_species,
        "retainedSpecies": retained,
        "groupPresence": group_presence,
    }


def _profile_assignment_category(metrics):
    if metrics["full"] and metrics["budgetPass"] and metrics["ratioPass"]:
        return "fullStrict"
    if metrics["full"] and metrics["budgetPass"]:
        return "fullBudget"
    if metrics["full"]:
        return "fullRejectedBudget"
    if metrics["budgetPass"] and metrics["ratioPass"]:
        return "reducedBudget"
    return "reducedRejectedFixedConstraints"


def find_full_profile_counterpart_witness(weight_vectors, groups, protected_slots=()):
    """Return one strict full-retention witness without enumerating reduced space."""
    qualities = tuple(sorted(weight_vectors))
    slot_count = len(weight_vectors[qualities[0]])
    available = tuple(slot for slot in range(slot_count) if slot not in protected_slots)
    differing_candidates = []
    for group_index, group in enumerate(groups):
        if not group["differing"]:
            continue
        candidates = []
        available_mask = sum(1 << slot for slot in available)
        subset = available_mask
        while subset:
            budget_pass = True
            for quality_index, quality in enumerate(qualities):
                total = sum(weight_vectors[quality])
                combined = Fraction(sum(weight_vectors[quality][slot] for slot in available if subset & (1 << slot)), total)
                tolerance = max(Fraction(1, 50), group["budgets"][quality_index] / 5)
                budget_pass &= abs(combined - group["budgets"][quality_index]) <= tolerance
            if budget_pass:
                first_mask = (subset - 1) & subset
                while first_mask:
                    second_mask = subset ^ first_mask
                    if second_mask:
                        ratio_pass = True
                        for quality in qualities:
                            total = sum(weight_vectors[quality])
                            first = Fraction(sum(weight_vectors[quality][slot] for slot in available if first_mask & (1 << slot)), total)
                            second = Fraction(sum(weight_vectors[quality][slot] for slot in available if second_mask & (1 << slot)), total)
                            ratio_pass &= max(first, second) / min(first, second) <= 2
                        if ratio_pass:
                            assignment = [None] * slot_count
                            for slot in available:
                                if first_mask & (1 << slot):
                                    assignment[slot] = (group_index, 0)
                                elif second_mask & (1 << slot):
                                    assignment[slot] = (group_index, 1)
                            candidates.append((subset, tuple(assignment)))
                    first_mask = (first_mask - 1) & subset
            subset = (subset - 1) & available_mask
        candidates.sort(key=lambda item: _profile_assignment_tie_key(item[1], groups))
        differing_candidates.append((group_index, candidates))
    differing_candidates.sort(key=lambda item: (len(item[1]), groups[item[0]]["id"]))
    shared_by_species = {}
    for index, group in enumerate(groups):
        if not group["differing"]:
            shared_by_species.setdefault(group["species"][0], []).append(index)

    def search(position, used, assignment):
        if position == len(differing_candidates):
            free = [slot for slot in available if not used & (1 << slot)]
            retained = {
                groups[group_index]["species"][species_index]
                for value in assignment if value is not None
                for group_index, species_index in (value,)
            }
            missing_shared_species = sorted(set(shared_by_species) - retained)
            if len(free) < len(missing_shared_species):
                return None
            completed = list(assignment)
            for species, slot in zip(missing_shared_species, free):
                group_index = shared_by_species[species][0]
                completed[slot] = (group_index, 0)
            return tuple(completed)
        _, candidates = differing_candidates[position]
        for mask, candidate in candidates:
            if used & mask:
                continue
            merged = tuple(candidate[slot] if candidate[slot] is not None else assignment[slot] for slot in range(slot_count))
            result = search(position + 1, used | mask, merged)
            if result is not None:
                return result
        return None

    return search(0, 0, (None,) * slot_count)


def solve_profile_counterpart_assignment(weight_vectors, canonical_groups, protected_slots=()):
    """Exhaustively solve a profile while compacting equivalent paths by DP state."""
    qualities = tuple(sorted(weight_vectors))
    slot_count = len(weight_vectors[qualities[0]])
    protected_slots = frozenset(protected_slots)
    cache_key = (
        PROFILE_COUNTERPART_SOLVER_VERSION,
        tuple((quality, tuple(weight_vectors[quality])) for quality in qualities),
        tuple(sorted((tuple(group["sourceSlots"]), tuple(group["species"])) for group in canonical_groups)),
        tuple(sorted(protected_slots)),
    )
    cached = _PROFILE_COUNTERPART_SOLVER_CACHE.get(cache_key)
    if cached is not None:
        return cached
    groups = []
    for index, source in enumerate(sorted(canonical_groups, key=lambda item: (tuple(item["sourceSlots"]), tuple(item["species"])))):
        species = tuple(sorted(set(source["species"])))
        groups.append({
            "id": f"G{index:02d}",
            "sourceSlots": tuple(source["sourceSlots"]),
            "species": species,
            "differing": len(species) == 2,
            "budgets": tuple(
                Fraction(sum(weight_vectors[quality][slot] for slot in source["sourceSlots"]), sum(weight_vectors[quality]))
                for quality in qualities
            ),
        })
    differing = [index for index, group in enumerate(groups) if group["differing"]]
    shared = [index for index, group in enumerate(groups) if not group["differing"]]
    differing_position = {group: position for position, group in enumerate(differing)}
    shared_position = {group: position for position, group in enumerate(shared)}

    domain = [None]
    for group_index, group in enumerate(groups):
        domain.extend((group_index, species_index) for species_index in range(len(group["species"])))
    domains = [([None] if slot in protected_slots else domain) for slot in range(slot_count)]
    source_species = sorted({species for group in groups for species in group["species"]})
    source_species_bits = {species: 1 << index for index, species in enumerate(source_species)}
    differing_member_bits = {
        (group_index, species_index): 1 << offset
        for offset, (group_index, species_index) in enumerate(
            (group_index, species_index)
            for group_index, group in enumerate(groups) if group["differing"]
            for species_index in range(len(group["species"]))
        )
    }
    # Count raw full-retention candidates by inclusion-exclusion over the
    # required distinct-species and differing-group-member features.  A source
    # state can cover both features at once, so this is more compact than a
    # two-bitset DP while counting the identical candidate universe.
    feature_offset = len(source_species)
    option_feature_masks = []
    for group_index, group in enumerate(groups):
        for species_index, species in enumerate(group["species"]):
            mask = source_species_bits[species]
            differing_bit = differing_member_bits.get((group_index, species_index))
            if differing_bit is not None:
                mask |= differing_bit << feature_offset
            option_feature_masks.append(mask)
    feature_count = len(source_species) + len(differing_member_bits)
    available_slot_count = slot_count - len(protected_slots)
    full_candidate_count = 0
    for omitted_features in range(1 << feature_count):
        allowed_state_count = 1 + sum(
            not (mask & omitted_features) for mask in option_feature_masks
        )
        term = allowed_state_count ** available_slot_count
        full_candidate_count += -term if omitted_features.bit_count() & 1 else term
    total_candidate_count = 1
    for slot_domain in domains:
        total_candidate_count *= len(slot_domain)

    available_slots = [slot for slot in range(slot_count) if slot not in protected_slots]
    group_candidates = []
    group_candidate_path_counts = []
    for group_index, group in enumerate(groups):
        candidate_classes = {}
        if not group["differing"]:
            group_candidates.append([])
            group_candidate_path_counts.append(0)
            continue
        else:
            available_mask = sum(1 << slot for slot in available_slots)
            subset = available_mask
            while True:
                total_error, budget_pass = Fraction(0), True
                combined_values = []
                for quality_index, quality in enumerate(qualities):
                    total = sum(weight_vectors[quality])
                    combined = Fraction(sum(weight_vectors[quality][slot] for slot in available_slots if subset & (1 << slot)), total)
                    error = abs(combined - group["budgets"][quality_index])
                    tolerance = max(Fraction(1, 50), group["budgets"][quality_index] / 5)
                    total_error += error
                    combined_values.append(combined)
                    budget_pass &= error <= tolerance
                if budget_pass:
                    first_mask = subset
                    while True:
                        second_mask = subset ^ first_mask
                        first_slots = [slot for slot in available_slots if first_mask & (1 << slot)]
                        second_slots = [slot for slot in available_slots if second_mask & (1 << slot)]
                        total_imbalance, ratio_pass = Fraction(0), True
                        for quality_index, quality in enumerate(qualities):
                            total = sum(weight_vectors[quality])
                            first = Fraction(sum(weight_vectors[quality][slot] for slot in first_slots), total)
                            second = combined_values[quality_index] - first
                            total_imbalance += abs(first - second)
                            if first_slots and second_slots:
                                ratio_pass &= max(first, second) / min(first, second) <= 2
                        retained_bits = 0
                        if first_slots:
                            retained_bits |= source_species_bits[group["species"][0]]
                        if second_slots:
                            retained_bits |= source_species_bits[group["species"][1]]
                        assignment = [None] * slot_count
                        for slot in first_slots:
                            assignment[slot] = (group_index, 0)
                        for slot in second_slots:
                            assignment[slot] = (group_index, 1)
                        group_full = bool(first_slots and second_slots)
                        assignment = tuple(assignment)
                        class_key = (subset, retained_bits, group_full, ratio_pass)
                        old_count, old_imbalance, old_assignment = candidate_classes.get(
                            class_key, (0, None, None)
                        )
                        if (
                            old_assignment is None
                            or (total_imbalance, _profile_assignment_tie_key(assignment, groups))
                            < (old_imbalance, _profile_assignment_tie_key(old_assignment, groups))
                        ):
                            old_imbalance, old_assignment = total_imbalance, assignment
                        candidate_classes[class_key] = (
                            old_count + 1, old_imbalance, old_assignment
                        )
                        if first_mask == 0:
                            break
                        first_mask = (first_mask - 1) & subset
                if subset == 0:
                    break
                subset = (subset - 1) & available_mask
        candidates = [
            (slot_mask, retained_bits, total_error, best_imbalance,
             group_full, ratio_pass, best_assignment, path_count)
            for (slot_mask, retained_bits, group_full, ratio_pass),
                (path_count, best_imbalance, best_assignment) in candidate_classes.items()
            for total_error in [sum(
                abs(
                    Fraction(
                        sum(weight_vectors[quality][slot] for slot in available_slots if slot_mask & (1 << slot)),
                        sum(weight_vectors[quality]),
                    ) - group["budgets"][quality_index]
                )
                for quality_index, quality in enumerate(qualities)
            )]
        ]
        candidates.sort(key=lambda candidate: (
            candidate[0], candidate[1], candidate[4], candidate[5],
            candidate[3], _profile_assignment_tie_key(candidate[6], groups),
        ))
        group_candidates.append(candidates)
        group_candidate_path_counts.append(sum(candidate[-1] for candidate in candidates))

    # Exact-cover DP over independently generated canonical-group candidates.
    # Candidate counts only need the occupied slots, retained-source mask, and
    # fixed-constraint flags.  The best exact objective tuple is an accumulator,
    # not part of state equivalence.  Keeping error and imbalance in the key
    # creates millions of equivalent states for twelve-slot land profiles.
    # Collapsing them is exact because both objectives are additive and shared
    # anchor completion changes neither objective.
    layer = {(0, 0, True, True): (1, Fraction(0), Fraction(0), (None,) * slot_count)}
    certificate_layers = []
    processing_order = sorted(differing, key=lambda index: (len(group_candidates[index]), groups[index]["id"]))
    for group_index in processing_order:
        next_layer = {}
        transition_count = 0
        transition_digest = hashlib.sha256()
        for state, (path_count, best_error, best_imbalance, prefix_assignment) in sorted(layer.items()):
            used, retained_bits, all_full, ratios_pass = state
            for (slot_mask, added_bits, added_error, added_imbalance,
                 group_full, group_ratio, group_assignment, candidate_path_count) in group_candidates[group_index]:
                if used & slot_mask:
                    continue
                assignment = tuple(group_assignment[slot] if group_assignment[slot] is not None else prefix_assignment[slot] for slot in range(slot_count))
                next_state = (
                    used | slot_mask, retained_bits | added_bits,
                    all_full and group_full, ratios_pass and group_ratio,
                )
                error = best_error + added_error
                imbalance = best_imbalance + added_imbalance
                old_count, old_error, old_imbalance, old_assignment = next_layer.get(
                    next_state, (0, None, None, None)
                )
                candidate_rank = (error, imbalance, _profile_assignment_tie_key(assignment, groups))
                old_rank = None if old_assignment is None else (
                    old_error, old_imbalance, _profile_assignment_tie_key(old_assignment, groups)
                )
                if old_rank is None or candidate_rank < old_rank:
                    old_error, old_imbalance, old_assignment = error, imbalance, assignment
                next_layer[next_state] = (
                    old_count + path_count * candidate_path_count,
                    old_error, old_imbalance, old_assignment
                )
                _hash_certificate_row(transition_digest, {
                    "sourceState": list(state),
                    "sourcePathCount": path_count,
                    "candidate": {
                        "slotMask": slot_mask,
                        "retainedSourceBits": added_bits,
                        "budgetError": fraction_row(added_error),
                        "counterpartImbalance": fraction_row(added_imbalance),
                        "groupFull": group_full,
                        "ratioPass": group_ratio,
                        "assignment": _certificate_assignment(group_assignment),
                        "pathCount": candidate_path_count,
                    },
                    "targetState": list(next_state),
                })
                transition_count += 1
        state_digest = hashlib.sha256()
        for state, (path_count, error, imbalance, assignment) in sorted(next_layer.items()):
            _hash_certificate_row(state_digest, {
                "state": list(state), "pathCount": path_count,
                "bestBudgetError": fraction_row(error),
                "bestCounterpartImbalance": fraction_row(imbalance),
                "bestAssignment": _certificate_assignment(assignment),
            })
        certificate_layers.append({
            "canonicalGroup": groups[group_index]["id"],
            "candidateClassCount": len(group_candidates[group_index]),
            "candidatePathCount": group_candidate_path_counts[group_index],
            "reachableStateCount": len(next_layer),
            "reachablePathCount": sum(item[0] for item in next_layer.values()),
            "stateDigest": state_digest.hexdigest(),
            "transitionCount": transition_count,
            "transitionDigest": transition_digest.hexdigest(),
        })
        layer = next_layer

    def shared_completion_count(remaining_count, presence_mask):
        present_count = presence_mask.bit_count()
        if remaining_count < present_count:
            return 0
        return sum(
            (-1) ** omitted * math.comb(present_count, omitted) * (present_count + 1 - omitted) ** remaining_count
            for omitted in range(present_count + 1)
        )

    shared_completion_cache = {}

    def best_shared_completion(remaining_slots, required_presence):
        cache_key = (remaining_slots, required_presence)
        if cache_key in shared_completion_cache:
            return shared_completion_cache[cache_key]
        required_groups = sorted(
            (
                shared[shared_index]
                for shared_index in range(len(shared))
                if required_presence & (1 << shared_index)
            ),
            key=lambda group_index: (groups[group_index]["species"][0], groups[group_index]["id"]),
        )
        if len(required_groups) > len(remaining_slots):
            result = None
        else:
            result = [None] * slot_count
            if not required_groups:
                selected_slots = ()
            else:
                occupied_slots = [
                    slot for slot in available_slots if slot not in remaining_slots
                ]
                if occupied_slots:
                    # Every free gap before the last already occupied slot
                    # makes the combined slot-index sequence lexicographically
                    # smaller when filled by a duplicate shared allocation.
                    prefix = tuple(
                        slot for slot in remaining_slots if slot <= occupied_slots[-1]
                    )
                    needed = max(0, len(required_groups) - len(prefix))
                    suffix = tuple(
                        slot for slot in remaining_slots if slot > occupied_slots[-1]
                    )[:needed]
                    selected_slots = prefix + suffix
                else:
                    selected_slots = tuple(remaining_slots[:len(required_groups)])
            # With the slot set fixed, repeat the smallest species/group at the
            # front and place every other required group once at the end. This
            # is the smallest species sequence, then canonical-group sequence.
            group_sequence = (
                [required_groups[0]] * (len(selected_slots) - len(required_groups))
                + required_groups
                if required_groups else []
            )
            for slot, group_index in zip(selected_slots, group_sequence):
                result[slot] = (group_index, 0)
            result = tuple(result)
        shared_completion_cache[cache_key] = result
        return result

    classified = {"fullStrict": 0, "fullBudget": 0, "fullRejectedBudget": 0, "reducedBudget": 0, "reducedRejectedFixedConstraints": 0}
    winners = {"fullStrict": None, "fullBudget": None, "reducedBudget": None}
    enumeration_digest = hashlib.sha256()
    shared_transition_count = 0
    shared_path_count = 0
    for state, (path_count, error, imbalance, assignment) in sorted(layer.items()):
        used = state[0]
        remaining_slots = tuple(slot for slot in available_slots if not used & (1 << slot))
        for presence_mask in range(1 << len(shared)):
            completion_count = shared_completion_count(len(remaining_slots), presence_mask)
            if not completion_count:
                continue
            shared_assignment = best_shared_completion(remaining_slots, presence_mask)
            completed = tuple(
                assignment[slot] if assignment[slot] is not None else shared_assignment[slot]
                for slot in range(slot_count)
            )
            added_bits = sum(
                source_species_bits[groups[group_index]["species"][0]]
                for shared_index, group_index in enumerate(shared)
                if presence_mask & (1 << shared_index)
            )
            completed_state = (
                state[0], state[1] | added_bits, state[2], state[3],
            )
            _, retained_bits, all_differing_counterparts, ratio_pass = completed_state
            retained_count = retained_bits.bit_count()
            full = all_differing_counterparts and retained_count == len(source_species)
            if full and ratio_pass:
                category = "fullStrict"
            elif full:
                category = "fullBudget"
            elif ratio_pass:
                category = "reducedBudget"
            else:
                category = "reducedRejectedFixedConstraints"
            candidate_path_count = path_count * completion_count
            classified[category] += candidate_path_count
            shared_transition_count += 1
            shared_path_count += candidate_path_count
            metrics = {"distinct": retained_count, "error": error, "imbalance": imbalance}
            rank = _profile_assignment_rank(completed, groups, metrics)
            if category in winners:
                old = winners[category]
                if old is None or rank < old[0]:
                    winners[category] = (rank, completed, completed_state, metrics)
            _hash_certificate_row(enumeration_digest, {
                "sourceState": list(state), "sharedPresenceMask": presence_mask,
                "sourcePathCount": path_count, "completionCount": completion_count,
                "category": category,
                "rank": {
                    "distinctSourceSpeciesRetained": -rank[0],
                    "combinedBudgetError": fraction_row(rank[1]),
                    "counterpartImbalance": fraction_row(rank[2]),
                    "slotSequence": list(rank[3]), "speciesSequence": list(rank[4]),
                    "canonicalGroupSequence": list(rank[5]),
                },
            })
    certificate_layers.append({
        "canonicalGroup": "SHARED_ANCHOR_COMPLETION",
        "candidateClassCount": len(shared),
        "completionTransitionClassCount": shared_transition_count,
        "reachablePathCount": shared_path_count,
        "transitionCount": shared_transition_count,
        "transitionDigest": enumeration_digest.hexdigest(),
    })

    reduced_ratio_failure_count = classified["reducedRejectedFixedConstraints"]
    classified["fullRejectedBudget"] = full_candidate_count - classified["fullStrict"] - classified["fullBudget"]
    reduced_budget_failure_count = (
        total_candidate_count - full_candidate_count
        - classified["reducedBudget"] - reduced_ratio_failure_count
    )
    classified["reducedRejectedFixedConstraints"] = (
        total_candidate_count - full_candidate_count - classified["reducedBudget"]
    )

    preferred = next((category for category in ("fullStrict", "fullBudget", "reducedBudget") if winners[category] is not None), None)
    if preferred is None:
        raise ValidationError("profile counterpart solver found no budget-valid assignment")
    selected_rank, selected_assignment, selected_state, selected_metrics = winners[preferred]
    selected_species = {
        groups[group]["species"][species]
        for value in selected_assignment if value is not None
        for group, species in (value,)
    }
    selected_rows = [
        {"targetSlot": slot, "state": "UNASSIGNED"} if value is None else {
            "targetSlot": slot, "state": "SOURCE", "canonicalGroup": groups[value[0]]["id"],
            "species": groups[value[0]]["species"][value[1]],
        }
        for slot, value in enumerate(selected_assignment)
    ]
    result = {
        "solverVersion": PROFILE_COUNTERPART_SOLVER_VERSION,
        "canonicalOrdering": [
            "canonical-groups-by-source-slot-sequence-then-source-species",
            "candidate-slots-by-target-index-with-UNASSIGNED-then-group-id-then-species",
            "differing-candidate-classes-by-slot-mask-retained-bits-group-full-ratio-pass-best-imbalance-lexical-assignment",
            "reachable-source-states-by-native-tuple-order-then-shared-presence-mask-ascending",
            "winner-objectives-distinct-species-desc-budget-error-counterpart-imbalance-target-slot-sequence-species-sequence",
            "canonical-group-sequence-only-breaks-complete-objective-equivalence",
        ],
        "qualities": list(qualities),
        "canonicalGroups": [{
            "id": group["id"], "sourceSlots": list(group["sourceSlots"]),
            "sourceSpecies": list(group["species"]), "differing": group["differing"],
            "sourceBudgets": {
                quality: fraction_row(group["budgets"][index])
                for index, quality in enumerate(qualities)
            },
        } for group in groups],
        "candidateDomains": [[
            {"state": "UNASSIGNED"} if value is None else {
                "state": "SOURCE", "canonicalGroup": groups[value[0]]["id"],
                "species": groups[value[0]]["species"][value[1]],
            }
            for value in slot_domain
        ] for slot_domain in domains],
        "fixedConstraints": {
            "productionSlotCount": slot_count,
            "productionWeights": {quality: list(weight_vectors[quality]) for quality in qualities},
            "protectedChinchouTargetSlots": sorted(protected_slots),
            "provenanceBindingsUsed": False,
            "sharedSourceSpeciesRequireAnchor": True,
            "sharedGroupsUseCounterpartBudget": False,
            "differingGroupTolerance": "max(1/50, sourceBudget/5)",
        },
        "dpCertificate": {
            "digestEncoding": "SHA-256 over newline-terminated canonical JSON objects with sorted keys, compact separators, and exact numerator/denominator fractions",
            "stateEquivalence": "occupied target mask, retained-source bitset, all-differing-counterparts flag, and retained-pair ratio flag; each state stores its path count and exact best budget-error, counterpart-imbalance, and lexical assignment accumulators; shared subsets use exact inclusion-exclusion path counts",
            "transitionRules": [
                "For each differing group, assign every subset of available target slots between its two named members, including the empty subset and either-member omission, and retain exactly the candidates inside every combined-budget tolerance.",
                "Compact candidates with equal occupied mask, retained-source bits, group-full flag, and ratio-pass flag by exact path count while retaining the best imbalance and lexical assignment representative.",
                "Combine differing-group classes only when occupied masks are disjoint; add exact errors and imbalances, multiply path counts, and retain the best objective representative for each equivalent state.",
                "For every reachable differing state, enumerate every shared-group presence subset and count all assignments of remaining slots to exactly that subset or UNASSIGNED by inclusion-exclusion.",
                "Classify full retention by every distinct source species plus both members of every differing group; a retained differing pair must pass two-to-one except in the fullBudget ratio-only category.",
            ],
            "fullRetentionCounting": {
                "method": "inclusion-exclusion over distinct-source-species and differing-group-member features",
                "requiredFeatureCount": feature_count,
                "availableTargetSlotCount": available_slot_count,
            },
            "layers": certificate_layers,
            "completionTransitionClassCount": shared_transition_count,
            "finalReachablePathCount": shared_path_count,
            "totalCandidateCount": total_candidate_count,
            "fullRetentionCandidateCount": full_candidate_count,
            "canonicalEnumerationDigest": enumeration_digest.hexdigest(),
            "candidateCounts": classified,
            "rejectionCounts": {
                "fullCombinedBudgetFailure": classified["fullRejectedBudget"],
                "reducedCombinedBudgetFailure": reduced_budget_failure_count,
                "reducedRetainedPairRatioFailure": reduced_ratio_failure_count,
            },
        },
        "selectedCategory": preferred,
        "selectedAssignment": selected_rows,
        "selectedRank": {
            "distinctSourceSpeciesRetained": -selected_rank[0],
            "combinedBudgetError": fraction_row(selected_rank[1]),
            "counterpartImbalance": fraction_row(selected_rank[2]),
            "slotSequence": list(selected_rank[3]),
            "speciesSequence": list(selected_rank[4]),
            "canonicalGroupSequence": [groups[index]["id"] for index in selected_rank[5]],
        },
        "omittedSourceSpecies": sorted(set(source_species) - selected_species),
        "omittedCounterparts": [
            {"canonicalGroup": group["id"], "species": species}
            for group_index, group in enumerate(groups) if group["differing"]
            for species_index, species in enumerate(group["species"])
            if not any(value == (group_index, species_index) for value in selected_assignment)
        ],
        "_groups": groups,
        "_selectedRaw": selected_assignment,
    }
    _PROFILE_COUNTERPART_SOLVER_CACHE[cache_key] = result
    return result


def validate_kanto_day_counterparts(row, day, by_label, standard_rod, location, enforce=True):
    method = row["method"]
    fire = _active_mons(by_label[row["fireRedSource"][0]], method)
    leaf = _active_mons(by_label[row["leafGreenSource"][0]], method)
    target = _active_mons(day, method)
    grouped = {}
    for slot, (fire_mon, leaf_mon) in enumerate(zip(fire, leaf)):
        grouped.setdefault((fire_mon["species"], leaf_mon["species"]), []).append(slot)
    canonical_groups = [
        {"sourceSlots": slots, "species": pair}
        for pair, slots in grouped.items()
    ]
    weight_vectors = (
        standard_rod["qualityWeights"] if method == "fishing_mons"
        else {"NONE": next(field["encounter_rates"] for field in day["group"]["fields"] if field["type"] == method)}
    )
    protected_slots = set(PROTECTED_KANTO_CHINCHOU_SLOTS.get(row["map"], ())) if method == "fishing_mons" else set()
    if protected_slots and any(target[slot]["species"] != "SPECIES_CHINCHOU" for slot in protected_slots):
        raise ValidationError(f"{location}: named protected Chinchou slots changed")
    qualities = tuple(sorted(weight_vectors))
    prepared_groups = []
    for index, source in enumerate(sorted(canonical_groups, key=lambda item: (tuple(item["sourceSlots"]), tuple(item["species"])))):
        species = tuple(sorted(set(source["species"])))
        prepared_groups.append({
            "id": f"G{index:02d}", "sourceSlots": tuple(source["sourceSlots"]),
            "species": species, "differing": len(species) == 2,
            "budgets": tuple(
                Fraction(sum(weight_vectors[quality][slot] for slot in source["sourceSlots"]), sum(weight_vectors[quality]))
                for quality in qualities
            ),
        })
    group_by_slots = {tuple(group["sourceSlots"]): index for index, group in enumerate(prepared_groups)}
    actual = []
    duplicate_slots = []
    day_provenance = {record["targetSlot"]: record for record in row["provenance"] if record["targetTime"] == "DAY"}
    for slot, mon in enumerate(target):
        provenance = day_provenance[slot]
        source_slots = tuple(provenance["ecologySourceGroup"]["fireRedSlots"])
        group_index = group_by_slots.get(source_slots)
        if provenance["reason"] == "FRLG_DUPLICATE":
            if group_index is None or prepared_groups[group_index]["differing"]:
                raise ValidationError(f"{location}: FRLG_DUPLICATE must use a shared FireRed/LeafGreen ecology group")
            if mon["species"] != prepared_groups[group_index]["species"][0]:
                raise ValidationError(f"{location}: FRLG_DUPLICATE target species does not match its shared ecology group")
            duplicate_slots.append((slot, group_index))
            actual.append(None)
            continue
        if (slot in protected_slots
         or provenance["reason"] not in {"FRLG_SHARED", "FRLG_VERSION_COUNTERPART"}
         or mon["species"] not in {species for group in prepared_groups for species in group["species"]}):
            actual.append(None)
            continue
        if group_index is None or mon["species"] not in prepared_groups[group_index]["species"]:
            raise ValidationError(f"{location}: final provenance does not map the authored source species to a canonical FRLG group")
        actual.append((group_index, prepared_groups[group_index]["species"].index(mon["species"])))
    actual = tuple(actual)
    for slot, group_index in duplicate_slots:
        if not any(
            other_slot != slot and value == (group_index, 0)
            for other_slot, value in enumerate(actual)
        ):
            raise ValidationError(
                f"{location}: FRLG_DUPLICATE requires a selected shared source allocation in another target slot"
            )
    actual_metrics = _evaluate_profile_assignment(actual, prepared_groups, qualities, weight_vectors)
    if actual_metrics["full"] and actual_metrics["budgetPass"] and actual_metrics["ratioPass"]:
        assigned = [(slot, value) for slot, value in enumerate(actual) if value is not None]
        return [{
            "solverVersion": PROFILE_COUNTERPART_SOLVER_VERSION,
            "proofKind": "FULL_RETENTION_WITNESS",
            "canonicalOrdering": ["canonical-group-by-source-slots-and-species", "target-slot-index", "species-constant"],
            "canonicalGroups": [{
                "id": group["id"], "sourceSlots": list(group["sourceSlots"]),
                "sourceSpecies": list(group["species"]), "differing": group["differing"],
                "sourceBudgets": {quality: fraction_row(group["budgets"][index]) for index, quality in enumerate(qualities)},
            } for group in prepared_groups],
            "candidateDomains": [[
                {"state": "UNASSIGNED"},
                *[
                    {"state": "SOURCE", "canonicalGroup": group["id"], "species": species}
                    for group in prepared_groups for species in group["species"]
                ],
            ] if slot not in protected_slots else [{"state": "UNASSIGNED"}] for slot in range(len(target))],
            "fixedConstraints": {
                "productionSlotCount": len(target),
                "productionWeights": {quality: list(weight_vectors[quality]) for quality in qualities},
                "protectedChinchouTargetSlots": sorted(protected_slots),
                "provenanceBindingsUsed": False,
                "sharedSourceSpeciesRequireAnchor": True,
                "sharedGroupsUseCounterpartBudget": False,
            },
            "selectedCategory": "fullStrict",
            "selectedAssignment": [
                {"targetSlot": slot, "state": "UNASSIGNED"} if value is None else {
                    "targetSlot": slot, "state": "SOURCE", "canonicalGroup": prepared_groups[value[0]]["id"],
                    "species": prepared_groups[value[0]]["species"][value[1]],
                } for slot, value in enumerate(actual)
            ],
            "selectedRank": {
                "distinctSourceSpeciesRetained": actual_metrics["distinct"],
                "combinedBudgetError": fraction_row(actual_metrics["error"]),
                "counterpartImbalance": fraction_row(actual_metrics["imbalance"]),
                "slotSequence": [slot for slot, _ in assigned],
                "speciesSequence": [prepared_groups[group]["species"][species] for _, (group, species) in assigned],
                "canonicalGroupSequence": [prepared_groups[group]["id"] for _, (group, _) in assigned],
            },
            "dpCertificate": {"proofKind": "AUTHORED_FULL_RETENTION_WITNESS", "recomputedByValidator": True},
            "currentAuthoredClassification": {
                "category": "fullStrict", "fullRetention": True,
                "combinedBudgetSatisfied": True, "retainedPairRatiosSatisfied": True,
                "satisfiesSelectedCategory": True,
            },
            "omittedSourceSpecies": [], "omittedCounterparts": [],
        }]

    full_witness = find_full_profile_counterpart_witness(weight_vectors, prepared_groups, protected_slots)
    if full_witness is not None:
        witness_metrics = _evaluate_profile_assignment(full_witness, prepared_groups, qualities, weight_vectors)
        witness_rows = [
            {"targetSlot": slot, "state": "UNASSIGNED"} if value is None else {
                "targetSlot": slot, "state": "SOURCE", "canonicalGroup": prepared_groups[value[0]]["id"],
                "species": prepared_groups[value[0]]["species"][value[1]],
            } for slot, value in enumerate(full_witness)
        ]
        if enforce:
            raise ValidationError(
                f"{location}: full-retention assignment is feasible; authored assignment fails fixed constraints; "
                f"witness {witness_rows}"
            )
        rank = _profile_assignment_rank(full_witness, prepared_groups, witness_metrics)
        return [{
            "solverVersion": PROFILE_COUNTERPART_SOLVER_VERSION,
            "proofKind": "FULL_RETENTION_WITNESS",
            "qualities": list(qualities),
            "canonicalGroups": [{
                "id": group["id"], "sourceSlots": list(group["sourceSlots"]),
                "sourceSpecies": list(group["species"]), "differing": group["differing"],
                "sourceBudgets": {
                    quality: fraction_row(group["budgets"][index])
                    for index, quality in enumerate(qualities)
                },
            } for group in prepared_groups],
            "candidateDomains": [[
                {"state": "UNASSIGNED"},
                *[
                    {"state": "SOURCE", "canonicalGroup": group["id"], "species": species}
                    for group in prepared_groups for species in group["species"]
                ],
            ] if slot not in protected_slots else [{"state": "UNASSIGNED"}] for slot in range(len(target))],
            "fixedConstraints": {
                "productionSlotCount": len(target),
                "productionWeights": {quality: list(weight_vectors[quality]) for quality in qualities},
                "protectedChinchouTargetSlots": sorted(protected_slots),
                "provenanceBindingsUsed": False,
                "sharedSourceSpeciesRequireAnchor": True,
                "sharedGroupsUseCounterpartBudget": False,
                "differingGroupTolerance": "max(1/50, sourceBudget/5)",
            },
            "selectedCategory": "fullStrict",
            "selectedAssignment": witness_rows,
            "selectedRank": {
                "distinctSourceSpeciesRetained": -rank[0],
                "combinedBudgetError": fraction_row(rank[1]),
                "counterpartImbalance": fraction_row(rank[2]),
                "slotSequence": list(rank[3]), "speciesSequence": list(rank[4]),
                "canonicalGroupSequence": [prepared_groups[index]["id"] for index in rank[5]],
            },
            "currentAuthoredClassification": {
                "category": _profile_assignment_category(actual_metrics),
                "fullRetention": actual_metrics["full"],
                "combinedBudgetSatisfied": actual_metrics["budgetPass"],
                "retainedPairRatiosSatisfied": actual_metrics["ratioPass"],
                "satisfiesSelectedCategory": False,
            },
            "omittedSourceSpecies": [], "omittedCounterparts": [],
            "dpCertificate": {"proofKind": "FULL_RETENTION_WITNESS", "recomputedByValidator": True},
        }]

    solution = solve_profile_counterpart_assignment(weight_vectors, canonical_groups, protected_slots)
    preferred = solution["selectedCategory"]
    if enforce and preferred == "fullStrict":
        # A strict full-retention assignment need not use one arbitrary solver
        # witness, but it must independently satisfy the same fixed constraints.
        if any(item["species"] in solution["omittedSourceSpecies"] for item in solution["selectedAssignment"] if item["state"] == "SOURCE"):
            raise AssertionError("invalid solver omission accounting")
        if not (actual_metrics["full"] and actual_metrics["budgetPass"] and actual_metrics["ratioPass"]):
            omitted = sorted({species for group in solution["_groups"] for species in group["species"]} - actual_metrics["retainedSpecies"])
            raise ValidationError(
                f"{location}: full-retention assignment is feasible; authored assignment fails fixed constraints"
                f" (omitted source species {omitted})"
            )
    elif enforce and actual != solution["_selectedRaw"]:
        raise ValidationError(
            f"{location}: counterpart assignment is not the deterministic profile solver choice "
            f"{solution['selectedAssignment']}"
        )
    public = {key: value for key, value in solution.items() if not key.startswith("_")}
    public["currentAuthoredClassification"] = {
        "category": _profile_assignment_category(actual_metrics),
        "fullRetention": actual_metrics["full"],
        "combinedBudgetSatisfied": actual_metrics["budgetPass"],
        "retainedPairRatiosSatisfied": actual_metrics["ratioPass"],
        "matchesSelectedAssignment": actual == solution["_selectedRaw"],
        "satisfiesSelectedCategory": (
            preferred == "fullStrict"
            and actual_metrics["full"] and actual_metrics["budgetPass"] and actual_metrics["ratioPass"]
        ) or (preferred != "fullStrict" and actual == solution["_selectedRaw"]),
    }
    return [public]


def validate_regional_manifest(document, profiles, config, known_species=None, path=DEFAULT_REGIONS, nat_dex_by_species=None):
    exact_keys(document, {"schemaVersion", "regions"}, path)
    if document["schemaVersion"] != 1 or isinstance(document["schemaVersion"], bool):
        raise ValidationError(f"{path}/schemaVersion: expected 1")
    exact_keys(document["regions"], {"KANTO", "JOHTO"}, f"{path}/regions")
    kanto = document["regions"]["KANTO"]
    exact_keys(kanto, {"product", "profiles", "changes"}, f"{path}/regions/KANTO")
    if kanto["product"] != "POKEMON_HNS":
        raise ValidationError(f"{path}/regions/KANTO/product: expected POKEMON_HNS")

    by_label = {profile["label"]: profile for profile in profiles}
    manifest_profiles, identities, aliases, maps, counterpart_proofs = [], set(), {}, set(), []
    standard_rod = load_standard_rod_fishing(DEFAULT_STANDARD_ROD_FISHING)
    profile_fields = {
        "map", "method", "dayBaseLabel", "nightBaseLabel", "nightMode",
        "activeSlotCount", "sourceKind", "fireRedSource", "leafGreenSource",
        "habitat", "provenance",
    }
    provenance_fields = {"targetTime", "targetSlot", "ecologySourceGroup", "levelSource", "reason"}
    ecology_fields = {"method", "fireRedSlots", "leafGreenSlots"}
    level_fields = {"version", "baseLabel", "method", "slot", "minLevel", "maxLevel"}
    for index, row in enumerate(_manifest_list(kanto["profiles"], f"{path}/regions/KANTO/profiles")):
        location = f"{path}/regions/KANTO/profiles/{index}"
        exact_keys(row, profile_fields, location)
        map_name = identifier(row["map"], f"{location}/map")
        method = row["method"]
        if method not in ACTIVE_SLOT_COUNTS:
            raise ValidationError(f"{location}/method: unsupported method")
        identity = (map_name, method)
        if identity in identities:
            raise ValidationError(f"{location}: duplicate Kanto profile identity")
        identities.add(identity); maps.add(map_name)
        if map_name not in KANTO_MAPS:
            raise ValidationError(f"{location}/map: not in the frozen Kanto ownership manifest")
        expected_count = ACTIVE_SLOT_COUNTS[method]
        if integer(row["activeSlotCount"], f"{location}/activeSlotCount", 1, expected_count) != expected_count:
            raise ValidationError(f"{location}/activeSlotCount: expected {expected_count}")
        if row["sourceKind"] not in {"DIRECT", "EQUIVALENT", "ANALOG"}:
            raise ValidationError(f"{location}/sourceKind: unsupported source kind")
        if row["habitat"] not in HABITATS:
            raise ValidationError(f"{location}/habitat: unsupported habitat")

        day_label = identifier(row["dayBaseLabel"], f"{location}/dayBaseLabel")
        night_label = identifier(row["nightBaseLabel"], f"{location}/nightBaseLabel")
        day = by_label.get(day_label)
        if day is None or day["product"] != "POKEMON_HNS" or day["map"] != map_name or day["time"] != "TIME_DAY" or method not in day["encounter"]:
            raise ValidationError(f"{location}/dayBaseLabel: unresolved Kanto day profile")
        if row["nightMode"] not in {"AUTHORED", "DAY_ALIAS"}:
            raise ValidationError(f"{location}/nightMode: expected AUTHORED or DAY_ALIAS")
        night = by_label.get(night_label)
        if row["nightMode"] == "AUTHORED":
            if night is None or night["product"] != "POKEMON_HNS" or night["map"] != map_name or night["time"] != "TIME_NIGHT" or method not in night["encounter"]:
                raise ValidationError(f"{location}/nightBaseLabel: unresolved authored Kanto night profile")
        else:
            if night is not None:
                raise ValidationError(f"{location}/nightBaseLabel: DAY_ALIAS label must be absent from wild_encounters.json")
            if time_and_header(night_label, config)[0] != "TIME_NIGHT" or time_and_header(night_label, config)[1] != day["header"]:
                raise ValidationError(f"{location}/nightBaseLabel: DAY_ALIAS must bind the day header at TIME_NIGHT")
            binding = (map_name, day["header"])
            if night_label in aliases and aliases[night_label] != binding:
                raise ValidationError(f"{location}/nightBaseLabel: DAY_ALIAS label spans maps or headers")
            aliases[night_label] = binding

        source_labels = {}
        for version, key, product in (("FIRERED", "fireRedSource", "FIRERED"), ("LEAFGREEN", "leafGreenSource", "LEAFGREEN")):
            labels = _manifest_list(row[key], f"{location}/{key}")
            if not labels:
                raise ValidationError(f"{location}/{key}: expected at least one source label")
            if len(set(labels)) != len(labels):
                raise ValidationError(f"{location}/{key}: duplicate source label")
            source_labels[version] = labels
            for source_label in labels:
                identifier(source_label, f"{location}/{key}")
                source = by_label.get(source_label)
                if source is None:
                    raise ValidationError(f"{location}/{key}: unknown source label {source_label}")
                if source["product"] != product or method not in source["encounter"]:
                    raise ValidationError(f"{location}/{key}: source product or target method mismatch")

        analog = KANTO_ANALOG_SOURCES.get(identity)
        equivalent = KANTO_EQUIVALENT_SOURCES.get(map_name)
        if analog is None and method == "rock_smash_mons":
            direct_map = map_name.removesuffix("_HNS")
            direct_exists = any(
                source["map"] == direct_map and source["product"] == version and method in source["encounter"]
                for source in profiles for version in ("FIRERED", "LEAFGREEN")
            )
            equivalent_exists = equivalent is not None and all(
                f"{equivalent}_{suffix}" in by_label and method in by_label[f"{equivalent}_{suffix}"]["encounter"]
                for suffix in ("FireRed", "LeafGreen")
            )
            if not direct_exists and not equivalent_exists:
                analog = "sRockTunnelB1F"
        if analog is not None:
            expected_kind, expected_stem = "ANALOG", analog
        elif equivalent is not None:
            expected_kind, expected_stem = "EQUIVALENT", equivalent
        else:
            expected_kind, expected_stem = "DIRECT", None
        if row["sourceKind"] != expected_kind:
            raise ValidationError(f"{location}/sourceKind: expected {expected_kind}")
        if expected_stem is not None:
            expected_sources = {
                "FIRERED": [f"{expected_stem}_FireRed"],
                "LEAFGREEN": [f"{expected_stem}_LeafGreen"],
            }
            if source_labels != expected_sources:
                raise ValidationError(f"{location}: {expected_kind} source labels do not match the frozen mapping")
        else:
            direct_map = map_name.removesuffix("_HNS")
            if any(by_label[label]["map"] != direct_map for labels in source_labels.values() for label in labels):
                raise ValidationError(f"{location}: DIRECT source does not use the matching numbered map")

        provenance = _manifest_list(row["provenance"], f"{location}/provenance")
        expected_slots = {(time, slot) for time in ("DAY", "NIGHT") for slot in range(expected_count)}
        actual_slots, normalized_provenance = set(), []
        for provenance_index, record in enumerate(provenance):
            record_location = f"{location}/provenance/{provenance_index}"
            exact_keys(record, provenance_fields, record_location)
            target_time = record["targetTime"]
            if target_time not in {"DAY", "NIGHT"}:
                raise ValidationError(f"{record_location}/targetTime: expected DAY or NIGHT")
            target_slot = integer(record["targetSlot"], f"{record_location}/targetSlot", 0, expected_count - 1)
            target_key = (target_time, target_slot)
            if target_key in actual_slots:
                raise ValidationError(f"{record_location}: duplicate target slot")
            actual_slots.add(target_key)
            if record["reason"] not in KANTO_REASONS:
                raise ValidationError(f"{record_location}/reason: unsupported provenance reason")
            ecology = record["ecologySourceGroup"]
            exact_keys(ecology, ecology_fields, f"{record_location}/ecologySourceGroup")
            if ecology["method"] != method:
                raise ValidationError(f"{record_location}/ecologySourceGroup/method: source and target method mismatch")
            ecology_slots = {}
            for version, key in (("FIRERED", "fireRedSlots"), ("LEAFGREEN", "leafGreenSlots")):
                values = _manifest_list(ecology[key], f"{record_location}/ecologySourceGroup/{key}")
                if not values or len(set(values)) != len(values):
                    raise ValidationError(f"{record_location}/ecologySourceGroup/{key}: expected unique source slots")
                ecology_slots[version] = [integer(value, f"{record_location}/ecologySourceGroup/{key}", 0, expected_count - 1) for value in values]
            if len(ecology_slots["FIRERED"]) != len(ecology_slots["LEAFGREEN"]):
                raise ValidationError(f"{record_location}/ecologySourceGroup: version slot arrays must have equal length")
            if ecology_slots["FIRERED"] != ecology_slots["LEAFGREEN"]:
                raise ValidationError(f"{record_location}/ecologySourceGroup: paired FRLG roles must use the same slot indices")
            fire_source_mons = _active_mons(by_label[source_labels["FIRERED"][0]], method)
            leaf_source_mons = _active_mons(by_label[source_labels["LEAFGREEN"][0]], method)
            first_slot = ecology_slots["FIRERED"][0]
            source_pair = (fire_source_mons[first_slot]["species"], leaf_source_mons[first_slot]["species"])
            complete_group = [
                slot for slot, (fire_mon, leaf_mon) in enumerate(zip(fire_source_mons, leaf_source_mons))
                if (fire_mon["species"], leaf_mon["species"]) == source_pair
            ]
            if ecology_slots["FIRERED"] != complete_group:
                raise ValidationError(f"{record_location}/ecologySourceGroup: source group omits or adds paired ecology roles")

            level = record["levelSource"]
            exact_keys(level, level_fields, f"{record_location}/levelSource")
            version = level["version"]
            if version not in source_labels:
                raise ValidationError(f"{record_location}/levelSource/version: unsupported version")
            base_label = identifier(level["baseLabel"], f"{record_location}/levelSource/baseLabel")
            if base_label not in source_labels[version] or level["method"] != method:
                raise ValidationError(f"{record_location}/levelSource: source label or method is outside the ecology source")
            source_slot = integer(level["slot"], f"{record_location}/levelSource/slot", 0, expected_count - 1)
            if source_slot not in ecology_slots[version]:
                raise ValidationError(f"{record_location}/levelSource/slot: slot is outside ecologySourceGroup")
            source_mon = _active_mons(by_label[base_label], method)[source_slot]
            source_minimum, source_maximum = source_mon.get("min_level", 2), source_mon.get("max_level", 100)
            if level["minLevel"] != source_minimum or level["maxLevel"] != source_maximum:
                raise ValidationError(f"{record_location}/levelSource: recorded source range does not match wild_encounters.json")
            target_profile = day if target_time == "DAY" or row["nightMode"] == "DAY_ALIAS" else night
            target_mon = _active_mons(target_profile, method)[target_slot]
            if target_mon.get("min_level", 2) != source_minimum or target_mon.get("max_level", 100) != source_maximum:
                raise ValidationError(f"{record_location}: target range does not match selected source range")
            paired_position = ecology_slots[version].index(source_slot)
            other_version = "LEAFGREEN" if version == "FIRERED" else "FIRERED"
            other_slot = ecology_slots[other_version][paired_position]
            fire_mon = _active_mons(by_label[source_labels["FIRERED"][0]], method)[ecology_slots["FIRERED"][paired_position]]
            leaf_mon = _active_mons(by_label[source_labels["LEAFGREEN"][0]], method)[ecology_slots["LEAFGREEN"][paired_position]]
            pair_species = (fire_mon["species"], leaf_mon["species"])
            if record["reason"] == "FRLG_DUPLICATE" and pair_species[0] != pair_species[1]:
                raise ValidationError(f"{record_location}: FRLG_DUPLICATE must use a shared FireRed/LeafGreen ecology group")
            if pair_species[0] == pair_species[1] or target_mon["species"] not in pair_species:
                selected = select_source_level_range(fire_mon, leaf_mon)
                if (version, source_minimum, source_maximum) != (selected["version"], selected["minLevel"], selected["maxLevel"]):
                    raise ValidationError(f"{record_location}/levelSource: does not use the required lower source range")
            else:
                expected_version = "FIRERED" if target_mon["species"] == pair_species[0] else "LEAFGREEN"
                if version != expected_version:
                    raise ValidationError(f"{record_location}/levelSource: counterpart species must use its own version range")
            if record["reason"] in {"FRLG_SHARED", "FRLG_VERSION_COUNTERPART", "FRLG_DUPLICATE"}:
                group_species = set()
                for source_version, slot_key in (("FIRERED", "fireRedSlots"), ("LEAFGREEN", "leafGreenSlots")):
                    for source_label in source_labels[source_version]:
                        source_mons = _active_mons(by_label[source_label], method)
                        group_species.update(source_mons[slot]["species"] for slot in ecology[slot_key])
                if target_mon["species"] not in group_species:
                    raise ValidationError(f"{record_location}: target species does not match its FRLG ecology source group")
            normalized_provenance.append(record)
        if actual_slots != expected_slots:
            missing = sorted(expected_slots - actual_slots)
            extra = sorted(actual_slots - expected_slots)
            raise ValidationError(f"{location}/provenance: missing active slots {missing}; unexpected {extra}")

        profile_proofs = validate_kanto_day_counterparts(row, day, by_label, standard_rod, location)
        for proof in profile_proofs:
            counterpart_proofs.append({"map": map_name, "method": method, **proof})

        # Route 21 uses North as its equivalent and requires the South source to
        # remain byte-for-byte identical for the selected method.
        if map_name == "MAP_ROUTE21_HNS":
            for version, suffix in (("FIRERED", "FireRed"), ("LEAFGREEN", "LeafGreen")):
                north = by_label.get(f"sRoute21North_{suffix}")
                south = by_label.get(f"sRoute21South_{suffix}")
                if north is None or south is None or north["encounter"].get(method) != south["encounter"].get(method):
                    raise ValidationError(f"{location}: Route 21 North and South {version} sources differ")
        manifest_profiles.append({**row, "provenance": normalized_provenance})

    if maps != KANTO_MAPS:
        raise ValidationError(f"{path}/regions/KANTO/profiles: Kanto map ownership mismatch; missing {sorted(KANTO_MAPS - maps)}; unexpected {sorted(maps - KANTO_MAPS)}")
    if len(manifest_profiles) != 129:
        raise ValidationError(f"{path}/regions/KANTO/profiles: expected exactly 129 profiles")
    method_counts = {method: sum(row["method"] == method for row in manifest_profiles) for method in ACTIVE_SLOT_COUNTS}
    if method_counts != {"land_mons": 41, "water_mons": 31, "rock_smash_mons": 25, "fishing_mons": 32}:
        raise ValidationError(f"{path}/regions/KANTO/profiles: profile method counts mismatch {method_counts}")
    topology_rates = []
    for row in manifest_profiles:
        for target_time in ("DAY", "NIGHT"):
            label = row["dayBaseLabel"] if target_time == "DAY" or row["nightMode"] == "DAY_ALIAS" else row["nightBaseLabel"]
            rate = by_label[label]["encounter"][row["method"]]["encounter_rate"]
            topology_rates.append((row["map"], row["method"], target_time, rate))
    topology_digest = hashlib.sha256(
        json.dumps(sorted(topology_rates), separators=(",", ":")).encode("ascii")
    ).hexdigest()
    if topology_digest != KANTO_TOPOLOGY_RATE_SHA256:
        raise ValidationError(f"{path}/regions/KANTO/profiles: frozen Kanto topology or encounter rates changed")
    route23_rates = {
        row["method"]: by_label[row["dayBaseLabel"]]["encounter"][row["method"]]["encounter_rate"]
        for row in manifest_profiles if row["map"] == "MAP_ROUTE23_HNS"
    }
    if route23_rates != {"land_mons": 21, "water_mons": 2, "fishing_mons": 20}:
        raise ValidationError(f"{path}/regions/KANTO/profiles: Route 23 rates must be land 21, Surf 2, and fishing 20")

    changes, change_identities = [], set()
    change_fields = {"map", "method", "time", "slot", "beforeSpecies", "afterSpecies", "changeKind", "reason"}
    manifest_by_identity = {(row["map"], row["method"]): row for row in manifest_profiles}
    for index, row in enumerate(_manifest_list(kanto["changes"], f"{path}/regions/KANTO/changes")):
        location = f"{path}/regions/KANTO/changes/{index}"
        exact_keys(row, change_fields, location)
        profile = manifest_by_identity.get((row["map"], row["method"]))
        if profile is None or row["time"] not in {"DAY", "NIGHT"}:
            raise ValidationError(f"{location}: unresolved target profile or time")
        slot = integer(row["slot"], f"{location}/slot", 0, profile["activeSlotCount"] - 1)
        change_identity = (row["map"], row["method"], row["time"], slot)
        if change_identity in change_identities:
            raise ValidationError(f"{location}: duplicate change target")
        change_identities.add(change_identity)
        for key in ("beforeSpecies", "afterSpecies"):
            species = identifier(row[key], f"{location}/{key}", SPECIES_IDENTIFIER)
            if known_species is not None and species not in known_species:
                raise ValidationError(f"{location}/{key}: unknown species")
        if row["changeKind"] not in KANTO_CHANGE_KINDS or not isinstance(row["reason"], str) or not row["reason"].strip():
            raise ValidationError(f"{location}: invalid change kind or reason")
        if row["beforeSpecies"] == row["afterSpecies"]:
            raise ValidationError(f"{location}: change must alter the species")
        target_label = profile["dayBaseLabel"] if row["time"] == "DAY" or profile["nightMode"] == "DAY_ALIAS" else profile["nightBaseLabel"]
        if _active_mons(by_label[target_label], profile["method"])[slot]["species"] != row["afterSpecies"]:
            raise ValidationError(f"{location}/afterSpecies: does not match wild_encounters.json")
        changes.append(row)

    changes_by_profile = {}
    for row in changes:
        changes_by_profile.setdefault((row["map"], row["method"], row["time"]), []).append(row)
    for (map_name, method, target_time), profile_changes in changes_by_profile.items():
        manifest_profile = manifest_by_identity[(map_name, method)]
        target_label = manifest_profile["dayBaseLabel"] if target_time == "DAY" or manifest_profile["nightMode"] == "DAY_ALIAS" else manifest_profile["nightBaseLabel"]
        final_mons = _active_mons(by_label[target_label], method)
        baseline_species = [mon["species"] for mon in final_mons]
        for change in profile_changes:
            baseline_species[change["slot"]] = change["beforeSpecies"]
        fire_mons = _active_mons(by_label[manifest_profile["fireRedSource"][0]], method)
        leaf_mons = _active_mons(by_label[manifest_profile["leafGreenSource"][0]], method)
        provenance_by_slot = {
            (record["targetTime"], record["targetSlot"]): record
            for record in manifest_profile["provenance"]
        }
        for change in profile_changes:
            if change["changeKind"] != "ADDITION" or map_name == "MAP_ROUTE23_HNS":
                continue
            location = f"{path}/regions/KANTO/changes/{changes.index(change)}"
            provenance = provenance_by_slot[(target_time, change["slot"])]
            if provenance["reason"] not in {"GEN2_LOCAL_ADDITION", "LATER_FAMILY_CONTINUITY"}:
                raise ValidationError(f"{location}: addition lacks addition provenance")
            ecology = provenance["ecologySourceGroup"]
            source_pairs = {
                (fire_mons[fire_slot]["species"], leaf_mons[leaf_slot]["species"])
                for fire_slot, leaf_slot in zip(ecology["fireRedSlots"], ecology["leafGreenSlots"])
            }
            if len(source_pairs) != 1 or next(iter(source_pairs))[0] != next(iter(source_pairs))[1]:
                raise ValidationError(f"{location}: addition must occupy a shared FRLG ecology group")
            shared_species = next(iter(source_pairs))[0]
            same_group_slots = [
                other["targetSlot"] for other in manifest_profile["provenance"]
                if other["targetTime"] == target_time and other["ecologySourceGroup"] == ecology
            ]
            if not any(slot != change["slot"] and final_mons[slot]["species"] == shared_species for slot in same_group_slots):
                raise ValidationError(f"{location}: addition removes the last shared FRLG species occurrence")
            if nat_dex_by_species is not None:
                national_dex = nat_dex_by_species.get(change["afterSpecies"])
                if national_dex is None or national_dex <= 151 or 252 <= national_dex <= 386:
                    raise ValidationError(f"{location}: addition must be a Generation II family or later-family continuity")
    baseline_ledger = sorted(
        (row["map"], row["method"], row["time"], row["slot"], row["beforeSpecies"])
        for row in changes
    )
    baseline_digest = hashlib.sha256(
        json.dumps(baseline_ledger, separators=(",", ":")).encode("ascii")
    ).hexdigest()
    if baseline_digest != KANTO_BASELINE_LEDGER_SHA256:
        raise ValidationError(f"{path}/regions/KANTO/changes: beforeSpecies ledger does not match the frozen pre-redesign baseline")
    changes.sort(key=lambda row: (row["map"], row["method"], row["time"], row["slot"], row["afterSpecies"]))
    manifest_profiles.sort(key=lambda row: (row["map"], row["method"]))
    day_aliases = [
        {"map": row["map"], "method": row["method"], "dayBaseLabel": row["dayBaseLabel"], "nightBaseLabel": row["nightBaseLabel"]}
        for row in manifest_profiles if row["nightMode"] == "DAY_ALIAS"
    ]
    return {"schemaVersion": 1, "product": "POKEMON_HNS", "profiles": manifest_profiles, "changes": changes, "dayAliases": day_aliases, "counterpartProofs": counterpart_proofs}


def load_regional_manifest(path, profiles, config, known_species=None):
    nat_dex = None if known_species is None else active_national_dex(DEFAULT_SPECIES_INFO)
    return validate_regional_manifest(load_json(path), profiles, config, known_species, path, nat_dex)


def profiles_with_day_aliases(profiles, regional_manifest, config):
    result = list(profiles)
    by_label = {profile["label"]: profile for profile in profiles}
    aliases = {}
    for item in regional_manifest["profiles"]:
        if item["nightMode"] != "DAY_ALIAS":
            continue
        day = by_label[item["dayBaseLabel"]]
        alias = aliases.get(item["nightBaseLabel"])
        if alias is None:
            alias = dict(day)
            alias["label"] = item["nightBaseLabel"]
            alias["time"], alias["header"] = time_and_header(alias["label"], config)
            alias["encounter"] = {"base_label": alias["label"], "map": day["map"]}
            aliases[alias["label"]] = alias
        alias["encounter"][item["method"]] = day["encounter"][item["method"]]
    return result + [aliases[label] for label in sorted(aliases)]


def validate_standard_rod_accessibility(standard_rod, profiles, known_species, config, path=DEFAULT_STANDARD_ROD_FISHING):
    profiles_by_label = {profile["label"]: profile for profile in profiles}
    old_weights = standard_rod["qualityWeights"]["OLD_ROD"]
    for index, row in enumerate(standard_rod["nativeSurfAccessibility"]):
        location = f"{path}/nativeSurfAccessibility/{index}"
        profile = profiles_by_label.get(row["baseLabel"])
        if profile is None:
            raise ValidationError(f"{location}/baseLabel: unknown profile")
        if profile["product"] != row["product"]:
            raise ValidationError(f"{location}/product: does not match profile")
        if row["timeOfDay"] not in config.times or profile["time"] != row["timeOfDay"]:
            raise ValidationError(f"{location}/timeOfDay: does not match resolved runtime time")
        if row["species"] not in known_species:
            raise ValidationError(f"{location}/species: unknown species")
        fishing = profile["encounter"].get("fishing_mons")
        if fishing is None or len(fishing.get("mons", [])) < FISHING_SLOT_COUNT:
            raise ValidationError(f"{location}/baseLabel: profile has no complete fishing table")
        species_weight = sum(
            old_weights[slot]
            for slot, mon in enumerate(fishing["mons"][:FISHING_SLOT_COUNT])
            if mon["species"] == row["species"]
        )
        if species_weight == 0:
            raise ValidationError(f"{location}/species: species is not authored in the fishing profile")
        if species_weight != row["expectedOldRodSuccessfulEncounterPercent"]:
            raise ValidationError(
                f"{location}/expectedOldRodSuccessfulEncounterPercent: expected {species_weight} from the Old Rod profile"
            )


def build_species_metadata(document, evolutions, known_species, ordinary_species, nat_dex_by_species=None):
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
    nat_dex_by_id = {}
    if nat_dex_by_species is not None:
        for name, nat_dex in nat_dex_by_species.items():
            if name in known_species:
                nat_dex_by_id.setdefault(known_species[name], nat_dex)
    for species in sorted(reachable, key=known_species.__getitem__):
        predecessor, level = predecessors.get(species, ("SPECIES_NONE", 0))
        alternate = any(row["method"] != "EVO_LEVEL" and row["target"] == species for row in evolutions.get(predecessor, []))
        national_dex = None if nat_dex_by_species is None else nat_dex_by_species.get(species, nat_dex_by_id.get(known_species[species]))
        if nat_dex_by_species is not None and national_dex is None:
            raise ValidationError(f"species_info/{species}: no active National Dex metadata")
        metadata.append({"species": species, "species_id": known_species[species], "national_dex": national_dex, "minimum_level": floors.get(species, 1), "predecessor": predecessor, "predecessor_id": known_species.get(predecessor, 0), "predecessor_level": level, "has_alternate_non_level_route": alternate})
    return metadata


def load_species_metadata(path, species_info_path, known_species, ordinary_species):
    return build_species_metadata(
        load_json(path), active_evolutions(species_info_path), known_species,
        ordinary_species, active_national_dex(species_info_path),
    )


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
    def __init__(self, output, data, config, regional_manifest=None):
        self.output, self.data, self.config = output, data, config
        self.regional_manifest = regional_manifest

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
            if group.get("for_maps", False) and self.regional_manifest is not None:
                for profile in self.regional_manifest["profiles"]:
                    if profile["nightMode"] != "DAY_ALIAS":
                        continue
                    day_label = profile["dayBaseLabel"]
                    _, header = time_and_header(day_label, self.config)
                    data = headers["data"].get(header)
                    if data is None:
                        raise ValidationError(f"{day_label}: DAY_ALIAS day header was not generated")
                    method = profile["method"]
                    day_info = day_label + "_" + method.title().replace("_", "") + "Info"
                    night_data = data.setdefault("TIME_NIGHT", {})
                    if method in night_data:
                        raise ValidationError(f"{profile['nightBaseLabel']}/{method}: DAY_ALIAS collides with authored night data")
                    night_data[method] = day_info
            self.write_headers(headers)


def render_scaling(output, scaling, offsets, metadata, standard_rod):
    output.write("\nconst u8 gStandardRodFishingWeights[WILD_ENCOUNTER_FISHING_ROD_NONE][FISH_WILD_COUNT] =\n{\n")
    for quality in FISHING_QUALITIES:
        weights = ", ".join(str(weight) for weight in standard_rod["qualityWeights"][quality])
        output.write(f"    [{RODS[quality]}] = {{ {weights} }},\n")
    output.write("};\n")
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


def render_header(encounters, config, scaling, offsets, metadata, standard_rod, regional_manifest=None):
    output = io.StringIO()
    output.write("//\n// DO NOT MODIFY THIS FILE! It is auto-generated by tools/wild_encounters/wild_encounters_to_header.py\n//\n\n\n")
    assembler = Assembler(output, encounters, config, regional_manifest)
    assembler.write_macros(); assembler.write_encounters(); render_scaling(output, scaling, offsets, metadata, standard_rod)
    return output.getvalue()


def project_level(scaling, vanilla, rating, offset):
    base, high_water = scaling["points"][0]["anchor_level"], 0
    for point in scaling["points"][:rating + 1]:
        raw = point["anchor_level"] + divide_round_signed((vanilla - base) * point["retention_numerator"], point["retention_denominator"])
        high_water = max(high_water, raw)
    return min(max(high_water + offset, 1), MAX_LEVEL)


def effective_species(species, level, by_species):
    result, changes = species, []
    while True:
        metadata = by_species[result]
        predecessor = metadata["predecessor"]
        if predecessor == "SPECIES_NONE" or level >= metadata["predecessor_level"]:
            return result, changes
        changes.append((result, predecessor)); result = predecessor


def stage_rank(species, by_species):
    rank = 0
    while by_species[species]["predecessor"] != "SPECIES_NONE":
        rank += 1; species = by_species[species]["predecessor"]
    return rank


def slot_summary(slot, scaling, offset, by_species, failures, location, exclude_species_none=False):
    summaries = [{"locked": False, "outcomes": {}, "outcomeCounts": {}, "changes": set()} for _ in range(scaling["projection_cap"] + 1)]
    for vanilla in range(slot["minimumLevel"], slot["maximumLevel"] + 1):
        previous_level, previous_rank = None, None
        for rating in range(scaling["projection_cap"] + 1):
            level = project_level(scaling, vanilla, rating, offset)
            species, changes = effective_species(slot["species"], level, by_species)
            outcome = summaries[rating]["outcomes"].setdefault(species, {"minimumLevel": level, "maximumLevel": level})
            outcome["minimumLevel"], outcome["maximumLevel"] = min(outcome["minimumLevel"], level), max(outcome["maximumLevel"], level)
            summaries[rating]["outcomeCounts"][species] = summaries[rating]["outcomeCounts"].get(species, 0) + 1
            summaries[rating]["changes"].update(changes)
            if (exclude_species_none and slot["species"] == "SPECIES_NONE") or level < by_species[species]["minimum_level"]:
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


def method_slots(profile, method, rod, standard_rod=None):
    field = next(field for field in profile["group"]["fields"] if field["type"] == method)
    # The existing engine owns selection counts through the field weight table.
    # Some HNS source entries carry inert trailing rows, so audit the same active
    # prefix rather than inventing weights for authored rows the engine cannot pick.
    if method == "fishing_mons":
        if standard_rod is None:
            standard_rod = load_standard_rod_fishing(DEFAULT_STANDARD_ROD_FISHING)
        indices = range(FISHING_SLOT_COUNT)
        weights = standard_rod["qualityWeights"][rod]
    else:
        indices = range(len(field["encounter_rates"]))
        weights = field["encounter_rates"]
    return [(index, profile["encounter"][method]["mons"][index], weights[index]) for index in indices]


def probability(numerator, denominator):
    value = Fraction(numerator, denominator)
    return {"numerator": value.numerator, "denominator": value.denominator}


def fraction_row(value):
    return probability(value.numerator, value.denominator)


def percentage_half_up(value):
    hundredths = round_half_up(value * 10000)
    return f"{hundredths // 100}.{hundredths % 100:02d}"


def generation_for(species, by_species, effective=False):
    if effective and species in {"SPECIES_WYNAUT", "SPECIES_AZURILL"}:
        return "GENERATION_II_FAMILY_EXTENSION"
    national_dex = by_species[species]["national_dex"]
    if national_dex is None:
        raise ValidationError(f"{species}: missing National Dex classification")
    if national_dex <= 151:
        return "GENERATION_I"
    if national_dex <= 251:
        return "GENERATION_II"
    if national_dex <= 386:
        return "INDEPENDENT_GENERATION_III"
    return "GENERATION_IV_ONWARD"


def is_forbidden_kanto_species(species, by_species, effective=False):
    if not effective and species in {"SPECIES_WYNAUT", "SPECIES_AZURILL"}:
        return True
    return generation_for(species, by_species, effective) == "INDEPENDENT_GENERATION_III"


def _profile_for_manifest_time(manifest_profile, time, profiles_by_label):
    label = manifest_profile["dayBaseLabel"]
    if time == "NIGHT" and manifest_profile["nightMode"] == "AUTHORED":
        label = manifest_profile["nightBaseLabel"]
    return profiles_by_label[label]


def kanto_profile_distribution(manifest_profile, time, rod, rating, profiles_by_label, scaling, offset_map, by_species, standard_rod):
    profile = _profile_for_manifest_time(manifest_profile, time, profiles_by_label)
    method = manifest_profile["method"]
    selected_rod = rod if method == "fishing_mons" else "NONE"
    slots = method_slots(profile, method, selected_rod, standard_rod)
    runtime_time = "TIME_NIGHT" if time == "NIGHT" else "TIME_DAY"
    runtime_key = (profile["product"], profile["header_id"], METHOD_AREAS[method], runtime_time, RODS[selected_rod])
    offset = offset_map.get(runtime_key, 0)
    prepared = []
    for slot_index, mon, weight in slots:
        authored_species = mon["species"]
        if authored_species == "SPECIES_NONE":
            continue
        outcomes = {}
        eligible = True
        minimum, maximum = mon.get("min_level", 2), mon.get("max_level", 100)
        for authored_level in range(min(minimum, maximum), max(minimum, maximum) + 1):
            if rating is None:
                resolved_species, projected_level = authored_species, authored_level
            else:
                projected_level = project_level(scaling, authored_level, rating, offset)
                resolved_species, _ = effective_species(authored_species, projected_level, by_species)
                if projected_level < by_species[resolved_species]["minimum_level"]:
                    eligible = False
            outcome = outcomes.setdefault(resolved_species, {"count": 0, "minimumLevel": projected_level, "maximumLevel": projected_level})
            outcome["count"] += 1
            outcome["minimumLevel"] = min(outcome["minimumLevel"], projected_level)
            outcome["maximumLevel"] = max(outcome["maximumLevel"], projected_level)
        if rating is None or eligible:
            prepared.append((slot_index, authored_species, weight, outcomes, sum(row["count"] for row in outcomes.values())))
    total_weight = sum(item[2] for item in prepared)
    aggregates, outcome_rows = {}, []
    if total_weight:
        for slot_index, authored_species, weight, outcomes, outcome_count in prepared:
            for resolved_species, outcome in sorted(outcomes.items()):
                value = Fraction(weight * outcome["count"], total_weight * outcome_count)
                aggregates[resolved_species] = aggregates.get(resolved_species, Fraction(0)) + value
                outcome_rows.append({
                    "targetSlot": slot_index,
                    "authoredSpecies": authored_species,
                    "resolvedSpecies": resolved_species,
                    "minimumProjectedLevel": outcome["minimumLevel"],
                    "maximumProjectedLevel": outcome["maximumLevel"],
                    "probability": fraction_row(value),
                })
    return {
        "map": manifest_profile["map"], "method": method, "time": time,
        "baseLabel": manifest_profile["nightBaseLabel"] if time == "NIGHT" else manifest_profile["dayBaseLabel"],
        "sourceBaseLabel": profile["label"], "runtimeTime": runtime_time,
        "nightMode": manifest_profile["nightMode"] if time == "NIGHT" else None,
        "fishingRod": selected_rod,
        "authoredAndEffectiveOutcomes": outcome_rows,
        "speciesProbabilities": [
            {"species": species, "probability": fraction_row(value)}
            for species, value in sorted(aggregates.items())
        ],
    }, aggregates


def _kanto_profile_slot_distributions(manifest_profile, time, rating, profiles_by_label, scaling, offset_map, by_species, standard_rod):
    """Return the eligible runtime slots and their exact outcome distributions."""
    profile = _profile_for_manifest_time(manifest_profile, time, profiles_by_label)
    method = manifest_profile["method"]
    runtime_time = "TIME_NIGHT" if time == "NIGHT" else "TIME_DAY"
    runtime_key = (profile["product"], profile["header_id"], METHOD_AREAS[method], runtime_time, RODS["NONE"])
    offset = offset_map.get(runtime_key, 0)
    result = []
    for slot_index, mon, weight in method_slots(profile, method, "NONE", standard_rod):
        authored_species = mon["species"]
        if authored_species == "SPECIES_NONE":
            continue
        outcomes = {}
        eligible = True
        minimum, maximum = mon.get("min_level", 2), mon.get("max_level", 100)
        levels = range(min(minimum, maximum), max(minimum, maximum) + 1)
        level_count = len(levels)
        for authored_level in levels:
            projected_level = project_level(scaling, authored_level, rating, offset)
            resolved_species, _ = effective_species(authored_species, projected_level, by_species)
            if projected_level < by_species[resolved_species]["minimum_level"]:
                eligible = False
            outcomes[resolved_species] = outcomes.get(resolved_species, 0) + 1
        if eligible:
            result.append({
                "slot": slot_index,
                "weight": weight,
                "authoredSpecies": authored_species,
                "outcomes": {
                    species: Fraction(count, level_count)
                    for species, count in outcomes.items()
                },
            })
    return result


def build_kanto_hoenn_sound_comparison(manifest, profiles_by_label, scaling, offset_map, by_species, standard_rod, failures):
    comparisons = []
    for manifest_profile in manifest["profiles"]:
        if manifest_profile["method"] not in {"land_mons", "water_mons"}:
            continue
        for time in ("DAY", "NIGHT"):
            for rating in range(10, min(80, scaling["projection_cap"]) + 1):
                slots = _kanto_profile_slot_distributions(
                    manifest_profile, time, rating, profiles_by_label, scaling,
                    offset_map, by_species, standard_rod,
                )
                total_weight = sum(slot["weight"] for slot in slots)
                off = {}
                if total_weight:
                    for slot in slots:
                        for species, within_slot in slot["outcomes"].items():
                            off[species] = off.get(species, Fraction(0)) + Fraction(slot["weight"], total_weight) * within_slot

                hoenn_slots = [
                    slot for slot in slots
                    if generation_for(slot["authoredSpecies"], by_species) == "INDEPENDENT_GENERATION_III"
                ]
                if not hoenn_slots or len(hoenn_slots) == len(slots):
                    on = dict(off)
                else:
                    sound = {}
                    for slot in hoenn_slots:
                        for species, within_slot in slot["outcomes"].items():
                            sound[species] = sound.get(species, Fraction(0)) + Fraction(1, len(hoenn_slots)) * within_slot
                    on = {
                        species: Fraction(9, 10) * off.get(species, 0) + Fraction(1, 10) * sound.get(species, 0)
                        for species in set(off) | set(sound)
                    }
                differences = [
                    {
                        "species": species,
                        "offProbability": fraction_row(off.get(species, Fraction(0))),
                        "onProbability": fraction_row(on.get(species, Fraction(0))),
                        "difference": fraction_row(on.get(species, Fraction(0)) - off.get(species, Fraction(0))),
                    }
                    for species in sorted(set(off) | set(on))
                    if off.get(species, Fraction(0)) != on.get(species, Fraction(0))
                ]
                passed = not differences
                if not passed:
                    failures.append(
                        f"{manifest_profile['map']}/{manifest_profile['method']}/{time}/rating {rating}: "
                        "Hoenn Sound changes the non-randomized ability-off lure-off distribution"
                    )
                comparisons.append({
                    "map": manifest_profile["map"],
                    "method": manifest_profile["method"],
                    "time": time,
                    "rating": rating,
                    "randomized": False,
                    "abilityAttraction": "OFF",
                    "lure": "OFF",
                    "offSpeciesProbabilities": [
                        {"species": species, "probability": fraction_row(value)}
                        for species, value in sorted(off.items())
                    ],
                    "onSpeciesProbabilities": [
                        {"species": species, "probability": fraction_row(value)}
                        for species, value in sorted(on.items())
                    ],
                    "differences": differences,
                    "passed": passed,
                })
    return {
        "statesCompared": ["OFF", "ON"],
        "ratings": list(range(10, min(80, scaling["projection_cap"]) + 1)),
        "profileComparisons": comparisons,
        "differences": [
            comparison
            for comparison in comparisons
            if comparison["differences"]
        ],
        "passed": all(comparison["passed"] for comparison in comparisons),
    }


def _generation_shares(distribution, by_species, effective):
    shares = {name: Fraction(0) for name in ("GENERATION_I", "GENERATION_II", "GENERATION_II_FAMILY_EXTENSION", "INDEPENDENT_GENERATION_III", "GENERATION_IV_ONWARD")}
    for species, value in distribution.items():
        shares[generation_for(species, by_species, effective)] += value
    shares["GENERATION_II_FAMILIES"] = shares["GENERATION_II"] + shares["GENERATION_II_FAMILY_EXTENSION"]
    return shares


def day_night_metrics(day, night):
    shared = set(day) & set(night)
    return {
        "dayShared": sum((day[species] for species in shared), Fraction(0)),
        "nightShared": sum((night[species] for species in shared), Fraction(0)),
        "distance": sum((abs(day.get(species, 0) - night.get(species, 0)) for species in set(day) | set(night)), Fraction(0)) / 2,
    }


def discrete_counterpart_candidates(target_slots, target_weights, species_pair, source_budget):
    candidates = []
    for mask in range(1, (1 << len(target_slots)) - 1):
        first_slots = tuple(target_slots[position] for position in range(len(target_slots)) if mask & (1 << position))
        second_slots = tuple(slot for slot in target_slots if slot not in first_slots)
        first = Fraction(sum(target_weights[slot] for slot in first_slots), sum(target_weights))
        second = Fraction(sum(target_weights[slot] for slot in second_slots), sum(target_weights))
        candidates.append((abs(first + second - source_budget), abs(first - second), first_slots, species_pair, max(first, second) / min(first, second)))
    return sorted(candidates, key=lambda item: item[:4])


_COUNTERPART_SOLVER_CACHE = {}


def solve_counterpart_assignment(weight_vectors, source_budgets, species_pair, target_slots=None):
    species = tuple(sorted(species_pair))
    qualities = tuple(sorted(weight_vectors))
    slot_count = len(weight_vectors[qualities[0]])
    if target_slots is None:
        target_slots = tuple(range(slot_count))
    else:
        target_slots = tuple(sorted(target_slots))
    structural_key = (
        tuple((quality, tuple(weight_vectors[quality]), source_budgets[quality]) for quality in qualities),
        target_slots,
    )
    structural = _COUNTERPART_SOLVER_CACHE.get(structural_key)
    if structural is None:
        candidates = []
        counts = {"fullStrict": 0, "fullBudget": 0, "reducedBudget": 0, "rejectedBudget": 0}
        for assignments in itertools.product((1, 2), repeat=len(target_slots)):
            states = [0] * slot_count
            for slot, state in zip(target_slots, assignments):
                states[slot] = state
            states = tuple(states)
            retained = len({state for state in states if state})
            if retained == 0:
                continue
            total_error, imbalance = Fraction(0), Fraction(0)
            within_budget, within_ratio = True, retained == 2
            metrics = []
            for quality in qualities:
                weights = weight_vectors[quality]
                total = sum(weights)
                first = Fraction(sum(weights[index] for index, state in enumerate(states) if state == 1), total)
                second = Fraction(sum(weights[index] for index, state in enumerate(states) if state == 2), total)
                budget = source_budgets[quality]
                error = abs(first + second - budget)
                tolerance = max(Fraction(1, 50), budget / 5)
                ratio = None if min(first, second) == 0 else max(first, second) / min(first, second)
                total_error += error; imbalance += abs(first - second)
                within_budget &= error <= tolerance
                within_ratio &= ratio is not None and ratio <= 2
                metrics.append((quality, first, second, error, tolerance, ratio))
            if within_budget and retained == 2 and within_ratio:
                category = "fullStrict"
            elif within_budget and retained == 2:
                category = "fullBudget"
            elif within_budget:
                category = "reducedBudget"
            else:
                category = "rejectedBudget"
            counts[category] += 1
            slot_sequence = tuple(index for index, state in enumerate(states) if state)
            # Species sequence is added after cache lookup; state 1 always maps
            # to the lexicographically smaller member.
            rank_prefix = (-retained, total_error, imbalance, slot_sequence)
            candidates.append((category, rank_prefix, states, metrics))
        candidate_digest = hashlib.sha256()
        for category, rank_prefix, states, metrics in candidates:
            candidate_digest.update(repr((category, rank_prefix, states, metrics)).encode("ascii"))
        structural = (counts, candidates, candidate_digest.hexdigest())
        _COUNTERPART_SOLVER_CACHE[structural_key] = structural
    counts, candidates, candidate_digest = structural
    preferred_category = next((category for category in ("fullStrict", "fullBudget", "reducedBudget") if counts[category]), None)
    if preferred_category is None:
        raise ValidationError("counterpart target slots cannot satisfy the combined probability budget")
    ranked = []
    for category, rank_prefix, states, metrics in candidates:
        if category != preferred_category:
            continue
        species_sequence = tuple(species[state - 1] for state in states if state)
        ranked.append((rank_prefix + (species_sequence,), states, metrics))
    ranked.sort(key=lambda item: item[0])
    rank, states, metrics = ranked[0]
    selected_states = tuple(states)
    enumerated = []
    for category, rank_prefix, candidate_states, candidate_metrics in candidates:
        retained = -rank_prefix[0]
        # The product contract requires every full-retention candidate, whether
        # valid or not, and every budget-valid reduced candidate.  Invalid
        # reduced candidates cannot participate in the reduced search.
        if retained != 2 and category != "reducedBudget":
            continue
        candidate_species_sequence = tuple(species[state - 1] for state in candidate_states if state)
        budget_pass = category in {"fullStrict", "fullBudget", "reducedBudget"}
        ratio_pass = category == "fullStrict" if retained == 2 else None
        if not budget_pass:
            fixed_result = "COMBINED_BUDGET_FAILURE"
        elif retained == 2 and not ratio_pass:
            fixed_result = "RATIO_ONLY_EXCEPTION_CANDIDATE"
        else:
            fixed_result = "PASS"
        selected = tuple(candidate_states) == selected_states and category == preferred_category
        if selected:
            decision = "SELECTED_BY_ORDERED_OBJECTIVES"
        elif category != preferred_category:
            decision = (
                "REJECTED_FULL_RETENTION_FIXED_CONSTRAINTS" if retained == 2
                else "REJECTED_BECAUSE_VALID_FULL_RETENTION_EXISTS"
            )
        else:
            decision = "REJECTED_BY_ORDERED_OBJECTIVES"
        enumerated.append({
            "candidateKind": "FULL_RETENTION" if retained == 2 else "REDUCED",
            "assignment": {
                species[index]: [slot for slot, state in enumerate(candidate_states) if state == index + 1]
                for index in range(2)
            },
            "fixedConstraintResult": fixed_result,
            "combinedBudgetSatisfied": budget_pass,
            "counterpartRatioSatisfied": ratio_pass,
            "objectives": {
                "distinctSourceSpeciesRetained": retained,
                "combinedBudgetError": fraction_row(rank_prefix[1]),
                "counterpartImbalance": fraction_row(rank_prefix[2]),
                "slotSequence": list(rank_prefix[3]),
                "speciesSequence": list(candidate_species_sequence),
            },
            "qualityMetrics": [
                {"fishingRod": quality, "firstProbability": fraction_row(first),
                 "secondProbability": fraction_row(second), "combinedBudgetError": fraction_row(error),
                 "budgetTolerance": fraction_row(tolerance),
                 "counterpartRatio": None if ratio is None else fraction_row(ratio)}
                for quality, first, second, error, tolerance, ratio in candidate_metrics
            ],
            "selectionReason": decision,
            "_states": tuple(candidate_states),
        })
    return {
        "species": species,
        "selectedCategory": preferred_category,
        "selectedStates": states,
        "selectedAssignment": {
            species[index]: [slot for slot, state in enumerate(states) if state == index + 1]
            for index in range(2)
        },
        "selectedRank": {
            "distinctSourceSpeciesRetained": -rank[0],
            "combinedBudgetError": fraction_row(rank[1]),
            "counterpartImbalance": fraction_row(rank[2]),
            "slotSequence": list(rank[3]), "speciesSequence": list(rank[4]),
        },
        "selectedQualityMetrics": [
            {"fishingRod": quality, "firstProbability": fraction_row(first),
             "secondProbability": fraction_row(second), "combinedBudgetError": fraction_row(error),
             "budgetTolerance": fraction_row(tolerance),
             "counterpartRatio": None if ratio is None else fraction_row(ratio)}
            for quality, first, second, error, tolerance, ratio in metrics
        ],
        "exhaustiveCandidateCounts": dict(counts),
        "exhaustiveCandidateDigest": candidate_digest,
        "enumeratedCandidates": enumerated,
        "_preferredStates": [item[1] for item in ranked],
    }


def _portfolio_row(time, rod, rating, manifest, profiles_by_label, scaling, offset_map, by_species, standard_rod, include_nonfishing_profiles=True):
    profiles, generations, by_method = [], {}, {}
    for item in manifest["profiles"]:
        report, distribution = kanto_profile_distribution(item, time, rod, rating, profiles_by_label, scaling, offset_map, by_species, standard_rod)
        if item["method"] == "fishing_mons" or include_nonfishing_profiles:
            profiles.append(report)
        shares = _generation_shares(distribution, by_species, rating is not None)
        for generation, value in shares.items():
            generations[generation] = generations.get(generation, Fraction(0)) + value
        method_shares = by_method.setdefault(item["method"], {})
        for generation, value in shares.items():
            method_shares[generation] = method_shares.get(generation, Fraction(0)) + value
    denominator = len(manifest["profiles"])
    generations = {key: value / denominator for key, value in generations.items()}
    diagnostics = {}
    for method, shares in by_method.items():
        count = sum(row["method"] == method for row in manifest["profiles"])
        diagnostics[method] = {key: fraction_row(value / count) for key, value in shares.items()}
    return {
        "time": time, "fishingRod": rod, "rating": rating,
        "profileDenominator": denominator,
        "generationProbabilities": {key: fraction_row(value) for key, value in generations.items()},
        "generationPercentages": {key: percentage_half_up(value) for key, value in generations.items()},
        "methodDiagnostics": diagnostics, "profiles": profiles,
    }, generations


def _source_weights(profile, method, rod, standard_rod):
    if method == "fishing_mons":
        return standard_rod["qualityWeights"][rod]
    field = next(field for field in profile["group"]["fields"] if field["type"] == method)
    return field["encounter_rates"][:ACTIVE_SLOT_COUNTS[method]]


def build_kanto_ecology_report(manifest, profiles_by_label, standard_rod, failures):
    report = []
    proofs_by_identity = {
        (proof["map"], proof["method"]): proof
        for proof in manifest["counterpartProofs"]
    }
    for profile in manifest["profiles"]:
        profile_proof = proofs_by_identity[(profile["map"], profile["method"])]
        proof_groups_by_slots = {
            tuple(group["sourceSlots"]): group
            for group in profile_proof.get("canonicalGroups", [])
        }
        provenance_groups = {}
        for record in profile["provenance"]:
            ecology = record["ecologySourceGroup"]
            key = (record["targetTime"], tuple(ecology["fireRedSlots"]), tuple(ecology["leafGreenSlots"]))
            provenance_groups.setdefault(key, []).append(record)
        for (time, fire_slots, leaf_slots), records in sorted(provenance_groups.items()):
            fire_profile = profiles_by_label[profile["fireRedSource"][0]]
            leaf_profile = profiles_by_label[profile["leafGreenSource"][0]]
            method = profile["method"]
            pairs = {
                ( _active_mons(fire_profile, method)[fire_slot]["species"], _active_mons(leaf_profile, method)[leaf_slot]["species"] )
                for fire_slot, leaf_slot in zip(fire_slots, leaf_slots)
            }
            if len(pairs) != 1:
                failures.append(f"{profile['map']}/{method}/{time}: ecology source group combines unlike FRLG species pairs")
            rods = FISHING_QUALITIES if method == "fishing_mons" else ("NONE",)
            rod_reports = []
            proof_group = proof_groups_by_slots.get(tuple(fire_slots)) if time == "DAY" else None
            if time == "DAY" and proof_group is None:
                failures.append(f"{profile['map']}/{method}/{time}: ecology source group is absent from counterpart proof")
            allocated_target_slots = (
                sorted(
                    assignment["targetSlot"]
                    for assignment in profile_proof["selectedAssignment"]
                    if assignment["state"] == "SOURCE"
                    and proof_group is not None
                    and assignment["canonicalGroup"] == proof_group["id"]
                )
                if time == "DAY" else sorted(record["targetSlot"] for record in records)
            )
            unassigned_target_slots = sorted(
                record["targetSlot"] for record in records
                if record["targetSlot"] not in allocated_target_slots
            )
            certified_omissions = (
                sorted(
                    omission["species"]
                    for omission in profile_proof["omittedCounterparts"]
                    if proof_group is not None
                    and omission["canonicalGroup"] == proof_group["id"]
                )
                if time == "DAY" else []
            )
            for rod in rods:
                fire_weights = _source_weights(fire_profile, method, rod, standard_rod)
                leaf_weights = _source_weights(leaf_profile, method, rod, standard_rod)
                source_budget = (Fraction(sum(fire_weights[slot] for slot in fire_slots), sum(fire_weights)) + Fraction(sum(leaf_weights[slot] for slot in leaf_slots), sum(leaf_weights))) / 2
                target_profile = _profile_for_manifest_time(profile, time, profiles_by_label)
                target_weights = _source_weights(target_profile, method, rod, standard_rod)
                target_slots = allocated_target_slots
                target_budget = Fraction(sum(target_weights[slot] for slot in target_slots), sum(target_weights))
                tolerance = max(Fraction(1, 50), source_budget / 5)
                pair = next(iter(pairs)) if pairs else ("SPECIES_NONE", "SPECIES_NONE")
                candidates, ratio = [], None
                ratio_exception_applied = False
                if pair[0] != pair[1]:
                    if time == "DAY" and abs(target_budget - source_budget) > tolerance:
                        failures.append(f"{profile['map']}/{method}/{time}/{rod}: counterpart ecology group budget outside tolerance")
                    values = []
                    for species in pair:
                        values.append(Fraction(sum(
                            target_weights[slot] for slot in target_slots
                            if _active_mons(target_profile, method)[slot]["species"] == species
                        ), sum(target_weights)))
                    if min(values) > 0:
                        ratio = max(values) / min(values)
                    candidates = discrete_counterpart_candidates(target_slots, target_weights, pair, source_budget)
                    retained_pair = min(values) > 0
                    ratio_exception_applied = (
                        time == "DAY"
                        and retained_pair
                        and ratio > 2
                        and profile_proof["selectedCategory"] == "fullBudget"
                    )
                    if time == "DAY" and retained_pair and ratio > 2 and not ratio_exception_applied:
                        failures.append(f"{profile['map']}/{method}/{time}/{rod}: retained counterpart ratio exceeds two-to-one")
                    if time == "DAY":
                        actual_omissions = sorted(
                            species for species, value in zip(pair, values) if value == 0
                        )
                        if actual_omissions != certified_omissions:
                            failures.append(
                                f"{profile['map']}/{method}/{time}/{rod}: certified counterpart omissions do not match selected allocation"
                            )
                rod_reports.append({
                    "fishingRod": rod, "sourceBudget": fraction_row(source_budget),
                    "targetBudget": fraction_row(target_budget), "budgetTolerance": fraction_row(tolerance),
                    "counterpartRatio": None if ratio is None else fraction_row(ratio),
                    "ratioExceptionApplied": ratio_exception_applied,
                    "discreteSlotCandidates": [
                        {"combinedBudgetError": fraction_row(candidate[0]), "counterpartDifference": fraction_row(candidate[1]),
                         "firstSpeciesSlots": list(candidate[2]), "speciesSequence": list(candidate[3]),
                         "counterpartRatio": fraction_row(candidate[4])}
                        for candidate in candidates
                    ],
                })
            report.append({
                "map": profile["map"], "method": method, "time": time,
                "sourceSpeciesPair": list(next(iter(pairs))) if len(pairs) == 1 else None,
                "fireRedSlots": list(fire_slots), "leafGreenSlots": list(leaf_slots),
                "targetSlots": allocated_target_slots,
                "unassignedTargetSlots": unassigned_target_slots,
                "selectedCategory": profile_proof["selectedCategory"] if time == "DAY" else None,
                "certifiedOmittedCounterparts": certified_omissions,
                "selectedSourceLevelRanges": [record["levelSource"] for record in sorted(records, key=lambda row: row["targetSlot"])],
                "rodReports": rod_reports,
            })
    return report


def build_kanto_audit(manifest, profiles, scaling, offsets, metadata, standard_rod, failures):
    profiles_by_label = {profile["label"]: profile for profile in profiles}
    by_species = {item["species"]: item for item in metadata}
    offset_map = {
        (item["product"], item["header_id"], item["area"], item["time"], item["rod"]): item["level_offset"]
        for item in offsets
    }
    authored_union, forbidden_authored, source_union_proofs = set(), [], []
    counterpart_proofs_by_identity = {}
    for proof in manifest["counterpartProofs"]:
        counterpart_proofs_by_identity.setdefault((proof["map"], proof["method"]), []).append(proof)
    for profile in manifest["profiles"]:
        method = profile["method"]
        source_union = {
            mon["species"]
            for key in ("fireRedSource", "leafGreenSource")
            for label in profile[key]
            for mon in _active_mons(profiles_by_label[label], method)
            if mon["species"] != "SPECIES_NONE"
        }
        day_species = {
            mon["species"] for mon in _active_mons(profiles_by_label[profile["dayBaseLabel"]], method)
            if mon["species"] != "SPECIES_NONE"
        }
        dropped = sorted(source_union - day_species)
        approved_omissions = {
            species for proof in counterpart_proofs_by_identity.get((profile["map"], method), [])
            for species in proof["omittedSourceSpecies"]
        }
        unproved_drops = sorted(set(dropped) - approved_omissions)
        if unproved_drops:
            failures.append(f"{profile['map']}/{method}: dropped FRLG source species lack exhaustive counterpart proof {unproved_drops}")
        source_capacity = profile["activeSlotCount"]
        source_union_over_capacity = len(source_union) > source_capacity
        source_union_proofs.append({
            "map": profile["map"], "method": method, "activeSlotCapacity": profile["activeSlotCount"],
            "sourceSlotCapacity": source_capacity,
            "sourceSpecies": sorted(source_union), "sourceSpeciesCount": len(source_union),
            "sourceUnionOverCapacity": source_union_over_capacity,
            "infeasibilityKind": None if not dropped else ("SOURCE_UNION_CAPACITY" if source_union_over_capacity else "DISCRETE_GROUP_ASSIGNMENT"),
            "selectedSourceSpeciesCount": len(source_union & day_species),
            "droppedSourceSpecies": dropped, "solverApprovedOmissions": sorted(approved_omissions),
        })
        for time in ("DAY", "NIGHT"):
            target = _profile_for_manifest_time(profile, time, profiles_by_label)
            for mon in _active_mons(target, profile["method"]):
                species = mon["species"]
                if species == "SPECIES_NONE":
                    continue
                authored_union.add(species)
                if is_forbidden_kanto_species(species, by_species):
                    forbidden_authored.append({"map": profile["map"], "method": profile["method"], "time": time, "species": species})
    if not 105 <= len(authored_union) <= 120:
        failures.append(f"Kanto authored species union has {len(authored_union)} species; expected 105 through 120")
    if forbidden_authored:
        failures.append("Kanto authored profiles contain forbidden species")

    retention = []
    materially_distinct_land = set()
    for profile in manifest["profiles"]:
        rods = FISHING_QUALITIES if profile["method"] == "fishing_mons" else ("NONE",)
        for rod in rods:
            _, day = kanto_profile_distribution(profile, "DAY", rod, None, profiles_by_label, scaling, offset_map, by_species, standard_rod)
            _, night = kanto_profile_distribution(profile, "NIGHT", rod, None, profiles_by_label, scaling, offset_map, by_species, standard_rod)
            metrics = day_night_metrics(day, night)
            day_retention, night_retention, distance = metrics["dayShared"], metrics["nightShared"], metrics["distance"]
            if day_retention < Fraction(7, 10) or night_retention < Fraction(7, 10):
                failures.append(f"{profile['map']}/{profile['method']}/{rod}: day/night shared-species retention below 70 percent")
            if profile["method"] == "land_mons" and profile["nightMode"] == "AUTHORED" and distance >= Fraction(1, 10):
                materially_distinct_land.add((profile["map"], profile["method"]))
            retention.append({
                "map": profile["map"], "method": profile["method"], "nightMode": profile["nightMode"],
                "fishingRod": rod, "daySharedProbability": fraction_row(day_retention),
                "nightSharedProbability": fraction_row(night_retention), "totalVariationDistance": fraction_row(distance),
            })
    if len(materially_distinct_land) < 25:
        failures.append(f"Kanto has {len(materially_distinct_land)} materially distinct land nights; expected at least 25")

    authored_portfolios = []
    for time in ("DAY", "NIGHT"):
        for rod in FISHING_QUALITIES:
            row, _ = _portfolio_row(time, rod, None, manifest, profiles_by_label, scaling, offset_map, by_species, standard_rod, rod == "OLD_ROD")
            authored_portfolios.append(row)

    effective_portfolios, forbidden_effective = [], []
    for rating in range(10, min(80, scaling["projection_cap"]) + 1):
        by_time_and_rod = {}
        for time in ("DAY", "NIGHT"):
            for rod in FISHING_QUALITIES:
                row, generations = _portfolio_row(time, rod, rating, manifest, profiles_by_label, scaling, offset_map, by_species, standard_rod, rod == "OLD_ROD")
                effective_portfolios.append(row)
                by_time_and_rod[(time, rod)] = generations
                gen1, gen2 = generations["GENERATION_I"], generations["GENERATION_II_FAMILIES"]
                gen3, gen4 = generations["INDEPENDENT_GENERATION_III"], generations["GENERATION_IV_ONWARD"]
                if time == "DAY":
                    valid = Fraction(3, 4) <= gen1 <= Fraction(17, 20) and Fraction(1, 10) <= gen2 <= Fraction(1, 5)
                else:
                    valid = Fraction(3, 5) <= gen1 <= Fraction(3, 4) and Fraction(1, 5) <= gen2 <= Fraction(7, 20)
                if not valid or gen3 != 0 or gen4 > Fraction(1, 20):
                    failures.append(f"Kanto {time}/{rod}/rating {rating}: generation portfolio outside required bands")
                if gen3:
                    forbidden_effective.append({"time": time, "fishingRod": rod, "rating": rating, "probability": fraction_row(gen3)})
        for rod in FISHING_QUALITIES:
            if by_time_and_rod[("NIGHT", rod)]["GENERATION_II_FAMILIES"] < by_time_and_rod[("DAY", rod)]["GENERATION_II_FAMILIES"] + Fraction(1, 20):
                failures.append(f"Kanto {rod}/rating {rating}: night Generation II is less than five points above day")

    opening_checks = []
    opening_maps = {"MAP_ROUTE1_HNS", "MAP_ROUTE2_HNS", "MAP_ROUTE3_HNS", "MAP_ROUTE22_HNS", "MAP_VIRIDIAN_FOREST_HNS", "MAP_MT_MOON_CAVE_HNS"}
    for profile in manifest["profiles"]:
        if profile["map"] not in opening_maps or profile["method"] != "land_mons":
            continue
        for time in ("DAY", "NIGHT"):
            target = _profile_for_manifest_time(profile, time, profiles_by_label)
            method = profile["method"]
            rod = "OLD_ROD" if method == "fishing_mons" else "NONE"
            runtime_key = (target["product"], target["header_id"], METHOD_AREAS[method], target["time"], RODS[rod])
            offset = offset_map.get(runtime_key, 0)
            highest = 0
            for mon in _active_mons(target, method):
                for authored_level in range(min(mon.get("min_level", 2), mon.get("max_level", 100)), max(mon.get("min_level", 2), mon.get("max_level", 100)) + 1):
                    highest = max(highest, project_level(scaling, authored_level, 10, offset))
            passed = highest <= 12
            if not passed:
                failures.append(f"{profile['map']}/{method}/{time}: Rating 10 opening level {highest} exceeds 12")
            opening_checks.append({"map": profile["map"], "method": method, "time": time, "highestEffectiveLevel": highest, "passed": passed})

    ecology = build_kanto_ecology_report(manifest, profiles_by_label, standard_rod, failures)
    hoenn_sound = build_kanto_hoenn_sound_comparison(
        manifest, profiles_by_label, scaling, offset_map, by_species,
        standard_rod, failures,
    )
    return {
        "ownership": {
            "maps": sorted(KANTO_MAPS), "mapCount": len(KANTO_MAPS),
            "profileDenominatorByTime": {"DAY": 129, "NIGHT": 129},
            "methodProfileCounts": {"land_mons": 41, "water_mons": 31, "rock_smash_mons": 25, "fishing_mons": 32},
            "profiles": manifest["profiles"],
        },
        "dayAliases": manifest["dayAliases"], "changes": manifest["changes"],
        "authoredSpeciesUnion": sorted(authored_union), "authoredSpeciesUnionCount": len(authored_union),
        "forbiddenSpecies": {"authored": forbidden_authored, "effective": forbidden_effective, "passed": not forbidden_authored and not forbidden_effective},
        "dayNightProfileMetrics": retention, "materiallyDistinctLandProfileCount": len(materially_distinct_land),
        "authoredPortfolios": authored_portfolios, "effectivePortfolios": effective_portfolios,
        "frlgEcologyGroups": ecology, "counterpartSolverProofs": manifest["counterpartProofs"],
        "sourceUnionCapacityProofs": source_union_proofs,
        "openingLevelChecks": opening_checks,
        "hoennSoundComparison": hoenn_sound,
    }


def audit_method(profile, method, rod, scaling, offset, by_species, failures, standard_rod):
    slots = []
    for index, mon, weight in method_slots(profile, method, rod, standard_rod):
        authored_minimum, authored_maximum = mon.get("min_level", 2), mon.get("max_level", 100)
        # Preserve the authored table verbatim in the generated header. The audit
        # uses its numeric envelope so legacy inverted ranges remain visible but
        # do not make the balance report impossible to produce.
        slot = {"species": mon["species"], "minimumLevel": min(authored_minimum, authored_maximum), "maximumLevel": max(authored_minimum, authored_maximum), "authoredMinimumLevel": authored_minimum, "authoredMaximumLevel": authored_maximum, "authoredRangeWasInverted": authored_minimum > authored_maximum}
        summaries, unlock = slot_summary(slot, scaling, offset, by_species, failures, f"{profile['product']}/{profile['label']}/{method}/{rod}/slot {index}", method == "fishing_mons")
        slots.append({"slot": index, "weight": weight, "original": slot, "summaries": summaries, "unlock": unlock})
    samples = []
    ratings = range(10, min(80, scaling["projection_cap"]) + 1) if method == "fishing_mons" else (value for value in SAMPLE_RATINGS if value <= scaling["projection_cap"])
    for rating in ratings:
        locked = [slot for slot in slots if slot["summaries"][rating]["locked"]]
        eligible = [slot for slot in slots if slot not in locked]
        if not eligible and method != "fishing_mons":
            failures.append(f"{profile['product']}/{profile['label']}/{method}/{rod}: all slots are locked at rating {rating}")
        total = sum(slot["weight"] for slot in eligible)
        outcomes = []
        for position, slot in enumerate(slots):
            summary = slot["summaries"][rating]
            if method == "fishing_mons":
                level_count = slot["original"]["maximumLevel"] - slot["original"]["minimumLevel"] + 1
                outcome = {
                    "slot": slot["slot"],
                    "authoredSpecies": slot["original"]["species"],
                    "weight": slot["weight"],
                    "eligible": not summary["locked"],
                    "effectiveSpeciesGivenSlotProbabilities": [
                        {"species": species, "probability": probability(count, level_count)}
                        for species, count in sorted(summary["outcomeCounts"].items())
                    ],
                }
                if not summary["locked"]:
                    eligible_position = eligible.index(slot)
                    mirrored = eligible[len(eligible) - eligible_position - 1]
                    outcome["mirroredSlot"] = mirrored["slot"]
                    outcome["lureOffSuccessfulEncounterProbability"] = probability(slot["weight"], total)
                    outcome["lureOnSuccessfulEncounterProbability"] = probability(4 * slot["weight"] + mirrored["weight"], 5 * total)
                    bite = FISHING_BASE_BITE_PERCENT[rod]
                    outcome["lureOffUnmodifiedCastProbability"] = probability(slot["weight"] * bite, total * 100)
                    outcome["lureOnUnmodifiedCastProbability"] = probability((4 * slot["weight"] + mirrored["weight"]) * bite, 5 * total * 100)
            else:
                outcome = {"slot": slot["slot"], "weight": slot["weight"], "locked": summary["locked"], "unlockRating": slot["unlock"], "effective": [{"species": species, **value} for species, value in sorted(summary["outcomes"].items())], "stageChanges": [{"fromSpecies": source, "toSpecies": target} for source, target in sorted(summary["changes"])], "renormalizedWeight": None if summary["locked"] else probability(slot["weight"], total)}
            outcomes.append(outcome)
        sample = {"rating": rating, "eligibleSlotCount": len(eligible), "lockedSlotCount": len(locked), "eligibleWeight": total, "lockedWeight": sum(slot["weight"] for slot in locked), "slotOutcomes": outcomes}
        if method == "fishing_mons":
            authored_aggregates = {}
            effective_aggregates = {}
            if total:
                for eligible_position, slot in enumerate(eligible):
                    mirrored = eligible[len(eligible) - eligible_position - 1]
                    species = slot["original"]["species"]
                    lure_off_slot = Fraction(slot["weight"], total)
                    lure_on_slot = Fraction(4 * slot["weight"] + mirrored["weight"], 5 * total)
                    authored = authored_aggregates.setdefault(species, {"lureOff": Fraction(0), "lureOn": Fraction(0)})
                    authored["lureOff"] += lure_off_slot
                    authored["lureOn"] += lure_on_slot
                    level_count = slot["original"]["maximumLevel"] - slot["original"]["minimumLevel"] + 1
                    for effective_species, count in slot["summaries"][rating]["outcomeCounts"].items():
                        conditional = Fraction(count, level_count)
                        effective = effective_aggregates.setdefault(effective_species, {"lureOff": Fraction(0), "lureOn": Fraction(0)})
                        effective["lureOff"] += lure_off_slot * conditional
                        effective["lureOn"] += lure_on_slot * conditional
            bite = FISHING_BASE_BITE_PERCENT[rod]
            def aggregate_rows(aggregates):
                return [
                {
                    "species": species,
                    "lureOffSuccessfulEncounterProbability": probability(values["lureOff"].numerator, values["lureOff"].denominator),
                    "lureOnSuccessfulEncounterProbability": probability(values["lureOn"].numerator, values["lureOn"].denominator),
                    "lureOffUnmodifiedCastProbability": probability((values["lureOff"] * Fraction(bite, 100)).numerator, (values["lureOff"] * Fraction(bite, 100)).denominator),
                    "lureOnUnmodifiedCastProbability": probability((values["lureOn"] * Fraction(bite, 100)).numerator, (values["lureOn"] * Fraction(bite, 100)).denominator),
                }
                for species, values in sorted(aggregates.items())
                ]
            sample["aggregateAuthoredSpeciesProbabilities"] = aggregate_rows(authored_aggregates)
            sample["aggregateSpeciesProbabilities"] = aggregate_rows(effective_aggregates)
        if method == "fishing_mons":
            sample["ratings"] = [sample.pop("rating")]
            if samples and {key: value for key, value in samples[-1].items() if key != "ratings"} == {key: value for key, value in sample.items() if key != "ratings"}:
                samples[-1]["ratings"].extend(sample["ratings"])
            else:
                samples.append(sample)
        else:
            samples.append(sample)
    row = {"label": profile["label"], "map": profile["map"], "header": profile["header"], "headerId": profile["header_id"], "timeOfDay": profile["time"], "method": method, "fishingRod": rod, "encounterRate": profile["encounter"][method]["encounter_rate"], "authoredSlotCount": len(profile["encounter"][method]["mons"]), "runtimeSlotCount": len(slots), "levelOffset": offset, "samples": samples}
    if method == "fishing_mons":
        row["weights"] = list(standard_rod["qualityWeights"][rod])
        row["baseBitePercent"] = FISHING_BASE_BITE_PERCENT[rod]
    return row


def validate_standard_rod_balance(standard_rod, profiles, scaling, metadata, offsets):
    """Validate release-blocking fishing invariants during ordinary generation."""
    failures = []
    for slot, weight in enumerate(standard_rod["qualityWeights"]["OLD_ROD"]):
        if Fraction(weight, 100) < Fraction(1, 200):
            failures.append(f"OLD_ROD/slot {slot}: below the 0.5 percent minimum when eligible")

    profiles_by_label = {profile["label"]: profile for profile in profiles}
    by_species = {item["species"]: item for item in metadata}
    offset_map = {
        (item["product"], item["header_id"], item["area"], item["time"], item["rod"]): item["level_offset"]
        for item in offsets
    }
    audited_profiles = {}
    for record in standard_rod["nativeSurfAccessibility"]:
        profile = profiles_by_label[record["baseLabel"]]
        row = audited_profiles.get(profile["label"])
        if row is None:
            key = (
                profile["product"], profile["header_id"], METHOD_AREAS["fishing_mons"],
                profile["time"], RODS["OLD_ROD"],
            )
            row = audit_method(
                profile, "fishing_mons", "OLD_ROD", scaling,
                offset_map.get(key, 0), by_species, failures, standard_rod,
            )
            audited_profiles[profile["label"]] = row
        for sample in row["samples"]:
            species_row = next(
                (item for item in sample["aggregateAuthoredSpeciesProbabilities"] if item["species"] == record["species"]),
                None,
            )
            success = Fraction(0) if species_row is None else Fraction(**species_row["lureOffSuccessfulEncounterProbability"])
            per_cast = Fraction(0) if species_row is None else Fraction(**species_row["lureOffUnmodifiedCastProbability"])
            expected = Fraction(record["expectedOldRodSuccessfulEncounterPercent"], 100)
            minimum_success = Fraction(record["minimumOldRodSuccessfulEncounterPercent"], 100)
            minimum_cast = Fraction(record["minimumOldRodUnmodifiedCastPercent"], 100)
            for rating in sample["ratings"]:
                identity = f"{record['product']}/{record['baseLabel']}/{record['timeOfDay']}/{record['species']}/rating {rating}"
                if success != expected:
                    failures.append(f"{identity}: expected {expected}, got {success}")
                if success < minimum_success:
                    failures.append(f"{identity}: below successful-encounter accessibility minimum")
                if per_cast < minimum_cast:
                    failures.append(f"{identity}: below unmodified-cast accessibility minimum")
    if failures:
        raise ValidationError("standard rod balance invariant failures: " + "; ".join(failures))


def species_projection_intervals(species, by_species):
    intervals = []
    for level in range(1, MAX_LEVEL + 1):
        effective, _ = effective_species(species, level, by_species)
        floor = by_species[effective]["minimum_level"]
        outcome = (effective, level >= floor, floor)
        if intervals and intervals[-1]["_outcome"] == outcome:
            intervals[-1]["maximumProjectedLevel"] = level
            continue
        intervals.append({
            "minimumProjectedLevel": level,
            "maximumProjectedLevel": level,
            "effectiveSpecies": effective,
            "eligible": outcome[1],
            "minimumOrdinaryWildLevel": floor,
            "_outcome": outcome,
        })
    for interval in intervals:
        del interval["_outcome"]
    return intervals


def build_cartographer_projection_model(profiles, header_ids, config, scaling, metadata, offsets, minimum_rating, maximum_rating, standard_rod=None):
    if standard_rod is None:
        standard_rod = load_standard_rod_fishing(DEFAULT_STANDARD_ROD_FISHING)
    if not 0 <= minimum_rating <= maximum_rating <= scaling["projection_cap"]:
        raise ValidationError("Cartographer Trainer Rating bounds exceed the projection curve")
    by_species = {item["species"]: item for item in metadata}
    offset_map = {
        (item["product"], item["header_id"], item["area"], item["time"], item["rod"]): item["level_offset"]
        for item in offsets
    }
    configured_offsets = sorted({0, *(item["level_offset"] for item in offsets)})
    level_projections = []
    for offset in configured_offsets:
        ratings = []
        for rating in range(minimum_rating, maximum_rating + 1):
            ratings.append({
                "rating": rating,
                "projectedLevels": [
                    project_level(scaling, authored_level, rating, offset)
                    for authored_level in range(1, MAX_LEVEL + 1)
                ],
            })
        level_projections.append({"levelOffset": offset, "ratings": ratings})

    species = []
    for item in metadata:
        species.append({
            "authoredSpecies": item["species"],
            "authoredSpeciesId": item["species_id"],
            "outcomesByProjectedLevel": species_projection_intervals(item["species"], by_species),
        })

    profiles_by_identity, runtime_identities = {}, set()
    for profile in profiles:
        for method in config.mon_types:
            if method not in profile["encounter"]:
                continue
            rods = ("OLD_ROD", "GOOD_ROD", "SUPER_ROD") if method == "fishing_mons" else ("NONE",)
            for rod in rods:
                profile_key = f"{profile['product']}/{profile['label']}/{method}/{rod}"
                runtime_identity = (
                    profile["product"], profile["header_id"], METHOD_AREAS[method],
                    profile["time"], RODS[rod],
                )
                if profile_key in profiles_by_identity:
                    raise ValidationError(f"duplicate Cartographer profile key {profile_key}")
                if runtime_identity in runtime_identities:
                    raise ValidationError(f"duplicate Cartographer runtime identity {runtime_identity}")
                runtime_identities.add(runtime_identity)
                slots = method_slots(profile, method, rod, standard_rod)
                profile_row = {
                    "profileKey": profile_key,
                    "product": profile["product"],
                    "map": profile["map"],
                    "baseLabel": profile["label"],
                    "header": profile["header"],
                    "headerId": profile["header_id"],
                    "runtimeTime": profile["time"],
                    "method": method,
                    "runtimeArea": METHOD_AREAS[method],
                    "fishingRod": rod,
                    "runtimeFishingRod": RODS[rod],
                    "levelOffset": offset_map.get(runtime_identity, 0),
                    "encounterRate": profile["encounter"][method]["encounter_rate"],
                    "authoredSlotCount": len(profile["encounter"][method]["mons"]),
                    "runtimeSlotCount": len(slots),
                }
                if method == "fishing_mons":
                    profile_row["weights"] = list(standard_rod["qualityWeights"][rod])
                profiles_by_identity[profile_key] = profile_row

    product_order = {product: index for index, (product, _) in enumerate(PRODUCTS)}
    method_order = {method: index for index, method in enumerate(config.mon_types)}
    rod_order = {rod: index for index, rod in enumerate(RODS)}
    ordered_profiles = sorted(
        profiles_by_identity.values(),
        key=lambda row: (
            product_order[row["product"]], row["headerId"], row["runtimeTime"],
            method_order[row["method"]], rod_order[row["fishingRod"]], row["baseLabel"],
        ),
    )
    return {
        "schemaVersion": 2,
        "trainerRating": {"minimum": minimum_rating, "maximum": maximum_rating},
        "authoredLevel": {"minimum": 1, "maximum": MAX_LEVEL},
        "products": [{"id": product, "displayName": display} for product, display in PRODUCTS],
        "levelProjections": level_projections,
        "species": species,
        "profiles": ordered_profiles,
        "headerCounts": {product: len(header_ids[product]) for product, _ in PRODUCTS},
    }


def build_cartographer_projection(encounters_path=DEFAULT_ENCOUNTERS, scaling_path=DEFAULT_SCALING, standard_rod_fishing_path=DEFAULT_STANDARD_ROD_FISHING, regions_path=DEFAULT_REGIONS, config_path=DEFAULT_CONFIG, rtc_constants_path=DEFAULT_RTC, species_path=DEFAULT_SPECIES, wild_encounter_species_path=DEFAULT_SPECIES_METADATA, species_info_path=DEFAULT_SPECIES_INFO, trainer_rating_path=DEFAULT_TRAINER_RATING):
    encounters = load_json(encounters_path)
    config = Config(config_path, rtc_constants_path, encounters)
    scaling = load_scaling(scaling_path)
    known_species = species_ids(species_path)
    profiles, header_ids = validate_encounters(encounters, known_species, config)
    standard_rod = load_standard_rod_fishing(standard_rod_fishing_path)
    validate_standard_rod_accessibility(standard_rod, profiles, known_species, config, standard_rod_fishing_path)
    ordinary_species = {
        mon["species"]
        for profile in profiles
        for method in config.mon_types
        for mon in profile["encounter"].get(method, {}).get("mons", [])
    }
    metadata = load_species_metadata(wild_encounter_species_path, species_info_path, known_species, ordinary_species)
    offsets = load_offsets(scaling["profile_offsets"], profiles, scaling_path)
    # Cartographer joins projection rows one-to-one with authored rows in
    # wild_encounters.json.  Validate DAY_ALIAS bindings as part of the regional
    # manifest, but materialize them only for generated C and balance-audit
    # runtime identities.
    load_regional_manifest(regions_path, profiles, config, known_species)
    minimum_rating, maximum_rating = trainer_rating_bounds(trainer_rating_path, scaling["projection_cap"])
    return build_cartographer_projection_model(
        profiles, header_ids, config, scaling, metadata, offsets, minimum_rating, maximum_rating, standard_rod,
    )


def build_wild_encounter_balance_audit(encounters_path=DEFAULT_ENCOUNTERS, scaling_path=DEFAULT_SCALING, standard_rod_fishing_path=DEFAULT_STANDARD_ROD_FISHING, regions_path=DEFAULT_REGIONS, config_path=DEFAULT_CONFIG, rtc_constants_path=DEFAULT_RTC, species_path=DEFAULT_SPECIES, wild_encounter_species_path=DEFAULT_SPECIES_METADATA, species_info_path=DEFAULT_SPECIES_INFO):
    encounters = load_json(encounters_path); config = Config(config_path, rtc_constants_path, encounters); scaling = load_scaling(scaling_path); known_species = species_ids(species_path)
    profiles, header_ids = validate_encounters(encounters, known_species, config)
    standard_rod = load_standard_rod_fishing(standard_rod_fishing_path)
    validate_standard_rod_accessibility(standard_rod, profiles, known_species, config, standard_rod_fishing_path)
    ordinary_species = {mon["species"] for profile in profiles for method in config.mon_types for mon in profile["encounter"].get(method, {}).get("mons", [])}
    metadata = load_species_metadata(wild_encounter_species_path, species_info_path, known_species, ordinary_species)
    by_species = {item["species"]: item for item in metadata}
    offsets = load_offsets(scaling["profile_offsets"], profiles, scaling_path)
    offset_map = {(item["product"], item["header_id"], item["area"], item["time"], item["rod"]): item["level_offset"] for item in offsets}
    failures, products = [], []
    for slot, weight in enumerate(standard_rod["qualityWeights"]["OLD_ROD"]):
        if Fraction(weight, 100) < Fraction(1, 200):
            failures.append(f"OLD_ROD/slot {slot}: below the 0.5 percent minimum when eligible")
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
                    population.append(audit_method(profile, method, rod, scaling, offset_map.get(key, 0), by_species, failures, standard_rod))
        products.append({"product": display, "headerCount": len(header_ids[product]), "profileCount": len(selected), "population": population})

    population_by_identity = {
        (product, row["label"], row["timeOfDay"], row["fishingRod"]): row
        for (product, _), product_row in zip(PRODUCTS, products)
        for row in product_row["population"]
        if row["method"] == "fishing_mons"
    }
    accessibility = []
    for record in standard_rod["nativeSurfAccessibility"]:
        row = population_by_identity[(record["product"], record["baseLabel"], record["timeOfDay"], "OLD_ROD")]
        rating_results = []
        for sample in row["samples"]:
            species_row = next((item for item in sample["aggregateAuthoredSpeciesProbabilities"] if item["species"] == record["species"]), None)
            success = Fraction(0) if species_row is None else Fraction(**species_row["lureOffSuccessfulEncounterProbability"])
            per_cast = Fraction(0) if species_row is None else Fraction(**species_row["lureOffUnmodifiedCastProbability"])
            expected = Fraction(record["expectedOldRodSuccessfulEncounterPercent"], 100)
            minimum_success = Fraction(record["minimumOldRodSuccessfulEncounterPercent"], 100)
            minimum_cast = Fraction(record["minimumOldRodUnmodifiedCastPercent"], 100)
            for rating in sample["ratings"]:
                identity = f"{record['product']}/{record['baseLabel']}/{record['timeOfDay']}/{record['species']}/rating {rating}"
                if success != expected:
                    failures.append(f"{identity}: expected {expected}, got {success}")
                if success < minimum_success:
                    failures.append(f"{identity}: below successful-encounter accessibility minimum")
                if per_cast < minimum_cast:
                    failures.append(f"{identity}: below unmodified-cast accessibility minimum")
                rating_results.append({
                    "rating": rating,
                    "successfulEncounterProbability": probability(success.numerator, success.denominator),
                    "unmodifiedCastProbability": probability(per_cast.numerator, per_cast.denominator),
                    "expectedUnmodifiedCasts": None if per_cast == 0 else probability(per_cast.denominator, per_cast.numerator),
                })
        accessibility.append({**record, "ratings": rating_results})

    regional_manifest = load_regional_manifest(regions_path, profiles, config, known_species)
    kanto = build_kanto_audit(regional_manifest, profiles, scaling, offsets, metadata, standard_rod, failures)
    return {"schemaVersion": 3, "sampleRatings": [rating for rating in SAMPLE_RATINGS if rating <= scaling["projection_cap"]], "exhaustiveFishingRatings": list(range(10, min(80, scaling["projection_cap"]) + 1)), "qualityWeights": standard_rod["qualityWeights"], "minimumEligibleOldRodEntryProbability": probability(min(standard_rod["qualityWeights"]["OLD_ROD"]), 100), "projection": {"cap": scaling["projection_cap"], "anchors": scaling["anchors"], "retention": [{"numerator": point["retention_numerator"], "denominator": point["retention_denominator"]} for point in scaling["points"]]}, "products": products, "nativeSurfAccessibility": accessibility, "regions": {"KANTO": kanto}, "invariants": {"passed": not failures, "failures": failures}}


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


def generate(encounters_path=DEFAULT_ENCOUNTERS, scaling_path=DEFAULT_SCALING, standard_rod_fishing_path=DEFAULT_STANDARD_ROD_FISHING, regions_path=DEFAULT_REGIONS, output_path=DEFAULT_OUTPUT, config_path=DEFAULT_CONFIG, rtc_constants_path=DEFAULT_RTC, species_path=DEFAULT_SPECIES, wild_encounter_species_path=DEFAULT_SPECIES_METADATA, species_info_path=DEFAULT_SPECIES_INFO):
    encounters = load_json(encounters_path); config = Config(config_path, rtc_constants_path, encounters); scaling = load_scaling(scaling_path); known_species = species_ids(species_path)
    profiles, _ = validate_encounters(encounters, known_species, config)
    standard_rod = load_standard_rod_fishing(standard_rod_fishing_path)
    validate_standard_rod_accessibility(standard_rod, profiles, known_species, config, standard_rod_fishing_path)
    ordinary_species = {mon["species"] for profile in profiles for method in config.mon_types for mon in profile["encounter"].get(method, {}).get("mons", [])}
    metadata = load_species_metadata(wild_encounter_species_path, species_info_path, known_species, ordinary_species)
    offsets = load_offsets(scaling["profile_offsets"], profiles, scaling_path)
    validate_standard_rod_balance(standard_rod, profiles, scaling, metadata, offsets)
    regional_manifest = load_regional_manifest(regions_path, profiles, config, known_species)
    atomic_write(output_path, render_header(encounters, config, scaling, offsets, metadata, standard_rod, regional_manifest))


def generate_wild_encounter_balance_audit(output_path=DEFAULT_AUDIT, **kwargs):
    audit = build_wild_encounter_balance_audit(**kwargs)
    if audit["invariants"]["failures"]:
        raise ValidationError("wild encounter balance audit invariant failures: " + "; ".join(audit["invariants"]["failures"]))
    atomic_write(output_path, json.dumps(audit, ensure_ascii=True, separators=(",", ":"), sort_keys=True) + "\n")
    return audit


def generate_cartographer_projection(output_path, **kwargs):
    projection = build_cartographer_projection(**kwargs)
    atomic_write(
        output_path,
        json.dumps(projection, ensure_ascii=True, separators=(",", ":"), sort_keys=True) + "\n",
    )
    return projection


def arguments():
    parser = argparse.ArgumentParser(description="Generate wild encounter scaling data")
    parser.add_argument("--encounters", type=Path, default=DEFAULT_ENCOUNTERS); parser.add_argument("--scaling", type=Path, default=DEFAULT_SCALING); parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--standard-rod-fishing", type=Path, default=DEFAULT_STANDARD_ROD_FISHING)
    parser.add_argument("--regions", type=Path, default=DEFAULT_REGIONS)
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG); parser.add_argument("--rtc-constants", type=Path, default=DEFAULT_RTC); parser.add_argument("--species", type=Path, default=DEFAULT_SPECIES)
    parser.add_argument("--wild-encounter-species", type=Path, default=DEFAULT_SPECIES_METADATA); parser.add_argument("--species-info", type=Path, default=DEFAULT_SPECIES_INFO)
    parser.add_argument("--trainer-rating", type=Path, default=DEFAULT_TRAINER_RATING)
    outputs = parser.add_mutually_exclusive_group()
    outputs.add_argument("--balance-audit", type=Path, nargs="?", const=DEFAULT_AUDIT)
    outputs.add_argument(
        "--cartographer-projection",
        type=Path,
        metavar="PATH",
        help="write the schema-versioned Trainer Rating projection model used by the devtools Cartographer",
    )
    return parser.parse_args()


def main():
    args = arguments()
    common = {"encounters_path": args.encounters, "scaling_path": args.scaling, "standard_rod_fishing_path": args.standard_rod_fishing, "regions_path": args.regions, "config_path": args.config, "rtc_constants_path": args.rtc_constants, "species_path": args.species, "wild_encounter_species_path": args.wild_encounter_species, "species_info_path": args.species_info}
    try:
        if args.cartographer_projection is not None:
            projection = generate_cartographer_projection(
                args.cartographer_projection,
                trainer_rating_path=args.trainer_rating,
                **common,
            )
            print(
                f"Cartographer wild encounter projection generated: {args.cartographer_projection} "
                f"({len(projection['profiles'])} profiles)"
            )
        elif args.balance_audit is None:
            generate(output_path=args.output, **common)
        else:
            audit = generate_wild_encounter_balance_audit(args.balance_audit, **common)
            count = sum(len(product["population"]) for product in audit["products"])
            print(f"wild encounter balance audit passed: {args.balance_audit} ({count} method rows)")
    except ValidationError as error:
        raise SystemExit(f"wild encounter generation failed: {error}") from error


if __name__ == "__main__":
    main()
