"""Guard: a notable trainer introduces themselves only through the shared introduction.

A text that has a notable trainer say "I'm X", "I am X" or "My name is X" must be the first-meeting text of a
`notable_introduce` call (see data/scripts/wayfarer_notable_talk.inc and
.product/specs/notable-story-continuity.md#2-introductions), or sit on the allow-list below. Otherwise a trainer
could introduce themselves to a player they have already met.
"""

import json
import re
import unittest
from pathlib import Path

GAME = Path(__file__).resolve().parents[2]
CATALOG = GAME / "tools/notable_trainers/catalog.json"
SCRIPT_ROOTS = [GAME / "data"]

# Map directories, files and labels that say "I am X" outside the shared introduction, each with its reason.
# Keep this short: a scene that plays in Wayfarer converts to `notable_introduce` instead.
ALLOWED = {
    # FRLG and HNS League rooms. Wayfarer's League runs through the circuit scripts (wayfarer_indigo_league.inc,
    # wayfarer_masters_league.inc, league_circuit.inc), so these rooms' own intros never play there.
    "PokemonLeague_BrunosRoom_Frlg": "FRLG league room, replaced by the circuit in Wayfarer",
    "PokemonLeague_LoreleisRoom_Frlg": "FRLG league room, replaced by the circuit in Wayfarer",
    "PokemonLeague_AgathasRoom_Frlg": "FRLG league room, replaced by the circuit in Wayfarer",
    "PokemonLeague_KarensRoom_hns": "HNS league room, replaced by the circuit in Wayfarer",
    "PokemonLeague_WillsRoom_hns": "HNS league room, replaced by the circuit in Wayfarer",
    "PokemonLeague_BrunosRoom_hns": "HNS league room, replaced by the circuit in Wayfarer",
    "PokemonLeague_KogasRoom_hns": "HNS league room, replaced by the circuit in Wayfarer",
    "EverGrandeCity_SidneysRoom": "Hoenn league room, replaced by the circuit in Wayfarer",
    "EverGrandeCity_PhoebesRoom": "Hoenn league room, replaced by the circuit in Wayfarer",
    "EverGrandeCity_GlaciasRoom": "Hoenn league room, replaced by the circuit in Wayfarer",
    "EverGrandeCity_DrakesRoom": "Hoenn league room, replaced by the circuit in Wayfarer",
    # FRLG Cinnabar Gym: Wayfarer runs wayfarer_cinnabar_full_scripts.inc instead.
    "CinnabarIsland_Gym_Frlg": "FRLG Cinnabar Gym, replaced in Wayfarer",
    # BLUE at Cinnabar is `#if !IS_WAYFARER`: it never plays in Wayfarer.
    "CinnabarIsland_Text_BlueBefore": "HNS-only scene, not built in Wayfarer",
    "CinnabarIsland_Text_Blue": "HNS-only scene, not built in Wayfarer",
    # Janine's Gym trainers pose as her (ninja decoys); the "I'm JANINE" is the disguise, not an introduction.
    "FuchsiaCity_Gym_Text_PicnickerCindy_Before": "a decoy claims to be her",
    "FuchsiaCity_Gym_Text_CamperBarry_Before": "a decoy claims to be her",
    "FuchsiaCity_Gym_Text_LassAlice_Before": "a decoy claims to be her",
    # FRLG Pewter Gym: Wayfarer's Pewter is PewterCity_Gym_hns, converted.
    "PewterCity_Gym_Frlg": "FRLG Pewter Gym, not in Wayfarer",
    # ROXANNE's rematch lines only play after the battle that first introduced her.
    "RustboroCity_Gym_Text_RoxannePreRematch": "a rematch follows a battle, which introduces",
    "RustboroCity_Gym_Text_RoxanneRematchNeedTwoMons": "a rematch follows a battle, which introduces",
    # BLUE's Viridian intro is `#if !IS_WAYFARER` (Giovanni holds the Gym there in Wayfarer).
    "ViridianCity_Text_BlueIntro": "HNS-only scene, not built in Wayfarer",
    "FuchsiaCity_PokemonCenter1F_Text_JanineImpersonator1": "an impersonator claims to be her",
    # Fame checker entries are a trainer card's own blurb, not a scene.
    "data/text/fame_checker_frlg.inc": "FRLG fame checker text, not in Wayfarer",
}

LABEL = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)::?\s*(?:@.*)?$")
STRING = re.compile(r'^\s*\.string "(.*)"\s*$')
INTRO = re.compile(r"^\s*notable_introduce\s+(NOTABLE_TRAINER_\w+)\s*,\s*(\w+)", re.M)
SHARED_INTRO = re.compile(r"loadword 0, (\w+)\n\tcall NotableStory_EventScript_Introduce")
NAMES = None


def names():
    global NAMES
    if NAMES is None:
        NAMES = sorted({t["name"] for t in json.loads(CATALOG.read_text())["trainers"]}, key=len, reverse=True)
    return NAMES


def self_introduction():
    # "I'm WHITNEY", "I am LANCE", "My name is Steven", "Name's BLUE" (a game text is upper case, a few are not).
    alternatives = "|".join(re.escape(n) for n in names())
    return re.compile(rf"\b(?:I'm|I am|My name is|My name's|Name's)\s+(?:{alternatives})\b(?![a-z]|'s)", re.I)


def plain(text):
    return re.sub(r"\\[npl]", " ", text)


def inc_files():
    for root in SCRIPT_ROOTS:
        yield from sorted(root.rglob("*.inc"))


def texts(path):
    """(label, text) for every label of a file that is followed by .string lines. Text control codes (\\n, \\p,
    \\l) read as spaces."""
    label, lines = None, []
    for raw in path.read_text(errors="replace").splitlines():
        match = LABEL.match(raw)
        if match:
            if label and lines:
                yield label, "".join(lines)
            label, lines = match.group(1), []
            continue
        match = STRING.match(raw)
        if match and label:
            lines.append(match.group(1) + " ")
        elif label and lines and raw.strip() and not raw.lstrip().startswith(("@", "#")):
            yield label, "".join(lines)
            label, lines = None, []
    if label and lines:
        yield label, "".join(lines)


def introduced_labels():
    labels = set()
    for path in inc_files():
        body = path.read_text(errors="replace")
        labels.update(m.group(2) for m in INTRO.finditer(body))
        labels.update(SHARED_INTRO.findall(body))
    return labels


def allowed(path, label):
    return label in ALLOWED or path.parent.name in ALLOWED or str(path.relative_to(GAME)) in ALLOWED


class StoryIntroductionGuard(unittest.TestCase):
    def test_self_introductions_go_through_the_shared_introduction(self):
        pattern, known = self_introduction(), introduced_labels()
        offenders = []
        for path in inc_files():
            for label, text in texts(path):
                if pattern.search(plain(text)) and label not in known and not allowed(path, label):
                    offenders.append(f"{path.relative_to(GAME)}: {label}: {pattern.search(plain(text)).group(0)}")
        self.maxDiff = None
        self.assertEqual(offenders, [], "self-introductions outside notable_introduce, see the module docstring")

    def test_the_guard_sees_an_introduction(self):
        pattern = self_introduction()
        self.assertTrue(pattern.search("Hi! I'm WHITNEY!"))
        self.assertTrue(pattern.search("My name is Steven."))
        self.assertTrue(pattern.search("I am LORELEI of the ELITE FOUR."))
        self.assertFalse(pattern.search("I'm LANCEr"))
        self.assertFalse(pattern.search("I'm sure WHITNEY is fine."))

    def test_each_introduction_text_exists_and_names_no_other_trainer(self):
        by_label = {}
        for path in inc_files():
            by_label.update(dict(texts(path)))
        pattern = self_introduction()
        for path in inc_files():
            for character, label in INTRO.findall(path.read_text(errors="replace")):
                self.assertIn(label, by_label, f"{path.name}: {label} has no text")
                for match in pattern.finditer(plain(by_label[label])):
                    self.assertIn(character.removeprefix("NOTABLE_TRAINER_").replace("_", " ").split()[0].lower(),
                                  match.group(0).lower(), f"{label} introduces someone else than {character}")


if __name__ == "__main__":
    unittest.main()
