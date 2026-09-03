from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path


TOOL_DIR = Path(__file__).resolve().parents[1]
FIXTURES = Path(__file__).resolve().parent / "fixtures"
sys.path.insert(0, str(TOOL_DIR))

import rom_report  # noqa: E402


class RomReportTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.symbol_text = (FIXTURES / "at_limit.sym").read_text(encoding="utf-8")
        cls.symbols = rom_report.parse_symbols(cls.symbol_text)

    def test_wayfarer_release_accepts_exact_threshold(self) -> None:
        report = rom_report.build_report(
            self.symbols,
            build="wayfarer",
            release=True,
            baseline_sizes=None,
        )

        self.assertEqual(report["rom"]["end_address"], "0x09F80000")
        self.assertEqual(report["rom"]["used_bytes"], 33_030_144)
        self.assertEqual(report["rom"]["unused_bytes"], 524_288)
        self.assertTrue(report["rom"]["wayfarer_release_limit_enforced"])

    def test_wayfarer_release_rejects_one_byte_over_threshold(self) -> None:
        symbols = dict(self.symbols)
        symbols["__rom_category_other_end"] += 1
        symbols["__rom_end"] += 1

        with self.assertRaisesRegex(rom_report.ReportError, "exceeds 0x09F80000"):
            rom_report.build_report(
                symbols,
                build="wayfarer",
                release=True,
                baseline_sizes=None,
            )

    def test_non_wayfarer_or_nonrelease_does_not_apply_wayfarer_limit(self) -> None:
        symbols = dict(self.symbols)
        symbols["__rom_category_other_end"] += 1
        symbols["__rom_end"] += 1

        for build, release in (("hns", True), ("wayfarer", False)):
            report = rom_report.build_report(
                symbols,
                build=build,
                release=release,
                baseline_sizes=None,
            )
            self.assertFalse(report["rom"]["wayfarer_release_limit_enforced"])

    def test_missing_symbol_is_rejected(self) -> None:
        text = self.symbol_text.replace(
            "08000400 g 00000000 __rom_category_graphics_start\n", ""
        )
        with self.assertRaisesRegex(
            rom_report.ReportError, "missing ROM symbols: __rom_category_graphics_start"
        ):
            rom_report.parse_symbols(text)

    def test_malformed_symbol_is_rejected(self) -> None:
        text = self.symbol_text.replace(
            "08000000 g 00000000 __rom_start", "not-an-address __rom_start"
        )
        with self.assertRaisesRegex(rom_report.ReportError, "malformed ROM symbols"):
            rom_report.parse_symbols(text)

    def test_conflicting_duplicate_symbol_is_rejected(self) -> None:
        text = self.symbol_text + "08000004 g 00000000 __rom_start\n"
        with self.assertRaisesRegex(rom_report.ReportError, "conflicting addresses"):
            rom_report.parse_symbols(text)

    def test_linker_map_assignment_format_is_accepted(self) -> None:
        text = self.symbol_text.replace(
            "08000000 g 00000000 __rom_start",
            "0x0000000008000000                __rom_start = ORIGIN (ROM)",
        )
        self.assertEqual(rom_report.parse_symbols(text), self.symbols)

    def test_absent_baseline_has_null_deltas_and_no_fallback(self) -> None:
        report = rom_report.build_report(
            self.symbols,
            build="wayfarer",
            release=True,
            baseline_sizes=None,
        )

        self.assertEqual(report["baseline"], {"available": False})
        self.assertTrue(
            all(category["delta_bytes"] is None for category in report["categories"].values())
        )

    def test_wayfarer_release_selects_checked_in_baseline_by_default(self) -> None:
        selected = rom_report.select_baseline_path(
            build="wayfarer",
            release=True,
            explicit_path=None,
            bootstrap=False,
        )

        self.assertEqual(selected, rom_report.DEFAULT_WAYFARER_BASELINE)
        baseline_document = json.loads(selected.read_text(encoding="utf-8"))
        baseline = rom_report.parse_baseline(json.dumps(baseline_document))
        self.assertEqual(
            baseline_document["accepted_commit"],
            "4f3cd0ab2baeb0ee142e00dff64a215c94737d05",
        )
        self.assertEqual(baseline_document["rom"]["used_bytes"], 32_865_136)
        self.assertEqual(baseline_document["rom"]["end_address"], "0x09F57B70")
        self.assertEqual(sum(baseline.values()), 32_865_136)

    def test_explicit_baseline_overrides_checked_in_default(self) -> None:
        override = Path("custom-baseline.json")

        selected = rom_report.select_baseline_path(
            build="wayfarer",
            release=True,
            explicit_path=override,
            bootstrap=False,
        )

        self.assertEqual(selected, override)

    def test_explicit_bootstrap_disables_default_baseline(self) -> None:
        selected = rom_report.select_baseline_path(
            build="wayfarer",
            release=True,
            explicit_path=None,
            bootstrap=True,
        )

        self.assertIsNone(selected)

    def test_cli_default_override_and_bootstrap_baseline_behavior(self) -> None:
        cases = (
            ((), True, -2_706_512),
            (("--baseline", str(FIXTURES / "accepted_baseline.json")), True, 1),
            (("--bootstrap-baseline",), False, None),
        )
        with tempfile.TemporaryDirectory() as directory:
            for index, (baseline_arguments, available, code_delta) in enumerate(cases):
                with self.subTest(baseline_arguments=baseline_arguments):
                    output = Path(directory) / f"report-{index}.json"
                    result = rom_report.main(
                        [
                            "--symbols",
                            str(FIXTURES / "at_limit.sym"),
                            "--build",
                            "wayfarer",
                            "--release",
                            *baseline_arguments,
                            "--output",
                            str(output),
                        ]
                    )
                    report = json.loads(output.read_text(encoding="utf-8"))
                    self.assertEqual(result, 0)
                    self.assertEqual(report["baseline"]["available"], available)
                    self.assertEqual(
                        report["categories"]["code"]["delta_bytes"], code_delta
                    )

    def test_baseline_options_reject_invalid_contexts_and_combination(self) -> None:
        cases = (
            ("wayfarer", True, Path("override.json"), True),
            ("hns", True, Path("override.json"), False),
            ("wayfarer", False, None, True),
        )
        for build, release, explicit_path, bootstrap in cases:
            with self.subTest(
                build=build,
                release=release,
                explicit_path=explicit_path,
                bootstrap=bootstrap,
            ):
                with self.assertRaises(rom_report.ReportError):
                    rom_report.select_baseline_path(
                        build=build,
                        release=release,
                        explicit_path=explicit_path,
                        bootstrap=bootstrap,
                    )

    def test_standalone_and_nonrelease_reports_do_not_select_a_baseline(self) -> None:
        for build, release in (("hns", True), ("emerald", True), ("wayfarer", False)):
            with self.subTest(build=build, release=release):
                self.assertIsNone(
                    rom_report.select_baseline_path(
                        build=build,
                        release=release,
                        explicit_path=None,
                        bootstrap=False,
                    )
                )

    def test_accepted_wayfarer_baseline_produces_signed_deltas(self) -> None:
        baseline = rom_report.parse_baseline(
            (FIXTURES / "accepted_baseline.json").read_text(encoding="utf-8")
        )
        report = rom_report.build_report(
            self.symbols,
            build="wayfarer",
            release=True,
            baseline_sizes=baseline,
        )

        self.assertEqual(report["categories"]["code"]["delta_bytes"], 1)
        self.assertEqual(report["categories"]["scripts"]["delta_bytes"], -1)
        self.assertEqual(report["categories"]["maps_layouts"]["delta_bytes"], 0)

    def test_malformed_baselines_are_rejected(self) -> None:
        malformed = (
            "not JSON",
            '{"schema_version": true, "build": "wayfarer", "categories": {}}',
            '{"schema_version": 2, "build": "wayfarer", "categories": {}}',
            '{"schema_version": 1, "build": "hns", "categories": {}}',
            '{"schema_version": 1, "build": "wayfarer", "categories": {}}',
        )
        for text in malformed:
            with self.subTest(text=text):
                with self.assertRaises(rom_report.ReportError):
                    rom_report.parse_baseline(text)

    def test_baseline_reconciliation_is_required(self) -> None:
        baseline = json.loads(
            (FIXTURES / "accepted_baseline.json").read_text(encoding="utf-8")
        )
        baseline["rom"]["used_bytes"] += 1
        with self.assertRaisesRegex(rom_report.ReportError, "does not reconcile"):
            rom_report.parse_baseline(json.dumps(baseline))

        baseline["rom"]["used_bytes"] -= 1
        baseline["categories"]["other"]["bytes"] -= 1
        with self.assertRaisesRegex(rom_report.ReportError, "category bytes"):
            rom_report.parse_baseline(json.dumps(baseline))

    def test_explicit_missing_baseline_is_an_error(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            missing = Path(directory) / "missing.json"
            with self.assertRaisesRegex(rom_report.ReportError, "unable to read baseline"):
                rom_report._read_text(missing, "baseline")

    def test_category_gap_is_rejected(self) -> None:
        symbols = dict(self.symbols)
        symbols["__rom_category_maps_layouts_start"] += 4

        with self.assertRaisesRegex(rom_report.ReportError, "gap before maps_layouts"):
            rom_report.build_report(
                symbols,
                build="wayfarer",
                release=True,
                baseline_sizes=None,
            )

    def test_category_overlap_is_rejected(self) -> None:
        symbols = dict(self.symbols)
        symbols["__rom_category_maps_layouts_start"] -= 4

        with self.assertRaisesRegex(rom_report.ReportError, "overlap at maps_layouts"):
            rom_report.build_report(
                symbols,
                build="wayfarer",
                release=True,
                baseline_sizes=None,
            )

    def test_category_total_reconciles_and_json_is_deterministic(self) -> None:
        report = rom_report.build_report(
            self.symbols,
            build="wayfarer",
            release=True,
            baseline_sizes=None,
        )
        rendered_once = rom_report.render_report(report)
        rendered_twice = rom_report.render_report(report)

        self.assertEqual(rendered_once, rendered_twice)
        decoded = json.loads(rendered_once)
        self.assertEqual(
            decoded["reconciliation"]["category_bytes"], decoded["rom"]["used_bytes"]
        )
        self.assertTrue(decoded["reconciliation"]["matches_used_bytes"])


if __name__ == "__main__":
    unittest.main()
