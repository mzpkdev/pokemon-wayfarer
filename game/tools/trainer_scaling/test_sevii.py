"""Reviewed policy coverage for the generated Sevii selected roster."""

import importlib.util
from pathlib import Path
import unittest


_GENERATOR_PATH = Path(__file__).with_name("generate.py")
_SPEC = importlib.util.spec_from_file_location("trainer_scaling_generate", _GENERATOR_PATH)
gen = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(gen)


class SeviiPolicyTests(unittest.TestCase):
    def test_all_selected_ordinary_records_use_ordinary_policy(self):
        allocation = gen.json.loads(gen.SEVII_ALLOCATION.read_text())["allocation"]["allocations"]
        rows = {row["id"]: row for row in gen.load_sevii_manifest()}
        self.assertEqual(len(rows), 136)
        self.assertTrue(all(rows[row["id"]]["policy"] == "ORDINARY"
                            for row in allocation if row["owner"] == "ordinary_trainer"))

    def test_only_reviewed_story_guards_enter_ordinary_scaling(self):
        rows = {row["id"]: row for row in gen.load_sevii_manifest()}
        ordinary = {
            "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_43",
            "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_44",
            "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_46",
            "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_49",
            "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_50",
            "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_51",
        }
        story = {ident for ident in rows if ident.startswith("TRAINER_WAYFARER_SEVII_") and ident in {
            "TRAINER_WAYFARER_SEVII_BIKER_GOON", "TRAINER_WAYFARER_SEVII_BIKER_GOON_2", "TRAINER_WAYFARER_SEVII_BIKER_GOON_3", "TRAINER_WAYFARER_SEVII_CUE_BALL_PAXTON", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_43", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_44", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_45", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_46", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_49", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_50", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_51", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_42", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_47", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_48", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN", "TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN_2", "TRAINER_WAYFARER_SEVII_SCIENTIST_GIDEON", "TRAINER_WAYFARER_SEVII_LADY_SELPHY",
        }}
        self.assertEqual({ident for ident in story if rows[ident]["policy"] == "ORDINARY"}, ordinary)
        self.assertTrue(all(rows[ident].get("reason") for ident in story - ordinary))


if __name__ == "__main__":
    unittest.main()
