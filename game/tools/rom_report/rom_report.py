#!/usr/bin/env python3
"""Create and validate a deterministic ROM budget report."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence


SCHEMA_VERSION = 1
ROM_START = 0x08000000
ROM_CAPACITY_END = 0x0A000000
WAYFARER_RELEASE_LIMIT = 0x09F80000

CATEGORY_ORDER = (
    "code",
    "scripts",
    "maps_layouts",
    "graphics",
    "audio",
    "trainer_data",
    "encounter_data",
    "other",
)

SYMBOLS = (
    "__rom_start",
    *(f"__rom_category_{category}_{edge}" for category in CATEGORY_ORDER for edge in ("start", "end")),
    "__rom_end",
)

_SYMBOL_RE = re.compile(r"\b(" + "|".join(re.escape(symbol) for symbol in SYMBOLS) + r")\b")
_HEX_RE = re.compile(r"(?<![0-9A-Za-z_])(?:0x)?[0-9A-Fa-f]{7,16}(?![0-9A-Za-z_])")


class ReportError(ValueError):
    """An invalid report input or ROM layout."""


def _hex_address(value: int) -> str:
    return f"0x{value:08X}"


def parse_symbols(text: str) -> dict[str, int]:
    """Read required addresses from GNU objdump/nm symbol or linker map text."""
    values: dict[str, int] = {}
    malformed: list[str] = []

    for line_number, line in enumerate(text.splitlines(), 1):
        match = _SYMBOL_RE.search(line)
        if match is None:
            continue

        symbol = match.group(1)
        address_matches = list(_HEX_RE.finditer(line, 0, match.start()))
        if not address_matches:
            malformed.append(f"line {line_number}: {symbol} has no hexadecimal address")
            continue

        address_text = address_matches[0].group(0)
        try:
            address = int(address_text, 16)
        except ValueError:
            malformed.append(f"line {line_number}: invalid address {address_text!r} for {symbol}")
            continue

        previous = values.get(symbol)
        if previous is not None and previous != address:
            raise ReportError(
                f"symbol {symbol} has conflicting addresses "
                f"{_hex_address(previous)} and {_hex_address(address)}"
            )
        values[symbol] = address

    if malformed:
        raise ReportError("malformed ROM symbols: " + "; ".join(malformed))

    missing = [symbol for symbol in SYMBOLS if symbol not in values]
    if missing:
        raise ReportError("missing ROM symbols: " + ", ".join(missing))

    return values


def validate_layout(symbols: Mapping[str, int]) -> dict[str, tuple[int, int]]:
    """Require categories to form an exact partition of the used ROM."""
    rom_start = symbols["__rom_start"]
    rom_end = symbols["__rom_end"]

    if rom_start != ROM_START:
        raise ReportError(
            f"__rom_start must be {_hex_address(ROM_START)}, got {_hex_address(rom_start)}"
        )
    if rom_end < rom_start:
        raise ReportError("__rom_end precedes __rom_start")
    if rom_end > ROM_CAPACITY_END:
        raise ReportError(
            f"__rom_end {_hex_address(rom_end)} exceeds 32 MiB ROM end "
            f"{_hex_address(ROM_CAPACITY_END)}"
        )

    ranges: dict[str, tuple[int, int]] = {}
    cursor = rom_start
    for category in CATEGORY_ORDER:
        start = symbols[f"__rom_category_{category}_start"]
        end = symbols[f"__rom_category_{category}_end"]
        if start != cursor:
            relation = "gap before" if start > cursor else "overlap at"
            raise ReportError(
                f"ROM category reconciliation failed: {relation} {category}; "
                f"expected {_hex_address(cursor)}, got {_hex_address(start)}"
            )
        if end < start:
            raise ReportError(f"ROM category {category} ends before it starts")
        ranges[category] = (start, end)
        cursor = end

    if cursor != rom_end:
        relation = "unclassified bytes before" if cursor < rom_end else "categories extend past"
        raise ReportError(
            f"ROM category reconciliation failed: {relation} __rom_end; "
            f"categories end at {_hex_address(cursor)}, ROM ends at {_hex_address(rom_end)}"
        )

    return ranges


def _require_mapping(value: Any, description: str) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        raise ReportError(f"malformed baseline: {description} must be an object")
    return value


def parse_baseline(text: str) -> dict[str, int]:
    """Validate the accepted Wayfarer report fields used to calculate deltas."""
    try:
        document = json.loads(text)
    except json.JSONDecodeError as error:
        raise ReportError(f"malformed baseline JSON: {error.msg}") from error

    root = _require_mapping(document, "root")
    schema_version = root.get("schema_version")
    if isinstance(schema_version, bool) or schema_version != SCHEMA_VERSION:
        raise ReportError(
            f"malformed baseline: schema_version must be {SCHEMA_VERSION}"
        )
    if root.get("build") != "wayfarer":
        raise ReportError("malformed baseline: build must be 'wayfarer'")

    categories = _require_mapping(root.get("categories"), "categories")
    expected = set(CATEGORY_ORDER)
    actual = set(categories)
    if actual != expected:
        missing = sorted(expected - actual)
        extra = sorted(actual - expected)
        details = []
        if missing:
            details.append("missing " + ", ".join(missing))
        if extra:
            details.append("unexpected " + ", ".join(extra))
        raise ReportError("malformed baseline categories: " + "; ".join(details))

    result: dict[str, int] = {}
    for category in CATEGORY_ORDER:
        entry = _require_mapping(categories[category], f"categories.{category}")
        size = entry.get("bytes")
        if isinstance(size, bool) or not isinstance(size, int) or size < 0:
            raise ReportError(
                f"malformed baseline: categories.{category}.bytes must be a nonnegative integer"
            )
        result[category] = size
    return result


def build_report(
    symbols: Mapping[str, int],
    *,
    build: str,
    release: bool,
    baseline_sizes: Mapping[str, int] | None,
) -> dict[str, Any]:
    ranges = validate_layout(symbols)
    rom_end = symbols["__rom_end"]
    used_bytes = rom_end - ROM_START
    unused_bytes = ROM_CAPACITY_END - rom_end
    enforce_wayfarer_limit = build == "wayfarer" and release

    if enforce_wayfarer_limit and rom_end > WAYFARER_RELEASE_LIMIT:
        raise ReportError(
            f"Wayfarer release __rom_end {_hex_address(rom_end)} exceeds "
            f"{_hex_address(WAYFARER_RELEASE_LIMIT)}"
        )

    category_report: dict[str, dict[str, Any]] = {}
    category_total = 0
    for category in CATEGORY_ORDER:
        start, end = ranges[category]
        size = end - start
        category_total += size
        category_report[category] = {
            "bytes": size,
            "delta_bytes": None if baseline_sizes is None else size - baseline_sizes[category],
            "end_address": _hex_address(end),
            "start_address": _hex_address(start),
        }

    if category_total != used_bytes:
        # validate_layout should make this unreachable, but keep the invariant
        # local to report construction as a defense against future changes.
        raise ReportError(
            f"ROM category bytes {category_total} do not reconcile with used bytes {used_bytes}"
        )

    return {
        "baseline": {"available": baseline_sizes is not None},
        "build": build,
        "categories": category_report,
        "reconciliation": {
            "category_bytes": category_total,
            "matches_used_bytes": True,
        },
        "release": release,
        "rom": {
            "capacity_end_address": _hex_address(ROM_CAPACITY_END),
            "end_address": _hex_address(rom_end),
            "start_address": _hex_address(ROM_START),
            "unused_bytes": unused_bytes,
            "used_bytes": used_bytes,
            "wayfarer_release_limit_address": _hex_address(WAYFARER_RELEASE_LIMIT),
            "wayfarer_release_limit_enforced": enforce_wayfarer_limit,
        },
        "schema_version": SCHEMA_VERSION,
    }


def render_report(report: Mapping[str, Any]) -> str:
    return json.dumps(report, indent=2, sort_keys=True) + "\n"


def _read_text(path: Path, description: str) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as error:
        raise ReportError(f"unable to read {description} {path}: {error.strerror}") from error


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--symbols", required=True, type=Path, help="GNU symbol or linker map file")
    parser.add_argument("--build", required=True, help="build identity, such as wayfarer or hns")
    parser.add_argument("--release", action="store_true", help="apply release-only policy")
    parser.add_argument(
        "--baseline",
        type=Path,
        help="previous accepted Wayfarer JSON report; omit for the first accepted build",
    )
    parser.add_argument("--output", type=Path, help="write JSON here instead of stdout")
    arguments = parser.parse_args(argv)

    try:
        symbols = parse_symbols(_read_text(arguments.symbols, "symbols"))
        baseline_sizes = None
        if arguments.baseline is not None:
            baseline_sizes = parse_baseline(_read_text(arguments.baseline, "baseline"))
        report = build_report(
            symbols,
            build=arguments.build,
            release=arguments.release,
            baseline_sizes=baseline_sizes,
        )
        rendered = render_report(report)
        if arguments.output is None:
            sys.stdout.write(rendered)
        else:
            try:
                arguments.output.write_text(rendered, encoding="utf-8")
            except OSError as error:
                raise ReportError(
                    f"unable to write report {arguments.output}: {error.strerror}"
                ) from error
    except ReportError as error:
        parser.error(str(error))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
