#!/usr/bin/env python3
"""Validate the frozen Wayfarer Sevii source catalog and event-island baseline.

The audit deliberately reads the source map and layout data rather than trusting
the manifest's copied identifiers.  It is useful before the selected maps are
linked, and remains useful after later milestones add event overlays and ferry
hooks.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any


MAP_PREFIXES = (
    "OneIsland_",
    "TwoIsland_",
    "ThreeIsland_",
    "FourIsland_",
    "FiveIsland_",
    "SixIsland_",
    "SevenIsland_",
    "MtEmber_",
    "TrainerTower_",
)
EXCLUDED_SOURCE_MAPS = ("SevenIsland_UnusedHouse",)
EVENT_ISLAND_PREFIXES = ("BirthIsland_", "NavelRock_")
EXPECTED_MAP_COUNT = 135
EXPECTED_LAYOUT_COUNT = 102
EXPECTED_RAW_LAYOUT_BYTES = 134_612
EVENT_KINDS = ("object_events", "warp_events", "coord_events", "bg_events")
NUMBERED_ISLAND_HEALS = {
    "HEAL_LOCATION_ONE_ISLAND",
    "HEAL_LOCATION_TWO_ISLAND",
    "HEAL_LOCATION_THREE_ISLAND",
    "HEAL_LOCATION_FOUR_ISLAND",
    "HEAL_LOCATION_FIVE_ISLAND",
    "HEAL_LOCATION_SIX_ISLAND",
    "HEAL_LOCATION_SEVEN_ISLAND",
}
NUMBERED_ISLAND_MAPSECS = {
    "MAPSEC_ONE_ISLAND", "MAPSEC_TWO_ISLAND", "MAPSEC_THREE_ISLAND",
    "MAPSEC_FOUR_ISLAND", "MAPSEC_FIVE_ISLAND", "MAPSEC_SIX_ISLAND",
    "MAPSEC_SEVEN_ISLAND",
}
OPPOSITE_CONNECTION = {"up": "down", "down": "up", "left": "right", "right": "left"}
PROHIBITED_SCRIPT_COMMANDS = {"trainerbattle", "giveitem", "givepokemon", "setwildbattle", "startwildbattle"}
PROHIBITED_STATE_TOKENS = (
    "VAR_MAP_SCENE_", "FLAG_SEVII_", "FLAG_CHAMPION", "NATIONAL_DEX",
    "LOSTELLE", "METEORITE", "ROCKET_PASSWORD", "ITEM_RUBY", "ITEM_SAPPHIRE",
)
COMMON_SHARED_HELPERS = {
    "EventScript_StrengthBoulder": "data/scripts/field_move_scripts_hns.inc",
}


class AuditError(ValueError):
    """A source, manifest, or event-island baseline invariant failed."""


def load_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as error:
        raise AuditError(f"missing required file: {display(path)}") from error
    except json.JSONDecodeError as error:
        raise AuditError(f"invalid JSON in {display(path)}: {error}") from error


def display(path: Path, root: Path | None = None) -> str:
    if root is not None:
        try:
            return path.relative_to(root).as_posix()
        except ValueError:
            pass
    return path.as_posix()


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def normalized_bytes(path: Path) -> bytes:
    """Return stable source bytes; JSON is structurally normalized."""
    if path.suffix == ".json":
        return json.dumps(load_json(path), sort_keys=True, separators=(",", ":")).encode()
    return path.read_bytes().replace(b"\r\n", b"\n")


def manifest_records(manifest: dict[str, Any]) -> list[dict[str, Any]]:
    if manifest.get("schema_version") != 1:
        raise AuditError("manifest schema_version must be 1")
    maps = manifest.get("maps")
    if not isinstance(maps, list):
        raise AuditError("manifest maps must be a list")
    required = {"source_map", "map_id", "layout", "category", "retained_events", "retained_map_scripts", "encounter_methods"}
    names: set[str] = set()
    for index, record in enumerate(maps):
        if not isinstance(record, dict):
            raise AuditError(f"manifest maps[{index}] must be an object")
        missing = sorted(required - record.keys())
        if missing:
            raise AuditError(f"manifest maps[{index}] missing {', '.join(missing)}")
        name = record["source_map"]
        if not isinstance(name, str) or not name:
            raise AuditError(f"manifest maps[{index}].source_map must be a nonempty string")
        if name in names:
            raise AuditError(f"manifest repeats source map {name}")
        names.add(name)
        if not isinstance(record["retained_events"], dict):
            raise AuditError(f"manifest {name}: retained_events must be an object")
        for event_type in ("object_events", "warp_events", "coord_events", "bg_events"):
            events = record["retained_events"].get(event_type)
            if not isinstance(events, list):
                raise AuditError(f"manifest {name}: retained_events.{event_type} must be a list")
        for key in ("retained_map_scripts", "encounter_methods"):
            if not isinstance(record[key], list):
                raise AuditError(f"manifest {name}: {key} must be a list")
    return maps


def source_group_order(root: Path) -> list[str]:
    groups = load_json(root / "data/maps/map_groups.json")
    result: list[str] = []
    for group in groups.get("group_order", []):
        for name in groups.get(group, []):
            if name.startswith(MAP_PREFIXES) and name not in EXCLUDED_SOURCE_MAPS:
                map_data = load_json(root / "data/maps" / name / "map.json")
                if map_data.get("game_version") == "frlg":
                    result.append(name)
    if len(result) != len(set(result)):
        raise AuditError("registered Sevii source map appears more than once in map_groups.json")
    return result


def event_island_source_maps(root: Path) -> list[str]:
    maps: list[str] = []
    for path in sorted((root / "data/maps").glob("*/map.json")):
        data = load_json(path)
        name = path.parent.name
        if name.startswith(EVENT_ISLAND_PREFIXES) and data.get("game_version") == "frlg":
            maps.append(name)
    return maps


def layout_catalog(root: Path) -> dict[str, dict[str, Any]]:
    layouts = load_json(root / "data/layouts/layouts.json").get("layouts")
    if not isinstance(layouts, list):
        raise AuditError("layouts.json layouts must be a list")
    catalog: dict[str, dict[str, Any]] = {}
    for layout in layouts:
        layout_id = layout.get("id")
        if not isinstance(layout_id, str):
            raise AuditError("layout without string id")
        if layout_id in catalog:
            raise AuditError(f"duplicate layout id {layout_id}")
        catalog[layout_id] = layout
    return catalog


def validate_exclusions(manifest: dict[str, Any], selected: set[str], event_sources: list[str]) -> list[dict[str, str]]:
    exclusions = manifest.get("exclusions")
    if not isinstance(exclusions, list):
        raise AuditError("manifest exclusions must be a list")
    by_name: dict[str, str] = {}
    for index, record in enumerate(exclusions):
        if not isinstance(record, dict) or not isinstance(record.get("source_map"), str) or not isinstance(record.get("reason"), str):
            raise AuditError(f"manifest exclusions[{index}] must have string source_map and reason")
        if not record["reason"].strip():
            raise AuditError(f"manifest exclusion {record['source_map']} has an empty reason")
        if record["source_map"] in by_name:
            raise AuditError(f"manifest repeats exclusion {record['source_map']}")
        by_name[record["source_map"]] = record["reason"]
    if selected & set(by_name):
        raise AuditError(f"selected source map is also excluded: {sorted(selected & set(by_name))[0]}")
    required = set(EXCLUDED_SOURCE_MAPS) | set(event_sources)
    missing = sorted(required - set(by_name))
    if missing:
        raise AuditError(f"manifest omissions lack exclusion reasons: {', '.join(missing)}")
    return [{"source_map": name, "reason": by_name[name]} for name in sorted(by_name)]


def block_bodies(path: Path) -> dict[str, bytes]:
    """Split ordinary ``Label::`` assembler blocks for protected-hook checks."""
    source = path.read_text(encoding="utf-8").replace("\r\n", "\n")
    # Event scripts use both global ``Label::`` and local-data ``Label:``
    # labels.  The baseline hashes both forms, so a one-colon label cannot be
    # smuggled into a protected source file as an untracked block.
    labels = list(re.finditer(r"(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}\s*$", source))
    bodies: dict[str, bytes] = {}
    for index, match in enumerate(labels):
        end = labels[index + 1].start() if index + 1 < len(labels) else len(source)
        bodies[match.group(1)] = source[match.start():end].encode()
    return bodies


def ferry_hook(hook: Any) -> tuple[tuple[str, str], dict[str, str]]:
    required = ("path", "label", "sha256", "source_warp", "wayfarer_warp")
    if not isinstance(hook, dict) or any(not isinstance(hook.get(key), str) for key in required):
        raise AuditError("allowed_ferry_hooks entries must define the guarded return-warp replacement")
    return (hook["path"], hook["label"]), {key: hook[key] for key in required}


def validate_ferry_hook_change(body: bytes, hook: dict[str, str]) -> None:
    """Allow only the reviewed Wayfarer preprocessor replacement of one warp."""
    if sha256_bytes(body) == hook["sha256"]:
        return
    source_warp = hook["source_warp"]
    guarded_warp = (
        f"#if IS_WAYFARER\n\t{hook['wayfarer_warp']}\n#else\n\t{source_warp}\n#endif"
    )
    current = body.decode("utf-8")
    if current.count(guarded_warp) != 1:
        raise AuditError("event-island ferry hook has an unapproved change")
    restored = current.replace(guarded_warp, f"\t{source_warp}")
    if sha256_bytes(restored.encode("utf-8")) != hook["sha256"]:
        raise AuditError("event-island ferry hook has an unapproved change")


def baseline_source_path(root: Path, recorded_path: str) -> Path:
    """Resolve both game-root and repository-root baseline path conventions."""
    direct = root / recorded_path
    if direct.is_file() or root.name != "game" or not recorded_path.startswith("game/"):
        return direct
    return root.parent / recorded_path


def validate_event_island_baseline(root: Path, baseline_path: Path) -> dict[str, Any]:
    """Check exact protected content while permitting only named ferry hooks.

    The first baseline is intentionally generated outside this tool.  A file
    with no allowed hook is byte-hash protected.  A script with an allowed hook
    is protected block-by-block and may change only the named labels.
    """
    baseline = load_json(baseline_path)
    if baseline.get("schema_version") != 1 or not isinstance(baseline.get("files"), list):
        raise AuditError("event-island baseline requires schema_version 1 and files")
    hooks = dict(ferry_hook(hook) for hook in baseline.get("allowed_ferry_hooks", []))
    insertions = baseline.get("allowed_global_insertions", [])
    if not isinstance(insertions, list):
        raise AuditError("event-island baseline allowed_global_insertions must be a list")
    insertions_by_path: dict[str, list[dict[str, str]]] = {}
    for insertion in insertions:
        required = ("path", "after", "insertion")
        if not isinstance(insertion, dict) or any(not isinstance(insertion.get(key), str) for key in required):
            raise AuditError("allowed_global_insertions entries require path, after, and insertion")
        insertions_by_path.setdefault(insertion["path"], []).append(insertion)
    rows = []
    for index, record in enumerate(baseline["files"]):
        if not isinstance(record, dict) or not isinstance(record.get("path"), str) or not isinstance(record.get("sha256"), str):
            raise AuditError(f"event-island baseline files[{index}] requires path and sha256")
        path = baseline_source_path(root, record["path"])
        if not path.is_file():
            raise AuditError(f"event-island baseline file missing: {record['path']}")
        actual_sha = sha256_file(path)
        normalized_sha = sha256_bytes(normalized_bytes(path))
        protected = record.get("protected_blocks", [])
        if not isinstance(protected, list):
            raise AuditError(f"event-island baseline {record['path']}: protected_blocks must be a list")
        file_hooks = {label: hook for (hook_path, label), hook in hooks.items() if hook_path == record["path"]}
        if file_hooks or protected:
            blocks = block_bodies(path)
            protected_names = set()
            for block in protected:
                if not isinstance(block, dict) or not isinstance(block.get("label"), str) or not isinstance(block.get("sha256"), str):
                    raise AuditError(f"event-island baseline {record['path']}: invalid protected block")
                label = block["label"]
                if label in protected_names:
                    raise AuditError(f"event-island baseline {record['path']}: repeated protected label {label}")
                protected_names.add(label)
                if label not in blocks:
                    raise AuditError(f"event-island protected label disappeared: {record['path']}:{label}")
                if sha256_bytes(blocks[label]) != block["sha256"]:
                    raise AuditError(f"event-island protected block changed: {record['path']}:{label}")
            unapproved = sorted(set(blocks) - protected_names - set(file_hooks))
            if unapproved:
                raise AuditError(f"event-island unapproved script block: {record['path']}:{unapproved[0]}")
            for label, hook in file_hooks.items():
                if label not in blocks:
                    raise AuditError(f"event-island ferry hook disappeared: {record['path']}:{label}")
                validate_ferry_hook_change(blocks[label], hook)
        elif actual_sha != record["sha256"]:
            restored = path.read_text(encoding="utf-8").replace("\r\n", "\n")
            for insertion in insertions_by_path.get(record["path"], []):
                expected = insertion["after"] + insertion["insertion"]
                if restored.count(expected) != 1:
                    raise AuditError(f"event-island global insertion changed: {record['path']}")
                restored = restored.replace(expected, insertion["after"], 1)
            if sha256_bytes(restored.encode("utf-8")) != record["sha256"]:
                raise AuditError(f"event-island baseline file changed: {record['path']}")
        expected_normalized = record.get("normalized_sha256")
        if expected_normalized is not None and not (file_hooks or protected):
            restored_normalized = normalized_bytes(path)
            for insertion in insertions_by_path.get(record["path"], []):
                expected = (insertion["after"] + insertion["insertion"]).encode()
                if restored_normalized.count(expected) != 1:
                    raise AuditError(f"event-island global insertion changed: {record['path']}")
                restored_normalized = restored_normalized.replace(expected, insertion["after"].encode(), 1)
            if sha256_bytes(restored_normalized) != expected_normalized:
                raise AuditError(f"event-island normalized source changed: {record['path']}")
        rows.append({"path": record["path"], "sha256": actual_sha, "normalized_sha256": normalized_sha,
                     "protected_block_count": len(protected), "allowed_ferry_hooks": sorted(file_hooks)})
    known_paths = {record["path"] for record in baseline["files"]}
    missing_hook_file = sorted({path for path, _ in hooks} - known_paths)
    if missing_hook_file:
        raise AuditError(f"event-island allowed hook has no baseline file: {missing_hook_file[0]}")
    missing_insertion_file = sorted(set(insertions_by_path) - known_paths)
    if missing_insertion_file:
        raise AuditError(f"event-island allowed insertion has no baseline file: {missing_insertion_file[0]}")
    return {"path": display(baseline_path, root), "file_count": len(rows), "files": rows}


def release_enabled_records(manifest: dict[str, Any], records: list[dict[str, Any]]) -> set[str]:
    """Return the catalog actually linkable by the Wayfarer adapter."""
    release_enabled = manifest.get("release_link_enabled")
    if not isinstance(release_enabled, bool):
        raise AuditError("manifest release_link_enabled must be a boolean")
    enabled: set[str] = set()
    for record in records:
        value = record.get("enabled", release_enabled)
        if not isinstance(value, bool):
            raise AuditError(f"manifest {record['source_map']}: enabled must be a boolean")
        if release_enabled and value:
            enabled.add(record["source_map"])
    return enabled


def retained_event_rows(record: dict[str, Any], source: dict[str, Any]) -> tuple[dict[str, dict[str, int]], list[dict[str, Any]]]:
    """Validate reviewed source identities and count the sanitized event overlay."""
    retained = record["retained_events"]
    counts: dict[str, dict[str, int]] = {}
    script_rows: list[dict[str, Any]] = []
    for kind in EVENT_KINDS:
        events = source.get(kind, [])
        if not isinstance(events, list):
            raise AuditError(f"{record['source_map']}: source {kind} must be a list")
        if kind == "warp_events":
            counts[kind] = {"source": len(events), "retained": 0, "removed": len(events)}
            continue
        rules = retained[kind]
        seen: set[int] = set()
        for rule in rules:
            if not isinstance(rule, dict) or not isinstance(rule.get("index"), int) or not isinstance(rule.get("source"), dict):
                raise AuditError(f"{record['source_map']}: retained {kind} requires index and exact source identity")
            index = rule["index"]
            if index < 0 or index >= len(events) or index in seen:
                raise AuditError(f"{record['source_map']}: retained {kind} has an invalid or repeated index")
            seen.add(index)
            if rule["source"] != events[index]:
                raise AuditError(f"{record['source_map']}: retained {kind}[{index}] source identity drifted")
            if kind == "object_events" and events[index].get("trainer_type", "TRAINER_TYPE_NONE") != "TRAINER_TYPE_NONE":
                raise AuditError(f"{record['source_map']}: retained object event {index} is a Trainer")
            source_script = str(events[index].get("script", ""))
            replacement = rule.get("wayfarer_script")
            if source_script not in ("", "0", "0x0", "NULL"):
                if (not isinstance(replacement, str)
                        or (not replacement.startswith("WayfarerSevii_") and replacement not in COMMON_SHARED_HELPERS)):
                    raise AuditError(f"{record['source_map']}: retained {kind}[{index}] lacks a Wayfarer-owned script")
                script_rows.append({"map": record["source_map"], "event_kind": kind, "index": index,
                                    "label": replacement})
            elif replacement not in (None, ""):
                raise AuditError(f"{record['source_map']}: retained {kind}[{index}] replaces a scriptless event")
        counts[kind] = {"source": len(events), "retained": len(seen), "removed": len(events) - len(seen)}
    return counts, script_rows


def event_output_and_paths(records: list[dict[str, Any]], sources: dict[str, dict[str, Any]], enabled: set[str]) -> tuple[dict[str, list[dict[str, Any]]], dict[str, list[dict[str, Any]]]]:
    """Project mapjson's retained static paths and annotate reverse paths."""
    map_ids = {record["source_map"]: record["map_id"] for record in records}
    source_by_id = {record["map_id"]: sources[record["source_map"]] for record in records}
    enabled_ids = {map_ids[name] for name in enabled}
    paths: dict[str, list[dict[str, Any]]] = {"warps": [], "connections": []}
    per_map: dict[str, list[dict[str, Any]]] = {}
    for record in records:
        name = record["source_map"]
        source = sources[name]
        output: list[dict[str, Any]] = []
        if name in enabled:
            for index, warp in enumerate(source.get("warp_events") or []):
                destination = warp.get("dest_map")
                if destination not in enabled_ids and destination not in {"MAP_DYNAMIC", "MAP_UNDEFINED"}:
                    continue
                row = {"map": name, "index": index, "destination": destination,
                       "destination_warp_id": warp.get("dest_warp_id"), "reciprocal": False}
                target = source_by_id.get(destination)
                if target is not None:
                    try:
                        target_index = int(str(warp.get("dest_warp_id")))
                    except ValueError as error:
                        raise AuditError(f"{name}: warp {index} has invalid destination warp id") from error
                    target_warps = target.get("warp_events") or []
                    if target_index < 0 or target_index >= len(target_warps):
                        raise AuditError(f"{name}: warp {index} points to missing destination warp {destination}[{target_index}]")
                    row["target_map"] = next(key for key, value in map_ids.items() if value == destination)
                    row["target_index"] = target_index
                    row["reciprocal"] = target_warps[target_index].get("dest_map") == record["map_id"]
                paths["warps"].append(row)
                output.append(row)
            for index, connection in enumerate(source.get("connections") or []):
                destination = connection.get("map")
                if destination not in enabled_ids:
                    continue
                target = source_by_id.get(destination)
                reciprocal = False
                if target is not None:
                    reciprocal = any(
                        candidate.get("map") == record["map_id"]
                        and candidate.get("direction") == OPPOSITE_CONNECTION.get(connection.get("direction"))
                        for candidate in (target.get("connections") or [])
                    )
                row = {"map": name, "index": index, "destination": destination,
                       "direction": connection.get("direction"), "offset": connection.get("offset"),
                       "reciprocal": reciprocal}
                paths["connections"].append(row)
                output.append(row)
        per_map[name] = output
    return paths, per_map


def script_closure(root: Path, records: list[dict[str, Any]]) -> dict[str, Any]:
    """Report every explicitly retained Wayfarer-owned script and reject story ownership."""
    includes: set[str] = set()
    labels: set[str] = set()
    map_script_tables = {f"{record['source_map']}_MapScripts" for record in records if record["retained_map_scripts"]}
    table_labels: set[str] = set()
    commands: set[str] = set()
    event_labels: set[str] = set()
    if (root / "data/scripts/wayfarer_sevii/common.inc").is_file():
        includes.add("data/scripts/wayfarer_sevii/common.inc")
    for record in records:
        for entry in record["retained_map_scripts"]:
            if not isinstance(entry, dict) or not isinstance(entry.get("include"), str):
                raise AuditError(f"{record['source_map']}: retained map script must name an include")
            include = entry["include"]
            if not include.startswith("data/scripts/wayfarer_sevii/") or ".." in Path(include).parts:
                raise AuditError(f"{record['source_map']}: script include is outside Wayfarer ownership")
            includes.add(include)
    for record in records:
        _, retained_labels = retained_event_rows(record, load_json(root / "data/maps" / record["source_map"] / "map.json"))
        event_labels.update(row["label"] for row in retained_labels)
    pending = sorted(includes)
    visited: set[str] = set()
    while pending:
        include = pending.pop()
        if include in visited:
            continue
        visited.add(include)
        path = root / include
        if not path.is_file():
            raise AuditError(f"Wayfarer-owned script include is missing: {include}")
        source = path.read_text(encoding="utf-8")
        for match in re.finditer(r"(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}", source):
            label = match.group(1)
            if label in map_script_tables:
                table_labels.add(label)
            elif not label.startswith("WayfarerSevii_"):
                raise AuditError(f"Wayfarer-owned script has non-Wayfarer label {label} in {include}")
            labels.add(label)
        for line in source.splitlines():
            code = line.split("@", 1)[0].strip()
            include_match = re.fullmatch(r'\.include\s+"([^"]+)"', code)
            if include_match:
                child = include_match.group(1)
                if child.startswith("data/scripts/wayfarer_sevii/"):
                    pending.append(child)
                continue
            command = re.match(r"([a-z][a-z0-9_]*)\b", code)
            if command:
                commands.add(command.group(1))
                if command.group(1) in PROHIBITED_SCRIPT_COMMANDS:
                    raise AuditError(f"Wayfarer-owned script uses prohibited command {command.group(1)} in {include}")
            if any(token in code for token in PROHIBITED_STATE_TOKENS):
                raise AuditError(f"Wayfarer-owned script uses prohibited FRLG state in {include}")
    shared_helpers: list[dict[str, Any]] = []
    for label in sorted(event_labels & set(COMMON_SHARED_HELPERS)):
        path = root / COMMON_SHARED_HELPERS[label]
        if not path.is_file():
            raise AuditError(f"approved shared helper source is missing: {label}")
        blocks = block_bodies(path)
        if label not in blocks:
            raise AuditError(f"approved shared helper label is missing: {label}")
        body = blocks[label].decode("utf-8")
        if any(re.search(rf"(?m)^\s*{command}\b", body) for command in PROHIBITED_SCRIPT_COMMANDS):
            raise AuditError(f"approved shared helper uses a prohibited command: {label}")
        if any(token in body for token in PROHIBITED_STATE_TOKENS):
            raise AuditError(f"approved shared helper uses prohibited FRLG state: {label}")
        shared_helpers.append({"label": label, "path": COMMON_SHARED_HELPERS[label]})
    missing = sorted(event_labels - labels - set(COMMON_SHARED_HELPERS))
    if missing:
        raise AuditError(f"retained event script is not owned by a retained Wayfarer include: {missing[0]}")
    return {"includes": sorted(visited), "labels": sorted(labels), "commands": sorted(commands),
            "retained_event_labels": sorted(event_labels), "shared_helpers": shared_helpers,
            "map_script_tables": sorted(table_labels)}


def recovery_and_ferry_report(root: Path, enabled_ids: set[str], release_enabled: bool) -> dict[str, Any]:
    """Prove the fixed healing, no-Fly, and protected-ferry surface from source."""
    heal_path = root / "src/data/heal_locations.json"
    heals: list[dict[str, Any]] = []
    if heal_path.is_file():
        rows = load_json(heal_path).get("heal_locations", [])
        heals = [row for row in rows if row.get("wayfarer_sevii") is True]
        found = {row.get("id") for row in heals}
        if found != NUMBERED_ISLAND_HEALS:
            raise AuditError("Wayfarer Sevii heal records must be exactly the seven numbered islands")
        for row in heals:
            if row.get("source") != "FRLG" or row.get("map") not in enabled_ids and release_enabled:
                raise AuditError(f"Wayfarer Sevii heal record is not backed by an enabled selected map: {row.get('id')}")
            if row.get("respawn_map") not in enabled_ids and release_enabled:
                raise AuditError(f"Wayfarer Sevii heal record has unavailable respawn map: {row.get('id')}")
            if str(row.get("map", "")).startswith(("MAP_BIRTH", "MAP_NAVEL")) or str(row.get("respawn_map", "")).startswith(("MAP_BIRTH", "MAP_NAVEL")):
                raise AuditError(f"Wayfarer Sevii heal record touches an event island: {row.get('id')}")
    fly_cases: list[str] = []
    region_map = root / "src/region_map.c"
    if region_map.is_file():
        text = region_map.read_text(encoding="utf-8")
        # The old FRLG cases are under !IS_HNS. A Wayfarer-only case may exist,
        # but none may return a Fly-enabled map section type.
        for mapsec in NUMBERED_ISLAND_MAPSECS:
            for match in re.finditer(rf"case\s+{mapsec}:([\s\S]{{0,220}}?)(?=\n\s*case\s+|\n\s*default:)", text):
                block = match.group(0)
                prefix = text[max(0, match.start() - 500):match.start()]
                if "#if IS_WAYFARER" in prefix[prefix.rfind("#if"):]:
                    if "MAPSECTYPE_CITY_CANFLY" in block:
                        fly_cases.append(mapsec)
    if fly_cases:
        raise AuditError(f"Wayfarer Sevii must not add Fly destinations: {fly_cases[0]}")
    ferry = {"vermilion_target": "MAP_VERMILION_CITY_PORT_INSIDE_HNS", "return": {"x": 8, "y": 9},
             "special_arrivals": {"Birth": {"map": "MAP_BIRTH_ISLAND_HARBOR_HNS", "x": 8, "y": 5},
                                  "Navel": {"map": "MAP_NAVEL_ROCK_HARBOR", "x": 8, "y": 5}}}
    ferry_inputs = (root / "src/seagallop.c", root / "src/wayfarer_sevii_ferry.c",
                    root / "data/maps/BirthIsland_Harbor_hns/scripts.inc", root / "data/maps/NavelRock_Harbor/scripts.inc")
    if release_enabled and all(path.is_file() for path in ferry_inputs):
        seagallop = ferry_inputs[0].read_text(encoding="utf-8")
        ferry_source = ferry_inputs[1].read_text(encoding="utf-8")
        expected_rows = (("SEAGALLOP_BIRTH_ISLAND", "MAP_BIRTH_ISLAND_HARBOR_HNS"),
                         ("SEAGALLOP_NAVEL_ROCK", "MAP_NAVEL_ROCK_HARBOR"))
        for destination, target in expected_rows:
            if not re.search(rf"\[{destination}\]\s*=\s*\{{\s*MAP_GROUP\({target}\),\s*MAP_NUM\({target}\),\s*0x08,\s*0x05", seagallop):
                raise AuditError(f"Wayfarer ferry has wrong {destination} target or arrival tile")
        for name, item, gate in (("BirthIsland", "ITEM_AURORA_TICKET", "WayfarerCanUseRegularAqua"),
                                 ("NavelRock", "ITEM_MYSTIC_TICKET", "FLAG_SYS_GAME_CLEAR")):
            match = re.search(rf"u16\s+WayfarerCanSailTo{name}\(void\)\s*\{{([\s\S]*?)\n\}}", ferry_source)
            if match is None or item not in match.group(1) or gate not in match.group(1):
                raise AuditError(f"Wayfarer ferry lacks the reviewed {name} read-only eligibility predicate")
            if re.search(r"(?:FlagSet|FlagClear|AddBagItem|RemoveBagItem|VarSet)", match.group(1)):
                raise AuditError(f"Wayfarer ferry {name} eligibility predicate writes persistent state")
        for path in ferry_inputs[2:]:
            body = path.read_text(encoding="utf-8")
            if "#if IS_WAYFARER\n\twarp MAP_VERMILION_CITY_PORT_INSIDE_HNS, 8, 9\n#else" not in body:
                raise AuditError(f"event-island harbor lacks the reviewed HNS Vermilion return: {display(path, root)}")
    return {"heals": heals, "blackout_destinations": [
        {"heal_location": row["id"], "map": row.get("respawn_map"), "npc": row.get("respawn_npc")}
        for row in heals], "numbered_island_fly_destinations": [], "ferry": ferry}


def build_report(root: Path, manifest_path: Path, *, expected_map_count: int = EXPECTED_MAP_COUNT,
                 expected_layout_count: int = EXPECTED_LAYOUT_COUNT,
                 expected_raw_bytes: int = EXPECTED_RAW_LAYOUT_BYTES,
                 baseline_path: Path | None = None) -> dict[str, Any]:
    root = root.resolve()
    manifest = load_json(manifest_path)
    records = manifest_records(manifest)
    enabled_names = release_enabled_records(manifest, records)
    source_order = source_group_order(root)
    selected_order = [record["source_map"] for record in records]
    if selected_order != source_order:
        raise AuditError("manifest map ordering or membership differs from registered FRLG Sevii source catalog")
    if len(records) != expected_map_count:
        raise AuditError(f"expected {expected_map_count} selected maps, found {len(records)}")
    event_sources = event_island_source_maps(root)
    exclusions = validate_exclusions(manifest, set(selected_order), event_sources)
    layouts = layout_catalog(root)
    selected_layouts: dict[str, dict[str, Any]] = {}
    maps_report = []
    sources: dict[str, dict[str, Any]] = {}
    retained_script_rows: list[dict[str, Any]] = []
    for record in records:
        name = record["source_map"]
        map_path = root / "data/maps" / name / "map.json"
        source = load_json(map_path)
        sources[name] = source
        if source.get("game_version") != "frlg":
            raise AuditError(f"{name}: source map is not FRLG")
        for key in ("map_id", "layout"):
            source_key = "id" if key == "map_id" else key
            if record[key] != source.get(source_key):
                raise AuditError(f"{name}: manifest {key} does not match source")
        layout_id = record["layout"]
        if layout_id not in layouts:
            raise AuditError(f"{name}: source layout {layout_id} is missing")
        layout = layouts[layout_id]
        if layout.get("game_version") not in (None, "frlg") or layout.get("layout_version") != "frlg":
            raise AuditError(f"{name}: {layout_id} is not an FRLG layout-version layout")
        selected_layouts[layout_id] = layout
        event_counts, script_rows = retained_event_rows(record, source)
        enabled = name in enabled_names
        for kind in ("object_events", "coord_events", "bg_events"):
            event_counts[kind]["emitted"] = event_counts[kind]["retained"] if enabled else 0
        source_warps = source.get("warp_events", [])
        emitted_warps = sum(
            1 for warp in source_warps
            if enabled and (warp.get("dest_map") in {other["map_id"] for other in records if other["source_map"] in enabled_names}
                            or warp.get("dest_map") in {"MAP_DYNAMIC", "MAP_UNDEFINED"})
        )
        event_counts["warp_events"] = {"source": len(source_warps), "retained": emitted_warps,
                                        "emitted": emitted_warps, "removed": len(source_warps) - emitted_warps}
        retained_script_rows.extend(script_rows)
        maps_report.append({
            "source_map": name, "map_id": record["map_id"], "layout": layout_id,
            "category": record["category"], "source_path": display(map_path, root),
            "enabled": enabled,
            "event_counts": event_counts,
            "encounter_methods": record["encounter_methods"],
        })
    if len(selected_layouts) != expected_layout_count:
        raise AuditError(f"expected {expected_layout_count} unique layouts, found {len(selected_layouts)}")
    layouts_report, raw_bytes, primary_tilesets, secondary_tilesets = [], 0, set(), set()
    for layout_id, layout in sorted(selected_layouts.items()):
        paths = []
        for key in ("blockdata_filepath", "border_filepath"):
            relative = layout.get(key)
            if not isinstance(relative, str):
                raise AuditError(f"{layout_id}: missing {key}")
            path = root / relative
            if not path.is_file():
                raise AuditError(f"{layout_id}: missing source data {relative}")
            raw_bytes += path.stat().st_size
            paths.append({"path": relative, "bytes": path.stat().st_size})
        primary_tilesets.add(layout.get("primary_tileset"))
        secondary_tilesets.add(layout.get("secondary_tileset"))
        layouts_report.append({"layout": layout_id, "source_files": paths,
                               "primary_tileset": layout.get("primary_tileset"),
                               "secondary_tileset": layout.get("secondary_tileset")})
    if raw_bytes != expected_raw_bytes:
        raise AuditError(f"expected {expected_raw_bytes} raw layout bytes, found {raw_bytes}")
    paths, _ = event_output_and_paths(records, sources, enabled_names)
    script_report = script_closure(root, records)
    if sorted({row["label"] for row in retained_script_rows}) != script_report["retained_event_labels"]:
        raise AuditError("retained event script report does not close over its Wayfarer-owned labels")
    recovery = recovery_and_ferry_report(root, {record["map_id"] for record in records if record["source_map"] in enabled_names},
                                         manifest["release_link_enabled"])
    baseline = None
    if baseline_path is not None:
        baseline = validate_event_island_baseline(root, baseline_path)
    return {
        "schema_version": 1,
        "product": "WAYFARER_SEVII_PORT",
        "manifest_path": display(manifest_path, root),
        "release_link_enabled": manifest["release_link_enabled"],
        "selected_map_count": len(maps_report),
        "enabled_map_count": len(enabled_names),
        "enabled_maps": [record["source_map"] for record in records if record["source_map"] in enabled_names],
        "selected_layout_count": len(layouts_report),
        "raw_layout_bytes": raw_bytes,
        "maps": maps_report,
        "layouts": layouts_report,
        "tilesets": {"primary": sorted(primary_tilesets), "secondary": sorted(secondary_tilesets)},
        "event_island_frlg_exclusions": event_sources,
        "intentional_exclusions": exclusions,
        "paths": paths,
        "retained_scripts": script_report,
        "recovery_and_ferry": recovery,
        "event_island_baseline": baseline,
        "invariants": {"passed": True},
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    root = args.root.resolve()
    manifest = (args.manifest or root / "src/data/wayfarer_sevii_maps.json")
    baseline = args.baseline or root / "tools/wayfarer_sevii_port/event_island_baseline.json"
    try:
        report = build_report(root, manifest, baseline_path=baseline)
    except AuditError as error:
        print(f"wayfarer-sevii-port audit: {error}", file=sys.stderr)
        return 1
    serialized = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(serialized, encoding="utf-8")
    else:
        print(serialized, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
