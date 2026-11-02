#include "constants/wayfarer_johto_story.h"

VictoryRoadKanto_1F_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VictoryRoadKanto_1F_OnTransition
	.byte 0

VictoryRoadKanto_1F_OnTransition::
	setvar VAR_0x8004, SILVER_CHAPTER_VICTORY_ROAD
	specialvar VAR_RESULT, WayfarerJohtoSilverAvailable
	call_if_eq VAR_RESULT, TRUE, VictoryRoadKanto_1F_EventScript_ShowSilver
	call_if_eq VAR_RESULT, FALSE, VictoryRoadKanto_1F_EventScript_HideSilver
	end

VictoryRoadKanto_1F_EventScript_ShowSilver::
	clearflag FLAG_HIDE_VICTORY_ROAD_SILVER
	return

VictoryRoadKanto_1F_EventScript_HideSilver::
	setflag FLAG_HIDE_VICTORY_ROAD_SILVER
	return

VictoryRoadKanto_1F_EventScript_Item_RareCandy::
	finditem ITEM_RARE_CANDY
	end
VictoryRoadKanto_1F_EventScript_Item_MaxEther::
	finditem ITEM_MAX_ETHER
	end
VictoryRoadKanto_EventScript_Item_Dark_Pulse::
    finditem ITEM_TM_DARK_PULSE
    end

@ Silver stands in the alcove at (26,6), off the corridor, while his chapter 10 battle is
@ playable. Talking to him offers the battle; declining or losing leaves the corridor open.
VictoryRoadKanto_1F_EventScript_Silver::
	setvar VAR_0x8004, SILVER_CHAPTER_VICTORY_ROAD
	specialvar VAR_RESULT, WayfarerJohtoSilverAvailable
	goto_if_eq VAR_RESULT, FALSE, VictoryRoadKanto_1F_EventScript_SilverInert
	lock
	faceplayer
	call_if_unset FLAG_WAYFARER_JOHTO_INDIGO_ENTERED, VictoryRoadKanto_1F_EventScript_RivalLeagueQuestion
	msgbox VictoryRoad_Text_RivalBefore, MSGBOX_DEFAULT
	msgbox VictoryRoad_Text_RivalChallenge, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VictoryRoadKanto_1F_EventScript_SilverDeclined
	playbgm MUS_HG_ENCOUNTER_RIVAL, FALSE
	setvar_forward VAR_WAYFARER_JOHTO_SILVER_CHAPTER, SILVER_CHAPTER_VICTORY_ROAD
	call_if_eq VAR_STARTER_MON, 0, VictoryRoadKanto_1F_Silver_Battle_Cyndaquil
	call_if_eq VAR_STARTER_MON, 1, VictoryRoadKanto_1F_Silver_Battle_Totodile
	call_if_eq VAR_STARTER_MON, 2, VictoryRoadKanto_1F_Silver_Battle_Chikorita
	setflag FLAG_WAYFARER_JOHTO_SILVER_WON_10
	setflag FLAG_WAYFARER_JOHTO_SILVER_WON_ANY
	playbgm MUS_HG_RIVAL_EXIT, FALSE
	call_if_set FLAG_WAYFARER_JOHTO_SILVER_PLAYED_7, VictoryRoadKanto_1F_EventScript_RivalDragonMaster
	msgbox VictoryRoad_Text_RivalAfter, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_VICTORYROAD_SILVER
	setflag FLAG_HIDE_VICTORY_ROAD_SILVER
	fadescreen FADE_FROM_BLACK
	fadedefaultbgm
	release
	end

VictoryRoadKanto_1F_EventScript_SilverInert::
	end

VictoryRoadKanto_1F_EventScript_SilverDeclined::
	msgbox VictoryRoad_Text_RivalDeclined, MSGBOX_DEFAULT
	closemessage
	release
	end

VictoryRoadKanto_1F_EventScript_RivalLeagueQuestion::
	msgbox VictoryRoad_Text_RivalLeagueQuestion, MSGBOX_DEFAULT
	return

VictoryRoadKanto_1F_EventScript_RivalDragonMaster::
	msgbox VictoryRoad_Text_RivalDragonMaster, MSGBOX_DEFAULT
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

VictoryRoad_Text_RivalLeagueQuestion:
    .string "Hold it.\p"
    .string "…Are you going to take the POKéMON\n"
    .string "LEAGUE challenge?$"

VictoryRoad_Text_RivalBefore:
    .string "…Don't make me laugh.\p"
    .string "You're so much weaker than I am.\p"
    .string "I'm not like I was before.\p"
    .string "I now have the best and strongest\n"
    .string "POKéMON with me.\p"
    .string "I'm invincible!$"

VictoryRoad_Text_RivalChallenge:
    .string "{PLAYER}!\n"
    .string "I challenge you!$"

VictoryRoad_Text_RivalDeclined:
    .string "I'll be here when you're ready.$"

VictoryRoad_Text_RivalDefeat:
    .string "…I couldn't win…\p"
    .string "I gave it every-thing I had…\p"
    .string "What you possess, and what I lack…$"

VictoryRoad_Text_RivalDragonMaster:
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

