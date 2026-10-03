#!/usr/bin/env python3
"""Measure Wayfarer RAM use from a linker map and enforce the RAM budget."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence


SCHEMA_VERSION = 1
DEFAULT_BUDGET = Path(__file__).with_name("ram_budget.json")
BUDGET_FILE_HINT = "tools/ram_report/ram_budget.json"

EWRAM_START = 0x02000000
EWRAM_END = 0x02040000
IWRAM_START = 0x03000000
IWRAM_END = 0x03008000
# sp_sys starts here (src/crt0.s) and the system stack grows down into the
# static IWRAM sections, so the gap between them is all the stack gets.
STACK_TOP = IWRAM_END - 0x1C0

BUILDS = ("release", "e2e", "test")
MEASURES = ("ewram_used", "iwram_static")

# Output sections the map must contain; a map without them is not one of ours.
REQUIRED_SECTIONS = (".ewram", ".ewram.sbss", ".iwram", ".iwram.bss")
# The mechanics-test runner keeps a few bytes at 0x03007F00 that survive soft
# resets (ld_script_test.ld). They sit above sp_sys, so they are not static
# data the stack can collide with.
EXCLUDED_IWRAM_SECTIONS = frozenset({".iwram.persistent"})

# Hard floors for the release build. They live here, not in ram_budget.json,
# so a PR cannot trade away the shipped game's safety margin by editing JSON:
# the stack has no overflow detection and silently corrupts IWRAM statics when
# it runs out, and EWRAM free space is the only room left for new features.
# Lowering either floor must be a reviewed change to this tool.
RELEASE_MIN_STACK_ROOM = 4096
RELEASE_MIN_EWRAM_FREE = 4096

MEASURE_LABELS = {
    "ewram_used": "EWRAM used",
    "iwram_static": "IWRAM static (.iwram + .iwram.bss)",
}
MEASURE_FIXES = {
    "ewram_used": (
        "move constant data to ROM tables and temporary buffers to the heap "
        "(Alloc/Free)"
    ),
    "iwram_static": (
        "move buffers to EWRAM (EWRAM_DATA) or the heap (Alloc/Free), since a "
        "plain static lands in IWRAM and shrinks the stack"
    ),
}

_START_RE = re.compile(r"^Linker script and memory map\s*$")
# Output sections start in column 0; input sections and assignments are indented.
_PLACEMENT = r"0x(?P<address>[0-9A-Fa-f]+)\s+0x(?P<size>[0-9A-Fa-f]+)(?:\s|$)"
_SECTION_RE = re.compile(r"^(?P<name>[^\s*]\S*)(?:\s+" + _PLACEMENT + r"|\s*$)")
_WRAPPED_RE = re.compile(r"^\s+" + _PLACEMENT)


class ReportError(ValueError):
    """An invalid map, budget, or argument."""


class BudgetError(Exception):
    """The measured RAM use breaks the budget."""

    def __init__(self, violations: Sequence[str]):
        super().__init__("\n".join(violations))
        self.violations = list(violations)


def _hex_address(value: int) -> str:
    return f"0x{value:08X}"


def _bytes(value: int) -> str:
    return f"{value:,} bytes"


def parse_map_sections(text: str) -> dict[str, tuple[int, int]]:
    """Read output sections as {name: (address, size)} from a GNU ld map."""
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if _START_RE.match(line):
            body = lines[index + 1 :]
            break
    else:
        raise ReportError("not a GNU ld map: missing 'Linker script and memory map'")

    sections: dict[str, tuple[int, int]] = {}
    pending: str | None = None
    for line in body:
        if pending is not None:
            # ld wraps a long section name and prints its address on the next line.
            wrapped = _WRAPPED_RE.match(line)
            if wrapped is not None:
                _add_section(sections, pending, wrapped)
            pending = None
            continue
        if not line or line[0].isspace():
            continue
        match = _SECTION_RE.match(line)
        if match is None or not match.group("name").startswith("."):
            continue
        if match.group("address") is None:
            pending = match.group("name")
        else:
            _add_section(sections, match.group("name"), match)

    missing = [name for name in REQUIRED_SECTIONS if name not in sections]
    if missing:
        raise ReportError("missing RAM output sections: " + ", ".join(missing))
    return sections


def _add_section(
    sections: dict[str, tuple[int, int]], name: str, match: re.Match[str]
) -> None:
    value = (int(match.group("address"), 16), int(match.group("size"), 16))
    previous = sections.get(name)
    if previous is not None and previous != value:
        raise ReportError(f"output section {name} appears twice with different placement")
    sections[name] = value


def measure(sections: Mapping[str, tuple[int, int]]) -> dict[str, int]:
    """Compute EWRAM use, IWRAM static size, and stack room."""
    ewram_end = EWRAM_START
    iwram_end = IWRAM_START
    for name, (address, size) in sections.items():
        end = address + size
        if EWRAM_START <= address < EWRAM_END:
            if end > EWRAM_END:
                raise ReportError(f"{name} ends at {_hex_address(end)}, past EWRAM")
            ewram_end = max(ewram_end, end)
        elif IWRAM_START <= address < IWRAM_END and name not in EXCLUDED_IWRAM_SECTIONS:
            if end > IWRAM_END:
                raise ReportError(f"{name} ends at {_hex_address(end)}, past IWRAM")
            iwram_end = max(iwram_end, end)

    return {
        "ewram_used": ewram_end - EWRAM_START,
        "ewram_free": EWRAM_END - ewram_end,
        "iwram_static": iwram_end - IWRAM_START,
        "iwram_static_end": iwram_end,
        # Negative when statics overlap the stack's starting point.
        "stack_room": STACK_TOP - iwram_end,
    }


def parse_budget(text: str) -> dict[str, dict[str, int | None]]:
    """Validate the checked-in ceilings.

    null marks a report-only measure. The mechanics-test ELF uses it for EWRAM:
    it already sits within bytes of the 256 KB hardware limit, so no ceiling
    tighter than the limit (which the linker enforces) could be met.
    """
    try:
        document = json.loads(text)
    except json.JSONDecodeError as error:
        raise ReportError(f"malformed budget JSON: {error.msg}") from error
    if not isinstance(document, dict):
        raise ReportError("malformed budget: root must be an object")
    schema_version = document.get("schema_version")
    if isinstance(schema_version, bool) or schema_version != SCHEMA_VERSION:
        raise ReportError(f"malformed budget: schema_version must be {SCHEMA_VERSION}")
    builds = document.get("builds")
    if not isinstance(builds, dict) or set(builds) != set(BUILDS):
        raise ReportError("malformed budget: builds must list exactly " + ", ".join(BUILDS))

    capacity = {"ewram_used": EWRAM_END - EWRAM_START, "iwram_static": STACK_TOP - IWRAM_START}
    budget: dict[str, dict[str, int | None]] = {}
    for build in BUILDS:
        entry = builds[build]
        if not isinstance(entry, dict):
            raise ReportError(f"malformed budget: builds.{build} must be an object")
        ceilings: dict[str, int | None] = {}
        for measure_name in MEASURES:
            if measure_name not in entry:
                raise ReportError(f"malformed budget: builds.{build}.{measure_name} is missing")
            value = entry[measure_name]
            if value is None:
                # The release build is the shipped game; every measure is enforced.
                if build == "release":
                    raise ReportError(
                        f"malformed budget: builds.release.{measure_name} cannot be "
                        "report-only"
                    )
            elif isinstance(value, bool) or not isinstance(value, int) or value < 0:
                raise ReportError(
                    f"malformed budget: builds.{build}.{measure_name} must be a "
                    "nonnegative integer or null"
                )
            elif value > capacity[measure_name]:
                raise ReportError(
                    f"malformed budget: builds.{build}.{measure_name} {value} exceeds "
                    f"the hardware limit {capacity[measure_name]}"
                )
            ceilings[measure_name] = value
        budget[build] = ceilings

    release = budget["release"]
    max_iwram = capacity["iwram_static"] - RELEASE_MIN_STACK_ROOM
    max_ewram = capacity["ewram_used"] - RELEASE_MIN_EWRAM_FREE
    if release["iwram_static"] > max_iwram:  # type: ignore[operator]
        raise ReportError(
            f"malformed budget: builds.release.iwram_static {release['iwram_static']} "
            f"would leave less than the {RELEASE_MIN_STACK_ROOM}-byte stack floor "
            f"(maximum {max_iwram})"
        )
    if release["ewram_used"] > max_ewram:  # type: ignore[operator]
        raise ReportError(
            f"malformed budget: builds.release.ewram_used {release['ewram_used']} "
            f"would leave less than the {RELEASE_MIN_EWRAM_FREE}-byte EWRAM floor "
            f"(maximum {max_ewram})"
        )
    return budget


def check_budget(
    measured: Mapping[str, int], *, build: str, ceilings: Mapping[str, int | None]
) -> list[str]:
    """Return one actionable message per violation."""
    violations: list[str] = []
    for measure_name in MEASURES:
        ceiling = ceilings[measure_name]
        actual = measured[measure_name]
        if ceiling is not None and actual > ceiling:
            violations.append(
                f"{build} build: {MEASURE_LABELS[measure_name]} is {_bytes(actual)}, "
                f"ceiling {_bytes(ceiling)} ({_bytes(actual - ceiling)} over). "
                f"Fix: {MEASURE_FIXES[measure_name]}, or raise "
                f"builds.{build}.{measure_name} deliberately in {BUDGET_FILE_HINT} "
                "in this PR."
            )

    if build == "release":
        if measured["stack_room"] < RELEASE_MIN_STACK_ROOM:
            violations.append(
                f"release build: stack room is {_bytes(measured['stack_room'])}, "
                f"hard floor {_bytes(RELEASE_MIN_STACK_ROOM)}. Fix: "
                f"{MEASURE_FIXES['iwram_static']}. This floor is fixed in "
                "ram_report.py and cannot be relaxed in the budget JSON."
            )
        if measured["ewram_free"] < RELEASE_MIN_EWRAM_FREE:
            violations.append(
                f"release build: EWRAM free is {_bytes(measured['ewram_free'])}, "
                f"hard floor {_bytes(RELEASE_MIN_EWRAM_FREE)}. Fix: "
                f"{MEASURE_FIXES['ewram_used']}. This floor is fixed in "
                "ram_report.py and cannot be relaxed in the budget JSON."
            )
    return violations


def build_report(
    measured: Mapping[str, int], *, build: str, ceilings: Mapping[str, int | None]
) -> dict[str, Any]:
    if build not in BUILDS:
        raise ReportError(f"unknown build {build!r}; expected one of " + ", ".join(BUILDS))
    violations = check_budget(measured, build=build, ceilings=ceilings)
    if violations:
        raise BudgetError(violations)

    release = build == "release"
    return {
        "build": build,
        "ewram": {
            "capacity_bytes": EWRAM_END - EWRAM_START,
            "ceiling_bytes": ceilings["ewram_used"],
            "enforced": ceilings["ewram_used"] is not None,
            "free_bytes": measured["ewram_free"],
            "min_free_bytes": RELEASE_MIN_EWRAM_FREE if release else None,
            "used_bytes": measured["ewram_used"],
        },
        "iwram": {
            "ceiling_bytes": ceilings["iwram_static"],
            "enforced": ceilings["iwram_static"] is not None,
            "static_bytes": measured["iwram_static"],
            "static_end_address": _hex_address(measured["iwram_static_end"]),
        },
        "schema_version": SCHEMA_VERSION,
        "stack": {
            "min_room_bytes": RELEASE_MIN_STACK_ROOM if release else None,
            "room_bytes": measured["stack_room"],
            "top_address": _hex_address(STACK_TOP),
        },
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
    parser.add_argument("--map", required=True, type=Path, help="GNU ld linker map")
    parser.add_argument("--build", required=True, choices=BUILDS, help="budget identity")
    parser.add_argument(
        "--budget", type=Path, default=DEFAULT_BUDGET, help="override the checked-in budget"
    )
    parser.add_argument("--output", type=Path, help="write JSON here instead of stdout")
    arguments = parser.parse_args(argv)

    if arguments.output is not None:
        # Never leave a stale passing report behind a failing check.
        arguments.output.unlink(missing_ok=True)

    try:
        sections = parse_map_sections(_read_text(arguments.map, "map"))
        budget = parse_budget(_read_text(arguments.budget, "budget"))
        report = build_report(
            measure(sections), build=arguments.build, ceilings=budget[arguments.build]
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
    except BudgetError as error:
        print(f"RAM budget check failed for {arguments.map}:", file=sys.stderr)
        for violation in error.violations:
            print(f"  - {violation}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
