import importlib.util
from pathlib import Path
import unittest


GAME = Path(__file__).resolve().parents[3]
MODULE = GAME / "tools/wayfarer_trainer_tower/audit.py"
SPEC = importlib.util.spec_from_file_location("wayfarer_trainer_tower_audit", MODULE)
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


class TrainerTowerAuditTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = (GAME / "src/trainer_tower_sets.c").read_text(encoding="utf-8")

    def test_repository_course_is_deterministic_and_complete(self):
        first, second = AUDIT.build_report(GAME), AUDIT.build_report(GAME)
        self.assertEqual(first, second)
        self.assertTrue(first["invariants"]["passed"])
        self.assertEqual(first["local_header"], {"id": 1, "num_floors": 8, "sha256": AUDIT._sha({"id": 1, "num_floors": 8})})
        self.assertEqual(first["formats"]["MIXED"]["floor_symbols"], list(AUDIT.FORMAT_ROWS["MIXED"]))
        self.assertEqual(first["formats"]["SINGLE"]["prize"], "TTPRIZE_UP_GRADE")
        self.assertEqual(first["formats"]["DOUBLE"]["prize"], "TTPRIZE_DRAGON_SCALE")
        self.assertEqual(first["formats"]["KNOCKOUT"]["prize"], "TTPRIZE_METAL_COAT")
        self.assertEqual(first["formats"]["MIXED"]["prize"], "TTPRIZE_KINGS_ROCK")
        self.assertEqual(len(first["floors"]), 28)

    def test_rejects_wrong_mixed_row_external_dependency_and_invalid_opponent(self):
        changed = self.source.replace("&sTrainerTowerFloor_Knockout_2\n    }\n};", "&sTrainerTowerFloor_Knockout_3\n    }\n};", 1)
        with self.assertRaisesRegex(AUDIT.TowerAuditError, "floor table drifted"):
            AUDIT.build_report(GAME, changed)
        with self.assertRaisesRegex(AUDIT.TowerAuditError, "external or e-Reader"):
            AUDIT.build_report(GAME, '#include "ereader.h"\n' + self.source)
        changed = self.source.replace(".species = SPECIES_RATICATE", ".species = SPECIES_NOT_A_MON", 1)
        with self.assertRaisesRegex(AUDIT.TowerAuditError, "species is invalid"):
            AUDIT.build_report(GAME, changed)

    def test_rejects_actor_shape_speech_and_frozen_source_drift(self):
        changed = self.source.replace("DUMMY_TOWER_TEAM(0),\n        DUMMY_TOWER_TEAM(0),", "DUMMY_TOWER_TEAM(0),\n        { .name = _(\"BAD\") },", 1)
        with self.assertRaisesRegex(AUDIT.TowerAuditError, "fields differ"):
            AUDIT.build_report(GAME, changed)
        changed = self.source.replace("EC_WORD_AHAHA", "EC_WORD_NOT_A_WORD", 1)
        with self.assertRaisesRegex(AUDIT.TowerAuditError, "invalid Easy Chat"):
            AUDIT.build_report(GAME, changed)
        previous = AUDIT.FROZEN_LOCAL_PAYLOAD_SHA256
        try:
            AUDIT.FROZEN_LOCAL_PAYLOAD_SHA256 = "0" * 64
            with self.assertRaisesRegex(AUDIT.TowerAuditError, "source drift"):
                AUDIT.build_report(GAME)
        finally:
            AUDIT.FROZEN_LOCAL_PAYLOAD_SHA256 = previous
