"""Pins the Pokédex obtainability audit (spec: home-pokedex-lists.md, "Obtainability").

A change that strands a species (an encounter table, a gift script, an evolution item) fails here,
naming the National entry, until the species gets another source or is marked dexNotRequired and
the record is re-written with `obtainability.py --write`.
"""
import copy
import json
from pathlib import Path
import sys
import unittest

TOOL = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOL))
import obtainability as O  # noqa: E402

AUDIT = None


def setUpModule():
    global AUDIT
    AUDIT = O.Audit()


def record():
    return json.loads(O.RECORD.read_text())


def without_source(audit, *species, kinds=('wild', 'script', 'roamer', 'trade')):
    """A copy of the audit solved again with the given species' sources of these kinds removed."""
    stranded = copy.copy(audit)
    stranded.sources = {kind: {name: where for name, where in found.items() if kind not in kinds or name not in species}
                        for kind, found in audit.sources.items()}
    stranded.solve()
    return stranded


class RecordedResult(unittest.TestCase):
    def test_world_matches_the_record_and_the_species_flags(self):
        found = O.problems(AUDIT, record())
        self.assertEqual(found, [], '\n'.join(found))

    def test_pinned_counts(self):
        document = record()
        self.assertEqual(document['enabledEntries'], len(AUDIT.entries))
        self.assertEqual((len(AUDIT.entries), len(document['unobtainable']), len(document['excusedButObtainable'])),
                         (968, 128, 88))
        by_nat = dict(AUDIT.entries)
        self.assertEqual([nat for nat in AUDIT.unobtainable() if AUDIT.required(nat, by_nat[nat])], [])

    def test_c_test_table_matches_the_record(self):
        self.assertIsNone(O.c_test_drift(record()))

    def test_prd_gen_v_species_are_unobtainable(self):
        unobtainable = {row['nat'] for row in record()['unobtainable']}
        for name in ('TIRTOUGA', 'CARRACOSTA', 'ARCHEN', 'ARCHEOPS', 'COBALION', 'TERRAKION', 'VIRIZION',
                     'TORNADUS', 'THUNDURUS', 'RESHIRAM', 'ZEKROM', 'LANDORUS', 'KYUREM'):
            self.assertIn(f'NATIONAL_DEX_{name}', unobtainable)


class Stranding(unittest.TestCase):
    def test_removing_the_only_source_of_a_species_fails_with_its_name(self):
        self.assertTrue('SPECIES_TROPIUS' in AUDIT.obtained)
        stranded = without_source(AUDIT, 'SPECIES_TROPIUS')
        found = O.problems(stranded, record())
        self.assertTrue(any(line.startswith('NATIONAL_DEX_TROPIUS is no longer obtainable') for line in found), found)
        self.assertTrue(any('NATIONAL_DEX_TROPIUS (SPECIES_TROPIUS) is unobtainable but the Pokédex requires it' in line
                            for line in found), found)

    def test_a_legendary_with_one_live_script_is_guarded(self):
        # Zapdos's FRLG Power Plant and its `#if !IS_WAYFARER` Route 10 script are dead in this build.
        self.assertEqual(AUDIT.sources['script']['SPECIES_ZAPDOS'], ['seteventmon data/scripts/wayfarer_power_plant.inc'])
        stranded = without_source(AUDIT, 'SPECIES_ZAPDOS', kinds=('script',))
        self.assertTrue(any(line.startswith('NATIONAL_DEX_ZAPDOS is no longer obtainable')
                            for line in O.problems(stranded, record())))

    def test_every_starter_is_obtainable_without_gifts(self):
        # The starter gifts are pick-one; at a low Trainer Rating the wild final-stage slots step
        # down to the basic forms, and the Day Care breeds them anyway.
        starters = ['SPECIES_' + name for name in ('BULBASAUR CHARMANDER SQUIRTLE CHIKORITA CYNDAQUIL TOTODILE '
                                                    'TREECKO TORCHIC MUDKIP SNIVY TEPIG OSHAWOTT CHESPIN FENNEKIN '
                                                    'FROAKIE ROWLET LITTEN POPPLIO GROOKEY SCORBUNNY SOBBLE').split()]
        stranded = without_source(AUDIT, *starters, kinds=('script',))
        for name in starters:
            self.assertTrue(stranded.how.get(name, '').startswith('wild:'), name)

    def test_low_trainer_rating_steps_a_slot_down(self):
        self.assertIn('wild MAP_ROUTE29_HNS land as a low-level MEGANIUM', AUDIT.sources['wild']['SPECIES_CHIKORITA'])

    def test_a_species_with_other_sources_survives(self):
        # Kecleon is wild on Routes 118-120 and also a scripted encounter.
        stranded = without_source(AUDIT, 'SPECIES_KECLEON', kinds=('script',))
        self.assertTrue('SPECIES_KECLEON' in stranded.obtained)


class Rules(unittest.TestCase):
    def test_evolution_needs_an_item_place_and_solo_method(self):
        self.assertEqual(AUDIT.blocked({'method': 'EVO_ITEM', 'param': 'ITEM_NOT_IN_GAME', 'conditions': []}),
                         'ITEM_NOT_IN_GAME is not in the world')
        self.assertEqual(AUDIT.blocked({'method': 'EVO_TRADE', 'param': '0', 'conditions': []}), 'EVO_TRADE only')
        self.assertIn('not a Wayfarer place', AUDIT.blocked(
            {'method': 'EVO_LEVEL', 'param': '0', 'conditions': [('IF_IN_MAPSEC', 'MAPSEC_NOWHERE')]}))
        self.assertIsNone(AUDIT.blocked({'method': 'EVO_ITEM', 'param': 'ITEM_FIRE_STONE', 'conditions': []}))

    def test_knows_move_needs_a_learnable_move(self):
        evolution = {'method': 'EVO_LEVEL', 'param': '0', 'conditions': [('IF_KNOWS_MOVE', 'MOVE_HYPER_DRILL')]}
        self.assertIsNone(AUDIT.blocked(evolution, 'SPECIES_DUNSPARCE'))
        self.assertIn('cannot learn', AUDIT.blocked(evolution, 'SPECIES_PIDGEY'))

    def test_day_care_gender_swaps(self):
        # Nidorina cannot breed; Nidorino with Ditto lays either Nidoran (P_NIDORAN_M_DITTO_BREED).
        self.assertEqual(AUDIT.egg_forms['SPECIES_NIDORINO'] & {'SPECIES_NIDORAN_F', 'SPECIES_NIDORAN_M'},
                         {'SPECIES_NIDORAN_F', 'SPECIES_NIDORAN_M'})
        self.assertFalse(AUDIT.breedable('SPECIES_NIDORINA'))

    def test_trade_needs_the_requested_pokemon(self):
        # Goldenrod's Machop trade asks for a Drowzee; take away Machop's wild slots to see it.
        traded = without_source(AUDIT, 'SPECIES_MACHOP', kinds=('wild',))
        self.assertTrue(traded.how['SPECIES_MACHOP'].startswith('trade:'))
        stranded = without_source(AUDIT, 'SPECIES_MACHOP', 'SPECIES_DROWZEE', 'SPECIES_HYPNO', kinds=('wild',))
        self.assertFalse('SPECIES_DROWZEE' in stranded.obtained)
        self.assertFalse(stranded.how.get('SPECIES_MACHOP', '').startswith('trade:'))

    def test_unreferenced_test_labels_give_nothing(self):
        text = ('Live_Script::\n\tgivemon SPECIES_EEVEE, 5\n\tend\n'
                'Dead_Test::\n\tgivemon SPECIES_MEW, 5\n\tend\n')
        live = ''.join(text[a:b] for a, b in O.live_offsets(text, {'Live_Script'}))
        self.assertIn('SPECIES_EEVEE', live)
        self.assertNotIn('SPECIES_MEW', live)

    def test_a_label_that_falls_through_stays_live(self):
        text = 'Start::\n\tlock\nNext::\n\tgivemon SPECIES_EEVEE, 5\n\tend\n'
        live = ''.join(text[a:b] for a, b in O.live_offsets(text, {'Start'}))
        self.assertIn('SPECIES_EEVEE', live)

    def test_false_gas_blocks_are_dropped(self):
        text = '\n'.join(['a', '.if 0', 'frlg', '.else', 'other', '.endif', '.if 1', 'hns', '.endif', '.if SYMBOL', 'kept', '.endif'])
        self.assertEqual(O.drop_false_blocks(text).split(), ['a', 'other', 'hns', 'kept'])

    def test_frlg_only_scripts_are_not_read(self):
        self.assertNotIn('data/maps/PowerPlant_Frlg/scripts.inc', AUDIT.files)
        self.assertNotIn('SPECIES_ZAPDOS', AUDIT.files['data/maps/Route10_hns/scripts.inc'])

    def test_config_gated_script_blocks_are_live(self):
        # `#if WAYFARER_LEAGUE_EVENTS` (config/league_circuit.h) guards live league scripts.
        self.assertIn('LeagueEvent_EventScript', AUDIT.files['data/scripts/league_circuit.inc'])

    def test_debug_scripts_and_dead_maps_are_not_sources(self):
        self.assertNotIn('data/scripts/debug.inc', AUDIT.files)
        self.assertFalse(any('CeladonCity_GameCorner_PrizeRoom_Frlg' in where
                             for found in AUDIT.sources.values() for sources in found.values() for where in sources))


if __name__ == '__main__':
    unittest.main()
