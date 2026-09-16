#!/usr/bin/env python3
"""Fit the approved Wayfarer wordmark into the GBA title sprite banner."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


NATIVE_SIZE = (128, 32)
SHEET_SIZE = (64, 64)
CONTENT_SIZE = (112, 28)
GRAY_LEVELS = tuple(round(index * 255 / 7) for index in range(8))

GAME_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = GAME_ROOT / "graphics/title_screen/wayfarer"
DEFAULT_INPUT = GAME_ROOT.parent / ".github/assets/wayfarer-simple-thick.png"


def palette() -> list[int]:
    """Index 0 is transparent; indexes 1..8 are black through white."""
    colours = [(0, 0, 0)] + [(gray, gray, gray) for gray in GRAY_LEVELS] + [(0, 0, 0)] * 7
    return [channel for colour in colours for channel in colour]


def native_banner(source: Path) -> Image.Image:
    """Downsample the original directly to the sprite's eight grayscale levels."""
    original = Image.open(source).convert("RGBA")
    if original.getchannel("A").getbbox() is None:
        raise ValueError("wordmark source has no visible pixels")

    fitted = original.resize(CONTENT_SIZE, Image.Resampling.LANCZOS)
    pixels = bytearray(NATIVE_SIZE[0] * NATIVE_SIZE[1])
    offset_x = (NATIVE_SIZE[0] - CONTENT_SIZE[0]) // 2
    offset_y = (NATIVE_SIZE[1] - CONTENT_SIZE[1]) // 2
    for y in range(CONTENT_SIZE[1]):
        for x in range(CONTENT_SIZE[0]):
            red, green, blue, alpha = fitted.getpixel((x, y))
            if alpha >= 128:
                gray_index = round((red + green + blue) / (3 * 255) * 7)
                pixels[(y + offset_y) * NATIVE_SIZE[0] + x + offset_x] = gray_index + 1

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
    rows = ["JASC-PAL", "0100", "16", "0 0 0"]
    rows.extend(f"{gray} {gray} {gray}" for gray in GRAY_LEVELS)
    rows.extend(["0 0 0"] * 7)
    path.write_bytes(("\r\n".join(rows) + "\r\n").encode("ascii"))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT, help="approved full-resolution wordmark PNG")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="directory for wayfarer_version.png and .pal")
    parser.add_argument("--review-output", type=Path, help="directory for unpacked native and 4x review PNGs")
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)

    native = native_banner(args.input)
    sheet = pack_sprite(native)
    if unpack_sprite(sheet).tobytes() != native.tobytes():
        raise AssertionError("banner sprite packing failed to round-trip")
    used = set(native.tobytes())
    if not {0, 1, 8} <= used or used - set(range(9)):
        raise AssertionError("banner must contain transparency and black-to-white grayscale only")

    sheet.save(output / "wayfarer_version.png", optimize=False)
    write_jasc_palette(output / "wayfarer_version.pal")
    if args.review_output:
        review = args.review_output.resolve()
        review.mkdir(parents=True, exist_ok=True)
        native.save(review / "wayfarer-banner-native.png", optimize=False)
        review_composite(native).resize((NATIVE_SIZE[0] * 4, NATIVE_SIZE[1] * 4), Image.Resampling.NEAREST).save(review / "wayfarer-banner-4x.png", optimize=False)
    print("Generated direct-downsample Wayfarer banner with eight grayscale levels.")


if __name__ == "__main__":
    main()
