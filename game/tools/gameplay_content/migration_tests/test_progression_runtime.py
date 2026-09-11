"""Temporary pre-migration arithmetic checks for adopted runtime consumers.

Remove this module with the migration target when v1 phase D is accepted and
equivalent domain coverage replaces these pre-migration checks.
"""
import ctypes
from decimal import Decimal, ROUND_HALF_UP
from pathlib import Path
import subprocess
import tempfile
import unittest

from tools.gameplay_content import progression

ROOT = Path(__file__).resolve().parents[3]
RATINGS = (0, 4, 8, 16, 30, 40, 55, 65, 80)
VALUES = (15, 16, 18, 23, 30, 42, 60, 80, 100)
MIGRATION_RATINGS = (0, 1, 4, 6, 8, 11, 16, 23, 30, 40, 47, 55, 60, 65, 73, 80, 81, 65535)


def expected_baseline(rating):
    clamped = min(rating, 80)
    index = next(index for index in range(1, len(RATINGS)) if clamped <= RATINGS[index])
    exact = (Decimal(VALUES[index - 1])
             + Decimal(clamped - RATINGS[index - 1])
             * Decimal(VALUES[index] - VALUES[index - 1])
             / Decimal(RATINGS[index] - RATINGS[index - 1]))
    return int(exact.quantize(Decimal(1), rounding=ROUND_HALF_UP))


class ProgressionRuntimeMigrationTests(unittest.TestCase):
    def test_runtime_consumers_match_pre_migration_cases(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            for product, is_wayfarer in (("wayfarer", 1), ("hns", 0)):
                with self.subTest(product=product):
                    (path / "global.h").write_text(f"""
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
""")
                    (path / "gameplay_progression.inc").write_text(progression.render_runtime())
                    source = (ROOT / "src/trainer_party_scaling.c").read_text()
                    functions = []
                    for name in ("GetLeagueScalingBaseline", "GetLeagueScalingLevel",
                                 "GetTrainerScalingLevel", "GetGymLeaderScalingLevel"):
                        start = source.index("u8 " + name + "(")
                        end = source.index("\n}", start) + 2
                        functions.append(source[start:end])
                    (path / "consumers.c").write_text(
                        '#include "global.h"\n#include "gameplay_progression.h"\n' + "\n".join(functions)
                    )
                    library = path / f"curves-{product}.so"
                    subprocess.run([
                        "cc", "-shared", "-fPIC", "-I" + str(path), "-I" + str(ROOT / "include"),
                        str(ROOT / "src/gameplay_progression.c"), str(path / "consumers.c"),
                        "-o", str(library),
                    ], check=True, capture_output=True)
                    runtime = ctypes.CDLL(str(library))
                    for rating in MIGRATION_RATINGS:
                        expected = expected_baseline(rating)
                        self.assertEqual(runtime.GetLeagueScalingBaseline(rating), expected)
                        for offset in (-2, 0):
                            self.assertEqual(
                                runtime.GetGymLeaderScalingLevel(rating, offset),
                                max(1, min(100, expected + offset)),
                            )
                        for encounter in (-4, 0, 8):
                            for offset in (-2, 0):
                                self.assertEqual(
                                    runtime.GetLeagueScalingLevel(rating, encounter, offset),
                                    max(1, min(100, expected + encounter + offset)),
                                )
                        for authored in (1, 5, 10, 42, 100):
                            adjustment = min(range(-1, 9), key=lambda value: abs(5 * value - (authored - 5)))
                            for policy in (0, 2):
                                self.assertEqual(
                                    runtime.GetTrainerScalingLevel(rating, authored, policy),
                                    max(1, min(100, expected - 8 + adjustment + 2 * (policy == 2))),
                                )
                    output = ctypes.c_ubyte(37)
                    self.assertEqual(runtime.EvaluateGameplayCurve(65535, 40, ctypes.byref(output)), 0)
                    self.assertEqual(output.value, 37)


if __name__ == "__main__":
    unittest.main()
