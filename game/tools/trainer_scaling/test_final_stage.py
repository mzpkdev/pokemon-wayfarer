"""Rules R1-R5 of the final-stage roster authoring pass."""
import unittest

import final_stage as fs

MODEL = None


def setUpModule():
    global MODEL
    MODEL = fs.load_model()


def team(trainer_class, region, *slots, trainer_id='TRAINER_TEST', gender=None):
    """slots: (species, level[, item[, gender]]) with bare names like 'MAGIKARP'."""
    ctx = fs.Context(trainer_id, f'TRAINER_CLASS_{trainer_class}', gender, region)
    mons = []
    for index, slot in enumerate(slots):
        species, level = slot[0], slot[1]
        item = slot[2] if len(slot) > 2 else None
        pokemon_gender = slot[3] if len(slot) > 3 else None
        mons.append(fs.Mon(index, f'SPECIES_{species}', level, item, pokemon_gender, None))
    fs.plan_team(MODEL, ctx, mons)
    return [m.after.removeprefix('SPECIES_') for m in mons], mons


class FinalStageTests(unittest.TestCase):
    def test_r1_every_line_reaches_its_final_stage(self):
        after, _ = team('HIKER', 'Kanto', ('MACHOP', 20), ('ZUBAT', 12), ('ODDISH', 15), ('GROWLITHE', 30))
        self.assertEqual(after, ['MACHAMP', 'CROBAT', 'VILEPLUME', 'ARCANINE'])

    def test_r1_cross_generation_extensions_are_allowed(self):
        after, _ = team('HIKER', 'Kanto', ('PRIMEAPE', 20), ('MAGNETON', 20), ('GIRAFARIG', 20))
        self.assertEqual(after, ['ANNIHILAPE', 'MAGNEZONE', 'FARIGIRAF'])

    def test_r4_six_magikarp_make_one_gyarados(self):
        after, _ = team('FISHERMAN', 'Kanto', *[('MAGIKARP', 10 + i) for i in range(6)])
        self.assertEqual(after.count('GYARADOS'), 1)
        self.assertEqual(after.count('MAGIKARP'), 5)
        self.assertEqual(after[5], 'GYARADOS')  # highest level

    def test_r4_ties_go_to_the_later_slot(self):
        after, _ = team('FISHERMAN', 'Kanto', *[('MAGIKARP', 10)] * 3)
        self.assertEqual(after, ['MAGIKARP', 'MAGIKARP', 'GYARADOS'])

    def test_r4_three_stage_line_with_five_copies(self):
        after, _ = team('HIKER', 'Kanto', *[('GEODUDE', 20)] * 5)
        self.assertEqual(sorted(after), ['GEODUDE', 'GEODUDE', 'GEODUDE', 'GOLEM', 'GRAVELER'])

    def test_r4_two_copies_of_a_three_stage_line_stay_final_plus_base(self):
        after, _ = team('HIKER', 'Kanto', ('ZUBAT', 20), ('ZUBAT', 20))
        self.assertEqual(after, ['ZUBAT', 'CROBAT'])

    def test_r4_existing_final_stage_consumes_the_final_quota(self):
        after, _ = team('FISHERMAN', 'Kanto', ('MAGIKARP', 30), ('GYARADOS', 20))
        self.assertEqual(after, ['MAGIKARP', 'GYARADOS'])

    def test_r4_existing_middle_stage_is_kept(self):
        after, _ = team('TEAM_ROCKET', 'Kanto', ('GOLBAT', 25), ('ZUBAT', 25), ('ZUBAT', 25), ('ZUBAT', 25))
        self.assertEqual(sorted(after), ['CROBAT', 'GOLBAT', 'ZUBAT', 'ZUBAT'])

    def test_r5_exceptions_keep_the_authored_species(self):
        everstone, _ = team('HIKER', 'Kanto', ('MACHOP', 20, 'ITEM_EVERSTONE'))
        self.assertEqual(everstone, ['MACHOP'])
        child, _ = team('TUBER_F', 'Hoenn', ('MARILL', 10))
        self.assertEqual(child, ['MARILL'])
        babies, _ = team('LADY', 'Hoenn', ('AZURILL', 10), ('AZURILL', 10))
        self.assertEqual(babies, ['AZURILL', 'AZURILL'])
        special, _ = team('EXPERT', 'Kanto', ('COSMOG', 10), ('KUBFU', 10))
        self.assertEqual(special, ['COSMOG', 'KUBFU'])

    def test_r5_exceptions_still_count_toward_the_line(self):
        after, mons = team('HIKER', 'Kanto', ('MACHOP', 20, 'ITEM_EVERSTONE'), ('MACHOP', 30))
        self.assertEqual(after, ['MACHOP', 'MACHAMP'])
        self.assertEqual(mons[0].exception, 'Everstone')

    def test_r2_regional_forms_follow_the_trainer_region(self):
        self.assertEqual(team('HIKER', 'Kanto', ('PIKACHU', 20))[0], ['RAICHU'])
        self.assertEqual(team('HIKER', 'Alola', ('PIKACHU', 20))[0], ['RAICHU_ALOLA'])
        self.assertEqual(team('HIKER', 'Hoenn', ('KOFFING', 20))[0], ['WEEZING'])
        self.assertEqual(team('HIKER', 'Sevii', ('KOFFING', 20))[0], ['WEEZING_GALAR'])
        self.assertEqual(team('HIKER', 'Johto', ('CYNDAQUIL', 20))[0], ['TYPHLOSION'])
        self.assertEqual(team('HIKER', 'Sinjoh', ('CYNDAQUIL', 20))[0], ['TYPHLOSION_HISUI'])

    def test_r2_region_only_further_steps_stop_outside_their_region(self):
        self.assertEqual(team('HIKER', 'Johto', ('URSARING', 40))[0], ['URSARING'])
        self.assertEqual(team('HIKER', 'Sinjoh', ('URSARING', 40))[0], ['URSALUNA'])
        self.assertEqual(team('HIKER', 'Johto', ('STANTLER', 30))[0], ['STANTLER'])
        self.assertEqual(team('HIKER', 'Sinjoh', ('STANTLER', 30))[0], ['WYRDEER'])
        self.assertEqual(team('HIKER', 'Kanto', ('SCYTHER', 30))[0], ['SCIZOR'])
        self.assertEqual(team('HIKER', 'Sinjoh', ('SCYTHER', 30))[0], ['KLEAVOR'])

    def test_r3_poliwag_line_follows_the_class(self):
        self.assertEqual(team('BLACK_BELT', 'Kanto', ('POLIWAG', 20))[0], ['POLIWRATH'])
        self.assertEqual(team('SWIMMER_M', 'Kanto', ('POLIWAG', 20))[0], ['POLITOED'])

    def test_r3_oddish_and_slowpoke_lines(self):
        self.assertEqual(team('AROMA_LADY', 'Hoenn', ('ODDISH', 20))[0], ['BELLOSSOM'])
        self.assertEqual(team('HIKER', 'Hoenn', ('ODDISH', 20))[0], ['VILEPLUME'])
        self.assertEqual(team('PSYCHIC_M', 'Johto', ('SLOWPOKE', 20))[0], ['SLOWKING'])
        self.assertEqual(team('HIKER', 'Johto', ('SLOWPOKE', 20))[0], ['SLOWBRO'])

    def test_r3_gender_dependent_branches_set_the_gender(self):
        after, mons = team('BLACK_BELT', 'Hoenn', ('RALTS', 20), gender='M')
        self.assertEqual((after, mons[0].new_gender), (['GALLADE'], 'M'))
        after, mons = team('PSYCHIC', 'Hoenn', ('RALTS', 20), gender='F')
        self.assertEqual((after, mons[0].new_gender), (['GARDEVOIR'], None))
        after, mons = team('SKIER', 'Johto', ('SNORUNT', 20), gender='F')
        self.assertEqual((after, mons[0].new_gender), (['FROSLASS'], 'F'))
        after, mons = team('HIKER', 'Johto', ('SNORUNT', 20), gender='M')
        self.assertEqual((after, mons[0].new_gender), (['GLALIE'], None))
        self.assertEqual(team('LASS', 'Hoenn', ('BURMY_PLANT', 20))[0], ['WORMADAM_PLANT'])
        self.assertEqual(team('HIKER', 'Hoenn', ('BURMY_PLANT', 20), gender='M')[0], ['MOTHIM_PLANT'])

    def test_r3_female_only_evolution_needs_a_compatible_gender(self):
        after, mons = team('HIKER', 'Hoenn', ('COMBEE', 20))
        self.assertEqual((after, mons[0].new_gender), (['VESPIQUEN'], 'F'))
        after, mons = team('HIKER', 'Hoenn', ('COMBEE', 20, None, 'M'))
        self.assertEqual((after, mons[0].new_gender), (['COMBEE'], None))

    def test_r3_eevee_follows_class_then_id(self):
        self.assertEqual(team('KINDLER', 'Johto', ('EEVEE', 20))[0], ['FLAREON'])
        self.assertEqual(team('SWIMMER_M', 'Johto', ('EEVEE', 20))[0], ['VAPOREON'])
        self.assertEqual(team('GUITARIST', 'Johto', ('EEVEE', 20))[0], ['JOLTEON'])
        self.assertEqual(team('PSYCHIC', 'Johto', ('EEVEE', 20))[0], ['ESPEON'])
        self.assertEqual(team('BIKER', 'Johto', ('EEVEE', 20))[0], ['UMBREON'])
        self.assertEqual(team('BUG_CATCHER', 'Johto', ('EEVEE', 20))[0], ['LEAFEON'])
        self.assertEqual(team('SKIER', 'Johto', ('EEVEE', 20))[0], ['GLACEON'])
        self.assertEqual(team('BEAUTY', 'Johto', ('EEVEE', 20))[0], ['SYLVEON'])
        fallback = [team('GENTLEMAN', 'Johto', ('EEVEE', 20), trainer_id=f'TRAINER_G{n}')[0][0] for n in range(40)]
        self.assertGreater(len(set(fallback)), 2)
        self.assertEqual(fallback, [team('GENTLEMAN', 'Johto', ('EEVEE', 20), trainer_id=f'TRAINER_G{n}')[0][0] for n in range(40)])

    def test_r3_random_branches_are_deterministic_by_id_and_nincada_skips_shedinja(self):
        for species, options in (('TYROGUE', {'HITMONCHAN', 'HITMONLEE', 'HITMONTOP'}),
                                 ('CLAMPERL', {'HUNTAIL', 'GOREBYSS'}),
                                 ('WURMPLE', {'BEAUTIFLY', 'DUSTOX'})):
            picks = {team('HIKER', 'Hoenn', (species, 20), ('GEODUDE', 20), trainer_id=f'TRAINER_X{n}')[0][0] for n in range(40)}
            self.assertEqual(picks, options)
            self.assertEqual(team('HIKER', 'Hoenn', (species, 20), ('GEODUDE', 20), trainer_id='TRAINER_X1')[0],
                             team('HIKER', 'Hoenn', (species, 20), ('GEODUDE', 20), trainer_id='TRAINER_X1')[0])
        self.assertEqual(team('HIKER', 'Hoenn', ('NINCADA', 20))[0], ['NINJASK'])

    def test_r6_illegal_authored_ability_is_dropped(self):
        _, mons = team('HIKER', 'Kanto', ('GEODUDE', 20))
        mons[0].ability = 'ABILITY_NONE_OF_THESE'
        self.assertTrue(fs.ability_drop(MODEL, mons[0]))
        mons[0].ability = 'ABILITY_STURDY'
        self.assertFalse(fs.ability_drop(MODEL, mons[0]))

    def test_party_header_parsing_and_editing(self):
        parsed = fs.parse_header('Nick (Luvdisc) (F) @ Big Root  ')
        self.assertEqual((parsed['nickname'], parsed['species'], parsed['gender'], parsed['item']), ('Nick', 'Luvdisc', 'F', 'Big Root'))
        parsed = fs.parse_header('Mr. Mime-Galar @ Eviolite')
        self.assertEqual(fs.species_constant(parsed['species']), 'SPECIES_MR_MIME_GALAR')
        self.assertEqual(fs.species_constant('Farfetch’d'), 'SPECIES_FARFETCHD')
        lines = ['=== TRAINER_X ===', 'Name: X', 'Gender: Male', '', 'Kirlia @ Sitrus Berry', 'Level: 20', 'Ability: Trace', '']
        block = fs.PartyBlock('TRAINER_X', lines)
        mon = fs.Mon(0, 'SPECIES_KIRLIA', 20, None, None, 'ABILITY_TRACE')
        mon.after, mon.new_gender, mon.drop_ability = 'SPECIES_GALLADE', 'M', True
        fs.apply_edits(lines, block, [mon], {})
        self.assertEqual(lines[4], 'Gallade (M) @ Sitrus Berry')
        self.assertIsNone(lines[6])


if __name__ == '__main__':
    unittest.main()
