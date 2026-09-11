"""Focused contracts for the bounded encounter post-link verifier."""
from pathlib import Path
import struct
import tempfile
import unittest

from tools.gameplay_content.common import ContentError
from tools.gameplay_content.postlink import verify_encounter_callers, verify_report


ADDRESS = 0x08000100
LABEL = "EventScript_TestTrainer"
TRAINER = 0x1234
MODE = 2


def write_elf(path, command=None, symbol_address=ADDRESS, mapping_duplicates=False):
    command = bytearray(command or bytes((0x5C, MODE << 4, 0, TRAINER & 0xFF, TRAINER >> 8)))
    section_names = b"\0.shstrtab\0.strtab\0.symtab\0.rodata\0"
    symbols = b"\0" + LABEL.encode() + b"\0"
    mapping_name = len(symbols)
    if mapping_duplicates:
        symbols += b"$t\0"
    section_offsets = {"names": 52, "strings": 96, "symbols": 128, "data": 160}
    symbol_size = 64 if mapping_duplicates else 32
    data_offset = section_offsets["symbols"] + symbol_size
    section_table_offset = data_offset + len(command)
    blob = bytearray(section_table_offset + 5 * 40)
    blob[:16] = b"\x7fELF" + bytes((1, 1, 1)) + b"\0" * 9
    struct.pack_into("<HHIIIIIHHHHHH", blob, 16, 2, 40, 1, 0, 0, section_table_offset,
                     0, 52, 0, 0, 40, 5, 1)
    blob[section_offsets["names"]:section_offsets["names"] + len(section_names)] = section_names
    blob[section_offsets["strings"]:section_offsets["strings"] + len(symbols)] = symbols
    struct.pack_into("<IIIBBH", blob, section_offsets["symbols"] + 16,
                     1, symbol_address, 0, 0, 0, 4)
    if mapping_duplicates:
        struct.pack_into("<IIIBBH", blob, section_offsets["symbols"] + 32,
                         mapping_name, ADDRESS, 0, 0, 0, 4)
        struct.pack_into("<IIIBBH", blob, section_offsets["symbols"] + 48,
                         mapping_name, ADDRESS + 1, 0, 0, 0, 4)
    blob[data_offset:data_offset + len(command)] = command
    def section(index, name, type_, address, offset, size, link=0, entry_size=0):
        struct.pack_into("<IIIIIIIIII", blob, section_table_offset + index * 40,
                         name, type_, 0, address, offset, size, link, 0, 1, entry_size)
    section(1, 1, 3, 0, section_offsets["names"], len(section_names))
    section(2, 11, 3, 0, section_offsets["strings"], len(symbols))
    section(3, 19, 2, 0, section_offsets["symbols"], symbol_size, 2, 16)
    section(4, 27, 1, ADDRESS, data_offset, len(command))
    Path(path).write_bytes(blob)


def row(**caller_changes):
    caller = {"label": LABEL, "trainer": TRAINER, "mode": MODE, "argumentOffset": 1}
    caller.update(caller_changes)
    return {"id": "test-trainer", "caller": caller}


class PostLinkTests(unittest.TestCase):
    def test_verifies_opcode_mode_trainer_and_registry_argument_offset(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "test.elf"
            write_elf(elf)
            verified = verify_encounter_callers(elf, [row(mode="continue")])
        self.assertEqual(verified, [{"id": "test-trainer", "label": LABEL, "address": ADDRESS,
                                     "argumentAddress": ADDRESS + 1, "mode": MODE, "trainer": TRAINER}])

    def test_empty_standalone_scope_does_not_require_an_elf(self):
        self.assertEqual(verify_report("missing.elf", {"encounters": {"encounters": []}}), [])

    def test_rejects_missing_encounter_projection(self):
        with self.assertRaisesRegex(ContentError, "encounters projection is required"):
            verify_report("missing.elf", {})

    def test_rejects_domain_override_that_omits_encounters(self):
        with self.assertRaisesRegex(ContentError, "encounters domain is required"):
            verify_report("missing.elf", {"domains": ["inventory"], "encounters": {"records": []}})

    def test_rejects_malformed_encounter_records(self):
        with self.assertRaisesRegex(ContentError, "encounters list"):
            verify_report("missing.elf", {"encounters": {"encounters": {}}})

    def test_rejects_noncanonical_or_ambiguous_projection_records(self):
        with self.assertRaisesRegex(ContentError, "must not contain records"):
            verify_report("missing.elf", {"encounters": {"records": []}})
        with self.assertRaisesRegex(ContentError, "must not contain records"):
            verify_report("missing.elf", {"encounters": {"records": [], "encounters": [row()]}})

    def test_rejects_symbol_shifted_to_argument_block(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "test.elf"
            write_elf(elf, symbol_address=ADDRESS + 1)
            with self.assertRaisesRegex(ContentError, "points at trainerbattle arguments"):
                verify_encounter_callers(elf, [row()])

    def test_ignores_unrelated_duplicate_arm_mapping_symbols(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "test.elf"
            write_elf(elf, mapping_duplicates=True)
            self.assertEqual(verify_encounter_callers(elf, [row()])[0]["trainer"], TRAINER)

    def test_rejects_wrong_opcode(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "test.elf"
            write_elf(elf, command=bytes((0x00, MODE << 4, 0, TRAINER & 0xFF, TRAINER >> 8)))
            with self.assertRaisesRegex(ContentError, "expected trainerbattle opcode"):
                verify_encounter_callers(elf, [row()])

    def test_rejects_wrong_registry_argument_offset(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "test.elf"
            write_elf(elf)
            with self.assertRaisesRegex(ContentError, "argument offset must be 1"):
                verify_encounter_callers(elf, [row(argumentOffset=2)])

    def test_rejects_missing_registry_argument_offset(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "test.elf"
            write_elf(elf)
            caller = row()["caller"]
            del caller["argumentOffset"]
            with self.assertRaisesRegex(ContentError, "caller.argumentOffset is required"):
                verify_encounter_callers(elf, [{"id": "test-trainer", "caller": caller}])

    def test_rejects_wrong_mode(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "test.elf"
            write_elf(elf)
            with self.assertRaisesRegex(ContentError, "expected mode"):
                verify_encounter_callers(elf, [row(mode=4)])

    def test_rejects_wrong_trainer(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "test.elf"
            write_elf(elf)
            with self.assertRaisesRegex(ContentError, "expected trainer"):
                verify_encounter_callers(elf, [row(trainer=0x9999)])


if __name__ == "__main__":
    unittest.main()
