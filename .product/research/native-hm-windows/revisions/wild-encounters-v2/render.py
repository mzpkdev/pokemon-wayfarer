"""Render a region's table data into its spec's tables.

Usage: python3 render.py [region ...]        (default: every region)
Rewrites everything after the "### Tables" heading of specs/<region>-encounter-tables.md from game/src/data/wild_encounters_v2/<region>.json,
keeping the hand-written preamble above it. Add --check to compare without writing."""
import json, os, sys
from collections import defaultdict
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '../../../../../game/tools/wild_encounters/v2'))  # check.py moved there
import check
SPECS = os.path.join(check.REPO, 'specs')
chain, BABIES, META, WEIGHTS = check.chain, check.BABIES, check.META, check.WEIGHTS

SPECIAL = {'NIDORAN_F': 'Nidoran♀', 'NIDORAN_M': 'Nidoran♂', 'MR_MIME': 'Mr. Mime', 'MIME_JR': 'Mime Jr.', 'FARFETCHD': "Farfetch'd",
           'PORYGON_Z': 'Porygon-Z', 'HO_OH': 'Ho-Oh',
           'SIRFETCHD': "Sirfetch'd", 'MR_RIME': 'Mr. Rime', 'FLABEBE': 'Flabébé'}
FORMS = {'POM_POM': 'Pom-Pom', 'PAU': "Pa'u", 'BAILE': 'Baile', 'SENSU': 'Sensu', 'MIDDAY': 'Midday', 'MIDNIGHT': 'Midnight',
         'DUSK': 'Dusk', 'OWN_TEMPO': 'Own Tempo', 'WHITE_STRIPED': 'White-Striped'}
FORM_FAMILIES = {'SCATTERBUG', 'SPEWPA', 'VIVILLON', 'FLABEBE', 'FLOETTE', 'FLORGES', 'PUMPKABOO', 'GOURGEIST'}
HYPHENATED = {'JANGMO_O': 'Jangmo-o', 'HAKAMO_O': 'Hakamo-o', 'KOMMO_O': 'Kommo-o'}
def nice(s):
    if s in SPECIAL: return SPECIAL[s]
    if s in HYPHENATED: return HYPHENATED[s]
    if s.endswith('_ALOLA'): return 'Alolan ' + nice(s[:-6])
    if s.endswith('_GALAR'): return 'Galarian ' + nice(s[:-6])
    if s.endswith('_HISUI'): return 'Hisuian ' + nice(s[:-6])
    root, _, rest = s.partition('_')
    if root in FORM_FAMILIES and rest: return f"{nice(root)} ({rest.replace('_', ' ').title()})"
    for k, v in FORMS.items():
        if s.endswith('_' + k): return f'{nice(s[:-len(k) - 1])} ({v})'
    return s.replace('_', ' ').title()
def label(cap):
    c = chain(cap)
    return nice(cap) if len(c) == 1 else f'{nice(c[-1])}–{nice(cap)}'

WATER_NAME = {'pond': 'ponds and rivers', 'sea': 'coast and sea', 'cold': 'cold water', 'cave': 'cave water', 'underwater': 'underwater'}
METHOD = {'land': 'Land', 'surf': 'Surfing', 'fish': 'Fishing', 'rock': 'Trees and rocks'}
_TILES = check.TILES

def rock_label(m):
    # HNS maps share one table for Headbutt trees and Rock Smash rocks; Emerald and FRLG have only rocks.
    return 'Trees and rocks' if _TILES.get(m, {}).get('ver', 'hns') == 'hns' else 'Rock Smash rocks'
LAND_W = [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1]
FIVE_W = [60, 30, 5, 4, 1]
ROD = {'Old': [38, 22, 10, 8, 8, 4, 3, 3, 2, 2], 'Good': [25, 18, 12, 10, 9, 7, 6, 5, 4, 4], 'Super': [12, 10, 11, 10, 10, 10, 10, 9, 9, 9]}

def table(meth, t):
    rows = []
    if meth == 'fish':
        rows.append('| Entry | Old Rod | Good Rod | Super Rod | Day | Night |\n| --- | --- | --- | --- | --- | --- |')
        for i in range(10):
            rows.append(f'| {i+1} | {ROD["Old"][i]}% | {ROD["Good"][i]}% | {ROD["Super"][i]}% | {label(t["day"][i])} | {label(t["night"][i])} |')
    else:
        w = LAND_W if meth == 'land' else FIVE_W
        rows.append('| Slot | Weight | Day | Night |\n| --- | --- | --- | --- |')
        for i in range(len(w)):
            rows.append(f'| {i+1} | {w[i]}% | {label(t["day"][i])} | {label(t["night"][i])} |')
    return '\n'.join(rows)

def render(region, data):
    out = []
    places = defaultdict(list)
    for m, meta in META.items():
        if meta['region'] == region: places[meta['place']].append(m)
    order = {'Road': 0, 'Wilds': 1, 'Outlands': 2, 'Dungeon': 3}
    for place, maps in sorted(places.items(), key=lambda kv: (order[META[kv[1][0]]['reach']], list(META).index(kv[1][0]))):
        meta0 = META[maps[0]]
        out.append(f'#### {place}\n\n{meta0["reach"]}, {meta0["band"]}.\n')
        for m in maps:
            out.append(f'**`{m}`**\n')
            wt = data.get(m, {}).get('water_type')
            if wt: out.append(f'Water type: {WATER_NAME[wt]}.\n')
            for meth in ('land', 'surf', 'fish', 'rock'):
                if meth not in data.get(m, {}): continue
                out.append(f'*{rock_label(m) if meth == "rock" else METHOD[meth]}*\n\n' + table(meth, data[m][meth]) + '\n')
    return '\n'.join(out)

def coverage(region, data):
    where = defaultdict(set)
    for m, t in data.items():
        for meth in t.values():
            if not isinstance(meth, dict): continue
            for slots in meth.values():
                for cap in slots:
                    for s in chain(cap): where[s].add(META[m]['place'])
    rows = []
    for s in sorted(where):
        rows.append(f'| {nice(s)} | {", ".join(sorted(where[s])[:4])}{" and more" if len(where[s]) > 4 else ""} |')
    return '| Species | Catchable at |\n| --- | --- |\n' + '\n'.join(rows)

MARK = '### Tables\n\n'

def spec_path(region): return os.path.join(SPECS, region.lower() + '-encounter-tables.md')

def rendered(region):
    data = json.load(open(os.path.join(check.HERE, region.lower() + '.json')))
    return render(region, data) + '\n\n### Coverage checklist\n\n' + coverage(region, data) + '\n'

if __name__ == '__main__':
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    regions = [r for r in check.CFG if not args or r.lower() in [a.lower() for a in args]]
    stale = False
    for region in regions:
        path = spec_path(region); text = open(path).read()
        new = text[:text.index(MARK) + len(MARK)] + rendered(region)
        if new == text: print(region, 'up to date'); continue
        stale = True
        if '--check' in sys.argv: print(region, 'differs from its data')
        else: open(path, 'w').write(new); print(region, 'rendered into', path)
    sys.exit(1 if stale and '--check' in sys.argv else 0)
