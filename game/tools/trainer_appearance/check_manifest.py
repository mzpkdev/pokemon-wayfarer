#!/usr/bin/env python3
"""Check every registered avatar's animation/frame references against its PNG sources.

--write refreshes the checked inventory, including failures. Missing action art is
an acceptance failure, never silently substituted with another character.
"""
import argparse
import json
import re
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DATA = ROOT / 'src/data/object_events'
MANIFEST = Path(__file__).with_name('manifest.json')


def arrays(path):
    return dict(re.findall(r'\b(\w+)\[\]\s*=\s*\{(.*?)\n\};', path.read_text(), re.S))


def png_size(path):
    data = path.read_bytes()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError(f'{path}: not PNG')
    return struct.unpack('>II', data[16:24])


def selected(expression):
    expression = expression.strip('() ')
    match = re.fullmatch(r'IS_(WAYFARER|FRLG)\s*\?\s*(\w+)\s*:\s*(\w+)', expression)
    return match[2 if match[1] == 'WAYFARER' else 3] if match else expression


def inventory():
    profiles = (ROOT / 'src/wayfarer_appearance.c').read_text()
    pointers = dict(re.findall(r'\[(OBJ_EVENT_GFX_\w+)\]\s*=\s*&?(\w+)', (DATA / 'object_event_graphics_info_pointers.h').read_text()))
    infos = dict(re.findall(r'const struct ObjectEventGraphicsInfo (\w+)\s*=\s*\{([^}]*)\};', (DATA / 'object_event_graphics_info.h').read_text(), re.S))
    pics = arrays(DATA / 'object_event_pic_tables.h')
    animations = arrays(DATA / 'object_event_anims.h')
    sources = dict(re.findall(r'\b(\w+)\[\]\s*=\s*INCBIN_U\d+\((.*?)\);', (DATA / 'object_event_graphics.h').read_text()))
    expected = {'NORMAL': {'sAnimTable_BrendanMayNormal', 'sAnimTable_RedGreenNormal'},
                'MACH_BIKE': {'sAnimTable_Standard'}, 'ACRO_BIKE': {'sAnimTable_AcroBike'},
                'SURFING': {'sAnimTable_Surfing'}, 'UNDERWATER': {'sAnimTable_Standard'},
                'FIELD_MOVE': {'sAnimTable_FieldMove'}, 'FISHING': {'sAnimTable_Fishing'},
                'WATERING': {'sAnimTable_Standard'},
                'VSSEEKER': {'sAnimTable_FieldMove', 'sAnimTable_RedGreenVSSeeker'}}
    movement = (ROOT / 'src/event_object_movement.c').read_text()
    palettes = dict((tag, symbol) for symbol, tag in re.findall(r'\{(gObjectEventPal_\w+),\s*(OBJ_EVENT_PAL_TAG_\w+)\}', movement))
    reflection_sets = dict(re.findall(r'\{(OBJ_EVENT_PAL_TAG_\w+),\s*(sReflectionPaletteTags_\w+)\}', movement))
    reflection_arrays = arrays(ROOT / 'src/event_object_movement.c')
    directions = ['SOUTH', 'NORTH', 'WEST', 'EAST']
    standard = {f'ANIM_STD_{speed}_{direction}' for speed in ['FACE', 'GO', 'GO_FAST', 'GO_FASTER', 'GO_FASTEST'] for direction in directions}
    contracts = {
        'NORMAL': standard | {f'ANIM_{action}_{direction}' for action in ['RUN', 'SPIN'] for direction in directions},
        'MACH_BIKE': standard, 'UNDERWATER': standard, 'WATERING': standard,
        'ACRO_BIKE': standard | {f'ANIM_{action}_{direction}' for action in ['BUNNY_HOP_BACK_WHEEL', 'BUNNY_HOP_FRONT_WHEEL', 'STANDING_WHEELIE_BACK_WHEEL', 'STANDING_WHEELIE_FRONT_WHEEL', 'MOVING_WHEELIE'] for direction in directions},
        'SURFING': standard | {f'ANIM_GET_ON_OFF_POKEMON_{direction}' for direction in directions},
        'FIELD_MOVE': {'ANIM_FIELD_MOVE'},
        'FISHING': {f'ANIM_{action}_{direction}' for action in ['TAKE_OUT_ROD', 'PUT_AWAY_ROD', 'HOOKED_POKEMON'] for direction in directions},
    }
    records = []
    for appearance, body in re.findall(r'\.id = (APPEARANCE_\w+).*?\.graphicsIds = \{(.*?)\n        \}', profiles, re.S):
        states = re.findall(r'\[PLAYER_AVATAR_STATE_(\w+)\] = (OBJ_EVENT_GFX_\w+)', body)
        if len(states) != 9:
            raise ValueError(f'{appearance}: expected nine states')
        for state, graphics in states:
            row = {'appearance': appearance, 'state': state, 'graphics': graphics,
                   'art': 'authored' if graphics.endswith('_WAYFARER') else 'reused',
                   'visual_validation': 'not performed', 'errors': []}
            records.append(row)
            if graphics not in pointers or pointers[graphics] not in infos:
                row['errors'].append('Missing required graphics descriptor and action art')
                continue
            row['descriptor'] = pointers[graphics]
            fields = dict(re.findall(r'\.(\w+)\s*=\s*([^,]+)', infos[row['descriptor']]))
            row['palette'] = fields['paletteTag'].strip()
            row['palette_symbol'] = palettes.get(row['palette'])
            if row['palette_symbol'] is None:
                row['errors'].append('Missing registered palette')
            row['reflection_palette'] = fields['reflectionPaletteTag'].strip()
            reflection_set = reflection_sets.get(row['palette'])
            row['reflection_palette_symbols'] = []
            if row['reflection_palette'] != 'OBJ_EVENT_PAL_TAG_NONE':
                if reflection_set is None:
                    row['errors'].append('Missing character reflection palette mapping')
                else:
                    for tag in re.findall(r'OBJ_EVENT_PAL_TAG_\w+', reflection_arrays[reflection_set]):
                        symbol = palettes.get(tag)
                        if symbol is None:
                            row['errors'].append(f'Missing reflection palette {tag}')
                        row['reflection_palette_symbols'].append(symbol)
            row['frame_dimensions'] = [int(fields['width']), int(fields['height'])]
            row['animation_table'] = table = selected(fields['anims'])
            if state not in {'FIELD_MOVE', 'VSSEEKER'} and not re.search(r'\.anims\s*=\s*' + re.escape(table) + r'\b', animations['sStepAnimTables']):
                row['errors'].append('Missing alternating-step animation registration')
            if table not in expected[state]:
                row['errors'].append(f'Wrong animation contract for {state}: {table}')
            frame_list = []
            sheets = {}
            for kind, source, w, h, index in re.findall(r'(overworld_frame|overworld_ascending_frames)\((\w+), (\d+), (\d+)(?:, (\d+))?\)', pics[fields['images'].strip()]):
                paths = re.findall(r'"([^"]+)"', sources[source])
                count = 0
                for path in paths:
                    path = str(Path(path).with_suffix('.png'))
                    width, height = png_size(ROOT / path)
                    fw, fh = int(w) * 8, int(h) * 8
                    if width % fw or height % fh:
                        row['errors'].append(f'{path}: sheet dimensions not divisible by frame dimensions')
                    n = width * height // (fw * fh)
                    sheets[path] = {'dimensions': [width, height], 'frame_count': n, 'symbol': source}
                    count += n
                indices = range(count) if kind == 'overworld_ascending_frames' else [int(index)]
                for idx in indices:
                    if idx >= count:
                        row['errors'].append(f'{source}: frame {idx} outside {count}-frame sheet')
                    frame_list.append({'symbol': source, 'index': idx})
            row['sheets'] = sheets
            row['frames'] = frame_list
            row['frame_count'] = len(frame_list)
            row['animations'] = {}
            for slot, expression in re.findall(r'\[(\w+)\]\s*=\s*([^,\n]+)', animations[table]):
                anim = selected(expression)
                indices = [int(x) for x in re.findall(r'ANIMCMD_FRAME\((\d+)', animations[anim])]
                row['animations'][slot] = {'symbol': anim, 'frames': indices}
                if any(i >= len(frame_list) for i in indices):
                    row['errors'].append(f'{anim}: references {max(indices)} outside {len(frame_list)}-frame table')
            if not row['animations']:
                # Native FRLG tables use their single action at slot zero.
                for anim in re.findall(r'\b(sAnim_\w+)\b', animations[table]):
                    indices = [int(x) for x in re.findall(r'ANIMCMD_FRAME\((\d+)', animations[anim])]
                    row['animations']['0'] = {'symbol': anim, 'frames': indices}
                    if any(i >= len(frame_list) for i in indices):
                        row['errors'].append(f'{anim}: frame index outside table')
            required = contracts.get(state, {'0'} if table == 'sAnimTable_RedGreenVSSeeker' else {'ANIM_FIELD_MOVE'})
            missing = required - row['animations'].keys()
            if missing:
                row['errors'].append('Missing required animation slots: ' + ', '.join(sorted(missing)))
    if len(records) != 54:
        raise ValueError(f'Expected 54 style/state combinations, found {len(records)}')
    return {'format': 1, 'scope': 'Wayfarer; static frame validation is not visual acceptance', 'states': records}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--write', action='store_true')
    args = parser.parse_args()
    result = inventory()
    serialized = json.dumps(result, indent=2) + '\n'
    if args.write:
        MANIFEST.write_text(serialized)
    elif not MANIFEST.exists() or MANIFEST.read_text() != serialized:
        raise SystemExit('Manifest stale: run check_manifest.py --write and review changes')
    errors = [(r['appearance'], r['state'], e) for r in result['states'] for e in r['errors']]
    for appearance, state, error in errors:
        print(f'{appearance}/{state}: {error}')
    print(f'{len(result["states"])} combinations checked; {len(errors)} failures; emulator validation not performed')
    return bool(errors)


if __name__ == '__main__':
    raise SystemExit(main())
