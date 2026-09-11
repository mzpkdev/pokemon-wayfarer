"""Read the map compiler's selected, effective projection, retaining raw evidence."""
import json
from pathlib import Path
import subprocess
from .common import ContentError, PRODUCTS, fingerprint, load_json

def normalize_inventory(document, product):
    selected = 'firered' if product == 'leafgreen' else product
    if document.get('schemaVersion') != 1:
        raise ContentError('SCHEMA', '<mapjson>', '', 'schemaVersion')
    if document.get('product') != selected:
        raise ContentError('INACTIVE', '<mapjson>', product, document.get('product'))
    result = []
    seen = set()
    for original in document['maps']:
        row = dict(original)
        if row['id'] in seen:
            raise ContentError('DUPLICATE', row['sourcePath'], row['id'], row['id'])
        seen.add(row['id'])
        row['objects'] = [dict(event, runtimeId=i + 1, localId=event.get('local_id'))
                          for i, event in enumerate(row['effective'].get('object_events', []))]
        row['sourceFingerprint'] = fingerprint(row['raw'])
        row['effectiveFingerprint'] = fingerprint(row['effective'])
        row['key'] = [product, row['sourceNamespace'], 'map', row['id']]
        result.append(row)
    return sorted(result, key=lambda row: row['key'])

def load_maps(root, product, mapjson=None):
    root = Path(root)
    if product not in PRODUCTS:
        raise ContentError('SCHEMA', '<configuration>', 'product', product)
    # Validate every source before json11 can discard duplicate object keys.
    for path in sorted((root / 'data/maps').glob('*/map.json')):
        load_json(path)
    load_json(root / 'data/maps/map_groups.json')
    binary = Path(mapjson or root / 'tools/mapjson/mapjson').resolve()
    selected = 'firered' if product == 'leafgreen' else product
    command = [str(binary), 'inventory', selected, 'data/maps/map_groups.json']
    # The inventory is a view of the compiler's selected map catalog.  Wayfarer
    # releases use the reviewed Sevii manifest; synthetic roots without one
    # deliberately retain the unextended catalog behavior.
    manifest = root / 'src/data/wayfarer_sevii_maps.json'
    if product == 'wayfarer' and manifest.is_file():
        command += ['--wayfarer-sevii-manifest', str(manifest.relative_to(root))]
    process = subprocess.run(command,
                             cwd=root, text=True, capture_output=True)
    if process.returncode:
        raise ContentError('UNRESOLVED', '<mapjson>', product, process.stderr)
    return normalize_inventory(json.loads(process.stdout), product)

def resolve_object(record, binding, path, key):
    matches = [event for event in record['objects'] if event.get('script') == binding['script']]
    if 'localId' in binding:
        local_id = binding['localId']
        if not isinstance(local_id, str) or not local_id or local_id.isdigit():
            raise ContentError('SCHEMA', path, key, local_id)
        matches = [event for event in matches if event['localId'] == local_id]
    if len(matches) != 1:
        raise ContentError('UNRESOLVED' if not matches else 'CONFLICT', path, key, binding)
    return matches[0]
