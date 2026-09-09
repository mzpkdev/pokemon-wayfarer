#!/usr/bin/env python3
"""Record linked framework build evidence without treating it as gameplay acceptance."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from rom_report.rom_report import build_report, parse_symbols


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def sections(text: str) -> list[dict]:
    result = []
    for line in text.splitlines():
        match = re.match(r"\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+", line)
        if match:
            name, size, vma, lma = match.groups()
            result.append(dict(name=name, bytes=int(size, 16), vma=int(vma, 16), lma=int(lma, 16)))
    if not result:
        raise ValueError("objdump returned no ELF sections")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", type=Path, required=True)
    parser.add_argument("--map", type=Path, required=True)
    parser.add_argument("--product", required=True, choices=["wayfarer", "hns", "emerald", "firered", "leafgreen"])
    parser.add_argument("--commit", required=True)
    parser.add_argument("--command", required=True)
    parser.add_argument("--config", type=Path, required=True, help="captured effective build configuration")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--base", type=Path)
    args = parser.parse_args()
    if not re.fullmatch(r"[0-9a-f]{40}", args.commit):
        parser.error("--commit requires the full source revision")
    map_symbols = parse_symbols(args.map.read_text())
    elf_symbols = parse_symbols(subprocess.check_output(["arm-none-eabi-objdump", "-t", str(args.elf)], text=True))
    if map_symbols != elf_symbols:
        parser.error("ELF and linker map have different ROM boundary symbols")
    linked = build_report(map_symbols, build=args.product,
                          release=True, baseline_sizes=None)
    layout = sections(subprocess.check_output(["arm-none-eabi-objdump", "-h", str(args.elf)], text=True))
    ram = {name: sum(s["bytes"] for s in layout if start <= s["vma"] < end)
           for name, start, end in [("ewram", 0x02000000, 0x02040000), ("iwram", 0x03000000, 0x03008000)]}
    report = dict(schemaVersion=1, product=args.product, commit=args.commit, command=args.command,
                  configSha256=digest(args.config),
                  toolchain=subprocess.check_output(["arm-none-eabi-gcc", "--version"], text=True).splitlines()[0],
                  artifacts={"elfSha256": digest(args.elf), "mapSha256": digest(args.map)},
                  linked=linked, sections=layout, staticRam=ram,
                  runtimeAcceptance="Requires separate cycle, stack, lifecycle, and behavior evidence")
    if args.base:
        base = json.loads(args.base.read_text())
        for key in ["product", "configSha256", "toolchain"]:
            if base[key] != report[key]:
                parser.error(f"non-equivalent build metadata: {key}")
        delta = linked["rom"]["used_bytes"] - base["linked"]["rom"]["used_bytes"]
        ram_delta = {name: ram[name] - base["staticRam"][name] for name in ram}
        report["comparison"] = dict(baseCommit=base["commit"], usedRomDelta=delta,
                                    staticRamDelta=ram_delta,
                                    categories={key: value["bytes"] - base["linked"]["categories"][key]["bytes"]
                                                for key, value in linked["categories"].items()},
                                    storageGatePassed=delta <= 0 and all(v <= 0 for v in ram_delta.values()))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")


if __name__ == "__main__":
    main()
