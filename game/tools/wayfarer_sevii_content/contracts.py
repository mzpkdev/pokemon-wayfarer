#!/usr/bin/env python3
"""Host-only contracts shared by future Wayfarer Sevii content domains."""
from __future__ import annotations

from collections import Counter
from copy import deepcopy
import hashlib
import importlib.util
import re
import subprocess
import tempfile
from pathlib import Path
from typing import Any


class ContractError(ValueError):
    pass


SCHEMA_VERSION = 1
TRAINER_ID_BASE, TRAINER_ID_LIMIT = 1515, 2048
STATE_FLAG_BASE, STATE_FLAG_CAPACITY = 0xC000, 256
STATE_VAR_BASE, STATE_VAR_CAPACITY = 0xD000, 32
STATE_DEFEAT_BITSET = "wayfarer_sevii_trainer_defeat"
OWNERS = frozenset(("ordinary_trainer", "story", "trainer_tower"))
ALL_OWNERS = OWNERS | {"exploration"}
CLASSIFICATIONS = frozenset(("ordinary", "story", "facility"))
POLICIES = frozenset(("ordinary", "objective_guard", "facility"))
OUTCOME_POLICIES = frozenset(("defeat_and_blackout", "win_progress_loss_pending", "facility_loss"))
BATTLE_TYPES = frozenset(("single", "double"))
STORAGES = frozenset(("flag", "var", "trainer_defeat"))
IDENTIFIER = re.compile(r"^[A-Z][A-Z0-9_]*$")
CONTENT_ID = re.compile(r"^[a-z][a-z0-9_.-]*$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
TRANSACTION_STEPS = {
    "handoff": ("establish_prerequisite", "attempt_destination", "consume_source", "set_receipt", "update_presentation"),
    "grant": ("establish_prerequisite", "attempt_destination", "set_receipt", "update_presentation"),
    "claim": ("establish_prerequisite", "attempt_destination", "clear_pending", "update_presentation"),
    "service": ("establish_prerequisite", "successful_service", "consume_source", "update_presentation"),
    "staged_grant": ("establish_prerequisite", "consume_source", "set_source_receipt", "attempt_destination", "set_receipt", "update_presentation"),
}
# Persistent story flags and generic variables begin clear.  This one
# externally-visible size-record sentinel deliberately begins at the engine's
# DEFAULT_MAX_SIZE value; keeping its narrow identity here prevents an
# accidental nonzero default from becoming available to arbitrary state slots.
NONZERO_INITIAL_STATES = {
    ("SEVII_HERACROSS_SIZE_RECORD", "story", "var", 4): 0x8000,
}


def _fail(message: str) -> None:
    raise ContractError(message)


def _object(value: Any, path: str) -> dict[str, Any]:
    if not isinstance(value, dict): _fail(f"{path} must be an object")
    return value


def _array(value: Any, path: str) -> list[Any]:
    if not isinstance(value, list): _fail(f"{path} must be a list")
    return value


def _text(value: Any, path: str, pattern: re.Pattern[str] | None = None) -> str:
    if not isinstance(value, str) or not value: _fail(f"{path} must be a non-empty string")
    if pattern and not pattern.fullmatch(value): _fail(f"{path} has an invalid identifier: {value!r}")
    return value


def _number(value: Any, path: str) -> int:
    if not isinstance(value, int) or isinstance(value, bool): _fail(f"{path} must be an integer")
    return value


def _keys(row: dict[str, Any], required: set[str], path: str, optional: set[str] = frozenset()) -> None:
    missing, unknown = sorted(required - row.keys()), sorted(row.keys() - required - optional)
    if missing: _fail(f"{path} missing required keys: {', '.join(missing)}")
    if unknown: _fail(f"{path} has unknown keys: {', '.join(unknown)}")


def _source_block(root: Path, trainer: str) -> bytes:
    path = root / "src/data/trainers_frlg.party"
    try: source = path.read_text(encoding="utf-8").replace("\r\n", "\n")
    except OSError as error: _fail(f"cannot read selected-party source {path}: {error}")
    marker = f"=== {trainer} ==="
    start = source.find(marker)
    if start < 0 or (start and source[start - 1] != "\n"): _fail(f"selected Trainer source is absent: {trainer}")
    end = source.find("\n=== ", start + len(marker))
    block = source[start:] if end < 0 else source[start:end + 1]
    return "\n".join(line.rstrip() for line in block.splitlines()).strip().encode() + b"\n"


def source_party_hash(root: Path, source_trainer: str) -> str:
    """Normalized hash of one authored FRLG party source record."""
    return hashlib.sha256(_source_block(root, source_trainer)).hexdigest()


def _scaling(root: Path):
    spec = importlib.util.spec_from_file_location("wayfarer_sevii_trainer_scaling", root / "tools/trainer_scaling/generate.py")
    if spec is None or spec.loader is None: _fail("cannot load Trainer scaling parser")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _compiled_record(header: str, trainer: str, parser: Any) -> str:
    match = re.search(rf"\[DIFFICULTY_NORMAL\]\[{re.escape(trainer)}\]\s*=\s*\{{", header)
    if match is None: _fail(f"selected Trainer compiler output is absent: {trainer}")
    _, end = parser.balanced(header, match.end() - 1)
    return header[match.start():end].strip() + "\n"


def selected_trainer_render(root: Path, allocations: list[dict[str, Any]]) -> list[dict[str, Any]]:
    """Render selected source parties through trainerproc and the scaling parser."""
    if not allocations: return []
    source = root / "src/data/trainers_frlg.party"
    with tempfile.TemporaryDirectory(prefix="wayfarer-sevii-party-") as directory:
        directory = Path(directory)
        binary, output = directory / "trainerproc", directory / "trainers_frlg.h"
        def command(args: list[str], **kwargs: Any) -> str:
            result = subprocess.run(args, text=True, capture_output=True, **kwargs)
            if result.returncode: _fail(f"selected-party compiler failed: {result.stderr.strip()}")
            return result.stdout
        command(["cc", "-O2", str(root / "tools/trainerproc/main.c"), "-o", str(binary)])
        preprocessed = command(["cpp", "-traditional-cpp", "-P", "-DPOKEMON_WAYFARER", "-DPOKEMON_HNS",
                                "-DIS_WAYFARER=1", "-DIS_HNS=1", "-DIS_FRLG=0", "-DIS_EMERALD=0",
                                "-I", str(root / "include"), str(source)])
        command([str(binary), "-i", "src/data/trainers_frlg.party", "-o", str(output), "-"], input=preprocessed)
        try: header = output.read_text(encoding="utf-8")
        except OSError as error: _fail(f"cannot read selected-party parser output: {error}")
    parser = _scaling(root)
    resolved = parser.resolve_rosters(parser.parse_output(header, "src/data/trainers_frlg.party"))
    rows = []
    for row in sorted(allocations, key=lambda item: item["slot"]):
        source = row["source_trainer"]
        if source not in resolved: _fail(f"selected Trainer parser output is absent: {source}")
        compiled = _compiled_record(header, source, parser)
        rows.append({"content_id": row["content_id"], "id": row["id"], "slot": row["slot"],
                     "numeric_id": TRAINER_ID_BASE + row["slot"], "source_trainer": source,
                     "source_hash": row["source_hash"], "classification": row["classification"],
                     "battle_policy": row["battle_policy"], "defeat_state": row["defeat_state"],
                     "source_block": _source_block(root, source).decode("utf-8"),
                     "compiled_record": compiled,
                     "compiled_record_sha256": hashlib.sha256(compiled.encode()).hexdigest(),
                     "party": deepcopy(resolved[source]["DIFFICULTY_NORMAL"])})
    return rows


def _content_owners(manifest: Any) -> dict[str, str]:
    found: dict[str, str] = {}
    def visit(node: Any, path: str) -> None:
        if isinstance(node, dict):
            if "content_id" in node and "owner" in node:
                if not isinstance(node.get("content_id"), str) or not isinstance(node.get("owner"), str): _fail(f"{path} content identity requires content_id and owner")
                content_id, owner = _text(node["content_id"], f"{path}.content_id", CONTENT_ID), _text(node["owner"], f"{path}.owner")
                if owner not in ALL_OWNERS: _fail(f"{path}.owner is not a content-domain owner: {owner}")
                # Exploration retains its own baseline contract. It is not a
                # producer or consumer of this future-content namespace.
                if owner != "exploration":
                    if content_id in found: _fail(f"duplicate content ownership: {content_id}")
                    found[content_id] = owner
            for key, value in node.items():
                if not (path == "manifest" and key == "contracts"): visit(value, f"{path}.{key}")
        elif isinstance(node, list):
            for index, value in enumerate(node): visit(value, f"{path}[{index}]")
    visit(manifest, "manifest")
    return found


def _active_count(root: Path) -> int:
    try:
        source = (root / "include/constants/opponents.h").read_text(encoding="utf-8")
        hns_source = (root / "include/constants/opponents_hns.h").read_text(encoding="utf-8")
    except OSError as error: _fail(f"cannot read Trainer collision contract: {error}")
    required = re.search(r"#define\s+TRAINERS_COUNT_WAYFARER\s+\(TRAINERS_COUNT_HNS\s*\+\s*TRAINERS_COUNT_EMERALD\s*-\s*1\)", source)
    hns, emerald = re.search(r"#define\s+TRAINERS_COUNT_HNS\s+(\d+)", hns_source), re.search(r"#define\s+TRAINERS_COUNT_EMERALD\s+(\d+)", source)
    if not required or not hns or not emerald: _fail("cannot audit active Wayfarer Trainer ID count")
    return int(hns.group(1)) + int(emerald.group(1)) - 1


def _namespace(root: Path, value: Any) -> dict[str, Any]:
    row = _object(value, "contracts.state_namespace")
    _keys(row, {"name", "flag_base", "flag_capacity", "var_base", "var_capacity", "defeat_bitset", "storage"}, "contracts.state_namespace")
    expected = {"name": "wayfarer_sevii", "flag_base": STATE_FLAG_BASE, "flag_capacity": STATE_FLAG_CAPACITY,
                "var_base": STATE_VAR_BASE, "var_capacity": STATE_VAR_CAPACITY,
                "defeat_bitset": STATE_DEFEAT_BITSET, "storage": "SaveBlock3"}
    if row != expected: _fail("contracts.state_namespace must use the fixed audited Sevii SaveBlock3 reservation")
    pattern = re.compile(r"^\s*#define\s+((?:FLAG|VAR)_[A-Za-z0-9_]+)\s+(0x[0-9A-Fa-f]+)\b", re.M)
    paths = list((root / "include/constants").glob("*.h")) + list((root / "data").glob("wayfarer_*source_constants.inc"))
    for path in paths:
        try: values = pattern.findall(path.read_text(encoding="utf-8"))
        except OSError as error: _fail(f"cannot audit Sevii state reservation {path}: {error}")
        for symbol, value in values:
            numeric = int(value, 16)
            owned = symbol.startswith("FLAG_WAYFARER_SEVII_") or symbol.startswith("VAR_WAYFARER_SEVII_")
            if not owned and (STATE_FLAG_BASE <= numeric < STATE_FLAG_BASE + STATE_FLAG_CAPACITY or STATE_VAR_BASE <= numeric < STATE_VAR_BASE + STATE_VAR_CAPACITY):
                _fail(f"Sevii state namespace collides with existing constant {path}: {value}")
    return expected


def _state_symbol(storage: str, ident: str) -> str:
    suffix = ident.removeprefix("SEVII_")
    if storage == "var": return f"VAR_WAYFARER_SEVII_{suffix}"
    if storage == "flag": return f"FLAG_WAYFARER_SEVII_{suffix}"
    return f"TRAINER_WAYFARER_SEVII_DEFEAT_{suffix}"


def _state_numeric_id(storage: str, slot: int) -> int:
    if storage == "flag": return STATE_FLAG_BASE + slot
    if storage == "var": return STATE_VAR_BASE + slot
    return TRAINER_ID_BASE + slot


def _valid_state_initial(ident: str, owner: str, storage: str, slot: int, initial: int) -> bool:
    if initial == 0:
        return True
    return NONZERO_INITIAL_STATES.get((ident, owner, storage, slot)) == initial


def _states(value: Any, owners: dict[str, str]) -> dict[str, dict[str, Any]]:
    states, slots = {}, set()
    for index, raw in enumerate(_array(value, "contracts.states")):
        path, row = f"contracts.states[{index}]", _object(raw, f"contracts.states[{index}]")
        _keys(row, {"id", "owner", "storage", "slot", "initial", "readers", "writers", "transitions"}, path, {"lifecycle", "transaction_id"})
        ident, owner, storage, slot, initial = _text(row["id"], f"{path}.id", IDENTIFIER), _text(row["owner"], f"{path}.owner"), _text(row["storage"], f"{path}.storage"), _number(row["slot"], f"{path}.slot"), _number(row["initial"], f"{path}.initial")
        lifecycle = row.get("lifecycle", "completion")
        transaction_id = row.get("transaction_id")
        capacity = STATE_FLAG_CAPACITY if storage == "flag" else STATE_VAR_CAPACITY if storage == "var" else TRAINER_ID_LIMIT - TRAINER_ID_BASE
        if not ident.startswith("SEVII_") or owner not in OWNERS or storage not in STORAGES or not 0 <= slot < capacity or not _valid_state_initial(ident, owner, storage, slot, initial) or (storage, slot) in slots or ident in states: _fail(f"{path} has an invalid state identity, owner, initial value, or slot")
        if lifecycle not in ("completion", "transactional") or (lifecycle == "completion" and transaction_id is not None) or (lifecycle == "transactional" and (owner not in ("story", "trainer_tower") or storage != "var" or not isinstance(transaction_id, str))): _fail(f"{path} has an invalid state lifecycle")
        readers, writers = _array(row["readers"], f"{path}.readers"), _array(row["writers"], f"{path}.writers")
        if len(set(readers)) != len(readers) or len(set(writers)) != len(writers): _fail(f"{path} repeats a state reader or writer")
        for content in readers + writers:
            content = _text(content, f"{path}.consumer", CONTENT_ID)
            if content not in owners: _fail(f"{path} names unresolved content: {content}")
        if any(owners[content] != owner for content in writers): _fail(f"{path} permits a write outside its owner namespace")
        transitions, transition_keys = _array(row["transitions"], f"{path}.transitions"), set()
        for n, raw_transition in enumerate(transitions):
            tpath, transition = f"{path}.transitions[{n}]", _object(raw_transition, f"{path}.transitions[{n}]")
            _keys(transition, {"from", "to", "caller"}, tpath)
            before, after, caller = _number(transition["from"], f"{tpath}.from"), _number(transition["to"], f"{tpath}.to"), _text(transition["caller"], f"{tpath}.caller", CONTENT_ID)
            maximum = 1 if storage in ("flag", "trainer_defeat") else 0xFFFF
            valid_transition = after > before if lifecycle == "completion" else (after > before or (before > 0 and after == 0))
            if before < 0 or not valid_transition or after > maximum or caller not in writers or (before, after, caller) in transition_keys: _fail(f"{tpath} is not an owned legal {lifecycle} transition")
            transition_keys.add((before, after, caller))
        states[ident] = {"id": ident, "symbol": _state_symbol(storage, ident), "numeric_id": _state_numeric_id(storage, slot), "owner": owner, "storage": storage, "slot": slot, "initial": initial, "lifecycle": lifecycle, "transaction_id": transaction_id,
                         "readers": sorted(readers), "writers": sorted(writers),
                         "transitions": [{"from": before, "to": after, "caller": caller} for before, after, caller in sorted(transition_keys)]}
        slots.add((storage, slot))
    return states


def _allocations(root: Path, value: Any, owners: dict[str, str], states: dict[str, dict[str, Any]]) -> list[dict[str, Any]]:
    rows, identities, slots, content_ids = [], set(), set(), set()
    fields = {"content_id", "id", "slot", "owner", "source_trainer", "source_hash", "classification", "battle_policy", "battle_type", "outcome_policy", "defeat_state", "defeat_base"}
    for index, raw in enumerate(_array(value, "contracts.trainer_ids.allocations")):
        path, row = f"contracts.trainer_ids.allocations[{index}]", _object(raw, f"contracts.trainer_ids.allocations[{index}]")
        _keys(row, fields, path)
        content, ident, owner = _text(row["content_id"], f"{path}.content_id", CONTENT_ID), _text(row["id"], f"{path}.id", IDENTIFIER), _text(row["owner"], f"{path}.owner")
        source, digest, defeat, defeat_base = _text(row["source_trainer"], f"{path}.source_trainer", IDENTIFIER), _text(row["source_hash"], f"{path}.source_hash", SHA256), _text(row["defeat_state"], f"{path}.defeat_state", IDENTIFIER), _text(row["defeat_base"], f"{path}.defeat_base", CONTENT_ID)
        slot, classification, policy = _number(row["slot"], f"{path}.slot"), _text(row["classification"], f"{path}.classification"), _text(row["battle_policy"], f"{path}.battle_policy")
        battle_type, outcome = _text(row["battle_type"], f"{path}.battle_type"), _text(row["outcome_policy"], f"{path}.outcome_policy")
        if owners.get(content) != owner or not ident.startswith("TRAINER_WAYFARER_SEVII_") or not source.startswith("TRAINER_") or classification not in CLASSIFICATIONS or policy not in POLICIES or battle_type not in BATTLE_TYPES or outcome not in OUTCOME_POLICIES: _fail(f"{path} has unowned content or invalid Trainer fields")
        if owner == "trainer_tower" or classification == "facility" or policy == "facility": _fail(f"{path} facility opponents do not enter the persistent Sevii Trainer allocation")
        if owner == "ordinary_trainer" and (classification != "ordinary" or policy != "ordinary"): _fail(f"{path} violates shared owner/classification/battle-policy rules")
        expected_outcome = {"ordinary": "defeat_and_blackout", "objective_guard": "win_progress_loss_pending"}[policy]
        if outcome != expected_outcome: _fail(f"{path} violates shared battle outcome policy")
        if not 0 <= slot < TRAINER_ID_LIMIT - TRAINER_ID_BASE or ident in identities or slot in slots or content in content_ids: _fail(f"{path} duplicates or exceeds fixed Trainer allocation")
        if defeat not in states or states[defeat]["storage"] != "trainer_defeat" or states[defeat]["owner"] != owner: _fail(f"{path}.defeat_state must be an owned Sevii defeat bit")
        if source_party_hash(root, source) != digest: _fail(f"{path} source drift for {source}")
        identities.add(ident); slots.add(slot); content_ids.add(content); rows.append(row)
    by_content = {row["content_id"]: row for row in rows}
    for row in rows:
        base = by_content.get(row["defeat_base"])
        if base is None or base["owner"] != row["owner"] or base["defeat_base"] != base["content_id"]:
            _fail(f"allocation {row['content_id']} has an invalid defeat base")
        state = states[row["defeat_state"]]
        if state["slot"] != base["slot"] or state["id"] != base["defeat_state"] or base["content_id"] not in state["writers"]:
            _fail(f"allocation {row['content_id']} does not route through its base defeat state")
    rendered = selected_trainer_render(root, rows)
    for row, output in zip(sorted(rows, key=lambda item: item["slot"]), rendered):
        expected_type = {"TRAINER_BATTLE_TYPE_SINGLES": "single", "TRAINER_BATTLE_TYPE_DOUBLES": "double"}.get(output["party"].get("battleType"))
        if expected_type is None or row["battle_type"] != expected_type:
            _fail(f"allocation {row['content_id']} battle_type does not match its preserved source party")
    return [{"content_id": r["content_id"], "id": r["id"], "slot": r["slot"], "numeric_id": TRAINER_ID_BASE + r["slot"], "owner": r["owner"], "source_trainer": r["source_trainer"], "source_hash": r["source_hash"], "classification": r["classification"], "battle_policy": r["battle_policy"], "battle_type": r["battle_type"], "outcome_policy": r["outcome_policy"], "defeat_state": r["defeat_state"], "defeat_base": r["defeat_base"]} for r in sorted(rows, key=lambda r: r["slot"])]


def _transactions(value: Any, owners: dict[str, str], states: dict[str, dict[str, Any]]) -> list[dict[str, Any]]:
    result, seen = [], set()
    fields = {"content_id", "owner", "kind", "prerequisite", "destination", "consume", "receipt", "steps"}
    for index, raw in enumerate(_array(value, "contracts.transactions")):
        path, row = f"contracts.transactions[{index}]", _object(raw, f"contracts.transactions[{index}]")
        _keys(row, fields, path, {"clear_payloads", "pending_state", "source_receipt"})
        content, owner, kind = _text(row["content_id"], f"{path}.content_id", CONTENT_ID), _text(row["owner"], f"{path}.owner"), _text(row["kind"], f"{path}.kind")
        receipt = row["receipt"]
        if owners.get(content) != owner or owner == "ordinary_trainer" or kind not in TRANSACTION_STEPS: _fail(f"{path} has unresolved content, an ordinary transaction, or an invalid kind")
        if kind == "claim":
            pending = _text(row.get("pending_state"), f"{path}.pending_state", IDENTIFIER)
            if receipt is not None or pending not in states or states[pending]["lifecycle"] != "transactional" or states[pending]["owner"] != owner or states[pending]["transaction_id"] != content or content not in states[pending]["writers"]:
                _fail(f"{path} has an invalid repeatable pending claim")
            if not any(transition["from"] > 0 and transition["to"] == 0 and transition["caller"] == content for transition in states[pending]["transitions"]):
                _fail(f"{path} pending claim cannot clear its payload after success")
        elif kind == "service":
            if receipt is not None or row.get("pending_state") is not None or row.get("clear_payloads", []):
                _fail(f"{path} has an invalid repeatable service receipt or payload")
        else:
            if row.get("pending_state") is not None: _fail(f"{path}.pending_state is reserved for a repeatable claim")
            receipt = _text(receipt, f"{path}.receipt", IDENTIFIER)
            if receipt not in states or states[receipt]["storage"] == "trainer_defeat" or states[receipt]["lifecycle"] != "completion" or states[receipt]["owner"] != owner or content not in states[receipt]["writers"]: _fail(f"{path} has an invalid receipt")
        source_receipt = row.get("source_receipt")
        if kind == "staged_grant":
            if row.get("pending_state") is not None or row.get("clear_payloads", []):
                _fail(f"{path} has an invalid staged grant payload")
            source_receipt = _text(source_receipt, f"{path}.source_receipt", IDENTIFIER)
            if source_receipt == receipt or source_receipt not in states or states[source_receipt]["storage"] == "trainer_defeat" or states[source_receipt]["lifecycle"] != "completion" or states[source_receipt]["owner"] != owner or content not in states[source_receipt]["writers"]:
                _fail(f"{path} has an invalid source receipt")
        elif source_receipt is not None:
            _fail(f"{path}.source_receipt is reserved for a staged grant")
        if not isinstance(row["prerequisite"], list) or not row["prerequisite"] or not all(isinstance(item, str) and item for item in row["prerequisite"]): _fail(f"{path}.prerequisite must be a reviewed non-empty list")
        clears = _array(row.get("clear_payloads", []), f"{path}.clear_payloads")
        if len(set(clears)) != len(clears): _fail(f"{path}.clear_payloads repeats a state")
        for state_id in clears:
            state_id = _text(state_id, f"{path}.clear_payloads", IDENTIFIER)
            if state_id not in states or states[state_id]["lifecycle"] != "transactional" or states[state_id]["owner"] != owner or states[state_id]["transaction_id"] != content:
                _fail(f"{path}.clear_payloads has an unreviewed transactional state")
            if kind == "claim" and state_id == row.get("pending_state"):
                _fail(f"{path}.clear_payloads repeats the claim pending state")
        expected_steps = list(TRANSACTION_STEPS[kind])
        if clears: expected_steps.insert(-1, "clear_payload")
        allowed_destinations = ("service",) if kind == "service" else ("bag", "party", "pc")
        if row["destination"] not in allowed_destinations or row["steps"] != expected_steps: _fail(f"{path} violates destination or transaction ordering")
        if (kind in ("handoff", "service", "staged_grant") and (not isinstance(row["consume"], str) or not row["consume"])) or (kind in ("grant", "claim") and row["consume"] is not None): _fail(f"{path} has invalid source consumption")
        if content in seen: _fail(f"{path} duplicates a content transaction")
        normalized = deepcopy(row); normalized["clear_payloads"] = sorted(clears); normalized["pending_state"] = row.get("pending_state"); normalized["source_receipt"] = source_receipt
        seen.add(content); result.append(normalized)
    transaction_ids = {row["content_id"] for row in result}
    cleared_payloads = {state_id for row in result for state_id in row["clear_payloads"]}
    cleared_payloads.update(row["pending_state"] for row in result if row["kind"] == "claim")
    for state in states.values():
        if state["lifecycle"] == "transactional" and state["transaction_id"] not in transaction_ids:
            _fail(f"transactional state {state['id']} has no declared successful transaction")
        if state["lifecycle"] == "transactional" and state["id"] not in cleared_payloads:
            _fail(f"transactional state {state['id']} is not cleared by its successful transaction")
    return sorted(result, key=lambda r: r["content_id"])


def _projected_access(manifest: Any, owners: dict[str, str], states: dict[str, dict[str, Any]], allocations: list[dict[str, Any]]) -> None:
    allocation_by_content = {row["content_id"]: row for row in allocations}
    def visit(node: Any) -> None:
        if isinstance(node, dict):
            content = node.get("content_id")
            if isinstance(content, str) and content in owners:
                allocation = allocation_by_content.get(content)
                if "battle_type" in node or "outcome_policy" in node:
                    if allocation is None or node.get("battle_type") != allocation["battle_type"] or node.get("outcome_policy") != allocation["outcome_policy"]:
                        _fail(f"content {content} battle type or outcome does not match its Trainer allocation")
                for key, writes in (("state_reads", False), ("state_writes", True)):
                    for state in _array(node.get(key, []), f"content {content}.{key}"):
                        state = _text(state, f"content {content}.{key}", IDENTIFIER)
                        if state not in states: _fail(f"content {content} references unknown state {state}")
                        if content not in states[state]["writers" if writes else "readers"]: _fail(f"content {content} has an undeclared state {'write' if writes else 'read'}: {state}")
                        if writes and states[state]["owner"] != owners[content]: _fail(f"content {content} writes outside owner namespace: {state}")
                        if writes and owners[content] == "ordinary_trainer" and states[state]["storage"] != "trainer_defeat": _fail(f"ordinary content {content} writes outside its defeat bitset: {state}")
            for key, value in node.items():
                if key != "contracts": visit(value)
        elif isinstance(node, list):
            for value in node: visit(value)
    visit(manifest)


def validate_contracts(root: Path, manifest: dict[str, Any]) -> dict[str, Any]:
    """Return a deterministic report or raise ContractError for invalid declarations."""
    root, manifest = Path(root), _object(manifest, "manifest")
    contracts = _object(manifest.get("contracts"), "manifest.contracts")
    _keys(contracts, {"schema_version", "trainer_ids", "state_namespace", "states", "transactions"}, "manifest.contracts")
    if _number(contracts["schema_version"], "contracts.schema_version") != SCHEMA_VERSION: _fail("unsupported Sevii contracts schema version")
    trainer_ids = _object(contracts["trainer_ids"], "contracts.trainer_ids")
    _keys(trainer_ids, {"base", "limit", "allocations"}, "contracts.trainer_ids")
    if trainer_ids["base"] != TRAINER_ID_BASE or trainer_ids["limit"] != TRAINER_ID_LIMIT: _fail("contracts.trainer_ids must use the fixed collision-audited Sevii range")
    active_count = _active_count(root)
    if active_count - 1 >= TRAINER_ID_BASE: _fail("active Wayfarer Trainer IDs collide with the fixed Sevii allocation")
    owners, namespace = _content_owners(manifest), _namespace(root, contracts["state_namespace"])
    states = _states(contracts["states"], owners)
    allocations = _allocations(root, trainer_ids["allocations"], owners, states)
    transactions = _transactions(contracts["transactions"], owners, states)
    _projected_access(manifest, owners, states, allocations)
    return {"schema_version": SCHEMA_VERSION,
            "trainer_ids": {"base": TRAINER_ID_BASE, "limit": TRAINER_ID_LIMIT, "active_count": active_count, "allocation_count": len(allocations), "allocations": allocations},
            "state_namespace": namespace, "states": [states[key] for key in sorted(states)], "transactions": transactions,
            "counts": {"owners": dict(sorted(Counter(owners.values()).items())), "states": len(states), "transactions": len(transactions)}}
