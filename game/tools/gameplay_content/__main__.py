"""Compile selected content into one immutable configuration generation."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import tempfile
from .common import ContentError, fingerprint
from .configuration import numeric_defines
from .maps import load_maps
from .trainers import load_trainers

ROOT = Path(__file__).resolve().parents[2]

def inputs(root):
    patterns = ('data/maps/*/map.json', 'data/maps/*/gameplay.json',
                'data/maps/*/scripts.inc', 'data/maps/map_groups.json',
                'data/scripts/**/*.inc', 'data/event_scripts.s',
                'data/gameplay/*.json', 'src/data/*.party', 'src/data/wayfarer_sevii_maps.json',
                'src/data/wayfarer_marts.h', 'src/wayfarer_persistence.c', 'include/regions.h',
                'include/config/*.h', 'include/constants/*.h',
                'include/constants/global.h', 'include/constants/flags*.h',
                'include/constants/wayfarer_persistence.h',
                'include/constants/wayfarer_marts.h',
                'tools/gameplay_content/**/*.py', 'tools/mapjson/*.cpp',
                'tools/mapjson/*.h', 'tools/trainerproc/*.c',
                'tools/trainer_scaling/*.py', 'tools/wayfarer_story_encounters/*.py',
                'src/data/wayfarer_story_encounter_*.h', 'src/wayfarer_story_encounter.c',
                'include/constants/wayfarer_story_encounters.h',
                'include/constants/battle_setup.h', 'include/constants/opponents*.h',
                'include/constants/trainers.h', 'include/wayfarer_story_encounter.h',
                'asm/macros*.inc', 'asm/macros/*.inc', 'asm/macros/event/*.inc',
                'constants/*.inc', 'data/wayfarer_*source_constants.inc',
                'tools/wayfarer_source_constants/*.py', 'tools/wayfarer_sevii_scripts/*.py',
                'tools/preproc/*.cpp', 'tools/preproc/*.h', 'charmap.txt')
    paths = {path for pattern in patterns for path in root.glob(pattern) if path.is_file()}
    return {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted(paths)}

def compile_content(root, product, cpp, cppflags, domains=("inventory", "progression", "services", "encounters"),
                    *, assembler='arm-none-eabi-as', asflags=()):
    defines = numeric_defines(root, cpp, cppflags)
    selected = ('wayfarer' if defines['IS_WAYFARER'] else 'hns' if defines['IS_HNS']
                else 'leafgreen' if defines['GAMEPLAY_LEAFGREEN'] else 'firered' if defines['IS_FRLG']
                else 'emerald')
    if selected != product:
        raise ContentError('INACTIVE', '<configuration>', product, selected)
    maps = load_maps(root, product)
    trainers = load_trainers(root, product, cpp, cppflags)
    hashes = inputs(root)
    config = {'schemaVersion': 1, 'adapterVersion': 2, 'product': product,
              'defines': defines, 'domains': sorted(domains), 'cpp': cpp, 'cppflags': cppflags,
              'assembler': assembler, 'asflags': asflags, 'inputHashes': hashes}
    report = dict(config, maps=maps, trainers=trainers,
                  selectedCounts={'maps': len(maps), 'trainers': len(trainers)},
                  unresolvedCoverage=[], legacyCoverage={},
                  runtimeResourceEvidence={'phaseA': 'host-only; linked measurements required for runtime adoption'})
    outputs = {}
    if 'progression' in domains:
        from . import progression
        outputs['gameplay_progression.inc'] = progression.render_runtime()
        report['progression'] = progression.load()
    if 'services' in domains:
        from .services import compile_services
        services = compile_services(root, product, defines, maps, cpp=cpp, cppflags=cppflags)
        outputs.update(services['outputs'])
        report['services'] = services['report']
    if 'encounters' in domains:
        from .encounters import compile_encounters
        encounters = compile_encounters(root, product, defines, maps, trainers, cpp=cpp, cppflags=cppflags,
                                        service_bindings=outputs.get('gameplay_mart_bindings.inc'),
                                        assembler=assembler, asflags=asflags)
        outputs.update(encounters['outputs'])
        report['encounters'] = encounters['report']
        hashes.update(encounters['report'].get('scriptInputHashes', {}))
    # Domain adapters can contribute actual compiler-discovered source inputs.
    # Fingerprint after their closure is complete, before immutable publication.
    digest = fingerprint(config)
    report['configurationDigest'] = digest
    outputs['inventory.json'] = json.dumps(report, indent=2, sort_keys=True) + '\n'
    return digest, outputs

def publish(output_root, product, digest, outputs, check=False):
    parent = Path(output_root) / product
    destination = parent / digest
    stale = [name for name, text in outputs.items()
             if not (destination / name).is_file() or (destination / name).read_text() != text]
    if check:
        if stale or not (parent / 'current').is_symlink() or (parent / 'current').resolve() != destination.resolve():
            raise ContentError('STALE_REVIEW', destination, '', ', '.join(stale) or 'current generation')
        return destination
    parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix='.generation-', dir=parent))
    try:
        for name, text in outputs.items():
            path = temporary / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text)
        if destination.exists():
            if stale:
                raise ContentError('CONFLICT', destination, '', 'immutable generation differs')
        else:
            temporary.rename(destination)
        link = parent / ('.current-' + temporary.name)
        link.symlink_to(digest, target_is_directory=True)
        os.replace(link, parent / 'current')
    finally:
        if temporary.exists():
            shutil.rmtree(temporary)
    return destination

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('command', choices=('generate', 'check', 'report'))
    parser.add_argument('--product', required=True, choices=('wayfarer', 'hns', 'emerald', 'firered', 'leafgreen'))
    parser.add_argument('--domains', nargs='+', choices=('inventory', 'progression', 'services', 'encounters'),
                        default=('inventory', 'progression', 'services', 'encounters'))
    parser.add_argument('--cpp', default='cpp')
    parser.add_argument('--cppflags', required=True)
    parser.add_argument('--assembler', default='arm-none-eabi-as')
    parser.add_argument('--asflags', default='')
    parser.add_argument('--output-root', default='build/gameplay-content')
    args = parser.parse_args()
    try:
        digest, outputs = compile_content(ROOT, args.product, args.cpp, args.cppflags, args.domains,
                                         assembler=args.assembler, asflags=args.asflags)
        directory = publish(args.output_root, args.product, digest, outputs, args.command == 'check')
        print(directory / 'inventory.json' if args.command == 'report' else directory)
    except (ContentError, ValueError) as exc:
        parser.exit(1, str(exc) + '\n')

if __name__ == '__main__':
    main()
