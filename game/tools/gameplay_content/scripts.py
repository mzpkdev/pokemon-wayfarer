"""Selected script commands from the production preproc/CPP/GAS pipeline.

The symbolic view uses CPP's directives-only mode: it preserves authored names
while evaluating the same conditionals as the resolved view. Neither view guesses
feature defaults or scans unreferenced source files into the selected inventory.
"""
from collections import defaultdict
import hashlib
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

from .common import ContentError

LOCATION = re.compile(r'^#\s+(\d+)\s+"([^"]+)"')
LABEL = re.compile(r'^\s*([A-Za-z_]\w*):\s*$')
BATTLE = re.compile(r'^\s*(trainerbattle(?:_\w+)?)\s+(.+)$')


class ScriptBlocks(dict):
    """Selected labels plus the complete production pipeline input closure."""

    def __init__(self, blocks, input_hashes):
        super().__init__(blocks)
        self.input_hashes = input_hashes


def source_hashes(root, *views):
    root = Path(root).resolve()
    paths = set()
    for view in views:
        for line in view.splitlines():
            marker = LOCATION.match(line)
            if marker:
                path = (root / marker[2]).resolve()
                if path.is_relative_to(root) and path.is_file():
                    paths.add(path)
    return {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted(paths)}


def parse_blocks(text):
    """Keep duplicate labels visible; ignore assembler macro definitions.

    preproc -s inserts .size metadata between labels. It has no command semantics
    and is excluded from review bodies. Authored assembler conditionals remain
    visible; production GAS output determines their selected instructions.
    """
    blocks = defaultdict(list)
    path, line_number = '', 0
    current = None
    macro_depth = 0
    conditional_depth = 0
    for physical_line, line in enumerate(text.splitlines(), 1):
        location = LOCATION.match(line)
        if location:
            line_number, path = int(location[1]), location[2]
            continue
        if line.startswith('#'):
            # -fdirectives-only also prints macro definitions and #undef lines.
            continue
        stripped = line.strip()
        if stripped.startswith('.macro '):
            macro_depth += 1
        elif stripped == '.endm':
            macro_depth -= 1
        elif not macro_depth:
            # This entire line is inserted by the production -s size annotator;
            # it may finish with .global for the next label on the same line.
            metadata = stripped.startswith('.ifdef ') and '; .size ' in stripped
            if metadata:
                line = re.sub(r'^\s*\.ifdef .*?;\s*\.size .*?;\s*\.endif\s*;?\s*', '', line)
                stripped = line.strip()
            if not stripped or stripped.startswith('.global '):
                line_number += 1
                continue
            if re.match(r'\.(?:if|ifdef|ifndef)\b', stripped):
                conditional_depth += 1
            elif stripped == '.endif':
                conditional_depth = max(0, conditional_depth - 1)
            label = LABEL.match(line)
            if label:
                current = {'path': path, 'line': line_number, 'body': [], '_lines': [],
                           'assemblerConditional': conditional_depth > 0}
                blocks[label[1]].append(current)
            elif current is not None and stripped:
                current['body'].append(line)
                current['_lines'].append(physical_line)
        line_number += 1
    for entries in blocks.values():
        for entry in entries:
            entry['body'] = '\n'.join(entry['body'])
    return dict(blocks)


def _run(command, root, source=None):
    result = subprocess.run(command, cwd=root, input=source, text=True, capture_output=True)
    if result.returncode:
        raise ContentError('UNRESOLVED', 'data/event_scripts.s', 'script pipeline', result.stderr)
    return result.stdout


def emitted_lines(listing):
    """Read GAS's emitted-byte rows, not its displayed inactive source lines."""
    return {int(match[1]) for line in listing.splitlines()
            if (match := re.match(r'^\s*(\d+)\s+[0-9a-fA-F]+\s+[0-9a-fA-F]{2,}\s', line))}


def load_script_blocks(root, product, maps, cpp='cpp', cppflags=(), *, service_bindings=None,
                       assembler='arm-none-eabi-as', asflags=()):
    """Return each selected label's symbolic and resolved command bodies.

    A fresh mart alias include is supplied before invoking CPP, so clean framework
    generation does not depend on its own previous output directory. The service
    compiler owns that include; this adapter does not duplicate its bindings.
    """
    root = Path(root)
    if service_bindings is None:
        from .configuration import numeric_defines
        from .services import compile_services
        service_bindings = compile_services(root, product, numeric_defines(root, cpp, cppflags),
                                            maps, cpp=cpp, cppflags=cppflags)['outputs']['gameplay_mart_bindings.inc']
    flags = shlex.split(cppflags) if isinstance(cppflags, str) else list(cppflags)
    # Provenance markers are required even if an external caller supplied -P.
    flags = [flag for flag in flags if flag != '-P']
    source = _run([str(root.resolve() / 'tools/preproc/preproc'), '-s',
                   'data/event_scripts.s', 'charmap.txt'], root)
    with tempfile.TemporaryDirectory(prefix='gameplay-script-bindings-') as directory:
        Path(directory, 'gameplay_mart_bindings.inc').write_text(service_bindings)
        command = [cpp, '-iquote', directory, *flags, '-I', 'include']
        symbolic_source = _run([*command, '-fdirectives-only', '-'], root, source)
        symbolic = parse_blocks(symbolic_source)
        resolved_source = _run([*command, '-'], root, source)
        assembled_source = _run([str(root.resolve() / 'tools/preproc/preproc'), '-ie',
                                 'data/event_scripts.s', 'charmap.txt'], root, resolved_source)
        assembly = Path(directory, 'scripts.s')
        assembly.write_text(assembled_source)
        listing, obj = Path(directory, 'scripts.lst'), Path(directory, 'scripts.o')
        flags_as = shlex.split(asflags) if isinstance(asflags, str) else list(asflags)
        _run([assembler, *flags_as, '-al=' + str(listing), '-o', str(obj), str(assembly)], root)
        emitted = emitted_lines(listing.read_text())
        from .postlink import Elf32
        compiled = Elf32(obj)
        defined = compiled.symbols
        resolved = parse_blocks(assembled_source)
    if symbolic.keys() != resolved.keys():
        raise ContentError('CONFLICT', 'data/event_scripts.s', 'CPP views', 'selected labels differ')
    selected = {}
    for label, entries in symbolic.items():
        counterparts = resolved[label]
        if len(entries) != len(counterparts):
            raise ContentError('CONFLICT', 'data/event_scripts.s', label, 'selected label multiplicity differs')
        for entry, counterpart in zip(entries, counterparts):
            if entry['path'] != counterpart['path']:
                raise ContentError('CONFLICT', entry['path'], label, 'selected source provenance differs')
            entry['resolvedBody'] = counterpart['body']
            actual_lines = counterpart['_lines']
            if label not in defined or not (len(entries) == 1 or emitted.intersection(actual_lines)):
                continue
            # An outer product .if has already been evaluated by GAS. Keep only
            # battle invocations that emitted bytes within this selected block.
            battles = [(number, BATTLE.match(line)) for number, line in zip(actual_lines, counterpart['body'].splitlines())
                       if BATTLE.match(line)]
            entry['selectedBattleOrdinals'] = [index for index, (number, _) in enumerate(battles) if number in emitted]
            # Relocatable sections can share address zero. Resolve bytes through
            # this symbol's section, never through a flat address lookup.
            if len(defined[label]) == 1:
                address, section_index = next(iter(defined[label]))
                section = compiled.sections[section_index]
                offset = address - section['address']
                if 0 <= offset and offset + 5 <= section['size']:
                    command_bytes = compiled.data[section['offset'] + offset:section['offset'] + offset + 5]
                    if command_bytes[0] == 0x5c:
                        entry['compiledBattle'] = {
                            'mode': command_bytes[1] >> 4,
                            'trainer': int.from_bytes(command_bytes[3:5], 'little'),
                        }
            entry.pop('_lines')
            selected.setdefault(label, []).append(entry)
    return ScriptBlocks(selected, source_hashes(root, source, symbolic_source, resolved_source))


def discover_battles(blocks):
    """Inventory every selected battle instruction without authorizing any.

    Commands after a label's first instruction are still discovered, but only an
    exact command label can become a declaration. Raw expressions are retained
    for the trainer domain to resolve; dynamic operands stay explicit evidence.
    """
    discovered = []
    for label, entries in sorted(blocks.items()):
        for occurrence, entry in enumerate(entries):
            commands = [line.strip() for line in entry['body'].splitlines() if line.strip()]
            actual = [line.strip() for line in entry['resolvedBody'].splitlines() if line.strip()]
            symbolic_battles = [(index, BATTLE.match(line)) for index, line in enumerate(commands) if BATTLE.match(line)]
            resolved_battles = [BATTLE.match(line) for line in actual if BATTLE.match(line)]
            if len(symbolic_battles) != len(resolved_battles):
                raise ContentError('CONFLICT', entry['path'], label, 'CPP battle views differ')
            for ordinal, ((index, battle), resolved) in enumerate(zip(symbolic_battles, resolved_battles)):
                if ordinal not in entry.get('selectedBattleOrdinals', range(len(symbolic_battles))):
                    continue
                discovered.append({'label': label, 'sourcePath': entry['path'], 'sourceLine': entry['line'],
                                   'commandIndex': index, 'occurrence': occurrence,
                                   'command': battle[0].strip(), 'macro': battle[1],
                                   'arguments': battle[2], 'resolvedArguments': resolved[2],
                                   'directCaller': index == 0 and len(entries) == 1,
                                   'assemblerConditional': entry['assemblerConditional']})
    return discovered
