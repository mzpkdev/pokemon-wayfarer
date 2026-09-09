"""Leaf parser for production mart stock profiles."""
import re
from pathlib import Path
from .common import ContentError
from .configuration import preprocess


def block_after(text, marker):
    start = text.index(marker)
    return text[start:text.index('};', start)]


def strip_comments(text):
    return re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)


def parse_profiles(text: str) -> dict[str, dict]:
    text = strip_comments(text)
    profiles: dict[str, dict] = {}
    table = block_after(text, "sWayfarerMartProfiles")
    for profile_id, macro, arguments in re.findall(
        r"\[(MART_PROFILE_[A-Z0-9_]+)\]\s*=\s*(MART_PROFILE_[A-Z_]+)\(([^)]*)\)", table
    ):
        args = [value.strip() for value in arguments.split(",")]
        if macro == "MART_PROFILE_WITH_RETAINED":
            signature, retained, common_mask, pp_recovery, category = args
        elif macro == "MART_PROFILE_NO_RETAINED":
            signature, common_mask, pp_recovery, category = args
            retained = None
        elif macro == "MART_PROFILE_FACILITY":
            retained, category = args
            signature = None
            common_mask = "MART_COMMON_ALL"
            pp_recovery = "TRUE"
        elif macro == "MART_PROFILE_EMPTY_FACILITY":
            (category,) = args
            signature = None
            retained = None
            common_mask = "MART_COMMON_ALL"
            pp_recovery = "TRUE"
        else:
            raise ValueError(f"unsupported profile macro {macro}")
        if profile_id in profiles:
            raise ValueError(f'duplicate stock profile {profile_id}')
        profiles[profile_id] = {
            "signature_array": signature,
            "retained_array": retained,
            "common_category_mask": common_mask,
            "supports_pp_recovery": pp_recovery == "TRUE",
            "category_mask": category,
        }
    authored = set(re.findall(r'\[(MART_PROFILE_[A-Z0-9_]+)\]\s*=', table[table.index('{') + 1:]))
    if authored != set(profiles):
        raise ValueError(f'unsupported stock profile initializer: {sorted(authored - set(profiles))}')
    return profiles



def load_profile_ids(root, cpp, cppflags):
    path = Path(root) / 'src/data/wayfarer_marts.h'
    text = strip_comments(path.read_text())
    # The generated clerk projection is downstream of this leaf adapter.
    text = text.replace('#include "gameplay_mart_clerks.inc"', '')
    # Protect profile names and typed initializers while CPP selects source rows,
    # including conditions surrounding the whole table.
    text = re.sub(r'\[(MART_PROFILE_[A-Z0-9_]+)\]', r'[GAMEPLAY_AUTHORED_\1]', text)
    text = re.sub(r'=\s*(MART_PROFILE_[A-Z_]+)\(', r'= GAMEPLAY_AUTHORED_\1(', text)
    header = '#define TRUE 1\n#define FALSE 0\n#include "constants/global.h"\n#include "config/wayfarer_marts.h"\n#include "constants/wayfarer_marts.h"\n'
    selected = preprocess(root, cpp, cppflags, header + text)
    if 'sWayfarerMartProfiles' not in selected:
        return {}
    profiles = parse_profiles(selected.replace('GAMEPLAY_AUTHORED_', ''))
    markers = ''.join(f'GAMEPLAY_PROFILE_VALUE_{name} {name}\n' for name in profiles)
    values = preprocess(root, cpp, cppflags, header + markers)
    seen = set()
    resolved = {}
    for line in values.splitlines():
        if not line.startswith('GAMEPLAY_PROFILE_VALUE_'):
            continue
        name, value = line.split(maxsplit=1)
        name = name.removeprefix('GAMEPLAY_PROFILE_VALUE_')
        if not re.fullmatch(r'(?:0[xX][0-9a-fA-F]+|[0-9]+)', value):
            raise ContentError('UNRESOLVED', path, name, value)
        number = int(value, 0)
        if not 0 < number < 65535:
            raise ContentError('CAPACITY', path, name, value)
        if number in seen:
            raise ContentError('CONFLICT', path, name, 'duplicate numeric profile')
        seen.add(number)
        resolved[name] = number
    return resolved
