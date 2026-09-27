#!/usr/bin/env python3
"""Regenerate the experimental explorer catalog from local trainerproc sources.

No network, ROM build, or temporary calibration files are required. Reference
moves/items describe authored sources only. Each trainer gets placeholder growth
(start TR, archetype, peak TR, and a lead for the rival) and a placeholder v0 roster (one ordered list of up to six roster slots) flattened
from the earlier ace/filler prototype. Real rosters are authored later; a roster
short of six is a content-gap warning, not a failure.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
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
# name, region, role, reference family/ID, handwritten early species (appended
# when no roster line already covers them).
ROSTER = [
    ('Brock', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_BROCK', ['Geodude', 'Onix']),
    ('Misty', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_MISTY', ['Staryu', 'Psyduck']),
    ('Lt. Surge', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_LT_SURGE', ['Voltorb', 'Pikachu']),
    ('Erika', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_ERIKA', ['Oddish', 'Bellsprout']),
    ('Janine', 'Kanto', 'Gym Leader', 'HNS', 'TRAINER_JANINE_HNS', ['Venonat', 'Koffing']),
    ('Sabrina', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_SABRINA', ['Abra', 'Drowzee']),
    ('Blaine', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_BLAINE', ['Growlithe', 'Ponyta']),
    ('Giovanni', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_GIOVANNI', ['Sandshrew', 'Rhyhorn']),
    ('Blue', 'Kanto', 'Champion', 'FRLG', 'TRAINER_CHAMPION_FIRST_SQUIRTLE', ['Pidgey', 'Eevee']),
    ('Lorelei', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_LORELEI', []),
    ('Bruno', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_BRUNO', []),
    ('Agatha', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_AGATHA', []),
    ('Koga', 'Johto', 'Elite Four', 'HNS', 'TRAINER_KOGA_1_HNS', []),
    ('Lance', 'Kanto', 'Champion', 'FRLG', 'TRAINER_ELITE_FOUR_LANCE', []),
    ('Falkner', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_FALKNER_1_HNS', ['Pidgey', 'Hoothoot']),
    ('Bugsy', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_BUGSY_1_HNS', ['Caterpie', 'Weedle']),
    ('Whitney', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_WHITNEY_1_HNS', ['Clefairy', 'Meowth']),
    ('Morty', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_MORTY_1_HNS', ['Gastly', 'Misdreavus']),
    ('Chuck', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_CHUCK_1_HNS', ['Machop', 'Makuhita']),
    ('Jasmine', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_JASMINE_1_HNS', ['Magnemite', 'Aron']),
    ('Pryce', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_PRYCE_1_HNS', ['Seel', 'Swinub']),
    ('Clair', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_CLAIR_1_HNS', ['Dratini', 'Horsea']),
    ('Will', 'Johto', 'Elite Four', 'HNS', 'TRAINER_WILL_1_HNS', []),
    ('Karen', 'Johto', 'Elite Four', 'HNS', 'TRAINER_KAREN_1_HNS', []),
    ('Roxanne', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_ROXANNE_1', ['Geodude', 'Nosepass']),
    ('Brawly', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_BRAWLY_1', ['Machop', 'Makuhita']),
    ('Wattson', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_WATTSON_1', ['Voltorb', 'Electrike']),
    ('Flannery', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_FLANNERY_1', ['Numel', 'Slugma']),
    ('Norman', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_NORMAN_1', ['Slakoth', 'Zigzagoon']),
    ('Winona', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_WINONA_1', ['Swablu', 'Taillow']),
    ('Juan', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_JUAN_1', ['Horsea', 'Barboach']),
    ('Sidney', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_SIDNEY', []),
    ('Phoebe', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_PHOEBE', []),
    ('Glacia', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_GLACIA', []),
    ('Drake', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_DRAKE', []),
    ('Wallace', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_WALLACE', []),
    ('Steven', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_STEVEN', []),
]


# Placeholder growth (contract section 7), tuned in the explorer to the balance
# targets: name -> (start TR, archetype, peak TR, lead). Only the rival has a lead.
# Lore: veterans plateau, rising stars bloom early, the strongest leaders bloom
# late, most others are steady; Champions and Lance get the highest peaks.
ARCHETYPES = ("steady", "early bloomer", "late bloomer", "plateau", "rival")
GROWTH = {
    "Brock": (2, "steady", 95, None), "Misty": (4, "steady", 110, None),
    "Lt. Surge": (6, "steady", 120, None), "Erika": (6, "steady", 150, None),
    "Janine": (8, "early bloomer", 100, None), "Sabrina": (25, "late bloomer", 180, None),
    "Blaine": (15, "plateau", 90, None), "Giovanni": (16, "steady", 170, None),
    "Blue": (10, "rival", 180, 10),
    "Lorelei": (40, "plateau", 92, None), "Bruno": (45, "plateau", 94, None),
    "Agatha": (50, "plateau", 95, None), "Koga": (30, "steady", 150, None),
    "Lance": (48, "late bloomer", 200, None),
    "Falkner": (1, "early bloomer", 80, None), "Bugsy": (3, "steady", 100, None),
    "Whitney": (4, "early bloomer", 95, None), "Morty": (8, "steady", 150, None),
    "Chuck": (12, "plateau", 85, None), "Jasmine": (12, "steady", 172, None),
    "Pryce": (18, "plateau", 92, None), "Clair": (20, "late bloomer", 185, None),
    "Will": (30, "early bloomer", 110, None), "Karen": (30, "steady", 155, None),
    "Roxanne": (2, "steady", 90, None), "Brawly": (4, "early bloomer", 90, None),
    "Wattson": (8, "plateau", 75, None), "Flannery": (6, "early bloomer", 100, None),
    "Norman": (20, "steady", 168, None), "Winona": (15, "late bloomer", 172, None),
    "Juan": (22, "late bloomer", 185, None),
    "Sidney": (25, "early bloomer", 105, None), "Phoebe": (25, "steady", 150, None),
    "Glacia": (40, "plateau", 90, None), "Drake": (45, "plateau", 93, None),
    "Wallace": (48, "late bloomer", 190, None), "Steven": (50, "late bloomer", 195, None),
}
GROWTH_NOTE = {
    "steady": "steady (keeps a fixed fraction of the player's pace)",
    "early bloomer": "early bloomer (a rising star: fast early, then slows)",
    "late bloomer": "late bloomer (a strong leader: slow start, strong finish)",
    "plateau": "plateau (a veteran who reaches their peak early and stops)",
    "rival": "rival (stays a lead ahead of world progress until their peak)",
}


def validate_growth(name, growth):
    """Start and peak TR are non-negative integers, peak >= start; only a rival has a lead."""
    start, archetype, peak, lead = growth
    if archetype not in ARCHETYPES:
        raise ValueError(f"{name}: unknown archetype {archetype!r}")
    for label, value in (("start TR", start), ("peak TR", peak)):
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            raise ValueError(f"{name}: {label} must be a non-negative integer")
    if peak < start:
        raise ValueError(f"{name}: peak TR must be at least start TR")
    if archetype == "rival":
        if not isinstance(lead, int) or isinstance(lead, bool) or lead < 0:
            raise ValueError(f"{name}: a rival needs a non-negative integer lead")
    elif lead is not None:
        raise ValueError(f"{name}: only a rival has a lead")


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
    if set(GROWTH) != {row[0] for row in ROSTER}:
        raise ValueError("GROWTH must list exactly the catalog trainers")
    for name, region, role, family, trainer, early in ROSTER:
        growth = GROWTH[name]
        validate_growth(name, growth)
        start, archetype, peak, lead = growth
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
                       "startTR": start, "archetype": archetype, "peakTR": peak, "lead": lead,
                       "trSource": f"PLACEHOLDER growth tuned in the explorer to the v0 balance targets: start TR {start}, {GROWTH_NOTE[archetype]}, peak TR {peak}" + (f", lead {lead}." if lead is not None else "."),
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
