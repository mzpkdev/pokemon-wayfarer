"""Select production League records for the ROM test table without copying parties."""
import json
from pathlib import Path
import re
import sys


def select_parties(game):
    inventory = json.loads(Path(__file__).with_name('inventory.json').read_text())['trainers']
    selected = []
    for row in inventory:
        filename = 'trainers.party' if row['tier'] == 3 else 'trainers_hns.party'
        source = (game / 'src/data' / filename).read_text()
        pattern = r'^=== ' + re.escape(row['trainer']) + r' ===\n.*?(?=^=== |\Z)'
        matches = re.findall(pattern, source, re.M | re.S)
        if len(matches) != 1:
            raise ValueError(f"Expected one production party for {row['trainer']}, found {len(matches)}")
        if row['id'] <= 14:
            raise ValueError(f"League ID collides with trainer_control.party: {row['id']}")
        condition = 'defined(POKEMON_WAYFARER) || defined(EMERALD)' if row['tier'] == 3 else 'defined(POKEMON_WAYFARER) || defined(POKEMON_HNS)'
        selected.append('#if ' + condition + '\n' + matches[0].strip() + '\n#endif')
    preamble = (game / 'src/data/trainers.party').read_text().split('/*', 1)[0]
    return preamble + '\n\n'.join(selected) + '\n'


if __name__ == '__main__':
    sys.stdout.write(select_parties(Path(__file__).resolve().parents[2]))
