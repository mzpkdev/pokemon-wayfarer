"""Ordered Nugget Bridge challenge without changing HNS's Machine Part scene."""
import copy
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import unittest

GAME_ROOT = Path(__file__).resolve().parents[3]
NAMES = ['Cale', 'Ali', 'Timmy', 'Reli', 'Ethan', 'Rocket']
POSITIONS = [(17, 19), (19, 16), (19, 13), (17, 12), (17, 6), (19, 3)]
SCRIPT_PATH = GAME_ROOT / 'data/scripts/wayfarer_nugget_bridge.inc'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def canonical(data):
    return json.dumps(data, sort_keys=True, separators=(',', ':')).encode()


class NuggetBridgeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.root = Path(cls.temp.name)
        cls.tool = cls.root / 'mapjson'
        subprocess.run(['g++', '-std=c++17', str(GAME_ROOT / 'tools/mapjson/mapjson.cpp'),
                        str(GAME_ROOT / 'tools/mapjson/json11.cpp'), '-o', str(cls.tool)], check=True)
        cls.map = json.loads((GAME_ROOT / 'data/maps/Route24_hns/map.json').read_text())
        cls.layout = next(layout for layout in json.loads((GAME_ROOT / 'data/layouts/layouts.json').read_text())['layouts']
                          if layout.get('id') == 'LAYOUT_ROUTE24_HNS')
        cls.blocks = (GAME_ROOT / cls.layout['blockdata_filepath']).read_bytes()

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def tile(self, x, y):
        self.assertTrue(0 <= x < self.layout['width'] and 0 <= y < self.layout['height'])
        return struct.unpack_from('<H', self.blocks, 2 * (y * self.layout['width'] + x))[0]

    def test_original_route_and_neighbor_stories_are_unchanged(self):
        original = copy.deepcopy(self.map)
        original['object_events'] = [event for event in original['object_events'] if not event.get('wayfarer_only')]
        self.assertEqual(len(original['object_events']), 8)
        self.assertEqual(digest(canonical(original)), '16b69c9ca80324616f9fa3f71894f1c83ca4362c355f8c79ae3270ddcc298283')
        files = {
            'data/maps/Route24_hns/scripts.inc': '7ac9c83c25b94c0dade26ec939754c23705b4b9214ab79b4ce4724afd8133171',
            'data/maps/Route24_Frlg/scripts.inc': '1411b48acab4b080e877fa865344dd6b468d6d8ec5a1cb6046ce3cc977222dcd',
            'data/maps/Route25_hns/scripts.inc': '47dc4c70596a0f9f5be088816bd6b316333bd84f0c885d4623cf25217272bf16',
            'data/layouts/Route24_hns/map.bin': '56f43c61dfc7c2557a7673dd56fbf3f7a310c7ff59002e8243c58e4e6bd67696',
        }
        for path, expected in files.items():
            self.assertEqual(digest((GAME_ROOT / path).read_bytes()), expected, path)
        source = json.loads((GAME_ROOT / 'data/maps/Route24_Frlg/map.json').read_text())
        self.assertEqual(digest(canonical(source)), 'b8fc05f7534be4b36343b0afef9201a3931c619947e1af274942a695dc0c3060')

    def test_actors_use_dry_bridge_and_leave_the_center_lane(self):
        self.assertEqual((self.layout['width'], self.layout['height']), (30, 22))
        self.assertEqual(len(self.blocks), 2 * self.layout['width'] * self.layout['height'])
        actors = [event for event in self.map['object_events'] if event.get('wayfarer_only')]
        self.assertEqual(len(self.map['object_events']), 14)
        self.assertLessEqual(len(self.map['object_events']), 15)
        self.assertEqual([(actor['x'], actor['y']) for actor in actors], POSITIONS)
        attrs = (GAME_ROOT / 'data/tilesets/secondary/cerulean_city_hns/metatile_attributes.bin').read_bytes()
        graphics = ['BUG_CATCHER', 'LASS', 'YOUNGSTER', 'LASS', 'CAMPER', 'ROCKET_M']
        for actor, name, graphic in zip(actors, NAMES, graphics):
            self.assertEqual(actor['local_id'], 'LOCALID_NUGGET_BRIDGE_' + name.upper())
            self.assertEqual(actor['script'], 'WayfarerNuggetBridge_EventScript_' + name)
            self.assertEqual(actor['graphics_id'], 'OBJ_EVENT_GFX_' + graphic + '_HNS')
            self.assertEqual((actor['elevation'], actor['movement_type'], actor['trainer_type'], actor['trainer_sight_or_berry_tree_id']),
                             (0, 'MOVEMENT_TYPE_FACE_DOWN', 'TRAINER_TYPE_NONE', '0'))
            self.assertFalse(16 <= actor['x'] <= 18 and 7 <= actor['y'] <= 11)
            for y in [actor['y'], actor['y'] + 1]:
                tile = self.tile(actor['x'], y)
                self.assertEqual(((tile >> 10) & 3, tile >> 12), (0, 3))
                metatile = tile & 0x3ff
                self.assertIn(metatile, [0x2e5, 0x2e6])
                self.assertEqual(struct.unpack_from('<H', attrs, 2 * (metatile - 640))[0] & 0xff, 0)
        self.assertFalse(any(actor['x'] == 18 for actor in actors))
        for y in range(self.layout['height']):
            self.assertEqual((self.tile(18, y) >> 10) & 3, 0)
        scene = (GAME_ROOT / 'data/maps/Route24_hns/scripts.inc').read_text()
        self.assertIn('setvar VAR_KANTO_ROCKET_STORY_STATE, 4', scene)
        self.assertIn('setobjectxyperm LOCALID_ROUTE24_GRUNT, 16, 8', scene)
        self.assertIn('setobjectxyperm LOCALID_ROUTE24_WOMAN, 16, 9', scene)

    def test_generator_preserves_hns_and_assigns_appended_local_ids(self):
        generated = {}
        for version in ['hns', 'wayfarer']:
            output = self.root / version
            output.mkdir()
            subprocess.run([str(self.tool), 'map', version, 'data/maps/Route24_hns/map.json',
                            'data/layouts/layouts.json', str(output)], cwd=GAME_ROOT, check=True, capture_output=True)
            generated[version] = (output / 'events.inc').read_text()
            ids = output / 'ids.h'
            subprocess.run([str(self.tool), 'event_constants', version,
                            'data/maps/Route24_hns/map.json', str(ids)], cwd=GAME_ROOT, check=True)
            constants = ids.read_text()
            self.assertIn('#define LOCALID_ROUTE24_GRUNT 1', constants)
            self.assertIn('#define LOCALID_ROUTE24_MAN 3', constants)
            self.assertIn('#define LOCALID_ROUTE24_WOMAN 4', constants)
            for index, name in enumerate(NAMES, 9):
                label = 'LOCALID_NUGGET_BRIDGE_' + name.upper()
                if version == 'wayfarer':
                    self.assertIn(f'#define {label} {index}', constants)
                else:
                    self.assertNotIn(label, constants)
        stripped = ''.join(line for line in generated['wayfarer'].splitlines(keepends=True)
                           if 'WayfarerNuggetBridge' not in line)
        self.assertEqual(stripped, generated['hns'])

    def test_order_and_reward_checks_precede_battle_staging(self):
        script = SCRIPT_PATH.read_text()
        for index, name in enumerate(NAMES[:5]):
            start = script.index(f'WayfarerNuggetBridge_EventScript_{name}::')
            end = script.index(f'WayfarerNuggetBridge_EventScript_{name}After::', start)
            block = script[start:end]
            self.assertIn(f'goto_if_defeated TRAINER_NUGGET_BRIDGE_{name.upper()}_HNS', block)
            for earlier in NAMES[:index]:
                self.assertIn(f'goto_if_not_defeated TRAINER_NUGGET_BRIDGE_{earlier.upper()}_HNS, WayfarerNuggetBridge_EventScript_Next{earlier}', block)
            self.assertLess(block.index('WayfarerCanStartOrdinaryBattleForScript'), block.index('\tlock'))
            self.assertRegex(block, r'GetBattleOutcome\n\s*goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerNuggetBridge_EventScript_End')
        rocket = script[script.index('WayfarerNuggetBridge_EventScript_Rocket::'):]
        add = rocket.index('additem ITEM_NUGGET')
        receipt = rocket.index('setflag FLAG_NUGGET_BRIDGE_NUGGET_RECEIVED_HNS')
        guard = rocket.index('WayfarerCanStartOrdinaryBattleForScript')
        self.assertLess(add, receipt)
        self.assertLess(receipt, guard)
        self.assertNotIn('checkitem ITEM_NUGGET', rocket)
        self.assertRegex(rocket[add:], r'additem ITEM_NUGGET\n\s*goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_NoRoomLocked')
        recruitment = rocket[rocket.index('WayfarerNuggetBridge_EventScript_Recruitment::'):]
        self.assertLess(recruitment.index('WayfarerCanStartOrdinaryBattleForScript'), recruitment.index('\tlock'))
        self.assertLess(recruitment.index('WayfarerCanStartOrdinaryBattleForScript'), recruitment.index('WayfarerNuggetBridge_Text_JoinTeamRocket'))
        self.assertEqual(script.count('GetBattleOutcome'), 6)
        self.assertEqual(re.findall(r'\b(?:setflag|clearflag) (\w+)', script), ['FLAG_NUGGET_BRIDGE_NUGGET_RECEIVED_HNS'])
        for forbidden in ['VAR_KANTO_ROCKET_STORY_STATE', 'FLAG_ROUTE25_GOT_NUGGET', 'VAR_MAP_SCENE_ROUTE24',
                          'TRAINER_GRUNT_31_HNS', 'applymovement', 'removeobject', 'setvar']:
            self.assertNotIn(forbidden, script)

    def test_receipt_is_wayfarer_local(self):
        for define, expected in [('POKEMON_WAYFARER', '0x4C3'), ('POKEMON_HNS', '0')]:
            output = subprocess.run(['cpp', '-dM', f'-D{define}', '-I', str(GAME_ROOT / 'include'),
                                     '-include', 'global.h', '-include', 'constants/flags.h', '-'],
                                    input='', text=True, capture_output=True, check=True).stdout
            macros = dict(re.findall(r'^#define\s+(\w+)\s+(.+)$', output, re.MULTILINE))
            self.assertEqual(macros['FLAG_NUGGET_BRIDGE_NUGGET_RECEIVED_HNS'], expected)
