from __future__ import annotations

import contextlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path


TOOL_DIR = Path(__file__).resolve().parents[1]
FIXTURES = Path(__file__).resolve().parent / "fixtures"
sys.path.insert(0, str(TOOL_DIR))

import ram_report  # noqa: E402


GENEROUS = {"ewram_used": 262_144, "iwram_static": 32_320}


def budget_document(**overrides: object) -> dict[str, object]:
    builds = {
        "release": {"ewram_used": 251_876, "iwram_static": 25_952},
        "e2e": {"ewram_used": 257_868, "iwram_static": 25_956},
        "test": {"ewram_used": None, "iwram_static": 30_052},
    }
    document: dict[str, object] = {"schema_version": 1, "builds": builds}
    document.update(overrides)
    return document


class RamReportTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.release_text = (FIXTURES / "release.map").read_text(encoding="utf-8")
        cls.release = ram_report.measure(ram_report.parse_map_sections(cls.release_text))
        cls.budget = ram_report.parse_budget(json.dumps(budget_document()))

    def test_release_map_measures(self) -> None:
        self.assertEqual(self.release["ewram_used"], 250_852)
        self.assertEqual(self.release["ewram_free"], 11_292)
        self.assertEqual(self.release["iwram_static"], 25_696)
        self.assertEqual(self.release["stack_room"], 6_624)

    def test_memory_configuration_and_input_sections_are_ignored(self) -> None:
        sections = ram_report.parse_map_sections(self.release_text)
        self.assertEqual(sections[".ewram.sbss"], (0x02000004, 0x3D3E0))
        self.assertNotIn("EWRAM", sections)
        self.assertNotIn(".sbss", sections)

    def test_wrapped_persistent_section_is_parsed_and_excluded(self) -> None:
        text = (FIXTURES / "test.map").read_text(encoding="utf-8")
        sections = ram_report.parse_map_sections(text)
        self.assertEqual(sections[".iwram.persistent"], (0x03007F00, 0x4))

        measured = ram_report.measure(sections)
        self.assertEqual(measured["ewram_used"], 262_112)
        self.assertEqual(measured["ewram_free"], 32)
        self.assertEqual(measured["iwram_static"], 29_796)
        self.assertEqual(measured["stack_room"], 2_524)

    def test_test_build_ewram_is_report_only(self) -> None:
        text = (FIXTURES / "test.map").read_text(encoding="utf-8")
        measured = ram_report.measure(ram_report.parse_map_sections(text))
        report = ram_report.build_report(
            measured, build="test", ceilings=self.budget["test"]
        )
        self.assertFalse(report["ewram"]["enforced"])
        self.assertTrue(report["iwram"]["enforced"])
        self.assertIsNone(report["stack"]["min_room_bytes"])

    def test_release_within_budget_reports(self) -> None:
        report = ram_report.build_report(
            self.release, build="release", ceilings=self.budget["release"]
        )
        self.assertEqual(report["ewram"]["used_bytes"], 250_852)
        self.assertEqual(report["iwram"]["static_end_address"], "0x03006460")
        self.assertEqual(report["stack"]["min_room_bytes"], 4096)

    def test_over_budget_fixture_names_build_measure_values_and_fix(self) -> None:
        text = (FIXTURES / "over_budget.map").read_text(encoding="utf-8")
        measured = ram_report.measure(ram_report.parse_map_sections(text))
        with self.assertRaises(ram_report.BudgetError) as caught:
            ram_report.build_report(
                measured, build="release", ceilings=self.budget["release"]
            )

        (message,) = caught.exception.violations
        self.assertIn("release build", message)
        self.assertIn("IWRAM static", message)
        self.assertIn("26,720 bytes", message)
        self.assertIn("ceiling 25,952 bytes", message)
        self.assertIn("EWRAM_DATA", message)
        self.assertIn("ram_budget.json", message)

    def test_exact_ceiling_passes_and_one_byte_over_fails(self) -> None:
        ceilings = {"ewram_used": 250_852, "iwram_static": 25_696}
        ram_report.build_report(self.release, build="e2e", ceilings=ceilings)

        ceilings = {"ewram_used": 250_851, "iwram_static": 25_696}
        with self.assertRaisesRegex(ram_report.BudgetError, "EWRAM used is 250,852"):
            ram_report.build_report(self.release, build="e2e", ceilings=ceilings)

    def test_release_floors_hold_even_with_generous_ceilings(self) -> None:
        measured = dict(self.release)
        measured["stack_room"] = 4_095
        measured["ewram_free"] = 4_095
        with self.assertRaises(ram_report.BudgetError) as caught:
            ram_report.build_report(measured, build="release", ceilings=GENEROUS)
        messages = "\n".join(caught.exception.violations)
        self.assertIn("stack room is 4,095 bytes, hard floor 4,096", messages)
        self.assertIn("EWRAM free is 4,095 bytes, hard floor 4,096", messages)
        self.assertIn("cannot be relaxed", messages)

        # The floors are release-only; other builds rely on their ceilings.
        ram_report.build_report(measured, build="e2e", ceilings=GENEROUS)

    def test_budget_cannot_set_release_ceiling_past_floor(self) -> None:
        document = budget_document()
        document["builds"]["release"]["iwram_static"] = 28_225  # type: ignore[index]
        with self.assertRaisesRegex(ram_report.ReportError, "stack floor"):
            ram_report.parse_budget(json.dumps(document))

        document = budget_document()
        document["builds"]["release"]["ewram_used"] = 258_049  # type: ignore[index]
        with self.assertRaisesRegex(ram_report.ReportError, "EWRAM floor"):
            ram_report.parse_budget(json.dumps(document))

    def test_release_measures_cannot_be_report_only(self) -> None:
        document = budget_document()
        document["builds"]["release"]["ewram_used"] = None  # type: ignore[index]
        with self.assertRaisesRegex(ram_report.ReportError, "cannot be report-only"):
            ram_report.parse_budget(json.dumps(document))

    def test_malformed_budgets_are_rejected(self) -> None:
        cases = {
            "malformed budget JSON": "{",
            "schema_version": json.dumps(budget_document(schema_version=2)),
            "builds must list exactly": json.dumps(
                budget_document(builds={"release": {}})
            ),
        }
        for message, text in cases.items():
            with self.subTest(message=message):
                with self.assertRaisesRegex(ram_report.ReportError, message):
                    ram_report.parse_budget(text)

        document = budget_document()
        document["builds"]["e2e"]["ewram_used"] = 262_145  # type: ignore[index]
        with self.assertRaisesRegex(ram_report.ReportError, "hardware limit"):
            ram_report.parse_budget(json.dumps(document))

        document = budget_document()
        document["builds"]["e2e"]["iwram_static"] = True  # type: ignore[index]
        with self.assertRaisesRegex(ram_report.ReportError, "nonnegative integer"):
            ram_report.parse_budget(json.dumps(document))

    def test_missing_section_and_non_map_are_rejected(self) -> None:
        text = self.release_text.replace(".iwram.bss      0x03000198", ".other  0x03000198")
        with self.assertRaisesRegex(ram_report.ReportError, "missing RAM output sections: .iwram.bss"):
            ram_report.parse_map_sections(text)
        with self.assertRaisesRegex(ram_report.ReportError, "not a GNU ld map"):
            ram_report.parse_map_sections("08000000 g 00000000 __rom_start\n")

    def test_checked_in_budget_admits_main_and_pr_147_measurements(self) -> None:
        budget = ram_report.parse_budget(
            ram_report.DEFAULT_BUDGET.read_text(encoding="utf-8")
        )
        # Measured on main at cf4d73bfa2 and on PR #147 (overworld walkers).
        for build, ewram_used, iwram_static in (
            ("release", 251_392, 25_696),
            ("release", 250_852, 25_696),
            ("e2e", 257_380, 25_700),
            ("e2e", 256_844, 25_700),
            ("test", 262_004, 29_796),
            ("test", 262_112, 29_796),
        ):
            with self.subTest(build=build):
                measured = {
                    "ewram_used": ewram_used,
                    "ewram_free": 262_144 - ewram_used,
                    "iwram_static": iwram_static,
                    "iwram_static_end": 0x03000000 + iwram_static,
                    "stack_room": 32_320 - iwram_static,
                }
                ram_report.build_report(measured, build=build, ceilings=budget[build])

    def test_cli_writes_report_and_fails_without_stale_output(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "pokewayfarer-ram.json"
            status = ram_report.main(
                ["--map", str(FIXTURES / "release.map"), "--build", "release", "--output", str(output)]
            )
            self.assertEqual(status, 0)
            self.assertEqual(
                json.loads(output.read_text(encoding="utf-8"))["iwram"]["static_bytes"], 25_696
            )

            stderr = io.StringIO()
            with contextlib.redirect_stderr(stderr):
                status = ram_report.main(
                    ["--map", str(FIXTURES / "over_budget.map"), "--build", "release", "--output", str(output)]
                )
            self.assertEqual(status, 1)
            self.assertFalse(output.exists())
            self.assertIn("RAM budget check failed", stderr.getvalue())
            self.assertIn("release build: IWRAM static", stderr.getvalue())


if __name__ == "__main__":
    unittest.main()
