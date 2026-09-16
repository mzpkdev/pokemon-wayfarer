#!/usr/bin/env python3
"""Pack Wayfarer's title artwork for the held Emerald Scene 1 composition.

The Pokémon logo is an 8bpp 256x64 image, but the GBA can only display it as
four 64x64 OBJ sprites.  This tool changes the tile order (not the pixels) so
each quarter is a contiguous OBJ frame.  It also remaps the existing 4bpp
WAYFARER banner to unused entries in the logo's 256-colour OBJ palette.

This intentionally has no Pillow dependency: title asset regeneration must be
possible in the normal game toolchain as well as the art-review environment.
"""

from __future__ import annotations

import argparse
import binascii
import json
import struct
import zlib
from pathlib import Path


GAME = Path(__file__).resolve().parents[1]
BACKGROUND = GAME / "graphics/title_screen/wayfarer/background"
DEFAULT_OUTPUT = GAME / "graphics/title_screen/wayfarer/overlays"


def chunks(data: bytes):
    pos = 8
    while pos < len(data):
        length = struct.unpack_from(">I", data, pos)[0]
        kind = data[pos + 4:pos + 8]
        payload = data[pos + 8:pos + 8 + length]
        yield kind, payload
        pos += 12 + length


def read_indexed_png(path: Path) -> tuple[int, int, bytes, list[tuple[int, int, int]]]:
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path} is not a PNG")
    header = palette = compressed = None
    for kind, payload in chunks(data):
        if kind == b"IHDR":
            header = struct.unpack(">IIBBBBB", payload)
        elif kind == b"PLTE":
            palette = [tuple(payload[i:i + 3]) for i in range(0, len(payload), 3)]
        elif kind == b"IDAT":
            compressed = (compressed or b"") + payload
    if header is None or palette is None or compressed is None:
        raise ValueError(f"{path} lacks indexed PNG data")
    width, height, depth, colour_type, compression, filter_method, interlace = header
    if colour_type != 3 or depth not in (4, 8) or (compression, filter_method, interlace) != (0, 0, 0):
        raise ValueError(f"{path} must be a non-interlaced indexed 4bpp or 8bpp PNG")
    raw = zlib.decompress(compressed)
    stride = (width * depth + 7) // 8
    rows = bytearray(width * height)
    previous = bytearray(stride)
    offset = 0
    for y in range(height):
        filter_type = raw[offset]
        source = raw[offset + 1:offset + 1 + stride]
        offset += stride + 1
        row = bytearray(source)
        for x in range(stride):
            left = row[x - 1] if x else 0
            up = previous[x]
            up_left = previous[x - 1] if x else 0
            if filter_type == 1:
                row[x] = (row[x] + left) & 0xFF
            elif filter_type == 2:
                row[x] = (row[x] + up) & 0xFF
            elif filter_type == 3:
                row[x] = (row[x] + ((left + up) >> 1)) & 0xFF
            elif filter_type == 4:
                p = left + up - up_left
                pa, pb, pc = abs(p - left), abs(p - up), abs(p - up_left)
                row[x] = (row[x] + (left if pa <= pb and pa <= pc else up if pb <= pc else up_left)) & 0xFF
            elif filter_type != 0:
                raise ValueError(f"{path} has unsupported PNG filter {filter_type}")
        rows[y * width:(y + 1) * width] = row
        previous = row
    if depth == 8:
        return width, height, bytes(rows), palette
    unpacked = bytearray(width * height)
    for y in range(height):
        for x in range(width):
            value = rows[y * stride + x // 2]
            unpacked[y * width + x] = value >> 4 if x % 2 == 0 else value & 0x0F
    return width, height, bytes(unpacked), palette


def png_chunk(kind: bytes, payload: bytes) -> bytes:
    return (struct.pack(">I", len(payload)) + kind + payload
            + struct.pack(">I", binascii.crc32(kind + payload) & 0xFFFFFFFF))


def write_indexed_png(path: Path, width: int, height: int, pixels: bytes, palette: list[tuple[int, int, int]]) -> None:
    if len(pixels) != width * height or len(palette) != 256:
        raise ValueError("invalid indexed image dimensions or palette")
    rows = b"".join(b"\0" + pixels[y * width:(y + 1) * width] for y in range(height))
    payload = (b"\x89PNG\r\n\x1a\n"
               + png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 3, 0, 0, 0))
               + png_chunk(b"PLTE", b"".join(bytes(rgb) for rgb in palette))
               + png_chunk(b"tRNS", bytes([0] + [255] * 255))
               + png_chunk(b"IDAT", zlib.compress(rows, 9))
               + png_chunk(b"IEND", b""))
    path.write_bytes(payload)


def read_jasc_palette(path: Path) -> list[tuple[int, int, int]]:
    rows = path.read_text(encoding="ascii").splitlines()
    if rows[:2] != ["JASC-PAL", "0100"]:
        raise ValueError(f"{path} is not a JASC palette")
    count = int(rows[2])
    values = [tuple(map(int, row.split())) for row in rows[3:3 + count]]
    if len(values) != count:
        raise ValueError(f"{path} has an incomplete palette")
    return values


def write_jasc_palette(path: Path, palette: list[tuple[int, int, int]]) -> None:
    rows = ["JASC-PAL", "0100", str(len(palette)), *(f"{r} {g} {b}" for r, g, b in palette)]
    path.write_bytes(("\r\n".join(rows) + "\r\n").encode("ascii"))


def rgb555(rgb: tuple[int, int, int]) -> int:
    r, g, b = rgb
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def pack_logo_obj(pixels: bytes, width: int, height: int) -> bytes:
    if (width, height) != (256, 64):
        raise ValueError("Pokémon logo must be 256x64")
    packed = bytearray()
    for part in range(4):
        for tile_y in range(8):
            for tile_x in range(8):
                x0, y0 = part * 64 + tile_x * 8, tile_y * 8
                for y in range(8):
                    packed.extend(pixels[(y0 + y) * width + x0:(y0 + y) * width + x0 + 8])
    if len(packed) != 0x4000:
        raise AssertionError("packed logo must occupy four 64x64 8bpp OBJ frames")
    return bytes(packed)


def tile_bytes_to_image(tile_bytes: bytes, width: int, height: int) -> bytes:
    """Arrange tile-order bytes as a PNG raster for gbagfx's tile scanner."""
    if len(tile_bytes) != width * height or width % 8 or height % 8:
        raise ValueError("invalid tile image")
    pixels = bytearray(width * height)
    tiles_per_row = width // 8
    for tile_index in range(len(tile_bytes) // 64):
        tile_x = (tile_index % tiles_per_row) * 8
        tile_y = (tile_index // tiles_per_row) * 8
        for y in range(8):
            source = tile_index * 64 + y * 8
            destination = (tile_y + y) * width + tile_x
            pixels[destination:destination + 8] = tile_bytes[source:source + 8]
    return bytes(pixels)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--logo", type=Path, default=BACKGROUND / "logo.png")
    parser.add_argument("--shared-palette", type=Path, default=BACKGROUND / "shared.pal")
    parser.add_argument("--banner", type=Path, default=GAME / "graphics/title_screen/wayfarer/wayfarer_version.png")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)

    logo_w, logo_h, logo_pixels, logo_palette = read_indexed_png(args.logo)
    shared = read_jasc_palette(args.shared_palette)
    if len(shared) != 256 or len(logo_palette) != 256:
        raise ValueError("Wayfarer logo and shared palette must have 256 entries")
    if [rgb555(c) for c in logo_palette] != [rgb555(c) for c in shared]:
        raise ValueError("logo palette does not match the shared title palette")

    banner_w, banner_h, banner_pixels, banner_palette = read_indexed_png(args.banner)
    if (banner_w, banner_h) != (64, 64):
        raise ValueError("WAYFARER banner must be the established 64x64 packed sheet")
    if len(banner_palette) < 3:
        raise ValueError("WAYFARER banner palette is incomplete")

    # The converter reserves 184..255.  Place the banner's literal white and
    # black there so its 8bpp pixels cannot alter any logo colours.
    WHITE_INDEX, BLACK_INDEX = 184, 185
    shared[WHITE_INDEX] = (255, 255, 255)
    shared[BLACK_INDEX] = (0, 0, 0)
    banner_obj = bytes(0 if pixel == 0 else WHITE_INDEX if pixel == 1 else BLACK_INDEX if pixel == 2 else 0
                       for pixel in banner_pixels)
    if set(banner_obj) - {0, WHITE_INDEX, BLACK_INDEX}:
        raise AssertionError("banner palette remap escaped its reserved entries")

    packed_logo = pack_logo_obj(logo_pixels, logo_w, logo_h)
    # gbagfx scans PNGs tile by tile, so turn the target tile stream back into
    # a normal raster before it serializes the four 64x64 OBJ frames.
    write_indexed_png(output / "pokemon_logo_obj.png", 64, 256, tile_bytes_to_image(packed_logo, 64, 256), shared)
    write_indexed_png(output / "wayfarer_version_obj.png", 64, 64, banner_obj, shared)
    write_jasc_palette(output / "overlay_palette.pal", shared)
    (output / "manifest.json").write_text(json.dumps({
        "format": 1,
        "logo": {"source": str(args.logo), "obj_sheet_size": [64, 256], "bytes": len(packed_logo), "frames": 4},
        "banner": {"source": str(args.banner), "obj_sheet_size": [64, 64], "bytes": len(banner_obj)},
        "palette": {"source": str(args.shared_palette), "entries": 256, "banner_white": WHITE_INDEX, "banner_black": BLACK_INDEX,
                    "reserved_4bpp_banks": [12, 13]},
        "obj_budget": {"logo": 0x4000, "banner": 0x1000, "press_start": 0x520, "flygon": 0x400, "total": 0x5920, "capacity": 0x8000},
    }, indent=2) + "\n", encoding="utf-8")
    print(f"Packed Wayfarer title overlays in {output}")


if __name__ == "__main__":
    main()
