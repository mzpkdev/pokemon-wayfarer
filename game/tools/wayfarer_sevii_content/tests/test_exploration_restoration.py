"""The Sevii manifest restores FRLG's exploration content exactly as authored."""
from collections import Counter
import json
from pathlib import Path
import re
import unittest

GAME = Path(__file__).resolve().parents[3]
MANIFEST = GAME / "src/data/wayfarer_sevii_maps.json"
FLAGS = GAME / "include/constants/flags.h"
WILD = GAME / "src/data/wayfarer_sevii_wild_encounters.json"
SCRIPTS = GAME / "data/scripts/wayfarer_sevii"
HELPERS = {"EventScript_RockSmash", "EventScript_CutTree", "EventScript_StrengthBoulder"}
SEVII_FLAG = re.compile(r"(?m)^#define (FLAG_WAYFARER_SEVII_[A-Z0-9_]+) WAYFARER_SEVII_FLAG_ID\((\d+)\)$")


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


class SeviiExplorationRestorationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = load(MANIFEST)
        cls.sources = {record["source_map"]: load(GAME / "data/maps" / record["source_map"] / "map.json")
                       for record in cls.manifest["maps"]}
        cls.rows = {}
        for record in cls.manifest["maps"]:
            for kind, rows in record["retained_events"].items():
                for row in rows:
                    cls.rows[(record["source_map"], kind, row["index"])] = row
        cls.excluded = {(identity["source_map"], identity["event_kind"], identity["index"]): exclusion["content_id"]
                        for exclusion in cls.manifest["exclusions"] for identity in exclusion["source_identities"]}

    def source_events(self, kind):
        for source_map, source in self.sources.items():
            for index, event in enumerate(source.get(kind, [])):
                yield source_map, index, event

    def test_pickup_exclusions_are_gone(self):
        ids = {exclusion["content_id"] for exclusion in self.manifest["exclusions"]}
        self.assertNotIn("exclusion.story.static-visible-pickups", ids)
        self.assertNotIn("exclusion.story.static-hidden-pickups", ids)

    def test_item_balls_keep_frlg_position_item_and_get_a_sevii_flag(self):
        items = [(m, i, e) for m, i, e in self.source_events("object_events") if e.get("graphics_id") == "OBJ_EVENT_GFX_ITEM_BALL"]
        self.assertEqual(len(items), 39)
        module = (SCRIPTS / "items.inc").read_text(encoding="utf-8")
        frlg = (GAME / "data/scripts/item_ball_scripts_frlg.inc").read_text(encoding="utf-8")
        for source_map, index, event in items:
            row = self.rows[(source_map, "object_events", index)]
            self.assertEqual(row["owner"], "exploration")
            self.assertEqual(row["source"], event)
            self.assertEqual(row["content_id"], f"exploration.{source_map.lower().replace('_', '-')}.object.{index}")
            self.assertEqual(row["overrides"], {"flag": "FLAG_WAYFARER_SEVII_ITEM_" + event["flag"].removeprefix("FLAG_HIDE_")})
            wrapper = row["wayfarer_script"]
            authored = re.search(rf"(?m)^{event['script']}::\n(\tfinditem [^\n]+)\n\tend$", frlg).group(1)
            self.assertIn(f"{wrapper}::\n{authored}\n\tend\n", module)

    def test_hidden_items_keep_frlg_source_and_get_a_sevii_flag(self):
        hidden = [(m, i, e) for m, i, e in self.source_events("bg_events") if e.get("type") == "hidden_item"]
        self.assertEqual(len(hidden), 58)
        for source_map, index, event in hidden:
            row = self.rows[(source_map, "bg_events", index)]
            self.assertEqual(row["owner"], "exploration")
            self.assertEqual(row["source"], event)
            self.assertNotIn("wayfarer_script", row)
            map_id = self.sources[source_map]["id"].removeprefix("MAP_")
            expected = {"flag": f"FLAG_WAYFARER_SEVII_HIDDEN_{map_id}_{index}"}
            if event["underfoot"]:
                # Daily world slots: nothing picks up an underfoot hidden item, so Wayfarer clears it.
                expected["underfoot"] = False
            self.assertEqual(row["overrides"], expected)

    def test_pickup_flags_use_unique_free_sevii_bank_slots(self):
        declared = SEVII_FLAG.findall(FLAGS.read_text(encoding="utf-8"))
        slots = Counter(slot for _, slot in declared)
        duplicates = sorted(slot for slot, count in slots.items() if count > 1)
        self.assertEqual(duplicates, [])
        by_name = {name: int(slot) for name, slot in declared}
        # Slots 1-9 are the cracked-ice range that starts at slot 1.
        reserved = set(range(1, 10)) | {int(slot) for _, slot in declared if not _.startswith(("FLAG_WAYFARER_SEVII_ITEM_", "FLAG_WAYFARER_SEVII_HIDDEN_"))}
        contract_slots = {row["slot"] for row in self.manifest["contracts"]["states"] if row["storage"] == "flag"}
        pickups = [row["overrides"]["flag"] for key, row in self.rows.items()
                   if row.get("owner") == "exploration" and "flag" in row.get("overrides", {}) and row["overrides"]["flag"] != "0"]
        self.assertEqual(len(pickups), 97)
        self.assertEqual(len(set(pickups)), 97)
        pickup_slots = {by_name[flag] for flag in pickups}
        self.assertEqual(pickup_slots, set(range(61, 61 + 97)))
        self.assertFalse(pickup_slots & reserved)
        self.assertFalse(pickup_slots & contract_slots)
        self.assertLess(max(pickup_slots), self.manifest["contracts"]["state_namespace"]["flag_capacity"])

    def test_field_move_terrain_is_retained_with_shared_helpers(self):
        counts = Counter()
        for source_map, index, event in self.source_events("object_events"):
            script = event.get("script")
            if script in HELPERS:
                row = self.rows[(source_map, "object_events", index)]
                self.assertEqual((row["owner"], row["wayfarer_script"], row["source"]), ("exploration", script, event))
                self.assertNotIn("overrides", row)
                counts[script] += 1
        self.assertEqual(counts, {"EventScript_RockSmash": 57, "EventScript_CutTree": 16, "EventScript_StrengthBoulder": 33})
        # Five Island shows the Meadow's border tree through a clone event.
        clone = self.rows[("FiveIsland_Frlg", "object_events", 2)]
        self.assertEqual(clone["source"]["type"], "clone")
        self.assertEqual(clone["source"]["target_local_id"], "LOCALID_FIVE_ISLAND_MEADOW_BORDER_TREE")
        meadow = self.sources["FiveIsland_Meadow_Frlg"]["object_events"]
        target = next(i for i, e in enumerate(meadow) if e.get("local_id") == "LOCALID_FIVE_ISLAND_MEADOW_BORDER_TREE")
        self.assertIn(("FiveIsland_Meadow_Frlg", "object_events", target), self.rows)

    def test_every_rock_smash_table_has_a_rock_to_smash(self):
        tables = {row["map"] for row in load(WILD)["profiles"] if row["method"] == "rock_smash_mons"}
        self.assertTrue(tables)
        rocks = {self.sources[source_map]["id"] for (source_map, kind, _), row in self.rows.items()
                 if kind == "object_events" and row.get("wayfarer_script") == "EventScript_RockSmash"}
        self.assertEqual(tables - rocks, set())

    def test_every_sign_is_restored_except_link_minigame_records(self):
        signs = [(m, i, e) for m, i, e in self.source_events("bg_events") if e.get("type") == "sign"]
        self.assertEqual(len(signs), 104)
        owners = Counter()
        for source_map, index, event in signs:
            key = (source_map, "bg_events", index)
            if key in self.excluded:
                self.assertEqual(self.excluded[key], "exclusion.exploration.link-minigame-records")
                self.assertIn(event["script"], {"TwoIsland_JoyfulGameCorner_EventScript_ShowDodrioBerryPickingRecords",
                                                "TwoIsland_JoyfulGameCorner_EventScript_ShowPokemonJumpRecords"})
                owners["excluded"] += 1
                continue
            row = self.rows[key]
            self.assertTrue(row["wayfarer_script"].startswith("WayfarerSevii_"))
            owners[row["owner"]] += 1
        self.assertEqual(owners["excluded"], 2)
        self.assertEqual(sum(owners.values()), 104)
        cages = [row for row in self.rows.values() if row.get("wayfarer_script") == "WayfarerSevii_RocketWarehouse_Cage"]
        self.assertEqual(len(cages), 22)
        self.assertTrue(all(row["owner"] == "story" and row["state_reads"] == ["SEVII_WAREHOUSE_CLEARED"] for row in cages))

    def test_sign_module_only_shows_text(self):
        commands = set(self.manifest["script_modules"]["signs"]["allowed_commands"])
        self.assertLessEqual(commands, {"braillemessage_wait", "braillemsgbox", "end", "lockall", "msgbox", "releaseall", "setvar"})
        text = (SCRIPTS / "signs.inc").read_text(encoding="utf-8")
        self.assertNotIn("famechecker", text)
        self.assertNotRegex(text, r"(?m)^\tsetvar (?!VAR_0x8005, 130$)")

    def test_spa_heal_and_dolls_are_restored(self):
        spa = self.rows[("OneIsland_KindleRoad_EmberSpa_Frlg", "coord_events", 0)]
        self.assertEqual(spa["wayfarer_script"], "WayfarerSevii_OneIsland_KindleRoad_EmberSpa_SpaHeal")
        self.assertIn("\tspecial HealPlayerParty\n", (SCRIPTS / "spa.inc").read_text(encoding="utf-8"))
        dolls = [(i, e) for i, e in enumerate(self.sources["FourIsland_LoreleisHouse_Frlg"]["object_events"])
                 if e.get("script") == "FourIsland_LoreleisHouse_EventScript_Doll"]
        self.assertEqual([i for i, _ in dolls], list(range(1, 15)))
        for index, event in dolls:
            row = self.rows[("FourIsland_LoreleisHouse_Frlg", "object_events", index)]
            visible = row.get("overrides", {}).get("flag", event["flag"])
            self.assertEqual(visible, "0")

    def test_story_coord_triggers_stay_out_of_scope(self):
        dropped = [(m, i) for m, i, e in self.source_events("coord_events")
                   if (m, "coord_events", i) not in self.rows]
        self.assertEqual(len(dropped), 23)
        self.assertNotIn(("OneIsland_KindleRoad_EmberSpa_Frlg", 0), dropped)

    def test_ruby_path_and_ruin_valley_keep_every_frlg_traversal_event(self):
        # The FRLG layouts are imported unchanged, so keeping every authored
        # rock, tree, boulder, item and sign at its source position keeps
        # these routes completable exactly as in FRLG.
        maps = [name for name in self.sources if name.startswith("MtEmber_RubyPath_")] + ["SixIsland_RuinValley_Frlg"]
        self.assertEqual(len(maps), 9)
        terrain = 0
        for source_map in maps:
            for index, event in enumerate(self.sources[source_map].get("object_events", [])):
                if event.get("script") in HELPERS or event.get("graphics_id") == "OBJ_EVENT_GFX_ITEM_BALL":
                    self.assertEqual(self.rows[(source_map, "object_events", index)]["source"], event)
                    terrain += event.get("script") in HELPERS
                    # Projected objects never block a path that FRLG left open.
                    self.assertEqual(event.get("trainer_type"), "TRAINER_TYPE_NONE")
            for index, event in enumerate(self.sources[source_map].get("bg_events", [])):
                self.assertIn((source_map, "bg_events", index), self.rows)
        self.assertGreater(terrain, 0)
        door = self.rows[("SixIsland_RuinValley_Frlg", "bg_events",
                          next(i for i, e in enumerate(self.sources["SixIsland_RuinValley_Frlg"]["bg_events"])
                               if e.get("script") == "SixIsland_RuinValley_EventScript_DottedHoleDoor"))]
        self.assertEqual(door["wayfarer_script"], "WayfarerSevii_RuinValley_DottedHoleDoor")
        environment = (SCRIPTS / "environment.inc").read_text(encoding="utf-8")
        self.assertIn("goto_if_set FLAG_WAYFARER_SEVII_DOTTED_HOLE_OPEN, WayfarerSevii_RuinValley_DottedHoleDoorOpen", environment)


if __name__ == "__main__":
    unittest.main()
