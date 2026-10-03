"""Runs the offline world-simulation report on the generated tables."""

import tempfile
import unittest
from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path

import report


def run(*args):
    with tempfile.TemporaryDirectory() as tmp:
        tsv = Path(tmp) / "itinerary.tsv"
        with redirect_stdout(StringIO()):
            status = report.main(["--itinerary", str(tsv)] + list(args))
        return status, tsv.read_text()


class ReportTest(unittest.TestCase):
    def test_all_badges_run_stays_valid_and_under_caps(self):
        status, itinerary = run("--heartbeats", "200", "--all-badges")
        self.assertEqual(status, 0)
        self.assertIn("\tdwelling\t", itinerary)

    def test_no_badges_keeps_leaders_home_locked(self):
        status, itinerary = run("--heartbeats", "50")
        self.assertEqual(status, 0)
        brock = [line for line in itinerary.splitlines() if "\tBROCK\t" in line]
        self.assertTrue(brock)
        self.assertTrue(all("\thome-locked\t" in line for line in brock))

    def test_player_path_and_league_resolution(self):
        status, itinerary = run("--heartbeats", "120", "--all-badges", "--wp", "90",
                                "--path", "VIRIDIAN_CITY_HNS,ROUTE2_HNS,PEWTER_CITY_HNS",
                                "--lineup", "WILL,LANCE,KAREN,WALLACE,STEVEN", "--resolve-at", "10")
        self.assertEqual(status, 0)
        self.assertIn("\taway: league\t", itinerary)
        self.assertIn("\tcelebrating\t", itinerary)

    def test_same_scenario_gives_identical_itineraries(self):
        args = ("--heartbeats", "150", "--all-badges", "--path", "CERULEAN_CITY_HNS,ROUTE24_HNS")
        self.assertEqual(run(*args), run(*args))


if __name__ == "__main__":
    unittest.main()
