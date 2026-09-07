#!/usr/bin/env python3
"""Report Wayfarer known-move coverage separately from standalone certificates."""

import argparse
from collections import Counter
from fractions import Fraction
from functools import lru_cache
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path

import wild_encounters_to_header as g

REVISION = g.ROOT.parent / '.product/research/native-hm-windows/revisions/nearby-access'
LEARNSET_HELPER = g.ROOT / 'tools/learnset_helpers/test_native_hm_windows.py'


def fraction(value):
    return {'numerator': value.numerator, 'denominator': value.denominator}


def source_context(map_name):
    return {
        'reachability': 'NOT_VERIFIED',
        'optionalArea': any(part in map_name for part in ('WHIRL_ISLANDS', 'SAFARI', 'ABANDONED_SHIP', 'CERULEAN_CAVE', 'NEW_MAUVILLE', 'MIRAGE_TOWER')),
        'lateArea': any(part in map_name for part in ('CERULEAN_CAVE', 'MT_SILVER', 'VICTORY_ROAD', 'TOHJO_FALLS')),
        'underwaterArea': 'UNDERWATER' in map_name,
        'outOfScopeArea': 'WHIRL_ISLANDS' in map_name,
        'regionClassification': 'HNS_UNCLASSIFIED' if map_name.endswith('_HNS') and map_name not in g.JOHTO_MAPS | g.KANTO_MAPS else 'CLASSIFIED',
        'classificationMethod': 'Conservative named-area flags; absence of a flag is not proof of accessibility.',
    }


class Model:
    def __init__(self):
        original = g.load_json(g.DEFAULT_ENCOUNTERS)
        encounters, self.replacements = g.apply_wayfarer_encounter_replacements(original)
        config = g.Config(g.DEFAULT_CONFIG, g.DEFAULT_RTC, encounters)
        known = g.species_ids(g.DEFAULT_SPECIES)
        profiles, _ = g.validate_encounters(encounters, known, config)
        self.scaling = g.load_scaling(g.DEFAULT_SCALING)
        self.rods = g.load_standard_rod_fishing(g.DEFAULT_STANDARD_ROD_FISHING)
        ordinary = {mon['species'] for p in profiles for method in config.mon_types
                    for mon in p['encounter'].get(method, {}).get('mons', [])}
        self.metadata = {r['species']: r for r in g.load_species_metadata(
            g.DEFAULT_SPECIES_METADATA, g.DEFAULT_SPECIES_INFO, known, ordinary)}
        offsets = g.load_offsets(self.scaling['profile_offsets'], profiles, g.DEFAULT_SCALING)
        offset_map = {(r['product'], r['header_id'], r['area'], r['time'], r['rod']): r['level_offset'] for r in offsets}
        regions = g.load_json(g.DEFAULT_REGIONS)['regions']
        aliases = {'profiles': [p for data in regions.values() for p in data.get('profiles', []) if 'nightMode' in p]}
        profiles = g.profiles_with_day_aliases(profiles, aliases, config)
        self.profiles = {}
        for p in profiles:
            if p['product'] not in {'EMERALD', 'POKEMON_HNS'}:
                continue
            for method in config.mon_types:
                if method not in p['encounter']:
                    continue
                for rod in g.FISHING_QUALITIES if method == 'fishing_mons' else ('NONE',):
                    identity = (p['map'], method, p['time'], rod)
                    offset = offset_map.get((p['product'], p['header_id'], g.METHOD_AREAS[method], p['time'], g.RODS[rod]), 0)
                    self.profiles[identity] = {
                        'label': p['label'], 'offset': offset,
                        'slots': tuple((m['species'], m.get('min_level', 2), m.get('max_level', 100), w)
                                       for _, m, w in g.method_slots(p, method, rod, self.rods))}
        self.roster = {r['species']: r for r in g.load_json(REVISION / 'roster.json')}
        spec = importlib.util.spec_from_file_location('native_hm_learnset_validation', LEARNSET_HELPER)
        helper = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(helper)
        baseline = g.load_json(REVISION.parents[1] / 'baseline.json')['species']
        self.windows = {}
        for mode, path in helper.SOURCES.items():
            tables = helper.preprocess((g.ROOT.parent / path).read_text(), wayfarer=True)
            for species, source in baseline.items():
                entries = tables[source[mode + '_symbol']]
                row = self.roster.get(species)
                if row and entries != row['modes'][mode]['entries']:
                    raise g.ValidationError(f'{species}/{mode}: production learnset differs from approved roster')
                for level in range(1, 101):
                    moves = []
                    for learned, move in entries:
                        if learned > level:
                            break
                        if learned != 0 and move not in moves:
                            moves = (moves + [move])[-4:]
                    self.windows[species, mode, level] = frozenset(moves)

    @lru_cache(maxsize=None)
    def level(self, authored, rating, offset):
        return g.project_level(self.scaling, authored, rating, offset)

    @lru_cache(maxsize=None)
    def slot(self, species, low, high, rating, offset):
        if species == 'SPECIES_NONE':
            return ()
        counts = Counter()
        for authored in range(low, high + 1):
            level = self.level(authored, rating, offset)
            effective, _ = g.effective_species(species, level, self.metadata)
            if level < self.metadata[effective]['minimum_level']:
                return ()
            counts[effective, level] += 1
        return tuple((species, level, count) for (species, level), count in counts.items())

    @lru_cache(maxsize=None)
    def measure(self, identity, mode, move, rating):
        profile = self.profiles[identity]
        eligible = []
        for species, low, high, weight in profile['slots']:
            outcomes = self.slot(species, low, high, rating, profile['offset'])
            if outcomes and weight:
                eligible.append((weight, high - low + 1, outcomes))
        total = sum(weight for weight, _, _ in eligible)
        carriers = Counter()
        for weight, count, outcomes in eligible:
            for species, level, occurrences in outcomes:
                if move in self.windows.get((species, mode, level), ()):
                    carriers[species, level] += Fraction(weight * occurrences, total * count)
        probability = sum(carriers.values(), Fraction(0))
        map_name, method, time, rod = identity
        return probability, {
            'map': map_name, 'method': method, 'timeOfDay': time, 'rod': rod,
            'baseLabel': profile['label'], 'successfulEncounterProbability': fraction(probability),
            'unmodifiedCastProbability': fraction(probability * Fraction(g.FISHING_BASE_BITE_PERCENT[rod], 100)) if rod != 'NONE' else None,
            'carriers': [{'species': species, 'level': level, 'probability': fraction(chance)}
                         for (species, level), chance in sorted(carriers.items())]}


def build_report():
    model = Model()
    failures, regional, directional = [], [], []
    coverage = g.load_json(REVISION / 'coverage.json')['results']
    if len(coverage) != 48:
        raise g.ValidationError('Expected 48 region/mode/utility rows')
    for row in coverage:
        observations = []
        for rating in range(81):
            witness = row['witnesses'][str(rating)]
            identity = tuple(witness[key] for key in ('map', 'method', 'time', 'rod'))
            chance, result = model.measure(identity, row['mode'], row['move'], rating)
            if not chance:
                failures.append(f"{row['region']}/{row['mode']}/{row['move']}/TR{rating}: no known-move witness")
            observations.append({'rating': rating, **result, 'sourceContext': source_context(identity[0])})
        regional.append({'region': row['region'], 'mode': row['mode'], 'move': row['move'], 'ratings': observations})
    scenarios = g.load_json(REVISION / 'scenarios.json')['scenarios']
    if len(scenarios) != 11:
        raise g.ValidationError('Expected 11 directional scenarios')
    for scenario in scenarios:
        for mode in ('modern', 'legacy'):
            for clock in ('DAY', 'NIGHT'):
                for rod in g.FISHING_QUALITIES:
                    candidates = []
                    for source in scenario['maps']:
                        if scenario['id'] in ('blackthorn_surf', 'den_whirlpool') and source['rank'] != 0:
                            continue
                        time = 'TIME_' + clock if source['map'].endswith('_HNS') else 'TIME_DAY'
                        for method in source['methods']:
                            identity = (source['map'], method, time, rod if method == 'fishing_mons' else 'NONE')
                            if identity in model.profiles:
                                candidates.append((identity, source['rank']))
                    observations = []
                    for rating in range(81):
                        choices = [(model.measure(identity, mode, 'MOVE_' + scenario['move'], rating), rank)
                                   for identity, rank in candidates]
                        if not choices:
                            raise g.ValidationError(f"{scenario['id']}: no candidate profiles")
                        (chance, best), rank = max(choices, key=lambda c: (c[0][0], -c[1]))
                        adequate = [((p, w), r) for (p, w), r in choices if p >= Fraction(2, 25)]
                        nearest = min(adequate, key=lambda c: (c[1], -c[0][0]), default=None)
                        qualifying = [(p, w) for (p, w), _ in choices
                                      if p >= Fraction(2, 25) and (w['rod'] == 'NONE' or rod == 'OLD_ROD')]
                        if rod != 'OLD_ROD':
                            qualifying += [(p, w) for (p, w), _ in choices if w['rod'] != 'NONE' and p > 0]
                        if not qualifying:
                            failures.append(f"{scenario['id']}/{mode}/{clock}/{rod}/TR{rating}: acquisition floor failed")
                        observations.append({'rating': rating, 'best': dict(best, detourRank=rank),
                                             'nearestQualifyingSource': dict(nearest[0][1], detourRank=nearest[1]) if nearest else None})
                    directional.append({'scenario': scenario['id'], 'move': 'MOVE_' + scenario['move'],
                                        'mode': mode, 'clock': clock, 'rod': rod,
                                        'minimumRequiredProbability': {'land': fraction(Fraction(2, 25)),
                                                                       'fishing': fraction(Fraction(2, 25)) if rod == 'OLD_ROD' else 'NONZERO'},
                                        'ratings': observations})
    digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    if digest(REVISION / 'proposal.json') != g.load_json(REVISION / 'results.json')['distributions']['revised']['proposal_sha256']:
        raise g.ValidationError('Approved proposal and results hashes disagree')
    return {
        'schemaVersion': 1, 'product': 'WAYFARER',
        'criterion': 'Aggregate probability that the effective caught species knows the required move at its projected level.',
        'evidenceKind': 'STATIC_PRODUCTION_DATA_MODEL',
        'productionAcceptance': {'status': 'RUN_SEPARATELY', 'test': 'game/test/native_hm_catch_coverage.c',
                                 'fixtures': 'game/test/data/native_hm_coverage.h'},
        'assumptions': ['Uniform authored-level and eligible-slot rolls; runtime modulo bias, lures and lead modifiers excluded.',
                        'Known moves are modeled from preprocessed production learnsets, checked against the approved roster; C initial-moveset tests must pass independently.',
                        'Reachability and return paths follow the reviewed directional inventory; this is not an E2E route proof.',
                        'Rod and capture supplies are prerequisites. Blackthorn and Den mandatory proofs use local rank-zero sources only.',
                        'Hoenn static day populations serve both clock cases. Good and Super Rod require nonzero coverage; the 8% floor applies to Old Rod or land.'],
        'assignmentSha256': digest(REVISION / 'proposal.json'),
        'sourceSpeciesCount': len({species for species, _, _ in model.windows}),
        'scenarioInventory': scenarios,
        'routeEvidence': [str(REVISION.relative_to(g.ROOT.parent) / name) for name in
                          ('hns-coasts.md', 'blackthorn-routes.md', 'hoenn-routes.md')],
        'sourceSha256': {str(p.relative_to(g.ROOT.parent)): digest(p) for p in
                         (g.DEFAULT_ENCOUNTERS, g.DEFAULT_WAYFARER_NATIVE_HM_ENCOUNTERS, g.DEFAULT_SCALING,
                          g.DEFAULT_SPECIES_METADATA, g.DEFAULT_STANDARD_ROD_FISHING, REVISION / 'roster.json', REVISION / 'scenarios.json',
                          g.ROOT / 'src/data/pokemon/level_up_learnsets/gen_7.h', g.ROOT / 'src/data/pokemon/level_up_learnsets/gen_3.h')},
        'encounterReplacements': model.replacements,
        'qualityWeights': model.rods['qualityWeights'],
        'nativeSurfAccessibility': [r for r in directional if r['move'] == 'MOVE_SURF'],
        'localWhirlpoolAccessibility': [r for r in directional if r['move'] == 'MOVE_WHIRLPOOL'],
        'regions': {
            region: {'coverageCriterion': 'ROSTER_KNOWN_MOVE', 'protectedAnchors' if region == 'JOHTO' else 'nativeHmCertificates':
                     [r for r in regional if r['region'] == region]}
            for region in ('JOHTO', 'KANTO', 'HOENN')},
        'regionalCellCount': sum(len(r['ratings']) for r in regional),
        'directionalCellCount': sum(len(r['ratings']) for r in directional),
        'invariants': {'passed': not failures, 'failures': failures},
    }


def summarize(report):
    rows = report['nativeSurfAccessibility'] + report['localWhirlpoolAccessibility']
    minimums, below_floor = {}, []
    for rod in g.FISHING_QUALITIES:
        observations = [(row, cell) for row in rows if row['rod'] == rod for cell in row['ratings']]
        minimums[rod] = fraction(min(Fraction(**cell['best']['successfulEncounterProbability']) for _, cell in observations))
        below_floor.extend({'scenario': row['scenario'], 'mode': row['mode'], 'clock': row['clock'],
                            'rod': rod, 'rating': cell['rating'], 'best': cell['best']}
                           for row, cell in observations
                           if Fraction(**cell['best']['successfulEncounterProbability']) < Fraction(2, 25))
    return {
        key: report[key] for key in ('schemaVersion', 'product', 'criterion', 'evidenceKind',
                                     'productionAcceptance', 'assumptions', 'assignmentSha256',
                                     'sourceSha256', 'sourceSpeciesCount', 'regionalCellCount',
                                     'directionalCellCount', 'qualityWeights', 'invariants')
    } | {
        'encounterReplacementCount': len({row['replacement'] for row in report['encounterReplacements']}),
        'changedProfileSlotCount': len(report['encounterReplacements']),
        'minimumDirectionalSuccessfulEncounterProbability': minimums,
        'belowEightPercentUpgradedRodCells': below_floor,
        'distinctBestDirectionalSources': len({(cell['best']['map'], cell['best']['method'], cell['best']['timeOfDay'], cell['best']['rod'])
                                               for row in rows for cell in row['ratings']}),
        'regionalWitnessScope': 'Approved regional witnesses include optional and late areas, including Whirl Islands. Their flags are explicit in the full report; none certify practical acquisition. The 11 practical scenarios exclude Whirl Islands.',
    }


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--summary', type=Path)
    args = parser.parse_args()
    report = build_report()
    serialized = json.dumps(report, separators=(',', ':'), sort_keys=True) + '\n'
    if args.output.suffix == '.gz':
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(gzip.compress(serialized.encode('utf-8'), mtime=0))
    else:
        g.atomic_write(args.output, serialized)
    if args.summary:
        g.atomic_write(args.summary, json.dumps(summarize(report), indent=2, sort_keys=True) + '\n')
    if report['invariants']['failures']:
        raise SystemExit('; '.join(report['invariants']['failures']))
    print(f"Wayfarer modeled coverage passed: {report['regionalCellCount']} regional and {report['directionalCellCount']} directional cells")
