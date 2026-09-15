#!/usr/bin/env python3
"""Audit the built-in local Trainer Tower payload without compiling a ROM.

The local course intentionally uses only the source-owned table in
``src/trainer_tower_sets.c``.  This parser is deliberately narrow: it accepts
the designated-initializer shape used by that source and fails closed when a
field, pointer, symbolic reference, or frozen course identity changes.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any


class TowerAuditError(ValueError):
    """The frozen local Trainer Tower course is incomplete or has drifted."""


FORMATS = ("SINGLE", "DOUBLE", "KNOCKOUT", "MIXED")
FORMAT_ROWS = {
    "SINGLE": tuple(f"sTrainerTowerFloor_Single_{index}" for index in range(1, 9)),
    "DOUBLE": tuple(f"sTrainerTowerFloor_Double_{index}" for index in range(1, 9)),
    "KNOCKOUT": tuple(f"sTrainerTowerFloor_Knockout_{index}" for index in range(1, 9)),
    "MIXED": ("sTrainerTowerFloor_Mixed_1", "sTrainerTowerFloor_Mixed_2", "sTrainerTowerFloor_Mixed_3",
              "sTrainerTowerFloor_Double_8", "sTrainerTowerFloor_Mixed_5", "sTrainerTowerFloor_Knockout_8",
              "sTrainerTowerFloor_Double_3", "sTrainerTowerFloor_Knockout_2"),
}
MIXED_TYPES = ("SINGLE", "SINGLE", "SINGLE", "DOUBLE", "DOUBLE", "KNOCKOUT", "DOUBLE", "KNOCKOUT")
FORMAT_PRIZES = {"SINGLE": "TTPRIZE_UP_GRADE", "DOUBLE": "TTPRIZE_DRAGON_SCALE",
                 "KNOCKOUT": "TTPRIZE_METAL_COAT", "MIXED": "TTPRIZE_KINGS_ROCK"}
REAL_TRAINERS = {"SINGLE": 1, "DOUBLE": 2, "KNOCKOUT": 3}
MON_FIELDS = ("species", "heldItem", "moves", "hpEV", "attackEV", "defenseEV", "speedEV", "spAttackEV",
              "spDefenseEV", "otId", "hpIV", "attackIV", "defenseIV", "speedIV", "spAttackIV",
              "spDefenseIV", "abilityNum", "personality", "nickname", "friendship")
TRAINER_FIELDS = ("name", "facilityClass", "textColor", "speechBefore", "speechWin", "speechLose", "speechAfter", "mons")
# Updated only by deliberately reviewing a regenerated report.  It is the
# canonical selected payload, not a hash of the entire mutable C source file.
FROZEN_LOCAL_PAYLOAD_SHA256 = "14615d30d634825bf7bef5185d0eafebf0fc62f35a094c146aef58d35ff229c1"


def _fail(message: str) -> None:
    raise TowerAuditError(message)


def _canonical(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True).encode("ascii")


def _sha(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _norm(value: str) -> str:
    return re.sub(r"\s+", "", value)


def _without_comments(source: str) -> str:
    """Remove C comments while leaving string literals intact."""
    out, index, quote = [], 0, None
    while index < len(source):
        char, following = source[index], source[index + 1:index + 2]
        if quote:
            out.append(char)
            if char == "\\" and following:
                out.append(following); index += 2; continue
            if char == quote: quote = None
            index += 1; continue
        if char in "\"'": quote = char; out.append(char); index += 1; continue
        if char == "/" and following == "/":
            index = source.find("\n", index)
            if index < 0: break
            out.append("\n"); index += 1; continue
        if char == "/" and following == "*":
            end = source.find("*/", index + 2)
            if end < 0: _fail("unterminated C comment")
            index = end + 2; continue
        out.append(char); index += 1
    return "".join(out)


def _matching(source: str, start: int) -> int:
    if start < 0 or source[start] != "{": _fail("expected initializer brace")
    depth, index, quote = 0, start, None
    while index < len(source):
        char = source[index]
        if quote:
            if char == "\\": index += 2; continue
            if char == quote: quote = None
        elif char in "\"'": quote = char
        elif char == "{": depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0: return index
        index += 1
    _fail("unterminated initializer")


def _unwrap(value: str, label: str) -> str:
    value = value.strip()
    if not value.startswith("{") or _matching(value, 0) != len(value) - 1:
        _fail(f"{label} must be a braced initializer")
    return value[1:-1]


def _split(value: str) -> list[str]:
    """Split a braced initializer's contents on its top-level commas."""
    parts, start, depth, quote = [], 0, 0, None
    for index, char in enumerate(value):
        if quote:
            if char == "\\": continue
            if char == quote: quote = None
        elif char in "\"'": quote = char
        elif char in "({[": depth += 1
        elif char in ")}]": depth -= 1
        elif char == "," and depth == 0:
            if value[start:index].strip(): parts.append(value[start:index].strip())
            start = index + 1
    if value[start:].strip(): parts.append(value[start:].strip())
    return parts


def _fields(value: str, label: str) -> dict[str, str]:
    result: dict[str, str] = {}
    for part in _split(_unwrap(value, label)):
        match = re.fullmatch(r"\.(\w+)\s*=\s*(.*)", part, flags=re.S)
        if not match: _fail(f"{label} has a non-designated field: {part[:48]!r}")
        name, field = match.group(1), match.group(2).strip()
        if name in result: _fail(f"{label} repeats .{name}")
        result[name] = field
    return result


def _initializer(source: str, name: str, required_static: bool = True) -> str:
    qualifier = r"static\s+const\s+struct" if required_static else r"const\s+struct"
    # The table is a pointer-to-const array, while floors are plain structs.
    # Accept both declarator shapes but only after the exact struct qualifier.
    match = re.search(rf"\b{qualifier}\b[^;={{]*?\b{re.escape(name)}(?:\s*\[[^]]+\])*\s*=\s*", source)
    if match is None: _fail(f"missing local initializer {name}")
    start, end = source.find("{", match.end()), -1
    end = _matching(source, start)
    return source[start:end + 1]


def _symbol_set(path: Path, prefix: str) -> set[str]:
    try: source = path.read_text(encoding="utf-8")
    except OSError as error: _fail(f"cannot read {path}: {error}")
    return set(re.findall(rf"^\s*(?:#define\s+)?({prefix}[A-Z0-9_]+)\s*(?:=|,|\b)", source, re.M))


def _number(value: str, label: str, maximum: int) -> int:
    try: number = int(value.strip(), 0)
    except ValueError: _fail(f"{label} must be an integer")
    if not 0 <= number <= maximum: _fail(f"{label} is outside 0..{maximum}")
    return number


def _speech(value: str, label: str, moves: set[str], species: set[str], words: set[str]) -> list[str]:
    tokens = _split(_unwrap(value, label))
    if len(tokens) != 6: _fail(f"{label} must contain six Easy Chat words")
    result = []
    for token in tokens:
        token = _norm(token)
        if token == "0xFFFF" or token in words: result.append(token); continue
        macro = re.fullmatch(r"EC_(MOVE|MOVE2)\(([A-Z0-9_]+)\)", token)
        if macro and f"MOVE_{macro.group(2)}" in moves: result.append(token); continue
        macro = re.fullmatch(r"EC_(POKEMON|POKEMON_NATIONAL)\(([A-Z0-9_]+)\)", token)
        if macro and f"SPECIES_{macro.group(2)}" in species: result.append(token); continue
        _fail(f"{label} has invalid Easy Chat reference {token}")
    return result


def _mon(value: str, label: str, refs: dict[str, set[str]], personalities: set[str]) -> dict[str, Any]:
    fields = _fields(value, label)
    if tuple(fields) != MON_FIELDS: _fail(f"{label} fields differ from the frozen BattleTowerPokemon contract")
    species, item = _norm(fields["species"]), _norm(fields["heldItem"])
    if species not in refs["species"]: _fail(f"{label}.species is invalid: {species}")
    if item not in refs["items"]: _fail(f"{label}.heldItem is invalid: {item}")
    move_values = [_norm(item) for item in _split(_unwrap(fields["moves"], f"{label}.moves"))]
    if len(move_values) != 4 or any(item not in refs["moves"] for item in move_values): _fail(f"{label}.moves has invalid move references")
    evs = {name: _number(fields[name], f"{label}.{name}", 255) for name in MON_FIELDS[3:9]}
    ivs = {name: _number(fields[name], f"{label}.{name}", 31) for name in MON_FIELDS[10:16]}
    ot_id = _norm(fields["otId"])
    if ot_id != "TRAINER_TOWER_OTID" and not re.fullmatch(r"\d+\|\(0<<16\)", ot_id):
        _fail(f"{label}.otId is not a static local owner id")
    ability = _number(fields["abilityNum"], f"{label}.abilityNum", 1)
    personality = _norm(fields["personality"])
    if personality not in personalities: _fail(f"{label}.personality is not a local personality macro")
    if not re.fullmatch(r'_\("[^"\\]+"\)', _norm(fields["nickname"])): _fail(f"{label}.nickname is invalid")
    friendship = _norm(fields["friendship"])
    if friendship != "MAX_FRIENDSHIP": friendship = _number(friendship, f"{label}.friendship", 255)
    return {"species": species, "held_item": item, "moves": move_values, "evs": evs, "ivs": ivs,
            "ability": ability, "personality": personality, "nickname": _norm(fields["nickname"]),
            "ot_id": ot_id, "friendship": friendship}


def _trainer(value: str, label: str, refs: dict[str, set[str]], personalities: set[str]) -> dict[str, Any] | None:
    if re.fullmatch(r"DUMMY_TOWER_TEAM\(\d+\)", _norm(value)): return None
    fields = _fields(value, label)
    if tuple(fields) != TRAINER_FIELDS: _fail(f"{label} fields differ from the frozen TrainerTowerTrainer contract")
    name, facility = _norm(fields["name"]), _norm(fields["facilityClass"])
    if not re.fullmatch(r'_\("[^"\\]+"\)', name): _fail(f"{label}.name is invalid")
    if facility not in refs["classes"]: _fail(f"{label}.facilityClass is invalid: {facility}")
    text_color = _number(fields["textColor"], f"{label}.textColor", 15)
    speeches = {name: _speech(fields[name], f"{label}.{name}", refs["moves"], refs["species"], refs["words"])
                for name in ("speechBefore", "speechWin", "speechLose", "speechAfter")}
    mons = _split(_unwrap(fields["mons"], f"{label}.mons"))
    if len(mons) != 6: _fail(f"{label}.mons must contain six static opponents")
    parsed = [_mon(mon, f"{label}.mons[{index}]", refs, personalities) for index, mon in enumerate(mons)]
    return {"name": name, "facility_class": facility, "text_color": text_color, "speeches": speeches,
            "party": parsed, "speech_sha256": _sha(speeches), "party_sha256": _sha(parsed)}


def _floor(source: str, name: str, refs: dict[str, set[str]], personalities: set[str]) -> dict[str, Any]:
    fields = _fields(_initializer(source, name), name)
    required = ("id", "floorIdx", "challengeType", "prize", "trainers", "checksum")
    if tuple(fields) != required: _fail(f"{name} fields differ from the frozen TrainerTowerFloor contract")
    if _norm(fields["floorIdx"]) != "MAX_TRAINER_TOWER_FLOORS": _fail(f"{name}.floorIdx is not the eight-floor local course")
    challenge = _norm(fields["challengeType"]).removeprefix("CHALLENGE_TYPE_")
    if challenge not in FORMATS[:3]: _fail(f"{name}.challengeType is invalid: {challenge}")
    prize = _norm(fields["prize"])
    if not re.fullmatch(r"TTPRIZE_[A-Z0-9_]+", prize): _fail(f"{name}.prize is invalid: {prize}")
    checksum = _number(fields["checksum"], f"{name}.checksum", 0xFFFFFFFF)
    trainers = [_trainer(value, f"{name}.trainers[{index}]", refs, personalities)
                for index, value in enumerate(_split(_unwrap(fields["trainers"], f"{name}.trainers")))]
    if len(trainers) != 3: _fail(f"{name}.trainers must contain three actor slots")
    real = [trainer for trainer in trainers if trainer is not None]
    if len(real) != REAL_TRAINERS[challenge]: _fail(f"{name} has wrong actor count for {challenge}")
    result = {"symbol": name, "id": _number(fields["id"], f"{name}.id", 255), "challenge_type": challenge,
              "prize": prize, "source_checksum": checksum, "trainers": real}
    result["sha256"] = _sha(result)
    return result


def _table(source: str) -> dict[str, tuple[str, ...]]:
    table = _unwrap(_initializer(source, "gTrainerTowerFloors", required_static=False), "gTrainerTowerFloors")
    fields: dict[str, str] = {}
    for part in _split(table):
        match = re.fullmatch(r"\[(CHALLENGE_TYPE_[A-Z]+)\]\s*=\s*(.*)", part, flags=re.S)
        if not match: _fail(f"local floor table has an invalid format row: {part[:48]!r}")
        if match.group(1) in fields: _fail(f"local floor table repeats {match.group(1)}")
        fields[match.group(1)] = match.group(2)
    if set(fields) != {f"CHALLENGE_TYPE_{name}" for name in FORMATS}: _fail("local floor table must expose exactly four formats")
    result = {}
    for name in FORMATS:
        rows = tuple(re.fullmatch(r"&(sTrainerTowerFloor_[A-Za-z0-9_]+)", _norm(row)).group(1)
                     if re.fullmatch(r"&(sTrainerTowerFloor_[A-Za-z0-9_]+)", _norm(row)) else _fail(f"{name} floor table has a non-local pointer")
                     for row in _split(_unwrap(fields[f"CHALLENGE_TYPE_{name}"], f"{name} floor table")))
        if len(rows) != 8: _fail(f"{name} floor table must contain eight floors")
        result[name] = rows
    return result


def _header(source: str) -> dict[str, int]:
    fields = _fields(_initializer(source, "gTrainerTowerLocalHeader", required_static=False), "gTrainerTowerLocalHeader")
    if set(fields) != {"numFloors", "id"}: _fail("local header must not carry external checksum or payload fields")
    if _norm(fields["numFloors"]) != "MAX_TRAINER_TOWER_FLOORS": _fail("local header does not declare eight floors")
    if _number(fields["id"], "local header id", 255) != 1: _fail("local header id changed from the frozen built-in set")
    return {"id": 1, "num_floors": 8}


def _source_text(root: Path, source_text: str | None) -> str:
    if source_text is not None: return _without_comments(source_text)
    path = root / "src/trainer_tower_sets.c"
    try: return _without_comments(path.read_text(encoding="utf-8"))
    except OSError as error: _fail(f"cannot read frozen local set source {path}: {error}")


def build_report(root: Path | str, source_text: str | None = None) -> dict[str, Any]:
    """Return a deterministic report, failing closed on local-course drift."""
    root, source = Path(root), _source_text(Path(root), source_text)
    forbidden = ("#include \"ereader.h\"", "#include \"mystery_gift.h\"", "#include \"record_mixing.h\"",
                 "gReceivedTrainerTower", "gEReader", "sEReader")
    if any(token in source for token in forbidden): _fail("local Trainer Tower source depends on external or e-Reader payload")
    refs = {"species": _symbol_set(root / "include/constants/species.h", "SPECIES_"),
            "items": _symbol_set(root / "include/constants/items.h", "ITEM_"),
            "moves": _symbol_set(root / "include/constants/moves.h", "MOVE_"),
            "classes": _symbol_set(root / "include/constants/trainers.h", "FACILITY_CLASS_"),
            "words": _symbol_set(root / "include/constants/easy_chat.h", "EC_WORD_")}
    personalities = set(re.findall(r"^\s*#define\s+(PERSONALITY_[A-Z0-9_]+)\b", source, re.M))
    if not personalities: _fail("local personality constants are absent")
    header, table = _header(source), _table(source)
    if table != FORMAT_ROWS: _fail("local floor table drifted from the frozen four-format course")
    floors = {symbol: _floor(source, symbol, refs, personalities)
              for symbol in sorted({symbol for rows in table.values() for symbol in rows})}
    formats = {}
    for name, rows in table.items():
        selected = [floors[row] for row in rows]
        types = tuple(row["challenge_type"] for row in selected)
        expected_types = (name,) * 8 if name != "MIXED" else MIXED_TYPES
        if types != expected_types: _fail(f"{name} authored floor formats are wrong: {types}")
        # The FRLG runtime reads the local set prize from floor zero. Remaining
        # floor prize fields are retained and hashed source data, not rewards.
        if selected[0]["prize"] != FORMAT_PRIZES[name]: _fail(f"{name} source prize is not {FORMAT_PRIZES[name]}")
        formats[name] = {"floor_symbols": list(rows), "floor_types": list(types), "prize": FORMAT_PRIZES[name],
                         "floor_sha256": [row["sha256"] for row in selected]}
    payload = {"header": header, "formats": formats, "floors": floors}
    fingerprint = _sha(payload)
    if FROZEN_LOCAL_PAYLOAD_SHA256 and fingerprint != FROZEN_LOCAL_PAYLOAD_SHA256:
        _fail(f"frozen local Trainer Tower source drift: expected {FROZEN_LOCAL_PAYLOAD_SHA256}, got {fingerprint}")
    return {"invariants": {"passed": True, "external_data": False, "ereader_payload": False,
                            "format_count": 4, "floors_per_format": 8, "mixed_sequence": list(FORMAT_ROWS["MIXED"])},
            "local_header": {**header, "sha256": _sha(header)}, "formats": formats, "floors": floors,
            "local_payload_sha256": fingerprint, "source_sha256": hashlib.sha256(source.encode()).hexdigest()}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path, help="write JSON report to this path")
    args = parser.parse_args(argv)
    try: report = build_report(args.root)
    except TowerAuditError as error:
        print(f"wayfarer-trainer-tower-audit: {error}", file=sys.stderr)
        return 1
    output = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output: args.output.write_text(output, encoding="utf-8")
    else: print(output, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
