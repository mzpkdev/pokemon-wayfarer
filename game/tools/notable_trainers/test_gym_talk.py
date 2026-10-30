"""Every Gym Leader's own Gym greets by the leader's friendship once nothing is owed (see include/notable_talk.h)."""

import json
import re
import unittest
from pathlib import Path

GAME = Path(__file__).resolve().parents[2]
ROUTINES = GAME / "tools/wayfarer_world/routines.json"
MAPS = GAME / "data/maps"
# Wayfarer runs this Gym's script from a separate file instead of the map's scripts.inc.
SCRIPT_FILES = {"CinnabarIsland_Gym_Frlg": MAPS / "wayfarer_cinnabar_full_scripts.inc"}
# Giovanni leaves for good with the TM: his object is removed, so no post-battle line is left to wrap.
NO_POST_BATTLE_LINE = {"NOTABLE_TRAINER_GIOVANNI"}
# Leaders outside the simulated roster whose Gym still greets (one id for the pair; both objects share a script).
EXTRA = {"MossdeepCity_Gym": "NOTABLE_TRAINER_TATE_LIZA"}
GREETING = re.compile(
    r"setvar VAR_0x8004, (NOTABLE_TRAINER_\w+)\n\tcall NotableTalk_EventScript_Greeting\n(?:#endif\n)?"
    r"\tmsgbox [^\n]+\n(?:#if IS_WAYFARER\n)?\tcall NotableTalk_EventScript_Farewell\n"
)


def gyms():
    for trainer in json.loads(ROUTINES.read_text())["trainers"]:
        if trainer.get("gym") and trainer["character"] not in NO_POST_BATTLE_LINE:
            yield trainer["gym"], trainer["character"]
    yield from EXTRA.items()


class GymTalkTests(unittest.TestCase):
    def test_each_leader_gym_greets_with_its_own_id(self):
        count = 0
        for gym, character in gyms():
            path = SCRIPT_FILES.get(gym, MAPS / gym / "scripts.inc")
            ids = set(GREETING.findall(path.read_text()))
            self.assertEqual(ids, {character}, f"{gym}: greeting ids {ids}, expected {character}")
            count += 1
        self.assertEqual(count, 23)


if __name__ == "__main__":
    unittest.main()
