import importlib.util
from fractions import Fraction
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[3]
GENERATOR_PATH = ROOT / "tools/wild_encounters/wild_encounters_to_header.py"
SPEC = importlib.util.spec_from_file_location("wild_encounter_generator", GENERATOR_PATH)
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)


class WildEncounterScalingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.encounters = GENERATOR.load_json(GENERATOR.DEFAULT_ENCOUNTERS)
        cls.config = GENERATOR.Config(GENERATOR.DEFAULT_CONFIG, GENERATOR.DEFAULT_RTC, cls.encounters)
        cls.scaling = GENERATOR.load_scaling(GENERATOR.DEFAULT_SCALING)
        cls.species = GENERATOR.species_ids(GENERATOR.DEFAULT_SPECIES)
        cls.profiles, cls.header_ids = GENERATOR.validate_encounters(
            cls.encounters, cls.species, cls.config
        )

    def test_projection_curve_is_exact_at_required_join_points(self):
        self.assertEqual(self.scaling["projection_cap"], 80)
        self.assertEqual(
            [(row["rating"], row["level"]) for row in self.scaling["anchors"]],
            [(0, 5), (4, 6), (8, 8), (16, 13), (30, 20), (40, 32), (55, 50), (65, 70), (80, 90)],
        )
        for rating, expected in ((0, Fraction(7, 20)), (30, Fraction(3, 5)), (80, Fraction(1, 10))):
            point = self.scaling["points"][rating]
            self.assertEqual(Fraction(point["retention_numerator"], point["retention_denominator"]), expected)

    def test_profile_population_spans_all_builds(self):
        self.assertEqual(
            {product: len(headers) for product, headers in self.header_ids.items()},
            {"EMERALD": 124, "FIRERED": 132, "LEAFGREEN": 132, "POKEMON_HNS": 168},
        )

    def test_reviewed_time_binding_preserves_legacy_core_header(self):
        label = "gMtSilver_SnowNight_hns_Day"
        self.assertEqual(
            GENERATOR.time_and_header(label, self.config),
            ("TIME_NIGHT", "gMtSilver_SnowNight_hns"),
        )

        def legacy_time_and_header(value):
            time = self.config.time_fallback
            header = value
            for candidate, suffix in self.config.times.items():
                if suffix in value:
                    time = candidate
                    header = header.replace("_" + suffix, "")
            return time, header

        legacy = [
            (entry["base_label"], GENERATOR.product_for(entry["base_label"]), *legacy_time_and_header(entry["base_label"]))
            for entry in self.encounters["wild_encounter_groups"][0]["encounters"]
        ]
        current = [
            (profile["label"], profile["product"], profile["time"], profile["header"])
            for profile in self.profiles
        ]
        self.assertEqual(current, legacy)
        self.assertEqual(
            {product: len([profile for profile in self.profiles if profile["product"] == product]) for product, _ in GENERATOR.PRODUCTS},
            {"EMERALD": 124, "FIRERED": 132, "LEAFGREEN": 132, "POKEMON_HNS": 258},
        )

    def test_species_metadata_is_numeric_and_keeps_reviewed_floors(self):
        ordinary = {
            mon["species"]
            for profile in self.profiles
            for method in self.config.mon_types
            for mon in profile["encounter"].get(method, {}).get("mons", [])
        }
        metadata = GENERATOR.load_species_metadata(
            GENERATOR.DEFAULT_SPECIES_METADATA,
            GENERATOR.DEFAULT_SPECIES_INFO,
            self.species,
            ordinary,
        )
        by_species = {row["species"]: row for row in metadata}
        self.assertEqual(by_species["SPECIES_KECLEON"]["species_id"], 352)
        self.assertEqual(by_species["SPECIES_KECLEON"]["minimum_level"], 20)
        self.assertEqual(by_species["SPECIES_SKARMORY"]["minimum_level"], 18)
        self.assertEqual(by_species["SPECIES_GYARADOS"]["predecessor"], "SPECIES_MAGIKARP")
        self.assertEqual(by_species["SPECIES_GYARADOS"]["predecessor_level"], 20)
        self.assertEqual(
            GENERATOR.effective_species("SPECIES_GYARADOS", 7, by_species),
            ("SPECIES_MAGIKARP", [("SPECIES_GYARADOS", "SPECIES_MAGIKARP")]),
        )
        # A trade route does not erase a separately unambiguous numeric
        # predecessor. Runtime and audit projection must still reverse it.
        self.assertEqual(by_species["SPECIES_GOLEM"]["predecessor"], "SPECIES_GRAVELER")
        self.assertEqual(by_species["SPECIES_GOLEM"]["predecessor_level"], 38)
        self.assertTrue(by_species["SPECIES_GOLEM"]["has_alternate_non_level_route"])
        self.assertEqual(
            GENERATOR.effective_species("SPECIES_GOLEM", 14, by_species),
            ("SPECIES_GEODUDE", [("SPECIES_GOLEM", "SPECIES_GRAVELER"), ("SPECIES_GRAVELER", "SPECIES_GEODUDE")]),
        )

    def test_species_parser_rejects_ambiguous_and_cyclic_predecessors(self):
        document = {
            "schemaVersion": 1,
            "minimumOrdinaryWildLevels": [],
            "predecessorResolutions": [],
        }
        species = {"SPECIES_NONE": 0, "SPECIES_A": 1, "SPECIES_B": 2, "SPECIES_C": 3}
        ambiguous = {
            "SPECIES_A": [{"method": "EVO_LEVEL", "parameter": "10", "target": "SPECIES_B"}],
            "SPECIES_B": [],
            "SPECIES_C": [{"method": "EVO_LEVEL", "parameter": "20", "target": "SPECIES_B"}],
        }
        with self.assertRaisesRegex(GENERATOR.ValidationError, "ambiguous"):
            GENERATOR.build_species_metadata(document, ambiguous, species, {"SPECIES_B"})
        cycle = {
            "SPECIES_A": [{"method": "EVO_LEVEL", "parameter": "10", "target": "SPECIES_B"}],
            "SPECIES_B": [{"method": "EVO_LEVEL", "parameter": "20", "target": "SPECIES_A"}],
        }
        with self.assertRaisesRegex(GENERATOR.ValidationError, "cycle"):
            GENERATOR.build_species_metadata(document, cycle, species, {"SPECIES_A"})

    def test_empty_offsets_are_valid_but_zero_offsets_are_not(self):
        self.assertEqual(GENERATOR.load_offsets([], self.profiles, GENERATOR.DEFAULT_SCALING), [])
        with self.assertRaisesRegex(GENERATOR.ValidationError, "omit zero"):
            GENERATOR.load_offsets(
                [{
                    "label": self.profiles[0]["label"],
                    "method": "land_mons",
                    "fishingRod": "NONE",
                    "levelOffset": 0,
                }],
                self.profiles,
                GENERATOR.DEFAULT_SCALING,
            )

    def test_audit_covers_products_slots_and_rod_partitions(self):
        audit = GENERATOR.build_wild_encounter_balance_audit()
        self.assertTrue(audit["invariants"]["passed"], audit["invariants"]["failures"])
        products = {row["product"]: row for row in audit["products"]}
        self.assertEqual(set(products), {"Emerald", "FireRed", "LeafGreen", "HNS"})
        for product in products.values():
            self.assertGreater(product["profileCount"], 0)
            self.assertGreater(product["headerCount"], 0)
        fishing = [
            row
            for product in products.values()
            for row in product["population"]
            if row["method"] == "fishing_mons"
        ]
        self.assertEqual({row["fishingRod"] for row in fishing}, {"OLD_ROD", "GOOD_ROD", "SUPER_ROD"})
        self.assertTrue(all("slotOutcomes" in sample for row in fishing for sample in row["samples"]))


if __name__ == "__main__":
    unittest.main()
