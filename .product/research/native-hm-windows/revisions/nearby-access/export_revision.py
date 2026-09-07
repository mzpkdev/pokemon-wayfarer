#!/usr/bin/env python3
"""Export the selected design without overwriting the original research snapshot."""
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
RESEARCH = HERE.parents[1]
ROOT = RESEARCH.parents[2]
sys.path.insert(0, str(RESEARCH))
import analyze
import make_locations
from coverage import CoverageModel


def revised_model(proposal):
    model = CoverageModel()
    replacements = proposal.get('encounter_replacements', [])
    g = model.tool
    species = set(model.metadata) | {r['species'] for r in replacements}
    known = g.species_ids(g.DEFAULT_SPECIES)
    model.metadata = {r['species']: r for r in g.load_species_metadata(
        g.DEFAULT_SPECIES_METADATA, g.DEFAULT_SPECIES_INFO, known, species)}
    for replacement in replacements:
        count = 0
        for p in model.profiles:
            if p['map'] != replacement['map'] or p['method'] != replacement['method']:
                continue
            old, low, high, weight = p['slots'][replacement['slot']]
            expected = replacement['expected_species']
            assert old in ([expected] if isinstance(expected, str) else expected)
            p['slots'][replacement['slot']] = (
                replacement['species'], replacement.get('min_level', low),
                replacement.get('max_level', high), weight)
            count += 1
        assert count, replacement
    for p in model.profiles:
        possible = set()
        for species, _, _, _ in p['slots']:
            while species != 'SPECIES_NONE':
                possible.add(species)
                species = model.metadata[species]['predecessor']
        p['possible_species'] = possible
    return model


def family_check(baseline, rows):
    # Conservative source inventory: include all explicit branches and forms,
    # rather than excluding a branch using an assumed evolution condition.
    edges = defaultdict(set)
    for path in (ROOT / 'game/src/data/pokemon/species_info').glob('gen_*_families.h'):
        chunks = re.split(r'\[(SPECIES_\w+)\]\s*=', path.read_text())
        for source, body in zip(chunks[1::2], chunks[2::2]):
            if '.evolutions' not in body:
                continue
            section = body.split('.evolutions', 1)[1]
            section = re.split(r'\n\s*\.\w+\s*=', section)[0]
            edges[source].update(re.findall(r'\{EVO_\w+\s*,[^,]+,\s*(SPECIES_\w+)', section))
    def roles(species):
        if species in rows:
            return set(rows[species]['roles'])
        source = baseline.get(species)
        return set().union(*(set(v) for v in source['natural_utilities'].values())) if source else set()
    checked = 0
    def visit(path, utilities):
        nonlocal checked
        if set(path) & rows.keys():
            assert len(utilities) <= 2, (path, utilities)
            checked += 1
        for target in edges.get(path[-1], ()):
            assert target not in path, path
            visit(path + [target], utilities | roles(target))
    for source in set(edges) | set(rows):
        visit([source], roles(source))
    return {'explicit_evolution_sources': len(edges), 'selected_path_prefixes_checked': checked,
            'maximum_two_authored_utility_types': True,
            'scope': 'Explicit source branches plus baseline/proposal roles, not manually taught or egg moves.'}


def main():
    proposal = json.loads((HERE / 'proposal.json').read_text())
    baseline = json.loads((RESEARCH / 'baseline.json').read_text())['species']
    rows = analyze.build_design(baseline, proposal)
    audit = family_check(baseline, rows)
    model = revised_model(proposal)
    stats, regional = analyze.analyze(rows, model)
    analyze.HERE = HERE
    analyze.export(rows, stats, regional)
    make_locations.HERE = HERE
    make_locations.main()
    (HERE / 'family-audit.json').write_text(json.dumps(audit, indent=2) + '\n')
    print(audit)


if __name__ == '__main__':
    main()
