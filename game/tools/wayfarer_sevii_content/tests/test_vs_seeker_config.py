import os
from pathlib import Path
import subprocess
import unittest


GAME = Path(__file__).resolve().parents[3]
INCLUDE = GAME / "include"


def expand_vs_seeker_flag(product_macro=None):
    command = [os.environ.get("CC", "cc"), "-E", "-P", "-I", str(INCLUDE), "-x", "c", "-"]
    if product_macro is not None:
        command.insert(1, f"-D{product_macro}")
    result = subprocess.run(
        command,
        input='#include "global.h"\nI_VS_SEEKER_CHARGING\n',
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=True,
    )
    return result.stdout.strip().splitlines()[-1].replace(" ", "")


class VsSeekerConfigTests(unittest.TestCase):
    def test_charging_is_enabled_only_for_wayfarer(self):
        self.assertEqual(expand_vs_seeker_flag("POKEMON_WAYFARER"), "(0xC000|(52))")
        self.assertEqual(expand_vs_seeker_flag("POKEMON_HNS"), "0")
        self.assertEqual(expand_vs_seeker_flag("FIRERED"), "0")
        self.assertEqual(expand_vs_seeker_flag(), "0")

    def test_vs_seeker_item_uses_the_charging_gate(self):
        items = (GAME / "src/data/items.h").read_text(encoding="utf-8")
        start = items.index("[ITEM_VS_SEEKER]")
        entry = items[start:items.index("[ITEM_TM_CASE]", start)]

        self.assertIn("#if I_VS_SEEKER_CHARGING != 0", entry)
        self.assertIn(".fieldUseFunc = FieldUseFunc_VsSeeker", entry)
        self.assertIn(".fieldUseFunc = ItemUseOutOfBattle_CannotUse", entry)


if __name__ == "__main__":
    unittest.main()
