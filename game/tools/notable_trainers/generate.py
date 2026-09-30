#!/usr/bin/env python3
"""Validate the game-owned v0 trainer catalog and emit its C tables."""

import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CATALOG = Path(__file__).with_name("catalog.json")
DATA_OUTPUT = ROOT / "src/data/notable_trainers/catalog.h"
REGISTRY_OUTPUT = ROOT / "src/data/notable_trainers/league_registry.h"
IDS_OUTPUT = ROOT / "include/constants/notable_trainers.h"
BADGE_ENCOUNTERS = {
    "TRAINER_BROCK_HNS", "TRAINER_MISTY_HNS", "TRAINER_LTSURGE_HNS",
    "TRAINER_ERIKA_HNS", "TRAINER_JANINE_HNS", "TRAINER_SABRINA_HNS",
    "TRAINER_BLAINE_HNS", "TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS",
    "TRAINER_FALKNER_1_HNS", "TRAINER_BUGSY_1_HNS", "TRAINER_WHITNEY_1_HNS",
    "TRAINER_MORTY_1_HNS", "TRAINER_CHUCK_1_HNS", "TRAINER_JASMINE_1_HNS",
    "TRAINER_PRYCE_1_HNS", "TRAINER_CLAIR_1_HNS", "TRAINER_ROXANNE_1",
    "TRAINER_BRAWLY_1", "TRAINER_WATTSON_1", "TRAINER_FLANNERY_1",
    "TRAINER_NORMAN_1", "TRAINER_WINONA_1", "TRAINER_TATE_AND_LIZA_1",
    "TRAINER_JUAN_1",
}


# Pool authoring rules from the notable-trainer specification.
FRUSTRATION = {move: category for category, moves in {
    "sleep": ["Hypnosis", "Sleep Powder", "Spore", "Lovely Kiss", "Yawn", "Sing", "Grass Whistle", "Dark Void"],
    "evasion": ["Double Team", "Minimize", "Smokescreen", "Sand Attack", "Flash", "Kinesis", "Mud Slap"],
    "OHKO": ["Sheer Cold", "Fissure", "Horn Drill", "Guillotine"],
    "trapping": ["Bind", "Wrap", "Fire Spin", "Whirlpool", "Sand Tomb", "Clamp", "Infestation", "Magma Storm",
                 "Snap Trap", "Thunder Cage", "Mean Look", "Block", "Spider Web", "Jaw Lock", "Anchor Shot",
                 "Spirit Shackle", "Octolock", "Fairy Lock", "Thousand Waves"],
    "Perish Song": ["Perish Song"],
    "Destiny Bond": ["Destiny Bond"],
    "infatuation/confusion": ["Attract", "Swagger", "Flatter", "Confuse Ray", "Supersonic", "Sweet Kiss",
                              "Teeter Dance", "Dynamic Punch"],
}.items() for move in moves}
TOXIC_MOVES = ("Toxic", "Toxic Spikes")


def symbol(prefix: str, name: str) -> str:
    return prefix + re.sub(r"[^A-Z0-9]+", "_", name.upper()).strip("_")


def known_symbols(path: str, prefix: str) -> set[str]:
    return set(re.findall(r"\b" + prefix + r"[A-Z0-9_]+", (ROOT / path).read_text()))


def validate_frustration(name, pool):
    """At most one frustration category per pool (FRUSTRATION), and no evasion with Toxic or Toxic Spikes."""
    found = {}
    categories = {symbol("MOVE_", move): category for move, category in FRUSTRATION.items()}
    pool_moves = {symbol("MOVE_", entry["move"]) for entry in pool}
    for entry in pool:
        category = categories.get(symbol("MOVE_", entry["move"]))
        if category:
            found.setdefault(category, []).append(entry["move"])
    if len(found) > 1:
        listed = "; ".join(f"{category}: {', '.join(moves)}" for category, moves in found.items())
        raise ValueError(f"{name} move pool: at most one frustration category, found {listed}")
    toxic = [move for move in TOXIC_MOVES if symbol("MOVE_", move) in pool_moves]
    if "evasion" in found and toxic:
        raise ValueError(f"{name} move pool: evasion ({', '.join(found['evasion'])}) never pairs with"
                         f" {', '.join(toxic)}")


def validate(data: dict) -> None:
    trainers = data["trainers"]
    limit = re.search(r"^#define MAX_NOTABLE_MOVE_POOL (\d+)$",
                      (ROOT / "include/notable_moves.h").read_text(), re.M)
    if limit is None:
        raise ValueError("runtime move-pool capacity is missing")
    pool_capacity = int(limit.group(1))
    if data["version"] != 1 or len(trainers) != 38:
        raise ValueError("v0 requires catalog version 1 and exactly 38 entries")
    known = {
        "SPECIES_": known_symbols("include/constants/species.h", "SPECIES_"),
        "MOVE_": known_symbols("include/constants/moves.h", "MOVE_"),
        "ITEM_": known_symbols("include/constants/items.h", "ITEM_"),
        "ABILITY_": known_symbols("include/constants/abilities.h", "ABILITY_"),
        # TrainerMon requires a concrete nature, not wild-encounter sentinels.
        "NATURE_": set(re.findall(r"^#define (NATURE_[A-Z0-9_]+)\s+\d+\b",
                                  (ROOT / "include/constants/pokemon.h").read_text(), re.M)),
        "OBJ_EVENT_GFX_": known_symbols("include/constants/event_objects.h", "OBJ_EVENT_GFX_"),
    }
    opponent_headers = list((ROOT / "include/constants").glob("opponents*.h"))
    opponent_headers += list((ROOT / "include/constants").glob("wayfarer_*trainers.h"))
    opponents = set().union(*(set(re.findall(r"^#define (TRAINER_[A-Z0-9_]+)", p.read_text(), re.M)) for p in opponent_headers))
    ids, aliases = set(), set()
    for trainer in trainers:
        name = trainer["name"]
        character_id = trainer["characterId"]
        if not isinstance(character_id, int) or character_id < 1 or character_id > 38 or character_id in ids:
            raise ValueError(f"{name}: invalid or duplicate characterId")
        ids.add(character_id)
        if trainer["archetype"] not in ("steady", "prodigy", "sleeper", "veteran", "rival", "legend", "star", "comeback", "burst"):
            raise ValueError(f"{name}: unknown archetype")
        if trainer["playStyle"] not in ("field_marshal", "gambler", "hexer", "brawler", "tactician", "sweeper", "turtle", "bomber"):
            raise ValueError(f"{name}: unknown play style")
        if trainer["homeRegion"] not in ("Kanto", "Johto", "Hoenn"):
            raise ValueError(f"{name}: unknown home region")
        if not all(type(trainer[k]) is bool for k in ("traveller", "aloof", "bossOmniscient", "doubleBattle")):
            raise ValueError(f"{name}: traits must be boolean")
        if not (0 <= trainer["startTR"] <= trainer["peakTR"] <= 65535):
            raise ValueError(f"{name}: invalid rating range")
        if trainer["archetype"] == "legend" and trainer["startTR"] != trainer["peakTR"]:
            raise ValueError(f"{name}: a Legend has constant rating")
        roster = trainer["roster"]
        if len(roster) != 6 or not roster[0]["isAce"] or roster[0]["levelOffset"] != 0:
            raise ValueError(f"{name}: invalid signature roster slot")
        if not 1 <= sum(bool(slot["isAce"]) for slot in roster) <= 3:
            raise ValueError(f"{name}: invalid ace count")
        for slot in roster:
            if not -6 <= slot["levelOffset"] <= 0:
                raise ValueError(f"{name}: invalid level offset")
            for prefix, field in (("SPECIES_", "species"), ("ITEM_", "item"), ("ABILITY_", "ability")):
                value = slot.get(field)
                if value and symbol(prefix, value) not in known[prefix]:
                    raise ValueError(f"{name}: unknown {field} {value}")
            nature = slot.get("nature")
            if nature is not None and (not isinstance(nature, str)
                                       or symbol("NATURE_", nature) not in known["NATURE_"]):
                raise ValueError(f"{name}: unknown nature {nature}")
        if not 1 <= len(trainer["movePool"]) <= pool_capacity:
            raise ValueError(f"{name}: move pool must contain 1..{pool_capacity} entries")
        for entry in trainer["movePool"]:
            move = symbol("MOVE_", entry["move"])
            if move == "MOVE_NONE" or move not in known["MOVE_"]:
                raise ValueError(f"{name}: invalid move pool entry {entry}")
            if "fromLevel" in entry and (type(entry["fromLevel"]) is not int
                                         or not 1 <= entry["fromLevel"] <= 100):
                raise ValueError(f"{name}: fromLevel must be an integer from 1 to 100")
        validate_frustration(name, trainer["movePool"])
        for alias in trainer["encounterIds"]:
            if alias not in opponents or alias in aliases:
                raise ValueError(f"{name}: unknown or duplicate encounter alias {alias}")
            aliases.add(alias)
        league = trainer["league"]
        if league["sourceTrainerId"] not in trainer["encounterIds"]:
            raise ValueError(f"{name}: league source must be the character's own encounter")
        if league["presentationId"] != character_id:
            raise ValueError(f"{name}: league presentation must identify this character")
        if league["objectGraphicsId"] not in known["OBJ_EVENT_GFX_"]:
            raise ValueError(f"{name}: unknown league overworld graphic")
        if type(league["enabled"]) is not bool or league["enabled"] == trainer["doubleBattle"]:
            raise ValueError(f"{name}: singles-only league eligibility mismatch")
        if league["enabled"] and league.get("disabledReason"):
            raise ValueError(f"{name}: enabled trainer has a disabled reason")
        if not league["enabled"] and not league.get("disabledReason"):
            raise ValueError(f"{name}: disabled trainer needs an authored reason")
    if ids != set(range(1, 39)):
        raise ValueError("character IDs must cover 1..38")
    if "TRAINER_BLUE_HNS" in aliases or "TRAINER_LEADER_GIOVANNI" in aliases:
        raise ValueError("excluded or superseded Gym encounter enrolled")
    if not BADGE_ENCOUNTERS <= aliases:
        raise ValueError(f"Wayfarer badge coverage incomplete: {sorted(BADGE_ENCOUNTERS - aliases)}")


def generate(data: dict) -> tuple[str, str, str]:
    trainers = sorted(data["trainers"], key=lambda t: t["characterId"])
    ids = ["#ifndef GUARD_CONSTANTS_NOTABLE_TRAINERS_H", "#define GUARD_CONSTANTS_NOTABLE_TRAINERS_H", "", "// Stable v0 character identities; encounter IDs are mapped separately.", "enum NotableTrainerId", "{", "    NOTABLE_TRAINER_NONE = 0,"]
    ids += [f"    {symbol('NOTABLE_TRAINER_', t['slug'])} = {t['characterId']}," for t in trainers]
    ids += ["};", "", "#endif // GUARD_CONSTANTS_NOTABLE_TRAINERS_H", ""]
    lines = ["// Generated from game/tools/notable_trainers/catalog.json. Do not edit by hand.", ""]
    for t in trainers:
        key = symbol("sNotableMovePool_", t["slug"])
        lines.append(f"static const struct NotableMovePoolEntry {key}[] =")
        lines.append("{")
        for entry in t["movePool"]:
            lines.append(f"    {{{symbol('MOVE_', entry['move'])}, {entry.get('fromLevel', 0)}}},")
        lines += ["};", ""]
    lines += ["const struct NotableTrainer gNotableTrainers[NOTABLE_TRAINER_COUNT] =", "{"]
    for t in trainers:
        key = symbol("sNotableMovePool_", t["slug"])
        lines += ["    {", f"        .characterId = {symbol('NOTABLE_TRAINER_', t['slug'])},", f"        .startTR = {t['startTR']},", f"        .peakTR = {t['peakTR']},", f"        .archetype = {symbol('NOTABLE_ARCHETYPE_', t['archetype'])},", f"        .homeRegion = {symbol('NOTABLE_REGION_', t['homeRegion'])},", f"        .playStyle = {symbol('NOTABLE_STYLE_', t['playStyle'])},", f"        .traveller = {'TRUE' if t['traveller'] else 'FALSE'},", f"        .aloof = {'TRUE' if t['aloof'] else 'FALSE'},", f"        .bossOmniscient = {'TRUE' if t['bossOmniscient'] else 'FALSE'},", f"        .isDoubleBattle = {'TRUE' if t['doubleBattle'] else 'FALSE'},", "        .roster =", "        {"]
        for slot in t["roster"]:
            fields = [f".species = {symbol('SPECIES_', slot['species'])}"]
            if slot.get("item"):
                fields.append(f".heldItem = {symbol('ITEM_', slot['item'])}")
            if slot.get("ability"):
                fields.append(f".ability = {symbol('ABILITY_', slot['ability'])}")
            if slot.get("nature") is not None:
                fields.append(f".nature = {symbol('NATURE_', slot['nature'])}")
            lines.append("            {" + ", ".join(fields) + "},")
        lines += ["        },", "        .levelOffsets = {" + ", ".join(str(s["levelOffset"]) for s in t["roster"]) + "},", f"        .aceMask = {sum((1 << i) for i, s in enumerate(t['roster']) if s['isAce'])},", f"        .movePool = {key},", f"        .movePoolCount = ARRAY_COUNT({key}),", "    },"]
    lines += ["};", "", "struct NotableEncounterAlias", "{", "    u16 trainerId;", "    u8 characterId;", "};", "", "static const struct NotableEncounterAlias sNotableEncounterAliases[] =", "{"]
    for t in trainers:
        for alias in t["encounterIds"]:
            lines.append(f"    {{{alias}, {symbol('NOTABLE_TRAINER_', t['slug'])}}},")
    lines += ["};", "", "static u8 GetNotableCharacterIdForEncounter(u16 trainerId)", "{", "    switch (trainerId)", "    {"]
    for t in trainers:
        for alias in t["encounterIds"]:
            lines.append(f"    case {alias}:")
        lines.append(f"        return {symbol('NOTABLE_TRAINER_', t['slug'])};")
    lines += ["    default:", "        return NOTABLE_TRAINER_NONE;", "    }", "}", ""]
    registry = ["// Generated from game/tools/notable_trainers/catalog.json. Do not edit by hand.", "",
                "const struct LeagueTrainer gLeagueTrainers[NOTABLE_TRAINER_COUNT] =", "{"]
    for t in trainers:
        league = t["league"]
        registry += ["    {",
                     f"        .characterId = {symbol('NOTABLE_TRAINER_', t['slug'])},",
                     f"        .sourceTrainerId = {league['sourceTrainerId']},",
                     f"        .presentationId = {league['presentationId']},",
                     f"        .objectGraphicsId = {league['objectGraphicsId']},",
                     f"        .enabled = {'TRUE' if league['enabled'] else 'FALSE'},",
                     "    },"]
    registry += ["};", ""]
    return "\n".join(ids), "\n".join(lines), "\n".join(registry)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="validate source and compare committed C output without writing")
    args = parser.parse_args()
    data = json.loads(CATALOG.read_text())
    validate(data)
    ids, table, registry = generate(data)
    if args.check:
        if IDS_OUTPUT.read_text() != ids or DATA_OUTPUT.read_text() != table or REGISTRY_OUTPUT.read_text() != registry:
            raise SystemExit("notable trainer generated data is stale; run tools/notable_trainers/generate.py")
    else:
        IDS_OUTPUT.write_text(ids)
        DATA_OUTPUT.write_text(table)
        REGISTRY_OUTPUT.write_text(registry)
    print(f"{len(data['trainers'])} trainers, {sum(len(t['encounterIds']) for t in data['trainers'])} encounter aliases")


if __name__ == "__main__":
    main()
