"""Wild encounters v2 generator output, cross-checked against the reference model (v2/hm_model.py)."""
import json
from pathlib import Path
import re
import sys
import tempfile
import unittest

TOOLS = Path(__file__).resolve().parents[1]
ROOT = TOOLS.parents[1]
sys.path.insert(0, str(TOOLS))
sys.path.insert(0, str(TOOLS / "v2"))
import wild_encounters_to_header as generator  # noqa: E402
import v2_emit  # noqa: E402
import hm_model  # noqa: E402
import cartographer_projection  # noqa: E402

TIMES = ("TIME_MORNING", "TIME_DAY", "TIME_EVENING", "TIME_NIGHT")
INFO_MEMBERS = {"land": "landMonsInfo", "surf": "waterMonsInfo", "rock": "rockSmashMonsInfo", "fish": "fishingMonsInfo"}
SLOTS = {"land": 12, "surf": 5, "rock": 5, "fish": 10}
REACH = {"WILD_REACH_ROAD": "Road", "WILD_REACH_WILDS": "Wilds", "WILD_REACH_OUTLANDS": "Outlands", "WILD_REACH_DUNGEON": "Dungeon"}
INTENT = {value: key for key, value in v2_emit.INTENTS.items()}
REGION = {"WILD_PLACE_REGION_OTHER": "Other", "WILD_PLACE_REGION_SAFARI": "Safari", "WILD_PLACE_REGION_SINJOH": "Sinjoh"}


def build():
    species = generator.species_ids(generator.DEFAULT_SPECIES)
    arrays, headers, trailer, summary = v2_emit.render(species)
    return species, arrays, headers, trailer, summary


class V2EmitTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.species, cls.arrays, cls.headers, cls.trailer, cls.summary = build()
        cls.keys, cls.tables, cls.meta = v2_emit.load()
        cls.header_blocks = cls.headers.split("    { // ")[1:]
        place_text = cls.trailer.split("gWildEncounterPlaces[] =")[1].split("gWildProwlerMinimums[]")[0]
        cls.places = re.findall(
            r"\.reach = (\w+), \.intent = (\w+), \.flat = (\d), \.region = (\w+),\n\s+\.floor = (\d+), \.floorCount = (\d+),", place_text)
        prowler_text = cls.trailer.split("gWildProwlerMinimums[] =")[1]
        cls.prowlers = [(name, int(minimum), int(safari), int(sinjoh)) for name, minimum, safari, sinjoh in
                        re.findall(r"\{ (SPECIES_\w+), (\d+), (\d), (\d) \},", prowler_text)]

    def test_one_header_per_key_in_meta_order_with_contest_days_consecutive(self):
        self.assertEqual(len(self.header_blocks), len(self.keys))
        self.assertEqual([block.split("\n")[0] for block in self.header_blocks], self.keys)
        contest = [i for i, key in enumerate(self.keys) if key.startswith("MAP_NATIONAL_PARK_BUG_CONTEST_HNS:")]
        self.assertEqual([self.keys[i].split(":")[1] for i in contest], ["TUESDAY", "THURSDAY", "SATURDAY"])
        self.assertEqual(contest, list(range(contest[0], contest[0] + 3)))
        for i in contest:
            self.assertIn("MAP_GROUP(MAP_NATIONAL_PARK_BUG_CONTEST_HNS)", self.header_blocks[i])
        self.assertEqual(sum("MAP_GROUP(MAP_ALTERING_CAVE)" in block for block in self.header_blocks), 1)

    def test_every_header_fills_all_four_times_of_day_from_the_right_table(self):
        for key, block in zip(self.keys, self.header_blocks):
            times = dict(re.findall(r"\[(TIME_\w+)\] =\n\s+\{\n((?:\s+\.\w+ = [^\n]+\n)+)", block))
            self.assertEqual(tuple(times), TIMES, key)
            for time, body in times.items():
                table = "day" if time in ("TIME_MORNING", "TIME_DAY") else "night"
                for method, member in INFO_MEMBERS.items():
                    found = re.search(rf"\.{member} = ([^,]+),", body).group(1)
                    if method in self.tables[key]:
                        self.assertRegex(found, rf"^&\w+_{table.capitalize()}_\w+Info$", f"{key} {time} {method}")
                    else:
                        self.assertEqual(found, "NULL", f"{key} {time} {method}")

    def test_arrays_hold_the_table_species_in_order(self):
        for key in self.keys[:: max(1, len(self.keys) // 40)]:
            stem = v2_emit.label_for(key)
            for method, (_, count, suffix) in v2_emit.METHODS.items():
                if method not in self.tables[key]:
                    continue
                for time in ("day", "night"):
                    name = f"{stem}_{time.capitalize()}_{suffix}"
                    body = self.arrays.split(f"const struct WildPokemon {name}[] =")[1].split("};")[0]
                    got = re.findall(r"SPECIES_(\w+) \}", body)
                    self.assertEqual(got, self.tables[key][method][time], name)
                    self.assertEqual(len(got), SLOTS[method])
                    rate = re.search(rf"{name}Info = \{{ (\d+),", self.arrays).group(1)
                    self.assertEqual(int(rate), v2_emit.table_rate(key, method, self.meta))

    def test_rates_follow_the_method_and_the_terrain_for_every_table(self):
        found = {}
        for key, block in zip(self.keys, self.header_blocks):
            for method in v2_emit.slot_lists(key, self.tables):
                if method == "land":
                    expected = 10 if self.meta[key]["map_type"] in ("INDOOR", "UNDERGROUND") else 20
                else:
                    expected = {"surf": 4, "fish": 30, "rock": 60}[method]
                for time in ("day", "night"):
                    name = f"{v2_emit.label_for(key)}_{time.capitalize()}_{v2_emit.METHODS[method][2]}"
                    rate = int(re.search(rf"{name}Info = \{{ (\d+),", self.arrays).group(1))
                    self.assertEqual(rate, expected, name)
                found.setdefault(method, set()).add(expected)
        self.assertEqual(found, {"land": {10, 20}, "surf": {4}, "fish": {30}, "rock": {60}})

    def test_named_maps_have_their_authored_rates(self):
        # Written out by hand so the check does not follow the emitter's own rule.
        named = {
            ("MAP_ROUTE1_HNS", "land"): 20,
            ("MAP_POWER_PLANT", "land"): 10,
            ("MAP_MT_MOON_CAVE_HNS", "land"): 10,
            ("MAP_UNDERWATER_ROUTE124", "surf"): 4,
            ("MAP_ROUTE124", "fish"): 30,
            ("MAP_SNOWSWEPT_CAVERN_HNS", "rock"): 60,
        }
        for (key, method), expected in named.items():
            for time in ("day", "night"):
                name = f"{v2_emit.label_for(key)}_{time.capitalize()}_{v2_emit.METHODS[method][2]}"
                rate = int(re.search(rf"{name}Info = \{{ (\d+),", self.arrays).group(1))
                self.assertEqual(rate, expected, name)

    def test_place_records_equal_the_reference_model_for_every_map(self):
        self.assertEqual(len(self.places), len(self.keys))
        for key, (reach, intent, flat, region, floor, floor_count) in zip(self.keys, self.places):
            meta = hm_model.META[key]
            self.assertEqual(REACH[reach], meta["reach"], key)
            self.assertEqual(REGION[region], meta["region"] if meta["region"] in ("Safari", "Sinjoh") else "Other", key)
            if meta["reach"] != "Dungeon":
                self.assertEqual((intent, flat, floor, floor_count), ("WILD_DUNGEON_MILD", "0", "0", "0"), key)
                continue
            label, model_flat = hm_model.INTENTS[key]
            self.assertEqual(INTENT[intent], label, key)
            self.assertEqual(bool(int(flat)), model_flat, key)
            self.assertEqual((int(floor), int(floor_count)), (meta["floor"] or 0, meta["floors"] or 0), key)

    def test_dungeon_place_level_inputs_reproduce_the_model(self):
        # hm_model.place_level reads (intent, flat, floor, floors); the emitted record must carry exactly those.
        for key, row in zip(self.keys, self.places):
            if hm_model.META[key]["reach"] == "Dungeon":
                label, flat = hm_model.INTENTS[key]
                self.assertEqual((INTENT[row[1]], bool(int(row[2]))), (label, flat))

    def test_prowler_minimums_equal_the_reference_model_for_every_species(self):
        species_ids = [self.species["SPECIES_" + name] for name in hm_model.SPECIES]
        self.assertEqual(len(set(species_ids)), len({i for i in species_ids}))
        ids = [self.species[name] for name, *_ in self.prowlers]
        self.assertEqual(ids, sorted(ids))
        self.assertEqual(len(ids), len(set(ids)))
        by_id = {self.species[name]: (minimum, safari, sinjoh) for name, minimum, safari, sinjoh in self.prowlers}
        self.assertEqual(len(self.prowlers), self.summary["prowlers"])
        names_by_id = {}
        for name in hm_model.SPECIES:
            names_by_id.setdefault(self.species["SPECIES_" + name], []).append(name)

        def emitted(name, region):
            row = by_id.get(self.species["SPECIES_" + name])
            if row is None:
                return None
            minimum, safari, sinjoh = row
            return None if (region == "Safari" and safari) or (region == "Sinjoh" and sinjoh) else minimum

        # Alias constants of one species (FURFROU and FURFROU_NATURAL) share its id, and the model
        # resolves aliases, so every name of a species gets the emitted minimum.
        for region in ("Other", "Safari", "Sinjoh"):
            for name in hm_model.SPECIES:
                self.assertEqual(emitted(name, region), hm_model.prowler_min(name, region), f"{name} in {region}")
        self.assertEqual(sorted({minimum for minimum, _, _ in by_id.values()}), [20, 25, 30])

    def test_a_prowler_row_covers_every_non_regional_form_but_no_regional_form(self):
        by_id = {self.species[name]: minimum for name, minimum, _, _ in self.prowlers}

        def emitted(name):
            return by_id.get(self.species["SPECIES_" + name])

        for name in ("ORICORIO_PAU", "ORICORIO_POM_POM", "ORICORIO_SENSU", "ORICORIO_BAILE", "FURFROU_HEART",
                     "MIMIKYU_BUSTED", "CRAMORANT_GULPING", "INDEEDEE_F", "EISCUE_NOICE"):
            self.assertEqual(emitted(name), 20, name)
            self.assertEqual(hm_model.prowler_min(name, "Other"), 20, name)
        for name in ("VULPIX_ALOLA", "TAUROS_PALDEA_AQUA", "BASCULIN_RED_STRIPED"):
            self.assertIsNone(emitted(name), name)
            self.assertIsNone(hm_model.prowler_min(name, "Other"), name)
        # Kalos rewards keep their Safari exemption on every form.
        row = next(r for r in self.prowlers if r[0] == "SPECIES_FURFROU_HEART")
        self.assertEqual(row[2], 1)

    def test_cartographer_shows_the_rate_the_game_rolls(self):
        rolled = cartographer_projection.rolled_rate
        for method in ("land", "surf"):
            self.assertEqual([rolled("Road", method, rate) for rate in (1, 7, 20, 25, 30, 255)], [1, 4, 12, 15, 18, 153])
        for method in ("rock", "fish"):
            self.assertEqual([rolled("Road", method, rate) for rate in (1, 7, 20, 25, 30, 60, 255)], [1, 7, 20, 25, 30, 60, 255])
        for method in ("land", "surf", "rock", "fish"):
            self.assertEqual([rolled(reach, method, 30) for reach in ("Wilds", "Outlands", "Dungeon")], [30, 30, 30])

    def test_every_map_constant_exists(self):
        ids = set()
        for path in (ROOT / "data/maps").glob("*/map.json"):
            ids.add(json.loads(path.read_text(encoding="utf-8")).get("id"))
        for key in self.keys:
            self.assertIn(v2_emit.map_constant(key), ids, key)

    def test_generated_header_has_the_contract_symbols_and_no_retired_ones(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "wild_encounters.h"
            generator.generate(output_path=output)
            text = output.read_text(encoding="utf-8")
        for symbol in ("gWildMonHeaders[]", "gBattlePikeWildMonHeaders[]", "gBattlePyramidWildMonHeaders[]", "gWildEncounterPlaces[]",
                       "gWildProwlerMinimums[]", "gWildProwlerMinimumCount", "gWildMonHeaderCount", "gStandardRodFishingWeights", "ENCOUNTER_CHANCE_LAND_MONS_TOTAL"):
            self.assertIn(symbol, text)
        for symbol in ("gWildEncounterScaling", "gWildEncounterProfileOffsets", "gWildEncounterSpeciesMetadata"):
            self.assertNotIn(symbol, text)
        self.assertNotIn("_Wayfarer_", text)


if __name__ == "__main__":
    unittest.main()
