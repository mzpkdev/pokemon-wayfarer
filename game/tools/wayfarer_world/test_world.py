"""Tests for the walker graph and spot table generator.

The map tests read the Wayfarer build's mapjson outputs; they skip when the
worktree's generated map files are for another map version.
"""

import copy
import sys
import unittest
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))

import authored  # noqa: E402
import build  # noqa: E402
import emit  # noqa: E402
import generate  # noqa: E402
import graph  # noqa: E402
import maps  # noqa: E402
import routines  # noqa: E402
import spots as sp  # noqa: E402
from maps import BuildError  # noqa: E402

ROOT = maps.DEFAULT_ROOT
HAVE_WAYFARER_MAPS = ((ROOT / ".map_version.wayfarer").exists()
                      and (ROOT / "data/maps/groups.inc").exists())
NEEDS_MAPS = unittest.skipUnless(HAVE_WAYFARER_MAPS,
                                 "needs the Wayfarer build's mapjson outputs")
_FULL = {}


def full():
    """One full build, shared by the tests that read it."""
    if not _FULL:
        world = maps.World(ROOT)
        world.build_scope()
        wg = build.WorldGraph(world)
        wg.check()
        rt = routines.Routines(wg)
        rt.check()
        _FULL.update(world=world, wg=wg, rt=rt)
    return _FULL["world"], _FULL["wg"], _FULL["rt"]


def spots_on(wg, name, kind):
    return [(s.x, s.y, s.facing) for s in wg.spots if s.map.name == name and s.kind == kind]


# --------------------------------------------------------------------------
# Synthetic grids: the collision and elevation rules.
# --------------------------------------------------------------------------

class FakeGrid:
    def __init__(self, rows, consts, elev=None, behaviours=None):
        self.h, self.w = len(rows), len(rows[0])
        self.family = "hns"
        self.col, self.mb, self.elev = [], [], []
        behaviours = behaviours or {}
        for y, row in enumerate(rows):
            for x, ch in enumerate(row):
                self.col.append(1 if ch == "#" else 0)
                self.mb.append(consts.mb[behaviours.get(ch, "MB_NORMAL")])
                self.elev.append(int(elev[y][x], 16) if elev else 3)

    def inside(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h

    def b(self, x, y):
        return self.mb[y * self.w + x]


class FakeInfo:
    name = "Synthetic"
    events = {"warps": [], "objects": []}


class FakeWorld:
    def __init__(self):
        self.root = ROOT
        self.consts = maps.Constants(ROOT)


class CollisionRules(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.world = FakeWorld()
        cls.walk = graph.Walk(cls.world)
        cls.consts = cls.world.consts

    def flood(self, rows, elev=None, behaviours=None, solid=()):
        grid = FakeGrid(rows, self.consts, elev, behaviours)
        return graph.MapFlood(FakeInfo(), grid, {y * grid.w + x for x, y in solid}, self.walk)

    def test_open_room_is_one_node(self):
        f = self.flood(["....", "....", "...."])
        self.assertEqual(len(f.components), 1)

    def test_collision_wall_splits(self):
        f = self.flood(["..#..", "..#..", "..#.."])
        self.assertEqual(len(f.components), 2)

    def test_elevation_mismatch_splits_and_zero_joins(self):
        rows = ["......"]
        self.assertEqual(len(self.flood(rows, elev=["333444"]).components), 2)
        # Elevation 0 is a transition: 3 -> 0 -> 4 is allowed both ways.
        self.assertEqual(len(self.flood(rows, elev=["333044"]).components), 1)

    def test_bridge_keeps_elevation(self):
        # A row of elevation-15 tiles crosses a column at 4; a walker at 3
        # on the bridge never meets the 4s below it and the 4s never meet 3.
        rows = [".....", ".....", "....."]
        elev = ["44444", "3FFF3", "44444"]
        f = self.flood(rows, elev=elev)
        left = f.tile_component(0, 1)
        right = f.tile_component(4, 1)
        self.assertEqual(left, right)
        self.assertNotEqual(left, f.tile_component(0, 0))
        # The bridge tile holds two states: elevation 3 and elevation 4.
        self.assertEqual(sorted(e for e, _ in f.states_of(2, 1)), [3, 4])

    def test_one_sided_wall_blocks_both_ways(self):
        # MB_IMPASSABLE_SOUTH on the top tile cuts its south edge.
        rows = [".S.", "..."]
        f = self.flood(["S", "."], behaviours={"S": "MB_IMPASSABLE_SOUTH"})
        self.assertEqual(len(f.components), 2)
        f = self.flood(rows, behaviours={"S": "MB_IMPASSABLE_SOUTH"})
        self.assertEqual(len(f.components), 1)  # around the side

    def test_ledges_water_and_objects_are_solid(self):
        self.assertEqual(len(self.flood([".v."], behaviours={"v": "MB_JUMP_SOUTH"}).components), 2)
        self.assertEqual(len(self.flood([".~."], behaviours={"~": "MB_POND_WATER"}).components), 2)
        self.assertEqual(len(self.flood(["..."], solid=[(1, 0)]).components), 2)

    def test_mismatch_rule(self):
        self.assertFalse(graph.mismatch(0, 4))
        self.assertFalse(graph.mismatch(3, 0))
        self.assertFalse(graph.mismatch(3, 15))
        self.assertTrue(graph.mismatch(3, 4))


class Hash(unittest.TestCase):
    def test_crc16_ccitt_false_check_value(self):
        self.assertEqual(emit.crc16_ccitt_false(b"123456789"), 0x29B1)


# --------------------------------------------------------------------------
# Real maps.
# --------------------------------------------------------------------------

@NEEDS_MAPS
class Graph(unittest.TestCase):
    def test_scope_matches_the_inventory(self):
        world, _, _ = full()
        regions = {}
        for m in world.scope:
            regions[m.region] = regions.get(m.region, 0) + 1
        self.assertEqual(regions, {"Kanto": 217, "Johto": 259, "Hoenn": 401, "Sevii": 135})

    def test_viridian_route2_lane_offset(self):
        _, wg, _ = full()
        viridian = wg.map_nodes("ViridianCity_hns")
        north = [e for n in viridian for e in n.edges if e.kind == graph.KIND_NORTH
                 and wg.nodes[e.target].map.name == "Route2_hns"]
        self.assertEqual(len(north), 1)
        lane = north[0]
        offset = lane.a - lane.c  # target coordinate = source coordinate - offset
        self.assertEqual(offset, 16)
        self.assertTrue(lane.a <= 25 <= lane.b)
        self.assertEqual(25 - offset, 9)  # Viridian (25, 0) <-> Route 2 (9, 79)
        back = [e for e in wg.nodes[lane.target].edges if e.kind == graph.KIND_SOUTH
                and wg.nodes[e.target].map.name == "ViridianCity_hns"]
        self.assertEqual(len(back), 1)
        self.assertTrue(back[0].a <= 9 <= back[0].b)
        self.assertEqual(9 - (back[0].a - back[0].c), 25)

    def test_route2_is_split(self):
        # The spec's two halves (solid rows 41-45) hold, and the Cut trees at
        # (11, 13), (18, 26), (15, 62) and (15, 69) split them further, since
        # walkers never cut: five nodes, not the spec's two.
        _, wg, _ = full()
        nodes = wg.map_nodes("Route2_hns")
        self.assertEqual([(n.x, n.y) for n in nodes],
                         [(8, 0), (13, 3), (16, 27), (18, 46), (5, 51)])
        pewter = [n for n in nodes if any(wg.nodes[e.target].map.name == "PewterCity_hns"
                                          for e in n.edges)]
        viridian = [n for n in nodes if any(wg.nodes[e.target].map.name == "ViridianCity_hns"
                                            for e in n.edges)]
        self.assertEqual(len(pewter), 1)
        self.assertEqual(len(viridian), 1)
        self.assertNotEqual(pewter[0].id, viridian[0].id)

    def test_viridian_pokemon_center_door(self):
        _, wg, _ = full()
        city = wg.map_nodes("ViridianCity_hns")
        doors = [e for n in city for e in n.edges if e.kind == graph.KIND_WARP
                 and wg.nodes[e.target].map.name == "ViridianCity_PokemonCenter_hns"]
        self.assertEqual(len(doors), 1)
        door = doors[0]
        self.assertEqual((door.a, door.b, door.c, door.warp_id), (30, 36, 0, 4))
        center = wg.nodes[door.target]
        out = [e for e in center.edges if e.kind == graph.KIND_WARP
               and wg.nodes[e.target].map.name == "ViridianCity_hns"]
        self.assertTrue(out)
        self.assertEqual(out[0].c, 4)  # lands on the city's warp 4, the door

    def test_edges_sorted_and_targets_exist(self):
        _, wg, _ = full()
        for n in wg.nodes:
            keys = [(e.kind, e.a, e.b) for e in n.edges]
            self.assertEqual(keys, sorted(keys))
            for e in n.edges:
                self.assertLess(e.target, len(wg.nodes))


@NEEDS_MAPS
class Spots(unittest.TestCase):
    def test_cerulean_center_counter(self):
        _, wg, _ = full()
        tiles = spots_on(wg, "CeruleanCity_PokemonCenter_hns", sp.CENTER_COUNTER)
        # Counter row (5-9, 3); the nurse's front (7, 4) stays free.
        self.assertEqual([(x, y) for x, y, _ in tiles], [(5, 4), (6, 4), (8, 4), (9, 4)])
        self.assertTrue(all(f == sp.DIR_NORTH for _, _, f in tiles))

    def test_rustboro_emerald_counter_row(self):
        _, wg, _ = full()
        tiles = spots_on(wg, "RustboroCity_PokemonCenter_1F", sp.CENTER_COUNTER)
        self.assertEqual(len(tiles), 5)
        self.assertTrue(all(y == 4 and f == sp.DIR_NORTH for _, y, f in tiles))

    def test_celadon_game_corner_sign(self):
        _, wg, _ = full()
        tiles = spots_on(wg, "CeladonCity_GameCorner_hns", sp.GAME_CORNER)
        self.assertIn((4, 6, sp.DIR_EAST), tiles)
        self.assertEqual(len(tiles), 16)

    def test_route1_tall_grass(self):
        world, wg, _ = full()
        info = world.maps["Route1_hns"]
        grid = world.grid(info)
        grass = wg.detector.tall_grass
        total = sum(1 for b in grid.mb if b in grass)
        self.assertEqual(total, 206)
        patches = [s for s in wg.spots if s.map is info and s.kind == sp.TALL_GRASS]
        for s in patches:
            self.assertGreaterEqual(len(s.area), sp.PATCH_MIN)
            self.assertIn((s.x, s.y), s.area)
            self.assertTrue(all(grid.b(x, y) in grass for x, y in s.area))

    def test_authored_drops(self):
        _, wg, _ = full()
        self.assertEqual(spots_on(wg, "GoldenrodCity_BikeShop_hns", sp.STORE), [])
        self.assertEqual(spots_on(wg, "SaffronCity_FightingDojo_hns", sp.GYM), [])
        self.assertTrue(spots_on(wg, "GoldenrodCity_BikeShop_hns", sp.NAMED))

    def test_spot_order_and_runs(self):
        _, wg, _ = full()
        keys = [s.key() for s in wg.spots]
        self.assertEqual(keys, sorted(keys))
        for s in wg.spots:
            if s.kind in (sp.TALL_GRASS, sp.SQUARE, sp.GYM):
                self.assertEqual(s.facing, sp.DIR_NONE)

    def test_named_spots_all_present(self):
        _, wg, _ = full()
        named = [s for s in wg.spots if s.kind == sp.NAMED]
        self.assertEqual(len(named), 59)
        for s in named:
            self.assertNotEqual(s.activities & 0xF, 15)
            if s.capacity == 2:
                self.assertTrue(s.flags & sp.FLAG_CAPACITY_2)

    def test_named_spot_validation_failure(self):
        world, wg, _ = full()
        rows = [
            {"region": "Kanto", "label": "On an object", "map": "CeruleanCity_PokemonCenter_hns",
             "x": 13, "y": 4, "activities": ["relax"], "capacity": 1},
            {"region": "Kanto", "label": "Bad activity", "map": "CeruleanCity_PokemonCenter_hns",
             "x": 2, "y": 6, "activities": ["dance"], "capacity": 1},
            {"region": "Johto", "label": "Wrong region", "map": "CeruleanCity_PokemonCenter_hns",
             "x": 2, "y": 6, "activities": ["relax"], "capacity": 1},
            {"region": "Kanto", "label": "Two indoors", "map": "CeruleanCity_PokemonCenter_hns",
             "x": 2, "y": 7, "activities": ["relax"], "capacity": 2},
            {"region": "Kanto", "label": "No map", "map": "NoSuchMap", "x": 1, "y": 1,
             "activities": ["relax"], "capacity": 1},
        ]
        problems = []
        out = authored.build_named(rows, world, wg.detector, problems)
        self.assertEqual(out, [])
        text = "\n".join(problems)
        for needle in ("holds an object", "activities", "the row is listed under Johto",
                       "over the interior 1 cap", "does not exist"):
            self.assertIn(needle, text)


@NEEDS_MAPS
class Routines(unittest.TestCase):
    def test_trainers_resolve(self):
        _, wg, rt = full()
        self.assertEqual(len(rt.trainers), 25)
        for t in rt.trainers:
            self.assertGreaterEqual(t.search_bound, 2 * t.radius)
            self.assertEqual(t.candidate_hops, sorted(t.candidate_hops))
            self.assertTrue(all(h <= t.radius for h in t.candidate_hops))
            for first, count, _, _ in t.favourites:
                self.assertTrue(1 <= count <= 255)
            if t.leader:
                self.assertEqual(wg.spots[t.own_gym_spot].kind, sp.GYM)
                self.assertEqual(wg.nodes[t.gym_node].map, t.gym_map)

    def test_broken_favourite_fails(self):
        _, wg, _ = full()
        data = maps.load_json(maps.TOOL_DIR / "routines.json")
        broken = copy.deepcopy(data)
        misty = next(e for e in broken["trainers"] if e["slug"] == "misty")
        misty["favourites"] = [{"serves": "fish", "map": "CeruleanCity_Gym_hns",
                                "kind": "tall_grass"}]
        surge = next(e for e in broken["trainers"] if e["slug"] == "lt-surge")
        surge["favourites"] = [{"serves": "gamble", "map": "CeladonCity_GameCorner_hns",
                                "kind": "game_corner"},
                               {"serves": "visit", "map": "VermilionCity_Gym_hns", "kind": "gym"}]
        path = Path(self._tmp()) / "routines.json"
        path.write_text(maps.json.dumps(broken), encoding="utf-8")
        rt = routines.Routines(wg, routines_path=path)
        text = "\n".join(rt.problems)
        self.assertIn("Misty: favourite", text)
        self.assertIn("resolves to no tall_grass spot", text)
        self.assertIn("own Gym", text)
        with self.assertRaises(BuildError):
            rt.check()

    def _tmp(self):
        import tempfile
        d = tempfile.mkdtemp(prefix="wayfarer-world-test-")
        self.addCleanup(lambda: __import__("shutil").rmtree(d, ignore_errors=True))
        return d


@NEEDS_MAPS
class Determinism(unittest.TestCase):
    def test_two_runs_identical(self):
        _, _, first, report = generate.generate(ROOT)
        _, _, second, _ = generate.generate(ROOT)
        self.assertEqual(first, second)
        self.assertIn("gWayfarerWorldContentHash = %s;" % report["content_hash"], first)


if __name__ == "__main__":
    unittest.main()
