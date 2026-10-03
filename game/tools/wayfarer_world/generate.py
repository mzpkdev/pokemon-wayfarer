#!/usr/bin/env python3
"""Generate the notable world's walker graph, spot table and trainer tables.

Writes src/data/wayfarer_world/tables.h (the definitions of every extern in
include/wayfarer_world_data.h) and a JSON report, and exits non-zero with
the reasons on any validation failure. Specs:
.product/specs/notable-world-simulation.md (walker graph, travel, record)
and .product/specs/notable-spots.md (kinds, detection, named spots).

    python3 tools/wayfarer_world/generate.py [--root game] [--output PATH]
        [--report PATH] [--print-inputs]

Inputs are the Wayfarer build's mapjson outputs (data/maps/groups.inc,
include/constants/map_groups.h, data/maps/*/{header,events,connections}.inc)
plus the static files `--print-inputs` lists.
"""

import argparse
import json
import sys
import time
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))

import build  # noqa: E402
import emit  # noqa: E402
import maps  # noqa: E402
import routines  # noqa: E402
from maps import BuildError  # noqa: E402

OUTPUT = Path("src/data/wayfarer_world/tables.h")
REPORT = Path("build/wayfarer-world-report.json")


def generate(root, only=None, named_rows=None, overrides=None, transit=None,
             routines_path=None):
    """Build everything; returns (world graph, routines, C text, report)."""
    started = time.monotonic()
    world = maps.World(root, only=only)
    world.build_scope(seed_all=only is not None)
    wg = build.WorldGraph(world, named_rows=named_rows, overrides=overrides, transit=transit)
    wg.check()
    rt = routines.Routines(wg, routines_path=routines_path)
    rt.check()
    crc = emit.content_hash(wg)
    text, candidate_count = emit.render(wg, rt, crc)
    byte_sizes = emit.sizes(wg, rt, candidate_count)
    if candidate_count > 0xFFFF:
        raise BuildError("%d candidates, over the u16 candidateStart" % candidate_count)
    rep = emit.report(wg, rt, crc, byte_sizes, time.monotonic() - started)
    return wg, rt, text, rep


def write_if_changed(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.read_text(encoding="utf-8") == text:
        return False
    path.write_text(text, encoding="utf-8")
    return True


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", type=Path, default=maps.DEFAULT_ROOT,
                        help="the game/ directory (default: this tool's game/)")
    parser.add_argument("--output", type=Path, help="tables.h path (default: %s)" % OUTPUT)
    parser.add_argument("--report", type=Path, help="report path (default: %s)" % REPORT)
    parser.add_argument("--print-inputs", action="store_true",
                        help="print the static input files, one per line, and exit")
    args = parser.parse_args(argv)
    root = args.root.resolve()
    if args.print_inputs:
        print("\n".join(maps.input_paths(root)))
        return 0
    try:
        wg, rt, text, rep = generate(root)
    except BuildError as err:
        print("wayfarer_world: validation failed:", file=sys.stderr)
        for line in str(err).splitlines():
            print("  - " + line, file=sys.stderr)
        return 1
    output = args.output or root / OUTPUT
    report = args.report or root / REPORT
    write_if_changed(Path(output), text + "\n")
    report = Path(report)
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(rep, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    for warning in rt.warnings:
        print("wayfarer_world: warning: " + warning, file=sys.stderr)
    sizes = rep["table_bytes"]
    print("wayfarer_world: %d nodes, %d edges, %d spots, %d bytes of tables, hash %s (%.1fs)"
          % (rep["nodes"], rep["edges"], rep["spots"], sizes["total"], rep["content_hash"],
             rep["generator_seconds"]))
    if rep["table_bytes_over_300k"]:
        print("wayfarer_world: warning: tables exceed 300 KB", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
