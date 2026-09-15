import hashlib
import importlib.util
import json
from pathlib import Path
import re
import tempfile
import unittest
from unittest.mock import patch


TOOL = Path(__file__).with_name("generate.py")
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_scripts_generate", TOOL)
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)
GAME_ROOT = TOOL.parents[2]


def digest(value):
    return hashlib.sha256(value.encode()).hexdigest()


class WayfarerSeviiScriptGenerationTest(unittest.TestCase):
    def test_checked_in_artifact_centrally_defines_every_registered_table(self):
        output = GAME_ROOT / "data/wayfarer_sevii_event_scripts.inc"
        rendered = GENERATOR.render(GAME_ROOT, GAME_ROOT / "src/data/wayfarer_sevii_maps.json")
        self.assertEqual(output.read_text(), rendered)
        manifest = json.loads((GAME_ROOT / "src/data/wayfarer_sevii_maps.json").read_text())
        for record in manifest["maps"]:
            table = f"{record['source_map']}_MapScripts::"
            self.assertEqual(rendered.count(table), 1)
            for handler in record["retained_map_scripts"]:
                self.assertIn(f"\tmap_script {handler['handler_type']}, {handler['wayfarer_script']}", rendered)
        environment = (GAME_ROOT / "data/scripts/wayfarer_sevii/environment.inc").read_text()
        self.assertNotIn("_Frlg_MapScripts::", environment)
        self.assertNotIn('data/maps/', rendered)
        self.assertNotIn('_Frlg/scripts.inc', rendered)
        modules = manifest["script_modules"]
        selected = GENERATOR.selected_module_names(manifest)
        source_labels = {
            row["label"]
            for module_name in selected
            for row in modules[module_name].get("allowed_externals", [])
            if row.get("kind") == "script_symbol" and row.get("path", "").startswith("data/maps/")
        }
        for label in source_labels:
            definitions = re.findall(rf"(?m)^{re.escape(label)}::?$", rendered)
            self.assertEqual(definitions, [f"{label}::"])

    def test_recursive_dependencies_include_special_implementation_sources(self):
        dependencies = GENERATOR.recursive_dependencies(GAME_ROOT, GAME_ROOT / "src/data/wayfarer_sevii_maps.json")
        self.assertIn("data/specials.inc", dependencies)
        self.assertIn("src/seagallop.c", dependencies)

    def fixture(self, root):
        source = root / "data/maps/OneIsland_Frlg/scripts.inc"
        source.parent.mkdir(parents=True)
        source.write_text("OneIsland_Frlg_MapScripts::\n\tmap_script MAP_SCRIPT_ON_LOAD, OneIsland_Frlg_OnLoad\n\t.byte 0\n")
        scripts = root / "data/scripts/wayfarer_sevii"
        scripts.mkdir(parents=True)
        (scripts / "common.inc").write_text("WayfarerSevii_EventScript_Empty::\n\tend\n")
        (scripts / "environment.inc").write_text("WayfarerSevii_OneIsland_OnLoad::\n\tend\n")
        return {
            "schema_version": 2,
            "content_domains": {"exploration": {"enabled": True}},
            "script_modules": {
                "common": {"owner": "exploration", "include": "data/scripts/wayfarer_sevii/common.inc",
                           "exports": ["WayfarerSevii_EventScript_Empty"], "allowed_commands": ["end"], "allowed_externals": []},
                "environment": {"owner": "exploration", "include": "data/scripts/wayfarer_sevii/environment.inc",
                                "exports": ["WayfarerSevii_OneIsland_OnLoad"], "allowed_commands": ["end"], "allowed_externals": []},
            },
            "maps": [{"source_map": "OneIsland_Frlg", "retained_map_scripts": [{
                "owner": "exploration", "content_id": "exploration.one-island.load", "module": "environment",
                "handler_type": "MAP_SCRIPT_ON_LOAD", "source_index": 0,
                "source": {"include": "data/maps/OneIsland_Frlg/scripts.inc", "label": "OneIsland_Frlg_OnLoad",
                           "sha256": digest("map_script MAP_SCRIPT_ON_LOAD, OneIsland_Frlg_OnLoad\n"),
                           "file_sha256": digest("OneIsland_Frlg_MapScripts::\n\tmap_script MAP_SCRIPT_ON_LOAD, OneIsland_Frlg_OnLoad\n\t.byte 0\n")},
                "wayfarer_script": "WayfarerSevii_OneIsland_OnLoad", "reason": "Source-free environmental baseline.",
            }]}],
        }

    def test_declarative_handler_emits_table_and_recursive_dependencies(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = root / "manifest.json"
            fixture = self.fixture(root)
            manifest.write_text(json.dumps(fixture))
            with patch.object(GENERATOR, "validated", return_value=(fixture, {})):
                rendered = GENERATOR.render(root, manifest)
            self.assertIn("OneIsland_Frlg_MapScripts::\n\tmap_script MAP_SCRIPT_ON_LOAD, WayfarerSevii_OneIsland_OnLoad\n\t.byte 0", rendered)

    def test_rejects_stale_source_handler_provenance(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = root / "manifest.json"
            fixture = self.fixture(root)
            fixture["maps"][0]["retained_map_scripts"][0]["source"]["label"] = "Changed"
            manifest.write_text(json.dumps(fixture))
            with self.assertRaisesRegex(GENERATOR.ClosureError, "source handler identity drifted"):
                GENERATOR.build_script_closure(root, fixture)

    def test_enabled_story_object_selects_its_module_and_disabled_one_does_not(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fixture = self.fixture(root)
            fixture["content_domains"]["story"] = {"enabled": False}
            fixture["script_modules"]["story"] = {
                "owner": "story", "include": "data/scripts/wayfarer_sevii/story/meteorite.inc",
                "exports": ["WayfarerSevii_Story_Meteorite"], "allowed_commands": ["end"], "allowed_externals": [],
            }
            fixture["maps"][0]["retained_events"] = {"object_events": [{
                "owner": "story", "source": {"script": "0x0"},
                "wayfarer_script": "WayfarerSevii_Story_Meteorite",
            }], "coord_events": [], "bg_events": []}
            self.assertNotIn("story", GENERATOR.selected_module_names(fixture))
            fixture["content_domains"]["story"]["enabled"] = True
            self.assertIn("story", GENERATOR.selected_module_names(fixture))

    def test_materializes_only_pinned_source_map_pure_text_data(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "data/maps/OneIsland_Frlg/scripts.inc"
            source.parent.mkdir(parents=True)
            source.write_text(
                "OneIsland_EventScript_Story::\n\tmsgbox OneIsland_Text_Hello\n\tend\n\n"
                "OneIsland_Text_Hello::\n\t.string \"Hello!$\"\n\n"
                "OneIsland_Text_Bad::\n\tmsgbox OneIsland_Text_Hello\n\tend\n",
                encoding="utf-8",
            )
            modules = {"story": {"allowed_externals": [
                {"kind": "script_symbol", "path": "data/maps/OneIsland_Frlg/scripts.inc",
                 "label": "OneIsland_Text_Hello"},
            ]}}
            blocks = "\n".join(GENERATOR.source_data_blocks(root, modules, {"story"}))
            self.assertIn("OneIsland_Text_Hello::", blocks)
            self.assertNotIn("OneIsland_EventScript_Story::", blocks)
            modules["story"]["allowed_externals"].append({
                "kind": "script_symbol", "path": "data/maps/OneIsland_Frlg/scripts.inc",
                "label": "OneIsland_Text_Bad",
            })
            with self.assertRaisesRegex(GENERATOR.GenerationError, "not pure text data"):
                GENERATOR.source_data_blocks(root, modules, {"story"})

    def test_enabled_event_requires_entrypoint_from_its_owner_module(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest_path = root / "manifest.json"
            fixture = self.fixture(root)
            fixture["content_domains"]["story"] = {"enabled": True}
            fixture["maps"][0]["retained_events"] = {"object_events": [{
                "owner": "story", "source": {"script": "OneIsland_Frlg_EventScript_Meteorite"},
                "wayfarer_script": "WayfarerSevii_OneIsland_OnLoad",
            }], "coord_events": [], "bg_events": []}
            manifest_path.write_text(json.dumps(fixture))
            with patch.object(GENERATOR, "validated", return_value=(fixture, {})):
                with self.assertRaisesRegex(GENERATOR.GenerationError, "not exported by an story module"):
                    GENERATOR.render(root, manifest_path)

    def test_atomic_write_preserves_timestamp_for_identical_content(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "artifact.inc"
            GENERATOR.atomic_write(output, "stable\n")
            first_mtime = output.stat().st_mtime_ns
            GENERATOR.atomic_write(output, "stable\n")
            self.assertEqual(first_mtime, output.stat().st_mtime_ns)


if __name__ == "__main__":
    unittest.main()
