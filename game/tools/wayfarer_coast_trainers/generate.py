#!/usr/bin/env python3
"""Generate the selected FRLG coast Trainer roster for Wayfarer."""

from __future__ import annotations

import hashlib
import importlib.util
import json
import re
import subprocess
import tempfile
from pathlib import Path


TOOL_DIR = Path(__file__).resolve().parent
GAME_ROOT = TOOL_DIR.parents[1]
REPO_ROOT = GAME_ROOT.parent
PARTIES = GAME_ROOT / "src/data/trainers_frlg.party"
BASE = 1651

# The ordering is stable: script-facing identities come first, followed by
# every authored staged party selected by the FRLG rematch tables.
BASE_TRAINERS = (
    # Routes 19, 20, 21 North, and 21 South.
    ("TRAINER_SWIMMER_MALE_RICHARD", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_REECE", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_MATTHEW", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_DOUGLAS", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_DAVID", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_TONY", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_AXLE", "ORDINARY"),
    ("TRAINER_SWIMMER_FEMALE_ANYA", "ORDINARY"),
    ("TRAINER_SWIMMER_FEMALE_ALICE", "ORDINARY"),
    ("TRAINER_SWIMMER_FEMALE_CONNIE", "ORDINARY"),
    ("TRAINER_SIS_AND_BRO_LIA_LUC", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_BARRY", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_DEAN", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_DARRIN", "ORDINARY"),
    ("TRAINER_SWIMMER_FEMALE_TIFFANY", "ORDINARY"),
    ("TRAINER_SWIMMER_FEMALE_NORA", "ORDINARY"),
    ("TRAINER_SWIMMER_FEMALE_SHIRLEY", "ORDINARY"),
    ("TRAINER_BIRD_KEEPER_ROGER", "ORDINARY"),
    ("TRAINER_PICNICKER_MISSY", "ORDINARY"),
    ("TRAINER_PICNICKER_IRENE", "ORDINARY"),
    ("TRAINER_FISHERMAN_RONALD", "ORDINARY"),
    ("TRAINER_FISHERMAN_CLAUDE", "ORDINARY"),
    ("TRAINER_FISHERMAN_WADE", "ORDINARY"),
    ("TRAINER_FISHERMAN_NOLAN", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_SPENCER", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_JACK", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_JEROME", "ORDINARY"),
    ("TRAINER_SWIMMER_MALE_ROLAND", "ORDINARY"),
    ("TRAINER_SIS_AND_BRO_LIL_IAN", "ORDINARY"),
    ("TRAINER_SWIMMER_FEMALE_MELISSA", "ORDINARY"),
    # Pokémon Mansion.
    ("TRAINER_SCIENTIST_TED", "ORDINARY"),
    ("TRAINER_YOUNGSTER_JOHNSON", "ORDINARY"),
    ("TRAINER_BURGLAR_ARNIE", "ORDINARY"),
    ("TRAINER_BURGLAR_SIMON", "ORDINARY"),
    ("TRAINER_SCIENTIST_BRAYDON", "ORDINARY"),
    ("TRAINER_BURGLAR_LEWIS", "ORDINARY"),
    ("TRAINER_SCIENTIST_IVAN", "ORDINARY"),
    # Cinnabar Gym.
    ("TRAINER_SUPER_NERD_ERIK", "GYM_MEMBER"),
    ("TRAINER_SUPER_NERD_AVERY", "GYM_MEMBER"),
    ("TRAINER_SUPER_NERD_DEREK", "GYM_MEMBER"),
    ("TRAINER_SUPER_NERD_ZAC", "GYM_MEMBER"),
    ("TRAINER_BURGLAR_QUINN", "GYM_MEMBER"),
    ("TRAINER_BURGLAR_RAMON", "GYM_MEMBER"),
    ("TRAINER_BURGLAR_DUSTY", "GYM_MEMBER"),
    ("TRAINER_LEADER_BLAINE", "GYM_LEADER"),
)

# Only these source families have later FRLG party records.  Families without
# a later record remain ordinary one-time Trainers even though their wrapper
# checks the common rematch helper.
REMATCH_STAGES = (
    ("TRAINER_SWIMMER_FEMALE_ALICE_2", "TRAINER_SWIMMER_FEMALE_ALICE"),
    ("TRAINER_SWIMMER_MALE_DARRIN_2", "TRAINER_SWIMMER_MALE_DARRIN"),
    ("TRAINER_PICNICKER_MISSY_2", "TRAINER_PICNICKER_MISSY"),
    ("TRAINER_PICNICKER_MISSY_3", "TRAINER_PICNICKER_MISSY"),
    ("TRAINER_FISHERMAN_WADE_2", "TRAINER_FISHERMAN_WADE"),
    ("TRAINER_SWIMMER_MALE_JACK_2", "TRAINER_SWIMMER_MALE_JACK"),
    ("TRAINER_SIS_AND_BRO_LIL_IAN_2", "TRAINER_SIS_AND_BRO_LIL_IAN"),
    ("TRAINER_SIS_AND_BRO_LIL_IAN_3", "TRAINER_SIS_AND_BRO_LIL_IAN"),
    ("TRAINER_SWIMMER_MALE_MATTHEW_2", "TRAINER_SWIMMER_MALE_MATTHEW"),
    ("TRAINER_SWIMMER_MALE_TONY_2", "TRAINER_SWIMMER_MALE_TONY"),
    ("TRAINER_SWIMMER_FEMALE_MELISSA_2", "TRAINER_SWIMMER_FEMALE_MELISSA"),
)


class InventoryError(ValueError):
    pass


def runtime_id(source: str) -> str:
    return "TRAINER_WAYFARER_COAST_" + source.removeprefix("TRAINER_")


def source_blocks() -> dict[str, str]:
    source = PARTIES.read_text(encoding="utf-8").replace("\r\n", "\n")
    markers = list(re.finditer(r"(?m)^=== (TRAINER_[A-Z0-9_]+) ===\s*$", source))
    return {
        match.group(1): source[match.start():markers[index + 1].start() if index + 1 < len(markers) else len(source)]
        for index, match in enumerate(markers)
    }


def build_inventory() -> list[dict]:
    bases = {source: index for index, (source, _) in enumerate(BASE_TRAINERS)}
    rows = []
    for source, policy in BASE_TRAINERS:
        rows.append({
            "id": runtime_id(source), "source_trainer": source,
            "defeat_base": source, "policy": policy, "kind": "base",
        })
    for source, defeat_base in REMATCH_STAGES:
        if defeat_base not in bases:
            raise InventoryError(f"unknown rematch base {defeat_base}")
        rows.append({
            "id": runtime_id(source), "source_trainer": source,
            "defeat_base": defeat_base, "policy": "ORDINARY", "kind": "rematch",
        })
    blocks = source_blocks()
    for index, row in enumerate(rows):
        source = row["source_trainer"]
        if source not in blocks:
            raise InventoryError(f"missing FRLG party source {source}")
        row["slot"] = index
        row["numeric_id"] = BASE + index
        row["source_hash"] = hashlib.sha256(blocks[source].encode()).hexdigest()
    if len(rows) != 56 or rows[-1]["numeric_id"] != 1706:
        raise InventoryError("coast allocation extent drift")
    return rows


def parser_module():
    path = GAME_ROOT / "tools/trainer_scaling/generate.py"
    spec = importlib.util.spec_from_file_location("wayfarer_coast_scaling_parser", path)
    if spec is None or spec.loader is None:
        raise InventoryError("cannot load trainer parser")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def compiled_records(rows: list[dict]) -> dict[str, str]:
    parser = parser_module()
    with tempfile.TemporaryDirectory(prefix="wayfarer-coast-trainers-") as directory:
        directory = Path(directory)
        binary, output = directory / "trainerproc", directory / "trainers_frlg.h"
        def command(args: list[str], **kwargs) -> str:
            result = subprocess.run(args, text=True, capture_output=True, **kwargs)
            if result.returncode:
                raise InventoryError(result.stderr.strip())
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
    for row in rows:
        source = row["source_trainer"]
        if source not in records:
            raise InventoryError(f"compiled record missing {source}")
        match = re.search(rf"\[{re.escape(source)}\]\s*=\s*\{{", header)
        if match is None:
            raise InventoryError(f"compiled source missing {source}")
        _, end = parser.balanced(header, match.end() - 1)
        record = re.sub(r"(?m)^#line .*\n", "", header[match.start():end])
        record = record.replace(f"[{source}]", f"[{row['id']}]", 1)
        if f"[{source}]" in record:
            raise InventoryError(f"raw ID leaked for {source}")
        result[source] = record.strip() + "\n"
    return result


def render_constants(rows: list[dict]) -> str:
    lines = [
        "// Generated by tools/wayfarer_coast_trainers/generate.py; do not edit.",
        "#ifndef GUARD_CONSTANTS_WAYFARER_COAST_TRAINERS_H",
        "#define GUARD_CONSTANTS_WAYFARER_COAST_TRAINERS_H",
        "",
    ]
    lines.extend(f"#define {row['id']:<72} {row['numeric_id']}" for row in rows)
    lines.extend([
        "", f"#define TRAINER_WAYFARER_COAST_FIRST {BASE}",
        f"#define TRAINER_WAYFARER_COAST_LAST  {rows[-1]['numeric_id']}",
        f"#define TRAINER_WAYFARER_COAST_COUNT {len(rows)}",
        "", "#if IS_WAYFARER", "#undef TRAINERS_COUNT_WAYFARER",
        f"#define TRAINERS_COUNT_WAYFARER      {BASE + len(rows)}", "",
    ])
    for row in rows:
        if row["kind"] != "base":
            continue
        lines.extend([f"#undef {row['source_trainer']}", f"#define {row['source_trainer']} {row['id']}"])
    lines.extend(["#endif", "", "#endif  // GUARD_CONSTANTS_WAYFARER_COAST_TRAINERS_H", ""])
    return "\n".join(lines)


def render_roster(rows: list[dict]) -> str:
    records = compiled_records(rows)
    lines = ["// Generated by tools/wayfarer_coast_trainers/generate.py; do not edit.", ""]
    for row in rows:
        lines.extend([records[row["source_trainer"]].rstrip() + ",", ""])
    return "\n".join(lines)


def render_defeat_router(rows: list[dict]) -> str:
    slots = {row["source_trainer"]: row["slot"] for row in rows if row["kind"] == "base"}
    values = [slots[row["defeat_base"]] for row in rows]
    lines = [
        "// Generated by tools/wayfarer_coast_trainers/generate.py; do not edit.",
        "#ifndef GUARD_WAYFARER_COAST_TRAINER_DEFEATS_H",
        "#define GUARD_WAYFARER_COAST_TRAINER_DEFEATS_H", "",
        "#include \"global.h\"", "#include \"constants/wayfarer_coast_trainers.h\"", "",
        "#define WAYFARER_COAST_TRAINER_DEFEAT_SLOT_NONE 0xFFFF", "",
        "static const u8 sWayfarerCoastTrainerDefeatSlots[] =", "{",
    ]
    lines.extend(f"    {value}," for value in values)
    lines.extend(["};", "", "static inline u16 WayfarerCoastTrainerGetDefeatSlot(u16 trainerId)", "{",
        "    if (trainerId < TRAINER_WAYFARER_COAST_FIRST || trainerId > TRAINER_WAYFARER_COAST_LAST)",
        "        return WAYFARER_COAST_TRAINER_DEFEAT_SLOT_NONE;",
        "    return sWayfarerCoastTrainerDefeatSlots[trainerId - TRAINER_WAYFARER_COAST_FIRST];",
        "}", "", "#endif  // GUARD_WAYFARER_COAST_TRAINER_DEFEATS_H", ""])
    return "\n".join(lines)


def render_report(rows: list[dict]) -> str:
    return json.dumps({"schema_version": 1, "allocation": {"base": BASE, "count": len(rows), "next_id": BASE + len(rows), "allocations": rows}}, indent=2, sort_keys=True) + "\n"


def write(path: Path, contents: str, check: bool) -> None:
    if check:
        if not path.is_file() or path.read_text(encoding="utf-8") != contents:
            raise InventoryError(f"stale generated output: {path}")
    else:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents, encoding="utf-8")


def main() -> int:
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    rows = build_inventory()
    outputs = {
        GAME_ROOT / "include/constants/wayfarer_coast_trainers.h": render_constants(rows),
        GAME_ROOT / "src/data/trainers_wayfarer_coast.h": render_roster(rows),
        GAME_ROOT / "include/wayfarer_coast_trainer_defeats.h": render_defeat_router(rows),
        REPO_ROOT / "docs/coast-trainer-implementation/allocation-inventory.json": render_report(rows),
    }
    for path, contents in outputs.items():
        write(path, contents, args.check)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
