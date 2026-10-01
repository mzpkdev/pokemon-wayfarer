#!/usr/bin/env python3
"""Audit checked-in Sinnoh foundation authorities without fetching a donor."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from foundation import (DONOR_COMMIT, DONOR_URL, EXPECTED_COUNTS, GROUPS,
                        FoundationError, load_json, selected_source)
from topology import effective_connections, effective_warps, validate_topology


def validate_identity(maps: dict[str, Any]) -> None:
    donor = maps.get("donor")
    if donor != {"url": DONOR_URL, "commit": DONOR_COMMIT} and not (isinstance(donor, dict) and donor.get("url") == DONOR_URL and donor.get("commit") == DONOR_COMMIT):
        raise FoundationError("manifest donor identity is not the frozen Sinnoh commit")


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
    # Preserve the donor flag while allowing an explicit Wayfarer presentation choice.
    show_map_name = properties.get("target_show_map_name", properties["show_map_name"])
    if not isinstance(show_map_name, bool):
        raise FoundationError(f"Sinnoh map {row['target_map']} has an invalid popup flag")
    return {
        "id": row["target_map_id"], "name": row["target_map"], "game_version": "sinnoh",
        "layout": row["target_layout"], "region": "REGION_SINNOH",
        "region_map_section": properties["target_map_section"],
        "music": properties["music"], "weather": properties["weather"],
        "map_type": properties["map_type"], "requires_flash": properties["requires_flash"],
        "allow_cycling": properties["allow_cycling"], "allow_escaping": properties["allow_escaping"],
        "allow_running": properties["allow_running"], "show_map_name": show_map_name,
        "battle_scene": properties["battle_scene"], "warp_events": effective_warps(row),
        # The frozen donor represents no connections as numeric 0; populated
        # rows preserve their explicit connection vectors.
        "connections": effective_connections(row)[0] or 0,
        "object_events": [], "coord_events": [], "bg_events": [],
    }


def require_exact_projection(actual: dict[str, Any], expected: dict[str, Any], kind: str, name: str) -> None:
    changed = sorted(key for key in set(actual) | set(expected) if actual.get(key) != expected.get(key))
    if changed:
        raise FoundationError(f"Sinnoh {kind} projection drifted: {name} ({', '.join(changed)})")


def expected_imported_layout(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "id": row["target_layout"], "name": f"{row['target_map']}_Layout",
        "width": row["dimensions"]["width"], "height": row["dimensions"]["height"],
        "primary_tileset": row["tilesets"]["proposed_target_primary"],
        "secondary_tileset": row["tilesets"]["proposed_target_secondary"],
        "game_version": "sinnoh", "layout_version": "emerald",
    }


def validate_imported_catalog(root: Path, rows: list[dict[str, Any]]) -> dict[str, Any]:
    """Verify the checked-in catalog and its effective topology."""

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
        expected = expected_imported_layout(row)
        require_exact_projection({key: layout.get(key) for key in expected}, expected,
                                 "layout", row["target_layout"])
        for kind, field in (("blockdata", "blockdata_filepath"),
                            ("border", "border_filepath")):
            path = layout.get(field)
            if not isinstance(path, str):
                raise FoundationError(f"Sinnoh layout {kind} path is missing: {row['target_layout']}")
            if not (root / path).is_file():
                raise FoundationError(f"Sinnoh layout payload is missing: {path}")

    sections = load_json(root / "src/data/region_map/region_map_sections.json").get("map_sections", [])
    expected_map_sections = []
    seen_sections: set[str] = set()
    for row in rows:
        properties = row["map_properties"]
        section = properties["target_map_section"]
        if section not in seen_sections:
            seen_sections.add(section)
            expected_map_sections.append({"id": section, "name": properties["target_map_section_label"].upper(), "wayfarer_sinnoh": True})
    if sections[-len(expected_map_sections):] != expected_map_sections:
        raise FoundationError("Sinnoh map sections are not appended in frozen manifest order")

    for row in rows:
        path = root / "data/maps" / row["target_map"] / "map.json"
        imported = load_json(path)
        require_exact_projection(imported, expected_imported_map(row), "map", row["target_map"])

    topology = validate_topology(rows)
    return {"verified": True, "map_count": len(rows), "layout_count": len(imported_layouts),
            "map_section_count": len(expected_map_sections),
            "topology_verdict": "STRUCTURALLY_VERIFIED",
            "topology": topology}


def validate_checked_in(root: Path, maps: dict[str, Any]) -> list[dict[str, Any]]:
    validate_identity(maps)
    if maps.get("schema_version") != 1:
        raise FoundationError("map manifest schema_version must be 1")
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
    seen_maps: set[str] = set()
    seen_targets: set[str] = set()
    seen_layout_targets: set[str] = set()
    totals = {key: 0 for key in ("warps", "connections", "object_events", "coord_events", "bg_events", "nonempty_map_scripts", "wild_encounter_profiles")}
    for order, row in enumerate(rows):
        required = {"order", "source_group", "source_group_order", "source_map", "source_map_id", "target_map_id", "source_layout", "target_layout", "dimensions", "layout_format", "tilesets", "map_properties", "warps", "connections", "empty_content"}
        if not isinstance(row, dict) or required - row.keys():
            raise FoundationError(f"map row {order} is missing required frozen fields")
        if row["order"] != order or row["source_map"] in seen_maps:
            raise FoundationError("map manifest order or source identity is not deterministic")
        seen_maps.add(row["source_map"])
        if row["target_map_id"] in seen_targets or row["target_layout"] in seen_layout_targets:
            raise FoundationError("proposed target map or layout symbol is duplicated")
        seen_targets.add(row["target_map_id"])
        seen_layout_targets.add(row["target_layout"])
        if row["layout_format"] != "emerald" or row["map_properties"].get("region") != "sinnoh":
            raise FoundationError(f"map {row['source_map']} lacks explicit Sinnoh Emerald provenance")
        effective_warps(row)
        effective_connections(row)
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
    if totals != {key: EXPECTED_COUNTS[key] for key in totals}:
        raise FoundationError(f"frozen map counts drifted: {totals}")
    return rows


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


def validate_donor(donor_root: Path, maps: list[dict[str, Any]]) -> dict[str, int]:
    group_rows, source_maps, layouts = selected_source(donor_root)
    if [row["source_map"] for row in maps] != [name for _, _, name in group_rows]:
        raise FoundationError("donor map ordering or membership differs from the frozen manifest")
    observed = donor_empty_counts(donor_root, source_maps)
    require_expected_empty_counts(observed)
    for row, (group, group_order, name) in zip(maps, group_rows, strict=True):
        source = source_maps[name]
        layout = layouts[source["layout"]]
        if row["source_group"] != group or row["source_group_order"] != group_order:
            raise FoundationError(f"donor source group drift for {name}")
        if row["source_map_id"] != source["id"] or row["source_layout"] != layout["id"] or row["dimensions"] != {"width": layout["width"], "height": layout["height"]}:
            raise FoundationError(f"donor identity or layout shape drift for {name}")
        if row["warps"] != (source.get("warp_events") or []) or row["connections"] != (source.get("connections") or []):
            raise FoundationError(f"donor topology drift for {name}")
        properties = row["map_properties"]
        if any(properties.get(key) != source.get(key) for key in ("music", "weather", "map_type", "requires_flash", "allow_cycling", "allow_escaping", "allow_running", "show_map_name", "battle_scene")) or properties.get("source_map_section") != source.get("region_map_section"):
            raise FoundationError(f"donor field-property drift for {name}")
    return observed


def build_report(root: Path, maps_path: Path, donor_root: Path | None = None) -> dict[str, Any]:
    maps = load_json(maps_path)
    rows = validate_checked_in(root, maps)
    porymap_contract = validate_porymap_contract(root, rows)
    catalog = validate_imported_catalog(root, rows)
    donor_counts = validate_donor(donor_root, rows) if donor_root is not None else None
    return {
        "schema_version": 1, "donor": maps["donor"],
        "manifest_integrity": {"verified": True, "selected_map_count": len(rows), "selected_layout_count": len({row["source_layout"] for row in rows}),
                                "frozen_manifest_counts": {"warps": sum(len(row["warps"]) for row in rows), "connections": sum(len(row["connections"]) for row in rows),
                                                           "object_events": 0, "coord_events": 0, "bg_events": 0, "nonempty_map_scripts": 0, "wild_encounter_profiles": 0},
                                "porymap": porymap_contract, "catalog": catalog},
        "donor_source_verification": {"performed": donor_root is not None, "hashes_verified": donor_root is not None,
                                        "observed_empty_content_counts": donor_counts},
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--maps", type=Path, default=Path("src/data/wayfarer_sinnoh_maps.json"))
    parser.add_argument("--donor-root", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    report = build_report(root, (root / args.maps).resolve(),
                          args.donor_root.resolve() if args.donor_root else None)
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
