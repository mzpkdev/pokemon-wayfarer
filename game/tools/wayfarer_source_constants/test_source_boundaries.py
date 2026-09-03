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

FLAG_OPERAND_RE = re.compile(
    r"(?m)^\s*(?:setflag|clearflag|checkflag|goto_if_(?:set|unset)|call_if_(?:set|unset))\s+"
    r"(FLAG_[A-Za-z0-9_]+)"
)
VAR_OPERAND_RE = re.compile(
    r"(?m)^\s*(?:setvar|addvar|subvar|copyvar|setorcopyvar|compare|switch|specialvar|"
    r"goto_if_(?:eq|ne|lt|le|gt|ge)|call_if_(?:eq|ne|lt|le|gt|ge)|map_script_2)\s+"
    r"(VAR_[A-Za-z0-9_]+)"
)
DEFINE_RE = re.compile(r"^#define\s+(FLAG_[A-Za-z0-9_]+|VAR_[A-Za-z0-9_]+)\s+0x([0-9A-F]+)$")
PAIR_RE = re.compile(r"\{ 0x([0-9A-F]+), 0x([0-9A-F]+) \}, // (FLAG_[A-Za-z0-9_]+|VAR_[A-Za-z0-9_]+)")


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

    def test_emerald_map_event_operands_are_mappable(self):
        checked = 0
        for _, data in self.emerald_maps():
            for event in data.get("object_events", []):
                flag = event.get("flag", "0")
                if isinstance(flag, str) and flag.startswith("FLAG_"):
                    self.assertIn(flag, self.hoenn)
                    value = self.hoenn[flag]
                    if value == 0:
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
            self.assertTrue(value <= 0x1F or 0x6000 <= value <= 0x6FFF or value >= 0x4000, flag)
        for var in variables:
            self.assertIn(var, self.hoenn)
            value = self.hoenn[var]
            self.assertTrue(0x7000 <= value <= 0x70FF or value >= 0x8000, var)

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
