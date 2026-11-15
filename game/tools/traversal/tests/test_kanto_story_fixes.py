"""Source-level regressions for the Kanto story-beat audit fixes (K1, K2, K3/K4, K6, K8).

Like the open-world traversal checks, these inspect event scripts and map JSON rather than
running a ROM: they pin the control flow that the audit found broken.
"""

from __future__ import annotations

import re
import unittest

from test_hns_open_world_traversal import DATA, MAPS, ScriptIndex, load_map, object_with_local_id, read

GAME = DATA.parent
GUARD_FLAGS = ("FLAG_INDIGOJUNCTION_HIDE_SILVER_GUARD", "FLAG_INDIGOJUNCTION_HIDE_KANTO_GUARD")


class KantoStoryFixesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.scripts = ScriptIndex()

    def test_k1_nothing_resurrects_a_released_reception_gate_guard(self) -> None:
        """Only releases set a guard flag; no script may clear one (that would bring a guard back)."""
        offenders = []
        for path in DATA.rglob("*.inc"):
            for number, line in enumerate(read(path).splitlines(), 1):
                code = line.split("@", 1)[0]
                if re.search(r"\bclearflag\s+(%s)\b" % "|".join(GUARD_FLAGS), code):
                    offenders.append(f"{path.relative_to(GAME)}:{number}")
        self.assertEqual(offenders, [], "a released guard must stay released")
        # The Route 27 first-steps scene still runs, and still starts the route state.
        first_steps = self.scripts.block("Route27_EventScript_FirstSteps")
        self.assertIn("setvar VAR_ROUTE27_STATE, 1", first_steps)

    def test_k2_pallet_native_oak_reaches_the_shared_badge_check(self) -> None:
        reachable = self.scripts.reachable_labels("PalletTown_Lab_EventScript_KantoComplete")
        self.assertIn("PalletTown_Lab_EventScript_Oak16Badges", reachable)
        complete = self.scripts.block("PalletTown_Lab_EventScript_KantoComplete")
        self.assertIn("WayfarerGetKantoBadgeCountForScript", complete)
        self.assertRegex(complete, r"goto_if_ge VAR_RESULT, 8, ")
        # The shared authority is the one that opens the guard.
        self.assertIn("setflag FLAG_INDIGOJUNCTION_HIDE_SILVER_GUARD", self.scripts.block("PalletTown_Lab_EventScript_Oak16Badges"))

    def test_k3_k4_flute_item_and_radio_resolve_the_same_snorlax(self) -> None:
        snorlax = self.scripts.block("VermilionCity_EventScript_Snorlax")
        self.assertIn("CheckRadioStation", snorlax)
        self.assertIn("checkitem ITEM_POKE_FLUTE", snorlax)
        for entry in (
            "VermilionCity_EventScript_SnorlaxWakeUp",
            "VermilionCity_EventScript_SnorlaxFlute",
            "VermilionCity_EventScript_SnorlaxOfferFlute",
        ):
            reachable = self.scripts.reachable_labels(entry)
            self.assertIn("VermilionCity_EventScript_SnorlaxBattle", reachable, entry)
        battle = self.scripts.block("VermilionCity_EventScript_SnorlaxBattle")
        self.assertIn("setflag FLAG_INDIGOJUNCTION_HIDE_KANTO_GUARD", battle)
        self.assertIn("setflag FLAG_HIDE_VERMILION_SNORLAX", battle)
        sleeping = read(MAPS / "VermilionCity_hns/scripts.inc")
        self.assertRegex(sleeping, r"VermilionCity_Text_SnorlaxSleeping:\n\t\.string[^$]*music", "the examine text must hint at music")

    def test_k4_flute_item_use_beside_the_snorlax_runs_the_wake_script(self) -> None:
        item_use = uncomment_c(read(GAME / "src/item_use.c"))
        self.assertIn("VermilionCity_EventScript_SnorlaxFlute", item_use)
        self.assertIn("FLAG_HIDE_VERMILION_SNORLAX", item_use)
        self.assertIn("MAP_VERMILION_CITY_HNS", item_use)

    def test_k6_viridian_giovanni_is_staged_by_the_finale_prerequisites(self) -> None:
        gym = load_map("ViridianCity_Gym_Frlg")
        giovanni = object_with_local_id(gym, "LOCALID_VIRIDIAN_GIOVANNI")
        self.assertEqual(giovanni["wayfarer_flag"], "FLAG_TEMP_2")
        stage = self.scripts.block("ViridianCity_Gym_EventScript_StageGiovanni")
        for needle in (
            "setflag FLAG_TEMP_2",
            "TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS",
            "FLAG_CELADON_ROCKET_HIDEOUT_SILPH_SCOPE_RECEIVED_HNS",
            "FLAG_SILPH_LIBERATED_HNS",
            "FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS",
        ):
            self.assertIn(needle, stage)
        self.assertIn("clearflag FLAG_TEMP_2", stage)

    def test_k8_misty_date_and_route14_run_the_same_suicune_check(self) -> None:
        route25 = read(MAPS / "Route25_hns/scripts.inc")
        # The block that ends the date sets Cerulean state 3 and must then run the Suicune check.
        self.assertRegex(
            route25,
            r"setvar VAR_CERULEAN_CITY_STATE, 3\n(?:\t@[^\n]*\n)?\tcall_if_eq VAR_SUICUNE_ENCOUNTERS, 5, CeruleanCity_Gym_SetSuicuneScene",
        )
        route14 = read(MAPS / "Route14_hns/scripts.inc")
        self.assertIn("call_if_ge VAR_CERULEAN_CITY_STATE, 3, CeruleanCity_Gym_SetSuicuneScene", route14)


def uncomment_c(text: str) -> str:
    return re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.S)


if __name__ == "__main__":
    unittest.main()
