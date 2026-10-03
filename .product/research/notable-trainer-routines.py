#!/usr/bin/env python3
"""Check the routines in notable-trainer-routines.md, read-only.

Reads the routine file next to this script, the trainer catalog
(devtools/ui/src/modules/trainer-balance/catalog.json), the named spots
(notable-named-spots.md), and the spots inventory's per-map output
(notable-spots-inventory/per-map.csv, from inventory.py), and checks:

  - every placeable trainer (all catalog trainers but Tate & Liza) has a
    home base, a cycle of 3-4 known activities, and at most 3 favourites;
    the twelve face-only trainers, not simulated in v0, are checked too, so
    their routines are ready for later;
  - every favourite resolves to a named spot, or to a detected spot of
    that kind on that map, in Wayfarer's scope;
  - every favourite's activity is one of that spot's activities, and the
    activity is a step of the trainer's cycle;
  - aloof trainers' favourites are not public spots; non-travellers'
    favourites are in their home region (the home base's region or the
    catalog's home region);
  - every cycle step has a candidate: a favourite, or a named or detected
    spot of a matching kind within the placeholder radius (3 map hops, 8
    for a traveller) of the home base, after the aloof filter.

Hops are approximated on the inventory's map graph: maps in scope, linked
by their map connections and by warps whose destination warps back. That
graph has no walking filters (a
Surf-only connection counts as a hop) and no transit edges, and counts map
hops, not walker-graph nodes, so it is an approximation of the spec's
radius. It prints a report and exits 1 on any failure. Python 3 standard
library only; run inventory.py first if per-map.csv is missing.

    python3 .product/research/notable-trainer-routines.py
"""

import csv
import json
import re
import sys
from collections import Counter, defaultdict, deque
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
INVENTORY_DIR = HERE / "notable-spots-inventory"
ROUTINES_DOC = HERE / "notable-trainer-routines.md"
CATALOG = REPO / "devtools/ui/src/modules/trainer-balance/catalog.json"

sys.dont_write_bytecode = True  # read-only: leave no __pycache__ behind
sys.path.insert(0, str(INVENTORY_DIR))
import inventory as inv  # noqa: E402  (map loading, scope, named spots)

ACTIVITIES = ("train", "care", "study", "home", "relax", "shop", "gamble",
              "sightsee", "fish", "visit", "lie low")
RADIUS, TRAVELLER_RADIUS = 3, 8  # spec placeholders, in hops
SKIPPED = {"tate-liza"}
# Face-only overworld sprites (3 frames, no walk cycle): not simulated in
# v0, per the world simulation spec's walking-sprite limitation. Their
# routines stay authored and checked for later, but they occupy no spot,
# so they are left out of the shared-favourite list.
FACE_ONLY = {"roxanne", "brawly", "wattson", "flannery", "winona", "sidney",
             "phoebe", "glacia", "drake", "agatha", "bruno", "koga"}

# Detected kinds: the name used in the routine file, the per-map.csv test,
# the activities that land on it, and whether it is public.
KINDS = {
    "Pokémon Center": (lambda r: int(r["center_counter_tiles"]) > 0,
                       {"care"}, True),
    "store": (lambda r: r["mart_floor"] == "1"
              and int(r["mart_shelf_tiles"]) > 0, {"shop"}, True),
    "Game Corner": (lambda r: int(r["slot_tiles"]) > 0, {"gamble"}, True),
    "Gym": (lambda r: r["gym"] == "1", {"visit"}, True),
    "tall grass": (lambda r: int(r["grass_patches"]) > 0, {"train"}, False),
    "water's edge": (lambda r: int(r["water_edge_tiles"]) > 0,
                     {"relax", "fish"}, False),
    "town square": (lambda r: int(r["squares"]) > 0, {"relax"}, True),
    "NPC chat": (lambda r: int(r["npc_chats"]) > 0, {"visit"}, True),
}
TOWN_TYPES = {"MAP_TYPE_TOWN", "MAP_TYPE_CITY"}


def load_world():
    all_maps = inv.load_maps()
    scope, events, *_ = inv.build_scope(all_maps)
    by_id = {d["id"]: d["name"] for d in scope.values()}
    warps = {name: {by_id.get(w.get("dest_map"))
                    for w in events[name]["warp_events"]} - {None}
             for name in scope}
    graph = defaultdict(set)
    for name, data in scope.items():
        # A warp counts only when its destination warps back: HNS keeps a
        # few placeholder warps (Fuchsia City's to New Bark Town) that no
        # door pairs with.
        for dest in warps[name]:
            if name in warps[dest]:
                graph[name].add(dest)
                graph[dest].add(name)
        for conn in inv.wayfarer_connections(data):
            dest = by_id.get(conn.get("map"))
            if dest:
                graph[name].add(dest)
                graph[dest].add(name)
    region = {name: inv.region_of(data) for name, data in scope.items()}
    mtype = {name: data["map_type"] for name, data in scope.items()}
    return scope, graph, region, mtype


def load_detected():
    detected = defaultdict(set)  # map -> kinds
    with (INVENTORY_DIR / "per-map.csv").open(encoding="utf-8") as f:
        for row in csv.DictReader(f):
            for kind, (test, _, _) in KINDS.items():
                if test(row):
                    detected[row["map"]].add(kind)
    return detected


def load_named():
    named = []
    for row in inv.parse_named_spots():
        named.append(dict(row, activities=set(row["activities"])))
    return named


def hops_from(graph, start, limit):
    dist, queue = {start: 0}, deque([start])
    while queue:
        cur = queue.popleft()
        if dist[cur] == limit:
            continue
        for nxt in sorted(graph[cur]):
            if nxt not in dist:
                dist[nxt] = dist[cur] + 1
                queue.append(nxt)
    return dist


def parse_routines():
    """Trainer sections: ### Name, a home-base line, a cycle and a
    favourites table (identified by their header's first cell)."""
    trainers, cur, table, in_region = {}, None, None, False
    for line in ROUTINES_DOC.read_text(encoding="utf-8").splitlines():
        head = re.match(r"^### (.+?)\s*$", line)
        if head and in_region:
            cur = trainers.setdefault(head.group(1), {
                "home": None, "cycle": [], "favourites": []})
            table = None
            continue
        if line.startswith("## "):
            in_region = bool(re.match(
                r"^## (Kanto|Johto|Hoenn|Sevii)\s*$", line))
            cur, table = None, None
            continue
        if cur is None:
            continue
        home = re.match(r"^- \*\*Home base:\*\* `(\w+)`", line)
        if home:
            cur["home"] = home.group(1)
            continue
        if not line.startswith("|"):
            table = None
            continue
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if cells[0] in ("Step", "#"):
            table = "cycle" if cells[0] == "Step" else "favourites"
            continue
        if not cells[0].isdigit():
            continue  # the separator row
        if table == "cycle":
            cur["cycle"].append(cells[1])
        elif table == "favourites":
            spot = re.match(r"^(.*?)\s*\(?`(\w+)`\)?$", cells[1])
            cur["favourites"].append({
                "label": spot.group(1).strip(" ,") if spot else cells[1],
                "map": spot.group(2) if spot else None,
                "kind": cells[2], "serves": cells[3]})
    return trainers


def main():
    scope, graph, region, mtype = load_world()
    detected = load_detected()
    named = load_named()
    catalog = json.loads(CATALOG.read_text(encoding="utf-8"))["trainers"]
    routines = parse_routines()

    def public_map(name):
        """A town or city map, or an interior entered from one."""
        if mtype[name] in TOWN_TYPES:
            return True
        return mtype[name] in inv.INTERIOR_TYPES and any(
            mtype.get(n) in TOWN_TYPES for n in graph[name])

    def own_gyms(home, leader):
        """A Gym Leader's own Gym: the Gym entered from their home base."""
        if not leader:
            return set()
        return {n for n in graph[home] if "Gym" in detected.get(n, ())}

    failures, missing, report = [], [], []
    fav_use = defaultdict(list)
    simulated = [t for t in catalog if t["id"] not in SKIPPED]
    for t in simulated:
        name = t["name"]
        r = routines.get(name)
        if r is None:
            failures.append("%s: no routine" % name)
            continue
        home = r["home"]
        if home not in scope:
            failures.append("%s: home base %s not in scope" % (name, home))
            continue
        leader = t["role"] == "Gym Leader"
        aloof, traveller = t["aloof"], t["traveller"]
        # A non-traveller's favourites stay in their home region: the home
        # base's region (Sevii for Lorelei) or the catalog's (Johto for
        # Koga, whose home base is Fuchsia City).
        home_regions = {region[home], t["homeRegion"]}
        radius = TRAVELLER_RADIUS if traveller else RADIUS
        dist = hops_from(graph, home, radius)
        if not 3 <= len(r["cycle"]) <= 4:
            failures.append("%s: cycle has %d steps" % (name, len(r["cycle"])))
        for act in r["cycle"]:
            if act not in ACTIVITIES:
                failures.append("%s: unknown activity %r" % (name, act))
        if len(r["favourites"]) > 3:
            failures.append("%s: more than 3 favourites" % name)

        good_favs = []
        for fav in r["favourites"]:
            tag = "%s: favourite %s (%s)" % (name, fav["label"], fav["map"])
            m = fav["map"]
            if m not in scope:
                failures.append(tag + ": map not in Wayfarer scope")
                continue
            if fav["kind"] == "named":
                rows = [n for n in named if n["map"] == m
                        and n["spot"] == fav["label"]]
                if not rows:
                    failures.append(tag + ": no such named spot")
                    continue
                acts, cap = rows[0]["activities"], rows[0]["capacity"]
                public = public_map(m)
            elif fav["kind"] in KINDS:
                if fav["kind"] not in detected.get(m, ()):
                    failures.append(
                        tag + ": no detected %s there" % fav["kind"])
                    continue
                acts, public = KINDS[fav["kind"]][1], KINDS[fav["kind"]][2]
                cap = 1 if mtype[m] in inv.INTERIOR_TYPES else 3
            else:
                failures.append(tag + ": unknown kind %r" % fav["kind"])
                continue
            if fav["serves"] not in acts:
                failures.append(tag + ": serves %s, spot offers %s"
                                % (fav["serves"], "/".join(sorted(acts))))
                continue
            if fav["serves"] not in r["cycle"]:
                failures.append(tag + ": serves %s, not a cycle step"
                                % fav["serves"])
            if aloof and public and fav["serves"] != "home":
                failures.append(tag + ": public spot for an aloof trainer")
            if not traveller and region[m] not in home_regions:
                failures.append(tag + ": in %s, outside %s" % (
                    region[m], " and ".join(sorted(home_regions))))
            if fav["kind"] == "Gym" and m in own_gyms(home, leader):
                failures.append(tag + ": their own Gym")
            if t["id"] not in FACE_ONLY:
                fav_use[(m, fav["label"], fav["kind"])].append((name, cap))
            good_favs.append(fav)

        steps = []
        for act in r["cycle"]:
            favs = [f for f in good_favs if f["serves"] == act]
            if act == "home":
                if leader:
                    steps.append((act, "own Gym"))
                    continue
                here = [n["spot"] for n in named if n["map"] == home]
                if detected.get(home) or here or favs:
                    steps.append((act, "home map"))
                else:
                    steps.append((act, None))
                continue
            cands = []
            for n in named:
                if (act in n["activities"] and n["map"] in dist
                        and not (aloof and public_map(n["map"]))):
                    cands.append("%s (%d)" % (n["spot"], dist[n["map"]]))
            for m, d in dist.items():
                for kind in detected.get(m, ()):
                    _, acts, public = KINDS[kind]
                    if act not in acts or (aloof and public):
                        continue
                    if kind == "Gym" and m in own_gyms(home, leader):
                        continue
                    cands.append("%s %s (%d)" % (m, kind, d))
            if favs:
                steps.append((act, "favourite %s; %d derived"
                              % (favs[0]["label"], len(cands))))
            elif cands:
                nearest = min(
                    cands, key=lambda c: int(c.rsplit("(", 1)[1][:-1]))
                steps.append((act, "%d derived, nearest %s"
                              % (len(cands), nearest)))
            else:
                steps.append((act, None))
                missing.append("%s: %s" % (name, act))
        report.append((name, home, traveller, aloof, steps,
                       t["id"] in FACE_ONLY))

    for name in routines:
        if name not in {t["name"] for t in simulated}:
            failures.append("%s: routine for a trainer not simulated" % name)

    print("# Routine check\n")
    print("Trainers: %d placeable, %d with a routine, %d simulated in v0 "
          "(%d face-only, routines kept for later).\n" % (
              len(simulated),
              sum(1 for t in simulated if t["name"] in routines),
              sum(1 for t in simulated if t["id"] not in FACE_ONLY),
              sum(1 for t in simulated if t["id"] in FACE_ONLY)))
    for name, home, traveller, aloof, steps, later in report:
        flags = (("traveller", traveller), ("aloof", aloof),
                 ("not simulated in v0", later))
        traits = ", ".join(x for x, on in flags if on)
        print("- %s (%s%s)" % (name, home, ", " + traits if traits else ""))
        for act, what in steps:
            print("  - %s: %s" % (act, what or "NO CANDIDATE"))
    print("\n## Shared favourites among v0 trainers (informational)\n")
    shared = False
    for (m, label, kind), users in sorted(fav_use.items()):
        cap = users[0][1]
        if len(users) > 1:
            shared = True
            print("- %s %s (%s), capacity %d: %s" % (
                label, m, kind, cap, ", ".join(u for u, _ in users)))
    if not shared:
        print("- none")
    print("\n## Steps with no candidate\n")
    print("\n".join("- " + s for s in missing) or "- none")
    print("\n## Failures\n")
    print("\n".join("- " + s for s in failures) or "- none")
    counts = Counter(len(r["favourites"]) for r in routines.values())
    print("\nFavourites per trainer: %s" % dict(sorted(counts.items())))
    return 1 if failures or missing else 0


if __name__ == "__main__":
    sys.exit(main())
