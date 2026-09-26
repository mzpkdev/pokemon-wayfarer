"""FRLG Viridian Gym source selection and Wayfarer encounter contracts."""
from __future__ import annotations

import json
import re
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path

import generate as trainer_inventory

ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    'TAMER_JASON': 'JASON', 'TAMER_COLE': 'COLE',
    'BLACK_BELT_ATSUSHI': 'ATSUSHI', 'BLACK_BELT_KIYO': 'KIYO',
    'BLACK_BELT_TAKASHI': 'TAKASHI', 'COOLTRAINER_SAMUEL': 'SAMUEL',
    'COOLTRAINER_YUJI': 'YUJI', 'COOLTRAINER_WARREN': 'WARREN',
    'LEADER_GIOVANNI': 'GIOVANNI',
}


def party_block(source: str, trainer: str) -> str:
    return source.split(f'=== {trainer} ===', 1)[1].split('=== TRAINER_', 1)[0].strip()


class ViridianSourceContracts(unittest.TestCase):
    def test_wayfarer_compiles_donor_scripts_without_hns_label_collisions(self):
        table = (ROOT / 'data/event_scripts.s').read_text()
        include = '\t.include "data/maps/ViridianCity_Gym_Frlg/scripts.inc"'
        first = table.index(include)
        second = table.index(include, first + len(include))
        wayfarer_start = table.rfind('#if IS_WAYFARER', 0, first)
        self.assertGreater(wayfarer_start, 0)
        self.assertGreater(table.find('#endif', wayfarer_start), first)
        self.assertGreater(table.rfind('.if IS_FRLG', 0, second), first)

        labels = []
        for name in ('ViridianCity_Gym_Frlg', 'ViridianCity_Gym_hns'):
            path = ROOT / f'data/maps/{name}/scripts.inc'
            source = subprocess.run(
                ['cpp', '-traditional-cpp', '-P', '-DIS_WAYFARER=1', '-DIS_FRLG=0', str(path)],
                check=True, text=True, capture_output=True).stdout
            labels.append(set(re.findall(r'^([A-Za-z][A-Za-z0-9_]*):{1,2}\s*$', source, re.M)))
        self.assertFalse(labels[0] & labels[1])

    def test_nine_selected_parties_match_frlg_and_have_distinct_policies(self):
        donor = (ROOT / 'src/data/trainers_frlg.party').read_text()
        selected = (ROOT / 'src/data/trainers_wayfarer_local.party').read_text()
        records = {row['id']: row for row in json.loads(trainer_inventory.MANIFEST.read_text())['records']}
        ids = trainer_inventory.trainer_ids(records)
        selected_ids = []
        for original, name in SOURCES.items():
            runtime_id = f'TRAINER_VIRIDIAN_GYM_{name}_HNS'
            selected_ids.append(ids[runtime_id])
            self.assertEqual(party_block(selected, runtime_id),
                             party_block(donor, f'TRAINER_{original}'))
            self.assertEqual(records[runtime_id]['policy'],
                             'GYM_LEADER' if name == 'GIOVANNI' else 'GYM_MEMBER')
        self.assertEqual(sorted(selected_ids), list(range(1800, 1809)))
        giovanni = party_block(selected, 'TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS')
        self.assertEqual([line for line in giovanni.splitlines()
                          if line in {'Rhyhorn', 'Dugtrio', 'Nidoqueen', 'Nidoking'}],
                         ['Rhyhorn', 'Dugtrio', 'Nidoqueen', 'Nidoking', 'Rhyhorn'])
        self.assertEqual([line for line in giovanni.splitlines() if line.startswith('Level: ')],
                         ['Level: 45', 'Level: 42', 'Level: 44', 'Level: 45', 'Level: 50'])

    def test_complete_frlg_map_and_return_warps_are_selected(self):
        gym = json.loads((ROOT / 'data/maps/ViridianCity_Gym_Frlg/map.json').read_text())
        layout = json.loads((ROOT / 'data/layouts/layouts.json').read_text())
        self.assertTrue(gym['wayfarer_include'])
        self.assertEqual(gym['layout'], 'LAYOUT_VIRIDIAN_CITY_GYM')
        selected_layout = next(item for item in layout['layouts']
                               if item['id'] == 'LAYOUT_VIRIDIAN_CITY_GYM')
        self.assertTrue(selected_layout['wayfarer_include'])
        self.assertEqual((selected_layout['width'], selected_layout['height']), (20, 24))
        self.assertEqual(selected_layout['blockdata_filepath'],
                         'data/layouts/ViridianCity_Gym_Frlg/map.bin')
        self.assertEqual(len(gym['object_events']), 10)  # Eight Trainers, Giovanni, guide.
        for warp in gym['warp_events']:
            self.assertEqual(warp['wayfarer_dest_map'], 'MAP_VIRIDIAN_CITY_HNS')
            self.assertEqual(warp['wayfarer_dest_warp_id'], '1')
        self.assertEqual([(warp['x'], warp['y']) for warp in gym['warp_events']],
                         [(16, 22), (17, 22), (18, 22)])
        self.assertEqual(gym['object_events'][7]['wayfarer_flag'],
                         'FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS')
        hidden = next(event for event in gym['bg_events'] if event['type'] == 'hidden_item')
        self.assertEqual(hidden['flag'], 'FLAG_HIDDEN_ITEM_VIRIDIAN_CITY_GYM_MACHO_BRACE')
        self.assertEqual(hidden['wayfarer_flag'], 'FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS')
        self.assertTrue(hidden['underfoot'])
        self.assertFalse(hidden['wayfarer_underfoot'])

    def test_three_exit_fallback_is_limited_to_the_walkable_gym_doorway(self):
        blocks = struct.unpack('<480H',
                               (ROOT / 'data/layouts/ViridianCity_Gym_Frlg/map.bin').read_bytes())
        for x in range(16, 19):
            block = blocks[22 * 20 + x]
            self.assertEqual((block >> 10) & 3, 0)  # Walkable source tile.
            self.assertEqual(block >> 12, 3)         # Matches the event elevation.
        source = (ROOT / 'src/field_control_avatar.c').read_text()
        self.assertIn('gMapHeader.mapLayoutId == LAYOUT_VIRIDIAN_CITY_GYM', source)
        self.assertIn('warpEventId >= 0 && warpEventId <= 2', source)
        self.assertIn('position->x - MAP_OFFSET == 16 + warpEventId', source)
        self.assertIn('position->y - MAP_OFFSET == 22', source)

    def test_generated_events_resolve_wayfarer_guide_and_statues(self):
        tool = ROOT / 'tools/mapjson'
        with tempfile.TemporaryDirectory(prefix='viridian-mapjson-') as directory:
            binary = Path(directory) / 'mapjson'
            output = Path(directory) / 'events'
            output.mkdir()
            subprocess.run(['c++', '-Wall', '-std=c++17', '-O2',
                            str(tool / 'json11.cpp'), str(tool / 'mapjson.cpp'),
                            '-o', str(binary)], check=True, capture_output=True)
            subprocess.run([str(binary), 'map', 'wayfarer',
                            'data/maps/ViridianCity_Gym_Frlg/map.json',
                            'data/layouts/layouts.json', str(output)],
                           cwd=ROOT, check=True, capture_output=True)
            events = (output / 'events.inc').read_text()
        self.assertIn('ViridianCity_FrlgGym_EventScript_GymGuy', events)
        self.assertEqual(events.count('ViridianCity_FrlgGym_EventScript_GymStatue'), 2)
        self.assertNotIn('ViridianCity_Gym_EventScript_GymGuy,', events)
        self.assertNotIn('ViridianCity_Gym_EventScript_GymStatue\n', events)
        self.assertIn('ITEM_MACHO_BRACE, FLAG_VIRIDIAN_GYM_HIDDEN_MACHO_BRACE_HNS, 1, FALSE', events)

    def test_wayfarer_registers_frlg_guide_sprite(self):
        graphics = (ROOT / 'src/data/object_events/object_event_graphics_info_pointers.h').read_text()
        wayfarer = graphics.split('#if IS_WAYFARER', 1)[1].split('#endif', 1)[0]
        self.assertIn('[OBJ_EVENT_GFX_GYM_GUY]', wayfarer)

    def test_local_prerequisites_and_reward_handoff_are_separate(self):
        source = (ROOT / 'data/maps/ViridianCity_Gym_Frlg/scripts.inc').read_text()
        active = source.split('#if IS_WAYFARER', 1)[1].split('#else', 1)[0]
        for marker in ('goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS',
                       'goto_if_unset FLAG_CELADON_ROCKET_HIDEOUT_SILPH_SCOPE_RECEIVED_HNS',
                       'goto_if_unset FLAG_SILPH_LIBERATED_HNS'):
            self.assertIn(marker, active)
        self.assertLess(active.index('additem ITEM_TM_EARTHQUAKE'),
                        active.index('setflag FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS'))
        self.assertIn('goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_NoRoomForEarthquake',
                      active.split('additem ITEM_TM_EARTHQUAKE', 1)[1])
        self.assertNotIn('FLAG_HIDE_MISC_KANTO_ROCKETS', active)
        self.assertNotIn('ITEM_TM26', active)


class BlueDojoUnlockContracts(unittest.TestCase):
    def test_dojo_visibility_is_derived_only_from_the_indigo_seam(self):
        dojo = (ROOT / 'data/maps/SaffronCity_FightingDojoVIP_hns/scripts.inc').read_text()
        self.assertIn('callnative SyncBlueDojoVisibility', dojo)
        self.assertIn('trainerbattle_no_intro TRAINER_BLUE_DOJO_HNS', dojo)
        rematch = dojo.split('SaffronCity_FightingDojoVIP_EventScript_Blue_Rematch::', 1)[1].split('\n\n', 1)[0]
        self.assertIn('givebp 10', rematch)
        self.assertNotIn('VIRIDIAN battle', dojo)
        sync = (ROOT / 'src/wayfarer_blue_dojo.c').read_text()
        self.assertIn('HasCommittedFirstIndigoVictory()', sync)
        # League commit and rollback paths must not own the Dojo projection.
        self.assertNotIn('FLAG_HIDE_DOJO_BLUE', (ROOT / 'src/league_circuit.c').read_text())


if __name__ == '__main__':
    unittest.main()
