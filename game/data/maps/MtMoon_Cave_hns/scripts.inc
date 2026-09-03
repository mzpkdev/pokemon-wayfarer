MtMoon_Cave_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MtMoon_Cave_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	.byte 0

MtMoon_Cave_OnTransition: 
	callnative SetTimeBasedEncounters
	goto_if_ne VAR_PEWTER_CITY_STATE, 0, MtMoon_Cave_EventScript_EndTransition
	setvar VAR_PEWTER_CITY_STATE, 1
MtMoon_Cave_EventScript_EndTransition::
	end

MtMoon_Cave_EventScript_Sign::
	msgbox MtMoon_Cave_Text_Sign, MSGBOX_SIGN
	end

MtMoon_Cave_EventScript_Silver::
	lock
	faceplayer
	msgbox MtMoon_Cave_Text_Rival_Before, MSGBOX_DEFAULT
	msgbox MtMoon_Cave_Text_Rival_BattleOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MtMoon_Cave_EventScript_SilverDeclined
	playbgm MUS_HG_ENCOUNTER_RIVAL, FALSE
	goto_if_eq VAR_STARTER_MON, 0, MtMoon_Cave_EventScript_Silver_Battle_Cyndaquil
	goto_if_eq VAR_STARTER_MON, 1, MtMoon_Cave_EventScript_Silver_Battle_Totodile
	goto_if_eq VAR_STARTER_MON, 2, MtMoon_Cave_EventScript_Silver_Battle_Chikorita
	release
	end

MtMoon_Cave_EventScript_SilverDeclined::
	msgbox MtMoon_Cave_Text_Rival_Declined, MSGBOX_DEFAULT
	release
	end

MtMoon_Cave_EventScript_Silver_Battle_Cyndaquil::
	trainerbattle_no_intro TRAINER_RIVAL_CYNDAQUIL_6_HNS, MtMoon_Cave_Text_Rival_Win
	goto MtMoon_Cave_EventScript_SilverAfterBattle

MtMoon_Cave_EventScript_Silver_Battle_Totodile::
	trainerbattle_no_intro TRAINER_RIVAL_TOTODILE_6_HNS, MtMoon_Cave_Text_Rival_Win
	goto MtMoon_Cave_EventScript_SilverAfterBattle

MtMoon_Cave_EventScript_Silver_Battle_Chikorita::
	trainerbattle_no_intro TRAINER_RIVAL_CHIKORITA_6_HNS, MtMoon_Cave_Text_Rival_Win
	goto MtMoon_Cave_EventScript_SilverAfterBattle

MtMoon_Cave_EventScript_SilverAfterBattle::
	playbgm MUS_HG_RIVAL_EXIT, FALSE
	msgbox MtMoon_Cave_Text_Rival_After, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_MTMOON_SILVER
	setflag FLAG_HIDE_MTMOON_SILVER
	clearflag FLAG_HIDE_INDIGO_PLATEAU_SILVER
	setvar VAR_PEWTER_CITY_STATE, 2
	fadescreen FADE_FROM_BLACK
	fadedefaultbgm
	release
	end

MtMoon_Cave_Text_Rival_Before:
	.string "..................................\p"
	.string "It's been a while, {PLAYER}.\p"
	.string "…Since I lost to you, I thought about\n"
	.string "what I was lacking with my POKéMON…\p"
	.string "And we came up with an answer.\p"
	.string "{PLAYER}, now we'll show you!$"

MtMoon_Cave_Text_Rival_BattleOffer:
	.string "Will you battle me?$"

MtMoon_Cave_Text_Rival_Declined:
	.string "I'll be here when you're ready.$"

MtMoon_Cave_Text_Rival_Win:
	.string "..................................\p"
	.string "I thought I raised my POKéMON to be the\n"
	.string "best they could be…\p"
	.string "…But it still wasn't enough…$"

MtMoon_Cave_Text_Rival_After:
	.string "..................................\p"
	.string "…You won, fair and square.\p"
	.string "I admit it. But this isn't the end.\p"
	.string "I'm going to be the greatest POKéMON\n"
	.string "trainer ever.\p"
	.string "Because these guys are behind me.\p"
	.string "…Listen, {PLAYER}.\p"
	.string "One of these days I'm going to prove\n"
	.string "how good I am by beating you.$"

MtMoon_Cave_Text_Rival_Loss:
	.string "..................................\p"
	.string "I've repaid my debt to you.\p"
	.string "With my POKéMON, I'm going to beat the\n"
	.string "CHAMPION and become the world's greatest\n"
	.string "trainer.$"

MtMoon_Cave_Text_Sign:
	.string "MT MOON SQUARE ->$"
