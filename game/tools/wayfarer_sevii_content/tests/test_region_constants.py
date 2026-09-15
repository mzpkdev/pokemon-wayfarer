"""Compile real generated map-section constants for each HNS-engine product."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

GAME = Path(__file__).resolve().parents[3]


class RegionConstantTests(unittest.TestCase):
    def test_standalone_hns_aliases_and_wayfarer_real_ids(self):
        data = GAME / 'src/data/region_map/region_map_sections.json'
        template = data.with_name('region_map_sections.constants.json.txt')
        sections = [row['id'] for row in json.loads(data.read_text())['map_sections']
                    if row.get('wayfarer_sevii')]
        self.assertTrue(sections)
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            header = directory / 'sections.h'
            subprocess.run([str(GAME / 'tools/jsonproc/jsonproc'), str(data), str(template), str(header)], check=True, capture_output=True)
            for product in ('POKEMON_HNS', 'POKEMON_WAYFARER'):
                comparison = '== 0' if product == 'POKEMON_HNS' else '> 0'
                source = '#include "sections.h"\n' + '\n'.join(
                    f'_Static_assert({symbol} {comparison}, "{symbol}");' for symbol in sections)
                result = subprocess.run(['cc', '-x', 'c', '-fsyntax-only', f'-D{product}',
                                         '-I', str(directory), '-I', str(GAME / 'include'), '-'],
                                        input=source, text=True, capture_output=True)
                self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == '__main__':
    unittest.main()
