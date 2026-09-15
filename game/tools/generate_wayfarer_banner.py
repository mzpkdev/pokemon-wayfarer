#!/usr/bin/env python3
"""Generate the Wayfarer title sprite banner with GBA-style outlined lettering."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


NATIVE_SIZE = (128, 32)
SHEET_SIZE = (64, 64)
SCALE = 2
OUTLINE_RADIUS = 2
TEXT = "WAYFARER"
LETTER_DROPS = (2, 1, 0, 0, 0, 0, 1, 2)

GAME_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = GAME_ROOT / "graphics/title_screen/wayfarer"

# Six-by-eight, filled display glyphs. Scaling each source pixel 2x keeps the
# letters deliberately chunky at the title screen's native resolution.
GLYPHS = {
    "W": (
        "11000011",
        "11000011",
        "11000011",
        "11011011",
        "11011011",
        "11100111",
        "11100111",
        "01100110",
    ),
    "A": (
        "011110",
        "110011",
        "110011",
        "110011",
        "111111",
        "110011",
        "110011",
        "110011",
    ),
    "Y": (
        "110011",
        "110011",
        "011110",
        "001100",
        "001100",
        "001100",
        "001100",
        "001100",
    ),
    "F": (
        "111111",
        "110000",
        "110000",
        "111110",
        "110000",
        "110000",
        "110000",
        "110000",
    ),
    "R": (
        "111110",
        "110011",
        "110011",
        "111110",
        "111100",
        "110110",
        "110011",
        "110011",
    ),
    "E": (
        "111111",
        "110000",
        "110000",
        "111110",
        "110000",
        "110000",
        "110000",
        "111111",
    ),
}


def palette() -> list[int]:
    """Index 0 is transparent; indexes 1 and 2 are exact white and black."""
    colours = [(0, 0, 0), (255, 255, 255), (0, 0, 0)] + [(0, 0, 0)] * 13
    return [channel for colour in colours for channel in colour]


def native_banner() -> Image.Image:
    width, height = NATIVE_SIZE
    pixels = bytearray(width * height)
    glyph_widths = [len(GLYPHS[letter][0]) * SCALE for letter in TEXT]
    glyph_height = len(GLYPHS["W"]) * SCALE
    spacing = 2
    text_width = sum(glyph_widths) + (len(TEXT) - 1) * spacing
    x0 = (width - text_width) // 2
    y0 = (height - glyph_height) // 2
    fill: set[tuple[int, int]] = set()

    cursor_x = x0
    for letter_index, letter in enumerate(TEXT):
        letter_y = y0 + LETTER_DROPS[letter_index]
        for row, source_row in enumerate(GLYPHS[letter]):
            for column, bit in enumerate(source_row):
                if bit != "1":
                    continue
                for y in range(letter_y + row * SCALE, letter_y + (row + 1) * SCALE):
                    for x in range(cursor_x + column * SCALE, cursor_x + (column + 1) * SCALE):
                        fill.add((x, y))
        cursor_x += glyph_widths[letter_index] + spacing

    for x, y in fill:
        for dy in range(-OUTLINE_RADIUS, OUTLINE_RADIUS + 1):
            for dx in range(-OUTLINE_RADIUS, OUTLINE_RADIUS + 1):
                px, py = x + dx, y + dy
                if 0 <= px < width and 0 <= py < height:
                    pixels[py * width + px] = 2
    for x, y in fill:
        pixels[y * width + x] = 1

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
    """Render transparency against neutral grey so the black outline is reviewable."""
    background = Image.new("RGB", NATIVE_SIZE, (150, 150, 150))
    visible = Image.frombytes("L", NATIVE_SIZE, bytes(255 if index else 0 for index in native.tobytes()))
    background.paste(native.convert("RGB"), mask=visible)
    return background


def write_jasc_palette(path: Path) -> None:
    rows = ["JASC-PAL", "0100", "16", "0 0 0", "255 255 255", "0 0 0"]
    rows.extend(["0 0 0"] * 13)
    path.write_text("\n".join(rows) + "\n", encoding="ascii")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="directory for wayfarer_version.png and .pal")
    parser.add_argument("--review-output", type=Path, help="directory for unpacked native and 4x review PNGs")
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)

    native = native_banner()
    sheet = pack_sprite(native)
    if unpack_sprite(sheet).tobytes() != native.tobytes():
        raise AssertionError("banner sprite packing failed to round-trip")
    if set(native.tobytes()) - {0, 1, 2}:
        raise AssertionError("banner uses an index outside its first three palette entries")
    if not 1 in native.tobytes() or not 2 in native.tobytes():
        raise AssertionError("banner must contain white fill and black outline")

    sheet.save(output / "wayfarer_version.png", optimize=False)
    write_jasc_palette(output / "wayfarer_version.pal")
    if args.review_output:
        review = args.review_output.resolve()
        review.mkdir(parents=True, exist_ok=True)
        native.save(review / "wayfarer-banner-native.png", optimize=False)
        review_composite(native).resize((NATIVE_SIZE[0] * 4, NATIVE_SIZE[1] * 4), Image.Resampling.NEAREST).save(review / "wayfarer-banner-4x.png", optimize=False)
    print("Generated Wayfarer banner with white fill, black outline, and a 64x64 packed sprite sheet.")


if __name__ == "__main__":
    main()
