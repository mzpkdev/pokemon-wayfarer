"""Verify compiled encounter callers against the linked trainerbattle commands.

The event assembler emits a trainerbattle command as one opcode byte followed by
its packed ``TrainerBattleParameter``.  The runtime registries retain a pointer
to that argument block, so a declaration label must name the opcode and its
registered argument offset is always one.  This module deliberately reads only
the ELF section and symbol tables needed to prove that small contract.
"""
import argparse
from pathlib import Path
import struct

from .common import ContentError, load_json


ELF_MAGIC = b"\x7fELF"
ELFCLASS32 = 1
ELFDATA2LSB = 1
SHT_SYMTAB = 2
SHN_UNDEF = 0
SHN_ABS = 0xFFF1

TRAINERBATTLE_OPCODE = 0x5C
TRAINERBATTLE_ARGUMENT_OFFSET = 1
TRAINERBATTLE_FLAGS_OFFSET = 1
TRAINERBATTLE_TRAINER_OFFSET = 3

TRAINERBATTLE_MODES = {
    "single": 0,
    "continue_no_music": 1,
    "continue": 2,
    "single_no_intro": 3,
    "double": 4,
    "rematch": 5,
    "continue_double": 6,
    "rematch_double": 7,
    "continue_double_no_music": 8,
    "two_trainers_no_intro": 13,
    "early_rival": 14,
}


def _error(path, key, reason):
    raise ContentError("POST_LINK", path, key, reason)


class Elf32:
    """Minimal, bounded reader for the linked ARM ELF's symbols and sections."""

    def __init__(self, path):
        self.path = Path(path)
        try:
            self.data = self.path.read_bytes()
        except OSError as exc:
            _error(self.path, "", str(exc))
        if len(self.data) < 52 or self.data[:4] != ELF_MAGIC:
            _error(self.path, "", "expected ELF32 file")
        if self.data[4] != ELFCLASS32 or self.data[5] != ELFDATA2LSB:
            _error(self.path, "", "expected little-endian ELF32 file")
        self._parse_sections()
        self.symbols = self._parse_symbols()

    def _unpack_from(self, format_, offset, key):
        size = struct.calcsize(format_)
        if offset < 0 or offset + size > len(self.data):
            _error(self.path, key, "truncated ELF")
        return struct.unpack_from(format_, self.data, offset)

    def _slice(self, offset, size, key):
        if offset < 0 or size < 0 or offset + size > len(self.data):
            _error(self.path, key, "ELF range is outside the file")
        return self.data[offset:offset + size]

    def _parse_sections(self):
        (_, _, _, _, _, shoff, _, _, _, _, shentsize, shnum, shstrndx) = self._unpack_from(
            "<HHIIIIIHHHHHH", 16, "header")
        if not shoff or not shnum or shentsize < 40 or shstrndx >= shnum:
            _error(self.path, "header", "missing valid section table")
        self._slice(shoff, shentsize * shnum, "section table")
        self.sections = []
        for index in range(shnum):
            values = self._unpack_from("<IIIIIIIIII", shoff + index * shentsize, f"section {index}")
            self.sections.append({
                "name": values[0], "type": values[1], "address": values[3],
                "offset": values[4], "size": values[5], "link": values[6],
                "entry_size": values[9],
            })
        names = self.sections[shstrndx]
        self._section_names = self._slice(names["offset"], names["size"], "section names")

    @staticmethod
    def _string(table, offset, key):
        if offset >= len(table):
            raise ValueError(f"{key}: string outside string table")
        end = table.find(b"\0", offset)
        if end == -1:
            raise ValueError(f"{key}: unterminated string")
        return table[offset:end].decode("ascii")

    def _parse_symbols(self):
        symbols = {}
        for index, section in enumerate(self.sections):
            if section["type"] != SHT_SYMTAB:
                continue
            if section["entry_size"] < 16 or section["size"] % section["entry_size"]:
                _error(self.path, f"section {index}", "invalid symbol table")
            if section["link"] >= len(self.sections):
                _error(self.path, f"section {index}", "symbol string table is missing")
            strings_section = self.sections[section["link"]]
            strings = self._slice(strings_section["offset"], strings_section["size"], "symbol strings")
            self._slice(section["offset"], section["size"], f"symbol table {index}")
            for entry in range(section["size"] // section["entry_size"]):
                name_offset, value, _, _, _, section_index = self._unpack_from(
                    "<IIIBBH", section["offset"] + entry * section["entry_size"], f"symbol {entry}")
                if not name_offset:
                    continue
                try:
                    name = self._string(strings, name_offset, f"symbol {entry}")
                except (UnicodeDecodeError, ValueError) as exc:
                    _error(self.path, f"symbol {entry}", str(exc))
                if section_index in (SHN_UNDEF, SHN_ABS) or section_index >= len(self.sections):
                    continue
                resolved = (value, section_index)
                symbols.setdefault(name, set()).add(resolved)
        if not symbols:
            _error(self.path, "", "no ELF symbol table")
        return symbols

    def byte_at(self, address, key):
        for section in self.sections:
            start = section["address"]
            if start <= address < start + section["size"]:
                return self._slice(section["offset"] + address - start, 1, key)[0]
        _error(self.path, key, f"address 0x{address:08X} is outside a section")

    def has_byte_at(self, address):
        return any(section["address"] <= address < section["address"] + section["size"]
                   for section in self.sections)

    def u16_at(self, address, key):
        for section in self.sections:
            start = section["address"]
            if start <= address and address + 2 <= start + section["size"]:
                return self._unpack_from("<H", section["offset"] + address - start, key)[0]
        _error(self.path, key, f"address 0x{address:08X} is outside a section")

    def symbol_address(self, name):
        try:
            candidates = self.symbols[name]
        except KeyError:
            _error(self.path, name, "symbol is absent")
        if len(candidates) != 1:
            _error(self.path, name, "ambiguous symbol")
        return next(iter(candidates))[0]


def _required_caller(row, index):
    if not isinstance(row, dict):
        _error("<encounters>", str(index), "record must be an object")
    caller = row.get("caller")
    if not isinstance(caller, dict):
        _error("<encounters>", row.get("id", str(index)), "caller must be an object")
    label = caller.get("label")
    trainer = caller.get("trainer")
    mode = caller.get("mode")
    identifier = row.get("id", label)
    if not isinstance(label, str) or not label:
        _error("<encounters>", str(identifier), "caller.label must be a nonempty string")
    if isinstance(trainer, bool) or not isinstance(trainer, int) or not 0 <= trainer <= 0xFFFF:
        _error("<encounters>", str(identifier), "caller.trainer must be a u16")
    if "argumentOffset" not in caller:
        _error("<encounters>", str(identifier), "caller.argumentOffset is required")
    offset = caller["argumentOffset"]
    if isinstance(offset, bool) or not isinstance(offset, int):
        _error("<encounters>", str(identifier), "caller.argumentOffset must be an integer")
    return identifier, label, trainer, _mode_value(mode, identifier), offset


def _mode_value(mode, identifier):
    if isinstance(mode, bool):
        _error("<encounters>", str(identifier), "caller.mode must be a trainer battle mode")
    if isinstance(mode, int):
        if 0 <= mode <= 15:
            return mode
    elif isinstance(mode, str):
        normalized = mode.lower().removeprefix("trainer_battle_")
        if normalized in TRAINERBATTLE_MODES:
            return TRAINERBATTLE_MODES[normalized]
    _error("<encounters>", str(identifier), "caller.mode must be a supported trainer battle mode")


def verify_encounter_callers(elf_path, declarations):
    """Verify compiled encounter declaration rows against one linked ELF.

    An empty selected scope is valid and intentionally does not open ``elf_path``.
    This lets standalone products retain an empty Phase-D outcome scope.
    """
    if not isinstance(declarations, list):
        _error("<encounters>", "", "encounters must be a list")
    if not declarations:
        return []
    elf = Elf32(elf_path)
    verified = []
    for index, row in enumerate(declarations):
        identifier, label, expected_trainer, expected_mode, argument_offset = _required_caller(row, index)
        if argument_offset != TRAINERBATTLE_ARGUMENT_OFFSET:
            _error(elf.path, label, f"argument offset must be {TRAINERBATTLE_ARGUMENT_OFFSET}, got {argument_offset}")
        address = elf.symbol_address(label)
        opcode = elf.byte_at(address, label)
        if opcode != TRAINERBATTLE_OPCODE:
            if elf.has_byte_at(address - 1) and elf.byte_at(address - 1, label) == TRAINERBATTLE_OPCODE:
                _error(elf.path, label, "symbol points at trainerbattle arguments; it must point at the opcode")
            _error(elf.path, label, f"expected trainerbattle opcode 0x{TRAINERBATTLE_OPCODE:02X}, got 0x{opcode:02X}")
        flags = elf.byte_at(address + TRAINERBATTLE_FLAGS_OFFSET, label)
        actual_mode = flags >> 4
        if actual_mode != expected_mode:
            _error(elf.path, label, f"expected mode {expected_mode}, got {actual_mode}")
        actual_trainer = elf.u16_at(address + TRAINERBATTLE_TRAINER_OFFSET, label)
        if actual_trainer != expected_trainer:
            _error(elf.path, label, f"expected trainer {expected_trainer}, got {actual_trainer}")
        verified.append({"id": identifier, "label": label, "address": address,
                         "argumentAddress": address + argument_offset,
                         "mode": actual_mode, "trainer": actual_trainer})
    return verified


def verify_report(elf_path, report):
    """Verify the ``encounters`` projection in a gameplay-content inventory."""
    if not isinstance(report, dict):
        _error("<inventory>", "", "report must be an object")
    if "encounters" not in report:
        _error("<inventory>", "", "encounters projection is required")
    if "domains" in report:
        domains = report["domains"]
        if not isinstance(domains, list) or "encounters" not in domains:
            _error("<inventory>", "", "encounters domain is required for post-link validation")
    projection = report["encounters"]
    if not isinstance(projection, dict):
        _error("<inventory>", "", "encounters projection must be an object")
    if "records" in projection:
        _error("<inventory>", "", "encounters projection must not contain records")
    if "encounters" not in projection:
        _error("<inventory>", "", "encounters projection must contain encounters")
    encounters = projection["encounters"]
    if not isinstance(encounters, list):
        _error("<inventory>", "", "encounters projection must contain an encounters list")
    return verify_encounter_callers(elf_path, encounters)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True,
                        help="gameplay-content inventory JSON containing encounters")
    args = parser.parse_args()
    try:
        verified = verify_report(args.elf, load_json(args.report))
    except (ContentError, ValueError) as exc:
        parser.exit(1, str(exc) + "\n")
    print(f"Validated {len(verified)} linked encounter caller(s).")


if __name__ == "__main__":
    main()
