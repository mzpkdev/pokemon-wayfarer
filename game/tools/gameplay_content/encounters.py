"""Compile reviewed trainer-battle caller contracts.

Encounter identity is a script label plus its command arguments, never a
trainer id.  The compact C registry deliberately contains only the fields the
ordinary runtime needs; the host report retains the reviewed command evidence.
"""
from __future__ import annotations

from collections import defaultdict
import hashlib
from pathlib import Path
import re

from .common import ContentError, PRODUCTS, assign_indexes, load_json
from .configuration import preprocess
from .scripts import discover_battles, load_script_blocks


MODES = {
    'trainerbattle_single': 0,
    'trainerbattle_no_intro': 3,
    'trainerbattle_double': 4,
    'trainerbattle_rematch': 5,
    'trainerbattle_rematch_double': 7,
}
COMPILED_MODES = {
    'trainerbattle_single': frozenset((0, 1, 2)),
    'trainerbattle_double': frozenset((4, 6, 8)),
    'trainerbattle_rematch': frozenset((5,)),
    'trainerbattle_rematch_double': frozenset((7,)),
    'trainerbattle_no_intro': frozenset((3,)),
}
FAMILIES = frozenset(('legacy', 'ordinary', 'objective_guard', 'deferred_rival'))
SCALING = frozenset(('ORDINARY', 'GYM_MEMBER', 'GYM_LEADER', 'EXCLUDED'))
OUTCOMES = frozenset(('LEGACY', 'LOSS_RETURN', 'DEFER'))
_LABEL = re.compile(r'^\s*([A-Za-z_]\w*)::?\s*$')


def _keys(value, required, optional, path, key):
    if not isinstance(value, dict) or set(value) - required - optional or required - set(value):
        raise ContentError('SCHEMA', path, key, f'expected {sorted(required)}; optional {sorted(optional)}')


def _identity(value, path, key):
    if not isinstance(value, str) or not re.fullmatch(r'[A-Za-z_]\w*', value):
        raise ContentError('SCHEMA', path, key, value)


def _commands(body):
    return [line.split('@', 1)[0].strip() for line in body.splitlines()
            if line.split('@', 1)[0].strip() and not line.lstrip().startswith('#')
            and not _LABEL.match(line)]


def command_fingerprint(body):
    return hashlib.sha256('\n'.join(_commands(body)).encode()).hexdigest()


def validate_encounter(encounter, path):
    _keys(encounter, {'id', 'products', 'caller', 'profile', 'review'}, {'baseEncounter'}, path, 'encounter')
    key = encounter['id']
    _identity(key, path, 'id')
    products = encounter['products']
    if not isinstance(products, list) or not products or len(products) != len(set(products)) or any(product not in PRODUCTS for product in products):
        raise ContentError('SCHEMA', path, key, products)
    _keys(encounter['caller'], {'label'}, {'map', 'contextIndependent'}, path, key)
    _identity(encounter['caller']['label'], path, key)
    has_map = 'map' in encounter['caller']
    independent = encounter['caller'].get('contextIndependent') is True
    if has_map == independent:
        raise ContentError('SCHEMA', path, key, 'caller needs exactly one of map/contextIndependent')
    if has_map:
        _identity(encounter['caller']['map'], path, key)
    _keys(encounter['review'], {'commandFingerprint', 'source'}, set(), path, key)
    if not isinstance(encounter['review']['commandFingerprint'], str) or not re.fullmatch(r'[0-9a-f]{64}', encounter['review']['commandFingerprint']):
        raise ContentError('SCHEMA', path, key, 'commandFingerprint')
    if not isinstance(encounter['review']['source'], str) or not encounter['review']['source'].startswith('data/'):
        raise ContentError('SCHEMA', path, key, 'source')
    profile = encounter['profile']
    _keys(profile, {'family', 'scalingPolicy', 'outcomePolicy', 'entry', 'refusal', 'success', 'nonVictory', 'dialogue', 'stableKey'}, {'scene'}, path, key)
    if profile['family'] not in FAMILIES or profile['scalingPolicy'] not in SCALING or profile['outcomePolicy'] not in OUTCOMES:
        raise ContentError('SCHEMA', path, key, profile)
    if type(profile['stableKey']) is not int or not 0 <= profile['stableKey'] <= 0xffff:
        raise ContentError('SCHEMA', path, key, 'stableKey')
    if not isinstance(profile['dialogue'], str) or not re.fullmatch(r'[A-Z_]+', profile['dialogue']):
        raise ContentError('SCHEMA', path, key, 'dialogue')
    if any(not isinstance(profile[field], str) or not profile[field] for field in ('entry', 'refusal', 'success', 'nonVictory')):
        raise ContentError('SCHEMA', path, key, 'contract')
    if profile['family'] == 'objective_guard':
        _keys(profile.get('scene'), {'pendingPredicate', 'retreatScript'}, set(), path, key)
    if profile['family'] == 'deferred_rival':
        _keys(profile.get('scene'), {'sceneId', 'trigger', 'objectLifecycle', 'rearmRegion', 'eligibilityPredicate', 'object', 'activationRect'}, set(), path, key)
        _keys(profile['scene']['object'], {'localId', 'elevation'}, set(), path, key)
        _identity(profile['scene']['object']['localId'], path, key)
        if type(profile['scene']['object']['elevation']) is not int:
            raise ContentError('SCHEMA', path, key, 'scene.object.elevation')
        _keys(profile['scene']['activationRect'], {'x', 'y', 'width', 'height'}, set(), path, key)
        if any(type(profile['scene']['activationRect'][field]) is not int
               for field in ('x', 'y', 'width', 'height')):
            raise ContentError('SCHEMA', path, key, 'scene.activationRect')
    runtime_contract = (profile['entry'], profile['refusal'], profile['success'])
    if profile['family'] == 'legacy':
        if runtime_contract != ('legacy_runtime',) * 3 or profile['nonVictory'] != 'legacy_runtime' or profile['outcomePolicy'] != 'LEGACY':
            raise ContentError('SCHEMA', path, key, 'legacy runtime contract')
    elif runtime_contract != ('trainerbattle', 'WayfarerStoryTryStartTrainerBattle', 'authored'):
        raise ContentError('SCHEMA', path, key, 'unsupported trainerbattle runtime contract')
    elif profile['family'] == 'ordinary':
        expected = ('EventScript_WayfarerStoryLossRetreat'
                    if profile['outcomePolicy'] == 'LOSS_RETURN' else 'authored')
        if profile['outcomePolicy'] not in {'LOSS_RETURN', 'LEGACY'} or profile['nonVictory'] != expected:
            raise ContentError('SCHEMA', path, key, 'ordinary outcome continuation')
    elif profile['family'] == 'objective_guard':
        if profile['outcomePolicy'] != 'LOSS_RETURN' or profile['nonVictory'] != profile['scene']['retreatScript']:
            raise ContentError('SCHEMA', path, key, 'objective outcome continuation')
    elif profile['family'] == 'deferred_rival' and profile['outcomePolicy'] != 'LOSS_RETURN':
        raise ContentError('SCHEMA', path, key, 'deferred rival outcome continuation')
    if 'baseEncounter' in encounter and (not isinstance(encounter['baseEncounter'], str) or not re.fullmatch(r'[A-Za-z_]\w*', encounter['baseEncounter'])):
        raise ContentError('SCHEMA', path, key, 'baseEncounter')


def load_declarations(root):
    root = Path(root)
    result = []
    paths = sorted((root / 'data/maps').glob('*/gameplay.json'))
    shared = root / 'data/gameplay/shared.json'
    if shared.is_file():
        paths.append(shared)
    for path in paths:
        document = load_json(path)
        required = {'schemaVersion', 'services', 'encounters'}
        allowed = required | ({'legacyTrainers'} if path == shared else set())
        _keys(document, required, allowed - required, path, '')
        if document['schemaVersion'] != 1 or not isinstance(document['encounters'], list):
            raise ContentError('SCHEMA', path, '', 'schemaVersion/encounters')
        for encounter in document['encounters']:
            validate_encounter(encounter, path)
            result.append((path, encounter))
    return result


def _resolved_command(path, body, encounter):
    commands = _commands(body)
    if not commands:
        raise ContentError('UNRESOLVED', path, encounter['id'], encounter['caller']['label'])
    first = commands[0]
    fields = first.split(None, 1)
    mode = MODES.get(fields[0])
    if mode is None:
        raise ContentError('UNSUPPORTED_CONTEXT', path, encounter['id'], first)
    arguments = [value.strip() for value in fields[1].split(',')] if len(fields) == 2 else []
    trainer_match = re.match(r'(TRAINER_[A-Z0-9_]+)\b', fields[1]) if len(fields) == 2 else None
    if trainer_match is None:
        raise ContentError('UNRESOLVED', path, encounter['id'], first)
    return first, mode, trainer_match.group(1)


_TRAINER_ARGUMENTS = {
    'trainerbattle': (2, 7),
    'trainerbattle_single': (0,), 'trainerbattle_double': (0,),
    'trainerbattle_rematch': (0,), 'trainerbattle_rematch_double': (0,),
    'trainerbattle_no_intro': (0,), 'trainerbattle_two_trainers': (0, 2),
    'trainerbattle_earlyrival': (0,),
}


def _battle_trainers(macro, arguments):
    """Extract only documented trainer operands from an emitted macro call."""
    operands = [operand.strip() for operand in arguments.split(',')]
    indexes = _TRAINER_ARGUMENTS.get(macro)
    if indexes is None:
        return [], list(operands)
    trainers, unresolved = [], []
    for index in indexes:
        if index >= len(operands):
            unresolved.append(f'<missing trainer argument {index}>')
            continue
        operand = operands[index]
        if operand == 'TRAINER_NONE':
            continue
        match = re.fullmatch(r'(TRAINER_[A-Z0-9_]+)', operand)
        # GAS accepts whitespace in place of the first comma. Keep that
        # production spelling signature-aware instead of treating its intro
        # text as part of a dynamic trainer expression.
        if match is None and index == 0:
            match = re.match(r'(TRAINER_[A-Z0-9_]+)\b', operand)
        if match and not match.group(1).startswith('TRAINER_BATTLE_'):
            trainers.append(match.group(1))
        else:
            unresolved.append(operand)
    return trainers, unresolved


def _discover_battle_callers(blocks):
    """Inventory every selected battle; only reviewed direct callers authorize."""
    result = []
    for battle in discover_battles(blocks):
        mode = MODES.get(battle['macro'])
        trainers, unresolved = _battle_trainers(battle['macro'], battle['arguments'])
        compiled = blocks[battle['label']][battle['occurrence']].get('compiledBattle')
        result.append({'label': battle['label'], 'source': battle['sourcePath'],
                       'sourceLine': battle['sourceLine'], 'commandIndex': battle['commandIndex'],
                       'occurrence': battle['occurrence'],
                       'directCaller': battle['directCaller'],
                       'assemblerConditional': battle['assemblerConditional'],
                       'macro': battle['macro'], 'arguments': battle['arguments'],
                       'resolvedArguments': battle['resolvedArguments'],
                       'trainerIds': trainers, 'trainerId': trainers[0] if len(trainers) == 1 else None,
                       'unresolvedTrainerOperands': unresolved,
                       'mode': compiled['mode'] if battle['commandIndex'] == 0 and isinstance(compiled, dict) else mode,
                       'compiledBattle': compiled if battle['commandIndex'] == 0 else None,
                       'commandFingerprint': command_fingerprint(blocks[battle['label']][battle['occurrence']]['body'])})
    return result


def _ordinary_header(rows):
    lines = ['// Generated by tools/gameplay_content/encounters.py.',
             '// Only reviewed declarations authorize field-loss return.',
             '#if IS_WAYFARER',
             'extern const u8 EventScript_WayfarerStoryLossRetreat[];']
    lines += [f'extern const u8 {row["caller"]["label"]}[];' for row in rows]
    lines += ['#define GAMEPLAY_ENCOUNTER_ARGUMENT_OFFSET 1',
              '#define ORDINARY_ENTRY(_caller, _key, _dialogue, _flags) \\',
              '    { .caller = (_caller) + GAMEPLAY_ENCOUNTER_ARGUMENT_OFFSET, .stableKey = (_key), \\',
              '      .dialogue = WAYFARER_STORY_DIALOGUE_ ## _dialogue, .flags = (_flags) },',
              'const struct WayfarerOrdinaryEncounter gWayfarerStoryOrdinaryEncounters[] =', '{']
    for row in rows:
        profile = row['profile']
        flags = []
        if row['caller']['mode'] not in (5, 7):
            flags.append('WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT')
        if profile['outcomePolicy'] == 'LOSS_RETURN':
            flags.append('WAYFARER_STORY_FLAG_LOSS_RETURN')
        lines.append(f'    ORDINARY_ENTRY({row["caller"]["label"]}, {profile["stableKey"]}, {profile["dialogue"]}, {" | ".join(flags) or "0"})')
    lines += ['};', 'const u32 gWayfarerStoryOrdinaryEncounterCount = ARRAY_COUNT(gWayfarerStoryOrdinaryEncounters);',
              '#undef ORDINARY_ENTRY', '#undef GAMEPLAY_ENCOUNTER_ARGUMENT_OFFSET', '#else',
              'const struct WayfarerOrdinaryEncounter gWayfarerStoryOrdinaryEncounters[] = {{0}};',
              'const u32 gWayfarerStoryOrdinaryEncounterCount = 0;', '#endif', '']
    return '\n'.join(lines)


# Scene records have deliberately closed adapters.  They name the existing C
# lifecycle predicates and map-object layout rather than inventing a generic
# host-to-runtime predicate language.  The two records below are the required
# Phase-D expanded migrations; every other regional row remains a reviewed
# legacy adapter in the pre-existing table.
_SCENE_RUNTIME = {
    ('objective_guard', 'RusturfTunnel_EventScript_GruntTrainerBattle'): {
        'scene': {'pendingPredicate': 'WayfarerHoennRusturfRescuePending',
                  'retreatScript': 'RusturfTunnel_EventScript_WayfarerGruntLossRetreat'},
        'trigger': 'NULL', 'sceneId': 'WAYFARER_STORY_SCENE_RUSTURF_AQUA',
        'policy': 'WAYFARER_STORY_POLICY_OBJECTIVE_GUARD', 'map': 'MAP_RUSTURF_TUNNEL',
        'localId': '0', 'elevation': 'WAYFARER_STORY_ANY_ELEVATION',
        'width': '0', 'height': '0', 'x': 'WAYFARER_STORY_NO_COORD',
        'y': 'WAYFARER_STORY_NO_COORD',
    },
    ('deferred_rival', 'Route110_EventScript_MayBattleTreecko'): {
        'scene': {'sceneId': 'WAYFARER_STORY_SCENE_ROUTE110_RIVAL',
                  'trigger': 'Route110_EventScript_RivalScene',
                  'objectLifecycle': 'TRANSIENT_OBJECT', 'rearmRegion': 'MAP_ROUTE110',
                  'eligibilityPredicate': 'WayfarerHoennRoute110RivalPending',
                  'object': {'localId': 'LOCALID_ROUTE110_RIVAL', 'elevation': 3},
                  'activationRect': {'x': 33, 'y': 56, 'width': 3, 'height': 1}},
        'sceneId': 'WAYFARER_STORY_SCENE_ROUTE110_RIVAL',
        'policy': 'WAYFARER_STORY_POLICY_DEFERRED_RIVAL', 'map': 'MAP_ROUTE110',
        'localId': 'WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL', 'elevation': '3',
        'runtimeId': 28,
        'width': '3', 'height': '1', 'x': '33', 'y': '56',
    },
}


def _scene_header(rows):
    expanded = {(row['profile']['family'], row['caller']['label']): row
                for row in rows if row['profile']['family'] in {'objective_guard', 'deferred_rival'}}
    if expanded and set(expanded) != set(_SCENE_RUNTIME):
        raise ContentError('UNSUPPORTED_CONTEXT', '<encounters>', '',
                           sorted(set(expanded) ^ set(_SCENE_RUNTIME)))
    lines = ['// Generated by tools/gameplay_content/encounters.py.',
             '// Closed adapters preserve existing Hoenn scene lifecycle payloads.']
    if not expanded:
        return '\n'.join(lines) + '\n'
    names = {
        'RusturfTunnel_EventScript_GruntTrainerBattle': 'GAMEPLAY_ENCOUNTER_RUSTURF_AQUA_ROW',
        'Route110_EventScript_MayBattleTreecko': 'GAMEPLAY_ENCOUNTER_ROUTE110_RIVAL_ROW',
    }
    for key, adapter in _SCENE_RUNTIME.items():
        row = expanded[key]
        profile, scene = row['profile'], row['profile']['scene']
        if scene != adapter['scene']:
            raise ContentError('CONFLICT', row['sourcePath'], row['id'], 'closed scene lifecycle adapter')
        trigger = adapter.get('trigger', scene.get('trigger'))
        flags = ['WAYFARER_STORY_FLAG_LOSS_RETURN'] if profile['outcomePolicy'] == 'LOSS_RETURN' else []
        if scene.get('objectLifecycle') == 'TRANSIENT_OBJECT':
            flags.append('WAYFARER_STORY_FLAG_TRANSIENT_OBJECT')
        if scene.get('rearmRegion'):
            flags.append('WAYFARER_STORY_FLAG_REARM_ON_LEAVE')
        predicate = scene.get('pendingPredicate', scene.get('eligibilityPredicate'))
        lines += [f'#define {names[row["caller"]["label"]]} \\',
                  f'    HOENN_ENTRY({row["caller"]["label"]} + {row["caller"]["argumentOffset"]}, '
                  f'{profile["nonVictory"]}, {trigger}, {profile["stableKey"]}, '
                  f'{adapter["sceneId"]}, {adapter["policy"]}, '
                  f'WAYFARER_STORY_DIALOGUE_{profile["dialogue"]}, '
                  f'{" | ".join(flags) or "0"}, {scene.get("rearmRegion", adapter["map"])}, {adapter["localId"]}, '
                  f'{scene.get("object", {}).get("elevation", adapter["elevation"])}, '
                  f'{scene.get("activationRect", {}).get("width", adapter["width"])}, '
                  f'{scene.get("activationRect", {}).get("height", adapter["height"])}, '
                  f'{scene.get("activationRect", {}).get("x", adapter["x"])}, '
                  f'{scene.get("activationRect", {}).get("y", adapter["y"])}, {predicate}),']
    return '\n'.join(lines) + '\n'


def _validate_scene_binding(authored, maps, path):
    """Bind deferred object lifecycle fields to the selected map inventory."""
    if authored['profile']['family'] != 'deferred_rival':
        return
    scene = authored['profile']['scene']
    record = next((record for record in maps if record['name'] == authored['caller']['map']), None)
    if record is None or record['id'] != scene['rearmRegion']:
        raise ContentError('CONFLICT', path, authored['id'], 'deferred rearm region')
    matching = [object_ for object_ in record['objects']
                if object_.get('localId') == scene['object']['localId']]
    if len(matching) != 1 or matching[0]['elevation'] != scene['object']['elevation']:
        raise ContentError('CONFLICT', path, authored['id'], 'deferred map object binding')
    adapter = _SCENE_RUNTIME.get((authored['profile']['family'], authored['caller']['label']))
    if adapter and matching[0]['runtimeId'] != adapter['runtimeId']:
        raise ContentError('CONFLICT', path, authored['id'], 'deferred object runtime id')


def _trainer_values(root, cpp, cppflags, trainer_names):
    """Evaluate selected C trainer constants through the build preprocessor."""
    if not trainer_names:
        return {}
    source = '#include "constants/global.h"\n#include "constants/opponents.h"\n'
    source += ''.join(f'GAMEPLAY_TRAINER_{index} = {name};\n'
                      for index, name in enumerate(sorted(trainer_names)))
    output = preprocess(root, cpp, cppflags, source)
    values = {}
    names = sorted(trainer_names)
    for line in output.splitlines():
        match = re.fullmatch(r'GAMEPLAY_TRAINER_(\d+)\s*=\s*([^;]+);', line.strip())
        if not match:
            continue
        expression = match.group(2)
        if not re.fullmatch(r'[0-9\s()+*/%<>&|~-]+', expression):
            raise ContentError('UNRESOLVED', '<trainer constants>', names[int(match.group(1))], expression)
        try:
            values[names[int(match.group(1))]] = int(eval(expression, {'__builtins__': {}}, {}))
        except (ArithmeticError, SyntaxError, ValueError) as exc:
            raise ContentError('UNRESOLVED', '<trainer constants>', names[int(match.group(1))], expression) from exc
    if set(values) != set(trainer_names) or any(not 0 <= value <= 0xffff for value in values.values()):
        raise ContentError('UNRESOLVED', '<trainer constants>', '', sorted(set(trainer_names) - set(values)))
    return values


def compile_encounters(root, product, defines, maps, trainers, *, cpp='cpp', cppflags=(), service_bindings=None,
                       assembler='arm-none-eabi-as', asflags=()):
    """Compile selected caller contracts and a compact ordinary registry."""
    root = Path(root)
    blocks = load_script_blocks(root, product, maps, cpp, cppflags,
                                service_bindings=service_bindings,
                                assembler=assembler, asflags=asflags)
    map_names = {row['name'] for row in maps}
    rows, disabled, seen = [], [], set()
    for path, authored in load_declarations(root):
        for selected in authored['products']:
            # Local ids are scoped by their map; shared-script ids are global.
            scope = path.parent.name if path.parent.parent.name == 'maps' else 'shared'
            identity = (selected, scope, authored['id'])
            if identity in seen:
                raise ContentError('DUPLICATE', path, authored['id'], identity)
            seen.add(identity)
        if product not in authored['products']:
            disabled.append({'sourcePath': str(path.relative_to(root)), 'id': authored['id'], 'reason': 'product inactive'})
            continue
        label = authored['caller']['label']
        if label not in blocks:
            raise ContentError('UNRESOLVED', path, authored['id'], label)
        candidates = [candidate for candidate in blocks[label]
                      if candidate['path'] == authored['review']['source']]
        if len(candidates) != 1:
            raise ContentError('DUPLICATE', path, authored['id'], 'active script label ' + label)
        source, body = candidates[0]['path'], candidates[0]['body']
        review = authored['review']
        if command_fingerprint(body) != review['commandFingerprint']:
            raise ContentError('STALE_REVIEW', path, authored['id'], review['commandFingerprint'])
        command, basic_mode, trainer = _resolved_command(source, body, authored)
        compiled = candidates[0].get('compiledBattle')
        if not isinstance(compiled, dict) or type(compiled.get('mode')) is not int or type(compiled.get('trainer')) is not int:
            raise ContentError('UNRESOLVED', path, authored['id'], 'missing compiled trainerbattle payload')
        macro = command.split(None, 1)[0]
        if compiled['mode'] not in COMPILED_MODES[macro]:
            raise ContentError('UNSUPPORTED_CONTEXT', path, authored['id'], 'compiled trainerbattle mode ' + str(compiled['mode']))
        map_name = authored['caller'].get('map')
        if map_name is not None and map_name not in map_names:
            raise ContentError('INACTIVE', path, authored['id'], map_name)
        namespace = next((record['sourceNamespace'] for record in maps if record['name'] == map_name), 'shared')
        row = dict(authored, key=[product, namespace, 'encounter', authored['id']],
                   sourcePath=str(path.relative_to(root)), caller=dict(authored['caller'], source=source, trainer=trainer,
                                                                         mode=compiled['mode'], basicMode=basic_mode,
                                                                         compiledTrainer=compiled['trainer'], argumentOffset=1),
                   command=command)
        # Map-local declarations must remain selected by the actual map inventory.
        if path.parent.parent.name == 'maps' and path.parent.name not in map_names:
            raise ContentError('INACTIVE', path, authored['id'], path.parent.name)
        if path.parent.parent.name == 'maps' and map_name != path.parent.name:
            raise ContentError('CONFLICT', path, authored['id'], 'map-local caller binding')
        _validate_scene_binding(authored, maps, path)
        if trainer not in trainers:
            raise ContentError('UNRESOLVED', path, authored['id'], trainer)
        rows.append(row)
    bound = {}
    for row in rows:
        identity = (row['caller']['label'], row['caller']['source'])
        if identity in bound:
            raise ContentError('DUPLICATE', row['sourcePath'], row['id'],
                               'active caller already bound by ' + bound[identity]['id'])
        bound[identity] = row
    discovered = _discover_battle_callers(blocks)
    unresolved = [row for row in discovered if row['unresolvedTrainerOperands']]
    if unresolved:
        first = unresolved[0]
        raise ContentError('UNRESOLVED', first['source'], first['label'],
                           ', '.join(first['unresolvedTrainerOperands']))
    missing = sorted({trainer for row in discovered for trainer in row['trainerIds'] if trainer not in trainers})
    if product == 'wayfarer' and missing:
        raise ContentError('UNRESOLVED', '<discovered callers>', '', ', '.join(missing))
    names = {row['caller']['trainer'] for row in rows}
    # Standalone products can assemble dormant imported scripts whose actors
    # are intentionally absent from that product's roster. Their packed bytes
    # remain discovery evidence, but never become a selected numeric projection.
    names.update(trainer for row in discovered for trainer in row['trainerIds'] if trainer in trainers)
    values = _trainer_values(root, cpp, cppflags, names) if (root / 'include/constants/opponents.h').is_file() else {}
    for row in rows:
        symbolic = row['caller']['trainer']
        row['caller']['trainerId'] = symbolic
        if symbolic in values:
            row['caller']['trainer'] = values[symbolic]
            if row['caller']['trainer'] != row['caller']['compiledTrainer']:
                raise ContentError('CONFLICT', row['sourcePath'], row['id'], 'compiled trainer differs from symbolic trainer')
    for row in discovered:
        row['trainerValues'] = [values[trainer] for trainer in row['trainerIds'] if trainer in values]
        compiled = row['compiledBattle']
        if compiled and len(row['trainerValues']) == 1 and row['trainerValues'][0] != compiled['trainer']:
            raise ContentError('CONFLICT', row['source'], row['label'], 'compiled trainer differs from symbolic trainer')
    rows = assign_indexes(rows)
    by_id = {row['id']: row for row in rows}
    for row in rows:
        base = row.get('baseEncounter')
        if base and (base not in by_id
                     or by_id[base]['profile']['stableKey'] != row['profile']['stableKey']
                     or by_id[base]['profile']['dialogue'] != row['profile']['dialogue']):
            raise ContentError('UNRESOLVED', row['sourcePath'], row['id'], base)
    ordinary = [row for row in rows if row['profile']['family'] == 'ordinary']
    # The legacy compact table is a frozen runtime payload; its historical
    # caller-label ordering is independent from host qualified-key indexing.
    ordinary.sort(key=lambda row: row['caller']['label'])
    policies = defaultdict(set)
    for row in rows:
        policies[row['caller']['trainerId']].add(row['profile']['scalingPolicy'])
    conflicts = sorted(trainer for trainer, values in policies.items() if len(values) != 1)
    if conflicts:
        raise ContentError('CONFLICT', '<encounters>', '', ', '.join(conflicts))
    declared = {(row['caller']['label'], row['caller']['source']) for row in rows}
    # A declaration describes exactly its label's opening command. Later calls
    # stay report-only and cannot inherit authorization; unresolved operands
    # have already failed closed above.
    unreviewed = [row for row in discovered
                  if not row['directCaller'] or (row['label'], row['source']) not in declared]
    report = {'encounters': rows, 'ordinaryEncounterCount': len(ordinary),
              'legacyEncounterAdapters': [row for row in rows if row['profile']['family'] == 'legacy'],
              'disabledEncounters': disabled,
              'unselectedTrainerIds': missing,
              'scriptInputHashes': getattr(blocks, 'input_hashes', {}),
              'discoveredBattleCallers': discovered,
              'unreviewedBattleCallers': unreviewed,
              'trainerPolicies': {trainer: next(iter(values)) for trainer, values in sorted(policies.items())}}
    # The imported scaling inventory is Wayfarer-owned. Standalone products
    # retain their selected encounter discovery above but must not consume its
    # fallback rows or require Wayfarer's policy coverage.
    if product == 'wayfarer':
        from .trainers import compile_scaling_policies
        report['scaling'] = compile_scaling_policies(
            root, product, trainers,
            [{'id': row['caller']['source'] + '/' + row['caller']['label'], 'trainer': row['caller']['trainerId'],
              'scalingPolicy': row['profile']['scalingPolicy']} for row in rows],
            discovered_callers=[{'id': (row['source'] + '/' + row['label'] if row['directCaller']
                                        else row['source'] + '/' + row['label'] +
                                        '#' + str(row['occurrence']) + ':' + str(row['commandIndex']))
                                     + ('' if len(row['trainerIds']) == 1 else '@' + str(index)),
                                 'trainer': trainer}
                                for row in discovered
                                for index, trainer in enumerate(row['trainerIds'])
                                if trainer in trainers])
    return {'report': report, 'outputs': {'gameplay_encounters.inc': _ordinary_header(ordinary),
                                          'gameplay_encounter_scenes.inc': _scene_header(rows)}}
