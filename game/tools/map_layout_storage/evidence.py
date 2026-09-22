#!/usr/bin/env python3
"""Build and validate a map-layout rollout evidence bundle."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
import subprocess
from pathlib import Path
from typing import Any


class EvidenceError(ValueError):
    pass


REQUIRED_TIMING_CASES = {
    "black-screen-warp-emerald-canary",
    "black-screen-warp-frlg-canary",
    "black-screen-warp-hns-canary",
    "black-screen-save-reload-hns-canary",
}

EXPECTED_STAGE1_CANARIES = {
    "LAYOUT_FORTREE_CITY_HOUSE1": "emerald",
    "LAYOUT_FIVE_ISLAND_RESORT_GORGEOUS_HOUSE": "frlg",
    "LAYOUT_OLIVINE_CITY_CAFE_HNS": "hns",
}

E2E_ROM_NAMES = {
    "legacy": "pokemon-wayfarer-e2e-legacy-nosinnoh.gba",
    "raw": "pokemon-wayfarer-e2e-nosinnoh.gba",
    "hybrid": "pokemon-wayfarer-e2e-hybrid-nosinnoh.gba",
}

LEGACY_FORBIDDEN_SYMBOLS = {
    "AbortMapLayoutLoad",
    "CB2_MapLayoutLoadError",
    "MapLayoutAcquireView",
    "MapLayoutBeginLoadContext",
    "MapLayoutCopyFull",
    "MapLayoutCopyFullWithContext",
    "MapLayoutCopyRect",
    "MapLayoutCopyRectWithContext",
    "MapLayoutEndLoadContext",
    "MapLayoutReadTile",
    "MapLayoutReleaseView",
    "gMapLayoutLoadError",
}


def _load(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text())
    if not isinstance(value, dict):
        raise EvidenceError(f"{path} must contain a JSON object")
    return value


def _percentile(values: list[int], percentile: float) -> int:
    ordered = sorted(values)
    return ordered[math.ceil(percentile * len(ordered)) - 1]


def summarize_timing(document: dict[str, Any], revision: str,
                     expected_roms: dict[str, str] | None = None) -> dict[str, Any]:
    if document.get("schema_version") != 1 or document.get("source_revision") != revision:
        raise EvidenceError("timing evidence schema or source revision mismatch")
    device = document.get("device")
    timer = document.get("timer")
    measurement_revision = document.get("measurement_revision")
    if (not isinstance(device, str) or not device or not isinstance(timer, str) or not timer
            or not isinstance(measurement_revision, str) or not measurement_revision):
        raise EvidenceError("timing evidence requires device, timer, and measurement revision")
    builds = document.get("builds")
    if not isinstance(builds, dict) or set(builds) != {"legacy", "raw", "hybrid"}:
        raise EvidenceError("timing evidence requires checksums for all three builds")
    for mode, build in builds.items():
        digest = build.get("rom_sha256") if isinstance(build, dict) else None
        if not isinstance(digest, str) or re.fullmatch(r"[0-9a-f]{64}", digest) is None:
            raise EvidenceError(f"timing build {mode} requires a lowercase ROM SHA-256")
        if expected_roms is not None and digest != expected_roms[mode]:
            raise EvidenceError(f"timing build {mode} does not match the retained E2E ROM")
    configurations = document.get("configurations")
    if not isinstance(configurations, dict) or set(configurations) != {"legacy", "raw", "hybrid"}:
        raise EvidenceError("timing evidence requires legacy, raw, and hybrid configurations")
    case_sets = []
    for mode, cases in configurations.items():
        if not isinstance(cases, dict) or not cases:
            raise EvidenceError(f"timing configuration {mode} requires cases")
        case_sets.append(set(cases))
    if any(cases != case_sets[0] for cases in case_sets[1:]):
        raise EvidenceError("timing configurations must contain identical cases")
    if case_sets[0] != REQUIRED_TIMING_CASES:
        raise EvidenceError("timing evidence does not contain the required Stage 1 cases")

    rows = []
    passed = True
    for case in sorted(case_sets[0]):
        samples: dict[str, list[int]] = {}
        for mode in ("legacy", "raw", "hybrid"):
            values = configurations[mode][case]
            if (not isinstance(values, list) or len(values) < 100
                    or any(isinstance(value, bool) or not isinstance(value, int) or value < 0
                           for value in values)):
                raise EvidenceError(f"timing case {case}/{mode} requires at least 100 frame samples")
            samples[mode] = values
        if len({len(values) for values in samples.values()}) != 1:
            raise EvidenceError(f"timing case {case} must contain paired sample counts")
        legacy_to_raw = [raw - legacy for raw, legacy
                         in zip(samples["raw"], samples["legacy"])]
        raw_to_hybrid = [hybrid - raw for hybrid, raw
                         in zip(samples["hybrid"], samples["raw"])]
        legacy_to_hybrid = [hybrid - legacy for hybrid, legacy
                            in zip(samples["hybrid"], samples["legacy"])]
        row_passed = max(legacy_to_hybrid) <= 5
        passed &= row_passed
        rows.append({
            "case": case,
            "sample_count": len(legacy_to_hybrid),
            "legacy_frames": {
                "median": _percentile(samples["legacy"], 0.50),
                "p95": _percentile(samples["legacy"], 0.95),
                "max": max(samples["legacy"]),
            },
            "raw_frames": {
                "median": _percentile(samples["raw"], 0.50),
                "p95": _percentile(samples["raw"], 0.95),
                "max": max(samples["raw"]),
            },
            "hybrid_frames": {
                "median": _percentile(samples["hybrid"], 0.50),
                "p95": _percentile(samples["hybrid"], 0.95),
                "max": max(samples["hybrid"]),
            },
            "legacy_to_raw_additional_frames": {
                "median": _percentile(legacy_to_raw, 0.50),
                "p95": _percentile(legacy_to_raw, 0.95),
                "max": max(legacy_to_raw),
            },
            "raw_to_hybrid_additional_frames": {
                "median": _percentile(raw_to_hybrid, 0.50),
                "p95": _percentile(raw_to_hybrid, 0.95),
                "max": max(raw_to_hybrid),
            },
            "legacy_to_hybrid_additional_frames": {
                "median": _percentile(legacy_to_hybrid, 0.50),
                "p95": _percentile(legacy_to_hybrid, 0.95),
                "max": max(legacy_to_hybrid),
            },
            "passed": row_passed,
        })
    return {
        "device": device,
        "timer": timer,
        "measurement_revision": measurement_revision,
        "builds": builds,
        "cases": rows,
        "passed": passed,
    }


def summarize_rollback(document: dict[str, Any], revision: str,
                       expected_roms: dict[str, str] | None = None) -> dict[str, Any]:
    if document.get("schema_version") != 1 or document.get("source_revision") != revision:
        raise EvidenceError("rollback evidence schema or source revision mismatch")
    hybrid = document.get("hybrid_save")
    raw = document.get("raw_continue")
    decorated = document.get("decorated_secret_base")
    raw_decorated = document.get("raw_decorated_secret_base_continue")
    roms = document.get("roms")
    for name, value in (("hybrid_save", hybrid), ("raw_continue", raw),
                        ("decorated_secret_base", decorated),
                        ("raw_decorated_secret_base_continue", raw_decorated)):
        if not isinstance(value, dict):
            raise EvidenceError(f"rollback evidence requires {name}")
    if not isinstance(roms, dict) or set(roms) != {"hybrid", "raw"}:
        raise EvidenceError("rollback evidence requires hybrid and raw ROM identities")
    for mode in ("hybrid", "raw"):
        digest = roms[mode].get("rom_sha256") if isinstance(roms[mode], dict) else None
        if not isinstance(digest, str) or re.fullmatch(r"[0-9a-f]{64}", digest) is None:
            raise EvidenceError(f"rollback {mode} ROM requires a lowercase SHA-256")
        if expected_roms is not None and digest != expected_roms[mode]:
            raise EvidenceError(f"rollback {mode} run does not match the retained E2E ROM")
    if (hybrid.get("map") != raw.get("map") or hybrid.get("player") != raw.get("player")
            or raw.get("resaved_and_reloaded") is not True):
        raise EvidenceError("raw control did not continue and resave the hybrid map save")
    digest = hybrid.get("save_sha256")
    if not isinstance(digest, str) or re.fullmatch(r"[0-9a-f]{64}", digest) is None:
        raise EvidenceError("hybrid rollback save requires a SHA-256")
    if (decorated.get("map") != raw_decorated.get("map")
            or decorated.get("decoration_count", 0) < 1
            or decorated.get("decoration_count") != raw_decorated.get("decoration_count")
            or decorated.get("decoration_fingerprint")
            != raw_decorated.get("decoration_fingerprint")
            or raw_decorated.get("resaved_and_reloaded") is not True):
        raise EvidenceError("raw control did not preserve the decorated Secret Base save")
    decorated_digest = decorated.get("save_sha256")
    if not isinstance(decorated_digest, str) or re.fullmatch(r"[0-9a-f]{64}", decorated_digest) is None:
        raise EvidenceError("decorated Secret Base save requires a SHA-256")
    return {
        "hybrid_save_sha256": digest,
        "decorated_secret_base_save_sha256": decorated_digest,
        "decorated_secret_base_fingerprint": decorated["decoration_fingerprint"],
        "roms": roms,
        "passed": True,
    }


def parse_elf_memory(map_text: str, nm_text: str) -> dict[str, Any]:
    section_pattern = re.compile(r"^\.(ewram(?:\.sbss)?|iwram(?:\.bss)?)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)", re.M)
    sections = {name: int(size, 16) for name, _address, size in section_pattern.findall(map_text)}
    required_sections = {"ewram", "ewram.sbss", "iwram", "iwram.bss"}
    if set(sections) != required_sections:
        raise EvidenceError("ELF map is missing EWRAM or IWRAM output sections")
    symbols: dict[str, dict[str, Any]] = {}
    for line in nm_text.splitlines():
        fields = line.split()
        if len(fields) == 4 and fields[3] in {"gHeap", "sBackupMapData", "gMapLayoutLoadError"}:
            symbols[fields[3]] = {"address": f"0x{int(fields[0], 16):08X}", "bytes": int(fields[1], 16)}
    if "gHeap" not in symbols or "sBackupMapData" not in symbols:
        raise EvidenceError("ELF symbol report is missing gHeap or sBackupMapData")
    return {
        "ewram_bytes": sections["ewram"] + sections["ewram.sbss"],
        "ewram_sections": sections,
        "iwram_bytes": sections["iwram"] + sections["iwram.bss"],
        "stack_reserve_bytes": 0x1C0,
        "symbols": symbols,
    }


def validate_legacy_symbols(nm_text: str) -> None:
    linked = {fields[-1] for line in nm_text.splitlines()
              if len(fields := line.split()) >= 3}
    forbidden = sorted(linked & LEGACY_FORBIDDEN_SYMBOLS)
    if forbidden:
        raise EvidenceError(
            "legacy baseline linked compression infrastructure: " + ", ".join(forbidden))


def _sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _build_record(directory: Path, mode: str, nm: str, artifact_root: Path) -> dict[str, Any]:
    rom = _load(directory / "pokewayfarer-release-size.json")
    storage = _load(directory / "map-layout-storage.json")
    if storage.get("storage_mode") != mode:
        raise EvidenceError(f"{mode} storage report has the wrong mode")
    elf = directory / "pokewayfarer-release.elf"
    map_path = directory / "pokewayfarer-release.map"
    nm_result = subprocess.run([nm, "-S", "--size-sort", str(elf)], check=True, capture_output=True, text=True)
    memory = parse_elf_memory(map_path.read_text(errors="replace"), nm_result.stdout)
    if mode == "legacy":
        validate_legacy_symbols(nm_result.stdout)
    e2e_rom = artifact_root / E2E_ROM_NAMES[mode]
    return {
        "rom": rom["rom"],
        "rom_categories": rom["categories"],
        "memory": memory,
        "e2e_rom": {"path": e2e_rom.name, "sha256": _sha256(e2e_rom)},
        "storage_manifest_sha256": _sha256(directory / "map-layout-storage.json"),
        "storage": storage,
        "legacy_infrastructure_absent": True if mode == "legacy" else None,
    }


def build_bundle(root: Path, timing: dict[str, Any] | None, rollback: dict[str, Any] | None,
                 expected_revision: str, artifact_root: Path,
                 nm: str = "arm-none-eabi-nm") -> dict[str, Any]:
    builds = {mode: _build_record(root / mode, mode, nm, artifact_root)
              for mode in ("legacy", "raw", "hybrid")}
    revisions = {record["storage"].get("source_revision") for record in builds.values()}
    if revisions != {expected_revision}:
        raise EvidenceError("paired build source revisions do not match the accepted source tree")
    revision = expected_revision
    canaries = [row for row in builds["hybrid"]["storage"]["layouts"] if row.get("rollout_stage") == "stage1"]
    actual_canaries = {row.get("layout_id"): row.get("source_version") for row in canaries}
    if len(canaries) != len(EXPECTED_STAGE1_CANARIES) or actual_canaries != EXPECTED_STAGE1_CANARIES:
        raise EvidenceError("Stage 1 requires the approved Emerald, FRLG, and HNS canaries")
    if len({row["raw_file_bytes"] for row in canaries}) != len(canaries):
        raise EvidenceError("Stage 1 requires distinct canary sizes")
    if any(row["storage"] != "gba_lz77" for row in canaries):
        raise EvidenceError("every Stage 1 canary must be compressed")
    if builds["raw"]["storage"]["totals"]["compressed_entries"] != 0:
        raise EvidenceError("raw rollback report contains compressed layouts")
    if builds["legacy"]["storage"]["totals"]["descriptor_bytes"] != 0:
        raise EvidenceError("legacy size report contains descriptors")

    for mode, build in builds.items():
        symbols = build["memory"]["symbols"]
        if symbols["gHeap"]["bytes"] != 0x1C500 or symbols["sBackupMapData"]["bytes"] != 20480:
            raise EvidenceError(f"{mode} changed the fixed heap or backup-map allocation")
    legacy_memory = builds["legacy"]["memory"]
    memory_passed = all(
        build["memory"][region] - legacy_memory[region] <= 64
        for build in (builds["raw"], builds["hybrid"])
        for region in ("ewram_bytes", "iwram_bytes")
    )
    if not memory_passed:
        raise EvidenceError("map-layout rollout added a forbidden static memory buffer")

    legacy_used = builds["legacy"]["rom"]["used_bytes"]
    raw_used = builds["raw"]["rom"]["used_bytes"]
    hybrid_used = builds["hybrid"]["rom"]["used_bytes"]
    expected_e2e_roms = {mode: build["e2e_rom"]["sha256"] for mode, build in builds.items()}
    timing_report = None if timing is None else summarize_timing(timing, revision, expected_e2e_roms)
    rollback_report = None if rollback is None else summarize_rollback(rollback, revision, expected_e2e_roms)
    promotion_passed = (timing_report is not None and timing_report["passed"]
                        and rollback_report is not None and rollback_report["passed"])
    return {
        "schema_version": 1,
        "rollout_stage": "stage1",
        "source_revision": revision,
        "canaries": [{key: row[key] for key in (
            "layout_id", "layout_name", "source_version", "raw_file_bytes", "stored_bytes")}
            for row in canaries],
        "builds": builds,
        "rom_comparison": {
            "legacy_to_hybrid_bytes": hybrid_used - legacy_used,
            "raw_to_hybrid_bytes": hybrid_used - raw_used,
            "legacy_used_bytes": legacy_used,
            "raw_used_bytes": raw_used,
            "hybrid_used_bytes": hybrid_used,
        },
        "static_memory_passed": memory_passed,
        "timing": timing_report or {"passed": False, "reason": "accurate timing evidence not supplied"},
        "rollback": rollback_report or {"passed": False, "reason": "rollback evidence not supplied"},
        "promotion_passed": promotion_passed,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--timing", type=Path)
    parser.add_argument("--rollback", type=Path)
    parser.add_argument("--source-revision", required=True)
    parser.add_argument("--artifact-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--nm", default="arm-none-eabi-nm")
    parser.add_argument("--allow-incomplete", action="store_true")
    args = parser.parse_args()
    bundle = build_bundle(args.root, _load(args.timing) if args.timing else None,
                          _load(args.rollback) if args.rollback else None,
                          args.source_revision, args.artifact_root, args.nm)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(bundle, indent=2, sort_keys=True) + "\n")
    if not bundle["promotion_passed"] and not args.allow_incomplete:
        print("Stage 1 promotion blocked by incomplete or failing timing/rollback evidence.")
        return 1
    print(f"Stage 1 evidence bundle: {args.output} (promotion_passed={bundle['promotion_passed']})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
