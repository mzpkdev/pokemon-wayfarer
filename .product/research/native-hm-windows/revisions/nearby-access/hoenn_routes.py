"""Read-only Emerald layout decoder for manual nearby-access review.

This is a tile inspection aid, not a full movement or script interpreter.
"""
import json
import re
import struct
import sys
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[5]
GAME = ROOT / "game"
LAYOUTS = {x["id"]: x for x in json.loads((GAME / "data/layouts/layouts.json").read_text())["layouts"]}


def decode(name):
    events = json.loads((GAME / f"data/maps/{name}/map.json").read_text())
    layout = LAYOUTS[events["layout"]]
    assert layout["layout_version"] == "emerald"
    def attrs(symbol, kind):
        stem = re.sub(r"(?<!^)(?=[A-Z])", "_", symbol.removeprefix("gTileset_")).lower()
        data = (GAME / f"data/tilesets/{kind}/{stem}/metatile_attributes.bin").read_bytes()
        return struct.unpack(f"<{len(data)//2}H", data)
    primary = attrs(layout["primary_tileset"], "primary")
    secondary = attrs(layout["secondary_tileset"], "secondary")
    data = (GAME / layout["blockdata_filepath"]).read_bytes()
    tiles = []
    for value in struct.unpack(f"<{len(data)//2}H", data):
        mid = value & 1023
        behavior = (primary[mid] if mid < 512 else secondary[mid - 512]) & 255
        tiles.append({"id": mid, "behavior": behavior, "collision": value >> 10 & 3, "elevation": value >> 12})
    return events, layout, tiles


def display(name):
    events, layout, tiles = decode(name)
    w, h = layout["width"], layout["height"]
    objects = {(o["x"], o["y"]): o for o in events["object_events"]}
    print(name, w, h, "# collision; ~ water; g grass; L ledge; o object; . ordinary walk tile")
    for y in range(h):
        line = ""
        for x in range(w):
            t = tiles[y*w+x]
            b = t["behavior"]
            c = "#" if t["collision"] else "~" if b in (16,17,18,19,20,21,24,25,26,40,42,43,44,45,80,81,82,83) else "g" if b in (2,3,9,36) else "L" if 56 <= b <= 63 else "."
            if (x,y) in objects:
                c = "o"
            line += c
        print(f"{y:3} {line}")
    print("warps", events["warp_events"])
    print("coords", events["coord_events"])


def walk_component(name, start, *, ignore_objects=(), ledges=False, targets=()):
    """Conservative same-map, bidirectional land component; all objects blocked.

    No ledges, bikes, doors, currents, Surf, or script mutation edges. Ordinary
    transition elevation zero is allowed. Multi-level bridges are excluded.
    """
    events, layout, tiles = decode(name)
    w, h = layout["width"], layout["height"]
    occupied = {(o["x"], o["y"]) for o in events["object_events"] if (o["x"],o["y"]) not in ignore_objects}
    allowed = {0, 2, 3, 7, 9, 10, 12, 22, 23, 33, 116, 117, 118, 119}
    def tile(p):
        return tiles[p[1]*w+p[0]]
    parents = {start: None}
    queue = deque([start])
    while queue:
        p = queue.popleft()
        for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
            q = p[0]+dx,p[1]+dy
            if not (0 <= q[0] < w and 0 <= q[1] < h) or q in parents or q in occupied:
                continue
            a,b=tile(p),tile(q)
            jumps = {56:{(1,0)},57:{(-1,0)},58:{(0,-1)},59:{(0,1)},60:{(1,0),(0,-1)},61:{(-1,0),(0,-1)},62:{(1,0),(0,1)},63:{(-1,0),(0,1)}}
            if ledges and (dx,dy) in jumps.get(b["behavior"],set()):
                q = q[0]+dx,q[1]+dy
                if not (0 <= q[0] < w and 0 <= q[1] < h) or q in parents or q in occupied:
                    continue
                b=tile(q)
                if not b["collision"] and b["behavior"] in allowed:
                    parents[q]=p
                    queue.append(q)
                continue
            if b["collision"] or b["behavior"] not in allowed or b["elevation"] == 15:
                continue
            if a["elevation"] != b["elevation"] and a["elevation"] and b["elevation"]:
                continue
            parents[q]=p
            queue.append(q)
    grass = sorted(p for p in parents if tile(p)["behavior"] in (2,3,9))
    banks = []
    for p in parents:
        if tile(p)["elevation"] != 3:
            continue
        for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
            q=p[0]+dx,p[1]+dy
            if not (0 <= q[0]<w and 0<=q[1]<h) or q in occupied:
                continue
            t=tile(q)
            if t["collision"] == 0 and t["elevation"] == 1 and t["behavior"] in (16,17,18,20,21,80,81,82,83):
                banks.append((p,q))
    def path(end):
        result=[]
        while end is not None:
            result.append(end)
            end=parents[end]
        return result[::-1]
    return {"start":start,"reachable_tiles":len(parents),"grass_count":len(grass),"grass_example":grass[:1],"bank_count":len(banks),"bank_example":banks[:1],"grass_path":path(grass[0]) if grass else [],"bank_path":path(banks[0][0]) if banks else [],"target_paths":{str(p):path(p) if p in parents else [] for p in targets},"edges": {"left": sorted(p for p in parents if p[0]==0),"right":sorted(p for p in parents if p[0]==w-1),"up":sorted(p for p in parents if p[1]==0),"down": sorted(p for p in parents if p[1]==h-1)}}


def report():
    cases = [
        ("lilycove", "LilycoveCity", (24,15), (), ((0,16),)),
        ("lilycove_bank_return", "LilycoveCity", (69,22), (), ((24,15),)),
        ("route121_east", "Route121", (79,6), (), ((0,6),)),
        ("route121_cleared", "Route121", (79,6), ((30,7),(31,7),(30,8)), ((0,6),)),
        ("route120_south", "Route120", (39,86), (), ()),
        ("route118_west", "Route118", (0,10), (), ((79,10),)),
        ("mauville_transit", "MauvilleCity", (39,10), (), ((0,8),)),
        ("route117_east", "Route117", (59,8), (), ()),
        ("route118_east", "Route118", (79,10), (), ((0,10), (57,0))),
        ("route119_south", "Route119", (17,139), (), ()),
        ("route123_west", "Route123", (0,10), (), ()),
        ("mossdeep", "MossdeepCity", (28,17), (), ()),
        ("pacifidlog", "PacifidlogTown", (8,16), (), ()),
    ]
    result={"method":"Conservative collision/elevation/behavior graph plus cardinal ledge jumps; every object position blocked unless explicitly excluded. Manually reviewed scripts, connections, and border destinations separately. Not a complete engine interpreter.", "cases": {}}
    for key,name,start,ignore,targets in cases:
        result["cases"][key]={"map":decode(name)[0]["id"],"ignored_object_positions":ignore,**walk_component(name,start,ignore_objects=ignore,ledges=True,targets=targets)}
    assert result["cases"]["lilycove_bank_return"]["target_paths"]["(24, 15)"]
    assert not result["cases"]["route121_east"]["target_paths"]["(0, 6)"]
    assert result["cases"]["route121_cleared"]["target_paths"]["(0, 6)"]
    assert not result["cases"]["route118_west"]["grass_count"]
    assert result["cases"]["route118_east"]["grass_count"]
    assert not result["cases"]["route123_west"]["grass_count"]
    for key in ("lilycove", "route121_east", "route118_west", "route118_east", "route117_east", "mossdeep", "pacifidlog"):
        assert result["cases"][key]["bank_count"]
    path=Path(__file__).with_suffix(".json")
    path.write_text(json.dumps(result,indent=2)+"\n")
    print(path)


if __name__ == "__main__":
    if sys.argv[1:] == ["--report"]:
        report()
    else:
        for name in sys.argv[1:]:
            display(name)
