"""Cerulean's local burglary preserves HNS geography and reward ownership."""
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import unittest

GAME_ROOT = Path(__file__).resolve().parents[3]
SCRIPT = GAME_ROOT / 'data/scripts/wayfarer_cerulean_burglary.inc'


class CeruleanBurglaryTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.root = Path(cls.temp.name)
        cls.tool = cls.root / 'mapjson'
        subprocess.run(['g++', '-std=c++17',
                        str(GAME_ROOT / 'tools/mapjson/mapjson.cpp'),
                        str(GAME_ROOT / 'tools/mapjson/json11.cpp'),
                        '-o', str(cls.tool)], check=True)
        cls.layouts = {layout['id']: layout for layout in json.loads(
            (GAME_ROOT / 'data/layouts/layouts.json').read_text())['layouts']}

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def generated(self, name, version):
        output = self.root / f'{name}-{version}'
        output.mkdir(exist_ok=True)
        subprocess.run([str(self.tool), 'map', version, f'data/maps/{name}/map.json',
                        'data/layouts/layouts.json', str(output)],
                       cwd=GAME_ROOT, check=True, capture_output=True)
        return {name: (output / name).read_text() for name in ['events.inc', 'header.inc']}

    def grid(self, layout_id, x, y):
        layout = self.layouts[layout_id]
        self.assertTrue(0 <= x < layout['width'] and 0 <= y < layout['height'])
        data = (GAME_ROOT / layout['blockdata_filepath']).read_bytes()
        self.assertEqual(len(data), 2 * layout['width'] * layout['height'])
        tile = struct.unpack_from('<H', data, 2 * (y * layout['width'] + x))[0]
        return tile & 0x3ff, (tile >> 10) & 3, tile >> 12

    def test_new_layout_appends_without_renumbering_existing_layouts(self):
        entries = json.loads((GAME_ROOT / 'data/layouts/layouts.json').read_text())['layouts']
        original_ids = [entry.get('id') for entry in entries[:1331]]
        digest = hashlib.sha256(json.dumps(original_ids, separators=(',', ':')).encode()).hexdigest()
        self.assertEqual(digest, '340d1c7b81bba5fef910e294543c312e096a09698c8fb61821ab9f3b09b84f4f')
        self.assertEqual(entries[1331]['id'], 'LAYOUT_CERULEAN_CITY_BURGLARY_WAYFARER')

    def test_bounded_layout_and_house_approaches(self):
        hashes = {
            'CeruleanCity_hns': 'bade52bf6886a51bf7c3a0cce5a30d9b2884b44a2cf70b6615378f67aa3a6982',
            'CeruleanCity_House2_hns': '517191f70f60587e544d08097dc205e58ce11b5c389bba26e90818dcbeb096b4',
            'CeruleanCity_House2_Frlg': '333f67d83f0c5d7b3c693f6c677f49b65c2f6bc93e5c992ba8ce40b27e2082d8',
        }
        for name, digest in hashes.items():
            self.assertEqual(hashlib.sha256((GAME_ROOT / f'data/layouts/{name}/map.bin').read_bytes()).hexdigest(), digest)
        city_id = 'LAYOUT_CERULEAN_CITY_BURGLARY_WAYFARER'
        source_id = 'LAYOUT_CERULEAN_CITY_HNS'
        city = self.layouts[city_id]
        source = self.layouts[source_id]
        for field in ['width', 'height', 'border_filepath', 'primary_tileset', 'secondary_tileset', 'layout_version']:
            self.assertEqual(city[field], source[field])
        changes = {(x, y) for x in range(city['width']) for y in range(city['height'])
                   if self.grid(city_id, x, y) != self.grid(source_id, x, y)}
        self.assertEqual(changes, {(34, 17), (35, 17), (34, 18), (35, 18)})
        for x, y in [*changes, (37, 23), (36, 23), (35, 23), (35, 16)]:
            self.assertEqual(self.grid(city_id, x, y)[1:], (0, 3))
        for x, y in [(1, 2), (2, 2), (8, 5), (7, 5), (2, 4), (2, 5), (4, 2), (4, 3)]:
            self.assertEqual(self.grid('LAYOUT_CERULEAN_CITY_HOUSE2', x, y)[1:], (0, 3))

    def test_generator_selects_routes_household_and_layout(self):
        city = self.generated('CeruleanCity_hns', 'wayfarer')
        source_city = self.generated('CeruleanCity_hns', 'hns')
        house = self.generated('CeruleanCity_House2_Frlg', 'wayfarer')
        source_house = self.generated('CeruleanCity_House2_Frlg', 'firered')
        self.assertIn(self.layouts['LAYOUT_CERULEAN_CITY_BURGLARY_WAYFARER']['name'], city['header.inc'])
        self.assertNotIn('BURGLARY_WAYFARER', source_city['header.inc'])
        self.assertIn('warp_def 35, 22, 0, 1, MAP_CERULEAN_CITY_HOUSE2', city['events.inc'])
        self.assertIn('warp_def 35, 18, 3, 3, MAP_CERULEAN_CITY_HOUSE2', city['events.inc'])
        self.assertIn('warp_def 35, 22, 0, 0, MAP_CERULEAN_CITY_HOUSE2_HNS', source_city['events.inc'])
        self.assertIn('WayfarerCeruleanBurglary_EventScript_RearEntry', city['events.inc'])
        self.assertNotIn('WayfarerCeruleanBurglary', source_city['events.inc'])
        self.assertNotIn('CeruleanCity_House2_EventScript_Hiker', house['events.inc'])
        self.assertIn('CeruleanCity_House2_EventScript_Hiker', source_house['events.inc'])
        self.assertNotIn('WayfarerCeruleanBurglary', source_house['events.inc'])
        self.assertEqual(len(re.findall(r'^\s*object_event ', house['events.inc'], re.MULTILINE)), 3)
        self.assertIn('object_event 1, OBJ_EVENT_GFX_WOMAN_3_HNS, 8, 5, 3', house['events.inc'])
        self.assertIn('object_event 2, OBJ_EVENT_GFX_BALDING_MAN_HNS, 1, 2, 3', house['events.inc'])
        self.assertIn('object_event 3, OBJ_EVENT_GFX_MON_BASE+SPECIES_DIGLETT, 2, 4, 3', house['events.inc'])
        self.assertIn('warp_def 4, 1, 0, 10, MAP_CERULEAN_CITY_HNS', house['events.inc'])
        self.assertEqual(house['events.inc'].count('2, MAP_CERULEAN_CITY_HNS'), 3)
        paths = ['data/maps/CeruleanCity_hns/map.json', 'data/maps/CeruleanCity_House2_Frlg/map.json']
        for version in ['wayfarer', 'hns', 'firered']:
            output = self.root / f'ids-{version}.h'
            subprocess.run([str(self.tool), 'event_constants', version, *paths, str(output)], cwd=GAME_ROOT, check=True)
            ids = output.read_text()
            if version == 'wayfarer':
                for name, number in [('WOMAN', 1), ('MAN', 2), ('DIGLETT', 3)]:
                    self.assertIn(f'#define LOCALID_CERULEAN_BURGLARY_{name} {number}', ids)
                self.assertIn('#define WARP_ID_CERULEAN_BURGLARY_REAR 10', ids)
            else:
                self.assertNotIn('CERULEAN_BURGLARY', ids)

    def test_story_state_is_local_and_receipt_is_transactional(self):
        script = SCRIPT.read_text()
        battle = script.index('trainerbattle_no_intro')
        guard = script.index('WayfarerCanStartOrdinaryBattleForScript')
        lock = script.index('\tlock')
        self.assertLess(guard, lock)
        self.assertLess(lock, battle)
        self.assertIn('goto_if_defeated TRAINER_CERULEAN_BURGLARY_GRUNT_HNS, WayfarerCeruleanBurglary_EventScript_Reward', script[:guard])
        self.assertRegex(script[battle:], r'GetBattleOutcome\n\s*goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeruleanBurglary_EventScript_End')
        reward = script[script.index('WayfarerCeruleanBurglary_EventScript_Reward::'):]
        space = reward.index('checkitemspace ITEM_TM_DIG')
        add = reward.index('additem ITEM_TM_DIG')
        receipt = reward.index('setflag FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS')
        removal = reward.index('removeobject LOCALID_CERULEAN_BURGLARY_GRUNT')
        self.assertNotIn('checkitem ITEM_TM_DIG', reward[:space])
        self.assertLess(space, add)
        self.assertLess(add, receipt)
        self.assertLess(receipt, removal)
        self.assertRegex(reward[add:], r'additem ITEM_TM_DIG\n\s*goto_if_eq VAR_RESULT, FALSE, WayfarerCeruleanBurglary_EventScript_NoRoom')
        self.assertEqual(re.findall(r'\b(?:setflag|clearflag) (\w+)', script),
                         ['FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS'])
        self.assertNotIn('setvar ', script)
        self.assertNotIn('ITEM_TM28', script)

    def test_receipt_flag_and_house_routes_are_wayfarer_only(self):
        for define, expected in [('POKEMON_WAYFARER', '0x4C2'), ('POKEMON_HNS', '0')]:
            result = subprocess.run(
                ['cpp', '-dM', f'-D{define}', '-I', str(GAME_ROOT / 'include'),
                 '-include', 'global.h', '-include', 'constants/flags.h', '-'],
                input='', text=True, capture_output=True, check=True)
            macros = dict(re.findall(r'^#define\s+(\w+)\s+(.+)$', result.stdout, re.MULTILINE))
            self.assertEqual(macros['FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS'], expected)
        house = json.loads((GAME_ROOT / 'data/maps/CeruleanCity_House2_Frlg/map.json').read_text())
        self.assertTrue(house['wayfarer_include'])
        self.assertTrue(all(actor['wayfarer_exclude'] for actor in house['object_events'][:2]))
        self.assertTrue(all(actor['wayfarer_only'] for actor in house['object_events'][2:]))
        city = json.loads((GAME_ROOT / 'data/maps/CeruleanCity_hns/map.json').read_text())
        rear = next(event for event in city['bg_events'] if event.get('wayfarer_only'))
        self.assertEqual((rear['x'], rear['y'], rear['elevation'], rear['player_facing_dir']),
                         (35, 19, 3, 'BG_EVENT_PLAYER_FACING_SOUTH'))
        self.assertIn('warp MAP_CERULEAN_CITY_HOUSE2, 4, 2', SCRIPT.read_text())
