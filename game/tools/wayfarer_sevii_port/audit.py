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
            raise AuditError(f"event-island baseline file changed: {record['path']}")
        expected_normalized = record.get("normalized_sha256")
        if expected_normalized is not None and not (file_hooks or protected) and normalized_sha != expected_normalized:
            raise AuditError(f"event-island normalized source changed: {record['path']}")
        rows.append({"path": record["path"], "sha256": actual_sha, "normalized_sha256": normalized_sha,
                     "protected_block_count": len(protected), "allowed_ferry_hooks": sorted(file_hooks)})
    known_paths = {record["path"] for record in baseline["files"]}
    missing_hook_file = sorted({path for path, _ in hooks} - known_paths)
    if missing_hook_file:
        raise AuditError(f"event-island allowed hook has no baseline file: {missing_hook_file[0]}")
    return {"path": display(baseline_path, root), "file_count": len(rows), "files": rows}


def build_report(root: Path, manifest_path: Path, *, expected_map_count: int = EXPECTED_MAP_COUNT,
                 expected_layout_count: int = EXPECTED_LAYOUT_COUNT,
                 expected_raw_bytes: int = EXPECTED_RAW_LAYOUT_BYTES,
                 baseline_path: Path | None = None) -> dict[str, Any]:
    root = root.resolve()
    manifest = load_json(manifest_path)
    records = manifest_records(manifest)
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
    for record in records:
        name = record["source_map"]
        map_path = root / "data/maps" / name / "map.json"
        source = load_json(map_path)
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
        maps_report.append({
            "source_map": name, "map_id": record["map_id"], "layout": layout_id,
            "category": record["category"], "source_path": display(map_path, root),
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
    baseline = None
    if baseline_path is not None:
        baseline = validate_event_island_baseline(root, baseline_path)
    return {
        "schema_version": 1,
        "product": "WAYFARER_SEVII_PORT",
        "manifest_path": display(manifest_path, root),
        "selected_map_count": len(maps_report),
        "selected_layout_count": len(layouts_report),
        "raw_layout_bytes": raw_bytes,
        "maps": maps_report,
        "layouts": layouts_report,
        "tilesets": {"primary": sorted(primary_tilesets), "secondary": sorted(secondary_tilesets)},
        "event_island_frlg_exclusions": event_sources,
        "intentional_exclusions": exclusions,
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
