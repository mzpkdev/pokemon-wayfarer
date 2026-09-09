import json
from pathlib import Path
import re
import unittest


GAME_ROOT = Path(__file__).resolve().parents[2]
ENVIRONMENT = GAME_ROOT / "data/scripts/wayfarer_sevii/environment.inc"


class WayfarerSeviiEnvironmentTest(unittest.TestCase):
    def test_environment_wrappers_are_source_free(self):
        text = ENVIRONMENT.read_text()
        for map_name in (
            "FourIsland_IcefallCave_1F_Frlg",
            "SixIsland_RuinValley_Frlg",
            "FiveIsland_Meadow_Frlg",
            "FiveIsland_RocketWarehouse_Frlg",
            "SevenIsland_SevaultCanyon_TanobyKey_Frlg",
        ):
            self.assertIn(f"{map_name}_MapScripts::", text)
        self.assertIn("FLAG_WAYFARER_SEVII_TANOBY_COMPLETE", text)
        self.assertIn("FLAG_WAYFARER_SEVII_DOTTED_HOLE_OPEN", text)
        self.assertIn("VAR_WAYFARER_SEVII_ICEFALL_FALL", text)
        self.assertNotIn("map_script_2 VAR_TEMP_1", text)
        for forbidden in (
            "FLAG_USED_CUT_ON_RUIN_VALLEY_BRAILLE",
            "FLAG_SYS_UNLOCKED_TANOBY_RUINS",
            "FLAG_UNLOCKED_ROCKET_WAREHOUSE",
            "FLAG_DEFEATED_ROCKETS_IN_WAREHOUSE",
            "FLAG_RECOVERED_SAPPHIRE",
            "trainerbattle",
            "giveitem",
        ):
            self.assertNotIn(forbidden, text)

    def test_lost_cave_every_room_can_recover_to_entrance(self):
        maps_root = GAME_ROOT / "data/maps"
        entrance = "FiveIsland_LostCave_Entrance_Frlg"
        graph = {}
        for number in range(1, 15):
            name = f"FiveIsland_LostCave_Room{number}_Frlg"
            data = json.loads((maps_root / name / "map.json").read_text())
            graph[name] = {
                self._map_name_from_id(warp["dest_map"])
                for warp in data["warp_events"]
            }

        for start in graph:
            seen = {start}
            pending = [start]
            while pending:
                current = pending.pop()
                for destination in graph.get(current, set()):
                    if destination not in seen:
                        seen.add(destination)
                        pending.append(destination)
            self.assertIn(entrance, seen, start)

    @staticmethod
    def _map_name_from_id(map_id):
        # The imported map IDs are mechanically derived from their source
        # directories. Resolve through map.json instead of assuming a spelling
        # transform for numeric room names.
        for path in (GAME_ROOT / "data/maps").glob("FiveIsland_LostCave_*_Frlg/map.json"):
            data = json.loads(path.read_text())
            name = data["name"]
            snake = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", name).upper()
            if f"MAP_{snake.removesuffix('_FRLG')}" == map_id:
                return data["name"]
        raise AssertionError(f"unknown Lost Cave target {map_id}")


if __name__ == "__main__":
    unittest.main()
