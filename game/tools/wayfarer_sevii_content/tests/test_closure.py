import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest


TOOL = Path(__file__).resolve().parents[1] / "closure.py"
SPEC = importlib.util.spec_from_file_location("wayfarer_sevii_closure", TOOL)
CLOSURE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CLOSURE)


def sha(value):
    return hashlib.sha256(value.encode()).hexdigest()


class WayfarerSeviiClosureTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        source = self.root / "data/maps/OneIsland_Frlg/scripts.inc"
        source.parent.mkdir(parents=True)
        source.write_text(
            "OneIsland_Frlg_MapScripts::\n"
            "\tmap_script MAP_SCRIPT_ON_LOAD, OneIsland_Frlg_OnLoad\n"
            "\t.byte 0\n\n"
            "OneIsland_Frlg_OnLoad::\n\tend\n"
        )
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.parent.mkdir(parents=True)
        module.write_text("WayfarerSevii_OneIsland_OnLoad::\n\tcall WayfarerSevii_Helper\n\tend\n\nWayfarerSevii_Helper::\n\tend\n")

    def tearDown(self):
        self.temp.cleanup()

    def manifest(self):
        return {
            "script_modules": {
                "environment": {
                    "owner": "exploration",
                    "include": "data/scripts/wayfarer_sevii/environment.inc",
                    "exports": ["WayfarerSevii_OneIsland_OnLoad", "WayfarerSevii_Helper"],
                    "allowed_externals": [], "allowed_commands": ["call", "end"],
                },
            },
            "maps": [{
                "source_map": "OneIsland_Frlg",
                "retained_map_scripts": [{
                    "owner": "exploration", "content_id": "exploration.one_island.load",
                    "module": "environment", "handler_type": "MAP_SCRIPT_ON_LOAD", "source_index": 0,
                    "source": {"include": "data/maps/OneIsland_Frlg/scripts.inc", "label": "OneIsland_Frlg_OnLoad",
                               "sha256": sha("map_script MAP_SCRIPT_ON_LOAD, OneIsland_Frlg_OnLoad\n"),
                               "file_sha256": sha("OneIsland_Frlg_MapScripts::\n\tmap_script MAP_SCRIPT_ON_LOAD, OneIsland_Frlg_OnLoad\n\t.byte 0\n\nOneIsland_Frlg_OnLoad::\n\tend\n")},
                    "wayfarer_script": "WayfarerSevii_OneIsland_OnLoad", "reason": "Source-free wrapper.",
                }],
            }],
        }

    def test_reports_owned_module_and_exact_handler_provenance(self):
        report = CLOSURE.build_script_closure(self.root, self.manifest())
        self.assertEqual(report["labels"], ["WayfarerSevii_Helper", "WayfarerSevii_OneIsland_OnLoad"])
        self.assertEqual(report["map_script_tables"][0]["handler_type"], "MAP_SCRIPT_ON_LOAD")
        self.assertIn("data/maps/OneIsland_Frlg/scripts.inc", CLOSURE.dependency_paths(self.root, self.manifest()))

    def test_reports_atomic_story_transaction_specials_and_c_owned_state_effects(self):
        specials = {
            "WayfarerSevii_TryGiveItemThenSetFlag": ("grant", ("VAR_0x8005",)),
            "WayfarerSevii_TryRemoveItemThenSetFlag": ("handoff", ("VAR_0x8005",)),
            "WayfarerSevii_TryExchangeItemForReward": ("handoff", ("VAR_0x8006",)),
            "WayfarerSevii_TryExchangeItemForRewardThenSetFlags": ("handoff", ("VAR_0x8006", "VAR_0x8007")),
            "WayfarerSevii_TryGiveEggThenSetFlag": ("grant", ("VAR_0x8005",)),
            "WayfarerSevii_TryClaimSelphyPendingReward": ("claim", ()),
        }
        source = self.root / "data/specials.inc"
        source.parent.mkdir(parents=True, exist_ok=True)
        source.write_text("".join(f"def_special {name}\n" for name in specials), encoding="utf-8")
        implementation = self.root / "src/wayfarer_sevii_story.c"
        implementation.parent.mkdir(parents=True)
        implementation.write_text("\n".join(f"void {name}(void) {{}}" for name in specials), encoding="utf-8")
        module = self.root / "data/scripts/wayfarer_sevii/story/transactions.inc"
        module.parent.mkdir(parents=True)
        labels = []
        source_lines = []
        for index, (special, (_, variables)) in enumerate(specials.items()):
            label = f"WayfarerSevii_Transaction{index}"
            labels.append(label)
            source_lines.append(f"{label}::")
            for variable in variables:
                source_lines.append(f"\tsetvar {variable}, FLAG_WAYFARER_SEVII_RECEIPT_{index}_{variable[-1]}")
            source_lines.extend((f"\tspecialvar VAR_RESULT, {special}", "\tend"))
        module.write_text("\n".join(source_lines) + "\n", encoding="utf-8")
        source_sha = hashlib.sha256(source.read_bytes()).hexdigest()
        implementation_sha = hashlib.sha256(implementation.read_bytes()).hexdigest()
        manifest = {
            "script_modules": {"story_transactions": {
                "owner": "story", "include": "data/scripts/wayfarer_sevii/story/transactions.inc",
                "exports": labels, "allowed_commands": ["setvar", "specialvar", "end"],
                "allowed_externals": [{
                    "label": name, "kind": "special", "path": "data/specials.inc", "sha256": source_sha,
                    "implementation_path": "src/wayfarer_sevii_story.c", "implementation_sha256": implementation_sha,
                } for name in specials],
            }},
            "maps": [],
        }

        report = CLOSURE.build_script_closure(self.root, manifest)
        operations = {(row["label"], row["command"]): row for row in report["content_operations"]}
        for index, (special, (kind, _)) in enumerate(specials.items()):
            self.assertIn((f"WayfarerSevii_Transaction{index}", special), operations)
            self.assertEqual(CLOSURE.transaction_kind(special), kind)
        writes = {(row["label"], row["state"]) for row in report["state_operations"] if row["access"] == "write"}
        self.assertIn(("WayfarerSevii_Transaction0", "FLAG_WAYFARER_SEVII_RECEIPT_0_5"), writes)
        self.assertIn(("WayfarerSevii_Transaction3", "FLAG_WAYFARER_SEVII_RECEIPT_3_6"), writes)
        self.assertIn(("WayfarerSevii_Transaction3", "FLAG_WAYFARER_SEVII_RECEIPT_3_7"), writes)
        self.assertIn(("WayfarerSevii_Transaction5", "VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD"), writes)

    def test_rejects_duplicate_handler_type(self):
        manifest = self.manifest()
        manifest["maps"][0]["retained_map_scripts"].append(dict(manifest["maps"][0]["retained_map_scripts"][0]))
        with self.assertRaisesRegex(CLOSURE.ClosureError, "duplicate map-script handler type"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_module_collision_with_centrally_generated_map_table(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text("OneIsland_Frlg_MapScripts::\n\t.byte 0\nWayfarerSevii_OneIsland_OnLoad::\n\tend\n")
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["OneIsland_Frlg_MapScripts", "WayfarerSevii_OneIsland_OnLoad"]
        manifest["script_modules"]["environment"]["allowed_commands"] = ["end"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "owns generated map-script table label"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_reference_expression(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text("WayfarerSevii_OneIsland_OnLoad::\n\tcallnative UnreviewedNative+1\n\tend\n")
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["WayfarerSevii_OneIsland_OnLoad"]
        manifest["script_modules"]["environment"]["allowed_commands"] = ["callnative", "end"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "malformed reference operand UnreviewedNative\\+1"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_constant_in_pointer_reference_slot(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text("WayfarerSevii_OneIsland_OnLoad::\n\tcallnative VAR_TEMP_0\n\tend\n")
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["WayfarerSevii_OneIsland_OnLoad"]
        manifest["script_modules"]["environment"]["allowed_commands"] = ["callnative", "end"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "malformed reference operand VAR_TEMP_0"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_trainerbattle_keeps_numeric_fields_out_of_pointer_closure(self):
        refs = CLOSURE._line_references(
            "trainerbattle TRAINER_BATTLE_SINGLE, LOCALID_NONE, TRAINER_TEST, Intro, Lose, Continue, "
            "LOCALID_NONE, TRAINER_NONE, IntroB, LoseB, ContinueB, Victory, CannotBattle, FALSE, TRUE, FALSE, FALSE"
        )
        self.assertEqual([label for label, _ in refs], ["Intro", "Lose", "Continue", "IntroB", "LoseB", "ContinueB", "Victory", "CannotBattle"])
        self.assertEqual([label for label, _ in CLOSURE._line_references("trainerbattle_single TRAINER_TEST, Intro, Lose")], ["Intro", "Lose"])
        self.assertEqual([label for label, _ in CLOSURE._line_references("trainerbattle_double TRAINER_TEST, Intro, Lose, NotEnough")], ["Intro", "Lose", "NotEnough"])
        with self.assertRaisesRegex(CLOSURE.ClosureError, "malformed reference operand VAR_TEMP_0"):
            CLOSURE._line_references("trainerbattle_single TRAINER_TEST, Intro, VAR_TEMP_0")

    def test_rejects_unknown_external_kind(self):
        manifest = self.manifest()
        path = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        manifest["script_modules"]["environment"]["allowed_externals"] = [{
            "label": "ReviewedButInvalid", "kind": "made_up_kind",
            "path": "data/scripts/wayfarer_sevii/environment.inc",
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        }]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "unsupported kind made_up_kind"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_external_source_path_escape(self):
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["allowed_externals"] = [{
            "label": "Escaped", "kind": "script_symbol", "path": "../outside.inc", "sha256": "0" * 64,
        }]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "must be a root-relative path"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_external_source_symlink_escape(self):
        with tempfile.TemporaryDirectory() as outside_directory:
            outside = Path(outside_directory) / "outside.inc"
            outside.write_text("Escaped::\n\tend\n")
            link = self.root / "data/escape.inc"
            link.symlink_to(outside)
            manifest = self.manifest()
            manifest["script_modules"]["environment"]["allowed_externals"] = [{
                "label": "Escaped", "kind": "script_symbol", "path": "data/escape.inc",
                "sha256": hashlib.sha256(outside.read_bytes()).hexdigest(),
            }]
            with self.assertRaisesRegex(CLOSURE.ClosureError, "resolves outside the game root"):
                CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_source_handler_body_drift(self):
        source = self.root / "data/maps/OneIsland_Frlg/scripts.inc"
        source.write_text(source.read_text().replace("\tend\n", "\tnop\n\tend\n"))
        with self.assertRaisesRegex(CLOSURE.ClosureError, "source script file drifted"):
            CLOSURE.build_script_closure(self.root, self.manifest())

    def test_rejects_unreviewed_source_include(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text('.include "data/maps/OneIsland_Frlg/scripts.inc"\nWayfarerSevii_OneIsland_OnLoad::\n\tend\n')
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["WayfarerSevii_OneIsland_OnLoad"]
        manifest["script_modules"]["environment"]["allowed_commands"] = ["end"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "must stay under"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_unavailable_reference(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text("WayfarerSevii_OneIsland_OnLoad::\n\tcall MissingEntry\n\tend\n")
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["WayfarerSevii_OneIsland_OnLoad"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "unresolved or unreviewed dependency MissingEntry"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_unpinned_external_dependency(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text("WayfarerSevii_OneIsland_OnLoad::\n\tspecial ReviewedSpecial\n\tend\n")
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["WayfarerSevii_OneIsland_OnLoad"]
        manifest["script_modules"]["environment"]["allowed_commands"] = ["special", "end"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "unresolved or unreviewed dependency ReviewedSpecial"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_raw_or_non_sevii_persistent_state(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text("WayfarerSevii_OneIsland_OnLoad::\n\tsetflag FLAG_SYS_GAME_CLEAR\n\tend\n")
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["WayfarerSevii_OneIsland_OnLoad"]
        manifest["script_modules"]["environment"]["allowed_commands"] = ["setflag", "end"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "non-Sevii flag FLAG_SYS_GAME_CLEAR"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_module_outside_its_owner_directory(self):
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["owner"] = "story"
        with self.assertRaisesRegex(CLOSURE.ClosureError, "must stay under data/scripts/wayfarer_sevii/story/"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_recursive_include_from_another_owner_directory(self):
        story = self.root / "data/scripts/wayfarer_sevii/story/main.inc"
        story.parent.mkdir(parents=True)
        story.write_text('.include "data/scripts/wayfarer_sevii/trainers/helper.inc"\nWayfarerSevii_Story_Main::\n\tend\n')
        helper = self.root / "data/scripts/wayfarer_sevii/trainers/helper.inc"
        helper.parent.mkdir(parents=True)
        helper.write_text("WayfarerSevii_Trainer_Helper::\n\tend\n")
        manifest = {"script_modules": {"story": {
            "owner": "story", "include": "data/scripts/wayfarer_sevii/story/main.inc",
            "exports": ["WayfarerSevii_Story_Main"], "allowed_commands": ["end"], "allowed_externals": [],
        }}, "maps": []}
        with self.assertRaisesRegex(CLOSURE.ClosureError, "must stay under data/scripts/wayfarer_sevii/story/"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_unreviewed_file_directive_and_label_expression(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text('WayfarerSevii_OneIsland_OnLoad::\n\t.incbin "unreviewed.bin"\n\tend\n')
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["WayfarerSevii_OneIsland_OnLoad"]
        manifest["script_modules"]["environment"]["allowed_commands"] = ["end"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "unreviewed file-bearing directive"):
            CLOSURE.build_script_closure(self.root, manifest)
        module.write_text("WayfarerSevii_OneIsland_OnLoad::\n\t.4byte WayfarerSevii_OneIsland_OnLoad + 4\n\tend\n")
        with self.assertRaisesRegex(CLOSURE.ClosureError, "unsupported assembler directive"):
            CLOSURE.build_script_closure(self.root, manifest)

    def test_rejects_non_transient_equ_state_alias(self):
        module = self.root / "data/scripts/wayfarer_sevii/environment.inc"
        module.write_text(".equ STORY_CLEAR, FLAG_SYS_GAME_CLEAR\nWayfarerSevii_OneIsland_OnLoad::\n\tcheckflag STORY_CLEAR\n\tend\n")
        manifest = self.manifest()
        manifest["script_modules"]["environment"]["exports"] = ["WayfarerSevii_OneIsland_OnLoad"]
        manifest["script_modules"]["environment"]["allowed_commands"] = ["checkflag", "end"]
        with self.assertRaisesRegex(CLOSURE.ClosureError, "unsupported assembler directive"):
            CLOSURE.build_script_closure(self.root, manifest)


if __name__ == "__main__":
    unittest.main()
