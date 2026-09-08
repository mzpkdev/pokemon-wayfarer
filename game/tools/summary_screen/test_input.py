"""Exercise the production input handler with host-side UI stubs."""
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]


class SummaryInputTests(unittest.TestCase):
    def test_skills_shortcuts_and_l_equals_a(self):
        source = (ROOT / 'src/pokemon_summary_screen.c').read_text()
        start = source.index('static void Task_HandleInput(u8 taskId)\n{')
        end = source.index('\nstatic void ShowMonSkillsInfo(', start)
        handler = source[start:end]
        harness = r'''
#include <assert.h>
typedef unsigned char u8;
enum { FALSE, TRUE };
enum { DPAD_UP=1, DPAD_DOWN=2, DPAD_LEFT=4, DPAD_RIGHT=8,
       A_BUTTON=16, B_BUTTON=32, START_BUTTON=64, R_BUTTON=128, L_BUTTON=256 };
enum { PSS_PAGE_INFO, PSS_PAGE_SKILLS, PSS_PAGE_BATTLE_MOVES, PSS_PAGE_CONTEST_MOVES };
enum { SUMMARY_SKILLS_MODE_STATS, SUMMARY_SKILLS_MODE_IVS, SUMMARY_SKILLS_MODE_EVS };
enum { OPTIONS_BUTTON_MODE_L_EQUALS_A=1, SE_SELECT=1 };
static unsigned keys;
#define JOY_NEW(button) (keys & (button))
static struct { int active; } gPaletteFade;
static struct { int currPageIndex, skillsPageMode; } screen, *sMonSummaryScreen=&screen;
static struct { int optionsButtonMode; } save, *gSaveBlock2Ptr=&save;
static int selection, skill, closeCount, navigation;
static int MenuHelpers_ShouldWaitForLinkRecv(void) { return FALSE; }
static void ChangeSummaryPokemon(u8 taskId, int direction) { navigation=direction; }
static void ChangePage(u8 taskId, int direction) { navigation=direction; }
static void PlaySE(int sound) {}
static void SwitchToMoveSelection(u8 taskId) { selection++; }
static void StopPokemonAnimations(void) {}
static void BeginCloseSummaryScreen(u8 taskId) { closeCount++; }
static void ShowMonSkillsInfo(u8 taskId, int mode) { skill=mode; }
'''
        cases = r'''
static void press(unsigned physicalKeys, int page, int buttonMode)
{
    screen.currPageIndex=page;
    screen.skillsPageMode=-1;
    save.optionsButtonMode=buttonMode;
    keys=physicalKeys;
    // main.c retains the physical L bit when it also maps L to A.
    if (buttonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A && (keys & L_BUTTON))
        keys |= A_BUTTON;
    selection=closeCount=navigation=0;
    skill=-1;
    Task_HandleInput(0);
}
int main(void)
{
    for (int mode=0; mode<=OPTIONS_BUTTON_MODE_L_EQUALS_A; mode++) {
        press(L_BUTTON, PSS_PAGE_SKILLS, mode);
        assert(skill == SUMMARY_SKILLS_MODE_EVS);
        assert(screen.skillsPageMode == SUMMARY_SKILLS_MODE_EVS && selection == 0);
        press(R_BUTTON, PSS_PAGE_SKILLS, mode);
        assert(skill == SUMMARY_SKILLS_MODE_IVS);
        press(START_BUTTON, PSS_PAGE_SKILLS, mode);
        assert(skill == SUMMARY_SKILLS_MODE_STATS);
        press(A_BUTTON, PSS_PAGE_SKILLS, mode);
        assert(skill == -1 && selection == 0);
        press(A_BUTTON, PSS_PAGE_BATTLE_MOVES, mode);
        assert(selection == 1);
        press(L_BUTTON, PSS_PAGE_BATTLE_MOVES, mode);
        assert(selection == (mode == OPTIONS_BUTTON_MODE_L_EQUALS_A));
        press(L_BUTTON, PSS_PAGE_CONTEST_MOVES, mode);
        assert(selection == (mode == OPTIONS_BUTTON_MODE_L_EQUALS_A));
        press(B_BUTTON, PSS_PAGE_SKILLS, mode);
        assert(closeCount == 1 && skill == -1);
        press(DPAD_RIGHT, PSS_PAGE_SKILLS, mode);
        assert(navigation == 1);
    }
    gPaletteFade.active=TRUE;
    press(L_BUTTON, PSS_PAGE_SKILLS, OPTIONS_BUTTON_MODE_L_EQUALS_A);
    assert(skill == -1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory(prefix='summary-input-') as directory:
            path = Path(directory) / 'input.c'
            binary = Path(directory) / 'input'
            path.write_text(harness + handler + cases)
            compiled = subprocess.run(
                ['cc', '-std=c99', '-Wall', '-Werror', str(path), '-o', str(binary)],
                capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == '__main__':
    unittest.main()
