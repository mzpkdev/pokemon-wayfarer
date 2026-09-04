#!/usr/bin/env python3
"""Generate the focused static audit for Wayfarer's first Hoenn trip."""

from __future__ import annotations

import argparse
import json
import os
import re
import struct
import tempfile
from collections import deque
from pathlib import Path


GAME_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUTPUT = GAME_ROOT / "build/wayfarer-hoenn-entry-audit.json"

ARRIVAL_MAP = "MAP_SLATEPORT_CITY_HARBOR"
ARRIVAL_COORD = (9, 11)
ARRIVAL_ELEVATION = 3
HEAL_LOCATION = "HEAL_LOCATION_SLATEPORT_CITY"


class AuditError(RuntimeError):
    pass


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as error:
        raise AuditError(f"cannot read {path}: {error}") from error


def read_json(path: Path):
    try:
        return json.loads(read_text(path))
    except json.JSONDecodeError as error:
        raise AuditError(f"cannot parse {path}: {error}") from error


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
            stream.write(content)
        os.replace(temporary, path)
    except BaseException:
        Path(temporary).unlink(missing_ok=True)
        raise


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AuditError(message)


def strip_comments(source: str) -> str:
    return "\n".join(line.split("@", 1)[0] for line in source.splitlines())


def _condition_value(condition: str, symbols: dict[str, int]) -> bool:
    condition = condition.split("//", 1)[0].split("/*", 1)[0].strip()
    expression = re.sub(
        r"defined\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)",
        lambda match: "1" if symbols.get(match.group(1), 0) else "0",
        condition,
    )
    expression = re.sub(
        r"\b[A-Za-z_][A-Za-z0-9_]*\b",
        lambda match: str(symbols.get(match.group(0), 0)),
        expression,
    )
    expression = expression.replace("&&", " and ").replace("||", " or ")
    expression = re.sub(r"!(?!=)", " not ", expression)
    if re.fullmatch(r"[0-9\s()<>!=.+\-*/%andornot]+", expression) is None:
        raise AuditError(f"unsupported preprocessor condition: {condition}")
    return bool(eval(expression, {"__builtins__": {}}, {}))


def filter_product(source: str, *, wayfarer: bool) -> str:
    """Select C/preprocessed-script branches for Wayfarer or standalone HNS."""

    symbols = {
        "IS_WAYFARER": int(wayfarer),
        "IS_HNS": 1,
        "IS_EMERALD": 0,
        "IS_FRLG": 0,
        "TRUE": 1,
        "FALSE": 0,
    }
    output: list[str] = []
    stack = [(True, False, True)]
    for line in source.splitlines():
        directive = re.match(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b\s*(.*)$", line)
        if directive is None:
            if stack[-1][2]:
                output.append(line)
            continue
        command, argument = directive.groups()
        if command in {"if", "ifdef", "ifndef"}:
            parent_active = stack[-1][2]
            if command == "if":
                matched = _condition_value(argument, symbols)
            else:
                defined = bool(symbols.get(argument.strip(), 0))
                matched = defined if command == "ifdef" else not defined
            stack.append((parent_active, matched, parent_active and matched))
        elif command == "elif":
            require(len(stack) > 1, "unmatched #elif")
            parent_active, prior_matched, _ = stack[-1]
            matched = not prior_matched and _condition_value(argument, symbols)
            stack[-1] = (parent_active, prior_matched or matched, parent_active and matched)
        elif command == "else":
            require(len(stack) > 1, "unmatched #else")
            parent_active, prior_matched, _ = stack[-1]
            matched = not prior_matched
            stack[-1] = (parent_active, True, parent_active and matched)
        else:
            require(len(stack) > 1, "unmatched #endif")
            stack.pop()
    require(len(stack) == 1, "unterminated preprocessor condition")
    return "\n".join(output)


class ScriptIndex:
    LABEL = re.compile(r"(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}\s*$")

    def __init__(self, sources: list[tuple[Path, str]]) -> None:
        self.blocks: dict[str, str] = {}
        self.paths: dict[str, Path] = {}
        for path, source in sources:
            source = strip_comments(source)
            matches = list(self.LABEL.finditer(source))
            for number, match in enumerate(matches):
                label = match.group(1)
                end = matches[number + 1].start() if number + 1 < len(matches) else len(source)
                self.blocks[label] = source[match.end() : end]
                self.paths[label] = path

    def block(self, label: str) -> str:
        require(label in self.blocks, f"script label {label} is not defined")
        return self.blocks[label]

    def reachable_labels(self, start: str, limit: int = 800) -> set[str]:
        found: set[str] = set()
        pending = deque([start])
        while pending:
            label = pending.popleft()
            if label in found:
                continue
            require(label in self.blocks, f"script label {label} is not defined")
            found.add(label)
            require(len(found) <= limit, f"script graph from {start} exceeded {limit} labels")
            for token in re.findall(r"\b[A-Za-z_][A-Za-z0-9_]*\b", self.blocks[label]):
                if token in self.blocks and token not in found:
                    pending.append(token)
        return found

    def reachable_text(self, start: str) -> str:
        return "\n".join(self.blocks[label] for label in sorted(self.reachable_labels(start)))


def _extract_initializer(source: str, name: str) -> list[str]:
    match = re.search(
        rf"static\s+const\s+struct\s+MenuAction\s+{re.escape(name)}\s*\[\]\s*=\s*\{{(.*?)\n\}};",
        source,
        re.DOTALL,
    )
    require(match is not None, f"menu list {name} is not defined")
    return re.findall(r"\{\s*([A-Za-z_][A-Za-z0-9_]*)\s*\}", match.group(1))


def _case_targets(body: str) -> dict[int, str]:
    return {
        int(number): target
        for number, target in re.findall(r"(?m)^\s*case\s+(\d+)\s*,\s*([A-Za-z_][A-Za-z0-9_]*)", body)
    }


def _position(source: str, pattern: str, message: str, start: int = 0) -> int:
    match = re.search(pattern, source[start:], re.MULTILINE | re.DOTALL)
    require(match is not None, message)
    return start + match.start()


def audit_arrival(game_root: Path) -> dict:
    map_path = game_root / "data/maps/SlateportCity_Harbor/map.json"
    map_data = read_json(map_path)
    require(map_data.get("id") == ARRIVAL_MAP, f"arrival map id must be {ARRIVAL_MAP}")
    require(map_data.get("game_version") == "emerald", "arrival map must retain Emerald provenance")

    x, y = ARRIVAL_COORD
    for event_type in ("coord_events", "object_events", "warp_events"):
        matches = [event for event in map_data[event_type] if (event.get("x"), event.get("y")) == (x, y)]
        require(not matches, f"arrival coordinate {ARRIVAL_COORD} overlaps {event_type}")

    layouts = read_json(game_root / "data/layouts/layouts.json")["layouts"]
    matches = [layout for layout in layouts if layout.get("id") == map_data.get("layout")]
    require(len(matches) == 1, f"arrival layout {map_data.get('layout')} must resolve exactly once")
    layout = matches[0]
    width, height = layout["width"], layout["height"]
    require(0 <= x < width and 0 <= y < height, "arrival coordinate is outside the layout")
    block_path = game_root / layout["blockdata_filepath"]
    block_data = block_path.read_bytes()
    require(len(block_data) == width * height * 2, "arrival layout blockdata has an invalid size")
    blocks = struct.unpack(f"<{width * height}H", block_data)

    def block_at(px: int, py: int) -> tuple[int, int, int]:
        value = blocks[py * width + px]
        return value & 0x3FF, (value >> 10) & 0x3, (value >> 12) & 0xF

    metatile, collision, elevation = block_at(x, y)
    require(collision == 0, f"arrival coordinate {ARRIVAL_COORD} is collision-blocked")
    require(elevation == ARRIVAL_ELEVATION, f"arrival elevation must be {ARRIVAL_ELEVATION}, got {elevation}")

    exits = [
        event
        for event in map_data["warp_events"]
        if event.get("dest_map") == "MAP_SLATEPORT_CITY"
    ]
    require(exits, "Slateport Harbor has no ordinary warp exit to Slateport City")
    occupied = {
        (event["x"], event["y"])
        for event in map_data["object_events"]
        if event.get("elevation") in (0, ARRIVAL_ELEVATION)
    }
    exit_coords = {(event["x"], event["y"]) for event in exits}
    pending = deque([(x, y)])
    previous: dict[tuple[int, int], tuple[int, int] | None] = {(x, y): None}
    reached: tuple[int, int] | None = None
    while pending:
        current = pending.popleft()
        if current in exit_coords:
            reached = current
            break
        for candidate in (
            (current[0] - 1, current[1]),
            (current[0] + 1, current[1]),
            (current[0], current[1] - 1),
            (current[0], current[1] + 1),
        ):
            if candidate in previous or candidate in occupied:
                continue
            px, py = candidate
            if not (0 <= px < width and 0 <= py < height):
                continue
            _, candidate_collision, candidate_elevation = block_at(px, py)
            if candidate_collision != 0:
                continue
            if candidate not in exit_coords and candidate_elevation != ARRIVAL_ELEVATION:
                continue
            previous[candidate] = current
            pending.append(candidate)
    require(reached is not None, "arrival coordinate has no unobstructed path to an ordinary harbor exit")
    path = []
    cursor: tuple[int, int] | None = reached
    while cursor is not None:
        path.append(list(cursor))
        cursor = previous[cursor]
    path.reverse()

    heal_rows = read_json(game_root / "src/data/heal_locations.json")["heal_locations"]
    heal_matches = [row for row in heal_rows if row.get("id") == HEAL_LOCATION]
    require(len(heal_matches) == 1, f"heal location {HEAL_LOCATION} must resolve exactly once")
    heal = heal_matches[0]
    require(heal.get("source") == "EMERALD", "Slateport heal location must have Emerald provenance")
    require(heal.get("map") == "MAP_SLATEPORT_CITY", "Slateport heal location must resolve to Slateport City")

    return {
        "map": ARRIVAL_MAP,
        "coordinate": [x, y],
        "metatile": metatile,
        "collision": collision,
        "elevation": elevation,
        "eventOverlaps": {"coordinate": 0, "object": 0, "warp": 0},
        "ordinaryExit": {
            "coordinate": list(reached),
            "destinationMap": "MAP_SLATEPORT_CITY",
            "path": path,
        },
        "healLocation": HEAL_LOCATION,
    }


def audit_menus_and_routes(game_root: Path) -> dict:
    menu_path = game_root / "src/data/script_menu.h"
    menu_source = read_text(menu_path)
    wayfarer_menu_source = filter_product(menu_source, wayfarer=True)
    hns_menu_source = filter_product(menu_source, wayfarer=False)
    expected_optional = [
        "gText_SouthernIsland",
        "gText_BirthIsland",
        "gText_FarawayIsland",
        "gText_BattleFrontier",
        "gText_Exit",
    ]
    hns_vermilion = _extract_initializer(hns_menu_source, "MultichoiceList_VermilionHarbor")
    wayfarer_vermilion = _extract_initializer(wayfarer_menu_source, "MultichoiceList_VermilionHarbor")
    hns_olivine = _extract_initializer(hns_menu_source, "MultichoiceList_OlivineHarbor")
    wayfarer_olivine = _extract_initializer(wayfarer_menu_source, "MultichoiceList_OlivineHarbor")
    require(hns_vermilion == ["gText_Olivine", *expected_optional], "standalone HNS Vermilion menu changed")
    require(
        wayfarer_vermilion == ["gText_SlateportCity", *expected_optional],
        "Wayfarer Vermilion must replace only menu slot 0 with Slateport",
    )
    expected_olivine = ["gText_Vermilion", *expected_optional]
    require(hns_olivine == expected_olivine, "standalone HNS Olivine menu changed")
    require(wayfarer_olivine == expected_olivine, "Wayfarer Olivine menu or slot indices changed")

    vermilion_path = game_root / "data/maps/VermilionCity_PortInside_hns/scripts.inc"
    olivine_path = game_root / "data/maps/OlivineCity_PortInside_hns/scripts.inc"
    raw_vermilion = read_text(vermilion_path)
    raw_olivine = read_text(olivine_path)
    wayfarer_index = ScriptIndex([(vermilion_path, filter_product(raw_vermilion, wayfarer=True))])
    hns_index = ScriptIndex([(vermilion_path, filter_product(raw_vermilion, wayfarer=False))])
    olivine_index = ScriptIndex([(olivine_path, filter_product(raw_olivine, wayfarer=True))])

    wayfarer_root = wayfarer_index.reachable_text("VermilionPort_EventScript_Sailor")
    hns_root = hns_index.reachable_text("VermilionPort_EventScript_Sailor")
    require(re.search(r"goto_if_ge\s+VAR_SSAQUA_STATE\s*,\s*8\s*,", wayfarer_root) is not None,
            "Wayfarer Vermilion must gate its menu on completed voyage state")
    require("MULTI_VERMILION_HARBOR" in wayfarer_root, "Wayfarer Vermilion does not use the guarded harbor menu")
    require("MAP_OLIVINE_CITY_PORT_INSIDE_HNS" not in wayfarer_root,
            "Wayfarer exposes a Vermilion-to-Olivine S.S. Aqua route")
    require("MAP_OLIVINE_CITY_PORT_INSIDE_HNS" in hns_root,
            "standalone HNS lost the Vermilion-to-Olivine route")
    require("HEAL_LOCATION_OLIVINE_CITY_HNS" in hns_root,
            "standalone HNS lost its Olivine recovery destination")

    after_wayfarer = wayfarer_index.block("VermilionPort_EventScript_Sailor_AfterKanto")
    after_hns = hns_index.block("VermilionPort_EventScript_Sailor_AfterKanto")
    wayfarer_cases = _case_targets(after_wayfarer)
    hns_cases = _case_targets(after_hns)
    require(set(wayfarer_cases) == set(range(6)), "Wayfarer Vermilion menu case indices changed")
    require(set(hns_cases) == set(range(6)), "standalone HNS Vermilion menu case indices changed")
    require(
        [wayfarer_cases[index] for index in range(1, 6)] == [hns_cases[index] for index in range(1, 6)],
        "Wayfarer changed an optional Vermilion destination slot",
    )
    for index in range(1, 6):
        label = wayfarer_cases[index]
        require(
            wayfarer_index.reachable_text(label) == hns_index.reachable_text(label),
            f"Wayfarer changed optional Vermilion destination behavior at slot {index}",
        )
    slateport_label = wayfarer_cases[0]
    require("Slateport" in slateport_label, "Wayfarer Vermilion slot 0 does not select Slateport")
    departure = wayfarer_index.reachable_text(slateport_label)
    state_pos = _position(
        departure,
        r"goto_if_(?:lt|le|ne)\s+VAR_SSAQUA_STATE\s*,\s*8\s*,",
        "Slateport selection must recheck completed voyage state",
    )
    ticket_pos = _position(
        departure,
        r"checkitem\s+ITEM_SS_TICKET",
        "Slateport selection must recheck the shared S.S. Ticket",
        state_pos,
    )
    failure_pos = _position(
        departure,
        r"goto_if_eq\s+VAR_RESULT\s*,\s*FALSE\s*,",
        "Slateport selection must branch on a failed ticket recheck",
        ticket_pos,
    )
    require(
        re.search(r"\b(?:setvar|setflag|clearflag|giveitem|removeitem|setrespawn|warp|warpsilent|special|specialvar)\b",
                  departure[:failure_pos]) is None,
        "Slateport selection changes persistent or travel state before the ticket failure branch",
    )
    preflight_pos = _position(
        departure,
        r"specialvar\s+VAR_RESULT\s*,\s*WayfarerPrepareHoennEntry",
        "Slateport selection must run the destination and initialization preflight",
        failure_pos,
    )
    preflight_failure_pos = _position(
        departure,
        r"goto_if_eq\s+VAR_RESULT\s*,\s*FALSE\s*,",
        "Slateport selection must stop on a failed entry preflight",
        preflight_pos,
    )
    presentation_pos = _position(
        departure,
        r"call\s+VermilionPort_EventScript_EnterShip",
        "Slateport selection must retain the Vermilion S.S. Aqua departure presentation",
        preflight_failure_pos,
    )
    respawn_pos = _position(
        departure,
        rf"setrespawn\s+{HEAL_LOCATION}",
        "Slateport heal destination must be committed before arrival",
        presentation_pos,
    )
    warp_pos = _position(
        departure,
        rf"warp(?:silent)?\s+{ARRIVAL_MAP}\s*,\s*{ARRIVAL_COORD[0]}\s*,\s*{ARRIVAL_COORD[1]}",
        "Slateport warp map or coordinate does not match the audited destination",
        respawn_pos,
    )
    require("removeitem ITEM_SS_TICKET" not in departure, "the shared S.S. Ticket is consumed")
    require("setvar VAR_SSAQUA_STATE" not in departure, "the Hoenn trip changes completed S.S. Aqua voyage state")

    standalone_olivine_index = ScriptIndex([(olivine_path, filter_product(raw_olivine, wayfarer=False))])
    olivine_root = olivine_index.reachable_text("OlivinePort_EventScript_Sailor")
    require(
        olivine_root == standalone_olivine_index.reachable_text("OlivinePort_EventScript_Sailor"),
        "Wayfarer changed Olivine harbor behavior",
    )
    require(re.search(r"goto_if_ge\s+VAR_SSAQUA_STATE\s*,\s*8\s*,", olivine_root) is not None,
            "Wayfarer Olivine does not retain its post-voyage menu")
    require("checkitem ITEM_SS_TICKET" in olivine_root, "Olivine-to-Vermilion lost its ticket gate")
    require("MAP_VERMILION_CITY_PORT_INSIDE_HNS" in olivine_root,
            "Wayfarer Olivine lost its Vermilion route")
    require("MAP_SLATEPORT_CITY_HARBOR" not in olivine_root,
            "Wayfarer added a direct Olivine-to-Slateport route")
    olivine_cases = _case_targets(olivine_index.block("OlivinePort_EventScript_Sailor_AfterKanto"))
    require(
        olivine_cases == {
            0: "OlivinePort_EventScript_ChoseVermilion",
            1: "OlivinePort_EventScript_ChoseSouthernIsland",
            2: "OlivinePort_EventScript_ChoseBirthIsland",
            3: "OlivinePort_EventScript_ChoseFarawayIsland",
            4: "OlivinePort_EventScript_ChoseBattleFrontier",
            5: "OlivinePort_EventScript_Sailor_Refused",
        },
        "Olivine harbor destination indices changed",
    )

    return {
        "menu": {
            "wayfarerVermilion": wayfarer_vermilion,
            "standaloneHnsVermilion": hns_vermilion,
            "wayfarerOlivine": wayfarer_olivine,
            "slateportSlot": 0,
            "preservedOptionalSlots": list(range(1, 6)),
        },
        "departure": {
            "selectionLabel": slateport_label,
            "voyageStateMinimum": 8,
            "ticket": "ITEM_SS_TICKET",
            "ticketConsumed": False,
            "healBeforeWarp": respawn_pos < warp_pos,
        },
        "routes": {
            "wayfarerVermilionToOlivine": False,
            "standaloneHnsVermilionToOlivine": True,
            "wayfarerOlivineToVermilion": True,
        },
    }


def audit_hoenn_ports_and_ticket(game_root: Path) -> dict:
    emerald_maps = []
    aqua_departures = []
    for map_path in sorted((game_root / "data/maps").glob("*/map.json")):
        data = read_json(map_path)
        if data.get("game_version", "emerald") != "emerald":
            continue
        emerald_maps.append(data["id"])
        script_path = map_path.with_name("scripts.inc")
        source = read_text(script_path) if script_path.is_file() else ""
        if re.search(r"SS_?AQUA|S\.S\.?\s*AQUA|SSAqua", source, re.IGNORECASE):
            aqua_departures.append(str(script_path.relative_to(game_root)))
        for event in data.get("object_events", []):
            payload = " ".join(str(value) for value in event.values())
            if re.search(r"SS_?AQUA|S\.S\.?\s*AQUA|SSAqua", payload, re.IGNORECASE):
                aqua_departures.append(str(map_path.relative_to(game_root)))
    require(not aqua_departures, f"Hoenn contains S.S. Aqua departure content: {sorted(set(aqua_departures))}")

    harbor_path = game_root / "data/maps/SlateportCity_Harbor/scripts.inc"
    harbor = strip_comments(filter_product(read_text(harbor_path), wayfarer=True))
    require(
        re.search(
            r"SlateportCity_Harbor_OnTransition:.*?call_if_set\s+FLAG_SYS_GAME_CLEAR\s*,\s*"
            r"SlateportCity_Harbor_EventScript_ShowSSTidal",
            harbor,
            re.DOTALL,
        ) is not None,
        "S.S. Tidal visibility must remain gated by Hoenn game-clear state",
    )
    attendant = re.search(
        r"SlateportCity_Harbor_EventScript_FerryAttendant:{1,2}(.*?)(?=\n[A-Za-z_][A-Za-z0-9_]*:{1,2})",
        harbor,
        re.DOTALL,
    )
    require(attendant is not None, "Slateport S.S. Tidal attendant script is missing")
    require("FLAG_SYS_GAME_CLEAR" in attendant.group(1), "S.S. Tidal service lost its Hoenn game-clear gate")
    require("VAR_SSAQUA_STATE" not in harbor, "S.S. Tidal was coupled to HNS S.S. Aqua voyage state")

    ticket_source = strip_comments(
        filter_product(read_text(game_root / "data/scripts/players_house.inc"), wayfarer=True)
    )
    ticket_index = ScriptIndex(
        [(game_root / "data/scripts/players_house.inc", ticket_source)]
    )
    ticket = ticket_index.block("PlayersHouse_1F_EventScript_GetSSTicketAndSeeLatiTV")
    check_pos = _position(ticket, r"checkitem\s+ITEM_SS_TICKET", "ticket event must check for the shared ticket")
    branch_pos = _position(
        ticket,
        r"goto_if_eq\s+VAR_RESULT\s*,\s*TRUE\s*,",
        "ticket event must skip a duplicate grant when the shared ticket is owned",
        check_pos,
    )
    grant_pos = _position(ticket, r"giveitem\s+ITEM_SS_TICKET", "ticket event lost its normal grant", branch_pos)
    duplicate_target = re.search(
        r"goto_if_eq\s+VAR_RESULT\s*,\s*TRUE\s*,\s*([A-Za-z_][A-Za-z0-9_]*)",
        ticket[branch_pos:],
    )
    require(duplicate_target is not None, "ticket duplicate guard has no success target")
    after_ticket = ticket_index.block(duplicate_target.group(1))
    require(
        re.search(r"setflag\s+FLAG_RECEIVED_SS_TICKET", after_ticket) is not None,
        "ticket event must record Hoenn receipt state on both paths",
    )
    require("giveitem ITEM_SS_TICKET" not in after_ticket, "duplicate ticket path still grants the key item")

    return {
        "emeraldMapsScanned": len(emerald_maps),
        "hoennSsAquaDepartures": [],
        "ssTidal": {"visibilityGate": "FLAG_SYS_GAME_CLEAR", "coupledToSsAqua": False},
        "postgameTicket": {
            "sharedItem": "ITEM_SS_TICKET",
            "duplicateGuard": True,
            "receiptFlag": "FLAG_RECEIVED_SS_TICKET",
            "receiptCommittedAfterGrantPath": True,
        },
    }


def _enclosing_function(source: str, marker: int) -> tuple[str, str] | None:
    header = re.compile(
        r"(?m)^[A-Za-z_][A-Za-z0-9_\s*]*\s+([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*\{"
    )
    for match in reversed([candidate for candidate in header.finditer(source) if candidate.start() < marker]):
        depth = 0
        for index in range(match.end() - 1, len(source)):
            if source[index] == "{":
                depth += 1
            elif source[index] == "}":
                depth -= 1
                if depth == 0:
                    if marker < index:
                        return match.group(1), source[match.end() : index]
                    break
    return None


def audit_initialization(game_root: Path) -> dict:
    marker_re = re.compile(
        r"WayfarerSetHoennStateInitialized\s*\(\s*TRUE\s*\)|wayfarerHoenn\.initialized\s*=\s*TRUE"
    )
    candidates = []
    for path in sorted((game_root / "src").rglob("*.c")):
        raw_source = read_text(path)
        if marker_re.search(raw_source) is None:
            continue
        source = filter_product(raw_source, wayfarer=True)
        for marker in marker_re.finditer(source):
            enclosing = _enclosing_function(source, marker.start())
            if enclosing is not None:
                candidates.append((path, enclosing[0], enclosing[1], marker.group(0)))
    require(len(candidates) == 1, f"expected one Hoenn entry initialization commit, found {len(candidates)}")
    path, function, body, marker_text = candidates[0]
    marker_pos = body.index(marker_text)
    after = body[marker_pos + len(marker_text) :]
    require(
        re.match(r"\s*;?\s*return\s+TRUE\s*;", after) is not None,
        "Hoenn initialized state must be the final commit before successful return",
    )
    require("REGION_HOENN" in body[:marker_pos], "Hoenn entry initialization must record Hoenn as current/visited")
    require("SetLastHealLocationWarp(healLocationId)" in body[:marker_pos],
            "Hoenn entry initialization must register its preflighted recovery location")
    require("HOENN_STARTER_CHOICE_NONE" in body[:marker_pos], "Hoenn entry must preserve an unchosen starter baseline")
    require(
        re.search(r"WayfarerHoennStateIsInitialized\s*\(\s*\)", body[:marker_pos]) is not None,
        "Hoenn entry initialization must be idempotently guarded",
    )
    public_source = filter_product(read_text(game_root / "src/wayfarer_persistence.c"), wayfarer=True)
    public = re.search(r"u16\s+WayfarerPrepareHoennEntry\s*\([^)]*\)\s*\{(.*?)\n\}", public_source, re.DOTALL)
    require(public is not None, "WayfarerPrepareHoennEntry is missing")
    for value in (ARRIVAL_MAP, str(ARRIVAL_COORD[0]), str(ARRIVAL_COORD[1]), HEAL_LOCATION):
        require(value in public.group(1), f"WayfarerPrepareHoennEntry does not pin {value}")

    event_scripts = filter_product(read_text(game_root / "data/event_scripts.s"), wayfarer=True)
    includes = re.findall(r"(?m)^\s*\.include\s+\"([^\"]+)\"", event_scripts)
    baseline_include = "data/scripts/wayfarer_hoenn_entry.inc"
    require(includes.count(baseline_include) == 1, "Hoenn entry baseline must be included exactly once")
    include_index = includes.index(baseline_include)
    require(
        include_index > 0 and includes[include_index - 1] == "data/wayfarer_hoenn_source_constants.inc",
        "Hoenn entry baseline must immediately follow the fixed Hoenn source aliases",
    )
    require(
        include_index + 1 < len(includes)
        and includes[include_index + 1] == "data/wayfarer_engine_source_constants.inc",
        "engine source aliases must be restored immediately after the Hoenn entry baseline",
    )

    baseline_path = game_root / baseline_include
    baseline_source = strip_comments(read_text(baseline_path))
    baseline_index = ScriptIndex([(baseline_path, baseline_source)])
    baseline_body = baseline_index.block("WayfarerHoennEntry_EventScript_InitializeBaseline")
    commands = [line.strip() for line in baseline_body.splitlines() if line.strip()]
    require(commands and commands[-1] == "end", "Hoenn entry baseline must terminate with end")
    require(
        all(command.startswith("setflag FLAG_") for command in commands[:-1]),
        "Hoenn entry baseline may only establish map-object visibility flags",
    )
    new_game_path = game_root / "data/scripts/new_game.inc"
    emerald_new_game = ScriptIndex([(new_game_path, strip_comments(read_text(new_game_path)))])
    emerald_flags = re.findall(
        r"(?m)^\s*setflag\s+(FLAG_[A-Za-z0-9_]+)",
        emerald_new_game.block("EventScript_ResetAllMapFlags"),
    )
    baseline_flags = re.findall(r"(?m)^\s*setflag\s+(FLAG_[A-Za-z0-9_]+)", baseline_body)
    require(
        baseline_flags == emerald_flags,
        "Hoenn entry object-visibility baseline differs from Emerald new-game visibility",
    )
    require("EventScript_ResetAllBerries" not in baseline_body, "Hoenn entry must not reset shared berry state")
    require("VAR_" not in baseline_body, "Hoenn entry baseline advances a Hoenn story variable")
    for forbidden in ("FLAG_SYS_GAME_CLEAR", "FLAG_RECEIVED_SS_TICKET"):
        require(forbidden not in baseline_body, f"Hoenn entry baseline advances {forbidden}")
    return {
        "function": "WayfarerPrepareHoennEntry",
        "implementationFunction": function,
        "path": str(path.relative_to(game_root)),
        "idempotent": True,
        "currentRegion": "REGION_HOENN",
        "healLocation": HEAL_LOCATION,
        "starterChoice": "HOENN_STARTER_CHOICE_NONE",
        "initializedCommittedLast": True,
        "baseline": {
            "label": "WayfarerHoennEntry_EventScript_InitializeBaseline",
            "visibilityFlags": len(baseline_flags),
            "matchesEmeraldObjectVisibility": True,
            "resetsBerries": False,
            "fixedHoennAliasScope": True,
            "engineAliasesRestored": True,
        },
    }


def build_audit(game_root: Path) -> dict:
    game_root = game_root.resolve()
    arrival = audit_arrival(game_root)
    travel = audit_menus_and_routes(game_root)
    ports = audit_hoenn_ports_and_ticket(game_root)
    initialization = audit_initialization(game_root)
    return {
        "schemaVersion": 1,
        "build": "wayfarer",
        "audit": "hoenn-entry",
        "arrival": arrival,
        "travel": travel,
        "initialization": initialization,
        "hoennPorts": ports,
        "status": "pass",
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-root", type=Path, default=GAME_ROOT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args(argv)
    try:
        audit = build_audit(args.game_root)
        atomic_write(args.output, json.dumps(audit, indent=2, sort_keys=True) + "\n")
    except AuditError as error:
        parser.error(str(error))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
