import copy
import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location(
    "wild_encounter_generator", ROOT / "tools/wild_encounters/wild_encounters_to_header.py"
)
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)


class TrainerPredecessorTests(unittest.TestCase):
    def setUp(self):
        self.document = {
            "schemaVersion": 1,
            "minimumOrdinaryWildLevels": [
                {"species": "SPECIES_A", "minimumOrdinaryWildLevel": 90},
            ],
            "predecessorResolutions": [],
        }
        self.known = {
            "SPECIES_NONE": 0, "SPECIES_A": 1, "SPECIES_B": 2,
            "SPECIES_C": 3, "SPECIES_FORM": 4,
        }
        self.evolutions = {
            "SPECIES_A": [self.edge("SPECIES_B", 16)],
            "SPECIES_B": [self.edge("SPECIES_C", 36)],
            "SPECIES_C": [],
            "SPECIES_FORM": [],
        }

    @staticmethod
    def edge(target, level, method="EVO_LEVEL"):
        return {"target": target, "parameter": str(level), "method": method}

    def metadata(self, species):
        return GENERATOR.build_trainer_species_metadata(
            self.document, self.evolutions, self.known, species,
        )

    def test_multistage_closure_and_exact_thresholds_without_floors(self):
        rows = self.metadata({"SPECIES_C"})
        self.assertEqual([row["species"] for row in rows], ["SPECIES_A", "SPECIES_B", "SPECIES_C"])
        self.assertTrue(all("minimum_level" not in row for row in rows))
        by_species = {row["species"]: row for row in rows}
        for level, expected in [(1, "SPECIES_A"), (15, "SPECIES_A"), (16, "SPECIES_B"), (35, "SPECIES_B"), (36, "SPECIES_C"), (100, "SPECIES_C")]:
            with self.subTest(level=level):
                self.assertEqual(GENERATOR.effective_species("SPECIES_C", level, by_species)[0], expected)
        self.assertEqual(GENERATOR.effective_species("SPECIES_A", 100, by_species), ("SPECIES_A", []))

    def test_forms_and_non_level_evolutions_have_no_inferred_edge(self):
        self.evolutions["SPECIES_C"] = [self.edge("SPECIES_FORM", 1, "EVO_ITEM")]
        rows = self.metadata({"SPECIES_FORM"})
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["predecessor"], "SPECIES_NONE")
        self.evolutions["SPECIES_C"] = [self.edge("SPECIES_FORM", 40)]
        self.assertEqual(self.metadata({"SPECIES_FORM"})[-1]["predecessor"], "SPECIES_C")

    def test_ambiguous_graph_requires_valid_explicit_resolution(self):
        self.evolutions["SPECIES_FORM"] = [self.edge("SPECIES_B", 20)]
        with self.assertRaisesRegex(GENERATOR.ValidationError, "ambiguous"):
            self.metadata({"SPECIES_C"})
        self.document["predecessorResolutions"] = [{
            "species": "SPECIES_B", "predecessorSpecies": "SPECIES_FORM", "predecessorLevel": 20,
        }]
        self.assertEqual(self.metadata({"SPECIES_C"})[0]["predecessor"], "SPECIES_FORM")
        self.document["predecessorResolutions"][0]["predecessorLevel"] = 21
        with self.assertRaisesRegex(GENERATOR.ValidationError, "invalid predecessor resolution"):
            self.metadata({"SPECIES_C"})

    def test_cycles_rejected_even_outside_requested_inventory(self):
        self.evolutions["SPECIES_C"] = [self.edge("SPECIES_A", 50)]
        with self.assertRaisesRegex(GENERATOR.ValidationError, "cycle"):
            self.metadata({"SPECIES_FORM"})

    def test_unknown_inactive_and_empty_species_rejected(self):
        self.known["SPECIES_INACTIVE"] = 5
        for species in ["SPECIES_UNKNOWN", "SPECIES_INACTIVE", "SPECIES_NONE"]:
            with self.subTest(species=species):
                with self.assertRaisesRegex(GENERATOR.ValidationError, "unknown or inactive"):
                    self.metadata({species})

    def test_numeric_aliases_use_active_graph_and_preserve_exact_forms(self):
        self.known["SPECIES_ALIAS"] = self.known["SPECIES_B"]
        self.known["SPECIES_FORM_ALIAS"] = self.known["SPECIES_FORM"]
        rows = self.metadata({"SPECIES_ALIAS", "SPECIES_B", "SPECIES_FORM_ALIAS"})
        graph = {row["species"]: row for row in rows}
        self.assertEqual(GENERATOR.effective_species("SPECIES_ALIAS", 7, graph)[0], "SPECIES_A")
        self.assertEqual(GENERATOR.effective_species("SPECIES_ALIAS", 16, graph)[0], "SPECIES_ALIAS")
        self.assertEqual(GENERATOR.effective_species("SPECIES_FORM_ALIAS", 7, graph)[0], "SPECIES_FORM_ALIAS")
        self.assertEqual(GENERATOR.render_trainer_predecessor_header(rows).count(", SPECIES_A, 16 }"), 1)

    def test_unknown_numeric_predecessor_is_a_validation_error(self):
        self.evolutions["SPECIES_UNKNOWN"] = [self.edge("SPECIES_FORM", 20)]
        with self.assertRaisesRegex(GENERATOR.ValidationError, "malformed numeric evolution"):
            self.metadata({"SPECIES_FORM"})

    def test_generation_is_deterministic_and_does_not_mutate_inputs(self):
        before = copy.deepcopy((self.document, self.evolutions, self.known))
        self.assertEqual(self.metadata(["SPECIES_C", "SPECIES_FORM"]), self.metadata(["SPECIES_FORM", "SPECIES_C", "SPECIES_C"]))
        self.assertEqual(before, (self.document, self.evolutions, self.known))

    def test_runtime_header_contains_only_edges_and_has_empty_sentinel(self):
        rows = self.metadata({"SPECIES_C", "SPECIES_FORM"})
        output = GENERATOR.render_trainer_predecessor_header(rows)
        self.assertIn("{ SPECIES_B, SPECIES_A, 16 }", output)
        self.assertIn("{ SPECIES_C, SPECIES_B, 36 }", output)
        self.assertNotIn("SPECIES_FORM", output)
        self.assertEqual(output, GENERATOR.render_trainer_predecessor_header(list(reversed(rows))))
        self.assertIn("{ SPECIES_NONE, SPECIES_NONE, 0 }", GENERATOR.render_trainer_predecessor_header([]))

    def test_active_graph_can_cover_every_species_without_wild_inventory(self):
        known = GENERATOR.species_ids(GENERATOR.DEFAULT_SPECIES)
        evolutions = GENERATOR.active_evolutions(GENERATOR.DEFAULT_SPECIES_INFO)
        selected = {species for species in evolutions if species in known and known[species] != 0}
        rows = GENERATOR.build_trainer_species_metadata(
            GENERATOR.load_json(GENERATOR.DEFAULT_SPECIES_METADATA), evolutions, known, selected,
        )
        self.assertEqual({row["species"] for row in rows}, selected)
        by_species = {row["species"]: row for row in rows}
        self.assertEqual(GENERATOR.effective_species("SPECIES_DRAGONITE", 7, by_species)[0], "SPECIES_DRATINI")
        self.assertEqual(GENERATOR.effective_species("SPECIES_SKARMORY", 7, by_species)[0], "SPECIES_SKARMORY")
        self.assertEqual(GENERATOR.effective_species("SPECIES_RAICHU", 7, by_species)[0], "SPECIES_RAICHU")


if __name__ == "__main__":
    unittest.main()
