"""Compiled roster adapter using trainerproc and its existing output parser."""
from pathlib import Path
import subprocess
import shlex
import tempfile
from tools.trainer_scaling.generate import parse_output, resolve_rosters
from .common import ContentError
from .configuration import preprocess

SOURCES = {'wayfarer': ('trainers_hns.party', 'trainers.party'),
           'hns': ('trainers_hns.party',), 'emerald': ('trainers.party',),
           'firered': ('trainers_frlg.party',), 'leafgreen': ('trainers_frlg.party',)}

def load_trainers(root, product, cpp, cppflags):
    root = Path(root)
    result = {}
    with tempfile.TemporaryDirectory(prefix='gameplay-rosters-') as temporary:
        binary = Path(temporary) / 'trainerproc'
        proc = subprocess.run(['cc', '-O2', str(root / 'tools/trainerproc/main.c'), '-o', str(binary)],
                              text=True, capture_output=True)
        if proc.returncode:
            raise ContentError('UNRESOLVED', '<trainerproc>', '', proc.stderr)
        for name in SOURCES[product]:
            source = root / 'src/data' / name
            flags = shlex.split(cppflags) if isinstance(cppflags, str) else list(cppflags)
            text = preprocess(root, cpp, [*flags, "-traditional-cpp"], source.read_text())
            output = Path(temporary) / (name + '.h')
            proc = subprocess.run([str(binary), '-i', f'src/data/{name}', '-o', str(output), '-'],
                                  input=text, cwd=root, text=True, capture_output=True)
            if proc.returncode:
                raise ContentError('UNRESOLVED', source, '', proc.stderr)
            parsed = parse_output(output.read_text(), f'src/data/{name}')
            if result.keys() & parsed.keys():
                raise ContentError('DUPLICATE', source, '', sorted(result.keys() & parsed.keys()))
            result.update(parsed)
    return resolve_rosters(result)
