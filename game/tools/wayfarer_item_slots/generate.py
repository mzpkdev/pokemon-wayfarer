#!/usr/bin/env python3
"""Generate the daily world slots' item spot table and dynamic pools.

Specs: .product/specs/world-items.md (reach of an item spot, dynamic pools, static prizes,
fixed story items, Validation) and .product/specs/daily-world-slots.md (Slot data, Items,
Generation and reports). Applies to the Wayfarer build only.

Writes, under src/data/item_slots/:
  spots.h   the item spot rows, sorted by map then ball/hidden then id (the row number is the
            spot's dense slot index in the save's clearedToday bitset)
  pools.h   tier odds and the weighted Regular/Better/Special pools per region
  include/constants/world_item_spots.h  the spot count the save struct is sized from
  report.md slot status by region with reasons, and the pool weights per reach and region

    python3 tools/wayfarer_item_slots/generate.py [--check]

Reads the Wayfarer build's mapjson outputs (run `make BUILD=wayfarer generated`), the shared reach
resolution of tools/trainer_scaling, and the reviewed data in data.json (transcribed from the
world items spec by spec_tables.py) and fixed.json. Any validation failure aborts with every
reason listed; nothing falls back silently at runtime.
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
import collect as C  # noqa: E402

R = C.R
ROOT = C.ROOT
OUTPUT = ROOT / 'src/data/item_slots'
COUNT_HEADER = ROOT / 'include/constants/world_item_spots.h'
DATA = TOOL / 'data.json'
FIXED = TOOL / 'fixed.json'
ITEMS_HEADER = ROOT / 'src/data/items.h'
TMS_HEADER = ROOT / 'include/constants/tms_hms.h'
POOL_REGIONS = ('Kanto', 'Sevii', 'Johto', 'Alola', 'Sinjoh', 'Hoenn', 'Other')
TIER_NAMES = ('Road', 'Wilds', 'Outlands')
POOL_TIERS = ('Regular', 'Better', 'Special')
GROUP_STONE, GROUP_TM = 'STONE', 'TM'
KEY_OR_HM_POCKETS = ('POCKET_KEY_ITEMS',)
# Key-pocket items that Wayfarer treats as consumables, so their balls become dynamic (daily world slots PRD, Content).
CONSUMABLE_KEY_POCKET = ('ITEM_ESCAPE_ROPE',)
# Regional flavour (world-items.md, Regional flavour): additions and moves on top of the shared tables.
APRICORN_WEIGHT = 2
SHARD_WEIGHT = 2
FLAVOUR_MOVES = {
    'Hoenn': [('ITEM_HEART_SCALE', 'Special', 'Better', 3)],
    'Sevii': [('ITEM_BIG_PEARL', 'Special', 'Better', 2)],
    'Kanto': [('ITEM_MOOMOO_MILK', 'Better', 'Regular', 4)],
}
SEVII_PEARL_BONUS = 3


OVERRIDES = json.loads(FIXED.read_text()).get('region_overrides', {})


NOTES = []  # spec-vs-data observations the report lists


class GenerateError(ValueError):
    pass


# ---------- item data ----------
def item_symbols():
    """{ITEM_*: pocket} for every item name the Wayfarer build defines: item data, aliases and TM/HM names."""
    text = ITEMS_HEADER.read_text()
    marks = list(re.finditer(r'^\s*\[(ITEM_\w+)\]\s*=', text, re.M))
    pockets = {}
    for i, mark in enumerate(marks):
        end = marks[i + 1].start() if i + 1 < len(marks) else len(text)
        pocket = re.search(r'\.pocket\s*=\s*(\w+)', text[mark.end():end])
        pockets[mark.group(1)] = pocket.group(1) if pocket else None
    constants = (ROOT / 'include/constants/items.h').read_text()
    for name, target in re.findall(r'^\s*(ITEM_\w+)\s*=\s*(ITEM_\w+)\s*,', constants, re.M):
        if target in pockets:
            pockets.setdefault(name, pockets[target])
    for name in re.findall(r'^\s*(ITEM_(?:TM|HM)\d+)\s*=', constants, re.M):
        pockets.setdefault(name, 'POCKET_TM_HM')
    for name in macro_names('TM'):
        pockets.setdefault('ITEM_TM_' + name, 'POCKET_TM_HM')
    for name in macro_names('HM'):
        pockets.setdefault('ITEM_HM_' + name, 'POCKET_TM_HM')
    return pockets


def macro_names(name):
    """The first (Wayfarer) FOREACH_<name>(F) list of tms_hms.h."""
    text = TMS_HEADER.read_text()
    start = text.index(f'#define FOREACH_{name}(F)')
    block = text[start:text.index('\n\n', start)]
    return re.findall(r'F\((\w+)\)', block)


# ---------- spots ----------
def classify(spots, fixed, pockets, errors):
    """Mark each collected spot item/fixed/empty; every ball without a recognised item must be reviewed in fixed.json."""
    explicit = {}
    for row in fixed['spots']:
        for ident in row['ids']:
            explicit[(row['map'], ident)] = row['reason']
        for x, y in row['hidden']:
            explicit[(row['map'], 'hidden', x, y)] = row['reason']
    seen = set()
    prefixes = fixed['map_prefixes']
    for spot in spots:
        reason = None
        mark = (spot.map_name, spot.id) if spot.kind == 'ball' else (spot.map_name, 'hidden', spot.x, spot.y)
        if mark in explicit:
            reason = explicit[mark]
            seen.add(mark)
        else:
            for row in prefixes:
                if spot.map_name.startswith(row['prefix']):
                    reason = row['reason']
        if reason:
            spot.status, spot.reason = 'fixed', reason
        elif spot.status == 'unrecognised':
            errors.append(f'{spot.label}: an item ball whose script gives no single item; add it to fixed.json if it is a story or gift ball')
        elif spot.status == 'empty':
            spot.reason = 'hidden item row with no item'
        elif spot.authored not in pockets:
            errors.append(f'{spot.label}: authored item {spot.authored} is not an item of the Wayfarer build')
        elif (pockets[spot.authored] in KEY_OR_HM_POCKETS and spot.authored not in CONSUMABLE_KEY_POCKET) or spot.authored.startswith('ITEM_HM'):
            spot.status, spot.reason = 'fixed', f'key item or HM ({spot.authored}) keeps its authored spot'
    for key in explicit:
        if key not in seen:
            errors.append(f'fixed.json names {key[0]} {key[1:]}, which is not an item spot of a compiled map')


def region_and_reach(spots, errors, maps, root):
    """Region of every kept spot and its reach tier (world-items.md, Reach of an item spot)."""
    listed, wild = R.wild_places(root)
    resolver = R.Resolver(maps, listed, wild, R.Rules())
    region_cache, reach_cache = {}, {}
    for spot in spots:
        if spot.status != 'item':
            continue
        if spot.map_name not in region_cache:
            region_cache[spot.map_name] = C.region_of(spot.map_name, root)
        spot.region = region_cache[spot.map_name]
        for prefix, region in OVERRIDES.items():
            if spot.map_name.startswith(prefix):
                spot.region = region
        if spot.map_const not in reach_cache:
            reach_cache[spot.map_const] = resolve_reach(resolver, spot.map_const, maps, errors)
        spot.tier, spot.reach_rule, spot.reach_detail = reach_cache[spot.map_const] or (None, None, None)


def resolve_reach(resolver, const, maps, errors):
    name = maps[const].name
    try:
        found = resolver.resolve(const)
    except R.ReachError as error:
        errors.append(f'{name}: {error}')
        return None
    if found is None:
        if R.GYM_RE.search(name):
            return 'Road', 'gym', 'item spots inside Gyms count as Road'
        if maps[const].map_type in ('MAP_TYPE_TOWN', 'MAP_TYPE_CITY'):
            return 'Road', 'settlement', 'a town or city is Road (reach assignments, Settlement rule)'
        errors.append(f'{name}: no rule resolves its reach (add it to reach_places.json)')
        return None
    place, rule, detail = found
    reach = place['reach']
    if reach == 'WILD_REACH_DUNGEON':
        # A single-floor or flat dungeon has no entrance floor: it counts as deeper on every map (reach assignments).
        entrance = place['floor'] == 0 and not place['flat'] and place['floorCount'] > 1
        return ('Wilds' if entrance else 'Outlands'), rule, f"{detail}; {'entrance floor' if entrance else 'deeper floor'}"
    return reach.removeprefix('WILD_REACH_').capitalize(), rule, detail


# ---------- prizes ----------
def locate(spec, spots, sevii, errors, where):
    """The Spot a prize (or moved-from) row names; ball rows use the compiled local id, Sevii ones their exploration row."""
    name = spec['map'] if 'map' in spec else None
    candidates = [s for s in spots if s.map_name == name]
    if not candidates:
        errors.append(f'{where}: map {name} is not compiled by Wayfarer')
        return None
    if spec['kind'] == 'ball':
        ident = spec['local_id']
        if name in sevii:
            row = sevii[name].get(ident - 1)
            compiled = [s for s in candidates if s.kind == 'ball' and row is not None and s.id == row[0]]
            if row is not None and (row[1], row[2]) == (spec['x'], spec['y']) and compiled and (compiled[0].x, compiled[0].y) == (spec['x'], spec['y']):
                ident = row[0]
            else:
                # The spec's local id (N + 1) disagrees with the exploration row at these coordinates: coordinates win.
                found = [s for s in candidates if s.kind == 'ball' and (s.x, s.y) == (spec['x'], spec['y'])]
                if len(found) == 1:
                    if found[0].id != spec['local_id']:
                        NOTES.append(f"{where}: {name} local id {spec['local_id']} is not the ball at {spec['x']},{spec['y']}; resolved by coordinates to local id {found[0].id}")
                    ident = found[0].id
                else:
                    errors.append(f'{where}: no Sevii ball at {spec["x"]},{spec["y"]} in {name}')
                    return None
        match = [s for s in candidates if s.kind == 'ball' and s.id == ident]
    else:
        match = [s for s in candidates if s.kind == 'hidden' and (s.x, s.y) == (spec['x'], spec['y'])]
    if len(match) != 1:
        errors.append(f'{where}: {name} has no {"item ball" if spec["kind"] == "ball" else "hidden item"} '
                      f'{spec.get("local_id", "")} at {spec["x"]},{spec["y"]}')
        return None
    spot = match[0]
    if spec['kind'] == 'ball' and (spot.x, spot.y) != (spec['x'], spec['y']):
        errors.append(f'{where}: {name} local id {spot.id} sits at {spot.x},{spot.y}, not {spec["x"]},{spec["y"]}')
        return None
    return spot


def no_flag(flag):
    return flag in C.NO_FLAG


def apply_prizes(spots, data, pockets, sevii, errors):
    tms = {'ITEM_TM_' + name for name in macro_names('TM')}
    mart_tms = set(data['mart_tms'])
    by_region_item = defaultdict(list)
    legends = defaultdict(list)
    taken = {}
    for prize in data['prizes']:
        where = f"{prize['region']} {prize['trail']} #{prize['n']}"
        item = prize['item']
        if item not in pockets:
            errors.append(f'{where}: {item} is not an item of the Wayfarer build')
            continue
        if item in mart_tms:
            errors.append(f'{where}: {item} is a TM a mart sells')
        spot = locate(prize, spots, sevii, errors, where)
        if prize.get('moved_from'):
            origin = locate({**prize['moved_from']['spot'], 'map': prize['moved_from']['map']}, spots, sevii, errors, f'{where} (moved from)')
            if origin is not None:
                if origin.status == 'fixed':
                    errors.append(f'{where}: moved from a fixed story spot ({origin.label})')
                if origin is spot:
                    errors.append(f'{where}: moved from its own spot')
        if spot is None:
            continue
        if spot.status == 'fixed':
            errors.append(f'{where}: {spot.label} is a fixed story spot ({spot.reason})')
            continue
        if spot in taken:
            errors.append(f'{where}: {spot.label} is also the prize of {taken[spot]}')
            continue
        if spot.status != 'item':
            errors.append(f'{where}: {spot.label} is not an item spot')
            continue
        taken[spot] = where
        if (prize['shown'] == 'Hidden') != (spot.kind == 'hidden'):
            errors.append(f'{where}: the spec shows it as {prize["shown"]} but {spot.label} is a {"hidden item" if spot.kind == "hidden" else "ball"}')
        if no_flag(spot.flag):
            errors.append(f'{where}: {spot.label} has no flag to keep, so it cannot be a prize')
        if spot.kind == 'ball' and spot.tier == 'Road':
            errors.append(f'{where}: a visible ball on Road ({spot.label}) is never a prize')
        if prize['tier'] == 'Legend':
            legends[item].append(where)
        by_region_item[(spot.region, item)].append(where)
        if spot.region != prize['region']:
            errors.append(f"{where}: {spot.label} is in region {spot.region}, not {prize['region']}")
        spot.status, spot.prize, spot.prize_tier = 'prize', item, prize['tier']
        spot.reason = f"{prize['trail']} #{prize['n']} ({prize['tier']})"
    for item, places in legends.items():
        if len(places) > 1:
            errors.append(f'Legend {item} appears more than once: {places}')
    legend_items = set(legends)
    for prize in data['prizes']:
        if prize['tier'] != 'Legend' and prize['item'] in legend_items:
            errors.append(f"{prize['region']} {prize['trail']} #{prize['n']}: {prize['item']} is a Legend elsewhere and unique game-wide")
    for (region, item), places in by_region_item.items():
        if len(places) > 1 and item not in legend_items:
            errors.append(f'{item} is a prize twice in {region}: {places}')
    for prize in data['prizes']:
        if prize['item'].startswith('ITEM_TM_') and prize['item'] not in tms:
            errors.append(f"{prize['item']} is not a TM of the Wayfarer build")
    counts = Counter(p['region'] for p in data['prizes'])
    for region, expected in data['spec_totals'].items():
        if counts[region] != expected['total']:
            errors.append(f'{region}: {counts[region]} prizes, the spec totals say {expected["total"]}')
    if len(data['prizes']) != data['spec_overall']:
        errors.append(f"{len(data['prizes'])} prizes, the spec says {data['spec_overall']}")
    return legend_items


def reach_disagreements(spots, data):
    """Prize rows whose spec Reach column names a different tier than the resolved one (report only)."""
    out = []
    by_key = {}
    for spot in spots:
        if spot.status == 'prize':
            by_key[(spot.map_name, spot.kind, spot.x, spot.y)] = spot
    for prize in data['prizes']:
        spot = by_key.get((prize['map'], prize['kind'], prize['x'], prize['y']))
        if spot is None and prize['kind'] == 'ball':
            spot = next((s for s in by_key.values() if s.map_name == prize['map'] and s.kind == 'ball' and (s.x, s.y) == (prize['x'], prize['y'])), None)
        if spot is None:
            continue
        text = prize['spec_reach']
        if text.startswith('Road'):
            said = 'Road'
        elif text.startswith('Wilds'):
            said = 'Wilds'
        elif text.startswith('Outlands'):
            said = 'Outlands'
        else:
            match = re.search(r'(?:floor|room) (\d+) of (\d+)', text) or re.search(r'\((\d+) of (\d+)\)', text)
            if not match:
                continue
            said = 'Wilds' if match.group(1) == '1' else 'Outlands'
        if said != spot.tier:
            out.append((spot, text, said))
    return out


# ---------- pools ----------
def pool_tables(data, pockets, legend_items):
    """{region: {tier: [(item or group, weight)]}} and the group member lists."""
    tms = ['ITEM_TM_' + name for name in macro_names('TM')]
    dynamic_tms = [tm for tm in tms if tm not in legend_items]
    base = {tier: [(row['item'], row['weight']) for row in data['pools'][tier]] for tier in POOL_TIERS}
    for tier in POOL_TIERS:
        for item, _ in base[tier]:
            if item == 'Evolution stone':
                continue
            if item == 'TM':
                continue
            if item not in pockets:
                raise GenerateError(f'pool item {item} is not an item of the Wayfarer build')
    base = {tier: [(GROUP_STONE if item == 'Evolution stone' else GROUP_TM if item == 'TM' else item, weight) for item, weight in rows]
            for tier, rows in base.items()}
    apricorns = sorted(item for item in pockets if item.endswith('_APRICORN'))
    shards = ['ITEM_RED_SHARD', 'ITEM_BLUE_SHARD', 'ITEM_YELLOW_SHARD', 'ITEM_GREEN_SHARD']
    pools = {}
    for region in POOL_REGIONS:
        tiers = {tier: list(rows) for tier, rows in base.items()}
        for item, source, dest, weight in FLAVOUR_MOVES.get(region, []):
            tiers[source] = [(i, w) for i, w in tiers[source] if i != item]
            tiers[dest].append((item, weight))
        if region == 'Johto':
            tiers['Regular'] += [(item, APRICORN_WEIGHT) for item in apricorns]
        if region == 'Hoenn':
            tiers['Better'] += [(item, SHARD_WEIGHT) for item in shards]
        if region == 'Sevii':
            tiers['Better'] = [(i, w + SEVII_PEARL_BONUS if i == 'ITEM_PEARL' else w) for i, w in tiers['Better']]
        for tier, rows in tiers.items():
            names = [item for item, _ in rows]
            if len(set(names)) != len(names):
                raise GenerateError(f'{region} {tier} pool lists an item twice')
            for item in names:
                if item not in (GROUP_STONE, GROUP_TM) and item not in pockets:
                    raise GenerateError(f'{region} {tier} pool item {item} is not an item of the Wayfarer build')
        pools[region] = tiers
    stones = list(data['stones'])
    for item in stones:
        if item not in pockets:
            raise GenerateError(f'evolution stone {item} is not an item of the Wayfarer build')
    odds = data['tier_odds']
    if sorted(odds) != sorted(TIER_NAMES) or any(sum(row) != 100 for row in odds.values()):
        raise GenerateError('tier odds must list Road, Wilds and Outlands and sum to 100')
    legend_tms = sorted(set(tms) & legend_items)
    for tm in legend_tms:
        if tm in dynamic_tms:
            raise GenerateError(f'Legend TM {tm} is in the dynamic TM list')
    for item in dynamic_tms:
        if item not in pockets:
            raise GenerateError(f'TM {item} has no item data')
    return pools, stones, dynamic_tms


# ---------- output ----------
def c_tier(tier):
    return f'WORLD_ITEM_TIER_{tier.upper()}'


def c_region(region):
    return f'WORLD_ITEM_REGION_{region.upper()}'


def render_spots(rows):
    lines = ['// Generated by tools/wayfarer_item_slots/generate.py; edit the world items spec, data.json or fixed.json, then regenerate.',
             '// One row per kept item spot, ordered by map, then ball before hidden, then id: the row number is the spot\'s slot index.',
             'static const struct WorldItemSpot sWorldItemSpots[WORLD_ITEM_SPOT_COUNT] =', '{']
    for spot in rows:
        kind = 'WORLD_ITEM_SPOT_PRIZE' if spot.status == 'prize' else 'WORLD_ITEM_SPOT_DYNAMIC'
        shape = 'WORLD_ITEM_SPOT_HIDDEN' if spot.kind == 'hidden' else 'WORLD_ITEM_SPOT_BALL'
        lines.append(f'    {{ MAP_GROUP({spot.map_const}), MAP_NUM({spot.map_const}), {spot.id}, '
                     f'WORLD_ITEM_ATTRS({shape}, {kind}, {c_tier(spot.tier)}, {c_region(spot.region)}), {spot.prize or "ITEM_NONE"} }},'
                     f' // {spot.map_name} {spot.kind} {spot.x},{spot.y}')
    lines += ['};', '']
    return '\n'.join(lines)


def render_count(count):
    return ('// Generated by tools/wayfarer_item_slots/generate.py.\n'
            '#ifndef GUARD_CONSTANTS_WORLD_ITEM_SPOTS_H\n#define GUARD_CONSTANTS_WORLD_ITEM_SPOTS_H\n\n'
            f'#define WORLD_ITEM_SPOT_COUNT {count}\n\n#endif\n')


def pool_symbol(region, tier):
    return f'sWorldItemPool_{region}_{tier}'


def render_pools(pools, stones, dynamic_tms, odds, empty_percent):
    lines = ['// Generated by tools/wayfarer_item_slots/generate.py; edit the world items spec or data.json, then regenerate.',
             f'#define WORLD_ITEM_EMPTY_PERCENT {empty_percent}', '',
             '// Cumulative tier odds per reach tier: Regular below [0], Better below [1], Special otherwise.',
             'static const u8 sWorldItemTierOdds[WORLD_ITEM_TIER_COUNT][2] =', '{']
    for tier in TIER_NAMES:
        regular, better, _ = odds[tier]
        lines.append(f'    [{c_tier(tier)}] = {{ {regular}, {regular + better} }},')
    lines += ['};', '',
              'static const u16 sWorldItemStones[] =', '{'] + [f'    {item},' for item in stones] + ['};', '',
              'static const u16 sWorldItemDynamicTms[] =', '{'] + [f'    {item},' for item in dynamic_tms] + ['};', '']
    for region in POOL_REGIONS:
        for tier in POOL_TIERS:
            lines += [f'static const struct WorldItemPoolEntry {pool_symbol(region, tier)}[] =', '{']
            for item, weight in pools[region][tier]:
                symbol = {GROUP_STONE: 'WORLD_ITEM_GROUP_STONE', GROUP_TM: 'WORLD_ITEM_GROUP_TM'}.get(item, item)
                lines.append(f'    {{ {symbol}, {weight} }},')
            lines += ['};', '']
    lines += ['static const struct WorldItemPool sWorldItemPools[WORLD_ITEM_REGION_COUNT][3] =', '{']
    for region in POOL_REGIONS:
        lines.append(f'    [{c_region(region)}] =')
        lines.append('    {')
        for tier in POOL_TIERS:
            total = sum(weight for _, weight in pools[region][tier])
            symbol = pool_symbol(region, tier)
            lines.append(f'        {{ {symbol}, ARRAY_COUNT({symbol}), {total} }},')
        lines.append('    },')
    lines += ['};', '']
    return '\n'.join(lines)


def pct(value):
    return f'{value:.2f}%'


def render_report(rows, all_spots, data, pools, stones, dynamic_tms, disagreements, fixed_rows):
    lines = ['# World items report', '',
             'Generated by `tools/wayfarer_item_slots/generate.py`; the tables are `spots.h` and `pools.h`. Slot status by',
             'region with the reason for each, then the pool weights per reach and region. See the world items and daily',
             'world slots specs.', '']
    kept = [s for s in all_spots if s.status in ('prize', 'item')]
    lines += ['## Totals by region', '',
              '| Region | Balls | Hidden | Prizes (Find / Treasure / Legend) | Dynamic | Fixed (excluded) | Spec prizes | Spec dynamic |',
              '| --- | ---: | ---: | --- | ---: | ---: | ---: | --- |']
    spec_dynamic = {'Kanto': '128', 'Sevii': '64', 'Johto': 'about 81', 'Alola': 'about 6', 'Sinjoh': 'about 13', 'Hoenn': 'about 280'}
    for region in POOL_REGIONS:
        mine = [s for s in all_spots if (s.region == region if s.status in ('prize', 'item') else False)]
        prizes = [s for s in mine if s.status == 'prize']
        tiers = Counter(s.prize_tier for s in prizes)
        dyn = [s for s in mine if s.status == 'item']
        fixed_here = [s for s in fixed_rows if s.region == region]
        if not mine and not fixed_here:
            continue
        spec_total = data['spec_totals'].get(region, {}).get('total', '-')
        lines.append(f"| {region} | {sum(1 for s in mine if s.kind == 'ball')} | {sum(1 for s in mine if s.kind == 'hidden')} | "
                     f"{len(prizes)} ({tiers['Find']} / {tiers['Treasure']} / {tiers['Legend']}) | {len(dyn)} | {len(fixed_here)} | {spec_total} | {spec_dynamic.get(region, '-')} |")
    lines += [f'| total | {sum(1 for s in kept if s.kind == "ball")} | {sum(1 for s in kept if s.kind == "hidden")} | '
              f"{sum(1 for s in kept if s.status == 'prize')} | {sum(1 for s in kept if s.status == 'item')} | {len(fixed_rows)} | {data['spec_overall']} | |", '']
    other_fixed = [s for s in all_spots if s.status in ('fixed', 'empty') and s not in fixed_rows]
    lines += [f'{len(kept)} kept spots (the save\'s slot indices), {len(fixed_rows)} fixed story balls, {len(other_fixed)} other excluded rows '
              '(Battle Pyramid, contest halls and empty hidden rows).', '']
    lines += ['## Reach tiers', '', '| Region | Road | Wilds | Outlands |', '| --- | ---: | ---: | ---: |']
    for region in POOL_REGIONS:
        mine = [s for s in kept if s.region == region]
        if mine:
            counts = Counter(s.tier for s in mine)
            lines.append(f'| {region} | {counts["Road"]} | {counts["Wilds"]} | {counts["Outlands"]} |')
    lines += ['']
    if disagreements:
        lines += ['## Prize reach notes', '',
                  'Prize rows whose Reach column in the spec names a different tier than the shared resolution gives. The resolution',
                  'wins; these are for review, not failures.', '', '| Prize | Spec says | Resolved | Resolved by |', '| --- | --- | --- | --- |']
        for spot, text, said in disagreements:
            lines.append(f'| {spot.label} ({spot.prize}) | {text} | {spot.tier} | {spot.reach_detail} |')
        lines += ['']
    if NOTES:
        lines += ['## Spec notes', '', 'Places where the world items spec and the compiled maps disagree; the compiled map wins.', '']
        lines += [f'- {note}' for note in sorted(set(NOTES))] + ['']
    lines += ['## Dynamic pools', '',
              f"A dynamic find is empty {data['empty_percent']}% of days; otherwise it rolls a tier from the spot's reach, then an item by weight.",
              '', '| Reach | Regular | Better | Special |', '| --- | ---: | ---: | ---: |']
    for tier in TIER_NAMES:
        lines.append(f"| {tier} | {data['tier_odds'][tier][0]}% | {data['tier_odds'][tier][1]}% | {data['tier_odds'][tier][2]}% |")
    lines += ['', f'Evolution stone group: {", ".join(item.removeprefix("ITEM_") for item in stones)} (equal odds). '
              f'TM group: {len(dynamic_tms)} TMs (every TM except Legend prizes), equal odds.', '']
    for region in POOL_REGIONS:
        lines += [f'### {region} pool weights', '']
        for tier in POOL_TIERS:
            total = sum(weight for _, weight in pools[region][tier])
            lines.append(f'**{tier}** (total weight {total}): ' + ', '.join(
                f'{("Evolution stone" if item == GROUP_STONE else "TM" if item == GROUP_TM else item.removeprefix("ITEM_"))} {weight}'
                for item, weight in pools[region][tier]))
            lines.append('')
        lines += ['Chance of a find per spot by reach (empty days included):', '', '| Reach | Empty | Regular | Better | Special |', '| --- | ---: | ---: | ---: | ---: |']
        for tier in TIER_NAMES:
            live = 1 - data['empty_percent'] / 100
            odds = data['tier_odds'][tier]
            lines.append(f"| {tier} | {pct(data['empty_percent'])} | " + ' | '.join(pct(100 * live * odds[i] / 100) for i in range(3)) + ' |')
        lines.append('')
    lines += ['## Spots', '', 'Every item spot by region. Status is prize (kept flag, gives its prize once), dynamic (daily pool) or fixed (never touched).', '']
    for region in POOL_REGIONS:
        mine = [s for s in rows if s.region == region]
        fixed_here = [s for s in fixed_rows if s.region == region]
        if not mine and not fixed_here:
            continue
        lines += [f'### {region}', '', '| # | Spot | Authored item | Status | Reach | Reason |', '| ---: | --- | --- | --- | --- | --- |']
        for spot in mine:
            status = 'prize' if spot.status == 'prize' else 'dynamic'
            reason = spot.reason if spot.status == 'prize' else spot.reach_detail
            tier = f'{spot.tier}' + (f" -> {spot.prize}" if spot.prize else '')
            lines.append(f'| {spot.index} | {spot.label} | {spot.authored} | {status} | {tier} | {reason} |')
        for spot in fixed_here:
            lines.append(f'| - | {spot.label} | {spot.authored or "-"} | fixed | - | {spot.reason} |')
        lines.append('')
    others = [s for s in other_fixed]
    lines += ['## Other excluded rows', '', 'Rows in maps outside the daily slots: Battle Pyramid and contest halls, and hidden rows with no item.', '']
    summary = Counter((s.reason) for s in others)
    for reason, count in sorted(summary.items()):
        lines.append(f'- {count} x {reason}')
    return '\n'.join(lines) + '\n'


# ---------- driver ----------
def build(root=ROOT):
    data = json.loads(DATA.read_text())
    fixed = json.loads(FIXED.read_text())
    errors = []
    maps = R.load_maps(root)
    labels = R.label_index(root)
    pockets = item_symbols()
    spots = C.collect(maps, labels, root)
    classify(spots, fixed, pockets, errors)
    # Fixed story balls get a region for the report; Battle Pyramid and contest hall balls stay out of it.
    region_and_reach(spots, errors, maps, root)
    legends = apply_prizes(spots, data, pockets, C.sevii_rows(root), errors)
    for spot in spots:
        if spot.status == 'fixed' and spot.region is None and not any(spot.map_name.startswith(r['prefix']) for r in fixed['map_prefixes']):
            spot.region = C.region_of(spot.map_name, root)
    if errors:
        raise GenerateError('\n'.join(sorted(set(errors))))
    pools, stones, dynamic_tms = pool_tables(data, pockets, legends)
    rows = sorted((s for s in spots if s.status in ('prize', 'item')), key=lambda s: s.key)
    for index, spot in enumerate(rows):
        spot.index = index
    fixed_rows = [s for s in spots if s.status == 'fixed' and s.region is not None]
    return {'data': data, 'spots': spots, 'rows': rows, 'pools': pools, 'stones': stones, 'tms': dynamic_tms, 'fixed_rows': fixed_rows,
            'disagreements': reach_disagreements(spots, data)}


def outputs(state):
    data = state['data']
    return {
        'spots.h': render_spots(state['rows']),
        '@count': render_count(len(state['rows'])),
        'pools.h': render_pools(state['pools'], state['stones'], state['tms'], data['tier_odds'], data['empty_percent']),
        'report.md': render_report(state['rows'], state['spots'], data, state['pools'], state['stones'], state['tms'],
                                   state['disagreements'], state['fixed_rows']),
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
    rows = state['rows']
    print(json.dumps({'spots': len(rows), 'prizes': sum(1 for s in rows if s.status == 'prize'),
                      'dynamic': sum(1 for s in rows if s.status == 'item'), 'fixed': len(state['fixed_rows'])}, sort_keys=True))


if __name__ == '__main__':
    main()
