#!/usr/bin/env python3
"""Generate and audit the Wayfarer Hoenn source-content manifest."""

from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import os
import re
import tempfile
from collections import Counter, defaultdict
from pathlib import Path


TOOL_DIR = Path(__file__).resolve().parent
GAME_ROOT = TOOL_DIR.parents[1]
DEFAULT_POLICY = TOOL_DIR / "classification.json"
DEFAULT_OUTPUT = GAME_ROOT / "build/wayfarer-hoenn-content-manifest.json"

EVENT_KEYS = ("object_events", "warp_events", "coord_events", "bg_events")
METHOD_KEYS = ("land_mons", "water_mons", "rock_smash_mons", "fishing_mons")
METHOD_NAMES = {
    "land_mons": ("land",),
    "water_mons": ("water",),
    "rock_smash_mons": ("rockSmash",),
    "fishing_mons": ("oldRod", "goodRod", "superRod"),
}
GENERATED_METHOD_SUFFIXES = {
    "land": "_LandMonsInfo",
    "water": "_WaterMonsInfo",
    "rockSmash": "_RockSmashMonsInfo",
    "oldRod": "_FishingMonsInfo",
    "goodRod": "_FishingMonsInfo",
    "superRod": "_FishingMonsInfo",
}
PERSISTENT_RE = re.compile(r"\b(?:FLAG|VAR)_[A-Za-z0-9_]+\b")
TRAINER_RE = re.compile(r"\bTRAINER_[A-Z0-9_]+\b")
SCRIPT_LABEL_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)::?\s*$", re.MULTILINE)
SCRIPT_TOKEN_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
SCRIPT_INCLUDE_RE = re.compile(r'^\s*\.include\s+"(data/[^"]+\.(?:inc|s))"', re.MULTILINE)
DEFINE_RE = re.compile(r"^\s*#define\s+([A-Za-z_][A-Za-z0-9_]*)\s+(.+?)\s*$", re.MULTILINE)
MAPPED_DEFINE_RE = re.compile(r"^\s*#define\s+((?:FLAG|VAR)_[A-Za-z0-9_]+)\s+(0x[0-9A-Fa-f]+)\s*$", re.MULTILINE)
TRAINER_SOURCE_RE = re.compile(
    r"^\s*#define\s+(TRAINER_[A-Z0-9_]+)\s+TRAINER_EMERALD_ID\((\d+)\)\s*$",
    re.MULTILINE,
)
WAYFARER_CPP_SYMBOLS = {
    "IS_WAYFARER": 1,
    "IS_HNS": 1,
    "IS_EMERALD": 0,
    "IS_FRLG": 0,
    "IS_FIRERED": 0,
    "IS_LEAFGREEN": 0,
    "MODERN": 1,
    "POKEMON_WAYFARER": 1,
    "POKEMON_HNS": 0,
    "POKEMON_EMERALD": 0,
    "FIRERED": 0,
    "LEAFGREEN": 0,
}
PARTY_RE = re.compile(r"^===\s+(TRAINER_[A-Z0-9_]+)\s+===$", re.MULTILINE)
FLY_RE = re.compile(
    r"\{\s*REGION_MAP_HOENN\s*,\s*([^,]+),\s*(?:WAYFARER_HOENN_)?(MAPSEC_[A-Z0-9_]+)\s*\}"
)


class AuditError(ValueError):
    pass


def read_json(path: Path):
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise AuditError(f"cannot read {path}: {error}") from error


def strip_asm_comments(source: str) -> str:
    return "\n".join(line.split("@", 1)[0] for line in source.splitlines())


def evaluate_wayfarer_condition(condition: str) -> bool:
    expression = re.sub(
        r"defined\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)",
        lambda match: "1" if WAYFARER_CPP_SYMBOLS.get(match.group(1), 0) else "0",
        condition,
    )
    expression = re.sub(
        r"\b[A-Za-z_][A-Za-z0-9_]*\b",
        lambda match: str(WAYFARER_CPP_SYMBOLS.get(match.group(0), 0)),
        expression,
    )
    expression = expression.replace("&&", " and ").replace("||", " or ")
    expression = re.sub(r"!(?!=)", " not ", expression)
    if re.fullmatch(r"[0-9\s()<>!=.+\-*/%andornot]+", expression) is None:
        raise AuditError(f"unsupported preprocessor condition: {condition}")
    return bool(eval(expression, {"__builtins__": {}}, {}))


def filter_wayfarer_source(source: str) -> str:
    output = []
    # Each frame stores parent activity, whether a prior branch matched, and
    # whether the current branch is active.
    stack = [(True, False, True)]
    for line in source.splitlines():
        directive = re.match(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b\s*(.*)$", line)
        if directive is None:
            if stack[-1][2]:
                output.append(line)
            continue
        command, argument = directive.groups()
        if command in {"if", "ifdef", "ifndef"}:
            parent_active = stack[-1][2]
            if command == "if":
                matched = evaluate_wayfarer_condition(argument)
            else:
                defined = bool(WAYFARER_CPP_SYMBOLS.get(argument.strip(), 0))
                matched = defined if command == "ifdef" else not defined
            stack.append((parent_active, matched, parent_active and matched))
        elif command == "elif":
            if len(stack) == 1:
                raise AuditError("unmatched #elif in event script")
            parent_active, prior_matched, _ = stack[-1]
            matched = not prior_matched and evaluate_wayfarer_condition(argument)
            stack[-1] = (parent_active, prior_matched or matched, parent_active and matched)
        elif command == "else":
            if len(stack) == 1:
                raise AuditError("unmatched #else in event script")
            parent_active, prior_matched, _ = stack[-1]
            matched = not prior_matched
            stack[-1] = (parent_active, True, parent_active and matched)
        else:
            if len(stack) == 1:
                raise AuditError("unmatched #endif in event script")
            stack.pop()
    if len(stack) != 1:
        raise AuditError("unterminated preprocessor condition in event script")
    return "\n".join(output)


def assembler_condition_value(condition: str) -> bool | None:
    """Evaluate build-selector expressions, leaving unrelated config guards intact."""
    identifiers = set(re.findall(r"\b[A-Za-z_][A-Za-z0-9_]*\b", condition))
    if identifiers and not identifiers.issubset(WAYFARER_CPP_SYMBOLS):
        return None
    return evaluate_wayfarer_condition(condition)


def filter_wayfarer_assembler_source(source: str) -> str:
    """Apply known .if build guards while conservatively retaining unknown guards."""
    output = []
    # Each frame stores parent activity, whether a prior branch matched, current
    # activity, and whether this tool understands the condition.
    stack = [(True, False, True, True)]
    for line in source.splitlines():
        directive = re.match(
            r"^\s*\.(if|ifdef|ifndef|elseif|else|endif)\b\s*(.*)$", line
        )
        if directive is None:
            if stack[-1][2]:
                output.append(line)
            continue
        command, argument = directive.groups()
        if command in {"if", "ifdef", "ifndef"}:
            parent_active = stack[-1][2]
            if command == "if":
                value = assembler_condition_value(argument)
            else:
                symbol = argument.strip()
                value = (bool(WAYFARER_CPP_SYMBOLS[symbol])
                         if symbol in WAYFARER_CPP_SYMBOLS else None)
                if command == "ifndef" and value is not None:
                    value = not value
            if value is None:
                stack.append((parent_active, False, parent_active, False))
            else:
                stack.append((parent_active, value, parent_active and value, True))
        elif command == "elseif":
            if len(stack) == 1:
                raise AuditError("unmatched .elseif in event script")
            parent_active, prior_matched, _, understood = stack[-1]
            value = assembler_condition_value(argument)
            if not understood or value is None:
                stack[-1] = (parent_active, prior_matched, parent_active, False)
            else:
                matched = not prior_matched and value
                stack[-1] = (parent_active, prior_matched or matched,
                             parent_active and matched, True)
        elif command == "else":
            if len(stack) == 1:
                raise AuditError("unmatched .else in event script")
            parent_active, prior_matched, _, understood = stack[-1]
            if understood:
                stack[-1] = (parent_active, True,
                             parent_active and not prior_matched, True)
            else:
                stack[-1] = (parent_active, prior_matched, parent_active, False)
        else:
            if len(stack) == 1:
                raise AuditError("unmatched .endif in event script")
            stack.pop()
    if len(stack) != 1:
        raise AuditError("unterminated assembler condition in event script")
    return "\n".join(output)


def filter_event_script_source(source: str) -> str:
    source = strip_asm_comments(source)
    return filter_wayfarer_assembler_source(filter_wayfarer_source(source))


def fingerprint(rows) -> str:
    payload = "".join(f"{row}\n" for row in rows)
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def canonical_hash(value) -> str:
    payload = json.dumps(value, sort_keys=True, separators=(",", ":"))
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def file_hash(path: Path) -> str | None:
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
            stream.write(content)
        os.replace(temporary, path)
    except BaseException:
        Path(temporary).unlink(missing_ok=True)
        raise


def classify(name: str, policy: dict) -> dict:
    for rule in policy.get("rules", []):
        if any(fnmatch.fnmatchcase(name, pattern) for pattern in rule.get("globs", [])):
            return {key: value for key, value in rule.items() if key != "globs"}
    return dict(policy["default"])


def parse_mapped_constants(path: Path) -> dict[str, int]:
    if not path.exists():
        return {}
    return {name: int(value, 16) for name, value in MAPPED_DEFINE_RE.findall(path.read_text(encoding="utf-8"))}


def selected_catalog(game_root: Path):
    maps_root = game_root / "data/maps"
    groups = read_json(maps_root / "map_groups.json")
    rows = []
    all_selected_ids = set()
    all_ids = {}
    for group_number, group_name in enumerate(groups["group_order"]):
        for map_number, map_name in enumerate(groups.get(group_name, [])):
            path = maps_root / map_name / "map.json"
            data = read_json(path)
            all_ids[data["id"]] = data.get("game_version", "emerald")
            if data.get("game_version", "emerald") in {"hns", "emerald"}:
                all_selected_ids.add(data["id"])
            if data.get("game_version", "emerald") == "emerald":
                rows.append((group_number, group_name, map_number, map_name, path, data))
    return rows, all_selected_ids, all_ids


def load_layouts(game_root: Path):
    layouts = {}
    for number, source in enumerate(
        read_json(game_root / "data/layouts/layouts.json")["layouts"], 1
    ):
        layout = dict(source)
        layout["catalog_number"] = number
        layouts[layout["id"]] = layout
    return layouts


def load_heals(game_root: Path):
    rows = read_json(game_root / "src/data/heal_locations.json")["heal_locations"]
    return [row for row in rows if row.get("source", "EMERALD").upper() == "EMERALD"]


def load_wild_methods(game_root: Path):
    methods = defaultdict(set)
    profiles = []
    source = read_json(game_root / "src/data/wild_encounters.json")
    for group in source["wild_encounter_groups"]:
        if not group.get("for_maps"):
            continue
        for encounter in group.get("encounters", []):
            profile_methods = set()
            for key in METHOD_KEYS:
                if encounter.get(key) is not None:
                    profile_methods.update(METHOD_NAMES[key])
            methods[encounter["map"]].update(profile_methods)
            profiles.append({"map": encounter["map"], "label": encounter.get("base_label"),
                             "methods": sorted(profile_methods),
                             "authoredSha256": hashlib.sha256(json.dumps(
                                 encounter, sort_keys=True, separators=(",", ":")
                             ).encode("utf-8")).hexdigest()})
    return methods, profiles


def event_script_source_paths(game_root: Path):
    """Return the authored source closure compiled by data/event_scripts.s."""
    entry = game_root / "data/event_scripts.s"
    pending = [entry]
    visited = set()
    while pending:
        path = pending.pop()
        if path in visited or not path.is_file():
            continue
        visited.add(path)
        source = filter_event_script_source(path.read_text(encoding="utf-8"))
        pending.extend(game_root / relative for relative in SCRIPT_INCLUDE_RE.findall(source))
    return sorted(visited)


def script_symbols(game_root: Path):
    symbols = set()
    symbol_paths = {}
    symbol_blocks = {}
    for path in event_script_source_paths(game_root):
        try:
            source = filter_event_script_source(path.read_text(encoding="utf-8"))
            matches = list(SCRIPT_LABEL_RE.finditer(source))
            found = [match.group(1) for match in matches]
            symbols.update(found)
            for index, match in enumerate(matches):
                symbol = match.group(1)
                symbol_paths.setdefault(symbol, path)
                end = matches[index + 1].start() if index + 1 < len(matches) else len(source)
                symbol_blocks.setdefault(symbol, (
                    path,
                    source[match.start():end],
                    source.count("\n", 0, match.start()) + 1,
                ))
        except UnicodeDecodeError:
            continue
    return symbols, symbol_paths, symbol_blocks


def reachable_script_blocks(references: set[str], symbols: set[str], symbol_blocks: dict):
    """Return all script-label blocks reachable from the supplied entry labels."""
    pending = list(sorted(references & symbols))
    visited = set()
    blocks = []
    while pending:
        symbol = pending.pop()
        if symbol in visited:
            continue
        visited.add(symbol)
        block = symbol_blocks.get(symbol)
        if block is None:
            continue
        blocks.append((symbol, *block))
        nested = referenced_script_symbols(block[1], symbols)
        pending.extend(sorted(nested - visited))
    return blocks


def referenced_script_symbols(source: str, symbols: set[str]):
    references = set()
    for line in strip_asm_comments(source).splitlines():
        stripped = line.strip()
        if (not stripped
                or stripped.startswith((".string", "#"))
                or SCRIPT_LABEL_RE.fullmatch(stripped)):
            continue
        references.update(set(SCRIPT_TOKEN_RE.findall(stripped)) & symbols)
    return references


def trainer_registry(game_root: Path):
    constant_source = "\n".join(
        path.read_text(encoding="utf-8")
        for path in sorted((game_root / "include/constants").glob("opponents*.h"))
    )
    known = {name for name, _ in DEFINE_RE.findall(constant_source) if name.startswith("TRAINER_")}
    source_ids = {name: int(value) for name, value in TRAINER_SOURCE_RE.findall(constant_source)}
    offset_match = re.search(r"#define\s+WAYFARER_HOENN_TRAINER_OFFSET\s+(\d+)", constant_source)
    if offset_match is None:
        raise AuditError("WAYFARER_HOENN_TRAINER_OFFSET is missing")
    offset = int(offset_match.group(1))
    party_source = (game_root / "src/data/trainers.party").read_text(encoding="utf-8")
    headings = list(PARTY_RE.finditer(party_source))
    parties = {}
    for index, heading in enumerate(headings):
        end = headings[index + 1].start() if index + 1 < len(headings) else len(party_source)
        parties[heading.group(1)] = party_source[heading.start():end].strip()
    return known, source_ids, offset, parties


def referenced_trainers(source: str, known: set[str]):
    trainers = set()
    for line in strip_asm_comments(source).splitlines():
        command = line.strip().split(None, 1)
        if not command:
            continue
        if not (
            command[0].startswith("trainerbattle")
            or command[0] in {"goto_if_defeated", "goto_if_not_defeated", "register_matchcall"}
        ):
            continue
        trainers.update(token for token in TRAINER_RE.findall(line) if token in known)
    trainers.discard("TRAINER_NONE")
    return trainers


def event_script_references(data: dict):
    references = set()
    for key in ("object_events", "coord_events", "bg_events"):
        for event in data.get(key) or []:
            script = event.get("script")
            if isinstance(script, str) and script not in {"0", "0x0", "NULL"}:
                references.add(script)
    return references


def effective_owner(game_root: Path, map_name: str, data: dict, field: str):
    owner_name = data.get(field, map_name)
    owner_path = game_root / "data/maps" / owner_name / "map.json"
    if not owner_path.is_file():
        raise AuditError(f"{map_name} {field} references missing map {owner_name}")
    owner_data = read_json(owner_path)
    expected_source = data.get("game_version", "emerald")
    if owner_data.get("game_version", "emerald") != expected_source:
        raise AuditError(f"{map_name} {field} crosses source boundary at {owner_name}")
    if owner_data.get(field) and owner_data[field] != owner_name:
        raise AuditError(f"{map_name} {field} ownership must not be chained")
    return owner_name, owner_path, owner_data


def load_fly_sections(game_root: Path):
    source = (game_root / "src/region_map.c").read_text(encoding="utf-8")
    return {section: flag.strip() for flag, section in FLY_RE.findall(source)}


def build_manifest(game_root: Path, policy_path: Path, hoenn_constants: Path, engine_constants: Path):
    policy = read_json(policy_path)
    if policy.get("schemaVersion") != 1 or policy.get("source") != "emerald":
        raise AuditError("classification policy must be schemaVersion 1 for the emerald source")

    catalog, selected_ids, all_ids = selected_catalog(game_root)
    layouts = load_layouts(game_root)
    heals = load_heals(game_root)
    wild_methods, all_wild_profiles = load_wild_methods(game_root)
    fly_sections = load_fly_sections(game_root)
    symbols, symbol_paths, symbol_blocks = script_symbols(game_root)
    known_trainers, trainer_source_ids, trainer_offset, trainer_parties = trainer_registry(game_root)
    hoenn_values = parse_mapped_constants(hoenn_constants)
    engine_values = parse_mapped_constants(engine_constants)
    tileset_declarations = "\n".join(
        path.read_text(encoding="utf-8")
        for path in (game_root / "include/tilesets.h", game_root / "src/data/tilesets/headers.h")
    )
    direct_persistent_names = set()
    for path in (game_root / "include/constants/flags.h", game_root / "include/constants/vars.h"):
        direct_persistent_names.update(name for name, _ in DEFINE_RE.findall(path.read_text(encoding="utf-8")))
    map_records_by_id = {}
    for path in (game_root / "data/maps").glob("*/map.json"):
        data = read_json(path)
        map_records_by_id[data["id"]] = (path.parent.name, data)

    failures = []
    if trainer_offset != policy.get("expectedTrainerOffset"):
        failures.append(
            f"Wayfarer Hoenn Trainer offset expected {policy.get('expectedTrainerOffset')} "
            f"but found {trainer_offset}"
        )
    catalog_names = [row[3] for row in catalog]
    catalog_hash = fingerprint(catalog_names)
    grouped_name_counts = Counter(catalog_names)
    duplicate_names = sorted(name for name, count in grouped_name_counts.items() if count != 1)
    if duplicate_names:
        failures.append(f"Emerald maps occur more than once in map_groups.json: {', '.join(duplicate_names)}")
    disk_emerald_names = {
        path.parent.name
        for path in (game_root / "data/maps").glob("*/map.json")
        if read_json(path).get("game_version", "emerald") == "emerald"
    }
    omitted_names = sorted(disk_emerald_names - set(catalog_names))
    if omitted_names:
        failures.append(f"Emerald maps omitted from map_groups.json: {', '.join(omitted_names)}")
    if len(catalog) != policy["expectedMapCount"]:
        failures.append(f"catalog expected {policy['expectedMapCount']} Emerald maps but found {len(catalog)}")
    if catalog_hash != policy["expectedCatalogSha256"]:
        failures.append(f"ordered Emerald catalog fingerprint changed: {catalog_hash}")

    heals_by_map = defaultdict(list)
    for heal in heals:
        heals_by_map[heal["map"]].append(heal)
        for field in ("map", "respawn_map"):
            target = heal.get(field)
            if target and target not in selected_ids:
                failures.append(f"heal {heal['id']} {field} references unavailable map {target}")
        respawn_map = heal.get("respawn_map")
        respawn_npc = heal.get("respawn_npc")
        if respawn_map in selected_ids and respawn_npc not in {None, "0", "LOCALID_NONE"}:
            target_name, target_data = map_records_by_id[respawn_map]
            _, _, target_events = effective_owner(
                game_root, target_name, target_data, "shared_events_map"
            )
            local_ids = {event.get("local_id") for event in target_events.get("object_events") or []}
            if respawn_npc not in local_ids:
                failures.append(
                    f"heal {heal['id']} references missing respawn NPC {respawn_npc} on {respawn_map}"
                )

    maps = []
    trainer_fingerprint_rows = []
    map_content_fingerprint_rows = []
    referenced_trainer_names = set()
    raw_starter_consumers = []
    scoped_starter_consumers = []
    persistent_occurrences = defaultdict(list)
    all_emerald_ids = {row[5]["id"] for row in catalog}

    for group_number, group_name, map_number, map_name, map_path, data in catalog:
        classification = classify(map_name, policy)
        if classification.get("classification") not in {"required", "optional", "excluded"}:
            failures.append(f"{map_name} has invalid classification")
        if group_number > 127 or map_number > 127:
            failures.append(f"{map_name} group/map number exceeds signed warp range: {group_number}/{map_number}")

        layout_id = data.get("layout")
        layout = layouts.get(layout_id)
        layout_info = None
        if layout is None:
            failures.append(f"{map_name} references missing layout {layout_id}")
        else:
            if layout.get("game_version", "emerald") != "emerald":
                failures.append(f"{map_name} references non-Emerald layout {layout_id}")
            assets = {
                "border": layout.get("border_filepath"),
                "blockdata": layout.get("blockdata_filepath"),
            }
            for kind, asset in assets.items():
                if not asset or not (game_root / asset).is_file():
                    failures.append(f"{map_name} layout {layout_id} has missing {kind} asset {asset}")
            for key in ("primary_tileset", "secondary_tileset"):
                tileset = layout.get(key)
                if not tileset or not re.search(rf"\b{re.escape(tileset or '')}\b", tileset_declarations):
                    failures.append(f"{map_name} layout {layout_id} has missing {key} {tileset}")
            layout_info = {
                "id": layout_id,
                "number": layout["catalog_number"],
                "name": layout.get("name"),
                "version": layout.get("layout_version", layout.get("game_version", "emerald")),
                "primaryTileset": layout.get("primary_tileset"),
                "secondaryTileset": layout.get("secondary_tileset"),
                "assets": assets,
            }

        script_owner = data.get("shared_scripts_map", map_name)
        if script_owner == map_name:
            script_path = map_path.with_name("scripts.inc")
        else:
            script_label = f"{script_owner}_MapScripts"
            script_path = symbol_paths.get(script_label)
            if script_path is None:
                raise AuditError(f"{map_name} shared_scripts_map has no {script_label} definition")
        script_source = filter_wayfarer_source(
            script_path.read_text(encoding="utf-8") if script_path.exists() else ""
        )
        event_owner, _, event_data = effective_owner(
            game_root, map_name, data, "shared_events_map"
        )
        script_refs = event_script_references(event_data)
        missing_script_refs = sorted(script_refs - symbols)
        if missing_script_refs and classification.get("classification") == "required":
            failures.append(f"{map_name} has unresolved event scripts: {', '.join(missing_script_refs)}")

        script_entry_symbols = script_refs | referenced_script_symbols(script_source, symbols)
        reachable_blocks = reachable_script_blocks(script_entry_symbols, symbols, symbol_blocks)
        external_blocks = [block for block in reachable_blocks if block[1] != script_path]
        trainer_source = "\n".join([script_source, *(block[2] for block in external_blocks)])
        map_trainers = sorted(referenced_trainers(trainer_source, known_trainers))
        trainer_rows = []
        for trainer in map_trainers:
            referenced_trainer_names.add(trainer)
            source_id = trainer_source_ids.get(trainer)
            trainer_rows.append({
                "id": trainer,
                "sourceId": source_id,
                "wayfarerId": source_id + trainer_offset if source_id is not None else None,
            })
            if source_id is None:
                failures.append(f"{map_name} references Trainer without an Emerald source ID: {trainer}")
            elif source_id + trainer_offset >= 2048:
                failures.append(f"{map_name} Trainer {trainer} crosses partner boundary 2048")
            if trainer not in trainer_parties:
                failures.append(f"{map_name} references missing authored Trainer party {trainer}")
        trainer_fingerprint_rows.append(f"{map_name}:{','.join(map_trainers)}")

        source_parts = [
            (str(map_path.relative_to(game_root)), map_path.read_text(encoding="utf-8"), 1),
            (str(script_path.relative_to(game_root)), script_source, 1),
            *((str(path.relative_to(game_root)), source, start_line)
              for _, path, source, start_line in external_blocks),
        ]
        for source_name, source, start_line in source_parts:
            for line_number, line in enumerate(strip_asm_comments(source).splitlines(), start_line):
                for token in PERSISTENT_RE.findall(line):
                    persistent_occurrences[token].append({"map": map_name, "path": source_name, "line": line_number})
                for _ in re.finditer(r"\bVAR_STARTER_MON\b", line):
                    raw_starter_consumers.append({"map": map_name, "path": source_name, "line": line_number})
                for _ in re.finditer(r"\bVAR_HOENN_STARTER_CHOICE\b", line):
                    scoped_starter_consumers.append({"map": map_name, "path": source_name, "line": line_number})

        warps = []
        for index, warp in enumerate(event_data.get("warp_events") or []):
            target = warp.get("dest_map")
            dynamic = target == "MAP_DYNAMIC"
            resolved = dynamic or target in selected_ids
            if not resolved:
                failures.append(f"{map_name} warp {index} references unavailable map {target}")
            if resolved and not dynamic:
                try:
                    destination_warp = int(warp.get("dest_warp_id"))
                except (TypeError, ValueError):
                    destination_warp = -1
                if destination_warp >= 0:
                    target_name, target_data = map_records_by_id[target]
                    _, _, target_events = effective_owner(
                        game_root, target_name, target_data, "shared_events_map"
                    )
                    target_warp_count = len(target_events.get("warp_events") or [])
                    if destination_warp >= target_warp_count:
                        failures.append(
                            f"{map_name} warp {index} targets missing warp {destination_warp} "
                            f"on {target} ({target_warp_count} available)"
                        )
            warps.append({"index": index, "map": target, "warpId": warp.get("dest_warp_id"),
                          "dynamic": dynamic, "resolved": resolved})

        connections = []
        for connection in data.get("connections") or []:
            target = connection.get("map")
            resolved = target in selected_ids
            if not resolved:
                failures.append(f"{map_name} connection references unavailable map {target}")
            connections.append({"direction": connection.get("direction"), "offset": connection.get("offset"),
                                "map": target, "resolved": resolved})

        fly = None
        section = data.get("region_map_section")
        if section in fly_sections and data.get("map_type") in {"MAP_TYPE_TOWN", "MAP_TYPE_CITY"}:
            fly = {"mapSection": section, "visitedFlag": fly_sections[section]}

        methods = sorted(wild_methods.get(data["id"], set()))
        event_table_present = all(key in event_data for key in EVENT_KEYS)
        if classification.get("classification") == "required" and not event_table_present:
            failures.append(f"required map {map_name} has no complete effective event table")
        authored_content_hash = canonical_hash({
            "map": data,
            "effectiveEvents": event_data,
            "layout": layout,
            "layoutAssets": {
                kind: file_hash(game_root / asset) if asset else None
                for kind, asset in (layout_info or {}).get("assets", {}).items()
            },
            "scriptOwner": script_owner,
            "scriptSourceSha256": hashlib.sha256(script_source.encode("utf-8")).hexdigest(),
            "reachableExternalScripts": [
                {
                    "symbol": symbol,
                    "path": str(path.relative_to(game_root)),
                    "sha256": hashlib.sha256(source.encode("utf-8")).hexdigest(),
                }
                for symbol, path, source, _ in sorted(external_blocks)
            ],
            "healLocations": sorted(heals_by_map.get(data["id"], []), key=lambda row: row["id"]),
            "flyDestination": fly,
        })
        map_content_fingerprint_rows.append(f"{map_name}:{authored_content_hash}")
        maps.append({
            "id": data["id"],
            "name": map_name,
            "source": "emerald",
            "catalogIncluded": True,
            "group": {"name": group_name, "number": group_number},
            "number": map_number,
            "layout": layout_info,
            "scripts": {"present": script_path.exists(), "owner": script_owner,
                        "shared": script_owner != map_name, "eventReferences": sorted(script_refs)},
            "events": {"tablePresent": event_table_present,
                       "owner": event_owner, "shared": event_owner != map_name,
                       "counts": {"objects": len(event_data.get("object_events") or []),
                                  "warps": len(event_data.get("warp_events") or []),
                                  "coordinates": len(event_data.get("coord_events") or []),
                                  "backgrounds": len(event_data.get("bg_events") or [])}},
            "warps": warps,
            "connections": connections,
            "healLocations": sorted(heals_by_map.get(data["id"], []), key=lambda row: row["id"]),
            "flyDestination": fly,
            "trainers": trainer_rows,
            "wildEncounterMethods": methods,
            "authoredContentSha256": authored_content_hash,
            "classification": classification,
        })

    for encounter_map in sorted(wild_methods):
        if encounter_map not in all_ids:
            failures.append(f"wild encounter profile references unknown map {encounter_map}")
        elif all_ids[encounter_map] == "emerald" and encounter_map not in all_emerald_ids:
            failures.append(f"Emerald wild encounter profile references omitted map {encounter_map}")

    trainer_hash = fingerprint(trainer_fingerprint_rows)
    trainer_party_hash = fingerprint(
        f"{trainer}:{hashlib.sha256(trainer_parties[trainer].encode('utf-8')).hexdigest()}"
        for trainer in sorted(referenced_trainer_names)
        if trainer in trainer_parties
    )
    map_content_hash = fingerprint(map_content_fingerprint_rows)
    wild_rows = [f"{entry['name']}:{','.join(entry['wildEncounterMethods'])}" for entry in maps]
    wild_hash = fingerprint(wild_rows)
    emerald_wild_profiles = [profile for profile in all_wild_profiles if profile["map"] in all_emerald_ids]
    generated_wild_path = game_root / "src/data/wild_encounters.h"
    generated_wild_source = (
        generated_wild_path.read_text(encoding="utf-8") if generated_wild_path.is_file() else ""
    )
    if not generated_wild_source:
        failures.append("generated wild encounter table src/data/wild_encounters.h is missing")
    for profile in emerald_wild_profiles:
        if not profile["label"]:
            failures.append(f"Emerald wild profile for {profile['map']} has no generated label")
            continue
        for method in profile["methods"]:
            generated_label = profile["label"] + GENERATED_METHOD_SUFFIXES[method]
            if generated_label not in generated_wild_source:
                failures.append(
                    f"generated wild encounter table omits {generated_label} for {profile['map']}"
                )
    wild_profile_hash = fingerprint(
        f"{profile['map']}:{profile['label'] or ''}:{','.join(profile['methods'])}:{profile['authoredSha256']}"
        for profile in emerald_wild_profiles
    )
    starter_counts = Counter(row["map"] for row in scoped_starter_consumers)
    starter_hash = fingerprint(f"{name}:{starter_counts[name]}" for name in sorted(starter_counts))
    for field, actual in (
        ("expectedMapContentSha256", map_content_hash),
        ("expectedTrainerReferencesSha256", trainer_hash),
        ("expectedTrainerPartiesSha256", trainer_party_hash),
        ("expectedWildMethodsSha256", wild_hash),
        ("expectedWildProfilesSha256", wild_profile_hash),
        ("expectedStarterConsumersSha256", starter_hash),
    ):
        if policy.get(field) != actual:
            failures.append(f"policy {field} changed: {actual}")
    if policy.get("expectedWildProfileCount") != len(emerald_wild_profiles):
        failures.append(
            f"policy expected {policy.get('expectedWildProfileCount')} Emerald wild profiles "
            f"but found {len(emerald_wild_profiles)}"
        )

    required_maps = {entry["name"] for entry in maps if entry["classification"]["classification"] == "required"}
    for token, occurrences in sorted(persistent_occurrences.items()):
        if token in hoenn_values:
            value = hoenn_values[token]
            if value == 0 and any(row["map"] in required_maps for row in occurrences):
                failures.append(f"required Hoenn consumer maps persistent constant {token} to zero")
        elif token not in engine_values and token not in direct_persistent_names:
            failures.append(f"Hoenn source consumer has no generated persistent constant mapping for {token}")

    starter_mapping = hoenn_values.get("VAR_STARTER_MON")
    engine_starter = engine_values.get("VAR_STARTER_MON")
    if raw_starter_consumers:
        failures.append("included Hoenn consumers retain raw VAR_STARTER_MON instead of VAR_HOENN_STARTER_CHOICE")

    classification_counts = Counter(entry["classification"]["classification"] for entry in maps)
    optional_systems = []
    for rule in policy.get("rules", []):
        affected = [entry["name"] for entry in maps if entry["classification"].get("system") == rule.get("system")]
        if affected:
            optional_systems.append({
                "system": rule.get("system"),
                "status": rule["disposition"],
                "reason": rule["reason"],
                "specification": rule["specification"],
                "maps": affected,
            })
    manifest = {
        "schemaVersion": 1,
        "build": "wayfarer",
        "source": "emerald",
        "generatedDataLinkedIntoRom": False,
        "summary": {
            "mapCount": len(maps),
            "groupCount": len({entry["group"]["number"] for entry in maps}),
            "classificationCounts": dict(sorted(classification_counts.items())),
            "trainerReferenceCount": sum(len(entry["trainers"]) for entry in maps),
            "wildProfileMapCount": sum(bool(entry["wildEncounterMethods"]) for entry in maps),
            "wildProfileCount": len(emerald_wild_profiles),
            "starterConsumerCount": len(scoped_starter_consumers),
        },
        "fingerprints": {
            "catalogSha256": catalog_hash,
            "mapContentSha256": map_content_hash,
            "trainerReferencesSha256": trainer_hash,
            "trainerPartiesSha256": trainer_party_hash,
            "wildMethodsSha256": wild_hash,
            "wildProfilesSha256": wild_profile_hash,
            "starterConsumersSha256": starter_hash,
        },
        "starterVariable": {
            "rawSourceOccurrences": raw_starter_consumers,
            "sourceScopedOccurrences": scoped_starter_consumers,
            "wayfarerValue": starter_mapping,
            "engineValue": engine_starter,
            "sourceScoped": starter_mapping is not None and starter_mapping != engine_starter,
        },
        "persistentConstants": {
            "referencedCount": len(persistent_occurrences),
            "occurrences": dict(sorted(persistent_occurrences.items())),
        },
        "optionalSystems": optional_systems,
        "wildProfiles": emerald_wild_profiles,
        "maps": maps,
        "audit": {"passed": not failures, "failures": failures},
    }
    return manifest


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=GAME_ROOT, help="game repository root")
    parser.add_argument("--policy", type=Path, default=DEFAULT_POLICY)
    parser.add_argument("--hoenn-constants", type=Path)
    parser.add_argument("--engine-constants", type=Path)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    return parser.parse_args()


def main():
    args = parse_args()
    root = args.root.resolve()
    hoenn_constants = args.hoenn_constants or root / "data/wayfarer_hoenn_source_constants.inc"
    engine_constants = args.engine_constants or root / "data/wayfarer_engine_source_constants.inc"
    try:
        manifest = build_manifest(root, args.policy.resolve(), hoenn_constants.resolve(), engine_constants.resolve())
    except AuditError as error:
        raise SystemExit(f"Wayfarer Hoenn content audit failed: {error}") from error
    atomic_write(args.output, json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    if not manifest["audit"]["passed"]:
        for failure in manifest["audit"]["failures"]:
            print(f"error: {failure}")
        raise SystemExit(f"Wayfarer Hoenn content audit failed with {len(manifest['audit']['failures'])} error(s)")
    print(f"Wayfarer Hoenn content audit passed: {args.output} ({manifest['summary']['mapCount']} maps)")


if __name__ == "__main__":
    main()
