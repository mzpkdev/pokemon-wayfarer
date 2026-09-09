import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_audit", TOOLS / "audit.py")
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


def digest(value):
    return hashlib.sha256(value).hexdigest()


class WayfarerSeviiPortAuditTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        (self.root / "data/maps").mkdir(parents=True)
        (self.root / "data/layouts").mkdir(parents=True)
        (self.root / "src/data").mkdir(parents=True)

    def tearDown(self):
        self.temp.cleanup()

    def add_map(self, name, map_id, layout, version="frlg"):
        directory = self.root / "data/maps" / name
        directory.mkdir(exist_ok=True)
        (directory / "map.json").write_text(json.dumps({"id": map_id, "layout": layout, "game_version": version}))

    def source_fixture(self):
        self.add_map("OneIsland_Frlg", "MAP_ONE", "LAYOUT_ONE")
        self.add_map("TwoIsland_Frlg", "MAP_TWO", "LAYOUT_TWO")
        self.add_map("BirthIsland_Harbor_Frlg", "MAP_BIRTH", "LAYOUT_BIRTH")
        self.add_map("NavelRock_Harbor_Frlg", "MAP_NAVEL", "LAYOUT_NAVEL")
        self.add_map("SevenIsland_UnusedHouse", "MAP_UNUSED", "LAYOUT_UNUSED")
        (self.root / "data/maps/map_groups.json").write_text(json.dumps({
            "group_order": ["gSevii"],
            "gSevii": ["OneIsland_Frlg", "TwoIsland_Frlg"],
        }))
        layouts = []
        for layout_id, size in (("LAYOUT_ONE", 3), ("LAYOUT_TWO", 5)):
            stem = layout_id.lower()
            map_path = self.root / "data/layouts" / f"{stem}.map.bin"
            border_path = self.root / "data/layouts" / f"{stem}.border.bin"
            map_path.write_bytes(b"m" * size)
            border_path.write_bytes(b"b")
            layouts.append({"id": layout_id, "layout_version": "frlg", "game_version": "frlg",
                            "blockdata_filepath": map_path.relative_to(self.root).as_posix(),
                            "border_filepath": border_path.relative_to(self.root).as_posix(),
                            "primary_tileset": "gPrimary", "secondary_tileset": "gSecondary"})
        (self.root / "data/layouts/layouts.json").write_text(json.dumps({"layouts": layouts}))
        return {
            "schema_version": 1,
            "maps": [
                {"source_map": "OneIsland_Frlg", "map_id": "MAP_ONE", "layout": "LAYOUT_ONE", "category": "one",
                 "retained_events": {key: [] for key in ("object_events", "warp_events", "coord_events", "bg_events")},
                 "retained_map_scripts": [], "encounter_methods": []},
                {"source_map": "TwoIsland_Frlg", "map_id": "MAP_TWO", "layout": "LAYOUT_TWO", "category": "two",
                 "retained_events": {key: [] for key in ("object_events", "warp_events", "coord_events", "bg_events")},
                 "retained_map_scripts": [], "encounter_methods": []},
            ],
            "exclusions": [
                {"source_map": "BirthIsland_Harbor_Frlg", "reason": "Existing Wayfarer content."},
                {"source_map": "NavelRock_Harbor_Frlg", "reason": "Existing Wayfarer content."},
                {"source_map": "SevenIsland_UnusedHouse", "reason": "Not registered."},
            ],
        }

    def report(self, manifest):
        path = self.root / "src/data/wayfarer_sevii_maps.json"
        path.write_text(json.dumps(manifest))
        return AUDIT.build_report(self.root, path, expected_map_count=2, expected_layout_count=2, expected_raw_bytes=10)

    def test_reports_deterministic_source_provenance(self):
        report = self.report(self.source_fixture())
        self.assertEqual(report["selected_map_count"], 2)
        self.assertEqual(report["selected_layout_count"], 2)
        self.assertEqual(report["raw_layout_bytes"], 10)
        self.assertEqual([row["source_map"] for row in report["maps"]], ["OneIsland_Frlg", "TwoIsland_Frlg"])
        self.assertEqual(report["event_island_frlg_exclusions"], ["BirthIsland_Harbor_Frlg", "NavelRock_Harbor_Frlg"])
        first = json.dumps(report, sort_keys=True, separators=(",", ":"))
        second = json.dumps(self.report(self.source_fixture()), sort_keys=True, separators=(",", ":"))
        self.assertEqual(first, second)

    def test_rejects_unreviewed_registered_map_and_source_drift(self):
        manifest = self.source_fixture()
        manifest["maps"] = manifest["maps"][:1]
        with self.assertRaisesRegex(AUDIT.AuditError, "ordering or membership"):
            self.report(manifest)
        manifest = self.source_fixture()
        manifest["maps"][0]["layout"] = "LAYOUT_TWO"
        with self.assertRaisesRegex(AUDIT.AuditError, "does not match source"):
            self.report(manifest)

    def test_rejects_missing_event_island_exclusion(self):
        manifest = self.source_fixture()
        manifest["exclusions"] = manifest["exclusions"][1:]
        with self.assertRaisesRegex(AUDIT.AuditError, "lack exclusion reasons"):
            self.report(manifest)

    def test_allows_only_named_ferry_hook_in_baseline(self):
        script = self.root / "data/maps/BirthIsland_Harbor_hns/scripts.inc"
        script.parent.mkdir()
        original = b"Protected::\n\tend\nSailor::\n\tend\n"
        script.write_bytes(original)
        baseline = self.root / "baseline.json"
        blocks = AUDIT.block_bodies(script)
        baseline.write_text(json.dumps({"schema_version": 1, "files": [{
            "path": "data/maps/BirthIsland_Harbor_hns/scripts.inc", "sha256": digest(original),
            "protected_blocks": [{"label": "Protected", "sha256": digest(blocks["Protected"])}],
        }], "allowed_ferry_hooks": [{"path": "data/maps/BirthIsland_Harbor_hns/scripts.inc", "label": "Sailor"}]}))
        script.write_text("Protected::\n\tend\nSailor::\n\tgoto WayfarerReturn\n")
        result = AUDIT.validate_event_island_baseline(self.root, baseline)
        self.assertEqual(result["file_count"], 1)
        script.write_text("Protected::\n\tgoto Bad\nSailor::\n\tend\n")
        with self.assertRaisesRegex(AUDIT.AuditError, "protected block changed"):
            AUDIT.validate_event_island_baseline(self.root, baseline)


if __name__ == "__main__":
    unittest.main()
