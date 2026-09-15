#!/usr/bin/env python3
"""Generate the Wayfarer-owned Sevii map-script linkage artifact.

The manifest is the only projection boundary. Authored owner modules export
handlers, but never define source-map tables: this generator emits every table
once and validates the complete selected script closure before writing it.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import tempfile
from pathlib import Path


TOOL_DIR = Path(__file__).resolve().parent
GAME_ROOT = TOOL_DIR.parents[1]
sys.path.insert(0, str(TOOL_DIR.parent))

from wayfarer_sevii_content.closure import (  # noqa: E402
    ClosureError,
    build_script_closure,
    dependency_paths,
    module_for_export,
)
from wayfarer_sevii_content import contracts, schema  # noqa: E402


DEFAULT_MANIFEST = GAME_ROOT / "src/data/wayfarer_sevii_maps.json"
DEFAULT_OUTPUT = GAME_ROOT / "data/wayfarer_sevii_event_scripts.inc"


class GenerationError(ValueError):
    pass


SOURCE_LABEL = re.compile(
    r"(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}\s*(?:@.*)?$"
)


def read_json(path: Path) -> dict:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise GenerationError(f"cannot read {path}: {error}") from error
    if not isinstance(value, dict):
        raise GenerationError(f"manifest root must be an object: {path}")
    return value


def validated(root: Path, manifest_path: Path) -> tuple[dict, dict]:
    manifest = read_json(manifest_path)
    try:
        manifest = schema.validate_manifest(root, manifest)
        contracts.validate_contracts(root, manifest)
        closure = build_script_closure(root, manifest)
    except (ClosureError, schema.SchemaError, contracts.ContractError) as error:
        raise GenerationError(str(error)) from error
    return manifest, closure


def handler_rows(manifest: dict) -> dict[str, list[dict]]:
    rows: dict[str, list[dict]] = {}
    for record in manifest["maps"]:
        source_map = record["source_map"]
        handlers = [
            handler for handler in record.get("retained_map_scripts", [])
            if manifest["content_domains"][schema.OWNER_DOMAINS[handler["owner"]]]["enabled"]
        ]
        rows[source_map] = sorted(handlers, key=lambda handler: handler["source_index"])
    return rows


def selected_module_names(manifest: dict) -> set[str]:
    """Return modules required by all enabled map handlers and event scripts."""
    selected = {"common"}
    for record in manifest["maps"]:
        for handler in handler_rows(manifest)[record["source_map"]]:
            selected.add(handler["module"])
        for event_kind in ("object_events", "coord_events", "bg_events"):
            for event in record.get("retained_events", {}).get(event_kind, []):
                owner = event.get("owner")
                domain = schema.OWNER_DOMAINS.get(owner)
                if domain is None or not manifest["content_domains"][domain]["enabled"]:
                    continue
                entrypoint = event.get("wayfarer_script")
                if not isinstance(entrypoint, str):
                    raise GenerationError(f"{record['source_map']} {event_kind} has no Wayfarer entrypoint")
                module = module_for_export(manifest, entrypoint, owner)
                if module is not None:
                    selected.add(module)
    return selected


def _source_label_block(root: Path, relative: str, label: str) -> str:
    source = (root / relative).read_text(encoding="utf-8").replace("\r\n", "\n")
    matches = list(SOURCE_LABEL.finditer(source))
    for index, match in enumerate(matches):
        if match.group(1) != label:
            continue
        end = matches[index + 1].start() if index + 1 < len(matches) else len(source)
        return source[match.start():end].rstrip() + "\n"
    raise GenerationError(f"pinned source label is missing: {label} in {relative}")


def source_data_blocks(root: Path, modules: dict, selected_modules: set[str]) -> list[str]:
    """Copy only pinned text data from source-map scripts.

    Wayfarer never links the FRLG source map scripts themselves. Selected
    story handlers may reuse reviewed dialogue, so those individual label
    blocks must be materialized in the generated projection. Event scripts and
    every non-string data directive remain forbidden here.
    """
    blocks: dict[str, tuple[str, str]] = {}
    for module_name in sorted(selected_modules):
        for row in modules[module_name].get("allowed_externals", []):
            relative = row.get("path", "")
            label = row.get("label", "")
            if row.get("kind") != "script_symbol" or not relative.startswith("data/maps/"):
                continue
            block = _source_label_block(root, relative, label)
            directives = []
            for line in block.splitlines()[1:]:
                code = line.split("@", 1)[0].strip()
                if not code:
                    continue
                if not code.startswith(".string "):
                    raise GenerationError(f"source-map external is not pure text data: {label}")
                directives.append(code)
            if not directives:
                raise GenerationError(f"source-map external has no text data: {label}")
            previous = blocks.get(label)
            if previous is not None and previous != (relative, block):
                raise GenerationError(f"source data label is pinned to multiple definitions: {label}")
            blocks[label] = (relative, block)
    return [blocks[label][1] for label in sorted(blocks)]


def render(root: Path, manifest_path: Path) -> str:
    manifest, _ = validated(root, manifest_path)
    modules = manifest["script_modules"]
    rows_by_map = handler_rows(manifest)
    try:
        selected_modules = selected_module_names(manifest)
    except ClosureError as error:
        raise GenerationError(str(error)) from error
    missing = selected_modules - set(modules)
    if missing:
        raise GenerationError(f"manifest has no script module {sorted(missing)[0]}")
    includes = sorted({modules[name]["include"] for name in selected_modules})

    lines = [
        "@",
        "@ DO NOT MODIFY THIS FILE! It is auto-generated from src/data/wayfarer_sevii_maps.json",
        "@",
        "@ Central Wayfarer-owned map-script tables; source FRLG scripts are never linked.",
        "@",
        "",
    ]
    lines.extend(f'\t.include "{include}"' for include in includes)
    lines.append("")
    data_blocks = source_data_blocks(root, modules, selected_modules)
    if data_blocks:
        lines.extend((
            "@ Pinned FRLG source text data used by selected handlers.",
            "@ No source event-script block is linked.",
            "",
        ))
        for block in data_blocks:
            lines.extend((block.rstrip(), ""))
    for record in manifest["maps"]:
        source_map = record["source_map"]
        lines.append(f"{source_map}_MapScripts::")
        for row in rows_by_map[source_map]:
            lines.append(f"\tmap_script {row['handler_type']}, {row['wayfarer_script']}")
        lines.extend(("\t.byte 0", ""))
    return "\n".join(lines).rstrip() + "\n"


def _relative(path: Path, root: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError as error:
        raise GenerationError(f"tool is outside game root: {path}") from error


def recursive_dependencies(root: Path, manifest_path: Path) -> list[str]:
    manifest, _ = validated(root, manifest_path)
    try:
        paths = dependency_paths(root, manifest)
    except ClosureError as error:
        raise GenerationError(str(error)) from error
    paths.extend((_relative(TOOL_DIR / "generate.py", root),
                  _relative(TOOL_DIR.parent / "wayfarer_sevii_content" / "closure.py", root)))
    return sorted(set(paths))


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


def write_depfile(path: Path, output: Path, dependencies: list[str], root: Path) -> None:
    target = _relative(output, root)
    escaped = " ".join(dependency.replace(" ", "\\ ") for dependency in dependencies)
    atomic_write(path, f"{target}: {escaped}\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=GAME_ROOT)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--dependencies", action="store_true", help="print validated recursive prerequisites")
    parser.add_argument("--depfile", type=Path, help="write Make dependency rule for the generated artifact")
    args = parser.parse_args()
    if args.dependencies and (args.check or args.depfile):
        parser.error("--dependencies cannot be combined with --check or --depfile")
    root = args.root.resolve()
    manifest = (args.manifest or root / "src/data/wayfarer_sevii_maps.json").resolve()
    output = (args.output or root / "data/wayfarer_sevii_event_scripts.inc").resolve()
    try:
        if args.dependencies:
            print("\n".join(recursive_dependencies(root, manifest)))
            return 0
        content = render(root, manifest)
        if args.depfile:
            write_depfile(args.depfile.resolve(), output, recursive_dependencies(root, manifest), root)
        if args.check:
            if not output.is_file() or output.read_text(encoding="utf-8") != content:
                parser.error(f"stale generated Sevii script artifact: {output}")
            return 0
    except GenerationError as error:
        parser.error(str(error))
    atomic_write(output, content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
