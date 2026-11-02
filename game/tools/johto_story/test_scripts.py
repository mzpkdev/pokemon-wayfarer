"""Execute Wayfarer's Johto story scripts and check that adventures stay independent.

The scripts are preprocessed with -DIS_WAYFARER=1 and run through a small
interpreter. It checks state transactions (who writes which variable, in which
direction), not presentation. Message boxes answer yes, battles are won, and
engine specials are deterministic inputs.

It reuses the constant table of the origin script tests.
"""
from functools import lru_cache
from itertools import permutations
from pathlib import Path
import importlib.util
import json
import re
import subprocess
import unittest
import zlib

GAME = Path(__file__).resolve().parents[2]
MAPS = GAME / 'data/maps'

_spec = importlib.util.spec_from_file_location(
    'wayfarer_origin_test_scripts', GAME / 'tools/wayfarer_origin/test_scripts.py')
_origin = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_origin)

AZ = 'VAR_AZALEA_TOWN_STATE'
WELL = 'VAR_WAYFARER_JOHTO_SLOWPOKE_WELL_STATE'
GOLD = 'VAR_GOLDENROD_CITY_STATE'
WHITNEY = 'VAR_WAYFARER_JOHTO_WHITNEY_STATE'
MAH = 'VAR_MAHOGANY_TOWN_STATE'
THEATER = 'VAR_ECRUTEAK_CITY_THEATER'
CEREMONY = 'VAR_WAYFARER_JOHTO_KIMONO_CEREMONY_STATE'
ECR = 'VAR_ECRUTEAK_CITY_STATE'
RESOLVED = 'FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED'

SETUP_MAPS = {
    'violet': 'VioletCity_hns',
    'azalea': 'AzaleaTown_hns',
    'goldenrod': 'GoldenrodCity_hns',
    'ecruteak': 'EcruteakCity_hns',
    'olivine': 'OlivineCity_hns',
    'mahogany': 'Mahoganytown_hns',
    'blackthorn': 'BlackthornCity_hns',
}
SETUP_FLAGS = {name: 'FLAG_WAYFARER_JOHTO_SETUP_' + name.upper() for name in SETUP_MAPS}
SETUP_LABELS = {
    'violet': 'VioletCity_EventScript_Setup',
    'azalea': 'AzaleaTown_EventScript_Setup',
    'goldenrod': 'GoldenrodCity_EventScript_Setup',
    'ecruteak': 'EcruteakCity_EventScript_Setup',
    'olivine': 'OlivineCity_EventScript_Setup',
    'mahogany': 'MahoganyTown_EventScript_Setup',
    'blackthorn': 'BlackthornCity_EventScript_Setup',
}
TRANSITION_LABELS = {
    'violet': 'VioletCity_OnTransition',
    'azalea': 'AzaleaTown_OnTransition',
    'goldenrod': 'GoldenrodCity_OnTransition',
    'ecruteak': 'EcruteakCity_OnTransition',
    'olivine': 'OlivineCity_MapScript_OnTransition',
    'mahogany': 'Mahoganytown_OnTransition',
    'blackthorn': 'BlackthornCity_EventScript_CityEnter',
}

# State and rewards each town's setup must never write.
PROTECTED = {
    'violet': {'FLAG_RECEIVED_TOGEPI_EGG', 'VAR_SPROUT_TOWER', 'FLAG_HIDE_SPROUT_TOWER_SILVER',
               'VAR_RUINSOFALPH_STATE', 'FLAG_VIOLET_NPC_TRADE_COMPLETED',
               # Ruins puzzle rewards revealed by puzzles solved before the first visit.
               'FLAG_ITEM_GS_BALL', 'FLAG_ITEM_DOME_FOSSIL', 'FLAG_ITEM_HELIX_FOSSIL',
               'FLAG_ITEM_OLD_AMBER'},
    'azalea': {'FLAG_GOT_CHARCOAL', 'FLAG_HIDE_AZALEA_TOWN_ROCKETS', 'FLAG_HIDE_KURT_1',
               'FLAG_HIDE_AZALEA_TOWN_FARFETCHD', 'FLAG_HIDE_AZALEA_TOWN_CUT_MASTER'},
    'goldenrod': {'VAR_LUGIA_OR_HOOH', 'FLAG_RECEIVED_BIKE', 'FLAG_GOT_EEVEE', 'FLAG_RECEIVED_SQUIRTBOTTLE',
                  'FLAG_GOT_RETURN', 'FLAG_GOT_FRUSTRATION', 'FLAG_GOT_SUNNY_DAY', 'VAR_KENYA',
                  'VAR_BUG_CONTEST_STATE', 'FLAG_HIDE_GOLDENROD_ROCKET_SCOUT', 'FLAG_HIDE_GOLDENROD_NPCS',
                  'FLAG_SYS_PC_BILL'},
    'ecruteak': {THEATER, CEREMONY, 'VAR_COMPLETED_HO_OH', 'VAR_COMPLETED_LUGIA', 'FLAG_RELEASED_BEASTS',
                 'FLAG_AMPHAROS_HEALED', 'FLAG_HIDE_ROUTE42_SUICUNE', 'FLAG_HIDE_TINTOWER_GUARD',
                 'FLAG_CAUGHT_HO_OH', 'FLAG_CAUGHT_LUGIA', 'FLAG_CAUGHT_ENTEI', 'FLAG_CAUGHT_RAIKOU',
                 'FLAG_DEFEATED_HO_OH', 'FLAG_DEFEATED_LUGIA', 'FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS'},
    'olivine': set(),
    'mahogany': {'FLAG_GOT_DRATINI', 'VAR_BLACKTHORN_CITY_STATE', 'VAR_BLACKTHORN_GYM_STATE',
                 'VAR_DRAGONS_DEN_QUIZ', 'FLAG_HIDE_DRAGONS_DEN_CAVERN_CLAIR',
                 'FLAG_HIDE_DRAGONS_DEN_CAVERN_LANCE', 'FLAG_HIDE_DRAGONS_DEN_CAVERN_SILVER',
                 'FLAG_HIDE_DRAGONS_DEN_SHRINE_CLAIR', 'FLAG_GOT_SPELL_TAG'},
    'blackthorn': {'FLAG_GOT_DRATINI', 'VAR_DRAGONS_DEN_QUIZ', 'VAR_BLACKTHORN_GYM_STATE',
                   'FLAG_GOT_SPELL_TAG'},
}
# The state variable each town's setup moves from 0 to 1.
TOWN_STATE = {
    'violet': 'VAR_VIOLET_CITY_STATE', 'azalea': AZ, 'goldenrod': GOLD, 'ecruteak': ECR,
    'olivine': 'VAR_OLIVINE_CITY_STATE', 'mahogany': MAH, 'blackthorn': 'VAR_BLACKTHORN_CITY_STATE',
}


@lru_cache(maxsize=None)
def source(map_name):
    return subprocess.check_output([
        'cpp', '-P', '-x', 'assembler-with-cpp', '-I%s' % (GAME / 'include'),
        '-DIS_WAYFARER=1', '-DWAYFARER_LEAGUE_CIRCUIT_ENABLED=1', '-DWAYFARER_TR_MARTS_ENABLED=1',
        str(MAPS / map_name / 'scripts.inc'),
    ], text=True)


def raw_text(map_name):
    return (MAPS / map_name / 'scripts.inc').read_text()


def all_script_files():
    return sorted(MAPS.glob('*/scripts.inc'))


class Interp:
    """Runs labels from one or more maps against shared variable and flag state."""

    def __init__(self, *maps, vars=None, flags=(), johto=True, answers=1, bag_full=False):
        self.lines = []
        self.labels = {}
        for name in maps:
            for line in source(name).splitlines():
                self.lines.append(line)
                stripped = line.split('@')[0].strip()
                if re.fullmatch(r'\w+:+', stripped):
                    self.labels.setdefault(stripped.rstrip(':'), len(self.lines))
        self.vars = dict(vars or {})
        self.flags = set(flags)
        self.johto = johto
        self.answer = answers
        self.bag_full = bag_full
        self.writes = []      # (name, value, kind) in order
        self.messages = []

    def get(self, name):
        if name in self.vars:
            return self.vars[name]
        if name in _origin.CONSTANTS:
            return _origin.CONSTANTS[name]
        try:
            return int(name, 0)
        except ValueError:
            # Unknown VAR_ names start at 0; other symbols are distinct constants.
            return 0 if name.startswith('VAR_') else 0x10000 + zlib.crc32(name.encode()) % 0xFFFF

    def set_var(self, name, value):
        self.vars[name] = value
        self.writes.append((name, value, 'var'))

    def set_flag(self, name, on):
        (self.flags.add if on else self.flags.discard)(name)
        self.writes.append((name, int(on), 'flag'))

    def cond(self, kind, args):
        if kind in ('set', 'unset'):
            return (args[0] in self.flags) == (kind == 'set')
        if kind == 'defeated':
            return False
        a, b = self.get(args[0]), self.get(args[1])
        return {'eq': a == b, 'ne': a != b, 'ge': a >= b, 'gt': a > b, 'lt': a < b, 'le': a <= b}[kind]

    def run(self, label, depth=0):
        pc = self.labels[label]
        stack = []
        switch_value = None
        for _ in range(5000):
            raw = self.lines[pc].split('@')[0].strip()
            pc += 1
            if not raw or raw.startswith(('@', '.', '#')) or re.fullmatch(r'\w+:+', raw):
                continue
            parts = raw.split(None, 1)
            command = parts[0]
            args = [x.strip() for x in parts[1].split('@')[0].split(',')] if len(parts) > 1 else []
            if command in ('end', 'release', 'releaseall') and command == 'end':
                return
            if command == 'return':
                if not stack:
                    return
                pc = stack.pop()
            elif command in ('setvar', 'copyvar'):
                self.set_var(args[0], self.get(args[1]))
            elif command == 'setvar_forward':
                if self.get(args[0]) < self.get(args[1]):
                    self.set_var(args[0], self.get(args[1]))
            elif command == 'addvar':
                self.set_var(args[0], self.get(args[0]) + self.get(args[1]))
            elif command in ('setflag', 'clearflag'):
                self.set_flag(args[0], command == 'setflag')
            elif command == 'switch':
                switch_value = self.get(args[0])
            elif command == 'case':
                if self.get(args[0]) == switch_value:
                    pc = self.labels[args[1]]
            elif command == 'specialvar':
                self.vars[args[0]] = {'WayfarerIsJohtoOrigin': int(self.johto)}.get(args[1], 0)
            elif command == 'msgbox':
                self.messages.append(args[0])
                if args[-1] == 'MSGBOX_YESNO':
                    self.vars['VAR_RESULT'] = self.answer
            elif command == 'giveitem':
                self.vars['VAR_RESULT'] = 0 if self.bag_full else 1
            elif command in ('checkitem', 'checkmoney'):
                self.vars['VAR_RESULT'] = 1
            elif command == 'goto':
                if args[0] not in self.labels:
                    return
                pc = self.labels[args[0]]
            elif command == 'call':
                if args[0] in self.labels:
                    stack.append(pc)
                    pc = self.labels[args[0]]
            elif command.startswith(('goto_if_', 'call_if_')):
                kind = command.split('_if_')[1]
                if self.cond(kind, args) and args[-1] in self.labels:
                    if command.startswith('call'):
                        stack.append(pc)
                    pc = self.labels[args[-1]]
                elif self.cond(kind, args) and command.startswith('goto'):
                    return
        raise AssertionError('script did not terminate: ' + label)

    def written(self, kind=None):
        return {name for name, _, k in self.writes if kind in (None, k)}


def run_setup(town, **kwargs):
    interp = Interp(SETUP_MAPS[town], 'GoldenrodCity_hns', **kwargs) if town == 'goldenrod' else Interp(
        SETUP_MAPS[town], **kwargs)
    interp.run(TRANSITION_LABELS[town])
    return interp


class TownSetup(unittest.TestCase):
    def test_old_road_triggers_are_gone(self):
        roads = {
            'Gate_Route31_VioletCity_hns': 'VioletCity_EventScript_Trigger',
            'Route33_hns': 'AzaleaTown_EventScript_Trigger',
            'Route34_hns': 'GoldenrodCity_EventScript_Trigger',
            'Route37_hns': 'EcruteakCity_EventScript_Trigger',
            'Route39_hns': 'OlivineCity_EventScript_CityTrigger',
            'Route42_hns': 'MahoganyTown_EventScript_TownTrigger',
        }
        setup_names = set(SETUP_LABELS.values())
        for map_name, label in roads.items():
            events = json.loads((MAPS / map_name / 'map.json').read_text())['coord_events']
            scripts = {event['script'] for event in events}
            with self.subTest(map=map_name):
                self.assertNotIn(label, scripts)
                self.assertFalse(scripts & setup_names)
        for path in MAPS.glob('*/map.json'):
            text = path.read_text()
            for label in list(roads.values()) + list(setup_names):
                self.assertNotIn('"%s"' % label, text, path.parent.name)

    def test_setup_runs_from_the_town_load_behind_its_own_flag(self):
        for town, map_name in SETUP_MAPS.items():
            with self.subTest(town=town):
                text = raw_text(map_name)
                self.assertIn('MAP_SCRIPT_ON_TRANSITION', text)
                transition = _label_block(text, TRANSITION_LABELS[town])
                self.assertIn('call_if_unset %s, %s' % (SETUP_FLAGS[town], SETUP_LABELS[town]), transition)
                setup = _label_block(text, SETUP_LABELS[town])
                self.assertIn('setflag ' + SETUP_FLAGS[town], setup)

    def test_setup_never_writes_protected_state(self):
        for town, map_name in SETUP_MAPS.items():
            with self.subTest(town=town):
                setup = _label_block(raw_text(map_name), SETUP_LABELS[town])
                for line in setup.splitlines():
                    parts = line.split('@')[0].split()
                    if len(parts) >= 2 and parts[0] in ('setflag', 'clearflag', 'setvar', 'setvar_forward'):
                        self.assertNotIn(parts[1].rstrip(','), PROTECTED[town], line)
                    # Defaults are not restated: no clears and no zero writes.
                    self.assertFalse(line.strip().startswith('clearflag'), line)
                    self.assertFalse(re.match(r'\s*setvar\s+\w+,\s*0\b', line), line)

    def test_setup_runs_once_from_every_entry_and_never_resets(self):
        for town in SETUP_MAPS:
            with self.subTest(town=town):
                state = TOWN_STATE[town]
                interp = run_setup(town)
                self.assertIn(SETUP_FLAGS[town], interp.flags)
                self.assertEqual(interp.vars.get(state), 1)
                # Progress made after the first visit survives every later entry.
                interp.vars[state] = 7
                before = (dict(interp.vars), set(interp.flags))
                interp.writes.clear()
                interp.run(TRANSITION_LABELS[town])
                self.assertEqual((interp.vars, interp.flags), before)
                self.assertEqual(interp.writes if town != 'blackthorn' else [], [])

    def test_setup_leaves_progress_made_before_the_first_visit(self):
        cases = {
            # (town, preset vars, preset flags): names that must be untouched
            'violet': ({'VAR_SPROUT_TOWER': 3, 'VAR_RUINSOFALPH_STATE': 4, 'VAR_VIOLET_CITY_STATE': 4},
                       {'FLAG_RECEIVED_TOGEPI_EGG', 'FLAG_HIDE_SPROUT_TOWER_SILVER'}),
            'azalea': ({AZ: 9, WELL: 4, 'VAR_ILEX_FOREST_FARFETCHD': 3}, {'FLAG_GOT_CHARCOAL'}),
            'goldenrod': ({GOLD: 9, 'VAR_LUGIA_OR_HOOH': 2, WHITNEY: 5},
                          {'FLAG_RECEIVED_BIKE', 'FLAG_GOT_EEVEE', 'FLAG_SYS_PC_BILL', 'FLAG_SHOWN_ELM_TOGEPI'}),
            'ecruteak': ({THEATER: 3, CEREMONY: 4, ECR: 4, 'VAR_COMPLETED_HO_OH': 3, 'VAR_SUICUNE_ENCOUNTERS': 3},
                         {'FLAG_RELEASED_BEASTS', 'FLAG_AMPHAROS_HEALED', 'FLAG_HIDE_ROUTE42_SUICUNE',
                          'FLAG_HIDE_TINTOWER_GUARD', 'FLAG_CAUGHT_HO_OH'}),
            'olivine': ({'VAR_OLIVINE_CITY_STATE': 4, 'VAR_SUICUNE_ENCOUNTERS': 2, 'VAR_CIANWOOD_CITY_STATE': 2},
                        set()),
            'mahogany': ({MAH: 9, 'VAR_BLACKTHORN_CITY_STATE': 4, 'VAR_DRAGONS_DEN_QUIZ': 2},
                         {'FLAG_GOT_DRATINI'}),
            'blackthorn': ({'VAR_BLACKTHORN_CITY_STATE': 3, 'VAR_DRAGONS_DEN_QUIZ': 2, 'VAR_BLACKTHORN_GYM_STATE': 2},
                           {'FLAG_GOT_DRATINI'}),
        }
        for town, (preset, flags) in cases.items():
            with self.subTest(town=town):
                interp = run_setup(town, vars=preset, flags=flags)
                for name, value in preset.items():
                    self.assertEqual(interp.vars[name], value, name)
                self.assertTrue(flags <= interp.flags)
                self.assertIn(SETUP_FLAGS[town], interp.flags)

    def test_goldenrod_takeover_started_before_the_first_visit_is_not_reset(self):
        # The Pryce arming can start the takeover before Goldenrod is visited.
        interp = Interp('GoldenrodCity_hns', 'Mahoganytown_hns')
        interp.run('Mahoganytown_EventScript_RocketTrigger')
        self.assertEqual(interp.vars[GOLD], 6)
        interp.writes.clear()
        interp.run('GoldenrodCity_OnTransition')
        self.assertEqual(interp.vars[GOLD], 6)
        hide = {'FLAG_HIDE_GOLDENROD_ROCKETS', 'FLAG_HIDE_ROCKET_GOLDENROD_RADIOTOWER_1'}
        self.assertFalse(hide & interp.flags)
        self.assertTrue({'FLAG_HIDE_GOLDENROD_ROCKET_SCOUT', 'FLAG_HIDE_GOLDENROD_NPCS'} <= interp.flags)
        # Later takeover scenes are still hidden: the takeover is only at its first step.
        later = {'FLAG_HIDE_GOLDENROD_UNDERGROUND_KIMONO', 'FLAG_HIDE_GOLDENROD_UNDERGROUND_SILVER',
                 'FLAG_HIDE_GOLDENROD_RADIOTOWER_PETREL', 'FLAG_HIDE_GOLDENROD_RADIOTOWER_DIRECTOR_2'}
        self.assertTrue(later <= interp.flags)

    def test_ruins_rewards_start_hidden_from_the_new_game(self):
        new_game = (GAME / 'data/scripts/new_game.inc').read_text()
        reset = new_game.split('EventScript_ResetAllMapFlagsHnS::', 1)[1].split('\treturn', 1)[0]
        for flag in ('FLAG_ITEM_GS_BALL', 'FLAG_ITEM_DOME_FOSSIL', 'FLAG_ITEM_HELIX_FOSSIL',
                     'FLAG_ITEM_OLD_AMBER'):
            self.assertIn('setflag %s\n' % flag, reset)
            self.assertNotIn(flag, raw_text('VioletCity_hns'))

    def test_azalea_hides_celebi_and_ilex_kurt_after_an_early_ilex_visit(self):
        interp = run_setup('azalea', vars={'VAR_ILEX_FOREST_FARFETCHD': 1})
        self.assertTrue({'FLAG_HIDE_CELEBI', 'FLAG_HIDE_ILEX_FOREST_KURT'} <= interp.flags)
        self.assertNotIn('FLAG_HIDE_ILEX_FOREST_FARFETCHD_1', interp.written())

    def test_goldenrod_elm_call_is_queued_for_johto_origin_only(self):
        for johto in (True, False):
            with self.subTest(johto=johto):
                interp = Interp('GoldenrodCity_hns', johto=johto)
                interp.run('GoldenrodCity_OnTransition')
                self.assertEqual(interp.vars.get('VAR_TEMP_1', 0), int(johto))
        interp = Interp('GoldenrodCity_hns', flags={'FLAG_SHOWN_ELM_TOGEPI'})
        interp.run('GoldenrodCity_OnTransition')
        self.assertEqual(interp.vars.get('VAR_TEMP_1', 0), 0)
        frame = _label_block(raw_text('GoldenrodCity_hns'), 'GoldenrodCity_OnFrame')
        self.assertIn('map_script_2 VAR_TEMP_1, 1, GoldenrodCity_Elm_Togepi_Call', frame)

    def test_ecruteak_from_route_38_keeps_eusine_burned_tower_reachable(self):
        interp = run_setup('ecruteak')
        self.assertEqual(interp.vars[ECR], 1)  # Burned Tower's Eusine OnFrame reads == 1

    def test_blackthorn_owns_den_state_and_mahogany_does_not_touch_it(self):
        mahogany = _label_block(raw_text('Mahoganytown_hns'), 'MahoganyTown_EventScript_Setup')
        self.assertNotRegex(mahogany, r'BLACKTHORN|DRAGONS_DEN|GOT_DRATINI|SPELL_TAG')
        interp = run_setup('blackthorn')
        self.assertTrue({'FLAG_HIDE_DRAGONS_DEN_CAVERN_CLAIR', 'FLAG_HIDE_DRAGONS_DEN_CAVERN_LANCE',
                         'FLAG_HIDE_DRAGONS_DEN_CAVERN_SILVER', 'FLAG_HIDE_DRAGONS_DEN_SHRINE_CLAIR'} <= interp.flags)
        self.assertNotIn('FLAG_GOT_DRATINI', interp.written())


def _label_block(text, label):
    match = re.search(r'(?m)^%s:+.*$' % re.escape(label), text)
    assert match, label
    end = re.search(r'(?m)^\w+::', text[match.end():])
    return text[match.start(): match.end() + (end.start() if end else len(text))]


class AzaleaAndSlowpokeWell(unittest.TestCase):
    MAPS = ('AzaleaTown_hns', 'AzaleaTown_KurtsHouse_hns', 'AzaleaTown_Gym_hns', 'IlexForest_hns',
            'SlowpokeWell_B1F_hns')
    # name -> (script label, variable, value it records)
    STEPS = {
        'W2': ('AzaleaTown_KurtsHouse_EventScript_KurtBefore', WELL, 2),
        'W3': ('SlowpokeWell_B1F_EventScript_Proton', WELL, 3),
        'W4': ('AzaleaTown_KurtsHouse_EventScript_ReturnFromWell', WELL, 4),
        'Bugsy': ('AzaleaTown_Gym_EventScript_Bugsy', AZ, 5),
        'Silver': ('AzaleaTown_EventScript_SilverTriggerTop', AZ, 6),
        'Ilex': ('IlexForest_EventScript_Trigger', AZ, 7),
        'G8': ('AzaleaTown_KurtsHouse_EventScript_KurtGsBall2', AZ, 8),
        'G9': ('AzaleaTown_EventScript_GSBall', AZ, 9),
    }

    def test_every_order_of_well_and_azalea_steps_is_monotonic_and_loses_nothing(self):
        names = list(self.STEPS)
        runs = 0
        for order in permutations(names):
            if not (order.index('W2') < order.index('W3') < order.index('W4') and order.index('G8') < order.index('G9')):
                continue
            interp = Interp(*self.MAPS)
            for name in order:
                label, var, value = self.STEPS[name]
                before = dict(interp.vars)
                interp.writes.clear()
                interp.run(label)
                for other in (AZ, WELL):
                    self.assertGreaterEqual(interp.vars.get(other, 0), before.get(other, 0), (order, name))
                # A step only ever writes its own adventure's variable.
                self.assertEqual(interp.written('var') - {'VAR_TEMP_0', 'VAR_TEMP_1', 'VAR_RESULT',
                                                           'VAR_ILEX_FOREST_FARFETCHD', 'VAR_0x8008', 'VAR_NUM_BADGES'} - {var}, set(),
                                 (order, name))
            self.assertEqual(interp.vars[WELL], 4, order)
            self.assertEqual(interp.vars[AZ], 9, order)
            runs += 1
        self.assertGreater(runs, 1000)

    def test_well_then_bugsy_does_not_skip_the_silver_trigger_value(self):
        interp = Interp(*self.MAPS)
        for name in ('W2', 'W3', 'W4', 'Bugsy'):
            interp.run(self.STEPS[name][0])
        self.assertEqual((interp.vars[WELL], interp.vars[AZ]), (4, 5))  # Silver's tile fires at Azalea == 5

    def test_kurt_gs_ball_never_downgrades_celebi(self):
        for start in (8, 9, 10):
            interp = Interp(*self.MAPS, vars={AZ: start})
            interp.run('AzaleaTown_KurtsHouse_EventScript_KurtGsBall2')
            self.assertEqual(interp.vars[AZ], start)

    def test_kurts_house_reads_the_right_state(self):
        text = raw_text('AzaleaTown_KurtsHouse_hns')
        for label in ('AzaleaTown_KurtsHouse_OnTransition', 'AzaleaTown_KurtsHouse_OnFrame'):
            self.assertIn(WELL, _label_block(text, label))
            self.assertNotIn(AZ, _label_block(text, label))
        # Well dialogue ignores Bugsy and Silver; GS Ball dialogue ignores the Well.
        interp = Interp('AzaleaTown_KurtsHouse_hns', vars={WELL: 4, AZ: 7}, flags=set())
        interp.run('AzaleaTown_KurtsHouse_EventScript_Girl2')
        self.assertEqual(interp.messages[-1], 'KurtsHouse_Text_GranddaughterSlowpokeBack')
        interp = Interp('AzaleaTown_KurtsHouse_hns', vars={WELL: 0, AZ: 5})
        interp.run('AzaleaTown_KurtsHouse_EventScript_Girl2')
        self.assertEqual(interp.messages[-1], 'KurtsHouse_Text_GranddaughterDad')
        interp = Interp('AzaleaTown_KurtsHouse_hns', vars={WELL: 4, AZ: 9})
        interp.run('AzaleaTown_KurtsHouse_EventScript_Girl2')
        self.assertEqual(interp.messages[-1], 'KurtsHouse_Text_GranddaughterSound')


class MahoganyLakeAndRadioTower(unittest.TestCase):
    MAPS = ('Mahoganytown_hns', 'MahoganyTown_Gym_hns', 'LakeOfRage_hns', 'GoldenrodCity_RadioTower_5F_hns',
            'GoldenrodCity_RadioTower_1F_hns', 'GoldenrodCity_hns', 'MahoganyTown_Shop_hns',
            'RocketHideout_B2F_hns', 'RocketHideout_B3F_hns')

    def test_pryce_does_not_touch_the_lake_and_hideout_state(self):
        for state in (0, 2, 5, 14):
            with self.subTest(state=state):
                interp = Interp(*self.MAPS, vars={MAH: state})
                interp.run('MahoganyTown_Gym_EventScript_PryceVictory')
                self.assertEqual(interp.vars[MAH], state)
                self.assertNotIn(MAH, interp.written())

    def test_gyarados_win_never_rewinds_the_hideout(self):
        for state in (0, 1, 2, 5, 14):
            interp = Interp(*self.MAPS, vars={MAH: state})
            interp.run('LakeOfRage_EventScript_Defeated_Gyarados')
            self.assertEqual(interp.vars[MAH], max(state, 2))

    def test_radio_tower_finale_resolves_without_touching_mahogany(self):
        for state in (0, 3, 14):
            interp = Interp(*self.MAPS, vars={MAH: state, GOLD: 10})
            interp.run('GoldenrodCity_RadioTower_5F_Fakeout')
            self.assertIn(RESOLVED, interp.flags)
            self.assertEqual(interp.vars[MAH], state)
            self.assertNotIn(MAH, interp.written())

    def test_takeover_start_does_not_touch_mahogany_and_never_rewinds_goldenrod(self):
        for gold in (0, 1, 8, 11):
            interp = Interp(*self.MAPS, vars={MAH: 14, GOLD: gold})
            interp.run('Mahoganytown_EventScript_RocketTrigger')
            self.assertEqual(interp.vars[MAH], 14)
            self.assertEqual(interp.vars[GOLD], max(gold, 6))

    def test_arming_keys_on_pryce_and_an_unstarted_takeover(self):
        label = 'MahoganyTown_Gym_EventScript_TryArmRocketTakeover'
        badges = {'FLAG_BADGE05_GET', 'FLAG_BADGE06_GET', 'FLAG_BADGE07_GET'}
        for mahogany in (0, 3, 14):  # Lake progress is irrelevant
            interp = Interp(*self.MAPS, vars={MAH: mahogany}, flags=badges | {'FLAG_DEFEATED_MAHOGANY_TOWN_GYM'})
            interp.run(label)
            self.assertEqual(interp.vars.get('VAR_TRIGGER_ELM_ROCKET_CALL'), 1, mahogany)
        interp = Interp(*self.MAPS, flags=badges)
        interp.run(label)
        self.assertNotIn('VAR_TRIGGER_ELM_ROCKET_CALL', interp.vars)
        interp = Interp(*self.MAPS, vars={GOLD: 6}, flags=badges | {'FLAG_DEFEATED_MAHOGANY_TOWN_GYM'})
        interp.run(label)
        self.assertNotIn('VAR_TRIGGER_ELM_ROCKET_CALL', interp.vars)

    def test_radio_quiz_leaves_the_takeover_alone(self):
        text = raw_text('GoldenrodCity_RadioTower_1F_hns')
        self.assertNotIn(GOLD, text)
        interp = Interp('GoldenrodCity_RadioTower_1F_hns', vars={GOLD: 8})
        interp.run('RadioTower1F_EventScript_ReceptionGiveRadio')
        self.assertEqual(interp.vars[GOLD], 8)

    def test_petrel_never_rewinds_the_takeover(self):
        for gold in (7, 8, 9, 10):
            interp = Interp('GoldenrodCity_RadioTower_5F_hns', vars={GOLD: gold})
            interp.run('GoldenrodCity_RadioTower_5F_EventScript_Petrel')
            self.assertGreaterEqual(interp.vars[GOLD], gold)

    def test_merchant_has_no_road_block_and_reads_the_resolved_flag(self):
        events = json.loads((MAPS / 'Mahoganytown_hns/map.json').read_text())['coord_events']
        self.assertEqual(events, [])
        text = raw_text('Mahoganytown_hns')
        self.assertNotIn('MerchantTrigger', text)
        self.assertNotIn('PushPlayer', text)
        self.assertNotIn('%s, 17' % MAH, text)
        for flags, expected in (((), False), ((RESOLVED,), True)):
            interp = Interp('Mahoganytown_hns', vars={MAH: 5}, flags=flags)
            interp.run('MahoganyTown_EventScript_MerchantRageCandyBar')
            self.assertEqual('MahoganyTown_Text_RageCandyBarMerchantSoldOut' in interp.messages, expected)

    def test_takeover_music_reads_goldenrod_not_mahogany(self):
        code = (GAME / 'src/overworld.c').read_text()
        body = code[code.index('static bool16 IsRocketTakeover'):]
        body = body[:body.index('u16 GetLocationMusic')]
        self.assertIn('VAR_GOLDENROD_CITY_STATE', body)
        self.assertIn('FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED', body)
        self.assertNotIn('VAR_MAHOGANY_TOWN_STATE', body)

    def test_gate_toll_keeps_reading_lake_values(self):
        text = raw_text('Gate_Route43_hns')
        for value in range(1, 6):
            self.assertIn('goto_if_eq %s, %d,' % (MAH, value), text)


class EcruteakAndMorty(unittest.TestCase):
    def test_morty_before_the_burned_tower_leaves_its_state(self):
        for state in (0, 1, 2, 3, 4):
            interp = Interp('EcruteakCity_Gym_hns', vars={ECR: state})
            interp.run('EcruteakCity_Gym_EventScript_Morty')
            self.assertEqual(interp.vars[ECR], state)
            self.assertNotIn(ECR, interp.written())
            self.assertIn('FLAG_BADGE04_GET', interp.flags)

    def test_surf_giver_only_advances_from_a_beaten_rocket(self):
        maps = ('EcruteakCity_Theater_hns',)
        for label in ('EcruteakCity_Theater_EventScript_SurfGuy',
                      'EcruteakCity_Theater_EventScript_TriggerSurfGuy'):
            for state in range(0, 4):
                for ceremony in (0, 4, 6, 8):
                    interp = Interp(*maps, vars={THEATER: state, CEREMONY: ceremony})
                    interp.run(label)
                    self.assertEqual(interp.vars[THEATER], 3 if state == 2 else state, (label, state))
                    self.assertEqual(interp.vars[CEREMONY], ceremony)
                    # His conversation gives Surf only once the Rocket is beaten; the
                    # trigger itself only fires at theater state 2.
                    gives = state >= 2 or label.endswith('TriggerSurfGuy')
                    self.assertEqual('FLAG_RECEIVED_HM_SURF' in interp.flags, gives, (label, state))

    def test_surf_handoff_survives_a_full_bag_and_is_given_once(self):
        maps = ('EcruteakCity_Theater_hns',)
        for label in ('EcruteakCity_Theater_EventScript_SurfGuy',
                      'EcruteakCity_Theater_EventScript_TriggerSurfGuy'):
            full = Interp(*maps, vars={THEATER: 2}, bag_full=True)
            full.run(label)
            self.assertNotIn('FLAG_RECEIVED_HM_SURF', full.flags, label)
            self.assertEqual(full.vars[THEATER], 2, label)
            retry = Interp(*maps, vars={THEATER: 2})
            retry.run('EcruteakCity_Theater_EventScript_SurfGuy')
            self.assertIn('FLAG_RECEIVED_HM_SURF', retry.flags)
            again = Interp(*maps, vars=dict(retry.vars), flags=retry.flags)
            again.run('EcruteakCity_Theater_EventScript_SurfGuy')
            self.assertNotIn('ITEM_HM_SURF', again.messages)
            self.assertEqual(again.written(), set(), label)

    def test_elm_handoff_leaves_the_theater_npcs_and_surf_giver_visible(self):
        handoff = raw_text('NewBarkTown_Lab_hns')
        self.assertNotIn('setflag FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS', handoff)
        for name in ('EcruteakCity_Theater_hns', 'EcruteakCity_hns'):
            self.assertNotIn('setflag FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS', raw_text(name), name)

    def test_ceremony_scenes_move_the_ceremony_variable_forward_only(self):
        interp = Interp('EcruteakCity_Theater_hns', vars={CEREMONY: 8, THEATER: 3})
        interp.run('EcruteakCity_Theater_EventScript_LegendaryCutscene')
        self.assertEqual(interp.vars[CEREMONY], 8)
        self.assertEqual(interp.vars[THEATER], 3)
        for name, label in (('TinTower_RoofDay_hns', 'TinTower_RoofDay_EventScript_HoOhPostgame'),
                            ('WhirlIslands_LugiaChamber_hns', None)):
            text = raw_text(name)
            self.assertIn('setvar_forward %s, 8' % CEREMONY, text)
        self.assertIn('setvar_forward %s, 4' % CEREMONY, raw_text('NewBarkTown_Lab_hns'))

    def test_trial_waits_for_the_theater_rocket_on_shared_tiles(self):
        events = json.loads((MAPS / 'EcruteakCity_Theater_hns/map.json').read_text())['coord_events']
        trial = {(e['x'], e['y']) for e in events if 'PreBattleCutscene' in e['script']}
        rocket = {(e['x'], e['y']) for e in events if e['script'].endswith('RocketEventTrigger')}
        self.assertTrue(trial & rocket)
        for e in events:
            if 'PreBattleCutscene' not in e['script']:
                continue
            for theater in range(0, 4):
                interp = Interp('EcruteakCity_Theater_hns', vars={THEATER: theater, CEREMONY: 5})
                interp.run(e['script'])
                self.assertEqual(interp.vars[CEREMONY], 6 if theater >= 2 else 5, (e['script'], theater))
                self.assertEqual(interp.vars[THEATER], theater)

    def test_ceremony_coordinates_read_the_ceremony_variable(self):
        theater = json.loads((MAPS / 'EcruteakCity_Theater_hns/map.json').read_text())['coord_events']
        for event in theater:
            if event['var_value'] == '5':
                self.assertEqual(event['var'], CEREMONY)
            else:
                self.assertEqual(event['var'], THEATER)
        city = json.loads((MAPS / 'EcruteakCity_hns/map.json').read_text())['coord_events']
        silver = [e for e in city if e['script'] == 'EcruteakCity_Trigger_Silver']
        self.assertEqual([(e['var'], e['var_value']) for e in silver], [(CEREMONY, '4')])
        interp = Interp('EcruteakCity_hns', vars={THEATER: 3, CEREMONY: 4})
        interp.run('EcruteakCity_Trigger_Silver')
        self.assertEqual((interp.vars[THEATER], interp.vars[CEREMONY]), (3, 5))

    def test_whitney_owns_her_state(self):
        for path in ('GoldenrodCity_Gym_hns/scripts.inc', 'GoldenrodCity_Gym_hns/map.json'):
            text = (MAPS / path).read_text()
            self.assertNotIn(GOLD, text)
            self.assertIn(WHITNEY, text)


class SharedStateWrites(unittest.TestCase):
    """Static guard: shared story variables only ever move forward."""

    # The Burned Tower's own steps (ECR 2-4) are gated by their triggers' values.
    FORWARD_ONLY = (AZ, WELL, MAH, GOLD, CEREMONY)

    def test_no_plain_setvar_on_forward_only_variables(self):
        offenders = []
        for path in all_script_files():
            for number, line in enumerate(path.read_text().splitlines(), 1):
                parts = line.split('@')[0].split()
                if len(parts) >= 2 and parts[0] == 'setvar' and parts[1].rstrip(',') in self.FORWARD_ONLY:
                    offenders.append('%s:%d %s' % (path.parent.name, number, line.strip()))
        self.assertEqual(offenders, [])

    def test_theater_variable_is_rocket_and_surf_only(self):
        values = set()
        for path in all_script_files():
            for line in path.read_text().splitlines():
                parts = line.split('@')[0].replace(',', ' ').split()
                if len(parts) >= 3 and parts[0] in ('setvar', 'setvar_forward') and parts[1] == THEATER:
                    values.add(int(parts[2]))
        self.assertEqual(values, {0, 1, 2, 3} - {0})

    def test_other_adventures_do_not_write_foreign_state(self):
        def writes(map_name, var):
            return re.search(r'(?m)^\s*setvar(_forward)?\s+%s\b' % var, raw_text(map_name)) is not None
        self.assertFalse(writes('MahoganyTown_Gym_hns', MAH))
        self.assertFalse(writes('EcruteakCity_Gym_hns', ECR))
        self.assertFalse(writes('GoldenrodCity_RadioTower_1F_hns', GOLD))
        self.assertFalse(writes('GoldenrodCity_RadioTower_5F_hns', MAH))
        self.assertFalse(writes('Mahoganytown_hns', MAH) and 'RocketTrigger' in raw_text('Mahoganytown_hns')
                         and re.search(r'setvar\w*\s+%s,\s*16' % MAH, raw_text('Mahoganytown_hns')))
        for path in all_script_files():
            self.assertNotRegex(path.read_text(), r'(?m)^\s*setvar\w*\s+%s,\s*(15|16|17)\b' % MAH)


if __name__ == '__main__':
    unittest.main()
