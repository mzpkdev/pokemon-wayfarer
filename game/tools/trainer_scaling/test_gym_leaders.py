"""Structural tests for the authored Gym Leader roster source and report."""
from __future__ import annotations

import unittest

import gym_leaders as gym


class GymLeaderRosterTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.inventory = gym.generate(check=True)
        cls.rosters = cls.inventory["rosters"]
        cls.projections = cls.inventory["projections"]

    def test_exact_initial_identity_and_variant_coverage(self):
        self.assertEqual(self.inventory["encounterIdentities"], 24)
        self.assertEqual(self.inventory["resolvedVariants"], 30)
        self.assertEqual(len({row["identity"] for row in self.rosters}), 24)
        self.assertEqual(len({row["trainer"] for row in self.rosters}), 30)
        self.assertEqual({row["identity"] for row in self.rosters}, {
            "Brock", "Misty", "Lt. Surge", "Erika", "Janine", "Sabrina", "Blaine", "Blue",
            "Falkner", "Bugsy", "Whitney", "Morty", "Chuck", "Jasmine", "Pryce", "Clair",
            "Roxanne", "Brawly", "Wattson", "Flannery", "Norman", "Winona", "Tate/Liza", "Juan",
        })
        self.assertEqual({row["trainer"] for row in self.rosters if row["identity"] == "Chuck"}, {
            "TRAINER_CHUCK_1_HNS", "TRAINER_CHUCK_1_2_HNS", "TRAINER_CHUCK_1_3_HNS"})
        self.assertEqual({row["trainer"] for row in self.rosters if row["identity"] == "Jasmine"}, {
            "TRAINER_JASMINE_1_HNS", "TRAINER_JASMINE_1_2_HNS", "TRAINER_JASMINE_1_3_HNS"})
        self.assertEqual({row["trainer"] for row in self.rosters if row["identity"] == "Pryce"}, {
            "TRAINER_PRYCE_1_HNS", "TRAINER_PRYCE_1_2_HNS", "TRAINER_PRYCE_1_3_HNS"})

    def test_thresholds_anchors_and_interpolation_ties(self):
        self.assertEqual([gym.party_size(rating) for rating in (0, 7, 8, 21, 22, 33, 34, 39, 40, 80)],
                         [2, 2, 3, 3, 4, 4, 5, 5, 6, 6])
        self.assertEqual([gym.level(rating, 0) for rating, _ in gym.LEADER_ANCHORS],
                         [value for _, value in gym.LEADER_ANCHORS])
        # 4 -> 8 rises from 16 to 18: exact half at rating 6 rounds upward.
        self.assertEqual(gym.level(6, 0), 17)
        self.assertEqual(gym.level(0, -2), 13)
        self.assertEqual(gym.level(40, -1), 41)
        self.assertEqual(gym.level(80, 0), 100)
        for offset in (-2, -1, 0):
            levels = [gym.level(rating, offset) for rating in range(81)]
            self.assertEqual(levels, sorted(levels))

    def test_six_slots_policy_and_retention_order_are_explicit(self):
        for roster in self.rosters:
            self.assertEqual(len(roster["members"]), 6)
            self.assertEqual(sorted(member["battleOrder"] for member in roster["members"]), list(range(6)))
            self.assertTrue(roster["members"][0]["isAce"])
            for member in roster["members"]:
                self.assertEqual(member["movePolicy"], "AUTHORED")
                self.assertTrue(member["moves"])
                # The source compiler omits trailing MOVE_NONE entries. The
                # generated initializer keeps those positions zero-filled.
                self.assertLessEqual(len(member["moves"]), 4)
                if member["isAce"]:
                    self.assertEqual(member["levelOffset"], 0)
                else:
                    self.assertIn(member["levelOffset"], (-1, -2))
            if roster["isDoubleBattle"]:
                self.assertEqual([member["species"] for member in roster["members"][:2]],
                                 ["SPECIES_LUNATONE", "SPECIES_SOLROCK"])
                self.assertEqual([member["battleOrder"] for member in roster["members"][:2]], [0, 1])
                self.assertTrue(all(member["isAce"] for member in roster["members"][:2]))
            else:
                ace_orders = [member["battleOrder"] for member in roster["members"] if member["isAce"]]
                support_orders = [member["battleOrder"] for member in roster["members"] if not member["isAce"]]
                self.assertGreater(min(ace_orders), max(support_orders))

    def test_every_integer_rating_keeps_prefix_then_orders_output(self):
        roster_by_id = {row["trainer"]: row for row in self.rosters}
        for projection in self.projections:
            roster = roster_by_id[projection["trainer"]]
            retained = roster["members"][:projection["count"]]
            self.assertEqual(projection["selectedSourceIndices"], list(range(projection["count"])))
            self.assertEqual(projection["outputSourceIndices"],
                             [member["sourceIndex"] for member in sorted(retained, key=lambda member: member["battleOrder"])])
            self.assertEqual(len(projection["members"]), projection["count"])
            self.assertTrue(any(member["isAce"] for member in projection["members"]))
        tate_early = next(row for row in self.projections if row["trainer"] == "TRAINER_TATE_AND_LIZA_1" and row["rating"] == 0)
        self.assertEqual(tate_early["outputSourceIndices"], [0, 1])
        self.assertEqual([member["species"] for member in tate_early["members"]], ["SPECIES_LUNATONE", "SPECIES_SOLROCK"])

    def test_active_janine_blue_and_excluded_aliases_are_reported(self):
        by_identity = {row["identity"]: row for row in self.rosters}
        self.assertEqual(by_identity["Janine"]["trainer"], "TRAINER_JANINE_HNS")
        self.assertEqual(by_identity["Janine"]["members"][0]["species"], "SPECIES_CROBAT")
        self.assertEqual(by_identity["Blue"]["trainer"], "TRAINER_BLUE_HNS")
        self.assertEqual(by_identity["Blue"]["members"][0]["species"], "SPECIES_PIDGEOT")
        self.assertEqual({alias for row in self.rosters for alias in row["aliases"] if alias.endswith("_DOJO_HNS")}, {
            "TRAINER_BROCK_DOJO_HNS", "TRAINER_MISTY_DOJO_HNS", "TRAINER_LTSURGE_DOJO_HNS",
            "TRAINER_ERIKA_DOJO_HNS", "TRAINER_JANINE_DOJO_HNS", "TRAINER_SABRINA_DOJO_HNS",
            "TRAINER_BLAINE_DOJO_HNS", "TRAINER_BLUE_DOJO_HNS"})


if __name__ == "__main__":
    unittest.main()
