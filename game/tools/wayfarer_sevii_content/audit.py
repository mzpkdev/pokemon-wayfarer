#!/usr/bin/env python3
"""Build the deterministic Wayfarer Sevii content-overlay audit.

This is deliberately a host-side proof.  It validates the schema, script
closure, and runtime contracts first, then compares the immutable exploration,
wild-encounter, and event-island surfaces with the accepted pre-overlay
baseline.  Later content can add enabled domain records without relaxing the
frozen exploration projection.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import importlib.util
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable


TOOL_DIR = Path(__file__).resolve().parent
if str(TOOL_DIR) not in sys.path:
    sys.path.insert(0, str(TOOL_DIR))

import closure
import contracts
import schema


EVENT_KINDS = ("object_events", "coord_events", "bg_events")
GEOMETRY_EVENT_KINDS = ("warp_events", "connections")
SOURCELESS_SCRIPTS = {"", "0", "0x0", "NULL"}
ROM_START = 0x08000000
ROM_CAPACITY_END = 0x0A000000
WAYFARER_RELEASE_LIMIT = 0x09F80000
REQUIRED_RESERVE_BYTES = 512 * 1024
LABEL_DEFINITION = re.compile(r"(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}\s*(?:@.*)?$")
SCRIPT_COMMAND = re.compile(r"(?m)^\s*([a-z][a-z0-9_]*)\b")
WAYFARER_LABEL = re.compile(r"\bWayfarerSevii_[A-Za-z0-9_]+\b")
STATE_WRITE_COMMANDS = frozenset(("setflag", "clearflag", "setvar", "addvar", "subvar", "copyvar"))
TRAINER_COMMANDS = frozenset(closure.TRAINER_COMMANDS)
TRANSACTION_COMMANDS = frozenset(closure.TRANSACTION_COMMANDS)


class AuditError(ValueError):
    """A content-overlay invariant or accepted baseline check failed."""


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=True, sort_keys=True, separators=(",", ":")).encode("utf-8")


def digest(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def file_digest(path: Path) -> str:
    try:
        if path.suffix == ".json":
            return digest(load_json(path))
        return hashlib.sha256(path.read_bytes().replace(b"\r\n", b"\n")).hexdigest()
    except FileNotFoundError as error:
        raise AuditError(f"missing required file: {path}") from error


def load_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as error:
        raise AuditError(f"missing required file: {path}") from error
    except json.JSONDecodeError as error:
        raise AuditError(f"invalid JSON in {path}: {error}") from error


def relative(path: Path, root: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def source_map_path(root: Path, source_map: str) -> Path:
    return root / "data/maps" / source_map / "map.json"


def active_domains(manifest: dict[str, Any]) -> dict[str, dict[str, Any]]:
    domains = manifest.get("content_domains")
    if not isinstance(domains, dict):
        raise AuditError("manifest content_domains must be an object")
    return domains


def selected_records(manifest: dict[str, Any], domains: Iterable[str] | None = None) -> list[dict[str, Any]]:
    try:
        return schema.selected_records(manifest, domains)
    except (schema.SchemaError, ValueError) as error:
        raise AuditError(str(error)) from error


def _output_event(row: dict[str, Any]) -> dict[str, Any]:
    """Return the mapjson-visible event identity without retaining future data.

    The projection boundary copies the reviewed source event and applies only
    its declared replacement script and approved overrides.  Keeping this
    implementation here makes the baseline independent of mutable mapjson
    implementation details while still detecting a changed projected event.
    """
    source = row.get("source")
    if not isinstance(source, dict):
        raise AuditError(f"{row.get('content_id', '<unknown>')}: source must be an object")
    output = copy.deepcopy(source)
    source_script = str(source.get("script", ""))
    replacement = row.get("wayfarer_script")
    if source_script not in SOURCELESS_SCRIPTS:
        if not isinstance(replacement, str) or not replacement:
            raise AuditError(f"{row.get('content_id', '<unknown>')}: missing Wayfarer replacement script")
        output["script"] = replacement
    overrides = row.get("overrides", {})
    if overrides is None:
        overrides = {}
    if not isinstance(overrides, dict):
        raise AuditError(f"{row.get('content_id', '<unknown>')}: overrides must be an object")
    output.update(overrides)
    return output


def _record_rows_by_map(records: list[dict[str, Any]]) -> dict[tuple[str, str], list[dict[str, Any]]]:
    rows: dict[tuple[str, str], list[dict[str, Any]]] = defaultdict(list)
    for row in records:
        kind = row.get("event_kind")
        if kind not in (*EVENT_KINDS, "map_scripts"):
            raise AuditError(f"{row.get('content_id', '<unknown>')}: unsupported selected event kind {kind!r}")
        rows[(str(row["source_map"]), str(kind))].append(row)
    for bucket in rows.values():
        bucket.sort(key=lambda row: (int(row.get("index", row.get("source_index", -1))), str(row["content_id"])))
    return rows


def exploration_projection(root: Path, manifest: dict[str, Any]) -> dict[str, Any]:
    """Canonical immutable exploration events, internal geography, and layouts."""
    records = selected_records(manifest, ("exploration",))
    rows_by_map = _record_rows_by_map(records)
    maps = manifest.get("maps")
    if not isinstance(maps, list):
        raise AuditError("manifest maps must be a list")
    map_ids = {entry.get("map_id") for entry in maps if isinstance(entry, dict)}
    if len(map_ids) != len(maps) or None in map_ids:
        raise AuditError("manifest maps must have unique map_id values")

    layouts_document = load_json(root / "data/layouts/layouts.json")
    layout_by_id = {entry.get("id"): entry for entry in layouts_document.get("layouts", []) if isinstance(entry, dict)}
    map_rows: list[dict[str, Any]] = []
    layout_ids: set[str] = set()
    raw_layout_bytes = 0

    for entry in maps:
        if not isinstance(entry, dict):
            raise AuditError("manifest map must be an object")
        name = entry.get("source_map")
        map_id = entry.get("map_id")
        layout_id = entry.get("layout")
        if not all(isinstance(value, str) and value for value in (name, map_id, layout_id)):
            raise AuditError("manifest map lacks source_map, map_id, or layout")
        source = load_json(source_map_path(root, name))
        if source.get("id") != map_id or source.get("layout") != layout_id:
            raise AuditError(f"{name}: map identity differs from manifest")
        layout = layout_by_id.get(layout_id)
        if not isinstance(layout, dict):
            raise AuditError(f"{name}: referenced layout is missing: {layout_id}")
        layout_ids.add(layout_id)

        projected_events: list[dict[str, Any]] = []
        for kind in EVENT_KINDS:
            source_events = source.get(kind, [])
            if not isinstance(source_events, list):
                raise AuditError(f"{name}: source {kind} must be a list")
            for row in rows_by_map.get((name, kind), []):
                index = row.get("index")
                if not isinstance(index, int) or index < 0 or index >= len(source_events):
                    raise AuditError(f"{row['content_id']}: source index is invalid")
                if row.get("source") != source_events[index]:
                    raise AuditError(f"{row['content_id']}: selected source event drifted")
                projected_events.append({
                    "event_kind": kind,
                    "index": index,
                    "content_id": row["content_id"],
                    "source_sha256": digest(row["source"]),
                    "output_sha256": digest(_output_event(row)),
                })

        handlers = []
        for row in rows_by_map.get((name, "map_scripts"), []):
            handlers.append({
                "index": row.get("index", row.get("source_index")),
                "content_id": row["content_id"],
                "source_sha256": digest(row.get("source")),
                "output_sha256": digest({
                    "wayfarer_script": row.get("wayfarer_script"),
                    "handler_type": row.get("handler_type"),
                }),
            })

        map_metadata = {key: value for key, value in source.items() if key not in (*EVENT_KINDS, *GEOMETRY_EVENT_KINDS)}
        source_warps = source.get("warp_events") or []
        source_connections = source.get("connections") or []
        if not isinstance(source_warps, list) or not isinstance(source_connections, list):
            raise AuditError(f"{name}: source geography must use event lists")
        internal_warps = [
            {"index": index, "sha256": digest(event)}
            for index, event in enumerate(source_warps)
            if isinstance(event, dict) and event.get("dest_map") in (*map_ids, "MAP_DYNAMIC", "MAP_UNDEFINED")
        ]
        internal_connections = [
            {"index": index, "sha256": digest(event)}
            for index, event in enumerate(source_connections)
            if isinstance(event, dict) and event.get("map") in map_ids
        ]
        map_rows.append({
            "source_map": name,
            "map_id": map_id,
            "layout": layout_id,
            "category": entry.get("category"),
            "metadata_sha256": digest(map_metadata),
            "events": projected_events,
            "map_scripts": handlers,
            "warps": internal_warps,
            "connections": internal_connections,
        })

    layouts: list[dict[str, Any]] = []
    for layout_id in sorted(layout_ids):
        layout = layout_by_id[layout_id]
        sources = []
        for key in ("blockdata_filepath", "border_filepath"):
            value = layout.get(key)
            if not isinstance(value, str) or not value:
                raise AuditError(f"{layout_id}: missing {key}")
            path = root / value
            try:
                bytes_count = path.stat().st_size
            except FileNotFoundError as error:
                raise AuditError(f"{layout_id}: missing {value}") from error
            raw_layout_bytes += bytes_count
            sources.append({"path": value, "bytes": bytes_count, "sha256": file_digest(path)})
        layouts.append({
            "layout": layout_id,
            "metadata_sha256": digest({key: value for key, value in layout.items() if key not in ("blockdata_filepath", "border_filepath")}),
            "source_files": sources,
        })
    return {
        "maps": map_rows,
        "layouts": layouts,
        "map_count": len(map_rows),
        "layout_count": len(layouts),
        "raw_layout_bytes": raw_layout_bytes,
    }


def _load_port_audit(root: Path) -> Any:
    """Load the existing protected event-island audit without duplicating it."""
    path = root / "tools/wayfarer_sevii_port/audit.py"
    if not path.is_file():
        raise AuditError(f"missing existing event-island audit: {relative(path, root)}")
    spec = importlib.util.spec_from_file_location("wayfarer_sevii_port_event_island_audit", path)
    if spec is None or spec.loader is None:
        raise AuditError("cannot load existing event-island audit")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def event_island_report(root: Path) -> dict[str, Any]:
    baseline = root / "tools/wayfarer_sevii_port/event_island_baseline.json"
    module = _load_port_audit(root)
    try:
        report = module.validate_event_island_baseline(root, baseline)
    except Exception as error:  # The legacy module owns exact protected-hook diagnostics.
        raise AuditError(str(error)) from error
    return {"sha256": file_digest(baseline), "report": report}


def content_inventory_report(manifest: dict[str, Any]) -> dict[str, Any]:
    domains = active_domains(manifest)
    # The schema always preserves enabled exploration when a development
    # selection asks for another domain.  Ask once so it cannot be counted once
    # per domain while all disabled milestone-one domains are inspected.
    all_records = selected_records(manifest)
    all_records.sort(key=lambda row: (row["domain"], row["source_map"], row["event_kind"], int(row.get("index", -1)), row["content_id"]))

    selected = []
    counts = {
        "domain": Counter(), "owner": Counter(), "map": Counter(), "event_kind": Counter(),
        "actor_role": Counter(), "battle_policy": Counter(), "reward_type": Counter(),
    }


    for row in all_records:
        source = row.get("source")
        selected.append({
            "content_id": row["content_id"], "domain": row["domain"], "owner": row["owner"],
            "source_map": row["source_map"], "map_id": row["map_id"], "event_kind": row["event_kind"],
            "index": row.get("index"), "source_sha256": digest(source),
            "output_sha256": digest(_output_event(row)) if row["event_kind"] in EVENT_KINDS else digest({
                "wayfarer_script": row.get("wayfarer_script"), "handler_type": row.get("handler_type"),
            }),
        })
        counts["domain"][row["domain"]] += 1
        counts["owner"][row["owner"]] += 1
        counts["map"][row["source_map"]] += 1
        counts["event_kind"][row["event_kind"]] += 1
        for key in ("actor_role", "battle_policy", "reward_type"):
            value = row.get(key)
            if value is not None:
                counts[key][str(value)] += 1

    exclusions = []
    for exclusion in manifest.get("exclusions", []):
        if not isinstance(exclusion, dict):
            raise AuditError("manifest exclusion must be an object")
        identities = exclusion.get("source_identities")
        if not isinstance(identities, list):
            raise AuditError("manifest exclusion must list source_identities")
        for identity in identities:
            if not isinstance(identity, dict):
                raise AuditError("manifest exclusion source identity must be an object")
            exclusions.append({
                "content_id": exclusion.get("content_id"), "source_map": identity.get("source_map"),
                "event_kind": identity.get("event_kind"), "index": identity.get("index"),
                "owner": exclusion.get("owner"), "reason": exclusion.get("reason"),
                "source_sha256": digest(identity.get("source")) if "source" in identity else None,
            })
    exclusions.sort(key=lambda row: (str(row["source_map"]), str(row["event_kind"]), str(row["index"]), str(row["reason"])))
    return {
        "domains": [{
            "domain": name, "owner": data.get("owner"), "enabled": data.get("enabled"),
            "inventory_ids": sorted(data.get("inventory", [])),
        } for name, data in sorted(domains.items())],
        "selected": selected,
        "excluded": exclusions,
        "counts": {name: dict(sorted(counter.items())) for name, counter in counts.items()},
    }


def _script_label_bodies(root: Path, closure_report: dict[str, Any]) -> dict[str, str]:
    """Read label bodies from the closure-validated owned include set."""
    bodies: dict[str, str] = {}
    for include in closure_report.get("includes", []):
        path = root / include
        try:
            source = path.read_text(encoding="utf-8").replace("\r\n", "\n")
        except OSError as error:
            raise AuditError(f"cannot read closure include {include}: {error}") from error
        definitions = list(LABEL_DEFINITION.finditer(source))
        for index, match in enumerate(definitions):
            label = match.group(1)
            end = definitions[index + 1].start() if index + 1 < len(definitions) else len(source)
            if label in bodies:
                raise AuditError(f"closure has duplicate label body {label}")
            bodies[label] = source[match.start():end]
    return bodies


def _reachable_script_labels(entry: str, bodies: dict[str, str]) -> list[str]:
    if entry not in bodies:
        raise AuditError(f"selected content entry point is absent from the owned closure: {entry}")
    pending, visited = [entry], set()
    while pending:
        label = pending.pop()
        if label in visited:
            continue
        visited.add(label)
        pending.extend(target for target in WAYFARER_LABEL.findall(bodies[label]) if target in bodies and target not in visited)
    return sorted(visited)


def _label_graph(labels: list[str], bodies: dict[str, str]) -> dict[str, set[str]]:
    available = set(labels)
    return {label: {target for target in WAYFARER_LABEL.findall(bodies[label]) if target in available}
            for label in labels}


def _labels_reachable(starts: set[str], targets: set[str], graph: dict[str, set[str]],
                      blocked: set[str] = frozenset()) -> bool:
    pending, seen = list(starts - blocked), set()
    while pending:
        label = pending.pop()
        if label in seen or label in blocked:
            continue
        if label in targets:
            return True
        seen.add(label)
        pending.extend(graph.get(label, set()) - seen - blocked)
    return False


def _service_payment_follows_successful_relearner(entry: str, labels: list[str],
                                                   bodies: dict[str, str]) -> bool:
    """Prove the Move Maniac's payment paths follow a successful teach action.

    This is deliberately source-faithful rather than a general control-flow
    theorem: the reviewed repeatable service is the Move Relearner special,
    whose successful completion is reported in ``VAR_0x8004``.  A payment
    label must not be reachable before that special, and its success branch
    must be able to reach every removal label.
    """
    graph = _label_graph(labels, bodies)
    service_labels = {label for label in labels
                      if re.search(r"(?m)^\s*special\s+TeachMoveRelearnerMove\b", bodies[label])}
    payment_labels = {label for label in labels
                      if re.search(r"(?m)^\s*removeitem\b", bodies[label])}
    if not service_labels or not payment_labels:
        return False
    if _labels_reachable({entry}, payment_labels, graph, service_labels):
        return False

    success_sources: set[str] = set()
    for label in service_labels:
        lines = bodies[label].splitlines()
        special_index = next((index for index, line in enumerate(lines)
                              if re.fullmatch(r"\s*special\s+TeachMoveRelearnerMove\s*(?:@.*)?", line)), None)
        if special_index is None:
            continue
        guard_index = next((index for index in range(special_index + 1, len(lines))
                            if re.fullmatch(r"\s*goto_if_eq\s+VAR_0x8004\s*,\s*0\s*,\s*WayfarerSevii_[A-Za-z0-9_]+\s*(?:@.*)?", lines[index])), None)
        if guard_index is None:
            continue
        success_sources.update(target for target in WAYFARER_LABEL.findall("\n".join(lines[guard_index + 1:]))
                               if target in graph)
    return bool(success_sources) and all(
        _labels_reachable(success_sources, {payment}, graph) for payment in payment_labels)


def _battle_types(source: str, operations: list[dict[str, Any]]) -> list[str]:
    """Return the script form used by each owned Trainer battle command.

    The explicit single/double opcodes are the preferred source of truth.  A
    retained generic ``trainerbattle`` remains supported only when its line
    makes the battle type explicit, so an allocation cannot silently change
    the selected-party battle format.
    """
    result: list[str] = []
    commands = [operation["command"] for operation in operations if operation.get("kind") == "battle"]
    if not operations:
        commands = [command for command in SCRIPT_COMMAND.findall(source) if command in TRAINER_COMMANDS]
    for command in commands:
        if command == "trainerbattle_single":
            result.append("single")
        elif command == "trainerbattle_double":
            result.append("double")
        elif command == "trainerbattle":
            lines = [line.upper() for line in source.splitlines()
                     if line.split("@", 1)[0].strip().startswith("trainerbattle ")]
            for line in lines:
                if "DOUBLE" in line:
                    result.append("double")
                elif "SINGLE" in line:
                    result.append("single")
                else:
                    raise AuditError("generic trainerbattle must name SINGLE or DOUBLE")
        else:  # The closure owns the command set; do not accept an untyped extension here.
            raise AuditError(f"unreviewed Trainer battle command {command}")
    return result


def _script_command_token(line: str) -> str | None:
    """Return a command token without treating a comment-only line as code."""
    code = line.split("@", 1)[0].strip()
    return code.split(maxsplit=1)[0] if code else None


def validate_contract_closure(root: Path, manifest: dict[str, Any], closure_report: dict[str, Any],
                              contracts_report: dict[str, Any]) -> dict[str, Any]:
    """Link host contract declarations to selected owned script command surfaces.

    The audit proves that declared state, battle, and item/Pokemon transaction
    surfaces have a matching owned command closure. Runtime atomicity remains a
    consumer-milestone mechanics responsibility and is reported as such.
    """
    bodies = _script_label_bodies(root, closure_report)
    states = {row["id"]: row for row in contracts_report.get("states", [])}
    allocations = {row["content_id"]: row for row in contracts_report.get("trainer_ids", {}).get("allocations", [])}
    transactions = {row["content_id"]: row for row in contracts_report.get("transactions", [])}
    reports = []
    for row in (row for row in selected_records(manifest) if row.get("owner") != "exploration"):
        content_id, entry = row["content_id"], row.get("wayfarer_script")
        if not isinstance(entry, str) or not entry.startswith("WayfarerSevii_"):
            raise AuditError(f"{content_id}: non-exploration content lacks a Wayfarer-owned script entry")
        labels = _reachable_script_labels(entry, bodies)
        source = "\n".join(bodies[label] for label in labels)
        commands = sorted(set(SCRIPT_COMMAND.findall(source)))
        symbols = {state_id: state["symbol"] for state_id, state in states.items()}
        declared_reads, declared_writes = set(row.get("state_reads", [])), set(row.get("state_writes", []))
        unknown = (declared_reads | declared_writes) - set(symbols)
        if unknown:
            raise AuditError(f"{content_id}: contract closure names unknown state {sorted(unknown)[0]}")
        closure_operations = [operation for operation in closure_report.get("state_operations", [])
                              if operation.get("label") in labels]
        content_operations = [operation for operation in closure_report.get("content_operations", [])
                              if operation.get("label") in labels]
        if closure_operations:
            written = {operation["state"] for operation in closure_operations if operation.get("access") == "write"}
            read = {operation["state"] for operation in closure_operations if operation.get("access") == "read"}
        else:
            written = {
                symbol for symbol in symbols.values()
                if any(_script_command_token(line) in STATE_WRITE_COMMANDS and symbol in line
                       for line in source.splitlines())
            }
            read = {
                symbol for symbol in symbols.values()
                if any(_script_command_token(line) in closure.STATE_READS and symbol in line
                       for line in source.splitlines())
            }
        # Defeat-bit transitions are engine-owned battle outcomes, not event
        # script flag operations.  Every other declared state must appear in
        # the owned script operation graph.
        script_reads = {state for state in declared_reads if states[state].get("storage") != "trainer_defeat"}
        script_writes = {state for state in declared_writes if states[state].get("storage") != "trainer_defeat"}
        expected_writes = {symbols[state] for state in script_writes}
        missing_writes, undeclared_writes = expected_writes - written, written - expected_writes
        if missing_writes:
            raise AuditError(f"{content_id}: declared state write has no owned script command: {sorted(missing_writes)[0]}")
        if undeclared_writes:
            raise AuditError(f"{content_id}: owned script writes undeclared state {sorted(undeclared_writes)[0]}")
        expected_reads = {symbols[state] for state in script_reads}
        missing_reads, undeclared_reads = expected_reads - read, read - expected_reads
        if missing_reads:
            raise AuditError(f"{content_id}: declared state read has no owned script reference: {sorted(missing_reads)[0]}")
        if undeclared_reads:
            raise AuditError(f"{content_id}: owned script reads undeclared state {sorted(undeclared_reads)[0]}")
        allocation = allocations.get(content_id)
        battles = _battle_types(source, content_operations)
        if battles:
            if allocation is None:
                raise AuditError(f"{content_id}: unowned Trainer battle caller")
            if allocation["id"] not in source:
                raise AuditError(f"{content_id}: Trainer battle does not reference its allocated ID")
            if (row.get("battle_type") != allocation["battle_type"]
                    or row.get("outcome_policy") != allocation["outcome_policy"]):
                raise AuditError(f"{content_id}: event battle_type or outcome_policy differs from its Trainer allocation")
            if battles != [allocation["battle_type"]]:
                raise AuditError(f"{content_id}: Trainer battle command form does not match allocated battle_type")
        elif allocation is not None:
            raise AuditError(f"{content_id}: allocated Trainer has no owned battle command")
        transaction = transactions.get(content_id)
        transaction_commands = [operation["command"] for operation in content_operations
                                if operation.get("kind") == "transaction"]
        if not content_operations:
            transaction_commands = [command for command in commands if command in TRANSACTION_COMMANDS]
        if transaction_commands and transaction is None:
            raise AuditError(f"{content_id}: item or Pokemon command has no transaction declaration")
        if transaction is not None and not transaction_commands:
            raise AuditError(f"{content_id}: declared transaction has no owned item or Pokemon command")
        if transaction is not None:
            special_kinds = {kind for command in transaction_commands
                             if (kind := closure.transaction_kind(command)) is not None}
            if special_kinds and special_kinds != {transaction["kind"]}:
                raise AuditError(f"{content_id}: atomic transaction special does not match declared transaction kind")
            receipt = transaction.get("receipt")
            if transaction["kind"] == "service":
                if not _service_payment_follows_successful_relearner(entry, labels, bodies):
                    raise AuditError(f"{content_id}: repeatable service payment can precede successful service")
            elif receipt is None:
                pending = transaction.get("pending_state")
                if not isinstance(pending, str) or pending not in states:
                    raise AuditError(f"{content_id}: repeatable claim lacks a resolved pending state")
                if states[pending]["symbol"] not in written:
                    raise AuditError(f"{content_id}: repeatable claim does not clear its pending state")
                for state_id in transaction.get("clear_payloads", []):
                    if states[state_id]["symbol"] not in written:
                        raise AuditError(f"{content_id}: repeatable claim does not clear payload state {state_id}")
            elif states[receipt]["symbol"] not in written:
                raise AuditError(f"{content_id}: transaction receipt is not written by its owned script")
        if row["owner"] == "ordinary_trainer":
            if len(battles) != 1:
                raise AuditError(f"{content_id}: ordinary Trainer content must have exactly one owned battle command")
            if transaction is not None or transaction_commands:
                raise AuditError(f"{content_id}: ordinary Trainer content cannot declare an item or Pokemon transaction")
            ordinary_states = declared_reads | declared_writes
            forbidden = [state for state in ordinary_states
                         if states[state].get("owner") != "ordinary_trainer"
                         or states[state].get("storage") != "trainer_defeat"]
            if forbidden:
                raise AuditError(f"{content_id}: ordinary Trainer content cannot use story state {sorted(forbidden)[0]}")
        reports.append({"content_id": content_id, "owner": row["owner"], "entry": entry,
                        "reachable_labels": labels, "commands": commands,
                        "state_reads": sorted(declared_reads), "state_writes": sorted(declared_writes),
                        "trainer_allocation": allocation["id"] if allocation else None,
                        "battle_commands": battles,
                        "transaction": transaction["kind"] if transaction else None,
                        "transaction_commands": sorted(transaction_commands),
                        "pending_state": transaction.get("pending_state") if transaction else None})
    return {"entries": reports, "transaction_order": {
        "host_declaration_validated": True,
        "runtime_semantics": "future content must prove transaction ordering and recovery with mechanics tests",
    }}


def standalone_projection_report(root: Path) -> dict[str, Any]:
    """Report source identities that standalone map projections consume.

    These records deliberately exclude the mutable mapjson executable.  The
    corresponding mapjson fixture tests prove renderer behavior; this audit
    pins the immutable catalog identities that standalone products project.
    """
    groups = load_json(root / "data/maps/map_groups.json")
    rows: dict[str, list[dict[str, str]]] = defaultdict(list)
    for group in groups.get("group_order", []):
        for name in groups.get(group, []):
            source = load_json(source_map_path(root, name))
            version = str(source.get("game_version", "emerald"))
            rows[version].append({"name": name, "id": source.get("id"), "layout": source.get("layout")})
    products = []
    for version, identities in sorted(rows.items()):
        products.append({"source_version": version, "map_count": len(identities), "identity_sha256": digest(identities)})
    return {"products": products}


def rom_report(path: Path | None, root: Path) -> dict[str, Any]:
    if path is None:
        return {"measured": False, "reason": "no candidate release-size report supplied"}
    data = load_json(path)
    rom = data.get("rom")
    if not isinstance(rom, dict):
        raise AuditError(f"ROM report {relative(path, root)} has no rom object")
    end_text = rom.get("end_address")
    used_bytes = rom.get("used_bytes")
    if not isinstance(end_text, str) or not isinstance(used_bytes, int):
        raise AuditError(f"ROM report {relative(path, root)} lacks end_address or used_bytes")
    try:
        end = int(end_text, 0)
    except ValueError as error:
        raise AuditError(f"ROM report {relative(path, root)} has invalid end_address") from error
    if end - ROM_START != used_bytes:
        raise AuditError(f"ROM report {relative(path, root)} used_bytes disagrees with end_address")
    if end > WAYFARER_RELEASE_LIMIT:
        raise AuditError(f"ROM report {relative(path, root)} exceeds Wayfarer 512 KiB reserve")
    return {
        "measured": True, "path": relative(path, root), "used_bytes": used_bytes,
        "end_address": f"0x{end:08X}", "capacity_remaining_bytes": ROM_CAPACITY_END - end,
        "required_reserve_bytes": REQUIRED_RESERVE_BYTES,
        "headroom_above_required_reserve_bytes": WAYFARER_RELEASE_LIMIT - end,
    }


def validate_baseline(manifest: dict[str, Any], baseline: dict[str, Any], projection: dict[str, Any], root: Path) -> dict[str, str]:
    if baseline.get("schema_version") != 1:
        raise AuditError("content baseline must have schema_version 1")
    expected = baseline.get("manifest_baseline")
    if not isinstance(expected, dict):
        raise AuditError("content baseline must declare manifest_baseline")
    actual = manifest.get("baseline")
    if actual != expected:
        raise AuditError("manifest baseline differs from the accepted foundation baseline")
    calculated = {
        "projection_sha256": digest(projection),
        "wild_encounters_sha256": file_digest(root / "src/data/wayfarer_sevii_wild_encounters.json"),
        "event_island_baseline_sha256": file_digest(root / "tools/wayfarer_sevii_port/event_island_baseline.json"),
    }
    for key, value in calculated.items():
        if expected.get(key) != value:
            raise AuditError(f"accepted {key} drifted")
    return calculated


def build_report(root: Path, manifest_path: Path | None = None, *, baseline_path: Path | None = None,
                 candidate_rom_report: Path | None = None) -> dict[str, Any]:
    root = root.resolve()
    manifest_path = (manifest_path or root / "src/data/wayfarer_sevii_maps.json").resolve()
    baseline_path = (baseline_path or TOOL_DIR / "baseline.json").resolve()
    manifest = load_json(manifest_path)
    try:
        manifest = schema.validate_manifest(root, manifest)
        closure_report = closure.build_script_closure(root, manifest)
        contracts_report = contracts.validate_contracts(root, manifest)
    except (schema.SchemaError, closure.ClosureError, contracts.ContractError, ValueError) as error:
        raise AuditError(str(error)) from error
    projection = exploration_projection(root, manifest)
    baseline = load_json(baseline_path)
    hashes = validate_baseline(manifest, baseline, projection, root)
    return {
        "schema_version": 1,
        "product": "WAYFARER_SEVII_CONTENT",
        "manifest_path": relative(manifest_path, root),
        "baseline_path": relative(baseline_path, root),
        "schema": {
            "schema_version": manifest["schema_version"],
            "map_count": len(manifest["maps"]),
            "domains": {
                name: {"owner": row["owner"], "enabled": row["enabled"], "inventory_count": len(row["inventory"])}
                for name, row in sorted(manifest["content_domains"].items())
            },
        },
        "content": content_inventory_report(manifest),
        "exploration_baseline": {**projection, "sha256": hashes["projection_sha256"]},
        "wild_encounters": {"sha256": hashes["wild_encounters_sha256"]},
        "event_island": event_island_report(root),
        "standalone_projection": standalone_projection_report(root),
        "script_closure": closure_report,
        "contracts": contracts_report,
        "contract_closure": validate_contract_closure(root, manifest, closure_report, contracts_report),
        "rom": rom_report(candidate_rom_report.resolve() if candidate_rom_report else None, root),
        "invariants": {"passed": True},
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=TOOL_DIR.parents[2])
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--rom-report", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    try:
        report = build_report(args.root, args.manifest, baseline_path=args.baseline,
                              candidate_rom_report=args.rom_report)
    except AuditError as error:
        print(f"wayfarer-sevii-content audit: {error}", file=sys.stderr)
        return 1
    output = json.dumps(report, ensure_ascii=True, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(output, encoding="utf-8")
    else:
        print(output, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
