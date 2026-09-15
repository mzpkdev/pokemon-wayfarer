import json
from pathlib import Path
import re
import unittest


GAME = Path(__file__).resolve().parents[3]
REPO = GAME.parent
INVENTORY = REPO / "docs/sevii-trainer-implementation/allocation-inventory.json"
REGISTRY = GAME / "src/data/wayfarer_sevii_rematches.h"


class SeviiRematchRegistryTests(unittest.TestCase):
    def test_registry_covers_each_rematch_family_in_frozen_party_order(self):
        inventory = json.loads(INVENTORY.read_text(encoding="utf-8"))
        allocations = inventory["allocation"]["allocations"]
        by_base = {}
        for allocation in allocations:
            if allocation["owner"] == "ordinary_trainer":
                by_base.setdefault(allocation["defeat_base"], []).append(allocation["id"])

        expected_bases = []
        for obj in inventory["ordinary"]["objects_inventory"]:
            base = obj["source_trainer"]
            if obj["rematch_capable"] and base not in expected_bases:
                expected_bases.append(base)

        rows = []
        for line in REGISTRY.read_text(encoding="utf-8").splitlines():
            match = re.fullmatch(r"SEVII_REMATCH\(([^)]*)\),", line)
            if match:
                rows.append([item.strip() for item in match.group(1).split(",")])

        expected = []
        for base in expected_bases:
            parties = by_base[base]
            expected.append(parties + [parties[-1]] * (5 - len(parties)))

        self.assertEqual(len(expected), 64)
        self.assertEqual(rows, expected)
        self.assertEqual({row[0] for row in rows}, {by_base[base][0] for base in expected_bases})
        self.assertTrue(all(symbol.startswith("TRAINER_WAYFARER_SEVII_") for row in rows for symbol in row))


if __name__ == "__main__":
    unittest.main()
