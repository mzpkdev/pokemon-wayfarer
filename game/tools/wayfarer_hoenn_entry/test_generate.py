from __future__ import annotations

import importlib.util
import json
import struct
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("generate.py")
SPEC = importlib.util.spec_from_file_location("wayfarer_hoenn_entry", MODULE_PATH)
AUDIT = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(AUDIT)


class ArrivalAuditTest(unittest.TestCase):
    def make_fixture(self, *, collision: int = 0, overlap: bool = False) -> Path:
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        map_dir = root / "data/maps/SlateportCity_Harbor"
        layout_dir = root / "data/layouts/Harbor"
        heal_dir = root / "src/data"
        map_dir.mkdir(parents=True)
        layout_dir.mkdir(parents=True)
        heal_dir.mkdir(parents=True)

        coord_events = []
        if overlap:
            coord_events.append({"x": 9, "y": 11, "elevation": 3})
        (map_dir / "map.json").write_text(
            json.dumps(
                {
                    "id": AUDIT.ARRIVAL_MAP,
                    "game_version": "emerald",
                    "layout": "LAYOUT_HARBOR",
                    "coord_events": coord_events,
                    "object_events": [],
                    "warp_events": [
                        {
                            "x": 11,
                            "y": 14,
                            "elevation": 0,
                            "dest_map": "MAP_SLATEPORT_CITY",
                            "dest_warp_id": "8",
                        }
                    ],
                }
            ),
            encoding="utf-8",
        )
        (root / "data/layouts/layouts.json").write_text(
            json.dumps(
                {
                    "layouts": [
                        {
                            "id": "LAYOUT_HARBOR",
                            "width": 24,
                            "height": 15,
                            "blockdata_filepath": "data/layouts/Harbor/map.bin",
                        }
                    ]
                }
            ),
            encoding="utf-8",
        )
        blocks = [0x3C00] * (24 * 15)
        for x, y in ((9, 11), (10, 11), (11, 11), (11, 12), (11, 13)):
            blocks[y * 24 + x] = 0x3000 | (collision << 10)
        blocks[14 * 24 + 11] = 0
        (layout_dir / "map.bin").write_bytes(struct.pack(f"<{len(blocks)}H", *blocks))
        (heal_dir / "heal_locations.json").write_text(
            json.dumps(
                {
                    "heal_locations": [
                        {
                            "id": AUDIT.HEAL_LOCATION,
                            "source": "EMERALD",
                            "map": "MAP_SLATEPORT_CITY",
                        }
                    ]
                }
            ),
            encoding="utf-8",
        )
        return root

    def test_records_walkable_event_free_path_to_ordinary_exit(self) -> None:
        result = AUDIT.audit_arrival(self.make_fixture())
        self.assertEqual(result["coordinate"], [9, 11])
        self.assertEqual(result["collision"], 0)
        self.assertEqual(result["elevation"], 3)
        self.assertEqual(result["ordinaryExit"]["coordinate"], [11, 14])
        self.assertEqual(result["ordinaryExit"]["path"][0], [9, 11])
        self.assertEqual(result["ordinaryExit"]["path"][-1], [11, 14])

    def test_rejects_collision_at_arrival(self) -> None:
        with self.assertRaisesRegex(AUDIT.AuditError, "collision-blocked"):
            AUDIT.audit_arrival(self.make_fixture(collision=1))

    def test_rejects_arrival_event_overlap(self) -> None:
        with self.assertRaisesRegex(AUDIT.AuditError, "overlaps coord_events"):
            AUDIT.audit_arrival(self.make_fixture(overlap=True))


class ProductFilterTest(unittest.TestCase):
    def test_selects_wayfarer_and_standalone_hns_branches(self) -> None:
        source = """before
#if IS_WAYFARER
wayfarer
#else
hns
#endif
after"""
        self.assertEqual(AUDIT.filter_product(source, wayfarer=True).splitlines(), ["before", "wayfarer", "after"])
        self.assertEqual(AUDIT.filter_product(source, wayfarer=False).splitlines(), ["before", "hns", "after"])


class RepositoryContractTest(unittest.TestCase):
    def test_repository_hoenn_entry_contract(self) -> None:
        result = AUDIT.build_audit(AUDIT.GAME_ROOT)
        self.assertEqual(result["status"], "pass")
        self.assertEqual(result["travel"]["menu"]["slateportSlot"], 0)
        self.assertTrue(result["initialization"]["initializedCommittedLast"])
        self.assertEqual(result["hoennPorts"]["hoennSsAquaDepartures"], [])


if __name__ == "__main__":
    unittest.main()
