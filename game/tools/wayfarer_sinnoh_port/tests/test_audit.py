import copy
import importlib.util
import json
from pathlib import Path
import sys
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


class SinnohCatalogTests(unittest.TestCase):
    def setUp(self):
        self.maps_path = GAME / "src/data/wayfarer_sinnoh_maps.json"

    def test_compact_catalog_passes_without_a_donor_checkout(self):
        report = AUDIT.build_report(GAME, self.maps_path)
        self.assertEqual(report["manifest_integrity"]["selected_map_count"], 133)
        self.assertEqual(report["manifest_integrity"]["catalog"]["topology"]["live_warp_count"], 228)
        self.assertFalse(report["donor_source_verification"]["performed"])

    def test_duplicate_map_identity_is_rejected(self):
        maps = json.loads(self.maps_path.read_text())
        maps["maps"][1]["target_map_id"] = maps["maps"][0]["target_map_id"]
        with self.assertRaisesRegex(AUDIT.FoundationError, "duplicated"):
            AUDIT.validate_checked_in(GAME, maps)

    def test_topology_uses_repairs_without_duplicate_fixtures(self):
        rows = json.loads(self.maps_path.read_text())["maps"]
        repairs = [repair for row in rows for repair in row.get("topology", {}).get("repairs", [])]
        self.assertEqual(len(repairs), 31)
        self.assertTrue(all("fixture" not in repair for repair in repairs))
        self.assertEqual(AUDIT.validate_topology(rows)["connection_repair_count"], 3)

    def test_bad_repair_is_rejected(self):
        rows = json.loads(self.maps_path.read_text())["maps"]
        row = copy.deepcopy(next(row for row in rows if row.get("topology", {}).get("repairs")))
        row["topology"]["repairs"][0]["source"] = {}
        with self.assertRaisesRegex(AUDIT.FoundationError, "ambiguous topology repair"):
            AUDIT.effective_warps(row)


if __name__ == "__main__":
    unittest.main()
