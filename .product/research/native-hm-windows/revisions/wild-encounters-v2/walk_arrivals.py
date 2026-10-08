"""List the encounter sources reachable on foot from each Sevii and Alola arrival.

Sevii: the Seagallop ferry lands at each island's harbor (src/seagallop.c, 8, 5).
Alola: the Kanto boat lands on Melemele (34, 26); the Tapu signs' fast travel
lands on Akala (9, 11), Ula'ula's forest (16, 5) and Poni (13, 8), and Surf
landings reach the same areas. Hoenn: Mr. Briney's boat lands at Dewford
(13, 10). Sootopolis and Ever Grande use their Pokemon Center doors.
scenarios_regions.json is built from this output.
"""
import json, os
import walker as W
import sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../../../game/tools/wild_encounters/v2'))
import hm_model

HERE = os.path.dirname(os.path.abspath(__file__))
META = json.load(open(os.path.join(hm_model.HERE, 'meta.json')))  # game/src/data/wild_encounters_v2/
ARRIVALS = {
    'one_island': ('MAP_ONE_ISLAND_HARBOR', 8, 5), 'two_island': ('MAP_TWO_ISLAND_HARBOR', 8, 5),
    'three_island': ('MAP_THREE_ISLAND_HARBOR', 8, 5), 'four_island': ('MAP_FOUR_ISLAND_HARBOR', 8, 5),
    'five_island': ('MAP_FIVE_ISLAND_HARBOR', 8, 5), 'six_island': ('MAP_SIX_ISLAND_HARBOR', 8, 5),
    'seven_island': ('MAP_SEVEN_ISLAND_HARBOR', 8, 5), 'melemele': ('MAP_MELEMELE_ISLE_HNS', 34, 26),
    'akala': ('MAP_AKALA_ISLE_HNS', 9, 11), 'ulaula': ('MAP_ULA_ULA_FOREST_HNS', 16, 5),
    'poni': ('MAP_PONI_ISLE_HNS', 13, 8),
    'dewford': ('MAP_DEWFORD_TOWN', 13, 10),  # Mr. Briney's boat
    'into_sinjoh': ('MAP_SNOWSWEPT_CAVERN_HNS', 50, 68),  # from Mt. Silver's waterfall room
}
for m in ('MAP_SOOTOPOLIS_CITY', 'MAP_EVER_GRANDE_CITY'):
    door = [(e['x'], e['y']) for e in W.MAPS[m]['warp_events'] if 'POKEMON_CENTER' in e['dest_map']][0]
    ARRIVALS[m[4:].lower()] = (m, door[0], door[1])
door = [(e['x'], e['y']) for e in W.MAPS['MAP_NEW_SINJOH_HNS']['warp_events'] if 'POKEMON_CENTER' in e['dest_map']][0]
ARRIVALS['out_of_sinjoh'] = ('MAP_NEW_SINJOH_HNS', door[0], door[1])
for name, start in ARRIVALS.items():
    src = W.sources(W.walk([start]))
    print(name, {m: sorted(v) for m, v in sorted(src.items()) if m in META})
