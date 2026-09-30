import tempfile
import unittest
from pathlib import Path

from stamp import MARKER, TAIL, ZERO_ID, run


class StampTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.rom = Path(self.temp.name) / "test.gba"
        self.pristine = b"ROM-HEADER" + MARKER + ZERO_ID + TAIL + b"compiled-data"
        self.rom.write_bytes(self.pristine)

    def test_stamp_verify_and_repeat(self):
        result = run(self.rom, False)
        stamped = self.rom.read_bytes()
        self.assertNotEqual(stamped, self.pristine)
        self.assertEqual(run(self.rom, True), result)
        self.assertEqual(run(self.rom, False), result)
        self.assertEqual(self.rom.read_bytes(), stamped)

    def test_compiled_content_change_changes_id(self):
        first = run(self.rom, False)
        self.rom.write_bytes(self.pristine + b"changed")
        self.assertNotEqual(run(self.rom, False), first)

    def test_tamper_fails_verification_and_restamp(self):
        run(self.rom, False)
        tampered = bytearray(self.rom.read_bytes())
        tampered[-1] ^= 1
        self.rom.write_bytes(tampered)
        with self.assertRaisesRegex(ValueError, "does not match"):
            run(self.rom, True)
        with self.assertRaisesRegex(ValueError, "unexpected existing"):
            run(self.rom, False)

    def test_missing_duplicate_and_malformed_tags_fail(self):
        for rom in (b"no tag", self.pristine + self.pristine, MARKER + ZERO_ID + b"BAD!"):
            self.rom.write_bytes(rom)
            with self.assertRaises(ValueError):
                run(self.rom, False)


if __name__ == "__main__":
    unittest.main()
