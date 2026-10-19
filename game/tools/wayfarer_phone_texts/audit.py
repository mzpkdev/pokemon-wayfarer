#!/usr/bin/env python3
"""Audit of the phone call texts (.product/specs/phone-rematches.md, Calls and Validation).

Checks that every phone contact (HNS, Hoenn, Sevii, coast) has the texts the call code asks for, that every Battle and
FoundItem text names today's place with {STR_VAR_2}, and that every line fits the call window.  Sources:

  data/text/match_call_{hns,hoenn,sevii_coast}.inc   the texts
  src/data/wayfarer_phone_{hoenn,sevii,coast}.h      the Hoenn, Sevii and coast contacts, keyed by first trainer
  src/match_call.c                                   the HNS contacts (sHnsMatchCallTrainers)
  src/battle_setup.c, src/data/wayfarer_sevii_rematches.h, src/wayfarer_coast_trainers.c   the rematch families
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

LINE_WIDTH = 30     # characters per line of the call window
BOX_LINES = 3       # a box shows at most three lines (a \l scrolls the third in)
HOENN_BOX_LINES = 2 # the converted Emerald texts keep to two lines a box (the spec's conversion rules)
PLACEHOLDER_WIDTHS = {'PLAYER': 7, 'STR_VAR_1': 16, 'STR_VAR_2': 16, 'STR_VAR_3': 10}
GROUPS = ('HOENN', 'SEVII', 'COAST')


class AuditError(Exception):
    pass


def read(root, relative):
    return (root / relative).read_text(encoding='utf-8')


def parse_strings(source):
    """label -> the list of its .string lines' contents, escapes kept as written (\\n, \\l, \\p)."""
    strings = {}
    for match in re.finditer(r'^(MatchCall_\w+)::\n((?:[ \t]+\.string ".*"\n)+)', source, re.M):
        strings[match.group(1)] = re.findall(r'\.string "(.*)"', match.group(2))
    return strings


def parse_texts(source):
    """label -> the text of its .string lines, escapes kept as written (\\n, \\l, \\p)."""
    return {label: ''.join(parts) for label, parts in parse_strings(source).items()}


def format_problems(label, parts):
    """One terminating $ at the very end, and every other .string line ends where the text can continue: a line break or box."""
    problems = []
    text = ''.join(parts)
    if text.count('$') != 1 or not text.endswith('$'):
        problems.append(f'{label}: needs exactly one $, at the end of the last line')
    for index, part in enumerate(parts[:-1]):
        if not re.search(r'\\[nlp]$', part):
            problems.append(f'{label}: line {index + 1} does not end in \\n, \\l or \\p')
    return problems


def split_boxes(text):
    """The boxes of a text, each a list of lines. The terminating $ is dropped."""
    text = text.rstrip('$')
    boxes = []
    for box in text.split('\\p'):
        boxes.append(re.split(r'\\[nl]', box))
    return boxes


def expand(line):
    """The line with each placeholder replaced by as many characters as the game puts there."""
    def fill(match):
        name = match.group(1)
        if name not in PLACEHOLDER_WIDTHS:
            raise AuditError(f'unknown placeholder {{{name}}}')
        return 'x' * PLACEHOLDER_WIDTHS[name]
    return re.sub(r'\{(\w+)\}', fill, line)


def fit_problems(label, text, box_lines=BOX_LINES):
    problems = []
    try:
        for index, box in enumerate(split_boxes(text)):
            if len(box) > box_lines:
                problems.append(f'{label}: box {index + 1} has {len(box)} lines, at most {box_lines}')
            for line in box:
                width = len(expand(line))
                if width > LINE_WIDTH:
                    problems.append(f'{label}: box {index + 1} line "{line}" is {width} characters wide, at most {LINE_WIDTH}')
    except AuditError as error:
        problems.append(f'{label}: {error}')
    return problems


# ---------- contacts ----------
def strip_blocks(source, condition):
    """The source without the #if <condition> ... #endif blocks."""
    return re.sub(r'#if ' + re.escape(condition) + r'\n.*?#endif[^\n]*\n', '', source, flags=re.S)


def hns_contacts(root):
    """[(name, first trainer, texts count, has gift)] of the HNS contacts Wayfarer compiles."""
    source = read(root, 'src/match_call.c')
    start = source.index('sHnsMatchCallTrainers[] =')
    table = strip_blocks(source[start:source.index('\n};', start)], '!IS_WAYFARER')
    rows = []
    for match in re.finditer(r'HNS_MC_(ENTRY|ITEM)([23])\(\s*(\w+),\s*(TRAINER_\w+)', table):
        rows.append((match.group(3), match.group(4), int(match.group(2)), match.group(1) == 'ITEM'))
    return rows


def listed_contacts(root, group):
    source = read(root, f'src/data/wayfarer_phone_{group.lower()}.h')
    return re.findall(r'^PHONE_CONTACT\((\w+), (\w+)\)', source, re.M)


def table_rows(root):
    """[(first trainer, map)] of the rematch table rows Wayfarer compiles, with the Hoenn block kept apart."""
    source = read(root, 'src/battle_setup.c')
    start = source.index('#if IS_HNS\nconst struct RematchTrainer')
    block = source[start:source.index('\n#else', start)]
    block = strip_blocks(block, '!IS_WAYFARER')
    hoenn_start = block.index('#if IS_WAYFARER')
    hns, hoenn = block[:hoenn_start], block[hoenn_start:]
    pattern = r'=\s*REMATCH\(\s*(TRAINER_\w+),'
    return re.findall(pattern, hns), re.findall(pattern, hoenn)


def sevii_contacts(root):
    """First trainers of the Sevii families whose stages differ, in registry order."""
    firsts = []
    for match in re.finditer(r'SEVII_REMATCH\(([^)]*)\)', read(root, 'src/data/wayfarer_sevii_rematches.h')):
        ids = [part.strip() for part in match.group(1).split(',')]
        if len(set(ids)) > 1:
            firsts.append(ids[0])
    return firsts


def coast_contacts(root):
    source = read(root, 'src/wayfarer_coast_trainers.c')
    source = source[source.index('sWayfarerCoastRematchFamilies'):]
    source = source[:source.index('};')]
    return [match.group(1) for match in re.finditer(r'\{\{(TRAINER_\w+),', source)]


def placed_trainers(root):
    """First trainers that have a daily slot, and those that have a fixed home (src/data/trainer_slots/slots.h)."""
    source = read(root, 'src/data/trainer_slots/slots.h')
    split = source.index('sWorldTrainerFixedHomes')
    pattern = r'^\s*\{ (TRAINER_\w+), MAP_GROUP'
    return (set(re.findall(pattern, source[:split], re.M)), set(re.findall(pattern, source[split:], re.M)))


def audit(root=ROOT):
    errors = []
    texts = {}
    strings = {}
    for name in ('hns', 'hoenn', 'sevii_coast'):
        source = read(root, f'data/text/match_call_{name}.inc')
        texts.update(parse_texts(source))
        strings.update(parse_strings(source))

    def need(label, place=True, fit=True, box_lines=BOX_LINES):
        if label not in texts:
            errors.append(f'missing text {label}')
            return
        if place and '{STR_VAR_2}' not in texts[label]:
            errors.append(f'{label} does not name today\'s place with {{STR_VAR_2}}')
        if fit:
            errors.extend(fit_problems(label, texts[label], box_lines))
            errors.extend(format_problems(label, strings[label]))

    # HNS: the table rows and the call table describe the same contacts.
    hns_rows, hoenn_rows = table_rows(root)
    hns = hns_contacts(root)
    if sorted(row[1] for row in hns) != sorted(hns_rows):
        errors.append('the HNS call table and the HNS rematch table rows differ: '
                      f'{sorted(set(row[1] for row in hns) ^ set(hns_rows))}')
    for name, trainer, general, gift in hns:
        for number in range(1, general + 1):
            need(f'MatchCall_HNS_{name}_General{number}', place=False, fit=False)  # unchanged Crystal texts
        need(f'MatchCall_HNS_{name}_Battle')
        if gift:
            need(f'MatchCall_HNS_{name}_FoundItem')
    if len(hns) != 39:
        errors.append(f'{len(hns)} HNS contacts, the spec has 39')

    # Hoenn: every Hoenn rematch row has a contact.
    hoenn = listed_contacts(root, 'hoenn')
    if sorted(trainer for _, trainer in hoenn) != sorted(hoenn_rows):
        errors.append('the Hoenn contact list and the Hoenn rematch table rows differ: '
                      f'{sorted(set(t for _, t in hoenn) ^ set(hoenn_rows))}')
    if len(hoenn) != 64:
        errors.append(f'{len(hoenn)} Hoenn contacts, the spec has 64')

    # Sevii and coast: the lists follow the registries.
    sevii = listed_contacts(root, 'sevii')
    if [trainer for _, trainer in sevii] != sevii_contacts(root):
        errors.append('the Sevii contact list is not the Sevii families that carry a stronger team, in registry order')
    if len(sevii) != 32:
        errors.append(f'{len(sevii)} Sevii contacts, the spec has 32')
    coast = listed_contacts(root, 'coast')
    if [trainer for _, trainer in coast] != coast_contacts(root):
        errors.append('the coast contact list is not the coast families in registry order')
    if len(coast) != 9:
        errors.append(f'{len(coast)} coast contacts, the spec has 9')

    # Every contact stands somewhere: in a daily slot or at a fixed home, or its calls could never name a place.
    slotted, fixed = placed_trainers(root)
    firsts = set(hns_rows) | set(hoenn_rows) | set(sevii_contacts(root)) | set(coast_contacts(root))
    for trainer in sorted(firsts - slotted - fixed):
        errors.append(f'{trainer} is a phone contact with neither a daily slot nor a fixed home')

    for group, contacts in zip(GROUPS, (hoenn, sevii, coast)):
        names = [name for name, _ in contacts]
        if len(set(names)) != len(names):
            errors.append(f'{group} contacts repeat a name')
        for name in names:
            lines = HOENN_BOX_LINES if group == 'HOENN' else BOX_LINES
            for number in (1, 2, 3):
                need(f'MatchCall_{group}_{name}_General{number}', place=False, box_lines=lines)
            need(f'MatchCall_{group}_{name}_Battle', box_lines=lines)
    return sorted(set(errors))


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    parser.add_argument('--root', type=Path, default=ROOT)
    args = parser.parse_args()
    errors = audit(args.root)
    for error in errors:
        print(error, file=sys.stderr)
    if errors:
        print(f'{len(errors)} phone text problem(s)', file=sys.stderr)
        return 1
    print('phone texts: 39 HNS, 64 Hoenn, 32 Sevii and 9 coast contacts')
    return 0


if __name__ == '__main__':
    sys.exit(main())
