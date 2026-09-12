#!/usr/bin/env python3
"""Reproduce the Wayfarer layout-storage catalog without changing authored maps.

The selection mirrors mapjson.cpp at the supplied source revision: Wayfarer takes
HNS and Emerald layouts, plus FRLG layout IDs enabled by the Sevii manifest.  It
keeps a per-layout TSV, generated LZ streams, and a deterministic JSON summary.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import subprocess
import sys
import zlib
from pathlib import Path


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def align4(value: int) -> int:
    return (value + 3) & ~3


def lz77_decode(stream: bytes) -> tuple[bytes, int]:
    """Independent bounded decoder for the project's type-0x10 streams."""
    if len(stream) < 4 or stream[0] != 0x10:
        raise ValueError("missing GBA LZ77 header")
    expected = stream[1] | (stream[2] << 8) | (stream[3] << 16)
    out = bytearray()
    pos = 4
    while len(out) < expected:
        if pos >= len(stream):
            raise ValueError("truncated flags")
        flags = stream[pos]
        pos += 1
        for bit in range(8):
            if len(out) == expected:
                break
            if flags & (0x80 >> bit):
                if pos + 2 > len(stream):
                    raise ValueError("truncated backreference")
                token = (stream[pos] << 8) | stream[pos + 1]
                pos += 2
                count = (token >> 12) + 3
                distance = (token & 0x0FFF) + 1
                if distance > len(out) or len(out) + count > expected:
                    raise ValueError("invalid backreference")
                for _ in range(count):
                    out.append(out[-distance])
            else:
                if pos >= len(stream):
                    raise ValueError("truncated literal")
                out.append(stream[pos])
                pos += 1
    return bytes(out), pos


def source_version(layout: dict) -> str:
    return str(layout.get("game_version", "emerald"))


def selected_layouts(root: Path) -> tuple[list[dict], dict]:
    layouts_path = root / "game/data/layouts/layouts.json"
    manifest_path = root / "game/src/data/wayfarer_sevii_maps.json"
    layouts = json.loads(layouts_path.read_text())["layouts"]
    manifest = json.loads(manifest_path.read_text())
    if manifest.get("schema_version") != 1:
        raise ValueError("unsupported Sevii manifest schema")
    release_enabled = manifest.get("release_link_enabled")
    if not isinstance(release_enabled, bool):
        raise ValueError("Sevii manifest lacks boolean release_link_enabled")
    enabled_layout_ids = {
        str(entry["layout"])
        for entry in manifest["maps"]
        if entry.get("enabled", release_enabled)
    }
    result = []
    table_slot = 0
    for layout in layouts:
        if not isinstance(layout, dict) or not layout:
            continue
        border = root / "game" / layout.get("border_filepath", "")
        if not border.is_file():
            continue  # exact mapjson table behavior
        table_slot += 1
        version = source_version(layout)
        chosen = version in {"hns", "emerald"} or (
            version == "frlg" and str(layout.get("id", "")) in enabled_layout_ids
        )
        if not chosen:
            continue
        source = root / "game" / str(layout["blockdata_filepath"])
        if not source.is_file():
            raise ValueError(f"selected source missing: {source}")
        width, height = int(layout["width"]), int(layout["height"])
        logical = width * height * 2
        # fieldmap.h: MAP_OFFSET_W=15, MAP_OFFSET_H=14 and
        # MAX_MAP_DATA_SIZE=10,240 u16 entries (20,480 bytes).
        padded_grid_tiles = (width + 15) * (height + 14)
        if padded_grid_tiles > 10240:
            raise ValueError(f"source exceeds current padded backup grid: {source}")
        if source.stat().st_size < logical:
            raise ValueError(f"source shorter than logical grid: {source}")
        result.append({
            "table_slot": table_slot,
            "layout_id": str(layout["id"]),
            "layout_name": str(layout["name"]),
            "version": version,
            "width": width,
            "height": height,
            "source": source,
            "logical_bytes": logical,
            "padded_grid_tiles": padded_grid_tiles,
            "padded_grid_bytes": padded_grid_tiles * 2,
        })
    identity = {
        "layouts_json_sha256": sha256(layouts_path),
        "sevii_manifest_sha256": sha256(manifest_path),
        "sevii_release_link_enabled": release_enabled,
        "sevii_enabled_layout_count": len(enabled_layout_ids),
    }
    return result, identity


def special_candidate(name: str) -> str:
    """Potential exception families; this is an inventory aid, not policy."""
    lower = name.lower()
    if "battlepyramid" in lower:
        return "battle_pyramid"
    if "trainerhill" in lower:
        return "trainer_hill"
    if "secretbase" in lower:
        return "secret_base"
    if "player" in lower and "room" in lower:
        return "player_room"
    return ""


def validate_audit_inventory(audit_dir: Path, rows: list[dict]) -> dict:
    """Compare the temporary audit rows without mistaking them for provenance."""
    selected_path = audit_dir / "wayfarer-production-selected-layouts.tsv"
    compression_path = audit_dir / "wayfarer-production-compression.tsv"
    if not selected_path.is_file() or not compression_path.is_file():
        raise ValueError(f"audit inventory files missing in {audit_dir}")
    old_selected = list(csv.reader(selected_path.open(), delimiter="\t"))
    old_compression = list(csv.reader(compression_path.open(), delimiter="\t"))
    current_by_source = {row["source"]: row for row in rows}
    old_selected_by_source = {row[5]: row for row in old_selected}
    old_compression_by_source = {row[2]: row for row in old_compression}
    selected_field_mismatches = 0
    for source in set(old_selected_by_source) & set(current_by_source):
        old, current = old_selected_by_source[source], current_by_source[source]
        selected_field_mismatches += (old[:5] != [
            current["layout_id"], current["layout_name"], current["version"],
            str(current["width"]), str(current["height"]),
        ])
    raw_size_mismatches = sum(
        int(old_compression_by_source[source][0]) != current["raw_file_bytes"]
        for source, current in current_by_source.items() if source in old_compression_by_source
    )
    lz_size_mismatches = sum(
        int(old_compression_by_source[source][1]) != current["lz_stream_bytes"]
        for source, current in current_by_source.items() if source in old_compression_by_source
    )
    historical_source = audit_dir / "NewSinjoh_hns-868e154.bin"
    current_source = next((
        row["source"] for row in rows if row["source"] == "data/layouts/NewSinjoh_hns/map.bin"
    ), None)
    historical_source_sample = None
    if historical_source.is_file() and current_source is not None:
        historical_source_sample = {
            "historical_artifact": historical_source.name,
            "historical_sha256": sha256(historical_source),
            "current_source_path": current_source,
            "current_sha256": current_by_source[current_source]["source_sha256"],
            "matches_current": sha256(historical_source) == current_by_source[current_source]["source_sha256"],
        }
    return {
        "selected_inventory_sha256": sha256(selected_path),
        "compression_inventory_sha256": sha256(compression_path),
        "selected_rows": len(old_selected),
        "compression_rows": len(old_compression),
        "selected_path_set_matches": set(old_selected_by_source) == set(current_by_source),
        "selected_metadata_field_mismatches": selected_field_mismatches,
        "compression_path_set_matches": set(old_compression_by_source) == set(current_by_source),
        "compression_raw_size_mismatches": raw_size_mismatches,
        "compression_lz_size_mismatches": lz_size_mismatches,
        "audit_all_lz_total_bytes": sum(int(row[1]) for row in old_compression),
        "current_all_lz_total_bytes": sum(row["lz_stream_bytes"] for row in rows),
        "historical_source_sample": historical_source_sample,
        "note": "The temporary TSVs contain no source hashes. They establish matching paths, metadata, raw lengths, and LZ lengths, not byte identity. The compression TSV is all-LZ output; its 36 expansion cases require raw fallback to reach the hybrid stored total.",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--gbagfx", type=Path, required=True)
    parser.add_argument("--audit-dir", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    output = args.output.resolve()
    gbagfx = args.gbagfx.resolve()
    if not gbagfx.is_file():
        raise ValueError(f"gbagfx executable missing: {gbagfx}")
    output.mkdir(parents=True, exist_ok=True)
    stream_dir = output / "streams"
    stream_dir.mkdir(exist_ok=True)
    selected, identity = selected_layouts(root)
    rows = []
    totals = {"raw_bytes": 0, "stored_bytes": 0, "compressed_entries": 0,
              "raw_fallback_entries": 0, "compressed_stream_bytes": 0,
              "payload_alignment_bytes": 0}
    exception_candidates: dict[str, dict[str, int]] = {}
    payload_cursor = 0
    for layout in selected:
        source: Path = layout["source"]
        raw = source.read_bytes()
        stream_name = f"{layout['table_slot']:04d}-{layout['layout_id']}.lz"
        stream_path = stream_dir / stream_name
        subprocess.run([str(gbagfx), str(source), str(stream_path)], check=True)
        compressed = stream_path.read_bytes()
        decoded, consumed = lz77_decode(compressed)
        padding = compressed[consumed:]
        if decoded != raw:
            raise ValueError(f"round-trip mismatch: {source}")
        if len(padding) > 3 or any(padding):
            raise ValueError(f"invalid LZ output padding: {stream_path}")
        # gbagfx pads LZ output.  Raw obtains the same placement alignment when
        # emitted after a prior payload, so policy compares aligned allocations.
        compressed_alloc = align4(len(compressed))
        raw_alloc = align4(len(raw))
        compressed_selected = compressed_alloc < raw_alloc
        if not compressed_selected:
            stream_path.unlink()
        stored = len(compressed) if compressed_selected else len(raw)
        allocation = compressed_alloc if compressed_selected else raw_alloc
        interrecord_padding = (-payload_cursor) % 4
        payload_cursor += interrecord_padding + stored
        totals["payload_alignment_bytes"] += interrecord_padding
        totals["raw_bytes"] += len(raw)
        totals["stored_bytes"] += stored
        if compressed_selected:
            totals["compressed_entries"] += 1
            totals["compressed_stream_bytes"] += len(compressed)
        else:
            totals["raw_fallback_entries"] += 1
        candidate = special_candidate(layout["layout_name"])
        if candidate:
            bucket = exception_candidates.setdefault(candidate, {"layouts": 0, "raw_bytes": 0, "auto_saved_bytes": 0})
            bucket["layouts"] += 1
            bucket["raw_bytes"] += len(raw)
            bucket["auto_saved_bytes"] += len(raw) - stored
        rows.append({
            **{key: value for key, value in layout.items() if key != "source"},
            "source": source.relative_to(root / "game").as_posix(),
            "source_path": source.relative_to(root / "game").as_posix(),
            "source_sha256": hashlib.sha256(raw).hexdigest(),
            "source_crc32": f"{zlib.crc32(raw) & 0xFFFFFFFF:08x}",
            "raw_file_bytes": len(raw),
            "trailing_bytes": len(raw) - layout["logical_bytes"],
            "lz_stream_bytes": len(compressed),
            "lz_stream_sha256": hashlib.sha256(compressed).hexdigest(),
            "lz_stream_crc32": f"{zlib.crc32(compressed) & 0xFFFFFFFF:08x}",
            "lz_decode_consumed_bytes": consumed,
            "lz_padding_bytes": len(padding),
            "storage": "gba_lz77" if compressed_selected else "raw",
            "stored_bytes": stored,
            "aligned_storage_bytes": allocation,
            "interrecord_alignment_bytes": interrecord_padding,
            "compressed_stream_artifact": f"streams/{stream_name}" if compressed_selected else "",
            "special_raw_exception_candidate": candidate,
        })
    totals["gross_payload_saved_bytes"] = totals["raw_bytes"] - totals["stored_bytes"]
    totals["aligned_payload_bytes"] = payload_cursor
    totals["descriptor_bytes_28_each"] = len(rows) * 28
    totals["descriptor_interrecord_alignment_bytes"] = 0
    largest_decoded = max(rows, key=lambda row: row["raw_file_bytes"])
    largest_padded = max(rows, key=lambda row: row["padded_grid_tiles"])
    totals["largest_decoded_file_bytes"] = largest_decoded["raw_file_bytes"]
    totals["largest_decoded_layout_id"] = largest_decoded["layout_id"]
    totals["largest_padded_grid_tiles"] = largest_padded["padded_grid_tiles"]
    totals["largest_padded_grid_bytes"] = largest_padded["padded_grid_bytes"]
    totals["backup_grid_capacity_tiles"] = 10240
    totals["backup_grid_capacity_bytes"] = 20480
    totals["before_code_checksum_exception_net_bytes"] = (
        totals["raw_bytes"] - totals["aligned_payload_bytes"] - totals["descriptor_bytes_28_each"]
    )
    audit_validation = validate_audit_inventory(args.audit_dir, rows) if args.audit_dir else None
    fields = list(rows[0]) if rows else []
    tsv_path = output / "per-layout.tsv"
    tsv_path.write_text("\t".join(fields) + "\n" + "\n".join(
        "\t".join(str(row[field]) for field in fields) for row in rows
    ) + "\n")
    (output / "per-layout.json").write_text(json.dumps(rows, indent=2, sort_keys=True) + "\n")
    try:
        revision = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True).strip()
    except subprocess.CalledProcessError:
        revision = "unavailable"
    summary = {
        "source_revision": revision,
        "selector": "mapjson.cpp data_matches_version/layout_matches_version; map_data_rules.mk Wayfarer manifest input",
        "tool": {"path": str(gbagfx), "sha256": sha256(gbagfx)},
        "identity": identity,
        "selected_layout_count": len(rows),
        "totals": totals,
        "potential_special_raw_exception_exposure": exception_candidates,
        "artifacts": {
            "per_layout_json": "per-layout.json",
            "per_layout_tsv": "per-layout.tsv",
            "compressed_stream_directory": "streams",
        },
    }
    if audit_validation is not None:
        summary["temporary_audit_inventory_validation"] = audit_validation
    (output / "summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n")
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"catalog: {error}", file=sys.stderr)
        raise SystemExit(1)
