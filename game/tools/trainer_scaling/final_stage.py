#!/usr/bin/env python3
"""Author regular Trainer rosters at final evolution stages.

The regular scaler steps every species down through the shared evolution-stage
table (game/tools/notable_trainers/evolution.py), so authored rosters should
name the *final* stage of each line and let the level decide the stage.  This
tool applies that authoring pass reproducibly:

  R1  every Pokemon becomes the final stage of its line (full evolution graph
      as compiled for Wayfarer)
  R2  regional forms / region-locked evolutions only for Trainers of that region
  R3  ordinary branches chosen deterministically by Trainer theme or id
  R4  duplicate cap: per team and per line only one copy reaches the final stage
      (the highest authored level, ties to the later slot); with three or more
      copies of a line the next one reaches the middle stage
  R5  identity exceptions keep the authored species
  R6  an authored ability the new species cannot have is dropped

A slot is only promoted to a species the runtime step-down can walk back to the
authored one, so babies (the stage table omits baby -> parent edges) keep their
species.

Usage:
  final_stage.py --report        rewrite final_stage_report.md (no source edits)
  final_stage.py --apply         edit the source .party files / selected rosters
  final_stage.py --check         fail when --apply would change anything

The report compares the authored species with the new ones, so it can only be
built from sources that still hold the authored species.  Once the sources are
final-staged it would be an empty "0 changed" report, so --report refuses to
write then and the committed report stays the record of the authoring pass.  To
rebuild it after changing the rules, restore the party sources and generated
trainer headers from before the authoring commit, then run --apply, which
rewrites the sources and the report together.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import re
import sys
import zlib
from collections import Counter, OrderedDict
from pathlib import Path

TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[1]
DATA = ROOT / 'src/data'
REPORT = ROOT / 'src/data/trainer_scaling/final_stage_report.md'
CONSTANTS = ROOT / 'include/constants/species.h'
PARTY_FILES = ('trainers.party', 'trainers_hns.party', 'trainers_wayfarer_local.party')
FRLG_PARTY = 'trainers_frlg.party'
TOWER_ROSTER = 'trainers_wayfarer_tower.h'

REGION_TAG = {'Alola': 'ALOLA', 'Sevii': 'GALAR', 'Sinjoh': 'HISUI'}
FORM_TAGS = ('ALOLA', 'GALAR', 'HISUI', 'PALDEA')
# Region-only evolutions whose species carries no form suffix.
HISUI_ONLY = frozenset(f'SPECIES_{n}' for n in (
    'KLEAVOR', 'URSALUNA', 'URSALUNA_BLOODMOON', 'WYRDEER', 'OVERQWIL', 'SNEASLER',
    'BASCULEGION_M', 'BASCULEGION_F'))

# Whole lines that keep their authored species (legendary / mythical / special).
SPECIAL_LINES = frozenset(f'SPECIES_{n}' for n in (
    'COSMOG', 'COSMOEM', 'SOLGALEO', 'LUNALA', 'TYPE_NULL', 'SILVALLY', 'KUBFU',
    'URSHIFU_SINGLE_STRIKE', 'URSHIFU_RAPID_STRIKE', 'POIPOLE', 'NAGANADEL',
    'MELTAN', 'MELMETAL', 'SHEDINJA'))

# Classes whose Pokemon keep their kid identity.
CHILD_CLASSES = ('TUBER', 'PRESCHOOLER', 'SCHOOL_KID', 'TWINS', 'SIS_AND_BRO')

FIGHTING = ('BLACK_BELT', 'BATTLE_GIRL', 'CRUSH_GIRL', 'CRUSH_KIN')
COOL = FIGHTING + ('COOLTRAINER', 'COOLTRAINER_2', 'EXPERT', 'NINJA_BOY', 'PKMN_RANGER', 'TRIATHLETE', 'DRAGON_TAMER')
PRETTY = ('AROMA_LADY', 'BEAUTY', 'LADY', 'LASS', 'PICNICKER', 'PARASOL_LADY', 'KIMONO_GIRL',
          'PAINTER', 'SWIMMER_F', 'TUBER_F', 'YOUNG_COUPLE', 'COOL_COUPLE')
PSYCHIC = ('PSYCHIC', 'PSYCHIC_M', 'SAGE', 'MEDIUM', 'HEX_MANIAC', 'CHANNELER')
EEVEE_THEMES = OrderedDict([
    ('SPECIES_FLAREON', ('KINDLER', 'FIREBREATHER', 'TEAM_MAGMA')),
    ('SPECIES_VAPOREON', ('SWIMMER_M', 'SWIMMER_F', 'FISHERMAN', 'SAILOR', 'TUBER_M', 'TUBER_F', 'TUBER',
                          'TEAM_AQUA', 'SIS_AND_BRO', 'SURFER')),
    ('SPECIES_JOLTEON', ('GUITARIST', 'ROCKER', 'ENGINEER', 'SCIENTIST', 'GAMBLER')),
    ('SPECIES_ESPEON', ('PSYCHIC', 'PSYCHIC_M', 'SAGE', 'MEDIUM', 'HEX_MANIAC', 'CHANNELER')),
    ('SPECIES_UMBREON', ('BIKER', 'TEAM_ROCKET', 'BURGLAR', 'CUE_BALL', 'TOUGH', 'RUIN_MANIAC')),
    ('SPECIES_LEAFEON', ('BUG_CATCHER', 'BUG_MANIAC', 'CAMPER', 'PKMN_RANGER', 'HIKER', 'POKEFAN', 'PKMN_BREEDER')),
    ('SPECIES_GLACEON', ('SKIER',)),
    ('SPECIES_SYLVEON', ('BEAUTY', 'LADY', 'LASS', 'PICNICKER', 'PARASOL_LADY', 'KIMONO_GIRL', 'AROMA_LADY',
                         'PAINTER', 'YOUNG_COUPLE', 'COOL_COUPLE', 'BATTLE_GIRL')),
])


# The party `Gender:` field is unreliable (several female classes say Male), so
# unmistakable classes decide the Trainer's gender before the field does.
FEMALE_CLASSES = ('LASS', 'BEAUTY', 'LADY', 'AROMA_LADY', 'PARASOL_LADY', 'KIMONO_GIRL', 'BATTLE_GIRL',
                  'CRUSH_GIRL', 'PICNICKER', 'SWIMMER_F', 'TUBER_F')
MALE_CLASSES = ('BLACK_BELT', 'BUG_CATCHER', 'CAMPER', 'HIKER', 'FISHERMAN', 'SAILOR', 'GENTLEMAN', 'SWIMMER_M',
                'TUBER_M', 'SUPER_NERD', 'YOUNGSTER', 'BIKER', 'JUGGLER', 'POKEMANIAC', 'RUIN_MANIAC', 'COLLECTOR',
                'FIREBREATHER', 'KINDLER', 'NINJA_BOY', 'GUITARIST', 'BIRD_KEEPER', 'BURGLAR', 'RICH_BOY',
                'CRUSH_KIN', 'DRAGON_TAMER', 'CUE_BALL', 'TAMER', 'BUG_MANIAC')


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules.setdefault(name, module)
    spec.loader.exec_module(module)
    return module


def class_key(trainer_class):
    """TRAINER_CLASS_SWIMMER_F_HNS -> SWIMMER_F."""
    name = trainer_class.removeprefix('TRAINER_CLASS_')
    for suffix in ('_HNS', '_FRLG'):
        name = name.removesuffix(suffix)
    return name


def stable_hash(*parts):
    return zlib.crc32(':'.join(parts).encode())


# --------------------------------------------------------------------------
# Evolution model


class Model:
    """Evolution graph plus the species facts the rules need."""

    def __init__(self, evolutions, babies, aliases, gender_ratio, abilities, numbers, step_back=None):
        self.babies = set(babies)
        # Runtime step-back edges {successor: predecessor}; None skips the check.
        self.step_back = step_back
        self.aliases = dict(aliases)
        self.ratio = gender_ratio
        self.abilities = abilities
        self.numbers = numbers  # species -> numeric id for stable ordering
        self.succ = {}
        self.pred = {}
        for species, edges in evolutions.items():
            targets = []
            for edge in edges:
                target = edge['target']
                # Shedinja is a side effect of Nincada's evolution, not a stage.
                if edge['method'] in ('EVO_NONE', 'EVO_SPLIT_FROM_EVO') or target not in evolutions:
                    continue
                if target not in targets:
                    targets.append(target)
            self.succ[species] = targets
            for target in targets:
                self.pred.setdefault(target, [])
                if species not in self.pred[target]:
                    self.pred[target].append(species)

    def canonical(self, species):
        seen = 0
        while species in self.aliases and seen < 8:
            species, seen = self.aliases[species], seen + 1
        return species

    def base(self, species):
        seen = set()
        while self.pred.get(species) and species not in seen:
            seen.add(species)
            species = self.pred[species][0]
        return species

    def depth(self, species):
        depth, seen = 0, set()
        while self.pred.get(species) and species not in seen:
            seen.add(species)
            species = self.pred[species][0]
            depth += 1
        return depth

    def reaches_back(self, species, authored):
        """True when the runtime step-down can walk from `species` to `authored`.

        The stage table omits baby -> parent edges, so a promoted species must
        never sit beyond one the table cannot step back from.
        """
        if self.step_back is None:
            return True
        for _ in range(8):
            if species == authored:
                return True
            species = self.step_back.get(species)
            if species is None:
                return False
        return False

    def gender_only(self, species):
        """'M' or 'F' when the species can only be that gender, else None."""
        ratio = self.ratio.get(species, '')
        if ratio == 'MON_MALE' or re.fullmatch(r'PERCENT_FEMALE\(0(\.0*)?\)', ratio):
            return 'M'
        if ratio == 'MON_FEMALE' or re.fullmatch(r'PERCENT_FEMALE\(100(\.0*)?\)', ratio):
            return 'F'
        return None

    def is_baby(self, species):
        return species in self.babies


def form_tag(species):
    for tag in FORM_TAGS:
        if species.endswith('_' + tag):
            return tag
    return None


def regional_tag(species):
    return 'HISUI' if species in HISUI_ONLY else form_tag(species)


def load_model():
    evolution = load_module('notable_evolution', ROOT / 'tools/notable_trainers/evolution.py')
    audit = load_module('trainer_scaling_audit', TOOL / 'audit.py')
    evolutions = evolution.game_evolutions()
    values, by_name = evolution.symbols()
    catalog = json.loads(evolution.CATALOG.read_text())
    babies = {evolution.species_symbol(name, by_name) for name in catalog['babies']}
    aliases = {}
    for alias, target in re.findall(r'^#define\s+(SPECIES_[A-Z0-9_]+)\s+(SPECIES_[A-Z0-9_]+)\s*$',
                                    CONSTANTS.read_text(), re.M):
        aliases[alias] = target
    species_data, _, _ = audit.load_data()
    ratio = {name: data['gender'] for name, data in species_data.items()}
    abilities = {name: [a for a in data['abilities'] if a != 'ABILITY_NONE'] for name, data in species_data.items()}
    step_back = {successor: predecessor for successor, (predecessor, _) in evolution.stage_table().items()}
    return Model(evolutions, babies, aliases, ratio, abilities, values, step_back)


# --------------------------------------------------------------------------
# Trainer context and the choice rules


class Context:
    """What the rules know about one Trainer."""

    def __init__(self, trainer_id, trainer_class, gender, region):
        self.id = trainer_id
        self.cls = class_key(trainer_class)
        self.region = region
        self.tag = REGION_TAG.get(region)
        if self.cls in FEMALE_CLASSES:
            gender = 'F'
        elif self.cls in MALE_CLASSES:
            gender = 'M'
        self.gender = gender  # 'M', 'F' or None

    def in_class(self, names):
        return self.cls in names


def eevee_target(ctx):
    for target, classes in EEVEE_THEMES.items():
        if ctx.in_class(classes):
            return target, 'theme'
    order = list(EEVEE_THEMES)
    return order[stable_hash(ctx.id, 'eevee') % len(order)], 'id'


def pick_by_name(candidates, wanted):
    for target in wanted:
        if target in candidates:
            return target
    return None


def choose(model, ctx, species, pokemon_gender, candidates=None):
    """Pick the next evolution of `species` for this Trainer, or (None, why)."""
    source_tag = regional_tag(species)
    options = list(model.succ.get(species, [])) if candidates is None else list(candidates)
    tags = set()
    allowed = []
    for target in options:
        tag = regional_tag(target)
        if tag is not None and tag != ctx.tag and tag != source_tag:
            continue
        allowed.append(target)
    if len(allowed) != len(options):
        tags.add('R2')
    options = allowed
    preferred = [t for t in options if ctx.tag and regional_tag(t) == ctx.tag]
    if preferred and len(preferred) != len(options):
        options = preferred
        tags.add('R2')
    elif preferred and ctx.tag and any(regional_tag(t) == ctx.tag for t in options):
        tags.add('R2')
    if pokemon_gender:
        options = [t for t in options if model.gender_only(t) in (None, pokemon_gender)]
    if not options:
        return None, tags
    if len(options) == 1:
        return options[0], tags
    tags.add('R3')
    male = ctx.gender == 'M'
    female = ctx.gender == 'F'
    if pokemon_gender:
        male, female = pokemon_gender == 'M', pokemon_gender == 'F'
    pick = None
    if species == 'SPECIES_POLIWHIRL':
        pick = pick_by_name(options, ['SPECIES_POLIWRATH' if ctx.in_class(FIGHTING) else 'SPECIES_POLITOED'])
    elif species == 'SPECIES_GLOOM':
        pick = pick_by_name(options, ['SPECIES_BELLOSSOM' if ctx.in_class(PRETTY) else 'SPECIES_VILEPLUME'])
    elif species in ('SPECIES_SLOWPOKE', 'SPECIES_SLOWPOKE_GALAR'):
        king = ctx.in_class(PSYCHIC)
        wanted = ['SPECIES_SLOWKING' if king else 'SPECIES_SLOWBRO', 'SPECIES_SLOWKING_GALAR' if king else 'SPECIES_SLOWBRO_GALAR']
        pick = pick_by_name(options, wanted)
    elif species == 'SPECIES_KIRLIA':
        gallade = male and not female and ctx.in_class(COOL)
        pick = pick_by_name(options, ['SPECIES_GALLADE' if gallade else 'SPECIES_GARDEVOIR'])
    elif species == 'SPECIES_SNORUNT':
        pick = pick_by_name(options, ['SPECIES_FROSLASS' if female and not male else 'SPECIES_GLALIE'])
    elif species.startswith('SPECIES_BURMY'):
        suffix = species.removeprefix('SPECIES_BURMY')
        kind = 'WORMADAM' if female and not male else 'MOTHIM'
        pick = pick_by_name(options, [f'SPECIES_{kind}{suffix}'])
    elif species == 'SPECIES_ESPURR':
        pick = pick_by_name(options, ['SPECIES_MEOWSTIC_F' if female and not male else 'SPECIES_MEOWSTIC_M'])
    elif species == 'SPECIES_EEVEE':
        target, why = eevee_target(ctx)
        pick = pick_by_name(options, [target])
        tags.add('R3:' + why)
    if pick is None:
        options = sorted(options, key=lambda t: model.numbers.get(t, 0))
        pick = options[stable_hash(ctx.id, species) % len(options)]
        tags.add('R3:id')
    return pick, tags


def final_path(model, ctx, species, pokemon_gender):
    """Walk forward to the last stage the runtime can step back from.

    Normally that is the final stage of the line; a baby (no stage-table edge to
    its parent) therefore keeps its species.

    Returns (path, tags, gender) where path starts with `species`, gender is the
    gender the Pokemon must be set to ('M'/'F'/None), tags name the rules used.
    """
    path, tags, forced = [species], set(), None
    current = species
    for _ in range(8):
        target, step_tags = choose(model, ctx, current, forced or pokemon_gender)
        tags |= step_tags
        if target is None or not model.reaches_back(target, species):
            break
        only = model.gender_only(target)
        # A free gender choice that becomes single-gender must be set explicitly.
        if only and model.gender_only(current) is None and not pokemon_gender:
            forced = only
        path.append(target)
        current = target
    return path, tags, forced


# --------------------------------------------------------------------------
# Team planning (R1, R4, R5)


class Mon:
    def __init__(self, slot, species, level, item, gender, ability, exception=None):
        self.slot = slot
        self.species = species
        self.level = level
        self.item = item
        self.gender = gender      # explicit 'M'/'F' or None
        self.ability = ability    # ability constant or None
        self.exception = exception
        self.after = species
        self.new_gender = None
        self.tags = []
        self.note = None
        self.drop_ability = False


def plan_team(model, ctx, mons):
    """Decide each slot's final species; mutates and returns `mons`."""
    for mon in mons:
        mon.species = model.canonical(mon.species)
        mon.after = mon.species
    if mons and all(model.is_baby(m.species) for m in mons):
        for mon in mons:
            mon.exception = mon.exception or 'baby-only team'
    if ctx.cls in CHILD_CLASSES or any(ctx.cls.startswith(c + '_') for c in CHILD_CLASSES):
        for mon in mons:
            mon.exception = mon.exception or 'child class'
    for mon in mons:
        if mon.item == 'ITEM_EVERSTONE':
            mon.exception = mon.exception or 'Everstone'
        elif mon.item == 'ITEM_EVIOLITE':
            mon.exception = mon.exception or 'Eviolite'
        base = model.base(mon.species)
        if mon.species in SPECIAL_LINES or base in SPECIAL_LINES:
            mon.exception = mon.exception or 'legendary/special line'
    lines = OrderedDict()
    for mon in mons:
        lines.setdefault(model.base(mon.species), []).append(mon)
    for base, members in lines.items():
        movable = [m for m in members if not m.exception]
        plans = {}
        for mon in members:
            path, tags, forced = final_path(model, ctx, mon.species, mon.gender)
            plans[mon.slot] = (path, tags, forced)
        ranked = sorted(movable, key=lambda m: (-m.level, -m.slot))
        if not ranked:
            continue
        # The line's final depth follows the highest-ranked copy that can move.
        final_depth = max(model.depth(plans[m.slot][0][-1]) for m in ranked)
        mid_depth = final_depth - 1
        copies = len(members)
        final_used = sum(1 for m in members if model.depth(m.species) >= final_depth)
        winner = None
        if final_used == 0:
            winner = next((m for m in ranked if model.depth(plans[m.slot][0][-1]) > model.depth(m.species)), None)
        if winner is not None:
            path, tags, forced = plans[winner.slot]
            winner.after, winner.new_gender = path[-1], forced
            winner.tags += sorted(tags | {'R1'})
        if copies >= 3 and mid_depth >= 1:
            mid_used = sum(1 for m in members if model.depth(m.after) == mid_depth)
            if mid_used == 0:
                for mon in ranked:
                    path, tags, forced = plans[mon.slot]
                    steps = [p for p in path if model.depth(p) == mid_depth]
                    if mon is winner or mon.after != mon.species or not steps \
                            or model.depth(mon.species) >= mid_depth:
                        continue
                    mon.after = steps[0]
                    mon.new_gender = forced if model.gender_only(mon.after) else None
                    mon.tags += sorted(tags | {'R1', 'R4:mid'})
                    break
        for mon in ranked:
            if mon.after == mon.species and len(plans[mon.slot][0]) > 1:
                mon.note = 'R4 cap'
                mon.tags.append('R4')
        # Forced gender only matters when a single-gender stage was reached.
    for mon in mons:
        if mon.exception and mon.after == mon.species:
            mon.tags = ['R5:' + mon.exception]
        if mon.after == mon.species:
            mon.new_gender = None
        elif mon.new_gender and mon.gender == mon.new_gender:
            mon.new_gender = None
    return mons


def ability_drop(model, mon):
    """R6: an authored ability the new species cannot have is dropped."""
    return bool(mon.ability and mon.after != mon.species
                and mon.ability not in model.abilities.get(mon.after, []))


# --------------------------------------------------------------------------
# Party source parsing and editing

SPECIES_IDENT = r"[^()@\n]+?"
HEADER = re.compile(
    r"^(?P<first>" + SPECIES_IDENT + r")"
    r"(?:\s*\((?P<second>[^()]+)\))?"
    r"(?:\s*\((?P<third>[^()]+)\))?"
    r"(?P<item>\s*@\s*.+?)?(?P<tail>\s*)$")


def species_constant(token):
    """Mirror trainerproc's fprint_species."""
    token = token.strip()
    if token.startswith('SPECIES_'):
        return token
    out, underscore = [], False
    i = 0
    while i < len(token):
        c = token[i]
        if c.isascii() and (c.isalnum()):
            if underscore:
                out.append('_')
            underscore = False
            out.append(c.upper())
        elif c in "'%’":
            pass
        elif c == '♂':
            underscore = False
            out.append('_M')
        elif c == '♀':
            underscore = False
            out.append('_F')
        elif c == 'é':
            if underscore:
                out.append('_')
            underscore = False
            out.append('E')
        else:
            underscore = True
        i += 1
    return 'SPECIES_' + ''.join(out)


GENDERED_SPECIES = {'Basculegion': ('Basculegion-M', 'Basculegion-F'), 'Indeedee': ('Indeedee-M', 'Indeedee-F'),
                    'Oinkologne': ('Oinkologne-M', 'Oinkologne-F'), 'Meowstic': ('Meowstic-M', 'Meowstic-F'),
                    'Nidoran': ('Nidoran-M', 'Nidoran-F')}


def parse_header(line):
    match = HEADER.match(line)
    if not match:
        return None
    first, second, third = match.group('first'), match.group('second'), match.group('third')
    if second is None:
        nickname, species, gender, species_span = None, first, None, match.span('first')
    elif third is not None:
        nickname, species, gender = first, second, third
        species_span = match.span('second')
    elif len(second.strip()) == 1:
        nickname, species, gender, species_span = None, first, second.strip(), match.span('first')
    else:
        nickname, species, gender = first, second, None
        species_span = match.span('second')
    return {'nickname': nickname, 'species': species.strip(), 'gender': gender, 'span': species_span, 'match': match,
            'item': (match.group('item') or '').replace('@', '').strip() or None}


class PartyBlock:
    """One `=== TRAINER_X ===` block: raw lines plus parsed Pokemon entries."""

    ATTRIBUTE = re.compile(r'^(?:[A-Z][A-Za-z ]*:|- |[A-Za-z]+ Nature\s*$)')

    def __init__(self, trainer_id, lines):
        self.id = trainer_id
        self.lines = lines
        self.gender = None
        self.mons = []
        self._parse()

    def _parse(self):
        index = 1
        while index < len(self.lines) and self.lines[index].strip():
            if self.lines[index].startswith('Gender:'):
                self.gender = {'Male': 'M', 'Female': 'F'}.get(self.lines[index].split(':', 1)[1].strip())
            index += 1
        mon = None
        for i in range(index, len(self.lines)):
            line = self.lines[i]
            if not line.strip() or line.startswith('#'):
                mon = None
                continue
            if mon is None:
                parsed = parse_header(line)
                if parsed is None:
                    continue
                parsed['line'] = i
                parsed['ability_line'] = None
                parsed['other'] = []
                self.mons.append(parsed)
                mon = parsed
            elif line.startswith('Ability:'):
                mon['ability_line'] = i


def split_blocks(text):
    """Return ([(id, start, end)], lines) for a party file."""
    lines = text.split('\n')
    marks = [(i, re.match(r'^=== (TRAINER_\w+) ===\s*$', l)) for i, l in enumerate(lines)]
    marks = [(i, m.group(1)) for i, m in marks if m]
    spans = []
    for n, (i, trainer) in enumerate(marks):
        end = marks[n + 1][0] if n + 1 < len(marks) else len(lines)
        spans.append((trainer, i, end))
    return spans, lines


def render_species(constant, spellings):
    if constant in spellings:
        return spellings[constant]
    name = constant.removeprefix('SPECIES_')
    return '-'.join(part.capitalize() for part in name.split('_'))


def apply_edits(lines, block, decisions, spellings):
    """Mutate `lines` for one trainer; returns the number of edited slots."""
    edited = 0
    for mon, decision in sorted(zip(block.mons, decisions), key=lambda p: -p[0]['line']):
        if decision.after == decision.species and not decision.new_gender:
            continue
        i = mon['line']
        line = lines[i]
        start, end = mon['span']
        match = mon['match']
        if decision.after != decision.species:
            line = line[:start] + render_species(decision.after, spellings) + line[end:]
            end = start + len(render_species(decision.after, spellings))
        if decision.new_gender and not mon['gender']:
            # Insert " (M)" right after the species (and nickname parenthesis).
            insert = end + (1 if mon['nickname'] else 0)
            line = line[:insert] + f' ({decision.new_gender})' + line[insert:]
        elif decision.new_gender and mon['gender']:
            line = re.sub(r'\((?:M|F|Male|Female)\)', f'({decision.new_gender})', line, count=1)
        lines[i] = line
        edited += 1
        if decision.drop_ability and mon['ability_line'] is not None:
            lines[mon['ability_line']] = None
    return edited


# --------------------------------------------------------------------------
# Covered Trainer inventory

def gather():
    """Return covered trainer records: {id: info} plus source-file mapping."""
    generate = load_module('trainer_scaling_generate', TOOL / 'generate.py')
    raw = generate.load_inventory()
    manifest = json.loads(generate.MANIFEST.read_text())
    manifest = dict(manifest, records=manifest.get('records', []) + generate.load_sevii_manifest()
                    + generate.load_coast_manifest() + generate.load_anne_manifest()
                    + generate.load_celadon_hideout_manifest() + generate.load_tower_manifest()
                    + generate.load_indigo_manifest())
    records = generate.resolve_rosters(raw)
    covered = {}
    for row in manifest['records']:
        if row['policy'] in ('ORDINARY', 'GYM_MEMBER'):
            covered[row['id']] = {'policy': row['policy'], 'region': row['region'], 'record': records[row['id']],
                                  'raw': raw[row['id']]}
    exceptions = {(row['owner'], row['slot']) for row in manifest.get('move_exceptions', [])}
    return covered, exceptions


def frlg_sources():
    """Runtime id -> frlg source party id for the selected FRLG rosters."""
    repo = ROOT.parent
    result = {}
    for name, key in (('sevii', 'source_trainer'), ('coast', 'source_trainer'), ('ss-anne', 'source')):
        rows = json.loads((repo / f'docs/{name}-trainer-implementation/allocation-inventory.json').read_text())['allocation']['allocations']
        for row in rows:
            result[row['id']] = row[key]
    hideout = load_module('hideout_generate', ROOT / 'tools/wayfarer_celadon_hideout_trainers/generate.py')
    for source, target in zip(hideout.SOURCES, hideout.TARGETS):
        result[target] = source
    return result


def mon_from_record(slot, entry, exceptions, owner):
    item = entry.get('heldItem')
    gender = {'TRAINER_MON_MALE': 'M', 'TRAINER_MON_FEMALE': 'F'}.get(entry.get('gender'))
    ability = entry.get('ability')
    ability = ability if ability and ability != 'ABILITY_NONE' else None
    exception = 'reviewed move exception' if (owner, slot) in exceptions else None
    return Mon(slot, entry['species'], entry['lvl'], item, gender, ability, exception)


def plan_block_text(model, text, trainer_id, region, spellings=None):
    """Apply the authoring rules to one party block (without its `=== ID ===` line).

    Selected copies of donor parties are final-staged while the donor stays as
    authored, so contract tests compare the selected block with this planned
    donor block.  `trainer_id` is the runtime id of the selected copy because
    branch choices are hashed from it.
    """
    lines = [f'=== {trainer_id} ==='] + text.split('\n')
    block = PartyBlock(trainer_id, lines)
    trainer_class = next(l.split(':', 1)[1] for l in lines[1:] if l.startswith('Class:'))
    ctx = Context(trainer_id, 'TRAINER_CLASS_' + trainer_class.strip().upper().replace(' ', '_'), block.gender, region)
    mons = []
    for index, mon in enumerate(block.mons):
        fields = {}
        for line in lines[mon['line'] + 1:]:
            if not line.strip():
                break
            if ':' in line:
                key, value = line.split(':', 1)
                fields[key] = value.strip()
        ability = fields.get('Ability')
        ability = 'ABILITY_' + re.sub(r'[^A-Z0-9]+', '_', ability.upper()) if ability else None
        gender = mon['gender'][0].upper() if mon['gender'] else None
        mons.append(Mon(index, normalise_token(mon['species'], mon['gender']), int(fields['Level']),
                        'ITEM_' + re.sub(r'[^A-Z0-9]+', '_', mon['item'].upper()) if mon['item'] else None, gender, ability))
    plan_team(model, ctx, mons)
    for mon in mons:
        mon.drop_ability = ability_drop(model, mon)
    apply_edits(lines, block, mons, spellings or {})
    return '\n'.join(l for l in lines[1:] if l is not None)


# --------------------------------------------------------------------------
# Selected-roster header backend (Pokemon Tower has no party source)

def edit_roster_header(text, trainer_id, decisions):
    start = re.search(rf'\[{re.escape(trainer_id)}\]\s*=', text)
    end = re.search(r'\n\[TRAINER_\w+\]\s*=', text[start.end():])
    stop = start.end() + end.start() if end else len(text)
    block = text[start.start():stop]
    marks = list(re.finditer(r'\.species = (SPECIES_\w+),', block))
    if len(marks) != len(decisions):
        raise ValueError(f'{trainer_id}: roster header has {len(marks)} Pokemon, expected {len(decisions)}')
    pieces, cursor = [], 0
    for index, mark in enumerate(marks):
        region_end = marks[index + 1].start() if index + 1 < len(marks) else len(block)
        region = block[mark.start():region_end]
        decision = decisions[index]
        if decision.after != decision.species:
            region = region.replace(mark.group(0), f'.species = {decision.after},', 1)
        if decision.new_gender:
            kind = 'TRAINER_MON_MALE' if decision.new_gender == 'M' else 'TRAINER_MON_FEMALE'
            region = re.sub(r'\.gender = TRAINER_MON_\w+,', f'.gender = {kind},', region, count=1)
        if decision.drop_ability:
            region = re.sub(r'\n\s*\.ability = ABILITY_\w+,', '', region, count=1)
        pieces.append(block[cursor:mark.start()])
        pieces.append(region)
        cursor = region_end
    pieces.append(block[cursor:])
    return text[:start.start()] + ''.join(pieces) + text[stop:]


# --------------------------------------------------------------------------
# Orchestration

def normalise_token(token, gender):
    """Raw party species token to its constant, mirroring trainerproc."""
    token = token.strip()
    if token in GENDERED_SPECIES and gender:
        token = GENDERED_SPECIES[token][0 if gender[0] in 'M' else 1]
    return species_constant(token)


def collect_spellings(texts):
    spellings = {}
    for text in texts:
        for line in text.split('\n'):
            parsed = parse_header(line) if line and line[0].isalpha() and not re.match(r'^[A-Z][A-Za-z ]*:', line) else None
            if parsed:
                spellings.setdefault(species_constant(parsed['species']), parsed['species'])
    return spellings


def plan_all(model=None, covered=None, exceptions=None):
    """Plan every covered roster.  Returns (plans, edits) without touching disk.

    plans: {trainer id: {'ctx', 'mons', 'source'}}
    edits: {path: new text}
    """
    model = model or load_model()
    if covered is None:
        covered, exceptions = gather()
    frlg = frlg_sources()
    texts = {name: (DATA / name).read_text() for name in (*PARTY_FILES, FRLG_PARTY)}
    tower_text = (DATA / TOWER_ROSTER).read_text()
    spellings = collect_spellings(texts.values())
    parsed = {}
    for name, text in texts.items():
        spans, lines = split_blocks(text)
        parsed[name] = {'lines': lines, 'blocks': {t: PartyBlock(t, lines[s:e]) for t, s, e in spans}, 'spans': {t: (s, e) for t, s, e in spans}}
    plans, claimed = OrderedDict(), {}
    tower_edits = []
    for trainer_id, info in sorted(covered.items()):
        record, raw = info['record'], info['raw']
        owner = record.get('owner', trainer_id)
        if owner != trainer_id:
            continue
        source_path = raw['source'].removeprefix('src/data/')
        if source_path in PARTY_FILES:
            file_name, block_id = source_path, trainer_id
        elif trainer_id in frlg:
            file_name, block_id = FRLG_PARTY, frlg[trainer_id]
        elif source_path == TOWER_ROSTER:
            file_name, block_id = TOWER_ROSTER, trainer_id
        else:
            continue
        if file_name != TOWER_ROSTER:
            block = parsed[file_name]['blocks'].get(block_id)
            if block is None:
                raise ValueError(f'{trainer_id}: no party block {block_id} in {file_name}')
            if (file_name, block_id) in claimed:
                other = plans[claimed[file_name, block_id]]
                if [m['species'] for m in record['slots']] != [m.species for m in other['mons_before']]:
                    raise ValueError(f'{trainer_id}: shares {block_id} with a differing roster')
                continue
            claimed[file_name, block_id] = trainer_id
            if len(block.mons) != len(record['slots']):
                raise ValueError(f'{trainer_id}: parsed {len(block.mons)} Pokemon, compiled {len(record["slots"])}')
            for mon, entry in zip(block.mons, record['slots']):
                if normalise_token(mon['species'], mon['gender']) != model.canonical(entry['species']) \
                        and normalise_token(mon['species'], mon['gender']) != entry['species']:
                    raise ValueError(f'{trainer_id}: parsed {mon["species"]} but compiled {entry["species"]}')
            gender = block.gender
        else:
            block = None
            gender = {'TRAINER_GENDER_MALE': 'M', 'TRAINER_GENDER_FEMALE': 'F'}.get(record.get('gender'))
        ctx = Context(trainer_id, record['trainerClass'], gender, info['region'])
        mons = [mon_from_record(i, entry, exceptions, owner) for i, entry in enumerate(record['slots'])]
        before = [(m.species) for m in mons]
        plan_team(model, ctx, mons)
        for mon in mons:
            mon.drop_ability = ability_drop(model, mon)
        plans[trainer_id] = {'ctx': ctx, 'mons': mons, 'mons_before': [type('B', (), {'species': s}) for s in before],
                             'source': (file_name, block_id), 'info': info}
    edits = {}
    for (file_name, block_id), trainer_id in claimed.items():
        entry = parsed[file_name]
        block = entry['blocks'][block_id]
        start, _ = entry['spans'][block_id]
        local = list(block.lines)
        apply_edits(local, block, plans[trainer_id]['mons'], spellings)
        entry['lines'][start:start + len(local)] = local
    for file_name, entry in parsed.items():
        if any(claimed_file == file_name for claimed_file, _ in claimed):
            edits[file_name] = '\n'.join(l for l in entry['lines'] if l is not None)
    for trainer_id, plan in plans.items():
        if plan['source'][0] == TOWER_ROSTER:
            tower_text = edit_roster_header(tower_text, trainer_id, plan['mons'])
            edits[TOWER_ROSTER] = tower_text
    return plans, edits


# --------------------------------------------------------------------------
# Report

def short(species):
    return species.removeprefix('SPECIES_')


def spot_checks(plans, model, audit, evolution, table):
    """Step-back examples for stone / trade / friendship finals at low and high Rating."""
    wanted = ('SPECIES_VILEPLUME', 'SPECIES_GENGAR', 'SPECIES_STEELIX', 'SPECIES_RAICHU', 'SPECIES_MAGNEZONE', 'SPECIES_ARCANINE')
    lines, seen = [], set()
    for trainer_id, plan in plans.items():
        mons = plan['mons']
        hit = next((m for m in mons if m.after in wanted and m.after != m.species and m.after not in seen), None)
        if hit is None:
            continue
        seen.add(hit.after)
        policy = plan['info']['policy']
        lines.append(f"- `{trainer_id}` ({plan['ctx'].cls}, {plan['ctx'].region}): authored "
                     + ', '.join(f"{short(m.species)} {m.level}" for m in mons))
        for rating in (0, 16, 40, 80):
            team = []
            for m in mons:
                level = audit.project(rating, m.level, policy)
                team.append(f"{short(evolution.step_down(m.after, level, table))} {level}")
            lines.append(f"  - Rating {rating}: " + ', '.join(team))
    return lines


def build_report(plans, model):
    audit = load_module('trainer_scaling_audit', TOOL / 'audit.py')
    evolution = load_module('notable_evolution', ROOT / 'tools/notable_trainers/evolution.py')
    table = evolution.stage_table()
    total_slots = sum(len(p['mons']) for p in plans.values())
    changed_slots = [(t, m) for t, p in plans.items() for m in p['mons'] if m.after != m.species]
    changed_trainers = {t for t, _ in changed_slots}
    capped = [(t, m) for t, p in plans.items() for m in p['mons'] if m.after == m.species and 'R4' in m.tags]
    exceptions = [(t, m) for t, p in plans.items() for m in p['mons'] if m.exception and m.after == m.species]
    branches = Counter()
    for t, p in plans.items():
        for m in p['mons']:
            if m.after != m.species and 'R3' in m.tags:
                branches[(short(m.species), short(m.after))] += 1
    regional = Counter((short(m.species), short(m.after)) for t, m in changed_slots if 'R2' in m.tags)
    out = ['# Final-stage roster authoring report', '',
           'Generated by `game/tools/trainer_scaling/final_stage.py`. Every regular roster (ORDINARY and GYM_MEMBER) names the final stage of each line;'
           ' the scaler\'s step-back rule lowers each Pokemon to the stage its level supports.', '',
           '## Summary', '', '| Measure | Count |', '| --- | ---: |',
           f'| Covered Trainer rosters | {len(plans)} |', f'| Rosters changed | {len(changed_trainers)} |',
           f'| Slots | {total_slots} |', f'| Slots changed (R1 final stage) | {len(changed_slots)} |',
           f'| Slots reaching the middle stage (R4 three-or-more copies) | {sum("R4:mid" in m.tags for _, m in changed_slots)} |',
           f'| Slots left at the authored stage by the duplicate cap (R4) | {len(capped)} |',
           f'| Slots with a regional / region-locked choice (R2) | {sum("R2" in m.tags for _, m in changed_slots)} |',
           f'| Slots with a theme or id branch choice (R3) | {sum("R3" in m.tags for _, m in changed_slots)} |',
           f'| Slots with an explicitly set gender | {sum(bool(m.new_gender) for _, m in changed_slots)} |',
           f'| Authored abilities dropped as illegal for the new species (R6) | {sum(m.drop_ability for _, m in changed_slots)} |',
           f'| Slots kept by an identity exception (R5) | {len(exceptions)} |', '']
    kinds = Counter(m.exception for _, m in exceptions)
    out += ['| Identity exception | Slots |', '| --- | ---: |'] + [f'| {k} | {v} |' for k, v in sorted(kinds.items())] + ['']
    out += ['## Branches chosen (R3)', '', '| Authored stage | Chosen final | Slots |', '| --- | --- | ---: |']
    out += [f'| {a} | {b} | {n} |' for (a, b), n in sorted(branches.items())] + ['']
    out += ['## Regional filtering applied (R2)', '',
            'Region-locked forms and evolutions were excluded outside their region (Alolan forms outside Alola, Galarian forms outside Sevii, Hisui-only evolutions outside Sinjoh). Slots affected:', '',
            '| Authored | Final | Slots |', '| --- | --- | ---: |']
    out += [f'| {a} | {b} | {n} |' for (a, b), n in sorted(regional.items())] + ['']
    out += ['## Duplicate caps and middle stages (R4)', '',
            'Per team and per line only one copy reaches the final stage (highest authored level, ties to the later slot); with three or more copies of a line the next copy reaches the middle stage. Slots held back (authored species kept):', '']
    by_trainer = OrderedDict()
    for t, m in capped:
        by_trainer.setdefault(t, []).append(f'slot {m.slot} {short(m.species)}')
    out += [f'- `{t}`: ' + ', '.join(items) for t, items in by_trainer.items()] + ['']
    out += ['## Identity exceptions (R5)', '']
    for kind in sorted(kinds):
        grouped = OrderedDict()
        for t, m in exceptions:
            if m.exception == kind:
                grouped.setdefault(t, []).append(f'slot {m.slot} {short(m.species)}')
        out += [f'### {kind}', ''] + [f'- `{t}`: ' + ', '.join(items) for t, items in grouped.items()] + ['']
    child = Counter(plans[t]['ctx'].cls for t, m in exceptions if m.exception == 'child class')
    out += ['## Doubtful cases for review', '',
            '- Child-class exemption (R5b) covers ' + ', '.join(f'{k} ({v} slots)' for k, v in sorted(child.items()))
            + '. The brief named Tuber and Preschooler; SCHOOL_KID, TWINS and SIS_AND_BRO were added as kid identities. Edit `CHILD_CLASSES` in `final_stage.py` to narrow or widen it.',
            '- Middle stage (R4): the second copy of a three-stage line reaches the middle stage only when the team holds three or more copies of that line. This satisfies both brief examples (5 Geodude -> Golem + Graveler + 3 Geodude, 2 Zubat -> Crobat + Zubat).',
            '- A copy already authored at the final stage consumes the final quota, so [Magikarp 30, Gyarados 20] stays as authored instead of becoming two Gyarados.',
            '- Babies keep their species: the shared stage table has no baby -> parent edge, so promoting one (Munchlax -> Snorlax) would field the evolved species at every level.',
            '- Eviolite holders (R5, judgment call) keep their authored stage because the item only works on not-fully-evolved Pokemon.',
            '- Trainer gender for Gallade / Froslass / Wormadam / Mothim / Meowstic comes from the class (female or male classes) and falls back to the party `Gender:` field, which is wrong on several HNS rosters (female classes marked Male).',
            '- Class themes for R3 (fighting, pretty, psychic, cool, Eevee themes) are the lists at the top of `final_stage.py`; borderline classes (Picnicker as pretty, Triathlete / Ranger / Dragon Tamer as cool, Hiker / Pokefan / Breeder as grass for Eevee) are judgment calls.',
            '- Annihilape, Farigiraf, Kingambit, Dudunsparce and Magnezone-style cross-generation finals appear because the compiled evolution graph keeps them.',
            '- Held items are untouched: Light Ball stays on Raichu, Black Belt on Machamp, Nugget / berries on evolved slots.',
            '- Hisui rules (Kleavor, Ursaluna, Wyrdeer, Overqwil, Sneasler, Basculegion) are implemented and unit tested, but no covered Trainer is in Sinjoh, so none is exercised by the shipped rosters; Ursaring and Stantler stop at their standard stage everywhere.',
            '- Pokemon Tower (`trainers_wayfarer_tower.h`) has no party source, so its 16 rosters are edited in the selected header directly.',
            '- The Sevii, coast, S.S. Anne and Celadon Hideout rosters are edited in `trainers_frlg.party` (their donor), which also changes those FRLG-only donor trainers for standalone FRLG builds.',
            '']
    out += ['## Step-back spot checks', '',
            'Final-stage rosters projected through the scaler (policy level at the Rating, then the shared step-back table). Stone, trade and friendship finals appear at lower stages when the level is below their threshold.', '']
    out += spot_checks(plans, model, audit, evolution, table) + ['']
    out += ['## Per-Trainer changes', '', 'Format: slot, authored level, authored species, final species (rules). `R1` final stage, `R2` regional filter, `R3` branch, `R4:mid` middle stage, `gender` set explicitly, `ability dropped`.', '']
    for trainer_id, plan in plans.items():
        items = []
        for m in plan['mons']:
            if m.after == m.species:
                continue
            tags = [t for t in dict.fromkeys(m.tags)]
            if m.new_gender:
                tags.append(f'gender {m.new_gender}')
            if m.drop_ability:
                tags.append('ability dropped')
            items.append(f'{m.slot}: L{m.level} {short(m.species)} -> {short(m.after)} ({", ".join(tags)})')
        if items:
            out.append(f"- `{trainer_id}` ({plan['ctx'].cls}, {plan['ctx'].region}): " + '; '.join(items))
    return '\n'.join(out) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--apply', action='store_true')
    mode.add_argument('--check', action='store_true')
    mode.add_argument('--report', action='store_true')
    args = parser.parse_args()
    model = load_model()
    plans, edits = plan_all(model)
    if args.check:
        stale = [name for name, text in edits.items() if (DATA / name).read_text() != text]
        if stale:
            raise SystemExit(f'rosters not at final stages: {stale}')
        return
    if args.apply:
        for name, text in edits.items():
            (DATA / name).write_text(text)
    changed = any(m.after != m.species for p in plans.values() for m in p['mons'])
    if args.report and not changed:
        raise SystemExit('nothing to report: the sources are already final-staged, so the committed '
                         'report is kept (restore the pre-authoring sources and run --apply to rebuild it)')
    if changed:
        REPORT.write_text(build_report(plans, model))


if __name__ == '__main__':
    main()
