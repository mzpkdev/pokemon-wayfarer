#!/usr/bin/env python3
"""Read-only static terrain probe, not a full movement/script emulator."""
import json
import re
import struct
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[5]
GAME = ROOT / "game"
LAYOUTS = {x["id"]: x for x in json.loads((GAME / "data/layouts/layouts.json").read_text())["layouts"]}
HEADERS = (GAME / "src/data/tilesets/headers.h").read_text()
ATTRS = (GAME / "src/data/tilesets/metatiles.h").read_text()
BEHAVIORS = list(dict.fromkeys(re.findall(r"\bMB_\w+", (GAME / "include/constants/metatile_behaviors.h").read_text())))
DIRECTIONS = [(0, 1), (0, -1), (1, 0), (-1, 0)]


def words(path):
    data = path.read_bytes()
    return struct.unpack("<" + "H" * (len(data) // 2), data)


class Map:
    def __init__(self, name, layout=None):
        self.data = json.loads((GAME / "data/maps" / name / "map.json").read_text())
        self.layout = LAYOUTS[layout or self.data["layout"]]
        self.width, self.height = self.layout["width"], self.layout["height"]
        self.tiles = words(GAME / self.layout["blockdata_filepath"])
        self.attrs = []
        for key in ("primary_tileset", "secondary_tileset"):
            symbol = re.search(r"const struct Tileset " + self.layout[key] + r"\s*=.*?\.metatileAttributes = (\w+)", HEADERS, re.S)[1]
            filename = re.search(symbol + r'\[\] = INCBIN_U16\("([^"]+)', ATTRS)[1]
            self.attrs.append(words(GAME / filename))
        self.hard_objects = {(o["x"], o["y"]) for o in self.data["object_events"] if o["script"] == "EventScript_Whirlpool" or any(t in o["graphics_id"] for t in ("PUSHABLE_BOULDER", "BREAKABLE_ROCK", "WHIRLPOOL"))}

    def tile(self, p):
        x, y = p
        if not (0 <= x < self.width and 0 <= y < self.height):
            return None
        value = self.tiles[y * self.width + x]
        i = value & 1023
        behavior = self.attrs[i >= 640][i - 640 if i >= 640 else i] & 255
        return ((value >> 10) & 3, value >> 12, BEHAVIORS[behavior])

    def reachable(self, start, surf=False, slippery=True):
        # Forced ice motion is modeled; dynamic ice floors and puzzle objects are not.
        # All non-obstacle NPCs are omitted. This is a terrain witness, not exact game state.
        queue = deque([start])
        previous = {start: None}
        while queue:
            p = queue.popleft()
            for dx, dy in DIRECTIONS:
                q = (p[0] + dx, p[1] + dy)
                old = self.tile(p)
                def valid(pos):
                    t = self.tile(pos)
                    if t is None or pos in self.hard_objects:
                        return False
                    water = t[2] in ("MB_POND_WATER", "MB_OCEAN_WATER", "MB_INTERIOR_DEEP_WATER")
                    old_water = old[2] in ("MB_POND_WATER", "MB_OCEAN_WATER", "MB_INTERIOR_DEEP_WATER")
                    if t[0]:
                        return False
                    if not surf and ("WATER" in t[2] or "OCEAN" in t[2]) and "BRIDGE" not in t[2]:
                        return False
                    surf_transition = surf and ((water and old[1] == 3 and t[1] == 1) or (old_water and t[1] == 3))
                    if not surf_transition and old[1] not in (0, 15) and t[1] not in (0, 15, old[1]):
                        return False
                    return True
                if not valid(q):
                    continue
                if slippery:
                    while self.tile(q)[2] == "MB_ICE":
                        nxt = (q[0] + dx, q[1] + dy)
                        if not valid(nxt):
                            break
                        q = nxt
                if q not in previous:
                    previous[q] = p
                    queue.append(q)
        return previous


def inspect(name, start, surf=False, layout=None):
    m = Map(name, layout)
    seen = m.reachable(start, surf)
    banks = []
    for p in seen:
        for dx, dy in DIRECTIONS:
            q = p[0] + dx, p[1] + dy
            t = m.tile(q)
            if m.tile(p)[1] == 3 and t and t[0] == 0 and t[1] == 1 and t[2] in ("MB_POND_WATER", "MB_OCEAN_WATER"):
                banks.append({"stand": p, "water": q})
    return {"map": m.data["id"], "layout": m.layout["id"], "start": start, "surf": surf,
            "terrain_reachable_warps": [{"id": i, **w} for i, w in enumerate(m.data["warp_events"]) if (w["x"], w["y"]) in seen],
            "reachable_boundaries": {d: sorted([p for p in seen if p[axis] == end]) for d, axis, end in [("west", 0, 0), ("east", 0, m.width - 1), ("north", 1, 0), ("south", 1, m.height - 1)]},
            "shore_examples": banks[:5], "reachable_cells": len(seen)}


if __name__ == "__main__":
    cases = [("BlackthornCity_hns", (27, 49)), ("IcePath_1F_hns", (49, 32)),
             ("IcePath_1F_hns", (14, 24)), ("Route44_hns", (68, 13)),
             ("Mahoganytown_hns", (43, 12)), ("Route43_hns", (11, 49)),
             ("Route42_hns", (93, 12)), ("LakeOfRage_hns", (30, 43)),
             ("IcePath_B1F_hns", (9, 20)), ("IcePath_B1F_hns", (10, 32)),
             ("IcePath_B2F_hns", (26, 6)), ("IcePath_B3F_hns", (14, 21)),
             ("IcePath_B4F_hns", (21, 18))]
    results = [inspect(*case) for case in cases]
    den = Map("DragonsDen_Cavern_hns")
    without = den.reachable((31, 3), surf=True)
    den.hard_objects.clear()
    with_move = den.reachable((31, 3), surf=True)
    assert (31, 47) not in without and (31, 47) in with_move
    ice = Map("IcePath_B2F_hns")
    assert (16, 15) in ice.reachable((26, 6))
    ice.hard_objects.clear()
    assert (16, 15) not in ice.reachable((26, 6))
    assert (26, 6) in ice.reachable((16, 15))
    print(json.dumps({"terrain_probes": results, "checks": {"den_shrine_requires_whirlpool": True,
        "ice_return_requires_boulder_state": True}}, indent=2))
