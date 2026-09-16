#!/usr/bin/env python3
"""Fit the reviewed Wayfarer wordmark into the GBA title sprite banner."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageFilter


NATIVE_SIZE = (128, 32)
SHEET_SIZE = (64, 64)
CONTENT_SIZE = (112, 28)

GAME_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = GAME_ROOT / "graphics/title_screen/wayfarer"
DEFAULT_INPUT = DEFAULT_OUTPUT / "wayfarer_wordmark_pixel_fixed.png"


def palette() -> list[int]:
    """Index 0 is transparent; indexes 1 and 2 are exact white and black."""
    colours = [(0, 0, 0), (255, 255, 255), (0, 0, 0)] + [(0, 0, 0)] * 13
    return [channel for colour in colours for channel in colour]


def native_banner(source: Path) -> Image.Image:
    """Scale the pixel-fixed source and reduce it to the sprite's three indices."""
    original = Image.open(source).convert("RGBA")
    if original.getchannel("A").getbbox() is None:
        raise ValueError("wordmark source has no visible pixels")

    # Keep the fixed glyph interiors. Rebuild the outline after the fit so a
    # non-integer height reduction cannot break its one-pixel black edge.
    source_white = Image.new("L", original.size)
    source_white.putdata([
        255 if alpha >= 128 and red + green + blue >= 3 * 192 else 0
        for red, green, blue, alpha in original.get_flattened_data()
    ])
    fitted_white = source_white.resize(CONTENT_SIZE, Image.Resampling.NEAREST)
    fitted_black = fitted_white.filter(ImageFilter.MaxFilter(3))
    pixels = bytearray(NATIVE_SIZE[0] * NATIVE_SIZE[1])
    offset_x = (NATIVE_SIZE[0] - CONTENT_SIZE[0]) // 2
    offset_y = (NATIVE_SIZE[1] - CONTENT_SIZE[1]) // 2
    for y in range(CONTENT_SIZE[1]):
        for x in range(CONTENT_SIZE[0]):
            if fitted_black.getpixel((x, y)):
                pixels[(y + offset_y) * NATIVE_SIZE[0] + x + offset_x] = (
                    1 if fitted_white.getpixel((x, y)) else 2
                )

    image = Image.frombytes("P", NATIVE_SIZE, bytes(pixels))
    image.putpalette(palette())
    image.info["transparency"] = 0
    return image


def pack_sprite(native: Image.Image) -> Image.Image:
    """Pack left and right 64x32 sprite halves into the established 64x64 sheet."""
    if native.mode != "P" or native.size != NATIVE_SIZE:
        raise ValueError("native banner must be a 128x32 indexed image")
    source = native.tobytes()
    packed = bytearray(SHEET_SIZE[0] * SHEET_SIZE[1])
    for y in range(32):
        packed[y * 64:(y + 1) * 64] = source[y * 128:y * 128 + 64]
        packed[(y + 32) * 64:(y + 33) * 64] = source[y * 128 + 64:(y + 1) * 128]
    image = Image.frombytes("P", SHEET_SIZE, bytes(packed))
    image.putpalette(palette())
    image.info["transparency"] = 0
    return image


def unpack_sprite(sheet: Image.Image) -> Image.Image:
    if sheet.mode != "P" or sheet.size != SHEET_SIZE:
        raise ValueError("sprite sheet must be a 64x64 indexed image")
    source = sheet.tobytes()
    unpacked = bytearray(NATIVE_SIZE[0] * NATIVE_SIZE[1])
    for y in range(32):
        unpacked[y * 128:y * 128 + 64] = source[y * 64:(y + 1) * 64]
        unpacked[y * 128 + 64:(y + 1) * 128] = source[(y + 32) * 64:(y + 33) * 64]
    image = Image.frombytes("P", NATIVE_SIZE, bytes(unpacked))
    image.putpalette(palette())
    image.info["transparency"] = 0
    return image


def review_composite(native: Image.Image) -> Image.Image:
    """Render transparency against grey so the black outline is reviewable."""
    background = Image.new("RGB", NATIVE_SIZE, (150, 150, 150))
    visible = Image.frombytes("L", NATIVE_SIZE, bytes(255 if index else 0 for index in native.tobytes()))
    background.paste(native.convert("RGB"), mask=visible)
    return background


def write_jasc_palette(path: Path) -> None:
    rows = ["JASC-PAL", "0100", "16", "0 0 0", "255 255 255", "0 0 0"]
    rows.extend(["0 0 0"] * 13)
    path.write_bytes(("\r\n".join(rows) + "\r\n").encode("ascii"))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT, help="reviewed Pixel Art Fixer PNG")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="directory for wayfarer_version.png and .pal")
    parser.add_argument("--review-output", type=Path, help="directory for unpacked native and 4x review PNGs")
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)

    native = native_banner(args.input)
    sheet = pack_sprite(native)
    if unpack_sprite(sheet).tobytes() != native.tobytes():
        raise AssertionError("banner sprite packing failed to round-trip")
    if set(native.tobytes()) != {0, 1, 2}:
        raise AssertionError("banner must contain transparency, white fill, and black outline only")

    sheet.save(output / "wayfarer_version.png", optimize=False)
    write_jasc_palette(output / "wayfarer_version.pal")
    if args.review_output:
        review = args.review_output.resolve()
        review.mkdir(parents=True, exist_ok=True)
        native.save(review / "wayfarer-banner-native.png", optimize=False)
        review_composite(native).resize((NATIVE_SIZE[0] * 4, NATIVE_SIZE[1] * 4), Image.Resampling.NEAREST).save(review / "wayfarer-banner-4x.png", optimize=False)
    print("Generated Pixel Art Fixer-derived Wayfarer banner with white fill and black outline.")


if __name__ == "__main__":
    main()
