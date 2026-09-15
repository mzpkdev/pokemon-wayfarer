import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


TOOL = Path(__file__).parents[1] / "schema.py"
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_content_schema", TOOL)
SCHEMA = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(SCHEMA)


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(",", ":")).encode()).hexdigest()


class WayfarerSeviiContentSchemaTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        (self.root / "data/maps/OneIsland_Frlg").mkdir(parents=True)
        (self.root / "src/data").mkdir(parents=True)
        (self.root / "tools/wayfarer_sevii_port").mkdir(parents=True)
        self.source = {
            "object_events": [{
                "local_id": "LOCALID_TEST", "type": "object", "x": 1, "y": 2,
                "elevation": 0, "script": "OneIsland_Source", "flag": "0",
            }],
            "warp_events": [],
            "coord_events": [{
                "type": "trigger", "x": 3, "y": 4, "elevation": 0,
                "var": "VAR_TEMP_0", "var_value": "0", "script": "OneIsland_Trigger",
            }],
            "bg_events": [],
        }
        (self.root / "data/maps/OneIsland_Frlg/map.json").write_text(json.dumps(self.source))
        wild = {"wild": "frozen"}
        event_island = {"islands": "frozen"}
        (self.root / "src/data/wayfarer_sevii_wild_encounters.json").write_text(json.dumps(wild))
        (self.root / "tools/wayfarer_sevii_port/event_island_baseline.json").write_text(json.dumps(event_island))
        self.baseline = {
            "projection_sha256": "0" * 64,
            "wild_encounters_sha256": digest(wild),
            "event_island_baseline_sha256": digest(event_island),
        }

    def tearDown(self):
        self.temp.cleanup()

    def manifest(self):
        object_row = {
            "index": 0, "source": self.source["object_events"][0],
            "wayfarer_script": "WayfarerSevii_Test", "owner": "exploration",
            "content_id": "exploration.one-island.test", "reason": "Keep the exploration service.",
        }
        return {
            "schema_version": 2,
            "release_link_enabled": True,
            "baseline": self.baseline,
            "contracts": {
                "schema_version": 1,
                "trainer_ids": {"base": 1515, "limit": 2048, "allocations": []},
                "state_namespace": {"name": "wayfarer_sevii", "flag_base": 49152,
                                    "flag_capacity": 256, "var_base": 53248, "var_capacity": 32,
                                    "defeat_bitset": "wayfarer_sevii_trainer_defeat", "storage": "SaveBlock3"},
                "states": [], "transactions": [],
            },
            "content_domains": {
                "exploration": {"owner": "exploration", "enabled": True,
                                "inventory": [object_row["content_id"]]},
                "ordinary_trainers": {"owner": "ordinary_trainer", "enabled": False, "inventory": []},
                "story": {"owner": "story", "enabled": False, "inventory": []},
                "trainer_tower": {"owner": "trainer_tower", "enabled": False, "inventory": []},
            },
            "script_modules": {},
            "maps": [{
                "source_map": "OneIsland_Frlg", "map_id": "MAP_ONE", "layout": "LAYOUT_ONE",
                "category": "OneIsland",
                "retained_events": {"object_events": [object_row], "warp_events": [],
                                    "coord_events": [], "bg_events": []},
                "retained_map_scripts": [], "encounter_methods": [],
            }],
            "exclusions": [{
                "content_id": "exclusion.old-travel-gate", "reason": "Campaign travel stays absent.",
                "source_identities": [{"source_map": "OneIsland_Frlg", "event_kind": "coord_events", "index": 0,
                                       "source": self.source["coord_events"][0]}],
            }],
        }

    def test_validates_source_identity_and_returns_a_copy(self):
        manifest = self.manifest()
        validated = SCHEMA.validate_manifest(self.root, manifest)
        self.assertEqual(validated, manifest)
        self.assertIsNot(validated, manifest)
        selected = SCHEMA.selected_records(validated)
        self.assertEqual(len(selected), 1)
        self.assertEqual(SCHEMA.source_identity(selected[0]), ("OneIsland_Frlg", "object_events", 0))
        self.assertEqual(selected[0]["domain"], "exploration")

    def test_validates_disabled_future_content_before_it_is_selected(self):
        manifest = self.manifest()
        story = copy.deepcopy(manifest["maps"][0]["retained_events"]["object_events"][0])
        story.update({"owner": "story", "content_id": "story.one-island.test", "source": {"wrong": True}})
        manifest["content_domains"]["story"]["inventory"].append(story["content_id"])
        manifest["maps"][0]["retained_events"]["object_events"].append(story)
        with self.assertRaisesRegex(SCHEMA.SchemaError, "exactly match"):
            SCHEMA.validate_manifest(self.root, manifest)

    def test_rejects_spatial_override_and_raw_state_namespace(self):
        manifest = self.manifest()
        row = manifest["maps"][0]["retained_events"]["object_events"][0]
        row["overrides"] = {"x": 9}
        with self.assertRaisesRegex(SCHEMA.SchemaError, "forbidden fields"):
            SCHEMA.validate_manifest(self.root, manifest)
        row["overrides"] = {}
        row["state_writes"] = ["FLAG_SYS_GAME_CLEAR"]
        with self.assertRaisesRegex(SCHEMA.SchemaError, "raw or non-Sevii"):
            SCHEMA.validate_manifest(self.root, manifest)

    def test_rejects_duplicate_content_inventory_resolution(self):
        manifest = self.manifest()
        manifest["content_domains"]["exploration"]["inventory"].append("exploration.other")
        with self.assertRaisesRegex(SCHEMA.SchemaError, "resolve exactly once"):
            SCHEMA.validate_manifest(self.root, manifest)

    def test_rejects_frozen_wild_drift(self):
        manifest = self.manifest()
        manifest["baseline"]["wild_encounters_sha256"] = "1" * 64
        with self.assertRaisesRegex(SCHEMA.SchemaError, "frozen normalized source"):
            SCHEMA.validate_manifest(self.root, manifest)

    def test_accepts_future_contract_declaration_lists_without_enabling_content(self):
        manifest = self.manifest()
        manifest["contracts"]["trainer_ids"]["allocations"] = [{"future": "declaration"}]
        manifest["contracts"]["states"] = [{"future": "declaration"}]
        manifest["contracts"]["transactions"] = [{"future": "declaration"}]
        validated = SCHEMA.validate_manifest(self.root, manifest)
        self.assertFalse(validated["content_domains"]["story"]["enabled"])

    def test_rejects_non_exploration_raw_source_visibility_without_an_override(self):
        manifest = self.manifest()
        self.source["object_events"][0]["flag"] = "FLAG_HIDE_FRLG_ACTOR"
        (self.root / "data/maps/OneIsland_Frlg/map.json").write_text(json.dumps(self.source))
        row = manifest["maps"][0]["retained_events"]["object_events"][0]
        row.update({"source": self.source["object_events"][0], "owner": "story", "content_id": "story.actor"})
        manifest["content_domains"]["exploration"]["inventory"] = []
        manifest["content_domains"]["story"]["inventory"] = ["story.actor"]
        with self.assertRaisesRegex(SCHEMA.SchemaError, "raw FRLG persistent state"):
            SCHEMA.validate_manifest(self.root, manifest)

    def test_accepts_tower_transient_source_visibility_without_persistent_override(self):
        manifest = self.manifest()
        self.source["object_events"][0]["flag"] = "FLAG_TEMP_2"
        (self.root / "data/maps/OneIsland_Frlg/map.json").write_text(json.dumps(self.source))
        row = manifest["maps"][0]["retained_events"]["object_events"][0]
        row.update({"source": self.source["object_events"][0], "owner": "trainer_tower",
                    "content_id": "trainer_tower.floor_actor"})
        manifest["content_domains"]["exploration"]["inventory"] = []
        manifest["content_domains"]["trainer_tower"]["inventory"] = ["trainer_tower.floor_actor"]
        validated = SCHEMA.validate_manifest(self.root, manifest)
        self.assertEqual(validated["maps"][0]["retained_events"]["object_events"][0]["source"]["flag"], "FLAG_TEMP_2")

    def test_rejects_absolute_exclusion_script_source(self):
        manifest = self.manifest()
        manifest["exclusions"][0]["source_identities"][0] = {
            "source_map": "OneIsland_Frlg", "event_kind": "script_label", "index": 0,
            "source": {"path": "/tmp/not-wayfarer.inc", "label": "Outside",
                       "file_sha256": "0" * 64, "body_sha256": "0" * 64},
        }
        with self.assertRaisesRegex(SCHEMA.SchemaError, "must stay under the game root"):
            SCHEMA.validate_manifest(self.root, manifest)


if __name__ == "__main__":
    unittest.main()
