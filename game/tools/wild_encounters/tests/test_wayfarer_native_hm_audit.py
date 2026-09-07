import importlib.util
from fractions import Fraction
from pathlib import Path
import sys
import unittest

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))
SPEC = importlib.util.spec_from_file_location('wayfarer_native_hm_audit', TOOLS / 'wayfarer_native_hm_audit.py')
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


class WayfarerKnownMoveAuditTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.model = AUDIT.Model()

    def test_model_reads_unselected_species_native_utilities(self):
        self.assertNotIn('SPECIES_RAYQUAZA', self.model.roster)
        self.assertIn('MOVE_FLY', self.model.windows['SPECIES_RAYQUAZA', 'modern', 100])
        self.assertEqual(len({species for species, _, _ in self.model.windows}), 1333)

    def test_known_move_aggregation_does_not_require_a_named_anchor(self):
        identity = ('MAP_MOSSDEEP_CITY', 'fishing_mons', 'TIME_DAY', 'OLD_ROD')
        probability, witness = self.model.measure(identity, 'modern', 'MOVE_SURF', 80)
        self.assertGreaterEqual(probability, Fraction(2, 25))
        self.assertEqual(sum((Fraction(**row['probability']) for row in witness['carriers']), Fraction(0)), probability)
        self.assertEqual(Fraction(**witness['unmodifiedCastProbability']), probability / 4)
        self.assertNotIn('SPECIES_WAILMER', {row['species'] for row in witness['carriers']})

    def test_expected_utility_absence_is_preserved(self):
        self.assertIn('MOVE_SURF', self.model.windows['SPECIES_PSYDUCK', 'modern', 21])
        self.assertNotIn('MOVE_SURF', self.model.windows['SPECIES_PSYDUCK', 'modern', 22])
        self.assertIn('MOVE_DIVE', self.model.windows['SPECIES_PSYDUCK', 'modern', 22])

    def test_out_of_scope_regional_witness_is_explicit(self):
        context = AUDIT.source_context('MAP_WHIRL_ISLANDS_B2F_HNS')
        self.assertTrue(context['outOfScopeArea'])
        self.assertTrue(context['optionalArea'])
        self.assertEqual(context['reachability'], 'NOT_VERIFIED')
