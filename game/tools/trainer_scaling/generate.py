#!/usr/bin/env python3
"""Inventory compiled Wayfarer rosters and validate reviewed ID policies."""
from __future__ import annotations
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
TOOL = Path(__file__).resolve().parent
MANIFEST = TOOL / 'classification.json'
SEVII_ALLOCATION = ROOT.parent / 'docs/sevii-trainer-implementation/allocation-inventory.json'
SEVII_POLICY = TOOL / 'sevii_policy.json'
SEVII_ROSTER = ROOT / 'src/data/trainers_wayfarer_sevii.h'
COAST_ALLOCATION = ROOT.parent / 'docs/coast-trainer-implementation/allocation-inventory.json'
COAST_ROSTER = ROOT / 'src/data/trainers_wayfarer_coast.h'
OUTPUT = ROOT / 'src/data/trainer_scaling'
SOURCES = ('trainers_hns.party', 'trainers.party')
POLICIES = {'EXCLUDED': 0, 'ORDINARY': 1, 'GYM_MEMBER': 2, 'GYM_LEADER': 3}
ID_RE = re.compile(r'\bTRAINER_[A-Z0-9_]+\b')

class ValidationError(ValueError):
    pass

def command(args, **kwargs):
    result = subprocess.run(args, text=True, capture_output=True, **kwargs)
    if result.returncode:
        raise ValidationError(f'{args[0]} failed: {result.stderr}')
    return result.stdout

def balanced(source, start):
    depth = 0
    quote = None
    escape = False
    for i in range(start, len(source)):
        ch = source[i]
        if quote:
            if escape: escape = False
            elif ch == '\\': escape = True
            elif ch == quote: quote = None
        elif ch in ('"', "'"): quote = ch
        elif ch == '{': depth += 1
        elif ch == '}':
            depth -= 1
            if not depth: return source[start + 1:i], i + 1
    raise ValidationError('unbalanced trainerproc output')

def fields(body):
    result = {}
    for match in re.finditer(r'\.(\w+)\s*=\s*([^\n]+)', body):
        key, value = match.groups()
        value = value.rstrip(', ')
        if key == 'moves': value = re.findall(r'MOVE_[A-Z0-9_]+', value)
        elif value.isdigit(): value = int(value)
        result[key] = value
    moves = re.search(r'\.moves\s*=\s*\{', body)
    if moves:
        tuple_body, _ = balanced(body, moves.end() - 1)
        result['moves'] = re.findall(r'MOVE_[A-Z0-9_]+', tuple_body)
    return result

def parse_output(text, source):
    text = re.sub(r'^#.*$', '', text, flags=re.M)
    result = {}
    for match in re.finditer(r'\[(DIFFICULTY_\w+)\]\[(TRAINER_\w+)\]\s*=\s*\{', text):
        variant, trainer = match.groups()
        body, _ = balanced(text, match.end() - 1)
        party = re.search(r'\.party\s*=\s*\(const struct TrainerMon\[\]\)\s*\{', body)
        record = fields(body[:party.start()] if party else body)
        record['source'] = source
        record['slots'] = []
        if party:
            slots, _ = balanced(body, party.end() - 1)
            pos = 0
            while (pos := slots.find('{', pos)) != -1:
                slot, pos = balanced(slots, pos)
                record['slots'].append(fields(slot))
        if variant in result.setdefault(trainer, {}):
            raise ValidationError(f'duplicate {trainer}/{variant}')
        result[trainer][variant] = record
    return result

def load_inventory():
    """Run the real party compiler; no inferred parsing of authored rosters."""
    result = {}
    with tempfile.TemporaryDirectory(prefix='trainer-scaling-') as directory:
        directory = Path(directory)
        binary = directory / 'trainerproc'
        command(['cc', '-O2', str(ROOT / 'tools/trainerproc/main.c'), '-o', str(binary)])
        for name in SOURCES:
            source = ROOT / 'src/data' / name
            preprocessed = command(['cpp', '-traditional-cpp', '-P', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS', '-DIS_WAYFARER=1', '-DIS_HNS=1', '-DIS_FRLG=0', '-DIS_EMERALD=0', str(source)])
            header = directory / (name + '.h')
            command([str(binary), '-i', f'src/data/{name}', '-o', str(header), '-'], input=preprocessed)
            parsed = parse_output(header.read_text(), f'src/data/{name}')
            overlap = result.keys() & parsed.keys()
            if overlap: raise ValidationError(f'duplicate source IDs: {sorted(overlap)}')
            result.update(parsed)
    # The selected roster is already trainerproc output generated from the
    # frozen FRLG source-key inventory.  Do not compile or inspect the full
    # FRLG roster here: only the allocated Wayfarer records are eligible.
    if not SEVII_ROSTER.is_file():
        raise ValidationError(f'missing generated Sevii selected roster: {SEVII_ROSTER}')
    selected = parse_output(SEVII_ROSTER.read_text(), 'src/data/trainers_wayfarer_sevii.h')
    overlap = result.keys() & selected.keys()
    if overlap: raise ValidationError(f'duplicate selected Sevii IDs: {sorted(overlap)}')
    result.update(selected)
    if not COAST_ROSTER.is_file():
        raise ValidationError(f'missing generated Coast roster: {COAST_ROSTER}')
    coast = parse_output(COAST_ROSTER.read_text(), 'src/data/trainers_wayfarer_coast.h')
    overlap = result.keys() & coast.keys()
    if overlap: raise ValidationError(f'duplicate selected Coast IDs: {sorted(overlap)}')
    result.update(coast)
    return result

def load_sevii_manifest():
    """Expand the frozen allocation into reviewed scaling classifications."""
    try:
        allocation = json.loads(SEVII_ALLOCATION.read_text())['allocation']['allocations']
        policy = json.loads(SEVII_POLICY.read_text())
    except (OSError, KeyError, TypeError, json.JSONDecodeError) as error:
        raise ValidationError(f'cannot load Sevii scaling inventory: {error}')
    if policy.get('version') != 1 or policy.get('ordinary_policy') != 'ORDINARY':
        raise ValidationError('unsupported Sevii scaling policy inventory')
    story = policy.get('story')
    if not isinstance(story, dict): raise ValidationError('Sevii story scaling policy must be an object')
    if len(allocation) != 136 or [row.get('slot') for row in allocation] != list(range(136)):
        raise ValidationError('Sevii selected allocation is not the frozen dense 136-slot inventory')
    rows = []
    for row in allocation:
        ident, source, owner = row.get('id'), row.get('source_trainer'), row.get('owner')
        if not isinstance(ident, str) or not isinstance(source, str):
            raise ValidationError('Sevii allocation has an invalid selected Trainer identity')
        if owner == 'ordinary_trainer':
            item = {'policy': 'ORDINARY'}
        elif owner == 'story':
            item = story.get(source)
            if not isinstance(item, dict) or item.get('policy') not in POLICIES:
                raise ValidationError(f'Sevii planned story Trainer has no reviewed scaling policy: {source}')
        else:
            raise ValidationError(f'Sevii allocation has unsupported owner: {owner}')
        record = {
            'id': ident,
            'policy': item['policy'],
            'region': 'Sevii',
            'evidence': [{'path': 'src/data/trainers_wayfarer_sevii.h', 'symbol': ident}],
        }
        if record['policy'] == 'EXCLUDED':
            reason = item.get('reason')
            if not isinstance(reason, str) or not reason:
                raise ValidationError(f'Sevii excluded Trainer lacks a reviewed reason: {source}')
            record['reason'] = reason
        rows.append(record)
    expected_story = {row['source_trainer'] for row in allocation if row['owner'] == 'story'}
    if set(story) != expected_story:
        raise ValidationError('Sevii story scaling inventory does not exactly match planned allocation keys')
    return rows

def load_coast_manifest():
    """Expand the reviewed Coast allocation into Trainer Rating policies."""
    try:
        allocation = json.loads(COAST_ALLOCATION.read_text())['allocation']
        rows = allocation['allocations']
    except (OSError, KeyError, TypeError, json.JSONDecodeError) as error:
        raise ValidationError(f'cannot load Coast scaling inventory: {error}')
    if (allocation.get('base'), allocation.get('count'), allocation.get('next_id')) != (1651, 56, 1707):
        raise ValidationError('Coast Trainer allocation extent drift')
    if [row.get('slot') for row in rows] != list(range(56)):
        raise ValidationError('Coast Trainer allocation is not dense')
    result = []
    for row in rows:
        ident, policy = row.get('id'), row.get('policy')
        if not isinstance(ident, str) or policy not in POLICIES:
            raise ValidationError('Coast Trainer allocation has an invalid policy row')
        result.append({
            'id': ident,
            'policy': policy,
            'region': 'Kanto',
            'evidence': [{'path': 'src/data/trainers_wayfarer_coast.h', 'symbol': ident}],
        })
    return result

def trainer_ids(names):
    # The first argument is quoted so preprocessing only expands the value.
    source = '#include "constants/opponents.h"\n' + '\n'.join(f'ID("{name}", {name})' for name in sorted(names))
    output = command(['cpp', '-P', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS', '-DIS_WAYFARER=1', '-DIS_HNS=1', '-I', str(ROOT / 'include'), '-'], input=source)
    result = {}
    for name, expression in re.findall(r'ID\("(TRAINER_\w+)",\s*(.*?)\)\s*$', output, re.M):
        if not re.fullmatch(r'[0-9 ()+\-]+', expression):
            raise ValidationError(f'unknown ID {name}: {expression}')
        result[name] = eval(expression, {'__builtins__': {}}, {})
    if set(result) != set(names): raise ValidationError('unresolved trainer constants')
    if len(set(result.values())) != len(result): raise ValidationError('duplicate numeric trainer IDs')
    return result

def resolve_rosters(records):
    result = {}
    def resolve(trainer, variant, chain=()):
        key = (trainer, variant)
        if key in chain: raise ValidationError(f'override cycle: {chain + (key,)}')
        if trainer not in records: raise ValidationError(f'override references missing {trainer}')
        variants = records[trainer]
        fixed_hoenn = any(row.get('source') == 'src/data/trainers.party' for row in variants.values())
        actual = variant if variant in variants and not fixed_hoenn else 'DIFFICULTY_NORMAL'
        if actual not in variants: raise ValidationError(f'{trainer} lacks normal difficulty')
        row = dict(variants[actual])
        money_slots = row.get('slots', [])
        money_size = row.get('partySize', 0)
        row['money_party_null'] = not bool(money_slots)
        row['money_level'] = money_slots[money_size - 1]['lvl'] if money_slots and 0 < money_size <= len(money_slots) else None
        row['money_trainer_class'] = row.get('trainerClass')
        if row.get('overrideTrainer'):
            parent = resolve(row['overrideTrainer'], variant, chain + (key,))
            row.update(slots=parent['slots'], owner=parent['owner'], owner_variant=parent['owner_variant'], poolSize=parent.get('poolSize', 0))
            row.setdefault('partySize', parent.get('partySize', 0))
        else:
            row.update(owner=trainer, owner_variant=actual)
        size = row.get('partySize', 0)
        if row['slots'] and (not isinstance(size, int) or not 1 <= size <= 6 or size > len(row['slots'])):
            raise ValidationError(f'{trainer}/{variant}: invalid party size {size}/{len(row["slots"])}')
        if row.get('poolSize', 0) not in (0, len(row['slots'])):
            raise ValidationError(f'{trainer}/{variant}: pool size mismatch')
        for slot in row['slots']:
            if not 1 <= slot.get('lvl', 0) <= 100: raise ValidationError(f'{trainer}/{variant}: invalid level')
            if not str(slot.get('species', '')).startswith('SPECIES_'): raise ValidationError(f'{trainer}: missing species')
        return row
    # Difficulty fallback and overrides are evaluated for every selectable variant.
    variants = {'DIFFICULTY_NORMAL', 'DIFFICULTY_EASY', 'DIFFICULTY_HARD'}
    variants.update(v for rows in records.values() for v in rows)
    for trainer in records:
        result[trainer] = {variant: resolve(trainer, variant) for variant in sorted(variants)}
    return result

def references():
    refs = defaultdict(set)
    non_opponents = set()
    for header in (ROOT / 'include/constants').glob('*.h'):
        if header.name.startswith('opponents'): continue
        non_opponents.update(re.findall(r'^#define\s+(TRAINER_[A-Z0-9_]+)', header.read_text(), re.M))
    # Follow the product-selected assembly includes rather than every map in
    # map_groups.json. Retired source maps remain catalogued for standalone
    # products, but their scripts and Trainer references are not in Wayfarer.
    event_source = (ROOT / 'data/event_scripts.s').read_text()
    event_source = '\n'.join(line for line in event_source.splitlines() if not line.startswith('#include'))
    event_source = command(['cpp', '-P', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS', '-DIS_WAYFARER=1', '-DIS_HNS=1', '-DIS_FRLG=0', '-DIS_EMERALD=0', '-'], input=event_source)
    paths = [ROOT / relative for relative in re.findall(r'\.include\s+"((?:data/maps/[^\"]+|data/scripts/[^\"]+))"', event_source) if '_frlg' not in relative.lower()]
    active_sources = {}
    for path in sorted(set(paths)):
        if not path.exists(): continue
        active_sources[path] = command(['cpp', '-P', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS', '-DIS_WAYFARER=1', '-DIS_HNS=1', '-DIS_FRLG=0', '-DIS_EMERALD=0', '-I', str(ROOT / 'include'), '-iquote', str(path.parent), '-'], input=path.read_text())
    selected = list(active_sources.items())
    for path, source in selected:
        for line in source.splitlines():
            if re.match(r'\s*(trainerbattle\w*|goto_if_(?:not_)?defeated|register_matchcall|settrainerflag|cleartrainerflag|setvar|setorcopyvar)\s', line, re.I):
                for trainer in ID_RE.findall(line.split('@', 1)[0]):
                    if trainer != 'TRAINER_NONE' and trainer not in non_opponents and not trainer.startswith('TRAINER_BATTLE_'): refs[trainer].add(str(path.relative_to(ROOT)))
    # Preprocess conditionals so standalone rematch registries do not leak in.
    source = (ROOT / 'src/battle_setup.c').read_text()
    source = '\n'.join(line for line in source.splitlines() if not line.startswith('#include'))
    source = command(['cpp', '-P', '-DPOKEMON_WAYFARER', '-DPOKEMON_HNS', '-DIS_WAYFARER=1', '-DIS_HNS=1', '-DIS_FRLG=0', '-'], input=source)
    for line in source.splitlines():
        if '= REMATCH(' in line:
            for trainer in ID_RE.findall(line): refs[trainer].add('src/battle_setup.c')
    return {key: sorted(value) for key, value in refs.items()}

def region_from_evidence(trainer, refs):
    if not trainer.endswith('_HNS'): return 'Hoenn'
    locations = ' '.join(refs)
    if any(word in locations for word in ('NewSinjoh', 'Sinjoh_', 'Route49_', 'Route50_', 'Snowswept')): return 'Sinjoh'
    if any(word in locations for word in ('Melemele', 'Akala', 'Ulaula', 'Poni', 'Alola')): return 'Alola'
    if any(word in locations for word in ('Pallet', 'Viridian', 'Pewter', 'Cerulean', 'Vermilion', 'Lavender', 'Celadon', 'Fuchsia', 'Saffron', 'Cinnabar', 'Indigo', 'RockTunnel', 'MtMoon', 'PokemonMansion', 'PowerPlant', 'Diglett', 'Seafoam', 'VictoryRoad')): return 'Kanto'
    if any(word in locations for word in ('NewBark', 'Cherrygrove', 'Violet', 'Azalea', 'Goldenrod', 'Ecruteak', 'Olivine', 'Cianwood', 'Mahogany', 'Blackthorn', 'Ilex', 'UnionCave', 'Slowpoke', 'BurnedTower', 'SproutTower', 'RuinsOfAlph', 'NationalPark', 'WhirlIsland', 'MtMortar', 'LakeOfRage', 'IcePath', 'DragonDen', 'DragonsDen', 'TinTower', 'BellTower', 'MtSilver', 'DarkCave', 'MtTolarance', 'Tohjo')): return 'Johto'
    routes = [int(value) for value in re.findall(r'Route(\d+)', locations)]
    if routes: return 'Kanto' if min(routes) <= 28 else 'Johto'
    return 'HNS unplaced'


def propose(records, refs):
    """Discovery proposes reviewable records; generation never infers policy."""
    bosses = ('LEADER', 'ELITE_FOUR', 'CHAMPION', 'RIVAL', 'ADMIN', 'AQUA_LEADER', 'MAGMA_LEADER', 'ARENA_TYCOON', 'DOME_ACE', 'FACTORY_HEAD', 'PALACE_MAVEN', 'PIKE_QUEEN', 'PYRAMID_KING', 'SALON_MAIDEN', 'RS_PROTAG')
    special = ('WALLY', 'BRENDAN', 'MAY_', 'STEVEN', 'RED_', 'EUSINE', 'SAMSON_OAK', 'PHANTONOMY')
    rows = []
    for trainer, variants in sorted(records.items()):
        if not any(row['slots'] for row in variants.values()): continue
        row = variants['DIFFICULTY_NORMAL']
        cls = row.get('trainerClass', '')
        evidence = refs.get(trainer, [])
        excluded = any(x in cls for x in bosses) or any(trainer.removeprefix('TRAINER_').startswith(x) for x in special)
        gym = any('Gym' in ref for ref in evidence)
        policy = 'EXCLUDED' if excluded else 'GYM_MEMBER' if gym else 'ORDINARY'
        reason = 'Authored boss, rival, facility head, or special story opponent.' if excluded else ''
        entry = dict(id=trainer, policy=policy, region=region_from_evidence(trainer, evidence), evidence=[{'path': row['source'], 'symbol': trainer, 'role': cls}] + [{'path': ref, 'symbol': trainer} for ref in evidence])
        if reason: entry['reason'] = reason
        if gym and any('/maps/' in ref and 'Gym' not in ref for ref in evidence): entry['role_review'] = 'Gym membership takes precedence for this shared ID at every location.'
        rows.append(entry)
    return {'version': 1, 'records': rows, 'move_exceptions': []}

def validate_manifest(manifest, records, ids, refs):
    if manifest.get('version') != 1 or isinstance(manifest.get('version'), bool): raise ValidationError('unsupported classification manifest version')
    seen = set()
    populated = {trainer for trainer, variants in records.items() if any(row['slots'] for row in variants.values())}
    for row in manifest.get('records', []):
        trainer = row.get('id')
        if trainer in seen: raise ValidationError(f'duplicate manifest ID {trainer}')
        seen.add(trainer)
        if trainer not in populated or trainer not in ids: raise ValidationError(f'unknown or empty manifest ID {trainer}')
        if row.get('policy') not in POLICIES: raise ValidationError(f'{trainer}: unknown policy')
        if row['policy'] == 'EXCLUDED' and not row.get('reason'): raise ValidationError(f'{trainer}: excluded without reason')
        if not row.get('evidence'): raise ValidationError(f'{trainer}: missing evidence')
        for evidence in row['evidence']:
            path = ROOT / evidence['path']
            if not path.is_file() or evidence.get('symbol', trainer) not in path.read_text(): raise ValidationError(f'{trainer}: stale evidence {evidence}')
            if evidence.get('role') and evidence['role'] not in {record.get('trainerClass') for record in records[trainer].values()}: raise ValidationError(f'{trainer}: stale authored role {evidence}')
        locations = refs.get(trainer, [])
        if any('Gym' in ref for ref in locations) and any('/maps/' in ref and 'Gym' not in ref for ref in locations) and row['policy'] != 'EXCLUDED' and not row.get('role_review'):
            raise ValidationError(f'{trainer}: shared Gym/ordinary role requires review')
    missing = populated - seen
    if missing: raise ValidationError(f'unclassified IDs: {sorted(missing)}')
    for trainer in refs:
        if trainer not in populated: raise ValidationError(f'script/rematch references hole or empty roster: {trainer}: {refs[trainer]}')
    exception_keys = set()
    for exception in manifest.get('move_exceptions', []):
        owner, variant, slot = exception.get('owner'), exception.get('variant'), exception.get('slot')
        key = (owner, variant, slot)
        if key in exception_keys: raise ValidationError(f'duplicate move exception {key}')
        exception_keys.add(key)
        if owner not in records or variant not in records[owner] or not isinstance(slot, int) or not 0 <= slot < len(records[owner][variant]['slots']):
            raise ValidationError(f'invalid move exception {key}')
        if records[owner][variant]['owner'] != owner or not exception.get('reason'):
            raise ValidationError(f'move exception requires resolved owner and reviewed reason: {key}')

def review_report(report, manifest):
    lines = ['# Trainer scaling inventory and balance review', '', 'This report checks authored source records and projected parties. It does not establish playable balance or report emulator playtesting.', '', '| Policy | Populated IDs |', '| --- | ---: |']
    lines += [f'| {policy} | {count} |' for policy, count in sorted(report['counts'].items())]
    lines += ['', '| Region | Populated IDs |', '| --- | ---: |']
    lines += [f'| {region} | {count} |' for region, count in sorted(report['region_counts'].items())]
    lines += ['', 'HNS unplaced means a compiled roster lacks a direct regional map reference. The manifest preserves that uncertainty instead of assigning a region from its Trainer name.', '', 'All eligible slots use generated level-up moves unless listed in the reviewed exception manifest. Bosses, rivals, facility heads, the eight Kimono story opponents, and four Sinjoh Plate gates retain authored construction.', '', '## Level and reward inputs', '', 'Baseline anchors: 0:7, 4:8, 8:10, 16:15, 30:22, 40:34, 55:52, 65:72, 80:92. Each slot adds its bounded authored-level adjustment; Gym members add two levels. Final levels stop at 100. The player soft cap does not clamp opponents.', '', report['reward_policy'], '']
    balance = report.get('balance', {})
    if balance:
        lines += [f"The audit evaluated {balance['evaluated_slot_ratings_modes']:,} slot, Rating, and learnset-mode combinations with {len(balance['structural_failures'])} structural failures.", '', '## Highest early parties', '', '| Trainer | Party size | Rating 0 species and levels |', '| --- | ---: | --- |']
        for party in balance['highest_early_parties']:
            mons = ', '.join(f"{slot['species'].removeprefix('SPECIES_')} {slot['level']}" for slot in party['modes']['normal'])
            lines.append(f"| {party['id']} | {party['party_size']} | {mons} |")
        lines += ['', '## Largest early parties', '', '| Trainer | Party size | Rating 0 species and levels |', '| --- | ---: | --- |']
        for party in balance['largest_early_parties']:
            mons = ', '.join(f"{slot['species'].removeprefix('SPECIES_')} {slot['level']}" for slot in party['modes']['normal'])
            lines.append(f"| {party['id']} | {party['party_size']} | {mons} |")
        outcomes = balance['outcomes']
        lines += ['', '## Species and retained-field observations', '']
        for key, label in [('custom_moves_replaced', 'Custom authored moves replaced'), ('ability_fallback', 'Authored ability requires fallback'), ('gender_adjustment', 'Authored gender requires adjustment'), ('gimmick_suppression', 'Incompatible gimmick suppressed'), ('held_item_review', 'Held item retained after species reversal'), ('above_soft_cap', 'Opponent above player soft cap'), ('high_bst_no_predecessor', 'High-stat species without numeric predecessor')]:
            lines.append(f"- {label}: {sum(bool(outcome.get(key)) for outcome in outcomes)} distinct projected outcomes. Exact affected IDs, variants, slots, and Rating intervals are indexed in inventory.json.")
        lines += ['', '## Representative parties', '', 'The machine-readable inventory contains full normal and legacy moves, abilities, XP species inputs, authored money inputs, and every pool candidate for representative parties at Ratings 0, 4, 8, 16, 30, 40, 55, 63, 65, 68, 76, and 80. Its slot and projection tables cover all other eligible source slots.', '', '## Remaining validation', '', 'Playtest early, middle, and late careers in every included region, including Gym doubles, utility-heavy learnsets, powerful species without numeric predecessors, and the largest early parties above. Passing structural validation is not a balance approval.']
    return '\n'.join(lines) + '\n'


def write_output(path, content, check):
    if check:
        if not path.exists() or path.read_text() != content: raise ValidationError(f'stale generated output: {path.relative_to(ROOT)}')
    else:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)

def generate(check=False, proposal=None, no_audit=False):
    raw = load_inventory()
    records = resolve_rosters(raw)
    ids = trainer_ids(raw)
    refs = references()
    if proposal:
        proposal.write_text(json.dumps(propose(records, refs), indent=2) + '\n')
        return
    manifest = json.loads(MANIFEST.read_text())
    manifest = dict(manifest, records=manifest.get('records', []) + load_sevii_manifest() + load_coast_manifest())
    validate_manifest(manifest, records, ids, refs)
    rows = sorted(manifest['records'], key=lambda row: ids[row['id']])
    header = '// Generated by tools/trainer_scaling/generate.py; edit classification.json, sevii_policy.json, or the Coast trainer allocation.\n'
    header += 'static const u8 sTrainerScalingPolicies[TRAINERS_COUNT] =\n{\n'
    header += ''.join(f'    [{row["id"]}] = {POLICIES[row["policy"]]},\n' for row in rows)
    header += '};\n'
    write_output(OUTPUT / 'policies.h', header, check)
    exceptions = '// Generated by tools/trainer_scaling/generate.py.\nstatic const struct TrainerScalingMoveException sTrainerScalingMoveExceptions[] =\n{\n'
    exceptions += ''.join(f'    {{{row["owner"]}, {row["variant"]}, {row["slot"]}}},\n' for row in sorted(manifest['move_exceptions'], key=lambda row: (row['owner'], row['variant'], row['slot'])))
    exceptions += '    {TRAINERS_COUNT, 0, 0},\n};\n'
    write_output(OUTPUT / 'move_exceptions.h', exceptions, check)
    import sys
    sys.path.insert(0, str(ROOT / 'tools/wild_encounters'))
    import wild_encounters_to_header as wild
    # Gym Leaders have their own authored six-slot curve.  They intentionally
    # do not feed the ordinary predecessor table (or the ordinary audit).
    eligible = {row['id'] for row in rows if row['policy'] in ('ORDINARY', 'GYM_MEMBER')}
    species = {slot['species'] for trainer in eligible for record in records[trainer].values() for slot in record['slots']}
    metadata = wild.load_trainer_species_metadata(wild.DEFAULT_SPECIES_METADATA, wild.DEFAULT_SPECIES_INFO, wild.species_ids(wild.DEFAULT_SPECIES), species)
    write_output(OUTPUT / 'predecessors.h', wild.render_trainer_predecessor_header(metadata), check)
    report = {'counts': dict(Counter(row['policy'] for row in rows)), 'region_counts': dict(Counter(row['region'] for row in rows)), 'populated_ids': len(rows), 'unresolved_classification_candidates': [], 'structural_failures': [], 'move_exceptions': manifest['move_exceptions'], 'excluded': [{'id': row['id'], 'reason': row['reason']} for row in rows if row['policy'] == 'EXCLUDED'], 'records': raw, 'roster_inventory': 'Trainerproc source variants appear once in records. Balance slots name resolved owners and all selectable difficulty variants, including fallbacks.', 'context_exclusions': ['Frontier', 'Trainer Hill', 'e-Reader', 'Secret Base', 'rental', 'link', 'recorded', 'external', 'partner', 'player', 'raw Trainer pointer/debug'], 'reward_policy': 'Battle XP reads effective species and levels; prize money retains authored party levels and class multiplier.'}
    if not no_audit:
        from audit import build_audit
        report['balance'] = build_audit(records, manifest)
        if report['balance'].get('structural_failures'):
            raise ValidationError(f"slot audit structural failures: {report['balance']['structural_failures']}")
    write_output(OUTPUT / 'audit.md', review_report(report, manifest), check)
    write_output(OUTPUT / 'inventory.json', json.dumps(report, separators=(',', ':'), sort_keys=True) + '\n', check)
    print(json.dumps({'populated_ids': len(rows), 'counts': report['counts'], 'structural_failures': []}, sort_keys=True))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--propose', type=Path, help='write a candidate manifest for explicit review')
    parser.add_argument('--no-audit', action='store_true', help=argparse.SUPPRESS)
    args = parser.parse_args()
    try: generate(args.check, args.propose, args.no_audit)
    except (ValidationError, OSError, ValueError) as error: parser.exit(1, f'{error}\n')

if __name__ == '__main__': main()
