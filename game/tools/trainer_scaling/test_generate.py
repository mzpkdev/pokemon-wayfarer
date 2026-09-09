"""Structural gates for reviewed Trainer classification and roster ownership."""
import copy
import json
import unittest
import generate as gen


SS_ANNE_TRAINERS = (
    'TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS',
    'TRAINER_SS_ANNE_LASS_ANN_HNS',
    'TRAINER_SS_ANNE_LASS_DAWN_HNS',
    'TRAINER_SS_ANNE_SAILOR_EDMOND_HNS',
    'TRAINER_SS_ANNE_SAILOR_TREVOR_HNS',
    'TRAINER_SS_ANNE_SAILOR_LEONARD_HNS',
    'TRAINER_SS_ANNE_SAILOR_DUNCAN_HNS',
    'TRAINER_SS_ANNE_SAILOR_HUEY_HNS',
    'TRAINER_SS_ANNE_SAILOR_DYLAN_HNS',
    'TRAINER_SS_ANNE_SAILOR_PHILLIP_HNS',
    'TRAINER_SS_ANNE_FISHERMAN_DALE_HNS',
    'TRAINER_SS_ANNE_FISHERMAN_BARNY_HNS',
    'TRAINER_SS_ANNE_GENTLEMAN_THOMAS_HNS',
    'TRAINER_SS_ANNE_GENTLEMAN_ARTHUR_HNS',
    'TRAINER_SS_ANNE_GENTLEMAN_BROOKS_HNS',
    'TRAINER_SS_ANNE_GENTLEMAN_LAMAR_HNS',
)

SS_ANNE_PARTIES = {
    'TRAINER_SS_ANNE_YOUNGSTER_TYLER_HNS': (('SPECIES_NIDORAN_M', 21),),
    'TRAINER_SS_ANNE_LASS_ANN_HNS': (('SPECIES_PIDGEY', 18), ('SPECIES_NIDORAN_F', 18)),
    'TRAINER_SS_ANNE_LASS_DAWN_HNS': (('SPECIES_RATTATA', 18), ('SPECIES_PIKACHU', 18)),
    'TRAINER_SS_ANNE_SAILOR_EDMOND_HNS': (('SPECIES_MACHOP', 18), ('SPECIES_SHELLDER', 18)),
    'TRAINER_SS_ANNE_SAILOR_TREVOR_HNS': (('SPECIES_MACHOP', 17), ('SPECIES_TENTACOOL', 17)),
    'TRAINER_SS_ANNE_SAILOR_LEONARD_HNS': (('SPECIES_SHELLDER', 21),),
    'TRAINER_SS_ANNE_SAILOR_DUNCAN_HNS': (('SPECIES_HORSEA', 17), ('SPECIES_SHELLDER', 17), ('SPECIES_TENTACOOL', 17)),
    'TRAINER_SS_ANNE_SAILOR_HUEY_HNS': (('SPECIES_TENTACOOL', 18), ('SPECIES_STARYU', 18)),
    'TRAINER_SS_ANNE_SAILOR_DYLAN_HNS': (('SPECIES_HORSEA', 17), ('SPECIES_HORSEA', 17), ('SPECIES_HORSEA', 17)),
    'TRAINER_SS_ANNE_SAILOR_PHILLIP_HNS': (('SPECIES_MACHOP', 20),),
    'TRAINER_SS_ANNE_FISHERMAN_DALE_HNS': (('SPECIES_GOLDEEN', 17), ('SPECIES_TENTACOOL', 17), ('SPECIES_GOLDEEN', 17)),
    'TRAINER_SS_ANNE_FISHERMAN_BARNY_HNS': (('SPECIES_TENTACOOL', 17), ('SPECIES_STARYU', 17), ('SPECIES_SHELLDER', 17)),
    'TRAINER_SS_ANNE_GENTLEMAN_THOMAS_HNS': (('SPECIES_GROWLITHE', 18), ('SPECIES_GROWLITHE', 18)),
    'TRAINER_SS_ANNE_GENTLEMAN_ARTHUR_HNS': (('SPECIES_NIDORAN_M', 19), ('SPECIES_NIDORAN_F', 19)),
    'TRAINER_SS_ANNE_GENTLEMAN_BROOKS_HNS': (('SPECIES_PIKACHU', 23),),
    'TRAINER_SS_ANNE_GENTLEMAN_LAMAR_HNS': (('SPECIES_GROWLITHE', 17), ('SPECIES_PONYTA', 17)),
}

CELADON_HIDEOUT_TRAINERS = tuple(
    [f'TRAINER_CELADON_HIDEOUT_GRUNT_{number}_HNS' for number in range(7, 19)]
    + ['TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS']
)

CELADON_HIDEOUT_PARTIES = {
    'TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS': (('SPECIES_RATICATE', 20), ('SPECIES_ZUBAT', 20)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_8_HNS': (('SPECIES_DROWZEE', 21), ('SPECIES_MACHOP', 21)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_9_HNS': (('SPECIES_RATICATE', 21), ('SPECIES_RATICATE', 21)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_10_HNS': (('SPECIES_GRIMER', 20), ('SPECIES_KOFFING', 20), ('SPECIES_KOFFING', 20)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_11_HNS': (('SPECIES_RATTATA', 19), ('SPECIES_RATICATE', 19), ('SPECIES_RATICATE', 19), ('SPECIES_RATTATA', 19)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_12_HNS': (('SPECIES_GRIMER', 22), ('SPECIES_KOFFING', 22)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_13_HNS': (('SPECIES_ZUBAT', 17), ('SPECIES_KOFFING', 17), ('SPECIES_GRIMER', 17), ('SPECIES_ZUBAT', 17), ('SPECIES_RATICATE', 17)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_14_HNS': (('SPECIES_RATTATA', 20), ('SPECIES_RATICATE', 20), ('SPECIES_DROWZEE', 20)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_15_HNS': (('SPECIES_MACHOP', 21), ('SPECIES_MACHOP', 21)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_16_HNS': (('SPECIES_SANDSHREW', 23), ('SPECIES_EKANS', 23), ('SPECIES_SANDSLASH', 23)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_17_HNS': (('SPECIES_EKANS', 23), ('SPECIES_SANDSHREW', 23), ('SPECIES_ARBOK', 23)),
    'TRAINER_CELADON_HIDEOUT_GRUNT_18_HNS': (('SPECIES_KOFFING', 21), ('SPECIES_ZUBAT', 21)),
    'TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS': (('SPECIES_ONIX', 25), ('SPECIES_RHYHORN', 24), ('SPECIES_KANGASKHAN', 29)),
}


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

    def test_persistent_anne_trainer_catalog_is_complete_and_wayfarer_only(self):
        records = gen.load_inventory()
        ids = gen.trainer_ids(SS_ANNE_TRAINERS)
        self.assertEqual(
            [ids[trainer] for trainer in SS_ANNE_TRAINERS],
            list(range(1515, 1531)),
        )
        self.assertEqual(set(SS_ANNE_PARTIES), set(SS_ANNE_TRAINERS))
        self.assertEqual(len(set(ids.values())), 16)
        for trainer in SS_ANNE_TRAINERS:
            roster = records[trainer]['DIFFICULTY_NORMAL']
            self.assertEqual(roster['source'], 'src/data/trainers_wayfarer.party')
            actual_party = tuple(
                (slot['species'], slot['lvl'])
                for slot in roster['slots'][:roster['partySize']]
            )
            self.assertEqual(actual_party, SS_ANNE_PARTIES[trainer])

        manifest = json.loads(gen.MANIFEST.read_text())
        rows = {
            row['id']: row
            for row in manifest['records']
            if row['id'] in SS_ANNE_TRAINERS
        }
        self.assertEqual(set(rows), set(SS_ANNE_TRAINERS))
        for trainer, row in rows.items():
            self.assertEqual(row['policy'], 'ORDINARY')
            self.assertTrue(any(
                evidence['path'] == 'src/data/trainers_wayfarer.party'
                and evidence['symbol'] == trainer
                for evidence in row['evidence']
            ))

    def test_nugget_bridge_preserves_source_parties_and_ordinary_scaling(self):
        names = ['CALE', 'ALI', 'TIMMY', 'RELI', 'ETHAN', 'ROCKET']
        sources = ['BUG_CATCHER_CALE', 'LASS_ALI', 'YOUNGSTER_TIMMY',
                   'LASS_RELI', 'CAMPER_ETHAN', 'TEAM_ROCKET_GRUNT_6']
        trainers = ['TRAINER_NUGGET_BRIDGE_' + name + '_HNS' for name in names]
        ids = gen.trainer_ids(trainers)
        self.assertEqual([ids[trainer] for trainer in trainers], list(range(1550, 1556)))
        authored = (gen.ROOT / 'src/data/trainers_wayfarer.party').read_text()
        source = (gen.ROOT / 'src/data/trainers_frlg.party').read_text()
        manifest = {row['id']: row for row in json.loads(gen.MANIFEST.read_text())['records']}
        records = gen.load_inventory()
        for trainer, original in zip(trainers, sources):
            target_party = authored.split('=== ' + trainer + ' ===')[1].split('===')[0].strip()
            source_party = source.split('=== TRAINER_' + original + ' ===')[1].split('===')[0].strip()
            self.assertEqual(target_party, source_party)
            self.assertEqual(records[trainer]['DIFFICULTY_NORMAL']['source'], 'src/data/trainers_wayfarer.party')
            self.assertEqual((manifest[trainer]['policy'], manifest[trainer]['region']), ('ORDINARY', 'Kanto'))

    def test_cerulean_burglary_retains_authored_party_and_ordinary_scaling(self):
        trainer = 'TRAINER_CERULEAN_BURGLARY_GRUNT_HNS'
        self.assertEqual(gen.trainer_ids([trainer])[trainer], 1549)
        roster = gen.load_inventory()[trainer]['DIFFICULTY_NORMAL']
        self.assertEqual(roster['source'], 'src/data/trainers_wayfarer.party')
        self.assertEqual(tuple((slot['species'], slot['lvl'])
                              for slot in roster['slots'][:roster['partySize']]),
                         (('SPECIES_MACHOP', 17), ('SPECIES_DROWZEE', 17)))
        row = next(row for row in json.loads(gen.MANIFEST.read_text())['records']
                   if row['id'] == trainer)
        self.assertEqual((row['policy'], row['region']), ('ORDINARY', 'Kanto'))

    def test_celadon_hideout_trainer_catalog_is_complete_and_classified(self):
        records = gen.load_inventory()
        ids = gen.trainer_ids(CELADON_HIDEOUT_TRAINERS)
        self.assertEqual(
            [ids[trainer] for trainer in CELADON_HIDEOUT_TRAINERS],
            list(range(1531, 1544)),
        )
        self.assertEqual(set(CELADON_HIDEOUT_PARTIES), set(CELADON_HIDEOUT_TRAINERS))
        self.assertEqual(len(set(ids.values())), 13)
        for trainer in CELADON_HIDEOUT_TRAINERS:
            roster = records[trainer]['DIFFICULTY_NORMAL']
            self.assertEqual(roster['source'], 'src/data/trainers_wayfarer.party')
            actual_party = tuple(
                (slot['species'], slot['lvl'])
                for slot in roster['slots'][:roster['partySize']]
            )
            self.assertEqual(actual_party, CELADON_HIDEOUT_PARTIES[trainer])

        manifest = json.loads(gen.MANIFEST.read_text())
        rows = {
            row['id']: row
            for row in manifest['records']
            if row['id'] in CELADON_HIDEOUT_TRAINERS
        }
        self.assertEqual(set(rows), set(CELADON_HIDEOUT_TRAINERS))
        for trainer, row in rows.items():
            expected_policy = 'EXCLUDED' if trainer.endswith('GIOVANNI_HNS') else 'ORDINARY'
            self.assertEqual(row['policy'], expected_policy)
            if expected_policy == 'EXCLUDED':
                self.assertTrue(row['reason'])
            self.assertTrue(any(
                evidence['path'] == 'src/data/trainers_wayfarer.party'
                and evidence['symbol'] == trainer
                for evidence in row['evidence']
            ))

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
