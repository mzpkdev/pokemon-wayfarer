"""Wild encounters v2 projection for Cartographer (devtools).

Writes one JSON document describing the Wayfarer wild tables as the v2 model plays them: for
every header key (a map, or a Bug Contest day) its place (reach, dungeon intent and floor), the
rate and the day and night species per method, the place level at every Trainer Rating, and the
exact outcome distribution of every slot species at every place level it can reach.

Everything is computed by hm_model.py (the reference level model: place_level, slot_dist), so the
catalog shows what the game rolls. Nothing here touches the header emission.

Usage: cartographer_projection.py OUTPUT.json
"""
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import hm_model  # noqa: E402
import v2_emit  # noqa: E402

SCHEMA_VERSION = 3
TRAINER_RATING = (0, 160)  # the level scalers are flat past 160
METHODS = ("land", "surf", "rock", "fish")
FISHING_RODS = ("old", "good", "super")
OUTCOME_DENOMINATOR = 50  # slot_dist probabilities are multiples of 1/50


def region_class(region):
    """The only regions that change an outcome: prowler minimums are lifted in Safari and Sinjoh places."""
    return region if region in ("Safari", "Sinjoh") else "Other"


def distribution(species, place_level, region):
    """[[species, level, weight out of 50]] for one slot species at one place level, merged and sorted."""
    merged = {}
    for outcome, level, probability in hm_model.slot_dist(species, place_level, region):
        merged[(outcome, level)] = merged.get((outcome, level), 0) + probability
    rows = []
    for (outcome, level), probability in sorted(merged.items(), key=lambda item: (item[0][1], item[0][0])):
        weight = probability * OUTCOME_DENOMINATOR
        if weight.denominator != 1:
            raise SystemExit(f"{species}@{place_level}: outcome weight {probability} is not a multiple of 1/{OUTCOME_DENOMINATOR}")
        rows.append([outcome, level, int(weight)])
    return rows


def build():
    keys, tables, meta, rates, defaults = v2_emit.load()
    low, high = TRAINER_RATING
    sets, needed = [], {}
    for key in keys:
        entry = meta[key]
        reach, region = entry["reach"], entry["region"]
        intent = None
        if reach == "Dungeon":
            intent = {"intent": hm_model.INTENTS[v2_emit.map_constant(key)][0],
                      "flat": hm_model.INTENTS[v2_emit.map_constant(key)][1]}
        levels = [hm_model.place_level(key, rating) for rating in range(low, high + 1)]
        methods = {}
        for method, table in v2_emit.slot_lists(key, tables).items():
            day_rate, night_rate, defaulted = v2_emit.table_rate(key, method, rates, defaults)
            methods[method] = {"rate": {"day": day_rate, "night": night_rate}, "defaultRate": defaulted,
                               "day": table["day"], "night": table["night"]}
            for species in table["day"] + table["night"]:
                if species != "NONE":
                    needed.setdefault((species, region_class(region)), set()).update(levels)
        sets.append({
            "key": key,
            "map": v2_emit.map_constant(key),
            "baseLabel": v2_emit.label_for(key),
            "variant": key.split(":")[1] if ":" in key else None,
            "sourceFile": f"{region.lower()}.json",
            "place": {"name": entry["place"], "region": region, "regionClass": region_class(region), "reach": reach,
                      "dungeon": intent, "floor": entry["floor"], "floors": entry["floors"]},
            "placeLevels": levels,
            "methods": methods,
        })
    outcomes = []
    for species, cls in sorted(needed):
        by_level = {str(level): distribution(species, level, cls) for level in sorted(needed[(species, cls)])}
        outcomes.append({"species": species, "regionClass": cls, "byPlaceLevel": by_level})
    return {
        "schemaVersion": SCHEMA_VERSION,
        "trainerRating": {"minimum": low, "maximum": high},
        "outcomeDenominator": OUTCOME_DENOMINATOR,
        "weights": {"land": hm_model.WEIGHTS["land"], "surf": hm_model.WEIGHTS["surf"], "rock": hm_model.WEIGHTS["rock"],
                    "fish": hm_model.RODS},
        "sets": sets,
        "outcomes": outcomes,
    }


def main(argv):
    if len(argv) != 2:
        raise SystemExit(__doc__)
    path = Path(argv[1])
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(build(), separators=(",", ":"), ensure_ascii=False) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main(sys.argv)
