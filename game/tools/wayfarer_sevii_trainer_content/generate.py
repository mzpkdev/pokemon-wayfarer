#!/usr/bin/env python3
"""Project the frozen ordinary Sevii Trainer inventory into the overlay.

The allocation owner publishes the source keys and hashes.  This generator is
deliberately separate from the roster generator: it owns only map projection,
owned script wrappers, copied dialogue, and the host contract declarations.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path


TOOL = Path(__file__).resolve().parent
GAME = TOOL.parents[1]
REPO = GAME.parent
MANIFEST = GAME / "src/data/wayfarer_sevii_maps.json"
INVENTORY = REPO / "docs/sevii-trainer-implementation/allocation-inventory.json"
SOURCE_SCRIPTS = GAME / "data/scripts/trainers_frlg.inc"
SOURCE_TEXT = GAME / "data/text/trainers_frlg.inc"
OUTPUT = GAME / "data/scripts/wayfarer_sevii/trainers/ordinary.inc"

LABEL = re.compile(r"(?m)^([A-Za-z_][A-Za-z0-9_]*)::\s*$")
TOKEN = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")


class ProjectionError(ValueError):
    pass


def labels(path: Path) -> dict[str, str]:
    source = path.read_text(encoding="utf-8").replace("\r\n", "\n")
    found = list(LABEL.finditer(source))
    return {match.group(1): source[match.start(): found[index + 1].start() if index + 1 < len(found) else len(source)]
            for index, match in enumerate(found)}


def load(path: Path) -> dict:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ProjectionError(f"{path} must contain an object")
    return value


def content_id(source_map: str, index: int) -> str:
    return "ordinary." + source_map.removesuffix("_Frlg").replace("_", "-").lower() + f".object.{index}"


def state_id(source_trainer: str) -> str:
    return "SEVII_ORDINARY_" + source_trainer.removeprefix("TRAINER_") + "_DEFEATED"


def owner_rows() -> tuple[list[dict], list[dict]]:
    inventory = load(INVENTORY)["allocation"]["allocations"]
    bases = [row for row in inventory if row["owner"] == "ordinary_trainer" and row["kind"] == "base"]
    ordinary = [row for row in inventory if row["owner"] == "ordinary_trainer"]
    if len(bases) != 81 or len(ordinary) != 118:
        raise ProjectionError("frozen ordinary allocation inventory drifted")
    return bases, ordinary


def projection() -> tuple[list[dict], dict[str, dict], dict[str, dict]]:
    bases, ordinary = owner_rows()
    source_wrappers = labels(SOURCE_SCRIPTS)
    source_text = labels(SOURCE_TEXT)
    events: list[dict] = []
    families: dict[str, dict] = {}
    source_maps: dict[str, dict] = {}
    for base in bases:
        family_id = content_id(*base["objects"][0].rsplit(":", 1)) if False else None
        members: list[str] = []
        for identity in base["objects"]:
            source_map, raw_index = identity.rsplit(":", 1)
            index = int(raw_index)
            source_maps.setdefault(source_map, load(GAME / "data/maps" / source_map / "map.json"))
            source = source_maps[source_map]["object_events"][index]
            wrapper_pool = source_wrappers | source_text | labels(GAME / "data/maps" / source_map / "scripts.inc")
            wrapper = wrapper_pool.get(source["script"])
            if wrapper is None:
                raise ProjectionError(f"missing source Trainer wrapper {source['script']}")
            expected = re.search(r"(?m)^\s*trainerbattle_(single|double)\s+(TRAINER_[A-Z0-9_]+)", wrapper)
            if expected is None or expected.group(2) != base["source_trainer"] or expected.group(1) != base["battle_type"]:
                raise ProjectionError(f"source battle drift for {source_map}:{index}")
            commands = [line.split("@", 1)[0].strip() for line in wrapper.splitlines()[1:]
                        if line.split("@", 1)[0].strip() and not line.lstrip().startswith(".")]
            if not commands or not commands[0].startswith(f"trainerbattle_{base['battle_type']}"):
                raise ProjectionError(f"{source_map}:{index} does not begin with its source sight/talk battle command")
            identifier = content_id(source_map, index)
            members.append(identifier)
            events.append({"content_id": identifier, "source_map": source_map, "index": index,
                           "source": source, "source_script": source["script"], "base": base,
                           "wrapper": wrapper, "wrapper_pool": wrapper_pool})
        canonical = members[0]
        families[canonical] = {"base": base, "members": members,
                               "pair_id": f"ordinary.pair.{base['source_trainer'].removeprefix('TRAINER_').lower()}" if len(members) == 2 else None}
    if len(events) != 87 or sum(len(value["members"]) == 2 for value in families.values()) != 6:
        raise ProjectionError("ordinary source object/pair inventory drifted")
    by_source = {row["source_trainer"]: row for row in ordinary}
    return events, families, by_source


def manifest_update(manifest: dict) -> dict:
    events, families, by_source = projection()
    canonical_for = {member: canonical for canonical, family in families.items() for member in family["members"]}
    event_by_map: dict[str, list[dict]] = {}
    for event in events:
        base, canonical = event["base"], canonical_for[event["content_id"]]
        family = families[canonical]
        cid = event["content_id"]
        event_by_map.setdefault(event["source_map"], []).append({
            "index": event["index"], "source": event["source"],
            "wayfarer_script": wayfarer_label(event["source_script"]), "owner": "ordinary_trainer",
            "content_id": cid, "trainer_content_id": canonical, "pair_id": family["pair_id"],
            "rematch_family": base["source_trainer"] if "ShouldTryRematchBattle" in event["wrapper"] else None,
            "reason": "Exact FRLG ordinary Trainer projection with Wayfarer-owned battle wrapper and dialogue.",
            "overrides": {"script": wayfarer_label(event["source_script"])},
            "state_reads": [], "state_writes": [state_id(base["source_trainer"])],
            "actor_role": "ordinary_trainer", "scaling_policy": "ordinary",
            "battle_type": base["battle_type"], "outcome_policy": "defeat_and_blackout",
            "trainer": base["id"],
        })
    ordinary_records = [row for rows in event_by_map.values() for row in rows]
    manifest["content_domains"]["ordinary_trainers"] = {
        "owner": "ordinary_trainer", "enabled": True,
        "inventory": [row["content_id"] for row in sorted(ordinary_records, key=lambda row: (row["content_id"]))],
    }
    manifest["script_modules"]["ordinary_trainers"] = {
        "owner": "ordinary_trainer", "include": "data/scripts/wayfarer_sevii/trainers/ordinary.inc",
        "exports": sorted(script_exports(events)),
        "allowed_commands": ["end", "goto_if_eq", "msgbox", "specialvar", "trainerbattle_double",
                             "trainerbattle_rematch", "trainerbattle_rematch_double", "trainerbattle_single"],
        "allowed_externals": [external_special("ShouldTryRematchBattle")],
    }
    for record in manifest["maps"]:
        rows = event_by_map.get(record["source_map"], [])
        retained = record["retained_events"]["object_events"]
        retained[:] = [row for row in retained if row.get("owner") != "ordinary_trainer"] + sorted(rows, key=lambda row: row["index"])
    allocations, states = [], []
    for canonical, family in families.items():
        base = family["base"]
        defeat = state_id(base["source_trainer"])
        members = family["members"]
        states.append({"id": defeat, "owner": "ordinary_trainer", "storage": "trainer_defeat", "slot": base["slot"],
                       "initial": 0, "readers": [], "writers": members,
                       "transitions": [{"from": 0, "to": 1, "caller": canonical}]})
    for source in sorted(by_source.values(), key=lambda row: row["slot"]):
        base = next(family for family in families.values() if family["base"]["source_trainer"] == source["defeat_base"])["base"]
        canonical = next(key for key, family in families.items() if family["base"] is base)
        allocation_content = canonical if source["kind"] == "base" else "ordinary.roster." + source["source_trainer"].removeprefix("TRAINER_").lower()
        allocations.append({"content_id": allocation_content, "id": source["id"], "slot": source["slot"], "owner": "ordinary_trainer",
                            "source_trainer": source["source_trainer"], "source_hash": source["source_hash"],
                            "classification": "ordinary", "battle_policy": "ordinary", "battle_type": source["battle_type"],
                            "outcome_policy": "defeat_and_blackout", "defeat_state": state_id(base["source_trainer"]),
                            "defeat_base": canonical})
    manifest["contracts"]["trainer_ids"]["allocations"] = allocations
    manifest["contracts"]["states"] = sorted(states, key=lambda row: row["slot"])
    return manifest


def external_special(label: str) -> dict:
    specials, impl = GAME / "data/specials.inc", GAME / "src/battle_setup.c"
    return {"label": label, "kind": "special", "path": "data/specials.inc",
            "sha256": hashlib.sha256(specials.read_bytes()).hexdigest(),
            "implementation_path": "src/battle_setup.c", "implementation_sha256": hashlib.sha256(impl.read_bytes()).hexdigest()}


def wayfarer_label(label: str) -> str:
    return "WayfarerSevii_Trainer_" + label


def script_exports(events: list[dict]) -> set[str]:
    output: set[str] = set()
    for event in events:
        pending, visited = [event["source_script"]], set()
        while pending:
            label = pending.pop()
            if label in visited:
                continue
            visited.add(label)
            scripts = event["wrapper_pool"]
            body = scripts[label]
            output.add(wayfarer_label(label))
            for target in re.findall(r"\b([A-Za-z_][A-Za-z0-9_]*)\b", body):
                if target in scripts and target.startswith(event["source_script"]):
                    pending.append(target)
            for text in re.findall(r"\b([A-Za-z_][A-Za-z0-9_]*_Text_[A-Za-z0-9_]+)\b", body):
                output.add(wayfarer_label(text))
    return output


def render_script() -> str:
    events, _, _ = projection()
    texts = {}
    selected: set[str] = set()
    for event in events:
        source_map = event["source_map"]
        texts.setdefault(source_map, labels(GAME / "data/maps" / source_map / "scripts.inc"))
    chunks: dict[str, str] = {}
    rename: dict[str, str] = {}
    for event in events:
        pending, visited = [event["source_script"]], set()
        while pending:
            label = pending.pop()
            if label in visited:
                continue
            visited.add(label)
            scripts = event["wrapper_pool"]
            body = scripts[label]
            chunks[label] = body
            rename[label] = wayfarer_label(label)
            for target in TOKEN.findall(body):
                if target in scripts and target.startswith(event["source_script"]):
                    pending.append(target)
                if "_Text_" in target:
                    text = scripts.get(target) or texts[event["source_map"]].get(target)
                    if text is None:
                        raise ProjectionError(f"missing source dialogue {target}")
                    chunks[target] = text
                    rename[target] = wayfarer_label(target)
    bases, ordinary = owner_rows()
    id_by_source = {row["source_trainer"]: row["id"] for row in ordinary}
    for event in events:
        id_by_source[event["base"]["source_trainer"]] = event["base"]["id"]
    def replace(body: str) -> str:
        def one(match: re.Match[str]) -> str:
            token = match.group(0)
            return rename.get(token, id_by_source.get(token, token))
        return TOKEN.sub(one, body)
    lines = ["@ Generated by tools/wayfarer_sevii_trainer_content/generate.py.",
             "@ Exact ordinary FRLG battle text; source Trainer IDs are provenance only.", ""]
    for label in sorted(chunks):
        lines.append(replace(chunks[label]).rstrip())
        lines.append("")
    return "\n".join(lines).rstrip() + "\n"


def write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.is_file() or path.read_text(encoding="utf-8") != content:
        path.write_text(content, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    manifest, scripts = manifest_update(load(MANIFEST)), render_script()
    rendered_manifest = json.dumps(manifest, indent=2) + "\n"
    outputs = {MANIFEST: rendered_manifest, OUTPUT: scripts}
    stale = [path for path, content in outputs.items() if not path.is_file() or path.read_text(encoding="utf-8") != content]
    if args.check:
        if stale:
            parser.error("stale ordinary Trainer projection: " + ", ".join(str(path) for path in stale))
        return 0
    for path, content in outputs.items():
        write(path, content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
