"""Exhaustive, deterministic audit of projected opposing Trainer slots."""
from functools import lru_cache
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from gameplay_content import progression
import ast
import importlib.util
import json
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("wild_metadata", ROOT / "tools/wild_encounters/wild_encounters_to_header.py")
wild = importlib.util.module_from_spec(spec)
spec.loader.exec_module(wild)
ANCHORS = progression.load()["ordinary_trainer_baseline"]
CAPS = progression.load()["soft_cap"]
MILESTONES = (0, 4, 8, 16, 30, 40, 55, 63, 65, 68, 76, 80)


def interpolate(rating, anchors=ANCHORS):
    rating = max(0, min(80, rating))
    for (r0, v0), (r1, v1) in zip(anchors, anchors[1:]):
        if rating <= r1:
            return v0 + (2 * (rating - r0) * (v1 - v0) + r1 - r0) // (2 * (r1 - r0))
    return anchors[-1][1]


def project(rating, authored, policy):
    difference = authored - 5
    adjustment = (abs(difference) + 2) // 5 * (-1 if difference < 0 else 1)
    return max(1, min(100, interpolate(rating) + max(-1, min(8, adjustment)) + (2 if policy == "GYM_MEMBER" else 0)))


def preprocess(source):
    command = ["cpp", "-P", "-DTRUE=1", "-DFALSE=0", "-DPOKEMON_WAYFARER", "-I", str(ROOT / "include"), "-I", str(ROOT),
               "-include", "constants/global.h", "-include", "config/general.h", "-include", "config/pokemon.h", "-x", "c", "-"]
    result = subprocess.run(command, input=source, text=True, capture_output=True, check=True)
    return result.stdout


def blocks(source, pattern):
    for match in re.finditer(pattern, source):
        start = match.end() - 1
        end = wild.matching_delimiter(source, start, "{", "}", match.group(1))
        yield match.group(1), source[start:end + 1]


def learnsets(source):
    return {name: [(int(level), move) for move, level in re.findall(r"\.move\s*=\s*(MOVE_\w+)\s*,\s*\.level\s*=\s*(\d+)", body)]
            for name, body in blocks(source, r"struct LevelUpMove\s+(\w+)\s*\[\s*\]\s*=\s*\{")}


def number(expression):
    """Evaluate numeric species constants without executing source text."""
    expression = expression.strip()
    while expression.startswith("(") and expression.endswith(")") and wild.matching_delimiter(expression, 0, "(", ")", expression) == len(expression) - 1:
        expression = expression[1:-1].strip()
    if "?" in expression:
        condition, branches = expression.split("?", 1)
        yes, no = branches.split(":", 1)
        return number(yes if number(condition) else no)
    node = ast.parse(expression.replace("||", " or ").replace("&&", " and "), mode="eval").body
    def visit(node):
        if isinstance(node, ast.Constant) and isinstance(node.value, (int, float)):
            return node.value
        if isinstance(node, ast.BoolOp):
            return any(visit(value) for value in node.values) if isinstance(node.op, ast.Or) else all(visit(value) for value in node.values)
        if isinstance(node, ast.Compare) and len(node.ops) == 1:
            a, b = visit(node.left), visit(node.comparators[0])
            return {ast.GtE: a >= b, ast.Gt: a > b, ast.LtE: a <= b, ast.Lt: a < b, ast.Eq: a == b, ast.NotEq: a != b}[type(node.ops[0])]
        if isinstance(node, ast.BinOp):
            a, b = visit(node.left), visit(node.right)
            if isinstance(node.op, ast.Add): return a + b
            if isinstance(node.op, ast.Sub): return a - b
            if isinstance(node.op, ast.Mult): return a * b
            if isinstance(node.op, ast.Div): return a / b
        raise ValueError(f"unsupported numeric expression: {expression}")
    return int(visit(node))


@lru_cache(maxsize=1)
def load_data():
    source = preprocess('#include "src/data/pokemon/species_info.h"\n')
    form_source = preprocess('#include "src/data/pokemon/form_change_tables.h"\n')
    gigantamax_tables = {name for name, body in blocks(form_source, r"struct FormChange\s+(\w+)\s*\[\s*\]\s*=\s*\{") if "FORM_CHANGE_BATTLE_GIGANTAMAX" in body}
    species = {}
    for name, body in blocks(source, r"\[\s*(SPECIES_\w+)\s*\]\s*=\s*\{"):
        def field(key, default="0"):
            match = re.search(r"\." + key + r"\s*=\s*([^,\n]+)", body)
            return match.group(1).strip() if match else default
        species[name] = {"learnset": field("levelUpLearnset", "sNoneLevelUpLearnset"),
                         "abilities": re.findall(r"ABILITY_\w+", field("abilities")),
                         "gender": field("genderRatio"), "base_exp": number(field("expYield")),
                         "bst": sum(number(field(key)) for key in ("baseHP", "baseAttack", "baseDefense", "baseSpeed", "baseSpAttack", "baseSpDefense")),
                         "gigantamax": field("formChangeTable") in gigantamax_tables}
        ability_match = re.search(r"\.abilities\s*=\s*\{([^}]+)\}", body)
        if ability_match:
            species[name]["abilities"] = re.findall(r"ABILITY_\w+", ability_match.group(1))
    selection = "\n".join((f"#if P_LVL_UP_LEARNSETS == GEN_{generation}" if generation == 1 else f"#elif P_LVL_UP_LEARNSETS == GEN_{generation}") + f'\n#include "src/data/pokemon/level_up_learnsets/gen_{generation}.h"' for generation in range(1, 10)) + "\n#endif\n"
    normal = learnsets(preprocess(selection))
    legacy_source = (ROOT / "src/data/pokemon/level_up_learnsets_gen3.c").read_text()
    legacy_source = re.sub(r'^#include.*$', '', legacy_source, flags=re.M)
    legacy_source = preprocess('#include "src/data/pokemon/level_up_learnsets/gen_3.h"\n' + legacy_source)
    legacy = learnsets(legacy_source)
    legacy_map = dict(re.findall(r"\[\s*(SPECIES_\w+)\s*\]\s*=\s*(s\w+LevelUpLearnset)", legacy_source))
    moves, damaging = {}, set()
    move_source = preprocess('#include "src/data/moves_info.h"\n')
    for name, body in blocks(move_source, r"\[\s*(MOVE_\w+)\s*\]\s*=\s*\{"):
        pp = re.search(r"\.pp\s*=\s*(\d+)", body)
        moves[name] = int(pp.group(1)) if pp else 0
        power = re.search(r"\.power\s*=\s*(\d+)", body)
        if power and int(power.group(1)):
            damaging.add(name)
    moves["__damaging__"] = frozenset(damaging)
    schedules = {(name, mode): (legacy[legacy_map[name]] if mode == "legacy" and name in legacy_map else normal[data["learnset"]]) for name, data in species.items() for mode in ("normal", "legacy")}
    ids = wild.species_ids(wild.DEFAULT_SPECIES)
    canonical = {ids[name]: name for name in species if name in ids}
    for name, species_id in ids.items():
        if name not in species and species_id in canonical:
            original = canonical[species_id]
            species[name] = species[original]
            for mode in ("normal", "legacy"):
                schedules[name, mode] = schedules[original, mode]
    return species, schedules, moves


def initial_moves(schedule, level):
    moves = []
    for required, move in schedule:
        if required > level:
            break
        if required and move not in moves:
            moves.append(move)
            moves = moves[-4:]
    return moves


def build_audit(records, manifest):
    species_data, schedules, move_pp = load_data()
    policies = {row["id"]: row["policy"] for row in manifest["records"]}
    eligible_species = {slot["species"] for trainer, variants in records.items() if policies.get(trainer) in ("ORDINARY", "GYM_MEMBER") for row in variants.values() for slot in row["slots"]}
    metadata = wild.load_trainer_species_metadata(wild.DEFAULT_SPECIES_METADATA, wild.DEFAULT_SPECIES_INFO, wild.species_ids(wild.DEFAULT_SPECIES), eligible_species)
    graph = {row["species"]: row for row in metadata}
    exceptions = {(row["owner"], str(row["variant"]), row["slot"]): row for row in manifest.get("move_exceptions", [])}
    failures, slots, parties = [], [], []
    total = 0
    for trainer, variants in sorted(records.items()):
        policy = policies.get(trainer)
        if policy not in ("ORDINARY", "GYM_MEMBER"):
            continue
        variant_groups = {}
        for variant, roster in sorted(variants.items()):
            key = json.dumps(roster, sort_keys=True, separators=(",", ":"))
            # A reviewed exception can distinguish otherwise identical difficulty selections.
            key += json.dumps([row for row in manifest.get("move_exceptions", []) if row["owner"] == roster.get("owner", trainer) and row["variant"] == variant], sort_keys=True)
            variant_groups.setdefault(key, []).append(variant)
        for selectable_variants in variant_groups.values():
            variant = "DIFFICULTY_NORMAL" if "DIFFICULTY_NORMAL" in selectable_variants else selectable_variants[0]
            roster = variants[variant]
            projected_slots = []
            for index, slot in enumerate(roster["slots"]):
                authored = slot["species"]
                owner = roster.get("owner", trainer)
                row = {"id": trainer, "owner": owner, "variant": variant, "selectable_variants": selectable_variants, "slot": index, "authored": slot, "modes": {}}
                outcomes = {}
                for mode in ("normal", "legacy"):
                    intervals = []
                    for rating in range(81):
                        level = project(rating, slot["lvl"], policy)
                        species, _ = wild.effective_species(authored, level, graph)
                        data = species_data[species]
                        schedule = schedules[species, mode]
                        moves = initial_moves(schedule, level)
                        original_moves = [move for move in slot.get("moves", []) if move != "MOVE_NONE"]
                        retained = False
                        if (owner, str(variant), index) in exceptions and species == authored and original_moves and all(move in {m for required, m in schedule if required <= level} for move in original_moves):
                            moves, retained = original_moves, True
                        abilities = [ability for ability in data["abilities"] if ability != "ABILITY_NONE"]
                        ability = slot.get("ability", "ABILITY_NONE")
                        fallback = ability != "ABILITY_NONE" and ability not in abilities
                        gender = slot.get("gender", "TRAINER_MON_RANDOM_GENDER")
                        ratio = data["gender"]
                        gender_adjusted = ("FEMALE" in gender and ratio in ("MON_MALE", "0")) or ("MALE" in gender and "FEMALE" not in gender and ratio in ("MON_FEMALE", "254")) or (gender in ("TRAINER_MON_MALE", "TRAINER_MON_FEMALE") and ratio in ("MON_GENDERLESS", "255"))
                        changed = species != authored
                        enabled = lambda key: slot.get(key) not in (None, 0, "0", "FALSE", "TYPE_NONE")
                        gimmicks = []
                        if enabled("gigantamaxFactor") and not data.get("gigantamax", False):
                            gimmicks.append("gigantamaxFactor")
                        if enabled("shouldUseDynamax") and any(species.startswith("SPECIES_" + base) for base in ("ZACIAN", "ZAMAZENTA", "ETERNATUS")):
                            gimmicks.append("shouldUseDynamax")
                        held = slot.get("heldItem", "ITEM_NONE")
                        result = {"species": species, "moves": moves, "ability_fallback": fallback, "legal_abilities": abilities, "gender_adjustment": gender_adjusted,
                                  "gimmick_suppression": gimmicks, "held_item_review": held if changed and held != "ITEM_NONE" else None,
                                  "authored_moves_retained": retained, "custom_moves_replaced": bool(original_moves) and not retained,
                                  "above_soft_cap": level > interpolate(rating, CAPS), "base_exp": data["base_exp"],
                                  "utility_only_moves": not any(move in move_pp["__damaging__"] for move in moves) if "__damaging__" in move_pp else None,
                                  "high_bst_no_predecessor": data["bst"] >= 480 and graph[species]["predecessor"] == "SPECIES_NONE"}
                        if not abilities or not any(move_pp.get(move, 0) > 0 for move in moves):
                            failure = {"id": trainer, "variant": variant, "slot": index, "mode": mode, "rating": rating, "species": species, "level": level, "reason": "no legal ability" if not abilities else "empty usable moves"}
                            failures.append(failure)
                        outcomes[mode, rating] = {"level": level, **result}
                        if intervals and intervals[-1]["outcome"] == result:
                            intervals[-1]["rating_end"] = rating
                            intervals[-1]["level_end"] = level
                        else:
                            intervals.append({"rating_start": rating, "rating_end": rating, "level_start": level, "level_end": level, "outcome": result})
                        total += 1
                    row["modes"][mode] = intervals
                slots.append(row)
                projected_slots.append(outcomes)
            for rating in MILESTONES:
                parties.append({"id": trainer, "variant": variant, "selectable_variants": selectable_variants, "rating": rating, "party_size": roster["partySize"], "pool_size": roster.get("poolSize", 0),
                                "selection": "source order; pools list every candidate, runtime selects without replacement",
                                "money_inputs": {"null_party_fixed_reward": 20 if roster.get("money_party_null") else None, "authored_level": roster.get("money_level"), "trainer_class": roster.get("money_trainer_class", roster.get("trainerClass")), "single_multiplier": 4, "single_trainer_double_multiplier": 8, "two_opponents": "sum each opponent's single reward", "unknown_class_value": 5},
                                "modes": {mode: [outcomes[mode, rating] for outcomes in projected_slots] for mode in ("normal", "legacy")}})
    early = [row for row in parties if row["rating"] == 0]
    highest = sorted(early, key=lambda row: (-max((slot["level"] for slot in row["modes"]["normal"]), default=0), row["id"], str(row["variant"])))
    largest = sorted(early, key=lambda row: (-row["party_size"], row["id"], str(row["variant"])))
    regions = {row["id"]: row.get("region", "unknown") for row in manifest["records"]}
    representatives = set()
    for ordering in (early, highest, largest, list(reversed(highest))):
        covered = set()
        for row in ordering:
            key = regions[row["id"]], policies[row["id"]]
            if key not in covered:
                representatives.add((row["id"], row["variant"]))
                covered.add(key)
    projections, projection_ids = [], {}
    for row in slots:
        key = json.dumps(row["modes"], sort_keys=True, separators=(",", ":"))
        if key not in projection_ids:
            projection_ids[key] = len(projections)
            projections.append(row["modes"])
        row["projection"] = projection_ids[key]
        del row["modes"]
    outcome_rows, outcome_ids = [], {}
    for projection in projections:
        for intervals in projection.values():
            for interval in intervals:
                key = json.dumps(interval["outcome"], sort_keys=True, separators=(",", ":"))
                if key not in outcome_ids:
                    outcome_ids[key] = len(outcome_rows)
                    outcome_rows.append(interval["outcome"])
                interval["outcome"] = outcome_ids[key]
    concern_keys = ("ability_fallback", "gender_adjustment", "gimmick_suppression", "held_item_review", "custom_moves_replaced", "authored_moves_retained", "above_soft_cap", "high_bst_no_predecessor", "utility_only_moves")
    concern_counts = {key: 0 for key in concern_keys}
    changed_species = 0
    for row in slots:
        observed = [outcome_rows[interval["outcome"]] for intervals in projections[row["projection"]].values() for interval in intervals]
        for key in concern_keys:
            concern_counts[key] += any(outcome[key] for outcome in observed)
        changed_species += any(outcome["species"] != row["authored"]["species"] for outcome in observed)
    concern_counts["species_changes"] = changed_species
    def unique_trainers(rows):
        seen, output = set(), []
        for row in rows:
            if row["id"] not in seen:
                output.append(row)
                seen.add(row["id"])
            if len(output) == 10:
                break
        return output
    return {"structural_failures": failures, "evaluated_slot_ratings_modes": total, "baseline_anchors": ANCHORS, "slots": slots, "projections": projections, "outcomes": outcome_rows,
            "report_indexing": "Each slot's projection indexes projections; each interval's outcome indexes outcomes. Both tables use zero-based indices. Representative parties are expanded in full.",
            "slot_concern_counts": concern_counts,
            "representative_parties": [row for row in parties if (row["id"], row["variant"]) in representatives],
            "representative_selection": "For each region and policy: first ID, strongest early party, largest early party, and weakest early party. Every pool candidate is included; projections cover every other source slot.",
            "highest_early_parties": unique_trainers(highest),
            "largest_early_parties": unique_trainers(largest),
            "reward_policy": "Prize money reads the original encounter ID's party[partySize - 1].lvl without resolving overrides or pool selection; a NULL party yields 20. Class value defaults to 5, multiplied by level and 4 (single) or 8 (one-Trainer double); two opponents sum separate single rewards. Battle XP reads constructed species base EXP and effective level, then existing multipliers and player soft-cap reduction. Inputs are reported, not a predicted XP award.",
            "balance_status": "Structural validation does not establish playable balance; emulator playtests remain required."}
