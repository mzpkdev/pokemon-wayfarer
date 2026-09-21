"""Reviewed Sinnoh topology transforms and offline structural checks.

The frozen ``warps`` and ``connections`` fields remain donor facts.  A reviewed
repair is the only way an imported map may differ from that source topology.
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from foundation import FoundationError


REPAIR_CLOSE_WARP = "close_warp"
REPAIR_REPLACE_WARP_DESTINATION = "replace_warp_destination"
REPAIR_KINDS = {REPAIR_CLOSE_WARP, REPAIR_REPLACE_WARP_DESTINATION}
REPAIR_REPLACE_CONNECTION = "replace_connection"
REPAIR_ALLOW_ONE_WAY_CONNECTION = "allow_one_way_connection"
CONNECTION_REPAIR_KINDS = {REPAIR_REPLACE_CONNECTION, REPAIR_ALLOW_ONE_WAY_CONNECTION}


def repairs(row: dict[str, Any]) -> list[dict[str, Any]]:
    topology = row.get("topology")
    if not isinstance(topology, dict) or topology.get("state") != "STRUCTURALLY_REVIEWED_PENDING_LAYOUT_VALIDATION":
        raise FoundationError(f"Sinnoh map {row.get('source_map')} has unreviewed topology")
    value = topology.get("repairs")
    if not isinstance(value, list):
        raise FoundationError(f"Sinnoh map {row.get('source_map')} lacks a reviewed repair list")
    return value


def effective_warps(row: dict[str, Any]) -> list[dict[str, Any]]:
    """Return the runtime warps after explicit, index-addressed repairs."""
    source = row.get("warps")
    if not isinstance(source, list):
        raise FoundationError(f"Sinnoh map {row.get('source_map')} has malformed frozen warps")
    result = [dict(warp) for warp in source]
    removed: set[int] = set()
    seen: set[int] = set()
    for repair in repairs(row):
        if not isinstance(repair, dict):
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has a malformed topology repair")
        kind = repair.get("kind")
        index = repair.get("warp_index")
        if kind not in REPAIR_KINDS or not isinstance(index, int) or index < 0 or index >= len(source):
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has an invalid topology repair")
        if index in seen or repair.get("source") != source[index] or not isinstance(repair.get("reason"), str) or not repair["reason"]:
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has an ambiguous topology repair")
        seen.add(index)
        if kind == REPAIR_CLOSE_WARP:
            if repair.get("replacement") is not None:
                raise FoundationError(f"Sinnoh map {row.get('source_map')} closes a warp with a destination")
            expected_output = None
            expected_return = {"state": "closed"}
            removed.add(index)
        else:
            replacement = repair.get("replacement")
            if not isinstance(replacement, dict) or set(replacement) != {"dest_map", "dest_warp_id"}:
                raise FoundationError(f"Sinnoh map {row.get('source_map')} has an invalid warp replacement")
            result[index].update(replacement)
            expected_output = result[index]
            expected_return = {"state": "selected_destination_warp", "map": replacement["dest_map"],
                               "warp_id": replacement["dest_warp_id"]}
        fixture = repair.get("fixture")
        if not isinstance(fixture, dict) or set(fixture) != {"input", "output", "return"} \
         or fixture.get("input") != source[index] \
         or fixture.get("output") != expected_output or fixture.get("return") != expected_return:
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has an incomplete topology repair fixture")
    return [warp for index, warp in enumerate(result) if index not in removed]


def effective_connections(row: dict[str, Any]) -> tuple[list[dict[str, Any]], set[int]]:
    """Return runtime connections and explicitly reviewed one-way indexes."""
    source = row.get("connections")
    topology = row.get("topology")
    if not isinstance(source, list) or not isinstance(topology, dict):
        raise FoundationError(f"Sinnoh map {row.get('source_map')} has malformed frozen connections")
    repair_list = topology.get("connection_repairs", [])
    if not isinstance(repair_list, list):
        raise FoundationError(f"Sinnoh map {row.get('source_map')} lacks a reviewed connection repair list")
    result = [dict(connection) for connection in source]
    one_way: set[int] = set()
    seen: set[int] = set()
    for repair in repair_list:
        if not isinstance(repair, dict):
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has a malformed connection repair")
        kind = repair.get("kind")
        index = repair.get("connection_index")
        if kind not in CONNECTION_REPAIR_KINDS or not isinstance(index, int) or index < 0 or index >= len(source):
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has an invalid connection repair")
        if index in seen or repair.get("source") != source[index] or not isinstance(repair.get("reason"), str) or not repair["reason"]:
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has an ambiguous connection repair")
        seen.add(index)
        if kind == REPAIR_REPLACE_CONNECTION:
            replacement = repair.get("replacement")
            if not isinstance(replacement, dict) or set(replacement) != {"offset"} or not isinstance(replacement["offset"], int):
                raise FoundationError(f"Sinnoh map {row.get('source_map')} has an invalid connection replacement")
            result[index].update(replacement)
            expected_output = result[index]
            expected_return = {"state": "selected_inverse", "map": source[index]["map"],
                               "direction": source[index]["direction"], "offset": -replacement["offset"]}
        else:
            if repair.get("replacement") is not None:
                raise FoundationError(f"Sinnoh map {row.get('source_map')} marks a one-way connection with a replacement")
            one_way.add(index)
            expected_output = result[index]
            expected_return = {"state": "one_way"}
        fixture = repair.get("fixture")
        if not isinstance(fixture, dict) or set(fixture) != {"input", "output", "return"} \
         or fixture.get("input") != source[index] \
         or fixture.get("output") != expected_output or fixture.get("return") != expected_return:
            raise FoundationError(f"Sinnoh map {row.get('source_map')} has an incomplete connection repair fixture")
    return result, one_way


def validate_topology(rows: list[dict[str, Any]]) -> dict[str, Any]:
    """Check every surviving warp returns from its exact selected target warp.

    This is deliberately a structural verdict, not proof that all destination
    event tiles are playable in the field engine.
    """
    by_id = {row.get("target_map_id"): row for row in rows}
    if len(by_id) != len(rows):
        raise FoundationError("Sinnoh topology has duplicate map IDs")
    live_warps = 0
    repaired_warps = 0
    connection_count = 0
    connection_repair_count = 0
    opposite = {"left": "right", "right": "left", "up": "down", "down": "up"}
    connections: dict[tuple[str, int], dict[str, Any]] = {}
    one_way_connections: set[tuple[str, int]] = set()
    for row in rows:
        row_repairs = repairs(row)
        repaired_warps += len(row_repairs)
        for warp in effective_warps(row):
            destination = by_id.get(warp.get("dest_map"))
            if destination is None:
                raise FoundationError(f"Sinnoh live warp leaves the selected catalog: {row.get('source_map')}")
            warp_id = warp.get("dest_warp_id")
            if not isinstance(warp_id, str) or not warp_id.isdigit() or int(warp_id) >= len(effective_warps(destination)):
                raise FoundationError(f"Sinnoh live warp has no safe selected return: {row.get('source_map')}")
            return_warp = effective_warps(destination)[int(warp_id)]
            if return_warp.get("dest_map") != row["target_map_id"]:
                raise FoundationError(f"Sinnoh live warp has no reciprocal selected return: {row.get('source_map')}")
            live_warps += 1
        effective, one_way = effective_connections(row)
        connection_repair_count += len(row["topology"].get("connection_repairs", []))
        for index, connection in enumerate(effective):
            destination = by_id.get(connection.get("map"))
            direction = connection.get("direction")
            if destination is None or direction not in opposite:
                raise FoundationError(f"Sinnoh connection leaves the selected catalog: {row.get('source_map')}")
            key = (row["target_map_id"], index)
            connections[key] = connection
            if index in one_way:
                one_way_connections.add(key)
            connection_count += 1
    for key, connection in connections.items():
        if key in one_way_connections:
            continue
        candidates = [reverse_key for reverse_key, reverse in connections.items()
                      if reverse_key not in one_way_connections
                      and reverse_key[0] == connection["map"]
                      and reverse["map"] == key[0]
                      and reverse["direction"] == opposite[connection["direction"]]
                      and reverse["offset"] == -connection["offset"]]
        if len(candidates) != 1:
            raise FoundationError(f"Sinnoh connection has no uniquely consumed inverse: {key[0]}")
        inverse = candidates[0]
        reverse_candidates = [candidate for candidate, other in connections.items()
                              if candidate not in one_way_connections
                              and candidate[0] == connections[inverse]["map"]
                              and other["map"] == inverse[0]
                              and other["direction"] == opposite[connections[inverse]["direction"]]
                              and other["offset"] == -connections[inverse]["offset"]]
        if reverse_candidates != [key]:
            raise FoundationError(f"Sinnoh connection inverse is not uniquely consumed: {key[0]}")
    return {"structurally_verified": True, "source_warp_count": sum(len(row["warps"]) for row in rows),
            "live_warp_count": live_warps, "connection_count": connection_count,
            "repair_count": repaired_warps + connection_repair_count,
            "connection_repair_count": connection_repair_count,
            "one_way_connection_count": len(one_way_connections)}


def validate_raw_layout_topology(root: Path, rows: list[dict[str, Any]]) -> dict[str, Any]:
    """Validate raw layout bounds and seams without pretending door tiles walk.

    Warp event cells commonly carry a door collision value; determining their
    legal arrival presentation requires the field-animation runtime.  The
    common compressed-layout milestone owns that runtime proof.  Until then we
    prove everything raw blockdata can prove directly and publish the remaining
    gap instead of treating a blocked door cell as a broken map.
    """
    layouts = json.loads((root / "data/layouts/layouts.json").read_text(encoding="utf-8"))["layouts"]
    by_layout = {layout["id"]: layout for layout in layouts}
    by_id = {row["target_map_id"]: row for row in rows}
    grids: dict[str, tuple[int, int, list[int]]] = {}

    for row in rows:
        layout = by_layout.get(row["target_layout"])
        if not isinstance(layout, dict):
            raise FoundationError(f"Sinnoh raw layout is missing: {row['target_layout']}")
        width, height = layout.get("width"), layout.get("height")
        block_path = root / layout.get("blockdata_filepath", "")
        border_path = root / layout.get("border_filepath", "")
        if not isinstance(width, int) or not isinstance(height, int) or width <= 0 or height <= 0 \
         or not block_path.is_file() or block_path.stat().st_size != width * height * 2 \
         or not border_path.is_file() or border_path.stat().st_size != 8:
            raise FoundationError(f"Sinnoh raw layout payload is invalid: {row['target_layout']}")
        payload = block_path.read_bytes()
        grids[row["target_map_id"]] = (width, height,
                                        [payload[index] | payload[index + 1] << 8
                                         for index in range(0, len(payload), 2)])

    warp_destinations = 0
    test_entries = 0
    overlap_tiles = 0
    walkable_seam_tiles = 0
    walkable_seam_elevation_mismatches = 0
    for row in rows:
        entry = row["test_entry"]
        width, height, grid = grids[row["target_map_id"]]
        if not 0 <= entry["x"] < width or not 0 <= entry["y"] < height:
            raise FoundationError(f"Sinnoh test entry is out of raw-layout bounds: {row['source_map']}")
        tile = grid[entry["y"] * width + entry["x"]]
        if ((tile >> 10) & 3) != 0 or ((tile >> 12) & 0xF) != entry["elevation"]:
            raise FoundationError(f"Sinnoh test entry is not raw-walkable: {row['source_map']}")
        test_entries += 1
        for warp in effective_warps(row):
            destination = by_id[warp["dest_map"]]
            width, height, _ = grids[warp["dest_map"]]
            entry = effective_warps(destination)[int(warp["dest_warp_id"])]
            if not 0 <= entry["x"] < width or not 0 <= entry["y"] < height:
                raise FoundationError(f"Sinnoh warp destination is out of raw-layout bounds: {row['source_map']}")
            warp_destinations += 1
        width, height, source = grids[row["target_map_id"]]
        effective, _ = effective_connections(row)
        for connection in effective:
            dest_width, dest_height, destination = grids[connection["map"]]
            offset = connection["offset"]
            direction = connection["direction"]
            seam: list[tuple[int, int]] = []
            if direction in ("left", "right"):
                edge_x = 0 if direction == "left" else width - 1
                dest_x = dest_width - 1 if direction == "left" else 0
                seam = [(source[y * width + edge_x], destination[(y - offset) * dest_width + dest_x])
                        for y in range(height) if 0 <= y - offset < dest_height]
            else:
                edge_y = 0 if direction == "up" else height - 1
                dest_y = dest_height - 1 if direction == "up" else 0
                seam = [(source[edge_y * width + x], destination[dest_y * dest_width + x - offset])
                        for x in range(width) if 0 <= x - offset < dest_width]
            if not seam:
                raise FoundationError(f"Sinnoh connection has no raw-layout overlap: {row['source_map']}")
            overlap_tiles += len(seam)
            for source_tile, dest_tile in seam:
                if (source_tile >> 10 & 3) == 0 and (dest_tile >> 10 & 3) == 0:
                    if (source_tile >> 12 & 0xF) != (dest_tile >> 12 & 0xF):
                        walkable_seam_elevation_mismatches += 1
                    walkable_seam_tiles += 1
    return {"level": "STRUCTURAL_ONLY", "layouts_verified": len(grids),
            "borders_verified": len(grids), "test_entries_verified": test_entries,
            "warp_destination_bounds_verified": warp_destinations,
            "connection_overlap_tiles": overlap_tiles, "walkable_seam_tiles": walkable_seam_tiles,
            "walkable_seam_elevation_mismatches": walkable_seam_elevation_mismatches,
            "remaining_runtime_proof": ["warp_door_walkability_and_elevation",
                                        "connection_seam_walkability_and_elevation",
                                        "camera_border_transition"]}
