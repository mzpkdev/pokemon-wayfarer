"""Use the ROM preprocessor and flags; feature selection is owned by domains."""
import shlex
import subprocess
from .common import ContentError

FEATURES = ('IS_WAYFARER', 'IS_HNS', 'IS_EMERALD', 'IS_FRLG',
            'WAYFARER_TR_MARTS_ENABLED', 'B_TRAINER_PARTY_SCALING',
            'B_GYM_LEADER_SCALING', 'B_LEAGUE_SCALING')

def preprocess(root, cpp, cppflags, text):
    flags = shlex.split(cppflags) if isinstance(cppflags, str) else list(cppflags)
    process = subprocess.run([cpp, *flags, '-I', 'include', '-P', '-'],
                             input=text, cwd=root, text=True, capture_output=True)
    if process.returncode:
        raise ContentError('SCHEMA', '<configuration>', cpp, process.stderr)
    return process.stdout

def numeric_defines(root, cpp, cppflags):
    source = '#define TRUE 1\n#define FALSE 0\n#include "constants/global.h"\n'
    source += '#include "config/wayfarer_marts.h"\n#include "config/trainer_party_scaling.h"\n'
    source += '#ifdef LEAFGREEN\nGAMEPLAY_DEFINE_GAMEPLAY_LEAFGREEN 1\n#else\nGAMEPLAY_DEFINE_GAMEPLAY_LEAFGREEN 0\n#endif\n'
    for name in FEATURES:
        source += f'\n#if {name}\nGAMEPLAY_DEFINE_{name} 1\n#else\nGAMEPLAY_DEFINE_{name} 0\n#endif\n'
    output = preprocess(root, cpp, cppflags, source)
    return {line.split()[0][len('GAMEPLAY_DEFINE_'):]: int(line.split()[1])
            for line in output.splitlines() if line.startswith('GAMEPLAY_DEFINE_')}
