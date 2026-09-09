import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


TOOL = Path(__file__).with_name("generate.py")
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_scripts_generate", TOOL)
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)
GAME_ROOT = TOOL.parents[2]


class WayfarerSeviiScriptGenerationTest(unittest.TestCase):
    def test_checked_in_artifact_defines_only_source_free_map_tables(self):
        output = GAME_ROOT / "data/wayfarer_sevii_event_scripts.inc"
        rendered = GENERATOR.render(GAME_ROOT, GAME_ROOT / "src/data/wayfarer_sevii_maps.json")
        self.assertEqual(output.read_text(), rendered)
        manifest = json.loads((GAME_ROOT / "src/data/wayfarer_sevii_maps.json").read_text())
        for record in manifest["maps"]:
            label = f"{record['source_map']}_MapScripts::"
            if record["retained_map_scripts"]:
                owned_source = "\n".join(
                    (GAME_ROOT / entry["include"]).read_text()
                    for entry in record["retained_map_scripts"]
                )
                self.assertIn(label, owned_source)
            else:
                self.assertIn(label, rendered)
        self.assertNotIn('data/maps/', rendered)
        self.assertNotIn('_Frlg/scripts.inc', rendered)

    def test_explicit_wayfarer_script_include_owns_its_map_table(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/hub.inc"
            include.parent.mkdir(parents=True)
            include.write_text("OneIsland_Frlg_MapScripts::\n\t.byte 0\n")
            manifest = root / "manifest.json"
            manifest.write_text(json.dumps({"schema_version": 1, "maps": [{
                "source_map": "OneIsland_Frlg",
                "retained_map_scripts": [{"include": "data/scripts/wayfarer_sevii/hub.inc"}],
            }, {"source_map": "TwoIsland_Frlg", "retained_map_scripts": []}]}))
            rendered = GENERATOR.render(root, manifest)
            self.assertIn('\t.include "data/scripts/wayfarer_sevii/hub.inc"', rendered)
            self.assertNotIn("OneIsland_Frlg_MapScripts::\n\t.byte 0", rendered)
            self.assertIn("TwoIsland_Frlg_MapScripts::\n\t.byte 0", rendered)

    def test_rejects_source_script_include(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = root / "manifest.json"
            manifest.write_text(json.dumps({"schema_version": 1, "maps": [{
                "source_map": "OneIsland_Frlg",
                "retained_map_scripts": [{"include": "data/maps/OneIsland_Frlg/scripts.inc"}],
            }]}))
            with self.assertRaisesRegex(GENERATOR.GenerationError, "must stay under"):
                GENERATOR.render(root, manifest)

    def test_atomic_write_preserves_timestamp_for_identical_content(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "artifact.inc"
            GENERATOR.atomic_write(output, "stable\n")
            first_mtime = output.stat().st_mtime_ns
            GENERATOR.atomic_write(output, "stable\n")
            self.assertEqual(first_mtime, output.stat().st_mtime_ns)


if __name__ == "__main__":
    unittest.main()
