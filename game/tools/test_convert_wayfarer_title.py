#!/usr/bin/env python3
"""Checks for the Wayfarer title-background converter."""

from __future__ import annotations

import hashlib
import importlib.util
import tempfile
import unittest
from pathlib import Path

from PIL import Image


MODULE_PATH = Path(__file__).with_name("convert_wayfarer_title.py")
SPEC = importlib.util.spec_from_file_location("convert_wayfarer_title", MODULE_PATH)
assert SPEC and SPEC.loader
converter = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(converter)


def write_logo(path: Path) -> None:
    image = Image.new("P", (256, 64), 0)
    palette = [0] * 768
    # Index 0 is transparent black. Index 1 is deliberately opaque black.
    palette[3:6] = [0, 0, 0]
    palette[6:9] = [255, 0, 0]
    image.putpalette(palette)
    image.putdata([0, 1, 2] + [0] * (256 * 64 - 3))
    image.save(path)


def write_source(path: Path, transparent: bool = False) -> None:
    image = Image.new("RGBA", (248, 177), (56, 112, 184, 255))
    for x in range(124, 248):
        for y in range(88, 177):
            image.putpixel((x, y), (232, 136, 72, 255))
    if transparent:
        image.putpixel((0, 0), (0, 0, 0, 0))
    image.save(path)


class ConvertWayfarerTitleTests(unittest.TestCase):
    def test_rgb555_conversion_has_a_concrete_display_colour(self) -> None:
        self.assertEqual(converter.rgb555((57, 113, 185)), (7, 14, 23))
        self.assertEqual(converter.rgb888_from_555((7, 14, 23)), (56, 112, 184))

    def test_logo_reserves_transparent_zero_and_preserves_opaque_black(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            logo = Path(directory) / "logo.png"
            write_logo(logo)
            palette, pixels, _, colour_count = converter.compact_logo(logo)
        self.assertEqual(pixels[:3], bytes([0, 1, 2]))
        self.assertEqual(palette[0], (0, 0, 0))
        self.assertEqual(palette[1], (0, 0, 0))
        self.assertEqual(colour_count, 2)

    def test_scene_map_uses_scanline_tile_order_and_keeps_unused_entries_blank(self) -> None:
        pixels = bytearray(converter.SCREEN_WIDTH * converter.SCREEN_HEIGHT)
        for y in range(8):
            for x in range(8):
                pixels[y * converter.SCREEN_WIDTH + x] = 1
                pixels[y * converter.SCREEN_WIDTH + 8 + x] = 2
        tiles, tilemap = converter.scene_tiles(bytes(pixels))
        self.assertEqual(tiles[0], bytes(64))
        self.assertEqual(tiles[1], bytes([1]) * 64)
        self.assertEqual(tiles[2], bytes([2]) * 64)
        self.assertEqual(tilemap[:6], b"\x01\x00\x02\x00\x00\x00")
        self.assertEqual(tilemap[(20 * 32) * 2:], bytes((32 * 12) * 2))

    def test_generation_is_deterministic_and_round_trips_native_scene(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, logo, output = root / "source.png", root / "logo.png", root / "out"
            write_source(source)
            write_logo(logo)
            first = converter.build_assets(source, output, logo)
            before = {path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in output.iterdir()}
            second = converter.build_assets(source, output, logo)
            after = {path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in output.iterdir()}
            decoded = converter.decode_scene_indices(
                (output / "scene.8bpp").read_bytes(), (output / "scene.bin").read_bytes())
            with Image.open(output / "scene.png") as scene_file:
                palette = scene_file.getpalette()
            colour_offset = decoded[0] * 3
            self.assertEqual(tuple(palette[colour_offset:colour_offset + 3]), (56, 112, 184))
        self.assertEqual(first["tiles"], second["tiles"])
        self.assertEqual(before, after)
        self.assertEqual(len(decoded), 240 * 160)
        self.assertGreater(first["tiles"]["scene_count"], 1)
        self.assertLessEqual(first["tiles"]["scene_bytes"], converter.SCENE_TILE_BYTE_CAP)

    def test_scene_index_round_trip_preserves_the_logo_palette_offset(self) -> None:
        pixels = bytes([3]) * (converter.SCREEN_WIDTH * converter.SCREEN_HEIGHT)
        offset_pixels = bytes(value + 136 for value in pixels)
        tiles, tilemap = converter.scene_tiles(offset_pixels)
        self.assertEqual(converter.decode_scene_indices(b"".join(tiles), tilemap), offset_pixels)

    def test_transparent_and_malformed_inputs_are_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            transparent, malformed = root / "transparent.png", root / "broken.png"
            write_source(transparent, transparent=True)
            malformed.write_text("this is not a png", encoding="ascii")
            with self.assertRaisesRegex(ValueError, "fully opaque"):
                converter.read_opaque_rgb(transparent)
            with self.assertRaisesRegex(ValueError, "cannot read title source"):
                converter.read_opaque_rgb(malformed)


if __name__ == "__main__":
    unittest.main()
