import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock


TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


IMPORT = load_module("wayfarer_sinnoh_import_catalog", TOOLS / "import_catalog.py")


class SinnohCatalogImportPathTests(unittest.TestCase):
    @staticmethod
    def row(target_map="TwinleafTown", source_path="data/maps/TwinleafTown/map.json"):
        return {
            "source_map": "TwinleafTown", "target_map": target_map,
            "source_paths": {"map_json": source_path},
            "asset_records": {"blockdata": "block", "border": "border"},
        }

    @staticmethod
    def assets(block_path="data/layouts/TwinleafTown/map.bin", alias_path=None):
        block = {"record_id": "block", "source_path": block_path, "reuse_class": "SINNOH_NEW"}
        if alias_path is not None:
            block.update({
                "reuse_class": "EXACT_ALIAS", "canonical_owner": "LAYOUT_OWNER",
                "candidate_matches": [{"layout": "LAYOUT_OWNER", "path": alias_path}],
            })
        return {
            "block": block,
            "border": {"record_id": "border", "source_path": "data/layouts/TwinleafTown/border.bin", "reuse_class": "SINNOH_NEW"},
        }

    def assert_apply_preflight_rejects(self, row, assets, expression):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "root"
            donor = base / "donor"
            root.mkdir()
            donor.mkdir()
            escaped = base / "escaped"
            with mock.patch.object(IMPORT, "manifest_rows", return_value=[row]), \
                 mock.patch.object(IMPORT, "assets_by_id", return_value=assets):
                with self.assertRaisesRegex(IMPORT.FoundationError, expression):
                    IMPORT.check_or_write_catalog(root, donor, apply=True)
            self.assertFalse(escaped.exists())
            self.assertEqual(list(root.iterdir()), [])
            self.assertEqual(list(donor.iterdir()), [])

    def test_apply_rejects_absolute_target_map_before_writing(self):
        self.assert_apply_preflight_rejects(
            self.row(target_map="/tmp/escaped"), self.assets(), "target_map.*relative non-traversal"
        )

    def test_apply_rejects_traversal_asset_source_before_writing(self):
        self.assert_apply_preflight_rejects(
            self.row(), self.assets(block_path="../../escaped/map.bin"), "blockdata source_path.*relative non-traversal"
        )

    def test_apply_rejects_traversal_exact_alias_candidate_before_writing(self):
        self.assert_apply_preflight_rejects(
            self.row(), self.assets(alias_path="../../escaped/map.bin"), "exact-alias candidate path.*relative non-traversal"
        )
