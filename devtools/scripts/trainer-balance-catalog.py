#!/usr/bin/env python3
"""Regenerate the experimental explorer catalog from local trainerproc sources.

No network, ROM build, or temporary calibration files are required. Reference
moves/items describe authored sources only. Each trainer gets placeholder growth
(start TR, archetype, peak TR) and the user-directed roster draft v1 (DRAFT: six
ordered roster slots, identity/anime picks, 1-3 aces). A slot keeps authored
source battle content only when its species is in the trainer's source party;
otherwise it uses LEVEL_UP with no item. A roster short of six or a slot that
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
MAX_ACES = 3
CATALOG_SIZE = 38
OFFSET_MIN, OFFSET_MAX = -6, 0
DEFAULT_OFFSET = -2
# Baby pre-evolutions do not count when checking whether a line covers a species.
BABIES = {"SPECIES_PICHU", "SPECIES_CLEFFA", "SPECIES_IGGLYBUFF", "SPECIES_TYROGUE", "SPECIES_SMOOCHUM",
          "SPECIES_ELEKID", "SPECIES_MAGBY", "SPECIES_AZURILL", "SPECIES_WYNAUT", "SPECIES_BUDEW",
          "SPECIES_CHINGLING", "SPECIES_BONSLY", "SPECIES_MIME_JR", "SPECIES_HAPPINY", "SPECIES_MUNCHLAX",
          "SPECIES_MANTYKE", "SPECIES_RIOLU", "SPECIES_TOXEL"}
# name, region, role, reference family/ID.
ROSTER = [
    ('Brock', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_BROCK'),
    ('Misty', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_MISTY'),
    ('Lt. Surge', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_LT_SURGE'),
    ('Erika', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_ERIKA'),
    ('Janine', 'Kanto', 'Gym Leader', 'HNS', 'TRAINER_JANINE_HNS'),
    ('Sabrina', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_SABRINA'),
    ('Blaine', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_BLAINE'),
    ('Giovanni', 'Kanto', 'Gym Leader', 'FRLG', 'TRAINER_LEADER_GIOVANNI'),
    ('Blue', 'Kanto', 'Champion', 'FRLG', 'TRAINER_CHAMPION_FIRST_SQUIRTLE'),
    ('Lorelei', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_LORELEI'),
    ('Bruno', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_BRUNO'),
    ('Agatha', 'Kanto', 'Elite Four', 'FRLG', 'TRAINER_ELITE_FOUR_AGATHA'),
    ('Koga', 'Johto', 'Elite Four', 'HNS', 'TRAINER_KOGA_1_HNS'),
    ('Lance', 'Kanto', 'Champion', 'FRLG', 'TRAINER_ELITE_FOUR_LANCE'),
    ('Falkner', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_FALKNER_1_HNS'),
    ('Bugsy', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_BUGSY_1_HNS'),
    ('Whitney', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_WHITNEY_1_HNS'),
    ('Morty', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_MORTY_1_HNS'),
    ('Chuck', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_CHUCK_1_HNS'),
    ('Jasmine', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_JASMINE_1_HNS'),
    ('Pryce', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_PRYCE_1_HNS'),
    ('Clair', 'Johto', 'Gym Leader', 'HNS', 'TRAINER_CLAIR_1_HNS'),
    ('Will', 'Johto', 'Elite Four', 'HNS', 'TRAINER_WILL_1_HNS'),
    ('Karen', 'Johto', 'Elite Four', 'HNS', 'TRAINER_KAREN_1_HNS'),
    ('Roxanne', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_ROXANNE_1'),
    ('Brawly', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_BRAWLY_1'),
    ('Wattson', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_WATTSON_1'),
    ('Flannery', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_FLANNERY_1'),
    ('Norman', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_NORMAN_1'),
    ('Winona', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_WINONA_1'),
    ('Tate & Liza', 'Hoenn', 'Gym Leader duo', 'Emerald', 'TRAINER_TATE_AND_LIZA_1'),
    ('Juan', 'Hoenn', 'Gym Leader', 'Emerald', 'TRAINER_JUAN_1'),
    ('Sidney', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_SIDNEY'),
    ('Phoebe', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_PHOEBE'),
    ('Glacia', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_GLACIA'),
    ('Drake', 'Hoenn', 'Elite Four', 'Emerald', 'TRAINER_DRAKE'),
    ('Wallace', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_WALLACE'),
    ('Steven', 'Hoenn', 'Champion', 'Emerald', 'TRAINER_STEVEN'),
]


# Placeholder growth (contract sections 7, 8 and 11), tuned in the explorer to
# the balance targets: name -> (start TR, archetype, peak TR). Every archetype,
# Rival included, is a growth scaler. The lore-based assignments are approved
# (section 11): Veterans peak early, Stars explode mid-journey, Comebacks
# stall and return stronger, Bursts train in jumps, the strongest leaders are
# Sleepers, Agatha is a Legend, most others are Steady; Champions and Lance get
# the highest peaks. Gym Leaders (the duo included) start in the GYM_START_BAND
# by archetype (GYM_ARCHETYPE_BANDS), not by Gym order, varied a little by lore;
# no Gym Leader is a Legend. League-eligible Steadies and Bursts keep start +
# peak <= 190 so they stay at TR 95 or less at world progress 80.
ARCHETYPES = ("steady", "prodigy", "sleeper", "veteran", "rival",
              "legend", "star", "comeback", "burst")
GYM_START_BAND = (18, 40)
GYM_ARCHETYPE_BANDS = {
    "sleeper": (18, 26), "star": (18, 26),
    "prodigy": (22, 30),
    "steady": (24, 34), "burst": (24, 34),
    "veteran": (30, 40), "comeback": (30, 40),
}
GROWTH = {
    "Brock": (25, "steady", 100), "Misty": (26, "star", 110),
    "Lt. Surge": (30, "veteran", 95), "Erika": (28, "steady", 150),
    "Janine": (27, "prodigy", 100), "Sabrina": (24, "sleeper", 180),
    "Blaine": (37, "comeback", 90), "Giovanni": (24, "burst", 166),
    "Blue": (0, "rival", 170),
    "Lorelei": (40, "veteran", 92), "Bruno": (45, "comeback", 94),
    "Agatha": (95, "legend", 95), "Koga": (30, "steady", 150),
    "Lance": (48, "sleeper", 200),
    "Falkner": (22, "prodigy", 80), "Bugsy": (24, "star", 100),
    "Whitney": (26, "star", 95), "Morty": (26, "sleeper", 171),
    "Chuck": (34, "burst", 85), "Jasmine": (24, "steady", 166),
    "Pryce": (40, "comeback", 92), "Clair": (21, "sleeper", 185),
    "Will": (30, "prodigy", 110), "Karen": (30, "steady", 155),
    "Roxanne": (24, "steady", 90), "Brawly": (24, "burst", 90),
    "Wattson": (32, "veteran", 75), "Flannery": (23, "star", 100),
    "Norman": (26, "steady", 164), "Winona": (18, "sleeper", 172),
    "Tate & Liza": (26, "star", 170), "Juan": (23, "sleeper", 185),
    "Sidney": (25, "prodigy", 105), "Phoebe": (25, "steady", 150),
    "Glacia": (40, "veteran", 90), "Drake": (45, "veteran", 93),
    "Wallace": (48, "sleeper", 190), "Steven": (50, "sleeper", 195),
}
# The Gym Leader duo: one entry fought as a double battle, league-ineligible.
DUOS = {"Tate & Liza"}
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
    ("Eevee", "Jolteon", 30, "placeholder"),
    ("Eevee", "Flareon", 30, "placeholder"),
    ("Eevee", "Glaceon", 30, "placeholder"),
    ("Nincada", "Shedinja", 20, "placeholder"),
    ("Kirlia", "Gallade", 30, "placeholder"),
    ("Pikachu", "Raichu", 30, "placeholder"),
    ("Feebas", "Milotic", 30, "placeholder"),
    ("Skitty", "Delcatty", 30, "placeholder"),
    ("Dunsparce", "Dudunsparce", 32, "placeholder"),
    ("Exeggcute", "Exeggutor", 34, "placeholder"),
    ("Shellder", "Cloyster", 34, "placeholder"),
    ("Vulpix", "Ninetales", 35, "placeholder"),
    ("Clefairy", "Clefable", 36, "placeholder"),
    ("Jigglypuff", "Wigglytuff", 36, "placeholder"),
    ("Gloom", "Vileplume", 36, "placeholder"),
    ("Gloom", "Bellossom", 36, "placeholder"),
    ("Weepinbell", "Victreebel", 36, "placeholder"),
    ("Nidorina", "Nidoqueen", 36, "placeholder"),
    ("Nidorino", "Nidoking", 36, "placeholder"),
    ("Poliwhirl", "Poliwrath", 36, "placeholder"),
    ("Poliwhirl", "Politoed", 36, "placeholder"),
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
# The user-directed roster draft v1 (identity/anime picks): name -> six roster
# slots in join order as (species, isAce[, tag]). Roster slot 1 is the signature
# Pokémon (an ace at offset 0); other aces are at offset 0 and fillers at -2.
# Tags record off-type, anime or lore picks. Battle content is a placeholder: a
# slot keeps the authored moves/item/ability/nature of the same species in the
# trainer's source party, otherwise it uses LEVEL_UP with no item.
DRAFT = {
    "Brock": [("Steelix", True), ("Golem", False), ("Crobat", False, "anime Zubat"),
              ("Kabutops", False), ("Omastar", False), ("Aerodactyl", True)],
    "Misty": [("Starmie", True), ("Golduck", False, "anime Psyduck"), ("Gyarados", True),
              ("Politoed", False, "anime Poliwag"), ("Lapras", False, "HGSS"), ("Kingdra", True, "anime Horsea")],
    "Lt. Surge": [("Raichu", True, "anime"), ("Electrode", False), ("Electivire", True),
                  ("Magnezone", False), ("Jolteon", False), ("Luxray", False)],
    "Erika": [("Vileplume", True, "FRLG ace / anime Gloom"), ("Tangrowth", False, "anime Tangela"), ("Victreebel", True),
              ("Jumpluff", False, "HGSS"), ("Parasect", False), ("Bellossom", False)],
    "Janine": [("Venomoth", True), ("Ariados", False), ("Crobat", True),
               ("Weezing", False), ("Toxicroak", False, "ninja theme"), ("Muk", False)],
    "Sabrina": [("Alakazam", True, "anime Kadabra"), ("Mr. Mime", False), ("Espeon", True, "HGSS"),
                ("Gengar", False, "anime Haunter"), ("Hypno", False), ("Venomoth", False, "FRLG")],
    "Blaine": [("Magmortar", True, "anime Magmar"), ("Rapidash", False), ("Arcanine", True),
               ("Ninetales", False, "anime"), ("Magcargo", False, "HGSS"), ("Flareon", False)],
    "Giovanni": [("Rhyperior", True), ("Dugtrio", False), ("Nidoqueen", False),
                 ("Persian", True, "anime"), ("Marowak", False), ("Nidoking", True)],
    "Falkner": [("Pidgeot", True), ("Noctowl", False), ("Dodrio", False),
                ("Skarmory", True), ("Xatu", False), ("Honchkrow", False)],
    "Bugsy": [("Scizor", True, "anime Scyther"), ("Beedrill", False), ("Butterfree", False),
              ("Heracross", True), ("Yanmega", False), ("Ariados", False)],
    "Whitney": [("Miltank", True), ("Clefable", False, "anime Clefairy"), ("Wigglytuff", False),
                ("Ursaring", False), ("Girafarig", False), ("Blissey", True)],
    "Morty": [("Gengar", True), ("Drifblim", False), ("Mismagius", True),
              ("Spiritomb", False), ("Sableye", False), ("Dusknoir", True)],
    "Chuck": [("Poliwrath", True), ("Primeape", False), ("Hitmontop", False),
              ("Machamp", True), ("Heracross", False), ("Lucario", True)],
    "Jasmine": [("Steelix", True), ("Magnezone", False), ("Ampharos", True, "lore: Amphy"),
                ("Skarmory", False), ("Forretress", False), ("Bronzong", False)],
    "Pryce": [("Mamoswine", True, "anime Piloswine"), ("Dewgong", False), ("Cloyster", False),
              ("Weavile", True), ("Jynx", False), ("Abomasnow", False)],
    "Clair": [("Kingdra", True), ("Dragonite", False, "anime Dragonair"), ("Gyarados", False),
              ("Salamence", True), ("Flygon", False), ("Garchomp", True)],
    "Roxanne": [("Probopass", True), ("Sudowoodo", False), ("Relicanth", False),
                ("Golem", False), ("Bastiodon", False), ("Rampardos", True, "fossil lessons")],
    "Brawly": [("Hariyama", True), ("Breloom", False), ("Medicham", True),
               ("Machamp", False), ("Gallade", False), ("Hitmontop", False)],
    "Wattson": [("Manectric", True), ("Electrode", False), ("Magnezone", True),
                ("Lanturn", False), ("Rotom", False), ("Ampharos", False)],
    "Flannery": [("Torkoal", True, "anime"), ("Magcargo", False), ("Camerupt", True),
                 ("Houndoom", False), ("Ninetales", False), ("Rapidash", False)],
    "Norman": [("Slaking", True), ("Linoone", False), ("Spinda", False),
               ("Kangaskhan", True), ("Swellow", False), ("Snorlax", True)],
    "Winona": [("Altaria", True), ("Tropius", False), ("Pelipper", False),
               ("Swellow", False), ("Skarmory", True), ("Staraptor", False)],
    "Tate & Liza": [("Solrock", True), ("Lunatone", True), ("Claydol", False),
                    ("Xatu", False), ("Grumpig", False), ("Gardevoir", True)],
    "Juan": [("Kingdra", True), ("Luvdisc", False, "flamboyance"), ("Whiscash", False),
             ("Walrein", True), ("Crawdaunt", False), ("Politoed", False)],
    "Lorelei": [("Lapras", True), ("Dewgong", False), ("Cloyster", True),
                ("Slowbro", False), ("Glaceon", False), ("Jynx", False)],
    "Bruno": [("Machamp", True), ("Hitmontop", False), ("Hitmonlee", True),
              ("Steelix", False, "FRLG Onix"), ("Hitmonchan", False), ("Primeape", False)],
    "Agatha": [("Gengar", True), ("Arbok", False), ("Crobat", False),
               ("Marowak", False, "lore: Pokemon Tower"), ("Mismagius", False), ("Dusknoir", True)],
    "Koga": [("Crobat", True), ("Ariados", False), ("Forretress", False, "HGSS"),
             ("Muk", False), ("Venomoth", False), ("Weezing", True)],
    "Will": [("Xatu", True), ("Jynx", False), ("Slowbro", True),
             ("Exeggutor", False), ("Bronzong", False), ("Gardevoir", True)],
    "Karen": [("Umbreon", True), ("Vileplume", False, "HGSS"), ("Honchkrow", False),
              ("Gengar", False, "HGSS"), ("Weavile", False), ("Houndoom", True)],
    "Sidney": [("Absol", True), ("Mightyena", False), ("Shiftry", False),
               ("Cacturne", False), ("Crawdaunt", False), ("Sharpedo", True)],
    "Phoebe": [("Dusknoir", True), ("Sableye", False), ("Banette", True),
               ("Froslass", False), ("Drifblim", False), ("Shedinja", False, "gimmick")],
    "Glacia": [("Walrein", True), ("Glalie", False), ("Froslass", False),
               ("Abomasnow", False), ("Weavile", False), ("Walrein", True, "iconic duplicate")],
    "Drake": [("Salamence", True), ("Altaria", False), ("Kingdra", False),
              ("Flygon", True), ("Gyarados", False), ("Dragonite", True)],
    "Lance": [("Dragonite", True), ("Gyarados", False), ("Aerodactyl", False),
              ("Charizard", True, "HGSS"), ("Kingdra", False), ("Dragonite", True, "iconic duplicate")],
    "Wallace": [("Milotic", True), ("Wailord", False), ("Tentacruel", False),
                ("Ludicolo", False), ("Whiscash", False), ("Gyarados", True, "RS team")],
    "Steven": [("Metagross", True), ("Skarmory", False), ("Claydol", False),
               ("Aggron", True), ("Cradily", False), ("Armaldo", False, "RS team")],
    "Blue": [("Umbreon", True, "Eevee early"), ("Pidgeot", False, "games"), ("Alakazam", True),
             ("Nidoking", False), ("Scizor", False), ("Arcanine", True)],
}
DRAFT_NOTE = "user-directed roster draft v1 (identity/anime picks); battle content placeholder"
GROWTH_NOTE = {
    "steady": "a Steady (keeps a fixed fraction of the player's pace)",
    "prodigy": "a Prodigy (brilliant early, then evens out: fast early, then slows)",
    "sleeper": "a Sleeper (underestimated, strong at the end: slow start, strong finish)",
    "veteran": "a Veteran (peaked already, you overtake them: reaches their peak early and stops)",
    "rival": "the Rival (starts at 0 and stays about 10 ahead of the player from 4 badges)",
    "legend": "a Legend (never changes, waits at the top: TR stays at start TR, so peak TR equals start TR)",
    "star": "a Star (explodes mid-journey: slow start, explodes, then levels off)",
    "comeback": "a Comeback (stalls, then returns stronger: fast early, stalls mid-journey, surges late)",
    "burst": "a Burst (trains in jumps at milestones: a step scaler, Steady on average, jumping at 4, 8, 16 and 24 badges)",
}


def validate_growth(name, growth):
    """Start and peak TR are non-negative integers with peak >= start (peak = start for a Legend)."""
    start, archetype, peak = growth
    if archetype not in ARCHETYPES:
        raise ValueError(f"{name}: unknown archetype {archetype!r}")
    for label, value in (("start TR", start), ("peak TR", peak)):
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            raise ValueError(f"{name}: {label} must be a non-negative integer")
    if peak < start:
        raise ValueError(f"{name}: peak TR must be at least start TR")
    if archetype == "legend" and peak != start:
        raise ValueError(f"{name}: a Legend's peak TR must equal start TR")


def validate_gym_start(name, growth):
    """A Gym Leader starts in the Gym band, inside their archetype's sub-band; none is a Legend.

    An archetype without a sub-band (the Rival) uses the whole Gym band.
    """
    start, archetype, _ = growth
    if archetype == "legend":
        raise ValueError(f"{name}: a Gym Leader cannot be a Legend")
    low, high = GYM_ARCHETYPE_BANDS.get(archetype, GYM_START_BAND)
    if not GYM_START_BAND[0] <= start <= GYM_START_BAND[1] or not low <= start <= high:
        raise ValueError(f"{name}: Gym Leader start TR must be in {low}-{high} for {archetype}"
                         f" (Gym band {GYM_START_BAND[0]}-{GYM_START_BAND[1]})")


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


def optional(value, prefix, empty):
    return display(value, prefix) if value and value != empty else None


def known_species():
    """Species tokens defined in species.h that also have a species_info entry."""
    defined = set(re.findall(r"^#define\s+(SPECIES_\w+)\s+\S", SPECIES_CONSTANTS.read_text(), re.M))
    entries = {head for path in SPECIES_INFO.glob("*.h")
               for head in re.findall(r"^\s*\[(SPECIES_\w+)\]\s*=", path.read_text(), re.M)}
    return defined & entries


def draft_roster(name, source_slots, known):
    """The DRAFT roster as catalog roster slots, plus "species (source slot)" for kept content.

    `source_slots` is [(slot, provenance)]. A species must resolve in the game
    data. A slot whose species is in the source party keeps that source slot's
    moves/item/ability/nature (first match); every other slot uses LEVEL_UP
    with no item, ability or nature.
    """
    if name not in DRAFT:
        raise ValueError(f"{name}: add a DRAFT roster")
    roster, kept = [], []
    for index, (species, is_ace, *tag) in enumerate(DRAFT[name]):
        token = species_token(species)
        if token not in known:
            raise ValueError(f"DRAFT {name}[{index + 1}]: species {species!r} does not resolve in species_info/species.h")
        source, where = next(((slot, where) for slot, where in source_slots if slot["species"] == token), (None, None))
        entry = {"species": display(token, "SPECIES_"), "levelOffset": 0 if is_ace else DEFAULT_OFFSET,
                 "isAce": is_ace, "moves": "LEVEL_UP", "item": None, "ability": None, "nature": None}
        if source is not None:
            authored = member(source)
            kept.append(f"{entry['species']} ({where})")
            entry.update(moves=authored["moves"] or "LEVEL_UP", item=authored["item"],
                         ability=optional(source.get("ability"), "ABILITY_", "ABILITY_NONE"),
                         # trainerproc emits NATURE_HARDY when a party omits the nature.
                         nature=optional(source.get("nature"), "NATURE_", "NATURE_HARDY"))
        roster.append(entry)
    return roster, kept


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
        if not isinstance(entry["isAce"], bool):
            raise ValueError(f"{where}: isAce must be true or false")
        moves = entry["moves"]
        if moves != "LEVEL_UP" and not (isinstance(moves, list) and 1 <= len(moves) <= 4):
            raise ValueError(f"{where}: moves must be LEVEL_UP or 1-4 authored moves")
    if roster[0]["levelOffset"] != 0:
        raise ValueError(f"{name}: roster slot 1 must have level offset 0")
    if not roster[0]["isAce"]:
        raise ValueError(f"{name}: roster slot 1 (the signature Pokémon) must be an ace")
    if sum(entry["isAce"] for entry in roster) > MAX_ACES:
        raise ValueError(f"{name}: a roster has 1-{MAX_ACES} aces")


def generate():
    sources = load_sources()
    edges, previous = load_evolutions()
    table = evolution_table(edges)
    known = known_species()
    chains = {}
    kept = []
    non_final = []
    curated = json.loads(GYMS.read_text())
    if curated.get("version") != 1:
        raise ValueError("unsupported gym catalog")
    result = []
    gaps = []
    if set(GROWTH) != {row[0] for row in ROSTER} or set(DRAFT) != set(GROWTH):
        raise ValueError("GROWTH and DRAFT must list exactly the catalog trainers")
    for name, region, role, family, trainer in ROSTER:
        growth = GROWTH[name]
        validate_growth(name, growth)
        start, archetype, peak = growth
        gym = role.startswith("Gym Leader")
        if gym:
            validate_gym_start(name, growth)
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
        # The source party: the curated composition in gym_leaders.json (the
        # trainer's own authored parties), else the reference party.
        curated_roster = next((row for row in curated["rosters"] if row["identity"] == name), None)
        if curated_roster:
            source_slots = []
            for entry in sorted(curated_roster["members"], key=lambda entry: entry["battleOrder"]):
                owner, index = entry["sourceOwner"], entry["sourceSlot"]
                source_family = "HNS" if owner.endswith("_HNS") else "Emerald"
                source = sources[source_family].get(owner)
                if source is None or not 0 <= index < len(source["slots"]):
                    raise ValueError(f"missing curated source slot: {owner}:{index}")
                slot = source["slots"][index]
                if slot["species"] != entry["species"] or slot.get("moves", []) != entry["moves"] or slot.get("heldItem", "ITEM_NONE") != entry["item"]:
                    raise ValueError(f"stale curated species/moves/item: {owner}:{index}")
                source_slots.append((slot, f"{source['source']}:{owner}[{index}]"))
            origin = "the curated composition in game/src/data/trainer_scaling/gym_leaders.json"
        else:
            source_slots = [(slot, f"{records[trainer]['source']}:{trainer}[{index}]") for index, slot in enumerate(reference_slots)]
            origin = "the reference party"
        roster, kept_slots = draft_roster(name, source_slots, known)
        kept.append((name, [text.split(" (")[0] for text in kept_slots]))
        # A duo is fought as its source double battle; leagues are singles only.
        double = records[trainer].get("battleType") == "TRAINER_BATTLE_TYPE_DOUBLES"
        if double != (name in DUOS) or (role == "Gym Leader duo") != (name in DUOS):
            raise ValueError(f"{name}: only a listed duo is a Gym Leader duo fought as a double battle")
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
        tags = [f"{species} [{tag[0]}]" for species, _, *tag in DRAFT[name] if tag]
        unique_kept = list(dict.fromkeys(kept_slots))
        roster_source = (f"{DRAFT_NOTE}."
                         + (f" Tags: {', '.join(tags)}." if tags else "")
                         + f" Aces ({', '.join(entry['species'] for entry in roster if entry['isAce'])}) at offset 0, fillers at {DEFAULT_OFFSET}."
                         + (f" Species in {origin} keep that source slot's moves/item/ability/nature: {', '.join(unique_kept)};"
                            if unique_kept else f" No roster species is in {origin};")
                         + " other roster slots use LEVEL_UP with no item (a label: level-up learnsets are not resolved)."
                         + (" Fought as a double battle: both leaders send Pokémon from this one roster in order." if name in DUOS else ""))
        result.append({"id": slug(name), "name": name, "region": region, "role": role,
                       "doubleBattle": double, "leagueEligible": not double,
                       "source": {"label": f"{family} local reference", "path": records[trainer]["source"], "trainerId": trainer, "note": note},
                       "referenceParty": [member(slot) for slot in reference_slots],
                       "startTR": start, "archetype": archetype, "peakTR": peak,
                       "trSource": f"PLACEHOLDER growth tuned in the explorer to the v0 balance targets: start TR {start}"
                                   + (f" (placeholder start TR in the {GYM_START_BAND[0]}–{GYM_START_BAND[1]} Gym band by archetype)" if gym else "")
                                   + f", {GROWTH_NOTE[archetype]}, peak TR {peak}.",
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
        "kept": kept, "non_final": non_final,
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
        for name, species in report["kept"]:
            if species:
                print(f"source battle content kept by {name}: {', '.join(species)}")
        print(f"{len(report['placeholders'])} placeholder evolution levels: {', '.join(report['placeholders'])}")
        if report["non_final"]:
            # Section 9 recommends final stages; any authored stage is allowed.
            print(f"warning: {len(report['non_final'])} roster slots are not final stages: {', '.join(report['non_final'])}", file=sys.stderr)
        if gaps:
            # v0 requires six roster slots: warn, don't fail.
            print(f"warning: {len(gaps)} rosters have fewer than {ROSTER_SIZE} Pokémon: {', '.join(gaps)}", file=sys.stderr)
    except (ValueError, OSError, KeyError, IndexError) as error:
        parser.exit(1, f"trainer balance catalog: {error}\n")


if __name__ == "__main__":
    main()
