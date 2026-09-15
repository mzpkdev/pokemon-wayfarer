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

    def test_repository_tower_report_is_deterministic_and_preserves_the_baseline(self):
        first = AUDIT.build_report(GAME)
        second = AUDIT.build_report(GAME)
        self.assertEqual(first, second)
        self.assertTrue(first["invariants"]["passed"])
        self.assertEqual(first["exploration_baseline"]["map_count"], 135)
        self.assertEqual(first["exploration_baseline"]["layout_count"], 102)
        self.assertEqual(first["exploration_baseline"]["raw_layout_bytes"], 134_612)
        self.assertEqual(first["schema"]["domains"]["exploration"]["enabled"], True)
        for domain in ("ordinary_trainers", "story"):
            self.assertFalse(first["schema"]["domains"][domain]["enabled"])
            self.assertEqual(first["schema"]["domains"][domain]["inventory_count"], 0)
        self.assertTrue(first["schema"]["domains"]["trainer_tower"]["enabled"])
        self.assertEqual(first["schema"]["domains"]["trainer_tower"]["inventory_count"], 106)
        self.assertFalse(first["rom"]["measured"])
        self.assertEqual(first["contracts"]["trainer_ids"]["allocation_count"], 0)
        self.assertEqual(
            [(row["id"], row["slot"], row["lifecycle"]) for row in first["contracts"]["states"]],
            [("SEVII_TRAINER_TOWER_PENDING_PRIZE", 3, "transactional")],
        )
        self.assertEqual(
            [(row["content_id"], row["kind"], row["pending_state"]) for row in first["contracts"]["transactions"]],
            [("trainer-tower.lobby.object.2", "claim", "SEVII_TRAINER_TOWER_PENDING_PRIZE")],
        )

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
