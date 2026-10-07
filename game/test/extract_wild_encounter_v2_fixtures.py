#!/usr/bin/env python3
"""Generate test/data/wild_encounter_v2.h: what production must reproduce from the v2 reference model.

The reference model is game/tools/wild_encounters/v2/hm_model.py (place_level, slot_dist, young_limit,
prowler_min, predecessor/evo_level). The fixture holds, compactly:

  * the Road, Wilds and Outlands level at every TR from 0 to 160 (the spec's scalers);
  * one row per species of a v2 slot cap or step-down chain: young limit, baby flag, step-down
    predecessor and its evolution level, and the prowler minimum in each region;
  * one CRC per header of its place level at every TR;
  * one CRC per header and TR block of every day and night slot's exact outcome distribution,
    merged by (species, level): time, method, slot, then TR, as test/wild_encounter_exactness.c walks them.

--check compares the recorded input digest, which is cheap. --verify regenerates everything.
"""
import argparse
import hashlib
import sys
import zlib
from pathlib import Path

TEST_DIR = Path(__file__).resolve().parent
GAME = TEST_DIR.parent
REPO = GAME.parent
TARGET = TEST_DIR / "data/wild_encounter_v2.h"

sys.path.insert(0, str(GAME / "tools/wild_encounters"))
sys.path.insert(0, str(GAME / "tools/wild_encounters/v2"))

MAX_TR = 160
BLOCK_SIZE = 23  # TRs per digest block; 7 blocks cover 0..160
METHODS = ("land", "surf", "rock", "fish")
TIMES = ("day", "night")
REGION_INDEX = {"Other": 0, "Safari": 1, "Sinjoh": 2}


def input_files():
    files = [
        Path(__file__),
        TEST_DIR / "extract_native_hm_coverage_fixtures.py",
        GAME / "tools/wild_encounters/v2/hm_model.py",
        GAME / "tools/wild_encounters/v2/v2_emit.py",
        GAME / "tools/notable_trainers/evolution.json",
        GAME / "include/constants/species.h",
        REPO / ".product/specs/prowlers.md",
        REPO / ".product/specs/reach-assignments.md",
        REPO / ".product/research/native-hm-windows/revisions/nearby-access/scenarios.json",
        REPO / ".product/research/native-hm-windows/revisions/wild-encounters-v2/scenarios_regions.json",
        GAME / "src/data/pokemon/level_up_learnsets/gen_7.h",
    ]
    files += sorted((GAME / "src/data/wild_encounters_v2").glob("*.json"))
    files += sorted((GAME / "src/data/pokemon/species_info").glob("gen_*_families.h"))
    return files


def input_digest():
    digest = hashlib.sha256()
    for path in input_files():
        digest.update(path.name.encode() + b"\0" + path.read_bytes() + b"\0")
    return digest.hexdigest()


class Model:
    """The reference model plus the header order and species ids production uses."""

    def __init__(self):
        import hm_model as h
        import v2_emit
        import wild_encounters_to_header as generator

        self.h = h
        self.keys, self.tables, self.meta = v2_emit.load()
        self.map_constant = v2_emit.map_constant
        self.ids = generator.species_ids(GAME / "include/constants/species.h")
        self.cache = {}

    def species_id(self, name):
        return self.ids["SPECIES_" + name]

    def place_level(self, key, tr):
        return self.h.place_level(key, tr)

    def outcomes(self, cap, level, region):
        """[(species id, level, weight out of 50)] merged and sorted by (species id, level)."""
        entry = (cap, level, region)
        found = self.cache.get(entry)
        if found is None:
            merged = {}
            for species, lv, probability in self.h.slot_dist(cap, level, region):
                weight = probability * 50
                assert weight.denominator == 1, (cap, level, probability)
                merged[(self.species_id(species), lv)] = merged.get((self.species_id(species), lv), 0) + int(weight)
            found = tuple((s, lv, w) for (s, lv), w in sorted(merged.items()))
            assert sum(w for _, _, w in found) == 50
            self.cache[entry] = found
        return found

    def digest(self, header, block):
        key = self.keys[header]
        region = self.meta[key]["region"]
        region = region if region in ("Safari", "Sinjoh") else "Other"
        trs = range(block * BLOCK_SIZE, min((block + 1) * BLOCK_SIZE, MAX_TR + 1))
        places = [self.place_level(key, tr) for tr in trs]
        stream = bytearray()
        for time_index, time in enumerate(TIMES):
            for method_index, method in enumerate(METHODS):
                table = self.tables[key].get(method)
                if table is None:
                    continue
                for slot, cap in enumerate(table[time]):
                    if cap == "NONE":
                        continue
                    for tr, place in zip(trs, places):
                        merged = self.outcomes(cap, place, region)
                        stream += bytes([tr, time_index * 4 + method_index, slot, len(merged)])
                        for species, level, weight in merged:
                            stream += bytes([species & 0xFF, species >> 8, level, weight])
        return zlib.crc32(bytes(stream))

    def place_digest(self, header):
        key = self.keys[header]
        return zlib.crc32(bytes(self.place_level(key, tr) for tr in range(MAX_TR + 1)))

    def species_rows(self):
        h = self.h
        names = set()
        for key in self.keys:
            for method in METHODS:
                table = self.tables[key].get(method)
                if table is None:
                    continue
                for time in TIMES:
                    names.update(cap for cap in table[time] if cap != "NONE")
        pending = sorted(names)
        rows = {}
        while pending:
            name = pending.pop()
            if name in rows or "SPECIES_" + name not in self.ids:
                continue
            predecessor = h.predecessor(name)
            level = h.evo_level(predecessor, name) if predecessor else None
            limit = h.young_limit(name)
            rows[name] = (name, -1 if limit is None else limit, int(name in h.BABIES),
                          predecessor if predecessor and "SPECIES_" + predecessor in self.ids else None,
                          level or 0, [h.prowler_min(name, region) or 0 for region in REGION_INDEX])
            if predecessor:
                pending.append(predecessor)
        return [rows[name] for name in sorted(rows, key=self.species_id)]


def nibble_table():
    table = []
    for index in range(16):
        value = index
        for _ in range(4):
            value = (value >> 1) ^ (0xEDB88320 if value & 1 else 0)
        table.append(value)
    return table


def render():
    model = Model()
    h = model.h
    blocks = (MAX_TR + BLOCK_SIZE) // BLOCK_SIZE
    lines = ["// Generated by test/extract_wild_encounter_v2_fixtures.py; do not edit.",
             f"// Input digest: {input_digest()}",
             f"#define WILD_V2_HEADER_COUNT {len(model.keys)}",
             f"#define WILD_V2_TR_BLOCK_SIZE {BLOCK_SIZE}",
             f"#define WILD_V2_TR_BLOCKS {blocks}",
             "",
             "static const u32 sWildV2CrcNibbles[16] =", "{"]
    lines.append("    " + ", ".join(f"0x{value:08X}" for value in nibble_table()) + ",")
    lines += ["};", "", "// Road, Wilds, Outlands level at each TR from 0 to 160 (spec scalers).",
              "static const u8 sWildV2ReachLevels[161][3] =", "{"]
    for tr in range(MAX_TR + 1):
        lines.append("    { " + ", ".join(str(level) for level in h.reach_levels(tr)) + " },")
    lines += ["};", "", "// The map each header must hold, in header order (Bug Contest days are Tuesday, Thursday, Saturday).",
              "static const u16 sWildV2HeaderMaps[WILD_V2_HEADER_COUNT] =", "{"]
    for key in model.keys:
        lines.append(f"    {model.map_constant(key)},")
    lines += ["};", ""]
    lines += ["struct WildV2SpeciesRow", "{", "    u16 species;", "    s8 youngLimit;  // -1: no limit", "    u8 baby;",
              "    u16 predecessor;", "    u8 evolutionLevel;", "    u8 prowlerMinimum[3];  // other, Safari, Sinjoh", "};", "",
              "static const struct WildV2SpeciesRow sWildV2SpeciesRows[] =", "{"]
    for name, limit, baby, predecessor, level, minimums in model.species_rows():
        pred = "SPECIES_" + predecessor if predecessor else "SPECIES_NONE"
        lines.append(f"    {{ SPECIES_{name}, {limit}, {baby}, {pred}, {level}, {{ {', '.join(map(str, minimums))} }} }},")
    lines += ["};", "", "// CRC-32 of each header's place level at TR 0..160.", "static const u32 sWildV2PlaceDigests[WILD_V2_HEADER_COUNT] =", "{"]
    for header in range(len(model.keys)):
        lines.append(f"    0x{model.place_digest(header):08X},")
    lines += ["};", "", "// CRC-32 per header and TR block of every day and night slot's merged outcomes.",
              "static const u32 sWildV2OutcomeDigests[WILD_V2_HEADER_COUNT][WILD_V2_TR_BLOCKS] =", "{"]
    for header in range(len(model.keys)):
        lines.append("    { " + ", ".join(f"0x{model.digest(header, block):08X}" for block in range(blocks)) + " },")
    lines += ["};", ""]
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="compare the recorded input digest (cheap)")
    parser.add_argument("--verify", action="store_true", help="regenerate everything and compare")
    args = parser.parse_args()
    if args.verify:
        if TARGET.read_text() != render():
            raise SystemExit(f"{TARGET} is stale")
    elif args.check:
        if f"// Input digest: {input_digest()}" not in TARGET.read_text().split("\n")[:3]:
            raise SystemExit(f"{TARGET} is stale: run test/extract_wild_encounter_v2_fixtures.py")
    else:
        TARGET.write_text(render())


if __name__ == "__main__":
    main()
