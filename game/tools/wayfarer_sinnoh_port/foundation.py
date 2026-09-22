"""Shared, offline helpers for the frozen Sinnoh source-only foundation."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any


DONOR_URL = "https://github.com/LiderMorti00/Sinnoh-pokeemerald-expansion"
DONOR_COMMIT = "4eed17cc63c4ec8c24fbb20fe49e8d65cb4870d8"
GROUPS = (
    "gMapGroup_SinnohTownsRoutes", "gMapGroup_SpecialAreasSinnoh",
    "gMapGroup_DungeonsSinnoh", "gMapGroup_IndoorSinnoh",
    "gMapGroup_IndoorTwinleaf", "gMapGroup_IndoorSandgem",
    "gMapGroup_IndoorJubilife", "gMapGroup_IndoorOreburgh",
    "gMapGroup_IndoorFloaroma",
)
EXPECTED_COUNTS = {
    "maps": 133, "layouts": 133, "warps": 233, "connections": 114,
    "object_events": 0, "coord_events": 0, "bg_events": 0,
    "nonempty_map_scripts": 0, "wild_encounter_profiles": 0,
}
class FoundationError(ValueError):
    """A frozen source foundation invariant failed."""


def load_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise FoundationError(f"missing required file: {path.as_posix()}") from exc
    except json.JSONDecodeError as exc:
        raise FoundationError(f"invalid JSON in {path.as_posix()}: {exc}") from exc


def selected_source(donor_root: Path) -> tuple[list[tuple[str, int, str]], dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    groups = load_json(donor_root / "data/maps/map_groups.json")
    layouts = load_json(donor_root / "data/layouts/layouts.json").get("layouts")
    if not isinstance(layouts, list):
        raise FoundationError("donor layouts.json has no layouts list")
    layout_by_id = {row.get("id"): row for row in layouts if isinstance(row, dict) and isinstance(row.get("id"), str)}
    rows: list[tuple[str, int, str]] = []
    for group_order, group in enumerate(GROUPS):
        members = groups.get(group)
        if not isinstance(members, list):
            raise FoundationError(f"donor missing source group {group}")
        rows.extend((group, order, name) for order, name in enumerate(members))
    if len(rows) != EXPECTED_COUNTS["maps"] or len({name for _, _, name in rows}) != len(rows):
        raise FoundationError("donor selected map membership is not exactly 133 unique maps")
    maps: dict[str, dict[str, Any]] = {}
    for _, _, name in rows:
        source = load_json(donor_root / "data/maps" / name / "map.json")
        if not isinstance(source.get("layout"), str) or source["layout"] not in layout_by_id:
            raise FoundationError(f"donor map {name} has unknown layout")
        maps[name] = source
    selected_layouts = {source["layout"] for source in maps.values()}
    if len(selected_layouts) != EXPECTED_COUNTS["layouts"]:
        raise FoundationError("donor selected maps are not backed by exactly 133 layouts")
    return rows, maps, layout_by_id
