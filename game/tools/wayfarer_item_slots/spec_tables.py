#!/usr/bin/env python3
"""Read the reviewed tables of the world items spec into the data the generator uses.

`python3 tools/wayfarer_item_slots/spec_tables.py --write` regenerates data.json from
.product/specs/world-items.md after a spec edit; the unit tests fail while the two differ,
so the spec and the data the generator reads cannot drift apart.
"""
from __future__ import annotations

import json
from pathlib import Path
import re
import sys

TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[1]
SPEC = ROOT.parent / '.product/specs/world-items.md'
DATA = TOOL / 'data.json'
REGIONS = ('Kanto', 'Sevii', 'Johto', 'Alola', 'Sinjoh', 'Hoenn')
TIERS = ('Find', 'Treasure', 'Legend')
POOL_TIERS = ('Regular', 'Better', 'Special')
REACHES = ('Road', 'Wilds', 'Outlands')


class SpecError(ValueError):
    pass


def split_row(line):
    return [cell.strip() for cell in line.strip().strip('|').split('|')]


def tables_under(lines, start, end):
    """Markdown tables between two line numbers: [(header cells, [row cells])]."""
    tables, index = [], start
    while index < end:
        if lines[index].startswith('|') and index + 1 < end and re.match(r'^\|[\s:|-]+\|$', lines[index + 1]):
            header, rows = split_row(lines[index]), []
            index += 2
            while index < end and lines[index].startswith('|'):
                rows.append(split_row(lines[index]))
                index += 1
            tables.append((header, rows))
        else:
            index += 1
    return tables


def sections(lines):
    """[(level, title, first line, end line)] for every heading."""
    heads = [(len(m.group(1)), m.group(2).strip(), i) for i, line in enumerate(lines) if (m := re.match(r'^(#{1,6}) (.*)$', line))]
    result = []
    for position, (level, title, line) in enumerate(heads):
        end = len(lines)
        for later_level, _, later in heads[position + 1:]:
            if later_level <= level:
                end = later
                break
        result.append((level, title, line, end))
    return result


def unquote(cell):
    return cell.strip('`')


def parse_spot(cell):
    """'local id 3 (19,18)' or 'hidden 5,12', optionally '; moved from <map> <spot>'."""
    moved = None
    if '; moved from ' in cell:
        cell, origin = cell.split('; moved from ', 1)
        match = re.match(r'(\w+) (.*)$', origin)
        moved = {'map': match.group(1), 'spot': parse_spot(match.group(2))}
    ball = re.fullmatch(r'local id (\d+) \((\d+),(\d+)\)', cell)
    hidden = re.fullmatch(r'hidden (\d+),(\d+)', cell)
    if ball:
        spot = {'kind': 'ball', 'local_id': int(ball.group(1)), 'x': int(ball.group(2)), 'y': int(ball.group(3))}
    elif hidden:
        spot = {'kind': 'hidden', 'x': int(hidden.group(1)), 'y': int(hidden.group(2))}
    else:
        raise SpecError(f'unreadable spot: {cell!r}')
    if moved:
        spot['moved_from'] = moved
    return spot


def parse_prizes(lines):
    heads = sections(lines)
    prizes = []
    for level, title, start, end in heads:
        if level != 4:
            continue
        region = next(t for lv, t, s, e in heads if lv == 3 and s < start < e)
        if region not in REGIONS:
            continue
        for header, rows in tables_under(lines, start, end):
            if header[:3] != ['#', 'Map', 'Spot']:
                continue
            for row in rows:
                number, map_name, spot, shown, reach, item, tier = row[:7]
                if tier not in TIERS:
                    raise SpecError(f'{region} {title} #{number}: unknown tier {tier}')
                entry = {'region': region, 'trail': title, 'n': int(number), 'map': map_name, 'shown': shown.split()[0],
                         'item': unquote(item), 'tier': tier, 'spec_reach': reach}
                entry.update(parse_spot(spot))
                prizes.append(entry)
    return prizes


def parse_totals(text):
    match = re.search(r'^Totals: (.*?)\. Overall (\d+) prizes\.', text, re.M | re.S)
    totals = {}
    for region, count, find, treasure, legend in re.findall(r'(\w+) (\d+) \((\d+) Find, (\d+) Treasure, (\d+) Legend\)', match.group(1)):
        totals[region] = {'total': int(count), 'Find': int(find), 'Treasure': int(treasure), 'Legend': int(legend)}
    return totals, int(match.group(2))


def parse_pools(lines):
    heads = sections(lines)
    pools = {}
    for level, title, start, end in heads:
        if title in POOL_TIERS:
            (_, rows), = tables_under(lines, start, end)
            pools[title] = []
            for item, weight in rows:
                pools[title].append({'item': unquote(item) if item.startswith('`') else item, 'weight': int(weight)})
    odds = {}
    for level, title, start, end in heads:
        if title == 'Tier odds':
            (_, rows), = tables_under(lines, start, end)
            for reach, *values in rows:
                key = 'Road' if reach.startswith('Road') else 'Wilds' if reach.startswith('Wilds') else 'Outlands'
                odds[key] = [int(v.rstrip('%')) for v in values]
    return pools, odds


def parse_stones_and_flavour(text):
    stones = re.findall(r'`(ITEM_\w+_STONE)`', text[text.index('- **Evolution stone:**'):text.index('- **TM:**')])
    return stones


def parse_marts(text):
    paragraph = text[text.index('The mart-sold list is:'):]
    names = paragraph.split(':', 1)[1].split('.', 1)[0]
    return ['ITEM_TM_' + re.sub(r"[^A-Z0-9]+", '_', name.strip().upper()) for name in names.replace('\n', ' ').split(',')]


def parse_all(text):
    lines = text.splitlines()
    pools, odds = parse_pools(lines)
    prizes = parse_prizes(lines)
    totals, overall = parse_totals(text)
    return {'version': 1, 'empty_percent': 25, 'tier_odds': odds, 'pools': pools,
            'stones': parse_stones_and_flavour(text), 'mart_tms': parse_marts(text),
            'spec_totals': totals, 'spec_overall': overall, 'prizes': prizes}


def main():
    data = parse_all(SPEC.read_text(encoding='utf-8'))
    text = json.dumps(data, indent=1) + '\n'
    if '--write' in sys.argv:
        DATA.write_text(text)
    else:
        sys.exit(0 if DATA.read_text() == text else 'data.json differs from the spec: run spec_tables.py --write')


if __name__ == '__main__':
    main()
