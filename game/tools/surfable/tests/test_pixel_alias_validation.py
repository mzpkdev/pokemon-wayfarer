"""Regression cases for fail-closed source and linked-output validation."""
import importlib.util
from pathlib import Path
import shutil
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location('aliases', Path(__file__).with_name('test_surfable_pokemon_pic_aliases.py'))
aliases = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(aliases)


class AliasValidationTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.game = Path(self.directory.name)
        (self.game / aliases.DATA).mkdir(parents=True)
        (self.game / aliases.ASSETS).mkdir(parents=True)
        for name in ('surfable_pokemon_graphics.h', 'surfable_pokemon_pic_aliases.h', 'surfable_pokemon_pic_tables.h'):
            shutil.copyfile(aliases.GAME / aliases.DATA / name, self.game / aliases.DATA / name)
        for name, normal, shiny in aliases.manifest(self.game):
            size = 24576 if name in {'Lugia', 'Rayquaza', 'Arceus'} else 6144
            for stem in (normal, shiny):
                (self.game / aliases.ASSETS / (stem + '.4bpp')).write_bytes(bytes(size))

    def edit(self, name, transform):
        path = self.game / aliases.DATA / name
        path.write_text(transform(path.read_text()))

    def test_valid_inventory_excludes_commented_assets(self):
        normal, approved = aliases.validate(self.game)
        self.assertEqual((len(normal), len(approved)), (139, 61))
        self.assertNotIn('Rhyhorn', normal)

    def test_pixel_drift(self):
        path = self.game / aliases.ASSETS / '0007_squirtle_shiny.4bpp'
        path.write_bytes(b'\1' + bytes(6143))
        with self.assertRaisesRegex(ValueError, 'Squirtle: .*generated pixels diverge'):
            aliases.validate(self.game)

    def test_missing_shiny_output(self):
        (self.game / aliases.ASSETS / '0007_squirtle_shiny.4bpp').unlink()
        with self.assertRaisesRegex(ValueError, 'Squirtle: .*missing'):
            aliases.validate(self.game)

    def test_duplicate_manifest_row(self):
        self.edit('surfable_pokemon_pic_aliases.h', lambda text: text.replace('SURF_PIC_ALIAS(Pikachu, "0025_pikachu", "0025_pikachu_shiny")', 'SURF_PIC_ALIAS(Squirtle, "0007_squirtle", "0007_squirtle_shiny")'))
        with self.assertRaisesRegex(ValueError, 'duplicate names'):
            aliases.validate(self.game)

    def test_extra_shiny_incbin(self):
        self.edit('surfable_pokemon_graphics.h', lambda text: text + '\nconst u32 gSurfableShinyPokemonPic_Squirtle[] = INCBIN_U32("graphics/object_events/pics/pokemon/surfable/0007_squirtle_shiny.4bpp");\n')
        with self.assertRaisesRegex(ValueError, 'aliased INCBINs.*Squirtle'):
            aliases.validate(self.game)

    def test_unsized_alias(self):
        self.edit('surfable_pokemon_graphics.h', lambda text: text.replace('[ARRAY_COUNT(gSurfablePokemonPic_##Name)]', '[]'))
        with self.assertRaisesRegex(ValueError, 'strong, sized'):
            aliases.validate(self.game)

    def test_linked_symbols(self):
        normal, approved = aliases.validate(self.game)
        address = 0x8000000
        lines = []
        for name in normal:
            size = 24576 if name in {'Lugia', 'Rayquaza', 'Arceus'} else 6144
            lines.append(f'{address:08x} g {size:08x} gSurfablePokemonPic_{name}')
            if name not in approved:
                address += size
            lines.append(f'{address:08x} g {size:08x} gSurfableShinyPokemonPic_{name}')
            address += size
        path = self.game / 'release.sym'
        text = '\n'.join(lines)
        path.write_text(text)
        aliases.linked(path, normal, approved)
        path.write_text(text.replace(' g ', ' l '))
        aliases.linked(path, normal, approved)
        def relocate(symbol, target):
            original = next(line for line in lines if line.endswith(' ' + symbol))
            target_address = next(line.split()[0] for line in lines if line.endswith(' ' + target))
            return text.replace(original, target_address + original[8:])

        for broken, pattern in (
            (text.replace('08000000 g 00001800 gSurfableShinyPokemonPic_Squirtle', '08000004 g 00001800 gSurfableShinyPokemonPic_Squirtle'), 'Squirtle: alias address/size mismatch'),
            (relocate('gSurfableShinyPokemonPic_Wartortle', 'gSurfablePokemonPic_Wartortle'), 'Wartortle: non-approved pixel symbols share an address'),
            (relocate('gSurfablePokemonPic_Blastoise', 'gSurfablePokemonPic_Wartortle'), 'linked pixel ranges overlap'),
            (text.replace('gSurfableShinyPokemonPic_Squirtle', 'missing_Squirtle'), 'missing or extra linked symbols'),
            (text.replace('08000000 g 00001800', '08000000 w 00001800'), 'strong global or local symbol'),
            (text.replace('08000000 g 00001800 gSurfableShinyPokemonPic_Squirtle', '08000000 g 00000004 gSurfableShinyPokemonPic_Squirtle'), 'Squirtle: alias address/size mismatch'),
        ):
            with self.subTest(pattern=pattern):
                path.write_text(broken)
                with self.assertRaisesRegex(ValueError, pattern):
                    aliases.linked(path, normal, approved)


if __name__ == '__main__':
    unittest.main()
