import unittest

from evidence import (EvidenceError, parse_elf_memory, summarize_rollback,
                      summarize_timing, validate_legacy_symbols)


def timing_cases(value):
    return {
        "black-screen-warp-emerald-canary": [value] * 100,
        "black-screen-warp-frlg-canary": [value] * 100,
        "black-screen-warp-hns-canary": [value] * 100,
        "black-screen-save-reload-hns-canary": [value] * 100,
    }


class EvidenceTest(unittest.TestCase):
    def test_legacy_baseline_rejects_linked_compression_runtime(self):
        validate_legacy_symbols("08000000 00000004 T OrdinarySymbol\n")
        with self.assertRaises(EvidenceError):
            validate_legacy_symbols("08000000 00000004 T MapLayoutReadTile\n")

    def test_timing_requires_paired_hundred_sample_cases_and_enforces_maximum(self):
        document = {
            "schema_version": 1,
            "source_revision": "abc",
            "device": "SkyEmu pinned build",
            "timer": "complete field transition frames",
            "measurement_revision": "test-v1",
            "builds": {mode: {"rom_sha256": str(index) * 64}
                       for index, mode in enumerate(("legacy", "raw", "hybrid"), 1)},
            "configurations": {
                "legacy": timing_cases(20),
                "raw": timing_cases(21),
                "hybrid": timing_cases(25),
            },
        }
        report = summarize_timing(document, "abc")
        self.assertTrue(report["passed"])
        self.assertEqual(report["cases"][0]["legacy_to_raw_additional_frames"]["max"], 1)
        self.assertEqual(report["cases"][0]["raw_to_hybrid_additional_frames"]["max"], 4)
        self.assertEqual(report["cases"][0]["legacy_to_hybrid_additional_frames"]["max"], 5)
        document["configurations"]["hybrid"]["black-screen-warp-emerald-canary"][-1] = 26
        self.assertFalse(summarize_timing(document, "abc")["passed"])

    def test_timing_rejects_short_or_unpaired_samples(self):
        document = {
            "schema_version": 1, "source_revision": "abc", "device": "hardware", "timer": "frames",
            "measurement_revision": "test-v1",
            "builds": {mode: {"rom_sha256": "1" * 64}
                       for mode in ("legacy", "raw", "hybrid")},
            "configurations": {
                "legacy": {**timing_cases(1), "black-screen-warp-emerald-canary": [1] * 99},
                "raw": timing_cases(1), "hybrid": timing_cases(1),
            },
        }
        with self.assertRaisesRegex(EvidenceError, "at least 100"):
            summarize_timing(document, "abc")

    def test_rollback_requires_hybrid_to_raw_map_and_decorated_base(self):
        digest = "a" * 64
        document = {
            "schema_version": 1,
            "source_revision": "abc",
            "roms": {
                "hybrid": {"rom_sha256": "1" * 64},
                "raw": {"rom_sha256": "2" * 64},
            },
            "hybrid_save": {"map": "canary", "player": {"x": 1}, "save_sha256": digest},
            "raw_continue": {"map": "canary", "player": {"x": 1}, "resaved_and_reloaded": True},
            "decorated_secret_base": {
                "map": "base", "decoration_count": 2, "decoration_fingerprint": 42,
                "save_sha256": digest,
            },
            "raw_decorated_secret_base_continue": {
                "map": "base", "decoration_count": 2, "decoration_fingerprint": 42,
                "resaved_and_reloaded": True,
            },
        }
        self.assertTrue(summarize_rollback(document, "abc")["passed"])
        document["raw_decorated_secret_base_continue"]["decoration_fingerprint"] = 43
        with self.assertRaisesRegex(EvidenceError, "decorated Secret Base"):
            summarize_rollback(document, "abc")

    def test_elf_report_retains_required_regions_and_symbols(self):
        memory = parse_elf_memory(
            ".ewram 0x02000000 0x4\n.ewram.sbss 0x02000004 0x20\n"
            ".iwram 0x03000000 0x8\n.iwram.bss 0x03000008 0x10\n",
            "02000004 0000000c B gMapLayoutLoadError\n"
            "02000010 00001000 B gHeap\n02001010 00005000 B sBackupMapData\n",
        )
        self.assertEqual(memory["ewram_bytes"], 0x24)
        self.assertEqual(memory["iwram_bytes"], 0x18)
        self.assertEqual(memory["symbols"]["sBackupMapData"]["bytes"], 0x5000)


if __name__ == "__main__":
    unittest.main()
