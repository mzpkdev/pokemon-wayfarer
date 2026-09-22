import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest
import zlib


GAME_ROOT = Path(__file__).resolve().parents[3]
RETIRED_HNS_MAP_NAMES = (
    "CinnabarIsland_hns",
    "CinnabarIsland_PokemonCenter_hns",
    "SeafoamIslands_1F_hns",
    "SeafoamIslands_B1F_hns",
    "SeafoamIslands_Gym_hns",
    "SeafoamIslands_SecretCave_hns",
    "Route21_hns",
    "Route19_hns",
    "Route20_hns",
    "Route19_Cave_hns",
    "FuchsiaCity_Route19_Gate_hns",
)


class MapjsonWayfarerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build_dir = tempfile.TemporaryDirectory()
        cls.mapjson = Path(cls.build_dir.name) / "mapjson"
        subprocess.run(
            [
                os.environ.get("CXX", "g++"),
                "-std=c++17",
                str(GAME_ROOT / "tools/mapjson/mapjson.cpp"),
                str(GAME_ROOT / "tools/mapjson/json11.cpp"),
                "-o",
                str(cls.mapjson),
            ],
            check=True,
        )

    @classmethod
    def tearDownClass(cls):
        cls.build_dir.cleanup()

    def make_fixture(self):
        fixture = tempfile.TemporaryDirectory()
        root = Path(fixture.name)
        for directory in (
            "data/maps",
            "data/layouts",
            "include/constants",
            "src/data",
            "tools/mapjson",
        ):
            (root / directory).mkdir(parents=True, exist_ok=True)
        (root / "tools/mapjson/required_map_defines.json").write_text(
            json.dumps({"required_maps": [], "required_layouts": []})
        )
        return fixture, root

    @staticmethod
    def add_map(root, name, map_id, source_version, warps=None, region=None):
        map_dir = root / "data/maps" / name
        map_dir.mkdir()
        data = {
            "id": map_id,
            "name": name,
            "game_version": source_version,
            "warp_events": warps or [],
            "connections": [],
        }
        if region is not None:
            data["region"] = region
        (map_dir / "map.json").write_text(json.dumps(data))
        return map_dir / "map.json"

    def run_groups(self, root, version, map_files, manifest=None, sinnoh_manifest=None, sinnoh_assets=None):
        command = [
            str(self.mapjson),
            "groups",
            version,
            "data/maps/map_groups.json",
            *(str(path.relative_to(root)) for path in map_files),
            "data/maps",
            "include/constants",
        ]
        if manifest is not None:
            command.extend(
                [
                    "--wayfarer-sevii-manifest",
                    str(manifest.relative_to(root)),
                ]
            )
        if sinnoh_manifest is not None:
            command.extend(
                [
                    "--wayfarer-sinnoh-manifest",
                    str(sinnoh_manifest.relative_to(root)),
                ]
            )
        if sinnoh_assets is not None:
            command.extend(
                [
                    "--wayfarer-sinnoh-asset-manifest",
                    str(sinnoh_assets.relative_to(root)),
                ]
            )
        return subprocess.run(
            command,
            cwd=root,
            text=True,
            capture_output=True,
        )

    def run_map(self, root, version, map_file, layouts_file, manifest=None, sinnoh_manifest=None, sinnoh_assets=None):
        command = [
            str(self.mapjson),
            "map",
            version,
            str(map_file.relative_to(root)),
            str(layouts_file.relative_to(root)),
            str(map_file.parent.relative_to(root)),
        ]
        if manifest is not None:
            command.extend(
                [
                    "--wayfarer-sevii-manifest",
                    str(manifest.relative_to(root)),
                ]
            )
        if sinnoh_manifest is not None:
            command.extend(
                [
                    "--wayfarer-sinnoh-manifest",
                    str(sinnoh_manifest.relative_to(root)),
                ]
            )
        if sinnoh_assets is not None:
            command.extend(
                [
                    "--wayfarer-sinnoh-asset-manifest",
                    str(sinnoh_assets.relative_to(root)),
                ]
            )
        return subprocess.run(command, cwd=root, text=True, capture_output=True)

    def run_layouts(self, root, version, storage_mode=None, policy=None, report=None):
        command = [
            str(self.mapjson), "layouts", version,
            "data/layouts/layouts.json", "data/layouts", "include/constants",
        ]
        if policy is not None:
            command.extend(["--map-layout-storage-policy", str(policy.relative_to(root))])
        if storage_mode is not None:
            command.extend(["--map-layout-storage-mode", storage_mode])
        if report is not None:
            command.extend(["--map-layout-storage-report", str(report.relative_to(root))])
        if (root / "data/maps/map_groups.json").exists():
            command.extend(["--map-layout-canary-catalog", "data/maps/map_groups.json"])
        return subprocess.run(
            command,
            cwd=root,
            text=True,
            capture_output=True,
        )

    @staticmethod
    def add_layout(root, payload):
        layout_dir = root / "data/layouts/TestLayout"
        layout_dir.mkdir()
        (layout_dir / "border.bin").write_bytes(b"\0\0")
        (layout_dir / "map.bin").write_bytes(payload)
        (root / "data/layouts/layouts.json").write_text(json.dumps({
            "layouts_table_label": "gMapLayouts",
            "layouts": [{
                "id": "LAYOUT_TEST", "name": "Test_Layout",
                "game_version": "emerald", "width": 1, "height": 1,
                "border_filepath": "data/layouts/TestLayout/border.bin",
                "blockdata_filepath": "data/layouts/TestLayout/map.bin",
                "primary_tileset": "gTileset_Primary",
                "secondary_tileset": "gTileset_Secondary",
            }],
        }))

    def test_wayfarer_layouts_emit_raw_descriptor_for_complete_source_file(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        payload = b"\x34\x12\r\n\x1atrailing"
        self.add_layout(root, payload)

        result = self.run_layouts(root, "wayfarer")
        self.assertEqual(result.returncode, 0, result.stderr)
        generated = (root / "data/layouts/layouts.inc").read_text()
        table = (root / "data/layouts/layouts_table.inc").read_text()
        constants = (root / "include/constants/layouts.h").read_text()
        crc = zlib.crc32(payload)
        self.assertIn("__map_layout_payloads_start::", generated)
        self.assertIn("__map_layout_payloads_end::", generated)
        self.assertIn(".LTest_Layout_MapData:", generated)
        descriptor = generated.split(".LTest_Layout_MapData:", 1)[1].split("Test_Layout::", 1)[0]
        self.assertIn("\t.4byte .LTest_Layout_Blockdata", descriptor)
        self.assertEqual(descriptor.count(f"\t.4byte 0x{crc:X}"), 2)
        self.assertIn(f"\t.4byte {len(payload)}", descriptor)
        self.assertIn("\t.4byte 2", descriptor)
        self.assertIn("\t.byte 1\n\t.byte 0\n\t.2byte 0", descriptor)
        layout_record = generated.split("Test_Layout::", 1)[1]
        self.assertIn("\t.4byte .LTest_Layout_MapData", layout_record)
        self.assertIn("\t.if MAP_LAYOUT_TESTING_ASM", generated)
        self.assertIn(".LTest_Layout_RawOracle:", generated)
        self.assertIn("gMapLayoutPeakTestPayload::", generated)
        self.assertIn("gMapLayoutPeakTestDescriptor::", generated)
        self.assertIn("gMapLayoutPeakTestLayout::", generated)
        self.assertIn("\t.global gMapLayoutRawOracles", table)
        self.assertIn("\t.4byte .LTest_Layout_RawOracle", table)
        self.assertIn("#define MAP_LAYOUT_COUNT 1", constants)

        result = self.run_layouts(root, "emerald")
        self.assertEqual(result.returncode, 0, result.stderr)
        standalone = (root / "data/layouts/layouts.inc").read_text()
        self.assertNotIn("_MapData:", standalone)
        self.assertIn("\t.4byte .LTest_Layout_Blockdata", standalone)

    def test_wayfarer_hybrid_layouts_round_trip_and_report_compressed_storage(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        payload = b"\x34\x12" * 100
        self.add_layout(root, payload)
        policy = root / "storage.json"
        report = root / "build/storage.json"
        policy.write_text(json.dumps({"schema_version": 1, "default_policy": "auto", "rules": []}))

        result = self.run_layouts(root, "wayfarer", "hybrid", policy, report)
        self.assertEqual(result.returncode, 0, result.stderr)
        generated = (root / "data/layouts/layouts.inc").read_text()
        descriptor = generated.split(".LTest_Layout_MapData:", 1)[1].split("Test_Layout::", 1)[0]
        self.assertIn("\t.byte 1\n\t.byte 1\n\t.2byte 0", descriptor)
        production_payloads = generated.split("\t.if MAP_LAYOUT_TESTING_ASM", 1)[0]
        self.assertNotIn('map.bin"', production_payloads)
        storage_report = json.loads(report.read_text())
        self.assertEqual(storage_report["storage_mode"], "hybrid")
        self.assertEqual(storage_report["totals"]["compressed_entries"], 1)
        self.assertEqual(storage_report["totals"]["compressed_stored_payload_bytes"],
                         storage_report["totals"]["stored_payload_bytes"])
        self.assertIn("payload_alignment_bytes", storage_report["totals"])
        self.assertIn("codec_padding_bytes", storage_report["totals"])
        self.assertIn("raw_exception_entries", storage_report["totals"])
        self.assertIn("non_profitable_entries", storage_report["totals"])
        self.assertEqual(storage_report["totals_scope"],
                         "generated catalog including conditionally linked entries")
        self.assertIn("unconditionally_linked", storage_report["linkage_totals"])
        self.assertEqual(storage_report["missing_layouts"], [])
        self.assertEqual(storage_report["linked_net_savings"]["status"], "unavailable")
        self.assertEqual(storage_report["layouts"][0]["decoded_crc32"], f"0x{zlib.crc32(payload):X}")

    def test_wayfarer_legacy_size_layouts_emit_raw_records_without_descriptors(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        self.add_layout(root, b"\x34\x12" * 100)
        policy = root / "storage.json"
        report = root / "build/storage.json"
        policy.write_text(json.dumps({"schema_version": 1, "default_policy": "auto", "rules": []}))

        result = self.run_layouts(root, "wayfarer", "legacy", policy, report)
        self.assertEqual(result.returncode, 0, result.stderr)
        generated = (root / "data/layouts/layouts.inc").read_text()
        self.assertNotIn("_MapData:", generated)
        self.assertNotIn("__map_layout_payloads_start", generated)
        layout_record = generated.split("Test_Layout::", 1)[1]
        self.assertIn("\t.4byte .LTest_Layout_Blockdata", layout_record)
        storage_report = json.loads(report.read_text())
        self.assertEqual(storage_report["storage_mode"], "legacy")
        self.assertEqual(storage_report["totals"]["descriptor_bytes"], 0)

    def test_layout_storage_report_counts_forced_unprofitable_compression(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        self.add_layout(root, b"\x34\x12")
        policy = root / "storage.json"
        report = root / "report.json"
        policy.write_text(json.dumps({
            "schema_version": 1,
            "default_policy": "raw",
            "rules": [{"layout": "Test_Layout", "policy": "gba_lz77"}],
        }))

        result = self.run_layouts(root, "wayfarer", "hybrid", policy, report)
        self.assertEqual(result.returncode, 0, result.stderr)
        storage_report = json.loads(report.read_text())
        self.assertEqual(storage_report["totals"]["compressed_entries"], 1)
        self.assertEqual(storage_report["totals"]["non_profitable_entries"], 1)
        self.assertEqual(storage_report["totals"]["non_profitable_raw_bytes"], 2)
        self.assertGreater(storage_report["totals"]["non_profitable_candidate_bytes"], 4)

    def test_layout_storage_policy_rejects_malformed_rules(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        self.add_layout(root, b"\0\0")
        policy = root / "storage.json"
        for rules, message in (({}, "rules must be an array"),
                               (["not an object"], "rule must be an object")):
            with self.subTest(rules=rules):
                policy.write_text(json.dumps({
                    "schema_version": 1, "default_policy": "raw", "rules": rules,
                }))
                result = self.run_layouts(root, "wayfarer", "hybrid", policy)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(message, result.stderr)

    def test_layout_storage_policy_rejects_duplicate_rules(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        self.add_layout(root, b"\0\0")
        policy = root / "storage.json"
        policy.write_text(json.dumps({
            "schema_version": 1,
            "default_policy": "raw",
            "rules": [
                {"layout": "Missing", "policy": "raw", "reason": "fixture"},
                {"layout": "Missing", "policy": "raw", "reason": "duplicate"},
            ],
        }))

        result = self.run_layouts(root, "wayfarer", "hybrid", policy)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Duplicate map layout storage rule", result.stderr)

    def test_stage1_canary_must_not_be_a_connection_endpoint(self):
        for connections, message in (
            ([{"direction": "north", "offset": 0, "map": "MAP_OTHER"}], "MapConnection endpoint"),
            ([], None),
        ):
            with self.subTest(connections=connections):
                fixture, root = self.make_fixture()
                self.addCleanup(fixture.cleanup)
                self.add_layout(root, b"\x34\x12" * 100)
                test_map = self.add_map(root, "TestMap", "MAP_TEST", "emerald")
                map_data = json.loads(test_map.read_text())
                map_data["layout"] = "LAYOUT_TEST"
                map_data["connections"] = connections
                test_map.write_text(json.dumps(map_data))
                names = ["TestMap"]
                if connections:
                    other = self.add_map(root, "Other", "MAP_OTHER", "emerald")
                    other_data = json.loads(other.read_text())
                    other_data["layout"] = "LAYOUT_TEST"
                    other.write_text(json.dumps(other_data))
                    names.append("Other")
                (root / "data/maps/map_groups.json").write_text(json.dumps({
                    "group_order": ["gTest"], "gTest": names,
                    "connections_include_order": [],
                }))
                policy = root / "storage.json"
                report = root / "report.json"
                policy.write_text(json.dumps({
                    "schema_version": 1, "default_policy": "raw",
                    "rules": [{"layout": "Test_Layout", "policy": "gba_lz77",
                               "rollout_stage": "stage1"}],
                }))

                result = self.run_layouts(root, "wayfarer", "hybrid", policy, report)
                if message:
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn(message, result.stderr)
                else:
                    self.assertEqual(result.returncode, 0, result.stderr)
                    row = json.loads(report.read_text())["layouts"][0]
                    self.assertEqual(row["rollout_stage"], "stage1")

    def test_stage1_canary_rejects_special_immutable_consumer(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        self.add_layout(root, b"\x34\x12" * 100)
        layouts = json.loads((root / "data/layouts/layouts.json").read_text())
        layouts["layouts"][0]["name"] = "SecretBase_Test_Layout"
        (root / "data/layouts/layouts.json").write_text(json.dumps(layouts))
        test_map = self.add_map(root, "TestMap", "MAP_TEST", "emerald")
        map_data = json.loads(test_map.read_text())
        map_data["layout"] = "LAYOUT_TEST"
        test_map.write_text(json.dumps(map_data))
        (root / "data/maps/map_groups.json").write_text(json.dumps({
            "group_order": ["gTest"], "gTest": ["TestMap"],
            "connections_include_order": [],
        }))
        policy = root / "storage.json"
        policy.write_text(json.dumps({
            "schema_version": 1, "default_policy": "raw",
            "rules": [{"layout": "SecretBase_Test_Layout", "policy": "gba_lz77",
                       "rollout_stage": "stage1"}],
        }))

        result = self.run_layouts(root, "wayfarer", "hybrid", policy)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("special immutable consumer", result.stderr)

    def test_layout_generation_rejects_source_shorter_than_logical_tiles(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        self.add_layout(root, b"\x00")

        result = self.run_layouts(root, "wayfarer")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Invalid layout payload dimensions or length", result.stderr)

    def test_layout_generation_rejects_missing_selected_border(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        self.add_layout(root, b"\0\0")
        (root / "data/layouts/TestLayout/border.bin").unlink()

        result = self.run_layouts(root, "wayfarer")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing border file", result.stderr)

    def test_layout_generation_rejects_invalid_or_oversized_dimensions(self):
        for width, height, payload_size in ((-1, -1, 2), (100, 100, 20000)):
            with self.subTest(width=width, height=height):
                fixture, root = self.make_fixture()
                self.addCleanup(fixture.cleanup)
                self.add_layout(root, b"\0" * payload_size)
                catalog_path = root / "data/layouts/layouts.json"
                catalog = json.loads(catalog_path.read_text())
                catalog["layouts"][0]["width"] = width
                catalog["layouts"][0]["height"] = height
                catalog_path.write_text(json.dumps(catalog))

                result = self.run_layouts(root, "wayfarer")
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("do not fit the runtime backup map", result.stderr)

    def test_wayfarer_unlinks_replaced_and_orphaned_hns_maps_and_layouts(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        names = list(RETIRED_HNS_MAP_NAMES)
        source_maps = {
            name: json.loads((GAME_ROOT / "data/maps" / name / "map.json").read_text())
            for name in names
        }
        map_files = [
            self.add_map(root, name, source_maps[name]["id"], "hns")
            for name in names
        ]
        (root / "data/maps/map_groups.json").write_text(json.dumps({
            "group_order": ["gHns"], "gHns": names, "connections_include_order": [],
        }))
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))

        layouts = []
        for name in names:
            layout_dir = root / "data/layouts" / name
            layout_dir.mkdir()
            (layout_dir / "border.bin").write_bytes(b"\0\0")
            (layout_dir / "map.bin").write_bytes(b"\0\0")
            layouts.append({
                "id": source_maps[name]["layout"], "name": f"{name}_Layout",
                "game_version": "hns", "width": 1, "height": 1,
                "border_filepath": f"data/layouts/{name}/border.bin",
                "blockdata_filepath": f"data/layouts/{name}/map.bin",
                "primary_tileset": "gTileset_Primary", "secondary_tileset": "gTileset_Secondary",
            })
        (root / "data/layouts/layouts.json").write_text(json.dumps({
            "layouts_table_label": "gMapLayouts", "layouts": layouts,
        }))

        result = self.run_groups(root, "wayfarer", map_files)
        self.assertEqual(result.returncode, 0, result.stderr)
        groups = (root / "data/maps/groups.inc").read_text()
        headers = (root / "data/maps/headers.inc").read_text()
        events = (root / "data/maps/events.inc").read_text()
        for name in RETIRED_HNS_MAP_NAMES:
            self.assertNotIn(f"\t.4byte {name}\n", groups)
            self.assertNotIn(f"/{name}/header.inc", headers)
            self.assertNotIn(f"/{name}/events.inc", events)

        result = self.run_layouts(root, "wayfarer")
        self.assertEqual(result.returncode, 0, result.stderr)
        wayfarer_map_constants = (root / "include/constants/map_groups.h").read_text()
        wayfarer_layout_constants = (root / "include/constants/layouts.h").read_text()
        layout_headers = (root / "data/layouts/layouts.inc").read_text()
        layout_table = (root / "data/layouts/layouts_table.inc").read_text()
        for name in RETIRED_HNS_MAP_NAMES:
            self.assertNotIn(f"{name}_Layout::", layout_headers)
            self.assertNotIn(f"\t.4byte {name}_Layout\n", layout_table)

        result = self.run_groups(root, "hns", map_files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_layouts(root, "hns")
        self.assertEqual(result.returncode, 0, result.stderr)
        hns_groups = (root / "data/maps/groups.inc").read_text()
        hns_layouts = (root / "data/layouts/layouts.inc").read_text()
        self.assertEqual((root / "include/constants/map_groups.h").read_text(), wayfarer_map_constants)
        self.assertEqual((root / "include/constants/layouts.h").read_text(), wayfarer_layout_constants)
        for name in RETIRED_HNS_MAP_NAMES:
            self.assertIn(f"\t.4byte {name}\n", hns_groups)
            self.assertIn(f"{name}_Layout::", hns_layouts)

    def test_route20_seafoam_warps_follow_selected_build(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        route_name = "Route20_hns"
        route_source = json.loads((GAME_ROOT / "data/maps" / route_name / "map.json").read_text())
        route_map = self.add_map(root, route_name, route_source["id"], "hns")
        route_map.write_text(json.dumps(route_source))
        self.assertEqual(
            [(warp["dest_map"], warp["dest_warp_id"]) for warp in route_source["warp_events"]],
            [("MAP_SEAFOAM_ISLANDS_1F_HNS", "0"), ("MAP_SEAFOAM_ISLANDS_1F_HNS", "3")],
        )
        old_seafoam = self.add_map(
            root, "SeafoamIslands_1F_hns", "MAP_SEAFOAM_ISLANDS_1F_HNS", "hns"
        )
        coast_seafoam = self.add_map(
            root, "SeafoamIslands_1F_CoastPoc", "MAP_SEAFOAM_ISLANDS_1F_COAST_POC", "hns"
        )
        route19 = self.add_map(root, "Route19_hns", "MAP_ROUTE19_HNS", "hns")
        cinnabar_seam = self.add_map(root, "CinnabarIsland_SeamPoc", "MAP_CINNABAR_SEAM_POC", "hns")
        (root / "data/maps/map_groups.json").write_text(json.dumps({
            "group_order": ["gHns"],
            "gHns": [route_name, "Route19_hns", "CinnabarIsland_SeamPoc", "SeafoamIslands_1F_hns", "SeafoamIslands_1F_CoastPoc"],
            "connections_include_order": [],
        }))
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        source_layouts = json.loads((GAME_ROOT / "data/layouts/layouts.json").read_text())
        route_layout = next(
            layout for layout in source_layouts["layouts"]
            if layout.get("id") == route_source["layout"]
        )
        for key in ("border_filepath", "blockdata_filepath"):
            path = root / route_layout[key]
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"\0\0")
        layouts_file = root / "data/layouts/layouts.json"
        layouts_file.write_text(json.dumps({"layouts": [route_layout]}))

        map_files = [route_map, route19, cinnabar_seam, old_seafoam, coast_seafoam]
        result = self.run_groups(root, "wayfarer", map_files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_map(root, "wayfarer", route_map, layouts_file)
        self.assertEqual(result.returncode, 0, result.stderr)
        wayfarer_events = (route_map.parent / "events.inc").read_text()
        self.assertIn("\twarp_def 60, 8, 0, 3, MAP_SEAFOAM_ISLANDS_1F_COAST_POC", wayfarer_events)
        self.assertIn("\twarp_def 72, 14, 0, 4, MAP_SEAFOAM_ISLANDS_1F_COAST_POC", wayfarer_events)

        result = self.run_groups(root, "hns", map_files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_map(root, "hns", route_map, layouts_file)
        self.assertEqual(result.returncode, 0, result.stderr)
        hns_events = (route_map.parent / "events.inc").read_text()
        self.assertIn("\twarp_def 60, 8, 0, 0, MAP_SEAFOAM_ISLANDS_1F_HNS", hns_events)
        self.assertIn("\twarp_def 72, 14, 0, 3, MAP_SEAFOAM_ISLANDS_1F_HNS", hns_events)

    def test_seafoam_exits_return_to_selected_route20_only_in_wayfarer(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        name = "SeafoamIslands_1F_CoastPoc"
        source = json.loads((GAME_ROOT / "data/maps" / name / "map.json").read_text())
        seafoam = self.add_map(root, name, source["id"], "hns")
        seafoam.write_text(json.dumps(source))
        route20_hns = self.add_map(root, "Route20_hns", "MAP_ROUTE20_HNS", "hns")
        route20_poc = self.add_map(root, "Route20_CoastPoc", "MAP_ROUTE20_COAST_POC", "hns")
        b1f = self.add_map(root, "SeafoamIslands_B1F_CoastPoc", "MAP_SEAFOAM_ISLANDS_B1F_COAST_POC", "hns")
        (root / "data/maps/map_groups.json").write_text(json.dumps({
            "group_order": ["gHns"],
            "gHns": [name, "Route20_hns", "Route20_CoastPoc", "SeafoamIslands_B1F_CoastPoc"],
            "connections_include_order": [],
        }))
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        source_layouts = json.loads((GAME_ROOT / "data/layouts/layouts.json").read_text())
        layout = next(row for row in source_layouts["layouts"] if row.get("id") == source["layout"])
        for key in ("border_filepath", "blockdata_filepath"):
            path = root / layout[key]
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"\0\0")
        layouts_file = root / "data/layouts/layouts.json"
        layouts_file.write_text(json.dumps({"layouts": [layout]}))
        files = [seafoam, route20_hns, route20_poc, b1f]

        result = self.run_groups(root, "wayfarer", files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_map(root, "wayfarer", seafoam, layouts_file)
        self.assertEqual(result.returncode, 0, result.stderr)
        events = (seafoam.parent / "events.inc").read_text()
        self.assertIn("\twarp_def 6, 21, 3, 0, MAP_ROUTE20_HNS", events)
        self.assertIn("\twarp_def 32, 21, 3, 1, MAP_ROUTE20_HNS", events)

        result = self.run_groups(root, "hns", files)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_map(root, "hns", seafoam, layouts_file)
        self.assertEqual(result.returncode, 0, result.stderr)
        events = (seafoam.parent / "events.inc").read_text()
        self.assertIn("\twarp_def 6, 21, 15, 0, MAP_ROUTE20_COAST_POC", events)
        self.assertIn("\twarp_def 32, 21, 15, 1, MAP_ROUTE20_COAST_POC", events)

    @staticmethod
    def write_sevii_manifest(root, maps, release_link_enabled=False):
        inventory = []
        for map_record in maps:
            map_record.setdefault("retained_events", {})
            for event_kind in ("object_events", "warp_events", "coord_events", "bg_events"):
                map_record["retained_events"].setdefault(event_kind, [])
                for row in map_record["retained_events"][event_kind]:
                    if not isinstance(row, dict):
                        continue
                    content_id = row.setdefault(
                        "content_id",
                        f"exploration.{map_record['source_map'].lower()}.{event_kind}.{row['index']}",
                    )
                    row.setdefault("owner", "exploration")
                    row.setdefault("reason", "Fixture preserves a reviewed exploration event.")
                    inventory.append(content_id)
            map_record.setdefault("retained_map_scripts", [])
            map_record.setdefault("encounter_methods", [])
        path = root / "src/data/wayfarer_sevii_maps.json"
        path.write_text(
            json.dumps(
                {
                    "schema_version": 2,
                    "release_link_enabled": release_link_enabled,
                    "baseline": {
                        "projection_sha256": "0" * 64,
                        "wild_encounters_sha256": "0" * 64,
                        "event_island_baseline_sha256": "0" * 64,
                    },
                    "content_domains": {
                        "exploration": {"owner": "exploration", "enabled": True, "inventory": inventory},
                        "ordinary_trainers": {"owner": "ordinary_trainer", "enabled": False, "inventory": []},
                        "story": {"owner": "story", "enabled": False, "inventory": []},
                        "trainer_tower": {"owner": "trainer_tower", "enabled": False, "inventory": []},
                    },
                    "maps": maps,
                    "exclusions": [],
                }
            )
        )
        return path

    @staticmethod
    def frozen_sinnoh_records(maps, frozen=True):
        groups = [
            ("gMapGroup_SinnohTownsRoutes", 55), ("gMapGroup_SpecialAreasSinnoh", 5),
            ("gMapGroup_DungeonsSinnoh", 8), ("gMapGroup_IndoorSinnoh", 14),
            ("gMapGroup_IndoorTwinleaf", 6), ("gMapGroup_IndoorSandgem", 6),
            ("gMapGroup_IndoorJubilife", 20), ("gMapGroup_IndoorOreburgh", 13),
            ("gMapGroup_IndoorFloaroma", 6),
        ]
        records = [dict(record) for record in maps]
        if not frozen:
            groups = groups[:1]
        for record in records:
            record.setdefault("source_group", groups[0][0])
        for group_order, (group, count) in enumerate(groups):
            while sum(record.get("source_group") == group for record in records) < count:
                index = len(records)
                records.append({
                    "source_map": f"FrozenMap{index}",
                    "source_map_id": f"MAP_FROZEN_{index}",
                    "target_map": f"FrozenMap{index}",
                    "target_map_id": f"MAP_FROZEN_{index}",
                    "source_layout": f"LAYOUT_FROZEN_{index}",
                    "target_layout": f"LAYOUT_FROZEN_{index}",
                    "source_group": group,
                })
        source_group_orders = {group: 0 for group, _ in groups}
        for index, record in enumerate(records):
            group_order = next(
                group_index for group_index, (group, _) in enumerate(groups)
                if record.get("source_group", groups[0][0]) == group
            )
            record.setdefault("source_group", groups[group_order][0])
            record.setdefault("source_map", f"FrozenMap{index}")
            record.setdefault("source_map_id", f"MAP_FROZEN_{index}")
            record.setdefault("target_map", record["source_map"])
            record.setdefault("target_map_id", record["source_map_id"])
            record.setdefault("source_layout", f"LAYOUT_FROZEN_{index}")
            record.setdefault("target_layout", record["source_layout"])
            record["order"] = index
            record["source_group_order"] = source_group_orders[record["source_group"]]
            source_group_orders[record["source_group"]] += 1
            record.setdefault("layout_format", "emerald")
            record.setdefault("warps", [])
            record.setdefault("connections", [])
            record.setdefault("empty_content", {
                "object_events": 0, "coord_events": 0, "bg_events": 0,
                "map_scripts": 0, "wild_encounter_profiles": 0,
            })
            record.setdefault("asset_records", {
                "blockdata": f"blockdata.{record['target_layout']}",
                "border": f"border.{record['target_layout']}",
                "tilesets": [],
            })
            record.setdefault("inclusion", {"state": "FROZEN_NOT_SELECTED"})
        if frozen:
            records[0]["warps"] = [{}] * 233
            records[0]["connections"] = [{}] * 114
        return records, groups

    @classmethod
    def write_sinnoh_manifest(
        cls, root, maps, release_link_enabled=False, asset_manifest_ready=True, blockers=None, frozen=True,
    ):
        records, groups = cls.frozen_sinnoh_records(maps, frozen)
        path = root / "src/data/wayfarer_sinnoh_maps.json"
        path.write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "donor": {
                        "url": "https://github.com/LiderMorti00/Sinnoh-pokeemerald-expansion",
                        "commit": "4eed17cc63c4ec8c24fbb20fe49e8d65cb4870d8",
                    },
                    "selection": {
                        "release_link_enabled": release_link_enabled,
                        "asset_manifest_ready": asset_manifest_ready,
                        "blockers": blockers or [],
                    },
                    "source_groups": [
                        {"source_group": group, "target_group": group, "order": order, "map_count": count}
                        for order, (group, count) in enumerate(groups)
                    ],
                    "expected_counts": {
                        "maps": 133, "layouts": 133, "warps": 233, "connections": 114,
                        "object_events": 0, "coord_events": 0, "bg_events": 0,
                        "nonempty_map_scripts": 0, "wild_encounter_profiles": 0,
                    },
                    "maps": records,
                }
            )
        )
        return path

    @classmethod
    def write_sinnoh_asset_manifest(
        cls, root, maps, release_link_enabled=False, asset_manifest_ready=True, blockers=None,
        review_required=False, frozen=True,
    ):
        records, _ = cls.frozen_sinnoh_records(maps, frozen)
        assets = []
        for record in records:
            for asset_id in (record["asset_records"]["blockdata"], record["asset_records"]["border"]):
                assets.append({
                    "record_id": asset_id,
                    "reuse_class": "REVIEW_REQUIRED" if review_required and not assets else "SINNOH_NEW",
                    "selection_blocker": False,
                })
        path = root / "src/data/wayfarer_sinnoh_assets.json"
        path.write_text(json.dumps({
            "schema_version": 1,
            "donor": {
                "url": "https://github.com/LiderMorti00/Sinnoh-pokeemerald-expansion",
                "commit": "4eed17cc63c4ec8c24fbb20fe49e8d65cb4870d8",
            },
            "selection": {
                "release_link_enabled": release_link_enabled,
                "asset_manifest_ready": asset_manifest_ready,
                "blockers": blockers or [],
            },
            "records": assets,
        }))
        return path

    def test_wayfarer_selects_hns_and_emerald_without_mutating_heal_data(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(
            root, "HnsMap", "MAP_HNS", "hns", region="REGION_JOHTO"
        )
        emerald = self.add_map(root, "EmeraldMap", "MAP_EMERALD", "emerald")
        frlg = self.add_map(root, "FrlgMap", "MAP_FRLG", "frlg")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gEmerald", "gFrlg"],
                    "gHns": ["HnsMap"],
                    "gEmerald": ["EmeraldMap"],
                    "gFrlg": ["FrlgMap"],
                    "connections_include_order": [],
                }
            )
        )
        heal_data = json.dumps(
            {
                "heal_locations": [
                    {"id": "HEAL_HNS", "source": "HNS", "map": "MAP_HNS"},
                    {
                        "id": "HEAL_EMERALD",
                        "source": "EMERALD",
                        "map": "MAP_EMERALD",
                        "respawn_map": "MAP_EMERALD",
                        "respawn_npc": "LOCALID_NURSE",
                    },
                ]
            },
            indent=2,
        )
        heal_path = root / "src/data/heal_locations.json"
        heal_path.write_text(heal_data)

        result = self.run_groups(root, "wayfarer", [hns, emerald, frlg])

        self.assertEqual(result.returncode, 0, result.stderr)
        groups = (root / "data/maps/groups.inc").read_text()
        self.assertIn("\t.4byte HnsMap", groups)
        self.assertIn("\t.4byte EmeraldMap", groups)
        self.assertIn("gFrlg::\n\t.4byte NULL", groups)
        headers = (root / "data/maps/headers.inc").read_text()
        self.assertIn(
            '\t.include "data/wayfarer_hoenn_source_constants.inc"\n'
            '\t.include "data/maps/EmeraldMap/header.inc"',
            headers,
        )
        self.assertTrue(headers.rstrip().endswith(
            '\t.include "data/wayfarer_engine_source_constants.inc"'
        ))
        self.assertEqual(heal_path.read_text(), heal_data)
        map_sources = (root / "src/data/wayfarer_map_sources.h").read_text()
        self.assertIn("sWayfarerMapSourceOffsets[] = {0, 1, 2, 3, }", map_sources)
        self.assertIn("sWayfarerHoennMapSourceBits[] = {2, }", map_sources)
        self.assertIn(
            "sWayfarerMapRegionNibbles[] = "
            "{(REGION_JOHTO | (REGION_NONE << 4)), "
            "(REGION_NONE | (REGION_NONE << 4)), }",
            map_sources,
        )

    def test_wayfarer_rejects_unavailable_warp_destination(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(
            root,
            "HnsMap",
            "MAP_HNS",
            "hns",
            [{"dest_map": "MAP_FRLG"}],
        )
        frlg = self.add_map(root, "FrlgMap", "MAP_FRLG", "frlg")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gFrlg"],
                    "gHns": ["HnsMap"],
                    "gFrlg": ["FrlgMap"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps({"heal_locations": []})
        )

        result = self.run_groups(root, "wayfarer", [hns, frlg])

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("warp references unavailable map MAP_FRLG", result.stderr)

    def test_wayfarer_sevii_manifest_is_allowlist_based_and_release_gated(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(root, "HnsMap", "MAP_HNS", "hns")
        frlg = self.add_map(root, "OneIsland_Harbor", "MAP_SEVII", "frlg")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gFrlg"],
                    "gHns": ["HnsMap"],
                    "gFrlg": ["OneIsland_Harbor"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps({"heal_locations": []})
        )
        record = {
            "source_map": "OneIsland_Harbor",
            "map_id": "MAP_SEVII",
            "layout": "LAYOUT_SEVII",
        }
        manifest = self.write_sevii_manifest(root, [record])

        result = self.run_groups(root, "wayfarer", [hns, frlg], manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("gFrlg::\n\t.4byte NULL", (root / "data/maps/groups.inc").read_text())

        manifest = self.write_sevii_manifest(root, [record], release_link_enabled=True)
        result = self.run_groups(root, "wayfarer", [hns, frlg], manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("gFrlg::\n\t.4byte OneIsland_Harbor", (root / "data/maps/groups.inc").read_text())

        record["enabled"] = False
        manifest = self.write_sevii_manifest(root, [record], release_link_enabled=True)
        result = self.run_groups(root, "wayfarer", [hns, frlg], manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("gFrlg::\n\t.4byte NULL", (root / "data/maps/groups.inc").read_text())

    def test_wayfarer_sinnoh_manifest_is_the_only_selection_gate(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(root, "HnsMap", "MAP_HNS", "hns")
        sinnoh = self.add_map(root, "TwinleafTown", "MAP_TWINLEAF_TOWN", "sinnoh")
        sinnoh_data = json.loads(sinnoh.read_text())
        sinnoh_data["layout"] = "LAYOUT_TWINLEAF_TOWN"
        sinnoh.write_text(json.dumps(sinnoh_data))
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gSinnoh"],
                    "gHns": ["HnsMap"],
                    "gSinnoh": ["TwinleafTown"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        record = {
            "source_map": "TwinleafTown",
            "source_map_id": "MAP_TWINLEAF_TOWN",
            "target_map_id": "MAP_TWINLEAF_TOWN",
            "source_layout": "LAYOUT_TWINLEAF_TOWN",
            "target_layout": "LAYOUT_TWINLEAF_TOWN",
            "inclusion": {"state": "FROZEN_NOT_SELECTED"},
        }
        manifest = self.write_sinnoh_manifest(root, [record])
        assets = self.write_sinnoh_asset_manifest(root, [record])

        result = self.run_groups(root, "wayfarer", [hns, sinnoh], sinnoh_manifest=manifest, sinnoh_assets=assets)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(
            "gSinnoh::\n\t.if HAS_SINNOH_CONTENT_ASM\n\t.4byte TwinleafTown\n\t.else\n\t.4byte NULL\n\t.endif",
            (root / "data/maps/groups.inc").read_text(),
        )

        record["inclusion"]["state"] = "INCLUDED"
        manifest = self.write_sinnoh_manifest(root, [record], release_link_enabled=True)
        assets = self.write_sinnoh_asset_manifest(root, [record], release_link_enabled=True)
        result = self.run_groups(root, "wayfarer", [hns, sinnoh], sinnoh_manifest=manifest, sinnoh_assets=assets)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("gSinnoh::\n\t.4byte TwinleafTown", (root / "data/maps/groups.inc").read_text())

        for standalone_version in ("hns", "emerald", "firered"):
            result = self.run_groups(root, standalone_version, [hns, sinnoh])
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("gSinnoh::\n\t.4byte NULL", (root / "data/maps/groups.inc").read_text())

    def test_wayfarer_pending_sinnoh_map_has_no_script_table(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        map_dir = root / "data/maps/TwinleafTown"
        map_dir.mkdir()
        source = json.loads((GAME_ROOT / "data/maps/TwinleafTown/map.json").read_text())
        map_file = map_dir / "map.json"
        map_file.write_text(json.dumps(source))
        (root / "include/constants/map_groups.h").write_text(
            (GAME_ROOT / "include/constants/map_groups.h").read_text()
        )
        layout = next(
            row for row in json.loads((GAME_ROOT / "data/layouts/layouts.json").read_text())["layouts"]
            if row["id"] == source["layout"]
        )
        layout["border_filepath"] = "data/layouts/TwinleafTown/border.bin"
        layout["blockdata_filepath"] = "data/layouts/TwinleafTown/map.bin"
        layout_dir = root / "data/layouts/TwinleafTown"
        layout_dir.mkdir()
        (layout_dir / "border.bin").write_bytes(b"\0" * 8)
        (layout_dir / "map.bin").write_bytes(b"\0" * (int(layout["width"]) * int(layout["height"]) * 2))
        layouts_file = root / "data/layouts/layouts.json"
        layouts_file.write_text(json.dumps({"layouts": [layout]}))
        record = {
            "source_map": "TwinleafTown", "source_map_id": "MAP_TWINLEAF_TOWN",
            "target_map": "TwinleafTown", "target_map_id": "MAP_TWINLEAF_TOWN",
            "source_layout": source["layout"], "target_layout": source["layout"],
        }
        manifest = self.write_sinnoh_manifest(root, [record])
        assets = self.write_sinnoh_asset_manifest(root, [record])

        result = self.run_map(root, "wayfarer", map_file, layouts_file, sinnoh_manifest=manifest, sinnoh_assets=assets)

        self.assertEqual(result.returncode, 0, result.stderr)
        header = (map_dir / "header.inc").read_text()
        self.assertIn("\t.4byte TwinleafTown_MapEvents\n\t.4byte NULL\n", header)
        self.assertNotIn("TwinleafTown_MapScripts", header)

    def test_wayfarer_sinnoh_release_requires_ready_asset_manifest_without_blockers(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        sinnoh = self.add_map(root, "TwinleafTown", "MAP_TWINLEAF_TOWN", "sinnoh")
        sinnoh_data = json.loads(sinnoh.read_text())
        sinnoh_data["layout"] = "LAYOUT_TWINLEAF_TOWN"
        sinnoh.write_text(json.dumps(sinnoh_data))
        (root / "data/maps/map_groups.json").write_text(
            json.dumps({"group_order": ["gSinnoh"], "gSinnoh": ["TwinleafTown"], "connections_include_order": []})
        )
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        record = {
            "source_map": "TwinleafTown",
            "source_map_id": "MAP_TWINLEAF_TOWN",
            "target_map_id": "MAP_TWINLEAF_TOWN",
            "source_layout": "LAYOUT_TWINLEAF_TOWN",
            "target_layout": "LAYOUT_TWINLEAF_TOWN",
            "inclusion": {"state": "INCLUDED"},
        }
        manifest = self.write_sinnoh_manifest(
            root, [record], release_link_enabled=True, asset_manifest_ready=False,
        )
        assets = self.write_sinnoh_asset_manifest(
            root, [record], release_link_enabled=True, asset_manifest_ready=False,
        )
        result = self.run_groups(root, "wayfarer", [sinnoh], sinnoh_manifest=manifest, sinnoh_assets=assets)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("asset manifest is ready and blockers are clear", result.stderr)

        manifest = self.write_sinnoh_manifest(
            root, [record], release_link_enabled=True, blockers=["REVIEW_REQUIRED"],
        )
        assets = self.write_sinnoh_asset_manifest(
            root, [record], release_link_enabled=True, blockers=["REVIEW_REQUIRED"],
        )
        result = self.run_groups(root, "wayfarer", [sinnoh], sinnoh_manifest=manifest, sinnoh_assets=assets)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("asset manifest is ready and blockers are clear", result.stderr)

    def test_wayfarer_sinnoh_release_rejects_a_partial_frozen_catalog(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        sinnoh = self.add_map(root, "TwinleafTown", "MAP_TWINLEAF_TOWN", "sinnoh")
        sinnoh_data = json.loads(sinnoh.read_text())
        sinnoh_data["layout"] = "LAYOUT_TWINLEAF_TOWN"
        sinnoh.write_text(json.dumps(sinnoh_data))
        (root / "data/maps/map_groups.json").write_text(
            json.dumps({"group_order": ["gSinnoh"], "gSinnoh": ["TwinleafTown"], "connections_include_order": []})
        )
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        record = {
            "source_map": "TwinleafTown", "source_map_id": "MAP_TWINLEAF_TOWN",
            "target_map_id": "MAP_TWINLEAF_TOWN", "source_layout": "LAYOUT_TWINLEAF_TOWN",
            "target_layout": "LAYOUT_TWINLEAF_TOWN", "inclusion": {"state": "INCLUDED"},
        }
        manifest = self.write_sinnoh_manifest(root, [record], release_link_enabled=True, frozen=False)
        assets = self.write_sinnoh_asset_manifest(root, [record], release_link_enabled=True, frozen=False)
        result = self.run_groups(root, "wayfarer", [sinnoh], sinnoh_manifest=manifest, sinnoh_assets=assets)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("complete frozen catalog", result.stderr)

    def test_wayfarer_sinnoh_release_rejects_unresolved_asset_rows(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        sinnoh = self.add_map(root, "TwinleafTown", "MAP_TWINLEAF_TOWN", "sinnoh")
        sinnoh_data = json.loads(sinnoh.read_text())
        sinnoh_data["layout"] = "LAYOUT_TWINLEAF_TOWN"
        sinnoh.write_text(json.dumps(sinnoh_data))
        (root / "data/maps/map_groups.json").write_text(
            json.dumps({"group_order": ["gSinnoh"], "gSinnoh": ["TwinleafTown"], "connections_include_order": []})
        )
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        record = {
            "source_map": "TwinleafTown", "source_map_id": "MAP_TWINLEAF_TOWN",
            "target_map_id": "MAP_TWINLEAF_TOWN", "source_layout": "LAYOUT_TWINLEAF_TOWN",
            "target_layout": "LAYOUT_TWINLEAF_TOWN", "inclusion": {"state": "INCLUDED"},
        }
        manifest = self.write_sinnoh_manifest(root, [record], release_link_enabled=True)
        assets = self.write_sinnoh_asset_manifest(root, [record], release_link_enabled=True, review_required=True)
        result = self.run_groups(root, "wayfarer", [sinnoh], sinnoh_manifest=manifest, sinnoh_assets=assets)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("contains unresolved release asset", result.stderr)

    def test_wayfarer_sinnoh_manifest_rejects_unreviewed_target_ids(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        sinnoh = self.add_map(root, "TwinleafTown", "MAP_TWINLEAF_TOWN", "sinnoh")
        sinnoh_data = json.loads(sinnoh.read_text())
        sinnoh_data["layout"] = "LAYOUT_TWINLEAF_TOWN"
        sinnoh.write_text(json.dumps(sinnoh_data))
        (root / "data/maps/map_groups.json").write_text(
            json.dumps({"group_order": ["gSinnoh"], "gSinnoh": ["TwinleafTown"], "connections_include_order": []})
        )
        (root / "src/data/heal_locations.json").write_text(json.dumps({"heal_locations": []}))
        manifest = self.write_sinnoh_manifest(
            root,
            [
                {
                    "source_map": "TwinleafTown",
                    "source_map_id": "MAP_TWINLEAF_TOWN",
                    "target_map_id": "MAP_UNREVIEWED",
                    "source_layout": "LAYOUT_TWINLEAF_TOWN",
                    "target_layout": "LAYOUT_TWINLEAF_TOWN",
                    "inclusion": {"state": "INCLUDED"},
                }
            ],
            release_link_enabled=True,
        )
        assets = self.write_sinnoh_asset_manifest(root, [
            {
                "source_map": "TwinleafTown", "source_map_id": "MAP_TWINLEAF_TOWN",
                "target_map_id": "MAP_UNREVIEWED", "source_layout": "LAYOUT_TWINLEAF_TOWN",
                "target_layout": "LAYOUT_TWINLEAF_TOWN", "inclusion": {"state": "INCLUDED"},
            }
        ], release_link_enabled=True)

        result = self.run_groups(root, "wayfarer", [sinnoh], sinnoh_manifest=manifest, sinnoh_assets=assets)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("does not match its reviewed target IDs", result.stderr)

    def test_rejects_unsupported_map_provenance(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        unknown = self.add_map(root, "UnknownMap", "MAP_UNKNOWN", "crystal")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps({"group_order": ["gUnknown"], "gUnknown": ["UnknownMap"], "connections_include_order": []})
        )

        result = self.run_groups(root, "emerald", [unknown])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Unsupported map source version crystal", result.stderr)

    def test_wayfarer_sevii_events_strip_story_and_link_room_warps(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        map_dir = root / "data/maps/OneIsland_PokemonCenter_2F_Frlg"
        map_dir.mkdir()
        source = {
            "id": "MAP_SEVII_CENTER_2F",
            "name": "OneIsland_PokemonCenter_2F_Frlg",
            "game_version": "frlg",
            "layout": "LAYOUT_SEVII_CENTER_2F",
            "music": "MUS_NONE",
            "region_map_section": "MAPSEC_NONE",
            "requires_flash": False,
            "weather": "WEATHER_NONE",
            "map_type": "MAP_TYPE_INDOOR",
            "allow_cycling": False,
            "allow_escaping": False,
            "allow_running": False,
            "show_map_name": False,
            "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
            "object_events": [
                {"type": "object", "graphics_id": "OBJ_EVENT_GFX_TRAINER", "x": 1, "y": 1,
                 "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                 "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NORMAL",
                 "trainer_sight_or_berry_tree_id": "1", "script": "SevenIsland_SevaultCanyon_House_EventScript_ItemLuckyPunch", "flag": "0"},
                {"type": "object", "graphics_id": "OBJ_EVENT_GFX_NURSE", "x": 2, "y": 1,
                 "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                 "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NONE",
                 "trainer_sight_or_berry_tree_id": "0", "script": "SixIsland_WaterPath_House2_EventScript_Man", "flag": "0"},
            ],
            "warp_events": [
                {"x": 1, "y": 5, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_SEVII_1F"},
                {"x": 5, "y": 1, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_UNION_ROOM_FRLG"},
                {"x": 9, "y": 1, "elevation": 0, "dest_warp_id": "0", "dest_map": "MAP_TRADE_CENTER_FRLG"},
            ],
            "coord_events": [{"type": "trigger", "x": 1, "y": 1, "elevation": 0,
                              "var": "VAR_TEMP_0", "var_value": "0", "script": "SevenIsland_SevaultCanyon_House_EventScript_ChanseyDanceMan"}],
            "bg_events": [{"type": "sign", "x": 1, "y": 2, "elevation": 0,
                           "player_facing_dir": "BG_EVENT_PLAYER_FACING_ANY", "script": "SevenIsland_SevaultCanyon_House_EventScript_Chansey"}],
            "connections": [{"direction": "up", "offset": 0, "map": "MAP_SEVII_1F"}],
        }
        map_file = map_dir / "map.json"
        map_file.write_text(json.dumps(source))
        (root / "include/constants/map_groups.h").write_text(
            "enum { MAP_SEVII_CENTER_2F = (0 | (0 << 8)), MAP_SEVII_1F = (1 | (0 << 8)), };\n"
        )
        layout_file = root / "data/layouts/layouts.json"
        (root / "data/layouts/center.border.bin").touch()
        (root / "data/layouts/center.map.bin").touch()
        layout_file.write_text(json.dumps({"layouts": [{
            "id": "LAYOUT_SEVII_CENTER_2F", "name": "gMapLayout_SeviiCenter2F",
            "game_version": "frlg", "layout_version": "frlg", "width": 1, "height": 1,
            "border_filepath": "data/layouts/center.border.bin", "blockdata_filepath": "data/layouts/center.map.bin",
            "primary_tileset": "gTileset_General", "secondary_tileset": "gTileset_Petalburg",
            "border_width": 2, "border_height": 2,
        }]}))
        manifest = self.write_sevii_manifest(root, [
            {"source_map": "OneIsland_PokemonCenter_2F_Frlg", "map_id": "MAP_SEVII_CENTER_2F",
             "layout": "LAYOUT_SEVII_CENTER_2F", "enabled": True,
             "retained_events": {"object_events": [{
                 "index": 1,
                 "source": source["object_events"][1],
                 "wayfarer_script": "WayfarerSevii_Nurse",
                 "overrides": {"script": "WayfarerSevii_Nurse", "flag": "0"},
             }], "coord_events": [], "bg_events": []}},
            {"source_map": "OneIsland_PokemonCenter_1F_Frlg", "map_id": "MAP_SEVII_1F",
             "layout": "LAYOUT_SEVII_CENTER_1F", "enabled": True},
        ], release_link_enabled=True)

        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        events = (map_dir / "events.inc").read_text()
        self.assertIn("ObjectEvents", events)
        self.assertIn("WayfarerSevii_Nurse", events)
        self.assertRegex(events, r"object_event [^\n]*, 0\n")
        self.assertNotIn("CoordEvents", events)
        self.assertNotIn("BGEvents", events)
        self.assertIn("MAP_SEVII_1F", events)
        self.assertNotIn("MAP_UNION_ROOM_FRLG", events)
        self.assertNotIn("MAP_TRADE_CENTER_FRLG", events)
        self.assertNotIn("SevenIsland_SevaultCanyon_House_EventScript_ItemLuckyPunch", events)
        self.assertNotIn("SixIsland_WaterPath_House2_EventScript_Man", events)
        self.assertNotIn("SevenIsland_SevaultCanyon_House_EventScript_ChanseyDanceMan", events)
        self.assertNotIn("SevenIsland_SevaultCanyon_House_EventScript_Chansey", events)
        self.assertNotIn("SevenIsland_SevaultCanyon_House_EventScript_ItemLuckyPunch", events)
        connections = (map_dir / "connections.inc").read_text()
        self.assertIn("MAP_SEVII_1F", connections)

    def test_wayfarer_sanitizes_only_registered_sevii_map_events(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)

        def map_source(name, map_id, game_version, script, flag):
            return {
                "id": map_id,
                "name": name,
                "game_version": game_version,
                "layout": f"LAYOUT_{map_id[4:]}",
                "music": "MUS_NONE",
                "region_map_section": "MAPSEC_NONE",
                "requires_flash": False,
                "weather": "WEATHER_NONE",
                "map_type": "MAP_TYPE_TOWN",
                "allow_cycling": True,
                "allow_escaping": False,
                "allow_running": True,
                "show_map_name": True,
                "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
                "object_events": [{
                    "type": "object", "graphics_id": "OBJ_EVENT_GFX_NURSE", "x": 1, "y": 1,
                    "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                    "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NONE",
                    "trainer_sight_or_berry_tree_id": "0", "script": script, "flag": "0",
                }],
                "warp_events": [],
                "coord_events": [],
                "bg_events": [{
                    "type": "hidden_item", "x": 2, "y": 2, "elevation": 0,
                    "item": "ITEM_POTION", "flag": flag,
                }],
                "connections": [],
            }

        layouts = []
        map_files = {}
        for name, map_id, source, script, flag in (
            ("OneIsland_Frlg", "MAP_SEVII", "frlg", "FrlgStoryEvent", "FLAG_SEVII_ITEM"),
            ("SafariZone_Southeast", "MAP_SAFARI", "emerald", "SafariLegacyEvent", "FLAG_SAFARI_ITEM"),
            ("NavelRock_Top", "MAP_NAVEL", "emerald", "NavelLegacyEvent", "FLAG_NAVEL_ITEM"),
        ):
            map_dir = root / "data/maps" / name
            map_dir.mkdir()
            source_data = map_source(name, map_id, source, script, flag)
            map_file = map_dir / "map.json"
            map_file.write_text(json.dumps(source_data))
            map_files[name] = map_file
            border = root / f"data/layouts/{name}.border.bin"
            blockdata = root / f"data/layouts/{name}.map.bin"
            border.touch()
            blockdata.write_bytes(b"\0\0")
            layouts.append({
                "id": source_data["layout"], "name": f"gMapLayout_{name}",
                "game_version": source, "layout_version": source, "width": 1, "height": 1,
                "border_filepath": str(border.relative_to(root)),
                "blockdata_filepath": str(blockdata.relative_to(root)),
                "primary_tileset": "gTileset_General", "secondary_tileset": "gTileset_Petalburg",
                "border_width": 2, "border_height": 2,
            })
        layouts_file = root / "data/layouts/layouts.json"
        layouts_file.write_text(json.dumps({"layouts": layouts}))
        (root / "include/constants/map_groups.h").write_text(
            "enum { MAP_SEVII = (0 | (0 << 8)), MAP_SAFARI = (1 | (0 << 8)), "
            "MAP_NAVEL = (2 | (0 << 8)), };\n"
        )
        manifest = self.write_sevii_manifest(root, [{
            "source_map": "OneIsland_Frlg", "map_id": "MAP_SEVII", "layout": "LAYOUT_SEVII",
            "enabled": True, "retained_events": {"object_events": [], "coord_events": [], "bg_events": []},
        }], release_link_enabled=True)

        result = self.run_map(root, "wayfarer", map_files["OneIsland_Frlg"], layouts_file, manifest)
        self.assertEqual(result.returncode, 0, result.stderr)
        sevii_events = map_files["OneIsland_Frlg"].parent.joinpath("events.inc").read_text()
        self.assertNotIn("FrlgStoryEvent", sevii_events)
        self.assertNotIn("FLAG_SEVII_ITEM", sevii_events)

        for legacy_name, legacy_script, legacy_flag in (
            ("SafariZone_Southeast", "SafariLegacyEvent", "FLAG_SAFARI_ITEM"),
            ("NavelRock_Top", "NavelLegacyEvent", "FLAG_NAVEL_ITEM"),
        ):
            result = self.run_map(root, "wayfarer", map_files[legacy_name], layouts_file, manifest)
            self.assertEqual(result.returncode, 0, result.stderr)
            wayfarer_events = map_files[legacy_name].parent.joinpath("events.inc").read_text()
            result = self.run_map(root, "emerald", map_files[legacy_name], layouts_file)
            self.assertEqual(result.returncode, 0, result.stderr)
            emerald_events = map_files[legacy_name].parent.joinpath("events.inc").read_text()
            self.assertEqual(wayfarer_events, emerald_events)
            self.assertIn(legacy_script, wayfarer_events)
            self.assertIn(legacy_flag, wayfarer_events)
            self.assertIn("bg_hidden_item_event ", wayfarer_events)
            self.assertNotIn("bg_hidden_item_event_hoenn ", wayfarer_events)

    def test_wayfarer_sevii_rejects_retained_trainer_identity(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        map_dir = root / "data/maps/OneIsland_Frlg"
        map_dir.mkdir()
        trainer = {"type": "object", "graphics_id": "OBJ_EVENT_GFX_TRAINER", "x": 1, "y": 1,
                   "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 0,
                   "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NORMAL",
                   "trainer_sight_or_berry_tree_id": "1", "script": "Frlg_Trainer", "flag": "0"}
        source = {"id": "MAP_SEVII", "name": "OneIsland_Frlg", "game_version": "frlg",
                  "layout": "LAYOUT_SEVII", "music": "MUS_NONE", "region_map_section": "MAPSEC_NONE",
                  "requires_flash": False, "weather": "WEATHER_NONE", "map_type": "MAP_TYPE_TOWN",
                  "allow_cycling": True, "allow_escaping": False, "allow_running": True,
                  "show_map_name": True, "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
                  "object_events": [trainer], "warp_events": [], "coord_events": [], "bg_events": [],
                  "connections": []}
        map_file = map_dir / "map.json"
        map_file.write_text(json.dumps(source))
        (root / "data/layouts/sevii.border.bin").touch()
        (root / "data/layouts/sevii.map.bin").touch()
        layout_file = root / "data/layouts/layouts.json"
        layout_file.write_text(json.dumps({"layouts": [{
            "id": "LAYOUT_SEVII", "name": "gMapLayout_Sevii", "game_version": "frlg", "layout_version": "frlg",
            "width": 1, "height": 1, "border_filepath": "data/layouts/sevii.border.bin",
            "blockdata_filepath": "data/layouts/sevii.map.bin", "primary_tileset": "gTileset_General",
            "secondary_tileset": "gTileset_Petalburg", "border_width": 2, "border_height": 2,
        }]}))
        manifest = self.write_sevii_manifest(root, [{
            "source_map": "OneIsland_Frlg", "map_id": "MAP_SEVII", "layout": "LAYOUT_SEVII", "enabled": True,
            "retained_events": {"object_events": [{"index": 0, "source": trainer,
                                                       "wayfarer_script": "WayfarerSevii_Test"}]},
        }], release_link_enabled=True)

        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("is a Trainer and cannot be retained", result.stderr)

    def test_wayfarer_sevii_accepts_tower_transient_event_state_only(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        map_dir = root / "data/maps/TrainerTower_1F_Frlg"
        map_dir.mkdir()
        tower_actor = {
            "type": "object", "graphics_id": "OBJ_EVENT_GFX_TRAINER_TOWER_DUDE",
            "x": 1, "y": 1, "elevation": 0, "movement_type": "MOVEMENT_TYPE_FACE_DOWN",
            "movement_range_x": 0, "movement_range_y": 0, "trainer_type": "TRAINER_TYPE_NONE",
            "trainer_sight_or_berry_tree_id": "0", "script": "TowerActor", "flag": "FLAG_TEMP_6",
        }
        source = {
            "id": "MAP_TOWER", "name": "TrainerTower_1F_Frlg", "game_version": "frlg",
            "layout": "LAYOUT_TOWER", "music": "MUS_NONE", "region_map_section": "MAPSEC_NONE",
            "requires_flash": False, "weather": "WEATHER_NONE", "map_type": "MAP_TYPE_INDOOR",
            "allow_cycling": False, "allow_escaping": False, "allow_running": True,
            "show_map_name": False, "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
            "object_events": [tower_actor], "warp_events": [], "coord_events": [], "bg_events": [],
            "connections": [],
        }
        map_file = map_dir / "map.json"
        map_file.write_text(json.dumps(source))
        (root / "data/layouts/tower.border.bin").touch()
        (root / "data/layouts/tower.map.bin").touch()
        layout_file = root / "data/layouts/layouts.json"
        layout_file.write_text(json.dumps({"layouts": [{
            "id": "LAYOUT_TOWER", "name": "gMapLayout_Tower", "game_version": "frlg",
            "layout_version": "frlg", "width": 1, "height": 1,
            "border_filepath": "data/layouts/tower.border.bin",
            "blockdata_filepath": "data/layouts/tower.map.bin",
            "primary_tileset": "gTileset_General", "secondary_tileset": "gTileset_Petalburg",
            "border_width": 2, "border_height": 2,
        }]}))
        (root / "include/constants/map_groups.h").write_text(
            "enum { MAP_TOWER = (0 | (0 << 8)), };\n"
        )
        content_id = "trainer-tower.test.actor"
        row = {
            "index": 0, "source": tower_actor, "owner": "trainer_tower", "content_id": content_id,
            "reason": "Fixture preserves a transient Tower actor.",
            "wayfarer_script": "WayfarerSevii_TowerActor",
        }
        manifest = self.write_sevii_manifest(root, [{
            "source_map": "TrainerTower_1F_Frlg", "map_id": "MAP_TOWER", "layout": "LAYOUT_TOWER",
            "enabled": True, "retained_events": {"object_events": [row]},
        }], release_link_enabled=True)
        manifest_data = json.loads(manifest.read_text())
        manifest_data["content_domains"]["exploration"]["inventory"] = []
        manifest_data["content_domains"]["trainer_tower"] = {
            "owner": "trainer_tower", "enabled": True, "inventory": [content_id],
        }
        manifest.write_text(json.dumps(manifest_data))

        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertEqual(result.returncode, 0, result.stderr)

        manifest_data["content_domains"]["trainer_tower"] = {
            "owner": "trainer_tower", "enabled": False, "inventory": [],
        }
        manifest_data["content_domains"]["story"] = {
            "owner": "story", "enabled": True, "inventory": [content_id],
        }
        manifest_data["maps"][0]["retained_events"]["object_events"][0]["owner"] = "story"
        manifest.write_text(json.dumps(manifest_data))
        result = self.run_map(root, "wayfarer", map_file, layout_file, manifest)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("retains raw FRLG persistent state", result.stderr)

    def test_wayfarer_sevii_manifest_rejects_event_island_sources(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(root, "HnsMap", "MAP_HNS", "hns")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns"],
                    "gHns": ["HnsMap"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps({"heal_locations": []})
        )
        manifest = self.write_sevii_manifest(
            root,
            [
                {
                    "source_map": "BirthIsland_Harbor_Frlg",
                    "map_id": "MAP_BIRTH",
                    "layout": "LAYOUT_BIRTH",
                }
            ],
        )

        result = self.run_groups(root, "wayfarer", [hns], manifest)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("cannot select event-island map", result.stderr)

    def test_wayfarer_validates_selected_frlg_heal_locations(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        hns = self.add_map(root, "HnsMap", "MAP_HNS", "hns")
        frlg = self.add_map(root, "OneIsland_PokemonCenter_1F", "MAP_SEVII", "frlg")
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gHns", "gFrlg"],
                    "gHns": ["HnsMap"],
                    "gFrlg": ["OneIsland_PokemonCenter_1F"],
                    "connections_include_order": [],
                }
            )
        )
        (root / "src/data/heal_locations.json").write_text(
            json.dumps(
                {
                    "heal_locations": [
                        {
                            "id": "HEAL_SEVII",
                            "source": "FRLG",
                            "map": "MAP_SEVII",
                            "respawn_map": "MAP_MISSING",
                            "respawn_npc": "LOCALID_NURSE",
                        }
                    ]
                }
            )
        )
        manifest = self.write_sevii_manifest(
            root,
            [
                {
                    "source_map": "OneIsland_PokemonCenter_1F",
                    "map_id": "MAP_SEVII",
                    "layout": "LAYOUT_SEVII",
                    "enabled": True,
                }
            ],
            release_link_enabled=True,
        )

        result = self.run_groups(root, "wayfarer", [hns, frlg], manifest)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("references unavailable respawn map MAP_MISSING", result.stderr)

    def test_wayfarer_layouts_include_both_sources_and_emit_map_flags(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        layouts = []
        for source, suffix in (("emerald", "Emerald"), ("hns", "Hns"), ("frlg", "Frlg")):
            border = root / f"data/layouts/{suffix}.border.bin"
            blockdata = root / f"data/layouts/{suffix}.map.bin"
            border.touch()
            blockdata.write_bytes(b"\0\0")
            layouts.append(
                {
                    "id": f"LAYOUT_{suffix.upper()}",
                    "name": f"gMapLayout_{suffix}",
                    "game_version": source,
                    "layout_version": source,
                    "width": 1,
                    "height": 1,
                    "border_filepath": str(border.relative_to(root)),
                    "blockdata_filepath": str(blockdata.relative_to(root)),
                    "primary_tileset": "gTileset_General",
                    "secondary_tileset": "gTileset_Petalburg",
                    "border_width": 2,
                    "border_height": 2,
                }
            )
        layouts_data = {
            "layouts_table_label": "gMapLayouts",
            "layouts": layouts,
        }
        layouts_path = root / "data/layouts/layouts.json"
        layouts_path.write_text(json.dumps(layouts_data))

        result = subprocess.run(
            [
                str(self.mapjson),
                "layouts",
                "wayfarer",
                "data/layouts/layouts.json",
                "data/layouts",
                "include/constants",
            ],
            cwd=root,
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        headers = (root / "data/layouts/layouts.inc").read_text()
        table = (root / "data/layouts/layouts_table.inc").read_text()
        self.assertIn("gMapLayout_Emerald::", headers)
        self.assertIn("gMapLayout_Hns::", headers)
        self.assertNotIn("gMapLayout_Frlg::", headers)
        self.assertIn("\t.4byte gMapLayout_Emerald", table)
        self.assertIn("\t.4byte gMapLayout_Hns", table)
        production_table = table.split("\t.if MAP_LAYOUT_TESTING_ASM", 1)[0]
        self.assertTrue(production_table.rstrip().endswith("\t.4byte NULL"))

        map_dir = root / "data/maps/HnsMap"
        map_dir.mkdir()
        map_data = {
            "id": "MAP_HNS",
            "name": "HnsMap",
            "game_version": "hns",
            "layout": "LAYOUT_HNS",
            "music": "MUS_NONE",
            "region_map_section": "MAPSEC_NONE",
            "requires_flash": False,
            "weather": "WEATHER_NONE",
            "map_type": "MAP_TYPE_TOWN",
            "allow_cycling": True,
            "allow_escaping": False,
            "allow_running": True,
            "show_map_name": True,
            "battle_scene": "MAP_BATTLE_SCENE_NORMAL",
            "object_events": [],
            "warp_events": [],
            "coord_events": [],
            "bg_events": [],
            "connections": [],
        }
        (map_dir / "map.json").write_text(json.dumps(map_data))
        (root / "include/constants/map_groups.h").write_text(
            "enum { MAP_HNS = (0 | (0 << 8)), };\n"
        )
        result = subprocess.run(
            [
                str(self.mapjson),
                "map",
                "wayfarer",
                "data/maps/HnsMap/map.json",
                "data/layouts/layouts.json",
                "data/maps/HnsMap",
            ],
            cwd=root,
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("\tmap_header_flags ", (map_dir / "header.inc").read_text())

        first_mtimes = {
            path.name: path.stat().st_mtime_ns
            for path in map_dir.glob("*.inc")
        }
        result = subprocess.run(
            [
                str(self.mapjson),
                "map",
                "wayfarer",
                "data/maps/HnsMap/map.json",
                "data/layouts/layouts.json",
                "data/maps/HnsMap",
            ],
            cwd=root,
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(
            first_mtimes,
            {path.name: path.stat().st_mtime_ns for path in map_dir.glob("*.inc")},
        )

    def test_catalog_rejects_signed_byte_group_and_map_overflow(self):
        fixture, root = self.make_fixture()
        self.addCleanup(fixture.cleanup)
        only_map = self.add_map(root, "OnlyMap", "MAP_ONLY", "emerald")
        group_names = [f"g{i}" for i in range(129)]
        groups = {
            "group_order": group_names,
            "connections_include_order": [],
            **{name: [] for name in group_names},
        }
        (root / "data/maps/map_groups.json").write_text(json.dumps(groups))

        result = self.run_groups(root, "emerald", [only_map])

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Map group 128 exceeds the signed-byte warp limit", result.stderr)

        map_names = []
        map_files = []
        for i in range(129):
            name = f"Map{i}"
            map_names.append(name)
            map_files.append(self.add_map(root, name, f"MAP_{i}", "emerald"))
        (root / "data/maps/map_groups.json").write_text(
            json.dumps(
                {
                    "group_order": ["gMaps"],
                    "gMaps": map_names,
                    "connections_include_order": [],
                }
            )
        )

        result = self.run_groups(root, "emerald", map_files)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Map 128 in group gMaps exceeds the signed-byte warp limit", result.stderr)

    def test_repository_wayfarer_union_resolves_and_preserves_hns_positions(self):
        maps_root = GAME_ROOT / "data/maps"
        groups = json.loads((maps_root / "map_groups.json").read_text())
        expected_hns_groups = [
            "gMapGroup_TownsAndRoutes_Hns",
            "gMapGroup_IndoorNewBark_Hns",
            "gMapGroup_IndoorCherrygrove_Hns",
            "gMapGroup_IndoorViolet_Hns",
            "gMapGroup_IndoorAzalea_Hns",
            "gMapGroup_IndoorGoldenrod_Hns",
            "gMapGroup_IndoorEcruteak_Hns",
            "gMapGroup_IndoorOlivine_Hns",
            "gMapGroup_IndoorCianwood_Hns",
            "gMapGroup_IndoorMahogany_Hns",
            "gMapGroup_IndoorBlackthorn_Hns",
            "gMapGroup_IndoorPallet_Hns",
            "gMapGroup_IndoorViridian_Hns",
            "gMapGroup_IndoorPewter_Hns",
            "gMapGroup_IndoorCerulean_Hns",
            "gMapGroup_IndoorVermilion_Hns",
            "gMapGroup_IndoorLavender_Hns",
            "gMapGroup_IndoorCeladon_Hns",
            "gMapGroup_IndoorSaffron_Hns",
            "gMapGroup_IndoorFuchsia_Hns",
            "gMapGroup_IndoorCinnabar_Hns",
            "gMapGroup_IndoorIndigo_Hns",
            "gMapGroup_IndoorJohtoRoutes_Hns",
            "gMapGroup_IndoorKantoRoutes_Hns",
            "gMapGroup_Dungeons_Hns",
            "gMapGrouop_OutdoorAlola_Hns",
            "gMapGroup_IndoorAlola_Hns",
            "gMapGroup_IndoorDynamic_Hns",
            "gMapGroup_Sinjoh_Hns",
            "gMapGroup_IndoorSinjoh_Hns",
            "gMapGroup_SpecialArea_Hns",
        ]
        self.assertEqual(groups["group_order"][:31], expected_hns_groups)
        self.assertEqual(groups["group_order"][31], "gMapGroup_TownsAndRoutes")
        data = {
            path.parent.name: json.loads(path.read_text())
            for path in maps_root.glob("*/map.json")
        }
        included = []
        for group_num, group_name in enumerate(groups["group_order"]):
            self.assertLessEqual(group_num, 127)
            for map_num, map_name in enumerate(groups[group_name]):
                self.assertLessEqual(map_num, 127)
                map_data = data[map_name]
                if (map_data.get("game_version", "emerald") in {"hns", "emerald"}
                        and map_name not in RETIRED_HNS_MAP_NAMES):
                    included.append(map_data)

        included_ids = {map_data["id"] for map_data in included}
        self.assertEqual(len(included_ids), len(included))
        self.assertTrue(any(item.get("game_version") == "hns" for item in included))
        self.assertTrue(any(item.get("game_version", "emerald") == "emerald" for item in included))
        for map_data in included:
            for warp_index, warp in enumerate(map_data.get("warp_events", [])):
                destination = (
                    "MAP_SEAFOAM_ISLANDS_1F_COAST_POC"
                    if map_data["name"] == "Route20_hns" and warp_index < 2
                    else warp["dest_map"]
                )
                self.assertIn(
                    destination,
                    included_ids | {"MAP_DYNAMIC", "MAP_UNDEFINED"},
                )
            for connection in map_data.get("connections") or []:
                self.assertIn(connection["map"], included_ids)

        heal_locations = json.loads(
            (GAME_ROOT / "src/data/heal_locations.json").read_text()
        )["heal_locations"]
        self.assertEqual(
            [item["source"] for item in heal_locations],
            ["EMERALD"] * 22 + ["FRLG"] * 20 + ["HNS"] * 32,
        )
        for item in heal_locations:
            if item["source"] in {"EMERALD", "HNS"}:
                self.assertIn(item["map"], included_ids)
                if "respawn_map" in item:
                    self.assertIn(item["respawn_map"], included_ids)

    def test_repository_ss_anne_is_exact_closed_interior_selection(self):
        expected_maps = {
            "SSAnne_1F_Corridor_Frlg", "SSAnne_1F_Room1_Frlg", "SSAnne_1F_Room2_Frlg",
            "SSAnne_1F_Room3_Frlg", "SSAnne_1F_Room4_Frlg", "SSAnne_1F_Room5_Frlg",
            "SSAnne_1F_Room6_Frlg", "SSAnne_1F_Room7_Frlg", "SSAnne_2F_Corridor_Frlg",
            "SSAnne_2F_Room1_Frlg", "SSAnne_2F_Room2_Frlg", "SSAnne_2F_Room3_Frlg",
            "SSAnne_2F_Room4_Frlg", "SSAnne_2F_Room5_Frlg", "SSAnne_2F_Room6_Frlg",
            "SSAnne_3F_Corridor_Frlg", "SSAnne_B1F_Corridor_Frlg", "SSAnne_B1F_Room1_Frlg",
            "SSAnne_B1F_Room2_Frlg", "SSAnne_B1F_Room3_Frlg", "SSAnne_B1F_Room4_Frlg",
            "SSAnne_B1F_Room5_Frlg", "SSAnne_CaptainsOffice_Frlg", "SSAnne_Deck_Frlg",
            "SSAnne_Kitchen_Frlg",
        }
        expected_layouts = {
            "LAYOUT_SSANNE_1F_CORRIDOR", "LAYOUT_SSANNE_2F_CORRIDOR", "LAYOUT_SSANNE_3F_CORRIDOR",
            "LAYOUT_SSANNE_B1F_CORRIDOR", "LAYOUT_SSANNE_CAPTAINS_OFFICE", "LAYOUT_SSANNE_DECK",
            "LAYOUT_SSANNE_KITCHEN", "LAYOUT_SSANNE_ROOM1", "LAYOUT_SSANNE_ROOM2",
        }
        source = (GAME_ROOT / "tools/mapjson/mapjson.cpp").read_text()

        def selected_set(name):
            match = re.search(rf"const set<string> {name} = \{{(.*?)\}};", source, re.DOTALL)
            self.assertIsNotNone(match)
            return set(re.findall(r'"([^"]+)"', match.group(1)))

        self.assertEqual(selected_set("wayfarer_anne_map_names"), expected_maps)
        self.assertEqual(selected_set("wayfarer_anne_layout_ids"), expected_layouts)
        self.assertNotIn("SSAnne_Exterior_Frlg", expected_maps)

        map_data = {
            name: json.loads((GAME_ROOT / "data/maps" / name / "map.json").read_text())
            for name in expected_maps
        }
        self.assertEqual({entry["layout"] for entry in map_data.values()}, expected_layouts)
        self.assertTrue(all(entry["game_version"] == "frlg" for entry in map_data.values()))
        exterior_warps = [
            (name, index) for name, entry in map_data.items()
            for index, warp in enumerate(entry.get("warp_events", []))
            if warp["dest_map"] == "MAP_SSANNE_EXTERIOR"
        ]
        self.assertEqual(exterior_warps, [("SSAnne_1F_Corridor_Frlg", 2), ("SSAnne_1F_Corridor_Frlg", 3)])
        self.assertIn('resolved["dest_map"] = "MAP_DYNAMIC";', source)
        self.assertIn("setdynamicwarp MAP_VERMILION_CITY_PORT_INSIDE_HNS, 8, 9", (GAME_ROOT / "data/maps/SSAnne_1F_Corridor_Frlg/scripts.inc").read_text())
        map_names_by_id = {entry["id"]: name for name, entry in map_data.items()}
        graph = {name: set() for name in expected_maps}
        for name, entry in map_data.items():
            for warp in entry.get("warp_events", []):
                destination = warp["dest_map"]
                if destination == "MAP_SSANNE_EXTERIOR":
                    self.assertEqual(name, "SSAnne_1F_Corridor_Frlg")
                    continue
                self.assertIn(destination, map_names_by_id, f"{name} leaves the Anne interior closure")
                graph[name].add(map_names_by_id[destination])
        exit_map = "SSAnne_1F_Corridor_Frlg"
        for source_map in expected_maps:
            frontier, visited = [source_map], set()
            while frontier:
                current = frontier.pop()
                if current in visited:
                    continue
                visited.add(current)
                frontier.extend(graph[current] - visited)
            self.assertIn(exit_map, visited, f"{source_map} cannot reach the two dock exits")

        expected_item_remaps = {
            "FLAG_HIDE_SSANNE_1F_ROOM2_TM31": "FLAG_WAYFARER_SS_ANNE_ITEM_TM31",
            "FLAG_HIDE_SSANNE_2F_ROOM2_STARDUST": "FLAG_WAYFARER_SS_ANNE_ITEM_STARDUST",
            "FLAG_HIDE_SSANNE_2F_ROOM4_X_ATTACK": "FLAG_WAYFARER_SS_ANNE_ITEM_X_ATTACK",
            "FLAG_HIDE_SSANNE_B1F_ROOM2_TM44": "FLAG_WAYFARER_SS_ANNE_ITEM_TM44",
            "FLAG_HIDE_SSANNE_B1F_ROOM3_ETHER": "FLAG_WAYFARER_SS_ANNE_ITEM_ETHER",
            "FLAG_HIDE_SSANNE_B1F_ROOM5_SUPER_POTION": "FLAG_WAYFARER_SS_ANNE_ITEM_SUPER_POTION",
            "FLAG_HIDE_SSANNE_KITCHEN_GREAT_BALL": "FLAG_WAYFARER_SS_ANNE_ITEM_GREAT_BALL",
            "FLAG_HIDDEN_ITEM_SSANNE_B1F_CORRIDOR_HYPER_POTION": "FLAG_WAYFARER_SS_ANNE_ITEM_HYPER_POTION",
            "FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_CHESTO_BERRY": "FLAG_WAYFARER_SS_ANNE_ITEM_CHESTO_BERRY",
            "FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_PECHA_BERRY": "FLAG_WAYFARER_SS_ANNE_ITEM_PECHA_BERRY",
            "FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_CHERI_BERRY": "FLAG_WAYFARER_SS_ANNE_ITEM_CHERI_BERRY",
        }
        for source_flag, target_flag in expected_item_remaps.items():
            self.assertIn(f'{{"{source_flag}", "{target_flag}"}}', source)
        self.assertIn(
            '{"FLAG_HIDE_SS_ANNE_RIVAL", "FLAG_WAYFARER_SS_ANNE_HIDE_BLUE"}',
            source,
        )
        self.assertIn('coord["var"] = "VAR_WAYFARER_SS_ANNE_BLUE_SCENE"', source)
        persistence = (GAME_ROOT / "src/wayfarer_persistence.c").read_text()
        self.assertIn("FlagSet(FLAG_WAYFARER_SS_ANNE_HIDE_BLUE);", persistence)
        visitor_script = (GAME_ROOT / "data/maps/SSAnne_2F_Corridor_Frlg/scripts.inc").read_text()
        self.assertIn("clearflag FLAG_WAYFARER_SS_ANNE_HIDE_BLUE", visitor_script)
        self.assertIn("addobject LOCALID_SS_ANNE_RIVAL", visitor_script)
        self.assertIn("setflag FLAG_WAYFARER_SS_ANNE_HIDE_BLUE", visitor_script)
        self.assertIn("METATILE_SSAnne_Door", (GAME_ROOT / "src/field_door.c").read_text())
        graphics_pointers = (GAME_ROOT / "src/data/object_events/object_event_graphics_info_pointers.h").read_text()
        sevii_graphics = graphics_pointers[
            graphics_pointers.index("#if HAS_SEVII_CONTENT"):
            graphics_pointers.index("#endif // HAS_SEVII_CONTENT")
        ]
        self.assertIn(
            "[OBJ_EVENT_GFX_CAPTAIN]                  = &gObjectEventGraphicsInfo_Captain,",
            sevii_graphics,
        )
        regions_source = (GAME_ROOT / "include/regions.h").read_text()
        self.assertIn(
            "#if IS_WAYFARER\n    if (sectionId == MAPSEC_S_S_ANNE)\n"
            "        return REGION_KANTO;\n#endif",
            regions_source,
        )
        bill_map = json.loads((GAME_ROOT / "data/maps/Route25_BillsHouse_hns/map.json").read_text())
        self.assertEqual([event["local_id"] for event in bill_map["object_events"]], [
            "LOCALID_BILLS_GRANDPA", "LOCALID_BILLS_GRANDPA_NEW_MON",
            "LOCALID_BILLS_GRANDPA_OLD_MON", "LOCALID_BILLS_GRANDPA_PLAYER",
        ])
        self.assertIn('if (name == "Route25_BillsHouse_hns")', source)
        self.assertIn('{"local_id", 5}', source)
        self.assertIn('"Route25_BillsHouse_EventScript_Bill"', source)
        self.assertIn('"OBJ_EVENT_GFX_BILL_HNS"', source)
        flag_source = (GAME_ROOT / "include/constants/flags_hns.h").read_text()
        self.assertIn("#define FLAG_WAYFARER_BILL_RESCUED                              0x4B3", flag_source)
        self.assertIn("#define FLAG_WAYFARER_BILL_SS_TICKET_SETTLED                    0x4B4", flag_source)
        region_map_source = (GAME_ROOT / "src/region_map.c").read_text()
        jk_entries = region_map_source[
            region_map_source.index("static const struct RegionMapLocation sRegionMapEntries_JK[]"):
            region_map_source.index("const struct RegionMapLocation *GetActiveRegionMapEntries")
        ]
        self.assertIn(
            '[MAPSEC_S_S_ANNE]          = { 24, 7,  1, 1, COMPOUND_STRING("S.S. ANNE") }',
            jk_entries,
        )
        heal_locations = region_map_source[
            region_map_source.index("static const u8 sMapHealLocations[][3]"):
            region_map_source.index("static const struct FlyLocation sFlyLocations[]")
        ]
        anne_fallback = re.search(
            r"\[MAPSEC_S_S_ANNE\]\s*=\s*\{([^}]*)\}", heal_locations
        )
        self.assertIsNotNone(anne_fallback)
        self.assertIn("MAP_PALLET_TOWN", anne_fallback.group(1))
        self.assertNotIn("MAP_SSANNE", anne_fallback.group(1))
        fly_locations = region_map_source[
            region_map_source.index("static const struct FlyLocation sFlyLocations[]"):
            region_map_source.index("// Sprite data for SpriteCB_FlyDestIcon")
        ]
        self.assertNotIn("MAPSEC_S_S_ANNE", fly_locations)
        section_rows = json.loads(
            (GAME_ROOT / "src/data/region_map/region_map_sections.json").read_text()
        )["map_sections"]
        anne_section = next(row for row in section_rows if row["id"] == "MAPSEC_S_S_ANNE")
        self.assertTrue(anne_section["wayfarer_hns"])
        for template_name in (
            "region_map_sections.constants.json.txt",
            "region_map_sections.json.txt",
        ):
            template = (GAME_ROOT / "src/data/region_map" / template_name).read_text()
            self.assertIn('existsIn(map_section, "wayfarer_hns")', template)

        room6 = (GAME_ROOT / "data/maps/SSAnne_1F_Room6_Frlg/scripts.inc").read_text()
        wayfarer_room6 = room6.split("#if IS_WAYFARER", 1)[1].split("#else", 1)[0]
        self.assertIn("MSGBOX_NPC", wayfarer_room6)
        self.assertNotIn("PartyHeal", wayfarer_room6)


if __name__ == "__main__":
    unittest.main()
