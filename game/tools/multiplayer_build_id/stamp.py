#!/usr/bin/env python3
"""Stamp a PoC ROM with a digest of its compiled bytes, excluding the stamp."""

import argparse
import hashlib
from pathlib import Path


MARKER = b"WFPBIDv1"
TAIL = b"BID!"
ZERO_ID = bytes(16)


def find_tag(rom: bytes) -> int:
    start = rom.find(MARKER)
    if start < 0 or rom.find(MARKER, start + 1) >= 0:
        raise ValueError("expected exactly one multiplayer build-ID marker")
    id_offset = start + len(MARKER)
    if rom[id_offset + 16 : id_offset + 20] != TAIL:
        raise ValueError("multiplayer build-ID tag is malformed")
    return id_offset


def digest(rom: bytes, id_offset: int) -> bytes:
    pristine = bytearray(rom)
    pristine[id_offset : id_offset + 16] = ZERO_ID
    return hashlib.sha256(pristine).digest()[:16]


def run(path: Path, verify: bool) -> str:
    rom = path.read_bytes()
    id_offset = find_tag(rom)
    expected = digest(rom, id_offset)
    actual = rom[id_offset : id_offset + 16]
    if verify:
        if actual != expected:
            raise ValueError("multiplayer build ID does not match ROM contents")
    elif actual == ZERO_ID:
        patched = bytearray(rom)
        patched[id_offset : id_offset + 16] = expected
        path.write_bytes(patched)
    elif actual != expected:
        raise ValueError("ROM has an unexpected existing multiplayer build ID")
    return expected.hex()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--verify", action="store_true")
    args = parser.parse_args()
    print(run(args.rom, args.verify))


if __name__ == "__main__":
    main()
