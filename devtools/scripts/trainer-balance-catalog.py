#!/usr/bin/env python3
"""Regenerate the experimental explorer catalog from local trainerproc sources.

No network, ROM build, or temporary calibration files are required. Reference
moves/items describe authored sources only. Each trainer gets placeholder growth
(start TR, archetype, peak TR) and the user-directed roster draft v1 (DRAFT: six
ordered roster slots, identity/anime picks, 1-3 aces). A slot keeps authored
source item/ability/nature only when its species is in the trainer's source
party; otherwise it has no item. Roster slots carry no moves: each trainer has
one ordered move pool, the user-directed pool draft v1 (POOL_DRAFT). A roster
short of six or a slot that is not a final stage is a warning, not a failure.

The catalog also records each roster species' predecessor chain with evolution
levels: species_info EVO_LEVEL thresholds, or the shared evolution-level table
(EVOLUTION_LEVELS) for non-level evolutions. The explorer steps a member down
that chain until its level supports the stage. For every stage on those lines
it records the level-up learnset and the TM/tutor (teachable) list, and for
each line's first stage the egg moves of the species its Egg hatches as, all
as the Wayfarer ROM builds them (see load_learnsets), which the explorer's
move pool resolver reads. A pool entry without a from level goes only to a
member that learns the move by level-up (its own species or an earlier form),
so an entry every roster line learns only by TM/tutor or as an egg move is a
warning (it needs a from level), as is one no line can learn. Learnset
extraction needs a C preprocessor (arm-none-eabi-cpp, else cpp).
"""
from __future__ import annotations

import argparse
import importlib.util
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
GAME = ROOT / "game"
OUTPUT = ROOT / "devtools/ui/src/modules/trainer-balance/catalog.json"
GYMS = GAME / "src/data/trainer_scaling/gym_leaders.json"
SPECIES_INFO = GAME / "src/data/pokemon/species_info"
SPECIES_CONSTANTS = GAME / "include/constants/species.h"
GLOBAL_HEADER = GAME / "include/global.h"
POKEMON_C = GAME / "src/pokemon.c"
# Learnsets are read as the Wayfarer ROM builds them: the Makefile's
# GAME_VERSION and CPPFLAGS for BUILD=wayfarer (TEST=0, the legacy multiboot
# capabilities off, POKEMON_HNS added). -iquote src stands in for pokemon.c's
# own directory, which a quoted include searches first.
LEARNSET_BUILD = "POKEMON_WAYFARER"
LEARNSET_CPPFLAGS = ["-P", "-iquote", "include", "-iquote", "src", "-Wno-trigraphs", "-DMODERN=1", "-DTESTING=0",
                     f"-D{LEARNSET_BUILD}", "-std=gnu17", "-DENABLE_COLOSSEUM_MULTIBOOT=0",
                     "-DENABLE_BERRY_GLITCH_FIX_MULTIBOOT=0", "-DENABLE_EREADER_TRANSFER=0", "-DPOKEMON_HNS"]
MAX_POOL_MOVES = 64
POOL_NOTE = "user-directed pool draft v1"
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
# slot keeps the authored item/ability/nature of the same species in the
# trainer's source party, otherwise it has no item.
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
# The user-directed move pool draft v1: name -> (gimmick, ordered entries). An
# entry is a move, or (move, from level) when a roster line the entry is meant
# for learns it only by TM/tutor or as an egg move, or to hold back a strong
# move a line learns by level-up too early. From levels by tier: status and
# utility 20; attacks under 90 power and setup 30 (Shell Smash, Belly Drum,
# Quiver Dance 40); 90-100 power 38; 110+ power or a heavy drawback 45; OHKO
# 55. Order is content: top entries reach the aces first, so an ace's own moves
# sit above moves meant for later members, and a move listed twice goes to two
# members. A draft move no roster line can learn (by level-up on any stage,
# TM/tutor or egg move) is replaced by the closest legal move that keeps the
# gimmick.
POOL_DRAFT = {
    # Curse for Iron Defense (no roster line learns it).
    "Brock": ("hazards and sand walls (Sturdy walls, Stealth Rock, chip)", [
        "Stealth Rock", ("Sandstorm", 20), "Curse", "Stone Edge", "Earthquake", "Rock Slide", "Heavy Slam",
        "Rock Blast", "Cross Poison", "Explosion"]),
    "Misty": ("rain-boosted glass cannons", [
        ("Rain Dance", 20), "Surf", "Hydro Pump", ("Thunder", 45), "Psychic", "Recover", "Rapid Spin",
        "Ice Beam", "Dragon Dance", "Disable"]),
    # Flash Cannon twice: Electrode takes the first, Magnezone the second.
    "Lt. Surge": ("paralysis and pivoting", [
        "Thunder Wave", "Thunderbolt", ("Volt Switch", 30), "Discharge", "Light Screen", "Thunder Punch",
        ("Ice Punch", 30), "Flash Cannon", "Explosion", "Charge Beam", "Flash Cannon"]),
    "Erika": ("status and sleep under sun", [
        "Sleep Powder", "Stun Spore", "Leech Seed", "Giga Drain", "Sunny Day", "Solar Beam",
        ("Sludge Bomb", 38), "Synthesis", "Petal Dance", "Toxic"]),
    # Toxic Spikes: an egg move of the Venonat line (the ace), so from 20.
    "Janine": ("ninja poison and evasion", [
        ("Toxic", 20), ("Toxic Spikes", 20), ("Double Team", 20), ("Substitute", 20), "Sludge Bomb",
        ("U Turn", 30), "Poison Jab", "Cross Poison", "Smokescreen", ("Protect", 20)]),
    "Sabrina": ("mind control", [
        "Calm Mind", "Psychic", "Hypnosis", "Reflect", ("Light Screen", 20), "Future Sight", "Shadow Ball",
        "Barrier", "Psyshock", "Dazzling Gleam"]),
    # Morning Sun: an egg move of the Ponyta and Growlithe lines, so from 20.
    "Blaine": ("sun and raw fire", [
        "Sunny Day", "Flamethrower", "Fire Blast", ("Will O Wisp", 20), "Flare Blitz", "Extreme Speed",
        ("Solar Beam", 45), "Lava Plume", ("Overheat", 45), ("Morning Sun", 20)]),
    # Pay Day: Persian learns it from its earlier form Meowth (Lv 30).
    "Giovanni": ("ground brutes and the boss's cat", [
        "Earthquake", "Stone Edge", "Megahorn", ("Swords Dance", 30), "Fake Out", "Pay Day", "Nasty Plot",
        "Sucker Punch", "Earth Power", ("Sludge Wave", 38)]),
    # Brave Bird and Defog: egg moves of the Pidgey line (the ace), so from 45 and 20.
    "Falkner": ("speed and Tailwind", [
        "Tailwind", ("Brave Bird", 45), "Roost", "Air Slash", "Hurricane", "Feather Dance", ("U Turn", 30),
        "Drill Peck", "Mirror Move", ("Defog", 20)]),
    "Bugsy": ("setup and pivot bugs", [
        "Fury Cutter", "Swords Dance", "Bullet Punch", "U Turn", "Megahorn", "Close Combat", "Quiver Dance",
        "Sleep Powder", "Bug Buzz", "Sticky Web"]),
    "Whitney": ("Rollout and Milk Drink (the classic wall)", [
        "Rollout", "Milk Drink", ("Attract", 20), "Body Slam", "Stomp", "Metronome", ("Soft Boiled", 20),
        "Moonblast", "Hyper Voice", "Heal Bell"]),
    # Perish Song: Mismagius learns it from its earlier form Misdreavus (Lv 46). Destiny Bond: an egg move
    # of the Misdreavus line, so from 20 (the ace Gengar is full by then).
    "Morty": ("sleep, dream and curse", [
        "Hypnosis", "Dream Eater", "Shadow Ball", "Curse", ("Destiny Bond", 20), "Will O Wisp", "Hex",
        "Confuse Ray", "Perish Song", "Night Shade"]),
    # Mach Punch: an egg move of the Hitmontop line (Tyrogue), so from 30.
    "Chuck": ("waterfall-trained power", [
        "Dynamic Punch", "Bulk Up", ("Close Combat", 45), ("Mach Punch", 30), ("Focus Punch", 45),
        ("Waterfall", 30), ("Ice Punch", 30), "Triple Kick", "Cross Chop", "Aura Sphere"]),
    # Spikes twice: Skarmory takes the first, Forretress the second.
    "Jasmine": ("iron defense and hazards", [
        "Iron Tail", "Iron Defense", "Spikes", "Spikes", "Stealth Rock", "Rapid Spin", "Gyro Ball",
        ("Thunderbolt", 38), "Thunder Wave", "Flash Cannon", "Heavy Slam"]),
    # Freeze Dry: an egg move of the Swinub line (the ace), so from 30.
    "Pryce": ("hail and old-master patience", [
        "Hail", "Blizzard", "Ice Shard", "Icicle Crash", "Earthquake", ("Aurora Veil", 20), "Rest",
        ("Sleep Talk", 20), "Ice Beam", ("Freeze Dry", 30)]),
    # Dragon Claw for Draco Meteor (no roster line learns it).
    "Clair": ("rain dragons and Dragon Dance", [
        "Dragon Dance", "Outrage", ("Rain Dance", 20), "Hydro Pump", "Dragon Pulse", "Thunder Wave",
        "Earthquake", "Dragon Rush", "Aqua Tail", "Dragon Claw"]),
    "Roxanne": ("the fossil lesson: Sturdy counters and hazards", [
        ("Rock Tomb", 30), ("Stealth Rock", 20), "Sandstorm", "Power Gem", "Metal Burst", ("Head Smash", 45),
        "Rock Polish", "Earth Power", "Thunder Wave", ("Wood Hammer", 45)]),
    # Spore: Breloom learns it from its earlier form Shroomish (Lv 40).
    "Brawly": ("the surfer brawler: Fake Out and Bulk Up", [
        "Fake Out", "Bulk Up", "Close Combat", ("Drain Punch", 30), "Mach Punch", "Spore", ("Bullet Seed", 30),
        "Ice Punch", "Knock Off", ("Surf", 38)]),
    # Magnezone fills up (Magnet Rise, Light Screen, Flash Cannon, Thunder) above Explosion, so Electrode takes
    # it.
    "Wattson": ("Wahaha! Paralysis and Explosion", [
        "Thunder Wave", ("Volt Switch", 30), "Discharge", ("Thunderbolt", 38), "Magnet Rise", "Light Screen",
        "Flash Cannon", ("Thunder", 45), ("Explosion", 45), "Charge"]),
    # Yawn: an egg move for the ace Torkoal, so from 20; above Overheat so Torkoal takes it.
    "Flannery": ("sun and Eruption", [
        ("Sunny Day", 20), "Eruption", ("Yawn", 20), ("Overheat", 45), "Lava Plume", ("Will O Wisp", 20),
        ("Solar Beam", 45), "Earth Power", "Heat Wave", "Rapid Spin"]),
    # Extreme Speed: an egg move of the Zigzagoon line, so from 30. Belly Drum twice: Snorlax takes the first,
    # Linoone the second.
    "Norman": ("Facade and Belly Drum normals", [
        ("Facade", 30), "Slack Off", "Belly Drum", ("Extreme Speed", 30), "Fake Out", "Body Slam", "Belly Drum",
        "Double Edge", "Hammer Arm", "Encore", "Rest"]),
    "Winona": ("graceful flyers: Roost and Dragon Dance", [
        ("Aerial Ace", 30), ("Roost", 20), "Dragon Dance", ("Brave Bird", 45), ("Hurricane", 45), "Tailwind",
        "Spikes", "Whirlwind", "Cotton Guard", "Sky Attack"]),
    # Heal Pulse for Helping Hand (no roster line learns it). Explosion above Trick Room so Solrock takes it.
    "Tate & Liza": ("double-battle sync: Levitate + Earthquake, Trick Room", [
        ("Calm Mind", 30), ("Earthquake", 38), "Rock Slide", "Explosion", ("Trick Room", 20), "Heal Pulse",
        "Psychic", ("Reflect", 20), ("Light Screen", 20), "Cosmic Power"]),
    # Aqua Jet: an egg move of the Luvdisc and Corphish lines, so from 30; twice, so Crawdaunt takes the second.
    # Dragon Dance above Hydro Pump so Kingdra takes it.
    "Juan": ("flamboyant rain and charm", [
        ("Water Pulse", 30), ("Rain Dance", 20), "Dragon Dance", "Hydro Pump", ("Scald", 30), "Attract",
        "Charm", ("Aqua Jet", 30), ("Aqua Jet", 30), "Crabhammer", "Sheer Cold"]),
    # Freeze Dry: an egg move for the ace Lapras, so from 30.
    "Lorelei": ("ice and Shell Smash", [
        "Shell Smash", "Icicle Spear", ("Freeze Dry", 30), "Perish Song", "Blizzard", "Ice Beam", "Lovely Kiss",
        "Slack Off", ("Aurora Veil", 20), "Rest"]),
    # Feint for Fake Out (no roster line learns it).
    "Bruno": ("No Guard punches and Bulk Up", [
        "Dynamic Punch", "Bulk Up", "Close Combat", "High Jump Kick", "Mach Punch", ("Fire Punch", 30),
        "Ice Punch", "Thunder Punch", "Feint", ("Earthquake", 38)]),
    "Agatha": ("trap and drain", [
        "Mean Look", "Curse", "Destiny Bond", "Hypnosis", "Dream Eater", "Shadow Ball", "Glare", "Haze",
        "Bonemerang", ("Will O Wisp", 20)]),
    "Koga": ("poison stall and evasion", [
        ("Toxic", 20), "Toxic Spikes", "Spikes", "Minimize", ("Double Team", 20), ("Substitute", 20),
        "Sludge Bomb", ("Will O Wisp", 20), "Sticky Web", "Explosion"]),
    # Confuse Ray above Future Sight so the ace Xatu takes it.
    "Will": ("psychic masquerade: Trick Room and screens", [
        ("Trick Room", 20), "Calm Mind", "Psychic", "Reflect", ("Light Screen", 20), "Confuse Ray",
        "Future Sight", "Lovely Kiss", "Yawn", "Moonblast"]),
    # Wish: an egg move of the Eevee line (the ace), so from 20. Brave Bird: an egg move of the Murkrow line, so
    # from 45.
    "Karen": ("disruption. \"Strong Pokémon, weak Pokémon.\"", [
        "Foul Play", "Moonlight", ("Wish", 20), "Taunt", "Nasty Plot", "Dark Pulse", "Sucker Punch",
        "Sleep Powder", "Shadow Ball", ("Brave Bird", 45)]),
    # Aqua Jet twice (the second an egg move of the Corphish line, from 30): Sharpedo and Crawdaunt. Superpower:
    # an egg move of the Corphish line, so from 45.
    "Sidney": ("dark aggression and priority", [
        "Sucker Punch", "Swords Dance", "Knock Off", "Night Slash", "Crunch", "Taunt", ("Protect", 20),
        "Aqua Jet", ("Aqua Jet", 30), ("Superpower", 45), "Leaf Blade"]),
    # Pain Split, Recover and Spikes: egg moves (Duskull, Sableye, Snorunt lines), so from 20. Dusknoir fills up
    # (Ice Beam) above Shadow Sneak, so Banette takes it.
    "Phoebe": ("burns and grudges", [
        "Will O Wisp", ("Pain Split", 20), ("Trick Room", 20), ("Ice Beam", 38), "Shadow Sneak", "Destiny Bond",
        "Shadow Claw", ("Recover", 20), ("Spikes", 20), "Curse"]),
    # Spikes: an egg move of the Snorunt line, so from 20.
    "Glacia": ("hail and Sheer Cold", [
        "Hail", "Blizzard", ("Aurora Veil", 20), ("Sheer Cold", 55), "Freeze Dry", "Ice Shard", "Protect",
        ("Surf", 38), ("Spikes", 20), ("Explosion", 45)]),
    "Drake": ("sea-captain dragons", [
        "Dragon Claw", "Dragon Dance", "Outrage", "Earthquake", ("Waterfall", 30), "Flamethrower", "Crunch",
        "Dragon Pulse", ("Roost", 20), "Hydro Pump"]),
    # Extreme Speed: an egg move of the Dratini line, so from 30. Dragon Rush above Fire Blast so the ace fills
    # up and Charizard takes Fire Blast.
    "Lance": ("Hyper Beam Dragonite and speed", [
        "Hyper Beam", ("Extreme Speed", 30), "Dragon Dance", "Outrage", "Thunder Wave", "Dragon Rush",
        ("Fire Blast", 45), ("Earthquake", 38), "Waterfall", "Roost"]),
    # Mirror Coat: an egg move for the ace Milotic (Feebas), so from 30. Giga Drain twice: Tentacruel takes the
    # first, Ludicolo the second.
    "Wallace": ("rain, bulk and beauty", [
        "Rain Dance", ("Scald", 30), "Recover", ("Mirror Coat", 30), "Water Spout", ("Giga Drain", 30),
        ("Giga Drain", 30), "Toxic Spikes", "Earthquake", "Dragon Dance", ("Ice Beam", 38)]),
    # Head Smash and Recover: egg moves of the Aron and Lileep lines, so from 45 and 20.
    "Steven": ("Stealth Rock and Meteor Mash", [
        "Meteor Mash", "Bullet Punch", ("Stealth Rock", 20), ("Earthquake", 38), ("Head Smash", 45), "Spikes",
        ("Roost", 20), "Rapid Spin", ("Recover", 20), ("Swords Dance", 30)]),
    # Dark Pulse for Foul Play (no roster line learns it: no level-up, TM/tutor or egg move).
    "Blue": ("Gary's all-rounder: priority and coverage", [
        ("Dark Pulse", 30), "Moonlight", "Psychic", "Calm Mind", "Extreme Speed", "Flare Blitz", "Bullet Punch",
        "Swords Dance", "Earth Power", "Megahorn", "Hurricane", "Roost"]),
}
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


def c_block(text, start):
    """The end of the brace block opening at text[start] ("{"), skipping string and char literals."""
    depth, index, quote = 0, start, None
    while index < len(text):
        char = text[index]
        if quote:
            if char == "\\":
                index += 1
            elif char == quote:
                quote = None
        elif char in "\"'":
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
        index += 1
    raise ValueError("unbalanced braces in the preprocessed learnset data")


def learnset_translation_unit():
    """The C source the learnsets are preprocessed from, built from the game's own files.

    global.h's leading include block (minus constants/maps.h, which needs the
    generated map headers and only defines map constants), then pokemon.c's own
    P_LVL_UP_LEARNSETS #if chain that picks the level-up learnset header, the
    generated teachable header, the egg move header and species_info.h, as
    pokemon.c includes them.
    """
    includes = []
    for line in GLOBAL_HEADER.read_text().splitlines():
        if line.startswith("#include") and "constants/maps.h" not in line:
            includes.append(line)
        if "config/save.h" in line:
            break
    source = POKEMON_C.read_text()
    chain = re.search(r"^#if P_LVL_UP_LEARNSETS\b.*?^#endif", source, re.M | re.S)
    if not chain or '#include "data/pokemon/teachable_learnsets.h"' not in source[chain.end():]:
        raise ValueError("pokemon.c no longer selects level-up learnsets through P_LVL_UP_LEARNSETS")
    if '#include "data/pokemon/egg_moves.h"' not in source[chain.end():]:
        raise ValueError("pokemon.c no longer includes data/pokemon/egg_moves.h")
    return "\n".join([*includes, chain.group(0), '#include "data/pokemon/teachable_learnsets.h"',
                      '#include "data/pokemon/egg_moves.h"', '#include "data/pokemon/species_info.h"'])


def load_learnsets(tokens):
    """Each species token's level-up learnset, teachable (TM/tutor) list and egg moves, as the Wayfarer ROM builds them.

    The teachable header is generated by the game's own learnset helpers
    (make_tutors.py, make_teaching_types.py, make_teachables.py --build
    POKEMON_WAYFARER, the Makefile's recipe for BUILD=wayfarer) into a scratch
    copy of their inputs, so nothing is written under game/. The level-up
    learnsets, the teachable arrays and species_info are then run through the
    C preprocessor with the Makefile's CPPFLAGS for that build, so every
    generation-config #if resolves as it does in the ROM, and the expanded
    arrays are parsed. Egg moves come the same way: pokemon.c's own
    data/pokemon/egg_moves.h arrays, through each species_info entry's
    .eggMoveLearnset (none means no egg moves, as GetSpeciesEggMoves falls
    back to SPECIES_NONE's empty list). Returns (valid move tokens, {old move
    name: move token}, {species token: {"levelUp": [(level, move token)],
    "teachable": [move token], "egg": [move token]}}); level 0 is an
    evolution move.
    """
    with tempfile.TemporaryDirectory(prefix="trainer-balance-learnsets-") as directory:
        work = Path(directory)
        (work / "src/data/pokemon").mkdir(parents=True)
        (work / "tools/learnset_helpers").mkdir(parents=True)  # make_tutors.py touches its sibling here
        (work / "build").mkdir()
        for name in ("include", "data"):
            (work / name).symlink_to(GAME / name)
        for name in ("species_info", "species_info.h", "all_learnables.json", "special_movesets.json"):
            (work / "src/data/pokemon" / name).symlink_to(GAME / "src/data/pokemon" / name)
        helpers = GAME / "tools/learnset_helpers"
        command([sys.executable, str(helpers / "make_tutors.py"), "build/all_tutors.json"], cwd=work)
        command([sys.executable, str(helpers / "make_teaching_types.py"), "build/all_teaching_types.json"], cwd=work)
        command([sys.executable, str(helpers / "make_teachables.py"), "--build", LEARNSET_BUILD, "build"], cwd=work)
        header = work / "src/data/pokemon/teachable_learnsets.h"
        if not header.is_file():
            raise ValueError("make_teachables.py wrote no teachable_learnsets.h (is P_LEARNSET_HELPER_TEACHABLE off?)")
        # The unit sits beside its own data/pokemon/teachable_learnsets.h, which a
        # quoted include finds before game/src (the directory pokemon.c lives in).
        unit = work / "cpp/learnsets.c"
        (unit.parent / "data/pokemon").mkdir(parents=True)
        (unit.parent / "data/pokemon/teachable_learnsets.h").write_text(header.read_text())
        markers = "".join(f"\n__WAYFARER_SPECIES__ {token}" for token in tokens)
        # daycare.c swaps an incense baby Egg for its parent species only below Gen 9 incense breeding.
        incense = "\n#if P_INCENSE_BREEDING < GEN_9\n__WAYFARER_INCENSE_BREEDING__\n#endif"
        unit.write_text(learnset_translation_unit() + markers + incense + "\n")
        preprocessor = "arm-none-eabi-cpp" if shutil.which("arm-none-eabi-cpp") else "cpp"
        text = command([preprocessor, *LEARNSET_CPPFLAGS, str(unit)], cwd=GAME)
    if re.search(r"^__WAYFARER_INCENSE_BREEDING__$", text, re.M):
        raise ValueError("P_INCENSE_BREEDING is below Gen 9, so an incense baby's Egg may hatch as its parent"
                         " species (daycare.c AlterEggSpeciesWithIncenseItem): egg moves are not resolvable")
    ids = re.findall(r"^__WAYFARER_SPECIES__ (\S+)$", text, re.M)
    if len(ids) != len(tokens) or not all(value.isdigit() for value in ids):
        raise ValueError("could not resolve roster species IDs in the preprocessed data")
    moves_enum = re.search(r"enum __attribute__\(\(packed\)\) Move\s*\{(.*?)\};", text, re.S)
    if not moves_enum:
        raise ValueError("enum Move not found in the preprocessed data")
    members = re.findall(r"^\s*(\w+)(?:\s*=\s*([^,\n]+))?,", moves_enum.group(1), re.M)
    names = [name for name, _ in members]
    if "MOVES_COUNT" not in names:
        raise ValueError("enum Move has no MOVES_COUNT")
    # Real moves: MOVE_NONE < move < MOVES_COUNT (Z-Moves and Max Moves follow
    # it). An old name defined as another move (MOVE_FAINT_ATTACK =
    # MOVE_FEINT_ATTACK) is an alias of that move, not a move of its own.
    aliases = {name: value.strip() for name, value in members if value and value.strip().startswith("MOVE_")}
    valid = [name for name in names[:names.index("MOVES_COUNT")]
             if name.startswith("MOVE_") and name != "MOVE_NONE" and name not in aliases]
    level_up = {}
    for name, body in re.findall(r"static const struct LevelUpMove (s\w+LevelUpLearnset)\[\] = \{(.*?)\};", text, re.S):
        items = re.findall(r"\{([^{}]*)\}", body)
        entries = [re.fullmatch(r"\s*\.move = (\w+), \.level = (\d+)\s*", item) for item in items]
        if not entries or any(entry is None for entry in entries) or entries[-1].group(1) != "0xFFFF":
            raise ValueError(f"unexpected level-up learnset layout: {name}")
        level_up[name] = [(int(level), move) for move, level in (entry.groups() for entry in entries[:-1])]
    teachable = {}
    for name, body in re.findall(r"static const u16 (s\w+TeachableLearnset)\[\] = \{(.*?)\};", text, re.S):
        items = [item.strip() for item in body.split(",") if item.strip()]
        if not items or items[-1] != "0xFFFF":
            raise ValueError(f"unexpected teachable learnset layout: {name}")
        teachable[name] = items[:-1]
    egg = {}
    for name, body in re.findall(r"static const u16 (s\w+EggMoveLearnset)\[\] = \{(.*?)\};", text, re.S):
        items = [item.strip() for item in body.split(",") if item.strip()]
        if not items or items[-1] != "0xFFFF":
            raise ValueError(f"unexpected egg move learnset layout: {name}")
        egg[name] = items[:-1]
    if not egg:
        raise ValueError("no egg move learnsets in the preprocessed data")
    table = text.index("{", text.index("gSpeciesInfo[] ="))
    entries = {}
    # Top-level designated initializers "[id] = { ... }"; a later one for the
    # same species overrides an earlier one, as in C (HNS overrides).
    index, quote, depth = table, None, 0
    head = re.compile(r"\[(\d+)\] =\s*\{")
    while True:
        char = text[index]
        if quote:
            if char == "\\":
                index += 1
            elif char == quote:
                quote = None
        elif char in "\"'":
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                break
        elif char == "[" and depth == 1 and (match := head.match(text, index)):
            end = c_block(text, match.end() - 1)
            entries[match.group(1)] = text[match.end() - 1:end + 1]
            index = end
        index += 1
    result = {}
    for token, species_id in zip(tokens, ids):
        entry = entries.get(species_id)
        level_name = re.search(r"\.levelUpLearnset = (\w+),", entry or "")
        teach_name = re.search(r"\.teachableLearnset = (\w+),", entry or "")
        egg_name = re.search(r"\.eggMoveLearnset = (\w+),", entry or "")
        if not entry or not level_name or not teach_name:
            raise ValueError(f"{display(token, 'SPECIES_')}: no learnsets in species_info")
        if egg_name and egg_name.group(1) not in egg:
            raise ValueError(f"{display(token, 'SPECIES_')}: egg moves {egg_name.group(1)} not found")
        eggs = egg[egg_name.group(1)] if egg_name else []
        if any(move not in valid for move in eggs):
            raise ValueError(f"{display(token, 'SPECIES_')}: {egg_name.group(1)} lists an unknown move")
        for name, known in ((level_name.group(1), level_up), (teach_name.group(1), teachable)):
            if name not in known:
                raise ValueError(f"{display(token, 'SPECIES_')}: learnset {name} not found")
            if any(move not in valid for move in (known[name] if known is teachable else [move for _, move in known[name]])):
                raise ValueError(f"{display(token, 'SPECIES_')}: {name} lists an unknown move")
        result[token] = {"levelUp": level_up[level_name.group(1)], "teachable": teachable[teach_name.group(1)],
                         "egg": eggs}
    return valid, aliases, result


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


def egg_species(token, previous):
    """The species a line's Egg hatches as: daycare.c GetEggSpecies walks predecessors back to the root, babies included.

    With Gen 9 incense breeding (checked in load_learnsets) no incense item
    changes it. Ancestry must be unambiguous.
    """
    seen = {token}
    while previous.get(token):
        sources = previous[token]
        if len(sources) > 1:
            raise ValueError(f"{display(token, 'SPECIES_')}: ambiguous predecessors {', '.join(sources)}")
        token = sources[0]
        if token in seen:
            raise ValueError(f"{display(token, 'SPECIES_')}: evolution cycle")
        seen.add(token)
    return token


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
    """The DRAFT roster as catalog roster slots and "species (source slot)" for kept content.

    `source_slots` is [(slot, provenance)]. A species must resolve in the game
    data. A slot whose species is in the source party keeps that source slot's
    item/ability/nature (first match); every other slot has no item, ability
    or nature.
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
                 "isAce": is_ace, "item": None, "ability": None, "nature": None}
        if source is not None:
            authored = member(source)
            kept.append(f"{entry['species']} ({where})")
            entry.update(item=authored["item"],
                         ability=optional(source.get("ability"), "ABILITY_", "ABILITY_NONE"),
                         # trainerproc emits NATURE_HARDY when a party omits the nature.
                         nature=optional(source.get("nature"), "NATURE_", "NATURE_HARDY"))
        roster.append(entry)
    return roster, kept


def draft_pool(name):
    """The POOL_DRAFT move pool as catalog entries ({"move"[, "fromLevel"]}), in order."""
    return [{"move": entry} if isinstance(entry, str) else {"move": entry[0], "fromLevel": entry[1]}
            for entry in POOL_DRAFT[name][1]]


def validate_pool(name, pool, moves):
    """One ordered pool: valid move names, an optional from level in 1-100, at most MAX_POOL_MOVES entries."""
    if not isinstance(pool, list) or len(pool) > MAX_POOL_MOVES:
        raise ValueError(f"{name}: a move pool lists at most {MAX_POOL_MOVES} entries")
    for index, entry in enumerate(pool):
        where = f"{name} move pool [{index + 1}]"
        if not isinstance(entry, dict) or not set(entry) <= {"move", "fromLevel"} or "move" not in entry:
            raise ValueError(f"{where}: an entry is a move and an optional fromLevel")
        if entry["move"] not in moves:
            raise ValueError(f"{where}: unknown move {entry['move']!r}")
        level = entry.get("fromLevel", 1)
        if not isinstance(level, int) or isinstance(level, bool) or not 1 <= level <= 100:
            raise ValueError(f"{where}: fromLevel must be an integer from 1 to 100")


def default_moveset(level_up, level):
    """The constructor's default moveset (GiveBoxMonInitialMoveset): the last four level-up moves learned by `level`.

    Walks the learnset in order until a move above `level`, skips evolution
    moves (level 0) and moves already known, and drops the oldest when full.
    """
    moves = []
    for learned, move in level_up:
        if learned > level:
            break
        if learned == 0 or move in moves:
            continue
        moves = (moves if len(moves) < 4 else moves[1:]) + [move]
    return moves


def validate_usable_moves(name, chain, learnsets):
    """Every member has at least one usable move at every reachable level and stage, stepped-down stages included."""
    for position, (token, _) in enumerate(chain):
        # The lowest level at which the downward rule leaves the member at this stage.
        reached = 1 if position == 0 else chain[position][1]
        if not default_moveset(learnsets[token]["levelUp"], reached):
            raise ValueError(f"{name}: {display(token, 'SPECIES_')} has no level-up move at Lv {reached}")


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
        if "moves" in entry:
            raise ValueError(f"{where}: roster slots carry no moves (they come from the move pool)")
    if roster[0]["levelOffset"] != 0:
        raise ValueError(f"{name}: roster slot 1 must have level offset 0")
    if not roster[0]["isAce"]:
        raise ValueError(f"{name}: roster slot 1 (the signature Pokémon) must be an ace")
    if sum(entry["isAce"] for entry in roster) > MAX_ACES:
        raise ValueError(f"{name}: a roster has 1-{MAX_ACES} aces")


def pool_warnings(pool, level_up, learnable):
    """A pool's always-dormant entries, given the moves its roster lines learn.

    `level_up` holds the moves some stage learns by level-up and `learnable`
    every move some stage can learn (level-up, TM/tutor, or an egg move of the
    line). Returns (entries no stage can learn, entries without a from level
    that every learner gets only by TM/tutor or as an egg move), each as move
    names in pool order.
    """
    unlearnable = [entry["move"] for entry in pool if entry["move"] not in learnable]
    tm_only = [entry["move"] for entry in pool
               if "fromLevel" not in entry and entry["move"] in learnable and entry["move"] not in level_up]
    return unlearnable, tm_only


def learnset_data(lines, trainers, previous):
    """Compact learnsets for every stage on every roster line, and the move pools checked against them.

    Returns ({"moves": [name], "species": {species: {"levelUp": [level, move
    index, ...], "teachable": [move index][, "egg": [move index]]}}}, pool
    entries that no stage on the trainer's roster lines can learn, and pool
    entries without a from level that those lines learn only by TM/tutor or as
    egg moves). Level-up pairs keep the game's order; level 0 is an evolution
    move. Each line's first stage carries "egg": the egg moves of the species
    the line's Egg hatches as (egg_species, e.g. Pichu for Pikachu), empty
    when it has none.
    """
    stages = {token for chain in lines.values() for token, _ in chain}
    bases = {chain[0][0] for chain in lines.values()}
    hatches = {base: egg_species(base, previous) for base in bases}
    tokens = sorted(stages | set(hatches.values()))
    valid, aliases, learnsets = load_learnsets(tokens)
    names = {token: display(token, "MOVE_") for token in valid}
    # Source parties may use an old move name; the pool stores the move itself.
    canonical = {display(alias, "MOVE_"): names[target] for alias, target in aliases.items() if target in names}
    for trainer in trainers:
        for entry in trainer["movePool"]:
            entry["move"] = canonical.get(entry["move"], entry["move"])
    if len(set(names.values())) != len(names):
        raise ValueError("two moves share a display name")
    moves = sorted(names.values())
    position = {name: index for index, name in enumerate(moves)}
    for species, chain in lines.items():
        validate_usable_moves(species, chain, learnsets)
    species = {}
    for token in sorted(stages):
        data = learnsets[token]
        species[display(token, "SPECIES_")] = {
            "levelUp": [part for level, move in data["levelUp"] for part in (level, position[names[move]])],
            "teachable": sorted({position[names[move]] for move in data["teachable"]})}
        if token in bases:
            species[display(token, "SPECIES_")]["egg"] = sorted(
                {position[names[move]] for move in learnsets[hatches[token]]["egg"]})
    unlearnable = []
    tm_only = []
    for trainer in trainers:
        validate_pool(trainer["name"], trainer["movePool"], position)
        roster_stages = [token for entry in trainer["roster"] for token, _ in lines[entry["species"]]]
        level_up = {names[move] for token in roster_stages for _, move in learnsets[token]["levelUp"]}
        learnable = (level_up | {names[move] for token in roster_stages for move in learnsets[token]["teachable"]}
                     | {names[move] for entry in trainer["roster"]
                        for move in learnsets[hatches[lines[entry["species"]][0][0]]]["egg"]})
        never, needs_from = pool_warnings(trainer["movePool"], level_up, learnable)
        unlearnable += [f"{trainer['name']} {move}" for move in never]
        tm_only += [f"{trainer['name']} {move}" for move in needs_from]
    return {"moves": moves, "species": species}, unlearnable, tm_only


def generate():
    sources = load_sources()
    edges, previous = load_evolutions()
    table = evolution_table(edges)
    known = known_species()
    chains = {}
    lines = {}
    kept = []
    non_final = []
    curated = json.loads(GYMS.read_text())
    if curated.get("version") != 1:
        raise ValueError("unsupported gym catalog")
    result = []
    gaps = []
    if set(GROWTH) != {row[0] for row in ROSTER} or set(DRAFT) != set(GROWTH) or set(POOL_DRAFT) != set(GROWTH):
        raise ValueError("GROWTH, DRAFT and POOL_DRAFT must list exactly the catalog trainers")
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
        pool = draft_pool(name)
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
            lines[entry["species"]] = chain
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
                         + (f" Species in {origin} keep that source slot's item/ability/nature: {', '.join(unique_kept)};"
                            if unique_kept else f" No roster species is in {origin};")
                         + " other roster slots have no item. Roster slots carry no moves: members draw them from the move pool."
                         + (" Fought as a double battle: both leaders send Pokémon from this one roster in order." if name in DUOS else ""))
        result.append({"id": slug(name), "name": name, "region": region, "role": role,
                       "doubleBattle": double, "leagueEligible": not double,
                       "source": {"label": f"{family} local reference", "path": records[trainer]["source"], "trainerId": trainer, "note": note},
                       "referenceParty": [member(slot) for slot in reference_slots],
                       "startTR": start, "archetype": archetype, "peakTR": peak,
                       "trSource": f"PLACEHOLDER growth tuned in the explorer to the v0 balance targets: start TR {start}"
                                   + (f" (placeholder start TR in the {GYM_START_BAND[0]}–{GYM_START_BAND[1]} Gym band by archetype)" if gym else "")
                                   + f", {GROWTH_NOTE[archetype]}, peak TR {peak}.",
                       "roster": roster, "rosterSource": roster_source, "movePool": pool,
                       "movePoolSource": f"{POOL_NOTE}: {POOL_DRAFT[name][0]}"})
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
    learnsets, unlearnable, tm_only = learnset_data(lines, result, previous)
    return json.dumps({"evolution": evolution, "learnsets": learnsets, "trainers": result},
                      indent=2, ensure_ascii=False) + "\n", gaps, {
        "kept": kept, "non_final": non_final, "unlearnable": unlearnable, "tm_only": tm_only,
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
        if report["unlearnable"]:
            # Learnability is the resolver's rule: such an entry stays dormant, it is not invalid.
            print(f"warning: {len(report['unlearnable'])} move pool entries no roster line can learn (always dormant):"
                  f" {', '.join(report['unlearnable'])}", file=sys.stderr)
        if report["tm_only"]:
            # Without a from level only a level-up learner (own or earlier form) takes an entry: dormant until one is set.
            print(f"warning: {len(report['tm_only'])} move pool entries every roster line learns only by TM/tutor"
                  " or as an egg move"
                  f" and have no from level (always dormant; set a from level): {', '.join(report['tm_only'])}",
                  file=sys.stderr)
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
