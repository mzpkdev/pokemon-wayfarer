"""Collect the regular trainer slots of the compiled Wayfarer maps and analyse their scripts.

A trainer slot is an object event that starts a trainer battle. Its script is read the way the assembler sees it
(includes expanded, cpp'd, conditionals applied), following gotos, calls and fall-through, so a script that does
anything beyond battling, registering a phone number, handing over a gift and speaking is found by its commands.
"""
from __future__ import annotations

from collections import defaultdict
import json
from pathlib import Path
import re
import subprocess
import sys

TOOL = Path(__file__).resolve().parent
ROOT = TOOL.parents[1]
sys.path.insert(0, str(ROOT / 'tools/trainer_scaling'))
import reach_resolver as R  # noqa: E402

TRAINER_TYPES = ('TRAINER_TYPE_NORMAL', 'TRAINER_TYPE_SEE_ALL_DIRECTIONS', 'TRAINER_TYPE_BURIED')
ROTATING_TYPES = ('TRAINER_TYPE_NORMAL', 'TRAINER_TYPE_SEE_ALL_DIRECTIONS')
OPENING_BATTLES = ('trainerbattle_single', 'trainerbattle_double', 'trainerbattle_no_intro', 'trainerbattle')
REMATCH_BATTLES = ('trainerbattle_rematch', 'trainerbattle_rematch_double')
NO_FLAG = {'', '0', '0x0', 'FLAG_NONE', 'FLAG_0'}
POLICIES = {0: 'EXCLUDED', 1: 'ORDINARY', 2: 'GYM_MEMBER', 3: 'GYM_LEADER'}
CPP = ['cpp', '-P', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS', '-DIS_WAYFARER=1', '-DIS_HNS=1', '-I', str(ROOT / 'include')]

# Commands a plain trainer script may use: battling, speaking, phone registration, a gift hand-off and the plumbing
# between them. Anything else (movement, flags of the story, item checks of its own, warps) is "more than battle".
PLAIN_COMMANDS = {
    'end', 'release', 'releaseall', 'lock', 'faceplayer', 'closemessage', 'waitmovement', 'msgbox', 'goto', 'return', 'textcolor',
    'goto_if_eq', 'goto_if_ne', 'goto_if_set', 'goto_if_unset', 'goto_if_defeated', 'goto_if_not_defeated', 'call', 'special',
    'specialvar', 'setvar', 'register_matchcall', 'giveitem', 'checkitem', 'clearflag',
} | set(OPENING_BATTLES) | set(REMATCH_BATTLES)
PLAIN_SPECIALS = {'PlayerFaceTrainerAfterBattle', 'GetPlayerBigGuyGirlString'}
PLAIN_SPECIALVARS = {'ShouldTryRematchBattle', 'IsTrainerRegistered'}
# The one-time reward a rematch can hand over (checkitem, giveitem, no-op) keeps its trainer's script plain.
ALLOWED_CALLS = {'Common_EventScript_NoOp'}


class Slot:
    __slots__ = ('map_const', 'map_name', 'map_value', 'local_id', 'x', 'y', 'gfx', 'script', 'trainer_type', 'hide_flag', 'sight',
                 'movement', 'home', 'home_id', 'pair', 'klass', 'family', 'region', 'policy', 'status', 'reason', 'texts', 'contact',
                 'index', 'group', 'family_ids', 'gift', 'commands', 'map_type')

    def __init__(self, **fields):
        for slot in self.__slots__:
            setattr(self, slot, fields.get(slot))

    @property
    def label(self):
        return f'{self.map_name} local id {self.local_id} ({self.x},{self.y}) {self.home}'

    @property
    def key(self):
        return (self.map_value, self.local_id)


def read_lines(body):
    """[(command, [args])] of a label body: instructions only (no data directives, labels or text)."""
    out = []
    for raw in body.splitlines():
        line = re.sub(r'@.*', '', raw).strip()
        if not line or line.endswith(':') or line.startswith(('.', '#')):
            continue
        parts = line.split(None, 1)
        out.append((parts[0], R.macro_args(parts[1]) if len(parts) > 1 else []))
    return out


def string_text(body):
    """The text a `.string` label body spells, with the line breaks as spaces and no terminator."""
    pieces = re.findall(r'^[ \t]*\.string\s+"((?:[^"\\]|\\.)*)"', body, re.M)
    if not pieces:
        return None
    text = ''.join(pieces)
    text = re.sub(r'\\[nlp]', ' ', text).replace('\\$', '').rstrip('$')
    return re.sub(r'\s+', ' ', text).strip()


def symbol_numbers(names, headers=('constants/opponents.h', 'constants/flags.h', 'constants/vars.h')):
    """{symbol: number} through the real constant headers (the preprocessor evaluates the expressions)."""
    names = sorted(set(names))
    if not names:
        return {}
    source = ''.join(f'#include "{header}"\n' for header in headers) + '\n'.join(f'ID("{name}", {name})' for name in names)
    done = subprocess.run(CPP + ['-'], input=source, text=True, capture_output=True, cwd=ROOT)
    if done.returncode:
        raise R.ReachError(f'cpp failed on the constant headers: {done.stderr[:400]}')
    result = {}
    for name, expression in re.findall(r'ID\("(\w+)",\s*(.*?)\)\s*$', done.stdout, re.M):
        if R.NUMBER.match(expression):
            result[name] = eval(expression, {'__builtins__': {}}, {})
    return result


def gift_flag_names():
    names = set()
    for path in (ROOT / 'include/constants').glob('flags*.h'):
        names |= set(re.findall(r'^#define\s+(FLAG_\w*HAS_ITEM\w*)\b', path.read_text(), re.M))
    return names


def policies(root=ROOT):
    text = (root / 'src/data/trainer_scaling/policies.h').read_text()
    return {name: POLICIES[int(value)] for name, value in re.findall(r'\[(TRAINER_\w+)\]\s*=\s*(\d+)', text)}


def inventory(root=ROOT):
    """{TRAINER_*: (trainerClass, battleType)} of the committed scaling inventory."""
    records = json.loads((root / 'src/data/trainer_scaling/inventory.json').read_text())['records']
    return {name: (row.get('trainerClass', ''), row.get('battleType', '')) for name, row in records.items()}


class Analyzer:
    """Script analysis over the Wayfarer script assembly."""

    def __init__(self, labels, ordinary_numbers, root=ROOT):
        self.labels = labels
        self.numbers = ordinary_numbers            # {number: TRAINER_* name}
        self.by_name = {v: k for k, v in ordinary_numbers.items()}
        flag_names = gift_flag_names() | {'FLAG_HAS_MATCH_CALL'}
        self.flag_numbers = symbol_numbers(flag_names, ('constants/flags.h',))
        self.gift_numbers = {n for name, n in self.flag_numbers.items() if name != 'FLAG_HAS_MATCH_CALL'}
        self.plain_flags = set(self.flag_numbers.values())
        extra = symbol_numbers({'VAR_RESULT', 'VAR_0x8004'}, ('constants/vars.h',))
        self.var_result = extra.get('VAR_RESULT')
        self.var_8004 = extra.get('VAR_0x8004')
        self._closures = {}

    def closure(self, entry):
        """Labels reachable from `entry`, in discovery order, entry first."""
        if entry in self._closures:
            return self._closures[entry]
        order, seen, queue = [], set(), [entry]
        while queue:
            current = queue.pop(0)
            if current in seen or current not in self.labels:
                continue
            seen.add(current)
            order.append(current)
            body = re.sub(r'@.*', '', self.labels[current])
            found = [token for token in R.IDENT.findall(body) if token in self.labels]
            if current in R.FALLTHROUGH:
                found.insert(0, R.FALLTHROUGH[current])
            queue.extend(found)
        self._closures[entry] = order
        return order

    def analyse(self, entry, family_names):
        """Facts about one object's script: its battles, its texts and everything that makes it more than a plain trainer."""
        info = {'home': None, 'pair': False, 'trainers': set(), 'texts': [], 'problems': [], 'commands': set(), 'gift': False,
                'two_trainers': False, 'battles': 0}
        labels = self.closure(entry)
        family_numbers = {self.by_name[n] for n in family_names if n in self.by_name}
        for label in labels:
            for cmd, args in read_lines(self.labels[label]):
                info['commands'].add(cmd)
                if cmd.startswith('trainerbattle'):
                    self._battle(cmd, args, label, info)
                elif cmd == 'msgbox' and args and args[0] in self.labels:
                    if 'Register' not in args[0]:
                        info['texts'].append(('say', args[0]))
                elif cmd not in PLAIN_COMMANDS:
                    info['problems'].append(cmd)
                else:
                    self._argument_problems(cmd, args, family_numbers, info)
        info['trainers'].discard(0)
        return info

    def _battle(self, cmd, args, label, info):
        info['battles'] += 1
        if cmd == 'trainerbattle_two_trainers':
            info['two_trainers'] = True
            info['problems'].append(cmd)
            return
        if cmd == 'trainerbattle':
            info['problems'].append('trainerbattle (raw)')
            return
        number = eval(args[0], {'__builtins__': {}}, {}) if args and R.NUMBER.match(args[0]) else 0
        info['trainers'].add(number)
        if cmd in OPENING_BATTLES and info['home'] is None:
            info['home'] = number
            info['pair'] = cmd == 'trainerbattle_double'
        kind = 'rematch' if cmd in REMATCH_BATTLES else 'battle'
        for position, name in ((1, 'intro'), (2, 'defeat')):
            if len(args) > position and args[position] in self.labels:
                info['texts'].append(((kind if kind == 'rematch' else name), args[position]))

    def _argument_problems(self, cmd, args, family_numbers, info):
        def number(text):
            return eval(text, {'__builtins__': {}}, {}) if R.NUMBER.match(text or '') else None

        if cmd == 'special' and (not args or args[0] not in PLAIN_SPECIALS):
            info['problems'].append(f'special {args[0] if args else ""}'.strip())
        elif cmd == 'specialvar' and (len(args) < 2 or args[1] not in PLAIN_SPECIALVARS):
            info['problems'].append(f'specialvar {args[1] if len(args) > 1 else ""}'.strip())
        elif cmd in ('goto_if_eq', 'goto_if_ne') and (not args or number(args[0]) not in (self.var_result,)):
            info['problems'].append(f'{cmd} on a variable other than the result')
        elif cmd in ('goto_if_set', 'goto_if_unset'):
            pass    # branches on a flag (a phone flag, a gift flag); what a branch does is checked where it leads
        elif cmd == 'clearflag' and number(args[0] if args else '') not in self.gift_numbers:
            info['problems'].append('clearflag on a story flag')
        elif cmd in ('goto_if_defeated', 'goto_if_not_defeated'):
            if number(args[0] if args else '') not in family_numbers:
                info['problems'].append(f'{cmd} on another trainer')
        elif cmd == 'setvar' and number(args[0] if args else '') != self.var_8004:
            info['problems'].append('setvar on a story variable')
        elif cmd == 'call' and (not args or args[0] not in ALLOWED_CALLS and args[0] not in self.labels):
            info['problems'].append('call')
        elif cmd == 'giveitem':
            if args and number(args[0]) is not None and number(args[0]) >= 0x4000:
                info['gift'] = True
        elif cmd == 'checkitem':
            pass


def rematch_families(root=ROOT):
    """[(trainer names, kind)] of every rematch family the Wayfarer ROM knows: the HNS/Hoenn table rows, Sevii and coast."""
    import reach as RE  # noqa: E402  (tools/trainer_scaling is on sys.path)
    result = []
    for ids, table_map in RE.rematch_families(root):
        result.append([name for name in ids if name != 'TRAINER_NONE'])
    return result


def collect_slots(maps, labels, root=ROOT, talk_only=None):
    """Every trainer object of the compiled maps as an unclassified Slot row (status, reason and group are set later).
    A talk-only trainer (no sight line, so nothing for a daily slot to rotate) goes to `talk_only` when it is given."""
    ordinary_names = set(policies(root))
    families = rematch_families(root)
    all_names = set(ordinary_names) | {n for fam in families for n in fam}
    numbers = R.trainer_numbers_for(all_names, root) if hasattr(R, 'trainer_numbers_for') else None
    if numbers is None:
        import reach as RE
        numbers = RE.trainer_numbers(all_names, root)
    by_number = {number: name for name, number in numbers.items()}
    family_of = {}
    for fam in families:
        for name in fam:
            family_of.setdefault(name, fam)
    policy = policies(root)
    classes = inventory(root)
    analyzer = Analyzer(labels, by_number, root)
    slots = []
    for const, info in maps.items():
        for obj in info.events['objects']:
            if obj.get('kind') != 'object' or (obj['trainer_type'] not in TRAINER_TYPES and (talk_only is None or obj['trainer_type'] != 'TRAINER_TYPE_NONE')):
                continue
            if not R.TRAINERBATTLE.search('\n'.join(labels.get(l, '') for l in analyzer.closure(obj['script']))):
                continue
            slot = Slot(map_const=const, map_name=info.name, map_value=info.value, local_id=obj['local_id'], x=obj['x'], y=obj['y'],
                        gfx=obj['gfx'], script=obj['script'], trainer_type=obj['trainer_type'], hide_flag=obj['flag'],
                        sight=obj['sight'], movement=obj['movement'], map_type=info.map_type)
            facts = analyzer.analyse(obj['script'], family_of.get(by_number.get(0), []))
            home = by_number.get(facts['home']) if facts['home'] is not None else None
            if home is not None:
                facts = analyzer.analyse(obj['script'], family_of.get(home, [home]))
            slot.home, slot.home_id, slot.pair = home, facts['home'], facts['pair']
            slot.texts = facts['texts']
            slot.gift = facts['gift']
            slot.commands = facts
            slot.policy = policy.get(home)
            slot.family_ids = [by_number[n] for n in sorted(facts['trainers']) if n in by_number]
            cls, battle_type = classes.get(home, ('', ''))
            slot.klass = cls
            (slots if obj['trainer_type'] in TRAINER_TYPES else talk_only).append(slot)
    return slots, family_of, numbers
