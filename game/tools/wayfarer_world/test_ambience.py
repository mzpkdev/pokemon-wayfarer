"""Tests for the notable-ambience data (ambience.json) and its tables.

Needs no map data: the loader reads routines.json, the catalog and the
species info headers only.
"""

import copy
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))

import ambience  # noqa: E402
import emit  # noqa: E402
import maps  # noqa: E402
from maps import load_json  # noqa: E402

ROOT = maps.DEFAULT_ROOT
SPEC = ROOT.parent / ".product/specs/notable-ambience.md"
DATA = load_json(ambience.TOOL_DIR / "ambience.json")
CATALOG = load_json(ROOT / "tools/notable_trainers/catalog.json")["trainers"]
_SPRITES = []


def sprites():
    if not _SPRITES:
        _SPRITES.append(ambience.SpeciesSprites(ROOT))
    return _SPRITES[0]


def load(data=None, catalog=None, sprite_table=None):
    return ambience.Ambience(ROOT, ambience.sim_slugs(), catalog or CATALOG,
                             sprites=sprite_table or sprites(),
                             data=DATA if data is None else data)


def beat(data, beat_id):
    return next(b for b in data["beats"] if b["id"] == beat_id)


class FakeSprites:
    """The real sprite table with some species overridden."""

    def __init__(self, overrides):
        self.overrides = overrides

    def sprite(self, const):
        if const in self.overrides:
            size = self.overrides[const]
            return (size, None) if size else (None, "no sprite (test)")
        return sprites().sprite(const)


class LoadTest(unittest.TestCase):
    def test_loads_clean(self):
        amb = load()
        self.assertEqual(amb.problems, [])
        self.assertEqual(amb.warnings, [])
        self.assertEqual(len(amb.trainers), 25)

    def test_pool_matches_the_spec(self):
        """Every row of the spec's pool tables, in order, with its class."""
        amb = load()
        got = [(b.id, b.cls) for b in amb.beats]
        if SPEC.exists():
            text = SPEC.read_text(encoding="utf-8")
            pool = text[text.index("### The pool"):text.index("### Companion")]
            want = re.findall(r"^\| `(\w+)` \| (\w+) \|", pool, re.M)
            self.assertEqual(got, want)
        self.assertEqual(len(got), 35)
        self.assertLessEqual(len(got), ambience.MAX_BEATS)
        self.assertEqual(len({i for i, _ in got}), len(got))

    def test_preferred_beats_resolve(self):
        amb = load()
        preferred = {t.slug: t.preferred.id for t in amb.trainers if t.preferred}
        self.assertEqual(preferred, {
            "misty": "admire_water", "lt-surge": "push_ups", "erika": "doze", "whitney": "hum",
            "morty": "meditate", "chuck": "push_ups", "bugsy": "study_ground",
            "wallace": "flourish", "steven": "study_ground", "blue": "swagger"})
        for t in amb.trainers:
            if t.preferred:
                self.assertIs(amb.beats[t.preferred.index], t.preferred)

    def test_tags_one_to_three(self):
        for t in load().trainers:
            self.assertTrue(1 <= len(t.tags) <= 3, t.slug)

    def test_flags_and_derived_companion_need(self):
        amb = load()
        by = {b.id: b for b in amb.beats}
        self.assertIn("keep_walking", by["hum"].flags)
        self.assertNotIn("keep_walking", by["stretch"].flags)
        for k in ("greet_colleague", "greet_friend", "greet_family", "size_up"):
            self.assertIn("once_per_pair", by[k].flags)
        self.assertIn("once_per_approach", by["notice_player"].flags)
        self.assertIn("once_per_episode", by["player_lingers"].flags)
        self.assertEqual({b.id for b in amb.beats if "needs_companion" in b.flags},
                         {"ace_play", "ace_spar"})

    def test_steps_return_to_the_tile(self):
        self.assertIsNone(ambience.check_tile(
            [ambience.parse_step(s) for s in beat(DATA, "water_bite")["steps"]]))
        self.assertIsNone(ambience.check_tile(
            [ambience.parse_step(s) for s in beat(DATA, "quick_hop")["steps"]]))


class RelationTest(unittest.TestCase):
    def setUp(self):
        self.amb = load()

    def test_authored_and_derived(self):
        rel = self.amb.relation
        self.assertEqual(rel("steven", "wallace"), "friend")    # authored beats rival
        self.assertEqual(rel("lance", "clair"), "family")
        self.assertEqual(rel("brock", "misty"), "friend")       # authored beats colleague
        self.assertEqual(rel("juan", "wallace"), "friend")
        self.assertEqual(rel("blue", "lance"), "rival")         # rival beats colleague
        self.assertEqual(rel("blue", "steven"), "rival")
        self.assertEqual(rel("lt-surge", "erika"), "colleague")  # Kanto Gym Leaders
        self.assertEqual(rel("falkner", "pryce"), "colleague")   # Johto Gym Leaders
        self.assertEqual(rel("will", "lance"), "colleague")      # Indigo Elite Four and Champion
        self.assertEqual(rel("lorelei", "karen"), "colleague")   # Kanto's and Johto's Elite Four share Indigo
        self.assertEqual(rel("brock", "falkner"), "none")        # leaders of different regions
        self.assertEqual(rel("norman", "wallace"), "none")       # a leader and a Champion
        self.assertEqual(rel("will", "steven"), "none")          # different Leagues
        self.assertEqual(rel("misty", "brock"), rel("brock", "misty"))

    def test_rivals_are_the_four_champions(self):
        rivals = {frozenset(p["pair"]) for p in self.amb.pairs() if p["relation"] == "rival"}
        champs = ambience.CHAMPIONS
        want = {frozenset((a, b)) for a in champs for b in champs if a != b} - {frozenset(("steven", "wallace"))}
        self.assertEqual(rivals, want)

    def test_matrix_symmetric_with_empty_diagonal(self):
        m = self.amb.relations
        for i in range(25):
            self.assertEqual(m[i][i], 0)
            for j in range(25):
                self.assertEqual(m[i][j], m[j][i])


class CompanionTest(unittest.TestCase):
    def test_authored_first_then_aces(self):
        amb = load()
        by = {t.slug: t for t in amb.trainers}
        self.assertEqual([c for c, _ in by["brock"].companions],
                         ["SPECIES_ONIX", "SPECIES_STEELIX", "SPECIES_AERODACTYL"])
        self.assertEqual(by["misty"].companions[0][0], "SPECIES_PSYDUCK")
        self.assertEqual(len(by["misty"].companions), ambience.COMPANION_MAX)

    def test_first_ace_without_authored_companion(self):
        by = {t.slug: t for t in load().trainers}
        self.assertEqual(by["lance"].companions[0][0], "SPECIES_DRAGONITE")
        self.assertEqual(by["lt-surge"].companions[0][0], "SPECIES_RAICHU")
        # Lance's roster lists Dragonite twice: de-duplicated.
        self.assertEqual([c for c, _ in by["lance"].companions], ["SPECIES_DRAGONITE", "SPECIES_CHARIZARD"])

    def test_big_sprite_flagged(self):
        amb = load(sprite_table=FakeSprites({"SPECIES_STEELIX": "64x64"}))
        brock = amb.trainers[0]
        self.assertEqual(brock.companions[:2], [("SPECIES_ONIX", False), ("SPECIES_STEELIX", True)])
        text = emit.render_ambience(amb)
        self.assertIn("SPECIES_STEELIX | AMBIENCE_COMPANION_BIG", text)

    def test_real_big_sprite_detected(self):
        self.assertEqual(sprites().sprite("SPECIES_LUGIA")[0], "64x64")
        self.assertEqual(sprites().sprite("SPECIES_ONIX")[0], "32x32")
        size, why = sprites().sprite("SPECIES_STEELIX_MEGA")
        self.assertIsNone(size)
        self.assertIn("disabled by the config", why)

    def test_ace_without_sprite_dropped_with_warning(self):
        amb = load(sprite_table=FakeSprites({"SPECIES_STEELIX": None}))
        self.assertEqual(amb.problems, [])
        self.assertEqual([c for c, _ in amb.trainers[0].companions], ["SPECIES_ONIX", "SPECIES_AERODACTYL"])
        self.assertTrue(any("Steelix" in w for w in amb.warnings))


class ValidationTest(unittest.TestCase):
    """Each rule of the spec's "Validation fails the build" list, and the
    table limits, on a mutated copy of ambience.json."""

    def fails(self, mutate, pattern, catalog=None, sprite_table=None):
        data = copy.deepcopy(DATA)
        mutate(data)
        amb = load(data, catalog=catalog, sprite_table=sprite_table)
        self.assertTrue(amb.problems, "no problem reported")
        with self.assertRaises(ambience.BuildError):
            amb.check()
        self.assertTrue(any(re.search(pattern, p) for p in amb.problems), amb.problems)

    def set_steps(self, beat_id, steps):
        return lambda d: beat(d, beat_id).__setitem__("steps", steps)

    def test_unknown_primitive(self):
        self.fails(self.set_steps("hum", ["dance"]), r"unknown primitive 'dance'")

    def test_unknown_icon(self):
        self.fails(self.set_steps("water_wait", ["emote sparkles", "wait 8"]), r"unknown icon 'sparkles'")
        self.fails(self.set_steps("slots_result", ["wait 4", "emote !!/Z"]), r"unknown icon 'Z'")

    def test_ellipsis_alias(self):
        data = copy.deepcopy(DATA)
        beat(data, "water_wait")["steps"] = ["emote ...", "wait 8"]
        self.assertEqual(load(data).problems, [])

    def test_unknown_effect(self):
        self.fails(self.set_steps("flourish", ["spin", "effect fire own"]), r"unknown effect 'fire'")
        self.fails(self.set_steps("flourish", ["spin", "effect sparkle behind"]), r"effect tile 'behind'")

    def test_unknown_spot_kind(self):
        self.fails(lambda d: beat(d, "grass_rustle").__setitem__("when", [{"spot": ["beach"]}]),
                   r"unknown spot kind 'beach'")

    def test_unknown_activity(self):
        self.fails(lambda d: beat(d, "doze").__setitem__("when", ["dwelling", {"activity": ["nap"]}]),
                   r"unknown activity 'nap'")
        self.fails(lambda d: beat(d, "take_in_view").__setitem__("when", [{"spot": ["named:nap"]}]),
                   r"unknown activity 'nap'")

    def test_unknown_tag(self):
        self.fails(lambda d: d["trainers"]["brock"].__setitem__("tags", ["grumpy"]), r"unknown tag 'grumpy'")
        self.fails(lambda d: beat(d, "hum").__setitem__("who", {"tags_any": ["grumpy"]}),
                   r"unknown tag 'grumpy'")

    def test_unknown_trainer(self):
        self.fails(lambda d: d["trainers"].__setitem__("red", {"tags": ["stoic"]}), r"'red': unknown trainer")
        # Bruno is in the catalog but does not walk.
        self.fails(lambda d: d["relationships"].append({"relation": "friend", "pair": ["bruno", "lance"]}),
                   r"unknown trainer 'bruno'")

    def test_unknown_relation(self):
        self.fails(lambda d: d["relationships"].append({"relation": "enemy", "pair": ["blue", "erika"]}),
                   r"unknown relation 'enemy'")
        self.fails(lambda d: beat(d, "size_up").__setitem__("when", [{"notable_within": 3, "relation": "enemy"}]),
                   r"unknown relation 'enemy'")

    def test_unknown_flag_class_and_term(self):
        self.fails(lambda d: beat(d, "hum").__setitem__("flags", ["loud"]), r"unknown flag 'loud'")
        self.fails(lambda d: beat(d, "hum").__setitem__("class", "urgent"), r"unknown class 'urgent'")
        self.fails(lambda d: beat(d, "hum").__setitem__("when", ["sleeping"]), r"unknown when term 'sleeping'")
        self.fails(lambda d: beat(d, "admire_water").__setitem__("when", [{"facing": "lava"}]),
                   r"unknown facing 'lava'")

    def test_too_many_steps(self):
        self.fails(self.set_steps("look_around", ["wait 1"] * 9), r"9 steps; a beat has 1 to 8")
        self.fails(self.set_steps("look_around", []), r"0 steps; a beat has 1 to 8")

    def test_step_leaves_the_tile(self):
        self.fails(self.set_steps("grass_chase", ["effect grass_shake ahead", "step ahead", "wait 2"]),
                   r"ends 1 tile from its start")
        self.fails(self.set_steps("grass_chase", ["step ahead", "step ahead", "step back", "step back"]),
                   r"more than 1 tile away")
        # Back after a turn to the player undoes the step, whichever way it went.
        data = copy.deepcopy(DATA)
        beat(data, "grass_chase")["steps"] = ["step ahead", "face player", "step back"]
        self.assertEqual(load(data).problems, [])
        # ...but "ahead" after a turn need not return.
        self.fails(self.set_steps("grass_chase", ["step back", "face player", "step ahead"]),
                   r"ends \d tiles? from its start|more than 1 tile away")

    def test_preferred_beat_trainer_cannot_qualify(self):
        self.fails(lambda d: d["trainers"]["misty"].__setitem__("preferred", "push_ups"),
                   r"'misty': can't qualify for preferred beat 'push_ups': needs one of the tags athletic")
        self.fails(lambda d: d["trainers"]["blaine"].__setitem__("preferred", "grass_chase"),
                   r"excluded by the tag elder")
        self.fails(lambda d: d["trainers"]["misty"].__setitem__("preferred", "nap_time"),
                   r"preferred beat 'nap_time' is not in the pool")

    def test_preferred_companion_beat_needs_a_companion(self):
        catalog = copy.deepcopy(CATALOG)
        for t in catalog:
            if t["slug"] == "lt-surge":
                for mon in t["roster"]:
                    mon["isAce"] = False
        self.fails(lambda d: d["trainers"]["lt-surge"].__setitem__("preferred", "ace_spar"),
                   r"'lt-surge': can't qualify for preferred beat 'ace_spar': needs a companion",
                   catalog=catalog)

    def test_trainer_with_no_tag(self):
        self.fails(lambda d: d["trainers"]["brock"].__setitem__("tags", []), r"'brock': no tag")
        self.fails(lambda d: d["trainers"].pop("karen"), r"'karen': no tag")
        self.fails(lambda d: d["trainers"]["brock"].__setitem__("tags", ["cheerful", "curious", "elder", "cocky"]),
                   r"4 tags, over 3")

    def test_relationship_names_a_trainer_twice(self):
        self.fails(lambda d: d["relationships"].append({"relation": "friend", "pair": ["misty", "brock"]}),
                   r"names misty and brock twice")
        self.fails(lambda d: d["relationships"].append({"relation": "friend", "pair": ["erika", "erika"]}),
                   r"names the trainer erika twice")

    def test_companion_beat_for_a_trainer_with_no_ace(self):
        catalog = copy.deepcopy(CATALOG)
        for t in catalog:
            if t["slug"] in ("brock", "blue"):
                for mon in t["roster"]:
                    mon["isAce"] = False

        def mutate(d):
            d["trainers"]["brock"].pop("companion")
        # ace_play admits everyone: Brock; ace_spar (athletic or cocky): Blue.
        self.fails(mutate, r"beat 'ace_play': companion beat for brock", catalog=catalog)
        self.fails(mutate, r"beat 'ace_spar': companion beat for blue", catalog=catalog)

    def test_companion_species_without_follower_sprite(self):
        self.fails(lambda d: d["trainers"]["brock"].__setitem__("companion", "STEELIX_MEGA"),
                   r"companion 'STEELIX_MEGA' has no overworld follower sprite: .*disabled by the config")
        self.fails(lambda d: d["trainers"]["brock"].__setitem__("companion", "MISSINGNO"),
                   r"companion 'MISSINGNO' has no overworld follower sprite: SPECIES_MISSINGNO is not")

    def test_duplicate_beat_id(self):
        self.fails(lambda d: d["beats"].append(copy.deepcopy(beat(d, "hum"))), r"'hum': duplicate id")

    def test_too_many_beats(self):
        def mutate(d):
            for k in range(ambience.MAX_BEATS + 1 - len(d["beats"])):
                extra = copy.deepcopy(beat(d, "hum"))
                extra["id"] = "hum_%d" % k
                d["beats"].append(extra)
        self.fails(mutate, r"over AMBIENCE_MAX_BEATS")

    def test_conflicting_parameters(self):
        self.fails(lambda d: beat(d, "swagger").__setitem__(
            "when", [{"player_within": 3}, {"any": ["dwelling", {"player_within": 4}]}]),
            r"conflicting player_within: 3 and 4")

    def test_companion_steps_need_the_companion_out(self):
        self.fails(self.set_steps("ace_play", ["companion_do jump", "emote happy"]),
                   r"while no companion is out")


class HeaderTest(unittest.TestCase):
    """The Python lists mirror include/constants/wayfarer_ambience.h: the
    relation matrix and preferred index are numeric in the tables, and the
    tile check reads the face targets by value."""

    def setUp(self):
        self.text = (ROOT / "include/constants/wayfarer_ambience.h").read_text(encoding="utf-8")

    def enum(self, name):
        body = re.search(r"enum %s\s*\{(.*?)\};" % name, self.text, re.S).group(1)
        out, value = {}, -1
        for m in re.finditer(r"^\s*(\w+)(?:\s*=\s*(\w+))?,", body, re.M):
            value = int(m.group(2), 0) if m.group(2) and m.group(2)[0].isdigit() else value + 1
            out[m.group(1)] = value
        return out

    def bits(self, prefix):
        return {m.group(1)[len(prefix):].lower(): int(m.group(2))
                for m in re.finditer(r"#define (%s\w+)\s+\(1 << (\d+)\)" % prefix, self.text)}

    def test_lists_match(self):
        def names(enum, prefix):
            return [k[len(prefix):].lower() for k, _ in sorted(self.enum(enum).items(), key=lambda kv: kv[1])
                    if not k.endswith("_COUNT")]
        self.assertEqual(names("AmbienceClass", "AMBIENCE_CLASS_"), ambience.CLASSES)
        self.assertEqual(names("AmbienceOp", "AMBIENCE_OP_"), ambience.OPS)
        self.assertEqual(names("AmbienceRelation", "AMBIENCE_RELATION_"), ambience.RELATIONS)
        self.assertEqual(names("AmbienceStep", "AMBIENCE_STEP_"), ambience.STEPS)
        self.assertEqual(names("AmbienceObject", "AMBIENCE_OBJECT_"), ambience.OBJECTS)
        self.assertEqual(names("AmbienceEffect", "AMBIENCE_EFFECT_"), ambience.EFFECTS)
        self.assertEqual(names("AmbienceWhere", "AMBIENCE_WHERE_"), ambience.WHERE)
        self.assertEqual(names("AmbienceCompanionAct", "AMBIENCE_COMPANION_ACT_"), ambience.COMPANION_ACTS)
        self.assertEqual([k.lower() for k in ambience.ICON_NAMES], names("AmbienceIcon", "AMBIENCE_ICON_"))
        self.assertEqual({k[len("AMBIENCE_TARGET_"):].lower(): v for k, v in self.enum("AmbienceTarget").items()},
                         ambience.TARGETS)
        self.assertEqual(self.bits("AMBIENCE_TAG_"), {t: i for i, t in enumerate(ambience.TAGS)})
        self.assertEqual(self.bits("AMBIENCE_FACT_"), {t: i for i, t in enumerate(ambience.FACTS)})
        self.assertEqual(self.bits("AMBIENCE_FLAG_"), {t: i for i, t in enumerate(ambience.FLAGS)})
        for name, value in (("AMBIENCE_MAX_STEPS", ambience.MAX_STEPS), ("AMBIENCE_MAX_BEATS", ambience.MAX_BEATS),
                            ("AMBIENCE_COMPANION_MAX", ambience.COMPANION_MAX),
                            ("AMBIENCE_DEFAULT_COOLDOWN", ambience.DEFAULT_COOLDOWN)):
            self.assertRegex(self.text, r"#define %s\s+%d\b" % (name, value))

    @unittest.skipUnless(shutil.which(os.environ.get("HOSTCC", "cc")), "needs a host C compiler")
    def test_tables_compile_on_the_host(self):
        """The rendered tables against the real headers, sizes asserted."""
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "ambience.c"
            source.write_text('#include "gba/types.h"\n#include "constants/global.h"\n'
                              + emit.render_ambience(load()) + "\n", encoding="utf-8")
            result = subprocess.run([os.environ.get("HOSTCC", "cc"), "-std=gnu11", "-fsyntax-only", "-Wall",
                                     "-Werror", "-DPOKEMON_WAYFARER", "-iquote", str(ROOT / "include"),
                                     str(source)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)


class ReportTest(unittest.TestCase):
    def test_report_section(self):
        amb = load()
        rep = emit.ambience_report(amb)
        self.assertEqual(rep["beats"][0], "look_around")
        self.assertEqual(len(rep["beats"]), len(amb.beats))
        self.assertEqual(rep["trainers"][0]["companions"][0], {"species": "SPECIES_ONIX", "big": False})
        self.assertEqual(rep["rom_bytes"]["total"], sum(v for k, v in rep["rom_bytes"].items() if k != "total"))
        self.assertIn({"pair": ["lance", "clair"], "relation": "family", "authored": True}, rep["relations"])


if __name__ == "__main__":
    unittest.main()
