"""Host tests of the notable-ambience beat selection (src/wayfarer_ambience.c):
ambience/ambience_test.c, compiled against ambience/prelude.h instead of
global.h and linked with the real generated tables. Needs the Wayfarer
build's generated tables, like the report."""

import os
import re
import subprocess
import tempfile
import unittest
from pathlib import Path

import report

HERE = Path(__file__).resolve().parent
GAME = report.GAME
SOURCE = GAME / "src/wayfarer_ambience.c"


def beat_defines():
    """#define BEAT_<ID> <index> from the pool's [index] = { // id comments."""
    text = report.TABLES.read_text()
    start = text.index("gWayfarerAmbienceBeats[] = {")
    end = text.index("\n};", start)
    rows = re.findall(r"^\s*\[(\d+)\] = \{ // (\w+)$", text[start:end], re.M)
    return "".join(f"#define BEAT_{name.upper()} {index}\n" for index, name in rows)


class AmbienceSelectTest(unittest.TestCase):
    def test_selection(self):
        if not report.TABLES.exists():
            self.skipTest("needs the generated tables")
        cc = os.environ.get("HOSTCC", "cc")
        with tempfile.TemporaryDirectory() as tmp:
            (Path(tmp) / "ambience_beats.h").write_text(beat_defines())
            binary = Path(tmp) / "ambience_test"
            # No stub defines Random: a module that used it would fail to link.
            subprocess.run([cc, "-std=gnu11", "-O1", "-Wall", "-Werror", "-DPOKEMON_WAYFARER",
                            "-include", str(HERE / "ambience/prelude.h"),
                            "-iquote", str(GAME / "include"), "-iquote", str(GAME / "src"), "-iquote", tmp,
                            "-o", str(binary), str(HERE / "ambience/ambience_test.c"),
                            str(SOURCE), str(GAME / "src/wayfarer_world_data.c")], check=True)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("all ok", result.stdout)

    def test_never_reads_the_rng(self):
        text = SOURCE.read_text()
        for name in ("Random", "gRngValue", "rand("):
            self.assertNotIn(name, text)


if __name__ == "__main__":
    unittest.main()
