#!/usr/bin/env python3
"""Build deterministic 8bpp assets for the Wayfarer title scene.

The source concept stays as an ordinary RGBA PNG.  This tool creates the indexed
tile, map, and palette sources that the ROM's normal graphics rules consume.
"""

from __future__ import annotations

import argparse
import struct
from collections import Counter
from pathlib import Path
from typing import Iterable

from PIL import Image


SCREEN_WIDTH = 240
SCREEN_HEIGHT = 160
MAP_WIDTH_TILES = 32
MAP_HEIGHT_TILES = 32
TILE_SIZE = 8
SCENE_TILE_BYTE_CAP = 0xB000
LOGO_TILE_BYTE_CAP = 0x4000
PALETTE_ENTRIES = 256

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = ROOT / "graphics/title_screen/wayfarer/background"
DEFAULT_INPUT = DEFAULT_OUTPUT / "source.png"
DEFAULT_LOGO = ROOT / "graphics/title_screen/hns/pokemon_logo.png"


def rgb555(rgb: tuple[int, int, int]) -> tuple[int, int, int]:
    """Return the three five-bit GBA channels for an RGB888 colour."""
    return tuple(channel // 8 for channel in rgb)  # type: ignore[return-value]


def rgb888_from_555(colour: tuple[int, int, int]) -> tuple[int, int, int]:
    """Use values gbagfx converts back to this exact RGB555 triplet."""
    return tuple(channel * 8 for channel in colour)  # type: ignore[return-value]


def gba_palette_bytes(palette: list[tuple[int, int, int]]) -> bytes:
    if len(palette) != PALETTE_ENTRIES:
        raise ValueError(f"palette must have {PALETTE_ENTRIES} entries")
    return b"".join(struct.pack("<H", red | (green << 5) | (blue << 10)) for red, green, blue in palette)


def read_opaque_rgb(path: Path) -> Image.Image:
    try:
        image = Image.open(path)
        image.load()
    except (OSError, ValueError) as error:
        raise ValueError(f"cannot read title source {path}: {error}") from error
    rgba = image.convert("RGBA")
    if any(alpha != 255 for alpha in rgba.getchannel("A").get_flattened_data()):
        raise ValueError("title source must be fully opaque; transparency would consume palette index 0")
    return rgba.convert("RGB")


def compact_logo(path: Path) -> tuple[list[tuple[int, int, int]], bytes, Image.Image, int]:
    """Reserve zero for transparency and compact used opaque logo colours."""
    try:
        logo = Image.open(path)
        logo.load()
    except (OSError, ValueError) as error:
        raise ValueError(f"cannot read logo source {path}: {error}") from error
    if logo.mode != "P" or logo.size != (256, 64):
        raise ValueError("logo source must be a 256x64 indexed PNG")

    source_pixels = bytes(logo.get_flattened_data())
    source_palette = logo.getpalette()
    if source_palette is None:
        raise ValueError("logo source has no palette")

    compact: list[tuple[int, int, int]] = [(0, 0, 0)]
    lookup: dict[tuple[int, int, int], int] = {}
    remapped = bytearray(len(source_pixels))
    for position, source_index in enumerate(source_pixels):
        if source_index == 0:
            continue
        offset = source_index * 3
        colour = rgb555(tuple(source_palette[offset:offset + 3]))
        target_index = lookup.get(colour)
        if target_index is None:
            target_index = len(compact)
            lookup[colour] = target_index
            compact.append(colour)
        remapped[position] = target_index
    if len(compact) >= PALETTE_ENTRIES:
        raise ValueError(f"logo uses {len(compact) - 1} RGB555 colours; no scene palette entries remain")

    png = indexed_image(logo.size, bytes(remapped), compact + [(0, 0, 0)] * (PALETTE_ENTRIES - len(compact)))
    return compact, bytes(remapped), png, len(compact) - 1


def scene_rgb555(image: Image.Image, max_colours: int) -> tuple[list[tuple[int, int, int]], bytes]:
    """Nearest-neighbour scale, then quantize to at most ``max_colours`` RGB555 values."""
    resized = image.resize((SCREEN_WIDTH, SCREEN_HEIGHT), Image.Resampling.NEAREST)
    pixels = list(resized.get_flattened_data())
    colours = {rgb555(pixel) for pixel in pixels}
    if len(colours) > max_colours:
        # Pillow's median-cut result is deterministic for a fixed Pillow version.
        quantized = resized.quantize(colors=max_colours, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE).convert("RGB")
        pixels = list(quantized.get_flattened_data())

    palette: list[tuple[int, int, int]] = []
    lookup: dict[tuple[int, int, int], int] = {}
    indexed = bytearray()
    for pixel in pixels:
        colour = rgb555(pixel)
        index = lookup.get(colour)
        if index is None:
            index = len(palette)
            lookup[colour] = index
            palette.append(colour)
        indexed.append(index)
    if len(palette) > max_colours:
        raise AssertionError("RGB555 quantization exceeded its requested colour budget")
    return palette, bytes(indexed)


def tiles_from_pixels(pixels: bytes, width: int, height: int) -> list[bytes]:
    if width % TILE_SIZE or height % TILE_SIZE or len(pixels) != width * height:
        raise ValueError("tile source must be complete and aligned to 8x8 tiles")
    tiles = []
    for tile_y in range(0, height, TILE_SIZE):
        for tile_x in range(0, width, TILE_SIZE):
            tiles.append(b"".join(pixels[(tile_y + row) * width + tile_x:(tile_y + row) * width + tile_x + TILE_SIZE] for row in range(TILE_SIZE)))
    return tiles


def scene_tiles(scene_pixels: bytes) -> tuple[list[bytes], bytes]:
    """Deduplicate 240x160 scene tiles and place them in a 32x32 text map."""
    source_tiles = tiles_from_pixels(scene_pixels, SCREEN_WIDTH, SCREEN_HEIGHT)
    blank = bytes(TILE_SIZE * TILE_SIZE)
    tiles = [blank]
    indexes = {blank: 0}
    scene_map: list[int] = []
    for tile in source_tiles:
        index = indexes.get(tile)
        if index is None:
            index = len(tiles)
            indexes[tile] = index
            tiles.append(tile)
        scene_map.append(index)
    if len(tiles) * 64 > SCENE_TILE_BYTE_CAP:
        raise ValueError(f"scene tile data is {len(tiles) * 64:#x}, above {SCENE_TILE_BYTE_CAP:#x}")
    if len(tiles) > 1024:
        raise ValueError("scene needs more than the 10-bit text-map tile index permits")

    full_map = [0] * (MAP_WIDTH_TILES * MAP_HEIGHT_TILES)
    for y in range(SCREEN_HEIGHT // TILE_SIZE):
        start = y * MAP_WIDTH_TILES
        full_map[start:start + SCREEN_WIDTH // TILE_SIZE] = scene_map[y * (SCREEN_WIDTH // TILE_SIZE):(y + 1) * (SCREEN_WIDTH // TILE_SIZE)]
    return tiles, b"".join(struct.pack("<H", index) for index in full_map)


def indexed_image(size: tuple[int, int], pixels: bytes, palette: list[tuple[int, int, int]]) -> Image.Image:
    if len(palette) != PALETTE_ENTRIES:
        raise ValueError("indexed PNG palettes must contain 256 entries")
    image = Image.frombytes("P", size, pixels)
    image.putpalette([channel for colour in palette for channel in rgb888_from_555(colour)])
    image.info["transparency"] = 0
    return image


def tiles_atlas(tiles: Iterable[bytes], palette: list[tuple[int, int, int]]) -> Image.Image:
    serial = b"".join(tiles)
    if len(serial) % 64:
        raise ValueError("tile data must contain complete 8x8 tiles")
    # One tile per row means gbagfx emits bytes in exactly this order.
    return indexed_image((8, len(serial) // 8), serial, palette)


def write_jasc_palette(path: Path, palette: list[tuple[int, int, int]]) -> None:
    rows = ["JASC-PAL", "0100", str(PALETTE_ENTRIES)]
    rows.extend("{} {} {}".format(*rgb888_from_555(colour)) for colour in palette)
    path.write_bytes(("\r\n".join(rows) + "\r\n").encode("ascii"))


def decode_scene_indices(scene_tiles_data: bytes, tilemap: bytes) -> bytes:
    """Decode the visible 30x20 text-map area back to indexed pixels."""
    decoded = bytearray(SCREEN_WIDTH * SCREEN_HEIGHT)
    for tile_y in range(SCREEN_HEIGHT // TILE_SIZE):
        for tile_x in range(SCREEN_WIDTH // TILE_SIZE):
            tile_index = struct.unpack_from("<H", tilemap, (tile_y * MAP_WIDTH_TILES + tile_x) * 2)[0]
            if tile_index & 0xFC00:
                raise ValueError("scene text map contains palette-bank or flip bits")
            tile_offset = tile_index * 64
            if tile_offset + 64 > len(scene_tiles_data):
                raise ValueError("scene map references a tile beyond scene data")
            for row in range(TILE_SIZE):
                source_offset = tile_offset + row * TILE_SIZE
                destination_offset = (tile_y * TILE_SIZE + row) * SCREEN_WIDTH + tile_x * TILE_SIZE
                decoded[destination_offset:destination_offset + TILE_SIZE] = scene_tiles_data[source_offset:source_offset + TILE_SIZE]
    return bytes(decoded)


def build_assets(input_path: Path, output: Path, logo_path: Path) -> dict[str, object]:
    output.mkdir(parents=True, exist_ok=True)
    source = read_opaque_rgb(input_path)
    logo_palette, logo_pixels, logo_png, logo_colour_count = compact_logo(logo_path)
    scene_palette, scene_pixels = scene_rgb555(source, PALETTE_ENTRIES - len(logo_palette))
    shared_palette = logo_palette + scene_palette
    shared_palette.extend([(0, 0, 0)] * (PALETTE_ENTRIES - len(shared_palette)))

    scene_index_offset = len(logo_palette)
    scene_pixels = bytes(index + scene_index_offset for index in scene_pixels)
    scene_tiles_data, tilemap = scene_tiles(scene_pixels)
    scene_raw = b"".join(scene_tiles_data)
    logo_raw = b"".join(tiles_from_pixels(logo_pixels, 256, 64))
    if len(logo_raw) > LOGO_TILE_BYTE_CAP:
        raise ValueError(f"logo tile data is {len(logo_raw):#x}, above {LOGO_TILE_BYTE_CAP:#x}")
    if decode_scene_indices(scene_raw, tilemap) != scene_pixels:
        raise AssertionError("scene tiles and text map failed an indexed-pixel round trip")
    scene_png = tiles_atlas(scene_tiles_data, shared_palette)
    logo_png.putpalette([channel for colour in shared_palette for channel in rgb888_from_555(colour)])
    scene_png.save(output / "scene.png", optimize=False)
    logo_png.save(output / "logo.png", optimize=False)
    (output / "scene.8bpp").write_bytes(scene_raw)
    (output / "scene.bin").write_bytes(tilemap)
    (output / "logo.8bpp").write_bytes(logo_raw)
    (output / "shared.gbapal").write_bytes(gba_palette_bytes(shared_palette))
    write_jasc_palette(output / "shared.pal", shared_palette)

    return {
        "palette": {"entries": PALETTE_ENTRIES, "transparent_index": 0, "logo_rgb555_colours": logo_colour_count, "scene_rgb555_colours": len(scene_palette), "used_entries": len(logo_palette) + len(scene_palette)},
        "tiles": {"scene_count": len(scene_tiles_data), "scene_bytes": len(scene_raw), "scene_byte_cap": SCENE_TILE_BYTE_CAP, "scene_capacity_tiles": SCENE_TILE_BYTE_CAP // 64, "logo_count": len(logo_raw) // 64, "logo_bytes": len(logo_raw), "logo_byte_cap": LOGO_TILE_BYTE_CAP},
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT, help="opaque full-colour concept PNG")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="generated background directory")
    parser.add_argument("--logo-source", type=Path, default=DEFAULT_LOGO, help="existing HNS affine Pokemon logo PNG")
    args = parser.parse_args()
    result = build_assets(args.input.resolve(), args.output.resolve(), args.logo_source.resolve())
    tiles = result["tiles"]
    palette = result["palette"]
    print(f"Generated {tiles['scene_count']} scene tiles ({tiles['scene_bytes']:#x}) with {palette['used_entries']} shared palette entries.")


if __name__ == "__main__":
    main()
