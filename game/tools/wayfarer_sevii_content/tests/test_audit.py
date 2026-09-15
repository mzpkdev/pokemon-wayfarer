import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock


GAME = Path(__file__).resolve().parents[3]
TOOL = GAME / "tools/wayfarer_sevii_content/audit.py"
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_content_audit", TOOL)
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


class WayfarerSeviiContentAuditTests(unittest.TestCase):
    def manifest(self):
        return json.loads((GAME / "src/data/wayfarer_sevii_maps.json").read_text(encoding="utf-8"))

    def test_repository_story_report_is_deterministic_and_preserves_the_baseline(self):
        first = AUDIT.build_report(GAME)
        second = AUDIT.build_report(GAME)
        self.assertEqual(first, second)
        self.assertTrue(first["invariants"]["passed"])
        self.assertEqual(first["exploration_baseline"]["map_count"], 135)
        self.assertEqual(first["exploration_baseline"]["layout_count"], 102)
        self.assertEqual(first["exploration_baseline"]["raw_layout_bytes"], 134_612)
        self.assertEqual(first["schema"]["domains"]["exploration"]["enabled"], True)
        self.assertTrue(first["schema"]["domains"]["story"]["enabled"])
        self.assertGreater(first["schema"]["domains"]["story"]["inventory_count"], 0)
        self.assertTrue(any(entry["owner"] == "story" for entry in first["contract_closure"]["entries"]))
        for domain in ("ordinary_trainers", "trainer_tower"):
            self.assertFalse(first["schema"]["domains"][domain]["enabled"])
            self.assertEqual(first["schema"]["domains"][domain]["inventory_count"], 0)
        self.assertFalse(first["rom"]["measured"])

    def test_rejects_manifest_attempt_to_redefine_the_accepted_projection(self):
        manifest = self.manifest()
        manifest["baseline"]["projection_sha256"] = "0" * 64
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "manifest.json"
            path.write_text(json.dumps(manifest), encoding="utf-8")
            with self.assertRaisesRegex(AUDIT.AuditError, "manifest baseline differs"):
                AUDIT.build_report(GAME, path)

    def test_rom_report_requires_the_active_reserve_when_measurement_is_supplied(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "release-size.json"
            end = AUDIT.WAYFARER_RELEASE_LIMIT
            path.write_text(json.dumps({"rom": {
                "end_address": f"0x{end:08X}", "used_bytes": end - AUDIT.ROM_START,
            }}), encoding="utf-8")
            report = AUDIT.rom_report(path, GAME)
            self.assertTrue(report["measured"])
            self.assertEqual(report["headroom_above_required_reserve_bytes"], 0)
            path.write_text(json.dumps({"rom": {
                "end_address": f"0x{end + 1:08X}", "used_bytes": end + 1 - AUDIT.ROM_START,
            }}), encoding="utf-8")
            with self.assertRaisesRegex(AUDIT.AuditError, "512 KiB reserve"):
                AUDIT.rom_report(path, GAME)

    def test_projected_event_identity_includes_only_reviewed_script_and_override(self):
        source = {"type": "object", "script": "SourceScript", "flag": "0", "x": 4}
        row = {
            "content_id": "exploration.test.event", "source": source,
            "wayfarer_script": "WayfarerSevii_Test", "overrides": {"flag": "FLAG_WAYFARER_SEVII_TEST"},
        }
        self.assertEqual(AUDIT._output_event(row), {
            "type": "object", "script": "WayfarerSevii_Test", "flag": "FLAG_WAYFARER_SEVII_TEST", "x": 4,
        })

    def test_contract_closure_fallback_ignores_blank_and_comment_only_lines(self):
        row = {
            "content_id": "story.test.fallback", "owner": "story",
            "wayfarer_script": "WayfarerSevii_Fallback", "state_reads": [],
            "state_writes": ["SEVII_FALLBACK_RECEIPT"],
        }
        contracts = {
            "states": [{"id": "SEVII_FALLBACK_RECEIPT", "symbol": "FLAG_WAYFARER_SEVII_FALLBACK_RECEIPT", "storage": "flag"}],
            "trainer_ids": {"allocations": []}, "transactions": [],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/fallback.inc"
            include.parent.mkdir(parents=True)
            include.write_text(
                "WayfarerSevii_Fallback::\n"
                "\t@ This line is intentionally comment-only.\n"
                "\n"
                "\tsetflag FLAG_WAYFARER_SEVII_FALLBACK_RECEIPT\n"
                "\tend\n",
                encoding="utf-8",
            )
            closure = {"includes": ["data/scripts/wayfarer_sevii/story/fallback.inc"],
                       "state_operations": [], "content_operations": []}
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                report = AUDIT.validate_contract_closure(root, {}, closure, contracts)
            self.assertEqual(report["entries"][0]["state_writes"], ["SEVII_FALLBACK_RECEIPT"])

    def test_contract_closure_counts_object_visibility_override_as_a_state_read(self):
        row = {
            "content_id": "story.test.visible_actor", "owner": "story",
            "wayfarer_script": "WayfarerSevii_VisibleActor",
            "source": {"type": "object", "flag": "FLAG_HIDE_SOURCE_ACTOR"},
            "overrides": {"flag": "FLAG_WAYFARER_SEVII_VISIBLE_ACTOR"},
            "state_reads": ["SEVII_VISIBLE_ACTOR"], "state_writes": [],
        }
        contracts = {
            "states": [{"id": "SEVII_VISIBLE_ACTOR", "symbol": "FLAG_WAYFARER_SEVII_VISIBLE_ACTOR",
                        "storage": "flag"}],
            "trainer_ids": {"allocations": []}, "transactions": [],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/visible_actor.inc"
            include.parent.mkdir(parents=True)
            include.write_text("WayfarerSevii_VisibleActor::\n\tend\n", encoding="utf-8")
            closure = {"includes": ["data/scripts/wayfarer_sevii/story/visible_actor.inc"],
                       "state_operations": [], "content_operations": []}
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                report = AUDIT.validate_contract_closure(root, {}, closure, contracts)
            self.assertEqual(report["entries"][0]["state_reads"], ["SEVII_VISIBLE_ACTOR"])

    def test_contract_closure_binds_a_story_battle_state_and_receipt_to_owned_commands(self):
        row = {
            "content_id": "story.test.battle", "owner": "story", "wayfarer_script": "WayfarerSevii_StoryBattle",
            "state_reads": ["SEVII_STORY_READY"], "state_writes": ["SEVII_STORY_RECEIVED"],
            "battle_type": "single", "outcome_policy": "win_progress_loss_pending",
        }
        states = [
            {"id": "SEVII_STORY_READY", "symbol": "FLAG_WAYFARER_SEVII_STORY_READY"},
            {"id": "SEVII_STORY_RECEIVED", "symbol": "FLAG_WAYFARER_SEVII_STORY_RECEIVED"},
        ]
        contracts = {
            "states": states,
            "trainer_ids": {"allocations": [{
                "content_id": row["content_id"], "id": "TRAINER_WAYFARER_SEVII_TEST",
                "battle_type": "single", "outcome_policy": "win_progress_loss_pending",
            }]},
            "transactions": [{"content_id": row["content_id"], "kind": "grant", "receipt": "SEVII_STORY_RECEIVED"}],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/test.inc"
            include.parent.mkdir(parents=True)
            include.write_text(
                "WayfarerSevii_StoryBattle::\n"
                "\tcheckflag FLAG_WAYFARER_SEVII_STORY_READY\n"
                "\ttrainerbattle TRAINER_BATTLE_SINGLE, TRAINER_WAYFARER_SEVII_TEST\n"
                "\tgiveitem ITEM_POTION\n"
                "\tsetflag FLAG_WAYFARER_SEVII_STORY_RECEIVED\n"
                "\tend\n",
                encoding="utf-8",
            )
            closure = {
                "includes": ["data/scripts/wayfarer_sevii/story/test.inc"],
                "state_operations": [
                    {"label": "WayfarerSevii_StoryBattle", "access": "read", "state": "FLAG_WAYFARER_SEVII_STORY_READY"},
                    {"label": "WayfarerSevii_StoryBattle", "access": "write", "state": "FLAG_WAYFARER_SEVII_STORY_RECEIVED"},
                ],
                "content_operations": [
                    {"label": "WayfarerSevii_StoryBattle", "kind": "battle", "command": "trainerbattle_single"},
                    {"label": "WayfarerSevii_StoryBattle", "kind": "transaction", "command": "giveitem"},
                ],
            }
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                report = AUDIT.validate_contract_closure(root, {}, closure, contracts)
            self.assertEqual(report["entries"][0]["trainer_allocation"], "TRAINER_WAYFARER_SEVII_TEST")
            self.assertEqual(report["entries"][0]["transaction"], "grant")
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                contracts["trainer_ids"]["allocations"] = []
                with self.assertRaisesRegex(AUDIT.AuditError, "unowned Trainer battle caller"):
                    AUDIT.validate_contract_closure(root, {}, closure, contracts)

            contracts["trainer_ids"]["allocations"] = [{
                "content_id": row["content_id"], "id": "TRAINER_WAYFARER_SEVII_TEST",
                "battle_type": "single", "outcome_policy": "win_progress_loss_pending",
            }]
            closure["state_operations"].append({
                "label": "WayfarerSevii_StoryBattle", "access": "read", "state": "FLAG_WAYFARER_SEVII_UNDECLARED",
            })
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                with self.assertRaisesRegex(AUDIT.AuditError, "reads undeclared state"):
                    AUDIT.validate_contract_closure(root, {}, closure, contracts)

    def test_contract_closure_accepts_a_repeatable_claim_pending_clear(self):
        row = {
            "content_id": "story.test.claim", "owner": "story", "wayfarer_script": "WayfarerSevii_Claim",
            "state_reads": [], "state_writes": ["SEVII_CLAIM_PENDING"],
        }
        contracts = {
            "states": [{"id": "SEVII_CLAIM_PENDING", "symbol": "VAR_WAYFARER_SEVII_CLAIM_PENDING", "storage": "var"}],
            "trainer_ids": {"allocations": []},
            "transactions": [{"content_id": row["content_id"], "kind": "claim", "receipt": None,
                              "pending_state": "SEVII_CLAIM_PENDING"}],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/claim.inc"
            include.parent.mkdir(parents=True)
            include.write_text("WayfarerSevii_Claim::\n\tend\n", encoding="utf-8")
            closure = {
                "includes": ["data/scripts/wayfarer_sevii/story/claim.inc"],
                "state_operations": [{"label": "WayfarerSevii_Claim", "access": "write",
                                      "state": "VAR_WAYFARER_SEVII_CLAIM_PENDING"}],
                "content_operations": [{"label": "WayfarerSevii_Claim", "kind": "transaction",
                                        "command": "giveitem"}],
            }
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                report = AUDIT.validate_contract_closure(root, {}, closure, contracts)
            self.assertEqual(report["entries"][0]["pending_state"], "SEVII_CLAIM_PENDING")

    def test_contract_closure_accepts_atomic_special_receipt_without_script_item_or_flag_markers(self):
        row = {
            "content_id": "story.test.atomic_grant", "owner": "story",
            "wayfarer_script": "WayfarerSevii_AtomicGrant",
            "state_reads": ["SEVII_ATOMIC_RECEIPT"],
            "state_writes": ["SEVII_ATOMIC_RECEIPT"],
        }
        contracts = {
            "states": [{"id": "SEVII_ATOMIC_RECEIPT", "symbol": "FLAG_WAYFARER_SEVII_ATOMIC_RECEIPT", "storage": "flag"}],
            "trainer_ids": {"allocations": []},
            "transactions": [{"content_id": row["content_id"], "kind": "grant", "receipt": "SEVII_ATOMIC_RECEIPT"}],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/atomic.inc"
            include.parent.mkdir(parents=True)
            include.write_text(
                "WayfarerSevii_AtomicGrant::\n"
                "\tsetvar VAR_0x8004, ITEM_POTION\n"
                "\tsetvar VAR_0x8005, FLAG_WAYFARER_SEVII_ATOMIC_RECEIPT\n"
                "\tspecialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag\n"
                "\tend\n",
                encoding="utf-8",
            )
            closure = {
                "includes": ["data/scripts/wayfarer_sevii/story/atomic.inc"],
                "state_operations": [
                    {"label": "WayfarerSevii_AtomicGrant", "access": access,
                     "state": "FLAG_WAYFARER_SEVII_ATOMIC_RECEIPT"}
                    for access in ("read", "write")
                ],
                "content_operations": [{
                    "label": "WayfarerSevii_AtomicGrant", "kind": "transaction",
                    "command": "WayfarerSevii_TryGiveItemThenSetFlag",
                }],
            }
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                report = AUDIT.validate_contract_closure(root, {}, closure, contracts)
            self.assertEqual(report["entries"][0]["transaction_commands"], ["WayfarerSevii_TryGiveItemThenSetFlag"])
            contracts["transactions"][0]["kind"] = "handoff"
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                with self.assertRaisesRegex(AUDIT.AuditError, "does not match declared transaction kind"):
                    AUDIT.validate_contract_closure(root, {}, closure, contracts)

    def test_contract_closure_accepts_only_a_delivery_gated_handoff_retry_grant(self):
        row = {
            "content_id": "story.test.handoff_retry", "owner": "story",
            "wayfarer_script": "WayfarerSevii_Father",
            "state_reads": ["SEVII_DELIVERED", "SEVII_REWARD_RECEIVED"],
            "state_writes": ["SEVII_DELIVERED", "SEVII_REWARD_RECEIVED"],
        }
        contracts = {
            "states": [
                {"id": "SEVII_DELIVERED", "symbol": "FLAG_WAYFARER_SEVII_DELIVERED", "storage": "flag"},
                {"id": "SEVII_REWARD_RECEIVED", "symbol": "FLAG_WAYFARER_SEVII_REWARD_RECEIVED", "storage": "flag"},
            ],
            "trainer_ids": {"allocations": []},
            "transactions": [{"content_id": row["content_id"], "kind": "handoff",
                              "receipt": "SEVII_REWARD_RECEIVED"}],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/handoff.inc"
            include.parent.mkdir(parents=True)
            include.write_text(
                "WayfarerSevii_Father::\n"
                "\tgoto_if_set FLAG_WAYFARER_SEVII_DELIVERED, WayfarerSevii_FatherRetry\n"
                "\tsetvar VAR_0x8004, ITEM_METEORITE\n"
                "\tsetvar VAR_0x8005, ITEM_MOON_STONE\n"
                "\tsetvar VAR_0x8006, FLAG_WAYFARER_SEVII_DELIVERED\n"
                "\tsetvar VAR_0x8007, FLAG_WAYFARER_SEVII_REWARD_RECEIVED\n"
                "\tspecialvar VAR_RESULT, WayfarerSevii_TryExchangeItemForRewardThenSetFlags\n"
                "\tend\n"
                "WayfarerSevii_FatherRetry::\n"
                "\tgoto_if_set FLAG_WAYFARER_SEVII_REWARD_RECEIVED, WayfarerSevii_FatherComplete\n"
                "\tsetvar VAR_0x8004, ITEM_MOON_STONE\n"
                "\tsetvar VAR_0x8005, FLAG_WAYFARER_SEVII_REWARD_RECEIVED\n"
                "\tspecialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag\n"
                "\tend\n"
                "WayfarerSevii_FatherComplete::\n\tend\n",
                encoding="utf-8",
            )
            closure = {
                "includes": ["data/scripts/wayfarer_sevii/story/handoff.inc"],
                "state_operations": [
                    {"label": "WayfarerSevii_Father", "access": access, "state": state}
                    for state in ("FLAG_WAYFARER_SEVII_DELIVERED", "FLAG_WAYFARER_SEVII_REWARD_RECEIVED")
                    for access in ("read", "write")
                ],
                "content_operations": [
                    {"label": "WayfarerSevii_Father", "kind": "transaction",
                     "command": "WayfarerSevii_TryExchangeItemForRewardThenSetFlags"},
                    {"label": "WayfarerSevii_FatherRetry", "kind": "transaction",
                     "command": "WayfarerSevii_TryGiveItemThenSetFlag"},
                ],
            }
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                AUDIT.validate_contract_closure(root, {}, closure, contracts)
            include.write_text(include.read_text(encoding="utf-8").replace(
                "\tgoto_if_set FLAG_WAYFARER_SEVII_DELIVERED, WayfarerSevii_FatherRetry\n",
                "\tgoto WayfarerSevii_FatherRetry\n"), encoding="utf-8")
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                with self.assertRaisesRegex(AUDIT.AuditError, "does not match declared transaction kind"):
                    AUDIT.validate_contract_closure(root, {}, closure, contracts)

    def test_contract_closure_requires_move_maniac_payment_after_successful_service(self):
        row = {
            "content_id": "story.test.move_maniac", "owner": "story",
            "wayfarer_script": "WayfarerSevii_MoveManiac", "state_reads": [], "state_writes": [],
        }
        contracts = {
            "states": [], "trainer_ids": {"allocations": []},
            "transactions": [{
                "content_id": row["content_id"], "kind": "service", "receipt": None,
                "consume": "mushrooms", "destination": "service",
                "steps": ["establish_prerequisite", "successful_service", "consume_source", "update_presentation"],
            }],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/move_maniac.inc"
            include.parent.mkdir(parents=True)
            include.write_text(
                "WayfarerSevii_MoveManiac::\n"
                "\tgoto WayfarerSevii_MoveManiacTeach\n"
                "WayfarerSevii_MoveManiacTeach::\n"
                "\tspecial TeachMoveRelearnerMove\n"
                "\twaitstate\n"
                "\tgoto_if_eq VAR_0x8004, 0, WayfarerSevii_MoveManiacEnd\n"
                "\tgoto WayfarerSevii_MoveManiacPay\n"
                "WayfarerSevii_MoveManiacPay::\n"
                "\tremoveitem ITEM_BIG_MUSHROOM\n"
                "WayfarerSevii_MoveManiacEnd::\n"
                "\tend\n",
                encoding="utf-8",
            )
            closure = {
                "includes": ["data/scripts/wayfarer_sevii/story/move_maniac.inc"],
                "state_operations": [],
                "content_operations": [{
                    "label": "WayfarerSevii_MoveManiacPay", "kind": "transaction", "command": "removeitem",
                }],
            }
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                report = AUDIT.validate_contract_closure(root, {}, closure, contracts)
            self.assertEqual(report["entries"][0]["transaction"], "service")

            include.write_text(include.read_text(encoding="utf-8").replace(
                "\tgoto WayfarerSevii_MoveManiacTeach\n", "\tgoto WayfarerSevii_MoveManiacPay\n"), encoding="utf-8")
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                with self.assertRaisesRegex(AUDIT.AuditError, "payment can precede successful service"):
                    AUDIT.validate_contract_closure(root, {}, closure, contracts)

    def test_contract_closure_requires_atomic_party_only_egg_grant(self):
        row = {
            "content_id": "story.test.egg", "owner": "story", "wayfarer_script": "WayfarerSevii_Egg",
            "state_reads": [], "state_writes": ["SEVII_EGG_RECEIVED"],
        }
        contracts = {
            "states": [{"id": "SEVII_EGG_RECEIVED", "symbol": "FLAG_WAYFARER_SEVII_EGG_RECEIVED", "storage": "flag"}],
            "trainer_ids": {"allocations": []},
            "transactions": [{"content_id": row["content_id"], "kind": "grant", "receipt": "SEVII_EGG_RECEIVED"}],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/egg.inc"
            include.parent.mkdir(parents=True)
            include.write_text(
                "WayfarerSevii_Egg::\n"
                "\tgetpartysize\n"
                "\tgoto_if_eq VAR_RESULT, PARTY_SIZE, WayfarerSevii_EggFull\n"
                "\tgiveegg SPECIES_TOGEPI\n"
                "\tsetflag FLAG_WAYFARER_SEVII_EGG_RECEIVED\n"
                "WayfarerSevii_EggFull::\n\tend\n",
                encoding="utf-8",
            )
            closure = {
                "includes": ["data/scripts/wayfarer_sevii/story/egg.inc"],
                "state_operations": [{"label": "WayfarerSevii_Egg", "access": "write",
                                      "state": "FLAG_WAYFARER_SEVII_EGG_RECEIVED"}],
                "content_operations": [{"label": "WayfarerSevii_Egg", "kind": "transaction", "command": "giveegg"}],
            }
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                with self.assertRaisesRegex(AUDIT.AuditError, "must use the atomic"):
                    AUDIT.validate_contract_closure(root, {}, closure, contracts)
            include.write_text(
                "WayfarerSevii_Egg::\n"
                "\tsetvar VAR_0x8004, SPECIES_TOGEPI\n"
                "\tsetvar VAR_0x8005, FLAG_WAYFARER_SEVII_EGG_RECEIVED\n"
                "\tspecialvar VAR_RESULT, WayfarerSevii_TryGiveEggThenSetFlag\n"
                "\tend\n",
                encoding="utf-8",
            )
            closure["content_operations"] = [{
                "label": "WayfarerSevii_Egg", "kind": "transaction",
                "command": "WayfarerSevii_TryGiveEggThenSetFlag",
            }]
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                AUDIT.validate_contract_closure(root, {}, closure, contracts)

    def test_contract_closure_requires_staged_grant_source_commit_before_reward(self):
        row = {
            "content_id": "story.test.tectonix", "owner": "story", "wayfarer_script": "WayfarerSevii_Tectonix",
            "state_reads": [], "state_writes": ["SEVII_OFFERING_COMPLETE", "SEVII_REWARD_RECEIVED"],
        }
        contracts = {
            "states": [
                {"id": "SEVII_OFFERING_COMPLETE", "symbol": "FLAG_WAYFARER_SEVII_OFFERING_COMPLETE", "storage": "flag"},
                {"id": "SEVII_REWARD_RECEIVED", "symbol": "FLAG_WAYFARER_SEVII_REWARD_RECEIVED", "storage": "flag"},
            ],
            "trainer_ids": {"allocations": []},
            "transactions": [{
                "content_id": row["content_id"], "kind": "staged_grant", "receipt": "SEVII_REWARD_RECEIVED",
                "source_receipt": "SEVII_OFFERING_COMPLETE", "consume": "ITEM_LEMONADE", "destination": "bag",
                "steps": ["establish_prerequisite", "consume_source", "set_source_receipt", "attempt_destination", "set_receipt", "update_presentation"],
            }],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/story/tectonix.inc"
            include.parent.mkdir(parents=True)
            include.write_text(
                "WayfarerSevii_Tectonix::\n"
                "\tgoto_if_set FLAG_WAYFARER_SEVII_OFFERING_COMPLETE, WayfarerSevii_TectonixReward\n"
                "\tremoveitem ITEM_LEMONADE\n"
                "\tsetflag FLAG_WAYFARER_SEVII_OFFERING_COMPLETE\n"
                "\tgoto WayfarerSevii_TectonixReward\n"
                "WayfarerSevii_TectonixReward::\n"
                "\tgiveitem ITEM_TM42\n"
                "\tgoto_if_eq VAR_RESULT, FALSE, WayfarerSevii_TectonixFull\n"
                "\tsetflag FLAG_WAYFARER_SEVII_REWARD_RECEIVED\n"
                "WayfarerSevii_TectonixFull::\n\tend\n",
                encoding="utf-8",
            )
            closure = {
                "includes": ["data/scripts/wayfarer_sevii/story/tectonix.inc"],
                "state_operations": [
                    {"label": "WayfarerSevii_Tectonix", "access": "write", "state": "FLAG_WAYFARER_SEVII_OFFERING_COMPLETE"},
                    {"label": "WayfarerSevii_TectonixReward", "access": "write", "state": "FLAG_WAYFARER_SEVII_REWARD_RECEIVED"},
                ],
                "content_operations": [
                    {"label": "WayfarerSevii_Tectonix", "kind": "transaction", "command": "removeitem"},
                    {"label": "WayfarerSevii_TectonixReward", "kind": "transaction", "command": "giveitem"},
                ],
            }
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                AUDIT.validate_contract_closure(root, {}, closure, contracts)
            include.write_text(include.read_text(encoding="utf-8").replace(
                "\tsetflag FLAG_WAYFARER_SEVII_OFFERING_COMPLETE\n\tgoto WayfarerSevii_TectonixReward\n",
                "\tgoto WayfarerSevii_TectonixReward\n\tsetflag FLAG_WAYFARER_SEVII_OFFERING_COMPLETE\n"), encoding="utf-8")
            with mock.patch.object(AUDIT, "selected_records", return_value=[row]):
                with self.assertRaisesRegex(AUDIT.AuditError, "must consume and commit its source before rewarding"):
                    AUDIT.validate_contract_closure(root, {}, closure, contracts)

    def test_real_closure_and_contracts_accept_an_ordinary_single_battle_wrapper(self):
        """Exercise the production closure and Trainer contract, not mocked operations."""
        content_id = "ordinary.test.biker_goon"
        defeat = "SEVII_ORDINARY_BIKER_GOON_DEFEATED"
        manifest = {
            "content_domains": {
                "exploration": {"owner": "exploration", "enabled": True, "inventory": []},
                "ordinary_trainers": {"owner": "ordinary_trainer", "enabled": True, "inventory": [content_id]},
                "story": {"owner": "story", "enabled": False, "inventory": []},
                "trainer_tower": {"owner": "trainer_tower", "enabled": False, "inventory": []},
            },
            "maps": [{
                "source_map": "TestMap", "map_id": "MAP_TEST", "retained_map_scripts": [],
                "retained_events": {"object_events": [{
                    "index": 0, "source": {}, "wayfarer_script": "WayfarerSevii_OrdinaryBiker",
                    "owner": "ordinary_trainer", "content_id": content_id, "reason": "test wrapper",
                    "overrides": {}, "battle_type": "single", "outcome_policy": "defeat_and_blackout",
                }], "coord_events": [], "bg_events": []},
            }],
            "script_modules": {"ordinary_test": {
                "owner": "ordinary_trainer", "include": "data/scripts/wayfarer_sevii/trainers/test.inc",
                "exports": ["WayfarerSevii_OrdinaryBiker", "WayfarerSevii_OrdinaryBikerIntro",
                            "WayfarerSevii_OrdinaryBikerLose"], "allowed_externals": [],
                "allowed_commands": ["trainerbattle_single", "end"],
            }},
            "contracts": {
                "schema_version": 1,
                "trainer_ids": {"base": 1515, "limit": 2048, "allocations": [{
                    "content_id": content_id, "id": "TRAINER_WAYFARER_SEVII_TEST_BIKER", "slot": 0,
                    "owner": "ordinary_trainer", "source_trainer": "TRAINER_BIKER_GOON",
                    "source_hash": AUDIT.contracts.source_party_hash(GAME, "TRAINER_BIKER_GOON"),
                    "classification": "ordinary", "battle_policy": "ordinary", "battle_type": "single",
                    "outcome_policy": "defeat_and_blackout", "defeat_state": defeat, "defeat_base": content_id,
                }]},
                "state_namespace": {
                    "name": "wayfarer_sevii", "flag_base": 0xC000, "flag_capacity": 256,
                    "var_base": 0xD000, "var_capacity": 32,
                    "defeat_bitset": "wayfarer_sevii_trainer_defeat", "storage": "SaveBlock3",
                },
                "states": [{
                    "id": defeat, "owner": "ordinary_trainer", "storage": "trainer_defeat", "slot": 0,
                    "initial": 0, "readers": [], "writers": [content_id],
                    "transitions": [{"from": 0, "to": 1, "caller": content_id}],
                }],
                "transactions": [],
            },
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            include = root / "data/scripts/wayfarer_sevii/trainers/test.inc"
            include.parent.mkdir(parents=True)
            include.write_text(
                "WayfarerSevii_OrdinaryBiker::\n"
                "\ttrainerbattle_single TRAINER_WAYFARER_SEVII_TEST_BIKER, WayfarerSevii_OrdinaryBikerIntro, WayfarerSevii_OrdinaryBikerLose\n"
                "\tend\n"
                "WayfarerSevii_OrdinaryBikerIntro::\n"
                "\tend\n"
                "WayfarerSevii_OrdinaryBikerLose::\n"
                "\tend\n", encoding="utf-8",
            )
            closure = AUDIT.closure.build_script_closure(root, manifest)
            contracts = AUDIT.contracts.validate_contracts(GAME, manifest)
            report = AUDIT.validate_contract_closure(root, manifest, closure, contracts)
            self.assertEqual(report["entries"][0]["battle_commands"], ["single"])
            manifest["maps"][0]["retained_events"]["object_events"][0]["battle_type"] = "double"
            with self.assertRaisesRegex(AUDIT.AuditError, "battle_type"):
                AUDIT.validate_contract_closure(root, manifest, closure, contracts)


if __name__ == "__main__":
    unittest.main()
