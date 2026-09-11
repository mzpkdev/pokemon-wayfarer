"""Shared strict input validation and deterministic fingerprints."""
import hashlib
import json
from pathlib import Path

PRODUCTS = frozenset(('wayfarer', 'hns', 'emerald', 'firered', 'leafgreen'))

class ContentError(ValueError):
    def __init__(self, category, path, key='', reference=''):
        self.category = category
        super().__init__(f'{category}: {path}: {key}: {reference}')

def load_json(path):
    def pairs(items):
        result = {}
        for key, value in items:
            if key in result:
                raise ContentError('DUPLICATE', path, key, 'JSON key')
            result[key] = value
        return result
    try:
        return json.loads(Path(path).read_text(), object_pairs_hook=pairs)
    except json.JSONDecodeError as exc:
        raise ContentError('SCHEMA', path, '', str(exc)) from exc

def fingerprint(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()

def check_review(value, expected, path, key):
    if fingerprint(value) != expected:
        raise ContentError('STALE_REVIEW', path, key, expected)

def assign_indexes(records):
    if len(records) > 65535:
        raise ContentError('CAPACITY', '<inventory>', '', len(records))
    result = sorted(records, key=lambda row: tuple(row['key']))
    seen = set()
    for index, row in enumerate(result):
        key = tuple(row['key'])
        if key in seen:
            raise ContentError('DUPLICATE', row.get('sourcePath', ''), key, key)
        seen.add(key)
        row['index'] = index
    return result
