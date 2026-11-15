"""Every legendary is reachable and gated like its peers (Wayfarer build).

Runs the Wayfarer-preprocessed scripts through the interpreter in test_scripts.py: Steven's Vermilion
Lati scene waits for the Kanto championship, the Littleroot TV choice never restarts the Johto beasts,
Southern Island's Lati needs only the Eon Ticket charter, and the Mt. Silver Moltres and Cerulean Cave
Mewtwo wait for the legendary Trainer Rating like the other birds.
"""
import re
import unittest

from test_scripts import Interp, source

STEVEN_TRIGGERS = ('VermilionCity_EventScript_TriggerStevenLeft', 'VermilionCity_EventScript_TriggerStevenMiddle',
                   'VermilionCity_EventScript_TriggerStevenRight')


class StevenVermilionLati(unittest.TestCase):
    def test_every_trigger_waits_for_the_kanto_champion(self):
        for label in STEVEN_TRIGGERS:
            interp = Interp('VermilionCity_hns')
            interp.run(label)
            self.assertNotIn('InitKantoRoamers', interp.called, label)
            self.assertNotIn('FLAG_LATIOS_OR_LATIAS_ROAMING', interp.flags, label)
            self.assertNotIn('VAR_VERMILION_CITY_STEVEN', interp.written('var'), label)
            self.assertFalse(interp.locked, label)

    def test_a_kanto_champion_meets_steven_once(self):
        for label in STEVEN_TRIGGERS:
            interp = Interp('VermilionCity_hns', flags={'FLAG_IS_KANTO_CHAMPION'})
            interp.run(label)
            self.assertIn('InitKantoRoamers', interp.called, label)
            self.assertIn('FLAG_LATIOS_OR_LATIAS_ROAMING', interp.flags, label)
            self.assertEqual(interp.vars['VAR_VERMILION_CITY_STEVEN'], 1, label)


class LittlerootTvLati(unittest.TestCase):
    def test_the_colour_choice_never_restarts_a_roamer(self):
        interp = Interp('data/scripts/players_house.inc', vars={'VAR_0x8008': 0, 'VAR_RESULT': 1})
        interp.run('PlayersHouse_1F_EventScript_SeeLatiTV')
        self.assertNotIn('InitRoamer', interp.called)
        self.assertIn('VAR_ROAMER_POKEMON', interp.written('var'))

    def test_no_wayfarer_tv_script_calls_init_roamer(self):
        for path in ('data/scripts/players_house.inc', 'data/scripts/tv.inc'):
            self.assertIsNone(re.search(r'^\s*special\s+InitRoamer\b', source(path), re.MULTILINE), path)


class SouthernIslandLati(unittest.TestCase):
    def test_arriving_by_charter_is_enough(self):
        # FLAG_ENABLE_SHIP_SOUTHERN_ISLAND comes only from the cable club; Wayfarer never sets it.
        interp = Interp('SouthernIsland_Interior')
        interp.run('SouthernIsland_Interior_EventScript_Lati')
        self.assertIn('BattleSetup_StartLatiBattle', interp.called)
        self.assertNotIn('SouthernIsland_Interior_Text_Sign', interp.messages)

    def test_a_resolved_lati_stays_resolved(self):
        for flag in ('FLAG_DEFEATED_LATIAS_OR_LATIOS', 'FLAG_CAUGHT_LATIAS_OR_LATIOS'):
            interp = Interp('SouthernIsland_Interior', flags={flag})
            interp.run('SouthernIsland_Interior_EventScript_Lati')
            self.assertNotIn('BattleSetup_StartLatiBattle', interp.called, flag)


class LegendaryTrainerRating(unittest.TestCase):
    CASES = (
        ('MtSilver_1F_MoltresRoom_hns', 'MtSilver_1F_MoltresRoom_EventScript_Moltres',
         'WayfarerMtSilver_IsMoltresEligible', 'MtSilver_1F_MoltresRoom_Text_MoltresNotReady', 'FLAG_HIDE_MOLTRES'),
        ('CeruleanCave_B2F_hns', 'CeruleanCave3_EventScript_MewtwoNew',
         'WayfarerCeruleanCave_IsMewtwoEligible', 'CeruleanCave3_Text_MewtwoNotReady', 'FLAG_HIDE_MEWTWO'),
    )

    def test_below_the_threshold_the_legendary_waits(self):
        for map_name, label, special, text, hide in self.CASES:
            interp = Interp(map_name)
            interp.specials[special] = 0
            interp.run(label)
            self.assertEqual(interp.messages, [text], label)
            self.assertNotIn('BattleSetup_StartLegendaryBattle', interp.called, label)
            self.assertNotIn(hide, interp.flags, label)
            self.assertFalse(interp.locked, label)

    def test_at_the_threshold_the_battle_starts(self):
        for map_name, label, special, text, hide in self.CASES:
            interp = Interp(map_name)
            interp.specials[special] = 1
            interp.run(label)
            self.assertNotIn(text, interp.messages, label)
            self.assertIn('BattleSetup_StartLegendaryBattle', interp.called, label)
            self.assertIn(hide, interp.flags, label)

    def test_not_ready_text_never_names_a_rating(self):
        for map_name, *_ in self.CASES:
            self.assertNotRegex(source(map_name), r'(?i)rating\s*\d|TR\s*\d', map_name)
        self.assertNotRegex(source('data/scripts/wayfarer_sevii/story/celio_network.inc'), r'(?i)rating\s*\d')


if __name__ == '__main__':
    unittest.main()
