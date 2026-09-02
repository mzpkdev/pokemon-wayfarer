import importlib.util
import copy
from fractions import Fraction
from pathlib import Path
import tempfile
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
        cls.standard_rod = GENERATOR.load_standard_rod_fishing(
            GENERATOR.DEFAULT_STANDARD_ROD_FISHING
        )
        cls.species = GENERATOR.species_ids(GENERATOR.DEFAULT_SPECIES)
        cls.profiles, cls.header_ids = GENERATOR.validate_encounters(
            cls.encounters, cls.species, cls.config
        )
        ordinary = {
            mon["species"]
            for profile in cls.profiles
            for method in cls.config.mon_types
            for mon in profile["encounter"].get(method, {}).get("mons", [])
        }
        cls.metadata = GENERATOR.load_species_metadata(
            GENERATOR.DEFAULT_SPECIES_METADATA,
            GENERATOR.DEFAULT_SPECIES_INFO,
            cls.species,
            ordinary,
        )
        cls.by_species = {row["species"]: row for row in cls.metadata}
        cls.offsets = GENERATOR.load_offsets(
            cls.scaling["profile_offsets"], cls.profiles, GENERATOR.DEFAULT_SCALING
        )
        minimum, maximum = GENERATOR.trainer_rating_bounds(
            GENERATOR.DEFAULT_TRAINER_RATING, cls.scaling["projection_cap"]
        )
        cls.cartographer = GENERATOR.build_cartographer_projection_model(
            cls.profiles,
            cls.header_ids,
            cls.config,
            cls.scaling,
            cls.metadata,
            cls.offsets,
            minimum,
            maximum,
            cls.standard_rod,
        )

    def test_standard_rod_source_is_strict_and_exact(self):
        self.assertEqual(self.standard_rod["schemaVersion"], 1)
        self.assertEqual(
            self.standard_rod["qualityWeights"],
            {
                "OLD_ROD": [38, 22, 10, 8, 8, 4, 3, 3, 2, 2],
                "GOOD_ROD": [25, 18, 12, 10, 9, 7, 6, 5, 4, 4],
                "SUPER_ROD": [12, 10, 11, 10, 10, 10, 10, 9, 9, 9],
            },
        )
        self.assertEqual(len(self.standard_rod["nativeSurfAccessibility"]), 20)
        expected_recovery = {
            ("FIRERED", label, "TIME_DAY", species, expected)
            for label in ("sPalletTown_FireRed", "sCinnabarIsland_FireRed")
            for species, expected in (("SPECIES_HORSEA", 14), ("SPECIES_KRABBY", 8))
        } | {
            ("LEAFGREEN", label, "TIME_DAY", species, expected)
            for label in ("sPalletTown_LeafGreen", "sCinnabarIsland_LeafGreen")
            for species, expected in (("SPECIES_HORSEA", 8), ("SPECIES_KRABBY", 14))
        } | {
            ("POKEMON_HNS", label, "TIME_NIGHT" if label.endswith("_Night") else "TIME_DAY", "SPECIES_CHINCHOU", 11)
            for label in (
                "gOlivineCity_PortOutside_hns_Day", "gOlivineCity_PortOutside_hns_Night",
                "gVermilionCity_hns_Day", "gVermilionCity_hns_Night",
                "gVermilionCity_PortOutside_hns_Day", "gVermilionCity_PortOutside_hns_Night",
                "gCinnabarIsland_hns_Day", "gCinnabarIsland_hns_Night",
            )
        } | {
            ("POKEMON_HNS", "gCianwoodCity_hns_Day", "TIME_DAY", "SPECIES_CHINCHOU", 12),
            ("EMERALD", "gLilycoveCity", "TIME_DAY", "SPECIES_WAILMER", 19),
            ("EMERALD", "gMossdeepCity", "TIME_DAY", "SPECIES_WAILMER", 18),
            ("EMERALD", "gPacifidlogTown", "TIME_DAY", "SPECIES_WAILMER", 18),
        }
        self.assertEqual(
            {
                (row["product"], row["baseLabel"], row["timeOfDay"], row["species"], row["expectedOldRodSuccessfulEncounterPercent"])
                for row in self.standard_rod["nativeSurfAccessibility"]
            },
            expected_recovery,
        )
        self.assertTrue(all(
            row["minimumOldRodSuccessfulEncounterPercent"] == 8
            and row["minimumOldRodUnmodifiedCastPercent"] == 2
            for row in self.standard_rod["nativeSurfAccessibility"]
        ))
        GENERATOR.validate_standard_rod_accessibility(
            self.standard_rod, self.profiles, self.species, self.config
        )

    def test_standard_rod_source_rejects_shape_and_total_drift(self):
        document = {
            "schemaVersion": 1,
            "qualityWeights": self.standard_rod["qualityWeights"],
            "nativeSurfAccessibility": self.standard_rod["nativeSurfAccessibility"],
        }
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "standard-rod.json"
            import json

            malformed = json.loads(json.dumps(document))
            malformed["qualityWeights"]["OLD_ROD"] = [50] * 10
            path.write_text(json.dumps(malformed), encoding="utf-8")
            with self.assertRaisesRegex(GENERATOR.ValidationError, "total 100"):
                GENERATOR.load_standard_rod_fishing(path)

            malformed = json.loads(json.dumps(document))
            malformed["qualityWeights"]["GOOD_ROD"].append(1)
            path.write_text(json.dumps(malformed), encoding="utf-8")
            with self.assertRaisesRegex(GENERATOR.ValidationError, "exactly ten"):
                GENERATOR.load_standard_rod_fishing(path)

    def test_source_fishing_schema_requires_all_ten_runtime_slots(self):
        encounters = copy.deepcopy(self.encounters)
        fishing = next(
            field
            for field in encounters["wild_encounter_groups"][0]["fields"]
            if field["type"] == "fishing_mons"
        )
        fishing["encounter_rates"] = fishing["encounter_rates"][:9]
        fishing["groups"]["super_rod"].remove(9)
        config = GENERATOR.Config(
            GENERATOR.DEFAULT_CONFIG, GENERATOR.DEFAULT_RTC, encounters
        )
        with self.assertRaisesRegex(GENERATOR.ValidationError, "exactly ten source slots"):
            GENERATOR.validate_encounters(encounters, self.species, config)

    def test_all_regional_giver_scripts_use_the_shared_dynamic_transaction(self):
        givers = {
            "data/maps/DewfordTown/scripts.inc": "FLAG_RECEIVED_OLD_ROD",
            "data/maps/Route118/scripts.inc": "FLAG_RECEIVED_GOOD_ROD",
            "data/maps/MossdeepCity_House3/scripts.inc": "FLAG_RECEIVED_SUPER_ROD",
            "data/maps/VermilionCity_House1_Frlg/scripts.inc": "FLAG_GOT_OLD_ROD",
            "data/maps/FuchsiaCity_House2_Frlg/scripts.inc": "FLAG_GOT_GOOD_ROD",
            "data/maps/Route12_FishingHouse_Frlg/scripts.inc": "FLAG_GOT_SUPER_ROD",
            "data/maps/Route32_PokemonCenter_hns/scripts.inc": "FLAG_STANDARD_ROD_ROUTE32_CONTRIBUTED",
            "data/maps/OlivineCity_House3_hns/scripts.inc": "FLAG_STANDARD_ROD_OLIVINE_CONTRIBUTED",
            "data/maps/Route12_House_hns/scripts.inc": "FLAG_STANDARD_ROD_ROUTE12_CONTRIBUTED",
        }
        for relative_path, flag in givers.items():
            with self.subTest(path=relative_path):
                source = (ROOT / relative_path).read_text(encoding="utf-8")
                self.assertIn(f"goto_if_set {flag},", source)
                self.assertIn("MSGBOX_YESNO", source)
                self.assertIn(f"setvar VAR_0x8004, {flag}", source)
                self.assertIn("special Script_TryAwardStandardRod", source)
                self.assertIn("STANDARD_ROD_AWARD_ALREADY_CONTRIBUTED", source)
                self.assertIn("STANDARD_ROD_AWARD_NO_SPACE", source)
                self.assertIn("STANDARD_ROD_AWARD_INVALID_STATE", source)
                self.assertIn("copyvar VAR_0x8000, VAR_0x8005", source)
                self.assertIn("call EventScript_ObtainItemMessage", source)
                self.assertIn("goto_if_eq VAR_0x8005, ITEM_OLD_ROD", source)
                self.assertIn("goto_if_eq VAR_0x8005, ITEM_GOOD_ROD", source)
                self.assertIn("CompletedSuperRod", source)
                self.assertNotIn(f"setflag {flag}", source)
                self.assertNotRegex(source, r"(?:giveitem|additem|removeitem) ITEM_(?:OLD|GOOD|SUPER)_ROD")

        route12 = (ROOT / "data/maps/Route12_FishingHouse_Frlg/scripts.inc").read_text(encoding="utf-8")
        self.assertIn("goto_if_set FLAG_GOT_SUPER_ROD, Route12_FishingHouse_EventScript_CheckMagikarpRecord", route12)
        self.assertGreaterEqual(route12.count("goto Route12_FishingHouse_EventScript_ExplainMagikarpActivity"), 2)

    def test_ordinary_generation_balance_validation_rejects_locked_recovery(self):
        GENERATOR.validate_standard_rod_balance(
            self.standard_rod,
            self.profiles,
            self.scaling,
            self.metadata,
            self.offsets,
        )
        metadata = copy.deepcopy(self.metadata)
        next(
            row for row in metadata if row["species"] == "SPECIES_WAILMER"
        )["minimum_level"] = 100
        with self.assertRaisesRegex(
            GENERATOR.ValidationError,
            "standard rod balance invariant failures:.*below successful-encounter accessibility minimum",
        ):
            GENERATOR.validate_standard_rod_balance(
                self.standard_rod,
                self.profiles,
                self.scaling,
                metadata,
                self.offsets,
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
        reviewed_floors = {
            "SPECIES_KECLEON": 20,
            "SPECIES_SKARMORY": 18,
            "SPECIES_SCYTHER": 23,
            "SPECIES_PINSIR": 23,
            "SPECIES_CHANSEY": 23,
            "SPECIES_KANGASKHAN": 25,
            "SPECIES_TAUROS": 25,
            "SPECIES_RELICANTH": 25,
            "SPECIES_SNEASEL": 30,
            "SPECIES_MANTINE": 35,
            "SPECIES_BAGON": 20,
            "SPECIES_TROPIUS": 20,
            "SPECIES_ABSOL": 20,
            "SPECIES_HERACROSS": 20,
        }
        self.assertEqual(
            {
                species: row["minimum_level"]
                for species, row in by_species.items()
                if row["minimum_level"] > 1
            },
            reviewed_floors,
        )
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
        self.assertTrue(all(row["runtimeSlotCount"] == 10 for row in fishing))
        self.assertTrue(all(
            [rating for sample in row["samples"] for rating in sample["ratings"]] == list(range(10, 81))
            for row in fishing
        ))
        self.assertEqual(len(audit["nativeSurfAccessibility"]), 20)
        self.assertEqual(audit["minimumEligibleOldRodEntryProbability"], {"numerator": 1, "denominator": 50})
        for recovery in audit["nativeSurfAccessibility"]:
            self.assertEqual([row["rating"] for row in recovery["ratings"]], list(range(10, 81)))

        def fraction(row):
            return Fraction(row["numerator"], row["denominator"])

        checked_split_outcome = False
        for row in fishing:
            for sample in row["samples"]:
                if not any(len(slot["effectiveSpeciesGivenSlotProbabilities"]) > 1 for slot in sample["slotOutcomes"]):
                    continue
                expected = {}
                for slot in sample["slotOutcomes"]:
                    if not slot["eligible"]:
                        continue
                    slot_probability = fraction(slot["lureOffSuccessfulEncounterProbability"])
                    for effective in slot["effectiveSpeciesGivenSlotProbabilities"]:
                        expected[effective["species"]] = expected.get(effective["species"], Fraction(0)) + slot_probability * fraction(effective["probability"])
                actual = {
                    species["species"]: fraction(species["lureOffSuccessfulEncounterProbability"])
                    for species in sample["aggregateSpeciesProbabilities"]
                }
                self.assertEqual(actual, expected)
                self.assertEqual(sum(actual.values(), Fraction(0)), Fraction(1))
                checked_split_outcome = True
                break
            if checked_split_outcome:
                break
        self.assertTrue(checked_split_outcome)

    def test_cartographer_projection_has_runtime_bounds_and_strict_profile_identities(self):
        projection = self.cartographer
        self.assertEqual(projection["schemaVersion"], 2)
        self.assertEqual(projection["trainerRating"], {"minimum": 10, "maximum": 80})
        self.assertEqual(projection["authoredLevel"], {"minimum": 1, "maximum": 100})
        self.assertEqual(
            projection["headerCounts"],
            {"EMERALD": 124, "FIRERED": 132, "LEAFGREEN": 132, "POKEMON_HNS": 168},
        )

        rows = projection["profiles"]
        self.assertEqual(len({row["profileKey"] for row in rows}), len(rows))
        self.assertEqual(
            len({
                (
                    row["product"], row["headerId"], row["runtimeArea"],
                    row["runtimeTime"], row["runtimeFishingRod"],
                )
                for row in rows
            }),
            len(rows),
        )
        source_profiles = {profile["label"]: profile for profile in self.profiles}
        for row in rows:
            source = source_profiles[row["baseLabel"]]
            self.assertEqual(row["product"], source["product"])
            self.assertEqual(row["map"], source["map"])
            self.assertEqual(row["header"], source["header"])
            self.assertEqual(row["headerId"], source["header_id"])
            self.assertEqual(row["runtimeTime"], source["time"])
            self.assertEqual(row["runtimeSlotCount"], len(
                GENERATOR.method_slots(source, row["method"], row["fishingRod"])
            ))
            if row["method"] == "fishing_mons":
                self.assertEqual(row["runtimeSlotCount"], 10)
                self.assertEqual(row["weights"], self.standard_rod["qualityWeights"][row["fishingRod"]])
            else:
                self.assertNotIn("weights", row)

        mt_silver = [
            row for row in rows
            if row["baseLabel"] == "gMtSilver_SnowNight_hns_Day"
        ]
        self.assertTrue(mt_silver)
        self.assertTrue(all(row["runtimeTime"] == "TIME_NIGHT" for row in mt_silver))
        self.assertTrue(all(row["header"] == "gMtSilver_SnowNight_hns" for row in mt_silver))

    def test_cartographer_level_projection_is_exhaustive_and_exact(self):
        for offset_row in self.cartographer["levelProjections"]:
            offset = offset_row["levelOffset"]
            self.assertEqual(
                [row["rating"] for row in offset_row["ratings"]],
                list(range(10, 81)),
            )
            for rating_row in offset_row["ratings"]:
                self.assertEqual(len(rating_row["projectedLevels"]), 100)
                for authored_level, projected_level in enumerate(
                    rating_row["projectedLevels"], start=1
                ):
                    self.assertEqual(
                        projected_level,
                        GENERATOR.project_level(
                            self.scaling, authored_level, rating_row["rating"], offset
                        ),
                    )

    def test_cartographer_species_intervals_are_exhaustive_and_exact(self):
        rows = {row["authoredSpecies"]: row for row in self.cartographer["species"]}
        self.assertEqual(set(rows), set(self.by_species))
        for authored_species, row in rows.items():
            covered = []
            for interval in row["outcomesByProjectedLevel"]:
                covered.extend(range(
                    interval["minimumProjectedLevel"],
                    interval["maximumProjectedLevel"] + 1,
                ))
                for level in range(
                    interval["minimumProjectedLevel"],
                    interval["maximumProjectedLevel"] + 1,
                ):
                    effective, _ = GENERATOR.effective_species(
                        authored_species, level, self.by_species
                    )
                    floor = self.by_species[effective]["minimum_level"]
                    self.assertEqual(interval["effectiveSpecies"], effective)
                    self.assertEqual(interval["minimumOrdinaryWildLevel"], floor)
                    self.assertEqual(interval["eligible"], level >= floor)
            self.assertEqual(covered, list(range(1, 101)))

    def test_cartographer_lookup_composes_to_exact_slot_semantics(self):
        levels_by_offset = {
            row["levelOffset"]: {
                rating["rating"]: rating["projectedLevels"]
                for rating in row["ratings"]
            }
            for row in self.cartographer["levelProjections"]
        }
        species_rows = {
            row["authoredSpecies"]: row["outcomesByProjectedLevel"]
            for row in self.cartographer["species"]
        }

        def projected_outcome(authored_species, projected_level):
            return next(
                interval
                for interval in species_rows[authored_species]
                if interval["minimumProjectedLevel"] <= projected_level <= interval["maximumProjectedLevel"]
            )

        profiles = {profile["label"]: profile for profile in self.profiles}
        for profile_row in self.cartographer["profiles"]:
            profile = profiles[profile_row["baseLabel"]]
            offset = profile_row["levelOffset"]
            for slot_index, mon, _ in GENERATOR.method_slots(
                profile, profile_row["method"], profile_row["fishingRod"]
            ):
                authored_minimum = mon.get("min_level", 2)
                authored_maximum = mon.get("max_level", 100)
                slot = {
                    "species": mon["species"],
                    "minimumLevel": min(authored_minimum, authored_maximum),
                    "maximumLevel": max(authored_minimum, authored_maximum),
                }
                failures = []
                summaries, _ = GENERATOR.slot_summary(
                    slot,
                    self.scaling,
                    offset,
                    self.by_species,
                    failures,
                    f"{profile_row['profileKey']}/slot {slot_index}",
                )
                self.assertEqual(failures, [])
                for rating in range(10, 81):
                    outcomes, locked = {}, False
                    for authored_level in range(slot["minimumLevel"], slot["maximumLevel"] + 1):
                        projected_level = levels_by_offset[offset][rating][authored_level - 1]
                        outcome = projected_outcome(mon["species"], projected_level)
                        locked |= not outcome["eligible"]
                        effective = outcomes.setdefault(
                            outcome["effectiveSpecies"],
                            {"minimumLevel": projected_level, "maximumLevel": projected_level},
                        )
                        effective["minimumLevel"] = min(effective["minimumLevel"], projected_level)
                        effective["maximumLevel"] = max(effective["maximumLevel"], projected_level)
                    self.assertEqual(locked, summaries[rating]["locked"])
                    self.assertEqual(outcomes, summaries[rating]["outcomes"])

    def test_cartographer_projection_supports_nonzero_profile_offsets(self):
        profile = self.profiles[0]
        method = next(method for method in self.config.mon_types if method in profile["encounter"])
        rod = "OLD_ROD" if method == "fishing_mons" else "NONE"
        offset = {
            "product": profile["product"],
            "header_id": profile["header_id"],
            "area": GENERATOR.METHOD_AREAS[method],
            "time": profile["time"],
            "rod": GENERATOR.RODS[rod],
            "level_offset": 2,
        }
        projection = GENERATOR.build_cartographer_projection_model(
            self.profiles,
            self.header_ids,
            self.config,
            self.scaling,
            self.metadata,
            [offset],
            10,
            80,
        )
        self.assertEqual(
            [row["levelOffset"] for row in projection["levelProjections"]], [0, 2]
        )
        row = next(
            row for row in projection["profiles"]
            if row["baseLabel"] == profile["label"]
            and row["method"] == method
            and row["fishingRod"] == rod
        )
        self.assertEqual(row["levelOffset"], 2)

    def test_cartographer_projection_cli_writes_deterministic_json(self):
        with tempfile.TemporaryDirectory() as directory:
            first = Path(directory) / "first.json"
            second = Path(directory) / "second.json"
            GENERATOR.generate_cartographer_projection(first)
            GENERATOR.generate_cartographer_projection(second)
            self.assertEqual(first.read_bytes(), second.read_bytes())
            self.assertLess(first.stat().st_size, 2_000_000)
            self.assertEqual(
                GENERATOR.load_json(first)["trainerRating"],
                {"minimum": 10, "maximum": 80},
            )


if __name__ == "__main__":
    unittest.main()
