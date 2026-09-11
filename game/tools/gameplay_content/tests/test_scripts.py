"""Production assembler selection and script provenance contracts."""
from pathlib import Path
import shutil
import tempfile
import unittest

from tools.gameplay_content.scripts import discover_battles, emitted_lines, load_script_blocks, parse_blocks

ROOT = Path(__file__).resolve().parents[3]


class ScriptSelectionTests(unittest.TestCase):
    def test_size_annotation_does_not_swallow_following_local_label(self):
        blocks = parse_blocks('''# 1 "data/maps/Test/scripts.inc"
Battle:
 trainerbattle_single TRAINER_TEST, Intro, Defeat
 end
.ifdef Battle ; .size Battle, . - Battle ; .endif ; Intro:
 .byte 1
''')
        self.assertEqual(set(blocks), {'Battle', 'Intro'})
        self.assertEqual(blocks['Battle'][0]['body'].splitlines()[-1].strip(), 'end')
        self.assertEqual(blocks['Battle'][0]['path'], 'data/maps/Test/scripts.inc')

    def test_listing_requires_an_address_and_bytes(self):
        self.assertEqual(emitted_lines('''  42 0000 5C010203 \t trainerbattle_single 7, Intro, Defeat
  42      04000000
  43                   trainerbattle_single 8, Intro, Defeat
  44 0009 02           end
'''), {42, 44})

    @unittest.skipUnless((ROOT / 'tools/preproc/preproc').is_file() and shutil.which('arm-none-eabi-as'),
                         'build the production preprocessor and ARM assembler first')
    def test_real_pipeline_selects_cpp_and_assembler_branches_without_previous_outputs(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'tools/preproc').mkdir(parents=True)
            (root / 'tools/preproc/preproc').symlink_to(ROOT / 'tools/preproc/preproc')
            (root / 'charmap.txt').symlink_to(ROOT / 'charmap.txt')
            (root / 'include/config').mkdir(parents=True)
            (root / 'include/config/selection.h').write_text('#define SELECT_CPP 1\n#define TRAINER_TEST 7\n')
            (root / 'data/maps/Test').mkdir(parents=True)
            (root / 'data/event_scripts.s').write_text('''#include "config/selection.h"
#include "gameplay_mart_bindings.inc"
.macro trainerbattle_single trainer, intro, defeat
 .byte 0x5c, 0x60, 0
 .2byte \\trainer
.endm
.text
.byte 0, 0, 0, 0, 0
.section .rodata
.if SELECT_AS
.include "data/maps/Test/custom.encounters"
.else
SharedLabel::
 trainerbattle_single 99, 0, 0
.endif
''')
            included = root / 'data/maps/Test/custom.encounters'
            included.write_text('''#if SELECT_CPP
SharedLabel::
 trainerbattle_single TRAINER_TEST, 0, 0
UnknownCaller::
 trainerbattle_single 8, 0, 0
#else
InactiveCpp::
 trainerbattle_single 9, 0, 0
#endif
''')
            # A disconnected source file is not part of the production include chain.
            (root / 'data/maps/Test/unused.inc').write_text('Unused::\n trainerbattle_single 10, 0, 0\n')
            selected = load_script_blocks(root, 'wayfarer', [], service_bindings='.set MART_UNUSED, 1\n',
                                          asflags='--defsym SELECT_AS=1')
            self.assertEqual(set(selected), {'SharedLabel', 'UnknownCaller'})
            self.assertEqual(len(selected['SharedLabel']), 1)
            self.assertEqual(selected['SharedLabel'][0]['compiledBattle'], {'mode': 6, 'trainer': 7})
            self.assertEqual(selected['SharedLabel'][0]['path'], 'data/maps/Test/custom.encounters')
            self.assertIn('TRAINER_TEST', selected['SharedLabel'][0]['body'])
            self.assertIn('trainerbattle_single 7', selected['SharedLabel'][0]['resolvedBody'])
            discovered = discover_battles(selected)
            self.assertEqual(len(discovered), 2)
            self.assertTrue(all(row['directCaller'] for row in discovered))
            self.assertTrue(all('outcomePolicy' not in row for row in discovered))
            self.assertIn('data/maps/Test/custom.encounters', selected.input_hashes)
            self.assertNotIn('data/maps/Test/unused.inc', selected.input_hashes)
            alternate = load_script_blocks(root, 'wayfarer', [], service_bindings='',
                                           asflags='--defsym SELECT_AS=0')
            self.assertEqual(set(alternate), {'SharedLabel'})
            self.assertEqual(discover_battles(alternate)[0]['resolvedArguments'], '99, 0, 0')
            # Even currently inactive included source belongs to the pipeline's
            # input closure; source changes must not reuse an immutable digest.
            included.write_text(included.read_text().replace('TRAINER_TEST,', '11,'))
            changed = load_script_blocks(root, 'wayfarer', [], service_bindings='',
                                         asflags='--defsym SELECT_AS=0')
            self.assertNotEqual(alternate.input_hashes, changed.input_hashes)


if __name__ == '__main__':
    unittest.main()
