WayfarerCeruleanBurglary_EventScript_Grunt::
	goto_if_set FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS, WayfarerCeruleanBurglary_EventScript_End
	goto_if_defeated TRAINER_CERULEAN_BURGLARY_GRUNT_HNS, WayfarerCeruleanBurglary_EventScript_Reward
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerCeruleanBurglary_EventScript_NoParty
	lock
	faceplayer
	msgbox WayfarerCeruleanBurglary_Text_Intro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_CERULEAN_BURGLARY_GRUNT_HNS, WayfarerCeruleanBurglary_Text_Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeruleanBurglary_EventScript_End
WayfarerCeruleanBurglary_EventScript_Reward::
	lock
	faceplayer
	msgbox WayfarerCeruleanBurglary_Text_ReturnTM, MSGBOX_DEFAULT
	checkitemspace ITEM_TM_DIG
	goto_if_eq VAR_RESULT, FALSE, WayfarerCeruleanBurglary_EventScript_NoRoom
	additem ITEM_TM_DIG
	goto_if_eq VAR_RESULT, FALSE, WayfarerCeruleanBurglary_EventScript_NoRoom
	checkitem ITEM_TM_DIG
	goto_if_eq VAR_RESULT, FALSE, WayfarerCeruleanBurglary_EventScript_NoRoom
	msgreceiveditem WayfarerCeruleanBurglary_Text_ReceivedTM, ITEM_TM_DIG, 1
WayfarerCeruleanBurglary_EventScript_Complete::
	setflag FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS
	msgbox WayfarerCeruleanBurglary_Text_Departure, MSGBOX_DEFAULT
	closemessage
	removeobject LOCALID_CERULEAN_BURGLARY_GRUNT
WayfarerCeruleanBurglary_EventScript_End::
	releaseall
	end

WayfarerCeruleanBurglary_EventScript_NoRoom::
	msgbox WayfarerCeruleanBurglary_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

WayfarerCeruleanBurglary_EventScript_NoParty::
	msgbox WayfarerCeruleanBurglary_Text_NoParty, MSGBOX_NPC
	end

WayfarerCeruleanBurglary_EventScript_Man::
	goto_if_set FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS, WayfarerCeruleanBurglary_EventScript_ManRecovered
	msgbox WayfarerCeruleanBurglary_Text_ManRobbed, MSGBOX_NPC
	end

WayfarerCeruleanBurglary_EventScript_ManRecovered::
	msgbox WayfarerCeruleanBurglary_Text_ManRecovered, MSGBOX_NPC
	end

WayfarerCeruleanBurglary_EventScript_Woman::
	goto_if_set FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS, WayfarerCeruleanBurglary_EventScript_WomanRecovered
	msgbox WayfarerCeruleanBurglary_Text_WomanRobbed, MSGBOX_NPC
	end

WayfarerCeruleanBurglary_EventScript_WomanRecovered::
	msgbox WayfarerCeruleanBurglary_Text_WomanRecovered, MSGBOX_NPC
	end

WayfarerCeruleanBurglary_Text_Intro::
	.string "Hey! What are you looking at?\p"
	.string "Me? I'm an innocent bystander!\n"
	.string "Don't you believe me?{PLAY_BGM}{MUS_RG_ENCOUNTER_ROCKET}$"
WayfarerCeruleanBurglary_Text_Defeat::
	.string "GRUNT: Stop! I give up!\n"
	.string "I'll leave quietly!$"
WayfarerCeruleanBurglary_Text_ReturnTM::
	.string "…Okay.\nI'll return the TM I stole.$"
WayfarerCeruleanBurglary_Text_ReceivedTM::
	.string "{PLAYER} recovered TM28 from\n"
	.string "the TEAM ROCKET GRUNT.$"
WayfarerCeruleanBurglary_Text_Departure::
	.string "I better get moving!\nBye!$"
WayfarerCeruleanBurglary_Text_NoRoom::
	.string "Make room in your TM/HM pocket!\n"
	.string "I can't run until I give it to you!$"
WayfarerCeruleanBurglary_Text_NoParty::
	.string "Come back when your POKéMON can\n"
	.string "put up a fight!$"
WayfarerCeruleanBurglary_Text_ManRobbed::
	.string "TEAM ROCKET stole my TM for DIG!\p"
	.string "I was going to teach it to DIGLETT.\p"
	.string "That shady fellow is still outside,\n"
	.string "just east of our front door!$"
WayfarerCeruleanBurglary_Text_ManRecovered::
	.string "You recovered the TM for DIG?\n"
	.string "Thank you! You can keep it.\p"
	.string "DIGLETT learned DIG on its own.$"
WayfarerCeruleanBurglary_Text_WomanRobbed::
	.string "We were held up by TEAM ROCKET!\p"
	.string "They took our TM for DIG.\n"
	.string "Please be careful outside.$"
WayfarerCeruleanBurglary_Text_WomanRecovered::
	.string "You stopped that thief!\n"
	.string "Thank you for helping us.\p"
	.string "Keep the TM for DIG. I hope it\n"
	.string "helps you on your travels.$"

WayfarerCeruleanBurglary_EventScript_RearEntry::
	lockall
	msgbox WayfarerCeruleanBurglary_Text_RearEntry, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	warp MAP_CERULEAN_CITY_HOUSE2, 4, 2
	waitstate
	releaseall
	end

WayfarerCeruleanBurglary_Text_RearEntry::
	.string "TEAM ROCKET left a hole in the wall!\n"
	.string "You can enter the house here.$"
