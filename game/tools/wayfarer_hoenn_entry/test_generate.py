from __future__ import annotations

import importlib.util
import json
import struct
import tempfile
import unittest
from unittest.mock import patch
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
    def assert_script_mutation_rejected(self, filename, old, new, audit, message):
        original_read = AUDIT.read_text
        target = AUDIT.GAME_ROOT / filename
        self.assertIn(old, original_read(target))

        def mutated_read(path):
            source = original_read(path)
            return source.replace(old, new, 1) if path == target else source

        with patch.object(AUDIT, "read_text", side_effect=mutated_read):
            with self.assertRaisesRegex(AUDIT.AuditError, message):
                audit(AUDIT.GAME_ROOT)

    def test_vermilion_cannot_reintroduce_the_regular_aqua_gate(self):
        self.assert_script_mutation_rejected(
            "data/maps/VermilionCity_PortInside_hns/scripts.inc",
            "\tsetvar VAR_0x8004, FERRY_BERTH_DOCK\n\tcall Ferry_EventScript_SailorTalk",
            "\tspecialvar VAR_RESULT, WayfarerCanUseRegularAqua\n\tcall Ferry_EventScript_SailorTalk",
            AUDIT.audit_menus_and_routes, "regular-Aqua gate")

    def test_vermilion_ship_row_must_use_the_ferry_boarding(self):
        self.assert_script_mutation_rejected(
            "data/maps/VermilionCity_PortInside_hns/scripts.inc",
            "\tcall Ferry_EventScript_SailorTalk\n\tgoto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_SailorEnd\n\tcall VermilionPort_EventScript_FerryBoardingWalk",
            "\tcall VermilionPort_EventScript_FerryBoardingWalk",
            AUDIT.audit_menus_and_routes, "ferry specials")

    def test_ferry_boarding_walk_does_not_set_the_respawn(self):
        self.assert_script_mutation_rejected(
            "data/maps/OlivineCity_PortInside_hns/scripts.inc",
            "\tplayse SE_EXIT\n\tdelay 20\n\treturn\n#else",
            "\tsetrespawn HEAL_LOCATION_OLIVINE_CITY_HNS\n\tplayse SE_EXIT\n\tdelay 20\n\treturn\n#else",
            AUDIT.audit_menus_and_routes, "respawn")

    def test_olivine_maiden_voyage_stays_a_johto_only_special_case(self):
        self.assert_script_mutation_rejected(
            "data/maps/OlivineCity_PortInside_hns/scripts.inc",
            "goto_if_eq VAR_RESULT, FALSE, OlivinePort_EventScript_Sailor_Ferry\n\tgoto_if_ge VAR_SSAQUA_STATE, 8",
            "goto_if_eq VAR_RESULT, TRUE, OlivinePort_EventScript_Sailor_Ferry\n\tgoto_if_ge VAR_SSAQUA_STATE, 8",
            AUDIT.audit_menus_and_routes, "maiden voyage")

    def test_birth_eligibility_is_the_aurora_ticket_alone(self):
        self.assert_script_mutation_rejected(
            "src/wayfarer_sevii_ferry.c",
            "return CheckBagHasItem(ITEM_AURORA_TICKET, 1);",
            "return FlagGet(FLAG_SYS_GAME_CLEAR) && CheckBagHasItem(ITEM_AURORA_TICKET, 1);",
            AUDIT.audit_menus_and_routes, "Birth Island eligibility")

    def test_navel_eligibility_does_not_use_the_unavailable_ship_flag(self):
        self.assert_script_mutation_rejected(
            "src/wayfarer_sevii_ferry.c",
            "FlagGet(FLAG_SYS_GAME_CLEAR) && CheckBagHasItem(ITEM_MYSTIC_TICKET, 1)",
            "FlagGet(FLAG_ENABLE_SHIP_NAVEL_ROCK) && CheckBagHasItem(ITEM_MYSTIC_TICKET, 1)",
            AUDIT.audit_menus_and_routes, "unavailable ship flag")

    def test_numbered_island_harbors_cannot_reintroduce_story_gates(self):
        self.assert_script_mutation_rejected(
            "data/scripts/seagallop.inc",
            "#if IS_WAYFARER\n\tgoto EventScript_SeviiDestinationsPage1",
            "#if IS_WAYFARER\n\tgoto_if_unset FLAG_SEVII_TRAVEL_INTRO_SEEN, EventScript_CancelSail",
            AUDIT.audit_menus_and_routes, "complete ungated")

    def test_free_ticket_is_profile_owned(self):
        self.assert_script_mutation_rejected(
            "data/maps/SlateportCity_Harbor/scripts.inc",
            "specialvar VAR_RESULT, WayfarerCanReceiveSlateportTicket",
            "specialvar VAR_RESULT, WayfarerCanUseAquaMaidenVoyage",
            AUDIT.audit_hoenn_ports_and_ticket, "profile handoff policy")

    def test_free_ticket_failure_cannot_board(self):
        self.assert_script_mutation_rejected(
            "data/maps/SlateportCity_Harbor/scripts.inc",
            "goto_if_eq VAR_RESULT, FALSE, SlateportCity_Harbor_EventScript_TicketFailed",
            "goto_if_eq VAR_RESULT, FALSE, SlateportCity_Harbor_EventScript_FerryTalk",
            AUDIT.audit_hoenn_ports_and_ticket, "failed Ticket grant")

    def test_existing_ticket_skips_free_grant(self):
        self.assert_script_mutation_rejected(
            "data/maps/SlateportCity_Harbor/scripts.inc",
            "goto_if_eq VAR_RESULT, TRUE, SlateportCity_Harbor_EventScript_FerryTalk",
            "goto_if_eq VAR_RESULT, FALSE, SlateportCity_Harbor_EventScript_FerryTalk",
            AUDIT.audit_hoenn_ports_and_ticket, "skip the free grant")

    def test_slateport_berth_cannot_regain_the_game_clear_gate(self):
        self.assert_script_mutation_rejected(
            "data/maps/SlateportCity_Harbor/scripts.inc",
            "SlateportCity_Harbor_EventScript_FerryTalk::\n\tsetvar VAR_0x8004, FERRY_BERTH_DOCK",
            "SlateportCity_Harbor_EventScript_FerryTalk::\n\tgoto_if_set FLAG_SYS_GAME_CLEAR, SlateportCity_Harbor_EventScript_AskForTicket\n\tsetvar VAR_0x8004, FERRY_BERTH_DOCK",
            AUDIT.audit_hoenn_ports_and_ticket, "game-clear gate")

    def test_lilycove_cannot_regain_ship_enable_flags(self):
        self.assert_script_mutation_rejected(
            "data/maps/LilycoveCity_Harbor/scripts.inc",
            "LilycoveCity_Harbor_EventScript_FerryBoard::\n\tsetvar VAR_0x8004, FERRY_BERTH_DOCK",
            "LilycoveCity_Harbor_EventScript_FerryBoard::\n\tgoto_if_unset FLAG_ENABLE_SHIP_SOUTHERN_ISLAND, Ferry_EventScript_SailorEnd\n\tsetvar VAR_0x8004, FERRY_BERTH_DOCK",
            AUDIT.audit_hoenn_ports_and_ticket, "FLAG_ENABLE_SHIP")

    def test_state_initialization_cannot_set_the_region_or_respawn(self):
        self.assert_script_mutation_rejected(
            "src/wayfarer_persistence.c",
            "    WayfarerSetHoennStateInitialized(TRUE);\n}",
            "    SetLastHealLocationWarp(HEAL_LOCATION_SLATEPORT_CITY);\n    WayfarerSetHoennStateInitialized(TRUE);\n}",
            AUDIT.audit_initialization, "SetLastHealLocationWarp")

    def test_repository_hoenn_entry_contract(self) -> None:
        result = AUDIT.build_audit(AUDIT.GAME_ROOT)
        self.assertEqual(result["status"], "pass")
        self.assertEqual(result["travel"]["boarding"]["specials"], ["Ferry_GetBerthShip", "Ferry_Board", "Ferry_WarpAboard"])
        self.assertFalse(result["travel"]["boarding"]["ticketConsumed"])
        self.assertTrue(result["initialization"]["initializedCommittedLast"])
        self.assertEqual(result["initialization"]["stateInitializationFunction"], "WayfarerInitializeHoennState")
        self.assertTrue(result["hoennPorts"]["slateportBerth"]["retiredAquaAttendant"])
        self.assertEqual(result["hoennPorts"]["slateportBerth"]["coordinate"], [8, 10])
        self.assertFalse(result["hoennPorts"]["ssTidal"]["gameClearGate"])
        self.assertEqual(result["hoennPorts"]["lilycoveBerth"]["charters"], ["SOUTHERN", "FARAWAY", "FRONTIER"])


if __name__ == "__main__":
    unittest.main()
