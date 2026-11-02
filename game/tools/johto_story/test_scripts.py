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
CHAPTER = 'VAR_WAYFARER_JOHTO_SILVER_CHAPTER'
WON_ANY = 'FLAG_WAYFARER_JOHTO_SILVER_WON_ANY'
INDIGO_ENTERED = 'FLAG_WAYFARER_JOHTO_INDIGO_ENTERED'
SEED = 'FLAG_WAYFARER_JOHTO_MIRACLE_SEED_RECEIVED'
LAB = 'VAR_NEWBARKTOWN_LABSTATE'
BALL_OWED = 'FLAG_WAYFARER_JOHTO_ELM_MASTER_BALL_OWED'
BALL_GOT = 'FLAG_WAYFARER_JOHTO_ELM_MASTER_BALL_RECEIVED'
PEWTER_GIVEN = 'FLAG_WAYFARER_JOHTO_PEWTER_WING_GIVEN'
LUGIA_P = 'VAR_COMPLETED_LUGIA'
HOOH_P = 'VAR_COMPLETED_HO_OH'
BRANCH = 'VAR_LUGIA_OR_HOOH'
# The shared legendary threshold, read from its single source (the v0 scale is the Wayfarer build's).
_tr = re.search(r'#if WAYFARER_V0_TRAINERS\n#define WAYFARER_LEGENDARY_TR (\d+)',
                (GAME / 'include/wayfarer_sevii_story.h').read_text())
LEGENDARY_TR = int(_tr.group(1))
WINGS = {1: 'ITEM_SILVER_WING', 2: 'ITEM_RAINBOW_WING'}
# Ceremony values (include/constants/wayfarer_johto_story.h).
STARTED, DONE, RESOLVED_CEREMONY = 1, 2, 3


def won(chapter):
    return 'FLAG_WAYFARER_JOHTO_SILVER_WON_%d' % chapter


def played(chapter):
    return 'FLAG_WAYFARER_JOHTO_SILVER_PLAYED_%d' % chapter


def silver_playable(chapter, johto, started, flags):
    """The spec's Silver table, restated: whether a chapter's scene can play."""
    if not johto or chapter not in range(1, 13):
        return False
    return {
        1: started == 0,
        2: started <= 2 and won(2) not in flags,
        3: started < 4,
        4: started <= 4 and won(4) not in flags,
        5: started <= 5 and won(5) not in flags,
        6: started < 8 and played(6) not in flags,
        7: started < 8 and played(7) not in flags,
        8: started <= 8 and won(8) not in flags,
        9: RESOLVED in flags and started < 10 and played(9) not in flags,
        10: started <= 10 and won(10) not in flags,
        11: started in (10, 11) and won(11) not in flags,
        12: won(11) in flags,
    }[chapter]

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

# Ilex Forest's errand and Celebi objects: hidden by the new game, revealed by their own steps.
ILEX_ERRAND_FLAGS = {
    'FLAG_HIDE_ILEX_FOREST_APPRENTICE', 'FLAG_HIDE_ILEX_FOREST_MASTER', 'FLAG_HIDE_ILEX_FOREST_FARFETCHD_1',
    'FLAG_HIDE_ILEX_FOREST_FARFETCHD_2', 'FLAG_HIDE_ILEX_FOREST_FARFETCHD_3', 'FLAG_HIDE_ILEX_FOREST_FARFETCHD_4',
    'FLAG_HIDE_CELEBI', 'FLAG_HIDE_ILEX_FOREST_KURT',
}

# State and rewards each town's setup must never write.
PROTECTED = {
    'violet': {'FLAG_RECEIVED_TOGEPI_EGG', 'VAR_SPROUT_TOWER', 'FLAG_HIDE_SPROUT_TOWER_SILVER',
               'VAR_RUINSOFALPH_STATE', 'FLAG_VIOLET_NPC_TRADE_COMPLETED',
               # Ruins puzzle rewards revealed by puzzles solved before the first visit.
               'FLAG_ITEM_GS_BALL', 'FLAG_ITEM_DOME_FOSSIL', 'FLAG_ITEM_HELIX_FOSSIL',
               'FLAG_ITEM_OLD_AMBER'},
    'azalea': {'FLAG_GOT_CHARCOAL', 'FLAG_HIDE_AZALEA_TOWN_ROCKETS', 'FLAG_HIDE_KURT_1',
               'FLAG_HIDE_AZALEA_TOWN_FARFETCHD', 'FLAG_HIDE_AZALEA_TOWN_CUT_MASTER'} | ILEX_ERRAND_FLAGS,
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
        self.items = None       # None: every checked item is held; otherwise the held items
        self.tr = 0
        self.outcome = 'WON'    # what GetBattleOutcome reports
        self.answers = list(answers) if isinstance(answers, list) else None
        if self.answers:
            self.answer = self.answers[-1]
        self.battle = 'win'     # 'win' resumes after a trainerbattle; 'lose' never does
        self.starter_slot = 3
        self.writes = []      # (name, value, kind) in order
        self.messages = []
        self.battles = []     # (command, stored Silver chapter at that moment)
        self.given = []
        self.pokenav = []
        self.locked = False

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
                if args[1] == 'WayfarerJohtoSilverAvailable':
                    self.vars[args[0]] = int(self.silver_playable())
                elif args[1] == 'GetBattleOutcome':
                    self.vars[args[0]] = self.get('B_OUTCOME_' + self.outcome)
                elif args[1] == 'WayfarerJohtoKimonoMissing':
                    self.vars[args[0]] = self.kimono_missing()
                elif args[1] == 'WayfarerJohtoKimonoTrialAvailable':
                    self.vars[args[0]] = int(self.kimono_missing() == 0 and self.vars.get(CEREMONY, 0) < DONE)
                elif args[1] == 'WayfarerJohtoPewterWingMissing':
                    main = self.vars.get(LUGIA_P if self.vars.get(BRANCH, 0) == 1 else HOOH_P, 0)
                    self.vars[args[0]] = (main != 3) | (2 if self.tr < LEGENDARY_TR else 0)
                else:
                    self.vars[args[0]] = {'WayfarerIsJohtoOrigin': int(self.johto),
                                          # ORIGIN_SCENE_NATIVE (1) for Johto, ORIGIN_SCENE_VISITOR (0) otherwise.
                                          'WayfarerDispatchOriginScene': int(self.johto),
                                          'WayfarerGetOriginStarterSlot': self.starter_slot}.get(args[1], 0)
            elif command == 'msgbox':
                self.messages.append(args[0])
                if args[-1] == 'MSGBOX_YESNO':
                    self.vars['VAR_RESULT'] = self.answers.pop(0) if self.answers else self.answer
            elif command in ('lock', 'lockall'):
                self.locked = True
            elif command in ('release', 'releaseall'):
                self.locked = False
            elif command == 'pokenavcall':
                self.pokenav.append(args[0])
            elif command.startswith('trainerbattle'):
                self.battles.append((command, self.vars.get(CHAPTER, 0)))
                if self.battle == 'lose':
                    return
            elif command == 'giveitem':
                self.vars['VAR_RESULT'] = 0 if self.bag_full else 1
                if not self.bag_full:
                    self.given.append(args[0])
                    if self.items is not None:
                        self.items.add(args[0])
            elif command == 'checkitemspace':
                self.vars['VAR_RESULT'] = 0 if self.bag_full else 1
            elif command == 'checkitem':
                self.vars['VAR_RESULT'] = 1 if self.items is None else int(args[0] in self.items)
            elif command == 'checkmoney':
                self.vars['VAR_RESULT'] = 1
            elif command == 'removemoney':
                self.set_var('VAR_MONEY_SPENT', self.get('VAR_MONEY_SPENT') + self.get(args[0]))
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

    def kimono_missing(self):
        wing = WINGS.get(self.vars.get(BRANCH, 0))
        held = wing is not None and (self.items is None or wing in self.items)
        return (0 if held and RESOLVED in self.flags else 1) | (2 if self.tr < LEGENDARY_TR else 0)

    def silver_playable(self):
        chapter = self.get('VAR_0x8004')
        return silver_playable(chapter, self.johto, self.vars.get(CHAPTER, 0), self.flags)

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
                self.assertEqual([w for w in interp.writes if w[0] not in ('VAR_0x8004', 'VAR_RESULT')]
                                 if town != 'blackthorn' else [], [])

    def test_setup_leaves_progress_made_before_the_first_visit(self):
        cases = {
            # (town, preset vars, preset flags): names that must be untouched
            'violet': ({'VAR_SPROUT_TOWER': 3, 'VAR_RUINSOFALPH_STATE': 4, 'VAR_VIOLET_CITY_STATE': 4},
                       {'FLAG_RECEIVED_TOGEPI_EGG', 'FLAG_HIDE_SPROUT_TOWER_SILVER'}),
            'azalea': ({AZ: 9, WELL: 4, 'VAR_ILEX_FOREST_FARFETCHD': 3}, {'FLAG_GOT_CHARCOAL'}),
            'goldenrod': ({GOLD: 9, 'VAR_LUGIA_OR_HOOH': 2, WHITNEY: 5},
                          {'FLAG_RECEIVED_BIKE', 'FLAG_GOT_EEVEE', 'FLAG_SYS_PC_BILL', 'FLAG_SHOWN_ELM_TOGEPI'}),
            'ecruteak': ({THEATER: 3, CEREMONY: 2, ECR: 4, 'VAR_COMPLETED_HO_OH': 3, 'VAR_SUICUNE_ENCOUNTERS': 3},
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
        interp = Interp('GoldenrodCity_hns')
        interp.run('GoldenrodCity_EventScript_StartTakeover')
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
        reset = new_game.split('EventScript_ResetAllMapFlagsHnS::', 1)[1].split('\n\n', 1)[0]
        for flag in ('FLAG_ITEM_GS_BALL', 'FLAG_ITEM_DOME_FOSSIL', 'FLAG_ITEM_HELIX_FOSSIL',
                     'FLAG_ITEM_OLD_AMBER'):
            self.assertIn('setflag %s\n' % flag, reset)
            self.assertNotIn(flag, raw_text('VioletCity_hns'))

    def test_kimono_and_shadow_objects_start_hidden_from_the_new_game_not_from_ecruteak(self):
        new_game = (GAME / 'data/scripts/new_game.inc').read_text()
        reset = new_game.split('EventScript_ResetAllMapFlagsHnS::', 1)[1].split('\n\n', 1)[0]
        for flag in ('FLAG_HIDE_ROUTE39_SHADOW', 'FLAG_HIDE_WHIRL_ISLANDS_KIMONO_GIRLS',
                     'FLAG_HIDE_TIN_TOWER_KIMONO_GIRLS', 'FLAG_HIDE_ECRUTEAK_CITY_THEATER_KIMONOS'):
            self.assertIn('setflag %s\n' % flag, reset)
            # Ecruteak's setup neither hides them nor un-hides them, whatever progress exists.
            interp = run_setup('ecruteak', vars={CEREMONY: 2})
            self.assertNotIn(flag, interp.written())
        # What reveals them clears the flag first.
        route39 = Interp('Route39_hns')
        route39.run('Route39_EventScript_LegendaryTrigger')
        self.assertEqual([n for n, v, k in route39.writes if n == 'FLAG_HIDE_ROUTE39_SHADOW'], ['FLAG_HIDE_ROUTE39_SHADOW'] * 2)
        self.assertEqual([v for n, v, k in route39.writes if n == 'FLAG_HIDE_ROUTE39_SHADOW'], [0, 1])
        for branch, flag in ((1, 'FLAG_HIDE_WHIRL_ISLANDS_KIMONO_GIRLS'), (2, 'FLAG_HIDE_TIN_TOWER_KIMONO_GIRLS')):
            ceremony = Interp('EcruteakCity_Theater_hns', vars={BRANCH: branch})
            ceremony.run('EcruteakCity_Theater_EventScript_BeatGauntlet')
            self.assertIn(flag, ceremony.written('flag'))
            self.assertNotIn(flag, ceremony.flags)

    def test_ilex_errand_objects_start_hidden_from_the_new_game(self):
        # Ilex is reachable from Route 34 before Azalea's first load.
        new_game = (GAME / 'data/scripts/new_game.inc').read_text()
        reset = new_game.split('EventScript_ResetAllMapFlagsHnS::', 1)[1].split('\n\n', 1)[0]
        for flag in sorted(ILEX_ERRAND_FLAGS):
            self.assertIn('setflag %s\n' % flag, reset)
        interp = run_setup('azalea')
        self.assertFalse(ILEX_ERRAND_FLAGS & interp.written())

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
                self.assertEqual(interp.written('var') - {'VAR_TEMP_0', 'VAR_TEMP_1', 'VAR_TEMP_2', 'VAR_RESULT',
                                                           'VAR_ILEX_FOREST_FARFETCHD', 'VAR_0x8008', 'VAR_NUM_BADGES',
                                                           'VAR_0x8004', CHAPTER} - {var}, set(),
                                 (order, name))
            self.assertEqual(interp.vars[WELL], 4, order)
            self.assertEqual(interp.vars[AZ], 9, order)
            runs += 1
        self.assertGreater(runs, 1000)

    def test_well_then_bugsy_then_silver_walks_the_azalea_steps_in_order(self):
        interp = Interp(*self.MAPS)
        for name in ('W2', 'W3', 'W4', 'Bugsy'):
            interp.run(self.STEPS[name][0])
        self.assertEqual((interp.vars[WELL], interp.vars[AZ]), (4, 5))
        # Silver's Azalea scene follows Bugsy's win whatever the Well or later Azalea steps wrote.
        interp.run('AzaleaTown_EventScript_SilverTriggerTop')
        self.assertEqual(interp.vars[AZ], 6)
        self.assertEqual(interp.vars[CHAPTER], 4)

    def test_kurt_gs_ball_never_downgrades_a_running_chain(self):
        for start in (8, 9):
            interp = Interp(*self.MAPS, vars={AZ: start})
            interp.run('AzaleaTown_KurtsHouse_EventScript_KurtGsBall2')
            self.assertEqual(interp.vars[AZ], start)
        for start in (1, 5, 6, 7):
            interp = Interp(*self.MAPS, vars={AZ: start})
            interp.run('AzaleaTown_KurtsHouse_EventScript_KurtGsBall2')
            self.assertEqual(interp.vars[AZ], 8)

    def test_gs_ball_restarts_after_celebi_escapes(self):
        # Ilex Kurt writes 10 after the shrine whether or not Celebi was caught; with
        # the GS Ball still in the Bag, Kurt restarts the chain so Celebi isn't lost.
        interp = Interp(*self.MAPS, vars={AZ: 10})
        interp.run('AzaleaTown_KurtsHouse_EventScript_KurtGsBall2')
        self.assertEqual(interp.vars[AZ], 8)

    def test_fast_ball_survives_a_full_bag_without_replaying_every_frame(self):
        interp = Interp(*self.MAPS, vars={WELL: 3}, bag_full=True)
        interp.run('AzaleaTown_KurtsHouse_OnTransition')
        self.assertEqual(interp.vars['VAR_TEMP_2'], 1)
        interp.run('AzaleaTown_KurtsHouse_EventScript_ReturnFromWell')
        self.assertEqual(interp.vars[WELL], 3)       # still owed
        self.assertEqual(interp.vars['VAR_TEMP_2'], 0)  # OnFrame disarmed for this visit
        retry = Interp(*self.MAPS, vars={WELL: 3})
        retry.run('AzaleaTown_KurtsHouse_OnTransition')
        retry.run('AzaleaTown_KurtsHouse_EventScript_ReturnFromWell')
        self.assertEqual(retry.vars[WELL], 4)

    def test_bugsy_reveals_the_ilex_errand(self):
        interp = Interp(*self.MAPS)
        interp.run('AzaleaTown_Gym_EventScript_Bugsy')
        self.assertIn('FLAG_HIDE_AZALEA_TOWN_FARFETCHD', interp.flags)
        self.assertNotIn('FLAG_HIDE_ILEX_FOREST_APPRENTICE', interp.flags)
        self.assertNotIn('FLAG_HIDE_ILEX_FOREST_FARFETCHD_1', interp.flags)
        self.assertNotIn('VAR_ILEX_FOREST_FARFETCHD', _label_block(raw_text('AzaleaTown_Gym_hns'),
                                                                  'AzaleaTown_Gym_EventScript_Bugsy'))

    def test_hm_cut_survives_a_full_bag_and_stays_retryable(self):
        for label in ('IlexForest_EventScript_Trigger', 'IlexForest_EventScript_TriggerBot'):
            full = Interp(*self.MAPS, vars={'VAR_ILEX_FOREST_FARFETCHD': 2}, bag_full=True)
            full.run(label)
            self.assertEqual(full.vars['VAR_ILEX_FOREST_FARFETCHD'], 2, label)  # the trigger tiles fire again
            self.assertNotIn('FLAG_RECEIVED_HM_CUT', full.flags)
            self.assertNotIn(AZ, full.vars)
            self.assertIn('IlexForest_EventScript_CutBagFull', raw_text('IlexForest_hns'))
            retry = Interp(*self.MAPS, vars=dict(full.vars))
            retry.run(label)
            self.assertEqual(retry.vars['VAR_ILEX_FOREST_FARFETCHD'], 3, label)
            self.assertIn('FLAG_RECEIVED_HM_CUT', retry.flags)

    def test_gs_ball_survives_a_full_bag_without_replaying_every_frame(self):
        transition = _label_block(raw_text('AzaleaTown_hns'), 'AzaleaTown_OnTransition')
        self.assertNotIn('VAR_TEMP_1', _label_block(raw_text('AzaleaTown_hns'), 'AzaleaTown_OnFrame').replace(
            'map_script_2 VAR_TEMP_1, 1, AzaleaTown_EventScript_GSBall', ''))
        for state in (1, 7, 9, 10):
            interp = Interp(*self.MAPS, vars={AZ: state}, flags={SETUP_FLAGS['azalea']})
            interp.run('AzaleaTown_OnTransition')
            self.assertEqual(interp.vars.get('VAR_TEMP_1', 0), 0, state)
        full = Interp(*self.MAPS, vars={AZ: 8}, flags={SETUP_FLAGS['azalea']}, bag_full=True)
        full.run('AzaleaTown_OnTransition')
        self.assertEqual(full.vars['VAR_TEMP_1'], 1)
        full.run('AzaleaTown_EventScript_GSBall')
        self.assertEqual(full.vars[AZ], 8)             # still owed
        self.assertEqual(full.vars['VAR_TEMP_1'], 0)   # OnFrame disarmed for this visit
        self.assertIn('FLAG_HIDE_AZALEA_TOWN_KURT', full.flags)
        retry = Interp(*self.MAPS, vars=dict(full.vars), flags={SETUP_FLAGS['azalea']})
        retry.run('AzaleaTown_OnTransition')
        self.assertEqual(retry.vars['VAR_TEMP_1'], 1)
        retry.run('AzaleaTown_EventScript_GSBall')
        self.assertEqual(retry.vars[AZ], 9)

    def test_gs_ball_before_ilex_does_not_give_the_ilex_reward(self):
        maps = ('AzaleaTown_House1_hns',)
        for az in (5, 6, 7, 8, 9, 10):
            interp = Interp(*maps, vars={AZ: az, 'VAR_ILEX_FOREST_FARFETCHD': 0}, flags={'FLAG_HIDE_AZALEA_TOWN_ROCKETS'})
            interp.run('AzaleaTown_House1_EventScript_Apprentice')
            self.assertNotIn('FLAG_GOT_CHARCOAL', interp.flags, az)
            self.assertNotIn('CharcoalKiln_Text_Apprentice3', interp.messages, az)
        done = Interp(*maps, vars={AZ: 5, 'VAR_ILEX_FOREST_FARFETCHD': 3}, flags={'FLAG_HIDE_AZALEA_TOWN_ROCKETS'})
        done.run('AzaleaTown_House1_EventScript_Apprentice')
        self.assertIn('FLAG_GOT_CHARCOAL', done.flags)

    def test_boss_lines_follow_the_facts_not_the_azalea_step(self):
        rockets, bugsy = 'FLAG_HIDE_AZALEA_TOWN_ROCKETS', 'FLAG_DEFEATED_AZALEA_TOWN_GYM'
        cases = (
            # (flags, Ilex var) -> line
            (set(), 0, 'CharcoalKiln_Text_Boss1'),
            ({rockets}, 0, 'CharcoalKiln_Text_Boss2'),
            ({rockets, bugsy}, 0, 'CharcoalKiln_Text_Boss3'),
            ({rockets, bugsy}, 3, 'CharcoalKiln_Text_Boss4'),
            ({rockets}, 3, 'CharcoalKiln_Text_Boss4'),
        )
        for flags, ilex, line in cases:
            for az in (1, 5, 6, 7, 8, 9, 10):  # the GS Ball chain can put Azalea anywhere
                interp = Interp('AzaleaTown_House1_hns', vars={AZ: az, 'VAR_ILEX_FOREST_FARFETCHD': ilex},
                                flags=flags)
                interp.run('AzaleaTown_House1_EventScript_Boss')
                self.assertEqual(interp.messages[-1], line, (flags, ilex, az))
        self.assertNotIn(AZ, raw_text('AzaleaTown_House1_hns'))

    def test_kurts_house_reads_the_right_state(self):
        text = raw_text('AzaleaTown_KurtsHouse_hns')
        self.assertIn(WELL, _label_block(text, 'AzaleaTown_KurtsHouse_OnTransition'))
        self.assertIn('VAR_TEMP_2', _label_block(text, 'AzaleaTown_KurtsHouse_OnFrame'))
        for label in ('AzaleaTown_KurtsHouse_OnTransition', 'AzaleaTown_KurtsHouse_OnFrame'):
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


class SuicuneChase(unittest.TestCase):
    MAPS = ('Route42_hns', 'CianwoodCity_hns', 'OlivineCity_hns')

    def test_route_42_sighting_waits_for_cianwood(self):
        events = json.loads((MAPS / 'Route42_hns/map.json').read_text())['coord_events']
        trigger = [e for e in events if e['script'] == 'Route42_EventScript_Suicune_Trigger']
        self.assertEqual([(e['var'], e['var_value']) for e in trigger], [('VAR_TEMP_0', '0')])
        for encounters in range(0, 10):
            with self.subTest(encounters=encounters):
                interp = Interp(*self.MAPS, vars={'VAR_SUICUNE_ENCOUNTERS': encounters})
                interp.run('Route42_MapScript_OnTransition')
                here = encounters == 2
                # Suicune is on the road, and its scene armed, only at its own step.
                self.assertEqual('FLAG_HIDE_ROUTE42_SUICUNE' in interp.flags, not here)
                self.assertEqual(interp.vars.get('VAR_TEMP_0', 0), 0 if here else 1)
                self.assertEqual(interp.vars['VAR_SUICUNE_ENCOUNTERS'], encounters)

    def test_visiting_route_42_early_does_not_stick_the_chase(self):
        interp = Interp(*self.MAPS)
        interp.run('OlivineCity_EventScript_Setup')
        self.assertEqual(interp.vars['VAR_SUICUNE_ENCOUNTERS'], 1)
        interp.run('Route42_MapScript_OnTransition')       # Mahogany side first
        self.assertEqual(interp.vars['VAR_TEMP_0'], 1)
        interp.vars.pop('VAR_TEMP_0')
        interp.run('CianwoodCity_EventScript_SuicuneTrigger')
        self.assertEqual(interp.vars['VAR_SUICUNE_ENCOUNTERS'], 2)
        interp.run('Route42_MapScript_OnTransition')       # now it is Route 42's turn
        self.assertNotIn('FLAG_HIDE_ROUTE42_SUICUNE', interp.flags)
        self.assertEqual(interp.vars.get('VAR_TEMP_0', 0), 0)
        interp.run('Route42_EventScript_Suicune_Trigger')
        self.assertEqual(interp.vars['VAR_SUICUNE_ENCOUNTERS'], 3)


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

    def test_scout_start_from_zero_badges_declines_then_accepts(self):
        maps = ('GoldenrodCity_hns', 'GoldenrodCity_RadioTower_1F_hns')
        interp = Interp(*maps)
        interp.run('GoldenrodCity_OnTransition')
        # The scout is visible from the first visit; the quiz works before the takeover.
        self.assertNotIn('FLAG_HIDE_GOLDENROD_ROCKET_SCOUT', interp.flags)
        interp.run('RadioTower1F_EventScript_Reception')
        self.assertIn('RadioTower1F_Text_RadioCardWomanOfferQuiz', interp.messages)
        interp.run('RadioTower1F_EventScript_ReceptionGiveRadio')
        self.assertIn('FLAG_ENABLE_RADIO', interp.flags)
        # Declining changes nothing.
        interp.answers = [0]
        interp.messages.clear()
        interp.writes.clear()
        interp.run('GoldenrodCity_EventScript_RocketScout')
        self.assertEqual(interp.vars[GOLD], 1)
        self.assertFalse(interp.written('var') - {'VAR_RESULT'})
        self.assertNotIn('FLAG_HIDE_GOLDENROD_ROCKET_SCOUT', interp.flags)
        self.assertEqual(interp.messages, ['GoldenrodCity_Text_RocketScout1', 'GoldenrodCity_Text_RocketScout2',
                                           'GoldenrodCity_Text_RocketScoutFollow'])
        # Accepting starts the occupation: Goldenrod only, no Mahogany state, no phone call.
        interp.answers = [1]
        interp.run('GoldenrodCity_EventScript_RocketScout')
        self.assertEqual(interp.vars[GOLD], 6)
        self.assertTrue({'FLAG_HIDE_GOLDENROD_ROCKET_SCOUT', 'FLAG_HIDE_GOLDENROD_NPCS'} <= interp.flags)
        self.assertFalse({'FLAG_HIDE_GOLDENROD_ROCKETS', 'FLAG_HIDE_ROCKET_GOLDENROD_RADIOTOWER_1'} & interp.flags)
        self.assertNotIn(MAH, interp.written())
        self.assertNotIn('VAR_TRIGGER_ELM_ROCKET_CALL', interp.written())
        self.assertEqual(interp.pokenav, [])

    def test_takeover_start_never_rewinds_goldenrod(self):
        for gold in (0, 1, 8, 11):
            interp = Interp('GoldenrodCity_hns', vars={MAH: 14, GOLD: gold})
            interp.run('GoldenrodCity_EventScript_StartTakeover')
            self.assertEqual(interp.vars[MAH], 14)
            self.assertEqual(interp.vars[GOLD], max(gold, 6))

    def test_policeman_hides_with_the_scout_and_the_scout_prompts_first(self):
        block = _label_block(raw_text('GoldenrodCity_hns'), 'GoldenrodCity_EventScript_RocketScout')
        self.assertLess(block.index('MSGBOX_YESNO'), block.index('GoldenrodCity_EventScript_StartTakeover'))
        policeman = [o for o in json.loads((MAPS / 'GoldenrodCity_RadioTower_2F_hns/map.json').read_text())
                     ['object_events'] if o['script'].endswith('Policeman')]
        self.assertEqual([o['flag'] for o in policeman], ['FLAG_HIDE_GOLDENROD_ROCKET_SCOUT'])

    def test_nothing_arms_the_takeover_anywhere(self):
        for path in all_script_files():
            if 'Route24' in path.parent.name:
                continue
            text = path.read_text()
            for gone in ('TryArmRocketTakeover', 'SetupElmRocketCall', 'VAR_TRIGGER_ELM_ROCKET_CALL',
                         'Mahoganytown_EventScript_RocketTrigger', 'ElmPhoneRadioTowerRocketTakeoverText'):
                self.assertNotIn(gone, text, (path.parent.name, gone))
        # Winning the three west-Johto gyms starts nothing.
        for label, name in (('CianwoodGym_EventScript_ChuckVictory', 'CianwoodGym_hns'),
                            ('OlivineCity_Gym_EventScript_JasmineVictory', 'OlivineCity_Gym_hns'),
                            ('MahoganyTown_Gym_EventScript_PryceVictory', 'MahoganyTown_Gym_hns')):
            interp = Interp(name, flags={'FLAG_BADGE05_GET', 'FLAG_BADGE06_GET', 'FLAG_BADGE07_GET',
                                         'FLAG_DEFEATED_MAHOGANY_TOWN_GYM'})
            interp.run(label)
            self.assertNotIn(GOLD, interp.written(), label)

    def test_radio_quiz_is_unavailable_during_the_occupation_and_never_regresses_it(self):
        maps = ('GoldenrodCity_RadioTower_1F_hns',)
        for gold in range(0, 13):
            interp = Interp(*maps, vars={GOLD: gold})
            interp.run('RadioTower1F_EventScript_Reception')
            offered = 'RadioTower1F_Text_RadioCardWomanOfferQuiz' in interp.messages
            self.assertEqual(offered, not 6 <= gold <= 10, gold)
            self.assertEqual(interp.vars.get(GOLD, 0), gold)
            self.assertNotIn(GOLD, interp.written())
        interp = Interp(*maps, vars={GOLD: 8})
        interp.run('RadioTower1F_EventScript_ReceptionGiveRadio')
        self.assertEqual(interp.vars[GOLD], 8)

    def test_wing_is_owed_at_archer_and_given_once_after_baoba(self):
        maps = ('GoldenrodCity_RadioTower_5F_hns',)
        owed = 'FLAG_WAYFARER_JOHTO_RADIO_TOWER_WING_OWED'
        given = 'FLAG_WAYFARER_JOHTO_RADIO_TOWER_WING_GIVEN'
        for branch, wing in ((1, 'ITEM_SILVER_WING'), (2, 'ITEM_RAINBOW_WING')):
            # Archer is beaten before Baoba's choice: the Wing is owed, nothing is given.
            interp = Interp(*maps, vars={GOLD: 9})
            interp.vars['VAR_FACING'] = interp.get('DIR_SOUTH')
            interp.run('GoldenrodCity_RadioTower_5F_EventScript_Archer')
            self.assertIn(owed, interp.flags)
            self.assertEqual(interp.vars[GOLD], 10)
            self.assertEqual(interp.given, [])
            self.assertIn(RESOLVED, interp.flags)
            self.assertNotIn('GoldenrodCity_RadioTower_5F_Text_DirectorThanks', interp.messages)
            # The office Director only points at Baoba.
            interp.messages.clear()
            interp.run('GoldenrodCity_RadioTower_5F_EventScript_Petrel')
            self.assertEqual(interp.messages, ['GoldenrodCity_RadioTower_5F_Text_DirectorHoldsWing'])
            self.assertEqual(interp.given, [])
            # After the branch is set: a full Bag leaves it owed, a retry gives it, never twice.
            interp.vars['VAR_LUGIA_OR_HOOH'] = branch
            interp.bag_full = True
            interp.run('GoldenrodCity_RadioTower_5F_EventScript_Petrel')
            self.assertEqual((interp.given, given in interp.flags, owed in interp.flags), ([], False, True))
            interp.bag_full = False
            interp.run('GoldenrodCity_RadioTower_5F_EventScript_Petrel')
            self.assertEqual(interp.given, [wing])
            self.assertIn(given, interp.flags)
            interp.messages.clear()
            interp.run('GoldenrodCity_RadioTower_5F_EventScript_Petrel')
            self.assertEqual(interp.given, [wing])
            self.assertEqual(interp.messages[-1], 'GoldenrodCity_RadioTower_5F_Text_DirectorIntro')

    def test_wing_with_the_branch_already_chosen_comes_from_the_cutscene_once(self):
        maps = ('GoldenrodCity_RadioTower_5F_hns',)
        interp = Interp(*maps, vars={GOLD: 9, 'VAR_LUGIA_OR_HOOH': 1, 'VAR_FACING': Interp('GoldenrodCity_hns').get('DIR_SOUTH')})
        interp.run('GoldenrodCity_RadioTower_5F_EventScript_Archer')
        self.assertEqual(interp.given, ['ITEM_SILVER_WING'])
        interp.run('GoldenrodCity_RadioTower_5F_EventScript_Petrel')
        self.assertEqual(interp.given, ['ITEM_SILVER_WING'])
        # A full Bag during the cutscene leaves the Wing with the office Director.
        full = Interp(*maps, vars={GOLD: 9, 'VAR_LUGIA_OR_HOOH': 2, 'VAR_FACING': Interp('GoldenrodCity_hns').get('DIR_WEST')},
                      bag_full=True)
        full.run('GoldenrodCity_RadioTower_5F_EventScript_Archer')
        self.assertEqual(full.given, [])
        self.assertIn(RESOLVED, full.flags)
        full.bag_full = False
        full.run('GoldenrodCity_RadioTower_5F_EventScript_Petrel')
        self.assertEqual(full.given, ['ITEM_RAINBOW_WING'])

    def test_resolution_unlocks_only_the_merchant_dialogue(self):
        interp = Interp('GoldenrodCity_RadioTower_5F_hns', vars={GOLD: 10, MAH: 3, WELL: 2})
        interp.run('GoldenrodCity_RadioTower_5F_Fakeout')
        self.assertEqual((interp.vars[MAH], interp.vars[WELL]), (3, 2))
        self.assertNotIn(MAH, interp.written())
        self.assertNotIn(WELL, interp.written())

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

    def test_rage_candy_bar_checks_room_before_taking_money(self):
        label = 'MahoganyTown_EventScript_MerchantRageCandyBar'
        full = Interp('Mahoganytown_hns', bag_full=True)
        full.run(label)
        self.assertEqual(full.vars.get('VAR_MONEY_SPENT', 0), 0)
        roomy = Interp('Mahoganytown_hns')
        roomy.run(label)
        self.assertEqual(roomy.vars['VAR_MONEY_SPENT'], 300)
        for branch in raw_text('Mahoganytown_hns').split('removemoney 300')[:-1]:
            self.assertIn('checkitemspace ITEM_RAGE_CANDY_BAR', branch.rsplit('showmoneybox', 1)[1])

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
                for ceremony in (0, 1, 2, 3):
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

    def test_ceremony_gives_back_the_theater_npcs_it_removes(self):
        # removeobject hides through the NPCs' shared flag; each ending that removes
        # them must clear it again before it finishes.
        text = raw_text('EcruteakCity_Theater_hns')
        removals = [m.start() for m in re.finditer(r'removeobject LOCALID_ECRUTEAK_SISTER\b', text)]
        self.assertEqual(len(removals), 2)
        for start in removals:
            ending = text[start:].split('\trelease\n', 1)[0]
            self.assertIn('clearflag FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS', ending)

    def test_surf_trigger_with_a_full_bag_walks_him_back(self):
        text = raw_text('EcruteakCity_Theater_hns')
        block = _label_block(text, 'EcruteakCity_Theater_EventScript_TriggerSurfGuyBagFull')
        self.assertIn('EcruteakCity_Theater_Movement_SurfGuy2', block)
        self.assertIn('EcruteakCity_Theater_EventScript_TriggerSurfGuyBagFull',
                      _label_block(text, 'EcruteakCity_Theater_EventScript_TriggerSurfGuy'))

    def test_elm_handoff_leaves_the_theater_npcs_and_surf_giver_visible(self):
        handoff = raw_text('NewBarkTown_Lab_hns')
        self.assertNotIn('setflag FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS', handoff)
        for name in ('EcruteakCity_Theater_hns', 'EcruteakCity_hns'):
            self.assertNotIn('setflag FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS', raw_text(name), name)

    def test_ceremony_scenes_move_the_ceremony_variable_forward_only(self):
        interp = Interp('EcruteakCity_Theater_hns', vars={CEREMONY: 3, THEATER: 3})
        interp.run('EcruteakCity_Theater_EventScript_LegendaryCutscene')
        self.assertEqual(interp.vars[CEREMONY], 3)
        self.assertEqual(interp.vars[THEATER], 3)
        for name in ('TinTower_RoofDay_hns', 'WhirlIslands_LugiaChamber_hns'):
            self.assertIn('setvar_forward %s, KIMONO_CEREMONY_RESOLVED' % CEREMONY, raw_text(name))
        # Nothing but the ceremony and the chambers writes the ceremony, and Elm no longer starts it.
        for path in all_script_files():
            if CEREMONY in path.read_text():
                self.assertIn(path.parent.name, ('EcruteakCity_Theater_hns', 'TinTower_RoofDay_hns',
                                                 'WhirlIslands_LugiaChamber_hns', 'ReceptionGate_hns',
                                                 'NewBarkTown_Lab_hns', 'EcruteakCity_hns'))
        self.assertNotRegex(raw_text('NewBarkTown_Lab_hns'), r'setvar\w*\s+%s' % CEREMONY)

    def test_the_old_theater_entry_triggers_are_gone(self):
        events = json.loads((MAPS / 'EcruteakCity_Theater_hns/map.json').read_text())['coord_events']
        for event in events:
            self.assertEqual(event['var'], THEATER, event)
            self.assertNotIn('PreBattle', event['script'])
        self.assertNotIn('PreBattle', raw_text('EcruteakCity_Theater_hns'))
        # Ecruteak's Silver tile reads neither ceremony nor theater state.
        city = json.loads((MAPS / 'EcruteakCity_hns/map.json').read_text())['coord_events']
        silver = [e for e in city if e['script'] == 'EcruteakCity_Trigger_Silver']
        self.assertEqual([(e['var'], e['var_value']) for e in silver], [('VAR_TEMP_0', '0')])
        interp = Interp('EcruteakCity_hns', vars={THEATER: 3, CEREMONY: 2}, flags={RESOLVED})
        interp.run('EcruteakCity_Trigger_Silver')
        self.assertEqual((interp.vars[THEATER], interp.vars[CEREMONY]), (3, 2))

    def test_whitney_owns_her_state(self):
        for path in ('GoldenrodCity_Gym_hns/scripts.inc', 'GoldenrodCity_Gym_hns/map.json'):
            text = (MAPS / path).read_text()
            self.assertNotIn(GOLD, text)
            self.assertIn(WHITNEY, text)


class BagFullEndings(unittest.TestCase):
    """The interpreter treats a goto to an unknown label as the script ending, so check
    textually that every bag-full path ends the script and releases the player."""
    MAPS = ('AzaleaTown_hns', 'AzaleaTown_KurtsHouse_hns', 'AzaleaTown_House1_hns', 'IlexForest_hns',
            'Mahoganytown_hns', 'EcruteakCity_Theater_hns')

    def test_bag_full_branches_jump_to_an_ending_label(self):
        checked = 0
        for map_name in self.MAPS:
            text = raw_text(map_name)
            lines = text.splitlines()
            for number, line in enumerate(lines):
                match = re.match(r'\s*goto_if_eq VAR_RESULT, FALSE, (\w+)', line)
                if not match or not re.search(r'giveitem|checkitemspace', ' '.join(lines[max(0, number - 3):number])):
                    continue
                label = match.group(1)
                checked += 1
                self.assertNotEqual(label, 'Common_EventScript_BagIsFull', map_name)  # returns without releasing
                if label == 'Common_EventScript_ShowBagIsFull':
                    continue
                block = _label_block(text, label)
                self.assertRegex(block, r'(?m)^\s*release\n\s*end\b', (map_name, label))
        self.assertGreaterEqual(checked, 8)

    def test_charcoal_survives_a_full_bag(self):
        full = Interp('AzaleaTown_House1_hns', vars={'VAR_ILEX_FOREST_FARFETCHD': 3}, bag_full=True)
        full.run('AzaleaTown_House1_EventScript_Apprentice3')
        self.assertNotIn('FLAG_GOT_CHARCOAL', full.flags)
        ok = Interp('AzaleaTown_House1_hns', vars={'VAR_ILEX_FOREST_FARFETCHD': 3})
        ok.run('AzaleaTown_House1_EventScript_Apprentice3')
        self.assertIn('FLAG_GOT_CHARCOAL', ok.flags)

    def test_route42_eusine_starts_hidden_from_the_new_game(self):
        new_game = (GAME / 'data/scripts/new_game.inc').read_text()
        reset = new_game.split('EventScript_ResetAllMapFlagsHnS::', 1)[1].split('\n\n', 1)[0]
        self.assertIn('setflag FLAG_HIDE_ROUTE42_EUSINE\n', reset)


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
                    # The GS Ball chain's own restart from 10 after Celebi escapes.
                    if (path.parent.name, line.strip()) == ('AzaleaTown_KurtsHouse_hns',
                                                             'setvar VAR_AZALEA_TOWN_STATE, 8'):
                        continue
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


def _scene(maps, label, texts, *, vars=None, flags=(), battle=False, host=()):
    return {'maps': maps, 'label': label, 'texts': set(texts), 'vars': vars or {}, 'flags': set(flags),
            'battle': battle, 'host': set(host)}


# Silver's scenes by chapter: the script that plays it, its Silver-only text, the host state it needs,
# and the host variables it may move even when Silver is not part of it.
SCENES = {
    2: _scene(('CherrygroveCity_hns',), 'CherryGroveCity_EventScript_TriggerSilver',
              ['CherrygroveCity_Text_RivalSeen'], vars={'VAR_CHERRYGROVE_CITY_STATE': 3}, battle=True,
              host=['VAR_CHERRYGROVE_CITY_STATE']),
    3: _scene(('SproutTower_3F_hns',), 'SproutTower_3F_EventScript_TriggerSceneLeft',
              ['SproutTower_Text_ElderLecturesRival']),
    4: _scene(('AzaleaTown_hns',), 'AzaleaTown_EventScript_SilverTriggerTop',
              ['AzaleaTown_Text_RivalBefore', 'AzaleaTown_Text_RivalBeforeNoWell'],
              flags=['FLAG_DEFEATED_AZALEA_TOWN_GYM'], battle=True),
    5: _scene(('BurnedTower_1F_hns',), 'BurnedTower_1F_EventScript_Silver', ['BurnedTowerSilver_BeforeText'],
              vars={ECR: 2}, battle=True, host=[ECR]),
    6: _scene(('OlivineCity_hns',), 'OlivineCity_EventScript_SilverTrigger', ['OlivineCity_Text_SilverEncounter'],
              vars={'VAR_OLIVINE_CITY_STATE': 1}),
    7: _scene(('RocketHideout_B3F_hns',), 'RocketHideout_B3F_Trigger_Silver',
              ['RocketHideout_B3F_Text_Rival', 'RocketHideout_B3F_Text_RivalNeutral'], vars={MAH: 7}, host=[MAH]),
    8: _scene(('GoldenrodCity_UndergroundSwitches_hns',), 'GoldenrodCity_UndergroundSwitches_EventScript_SilverTriggerTop',
              ['GoldenrodUnderground_Text_SilverBefore'], vars={GOLD: 8}, battle=True, host=[GOLD]),
    9: _scene(('EcruteakCity_hns',), 'EcruteakCity_Trigger_Silver', ['EcruteakCity_Text_Silver2'],
              flags=[RESOLVED]),
    10: _scene(('VictoryRoadKanto_1F_hns',), 'VictoryRoadKanto_1F_EventScript_Silver', ['VictoryRoad_Text_RivalBefore'],
               battle=True),
    11: _scene(('MtMoon_Cave_hns',), 'MtMoon_Cave_EventScript_Silver', ['MtMoon_Cave_Text_Rival_Before'], battle=True),
    12: _scene(('IndigoPlateau_PokemonCenter_hns',), 'IndigoPlateau_EventScript_Silver',
               ['IndigoPlateau_Text_RivalEntrance1'], flags=[won(11)], battle=True),
}
BATTLE_CHAPTERS = (2, 4, 5, 8, 10, 11)
NO_SILVER_WRITES = {'VAR_RESULT', 'VAR_0x8004', 'VAR_0x8005', 'VAR_0x8006', 'VAR_0x8007', 'VAR_TEMP_0', 'VAR_TEMP_1'}


def play(chapter, *, johto=True, started=None, flags=(), vars=None, battle='win', answers=1, extra_maps=()):
    scene = SCENES[chapter]
    if started is None:
        started = 10 if chapter == 11 else 0  # Mt. Moon only follows Victory Road
    interp = Interp(*scene['maps'], *extra_maps, johto=johto, answers=answers,
                    vars={**scene['vars'], CHAPTER: started, **(vars or {})}, flags=scene['flags'] | set(flags))
    interp.battle = battle
    interp.run(scene['label'])
    return interp


def silver_text(chapter, interp):
    return bool(SCENES[chapter]['texts'] & set(interp.messages))


def silver_writes(chapter, interp):
    """Everything a scene wrote beyond the engine scratch variables and its host's own progress."""
    return interp.written() - NO_SILVER_WRITES - SCENES[chapter]['host']


class SilverChapters(unittest.TestCase):
    def test_every_scene_follows_the_chapter_table(self):
        for started in (0, 2, 4, 5, 8, 10, 11):
            wins = [(), (started,) if started else (), tuple(c for c in BATTLE_CHAPTERS if c <= started)]
            for won_chapters in wins:
                for extra in ((), (played(6), played(7), played(9))):
                    flags = {won(c) for c in won_chapters} | set(extra)
                    for chapter in SCENES:
                        interp = play(chapter, started=started, flags=flags)
                        full = flags | SCENES[chapter]['flags']
                        expected = silver_playable(chapter, True, started, full)
                        with self.subTest(chapter=chapter, started=started, flags=sorted(flags)):
                            self.assertEqual(silver_text(chapter, interp), expected)
                            self.assertEqual(bool(interp.battles), expected and SCENES[chapter]['battle'])

    def test_starting_a_battle_stores_the_chapter_just_before_it(self):
        for chapter in BATTLE_CHAPTERS + (12,):
            interp = play(chapter)
            with self.subTest(chapter=chapter):
                self.assertEqual(len(interp.battles), 1)
                if chapter == 12:
                    continue  # the daily rematch never moves the saved chapter
                self.assertEqual(interp.battles[0][1], chapter)
                self.assertEqual(interp.vars[CHAPTER], chapter)
                self.assertTrue({won(chapter), WON_ANY} <= interp.flags)

    def test_a_loss_leaves_the_battle_offered_and_the_win_unrecorded(self):
        for chapter in BATTLE_CHAPTERS:
            lost = play(chapter, battle='lose')
            with self.subTest(chapter=chapter):
                self.assertEqual(lost.battles[0][1], chapter)
                self.assertEqual(lost.vars[CHAPTER], chapter)
                self.assertFalse({won(chapter), WON_ANY} & lost.flags)
                again = play(chapter, started=chapter, flags=lost.flags, vars={k: v for k, v in lost.vars.items()
                                                                               if k != CHAPTER})
                self.assertTrue(silver_text(chapter, again))
                self.assertEqual(len(again.battles), 1)
                self.assertIn(won(chapter), again.flags)

    def test_a_later_battle_retires_earlier_scenes_without_rewards(self):
        for later in BATTLE_CHAPTERS:
            for chapter in SCENES:
                if chapter >= later or chapter in (9, 12):
                    continue
                interp = play(chapter, started=later)
                with self.subTest(later=later, chapter=chapter):
                    retired = not silver_playable(chapter, True, later, set())
                    self.assertEqual(silver_text(chapter, interp), not retired)
                    if retired:
                        self.assertEqual(interp.battles, [])
                        self.assertEqual(silver_writes(chapter, interp), set())

    def test_non_battle_scenes_never_change_the_stored_chapter(self):
        for chapter in (3, 6, 7, 9):
            interp = play(chapter, started=0)
            with self.subTest(chapter=chapter):
                self.assertTrue(silver_text(chapter, interp))
                self.assertEqual(interp.vars[CHAPTER], 0)

    def test_olivine_before_azalea_leaves_both_battles(self):
        maps = [SCENES[c]['maps'][0] for c in (4, 5, 6)]
        interp = Interp(*maps, vars={ECR: 2, 'VAR_OLIVINE_CITY_STATE': 1},
                        flags={'FLAG_DEFEATED_AZALEA_TOWN_GYM'})
        interp.run(SCENES[6]['label'])
        self.assertIn(played(6), interp.flags)
        self.assertEqual(interp.vars.get(CHAPTER, 0), 0)
        interp.run(SCENES[4]['label'])
        interp.run(SCENES[5]['label'])
        self.assertEqual([chapter for _, chapter in interp.battles], [4, 5])
        self.assertTrue({won(4), won(5)} <= interp.flags)

    def test_chapter_state_is_only_written_forward_and_before_a_battle(self):
        for path in all_script_files():
            for number, line in enumerate(path.read_text().splitlines(), 1):
                parts = line.split('@')[0].split()
                if len(parts) >= 2 and parts[1].rstrip(',') == CHAPTER:
                    self.assertEqual(parts[0], 'setvar_forward', '%s:%d' % (path.parent.name, number))

    def test_scene_trigger_scripts_need_no_starter_flag(self):
        for chapter, scene in SCENES.items():
            for name in scene['maps']:
                self.assertNotIn('FLAG_JOHTO_STARTER_CHOICE_COMMITTED', raw_text(name), name)

    def test_azalea_scene_fires_after_later_azalea_steps_have_passed_state_5(self):
        # Ilex Forest and the GS Ball can finish before Bugsy; the old state-5 tile would never fire.
        for azalea in (1, 7, 9):
            interp = play(4, vars={AZ: azalea})
            self.assertTrue(silver_text(4, interp), azalea)
            self.assertEqual(interp.vars[AZ], max(azalea, 6))
        interp = Interp(*SCENES[4]['maps'], vars={AZ: 5}, flags=set())
        interp.run(SCENES[4]['label'])
        self.assertEqual((interp.messages, interp.battles), ([], []))  # Bugsy not beaten yet

    def test_indigo_daily_follows_the_mt_moon_win(self):
        interp = Interp(*SCENES[12]['maps'], johto=True, flags=set())
        interp.run(SCENES[12]['label'])
        self.assertEqual((interp.messages, interp.battles), ([], []))
        for johto in (True, False):
            interp = Interp(*SCENES[12]['maps'], johto=johto, flags={won(11)})
            interp.run(SCENES[12]['label'])
            self.assertEqual(bool(interp.battles), johto)


class SilverVisitors(unittest.TestCase):
    def test_no_scene_plays_or_writes_silver_state_for_a_visitor(self):
        for chapter in SCENES:
            for started in (0, 10):
                interp = play(chapter, johto=False, started=started)
                with self.subTest(chapter=chapter, started=started):
                    self.assertFalse(silver_text(chapter, interp))
                    self.assertEqual(interp.battles, [])
                    self.assertEqual(silver_writes(chapter, interp), set())
                    self.assertFalse([name for name, value, kind in interp.writes
                                      if kind == 'flag' and value == 0 and 'SILVER' in name])

    def test_visitor_scene_entries_do_not_freeze_the_player(self):
        for chapter in SCENES:
            interp = play(chapter, johto=False)
            with self.subTest(chapter=chapter):
                self.assertFalse(interp.locked)

    def test_every_silver_object_is_hidden_on_load_for_a_visitor(self):
        for name, (maps, label, flag, extra_vars, extra_flags) in LOAD_HIDES.items():
            for started in (0, 10):
                interp = Interp(*maps, johto=False, vars={CHAPTER: started, **extra_vars}, flags=extra_flags)
                interp.run(label)
                with self.subTest(map=name, started=started):
                    self.assertIn(flag, interp.flags)
                    self.assertFalse([w for w in interp.writes if w[0] == flag and w[1] == 0])

    def test_new_bark_silver_is_hidden_for_the_hoenn_origin_too(self):
        interp = Interp('NewBarkTown_hns', johto=False, vars={'VAR_NEWBARK_TOWN_STATE': 2})
        interp.run('NewBarkTown_OnTransition')
        self.assertIn('FLAG_HIDE_SILVER_NEWBARKTOWN', interp.flags)
        source_c = (GAME / 'src/wayfarer_origin.c').read_text()
        self.assertNotIn('FlagClear(FLAG_HIDE_SILVER_NEWBARKTOWN)', source_c)
        self.assertIn('FlagSet(FLAG_HIDE_SILVER_NEWBARKTOWN)', source_c)

    def test_new_bark_silver_shows_for_a_fresh_johto_origin_only(self):
        interp = Interp('NewBarkTown_hns', johto=True, flags={'FLAG_HIDE_SILVER_NEWBARKTOWN'},
                        vars={'VAR_NEWBARK_TOWN_STATE': 2})
        interp.run('NewBarkTown_OnTransition')
        self.assertNotIn('FLAG_HIDE_SILVER_NEWBARKTOWN', interp.flags)
        interp = Interp('NewBarkTown_hns', johto=True, vars={'VAR_NEWBARK_TOWN_STATE': 2, CHAPTER: 2})
        interp.run('NewBarkTown_OnTransition')
        self.assertIn('FLAG_HIDE_SILVER_NEWBARKTOWN', interp.flags)

    def test_burned_tower_release_and_morty_complete_without_silver(self):
        for johto, started, flags in ((False, 0, ()), (True, 8, ()), (True, 0, (won(5),))):
            interp = Interp('BurnedTower_1F_hns', 'BurnedTower_B1F_hns', johto=johto,
                            vars={ECR: 2, CHAPTER: started}, flags=flags)
            interp.run('BurnedTower_1F_EventScript_Silver')
            with self.subTest(johto=johto, started=started):
                self.assertEqual(interp.battles, [])
                self.assertEqual(interp.messages, ['BurnedTower_1F_Text_FloorGaveWay'])
                self.assertEqual(interp.vars[ECR], 3)
                interp.run('BurnedTower_B1F_EventScript_ReleaseBeasts')
                self.assertEqual(interp.vars[ECR], 4)
                self.assertIn('FLAG_RELEASED_BEASTS', interp.flags)
                self.assertIn('FLAG_HIDE_ECRUTEAK_CITY_SAGE', interp.flags)  # Morty's Gym door opens

    def test_burned_tower_silver_has_his_own_hide_flag(self):
        events = json.loads((MAPS / 'BurnedTower_1F_hns/map.json').read_text())['object_events']
        flags = {event['local_id']: event['flag'] for event in events if 'local_id' in event}
        self.assertEqual(flags['LOCALID_BURNED_TOWER_SILVER'], 'FLAG_WAYFARER_JOHTO_HIDE_BURNED_TOWER_SILVER')
        self.assertEqual(flags['LOCALID_BURNED_TOWER_EUSINE'], 'FLAG_HIDE_BURNED_TOWER_NPCS')

    def test_underground_step_happens_without_silver(self):
        for johto, started, flags in ((False, 0, ()), (True, 10, ()), (True, 0, (won(8),))):
            for label in ('SilverTriggerTop', 'SilverTriggerBot'):
                interp = Interp(*SCENES[8]['maps'], johto=johto, vars={GOLD: 8, CHAPTER: started}, flags=flags)
                interp.run('GoldenrodCity_UndergroundSwitches_EventScript_' + label)
                with self.subTest(johto=johto, started=started, label=label):
                    self.assertEqual(interp.vars[GOLD], 9)
                    self.assertEqual(interp.battles, [])
                    self.assertEqual(interp.messages, [])

    def test_underground_loss_waits_for_a_win_or_a_later_chapter(self):
        lost = play(8, battle='lose')
        self.assertEqual(lost.vars[GOLD], 8)
        self.assertTrue(silver_playable(8, True, lost.vars[CHAPTER], lost.flags))
        retired = play(8, started=10, vars={GOLD: 8})
        self.assertEqual(retired.vars[GOLD], 9)

    def test_hideout_progress_does_not_wait_on_silver(self):
        for johto, started in ((False, 0), (True, 8)):
            interp = Interp(*SCENES[7]['maps'], johto=johto, vars={MAH: 7, CHAPTER: started})
            interp.run('RocketHideout_B3F_Trigger_Silver')
            self.assertEqual(interp.vars[MAH], 8)
            self.assertNotIn(played(7), interp.flags)
        pattern = re.compile(r'(?:goto_if_\w+|call_if_\w+|map_script_2)\s+VAR_MAHOGANY_TOWN_STATE,\s*8\b')
        for path in all_script_files():
            self.assertIsNone(pattern.search(path.read_text()), path.parent.name)
        for path in MAPS.glob('*/map.json'):
            for kind in ('coord_events', 'bg_events', 'object_events'):
                for event in json.loads(path.read_text()).get(kind, []):
                    self.assertFalse(event.get('var') == MAH and event.get('var_value') == '8', path.parent.name)


# (maps, transition label, hide flag, vars, flags) for every Silver object that exists on load
LOAD_HIDES = {
    'New Bark': (('NewBarkTown_hns',), 'NewBarkTown_OnTransition', 'FLAG_HIDE_SILVER_NEWBARKTOWN',
                 {'VAR_NEWBARK_TOWN_STATE': 2}, set()),
    'Cherrygrove': (('CherrygroveCity_hns',), 'CherryGroveCity_OnTransition', 'FLAG_HIDE_SILVER_CHERRYGROVE', {}, set()),
    'Sprout Tower 3F': (('SproutTower_3F_hns',), 'SproutTower_3F_OnTransition', 'FLAG_HIDE_SPROUT_TOWER_SILVER', {}, set()),
    'Azalea': (('AzaleaTown_hns',), 'AzaleaTown_OnTransition', 'FLAG_HIDE_AZALEA_TOWN_SILVER', {},
               {'FLAG_WAYFARER_JOHTO_SETUP_AZALEA'}),
    'Burned Tower 1F': (('BurnedTower_1F_hns',), 'BurnedTower_1F_OnTransition',
                        'FLAG_WAYFARER_JOHTO_HIDE_BURNED_TOWER_SILVER', {ECR: 2}, set()),
    'Victory Road 1F': (('VictoryRoadKanto_1F_hns',), 'VictoryRoadKanto_1F_OnTransition',
                        'FLAG_HIDE_VICTORY_ROAD_SILVER', {}, set()),
    'Mt. Moon': (('MtMoon_Cave_hns',), 'MtMoon_Cave_OnTransition', 'FLAG_HIDE_MTMOON_SILVER', {}, set()),
    'Indigo Center': (('IndigoPlateau_PokemonCenter_hns',), 'IndigoPlateau_Pokecenter_OnTransition',
                      'FLAG_HIDE_INDIGO_PLATEAU_SILVER', {}, {won(11)}),
}
# The objects whose visibility is fully derived (shown as well as hidden) on every load.
DERIVED = {'Burned Tower 1F': 5, 'Victory Road 1F': 10, 'Mt. Moon': 11, 'Indigo Center': 12}


class SilverVisibility(unittest.TestCase):
    def test_derived_objects_follow_the_chapter_rules_on_every_load(self):
        for name, chapter in DERIVED.items():
            maps, label, flag, extra_vars, extra_flags = LOAD_HIDES[name]
            for started in (0, 2, 5, 8, 10, 11):
                for wins in ((), tuple(c for c in BATTLE_CHAPTERS if c <= started)):
                    flags = {won(c) for c in wins} | set(extra_flags)
                    for stale in (True, False):
                        interp = Interp(*maps, vars={CHAPTER: started, **extra_vars},
                                        flags=flags | ({flag} if stale else set()))
                        interp.run(label)
                        with self.subTest(map=name, started=started, wins=wins, stale=stale):
                            self.assertEqual(flag not in interp.flags,
                                             silver_playable(chapter, True, started, flags))

    def test_mt_moon_is_hidden_until_victory_road_has_started(self):
        maps, label, flag, _, _ = LOAD_HIDES['Mt. Moon']
        for started, shown in ((0, False), (8, False), (10, True), (11, True)):
            interp = Interp(*maps, vars={CHAPTER: started})
            interp.run(label)
            self.assertEqual(flag not in interp.flags, shown, started)
        interp = Interp(*maps, vars={CHAPTER: 11}, flags={won(11)})
        interp.run(label)
        self.assertIn(flag, interp.flags)
        self.assertNotIn('FLAG_HIDE_MTMOON_SILVER', (GAME / 'data/scripts/new_game.inc').read_text())

    def test_hide_only_objects_are_hidden_once_retired(self):
        for name in ('Cherrygrove', 'Sprout Tower 3F', 'Azalea'):
            maps, label, flag, extra_vars, extra_flags = LOAD_HIDES[name]
            chapter = {'Cherrygrove': 2, 'Sprout Tower 3F': 3, 'Azalea': 4}[name]
            for started in (0, 2, 4, 5, 8, 10):
                interp = Interp(*maps, vars={CHAPTER: started, **extra_vars}, flags=extra_flags)
                interp.run(label)
                with self.subTest(map=name, started=started):
                    if not silver_playable(chapter, True, started, set()):
                        self.assertIn(flag, interp.flags)
                    else:
                        self.assertNotIn(flag, interp.flags)


class SilverDialogue(unittest.TestCase):
    def test_azalea_drops_the_rocket_question_without_the_well(self):
        for well, text in ((0, 'AzaleaTown_Text_RivalBeforeNoWell'), (2, 'AzaleaTown_Text_RivalBeforeNoWell'),
                           (3, 'AzaleaTown_Text_RivalBefore'), (4, 'AzaleaTown_Text_RivalBefore')):
            interp = play(4, vars={WELL: well})
            self.assertIn(text, interp.messages, well)
            self.assertNotIn('AzaleaTown_Text_RivalBefore' if well < 3 else 'AzaleaTown_Text_RivalBeforeNoWell',
                             interp.messages, well)
        no_well = re.search(r'AzaleaTown_Text_RivalBeforeNoWell:\n(.*?)\$', raw_text('AzaleaTown_hns'), re.S)[1]
        self.assertNotIn('ROCKET', no_well)

    def test_underground_before_line_needs_an_earlier_win(self):
        for won_before, text in ((False, 'GoldenrodUnderground_Text_SilverFirstBattle'),
                                 (True, 'GoldenrodUnderground_Text_SilverBeatenBefore')):
            interp = play(8, flags=[WON_ANY] if won_before else [])
            self.assertIn(text, interp.messages)
            self.assertEqual(len([m for m in interp.messages if m.startswith('GoldenrodUnderground_Text_Silver')
                                  and ('Beaten' in m or 'First' in m)]), 1)
        self.assertNotIn('beat me', re.search(r'SilverFirstBattle:\n(.*?)\$', raw_text('GoldenrodCity_UndergroundSwitches_hns'), re.S)[1])

    def test_underground_after_line_needs_the_hideout(self):
        lance = 'GoldenrodUnderground_Text_SilverAfterLance'
        interp = play(8, flags=[played(7)])
        self.assertIn(lance, interp.messages)
        interp = play(8)
        self.assertNotIn(lance, interp.messages)
        self.assertEqual(interp.messages[-2:], ['GoldenrodUnderground_Text_SilverAfter',
                                                'GoldenrodUnderground_Text_SilverAfterEnd'])

    def test_hideout_line_is_neutral_without_olivine(self):
        self.assertIn('RocketHideout_B3F_Text_Rival', play(7, flags=[played(6)]).messages)
        interp = play(7)
        self.assertIn('RocketHideout_B3F_Text_RivalNeutral', interp.messages)
        neutral = re.search(r'RivalNeutral:\n(.*?)\$', raw_text('RocketHideout_B3F_hns'), re.S)[1]
        self.assertNotIn("Didn't I tell you", neutral)

    def test_victory_road_after_line_needs_the_hideout(self):
        dragon = 'VictoryRoad_Text_RivalDragonMaster'
        self.assertIn(dragon, play(10, flags=[played(7)]).messages)
        self.assertNotIn(dragon, play(10).messages)
        defeat = re.search(r'RivalDefeat:\n(.*?)\$', raw_text('VictoryRoadKanto_1F_hns'), re.S)[1]
        self.assertTrue(defeat.rstrip().endswith('What you possess, and what I lack…'.strip()), defeat)
        self.assertNotIn('dragon', defeat)

    def test_victory_road_opener_drops_the_league_question_after_indigo(self):
        question = 'VictoryRoad_Text_RivalLeagueQuestion'
        self.assertIn(question, play(10).messages)
        self.assertNotIn(question, play(10, flags=[INDIGO_ENTERED]).messages)
        body = play(10, flags=[INDIGO_ENTERED]).messages
        self.assertEqual(body[0], 'VictoryRoad_Text_RivalBefore')
        opener = re.search(r'RivalBefore:\n(.*?)\$', raw_text('VictoryRoadKanto_1F_hns'), re.S)[1]
        self.assertTrue(opener.lstrip().startswith('.string "…Don\'t make me laugh.'))

    def test_mt_moon_drops_since_i_lost_without_a_win(self):
        beaten = play(11, started=10, flags=[WON_ANY])
        self.assertIn('MtMoon_Cave_Text_Rival_BeforeBeaten', beaten.messages)
        first = play(11, started=10)
        self.assertIn('MtMoon_Cave_Text_Rival_BeforeFirst', first.messages)
        self.assertNotIn('MtMoon_Cave_Text_Rival_BeforeBeaten', first.messages)
        text = re.search(r'BeforeFirst:\n(.*?)\$', raw_text('MtMoon_Cave_hns'), re.S)[1]
        self.assertNotIn('Since I lost to you', text)


class VictoryRoadCorridor(unittest.TestCase):
    def test_declining_and_losing_leave_the_corridor_passable(self):
        declined = play(10, answers=0)
        self.assertEqual(declined.battles, [])
        self.assertEqual(declined.messages[-1], 'VictoryRoad_Text_RivalDeclined')
        self.assertEqual(silver_writes(10, declined), set())
        self.assertTrue(silver_playable(10, True, 0, declined.flags))  # the offer stays available
        lost = play(10, battle='lose')
        self.assertTrue(silver_playable(10, True, lost.vars[CHAPTER], lost.flags))
        # Neither touches Route 27 or the League corridor's state.
        for interp in (declined, lost):
            self.assertNotIn('VAR_ROUTE27_STATE', interp.written())

    def test_corridor_has_no_trigger_and_silver_stands_off_every_path(self):
        data = json.loads((MAPS / 'VictoryRoadKanto_1F_hns/map.json').read_text())
        self.assertEqual(data['coord_events'], [])
        silver = [e for e in data['object_events'] if e.get('local_id') == 'LOCALID_VICTORYROAD_SILVER'][0]
        self.assertEqual(silver['script'], 'VictoryRoadKanto_1F_EventScript_Silver')
        tile = (silver['x'], silver['y'])
        self.assertEqual(tile, (26, 6))
        walkable = victory_road_walkable()
        self.assertIn(tile, walkable)
        pocket = {(26, 5), (26, 6)}
        # The alcove is a dead end: nothing but the lane's side opens into it ...
        for x, y in pocket:
            for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                if (nx, ny) in walkable and (nx, ny) not in pocket:
                    self.assertEqual((nx, ny), (x + 1, y), (x, y))
        # ... and closing it changes no route between the corridor's two ends.
        without = walkable - pocket
        self.assertEqual(reachable(without, (28, 3)), reachable(walkable, (28, 3)) - pocket)
        lane = {(x, 7) for x in (27, 28, 29)}
        self.assertTrue(lane <= reachable(without, (28, 3)))
        self.assertIn((28, 10), reachable(without, (28, 3)))

    def test_league_corridor_story_state_is_independent_of_silver(self):
        text = raw_text('VictoryRoadKanto_1F_hns')
        self.assertNotIn('VAR_ROUTE27_STATE', text)
        self.assertNotIn('LeagueCircuit', text)


def victory_road_walkable():
    import struct
    data = (GAME / 'data/layouts/VictoryRoadKanto_1F_hns/map.bin').read_bytes()
    width, height = 50, 30
    tiles = struct.unpack('<%dH' % (width * height), data)
    return {(x, y) for y in range(height) for x in range(width) if not (tiles[y * width + x] >> 10) & 3}


def reachable(walkable, start):
    seen, todo = {start}, [start]
    while todo:
        x, y = todo.pop()
        for step in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if step in walkable and step not in seen:
                seen.add(step)
                todo.append(step)
    return seen


class ElmAndSeedForVisitors(unittest.TestCase):
    def test_no_elm_call_plays_for_a_visitor(self):
        cases = (
            ('VioletCity_hns', 'VioletCity_EventScript_ElmCall_Trigger', {'VAR_VIOLET_CITY_STATE': 3},
             'VAR_VIOLET_CITY_STATE', 4),
            ('BlackthornCity_hns', 'BlackthornCity_EventScript_Call', {'VAR_BLACKTHORN_CITY_STATE': 4},
             'VAR_BLACKTHORN_CITY_STATE', 5),
            ('NewBarkTown_hns', 'NewBarkTown_EventScript_ElmCallAfterLeague', {'VAR_NEWBARK_TOWN_STATE': 6},
             'VAR_NEWBARK_TOWN_STATE', 7),
        )
        for map_name, label, vars_, state, after in cases:
            for johto in (True, False):
                interp = Interp(map_name, johto=johto, vars=vars_, flags={'FLAG_HIDE_VIOLET_CITY_AIDE'})
                interp.run(label)
                with self.subTest(map=map_name, johto=johto):
                    self.assertEqual(bool(interp.pokenav), johto)
                    # Every call moves its state on, so no frame script stays armed for a visitor.
                    self.assertEqual(interp.vars[state], after)
                    self.assertFalse(interp.locked)
                    if map_name == 'VioletCity_hns':
                        self.assertEqual('FLAG_HIDE_VIOLET_CITY_AIDE' in interp.flags, not johto)
                        self.assertEqual('FLAG_HIDE_NEWBARKTOWN_LAB_AIDE' in interp.flags, johto)

    def test_ceremony_scenes_and_calls_write_no_lab_state(self):
        for path in all_script_files():
            for number, line in enumerate(path.read_text().splitlines(), 1):
                parts = line.split('@')[0].replace(',', ' ').split()
                if len(parts) >= 3 and parts[0] in ('setvar', 'setvar_forward', 'addvar') \
                        and parts[1] == 'VAR_NEWBARKTOWN_LABSTATE':
                    # Only Elm's errand writes the lab state, and only its own values 0-6.
                    self.assertIn(path.parent.name, ('NewBarkTown_Lab_hns', 'Route30_MrPokemonsHouse_hns',
                                                     'NewBarkTown_PlayersHouse_1F_hns'), (path.parent.name, number))
                    self.assertLessEqual(int(parts[2]), 6, (path.parent.name, number))

    def test_den_trial_owes_the_master_ball_without_lab_state(self):
        for label in ('DragonsDen_Cavern_Trigger_ClairLeft', 'DragonsDen_Cavern_Trigger_ClairMid',
                      'DragonsDen_Cavern_Trigger_ClairRight'):
            interp = Interp('DragonsDen_Cavern_hns', vars={'VAR_BLACKTHORN_CITY_STATE': 3})
            interp.run(label)
            self.assertEqual(interp.vars['VAR_BLACKTHORN_CITY_STATE'], 4, label)
            self.assertIn(BALL_OWED, interp.flags, label)
            self.assertNotIn(LAB, interp.written(), label)

    def test_blackthorn_call_advances_for_the_johto_origin_and_never_touches_the_lab(self):
        for lab_state in (0, 3, 5, 6):
            interp = Interp('BlackthornCity_hns', vars={'VAR_BLACKTHORN_CITY_STATE': 4, LAB: lab_state})
            interp.run('BlackthornCity_EventScript_Call')
            self.assertEqual(interp.vars['VAR_BLACKTHORN_CITY_STATE'], 5)
            self.assertEqual(interp.vars[LAB], lab_state)
            self.assertEqual(len(interp.pokenav), 1)

    def test_visitor_elder_line_sends_them_to_elm_while_the_ball_is_owed(self):
        maps = ('DragonsDen_Shrine_hns',)
        line = 'DragonsDen_Shrine_Text_ElderElmAskedForYou'
        # Before or after Blackthorn's frame script moved the city on.
        for city in (4, 5):
            for got_dratini in (False, True):
                flags = {BALL_OWED} | ({'FLAG_GOT_DRATINI'} if got_dratini else set())
                interp = Interp(*maps, johto=False, vars={'VAR_BLACKTHORN_CITY_STATE': city}, flags=flags)
                interp.run('DragonsDen_Shrine_EventScript_Elder')
                self.assertIn(line, interp.messages, (city, got_dratini))
                self.assertNotIn(LAB, interp.written())
                # Quiet once the Ball is taken.
                interp.messages.clear()
                interp.flags.add(BALL_GOT)
                interp.run('DragonsDen_Shrine_EventScript_Elder')
                self.assertNotIn(line, interp.messages)
        # The Johto origin hears Elm's call instead; the Elder stays quiet.
        interp = Interp(*maps, johto=True, vars={'VAR_BLACKTHORN_CITY_STATE': 4}, flags={BALL_OWED})
        interp.run('DragonsDen_Shrine_EventScript_Elder')
        self.assertNotIn(line, interp.messages)
        # Before the trial completes he says nothing about Elm.
        interp = Interp(*maps, johto=False, vars={'VAR_BLACKTHORN_CITY_STATE': 3})
        interp.run('DragonsDen_Shrine_EventScript_Elder')
        self.assertNotIn(line, interp.messages)

    def test_visitor_blackthorn_frame_script_leaves_state_four_without_a_call(self):
        # BlackthornCity_OnFrame runs the call every frame while the city is at 4, holding the
        # player, so the visitor's skip must move it on too.
        self.assertIn('map_script_2 VAR_BLACKTHORN_CITY_STATE, 4, BlackthornCity_EventScript_Call',
                      raw_text('BlackthornCity_hns'))
        interp = Interp('BlackthornCity_hns', johto=False, vars={'VAR_BLACKTHORN_CITY_STATE': 4})
        interp.run('BlackthornCity_EventScript_Call')
        self.assertEqual((interp.vars['VAR_BLACKTHORN_CITY_STATE'], interp.pokenav), (5, []))

    def test_master_ball_at_tr_zero_once_for_both_origins_and_after_a_full_pocket(self):
        for johto in (True, False):
            interp = Interp('NewBarkTown_Lab_hns', johto=johto, flags={BALL_OWED}, bag_full=True)
            interp.run('NewBarkTown_Lab_EventScript_Elm')
            self.assertEqual((interp.given, BALL_GOT in interp.flags), ([], False), johto)
            interp.bag_full = False
            interp.run('NewBarkTown_Lab_EventScript_Elm')
            self.assertEqual(interp.given, ['ITEM_MASTER_BALL'], johto)
            self.assertIn(BALL_GOT, interp.flags)
            self.assertEqual('NewBarkTown_Lab_Text_VisitorElmIntroduces' in interp.messages, not johto)
            interp.run('NewBarkTown_Lab_EventScript_Elm')
            interp.run('NewBarkTown_Lab_EventScript_Elm')
            self.assertEqual(interp.given, ['ITEM_MASTER_BALL'], johto)
        block = _label_block(raw_text('NewBarkTown_Lab_hns'), 'NewBarkTown_Lab_EventScript_ElmMasterBall')
        self.assertNotRegex(block, 'TrainerRating|TRAINER_RATING')

    def test_master_ball_works_at_every_errand_stage_and_the_errand_still_completes(self):
        stages = {
            'before the egg return': ({LAB: 4}, set()),
            'before the Violet aide': ({LAB: 6}, {'FLAG_HIDE_NEWBARKTOWN_LAB_AIDE'}),
            'with the aide waiting and no starter yet': ({LAB: 1}, {'FLAG_HIDE_NEWBARKTOWN_LAB_AIDE'}),
            'after the ceremony': ({LAB: 6, CEREMONY: 3}, set()),
        }
        for name, (vars_, flags) in stages.items():
            interp = Interp('NewBarkTown_Lab_hns', vars=vars_, flags=set(flags) | {BALL_OWED})
            before = vars_[LAB]
            interp.run('NewBarkTown_Lab_EventScript_Elm')
            self.assertEqual(interp.given, ['ITEM_MASTER_BALL'], name)
            self.assertEqual(interp.vars[LAB], before, name)
            self.assertNotIn(LAB, interp.written(), name)
        # The errand still completes: the egg return runs after the ball was taken.
        interp = Interp('NewBarkTown_Lab_hns', vars={LAB: 4}, flags={BALL_OWED})
        interp.run('NewBarkTown_Lab_EventScript_Elm')
        interp.run('NewBarkTown_Lab_EventScript_PoliceEpisode')
        self.assertEqual(interp.vars[LAB], 5)
        # After the ceremony Elm's errand talk is not stuck on the ceremony lines.
        interp = Interp('NewBarkTown_Lab_hns', vars={LAB: 5, CEREMONY: 3},
                        flags={BALL_OWED, BALL_GOT, 'FLAG_WAYFARER_JOHTO_ELM_TICKET_RECEIVED'})
        interp.run('NewBarkTown_Lab_EventScript_Elm')
        self.assertIn('NewBarkTown_Lab_Text_ElmStudyingEgg', interp.messages)

    def test_master_ball_handoff_no_longer_starts_the_ceremony(self):
        for johto in (True, False):
            lab = Interp('NewBarkTown_Lab_hns', johto=johto, flags={BALL_OWED})
            lab.run('NewBarkTown_Lab_EventScript_Elm')
            self.assertEqual(lab.vars.get(CEREMONY, 0), 0)
            self.assertFalse(lab.written() & {THEATER, 'FLAG_HIDE_ECRUTEAK_SILVER', 'FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS',
                                              'FLAG_HIDE_ECRUTEAK_CITY_THEATER_KIMONOS', CEREMONY, LAB})
            # The trial is not open (no Wing, TR 0), so Elm does not mention the Kimono Girls.
            self.assertNotIn('NewBarkTown_Lab_Text_KimonoGirls', lab.messages)
            self.assertNotIn('INTERIM', raw_text('NewBarkTown_Lab_hns'))

    def test_elm_mentions_the_kimono_girls_only_while_the_trial_is_open_and_unfinished(self):
        text = raw_text('NewBarkTown_Lab_hns')
        self.assertNotIn('since that EGG got you', text)
        # The hint does not wait for the Dragon's Den: no Master Ball owed or received.
        for branch in (1, 2):
            talk = Interp('NewBarkTown_Lab_hns', vars={LAB: 6, BRANCH: branch, 'VAR_GOLDENROD_CITY_STATE': 1},
                          flags={RESOLVED, 'FLAG_RECEIVED_TOGEPI_EGG', 'FLAG_SHOWN_ELM_TOGEPI'})
            talk.items = {WINGS[branch]}
            talk.tr = LEGENDARY_TR
            talk.run('NewBarkTown_Lab_EventScript_Elm')
            self.assertIn('NewBarkTown_Lab_Text_KimonoWaiting', talk.messages, branch)
        for johto in (True, False):
            for branch in (1, 2):
                cases = (  # (ceremony, wing held, radio tower resolved, tr, expected)
                    (0, True, True, LEGENDARY_TR, True),
                    (STARTED, True, True, LEGENDARY_TR, True),
                    (DONE, True, True, LEGENDARY_TR, False),
                    (RESOLVED_CEREMONY, True, True, LEGENDARY_TR, False),
                    (0, False, True, LEGENDARY_TR, False),
                    (0, True, False, LEGENDARY_TR, False),
                    (0, True, True, LEGENDARY_TR - 1, False),
                )
                for ceremony, wing, resolved, tr, expected in cases:
                    flags = {BALL_OWED} | ({RESOLVED} if resolved else set())
                    # The handoff.
                    handoff = Interp('NewBarkTown_Lab_hns', johto=johto, vars={BRANCH: branch, CEREMONY: ceremony},
                                     flags=flags)
                    handoff.items = {WINGS[branch]} if wing else set()
                    handoff.tr = tr
                    handoff.run('NewBarkTown_Lab_EventScript_Elm')
                    self.assertEqual('NewBarkTown_Lab_Text_KimonoGirls' in handoff.messages, expected,
                                     ('handoff', johto, branch, ceremony, wing, resolved, tr))
                    self.assertEqual(handoff.vars.get(CEREMONY, 0), ceremony)
                    # The later talk (the errand is done and the Ball taken), also in a real late
                    # save: egg hatched and shown, Goldenrod visited.
                    if johto:
                        for late in (set(), {'FLAG_RECEIVED_TOGEPI_EGG', 'FLAG_SHOWN_ELM_TOGEPI'}):
                            talk = Interp('NewBarkTown_Lab_hns',
                                          vars={LAB: 6, BRANCH: branch, CEREMONY: ceremony,
                                                'VAR_GOLDENROD_CITY_STATE': 1 if late else 0},
                                          flags=flags | {BALL_GOT} | late)
                            talk.items = {WINGS[branch]} if wing else set()
                            talk.tr = tr
                            talk.run('NewBarkTown_Lab_EventScript_Elm')
                            self.assertEqual('NewBarkTown_Lab_Text_KimonoWaiting' in talk.messages, expected,
                                             ('talk', branch, ceremony, wing, resolved, tr, bool(late)))
                            # Once the encounter is resolved, Elm sends the player to the League.
                            self.assertEqual('NewBarkTown_Lab_Text_League' in talk.messages,
                                             ceremony == RESOLVED_CEREMONY,
                                             ('league', branch, ceremony, bool(late)))

    def test_post_league_call_owes_the_ticket_without_touching_the_lab(self):
        for johto in (True, False):
            interp = Interp('NewBarkTown_hns', 'NewBarkTown_Lab_hns', johto=johto,
                            vars={'VAR_NEWBARK_TOWN_STATE': 6, LAB: 5})
            interp.run('NewBarkTown_EventScript_ElmCallAfterLeague')
            self.assertEqual(interp.vars[LAB], 5)
            self.assertIn('FLAG_WAYFARER_JOHTO_ELM_TICKET_OWED', interp.flags)
            interp.run('NewBarkTown_Lab_EventScript_Elm')
            self.assertIn('NewBarkTown_Lab_Text_ElmGiveTicket1', interp.messages, johto)
            self.assertIn('FLAG_WAYFARER_JOHTO_ELM_TICKET_RECEIVED', interp.flags)
            interp.messages.clear()
            interp.run('NewBarkTown_Lab_EventScript_Elm')
            self.assertNotIn('NewBarkTown_Lab_Text_ElmGiveTicket1', interp.messages)

    def test_gym_boy_never_blocks_and_clair_and_the_den_need_no_other_badges(self):
        text = raw_text('BlackthornCity_hns')
        self.assertNotIn('MAP_SCRIPT_ON_LOAD', text)
        self.assertNotIn('MoveGymBoy', text)
        self.assertNotRegex(text, 'FLAG_BADGE0[567]_GET')
        self.assertNotRegex(raw_text('BlackthornCity_Gym_hns'), 'FLAG_BADGE0[567]_GET')
        self.assertNotRegex(raw_text('DragonsDen_Shrine_hns'), 'FLAG_BADGE0[567]_GET')
        for state, line in ((1, 'BlackthornCity_Text_ClairIsIn'), (2, 'BlackthornCity_Text_ClairIsBeaten')):
            interp = Interp('BlackthornCity_hns', vars={'VAR_BLACKTHORN_CITY_STATE': state})
            interp.run('BlackthornCity_EventScript_GymBoy')
            self.assertEqual(interp.messages, [line])
        # Clair, then the Den, with no other badge.
        gym = Interp('BlackthornCity_Gym_hns', vars={'VAR_BLACKTHORN_CITY_STATE': 1})
        gym.run('BlackthornGym_EventScript_Clair')
        self.assertEqual(gym.vars['VAR_BLACKTHORN_CITY_STATE'], 2)
        self.assertEqual(len(gym.battles), 1)

    def test_miracle_seed_is_given_once_before_and_after_falkner(self):
        for falkner_first in (False, True):
            interp = Interp('Route32_hns', 'VioletCity_Gym_hns', 'VioletCity_hns', johto=False,
                            vars={'VAR_VIOLET_CITY_STATE': 1})
            order = ['falkner', 'seed', 'call'] if falkner_first else ['seed', 'falkner', 'call']
            for step in order:
                if step == 'seed':
                    interp.run('Route32_EventScript_BaldingMan')
                    interp.run('Route32_EventScript_BaldingMan')
                elif step == 'falkner':
                    interp.run('VioletCity_Gym_EventScript_Falkner')
                else:
                    interp.run('VioletCity_EventScript_ElmCall_Trigger')
            with self.subTest(falkner_first=falkner_first):
                self.assertEqual(interp.given.count('ITEM_MIRACLE_SEED'), 1)
                self.assertIn(SEED, interp.flags)
                self.assertEqual(interp.vars['VAR_VIOLET_CITY_STATE'], 4)

    def test_miracle_seed_needs_no_silver_togepi_or_gym_and_survives_a_full_bag(self):
        interp = Interp('Route32_hns', johto=True, bag_full=True)
        interp.run('Route32_EventScript_BaldingMan')
        self.assertNotIn(SEED, interp.flags)
        self.assertEqual(interp.given, [])
        interp.bag_full = False
        interp.run('Route32_EventScript_BaldingMan')
        self.assertIn(SEED, interp.flags)
        self.assertEqual(interp.given, ['ITEM_MIRACLE_SEED'])
        interp.run('Route32_EventScript_BaldingMan')
        self.assertEqual(interp.given, ['ITEM_MIRACLE_SEED'])
        text = raw_text('Route32_hns')
        block = _label_block(text, 'Route32_EventScript_BaldingMan')
        for gone in ('SILVER', 'TOGEPI', 'DEFEATED_VIOLET_GYM', 'VAR_VIOLET_CITY_STATE'):
            self.assertNotIn(gone, block)
        self.assertLess(block.index('giveitem'), block.index('setflag %s' % SEED))

    def test_violet_state_writes_are_forward_only(self):
        for path in all_script_files():
            for number, line in enumerate(path.read_text().splitlines(), 1):
                parts = line.split('@')[0].split()
                if len(parts) >= 2 and parts[0] == 'setvar' and parts[1].rstrip(',') == 'VAR_VIOLET_CITY_STATE':
                    self.fail('%s:%d %s' % (path.parent.name, number, line.strip()))

    def test_origin_starter_slot_replaces_the_johto_starter_in_content(self):
        for path in all_script_files():
            if path.parent.name == 'NewBarkTown_Lab_hns' or 'Silver' in path.parent.name:
                continue
            text = path.read_text()
            if 'Melemele' in path.parent.name or 'MrPokemon' in path.parent.name:
                self.assertNotIn('VAR_STARTER_MON', text)


LEGENDARY_MAPS = ('EcruteakCity_Theater_hns', 'NewBarkTown_Lab_hns')


def zuki_talk(*, branch=1, wing=True, resolved=True, tr=LEGENDARY_TR, theater=3, ceremony=0, johto=True,
              lab=5, extra_flags=(), answers=1, extra_vars=None):
    flags = set(extra_flags) | ({RESOLVED} if resolved else set())
    interp = Interp(*LEGENDARY_MAPS, johto=johto, answers=answers, flags=flags,
                    vars={BRANCH: branch, THEATER: theater, CEREMONY: ceremony, LAB: lab, **(extra_vars or {})})
    interp.items = {WINGS[branch]} if wing and branch else set()
    interp.tr = tr
    interp.run('EcruteakCity_Theater_EventScript_Zuki')
    return interp


class KimonoTrial(unittest.TestCase):
    def test_only_the_wing_the_radio_tower_and_tr_together_offer_the_trial(self):
        needs = {1: 'EcruteakCity_Theater_Text_ZukiNeedsWing', 2: 'EcruteakCity_Theater_Text_ZukiNeedsTr',
                 3: 'EcruteakCity_Theater_Text_ZukiNeedsBoth'}
        for johto in (True, False):
            for branch in (1, 2):
                cases = (  # (wing held, radio tower resolved, tr, missing bits)
                    (False, False, 0, 3),                       # neither requirement
                    (True, True, 0, 2),                         # only the Wing (and the resolved tower)
                    (False, False, LEGENDARY_TR, 1),            # only TR
                    (False, True, LEGENDARY_TR, 1),             # tower resolved but the Wing is not held
                    (True, False, LEGENDARY_TR, 1),             # a Wing but the tower is not resolved
                    (True, True, LEGENDARY_TR - 1, 2),
                    (True, True, LEGENDARY_TR, 0),
                )
                for wing, resolved, tr, missing in cases:
                    with self.subTest(johto=johto, branch=branch, wing=wing, resolved=resolved, tr=tr):
                        interp = zuki_talk(branch=branch, wing=wing, resolved=resolved, tr=tr, johto=johto)
                        offered = 'EcruteakCity_Theater_Text_ZukiSeenAgain' in interp.messages
                        self.assertEqual(offered, missing == 0)
                        self.assertEqual(interp.vars.get(CEREMONY, 0), DONE if missing == 0 else 0)
                        self.assertEqual(len(interp.battles), 5 if missing == 0 else 0)
                        if missing:
                            self.assertIn(needs[missing], interp.messages)
                            self.assertIn('EcruteakCity_Theater_Text_ZukiTests', interp.messages)
                            self.assertFalse(interp.locked)

    def test_no_branch_speaks_of_the_legendary_without_naming_one(self):
        interp = zuki_talk(branch=0, wing=False, resolved=False, tr=LEGENDARY_TR)
        self.assertIn('EcruteakCity_Theater_Text_ZukiTestsNoBranch', interp.messages)
        self.assertNotIn('EcruteakCity_Theater_Text_ZukiTests', interp.messages)
        self.assertEqual(interp.battles, [])
        text = raw_text('EcruteakCity_Theater_hns')
        block = text.split('EcruteakCity_Theater_Text_ZukiTestsNoBranch:', 1)[1].split('$"', 1)[0]
        self.assertIn('BAOBA', block)
        self.assertIn('ROUTE 39', block)
        for name in ('LUGIA', 'HO-OH', 'STR_VAR_1'):
            self.assertNotIn(name, block)

    def test_the_trial_waits_for_the_rocket_and_the_rocket_can_be_beaten_before_or_after(self):
        # The Rocket scene is active: Zuki does not offer, however ready the player is.
        scene = zuki_talk(theater=1)
        self.assertEqual((scene.battles, scene.vars[CEREMONY]), ([], 0))
        self.assertIn('EcruteakCity_Theater_Text_KimonoGirlBeaten', scene.messages)
        # Ready, then beat the Rocket: the offer follows; the ceremony is untouched by the Rocket.
        rocket = Interp(*LEGENDARY_MAPS, vars={BRANCH: 1, THEATER: 1, CEREMONY: 0}, flags={RESOLVED})
        rocket.items = {WINGS[1]}
        rocket.tr = LEGENDARY_TR
        rocket.run('EcruteakCity_Theater_EventScript_Rocket')
        self.assertEqual((rocket.vars[THEATER], rocket.vars[CEREMONY]), (2, 0))
        rocket.run('EcruteakCity_Theater_EventScript_Zuki')
        self.assertEqual((len(rocket.battles), rocket.vars[CEREMONY]), (6, DONE))
        # Beat the Rocket first, then become ready.
        early = Interp(*LEGENDARY_MAPS, vars={BRANCH: 2, THEATER: 1, CEREMONY: 0})
        early.run('EcruteakCity_Theater_EventScript_Rocket')
        self.assertEqual(early.vars[THEATER], 2)
        early.run('EcruteakCity_Theater_EventScript_Zuki')
        self.assertEqual(early.battles[1:], [])

    def test_the_kimono_girls_appear_with_the_trial_and_stay(self):
        text = raw_text('EcruteakCity_Theater_hns')
        events = json.loads((MAPS / 'EcruteakCity_Theater_hns/map.json').read_text())['object_events']
        girls = [e for e in events if e.get('local_id') in ('LOCALID_ECRUTEAK_SAYO', 'LOCALID_ECRUTEAK_NAOKO',
                                                        'LOCALID_ECRUTEAK_KUNI', 'LOCALID_ECRUTEAK_MIKI')]
        self.assertEqual({e['flag'] for e in girls}, {'FLAG_HIDE_ECRUTEAK_CITY_THEATER_KIMONOS'})
        decline = zuki_talk(answers=0, extra_flags={'FLAG_HIDE_ECRUTEAK_CITY_THEATER_KIMONOS'})
        self.assertNotIn('FLAG_HIDE_ECRUTEAK_CITY_THEATER_KIMONOS', decline.flags)
        self.assertEqual((decline.vars[CEREMONY], decline.battles), (STARTED, []))
        self.assertIn('EcruteakCity_Theater_Text_ZukiNo', decline.messages)
        # Talking again goes straight to the question, without repeating the introduction.
        decline.messages.clear()
        decline.answer = 1
        decline.answers = None
        decline.run('EcruteakCity_Theater_EventScript_Zuki')
        self.assertNotIn('EcruteakCity_Theater_Text_ZukiWelcome', decline.messages)
        self.assertEqual(len(decline.battles), 5)
        # A Wing tossed after the start is explained, not ignored.
        tossed = Interp(*LEGENDARY_MAPS, vars={BRANCH: 1, THEATER: 3, CEREMONY: STARTED}, flags={RESOLVED})
        tossed.items = set()
        tossed.tr = LEGENDARY_TR
        tossed.run('EcruteakCity_Theater_EventScript_Zuki')
        self.assertEqual(tossed.battles, [])
        self.assertIn('EcruteakCity_Theater_Text_ZukiNeedsWing', tossed.messages)
        # The girls' own talk: before the question they send the player to Zuki.
        girl = Interp(*LEGENDARY_MAPS, vars={CEREMONY: STARTED})
        girl.run('EcruteakCity_Theater_EventScript_KimonoGirl')
        self.assertIn('EcruteakCity_Theater_Text_KimonoGirlsBeforeBattle', girl.messages)

    def test_the_gauntlet_ends_with_the_ceremony_done_and_only_the_main_progress_set(self):
        for branch, main, other, bell in ((1, LUGIA_P, HOOH_P, 'ITEM_TIDAL_BELL'), (2, HOOH_P, LUGIA_P, 'ITEM_CLEAR_BELL')):
            for johto in (True, False):
                interp = zuki_talk(branch=branch, johto=johto)
                self.assertEqual(interp.given, [bell])
                self.assertEqual(interp.vars[CEREMONY], DONE)
                self.assertEqual(interp.vars[main], 2)
                self.assertNotIn(other, interp.written())
                self.assertEqual(interp.vars[THEATER], 3)
                # The Sister is given back to the theater: the shared NPC flag ends clear.
                self.assertNotIn('FLAG_HIDE_ECRUTEAK_CITY_THEATER_NPCS', interp.flags)
                self.assertTrue({'FLAG_HIDE_ECRUTEAK_CITY_THEATER_ZUKI', 'FLAG_HIDE_ECRUTEAK_CITY_THEATER_KIMONOS'}
                                <= interp.flags)
                # Done: nothing offers it again.
                interp.run('EcruteakCity_Theater_EventScript_Zuki')
                self.assertEqual(len(interp.battles), 5)

    def test_only_the_ceremony_and_pewter_write_legendary_progress(self):
        # The Johto Hall of Fame used to set both to 4, which skipped the ceremony's encounter
        # (forward-only 2) and Pewter's resolved-at-3 check for anyone clearing the League first.
        writers = set()
        for path in all_script_files():
            for line in path.read_text().splitlines():
                parts = line.split('@')[0].replace(',', ' ').split()
                if len(parts) >= 2 and parts[0] in ('setvar', 'setvar_forward', 'addvar') \
                        and parts[1] in (LUGIA_P, HOOH_P):
                    writers.add(path.parent.name)
        # The Hall of Fame's remaining writes are in its non-Wayfarer (#else) arm, and Ecruteak's
        # are in the unreferenced EcruteakCity_Test_League debug label.
        self.assertEqual(writers, {'EcruteakCity_Theater_hns', 'PewterCity_hns', 'WhirlIslands_LugiaChamber_hns',
                                   'TinTower_RoofDay_hns', 'PokemonLeague_HallOfFame_hns', 'EcruteakCity_hns'})
        block = _label_block(raw_text('PokemonLeague_HallOfFame_hns'),
                             'PokemonLeague_HallOfFame_EventScript_SetFirstGameClearFlags')
        self.assertIn('SpawnJohtoRoamers', block)
        self.assertNotRegex(block, 'VAR_COMPLETED_(LUGIA|HO_OH)')
        self.assertNotIn('EcruteakCity_Test_League', ''.join(path.read_text() for path in all_script_files()
                                                             if path.parent.name != 'EcruteakCity_hns'))

    def test_encounter_resolution_completes_the_ceremony_for_every_outcome(self):
        for chamber, label, progress in (
                ('WhirlIslands_LugiaChamber_hns', 'WhirlIslands_LugiaChamber_EventScript_Lugia', LUGIA_P),
                ('TinTower_RoofDay_hns', 'TinTower_RoofDay_EventScript_HoOh', HOOH_P)):
            for outcome, resolves in (('WON', True), ('CAUGHT', True), ('MON_FLED', True),
                                      ('RAN', False), ('PLAYER_TELEPORTED', False)):
                interp = Interp(chamber, vars={progress: 2, CEREMONY: DONE})
                interp.outcome = outcome
                interp.run(label)
                self.assertEqual(interp.vars[progress], 3 if resolves else 2, (label, outcome))
                self.assertEqual(interp.vars[CEREMONY], RESOLVED_CEREMONY if resolves else DONE, (label, outcome))

    def test_chamber_legendaries_stay_hidden_before_their_progress(self):
        for chamber, label, progress, hide in (
                ('WhirlIslands_LugiaChamber_hns', 'WhirlIslands_LugiaChamber_OnTransition', LUGIA_P, 'FLAG_HIDE_LUGIA'),
                ('TinTower_RoofDay_hns', 'TinTower_RoofDay_OnTransition', HOOH_P, 'FLAG_HIDE_HO_OH')):
            for value, shown in ((0, False), (1, False), (2, True), (3, False), (4, True)):
                interp = Interp(chamber, vars={progress: value}, flags={'FLAG_DAY_POKEMON'})
                interp.run(label)
                self.assertEqual(hide not in interp.flags, shown, (chamber, value))
            # Navel Rock shares the flag; a visit there must not leave the Johto legendary standing.
            shared = Interp(chamber, vars={progress: 0})
            shared.run(label)
            self.assertIn(hide, shared.flags)

    def test_readers_of_the_ceremony_use_its_resolved_value(self):
        self.assertIn('goto_if_lt %s, KIMONO_CEREMONY_RESOLVED' % CEREMONY, raw_text('ReceptionGate_hns'))
        self.assertIn('%s, KIMONO_CEREMONY_RESOLVED, EcruteakCity_Theater_MapScript_SetKimonoMovement' % CEREMONY,
                      raw_text('EcruteakCity_Theater_hns'))


class KimonoCallbacks(unittest.TestCase):
    """Each Kimono Girl's memory plays only when its event happened."""

    TEXTS = {
        'zuki': 'EcruteakCity_Theater_Text_ZukiMetInVioletEgg',
        'zuki egg': 'EcruteakCity_Theater_Text_ZukiEggPassed',
        'naoko ilex': 'EcruteakCity_Theater_Text_NaokoIlex',
        'naoko well': 'EcruteakCity_Theater_Text_NaokoWell',
        'miki': 'EcruteakCity_Theater_Text_MikiRocket',
        'sayo ice': 'EcruteakCity_Theater_Text_SayoIce',
        'sayo den': 'EcruteakCity_Theater_Text_SayoDen',
    }

    def play(self, *, johto=True, lab=5, farfetchd=4, well=3, theater=3, ice=1, den=True, branch=1):
        flags = {BALL_OWED} if den else set()
        return zuki_talk(branch=branch, johto=johto, lab=lab, theater=theater, extra_flags=flags,
                         extra_vars={'VAR_ILEX_FOREST_FARFETCHD': farfetchd, WELL: well, 'VAR_ICE_PATH_STATE': ice})

    def test_every_memory_plays_when_its_event_happened(self):
        for branch in (1, 2):
            interp = self.play(branch=branch)
            for name, text in self.TEXTS.items():
                self.assertIn(text, interp.messages, name)
            for text in ('EcruteakCity_Theater_Text_ZukiFirstMeeting',):
                self.assertNotIn(text, interp.messages)
            self.assertIn('EcruteakCity_Theater_Text_KuniSeen', interp.messages)
            self.assertEqual(len(interp.battles), 5)

    def test_every_memory_is_dropped_without_its_event_and_the_challenge_stays(self):
        cases = {
            'zuki': dict(lab=4), 'zuki egg': dict(lab=0), 'naoko ilex': dict(farfetchd=3),
            'naoko well': dict(well=2), 'sayo ice': dict(ice=0), 'sayo den': dict(den=False),
        }
        for name, kwargs in cases.items():
            interp = self.play(**kwargs)
            self.assertNotIn(self.TEXTS[name], interp.messages, name)
            # Every other memory is untouched, and every girl still gives her challenge.
            for other, text in self.TEXTS.items():
                if other != name and not (name in ('zuki', 'zuki egg') and other in ('zuki', 'zuki egg')):
                    self.assertIn(text, interp.messages, (name, other))
            for challenge in ('NaokoChallenge', 'MikiName', 'SayoChallenge', 'KuniSeen'):
                self.assertIn('EcruteakCity_Theater_Text_' + challenge, interp.messages, (name, challenge))
            self.assertEqual(len(interp.battles), 5, name)
        # The Mystery Egg memory is Johto-origin only.
        visitor = self.play(johto=False)
        self.assertNotIn(self.TEXTS['zuki'], visitor.messages)
        self.assertNotIn(self.TEXTS['zuki egg'], visitor.messages)
        self.assertIn('EcruteakCity_Theater_Text_ZukiFirstMeeting', visitor.messages)
        self.assertIn('EcruteakCity_Theater_Text_ZukiBringBack', visitor.messages)

    def test_miki_remembers_the_rocket_only_after_it_was_beaten(self):
        for theater, remembers in ((0, False), (1, False), (2, True), (3, True)):
            interp = Interp('EcruteakCity_Theater_hns', vars={THEATER: theater})
            interp.run('EcruteakCity_Theater_EventScript_MikiBattle')
            self.assertEqual('EcruteakCity_Theater_Text_MikiRocket' in interp.messages, remembers, theater)
            self.assertIn('EcruteakCity_Theater_Text_MikiName', interp.messages)

    def test_the_other_girls_memories_follow_their_own_facts(self):
        # Each fact is read from the adventure that records it, and only from that.
        text = raw_text('EcruteakCity_Theater_hns')
        for needle in ('VAR_ILEX_FOREST_FARFETCHD, 4', 'VAR_WAYFARER_JOHTO_SLOWPOKE_WELL_STATE, 3',
                       'VAR_ICE_PATH_STATE, 1', 'FLAG_WAYFARER_JOHTO_ELM_MASTER_BALL_OWED',
                       'VAR_NEWBARKTOWN_LABSTATE, 5', 'WayfarerIsJohtoOrigin'):
            self.assertIn(needle, text)


class PewterWing(unittest.TestCase):
    def talk(self, *, branch=1, main=3, tr=LEGENDARY_TR, bag_full=False, flags=(), other=0, items=None):
        main_var, other_var = (LUGIA_P, HOOH_P) if branch == 1 else (HOOH_P, LUGIA_P)
        interp = Interp('PewterCity_hns', bag_full=bag_full, flags=flags,
                        vars={BRANCH: branch, main_var: main, other_var: other})
        interp.tr = tr
        interp.run('PewterCity_EventScript_Gramps')
        return interp

    def test_before_a_branch_is_chosen_he_chats_and_releases_the_player(self):
        interp = self.talk(branch=0, main=0)
        self.assertEqual(interp.messages, ['PewterCity_Text_Gramps_GotSilverWing'])
        self.assertFalse(interp.locked)
        self.assertEqual((interp.given, interp.written()), ([], set()))

    def test_he_explains_whichever_requirement_is_unmet(self):
        for branch in (1, 2):
            for main in (0, 2, 3, 4):
                for tr in (0, LEGENDARY_TR):
                    interp = self.talk(branch=branch, main=main, tr=tr)
                    unmet = (main != 3, tr < LEGENDARY_TR)
                    if unmet == (False, False):
                        continue
                    expected = {(True, False): 'PewterCity_Text_GrampsNeedsEncounter',
                                (False, True): 'PewterCity_Text_GrampsNeedsTr',
                                (True, True): 'PewterCity_Text_GrampsNeedsBoth'}[unmet]
                    self.assertEqual(interp.messages, [expected], (branch, main, tr))
                    self.assertEqual((interp.given, interp.written(), interp.locked), ([], set(), False))

    def test_the_gift_after_a_resolved_encounter_enables_the_level_70_one(self):
        for branch, wing, other_var, hide in ((1, 'ITEM_RAINBOW_WING', HOOH_P, 'FLAG_HIDE_TINTOWER_GUARD'),
                                              (2, 'ITEM_SILVER_WING', LUGIA_P, 'FLAG_HIDE_WHIRL_ISLANDS_TENTACRUEL')):
            interp = self.talk(branch=branch)
            self.assertEqual(interp.given, [wing])
            self.assertIn(PEWTER_GIVEN, interp.flags)
            self.assertIn(hide, interp.flags)
            self.assertEqual(interp.vars[other_var], 4)
            self.assertFalse(interp.locked)
            # Once given, never again, even if the Wing was tossed.
            again = Interp('PewterCity_hns', vars=dict(interp.vars), flags=interp.flags)
            again.tr = LEGENDARY_TR
            again.items = set()
            again.run('PewterCity_EventScript_Gramps')
            self.assertEqual((again.given, again.written()), ([], set()))

    def test_a_resolved_other_encounter_is_not_reopened(self):
        for branch, other_var in ((1, HOOH_P), (2, LUGIA_P)):
            interp = self.talk(branch=branch, other=3)
            self.assertEqual(len(interp.given), 1)
            self.assertEqual(interp.vars[other_var], 3)
            hall = self.talk(branch=branch, other=4)
            self.assertEqual(hall.vars[other_var], 4)

    def test_a_full_bag_keeps_the_gift_with_him(self):
        for branch, other_var in ((1, HOOH_P), (2, LUGIA_P)):
            full = self.talk(branch=branch, bag_full=True)
            self.assertEqual((full.given, full.written(), full.locked), ([], set(), False))
            self.assertNotIn(PEWTER_GIVEN, full.flags)
            retry = Interp('PewterCity_hns', vars=dict(full.vars), flags=full.flags)
            retry.tr = LEGENDARY_TR
            retry.run('PewterCity_EventScript_Gramps')
            self.assertEqual(len(retry.given), 1)
            self.assertEqual(retry.vars[other_var], 4)

    def test_the_receipt_follows_the_add(self):
        block = _label_block(raw_text('PewterCity_hns'), 'PewterCity_EventScript_GrampsRainbowWing')
        self.assertLess(block.index('giveitem'), block.index('setflag FLAG_WAYFARER_JOHTO_PEWTER_WING_GIVEN'))
        self.assertLess(block.index('giveitem'), block.index('setvar VAR_COMPLETED_HO_OH, 4'))
        block = _label_block(raw_text('PewterCity_hns'), 'PewterCity_EventScript_GrampsSilverWing')
        self.assertLess(block.index('giveitem'), block.index('setflag FLAG_WAYFARER_JOHTO_PEWTER_WING_GIVEN'))
        self.assertLess(block.index('giveitem'), block.index('setvar VAR_COMPLETED_LUGIA, 4'))

    def test_the_legendary_threshold_has_one_source(self):
        header = (GAME / 'include/wayfarer_sevii_story.h').read_text()
        self.assertIn('#define WAYFARER_BIRD_CAPTURE_TR WAYFARER_LEGENDARY_TR', header)
        self.assertEqual(len(re.findall(r'#define WAYFARER_LEGENDARY_TR ', header)), 2)  # v0 and today's scale
        c = (GAME / 'src/wayfarer_johto_story.c').read_text()
        self.assertEqual(c.count('GetTrainerRating() < WAYFARER_LEGENDARY_TR'), 2)


if __name__ == '__main__':
    unittest.main()
