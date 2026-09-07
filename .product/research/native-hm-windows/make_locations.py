#!/usr/bin/env python3
"""Render a compact world-location companion to the proposed utility roster."""
import csv
import json
import re
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
REGIONS = {'JOHTO': 'Johto', 'KANTO': 'Kanto', 'HOENN': 'Hoenn',
           'HNS_UNCLASSIFIED': 'Other HNS areas (access unverified)'}
METHODS = {'land_mons': 'land', 'water_mons': 'Surf', 'fishing_mons': 'fishing',
           'rock_smash_mons': 'interaction'}


def name(value):
    value = re.sub(r'^(SPECIES_|MOVE_|MAP_)', '', value).removesuffix('_HNS')
    value = re.sub(r'ROUTE(\d+)', r'ROUTE \1', value)
    value = value.replace('MAHOGANYTOWN', 'MAHOGANY_TOWN').replace('NEWSINJOH', 'NEW_SINJOH')
    value = value.replace('STEVENS', "STEVEN'S").replace('DRAGONS', "DRAGON'S")
    value = value.replace('_', ' ').title()
    return re.sub(r'\b(?:B\d+F|\d+F|\d+R)\b', lambda m: m[0].upper(), value, flags=re.I)


def flags(map_id, region):
    found = []
    if 'SAFARI' in map_id:
        found.append('Safari')
    if any(s in map_id for s in ('UNUSED', 'ALTERING', 'MIRAGE', 'BUG_CONTEST')):
        found.append('special/optional')
    if any(s in map_id for s in ('VICTORY_ROAD', 'MT_SILVER', 'CERULEAN_CAVE',
                                 'MAGMA_HIDEOUT', 'CAVE_OF_ORIGIN', 'UNDERWATER',
                                 'SKY_PILLAR', 'DESERT_UNDERPASS', 'STEVENS_CAVE', 'TIN_TOWER')):
        found.append('late/access-dependent')
    if region == 'HNS_UNCLASSIFIED':
        found.append('access unverified')
    return found


def priority(item, region):
    map_id, methods = item
    route = re.search(r'ROUTE(\d+)', map_id)
    return (bool(flags(map_id, region)),
            0 if set(methods) & {'land_mons', 'fishing_mons'} else 1,
            0 if route and not map_id.startswith('MAP_UNDERWATER') else
            1 if any(s in map_id for s in ('CITY', 'TOWN')) else 2,
            (int(route[1]) + (30 if region == 'JOHTO' and int(route[1]) < 29 else 0))
            if route else 999, map_id)


def locale(map_id):
    return re.split(r'_(?:B?\d+F|\d+R|STEVENS_ROOM|STEVENS_CAVE)', map_id)[0]


def describe(map_id, methods, region):
    labels = []
    for method, times in sorted(methods.items(), key=lambda p: list(METHODS).index(p[0])):
        if region == 'HOENN':
            when = ''
        elif times == {'TIME_DAY', 'TIME_NIGHT'}:
            when = ', day/night'
        else:
            when = ', ' + '/'.join(t.removeprefix('TIME_').lower() for t in sorted(times))
        method_name = 'underwater' if 'UNDERWATER' in map_id and method == 'water_mons' else METHODS[method]
        labels.append(method_name + when)
    note = flags(map_id, region)
    return name(map_id) + ' (' + '; '.join(labels) + ')' + (' [' + ', '.join(note) + ']' if note else '')


def main():
    roster = json.loads((HERE / 'roster.json').read_text())
    rows = list(csv.DictReader((HERE / 'locations.csv').open()))
    grouped = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(set))))
    for row in rows:
        grouped[row['species']][row['region']][row['map']][row['method']].add(row['time'])
    lines = [
        '# Where to catch the proposed utility Pokémon', '',
        f'This guide covers the {len(roster)} species in the [proposed roster](roster.md). '
        'Locations describe species presence at some Trainer Ratings, not possession of a utility move at every TR. '
        'Use [roster.csv](roster.csv) for the proposed move windows and regional TR coverage.', '',
        'Each region lists up to six representative locations, favoring routes, towns, land encounters, and fishing. '
        'The [complete location CSV](locations.csv) retains every modeled map ID, method, and time profile, including omitted floors and special areas. '
        'A table entry does not establish that the player can reach the map without the utility being sought.', '',
        'Land means grass or cave encounters. Surf requires access to water; fishing assumes a rod and a reachable shore. '
        'Interaction means the shared Rock Smash/Headbutt encounter table; the usable interaction depends on the map and has not been individually audited. '
        'Day/night refer to HNS time profiles; Hoenn uses static tables, so no time restriction is shown. '
        'Safari, optional, late, and other HNS areas are not guaranteed acquisition routes.', '',
    ]
    for entry in roster:
        species = entry['species']
        roles = ', '.join(name(move) for move in entry['roles'])
        lines.extend([f'## {name(species)} ({roles})', ''])
        if species in {'SPECIES_KIRLIA', 'SPECIES_HUNTAIL', 'SPECIES_GOREBYSS'}:
            lines.extend(['No ordinary wild encounter in the modeled tables. This is an evolution/reminder option; '
                          'do not count it as a direct wild catch for TR coverage.', ''])
            continue
        if species == 'SPECIES_CRAWDAUNT':
            lines.extend(['The only modeled wild placement is in a special HNS area. '
                          'Do not count it as ordinary Johto, Kanto, or Hoenn catch coverage.', ''])
        for region, label in REGIONS.items():
            maps = grouped[species].get(region, {})
            if not maps:
                continue
            ranked = sorted(maps.items(), key=lambda item: priority(item, region))
            selected, seen, route_count = [], set(), 0
            has_nonroute = any('ROUTE' not in map_id and not flags(map_id, region)
                               for map_id in maps)
            for item in ranked:
                if has_nonroute and 'ROUTE' in item[0] and route_count >= 4:
                    continue
                if locale(item[0]) not in seen:
                    selected.append(item)
                    seen.add(locale(item[0]))
                    route_count += 'ROUTE' in item[0]
                if len(selected) == 6:
                    break
            text = '; '.join(describe(map_id, methods, region) for map_id, methods in selected)
            omitted = len(maps) - len(selected)
            if omitted:
                text += f'. {omitted} more map/floor entries in the CSV'
            lines.extend([f'- {label}: {text}.'])
        if not grouped[species]:
            lines.append('No modeled ordinary encounter; availability requires separate verification.')
        lines.append('')
    (HERE / 'locations.md').write_text('\n'.join(lines))
    print(f'Wrote location guide for {len(roster)} species from {len(rows)} location rows.')


if __name__ == '__main__':
    main()
