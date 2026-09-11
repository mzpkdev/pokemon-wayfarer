"""Compile selected NPC service bindings from effective map events."""
from pathlib import Path
import re

from .common import ContentError, PRODUCTS, assign_indexes, load_json
from .maps import resolve_object, load_maps
from .mart_stock import load_profile_ids

# Shared entry points own dispatch families; declarations own map membership.
SHARED_CLERKS = {
    'Cherrygrove_Pokemart_EventScript_Clerk': 'MART_CLERK_FAMILY_CHERRYGROVE',
    'VioletCity_Mart_EventScript_Clerk': 'MART_CLERK_FAMILY_VIOLET',
    'BattleFrontier_Mart_EventScript_Clerk': 'MART_CLERK_FAMILY_FRONTIER',
    'BattleFrontier_Mart_EventScript_Clerk_hns': 'MART_CLERK_FAMILY_FRONTIER',
}


def _keys(value, required, optional, path, key):
    if not isinstance(value, dict) or set(value) - required - optional or required - set(value):
        raise ContentError('SCHEMA', path, key, f'expected {sorted(required)}; optional {sorted(optional)}')


def _identity(value, path, key):
    if not isinstance(value, str) or not value.strip():
        raise ContentError('SCHEMA', path, key, value)


def validate_service(service, path):
    _keys(service, {'id', 'products', 'kind', 'binding'}, {'contribution', 'profile'}, path, 'service')
    key = service['id']
    _identity(key, path, 'id')
    products = service['products']
    if not isinstance(products, list) or not products or any(not isinstance(p, str) or p not in PRODUCTS for p in products) or len(set(products)) != len(products):
        raise ContentError('SCHEMA', path, key, products)
    binding = service['binding']
    _keys(binding, {'script'}, {'localId'}, path, key)
    _identity(binding['script'], path, key)
    if 'localId' in binding and (not isinstance(binding['localId'], str) or not re.fullmatch(r'[A-Za-z_]\w*', binding['localId'])):
        raise ContentError('SCHEMA', path, key, binding['localId'])
    if service['kind'] == 'rod_contribution':
        if 'profile' in service or 'contribution' not in service:
            raise ContentError('SCHEMA', path, key, 'rod requires contribution and forbids profile')
        contribution = service['contribution']
        _keys(contribution, {'namespace', 'flag'}, {'aliasOf'}, path, key)
        if 'aliasOf' in contribution and (not isinstance(contribution['aliasOf'], str) or not re.fullmatch(r'MAP_[A-Z0-9_]+/[^/\s]+', contribution['aliasOf'])):
            raise ContentError('SCHEMA', path, key, contribution['aliasOf'])
        # Existing Standard Rod flags all belong to global persistence, including
        # contributions physically located in Hoenn in a Wayfarer game.
        if contribution['namespace'] != 'global':
            raise ContentError('UNSUPPORTED_CONTEXT', path, key, contribution['namespace'])
        if not isinstance(contribution['flag'], str) or not re.fullmatch(r'FLAG_[A-Z0-9_]+', contribution['flag']):
            raise ContentError('SCHEMA', path, key, contribution['flag'])
    elif service['kind'] == 'mart':
        if 'contribution' in service or 'profile' not in service or not isinstance(service['profile'], str) or not re.fullmatch(r'MART_PROFILE_[A-Z0-9_]+', service['profile']):
            raise ContentError('SCHEMA', path, key, 'mart requires a symbolic profile and forbids contribution')
    else:
        raise ContentError('SCHEMA', path, key, service['kind'])


def load_declarations(root):
    result = []
    for path in sorted((Path(root) / 'data/maps').glob('*/gameplay.json')):
        sidecar = load_json(path)
        _keys(sidecar, {'schemaVersion', 'services', 'encounters'}, set(), path, '')
        if type(sidecar['schemaVersion']) is not int or sidecar['schemaVersion'] != 1:
            raise ContentError('SCHEMA', path, '', sidecar['schemaVersion'])
        if not isinstance(sidecar['services'], list) or not isinstance(sidecar['encounters'], list):
            raise ContentError('SCHEMA', path, '', 'services and encounters must be arrays')
        if sidecar['encounters']:
            raise ContentError('UNSUPPORTED_CONTEXT', path, '', 'encounter adapter requires integrated trainer-only baseline')
        seen = set()
        for service in sidecar['services']:
            validate_service(service, path)
            for product in service['products']:
                key = (product, service['id'])
                if key in seen:
                    raise ContentError('DUPLICATE', path, key, service['id'])
                seen.add(key)
            result.append((path, service))
    return result


def binding_symbol(record):
    return 'GAMEPLAY_MART_' + re.sub(r'[^A-Za-z0-9_]', '_', record['mapName'] + '_' + record['id']).upper()


def compile_services(root, product, defines, maps, *, cpp='cpp', cppflags=(), validate_scripts=True):
    root = Path(root)
    by_name = {m['name']: m for m in maps}
    rows, disabled = [], []
    seen_bindings = set()
    profiles = load_profile_ids(root, cpp, cppflags) if defines.get('IS_WAYFARER', 0) and defines.get('WAYFARER_TR_MARTS_ENABLED', 0) else set()
    for path, service in load_declarations(root):
        if product not in service['products']:
            continue
        key = service['id']
        if service['kind'] == 'mart' and not (defines.get('IS_WAYFARER', 0) and defines.get('WAYFARER_TR_MARTS_ENABLED', 0)):
            disabled.append({'sourcePath': str(path.relative_to(root)), 'id': key, 'reason': 'mart feature disabled'})
            continue
        map_record = by_name.get(path.parent.name)
        if map_record is None:
            raise ContentError('INACTIVE', path, key, path.parent.name)
        event = resolve_object(map_record, service['binding'], path, key)
        identity = (map_record['id'], event['runtimeId'])
        if identity in seen_bindings:
            raise ContentError('CONFLICT', path, key, identity)
        seen_bindings.add(identity)
        row = dict(service, key=[product, map_record['sourceNamespace'], 'service', map_record['id'] + '/' + key],
                   map=map_record['id'], mapName=map_record['name'], sourceNamespace=map_record['sourceNamespace'],
                   physicalRegion=map_record['physicalRegion'], sourcePath=str(path.relative_to(root)), event=event)
        if service['kind'] == 'mart' and service['profile'] not in profiles:
            raise ContentError('UNRESOLVED', path, key, service['profile'])
        rows.append(row)
    rows = assign_indexes(rows)
    if validate_scripts:
        _validate_service_scripts(root, product, rows, maps, cpp, cppflags)
    legacy_rods = _hns_legacy_contributors(root, product, rows, maps, cpp, cppflags, validate_scripts)
    rods = _canonical_contributors([row for row in rows if row['kind'] == 'rod_contribution'] + legacy_rods)
    # Keep the existing numeric flag traversal without a second authored list.
    rods.sort(key=lambda row: (row.get('contributionValue', 0), row['contribution']['flag']))
    marts = [row for row in rows if row['kind'] == 'mart']
    symbols = [binding_symbol(row) for row in marts]
    if len(symbols) != len(set(symbols)):
        raise ContentError('CONFLICT', '<services>', '', 'generated mart symbol collision')
    header = ['#ifndef GUARD_GAMEPLAY_SERVICES_H', '#define GUARD_GAMEPLAY_SERVICES_H', '']
    header += ['#define STANDARD_ROD_CONTRIBUTORS(X) \\'] if rods else ['#define STANDARD_ROD_CONTRIBUTORS(X)']
    for i, row in enumerate(rods):
        header.append('    X(' + row['contribution']['flag'] + ')' + (' \\' if i + 1 < len(rods) else ''))
    header.append('')
    for row in marts:
        header.append(f'#define {binding_symbol(row)} {row["profile"]}')
    header += ['', '#endif', '']
    shared = []
    shared_contexts = set()
    families = tuple(dict.fromkeys(SHARED_CLERKS.values()))
    shared_rows = [row for row in marts if row['binding']['script'] in SHARED_CLERKS]
    shared_rows.sort(key=lambda row: (families.index(SHARED_CLERKS[row['binding']['script']]), profiles[row['profile']], row['key']))
    for row in shared_rows:
        family = SHARED_CLERKS.get(row['binding']['script'])
        if family:
            context = (family, row['map'])
            if context in shared_contexts:
                raise ContentError('UNSUPPORTED_CONTEXT', row['sourcePath'], row['id'], 'ambiguous shared clerk map/family')
            shared_contexts.add(context)
            shared.append(f'    {{ {family}, MAP_GROUP({row["map"]}), MAP_NUM({row["map"]}), {row["profile"]} }},')
    return {'report': {'services': rows, 'disabledServices': disabled, 'legacyRodContributors': legacy_rods, 'rodCount': len(rods), 'martCount': len(marts)},
            'outputs': {'gameplay_services.h': '\n'.join(header), 'gameplay_mart_clerks.inc': '\n'.join(shared) + '\n',
                        'gameplay_mart_bindings.inc': ''.join(f'.set {binding_symbol(row)}, {row["profile"]}\n' for row in marts)}}


def _canonical_contributors(rows):
    """Resolve explicit map/service aliases; only canonical flags count."""
    by_key = {row['map'] + '/' + row['id']: row for row in rows}
    roots = {}

    def resolve(key, visiting):
        if key in roots:
            return roots[key]
        row = by_key[key]
        if key in visiting:
            raise ContentError('CONFLICT', row['sourcePath'], row['id'], 'contribution alias cycle')
        alias = row['contribution'].get('aliasOf')
        if alias is None:
            roots[key] = key
            return key
        if alias not in by_key:
            raise ContentError('UNRESOLVED', row['sourcePath'], row['id'], alias)
        target = by_key[alias]
        if any(row['contribution'][field] != target['contribution'][field] for field in ('namespace', 'flag')):
            raise ContentError('CONFLICT', row['sourcePath'], row['id'], 'contribution alias identity ' + alias)
        roots[key] = resolve(alias, visiting | {key})
        row['canonicalContribution'] = roots[key]
        return roots[key]

    for key in by_key:
        resolve(key, set())
    canonical = [by_key[key] for key in sorted(set(roots.values()))]
    identities = set()
    for row in canonical:
        identity = (row['contribution']['namespace'], row.get('contributionValue', row['contribution']['flag']))
        if identity in identities:
            raise ContentError('CONFLICT', row['sourcePath'], row['id'], 'shared contribution requires aliasOf')
        identities.add(identity)
    return sorted(canonical, key=lambda row: row['key'])


def _hns_legacy_contributors(root, product, rows, maps, cpp, cppflags, validate_scripts):
    # IS_HNS has always recognized the Wayfarer six-flag set, even when the
    # standalone map catalog cannot reach its Hoenn givers. Keep that membership
    # without inventing active HNS objects or another editable flag list.
    if product != 'hns':
        return []
    canonical_maps = {row['name']: row for row in load_maps(root, 'wayfarer')}
    live_flags = {row['contribution']['flag'] for row in rows if row['kind'] == 'rod_contribution' and 'aliasOf' not in row['contribution']}
    live_services = {(row['mapName'], row['id']) for row in rows}
    retained = []
    for path, service in load_declarations(root):
        if service['kind'] != 'rod_contribution' or 'wayfarer' not in service['products'] or service['contribution']['flag'] in live_flags or (path.parent.name, service['id']) in live_services:
            continue
        canonical = canonical_maps.get(path.parent.name)
        if canonical is None:
            raise ContentError('INACTIVE', path, service['id'], 'canonical Wayfarer giver')
        if any(row['name'] == canonical['name'] for row in maps):
            raise ContentError('UNRESOLVED', path, service['id'], 'active HNS giver requires an HNS declaration')
        event = resolve_object(canonical, service['binding'], path, service['id'])
        retained.append(dict(service, key=[product, canonical['sourceNamespace'], 'legacy_rod_membership', canonical['id'] + '/' + service['id']],
                             map=canonical['id'], mapName=canonical['name'], sourceNamespace=canonical['sourceNamespace'], physicalRegion=canonical['physicalRegion'], sourcePath=str(path.relative_to(root)), event=event,
                             reason='Existing IS_HNS contribution membership; giver only active in Wayfarer', activeBinding=False))
    if validate_scripts:
        _validate_service_scripts(root, product, retained, [], cpp, cppflags)
    return retained


def _script_sections(text):
    sections = {}
    current = None
    for line in text.splitlines():
        match = re.match(r'^([A-Za-z_]\w*)::?\s*$', line)
        if match:
            previous, current = current, match[1]
            sections.setdefault(current, [])
            if previous and sections[previous] and not re.match(r'\s*(?:end|return|goto)(?:\s|$)', sections[previous][-1]):
                sections[previous].append('call ' + current)
        elif current and line.strip() and not line.lstrip().startswith('@'):
            sections[current].append(line)
    return sections


def _reachable(sections, entry):
    pending, visited, result = [entry], set(), []
    while pending:
        label = pending.pop()
        if label in visited:
            continue
        visited.add(label)
        result.append('GAMEPLAY_SECTION')
        for line in sections.get(label, []):
            result.append(line)
            branch = re.match(r'\s*(?:goto|call|case)(?:_if_\w+)?\s+(?:[^,]+,\s*)*([A-Za-z_]\w*)\s*$', line)
            if branch:
                pending.append(branch[1])
    return result


def _validate_service_scripts(root, product, rows, maps, cpp, cppflags):
    """Resolve product aliases with CPP, then inspect exact service entry paths."""
    from .configuration import preprocess
    rods = [row for row in rows if row['kind'] == 'rod_contribution']
    header = '#define TRUE 1\n#define FALSE 0\n#include "constants/global.h"\n#include "constants/flags.h"\n#include "config/wayfarer_marts.h"\n'
    cache = {}
    rod_bindings = {(row['map'], row['binding']['script']) for row in rods}
    for row in rods:
        source = root / 'data/maps' / row['mapName'] / 'scripts.inc'
        flag = row['contribution']['flag']
        processed = preprocess(root, cpp, cppflags, header + source.read_text() + '\nGAMEPLAY_FLAG_VALUE ' + flag + '\n')
        cache[row['mapName']] = processed
        expected = re.search(r'^GAMEPLAY_FLAG_VALUE\s+(.+)$', processed, re.M)
        if not expected or not re.fullmatch(r'0x[0-9a-fA-F]+|[0-9]+', expected.group(1).strip()):
            raise ContentError('UNRESOLVED', row['sourcePath'], row['id'], flag)
        value = int(expected.group(1).strip(), 0)
        if value <= 0 or value >= 0x4000:
            raise ContentError('UNSUPPORTED_CONTEXT', row['sourcePath'], row['id'], flag)
        reachable = _reachable(_script_sections(processed), row['binding']['script'])
        operands = []
        current_operand = None
        for line in reachable:
            if line == 'GAMEPLAY_SECTION':
                current_operand = None
            assignment = re.match(r'\s*setvar\s+VAR_0x8004,\s*(.+)', line)
            if assignment:
                current_operand = assignment[1].strip()
            if re.search(r'\bspecial(?:var)?\s+(?:.*?,\s*)?(?:Script_TryAwardStandardRod|Script_CheckStandardRodAwardAvailability)\b', line):
                operands.append(current_operand)
        if not operands or any(operand != expected[1].strip() for operand in operands) or not any('TryAwardStandardRod' in line for line in reachable):
            raise ContentError('UNRESOLVED', row['sourcePath'], row['id'], f'rod script contribution {flag}')
        row['contributionValue'] = value
    for map_record in maps:
        source = root / 'data/maps' / map_record['name'] / 'scripts.inc'
        if not source.exists() or 'TryAwardStandardRod' not in source.read_text():
            continue
        processed = cache.get(map_record['name'])
        if processed is None:
            processed = preprocess(root, cpp, cppflags, header + source.read_text())
        sections = _script_sections(processed)
        for event in map_record['objects']:
            if any('TryAwardStandardRod' in line for line in _reachable(sections, event['script'])) and (map_record['id'], event['script']) not in rod_bindings:
                raise ContentError('UNRESOLVED', source, event['script'], 'live Standard Rod giver has no declaration')
    verified_shared = set()
    for row in rows:
        if row['kind'] != 'mart':
            continue
        script = row['binding']['script']
        if script in SHARED_CLERKS:
            if script in verified_shared:
                continue
            owners = []
            for candidate in maps:
                path = root / 'data/maps' / candidate['name'] / 'scripts.inc'
                if path.exists() and re.search(r'^' + re.escape(script) + r'::?\s*$', path.read_text(), re.M):
                    owners.append(path)
            if len(owners) != 1:
                raise ContentError('UNRESOLVED', row['sourcePath'], row['id'], 'unique shared clerk source ' + script)
            processed = preprocess(root, cpp, cppflags, header + owners[0].read_text())
            reachable = _reachable(_script_sections(processed), script)
            if not any(SHARED_CLERKS[script] in line for line in reachable) or not any('WayfarerLookupMartProfileForSharedClerk' in line for line in reachable):
                raise ContentError('UNRESOLVED', row['sourcePath'], row['id'], 'shared clerk dispatch ' + script)
            verified_shared.add(script)
            continue
        source = root / 'data/maps' / row['mapName'] / 'scripts.inc'
        processed = preprocess(root, cpp, cppflags, header + source.read_text())
        reachable = _reachable(_script_sections(processed), row['binding']['script'])
        operand = 'setvar VAR_0x8004, ' + binding_symbol(row)
        if not any(line.strip() == operand for line in reachable) or not any('special WayfarerOpenMartProfile' in line for line in reachable):
            raise ContentError('UNRESOLVED', row['sourcePath'], row['id'], binding_symbol(row))
