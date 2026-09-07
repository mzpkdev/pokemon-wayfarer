#!/usr/bin/env python3
"""Generate and verify the checked-in Wayfarer mart catalog manifest.

The production data header is the source of truth.  This deliberately avoids a
second item catalog for the host audit: it parses the counted C lists and the
non-emitting map/binding macro, then derives every tier/challenge output.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


GAME_ROOT = Path(__file__).resolve().parents[2]
DATA_PATH = GAME_ROOT / "src/data/wayfarer_marts.h"
DEFAULT_OUTPUT = Path(__file__).with_name("catalog_manifest.json")
PROFILE_COUNT = 36
TIER_RATINGS = (0, 4, 16, 30, 40, 55)
WAYFARER_CONDITIONS = {
    "IS_WAYFARER": True,
    "IS_HNS": True,
    "IS_FRLG": False,
    "WAYFARER_TR_MARTS_ENABLED": True,
    "WAYFARER_LEAGUE_CIRCUIT_ENABLED": True,
    "OW_SHOW_ITEM_DESCRIPTIONS": 1,
    "OW_ITEM_DESCRIPTIONS_OFF": 0,
    # This audit does not need optional bugfix blocks, but it must balance
    # their preprocessor nesting while traversing active map scripts.
    "BUGFIX": False,
}


def block_after(text: str, marker: str) -> str:
    start = text.index(marker)
    end = text.index("};", start)
    return text[start:end]


def parse_arrays(text: str) -> dict[str, list[str]]:
    arrays: dict[str, list[str]] = {}
    for name, values in re.findall(r"static const u16 (s\w+)\[\] = \{([^}]*)\};", text):
        arrays[name] = re.findall(r"ITEM_[A-Z0-9_]+", values)
    return arrays


def parse_common_items(text: str) -> tuple[list[dict], list[dict]]:
    common = [
        {"minimum_tr": int(rating), "category": category, "item": item}
        for rating, category, item in re.findall(
            r"\{\s*(\d+),\s*(MART_COMMON_[A-Z_]+),\s*(ITEM_[A-Z0-9_]+)\s*\}",
            block_after(text, "sWayfarerMartCommonItems"),
        )
    ]
    pp = [
        {"minimum_tr": int(rating), "item": item}
        for rating, item in re.findall(
            r"\{\s*(\d+),\s*(ITEM_[A-Z0-9_]+)\s*\}",
            block_after(text, "sWayfarerMartPpItems"),
        )
    ]
    return common, pp


def parse_profiles(text: str) -> dict[str, dict]:
    profiles: dict[str, dict] = {}
    table = block_after(text, "sWayfarerMartProfiles")
    for profile_id, macro, arguments in re.findall(
        r"\[(MART_PROFILE_[A-Z0-9_]+)\]\s*=\s*(MART_PROFILE_[A-Z_]+)\(([^)]*)\)", table
    ):
        args = [value.strip() for value in arguments.split(",")]
        if macro == "MART_PROFILE_WITH_RETAINED":
            signature, retained, common_mask, pp_recovery, category = args
        elif macro == "MART_PROFILE_NO_RETAINED":
            signature, common_mask, pp_recovery, category = args
            retained = None
        elif macro == "MART_PROFILE_FACILITY":
            retained, category = args
            signature = None
            common_mask = "MART_COMMON_ALL"
            pp_recovery = "TRUE"
        elif macro == "MART_PROFILE_EMPTY_FACILITY":
            (category,) = args
            signature = None
            retained = None
            common_mask = "MART_COMMON_ALL"
            pp_recovery = "TRUE"
        else:
            raise ValueError(f"unsupported profile macro {macro}")
        profiles[profile_id] = {
            "signature_array": signature,
            "retained_array": retained,
            "common_category_mask": common_mask,
            "supports_pp_recovery": pp_recovery == "TRUE",
            "category_mask": category,
        }
    return profiles


def parse_bindings(text: str) -> dict[str, dict]:
    macro_start = text.index("#define WAYFARER_MART_PROFILE_BINDINGS")
    macro_end = text.index("#undef MART_PROFILE_WITH_RETAINED", macro_start)
    macro = text[macro_start:macro_end]
    rows = {}
    pattern = re.compile(
        r'X\((MART_PROFILE_[A-Z0-9_]+), "([^"]+)", (MAP_[A-Z0-9_]+), "([^"]+)", (MART_CATEGORY_[A-Z]+), "([^"]+)"\)'
    )
    for profile_id, map_name, map_id, script, category, retained_source in pattern.findall(macro):
        if profile_id in rows:
            raise ValueError(f"duplicate manifest binding for {profile_id}")
        rows[profile_id] = {
            "map": map_name,
            "compiled_map": f"MAP_GROUP({map_id}), MAP_NUM({map_id})",
            "script_binding": script,
            "category_mask": category,
            "retained_source": retained_source,
        }
    return rows


def parse_npc_bindings(binding: dict) -> list[dict]:
    """Read the actual map event instead of trusting a source-label convention."""
    map_path = GAME_ROOT / "data/maps" / binding["map"] / "map.json"
    map_data = json.loads(map_path.read_text())
    if map_data["id"] != binding["compiled_map"].split("(")[1].split(")")[0]:
        raise ValueError(f"compiled map mismatch for {binding['map']}")
    matches = [
        {
            "local_id": event.get("local_id"),
            "x": event["x"],
            "y": event["y"],
            "elevation": event["elevation"],
            "visibility_flag": event["flag"],
        }
        for event in map_data.get("object_events", [])
        if event.get("script") == binding["script_binding"]
    ]
    if len(matches) != 1:
        raise ValueError(f"expected one map NPC for {binding['profile_id']}, found {len(matches)}")
    return matches


def is_active_wayfarer_condition(expression: str) -> bool:
    """Evaluate the small conditional language used by event-script sources."""
    normalized = expression
    for name, value in WAYFARER_CONDITIONS.items():
        normalized = re.sub(rf"\b{name}\b", "1" if value else "0", normalized)
    normalized = normalized.replace("&&", " and ").replace("||", " or ")
    normalized = re.sub(r"!(?!=)", " not ", normalized).strip()
    if not re.fullmatch(r"[01 ()andornot!=]+", normalized):
        raise ValueError(f"unsupported event-script condition: {expression}")
    return bool(eval(normalized, {"__builtins__": {}}, {}))


def active_wayfarer_lines(text: str) -> list[tuple[int, str]]:
    """Apply #if/.if blocks as the Wayfarer event-script build does."""
    lines = []
    stack: list[tuple[bool, bool]] = []
    active = True
    for line_number, line in enumerate(text.splitlines(), start=1):
        defined = re.match(r"^\s*#(ifdef|ifndef)\s+([A-Za-z0-9_]+)\s*$", line)
        if defined:
            name = defined.group(2)
            branch = WAYFARER_CONDITIONS.get(name, False)
            if defined.group(1) == "ifndef":
                branch = not branch
            stack.append((active, branch))
            active = active and branch
            continue
        condition = re.match(r"^\s*(?:#if|\.if)\s+(.+?)\s*$", line)
        if condition:
            branch = is_active_wayfarer_condition(condition.group(1))
            stack.append((active, branch))
            active = active and branch
            continue
        elif_condition = re.match(r"^\s*(?:#elif|\.elseif)\s+(.+?)\s*$", line)
        if elif_condition:
            if not stack:
                raise ValueError("orphan event-script #elif/.elseif")
            parent, prior_branch = stack[-1]
            branch = is_active_wayfarer_condition(elif_condition.group(1))
            active = parent and not prior_branch and branch
            stack[-1] = (parent, prior_branch or branch)
            continue
        if re.match(r"^\s*(?:#else|\.else)\b", line):
            if not stack:
                raise ValueError("orphan event-script #else/.else")
            parent, previous_branch = stack[-1]
            active = parent and not previous_branch
            stack[-1] = (parent, True)
            continue
        if re.match(r"^\s*(?:#endif|\.endif)\b", line):
            if not stack:
                raise ValueError("orphan event-script #endif/.endif")
            active, _ = stack.pop()
            continue
        if active:
            lines.append((line_number, line))
    if stack:
        raise ValueError("unterminated event-script conditional")
    return lines


def active_wayfarer_map_names() -> set[str]:
    source = (GAME_ROOT / "data/event_scripts.s").read_text()
    return {
        match.group(1)
        for _, line in active_wayfarer_lines(source)
        if (match := re.match(r'^\s*\.include "data/maps/([^/]+)/scripts\.inc"$', line))
    }


def reachable_mart_commands(map_name: str) -> list[dict]:
    """Find active pokemart opcodes reached from a map's object-event scripts."""
    map_path = GAME_ROOT / "data/maps" / map_name / "map.json"
    map_data = json.loads(map_path.read_text())
    script_path = map_path.with_name("scripts.inc")
    sections: dict[str, list[tuple[int, str]]] = {}
    current_label = None
    for line_number, line in active_wayfarer_lines(script_path.read_text()):
        label = re.match(r"^([A-Za-z0-9_]+)::?\s*$", line)
        if label:
            current_label = label.group(1)
            sections.setdefault(current_label, [])
        elif current_label is not None:
            sections[current_label].append((line_number, line))

    roots = {
        event.get("script")
        for event in map_data.get("object_events", [])
        if event.get("script") in sections
    }
    reachable = set(roots)
    pending = list(roots)
    branch_re = re.compile(r"^\s*(?:goto|call)(?:_if_[A-Za-z0-9_]+)?\s+(?:[^,]+,\s*)*([A-Za-z0-9_]+)\s*$")
    while pending:
        label = pending.pop()
        for _, line in sections[label]:
            branch = branch_re.match(line)
            if branch and branch.group(1) in sections and branch.group(1) not in reachable:
                reachable.add(branch.group(1))
                pending.append(branch.group(1))

    commands = []
    command_index = 0
    for label, lines in sections.items():
        if label not in reachable:
            continue
        for line_number, line in lines:
            if re.match(r"^\s*pokemart(?:\s|$)", line):
                command_index += 1
                commands.append({
                    "script_label": label,
                    "command_index": command_index,
                    "line": line_number,
                    "root_event": label in roots,
                })
    return commands


def classify_mart_sources(profiles: list[dict]) -> list[dict]:
    """Record every authored ``pokemart`` call and every profile-only counter.

    A map can contain several unrelated cash counters (for example, the Safari
    Zone gate), so map-level classification is not sufficient evidence that a
    specialist was intentionally left alone.  Script label plus command index
    makes each call site addressable without trying to infer an NPC from a
    data-list label.
    """
    converted_bindings = {
        (profile["map"], profile["script_binding"]): profile["profile_id"]
        for profile in profiles
    }
    shared_labels = {
        "Cherrygrove_Pokemart_EventScript_Clerk",
        "VioletCity_Mart_EventScript_Clerk",
        "BattleFrontier_Mart_EventScript_Clerk",
        "BattleFrontier_Mart_EventScript_Clerk_hns",
    }
    rows = []
    converted_entries = set()
    profile_maps = {profile["map"] for profile in profiles}
    for map_name in sorted(active_wayfarer_map_names()):
        for command in reachable_mart_commands(map_name):
            binding = (map_name, command["script_label"])
            if binding in converted_bindings:
                classification = "converted"
                reason = "Approved global-TR profile; its NPC binding is recorded on the profile row."
                converted_entries.add(binding)
            elif map_name in profile_maps and not command["root_event"]:
                classification = "excluded"
                reason = "Reachable only through a converted counter's legacy/sentinel fallback; no active profile map enters it."
            else:
                classification = "preserved"
                reason = "Separate legacy specialist, standalone, alternate, or outlying cash-mart call; no approved profile is bound to this script entry."
            rows.append({
                "map": map_name,
                **command,
                "classification": classification,
                "reason": reason,
            })

    # Several converted town counters call one of the shared entry points in a
    # different map's source file, while Mahogany has no pokemart opcode at all.
    # They still need a counter-level record linked to their actual map event.
    for profile in profiles:
        binding = (profile["map"], profile["script_binding"])
        if binding in converted_entries:
            continue
        if profile["script_binding"] in shared_labels:
            reason = "Approved global-TR profile reached through a shared clerk entry point."
        else:
            reason = "Approved global-TR profile opened directly by this map event."
        rows.append({
            "map": profile["map"],
            "script_label": profile["script_binding"],
            "command_index": None,
            "line": None,
            "classification": "converted",
            "reason": reason,
        })
    return rows


def includes_mask(mask: str, category: str) -> bool:
    return mask == "MART_COMMON_ALL" or category in mask.split(" | ")


def append_unique(output: list[str], item: str) -> None:
    if item not in output:
        output.append(item)


def resolve_profile(profile: dict, arrays: dict[str, list[str]], common: list[dict], pp: list[dict], rating: int, challenge: bool) -> list[str]:
    output: list[str] = []
    for entry in common:
        if rating >= entry["minimum_tr"] and includes_mask(profile["common_category_mask"], entry["category"]):
            append_unique(output, entry["item"])
    if challenge and profile["supports_pp_recovery"]:
        for entry in pp:
            if rating >= entry["minimum_tr"]:
                append_unique(output, entry["item"])
    for array_name in (profile["signature_array"], profile["retained_array"]):
        if array_name:
            for item in arrays[array_name]:
                append_unique(output, item)
    return output


def build_manifest(text: str) -> dict:
    arrays = parse_arrays(text)
    common, pp = parse_common_items(text)
    profiles = parse_profiles(text)
    bindings = parse_bindings(text)

    if len(profiles) != PROFILE_COUNT - 1:
        raise ValueError(f"expected {PROFILE_COUNT - 1} real profiles, found {len(profiles)}")
    if set(profiles) != set(bindings):
        missing = sorted(set(profiles) ^ set(bindings))
        raise ValueError(f"profile/binding mismatch: {', '.join(missing)}")

    output_profiles = []
    for profile_id, profile in profiles.items():
        binding = bindings[profile_id]
        binding["profile_id"] = profile_id
        if profile["category_mask"] != binding["category_mask"]:
            raise ValueError(f"category mismatch for {profile_id}")
        signature = arrays.get(profile["signature_array"], []) if profile["signature_array"] else []
        retained = arrays.get(profile["retained_array"], []) if profile["retained_array"] else []
        tiers = {
            str(rating): {
                "normal": resolve_profile(profile, arrays, common, pp, rating, False),
                "challenge": resolve_profile(profile, arrays, common, pp, rating, True),
            }
            for rating in TIER_RATINGS
        }
        output_profiles.append({
            "profile_id": profile_id,
            **binding,
            "classification": "converted",
            "npc_bindings": parse_npc_bindings(binding),
            "supports_pp_recovery": profile["supports_pp_recovery"],
            "common_category_mask": profile["common_category_mask"],
            "signature_ids": signature,
            "retained_source_lists": [{"label": binding["retained_source"], "ids": retained}],
            "outputs": tiers,
        })

    classifications = classify_mart_sources(output_profiles)
    if len({(row["map"], row["script_label"], row["command_index"]) for row in classifications}) != len(classifications):
        raise ValueError("duplicate mart-source classification")
    return {
        "generated_from": "game/src/data/wayfarer_marts.h",
        "profile_count_including_none": PROFILE_COUNT,
        "common_items_in_display_order": common,
        "challenge_pp_items_in_display_order": pp,
        "profiles": output_profiles,
        "counter_classifications": classifications,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    manifest = json.dumps(build_manifest(DATA_PATH.read_text()), indent=2, sort_keys=False) + "\n"
    if args.check:
        if not args.output.exists() or args.output.read_text() != manifest:
            print(f"outdated mart manifest: {args.output}", file=sys.stderr)
            return 1
        return 0
    args.output.write_text(manifest)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
