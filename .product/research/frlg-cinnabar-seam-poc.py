"""Read-only coastal seam probe for the checked-in Cinnabar/Route 20/21 layouts.

This decodes source metatile collision, elevation, and behavior. It is not a
Porymap render or a substitute for engine movement and emulator traversal.
"""

import json
import re
import struct
from collections import deque
from pathlib import Path


GAME = Path(__file__).resolve().parents[2] / "game"
LAYOUTS = {
    row["id"]: row
    for row in json.loads((GAME / "data/layouts/layouts.json").read_text())["layouts"]
}
HEADERS = (GAME / "src/data/tilesets/headers.h").read_text()
ATTRS = (GAME / "src/data/tilesets/metatiles.h").read_text()
OCEAN = 21  # MB_OCEAN_WATER in constants/metatile_behaviors.h


def load(name):
    map_data = json.loads((GAME / "data/maps" / name / "map.json").read_text())
    layout = LAYOUTS[map_data["layout"]]
    frlg = layout["layout_version"] == "frlg"
    tables = []
    for kind in ("primary_tileset", "secondary_tileset"):
        tileset = layout[kind]
        body = re.search(
            rf"const struct Tileset {tileset}\s*=\s*\{{(.*?)\}};", HEADERS, re.S
        ).group(1)
        symbol = re.search(r"\.metatileAttributes = (\w+)", body).group(1)
        path = re.search(rf"\b{symbol}\[\] = INCBIN_U16\(\"([^\"]+)\"\)", ATTRS).group(1)
        raw = (GAME / path).read_bytes()
        # The source declares both files U16, but the FRLG runtime reads paired
        # halfwords as U32 attributes (fieldmap.c:GetAttributeByMetatileIdAndMapLayoutFrlg).
        fmt = "I" if frlg else "H"
        tables.append(struct.unpack("<" + fmt * (len(raw) // struct.calcsize(fmt)), raw))

    raw = (GAME / layout["blockdata_filepath"]).read_bytes()
    words = struct.unpack("<" + "H" * (len(raw) // 2), raw)
    width, height = layout["width"], layout["height"]
    cells = {}
    for y in range(height):
        for x in range(width):
            word = words[y * width + x]
            metatile = word & 1023
            attribute = tables[metatile >= 640][metatile - 640 if metatile >= 640 else metatile]
            cells[x, y] = {
                "collision": (word >> 10) & 3,
                "elevation": word >> 12,
                "behavior": attribute & (511 if frlg else 255),
            }
    return layout, cells


def edge(layout, cells, side):
    width, height = layout["width"], layout["height"]
    points = {
        "north": [(x, 0) for x in range(width)],
        "south": [(x, height - 1) for x in range(width)],
        "west": [(0, y) for y in range(height)],
        "east": [(width - 1, y) for y in range(height)],
    }[side]
    return "".join(
        "#" if cells[point]["collision"] else "~" if cells[point]["behavior"] == OCEAN else "."
        for point in points
    )


def flood(cells, starts, predicate):
    seen = {point for point in starts if point in cells and predicate(cells[point])}
    queue = deque(seen)
    while queue:
        x, y = queue.popleft()
        for point in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if point in cells and point not in seen and predicate(cells[point]):
                seen.add(point)
                queue.append(point)
    return seen


def main():
    town, tc = load("CinnabarIsland_Frlg")
    route20, r20c = load("Route20_hns")
    route21, r21c = load("Route21_hns")
    for name, layout, cells, side in (
        ("FRLG Cinnabar", town, tc, "east"),
        ("HNS Route 20", route20, r20c, "west"),
        ("FRLG Cinnabar", town, tc, "north"),
        ("HNS Route 21", route21, r21c, "south"),
    ):
        print(f"{name} {side}: {edge(layout, cells, side)}")

    open_water = lambda cell: not cell["collision"] and cell["behavior"] == OCEAN
    dry = lambda cell: not cell["collision"] and cell["behavior"] != OCEAN
    north = [(x, 0) for x in range(town["width"]) if open_water(tc[x, 0]) and open_water(r21c[x, route21["height"] - 1])]
    east = [(town["width"] - 1, y) for y in range(town["height"]) if open_water(tc[town["width"] - 1, y]) and open_water(r20c[0, y])]
    coastal = flood(tc, north + east, open_water)
    banks = {point for x, y in coastal for point in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)) if point in tc and dry(tc[point])}
    town_dry = flood(tc, [(20, 5)], dry)  # Authored Gym-door approach.
    route21_water = flood(r21c, [(x, route21["height"] - 1) for x, _ in north], open_water)
    route20_water = flood(r20c, [(0, y) for _, y in east], open_water)
    print(f"Zero-offset north shared open-water lanes: {[x for x, _ in north]}")
    print(f"Zero-offset east shared open-water lanes: {[y for _, y in east]}")
    print(f"Route 21 shared-lane water reaches north edge: {sorted(x for x in range(route21['width']) if (x, 0) in route21_water)}")
    print(f"Route 20 shared-lane water reaches east edge: {sorted(y for y in range(route20['height']) if (route20['width'] - 1, y) in route20_water)}")
    print(f"Town coastal water cells: {len(coastal)}; adjacent dry banks: {sorted(banks)}")
    print(f"Banks in Gym-approach dry component: {sorted(banks & town_dry)}")


if __name__ == "__main__":
    main()
