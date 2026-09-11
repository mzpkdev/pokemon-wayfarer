#!/usr/bin/env python3
"""Host audit for the checked-in Wayfarer mart profile source of truth."""

from __future__ import annotations

import sys
from pathlib import Path

import generate_manifest


COMMON = (
    (0, "MART_COMMON_BALLS", "ITEM_POKE_BALL"),
    (4, "MART_COMMON_BALLS", "ITEM_GREAT_BALL"),
    (30, "MART_COMMON_BALLS", "ITEM_ULTRA_BALL"),
    (0, "MART_COMMON_HP_MEDICINE", "ITEM_POTION"),
    (4, "MART_COMMON_HP_MEDICINE", "ITEM_SUPER_POTION"),
    (30, "MART_COMMON_HP_MEDICINE", "ITEM_HYPER_POTION"),
    (55, "MART_COMMON_HP_MEDICINE", "ITEM_MAX_POTION"),
    (55, "MART_COMMON_HP_MEDICINE", "ITEM_FULL_RESTORE"),
    (0, "MART_COMMON_STATUS_CURE", "ITEM_ANTIDOTE"),
    (0, "MART_COMMON_STATUS_CURE", "ITEM_PARALYZE_HEAL"),
    (0, "MART_COMMON_STATUS_CURE", "ITEM_AWAKENING"),
    (0, "MART_COMMON_STATUS_CURE", "ITEM_BURN_HEAL"),
    (0, "MART_COMMON_STATUS_CURE", "ITEM_ICE_HEAL"),
    (40, "MART_COMMON_STATUS_CURE", "ITEM_FULL_HEAL"),
    (30, "MART_COMMON_REVIVE", "ITEM_REVIVE"),
    (0, "MART_COMMON_REPELS", "ITEM_REPEL"),
    (16, "MART_COMMON_REPELS", "ITEM_SUPER_REPEL"),
    (40, "MART_COMMON_REPELS", "ITEM_MAX_REPEL"),
    (0, "MART_COMMON_ESCAPE_ROPE", "ITEM_ESCAPE_ROPE"),
)
PP = ((0, "ITEM_ETHER"), (16, "ITEM_ELIXIR"), (30, "ITEM_MAX_ETHER"), (55, "ITEM_MAX_ELIXIR"))

SIGNATURES = {
    "MART_PROFILE_CHERRYGROVE": ("ITEM_HEAL_BALL", "ITEM_POKE_DOLL"),
    "MART_PROFILE_VIOLET": ("ITEM_NEST_BALL", "ITEM_X_ACCURACY"),
    "MART_PROFILE_AZALEA": ("ITEM_NET_BALL", "ITEM_NEST_BALL", "ITEM_WOOD_MAIL"),
    "MART_PROFILE_GOLDENROD_2F": ("ITEM_FIRE_STONE", "ITEM_WATER_STONE", "ITEM_THUNDER_STONE", "ITEM_LUXURY_BALL"),
    "MART_PROFILE_ECRUTEAK": ("ITEM_DUSK_BALL", "ITEM_POKE_DOLL", "ITEM_RETRO_MAIL"),
    "MART_PROFILE_OLIVINE": ("ITEM_NET_BALL", "ITEM_HEAL_BALL", "ITEM_HARBOR_MAIL"),
    "MART_PROFILE_BLACKTHORN": ("ITEM_DUSK_BALL", "ITEM_TIMER_BALL", "ITEM_GUARD_SPEC"),
    "MART_PROFILE_MAHOGANY": ("ITEM_NET_BALL", "ITEM_DUSK_BALL"),
    "MART_PROFILE_VIRIDIAN": ("ITEM_NEST_BALL", "ITEM_POKE_DOLL", "ITEM_ORANGE_MAIL"),
    "MART_PROFILE_PEWTER": ("ITEM_DUSK_BALL", "ITEM_X_DEFENSE"),
    "MART_PROFILE_CERULEAN": ("ITEM_NET_BALL", "ITEM_HEAL_BALL", "ITEM_X_SPEED"),
    "MART_PROFILE_VERMILION": ("ITEM_NET_BALL", "ITEM_DIVE_BALL", "ITEM_HARBOR_MAIL"),
    "MART_PROFILE_LAVENDER": ("ITEM_DUSK_BALL", "ITEM_HEAL_BALL", "ITEM_SHADOW_MAIL"),
    "MART_PROFILE_CELADON_2F": ("ITEM_LUXURY_BALL", "ITEM_POKE_DOLL", "ITEM_RETRO_MAIL"),
    "MART_PROFILE_SAFFRON": ("ITEM_X_SP_ATK", "ITEM_X_SP_DEF", "ITEM_GUARD_SPEC"),
    "MART_PROFILE_FUCHSIA": ("ITEM_NET_BALL", "ITEM_NEST_BALL", "ITEM_FLUFFY_TAIL"),
    "MART_PROFILE_OLDALE": ("ITEM_HEAL_BALL", "ITEM_NEST_BALL"),
    "MART_PROFILE_PETALBURG": ("ITEM_NEST_BALL", "ITEM_X_DEFENSE", "ITEM_ORANGE_MAIL"),
    "MART_PROFILE_RUSTBORO": ("ITEM_TIMER_BALL", "ITEM_REPEAT_BALL"),
    "MART_PROFILE_SLATEPORT": ("ITEM_NET_BALL", "ITEM_DIVE_BALL", "ITEM_HARBOR_MAIL", "ITEM_LUXURY_BALL"),
    "MART_PROFILE_MAUVILLE": ("ITEM_X_SPEED", "ITEM_X_SP_ATK", "ITEM_X_ACCURACY", "ITEM_MECH_MAIL"),
    "MART_PROFILE_VERDANTURF": ("ITEM_NEST_BALL", "ITEM_FLUFFY_TAIL"),
    "MART_PROFILE_FALLARBOR": ("ITEM_DUSK_BALL", "ITEM_DIRE_HIT", "ITEM_X_DEFENSE"),
    "MART_PROFILE_LAVARIDGE": ("ITEM_HEAL_BALL", "ITEM_GUARD_SPEC", "ITEM_X_SP_DEF"),
    "MART_PROFILE_FORTREE": ("ITEM_NEST_BALL", "ITEM_NET_BALL", "ITEM_WOOD_MAIL", "ITEM_X_SPEED"),
    "MART_PROFILE_MOSSDEEP": ("ITEM_NET_BALL", "ITEM_DIVE_BALL"),
    "MART_PROFILE_SOOTOPOLIS": ("ITEM_DIVE_BALL", "ITEM_DUSK_BALL", "ITEM_SHADOW_MAIL"),
    "MART_PROFILE_LILYCOVE_2F_LEFT": ("ITEM_LUXURY_BALL", "ITEM_FLUFFY_TAIL"),
    "MART_PROFILE_LILYCOVE_2F_RIGHT": ("ITEM_WAVE_MAIL", "ITEM_MECH_MAIL"),
}

RETAINED = {
    "MART_PROFILE_GOLDENROD_2F": ("ITEM_FIRE_STONE", "ITEM_WATER_STONE", "ITEM_THUNDER_STONE"),
    "MART_PROFILE_PETALBURG": ("ITEM_X_SPEED", "ITEM_X_ATTACK", "ITEM_X_DEFENSE", "ITEM_ORANGE_MAIL"),
    "MART_PROFILE_RUSTBORO": ("ITEM_X_SPEED", "ITEM_X_ATTACK", "ITEM_X_DEFENSE", "ITEM_TIMER_BALL", "ITEM_REPEAT_BALL"),
    "MART_PROFILE_SLATEPORT": ("ITEM_HARBOR_MAIL",),
    "MART_PROFILE_MAUVILLE": ("ITEM_X_SPEED", "ITEM_X_ATTACK", "ITEM_X_DEFENSE", "ITEM_GUARD_SPEC", "ITEM_DIRE_HIT", "ITEM_X_ACCURACY"),
    "MART_PROFILE_VERDANTURF": ("ITEM_NEST_BALL", "ITEM_X_SP_ATK", "ITEM_FLUFFY_TAIL"),
    "MART_PROFILE_FALLARBOR": ("ITEM_X_SP_ATK", "ITEM_X_SPEED", "ITEM_X_ATTACK", "ITEM_X_DEFENSE", "ITEM_DIRE_HIT", "ITEM_GUARD_SPEC"),
    "MART_PROFILE_LAVARIDGE": ("ITEM_X_SPEED",),
    "MART_PROFILE_FORTREE": ("ITEM_WOOD_MAIL",),
    "MART_PROFILE_MOSSDEEP": ("ITEM_NET_BALL", "ITEM_DIVE_BALL", "ITEM_X_ATTACK", "ITEM_X_DEFENSE"),
    "MART_PROFILE_SOOTOPOLIS": ("ITEM_X_ATTACK", "ITEM_X_DEFENSE", "ITEM_SHADOW_MAIL"),
    "MART_PROFILE_LILYCOVE_2F_LEFT": ("ITEM_FLUFFY_TAIL",),
    "MART_PROFILE_LILYCOVE_2F_RIGHT": ("ITEM_WAVE_MAIL", "ITEM_MECH_MAIL"),
    "MART_PROFILE_INDIGO_PLATEAU": ("ITEM_PROTEIN", "ITEM_CALCIUM", "ITEM_IRON", "ITEM_ZINC", "ITEM_CARBOS", "ITEM_HP_UP"),
    "MART_PROFILE_BATTLE_FRONTIER_HNS": ("ITEM_PROTEIN", "ITEM_CALCIUM", "ITEM_IRON", "ITEM_ZINC", "ITEM_CARBOS", "ITEM_HP_UP"),
    "MART_PROFILE_BATTLE_FRONTIER": ("ITEM_PROTEIN", "ITEM_CALCIUM", "ITEM_IRON", "ITEM_ZINC", "ITEM_CARBOS", "ITEM_HP_UP"),
    "MART_PROFILE_TRAINER_HILL": ("ITEM_X_SPEED", "ITEM_X_SP_ATK", "ITEM_X_ATTACK", "ITEM_X_DEFENSE", "ITEM_DIRE_HIT", "ITEM_GUARD_SPEC", "ITEM_X_ACCURACY"),
}

LEFT_MASK = "MART_COMMON_BALLS | MART_COMMON_STATUS_CURE | MART_COMMON_ESCAPE_ROPE"
RIGHT_MASK = "MART_COMMON_HP_MEDICINE | MART_COMMON_REVIVE | MART_COMMON_REPELS"
GAME_ROOT = Path(__file__).resolve().parents[2]
GUARD = "#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED"


def append_unique(items: list[str], item: str) -> None:
    if item not in items:
        items.append(item)


def expected_output(profile_id: str, rating: int, challenge: bool) -> list[str]:
    mask = LEFT_MASK if profile_id == "MART_PROFILE_LILYCOVE_2F_LEFT" else RIGHT_MASK if profile_id == "MART_PROFILE_LILYCOVE_2F_RIGHT" else "MART_COMMON_ALL"
    supports_pp = profile_id != "MART_PROFILE_LILYCOVE_2F_LEFT"
    output: list[str] = []
    for minimum, category, item in COMMON:
        if rating >= minimum and generate_manifest.includes_mask(mask, category):
            append_unique(output, item)
    if challenge and supports_pp:
        for minimum, item in PP:
            if rating >= minimum:
                append_unique(output, item)
    for item in SIGNATURES.get(profile_id, ()) + RETAINED.get(profile_id, ()):
        append_unique(output, item)
    return output


def fail(message: str) -> None:
    raise AssertionError(message)


def main() -> int:
    data = generate_manifest.DATA_PATH.read_text()
    manifest = generate_manifest.build_manifest(data)
    profiles = {profile["profile_id"]: profile for profile in manifest["profiles"]}
    classifications = manifest["counter_classifications"]

    if tuple((entry["minimum_tr"], entry["category"], entry["item"]) for entry in manifest["common_items_in_display_order"]) != COMMON:
        fail("common catalog or display order differs from approved tiers")
    if tuple((entry["minimum_tr"], entry["item"]) for entry in manifest["challenge_pp_items_in_display_order"]) != PP:
        fail("challenge PP thresholds differ from approved tiers")
    if len(profiles) != 35 or set(SIGNATURES) - set(profiles):
        fail("conversion registry is incomplete")
    if not classifications or any(not row["reason"] for row in classifications):
        fail("every authored mart source needs a preserved/excluded/converted classification")
    classification_keys = {(row["map"], row["script_label"], row["command_index"]) for row in classifications}
    if len(classification_keys) != len(classifications):
        fail("every authored mart command needs one distinct classification row")
    active_maps = generate_manifest.active_wayfarer_map_names()
    if any(row["map"] not in active_maps for row in classifications):
        fail("inactive build-variant mart sources must not appear in the Wayfarer audit")
    if any("Legacy" in row["script_label"] and row["classification"] == "preserved" for row in classifications):
        fail("converted-clerk legacy fallbacks must be excluded, not preserved")

    seen_npcs = set()
    for profile in profiles.values():
        if profile["classification"] != "converted" or len(profile["npc_bindings"]) != 1:
            fail(f"missing converted map/NPC classification for {profile['profile_id']}")
        npc = profile["npc_bindings"][0]
        if npc["visibility_flag"] is None:
            fail(f"incomplete NPC binding for {profile['profile_id']}")
        key = (profile["map"], npc["local_id"] or profile["script_binding"], npc["x"], npc["y"])
        if key in seen_npcs:
            fail(f"duplicate converted NPC binding: {key}")
        seen_npcs.add(key)
        if not any(row["map"] == profile["map"] and row["classification"] == "converted" for row in classifications):
            fail(f"converted profile has no converted source classification: {profile['profile_id']}")
    mahogany_npc = profiles["MART_PROFILE_MAHOGANY"]["npc_bindings"][0]
    if mahogany_npc != {
        "local_id": "LOCALID_MAHOGANY_MERCHANT",
        "x": 30,
        "y": 11,
        "elevation": 0,
        "visibility_flag": "0",
    }:
        fail("Mahogany Supplies must bind the permanent exterior merchant at (30,11)")

    town_signatures = []
    for profile_id, profile in profiles.items():
        signature = tuple(profile["signature_ids"])
        retained = tuple(profile["retained_source_lists"][0]["ids"])
        if signature != SIGNATURES.get(profile_id, ()):
            fail(f"signature drift for {profile_id}: {signature}")
        if retained != RETAINED.get(profile_id, ()):
            fail(f"retained stock drift for {profile_id}: {retained}")
        if profile_id in SIGNATURES:
            if not 2 <= len(signature) <= 4:
                fail(f"signature length outside 2..4 for {profile_id}")
            town_signatures.append(signature)
        elif signature:
            fail(f"facility has a new signature: {profile_id}")

        for rating in range(81):
            for challenge in (False, True):
                expected = expected_output(profile_id, rating, challenge)
                resolved = generate_manifest.resolve_profile(
                    {
                        "signature_array": None,
                        "retained_array": None,
                        "common_category_mask": profile["common_category_mask"],
                        "supports_pp_recovery": profile["supports_pp_recovery"],
                    },
                    {},
                    manifest["common_items_in_display_order"],
                    manifest["challenge_pp_items_in_display_order"],
                    rating,
                    challenge,
                )
                # Use the independent expected local data after checking the
                # resolver's common prefix. This is the same source-level
                # assembly order used by the production C resolver.
                for item in signature + retained:
                    append_unique(resolved, item)
                if resolved != expected:
                    fail(f"catalog drift for {profile_id}, TR {rating}, challenge={challenge}")
                if len(resolved) != len(set(resolved)) or len(resolved) > 63:
                    fail(f"duplicate or oversized catalog for {profile_id}")

    if len(town_signatures) != 29 or len(set(town_signatures)) != 29:
        fail("town signatures must be 29 pairwise-distinct 2..4 item sets")
    if max(len(expected_output(profile_id, 55, True)) for profile_id in profiles) != 31:
        fail("largest catalog must remain 31 items")

    # Existing entry points, not map JSON wrappers, are the dispatch contract.
    shared = {
        "CherrygroveCity_Mart_hns": "MART_CLERK_FAMILY_CHERRYGROVE",
        "VioletCity_Mart_hns": "MART_CLERK_FAMILY_VIOLET",
        "BattleFrontier_Mart": "MART_CLERK_FAMILY_FRONTIER",
        "BattleFrontier_Mart_hns": "MART_CLERK_FAMILY_FRONTIER",
    }
    for map_name, family in shared.items():
        source = (GAME_ROOT / "data/maps" / map_name / "scripts.inc").read_text()
        if GUARD not in source or family not in source or "specialvar VAR_0x8004, WayfarerLookupMartProfileForSharedClerk" not in source:
            fail(f"shared clerk dispatch is incomplete for {map_name}")
        if "MART_PROFILE_NONE" not in source or "waitstate" not in source:
            fail(f"shared clerk fallback/resume is incomplete for {map_name}")
    shared_labels = {
        "Cherrygrove_Pokemart_EventScript_Clerk",
        "VioletCity_Mart_EventScript_Clerk",
        "BattleFrontier_Mart_EventScript_Clerk",
        "BattleFrontier_Mart_EventScript_Clerk_hns",
    }
    for profile_id, profile in profiles.items():
        map_name = profile["map"]
        if profile["script_binding"] in shared_labels:
            continue
        source = (GAME_ROOT / "data/maps" / map_name / "scripts.inc").read_text()
        if GUARD not in source or f"setvar VAR_0x8004, {profile_id}" not in source:
            fail(f"direct profile branch is incomplete for {profile_id}")
        if "special WayfarerOpenMartProfile" not in source or "waitstate" not in source:
            fail(f"direct profile opener/resume is incomplete for {profile_id}")
    mahogany = (GAME_ROOT / "data/maps/Mahoganytown_hns/scripts.inc").read_text()
    for token in ("MULTI_MAHOGANY_MERCHANT", "MerchantSupplies", "MerchantRageCandyBar", "MerchantExit", "goto MahoganyTown_EventScript_MerchantMenu"):
        if token not in mahogany:
            fail(f"Mahogany Supplies menu is missing {token}")
    for map_name, gate in {
        "CherrygroveCity_Mart_hns": "VAR_NEWBARK_TOWN_STATE",
        "OldaleTown_Mart": "FLAG_ADVENTURE_STARTED",
        "PetalburgCity_Mart": "FLAG_PETALBURG_MART_EXPANDED_ITEMS",
        "RustboroCity_Mart": "FLAG_MET_DEVON_EMPLOYEE",
        "TrainerHill_Entrance": "FLAG_SYS_GAME_CLEAR",
    }.items():
        source = (GAME_ROOT / "data/maps" / map_name / "scripts.inc").read_text()
        if GUARD not in source or "#else" not in source or gate not in source:
            fail(f"guarded legacy stock gate is incomplete for {map_name}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"mart catalog audit failed: {error}", file=sys.stderr)
        raise SystemExit(1)
