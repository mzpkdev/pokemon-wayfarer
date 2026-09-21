import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parents[1]
GAME = TOOLS.parents[1]
sys.path.insert(0, str(TOOLS))


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


AUDIT = load_module("wayfarer_sinnoh_audit", TOOLS / "audit.py")
FREEZE = load_module("wayfarer_sinnoh_freeze", TOOLS / "freeze.py")


class SinnohFoundationAuditTests(unittest.TestCase):
    def setUp(self):
        self.maps_path = GAME / "src/data/wayfarer_sinnoh_maps.json"
        self.assets_path = GAME / "src/data/wayfarer_sinnoh_assets.json"

    def test_checked_in_authorities_pass_without_a_donor_checkout(self):
        report = AUDIT.build_report(GAME, self.maps_path, self.assets_path)
        integrity = report["manifest_integrity"]
        self.assertEqual(integrity["selected_map_count"], 133)
        self.assertEqual(integrity["selected_layout_count"], 133)
        self.assertEqual(integrity["frozen_manifest_counts"]["warps"], 233)
        self.assertEqual(integrity["frozen_manifest_counts"]["connections"], 114)
        self.assertFalse(report["donor_source_verification"]["performed"])
        self.assertIsNone(report["donor_source_verification"]["observed_empty_content_counts"])
        self.assertTrue(report["review_required"])
        self.assertEqual(integrity["porymap"], {
            "base_game_version": {"value": "pokeemerald", "verified": True},
            "frozen_manifest_layout_format": {"value": "emerald", "verified": True, "map_count": 133},
            "imported_layout_verification": {"verified": False, "layout_count": 0},
        })

    def test_audit_report_is_deterministic_without_a_donor_checkout(self):
        first = AUDIT.build_report(GAME, self.maps_path, self.assets_path)
        second = AUDIT.build_report(GAME, self.maps_path, self.assets_path)
        self.assertEqual(json.dumps(first, sort_keys=True), json.dumps(second, sort_keys=True))

    def test_rejects_one_byte_exact_alias_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            candidate = root / "data/layouts/example/map.bin"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"\x01\x00")
            layouts = root / "data/layouts/layouts.json"
            layouts.write_text(json.dumps({"layouts": [{"id": "LAYOUT_EXAMPLE", "width": 1, "height": 1,
                                                       "blockdata_filepath": "data/layouts/example/map.bin"}]}))
            shape = {"width": 1, "height": 1, "element_bytes": 2, "byte_length": 2}
            record = {"record_id": "blockdata.example", "asset_family": "blockdata", "reuse_class": "EXACT_ALIAS",
                      "source_sha256": hashlib.sha256(b"\x01\x00").hexdigest(), "source_bytes": 2,
                      "shape": shape, "canonical_owner": "LAYOUT_EXAMPLE",
                      "candidate_matches": [{"layout": "LAYOUT_EXAMPLE", "path": "data/layouts/example/map.bin", "runtime_meaning_proven": True,
                                             "source_shape": shape, "candidate_shape": shape}]}
            AUDIT.validate_exact_worktree_assets(root, {record["record_id"]: record})
            candidate.write_bytes(b"\x02\x00")
            with self.assertRaisesRegex(AUDIT.FoundationError, "drifted"):
                AUDIT.validate_exact_worktree_assets(root, {record["record_id"]: record})

    def test_rejects_exact_alias_candidate_layout_shape_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            candidate = root / "data/layouts/example/map.bin"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"\x01\x00")
            shape = {"width": 1, "height": 1, "element_bytes": 2, "byte_length": 2}
            (root / "data/layouts/layouts.json").write_text(json.dumps({"layouts": [{"id": "LAYOUT_EXAMPLE", "width": 2, "height": 1,
                                                                            "blockdata_filepath": "data/layouts/example/map.bin"}]}))
            record = {"record_id": "blockdata.example", "asset_family": "blockdata", "reuse_class": "EXACT_ALIAS",
                      "source_sha256": hashlib.sha256(b"\x01\x00").hexdigest(), "source_bytes": 2, "shape": shape,
                      "canonical_owner": "LAYOUT_EXAMPLE", "candidate_matches": [{"layout": "LAYOUT_EXAMPLE", "path": "data/layouts/example/map.bin",
                      "runtime_meaning_proven": True, "source_shape": shape, "candidate_shape": shape}]}
            with self.assertRaisesRegex(AUDIT.FoundationError, "layout shape drifted"):
                AUDIT.validate_exact_worktree_assets(root, {record["record_id"]: record})

    def test_rejects_porymap_base_game_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "porymap.project.json").write_text(json.dumps({"base_game_version": "pokefirered"}))
            with self.assertRaisesRegex(AUDIT.FoundationError, "base_game_version pokeemerald"):
                AUDIT.validate_porymap_contract(root, [{"layout_format": "emerald"}])

    def test_rejects_frozen_manifest_layout_format_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "porymap.project.json").write_text(json.dumps({"base_game_version": "pokeemerald"}))
            with self.assertRaisesRegex(AUDIT.FoundationError, "layout_format emerald"):
                AUDIT.validate_porymap_contract(root, [{"layout_format": "frlg"}])

    def test_rejects_imported_target_layout_without_verification(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "porymap.project.json").write_text(json.dumps({"base_game_version": "pokeemerald"}))
            layouts = root / "data/layouts/layouts.json"
            layouts.parent.mkdir(parents=True)
            layouts.write_text(json.dumps({"layouts": [{"id": "LAYOUT_SINNOH_ROUTE201"}]}))
            with self.assertRaisesRegex(AUDIT.FoundationError, "require verification"):
                AUDIT.validate_porymap_contract(root, [{
                    "layout_format": "emerald", "target_layout": "LAYOUT_SINNOH_ROUTE201",
                }])

    def test_donor_empty_content_check_rejects_actual_nonempty_source(self):
        with tempfile.TemporaryDirectory() as directory:
            donor = Path(directory)
            wild = donor / "src/data/wild_encounters.json"
            wild.parent.mkdir(parents=True)
            wild.write_text(json.dumps({"wild_encounter_groups": [{"encounters": [{"map": "MAP_TEST"}]}]}))
            observed = AUDIT.donor_empty_counts(donor, {"Test": {"id": "MAP_TEST", "object_events": [{"x": 1}], "coord_events": [], "bg_events": [], "map_scripts": []}})
            self.assertEqual(observed["object_events"], 1)
            self.assertEqual(observed["wild_encounter_profiles"], 1)
            with self.assertRaisesRegex(AUDIT.FoundationError, "empty-content facts drifted"):
                AUDIT.require_expected_empty_counts(observed)

    def test_rejects_count_and_asset_reference_failures(self):
        maps = json.loads(self.maps_path.read_text())
        assets = json.loads(self.assets_path.read_text())
        maps["maps"] = maps["maps"][:-1]
        with self.assertRaisesRegex(AUDIT.FoundationError, "group count drifted|exactly 133"):
            AUDIT.validate_checked_in(GAME, maps, assets)
        maps = json.loads(self.maps_path.read_text())
        maps["maps"][0]["asset_records"]["blockdata"] = "blockdata.not_declared"
        with self.assertRaisesRegex(AUDIT.FoundationError, "missing or malformed asset reference"):
            AUDIT.validate_checked_in(GAME, maps, assets)

    def test_release_selection_stays_blocked_by_review_required_assets(self):
        with self.assertRaisesRegex(AUDIT.FoundationError, "asset gate"):
            AUDIT.build_report(GAME, self.maps_path, self.assets_path, release_selection=True)

    def test_collision_sections_keep_natural_sinnoh_labels(self):
        rows = {row["source_map"]: row for row in json.loads(self.maps_path.read_text())["maps"]}
        for name in ("TwinleafTown_Haouse1", "TwinleafTown_House2"):
            section = rows[name]["map_properties"]
            self.assertEqual(section["source_map_section"], "MAPSEC_LITTLEROOT_TOWN")
            self.assertEqual(section["target_map_section"], "MAPSEC_SINNOH_TWINLEAF_TOWN")
            self.assertEqual(section["target_map_section_label"], "Twinleaf Town")
        self.assertEqual(rows["PokmonLeague"]["map_properties"]["target_map_section_label"], "Sinnoh Pokemon League")


if __name__ == "__main__":
    unittest.main()
