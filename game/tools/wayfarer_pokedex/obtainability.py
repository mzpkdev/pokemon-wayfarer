#!/usr/bin/env python3
"""Which Pokédex species can a Wayfarer player obtain? (spec: home-pokedex-lists.md, "Obtainability")

Reads every way the Wayfarer build hands out a Pokémon and closes over evolution and breeding:

  wild      the wild encounters v2 tables (src/data/wild_encounters_v2/*.json): every region, the
            Safari Zones and the Bug-Catching Contest days, Rock Smash/Headbutt, Surf and fishing
  script    givemon / giveegg / setwildbattle / seteventmon in the Wayfarer script assembly, on
            maps the Wayfarer build compiles (statics, legendaries, gifts, fossils, starters,
            Game Corner prizes; a `givemon VAR_x` takes every species `setvar VAR_x, SPECIES_*`
            names in the same file)
  roamer    the roaming legendaries a compiled script starts (`special InitRoamer`, src/roamer.c)
  trade     the in-game trades a compiled script offers (src/data/trade.h)
  evolve    an evolution from an obtainable species whose conditions can be met here: the item
            exists in the world, the region or map section is a compiled map's, and so on
  breed     an egg from an obtainable parent at a compiled Day Care (the egg is the parent's
            first stage; a Day Care's region turns a foreign regional form into its own)

Link trades with a second cartridge, the randomizer and debug scripts do not count.

A pick-one gift counts as a source for every species it offers. The Johto and Hoenn starter trios are
such gifts (Elm or Birch's bag, plus Birch's reward or Silph Co.), so no single player gets all three of a
trio; that is an open owner decision recorded in .product/research/pokedex-obtainability.md, and it is
deliberately not modelled here so the recorded result does not change when it is settled.

The National Pokédex entries the tool cannot place, and why, are recorded in obtainability.json.
`--check` fails when the result or the species data's dexNotRequired flags drift from it, and
`--write` re-records it. Needs the Wayfarer map outputs (`make BUILD=wayfarer generated`).
"""
from __future__ import annotations

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re
import subprocess
import sys

TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[1]  # game/
RECORD = TOOL / 'obtainability.json'
C_TEST = ROOT / 'test/pokedex_obtainability.c'
C_BEGIN, C_END = '    // BEGIN obtainability.json', '    // END obtainability.json'
REGION_FILES = ('kanto', 'johto', 'hoenn', 'alola', 'sevii', 'safari', 'sinjoh')
GIVERS = ('givemon', 'giveegg', 'setwildbattle', 'setwildbattleshiny', 'seteventmon')
# Scripts that are assembled but never reachable in play.
NOT_PLAYABLE_SCRIPTS = {'data/scripts/debug.inc'}
FORM_REGIONS = {'isAlolanForm': 'REGION_ALOLA', 'isGalarianForm': 'REGION_GALAR',
                'isHisuianForm': 'REGION_HISUI', 'isPaldeanForm': 'REGION_PALDEA'}
FLAGS = ('isMythical', 'isLegendary', 'isUltraBeast', 'dexNotRequired', 'dexForceRequired') + tuple(FORM_REGIONS)
# Evolution conditions that hold for any trainer with time and an obtainable Pokémon of the right kind.
ALWAYS_MET = {'IF_TIME', 'IF_NOT_TIME', 'IF_MIN_FRIENDSHIP', 'IF_GENDER', 'IF_WEATHER',
              'IF_USED_MOVE_X_TIMES', 'IF_RECOIL_DAMAGE_GE', 'IF_CRITICAL_HITS_GE', 'IF_ATK_LT_DEF',
              'IF_ATK_GT_DEF', 'IF_ATK_EQ_DEF', 'IF_PID_MODULO_100_GT', 'IF_PID_MODULO_100_EQ',
              'IF_PID_MODULO_100_LT', 'IF_PID_UPPER_MODULO_10_GT', 'IF_PID_UPPER_MODULO_10_LT',
              'IF_PID_UPPER_MODULO_10_EQ', 'IF_MIN_BEAUTY', 'IF_CURRENT_DAMAGE_GE', 'IF_TYPE_IN_PARTY',
              'IF_AMPED_NATURE', 'IF_LOW_KEY_NATURE', 'IF_NATURE'}
# Evolution methods a lone player can trigger (EVO_TRADE needs a second cartridge).
# Eggs the Day Care swaps by the offspring's gender (DetermineEggSpeciesAndParentSlots, src/daycare.c;
# P_NIDORAN_M_DITTO_BREED is GEN_LATEST, so both directions apply).
EGG_SWAPS = {'SPECIES_NIDORAN_F': {'SPECIES_NIDORAN_M'}, 'SPECIES_NIDORAN_M': {'SPECIES_NIDORAN_F'},
             'SPECIES_ILLUMISE': {'SPECIES_VOLBEAT'}, 'SPECIES_VOLBEAT': {'SPECIES_ILLUMISE'},
             'SPECIES_MANAPHY': {'SPECIES_PHIONE'}}
SOLO_METHODS = {'EVO_LEVEL', 'EVO_ITEM', 'EVO_SPIN', 'EVO_BATTLE_END', 'EVO_SPLIT_FROM_EVO', 'EVO_LEVEL_BATTLE_ONLY'}


class AuditError(ValueError):
    pass


def read(path):
    return Path(path).read_text(encoding='utf-8', errors='replace')


def cpp(text, root=ROOT, includes=True, extra=()):
    """The Wayfarer build's view of C source: IS_WAYFARER and IS_HNS true, IS_FRLG false."""
    if not includes:
        text = re.sub(r'^\s*#\s*include\b.*$', '', text, flags=re.M)
    command = ['cpp', '-P', '-DTRUE=1', '-DFALSE=0', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS',
               '-I', str(root / 'include'), '-I', str(root), '-include', 'constants/global.h', *extra, '-']
    done = subprocess.run(command, input=text, text=True, capture_output=True, cwd=root)
    if done.returncode:
        raise AuditError(f'cpp failed: {done.stderr[:600]}')
    return done.stdout


def closing(text, start, opening='{', close='}'):
    depth = 0
    for index in range(start, len(text)):
        if text[index] == opening:
            depth += 1
        elif text[index] == close:
            depth -= 1
            if depth == 0:
                return index
    raise AuditError('unbalanced braces')


def braced_rows(text):
    """The top-level {...} items of a brace list body."""
    rows, index = [], 0
    while True:
        start = text.find('{', index)
        if start < 0:
            return rows
        end = closing(text, start)
        rows.append(text[start + 1:end])
        index = end + 1


# ---------- species data ----------
def species_aliases(root=ROOT):
    return dict(re.findall(r'^#define\s+(SPECIES_\w+)\s+(SPECIES_\w+)\s*$', read(root / 'include/constants/species.h'), re.M))


def canonical(name, aliases):
    seen = set()
    while name in aliases and name not in seen:
        seen.add(name)
        name = aliases[name]
    return name


def load_species(root=ROOT):
    """{SPECIES_*: record} for every species entry the Wayfarer build compiles (baseHP > 0)."""
    aliases = species_aliases(root)
    text = cpp(read(root / 'src/data/pokemon/species_info.h'), root,
               extra=('-include', 'config/pokemon.h', '-I', str(root / 'src/data/pokemon')))
    species = {}
    for match in re.finditer(r'\[\s*(SPECIES_\w+)\s*\]\s*=\s*\{', text):
        name, start = match.group(1), match.end() - 1
        body = text[start:closing(text, start) + 1]
        hp = re.search(r'\.baseHP\s*=\s*(\d+)', body)
        if not hp or int(hp.group(1)) == 0:
            continue
        nat = re.search(r'\.natDexNum\s*=\s*(\w+)', body)
        evolutions = []
        found = re.search(r'\.evolutions\s*=', body)
        if found:
            list_start = body.find('{', found.end())
            for row in braced_rows(body[list_start + 1:closing(body, list_start)]):
                fields = [part.strip() for part in re.split(r',(?![^(]*\))', row, maxsplit=3)]
                if fields[0] == 'EVOLUTIONS_END':
                    continue
                conditions = [(kind, value.strip()) for kind, value in re.findall(r'\{\s*(IF_\w+)\s*,?\s*([^{}]*?)\s*\}', row)]
                evolutions.append({'method': fields[0], 'param': fields[1], 'target': canonical(fields[2], aliases),
                                   'conditions': conditions})
        eggs = re.search(r'\.eggGroups\s*=\s*\{([^}]*)\}', body)
        gender = re.search(r'\.genderRatio\s*=\s*([^,\n]+),', body)
        record = {'nat': nat.group(1) if nat else None, 'evolutions': evolutions,
                  'eggGroups': [g.strip() for g in eggs.group(1).split(',')] if eggs else [],
                  'gender': gender.group(1).strip() if gender else '',
                  'items': re.findall(r'\.item(?:Common|Rare)\s*=\s*(ITEM_\w+)', body),
                  'learnsets': re.findall(r'\.(?:levelUpLearnset|eggMoveLearnset)\s*=\s*(\w+)', body)}
        for flag in FLAGS:
            record[flag] = bool(re.search(r'\.%s\s*=\s*(TRUE|1)\b' % flag, body))
        species[name] = record
    if len(species) < 900:
        raise AuditError(f'only {len(species)} species parsed from species_info.h')
    return species, aliases


LEVEL_UP_FILES = {'GEN_1': 'gen_1', 'GEN_2': 'gen_2', 'GEN_4': 'gen_4', 'GEN_5': 'gen_5', 'GEN_6': 'gen_6',
                  'GEN_7': 'gen_7', 'GEN_8': 'gen_8', 'GEN_9': 'gen_9', 'GEN_LATEST': 'gen_9'}


def load_learnsets(root=ROOT):
    """({learnset symbol: moves} for the configured level-up learnsets and egg moves,
    {SPECIES_*: moves} a TM or tutor may teach (all_learnables.json, the teachables generator's input))."""
    generation = re.search(r'#define\s+P_LVL_UP_LEARNSETS\s+(\w+)', read(root / 'include/config/pokemon.h')).group(1)
    learnsets = {}
    for path in (root / f'src/data/pokemon/level_up_learnsets/{LEVEL_UP_FILES[generation]}.h', root / 'src/data/pokemon/egg_moves.h'):
        for name, body in re.findall(r'\b(s\w+)\[\]\s*=\s*\{(.*?)\};', read(path), re.S):
            learnsets[name] = set(re.findall(r'\bMOVE_\w+', body))
    teachable = {'SPECIES_' + name: set(moves) for name, moves in json.loads(read(root / 'src/data/pokemon/all_learnables.json')).items()}
    return learnsets, teachable


def national_entries(species, aliases, root=ROOT):
    """[(NATIONAL_DEX_*, SPECIES_*)] in National order for every enabled entry: what the
    completion checks read through NationalPokedexNumToSpecies."""
    order = re.findall(r'^\s*(NATIONAL_DEX_\w+)\s*,', read(root / 'include/constants/pokedex.h'), re.M)
    source = read(root / 'src/pokemon.c')
    table = source[source.index('sNationalToSpeciesOrder'):]
    table = table[:table.index('};')]
    mapping = {f'NATIONAL_DEX_{name}': canonical(f'SPECIES_{name}', aliases)
               for name in re.findall(r'NATIONAL_TO_SPECIES\((\w+)\)', table)}
    mapping.update({nat: canonical(sp, aliases) for nat, sp in re.findall(r'\[(NATIONAL_DEX_\w+)\s*-\s*1\]\s*=\s*(SPECIES_\w+)', table)})
    return [(nat, mapping[nat]) for nat in order if nat != 'NATIONAL_DEX_NONE' and mapping.get(nat) in species]


# ---------- the compiled Wayfarer world ----------
def load_world(root=ROOT):
    """(maps {MAP_*: map.json}, script owners compiled) from the Wayfarer mapjson outputs."""
    groups, constants = root / 'data/maps/groups.inc', root / 'include/constants/map_groups.h'
    if not groups.is_file() or not constants.is_file():
        raise AuditError('missing data/maps/groups.inc: run `make BUILD=wayfarer generated` first')
    names, in_group = [], False
    for line in read(groups).splitlines():
        label = re.match(r'^(\w+)::', line)
        if label:
            in_group = label.group(1) != 'gMapGroups'
            continue
        item = re.match(r'^\s*\.4byte\s+(\w+)', line)
        if item and in_group:
            names.append(item.group(1))
    maps, owners = {}, set()
    for name in names:
        if name == 'NULL':  # a deselected map keeps its slot but has no header
            continue
        document = json.loads(read(root / 'data/maps' / name / 'map.json'))
        maps[document['id']] = document
        owners.add(name)
        header = root / 'data/maps' / name / 'header.inc'
        if header.is_file():
            label = re.search(r'\.4byte\s+(\w+)_MapScripts', read(header))
            if label:
                owners.add(label.group(1))
    if len(maps) < 1000:
        raise AuditError(f'only {len(maps)} compiled maps found in {groups}')
    return maps, owners


def drop_false_blocks(text):
    """Drop gas `.if`/`.else` branches whose numeric condition (after cpp) is false; an unknown one is kept."""
    out, stack, active = [], [], True
    for line in text.splitlines():
        head = re.match(r'\s*\.(if|else|endif)\b\s*(.*)$', line)
        if head is None:
            if active:
                out.append(line)
            continue
        kind, condition = head.groups()
        if kind == 'if':
            value = None
            if re.fullmatch(r'[\d\s()]+', condition):  # IS_FRLG and friends are plain numbers after cpp
                value = bool(eval(condition, {'__builtins__': {}}, {}))
            stack.append((active, value))
            active = active and value is not False
        elif kind == 'else' and stack:
            enclosing, value = stack[-1]
            active = enclosing and value is not True
        elif kind == 'endif' and stack:
            active = stack.pop()[0]
    return '\n'.join(out)


FILE_MARK = 'WAYFARER_POKEDEX_AUDIT_FILE'


def script_files(owners, root=ROOT):
    """{relative path: text} of the Wayfarer script assembly's playable files, as the build sees them:
    event_scripts.s through cpp with its gas `.if IS_FRLG` blocks dropped, and each included file's own
    `#if` branches evaluated (the build inlines includes before cpp)."""
    assembly = drop_false_blocks(cpp(read(root / 'data/event_scripts.s'), root, includes=False))
    chosen = []
    for relative in re.findall(r'^\s*\.include\s+"(data/[^"]+\.inc)"', assembly, re.M):
        owner = re.match(r'data/maps/([^/]+)/scripts\.inc$', relative)
        if relative in NOT_PLAYABLE_SCRIPTS or (owner and owner.group(1) not in owners):
            continue
        if (root / relative).is_file() and relative not in chosen:
            chosen.append(relative)
    # One cpp run over every file; species and item constants stay names because no constants header is read.
    joined = '\n'.join(f'{FILE_MARK} {relative}\n' + re.sub(r'@.*', '', read(root / relative)) for relative in chosen)
    processed = cpp(joined, root, includes=False, extra=('-x', 'assembler-with-cpp'))
    files = {}
    for chunk in processed.split(FILE_MARK + ' ')[1:]:
        relative, _, body = chunk.partition('\n')
        files[relative.strip()] = body
    if len(files) < 500 or set(files) != set(chosen):
        raise AuditError(f'only {len(files)} of {len(chosen)} playable script files survived preprocessing')
    return files


# ---------- sources ----------
def add(found, species, source):
    found.setdefault(species, []).append(source)


def wild_sources(maps, aliases, root=ROOT):
    found = {}
    for region in REGION_FILES:
        for key, tables in json.loads(read(root / f'src/data/wild_encounters_v2/{region}.json')).items():
            map_id = key.split(':')[0]
            if map_id not in maps:
                raise AuditError(f'{region}.json: {key} is not a compiled Wayfarer map')
            for method, table in tables.items():
                if not isinstance(table, dict):
                    continue
                for name in {s for time in ('day', 'night') for s in table.get(time, ()) if s != 'NONE'}:
                    add(found, canonical('SPECIES_' + name, aliases), f'wild {key} {method}')
    return found


LABEL = re.compile(r'^(\w+)::?[ \t]*$', re.M)
SCRIPT_END = re.compile(r'^\s*(end|return|goto|step_end|releaseall|\.\w+)\b')


def live_offsets(text, references):
    """The (start, end) spans of a script file that a referenced label reaches, directly or by
    running on past the next label (a body that does not end falls through)."""
    marks = list(LABEL.finditer(text))
    spans, live = [], False
    for index, mark in enumerate(marks):
        end = marks[index + 1].start() if index + 1 < len(marks) else len(text)
        body = text[mark.end():end]
        live = mark.group(1) in references or live
        if live:
            spans.append((mark.end(), end))
        lines = [line for line in body.splitlines() if line.strip()]
        if not lines or SCRIPT_END.match(lines[-1]):
            live = False
    return spans


def label_references(files, maps, root=ROOT):
    """Labels used anywhere but their own definition: map events and map script tables, other
    scripts, and C (specials and callbacks that start scripts by label)."""
    counts = defaultdict(int)
    for text in files.values():
        for token in re.findall(r'\b[A-Za-z_]\w*\b', LABEL.sub('', text)):
            counts[token] += 1
    for document in maps.values():
        for token in re.findall(r'"(\w+)"', json.dumps(document)):
            counts[token] += 1
    for path in list((root / 'src').glob('*.c')) + list((root / 'src/data').rglob('*.h')):
        for token in set(re.findall(r'\b\w*EventScript\w*\b|\b\w*_Script\w*\b', read(path))):
            counts[token] += 1
    return {token for token, count in counts.items() if count}


def script_sources(files, aliases, references):
    found = {}
    for relative, text in files.items():
        place = relative.split('/')[2] if relative.startswith('data/maps/') and relative.count('/') == 3 else relative
        live = ''.join(text[start:end] for start, end in live_offsets(text, references))
        for command, argument in re.findall(r'^\s*(%s)\s+([^\n]*)' % '|'.join(GIVERS), live, re.M):
            arguments = [part.strip() for part in argument.split(',')]
            candidates = [arguments[0]]
            if command == 'setwildbattle' and len(arguments) >= 4:
                candidates.append(arguments[3])
            for candidate in candidates:
                if candidate.startswith('SPECIES_'):
                    add(found, canonical(candidate, aliases), f'{command} {place}')
                elif candidate.startswith('VAR_'):
                    for name in re.findall(r'^\s*setvar\s+%s\s*,\s*(SPECIES_\w+)' % candidate, live, re.M):
                        add(found, canonical(name, aliases), f'{command} {candidate} {place}')
    return found


def roamer_sources(files, aliases, root=ROOT):
    text = cpp(read(root / 'src/roamer.c'), root, includes=False)
    called = {name for body in files.values() for name in re.findall(r'^\s*special\s+(\w+)', body, re.M)}
    found = {}
    for function in called:
        match = re.search(r'^void\s+%s\s*\(\s*void\s*\)\s*\{' % function, text, re.M)
        if match:
            start = match.end() - 1
            for name in re.findall(r'TryAddRoamer\(\s*(SPECIES_\w+)', text[start:closing(text, start)]):
                add(found, canonical(name, aliases), f'roamer special {function}')
    return found


def trade_sources(files, aliases, root=ROOT):
    text = cpp(read(root / 'src/data/trade.h'), root, includes=False)
    table = {}
    for match in re.finditer(r'\[\s*(INGAME_TRADE_\w+)\s*\]\s*=\s*\{', text):
        body = text[match.end() - 1:closing(text, match.end() - 1)]
        species = re.search(r'\.species\s*=\s*(SPECIES_\w+)', body)
        requested = re.search(r'\.requestedSpecies\s*=\s*(SPECIES_\w+)', body)
        if species:
            table[match.group(1)] = (canonical(species.group(1), aliases),
                                     canonical(requested.group(1), aliases) if requested else 'SPECIES_NONE')
    found = {}
    for relative, body in files.items():
        for trade in set(re.findall(r'\b(INGAME_TRADE_\w+)\b', body)):
            if trade in table:
                species, requested = table[trade]
                add(found, species, f'trade {trade} for {requested} {relative.split("/")[2] if relative.count("/") == 3 else relative}')
    return found


def item_sources(maps, files, root=ROOT):
    """{ITEM_*: [where]} for items the world hands out: map events (item balls, hidden items),
    scripts (gifts, finds, marts), daily world item slots and pools."""
    found = defaultdict(list)
    for map_id, document in maps.items():
        for item in set(re.findall(r'"(ITEM_\w+)"', json.dumps(document))):
            found[item].append(f'map {map_id}')
    for relative, text in files.items():
        for item in set(re.findall(r'^\s*(?:giveitem|finditem|additem|\.2byte)\s+(ITEM_\w+)', text, re.M)):
            found[item].append(f'script {relative}')
    for name in ('spots.h', 'pools.h'):
        path = root / 'src/data/item_slots' / name
        for item in set(re.findall(r'\b(ITEM_\w+)\b', read(path))):
            found[item].append(f'world item slot {name}')
    return found


def daycare_regions(maps):
    """Regions of compiled maps with a Day Care (where eggs are made)."""
    return {document.get('region') for document in maps.values()
            if re.search(r'Day_?Care', document['name'], re.I)}


# ---------- closure ----------
class Audit:
    def __init__(self, root=ROOT):
        self.species, self.aliases = load_species(root)
        self.entries = national_entries(self.species, self.aliases, root)
        self.maps, owners = load_world(root)
        self.files = script_files(owners, root)
        self.sources = {
            'wild': wild_sources(self.maps, self.aliases, root),
            'script': script_sources(self.files, self.aliases, label_references(self.files, self.maps, root)),
            'roamer': roamer_sources(self.files, self.aliases, root),
            'trade': trade_sources(self.files, self.aliases, root),
        }
        for kind, found in self.sources.items():
            for name in found:
                if name not in self.species:
                    raise AuditError(f'{kind} source names {name}, which the Wayfarer build does not enable')
        self.items = item_sources(self.maps, self.files, root)
        self.mapsecs = {document.get('region_map_section') for document in self.maps.values()}
        self.regions = {document.get('region') for document in self.maps.values()} - {None}
        self.daycares = daycare_regions(self.maps)
        self.learnsets, self.teachable = load_learnsets(root)
        self.prepare_breeding()
        self.solve()

    # --- evolution conditions ---
    def item_available(self, item):
        if item in self.items:
            return True
        # A wild Pokémon's held item comes with it when caught.
        return any(item in self.species[s]['items'] for s in self.sources['wild'])

    def can_learn(self, name, move):
        """By level-up or as an egg move; else a move a TM or tutor may teach (assumed in the world)."""
        record = self.species[name]
        if any(move in self.learnsets.get(symbol, ()) for symbol in record['learnsets']):
            return True
        return move in self.teachable.get(name, ()) or move in self.teachable.get(self.base_name(name), ())

    def blocked(self, evolution, parent=None):
        """Why an evolution of `parent` cannot happen here, or None when it can."""
        method, param = evolution['method'], evolution['param']
        if method not in SOLO_METHODS:
            return f'{method} only'
        if method == 'EVO_ITEM' and not self.item_available(param):
            return f'{param} is not in the world'
        reasons = []
        for kind, value in evolution['conditions']:
            if kind in ALWAYS_MET:
                continue
            if kind in ('IF_HOLD_ITEM', 'IF_BAG_ITEM_COUNT', 'IF_DEFEAT_X_WITH_ITEMS'):
                item = value.split(',')[0].strip()
                if not self.item_available(item):
                    reasons.append(f'{item} is not in the world')
            elif kind == 'IF_KNOWS_MOVE':
                if parent and not self.can_learn(parent, value):
                    reasons.append(f'{parent} cannot learn {value}')
            elif kind == 'IF_IN_MAPSEC':
                if value not in self.mapsecs:
                    reasons.append(f'{value} is not a Wayfarer place')
            elif kind == 'IF_IN_MAP':
                if value not in self.maps:
                    reasons.append(f'{value} is not a Wayfarer map')
            elif kind == 'IF_REGION':
                if value not in self.regions:
                    reasons.append(f'{value} is not a Wayfarer region')
            elif kind == 'IF_NOT_REGION':
                if not self.regions - {value}:
                    reasons.append(f'every place is {value}')
            elif kind == 'IF_SPECIES_IN_PARTY':
                if canonical(value, self.aliases) not in self.obtained:
                    reasons.append(f'{value} is not obtainable')
            elif kind == 'IF_TRADE_PARTNER_SPECIES':
                reasons.append('trade only')
            else:
                raise AuditError(f'unhandled evolution condition {kind}')
        return '; '.join(reasons) or None

    # --- breeding ---
    def prepare_breeding(self):
        """Precomputed egg species and Day Care regional forms (DetermineEggSpeciesAndParentSlots)."""
        previous = {}
        for name, record in self.species.items():
            for evolution in record['evolutions']:
                previous.setdefault(evolution['target'], name)
        self.previous = previous

        def first_stage(name):
            seen = set()
            while name in previous and name not in seen:
                seen.add(name)
                name = previous[name]
            return name
        self.egg = {name: first_stage(name) for name in self.species}
        # First stages grouped by base species name, with the region each form belongs to (None: no region).
        family = defaultdict(dict)
        for name, record in self.species.items():
            if self.egg[name] != name:
                continue
            regions = [region for flag, region in FORM_REGIONS.items() if record[flag]]
            family[self.base_name(name)][name] = regions[0] if regions else None
        self.egg_forms = {}
        for name in self.species:
            egg = self.egg[name]
            forms = family[self.base_name(egg)]
            out = {egg}
            for region in self.daycares:
                local = [form for form, form_region in forms.items() if form_region == region]
                if local:
                    out.update(local)  # the Day Care's region turns a foreign form into its own
                elif forms.get(egg) is not None:
                    out.update(form for form, form_region in forms.items() if form_region is None)
            for egg_form in list(out):
                out |= EGG_SWAPS.get(egg_form, set())
            if 'SPECIES_MANAPHY' in out:
                out.discard('SPECIES_MANAPHY')  # a Manaphy egg always hatches Phione
            self.egg_forms[name] = out

    @staticmethod
    def base_name(name):
        return re.sub(r'_(ALOLA|GALAR|HISUI|PALDEA)\w*$', '', name)

    def breedable(self, name):
        record = self.species[name]
        if not record['eggGroups'] or 'EGG_GROUP_NO_EGGS_DISCOVERED' in record['eggGroups']:
            return False
        needs_ditto = record['gender'] in ('MON_GENDERLESS', 'MON_MALE')
        return not needs_ditto or 'SPECIES_DITTO' in self.obtained

    def solve(self):
        self.how = {}
        for kind, found in self.sources.items():
            if kind == 'trade':
                continue  # a trade needs the Pokémon it asks for (below)
            for name, where in found.items():
                self.how.setdefault(name, f'{kind}: {where[0]}')
        self.obtained = set(self.how)
        changed = True
        while changed:
            changed = False
            for name, where in self.sources['trade'].items():
                if name in self.obtained:
                    continue
                for trade in where:
                    requested = trade.split()[3]
                    if requested == 'SPECIES_NONE' or requested in self.obtained:
                        self.obtained.add(name)
                        self.how[name] = f'trade: {trade}'
                        changed = True
                        break
            for name in sorted(self.obtained):
                record = self.species[name]
                for evolution in record['evolutions']:
                    target = evolution['target']
                    if target in self.species and target not in self.obtained and self.blocked(evolution, name) is None:
                        self.obtained.add(target)
                        self.how[target] = f'evolve: {name} {evolution["method"]} {evolution["param"]}'
                        changed = True
                if self.breedable(name):
                    for child in self.egg_forms[name]:
                        if child in self.species and child not in self.obtained:
                            self.obtained.add(child)
                            self.how[child] = f'breed: {name} at a Day Care'
                            changed = True

    # --- result ---
    def forms_of(self, nat):
        return [name for name, record in self.species.items() if record['nat'] == nat]

    def reason(self, nat):
        """Why no form of a National entry is obtainable."""
        reasons = []
        for name in self.forms_of(nat):
            for parent, record in self.species.items():
                for evolution in record['evolutions']:
                    if evolution['target'] == name and parent in self.obtained:
                        reasons.append(f'evolves from {parent} only by {evolution["method"]} {evolution["param"]}: {self.blocked(evolution, parent)}')
        if reasons:
            return sorted(set(reasons))[0]
        return 'no wild, static, gift, trade or family source'

    def unobtainable(self):
        """{NATIONAL_DEX_*: reason} for every enabled entry with no obtainable form."""
        return {nat: self.reason(nat) for nat, _ in self.entries
                if not any(form in self.obtained for form in self.forms_of(nat))}

    def how_for(self, nat):
        """How the first obtainable form of a National entry is obtained."""
        return next((self.how[form] for form in self.forms_of(nat) if form in self.obtained), 'not obtainable')

    def required(self, nat, species):
        record = self.species[species]
        return not (record['dexNotRequired'] or (record['isMythical'] and not record['dexForceRequired']))


def record_document(audit):
    unobtainable = audit.unobtainable()
    marked = {nat for nat, species in audit.entries if audit.species[species]['dexNotRequired']}
    return {
        'version': 1,
        'enabledEntries': len(audit.entries),
        'unobtainable': [{'nat': nat, 'species': species, 'mythical': audit.species[species]['isMythical'],
                          'reason': unobtainable[nat]}
                         for nat, species in audit.entries if nat in unobtainable],
        # Marked before this audit and kept by decision (Gen IV, regional forms and others); see the report.
        'excusedButObtainable': [{'nat': nat, 'species': species}
                                 for nat, species in audit.entries if nat in marked and nat not in unobtainable],
    }


def problems(audit, document):
    """Drift between the computed result, the record and the species data's flags."""
    out = []
    computed = record_document(audit)
    recorded = {row['nat'] for row in document['unobtainable']}
    now = {row['nat'] for row in computed['unobtainable']}
    for nat in sorted(now - recorded):
        out.append(f'{nat} is no longer obtainable ({computed_reason(computed, nat)}): '
                   'give it a source, or mark it dexNotRequired and re-record with --write')
    for nat in sorted(recorded - now):
        out.append(f'{nat} is now obtainable ({audit.how_for(nat)}): '
                   're-record with --write')
    excused = {row['nat'] for row in document.get('excusedButObtainable', ())}
    for nat in sorted({row['nat'] for row in computed['excusedButObtainable']} - excused):
        out.append(f'{nat} is obtainable ({audit.how_for(nat)}) but marked dexNotRequired: clear the flag, '
                   'or re-record with --write if excusing it is a decision')
    for nat in sorted(excused - {row['nat'] for row in computed['excusedButObtainable']} - now):
        out.append(f'{nat} is no longer excused: re-record with --write')
    by_nat = dict(audit.entries)
    for nat in sorted(now):
        species = by_nat[nat]
        if audit.required(nat, species):
            out.append(f'{nat} ({species}) is unobtainable but the Pokédex requires it: mark it dexNotRequired')
    return out


def c_table(document):
    """The C test's table body: the recorded unobtainable entries."""
    rows = [f"    {row['nat']}," for row in document['unobtainable']]
    return '\n'.join([C_BEGIN, *rows, C_END])


def c_test_drift(document, path=C_TEST):
    """A message when the C test's table is not the recorded set, else None."""
    text = read(path)
    if C_BEGIN not in text or C_END not in text:
        return f'{path.name} has no recorded table markers'
    current = text[text.index(C_BEGIN):text.index(C_END) + len(C_END)]
    if current != c_table(document):
        return f'{path.name} does not match obtainability.json: re-record with --write'
    return None


def write_c_test(document, path=C_TEST):
    text = read(path)
    text = text[:text.index(C_BEGIN)] + c_table(document) + text[text.index(C_END) + len(C_END):]
    path.write_text(text)


def computed_reason(document, nat):
    return next(row['reason'] for row in document['unobtainable'] if row['nat'] == nat)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    parser.add_argument('--check', action='store_true', help='fail on drift from obtainability.json')
    parser.add_argument('--write', action='store_true', help='re-record obtainability.json')
    parser.add_argument('--explain', metavar='SPECIES', help='print how a species is obtained')
    args = parser.parse_args(argv)
    try:
        audit = Audit()
    except AuditError as error:
        print(f'obtainability: {error}', file=sys.stderr)
        return 1
    if args.explain:
        name = canonical(args.explain if args.explain.startswith('SPECIES_') else 'SPECIES_' + args.explain, audit.aliases)
        print(f'{name}: {audit.how.get(name, "not obtainable")}')
        return 0
    document = record_document(audit)
    if args.write:
        RECORD.write_text(json.dumps(document, indent=1) + '\n')
        write_c_test(document)
    recorded = json.loads(read(RECORD)) if RECORD.is_file() else {'unobtainable': [], 'enabledEntries': 0}
    found = problems(audit, recorded)
    if recorded.get('enabledEntries') != len(audit.entries):
        found.append(f"the build enables {len(audit.entries)} National entries, the record {recorded.get('enabledEntries')}: "
                     're-record with --write')
    drift = c_test_drift(recorded)
    if drift:
        found.append(drift)
    for line in found:
        print(f'obtainability: {line}', file=sys.stderr)
    if not args.check and not args.write:
        for row in document['unobtainable']:
            print(f"{row['nat']:32} {row['species']:28} {'mythical ' if row['mythical'] else ''}{row['reason']}")
    return 1 if found and (args.check or args.write) else 0


if __name__ == '__main__':
    sys.exit(main())
