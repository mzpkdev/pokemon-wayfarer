"""Silph source roster and runtime script isolation contracts."""
import json
import re
import unittest
import generate as gen

TRAINERS = {'TRAINER_TEAM_ROCKET_GRUNT_23': 'TRAINER_SILPH_GRUNT_23_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_24': 'TRAINER_SILPH_GRUNT_24_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_25': 'TRAINER_SILPH_GRUNT_25_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_26': 'TRAINER_SILPH_GRUNT_26_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_27': 'TRAINER_SILPH_GRUNT_27_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_28': 'TRAINER_SILPH_GRUNT_28_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_29': 'TRAINER_SILPH_GRUNT_29_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_30': 'TRAINER_SILPH_GRUNT_30_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_31': 'TRAINER_SILPH_GRUNT_31_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_32': 'TRAINER_SILPH_GRUNT_32_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_33': 'TRAINER_SILPH_GRUNT_33_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_34': 'TRAINER_SILPH_GRUNT_34_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_35': 'TRAINER_SILPH_GRUNT_35_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_36': 'TRAINER_SILPH_GRUNT_36_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_37': 'TRAINER_SILPH_GRUNT_37_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_38': 'TRAINER_SILPH_GRUNT_38_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_39': 'TRAINER_SILPH_GRUNT_39_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_40': 'TRAINER_SILPH_GRUNT_40_HNS', 'TRAINER_TEAM_ROCKET_GRUNT_41': 'TRAINER_SILPH_GRUNT_41_HNS', 'TRAINER_SCIENTIST_BEAU': 'TRAINER_SILPH_SCIENTIST_BEAU_HNS', 'TRAINER_SCIENTIST_CONNOR': 'TRAINER_SILPH_SCIENTIST_CONNOR_HNS', 'TRAINER_SCIENTIST_ED': 'TRAINER_SILPH_SCIENTIST_ED_HNS', 'TRAINER_SCIENTIST_JERRY': 'TRAINER_SILPH_SCIENTIST_JERRY_HNS', 'TRAINER_SCIENTIST_JOSE': 'TRAINER_SILPH_SCIENTIST_JOSE_HNS', 'TRAINER_SCIENTIST_JOSHUA': 'TRAINER_SILPH_SCIENTIST_JOSHUA_HNS', 'TRAINER_SCIENTIST_PARKER': 'TRAINER_SILPH_SCIENTIST_PARKER_HNS', 'TRAINER_SCIENTIST_RODNEY': 'TRAINER_SILPH_SCIENTIST_RODNEY_HNS', 'TRAINER_SCIENTIST_TAYLOR': 'TRAINER_SILPH_SCIENTIST_TAYLOR_HNS', 'TRAINER_SCIENTIST_TRAVIS': 'TRAINER_SILPH_SCIENTIST_TRAVIS_HNS', 'TRAINER_JUGGLER_DALTON': 'TRAINER_SILPH_DALTON_HNS', 'TRAINER_BOSS_GIOVANNI_2': 'TRAINER_SILPH_GIOVANNI_HNS'}

class SilphContracts(unittest.TestCase):
    def test_authored_rosters_ids_and_policy(self):
        authored = (gen.ROOT / 'src/data/trainers_wayfarer.party').read_text()
        source = (gen.ROOT / 'src/data/trainers_frlg.party').read_text()
        ids = gen.trainer_ids(TRAINERS.values())
        rows = {row['id']: row for row in json.loads(gen.MANIFEST.read_text())['records']}
        self.assertEqual(list(ids[name] for name in TRAINERS.values()), list(range(1556, 1587)))
        for old, new in TRAINERS.items():
            self.assertEqual(authored.split('=== ' + new + ' ===')[1].split('===')[0].strip(),
                             source.split('=== ' + old + ' ===')[1].split('===')[0].strip())
            self.assertEqual(rows[new]['policy'], 'EXCLUDED' if 'GIOVANNI' in new else 'ORDINARY')

    def test_active_references_exclude_unlinked_source_and_blue(self):
        refs = gen.references()
        self.assertTrue(set(TRAINERS.values()).issubset(refs))
        self.assertTrue(set(TRAINERS).isdisjoint(refs))
        self.assertFalse(any('data/maps/SilphCo_' in path for paths in refs.values() for path in paths))

    def test_transaction_and_loss_boundaries(self):
        script = (gen.ROOT / 'data/scripts/wayfarer_silph.inc').read_text()
        self.assertNotRegex(script, r'\b(?:VAR_MAP_SCENE_SILPH_CO_7F|VAR_MAP_SCENE_SILPH_CO_11F|VAR_ELEVATOR_FLOOR|ITEM_CARD_KEY)\b')
        self.assertEqual(script.count('setflag FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS'), 1)
        self.assertEqual(script.count('clearflag FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS'), 1)
        self.assertEqual(script.count('setflag FLAG_SILPH_MASTER_BALL_RECEIVED_HNS'), 1)
        president = script.split('WayfarerSilph_EventScript_PresidentLiberated::')[1].split('SilphCo_11F_EventScript_Secretary::')[0]
        self.assertLess(president.index('checkitemspace ITEM_MASTER_BALL'), president.index('giveitem_msg SilphCo_11F_Text_ObtainedMasterBallFromPresident'))
        self.assertLess(president.index('giveitem_msg SilphCo_11F_Text_ObtainedMasterBallFromPresident'), president.index('setflag FLAG_SILPH_MASTER_BALL_RECEIVED_HNS'))
        self.assertLess(president.index('setflag FLAG_SILPH_MASTER_BALL_RECEIVED_HNS'), president.index('clearflag FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS'))
        self.assertEqual(script.count('trainerbattle_single TRAINER_SILPH_'), 30)
        battle = script.split('SilphCo_11F_EventScript_BattleGiovanni::')[1].split('SilphCo_11F_EventScript_GiovanniApproachLeft::')[0]
        self.assertLess(battle.index('B_OUTCOME_WON'), battle.index('setflag FLAG_SILPH_LIBERATED_HNS'))
        self.assertIn('B_OUTCOME_WON, WayfarerSilph_EventScript_GiovanniRetry', battle)
        self.assertIn('setobjectxy LOCALID_SILPH_CO_GIOVANNI, 6, 11', script)
        for trigger in ('SilphCo_11F_EventScript_GiovanniTriggerLeft',
                        'SilphCo_11F_EventScript_GiovanniTriggerRight'):
            branch = script.split(trigger + '::')[1].split('\n\n', 1)[0]
            self.assertLess(branch.index('lockall'), branch.index('WayfarerCanStartOrdinaryBattleForScript'))
            self.assertIn('FALSE, WayfarerSilph_EventScript_RocketNoParty', branch)
        self.assertEqual(len(re.findall(r'checkitemspace ITEM_', script)), 19)
        self.assertNotIn('VAR_STARTER_MON', script)

if __name__ == '__main__':
    unittest.main()
