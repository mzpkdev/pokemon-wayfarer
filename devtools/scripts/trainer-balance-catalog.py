#!/usr/bin/env python3
"""Regenerate the experimental explorer catalog from local trainerproc sources.

No network, ROM build, or temporary calibration files are required. Reference
moves/items describe authored sources only; prototype parties are not ROM teams.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
GAME = ROOT / "game"
OUTPUT = ROOT / "devtools/ui/src/modules/trainer-balance/catalog.json"
GYMS = GAME / "src/data/trainer_scaling/gym_leaders.json"
# Canonical growth-arc order (arc id = index). allowedArcs are stored in this
# order so source reordering never changes a seeded arc choice.
ARCS = ["steady", "early", "late", "plateau", "rival"]
# name, region, role, reference family/ID, home leagues (Indigo/Hoenn only;
# Sevii Masters is an open invitational), standing bias, allowed growth arcs,
# handwritten two-slot early species. Bias/arcs are Living Rivals D3 proposals:
# Gym -2 (strong leaders -1: Giovanni, Sabrina, Morty, Clair, Norman, Winona,
# Juan, so each region has a Gym Leader who can headline in some saves),
# Elite Four 0, Champion +2, Blue +1.
ROSTER = [
    ('Brock', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_BROCK', ['Indigo'], -2, ['steady', 'late'], ['Geodude', 'Onix']),
    ('Misty', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_MISTY', ['Indigo'], -2, ['steady', 'early'], ['Staryu', 'Psyduck']),
    ('Lt. Surge', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_LT_SURGE', ['Indigo'], -2, ['steady', 'plateau'], ['Voltorb', 'Pikachu']),
    ('Erika', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_ERIKA', ['Indigo'], -2, ['steady', 'late'], ['Oddish', 'Bellsprout']),
    ('Janine', 'Kanto', 'Gym Leader', 'HNS', 'TRAINER_JANINE_HNS', ['Indigo'], -2, ['early', 'late'], ['Venonat', 'Koffing']),
    ('Sabrina', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_SABRINA', ['Indigo'], -1, ['steady', 'early'], ['Abra', 'Drowzee']),
    ('Blaine', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_BLAINE', ['Indigo'], -2, ['steady', 'plateau'], ['Growlithe', 'Ponyta']),
    ('Giovanni', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_GIOVANNI', ['Indigo'], -1, ['steady', 'late'], ['Sandshrew', 'Rhyhorn']),
    ('Blue', 'Kanto', 'Champion', 'FRLG', 'TRAINER_CHAMPION_FIRST_SQUIRTLE', ['Indigo'], 1, ['early', 'rival'], ['Pidgey', 'Eevee']),
    ('Lorelei', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_LORELEI', ['Indigo'], 0, ['steady', 'plateau'], []),
    ('Bruno', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_BRUNO', ['Indigo'], 0, ['steady', 'plateau'], []),
    ('Agatha', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_AGATHA', ['Indigo'], 0, ['steady', 'plateau'], []),
    ('Koga', 'Johto', 'Elite Four', 'HNS', 'TRAINER_KOGA_1_HNS', ['Indigo'], 0, ['steady', 'late'], []),
    ('Lance', 'Kanto', 'Champion', 'FRLG', 'TRAINER_ELITE_FOUR_LANCE', ['Indigo'], 2, ['steady', 'rival'], []),
    ('Falkner', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_FALKNER_1_HNS', ['Indigo'], -2, ['steady', 'early'], ['Pidgey', 'Hoothoot']),
    ('Bugsy', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_BUGSY_1_HNS', ['Indigo'], -2, ['early', 'late'], ['Caterpie', 'Weedle']),
    ('Whitney', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_WHITNEY_1_HNS', ['Indigo'], -2, ['early', 'plateau'], ['Clefairy', 'Meowth']),
    ('Morty', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_MORTY_1_HNS', ['Indigo'], -1, ['steady', 'late'], ['Gastly', 'Misdreavus']),
    ('Chuck', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_CHUCK_1_HNS', ['Indigo'], -2, ['steady', 'plateau'], ['Machop', 'Makuhita']),
    ('Jasmine', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_JASMINE_1_HNS', ['Indigo'], -2, ['steady', 'late'], ['Magnemite', 'Aron']),
    ('Pryce', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_PRYCE_1_HNS', ['Indigo'], -2, ['steady', 'plateau'], ['Seel', 'Swinub']),
    ('Clair', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_CLAIR_1_HNS', ['Indigo'], -1, ['steady', 'early', 'rival'], ['Dratini', 'Horsea']),
    ('Will', 'Johto', 'Elite Four', 'HNS', 'TRAINER_WILL_1_HNS', ['Indigo'], 0, ['steady', 'early'], []),
    ('Karen', 'Johto', 'Elite Four', 'HNS', 'TRAINER_KAREN_1_HNS', ['Indigo'], 0, ['steady', 'late'], []),
    ('Roxanne', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_ROXANNE_1', ['Hoenn'], -2, ['early', 'late'], ['Geodude', 'Nosepass']),
    ('Brawly', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_BRAWLY_1', ['Hoenn'], -2, ['steady', 'early'], ['Machop', 'Makuhita']),
    ('Wattson', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_WATTSON_1', ['Hoenn'], -2, ['steady', 'plateau'], ['Voltorb', 'Electrike']),
    ('Flannery', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_FLANNERY_1', ['Hoenn'], -2, ['early', 'late'], ['Numel', 'Slugma']),
    ('Norman', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_NORMAN_1', ['Hoenn'], -1, ['steady', 'late'], ['Slakoth', 'Zigzagoon']),
    ('Winona', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_WINONA_1', ['Hoenn'], -1, ['steady', 'early'], ['Swablu', 'Taillow']),
    ('Juan', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_JUAN_1', ['Hoenn'], -1, ['steady', 'plateau'], ['Horsea', 'Barboach']),
    ('Sidney', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_SIDNEY', ['Hoenn'], 0, ['steady', 'early'], []),
    ('Phoebe', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_PHOEBE', ['Hoenn'], 0, ['early', 'late'], []),
    ('Glacia', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_GLACIA', ['Hoenn'], 0, ['steady', 'plateau'], []),
    ('Drake', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_DRAKE', ['Hoenn'], 0, ['steady', 'plateau'], []),
    ('Wallace', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_WALLACE', ['Hoenn'], 2, ['steady', 'late'], []),
    ('Steven', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_STEVEN', ['Hoenn'], 2, ['steady', 'early'], []),
]


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


def display(value, prefix):
    if not isinstance(value, str) or not value.startswith(prefix):
        raise ValueError(f"invalid source token: {value!r}")
    text = value[len(prefix):].replace("_", " ").title()
    return {"Mr Mime": "Mr. Mime", "Farfetchd": "Farfetch’d", "Ho Oh": "Ho-Oh", "Porygon Z": "Porygon-Z"}.get(text, text)


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
    return [member(slot) for slot in slots]


def generate():
    sources = load_sources()
    curated = json.loads(GYMS.read_text())
    if curated.get("version") != 1:
        raise ValueError("unsupported gym catalog")
    result = []
    for name, region, role, family, trainer, homes, bias, arcs, early in ROSTER:
        if not isinstance(bias, int) or not -6 <= bias <= 6:
            raise ValueError(f"bias must be an integer from -6 to 6: {name}")
        if not 2 <= len(arcs) <= 3 or arcs != sorted(set(arcs), key=ARCS.index):
            raise ValueError(f"allowed arcs must be 2-3 unique arcs in canonical order: {name}")
        if not set(homes) <= {"Indigo", "Hoenn"}:
            raise ValueError(f"home leagues are Indigo/Hoenn only; Sevii Masters is open: {name}")
        records = sources[family]
        reference = party(records, trainer)
        note = "Local authored reference party; moves and held items are comparison metadata only. All explorer defaults are experimental, not actual ROM teams."
        if family == "HNS":
            note += " HNS local variant, not a HeartGold/SoulSilver canonical party."
        if name == "Blue":
            note += " FRLG first Champion Squirtle starter branch selected explicitly; Blue remains Gym eligible in this prototype."
            note += " Standing bias +1 with only fast arcs (early, rival) keeps him a few levels above the world cap wherever it is at most 96 (all of edition 1). At the level ceiling the check is against the level base instead."
        if early:
            note += " Early party is a handwritten two-species PROTOTYPE for the opening two-member Gym stage."
        if name in ["Koga", "Will", "Karen"]:
            note += " This stronger HNS party is comparison evidence; explorer levels follow the world cap plus standing, not these authored levels."
        if name == "Steven":
            note += " Emerald optional late battle (levels 75–78), not the Ruby/Sapphire Champion party. Explorer levels follow the world cap plus standing, not these authored levels."
        roster = next((row for row in curated["rosters"] if row["identity"] == name), None)
        if roster:
            competitive = []
            provenance = []
            for entry in sorted(roster["members"], key=lambda entry: entry["battleOrder"]):
                owner, index = entry["sourceOwner"], entry["sourceSlot"]
                source_family = "HNS" if owner.endswith("_HNS") else "Emerald"
                source = sources[source_family].get(owner)
                if source is None or not 0 <= index < len(source["slots"]):
                    raise ValueError(f"missing curated source slot: {owner}:{index}")
                slot = source["slots"][index]
                if slot["species"] != entry["species"] or slot.get("moves", []) != entry["moves"] or slot.get("heldItem", "ITEM_NONE") != entry["item"]:
                    raise ValueError(f"stale curated species/moves/item: {owner}:{index}")
                competitive.append(member(slot))
                provenance.append(f"{source['source']}:{owner}[{index}] (offset {entry['levelOffset']}, ace {str(entry['isAce']).lower()}, {entry['movePolicy']})")
            if len(competitive) != 6:
                raise ValueError(f"curated six-slot party required: {name}")
            competitive_source = "Experimental six-slot composition from game/src/data/trainer_scaling/gym_leaders.json; authored metadata retained, including low-form species. " + "; ".join(provenance)
        else:
            competitive = reference
            competitive_source = f"Experimental reuse of {records[trainer]['source']}:{trainer}; source species/levels/moves/items retained."
            if len(competitive) < 6:
                competitive_source += " Incomplete final circuit party: source has fewer than six members; no invented sixth slot."
        result.append({"id": name.lower().replace(". ", "-").replace(" ", "-"), "name": name, "region": region, "role": role,
                       "gymEligible": bool(early), "homeLeagues": homes,
                       "source": {"label": f"{family} local reference", "path": records[trainer]["source"], "trainerId": trainer, "note": note},
                       "referenceParty": reference, "competitiveParty": competitive, "competitiveSource": competitive_source,
                       "earlyParty": early, "bias": bias, "allowedArcs": arcs})
    if len(result) != 37 or len({row["id"] for row in result}) != 37:
        raise ValueError("catalog must contain exactly 37 unique trainers")
    return json.dumps(result, indent=2, ensure_ascii=False) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if the committed catalog differs from local source extraction")
    args = parser.parse_args()
    try:
        content = generate()
        if args.check:
            # UI formatting owns whitespace; check the complete extracted data.
            canonical = lambda text: json.dumps(json.loads(text), sort_keys=True)
            if not OUTPUT.is_file() or canonical(OUTPUT.read_text()) != canonical(content):
                raise ValueError("stale catalog.json; run devtools/scripts/trainer-balance-catalog.py")
        else:
            OUTPUT.parent.mkdir(parents=True, exist_ok=True)
            OUTPUT.write_text(content)
        print(f"37 experimental trainers: {'checked' if args.check else 'generated'} {OUTPUT.relative_to(ROOT)}")
    except (ValueError, OSError, KeyError, IndexError) as error:
        parser.exit(1, f"trainer balance catalog: {error}\n")


if __name__ == "__main__":
    main()
