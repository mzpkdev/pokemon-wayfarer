"""Tests for the walker graph and spot table generator.

The map tests read the Wayfarer build's mapjson outputs; they skip when the
worktree's generated map files are for another map version.
"""

import copy
import os
import subprocess
import sys
import tempfile
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
if os.environ.get("WAYFARER_WORLD_REQUIRE_MAPS") == "1" and not HAVE_WAYFARER_MAPS:
    # make check on the Wayfarer map version: a missing map output is a
    # failure, not a reason to skip the map tests.
    raise RuntimeError("the Wayfarer build's mapjson outputs are missing")
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
        # (18, 26), (15, 62) and (15, 69) split them further, since walkers
        # never cut: four nodes, not the spec's two. The tree at (11, 13) is
        # an authored override (the way to Diglett's Cave).
        _, wg, _ = full()
        nodes = wg.map_nodes("Route2_hns")
        self.assertEqual([(n.x, n.y) for n in nodes],
                         [(8, 0), (16, 27), (18, 46), (5, 51)])
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

    def test_one_way_stairs_split_mt_mortar(self):
        # Sideways stairs are one-way in places: the door from 1F South at
        # (66, 61) and the water's-edge pocket at (56..59, 20) can't reach
        # each other on foot in either direction, so they are separate
        # nodes (the walker's grid search agrees, verifier F5).
        _, wg, _ = full()
        flood = wg.builder.floods["MtMortar_1F_North_hns"]
        door = flood.tile_component(66, 61)
        pocket = flood.tile_component(56, 20)
        self.assertNotEqual(door, pocket)
        self.assertEqual(flood.tile_component(66, 33), door)
        self.assertEqual(flood.tile_component(15, 31), flood.tile_component(53, 14))

    def test_strongly_connected(self):
        adj = {1: [2], 2: [1, 3], 3: [4], 4: [3]}
        comps = sorted(sorted(c) for c in graph.strongly_connected([1, 2, 3, 4], adj))
        self.assertEqual(comps, [[1, 2], [3, 4]])

    def test_water_edges_pair_up_on_land(self):
        _, wg, _ = full()
        water = [e for e in wg.edges if e.kind == graph.KIND_WATER]
        self.assertTrue(water)
        for e in water:
            back = wg.nodes[e.target].edges[e.c]
            self.assertEqual((back.kind, back.target), (graph.KIND_WATER, e.source))
            flood = wg.builder.floods[wg.nodes[e.source].map.name]
            self.assertEqual(flood.walkable[e.b * flood.grid.w + e.a], 1)
        # Sootopolis's only way out is the authored Dive link.
        soot = {n.id for n in wg.map_nodes("SootopolisCity")}
        out = {wg.nodes[e.target].map.name for e in water if e.source in soot}
        self.assertTrue(out - {"SootopolisCity"})

    def test_elite_four_fly_joins_indigo_and_the_reception_gate(self):
        # Victory Road is one-way on foot; the authored fly link is the way
        # down from Indigo Plateau (and back), off-screen only.
        _, wg, _ = full()
        indigo = {n.id for n in wg.map_nodes("IndigoPlateau_hns")}
        gate = {n.id for n in wg.map_nodes("ReceptionGate_hns")}
        water = [e for e in wg.edges if e.kind == graph.KIND_WATER]
        self.assertTrue(any(e.source in indigo and e.target in gate for e in water))
        self.assertTrue(any(e.source in gate and e.target in indigo for e in water))

    def test_only_firing_warps_are_edges(self):
        # A warp edge needs a warp event the engine fires: a warp behaviour
        # on its tile, a TryStartWarpEventScript layout fallback, or an
        # authored script door (Petalburg Gym). The walker's door step-in
        # (wayfarer_walkers.c, IsGoalTile) only ever targets such a tile.
        world, wg, _ = full()
        b = wg.builder
        for n in wg.nodes:
            for e in n.edges:
                if e.kind != graph.KIND_WARP:
                    continue
                self.assertTrue(b.fires(n.map, e.warp_id), (n.map.name, e.warp_id))
        # Terra Cave's dormant entrances (solid rock on five routes) are not
        # a hub between those routes.
        terra = {n.id for n in wg.nodes if n.map.name.startswith("TerraCave")}
        into = {n.map.name for n in wg.nodes for e in n.edges if e.target in terra
                and not n.map.name.startswith("TerraCave")}
        self.assertEqual(into, set())
        # A fall's landing back up (Burned Tower B1F (16, 12)) is no edge.
        b1f = wg.map_nodes("BurnedTower_B1F_hns")
        self.assertFalse(any(wg.nodes[e.target].map.name == "BurnedTower_1F_hns"
                             and e.kind == graph.KIND_WARP and e.warp_id == 1
                             for n in b1f for e in n.edges))
        # Petalburg Gym's script doors still lead from the lobby to Norman.
        gym = wg.map_nodes("PetalburgCity_Gym")
        lobby = next(n for n in gym if wg.builder.floods["PetalburgCity_Gym"].tile_component(4, 110) == n.comp)
        top = next(n for n in gym if wg.builder.floods["PetalburgCity_Gym"].tile_component(4, 3) == n.comp)
        seen, todo = {lobby.id}, [lobby.id]
        while todo:
            for e in wg.nodes[todo.pop()].edges:
                if wg.nodes[e.target].map.name == "PetalburgCity_Gym" and e.target not in seen:
                    seen.add(e.target)
                    todo.append(e.target)
        self.assertIn(top.id, seen)

    def test_e2e_proven_passages(self):
        # Passages the SkyEmu journeys walk (cinnabar-interior-port and the
        # Seafoam and Mansion journeys): an independent check of the firing
        # rules, not the predicate the generator uses. The Lab doors fire
        # through mapjson's coord triggers.
        _, wg, _ = full()
        cases = [("PokemonMansion_1F_Frlg", 9, 13, "PokemonMansion_2F_Frlg"),
                 ("PokemonMansion_2F_Frlg", 7, 14, "PokemonMansion_1F_Frlg"),
                 ("PokemonMansion_2F_Frlg", 8, 3, "PokemonMansion_3F_Frlg"),
                 ("PokemonMansion_3F_Frlg", 9, 3, "PokemonMansion_2F_Frlg"),
                 ("PokemonMansion_1F_Frlg", 26, 27, "PokemonMansion_B1F_Frlg"),
                 ("PokemonMansion_B1F_Frlg", 33, 29, "PokemonMansion_1F_Frlg"),
                 ("CinnabarIsland_PokemonLab_Entrance_Frlg", 13, 6, "CinnabarIsland_PokemonLab_Lounge_Frlg"),
                 ("CinnabarIsland_PokemonLab_Entrance_Frlg", 19, 6, "CinnabarIsland_PokemonLab_ResearchRoom_Frlg"),
                 ("CinnabarIsland_PokemonLab_Entrance_Frlg", 25, 6, "CinnabarIsland_PokemonLab_ExperimentRoom_Frlg"),
                 ("CinnabarIsland_PokemonLab_Lounge_Frlg", 7, 8, "CinnabarIsland_PokemonLab_Entrance_Frlg"),
                 ("CinnabarIsland_PokemonCenter_1F_Frlg", 2, 6, "CinnabarIsland_PokemonCenter_2F_Frlg"),
                 ("CinnabarIsland_PokemonCenter_2F_Frlg", 2, 6, "CinnabarIsland_PokemonCenter_1F_Frlg"),
                 ("CinnabarIsland_Gym_Frlg", 25, 22, "CinnabarIsland_Frlg"),
                 ("Route20_Frlg", 60, 9, "SeafoamIslands_1F_Frlg"),
                 ("Route20_Frlg", 72, 15, "SeafoamIslands_1F_Frlg"),
                 ("SeafoamIslands_1F_Frlg", 6, 20, "Route20_Frlg"),
                 ("SeafoamIslands_1F_Frlg", 32, 20, "Route20_Frlg")]
        for name, x, y, dest in cases:
            node = wg.node_of[(name, wg.builder.floods[name].tile_component(x, y))]
            targets = {wg.nodes[e.target].map.name for e in wg.nodes[node].edges}
            self.assertIn(dest, targets, (name, x, y))
        # Cerulean's Bike Shop door is a non-animated door with collision:
        # TryDoorWarp opens only animated doors, so it never fires.
        city = wg.map_nodes("CeruleanCity_hns")
        self.assertFalse(any(wg.nodes[e.target].map.name == "CeruleanCity_BikeShop_hns"
                             for n in city for e in n.edges))
        # A door under a New Game object (Lavaridge Gym's trainers on their
        # warp tiles) never fires either.
        self.assertFalse(any(e.kind == graph.KIND_WARP and e.warp_id in (8, 9, 12, 17)
                             for n in wg.map_nodes("LavaridgeTown_Gym_1F") for e in n.edges))

    def test_named_spot_needs_a_way_in(self):
        world, _, _ = full()
        rows = authored.load_named() + [
            {"region": "Kanto", "label": "Cerulean Bike Shop", "map": "CeruleanCity_BikeShop_hns",
             "x": 3, "y": 5, "activities": ["shop"], "capacity": 1}]
        wg = build.WorldGraph(world, named_rows=rows)
        self.assertTrue(any("Cerulean Bike Shop" in p and "no way in" in p for p in wg.problems))

    def test_script_warp_validation(self):
        world, wg, _ = full()
        with self.assertRaises(BuildError):
            build.WorldGraph(world, overrides=_overrides(self, {"script_warps": [
                {"map": "PetalburgCity_Gym", "warps": [0]}]})).check()

    def test_surfer_crosses_foam_rows(self):
        # Mt Mortar 1F South's row y=28 (plain floor at elevation 1) splits
        # the waterfall from the lower pool; a surfer crosses it, so the
        # upper floors and 1F North's (56, 20) pocket are reachable.
        _, wg, _ = full()
        flood = wg.builder.floods["MtMortar_1F_North_hns"]
        pocket = wg.node_of[("MtMortar_1F_North_hns", flood.tile_component(56, 20))]
        start = wg.node_of[("MtMortar_1F_North_hns", flood.tile_component(66, 61))]
        seen, todo = {start}, [start]
        while todo:
            for e in wg.nodes[todo.pop()].edges:
                if e.target not in seen:
                    seen.add(e.target)
                    todo.append(e.target)
        self.assertIn(pocket, seen)
        mortar = [n.id for n in wg.nodes if n.map.name.startswith("MtMortar")]
        self.assertEqual([n for n in mortar if n not in seen], [])

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

    def test_one_spot_per_tile(self):
        # Each spot seats its own trainer, so no tile (nor a named spot's
        # second tile) holds two spots; the more specific kind keeps it.
        _, wg, _ = full()
        seats = {}
        for s in wg.spots:
            tiles = [(s.x, s.y)] + ([tuple(s.second)] if s.second else [])
            for x, y in tiles:
                self.assertNotIn((s.map.name, x, y), seats)
                seats[(s.map.name, x, y)] = s
        # Violet Mart's clerk tile keeps its store spot, not the NPC chat.
        self.assertNotIn((6, 3), [(x, y) for x, y, _ in spots_on(wg, "VioletCity_Mart_hns", sp.NPC_CHAT)])
        self.assertIn((6, 3), [(x, y) for x, y, _ in spots_on(wg, "VioletCity_Mart_hns", sp.STORE)])

    def test_drops_must_match(self):
        _, wg, _ = full()
        self.assertEqual(wg.overrides.unused_drops(), [])
        bad = _overrides(self, {"drop": [{"map": "GoldenrodCity_BikeShop_hns", "kind": "gym"},
                                         {"map": "ViridianCity_hns", "x": 1, "y": 1}]})
        text = "\n".join(bad.unused_drops())
        self.assertIn("GoldenrodCity_BikeShop_hns has no detected gym", text)
        self.assertIn("ViridianCity_hns has no detected spot at (1, 1)", text)

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
        self.assertEqual(len(named), 58)
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

    def test_leader_objects_by_local_id(self):
        _, _, rt = full()
        by_name = {t.name: t for t in rt.trainers}
        for t in rt.trainers:
            if not t.leader:
                self.assertEqual(t.leader_local_id, 0)
                continue
            objects = {o["local_id"]: o for o in t.gym_map.events["objects"]}
            self.assertIn(t.leader_local_id, objects, t.name)
        # Fuchsia's Gym has four decoys drawn as Janine: the leader is the one
        # running her own script.
        janine = by_name["Janine"]
        leader = {o["local_id"]: o for o in janine.gym_map.events["objects"]}[janine.leader_local_id]
        self.assertTrue(leader["script"].endswith("_EventScript_Janine"))
        # Viridian's Gym draws Giovanni with FireRed's sprite.
        self.assertIn("OBJ_EVENT_GFX_GIOVANNI", by_name["Giovanni"].alt_graphics)

    def test_rom_paths_stay_in_region(self):
        # The ROM's search has no region limit; the build checks that each
        # non-traveller's ROM paths are its region-limited ones. Lorelei's
        # paths are all in Sevii: limited to Kanto, the check must fail.
        _, wg, rt = full()
        t = next(t for t in rt.trainers if t.slug == "lorelei")
        self.assertFalse(t.traveller)
        saved, before = t.regions, list(rt.problems)
        try:
            t.regions = {"Kanto"}
            rt.check_rom_paths(t, t.name, sorted({t.home_node} | {wg.spots[i].node for i in t.candidates}
                                                 | {wg.spots[i].node for f in t.favourites
                                                    for i in range(f[0], f[0] + f[1])}),
                               rt.allowed(t))
            self.assertTrue(any("ROM's path" in p or "home-hop" in p or "out-of-region" in p
                                for p in rt.problems[len(before):]))
        finally:
            t.regions = saved
            rt.problems[:] = before

    def test_reroutes_cannot_leave_the_region(self):
        # Today every out-of-region part touches a non-traveller's regions
        # at one node at most, so the check passes (the build ran it). Make
        # Saffron City "Johto" for a moment: a foreign pocket inside Kanto,
        # entered from four gates, through which a reroute could pass.
        _, wg, rt = full()
        t = next(t for t in rt.trainers if t.slug == "lt-surge")
        saffron = [n for n in wg.map_nodes("SaffronCity_hns")]
        before = list(rt.problems)
        try:
            for n in saffron:
                n.region = "Johto"
            rt.check_rom_paths(t, t.name, [t.home_node], rt.allowed(t))
            self.assertTrue(any("out-of-region part" in p for p in rt.problems[len(before):]))
        finally:
            for n in saffron:
                n.region = "Kanto"
            rt.problems[:] = before

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


def _overrides(case, extra):
    """An Overrides from a copy of the authored files plus `extra` in a new
    file."""
    import shutil
    d = tempfile.mkdtemp(prefix="wayfarer-world-overrides-")
    case.addCleanup(lambda: shutil.rmtree(d, ignore_errors=True))
    for path in sorted((maps.TOOL_DIR / "overrides").glob("*.json")):
        shutil.copy(path, d)
    (Path(d) / "zz_test.json").write_text(maps.json.dumps(extra), encoding="utf-8")
    return authored.Overrides(Path(d))


@NEEDS_MAPS
class Determinism(unittest.TestCase):
    def test_two_runs_identical(self):
        # Two processes with different hash seeds: set and dict iteration
        # order must not reach the tables.
        outputs = []
        with tempfile.TemporaryDirectory() as tmp:
            for seed in ("1", "777"):
                out, rep = Path(tmp) / ("tables%s.h" % seed), Path(tmp) / ("report%s.json" % seed)
                env = dict(os.environ, PYTHONHASHSEED=seed, PYTHONDONTWRITEBYTECODE="1")
                subprocess.run([sys.executable, str(Path(generate.__file__)), "--root", str(ROOT),
                                "--output", str(out), "--report", str(rep)],
                               check=True, env=env, capture_output=True)
                outputs.append((out.read_text(), __import__("json").loads(rep.read_text())))
        (first, report), (second, _) = outputs
        self.assertEqual(first, second)
        self.assertIn("gWayfarerWorldContentHash = %s;" % report["content_hash"], first)


if __name__ == "__main__":
    unittest.main()
