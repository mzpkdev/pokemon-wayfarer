"""Structural gates for reviewed Trainer classification and roster ownership."""
import copy
import unittest
import generate as gen


def roster(slots=None, **fields):
    return dict(slots=slots if slots is not None else [{'species': 'SPECIES_RATTATA', 'lvl': 10}], partySize=1, trainerClass='TRAINER_CLASS_YOUNGSTER', **fields)


class InventoryTests(unittest.TestCase):
    def test_real_compiler_parses_custom_move_tuple(self):
        records = gen.load_inventory()
        custom = [slot for rows in records.values() for row in rows.values() for slot in row['slots'] if 'moves' in slot]
        self.assertGreater(len(custom), 100)
        self.assertTrue(all(1 <= len(slot['moves']) <= 4 for slot in custom))

    def test_real_compiler_uses_wayfarer_roster_macros(self):
        records = gen.load_inventory()
        self.assertEqual(records['TRAINER_PHOEBE']['DIFFICULTY_NORMAL']['slots'][0]['lvl'], 87)

    def test_alias_chain_and_pool_keep_original_slots(self):
        records = {'TRAINER_A': {'DIFFICULTY_NORMAL': roster([], overrideTrainer='TRAINER_B')}, 'TRAINER_B': {'DIFFICULTY_NORMAL': roster([], overrideTrainer='TRAINER_C')}, 'TRAINER_C': {'DIFFICULTY_NORMAL': roster([{'species': 'SPECIES_RATTATA', 'lvl': 10}, {'species': 'SPECIES_PIDGEY', 'lvl': 20}], poolSize=2)}}
        resolved = gen.resolve_rosters(records)['TRAINER_A']['DIFFICULTY_HARD']
        self.assertEqual(resolved['owner'], 'TRAINER_C')
        self.assertEqual(len(resolved['slots']), 2)
        self.assertEqual(resolved['partySize'], 1)
        self.assertTrue(resolved['money_party_null'])
        self.assertEqual(records['TRAINER_A']['DIFFICULTY_NORMAL']['slots'], [])

    def test_alias_cycle_and_unknown_owner_fail(self):
        for target in ('TRAINER_A', 'TRAINER_UNKNOWN'):
            with self.assertRaises(gen.ValidationError):
                gen.resolve_rosters({'TRAINER_A': {'DIFFICULTY_NORMAL': roster([], overrideTrainer=target)}})

    def test_all_difficulty_variants(self):
        records = {'TRAINER_A': {'DIFFICULTY_NORMAL': roster(), 'DIFFICULTY_HARD': roster([{'species': 'SPECIES_RATICATE', 'lvl': 40}])}}
        resolved = gen.resolve_rosters(records)['TRAINER_A']
        self.assertEqual(resolved['DIFFICULTY_EASY']['slots'][0]['lvl'], 10)
        self.assertEqual(resolved['DIFFICULTY_HARD']['slots'][0]['lvl'], 40)

    def test_missing_duplicate_and_unknown_manifest_ids_fail(self):
        records = {'TRAINER_A': {'DIFFICULTY_NORMAL': roster()}}
        valid = {'id': 'TRAINER_A', 'policy': 'ORDINARY', 'evidence': [{'path': 'src/data/trainers.party', 'symbol': 'TRAINER_SAWYER_1'}]}
        for rows in ([], [valid, valid], [dict(valid, id='TRAINER_UNKNOWN')]):
            with self.assertRaises(gen.ValidationError):
                gen.validate_manifest({'version': 1, 'records': rows}, records, {'TRAINER_A': 1}, {})

    def test_script_reference_to_hole_is_fatal(self):
        with self.assertRaisesRegex(gen.ValidationError, 'hole or empty'):
            gen.validate_manifest({'version': 1, 'records': []}, {}, {}, {'TRAINER_GONE': ['map']})

    def test_sparse_ids_are_not_missing_records(self):
        gen.validate_manifest({'version': 1, 'records': []}, {'TRAINER_HOLE': {'DIFFICULTY_NORMAL': roster([])}}, {'TRAINER_HOLE': 700}, {})

    def test_stale_evidence_and_missing_exclusion_reason_fail(self):
        records = {'TRAINER_A': {'DIFFICULTY_NORMAL': roster()}}
        for row in ({'id': 'TRAINER_A', 'policy': 'EXCLUDED', 'evidence': []}, {'id': 'TRAINER_A', 'policy': 'ORDINARY', 'evidence': [{'path': 'src/data/trainers.party', 'symbol': 'TRAINER_NOT_REAL'}]}):
            with self.assertRaises(gen.ValidationError):
                gen.validate_manifest({'version': 1, 'records': [row]}, records, {'TRAINER_A': 1}, {})

    def test_shared_gym_role_requires_explicit_review(self):
        records = {'TRAINER_A': {'DIFFICULTY_NORMAL': roster()}}
        row = {'id': 'TRAINER_A', 'policy': 'GYM_MEMBER', 'evidence': [{'path': 'src/data/trainers.party', 'symbol': 'TRAINER_SAWYER_1'}]}
        with self.assertRaisesRegex(gen.ValidationError, 'requires review'):
            gen.validate_manifest({'version': 1, 'records': [row]}, records, {'TRAINER_A': 1}, {'TRAINER_A': ['data/maps/Town_Gym/scripts.inc', 'data/maps/Road/scripts.inc']})

    def test_hoenn_selection_is_fixed_normal(self):
        records = {'TRAINER_A': {'DIFFICULTY_NORMAL': roster(source='src/data/trainers.party'), 'DIFFICULTY_HARD': roster([{'species': 'SPECIES_RATICATE', 'lvl': 50}], source='src/data/trainers.party')}}
        resolved = gen.resolve_rosters(records)['TRAINER_A']['DIFFICULTY_HARD']
        self.assertEqual(resolved['owner_variant'], 'DIFFICULTY_NORMAL')
        self.assertEqual(resolved['slots'][0]['lvl'], 10)

    def test_unknown_schema_version_fails(self):
        with self.assertRaisesRegex(gen.ValidationError, 'manifest version'):
            gen.validate_manifest({'version': 2, 'records': []}, {}, {}, {})

    def test_proposal_does_not_match_boss_name_substrings(self):
        records = {'TRAINER_ALFRED_HNS': {'DIFFICULTY_NORMAL': roster(source='src/data/trainers_hns.party')}}
        self.assertEqual(gen.propose(records, {})['records'][0]['policy'], 'ORDINARY')

if __name__ == '__main__': unittest.main()
