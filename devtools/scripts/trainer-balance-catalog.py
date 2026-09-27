#!/usr/bin/env python3
"""Regenerate the experimental explorer catalog from local trainerproc sources.

No network, ROM build, or temporary calibration files are required. Reference
moves/items describe authored sources only. Each trainer gets a PROTOTYPE
Living Rivals roster (contract section 9: aces and a filler pool) derived from
its competitive or reference party; real rosters are authored later.
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
# Prototype roster defaults (contract section 9). Fillers are authored later.
PROTOTYPE_BASE_SCORE = 50
DEFAULT_FILLER_OFFSET = -2
# Evolutions without a level (stones, trades, friendship) get a provisional
# authored level: max(NON_LEVEL_EVOLVE, previous stage level + NON_LEVEL_STEP).
NON_LEVEL_EVOLVE = 32
NON_LEVEL_STEP = 10
# Baby pre-evolutions are not added to prototype lines.
BABIES = {"SPECIES_PICHU", "SPECIES_CLEFFA", "SPECIES_IGGLYBUFF", "SPECIES_TYROGUE", "SPECIES_SMOOCHUM",
          "SPECIES_ELEKID", "SPECIES_MAGBY", "SPECIES_AZURILL", "SPECIES_WYNAUT", "SPECIES_BUDEW",
          "SPECIES_CHINGLING", "SPECIES_BONSLY", "SPECIES_MIME_JR", "SPECIES_HAPPINY", "SPECIES_MUNCHLAX",
          "SPECIES_MANTYKE", "SPECIES_RIOLU", "SPECIES_TOXEL"}
# Canonical growth-arc order (arc id = index). allowedArcs are stored in this
# order so source reordering never changes a seeded arc choice.
ARCS = ["steady", "early", "late", "plateau", "rival"]
# name, region, role, reference family/ID, home leagues (Indigo/Hoenn only;
# Sevii Masters is an open invitational), standing bias, allowed growth arcs,
# handwritten early species (added as extra prototype fillers when no
# roster line already covers them). Bias/arcs are Living Rivals D3 proposals:
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


def load_evolutions():
    """Map each evolved species token to (pre-evolution, method, parameter).

    Reads the first `.evolutions = EVOLUTION(...)` entry that produces each
    species from game/src/data/pokemon/species_info. Only the relationship
    and EVO_LEVEL thresholds are used; conditions are ignored.
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
                previous.setdefault(entry.group(3), (head.group(1), entry.group(1), entry.group(2).strip()))
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


def evolution_line(token, evolutions):
    """Species line ending at `token`, as [(species token, level reached)]."""
    chain = [token]
    methods = []
    while chain[0] in evolutions and evolutions[chain[0]][0] not in BABIES:
        source, method, parameter = evolutions[chain[0]]
        chain.insert(0, source)
        methods.insert(0, (method, parameter))
    line = [(chain[0], 1)]
    for species, (method, parameter) in zip(chain[1:], methods):
        previous = line[-1][1]
        level = int(parameter) if method == "EVO_LEVEL" and parameter.isdigit() and int(parameter) > 0 else 0
        if level <= previous:
            level = max(NON_LEVEL_EVOLVE, previous + NON_LEVEL_STEP)
        line.append((species, level))
    return line


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


def ace_entry(slot, evolutions):
    """An ace: its species line, with the source moves/item/ability/nature on the final form."""
    authored = member(slot)
    ability = slot.get("ability")
    nature = slot.get("nature")
    line = []
    for token, level in evolution_line(slot["species"], evolutions):
        final = token == slot["species"]
        line.append({"species": display(token, "SPECIES_"), "level": level,
                     "moves": authored["moves"] if final else [],
                     "item": authored["item"] if final else None,
                     "ability": display(ability, "ABILITY_") if final and ability and ability != "ABILITY_NONE" else None,
                     # trainerproc emits NATURE_HARDY when a party omits the nature.
                     "nature": display(nature, "NATURE_") if final and nature and nature != "NATURE_HARDY" else None})
    return {"id": slug(authored["species"]), "line": line}


def filler_entry(token, offset, evolutions, ids):
    base = slug(display(token, "SPECIES_"))
    identifier, count = base, 1
    while identifier in ids:
        count += 1
        identifier = f"{base}-{count}"
    ids.add(identifier)
    return {"id": identifier,
            "line": [{"species": display(species, "SPECIES_"), "level": level} for species, level in evolution_line(token, evolutions)],
            "baseScore": PROTOTYPE_BASE_SCORE, "levelOffset": max(-6, min(0, offset)), "moves": "LEVEL_UP",
            "signatureMove": None, "requiresFlag": None}


def build_roster(members, early, evolutions):
    """Prototype roster from (slot, isAce, levelOffset) members plus early species."""
    aces = [ace_entry(slot, evolutions) for slot, is_ace, _ in members if is_ace]
    ids = {ace["id"] for ace in aces}
    fillers = [filler_entry(slot["species"], offset, evolutions, ids) for slot, is_ace, offset in members if not is_ace]
    covered = {stage["species"] for entry in aces + fillers for stage in entry["line"]}
    for name in early:
        if name not in covered:
            fillers.append(filler_entry(species_token(name), DEFAULT_FILLER_OFFSET, evolutions, ids))
            covered.update(stage["species"] for stage in fillers[-1]["line"])
    return {"prototype": True, "aces": aces, "fillers": fillers}


def validate_line(line, where, authored):
    if not isinstance(line, list) or not line:
        raise ValueError(f"{where}: line needs at least one species")
    if line[0]["level"] != 1:
        raise ValueError(f"{where}: the first species of a line starts at level 1")
    for previous, stage in zip(line, line[1:]):
        if not isinstance(stage["level"], int) or not previous["level"] < stage["level"] <= 100:
            raise ValueError(f"{where}: evolve levels must increase within 2-100")
    if len({stage["species"] for stage in line}) != len(line):
        raise ValueError(f"{where}: line repeats a species")
    if authored and any(len(stage["moves"]) > 4 for stage in line):
        raise ValueError(f"{where}: at most four authored moves per form")


def validate_roster(name, roster):
    aces, fillers = roster["aces"], roster["fillers"]
    if not 1 <= len(aces) <= 3:
        raise ValueError(f"{name}: a roster needs 1-3 aces")
    ids = [entry["id"] for entry in aces + fillers]
    if len(set(ids)) != len(ids) or not all(re.fullmatch(r"[a-z0-9-]+", i) for i in ids):
        raise ValueError(f"{name}: ace and filler IDs must be unique lowercase slugs")
    for ace in aces:
        validate_line(ace["line"], f"{name}/{ace['id']}", True)
    for filler in fillers:
        validate_line(filler["line"], f"{name}/{filler['id']}", False)
        if not isinstance(filler["levelOffset"], int) or not -6 <= filler["levelOffset"] <= 0:
            raise ValueError(f"{name}/{filler['id']}: levelOffset must be an integer from -6 to 0")
        if not isinstance(filler["baseScore"], int) or not 0 <= filler["baseScore"] <= 100:
            raise ValueError(f"{name}/{filler['id']}: baseScore must be an integer from 0 to 100")
        if filler["moves"] != "LEVEL_UP" or filler["requiresFlag"] not in (None,) and not str(filler["requiresFlag"]).strip():
            raise ValueError(f"{name}/{filler['id']}: invalid moves policy or requiresFlag")


def max_size(roster):
    """Members at size 6 (ace allowance 3) with no gameplay flags set."""
    aces = min(3, len(roster["aces"]))
    unlocked = sum(1 for filler in roster["fillers"] if filler["requiresFlag"] is None)
    return aces + min(unlocked, 6 - aces)


def generate():
    sources = load_sources()
    evolutions = load_evolutions()
    curated = json.loads(GYMS.read_text())
    if curated.get("version") != 1:
        raise ValueError("unsupported gym catalog")
    result = []
    gaps = []
    for name, region, role, family, trainer, homes, bias, arcs, early in ROSTER:
        if not isinstance(bias, int) or not -6 <= bias <= 6:
            raise ValueError(f"bias must be an integer from -6 to 6: {name}")
        if not 2 <= len(arcs) <= 3 or arcs != sorted(set(arcs), key=ARCS.index):
            raise ValueError(f"allowed arcs must be 2-3 unique arcs in canonical order: {name}")
        if not set(homes) <= {"Indigo", "Hoenn"}:
            raise ValueError(f"home leagues are Indigo/Hoenn only; Sevii Masters is open: {name}")
        records = sources[family]
        reference_slots = party(records, trainer)
        note = "Local authored reference party; moves and held items are comparison metadata only. All explorer defaults are experimental, not actual ROM teams."
        if family == "HNS":
            note += " HNS local variant, not a HeartGold/SoulSilver canonical party."
        if name == "Blue":
            note += " FRLG first Champion Squirtle starter branch selected explicitly."
            note += " Standing bias +1 with only fast arcs (early, rival) keeps him a few levels above the world cap wherever it is at most 96 (all of edition 1). At the level ceiling the check is against the level base instead."
        if name in ["Koga", "Will", "Karen"]:
            note += " This stronger HNS party is comparison evidence; explorer levels follow the world cap plus standing, not these authored levels."
        if name == "Steven":
            note += " Emerald optional late battle (levels 75–78), not the Ruby/Sapphire Champion party. Explorer levels follow the world cap plus standing, not these authored levels."
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
            origin = "the curated six-slot composition in game/src/data/trainer_scaling/gym_leaders.json (its ace and level offsets, clamped to -6..0): " + "; ".join(provenance)
        else:
            top = max(slot["lvl"] for slot in reference_slots)
            ace_index = max(index for index, slot in enumerate(reference_slots) if slot["lvl"] == top)
            members = [(slot, index == ace_index, slot["lvl"] - top) for index, slot in enumerate(reference_slots)]
            origin = f"{records[trainer]['source']}:{trainer} (ace = highest-level member, last on ties; filler offsets = source level − ace level, clamped to -6..0)"
        roster = build_roster(members, early, evolutions)
        validate_roster(name, roster)
        size = max_size(roster)
        if size < 6:
            gaps.append(f"{name} {size}/6")
        roster_source = (f"PROTOTYPE roster derived from {origin}."
                         f" Every other member is a filler line with baseScore {PROTOTYPE_BASE_SCORE};"
                         + (f" handwritten early species not already on a line ({', '.join(early)}) are extra fillers at offset {DEFAULT_FILLER_OFFSET};" if early else "")
                         + " lines and EVO_LEVEL thresholds come from game/src/data/pokemon/species_info, and stone, trade or"
                         f" friendship evolutions use max({NON_LEVEL_EVOLVE}, previous level + {NON_LEVEL_STEP}). Only the final ace form"
                         " carries source moves/item/ability/nature. Fillers use LEVEL_UP as a label: level-up learnsets are not resolved.")
        result.append({"id": slug(name), "name": name, "region": region, "role": role, "homeLeagues": homes,
                       "source": {"label": f"{family} local reference", "path": records[trainer]["source"], "trainerId": trainer, "note": note},
                       "referenceParty": [member(slot) for slot in reference_slots],
                       "roster": roster, "rosterSource": roster_source, "bias": bias, "allowedArcs": arcs})
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
            # Content gaps for roster authoring, not failures.
            print(f"warning: {len(gaps)} prototype rosters cannot reach 6 at max size: {', '.join(gaps)}", file=sys.stderr)
    except (ValueError, OSError, KeyError, IndexError) as error:
        parser.exit(1, f"trainer balance catalog: {error}\n")


if __name__ == "__main__":
    main()
