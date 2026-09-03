import importlib.util
import copy
from fractions import Fraction
import itertools
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
            {"EMERALD": 124, "FIRERED": 132, "LEAFGREEN": 132, "POKEMON_HNS": 169},
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
            {"EMERALD": 124, "FIRERED": 132, "LEAFGREEN": 132, "POKEMON_HNS": 271},
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

    def test_generation_classification_uses_base_national_dex(self):
        nat_dex = GENERATOR.active_national_dex(GENERATOR.DEFAULT_SPECIES_INFO)
        self.assertEqual(nat_dex["SPECIES_BULBASAUR"], 1)
        self.assertEqual(nat_dex["SPECIES_RATTATA_ALOLA"], 19)
        classified = {
            "SPECIES_BULBASAUR": {"national_dex": 1},
            "SPECIES_CHIKORITA": {"national_dex": 152},
            "SPECIES_TREECKO": {"national_dex": 252},
            "SPECIES_TURTWIG": {"national_dex": 387},
            "SPECIES_WYNAUT": {"national_dex": 360},
        }
        self.assertEqual(GENERATOR.generation_for("SPECIES_BULBASAUR", classified), "GENERATION_I")
        self.assertEqual(GENERATOR.generation_for("SPECIES_CHIKORITA", classified), "GENERATION_II")
        self.assertEqual(GENERATOR.generation_for("SPECIES_TREECKO", classified), "INDEPENDENT_GENERATION_III")
        self.assertEqual(GENERATOR.generation_for("SPECIES_TURTWIG", classified), "GENERATION_IV_ONWARD")
        self.assertEqual(GENERATOR.generation_for("SPECIES_WYNAUT", classified, effective=True), "GENERATION_II_FAMILY_EXTENSION")

    def test_source_range_selection_uses_midpoint_maximum_and_version_ties(self):
        self.assertEqual(
            GENERATOR.select_source_level_range(
                {"min_level": 4, "max_level": 8}, {"min_level": 3, "max_level": 8}
            ),
            {"version": "LEAFGREEN", "minLevel": 3, "maxLevel": 8},
        )
        self.assertEqual(
            GENERATOR.select_source_level_range(
                {"min_level": 4, "max_level": 8}, {"min_level": 3, "max_level": 9}
            ),
            {"version": "FIRERED", "minLevel": 4, "maxLevel": 8},
        )
        self.assertEqual(
            GENERATOR.select_source_level_range(
                {"min_level": 3, "max_level": 8}, {"min_level": 3, "max_level": 8}
            ),
            {"version": "FIRERED", "minLevel": 3, "maxLevel": 8},
        )

    def test_night_retention_and_distance_boundaries_are_exact(self):
        passing = GENERATOR.day_night_metrics(
            {"A": Fraction(7, 10), "B": Fraction(3, 10)},
            {"A": Fraction(7, 10), "C": Fraction(3, 10)},
        )
        self.assertEqual(passing, {"dayShared": Fraction(7, 10), "nightShared": Fraction(7, 10), "distance": Fraction(3, 10)})
        boundary = GENERATOR.day_night_metrics(
            {"A": Fraction(4, 5), "B": Fraction(1, 5)},
            {"A": Fraction(7, 10), "B": Fraction(3, 10)},
        )
        self.assertEqual(boundary["distance"], Fraction(1, 10))

    def test_discrete_counterpart_candidates_use_all_tie_breaks(self):
        candidates = GENERATOR.discrete_counterpart_candidates(
            [0, 1, 2], [50, 25, 25], ("SPECIES_A", "SPECIES_B"), Fraction(1)
        )
        self.assertEqual(candidates[0][0], Fraction(0))
        self.assertEqual(candidates[0][1], Fraction(0))
        self.assertEqual(candidates[0][2], (0,))
        self.assertEqual(candidates[0][3], ("SPECIES_A", "SPECIES_B"))

    def test_counterpart_solver_attempts_full_retention_then_ranks_reduced_choice(self):
        full = GENERATOR.solve_counterpart_assignment(
            {"NONE": [60, 30, 5, 4, 1]}, {"NONE": Fraction(9, 10)},
            ("SPECIES_B", "SPECIES_A"), (0, 1),
        )
        self.assertEqual(full["selectedCategory"], "fullStrict")
        self.assertEqual(full["selectedRank"]["distinctSourceSpeciesRetained"], 2)
        reduced = GENERATOR.solve_counterpart_assignment(
            {"OLD_ROD": [38, 22, 10, 8, 8, 4, 3, 3, 2, 2],
             "GOOD_ROD": [25, 18, 12, 10, 9, 7, 6, 5, 4, 4],
             "SUPER_ROD": [12, 10, 11, 10, 10, 10, 10, 9, 9, 9]},
            {"OLD_ROD": Fraction(2, 25), "GOOD_ROD": Fraction(9, 100), "SUPER_ROD": Fraction(1, 10)},
            ("SPECIES_KRABBY", "SPECIES_HORSEA"), (4,),
        )
        self.assertEqual(reduced["selectedCategory"], "reducedBudget")
        self.assertEqual(
            reduced["selectedAssignment"],
            {"SPECIES_HORSEA": [4], "SPECIES_KRABBY": []},
        )
        self.assertEqual(reduced["selectedRank"]["combinedBudgetError"], {"numerator": 0, "denominator": 1})
        self.assertTrue(all(
            row["candidateKind"] == "FULL_RETENTION" or row["combinedBudgetSatisfied"]
            for row in reduced["enumeratedCandidates"]
        ))
        self.assertEqual(reduced["exhaustiveCandidateCounts"]["fullStrict"], 0)
        self.assertEqual(reduced["exhaustiveCandidateCounts"]["fullBudget"], 0)
        self.assertEqual(
            sum(row["selectionReason"] == "SELECTED_BY_ORDERED_OBJECTIVES" for row in reduced["enumeratedCandidates"]),
            1,
        )

    def test_profile_solver_remaps_all_slots_and_proves_chinchou_is_binding(self):
        weights = {"NONE": [20, 20, 20, 20, 20]}
        groups = [
            {"sourceSlots": [0, 1], "species": ["SPECIES_A", "SPECIES_B"]},
            {"sourceSlots": [2], "species": ["SPECIES_C", "SPECIES_C"]},
            {"sourceSlots": [3], "species": ["SPECIES_D", "SPECIES_D"]},
            {"sourceSlots": [4], "species": ["SPECIES_E", "SPECIES_E"]},
        ]
        remapped = GENERATOR.solve_profile_counterpart_assignment(weights, groups)
        self.assertEqual(remapped["selectedCategory"], "fullStrict")
        self.assertGreater(remapped["dpCertificate"]["candidateCounts"]["fullStrict"], 0)
        self.assertTrue(all(
            any(value.get("canonicalGroup") == group["id"] for domain in remapped["candidateDomains"] for value in domain)
            for group in remapped["canonicalGroups"]
        ))

        protected = GENERATOR.solve_profile_counterpart_assignment(weights, groups, {4})
        self.assertEqual(protected["dpCertificate"]["candidateCounts"]["fullStrict"], 0)
        self.assertEqual(protected["dpCertificate"]["candidateCounts"]["fullBudget"], 0)
        self.assertEqual(protected["selectedCategory"], "reducedBudget")
        self.assertEqual(protected["fixedConstraints"]["protectedChinchouTargetSlots"], [4])
        self.assertEqual(protected["candidateDomains"][4], [{"state": "UNASSIGNED"}])

    def test_profile_solver_slot_tie_keeps_distinct_prefix_lengths(self):
        weights = {"NONE": [25, 25, 25, 25]}
        raw_groups = [
            {"sourceSlots": [0, 1], "species": ["SPECIES_B", "SPECIES_C"]},
            {"sourceSlots": [2, 3], "species": ["SPECIES_A", "SPECIES_A"]},
        ]
        solved = GENERATOR.solve_profile_counterpart_assignment(weights, raw_groups)
        groups = solved["_groups"]
        domains = [[None, *[(group, species) for group, row in enumerate(groups) for species in range(len(row["species"]))]]] * 4
        candidates = []
        for assignment in itertools.product(*domains):
            metrics = GENERATOR._evaluate_profile_assignment(assignment, groups, ("NONE",), weights)
            if metrics["full"] and metrics["budgetPass"] and metrics["ratioPass"]:
                candidates.append((GENERATOR._profile_assignment_rank(assignment, groups, metrics), assignment))
        expected = min(candidates, key=lambda row: row[0])[1]
        self.assertEqual(solved["_selectedRaw"], expected)

    def test_profile_solver_counts_one_percent_empty_group_candidate_exactly(self):
        weights = {"NONE": [1, 1, 1, 97]}
        raw_groups = [{
            "sourceSlots": [0],
            "species": ["SPECIES_A", "SPECIES_B"],
        }]
        solved = GENERATOR.solve_profile_counterpart_assignment(weights, raw_groups)
        self.assertEqual(solved["dpCertificate"]["totalCandidateCount"], 81)
        self.assertEqual(
            solved["dpCertificate"]["candidateCounts"],
            {
                "fullStrict": 12,
                "fullBudget": 0,
                "fullRejectedBudget": 38,
                "reducedBudget": 15,
                "reducedRejectedFixedConstraints": 16,
            },
        )
        self.assertEqual(solved["selectedCategory"], "fullStrict")
        self.assertEqual(
            solved["dpCertificate"]["rejectionCounts"],
            {
                "fullCombinedBudgetFailure": 38,
                "reducedCombinedBudgetFailure": 16,
                "reducedRetainedPairRatioFailure": 0,
            },
        )

        groups = solved["_groups"]
        brute_counts = {
            "fullStrict": 0,
            "fullBudget": 0,
            "fullRejectedBudget": 0,
            "reducedBudget": 0,
            "reducedRejectedFixedConstraints": 0,
        }
        for assignment in itertools.product((None, (0, 0), (0, 1)), repeat=4):
            metrics = GENERATOR._evaluate_profile_assignment(
                assignment, groups, ("NONE",), weights
            )
            if metrics["full"] and metrics["budgetPass"] and metrics["ratioPass"]:
                category = "fullStrict"
            elif metrics["full"] and metrics["budgetPass"]:
                category = "fullBudget"
            elif metrics["full"]:
                category = "fullRejectedBudget"
            elif metrics["budgetPass"] and metrics["ratioPass"]:
                category = "reducedBudget"
            else:
                category = "reducedRejectedFixedConstraints"
            brute_counts[category] += 1
        self.assertEqual(solved["dpCertificate"]["candidateCounts"], brute_counts)

        certificate = solved["dpCertificate"]
        self.assertEqual(
            certificate["fullRetentionCounting"]["method"],
            "inclusion-exclusion over distinct-source-species and differing-group-member features",
        )
        self.assertGreater(certificate["completionTransitionClassCount"], 0)
        self.assertEqual(len(certificate["canonicalEnumerationDigest"]), 64)
        self.assertTrue(certificate["layers"])
        self.assertTrue(all("transitionDigest" in layer for layer in certificate["layers"]))

    def test_profile_solver_compaction_matches_leaf_enumeration(self):
        fixtures = [
            (
                {"NONE": [40, 30, 20, 10]},
                [
                    {"sourceSlots": [0, 1], "species": ["SPECIES_A", "SPECIES_B"]},
                    {"sourceSlots": [2], "species": ["SPECIES_C", "SPECIES_C"]},
                    {"sourceSlots": [3], "species": ["SPECIES_C", "SPECIES_C"]},
                ],
                set(),
            ),
            (
                {
                    "OLD_ROD": [45, 30, 15, 10],
                    "SUPER_ROD": [10, 20, 30, 40],
                },
                [
                    {"sourceSlots": [0, 1], "species": ["SPECIES_A", "SPECIES_B"]},
                    {"sourceSlots": [2, 3], "species": ["SPECIES_C", "SPECIES_D"]},
                ],
                set(),
            ),
            (
                {"NONE": [80, 10, 5, 5]},
                [{"sourceSlots": [0, 1, 2, 3], "species": ["SPECIES_A", "SPECIES_B"]}],
                set(),
            ),
        ]
        saw_ratio_only = False
        for weights, raw_groups, protected in fixtures:
            solved = GENERATOR.solve_profile_counterpart_assignment(
                weights, raw_groups, protected
            )
            groups = solved["_groups"]
            qualities = tuple(sorted(weights))
            domain = [None, *[
                (group_index, species_index)
                for group_index, group in enumerate(groups)
                for species_index in range(len(group["species"]))
            ]]
            domains = [
                [None] if slot in protected else domain
                for slot in range(len(next(iter(weights.values()))))
            ]
            counts = {
                "fullStrict": 0,
                "fullBudget": 0,
                "fullRejectedBudget": 0,
                "reducedBudget": 0,
                "reducedRejectedFixedConstraints": 0,
            }
            winners = {"fullStrict": [], "fullBudget": [], "reducedBudget": []}
            for assignment in itertools.product(*domains):
                metrics = GENERATOR._evaluate_profile_assignment(
                    assignment, groups, qualities, weights
                )
                category = GENERATOR._profile_assignment_category(metrics)
                counts[category] += 1
                if category in winners:
                    winners[category].append((
                        GENERATOR._profile_assignment_rank(assignment, groups, metrics),
                        assignment,
                    ))
            preferred = next(category for category in (
                "fullStrict", "fullBudget", "reducedBudget"
            ) if winners[category])
            expected = min(winners[preferred], key=lambda item: item[0])
            self.assertEqual(solved["dpCertificate"]["candidateCounts"], counts)
            self.assertEqual(solved["selectedCategory"], preferred)
            self.assertEqual(solved["_selectedRaw"], expected[1])
            saw_ratio_only |= preferred == "fullBudget"
        self.assertTrue(saw_ratio_only)

    def test_profile_solver_shared_duplicate_can_improve_slot_tie(self):
        weights = {"NONE": [2, 2, 1]}
        raw_groups = [
            {"sourceSlots": [2], "species": ["SPECIES_A", "SPECIES_B"]},
            {"sourceSlots": [1, 0, 2], "species": ["SPECIES_C", "SPECIES_C"]},
        ]
        solved = GENERATOR.solve_profile_counterpart_assignment(weights, raw_groups)
        self.assertEqual(solved["selectedCategory"], "reducedBudget")
        self.assertEqual(solved["selectedRank"]["slotSequence"], [0, 1, 2])

        groups = solved["_groups"]
        domain = [None, *[
            (group_index, species_index)
            for group_index, group in enumerate(groups)
            for species_index in range(len(group["species"]))
        ]]
        brute = []
        for assignment in itertools.product(domain, repeat=3):
            metrics = GENERATOR._evaluate_profile_assignment(
                assignment, groups, ("NONE",), weights
            )
            if GENERATOR._profile_assignment_category(metrics) != "reducedBudget":
                continue
            brute.append((
                GENERATOR._profile_assignment_rank(assignment, groups, metrics),
                assignment,
            ))
        expected_rank, expected_assignment = min(brute, key=lambda item: item[0])
        self.assertEqual(solved["selectedRank"]["slotSequence"], list(expected_rank[3]))
        self.assertEqual(solved["_selectedRaw"], expected_assignment)

    def test_forbidden_species_distinguishes_authored_babies_from_effective_extensions(self):
        classified = {
            "SPECIES_WYNAUT": {"national_dex": 360},
            "SPECIES_AZURILL": {"national_dex": 298},
            "SPECIES_TREECKO": {"national_dex": 252},
        }
        self.assertTrue(GENERATOR.is_forbidden_kanto_species("SPECIES_WYNAUT", classified))
        self.assertFalse(GENERATOR.is_forbidden_kanto_species("SPECIES_WYNAUT", classified, effective=True))
        self.assertTrue(GENERATOR.is_forbidden_kanto_species("SPECIES_AZURILL", classified))
        self.assertFalse(GENERATOR.is_forbidden_kanto_species("SPECIES_AZURILL", classified, effective=True))
        self.assertTrue(GENERATOR.is_forbidden_kanto_species("SPECIES_TREECKO", classified, effective=True))

    def test_kanto_manifest_covers_direct_equivalent_and_analog_sources(self):
        manifest = GENERATOR.load_regional_manifest(
            GENERATOR.DEFAULT_REGIONS, self.profiles, self.config, self.species
        )
        by_identity = {(row["map"], row["method"]): row for row in manifest["profiles"]}
        self.assertEqual(by_identity[("MAP_ROUTE1_HNS", "land_mons")]["sourceKind"], "DIRECT")
        self.assertEqual(by_identity[("MAP_MT_MOON_CAVE_HNS", "land_mons")]["sourceKind"], "EQUIVALENT")
        self.assertEqual(by_identity[("MAP_ROUTE1_HNS", "water_mons")]["sourceKind"], "ANALOG")
        self.assertEqual(len(manifest["profiles"]), 129)
        self.assertEqual(
            [(row["map"], row["method"], row["time"], row["slot"], row["afterSpecies"]) for row in manifest["changes"]],
            sorted((row["map"], row["method"], row["time"], row["slot"], row["afterSpecies"]) for row in manifest["changes"]),
        )

    def test_kanto_manifest_rejects_incomplete_group_counterpart_and_rate_drift(self):
        document = GENERATOR.load_json(GENERATOR.DEFAULT_REGIONS)
        nat_dex = GENERATOR.active_national_dex(GENERATOR.DEFAULT_SPECIES_INFO)

        incomplete = copy.deepcopy(document)
        provenance = next(
            record
            for profile in incomplete["regions"]["KANTO"]["profiles"]
            for record in profile["provenance"]
            if len(record["ecologySourceGroup"]["fireRedSlots"]) > 1
        )
        provenance["ecologySourceGroup"]["fireRedSlots"].pop()
        provenance["ecologySourceGroup"]["leafGreenSlots"].pop()
        with self.assertRaisesRegex(GENERATOR.ValidationError, "omits or adds paired ecology roles"):
            GENERATOR.validate_regional_manifest(
                incomplete, self.profiles, self.config, self.species,
                GENERATOR.DEFAULT_REGIONS, nat_dex,
            )

        encounters = copy.deepcopy(self.encounters)
        target = next(
            row for row in encounters["wild_encounter_groups"][0]["encounters"]
            if row["base_label"] == "gRoute1_hns_Day"
        )
        target["land_mons"]["encounter_rate"] += 1
        config = GENERATOR.Config(GENERATOR.DEFAULT_CONFIG, GENERATOR.DEFAULT_RTC, encounters)
        profiles, _ = GENERATOR.validate_encounters(encounters, self.species, config)
        with self.assertRaisesRegex(GENERATOR.ValidationError, "frozen Kanto topology or encounter rates changed"):
            GENERATOR.validate_regional_manifest(
                document, profiles, config, self.species,
                GENERATOR.DEFAULT_REGIONS, nat_dex,
            )

        missing = copy.deepcopy(document)
        missing["regions"]["KANTO"]["profiles"].pop()
        with self.assertRaisesRegex(GENERATOR.ValidationError, "Kanto map ownership mismatch"):
            GENERATOR.validate_regional_manifest(
                missing, self.profiles, self.config, self.species,
                GENERATOR.DEFAULT_REGIONS, nat_dex,
            )

    def test_kanto_manifest_rejects_invalid_counterpart_and_change_ledger(self):
        document = GENERATOR.load_json(GENERATOR.DEFAULT_REGIONS)
        nat_dex = GENERATOR.active_national_dex(GENERATOR.DEFAULT_SPECIES_INFO)
        encounters = copy.deepcopy(self.encounters)
        manifest_profile = next(
            row for row in document["regions"]["KANTO"]["profiles"]
            if any(
                record["targetTime"] == "DAY"
                and record["reason"] == "FRLG_VERSION_COUNTERPART"
                for record in row["provenance"]
            )
        )
        record = next(
            record for record in manifest_profile["provenance"]
            if record["targetTime"] == "DAY" and record["reason"] == "FRLG_VERSION_COUNTERPART"
        )
        target = next(
            row for row in encounters["wild_encounter_groups"][0]["encounters"]
            if row["base_label"] == manifest_profile["dayBaseLabel"]
        )
        target[manifest_profile["method"]]["mons"][record["targetSlot"]]["species"] = "SPECIES_MEW"
        config = GENERATOR.Config(GENERATOR.DEFAULT_CONFIG, GENERATOR.DEFAULT_RTC, encounters)
        profiles, _ = GENERATOR.validate_encounters(encounters, self.species, config)
        with self.assertRaisesRegex(GENERATOR.ValidationError, "target species does not match its FRLG ecology source group"):
            GENERATOR.validate_regional_manifest(
                document, profiles, config, self.species,
                GENERATOR.DEFAULT_REGIONS, nat_dex,
            )

        duplicate_species = copy.deepcopy(document)
        duplicate_species_encounters = copy.deepcopy(self.encounters)
        duplicate_species_profile = next(
            row for row in duplicate_species["regions"]["KANTO"]["profiles"]
            if any(record["targetTime"] == "DAY" and record["reason"] == "FRLG_SHARED"
                   for record in row["provenance"])
        )
        duplicate_species_record = next(
            record for record in duplicate_species_profile["provenance"]
            if record["targetTime"] == "DAY" and record["reason"] == "FRLG_SHARED"
        )
        duplicate_species_record["reason"] = "FRLG_DUPLICATE"
        duplicate_species_target = next(
            row for row in duplicate_species_encounters["wild_encounter_groups"][0]["encounters"]
            if row["base_label"] == duplicate_species_profile["dayBaseLabel"]
        )
        duplicate_species_target[duplicate_species_profile["method"]]["mons"][
            duplicate_species_record["targetSlot"]
        ]["species"] = "SPECIES_MEW"
        duplicate_species_config = GENERATOR.Config(
            GENERATOR.DEFAULT_CONFIG, GENERATOR.DEFAULT_RTC, duplicate_species_encounters
        )
        duplicate_species_profiles, _ = GENERATOR.validate_encounters(
            duplicate_species_encounters, self.species, duplicate_species_config
        )
        with self.assertRaisesRegex(GENERATOR.ValidationError, "target species does not match its FRLG ecology source group"):
            GENERATOR.validate_regional_manifest(
                duplicate_species, duplicate_species_profiles, duplicate_species_config,
                self.species, GENERATOR.DEFAULT_REGIONS, nat_dex,
            )

        before = copy.deepcopy(document)
        before["regions"]["KANTO"]["changes"][0]["beforeSpecies"] = "SPECIES_MEW"
        with self.assertRaisesRegex(GENERATOR.ValidationError, "beforeSpecies ledger does not match"):
            GENERATOR.validate_regional_manifest(
                before, self.profiles, self.config, self.species,
                GENERATOR.DEFAULT_REGIONS, nat_dex,
            )
        after = copy.deepcopy(document)
        after["regions"]["KANTO"]["changes"][0]["afterSpecies"] = "SPECIES_MEW"
        with self.assertRaisesRegex(GENERATOR.ValidationError, "does not match wild_encounters.json"):
            GENERATOR.validate_regional_manifest(
                after, self.profiles, self.config, self.species,
                GENERATOR.DEFAULT_REGIONS, nat_dex,
            )
        illegal = copy.deepcopy(document)
        merge = next(row for row in illegal["regions"]["KANTO"]["changes"] if row["changeKind"] == "MERGE")
        merge["changeKind"] = "ADDITION"
        with self.assertRaisesRegex(GENERATOR.ValidationError, "addition lacks addition provenance"):
            GENERATOR.validate_regional_manifest(
                illegal, self.profiles, self.config, self.species,
                GENERATOR.DEFAULT_REGIONS, nat_dex,
            )

        duplicate_level = copy.deepcopy(document)
        duplicate_encounters = copy.deepcopy(self.encounters)
        duplicate_profile = next(
            row for row in duplicate_level["regions"]["KANTO"]["profiles"]
            if any(record["reason"] == "FRLG_SHARED"
                   for record in row["provenance"] if record["targetTime"] == "DAY")
        )
        duplicate_record = next(
            record for record in duplicate_profile["provenance"]
            if record["targetTime"] == "DAY"
            and record["reason"] == "FRLG_SHARED"
        )
        duplicate_record["reason"] = "FRLG_DUPLICATE"
        duplicate_target = next(
            row for row in duplicate_encounters["wild_encounter_groups"][0]["encounters"]
            if row["base_label"] == duplicate_profile["dayBaseLabel"]
        )
        duplicate_mon = duplicate_target[duplicate_profile["method"]]["mons"][duplicate_record["targetSlot"]]
        duplicate_mon["min_level"] = duplicate_mon.get("min_level", 2) + 1
        duplicate_config = GENERATOR.Config(
            GENERATOR.DEFAULT_CONFIG, GENERATOR.DEFAULT_RTC, duplicate_encounters
        )
        duplicate_profiles, _ = GENERATOR.validate_encounters(
            duplicate_encounters, self.species, duplicate_config
        )
        with self.assertRaisesRegex(GENERATOR.ValidationError, "target range does not match selected source range"):
            GENERATOR.validate_regional_manifest(
                duplicate_level, duplicate_profiles, duplicate_config, self.species,
                GENERATOR.DEFAULT_REGIONS, nat_dex,
            )

    def test_kanto_manifest_rejects_solver_tie_break_and_dropped_shared_anchor(self):
        document = GENERATOR.load_json(GENERATOR.DEFAULT_REGIONS)
        route1 = next(
            row for row in document["regions"]["KANTO"]["profiles"]
            if row["map"] == "MAP_ROUTE1_HNS" and row["method"] == "fishing_mons"
        )
        copied_profiles = copy.deepcopy(self.profiles)
        copied_by_label = {row["label"]: row for row in copied_profiles}
        copied_day = copied_by_label[route1["dayBaseLabel"]]
        selected_proof = GENERATOR.validate_kanto_day_counterparts(
            route1,
            {row["label"]: row for row in self.profiles}[route1["dayBaseLabel"]],
            {row["label"]: row for row in self.profiles},
            self.standard_rod,
            "fixture",
            enforce=False,
        )[0]
        groups = {
            group["id"]: group for group in selected_proof["canonicalGroups"]
        }
        selected_counterpart = next(
            item for item in selected_proof["selectedAssignment"]
            if item["state"] == "SOURCE" and groups[item["canonicalGroup"]]["differing"]
        )
        nonselected_species = next(
            species for species in groups[selected_counterpart["canonicalGroup"]]["sourceSpecies"]
            if species != selected_counterpart["species"]
        )
        copied_day["encounter"]["fishing_mons"]["mons"][
            selected_counterpart["targetSlot"]
        ]["species"] = nonselected_species
        with self.assertRaisesRegex(GENERATOR.ValidationError, "deterministic profile solver choice"):
            GENERATOR.validate_kanto_day_counterparts(
                route1, copied_day, copied_by_label, self.standard_rod, "fixture"
            )

        duplicate_mask = copy.deepcopy(route1)
        next(
            record for record in duplicate_mask["provenance"]
            if record["targetTime"] == "DAY"
            and record["targetSlot"] == selected_counterpart["targetSlot"]
        )["reason"] = "FRLG_DUPLICATE"
        with self.assertRaisesRegex(GENERATOR.ValidationError, "FRLG_DUPLICATE must use a shared"):
            GENERATOR.validate_kanto_day_counterparts(
                duplicate_mask,
                {row["label"]: row for row in self.profiles}[route1["dayBaseLabel"]],
                {row["label"]: row for row in self.profiles},
                self.standard_rod,
                "fixture",
            )

        hidden_counterpart = copy.deepcopy(route1)
        next(
            record for record in hidden_counterpart["provenance"]
            if record["targetTime"] == "DAY"
            and record["targetSlot"] == selected_counterpart["targetSlot"]
        )["reason"] = "NIGHT_REWEIGHT"
        with self.assertRaisesRegex(GENERATOR.ValidationError, "deterministic profile solver choice"):
            GENERATOR.validate_kanto_day_counterparts(
                hidden_counterpart,
                {row["label"]: row for row in self.profiles}[route1["dayBaseLabel"]],
                {row["label"]: row for row in self.profiles},
                self.standard_rod,
                "fixture",
            )

        selected_shared = next(
            item for item in selected_proof["selectedAssignment"]
            if item["state"] == "SOURCE" and not groups[item["canonicalGroup"]]["differing"]
        )
        shared_source_slots = groups[selected_shared["canonicalGroup"]]["sourceSlots"]
        missing_anchor = copy.deepcopy(route1)
        for provenance in missing_anchor["provenance"]:
            if (
                provenance["targetTime"] == "DAY"
                and provenance["ecologySourceGroup"]["fireRedSlots"] == shared_source_slots
                and provenance["reason"] in {"FRLG_SHARED", "FRLG_VERSION_COUNTERPART"}
            ):
                provenance["reason"] = "FRLG_DUPLICATE"
        with self.assertRaisesRegex(GENERATOR.ValidationError, "requires a selected shared source allocation"):
            GENERATOR.validate_kanto_day_counterparts(
                missing_anchor,
                {row["label"]: row for row in self.profiles}[route1["dayBaseLabel"]],
                {row["label"]: row for row in self.profiles},
                self.standard_rod,
                "fixture",
            )

        # Route 3's direct source profiles are unique to this manifest row. Turn
        # its Nidoran pair into Spearow/Nidoran_M without changing any source
        # slots or weights. The authored day remains a strict full-retention
        # witness because Spearow is then retained through that differing group.
        # Replacing the sole local Spearow anchor with a valid Generation II
        # addition must therefore reach the independent local-addition guard.
        route3 = next(
            row for row in document["regions"]["KANTO"]["profiles"]
            if row["map"] == "MAP_ROUTE3_HNS" and row["method"] == "land_mons"
        )
        fixture_profiles = copy.deepcopy(self.profiles)
        fixture_by_label = {row["label"]: row for row in fixture_profiles}
        leaf_source = fixture_by_label[route3["leafGreenSource"][0]]
        for slot in (4, 8):
            leaf_source["encounter"]["land_mons"]["mons"][slot]["species"] = "SPECIES_SPEAROW"
        for time, label in (("DAY", route3["dayBaseLabel"]), ("NIGHT", route3["nightBaseLabel"])):
            fixture_by_label[label]["encounter"]["land_mons"]["mons"][2]["species"] = "SPECIES_SPEAROW"
            next(
                change for change in document["regions"]["KANTO"]["changes"]
                if (change["map"], change["method"], change["time"], change["slot"])
                == (route3["map"], route3["method"], time, 2)
            )["afterSpecies"] = "SPECIES_SPEAROW"

        fixture_by_label[route3["dayBaseLabel"]]["encounter"]["land_mons"]["mons"][4]["species"] = "SPECIES_HOOTHOOT"
        next(
            provenance for provenance in route3["provenance"]
            if provenance["targetTime"] == "DAY" and provenance["targetSlot"] == 4
        )["reason"] = "GEN2_LOCAL_ADDITION"
        removed_anchor = next(
            change for change in document["regions"]["KANTO"]["changes"]
            if (change["map"], change["method"], change["time"], change["slot"])
            == (route3["map"], route3["method"], "DAY", 4)
        )
        removed_anchor.update({
            "afterSpecies": "SPECIES_HOOTHOOT",
            "changeKind": "ADDITION",
            "reason": "negative fixture removes the local shared anchor",
        })

        with self.assertRaisesRegex(
            GENERATOR.ValidationError,
            "addition removes the last shared FRLG species occurrence",
        ):
            GENERATOR.validate_regional_manifest(
                document, fixture_profiles, self.config, self.species,
                GENERATOR.DEFAULT_REGIONS,
                GENERATOR.active_national_dex(GENERATOR.DEFAULT_SPECIES_INFO),
            )

    def test_day_aliases_generate_explicit_night_header_bindings(self):
        manifest = GENERATOR.load_regional_manifest(
            GENERATOR.DEFAULT_REGIONS, self.profiles, self.config, self.species
        )
        rendered = GENERATOR.render_header(
            self.encounters, self.config, self.scaling, self.offsets,
            self.metadata, self.standard_rod, manifest,
        )
        self.assertTrue(manifest["dayAliases"])
        for alias in manifest["dayAliases"]:
            method_suffix = alias["method"].title().replace("_", "")
            day_info = f"&{alias['dayBaseLabel']}_{method_suffix}Info"
            self.assertGreaterEqual(rendered.count(day_info), 2)
            self.assertNotIn(f"const struct WildPokemon {alias['nightBaseLabel']}_{method_suffix}", rendered)
        alias = manifest["dayAliases"][0]
        profile = next(
            row for row in manifest["profiles"]
            if row["map"] == alias["map"] and row["method"] == alias["method"]
        )
        report, night = GENERATOR.kanto_profile_distribution(
            profile, "NIGHT", "OLD_ROD", 10,
            {row["label"]: row for row in self.profiles}, self.scaling,
            {(row["product"], row["header_id"], row["area"], row["time"], row["rod"]): row["level_offset"] for row in self.offsets},
            self.by_species, self.standard_rod,
        )
        _, day = GENERATOR.kanto_profile_distribution(
            profile, "DAY", "OLD_ROD", 10,
            {row["label"]: row for row in self.profiles}, self.scaling,
            {(row["product"], row["header_id"], row["area"], row["time"], row["rod"]): row["level_offset"] for row in self.offsets},
            self.by_species, self.standard_rod,
        )
        self.assertEqual(report["baseLabel"], alias["nightBaseLabel"])
        self.assertEqual(report["sourceBaseLabel"], alias["dayBaseLabel"])
        self.assertEqual(report["runtimeTime"], "TIME_NIGHT")
        self.assertEqual(report["nightMode"], "DAY_ALIAS")
        self.assertEqual(night, day)

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
        self.assertEqual(audit["schemaVersion"], 3)
        self.assertTrue(audit["invariants"]["passed"], audit["invariants"]["failures"])
        kanto = audit["regions"]["KANTO"]
        self.assertEqual(kanto["ownership"]["profileDenominatorByTime"], {"DAY": 129, "NIGHT": 129})
        self.assertEqual(kanto["authoredSpeciesUnionCount"], 110)
        self.assertTrue(kanto["counterpartSolverProofs"])
        omission_proofs = [
            proof for proof in kanto["counterpartSolverProofs"]
            if proof["omittedCounterparts"]
        ]
        self.assertTrue(omission_proofs)
        self.assertTrue(all(
            proof["dpCertificate"]["candidateCounts"]["fullStrict"] == 0
            and proof["dpCertificate"]["candidateCounts"]["fullBudget"] == 0
            and proof["dpCertificate"]["completionTransitionClassCount"] > 0
            and len(proof["dpCertificate"]["canonicalEnumerationDigest"]) == 64
            and proof["fixedConstraints"]["provenanceBindingsUsed"] is False
            for proof in omission_proofs
        ))
        self.assertTrue(any(
            proof["omittedCounterparts"]
            and proof["fixedConstraints"]["protectedChinchouTargetSlots"] == [4, 6]
            for proof in omission_proofs
        ))
        self.assertGreaterEqual(kanto["materiallyDistinctLandProfileCount"], 25)
        self.assertTrue(kanto["forbiddenSpecies"]["passed"])
        self.assertTrue(kanto["hoennSoundComparison"]["passed"])
        self.assertEqual(
            len(kanto["hoennSoundComparison"]["profileComparisons"]),
            (41 + 31) * 2 * 71,
        )
        self.assertTrue(all(
            row["randomized"] is False
            and row["abilityAttraction"] == "OFF"
            and row["lure"] == "OFF"
            and row["offSpeciesProbabilities"] == row["onSpeciesProbabilities"]
            and row["differences"] == []
            and row["passed"]
            for row in kanto["hoennSoundComparison"]["profileComparisons"]
        ))
        self.assertEqual(len(kanto["effectivePortfolios"]), 2 * 3 * 71)
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

    def test_hoenn_sound_comparison_computes_and_rejects_distribution_changes(self):
        manifest = GENERATOR.load_json(GENERATOR.DEFAULT_REGIONS)["regions"]["KANTO"]
        route1 = next(
            row for row in manifest["profiles"]
            if row["map"] == "MAP_ROUTE1_HNS" and row["method"] == "land_mons"
        )
        profiles = copy.deepcopy(self.profiles)
        profiles_by_label = {profile["label"]: profile for profile in profiles}
        profiles_by_label[route1["dayBaseLabel"]]["encounter"]["land_mons"]["mons"][0]["species"] = "SPECIES_TREECKO"
        offset_map = {
            (row["product"], row["header_id"], row["area"], row["time"], row["rod"]): row["level_offset"]
            for row in self.offsets
        }
        failures = []
        comparison = GENERATOR.build_kanto_hoenn_sound_comparison(
            {"profiles": [route1]}, profiles_by_label, self.scaling,
            offset_map, self.by_species, self.standard_rod, failures,
        )
        self.assertFalse(comparison["passed"])
        self.assertTrue(comparison["differences"])
        self.assertTrue(failures)
        day_rating_10 = next(
            row for row in comparison["profileComparisons"]
            if row["time"] == "DAY" and row["rating"] == 10
        )
        self.assertTrue(day_rating_10["differences"])
        self.assertFalse(day_rating_10["passed"])

    def test_ecology_audit_honors_certified_reduced_counterpart_omission(self):
        source_weights = [10, 10, 10, 10, 10, 10, 10, 10, 5, 5, 5, 5]
        target_weights = [10, 10, 8, 8, 4, 10, 10, 10, 10, 10, 5, 5]

        def mons(default):
            return [{"species": default, "min_level": 5, "max_level": 5} for _ in range(12)]

        fire_mons = mons("SPECIES_X")
        leaf_mons = mons("SPECIES_X")
        target_mons = mons("SPECIES_X")
        for slot in (0, 1):
            fire_mons[slot]["species"] = "SPECIES_A"
            leaf_mons[slot]["species"] = "SPECIES_B"
        for slot in (2, 3):
            target_mons[slot]["species"] = "SPECIES_A"
        target_mons[4]["species"] = "SPECIES_C"

        def fixture_profile(label, weights, entries):
            return {
                "label": label,
                "group": {"fields": [{"type": "land_mons", "encounter_rates": weights}]},
                "encounter": {"land_mons": {"mons": entries}},
            }

        profiles = {
            "fire": fixture_profile("fire", source_weights, fire_mons),
            "leaf": fixture_profile("leaf", source_weights, leaf_mons),
            "day": fixture_profile("day", target_weights, target_mons),
        }
        ecology = {"method": "land_mons", "fireRedSlots": [0, 1], "leafGreenSlots": [0, 1]}
        level = {"version": "FIRERED", "baseLabel": "fire", "method": "land_mons", "slot": 0, "minLevel": 5, "maxLevel": 5}
        profile = {
            "map": "MAP_FIXTURE", "method": "land_mons",
            "fireRedSource": ["fire"], "leafGreenSource": ["leaf"],
            "dayBaseLabel": "day", "nightBaseLabel": "day", "nightMode": "DAY_ALIAS",
            "provenance": [
                {"targetTime": "DAY", "targetSlot": 2, "ecologySourceGroup": ecology, "levelSource": level, "reason": "FRLG_VERSION_COUNTERPART"},
                {"targetTime": "DAY", "targetSlot": 3, "ecologySourceGroup": ecology, "levelSource": level, "reason": "FRLG_VERSION_COUNTERPART"},
                {"targetTime": "DAY", "targetSlot": 4, "ecologySourceGroup": ecology, "levelSource": level, "reason": "GEN2_LOCAL_ADDITION"},
            ],
        }
        proof = {
            "map": "MAP_FIXTURE", "method": "land_mons", "selectedCategory": "reducedBudget",
            "canonicalGroups": [{"id": "G00", "sourceSlots": [0, 1]}],
            "selectedAssignment": [
                {"targetSlot": 2, "state": "SOURCE", "canonicalGroup": "G00", "species": "SPECIES_A"},
                {"targetSlot": 3, "state": "SOURCE", "canonicalGroup": "G00", "species": "SPECIES_A"},
                {"targetSlot": 4, "state": "UNASSIGNED"},
            ],
            "omittedCounterparts": [{"canonicalGroup": "G00", "species": "SPECIES_B"}],
        }
        failures = []
        report = GENERATOR.build_kanto_ecology_report(
            {"profiles": [profile], "counterpartProofs": [proof]},
            profiles,
            self.standard_rod,
            failures,
        )
        self.assertEqual(failures, [])
        row = report[0]
        self.assertEqual(row["targetSlots"], [2, 3])
        self.assertEqual(row["unassignedTargetSlots"], [4])
        self.assertEqual(row["certifiedOmittedCounterparts"], ["SPECIES_B"])
        self.assertEqual(row["rodReports"][0]["targetBudget"], {"numerator": 4, "denominator": 25})
        self.assertIsNone(row["rodReports"][0]["counterpartRatio"])
        self.assertFalse(row["rodReports"][0]["ratioExceptionApplied"])

    def test_cartographer_projection_has_runtime_bounds_and_strict_profile_identities(self):
        projection = self.cartographer
        self.assertEqual(projection["schemaVersion"], 2)
        self.assertEqual(projection["trainerRating"], {"minimum": 10, "maximum": 80})
        self.assertEqual(projection["authoredLevel"], {"minimum": 1, "maximum": 100})
        self.assertEqual(
            projection["headerCounts"],
            {"EMERALD": 124, "FIRERED": 132, "LEAFGREEN": 132, "POKEMON_HNS": 169},
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
            authored_labels = {profile["label"] for profile in self.profiles}
            self.assertTrue(
                all(
                    row["baseLabel"] in authored_labels
                    for row in GENERATOR.load_json(first)["profiles"]
                )
            )


if __name__ == "__main__":
    unittest.main()
