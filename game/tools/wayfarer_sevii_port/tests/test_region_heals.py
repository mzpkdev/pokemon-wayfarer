import json
from pathlib import Path
import unittest


GAME = Path(__file__).resolve().parents[3]

HEAL_IDS = {
    "HEAL_LOCATION_ONE_ISLAND",
    "HEAL_LOCATION_TWO_ISLAND",
    "HEAL_LOCATION_THREE_ISLAND",
    "HEAL_LOCATION_FOUR_ISLAND",
    "HEAL_LOCATION_FIVE_ISLAND",
    "HEAL_LOCATION_SIX_ISLAND",
    "HEAL_LOCATION_SEVEN_ISLAND",
}


class WayfarerSeviiRegionAndHealTests(unittest.TestCase):
    def test_only_numbered_island_heals_opt_into_wayfarer_sevii(self):
        rows = json.loads((GAME / "src/data/heal_locations.json").read_text())["heal_locations"]
        opted_in = {row["id"] for row in rows if row.get("wayfarer_sevii")}
        self.assertEqual(opted_in, HEAL_IDS)
        self.assertTrue(all(row["source"] == "FRLG" for row in rows if row.get("wayfarer_sevii")))

        template = (GAME / "src/data/heal_locations.json.txt").read_text()
        self.assertEqual(template.count("HAS_{{ heal_location.source }}_CONTENT || (IS_WAYFARER && HAS_SEVII_CONTENT)"), 3)

    def test_sevii_sections_are_wayfarer_only_and_event_island_variants_stay_unavailable(self):
        sections = json.loads(
            (GAME / "src/data/region_map/region_map_sections.json").read_text()
        )["map_sections"]
        enabled = [row["id"] for row in sections if row.get("wayfarer_sevii")]
        manifest = json.loads((GAME / "src/data/wayfarer_sevii_maps.json").read_text())
        selected_sections = {
            json.loads((GAME / "data/maps" / record["source_map"] / "map.json").read_text())["region_map_section"]
            for record in manifest["maps"]
        }
        self.assertEqual(set(enabled), selected_sections)
        self.assertEqual(len(enabled), 43)
        self.assertIn("MAPSEC_ONE_ISLAND", enabled)
        self.assertIn("MAPSEC_EMBER_SPA", enabled)
        self.assertNotIn("MAPSEC_BIRTH_ISLAND_FRLG", enabled)
        self.assertNotIn("MAPSEC_NAVEL_ROCK_FRLG", enabled)
        self.assertFalse(any("BirthIsland" in record["source_map"] or "NavelRock" in record["source_map"]
                             for record in manifest["maps"]))

        constants = (GAME / "src/data/region_map/region_map_sections.constants.json.txt").read_text()
        self.assertIn("#if IS_WAYFARER", constants)
        self.assertIn('existsIn(map_section, "wayfarer_sevii")', constants)

    def test_region_map_routes_sevii_pages_without_fly_targets(self):
        source = (GAME / "src/region_map.c").read_text()
        self.assertIn("static bool8 IsWayfarerSeviiMapSecId", source)
        self.assertIn("return REGION_MAP_SEVII123;", source)
        self.assertIn("return REGION_MAP_SEVII45;", source)
        self.assertIn("return REGION_MAP_SEVII67;", source)
        self.assertIn("Numbered-island pages are ferry-only", source)
        self.assertIn("return MAPSECTYPE_ROUTE;", source)
        self.assertIn("sRegionMapSections_Sevii123[y][x] == 0", source)
        self.assertIn("sRegionMapSections_Sevii45[y][x] == 0", source)
        self.assertIn("sRegionMapSections_Sevii67[y][x] == 0", source)

        fly_locations = source[source.index("static const struct FlyLocation sFlyLocations[]"):
                               source.index("// Sprite data for SpriteCB_FlyDestIcon")]
        sevii_start = fly_locations.index(".regionMapType = REGION_MAP_SEVII123")
        self.assertIn("#if !IS_HNS", fly_locations[:sevii_start])


if __name__ == "__main__":
    unittest.main()
