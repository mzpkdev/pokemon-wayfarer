"""Run migrated C arithmetic against independent pre-migration expectations."""
import ctypes
from decimal import Decimal, ROUND_HALF_UP
from pathlib import Path
import subprocess
import tempfile
import unittest

from tools.gameplay_content import progression

ROOT = Path(__file__).resolve().parents[3]


class ProgressionRuntimeTests(unittest.TestCase):
    def test_runtime_consumers_exhaustively(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            for product, is_wayfarer in (('wayfarer', 1), ('hns', 0)):
                with self.subTest(product=product):
                    (path / 'global.h').write_text(f'''
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int bool32;
typedef signed char s8;
typedef signed int s32;
#define FALSE 0
#define TRUE 1
#define IS_WAYFARER {is_wayfarer}
#define NULL ((void*)0)
#define ARRAY_COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define TRAINER_SCALING_GYM_MEMBER 2
''')
                    (path / 'gameplay_progression.inc').write_text(progression.render_runtime())
                    source = (ROOT / 'src/trainer_party_scaling.c').read_text()
                    functions = []
                    for name in ('GetLeagueScalingBaseline', 'GetLeagueScalingLevel',
                                 'GetTrainerScalingLevel', 'GetGymLeaderScalingLevel'):
                        start = source.index('u8 ' + name + '(')
                        end = source.index('\n}', start) + 2
                        functions.append(source[start:end])
                    (path / 'consumers.c').write_text('#include "global.h"\n#include "gameplay_progression.h"\n' + '\n'.join(functions))
                    library = path / f'curves-{product}.so'
                    subprocess.run(['cc', '-shared', '-fPIC', '-I' + str(path), '-I' + str(ROOT / 'include'),
                                    str(ROOT / 'src/gameplay_progression.c'), str(path / 'consumers.c'),
                                    '-o', str(library)], check=True, capture_output=True)
                    runtime = ctypes.CDLL(str(library))
                    ratings = (0, 4, 8, 16, 30, 40, 55, 65, 80)
                    values = (15, 16, 18, 23, 30, 42, 60, 80, 100)
                    for rating in list(range(81)) + [81, 65535, 0xffffffff]:
                        clamped = min(rating, 80)
                        i = next(i for i in range(1, 9) if clamped <= ratings[i])
                        exact = Decimal(values[i-1]) + Decimal(clamped-ratings[i-1]) * Decimal(values[i]-values[i-1]) / Decimal(ratings[i]-ratings[i-1])
                        expected = int(exact.quantize(Decimal(1), rounding=ROUND_HALF_UP))
                        self.assertEqual(runtime.GetLeagueScalingBaseline(rating), expected)
                        for offset in range(-2, 1):
                            self.assertEqual(runtime.GetGymLeaderScalingLevel(rating, offset), max(1, min(100, expected+offset)))
                        for encounter in range(-4, 9):
                            for offset in range(-2, 1):
                                self.assertEqual(runtime.GetLeagueScalingLevel(rating, encounter, offset), max(1, min(100, expected+encounter+offset)))
                        for authored in range(1, 101):
                            adjustment = min(range(-1, 9), key=lambda a: abs(5*a-(authored-5)))
                            for policy in (0, 1, 2):
                                self.assertEqual(runtime.GetTrainerScalingLevel(rating, authored, policy), max(1, min(100, expected-8+adjustment+2*(policy == 2))))
                    output = ctypes.c_ubyte(37)
                    self.assertEqual(runtime.EvaluateGameplayCurve(65535, 40, ctypes.byref(output)), 0)
                    self.assertEqual(output.value, 37)
