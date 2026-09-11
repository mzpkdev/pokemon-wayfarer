#!/usr/bin/env python3
"""Generate the source-free Wayfarer script table for registered Sevii maps.

The FRLG map-script files remain behind ``.if IS_FRLG``.  This generator owns
the identically named map-script table symbols used by the selected headers,
without copying any FRLG script body into the composite Wayfarer build.
"""

from __future__ import annotations

import argparse
import json
import os
import tempfile
from pathlib import Path


TOOL_DIR = Path(__file__).resolve().parent
GAME_ROOT = TOOL_DIR.parents[1]
DEFAULT_MANIFEST = GAME_ROOT / "src/data/wayfarer_sevii_maps.json"
DEFAULT_OUTPUT = GAME_ROOT / "data/wayfarer_sevii_event_scripts.inc"
SCRIPT_ROOT = "data/scripts/wayfarer_sevii/"


class GenerationError(ValueError):
    pass


def read_json(path: Path) -> dict:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise GenerationError(f"cannot read {path}: {error}") from error


def map_script_includes(record: dict, root: Path) -> list[str]:
    """Return reviewed Wayfarer-owned script files requested by one map.

    A non-empty ``retained_map_scripts`` list means that its map script table
    is defined in one or more explicitly named source-free files.  Empty maps
    use the generated zero-entry table below.
    """
    entries = record.get("retained_map_scripts", [])
    if not isinstance(entries, list):
        raise GenerationError(f"{record['source_map']} retained_map_scripts must be a list")
    includes = []
    for entry in entries:
        if not isinstance(entry, dict) or not isinstance(entry.get("include"), str):
            raise GenerationError(
                f"{record['source_map']} retained_map_scripts entries must name an include"
            )
        include = entry["include"]
        if not include.startswith(SCRIPT_ROOT) or ".." in Path(include).parts:
            raise GenerationError(
                f"{record['source_map']} script include must stay under {SCRIPT_ROOT}"
            )
        if not (root / include).is_file():
            raise GenerationError(f"{record['source_map']} script include is missing: {include}")
        includes.append(include)
    return includes


def render(root: Path, manifest_path: Path) -> str:
    manifest = read_json(manifest_path)
    if manifest.get("schema_version") != 1:
        raise GenerationError("Wayfarer Sevii manifest has unsupported schema_version")
    maps = manifest.get("maps")
    if not isinstance(maps, list):
        raise GenerationError("Wayfarer Sevii manifest maps must be a list")

    records = []
    includes = set()
    names = set()
    for record in maps:
        if not isinstance(record, dict) or not isinstance(record.get("source_map"), str):
            raise GenerationError("Wayfarer Sevii manifest map has no source_map")
        name = record["source_map"]
        if name in names:
            raise GenerationError(f"Wayfarer Sevii manifest duplicates {name}")
        names.add(name)
        record_includes = map_script_includes(record, root)
        includes.update(record_includes)
        records.append((name, bool(record_includes)))

    lines = [
        "@",
        "@ DO NOT MODIFY THIS FILE! It is auto-generated from src/data/wayfarer_sevii_maps.json",
        "@",
        "@ Source-free Wayfarer map-script tables for the registered FRLG Sevii maps.",
        "@",
        "",
        '\t.include "data/scripts/wayfarer_sevii/common.inc"',
    ]
    lines.extend(f'\t.include "{include}"' for include in sorted(includes))
    if includes:
        lines.append("")
    for name, owns_table in records:
        if owns_table:
            continue
        lines.extend((f"{name}_MapScripts::", "\t.byte 0", ""))
    return "\n".join(lines).rstrip() + "\n"


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_file() and path.read_text(encoding="utf-8") == content:
        return
    descriptor, temporary = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
            stream.write(content)
        os.replace(temporary, path)
    except BaseException:
        Path(temporary).unlink(missing_ok=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=GAME_ROOT)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    root = args.root.resolve()
    manifest = (args.manifest or DEFAULT_MANIFEST).resolve()
    output = (args.output or DEFAULT_OUTPUT).resolve()
    try:
        content = render(root, manifest)
    except GenerationError as error:
        parser.error(str(error))
    if args.check:
        if not output.is_file() or output.read_text(encoding="utf-8") != content:
            parser.error(f"stale generated Sevii script artifact: {output}")
        return 0
    atomic_write(output, content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
