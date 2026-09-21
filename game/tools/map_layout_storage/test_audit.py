import tempfile
import unittest
from pathlib import Path

from audit import find_generated_symbol_violations, find_object_symbol_violations, find_violations


class SourceAuditTest(unittest.TestCase):
    def test_allows_storage_module_and_rejects_consumers(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "src").mkdir()
            (root / "test").mkdir()
            (root / "src" / "map_layout.c").write_text("return layout->mapData;\n")
            (root / "src" / "consumer.c").write_text("return mapLayout->mapData;\n")
            self.assertEqual(
                find_violations(root),
                ["src/consumer.c:1: return mapLayout->mapData;"],
            )

    def test_rejects_legacy_typed_payload_access(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "src").mkdir()
            (root / "test").mkdir()
            (root / "test" / "consumer.c").write_text("tile = layout->map[index];\n")
            self.assertEqual(len(find_violations(root)), 1)

    def test_rejects_public_generated_payload_labels(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            layouts = root / "data" / "layouts"
            layouts.mkdir(parents=True)
            (layouts / "layouts.inc").write_text(
                "Visible_Blockdata::\n.LPrivate_Blockdata:\nVisible_MapData:\n"
            )
            self.assertEqual(len(find_generated_symbol_violations(root)), 2)

    def test_rejects_exported_object_payload_symbols(self):
        output = "08000000 R Visible_Blockdata\n08000004 R PublicLayout\n"
        self.assertEqual(
            find_object_symbol_violations(output),
            ["exported map-layout storage symbol: Visible_Blockdata"],
        )


if __name__ == "__main__":
    unittest.main()
