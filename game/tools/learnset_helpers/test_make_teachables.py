import importlib.util
import io
import unittest
from pathlib import Path
from unittest.mock import patch


MODULE_PATH = Path(__file__).with_name("make_teachables.py")
SPEC = importlib.util.spec_from_file_location("make_teachables", MODULE_PATH)
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)


class TeachableRegistryBuildSelectionTest(unittest.TestCase):
    REGISTRY = """\
#if defined(POKEMON_HNS) || defined(POKEMON_WAYFARER)
F(SHARED)
#if defined(POKEMON_WAYFARER)
F(DIVE)
#else
F(WHIRLPOOL)
#endif
#else
F(EMERALD_DIVE)
#endif
"""

    def moves_for(self, build):
        with patch("builtins.open", return_value=io.StringIO(self.REGISTRY)):
            return list(GENERATOR.extract_repo_tms(build))

    def test_nested_build_guards_select_only_the_active_registry(self):
        self.assertEqual(self.moves_for("POKEMON_WAYFARER"), ["MOVE_SHARED", "MOVE_DIVE"])
        self.assertEqual(self.moves_for("POKEMON_HNS"), ["MOVE_SHARED", "MOVE_WHIRLPOOL"])
        self.assertEqual(self.moves_for("EMERALD"), ["MOVE_EMERALD_DIVE"])


if __name__ == "__main__":
    unittest.main()
