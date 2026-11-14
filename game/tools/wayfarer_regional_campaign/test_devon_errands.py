"""Execute the Devon errands' scripts: the Rustboro theft, the two deliveries, Stern and the Slateport harbor.

The scripts run through the Johto story tests' interpreter, which checks state transactions and not
presentation: message boxes pass, battles are won, and `items` is what the Bag holds.
This is the Hoenn independent story beats spec, Devon sections.
"""
from itertools import permutations
from pathlib import Path
import importlib.util
import json
import unittest

GAME = Path(__file__).resolve().parents[2]

_spec = importlib.util.spec_from_file_location('johto_story_test_scripts', GAME / 'tools/johto_story/test_scripts.py')
_johto = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_johto)

RUSTBORO = 'VAR_RUSTBORO_CITY_STATE'
TUNNEL = 'VAR_RUSTURF_TUNNEL_STATE'
WOODS = 'VAR_PETALBURG_WOODS_STATE'
HARBOR = 'VAR_SLATEPORT_HARBOR_STATE'
STREET = 'VAR_SLATEPORT_CITY_STATE'
OUTSIDE_MUSEUM = 'VAR_SLATEPORT_OUTSIDE_MUSEUM_STATE'
SCOTT = 'VAR_SCOTT_STATE'
DELIVERED_GOODS = 'FLAG_DELIVERED_DEVON_GOODS'
DELIVERED_LETTER = 'FLAG_DELIVERED_STEVEN_LETTER'
QUEUE = 'FLAG_HIDE_SLATEPORT_CITY_TEAM_AQUA'
MUSEUM_GRUNTS = 'FLAG_HIDE_SLATEPORT_CITY_OCEANIC_MUSEUM_AQUA_GRUNTS'
MUSEUM_STERN = 'FLAG_HIDE_SLATEPORT_CITY_OCEANIC_MUSEUM_2F_CAPTAIN_STERN'
PATRONS = 'FLAG_HIDE_SLATEPORT_CITY_HARBOR_PATRONS'
PARTS = 'ITEM_DEVON_PARTS'
LETTER = 'ITEM_LETTER'
# Everything the Stern deliveries set, at the museum and at the harbor alike.
DELIVERY_FLAGS = {DELIVERED_GOODS, MUSEUM_GRUNTS, 'FLAG_HIDE_ROUTE_110_TEAM_AQUA',
                  'FLAG_HIDE_RUSTBORO_CITY_DEVON_CORP_3F_EMPLOYEE'}


def play(map_name, label, *, items=(), flags=(), vars=None, bag_full=False, answers=1):
    interp = _johto.Interp(map_name, flags=flags, vars=vars, bag_full=bag_full, answers=answers)
    interp.items = set(items)
    interp.run(label)
    return interp


def story_writes(interp):
    return [write for write in interp.writes if write[0] != 'VAR_RESULT' and not write[0].startswith('VAR_0x')]


def map_events(map_name):
    return json.loads((GAME / 'data/maps' / map_name / 'map.json').read_text())


class RustboroTheft(unittest.TestCase):
    TRIGGER = 'RustboroCity_EventScript_StolenGoodsTrigger2'

    def test_the_street_scene_waits_on_state_zero_at_its_five_tiles(self):
        triggers = [event for event in map_events('RustboroCity')['coord_events']
                    if 'StolenGoodsTrigger' in event['script']]
        self.assertEqual(sorted((event['x'], event['y']) for event in triggers), [(23, y) for y in range(20, 25)])
        for event in triggers:
            self.assertEqual((event['var'], event['var_value']), (RUSTBORO, '0'))

    def test_roxanne_writes_nothing_of_the_theft(self):
        for state in (0, 2, 3, 4, 5, 6, 7, 8):
            won = play('RustboroCity_Gym', 'RustboroCity_Gym_EventScript_RoxanneDefeated', vars={RUSTBORO: state})
            self.assertEqual(won.get(RUSTBORO), state)
            self.assertNotIn(RUSTBORO, [write[0] for write in won.writes])

    def test_the_scene_starts_the_theft_with_no_badge(self):
        scene = play('RustboroCity', self.TRIGGER)
        self.assertEqual(scene.get(RUSTBORO), 2)
        self.assertEqual(scene.get(TUNNEL), 2)
        self.assertIn('FLAG_DEVON_GOODS_STOLEN', scene.flags)
        for shown in ('FLAG_HIDE_RUSTURF_TUNNEL_PEEKO', 'FLAG_HIDE_RUSTURF_TUNNEL_AQUA_GRUNT',
                      'FLAG_HIDE_ROUTE_116_MR_BRINEY', 'FLAG_HIDE_RUSTBORO_CITY_DEVON_EMPLOYEE_1'):
            self.assertIn((shown, 0, 'flag'), scene.writes)

    def test_a_late_theft_never_rewinds_the_tunnel(self):
        self.assertEqual(play('RustboroCity', self.TRIGGER, vars={TUNNEL: 6}).get(TUNNEL), 6)

    def test_the_employee_recalls_the_woods_only_after_the_woods(self):
        ask = 'RustboroCity_EventScript_EmployeeAskToGetGoods'
        self.assertEqual(play('RustboroCity', ask, vars={WOODS: 1}).messages, ['RustboroCity_Text_HelpMeIWasRobbed'])
        self.assertEqual(play('RustboroCity', ask).messages, ['RustboroCity_Text_HelpMeIWasRobbedStranger'])

    def test_returning_the_goods_waits_for_room_and_pays_once(self):
        ret = 'RustboroCity_EventScript_ReturnGoods'
        full = play('RustboroCity', ret, vars={RUSTBORO: 4}, bag_full=True)
        self.assertEqual(full.get(RUSTBORO), 4)
        self.assertEqual(full.given, [])
        self.assertNotIn('FLAG_RETURNED_DEVON_GOODS', full.flags)
        room = play('RustboroCity', ret, vars={RUSTBORO: 4})
        self.assertEqual(room.get(RUSTBORO), 5)
        self.assertEqual(room.given, ['ITEM_GREAT_BALL'])
        self.assertIn('FLAG_RETURNED_DEVON_GOODS', room.flags)

    def test_the_stranger_is_not_given_another_ball(self):
        ret = 'RustboroCity_EventScript_ReturnGoods'
        self.assertIn('RustboroCity_Text_YouGotItThankYouStranger', play('RustboroCity', ret).messages)
        self.assertIn('RustboroCity_Text_YouGotItThankYou', play('RustboroCity', ret, vars={WOODS: 1}).messages)

    def test_the_route_116_employee_recalls_the_woods_only_after_the_woods(self):
        talk = 'Route116_EventScript_DevonEmployee'
        self.assertIn('Route116_Text_YouGotPackageBack', play('Route116', talk).messages)
        recalled = play('Route116', talk, vars={WOODS: 1}).messages
        self.assertIn('Route116_Text_YouHelpedInWoodsAndGotPackageBack', recalled)
        self.assertNotIn('Route116_Text_YouGotPackageBack', recalled)

    def test_the_tunnel_grunt_fights_only_with_room_for_the_parts(self):
        full = play('RusturfTunnel', 'RusturfTunnel_EventScript_Grunt', vars={RUSTBORO: 3}, bag_full=True)
        self.assertEqual(full.battles, [])
        self.assertEqual(story_writes(full), [])
        fought = play('RusturfTunnel', 'RusturfTunnel_EventScript_Grunt', vars={RUSTBORO: 3})
        self.assertEqual(fought.given, [PARTS])
        self.assertEqual(fought.get(RUSTBORO), 4)

    def test_mr_stone_keeps_the_letter_for_a_full_bag(self):
        meet = play('RustboroCity_DevonCorp_3F', 'RustboroCity_DevonCorp_3F_EventScript_MeetPresident', bag_full=True)
        self.assertEqual(meet.given, [])
        self.assertEqual(meet.get(RUSTBORO), 6)
        talk = 'RustboroCity_DevonCorp_3F_EventScript_MrStone'
        self.assertEqual(play('RustboroCity_DevonCorp_3F', talk, bag_full=True).given, [])
        self.assertEqual(play('RustboroCity_DevonCorp_3F', talk).given, [LETTER])
        self.assertEqual(play('RustboroCity_DevonCorp_3F', talk, items=[LETTER]).given, [])
        self.assertEqual(play('RustboroCity_DevonCorp_3F', talk, items=[LETTER], bag_full=True).given, [])

    def test_mr_stone_recalls_the_woods_only_after_the_woods(self):
        meet = 'RustboroCity_DevonCorp_3F_EventScript_MeetPresident'
        self.assertIn('RustboroCity_DevonCorp_3F_Text_MrStoneSavedStaffOnce',
                      play('RustboroCity_DevonCorp_3F', meet).messages)
        self.assertIn('RustboroCity_DevonCorp_3F_Text_MrStoneSavedStaffTwice',
                      play('RustboroCity_DevonCorp_3F', meet, vars={WOODS: 1}).messages)


class Deliveries(unittest.TestCase):
    STEVEN = 'GraniteCave_StevensRoom_EventScript_Steven'
    MUSEUM_STERN = 'SlateportCity_OceanicMuseum_2F_EventScript_CaptStern'

    def test_steven_takes_nothing_from_empty_hands(self):
        met = play('GraniteCave_StevensRoom', self.STEVEN)
        self.assertEqual(story_writes(met), [])
        self.assertEqual(met.given, [])
        self.assertEqual(met.messages, ['GraniteCave_StevensRoom_Text_MyNameIsSteven', 'GraniteCave_StevensRoom_Text_Stones'])

    def test_steven_waits_for_room_and_pays_once(self):
        full = play('GraniteCave_StevensRoom', self.STEVEN, items=[LETTER], bag_full=True)
        self.assertEqual(story_writes(full), [])
        self.assertEqual(full.given, [])
        paid = play('GraniteCave_StevensRoom', self.STEVEN, items=[LETTER])
        self.assertIn(DELIVERED_LETTER, paid.flags)
        self.assertIn('FLAG_REGISTERED_STEVEN_POKENAV', paid.flags)
        self.assertEqual(paid.given, ['ITEM_TM_STEEL_WING'])

    def test_stern_takes_nothing_from_empty_hands(self):
        met = play('SlateportCity_OceanicMuseum_2F', self.MUSEUM_STERN)
        self.assertEqual(story_writes(met), [])
        self.assertEqual(met.battles, [])
        self.assertEqual(met.messages, ['SlateportCity_OceanicMuseum_2F_Text_ImStern'])

    def test_the_museum_delivery_keeps_its_confrontation(self):
        delivered = play('SlateportCity_OceanicMuseum_2F', self.MUSEUM_STERN, items=[PARTS])
        self.assertEqual(len(delivered.battles), 2)
        self.assertEqual(delivered.get(OUTSIDE_MUSEUM), 1)
        self.assertLessEqual(DELIVERY_FLAGS, delivered.flags)

    def test_a_delivery_after_wattson_counts_scott_without_replaying_his_door_scene(self):
        delivered = play('SlateportCity_OceanicMuseum_2F', self.MUSEUM_STERN, items=[PARTS],
                         vars={OUTSIDE_MUSEUM: 3, SCOTT: 2})
        self.assertEqual(delivered.get(OUTSIDE_MUSEUM), 3)
        self.assertEqual(delivered.get(SCOTT), 3)
        self.assertIn('FLAG_ENABLE_SCOTT_MATCH_CALL', delivered.flags)

    def test_either_delivery_can_come_first(self):
        for order in permutations(('letter', 'parts')):
            flags = set()
            for step in order:
                if step == 'letter':
                    done = play('GraniteCave_StevensRoom', self.STEVEN, items=[LETTER], flags=flags)
                else:
                    done = play('SlateportCity_OceanicMuseum_2F', self.MUSEUM_STERN, items=[PARTS], flags=flags)
                flags = done.flags
            self.assertLessEqual({DELIVERED_LETTER, DELIVERED_GOODS}, flags)

    def test_archie_recalls_meteor_falls_only_after_it(self):
        fresh = play('SlateportCity_OceanicMuseum_2F', self.MUSEUM_STERN, items=[PARTS])
        self.assertIn('SlateportCity_OceanicMuseum_2F_Text_ArchieIntroduction', fresh.messages)
        recalled = play('SlateportCity_OceanicMuseum_2F', self.MUSEUM_STERN, items=[PARTS],
                        flags=['FLAG_MET_ARCHIE_METEOR_FALLS'])
        self.assertIn('SlateportCity_OceanicMuseum_2F_Text_ArchieRecallsMeteorFalls', recalled.messages)
        self.assertNotIn('SlateportCity_OceanicMuseum_2F_Text_ArchieIntroduction', recalled.messages)

    def test_the_familiar_grunt_recognizes_only_the_player_he_fought(self):
        talk = 'SlateportCity_OceanicMuseum_1F_EventScript_FamiliarGrunt'
        stranger = play('SlateportCity_OceanicMuseum_1F', talk)
        self.assertEqual(stranger.given, [])
        self.assertEqual(story_writes(stranger), [])
        thumped = play('SlateportCity_OceanicMuseum_1F', talk, flags=['FLAG_RECOVERED_DEVON_GOODS'])
        self.assertEqual(thumped.given, ['ITEM_TM_THIEF'])

    def test_briney_names_only_the_deliveries_still_to_make(self):
        talk = 'Route104_MrBrineysHouse_EventScript_Briney'
        intro = 'FLAG_MR_BRINEY_SAILING_INTRO'
        both = 'Route104_MrBrineysHouse_Text_NeedToMakeDeliveriesSailToDewford'
        letter = 'Route104_MrBrineysHouse_Text_NeedToDeliverLetterSailToDewford'
        package = 'Route104_MrBrineysHouse_Text_NeedToDeliverPackageSailToDewford'
        self.assertEqual(play('Route104_MrBrineysHouse', talk, flags=[intro], answers=0).messages[0], both)
        self.assertEqual(play('Route104_MrBrineysHouse', talk, flags=[intro, DELIVERED_GOODS], answers=0).messages[0], letter)
        self.assertEqual(play('Route104_MrBrineysHouse', talk, flags=[intro, DELIVERED_LETTER], answers=0).messages[0], package)

    def test_brineys_first_talk_names_only_the_deliveries_still_to_make(self):
        talk = 'Route104_MrBrineysHouse_EventScript_Briney'
        both = 'Route104_MrBrineysHouse_Text_LetterAndPackage'
        letter = 'Route104_MrBrineysHouse_Text_LetterForDewford'
        package = 'Route104_MrBrineysHouse_Text_PackageForSlateport'
        for flags, named in (((), {both}), ((DELIVERED_GOODS,), {letter}), ((DELIVERED_LETTER,), {package}),
                             ((DELIVERED_GOODS, DELIVERED_LETTER), set())):
            talked = play('Route104_MrBrineysHouse', talk, flags=flags, answers=0)
            self.assertEqual({both, letter, package} & set(talked.messages), named)
            self.assertIn('Route104_MrBrineysHouse_Text_ItsYouLetsSailToDewford', talked.messages)

    def test_briney_offers_slateport_from_dewford_once_it_is_an_errand(self):
        sail = 'DewfordTown_EventScript_Briney'
        prompt = ['DewfordTown_Text_SetSailBackToPetalburg']
        self.assertEqual(play('DewfordTown', sail, answers=0).messages[:1], prompt)
        self.assertNotEqual(play('DewfordTown', sail, items=[PARTS]).messages[:1], prompt)
        self.assertNotEqual(play('DewfordTown', sail, flags=[DELIVERED_LETTER]).messages[:1], prompt)


class Dock(unittest.TestCase):
    TALK = 'SlateportCity_SternsShipyard_1F_EventScript_Dock'

    def test_every_branch_sends_the_queue_away(self):
        for flags in ((), ('FLAG_BADGE07_GET',), ('FLAG_SYS_GAME_CLEAR',), (DELIVERED_GOODS,),
                      ('FLAG_DOCK_REJECTED_DEVON_GOODS',)):
            for items in ((), (PARTS,)):
                talked = play('SlateportCity_SternsShipyard_1F', self.TALK, flags=flags, items=items)
                self.assertIn(QUEUE, talked.flags)

    def test_he_mentions_the_goods_only_to_their_holder(self):
        empty = play('SlateportCity_SternsShipyard_1F', self.TALK)
        self.assertIn('SlateportCity_SternsShipyard_1F_Text_MeetDock', empty.messages)
        self.assertNotIn('SlateportCity_SternsShipyard_1F_Text_MeetDockDeliverToStern', empty.messages)
        self.assertNotIn('FLAG_DOCK_REJECTED_DEVON_GOODS', empty.flags)
        held = play('SlateportCity_SternsShipyard_1F', self.TALK, items=[PARTS])
        self.assertIn('SlateportCity_SternsShipyard_1F_Text_MeetDockDeliverToStern', held.messages)
        again = play('SlateportCity_SternsShipyard_1F', self.TALK, flags=['FLAG_DOCK_REJECTED_DEVON_GOODS'])
        self.assertNotIn('SlateportCity_SternsShipyard_1F_Text_CouldYouFindStern', again.messages)

    def test_the_mind_badge_line_and_the_shipyard_briney_are_gone(self):
        self.assertNotIn('Briney', ''.join(str(event.get('script')) for event in
                                           map_events('SlateportCity_SternsShipyard_1F')['object_events']))
        badge = play('SlateportCity_SternsShipyard_1F', self.TALK, flags=['FLAG_BADGE07_GET'], items=[PARTS])
        self.assertIn('SlateportCity_SternsShipyard_1F_Text_MeetDockDeliverToStern', badge.messages)

    def test_the_woman_outside_goes_by_the_queue(self):
        talk = 'SlateportCity_EventScript_Woman1'
        self.assertEqual(play('SlateportCity', talk).messages, ['SlateportCity_Text_WhatsLongLineOverThere'])
        self.assertEqual(play('SlateportCity', talk, flags=[QUEUE]).messages, ['SlateportCity_Text_VisitedMuseumOften'])


class SternAndHarbor(unittest.TestCase):
    TALK = 'SlateportCity_Harbor_EventScript_CaptStern'
    SCENE = 'SlateportCity_Harbor_EventScript_AquaEscapeTrigger0'

    def harbor(self, *, items=(), flags=(), state=2, **vars):
        return play('SlateportCity_Harbor', self.TALK, items=items, flags=flags, vars={HARBOR: state, **vars})

    def test_the_museum_stern_leaves_when_magma_hideout_is_done(self):
        transition = 'SlateportCity_OceanicMuseum_2F_OnTransition'
        self.assertNotIn(MUSEUM_STERN, play('SlateportCity_OceanicMuseum_2F', transition).flags)
        for state in (1, 2):
            entered = play('SlateportCity_OceanicMuseum_2F', transition, vars={HARBOR: state})
            self.assertIn(MUSEUM_STERN, entered.flags)

    def test_a_late_delivery_sets_what_the_museum_sets_with_no_confrontation(self):
        late = self.harbor(items=[PARTS])
        self.assertEqual(late.battles, [])
        self.assertLessEqual(DELIVERY_FLAGS, late.flags)
        self.assertIn(MUSEUM_STERN, late.flags)
        self.assertIn(('FLAG_HIDE_ROUTE_116_DEVON_EMPLOYEE', 0, 'flag'), late.writes)
        self.assertEqual(late.get('VAR_REGISTER_BIRCH_STATE'), 1)
        self.assertEqual(late.get(HARBOR), 2)
        self.assertIn('SlateportCity_Harbor_Text_SternTakesParts', late.messages)

    def test_a_late_delivery_counts_scotts_meeting_without_waiting_on_it(self):
        late = self.harbor(items=[PARTS], VAR_SCOTT_STATE=3)
        self.assertEqual(late.get(SCOTT), 4)
        self.assertEqual(late.get(OUTSIDE_MUSEUM), 2)
        self.assertIn('FLAG_ENABLE_SCOTT_MATCH_CALL', late.flags)

    def test_a_late_delivery_leaves_a_battle_tent_scene_already_closed(self):
        self.assertEqual(self.harbor(items=[PARTS], VAR_SLATEPORT_OUTSIDE_MUSEUM_STATE=3).get(OUTSIDE_MUSEUM), 3)

    def test_a_pending_scene_holds_the_parts(self):
        waiting = self.harbor(items=[PARTS], state=1)
        self.assertEqual(story_writes(waiting), [])
        self.assertEqual(waiting.messages, ['SlateportCity_Harbor_Text_ThoseThugsAtTheHarbor'])

    def test_stern_recalls_the_museum_only_after_it(self):
        recalled = self.harbor(state=1, flags=[DELIVERED_GOODS])
        self.assertEqual(recalled.messages, ['SlateportCity_Harbor_Text_SameThugsTriedToRobAtMuseum'])

    def test_his_lines_follow_aquas_escape_and_not_the_badge(self):
        def line(flags):
            return self.harbor(flags=flags).messages[0]
        badge = 'FLAG_BADGE07_GET'
        escaped = 'FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE'
        spoke = 'FLAG_EVIL_TEAM_ESCAPED_STERN_SPOKE'
        steal = 'SlateportCity_Harbor_Text_CaptSternWhyStealMySubmarine'
        self.assertEqual(line([]), steal)
        self.assertEqual(line([badge]), steal)
        self.assertEqual(line([escaped]), 'SlateportCity_Harbor_Text_TeamAquaLeftNeedDive')
        self.assertEqual(line([escaped, badge]), 'SlateportCity_Harbor_Text_TeamAquaLeftNeedDive')
        self.assertEqual(line([escaped, spoke]), 'SlateportCity_Harbor_Text_NeedDiveToCatchSub')
        self.assertEqual(line([escaped, spoke, badge]), 'SlateportCity_Harbor_Text_WontBeLongBeforeWeFinishFerry')
        # Steven's Dive came first: he sends nobody to Mossdeep for what they already have.
        for flags in ([escaped, badge, 'FLAG_RECEIVED_HM_DIVE'], [badge, 'FLAG_RECEIVED_HM_DIVE']):
            self.assertEqual(line(flags), 'SlateportCity_Harbor_Text_WontBeLongBeforeWeFinishFerry')

    def test_the_scanner_trade_does_not_wait_for_aquas_escape(self):
        dive = self.harbor(items=['ITEM_SCANNER'], flags=['FLAG_BADGE07_GET', 'FLAG_RECEIVED_HM_DIVE'])
        self.assertNotIn('SlateportCity_Harbor_Text_CaptSternWhyStealMySubmarine', dive.messages)

    def test_the_scene_reopens_the_harbor_and_clears_the_queue(self):
        played = play('SlateportCity_Harbor', self.SCENE, vars={HARBOR: 1, STREET: 2}, flags=[PATRONS])
        self.assertEqual(played.get(HARBOR), 2)
        self.assertNotIn(PATRONS, played.flags)
        self.assertIn(QUEUE, played.flags)

    def test_the_scene_keeps_the_parts_and_their_delivery_open(self):
        played = play('SlateportCity_Harbor', self.SCENE, vars={HARBOR: 1, STREET: 2})
        self.assertNotIn(DELIVERED_GOODS, played.flags)
        self.assertEqual(self.harbor(items=[PARTS], flags=played.flags).get(SCOTT), 1)

    def test_a_ferry_passenger_ahead_of_stern_keeps_control(self):
        for trigger in range(4):
            early = play('SlateportCity_Harbor', f'SlateportCity_Harbor_EventScript_AquaEscapeTrigger{trigger}',
                         vars={HARBOR: 1, STREET: 1})
            self.assertFalse(early.locked)
            self.assertEqual(early.get(HARBOR), 1)
            self.assertEqual(story_writes(early), [])

    def test_the_street_stern_greets_by_what_they_share(self):
        scene = 'SlateportCity_EventScript_CaptStern'
        self.assertIn('SlateportCity_Text_OhYoureThatTrainer', play('SlateportCity', scene).messages)
        known = play('SlateportCity', scene, flags=[DELIVERED_GOODS]).messages
        self.assertIn('SlateportCity_Text_OhPlayerLookingGreat', known)
        self.assertNotIn('SlateportCity_Text_OhYoureThatTrainer', known)


if __name__ == '__main__':
    unittest.main()
