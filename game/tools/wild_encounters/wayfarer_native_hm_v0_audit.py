#!/usr/bin/env python3
"""Audit Wayfarer v0 native-HM coverage using production encounter/evolution inputs."""
import argparse
from collections import Counter
import csv
from fractions import Fraction
from functools import lru_cache
from pathlib import Path
import json
import sys

GAME = Path(__file__).resolve().parents[2]
ROOT = GAME.parent
sys.path.insert(0, str(GAME / 'tools/wild_encounters'))
sys.path.insert(0, str(GAME / 'tools/notable_trainers'))
import wayfarer_native_hm_audit as audit
import wild_encounters_to_header as g
import evolution

APPROVED = ROOT / '.product/research/native-hm-windows/revisions/nearby-access'
FIXTURE = GAME / 'test/data/native_hm_v0_coverage.h'
CAP = 160
ANCHORS = ((0, 6), (40, 24), (80, 40), (120, 58), (160, 78))
RETIRED_MAPS = {
    'MAP_CINNABAR_ISLAND_HNS': 'MAP_CINNABAR_ISLAND',
    'MAP_ROUTE19_HNS': 'MAP_ROUTE19',
    'MAP_SEAFOAM_ISLANDS_1F_HNS': 'MAP_SEAFOAM_ISLANDS_1F',
    'MAP_SEAFOAM_ISLANDS_B1F_HNS': 'MAP_SEAFOAM_ISLANDS_B1F',
}


def baseline(rating):
    for (start, low), (end, high) in zip(ANCHORS, ANCHORS[1:]):
        if rating < end:
            width = end - start
            rise = (rating - start) * (high - low)
            return low + (2 * rise + width) // (2 * width)
    return ANCHORS[-1][1]


def make_model():
    model = audit.Model()
    encounters, _ = g.apply_wayfarer_encounter_replacements(g.load_json(g.DEFAULT_ENCOUNTERS))
    config = g.Config(g.DEFAULT_CONFIG, g.DEFAULT_RTC, encounters)
    profiles, _ = g.validate_encounters(encounters, g.species_ids(g.DEFAULT_SPECIES), config)
    offsets = g.load_offsets(model.scaling['profile_offsets'], profiles, g.DEFAULT_SCALING)
    offset_map = {(r['product'], r['header_id'], r['area'], r['time'], r['rod']): r['level_offset'] for r in offsets}
    for p in profiles:
        if p['product'] != 'POKEMON_WAYFARER':
            continue
        for method in config.mon_types:
            if method not in p['encounter']:
                continue
            for rod in g.FISHING_QUALITIES if method == 'fishing_mons' else ('NONE',):
                identity = (p['map'], method, p['time'], rod)
                offset = offset_map.get((p['product'], p['header_id'], g.METHOD_AREAS[method], p['time'], g.RODS[rod]), 0)
                model.profiles[identity] = {
                    'label': p['label'], 'offset': offset,
                    'slots': tuple((mon['species'], mon.get('min_level', 2), mon.get('max_level', 100), weight)
                                   for _, mon, weight in g.method_slots(p, method, rod, model.rods)),
                }
    graph = evolution.game_evolutions()
    edges = {}
    for species, predecessor, authored in evolution.rows():
        if authored == 0:
            levels = {int(edge['parameter']) for edge in graph[predecessor]
                      if edge['target'] == species and edge['method'] in ('EVO_LEVEL', 'EVO_LEVEL_BATTLE_ONLY')
                      and edge['parameter'].isdecimal() and int(edge['parameter']) > 0}
            assert len(levels) == 1, (species, levels)
            authored = levels.pop()
        edges[species] = predecessor, authored

    @lru_cache(maxsize=None)
    def level(authored, rating, offset):
        high_water = -999
        for tr in range(min(rating, CAP) + 1):
            point = model.scaling['points'][tr // 2]
            raw = baseline(tr) + g.divide_round_signed(
                (authored - 6) * point['retention_numerator'], point['retention_denominator'])
            high_water = max(high_water, raw)
        return max(1, min(100, high_water + offset))

    @lru_cache(maxsize=None)
    def slot(species, low, high, rating, offset):
        if species == 'SPECIES_NONE':
            return ()
        outcomes = Counter()
        for authored in range(low, high + 1):
            projected = level(authored, rating, offset)
            effective = species
            for _ in range(8):
                edge = edges.get(effective)
                if edge is None or projected >= edge[1]:
                    break
                effective = edge[0]
            else:
                raise AssertionError(('ancestry depth', species, projected))
            metadata = model.metadata.get(effective)
            if metadata and projected < metadata['minimum_level']:
                return ()
            outcomes[effective, projected] += 1
        return tuple((species, projected, count) for (species, projected), count in outcomes.items())

    model.level = level
    model.slot = slot
    return model


def source_identity(row):
    return (RETIRED_MAPS.get(row['map'], row['map']), row['method'], row['time'], row['rod'])


def regional_inventory(model):
    inventory = {region: set() for region in ('JOHTO', 'KANTO', 'HOENN')}
    exclusions = ('SAFARI', 'ALTERING_CAVE', 'UNUSED', 'SKY_PILLAR', 'SEAFLOOR',
                  'CAVE_OF_ORIGIN', 'VICTORY_ROAD', 'MT_SILVER', 'TIN_TOWER',
                  'MAGMA_HIDEOUT', 'UNDERWATER', 'DESERT_UNDERPASS')
    with (APPROVED / 'locations.csv').open(newline='') as file:
        for row in csv.DictReader(file):
            if row['region'] not in inventory or any(term in row['map'] for term in exclusions):
                continue
            for rod in g.FISHING_QUALITIES if row['method'] == 'fishing_mons' else ('NONE',):
                identity = source_identity(dict(map=row['map'], method=row['method'], time=row['time'], rod=rod))
                if identity in model.profiles:
                    inventory[row['region']].add(identity)
    return inventory


def run():
    model = make_model()
    regional_rows = [r for r in json.loads((APPROVED / 'coverage.json').read_text())['results']
                     if r['mode'] == 'modern']
    scenarios = json.loads((APPROVED / 'scenarios.json').read_text())['scenarios']
    assert len(regional_rows) == 24 and len(scenarios) == 11
    missing = []
    inventory = regional_inventory(model)
    regional_failures = []
    witness_rows = []
    for row in regional_rows:
        historical = [source_identity(row['witnesses'][str(r)]) for r in range(81)]
        missing.extend(('regional', row['region'], row['move'], i) for i in historical if i not in model.profiles)
        identities = sorted(identity for identity in inventory[row['region']]
                            if row['move'] != 'MOVE_SURF' or identity[1] in ('land_mons', 'fishing_mons'))
        chosen = []
        for tr in range(CAP + 1):
            prior = historical[min(tr, 80)]
            candidates = list(dict.fromkeys([prior, *historical]))
            witness = None
            for identity in candidates:
                if identity in model.profiles and model.measure(identity, 'modern', row['move'], tr)[0]:
                    witness = identity
                    break
            if witness is None:
                chance, witness = max(((model.measure(identity, 'modern', row['move'], tr)[0], identity)
                                       for identity in identities), default=(Fraction(0), None),
                                      key=lambda item: item[0])
                if not chance:
                    witness = None
            if witness is None:
                regional_failures.append((row['region'], row['move'], tr))
            chosen.append(witness)
        witness_rows.append((row, chosen))
    directional_failures = []
    for scenario in scenarios:
        for clock in ('DAY', 'NIGHT'):
            for rod in g.FISHING_QUALITIES:
                identities = []
                for source in scenario['maps']:
                    if scenario['id'] in ('blackthorn_surf', 'den_whirlpool') and source['rank'] != 0:
                        continue
                    time = 'TIME_' + clock if source['map'].endswith('_HNS') else 'TIME_DAY'
                    for method in source['methods']:
                        identity = (RETIRED_MAPS.get(source['map'], source['map']), method, time,
                                    rod if method == 'fishing_mons' else 'NONE')
                        if identity in model.profiles:
                            identities.append(identity)
                        else:
                            missing.append(('scenario-source', scenario['id'], clock, rod, identity))
                if not identities:
                    missing.append(('scenario', scenario['id'], clock, rod))
                for tr in range(CAP + 1):
                    choices = [(model.measure(identity, 'modern', 'MOVE_' + scenario['move'], tr)[0], identity)
                               for identity in identities]
                    qualifying = [(chance, identity) for chance, identity in choices
                                  if chance >= Fraction(2, 25)
                                  if identity[1] == 'land_mons' or rod == 'OLD_ROD']
                    if rod != 'OLD_ROD':
                        qualifying += [(chance, identity) for chance, identity in choices
                                       if identity[1] == 'fishing_mons' and chance > 0]
                    if not qualifying:
                        best = max(choices, default=(Fraction(0), None))
                        directional_failures.append((scenario['id'], clock, rod, tr, str(best[0]), best[1]))
    result = {
        'inputs': {'runtime': 'game/src/wild_encounter.c', 'encounters': 'game/src/data/wild_encounters.json',
                   'overlays': 'game/src/data/wayfarer_native_hm_encounters.json',
                   'predecessors': 'game/src/data/notable_moves/predecessors.h',
                   'learnsets': 'game/src/data/pokemon/level_up_learnsets/gen_7.h',
                   'regional': str(APPROVED / 'coverage.json'), 'directional': str(APPROVED / 'scenarios.json')},
        'profileCount': len(model.profiles), 'missingSources': missing,
        'regionalFailures': regional_failures, 'directionalFailures': directional_failures,
        'regionalWitnesses': [{'region': row['region'], 'move': row['move'],
                               'profiles': [list(i) if i else None for i in chosen]}
                              for row, chosen in witness_rows],
    }
    return result


def render_fixture(result):
    identities = sorted({tuple(identity) for row in result['regionalWitnesses'] for identity in row['profiles']
                         if identity is not None})
    indexes = {identity: index for index, identity in enumerate(identities)}
    areas = {'land_mons': 'WILD_AREA_LAND', 'water_mons': 'WILD_AREA_WATER',
             'rock_smash_mons': 'WILD_AREA_ROCKS', 'fishing_mons': 'WILD_AREA_FISHING'}
    rods = {'NONE': 'NONE', 'OLD_ROD': 'OLD', 'GOOD_ROD': 'GOOD', 'SUPER_ROD': 'SUPER'}
    lines = ['// Generated by tools/wild_encounters/wayfarer_native_hm_v0_audit.py; do not edit.',
             'static const struct CoverageProfile sV0RegionalProfiles[] =', '{']
    for map_name, method, time, rod in identities:
        lines.append(f'    {{ {map_name}, {time}, {areas[method]}, WILD_ENCOUNTER_FISHING_ROD_{rods[rod]} }},')
    lines += ['};', '', 'static const struct RegionalCoverage sV0RegionalCoverage[] =', '{']
    for row in result['regionalWitnesses']:
        indices = [indexes[tuple(identity)] for identity in row['profiles']]
        lines.append(f'    {{ "{row["region"]}/{row["move"]}", {row["move"]},')
        lines.append('      { ' + ', '.join(map(str, indices)) + ' } },')
    lines += ['};', '']
    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true', help='verify all v0 cells and the checked-in fixture')
    parser.add_argument('--write-fixture', action='store_true', help='regenerate the checked-in v0 regional fixture')
    parser.add_argument('--report', type=Path, help='write detailed cell failures and witnesses as JSON')
    args = parser.parse_args()
    if args.check and args.write_fixture:
        parser.error('--check and --write-fixture are mutually exclusive')
    result = run()
    if args.report:
        args.report.write_text(json.dumps(result, indent=2) + '\n')
    failures = len(result['regionalFailures']) + len(result['directionalFailures'])
    print(f'v0 native HM: {failures} failing cells; {len(result["missingSources"])} absent optional profiles')
    if failures:
        print('regional:', result['regionalFailures'][:12])
        print('directional:', result['directionalFailures'][:12])
        raise SystemExit(1)
    fixture = render_fixture(result)
    if args.check and (not FIXTURE.exists() or FIXTURE.read_text() != fixture):
        raise SystemExit(f'out-of-date v0 native HM fixture: {FIXTURE}')
    if args.write_fixture:
        FIXTURE.write_text(fixture)
        print(f'wrote {FIXTURE}')


if __name__ == '__main__':
    main()
