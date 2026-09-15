"""Schema-v2 validation for the single Wayfarer Sevii projection manifest.

The map adapter consumes the same manifest as the host generators.  This
module is deliberately dependency-free so every host-side audit can validate
the selection before it reasons about scripts, state, or generated output.
"""

from __future__ import annotations

import copy
import hashlib
import json
import re
from pathlib import Path
from typing import Iterable


EVENT_KINDS = ("object_events", "warp_events", "coord_events", "bg_events")
OVERLAY_EVENT_KINDS = ("object_events", "coord_events", "bg_events")
DOMAINS = ("exploration", "ordinary_trainers", "story", "trainer_tower")
DOMAIN_OWNERS = {
    "exploration": "exploration",
    "ordinary_trainers": "ordinary_trainer",
    "story": "story",
    "trainer_tower": "trainer_tower",
}
OWNER_DOMAINS = {owner: domain for domain, owner in DOMAIN_OWNERS.items()}
ALLOWED_OVERRIDES = {
    "object_events": frozenset(("script", "flag")),
    "coord_events": frozenset(("script", "var", "var_value")),
    "bg_events": frozenset(("script", "flag")),
}
SHA256 = re.compile(r"^[0-9a-f]{64}$")
CONTENT_ID = re.compile(r"^[a-z][a-z0-9_.-]*$")
STATE_CONTRACT = re.compile(r"^SEVII_[A-Z0-9_]+$")
WAYFARER_OVERRIDE = re.compile(r"^(?:FLAG|VAR)_WAYFARER_SEVII_[A-Z0-9_]+$")
APPROVED_EXPLORATION_HELPERS = {"EventScript_StrengthBoulder"}
EVENT_ROW_KEYS = frozenset((
    "index", "source", "wayfarer_script", "owner", "content_id", "reason", "overrides",
    "state_reads", "state_writes", "visibility", "trainer", "battle", "transaction",
    "reward_receipt", "dependencies", "replaces", "actor_role", "scaling_policy",
    "battle_type", "outcome_policy", "required_dependencies",
))


class SchemaError(ValueError):
    """The manifest is not a safe, source-provenanced overlay selection."""


def canonical_json(value: object) -> str:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True)


def canonical_sha256(value: object) -> str:
    return hashlib.sha256(canonical_json(value).encode("utf-8")).hexdigest()


def source_identity(record: dict) -> tuple[str, str, int]:
    """Return the stable source identity for an event or map-script record."""
    index_key = "source_index" if record.get("event_kind") == "map_scripts" else "index"
    return (record["source_map"], record["event_kind"], record[index_key])


def selected_records(manifest: dict, domains: Iterable[str] | None = None) -> list[dict]:
    """Return enabled projected rows in deterministic manifest order.

    Rows are plain dictionaries so generators can add their own audit fields.
    `validate_manifest` should be called first when input is not trusted.
    """
    enabled = {
        domain
        for domain, entry in manifest.get("content_domains", {}).items()
        if isinstance(entry, dict) and entry.get("enabled") is True
    }
    if domains is not None:
        enabled &= set(domains) | {"exploration"}
    output: list[dict] = []
    for map_record in manifest.get("maps", []):
        for event_kind in OVERLAY_EVENT_KINDS:
            for row in map_record.get("retained_events", {}).get(event_kind, []):
                domain = OWNER_DOMAINS.get(row.get("owner"))
                if domain in enabled:
                    selected = copy.deepcopy(row)
                    selected.update({
                        "source_map": map_record["source_map"],
                        "map_id": map_record["map_id"],
                        "event_kind": event_kind,
                        "domain": domain,
                    })
                    output.append(selected)
        for row in map_record.get("retained_map_scripts", []):
            domain = OWNER_DOMAINS.get(row.get("owner"))
            if domain in enabled:
                selected = copy.deepcopy(row)
                selected.update({
                    "source_map": map_record["source_map"],
                    "map_id": map_record["map_id"],
                    "event_kind": "map_scripts",
                    "index": row["source_index"],
                    "domain": domain,
                })
                output.append(selected)
    return output


def _fail(context: str, message: str) -> None:
    raise SchemaError(f"{context}: {message}")


def _require_dict(value: object, context: str) -> dict:
    if not isinstance(value, dict):
        _fail(context, "must be an object")
    return value


def _require_list(value: object, context: str) -> list:
    if not isinstance(value, list):
        _fail(context, "must be a list")
    return value


def _require_string(value: object, context: str) -> str:
    if not isinstance(value, str) or not value:
        _fail(context, "must be a non-empty string")
    return value


def _read_json(path: Path) -> object:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise SchemaError(f"cannot read {path}: {error}") from error


def _validate_baseline(root: Path, baseline: object) -> None:
    baseline = _require_dict(baseline, "baseline")
    for field in ("projection_sha256", "wild_encounters_sha256", "event_island_baseline_sha256"):
        value = _require_string(baseline.get(field), f"baseline.{field}")
        if SHA256.fullmatch(value) is None:
            _fail(f"baseline.{field}", "must be a lowercase SHA-256 digest")
    frozen_files = {
        "wild_encounters_sha256": root / "src/data/wayfarer_sevii_wild_encounters.json",
        "event_island_baseline_sha256": root / "tools/wayfarer_sevii_port/event_island_baseline.json",
    }
    for field, path in frozen_files.items():
        actual = canonical_sha256(_read_json(path))
        if baseline[field] != actual:
            _fail(field, f"does not match frozen normalized source {path}")


def _validate_contracts(contracts: object) -> None:
    contracts = _require_dict(contracts, "contracts")
    expected = {
        "schema_version", "trainer_ids", "state_namespace", "states", "transactions",
    }
    if set(contracts) != expected or contracts.get("schema_version") != 1:
        _fail("contracts", "must use the shared schema-v1 contract envelope")
    trainer_ids = _require_dict(contracts.get("trainer_ids"), "contracts.trainer_ids")
    if trainer_ids.get("base") != 1515 or trainer_ids.get("limit") != 2048 or not isinstance(trainer_ids.get("allocations"), list):
        _fail("contracts.trainer_ids", "must retain the fixed empty foundation allocation")
    state = _require_dict(contracts.get("state_namespace"), "contracts.state_namespace")
    expected_state = {"name": "wayfarer_sevii", "flag_base": 49152, "flag_capacity": 256,
                      "var_base": 53248, "var_capacity": 32,
                      "defeat_bitset": "wayfarer_sevii_trainer_defeat", "storage": "SaveBlock3"}
    if state != expected_state or not isinstance(contracts.get("states"), list) or not isinstance(contracts.get("transactions"), list):
        _fail("contracts", "must use the shared state namespace and declaration lists")
    allocations = _require_list(trainer_ids.get("allocations"), "contracts.trainer_ids.allocations")
    if any(not isinstance(row, dict) for row in allocations):
        _fail("contracts.trainer_ids.allocations", "must contain declaration objects")


def _source_event(root: Path, source_map: str, event_kind: str, index: int) -> dict:
    path = root / "data/maps" / source_map / "map.json"
    source_map_json = _require_dict(_read_json(path), f"source map {source_map}")
    events = _require_list(source_map_json.get(event_kind), f"source map {source_map}.{event_kind}")
    if index < 0 or index >= len(events):
        _fail(f"{source_map}.{event_kind}[{index}]", "source index is out of bounds")
    return _require_dict(events[index], f"{source_map}.{event_kind}[{index}]")


def _validate_state_names(row: dict, context: str) -> None:
    for field in ("state_reads", "state_writes"):
        if field not in row:
            continue
        for state in _require_list(row[field], f"{context}.{field}"):
            state = _require_string(state, f"{context}.{field}")
            if STATE_CONTRACT.fullmatch(state) is None:
                _fail(f"{context}.{field}", f"raw or non-Sevii state name {state!r}")


def _validate_event_row(root: Path, map_record: dict, event_kind: str, row: object,
                        inventories: dict[str, set[str]], source_owners: set[tuple],
                        local_owners: set[tuple], coordinate_owners: set[tuple]) -> None:
    source_map = map_record["source_map"]
    context = f"{source_map}.{event_kind}"
    row = _require_dict(row, context)
    unknown = set(row) - EVENT_ROW_KEYS
    if unknown:
        _fail(context, f"contains unknown event fields {sorted(unknown)!r}")
    index = row.get("index")
    if not isinstance(index, int) or isinstance(index, bool) or index < 0:
        _fail(context, "index must be a non-negative integer")
    source = _require_dict(row.get("source"), f"{context}[{index}].source")
    actual_source = _source_event(root, source_map, event_kind, index)
    if canonical_json(source) != canonical_json(actual_source):
        _fail(f"{context}[{index}]", "does not exactly match its reviewed source event")
    owner = _require_string(row.get("owner"), f"{context}[{index}].owner")
    domain = OWNER_DOMAINS.get(owner)
    if domain is None:
        _fail(f"{context}[{index}].owner", "is not a typed content owner")
    content_id = _require_string(row.get("content_id"), f"{context}[{index}].content_id")
    if CONTENT_ID.fullmatch(content_id) is None:
        _fail(f"{context}[{index}].content_id", "must be a stable lowercase content ID")
    if content_id not in inventories[domain]:
        _fail(f"{context}[{index}].content_id", f"is missing from {domain} inventory")
    _require_string(row.get("reason"), f"{context}[{index}].reason")
    identity = (source_map, event_kind, index)
    if identity in source_owners:
        _fail(context, f"duplicates ownership of {source_map}.{event_kind}[{index}]")
    source_owners.add(identity)
    if event_kind == "object_events":
        local_id = source.get("local_id")
        if local_id is not None:
            key = (source_map, local_id)
            if key in local_owners:
                _fail(context, f"duplicates local_id ownership {local_id!r}")
            local_owners.add(key)
    else:
        key = (source_map, event_kind, source.get("x"), source.get("y"), source.get("elevation"))
        if key in coordinate_owners:
            _fail(context, "duplicates coordinate/background ownership")
        coordinate_owners.add(key)
    wayfarer_script = row.get("wayfarer_script")
    source_script = source.get("script")
    if source_script not in (None, "", "0", "0x0", "NULL"):
        _require_string(wayfarer_script, f"{context}[{index}].wayfarer_script")
    elif wayfarer_script is not None:
        _require_string(wayfarer_script, f"{context}[{index}].wayfarer_script")
    overrides = row.get("overrides", {})
    overrides = _require_dict(overrides, f"{context}[{index}].overrides")
    unexpected = set(overrides) - ALLOWED_OVERRIDES[event_kind]
    if unexpected:
        _fail(f"{context}[{index}].overrides", f"contains forbidden fields {sorted(unexpected)!r}")
    if "script" in overrides:
        if wayfarer_script is not None and overrides["script"] != wayfarer_script:
            _fail(f"{context}[{index}].overrides.script", "must match wayfarer_script")
        _require_string(overrides["script"], f"{context}[{index}].overrides.script")
    for field in ("flag", "var"):
        if field in overrides:
            value = _require_string(overrides[field], f"{context}[{index}].overrides.{field}")
            always_visible_object = field == "flag" and event_kind == "object_events" and value == "0"
            if not always_visible_object and WAYFARER_OVERRIDE.fullmatch(value) is None:
                _fail(f"{context}[{index}].overrides.{field}", "must use the Wayfarer Sevii namespace")
    if "flag" in overrides and event_kind == "bg_events" and source.get("type") != "hidden_item":
        _fail(f"{context}[{index}].overrides.flag", "is only valid for a hidden item")
    if wayfarer_script is not None and not wayfarer_script.startswith("WayfarerSevii_"):
        if not (owner == "exploration" and wayfarer_script in APPROVED_EXPLORATION_HELPERS):
            _fail(f"{context}[{index}].wayfarer_script", "must be a Wayfarer-owned script")
    if owner != "exploration":
        persistent_field = "var" if event_kind == "coord_events" else "flag"
        effective = overrides.get(persistent_field, source.get(persistent_field, "0"))
        if effective not in (None, "", "0", 0) and (
            not isinstance(effective, str) or WAYFARER_OVERRIDE.fullmatch(effective) is None
        ):
            _fail(f"{context}[{index}].{persistent_field}", "retains raw FRLG persistent state")
    _validate_state_names(row, f"{context}[{index}]")


def _validate_script_modules(modules: object) -> dict[str, dict]:
    modules = _require_dict(modules, "script_modules")
    for module_id, module in modules.items():
        _require_string(module_id, "script_modules key")
        module = _require_dict(module, f"script_modules.{module_id}")
        owner = _require_string(module.get("owner"), f"script_modules.{module_id}.owner")
        if owner not in OWNER_DOMAINS:
            _fail(f"script_modules.{module_id}.owner", "is not a typed content owner")
        include = _require_string(module.get("include"), f"script_modules.{module_id}.include")
        if not include.startswith("data/scripts/wayfarer_sevii/") or ".." in Path(include).parts:
            _fail(f"script_modules.{module_id}.include", "must stay under Wayfarer Sevii scripts")
        owner_directory = {
            "ordinary_trainer": "trainers/",
            "story": "story/",
            "trainer_tower": "trainer_tower/",
        }.get(owner, "")
        relative_include = include.removeprefix("data/scripts/wayfarer_sevii/")
        if owner_directory and not relative_include.startswith(owner_directory):
            _fail(f"script_modules.{module_id}.include", f"must stay under {owner_directory} for {owner}")
        if owner == "exploration" and "/" in relative_include:
            _fail(f"script_modules.{module_id}.include", "exploration modules must use the shared root")
        _require_list(module.get("exports"), f"script_modules.{module_id}.exports")
        _require_list(module.get("allowed_externals"), f"script_modules.{module_id}.allowed_externals")
    return modules


def _validate_map_script_row(map_record: dict, row: object, inventories: dict[str, set[str]],
                             source_owners: set[tuple], handler_owners: set[tuple], modules: dict[str, dict]) -> None:
    source_map = map_record["source_map"]
    context = f"{source_map}.retained_map_scripts"
    row = _require_dict(row, context)
    owner = _require_string(row.get("owner"), f"{context}.owner")
    domain = OWNER_DOMAINS.get(owner)
    if domain is None:
        _fail(f"{context}.owner", "is not a typed content owner")
    content_id = _require_string(row.get("content_id"), f"{context}.content_id")
    if content_id not in inventories[domain]:
        _fail(f"{context}.content_id", f"is missing from {domain} inventory")
    module_id = _require_string(row.get("module"), f"{context}.module")
    if module_id not in modules or modules[module_id]["owner"] != owner:
        _fail(f"{context}.module", "must name a module owned by the same domain")
    handler_type = _require_string(row.get("handler_type"), f"{context}.handler_type")
    source_index = row.get("source_index")
    if not isinstance(source_index, int) or isinstance(source_index, bool) or source_index < 0:
        _fail(f"{context}.source_index", "must be a non-negative integer")
    source = _require_dict(row.get("source"), f"{context}.source")
    _require_string(row.get("wayfarer_script"), f"{context}.wayfarer_script")
    _require_string(row.get("reason"), f"{context}.reason")
    identity = (source_map, "map_scripts", source_index)
    if identity in source_owners:
        _fail(context, "duplicates map-script source ownership")
    source_owners.add(identity)
    handler_identity = (source_map, handler_type)
    if handler_identity in handler_owners:
        _fail(context, f"duplicates handler type {handler_type}")
    handler_owners.add(handler_identity)
    _validate_state_names(row, context)


def _validate_exclusions(root: Path, exclusions: object, source_owners: set[tuple]) -> None:
    seen: set[str] = set()
    for exclusion in _require_list(exclusions, "exclusions"):
        exclusion = _require_dict(exclusion, "exclusions entry")
        exclusion_id = _require_string(exclusion.get("content_id"), "exclusions.content_id")
        if exclusion_id in seen:
            _fail("exclusions", f"duplicates content ID {exclusion_id}")
        seen.add(exclusion_id)
        _require_string(exclusion.get("reason"), f"exclusions.{exclusion_id}.reason")
        identities = _require_list(exclusion.get("source_identities"), f"exclusions.{exclusion_id}.source_identities")
        if not identities:
            _fail(f"exclusions.{exclusion_id}", "must list exact excluded source identities")
        for identity in identities:
            identity = _require_dict(identity, f"exclusions.{exclusion_id}.source_identities")
            source_map = _require_string(identity.get("source_map"), "exclusion source_map")
            kind = _require_string(identity.get("event_kind"), "exclusion event_kind")
            index = identity.get("index")
            if kind not in EVENT_KINDS + ("map", "catalog", "script_label") or not isinstance(index, int) or index < 0:
                _fail(f"exclusions.{exclusion_id}", "has invalid exact source identity")
            source = _require_dict(identity.get("source"), f"exclusions.{exclusion_id}.source")
            if kind == "script_label":
                relative = _require_string(source.get("path"), f"exclusions.{exclusion_id}.source.path")
                label = _require_string(source.get("label"), f"exclusions.{exclusion_id}.source.label")
                candidate = Path(relative)
                path = (root / candidate).resolve()
                try:
                    path.relative_to(root)
                except ValueError:
                    _fail(f"exclusions.{exclusion_id}", "excluded script source must stay under the game root")
                if candidate.is_absolute() or ".." in candidate.parts or not path.is_file():
                    _fail(f"exclusions.{exclusion_id}", "excluded script source is unavailable")
                text = path.read_text(encoding="utf-8").replace("\r\n", "\n")
                match = re.search(rf"(?m)^{re.escape(label)}::?\s*$", text)
                if match is None:
                    _fail(f"exclusions.{exclusion_id}", "excluded script label drifted")
                following = text[match.end():]
                next_label = re.search(r"(?m)^\S[^\n:]*::?\s*$", following)
                body = text[match.start():] if next_label is None else text[match.start():match.end() + next_label.start()]
                if source.get("file_sha256") != hashlib.sha256(path.read_bytes()).hexdigest() or source.get("body_sha256") != hashlib.sha256(body.encode()).hexdigest():
                    _fail(f"exclusions.{exclusion_id}", "excluded script body drifted")
                continue
            if kind in EVENT_KINDS:
                if canonical_json(source) != canonical_json(_source_event(root, source_map, kind, index)):
                    _fail(f"exclusions.{exclusion_id}", "excluded source event drifted")
            else:
                expected_path = (root / (f"data/maps/{source_map}/map.json" if kind == "map" else "data/maps/map_groups.json"))
                if source.get("path") != expected_path.relative_to(root).as_posix() or source.get("sha256") != canonical_sha256(_read_json(expected_path)):
                    _fail(f"exclusions.{exclusion_id}", "excluded source baseline drifted")
            if (source_map, kind, index) in source_owners:
                _fail(f"exclusions.{exclusion_id}", "also appears in selected content")


def validate_manifest(root: Path, manifest: dict) -> dict:
    """Validate and return a deep-copied schema-v2 manifest.

    Validation reads source map JSON for every retained event even when its
    content domain is disabled.  That keeps disabled future content reviewed
    and makes source drift fail deterministically before enablement.
    """
    root = Path(root).resolve()
    manifest = copy.deepcopy(_require_dict(manifest, "manifest"))
    if manifest.get("schema_version") != 2:
        _fail("schema_version", "must be 2")
    if not isinstance(manifest.get("release_link_enabled"), bool):
        _fail("release_link_enabled", "must be a boolean")
    _validate_baseline(root, manifest.get("baseline"))
    _validate_contracts(manifest.get("contracts"))
    domains = _require_dict(manifest.get("content_domains"), "content_domains")
    if set(domains) != set(DOMAINS):
        _fail("content_domains", f"must contain exactly {list(DOMAINS)!r}")
    inventories: dict[str, set[str]] = {}
    for domain in DOMAINS:
        entry = _require_dict(domains[domain], f"content_domains.{domain}")
        if entry.get("owner") != DOMAIN_OWNERS[domain]:
            _fail(f"content_domains.{domain}.owner", "does not match the typed domain owner")
        if not isinstance(entry.get("enabled"), bool):
            _fail(f"content_domains.{domain}.enabled", "must be a boolean")
        inventory = _require_list(entry.get("inventory"), f"content_domains.{domain}.inventory")
        if len(inventory) != len(set(inventory)):
            _fail(f"content_domains.{domain}.inventory", "contains duplicate content IDs")
        for content_id in inventory:
            if CONTENT_ID.fullmatch(_require_string(content_id, f"content_domains.{domain}.inventory")) is None:
                _fail(f"content_domains.{domain}.inventory", "contains an invalid content ID")
        inventories[domain] = set(inventory)
    if domains["exploration"]["enabled"] is not True:
        _fail("content_domains.exploration.enabled", "must remain enabled for the exploration baseline")
    modules = _validate_script_modules(manifest.get("script_modules", {}))
    maps = _require_list(manifest.get("maps"), "maps")
    map_names: set[str] = set()
    map_ids: set[str] = set()
    source_owners: set[tuple] = set()
    local_owners: set[tuple] = set()
    coordinate_owners: set[tuple] = set()
    handler_owners: set[tuple] = set()
    selected_content: set[str] = set()
    for map_record in maps:
        map_record = _require_dict(map_record, "maps entry")
        for field in ("source_map", "map_id", "layout", "category"):
            _require_string(map_record.get(field), f"maps.{field}")
        source_map = map_record["source_map"]
        if source_map in map_names or map_record["map_id"] in map_ids:
            _fail("maps", f"duplicates map {source_map}")
        map_names.add(source_map)
        map_ids.add(map_record["map_id"])
        retained = _require_dict(map_record.get("retained_events"), f"{source_map}.retained_events")
        if set(retained) != set(EVENT_KINDS):
            _fail(f"{source_map}.retained_events", "must contain every event-kind list")
        for event_kind in EVENT_KINDS:
            rows = _require_list(retained[event_kind], f"{source_map}.retained_events.{event_kind}")
            if event_kind == "warp_events" and rows:
                _fail(f"{source_map}.retained_events.warp_events", "must remain geometry-only and empty")
            for row in rows:
                _validate_event_row(root, map_record, event_kind, row, inventories, source_owners,
                                    local_owners, coordinate_owners)
                selected_content.add(_require_dict(row, "event row")["content_id"])
        for row in _require_list(map_record.get("retained_map_scripts"), f"{source_map}.retained_map_scripts"):
            _validate_map_script_row(map_record, row, inventories, source_owners, handler_owners, modules)
            selected_content.add(_require_dict(row, "map-script row")["content_id"])
        _require_list(map_record.get("encounter_methods"), f"{source_map}.encounter_methods")
    expected_content = set().union(*inventories.values())
    if selected_content != expected_content:
        missing = sorted(expected_content - selected_content)
        extra = sorted(selected_content - expected_content)
        _fail("content_domains inventories", f"must resolve exactly once (missing={missing}, extra={extra})")
    _validate_exclusions(root, manifest.get("exclusions"), source_owners)
    return manifest
