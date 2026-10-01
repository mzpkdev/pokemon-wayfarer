#!/usr/bin/env python3
"""Debug and balance report for the notable world simulation.

Compiles the ROM's simulation core and generated tables for the host, runs
N heartbeats of a scenario without playing, and reports itineraries,
crowding, coverage and search cost (notable-world-simulation.md, "Debug and
balance report"). Requires a Wayfarer build's generated headers
(src/data/wayfarer_world/tables.h, include/constants/map_groups.h).
"""

import argparse
import collections
import json
import os
import re
import subprocess
import sys
from pathlib import Path

GAME = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
SOURCES = [HERE / "sim_report.c", GAME / "src/wayfarer_world_sim.c", GAME / "src/wayfarer_world_data.c"]
TABLES = GAME / "src/data/wayfarer_world/tables.h"

ACTIVITIES = ["care", "shop", "gamble", "train", "relax", "fish", "visit", "study", "home", "sightsee", "lie low"]
STATES = ["travelling", "dwelling", "away: league", "away: partner", "pinned", "home-locked"]
LIFE = ["", "recovering", "celebrating", "brooding"]
SKIPS = ["radius", "aloof", "full"]
REGIONS = ["Kanto", "Johto", "Hoenn", "Sevii"]
KINDS = ["center counter", "center side", "store", "game corner", "gym", "tall grass", "water's edge",
         "square", "bench", "npc chat", "named"]


def map_names():
    text = (GAME / "include/constants/map_groups.h").read_text()
    names = {}
    for name, num, group in re.findall(r"(MAP_\w+)\s*=\s*\((\d+) \| \((\d+) << 8\)\)", text):
        names[int(num) | (int(group) << 8)] = name
    return names


def notable_names():
    text = (GAME / "include/constants/notable_trainers.h").read_text()
    return {int(v): n.replace("NOTABLE_TRAINER_", "") for n, v in re.findall(r"(NOTABLE_TRAINER_\w+) = (\d+)", text)}


def resolve_ids(values, names):
    by_name = {v: k for k, v in names.items()}
    out = []
    for value in values:
        value = value.strip().upper()
        if value.isdigit():
            out.append(int(value))
        elif value in by_name:
            out.append(by_name[value])
        else:
            raise SystemExit(f"unknown trainer {value!r}")
    return out


def resolve_maps(values, names):
    by_name = {v: k for k, v in names.items()}
    out = []
    for value in values:
        value = value.strip()
        key = value if value.startswith("MAP_") else "MAP_" + value
        if key not in by_name:
            raise SystemExit(f"unknown map {value!r}")
        out.append(by_name[key])
    return out


def build(binary):
    if not TABLES.exists():
        raise SystemExit(f"{TABLES} is missing: build the Wayfarer ROM (or run its generator) first")
    newest = max(p.stat().st_mtime for p in SOURCES + [TABLES])
    if binary.exists() and binary.stat().st_mtime >= newest:
        return
    binary.parent.mkdir(parents=True, exist_ok=True)
    cc = os.environ.get("HOSTCC", "cc")
    cmd = [cc, "-std=gnu11", "-O2", "-Wall", "-DPOKEMON_WAYFARER", "-iquote", str(GAME / 'include'), "-iquote", str(GAME / 'src'),
           "-o", str(binary)] + [str(s) for s in SOURCES]
    subprocess.run(cmd, check=True)


def run(binary, args):
    result = subprocess.run([str(binary)] + args, check=True, capture_output=True, text=True)
    return result.stdout


def parse(output):
    data = {"nodes": {}, "spots": {}, "trainers": {}, "records": [], "beats": [], "skips": [], "occ": [],
            "invalid": [], "resolved": []}
    for line in output.splitlines():
        parts = line.split()
        tag, values = parts[0], parts[1:]
        if tag == "tables":
            data["tables"] = dict(kv.split("=") for kv in values)
            continue
        nums = [int(v) for v in values]
        if tag == "node":
            data["nodes"][nums[0]] = {"map": nums[1], "flags": nums[2]}
        elif tag == "spot":
            data["spots"][nums[0]] = {"node": nums[1], "kind": nums[2], "x": nums[3], "y": nums[4]}
        elif tag == "trainer":
            data["trainers"][nums[0]] = dict(zip(["character", "homeMap", "homeNode", "flags", "candidates",
                                                  "searchBound"], nums[1:]))
        elif tag == "rec":
            data["records"].append(dict(zip(["hb", "slot", "node", "map", "state", "destKind", "destId", "destMap",
                                             "activity", "dwell", "life", "lifeSteps", "step", "arrival", "waited"],
                                            nums)))
        elif tag == "beat":
            data["beats"].append(dict(zip(["hb", "playerMap", "searches", "searchNodes", "searchNodesMax", "hops",
                                           "waits", "reroutes", "advances", "skips"], nums)))
        elif tag == "skip":
            data["skips"].append(dict(zip(["hb", "slot", "activity", "reason"], nums)))
        elif tag == "occ":
            data["occ"].append(dict(zip(["hb", "map", "count", "cap"], nums)))
        elif tag == "valid" and nums[1] == 0:
            data["invalid"].append(nums[0])
        elif tag == "resolved":
            data["resolved"].append(nums[0])
    return data


def summarise(data, maps, notables):
    def map_name(m):
        return maps.get(m, f"0x{m:04X}").replace("MAP_", "")

    trainers = data["trainers"]
    name = {s: notables.get(t["character"], str(t["character"])) for s, t in trainers.items()}
    beats = len(data["beats"])
    summary = {"heartbeats": beats, "invalid": data["invalid"], "tables": data.get("tables", {})}

    # Crowding.
    per_map = collections.defaultdict(list)
    caps = {}
    for row in data["occ"]:
        per_map[row["map"]].append(row["count"])
        caps[row["map"]] = row["cap"]
    over = [r for r in data["occ"] if r["count"] > r["cap"]]
    crowding = []
    for m, counts in per_map.items():
        crowding.append({"map": map_name(m), "cap": caps[m], "peak": max(counts),
                         "mean": round(sum(counts) / (beats + 1), 3),
                         "capHits": sum(1 for c in counts if c >= caps[m])})
    crowding.sort(key=lambda r: (-r["mean"], r["map"]))
    shared = collections.Counter()
    for r in data["occ"]:
        if r["cap"] > 1 and r["count"] >= 2:
            shared[r["count"]] += 1
    summary["crowding"] = {"overCap": over, "busiest": crowding[:15],
                           "outdoorShared": {str(k): v for k, v in sorted(shared.items())},
                           "waits": sum(b["waits"] for b in data["beats"]),
                           "reroutes": sum(b["reroutes"] for b in data["beats"])}

    # Coverage.
    activity = collections.defaultdict(collections.Counter)
    region = collections.defaultdict(collections.Counter)
    left_home = collections.defaultdict(bool)
    used_spots = set()
    for r in data["records"]:
        s = r["slot"]
        if r["state"] == 1:
            activity[s][ACTIVITIES[r["activity"]] if r["activity"] < len(ACTIVITIES) else "?"] += 1
        if r["state"] in (0, 1):
            flags = data["nodes"][r["node"]]["flags"]
            region[s][REGIONS[(flags >> 1) & 3]] += 1
            if r["map"] != trainers[s]["homeMap"]:
                left_home[s] = True
            if r["destKind"] in (1, 2) and r["destId"] in data["spots"]:
                used_spots.add(r["destId"])
    coverage = {}
    for s in sorted(trainers):
        total = sum(activity[s].values()) or 1
        rtotal = sum(region[s].values()) or 1
        coverage[name[s]] = {
            "activities": {k: round(v / total, 3) for k, v in activity[s].most_common()},
            "regions": {k: round(v / rtotal, 3) for k, v in region[s].most_common()},
            "leftHome": left_home[s],
        }
    kinds_unused = collections.Counter(KINDS[data["spots"][i]["kind"]] for i in data["spots"] if i not in used_spots)
    summary["coverage"] = {"trainers": coverage,
                           "neverLeftHome": [name[s] for s in sorted(trainers) if not left_home[s]],
                           "spotsUsed": len(used_spots), "spotsUnusedByKind": dict(kinds_unused)}

    # Skips.
    skips = collections.defaultdict(collections.Counter)
    for row in data["skips"]:
        skips[name[row["slot"]]][f"{ACTIVITIES[row['activity']]}:{SKIPS[row['reason']]}"] += 1
    summary["skips"] = {k: dict(v) for k, v in sorted(skips.items())}

    # Cost.
    nodes = [b["searchNodes"] for b in data["beats"]]
    summary["cost"] = {"searchNodesPerHeartbeatMax": max(nodes) if nodes else 0,
                       "searchNodesPerHeartbeatMean": round(sum(nodes) / len(nodes), 1) if nodes else 0,
                       "largestSingleSearch": max((b["searchNodesMax"] for b in data["beats"]), default=0),
                       "searchesPerHeartbeatMax": max((b["searches"] for b in data["beats"]), default=0)}
    return summary


def itinerary(data, maps, notables):
    trainers = data["trainers"]
    lines = ["hb\ttrainer\tmap\tnode\tstate\tactivity\tdest\tdestMap\tdwell\tlife\tstep"]
    for r in data["records"]:
        who = notables.get(trainers[r["slot"]]["character"], "?")
        dest = "-" if r["destKind"] == 0 else ("home" if r["destKind"] == 2 else str(r["destId"]))
        lines.append("\t".join(str(v) for v in (
            r["hb"], who, maps.get(r["map"], r["map"]).replace("MAP_", ""), r["node"], STATES[r["state"]],
            ACTIVITIES[r["activity"]] if r["activity"] < 11 else "-", dest,
            maps.get(r["destMap"], "-").replace("MAP_", ""), r["dwell"], LIFE[r["life"]], r["step"])))
    for row in data["skips"]:
        who = notables.get(trainers[row["slot"]]["character"], "?")
        lines.append(f"{row['hb']}\t{who}\tSKIP\t{ACTIVITIES[row['activity']]}\t{SKIPS[row['reason']]}")
    return "\n".join(lines) + "\n"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--heartbeats", type=int, default=200)
    parser.add_argument("--wp", type=int, default=0, help="world progress (player TR)")
    parser.add_argument("--badges", default="0,0,0", help="Kanto,Johto,Hoenn badge bit masks, e.g. 0xFF,0,0")
    parser.add_argument("--all-badges", action="store_true")
    parser.add_argument("--path", default="", help="comma-separated maps the player loads in turn (cycled); "
                                                   "empty means the player is nowhere in the world")
    parser.add_argument("--lineup", default="", help="accepted event lineup (battle order), away until resolved")
    parser.add_argument("--resolve-at", type=int, default=-1)
    parser.add_argument("--resolve-won", action="store_true")
    parser.add_argument("--provisional", default="")
    parser.add_argument("--rising", default="")
    parser.add_argument("--json", type=Path, help="write the summary JSON here")
    parser.add_argument("--itinerary", type=Path, help="write the per-heartbeat itinerary TSV here")
    parser.add_argument("--binary", type=Path, default=GAME / "build/wayfarer_world_report/sim_report")
    args = parser.parse_args(argv)

    maps, notables = map_names(), notable_names()
    build(args.binary)
    badges = "0xFF,0xFF,0xFF" if args.all_badges else args.badges
    cli = ["--heartbeats", str(args.heartbeats), "--wp", str(args.wp), "--badges", badges]
    if args.path:
        cli += ["--path", ",".join(str(m) for m in resolve_maps(args.path.split(","), maps))]
    for flag, value in (("--lineup", args.lineup), ("--provisional", args.provisional), ("--rising", args.rising)):
        if value:
            cli += [flag, ",".join(str(i) for i in resolve_ids(value.split(","), notables))]
    if args.resolve_at >= 0:
        cli += ["--resolve-at", str(args.resolve_at), "--resolve-won", "1" if args.resolve_won else "0"]
    data = parse(run(args.binary, cli))
    summary = summarise(data, maps, notables)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(summary, indent=2) + "\n")
    if args.itinerary:
        args.itinerary.parent.mkdir(parents=True, exist_ok=True)
        args.itinerary.write_text(itinerary(data, maps, notables))
    json.dump({k: summary[k] for k in ("heartbeats", "invalid", "cost")} | {
        "overCap": len(summary["crowding"]["overCap"]), "waits": summary["crowding"]["waits"],
        "reroutes": summary["crowding"]["reroutes"], "neverLeftHome": summary["coverage"]["neverLeftHome"],
        "skipTotals": {k: sum(v.values()) for k, v in summary["skips"].items()}}, sys.stdout, indent=2)
    print()
    return 1 if summary["invalid"] or summary["crowding"]["overCap"] else 0


if __name__ == "__main__":
    sys.exit(main())
