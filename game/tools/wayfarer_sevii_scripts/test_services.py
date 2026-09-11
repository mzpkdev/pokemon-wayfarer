from pathlib import Path
import unittest


SERVICES = Path(__file__).parents[2] / "data/scripts/wayfarer_sevii/services.inc"


class WayfarerSeviiServicesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.text = SERVICES.read_text()

    def test_defines_only_wayfarer_owned_service_labels(self):
        expected = {
            "WayfarerSevii_EventScript_Nurse::",
            "WayfarerSevii_EventScript_PC::",
            *[f"WayfarerSevii_EventScript_{island}IslandMart::" for island in (
                "Three", "Four", "Six", "Seven")],
            "WayfarerSevii_EventScript_FourIslandDaycareWoman::",
            *[f"WayfarerSevii_EventScript_{island}IslandHarborSailor::" for island in (
                "One", "Two", "Three", "Four", "Five", "Six", "Seven")],
        }
        for label in expected:
            self.assertIn(label, self.text)
        self.assertNotIn("_Frlg_EventScript_", self.text)

    def test_nurse_does_not_dispatch_frlg_center_story_or_multiplayer(self):
        prohibited = (
            "EventScript_PkmnCenterNurse_Frlg",
            "BufferUnionRoomPlayerName",
            "FLAG_IS_CHAMPION",
            "FLAG_SEVII_",
            "VAR_MAP_SCENE_",
        )
        for symbol in prohibited:
            self.assertNotIn(symbol, self.text)

    def test_daycare_omits_egg_delivery_and_frlg_campaign_state(self):
        prohibited = (
            "GiveEggFromDaycare",
            "RejectEggFromDayCare",
            "FLAG_",
            "VAR_MAP_SCENE_",
            "FourIsland_PokemonDayCare_Frlg",
        )
        for symbol in prohibited:
            self.assertNotIn(symbol, self.text)

    def test_each_harbor_selects_its_origin_then_uses_full_menu(self):
        for island in ("ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN"):
            self.assertIn(f"setvar VAR_0x8004, SEAGALLOP_{island}_ISLAND", self.text)
        self.assertEqual(self.text.count("goto WayfarerSevii_EventScript_HarborDestinationMenu"), 7)

    def test_ferry_wrapper_cannot_link_frlg_seagallop_story_labels(self):
        self.assertIn("WayfarerSevii_EventScript_HarborDestinationMenu::", self.text)
        self.assertIn("WayfarerSevii_EventScript_SailToSelectedDestination::", self.text)
        for source_label in (
            "EventScript_SeviiDestinationsPage1",
            "EventScript_SeviiDestinationsPage2",
            "EventScript_SailToDest",
            "EventScript_CheckFullSeagallopService",
        ):
            self.assertNotIn(source_label, self.text)


if __name__ == "__main__":
    unittest.main()
