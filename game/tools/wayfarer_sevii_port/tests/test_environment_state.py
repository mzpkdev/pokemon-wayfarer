from pathlib import Path
import unittest


GAME = Path(__file__).resolve().parents[3]


class WayfarerSeviiEnvironmentStateTests(unittest.TestCase):
    def test_sevii_state_uses_reserved_wayfarer_ids(self):
        flags = (GAME / "include/constants/flags.h").read_text()
        vars_ = (GAME / "include/constants/vars.h").read_text()

        self.assertIn("#define FLAG_WAYFARER_SEVII_DOTTED_HOLE_OPEN WAYFARER_SEVII_FLAG_ID(0)", flags)
        self.assertIn("#define FLAG_WAYFARER_SEVII_ICEFALL_CRACKED_ICE_START WAYFARER_SEVII_FLAG_ID(1)", flags)
        self.assertIn("#define FLAG_WAYFARER_SEVII_ICEFALL_CRACKED_ICE_COUNT 9", flags)
        self.assertIn("#define FLAG_WAYFARER_SEVII_TANOBY_COMPLETE WAYFARER_SEVII_FLAG_ID(10)", flags)
        self.assertIn("#define VAR_WAYFARER_SEVII_ICEFALL_FALL                  0x40FF", vars_)

    def test_dotted_hole_keeps_standalone_flag_path(self):
        source = (GAME / "src/field_specials.c").read_text()

        self.assertIn("#if IS_WAYFARER\n    if (FlagGet(FLAG_WAYFARER_SEVII_DOTTED_HOLE_OPEN)", source)
        self.assertIn("#else\n    if (FlagGet(FLAG_USED_CUT_ON_RUIN_VALLEY_BRAILLE)", source)
        self.assertIn("#if IS_WAYFARER\n    FlagSet(FLAG_WAYFARER_SEVII_DOTTED_HOLE_OPEN);", source)
        self.assertIn("#else\n    FlagSet(FLAG_USED_CUT_ON_RUIN_VALLEY_BRAILLE);", source)

    def test_icefall_keeps_standalone_temporary_state_path(self):
        source = (GAME / "src/field_tasks.c").read_text()

        self.assertIn("FlagSet(FLAG_WAYFARER_SEVII_ICEFALL_CRACKED_ICE_START + i);", source)
        self.assertIn("FlagGet(FLAG_WAYFARER_SEVII_ICEFALL_CRACKED_ICE_START + i)", source)
        self.assertIn("VarSet(VAR_WAYFARER_SEVII_ICEFALL_FALL, 1);", source)
        self.assertIn("#else\n            FlagSet(i + 1);", source)
        self.assertIn("#else\n            VarSet(VAR_TEMP_1, 1);", source)


if __name__ == "__main__":
    unittest.main()
