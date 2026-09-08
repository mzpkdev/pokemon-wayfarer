"""Execute the origin-sensitive portions of Johto's actual preprocessed scripts.

This checks script transactions and interception, not emulator presentation.
Engine delivery and menus are deterministic inputs to the small interpreter.
"""
from pathlib import Path
import json
import re
import subprocess
import unittest

GAME = Path(__file__).resolve().parents[2]
CONSTANTS = {'FALSE': 0, 'TRUE': 1, 'NO': 0, 'YES': 1,
             'MON_GIVEN_TO_PARTY': 0, 'MON_GIVEN_TO_PC': 1, 'MON_CANT_GIVE': 2,
             'HOENN_STARTER_CHOICE_TREECKO': 0, 'HOENN_STARTER_CHOICE_TORCHIC': 1,
             'HOENN_STARTER_CHOICE_MUDKIP': 2, 'HOENN_STARTER_CHOICE_NONE': 65535}


def source(map_name, wayfarer=True):
    return subprocess.check_output([
        'cpp', '-P', '-x', 'assembler-with-cpp', f'-I{GAME / "include"}',
        f'-DIS_WAYFARER={int(wayfarer)}', '-DWAYFARER_LEAGUE_CIRCUIT_ENABLED=1',
        str(GAME / 'data/maps' / map_name / 'scripts.inc'),
    ], text=True)


class Script:
    def __init__(self, map_name, *, native=False, policy=0, answers=(), delivery=0):
        self.lines = source(map_name).splitlines()
        self.labels = {line.strip().rstrip(':'): i + 1 for i, line in enumerate(self.lines)
                       if re.fullmatch(r'\w+:+', line.strip())}
        self.vars = {}
        self.flags = set()
        self.answers = iter(answers)
        self.delivery = delivery
        self.native = native
        self.policy = policy
        self.effects = []
        self.grants = []
        self.calls = []
        self.messages = []
        self.stopped = None

    def value(self, name):
        if name in self.vars:
            return self.vars[name]
        if name in CONSTANTS:
            return CONSTANTS[name]
        try:
            return int(name, 0)
        except ValueError:
            return name

    def run(self, label, stop=()):
        pc = self.labels[label]
        stack = []
        switch_value = None
        for _ in range(500):
            raw = self.lines[pc].strip()
            pc += 1
            if raw.rstrip(':') in stop:
                self.stopped = raw.rstrip(':')
                return
            if not raw or raw.startswith(('@', '.', '#')) or raw.endswith(':'):
                continue
            # Assembly indentation may use tabs between command and arguments.
            parts = raw.split(None, 1)
            command = parts[0]
            args = [x.strip() for x in parts[1].split(',')] if len(parts) > 1 else []
            if command == 'end':
                return
            if command == 'return':
                pc = stack.pop()
            elif command in ('setvar', 'copyvar'):
                self.vars[args[0]] = self.value(args[1])
            elif command in ('setflag', 'clearflag'):
                (self.flags.add if command == 'setflag' else self.flags.discard)(args[0])
            elif command == 'switch':
                switch_value = self.value(args[0])
            elif command == 'case':
                if self.value(args[0]) == switch_value:
                    pc = self.labels[args[1]]
            elif command == 'bufferspeciesname':
                self.vars[args[0]] = self.value(args[1])
            elif command == 'specialvar':
                fn = args[1]
                self.vars[args[0]] = ({'WayfarerDispatchOriginScene': self.policy,
                                     'WayfarerUsesNativeJohtoOpening': int(self.native),
                                     'WayfarerGetJohtoStarterSpecies': 'selected-species',
                                     'IsNuzlockeNicknamingActive': 0}[fn])
            elif command == 'msgbox':
                self.messages.append(args[0])
                if args[-1] == 'MSGBOX_YESNO':
                    self.vars['VAR_RESULT'] = next(self.answers)
            elif command == 'givemon':
                self.vars['VAR_RESULT'] = self.delivery
                if self.delivery != 2:
                    self.grants.append(self.value(args[0]))
            elif command.startswith(('goto', 'call')) and command != 'callnative':
                taken = True
                if '_if_' in command:
                    cond = command.split('_if_')[1]
                    if cond in ('set', 'unset'):
                        taken = (args[0] in self.flags) == (cond == 'set')
                    else:
                        a, b = self.value(args[0]), self.value(args[1])
                        taken = {'eq': lambda: a == b, 'ne': lambda: a != b,
                                 'ge': lambda: a >= b}[cond]()
                if taken:
                    target = args[-1]
                    if command.startswith('call'):
                        self.calls.append(target)
                    if target in stop:
                        self.stopped = target
                        return
                    if target in self.labels:
                        if command.startswith('call'):
                            stack.append(pc)
                        pc = self.labels[target]
                    elif not command.startswith('call'):
                        self.stopped = target
                        return
            elif command.startswith(('applymovement', 'setobject', 'setrespawn', 'trainerbattle')):
                self.effects.append(raw)
        raise AssertionError('script did not terminate')


LAB = 'NewBarkTown_Lab_hns'
PREFIX = 'NewBarkTown_Lab_EventScript_'
CHOSEN = 'FLAG_JOHTO_STARTER_CHOICE_COMMITTED'
RECEIVED = 'FLAG_JOHTO_STARTER_RECEIVED'


class JohtoOriginScripts(unittest.TestCase):
    def test_postponement_does_not_start_errand_or_choose(self):
        script = Script(LAB, answers=[0])
        script.vars.update(VAR_STARTER_MON=0, VAR_NEWBARKTOWN_LABSTATE=0,
                           VAR_NEWBARK_TOWN_STATE=2)
        script.run(PREFIX + 'VisitorIntro')
        self.assertNotIn(CHOSEN, script.flags)
        self.assertEqual(script.vars['VAR_NEWBARKTOWN_LABSTATE'], 0)
        self.assertEqual(script.vars['VAR_NEWBARK_TOWN_STATE'], 2)
        self.assertEqual(script.grants, [])

    def test_declined_ball_never_writes_committed_choice(self):
        for name in ('Chikorita', 'Cyndaquil', 'Totodile'):
            with self.subTest(name=name):
                script = Script(LAB, answers=[0])
                script.vars['VAR_STARTER_MON'] = 99
                script.run(PREFIX + name + 'Ball_Choice')
                self.assertEqual(script.vars['VAR_STARTER_MON'], 99)
                self.assertNotIn(CHOSEN, script.flags)

    def test_visitor_commits_every_slot_before_declined_gift(self):
        for slot in range(3):
            with self.subTest(slot=slot):
                script = Script(LAB, answers=[0])
                script.vars['PLAYER_STARTER_CANDIDATE'] = slot
                script.run(PREFIX + 'ChoseStarter', stop=[PREFIX + 'ElmCampaign'])
                self.assertEqual(script.vars['VAR_STARTER_MON'], slot)
                self.assertIn(CHOSEN, script.flags)
                self.assertNotIn(RECEIVED, script.flags)
                self.assertEqual(script.vars['VAR_NEWBARKTOWN_LABSTATE'], 2)
                self.assertEqual(script.vars['VAR_NEWBARK_TOWN_STATE'], 3)
                self.assertEqual(script.grants, [])

    def test_full_storage_and_retry_preserve_choice_and_progress(self):
        for slot in range(3):
            for delivery in (0, 1, 2):
                with self.subTest(slot=slot, delivery=delivery):
                    script = Script(LAB, answers=[1], delivery=delivery)
                    script.flags.add(CHOSEN)
                    script.vars.update(VAR_STARTER_MON=slot, VAR_NEWBARKTOWN_LABSTATE=9,
                                       VAR_NEWBARK_TOWN_STATE=7)
                    script.run(PREFIX + 'VisitorOfferGift', stop=[PREFIX + 'ElmCampaign'])
                    self.assertEqual(script.vars['VAR_STARTER_MON'], slot)
                    self.assertEqual(script.vars['VAR_NEWBARKTOWN_LABSTATE'], 9)
                    self.assertEqual(script.vars['VAR_NEWBARK_TOWN_STATE'], 7)
                    self.assertEqual(RECEIVED in script.flags, delivery != 2)
                    self.assertEqual(len(script.grants), int(delivery != 2))

    def test_received_gift_never_reenters_delivery(self):
        script = Script(LAB)
        script.flags.update([CHOSEN, RECEIVED])
        script.run(PREFIX + 'VisitorElm', stop=[PREFIX + 'ElmCampaign'])
        self.assertEqual(script.grants, [])
        self.assertEqual(script.stopped, PREFIX + 'ElmCampaign')

    def test_native_receipt_requires_successful_delivery(self):
        for delivery in (0, 2):
            script = Script(LAB, native=True, answers=[0], delivery=delivery)
            script.vars.update(PLAYER_STARTER_CANDIDATE=2, PLAYER_STARTER_SPECIES='species')
            script.run(PREFIX + 'ChoseStarter')
            self.assertEqual(RECEIVED in script.flags, delivery != 2)
            self.assertEqual(CHOSEN in script.flags, delivery != 2)

    def test_visitor_can_leave_while_choosing(self):
        data = json.loads((GAME / 'data/maps' / LAB / 'map.json').read_text())
        exits = [event['script'] for event in data['coord_events']
                 if event['var_value'] == '1' and event['script'] != 'NULL']
        self.assertTrue(exits)
        for label in exits:
            script = Script(LAB)
            script.run(label)
            self.assertEqual(script.effects, [])

    def test_household_intercepts_before_movement_or_quest_writes(self):
        for floor in ('1F', '2F'):
            map_name = f'NewBarkTown_PlayersHouse_{floor}_hns'
            labels = ([f'NewBarkTown_PlayersHouse_1F_EventScript_MomsIntroTrigger{i}' for i in (1, 2)]
                      if floor == '1F' else
                      [f'NewBarkTown_PlayersHouse_2F_Script_ClockSetupTrigger{i}' for i in (1, 2)])
            for policy in (0, 2):
                for label in labels:
                    script = Script(map_name, policy=policy)
                    script.vars['VAR_NEWBARKTOWN_LABSTATE'] = 8
                    script.run(label)
                    self.assertEqual(script.vars['VAR_NEWBARKTOWN_LABSTATE'], 8)
                    self.assertEqual(script.effects, [])
                    self.assertEqual(script.flags, set())

    def test_visitor_healing_does_not_open_family_savings(self):
        script = Script('NewBarkTown_PlayersHouse_1F_hns')
        script.run('NewBarkTown_PlayersHouse_1F_EventScript_Mom')
        self.assertIn('Common_EventScript_OutOfCenterPartyHeal', script.calls)
        self.assertFalse(any('Savings' in call for call in script.calls))
        self.assertEqual(script.flags, set())

    def test_upstairs_clock_interaction_is_read_only_for_visitors(self):
        map_name = 'NewBarkTown_PlayersHouse_2F_hns'
        data = json.loads((GAME / 'data/maps' / map_name / 'map.json').read_text())
        clocks = [event['script'] for event in data['bg_events'] if 'Clock' in event['script']]
        self.assertEqual(len(clocks), 1)
        script = Script(map_name)
        script.run(clocks[0])
        self.assertEqual(script.stopped, 'PlayersHouse_2F_EventScript_CheckWallClock')
        self.assertEqual(script.flags, set())
        self.assertFalse(any('SetWallClock' in call for call in script.calls))

    def test_deferred_silver_never_moves_player_or_starts_battle(self):
        maps = {
            'CherrygroveCity': 'CherryGroveCity_EventScript_TriggerSilver',
            'AzaleaTown': 'AzaleaTown_EventScript_SilverTriggerTop',
            'BurnedTower_1F': 'BurnedTower_1F_EventScript_Silver',
            'GoldenrodCity_UndergroundSwitches': 'GoldenrodCity_UndergroundSwitches_EventScript_SilverTriggerTop',
            'VictoryRoadKanto_1F': 'VictoryRoadKanto_1F_Trigger',
            'MtMoon_Cave': 'MtMoon_Cave_EventScript_Silver',
            'IndigoPlateau_PokemonCenter': 'IndigoPlateau_EventScript_Silver',
        }
        for name, label in maps.items():
            with self.subTest(map=name):
                script = Script(name + '_hns')
                script.run(label)
                self.assertEqual(script.effects, [])
                self.assertEqual(script.calls, [])
                self.assertEqual(script.flags, set())

    def test_town_dialogue_keeps_family_framing_native(self):
        entries = {
            'Lass': ('NewBarkTown_Text_VisitorLass', 'NewBarkTown_Text_LassState4'),
            'FatMan': ('NewBarkTown_Text_VisitorFatMan', 'NewBarkTown_Text_FatManState2'),
            'PlayersHouseMailbox': ('NewBarkTown_Text_VisitorHouse', 'NewBarkTown_Text_PlayersHouse'),
        }
        for name, (visitor_text, native_text) in entries.items():
            for policy in (0, 1, 2):
                with self.subTest(entry=name, policy=policy):
                    script = Script('NewBarkTown_hns', policy=policy)
                    script.vars['VAR_NEWBARK_TOWN_STATE'] = 4 if name == 'Lass' else 2
                    script.run('NewBarkTown_EventScript_' + name)
                    expected = [visitor_text] if policy == 0 else [native_text] if policy == 1 else []
                    self.assertEqual(script.messages, expected)

    def test_native_mom_retains_local_rival_contact(self):
        text = source('NewBarkTown_PlayersHouse_1F_hns')
        handoff = text.split('NewBarkTown_PlayersHouse_1F_EventScript_GivePokeGear::', 1)[1].split('return', 1)[0]
        self.assertIn('special WayfarerGrantSharedEquipment', handoff)
        self.assertIn('setflag FLAG_REGISTER_RIVAL_POKENAV', handoff)

    def test_authored_professor_does_not_fall_into_visitor(self):
        for label in ('Elm', 'ElmIntro'):
            script = Script(LAB, policy=2)
            script.run(PREFIX + label)
            self.assertEqual(script.grants, [])
            self.assertEqual(script.flags, set())

    def test_authored_initial_elm_triggers_precede_native_movement(self):
        for entry in ('TriggerStartLabLeft', 'TriggerStartLabRight', 'TriggerStartLab'):
            script = Script(LAB, policy=2)
            script.run(PREFIX + entry)
            self.assertEqual(script.effects, [])
            self.assertEqual(script.messages, [])
            self.assertEqual(script.grants, [])

    def test_johto_operands_do_not_write_hoenn_bank(self):
        lab = source(LAB)
        self.assertNotRegex(lab, r'(?:setvar|copyvar) VAR_HOENN_STARTER_CHOICE')
        self.assertNotRegex(lab, r'(?:setflag|clearflag) FLAG_HOENN_STARTER_RECEIVED')
        flags = (GAME / 'include/constants/flags.h').read_text()
        values = [int(re.search(rf'#define {flag}\s+(0x[0-9A-Fa-f]+)', flags)[1], 16)
                  for flag in (CHOSEN, RECEIVED)]
        self.assertEqual(len(set(values)), 2)
        self.assertTrue(all(0x91A < value < 0x960 for value in values))

    def test_standalone_has_no_wayfarer_state_or_helpers(self):
        for name in (LAB, 'NewBarkTown_PlayersHouse_1F_hns', 'NewBarkTown_PlayersHouse_2F_hns'):
            compiled = source(name, wayfarer=False)
            self.assertNotIn('Wayfarer', compiled)
            self.assertNotIn(CHOSEN, compiled)
            self.assertNotIn(RECEIVED, compiled)


class HoennDeferredStarterScripts(unittest.TestCase):
    lab = 'LittlerootTown_ProfessorBirchsLab'
    prefix = 'LittlerootTown_ProfessorBirchsLab_EventScript_'

    def test_gift_retry_keeps_committed_choice_campaign_and_rival_visibility(self):
        for slot in range(3):
            for state in (3, 4, 5):
                for answer, delivery in ((0, 0), (1, 0), (1, 1), (1, 2)):
                    with self.subTest(slot=slot, state=state, answer=answer, delivery=delivery):
                        script = Script(self.lab, answers=[answer], delivery=delivery)
                        script.vars.update(VAR_HOENN_STARTER_CHOICE=slot, VAR_BIRCH_LAB_STATE=state,
                                           VAR_LITTLEROOT_RIVAL_STATE=4, VAR_LITTLEROOT_TOWN_STATE=4)
                        script.flags.update(['FLAG_RESCUED_BIRCH', 'FLAG_HIDE_ROUTE_103_RIVAL',
                                             'FLAG_RECEIVED_POKEDEX_FROM_BIRCH', 'FLAG_HIDE_ROUTE_101_BOY'])
                        script.run(self.prefix + 'Birch')
                        self.assertEqual(script.vars['VAR_HOENN_STARTER_CHOICE'], slot)
                        self.assertEqual(script.vars['VAR_BIRCH_LAB_STATE'], state)
                        self.assertEqual(script.vars['VAR_LITTLEROOT_RIVAL_STATE'], 4)
                        self.assertEqual(script.vars['VAR_LITTLEROOT_TOWN_STATE'], 4)
                        self.assertIn('FLAG_HIDE_ROUTE_103_RIVAL', script.flags)
                        self.assertIn('FLAG_HIDE_ROUTE_101_BOY', script.flags)
                        given = answer == 1 and delivery != 2
                        self.assertEqual('FLAG_HOENN_STARTER_RECEIVED' in script.flags, given)
                        self.assertEqual(len(script.grants), int(given))

    def test_initial_choice_still_starts_local_campaign_when_gift_declined(self):
        for slot, name in enumerate(('Treecko', 'Torchic', 'Mudkip')):
            script = Script(self.lab, answers=[0])
            script.vars['VAR_BIRCH_LAB_STATE'] = 2
            script.flags.add('FLAG_HIDE_ROUTE_103_RIVAL')
            script.run(self.prefix + 'Choose' + name)
            self.assertEqual(script.vars['VAR_HOENN_STARTER_CHOICE'], slot)
            self.assertEqual(script.vars['VAR_BIRCH_LAB_STATE'], 3)
            self.assertNotIn('FLAG_HIDE_ROUTE_103_RIVAL', script.flags)
            self.assertEqual(script.grants, [])

    def test_authored_birch_never_enters_stock_pending_gift(self):
        for entry in ('Birch', 'GiveStarterEvent'):
            script = Script(self.lab, policy=2)
            script.vars.update(VAR_HOENN_STARTER_CHOICE=1, VAR_BIRCH_LAB_STATE=5)
            script.flags.add('FLAG_RESCUED_BIRCH')
            script.run(self.prefix + entry)
            self.assertEqual(script.messages, [])
            self.assertEqual(script.grants, [])
            self.assertEqual(script.vars['VAR_BIRCH_LAB_STATE'], 5)


if __name__ == '__main__':
    unittest.main()
