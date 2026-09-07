"""Check reviewed source identity and exact League levels for every input TR."""
from decimal import Decimal, ROUND_HALF_UP
import json
import unittest
import league

class LeagueScalingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rows = league.inventory()

    def test_reviewed_sources_and_generated_outputs(self):
        fixture = json.loads(league.Path(league.__file__).with_name('league_sources.json').read_text())
        self.assertEqual(fixture, [dict(trainer=r['trainer'], source=r['source']) for r in self.rows])
        self.assertEqual((league.OUT/'league.h').read_text(), league.render(self.rows))
        self.assertEqual(json.loads((league.OUT/'league.json').read_text()), json.loads(json.dumps(self.rows)))

    def test_every_integer_rating_matches_independent_decimal_oracle(self):
        for rating in range(81):
            for (r0,l0),(r1,l1) in zip(league.ANCHORS,league.ANCHORS[1:]):
                if rating <= r1:
                    expected = int((Decimal(l0)+Decimal(rating-r0)*Decimal(l1-l0)/Decimal(r1-r0)).quantize(Decimal(1), rounding=ROUND_HALF_UP))
                    break
            self.assertEqual(league.baseline(rating), expected)
            for row in self.rows:
                self.assertEqual(row['levels'][rating], [min(100,max(1,expected+row['encounterOffset']+offset)) for offset in row['offsets']])

    def test_explicit_aces_counts_order_and_monotonicity(self):
        self.assertEqual(len(self.rows),15)
        for i,row in enumerate(self.rows):
            self.assertEqual(row['owner'],row['trainer'])
            self.assertEqual(row['difficulty'],'DIFFICULTY_NORMAL')
            self.assertEqual(row['encounterIndex'],i%5)
            self.assertEqual(row['source']['partySize'],6 if i%5==4 or 5<=i<10 else 5)
            self.assertEqual(row['offsets'].count(0),1)
            self.assertEqual(row['offsets'][row['aceSlot']],0)
            self.assertEqual(row['source']['slots'][row['aceSlot']]['species'],'SPECIES_'+league.ROSTERS[i][1])
            for slot in range(row['source']['partySize']):
                levels=[levels[slot] for levels in row['levels']]
                self.assertEqual(levels,sorted(levels))
                self.assertEqual(len(row['source']['slots'][slot]['moves']),4)
        self.assertEqual(self.rows[9]['source']['slots'][-1]['species'],'SPECIES_ALTARIA')

    def test_examples_ties_clamp_and_champion_saturation(self):
        self.assertEqual([league.baseline(r) for r in (40,56,64,72,80)],[42,62,78,89,100])
        self.assertEqual(league.baseline(2),16)
        self.assertEqual(league.baseline(5),17)
        self.assertEqual(league.baseline(999),100)
        for i in (4,9,14):
            self.assertEqual(self.rows[i]['levels'][80][-1],100)
            self.assertEqual(self.rows[i]['levels'][80][1 if i==4 else 0],100)

if __name__=='__main__':
    unittest.main()
