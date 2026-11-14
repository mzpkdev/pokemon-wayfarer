import importlib.util
import json
import re
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


GAME = Path(__file__).resolve().parents[2]
MAPS = GAME / "data" / "maps"
GENERATOR = Path(__file__).with_name("generate.py")
SPEC = importlib.util.spec_from_file_location("wayfarer_source_constants", GENERATOR)
GENERATOR_MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR_MODULE)

FLAG_OPERAND_RE = re.compile(
    r"(?m)^\s*(?:setflag|clearflag|checkflag|goto_if_(?:set|unset)|call_if_(?:set|unset))\s+"
    r"(FLAG_[A-Za-z0-9_]+)"
)
VAR_OPERAND_RE = re.compile(
    r"(?m)^\s*(?:setvar|addvar|subvar|copyvar|setorcopyvar|compare|switch|specialvar|"
    r"goto_if_(?:eq|ne|lt|le|gt|ge)|call_if_(?:eq|ne|lt|le|gt|ge)|map_script_2)\s+"
    r"(VAR_[A-Za-z0-9_]+)"
)
DEFINE_RE = re.compile(r"^#define\s+(FLAG_[A-Za-z0-9_]+|VAR_[A-Za-z0-9_]+|MAPSEC_[A-Za-z0-9_]+)\s+0x([0-9A-F]+)$")
PAIR_RE = re.compile(r"\{ 0x([0-9A-F]+), 0x([0-9A-F]+) \}, // (FLAG_[A-Za-z0-9_]+|VAR_[A-Za-z0-9_]+)")

# Files that touch ids on purpose rather than as gameplay reads.
SCANNER_EXEMPT_FILES = {
    "debug.c": "developer menu pokes any flag or var by name",
    "e2e_test.c": "test harness seeds and inspects state by name",
}


def load_aliases(path):
    aliases = {}
    for line in path.read_text().splitlines():
        match = DEFINE_RE.match(line)
        if match:
            aliases[match.group(1)] = int(match.group(2), 16)
    return aliases


class SourceBoundaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cpp = shutil.which("arm-none-eabi-cpp") or shutil.which("cpp")
        cc = shutil.which("cc")
        if cpp is None or cc is None:
            raise unittest.SkipTest("C preprocessor and host compiler are required")

        cls.temp_dir = tempfile.TemporaryDirectory()
        cls.hoenn_path = Path(cls.temp_dir.name) / "hoenn.inc"
        cls.engine_path = Path(cls.temp_dir.name) / "engine.inc"
        cls.common_path = Path(cls.temp_dir.name) / "common.inc"
        cls.common_data_path = Path(cls.temp_dir.name) / "common.h"
        subprocess.run(
            [
                "python3", str(GENERATOR),
                "--cpp", cpp,
                "--cc", cc,
                "--include-dir", str(GAME / "include"),
                "--hoenn-output", str(cls.hoenn_path),
                "--engine-output", str(cls.engine_path),
                "--common-output", str(cls.common_path),
                "--common-data-output", str(cls.common_data_path),
                "--event-scripts", str(GAME / "data" / "event_scripts.s"),
                "--scripts-dir", str(GAME / "data" / "scripts"),
                "--maps-dir", str(MAPS),
            ],
            check=True,
        )
        cls.hoenn = load_aliases(cls.hoenn_path)
        cls.engine = load_aliases(cls.engine_path)
        cls.common = load_aliases(cls.common_path)
        cls.common_pairs = {
            match.group(3): (int(match.group(1), 16), int(match.group(2), 16))
            for match in PAIR_RE.finditer(cls.common_data_path.read_text())
        }
        cls.emerald_starter_values = GENERATOR_MODULE.dump_values(
            cc,
            GAME / "include",
            "POKEMON_EMERALD",
            [
                "VAR_STARTER_MON",
                "VAR_HOENN_STARTER_CHOICE",
                "HOENN_STARTER_CHOICE_NONE",
                "HOENN_STARTER_CHOICE_TREECKO",
                "HOENN_STARTER_CHOICE_TORCHIC",
                "HOENN_STARTER_CHOICE_MUDKIP",
                "FLAG_HOENN_STARTER_RECEIVED",
            ],
        )
        cls.hns_starter_values = GENERATOR_MODULE.dump_values(
            cc,
            GAME / "include",
            "POKEMON_HNS",
            ["VAR_STARTER_MON", "VAR_HOENN_STARTER_CHOICE", "FLAG_HOENN_STARTER_RECEIVED"],
        )
        cls.frlg_starter_values = GENERATOR_MODULE.dump_values(
            cc,
            GAME / "include",
            "FIRERED",
            ["VAR_STARTER_MON", "VAR_HOENN_STARTER_CHOICE", "FLAG_HOENN_STARTER_RECEIVED"],
        )

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "temp_dir"):
            cls.temp_dir.cleanup()

    def emerald_maps(self):
        for map_json in MAPS.glob("*/map.json"):
            data = json.loads(map_json.read_text())
            if data.get("game_version", "emerald") == "emerald":
                yield map_json.parent, data

    def test_representative_constants_use_stable_namespaces(self):
        self.assertEqual(self.hoenn["FLAG_RESCUED_BIRCH"], 0x6052)
        self.assertEqual(self.hoenn["FLAG_SYS_GAME_CLEAR"], 0x6864)
        self.assertEqual(self.hoenn["FLAG_BADGE01_GET"], 0x6867)
        self.assertEqual(self.hoenn["VAR_LITTLEROOT_TOWN_STATE"], 0x7050)
        self.assertEqual(self.engine["FLAG_RESCUED_BIRCH"], 0)
        self.assertEqual(self.engine["VAR_LITTLEROOT_TOWN_STATE"], 0x4050)
        self.assertEqual(self.hoenn["MAPSEC_PETALBURG_CITY"], 7)
        self.assertEqual(self.engine["MAPSEC_PETALBURG_CITY"], 0)

    def test_hoenn_starter_constants_are_source_scoped(self):
        self.assertEqual(self.hoenn["VAR_HOENN_STARTER_CHOICE"], 0x7023)
        self.assertEqual(self.hoenn["FLAG_HOENN_STARTER_RECEIVED"], 0x64FF)
        self.assertEqual(self.engine["VAR_STARTER_MON"], 0x4023)
        self.assertEqual(self.emerald_starter_values["VAR_STARTER_MON"], 0x4023)
        self.assertEqual(self.emerald_starter_values["VAR_HOENN_STARTER_CHOICE"], 0x4023)
        self.assertEqual(self.emerald_starter_values["FLAG_HOENN_STARTER_RECEIVED"], 0x4FF)
        self.assertEqual(self.emerald_starter_values["HOENN_STARTER_CHOICE_NONE"], 0xFFFF)
        self.assertEqual(self.emerald_starter_values["HOENN_STARTER_CHOICE_TREECKO"], 0)
        self.assertEqual(self.emerald_starter_values["HOENN_STARTER_CHOICE_TORCHIC"], 1)
        self.assertEqual(self.emerald_starter_values["HOENN_STARTER_CHOICE_MUDKIP"], 2)
        self.assertEqual(self.hns_starter_values["VAR_STARTER_MON"], 0x4023)
        self.assertEqual(self.hns_starter_values["VAR_HOENN_STARTER_CHOICE"], 0x4023)
        self.assertEqual(self.hns_starter_values["FLAG_HOENN_STARTER_RECEIVED"], 0)
        self.assertEqual(self.frlg_starter_values["VAR_HOENN_STARTER_CHOICE"], 0x4023)
        self.assertEqual(self.frlg_starter_values["FLAG_HOENN_STARTER_RECEIVED"], 0)

    def test_all_hoenn_starter_consumers_use_dedicated_symbol(self):
        occurrences = GENERATOR_MODULE.audit_hoenn_starter_symbols(MAPS)
        counts = {}
        for path, _, symbol in occurrences:
            self.assertEqual(symbol, "VAR_HOENN_STARTER_CHOICE")
            relative_path = str(path.relative_to(MAPS))
            counts[relative_path] = counts.get(relative_path, 0) + 1

        self.assertEqual(counts, {
            "LilycoveCity/scripts.inc": 2,
            "MauvilleCity_GameCorner/scripts.inc": 2,
            "PetalburgCity_PokemonCenter_1F/scripts.inc": 3,
            "Route103/scripts.inc": 2,
            "Route104/scripts.inc": 2,
            "Route110/scripts.inc": 2,
            "Route119/scripts.inc": 2,
            "RustboroCity/scripts.inc": 2,
        })

    def test_reserved_starter_received_source_flag_is_unused(self):
        authored_paths = [
            GAME / "data" / "event_scripts.s",
            *sorted((GAME / "data" / "scripts").glob("*.inc")),
            *sorted(MAPS.glob("*/map.json")),
            *sorted(MAPS.glob("*/scripts.inc")),
        ]
        uses = [str(path.relative_to(GAME)) for path in authored_paths
                if "FLAG_UNUSED_0x4FF" in path.read_text()]
        self.assertEqual(uses, [])
        self.assertTrue(
            GENERATOR_MODULE.FLAG_LOW_START <= 0x4FF <= GENERATOR_MODULE.FLAG_LOW_END
        )

    def test_emerald_map_event_operands_are_mappable(self):
        checked = 0
        for _, data in self.emerald_maps():
            for event in data.get("object_events", []):
                flag = event.get("flag", "0")
                if isinstance(flag, str) and flag.startswith("FLAG_"):
                    self.assertIn(flag, self.hoenn)
                    value = self.hoenn[flag]
                    if value == 0 or flag in GENERATOR_MODULE.ENGINE_SHARED_STATE:
                        continue
                    self.assertTrue(value <= 0x1F or 0x6000 <= value <= 0x6FFF or value >= 0x4000, flag)
                    checked += 1
            for event in data.get("coord_events", []):
                var = event.get("var", "0")
                if isinstance(var, str) and var.startswith("VAR_"):
                    self.assertIn(var, self.hoenn)
                    value = self.hoenn[var]
                    self.assertTrue(0x7000 <= value <= 0x70FF or value >= 0x8000, var)
                    checked += 1
            for event in data.get("bg_events", []):
                if event.get("type") == "hidden_item":
                    flag = event["flag"]
                    self.assertIn(flag, self.hoenn)
                    self.assertTrue(0x6000 <= self.hoenn[flag] <= 0x6FFF, flag)
                    checked += 1
        self.assertGreater(checked, 1000)

    def test_emerald_map_script_operands_are_mappable(self):
        flags = set()
        variables = set()
        for map_dir, _ in self.emerald_maps():
            script_path = map_dir / "scripts.inc"
            if not script_path.exists():
                continue
            source = script_path.read_text()
            flags.update(FLAG_OPERAND_RE.findall(source))
            variables.update(VAR_OPERAND_RE.findall(source))

        self.assertGreater(len(flags), 300)
        self.assertGreater(len(variables), 100)
        for flag in flags:
            if flag not in self.hoenn:
                self.assertIn(flag, self.engine)
                self.assertNotEqual(self.engine[flag], 0, flag)
                continue
            value = self.hoenn[flag]
            if value == 0:
                continue
            if flag in GENERATOR_MODULE.ENGINE_SHARED_FLAGS or flag in GENERATOR_MODULE.ENGINE_SHARED_STATE:
                self.assertEqual(value, self.engine[flag], flag)
                continue
            self.assertTrue(value <= 0x1F or 0x6000 <= value <= 0x6FFF or value >= 0x4000, flag)
        for var in variables:
            self.assertIn(var, self.hoenn)
            value = self.hoenn[var]
            if GENERATOR_MODULE.OBJ_GFX_VAR_START <= value <= GENERATOR_MODULE.OBJ_GFX_VAR_END:
                # The engine draws OBJ_EVENT_GFX_VAR_* objects from these ids, so they stay shared.
                self.assertTrue(var.startswith("VAR_OBJ_GFX_ID_"), var)
                continue
            if var in GENERATOR_MODULE.ENGINE_SHARED_STATE:
                self.assertEqual(value, self.engine[var], var)
                continue
            self.assertTrue(0x7000 <= value <= 0x70FF or value >= 0x8000, var)

    def hoenn_script_names_that_differ_from_the_engine(self):
        names = set()
        for map_dir, _ in self.emerald_maps():
            script_path = map_dir / "scripts.inc"
            if script_path.exists():
                names.update(GENERATOR_MODULE.PERSISTENT_TOKEN_RE.findall(script_path.read_text()))
        # Shared scripts run on Hoenn maps too; the ones whose ids differ are
        # exactly the common dispatch table.
        names.update(self.common_pairs)
        return {
            name for name in names
            if name in self.hoenn and self.hoenn[name] != self.engine.get(name, 0)
        }

    def c_source_names_outside_hoenn_routing(self):
        found = {}
        for path in sorted([*(GAME / "src").glob("*.c"), *(GAME / "include").glob("*.h")]):
            if path.name in SCANNER_EXEMPT_FILES:
                continue
            for number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
                line = re.sub(r"//.*", "", line)
                line = re.sub(r"\b(?:HOENN_(?:FLAG|VAR)_ID)\([^)]*\)", "", line)
                if re.match(r"\s*#\s*define\s+WAYFARER_HOENN_", line):
                    continue
                for name in GENERATOR_MODULE.PERSISTENT_TOKEN_RE.findall(line):
                    found.setdefault(name, []).append(f"{path.relative_to(GAME)}:{number}")
        return found

    def test_hoenn_id_wrappers_around_names_use_the_emerald_value(self):
        # HOENN_*_ID(NAME) adds the Hoenn namespace to whatever NAME means in the engine build. A name that
        # Wayfarer renumbers or zeroes (like the FLAG_HIDE_* flags) silently resolves to the wrong source id,
        # so the engine value must equal Emerald's.
        wrapper = re.compile(r"\bHOENN_(?:FLAG|VAR)_ID\(\s*((?:FLAG|VAR)_[A-Za-z0-9_]+)\s*\)")
        found = {}
        for directory in ("src", "include", "test"):
            for path in sorted((GAME / directory).rglob("*")):
                if path.suffix not in (".c", ".h") or path.name in SCANNER_EXEMPT_FILES:
                    continue
                text = re.sub(r"//.*", "", path.read_text(errors="replace"))
                for name in wrapper.findall(text):
                    found.setdefault(name, str(path.relative_to(GAME)))
        self.assertGreater(len(found), 5)
        cc = shutil.which("cc")
        emerald = GENERATOR_MODULE.dump_values(cc, GAME / "include", "POKEMON_EMERALD", sorted(found))
        wayfarer = GENERATOR_MODULE.dump_values(cc, GAME / "include", "POKEMON_HNS", sorted(found))
        for name, where in sorted(found.items()):
            self.assertEqual(wayfarer[name], emerald[name], f"{name} in {where}: write the Emerald id instead")

    def test_engine_code_does_not_read_a_hoenn_script_copy_of_its_state(self):
        # A Hoenn script writing 0x6000/0x7000-bank ids while C reads the engine
        # id (or the reverse) is a silent mismatch: the puzzle never completes,
        # the call never rings. Every name in both sets must be shared with the
        # engine by rule, or be listed here with the reason it is safe.
        allowed = {
            # routed by region through a Wayfarer* helper (wayfarer_persistence.c, tv.c); the engine id only serves the non-Hoenn branch
            "FLAG_SYS_FRONTIER_PASS": "bridged",
            "FLAG_SYS_PC_LANETTE": "bridged",
            "FLAG_SYS_TV_HOME": "bridged",
            "FLAG_SYS_TV_LATIAS_LATIOS": "bridged",
            "FLAG_SYS_TV_START": "bridged",
            "FLAG_SYS_TV_WATCH": "bridged",
            "FLAG_UNLOCKED_TRENDY_SAYINGS": "bridged",
            # product decision, deliberately left on the raw engine id; Hoenn scripts write their own copy, so these stay unbridged (see #235): Wally/Roxanne/Rival-Rayquaza step calls, Frontier vars, easy-chat and Hall of Fame GAME_CLEAR readers, badge counts for Pokenav/rating, Wattson rematch indicator. Level cap and Hoenn post-game content (contests, Trainer Hill, rematches) now follow Trainer Rating / Hoenn's own clear
            "FLAG_BADGE01_GET": "product",
            "FLAG_BADGE02_GET": "product",
            "FLAG_BADGE03_GET": "product",
            "FLAG_BADGE04_GET": "product",
            "FLAG_BADGE05_GET": "product",
            "FLAG_BADGE06_GET": "product",
            "FLAG_BADGE07_GET": "product",
            "FLAG_BADGE08_GET": "product",
            "FLAG_SYS_GAME_CLEAR": "product",
            "FLAG_DEFEATED_MAGMA_SPACE_CENTER": "product",
            "FLAG_ENABLE_FIRST_WALLY_POKENAV_CALL": "product",
            "FLAG_ENABLE_ROXANNE_FIRST_CALL": "product",
            "FLAG_IS_CHAMPION": "product",
            "FLAG_WATTSON_REMATCH_AVAILABLE": "product",
            "VAR_NUM_BADGES": "product",
            "VAR_ROXANNE_CALL_STEP_COUNTER": "product",
            "VAR_WALLY_CALL_STEP_COUNTER": "product",
            # Battle Frontier is a preservation target only; its facility state is not played in Wayfarer (see #235)
            "FLAG_CANCEL_BATTLE_ROOM_CHALLENGE": "frontier",
            "FLAG_CHOSEN_MULTI_BATTLE_NPC_PARTNER": "frontier",
            "FLAG_HIDE_BATTLE_TOWER_MULTI_BATTLE_PARTNER_ALT_1": "frontier",
            "FLAG_HIDE_BATTLE_TOWER_MULTI_BATTLE_PARTNER_ALT_2": "frontier",
            "FLAG_HIDE_BATTLE_TOWER_REPORTER": "frontier",
            "FLAG_LANDMARK_BATTLE_FRONTIER": "frontier",
            "FLAG_SYS_ARENA_GOLD": "frontier",
            "FLAG_SYS_ARENA_SILVER": "frontier",
            "FLAG_SYS_DOME_GOLD": "frontier",
            "FLAG_SYS_DOME_SILVER": "frontier",
            "FLAG_SYS_FACTORY_GOLD": "frontier",
            "FLAG_SYS_FACTORY_SILVER": "frontier",
            "FLAG_SYS_PALACE_GOLD": "frontier",
            "FLAG_SYS_PALACE_SILVER": "frontier",
            "FLAG_SYS_PIKE_GOLD": "frontier",
            "FLAG_SYS_PIKE_SILVER": "frontier",
            "FLAG_SYS_PYRAMID_GOLD": "frontier",
            "FLAG_SYS_PYRAMID_SILVER": "frontier",
            "FLAG_SYS_TOWER_GOLD": "frontier",
            "FLAG_SYS_TOWER_SILVER": "frontier",
            "VAR_BRAVO_TRAINER_BATTLE_TOWER_ON": "frontier",
            "VAR_FRONTIER_BATTLE_MODE": "frontier",
            "VAR_FRONTIER_FACILITY": "frontier",
            "VAR_FRONTIER_GAMBLER_STATE": "frontier",
            "VAR_FRONTIER_MANIAC_FACILITY": "frontier",
            "VAR_GOLDENROD_CITY_STATE": "frontier",
            "VAR_TEMP_CHALLENGE_STATUS": "frontier",
            "VAR_TEMP_PLAYING_PYRAMID_MUSIC": "frontier",
            "VAR_TRAINER_HILL_IS_ACTIVE": "frontier",
            # Pokenav match-call and rematch tables: the HNS contacts read engine ids, the Scott/Hoenn rows already use HOENN_FLAG_ID
            "FLAG_DEFEATED_DEWFORD_GYM": "matchcall",
            "FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY": "matchcall",
            "FLAG_DEFEATED_LAVARIDGE_GYM": "matchcall",
            "FLAG_DEFEATED_METEOR_FALLS_STEVEN": "matchcall",
            "FLAG_DEFEATED_MOSSDEEP_GYM": "matchcall",
            "FLAG_DEFEATED_PETALBURG_GYM": "matchcall",
            "FLAG_DEFEATED_SOOTOPOLIS_GYM": "matchcall",
            "FLAG_DELIVERED_DEVON_GOODS": "matchcall",
            "FLAG_DELIVERED_STEVEN_LETTER": "matchcall",
            "FLAG_ENABLE_BRAWLY_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_FLANNERY_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_JUAN_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_MOM_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_NORMAN_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_PROF_BIRCH_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_RIVAL_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_ROXANNE_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_SCOTT_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_TATE_AND_LIZA_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_WALLY_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_WATTSON_MATCH_CALL": "matchcall",
            "FLAG_ENABLE_WINONA_MATCH_CALL": "matchcall",
            "FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT": "matchcall",
            "FLAG_HIDE_MAUVILLE_CITY_WALLY": "matchcall",
            "FLAG_HIDE_SOOTOPOLIS_CITY_RAYQUAZA": "matchcall",
            "FLAG_HIDE_VICTORY_ROAD_ENTRANCE_WALLY": "matchcall",
            "FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN": "matchcall",
            "FLAG_MET_ARCHIE_METEOR_FALLS": "matchcall",
            "FLAG_MET_TEAM_AQUA_HARBOR": "matchcall",
            "FLAG_RECEIVED_CASTFORM": "matchcall",
            "FLAG_RECEIVED_EXP_SHARE": "matchcall",
            "FLAG_RECEIVED_HM_STRENGTH": "matchcall",
            "FLAG_RECEIVED_RED_OR_BLUE_ORB": "matchcall",
            "FLAG_REGISTERED_STEVEN_POKENAV": "matchcall",
            "FLAG_RUSTURF_TUNNEL_OPENED": "matchcall",
            "FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE": "matchcall",
            # region map and map-preview visited/landmark state, handled separately from puzzle state
            "FLAG_LANDMARK_POKEMON_LEAGUE": "map",
            "FLAG_LANDMARK_SOUTHERN_ISLAND": "map",
            "FLAG_VISITED_DEWFORD_TOWN": "map",
            "FLAG_VISITED_EVER_GRANDE_CITY": "map",
            "FLAG_VISITED_FALLARBOR_TOWN": "map",
            "FLAG_VISITED_FORTREE_CITY": "map",
            "FLAG_VISITED_KANTO": "map",
            "FLAG_VISITED_LAVARIDGE_TOWN": "map",
            "FLAG_VISITED_LILYCOVE_CITY": "map",
            "FLAG_VISITED_LITTLEROOT_TOWN": "map",
            "FLAG_VISITED_MAUVILLE_CITY": "map",
            "FLAG_VISITED_MOSSDEEP_CITY": "map",
            "FLAG_VISITED_OLDALE_TOWN": "map",
            "FLAG_VISITED_PACIFIDLOG_TOWN": "map",
            "FLAG_VISITED_PETALBURG_CITY": "map",
            "FLAG_VISITED_RUSTBORO_CITY": "map",
            "FLAG_VISITED_SLATEPORT_CITY": "map",
            "FLAG_VISITED_SOOTOPOLIS_CITY": "map",
            "FLAG_VISITED_VERDANTURF_TOWN": "map",
            "FLAG_WORLD_MAP_ROCKET_HIDEOUT_B1F": "map",
            "VAR_ABNORMAL_WEATHER_LOCATION": "map",
            # scratch the C side owns end to end (ice rows, rotating gates, voltorb flip); ClearTempFieldEventData clears both banks
            "VAR_TEMP_0": "temp",
            "VAR_TEMP_1": "temp",
            "VAR_TEMP_2": "temp",
            "VAR_TEMP_3": "temp",
            "VAR_TEMP_4": "temp",
            "VAR_TEMP_5": "temp",
            "VAR_TEMP_6": "temp",
            "VAR_TEMP_7": "temp",
            "VAR_TEMP_8": "temp",
            "VAR_TEMP_9": "temp",
            "VAR_TEMP_A": "temp",
            "VAR_TEMP_E": "temp",
            "VAR_TEMP_MIXED_RECORDS": "temp",
            "VAR_TEMP_RECORD_MIX_GIFT_ITEM": "temp",
            # pre-existing, outside this fix: engine-wide progress granted by Wayfarer origin code, mystery gift, ticket menus, fan club and similar; listed so new additions stand out
            "FLAG_ADDED_MATCH_CALL_TO_POKENAV": "other",
            "FLAG_DECORATION_1": "other",
            "FLAG_DECORATION_14": "other",
            "FLAG_DEFEATED_WALLY_VICTORY_ROAD": "other",
            "FLAG_ENABLE_SHIP_BIRTH_ISLAND": "other",
            "FLAG_ENABLE_SHIP_FARAWAY_ISLAND": "other",
            "FLAG_ENABLE_SHIP_NAVEL_ROCK": "other",
            "FLAG_ENABLE_SHIP_SOUTHERN_ISLAND": "other",
            "FLAG_HIDE_FANCLUB_BOY": "other",
            "FLAG_HIDE_FANCLUB_LADY": "other",
            "FLAG_HIDE_FANCLUB_LITTLE_BOY": "other",
            "FLAG_HIDE_FANCLUB_OLD_LADY": "other",
            "FLAG_HIDE_LILYCOVE_CONTEST_HALL_BLEND_MASTER": "other",
            "FLAG_HIDE_LILYCOVE_FAN_CLUB_INTERVIEWER": "other",
            "FLAG_MET_SCOTT_ON_SS_TIDAL": "other",
            "FLAG_MYSTERY_GIFT_DONE": "other",
            "FLAG_NURSE_UNION_ROOM_REMINDER": "other",
            "FLAG_PENDING_DAYCARE_EGG": "other",
            "FLAG_RECEIVED_AURORA_TICKET": "other",
            "FLAG_RECEIVED_GOOD_ROD": "other",
            "FLAG_RECEIVED_MYSTIC_TICKET": "other",
            "FLAG_RECEIVED_OLD_ROD": "other",
            "FLAG_RECEIVED_OLD_SEA_MAP": "other",
            "FLAG_RECEIVED_POKEDEX_FROM_BIRCH": "other",
            "FLAG_RECEIVED_POKENAV": "other",
            "FLAG_RECEIVED_RUNNING_SHOES": "other",
            "FLAG_RECEIVED_SUPER_ROD": "other",
            "FLAG_SET_WALL_CLOCK": "other",
            "FLAG_SHOWN_AURORA_TICKET": "other",
            "FLAG_SHOWN_EON_TICKET": "other",
            "FLAG_SHOWN_MYSTIC_TICKET": "other",
            "FLAG_SHOWN_OLD_SEA_MAP": "other",
            "FLAG_SYS_B_DASH": "other",
            "FLAG_SYS_CHAT_USED": "other",
            "FLAG_SYS_CYCLING_ROAD": "other",
            "FLAG_SYS_MYSTERY_EVENT_ENABLE": "other",
            "FLAG_SYS_MYSTERY_GIFT_ENABLE": "other",
            "FLAG_SYS_NATIONAL_DEX": "other",
            "FLAG_SYS_POKEDEX_GET": "other",
            "FLAG_SYS_POKEMON_GET": "other",
            "FLAG_SYS_POKENAV_GET": "other",
            "FLAG_SYS_RIBBON_GET": "other",
            "FLAG_SYS_USE_STRENGTH": "other",
            "VAR_CABLE_CLUB_STATE": "other",
            "VAR_DISTRIBUTE_EON_TICKET": "other",
            "VAR_ENIGMA_BERRY_AVAILABLE": "other",
            "VAR_GIFT_PICHU_SLOT": "other",
            "VAR_LEAGUE_STATE": "other",
            "VAR_LILYCOVE_FAN_CLUB_STATE": "other",
            "VAR_MAP_SCENE_FUCHSIA_CITY_SAFARI_ZONE_ENTRANCE": "other",
            "VAR_MASSAGE_COOLDOWN_STEP_COUNTER": "other",
            "VAR_NEWBARKTOWN_LABSTATE": "other",
            "VAR_PC_BOX_TO_SEND_MON": "other",
            "VAR_POKELOT_PRIZE_ITEM": "other",
            "VAR_SSAQUA_STATE": "other",
            "VAR_SS_TIDAL_STATE": "other",
            "VAR_STARTER_MON": "other",
        }
        mismatched = self.hoenn_script_names_that_differ_from_the_engine()
        referenced = self.c_source_names_outside_hoenn_routing()
        unexplained = sorted(
            name for name in mismatched & set(referenced)
            if name not in allowed
            and name not in GENERATOR_MODULE.ENGINE_SHARED_FLAGS
            and name not in GENERATOR_MODULE.ENGINE_SHARED_STATE
        )
        self.assertEqual(
            unexplained, [],
            "\n".join(f"{name}: {', '.join(referenced[name][:3])}" for name in unexplained),
        )
        for name in allowed:
            self.assertIn(name, mismatched & set(referenced), f"stale allow-list entry {name}")

    def test_source_boundaries_restore_hns_constants(self):
        source = (GAME / "data" / "event_scripts.s").read_text()
        hoenn = source.index('"data/wayfarer_hoenn_source_constants.inc"')
        petalburg = source.index('"data/maps/PetalburgCity/scripts.inc"')
        route124 = source.index('"data/maps/Route124_DivingTreasureHuntersHouse/scripts.inc"')
        restore = source.index('"data/wayfarer_engine_source_constants.inc"', route124)
        hns = source.index('"data/maps/TestMap2_hns/scripts.inc"')
        self.assertLess(hoenn, petalburg)
        self.assertLess(route124, restore)
        self.assertLess(restore, hns)

    def test_shared_players_house_var_is_runtime_dispatched(self):
        self.assertTrue(0xB000 <= self.common["VAR_LITTLEROOT_INTRO_STATE"] <= 0xBFFF)
        self.assertEqual(self.common_pairs["VAR_LITTLEROOT_INTRO_STATE"], (0x4092, 0x7092))

    def test_common_dispatch_ids_are_bounded_and_unique(self):
        flag_ids = [value for value in self.common.values() if 0xA000 <= value <= 0xAFFF]
        var_ids = [value for value in self.common.values() if 0xB000 <= value <= 0xBFFF]
        self.assertGreater(len(flag_ids), 250)
        self.assertGreater(len(var_ids), 100)
        self.assertEqual(len(flag_ids), len(set(flag_ids)))
        self.assertEqual(len(var_ids), len(set(var_ids)))

    def test_common_dispatch_boundary_covers_shared_scripts_only(self):
        source = (GAME / "data" / "event_scripts.s").read_text()
        common = source.index('"data/wayfarer_common_source_constants.inc"')
        first_shared = source.index('"data/scripts/std_msgbox.inc"')
        restore = source.index('"data/wayfarer_engine_source_constants.inc"', common)
        first_hns_map = source.index('"data/maps/TestMap2_hns/scripts.inc"')
        players_house = source.index('"data/scripts/players_house.inc"')
        self.assertLess(common, first_shared)
        self.assertLess(players_house, restore)
        self.assertLess(restore, first_hns_map)

    def test_interleaved_hns_scripts_use_engine_constants(self):
        source = (GAME / "data" / "event_scripts.s").read_text()
        state = "engine"
        checked = 0
        for line in source.splitlines():
            if "wayfarer_hoenn_source_constants.inc" in line:
                state = "hoenn"
            elif "wayfarer_common_source_constants.inc" in line:
                state = "common"
            elif "wayfarer_engine_source_constants.inc" in line:
                state = "engine"
            elif re.search(r'"data/maps/[^\"]+_hns/scripts\.inc"', line):
                self.assertEqual(state, "engine", line)
                checked += 1
        self.assertGreater(checked, 500)

    def test_hoenn_lifecycle_scripts_use_hoenn_constants(self):
        source = (GAME / "data" / "event_scripts.s").read_text()
        state = "engine"
        hall_of_fame = None
        whiteout_reset = None
        for line in source.splitlines():
            if "wayfarer_hoenn_source_constants.inc" in line:
                state = "hoenn"
            elif "wayfarer_common_source_constants.inc" in line:
                state = "common"
            elif "wayfarer_engine_source_constants.inc" in line:
                state = "engine"
            elif '"data/scripts/hall_of_fame.inc"' in line:
                hall_of_fame = state
            elif line.startswith("EventScript_ResetMrBriney::"):
                whiteout_reset = state

        self.assertEqual(hall_of_fame, "common")
        self.assertEqual(whiteout_reset, "common")
        self.assertIn("specialvar VAR_RESULT, WayfarerGetCurrentRegionForScript", source)


if __name__ == "__main__":
    unittest.main()
