"""Effective Sinnoh topology and bounded structural checks."""

from __future__ import annotations

from typing import Any

from foundation import FoundationError


REPAIR_CLOSE_WARP = "close_warp"
REPAIR_REPLACE_WARP_DESTINATION = "replace_warp_destination"
REPAIR_KINDS = {REPAIR_CLOSE_WARP, REPAIR_REPLACE_WARP_DESTINATION}
REPAIR_REPLACE_CONNECTION = "replace_connection"
REPAIR_ALLOW_ONE_WAY_CONNECTION = "allow_one_way_connection"
CONNECTION_REPAIR_KINDS = {REPAIR_REPLACE_CONNECTION, REPAIR_ALLOW_ONE_WAY_CONNECTION}


def repairs(row: dict[str, Any]) -> list[dict[str, Any]]:
    topology = row.get("topology", {})
    if not isinstance(topology, dict):
        raise FoundationError(f"Sinnoh map {row.get('source_map')} has malformed topology")
    value = topology.get("repairs", [])
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
            removed.add(index)
        else:
            replacement = repair.get("replacement")
            if not isinstance(replacement, dict) or set(replacement) != {"dest_map", "dest_warp_id"}:
                raise FoundationError(f"Sinnoh map {row.get('source_map')} has an invalid warp replacement")
            result[index].update(replacement)
    return [warp for index, warp in enumerate(result) if index not in removed]


def effective_connections(row: dict[str, Any]) -> tuple[list[dict[str, Any]], set[int]]:
    """Return runtime connections and explicitly reviewed one-way indexes."""
    source = row.get("connections")
    topology = row.get("topology", {})
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
        else:
            if repair.get("replacement") is not None:
                raise FoundationError(f"Sinnoh map {row.get('source_map')} marks a one-way connection with a replacement")
            one_way.add(index)
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
        connection_repair_count += len(row.get("topology", {}).get("connection_repairs", []))
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
