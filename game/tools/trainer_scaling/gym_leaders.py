#!/usr/bin/env python3
"""Generate and validate the authored Wayfarer Gym Leader scaling inventory.

The compact source table below deliberately names every retained member by its
reviewed party-source owner and source slot.  The party files remain the
authority for species, forms, moves, items, IVs, EVs, abilities, and all other
TrainerMon data; this table is the authority for the six-member roster,
retention order, battle order, ace designation, and level policy.
"""
from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
import sys

import generate as trainer_inventory


ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "src/data/trainer_scaling"
HEADER = OUTPUT / "gym_leaders.h"
REPORT = OUTPUT / "gym_leaders.md"
INVENTORY = OUTPUT / "gym_leaders.json"

PARTY_SIZE = 6
LEADER_ANCHORS = ((0, 15), (4, 16), (8, 18), (16, 23), (30, 30),
                  (40, 42), (55, 60), (65, 80), (80, 100))
SIZE_THRESHOLDS = ((8, 2), (22, 3), (34, 4), (40, 5), (81, 6))
AUTHORED = "AUTHORED"
LEVEL_UP = "LEVEL_UP"


class ValidationError(ValueError):
    pass


def member(owner, slot, battle_order, *, ace=False, offset=-2, policy=AUTHORED):
    """One explicit retained source member; owner/slot never means output index."""
    return dict(owner=owner, slot=slot, battleOrder=battle_order, isAce=ace,
                levelOffset=0 if ace else offset, movePolicy=policy)


def roster(identity, region, trainer, script, members, *, double=False,
           selection="DIFFICULTY_NORMAL", aliases=(), runtime_overrides=()):
    return dict(identity=identity, region=region, trainer=trainer,
                owner=trainer, difficulty=selection, script=script,
                members=members, isDoubleBattle=double,
                aliases=list(aliases), runtimeOverrides=list(runtime_overrides))


# The 24 named identities have 30 resolved initial-battle source variants.
# Each tuple is (identity, active Trainer ID, source-slot index) followed by
# explicit battle metadata.  All current authored custom moves are retained;
# no slot relies on a generated level-up tuple in this first roster review.
ROSTERS = (
    roster("Brock", "Kanto", "TRAINER_BROCK_HNS", "data/maps/PewterCity_Gym_hns/scripts.inc", (
        member("TRAINER_BROCK_HNS", 0, 5, ace=True), member("TRAINER_BROCK_HNS", 5, 4),
        member("TRAINER_BROCK_HNS", 1, 0), member("TRAINER_BROCK_HNS", 2, 1),
        member("TRAINER_BROCK_HNS", 3, 2), member("TRAINER_BROCK_HNS", 4, 3)),
        aliases=("TRAINER_BROCK_DOJO_HNS",)),
    roster("Misty", "Kanto", "TRAINER_MISTY_HNS", "data/maps/CeruleanCity_Gym_hns/scripts.inc", (
        member("TRAINER_MISTY_HNS", 5, 5, ace=True), member("TRAINER_MISTY_HNS", 4, 4),
        member("TRAINER_MISTY_HNS", 0, 0), member("TRAINER_MISTY_HNS", 1, 1),
        member("TRAINER_MISTY_HNS", 2, 2), member("TRAINER_MISTY_HNS", 3, 3)),
        aliases=("TRAINER_MISTY_DOJO_HNS",)),
    roster("Lt. Surge", "Kanto", "TRAINER_LTSURGE_HNS", "data/maps/VermilionCity_Gym_hns/scripts.inc", (
        member("TRAINER_LTSURGE_HNS", 5, 5, ace=True), member("TRAINER_LTSURGE_HNS", 4, 4),
        member("TRAINER_LTSURGE_HNS", 0, 0), member("TRAINER_LTSURGE_HNS", 1, 1),
        member("TRAINER_LTSURGE_HNS", 2, 2), member("TRAINER_LTSURGE_HNS", 3, 3)),
        aliases=("TRAINER_LTSURGE_DOJO_HNS",)),
    roster("Erika", "Kanto", "TRAINER_ERIKA_HNS", "data/maps/CeladonCity_Gym_hns/scripts.inc", (
        member("TRAINER_ERIKA_HNS", 5, 5, ace=True), member("TRAINER_ERIKA_HNS", 3, 4),
        member("TRAINER_ERIKA_HNS", 0, 0), member("TRAINER_ERIKA_HNS", 1, 1),
        member("TRAINER_ERIKA_HNS", 2, 2), member("TRAINER_ERIKA_HNS", 4, 3)),
        aliases=("TRAINER_ERIKA_DOJO_HNS",)),
    roster("Janine", "Kanto", "TRAINER_JANINE_HNS", "data/maps/FuchsiaCity_Gym_hns/scripts.inc", (
        member("TRAINER_JANINE_HNS", 4, 5, ace=True), member("TRAINER_JANINE_HNS", 5, 4),
        member("TRAINER_JANINE_HNS", 0, 0), member("TRAINER_JANINE_HNS", 1, 1),
        member("TRAINER_JANINE_HNS", 2, 2), member("TRAINER_JANINE_HNS", 3, 3)),
        aliases=("TRAINER_JANINE_DOJO_HNS",)),
    roster("Sabrina", "Kanto", "TRAINER_SABRINA_HNS", "data/maps/SaffronCity_Gym_hns/scripts.inc", (
        member("TRAINER_SABRINA_HNS", 5, 5, ace=True), member("TRAINER_SABRINA_HNS", 4, 4),
        member("TRAINER_SABRINA_HNS", 0, 0), member("TRAINER_SABRINA_HNS", 1, 1),
        member("TRAINER_SABRINA_HNS", 2, 2), member("TRAINER_SABRINA_HNS", 3, 3)),
        aliases=("TRAINER_SABRINA_DOJO_HNS",)),
    roster("Blaine", "Kanto", "TRAINER_BLAINE_HNS", "data/maps/SeafoamIslands_Gym_hns/scripts.inc", (
        member("TRAINER_BLAINE_HNS", 1, 5, ace=True), member("TRAINER_BLAINE_HNS", 0, 4),
        member("TRAINER_BLAINE_HNS", 2, 0), member("TRAINER_BLAINE_HNS", 3, 1),
        member("TRAINER_BLAINE_HNS", 4, 2), member("TRAINER_BLAINE_HNS", 5, 3)),
        aliases=("TRAINER_BLAINE_DOJO_HNS",)),
    roster("Blue", "Kanto", "TRAINER_BLUE_HNS", "data/maps/ViridianCity_Gym_hns/scripts.inc", (
        member("TRAINER_BLUE_HNS", 1, 5, ace=True), member("TRAINER_BLUE_HNS", 5, 4),
        member("TRAINER_BLUE_HNS", 0, 0), member("TRAINER_BLUE_HNS", 2, 1),
        member("TRAINER_BLUE_HNS", 3, 2), member("TRAINER_BLUE_HNS", 4, 3)),
        aliases=("TRAINER_BLUE_DOJO_HNS",)),

    roster("Falkner", "Johto", "TRAINER_FALKNER_1_HNS", "data/maps/VioletCity_Gym_hns/scripts.inc", (
        member("TRAINER_FALKNER_1_HNS", 1, 5, ace=True), member("TRAINER_FALKNER_1_HNS", 0, 4),
        member("TRAINER_FALKNER_2_HNS", 1, 0), member("TRAINER_FALKNER_2_HNS", 2, 1),
        member("TRAINER_FALKNER_2_HNS", 4, 2), member("TRAINER_FALKNER_2_HNS", 5, 3)),
        aliases=("TRAINER_FALKNER_2_HNS",)),
    roster("Bugsy", "Johto", "TRAINER_BUGSY_1_HNS", "data/maps/AzaleaTown_Gym_hns/scripts.inc", (
        member("TRAINER_BUGSY_1_HNS", 2, 5, ace=True), member("TRAINER_BUGSY_1_HNS", 0, 4),
        member("TRAINER_BUGSY_1_HNS", 1, 0), member("TRAINER_BUGSY_2_HNS", 1, 1),
        member("TRAINER_BUGSY_2_HNS", 2, 2), member("TRAINER_BUGSY_2_HNS", 3, 3)),
        aliases=("TRAINER_BUGSY_2_HNS",)),
    roster("Whitney", "Johto", "TRAINER_WHITNEY_1_HNS", "data/maps/GoldenrodCity_Gym_hns/scripts.inc", (
        member("TRAINER_WHITNEY_1_HNS", 2, 5, ace=True), member("TRAINER_WHITNEY_1_HNS", 0, 4),
        member("TRAINER_WHITNEY_1_HNS", 1, 0), member("TRAINER_WHITNEY_2_HNS", 1, 1),
        member("TRAINER_WHITNEY_2_HNS", 2, 2), member("TRAINER_WHITNEY_2_HNS", 3, 3)),
        aliases=("TRAINER_WHITNEY_2_HNS",)),
    roster("Morty", "Johto", "TRAINER_MORTY_1_HNS", "data/maps/EcruteakCity_Gym_hns/scripts.inc", (
        member("TRAINER_MORTY_1_HNS", 3, 5, ace=True), member("TRAINER_MORTY_1_HNS", 0, 4),
        member("TRAINER_MORTY_1_HNS", 2, 0), member("TRAINER_MORTY_1_HNS", 1, 1),
        member("TRAINER_MORTY_2_HNS", 0, 2), member("TRAINER_MORTY_2_HNS", 3, 3)),
        aliases=("TRAINER_MORTY_2_HNS",)),
    roster("Chuck", "Johto", "TRAINER_CHUCK_1_HNS", "data/maps/CianwoodGym_hns/scripts.inc", (
        member("TRAINER_CHUCK_1_HNS", 3, 5, ace=True), member("TRAINER_CHUCK_1_HNS", 0, 4),
        member("TRAINER_CHUCK_1_HNS", 1, 0), member("TRAINER_CHUCK_1_HNS", 2, 1),
        member("TRAINER_CHUCK_2_HNS", 0, 2), member("TRAINER_CHUCK_2_HNS", 4, 3)),
        aliases=("TRAINER_CHUCK_2_HNS",)),
    roster("Chuck", "Johto", "TRAINER_CHUCK_1_2_HNS", "data/maps/CianwoodGym_hns/scripts.inc", (
        member("TRAINER_CHUCK_1_2_HNS", 4, 5, ace=True), member("TRAINER_CHUCK_1_2_HNS", 0, 4),
        member("TRAINER_CHUCK_1_2_HNS", 1, 0), member("TRAINER_CHUCK_1_2_HNS", 2, 1),
        member("TRAINER_CHUCK_1_2_HNS", 3, 2), member("TRAINER_CHUCK_2_HNS", 4, 3)),
        aliases=("TRAINER_CHUCK_2_HNS",)),
    roster("Chuck", "Johto", "TRAINER_CHUCK_1_3_HNS", "data/maps/CianwoodGym_hns/scripts.inc", (
        member("TRAINER_CHUCK_1_3_HNS", 4, 5, ace=True), member("TRAINER_CHUCK_1_3_HNS", 0, 4),
        member("TRAINER_CHUCK_1_3_HNS", 1, 0), member("TRAINER_CHUCK_1_3_HNS", 2, 1),
        member("TRAINER_CHUCK_1_3_HNS", 3, 2), member("TRAINER_CHUCK_2_HNS", 4, 3)),
        aliases=("TRAINER_CHUCK_2_HNS",)),
    roster("Jasmine", "Johto", "TRAINER_JASMINE_1_HNS", "data/maps/OlivineCity_Gym_hns/scripts.inc", (
        member("TRAINER_JASMINE_1_HNS", 4, 5, ace=True), member("TRAINER_JASMINE_1_HNS", 0, 4),
        member("TRAINER_JASMINE_1_HNS", 1, 0), member("TRAINER_JASMINE_1_HNS", 2, 1),
        member("TRAINER_JASMINE_1_HNS", 3, 2), member("TRAINER_JASMINE_2_HNS", 1, 3)),
        aliases=("TRAINER_JASMINE_2_HNS",)),
    roster("Jasmine", "Johto", "TRAINER_JASMINE_1_2_HNS", "data/maps/OlivineCity_Gym_hns/scripts.inc", (
        member("TRAINER_JASMINE_1_2_HNS", 4, 5, ace=True), member("TRAINER_JASMINE_1_2_HNS", 0, 4),
        member("TRAINER_JASMINE_1_2_HNS", 1, 0), member("TRAINER_JASMINE_1_2_HNS", 2, 1),
        member("TRAINER_JASMINE_1_2_HNS", 3, 2), member("TRAINER_JASMINE_2_HNS", 1, 3)),
        aliases=("TRAINER_JASMINE_2_HNS",)),
    roster("Jasmine", "Johto", "TRAINER_JASMINE_1_3_HNS", "data/maps/OlivineCity_Gym_hns/scripts.inc", (
        member("TRAINER_JASMINE_1_3_HNS", 4, 5, ace=True), member("TRAINER_JASMINE_1_3_HNS", 0, 4),
        member("TRAINER_JASMINE_1_3_HNS", 1, 0), member("TRAINER_JASMINE_1_3_HNS", 2, 1),
        member("TRAINER_JASMINE_1_3_HNS", 3, 2), member("TRAINER_JASMINE_2_HNS", 1, 3)),
        aliases=("TRAINER_JASMINE_2_HNS",)),
    roster("Pryce", "Johto", "TRAINER_PRYCE_1_HNS", "data/maps/MahoganyTown_Gym_hns/scripts.inc", (
        member("TRAINER_PRYCE_1_HNS", 3, 5, ace=True), member("TRAINER_PRYCE_1_HNS", 1, 4),
        member("TRAINER_PRYCE_1_HNS", 0, 0), member("TRAINER_PRYCE_1_HNS", 2, 1),
        member("TRAINER_PRYCE_1_2_HNS", 0, 2), member("TRAINER_PRYCE_2_HNS", 3, 3)),
        aliases=("TRAINER_PRYCE_2_HNS",)),
    roster("Pryce", "Johto", "TRAINER_PRYCE_1_2_HNS", "data/maps/MahoganyTown_Gym_hns/scripts.inc", (
        member("TRAINER_PRYCE_1_2_HNS", 4, 5, ace=True), member("TRAINER_PRYCE_1_2_HNS", 1, 4),
        member("TRAINER_PRYCE_1_2_HNS", 0, 0), member("TRAINER_PRYCE_1_2_HNS", 2, 1),
        member("TRAINER_PRYCE_1_2_HNS", 3, 2), member("TRAINER_PRYCE_2_HNS", 3, 3)),
        aliases=("TRAINER_PRYCE_2_HNS",)),
    roster("Pryce", "Johto", "TRAINER_PRYCE_1_3_HNS", "data/maps/MahoganyTown_Gym_hns/scripts.inc", (
        member("TRAINER_PRYCE_1_3_HNS", 4, 5, ace=True), member("TRAINER_PRYCE_1_3_HNS", 1, 4),
        member("TRAINER_PRYCE_1_3_HNS", 0, 0), member("TRAINER_PRYCE_1_3_HNS", 2, 1),
        member("TRAINER_PRYCE_1_3_HNS", 3, 2), member("TRAINER_PRYCE_2_HNS", 3, 3)),
        aliases=("TRAINER_PRYCE_2_HNS",)),
    roster("Clair", "Johto", "TRAINER_CLAIR_1_HNS", "data/maps/BlackthornCity_Gym_hns/scripts.inc", (
        member("TRAINER_CLAIR_1_HNS", 4, 5, ace=True), member("TRAINER_CLAIR_1_HNS", 0, 4),
        member("TRAINER_CLAIR_1_HNS", 1, 0), member("TRAINER_CLAIR_1_HNS", 2, 1),
        member("TRAINER_CLAIR_1_HNS", 3, 2), member("TRAINER_CLAIR_2_HNS", 3, 3)),
        aliases=("TRAINER_CLAIR_2_HNS",)),

    roster("Roxanne", "Hoenn", "TRAINER_ROXANNE_1", "data/maps/RustboroCity_Gym/scripts.inc", (
        member("TRAINER_ROXANNE_1", 2, 5, ace=True), member("TRAINER_ROXANNE_1", 0, 4),
        member("TRAINER_ROXANNE_1", 1, 0), member("TRAINER_ROXANNE_2", 1, 1),
        member("TRAINER_ROXANNE_2", 2, 2), member("TRAINER_ROXANNE_3", 0, 3)),
        runtime_overrides=("TRAINER_ROXANNE_2", "TRAINER_ROXANNE_3", "TRAINER_ROXANNE_4", "TRAINER_ROXANNE_5")),
    roster("Brawly", "Hoenn", "TRAINER_BRAWLY_1", "data/maps/DewfordTown_Gym/scripts.inc", (
        member("TRAINER_BRAWLY_1", 2, 5, ace=True), member("TRAINER_BRAWLY_1", 0, 4),
        member("TRAINER_BRAWLY_1", 1, 0), member("TRAINER_BRAWLY_2", 2, 1),
        member("TRAINER_BRAWLY_2", 0, 2), member("TRAINER_BRAWLY_2", 3, 3)),
        runtime_overrides=("TRAINER_BRAWLY_2", "TRAINER_BRAWLY_3", "TRAINER_BRAWLY_4", "TRAINER_BRAWLY_5")),
    roster("Wattson", "Hoenn", "TRAINER_WATTSON_1", "data/maps/MauvilleCity_Gym/scripts.inc", (
        member("TRAINER_WATTSON_1", 3, 5, ace=True), member("TRAINER_WATTSON_1", 2, 4),
        member("TRAINER_WATTSON_1", 0, 0), member("TRAINER_WATTSON_1", 1, 1),
        member("TRAINER_WATTSON_2", 1, 2), member("TRAINER_WATTSON_4", 1, 3)),
        runtime_overrides=("TRAINER_WATTSON_2", "TRAINER_WATTSON_3", "TRAINER_WATTSON_4", "TRAINER_WATTSON_5")),
    roster("Flannery", "Hoenn", "TRAINER_FLANNERY_1", "data/maps/LavaridgeTown_Gym_1F/scripts.inc", (
        member("TRAINER_FLANNERY_1", 3, 5, ace=True), member("TRAINER_FLANNERY_1", 0, 4),
        member("TRAINER_FLANNERY_1", 1, 0), member("TRAINER_FLANNERY_1", 2, 1),
        member("TRAINER_FLANNERY_2", 0, 2), member("TRAINER_FLANNERY_2", 1, 3)),
        runtime_overrides=("TRAINER_FLANNERY_2", "TRAINER_FLANNERY_3", "TRAINER_FLANNERY_4", "TRAINER_FLANNERY_5")),
    roster("Norman", "Hoenn", "TRAINER_NORMAN_1", "data/maps/PetalburgCity_Gym/scripts.inc", (
        member("TRAINER_NORMAN_1", 3, 5, ace=True), member("TRAINER_NORMAN_1", 1, 4),
        member("TRAINER_NORMAN_1", 0, 0), member("TRAINER_NORMAN_1", 2, 1),
        member("TRAINER_NORMAN_3", 2, 2), member("TRAINER_NORMAN_5", 3, 3)),
        runtime_overrides=("TRAINER_NORMAN_2", "TRAINER_NORMAN_3", "TRAINER_NORMAN_4", "TRAINER_NORMAN_5")),
    roster("Winona", "Hoenn", "TRAINER_WINONA_1", "data/maps/FortreeCity_Gym/scripts.inc", (
        member("TRAINER_WINONA_1", 4, 5, ace=True), member("TRAINER_WINONA_1", 0, 4),
        member("TRAINER_WINONA_1", 1, 0), member("TRAINER_WINONA_1", 2, 1),
        member("TRAINER_WINONA_1", 3, 2), member("TRAINER_WINONA_2", 0, 3)),
        runtime_overrides=("TRAINER_WINONA_2", "TRAINER_WINONA_3", "TRAINER_WINONA_4", "TRAINER_WINONA_5")),
    roster("Tate/Liza", "Hoenn", "TRAINER_TATE_AND_LIZA_1", "data/maps/MossdeepCity_Gym/scripts.inc", (
        member("TRAINER_TATE_AND_LIZA_1", 2, 0, ace=True), member("TRAINER_TATE_AND_LIZA_1", 3, 1, ace=True),
        member("TRAINER_TATE_AND_LIZA_1", 0, 2), member("TRAINER_TATE_AND_LIZA_1", 1, 3),
        member("TRAINER_TATE_AND_LIZA_2", 0, 4), member("TRAINER_TATE_AND_LIZA_3", 0, 5)),
        double=True, runtime_overrides=("TRAINER_TATE_AND_LIZA_2", "TRAINER_TATE_AND_LIZA_3", "TRAINER_TATE_AND_LIZA_4", "TRAINER_TATE_AND_LIZA_5")),
    roster("Juan", "Hoenn", "TRAINER_JUAN_1", "data/maps/SootopolisCity_Gym_1F/scripts.inc", (
        member("TRAINER_JUAN_1", 4, 5, ace=True), member("TRAINER_JUAN_1", 0, 4),
        member("TRAINER_JUAN_1", 1, 0), member("TRAINER_JUAN_1", 2, 1),
        member("TRAINER_JUAN_1", 3, 2), member("TRAINER_JUAN_2", 2, 3)),
        runtime_overrides=("TRAINER_JUAN_2", "TRAINER_JUAN_3", "TRAINER_JUAN_4", "TRAINER_JUAN_5")),
)


def party_size(rating):
    rating = min(max(rating, 0), 80)
    return next(size for threshold, size in SIZE_THRESHOLDS if rating < threshold)


def level(rating, offset):
    rating = min(max(rating, 0), 80)
    for (r0, l0), (r1, l1) in zip(LEADER_ANCHORS, LEADER_ANCHORS[1:]):
        if rating <= r1:
            distance = r1 - r0
            numerator = (rating - r0) * (l1 - l0)
            baseline = l0 + (2 * numerator + distance) // (2 * distance)
            return min(100, max(1, baseline + offset))
    return min(100, max(1, 100 + offset))


def source_variant(records, owner):
    if owner not in records:
        raise ValidationError(f"unknown party source owner {owner}")
    return records[owner]["DIFFICULTY_NORMAL"]


def resolve(records, ids):
    """Resolve all source references without permitting implicit substitutions."""
    identities = set()
    seen_trainers = set()
    output = []
    for spec in ROSTERS:
        identity = spec["identity"]
        identities.add(identity)
        trainer = spec["trainer"]
        if trainer in seen_trainers:
            raise ValidationError(f"duplicate enrolled trainer ID {trainer}")
        seen_trainers.add(trainer)
        if trainer not in ids:
            raise ValidationError(f"unknown enrolled trainer ID {trainer}")
        legacy = source_variant(records, trainer)
        if not legacy["slots"] or not legacy.get("partySize"):
            raise ValidationError(f"{trainer}: missing legacy initial party")
        if len(spec["members"]) != PARTY_SIZE:
            raise ValidationError(f"{trainer}: roster must author exactly six members")
        expanded = []
        for source_index, descriptor in enumerate(spec["members"]):
            owner = descriptor["owner"]
            owner_record = source_variant(records, owner)
            slot = descriptor["slot"]
            if owner not in ids or not isinstance(slot, int) or not 0 <= slot < len(owner_record["slots"]):
                raise ValidationError(f"{trainer}: invalid source reference {owner}/{slot}")
            mon = copy.deepcopy(owner_record["slots"][slot])
            policy = descriptor["movePolicy"]
            if policy not in (AUTHORED, LEVEL_UP):
                raise ValidationError(f"{trainer}/{source_index}: unknown move policy {policy}")
            moves = mon.get("moves", [])
            if policy == AUTHORED and not moves:
                raise ValidationError(f"{trainer}/{source_index}: authored policy needs nonempty move tuple")
            if policy == LEVEL_UP and moves:
                raise ValidationError(f"{trainer}/{source_index}: level-up policy cannot carry authored moves")
            expanded.append(dict(sourceIndex=source_index, sourceOwner=owner,
                                 sourceSlot=slot, mon=mon, **descriptor))
        orders = [entry["battleOrder"] for entry in expanded]
        if sorted(orders) != list(range(PARTY_SIZE)):
            raise ValidationError(f"{trainer}: battle order is not a permutation")
        aces = [entry for entry in expanded if entry["isAce"]]
        if not aces or not expanded[0]["isAce"] or any(entry["sourceIndex"] > 1 or entry["levelOffset"] != 0 for entry in aces):
            raise ValidationError(f"{trainer}: invalid ace placement or level offset")
        if any(not entry["isAce"] and entry["levelOffset"] not in (-1, -2) for entry in expanded):
            raise ValidationError(f"{trainer}: non-ace offsets must be -1 or -2")
        if spec["isDoubleBattle"]:
            if len(aces) != 2 or [entry["mon"]["species"] for entry in expanded[:2]] != ["SPECIES_LUNATONE", "SPECIES_SOLROCK"] or orders[:2] != [0, 1]:
                raise ValidationError(f"{trainer}: Tate/Liza must open with Lunatone and Solrock")
        elif any(ace["battleOrder"] < nonace["battleOrder"] for ace in aces for nonace in expanded if not nonace["isAce"]):
            raise ValidationError(f"{trainer}: ace must construct after non-aces")
        if (legacy.get("battleType") == "TRAINER_BATTLE_TYPE_DOUBLES") != spec["isDoubleBattle"]:
            raise ValidationError(f"{trainer}: double-battle marker differs from legacy source")
        if "AI_FLAG_ACE_POKEMON" in legacy.get("aiFlags", "") or "AI_FLAG_DOUBLE_ACE_POKEMON" in legacy.get("aiFlags", ""):
            raise ValidationError(f"{trainer}: incompatible ace-lock AI flag")
        script = ROOT / spec["script"]
        if not script.is_file() or trainer not in script.read_text():
            raise ValidationError(f"{trainer}: stale initial-battle script reference {spec['script']}")
        # Kanto's Dojo formerly shared these source IDs.  Its clone has to
        # remain a byte-for-byte authored roster rather than silently taking
        # the initial badge encounter's runtime scaling policy.
        for alias in (item for item in spec["aliases"] if item.endswith("_DOJO_HNS")):
            alias_legacy = source_variant(records, alias)
            aliases_fields = {key: value for key, value in alias_legacy.items()
                              if key not in ("owner", "owner_variant")}
            legacy_fields = {key: value for key, value in legacy.items()
                             if key not in ("owner", "owner_variant")}
            if alias not in ids or aliases_fields != legacy_fields:
                raise ValidationError(f"{trainer}: excluded alias {alias} is not an exact legacy clone")
        for override in spec["runtimeOverrides"]:
            if override not in ids:
                raise ValidationError(f"{trainer}: unknown runtime rematch override {override}")
        output.append(dict(**spec, expanded=expanded, legacy=copy.deepcopy(legacy),
                           moneyBasis=dict(authoredLevel=legacy.get("money_level"),
                                           trainerClass=legacy.get("money_trainer_class"),
                                           partySize=legacy.get("partySize"),
                                           nullParty=legacy.get("money_party_null"))))
    expected = {"Brock", "Misty", "Lt. Surge", "Erika", "Janine", "Sabrina", "Blaine", "Blue",
                "Falkner", "Bugsy", "Whitney", "Morty", "Chuck", "Jasmine", "Pryce", "Clair",
                "Roxanne", "Brawly", "Wattson", "Flannery", "Norman", "Winona", "Tate/Liza", "Juan"}
    if identities != expected:
        raise ValidationError(f"initial identities mismatch: {sorted(identities ^ expected)}")
    return output


def validate_policy_inventory(resolved):
    """Keep the ordinary-policy manifest and this authored table inseparable."""
    manifest = json.loads(trainer_inventory.MANIFEST.read_text())
    policies = {row["id"]: row["policy"] for row in manifest["records"]}
    enrolled = {record["trainer"] for record in resolved}
    manifest_enrolled = {trainer for trainer, policy in policies.items()
                         if policy == "GYM_LEADER"}
    if enrolled != manifest_enrolled:
        raise ValidationError("Gym Leader policy inventory differs from authored roster table")
    excluded_contexts = {item for record in resolved
                         for item in record["aliases"] + record["runtimeOverrides"]}
    invalid = sorted(item for item in excluded_contexts if policies.get(item) != "EXCLUDED")
    if invalid:
        raise ValidationError(f"leader aliases or rematch overrides must be excluded: {invalid}")


def render_mon(mon, indent="        "):
    fields = ("species", "heldItem", "ability", "iv", "ev", "lvl", "ball", "friendship", "nature", "gender", "isShiny", "dynamaxLevel")
    lines = [indent + "{"]
    for field in fields:
        if field in mon:
            lines.append(f"{indent}    .{field} = {mon[field]},")
    if mon.get("moves"):
        lines.append(f"{indent}    .moves = {{ {', '.join(mon['moves'])} }},")
    lines.append(indent + "},")
    return "\n".join(lines)


def symbol(record, suffix):
    return "sGymLeader" + "".join(ch for ch in record["trainer"].title() if ch.isalnum()) + suffix


def render_header(resolved):
    lines = ["// Generated by tools/trainer_scaling/gym_leaders.py; edit its roster source.",
             "// This file deliberately stores an independent legacy party for randomizer and rollback paths.", ""]
    for record in resolved:
        expanded_name = symbol(record, "Party")
        legacy_name = symbol(record, "LegacyParty")
        lines.extend((f"static const struct TrainerMon {expanded_name}[PARTY_SIZE] =", "{"))
        lines.extend(render_mon(entry["mon"]) for entry in record["expanded"])
        lines.extend(("};", "", f"static const struct TrainerMon {legacy_name}[] =", "{"))
        lines.extend(render_mon(mon) for mon in record["legacy"]["slots"][:record["legacy"]["partySize"]])
        lines.extend(("};", ""))
    lines.extend(("static const struct GymLeaderScalingRoster sGymLeaderScalingRosters[] =", "{"))
    for record in resolved:
        lines.extend(("    {", f"        .trainerId = {record['trainer']},", f"        .ownerId = {record['owner']},",
                      f"        .difficulty = {record['difficulty']},", f"        .party = {symbol(record, 'Party')},",
                      f"        .legacyParty = {symbol(record, 'LegacyParty')},", f"        .legacyPartySize = {record['legacy']['partySize']},", "        .slots = {"))
        for entry in record["expanded"]:
            policy = "GYM_LEADER_MOVE_AUTHORED" if entry["movePolicy"] == AUTHORED else "GYM_LEADER_MOVE_LEVEL_UP"
            lines.append("            { .battleOrder = %d, .isAce = %s, .levelOffset = %d, .movePolicy = %s }," % (entry["battleOrder"], "TRUE" if entry["isAce"] else "FALSE", entry["levelOffset"], policy))
        lines.extend(("        },", f"        .isDoubleBattle = {'TRUE' if record['isDoubleBattle'] else 'FALSE'},", "    },"))
    lines.extend(("};", ""))
    return "\n".join(lines)


def report_rows(resolved):
    rows = []
    for record in resolved:
        for rating in range(81):
            count = party_size(rating)
            retained = record["expanded"][:count]
            ordered = sorted(retained, key=lambda entry: entry["battleOrder"])
            rows.append(dict(identity=record["identity"], region=record["region"], trainer=record["trainer"],
                             owner=record["owner"], difficulty=record["difficulty"], rating=rating, count=count,
                             selectedSourceIndices=[entry["sourceIndex"] for entry in retained],
                             outputSourceIndices=[entry["sourceIndex"] for entry in ordered],
                             members=[dict(sourceIndex=entry["sourceIndex"], sourceOwner=entry["sourceOwner"],
                                               sourceSlot=entry["sourceSlot"], species=entry["mon"]["species"],
                                               level=level(rating, entry["levelOffset"]), moves=entry["mon"].get("moves", []),
                                               movePolicy=entry["movePolicy"], item=entry["mon"].get("heldItem", "ITEM_NONE"),
                                               isAce=entry["isAce"], battleOrder=entry["battleOrder"], levelOffset=entry["levelOffset"])
                                           for entry in ordered], moneyBasis=record["moneyBasis"], isDoubleBattle=record["isDoubleBattle"]))
    return rows


def render_markdown(resolved):
    lines = ["# Gym Leader scaling roster inventory", "", "This generated inventory covers all 24 initial badge identities and all 30 resolved active source variants. It is structural evidence, not a claim that required ROM playtesting has succeeded.", "", "The complete 0–80 Trainer Rating table, including retained source indices, output order, levels, moves, items, aces, and legacy money basis, is in `gym_leaders.json` alongside this report.", ""]
    for record in resolved:
        lines.extend((f"## {record['identity']} — `{record['trainer']}`", "", f"Initial script: `{record['script']}`. Resolved owner/difficulty: `{record['owner']}` / `{record['difficulty']}`. Legacy money basis: level {record['moneyBasis']['authoredLevel']}, class `{record['moneyBasis']['trainerClass']}`, legacy size {record['moneyBasis']['partySize']}.", "", "| Retention | Battle order | Role | Offset | Source | Species | Moves | Item |", "| ---: | ---: | --- | ---: | --- | --- | --- | --- |"))
        for entry in record["expanded"]:
            mon = entry["mon"]
            lines.append("| %d | %d | %s | %d | `%s` slot %d | %s | %s | %s |" % (entry["sourceIndex"], entry["battleOrder"], "Ace" if entry["isAce"] else "Support", entry["levelOffset"], entry["sourceOwner"], entry["sourceSlot"], mon["species"].removeprefix("SPECIES_"), ", ".join(move.removeprefix("MOVE_") for move in mon.get("moves", [])) if entry["movePolicy"] == AUTHORED else "LEVEL_UP", mon.get("heldItem", "ITEM_NONE").removeprefix("ITEM_")))
        if record["aliases"] or record["runtimeOverrides"]:
            lines.extend(("", f"Excluded aliases: {', '.join('`' + item + '`' for item in record['aliases']) or 'none'}. Runtime rematch overrides: {', '.join('`' + item + '`' for item in record['runtimeOverrides']) or 'none'}."))
        lines.append("")
    return "\n".join(lines).rstrip() + "\n"


def write(path, contents, check):
    if check:
        if not path.is_file() or path.read_text() != contents:
            raise ValidationError(f"stale generated output: {path.relative_to(ROOT)}")
    else:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents)


def generate(check=False):
    raw = trainer_inventory.load_inventory()
    records = trainer_inventory.resolve_rosters(raw)
    ids = trainer_inventory.trainer_ids(raw)
    resolved = resolve(records, ids)
    validate_policy_inventory(resolved)
    write(HEADER, render_header(resolved), check)
    write(REPORT, render_markdown(resolved), check)
    inventory = dict(version=1, encounterIdentities=24, resolvedVariants=len(resolved),
                     partySizeThresholds=[dict(beforeRating=threshold, partySize=size) for threshold, size in SIZE_THRESHOLDS],
                     levelAnchors=[dict(rating=rating, aceLevel=ace_level) for rating, ace_level in LEADER_ANCHORS],
                     rosters=[dict(identity=row["identity"], region=row["region"], trainer=row["trainer"], owner=row["owner"], difficulty=row["difficulty"], script=row["script"], aliases=row["aliases"], runtimeOverrides=row["runtimeOverrides"], legacyPartySize=row["legacy"]["partySize"], moneyBasis=row["moneyBasis"], isDoubleBattle=row["isDoubleBattle"], members=[dict(sourceIndex=entry["sourceIndex"], sourceOwner=entry["sourceOwner"], sourceSlot=entry["sourceSlot"], species=entry["mon"]["species"], moves=entry["mon"].get("moves", []), item=entry["mon"].get("heldItem", "ITEM_NONE"), battleOrder=entry["battleOrder"], isAce=entry["isAce"], levelOffset=entry["levelOffset"], movePolicy=entry["movePolicy"]) for entry in row["expanded"]]) for row in resolved],
                     projections=report_rows(resolved),
                     balanceStatus="Generated roster tables are not balance or emulator-playtest acceptance.")
    write(INVENTORY, json.dumps(inventory, indent=2) + "\n", check)
    return inventory


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    try:
        generate(check=args.check)
    except ValidationError as error:
        print(f"gym leader roster validation failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
