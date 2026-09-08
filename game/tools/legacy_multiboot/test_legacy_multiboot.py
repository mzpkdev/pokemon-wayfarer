"""Host checks of production capability guards and menu/save decision code."""
from itertools import product
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
CAPABILITIES = (
    "ENABLE_COLOSSEUM_MULTIBOOT",
    "ENABLE_BERRY_GLITCH_FIX_MULTIBOOT",
    "ENABLE_EREADER_TRANSFER",
)
PAYLOADS = (
    ("pokemon_colosseum", "PokemonColosseum", "mb_colosseum.gba", 0x28000),
    ("berry_glitch_fix", "BerryGlitchFix", "mb_berry_fix.gba", 0x3BF4),
    ("ereader", "EReader", "mb_ereader.gba", 0x30E0),
)


def preprocess(source, **defines):
    # Keep production conditionals, but avoid pulling GBA headers into host tests.
    source = re.sub(r'^\s*#include[^\n]*', '', source, flags=re.M)
    result = subprocess.run(
        ['cpp', '-P', '-x', 'c', '-DTRUE=1', '-DFALSE=0',
         *[f'-D{k}={v}' for k, v in defines.items()], '-'],
        input=source, capture_output=True, text=True, check=True)
    return result.stdout


def block(source, marker):
    start = source.index(marker)
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


class LegacyMultibootTests(unittest.TestCase):
    def run_c(self, source):
        with tempfile.TemporaryDirectory(prefix='legacy-multiboot-') as directory:
            path = Path(directory) / 'test.c'
            binary = Path(directory) / 'test'
            path.write_text(source)
            result = subprocess.run(['cc', '-std=c99', '-Wall', '-Werror',
                                     str(path), '-o', str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)

    def configuration(self, build, **values):
        # The prefix includes product selection, config.mk and actual validation.
        # Stopping before build rules makes this safe during sequential ROM builds.
        prefix = (ROOT / 'Makefile').read_text().split('# Default make rule')[0]
        makefile = prefix + '\n.PHONY: probe\nprobe:\n\t@echo $(foreach capability,$(LEGACY_MULTIBOOT_CAPABILITIES),$($(capability)))\n'
        environment = os.environ.copy()
        for variable in ('MAKEFLAGS', 'MFLAGS', 'MAKELEVEL', 'MAKEOVERRIDES'):
            environment.pop(variable, None)
        return subprocess.run(['make', '--no-print-directory', '-s', '-f', '-',
                               'probe', f'BUILD={build}',
                               *[f'{k}={v}' for k, v in values.items()]],
                              cwd=ROOT, env=environment, input=makefile,
                              capture_output=True, text=True)

    def test_product_defaults_and_independent_configuration_matrix(self):
        for build in ('emerald', 'firered', 'leafgreen', 'hns', 'wayfarer'):
            with self.subTest(build=build):
                result = self.configuration(build)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout.split(), ['0' if build == 'wayfarer' else '1'] * 3)
            for values in product((0, 1), repeat=3):
                with self.subTest(build=build, values=values):
                    result = self.configuration(build, **dict(zip(CAPABILITIES, values)))
                    if build == 'wayfarer' and any(values):
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn('Wayfarer requires', result.stderr)
                    else:
                        self.assertEqual(result.returncode, 0, result.stderr)
                        self.assertEqual(result.stdout.split(), list(map(str, values)))

    def test_invalid_capability_values_fail_clearly(self):
        for capability, value in product(CAPABILITIES, ('', '2', '-1', 'true', '0 1')):
            with self.subTest(capability=capability, value=value):
                result = self.configuration('emerald', **{capability: value})
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(f'{capability} must be 0 or 1', result.stderr)

    def test_payload_definitions_follow_each_capability(self):
        for values in product((0, 1), repeat=3):
            defines = dict(zip(CAPABILITIES, values))
            for enabled, (filename, symbol, binary, size) in zip(values, PAYLOADS):
                with self.subTest(values=values, payload=filename):
                    output = preprocess((ROOT / f'data/multiboot_{filename}.s').read_text(), **defines)
                    for suffix in ('Start', 'End'):
                        self.assertEqual(f'gMultiBootProgram_{symbol}_{suffix}' in output, bool(enabled))
                    self.assertEqual('.incbin' in output, bool(enabled))
                    self.assertEqual((ROOT / 'data' / binary).stat().st_size, size)

    def test_berry_route_absent_but_other_chords_remain(self):
        for filename in ('title_screen.c', 'title_screen_frlg.c'):
            source = (ROOT / 'src' / filename).read_text()
            disabled = preprocess(source, ENABLE_BERRY_GLITCH_FIX_MULTIBOOT=0, IS_FRLG=int('frlg' in filename))
            enabled = preprocess(source, ENABLE_BERRY_GLITCH_FIX_MULTIBOOT=1, IS_FRLG=int('frlg' in filename))
            self.assertNotIn('CB2_InitBerryFixProgram', disabled)
            self.assertNotIn('TransitionToBerryFix', disabled)
            self.assertNotIn('GoToBerryFixScreen', disabled)
            self.assertIn('CB2_InitBerryFixProgram', enabled)
            self.assertIn('ResetRtc', disabled)
            self.assertIn('ClearSave', disabled)

    def test_visiting_trainer_validation_empty_invalid_valid_and_removed_field(self):
        source = (ROOT / 'src/battle_special.c').read_text()
        function = block(source, 'void ValidateEReaderTrainer(void)\n')
        for freed in (0, 1):
            production = preprocess(function, FREE_BATTLE_TOWER_E_READER=freed)
            self.run_c('''
#include <assert.h>
#include <stdint.h>
#include <string.h>
typedef uint32_t u32;
struct BattleTowerEReaderTrainer { u32 data[46]; u32 checksum; };
static unsigned gSpecialVar_Result;
''' + ('' if freed else '''
static struct { struct { struct BattleTowerEReaderTrainer ereaderTrainer; } frontier; } save, *gSaveBlock2Ptr=&save;
static void ClearEReaderTrainer(struct BattleTowerEReaderTrainer *trainer) { memset(trainer, 0, sizeof(*trainer)); }
''') + production + '''
int main(void) {
    ValidateEReaderTrainer();
    assert(gSpecialVar_Result == 1);
''' + ('' if freed else '''
    save.frontier.ereaderTrainer.data[0] = 42;
    save.frontier.ereaderTrainer.checksum = 41;
    ValidateEReaderTrainer();
    assert(gSpecialVar_Result == 1);
    for (unsigned i=0; i<46; i++) assert(save.frontier.ereaderTrainer.data[i] == 0);
    assert(save.frontier.ereaderTrainer.checksum == 0);
    save.frontier.ereaderTrainer.data[0] = 42;
    save.frontier.ereaderTrainer.checksum = 42;
    ValidateEReaderTrainer();
    assert(gSpecialVar_Result == 0);
    assert(save.frontier.ereaderTrainer.data[0] == 42);
''') + 'return 0; }')

    def test_trainer_hill_ignores_special_sector_and_transfer_code(self):
        source = (ROOT / 'src/trainer_hill.c').read_text()
        disabled = preprocess(source, ENABLE_EREADER_TRANSFER=0, FREE_TRAINER_HILL=0)
        self.assertNotIn('ReadTrainerHillAndValidate', disabled)
        self.assertNotIn('sEReader_Pal', disabled)
        self.assertIn('sChallengeData', disabled)
        self.assertIn('sFloorData', disabled)
        helpers = preprocess((ROOT / 'src/ereader_helpers.c').read_text(), ENABLE_EREADER_TRANSFER=0)
        self.assertEqual(helpers.strip(), '')
        functions = block(source, 'static void GetInEReaderMode(void)\n')
        functions += block(source, 'bool32 OnTrainerHillEReaderChallengeFloor(void)\n')
        self.run_c('''
#include <assert.h>
typedef int bool32;
enum { FALSE, TRUE, TRAINER_HILL_ENTRANCE=1 };
static int gSpecialVar_Result, active, map;
static void SetUpDataStruct(void) {}
static void FreeDataStruct(void) {}
static int InTrainerHillChallenge(void) { return active; }
static int GetCurrentTrainerHillMapId(void) { return map; }
''' + functions + '''
int main(void) {
    for (active=0; active<=1; active++)
    for (map=0; map<6; map++) {
        gSpecialVar_Result=TRUE;
        assert(OnTrainerHillEReaderChallengeFloor() == FALSE);
        GetInEReaderMode();
        assert(gSpecialVar_Result == FALSE);
    }
    return 0;
}
''')

    def test_main_menu_actions_with_adapter_present_absent_and_changed(self):
        source = (ROOT / 'src/main_menu.c').read_text()
        # Execute the production nested switch that computes the selected action.
        start = source.index('wirelessAdapterConnected = IsWirelessAdapterConnected();')
        switch = block(source[start:], 'switch (gTasks[taskId].tMenuType)')
        disabled = preprocess(source, ENABLE_EREADER_TRANSFER=0)
        self.assertNotIn('ACTION_EREADER', disabled)
        self.assertNotIn('CB2_InitEReader', disabled)
        self.assertNotIn('e-READER', disabled)
        item_count = re.search(r'tItemCount = [^;]+;', source).group()
        enums = ''.join(re.findall(r'enum\s*\{[^}]*\};', source[source.index('enum\n{\n    HAS_NO_SAVED_GAME'):source.index('#define MAIN_MENU_BORDER_TILE')]))
        enums = preprocess(enums, ENABLE_EREADER_TRANSFER=0)
        production = preprocess(switch, ENABLE_EREADER_TRANSFER=0)
        self.run_c('''
#include <assert.h>
''' + enums + '''
static int item_count(int tMenuType) {
    int tItemCount;
''' + item_count + '''
    return tItemCount;
}
static struct { int tMenuType, tCurrItem, tWirelessAdapterConnected; } gTasks[1];
static int select_action(int menu, int item, int cached, int wirelessAdapterConnected) {
    int taskId=0, action=-1;
    gTasks[0].tMenuType=menu;
    gTasks[0].tCurrItem=item;
    gTasks[0].tWirelessAdapterConnected=cached;
''' + production + '''
    return action;
}
int main(void) {
    for (int cached=0; cached<=1; cached++)
    for (int wireless=0; wireless<=1; wireless++)
    for (int menu=HAS_NO_SAVED_GAME; menu<=HAS_MYSTERY_EVENTS; menu++) {
        int count=item_count(menu);
        assert(count == menu+2);
        for (int item=0; item<count; item++) {
            int action=select_action(menu,item,cached,wireless);
            assert(action != -1);
            if (item == count-1) assert(action == ACTION_OPTION);
            if (item == 0) assert(action == (menu == HAS_NO_SAVED_GAME ? ACTION_NEW_GAME : ACTION_CONTINUE));
            if (menu >= HAS_MYSTERY_GIFT && item == 2) {
                assert(action == (wireless ? ACTION_MYSTERY_GIFT : ACTION_INVALID));
                if (!wireless) assert(gTasks[0].tMenuType == HAS_NO_SAVED_GAME);
            }
        }
    }
    return 0;
}
''')


if __name__ == '__main__':
    unittest.main()
