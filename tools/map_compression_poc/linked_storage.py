#!/usr/bin/env python3
"""Storage-only relink experiment. Outputs are NOT playable compression builds.

Requires a completed, unchanged BUILD=wayfarer release and catalog.py outputs.
Replaces only the generated maps object, relinks sequentially with production flags,
retains each result, then restores the original object and legacy release outputs.
Never modifies authored inputs or generated game assembly.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shlex
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
GAME = ROOT / "game"
ART = Path(__file__).resolve().parent / "artifacts"
OUT = ART / "linked"
CAT = ART / "catalog"
FILES = ["pokewayfarer-release" + ext for ext in (".elf", ".gba", ".map", "-size.json")]
OBJ = GAME / "build/wayfarer-release/data/maps.o"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def capture(mode):
    dest = OUT / mode
    dest.mkdir(parents=True, exist_ok=True)
    for name in FILES:
        shutil.copy2(GAME / name, dest / name)
    shutil.copy2(OBJ, dest / "maps.o")
    report = json.loads((dest / FILES[-1]).read_text())
    return {"rom_report": report, "sha256": {p.name: sha(p) for p in dest.iterdir() if p.is_file()}}


def generate(mode, rows, legacy):
    by_name = {row["layout_name"]: row for row in rows}
    seen = set()

    def replace(match):
        name, source = match.groups()
        row = by_name[name]
        assert source == row["source_path"]
        assert sha(GAME / source) == row["source_sha256"]
        compressed = mode == "hybrid" and row["storage"] == "gba_lz77"
        payload = CAT / row["compressed_stream_artifact"] if compressed else GAME / source
        assert sha(payload) == row["lz_stream_sha256" if compressed else "source_sha256"]
        stored = row["stored_bytes"] if compressed else row["raw_file_bytes"]
        crc = row["lz_stream_crc32"] if compressed else row["source_crc32"]
        seen.add(name)
        return (f'\t.balign 4\n{name}_Blockdata::\n\t.incbin "{payload}"\n'
                f'\t.balign 4\n{name}_PocDescriptor::\n'
                f'\t.4byte {name}_Blockdata, {stored}, {row["raw_file_bytes"]}, {row["logical_bytes"]}\n'
                f'\t.4byte 0x{crc}, 0x{row["source_crc32"]}\n'
                f'\t.byte 1, {int(compressed)}\n\t.2byte 0\n')

    generated = re.sub(r'(\w+)_Blockdata::\n\t.incbin "([^"]+)"\n', replace, legacy)
    assert seen == set(by_name), (len(seen), len(by_name))
    # The schema replaces the map pointer at the same offset. Legacy loader code
    # deliberately remains unchanged: this measures storage, never playable behavior.
    generated, count = re.subn(r'(?m)^\t.4byte (\w+)_Blockdata$', r'\t.4byte \1_PocDescriptor', generated)
    assert count == len(rows)
    inc = OUT / mode / "layouts.inc"
    inc.parent.mkdir(parents=True, exist_ok=True)
    inc.write_text(generated)
    asm = OUT / mode / "maps.s"
    asm.write_text((GAME / "data/maps.s").read_text().replace('"data/layouts/layouts.inc"', f'"{inc}"'))
    return asm


def main():
    global OUT
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=OUT, help="fresh directory for the three linked images")
    OUT = parser.parse_args().output.resolve()
    OUT.mkdir(parents=True, exist_ok=True)
    assert not (OUT / "comparison.json").exists(), "Use a fresh artifact directory for another experiment"
    rows = json.loads((CAT / "per-layout.json").read_text())
    legacy = (GAME / "data/layouts/layouts.inc").read_text()
    (OUT / "legacy-layouts.inc").write_text(legacy)
    commands = []
    results = {"legacy": capture("legacy")}
    baseline_cmd = next(line for line in (ART / "builds/legacy-build.log").read_text().splitlines()
                        if line.startswith("tools/preproc/preproc data/maps.s charmap.txt |"))
    try:
        for mode in ("raw-control", "hybrid"):
            asm = generate(mode, rows, legacy)
            cmd = baseline_cmd.replace("data/maps.s", shlex.quote(str(asm)))
            commands.append({"cwd": str(GAME), "command": cmd})
            with (OUT / mode / "assemble.log").open("w") as log:
                subprocess.run(["bash", "-o", "pipefail", "-c", cmd], cwd=GAME, stdout=log, stderr=subprocess.STDOUT, check=True)
            cmd = ["make", "-j6", "BUILD=wayfarer", "release"]
            commands.append({"cwd": str(GAME), "argv": cmd})
            with (OUT / mode / "relink.log").open("w") as log:
                subprocess.run(cmd, cwd=GAME, stdout=log, stderr=subprocess.STDOUT, check=True)
            results[mode] = capture(mode)
    finally:
        shutil.copy2(OUT / "legacy/maps.o", OBJ)
        for name in FILES:
            shutil.copy2(OUT / "legacy" / name, GAME / name)
        (OUT / "commands.json").write_text(json.dumps(commands, indent=2) + "\n")
    result = {"purpose": "Nonplayable storage-only relinks: no new runtime code, exception policy or shipping overhead",
              "source_revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
              "catalog_sha256": sha(CAT / "per-layout.json"), "descriptor_count": len(rows),
              "descriptor_bytes": len(rows) * 28, "builds": results}
    (OUT / "comparison.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({k: v["rom_report"]["rom"] for k, v in results.items()}, indent=2))


if __name__ == "__main__":
    main()
