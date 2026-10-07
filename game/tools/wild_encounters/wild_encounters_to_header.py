#!/usr/bin/env python3
"""Generate src/data/wild_encounters.h.

Wayfarer (IS_WAYFARER) gets only the wild encounters v2 headers, built from src/data/wild_encounters_v2/
(see v2/v2_emit.py): gWildMonHeaders[], gWildEncounterPlaces[] and the prowler minimums. The authored
Emerald, FireRed/LeafGreen and HNS tables in wild_encounters.json compile out under IS_WAYFARER and are
kept only for the standalone builds. The module also keeps the species and evolution helpers the
Trainer scaling tools import.
"""

import argparse
import io
import json
import os
from pathlib import Path
import re
import stat
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent / "v2"))
import v2_emit  # noqa: E402

DEFAULT_ENCOUNTERS = ROOT / "src/data/wild_encounters.json"
DEFAULT_SPECIES_METADATA = ROOT / "src/data/wild_encounter_species.json"
DEFAULT_STANDARD_ROD_FISHING = ROOT / "src/data/standard_rod_fishing.json"
DEFAULT_OUTPUT = ROOT / "src/data/wild_encounters.h"
DEFAULT_CONFIG = ROOT / "include/config/overworld.h"
DEFAULT_RTC = ROOT / "include/constants/rtc.h"
DEFAULT_SPECIES = ROOT / "include/constants/species.h"
DEFAULT_POKEDEX = ROOT / "include/constants/pokedex.h"
DEFAULT_SPECIES_INFO = ROOT / "src/data/pokemon/species_info.h"
DEFAULT_SPECIES_CONFIG = ROOT / "include/config/pokemon.h"

MAX_LEVEL = 100
MAX_U16 = 0xFFFF
IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
SPECIES_IDENTIFIER = re.compile(r"^SPECIES_[A-Z0-9_]+$")
RODS = {
    "NONE": "WILD_ENCOUNTER_FISHING_ROD_NONE",
    "OLD_ROD": "WILD_ENCOUNTER_FISHING_ROD_OLD",
    "GOOD_ROD": "WILD_ENCOUNTER_FISHING_ROD_GOOD",
    "SUPER_ROD": "WILD_ENCOUNTER_FISHING_ROD_SUPER",
}
PRODUCTS = (("EMERALD", "Emerald"), ("FIRERED", "FireRed"),
            ("LEAFGREEN", "LeafGreen"), ("POKEMON_HNS", "HNS"))
PRODUCT_GUARDS = {
    "EMERALD": "HAS_EMERALD_CONTENT",
    "FIRERED": "defined(FIRERED) && HAS_FRLG_CONTENT",
    "LEAFGREEN": "defined(LEAFGREEN) && HAS_FRLG_CONTENT",
    "POKEMON_HNS": "HAS_HNS_CONTENT",
}
FISHING_QUALITIES = ("OLD_ROD", "GOOD_ROD", "SUPER_ROD")
FISHING_SLOT_COUNT = 10
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


def load_standard_rod_fishing(path):
    source = load_json(path)
    exact_keys(source, {"schemaVersion", "qualityWeights"}, path)
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

    return source


def product_for(label):
    if "FireRed" in label:
        return "FIRERED"
    if "LeafGreen" in label:
        return "LEAFGREEN"
    if "_Hns" in label or "_hns" in label:
        return "POKEMON_HNS"
    return "EMERALD"


def product_guard(product):
    return PRODUCT_GUARDS[product]


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


def build_numeric_predecessors(resolution_rows, evolutions, known_species):
    """Validate the shared numeric graph without applying any encounter policy."""
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
            if (not 1 <= level <= MAX_LEVEL or predecessor not in known_species
                    or evolution["target"] not in known_species or evolution["target"] not in evolutions):
                raise ValidationError(f"species_info/{predecessor}: malformed numeric evolution")
            candidates.setdefault(evolution["target"], set()).add((predecessor, level))

    resolutions = {}
    rows = resolution_rows
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

    return predecessors


def build_trainer_species_metadata(document, evolutions, known_species, trainer_species):
    """Cover Trainer species and their exact predecessors, with no wild floors."""
    exact_keys(document, {"schemaVersion", "predecessorResolutions"}, "wild_encounter_species.json")
    if document["schemaVersion"] != 1 or isinstance(document["schemaVersion"], bool):
        raise ValidationError("wild_encounter_species.json/schemaVersion: expected 1")
    predecessors = build_numeric_predecessors(document["predecessorResolutions"], evolutions, known_species)
    active_by_id = {}
    for species in evolutions:
        if species in known_species:
            active_by_id.setdefault(known_species[species], []).append(species)
    trainer_species = set(trainer_species)
    for species in sorted(trainer_species):
        active = active_by_id.get(known_species.get(species), [])
        if not active or known_species[species] == 0:
            raise ValidationError(f"trainer_species/{species}: unknown or inactive species")
        if species not in evolutions:
            if len(active) != 1:
                raise ValidationError(f"trainer_species/{species}: ambiguous active numeric alias")
            if active[0] in predecessors:
                predecessors[species] = predecessors[active[0]]
    reachable = set()
    for species in sorted(trainer_species):
        current = species
        while current not in reachable:
            reachable.add(current)
            if current not in predecessors:
                break
            current = predecessors[current][0]
    metadata = []
    for species in sorted(reachable, key=lambda name: (known_species[name], name)):
        predecessor, level = predecessors.get(species, ("SPECIES_NONE", 0))
        metadata.append({
            "species": species,
            "species_id": known_species[species],
            "predecessor": predecessor,
            "predecessor_id": known_species.get(predecessor, 0),
            "predecessor_level": level,
        })
    return metadata


def load_trainer_species_metadata(path, species_info_path, known_species, trainer_species):
    return build_trainer_species_metadata(
        load_json(path), active_evolutions(species_info_path), known_species, trainer_species,
    )


def render_trainer_predecessor_header(metadata):
    output = io.StringIO()
    output.write("// Generated by the Trainer inventory. Do not edit.\n")
    output.write("static const struct TrainerScalingPredecessor sTrainerScalingPredecessors[] =\n{\n")
    emitted = set()
    for row in sorted(metadata, key=lambda row: (row["species_id"], row["species"])):
        if row["predecessor_id"] and row["species_id"] not in emitted:
            output.write(f"    {{ {row['species']}, {row['predecessor']}, {row['predecessor_level']} }},\n")
            emitted.add(row["species_id"])
    if not emitted:
        output.write("    { SPECIES_NONE, SPECIES_NONE, 0 },\n")
    output.write("};\n")
    return output.getvalue()


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


class Assembler:
    """Writes the macros, the mon arrays and the header tables of wild_encounters.h."""

    def __init__(self, output, data, config, v2):
        self.output, self.data, self.config = output, data, config
        self.v2_arrays, self.v2_headers, self.v2_trailer, _ = v2

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

    def write_headers(self, headers, map_backed):
        self.line(f"const struct WildPokemonHeader {headers['label']}[] ="); self.line("{")
        if map_backed:
            self.line("#if IS_WAYFARER")
            self.output.write(self.v2_headers)
            self.line("#else")
        for product, _ in PRODUCTS:
            for label, data in headers["data"].items():
                if product_for(label) != product:
                    continue
                self.line(); self.line(f"#if {product_guard(product)}"); self.line("{", 1)
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
        if map_backed:
            self.line("#endif")
        self.write_terminator(); self.line("};")

    def write_encounters(self):
        for group in self.data["wild_encounter_groups"]:
            map_backed = group.get("for_maps", False)
            headers, counter = {"label": group["label"], "data": {}}, 1
            if map_backed:
                self.line("#if IS_WAYFARER")
                self.output.write(self.v2_arrays)
                self.line("#else  // The authored Emerald, FireRed/LeafGreen and HNS tables are for the standalone builds.")
            for encounter in group["encounters"]:
                map_group, map_num = "0", str(counter)
                if map_backed:
                    map_group, map_num = f"MAP_GROUP({encounter['map']})", f"MAP_NUM({encounter['map']})"
                counter += 1
                time, header = time_and_header(encounter["base_label"], self.config)
                data = headers["data"].setdefault(header, {"mapGroup": map_group, "mapNum": map_num, "map": encounter.get("map")})
                if data["mapGroup"] != map_group or data["mapNum"] != map_num:
                    raise ValidationError(f"{encounter['base_label']}: shared header spans maps")
                time_data = data.setdefault(time, {})
                self.line(f"#if {product_guard(product_for(header))}")
                for method in self.config.mon_types:
                    if method not in encounter:
                        continue
                    name = encounter["base_label"] + "_" + method.title().replace("_", "")
                    self.write_mons(name, encounter[method])
                    if method in time_data:
                        raise ValidationError(f"{encounter['base_label']}/{method}: duplicate time data")
                    time_data[method] = name + "Info"
                self.line("#endif")
            if map_backed:
                self.line("#endif")
            self.write_headers(headers, map_backed)
        self.output.write(self.v2_trailer)


def render_rod_weights(output, standard_rod):
    output.write("\nconst u8 gStandardRodFishingWeights[WILD_ENCOUNTER_FISHING_ROD_NONE][FISH_WILD_COUNT] =\n{\n")
    for quality in FISHING_QUALITIES:
        weights = ", ".join(str(weight) for weight in standard_rod["qualityWeights"][quality])
        output.write(f"    [{RODS[quality]}] = {{ {weights} }},\n")
    output.write("};\n")


def render_header(encounters, config, standard_rod, v2):
    output = io.StringIO()
    output.write("//\n// DO NOT MODIFY THIS FILE! It is auto-generated by tools/wild_encounters/wild_encounters_to_header.py\n//\n\n\n")
    assembler = Assembler(output, encounters, config, v2)
    assembler.write_macros(); assembler.write_encounters(); render_rod_weights(output, standard_rod)
    return output.getvalue()


def generate(encounters_path=DEFAULT_ENCOUNTERS, standard_rod_fishing_path=DEFAULT_STANDARD_ROD_FISHING, output_path=DEFAULT_OUTPUT,
             config_path=DEFAULT_CONFIG, rtc_constants_path=DEFAULT_RTC, species_path=DEFAULT_SPECIES, v2_data_dir=v2_emit.DATA_DIR):
    encounters = load_json(encounters_path); config = Config(config_path, rtc_constants_path, encounters)
    known_species = species_ids(species_path)
    validate_encounters(encounters, known_species, config)
    standard_rod = load_standard_rod_fishing(standard_rod_fishing_path)
    try:
        v2 = v2_emit.render(known_species, data_dir=v2_data_dir)
    except v2_emit.V2Error as error:
        raise ValidationError(f"wild encounters v2: {error}") from error
    atomic_write(output_path, render_header(encounters, config, standard_rod, v2))
    return v2[3]


def arguments():
    parser = argparse.ArgumentParser(description="Generate wild encounter headers")
    parser.add_argument("--encounters", type=Path, default=DEFAULT_ENCOUNTERS); parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--standard-rod-fishing", type=Path, default=DEFAULT_STANDARD_ROD_FISHING)
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG); parser.add_argument("--rtc-constants", type=Path, default=DEFAULT_RTC)
    parser.add_argument("--species", type=Path, default=DEFAULT_SPECIES)
    parser.add_argument("--v2-data", type=Path, default=v2_emit.DATA_DIR)
    return parser.parse_args()


def main():
    args = arguments()
    try:
        summary = generate(args.encounters, args.standard_rod_fishing, args.output, args.config, args.rtc_constants, args.species, args.v2_data)
    except ValidationError as error:
        raise SystemExit(f"wild encounter generation failed: {error}") from error
    print(f"wild encounters v2: {summary['headers']} headers, {summary['prowlers']} prowler species, "
          f"{len(summary['defaulted'])} method tables on a default encounter rate")


if __name__ == "__main__":
    main()
