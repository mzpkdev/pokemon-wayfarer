#!/usr/bin/env python3
"""Create the 64x64, 4bpp Wayfarer intro Poké Ball sprite source.

Install Pillow with ``pip install -r game/tools/requirements-wayfarer-title.txt``.
The approved 48px sprite is copied unchanged into the centred portion of the
64px GBA canvas. Palette index zero remains GBA transparency.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


TARGET_SIZE = 64
SOURCE_SIZE = 48
TRANSPARENCY_THRESHOLD = 48
ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = ROOT.parent / ".github/assets/wayfarer-intro-pokeball-v2-dirt-48-fixed.png"
DEFAULT_OUTPUT = ROOT / "graphics/intro/scene_1/wayfarer_pokeball.png"


def opaque_samples(image: Image.Image) -> Image.Image:
    """Return one RGB sample for each source pixel that will remain visible."""
    rgba = image.convert("RGBA")
    rgb = rgba.convert("RGB")
    alpha = rgba.getchannel("A")
    samples = [pixel for pixel, opacity in zip(rgb.get_flattened_data(), alpha.get_flattened_data()) if opacity >= TRANSPARENCY_THRESHOLD]
    if not samples:
        raise ValueError("the resized Poké Ball has no opaque pixels")
    sample_image = Image.new("RGB", (len(samples), 1))
    sample_image.putdata(samples)
    return sample_image


def nearest_palette_index(pixel: tuple[int, int, int], palette: list[tuple[int, int, int]]) -> int:
    return min(range(len(palette)), key=lambda index: sum((pixel[channel] - palette[index][channel]) ** 2 for channel in range(3)))


def convert(source: Path, output: Path) -> None:
    with Image.open(source) as source_file:
        artwork = source_file.convert("RGBA")
        if artwork.size != (SOURCE_SIZE, SOURCE_SIZE):
            raise ValueError(f"Poké Ball source must be {SOURCE_SIZE}x{SOURCE_SIZE}px")
        resized = Image.new("RGBA", (TARGET_SIZE, TARGET_SIZE))
        resized.alpha_composite(artwork, ((TARGET_SIZE - SOURCE_SIZE) // 2, (TARGET_SIZE - SOURCE_SIZE) // 2))

    palette_image = opaque_samples(resized).quantize(colors=15, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    raw_palette = palette_image.getpalette()
    assert raw_palette is not None
    palette = [tuple(raw_palette[index:index + 3]) for index in range(0, 15 * 3, 3)]

    indexed_pixels: list[int] = []
    for pixel in resized.get_flattened_data():
        red, green, blue, alpha = pixel
        indexed_pixels.append(0 if alpha < TRANSPARENCY_THRESHOLD else nearest_palette_index((red, green, blue), palette) + 1)

    indexed = Image.new("P", (TARGET_SIZE, TARGET_SIZE))
    indexed.putpalette([0, 0, 0] + [channel for colour in palette for channel in colour] + [0] * (256 * 3 - 16 * 3))
    indexed.putdata(indexed_pixels)
    output.parent.mkdir(parents=True, exist_ok=True)
    indexed.save(output, transparency=0, bits=4, optimize=False)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()
    convert(args.source.resolve(), args.output.resolve())
    print(f"wrote {args.output.resolve()} as a {TARGET_SIZE}x{TARGET_SIZE} indexed PNG")


if __name__ == "__main__":
    main()
