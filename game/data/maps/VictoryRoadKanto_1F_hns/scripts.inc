VictoryRoadKanto_1F_hns_MapScripts::
	.byte 0

VictoryRoadKanto_1F_EventScript_Item_RareCandy::
	finditem ITEM_RARE_CANDY
	end
VictoryRoadKanto_1F_EventScript_Item_MaxEther::
	finditem ITEM_MAX_ETHER
	end
VictoryRoadKanto_EventScript_Item_Dark_Pulse::
    finditem ITEM_TM_DARK_PULSE
    end

VictoryRoadKanto_1F_Trigger::
#if IS_WAYFARER
	goto_if_unset FLAG_JOHTO_STARTER_CHOICE_COMMITTED, VictoryRoadKanto_1F_EventScript_DeferredSilver
#endif
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
	specialvar VAR_0x8004, LeagueCircuit_GetRequiredRegion
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	goto_if_eq VAR_RESULT, TRUE, LeagueCircuit_EventScript_TravelAllowed
#endif
#if IS_WAYFARER
	setvar VAR_0x8004, WAYFARER_STORY_SCENE_SILVER_VICTORY_ROAD
	specialvar VAR_RESULT, WayfarerStoryCanStartScene
	goto_if_eq VAR_RESULT, WAYFARER_STORY_GATE_ALLOW, VictoryRoadKanto_1F_Trigger_Allowed
	goto_if_eq VAR_RESULT, WAYFARER_STORY_GATE_REFUSE, VictoryRoadKanto_1F_EventScript_NoParty
	goto VictoryRoadKanto_1F_EventScript_DeferredSilver
#endif
VictoryRoadKanto_1F_Trigger_Allowed::
	lock
	playbgm MUS_HG_ENCOUNTER_RIVAL, FALSE
	delay 5
	playse SE_PIN
	applymovement2 OBJ_EVENT_ID_PLAYER, Common_Movement_ExclamationMark
	waitmovement OBJ_EVENT_ID_PLAYER
	addobject LOCALID_VICTORYROAD_SILVER
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 27, VictoryRoadKanto_1F_EventScript_SilverEnterLeft
	call_if_eq VAR_TEMP_0, 28, VictoryRoadKanto_1F_EventScript_SilverEnterMid
	call_if_eq VAR_TEMP_0, 29, VictoryRoadKanto_1F_EventScript_SilverEnterRight
	msgbox VictoryRoad_Text_RivalBefore, MSGBOX_DEFAULT
	call_if_eq VAR_STARTER_MON, 0, VictoryRoadKanto_1F_Silver_Battle_Cyndaquil
	call_if_eq VAR_STARTER_MON, 1, VictoryRoadKanto_1F_Silver_Battle_Totodile
	call_if_eq VAR_STARTER_MON, 2, VictoryRoadKanto_1F_Silver_Battle_Chikorita
	playbgm MUS_HG_RIVAL_EXIT, FALSE
	msgbox VictoryRoad_Text_RivalAfter, MSGBOX_DEFAULT
	closemessage
	applymovement2 LOCALID_VICTORYROAD_SILVER, VictoryRoadKanto_1F_Movement_SilverLeave
	waitmovement 0
	removeobject LOCALID_VICTORYROAD_SILVER
	fadedefaultbgm
	release
	setvar VAR_ROUTE27_STATE, 3
	end

VictoryRoadKanto_1F_EventScript_SilverEnterLeft::
	applymovement2 LOCALID_VICTORYROAD_SILVER, VictoryRoadKanto_1F_Movement_SilverEnterLeft
	applymovement2 OBJ_EVENT_ID_PLAYER, Common_Movement_WalkDown1
	applymovement2 OBJ_EVENT_ID_FOLLOWER, Common_Movement_WalkUp1
	waitmovement LOCALID_VICTORYROAD_SILVER
	return

VictoryRoadKanto_1F_EventScript_SilverEnterMid::
	applymovement2 LOCALID_VICTORYROAD_SILVER, VictoryRoadKanto_1F_Movement_SilverEnterMid
	applymovement2 OBJ_EVENT_ID_PLAYER, Common_Movement_WalkDown1
	applymovement2 OBJ_EVENT_ID_FOLLOWER, Common_Movement_WalkUp1
	waitmovement LOCALID_VICTORYROAD_SILVER
	return

VictoryRoadKanto_1F_EventScript_SilverEnterRight::
	applymovement2 LOCALID_VICTORYROAD_SILVER, VictoryRoadKanto_1F_Movement_SilverEnterRight
	applymovement2 OBJ_EVENT_ID_PLAYER, Common_Movement_WalkDown1
	applymovement2 OBJ_EVENT_ID_FOLLOWER, Common_Movement_WalkUp1
	waitmovement LOCALID_VICTORYROAD_SILVER
	return


VictoryRoadKanto_1F_Silver_Battle_Cyndaquil::
	trainerbattle_no_intro TRAINER_RIVAL_CYNDAQUIL_5_HNS, VictoryRoad_Text_RivalDefeat
	return

VictoryRoadKanto_1F_Silver_Battle_Totodile::
	trainerbattle_no_intro TRAINER_RIVAL_TOTODILE_5_HNS, VictoryRoad_Text_RivalDefeat
	return

VictoryRoadKanto_1F_Silver_Battle_Chikorita::
	trainerbattle_no_intro TRAINER_RIVAL_CHIKORITA_5_HNS, VictoryRoad_Text_RivalDefeat
	return

VictoryRoadKanto_1F_Movement_SilverEnterLeft:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	step_end
VictoryRoadKanto_1F_Movement_SilverEnterMid:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	step_end
VictoryRoadKanto_1F_Movement_SilverEnterRight:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	step_end
VictoryRoadKanto_1F_Movement_SilverLeave:
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

VictoryRoad_Text_RivalBefore:
    .string "Hold it.\p"
    .string "…Are you going to take the POKéMON\n"
    .string "LEAGUE challenge?\p"
    .string "…Don't make me laugh.\p"
    .string "You're so much weaker than I am.\p"
    .string "I'm not like I was before.\p"
    .string "I now have the best and strongest\n"
    .string "POKéMON with me.\p"
    .string "I'm invincible!\p"
    .string "{PLAYER}!\n"
    .string "I challenge you!$"

VictoryRoad_Text_RivalDefeat:
    .string "…I couldn't win…\p"
    .string "I gave it every-thing I had…\p"
    .string "What you possess, and what I lack…\p"
    .string "I'm beginning to understand what\n"
    .string "that dragon master said to me…$"

VictoryRoad_Text_RivalAfter:
    .string "…I haven't given up on becoming the\n"
    .string "greatest trainer…\p"
    .string "I'm going to find out why I can't\n"
    .string "win and become stronger…\p"
    .string "When I do, I will challenge you.\p"
    .string "And I'll beat you down with all my\n"
    .string "power.\p"
    .string "…Humph! You keep at it until then.$"

#if IS_WAYFARER
VictoryRoadKanto_1F_EventScript_DeferredSilver::
	releaseall
	end

VictoryRoadKanto_1F_EventScript_NoParty::
	special WayfarerStoryShowSelectedNoPartyDialogue
	waitmessage
	waitbuttonpress
	releaseall
	end
#endif
