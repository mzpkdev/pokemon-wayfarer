"""Independent numerical oracle for the shipped progression migration."""
import copy
from decimal import Decimal, ROUND_HALF_UP
import json
from pathlib import Path
import tempfile
import unittest

from tools.gameplay_content import progression


class ProgressionTests(unittest.TestCase):
    def test_all_ratings_match_independent_baseline(self):
        ratings = (0, 4, 8, 16, 30, 40, 55, 65, 80)
        expected = {
            "wild_baseline": (5, 6, 8, 13, 20, 32, 50, 70, 90),
            "ordinary_trainer_baseline": (7, 8, 10, 15, 22, 34, 52, 72, 92),
            "soft_cap": (15, 16, 18, 23, 30, 42, 60, 80, 100),
            "gym_baseline": (15, 16, 18, 23, 30, 42, 60, 80, 100),
            "league_baseline": (15, 16, 18, 23, 30, 42, 60, 80, 100),
        }
        curves = progression.load()
        for name, levels in expected.items():
            for rating in range(-1, 83):
                clamped = min(80, max(0, rating))
                i = next(i for i in range(1, len(ratings)) if clamped <= ratings[i])
                value = Decimal(levels[i-1]) + Decimal(clamped-ratings[i-1]) * Decimal(levels[i]-levels[i-1]) / Decimal(ratings[i]-ratings[i-1])
                golden = int(value.quantize(Decimal(1), rounding=ROUND_HALF_UP))
                self.assertEqual(progression.evaluate(name, rating, curves), golden)

    def test_wild_projection_consumes_independent_baseline(self):
        from tools.wild_encounters import wild_encounters_to_header as wild
        scaling = wild.load_scaling(wild.DEFAULT_SCALING)
        ratings = (0, 4, 8, 16, 30, 40, 55, 65, 80)
        levels = (5, 6, 8, 13, 20, 32, 50, 70, 90)
        for rating in range(81):
            i = next(i for i in range(1, 9) if rating <= ratings[i])
            value = Decimal(levels[i-1]) + Decimal(rating-ratings[i-1]) * Decimal(levels[i]-levels[i-1]) / Decimal(ratings[i]-ratings[i-1])
            self.assertEqual(scaling['points'][rating]['anchor_level'],
                             int(value.quantize(Decimal(1), rounding=ROUND_HALF_UP)))

    def test_interning_does_not_couple_names(self):
        curves = progression.load()
        original = progression.render_runtime(curves)
        self.assertEqual(original.count("static const u8"), 2)
        curves["gym_baseline"] = ((0, 12), (80, 95))
        self.assertEqual(progression.evaluate("soft_cap", 0, curves), 15)
        self.assertEqual(progression.evaluate("gym_baseline", 0, curves), 12)
        self.assertEqual(progression.render_runtime(curves).count("static const u8"), 3)

    def test_invalid_schema(self):
        source = json.loads(progression.SOURCE.read_text())
        mutations = [lambda s: s.update(schemaVersion=True), lambda s: s.update(extra=1),
                     lambda s: s['curves'].append(s['curves'][0]),
                     lambda s: s['curves'][0].update(interpolation='floor'),
                     lambda s: s['curves'][0]['points'][0].update(rating=1),
                     lambda s: s['curves'][0]['points'][1].update(value=0),
                     lambda s: s['curves'][0]['points'][1].update(rating=True)]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'progression.json'
            for mutate in mutations:
                invalid = copy.deepcopy(source)
                mutate(invalid)
                path.write_text(json.dumps(invalid))
                with self.assertRaises(ValueError):
                    progression.load(path)
            path.write_text('{"schemaVersion":1,"schemaVersion":1,"curves":[]}')
            with self.assertRaises(ValueError):
                progression.load(path)
