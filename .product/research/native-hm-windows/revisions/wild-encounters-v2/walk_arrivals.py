"""List the encounter sources reachable on foot from each Sevii and Alola arrival.

Sevii: the Seagallop ferry lands at each island's harbor (src/seagallop.c, 8, 5).
Alola: the Kanto boat lands on Melemele (34, 26); the Tapu signs' fast travel
lands on Akala (9, 11), Ula'ula's forest (16, 5) and Poni (13, 8), and Surf
landings reach the same areas. scenarios_regions.json is built from this output.
"""
import json, os
import walker as W

HERE = os.path.dirname(os.path.abspath(__file__))
META = json.load(open(os.path.join(HERE, 'data', 'meta.json')))
ARRIVALS = {
    'one_island': ('MAP_ONE_ISLAND_HARBOR', 8, 5), 'two_island': ('MAP_TWO_ISLAND_HARBOR', 8, 5),
    'three_island': ('MAP_THREE_ISLAND_HARBOR', 8, 5), 'four_island': ('MAP_FOUR_ISLAND_HARBOR', 8, 5),
    'five_island': ('MAP_FIVE_ISLAND_HARBOR', 8, 5), 'six_island': ('MAP_SIX_ISLAND_HARBOR', 8, 5),
    'seven_island': ('MAP_SEVEN_ISLAND_HARBOR', 8, 5), 'melemele': ('MAP_MELEMELE_ISLE_HNS', 34, 26),
    'akala': ('MAP_AKALA_ISLE_HNS', 9, 11), 'ulaula': ('MAP_ULA_ULA_FOREST_HNS', 16, 5),
    'poni': ('MAP_PONI_ISLE_HNS', 13, 8),
}
for name, start in ARRIVALS.items():
    src = W.sources(W.walk([start]))
    print(name, {m: sorted(v) for m, v in sorted(src.items()) if m in META})
