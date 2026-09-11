"""Temporary selected-product expectations for the rod and mart migration.

Remove this module with the migration target when v1 phase D is accepted and
equivalent domain coverage replaces these pre-framework snapshots.
"""
from pathlib import Path
import unittest

from tools.gameplay_content.maps import load_maps
from tools.gameplay_content.services import compile_services

ROOT = Path(__file__).resolve().parents[3]


class SelectedProductMigrationTests(unittest.TestCase):
    def test_independent_contributor_flags_and_mart_coverage(self):
        six = {
            "FLAG_STANDARD_ROD_ROUTE32_CONTRIBUTED",
            "FLAG_STANDARD_ROD_OLIVINE_CONTRIBUTED",
            "FLAG_STANDARD_ROD_ROUTE12_CONTRIBUTED",
            "FLAG_STANDARD_ROD_DEWFORD_CONTRIBUTED",
            "FLAG_STANDARD_ROD_ROUTE118_CONTRIBUTED",
            "FLAG_STANDARD_ROD_MOSSDEEP_CONTRIBUTED",
        }
        for product, switches, expected in (
            ("wayfarer", ["-DPOKEMON_WAYFARER", "-DPOKEMON_HNS"], six),
            ("hns", ["-DPOKEMON_HNS"], six),
            ("emerald", ["-DEMERALD"], {"FLAG_RECEIVED_OLD_ROD", "FLAG_RECEIVED_GOOD_ROD", "FLAG_RECEIVED_SUPER_ROD"}),
            ("firered", ["-DFIRERED"], {"FLAG_GOT_OLD_ROD", "FLAG_GOT_GOOD_ROD", "FLAG_GOT_SUPER_ROD"}),
            ("leafgreen", ["-DLEAFGREEN"], {"FLAG_GOT_OLD_ROD", "FLAG_GOT_GOOD_ROD", "FLAG_GOT_SUPER_ROD"}),
        ):
            with self.subTest(product=product):
                result = compile_services(
                    ROOT,
                    product,
                    {"IS_WAYFARER": int(product == "wayfarer"), "WAYFARER_TR_MARTS_ENABLED": 1},
                    load_maps(ROOT, product),
                    cppflags=switches,
                )
                report = result["report"]
                contributors = [row for row in report["services"] if row["kind"] == "rod_contribution"]
                contributors += report["legacyRodContributors"]
                self.assertEqual({row["contribution"]["flag"] for row in contributors}, expected)
                self.assertEqual(report["martCount"], 35 if product == "wayfarer" else 0)
                self.assertFalse(any(row["sourceNamespace"] == "frlg" for row in report["services"] if row["kind"] == "mart"))
                if product == "hns":
                    self.assertEqual(len(report["legacyRodContributors"]), 3)
                    self.assertTrue(all(not row["activeBinding"] for row in report["legacyRodContributors"]))


if __name__ == "__main__":
    unittest.main()
