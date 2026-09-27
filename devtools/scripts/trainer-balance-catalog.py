#!/usr/bin/env python3
"""Regenerate the experimental explorer catalog from local trainerproc sources.

No network, ROM build, or temporary calibration files are required. Reference
moves/items describe authored sources only. Each trainer gets an authored TR and
a placeholder v0 roster (one ordered list of up to six roster slots) flattened
from the earlier ace/filler prototype. Real rosters are authored later; a roster
short of six is a content-gap warning, not a failure.
"""
from __future__ import annotations

import argparse
from fractions import Fraction
import importlib.util
import json
import math
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
GAME = ROOT / "game"
OUTPUT = ROOT / "devtools/ui/src/modules/trainer-balance/catalog.json"
GYMS = GAME / "src/data/trainer_scaling/gym_leaders.json"
SPECIES_INFO = GAME / "src/data/pokemon/species_info"
ROSTER_SIZE = 6
OFFSET_MIN, OFFSET_MAX = -6, 0
DEFAULT_OFFSET = -2
# Baby pre-evolutions do not count when checking whether a line covers a species.
BABIES = {"SPECIES_PICHU", "SPECIES_CLEFFA", "SPECIES_IGGLYBUFF", "SPECIES_TYROGUE", "SPECIES_SMOOCHUM",
          "SPECIES_ELEKID", "SPECIES_MAGBY", "SPECIES_AZURILL", "SPECIES_WYNAUT", "SPECIES_BUDEW",
          "SPECIES_CHINGLING", "SPECIES_BONSLY", "SPECIES_MIME_JR", "SPECIES_HAPPINY", "SPECIES_MUNCHLAX",
          "SPECIES_MANTYKE", "SPECIES_RIOLU", "SPECIES_TOXEL"}
# name, region, role, reference family/ID, OLD-scale TR, handwritten early
# species (appended when no roster line already covers them).
# The old TR is suggestedStartTR from the first explorer catalog (commit
# c3c83991ec, before the Living Rivals rework): Gym 1-5, Blue 6, Elite Four
# 40-53, Champions 53-55, on the retired 0-80 player TR scale. new_scale_tr()
# converts it to the rescaled player TR (contract section 6). Placeholder
# until the authoring session sets real values.
ROSTER = [
    ('Brock', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_BROCK', 2, ['Geodude', 'Onix']),
    ('Misty', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_MISTY', 3, ['Staryu', 'Psyduck']),
    ('Lt. Surge', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_LT_SURGE', 3, ['Voltorb', 'Pikachu']),
    ('Erika', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_ERIKA', 3, ['Oddish', 'Bellsprout']),
    ('Janine', 'Kanto', 'Gym Leader', 'HNS', 'TRAINER_JANINE_HNS', 4, ['Venonat', 'Koffing']),
    ('Sabrina', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_SABRINA', 4, ['Abra', 'Drowzee']),
    ('Blaine', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_BLAINE', 4, ['Growlithe', 'Ponyta']),
    ('Giovanni', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_GIOVANNI', 5, ['Sandshrew', 'Rhyhorn']),
    ('Blue', 'Kanto', 'Champion', 'FRLG', 'TRAINER_CHAMPION_FIRST_SQUIRTLE', 6, ['Pidgey', 'Eevee']),
    ('Lorelei', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_LORELEI', 50, []),
    ('Bruno', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_BRUNO', 52, []),
    ('Agatha', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_AGATHA', 53, []),
    ('Koga', 'Johto', 'Elite Four', 'HNS', 'TRAINER_KOGA_1_HNS', 42, []),
    ('Lance', 'Kanto', 'Champion', 'FRLG', 'TRAINER_ELITE_FOUR_LANCE', 55, []),
    ('Falkner', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_FALKNER_1_HNS', 1, ['Pidgey', 'Hoothoot']),
    ('Bugsy', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_BUGSY_1_HNS', 2, ['Caterpie', 'Weedle']),
    ('Whitney', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_WHITNEY_1_HNS', 3, ['Clefairy', 'Meowth']),
    ('Morty', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_MORTY_1_HNS', 3, ['Gastly', 'Misdreavus']),
    ('Chuck', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_CHUCK_1_HNS', 4, ['Machop', 'Makuhita']),
    ('Jasmine', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_JASMINE_1_HNS', 4, ['Magnemite', 'Aron']),
    ('Pryce', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_PRYCE_1_HNS', 4, ['Seel', 'Swinub']),
    ('Clair', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_CLAIR_1_HNS', 5, ['Dratini', 'Horsea']),
    ('Will', 'Johto', 'Elite Four', 'HNS', 'TRAINER_WILL_1_HNS', 40, []),
    ('Karen', 'Johto', 'Elite Four', 'HNS', 'TRAINER_KAREN_1_HNS', 44, []),
    ('Roxanne', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_ROXANNE_1', 3, ['Geodude', 'Nosepass']),
    ('Brawly', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_BRAWLY_1', 3, ['Machop', 'Makuhita']),
    ('Wattson', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_WATTSON_1', 3, ['Voltorb', 'Electrike']),
    ('Flannery', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_FLANNERY_1', 4, ['Numel', 'Slugma']),
    ('Norman', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_NORMAN_1', 4, ['Slakoth', 'Zigzagoon']),
    ('Winona', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_WINONA_1', 4, ['Swablu', 'Taillow']),
    ('Juan', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_JUAN_1', 5, ['Horsea', 'Barboach']),
    ('Sidney', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_SIDNEY', 46, []),
    ('Phoebe', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_PHOEBE', 47, []),
    ('Glacia', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_GLACIA', 49, []),
    ('Drake', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_DRAKE', 51, []),
    ('Wallace', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_WALLACE', 53, []),
    ('Steven', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_STEVEN', 53, []),
]


# Retired 0-80 level-cap curve and the rescaled one (24 badges = TR 160).
OLD_CAP = [(0, 15), (4, 16), (8, 18), (16, 23), (30, 30), (40, 42), (55, 60), (65, 80), (80, 100)]
NEW_CAP = [(0, 15), (40, 28), (80, 50), (120, 75), (160, 100)]
LEAGUE_OLD, LEAGUE_NEW = (40, 55), (70, 95)


def half_up(value):
    return math.floor(value + Fraction(1, 2))


def curve(anchors, tr):
    """Unrounded level at `tr` (linear between anchors, flat outside)."""
    if tr <= anchors[0][0]:
        return Fraction(anchors[0][1])
    for (lo_tr, lo_lv), (hi_tr, hi_lv) in zip(anchors, anchors[1:]):
        if tr <= hi_tr:
            return lo_lv + Fraction(tr - lo_tr, hi_tr - lo_tr) * (hi_lv - lo_lv)
    return Fraction(anchors[-1][1])


def inverse(anchors, level):
    """The TR at which a strictly rising curve reaches `level`."""
    for (lo_tr, lo_lv), (hi_tr, hi_lv) in zip(anchors, anchors[1:]):
        if level <= hi_lv:
            return lo_tr + (level - lo_lv) * Fraction(hi_tr - lo_tr, hi_lv - lo_lv)
    return Fraction(anchors[-1][0])


def new_scale_tr(role, old):
    """Return (new TR, how it was converted).

    Gym Leaders and Blue keep their team level (old level-cap curve level -> the
    TR giving it on the new level-cap curve); Elite Four and Champions map old 40-55
    linearly onto new 70-95, so the league top five land near TR 85-95.
    """
    if role == "Gym Leader" or old < LEAGUE_OLD[0]:
        level = curve(OLD_CAP, old)
        return half_up(inverse(NEW_CAP, level)), f"by level equivalence (old level-cap curve Lv {float(level):g} -> the TR giving it on the new level-cap curve)"
    (a, b), (c, d) = LEAGUE_OLD, LEAGUE_NEW
    return half_up(c + Fraction(old - a, b - a) * (d - c)), f"by mapping the old league band {a}-{b} linearly onto {c}-{d}"



def command(args, **kwargs):
    result = subprocess.run(args, text=True, capture_output=True, **kwargs)
    if result.returncode:
        raise ValueError(f"{args[0]} failed: {result.stderr.strip()}")
    return result.stdout


def load_sources():
    spec = importlib.util.spec_from_file_location("trainer_scaling_parser", GAME / "tools/trainer_scaling/generate.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    sources = {}
    with tempfile.TemporaryDirectory(prefix="trainer-balance-catalog-") as directory:
        directory = Path(directory)
        binary = directory / "trainerproc"
        command(["cc", "-O2", str(GAME / "tools/trainerproc/main.c"), "-o", str(binary)])
        for label, filename in [("FRLG", "trainers_frlg.party"), ("HNS", "trainers_hns.party"), ("Emerald", "trainers.party")]:
            # Standalone Emerald preserves WAYFARER_LEAGUE_LEVEL's original
            # Emerald input rather than the active Wayfarer League override.
            defines = ["-DPOKEMON_EMERALD", "-DIS_EMERALD=1", "-DIS_HNS=0", "-DIS_FRLG=0", "-DIS_WAYFARER=0"] if label == "Emerald" else ["-DPOKEMON_HNS", "-DIS_HNS=1", "-DIS_EMERALD=0", "-DIS_FRLG=0", "-DIS_WAYFARER=0"]
            source = GAME / "src/data" / filename
            preprocessed = command(["cpp", "-traditional-cpp", "-P", *defines, "-I", str(GAME / "include"), str(source)])
            header = directory / (filename + ".h")
            command([str(binary), "-i", f"src/data/{filename}", "-o", str(header), "-"], input=preprocessed)
            sources[label] = module.parse_output(header.read_text(), f"game/src/data/{filename}")
    return sources


def load_evolutions():
    """Map each evolved species token to its pre-evolution token.

    Reads the first `.evolutions = EVOLUTION(...)` entry that produces each
    species from game/src/data/pokemon/species_info. Only the relationship is
    used, to tell whether a roster line already covers an early species.
    """
    previous = {}
    for path in sorted(SPECIES_INFO.glob("gen_*_families.h")):
        text = path.read_text()
        heads = list(re.finditer(r"^\s*\[(SPECIES_\w+)\]\s*=", text, re.M))
        for index, head in enumerate(heads):
            end = heads[index + 1].start() if index + 1 < len(heads) else len(text)
            block = re.search(r"\.evolutions\s*=\s*EVOLUTION\((.*?)\),\s*\n", text[head.end():end], re.S)
            if not block:
                continue
            for entry in re.finditer(r"\{(EVO_\w+),\s*([^,]+),\s*(SPECIES_\w+)", block.group(1)):
                previous.setdefault(entry.group(3), head.group(1))
    if not previous:
        raise ValueError("no evolutions found in species_info")
    return previous


def display(value, prefix):
    if not isinstance(value, str) or not value.startswith(prefix):
        raise ValueError(f"invalid source token: {value!r}")
    text = value[len(prefix):].replace("_", " ").title()
    return {"Mr Mime": "Mr. Mime", "Farfetchd": "Farfetch’d", "Ho Oh": "Ho-Oh", "Porygon Z": "Porygon-Z"}.get(text, text)


def species_token(name):
    return "SPECIES_" + name.upper().replace(". ", "_").replace(" ", "_").replace("’", "").replace("-", "_")


def slug(name):
    return re.sub(r"[^a-z0-9]+", "-", name.lower()).strip("-")


def member(slot):
    level = slot.get("lvl")
    if not isinstance(level, int) or not 1 <= level <= 100:
        raise ValueError(f"invalid authored level: {level!r}")
    moves = slot.get("moves", [])
    if not isinstance(moves, list) or len(moves) > 4:
        raise ValueError("invalid authored moves")
    item = slot.get("heldItem", "ITEM_NONE")
    return {"species": display(slot.get("species"), "SPECIES_"), "level": level,
            "moves": [display(move, "MOVE_") for move in moves if move != "MOVE_NONE"],
            "item": None if item == "ITEM_NONE" else display(item, "ITEM_")}


def party(records, trainer):
    if trainer not in records:
        raise ValueError(f"missing source trainer: {trainer}")
    record = records[trainer]
    if record.get("overrideTrainer"):
        raise ValueError(f"reference selection needs explicit override resolution: {trainer}")
    slots = record["slots"]
    size = record.get("partySize")
    if not isinstance(size, int) or not 1 <= size <= 6 or size != len(slots):
        raise ValueError(f"invalid source party size: {trainer}")
    return slots


def line_species(token, evolutions):
    """Display names of the species line ending at `token` (babies excluded)."""
    chain = [token]
    while chain[0] in evolutions and evolutions[chain[0]] not in BABIES:
        chain.insert(0, evolutions[chain[0]])
    return {display(species, "SPECIES_") for species in chain}


def optional(value, prefix, empty):
    return display(value, prefix) if value and value != empty else None


def build_roster(members, early, evolutions):
    """Flatten (slot, isAce, levelOffset) members: aces first, then the rest, then early species.

    Mirrors the earlier prototype order (aces, fillers, handwritten early
    fillers not covered by a line). Each roster slot takes the species at the end of
    its line (the source species). Aces keep their source moves, item,
    ability and nature at offset 0; everything else uses LEVEL_UP.
    """
    entries = []
    covered = set()
    for slot, is_ace, offset in sorted(members, key=lambda entry: not entry[1]):
        authored = member(slot)
        covered |= line_species(slot["species"], evolutions)
        entries.append({"species": authored["species"],
                        "levelOffset": 0 if is_ace else max(OFFSET_MIN, min(OFFSET_MAX, offset)),
                        "moves": (authored["moves"] or "LEVEL_UP") if is_ace else "LEVEL_UP",
                        "item": authored["item"] if is_ace else None,
                        "ability": optional(slot.get("ability"), "ABILITY_", "ABILITY_NONE") if is_ace else None,
                        # trainerproc emits NATURE_HARDY when a party omits the nature.
                        "nature": optional(slot.get("nature"), "NATURE_", "NATURE_HARDY") if is_ace else None})
    extras = []
    for name in early:
        if name not in covered:
            token = species_token(name)
            covered |= line_species(token, evolutions)
            extras.append(display(token, "SPECIES_"))
            entries.append({"species": extras[-1], "levelOffset": DEFAULT_OFFSET, "moves": "LEVEL_UP",
                            "item": None, "ability": None, "nature": None})
    return entries[:ROSTER_SIZE], len(entries), [name for name in extras if any(e["species"] == name for e in entries[:ROSTER_SIZE])]


def validate_roster(name, roster):
    """v0 roster rules. More than six or a bad roster slot fails; fewer than six only warns."""
    if not 1 <= len(roster) <= ROSTER_SIZE:
        raise ValueError(f"{name}: a roster lists 1-{ROSTER_SIZE} Pokémon")
    for index, entry in enumerate(roster):
        where = f"{name}[{index + 1}]"
        if not isinstance(entry["species"], str) or not entry["species"].strip():
            raise ValueError(f"{where}: species is required")
        if not isinstance(entry["levelOffset"], int) or not OFFSET_MIN <= entry["levelOffset"] <= OFFSET_MAX:
            raise ValueError(f"{where}: levelOffset must be an integer from {OFFSET_MIN} to {OFFSET_MAX}")
        moves = entry["moves"]
        if moves != "LEVEL_UP" and not (isinstance(moves, list) and 1 <= len(moves) <= 4):
            raise ValueError(f"{where}: moves must be LEVEL_UP or 1-4 authored moves")
    if roster[0]["levelOffset"] != 0:
        raise ValueError(f"{name}: roster slot 1 must have level offset 0")


def generate():
    sources = load_sources()
    evolutions = load_evolutions()
    curated = json.loads(GYMS.read_text())
    if curated.get("version") != 1:
        raise ValueError("unsupported gym catalog")
    result = []
    gaps = []
    for name, region, role, family, trainer, old_tr, early in ROSTER:
        tr, conversion = new_scale_tr(role, old_tr)
        if not isinstance(tr, int) or tr < 0:
            raise ValueError(f"TR must be a non-negative integer: {name}")
        records = sources[family]
        reference_slots = party(records, trainer)
        note = "Local authored reference party; moves and held items are comparison metadata only. All explorer defaults are experimental, not actual ROM teams."
        if family == "HNS":
            note += " HNS local variant, not a HeartGold/SoulSilver canonical party."
        if name == "Blue":
            note += " FRLG first Champion Squirtle starter branch selected explicitly."
        if name in ["Koga", "Will", "Karen"]:
            note += " This stronger HNS party is comparison evidence; explorer levels follow TR, not these authored levels."
        if name == "Steven":
            note += " Emerald optional late battle (levels 75–78), not the Ruby/Sapphire Champion party. Explorer levels follow TR, not these authored levels."
        curated_roster = next((row for row in curated["rosters"] if row["identity"] == name), None)
        if curated_roster:
            members = []
            provenance = []
            for entry in sorted(curated_roster["members"], key=lambda entry: entry["battleOrder"]):
                owner, index = entry["sourceOwner"], entry["sourceSlot"]
                source_family = "HNS" if owner.endswith("_HNS") else "Emerald"
                source = sources[source_family].get(owner)
                if source is None or not 0 <= index < len(source["slots"]):
                    raise ValueError(f"missing curated source slot: {owner}:{index}")
                slot = source["slots"][index]
                if slot["species"] != entry["species"] or slot.get("moves", []) != entry["moves"] or slot.get("heldItem", "ITEM_NONE") != entry["item"]:
                    raise ValueError(f"stale curated species/moves/item: {owner}:{index}")
                members.append((slot, bool(entry["isAce"]), entry["levelOffset"]))
                provenance.append(f"{source['source']}:{owner}[{index}]")
            if len(members) != 6 or sum(is_ace for _, is_ace, _ in members) != 1:
                raise ValueError(f"curated six-slot party with one ace required: {name}")
            origin = "the curated six-slot composition in game/src/data/trainer_scaling/gym_leaders.json (signature Pokémon first, then battle order; its level offsets clamped to -6..0): " + "; ".join(provenance)
        else:
            top = max(slot["lvl"] for slot in reference_slots)
            ace_index = max(index for index, slot in enumerate(reference_slots) if slot["lvl"] == top)
            members = [(slot, index == ace_index, slot["lvl"] - top) for index, slot in enumerate(reference_slots)]
            origin = f"{records[trainer]['source']}:{trainer} (roster slot 1 = highest-level member, last on ties; other offsets = source level − its level, clamped to -6..0)"
        roster, total, extras = build_roster(members, early, evolutions)
        if name == "Blue":
            # One fixed six regardless of the player's starter: the source starter is replaced.
            roster[0] = {"species": "Eevee", "levelOffset": 0, "moves": "LEVEL_UP",
                         "item": None, "ability": None, "nature": None}
        validate_roster(name, roster)
        if len(roster) < ROSTER_SIZE:
            gaps.append(f"{name} {len(roster)}/{ROSTER_SIZE}")
        roster_source = (f"PLACEHOLDER roster flattened from the earlier prototype, derived from {origin}."
                         " Roster slot 1 keeps the source moves/item/ability/nature at offset 0; other roster slots use LEVEL_UP"
                         " (a label: level-up learnsets are not resolved)."
                         + (f" Handwritten early species {', '.join(extras)} fill the remaining roster slots at offset {DEFAULT_OFFSET}." if extras else "")
                         + (f" The flattened list had {total} Pokémon; only the first {ROSTER_SIZE} are kept." if total > ROSTER_SIZE else "")
                         + (" Roster slot 1: starter replaced by Eevee for now (user), LEVEL_UP with no item." if name == "Blue" else ""))
        result.append({"id": slug(name), "name": name, "region": region, "role": role,
                       "source": {"label": f"{family} local reference", "path": records[trainer]["source"], "trainerId": trainer, "note": note},
                       "referenceParty": [member(slot) for slot in reference_slots],
                       "tr": tr,
                       "trSource": f"PLACEHOLDER TR on the rescaled player TR scale (24 badges = TR 160): old-scale TR {old_tr} (suggestedStartTR, first explorer catalog, commit c3c83991ec) converted {conversion}.",
                       "roster": roster, "rosterSource": roster_source})
    if len(result) != 37 or len({row["id"] for row in result}) != 37:
        raise ValueError("catalog must contain exactly 37 unique trainers")
    return json.dumps(result, indent=2, ensure_ascii=False) + "\n", gaps


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if the committed catalog differs from local source extraction")
    args = parser.parse_args()
    try:
        content, gaps = generate()
        if args.check:
            # UI formatting owns whitespace; check the complete extracted data.
            canonical = lambda text: json.dumps(json.loads(text), sort_keys=True)
            if not OUTPUT.is_file() or canonical(OUTPUT.read_text()) != canonical(content):
                raise ValueError("stale catalog.json; run devtools/scripts/trainer-balance-catalog.py")
        else:
            OUTPUT.parent.mkdir(parents=True, exist_ok=True)
            OUTPUT.write_text(content)
        print(f"37 experimental trainers: {'checked' if args.check else 'generated'} {OUTPUT.relative_to(ROOT)}")
        if gaps:
            # v0 requires six roster slots, but rosters are authored next: warn, don't fail.
            print(f"warning: {len(gaps)} placeholder rosters have fewer than {ROSTER_SIZE} Pokémon: {', '.join(gaps)}", file=sys.stderr)
    except (ValueError, OSError, KeyError, IndexError) as error:
        parser.exit(1, f"trainer balance catalog: {error}\n")


if __name__ == "__main__":
    main()
