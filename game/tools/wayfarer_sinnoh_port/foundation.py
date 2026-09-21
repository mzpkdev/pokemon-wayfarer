"""Shared, offline helpers for the frozen Sinnoh source-only foundation."""

from __future__ import annotations

import hashlib
import json
import re
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
EVENT_KINDS = ("object_events", "coord_events", "bg_events")
REUSE_CLASSES = {"EXISTING_REFERENCE", "EXACT_ALIAS", "SINNOH_VARIANT", "SINNOH_NEW", "REVIEW_REQUIRED"}


class FoundationError(ValueError):
    """A frozen source foundation invariant failed."""


def load_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise FoundationError(f"missing required file: {path.as_posix()}") from exc
    except json.JSONDecodeError as exc:
        raise FoundationError(f"invalid JSON in {path.as_posix()}: {exc}") from exc


def canonical_json(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def rel(path: Path, root: Path) -> str:
    return path.relative_to(root).as_posix()


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


def current_symbols(root: Path) -> tuple[set[str], set[str]]:
    map_ids: set[str] = set()
    for path in sorted((root / "data/maps").glob("*/map.json")):
        map_id = load_json(path).get("id")
        if isinstance(map_id, str):
            map_ids.add(map_id)
    layouts = load_json(root / "data/layouts/layouts.json").get("layouts", [])
    layout_ids = {row.get("id") for row in layouts if isinstance(row, dict) and isinstance(row.get("id"), str)}
    return map_ids, layout_ids


def sinnoh_symbol(symbol: str) -> str:
    prefix, separator, suffix = symbol.partition("_")
    if not separator:
        raise FoundationError(f"cannot create a collision-safe Sinnoh symbol from {symbol}")
    return f"{prefix}_SINNOH_{suffix}"


def target_symbol(symbol: str, existing: set[str]) -> tuple[str, str | None]:
    if symbol not in existing:
        return symbol, None
    proposed = sinnoh_symbol(symbol)
    if proposed in existing:
        raise FoundationError(f"Sinnoh collision-safe symbol already exists: {proposed}")
    return proposed, "existing Wayfarer symbol collision"


def tileset_path(symbol: str) -> Path:
    name = symbol.removeprefix("gTileset_")
    snake = re.sub(r"(?<!^)([A-Z])", r"_\1", name).lower().replace("brendans_mays", "brendans_mays")
    primary = {"General", "Building"}
    return Path("data/tilesets") / ("primary" if name in primary else "secondary") / snake


def source_tree_hashes(root: Path, path: Path) -> list[dict[str, Any]]:
    absolute = root / path
    if not absolute.is_dir():
        raise FoundationError(f"missing donor tileset source directory: {path.as_posix()}")
    return [{"path": (path / file.relative_to(absolute)).as_posix(), "sha256": sha256_file(file), "bytes": file.stat().st_size}
            for file in sorted(absolute.rglob("*")) if file.is_file()]


def section_target(name: str, source_section: str) -> tuple[str, str]:
    # All target sections are appended, never inherited from a donor spelling.
    if name in {"TwinleafTown_Haouse1", "TwinleafTown_House2"}:
        return "MAPSEC_SINNOH_TWINLEAF_TOWN", "Twinleaf Town"
    if name == "PokmonLeague":
        return "MAPSEC_SINNOH_POKEMON_LEAGUE", "Sinnoh Pokemon League"
    label = source_section.removeprefix("MAPSEC_").replace("_", " ").title()
    return f"MAPSEC_SINNOH_{source_section.removeprefix('MAPSEC_')}", label


def source_hashes(donor_root: Path, name: str, layout: dict[str, Any]) -> tuple[dict[str, str], dict[str, str]]:
    map_path = donor_root / "data/maps" / name / "map.json"
    layout_path = donor_root / "data/layouts/layouts.json"
    block_path = donor_root / layout["blockdata_filepath"]
    border_path = donor_root / layout["border_filepath"]
    paths = {"map_json": f"data/maps/{name}/map.json", "layout_json": "data/layouts/layouts.json",
             "blockdata": layout["blockdata_filepath"], "border": layout["border_filepath"]}
    hashes = {"map_json": sha256_file(map_path),
              "map_json_canonical": sha256_bytes(canonical_json(load_json(map_path))),
              "layout_json": sha256_file(layout_path),
              "layout_json_record_canonical": sha256_bytes(canonical_json(layout)),
              "layout_catalog": sha256_file(layout_path),
              "layout_catalog_canonical": sha256_bytes(canonical_json(load_json(layout_path))),
              "blockdata": sha256_file(block_path), "border": sha256_file(border_path)}
    return paths, hashes
