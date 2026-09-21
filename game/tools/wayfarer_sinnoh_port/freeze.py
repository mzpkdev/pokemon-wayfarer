#!/usr/bin/env python3
"""Freeze the registered donor catalog into source-only Sinnoh authorities.

This command is intentionally offline: callers supply an already checked-out
donor root and the exact Wayfarer baseline they reviewed.  It never clones,
fetches, or copies donor payloads into the worktree.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from foundation import (DONOR_COMMIT, DONOR_URL, EXPECTED_COUNTS, GROUPS,
                        EVENT_KINDS, canonical_json, current_symbols, load_json, rel, section_target,
                        selected_source, sha256_bytes, sha256_file, source_hashes,
                        source_tree_hashes, target_symbol, tileset_path)


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def header_block(path: Path, symbol: str) -> bytes:
    source = path.read_text(encoding="utf-8")
    start = source.find(f"const struct Tileset {symbol} =")
    if start < 0:
        raise ValueError(f"tileset descriptor {symbol} is absent from {path}")
    end = source.find("\n};", start)
    if end < 0:
        raise ValueError(f"tileset descriptor {symbol} is malformed in {path}")
    return source[start:end + 3].replace("\r\n", "\n").encode("utf-8")


def target_layouts(root: Path) -> list[dict[str, Any]]:
    layouts = load_json(root / "data/layouts/layouts.json").get("layouts", [])
    if not isinstance(layouts, list):
        raise ValueError("Wayfarer layouts.json has no layouts list")
    return [row for row in layouts if isinstance(row, dict)]


def exact_candidates(root: Path, source_path: Path, key: str, source_layout: dict[str, Any]) -> list[dict[str, Any]]:
    source_bytes = source_path.read_bytes()
    result: list[dict[str, Any]] = []
    for layout in target_layouts(root):
        candidate_path = layout.get(key)
        if not isinstance(candidate_path, str):
            continue
        absolute = root / candidate_path
        if not absolute.is_file() or absolute.read_bytes() != source_bytes:
            continue
        source_shape = ({"width": source_layout["width"], "height": source_layout["height"], "element_bytes": 2,
                         "byte_length": source_layout["width"] * source_layout["height"] * 2}
                        if key == "blockdata_filepath" else {"tile_count": 4, "element_bytes": 2, "byte_length": 8})
        candidate_shape = ({"width": layout.get("width"), "height": layout.get("height"), "element_bytes": 2,
                            "byte_length": (layout.get("width") or 0) * (layout.get("height") or 0) * 2}
                           if key == "blockdata_filepath" else {"tile_count": 4, "element_bytes": 2, "byte_length": 8})
        runtime_proven = (layout.get("layout_version", "emerald") == "emerald" and len(source_bytes) == source_shape["byte_length"]
                          and source_shape == candidate_shape)
        result.append({"layout": layout["id"], "path": candidate_path,
                       "runtime_meaning_proven": runtime_proven,
                       "source_shape": source_shape, "candidate_shape": candidate_shape,
                       "consumer_tilesets": {"primary": layout.get("primary_tileset"), "secondary": layout.get("secondary_tileset")}})
    return result


def component_id(symbol: str, path: str) -> str:
    return f"tileset.{symbol}.{path.replace('/', '.')}"


def tileset_records(root: Path, donor_root: Path, layouts: list[dict[str, Any]]) -> tuple[dict[str, dict[str, Any]], dict[str, list[str]], dict[str, str]]:
    """Return independently stored component records and a concrete closure."""
    symbols = sorted({layout[key] for layout in layouts for key in ("primary_tileset", "secondary_tileset")})
    donor_headers = donor_root / "src/data/tilesets/headers.h"
    target_headers = root / "src/data/tilesets/headers.h"
    target_header_text = target_headers.read_text(encoding="utf-8") if target_headers.is_file() else ""
    records: dict[str, dict[str, Any]] = {}
    closure: dict[str, list[str]] = {}
    targets: dict[str, str] = {}
    for symbol in symbols:
        source_path = tileset_path(symbol)
        source_files = source_tree_hashes(donor_root, source_path)
        name = symbol.removeprefix("gTileset_")
        target_path = root / source_path
        headers_match = symbol in target_header_text and header_block(target_headers, symbol) == header_block(donor_headers, symbol)
        files_match = target_path.is_dir() and source_tree_hashes(root, source_path) == source_files
        # Source-only milestone: generated and decoded production bytes are not
        # yet checked in, so a matching source tree is evidence, not an alias.
        target_symbol = symbol if not target_path.exists() else f"gTileset_Sinnoh_{name}"
        targets[symbol] = target_symbol
        common = {"asset_family": "tileset_component", "source_symbol": symbol, "proposed_target_symbol": target_symbol,
                  "layout_format": "emerald", "consumers": sorted(layout["id"] for layout in layouts if symbol in (layout["primary_tileset"], layout["secondary_tileset"])),
                  "canonical_owner": None, "expected_linked_delta": "unmeasured-source-only", "linkage": "pending", "visibility": "pending", "output_section": "pending",
                  "proof": {"command": "wayfarer_sinnoh_port.audit --root <worktree>", "version": 1},
                  "generated_path": None, "generated_symbol": None, "generated_sha256": None, "decoded_sha256": None, "decoded_bytes": None}
        component_ids: list[str] = []
        descriptor_id = f"tileset.{symbol}.descriptor"
        descriptor = common | {"record_id": descriptor_id, "component": "descriptor", "source_path": "src/data/tilesets/headers.h",
                               "source_sha256": sha256_bytes(header_block(donor_headers, symbol)), "source_bytes": len(header_block(donor_headers, symbol)),
                               "array_shape": None, "alignment": None, "compression": None}
        descriptor.update(
            reuse_class="REVIEW_REQUIRED",
            selection_blocker=True,
            rationale=(
                "Do not classify or select this source input until generated and decoded "
                "production-byte, shape, linkage, and runtime-meaning verification is reviewed."
            ),
        )
        records[descriptor_id] = descriptor
        component_ids.append(descriptor_id)
        for source_file in source_files:
            relative = Path(source_file["path"]).relative_to(source_path).as_posix()
            extension = Path(relative).suffix
            component = "palette" if relative.startswith("palettes/") else ("graphics" if relative.endswith(".png") else ("metatile_attributes" if relative == "metatile_attributes.bin" else "metatiles"))
            record = common | {"record_id": component_id(symbol, relative), "component": component,
                               "source_path": source_file["path"], "source_sha256": source_file["sha256"], "source_bytes": source_file["bytes"],
                               "array_shape": [16] if component == "palette" else None, "alignment": 2 if component in {"palette", "metatiles", "metatile_attributes"} else 4, "compression": None,
                               "generated_path": None, "generated_symbol": None, "generated_sha256": None, "decoded_sha256": None, "decoded_bytes": None}
            record.update(
                reuse_class="REVIEW_REQUIRED",
                selection_blocker=True,
                rationale=(
                    "Do not classify or select this source input until generated and decoded "
                    "production-byte, shape, linkage, and runtime-meaning verification is reviewed."
                ),
            )
            records[record["record_id"]] = record
            component_ids.append(record["record_id"])
        closure[symbol] = component_ids
    return records, closure, targets


def build_manifests(root: Path, donor_root: Path, baseline_commit: str) -> tuple[dict[str, Any], dict[str, Any]]:
    groups, source_maps, layouts_by_id = selected_source(donor_root)
    current_maps, current_layout_ids = current_symbols(root)
    source_layouts = [layouts_by_id[source_maps[name]["layout"]] for _, _, name in groups]
    tilesets, tileset_closure, tileset_targets = tileset_records(root, donor_root, source_layouts)
    asset_rows: list[dict[str, Any]] = list(tilesets.values())
    maps: list[dict[str, Any]] = []
    layout_catalog_path = donor_root / "data/layouts/layouts.json"
    layout_catalog_hash = sha256_file(layout_catalog_path)
    for order, (group, group_order, name) in enumerate(groups):
        source = source_maps[name]
        layout = layouts_by_id[source["layout"]]
        target_map, map_rename = target_symbol(source["id"], current_maps)
        target_layout, layout_rename = target_symbol(layout["id"], current_layout_ids)
        paths, hashes = source_hashes(donor_root, name, layout)
        section, section_label = section_target(name, source["region_map_section"])
        block_id, border_id = f"blockdata.{layout['id']}", f"border.{layout['id']}"
        asset_refs = {"blockdata": block_id, "border": border_id,
                      "tilesets": tileset_closure[layout["primary_tileset"]] + tileset_closure[layout["secondary_tileset"]]}
        for kind, record_id, path_key in (("blockdata", block_id, "blockdata_filepath"), ("border", border_id, "border_filepath")):
            source_path = donor_root / layout[path_key]
            candidates = exact_candidates(root, source_path, path_key, layout)
            proven = next((row for row in candidates if row["runtime_meaning_proven"]), None)
            asset_rows.append({
                "record_id": record_id, "asset_family": kind, "source_path": layout[path_key],
                "source_sha256": sha256_file(source_path), "source_bytes": source_path.stat().st_size,
                "layout": layout["id"], "layout_format": "emerald",
                "shape": {"width": layout["width"], "height": layout["height"], "element_bytes": 2, "byte_length": layout["width"] * layout["height"] * 2} if kind == "blockdata" else {"tile_count": 4, "element_bytes": 2, "byte_length": 8},
                "consumers": [layout["id"]], "candidate_matches": candidates,
                "reuse_class": "EXACT_ALIAS" if proven else "SINNOH_NEW",
                "canonical_owner": proven["layout"] if proven else None,
                "selection_blocker": False, "expected_linked_delta": "unmeasured-source-only",
                "rationale": "Complete payload bytes, decoded length, Emerald packing, and shape match; layout descriptors and tilesets remain independent." if proven else "No complete current-Wayfarer payload-equivalence proof; keep independent pending a later import.",
                "proof": {"command": "wayfarer_sinnoh_port.audit --root <worktree>", "version": 1},
            })
        maps.append({
            "record_id": f"map.{source['id']}", "order": order, "source_group": group, "source_group_order": group_order,
            "source_map": name, "target_map": name, "source_map_id": source["id"], "target_map_id": target_map,
            "source_layout": layout["id"], "target_layout": target_layout,
            "renames": {"map": map_rename, "layout": layout_rename}, "source_paths": paths, "source_hashes": hashes,
            "dimensions": {"width": layout["width"], "height": layout["height"]}, "layout_format": "emerald",
            "tilesets": {"source_primary": layout["primary_tileset"], "source_secondary": layout["secondary_tileset"],
                         "proposed_target_primary": tileset_targets[layout["primary_tileset"]], "proposed_target_secondary": tileset_targets[layout["secondary_tileset"]]},
            "map_properties": {key: source.get(key) for key in ("music", "weather", "map_type", "requires_flash", "allow_cycling", "allow_escaping", "allow_running", "show_map_name", "battle_scene")}
                              | {"source_map_section": source["region_map_section"], "target_map_section": section, "target_map_section_label": section_label, "region": "sinnoh"},
            "warps": source.get("warp_events") or [], "connections": source.get("connections") or [],
            "empty_content": {key: len(source.get(key, [])) for key in EVENT_KINDS} | {"map_scripts": 0, "wild_encounter_profiles": 0},
            "asset_records": asset_refs, "topology": {"state": "FROZEN_SOURCE_PENDING_REVIEW", "repair": None},
            "inclusion": {"state": "FROZEN_NOT_SELECTED", "reason": "Generated asset verification and later topology review are intentionally incomplete in the source-only foundation."},
        })
    blockers = sorted(row["record_id"] for row in asset_rows if row.get("selection_blocker"))
    map_manifest = {
        "schema_version": 1, "donor": {"url": DONOR_URL, "commit": DONOR_COMMIT, "layout_catalog_sha256": layout_catalog_hash},
        "selection": {"release_link_enabled": False, "asset_manifest_ready": False, "blockers": blockers,
                      "allowed_inclusion_states": ["FROZEN_NOT_SELECTED", "INCLUDED", "EXCLUDED"],
                      "reason": "Frozen source inventory only; no public travel, story, encounters, or asset import is selected."},
        "source_groups": [{"source_group": group, "target_group": group, "order": order,
                           "map_count": sum(1 for source_group, _, _ in groups if source_group == group)} for order, group in enumerate(GROUPS)],
        "expected_counts": EXPECTED_COUNTS, "maps": maps,
    }
    asset_manifest = {"schema_version": 1, "donor": {"url": DONOR_URL, "commit": DONOR_COMMIT},
                      "wayfarer_baseline": {"commit": baseline_commit}, "selection": {"release_link_enabled": False, "asset_manifest_ready": False, "blockers": blockers},
                      "records": sorted(asset_rows, key=lambda row: row["record_id"])}
    return map_manifest, asset_manifest


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--donor-root", type=Path, required=True)
    parser.add_argument("--wayfarer-baseline", required=True)
    parser.add_argument("--maps-output", type=Path, required=True)
    parser.add_argument("--assets-output", type=Path, required=True)
    args = parser.parse_args()
    maps, assets = build_manifests(args.root.resolve(), args.donor_root.resolve(), args.wayfarer_baseline)
    write_json(args.maps_output, maps)
    write_json(args.assets_output, assets)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
