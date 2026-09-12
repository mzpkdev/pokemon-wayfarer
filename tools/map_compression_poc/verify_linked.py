#!/usr/bin/env python3
"""Verify linked experiment payloads/descriptors/IDs against authored inputs."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import zlib

from catalog import lz77_decode

ROOT = Path(__file__).resolve().parents[2]
ART = Path(__file__).resolve().parent / "artifacts"
OUT = ART / "linked"


def main():
    global OUT
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=OUT, help="linked_storage.py experiment directory")
    OUT = parser.parse_args().output.resolve()
    rows = json.loads((ART / "catalog/per-layout.json").read_text())
    table = re.findall(r"(?m)^\s*\.4byte (\w+)$", (ROOT / "game/data/layouts/layouts_table.inc").read_text())
    results = {}
    for mode in ("legacy", "raw-control", "hybrid"):
        elf = OUT / mode / "pokewayfarer-release.elf"
        nm = subprocess.check_output(["arm-none-eabi-nm", "-n", str(elf)], text=True)
        syms = {}
        for line in nm.splitlines():
            parts = line.split()
            if len(parts) == 3:
                syms[parts[2]] = int(parts[0], 16)
        rom = (elf.with_suffix(".gba")).read_bytes()

        def data(addr, size):
            assert 0x08000000 <= addr <= addr + size <= syms["__rom_end"]
            return rom[addr - 0x08000000:addr - 0x08000000 + size]

        for i, name in enumerate(table):
            ptr, = struct.unpack("<I", data(syms["gMapLayouts"] + 4 * i, 4))
            assert ptr == (0 if name == "NULL" else syms[name]), (mode, i, name)
        for row in rows:
            name = row["layout_name"]
            w, h, border, ptr = struct.unpack("<IIII", data(syms[name], 16))
            assert (w, h) == (row["width"], row["height"])
            assert table[row["table_slot"] - 1] == name
            if mode == "legacy":
                assert ptr == syms[name + "_Blockdata"]
                raw = data(ptr, row["raw_file_bytes"])
            else:
                assert ptr == syms[name + "_PocDescriptor"] and ptr % 4 == 0
                payload, stored, decoded, logical, stored_crc, raw_crc, schema, codec, flags = struct.unpack("<IIIIIIBBH", data(ptr, 28))
                assert payload == syms[name + "_Blockdata"] and payload % 4 == 0
                assert (decoded, logical, schema, flags) == (row["raw_file_bytes"], row["logical_bytes"], 1, 0)
                assert codec == int(mode == "hybrid" and row["storage"] == "gba_lz77")
                stream = data(payload, stored)
                assert zlib.crc32(stream) == stored_crc
                if codec:
                    raw, end = lz77_decode(stream)
                    assert stored == (end + 3) & ~3 and not any(stream[end:])
                else:
                    raw = stream
                assert len(raw) == decoded and zlib.crc32(raw) == raw_crc
            assert hashlib.sha256(raw).hexdigest() == row["source_sha256"]
            assert raw == (ROOT / "game" / row["source_path"]).read_bytes()
        results[mode] = {"verified_layouts": len(rows), "verified_table_slots": len(table),
                         "rom_end": hex(syms["__rom_end"]), "used_bytes": syms["__rom_end"] - 0x08000000,
                         "elf_sha256": hashlib.sha256(elf.read_bytes()).hexdigest()}
    results["storage_only_saved_bytes"] = results["legacy"]["used_bytes"] - results["hybrid"]["used_bytes"]
    results["raw_control_overhead_bytes"] = results["raw-control"]["used_bytes"] - results["legacy"]["used_bytes"]
    (OUT / "verified.json").write_text(json.dumps(results, indent=2) + "\n")
    print(json.dumps(results, indent=2))


if __name__ == "__main__":
    main()
