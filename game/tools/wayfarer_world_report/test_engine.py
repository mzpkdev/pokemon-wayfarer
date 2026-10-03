"""Host tests of the engine seam (src/wayfarer_world.c): engine/engine_test.c,
compiled against engine/prelude.h instead of the engine headers. Needs the
Wayfarer build's generated tables, like the report."""

import os
import subprocess
import tempfile
import unittest
from pathlib import Path

import report

HERE = Path(__file__).resolve().parent
GAME = report.GAME


class EngineSeamTest(unittest.TestCase):
    def test_engine_seam(self):
        if not report.TABLES.exists():
            self.skipTest("needs the generated tables")
        cc = os.environ.get("HOSTCC", "cc")
        with tempfile.TemporaryDirectory() as tmp:
            binary = Path(tmp) / "engine_test"
            subprocess.run([cc, "-std=gnu11", "-O1", "-Wall", "-Werror", "-DPOKEMON_WAYFARER",
                            "-include", str(HERE / "engine/prelude.h"),
                            "-iquote", str(GAME / "include"), "-iquote", str(GAME / "src"),
                            "-o", str(binary), str(HERE / "engine/engine_test.c"),
                            str(GAME / "src/wayfarer_world.c"), str(GAME / "src/wayfarer_world_sim.c"),
                            str(GAME / "src/wayfarer_world_data.c")], check=True)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("all ok", result.stdout)


if __name__ == "__main__":
    unittest.main()
