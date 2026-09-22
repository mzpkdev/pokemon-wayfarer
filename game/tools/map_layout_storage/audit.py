#!/usr/bin/env python3
"""Reject layout-payload access outside the storage module."""

from __future__ import annotations

import argparse
import re
import subprocess
from pathlib import Path


FORBIDDEN = (
    re.compile(r"->\s*mapData\b"),
    re.compile(r"\b(?:mapLayout|layout)\s*->\s*map\b"),
    re.compile(r"gMapHeader\s*\.\s*mapLayout\s*->\s*map\b"),
)
PUBLIC_PAYLOAD_LABEL = re.compile(r"^(?!\.L)\S+(?:_Blockdata|_MapData)::?$")


def find_violations(root: Path) -> list[str]:
    violations: list[str] = []
    source_root = root / "src"
    for path in sorted((*source_root.rglob("*.c"), *(root / "test").rglob("*.c"))):
        if path == source_root / "map_layout.c":
            continue
        for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            if any(pattern.search(line) for pattern in FORBIDDEN):
                violations.append(f"{path.relative_to(root)}:{line_number}: {line.strip()}")
    return violations


def find_generated_symbol_violations(root: Path) -> list[str]:
    path = root / "data" / "layouts" / "layouts.inc"
    if not path.exists():
        return []
    return [
        f"{path.relative_to(root)}:{line_number}: {line.strip()}"
        for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1)
        if PUBLIC_PAYLOAD_LABEL.match(line.strip())
    ]


def find_object_symbol_violations(nm_output: str) -> list[str]:
    violations = []
    for line in nm_output.splitlines():
        symbol = line.rsplit(maxsplit=1)[-1] if line.split() else ""
        if symbol.endswith(("_Blockdata", "_MapData")):
            violations.append(f"exported map-layout storage symbol: {symbol}")
    return violations


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--object", type=Path)
    parser.add_argument("--nm", default="arm-none-eabi-nm")
    args = parser.parse_args()
    root = args.root.resolve()
    violations = find_violations(root) + find_generated_symbol_violations(root)
    if args.object is not None:
        result = subprocess.run(
            [args.nm, "-g", str(args.object)], check=True, capture_output=True, text=True
        )
        violations += find_object_symbol_violations(result.stdout)
    if violations:
        print("Direct map-layout payload access is private to src/map_layout.c:")
        print("\n".join(violations))
        return 1
    print("Map-layout payload access audit passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
