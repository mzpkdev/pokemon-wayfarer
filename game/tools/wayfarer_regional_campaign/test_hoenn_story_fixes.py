"""Hoenn story-beat audit fixes: the Littleroot S.S. Ticket scene for visitors, the Groudon-only invasion trigger.

The scripts run through the Johto story tests' interpreter, which checks state transactions and not presentation.
The household policy is read through a special that the interpreter answers from its Johto-origin switch, so
`johto=True` plays a native household and `johto=False` a visitor's.
"""
from pathlib import Path
import importlib.util
import re
import subprocess
import unittest

GAME = Path(__file__).resolve().parents[2]

_spec = importlib.util.spec_from_file_location('johto_story_test_scripts', GAME / 'tools/johto_story/test_scripts.py')
_johto = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_johto)

DAD_TEXTS = ('PlayersHouse_1F_Text_TicketFromBrineyCameForYou', 'PlayersHouse_1F_Text_PortsInSlateportLilycove',
             'PlayersHouse_1F_Text_BetterGetBackToGym', 'PlayersHouse_1F_Text_DadShouldStayLonger')
GROUDON = 'FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT'
INVASION = 'VAR_MOSSDEEP_CITY_STATE'
SPACE_CENTER = 'VAR_MOSSDEEP_SPACE_CENTER_STATE'


def preprocess(path):
    text = subprocess.check_output([
        'cpp', '-P', '-x', 'assembler-with-cpp', '-I%s' % (GAME / 'include'),
        '-DIS_WAYFARER=1', '-DWAYFARER_LEAGUE_CIRCUIT_ENABLED=1', str(path),
    ], text=True)
    return text.replace('WayfarerUsesNativeHoennHousehold', 'WayfarerIsJohtoOrigin')


def play(files, label, *, native, flags=(), vars=None, items=()):
    interp = _johto.Interp(johto=native, flags=flags, vars=vars)
    for path in files:
        for line in preprocess(GAME / path).splitlines():
            interp.lines.append(line)
            stripped = line.split('@')[0].strip()
            if re.fullmatch(r'\w+:+', stripped):
                interp.labels.setdefault(stripped.rstrip(':'), len(interp.lines))
    interp.items = set(items)
    interp.run(label)
    return interp


def play_map(map_name, label, **kwargs):
    return play(['data/maps/%s/scripts.inc' % map_name], label, **kwargs)


def play_scene(*, native, **kwargs):
    # The scene is common script text that every Littleroot house calls from its frame table
    return play(['data/scripts/players_house.inc'], 'PlayersHouse_1F_EventScript_GetSSTicketAndSeeLatiTV',
                native=native, **kwargs)


class LittlerootTicketScene(unittest.TestCase):
    def test_the_hall_of_fame_brings_norman_home_only_for_a_native_household(self):
        for native in (True, False):
            hall = play(['data/scripts/hall_of_fame.inc'], 'EverGrandeCity_HallOfFame_EventScript_ReadyReceiveSSTicketEvent',
                        native=native)
            with self.subTest(native=native):
                self.assertEqual(hall.get('VAR_LITTLEROOT_HOUSES_STATE_MAY'), 3)
                self.assertEqual(hall.get('VAR_LITTLEROOT_HOUSES_STATE_BRENDAN'), 3)
                self.assertEqual(('FLAG_HIDE_PLAYERS_HOUSE_DAD', 0, 'flag') in hall.writes, native)

    def test_a_visitor_gets_the_ticket_and_the_roamer_without_a_father(self):
        scene = play_scene(native=False)
        self.assertEqual(scene.given, ['ITEM_SS_TICKET'])
        self.assertIn('FLAG_RECEIVED_SS_TICKET', scene.flags)
        self.assertIn('FLAG_LATIOS_OR_LATIAS_ROAMING', scene.flags)
        self.assertEqual(scene.get('VAR_LITTLEROOT_HOUSES_STATE_MAY'), 4)
        self.assertEqual(scene.get('VAR_LITTLEROOT_HOUSES_STATE_BRENDAN'), 4)
        for text in DAD_TEXTS:
            self.assertNotIn(text, scene.messages)
        self.assertIn('PlayersHouse_1F_Text_VisitorTicketFromBriney', scene.messages)

    def test_a_visitor_holding_a_ticket_is_not_given_a_second_but_still_sees_the_tv(self):
        scene = play_scene(native=False, items=['ITEM_SS_TICKET'])
        self.assertEqual(scene.given, [])
        self.assertIn('FLAG_RECEIVED_SS_TICKET', scene.flags)
        self.assertIn('FLAG_LATIOS_OR_LATIAS_ROAMING', scene.flags)

    def test_the_visitor_variant_never_addresses_the_dad_object(self):
        source = preprocess(GAME / 'data/scripts/players_house.inc')
        start = source.index('PlayersHouse_1F_EventScript_VisitorGetSSTicketAndSeeLatiTV::')
        block = source[start:source.index('PlayersHouse_1F_Text_VisitorTicketFromBriney:')]
        self.assertNotIn('VAR_0x8009', block)
        self.assertNotIn('Dad', block.replace('MomApproachDad', ''))

    def test_a_native_household_keeps_the_original_scene(self):
        scene = play_scene(native=True)
        for text in DAD_TEXTS:
            self.assertIn(text, scene.messages)
        self.assertEqual(scene.given, ['ITEM_SS_TICKET'])
        self.assertIn('FLAG_LATIOS_OR_LATIAS_ROAMING', scene.flags)
        self.assertNotIn('PlayersHouse_1F_Text_VisitorTicketFromBriney', scene.messages)


class MossdeepInvasion(unittest.TestCase):
    def transition(self, flags=(), **vars):
        return play_map('MossdeepCity', 'MossdeepCity_OnTransition', native=True, flags=flags, vars=vars)

    def test_groudon_alone_starts_the_invasion(self):
        started = self.transition(flags=[GROUDON])
        self.assertEqual(started.get(INVASION), 1)
        self.assertEqual(started.get(SPACE_CENTER), 1)
        self.assertIn(('FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_STEVEN', 0, 'flag'), started.writes)

    def test_the_mind_badge_alone_starts_nothing(self):
        quiet = self.transition(flags=['FLAG_BADGE07_GET'])
        self.assertEqual(quiet.get(INVASION), 0)
        self.assertNotIn(SPACE_CENTER, [write[0] for write in quiet.writes])

    def test_a_native_who_beat_tate_and_liza_first_still_gets_it_once(self):
        flags = [GROUDON, 'FLAG_BADGE07_GET']
        self.assertEqual(self.transition(flags=flags).get(INVASION), 1)
        for state in (1, 2, 3):
            again = self.transition(flags=flags, **{INVASION: state})
            self.assertEqual(again.get(INVASION), state)
            self.assertNotIn(SPACE_CENTER, [write[0] for write in again.writes])

    def test_the_gym_win_writes_no_invasion_state(self):
        gym = (GAME / 'data/maps/MossdeepCity_Gym/scripts.inc').read_text()
        for name in (INVASION, SPACE_CENTER, 'FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_STEVEN'):
            self.assertNotIn(name, gym)

    def test_the_space_center_ground_floor_never_shows_a_waiting_steven(self):
        for state in (0, 1, 2):
            hidden = play_map('MossdeepCity_SpaceCenter_1F', 'MossdeepCity_SpaceCenter_1F_OnTransition', native=True,
                              vars={INVASION: state})
            self.assertIn(('FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_1F_STEVEN', 1, 'flag'), hidden.writes)


if __name__ == '__main__':
    unittest.main()
