#!/usr/bin/env python3
"""Generate the original indexed assets for the Wayfarer title screen."""

from __future__ import annotations

import argparse
import binascii
import struct
import zlib
from pathlib import Path


PALETTE = [
    (8, 16, 32),      # transparent / deepest navy
    (13, 28, 52),     # night sky
    (22, 48, 76),     # blue sky
    (37, 77, 101),    # pale blue sky
    (198, 87, 89),    # coral dawn
    (238, 142, 95),   # peach dawn
    (244, 190, 102),  # route gold
    (255, 226, 157),  # sunrise cream
    (9, 39, 58),      # deep ocean
    (13, 65, 78),     # ocean teal
    (33, 102, 107),   # wave teal
    (14, 29, 43),     # distant silhouette
    (23, 53, 63),     # island silhouette
    (38, 76, 75),     # lit island edge
    (57, 42, 43),     # warm cliff
    (241, 241, 214),  # compass highlight
]


FONT = {
    "A": ("01110", "10001", "10001", "11111", "10001", "10001", "10001"),
    "E": ("11111", "10000", "10000", "11110", "10000", "10000", "11111"),
    "F": ("11111", "10000", "10000", "11110", "10000", "10000", "10000"),
    "R": ("11110", "10001", "10001", "11110", "10100", "10010", "10001"),
    "W": ("10001", "10001", "10001", "10101", "10101", "11011", "10001"),
    "Y": ("10001", "10001", "01010", "00100", "00100", "00100", "00100"),
}


def png_chunk(kind: bytes, payload: bytes) -> bytes:
    return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", binascii.crc32(kind + payload) & 0xFFFFFFFF)


def write_indexed_png(path: Path, width: int, height: int, pixels: list[int], palette: list[tuple[int, int, int]]) -> None:
    rows = b"".join(b"\0" + bytes(pixels[y * width:(y + 1) * width]) for y in range(height))
    plte = b"".join(bytes(rgb) for rgb in palette)
    data = b"\x89PNG\r\n\x1a\n"
    data += png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 3, 0, 0, 0))
    data += png_chunk(b"PLTE", plte)
    data += png_chunk(b"tRNS", bytes([0] + [255] * (len(palette) - 1)))
    data += png_chunk(b"IDAT", zlib.compress(rows, 9))
    data += png_chunk(b"IEND", b"")
    path.write_bytes(data)


def write_jasc_palette(path: Path, palette: list[tuple[int, int, int]]) -> None:
    lines = ["JASC-PAL", "0100", str(len(palette)), *(f"{r} {g} {b}" for r, g, b in palette)]
    path.write_text("\n".join(lines) + "\n", encoding="ascii")


class Canvas:
    def __init__(self, width: int, height: int, color: int = 0):
        self.width = width
        self.height = height
        self.pixels = [color] * (width * height)

    def set(self, x: int, y: int, color: int) -> None:
        if 0 <= x < self.width and 0 <= y < self.height:
            self.pixels[y * self.width + x] = color

    def rect(self, x0: int, y0: int, x1: int, y1: int, color: int) -> None:
        for y in range(max(0, y0), min(self.height, y1)):
            start = y * self.width + max(0, x0)
            end = y * self.width + min(self.width, x1)
            self.pixels[start:end] = [color] * (end - start)

    def line(self, x0: int, y0: int, x1: int, y1: int, color: int, width: int = 1) -> None:
        dx, sx = abs(x1 - x0), 1 if x0 < x1 else -1
        dy, sy = -abs(y1 - y0), 1 if y0 < y1 else -1
        err = dx + dy
        while True:
            radius = width // 2
            self.rect(x0 - radius, y0 - radius, x0 + radius + 1, y0 + radius + 1, color)
            if x0 == x1 and y0 == y1:
                return
            twice = 2 * err
            if twice >= dy:
                err += dy
                x0 += sx
            if twice <= dx:
                err += dx
                y0 += sy

    def polygon(self, points: list[tuple[int, int]], color: int) -> None:
        min_y = max(0, min(y for _, y in points))
        max_y = min(self.height - 1, max(y for _, y in points))
        for y in range(min_y, max_y + 1):
            intersections = []
            for i, (x1, y1) in enumerate(points):
                x2, y2 = points[(i + 1) % len(points)]
                if (y1 <= y < y2) or (y2 <= y < y1):
                    intersections.append(round(x1 + (y - y1) * (x2 - x1) / (y2 - y1)))
            intersections.sort()
            for i in range(0, len(intersections) - 1, 2):
                self.rect(intersections[i], y, intersections[i + 1] + 1, y + 1, color)

    def circle(self, cx: int, cy: int, radius: int, color: int) -> None:
        x, y, error = radius, 0, 1 - radius
        while x >= y:
            for px, py in ((x, y), (y, x), (-y, x), (-x, y), (-x, -y), (-y, -x), (y, -x), (x, -y)):
                self.set(cx + px, cy + py, color)
            y += 1
            if error < 0:
                error += 2 * y + 1
            else:
                x -= 1
                error += 2 * (y - x) + 1

    def disc(self, cx: int, cy: int, radius: int, color: int) -> None:
        for y in range(cy - radius, cy + radius + 1):
            half_width = int(max(0, radius * radius - (y - cy) * (y - cy)) ** 0.5)
            self.rect(cx - half_width, y, cx + half_width + 1, y + 1, color)


def create_scene() -> Canvas:
    canvas = Canvas(256, 256, 1)

    # Clustered dawn sky: calm behind the logo, textured toward the horizon.
    canvas.rect(0, 0, 256, 27, 1)
    canvas.rect(0, 27, 256, 52, 2)
    canvas.rect(0, 52, 256, 72, 3)
    canvas.rect(0, 72, 256, 88, 4)
    canvas.rect(0, 88, 256, 104, 5)
    for y in range(30, 100, 4):
        color = 3 if y < 70 else 5 if y < 88 else 7
        for x in range((y // 4) % 8, 256, 16):
            canvas.rect(x, y, x + 2, y + 1, color)
    for x, y in ((17, 16), (42, 34), (72, 11), (205, 19), (235, 43), (221, 65)):
        canvas.set(x, y, 7)
        canvas.set(x - 1, y, 6)
        canvas.set(x + 1, y, 6)

    # Windswept clouds add depth without competing with the title.
    canvas.polygon([(0, 62), (18, 60), (26, 55), (40, 58), (54, 57), (68, 62)], 2)
    canvas.line(5, 63, 58, 63, 3)
    canvas.polygon([(181, 48), (194, 45), (202, 39), (213, 43), (226, 42), (244, 49), (256, 50)], 2)
    canvas.line(188, 50, 255, 50, 3)

    # A warm sun doubles as the heart of a weathered compass rose.
    cx, cy = 126, 95
    canvas.disc(cx, cy, 29, 7)
    canvas.disc(cx, cy, 24, 5)
    for y in range(cy - 20, cy + 21, 5):
        canvas.line(cx - 22, y, cx + 22, y, 7)
    canvas.circle(cx, cy, 32, 6)
    canvas.circle(cx, cy, 30, 15)
    for angle_point in ((cx, cy - 36), (cx + 36, cy), (cx, cy + 36), (cx - 36, cy)):
        canvas.line(cx, cy, *angle_point, 6)
    canvas.polygon([(cx, cy - 27), (cx + 3, cy - 3), (cx, cy + 1), (cx - 3, cy - 3)], 6)
    canvas.line(cx, cy - 24, cx, cy - 5, 15)
    canvas.polygon([(cx, cy + 29), (cx + 5, cy + 3), (cx, cy - 2), (cx - 5, cy + 3)], 11)
    canvas.polygon([(cx + 29, cy), (cx + 3, cy + 5), (cx - 2, cy), (cx + 3, cy - 5)], 7)
    canvas.polygon([(cx - 29, cy), (cx - 3, cy + 5), (cx + 2, cy), (cx - 3, cy - 5)], 11)
    canvas.rect(cx - 2, cy - 2, cx + 3, cy + 3, 15)

    # Distinct regional silhouettes: forest coast, island town, and volcanic range.
    canvas.polygon([(0, 106), (13, 98), (26, 83), (38, 99), (50, 91), (66, 107)], 11)
    canvas.polygon([(28, 107), (52, 99), (69, 86), (82, 99), (98, 104), (108, 108)], 12)
    for x, height in ((7, 7), (17, 11), (47, 8), (57, 12), (89, 7)):
        canvas.polygon([(x - 3, 106), (x, 106 - height), (x + 3, 106)], 13)
    canvas.polygon([(151, 108), (166, 100), (178, 104), (190, 96), (202, 107)], 12)
    canvas.rect(175, 94, 181, 104, 11)
    canvas.rect(173, 97, 183, 100, 11)
    canvas.set(178, 96, 7)
    canvas.polygon([(196, 108), (211, 93), (220, 77), (228, 91), (239, 98), (256, 87), (256, 109)], 11)
    canvas.line(217, 80, 222, 80, 4)
    canvas.line(219, 81, 224, 81, 5)
    canvas.line(0, 107, 106, 107, 13)
    canvas.line(151, 108, 255, 108, 13)

    # Layered ocean and broken highlights suggest distance and motion.
    canvas.rect(0, 109, 256, 130, 10)
    canvas.rect(0, 130, 256, 160, 9)
    canvas.rect(0, 160, 256, 256, 8)
    for y, offset, color in ((113, 2, 3), (119, 9, 13), (128, 1, 3), (137, 7, 10), (147, 0, 10), (156, 11, 9)):
        for x in range(offset, 256, 24):
            length = 8 if (x // 8 + y) % 2 else 12
            canvas.line(x, y, x + length, y, color)
            if length == 12:
                canvas.line(x + 3, y + 1, x + 8, y + 1, color)
    canvas.polygon([(114, 109), (121, 109), (116, 160), (102, 160)], 5)
    canvas.polygon([(124, 109), (130, 109), (138, 160), (119, 160)], 6)
    canvas.polygon([(131, 109), (136, 109), (153, 160), (139, 160)], 5)

    # A curved chain of waypoints ties the regions together.
    route = [(14, 138), (48, 128), (78, 130), (107, 121), (138, 124), (169, 116), (201, 120), (231, 112)]
    for a, b in zip(route, route[1:]):
        x0, y0 = a
        x1, y1 = b
        steps = max(abs(x1 - x0), abs(y1 - y0))
        for step in range(0, steps + 1, 7):
            x = round(x0 + (x1 - x0) * step / steps)
            y = round(y0 + (y1 - y0) * step / steps)
            canvas.rect(x - 1, y - 1, x + 2, y + 2, 7)
    for x, y in route[1:-1]:
        canvas.rect(x - 3, y - 3, x + 4, y + 4, 6)
        canvas.rect(x - 2, y - 2, x + 3, y + 3, 7)
        canvas.set(x, y, 15)

    # Foreground cliff and a more characterful traveler, clear of PRESS START.
    canvas.polygon([(184, 160), (188, 148), (195, 144), (199, 134), (209, 130), (218, 136), (226, 132), (237, 142), (256, 138), (256, 160)], 14)
    canvas.polygon([(209, 160), (218, 143), (231, 139), (244, 147), (256, 145), (256, 160)], 11)
    canvas.line(188, 151, 204, 146, 5)
    canvas.line(229, 145, 248, 151, 4)
    canvas.rect(218, 148, 222, 151, 6)
    canvas.line(207, 160, 214, 141, 5)
    canvas.line(228, 160, 224, 143, 4)

    canvas.polygon([(204, 109), (210, 104), (218, 106), (222, 112), (218, 119), (207, 119), (202, 114)], 11)
    canvas.line(204, 111, 209, 105, 7)
    canvas.rect(206, 117, 220, 133, 1)
    canvas.rect(210, 120, 218, 128, 4)
    canvas.polygon([(206, 129), (220, 129), (228, 145), (200, 145)], 2)
    canvas.line(204, 123, 201, 138, 5)
    canvas.rect(200, 121, 206, 132, 14)
    canvas.rect(201, 122, 205, 130, 4)
    canvas.polygon([(220, 124), (237, 132), (229, 137), (216, 131)], 3)
    canvas.line(205, 123, 197, 139, 1, 3)
    canvas.line(220, 123, 229, 137, 1, 3)
    canvas.rect(199, 124, 204, 136, 6)
    canvas.rect(203, 141, 210, 158, 1)
    canvas.rect(217, 141, 223, 158, 1)
    canvas.rect(199, 136, 205, 140, 7)

    # Tiny seabirds keep the upper horizon alive without introducing a mascot.
    canvas.line(74, 77, 78, 75, 11)
    canvas.line(78, 75, 82, 77, 11)
    canvas.line(164, 69, 168, 67, 11)
    canvas.line(168, 67, 172, 69, 11)

    return canvas


def flip_h(tile: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(tile[y * 8 + (7 - x)] for y in range(8) for x in range(8))


def flip_v(tile: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(tile[(7 - y) * 8 + x] for y in range(8) for x in range(8))


def tile_scene(canvas: Canvas) -> tuple[list[int], bytes, int]:
    unique: list[tuple[int, ...]] = []
    lookup: dict[tuple[int, ...], tuple[int, int]] = {}
    near_variants: list[tuple[tuple[int, ...], int, int]] = []
    tilemap = bytearray()
    for tile_y in range(32):
        for tile_x in range(32):
            tile = tuple(canvas.pixels[(tile_y * 8 + y) * 256 + tile_x * 8 + x] for y in range(8) for x in range(8))
            match = lookup.get(tile)
            if match is None:
                # Merge tiles that differ by only one pixel. This preserves the
                # scene visually while keeping the richer composition inside a
                # single 256-tile GBA character block.
                for variant, variant_id, variant_flags in near_variants:
                    if sum(a != b for a, b in zip(tile, variant)) <= 1:
                        match = (variant_id, variant_flags)
                        lookup[tile] = match
                        break
            if match is None:
                tile_id = len(unique)
                if tile_id >= 256:
                    raise RuntimeError("scene exceeds the 256-tile GBA character block")
                unique.append(tile)
                variants = ((tile, 0), (flip_h(tile), 1 << 10), (flip_v(tile), 1 << 11), (flip_v(flip_h(tile)), 3 << 10))
                for variant, flags in variants:
                    lookup.setdefault(variant, (tile_id, flags))
                    near_variants.append((variant, tile_id, flags))
                match = (tile_id, 0)
            tile_id, flags = match
            tilemap.extend(struct.pack("<H", tile_id | flags | (14 << 12)))

    sheet = [0] * (128 * 128)
    for tile_id, tile in enumerate(unique):
        sx = (tile_id % 16) * 8
        sy = (tile_id // 16) * 8
        for y in range(8):
            for x in range(8):
                sheet[(sy + y) * 128 + sx + x] = tile[y * 8 + x]
    return sheet, bytes(tilemap), len(unique)


def create_banner() -> Canvas:
    banner = Canvas(128, 32, 0)
    text = "WAYFARER"
    scale = 2
    glyph_width = 5 * scale
    spacing = 2
    width = len(text) * glyph_width + (len(text) - 1) * spacing
    start_x = (128 - width) // 2
    start_y = 7
    mask: set[tuple[int, int]] = set()
    for letter_index, letter in enumerate(text):
        x0 = start_x + letter_index * (glyph_width + spacing)
        for gy, row in enumerate(FONT[letter]):
            for gx, pixel in enumerate(row):
                if pixel == "1":
                    for dy in range(scale):
                        for dx in range(scale):
                            mask.add((x0 + gx * scale + dx, start_y + gy * scale + dy))
    for x, y in mask:
        banner.set(x + 2, y + 2, 6)
    for x, y in mask:
        for ox, oy in ((-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (1, 1)):
            if (x + ox, y + oy) not in mask:
                banner.set(x + ox, y + oy, 1)
    for x, y in mask:
        banner.set(x, y, 15 if y < start_y + 7 else 7)
    banner.line(start_x - 3, 25, start_x + width + 2, 25, 4)
    for x in range(start_x, start_x + width, 8):
        banner.set(x, 27, 6)
    return banner


def pack_banner(banner: Canvas) -> list[int]:
    packed = [0] * (64 * 64)
    for y in range(32):
        for x in range(64):
            packed[y * 64 + x] = banner.pixels[y * 128 + x]
            packed[(y + 32) * 64 + x] = banner.pixels[y * 128 + x + 64]
    return packed


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)

    scene = create_scene()
    sheet, tilemap, tile_count = tile_scene(scene)
    write_indexed_png(args.output / "scene.png", 128, 128, sheet, PALETTE)
    (args.output / "scene.bin").write_bytes(tilemap)
    write_jasc_palette(args.output / "scene.pal", PALETTE)

    banner = create_banner()
    write_indexed_png(args.output / "wayfarer_version.png", 64, 64, pack_banner(banner), PALETTE)
    write_jasc_palette(args.output / "wayfarer_version.pal", PALETTE)
    print(f"Generated Wayfarer title assets with {tile_count} unique scene tiles.")


if __name__ == "__main__":
    main()
