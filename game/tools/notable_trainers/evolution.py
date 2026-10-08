"""Validate authored v0 evolution thresholds and generate ROM predecessor rows."""

import argparse
import importlib.util
import json
import re
from pathlib import Path


GAME = Path(__file__).resolve().parents[2]
CATALOG = Path(__file__).with_suffix(".json")
OUTPUT = GAME / "src/data/notable_moves/predecessors.h"
SPECIES = GAME / "include/constants/species.h"
SOURCE = GAME / "src/data/pokemon/species_info.h"
WILD_PARSER = GAME / "tools/wild_encounters/wild_encounters_to_header.py"


def symbols():
    text = SPECIES.read_text()
    values = {
        symbol: int(value)
        for symbol, value in re.findall(r"^#define\s+(SPECIES_[A-Z0-9_]+)\s+(\d+)\s*$", text, re.M)
    }
    by_name = {}
    for symbol in values:
        by_name.setdefault(re.sub(r"[^a-z0-9]", "", symbol[8:].lower()), []).append(symbol)
    return values, by_name


def species_symbol(name, by_name):
    special = {"Nidoran♀": "SPECIES_NIDORAN_F", "Nidoran♂": "SPECIES_NIDORAN_M"}
    if name in special:
        return special[name]
    matches = by_name.get(re.sub(r"[^a-z0-9]", "", name.lower()), [])
    if len(matches) != 1:
        raise ValueError(f"{name}: expected one species constant, got {matches}")
    return matches[0]


def game_evolutions():
    spec = importlib.util.spec_from_file_location("wild_encounters_to_header", WILD_PARSER)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.active_evolutions(SOURCE)


def rows():
    data = json.loads(CATALOG.read_text())
    if (data.get("version") != 1 or not isinstance(data.get("chains"), dict)
            or not isinstance(data.get("babies"), list)
            or not isinstance(data.get("nonLevelEdges"), list)):
        raise ValueError("expected v1 evolution catalog")
    values, by_name = symbols()
    evolutions = game_evolutions()
    result = {}
    babies = {species_symbol(name, by_name) for name in data["babies"]}
    for predecessor, edges in evolutions.items():
        if predecessor in babies or predecessor not in values:
            continue
        for edge in edges:
            if (edge["method"] not in ("EVO_LEVEL", "EVO_LEVEL_BATTLE_ONLY")
                    or not edge["parameter"].isdecimal() or int(edge["parameter"]) == 0
                    or edge["target"] not in values):
                continue
            successor = edge["target"]
            row = (successor, predecessor, 0)
            previous = result.get(successor)
            if previous is not None and previous != row:
                raise ValueError(f"{successor}: ambiguous numeric predecessor")
            result[successor] = row
    for final, chain in data["chains"].items():
        if len(chain) % 2 != 1 or chain[-1] != final:
            raise ValueError(f"{final}: invalid chain")
        last_level = 1
        for index in range(0, len(chain) - 2, 2):
            predecessor = species_symbol(chain[index], by_name)
            successor = species_symbol(chain[index + 2], by_name)
            authored_level = chain[index + 1]
            if not isinstance(authored_level, int) or not last_level < authored_level <= 100:
                raise ValueError(f"{predecessor}->{successor}: invalid threshold {authored_level}")
            candidates = [edge for edge in evolutions[predecessor] if edge["target"] == successor]
            if not candidates:
                raise ValueError(f"{predecessor}->{successor}: not in active game evolution graph")
            numeric = [int(edge["parameter"]) for edge in candidates
                       if edge["method"] in ("EVO_LEVEL", "EVO_LEVEL_BATTLE_ONLY")
                       and edge["parameter"].isdecimal()
                       and int(edge["parameter"]) > 0]
            if numeric and (len(set(numeric)) != 1 or numeric[0] != authored_level):
                raise ValueError(f"{predecessor}->{successor}: catalog {authored_level} differs from game {numeric}")
            row = (successor, predecessor, 0 if numeric else authored_level)
            previous = result.get(successor)
            if previous is not None and previous != row:
                raise ValueError(f"{successor}: conflicting predecessor")
            result[successor] = row
            last_level = authored_level
    for index, edge in enumerate(data["nonLevelEdges"]):
        if set(edge) != {"predecessor", "successor", "level"}:
            raise ValueError(f"nonLevelEdges/{index}: expected predecessor, successor, level")
        predecessor, successor, level = edge["predecessor"], edge["successor"], edge["level"]
        if predecessor not in values or successor not in values or predecessor in babies:
            raise ValueError(f"nonLevelEdges/{index}: invalid species")
        if not isinstance(level, int) or level < 2 or level > 100:
            raise ValueError(f"nonLevelEdges/{index}: invalid level")
        candidates = [e for e in evolutions[predecessor] if e["target"] == successor]
        if not candidates or any(e["method"] in ("EVO_LEVEL", "EVO_LEVEL_BATTLE_ONLY")
                                 and e["parameter"].isdecimal() and int(e["parameter"]) > 0
                                 for e in candidates):
            raise ValueError(f"nonLevelEdges/{index}: not a nonlevel game edge")
        if successor in result:
            raise ValueError(f"nonLevelEdges/{index}: duplicate predecessor for {successor}")
        result[successor] = (successor, predecessor, level)

    for predecessor, edges in evolutions.items():
        if predecessor in babies:
            continue
        for edge in edges:
            if edge["method"] != "EVO_NONE" and edge["target"] in values and edge["target"] not in result:
                raise ValueError(f"{predecessor}->{edge['target']}: missing evolution threshold")

    stage = {}
    def level_of(species, visiting=()):
        if species in stage:
            return stage[species]
        if species in visiting:
            raise ValueError(f"{species}: predecessor cycle")
        if species not in result:
            return 1
        _, predecessor, authored_level = result[species]
        if predecessor in babies:
            raise ValueError(f"{species}: baby predecessor")
        if authored_level:
            level = authored_level
        else:
            levels = {int(e["parameter"]) for e in evolutions[predecessor]
                      if e["target"] == species and e["method"] in ("EVO_LEVEL", "EVO_LEVEL_BATTLE_ONLY")
                      and e["parameter"].isdecimal() and int(e["parameter"]) > 0}
            if len(levels) != 1:
                raise ValueError(f"{species}: missing numeric evolution level")
            level = levels.pop()
        if level <= level_of(predecessor, visiting + (species,)):
            raise ValueError(f"{species}: evolution level does not increase along line")
        stage[species] = level
        return level

    for species in result:
        level_of(species)
    ordered = sorted(result.values(), key=lambda row: values[row[0]])
    return ordered


def stage_table():
    """Return {successor: (predecessor, level)} with engine EVO_LEVEL values resolved.

    This is the exact edge set and threshold the ROM's StepDownSpeciesToLevel
    uses, so Python projections match runtime.
    """
    evolutions = game_evolutions()
    table = {}
    for successor, predecessor, level in rows():
        if not level:
            level = next(int(e["parameter"]) for e in evolutions[predecessor]
                         if e["target"] == successor and e["method"] in ("EVO_LEVEL", "EVO_LEVEL_BATTLE_ONLY")
                         and e["parameter"].isdecimal() and int(e["parameter"]) > 0)
        table[successor] = (predecessor, level)
    return table


def step_down(species, level, table):
    """Python twin of StepDownSpeciesToLevel."""
    for _ in range(8):
        edge = table.get(species)
        if edge is None or level >= edge[1]:
            break
        species = edge[0]
    return species


def baby_rows():
    """Keep baby ancestry available for egg moves, separately from party scaling."""
    data = json.loads(CATALOG.read_text())
    values, by_name = symbols()
    evolutions = game_evolutions()
    result = {}
    for name in data["babies"]:
        predecessor = species_symbol(name, by_name)
        for edge in evolutions.get(predecessor, []):
            successor = edge["target"]
            if edge["method"] == "EVO_NONE" or successor not in values:
                continue
            row = (successor, predecessor, 0)
            if successor in result and result[successor] != row:
                raise ValueError(f"{successor}: ambiguous baby predecessor")
            result[successor] = row
    return sorted(result.values(), key=lambda row: values[row[0]])


def render(entries):
    lines = ["// Generated by game/tools/notable_trainers/evolution.py. Do not edit.",
             "// Zero means the engine's active EVO_LEVEL parameter owns the threshold.",
             "static const struct NotablePredecessor sNotablePredecessors[] =", "{"]
    for species, predecessor, level in entries:
        lines.append(f"    {{ {species}, {predecessor}, {level} }},")
    lines.extend(["};", "", "// Egg-move ancestry includes babies; party step-down does not.",
                  "static const struct NotablePredecessor sNotableBabyPredecessors[] =", "{"])
    for species, predecessor, level in baby_rows():
        lines.append(f"    {{ {species}, {predecessor}, {level} }},")
    lines.extend(["};", ""])
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    text = render(rows())
    if args.check:
        if not OUTPUT.exists() or OUTPUT.read_text() != text:
            raise SystemExit(f"out of date: {OUTPUT}")
    else:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(text)


if __name__ == "__main__":
    main()
