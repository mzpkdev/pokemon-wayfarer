#!/usr/bin/env python3
"""Map-to-reach resolution shared by the trainer reach table and the item spot generator.

Specs: reach-assignments.md ("Places without wild encounters"), trainer party scaling ("v0 levels")
and world items ("Reach of an item spot"). Loads the compiled Wayfarer maps and script text, the wild
reach data and the reviewed rules in reach_places.json, and resolves a map to a place record.
"""
from __future__ import annotations

from collections import defaultdict, deque
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
TOOL = Path(__file__).resolve().parent
RULES = TOOL / 'reach_places.json'
# The Makefile's CPPFLAGS for the Wayfarer product (TESTING=0).
CPP_FLAGS = ['-P', '-x', 'assembler-with-cpp', '-Wno-trigraphs', '-DMODERN=1', '-DTESTING=0', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS',
             '-iquote', 'include']
REACH_LEVELS = ('Road', 'Wilds', 'Outlands')  # the reaches reach_maps may name
GYM_RE = re.compile(r'Gym', re.I)
FACILITY_RE = re.compile(r'^(BattleFrontier|BattleTower|BattleDome|BattlePalace|BattleArena|BattleFactory|BattlePike|BattlePyramid|TrainerHill|BattleColosseum|BattleTent)', re.I)
OUTDOOR_TYPES = {'MAP_TYPE_TOWN', 'MAP_TYPE_CITY', 'MAP_TYPE_ROUTE', 'MAP_TYPE_OCEAN_ROUTE'}
FERRY_RE = re.compile(r'^SS(Aqua|Tidal)', re.I)


class ReachError(ValueError):
    pass


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def read(path):
    return Path(path).read_text(encoding='utf-8', errors='replace')


# ---------- the compiled Wayfarer maps ----------
class MapInfo:
    __slots__ = ('name', 'const', 'value', 'group', 'num', 'map_type', 'events', 'scripts_owner', 'warps', 'events_owner')


def load_maps(root=ROOT):
    """Every compiled Wayfarer map (mapjson's wayfarer groups) with its events."""
    world = load_module('reach_world_maps', root / 'tools/wayfarer_world/maps.py')
    for needed in ('data/maps/groups.inc', 'include/constants/map_groups.h'):
        if not (root / needed).is_file():
            raise ReachError(f'missing {needed}: run `make BUILD=wayfarer generated` first')
    names = [name for _, group in world.parse_groups(root) for name in group]
    constants = world.parse_map_constants(root)
    if len(names) != len(constants):
        raise ReachError('groups.inc and map_groups.h list different map counts')
    maps = {}
    for name, (const, value) in zip(names, constants):
        if name == 'NULL':
            continue  # a deselected map keeps its constant but has no header
        info = MapInfo()
        info.name, info.const, info.value = name, const, value
        info.group, info.num = value >> 8, value & 0xFF
        directory = root / 'data/maps' / name
        header = world.parse_header(read(directory / 'header.inc'))
        info.map_type = header['map_type']
        owner = header['events_label'].rsplit('_MapEvents', 1)[0]
        info.events = world.parse_events(read(root / 'data/maps' / owner / 'events.inc'))
        info.events_owner = owner
        info.scripts_owner = header['scripts_label'].rsplit('_MapScripts', 1)[0]
        info.warps = sorted({warp['dest_map'] for warp in info.events['warps'] if warp['dest_map'].startswith('MAP_')})
        maps[const] = info
    return maps

# ---------- the active script text ----------
def expand_includes(root, relative, stack=()):
    """event_scripts.s with every .include expanded inline, as tools/preproc does before cpp."""
    if relative in stack:
        raise ReachError(f'include cycle at {relative}')
    path = root / relative
    if not path.is_file():
        raise ReachError(f'{relative}: included script file is missing')
    out = []
    for line in read(path).splitlines():
        match = re.match(r'\s*\.include\s+"([^"]+)"', line)
        if match:
            out.append(expand_includes(root, match.group(1), stack + (relative,)))
        else:
            out.append(line)
    return '\n'.join(out)


SAFE_CONDITION = re.compile(r'^[0-9xXa-fA-F\s=!<>()+\-*&|]+$')


def condition_value(condition):
    """True/False for a numeric `.if` expression, None for one that names assembler symbols."""
    condition = re.sub(r'\bTRUE\b', '1', re.sub(r'\bFALSE\b', '0', condition.strip()))
    if not SAFE_CONDITION.match(condition):
        return None
    return bool(eval(condition.replace('&&', ' and ').replace('||', ' or '), {'__builtins__': {}}, {}))


def apply_conditionals(text):
    """Drop the branches of numeric `.if`/`.elseif`/`.else`/`.endif` blocks that assemble to nothing.

    A condition naming assembler symbols stays unknown: every branch is kept.
    """
    # frame: [enclosing activity, a branch is surely taken, an earlier branch may have been taken]
    out, stack, active = [], [], True
    for line in text.splitlines():
        directive = line.strip()
        head = re.match(r'\.(if|ifdef|ifndef|elseif|else|endif)\b\s*(.*)$', directive)
        if head is None:
            if active:
                out.append(line)
            continue
        kind, argument = head.groups()
        if kind in ('if', 'ifdef', 'ifndef'):
            state = condition_value(argument) if kind == 'if' else None
            stack.append([active, state is True, state is None])
            active = active and state is not False
        elif kind in ('elseif', 'else') and stack:
            frame = stack[-1]
            if frame[1]:
                state = False
            elif kind == 'else':
                state = None if frame[2] else True
            else:
                # A true condition ends the block, but an unknown earlier branch may still be the one taken.
                value = condition_value(argument)
                frame[1] = value is True
                state = None if frame[2] and value is True else value
            frame[1] = frame[1] or state is True
            frame[2] = frame[2] or state is None
            active = frame[0] and state is not False
        elif kind == 'endif' and stack:
            active = stack.pop()[0]
        elif active:
            out.append(line)
    return '\n'.join(out)


def script_text(root=ROOT):
    """The Wayfarer ROM's script assembly: includes expanded, cpp'd with the Makefile flags, `.if`s applied."""
    source = expand_includes(root, 'data/event_scripts.s')
    done = subprocess.run(['cpp', *CPP_FLAGS, '-'], input=source, cwd=root, text=True, capture_output=True)
    if done.returncode:
        raise ReachError(f'cpp failed on the script assembly: {done.stderr[:600]}')
    return apply_conditionals(re.sub(r'^\s*\.macro\b.*?^\s*\.endm\b', '', done.stdout, flags=re.S | re.M))


TRAINERBATTLE = re.compile(r'^[ \t]*(trainerbattle\w*)[ \t]+([^@\n]*)', re.M)
IDENT = re.compile(r'\b[A-Za-z_]\w*\b')
NUMBER = re.compile(r'^[0-9xXa-fA-F\s()+\-*]+$')
# Arguments that name opposing Trainers, by macro: the first, except the two-Trainer and raw forms.
TRAINER_ARGS = {'trainerbattle_two_trainers': (0, 2), 'trainerbattle': (2, 7)}


FALLTHROUGH = {}  # label -> the label whose script it runs on into
SCRIPT_END = {'end', 'goto', 'return', 'step_end'}


def ends_script(body):
    """True when a script body cannot run on into the label that follows it."""
    lines = [re.sub(r'@.*', '', line).strip() for line in body.splitlines()]
    lines = [line for line in lines if line]
    if not lines:
        return False
    first = lines[-1].split()[0]
    return first in SCRIPT_END or first.startswith('.')


def label_index(root=ROOT):
    """{label: body} over the Wayfarer ROM's script assembly."""
    text = script_text(root)
    labels = {}
    FALLTHROUGH.clear()
    marks = list(re.finditer(r'^(\w+)::?[ \t]*(?:@.*)?$', text, re.M))
    for i, mark in enumerate(marks):
        end = marks[i + 1].start() if i + 1 < len(marks) else len(text)
        body = text[mark.end():end]
        if mark.group(1) in labels:
            # Table macros repeat labels under assembler symbols; two bodies that battle would be a real clash.
            if body_trainers(body) and body_trainers(labels[mark.group(1)]):
                raise ReachError(f'script label {mark.group(1)} is defined twice in the Wayfarer assembly')
            if not body_trainers(body):
                continue
        labels[mark.group(1)] = body
        if i + 1 < len(marks) and not ends_script(body):
            FALLTHROUGH[mark.group(1)] = marks[i + 1].group(1)
    return labels


def macro_args(text):
    """Arguments of an assembler macro call: separated by commas or spaces outside parentheses."""
    args, current, depth = [], '', 0
    for char in text.strip():
        if char == '(':
            depth += 1
        elif char == ')':
            depth -= 1
        if depth == 0 and (char == ',' or char.isspace()):
            if current:
                args.append(current)
            current = ''
        else:
            current += char
    if current:
        args.append(current)
    return args


def body_trainers(body):
    """Numeric Trainer IDs a script body battles (cpp has already expanded the TRAINER_* names)."""
    found = set()
    for match in TRAINERBATTLE.finditer(body):
        macro, args = match.group(1), macro_args(match.group(2))
        for index in TRAINER_ARGS.get(macro, (0,)):
            if index >= len(args):
                raise ReachError(f'{macro} {match.group(2)}: missing Trainer argument')
            if not NUMBER.match(args[index]):
                raise ReachError(f'{macro} {match.group(2)}: Trainer argument is not a number')
            found.add(eval(args[index], {'__builtins__': {}}, {}))
    return found - {0}


# ---------- reach data ----------
def wild_places(root=ROOT):
    """{map const: place dict} for every map the wild reach assignments list."""
    wild = load_module('reach_v2_emit', root / 'tools/wild_encounters/v2/v2_emit.py')
    keys, _, meta = wild.load()
    intents = wild.parse_intents()
    places = {}
    for key in keys:
        const = wild.map_constant(key)
        place = wild.place_of(key, meta, intents)
        label = meta[key]['place']
        if const in places and (places[const]['place'] != place):
            raise ReachError(f'{const}: wild headers disagree on its place')
        places[const] = {'place': place, 'name': label, 'reach': meta[key]['reach'], 'intent_name': intents.get(const, (None,))[0]}
    return places, wild


class Rules:
    def __init__(self, path=RULES):
        data = json.loads(read(path))
        if data.get('version') != 1:
            raise ReachError('unsupported reach_places.json version')
        self.data = data
        self.reach_maps = data['reach_maps']
        self.dungeons = data['dungeons']
        self.new_dungeons = data['new_dungeons']
        self.interior_overrides = data.get('interior_overrides', {})

    def constants(self):
        names = set(self.reach_maps) | set(self.interior_overrides) | set(self.interior_overrides.values())
        for dungeon in self.dungeons + self.new_dungeons:
            for step in dungeon['steps']:
                names |= set(step)
        return names


def place_record(wild, reach, intent='Mild', flat=False, floor=0, floors=0, region='Other'):
    return {'reach': wild.REACHES[reach], 'intent': wild.INTENTS[intent] if reach == 'Dungeon' else 'WILD_DUNGEON_MILD',
            'flat': int(flat), 'region': wild.REGIONS.get(region, 'WILD_PLACE_REGION_OTHER'), 'floor': floor, 'floorCount': floors}


class Resolver:
    """Maps a hosting map to (place record, rule id, detail)."""

    def __init__(self, maps, listed, wild, rules):
        self.maps, self.listed, self.wild, self.rules = maps, listed, wild, rules
        known = set(maps)
        missing = sorted(name for name in rules.constants() if name not in known)
        if missing:
            raise ReachError(f'reach_places.json names maps the Wayfarer build lacks: {missing}')
        self.joined, self.new_floor, self.reach_rule = {}, {}, {}
        for const, reach in rules.reach_maps.items():
            if reach not in REACH_LEVELS:
                raise ReachError(f'{const}: unknown reach {reach}')
            self.reach_rule[const] = reach
        self._index_dungeons()
        self._cache = {}

    def _index_dungeons(self):
        wild = self.wild
        for dungeon in self.rules.dungeons:
            steps = dungeon['steps']
            members = [const for step in steps for const in step]
            if len(set(members)) != len(members):
                raise ReachError(f"{dungeon['name']}: a map appears twice in its floor order")
            listed = [const for const in members if const in self.listed]
            if not listed:
                raise ReachError(f"{dungeon['name']}: joins no listed dungeon")
            base = self.listed[listed[0]]['place']
            if base['reach'] != 'WILD_REACH_DUNGEON':
                raise ReachError(f"{dungeon['name']}: {listed[0]} is not a dungeon in reach-assignments.md")
            wild_order = sorted(listed, key=lambda c: self.listed[c]['place']['floor'])
            positions = [next(i for i, step in enumerate(steps) if const in step) for const in wild_order]
            if positions != sorted(positions):
                raise ReachError(f"{dungeon['name']}: floor order disagrees with the wild floor order")
            intent_name = dungeon.get('intent') or self.listed[listed[0]]['intent_name']
            for index, step in enumerate(steps):
                for const in step:
                    self.joined[const] = {
                        'dungeon': dungeon['name'], 'step': index, 'steps': len(steps),
                        'record': place_record(wild, 'Dungeon', intent_name, False, index, len(steps)),
                        'listed': const in self.listed, 'rule': 'join'}
        for dungeon in self.rules.new_dungeons:
            steps = dungeon['steps']
            members = [const for step in steps for const in step]
            if len(set(members)) != len(members):
                raise ReachError(f"{dungeon['name']}: a map appears twice in its floor order")
            flat = len(steps) == 1
            for index, step in enumerate(steps):
                for const in step:
                    if const in self.joined or const in self.new_floor:
                        raise ReachError(f'{const}: placed by two dungeons')
                    if const in self.listed:
                        raise ReachError(f"{const}: new dungeon {dungeon['name']} names a map the wild data already lists")
                    self.new_floor[const] = {
                        'dungeon': dungeon['name'], 'step': index, 'steps': len(steps),
                        'record': place_record(wild, 'Dungeon', dungeon['intent'], flat, 0 if flat else index, 1 if flat else len(steps))}

    def kind(self, const):
        name = self.maps[const].name
        if FERRY_RE.match(name):
            return 'ferry'
        if GYM_RE.search(name) or FACILITY_RE.match(name):
            return 'gym-or-facility'
        return 'other'

    def direct(self, const):
        """Rules that need no neighbour: listed, joins, new dungeons, ferries."""
        if const in self.joined:
            join = self.joined[const]
            label = 'join (listed map re-stepped)' if join['listed'] else 'join'
            return join['record'], label, f"{join['dungeon']} step {join['step'] + 1} of {join['steps']}"
        if const in self.listed:
            entry = self.listed[const]
            return entry['place'], 'listed', f"{entry['name']} ({entry['reach']})"
        if const in self.reach_rule:
            return place_record(self.wild, self.reach_rule[const]), 'join', f'{self.reach_rule[const]} by the joining table'
        if const in self.new_floor:
            new = self.new_floor[const]
            return new['record'], 'new-dungeon', f"{new['dungeon']} step {new['step'] + 1} of {new['steps']}"
        if self.kind(const) == 'ferry':
            return place_record(self.wild, 'Road'), 'ferry', 'ferries are Road'
        return None

    def resolve(self, const):
        if const in self._cache:
            return self._cache[const]
        result = self.direct(const)
        if result is None and self.kind(const) == 'gym-or-facility':
            result = None
        elif result is None:
            result = self.interior(const)
        self._cache[const] = result
        return result

    def interior(self, const):
        """Interiors take the map they open onto: the nearest placed map by warps."""
        override = self.rules.interior_overrides.get(const)
        if override:
            found = self.resolve_direct_or_interior(override)
            if found is None:
                raise ReachError(f'{const}: interior override {override} has no reach')
            return found[0], 'interior', f'{self.maps[override].name} (reviewed override)'
        seen, queue = {const}, deque([(const, 0)])
        best, best_distance = {}, None
        while queue:
            current, distance = queue.popleft()
            if best_distance is not None and distance >= best_distance:
                continue
            for target in self.maps[current].warps:
                if target in seen or target not in self.maps:
                    continue
                seen.add(target)
                if self.kind(target) == 'gym-or-facility':
                    continue
                found = self.direct(target)
                if found is not None:
                    best_distance = distance + 1
                    best[target] = found
                else:
                    queue.append((target, distance + 1))
        if not best:
            return None
        places = {json.dumps(found[0], sort_keys=True) for found in best.values()}
        if len(places) > 1:
            # An interior opens onto the outside: when it also leads to a dungeon floor, the outdoor map wins.
            outdoors = {t: found for t, found in best.items() if self.maps[t].map_type in OUTDOOR_TYPES}
            if outdoors:
                best = outdoors
                places = {json.dumps(found[0], sort_keys=True) for found in best.values()}
        if len(places) > 1:
            names = sorted(f'{self.maps[t].name} ({found[2]})' for t, found in best.items())
            raise ReachError(f'{const}: interior opens onto maps with different reaches {names}; '
                             'add an interior_overrides entry to reach_places.json')
        target = sorted(best)[0]
        found = best[target]
        via = ', '.join(sorted(self.maps[t].name for t in best))
        return found[0], 'interior', f'opens onto {via}'

    def resolve_direct_or_interior(self, const):
        return self.direct(const) or self.interior(const)
