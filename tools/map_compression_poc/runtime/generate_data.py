#!/usr/bin/env python3
"""Generate the two-map runtime POC payloads without modifying game assets."""
from __future__ import annotations

import hashlib
import json
import shutil
import subprocess
import zlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "tools/map_compression_poc/artifacts/runtime/data"
GBAGFX = ROOT / "tools/map_compression_poc/artifacts/catalog/gbagfx"
PICKS = (
    ("route47", "Route47_hns", "LAYOUT_ROUTE47_HNS"),
    ("route48", "Route48_hns", "LAYOUT_ROUTE48_HNS"),
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decode(stream: bytes) -> tuple[bytes, int]:
    if len(stream) < 4 or stream[0] != 0x10:
        raise ValueError("not a GBA LZ77 stream")
    size = stream[1] | stream[2] << 8 | stream[3] << 16
    output = bytearray()
    pos = 4
    while len(output) < size:
        flags = stream[pos]
        pos += 1
        for bit in range(8):
            if len(output) == size:
                break
            if flags & (0x80 >> bit):
                token = stream[pos] << 8 | stream[pos + 1]
                pos += 2
                count, distance = (token >> 12) + 3, (token & 0xFFF) + 1
                if distance > len(output) or len(output) + count > size:
                    raise ValueError("invalid LZ77 match")
                for _ in range(count):
                    output.append(output[-distance])
            else:
                output.append(stream[pos])
                pos += 1
    return bytes(output), pos


def main() -> None:
    layouts = {item["id"]: item for item in json.loads((ROOT / "game/data/layouts/layouts.json").read_text())["layouts"] if item}
    OUT.mkdir(parents=True, exist_ok=True)
    rows = []
    for short, map_name, layout_id in PICKS:
        layout = layouts[layout_id]
        raw_src = ROOT / "game" / layout["blockdata_filepath"]
        raw_dst = OUT / f"{short}.raw"
        lz_dst = OUT / f"{short}.lz"
        shutil.copyfile(raw_src, raw_dst)
        subprocess.run([str(GBAGFX), str(raw_dst), str(lz_dst)], check=True)
        decoded, consumed = decode(lz_dst.read_bytes())
        raw = raw_dst.read_bytes()
        if decoded != raw:
            raise ValueError(f"round trip failed for {short}")
        map_json = json.loads((ROOT / "game/data/maps" / map_name / "map.json").read_text())
        rows.append({
            "symbol": short,
            "map": map_name,
            "layout_id": layout_id,
            "width": layout["width"], "height": layout["height"],
            "raw_bytes": len(raw), "raw_sha256": digest(raw_dst),
            "lz_bytes": lz_dst.stat().st_size, "lz_sha256": digest(lz_dst),
            "lz_decode_consumed": consumed,
            "connections": map_json.get("connections", []),
            "authored_source": str(raw_src.relative_to(ROOT)),
            "authored_source_sha256": digest(raw_src),
        })
    (OUT / "identity.json").write_text(json.dumps({
        "source_revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        "gbagfx": str(GBAGFX.relative_to(ROOT)), "gbagfx_sha256": digest(GBAGFX),
        "maps": rows,
        "chosen_connection": "Route47_hns north to Route48_hns: map.json direction up, offset 19",
    }, indent=2, sort_keys=True) + "\n")
    constants = ["/* Generated from the exact runtime POC input artifacts. */"]
    for row in rows:
        prefix = row["symbol"].upper()
        constants.extend((
            f"#define {prefix}_RAW_BYTES {row['raw_bytes']}u",
            f"#define {prefix}_LZ_BYTES {row['lz_bytes']}u",
            f"#define {prefix}_RAW_CRC 0x{zlib.crc32((OUT / (row['symbol'] + '.raw')).read_bytes()):08x}u",
            f"#define {prefix}_LZ_CRC 0x{zlib.crc32((OUT / (row['symbol'] + '.lz')).read_bytes()):08x}u",
        ))
    table = [zlib.crc32(bytes([i])) ^ zlib.crc32(b"") for i in range(256)]
    # zlib's public CRC API includes init/final xor; generate the reflected table directly.
    table = []
    for i in range(256):
        value = i
        for _ in range(8): value = (value >> 1) ^ (0xedb88320 if value & 1 else 0)
        table.append(value)
    constants.append("static const unsigned int poc_crc32_table[256] = {" + ",".join(f"0x{x:08x}u" for x in table) + "};")
    (OUT / "generated.h").write_text("\n".join(constants) + "\n")
    asm = [".section .rodata", ".align 2"]
    for row in rows:
        for kind in ("raw", "lz"):
            asm.extend((f".global poc_{row['symbol']}_{kind}", f"poc_{row['symbol']}_{kind}:", f"  .incbin \"{(OUT / (row['symbol'] + '.' + kind)).resolve()}\"", ".align 2"))
    (OUT / "maps.s").write_text("\n".join(asm) + "\n")


if __name__ == "__main__":
    main()
