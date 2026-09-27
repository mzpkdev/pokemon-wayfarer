#!/usr/bin/env python3
"""Regenerate the experimental explorer catalog from local trainerproc sources.

No network, ROM build, or temporary calibration files are required. Reference
moves/items describe authored sources only. Each trainer gets placeholder growth
(start TR, archetype, peak TR) and a placeholder v0 roster (one ordered list of up to six roster slots) flattened
from the earlier ace/filler prototype, converted to final stages (contract
section 9). Real rosters are authored later; a roster short of six or a slot that
is not a final stage is a warning, not a failure.

The catalog also records each roster species' predecessor chain with evolution
levels: species_info EVO_LEVEL thresholds, or the shared evolution-level table
(EVOLUTION_LEVELS) for non-level evolutions. The explorer steps a member down
that chain until its level supports the stage.
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
SPECIES_CONSTANTS = GAME / "include/constants/species.h"
ROSTER_SIZE = 6
CATALOG_SIZE = 38
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
    ('Tate & Liza', 'Hoenn', 'Gym Leader duo', 'Emerald', 'TRAINER_TATE_AND_LIZA_1', []),
    ('Juan', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_JUAN_1', ['Horsea', 'Barboach']),
    ('Sidney', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_SIDNEY', []),
    ('Phoebe', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_PHOEBE', []),
    ('Glacia', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_GLACIA', []),
    ('Drake', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_DRAKE', []),
    ('Wallace', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_WALLACE', []),
    ('Steven', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_STEVEN', []),
]


# Placeholder growth (contract sections 7 and 8), tuned in the explorer to the
# balance targets: name -> (start TR, archetype, peak TR). Every archetype,
# rival included, is a growth scaler. Lore: veterans plateau, rising stars bloom
# early, the strongest leaders bloom late, most others are steady; Champions and
# Lance get the highest peaks. Early Gym openers start high enough that their
# opening teams stay classic-like on the team level scaler's low end.
ARCHETYPES = ("steady", "early bloomer", "late bloomer", "plateau", "rival")
GROWTH = {
    "Brock": (20, "steady", 95), "Misty": (4, "steady", 110),
    "Lt. Surge": (6, "steady", 120), "Erika": (6, "steady", 150),
    "Janine": (8, "early bloomer", 100), "Sabrina": (25, "late bloomer", 180),
    "Blaine": (15, "plateau", 90), "Giovanni": (16, "steady", 170),
    "Blue": (0, "rival", 170),
    "Lorelei": (40, "plateau", 92), "Bruno": (45, "plateau", 94),
    "Agatha": (50, "plateau", 95), "Koga": (30, "steady", 150),
    "Lance": (48, "late bloomer", 200),
    "Falkner": (14, "early bloomer", 80), "Bugsy": (3, "steady", 100),
    "Whitney": (4, "early bloomer", 95), "Morty": (8, "steady", 150),
    "Chuck": (12, "plateau", 85), "Jasmine": (12, "steady", 172),
    "Pryce": (18, "plateau", 92), "Clair": (12, "late bloomer", 185),
    "Will": (30, "early bloomer", 110), "Karen": (30, "steady", 155),
    "Roxanne": (20, "steady", 90), "Brawly": (4, "early bloomer", 90),
    "Wattson": (8, "plateau", 75), "Flannery": (6, "early bloomer", 100),
    "Norman": (20, "steady", 168), "Winona": (10, "late bloomer", 172),
    "Tate & Liza": (22, "steady", 160), "Juan": (22, "late bloomer", 185),
    "Sidney": (25, "early bloomer", 105), "Phoebe": (25, "steady", 150),
    "Glacia": (40, "plateau", 90), "Drake": (45, "plateau", 93),
    "Wallace": (48, "late bloomer", 190), "Steven": (50, "late bloomer", 195),
}
# The Gym Leader duo: one entry fought as a double battle, league-ineligible.
# Roster (Emerald source slots): the Solrock/Lunatone signature pair at offset 0
# with their source battle content, then PLACEHOLDER Hoenn Psychic/Rock picks.
DUOS = {"Tate & Liza": [("TRAINER_TATE_AND_LIZA_1", 3, "SPECIES_SOLROCK", True),
                        ("TRAINER_TATE_AND_LIZA_1", 2, "SPECIES_LUNATONE", True),
                        ("TRAINER_TATE_AND_LIZA_1", 0, "SPECIES_CLAYDOL", False),
                        ("TRAINER_TATE_AND_LIZA_1", 1, "SPECIES_XATU", False),
                        ("TRAINER_KATELYNN", 0, "SPECIES_GARDEVOIR", False),
                        ("TRAINER_VALERIE_5", 2, "SPECIES_GRUMPIG", False)]}
# Section 9 shared evolution-level table: (predecessor, species, level, status).
# The table covers only evolutions without a level in the game data (item,
# trade, friendship, other); they step down below this level like level
# evolutions. Level evolutions use the game's own species_info EVO_LEVEL, so a
# row for an edge that already has one is rejected. "authored" rows are the
# contract examples; "placeholder" rows cover the other non-level edges the
# rosters need and await content review.
EVOLUTION_LEVELS = [
    ("Onix", "Steelix", 35, "authored"),
    ("Staryu", "Starmie", 30, "authored"),
    ("Growlithe", "Arcanine", 35, "authored"),
    ("Eevee", "Vaporeon", 30, "placeholder"),
    ("Eevee", "Espeon", 30, "placeholder"),
    ("Eevee", "Umbreon", 30, "placeholder"),
    ("Pikachu", "Raichu", 30, "placeholder"),
    ("Feebas", "Milotic", 30, "placeholder"),
    ("Skitty", "Delcatty", 30, "placeholder"),
    ("Dunsparce", "Dudunsparce", 32, "placeholder"),
    ("Exeggcute", "Exeggutor", 34, "placeholder"),
    ("Shellder", "Cloyster", 34, "placeholder"),
    ("Clefairy", "Clefable", 36, "placeholder"),
    ("Gloom", "Vileplume", 36, "placeholder"),
    ("Gloom", "Bellossom", 36, "placeholder"),
    ("Weepinbell", "Victreebel", 36, "placeholder"),
    ("Nidorina", "Nidoqueen", 36, "placeholder"),
    ("Nidorino", "Nidoking", 36, "placeholder"),
    ("Poliwhirl", "Poliwrath", 36, "placeholder"),
    ("Roselia", "Roserade", 36, "placeholder"),
    ("Yanma", "Yanmega", 36, "placeholder"),
    ("Misdreavus", "Mismagius", 36, "placeholder"),
    ("Murkrow", "Honchkrow", 36, "placeholder"),
    ("Nuzleaf", "Shiftry", 36, "placeholder"),
    ("Lombre", "Ludicolo", 36, "placeholder"),
    ("Slowpoke", "Slowking", 37, "placeholder"),
    ("Golbat", "Crobat", 38, "placeholder"),
    ("Sneasel", "Weavile", 38, "placeholder"),
    ("Scyther", "Scizor", 40, "placeholder"),
    ("Tangela", "Tangrowth", 40, "placeholder"),
    ("Chansey", "Blissey", 40, "placeholder"),
    ("Stantler", "Wyrdeer", 40, "placeholder"),
    ("Girafarig", "Farigiraf", 40, "placeholder"),
    ("Nosepass", "Probopass", 40, "placeholder"),
    ("Snorunt", "Froslass", 42, "placeholder"),
    ("Magneton", "Magnezone", 45, "placeholder"),
    ("Electabuzz", "Electivire", 45, "placeholder"),
    ("Magmar", "Magmortar", 45, "placeholder"),
    ("Primeape", "Annihilape", 45, "placeholder"),
    ("Piloswine", "Mamoswine", 45, "placeholder"),
    ("Seadra", "Kingdra", 45, "placeholder"),
    ("Ursaring", "Ursaluna", 50, "placeholder"),
    ("Dusclops", "Dusknoir", 50, "placeholder"),
    ("Rhydon", "Rhyperior", 55, "placeholder"),
]
# Branching lines where a placeholder species is not already final: the canonical final stage.
FINAL_CHOICE = {"Scyther": "Scizor", "Ursaring": "Ursaluna"}
GROWTH_NOTE = {
    "steady": "steady (keeps a fixed fraction of the player's pace)",
    "early bloomer": "early bloomer (a rising star: fast early, then slows)",
    "late bloomer": "late bloomer (a strong leader: slow start, strong finish)",
    "plateau": "plateau (a veteran who reaches their peak early and stops)",
    "rival": "rival (starts at 0 and stays about 10 ahead of the player from 4 badges)",
}


def validate_growth(name, growth):
    """Start and peak TR are non-negative integers with peak >= start."""
    start, archetype, peak = growth
    if archetype not in ARCHETYPES:
        raise ValueError(f"{name}: unknown archetype {archetype!r}")
    for label, value in (("start TR", start), ("peak TR", peak)):
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            raise ValueError(f"{name}: {label} must be a non-negative integer")
    if peak < start:
        raise ValueError(f"{name}: peak TR must be at least start TR")


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
    """Read every evolution edge from game/src/data/pokemon/species_info.

    Returns (edges, previous): edges maps (predecessor, species) tokens to the
    lowest numeric EVO_LEVEL threshold between them, or None when the edge has
    only non-level methods (item, trade, friendship, level 0 with conditions,
    other). previous maps each evolved species token to its predecessor tokens.
    A default form (e.g. SPECIES_DUDUNSPARCE_TWO_SEGMENT) is read as its base
    name (SPECIES_DUDUNSPARCE), the token rosters use.
    """
    base = {form: name for name, form in re.findall(r"^#define\s+(SPECIES_\w+)\s+(SPECIES_\w+)\s*$",
                                                     SPECIES_CONSTANTS.read_text(), re.M)}
    edges = {}
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
                method, parameter = entry.group(1), entry.group(2).strip()
                source, target = base.get(head.group(1), head.group(1)), base.get(entry.group(3), entry.group(3))
                key = (source, target)
                level = int(parameter) if method == "EVO_LEVEL" and parameter.isdigit() and int(parameter) > 0 else None
                known = edges.get(key)
                edges[key] = level if known is None else known if level is None else min(known, level)
                previous.setdefault(target, [])
                if source not in previous[target]:
                    previous[target].append(source)
    if not edges:
        raise ValueError("no evolutions found in species_info")
    return edges, previous


def evolution_table(edges):
    """Validate the shared evolution-level table against species_info edges.

    One level per edge, every edge real and without a species_info EVO_LEVEL
    (level evolutions use the game's level), levels 2-100. Returns
    {(predecessor token, species token): (level, placeholder)}.
    """
    table = {}
    for source, target, level, status in EVOLUTION_LEVELS:
        key = (species_token(source), species_token(target))
        where = f"evolution-level table {source}->{target}"
        if key in table:
            raise ValueError(f"{where}: listed more than once (one level per edge)")
        if key not in edges:
            raise ValueError(f"{where}: no such evolution in species_info")
        if edges[key] is not None:
            raise ValueError(f"{where}: already a level evolution in species_info (Lv {edges[key]});"
                             " the table covers only evolutions without a level, so remove the row")
        if not isinstance(level, int) or isinstance(level, bool) or not 2 <= level <= 100:
            raise ValueError(f"{where}: level must be an integer from 2 to 100")
        if status not in ("authored", "placeholder"):
            raise ValueError(f"{where}: status must be authored or placeholder")
        table[key] = (level, status == "placeholder")
    return table


def evolution_chain(token, edges, previous, table):
    """The line ending at `token` as [(species token, evolution level or None)], base first.

    Babies are not stepped down to. A stage's evolution level is the
    species_info EVO_LEVEL threshold, or the shared table's level for a
    non-level edge; a non-level edge missing from the table fails. Levels must
    increase along the line, ancestry must be unambiguous, and a cycle fails.
    """
    chain = [(token, None)]
    seen = {token}
    while True:
        head = chain[0][0]
        sources = [source for source in previous.get(head, []) if source not in BABIES]
        if not sources:
            break
        if len(sources) > 1:
            raise ValueError(f"{display(head, 'SPECIES_')}: ambiguous predecessors {', '.join(sources)}")
        source = sources[0]
        if source in seen:
            raise ValueError(f"{display(head, 'SPECIES_')}: evolution cycle through {display(source, 'SPECIES_')}")
        seen.add(source)
        key = (source, head)
        level = table[key][0] if key in table else edges[key]
        if level is None:
            raise ValueError(f"{display(source, 'SPECIES_')}->{display(head, 'SPECIES_')} is a non-level evolution:"
                             " add it to the shared evolution-level table (EVOLUTION_LEVELS)")
        chain[0] = (head, level)
        chain.insert(0, (source, None))
    levels = [level for _, level in chain[1:]]
    if any(later <= earlier for earlier, later in zip(levels, levels[1:])):
        raise ValueError(f"{display(token, 'SPECIES_')}: evolution levels must increase along the line ({levels})")
    return chain


def final_stage(token, edges):
    """The final stage of the line through `token`; a branch needs a FINAL_CHOICE entry."""
    seen = {token}
    while True:
        targets = sorted({target for source, target in edges if source == token})
        if not targets:
            return token
        name = display(token, "SPECIES_")
        if len(targets) > 1:
            if name not in FINAL_CHOICE:
                raise ValueError(f"{name} branches ({', '.join(targets)}): add a FINAL_CHOICE entry")
            token = species_token(FINAL_CHOICE[name])
            if token not in targets:
                raise ValueError(f"FINAL_CHOICE {name}: {FINAL_CHOICE[name]} is not one of its evolutions")
        else:
            token = targets[0]
        if token in seen:
            raise ValueError(f"{name}: evolution cycle")
        seen.add(token)


def display(value, prefix):
    if not isinstance(value, str) or not value.startswith(prefix):
        raise ValueError(f"invalid source token: {value!r}")
    text = value[len(prefix):].replace("_", " ").title()
    return {"Mr Mime": "Mr. Mime", "Farfetchd": "Farfetch’d", "Ho Oh": "Ho-Oh", "Porygon Z": "Porygon-Z",
            "Nidoran F": "Nidoran♀", "Nidoran M": "Nidoran♂"}.get(text, text)


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


def line_species(token, previous):
    """Display names of the species line ending at `token` (babies excluded)."""
    chain = [token]
    while previous.get(chain[0]) and previous[chain[0]][0] not in BABIES and previous[chain[0]][0] not in chain:
        chain.insert(0, previous[chain[0]][0])
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
    edges, previous = load_evolutions()
    table = evolution_table(edges)
    chains = {}
    conversions = []
    non_final = []
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
        start, archetype, peak = growth
        records = sources[family]
        reference_slots = party(records, trainer)
        note = "Local authored reference party; moves and held items are comparison metadata only. All explorer defaults are experimental, not actual ROM teams."
        if family == "HNS":
            note += " HNS local variant, not a HeartGold/SoulSilver canonical party."
        if name == "Blue":
            note += " FRLG first Champion Squirtle starter branch selected explicitly."
        if name in ["Koga", "Will", "Karen"]:
            note += " This stronger HNS party is comparison evidence; explorer levels follow TR, not these authored levels."
        if name in DUOS:
            note += " Emerald Mossdeep double battle; one shared entry for both leaders, league-ineligible (leagues are singles only)."
        if name == "Steven":
            note += " Emerald optional late battle (levels 75–78), not the Ruby/Sapphire Champion party. Explorer levels follow TR, not these authored levels."
        curated_roster = next((row for row in curated["rosters"] if row["identity"] == name), None)
        if name in DUOS:
            members = []
            provenance = []
            for owner, index, species, is_signature in DUOS[name]:
                source = sources[family].get(owner)
                if source is None or not 0 <= index < len(source["slots"]) or source["slots"][index]["species"] != species:
                    raise ValueError(f"missing or stale duo source slot: {owner}:{index} ({species})")
                members.append((source["slots"][index], is_signature, DEFAULT_OFFSET))
                provenance.append(f"{source['source']}:{owner}[{index}]")
            origin = ("Emerald source slots (roster slots 1-2 are the Solrock and Lunatone signature pair at offset 0;"
                      " roster slots 3-6 are PLACEHOLDER Hoenn Psychic/Rock picks): " + "; ".join(provenance))
        elif curated_roster:
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
        roster, total, extras = build_roster(members, early, previous)
        # A duo is fought as its source double battle; leagues are singles only.
        double = records[trainer].get("battleType") == "TRAINER_BATTLE_TYPE_DOUBLES"
        if double != (name in DUOS) or (role == "Gym Leader duo") != (name in DUOS):
            raise ValueError(f"{name}: only a listed duo is a Gym Leader duo fought as a double battle")
        if name == "Blue":
            # One fixed six regardless of the player's starter: the source starter is replaced.
            roster[0] = {"species": "Eevee", "levelOffset": 0, "moves": "LEVEL_UP",
                         "item": None, "ability": None, "nature": None}
        # Section 9: roster slots author final stages. Convert each placeholder
        # species to the final stage of its line (Blue's Eevee stays, user choice).
        converted = []
        for index, entry in enumerate(roster):
            if name == "Blue" and index == 0:
                continue
            final = display(final_stage(species_token(entry["species"]), edges), "SPECIES_")
            if final != entry["species"]:
                converted.append(f"{entry['species']} -> {final}")
                # Source moves and ability belong to the source species, not the new final stage.
                entry.update(species=final, moves="LEVEL_UP", ability=None)
        if name == "Brock":
            # Brock (user): Steelix is the signature Pokémon and Golem keeps its source
            # battle content in roster slot 2; the rest keeps its order, so Kleavor drops off.
            golem = next(entry for entry in roster if entry["species"] == "Golem")
            golem["levelOffset"] = DEFAULT_OFFSET
            rest = [entry for entry in roster if entry is not golem]
            roster = [{"species": "Steelix", "levelOffset": 0, "moves": "LEVEL_UP", "item": None,
                       "ability": None, "nature": None}, golem, *rest][:ROSTER_SIZE]
        conversions.append((name, converted))
        for index, entry in enumerate(roster):
            token = species_token(entry["species"])
            chain = evolution_chain(token, edges, previous, table)
            if any(source == token for source, _ in edges):
                non_final.append(f"{name}[{index + 1}] {entry['species']}")
            chains[entry["species"]] = [part for position, (species, level) in enumerate(chain)
                                        for part in ([level] if position else []) + [display(species, "SPECIES_")]]
        validate_roster(name, roster)
        if len(roster) < ROSTER_SIZE:
            gaps.append(f"{name} {len(roster)}/{ROSTER_SIZE}")
        signature = ("Roster slots 1-2 keep" if name in DUOS else "Golem (roster slot 2) keeps" if name == "Brock"
                     else "Roster slot 1 keeps")
        roster_source = ((f"PLACEHOLDER duo roster derived from {origin}." if name in DUOS
                          else f"PLACEHOLDER roster flattened from the earlier prototype, derived from {origin}.")
                         + f" {signature} the source moves/item/ability/nature{'' if name == 'Brock' else ' at offset 0'}; other roster slots use LEVEL_UP"
                         " (a label: level-up learnsets are not resolved)."
                         + (f" Handwritten early species {', '.join(extras)} fill the remaining roster slots at offset {DEFAULT_OFFSET}." if extras else "")
                         + (f" The flattened list had {total} Pokémon; only the first {ROSTER_SIZE} are kept." if total > ROSTER_SIZE else "")
                         + (f" Converted to final stages (section 9; converted roster slots use LEVEL_UP with no ability): {', '.join(converted)}." if converted else "")
                         + (" Brock (user): Steelix is roster slot 1 at offset 0 and Golem roster slot 2 keeps the source battle content; Kleavor dropped off." if name == "Brock" else "")
                         + (" Roster slot 1: starter replaced by Eevee for now (user), LEVEL_UP with no item; not a final stage (allowed)." if name == "Blue" else "")
                         + (" Fought as a double battle: both leaders send Pokémon from this one roster in order." if name in DUOS else ""))
        result.append({"id": slug(name), "name": name, "region": region, "role": role,
                       "doubleBattle": double, "leagueEligible": not double,
                       "source": {"label": f"{family} local reference", "path": records[trainer]["source"], "trainerId": trainer, "note": note},
                       "referenceParty": [member(slot) for slot in reference_slots],
                       "startTR": start, "archetype": archetype, "peakTR": peak,
                       "trSource": f"PLACEHOLDER growth tuned in the explorer to the v0 balance targets: start TR {start}, {GROWTH_NOTE[archetype]}, peak TR {peak}.",
                       "roster": roster, "rosterSource": roster_source})
    # 37 characters plus the Tate & Liza duo.
    if len(result) != CATALOG_SIZE or len({row["id"] for row in result}) != CATALOG_SIZE:
        raise ValueError(f"catalog must contain exactly {CATALOG_SIZE} unique entries")
    # Each roster species' predecessor chain, ascending: [base, level, stage 2,
    # level, ..., species]. A stage is kept while the member's level is at least
    # the level before it; otherwise it steps down. Earlier stages in a chain
    # evolve; notFinal lists the roster species that also evolve further.
    evolution = {"chains": dict(sorted(chains.items())),
                 "notFinal": sorted(stage for stage in chains
                                    if any(source == species_token(stage) for source, _ in edges))}
    return json.dumps({"evolution": evolution, "trainers": result}, indent=2, ensure_ascii=False) + "\n", gaps, {
        "conversions": conversions, "non_final": non_final,
        "placeholders": [f"{source}->{target} {level}" for source, target, level, status in EVOLUTION_LEVELS if status == "placeholder"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if the committed catalog differs from local source extraction")
    args = parser.parse_args()
    try:
        content, gaps, report = generate()
        if args.check:
            # UI formatting owns whitespace; check the complete extracted data.
            canonical = lambda text: json.dumps(json.loads(text), sort_keys=True)
            if not OUTPUT.is_file() or canonical(OUTPUT.read_text()) != canonical(content):
                raise ValueError("stale catalog.json; run devtools/scripts/trainer-balance-catalog.py")
        else:
            OUTPUT.parent.mkdir(parents=True, exist_ok=True)
            OUTPUT.write_text(content)
        print(f"{CATALOG_SIZE} experimental notable trainer entries: {'checked' if args.check else 'generated'} {OUTPUT.relative_to(ROOT)}")
        for name, converted in report["conversions"]:
            if converted:
                print(f"converted {name}: {', '.join(converted)}")
        print(f"{len(report['placeholders'])} placeholder evolution levels: {', '.join(report['placeholders'])}")
        if report["non_final"]:
            # Section 9 recommends final stages; any authored stage is allowed.
            print(f"warning: {len(report['non_final'])} roster slots are not final stages: {', '.join(report['non_final'])}", file=sys.stderr)
        if gaps:
            # v0 requires six roster slots, but rosters are authored next: warn, don't fail.
            print(f"warning: {len(gaps)} placeholder rosters have fewer than {ROSTER_SIZE} Pokémon: {', '.join(gaps)}", file=sys.stderr)
    except (ValueError, OSError, KeyError, IndexError) as error:
        parser.exit(1, f"trainer balance catalog: {error}\n")


if __name__ == "__main__":
    main()
