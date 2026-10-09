#!/usr/bin/env python3
"""Resolve the reach of every map that hosts a covered (ORDINARY) Trainer.

Specs: the trainer party scaling spec ("v0 levels") and the reach assignments
spec ("Places without wild encounters"); reach-assignments.md is a CI build input.

Writes src/data/trainer_scaling/trainer_places.h (the compact ROM table the
battle code reads) and src/data/trainer_scaling/reach_report.md (which rule
resolved each map). A covered Trainer on a map with no resolvable entry, a map
constant the data names but the Wayfarer build lacks, or any ambiguity the data
does not settle fails generation: nothing falls back silently at runtime.

    python3 tools/trainer_scaling/reach.py [--check]

Inputs are the Wayfarer build's mapjson outputs (data/maps/groups.inc,
include/constants/map_groups.h, data/maps/*/{header,events}.inc; run
`make BUILD=wayfarer generated`), the active Wayfarer script files, the
ORDINARY entries of policies.h, the wild generator's reach data
(src/data/wild_encounters_v2/meta.json and the reach-assignments intents) and
the reviewed rule data in reach_places.json.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re
import subprocess
import sys


TOOL = Path(__file__).resolve().parent
if str(TOOL) not in sys.path:
    sys.path.insert(0, str(TOOL))
from reach_resolver import *  # noqa: E402,F401,F403  (the shared resolution; tests reach it through this module)
from reach_resolver import ROOT, RULES, FALLTHROUGH, IDENT, NUMBER  # noqa: E402,F401

OUTPUT = ROOT / 'src/data/trainer_scaling'
TABLE = OUTPUT / 'trainer_places.h'
REPORT = OUTPUT / 'reach_report.md'
POLICIES_HEADER = OUTPUT / 'policies.h'
ORDINARY = 1
SEVII_REMATCHES = ROOT / 'src/data/wayfarer_sevii_rematches.h'
COAST_REMATCHES = ROOT / 'src/wayfarer_coast_trainers.c'


def trainer_numbers(names, root=ROOT):
    """{TRAINER_* name: runtime ID} through the real opponent headers."""
    source = '#include "constants/opponents.h"\n' + '\n'.join(f'ID("{name}", {name})' for name in sorted(names))
    done = subprocess.run(['cpp', '-P', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS', '-DIS_WAYFARER=1', '-DIS_HNS=1', '-I', str(root / 'include'), '-'],
                          input=source, text=True, capture_output=True)
    if done.returncode:
        raise ReachError(f'cpp failed on the opponent constants: {done.stderr[:400]}')
    result = {}
    for name, expression in re.findall(r'ID\("(TRAINER_\w+)",\s*(.*?)\)\s*$', done.stdout, re.M):
        if not NUMBER.match(expression):
            raise ReachError(f'{name} does not resolve to a number: {expression}')
        result[name] = eval(expression, {'__builtins__': {}}, {})
    if set(result) != set(names):
        raise ReachError(f'unresolved Trainer constants: {sorted(set(names) - set(result))[:5]}')
    return result


def hosting(maps, labels):
    """{trainer: {map const}} from the scripts each compiled map can run: its events, its map script table, and what they reach."""
    host = defaultdict(set)
    reach_cache = {}

    def closure(label):
        """Labels reachable from `label` (itself included)."""
        if label in reach_cache:
            return reach_cache[label]
        seen, stack = set(), [label]
        while stack:
            current = stack.pop()
            if current in seen or current not in labels:
                continue
            seen.add(current)
            stack.extend(token for token in IDENT.findall(re.sub(r'@.*', '', labels[current])) if token in labels)
            if current in FALLTHROUGH:
                stack.append(FALLTHROUGH[current])
        reach_cache[label] = seen
        return seen

    for const, info in maps.items():
        seeds = set()
        for kind in ('objects', 'coords', 'bgs'):
            for event in info.events[kind]:
                script = event.get('script', '')
                if script and script != 'NULL':
                    seeds.add(script)
        # The map's own script table (frame, load and transition scripts) leads to the rest; labels nothing
        # reaches stay out, such as the FRLG paths a Wayfarer `#if` leaves behind.
        table = f'{info.scripts_owner}_MapScripts'
        if table in labels:
            seeds.add(table)
        reached = set()
        for seed in seeds:
            reached |= closure(seed)
        for label in reached:
            for trainer in body_trainers(labels[label]):
                host[trainer].add(const)
    return host


def rematch_families(root=ROOT):
    """[(trainer ids, map const or None)]: the Wayfarer rematch table rows (with their map), the Sevii rows and
    the Hoenn rows, which the ROM does not compile yet (no map: their stages stand where the first stage does)."""
    families = []
    source = read(root / 'src/battle_setup.c')
    start = source.index('#if IS_HNS\nconst struct RematchTrainer')
    middle = source.index('\n#else', start)
    end = source.index('\n#endif', middle)
    hns = re.sub(r'#if !IS_WAYFARER.*?#endif', '', source[start:middle], flags=re.S)  # the Wayfarer table
    for match in re.finditer(r'=\s*REMATCH\(([^)]*)\)', hns):
        parts = [part.strip() for part in match.group(1).split(',')]
        families.append((parts[:5], parts[5]))
    for match in re.finditer(r'=\s*REMATCH\(([^)]*)\)', source[middle:end]):
        families.append(([part.strip() for part in match.group(1).split(',')][:5], None))
    for match in re.finditer(r'SEVII_REMATCH\(([^)]*)\)', read(SEVII_REMATCHES)):
        families.append(([part.strip() for part in match.group(1).split(',')], None))
    coast = read(COAST_REMATCHES)
    coast = coast[coast.index('sWayfarerCoastRematchFamilies'):]
    for match in re.finditer(r'\{\{([^}]*)\}\}', coast[:coast.index('};')]):
        families.append(([part.strip() for part in match.group(1).split(',') if part.strip() != 'TRAINER_NONE'], None))
    return families


def ordinary_trainers():
    found = {m.group(1) for m in re.finditer(r'\[(TRAINER_\w+)\]\s*=\s*(\d+)', read(POLICIES_HEADER)) if int(m.group(2)) == ORDINARY}
    if not found:
        raise ReachError('policies.h lists no ORDINARY trainers: run generate.py first')
    return found


def collect(root=ROOT):
    maps = load_maps(root)
    labels = label_index(root)
    ordinary = ordinary_trainers()
    families = rematch_families(root)
    numbers = trainer_numbers(ordinary | {name for ids, _ in families for name in ids}, root)
    by_number = {number: name for name, number in numbers.items()}
    host = defaultdict(set)
    for number, consts in hosting(maps, labels).items():
        if number in by_number:
            host[by_number[number]] |= consts
    for ids, table_map in families:
        ids = set(ids)
        known = set().union(*(host.get(trainer, set()) for trainer in ids))
        if not known and table_map:
            if table_map not in maps:
                raise ReachError(f'rematch table names {table_map}, which the Wayfarer build lacks')
            known = {table_map}
        for trainer in ids:
            host[trainer] |= known
    return maps, ordinary, host


def resolve_hosted(maps, ordinary, host, listed, wild, rules):
    """Resolve every map that hosts an ORDINARY Trainer; raise listing each one that cannot be."""
    resolver = Resolver(maps, listed, wild, rules)
    hosted = defaultdict(set)  # map const -> ORDINARY trainers
    unplaced = []
    for trainer in sorted(ordinary):
        if host.get(trainer):
            for const in host[trainer]:
                hosted[const].add(trainer)
        else:
            unplaced.append(trainer)
    resolved, errors = {}, []
    for const in sorted(hosted, key=lambda c: maps[c].value):
        try:
            found = resolver.resolve(const)
        except ReachError as error:
            errors.append(str(error))
            continue
        if found is not None:
            resolved[const] = found
        elif resolver.kind(const) == 'gym-or-facility':
            # Gyms and battle facilities have no reach and Gym members use their own curve: an ORDINARY
            # Trainer there is a classification question for the specs, not something to guess.
            errors.append(f'{const} ({maps[const].name}) is a Gym or facility with no reach, '
                          f'yet hosts {len(hosted[const])} ORDINARY Trainer(s): {sorted(hosted[const])[:4]}')
        else:
            errors.append(f'{const} ({maps[const].name}) hosts {len(hosted[const])} covered Trainer(s) '
                          f'but no rule resolves its reach, e.g. {sorted(hosted[const])[:3]}')
    if errors:
        raise ReachError('\n'.join(errors))
    return {'maps': maps, 'resolved': resolved, 'hosted': hosted, 'unplaced': unplaced, 'ordinary': ordinary}


def resolve_all(root=ROOT):
    maps, ordinary, host = collect(root)
    listed, wild = wild_places(root)
    return resolve_hosted(maps, ordinary, host, listed, wild, Rules())


# ---------- output ----------
def render_table(state):
    maps, resolved = state['maps'], state['resolved']
    rows = []
    for const in sorted(resolved, key=lambda c: maps[c].value):
        place, rule, detail = resolved[const]
        rows.append(f'    {{ // {const}: {rule}, {detail}\n'
                    f'        MAP_GROUP({const}), MAP_NUM({const}),\n'
                    f'        {{ .reach = {place["reach"]}, .intent = {place["intent"]}, .flat = {place["flat"]}, '
                    f'.region = {place["region"]},\n'
                    f'          .floor = {place["floor"]}, .floorCount = {place["floorCount"]} }},\n    }},\n')
    return ('// Generated by tools/trainer_scaling/reach.py; edit reach_places.json or the wild reach data, then regenerate.\n'
            '// Every map that hosts an ORDINARY Trainer: the place its battles take their level from.\n'
            'static const struct TrainerMapPlace sTrainerMapPlaces[] =\n{\n' + ''.join(rows) + '};\n')


def place_text(place):
    if place['reach'] != 'WILD_REACH_DUNGEON':
        return place['reach'].removeprefix('WILD_REACH_').capitalize()
    text = f"{place['intent'].removeprefix('WILD_DUNGEON_').replace('_', ' ').lower()} dungeon"
    if place['flat'] or place['floorCount'] <= 1:
        return text + ', single floor'
    return text + f", floor {place['floor'] + 1} of {place['floorCount']}"


def render_report(state):
    maps, resolved, hosted = state['maps'], state['resolved'], state['hosted']
    by_rule = Counter(rule for _, rule, _ in resolved.values())
    lines = ['# Trainer reach report', '',
             'Generated by `tools/trainer_scaling/reach.py`; the table is `trainer_places.h`. Each row says which rule',
             'resolved a map that hosts an `ORDINARY` Trainer (see the trainer party scaling spec, "v0 levels"),',
             'in the order listed, then the reach or dungeon floor its battles use.', '',
             '| Rule | Maps |', '| --- | ---: |']
    names = {'listed': 'listed in reach assignments', 'join': 'joins an existing place',
             'join (listed map re-stepped)': 'listed map of a joined dungeon (re-stepped)',
             'new-dungeon': 'story-site dungeon', 'ferry': 'ferry (Road)', 'interior': 'interior (map it opens onto)'}
    for rule in ('listed', 'join (listed map re-stepped)', 'join', 'new-dungeon', 'ferry', 'interior'):
        lines.append(f'| {names[rule]} | {by_rule.get(rule, 0)} |')
    lines += [f'| total | {len(resolved)} |', '',
              f'{len(state["ordinary"])} ORDINARY Trainer IDs: {sum(len(v) for v in hosted.values())} placements on {len(resolved)} maps, '
              f'{len(state["ordinary"]) - len(state["unplaced"])} IDs placed, {len(state["unplaced"])} unplaced.', '']
    lines += ['## Maps', '', '| Map | Rule | Resolved by | Place | ORDINARY Trainers |', '| --- | --- | --- | --- | ---: |']
    for const in sorted(resolved, key=lambda c: maps[c].value):
        place, rule, detail = resolved[const]
        lines.append(f'| `{const}` | {rule} | {detail} | {place_text(place)} | {len(hosted[const])} |')
    lines += ['', '## Gym and facility maps', '',
              'Gyms and battle facilities have no reach (Gym members use their own curve). None hosts an `ORDINARY` Trainer; generation',
              'fails if one ever does, so the table never holds a Gym.']
    unplaced = state['unplaced']
    lines += ['', '## ORDINARY Trainer IDs with no hosting map', '',
              'No compiled Wayfarer map runs a script that battles these IDs, and no rematch family places them, so they never',
              'start a battle in the ROM. They need no map entry. If a script ever places one, regenerate: an unresolvable map fails.', '']
    groups = defaultdict(list)
    for trainer in unplaced:
        groups[re.sub(r'(_\d+)?(_HNS)?$', '', trainer)].append(trainer)
    lines.append(f'{len(unplaced)} IDs in {len(groups)} families:')
    lines.append('')
    lines.append(', '.join(f'`{trainer}`' for trainer in unplaced))
    return '\n'.join(lines) + '\n'


def write_output(path, content, check):
    if check:
        if not path.exists() or path.read_text() != content:
            raise ReachError(f'stale generated output: {path.relative_to(ROOT)}')
    else:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)


def generate(check=False):
    state = resolve_all()
    write_output(TABLE, render_table(state), check)
    write_output(REPORT, render_report(state), check)
    print(json.dumps({'maps': len(state['resolved']), 'unplaced_ordinary': len(state['unplaced']),
                      'rules': dict(Counter(rule for _, rule, _ in state['resolved'].values()))}, sort_keys=True))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    try:
        generate(args.check)
    except (ReachError, OSError, ValueError) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
