"""Validate and report the Wayfarer Sevii owned script closure.

The Sevii map manifest names every map-script handler, while this module owns
the other half of that contract: modules, their exports, and their reviewed
external dependencies.  It deliberately parses the small, stable subset of
event-script assembly that carries symbol references.  It accepts only the
finite reviewed directive subset used by owned modules; unknown directives
fail instead of becoming an escape hatch for script references or file input.
"""

from __future__ import annotations

import hashlib
import re
from pathlib import Path
from typing import Any


SCRIPT_ROOT = "data/scripts/wayfarer_sevii/"
OWNERS = {"exploration", "ordinary_trainer", "story", "trainer_tower"}
SHARED_EXPLORATION_EVENT_LABELS = {"EventScript_StrengthBoulder"}
LABEL = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
LABEL_DEF = re.compile(r"(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}\s*(?:@.*)?$")
INCLUDE = re.compile(r'^\s*\.include\s+"([^"]+)"\s*(?:@.*)?$')
MAP_SCRIPT = re.compile(r"^\s*map_script\s+(MAP_SCRIPT_[A-Z0-9_]+)\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*(?:@.*)?$", re.MULTILINE)
EQU = re.compile(r"^\s*\.equ\s+([A-Za-z_][A-Za-z0-9_]*)\s*,\s*((?:VAR|FLAG)_TEMP_[A-Z0-9_]+)\s*(?:@.*)?$")
DATA_DIRECTIVE = re.compile(r"^\s*\.(byte|2byte|4byte|word)\s+([A-Za-z_][A-Za-z0-9_]*|0x[0-9A-Fa-f]+|[0-9]+)\s*(?:@.*)?$")

# Commands with symbol operands.  The listed positions are zero based after
# the command name.  ``last`` is used by the conditional branch family.
REFERENCE_OPERANDS: dict[str, tuple[int | str, ...]] = {
    "call": (0,), "goto": (0,), "goto_if_eq": (2,), "goto_if_ne": (2,),
    "goto_if_lt": (2,), "goto_if_le": (2,), "goto_if_gt": (2,), "goto_if_ge": (2,),
    "goto_if_set": (1,), "goto_if_unset": (1,), "call_if_set": (1,),
    "call_if_unset": (1,), "call_if_eq": (2,), "call_if_ne": (2,),
    "call_if_lt": (2,), "call_if_le": (2,), "call_if_gt": (2,), "call_if_ge": (2,),
    "map_script": (1,), "map_script_2": (2,), "applymovement": (1,),
    "msgbox": (0,), "message": (0,), "braillemsgbox": (0,), "pokemart": (0,),
    "callnative": (0,), "special": (0,), "specialvar": (1,),
    "loadword": (1,), "loadbytefromptr": (1,), "loadwordfromptr": (1,),
}
TRAINER_COMMANDS = {"trainerbattle", "trainerbattle_single", "trainerbattle_double"}
TRANSACTION_COMMANDS = {"giveitem", "givepokemon", "removeitem"}
STATE_WRITES = {"setflag", "clearflag", "setvar", "addvar", "subvar", "copyvar", "setorcopyvar"}
STATE_READS = {"checkflag", "checkvar", "compare", "goto_if_set", "goto_if_unset", "call_if_set", "call_if_unset", "goto_if_eq", "goto_if_ne", "goto_if_lt", "goto_if_le", "goto_if_gt", "goto_if_ge", "call_if_eq", "call_if_ne", "call_if_lt", "call_if_le", "call_if_gt", "call_if_ge"}
TRANSIENT_VARS = {"VAR_RESULT", "VAR_LAST_TALKED", "VAR_FACING", "VAR_0x8004", "VAR_0x8005", "VAR_0x8006", "VAR_0x8007", "VAR_0x8008", "VAR_0x8009", "VAR_0x800A"}

# These are the only story specials whose C implementations are reviewed as
# atomic content transactions.  Their implementation source is pinned by the
# manifest's special external contract, so this table intentionally names the
# exact calling ABI rather than accepting arbitrary ``specialvar`` calls.
# ``state_vars`` are receipt flags supplied through event-script temporary
# variables; the special reads and writes them after it has completed the safe
# destination/consumption sequence.  Selphy's claim owns its fixed payload
# clear entirely in C.
TRANSACTION_SPECIALS: dict[str, dict[str, Any]] = {
    "WayfarerSevii_TryGiveItemThenSetFlag": {"kind": "grant", "state_vars": ("VAR_0x8005",)},
    "WayfarerSevii_TryRemoveItemThenSetFlag": {"kind": "handoff", "state_vars": ("VAR_0x8005",)},
    "WayfarerSevii_TryExchangeItemForReward": {"kind": "handoff", "state_vars": ("VAR_0x8006",)},
    "WayfarerSevii_TryExchangeItemForRewardThenSetFlags": {
        "kind": "handoff", "state_vars": ("VAR_0x8006", "VAR_0x8007"),
    },
    "WayfarerSevii_TryGiveEggThenSetFlag": {"kind": "grant", "state_vars": ("VAR_0x8005",)},
    "WayfarerSevii_TryClaimSelphyPendingReward": {
        "kind": "claim",
        "fixed_states": (
            "VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES",
            "VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD",
            "VAR_WAYFARER_SEVII_SELPHY_REQUEST_ACTIVE",
        ),
    },
}

# Constants name engine values, not script/data labels.  A dependency is
# intentionally rejected unless it is a local export or a manifest row.
CONSTANT_PREFIXES = (
    "VAR_", "FLAG_", "MAP_", "ITEM_", "SE_", "MUS_", "SONG_", "SFX_",
    "LOCALID_", "OBJ_EVENT_", "METATILE_", "MOVEMENT_", "STEP_", "GAME_",
    "DAYCARE_", "PARTY_", "SELECT_", "FADE_", "FAMECHECKER_", "TRAINER_",
    "BATTLE_", "WEATHER_", "MAPSEC_", "FLDEFF_", "TRUE", "FALSE", "YES", "NO",
)


class ClosureError(ValueError):
    """A selected module is incomplete, ambiguous, or reaches unreviewed code."""


def _sha256(value: str) -> str:
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def _relative(path: Path, root: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError as error:
        raise ClosureError(f"script dependency is outside the game root: {path}") from error


def _root_relative_path(value: Any, *, field: str) -> str:
    if not isinstance(value, str) or not value or Path(value).is_absolute() or ".." in Path(value).parts:
        raise ClosureError(f"{field} must be a root-relative path")
    return value


def _resolved_under_root(root: Path, relative: str, *, field: str) -> Path:
    candidate = (root / relative).resolve()
    try:
        candidate.relative_to(root.resolve())
    except ValueError as error:
        raise ClosureError(f"{field} resolves outside the game root") from error
    return candidate


def _owned_path(value: Any, *, field: str, owner: str | None = None) -> str:
    if not isinstance(value, str) or not value.startswith(SCRIPT_ROOT) or ".." in Path(value).parts:
        raise ClosureError(f"{field} must stay under {SCRIPT_ROOT}")
    if owner is not None:
        relative = value.removeprefix(SCRIPT_ROOT)
        roots = {
            "ordinary_trainer": "trainers/",
            "story": "story/",
            "trainer_tower": "trainer_tower/",
        }
        required = roots.get(owner)
        if required is not None and not relative.startswith(required):
            raise ClosureError(f"{field} must stay under {SCRIPT_ROOT}{required}")
        if owner == "exploration" and relative.startswith(tuple(roots.values())):
            raise ClosureError(f"{field} cannot enter another owner's script directory")
    return value


def _module_rows(manifest: dict[str, Any]) -> dict[str, dict[str, Any]]:
    modules = manifest.get("script_modules")
    if not isinstance(modules, dict) or not modules:
        raise ClosureError("manifest script_modules must be a non-empty object")
    normalized: dict[str, dict[str, Any]] = {}
    for name, row in modules.items():
        if not isinstance(name, str) or not LABEL.fullmatch(name) or not isinstance(row, dict):
            raise ClosureError("manifest script_modules entries need a module id and object")
        owner = row.get("owner")
        if owner not in OWNERS:
            raise ClosureError(f"script module {name} has an invalid owner")
        include = _owned_path(row.get("include"), field=f"script module {name} include", owner=owner)
        exports = row.get("exports")
        if not isinstance(exports, list) or not exports or any(not isinstance(label, str) or not LABEL.fullmatch(label) for label in exports):
            raise ClosureError(f"script module {name} exports must be a non-empty label list")
        if len(exports) != len(set(exports)):
            raise ClosureError(f"script module {name} exports duplicate a label")
        externals = row.get("allowed_externals")
        if not isinstance(externals, list):
            raise ClosureError(f"script module {name} allowed_externals must be a list")
        commands = row.get("allowed_commands")
        if not isinstance(commands, list) or not commands or any(not isinstance(command, str) or not re.fullmatch(r"[a-z][a-z0-9_]*", command) for command in commands):
            raise ClosureError(f"script module {name} allowed_commands must be a non-empty command list")
        normalized[name] = {"owner": owner, "include": include, "exports": set(exports),
                            "allowed_externals": externals, "allowed_commands": set(commands)}
    return normalized


def module_for_export(manifest: dict[str, Any], label: str, owner: str) -> str | None:
    """Resolve one owner module for an event entrypoint.

    A selected event cannot rely on a module merely because some unrelated
    map-script handler happens to select it.  The one historical shared field
    helper remains an explicit exploration-only exception.
    """
    if owner not in OWNERS:
        raise ClosureError(f"event entrypoint {label} has invalid owner {owner}")
    if label in SHARED_EXPLORATION_EVENT_LABELS:
        if owner != "exploration":
            raise ClosureError(f"non-exploration event cannot use shared helper {label}")
        return None
    matches = [name for name, module in _module_rows(manifest).items()
               if module["owner"] == owner and label in module["exports"]]
    if len(matches) != 1:
        if not matches:
            raise ClosureError(f"event entrypoint {label} is not exported by an {owner} module")
        raise ClosureError(f"event entrypoint {label} is exported by multiple {owner} modules")
    return matches[0]


def _handler_rows(manifest: dict[str, Any]) -> list[dict[str, Any]]:
    maps = manifest.get("maps")
    if not isinstance(maps, list):
        raise ClosureError("manifest maps must be a list")
    rows: list[dict[str, Any]] = []
    seen: set[tuple[str, str]] = set()
    for record in maps:
        if not isinstance(record, dict) or not isinstance(record.get("source_map"), str):
            raise ClosureError("manifest map has no source_map")
        handlers = record.get("retained_map_scripts", [])
        if not isinstance(handlers, list):
            raise ClosureError(f"{record['source_map']} retained_map_scripts must be a list")
        for index, row in enumerate(handlers):
            required = ("owner", "content_id", "module", "handler_type", "source_index", "source", "wayfarer_script", "reason")
            if not isinstance(row, dict) or any(key not in row for key in required):
                raise ClosureError(f"{record['source_map']} retained_map_scripts[{index}] lacks declarative handler fields")
            if row["owner"] not in OWNERS or not isinstance(row["content_id"], str) or not row["content_id"]:
                raise ClosureError(f"{record['source_map']} retained_map_scripts[{index}] has invalid owner or content_id")
            if not isinstance(row["module"], str) or not isinstance(row["handler_type"], str) or not row["handler_type"].startswith("MAP_SCRIPT_"):
                raise ClosureError(f"{record['source_map']} retained_map_scripts[{index}] has invalid module or handler_type")
            if not isinstance(row["source_index"], int) or row["source_index"] < 0:
                raise ClosureError(f"{record['source_map']} retained_map_scripts[{index}] has invalid source_index")
            if not isinstance(row["source"], dict) or not isinstance(row["reason"], str) or not row["reason"]:
                raise ClosureError(f"{record['source_map']} retained_map_scripts[{index}] lacks source provenance or reason")
            if not isinstance(row["wayfarer_script"], str) or not row["wayfarer_script"].startswith("WayfarerSevii_"):
                raise ClosureError(f"{record['source_map']} retained_map_scripts[{index}] lacks a Wayfarer-owned handler")
            key = (record["source_map"], row["handler_type"])
            if key in seen:
                raise ClosureError(f"duplicate map-script handler type: {record['source_map']} {row['handler_type']}")
            seen.add(key)
            copy = dict(row)
            copy["source_map"] = record["source_map"]
            copy["manifest_index"] = index
            rows.append(copy)
    return rows


def _source_handlers(root: Path, source_map: str) -> list[tuple[str, str]]:
    path = root / "data/maps" / source_map / "scripts.inc"
    if not path.is_file():
        raise ClosureError(f"source map script is missing: {_relative(path, root)}")
    source = path.read_text(encoding="utf-8")
    table = re.search(rf"(?m)^{re.escape(source_map)}_MapScripts::\s*$([\s\S]*?)(?=^\S.*::|\Z)", source)
    if table is None:
        raise ClosureError(f"source map has no map-script table: {source_map}")
    return [(match.group(1), match.group(2)) for match in MAP_SCRIPT.finditer(table.group(1))]


def _validate_provenance(root: Path, handler: dict[str, Any]) -> None:
    source = handler["source"]
    if source.get("kind") == "baseline":
        expected = _sha256(f"map_script {handler['handler_type']}, {handler['wayfarer_script']}\n")
        if source.get("sha256") != expected:
            raise ClosureError(f"{handler['source_map']} {handler['handler_type']} baseline provenance hash drifted")
        return
    expected_include = f"data/maps/{handler['source_map']}/scripts.inc"
    if (source.get("include") != expected_include or not isinstance(source.get("label"), str)
            or not isinstance(source.get("sha256"), str) or not isinstance(source.get("file_sha256"), str)):
        raise ClosureError(f"{handler['source_map']} {handler['handler_type']} has invalid exact source provenance")
    if hashlib.sha256((root / expected_include).read_bytes()).hexdigest() != source["file_sha256"]:
        raise ClosureError(f"{handler['source_map']} {handler['handler_type']} source script file drifted")
    rows = _source_handlers(root, handler["source_map"])
    index = handler["source_index"]
    if index >= len(rows):
        raise ClosureError(f"{handler['source_map']} {handler['handler_type']} source handler disappeared")
    source_type, source_label = rows[index]
    if source_type != handler["handler_type"] or source_label != source["label"]:
        raise ClosureError(f"{handler['source_map']} {handler['handler_type']} source handler identity drifted")
    expected = _sha256(f"map_script {source_type}, {source_label}\n")
    if source["sha256"] != expected:
        raise ClosureError(f"{handler['source_map']} {handler['handler_type']} source handler hash drifted")


def _module_files(root: Path, include: str, owner: str) -> list[str]:
    pending = [include]
    seen: set[str] = set()
    while pending:
        current = pending.pop()
        if current in seen:
            continue
        seen.add(current)
        path = root / current
        if not path.is_file():
            raise ClosureError(f"Wayfarer-owned script include is missing: {current}")
        for line in path.read_text(encoding="utf-8").splitlines():
            _validate_directive(line, current)
            match = INCLUDE.match(line)
            if match:
                pending.append(_owned_path(match.group(1), field=f"include in {current}", owner=owner))
    return sorted(seen)


def _line_references(line: str) -> list[tuple[str, str]]:
    code = line.split("@", 1)[0].strip()
    if not code or code.startswith("#"):
        return []
    data = re.fullmatch(r"\.(?:2byte|4byte|word)\s+([A-Za-z_][A-Za-z0-9_]*)\s*", code)
    if data:
        return [(data.group(1), "data")]
    if code.startswith("."):
        return []
    parts = [part.strip() for part in code.replace(",", " ").split()]
    if not parts:
        return []
    command, operands = parts[0], parts[1:]
    if command in TRAINER_COMMANDS:
        result = []
        if command == "trainerbattle":
            positions, nullable = (3, 4, 5, 8, 9, 10, 11, 12), {3, 4, 5, 8, 9, 10, 11, 12}
        elif command == "trainerbattle_single":
            positions, nullable = (1, 2, 3), {3}
        else:
            positions, nullable = (1, 2, 3, 4), {4}
        for position in positions:
            if position >= len(operands):
                if position in nullable:
                    continue
                raise ClosureError(f"malformed trainerbattle operands for {command}")
            token = operands[position]
            if LABEL.fullmatch(token) and not _is_constant(token):
                result.append((token, "trainer"))
            elif position in nullable and token in {"NULL", "FALSE"}:
                continue
            else:
                raise ClosureError(f"malformed reference operand {token} for {command}")
        return result
    positions = REFERENCE_OPERANDS.get(command, ())
    if command.startswith(("goto_if_", "call_if_")):
        positions = ("last",)
    result: list[tuple[str, str]] = []
    for position in positions:
        if position == "last":
            token = operands[-1] if operands else ""
        elif isinstance(position, int) and position < len(operands):
            token = operands[position]
        else:
            continue
        if token:
            if LABEL.fullmatch(token) and not _is_constant(token):
                result.append((token, command))
            else:
                raise ClosureError(f"malformed reference operand {token} for {command}")
    return result


def _validate_directive(line: str, relative: str) -> None:
    """Allow only typed assembly directives used by reviewed script modules."""
    code = line.split("@", 1)[0].strip()
    if not code.startswith("."):
        return
    if INCLUDE.match(line) or EQU.match(line) or DATA_DIRECTIVE.match(line):
        return
    if re.fullmatch(r"\.align\s+[0-9]+\s*", code):
        return
    if re.fullmatch(r'\.string\s+"(?:[^"\\]|\\.)*"\s*', code):
        return
    if code.startswith(".incbin"):
        raise ClosureError(f"script module has unreviewed file-bearing directive in {relative}: .incbin")
    raise ClosureError(f"script module has unsupported assembler directive in {relative}: {code.split()[0]}")


def _script_command(line: str) -> str | None:
    """Return an executable command, rejecting assembly that looks executable."""
    code = line.split("@", 1)[0].strip()
    if not code or code.startswith(("#", ".")) or LABEL_DEF.fullmatch(code):
        return None
    match = re.fullmatch(r"([a-z][a-z0-9_]*)(?:\s+.*)?", code)
    if match is None:
        raise ClosureError(f"unrecognized executable script line: {code}")
    return match.group(1)


def _operands(line: str) -> list[str]:
    code = line.split("@", 1)[0].strip()
    if not code or code.startswith((".", "#")):
        return []
    parts = [part.strip() for part in code.replace(",", " ").split()]
    return parts[1:]


def _state_operands(command: str, line: str) -> list[tuple[str, str]]:
    operands = _operands(line)
    if command in STATE_WRITES | STATE_READS:
        result = [("write" if command in STATE_WRITES else "read", operands[0])] if operands else []
        if command in {"copyvar", "setorcopyvar"} and len(operands) > 1:
            result.append(("read", operands[1]))
        return result
    return []


def transaction_kind(command: str) -> str | None:
    """Return the reviewed semantic kind for an atomic transaction special."""
    effect = TRANSACTION_SPECIALS.get(command)
    return effect["kind"] if effect is not None else None


def _special_transaction_states(label: str, transient_values: dict[str, str], *, module: str,
                                relative: str) -> set[str]:
    """Resolve one reviewed special's C-owned persistent state effects.

    The event VM only passes scalar temporary variables to a special.  We
    therefore require an immediately concrete, owned state symbol for every
    receipt argument instead of guessing through arbitrary script flow.
    """
    effect = TRANSACTION_SPECIALS[label]
    states = set(effect.get("fixed_states", ()))
    for variable in effect.get("state_vars", ()):
        target = transient_values.get(variable)
        if target is None:
            raise ClosureError(f"script module {module} transaction special {label} lacks a concrete {variable} receipt in {relative}")
        state = _validate_state_operand(target, module=module, relative=relative, aliases=set())
        if state is None:
            raise ClosureError(f"script module {module} transaction special {label} has a non-persistent {variable} receipt in {relative}")
        states.add(state)
    return states


def _validate_state_operand(target: str, *, module: str, relative: str, aliases: set[str]) -> str | None:
    if target.isdigit() or target.lower().startswith("0x"):
        raise ClosureError(f"script module {module} writes or reads raw numeric state {target} in {relative}")
    if target.startswith("FLAG_"):
        if target.startswith("FLAG_TEMP_"):
            return None
        if not target.startswith("FLAG_WAYFARER_SEVII_"):
            raise ClosureError(f"script module {module} uses non-Sevii flag {target} in {relative}")
        return target
    if target.startswith("VAR_"):
        if target.startswith("VAR_WAYFARER_SEVII_"):
            return target
        if target.startswith("VAR_TEMP_") or target in TRANSIENT_VARS:
            return None
        raise ClosureError(f"script module {module} uses non-Sevii variable {target} in {relative}")
    if target in aliases:
        return None
    raise ClosureError(f"script module {module} uses unknown state operand {target} in {relative}")


def _is_constant(label: str) -> bool:
    return label.isdigit() or label.startswith(CONSTANT_PREFIXES)


def _external_rows(root: Path, module: str, rows: list[Any]) -> dict[str, dict[str, str]]:
    result: dict[str, dict[str, str]] = {}
    for row in rows:
        if not isinstance(row, dict) or not isinstance(row.get("label"), str) or not LABEL.fullmatch(row["label"]):
            raise ClosureError(f"script module {module} has an invalid allowed external")
        required = ("kind", "path", "sha256")
        if any(not isinstance(row.get(key), str) or not row[key] for key in required):
            raise ClosureError(f"script module {module} external {row['label']} must pin kind, path, and sha256")
        if row["label"] in result:
            raise ClosureError(f"script module {module} repeats external {row['label']}")
        source_path = _root_relative_path(row["path"], field=f"script module {module} external {row['label']} path")
        path = _resolved_under_root(root, source_path, field=f"script module {module} external {row['label']} path")
        if not path.is_file():
            raise ClosureError(f"script module {module} external source is missing: {row['path']}")
        actual = hashlib.sha256(path.read_bytes()).hexdigest()
        if actual != row["sha256"]:
            raise ClosureError(f"script module {module} external source drifted: {row['label']}")
        source = path.read_text(encoding="utf-8", errors="ignore")
        label = row["label"]
        kind = row["kind"]
        if kind not in {"special", "native", "script_symbol"}:
            raise ClosureError(f"script module {module} external {label} has unsupported kind {kind}")
        if kind == "special":
            if re.search(rf"(?m)^\s*def_special\s+{re.escape(label)}\b", source) is None:
                raise ClosureError(f"script module {module} special table lacks {label}")
            impl_path, impl_sha = row.get("implementation_path"), row.get("implementation_sha256")
            if not isinstance(impl_sha, str):
                raise ClosureError(f"script module {module} special {label} lacks a pinned implementation")
            impl_path = _root_relative_path(impl_path, field=f"script module {module} special {label} implementation_path")
            impl = _resolved_under_root(root, impl_path, field=f"script module {module} special {label} implementation_path")
            if not impl.is_file():
                raise ClosureError(f"script module {module} special {label} lacks a pinned implementation")
            if hashlib.sha256(impl.read_bytes()).hexdigest() != impl_sha:
                raise ClosureError(f"script module {module} special implementation drifted: {label}")
            implementation = impl.read_text(encoding="utf-8", errors="ignore")
            if re.search(rf"(?m)^\s*(?:static\s+)?(?:void|u8|u16|u32|s8|s16|s32|bool8|bool32)\s+{re.escape(label)}\s*\(", implementation) is None:
                raise ClosureError(f"script module {module} special implementation lacks {label}")
        elif kind == "native":
            if re.search(rf"(?m)^\s*(?:static\s+)?(?:void|u8|u16|u32|s8|s16|s32|bool8|bool32)\s+{re.escape(label)}\s*\(", source) is None:
                raise ClosureError(f"script module {module} native source lacks {label}")
        elif kind == "script_symbol" and re.search(rf"(?m)^{re.escape(label)}:{'{1,2}'}\s*$", source) is None:
            raise ClosureError(f"script module {module} script source lacks {label}")
        result[label] = {key: row[key] for key in ("kind", "path", "sha256")}
        if kind == "special":
            result[label].update({"implementation_path": row["implementation_path"], "implementation_sha256": row["implementation_sha256"]})
    return result


def dependency_paths(root: Path, manifest: dict[str, Any]) -> list[str]:
    """Return manifest-declared recursive script/source prerequisites.

    Validation is intentionally shared with the generator, so a new child
    include cannot bypass Make's dependency graph.
    """
    root = root.resolve()
    modules = _module_rows(manifest)
    handlers = _handler_rows(manifest)
    paths: set[str] = set()
    for module in modules.values():
        paths.update(_module_files(root, module["include"], module["owner"]))
        for external in _external_rows(root, "dependency", module["allowed_externals"]).values():
            paths.add(external["path"])
            if "implementation_path" in external:
                paths.add(external["implementation_path"])
    for handler in handlers:
        _validate_provenance(root, handler)
        if handler["source"].get("kind") != "baseline":
            paths.add(handler["source"]["include"])
    return sorted(paths)


def build_script_closure(root: Path, manifest: dict[str, Any]) -> dict[str, Any]:
    """Validate selected modules and return a deterministic dependency graph."""
    root = root.resolve()
    modules = _module_rows(manifest)
    handlers = _handler_rows(manifest)
    for handler in handlers:
        _validate_provenance(root, handler)
        module = modules.get(handler["module"])
        if module is None:
            raise ClosureError(f"{handler['source_map']} handler names unknown module {handler['module']}")
        if module["owner"] != handler["owner"]:
            raise ClosureError(f"{handler['source_map']} handler owner does not match module {handler['module']}")
        if handler["wayfarer_script"] not in module["exports"]:
            raise ClosureError(f"{handler['source_map']} handler is not exported by module {handler['module']}")

    label_owner: dict[str, str] = {}
    module_files: dict[str, list[str]] = {}
    external_by_module: dict[str, dict[str, dict[str, str]]] = {}
    dependencies: set[tuple[str, str, str]] = set()
    state_operations: set[tuple[str, str, str, str]] = set()
    content_operations: set[tuple[str, str, str, str]] = set()
    generated_map_tables = {f"{record['source_map']}_MapScripts" for record in manifest.get("maps", [])
                            if isinstance(record, dict) and isinstance(record.get("source_map"), str)}
    for name, module in modules.items():
        files = _module_files(root, module["include"], module["owner"])
        module_files[name] = files
        found: set[str] = set()
        for relative in files:
            source = (root / relative).read_text(encoding="utf-8")
            found.update(match.group(1) for match in LABEL_DEF.finditer(source))
            for line in source.splitlines():
                include = INCLUDE.match(line)
                if include:
                    child = _owned_path(include.group(1), field=f"include in {relative}", owner=modules[name]["owner"])
                    dependencies.add((relative, child, "include"))
        if found != module["exports"]:
            missing, unreviewed = sorted(module["exports"] - found), sorted(found - module["exports"])
            detail = f"missing {missing[0]}" if missing else f"unreviewed {unreviewed[0]}"
            raise ClosureError(f"script module {name} exports do not match its labels: {detail}")
        collision = sorted(found & generated_map_tables)
        if collision:
            raise ClosureError(f"script module {name} owns generated map-script table label {collision[0]}")
        for label in found:
            prior = label_owner.get(label)
            if prior is not None and prior != name:
                raise ClosureError(f"script label {label} is exported by both {prior} and {name}")
            label_owner[label] = name
        external_by_module[name] = _external_rows(root, name, module["allowed_externals"])

    for name, files in module_files.items():
        externals = external_by_module[name]
        transient_aliases: set[str] = set()
        for relative in files:
            for line in (root / relative).read_text(encoding="utf-8").splitlines():
                _validate_directive(line, relative)
                match = EQU.match(line)
                if match:
                    transient_aliases.add(match.group(1))
        for relative in files:
            current_label = "<module>"
            transient_values: dict[str, str] = {}
            for line in (root / relative).read_text(encoding="utf-8").splitlines():
                label_match = LABEL_DEF.fullmatch(line.split("@", 1)[0].strip())
                if label_match:
                    current_label = label_match.group(1)
                    transient_values = {}
                command = _script_command(line)
                if command is not None and command not in modules[name]["allowed_commands"]:
                    raise ClosureError(f"script module {name} uses unreviewed command {command} in {relative}")
                if command is not None:
                    if command in TRAINER_COMMANDS:
                        content_operations.add((name, current_label, "battle", command))
                    elif command in TRANSACTION_COMMANDS:
                        content_operations.add((name, current_label, "transaction", command))
                    for access, target in _state_operands(command, line):
                        state = _validate_state_operand(target, module=name, relative=relative, aliases=transient_aliases)
                        if state is not None:
                            state_operations.add((name, current_label, access, state))
                    operands = _operands(line)
                    if command in STATE_WRITES and operands and operands[0] in TRANSIENT_VARS:
                        if command == "setvar" and len(operands) > 1:
                            transient_values[operands[0]] = operands[1]
                        else:
                            transient_values.pop(operands[0], None)
                    if command == "specialvar" and len(operands) > 1 and operands[1] in TRANSACTION_SPECIALS:
                        special = operands[1]
                        content_operations.add((name, current_label, "transaction", special))
                        for state in _special_transaction_states(special, transient_values, module=name, relative=relative):
                            # Receipt and pending-payload reads/writes happen
                            # inside the pinned C special, after its guarded
                            # transaction ordering.  They must still appear
                            # in the contract closure as owned state access.
                            state_operations.add((name, current_label, "read", state))
                            state_operations.add((name, current_label, "write", state))
                for label, command in _line_references(line):
                    if _is_constant(label):
                        continue
                    if label in label_owner:
                        if modules[label_owner[label]]["owner"] != modules[name]["owner"]:
                            raise ClosureError(f"script module {name} reaches cross-owner helper {label} in {relative}")
                        dependencies.add((relative, label, command))
                    elif label in externals:
                        external = externals[label]
                        dependencies.add((relative, label, external["kind"]))
                    else:
                        raise ClosureError(f"script module {name} has unresolved or unreviewed dependency {label} in {relative}")

    table_rows = sorted((row["source_map"], row["handler_type"], row["wayfarer_script"], row["module"], row["owner"], row["content_id"])
                        for row in handlers)
    return {
        "includes": sorted({file for files in module_files.values() for file in files}),
        "modules": [{"module": name, "owner": modules[name]["owner"], "include": modules[name]["include"],
                     "exports": sorted(modules[name]["exports"])} for name in sorted(modules)],
        "labels": sorted(label_owner),
        "map_script_tables": [{"source_map": source_map, "handler_type": handler_type,
                               "wayfarer_script": script, "module": module, "owner": owner,
                               "content_id": content_id}
                              for source_map, handler_type, script, module, owner, content_id in table_rows],
        "dependencies": [{"from": left, "to": right, "kind": kind}
                         for left, right, kind in sorted(dependencies)],
        "external_dependencies": [{"module": module, "label": label, **value}
                                  for module in sorted(external_by_module)
                                  for label, value in sorted(external_by_module[module].items())],
        "state_operations": [{"module": module, "label": label, "access": access, "state": state}
                             for module, label, access, state in sorted(state_operations)],
        "content_operations": [{"module": module, "label": label, "kind": kind, "command": command}
                               for module, label, kind, command in sorted(content_operations)],
        "unresolved_labels": [],
    }
