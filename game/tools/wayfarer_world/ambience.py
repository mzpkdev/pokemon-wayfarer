"""Notable ambience: the beat pool, tags, relations and companions.

Reads ambience.json (.product/specs/notable-ambience.md, "Data"), the
simulated trainers (routines.json, in catalog order: the sim slot), the
notable-trainer catalog (roles, home regions, roster aces) and the species
info headers (each species' overworld follower sprite), validates it all,
and resolves what emit.render_ambience writes. Needs no map data.

The lists below mirror include/constants/wayfarer_ambience.h in order; the
emitted tables name the C constants, so a renamed constant fails the build.
"""

import itertools
import json
import re
from pathlib import Path

from authored import ACTIVITIES
from maps import BuildError, TOOL_DIR, load_json, read_text
from spots import KIND_KEYS, NAMED

MAX_STEPS, MAX_BEATS, COMPANION_MAX = 8, 40, 4
DEFAULT_COOLDOWN = 80
PREFERRED_NONE = 0xFF
COMPANION_BIG = 1 << 15

CLASSES = ["idle", "transition", "react"]
TAGS = ["athletic", "cheerful", "curious", "dreamy", "mystic", "elegant", "water", "elder",
        "nimble", "cocky", "stoic"]
RELATIONS = ["none", "family", "friend", "rival", "colleague"]
AUTHORED_RELATIONS = ("family", "friend")
FLAGS = ["keep_walking", "once_per_approach", "once_per_pair", "once_per_episode",
         "needs_companion"]
AUTHORED_FLAGS = FLAGS[:4]
FACTS = ["walking", "dwelling", "arriving", "leaving", "blocked", "outdoors", "companion_room",
         "facing_water", "facing_grass", "facing_npc", "facing_counter", "spot", "not_spot",
         "activity", "player_within", "player_adjacent", "notable"]
SIMPLE_TERMS = FACTS[:7]
FACING = ["water", "grass", "npc", "counter"]
OPS = ["face", "look_back", "wait", "step", "jump", "spin", "bow", "turn", "emote",
       "emote_parity", "effect", "companion_out", "companion_in", "companion_do"]
TARGETS = {"down": 1, "up": 2, "left": 3, "right": 4, "player": 5, "other": 6, "target": 7,
           "away": 8, "companion": 9}
DIRECTIONS = {1: (0, 1), 2: (0, -1), 3: (-1, 0), 4: (1, 0)}  # DIR_SOUTH/NORTH/WEST/EAST
STEPS = ["ahead", "back", "left", "right"]
OBJECTS = ["nurse", "npc"]
ICONS = ["!", "!!", "?", "X", "heart", "…", "happy", "music", "love", "curious", "pensive",
         "sad", "angry", "surprise"]
ICON_NAMES = ["EXCLAMATION", "DOUBLE_EXCL", "QUESTION", "X", "HEART", "ELLIPSIS", "HAPPY",
              "MUSIC", "LOVE", "CURIOUS", "PENSIVE", "SAD", "ANGRY", "SURPRISE"]
ICON_ALIASES = {"...": "…"}
EFFECTS = ["ripple", "splash", "grass_shake", "dust", "sparkle"]
WHERE = ["ahead", "own"]
COMPANION_ACTS = ["face", "jump"]
SPOT_KINDS = [k for k in KIND_KEYS if KIND_KEYS.index(k) != NAMED]
TOP_KEYS = {"_comment", "trainers", "relationships", "beats"}
BEAT_KEYS = {"id", "class", "when", "who", "flags", "cooldown", "steps", "note"}
WHO_KEYS = {"tags_any", "tags_none", "companion"}
TRAINER_KEYS = {"tags", "preferred", "companion", "note"}
CHAMPIONS = {"blue", "lance", "wallace", "steven"}

# Wait and parameter bounds: the ROM fields are u8.
PARAM_MAX = 255


# A reaction limit needs the `when` term (a must, in the top-level list) its
# latch follows: the partner a pair is greeted by, the range an approach is
# cleared by leaving, the standing an episode ends with.
LIMIT_TERMS = {"once_per_pair": ("notable", "notable_within"),
               "once_per_approach": ("player_within", "player_within"),
               "once_per_episode": ("player_adjacent", "player_adjacent_ticks")}


def load_data(path):
    """ambience.json -> (data, duplicate-key problems). JSON allows a key
    twice in one object and keeps the last; here that is a mistake."""
    duplicates = []

    def pairs(items):
        out, twice = {}, []
        for key, value in items:
            if key in out:
                twice.append(key)
            out[key] = value
        where = " in beat %r" % out["id"] if isinstance(out.get("id"), str) else ""
        duplicates.extend("duplicate key %r%s" % (key, where) for key in twice)
        return out
    data = json.loads(Path(path).read_text(encoding="utf-8"), object_pairs_hook=pairs)
    return data, duplicates


def activity_key(name):
    """'lie low' and 'lie_low' both name WORLD_ACTIVITY_LIE_LOW."""
    return name.replace("_", " ") if isinstance(name, str) else name


ACTIVITY_INDEX = {a: i for i, a in enumerate(ACTIVITIES)}


# --------------------------------------------------------------------------
# Species: constants and overworld follower sprites.
# --------------------------------------------------------------------------

def species_const(name):
    """A catalog or authored species name as its SPECIES_* constant:
    'Mr. Mime' -> SPECIES_MR_MIME, 'Nidoran♀' -> SPECIES_NIDORAN_F."""
    text = name.strip().upper().replace("♀", "_F").replace("♂", "_M")
    text = text.replace("'", "").replace(".", "").replace("É", "E")
    text = re.sub(r"[^A-Z0-9]+", "_", text).strip("_")
    if text.startswith("SPECIES_"):
        text = text[len("SPECIES_"):]
    return "SPECIES_" + text


CONFIG_HEADERS = ("include/gba/defines.h", "include/config/general.h",
                  "include/config/species_enabled.h", "include/config/pokemon.h",
                  "include/config/overworld.h")
SPECIES_INFO_DIR = "src/data/pokemon/species_info"


class Unresolved(Exception):
    pass


class Preprocessor:
    """Just enough of the C preprocessor to evaluate the species info
    headers' #if guards: object-like #defines from the config headers
    (the last definition wins; the test-build overrides in config/test.h
    are not read)."""

    def __init__(self, root):
        self.defines = {}
        for rel in CONFIG_HEADERS:
            path = Path(root) / rel
            if not path.exists():
                continue
            for line in read_text(path).splitlines():
                m = re.match(r"\s*#\s*define\s+(\w+)(?!\()\s*(.*)$", line)
                if m:
                    self.defines[m.group(1)] = re.sub(r"//.*|/\*.*?\*/", "", m.group(2)).strip()

    def value(self, expr, depth=0):
        if depth > 16:
            raise Unresolved(expr)
        expr = re.sub(r"//.*|/\*.*?\*/", "", expr).strip()
        expr = re.sub(r"defined\s*\(\s*(\w+)\s*\)|defined\s+(\w+)",
                      lambda m: "1" if (m.group(1) or m.group(2)) in self.defines else "0", expr)

        def ident(m):
            word = m.group(0)
            if word in ("and", "or", "not"):
                return word
            if word not in self.defines:
                raise Unresolved(word)
            return "(%d)" % self.value(self.defines[word] or "1", depth + 1)
        expr = expr.replace("&&", " and ").replace("||", " or ")
        expr = re.sub(r"!(?!=)", " not ", expr)
        expr = re.sub(r"\b[A-Za-z_]\w*\b", ident, expr)
        expr = re.sub(r"\b(\d+)[uUlL]+\b", r"\1", expr)
        try:
            return int(eval(expr, {"__builtins__": {}}, {}))  # noqa: S307 - config arithmetic only
        except Exception as err:  # noqa: BLE001
            raise Unresolved("%s (%s)" % (expr, err))


class SpeciesSprites:
    """SPECIES_* -> follower sprite size ('32x32', '64x64') or None.

    Finds each `[SPECIES_X] =` block in the species info headers and its
    OVERWORLD(pic, SIZE_..., ...) entry, honouring the #if guards around
    both: a block behind a disabled config (P_FAMILY_*, P_*_FORMS) has no
    sprite, and the reason says so. A block defined through a macro
    (`[SPECIES_X] = SOME_INFO(...)`) is not followed; it reads as no sprite."""

    def __init__(self, root):
        self.root = Path(root)
        self.pp = Preprocessor(root)
        # SPECIES_* -> its value; an alias (SPECIES_AEGISLASH ->
        # SPECIES_AEGISLASH_SHIELD) shares its target's block.
        self.known = dict(re.findall(r"#define\s+(SPECIES_\w+)\s+(\w+)",
                                     read_text(self.root / "include/constants/species.h")))
        self.info = {}  # SPECIES_X -> (size or None, reason)
        self.objects = self._enabled("OW_POKEMON_OBJECT_EVENTS")
        for path in sorted((self.root / SPECIES_INFO_DIR).glob("gen_*_families.h")):
            self._scan(path)

    def _enabled(self, name):
        try:
            return bool(self.pp.value(name))
        except Unresolved:
            return False

    def _scan(self, path):
        stack = []  # [active, taken, reason]
        current, reason_at_start, in_macro = None, None, False
        lines = read_text(path).splitlines()
        rel = path.relative_to(self.root)
        i = 0
        while i < len(lines):
            line = lines[i]
            d = re.match(r"\s*#\s*(ifdef|ifndef|if|elif|else|endif)\b\s*(.*)$", line)
            if d:
                kind, expr = d.group(1), re.sub(r"//.*", "", d.group(2)).strip()
                if kind in ("if", "ifdef", "ifndef"):
                    ok, why = self._test(kind, expr)
                    stack.append([ok, ok, None if ok else why])
                elif kind == "elif" and stack:
                    ok, why = self._test("if", expr)
                    top = stack[-1]
                    top[0] = ok and not top[1]
                    top[1] = top[1] or ok
                    top[2] = None if top[0] else why
                elif kind == "else" and stack:
                    top = stack[-1]
                    top[0] = not top[1]
                    top[1] = True
                    top[2] = None if top[0] else "#else of a taken branch"
                elif kind == "endif" and stack:
                    stack.pop()
                i += 1
                continue
            if in_macro or re.match(r"\s*#\s*define\b", line):
                # A macro body (species info templates) is not a block.
                in_macro = line.rstrip().endswith("\\")
                current = None
                i += 1
                continue
            off = next((s[2] for s in stack if not s[0]), None)
            m = re.match(r"\s*\[(SPECIES_\w+)\]\s*=\s*(\w+\s*\()?", line)
            if m:
                current = m.group(1)
                if m.group(2):
                    self.info.setdefault(current, (None, "%s:%d: defined through the macro %s"
                                                   % (rel, i + 1, m.group(2).rstrip("( "))))
                    current = None
                else:
                    reason_at_start = off
                    if current not in self.info:
                        self.info[current] = (None, "%s:%d: no OVERWORLD entry" % (rel, i + 1)
                                              if off is None else
                                              "%s:%d: disabled by the config (%s)" % (rel, i + 1, off))
                i += 1
                continue
            o = re.match(r"\s*OVERWORLD\s*\(", line)
            if o and current is not None:
                text = " ".join(lines[i:i + 8])
                size = re.search(r"OVERWORLD\s*\(\s*\w+\s*,\s*SIZE_(32x32|64x64)\b", text)
                why = reason_at_start or off
                found = self.info.get(current, (None, None))[0] is not None
                if why is not None:
                    # An #if/#else alternative: never hide the enabled one.
                    if not found:
                        self.info[current] = (None, "%s:%d: disabled by the config (%s)"
                                              % (rel, i + 1, why))
                elif not self.objects:
                    self.info[current] = (None, "OW_POKEMON_OBJECT_EVENTS is off")
                elif size:
                    self.info[current] = (size.group(1), None)
                current = None
            i += 1

    def _test(self, kind, expr):
        if kind in ("ifdef", "ifndef"):
            ok = expr.split()[0] in self.pp.defines if expr else False
            ok = ok if kind == "ifdef" else not ok
            return ok, None if ok else "#%s %s" % (kind, expr)
        try:
            ok = bool(self.pp.value(expr))
            return ok, None if ok else expr
        except Unresolved as err:
            return False, "%s: unresolved %s" % (expr, err)

    def sprite(self, const):
        """(size or None, reason)."""
        if const not in self.known:
            return None, "%s is not a species constant" % const
        seen = {const}
        while const not in self.info and self.known.get(const) in self.known:
            const = self.known[const]
            if const in seen:
                break
            seen.add(const)
        return self.info.get(const, (None, "no species info block"))


# --------------------------------------------------------------------------
# Steps and context.
# --------------------------------------------------------------------------

class Step:
    def __init__(self, op, arg=0, arg_name="0", text=""):
        self.op, self.arg, self.arg_name, self.text = op, arg, arg_name, text


def parse_step(text):
    """One step string -> Step, or raise BuildError (unknown primitive,
    icon, effect, target...)."""
    if not isinstance(text, str):
        raise BuildError("step %r is not a string" % (text,))
    words = text.split()
    if not words:
        raise BuildError("empty step")
    head, args = words[0], words[1:]

    def need(n):
        if len(args) != n:
            raise BuildError("step %r: %s takes %d argument%s" % (text, head, n, "" if n == 1 else "s"))

    def icon(name):
        name = ICON_ALIASES.get(name, name)
        if name not in ICONS:
            raise BuildError("step %r: unknown icon %r (%s)" % (text, name, " ".join(ICONS)))
        return ICONS.index(name)

    if head == "face":
        need(1)
        if args[0] not in TARGETS:
            raise BuildError("step %r: unknown face target %r" % (text, args[0]))
        return Step("face", TARGETS[args[0]], "AMBIENCE_TARGET_" + args[0].upper(), text)
    if head == "look_back":
        need(0)
        return Step("look_back", text=text)
    if head == "wait":
        need(1)
        if not args[0].isdigit() or not 1 <= int(args[0]) <= PARAM_MAX:
            raise BuildError("step %r: wait takes 1 to %d ticks" % (text, PARAM_MAX))
        return Step("wait", int(args[0]), args[0], text)
    if head == "step":
        need(1)
        if args[0] not in STEPS:
            raise BuildError("step %r: unknown step direction %r (%s)" % (text, args[0], ", ".join(STEPS)))
        return Step("step", STEPS.index(args[0]), "AMBIENCE_STEP_" + args[0].upper(), text)
    if head in ("jump", "spin"):
        need(0)
        return Step(head, text=text)
    if head in ("bow", "turn"):
        need(1)
        want = "nurse" if head == "bow" else "npc"
        if args[0] != want:
            raise BuildError("step %r: %s takes %s" % (text, head, want))
        return Step(head, OBJECTS.index(want), "AMBIENCE_OBJECT_" + want.upper(), text)
    if head == "emote":
        need(1)
        if "/" in args[0] and args[0] not in ICONS:
            even, odd = args[0].split("/", 1)
            e, o = icon(even), icon(odd)
            return Step("emote_parity", e | o << 4,
                        "AMBIENCE_ICON_%s | (AMBIENCE_ICON_%s << AMBIENCE_PARITY_SHIFT)"
                        % (ICON_NAMES[e], ICON_NAMES[o]), text)
        k = icon(args[0])
        return Step("emote", k, "AMBIENCE_ICON_" + ICON_NAMES[k], text)
    if head == "effect":
        need(2)
        if args[0] not in EFFECTS:
            raise BuildError("step %r: unknown effect %r (%s)" % (text, args[0], ", ".join(EFFECTS)))
        if args[1] not in WHERE:
            raise BuildError("step %r: effect tile %r: ahead or own" % (text, args[1]))
        return Step("effect", EFFECTS.index(args[0]) | WHERE.index(args[1]) << 4,
                    "AMBIENCE_EFFECT_%s | (AMBIENCE_WHERE_%s << AMBIENCE_WHERE_SHIFT)"
                    % (args[0].upper(), args[1].upper()), text)
    if head == "companion":
        need(1)
        if args[0] not in ("out", "in"):
            raise BuildError("step %r: companion out or in" % text)
        return Step("companion_" + args[0], text=text)
    if head == "companion_do":
        need(1)
        if args[0] not in COMPANION_ACTS:
            raise BuildError("step %r: unknown companion action %r (%s)"
                             % (text, args[0], ", ".join(COMPANION_ACTS)))
        return Step("companion_do", COMPANION_ACTS.index(args[0]),
                    "AMBIENCE_COMPANION_ACT_" + args[0].upper(), text)
    raise BuildError("step %r: unknown primitive %r" % (text, head))


def check_tile(steps):
    """A beat may step at most 1 tile from its start and must end there.

    `ahead` walks the current facing; `left`/`right` walk west/east and turn
    that way; `back` undoes the last unreturned step, or with none walks
    backwards, keeping the facing. The facing at the start, and after a turn
    to the player, another walker, the target or the companion, is unknown,
    so every combination is tried. Returns a problem or None."""
    dynamic = sum(1 for s in steps if s.op == "face" and s.arg not in DIRECTIONS)
    for start, *turns in itertools.product(DIRECTIONS, *[DIRECTIONS] * dynamic):
        turns = iter(turns)
        facing, pos, pending = start, (0, 0), []
        for s in steps:
            if s.op == "face":
                facing = s.arg if s.arg in DIRECTIONS else next(turns)
            elif s.op == "look_back":
                facing = start
            elif s.op == "step":
                name = STEPS[s.arg]
                if name == "ahead":
                    d = DIRECTIONS[facing]
                elif name == "left":
                    d, facing = DIRECTIONS[3], 3
                elif name == "right":
                    d, facing = DIRECTIONS[4], 4
                elif pending:
                    d = (-pending[-1][0], -pending[-1][1])
                else:
                    d = (-DIRECTIONS[facing][0], -DIRECTIONS[facing][1])
                if pending and d == (-pending[-1][0], -pending[-1][1]):
                    pending.pop()
                else:
                    pending.append(d)
                pos = (pos[0] + d[0], pos[1] + d[1])
                if abs(pos[0]) + abs(pos[1]) > 1:
                    return "step %r leaves the tile next to the start (more than 1 tile away)" % s.text
        if pos != (0, 0):
            return "ends %d tile%s from its start tile without stepping back" % (
                abs(pos[0]) + abs(pos[1]), "" if abs(pos[0]) + abs(pos[1]) == 1 else "s")
    return None


class Beat:
    pass


def parse_count(value, what):
    if isinstance(value, bool) or not isinstance(value, int) or not 1 <= value <= PARAM_MAX:
        raise BuildError("%s: %r is not 1 to %d" % (what, value, PARAM_MAX))
    return value


def parse_term(b, term, into, nested):
    """Add one `when` term to `into` (b.all or b.any); its parameters go to
    the beat and must not conflict with another term's."""
    def param(name, value):
        if name in b.params and b.params[name] != value:
            raise BuildError("conflicting %s: %r and %r" % (name, b.params[name], value))
        b.params[name] = value

    if isinstance(term, str):
        if term not in SIMPLE_TERMS:
            raise BuildError("unknown when term %r" % term)
        into.add(term)
        return
    if not isinstance(term, dict) or not term:
        raise BuildError("unknown when term %r" % (term,))
    keys = set(term)
    if keys == {"any"}:
        if nested:
            raise BuildError("an any term inside an any term")
        if not isinstance(term["any"], list) or len(term["any"]) < 2:
            raise BuildError("any takes a list of at least two terms")
        for sub in term["any"]:
            parse_term(b, sub, b.any, True)
        return
    if keys == {"facing"}:
        if term["facing"] not in FACING:
            raise BuildError("unknown facing %r (%s)" % (term["facing"], ", ".join(FACING)))
        into.add("facing_" + term["facing"])
        return
    if keys in ({"spot"}, {"spot_not"}):
        key = keys.pop()
        kinds = term[key]
        if not isinstance(kinds, list) or not kinds:
            raise BuildError("%s takes a list of spot kinds" % key)
        plain, named = set(), set()
        for kind in kinds:
            if isinstance(kind, str) and kind.startswith("named:") and key == "spot":
                act = activity_key(kind[len("named:"):])
                if act not in ACTIVITY_INDEX:
                    raise BuildError("unknown activity %r in spot kind %r" % (act, kind))
                named.add(ACTIVITY_INDEX[act])
            elif kind in SPOT_KINDS:
                plain.add(KIND_KEYS.index(kind))
            else:
                raise BuildError("unknown spot kind %r (%s%s)" % (
                    kind, ", ".join(SPOT_KINDS), ", named:<activity>" if key == "spot" else ""))
        if key == "spot":
            param("spot", (frozenset(plain), frozenset(named)))
            into.add("spot")
        else:
            param("spot_not", frozenset(plain))
            into.add("not_spot")
        # Both hold only while dwelling; say so in the mask when it is a must.
        if into is b.all:
            into.add("dwelling")
        return
    if keys == {"activity"}:
        acts = term["activity"]
        if not isinstance(acts, list) or not acts:
            raise BuildError("activity takes a list of activities")
        idx = set()
        for act in acts:
            if activity_key(act) not in ACTIVITY_INDEX:
                raise BuildError("unknown activity %r (%s)" % (act, ", ".join(ACTIVITIES)))
            idx.add(ACTIVITY_INDEX[activity_key(act)])
        param("activity", frozenset(idx))
        into.add("activity")
        return
    if keys == {"player_within"}:
        param("player_within", parse_count(term["player_within"], "player_within"))
        into.add("player_within")
        return
    if keys == {"player_adjacent_ticks"}:
        param("player_adjacent_ticks", parse_count(term["player_adjacent_ticks"], "player_adjacent_ticks"))
        into.add("player_adjacent")
        return
    if keys in ({"notable_within"}, {"notable_within", "relation"}):
        param("notable_within", parse_count(term["notable_within"], "notable_within"))
        rel = term.get("relation", "none")
        if rel not in RELATIONS:
            raise BuildError("unknown relation %r (%s)" % (rel, ", ".join(RELATIONS[1:])))
        param("relation", rel)
        into.add("notable")
        return
    raise BuildError("unknown when term %r" % (term,))


def parse_beat(row):
    b = Beat()
    if not isinstance(row, dict):
        raise BuildError("beat %r is not an object" % (row,))
    b.id = row.get("id")
    tag = "beat %r" % b.id
    if not isinstance(b.id, str) or not re.fullmatch(r"[a-z][a-z0-9_]*", b.id):
        raise BuildError("%s: id must be a lower-case name" % tag)
    unknown = sorted(set(row) - BEAT_KEYS)
    if unknown:
        raise BuildError("%s: unknown field %s" % (tag, ", ".join(unknown)))
    try:
        if row.get("class") not in CLASSES:
            raise BuildError("unknown class %r (%s)" % (row.get("class"), ", ".join(CLASSES)))
        b.cls = row["class"]
        b.all, b.any, b.params = set(), set(), {}
        when = row.get("when")
        if not isinstance(when, list) or not when:
            raise BuildError("when takes a list of terms")
        for term in when:
            parse_term(b, term, b.all, False)
        who = row.get("who", {})
        if not isinstance(who, dict):
            raise BuildError("who takes an object")
        bad = sorted(set(who) - WHO_KEYS)
        if bad:
            raise BuildError("unknown who field %s" % ", ".join(bad))
        b.tags_any, b.tags_none = [], []
        for key in ("tags_any", "tags_none"):
            for t in who.get(key, []):
                if t not in TAGS:
                    raise BuildError("unknown tag %r in %s (%s)" % (t, key, ", ".join(TAGS)))
                getattr(b, key).append(t)
        if set(b.tags_any) & set(b.tags_none):
            raise BuildError("a tag in both tags_any and tags_none")
        flags = row.get("flags", [])
        for f in flags:
            if f not in AUTHORED_FLAGS:
                raise BuildError("unknown flag %r (%s)" % (f, ", ".join(AUTHORED_FLAGS)))
        b.flags = set(flags)
        cooldown = row.get("cooldown", DEFAULT_COOLDOWN)
        b.cooldown = parse_count(cooldown, "cooldown")
        steps = row.get("steps")
        if not isinstance(steps, list) or not 1 <= len(steps) <= MAX_STEPS:
            raise BuildError("%s steps; a beat has 1 to %d" % (
                len(steps) if isinstance(steps, list) else "no", MAX_STEPS))
        b.steps = [parse_step(s) for s in steps]
        ops = [s.op for s in b.steps]
        if "companion_out" in ops or who.get("companion") is True:
            b.flags.add("needs_companion")
        if "companion" in who and who["companion"] is not True:
            raise BuildError("who.companion takes true")
        out = False
        for s in b.steps:
            if s.op == "companion_out":
                if out:
                    raise BuildError("companion out twice")
                out = True
            elif s.op == "companion_in":
                if not out:
                    raise BuildError("companion in without companion out")
                out = False
            elif s.op == "companion_do" or (s.op == "face" and s.arg == TARGETS["companion"]):
                if not out:
                    raise BuildError("step %r while no companion is out" % s.text)
        terms = b.all | b.any
        if "companion_room" in terms and b.cls != "idle":
            raise BuildError("companion_room only in an idle beat (the walker works it out only "
                             "where an idle beat can win)")
        if "blocked" in terms and b.cls != "react":
            raise BuildError("blocked only in a react beat (it holds only at the blocked react check)")
        for flag, (fact, term) in LIMIT_TERMS.items():
            if flag in b.flags and fact not in b.all:
                raise BuildError("a %s beat needs a %s term in its when list (not inside an any)"
                                 % (flag, term))
        if "keep_walking" in b.flags and any(s.op != "emote" for s in b.steps):
            raise BuildError("keep_walking beats may only show icons")
        problem = check_tile(b.steps)
        if problem:
            raise BuildError(problem)
    except BuildError as err:
        raise BuildError("%s: %s" % (tag, err))
    return b


# --------------------------------------------------------------------------
# The whole file.
# --------------------------------------------------------------------------

class AmbienceTrainer:
    pass


def sim_slugs(routines_path=None):
    return [e["slug"] for e in load_json(routines_path or TOOL_DIR / "routines.json")["trainers"]
            if e.get("simulated")]


class Ambience:
    """Validates ambience.json and resolves the tables. `sim` is the list of
    simulated trainer slugs in slot order; `catalog` the catalog's trainer
    list; `sprites` a SpeciesSprites (or anything with known and sprite())."""

    def __init__(self, root, sim, catalog, ambience_path=None, sprites=None, data=None):
        self.root = Path(root)
        self.path = ambience_path or TOOL_DIR / "ambience.json"
        duplicates = []
        if data is None:
            data, duplicates = load_data(self.path)
        self.data = data
        self.sim = list(sim)
        self.slot = {slug: i for i, slug in enumerate(self.sim)}
        self.catalog = {t["slug"]: t for t in catalog}
        self.sprites = sprites if sprites is not None else SpeciesSprites(self.root)
        self.problems, self.warnings = list(duplicates), []
        if not isinstance(self.data, dict):
            self.problems.append("the file is not an object")
            self.data = {}
        unknown = sorted(set(self.data) - TOP_KEYS)
        if unknown:
            self.problems.append("unknown top-level key %s (%s)"
                                 % (", ".join(map(repr, unknown)), ", ".join(sorted(TOP_KEYS))))
        self.beats, self.trainers = [], []
        self.relations = [[0] * len(self.sim) for _ in self.sim]
        self.authored_pairs = {}
        self._beats()
        self._trainers()
        self._relations()
        self._companion_beats()

    @classmethod
    def from_routines(cls, rt, ambience_path=None):
        catalog = load_json(rt.root / "tools/notable_trainers/catalog.json")["trainers"]
        return cls(rt.root, [t.slug for t in rt.trainers], catalog, ambience_path=ambience_path)

    def check(self):
        if self.problems:
            raise BuildError("\n".join("ambience.json: " + p for p in self.problems))

    # Beats --------------------------------------------------------------

    def _beats(self):
        rows = self.data.get("beats")
        if not isinstance(rows, list) or not rows:
            self.problems.append("no beats")
            return
        if len(rows) > MAX_BEATS:
            self.problems.append("%d beats, over AMBIENCE_MAX_BEATS (%d)" % (len(rows), MAX_BEATS))
        seen = set()
        for row in rows:
            try:
                b = parse_beat(row)
            except BuildError as err:
                self.problems.append(str(err))
                continue
            if b.id in seen:
                self.problems.append("beat %r: duplicate id" % b.id)
                continue
            seen.add(b.id)
            b.index = len(self.beats)
            self.beats.append(b)
        self.beat_by_id = {b.id: b for b in self.beats}

    # Trainers -----------------------------------------------------------

    def _trainers(self):
        entries = self.data.get("trainers")
        if not isinstance(entries, dict):
            self.problems.append("trainers takes an object keyed by catalog slug")
            entries = {}
        for slug in entries:
            if slug not in self.slot:
                self.problems.append("trainer %r: unknown trainer (not a simulated catalog slug)" % slug)
        for slug in self.sim:
            t = AmbienceTrainer()
            t.slug, t.slot = slug, self.slot[slug]
            cat = self.catalog.get(slug)
            t.name = cat["name"] if cat else slug
            e = entries.get(slug, {})
            tag = "trainer %r" % slug
            bad = sorted(set(e) - TRAINER_KEYS)
            if bad:
                self.problems.append("%s: unknown field %s" % (tag, ", ".join(bad)))
            tags = e.get("tags", [])
            if not tags:
                self.problems.append("%s: no tag (a walking notable has 1 to 3)" % tag)
            elif len(tags) > 3:
                self.problems.append("%s: %d tags, over 3" % (tag, len(tags)))
            if len(set(tags)) != len(tags):
                self.problems.append("%s: a tag listed twice" % tag)
            for x in tags:
                if x not in TAGS:
                    self.problems.append("%s: unknown tag %r (%s)" % (tag, x, ", ".join(TAGS)))
            t.tags = [x for x in tags if x in TAGS]
            t.companions, t.authored_companion = self._companions(t, e.get("companion"), cat, tag)
            t.preferred = None
            pref = e.get("preferred")
            if pref is not None:
                b = getattr(self, "beat_by_id", {}).get(pref)
                if b is None:
                    self.problems.append("%s: preferred beat %r is not in the pool" % (tag, pref))
                else:
                    why = self.excludes(b, t)
                    if why:
                        self.problems.append("%s: can't qualify for preferred beat %r: %s"
                                             % (tag, pref, why))
                    else:
                        t.preferred = b
            self.trainers.append(t)

    def excludes(self, b, t):
        """Why trainer t can never run beat b (its who), or None."""
        if b.tags_any and not set(b.tags_any) & set(t.tags):
            return "needs one of the tags %s" % ", ".join(b.tags_any)
        if set(b.tags_none) & set(t.tags):
            return "excluded by the tag %s" % ", ".join(sorted(set(b.tags_none) & set(t.tags)))
        if "needs_companion" in b.flags and not t.companions:
            return "needs a companion, and the trainer has none"
        return None

    def _companions(self, t, authored, cat, tag):
        """The authored companion first, then the roster's aces in roster
        order, de-duplicated, up to AMBIENCE_COMPANION_MAX: [(SPECIES_X, big)]."""
        out, seen = [], set()
        if authored is not None:
            const = species_const(authored) if isinstance(authored, str) else repr(authored)
            size, why = self.sprites.sprite(const) if isinstance(authored, str) else (None, "not a name")
            if size is None:
                self.problems.append("%s: companion %r has no overworld follower sprite: %s"
                                     % (tag, authored, why))
            else:
                out.append((const, size == "64x64"))
                seen.add(const)
        for mon in (cat or {}).get("roster", []):
            if not mon.get("isAce"):
                continue
            const = species_const(mon["species"])
            if const in seen:
                continue
            seen.add(const)
            size, why = self.sprites.sprite(const)
            if size is None:
                self.warnings.append("%s: ace %s dropped from the companions: %s"
                                     % (tag, mon["species"], why))
                continue
            out.append((const, size == "64x64"))
        return out[:COMPANION_MAX], authored

    # Relations ----------------------------------------------------------

    def league(self, cat):
        """Kanto and Johto share the Indigo League (Indigo Plateau; the
        catalog's Johto Elite Four, Will and Karen, sit beside Kanto's
        Lorelei, and Lance is its Champion); Hoenn has its own. The
        catalog's `league` field is the trainer's league presentation, not
        which League they belong to, so the home region decides."""
        return {"Kanto": "Indigo", "Johto": "Indigo", "Hoenn": "Hoenn"}.get(cat.get("homeRegion"))

    def derive(self, a, b):
        """The derived relation of two simulated trainers (catalog data):
        rival: both are Champions (role "Champion");
        colleague: both Gym Leaders (role "Gym Leader") with the same
        homeRegion (a missing one matches nothing), or both in {"Elite
        Four", "Champion"} of the same League (see league()). Rival wins over colleague: Blue and Lance are both
        Indigo's Champions and rivals."""
        ca, cb = self.catalog.get(a, {}), self.catalog.get(b, {})
        ra, rb = ca.get("role"), cb.get("role")
        if ra == "Champion" and rb == "Champion":
            return "rival"
        if ra == "Gym Leader" and rb == "Gym Leader" and ca.get("homeRegion") \
                and ca.get("homeRegion") == cb.get("homeRegion"):
            return "colleague"
        league = {"Elite Four", "Champion"}
        if ra in league and rb in league and self.league(ca) and self.league(ca) == self.league(cb):
            return "colleague"
        return "none"

    def _relations(self):
        champions = {s for s in self.sim if self.catalog.get(s, {}).get("role") == "Champion"}
        if champions != CHAMPIONS:
            self.problems.append("the simulated Champions are %s, expected %s"
                                 % (", ".join(sorted(champions)), ", ".join(sorted(CHAMPIONS))))
        rows = self.data.get("relationships", [])
        if not isinstance(rows, list):
            self.problems.append("relationships takes a list")
            rows = []
        for row in rows:
            rel = row.get("relation") if isinstance(row, dict) else None
            pair = row.get("pair") if isinstance(row, dict) else None
            tag = "relationship %r" % (pair,)
            if rel not in AUTHORED_RELATIONS:
                self.problems.append("%s: unknown relation %r (authored: %s)"
                                     % (tag, rel, ", ".join(AUTHORED_RELATIONS)))
                continue
            if not isinstance(pair, list) or len(pair) != 2:
                self.problems.append("%s: a pair names two trainers" % tag)
                continue
            unknown = [p for p in pair if p not in self.slot]
            if unknown:
                self.problems.append("%s: unknown trainer %s (not a simulated catalog slug)"
                                     % (tag, ", ".join(map(repr, unknown))))
                continue
            if pair[0] == pair[1]:
                self.problems.append("%s: names the trainer %s twice" % (tag, pair[0]))
                continue
            key = frozenset(pair)
            if key in self.authored_pairs:
                self.problems.append("%s: names %s and %s twice (already %s)"
                                     % (tag, pair[0], pair[1], self.authored_pairs[key]))
                continue
            self.authored_pairs[key] = rel
        for a, b in itertools.combinations(self.sim, 2):
            rel = self.authored_pairs.get(frozenset((a, b))) or self.derive(a, b)
            value = RELATIONS.index(rel)
            self.relations[self.slot[a]][self.slot[b]] = value
            self.relations[self.slot[b]][self.slot[a]] = value

    def relation(self, a, b):
        return RELATIONS[self.relations[self.slot[a]][self.slot[b]]]

    # Companion beats ----------------------------------------------------

    def _companion_beats(self):
        for b in self.beats:
            if "needs_companion" not in b.flags:
                continue
            for t in self.trainers:
                if t.companions:
                    continue
                if (b.tags_any and not set(b.tags_any) & set(t.tags)) or set(b.tags_none) & set(t.tags):
                    continue
                self.problems.append("beat %r: companion beat for %s, who has no ace with an "
                                     "overworld follower sprite (or authored companion)"
                                     % (b.id, t.slug))

    # Output helpers -------------------------------------------------------

    def pairs(self):
        out = []
        for a, b in itertools.combinations(self.sim, 2):
            rel = self.relation(a, b)
            if rel != "none":
                out.append({"pair": [a, b], "relation": rel,
                            "authored": frozenset((a, b)) in self.authored_pairs})
        return out


def load(root, ambience_path=None, routines_path=None, catalog_path=None, sprites=None, data=None):
    """The ambience without the map build: the simulated slugs straight from
    routines.json (as Routines takes them, in catalog order)."""
    root = Path(root)
    catalog = load_json(catalog_path or root / "tools/notable_trainers/catalog.json")["trainers"]
    return Ambience(root, sim_slugs(routines_path), catalog, ambience_path=ambience_path,
                    sprites=sprites, data=data)
