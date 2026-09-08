#!/usr/bin/env python3
"""Compare preserved baseline/optimized Wayfarer release artifacts, never ROM lengths."""
import argparse
import json
import importlib.util
from pathlib import Path
import re
import struct

SAVINGS = 430080
PREFIX = 'pokewayfarer-release'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def load(directory):
    report = json.loads((directory / f'{PREFIX}-size.json').read_text())
    symbols = {}
    for line in (directory / f'{PREFIX}.sym').read_text().splitlines():
        fields = line.split()
        if len(fields) == 4:
            address, binding, size, name = fields
            symbols[name] = (int(address, 16), int(size, 16))
    map_text = (directory / f'{PREFIX}.map').read_text()
    end = symbols['__rom_end'][0]
    require(any(int(value, 16) == end for value in re.findall(r'(0x[0-9a-fA-F]+)\s+__rom_end\s*=', map_text)),
            f'{directory}: map __rom_end disagrees with symbols')
    require(report['build'] == 'wayfarer' and report['release'], f'{directory}: not a Wayfarer release')
    require(report['baseline']['available'], f'{directory}: accepted baseline missing')
    require(report['rom']['wayfarer_release_limit_enforced'], f'{directory}: release reserve not enforced')
    require(end == int(report['rom']['end_address'], 16), f'{directory}: report __rom_end mismatch')
    require(end <= int(report['rom']['wayfarer_release_limit_address'], 16), f'{directory}: reserve exceeded')
    return report, symbols, (directory / f'{PREFIX}.gba').read_bytes(), map_text


def compare(baseline, optimized):
    spec = importlib.util.spec_from_file_location('surf_aliases', Path(__file__).parent / 'tests/test_surfable_pokemon_pic_aliases.py')
    validator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(validator)
    normal, aliases = validator.validate(optimized)
    validator.linked(optimized / f'{PREFIX}.sym', normal, aliases)
    before, bs, br, bm = load(baseline)
    after, os, ore, om = load(optimized)
    for name in normal:
        require(bs['gSurfablePokemonPic_' + name][0] != bs['gSurfableShinyPokemonPic_' + name][0],
                f'{name}: baseline already aliases the pair')
    for field in ('used_bytes', 'unused_bytes', 'end_address'):
        a, b = before['rom'][field], after['rom'][field]
        if field == 'end_address':
            a, b = int(a, 16), int(b, 16)
        delta = b - a if field == 'unused_bytes' else a - b
        require(delta == SAVINGS, f'rom.{field}: expected {SAVINGS}, got {delta}')
    require(bs['__rom_end'][0] - os['__rom_end'][0] == SAVINGS, '__rom_end delta mismatch')
    require(set(before['categories']) == set(after['categories']), 'category inventory changed')
    for category in before['categories']:
        delta = before['categories'][category]['bytes'] - after['categories'][category]['bytes']
        require(delta == (SAVINGS if category == 'other' else 0), f'{category}: unexpected delta {delta}')
    # Compare every linked Surf pixel and palette byte at its relocated address.
    names = {name for name in bs if name.startswith(('gSurfablePokemonPic_', 'gSurfableShinyPokemonPic_',
                                                    'gSurfablePokemonPalette_', 'gSurfablePokemonShinyPalette_',
                                                    'gSurfablePokemonShinyModernPalette_'))}
    require(names == {name for name in os if name.startswith(('gSurfablePokemonPic_', 'gSurfableShinyPokemonPic_',
                                                             'gSurfablePokemonPalette_', 'gSurfablePokemonShinyPalette_',
                                                             'gSurfablePokemonShinyModernPalette_'))}, 'Surf pixel/palette symbol inventory changed')
    for name in names:
        ba, size = bs[name]
        oa, other_size = os[name]
        require(size == other_size and size > 0, f'{name}: size changed')
        require(br[ba - 0x08000000:ba - 0x08000000 + size] == ore[oa - 0x08000000:oa - 0x08000000 + size],
                f'{name}: linked bytes changed')
    palette_names = sorted(name for name in names if 'Palette_' in name)
    require(len({bs[name][0] for name in palette_names}) == len(palette_names), 'baseline palettes unexpectedly share addresses')
    require(len({os[name][0] for name in palette_names}) == len(palette_names), 'optimized palettes share addresses')
    table_names = {name for name in bs if re.fullmatch(r'gSurfing(?:Overworld|Overlay)(?:Shiny)?PicTable_\w+', name)}
    require(table_names == {name for name in os if re.fullmatch(r'gSurfing(?:Overworld|Overlay)(?:Shiny)?PicTable_\w+', name)},
            'frame-table inventory changed')
    for name in table_names:
        ba, size = bs[name]
        oa, other_size = os[name]
        require(size == other_size and size % 8 == 0, f'{name}: frame count changed')
        pixel_name = ('gSurfableShinyPokemonPic_' if 'Shiny' in name else 'gSurfablePokemonPic_') + name.split('_', 1)[1]
        for offset in range(0, size, 8):
            bp, bd = struct.unpack_from('<II', br, ba - 0x08000000 + offset)
            op, od = struct.unpack_from('<II', ore, oa - 0x08000000 + offset)
            require(bd == od and bp - bs[pixel_name][0] == op - os[pixel_name][0],
                    f'{name}: frame {offset // 8} dimensions/offset changed')
    return {'savings_bytes': SAVINGS, 'baseline_rom': before['rom'], 'optimized_rom': after['rom'],
            'unchanged_pixel_palette_symbols': len(names), 'separate_palettes': len(palette_names),
            'unchanged_frame_tables': len(table_names)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('baseline', type=Path)
    parser.add_argument('optimized', type=Path)
    args = parser.parse_args()
    try:
        print(json.dumps(compare(args.baseline, args.optimized), indent=2))
    except (ValueError, KeyError, OSError) as error:
        parser.exit(1, f'{error}\n')


if __name__ == '__main__':
    main()
