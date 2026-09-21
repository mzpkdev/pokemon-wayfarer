#!/usr/bin/env python3
"""Import the sealed Sinnoh map catalog from a verified local donor checkout.

The checked-in manifests are the authority for both membership and every target
symbol.  This command intentionally never fetches, discovers additional donor
maps, or edits map blockdata.  ``--check`` proves that a prior import remains
the deterministic projection of those authorities.
"""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path
from typing import Any

from foundation import DONOR_COMMIT, DONOR_URL, EXPECTED_COUNTS, FoundationError, load_json, sha256_file
from topology import effective_connections, effective_warps


def contained_path(base: Path, relative: str | Path, field: str) -> Path:
    """Resolve a manifest-derived path only when it stays under its authority."""
    candidate = Path(relative)
    if candidate.is_absolute() or ".." in candidate.parts:
        raise FoundationError(f"Sinnoh catalog {field} must be a relative non-traversal path")
    resolved_base = base.resolve()
    resolved = (resolved_base / candidate).resolve()
    try:
        resolved.relative_to(resolved_base)
    except ValueError as exc:
        raise FoundationError(f"Sinnoh catalog {field} escapes its authority root") from exc
    return resolved


def map_path(root: Path, target_map: str) -> Path:
    if not isinstance(target_map, str) or not target_map:
        raise FoundationError("Sinnoh catalog target_map must be a nonempty path component")
    return contained_path(root, Path("data/maps") / target_map / "map.json", "target_map")


def donor_path(donor_root: Path, relative: Any, field: str) -> Path:
    if not isinstance(relative, str) or not relative:
        raise FoundationError(f"Sinnoh catalog {field} must be a nonempty path")
    return contained_path(donor_root, relative, field)


def root_path(root: Path, relative: Any, field: str) -> Path:
    if not isinstance(relative, str) or not relative:
        raise FoundationError(f"Sinnoh catalog {field} must be a nonempty path")
    return contained_path(root, relative, field)


def write_json(path: Path, value: Any) -> None:
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def manifest_rows(root: Path) -> list[dict[str, Any]]:
    manifest = load_json(root / "src/data/wayfarer_sinnoh_maps.json")
    donor = manifest.get("donor")
    if not isinstance(donor, dict) or donor.get("url") != DONOR_URL or donor.get("commit") != DONOR_COMMIT:
        raise FoundationError("Sinnoh catalog import requires the frozen donor identity")
    rows = manifest.get("maps")
    if not isinstance(rows, list) or len(rows) != EXPECTED_COUNTS["maps"]:
        raise FoundationError("Sinnoh catalog import requires exactly 133 manifest maps")
    if [row.get("order") for row in rows] != list(range(EXPECTED_COUNTS["maps"])):
        raise FoundationError("Sinnoh catalog import requires deterministic manifest ordering")
    return rows


def assets_by_id(root: Path) -> dict[str, dict[str, Any]]:
    records = load_json(root / "src/data/wayfarer_sinnoh_assets.json").get("records")
    if not isinstance(records, list):
        raise FoundationError("Sinnoh asset manifest has no records")
    result = {record.get("record_id"): record for record in records if isinstance(record, dict)}
    if len(result) != len(records):
        raise FoundationError("Sinnoh asset manifest has duplicate or malformed records")
    return result


def transformed_map(source: dict[str, Any], row: dict[str, Any]) -> dict[str, Any]:
    result = dict(source)
    result.update({
        "id": row["target_map_id"],
        "name": row["target_map"],
        "game_version": "sinnoh",
        "layout": row["target_layout"],
        "region_map_section": row["map_properties"]["target_map_section"],
        "region": "REGION_SINNOH",
        "warp_events": effective_warps(row),
        "connections": effective_connections(row)[0] or 0,
    })
    for key in ("object_events", "coord_events", "bg_events"):
        if result.get(key) != []:
            raise FoundationError(f"Sinnoh source map {row['source_map']} has nonempty {key}")
    if result.get("map_scripts") not in (None, []):
        raise FoundationError(f"Sinnoh source map {row['source_map']} has nonempty map scripts")
    return result


def asset_path(root: Path, row: dict[str, Any], asset: dict[str, Any], kind: str) -> str:
    if asset.get("reuse_class") == "EXACT_ALIAS":
        candidates = asset.get("candidate_matches")
        owner = asset.get("canonical_owner")
        candidate = next((item for item in candidates if item.get("layout") == owner), None) if isinstance(candidates, list) else None
        if not isinstance(candidate, dict) or not isinstance(candidate.get("path"), str):
            raise FoundationError(f"Sinnoh exact alias {asset.get('record_id')} has no canonical path")
        root_path(root, candidate["path"], f"exact-alias candidate path for {asset.get('record_id')}")
        return candidate["path"]
    suffix = "map.bin" if kind == "blockdata" else "border.bin"
    target = f"data/layouts/sinnoh/{row['target_map']}/{suffix}"
    root_path(root, target, f"generated {kind} path")
    return target


def transformed_layout(root: Path, source: dict[str, Any], row: dict[str, Any], assets: dict[str, dict[str, Any]]) -> dict[str, Any]:
    block = assets.get(row["asset_records"]["blockdata"])
    border = assets.get(row["asset_records"]["border"])
    if not isinstance(block, dict) or not isinstance(border, dict):
        raise FoundationError(f"Sinnoh layout {row['target_layout']} lacks payload authority")
    result = dict(source)
    result.update({
        "id": row["target_layout"],
        "game_version": "sinnoh",
        "layout_version": "emerald",
        "primary_tileset": row["tilesets"]["proposed_target_primary"],
        "secondary_tileset": row["tilesets"]["proposed_target_secondary"],
        "blockdata_filepath": asset_path(root, row, block, "blockdata"),
        "border_filepath": asset_path(root, row, border, "border"),
    })
    return result


def copy_payload(root: Path, donor_root: Path, row: dict[str, Any], asset: dict[str, Any], kind: str, apply: bool) -> None:
    if asset.get("reuse_class") == "EXACT_ALIAS":
        return
    source = donor_path(donor_root, asset.get("source_path"), f"{kind} source_path")
    target = root_path(root, asset_path(root, row, asset, kind), f"checked-in {kind} path")
    if not source.is_file() or sha256_file(source) != asset.get("source_sha256"):
        raise FoundationError(f"Sinnoh donor {kind} hash drift for {row['source_map']}")
    if apply:
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, target)
    if not target.is_file() or sha256_file(target) != asset.get("source_sha256"):
        raise FoundationError(f"Sinnoh checked-in {kind} hash drift for {row['source_map']}")


def expected_sections(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    seen: set[str] = set()
    for row in rows:
        properties = row["map_properties"]
        section = properties["target_map_section"]
        if section in seen:
            continue
        seen.add(section)
        result.append({
            "id": section,
            "name": properties["target_map_section_label"].upper(),
            "wayfarer_sinnoh": True,
        })
    return result


def validate_path_contract(root: Path, donor_root: Path, rows: list[dict[str, Any]], assets: dict[str, dict[str, Any]]) -> None:
    """Preflight every external path before --apply can mutate catalog files."""
    for row in rows:
        map_path(root, row.get("target_map"))
        source_paths = row.get("source_paths")
        if not isinstance(source_paths, dict):
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has no source paths")
        donor_path(donor_root, source_paths.get("map_json"), "map_json source path")
        refs = row.get("asset_records")
        if not isinstance(refs, dict):
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has no asset references")
        for kind in ("blockdata", "border"):
            asset = assets.get(refs.get(kind))
            if not isinstance(asset, dict):
                raise FoundationError(f"Sinnoh map {row.get('source_map')} lacks {kind} asset authority")
            donor_path(donor_root, asset.get("source_path"), f"{kind} source_path")
            root_path(root, asset_path(root, row, asset, kind), f"checked-in {kind} path")


def check_or_write_catalog(root: Path, donor_root: Path, apply: bool) -> None:
    rows = manifest_rows(root)
    assets = assets_by_id(root)
    validate_path_contract(root, donor_root, rows, assets)
    donor_layouts = load_json(donor_root / "data/layouts/layouts.json").get("layouts")
    if not isinstance(donor_layouts, list):
        raise FoundationError("Sinnoh donor layout catalog is malformed")
    donor_layout_by_id = {layout.get("id"): layout for layout in donor_layouts if isinstance(layout, dict)}

    map_groups_path = root / "data/maps/map_groups.json"
    groups = load_json(map_groups_path)
    expected_groups = [group["target_group"] for group in load_json(root / "src/data/wayfarer_sinnoh_maps.json")["source_groups"]]
    existing_order = groups.get("group_order")
    if not isinstance(existing_order, list):
        raise FoundationError("Wayfarer map group catalog is malformed")
    old_order = [group for group in existing_order if group not in expected_groups]
    if existing_order not in (old_order, old_order + expected_groups):
        raise FoundationError("Sinnoh map groups must be appended as one contiguous catalog")
    expected_members = {group: [] for group in expected_groups}
    for row in rows:
        expected_members[row["source_group"]].append(row["target_map"])
    if apply:
        groups["group_order"] = old_order + expected_groups
        for group, members in expected_members.items():
            groups[group] = members
        write_json(map_groups_path, groups)
    elif existing_order != old_order + expected_groups or any(groups.get(group) != members for group, members in expected_members.items()):
        raise FoundationError("checked-in Sinnoh map groups differ from the frozen manifest")

    layouts_path = root / "data/layouts/layouts.json"
    layouts_data = load_json(layouts_path)
    layouts = layouts_data.get("layouts")
    if not isinstance(layouts, list):
        raise FoundationError("Wayfarer layout catalog is malformed")
    expected_layouts: list[dict[str, Any]] = []
    for row in rows:
        donor_layout = donor_layout_by_id.get(row["source_layout"])
        if not isinstance(donor_layout, dict):
            raise FoundationError(f"Sinnoh donor layout is missing: {row['source_layout']}")
        expected_layouts.append(transformed_layout(root, donor_layout, row, assets))
    target_layouts = {row["target_layout"] for row in rows}
    old_layouts = [layout for layout in layouts if layout.get("id") not in target_layouts]
    actual_sinnoh_layouts = [layout for layout in layouts if layout.get("id") in target_layouts]
    if apply:
        layouts_data["layouts"] = old_layouts + expected_layouts
        write_json(layouts_path, layouts_data)
    elif actual_sinnoh_layouts != expected_layouts or layouts != old_layouts + expected_layouts:
        raise FoundationError("checked-in Sinnoh layouts differ from the frozen manifest")

    sections_path = root / "src/data/region_map/region_map_sections.json"
    sections_data = load_json(sections_path)
    sections = sections_data.get("map_sections")
    if not isinstance(sections, list):
        raise FoundationError("Wayfarer map-section catalog is malformed")
    expected = expected_sections(rows)
    target_sections = {section["id"] for section in expected}
    old_sections = [section for section in sections if section.get("id") not in target_sections]
    actual_sections = [section for section in sections if section.get("id") in target_sections]
    if apply:
        sections_data["map_sections"] = old_sections + expected
        write_json(sections_path, sections_data)
    elif actual_sections != expected or sections != old_sections + expected:
        raise FoundationError("checked-in Sinnoh map sections differ from the frozen manifest")

    for row in rows:
        source_path = donor_path(donor_root, row["source_paths"]["map_json"], "map_json source path")
        source = load_json(source_path)
        if sha256_file(source_path) != row["source_hashes"]["map_json"]:
            raise FoundationError(f"Sinnoh donor map hash drift for {row['source_map']}")
        expected_map = transformed_map(source, row)
        path = map_path(root, row["target_map"])
        if apply:
            path.parent.mkdir(parents=True, exist_ok=True)
            write_json(path, expected_map)
        elif load_json(path) != expected_map:
            raise FoundationError(f"checked-in Sinnoh map differs from frozen donor: {row['source_map']}")
        copy_payload(root, donor_root, row, assets[row["asset_records"]["blockdata"]], "blockdata", apply)
        copy_payload(root, donor_root, row, assets[row["asset_records"]["border"]], "border", apply)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--donor-root", type=Path, required=True)
    parser.add_argument("--apply", action="store_true")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    if args.apply == args.check:
        parser.error("select exactly one of --apply or --check")
    check_or_write_catalog(args.root.resolve(), args.donor_root.resolve(), args.apply)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
