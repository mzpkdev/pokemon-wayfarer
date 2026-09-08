#!/usr/bin/env python3
"""Validate Surf source aliases, generated pixels, and optional linked symbols."""
import argparse
from collections import Counter
from pathlib import Path
import re
import sys

GAME = Path(__file__).resolve().parents[3]
DATA = Path('src/data/object_events/surfable')
ASSETS = Path('graphics/object_events/pics/pokemon/surfable')
ROW = re.compile(r'SURF_PIC_ALIAS\((\w+),\s*"([^"\n]+)",\s*"([^"\n]+)"\)')
DEFINITION = re.compile(r'const u32 gSurfable(Shiny)?PokemonPic_(\w+)\[\]\s*=\s*INCBIN_U32\("([^"\n]+)"\);')
ALIAS = '''#define SURF_PIC_ALIAS(Name, NormalStem, ShinyStem) \\
    extern const u32 gSurfableShinyPokemonPic_##Name[ARRAY_COUNT(gSurfablePokemonPic_##Name)] \\
        __attribute__((alias("gSurfablePokemonPic_" #Name)));
#include "surfable_pokemon_pic_aliases.h"
#undef SURF_PIC_ALIAS'''


def require(condition, message):
    if not condition:
        raise ValueError(message)


def uncomment(text):
    return re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)


def manifest(game):
    path = game / DATA / 'surfable_pokemon_pic_aliases.h'
    text = uncomment(path.read_text())
    rows = ROW.findall(text)
    require(not ROW.sub('', text).strip(), f'{path}: unexpected manifest syntax')
    require(len(rows) == 61, f'{path}: expected 61 rows, got {len(rows)}')
    for column, label in ((0, 'names'), (1, 'normal paths'), (2, 'shiny paths')):
        duplicates = [key for key, count in Counter(row[column] for row in rows).items() if count > 1]
        require(not duplicates, f'{path}: duplicate {label}: {duplicates}')
    return rows


def validate(game):
    rows = manifest(game)
    path = game / DATA / 'surfable_pokemon_graphics.h'
    source = uncomment(path.read_text())
    require(source.count(ALIAS) == 1, f'{path}: expected one strong, sized manifest alias expansion')
    remainder = source.replace(ALIAS, '')
    require('SURF_PIC_ALIAS' not in remainder and 'alias(' not in remainder,
            f'{path}: alias outside the reviewed manifest expansion')
    normal, shiny = {}, {}
    for is_shiny, name, asset in DEFINITION.findall(source):
        target = shiny if is_shiny else normal
        require(name not in target, f'{path}: duplicate pixel definition for {name}')
        target[name] = asset
    # No alternate declarations, pointers, or macro substitutions may hide pixels.
    residue = DEFINITION.sub('', remainder)
    require(not re.search(r'gSurfable(?:Shiny)?PokemonPic_', residue),
            f'{path}: unrecognized pixel definition or substitution')
    aliases = {row[0] for row in rows}
    require(len(normal) == 139, f'{path}: expected 139 active normal definitions, got {len(normal)}')
    require(len(shiny) == 78 and set(shiny) == set(normal) - aliases,
            f'{path}: expected 78 independent shiny definitions; missing/excess: '
            f'{sorted(set(shiny) ^ (set(normal) - aliases))}; aliased INCBINs: {sorted(set(shiny) & aliases)}')
    tables = uncomment((game / DATA / 'surfable_pokemon_pic_tables.h').read_text())
    for prefix in ('gSurfablePokemonPic_', 'gSurfableShinyPokemonPic_'):
        names = set(re.findall(prefix + r'(\w+)', tables))
        require(names == set(normal), f'picture tables: {prefix} inventory mismatch: {sorted(names ^ set(normal))}')
    sizes = []
    for name, normal_stem, shiny_stem in rows:
        paths = [ASSETS / (stem + '.4bpp') for stem in (normal_stem, shiny_stem)]
        context = f'{name}: {paths[0]} / {paths[1]}'
        require(normal.get(name) == str(paths[0]), f'{context}: not the active normal definition')
        require(shiny_stem == normal_stem + '_shiny', f'{context}: unexpected shiny source path')
        blobs = []
        for asset in paths:
            require((game / asset).is_file(), f'{context}: missing {asset}; regenerate with Make')
            blobs.append((game / asset).read_bytes())
        require(blobs[0] == blobs[1], f'{context}: generated pixels diverge ({len(blobs[0])}/{len(blobs[1])} bytes)')
        expected = 24576 if name in {'Lugia', 'Rayquaza', 'Arceus'} else 6144
        require(len(blobs[0]) == expected, f'{context}: expected {expected} bytes, got {len(blobs[0])}')
        sizes.append(len(blobs[0]))
    require(Counter(sizes) == {6144: 58, 24576: 3} and sum(sizes) == 430080,
            'approved payload must be 58 x 6144 + 3 x 24576 = 430080 bytes')
    return normal, aliases


def linked(symbols_path, normal, aliases):
    symbols = {}
    for line in symbols_path.read_text().splitlines():
        fields = line.split()
        if len(fields) == 4 and fields[3].startswith(('gSurfablePokemonPic_', 'gSurfableShinyPokemonPic_')):
            address, binding, size, name = fields
            require(name not in symbols, f'{symbols_path}: duplicate symbol {name}')
            # Release LTO may internalize a strong definition without making it weak.
            require(binding in {'g', 'l'}, f'{name}: expected a strong global or local symbol, got {binding}')
            symbols[name] = (int(address, 16), int(size, 16))
    expected_names = {prefix + name for name in normal for prefix in ('gSurfablePokemonPic_', 'gSurfableShinyPokemonPic_')}
    require(set(symbols) == expected_names, f'{symbols_path}: missing or extra linked symbols: {sorted(set(symbols) ^ expected_names)}')
    for name in normal:
        a = symbols['gSurfablePokemonPic_' + name]
        b = symbols['gSurfableShinyPokemonPic_' + name]
        require(a[1] > 0 and b[1] > 0, f'{name}: zero-sized pixel symbol')
        if name in aliases:
            expected = 24576 if name in {'Lugia', 'Rayquaza', 'Arceus'} else 6144
            require(a == b and a[1] == expected, f'{name}: alias address/size mismatch: {a} / {b}')
        else:
            require(a[0] != b[0], f'{name}: non-approved pixel symbols share an address')
    # Distinct species must never share or overlap a linked pixel allocation.
    ranges = sorted((address, address + size, name) for name, (address, size) in symbols.items()
                    if not (name.startswith('gSurfableShinyPokemonPic_') and name.removeprefix('gSurfableShinyPokemonPic_') in aliases))
    for previous, current in zip(ranges, ranges[1:]):
        require(previous[1] <= current[0], f'linked pixel ranges overlap: {previous} / {current}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', type=Path, default=GAME)
    parser.add_argument('--symbols', type=Path, help='optimized make syms output')
    parser.add_argument('--list-assets', action='store_true', help='print Make prerequisites')
    args = parser.parse_args()
    try:
        if args.list_assets:
            print(' '.join(str(ASSETS / (stem + '.4bpp')) for row in manifest(args.game) for stem in row[1:]))
            return
        normal, aliases = validate(args.game)
        if args.symbols:
            linked(args.symbols, normal, aliases)
        print('Surf pixel aliases: 139 active pairs, 61 aliases, 78 independent shiny sheets; 430080 bytes' + ('; linked symbols verified' if args.symbols else ''))
    except (ValueError, OSError) as error:
        print(f'Surf pixel alias validation failed: {error}', file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
