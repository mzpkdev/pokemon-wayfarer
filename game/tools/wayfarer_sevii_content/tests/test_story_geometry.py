"""Story actors never seal a route the player must walk, checked against layout collision.

The script-level suites cannot see a boxed-in trainer or an actor standing on the
only tile in front of a cave, so these tests load the real layout collision and
the actors the manifest retains, then walk the map.
"""
from collections import deque
import json
from pathlib import Path
import re
import struct
import unittest

GAME = Path(__file__).resolve().parents[3]
MANIFEST = GAME / "src/data/wayfarer_sevii_maps.json"
SCRIPT_HELPERS = {"EventScript_RockSmash", "EventScript_CutTree", "EventScript_StrengthBoulder"}
STORY_SCRIPTS = GAME / "data/scripts/wayfarer_sevii/story/celio_network.inc"


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


class Stage:
    """One map with the actors that are visible for a given story stage."""

    def __init__(self, manifest, source_map, cleared_flags=(), set_flags=()):
        record = next(row for row in manifest["maps"] if row["source_map"] == source_map)
        self.map_json = load(GAME / "data/maps" / source_map / "map.json")
        layouts = load(GAME / "data/layouts/layouts.json")["layouts"]
        layout = next(row for row in layouts if row["id"] == self.map_json["layout"])
        self.width, self.height = layout["width"], layout["height"]
        blocks = (GAME / layout["blockdata_filepath"]).read_bytes()
        self.blocked = set()
        for y in range(self.height):
            for x in range(self.width):
                (value,) = struct.unpack_from("<H", blocks, (y * self.width + x) * 2)
                if (value >> 10) & 3:
                    self.blocked.add((x, y))
        initial = {}
        for state in manifest["contracts"]["states"]:
            if state["storage"] == "flag":
                initial["FLAG_WAYFARER_" + state["id"]] = state["initial"]
        self.actors = {}
        for row in record["retained_events"].get("object_events", []):
            source = row["source"]
            if row.get("wayfarer_script") in SCRIPT_HELPERS:
                continue
            flag = row.get("overrides", {}).get("flag", source.get("flag", "0"))
            hidden = flag not in ("0", None) and initial.get(flag, 0) == 1
            if flag in cleared_flags:
                hidden = False
            if flag in set_flags:
                hidden = True
            if not hidden:
                self.actors[row["content_id"]] = (source["x"], source["y"])

    def reachable(self, starts, extra_blocked=()):
        blocked = self.blocked | set(self.actors.values()) | set(extra_blocked)
        seen = {start for start in starts if start not in blocked}
        queue = deque(seen)
        while queue:
            x, y = queue.popleft()
            for step in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                tile = (x + step[0], y + step[1])
                if (0 <= tile[0] < self.width and 0 <= tile[1] < self.height
                        and tile not in blocked and tile not in seen):
                    seen.add(tile)
                    queue.append(tile)
        return seen

    def south_edge(self):
        return [(x, self.height - 1) for x in range(self.width) if (x, self.height - 1) not in self.blocked]

    def can_stand_next_to(self, content_id, reachable):
        x, y = self.actors[content_id]
        return any((x + dx, y + dy) in reachable for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))


class ThreeIslandBlockadeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = load(MANIFEST)

    def test_island_is_passable_before_lostelle_starts(self):
        stage = Stage(self.manifest, "ThreeIsland_Frlg")
        reachable = stage.reachable(stage.south_edge())
        for warp in stage.map_json["warp_events"]:
            door = (warp["x"], warp["y"])
            self.assertTrue(
                any((door[0] + dx, door[1] + dy) in reachable for dx, dy in ((0, 1), (1, 0), (-1, 0), (0, -1))),
                f"{door} unreachable from the Port before the quest starts")
        self.assertNotIn("story.lostelle.biker-goon-1", stage.actors)

    def test_every_required_biker_is_reachable_from_the_port_in_order(self):
        stage = Stage(self.manifest, "ThreeIsland_Frlg",
                      cleared_flags={"FLAG_WAYFARER_SEVII_HIDE_THREE_ISLAND_BIKERS"})
        reachable = stage.reachable(stage.south_edge())
        for content in ("story.lostelle.biker-goon-1", "story.lostelle.biker-goon-2",
                        "story.lostelle.biker-goon-3", "story.lostelle.cue-ball-paxton"):
            self.assertIn(content, stage.actors)
            self.assertTrue(stage.can_stand_next_to(content, reachable),
                            f"{content} is boxed in: the player cannot stand beside it from the Port")

    def test_defender_is_reachable_once_the_bikers_leave(self):
        stage = Stage(self.manifest, "ThreeIsland_Frlg")
        reachable = stage.reachable(stage.south_edge())
        self.assertTrue(stage.can_stand_next_to("story.lostelle.three-island-defender", reachable))


class MtEmberRubyPathTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = load(MANIFEST)
        cls.script = STORY_SCRIPTS.read_text(encoding="utf-8")

    def cave_approach(self, stage):
        warp = next(row for row in stage.map_json["warp_events"] if row["dest_map"] == "MAP_MT_EMBER_RUBY_PATH_1F")
        return (warp["x"], warp["y"] + 1)

    def test_cave_entrance_reachable_while_guards_stand_post(self):
        stage = Stage(self.manifest, "MtEmber_Exterior_Frlg",
                      cleared_flags={"FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS"})
        self.assertIn("story.celio.ruby-guard-43", stage.actors)
        self.assertIn("story.celio.ruby-guard-44", stage.actors)
        start = [(row["x"], row["y"] - 1) for row in stage.map_json["warp_events"]
                 if row["dest_map"] == "MAP_ONE_ISLAND_KINDLE_ROAD"]
        reachable = stage.reachable(start)
        self.assertIn(self.cave_approach(stage), reachable)
        for guard in ("story.celio.ruby-guard-43", "story.celio.ruby-guard-44"):
            self.assertTrue(stage.can_stand_next_to(guard, reachable), f"{guard} cannot be challenged")

    def test_guards_hide_for_good_once_both_are_beaten(self):
        body = re.search(r"(?ms)^WayfarerSevii_RubyLead_DismissGuards:\n(.*?)^WayfarerSevii_RubyLead_EndDismissGuards:",
                         self.script)
        self.assertIsNotNone(body)
        for guard in ("GRUNT_43", "GRUNT_44"):
            self.assertIn(f"goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_{guard}", body.group(1))
        self.assertIn("setflag FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS", body.group(1))
        self.assertIn("removeobject LOCALID_MT_EMBER_GRUNT1", body.group(1))
        self.assertIn("removeobject LOCALID_MT_EMBER_GRUNT2", body.group(1))
        for number in ("1", "2"):
            script = re.search(rf"(?ms)^WayfarerSevii_EventScript_RubyGuard{number}::\n(.*?)^\tend", self.script).group(1)
            self.assertIn("call WayfarerSevii_RubyLead_DismissGuards", script)

    def test_entrance_never_waits_on_story_state(self):
        onload = re.search(r"(?ms)^WayfarerSevii_MtEmberExterior_OnLoad::\n(.*?)^\tend", self.script).group(1)
        self.assertIn("setmetatile 42, 39, METATILE_MtEmber_CaveEntrance", onload)
        self.assertNotIn("goto_if", onload)


class WarehouseSightTrainerTests(unittest.TestCase):
    """Sight trainers must reach `trainerbattle` before any effectful command.

    CheckTrainer runs a sight trainer's script until its first effect. When that
    effect is not `trainerbattle` (for example `lock`), a beaten Rocket has no
    defeat check and walks up to the player on every step in view.
    """

    def test_combatant_scripts_open_with_trainerbattle(self):
        script = STORY_SCRIPTS.read_text(encoding="utf-8")
        for name in ("Grunt42", "Admin1", "Grunt47", "Grunt48", "Admin2"):
            body = re.search(rf"(?ms)^WayfarerSevii_EventScript_Warehouse{name}::\n(.*?)^\tend", script).group(1)
            first = body.strip().splitlines()[0].split()[0]
            self.assertEqual(first, "trainerbattle_single", f"{name} must begin with trainerbattle_single")

    def test_each_opponent_appears_only_after_the_previous_one_falls(self):
        manifest = load(MANIFEST)
        record = next(row for row in manifest["maps"] if row["source_map"] == "FiveIsland_RocketWarehouse_Frlg")
        flags = {row["content_id"]: row.get("overrides", {}).get("flag")
                 for row in record["retained_events"]["object_events"] if "overrides" in row}
        chain = ["grunt42", "admin1", "grunt47", "grunt48", "admin2"]
        hide = [flags["story.celio.warehouse-warehouse" + name] for name in chain]
        self.assertEqual(len(set(hide)), 5, "every opponent needs its own presentation flag")
        script = STORY_SCRIPTS.read_text(encoding="utf-8")
        for name, nxt in zip(("Grunt42", "Admin1", "Grunt47", "Grunt48"), ("Admin1", "Grunt47", "Grunt48", "Admin2")):
            body = re.search(rf"(?ms)^WayfarerSevii_EventScript_Warehouse{name}::\n(.*?)^\tend", script).group(1)
            self.assertIn(f"call WayfarerSevii_Warehouse_Reveal{nxt}", body)


class ArrivalWarpTests(unittest.TestCase):
    def test_no_story_actor_stands_on_a_warp_tile(self):
        manifest = load(MANIFEST)
        for record in manifest["maps"]:
            map_json = load(GAME / "data/maps" / record["source_map"] / "map.json")
            warps = {(row["x"], row["y"]) for row in map_json["warp_events"]}
            for row in record["retained_events"].get("object_events", []):
                if row["owner"] != "story":
                    continue
                source = row["source"]
                self.assertNotIn((source["x"], source["y"]), warps,
                                 f"{row['content_id']} stands on a warp tile in {record['source_map']}")


if __name__ == "__main__":
    unittest.main()
