#!/usr/bin/env python3
"""Generate the daily world slots' trainer slot tables, trainer groups and reports.

Specs: .product/specs/daily-world-slots.md (Slot data, Which trainers are daily, Dialogue audit, Generation and reports)
and trainer-party-scaling.md (the ORDINARY classification). Applies to the Wayfarer build only.

Writes, under src/data/trainer_slots/:
  slots.h      the trainer slot rows (sorted by map then local id: the row number is the slot's dense index, which
               the save's homeBeatenToday bitset and, after the item spots, its clearedToday bitset use), the group
               tables, the fixed contacts' homes and the Sevii contact order
  report.md    slot status by region with reasons, group sizes, the dialogue audit and its resolutions
  dialogue.md  the inventory of every daily candidate's lines
  include/constants/world_trainer_slots.h  the counts the save struct and the per-map table are sized from

    python3 tools/wayfarer_trainer_slots/generate.py [--check]

Reads the Wayfarer build's mapjson outputs (run `make BUILD=wayfarer generated`), the classification the trainer
scaling generator wrote (policies.h, inventory.json) and the reviewed data in reviewed.json and landmarks.json.
Any validation failure aborts with every reason listed; nothing falls back silently at runtime.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re
import sys

TOOL = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOL))
import audit as A  # noqa: E402
import collect as C  # noqa: E402
import places as P  # noqa: E402

R = C.R
ROOT = C.ROOT
OUTPUT = ROOT / 'src/data/trainer_slots'
COUNT_HEADER = ROOT / 'include/constants/world_trainer_slots.h'
REVIEWED = TOOL / 'reviewed.json'
REGIONS = ('Kanto', 'Sevii', 'Johto', 'Alola', 'Sinjoh', 'Hoenn')
SEVII_CONTACT_BASE = 118        # contact index layout of phone-rematches.md: table rows 0-117, Sevii, then the coast
COAST_CONTACT_BASE = 150
CONTACT_COUNT = 160
SEVII_REMATCHES = ROOT / 'src/data/wayfarer_sevii_rematches.h'


class GenerateError(ValueError):
    pass


# ---------- regions and classes ----------
def region_of(map_name, root=ROOT):
    data = json.loads((root / 'data/maps' / map_name / 'map.json').read_text())
    version, region = data.get('game_version', 'emerald'), data.get('region')
    if version == 'emerald':
        return 'Hoenn'
    if version == 'frlg':
        return {'REGION_KANTO': 'Kanto', 'REGION_GALAR': 'Sevii'}.get(region, 'Other')
    return {'REGION_KANTO': 'Kanto', 'REGION_JOHTO': 'Johto', 'REGION_ALOLA': 'Alola', 'REGION_HISUI': 'Sinjoh'}.get(region, 'Other')


def class_family(klass, aliases):
    """TRAINER_CLASS_HIKER_HNS and TRAINER_CLASS_HIKER_FRLG are one family, HIKER."""
    name = klass.removeprefix('TRAINER_CLASS_')
    name = re.sub(r'_(HNS|FRLG)$', '', name)
    return aliases.get(name, name)


# ---------- units ----------
class Unit:
    """One trainer slot: the objects (one, or two for a pair) that show one home trainer."""
    __slots__ = ('objects', 'map_const', 'map_name', 'map_value', 'home', 'home_id', 'pair', 'klass', 'family', 'region', 'policy',
                 'status', 'reason', 'texts', 'group', 'index', 'lines', 'gift', 'contact', 'map_type', 'place', 'place_phrases', 'range_kind', 'absence')

    def __init__(self, **fields):
        for slot in self.__slots__:
            setattr(self, slot, fields.get(slot))

    @property
    def first(self):
        return self.objects[0]

    @property
    def label(self):
        ids = '+'.join(str(o.local_id) for o in self.objects)
        return f'{self.map_name} local id {ids} {self.home}'

    @property
    def key(self):
        return (self.map_value, self.objects[0].local_id)


def form_units(slots, errors):
    """Group the trainer objects of a map by home trainer: a pair is two objects of one double-battle trainer."""
    by_key = defaultdict(list)
    loose = []
    for slot in slots:
        if slot.home is None:
            loose.append(slot)
            continue
        by_key[(slot.map_const, slot.home)].append(slot)
    units = []
    for (const, home), objects in by_key.items():
        objects.sort(key=lambda s: s.local_id)
        first = objects[0]
        unit = Unit(objects=objects, map_const=const, map_name=first.map_name, map_value=first.map_value, home=home, home_id=first.home_id,
                    pair=first.pair, klass=first.klass, policy=first.policy, texts=[], gift=any(o.gift for o in objects), map_type=first.map_type)
        for obj in objects:
            unit.texts += obj.texts
        units.append(unit)
    return sorted(units, key=lambda u: u.key), loose


def classify(units, reviewed, errors):
    """Status of every unit: daily, or not daily with a reason (the first reason that applies)."""
    fixed_maps = reviewed['not_daily_maps']
    fixed_classes = {row['class']: row['reason'] for row in reviewed['not_daily_classes']}
    fixed_slots = {}
    for row in reviewed['not_daily_slots']:
        for ident in row['ids']:
            fixed_slots[(row['map'], ident)] = row['reason']
    seen = set()
    aliases = reviewed['class_aliases']
    for unit in units:
        reason = None
        objects = unit.objects
        problems = sorted({p for o in objects for p in o.commands['problems']})
        if unit.policy != 'ORDINARY':
            reason = f'classification {unit.policy or "none"}, not ORDINARY (trainer party scaling)'
        elif any(o.trainer_type not in C.ROTATING_TYPES for o in objects):
            reason = 'buried trainer: it reveals itself out of the ground'
        elif any(o.hide_flag not in C.NO_FLAG for o in objects):
            flags = sorted({o.hide_flag for o in objects if o.hide_flag not in C.NO_FLAG})
            reason = f'hidden by a flag ({", ".join(flags)}): a story or time-of-day trainer'
        elif problems:
            reason = 'script does more than battle: ' + ', '.join(problems)
        elif unit.pair and len(objects) != 2:
            reason = f'pair battle with {len(objects)} object(s), a pair needs exactly two'
        elif not unit.pair and len(objects) != 1:
            reason = f'{len(objects)} objects for one single-battle trainer'
        elif any(not re.fullmatch(r'OBJ_EVENT_GFX_\w+', o.gfx) or o.gfx.startswith('OBJ_EVENT_GFX_VAR') for o in objects):
            reason = 'variable graphics'
        elif not unit.klass:
            reason = 'no trainer class in the inventory'
        if reason is None:
            for row in fixed_maps:
                if unit.map_name.startswith(row['prefix']):
                    reason = row['reason']
                    break
        if reason is None:
            family = class_family(unit.klass, aliases)
            for key, text in fixed_classes.items():
                if family == key:
                    reason = text
        for obj in objects:
            mark = (unit.map_name, obj.local_id)
            if mark in fixed_slots:
                seen.add(mark)
                reason = reason or fixed_slots[mark]
        unit.status = 'not-daily' if reason else 'daily'
        unit.reason = reason
        unit.family = class_family(unit.klass, aliases) if unit.klass else None
    compiled = {u.map_name for u in units}
    for key in fixed_slots:
        if key not in seen:
            what = 'a trainer slot of a compiled map' if key[0] in compiled else 'a map Wayfarer compiles with trainers'
            errors.append(f'reviewed.json not_daily_slots names {key[0]} local id {key[1]}, which is not {what}')
    known_maps = {u.map_name for u in units}
    for row in fixed_maps:
        if not any(name.startswith(row['prefix']) for name in known_maps):
            errors.append(f'reviewed.json not_daily_maps prefix {row["prefix"]} matches no trainer slot map')
    families = {u.family for u in units}
    for key in fixed_classes:
        if key not in families:
            errors.append(f'reviewed.json not_daily_classes names {key}, which no trainer slot has')


# ---------- the dialogue audit ----------
def line_texts(unit, labels):
    """[(kind, label, text)] of the lines the unit's candidate says, in order, without repeats."""
    out, seen = [], set()
    for kind, label in unit.texts:
        if label in seen:
            continue
        seen.add(label)
        text = C.string_text(labels.get(label, ''))
        if text:
            out.append((kind, label, text))
    return out


def parse_range(row, specs, errors):
    """(kind, place) of a dialogue row: class, home or place:<type>."""
    text = row.get('range')
    if text in ('class', 'home'):
        return text, None
    if isinstance(text, str) and text.startswith('place:') and text[6:] in specs:
        return 'place', text[6:]
    errors.append(f'reviewed.json dialogue {row["trainer"]}: range must be class, home or place:<type> with a known type, not {text!r}')
    return None, None


def run_audit(units, labels, errors, reviewed):
    words = A.load_words()
    patterns = A.build_patterns(A.map_names(), words)
    specs = P.load_specs()
    resolutions = {}
    for row in reviewed['dialogue']:
        if row['trainer'] in resolutions:
            errors.append(f'reviewed.json dialogue lists {row["trainer"]} twice')
        if not row.get('reason'):
            errors.append(f'reviewed.json dialogue {row["trainer"]}: needs a reason')
        if bool(row.get('not_daily')) == ('range' in row):
            errors.append(f'reviewed.json dialogue {row["trainer"]}: needs either not_daily or a range')
        if 'range' in row:
            row['_range'] = parse_range(row, specs, errors)
            if row['_range'][0] == 'place' and not row.get('phrases'):
                errors.append(f'reviewed.json dialogue {row["trainer"]}: a place range needs the phrases that need it')
        resolutions[row['trainer']] = row
    flagged = {}
    for unit in units:
        if unit.status != 'daily':
            continue
        unit.lines = line_texts(unit, labels)
        hits = []
        for kind, label, text in unit.lines:
            for found in A.flag_text(text, patterns):
                hits.append((kind, label, text, found))
        if hits:
            flagged[unit.home] = hits
    used = set()
    for unit in units:
        if unit.status != 'daily':
            continue
        hits = flagged.get(unit.home)
        row = resolutions.get(unit.home)
        unit.range_kind = 'class'
        if row is not None:
            used.add(unit.home)
        if hits and row is None:
            errors.append(f'dialogue audit: {unit.label} names a place ({hits[0][3][1]!r}: {hits[0][2][:60]!r}) and has no reviewed resolution')
        if row is None:
            continue
        unit.reason = row['reason']
        if row.get('not_daily'):
            unit.status, unit.reason, unit.range_kind = 'not-daily', 'dialogue audit: ' + row['reason'], None
            continue
        kind, place = row.get('_range', (None, None))
        if kind is None:
            continue
        unit.range_kind = kind
        if 'absence' in row:
            unit.absence = row['absence']
        if kind == 'class':
            if not row.get('was'):
                if hits:
                    errors.append(f'dialogue audit: {unit.label} keeps its class range but a line names a place ({hits[0][3][1]!r}); reword it (listing the old line as was) or give it a place or home range')
            elif hits:
                errors.append(f'dialogue audit: {unit.label} is marked reworded but a line still names a place ({hits[0][3][1]!r}); fix the line or the resolution')
        elif kind == 'place':
            if not hits:
                errors.append(f'dialogue audit: {unit.label} is assigned a place type but no line is flagged any more; remove the resolution')
            else:
                named = {phrase.lower() for phrase in row.get('phrases', [])}
                for kind2, label, text, found in hits:
                    if found[1].lower() not in named:
                        errors.append(f'dialogue audit: {unit.label} line names {found[1]!r}, which its place type {place} does not cover; list the phrase or take it out of the daily slots')
                        break
                unit.place, unit.place_phrases = place, row.get('phrases', [])
    names = {u.home for u in units}
    for trainer in resolutions:
        if trainer not in names:
            errors.append(f'reviewed.json dialogue names {trainer}, which is not a trainer slot of a compiled map')
        elif trainer not in used:
            errors.append(f'reviewed.json dialogue resolves {trainer}, which is not a daily candidate')
    return flagged, resolutions


def apply_absence(units, reviewed, errors):
    """Every daily slot's absence chance: the default, or a reviewed per-trainer value with a reason."""
    default = reviewed['default_absence_percent']
    if not isinstance(default, int) or not 0 <= default <= 100:
        errors.append('reviewed.json default_absence_percent must be a percent')
    by_home = {u.home: u for u in units}
    seen = set()
    for row in reviewed['absence']:
        unit = by_home.get(row['trainer'])
        if row['trainer'] in seen:
            errors.append(f'reviewed.json absence lists {row["trainer"]} twice')
        seen.add(row['trainer'])
        if unit is None or unit.status != 'daily':
            errors.append(f'reviewed.json absence names {row["trainer"]}, which is not a daily trainer slot')
        elif not row.get('reason') or not isinstance(row.get('percent'), int) or not 0 <= row['percent'] <= 100:
            errors.append(f'reviewed.json absence {row["trainer"]}: needs a percent from 0 to 100 and a reason')
        elif unit.absence is not None:
            errors.append(f'reviewed.json absence {row["trainer"]}: its absence is already set by its dialogue resolution')
        else:
            unit.absence = row['percent']
    for unit in units:
        if unit.status == 'daily' and unit.absence is None:
            unit.absence = default
    levels = sorted({u.absence for u in units if u.status == 'daily'}, key=lambda p: (p != default, p))
    if len(levels) > 8:
        errors.append(f'{len(levels)} distinct absence chances do not fit the 3-bit index of a slot row')
    return levels


def check_scripts(daily, labels, errors, root=ROOT):
    """The table names each script by symbol, so it has to be a global label (`Label::`)."""
    text = R.script_text(root)
    globals_ = set(re.findall(r'^(\w+)::', text, re.M))
    for unit in daily:
        for obj in unit.objects:
            if obj.script not in globals_:
                errors.append(f'{unit.label}: script {obj.script} is not a global label, so the slot table cannot name it')


def check_reach(daily, errors, root=ROOT):
    """A wanderer's level comes from the reach of the map it battles on (trainer party scaling, v0 levels): that table has an
    entry for every map hosting an ORDINARY trainer, and every slot's map must be one of them."""
    text = (root / 'src/data/trainer_scaling/trainer_places.h').read_text()
    known = set(re.findall(r'MAP_GROUP\((MAP_\w+)\)', text))
    for const in sorted({u.map_const for u in daily} - known):
        errors.append(f'{const}: a daily trainer slot map with no reach entry in trainer_places.h, so a wanderer there would take the Road level')


def check_places(units, errors, root=ROOT):
    """A trainer in a place-type group stands in that kind of place: checked against the map metadata."""
    placed = [u for u in units if u.place and u.status == 'daily']
    if not placed:
        return
    specs = P.load_specs()
    grid_maps = {u.map_name for u in placed if u.place in specs and P.needs_grid(specs[u.place])}
    world = P.load_world(grid_maps, root) if grid_maps else None
    for unit in placed:
        if unit.place not in specs:
            errors.append(f'{unit.label}: unknown place type {unit.place}')
        elif not P.holds(specs[unit.place], unit, world):
            errors.append(f'{unit.label}: assigned place type {unit.place} ({specs[unit.place]["describe"]}) but its slot is not one')


# ---------- ranges and contacts ----------
GROUP_HOME = 255    # the group field of a home-range slot: no group, the slot's own spot is its whole range


def group_label(unit):
    """The group a class- or place-range slot belongs to: its place type (marked with @) or its class family. A home range has none."""
    if unit.range_kind == 'home':
        return None
    return '@' + unit.place if unit.place else unit.family


def make_groups(units, errors):
    daily = [u for u in units if u.status == 'daily']
    for unit in daily:
        unit.region = region_of(unit.map_name)
        if unit.region not in REGIONS:
            errors.append(f'{unit.label}: region {unit.region} has no trainer group')
    for index, unit in enumerate(daily):
        unit.index = index
    grouped = [u for u in daily if unit_has_group(u)]
    keys = sorted({(REGIONS.index(u.region) if u.region in REGIONS else 99, group_label(u), u.pair) for u in grouped})
    by_key = {key: gid for gid, key in enumerate(keys)}
    if len(keys) > GROUP_HOME:
        errors.append(f'{len(keys)} trainer groups do not fit the {GROUP_HOME} ids a slot row\'s group field has before the home marker')
    members = defaultdict(list)
    for unit in daily:
        if unit.range_kind == 'home':
            unit.group = None
            continue
        if not group_label(unit) or unit.region not in REGIONS:
            errors.append(f'{unit.label}: a daily trainer without a range')
            continue
        unit.group = by_key[(REGIONS.index(unit.region), group_label(unit), unit.pair)]
        members[unit.group].append(unit)
    homes = Counter(u.home for u in daily)
    for home, count in homes.items():
        if count > 1:
            errors.append(f'{home} is the home of {count} daily slots; a trainer is in one place a day')
    for gid, group in members.items():
        names = [u.home for u in group]
        if len(set(names)) != len(names):
            errors.append(f'group {gid}: a wanderer cannot fit its slots (duplicate home trainer)')
    return keys, members


def unit_has_group(unit):
    return unit.range_kind != 'home'


def sevii_contacts(root=ROOT):
    """The Sevii families that become phone contacts: those whose rematch stages differ (a stronger team), in registry order.
    Returns a list the registry's length: the contact index or 255."""
    families = []
    for match in re.finditer(r'SEVII_REMATCH\(([^)]*)\)', (root / 'src/data/wayfarer_sevii_rematches.h').read_text()):
        families.append([part.strip() for part in match.group(1).split(',')])
    result, next_index = [], SEVII_CONTACT_BASE
    for ids in families:
        if len(set(ids)) > 1:
            result.append(next_index)
            next_index += 1
        else:
            result.append(255)
    if next_index > COAST_CONTACT_BASE:
        raise GenerateError(f'{next_index - SEVII_CONTACT_BASE} Sevii contacts do not fit the {COAST_CONTACT_BASE - SEVII_CONTACT_BASE} reserved')
    return result


# ---------- output ----------
def c_trainer(name):
    return name


def render_slots(daily, members, keys, fixed_homes, sevii, per_map_max, group_max, levels):
    lines = ['// Generated by tools/wayfarer_trainer_slots/generate.py; edit reviewed.json, the maps or the classification, then regenerate.',
             "// One row per daily trainer slot, ordered by map, then local id: the row number is the slot's dense index.", '']
    scripts = sorted({o.script for u in daily for o in u.objects})
    lines += [f'extern const u8 {name}[];' for name in scripts] + ['']
    lines += ['// The absence chances (percent) the rows use; the index is in bits 2-4 of a row\'s flags.',
              'static const u8 sWorldTrainerAbsence[] = { ' + ', '.join(str(p) for p in levels) + ' };', '']
    lines += ['static const struct WorldTrainerSlot sWorldTrainerSlots[WORLD_TRAINER_SLOT_COUNT] =', '{']
    for unit in daily:
        a = unit.objects[0]
        b = unit.objects[1] if len(unit.objects) > 1 else None
        flags = ['WORLD_TRAINER_SLOT_PAIR'] if unit.pair else []
        if unit.range_kind == 'home':
            flags.append('WORLD_TRAINER_SLOT_HOME')
        flags.append(f'WORLD_TRAINER_ABSENCE({levels.index(unit.absence)})')
        group = 'WORLD_TRAINER_GROUP_HOME' if unit.group is None else str(unit.group)
        lines.append(f'    {{ {c_trainer(unit.home)}, MAP_GROUP({unit.map_const}), MAP_NUM({unit.map_const}), {{ {a.local_id}, {b.local_id if b else 0} }}, '
                     f'{group}, {" | ".join(flags)}, '
                     f'{{ {a.gfx}, {b.gfx if b else "0"} }}, {{ {a.script}, {b.script if b else "NULL"} }} }},'
                     f' // {unit.map_name} {group_label(unit) or "home"} {unit.absence}%')
    lines += ['};', '']
    flat, starts = [], []
    for gid in range(len(keys)):
        starts.append((len(flat), len(members[gid])))
        flat += [u.index for u in sorted(members[gid], key=lambda u: u.index)]
    lines += ['// The slots of each class or place group in a fixed order (ascending slot index), flat; a group is a start and a count.',
              'static const u16 sWorldTrainerGroupSlots[] =', '{']
    for start in range(0, len(flat), 12):
        lines.append('    ' + ', '.join(str(i) for i in flat[start:start + 12]) + ',')
    lines += ['};', '', 'static const struct WorldTrainerGroup sWorldTrainerGroups[WORLD_TRAINER_GROUP_COUNT] =', '{']
    for gid, (start, count) in enumerate(starts):
        region, family, pair = keys[gid]
        lines.append(f'    {{ {start}, {count} }}, // {gid}: {REGIONS[region]} {family}{" (pairs)" if pair else ""}')
    lines += ['};', '', '// Contacts that are not daily stay at home: where a rematch family\'s first trainer stands (WhereIsTrainerToday).',
              'static const struct WorldTrainerFixedHome sWorldTrainerFixedHomes[] =', '{']
    for unit in fixed_homes:
        lines.append(f'    {{ {c_trainer(unit.home)}, MAP_GROUP({unit.map_const}), MAP_NUM({unit.map_const}), {unit.first.local_id} }}, // {unit.map_name}')
    lines += ['};', '', '// Contact index of each Sevii rematch family (255: not a phone contact).',
              'static const u8 sWorldTrainerSeviiContacts[] =', '{']
    for start in range(0, len(sevii), 16):
        lines.append('    ' + ', '.join(str(i) for i in sevii[start:start + 16]) + ',')
    lines += ['};', '']
    return '\n'.join(lines)


def render_count(slot_count, group_count, fixed_count, per_map_max, group_max, sevii_count):
    return ('// Generated by tools/wayfarer_trainer_slots/generate.py.\n'
            '#ifndef GUARD_CONSTANTS_WORLD_TRAINER_SLOTS_H\n#define GUARD_CONSTANTS_WORLD_TRAINER_SLOTS_H\n\n'
            f'#define WORLD_TRAINER_SLOT_COUNT        {slot_count}\n'
            f'#define WORLD_TRAINER_GROUP_COUNT       {group_count}\n'
            f'#define WORLD_TRAINER_FIXED_HOME_COUNT  {fixed_count}\n'
            f'#define WORLD_TRAINER_MAP_SLOTS_MAX     {per_map_max}   // most daily slots on one map\n'
            f'#define WORLD_TRAINER_GROUP_SLOTS_MAX   {group_max}   // most slots in one group\n'
            f'#define WORLD_TRAINER_SEVII_FAMILIES    {sevii_count}\n\n#endif\n')


def reason_class(reason):
    """The reason with its per-slot detail dropped, so the totals table groups alike slots."""
    if reason.startswith('dialogue audit'):
        return 'dialogue audit: a line is about the place (resolved below)'
    if reason.startswith('script does more'):
        return 'script does more than battle'
    if reason.startswith('hidden by a flag'):
        return 'hidden by a story or time-of-day flag'
    return reason


def render_report(units, daily, members, keys, flagged, resolutions, reviewed, levels):
    default = reviewed['default_absence_percent']
    lines = ['# Trainer slots report', '',
             'Generated by `tools/wayfarer_trainer_slots/generate.py`; the tables are `slots.h`, the lines are in `dialogue.md`. One rule:',
             'a beaten trainer is placed somewhere in its range each day and is battleable once there; each free slot is empty with its',
             'absence chance. Slot status by region with the reason for each, the groups and the dialogue audit. See the daily world slots spec.', '']
    by_region = defaultdict(lambda: Counter())
    for unit in units:
        region = unit.region or region_of(unit.map_name)
        if unit.status == 'daily':
            kind = unit.range_kind
        else:
            kind = 'not-daily' if unit.policy == 'ORDINARY' else 'other'
        by_region[region][kind] += 1
    count = lambda kind: sum(1 for u in daily if u.range_kind == kind)
    zero = [u for u in daily if u.absence == 0]
    lines += ['## Totals by region', '',
              'A slot is one trainer or one pair. A daily trainer\'s range is its class group (class), a place-type group of the kind of place its lines',
              'need (place), or its own slot (home). Not daily are `ORDINARY` trainers that stay where they are authored (reason below); other are',
              'trainers the scaling classification keeps out of the daily slots (Gym members, bosses, ...).', '',
              '| Region | Class range | Place range | Home range | Not daily (ORDINARY) | Other trainers |', '| --- | ---: | ---: | ---: | ---: | ---: |']
    for region in REGIONS:
        counts = by_region.get(region)
        if counts:
            lines.append(f"| {region} | {counts['class']} | {counts['place']} | {counts['home']} | {counts['not-daily']} | {counts['other']} |")
    lines.append(f"| total | {count('class')} | {count('place')} | {count('home')} | "
                 f"{sum(1 for u in units if u.status == 'not-daily' and u.policy == 'ORDINARY')} | {sum(1 for u in units if u.policy != 'ORDINARY')} |")
    levels_text = ', '.join(f'{p}%: {sum(1 for u in daily if u.absence == p)}' for p in levels)
    lines += ['', f'{len(daily)} daily slots. Absence chances (default {default}%): {levels_text}. {len(zero)} slot(s) at 0% (someone is always there).']
    if zero:
        lines += ['', '| Trainer | Slot | Range | Why |', '| --- | --- | --- | --- |']
        why = {row['trainer']: row['reason'] for row in reviewed['absence']}
        why.update({row['trainer']: row['reason'] for row in resolutions.values() if 'absence' in row})
        lines += [f'| {u.home} | {u.map_name} | {u.range_kind} | {why.get(u.home, "")} |' for u in zero]
    lines += ['']
    reasons = Counter()
    for unit in units:
        if unit.status == 'not-daily' and unit.policy == 'ORDINARY':
            reasons[reason_class(unit.reason)] += 1
    lines += ['## Why ORDINARY trainers are not daily', '', '| Reason | Slots |', '| --- | ---: |']
    for reason, n in sorted(reasons.items(), key=lambda kv: (-kv[1], kv[0])):
        lines.append(f'| {reason} | {n} |')
    lines += ['']
    lines += ['## Trainer groups', '',
              'One group per class family and region, split into singles and pairs, and one group per place type and region for the trainers',
              'whose lines need a kind of place (marked @, mixing classes; each slot is checked against `place_types.json`). Wanderers of a group',
              'are its defeated trainers and fill its slots, so a group of one slot never shows anyone but its own trainer. A home range has no',
              'group: the slot is its own range (the row\'s group field is the home marker), so it costs no group id.', '',
              '| Group | Region | Class family or @place type | Pairs | Slots |', '| ---: | --- | --- | --- | ---: |']
    for gid, (region, family, pair) in enumerate(keys):
        lines.append(f'| {gid} | {REGIONS[region]} | {family} | {"yes" if pair else ""} | {len(members[gid])} |')
    place_groups = [gid for gid, key in enumerate(keys) if key[1].startswith('@')]
    lines += ['', f'{len(place_groups)} place-type groups, {sum(1 for g in place_groups if len(members[g]) == 1)} of one slot; '
              f'{sum(len(members[g]) for g in place_groups)} trainers in them. {count("home")} trainers have a home range.']
    sizes = Counter(len(members[g]) for g in members)
    lines += ['', f'{len(keys)} groups of {GROUP_HOME} possible; sizes: ' + ', '.join(f'{n} slot(s) x {c}' for n, c in sorted(sizes.items())), '']
    lines += ['## Dialogue audit', '',
              f'{len(daily) + sum(1 for u in units if u.status == "not-daily" and u.reason.startswith("dialogue audit"))} candidates were audited; '
              f'{len(flagged)} have a line that names a map, landmark, building or local event. Each is resolved below by its range: **class** after a',
              'reword (the line was edited to neutral wording in the script source, the audit then finds nothing), **place:<type>** (the trainer',
              'joins the group of that place type, whose slots are all that kind of place), **home** (the lines are about the spot itself, so the',
              'trainer is placed only in its own slot), or **not daily** (its absence would break its lines).', '']
    kinds = Counter('not daily' if r.get('not_daily') else r['_range'][0] for r in resolutions.values())
    lines += [f'Resolutions: {kinds["class"]} class (reworded), {kinds["place"]} place, {kinds["home"]} home, {kinds["not daily"]} not daily.', '']
    lines += ['| Trainer | Slot | Range | Absence | Reason |', '| --- | --- | --- | --- | --- |']
    by_home = {u.home: u for u in units}
    for trainer, row in sorted(resolutions.items()):
        unit = by_home.get(trainer)
        if row.get('not_daily'):
            what = 'not daily'
        else:
            what = row['range'] + (' (' + ', '.join(f'"{p}"' for p in row['phrases']) + ')' if row['range'].startswith('place') else '')
        lines.append(f'| {trainer} | {unit.map_name if unit else "-"} | {what} | {unit.absence if unit and unit.status == "daily" else "-"} | {row["reason"]} |')
    words = A.load_words()
    lines += ['', '### Phrases the audit allows', '',
              'Narrow phrases that read fine anywhere (case-sensitive). A trainer whose only flagged phrases are these keeps its class range.', '',
              '| Pattern | Why it is fine anywhere |', '| --- | --- |']
    lines += [f'| `{row["pattern"]}` | {row["reason"]} |' for row in words['allowed_phrases']]
    lines += ['']
    lines += ['## Slots', '', 'Every trainer slot with an `ORDINARY` trainer: daily slots by index, then the ones that are not daily.', '',
              '| # | Slot | Trainer | Range | Group | Absence | Status |', '| ---: | --- | --- | --- | ---: | ---: | --- |']
    for unit in daily:
        lines.append(f'| {unit.index} | {unit.label.split(" " + unit.home)[0]} | {unit.home} | {group_label(unit) or "home"} | '
                     f'{"-" if unit.group is None else unit.group} | {unit.absence}% | daily |')
    for unit in units:
        if unit.status == 'not-daily' and unit.policy == 'ORDINARY':
            lines.append(f'| - | {unit.label.split(" " + unit.home)[0]} | {unit.home} | {unit.family or "-"} | - | - | not daily: {unit.reason} |')
    return '\n'.join(lines) + '\n'


def render_dialogue(daily_all, flagged):
    lines = ['# Trainer dialogue inventory', '',
             'Generated by `tools/wayfarer_trainer_slots/generate.py`. Every daily candidate\'s intro, defeat, post-battle and rematch lines,',
             'as the dialogue audit read them. A mark in the last column means a line names a map, landmark, building or local event.', '',
             '| Trainer | Slot | Line | Text | Flag |', '| --- | --- | --- | --- | --- |']
    for unit in daily_all:
        marks = {(label): found for _, label, _, found in flagged.get(unit.home, [])}
        for kind, label, text in unit.lines or []:
            mark = marks.get(label)
            lines.append(f'| {unit.home} | {unit.map_name} | {kind} | {text.replace("|", "/")} | {(mark[0] + ": " + mark[1]) if mark else ""} |')
    return '\n'.join(lines) + '\n'


# ---------- driver ----------
def build(root=ROOT):
    reviewed = json.loads(REVIEWED.read_text())
    errors = []
    maps = R.load_maps(root)
    labels = R.label_index(root)
    talk_only = []
    slots, family_of, numbers = C.collect_slots(maps, labels, root, talk_only)
    units, loose = form_units(slots, errors)
    classify(units, reviewed, errors)
    flagged, resolutions = run_audit(units, labels, errors, reviewed)
    for unit in units:
        unit.region = unit.region or region_of(unit.map_name)
    levels = apply_absence(units, reviewed, errors)
    check_places(units, errors, root)
    keys, members = make_groups(units, errors)
    daily = sorted((u for u in units if u.status == 'daily'), key=lambda u: u.index)
    fixed_homes = [u for u in units if u.status == 'not-daily' and u.policy == 'ORDINARY' and u.home in family_of
                   and family_of[u.home][0] == u.home]
    # A talk-only rematch lead (Route 42's Tully) never sights the player, so there is no slot to rotate: it stays at home.
    for unit in form_units([s for s in talk_only if s.home is not None], errors)[0]:
        if unit.policy == 'ORDINARY' and unit.home in family_of and family_of[unit.home][0] == unit.home:
            unit.status, unit.reason = 'not-daily', 'talk-only: no sight line to rotate'
            fixed_homes.append(unit)
    per_map = Counter(u.map_const for u in daily)
    per_map_max = max(per_map.values()) if per_map else 0
    group_max = max((len(m) for m in members.values()), default=0)
    for const, count in per_map.items():
        locals_ = sorted(o.local_id for u in daily if u.map_const == const for o in u.objects)
        if len(set(locals_)) != len(locals_):
            errors.append(f'{const}: duplicate local ids among daily slots')
        ids = [o['local_id'] for o in maps[const].events['objects']]
        if ids != list(range(1, len(ids) + 1)):
            errors.append(f'{maps[const].name}: local ids are not dense, the runtime indexes templates by local id - 1')
    check_scripts(daily, labels, errors, root)
    check_reach(daily, errors, root)
    if errors:
        raise GenerateError('\n'.join(sorted(set(errors))))
    sevii = sevii_contacts(root)
    return {'units': units, 'daily': daily, 'members': members, 'keys': keys, 'flagged': flagged, 'resolutions': resolutions,
            'fixed_homes': fixed_homes, 'per_map_max': per_map_max, 'group_max': group_max, 'sevii': sevii, 'reviewed': reviewed, 'levels': levels}


def outputs(state):
    daily = state['daily']
    count = render_count(len(daily), len(state['keys']), len(state['fixed_homes']), state['per_map_max'], state['group_max'], len(state['sevii']))
    return {
        'slots.h': render_slots(daily, state['members'], state['keys'], state['fixed_homes'], state['sevii'], state['per_map_max'], state['group_max'], state['levels']),
        '@count': count,
        'report.md': render_report(state['units'], daily, state['members'], state['keys'], state['flagged'], state['resolutions'], state['reviewed'], state['levels']),
        'dialogue.md': render_dialogue(daily, state['flagged']),
    }


def write_outputs(files, check):
    for name, content in files.items():
        path = COUNT_HEADER if name == '@count' else OUTPUT / name
        if check:
            if not path.exists() or path.read_text() != content:
                raise GenerateError(f'stale generated output: {path.relative_to(ROOT)}')
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--check', action='store_true', help='fail when the committed output is stale')
    args = parser.parse_args()
    try:
        state = build()
        write_outputs(outputs(state), args.check)
    except (GenerateError, R.ReachError, OSError, ValueError, KeyError) as error:
        parser.exit(1, f'{error}\n')
    daily = state['daily']
    print(json.dumps({'slots': len(daily), 'groups': len(state['keys']), 'fixed_homes': len(state['fixed_homes']),
                      'flagged': len(state['flagged']), 'map_max': state['per_map_max'], 'group_max': state['group_max']}, sort_keys=True))


if __name__ == '__main__':
    main()
