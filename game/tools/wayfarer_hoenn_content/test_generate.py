import importlib.util
import hashlib
import json
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("generate.py")
SPEC = importlib.util.spec_from_file_location("wayfarer_hoenn_content", MODULE_PATH)
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)
GAME_ROOT = Path(__file__).resolve().parents[2]


class WayfarerHoennContentAuditTest(unittest.TestCase):
    def make_fixture(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        for directory in (
            "data/maps/EmeraldMap",
            "data/maps/HnsMap",
            "data/layouts/TestLayout",
            "data/scripts",
            "include/constants",
            "src/data",
            "src/data/tilesets",
        ):
            (root / directory).mkdir(parents=True, exist_ok=True)

        emerald = {
            "id": "MAP_EMERALD",
            "name": "EmeraldMap",
            "game_version": "emerald",
            "layout": "LAYOUT_TEST",
            "region_map_section": "MAPSEC_TEST",
            "map_type": "MAP_TYPE_TOWN",
            "connections": [{"direction": "up", "offset": 0, "map": "MAP_HNS"}],
            "object_events": [{"script": "EmeraldMap_EventScript_Test", "flag": "FLAG_TEST"}],
            "warp_events": [{"dest_map": "MAP_HNS", "dest_warp_id": "0"}],
            "coord_events": [],
            "bg_events": [],
        }
        hns = {
            "id": "MAP_HNS",
            "name": "HnsMap",
            "game_version": "hns",
            "layout": "LAYOUT_TEST",
            "connections": [],
            "object_events": [],
            "warp_events": [{"dest_map": "MAP_DYNAMIC", "dest_warp_id": "-1"}],
            "coord_events": [],
            "bg_events": [],
        }
        self.write_json(root / "data/maps/EmeraldMap/map.json", emerald)
        self.write_json(root / "data/maps/HnsMap/map.json", hns)
        self.write_json(
            root / "data/maps/map_groups.json",
            {"group_order": ["gEmerald", "gHns"], "gEmerald": ["EmeraldMap"], "gHns": ["HnsMap"]},
        )
        self.write_json(
            root / "data/layouts/layouts.json",
            {"layouts_table_label": "gMapLayouts", "layouts": [{
                "id": "LAYOUT_TEST",
                "name": "TestLayout",
                "layout_version": "emerald",
                "primary_tileset": "gTileset_Primary",
                "secondary_tileset": "gTileset_Secondary",
                "border_filepath": "data/layouts/TestLayout/border.bin",
                "blockdata_filepath": "data/layouts/TestLayout/map.bin",
            }]},
        )
        (root / "data/layouts/TestLayout/border.bin").touch()
        (root / "data/layouts/TestLayout/map.bin").touch()
        (root / "data/maps/EmeraldMap/scripts.inc").write_text(
            "EmeraldMap_MapScripts::\nEmeraldMap_EventScript_Test::\n"
            "\ttrainerbattle_single TRAINER_ALPHA, Text_Intro, Text_Defeat\n"
            "\tsetflag FLAG_TEST\n",
            encoding="utf-8",
        )
        self.write_json(
            root / "src/data/wild_encounters.json",
            {"wild_encounter_groups": [{"label": "gWildMonHeaders", "for_maps": True,
              "encounters": [{"map": "MAP_EMERALD", "base_label": "gEmerald",
                              "land_mons": {}, "fishing_mons": {}}]}]},
        )
        (root / "src/data/wild_encounters.h").write_text(
            "gEmerald_LandMonsInfo\ngEmerald_FishingMonsInfo\n", encoding="utf-8"
        )
        self.write_json(
            root / "src/data/heal_locations.json",
            {"heal_locations": [{"id": "HEAL_TEST", "source": "EMERALD", "map": "MAP_EMERALD"}]},
        )
        (root / "src/region_map.c").write_text(
            "{REGION_MAP_HOENN, HOENN_FLAG_ID(1), MAPSEC_TEST}\n", encoding="utf-8"
        )
        (root / "include/tilesets.h").write_text(
            "extern int gTileset_Primary;\nextern int gTileset_Secondary;\n", encoding="utf-8"
        )
        (root / "src/data/tilesets/headers.h").write_text("", encoding="utf-8")
        (root / "include/constants/opponents.h").write_text(
            "#define WAYFARER_HOENN_TRAINER_OFFSET 630\n"
            "#define TRAINER_ALPHA TRAINER_EMERALD_ID(1)\n",
            encoding="utf-8",
        )
        (root / "src/data/trainers.party").write_text("=== TRAINER_ALPHA ===\n", encoding="utf-8")
        (root / "include/constants/flags.h").write_text("#define FLAG_TEST 0x20\n", encoding="utf-8")
        (root / "include/constants/vars.h").write_text("#define VAR_STARTER_MON 0x4023\n", encoding="utf-8")
        (root / "data/wayfarer_hoenn_source_constants.inc").write_text(
            "#define FLAG_TEST 0x6020\n#define VAR_STARTER_MON 0x7023\n", encoding="utf-8"
        )
        (root / "data/wayfarer_engine_source_constants.inc").write_text(
            "#define FLAG_TEST 0x20\n#define VAR_STARTER_MON 0x4023\n", encoding="utf-8"
        )

        policy = {
            "schemaVersion": 1,
            "source": "emerald",
            "expectedMapCount": 1,
            "expectedCatalogSha256": GENERATOR.fingerprint(["EmeraldMap"]),
            "expectedMapContentSha256": "fixture-filled-below",
            "expectedTrainerReferencesSha256": GENERATOR.fingerprint(["EmeraldMap:TRAINER_ALPHA"]),
            "expectedTrainerOffset": 630,
            "expectedTrainerPartiesSha256": GENERATOR.fingerprint([
                "TRAINER_ALPHA:" + hashlib.sha256(b"=== TRAINER_ALPHA ===").hexdigest()
            ]),
            "expectedWildMethodsSha256": GENERATOR.fingerprint(["EmeraldMap:goodRod,land,oldRod,superRod"]),
            "expectedWildProfileCount": 1,
            "expectedWildProfilesSha256": GENERATOR.fingerprint([
                "MAP_EMERALD:gEmerald:goodRod,land,oldRod,superRod:" + hashlib.sha256(
                    json.dumps(
                        {"map": "MAP_EMERALD", "base_label": "gEmerald", "land_mons": {}, "fishing_mons": {}},
                        sort_keys=True,
                        separators=(",", ":"),
                    ).encode("utf-8")
                ).hexdigest()
            ]),
            "expectedStarterConsumersSha256": GENERATOR.fingerprint([]),
            "default": {"classification": "required", "reason": "fixture", "specification": "fixture", "disposition": "included"},
            "rules": [],
        }
        policy_path = root / "policy.json"
        self.write_json(policy_path, policy)
        initial_manifest = GENERATOR.build_manifest(
            root,
            policy_path,
            root / "data/wayfarer_hoenn_source_constants.inc",
            root / "data/wayfarer_engine_source_constants.inc",
        )
        policy["expectedMapContentSha256"] = initial_manifest["fingerprints"]["mapContentSha256"]
        self.write_json(policy_path, policy)
        return root, policy_path

    @staticmethod
    def write_json(path, value):
        path.write_text(json.dumps(value), encoding="utf-8")

    def run_fixture(self, root, policy):
        return GENERATOR.build_manifest(
            root,
            policy,
            root / "data/wayfarer_hoenn_source_constants.inc",
            root / "data/wayfarer_engine_source_constants.inc",
        )

    def test_fixture_records_content_and_resolves_union_targets(self):
        root, policy = self.make_fixture()
        manifest = self.run_fixture(root, policy)
        self.assertTrue(manifest["audit"]["passed"], manifest["audit"]["failures"])
        entry = manifest["maps"][0]
        self.assertEqual((entry["group"]["number"], entry["number"]), (0, 0))
        self.assertEqual(entry["layout"]["number"], 1)
        self.assertEqual(entry["events"]["counts"], {"objects": 1, "warps": 1, "coordinates": 0, "backgrounds": 0})
        self.assertEqual(entry["wildEncounterMethods"], ["goodRod", "land", "oldRod", "superRod"])
        self.assertEqual(entry["trainers"][0]["wayfarerId"], 631)
        self.assertTrue(entry["flyDestination"])

    def test_missing_target_and_layout_asset_fail_the_audit(self):
        root, policy = self.make_fixture()
        data_path = root / "data/maps/EmeraldMap/map.json"
        data = json.loads(data_path.read_text())
        data["warp_events"][0]["dest_map"] = "MAP_MISSING"
        del data["bg_events"]
        self.write_json(data_path, data)
        (root / "data/layouts/TestLayout/map.bin").unlink()
        manifest = self.run_fixture(root, policy)
        failures = "\n".join(manifest["audit"]["failures"])
        self.assertIn("missing blockdata asset", failures)
        self.assertIn("references unavailable map MAP_MISSING", failures)
        self.assertIn("no complete effective event table", failures)

    def test_valid_topology_or_script_drift_changes_content_fingerprint(self):
        root, policy = self.make_fixture()
        data_path = root / "data/maps/EmeraldMap/map.json"
        data = json.loads(data_path.read_text())
        data["warp_events"][0] = {"dest_map": "MAP_DYNAMIC", "dest_warp_id": "-1"}
        self.write_json(data_path, data)
        script_path = root / "data/maps/EmeraldMap/scripts.inc"
        script_path.write_text(script_path.read_text() + "\tdelay 1\n", encoding="utf-8")

        manifest = self.run_fixture(root, policy)
        failures = "\n".join(manifest["audit"]["failures"])
        self.assertIn("expectedMapContentSha256 changed", failures)

    def test_shared_event_script_trainers_and_state_are_audited(self):
        root, policy = self.make_fixture()
        map_path = root / "data/maps/EmeraldMap/map.json"
        data = json.loads(map_path.read_text())
        data["object_events"][0]["script"] = "Shared_EventScript_Trainer"
        self.write_json(map_path, data)
        (root / "data/maps/EmeraldMap/scripts.inc").write_text(
            "EmeraldMap_MapScripts::\n",
            encoding="utf-8",
        )
        (root / "data/scripts/shared.inc").write_text(
            "Shared_EventScript_Trainer::\n"
            "\ttrainerbattle_single TRAINER_ALPHA, Text_Intro, Text_Defeat\n"
            "\tsetflag FLAG_TEST\n",
            encoding="utf-8",
        )

        updated_policy = json.loads(policy.read_text())
        changed_manifest = self.run_fixture(root, policy)
        updated_policy["expectedMapContentSha256"] = changed_manifest["fingerprints"]["mapContentSha256"]
        self.write_json(policy, updated_policy)

        manifest = self.run_fixture(root, policy)
        self.assertTrue(manifest["audit"]["passed"], manifest["audit"]["failures"])
        self.assertEqual([row["id"] for row in manifest["maps"][0]["trainers"]], ["TRAINER_ALPHA"])
        occurrences = manifest["persistentConstants"]["occurrences"]["FLAG_TEST"]
        self.assertTrue(any(row["path"] == "data/scripts/shared.inc" for row in occurrences))

    def test_raw_shared_starter_symbol_fails_and_is_enumerated(self):
        root, policy_path = self.make_fixture()
        script = root / "data/maps/EmeraldMap/scripts.inc"
        script.write_text(script.read_text() + "\tswitch VAR_STARTER_MON\n", encoding="utf-8")
        manifest = self.run_fixture(root, policy_path)
        self.assertFalse(manifest["audit"]["passed"])
        self.assertEqual(len(manifest["starterVariable"]["rawSourceOccurrences"]), 1)
        self.assertIn("retain raw VAR_STARTER_MON", "\n".join(manifest["audit"]["failures"]))

    def test_authored_population_and_trainer_party_drift_fail(self):
        root, policy = self.make_fixture()
        wild_path = root / "src/data/wild_encounters.json"
        wild = json.loads(wild_path.read_text())
        wild["wild_encounter_groups"][0]["encounters"][0]["land_mons"] = {
            "mons": [{"species": "SPECIES_CHANGED", "min_level": 2, "max_level": 3}]
        }
        self.write_json(wild_path, wild)
        (root / "src/data/trainers.party").write_text(
            "=== TRAINER_ALPHA ===\nSPECIES_CHANGED @ 99\n", encoding="utf-8"
        )
        manifest = self.run_fixture(root, policy)
        failures = "\n".join(manifest["audit"]["failures"])
        self.assertIn("expectedWildProfilesSha256 changed", failures)
        self.assertIn("expectedTrainerPartiesSha256 changed", failures)

    def test_repository_catalog_has_all_518_maps_and_effective_shared_tables(self):
        manifest = GENERATOR.build_manifest(
            GAME_ROOT,
            MODULE_PATH.with_name("classification.json"),
            GAME_ROOT / "data/wayfarer_hoenn_source_constants.inc",
            GAME_ROOT / "data/wayfarer_engine_source_constants.inc",
        )
        self.assertTrue(manifest["audit"]["passed"], manifest["audit"]["failures"])
        self.assertEqual(manifest["summary"]["mapCount"], 518)
        by_name = {entry["name"]: entry for entry in manifest["maps"]}
        self.assertTrue(by_name["ContestHallBeauty"]["events"]["shared"])
        self.assertEqual(by_name["ContestHallBeauty"]["events"]["owner"], "ContestHall")
        self.assertEqual(by_name["Route104"]["wildEncounterMethods"], ["goodRod", "land", "oldRod", "superRod", "water"])
        self.assertGreater(len(by_name["DewfordTown_Gym"]["trainers"]), 1)
        statuses = {entry["system"]: entry["status"] for entry in manifest["optionalSystems"]}
        self.assertEqual(statuses["contests"], "included-with-named-limitation")
        self.assertEqual(statuses["multiplayer"], "excluded-from-first-milestone")


if __name__ == "__main__":
    unittest.main()
