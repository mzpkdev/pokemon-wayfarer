import copy
import importlib.util
import json
from pathlib import Path
import unittest


GAME = Path(__file__).resolve().parents[3]
MODULE = GAME / "tools/wayfarer_sevii_content/contracts.py"
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_contracts", MODULE)
CONTRACTS = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CONTRACTS)


def namespace():
    return {
        "name": "wayfarer_sevii", "flag_base": 0xC000, "flag_capacity": 256,
        "var_base": 0xD000, "var_capacity": 32,
        "defeat_bitset": "wayfarer_sevii_trainer_defeat", "storage": "SaveBlock3",
    }


def empty_contracts():
    return {"schema_version": 1, "trainer_ids": {"base": 1515, "limit": 2048, "allocations": []},
            "state_namespace": namespace(), "states": [], "transactions": []}


def fixture():
    digest = CONTRACTS.source_party_hash(GAME, "TRAINER_BIKER_GOON")
    result = {
        "content_domains": [
            {"content_id": "story.biker_goon", "owner": "story", "state_reads": ["SEVII_BIKER_GOON_DEFEATED"]},
            {"content_id": "story.meteorite", "owner": "story", "state_writes": ["SEVII_METEORITE_RECEIVED"]},
        ],
        "contracts": empty_contracts(),
    }
    result["contracts"]["states"] = [
        {"id": "SEVII_BIKER_GOON_DEFEATED", "owner": "story", "storage": "trainer_defeat", "slot": 4,
         "initial": 0, "readers": ["story.biker_goon"], "writers": ["story.biker_goon"],
         "transitions": [{"from": 0, "to": 1, "caller": "story.biker_goon"}]},
        {"id": "SEVII_METEORITE_RECEIVED", "owner": "story", "storage": "flag", "slot": 0,
         "initial": 0, "readers": [], "writers": ["story.meteorite"],
         "transitions": [{"from": 0, "to": 1, "caller": "story.meteorite"}]},
    ]
    result["contracts"]["trainer_ids"]["allocations"] = [{
        "content_id": "story.biker_goon", "id": "TRAINER_WAYFARER_SEVII_BIKER_GOON", "slot": 4,
        "owner": "story", "source_trainer": "TRAINER_BIKER_GOON", "source_hash": digest,
        "classification": "story", "battle_policy": "objective_guard", "defeat_state": "SEVII_BIKER_GOON_DEFEATED",
        "battle_type": "single", "outcome_policy": "win_progress_loss_pending", "defeat_base": "story.biker_goon",
    }]
    result["contracts"]["transactions"] = [{
        "content_id": "story.meteorite", "owner": "story", "kind": "grant", "prerequisite": ["bill_offer"],
        "destination": "bag", "consume": None, "receipt": "SEVII_METEORITE_RECEIVED",
        "steps": ["establish_prerequisite", "attempt_destination", "set_receipt", "update_presentation"],
    }]
    return result


class SeviiContentContractTests(unittest.TestCase):
    def test_empty_milestone_contract_is_deterministic(self):
        manifest = {"contracts": empty_contracts()}
        first = CONTRACTS.validate_contracts(GAME, manifest)
        second = CONTRACTS.validate_contracts(GAME, manifest)
        self.assertEqual(json.dumps(first, sort_keys=True), json.dumps(second, sort_keys=True))
        self.assertEqual(first["trainer_ids"]["active_count"], 1515)
        self.assertEqual(first["trainer_ids"]["allocation_count"], 0)

    def test_selected_party_render_and_contract_map_fixed_slot(self):
        report = CONTRACTS.validate_contracts(GAME, fixture())
        allocation = report["trainer_ids"]["allocations"][0]
        self.assertEqual((allocation["slot"], allocation["numeric_id"]), (4, 1519))
        rendered = CONTRACTS.selected_trainer_render(GAME, fixture()["contracts"]["trainer_ids"]["allocations"])
        self.assertEqual(rendered[0]["party"]["partySize"], 2)
        self.assertEqual(rendered[0]["party"]["slots"][0]["species"], "SPECIES_KOFFING")
        self.assertIn(".trainerClass = TRAINER_CLASS_BIKER_FRLG", rendered[0]["compiled_record"])
        self.assertIn("- Haze", rendered[0]["source_block"])

    def test_selected_render_preserves_compiler_metadata_and_authored_item_ivs(self):
        allocation = fixture()["contracts"]["trainer_ids"]["allocations"][0]
        allocation.update({"source_trainer": "TRAINER_CRUSH_GIRL_TANYA",
                           "source_hash": CONTRACTS.source_party_hash(GAME, "TRAINER_CRUSH_GIRL_TANYA")})
        rendered = CONTRACTS.selected_trainer_render(GAME, [allocation])[0]
        self.assertEqual(rendered["party"]["trainerName"], '_("TANYA")')
        self.assertEqual(rendered["party"]["slots"][0]["heldItem"], "ITEM_BLACK_BELT")
        self.assertIn("TRAINER_PARTY_IVS(6, 6, 6, 6, 6, 6)", rendered["compiled_record"])
        self.assertIn("AI: Check Bad Move", rendered["source_block"])

    def test_rematch_maps_to_its_explicit_base_defeat_bit(self):
        manifest = fixture()
        manifest["content_domains"].append({"content_id": "story.biker_goon_rematch", "owner": "story"})
        manifest["contracts"]["trainer_ids"]["allocations"].append({
            "content_id": "story.biker_goon_rematch", "id": "TRAINER_WAYFARER_SEVII_BIKER_GOON_2", "slot": 5,
            "owner": "story", "source_trainer": "TRAINER_BIKER_GOON_2",
            "source_hash": CONTRACTS.source_party_hash(GAME, "TRAINER_BIKER_GOON_2"),
            "classification": "story", "battle_policy": "objective_guard",
            "battle_type": "single", "outcome_policy": "win_progress_loss_pending",
            "defeat_state": "SEVII_BIKER_GOON_DEFEATED", "defeat_base": "story.biker_goon",
        })
        report = CONTRACTS.validate_contracts(GAME, manifest)
        rematch = report["trainer_ids"]["allocations"][1]
        self.assertEqual((rematch["numeric_id"], rematch["defeat_base"]), (1520, "story.biker_goon"))

    def test_rejects_source_drift_and_content_aliases(self):
        manifest = fixture()
        manifest["contracts"]["trainer_ids"]["allocations"][0]["source_hash"] = "0" * 64
        with self.assertRaisesRegex(CONTRACTS.ContractError, "source drift"):
            CONTRACTS.validate_contracts(GAME, manifest)
        manifest = fixture()
        manifest["content_domains"].append({"content_id": "story.biker_goon", "owner": "story"})
        with self.assertRaisesRegex(CONTRACTS.ContractError, "duplicate content ownership"):
            CONTRACTS.validate_contracts(GAME, manifest)

    def test_rejects_unknown_state_write_missing_receipt_and_wrong_order(self):
        manifest = fixture()
        manifest["content_domains"][0]["state_writes"] = ["SEVII_UNKNOWN"]
        with self.assertRaisesRegex(CONTRACTS.ContractError, "unknown state"):
            CONTRACTS.validate_contracts(GAME, manifest)
        manifest = fixture()
        manifest["contracts"]["transactions"][0]["receipt"] = "SEVII_UNKNOWN"
        with self.assertRaisesRegex(CONTRACTS.ContractError, "invalid receipt"):
            CONTRACTS.validate_contracts(GAME, manifest)
        manifest = fixture()
        manifest["contracts"]["transactions"][0]["steps"][-2:] = ["update_presentation", "set_receipt"]
        with self.assertRaisesRegex(CONTRACTS.ContractError, "transaction ordering"):
            CONTRACTS.validate_contracts(GAME, manifest)

    def test_rejects_cross_owner_writes_and_defeat_alias(self):
        manifest = fixture()
        manifest["content_domains"].append({"content_id": "ordinary.trainer", "owner": "ordinary_trainer"})
        manifest["contracts"]["states"][0]["writers"] = ["ordinary.trainer"]
        manifest["contracts"]["states"][0]["transitions"][0]["caller"] = "ordinary.trainer"
        with self.assertRaisesRegex(CONTRACTS.ContractError, "owner namespace"):
            CONTRACTS.validate_contracts(GAME, manifest)
        manifest = fixture()
        duplicate = copy.deepcopy(manifest["contracts"]["states"][0])
        duplicate["id"] = "SEVII_OTHER_DEFEAT"
        manifest["contracts"]["states"].append(duplicate)
        with self.assertRaisesRegex(CONTRACTS.ContractError, "slot"):
            CONTRACTS.validate_contracts(GAME, manifest)

    def test_transactional_story_and_tower_payloads_can_clear_after_success(self):
        manifest = fixture()
        manifest["content_domains"] += [
            {"content_id": "story.selphy_request", "owner": "story"},
            {"content_id": "trainer_tower.prize_claim", "owner": "trainer_tower"},
        ]
        manifest["contracts"]["states"] += [
            {"id": "SEVII_SELPHY_PENDING_REWARD", "owner": "story", "storage": "var", "slot": 1, "initial": 0,
             "lifecycle": "transactional", "transaction_id": "story.selphy_request", "readers": ["story.selphy_request"], "writers": ["story.selphy_request"],
             "transitions": [{"from": 0, "to": 25, "caller": "story.selphy_request"}, {"from": 25, "to": 0, "caller": "story.selphy_request"}]},
            {"id": "SEVII_TOWER_PENDING_PRIZE", "owner": "trainer_tower", "storage": "var", "slot": 2, "initial": 0,
             "lifecycle": "transactional", "transaction_id": "trainer_tower.prize_claim", "readers": ["trainer_tower.prize_claim"], "writers": ["trainer_tower.prize_claim"],
             "transitions": [{"from": 0, "to": 50, "caller": "trainer_tower.prize_claim"}, {"from": 50, "to": 0, "caller": "trainer_tower.prize_claim"}]},
        ]
        manifest["contracts"]["transactions"] += [
            {"content_id": "story.selphy_request", "owner": "story", "kind": "claim", "prerequisite": ["request_active"], "destination": "bag", "consume": None,
             "receipt": None, "pending_state": "SEVII_SELPHY_PENDING_REWARD",
             "steps": ["establish_prerequisite", "attempt_destination", "clear_pending", "update_presentation"]},
            {"content_id": "trainer_tower.prize_claim", "owner": "trainer_tower", "kind": "claim", "prerequisite": ["challenge_complete"], "destination": "bag", "consume": None,
             "receipt": None, "pending_state": "SEVII_TOWER_PENDING_PRIZE",
             "steps": ["establish_prerequisite", "attempt_destination", "clear_pending", "update_presentation"]},
        ]
        report = CONTRACTS.validate_contracts(GAME, manifest)
        self.assertEqual({row["id"] for row in report["states"] if row["lifecycle"] == "transactional"}, {"SEVII_SELPHY_PENDING_REWARD", "SEVII_TOWER_PENDING_PRIZE"})

    def test_rejects_ordinary_transactions_completion_resets_and_unknown_battle_outcome(self):
        manifest = fixture()
        manifest["content_domains"].append({"content_id": "ordinary.trainer", "owner": "ordinary_trainer"})
        manifest["contracts"]["states"].append({"id": "SEVII_ORDINARY_RECEIPT", "owner": "ordinary_trainer", "storage": "flag", "slot": 5, "initial": 0,
            "readers": [], "writers": ["ordinary.trainer"], "transitions": [{"from": 0, "to": 1, "caller": "ordinary.trainer"}]})
        manifest["contracts"]["transactions"].append({"content_id": "ordinary.trainer", "owner": "ordinary_trainer", "kind": "grant", "prerequisite": ["battle"], "destination": "bag", "consume": None,
            "receipt": "SEVII_ORDINARY_RECEIPT", "steps": ["establish_prerequisite", "attempt_destination", "set_receipt", "update_presentation"]})
        with self.assertRaisesRegex(CONTRACTS.ContractError, "ordinary transaction"):
            CONTRACTS.validate_contracts(GAME, manifest)
        manifest = fixture()
        manifest["contracts"]["states"][1]["transitions"].append({"from": 1, "to": 0, "caller": "story.meteorite"})
        with self.assertRaisesRegex(CONTRACTS.ContractError, "completion transition"):
            CONTRACTS.validate_contracts(GAME, manifest)
        manifest = fixture()
        manifest["content_domains"][0]["battle_type"] = "double"
        manifest["content_domains"][0]["outcome_policy"] = "win_progress_loss_pending"
        with self.assertRaisesRegex(CONTRACTS.ContractError, "battle type or outcome"):
            CONTRACTS.validate_contracts(GAME, manifest)


if __name__ == "__main__":
    unittest.main()
