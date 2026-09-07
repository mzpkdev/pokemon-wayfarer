#!/usr/bin/env python3
"""Read-only ordinary encounter projection for native-HM design research.

Probabilities are conditional on a successful ordinary encounter, with no lure
or lead-Pokemon slot/level modifiers. They are not encounter/cast success odds.
This enumerates encounter TABLES, not map reachability. In particular water_mons
cannot certify shore access to Surf; fishing still needs an accessible bank and
a rod; rock_smash_mons may be Headbutt or Rock Smash depending on the map.
Optional, event-gated and unused map tables remain visible, never certified.

Usage from Python: model = CoverageModel(); model.records(species={...},
maps={...}, ratings=range(81)). Each record includes an exact Fraction probability.
The model keeps compact source profiles and caches slot/rating outcomes, rather
than materializing a many-million-row encounter export.
"""

import argparse
from collections import Counter
from fractions import Fraction
from functools import lru_cache
import importlib.util
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]


class CoverageModel:
    def __init__(self, root=ROOT):
        tool_path = Path(root) / "game/tools/wild_encounters/wild_encounters_to_header.py"
        spec = importlib.util.spec_from_file_location("hm_wild_projection", tool_path)
        self.tool = g = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(g)
        encounters = g.load_json(g.DEFAULT_ENCOUNTERS)
        config = g.Config(g.DEFAULT_CONFIG, g.DEFAULT_RTC, encounters)
        self.scaling = g.load_scaling(g.DEFAULT_SCALING)
        known = g.species_ids(g.DEFAULT_SPECIES)
        profiles, _ = g.validate_encounters(encounters, known, config)
        self.standard_rod = g.load_standard_rod_fishing(g.DEFAULT_STANDARD_ROD_FISHING)
        ordinary = {mon["species"] for p in profiles for method in config.mon_types
                    for mon in p["encounter"].get(method, {}).get("mons", [])}
        self.metadata = {item["species"]: item for item in g.load_species_metadata(
            g.DEFAULT_SPECIES_METADATA, g.DEFAULT_SPECIES_INFO, known, ordinary)}
        offsets = g.load_offsets(self.scaling["profile_offsets"], profiles, g.DEFAULT_SCALING)
        offset_map = {(r["product"], r["header_id"], r["area"], r["time"], r["rod"]):
                      r["level_offset"] for r in offsets}
        # The checked-in regional manifest supplies runtime DAY_ALIAS rows.
        # Avoid its unrelated ecological optimization/certificate audit here.
        regions = g.load_json(g.DEFAULT_REGIONS)["regions"]
        region_by_map = {p["map"]: region for region, data in regions.items()
                         for p in data.get("profiles", [])}
        alias_manifest = {"profiles": [p for data in regions.values()
                                       for p in data.get("profiles", []) if "nightMode" in p]}
        profiles = g.profiles_with_day_aliases(profiles, alias_manifest, config)
        self.minimum_rating, self.maximum_rating = g.trainer_rating_bounds(
            g.DEFAULT_TRAINER_RATING, self.scaling["projection_cap"])
        self.levels = {offset: tuple(tuple(g.project_level(self.scaling, level, rating, offset)
                                         for level in range(1, 101))
                                    for rating in range(self.maximum_rating + 1))
                       for offset in {0, *(r["level_offset"] for r in offsets)}}
        self.profiles = []
        for p in profiles:
            if p["product"] not in {"POKEMON_HNS", "EMERALD"}:
                continue
            for method in config.mon_types:
                if method not in p["encounter"]:
                    continue
                for rod in (g.FISHING_QUALITIES if method == "fishing_mons" else ("NONE",)):
                    identity = (p["product"], p["header_id"], g.METHOD_AREAS[method],
                                p["time"], g.RODS[rod])
                    slots, possible = [], set()
                    for _, mon, weight in g.method_slots(p, method, rod, self.standard_rod):
                        low, high = sorted((mon.get("min_level", 2), mon.get("max_level", 100)))
                        slots.append((mon["species"], low, high, weight))
                        current = mon["species"]
                        while current != "SPECIES_NONE":
                            possible.add(current)
                            current = self.metadata[current]["predecessor"]
                    self.profiles.append({
                        "map": p["map"], "label": p["label"],
                        "region": "HOENN" if p["product"] == "EMERALD" else region_by_map.get(p["map"], "HNS_UNCLASSIFIED"),
                        "product": p["product"], "method": method, "time": p["time"],
                        "rod": rod, "offset": offset_map.get(identity, 0),
                        "slots": slots, "possible_species": possible,
                        "reachability": "NOT_VERIFIED", "encounter_rate": p["encounter"][method]["encounter_rate"],
                    })

    @lru_cache(maxsize=200000)
    def _slot_outcomes(self, species, low, high, offset, rating):
        if species == "SPECIES_NONE":
            return ()
        counts = Counter()
        for vanilla in range(low, high + 1):
            level = self.levels[offset][rating][vanilla - 1]
            effective, _ = self.tool.effective_species(species, level, self.metadata)
            # Runtime rejects the WHOLE slot if any authored level is ineligible.
            if level < self.metadata[effective]["minimum_level"]:
                return ()
            counts[(effective, level)] += 1
        return tuple((species, level, count) for (species, level), count in counts.items())

    def profile_outcomes(self, profile, rating):
        """Aggregate (effective species, level) -> exact encounter probability."""
        eligible = []
        for species, low, high, weight in profile["slots"]:
            outcomes = self._slot_outcomes(species, low, high, profile["offset"], rating)
            if outcomes and weight:
                eligible.append((weight, high - low + 1, outcomes))
        total = sum(weight for weight, _, _ in eligible)
        result = Counter()
        for weight, count, outcomes in eligible:
            for species, level, occurrences in outcomes:
                result[(species, level)] += Fraction(weight * occurrences, total * count)
        return dict(result)

    def records(self, species=None, maps=None, ratings=None, methods=None, regions=None, rods=None):
        """Yield matching records; string filters or iterables are both accepted."""
        def normalize(value):
            return {value} if isinstance(value, str) else None if value is None else set(value)
        species, maps, methods, regions, rods = map(normalize, (species, maps, methods, regions, rods))
        ratings = tuple(range(self.minimum_rating, self.maximum_rating + 1) if ratings is None
                        else (ratings,) if isinstance(ratings, int) else ratings)
        if any(r < self.minimum_rating or r > self.maximum_rating for r in ratings):
            raise ValueError("Trainer Rating outside supported range")
        for p in self.profiles:
            if species is not None and not p["possible_species"] & species:
                continue
            if any(allowed is not None and p[key] not in allowed for key, allowed in
                   (("map", maps), ("method", methods), ("region", regions), ("rod", rods))):
                continue
            context = {key: p[key] for key in ("map", "label", "region", "method", "time", "rod", "reachability")}
            for rating in ratings:
                for (effective, level), probability in self.profile_outcomes(p, rating).items():
                    if species is None or effective in species:
                        yield {**context, "species": effective, "TR": rating,
                               "level": level, "probability": probability}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--species", default="SPECIES_CHINCHOU")
    parser.add_argument("--map", default="MAP_CINNABAR_ISLAND_HNS")
    parser.add_argument("--ratings", default="0,80")
    args = parser.parse_args()
    model = CoverageModel()
    for row in model.records(species=args.species, maps=args.map,
                             ratings=[int(r) for r in args.ratings.split(",")]):
        row["probability"] = str(row["probability"])
        print(json.dumps(row, sort_keys=True))


if __name__ == "__main__":
    main()
