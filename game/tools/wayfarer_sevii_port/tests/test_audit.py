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
            "release_link_enabled": False,
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
        self.assertFalse(report["release_link_enabled"])
        self.assertEqual(report["enabled_map_count"], 0)
        self.assertEqual(report["paths"], {"warps": [], "connections": []})
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

    def test_reports_only_the_release_enabled_catalog_and_reciprocal_warps(self):
        manifest = self.source_fixture()
        manifest["release_link_enabled"] = True
        first = self.root / "data/maps/OneIsland_Frlg/map.json"
        second = self.root / "data/maps/TwoIsland_Frlg/map.json"
        first_data = json.loads(first.read_text())
        second_data = json.loads(second.read_text())
        first_data["warp_events"] = [{"dest_map": "MAP_TWO", "dest_warp_id": "0"}]
        second_data["warp_events"] = [{"dest_map": "MAP_ONE", "dest_warp_id": "0"}]
        first.write_text(json.dumps(first_data))
        second.write_text(json.dumps(second_data))
        report = self.report(manifest)
        self.assertEqual(report["enabled_map_count"], 2)
        self.assertEqual(len(report["paths"]["warps"]), 2)
        self.assertTrue(all(row["reciprocal"] for row in report["paths"]["warps"]))
        manifest["maps"][0]["enabled"] = False
        report = self.report(manifest)
        self.assertEqual(report["enabled_map_count"], 1)
        self.assertEqual(report["paths"]["warps"], [])

    def test_rejects_unowned_or_prohibited_retained_scripts(self):
        manifest = self.source_fixture()
        map_path = self.root / "data/maps/OneIsland_Frlg/map.json"
        source = json.loads(map_path.read_text())
        event = {"type": "object", "trainer_type": "TRAINER_TYPE_NONE", "script": "OneIsland_Source"}
        source["object_events"] = [event]
        map_path.write_text(json.dumps(source))
        record = manifest["maps"][0]
        record["retained_events"]["object_events"] = [{
            "index": 0, "source": event, "wayfarer_script": "WayfarerSevii_Test",
        }]
        record["retained_map_scripts"] = [{"include": "data/scripts/wayfarer_sevii/test.inc"}]
        include = self.root / "data/scripts/wayfarer_sevii/test.inc"
        include.parent.mkdir(parents=True)
        include.write_text("WayfarerSevii_Test::\n\tend\n")
        self.assertEqual(self.report(manifest)["retained_scripts"]["retained_event_labels"], ["WayfarerSevii_Test"])
        include.write_text("WayfarerSevii_Test::\n\tgiveitem ITEM_POTION\n\tend\n")
        with self.assertRaisesRegex(AUDIT.AuditError, "prohibited command giveitem"):
            self.report(manifest)

    def test_allows_only_named_ferry_hook_in_baseline(self):
        script = self.root / "data/maps/BirthIsland_Harbor_hns/scripts.inc"
        script.parent.mkdir()
        original = b"Protected::\n\tend\nSailor::\n\twarp MAP_LILYCOVE_CITY_HARBOR, 8, 11\n"
        script.write_bytes(original)
        baseline = self.root / "baseline.json"
        blocks = AUDIT.block_bodies(script)
        baseline.write_text(json.dumps({"schema_version": 1, "files": [{
            "path": "data/maps/BirthIsland_Harbor_hns/scripts.inc", "sha256": digest(original),
            "protected_blocks": [{"label": "Protected", "sha256": digest(blocks["Protected"])}],
        }], "allowed_ferry_hooks": [{
            "path": "data/maps/BirthIsland_Harbor_hns/scripts.inc", "label": "Sailor",
            "sha256": digest(blocks["Sailor"]), "source_warp": "warp MAP_LILYCOVE_CITY_HARBOR, 8, 11",
            "wayfarer_warp": "warp MAP_VERMILION_CITY_PORT_INSIDE_HNS, 8, 9",
        }]}))
        script.write_text("Protected::\n\tend\nSailor::\n#if IS_WAYFARER\n\twarp MAP_VERMILION_CITY_PORT_INSIDE_HNS, 8, 9\n#else\n\twarp MAP_LILYCOVE_CITY_HARBOR, 8, 11\n#endif\n")
        result = AUDIT.validate_event_island_baseline(self.root, baseline)
        self.assertEqual(result["file_count"], 1)
        script.write_text("Protected::\n\tend\nSailor::\n\tgiveitem ITEM_MASTER_BALL\n")
        with self.assertRaisesRegex(AUDIT.AuditError, "ferry hook has an unapproved change"):
            AUDIT.validate_event_island_baseline(self.root, baseline)
        script.write_text("Protected::\n\tgoto Bad\nSailor::\n\tend\n")
        with self.assertRaisesRegex(AUDIT.AuditError, "protected block changed"):
            AUDIT.validate_event_island_baseline(self.root, baseline)

    def test_allows_only_exact_global_wayfarer_script_insertion(self):
        script = self.root / "data/event_scripts.s"
        script.parent.mkdir(exist_ok=True)
        original = b'#if IS_WAYFARER\n\t.include "data/wayfarer_engine_source_constants.inc"\n#endif\n'
        script.write_bytes(original)
        baseline = self.root / "baseline.json"
        baseline.write_text(json.dumps({"schema_version": 1, "files": [{
            "path": "data/event_scripts.s", "sha256": digest(original),
        }], "allowed_ferry_hooks": [], "allowed_global_insertions": [{
            "path": "data/event_scripts.s",
            "after": '\t.include "data/wayfarer_engine_source_constants.inc"\n',
            "insertion": '\t.include "data/wayfarer_sevii_event_scripts.inc"\n',
        }]}))
        script.write_text('#if IS_WAYFARER\n\t.include "data/wayfarer_engine_source_constants.inc"\n\t.include "data/wayfarer_sevii_event_scripts.inc"\n#endif\n')
        AUDIT.validate_event_island_baseline(self.root, baseline)
        script.write_text('#if IS_WAYFARER\n\t.include "data/wayfarer_engine_source_constants.inc"\n\t.include "data/wayfarer_sevii_event_scripts.inc"\n\t.include "data/unapproved.inc"\n#endif\n')
        with self.assertRaisesRegex(AUDIT.AuditError, "baseline file changed"):
            AUDIT.validate_event_island_baseline(self.root, baseline)


if __name__ == "__main__":
    unittest.main()
