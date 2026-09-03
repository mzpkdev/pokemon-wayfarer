import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("generate.py")
SPEC = importlib.util.spec_from_file_location("wayfarer_source_constants", MODULE_PATH)
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)


class SourceConstantTests(unittest.TestCase):
    def test_persistent_flag_is_namespaced(self):
        self.assertEqual(GENERATOR.hoenn_value("FLAG_RESCUED_BIRCH", 0x52), 0x6052)
        self.assertEqual(GENERATOR.hoenn_value("FLAG_SYS_GAME_CLEAR", 0x864), 0x6864)

    def test_nonpersistent_flag_keeps_engine_id(self):
        self.assertEqual(GENERATOR.hoenn_value("FLAG_TEMP_1", 1), 1)
        self.assertEqual(GENERATOR.hoenn_value("FLAG_SPECIAL", 0x4001), 0x4001)

    def test_normal_var_is_namespaced(self):
        self.assertEqual(GENERATOR.hoenn_value("VAR_LITTLEROOT_TOWN_STATE", 0x4050), 0x7050)
        self.assertEqual(GENERATOR.hoenn_value("VAR_RESULT", 0x800D), 0x800D)

    def test_macro_aliases_resolve_to_integers(self):
        values, skipped = GENERATOR.resolve_constants(
            {
                "FLAG_BASE": "0x20",
                "FLAG_ALIAS": "(FLAG_BASE + 3)",
                "VAR_TEST": "0x4000U",
            }
        )
        self.assertFalse(skipped)
        self.assertEqual(values["FLAG_ALIAS"], 0x23)
        self.assertEqual(values["VAR_TEST"], 0x4000)

    def test_collects_only_emerald_map_section_constants(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            maps = Path(temp_dir)
            for name, source, mapsec in (
                ("Petalburg", "emerald", "MAPSEC_PETALBURG_CITY"),
                ("NewBark", "hns", "MAPSEC_NEW_BARK_TOWN"),
            ):
                path = maps / name
                path.mkdir()
                (path / "map.json").write_text(json.dumps({
                    "game_version": source,
                    "region_map_section": mapsec,
                }))

            self.assertEqual(
                GENERATOR.collect_hoenn_map_section_names(maps),
                {"MAPSEC_PETALBURG_CITY"},
            )

    def test_starter_audit_rejects_raw_symbol_in_emerald_map(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            maps = Path(temp_dir)
            emerald = maps / "Route103"
            emerald.mkdir()
            (emerald / "map.json").write_text(json.dumps({"game_version": "emerald"}))
            (emerald / "scripts.inc").write_text("\tswitch VAR_STARTER_MON\n")

            with self.assertRaisesRegex(
                ValueError,
                r"Route103/scripts\.inc:1.*use VAR_HOENN_STARTER_CHOICE",
            ):
                GENERATOR.audit_hoenn_starter_symbols(maps)

    def test_starter_audit_ignores_hns_source_maps(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            maps = Path(temp_dir)
            hns = maps / "NewBarkTown_hns"
            hns.mkdir()
            (hns / "map.json").write_text(json.dumps({"game_version": "hns"}))
            (hns / "scripts.inc").write_text("\tsetvar VAR_STARTER_MON, 0\n")

            self.assertEqual(GENERATOR.audit_hoenn_starter_symbols(maps), [])


if __name__ == "__main__":
    unittest.main()
