#!/usr/bin/env python3
"""Audit checked-in Sinnoh foundation authorities without fetching a donor."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from foundation import (DONOR_COMMIT, DONOR_URL, EXPECTED_COUNTS, GROUPS, REUSE_CLASSES,
                        FoundationError, canonical_json, current_symbols, load_json, selected_source,
                        sha256_bytes, sha256_file, source_hashes, source_tree_hashes, target_symbol,
                        tileset_path)


def records_by_id(manifest: dict[str, Any]) -> dict[str, dict[str, Any]]:
    rows = manifest.get("records")
    if manifest.get("schema_version") != 1 or not isinstance(rows, list):
        raise FoundationError("asset manifest schema_version must be 1 and records must be a list")
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        if not isinstance(row, dict) or not isinstance(row.get("record_id"), str):
            raise FoundationError("asset record must have a string record_id")
        if row["record_id"] in result:
            raise FoundationError(f"asset manifest repeats record {row['record_id']}")
        if row.get("reuse_class") not in REUSE_CLASSES:
            raise FoundationError(f"asset {row['record_id']} has an invalid reuse_class")
        if row.get("asset_family") == "tileset_component":
            required = {"source_symbol", "proposed_target_symbol", "component", "source_path", "source_sha256", "source_bytes", "generated_path", "generated_symbol", "generated_sha256", "decoded_sha256", "decoded_bytes", "array_shape", "alignment", "compression", "linkage", "visibility", "output_section"}
            if required - row.keys():
                raise FoundationError(f"tileset component {row['record_id']} lacks source/generation contract fields")
            if not row["generated_sha256"] or not row["decoded_sha256"] or not isinstance(row["decoded_bytes"], int):
                raise FoundationError(f"tileset component {row['record_id']} lacks generated and decoded production proof")
        result[row["record_id"]] = row
    return result


def validate_identity(maps: dict[str, Any], assets: dict[str, Any]) -> None:
    for manifest in (maps, assets):
        donor = manifest.get("donor")
        if donor != {"url": DONOR_URL, "commit": DONOR_COMMIT} and not (isinstance(donor, dict) and donor.get("url") == DONOR_URL and donor.get("commit") == DONOR_COMMIT):
            raise FoundationError("manifest donor identity is not the frozen Sinnoh commit")
    baseline = assets.get("wayfarer_baseline", {}).get("commit")
    if not isinstance(baseline, str) or len(baseline) != 40:
        raise FoundationError("asset manifest has no exact actual Wayfarer baseline commit")
    if assets.get("implementation_head") != "d26a524d66a2772e81efe7a3838b5702f7960719":
        raise FoundationError("asset manifest implementation head drifted")


def validate_porymap_contract(root: Path, rows: list[dict[str, Any]]) -> dict[str, Any]:
    project = load_json(root / "porymap.project.json")
    if project.get("base_game_version") != "pokeemerald":
        raise FoundationError("Porymap project must use base_game_version pokeemerald")
    if any(row.get("layout_format") != "emerald" for row in rows):
        raise FoundationError("frozen manifest rows must declare layout_format emerald")
    layouts = load_json(root / "data/layouts/layouts.json").get("layouts", [])
    by_id = {
        layout.get("id"): layout for layout in layouts
        if isinstance(layout, dict) and isinstance(layout.get("id"), str)
    }
    imported_targets = [row["target_layout"] for row in rows]
    if any(target not in by_id for target in imported_targets):
        raise FoundationError("Sinnoh imported target layouts are incomplete")
    for row in rows:
        layout = by_id[row["target_layout"]]
        if layout.get("game_version") != "sinnoh" or layout.get("layout_version") != "emerald":
            raise FoundationError(f"Sinnoh imported layout provenance drifted: {row['target_layout']}")
    return {
        "base_game_version": {"value": "pokeemerald", "verified": True},
        "frozen_manifest_layout_format": {
            "value": "emerald", "verified": True, "map_count": len(rows),
        },
        "imported_layout_verification": {
            "verified": True, "layout_count": len(imported_targets),
        },
    }


def expected_imported_map(row: dict[str, Any]) -> dict[str, Any]:
    properties = row["map_properties"]
    return {
        "id": row["target_map_id"], "name": row["target_map"], "game_version": "sinnoh",
        "layout": row["target_layout"], "region": "REGION_SINNOH",
        "region_map_section": properties["target_map_section"],
        "music": properties["music"], "weather": properties["weather"],
        "map_type": properties["map_type"], "requires_flash": properties["requires_flash"],
        "allow_cycling": properties["allow_cycling"], "allow_escaping": properties["allow_escaping"],
        "allow_running": properties["allow_running"], "show_map_name": properties["show_map_name"],
        "battle_scene": properties["battle_scene"], "warp_events": row["warps"],
        # The frozen donor represents no connections as numeric 0; populated
        # rows preserve their explicit connection vectors.
        "connections": row["connections"] if row["connections"] else 0,
        "object_events": [], "coord_events": [], "bg_events": [],
    }


def require_exact_projection(actual: dict[str, Any], expected: dict[str, Any], kind: str, name: str) -> None:
    changed = sorted(key for key in set(actual) | set(expected) if actual.get(key) != expected.get(key))
    if changed:
        raise FoundationError(f"Sinnoh {kind} projection drifted: {name} ({', '.join(changed)})")


def expected_imported_layout(root: Path, row: dict[str, Any], assets: dict[str, dict[str, Any]]) -> dict[str, Any]:
    from import_catalog import asset_path

    return {
        "id": row["target_layout"], "name": f"{row['target_map']}_Layout",
        "width": row["dimensions"]["width"], "height": row["dimensions"]["height"],
        "primary_tileset": row["tilesets"]["proposed_target_primary"],
        "secondary_tileset": row["tilesets"]["proposed_target_secondary"],
        "blockdata_filepath": asset_path(root, row, assets[row["asset_records"]["blockdata"]], "blockdata"),
        "border_filepath": asset_path(root, row, assets[row["asset_records"]["border"]], "border"),
        "game_version": "sinnoh", "layout_version": "emerald",
    }


def validate_imported_catalog(root: Path, rows: list[dict[str, Any]], assets: dict[str, dict[str, Any]]) -> dict[str, Any]:
    """Verify the pending catalog is authored but cannot be runtime-selected yet."""
    from import_catalog import expected_sections

    if any(row["inclusion"]["state"] != "FROZEN_NOT_SELECTED" for row in rows):
        raise FoundationError("imported Sinnoh catalog must remain pending topology selection")
    if any(row["topology"] != {"state": "FROZEN_SOURCE_PENDING_REVIEW", "repair": None} for row in rows):
        raise FoundationError("imported Sinnoh catalog topology is not frozen pending review")

    groups = load_json(root / "data/maps/map_groups.json")
    expected_groups = list(GROUPS)
    if groups.get("group_order", [])[-len(expected_groups):] != expected_groups:
        raise FoundationError("Sinnoh map groups are not appended contiguously")
    expected_members = {group: [] for group in expected_groups}
    for row in rows:
        expected_members[row["source_group"]].append(row["target_map"])
    if any(groups.get(group) != members for group, members in expected_members.items()):
        raise FoundationError("Sinnoh map-group membership differs from the frozen manifest")

    layouts = load_json(root / "data/layouts/layouts.json").get("layouts", [])
    target_layouts = [row["target_layout"] for row in rows]
    imported_layouts = [layout for layout in layouts if isinstance(layout, dict) and layout.get("id") in set(target_layouts)]
    if [layout.get("id") for layout in imported_layouts] != target_layouts or layouts[-len(rows):] != imported_layouts:
        raise FoundationError("Sinnoh layouts are not appended in frozen manifest order")
    for row, layout in zip(rows, imported_layouts, strict=True):
        expected = expected_imported_layout(root, row, assets)
        require_exact_projection(layout, expected, "layout", row["target_layout"])
        for kind, path in (("blockdata", expected["blockdata_filepath"]),
                           ("border", expected["border_filepath"])):
            if not (root / path).is_file():
                raise FoundationError(f"Sinnoh layout payload is missing: {path}")
            asset = assets[row["asset_records"][kind]]
            if asset.get("reuse_class") != "EXACT_ALIAS" and sha256_file(root / path) != asset.get("source_sha256"):
                raise FoundationError(f"Sinnoh layout payload hash drifted: {path}")

    sections = load_json(root / "src/data/region_map/region_map_sections.json").get("map_sections", [])
    expected_map_sections = expected_sections(rows)
    if sections[-len(expected_map_sections):] != expected_map_sections:
        raise FoundationError("Sinnoh map sections are not appended in frozen manifest order")

    for row in rows:
        path = root / "data/maps" / row["target_map"] / "map.json"
        imported = load_json(path)
        require_exact_projection(imported, expected_imported_map(row), "map", row["target_map"])

    return {"verified": True, "map_count": len(rows), "layout_count": len(imported_layouts),
            "map_section_count": len(expected_map_sections), "release_link_enabled": False}


def validate_checked_in(root: Path, maps: dict[str, Any], assets: dict[str, Any]) -> tuple[list[dict[str, Any]], dict[str, dict[str, Any]]]:
    validate_identity(maps, assets)
    if maps.get("schema_version") != 1:
        raise FoundationError("map manifest schema_version must be 1")
    selection = maps.get("selection")
    asset_selection = assets.get("selection")
    if not isinstance(selection, dict) or not isinstance(asset_selection, dict):
        raise FoundationError("both manifests require selection metadata")
    if selection.get("release_link_enabled") != asset_selection.get("release_link_enabled"):
        raise FoundationError("map and asset release selection gates disagree")
    if selection.get("asset_manifest_ready") != asset_selection.get("asset_manifest_ready"):
        raise FoundationError("map and asset readiness gates disagree")
    if selection.get("release_link_enabled") or selection.get("asset_manifest_ready"):
        raise FoundationError("pending topology catalog must keep release selection closed")
    if selection.get("allowed_inclusion_states") != ["FROZEN_NOT_SELECTED", "INCLUDED", "EXCLUDED"]:
        raise FoundationError("map manifest has invalid inclusion-state contract")
    expected_groups = [{"source_group": group, "target_group": group, "order": order} for order, group in enumerate(GROUPS)]
    actual_groups = maps.get("source_groups")
    if not isinstance(actual_groups, list) or [{key: row.get(key) for key in ("source_group", "target_group", "order")} for row in actual_groups] != expected_groups:
        raise FoundationError("source group ordering or target identity changed")
    expected_group_counts = {group: 0 for group in GROUPS}
    for row in maps.get("maps", []):
        if isinstance(row, dict) and row.get("source_group") in expected_group_counts:
            expected_group_counts[row["source_group"]] += 1
    if any(row.get("map_count") != expected_group_counts[row["source_group"]] for row in actual_groups):
        raise FoundationError("source group count drifted")
    if maps.get("expected_counts") != EXPECTED_COUNTS:
        raise FoundationError("map manifest expected counts drifted")
    rows = maps.get("maps")
    if not isinstance(rows, list) or len(rows) != EXPECTED_COUNTS["maps"]:
        raise FoundationError("map manifest must contain exactly 133 maps")
    asset_rows = records_by_id(assets)
    map_ids, layout_ids = current_symbols(root)
    # Imported symbols are intentional appended declarations, not pre-existing
    # collisions that would require a second SINNOH rename.
    declared_map_targets = {row.get("target_map_id") for row in rows if isinstance(row, dict)}
    declared_layout_targets = {row.get("target_layout") for row in rows if isinstance(row, dict)}
    map_ids -= declared_map_targets
    layout_ids -= declared_layout_targets
    seen_maps: set[str] = set()
    seen_targets: set[str] = set()
    seen_layout_targets: set[str] = set()
    totals = {key: 0 for key in ("warps", "connections", "object_events", "coord_events", "bg_events", "nonempty_map_scripts", "wild_encounter_profiles")}
    for order, row in enumerate(rows):
        required = {"record_id", "order", "source_group", "source_group_order", "source_map", "source_map_id", "target_map_id", "source_layout", "target_layout", "renames", "source_paths", "source_hashes", "dimensions", "layout_format", "tilesets", "map_properties", "warps", "connections", "empty_content", "asset_records", "topology", "inclusion"}
        if not isinstance(row, dict) or required - row.keys():
            raise FoundationError(f"map row {order} is missing required frozen fields")
        if row["order"] != order or row["source_map"] in seen_maps:
            raise FoundationError("map manifest order or source identity is not deterministic")
        seen_maps.add(row["source_map"])
        if row["target_map_id"] in seen_targets or row["target_layout"] in seen_layout_targets:
            raise FoundationError("proposed target map or layout symbol is duplicated")
        seen_targets.add(row["target_map_id"])
        seen_layout_targets.add(row["target_layout"])
        expected_map_target, expected_map_rename = target_symbol(row["source_map_id"], map_ids)
        expected_layout_target, expected_layout_rename = target_symbol(row["source_layout"], layout_ids)
        if row["target_map_id"] != expected_map_target or row["target_layout"] != expected_layout_target or row["renames"] != {"map": expected_map_rename, "layout": expected_layout_rename}:
            raise FoundationError(f"map {row['source_map']} has an unrecorded or non-minimal target collision rename")
        if row["layout_format"] != "emerald" or row["map_properties"].get("region") != "sinnoh":
            raise FoundationError(f"map {row['source_map']} lacks explicit Sinnoh Emerald provenance")
        inclusion = row["inclusion"]
        if inclusion.get("state") not in selection["allowed_inclusion_states"] or not isinstance(inclusion.get("reason"), str):
            raise FoundationError(f"map {row['source_map']} has invalid inclusion metadata")
        if row["topology"] != {"state": "FROZEN_SOURCE_PENDING_REVIEW", "repair": None}:
            raise FoundationError(f"map {row['source_map']} has an unreviewed topology change")
        if row["empty_content"] != {"object_events": 0, "coord_events": 0, "bg_events": 0, "map_scripts": 0, "wild_encounter_profiles": 0}:
            raise FoundationError(f"map {row['source_map']} violates frozen empty-content policy")
        for key in ("warps", "connections"):
            if not isinstance(row[key], list):
                raise FoundationError(f"map {row['source_map']} {key} must be a list")
            totals[key] += len(row[key])
        for key in ("object_events", "coord_events", "bg_events"):
            totals[key] += row["empty_content"][key]
        totals["nonempty_map_scripts"] += row["empty_content"]["map_scripts"]
        totals["wild_encounter_profiles"] += row["empty_content"]["wild_encounter_profiles"]
        refs = row["asset_records"]
        tile_refs = refs.get("tilesets")
        expected_refs = {refs.get("blockdata"), refs.get("border"), *(tile_refs if isinstance(tile_refs, list) else [])}
        if not isinstance(tile_refs, list) or not tile_refs or len(expected_refs) != len(tile_refs) + 2 or any(reference not in asset_rows for reference in expected_refs):
            raise FoundationError(f"map {row['source_map']} has a missing or malformed asset reference")
        if any(asset_rows[reference].get("asset_family") != "tileset_component" for reference in tile_refs):
            raise FoundationError(f"map {row['source_map']} tileset closure is not concrete component records")
    if totals != {key: EXPECTED_COUNTS[key] for key in totals}:
        raise FoundationError(f"frozen map counts drifted: {totals}")
    return rows, asset_rows


def validate_exact_worktree_assets(root: Path, assets: dict[str, dict[str, Any]]) -> None:
    layouts = load_json(root / "data/layouts/layouts.json").get("layouts", [])
    layout_by_id = {row.get("id"): row for row in layouts if isinstance(row, dict) and isinstance(row.get("id"), str)}
    for record in assets.values():
        if record.get("asset_family") not in {"blockdata", "border"} or record.get("reuse_class") != "EXACT_ALIAS":
            continue
        candidates = record.get("candidate_matches")
        if not isinstance(candidates, list) or not candidates:
            raise FoundationError(f"exact alias {record['record_id']} has no canonical candidate")
        canonical = next((row for row in candidates if row.get("layout") == record.get("canonical_owner")), None)
        if canonical is None or not canonical.get("runtime_meaning_proven"):
            raise FoundationError(f"exact alias {record['record_id']} lacks runtime-meaning proof")
        layout = layout_by_id.get(canonical["layout"])
        key = "blockdata_filepath" if record["asset_family"] == "blockdata" else "border_filepath"
        if layout is None or layout.get(key) != canonical.get("path"):
            raise FoundationError(f"exact alias {record['record_id']} canonical layout/path drifted")
        path = root / canonical["path"]
        shape = record.get("shape")
        if not isinstance(shape, dict) or canonical.get("source_shape") != shape or canonical.get("candidate_shape") != shape:
            raise FoundationError(f"exact alias {record['record_id']} lacks matching frozen payload shapes")
        if record["asset_family"] == "blockdata":
            expected_shape = {"width": layout.get("width"), "height": layout.get("height"), "element_bytes": 2,
                              "byte_length": (layout.get("width") or 0) * (layout.get("height") or 0) * 2}
        else:
            expected_shape = {"tile_count": 4, "element_bytes": 2, "byte_length": 8}
        if shape != expected_shape or record.get("source_bytes") != shape["byte_length"]:
            raise FoundationError(f"exact alias {record['record_id']} layout shape drifted")
        if not path.is_file() or path.stat().st_size != shape["byte_length"] or sha256_file(path) != record.get("source_sha256"):
            raise FoundationError(f"exact alias {record['record_id']} drifted from its current-Wayfarer candidate")


def donor_empty_counts(donor_root: Path, source_maps: dict[str, dict[str, Any]]) -> dict[str, int]:
    """Inspect donor event/script/encounter content; manifest assertions are not proof."""
    counts = {"object_events": 0, "coord_events": 0, "bg_events": 0, "nonempty_map_scripts": 0, "wild_encounter_profiles": 0}
    selected_ids = {source["id"] for source in source_maps.values()}
    for name, source in source_maps.items():
        for key in ("object_events", "coord_events", "bg_events"):
            value = source.get(key) or []
            if not isinstance(value, list):
                raise FoundationError(f"donor {name} {key} is not a list")
            counts[key] += len(value)
        scripts = source.get("map_scripts") or source.get("map_script") or []
        if scripts:
            counts["nonempty_map_scripts"] += 1
    wild_path = donor_root / "src/data/wild_encounters.json"
    wild = load_json(wild_path)
    groups = wild.get("wild_encounter_groups")
    if not isinstance(groups, list):
        raise FoundationError("donor wild encounter catalog is malformed")
    for group in groups:
        if not isinstance(group, dict):
            raise FoundationError("donor wild encounter group is malformed")
        encounters = group.get("encounters", [])
        if not isinstance(encounters, list):
            raise FoundationError("donor wild encounter group encounters is malformed")
        counts["wild_encounter_profiles"] += sum(1 for entry in encounters if isinstance(entry, dict) and entry.get("map") in selected_ids)
    return counts


def require_expected_empty_counts(observed: dict[str, int]) -> None:
    expected_empty = {key: EXPECTED_COUNTS[key] for key in observed}
    if observed != expected_empty:
        raise FoundationError(f"donor frozen empty-content facts drifted: {observed}")


def validate_donor(donor_root: Path, maps: list[dict[str, Any]], assets: dict[str, dict[str, Any]]) -> dict[str, int]:
    group_rows, source_maps, layouts = selected_source(donor_root)
    if [row["source_map"] for row in maps] != [name for _, _, name in group_rows]:
        raise FoundationError("donor map ordering or membership differs from the frozen manifest")
    observed = donor_empty_counts(donor_root, source_maps)
    require_expected_empty_counts(observed)
    for row, (group, group_order, name) in zip(maps, group_rows, strict=True):
        source = source_maps[name]
        layout = layouts[source["layout"]]
        paths, hashes = source_hashes(donor_root, name, layout)
        if row["source_group"] != group or row["source_group_order"] != group_order or row["source_paths"] != paths or row["source_hashes"] != hashes:
            raise FoundationError(f"donor source path or hash drift for {name}")
        if row["source_map_id"] != source["id"] or row["source_layout"] != layout["id"] or row["dimensions"] != {"width": layout["width"], "height": layout["height"]}:
            raise FoundationError(f"donor identity or layout shape drift for {name}")
        if row["warps"] != (source.get("warp_events") or []) or row["connections"] != (source.get("connections") or []):
            raise FoundationError(f"donor topology drift for {name}")
        properties = row["map_properties"]
        if any(properties.get(key) != source.get(key) for key in ("music", "weather", "map_type", "requires_flash", "allow_cycling", "allow_escaping", "allow_running", "show_map_name", "battle_scene")) or properties.get("source_map_section") != source.get("region_map_section"):
            raise FoundationError(f"donor field-property drift for {name}")
    for record in assets.values():
        if record.get("asset_family") == "tileset_component":
            source_path = donor_root / record["source_path"]
            if record.get("component") == "descriptor":
                from freeze import header_block
                if sha256_bytes(header_block(donor_root / "src/data/tilesets/headers.h", record["source_symbol"])) != record.get("source_sha256"):
                    raise FoundationError(f"donor tileset descriptor hash drift for {record['record_id']}")
            elif not source_path.is_file() or sha256_file(source_path) != record.get("source_sha256"):
                raise FoundationError(f"donor tileset source hash drift for {record['record_id']}")
        elif record.get("asset_family") in {"blockdata", "border"}:
            source_path = donor_root / record["source_path"]
            if not source_path.is_file() or sha256_file(source_path) != record.get("source_sha256"):
                raise FoundationError(f"donor asset source hash drift for {record['record_id']}")
    return observed


def build_report(root: Path, maps_path: Path, assets_path: Path, donor_root: Path | None = None, release_selection: bool = False) -> dict[str, Any]:
    maps = load_json(maps_path)
    asset_manifest = load_json(assets_path)
    rows, assets = validate_checked_in(root, maps, asset_manifest)
    porymap_contract = validate_porymap_contract(root, rows)
    catalog = validate_imported_catalog(root, rows, assets)
    validate_exact_worktree_assets(root, assets)
    donor_counts = validate_donor(donor_root, rows, assets) if donor_root is not None else None
    blockers = sorted(record_id for record_id, row in assets.items() if row.get("reuse_class") == "REVIEW_REQUIRED" or row.get("selection_blocker"))
    if maps["selection"].get("blockers") != blockers or asset_manifest["selection"].get("blockers") != blockers:
        raise FoundationError("selection blockers do not exactly represent unresolved assets")
    if release_selection:
        if not maps["selection"].get("release_link_enabled") or not maps["selection"].get("asset_manifest_ready"):
            raise FoundationError("release selection requested before the frozen asset gate is ready")
        if blockers:
            raise FoundationError("release selection has REVIEW_REQUIRED asset rows")
        if any(row["inclusion"]["state"] != "INCLUDED" for row in rows):
            raise FoundationError("release selection has maps that are not INCLUDED")
    return {
        "schema_version": 1, "donor": maps["donor"], "wayfarer_baseline": asset_manifest["wayfarer_baseline"],
        "manifest_integrity": {"verified": True, "selected_map_count": len(rows), "selected_layout_count": len({row["source_layout"] for row in rows}),
                                "frozen_manifest_counts": {"warps": sum(len(row["warps"]) for row in rows), "connections": sum(len(row["connections"]) for row in rows),
                                                           "object_events": 0, "coord_events": 0, "bg_events": 0, "nonempty_map_scripts": 0, "wild_encounter_profiles": 0},
                                "exact_worktree_aliases_verified": sum(1 for row in assets.values() if row.get("reuse_class") == "EXACT_ALIAS"),
                                "porymap": porymap_contract, "catalog": catalog},
        "donor_source_verification": {"performed": donor_root is not None, "hashes_verified": donor_root is not None,
                                        "observed_empty_content_counts": donor_counts},
        "selection": maps["selection"], "asset_record_count": len(assets), "review_required": blockers,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--maps", type=Path, default=Path("src/data/wayfarer_sinnoh_maps.json"))
    parser.add_argument("--assets", type=Path, default=Path("src/data/wayfarer_sinnoh_assets.json"))
    parser.add_argument("--donor-root", type=Path)
    parser.add_argument("--release-selection", action="store_true")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    report = build_report(root, (root / args.maps).resolve(), (root / args.assets).resolve(), args.donor_root.resolve() if args.donor_root else None, args.release_selection)
    payload = json.dumps(report, indent=2, ensure_ascii=False) + "\n"
    if args.output:
        args.output.write_text(payload, encoding="utf-8")
    else:
        print(payload, end="")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except FoundationError as error:
        raise SystemExit(f"Sinnoh foundation audit failed: {error}")
