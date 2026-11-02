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

SLATEPORT_AQUA_LOCAL_ID = "LOCALID_SLATEPORT_HARBOR_WAYFARER_AQUA_ATTENDANT"
SLATEPORT_AQUA_SCRIPT = "WayfarerHoennEntry_EventScript_SlateportAquaAttendant"
SLATEPORT_AQUA_HIDE_FLAG = "FLAG_HIDE_SLATEPORT_CITY_HARBOR_WAYFARER_AQUA_ATTENDANT"
OLIVINE_PORT_MAP = "MAP_OLIVINE_CITY_PORT_INSIDE_HNS"
OLIVINE_PORT_COORD = (8, 16)
OLIVINE_HEAL_LOCATION = "HEAL_LOCATION_OLIVINE_CITY_HNS"
SLATEPORT_AQUA_COORD = (15, 11)
SLATEPORT_AQUA_ELEVATION = 3
SLATEPORT_AQUA_ESCAPE_RESERVED_COORDS = {
    *( (x, 9) for x in range(7, 16) ),
    (7, 10), (8, 10), (8, 11), (8, 12), (8, 13), (8, 14),
}


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


def filter_product(source: str, *, wayfarer: bool, emerald: bool = False) -> str:
    """Select branches for Wayfarer, standalone HNS, or standalone Emerald."""

    symbols = {
        "IS_WAYFARER": int(wayfarer),
        "IS_HNS": int(not emerald),
        "IS_EMERALD": int(emerald),
        "IS_FRLG": 0,
        SLATEPORT_AQUA_HIDE_FLAG: int(wayfarer or emerald),
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


def audit_harbor_position(game_root: Path, map_data: dict, *, coordinate: tuple[int, int], elevation: int,
                          excluded_local_id: str) -> dict:
    """Prove a fixed Harbor object is walkable and can reach a normal exit."""
    x, y = coordinate
    for event_type in ("coord_events", "warp_events"):
        matches = [event for event in map_data[event_type] if (event.get("x"), event.get("y")) == coordinate]
        require(not matches, f"Harbor coordinate {coordinate} overlaps {event_type}")
    overlaps = [event for event in map_data["object_events"] if (event.get("x"), event.get("y")) == coordinate
                and event.get("local_id") != excluded_local_id]
    require(not overlaps, f"Harbor coordinate {coordinate} overlaps another object event")
    layouts = read_json(game_root / "data/layouts/layouts.json")["layouts"]
    matches = [layout for layout in layouts if layout.get("id") == map_data.get("layout")]
    require(len(matches) == 1, f"Harbor layout {map_data.get('layout')} must resolve exactly once")
    layout = matches[0]
    width, height = layout["width"], layout["height"]
    require(0 <= x < width and 0 <= y < height, f"Harbor coordinate {coordinate} is outside the layout")
    block_data = (game_root / layout["blockdata_filepath"]).read_bytes()
    require(len(block_data) == width * height * 2, "Harbor layout blockdata has an invalid size")
    blocks = struct.unpack(f"<{width * height}H", block_data)

    def block_at(px: int, py: int) -> tuple[int, int]:
        value = blocks[py * width + px]
        return (value >> 10) & 0x3, (value >> 12) & 0xF

    collision, actual_elevation = block_at(x, y)
    require(collision == 0, f"Harbor coordinate {coordinate} is collision-blocked")
    require(actual_elevation == elevation, f"Harbor coordinate {coordinate} has elevation {actual_elevation}, not {elevation}")
    exits = [event for event in map_data["warp_events"] if event.get("dest_map") in {"MAP_SLATEPORT_CITY", "MAP_LILYCOVE_CITY"}]
    require(exits, "Harbor has no ordinary city exit")
    occupied = {(event["x"], event["y"]) for event in map_data["object_events"]
                if event.get("local_id") != excluded_local_id and event.get("elevation") in (0, elevation)}
    exit_coords = {(event["x"], event["y"]) for event in exits}
    pending = deque([coordinate])
    previous: dict[tuple[int, int], tuple[int, int] | None] = {coordinate: None}
    reached: tuple[int, int] | None = None
    while pending:
        current = pending.popleft()
        if current in exit_coords:
            reached = current
            break
        for candidate in ((current[0] - 1, current[1]), (current[0] + 1, current[1]),
                          (current[0], current[1] - 1), (current[0], current[1] + 1)):
            if candidate in previous or candidate in occupied:
                continue
            px, py = candidate
            if not (0 <= px < width and 0 <= py < height):
                continue
            candidate_collision, candidate_elevation = block_at(px, py)
            if candidate_collision != 0 or (candidate not in exit_coords and candidate_elevation != elevation):
                continue
            previous[candidate] = current
            pending.append(candidate)
    require(reached is not None, f"Harbor coordinate {coordinate} has no unobstructed path to an ordinary exit")
    path = []
    cursor: tuple[int, int] | None = reached
    while cursor is not None:
        path.append(list(cursor))
        cursor = previous[cursor]
    path.reverse()
    return {"collision": collision, "elevation": actual_elevation, "ordinaryExit": list(reached), "path": path}


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


FERRY_SCRIPT_PATH = "data/scripts/wayfarer_ferry.inc"


def _ferry_index(game_root: Path, extra: list[tuple[Path, str]]) -> ScriptIndex:
    """A script index of the given map scripts plus the shared ferry scripts (Wayfarer branches)."""

    ferry_path = game_root / FERRY_SCRIPT_PATH
    return ScriptIndex([*extra, (ferry_path, filter_product(read_text(ferry_path), wayfarer=True))])


def audit_menus_and_routes(game_root: Path) -> dict:
    """The Vermilion and Olivine sailors board the ferry through the shared ferry specials."""

    vermilion_path = game_root / "data/maps/VermilionCity_PortInside_hns/scripts.inc"
    olivine_path = game_root / "data/maps/OlivineCity_PortInside_hns/scripts.inc"
    raw_vermilion = read_text(vermilion_path)
    raw_olivine = read_text(olivine_path)
    ferry_path = game_root / "src/wayfarer_sevii_ferry.c"
    ferry_source = read_text(ferry_path)
    seagallop_path = game_root / "data/scripts/seagallop.inc"
    seagallop_source = read_text(seagallop_path)
    wayfarer_index = _ferry_index(game_root, [(vermilion_path, filter_product(raw_vermilion, wayfarer=True))])
    hns_index = ScriptIndex([(vermilion_path, filter_product(raw_vermilion, wayfarer=False))])
    olivine_index = _ferry_index(game_root, [(olivine_path, filter_product(raw_olivine, wayfarer=True))])

    # ---- Vermilion: one sailor runs the S.S. berth, the Seagallop and the Anne adventure ----
    wayfarer_top_level = wayfarer_index.block("VermilionPort_EventScript_Sailor")
    wayfarer_root = wayfarer_index.reachable_text("VermilionPort_EventScript_Sailor")
    hns_root = hns_index.reachable_text("VermilionPort_EventScript_Sailor")
    require(
        "special DrawWayfarerVermilionPortMenu" in wayfarer_top_level
        and "specialvar VAR_RESULT, GetWayfarerVermilionPortChoice" in wayfarer_top_level
        and _case_targets(wayfarer_top_level) == {
            0: "VermilionPort_EventScript_Sailor_Ferry",
            1: "VermilionPort_EventScript_Sailor_Sevii",
            2: "VermilionPort_EventScript_Sailor_BoardSSAnne",
            3: "VermilionPort_EventScript_Sailor_Cancel",
        },
        "Wayfarer Vermilion must open the ship / Sevii / Anne adventure menu",
    )
    require("WayfarerCanUseRegularAqua" not in wayfarer_root and "MULTI_VERMILION_HARBOR" not in wayfarer_root
            and "WayfarerCanUseRegularAqua" not in ferry_source,
            "Wayfarer Vermilion still reads the retired regular-Aqua gate or destination menu")
    require("MAP_SLATEPORT_CITY_HARBOR" not in wayfarer_root and "MAP_OLIVINE_CITY_PORT_INSIDE_HNS" not in wayfarer_root,
            "Wayfarer Vermilion still offers a fixed destination")
    require("MULTI_VERMILION_HARBOR" in hns_root and "MAP_OLIVINE_CITY_PORT_INSIDE_HNS" in hns_root,
            "standalone HNS Vermilion lost its harbor menu")
    ferry_branch = wayfarer_index.block("VermilionPort_EventScript_Sailor_Ferry")
    require("call Ferry_EventScript_SailorTalk" in ferry_branch and "call VermilionPort_EventScript_FerryBoardingWalk" in ferry_branch
            and ferry_branch.index("FerryBoardingWalk") < ferry_branch.index("special Ferry_WarpAboard"),
            "Vermilion's ship row must talk, walk and then warp aboard through the ferry specials")
    walk = wayfarer_index.block("VermilionPort_EventScript_FerryBoardingWalk")
    require("setrespawn" not in walk and "LOCALID_VERMILIONMARINA_BOAT" not in walk,
            "Vermilion's ferry boarding walk must not set the respawn or move the boat")
    talk = wayfarer_index.reachable_text("Ferry_EventScript_SailorTalk")
    for needle in ("specialvar VAR_RESULT, Ferry_GetBerthShip", "checkitem ITEM_SS_TICKET", "specialvar VAR_RESULT, Ferry_Board"):
        require(needle in talk, f"the shared gangway sailor lost {needle}")
    require("removeitem" not in talk and "FLAG_SYS_GAME_CLEAR" not in talk, "the shared gangway sailor consumes or gates on something")
    anne_branch = wayfarer_index.block("VermilionPort_EventScript_Sailor_BoardSSAnne")
    require(
        re.search(r"specialvar\s+VAR_RESULT\s*,\s*WayfarerSSAnne_HasUnfinishedContent\s+goto_if_eq\s+VAR_RESULT\s*,\s*FALSE\s*,\s*VermilionPort_EventScript_Sailor_SSAnneComplete", anne_branch) is not None
        and "Ferry_" not in anne_branch,
        "the Anne adventure's entry must stay its own boarding, not the ferry's",
    )

    sevii_branch = wayfarer_index.block("VermilionPort_EventScript_Sailor_Sevii")
    require(
        "special DrawWayfarerVermilionSeviiDestinationMenu" in sevii_branch
        and "specialvar VAR_0x8006, GetWayfarerVermilionSeviiDestination" in sevii_branch
        and all(
            re.search(rf"case\s+{destination}\s*,\s*VermilionPort_EventScript_Sailor_SetSailToSeviiDestination", sevii_branch) is not None
            for destination in ("SEAGALLOP_ONE_ISLAND", "SEAGALLOP_BIRTH_ISLAND", "SEAGALLOP_NAVEL_ROCK")
        ),
        "Wayfarer Sevii selector does not dispatch stable Seagallop destination IDs",
    )
    bill_path = game_root / "data/maps/Route25_BillsHouse_hns/scripts.inc"
    bill_raw = read_text(bill_path)
    bill_source = filter_product(bill_raw, wayfarer=True)
    bill_standalone = filter_product(bill_raw, wayfarer=False)
    bill_index = ScriptIndex([(bill_path, bill_source)])
    bill_gift = bill_index.reachable_text("Route25_BillsHouse_EventScript_Bill")
    require(
        "FLAG_WAYFARER_BILL_RESCUED" in bill_gift
        and "removeobject 5" in bill_gift
        and "setvar VAR_OBJ_GFX_ID_3, OBJ_EVENT_GFX_BILL_HNS" in bill_gift
        and "addobject 5" in bill_gift
        and re.search(r"checkitem\s+ITEM_SS_TICKET\s+goto_if_eq\s+VAR_RESULT\s*,\s*TRUE\s*,\s*Route25_BillsHouse_EventScript_BillTicketAlreadyOwned", bill_gift) is not None,
        "Wayfarer Bill must visibly resolve and branch around an existing S.S. Ticket",
    )
    require(
        "setflag FLAG_WAYFARER_BILL_SS_TICKET_SETTLED" in bill_index.block("Route25_BillsHouse_EventScript_BillTicketAlreadyOwned")
        and not re.search(r"(?:giveitem_msg|additem)", bill_index.block("Route25_BillsHouse_EventScript_BillTicketAlreadyOwned"))
        and re.search(r"checkitemspace\s+ITEM_SS_TICKET\s+goto_if_eq\s+VAR_RESULT\s*,\s*FALSE\s*,\s*Route25_BillsHouse_EventScript_BillNoRoom", bill_gift) is not None
        and "setflag FLAG_WAYFARER_BILL_SS_TICKET_SETTLED" not in bill_index.block("Route25_BillsHouse_EventScript_BillNoRoom"),
        "Wayfarer Bill must leave a full-Bag ticket reward retryable without duplicates",
    )
    require(
        "Route25_BillsHouse_EventScript_Bill::" not in bill_standalone
        and "Route25_BillsHouse_EventScript_Gramps::" in bill_standalone,
        "standalone HNS Bill's House service must remain unchanged",
    )
    require(
        re.search(r"u16\s+WayfarerCanSailToBirthIsland\s*\([^)]*\)\s*\{\s*return\s+CheckBagHasItem\s*\(\s*ITEM_AURORA_TICKET\s*,\s*1\s*\)\s*;\s*\}", ferry_source, re.DOTALL) is not None,
        "Birth Island eligibility must be the read-only Aurora Ticket check",
    )
    require("FLAG_ENABLE_SHIP_NAVEL_ROCK" not in ferry_source,
            "Wayfarer Navel Rock eligibility must not use the unavailable ship flag")
    require(
        re.search(r"u16\s+WayfarerCanSailToNavelRock\s*\([^)]*\)\s*\{\s*return\s+FlagGet\s*\(\s*FLAG_SYS_GAME_CLEAR\s*\)\s*&&\s*CheckBagHasItem\s*\(\s*ITEM_MYSTIC_TICKET\s*,\s*1\s*\)\s*;\s*\}", ferry_source, re.DOTALL) is not None,
        "Navel Rock eligibility must be the read-only League-clear and Mystic Ticket conjunction",
    )
    require(re.search(r"\b(?:FlagSet|FlagClear|AddBagItem|RemoveBagItem)\s*\(", ferry_source) is None,
            "Wayfarer special-island eligibility helpers must not mutate flags or the Bag")

    wayfarer_seagallop = ScriptIndex([(seagallop_path, filter_product(seagallop_source, wayfarer=True))])
    for label in ("EventScript_ChooseDestFromOneIsland", "EventScript_ChooseDestFromTwoIsland", "EventScript_ChooseDestFromIsland"):
        block = wayfarer_seagallop.block(label)
        require(block.lstrip().startswith("goto EventScript_SeviiDestinationsPage1\n"),
                f"Wayfarer {label} must offer the complete ungated numbered-island service")

    special_harbors = (
        ("BirthIsland_Harbor_hns", "BirthIsland_Harbor_hns_EventScript_Sailor"),
        ("NavelRock_Harbor", "NavelRock_Harbor_EventScript_Sailor"),
    )
    for map_name, sailor_label in special_harbors:
        harbor_path = game_root / "data/maps" / map_name / "scripts.inc"
        harbor_source = read_text(harbor_path)
        wayfarer_harbor = ScriptIndex([(harbor_path, filter_product(harbor_source, wayfarer=True))]).block(sailor_label)
        standalone_harbor = ScriptIndex([(harbor_path, filter_product(harbor_source, wayfarer=False))]).block(sailor_label)
        require("warp MAP_VERMILION_CITY_PORT_INSIDE_HNS, 8, 9" in wayfarer_harbor,
                f"{map_name} Wayfarer sailor must return to the HNS Vermilion safe tile")
        require("warp MAP_LILYCOVE_CITY_HARBOR, 8, 11" in standalone_harbor,
                f"{map_name} standalone sailor must retain its Lilycove return")

    # ---- Olivine: the maiden voyage keeps its own boarding; every other case boards the ferry ----
    standalone_olivine_index = ScriptIndex([(olivine_path, filter_product(raw_olivine, wayfarer=False))])
    olivine_root = olivine_index.reachable_text("OlivinePort_EventScript_Sailor")
    standalone_olivine_root = standalone_olivine_index.reachable_text("OlivinePort_EventScript_Sailor")
    require(re.search(r"goto_if_ge\s+VAR_SSAQUA_STATE\s*,\s*8\s*,", standalone_olivine_root) is not None
            and "WayfarerCanUseRegularAqua" not in standalone_olivine_root
            and "WayfarerCanUseAquaMaidenVoyage" not in standalone_olivine_root,
            "standalone HNS Olivine must retain its native voyage gates")
    require("WayfarerCanUseRegularAqua" not in olivine_root and "MULTI_OLIVINE_HARBOR" not in olivine_root,
            "Wayfarer Olivine still reads the retired regular-Aqua gate or destination menu")
    root_block = olivine_index.block("OlivinePort_EventScript_Sailor")
    require(re.search(r"WayfarerCanUseAquaMaidenVoyage\s+goto_if_eq\s+VAR_RESULT\s*,\s*FALSE\s*,\s*OlivinePort_EventScript_Sailor_Ferry\s+goto_if_ge\s+VAR_SSAQUA_STATE\s*,\s*8\s*,\s*OlivinePort_EventScript_Sailor_Ferry", root_block) is not None,
            "Olivine must run the maiden voyage only while a Johto start has not finished it")
    ferry_block = olivine_index.block("OlivinePort_EventScript_Sailor_Ferry")
    require("call Ferry_EventScript_SailorTalk" in ferry_block and "call OlivinePort_EventScript_FerryBoardingWalk" in ferry_block
            and "special Ferry_WarpAboard" in ferry_block,
            "Olivine's ferry boarding must go through the ferry specials")
    require("setrespawn" not in olivine_index.block("OlivinePort_EventScript_FerryBoardingWalk"),
            "Olivine's ferry boarding walk must not set the respawn")
    for label in ("OlivinePort_EventScript_Sailor_MaidenVoyage", "OlivinePort_EventScript_Sailor_ResumeMaidenVoyage"):
        require(olivine_index.reachable_text(label).split() == standalone_olivine_index.reachable_text(label).split(),
                f"Wayfarer changed native voyage behavior at {label}")
    require("MAP_SLATEPORT_CITY_HARBOR" not in olivine_root, "Wayfarer added a direct Olivine-to-Slateport route")

    return {
        "menu": {
            "wayfarerVermilionTopLevel": ["S.S. ship in port", "SEVII ISLANDS", "BOARD S.S. ANNE (before the reward)", "CANCEL"],
            "seviiFirstDestination": "ONE ISLAND",
        },
        "boarding": {
            "talk": "Ferry_EventScript_SailorTalk",
            "ticket": "ITEM_SS_TICKET",
            "ticketConsumed": False,
            "specials": ["Ferry_GetBerthShip", "Ferry_Board", "Ferry_WarpAboard"],
            "walksSetRespawn": False,
        },
        "specialIslandEligibility": {
            "birthIsland": "ITEM_AURORA_TICKET",
            "navelRock": "FLAG_SYS_GAME_CLEAR && ITEM_MYSTIC_TICKET",
            "readOnly": True,
        },
        "specialIslandReturns": {
            "arrival": {"birthIsland": [8, 5], "navelRock": [8, 5]},
            "return": {"map": "MAP_VERMILION_CITY_PORT_INSIDE_HNS", "coordinate": [8, 9]},
            "standaloneReturn": {"map": "MAP_LILYCOVE_CITY_HARBOR", "coordinate": [8, 11]},
        },
    }


def audit_hoenn_ports_and_ticket(game_root: Path) -> dict:
    """Slateport and Lilycove berths call the ferry boarding specials; the Tidal is no longer game-clear gated."""

    slateport_path = game_root / "data/maps/SlateportCity_Harbor/scripts.inc"
    lilycove_path = game_root / "data/maps/LilycoveCity_Harbor/scripts.inc"
    tidal_path = game_root / "data/maps/SSTidalCorridor/scripts.inc"
    slateport_raw = read_text(slateport_path)
    lilycove_raw = read_text(lilycove_path)
    slateport_index = _ferry_index(game_root, [(slateport_path, filter_product(slateport_raw, wayfarer=True))])
    lilycove_index = _ferry_index(game_root, [(lilycove_path, filter_product(lilycove_raw, wayfarer=True))])
    tidal_index = _ferry_index(game_root, [(tidal_path, filter_product(read_text(tidal_path), wayfarer=True))])
    standalone_slateport = ScriptIndex([(slateport_path, filter_product(slateport_raw, wayfarer=False))])

    # ---- Slateport: the attendant is the gangway sailor and gives the Hoenn start's Ticket ----
    attendant = slateport_index.block("SlateportCity_Harbor_EventScript_FerryAttendant")
    ticket_pos = _position(attendant, r"checkitem\s+ITEM_SS_TICKET\s+goto_if_eq\s+VAR_RESULT\s*,\s*TRUE\s*,\s*SlateportCity_Harbor_EventScript_FerryTalk",
                           "an existing Ticket must skip the free grant")
    grant_pos = _position(attendant, r"specialvar\s+VAR_RESULT\s*,\s*WayfarerCanReceiveSlateportTicket\s+goto_if_eq\s+VAR_RESULT\s*,\s*FALSE\s*,\s*SlateportCity_Harbor_EventScript_FerryTalk",
                          "the free Ticket must stay under the profile handoff policy", ticket_pos)
    _position(attendant, r"giveitem\s+ITEM_SS_TICKET\s+goto_if_eq\s+VAR_RESULT\s*,\s*FALSE\s*,\s*SlateportCity_Harbor_EventScript_TicketFailed",
              "a failed Ticket grant must stop", grant_pos)
    ferry_talk = slateport_index.block("SlateportCity_Harbor_EventScript_FerryTalk")
    require("call Ferry_EventScript_SailorTalk" in ferry_talk and "call SlateportCity_Harbor_EventScript_FerryBoardingWalk" in ferry_talk
            and "special Ferry_WarpAboard" in ferry_talk, "the Slateport berth must board through the ferry specials")
    attendant_graph = slateport_index.reachable_text("SlateportCity_Harbor_EventScript_FerryAttendant")
    require("FLAG_SYS_GAME_CLEAR" not in attendant_graph and "WayfarerCanUseRegularAqua" not in attendant_graph
            and "MULTI_SSTIDAL" not in attendant_graph and "VAR_SS_TIDAL_STATE" not in attendant_graph,
            "the Slateport berth still has a game-clear gate, a destination menu or the cruise state")
    require("WayfarerHoennEntry_EventScript_SlateportAquaAttendant" not in slateport_index.blocks,
            "the retired Slateport Aqua attendant is still defined")
    require("FLAG_SYS_GAME_CLEAR" in standalone_slateport.reachable_text("SlateportCity_Harbor_EventScript_FerryAttendant"),
            "standalone Emerald Slateport lost its game-clear gate")
    map_data = read_json(game_root / "data/maps/SlateportCity_Harbor/map.json")
    require(not [event for event in map_data["object_events"]
                 if event.get("script") == "WayfarerHoennEntry_EventScript_SlateportAquaAttendant"],
            "the retired Slateport Aqua attendant object is still placed")
    tidal_objects = [event for event in map_data["object_events"] if event.get("local_id") == "LOCALID_SLATEPORT_HARBOR_SS_TIDAL"]
    require(len(tidal_objects) == 1 and tidal_objects[0]["graphics_id"] == "OBJ_EVENT_GFX_VAR_E"
            and tidal_objects[0]["flag"] == "FLAG_TEMP_1F",
            "the Slateport berth ship must be a variable-graphics object hidden by the berth temp flag")
    attendants = [event for event in map_data["object_events"]
                  if event.get("script") == "SlateportCity_Harbor_EventScript_FerryAttendant"]
    require(len(attendants) == 1 and (attendants[0]["x"], attendants[0]["y"]) == (8, 10)
            and attendants[0]["flag"] == "FLAG_HIDE_SLATEPORT_CITY_HARBOR_PATRONS",
            "the Slateport attendant must stay at (8,10) and hide for the Aqua escape")

    # ---- Lilycove: the attendant boards the ship in port or opens the charter menu ----
    lilycove_map = read_json(game_root / "data/maps/LilycoveCity_Harbor/map.json")
    lilycove_tidal = [event for event in lilycove_map["object_events"] if event.get("local_id") == "LOCALID_LILYCOVE_HARBOR_SS_TIDAL"]
    require(len(lilycove_tidal) == 1 and lilycove_tidal[0]["graphics_id"] == "OBJ_EVENT_GFX_VAR_E"
            and lilycove_tidal[0]["flag"] == "FLAG_TEMP_1F",
            "the Lilycove berth ship must be a variable-graphics object hidden by the berth temp flag")
    lilycove_attendant = lilycove_index.reachable_text("LilycoveCity_Harbor_EventScript_FerryAttendant")
    for needle in ("GetFerryLilycoveCharterCount", "DrawFerryLilycoveCharterMenu", "GetFerryLilycoveCharter",
                   "call Ferry_EventScript_SailorTalk", "special Ferry_WarpAboard"):
        require(needle in lilycove_attendant, f"the Lilycove attendant lost {needle}")
    require("FLAG_SYS_GAME_CLEAR" not in lilycove_attendant and "FLAG_ENABLE_SHIP_" not in lilycove_attendant
            and "SS_TIDAL_BOARD" not in lilycove_attendant and "MULTI_SSTIDAL" not in lilycove_attendant
            and "BirthIsland" not in lilycove_attendant.replace("BirthIslandFirstTime", "")
            and "NavelRock" not in lilycove_attendant.replace("NavelRockFirstTime", ""),
            "the Lilycove attendant still reads the game-clear gate, FLAG_ENABLE_SHIP_* flags or a Tidal destination menu")

    # ---- Tidal: no cruise state machine, Scott gated on the League, the exit sailor goes ashore by the voyage ----
    tidal_text = "\n".join(tidal_index.blocks[label] for label in tidal_index.blocks if label.startswith("SSTidalCorridor_OnFrame"))
    require("VAR_SS_TIDAL_STATE" not in tidal_text, "the Tidal's cruise state machine still runs on frame")
    require("FLAG_SYS_GAME_CLEAR" in tidal_index.block("SSTidalCorridor_OnTransition"),
            "Scott aboard the Tidal must wait for the League")
    require("goto Ferry_EventScript_ExitSailor" in tidal_index.block("SSTidalCorridor_EventScript_ExitSailor"),
            "the Tidal's exit sailor must go ashore by the voyage state")

    ferry_source = filter_product(read_text(game_root / "src/wayfarer_ferry.c"), wayfarer=True)
    require("case FERRY_SHIP_TIDAL:\n        return TRUE;" in ferry_source, "the Tidal must be in service for a Ticket holder, with no game-clear gate")
    require("FLAG_SYS_GAME_CLEAR" not in ferry_source, "the ferry service reads the Hoenn game-clear flag")

    return {
        "hoennSsAquaDepartures": [],
        "slateportBerth": {
            "attendant": "SlateportCity_Harbor_EventScript_FerryAttendant",
            "coordinate": [8, 10],
            "freeTicketPolicy": "WayfarerCanReceiveSlateportTicket",
            "retiredAquaAttendant": True,
            "shipObject": "OBJ_EVENT_GFX_VAR_E",
        },
        "lilycoveBerth": {"attendant": "LilycoveCity_Harbor_EventScript_FerryAttendant", "charters": ["SOUTHERN", "FARAWAY", "FRONTIER"]},
        "ssTidal": {"visibilityGate": None, "gameClearGate": False, "scottGate": "FLAG_SYS_GAME_CLEAR"},
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
    require(len(candidates) == 2, f"expected native and baseline Hoenn initialization commits, found {len(candidates)}")
    commits = {entry[1]: entry for entry in candidates}
    require(set(commits) == {"InitializeLittleroot", "InitializeWayfarerHoennBaseline"},
            "Hoenn initialization may only commit in the native profile or the shared baseline")
    native_path, native_function, native_body, native_marker = commits["InitializeLittleroot"]
    require(native_path == game_root / "src/wayfarer_origin.c",
            "native Hoenn initialization must be owned by the origin profile")
    native_before, native_after = native_body.split(native_marker, 1)
    require(native_after.strip() == ";", "native Hoenn initialized marker must be committed last")
    require("RunScriptImmediately(WayfarerHoennOrigin_EventScript_InitializeBaseline)" in native_before,
            "native Hoenn initialization must use its source-fixed baseline")
    require("WayfarerHoennVisitor_EventScript_InitializeArrival" not in native_body,
            "native Hoenn must not skip its opening with visitor arrival state")
    require("VarSet(VAR_NEWBARK_TOWN_STATE, 2)" in native_before
            and "VarSet(VAR_NEWBARKTOWN_LABSTATE, 0)" in native_before,
            "native Hoenn must establish dormant pre-Elm Johto state")
    path, function, body, marker_text = commits["InitializeWayfarerHoennBaseline"]
    marker_pos = body.index(marker_text)
    after = body[marker_pos + len(marker_text) :]
    require(re.fullmatch(r"\s*;\s*", after) is not None,
            "Hoenn initialized state must be the final commit of the state initialization")
    # Initializing Hoenn's state sets neither the saved region nor the respawn (boarding the Tidal elsewhere must not).
    for forbidden in ("REGION_HOENN", "SetLastHealLocationWarp", "WayfarerSetSavedCurrentRegion", "SetRegionVisitedState"):
        require(forbidden not in body, f"Hoenn state initialization must not touch {forbidden}")
    require("HOENN_STARTER_CHOICE_NONE" in body[:marker_pos], "Hoenn entry must preserve an unchosen starter baseline")
    require(
        re.search(r"WayfarerHoennStateIsInitialized\s*\(\s*\)", body[:marker_pos]) is not None,
        "Hoenn entry initialization must be idempotently guarded",
    )
    public_source = filter_product(read_text(game_root / "src/wayfarer_persistence.c"), wayfarer=True)
    public = re.search(r"bool8\s+WayfarerPrepareHoennEntry\s*\([^)]*\)\s*\{(.*?)\n\}", public_source, re.DOTALL)
    require(public is not None, "WayfarerPrepareHoennEntry is missing")
    arrival = public.group(1)
    require("InitializeWayfarerHoennBaseline()" in arrival
            and arrival.index("InitializeWayfarerHoennBaseline()") < arrival.index("WayfarerSetSavedCurrentRegion(REGION_HOENN)")
            and "SetLastHealLocationWarp(healLocationId)" in arrival,
            "Hoenn arrival must validate, initialize the state, then set the region and the respawn")
    require(arrival.index("IsWayfarerHoennEntryDestinationValid") < arrival.index("InitializeWayfarerHoennBaseline()"),
            "Hoenn arrival must preflight before changing anything")
    ferry_source = filter_product(read_text(game_root / "src/wayfarer_ferry.c"), wayfarer=True)
    for port, tile, heal in (("FERRY_PORT_SLATEPORT", "MAP_SLATEPORT_CITY_HARBOR", "HEAL_LOCATION_SLATEPORT_CITY"),
                             ("FERRY_PORT_LILYCOVE", "MAP_LILYCOVE_CITY_HARBOR", "HEAL_LOCATION_LILYCOVE_CITY")):
        entry = re.search(rf"\[{port}\]\s*=\s*\{{(.*?)\}},", ferry_source, re.DOTALL)
        require(entry is not None and tile in entry.group(1) and heal in entry.group(1) and "TRUE" in entry.group(1),
                f"the ferry port table does not pin {port}")

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
        all(command.startswith(("setflag FLAG_", "clearflag FLAG_")) for command in commands[:-1]),
        "Hoenn entry baseline may only establish map-object visibility flags",
    )
    new_game_path = game_root / "data/scripts/new_game.inc"
    emerald_new_game = ScriptIndex([(new_game_path, strip_comments(read_text(new_game_path)))])
    emerald_flags = re.findall(
        r"(?m)^\s*setflag\s+(FLAG_[A-Za-z0-9_]+)",
        emerald_new_game.block("EventScript_ResetAllMapFlags"),
    )
    baseline_flags = re.findall(r"(?m)^\s*setflag\s+(FLAG_[A-Za-z0-9_]+)", baseline_body)
    emerald_flags = [flag for flag in emerald_flags if flag != SLATEPORT_AQUA_HIDE_FLAG]
    require(
        baseline_flags == emerald_flags,
        "Hoenn entry object-visibility baseline differs from Emerald new-game visibility",
    )
    require("EventScript_ResetAllBerries" not in baseline_body, "Hoenn entry must not reset shared berry state")
    require("VAR_" not in baseline_body, "Hoenn entry baseline advances a Hoenn story variable")
    for forbidden in ("FLAG_SYS_GAME_CLEAR", "FLAG_RECEIVED_SS_TICKET"):
        require(forbidden not in baseline_body, f"Hoenn entry baseline advances {forbidden}")
    require(
        SLATEPORT_AQUA_HIDE_FLAG not in baseline_body,
        "Wayfarer Hoenn entry baseline must not place the retired Slateport Aqua attendant",
    )
    raw_new_game = read_text(new_game_path)
    standalone_new_game = strip_comments(filter_product(raw_new_game, wayfarer=False, emerald=True))
    hns_new_game = strip_comments(filter_product(raw_new_game, wayfarer=False))
    wayfarer_new_game = strip_comments(filter_product(raw_new_game, wayfarer=True))
    require(
        re.search(rf"setflag\s+{SLATEPORT_AQUA_HIDE_FLAG}", standalone_new_game) is not None,
        "non-Wayfarer Emerald initialization must hide the dedicated Slateport Aqua attendant",
    )
    require(
        SLATEPORT_AQUA_HIDE_FLAG not in hns_new_game,
        "standalone HNS initialization must not reference the Emerald-only Aqua attendant flag",
    )
    return {
        "function": "WayfarerPrepareHoennEntry",
        "stateInitializationFunction": "WayfarerInitializeHoennState",
        "implementationFunction": function,
        "nativeInitializationFunction": native_function,
        "nativeInitializedCommittedLast": True,
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
            "slateportAquaAttendantRetired": True,
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
