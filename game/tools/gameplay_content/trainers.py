"""Compiled roster adapter using trainerproc and its existing output parser."""
from pathlib import Path
import subprocess
import shlex
import tempfile
from tools.trainer_scaling.generate import parse_output, resolve_rosters
from .common import ContentError, PRODUCTS, load_json
from .configuration import preprocess

SOURCES = {'wayfarer': ('trainers_hns.party', 'trainers.party'),
           'hns': ('trainers_hns.party',), 'emerald': ('trainers.party',),
           'firered': ('trainers_frlg.party',), 'leafgreen': ('trainers_frlg.party',)}
SCALING_POLICIES = frozenset(('ORDINARY', 'GYM_MEMBER', 'GYM_LEADER', 'EXCLUDED'))
SOURCE_NAMESPACES = {
    'src/data/trainers_hns.party': 'hns',
    'src/data/trainers.party': 'emerald',
    'src/data/trainers_frlg.party': 'firered',
}
WAYFARER_CPPFLAGS = ('-iquote include -Wno-trigraphs -DMODERN=1 -DTESTING=0 '
                     '-DPOKEMON_WAYFARER -std=gnu17 '
                     '-DENABLE_COLOSSEUM_MULTIBOOT=0 '
                     '-DENABLE_BERRY_GLITCH_FIX_MULTIBOOT=0 '
                     '-DENABLE_EREADER_TRANSFER=0 -DPOKEMON_HNS')


def _keys(value, required, optional, path, key):
    if not isinstance(value, dict) or set(value) - required - optional or required - set(value):
        raise ContentError('SCHEMA', path, key, f'expected {sorted(required)}; optional {sorted(optional)}')


def _identity(value, path, key):
    if not isinstance(value, str) or not value.strip():
        raise ContentError('SCHEMA', path, key, value)


def _products(value, path, key):
    if (not isinstance(value, list) or not value or len(value) != len(set(value))
            or any(product not in PRODUCTS for product in value)):
        raise ContentError('SCHEMA', path, key, value)


def _roster_namespace(trainers, trainer, path, key):
    sources = {row.get('source') for row in trainers[trainer].values()}
    namespaces = {SOURCE_NAMESPACES.get(source) for source in sources}
    if len(namespaces) != 1 or None in namespaces:
        raise ContentError('UNRESOLVED', path, key, sorted(sources))
    return namespaces.pop()


def _validate_review_schema(review, path, key):
    _keys(review, {'sourceRefs'}, {'reason'}, path, key)
    references = review['sourceRefs']
    if not isinstance(references, list) or not references:
        raise ContentError('SCHEMA', path, key, 'review.sourceRefs')
    for reference in references:
        _keys(reference, {'path', 'symbol'}, {'role'}, path, key)
        _identity(reference['path'], path, key)
        _identity(reference['symbol'], path, key)
        if 'role' in reference:
            _identity(reference['role'], path, key)
    if 'reason' in review:
        _identity(review['reason'], path, key)


def _validate_review(review, path, key, trainer, variants):
    _validate_review_schema(review, path, key)
    classes = {record.get('trainerClass') for record in variants.values()}
    for reference in review['sourceRefs']:
        source = Path(path).parents[2] / reference['path']
        if not source.is_file() or reference['symbol'] not in source.read_text():
            raise ContentError('STALE_REVIEW', path, key, reference)
        if 'role' in reference and reference['role'] not in classes:
            raise ContentError('STALE_REVIEW', path, key, reference)


def load_legacy_trainers(root, product, trainers, *, partial=False):
    """Load reviewed fallback policy rows for rosters not fully caller-migrated.

    This deliberately owns only policy and provenance.  The compiler derives the
    selected roster's source, shape, and regional evidence from ``trainers``.
    """
    root = Path(root)
    path = root / 'data/gameplay/shared.json'
    shared = load_json(path)
    _keys(shared, {'schemaVersion', 'services', 'encounters', 'legacyTrainers'}, set(), path, '')
    if type(shared['schemaVersion']) is not int or shared['schemaVersion'] != 1:
        raise ContentError('SCHEMA', path, '', shared['schemaVersion'])
    if not all(isinstance(shared[name], list) for name in ('services', 'encounters', 'legacyTrainers')):
        raise ContentError('SCHEMA', path, '', 'shared arrays')
    selected, seen_ids, seen_trainers = [], set(), set()
    all_ids = set()
    for row in shared['legacyTrainers']:
        _keys(row, {'id', 'products', 'sourceNamespace', 'trainer', 'scalingPolicy', 'review'}, set(), path, 'legacyTrainers')
        _identity(row['id'], path, 'legacyTrainers')
        if row['id'] in all_ids:
            raise ContentError('DUPLICATE', path, row['id'], 'legacy trainer id')
        all_ids.add(row['id'])
        _products(row['products'], path, row['id'])
        _identity(row['sourceNamespace'], path, row['id'])
        if row['sourceNamespace'] not in set(SOURCE_NAMESPACES.values()):
            raise ContentError('SCHEMA', path, row['id'], row['sourceNamespace'])
        _identity(row['trainer'], path, row['id'])
        if row['scalingPolicy'] not in SCALING_POLICIES:
            raise ContentError('SCHEMA', path, row['id'], row['scalingPolicy'])
        if not isinstance(row['review'], dict):
            raise ContentError('SCHEMA', path, row['id'], 'review')
        _validate_review_schema(row['review'], path, row['id'])
        if row['scalingPolicy'] == 'EXCLUDED' and not row['review'].get('reason'):
            raise ContentError('SCHEMA', path, row['id'], 'excluded policy requires review.reason')
        if product not in row['products']:
            continue
        if row['id'] in seen_ids or row['trainer'] in seen_trainers:
            raise ContentError('DUPLICATE', path, row['id'], row['trainer'])
        seen_ids.add(row['id'])
        seen_trainers.add(row['trainer'])
        if row['trainer'] not in trainers:
            if partial:
                continue
            raise ContentError('UNRESOLVED', path, row['id'], row['trainer'])
        namespace = _roster_namespace(trainers, row['trainer'], path, row['id'])
        if row['sourceNamespace'] != namespace:
            raise ContentError('CONFLICT', path, row['id'], f'{row["sourceNamespace"]} != {namespace}')
        _validate_review(row['review'], path, row['id'], row['trainer'], trainers[row['trainer']])
        selected.append(row)
    return sorted(selected, key=lambda row: row['trainer'])


def compile_scaling_policies(root, product, trainers, caller_policies=(), *, discovered_callers=None):
    """Combine reviewed fallbacks with exact migrated caller policies.

    ``caller_policies`` contains migrated declarations with ``id``, ``trainer``,
    and ``scalingPolicy``.  Before removing a fallback, callers must also supply
    the complete discovered projection through ``discovered_callers`` (``id``
    and ``trainer``).  This prevents one known declaration from making an
    undiscovered caller appear fully migrated.
    """
    root = Path(root)
    fallback_rows = load_legacy_trainers(root, product, trainers)
    fallback = {row['trainer']: row['scalingPolicy'] for row in fallback_rows}
    migrated, migrated_ids = {}, {}
    for row in caller_policies:
        _keys(row, {'id', 'trainer', 'scalingPolicy'}, set(), '<encounters>', 'caller')
        _identity(row['id'], '<encounters>', 'caller')
        _identity(row['trainer'], '<encounters>', row['id'])
        if row['scalingPolicy'] not in SCALING_POLICIES:
            raise ContentError('SCHEMA', '<encounters>', row['id'], row['scalingPolicy'])
        if row['trainer'] not in trainers:
            raise ContentError('UNRESOLVED', '<encounters>', row['id'], row['trainer'])
        previous_trainer = migrated_ids.setdefault(row['id'], row['trainer'])
        if previous_trainer != row['trainer']:
            raise ContentError('CONFLICT', '<encounters>', row['id'], 'migrated caller trainer differs')
        previous = migrated.setdefault(row['trainer'], row['scalingPolicy'])
        if previous != row['scalingPolicy']:
            raise ContentError('CONFLICT', '<encounters>', row['trainer'], 'migrated caller scaling policies disagree')
        if row['trainer'] in fallback and fallback[row['trainer']] != row['scalingPolicy']:
            raise ContentError('CONFLICT', '<encounters>', row['trainer'], 'migrated caller disagrees with reviewed fallback')
    discovered = None
    if discovered_callers is not None:
        discovered = {}
        identities = {}
        for row in discovered_callers:
            _keys(row, {'id', 'trainer'}, set(), '<encounters>', 'discovered caller')
            _identity(row['id'], '<encounters>', 'discovered caller')
            _identity(row['trainer'], '<encounters>', row['id'])
            if row['trainer'] not in trainers:
                raise ContentError('UNRESOLVED', '<encounters>', row['id'], row['trainer'])
            previous_trainer = identities.setdefault(row['id'], row['trainer'])
            if previous_trainer != row['trainer']:
                raise ContentError('CONFLICT', '<encounters>', row['id'], 'discovered caller trainer differs')
            discovered.setdefault(row['trainer'], set()).add(row['id'])
        unknown_migrations = sorted(set(migrated_ids) - set(identities))
        if unknown_migrations:
            raise ContentError('CONFLICT', '<encounters>', unknown_migrations[0],
                               'migrated caller absent from discovery projection')
    populated = {trainer for trainer, variants in trainers.items()
                 if any(record.get('slots') for record in variants.values())}
    policies = {}
    for trainer in sorted(populated):
        policy = fallback.get(trainer, migrated.get(trainer))
        if policy is None:
            raise ContentError('UNRESOLVED', '<encounters>', trainer,
                               'roster has neither reviewed fallback nor migrated caller policy')
        if trainer not in fallback:
            if discovered is None:
                raise ContentError('UNRESOLVED', '<encounters>', trainer,
                                   'fallback removal requires complete discovered caller projection')
            missing = sorted(discovered.get(trainer, set()) -
                             {key for key, value in migrated_ids.items() if value == trainer})
            if not discovered.get(trainer) or missing:
                raise ContentError('UNRESOLVED', '<encounters>', trainer,
                                   'discovered caller lacks migrated declaration: ' + ', '.join(missing))
        policies[trainer] = policy
    return {'legacyTrainers': fallback_rows, 'migratedPolicies': migrated,
            'policies': policies,
            'legacyCoverage': {'fallbackRows': len(fallback_rows),
                               'migratedTrainerRows': len(migrated),
                               'discoveredCallerRows': (None if discovered is None else sum(len(rows) for rows in discovered.values())),
                               'populatedRosterRows': len(populated)}}


def compile_wayfarer_scaling_projection(root, trainers):
    """Resolve the complete Wayfarer caller inventory for scaling generation.

    The generated C policy table is a projection of fallback rows plus reviewed
    encounter declarations.  Calling the same production script adapter here
    means that pruning a fallback cannot make a local scaling run silently
    substitute a partial caller inventory.
    """
    from .configuration import numeric_defines
    from .encounters import compile_encounters
    from .maps import load_maps
    root = Path(root)
    defines = numeric_defines(root, 'arm-none-eabi-cpp', WAYFARER_CPPFLAGS)
    maps = load_maps(root, 'wayfarer')
    result = compile_encounters(root, 'wayfarer', defines, maps, trainers,
                                cpp='arm-none-eabi-cpp', cppflags=WAYFARER_CPPFLAGS,
                                assembler='arm-none-eabi-as',
                                asflags='-mcpu=arm7tdmi -march=armv4t -meabi=5 '
                                        '--defsym MODERN=1 --defsym POKEMON_WAYFARER=1 '
                                        '--defsym POKEMON_HNS=1')
    return result['report']

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
