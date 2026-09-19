#!/usr/bin/env python3
"""Freeze the shared Wayfarer Sevii Trainer allocation and source inventory."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import re
import subprocess
import tempfile
from collections import Counter, OrderedDict
from pathlib import Path


TOOL_DIR = Path(__file__).resolve().parent
GAME_ROOT = TOOL_DIR.parents[1]
REPO_ROOT = GAME_ROOT.parent
MANIFEST = GAME_ROOT / "src/data/wayfarer_sevii_maps.json"
PARTIES = GAME_ROOT / "src/data/trainers_frlg.party"
COMMON_TRAINERS = GAME_ROOT / "data/scripts/trainers_frlg.inc"
ALLOCATION_BASE = 1515
RUNTIME_CONSTANTS = GAME_ROOT / "include/constants/wayfarer_sevii_trainers.h"
RUNTIME_ROSTER = GAME_ROOT / "src/data/trainers_wayfarer_sevii.h"
RUNTIME_DEFEAT_ROUTER = GAME_ROOT / "include/wayfarer_sevii_trainer_defeats.h"

STORY_SOURCE_TRAINERS = (
    "TRAINER_BIKER_GOON",
    "TRAINER_BIKER_GOON_2",
    "TRAINER_BIKER_GOON_3",
    "TRAINER_CUE_BALL_PAXTON",
    "TRAINER_TEAM_ROCKET_GRUNT_43",
    "TRAINER_TEAM_ROCKET_GRUNT_44",
    "TRAINER_TEAM_ROCKET_GRUNT_45",
    "TRAINER_TEAM_ROCKET_GRUNT_46",
    "TRAINER_TEAM_ROCKET_GRUNT_49",
    "TRAINER_TEAM_ROCKET_GRUNT_50",
    "TRAINER_TEAM_ROCKET_GRUNT_51",
    "TRAINER_TEAM_ROCKET_GRUNT_42",
    "TRAINER_TEAM_ROCKET_GRUNT_47",
    "TRAINER_TEAM_ROCKET_GRUNT_48",
    "TRAINER_TEAM_ROCKET_ADMIN",
    "TRAINER_TEAM_ROCKET_ADMIN_2",
    "TRAINER_SCIENTIST_GIDEON",
    "TRAINER_LADY_SELPHY",
)

STORY_NORMAL_OBJECTS = frozenset(
    [("FiveIsland_RocketWarehouse_Frlg", index) for index in range(6)]
    + [("FiveIsland_Meadow_Frlg", index) for index in range(3)]
    + [("SixIsland_OutcastIsland_Frlg", 0)]
)


class InventoryError(ValueError):
    pass


def load_json(path: Path) -> dict:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise InventoryError(f"{path} must contain an object")
    return value


def label_bodies(path: Path) -> dict[str, str]:
    source = path.read_text(encoding="utf-8").replace("\r\n", "\n")
    labels = list(re.finditer(r"(?m)^([A-Za-z0-9_]+)::\s*$", source))
    return {
        match.group(1): source[match.start() : labels[index + 1].start() if index + 1 < len(labels) else len(source)]
        for index, match in enumerate(labels)
    }


def party_blocks() -> dict[str, str]:
    source = PARTIES.read_text(encoding="utf-8").replace("\r\n", "\n")
    markers = list(re.finditer(r"(?m)^=== (TRAINER_[A-Z0-9_]+) ===\s*$", source))
    result = {}
    for index, match in enumerate(markers):
        end = markers[index + 1].start() if index + 1 < len(markers) else len(source)
        block = "\n".join(line.rstrip() for line in source[match.start() : end].splitlines()).strip() + "\n"
        result[match.group(1)] = block
    return result


def source_hash(block: str) -> str:
    return hashlib.sha256(block.encode("utf-8")).hexdigest()


def ordinary_objects(manifest: dict) -> list[dict]:
    common = label_bodies(COMMON_TRAINERS)
    map_scripts: dict[str, dict[str, str]] = {}
    rows = []
    for map_record in manifest["maps"]:
        source_map = map_record["source_map"]
        source = load_json(GAME_ROOT / "data/maps" / source_map / "map.json")
        map_scripts[source_map] = label_bodies(GAME_ROOT / "data/maps" / source_map / "scripts.inc")
        for index, event in enumerate(source.get("object_events", [])):
            if event.get("trainer_type") != "TRAINER_TYPE_NORMAL" or (source_map, index) in STORY_NORMAL_OBJECTS:
                continue
            script = event["script"]
            body = common.get(script) or map_scripts[source_map].get(script)
            if body is None:
                raise InventoryError(f"cannot resolve source Trainer wrapper {script}")
            battle = re.search(r"(?m)^\s*trainerbattle_(single|double)\s+(TRAINER_[A-Z0-9_]+)", body)
            if battle is None:
                raise InventoryError(f"{script} has no typed source Trainer battle")
            rows.append(
                {
                    "source_map": source_map,
                    "source_index": index,
                    "local_id": index + 1,
                    "source_script": script,
                    "source_trainer": battle.group(2),
                    "battle_type": battle.group(1),
                    "rematch_capable": "ShouldTryRematchBattle" in body,
                    "talk_battle": int(event.get("trainer_sight_or_berry_tree_id", 0)) == 0,
                    "source_event": event,
                }
            )
    return rows


def allocation_inventory(objects: list[dict], blocks: dict[str, str]) -> list[dict]:
    bases: OrderedDict[str, dict] = OrderedDict()
    for row in objects:
        base = row["source_trainer"]
        family = bases.setdefault(
            base,
            {
                "battle_type": row["battle_type"],
                "rematch_capable": row["rematch_capable"],
                "objects": [],
            },
        )
        if family["battle_type"] != row["battle_type"] or family["rematch_capable"] != row["rematch_capable"]:
            raise InventoryError(f"inconsistent paired identity {base}")
        family["objects"].append(f"{row['source_map']}:{row['source_index']}")

    allocations = []
    for base, family in bases.items():
        sources = [base]
        if family["rematch_capable"]:
            sources.extend(source for stage in range(2, 7) if (source := f"{base}_{stage}") in blocks)
        for stage, source in enumerate(sources):
            allocations.append(
                {
                    "slot": len(allocations),
                    "numeric_id": ALLOCATION_BASE + len(allocations),
                    "id": f"TRAINER_WAYFARER_SEVII_{source.removeprefix('TRAINER_')}",
                    "source_trainer": source,
                    "source_hash": source_hash(blocks[source]),
                    "owner": "ordinary_trainer",
                    "kind": "base" if stage == 0 else "rematch",
                    "defeat_base": base,
                    "battle_type": family["battle_type"],
                    "objects": family["objects"] if stage == 0 else [],
                }
            )

    for source in STORY_SOURCE_TRAINERS:
        if source not in blocks:
            raise InventoryError(f"planned story Trainer source is absent: {source}")
        allocations.append(
            {
                "slot": len(allocations),
                "numeric_id": ALLOCATION_BASE + len(allocations),
                "id": f"TRAINER_WAYFARER_SEVII_{source.removeprefix('TRAINER_')}",
                "source_trainer": source,
                "source_hash": source_hash(blocks[source]),
                "owner": "story",
                "kind": "planned",
                "defeat_base": source,
                "battle_type": "single",
                "objects": [],
            }
        )
    return allocations


def build_report() -> dict:
    objects = ordinary_objects(load_json(MANIFEST))
    allocations = allocation_inventory(objects, party_blocks())
    base_counts = Counter(row["source_trainer"] for row in objects)
    report = {
        "schema_version": 1,
        "allocation": {
            "base": ALLOCATION_BASE,
            "limit": 2048,
            "count": len(allocations),
            "next_id": ALLOCATION_BASE + len(allocations),
            "allocations": allocations,
        },
        "ordinary": {
            "objects": len(objects),
            "base_identities": len(base_counts),
            "single_objects": sum(row["battle_type"] == "single" for row in objects),
            "pair_identities": sum(count == 2 for count in base_counts.values()),
            "sight_objects": sum(not row["talk_battle"] for row in objects),
            "talk_objects": sum(row["talk_battle"] for row in objects),
            "rematch_objects": sum(row["rematch_capable"] for row in objects),
            "single_stage_objects": sum(not row["rematch_capable"] for row in objects),
            "selected_party_records": sum(row["owner"] == "ordinary_trainer" for row in allocations),
            "objects_inventory": objects,
        },
        "planned_story_party_records": len(STORY_SOURCE_TRAINERS),
    }
    expected = (87, 81, 75, 6, 86, 1, 70, 17)
    actual = tuple(report["ordinary"][key] for key in (
        "objects", "base_identities", "single_objects", "pair_identities", "sight_objects",
        "talk_objects", "rematch_objects", "single_stage_objects",
    ))
    if actual != expected:
        raise InventoryError(f"ordinary source inventory drift: expected {expected}, found {actual}")
    if len(allocations) != 136 or allocations[-1]["numeric_id"] != 1650:
        raise InventoryError("shared dense allocation drift")
    return report


def render_coordination(report: dict) -> str:
    lines = [
        "# Sevii Trainer implementation coordination",
        "",
        "Status: allocation and runtime interface freeze published by commit `7fa44c8b75`.",
        "",
        "## Shared Trainer allocation",
        "",
        "The collision-audited allocation starts at 1515 and is dense through 1650 inclusive. `TRAINERS_COUNT_WAYFARER` becomes 1651 when the shared roster lands. Slots 0–117 are every distinct ordinary base/rematch party source; slots 118–135 reserve every planned story opponent. Trainer Tower opponents are facility-local and never enter this range. Source FRLG numeric IDs are provenance only and must never reach runtime.",
        "",
        "The checked-in machine-readable `allocation-inventory.json` is the single authoritative per-key inventory. It records every slot, runtime ID, generated symbol, source key, owner, kind, defeat base, object reference, and normalized party hash; this coordination note deliberately does not duplicate its 136 rows.",
    ]
    lines.extend(
        [
            "",
            "## Saved defeat interface required from story",
            "",
            "Story owns the `SaveBlock3` aggregate and new-game/save-version plumbing. Its published bank reserves the fixed 533-bit allocation capacity (67 bytes) indexed by slot; the current dense inventory uses slots 0–135. The payload is exposed without revealing the aggregate:",
            "",
            "```c",
            "bool32 WayfarerSeviiTrainerDefeatGet(u16 slot);",
            "void WayfarerSeviiTrainerDefeatSet(u16 slot);",
            "void WayfarerSeviiTrainerDefeatClear(u16 slot);",
            "```",
            "",
            "The functions must ignore slots at or above 533 safely. Trainer runtime maps every generated ID to its frozen `defeat_base` slot before calling them. Base victories set one bit; rematch IDs alias that base bit for object presentation and never allocate another persistent bit. Story objective callers may use their reserved slots through the same accessors. The Trainer branch will not define another saved aggregate.",
            "",
            "## Runtime surfaces owned here",
            "",
            "The Trainer branch will publish generated constants/roster data for all 136 keys, `TRAINERS_COUNT_WAYFARER = 1651`, one scaling classification per populated ID, the generated defeat-base lookup, and a separate Sevii Vs. Seeker registry. The registry consumes Story's compact persistent rematch-stage and pending-ready accessors and does not append 64 families to the existing fixed SaveBlock1 rematch-index array. Trainer owns Sevii flag slot 52 (`FLAG_WAYFARER_SEVII_VS_SEEKER_CHARGING`) for the Wayfarer-only Vs. Seeker charge lifecycle; Story owns slots 11-51 and presentation flags 53-60 (including returned Lostelle at 60); unallocated slots begin at 61.",
            "",
            "Ordinary content uses normal victory/blackout routing. Story callers use their own objective continuations but consume the same generated roster IDs and defeat accessors. Tower remains excluded from ordinary scaling, persistent Trainer defeat, and this allocation.",
            "",
            "## Integration rule",
            "",
            "Consumers should integrate coherent prerequisite commits by hash and must not renumber the published range. Combined integration must merge domain-specific manifest records and regenerate the single projection outputs; no isolated branch can claim combined-manifest readiness.",
            "",
        ]
    )
    return "\n".join(lines)


def write_if_changed(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.is_file() or path.read_text(encoding="utf-8") != content:
        path.write_text(content, encoding="utf-8")


def scaling_parser():
    """Use the established trainerproc parser for the selected source records."""
    path = GAME_ROOT / "tools/trainer_scaling/generate.py"
    spec = importlib.util.spec_from_file_location("wayfarer_sevii_scaling_parser", path)
    if spec is None or spec.loader is None:
        raise InventoryError(f"cannot load selected Trainer parser: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def compiled_frlg_records(allocations: list[dict]) -> dict[str, str]:
    """Compile FRLG once, retaining only the frozen selected source records."""
    parser = scaling_parser()
    with tempfile.TemporaryDirectory(prefix="wayfarer-sevii-trainers-") as directory:
        directory = Path(directory)
        binary, output = directory / "trainerproc", directory / "trainers_frlg.h"

        def command(args: list[str], **kwargs) -> str:
            result = subprocess.run(args, text=True, capture_output=True, **kwargs)
            if result.returncode:
                raise InventoryError(f"selected Trainer compiler failed: {result.stderr.strip()}")
            return result.stdout

        command(["cc", "-O2", str(GAME_ROOT / "tools/trainerproc/main.c"), "-o", str(binary)])
        preprocessed = command([
            "cpp", "-traditional-cpp", "-P", "-DPOKEMON_WAYFARER", "-DPOKEMON_HNS",
            "-DIS_WAYFARER=1", "-DIS_HNS=1", "-DIS_FRLG=0", "-DIS_EMERALD=0",
            "-I", str(GAME_ROOT / "include"), str(PARTIES),
        ])
        command([str(binary), "-i", "src/data/trainers_frlg.party", "-o", str(output), "-"], input=preprocessed)
        header = output.read_text(encoding="utf-8")

    records = parser.parse_output(header, "src/data/trainers_frlg.party")
    result = {}
    for row in allocations:
        source = row["source_trainer"]
        if source not in records:
            raise InventoryError(f"selected Trainer compiler output is absent: {source}")
        match = re.search(rf"\[{re.escape(source)}\]\s*=\s*\{{", header)
        if match is None:
            raise InventoryError(f"selected Trainer compiled record is absent: {source}")
        _, end = parser.balanced(header, match.end() - 1)
        # Generated source locations should not leak source FRLG names into the
        # Wayfarer compilation unit. The record itself remains byte-for-byte
        # trainerproc output apart from its destination identity.
        record = re.sub(r"(?m)^#line .*\n", "", header[match.start():end])
        record = record.replace(f"[{source}]", f"[{row['id']}]", 1)
        if f"[{source}]" in record:
            raise InventoryError(f"raw FRLG Trainer ID leaked into generated record: {source}")
        result[source] = record.strip() + "\n"
    if len(result) != len(allocations):
        raise InventoryError("selected Trainer record count drift")
    return result


def render_runtime_constants(report: dict) -> str:
    allocations = report["allocation"]["allocations"]
    lines = [
        "// Generated by tools/wayfarer_sevii_trainers/generate.py; do not edit.",
        "// Frozen selected FRLG source keys; source numeric IDs are provenance only.",
        "#ifndef GUARD_CONSTANTS_WAYFARER_SEVII_TRAINERS_H",
        "#define GUARD_CONSTANTS_WAYFARER_SEVII_TRAINERS_H",
        "",
    ]
    lines.extend(f"#define {row['id']:<68} {row['numeric_id']}" for row in allocations)
    lines.extend([
        "",
        f"#define TRAINER_WAYFARER_SEVII_FIRST {ALLOCATION_BASE}",
        f"#define TRAINER_WAYFARER_SEVII_LAST  {allocations[-1]['numeric_id']}",
        "#if IS_WAYFARER",
        f"#define TRAINERS_COUNT_WAYFARER      {report['allocation']['next_id']}",
        "#endif",
        "",
        "#endif  // GUARD_CONSTANTS_WAYFARER_SEVII_TRAINERS_H",
        "",
    ])
    return "\n".join(lines)


def render_runtime_roster(report: dict) -> str:
    allocations = report["allocation"]["allocations"]
    records = compiled_frlg_records(allocations)
    lines = [
        "// Generated by tools/wayfarer_sevii_trainers/generate.py; do not edit.",
        "// Only frozen selected FRLG Trainer records appear in this Wayfarer table.",
        "",
    ]
    for row in allocations:
        lines.append(records[row["source_trainer"]].rstrip() + ",")
        lines.append("")
    result = "\n".join(lines)
    expected = len(allocations)
    actual = len(re.findall(r"\[TRAINER_WAYFARER_SEVII_", result))
    if actual != expected:
        raise InventoryError(f"generated roster count drift: expected {expected}, found {actual}")
    return result


def render_defeat_router(report: dict) -> str:
    """Render the runtime-ID to persistent-defeat-slot projection.

    The allocation inventory deliberately contains a record for each selected
    rematch party.  Persistent defeat state is instead per source identity, so
    every rematch record resolves to its family's base slot here.
    """
    allocations = report["allocation"]["allocations"]
    base_slots = {
        row["source_trainer"]: row["slot"]
        for row in allocations
        if row["kind"] in ("base", "planned")
    }
    slots = []
    for row in allocations:
        try:
            slots.append(base_slots[row["defeat_base"]])
        except KeyError as error:
            raise InventoryError(f"unknown defeat base {row['defeat_base']}") from error

    if len(slots) != len(allocations):
        raise InventoryError("generated defeat-slot count drift")
    if max(slots) >= len(allocations):
        raise InventoryError("generated defeat slot exceeds allocation")

    lines = [
        "// Generated by tools/wayfarer_sevii_trainers/generate.py; do not edit.",
        "// Maps every selected runtime Trainer ID to its persistent defeat slot.",
        "#ifndef GUARD_WAYFARER_SEVII_TRAINER_DEFEATS_H",
        "#define GUARD_WAYFARER_SEVII_TRAINER_DEFEATS_H",
        "",
        "#include \"global.h\"",
        "#include \"constants/wayfarer_sevii_trainers.h\"",
        "",
        "#define WAYFARER_SEVII_TRAINER_DEFEAT_SLOT_NONE 0xFFFF",
        "",
        "static const u16 sWayfarerSeviiTrainerDefeatSlots[] =",
        "{",
    ]
    lines.extend(f"    {slot}," for slot in slots)
    lines.extend([
        "};",
        "",
        "static inline u16 WayfarerSeviiTrainerGetDefeatSlot(u16 trainerId)",
        "{",
        "    if (trainerId < TRAINER_WAYFARER_SEVII_FIRST || trainerId > TRAINER_WAYFARER_SEVII_LAST)",
        "        return WAYFARER_SEVII_TRAINER_DEFEAT_SLOT_NONE;",
        "    return sWayfarerSeviiTrainerDefeatSlots[trainerId - TRAINER_WAYFARER_SEVII_FIRST];",
        "}",
        "",
        "#endif  // GUARD_WAYFARER_SEVII_TRAINER_DEFEATS_H",
        "",
    ])
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    report = build_report()
    report_path = REPO_ROOT / "docs/sevii-trainer-implementation/allocation-inventory.json"
    coordination_path = REPO_ROOT / "docs/sevii-trainer-implementation/coordination.md"
    outputs = {
        report_path: json.dumps(report, indent=2, sort_keys=True) + "\n",
        coordination_path: render_coordination(report),
        RUNTIME_CONSTANTS: render_runtime_constants(report),
        RUNTIME_ROSTER: render_runtime_roster(report),
        RUNTIME_DEFEAT_ROUTER: render_defeat_router(report),
    }
    if args.check:
        stale = [path for path, content in outputs.items() if not path.is_file() or path.read_text(encoding="utf-8") != content]
        if stale:
            parser.error("stale generated Trainer coordination: " + ", ".join(str(path) for path in stale))
        return 0
    for path, content in outputs.items():
        write_if_changed(path, content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
