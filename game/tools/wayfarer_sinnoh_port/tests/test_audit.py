import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
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
ASSETS = load_module("wayfarer_sinnoh_assets", TOOLS / "assets.py")
FREEZE = load_module("wayfarer_sinnoh_freeze", TOOLS / "freeze.py")


class SinnohFoundationAuditTests(unittest.TestCase):
    def setUp(self):
        self.maps_path = GAME / "src/data/wayfarer_sinnoh_maps.json"
        self.assets_path = GAME / "src/data/wayfarer_sinnoh_assets.json"

    def test_checked_in_authorities_pass_without_a_donor_checkout(self):
        report = AUDIT.build_report(GAME, self.maps_path, self.assets_path)
        integrity = report["manifest_integrity"]
        self.assertEqual(integrity["selected_map_count"], 133)
        self.assertEqual(integrity["selected_layout_count"], 133)
        self.assertEqual(integrity["frozen_manifest_counts"]["warps"], 233)
        self.assertEqual(integrity["frozen_manifest_counts"]["connections"], 114)
        self.assertFalse(report["donor_source_verification"]["performed"])
        self.assertIsNone(report["donor_source_verification"]["observed_empty_content_counts"])
        self.assertEqual(report["review_required"], [])
        self.assertEqual(integrity["porymap"], {
            "base_game_version": {"value": "pokeemerald", "verified": True},
            "frozen_manifest_layout_format": {"value": "emerald", "verified": True, "map_count": 133},
            "imported_layout_verification": {"verified": False, "layout_count": 0},
        })

    def test_audit_report_is_deterministic_without_a_donor_checkout(self):
        first = AUDIT.build_report(GAME, self.maps_path, self.assets_path)
        second = AUDIT.build_report(GAME, self.maps_path, self.assets_path)
        self.assertEqual(json.dumps(first, sort_keys=True), json.dumps(second, sort_keys=True))

    def test_rejects_one_byte_exact_alias_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            candidate = root / "data/layouts/example/map.bin"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"\x01\x00")
            layouts = root / "data/layouts/layouts.json"
            layouts.write_text(json.dumps({"layouts": [{"id": "LAYOUT_EXAMPLE", "width": 1, "height": 1,
                                                       "blockdata_filepath": "data/layouts/example/map.bin"}]}))
            shape = {"width": 1, "height": 1, "element_bytes": 2, "byte_length": 2}
            record = {"record_id": "blockdata.example", "asset_family": "blockdata", "reuse_class": "EXACT_ALIAS",
                      "source_sha256": hashlib.sha256(b"\x01\x00").hexdigest(), "source_bytes": 2,
                      "shape": shape, "canonical_owner": "LAYOUT_EXAMPLE",
                      "candidate_matches": [{"layout": "LAYOUT_EXAMPLE", "path": "data/layouts/example/map.bin", "runtime_meaning_proven": True,
                                             "source_shape": shape, "candidate_shape": shape}]}
            AUDIT.validate_exact_worktree_assets(root, {record["record_id"]: record})
            candidate.write_bytes(b"\x02\x00")
            with self.assertRaisesRegex(AUDIT.FoundationError, "drifted"):
                AUDIT.validate_exact_worktree_assets(root, {record["record_id"]: record})

    def test_rejects_exact_alias_candidate_layout_shape_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            candidate = root / "data/layouts/example/map.bin"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"\x01\x00")
            shape = {"width": 1, "height": 1, "element_bytes": 2, "byte_length": 2}
            (root / "data/layouts/layouts.json").write_text(json.dumps({"layouts": [{"id": "LAYOUT_EXAMPLE", "width": 2, "height": 1,
                                                                            "blockdata_filepath": "data/layouts/example/map.bin"}]}))
            record = {"record_id": "blockdata.example", "asset_family": "blockdata", "reuse_class": "EXACT_ALIAS",
                      "source_sha256": hashlib.sha256(b"\x01\x00").hexdigest(), "source_bytes": 2, "shape": shape,
                      "canonical_owner": "LAYOUT_EXAMPLE", "candidate_matches": [{"layout": "LAYOUT_EXAMPLE", "path": "data/layouts/example/map.bin",
                      "runtime_meaning_proven": True, "source_shape": shape, "candidate_shape": shape}]}
            with self.assertRaisesRegex(AUDIT.FoundationError, "layout shape drifted"):
                AUDIT.validate_exact_worktree_assets(root, {record["record_id"]: record})

    def test_rejects_porymap_base_game_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "porymap.project.json").write_text(json.dumps({"base_game_version": "pokefirered"}))
            with self.assertRaisesRegex(AUDIT.FoundationError, "base_game_version pokeemerald"):
                AUDIT.validate_porymap_contract(root, [{"layout_format": "emerald"}])

    def test_rejects_frozen_manifest_layout_format_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "porymap.project.json").write_text(json.dumps({"base_game_version": "pokeemerald"}))
            with self.assertRaisesRegex(AUDIT.FoundationError, "layout_format emerald"):
                AUDIT.validate_porymap_contract(root, [{"layout_format": "frlg"}])

    def test_rejects_imported_target_layout_without_verification(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "porymap.project.json").write_text(json.dumps({"base_game_version": "pokeemerald"}))
            layouts = root / "data/layouts/layouts.json"
            layouts.parent.mkdir(parents=True)
            layouts.write_text(json.dumps({"layouts": [{"id": "LAYOUT_SINNOH_ROUTE201"}]}))
            with self.assertRaisesRegex(AUDIT.FoundationError, "require verification"):
                AUDIT.validate_porymap_contract(root, [{
                    "layout_format": "emerald", "target_layout": "LAYOUT_SINNOH_ROUTE201",
                }])

    def test_donor_empty_content_check_rejects_actual_nonempty_source(self):
        with tempfile.TemporaryDirectory() as directory:
            donor = Path(directory)
            wild = donor / "src/data/wild_encounters.json"
            wild.parent.mkdir(parents=True)
            wild.write_text(json.dumps({"wild_encounter_groups": [{"encounters": [{"map": "MAP_TEST"}]}]}))
            observed = AUDIT.donor_empty_counts(donor, {"Test": {"id": "MAP_TEST", "object_events": [{"x": 1}], "coord_events": [], "bg_events": [], "map_scripts": []}})
            self.assertEqual(observed["object_events"], 1)
            self.assertEqual(observed["wild_encounter_profiles"], 1)
            with self.assertRaisesRegex(AUDIT.FoundationError, "empty-content facts drifted"):
                AUDIT.require_expected_empty_counts(observed)

    def test_rejects_count_and_asset_reference_failures(self):
        maps = json.loads(self.maps_path.read_text())
        assets = json.loads(self.assets_path.read_text())
        maps["maps"] = maps["maps"][:-1]
        with self.assertRaisesRegex(AUDIT.FoundationError, "group count drifted|exactly 133"):
            AUDIT.validate_checked_in(GAME, maps, assets)
        maps = json.loads(self.maps_path.read_text())
        maps["maps"][0]["asset_records"]["blockdata"] = "blockdata.not_declared"
        with self.assertRaisesRegex(AUDIT.FoundationError, "missing or malformed asset reference"):
            AUDIT.validate_checked_in(GAME, maps, assets)

    def test_release_selection_stays_blocked_by_frozen_selection_gate(self):
        with self.assertRaisesRegex(AUDIT.FoundationError, "asset gate"):
            AUDIT.build_report(GAME, self.maps_path, self.assets_path, release_selection=True)

    def test_tileset_implementation_is_technical_and_has_no_attribution_state(self):
        maps = json.loads(self.maps_path.read_text())
        assets = json.loads(self.assets_path.read_text())

        def has_key(value, key):
            if isinstance(value, dict):
                return key in value or any(has_key(item, key) for item in value.values())
            if isinstance(value, list):
                return any(has_key(item, key) for item in value)
            return False

        self.assertFalse(has_key(maps, "attribution"))
        self.assertFalse(has_key(assets, "attribution"))

        records = [row for row in assets["records"] if row["asset_family"] == "tileset_component"]
        self.assertEqual(len(records), 561)
        self.assertEqual(maps["selection"]["blockers"], [])
        self.assertEqual(assets["selection"]["blockers"], [])
        self.assertTrue(all(row["reuse_class"] != "REVIEW_REQUIRED" for row in records))
        self.assertTrue(all(row["selection_blocker"] is False for row in records))
        self.assertTrue(all(row["generated_sha256"] and row["decoded_sha256"] and row["decoded_bytes"] >= 0 for row in records))
        selection_rationales = [
            maps["selection"]["reason"],
            *(row["inclusion"]["reason"] for row in maps["maps"]),
            *(row["rationale"] for row in records),
        ]
        self.assertTrue(all(
            all(word not in rationale.lower() for word in ("legal", "attribution", "license", "permission"))
            for rationale in selection_rationales
        ))
        self.assertFalse(maps["selection"]["release_link_enabled"])
        self.assertFalse(maps["selection"]["asset_manifest_ready"])
        self.assertTrue(all(row["inclusion"]["state"] == "FROZEN_NOT_SELECTED" for row in maps["maps"]))

    def test_closed_asset_gate_excludes_sinnoh_aggregates_from_wayfarer(self):
        global_header = (GAME / "include/constants/global.h").read_text(encoding="utf-8")
        wayfarer = global_header.split("#elif defined(POKEMON_WAYFARER)", 1)[1].split("#elif defined(POKEMON_HNS)", 1)[0]
        self.assertIn("#define HAS_SINNOH_CONTENT 0", wayfarer)

    def test_special_graphics_proofs_use_exact_make_rules_and_fresh_outputs(self):
        manifest = json.loads(self.assets_path.read_text())
        expected = {
            "data/tilesets/primary/building/tiles.4bpp.smol": ("-num_tiles", "502", "-Wnum_tiles"),
            "data/tilesets/secondary/shop/tiles.4bpp.fastSmol": ("-num_tiles", "502", "-Wnum_tiles"),
            "data/tilesets/secondary/pokemon_center/tiles.4bpp.fastSmol": ("-num_tiles", "478", "-Wnum_tiles"),
            "data/tilesets/secondary/cave/tiles.4bpp.fastSmol": ("-num_tiles", "425", "-Wnum_tiles"),
            "data/tilesets/secondary/pokemon_school/tiles.4bpp.fastSmol": ("-num_tiles", "278", "-Wnum_tiles"),
            "data/tilesets/secondary/pretty_petal_flower_shop/tiles.4bpp.fastSmol": ("-num_tiles", "345", "-Wnum_tiles"),
            "data/tilesets/secondary/generic_building/tiles.4bpp.fastSmol": ("-num_tiles", "509", "-Wnum_tiles"),
            "data/tilesets/secondary/rustboro_gym/tiles.4bpp.fastSmol": ("-num_tiles", "60", "-Wnum_tiles"),
        }
        graphics = [row for row in manifest["records"] if row.get("component") == "graphics"]
        self.assertEqual(ASSETS.tileset_gfx_dir(GAME), Path("data/tilesets"))
        self.assertEqual({row["generated_path"] for row in graphics if ASSETS.graphics_encoder_args(GAME, GAME / row["generated_path"])}, set(expected))
        self.assertEqual(ASSETS.compression_encoder_args(GAME, Path("tiles.4bpp.fastSmol")), ("-w", "$<", "$@", "false", "false", "false"))
        self.assertEqual(ASSETS.compression_encoder_args(GAME, Path("tiles.4bpp.smol")), ("-w", "$<", "$@"))
        for row in graphics:
            rule = ASSETS.graphics_encoder_args(GAME, GAME / row["generated_path"])
            self.assertEqual(rule, expected.get(row["generated_path"], ()))
            expected_rule = (f"graphics_file_rules.mk:{ASSETS.decoded_path(Path(row['generated_path'])).as_posix()}"
                             if rule else "Makefile:%.4bpp:%.png")
            self.assertEqual(row["encoder_rule"], expected_rule)
            self.assertEqual(row["encoder_args"], ["$<", "$@", *rule])

        # Regenerate every special target outside the worktree.  A generic
        # stale .4bpp cannot satisfy this byte comparison with a clean-like
        # invocation of the production rule.
        with tempfile.TemporaryDirectory() as directory:
            fresh_root = Path(directory)
            for row in graphics:
                if row["generated_path"] not in expected:
                    continue
                target = GAME / row["generated_path"]
                fresh = fresh_root / Path(row["generated_path"]).name
                ASSETS.generate_component(GAME, GAME / ASSETS.production_source(row), fresh, rule_output=target)
                self.assertEqual(fresh.read_bytes(), target.read_bytes(), row["record_id"])
                self.assertEqual(fresh.with_suffix("").read_bytes(), target.with_suffix("").read_bytes(), row["record_id"])

    def test_full_transformed_closure_uses_current_make_recipes(self):
        manifest = json.loads(self.assets_path.read_text())
        transformed = [
            row for row in manifest["records"]
            if row.get("asset_family") == "tileset_component"
            and row.get("component") != "descriptor"
            and ASSETS.production_source(row).suffix in {".png", ".pal"}
        ]
        self.assertEqual(len(transformed), 483)
        self.assertEqual({row["encoder_rule"] for row in transformed if row["component"] == "palette"}, {"Makefile:%.gbapal:%.pal"})
        self.assertEqual({row["encoder_rule"] for row in transformed if row["component"] == "animation"}, {"Makefile:%.4bpp:%.png"})
        for row in transformed:
            rule, args = ASSETS.gbagfx_recipe(GAME, ASSETS.production_source(row), Path(row["generated_path"]))
            self.assertEqual(row["encoder_rule"], rule)
            self.assertEqual(row["encoder_args"], list(args))
        self.assertEqual(ASSETS.verify_fresh_transformed_components(GAME, manifest), len(transformed))

    def test_production_verify_rejects_encoder_rule_drift_with_stale_outputs(self):
        manifest = json.loads(self.assets_path.read_text())
        rules = (GAME / "graphics_file_rules.mk").read_text(encoding="utf-8")
        drifted_rules = rules.replace("-num_tiles 502 -Wnum_tiles", "-num_tiles 501 -Wnum_tiles", 1)
        with self.assertRaisesRegex(ValueError, "fresh transformed production drift|GFX encoder rule drift"):
            ASSETS.verify(GAME, manifest, graphics_rules_text=drifted_rules)

        relocated_rules = rules.replace("TILESETGFXDIR := data/tilesets", "TILESETGFXDIR := data/alternate_tilesets", 1)
        with self.assertRaisesRegex(ValueError, "fresh transformed production drift|GFX encoder rule drift"):
            ASSETS.verify(GAME, manifest, graphics_rules_text=relocated_rules)
        with self.assertRaisesRegex(ValueError, "missing or ambiguous TILESETGFXDIR assignment"):
            ASSETS.tileset_gfx_dir(GAME, rules_text=rules.replace("TILESETGFXDIR := data/tilesets\n", "", 1))
        with self.assertRaisesRegex(ValueError, "unsupported TILESETGFXDIR assignment"):
            ASSETS.tileset_gfx_dir(GAME, rules_text=rules.replace(":= data/tilesets", "?= data/tilesets", 1))

        makefile = (GAME / "Makefile").read_text(encoding="utf-8")
        drifted_makefile = makefile.replace("%.fastSmol: %      ; $(SMOL) -w $< $@ false false false", "%.fastSmol: %      ; $(SMOL) $< $@ false false false", 1)
        with self.assertRaisesRegex(ValueError, "compression recipe semantic drift"):
            ASSETS.verify(GAME, manifest, makefile_text=drifted_makefile)

    def test_production_verify_rejects_generic_graphics_palette_and_animation_rule_drift(self):
        manifest = json.loads(self.assets_path.read_text())
        makefile = (GAME / "Makefile").read_text(encoding="utf-8")
        generic_graphics = makefile.replace("%.4bpp:     %.png  ; $(GFX) $< $@", "%.4bpp:     %.png  ; $(GFX) $@ $<", 1)
        with self.assertRaisesRegex(ValueError, "generic GFX recipe semantic drift for %\\.4bpp:%\\.png"):
            ASSETS.verify(GAME, manifest, makefile_text=generic_graphics)
        animation = next(row for row in manifest["records"] if row.get("component") == "animation")
        self.assertEqual(animation["encoder_rule"], "Makefile:%.4bpp:%.png")

        generic_palette = makefile.replace("%.gbapal:   %.pal  ; $(GFX) $< $@", "%.gbapal:   %.pal  ; $(GFX) $@ $<", 1)
        with self.assertRaisesRegex(ValueError, "generic GFX recipe semantic drift for %\\.gbapal:%\\.pal"):
            ASSETS.verify(GAME, manifest, makefile_text=generic_palette)
        palette = next(row for row in manifest["records"] if row.get("component") == "palette")
        self.assertEqual(palette["encoder_rule"], "Makefile:%.gbapal:%.pal")

    def test_runtime_inputs_schedule_asset_verification(self):
        runtime_inputs = (
            "Makefile",
            "graphics_file_rules.mk",
            "tools/gbagfx/gbagfx",
            "tools/compresSmol/compresSmol",
            "src/data/tilesets/graphics.h",
            "src/data/tilesets/headers.h",
            "src/data/tilesets/metatiles.h",
            "src/tileset_anims.c",
        )
        for path in runtime_inputs:
            result = subprocess.run(
                ["make", "NODEP=1", "BUILD=wayfarer", "-n", "-W", path, "wayfarer-sinnoh-port-assets"],
                cwd=GAME,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("assets.py --root . --verify", result.stdout, path)
            if path.startswith("tools/"):
                tool_directory = path.rsplit("/", 1)[0]
                self.assertLess(result.stdout.index(f"make -C {tool_directory}"), result.stdout.index("assets.py --root . --verify"), path)

    def test_asset_verifier_rejects_stale_production_proof_and_missing_consumer(self):
        manifest = json.loads(self.assets_path.read_text())
        component = next(row for row in manifest["records"] if row["asset_family"] == "tileset_component" and row["component"] != "descriptor")
        component["generated_sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "exact reference production mismatch|generated production hash"):
            ASSETS.verify(GAME, manifest, production=False)
        manifest = json.loads(self.assets_path.read_text())
        component = next(row for row in manifest["records"] if row["asset_family"] == "tileset_component")
        component["consumers"] = []
        with self.assertRaisesRegex(ValueError, "undeclared consumer"):
            ASSETS.verify(GAME, manifest, production=False)

    def test_layout_alias_resolver_rejects_chains_and_wrong_owner(self):
        manifest = json.loads(self.assets_path.read_text())
        alias = next(row for row in manifest["records"] if row["asset_family"] == "blockdata" and row["reuse_class"] == "EXACT_ALIAS")
        alias["alias_of"] = alias["record_id"]
        with self.assertRaisesRegex(ValueError, "chains"):
            ASSETS.resolve_layout_aliases(GAME, manifest)
        manifest = json.loads(self.assets_path.read_text())
        alias = next(row for row in manifest["records"] if row["asset_family"] == "border" and row["reuse_class"] == "EXACT_ALIAS")
        alias["canonical_owner"] = "LAYOUT_NOT_AN_OWNER"
        with self.assertRaisesRegex(ValueError, "canonical layout owner"):
            ASSETS.resolve_layout_aliases(GAME, manifest)

    def test_asset_contract_rejects_shape_length_compression_and_existing_target_drift(self):
        manifest = json.loads(self.assets_path.read_text())
        component = next(row for row in manifest["records"] if row.get("asset_family") == "tileset_component" and row["component"] == "graphics")
        component["decoded_bytes"] += 1
        with self.assertRaisesRegex(ValueError, "production shape drift|generated production hash|exact reference production mismatch"):
            ASSETS.verify(GAME, manifest, production=False)
        manifest = json.loads(self.assets_path.read_text())
        component = next(row for row in manifest["records"] if row.get("asset_family") == "tileset_component" and row["component"] == "graphics")
        component["compression"] = False
        with self.assertRaisesRegex(ValueError, "compression"):
            ASSETS.verify(GAME, manifest, production=False)
        manifest = json.loads(self.assets_path.read_text())
        component = next(row for row in manifest["records"] if row["reuse_class"] == "SINNOH_VARIANT" and row["component"] != "descriptor")
        component["proposed_target_symbol"] = "gTileset_General"
        with self.assertRaisesRegex(ValueError, "variant reaches"):
            ASSETS.verify(GAME, manifest, production=False)

    def test_asset_contract_rejects_missing_checked_source_and_filename_only_match(self):
        manifest = json.loads(self.assets_path.read_text())
        component = next(row for row in manifest["records"] if row.get("storage_role") == "source_only_proof")
        component["checked_in_source_sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "checked-in source drift"):
            ASSETS.verify(GAME, manifest, production=False)
        manifest = json.loads(self.assets_path.read_text())
        component = next(row for row in manifest["records"] if row["reuse_class"] == "EXISTING_REFERENCE" and row["component"] != "descriptor")
        component["source_sha256"] = "0" * 64
        with self.assertRaisesRegex(AUDIT.FoundationError, "donor tileset source hash drift"):
            AUDIT.validate_donor(Path("/tmp/sinnoh-donor-4eed17"), json.loads(self.maps_path.read_text())["maps"], AUDIT.records_by_id(manifest))

    def test_asset_runtime_closure_uses_actual_descriptor_consumers(self):
        manifest = json.loads(self.assets_path.read_text())
        ASSETS.verify_runtime_component_closure(GAME, manifest)
        building_tiles = next(row for row in manifest["records"] if row["record_id"] == "tileset.gTileset_Building.tiles.png")
        self.assertEqual(building_tiles["generated_symbol"], "gTilesetTiles_InsideBuilding")
        self.assertEqual(building_tiles["generated_path"], "data/tilesets/primary/building/tiles.4bpp.smol")
        self.assertEqual(building_tiles["canonical_owner"], "gTilesetTiles_InsideBuilding")

        nonexistent = copy.deepcopy(manifest)
        component = next(row for row in nonexistent["records"] if row["record_id"] == "tileset.gTileset_Building.tiles.png")
        component["generated_symbol"] = "gTilesetTiles_Sinnoh_Building"
        with self.assertRaisesRegex(ValueError, "runtime symbol closure drift"):
            ASSETS.verify(GAME, nonexistent, production=False)

        unreachable = copy.deepcopy(manifest)
        component = next(row for row in unreachable["records"] if row["record_id"] == "tileset.gTileset_Building.tiles.png")
        component["generated_path"] = "data/tilesets/sinnoh/primary/building/tiles.4bpp.fastSmol"
        with self.assertRaisesRegex(ValueError, "runtime path closure drift"):
            ASSETS.verify(GAME, unreachable, production=False)

    def test_animation_contract_uses_production_u16_declarations(self):
        manifest = json.loads(self.assets_path.read_text())
        animation = next(row for row in manifest["records"] if row["record_id"] == "tileset.gTileset_Building.anim.tv_turned_on.0.png")
        self.assertEqual(animation["array_shape"]["element_type"], "u16")
        self.assertEqual(animation["array_shape"]["element_count"], animation["decoded_bytes"] // 2)
        self.assertEqual(animation["alignment"], 4)

        wrong_type = copy.deepcopy(manifest)
        animation = next(row for row in wrong_type["records"] if row["record_id"] == "tileset.gTileset_Building.anim.tv_turned_on.0.png")
        animation["array_shape"]["element_type"] = "u8"
        with self.assertRaisesRegex(ValueError, "production shape drift"):
            ASSETS.verify(GAME, wrong_type, production=False)

        wrong_alignment = copy.deepcopy(manifest)
        animation = next(row for row in wrong_alignment["records"] if row["record_id"] == "tileset.gTileset_Building.anim.tv_turned_on.0.png")
        animation["alignment"] = 2
        with self.assertRaisesRegex(ValueError, "production alignment drift"):
            ASSETS.verify(GAME, wrong_alignment, production=False)

    def test_collision_sections_keep_natural_sinnoh_labels(self):
        rows = {row["source_map"]: row for row in json.loads(self.maps_path.read_text())["maps"]}
        for name in ("TwinleafTown_Haouse1", "TwinleafTown_House2"):
            section = rows[name]["map_properties"]
            self.assertEqual(section["source_map_section"], "MAPSEC_LITTLEROOT_TOWN")
            self.assertEqual(section["target_map_section"], "MAPSEC_SINNOH_TWINLEAF_TOWN")
            self.assertEqual(section["target_map_section_label"], "Twinleaf Town")
        self.assertEqual(rows["PokmonLeague"]["map_properties"]["target_map_section_label"], "Sinnoh Pokemon League")


if __name__ == "__main__":
    unittest.main()
