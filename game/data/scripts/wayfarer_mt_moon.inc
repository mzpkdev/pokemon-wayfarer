@ Wayfarer-only FRLG fossil adventure in the selected HNS Mt. Moon cave.

WayfarerMtMoon_OnTransition::
	goto_if_set FLAG_GOT_FOSSIL_FROM_MT_MOON_HNS, WayfarerMtMoon_HideFossils
	goto_if_defeated TRAINER_MT_MOON_MIGUEL_HNS, WayfarerMtMoon_ShowFossils
	goto WayfarerMtMoon_HideFossils

WayfarerMtMoon_ShowFossils::
	clearflag FLAG_HIDE_MT_MOON_DOME_FOSSIL_HNS
	clearflag FLAG_HIDE_MT_MOON_HELIX_FOSSIL_HNS
	addobject LOCALID_MTMOON_DOME_FOSSIL
	addobject LOCALID_MTMOON_HELIX_FOSSIL
	return

WayfarerMtMoon_HideFossils::
	setflag FLAG_HIDE_MT_MOON_DOME_FOSSIL_HNS
	setflag FLAG_HIDE_MT_MOON_HELIX_FOSSIL_HNS
	removeobject LOCALID_MTMOON_DOME_FOSSIL
	removeobject LOCALID_MTMOON_HELIX_FOSSIL
	return

WayfarerMtMoon_EventScript_Grunt1::
	goto_if_defeated TRAINER_MT_MOON_ROCKET_GRUNT_1_HNS, WayfarerMtMoon_EventScript_Grunt1After
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoParty
WayfarerMtMoon_EventScript_Grunt1TrainerBattle::
	trainerbattle_single TRAINER_MT_MOON_ROCKET_GRUNT_1_HNS, WayfarerMtMoon_Text_Grunt1Intro, WayfarerMtMoon_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerMtMoon_EventScript_EndAfterBattle
WayfarerMtMoon_EventScript_Grunt1After::
	msgbox WayfarerMtMoon_Text_Grunt1After, MSGBOX_AUTOCLOSE
	end

WayfarerMtMoon_EventScript_Grunt2::
	goto_if_defeated TRAINER_MT_MOON_ROCKET_GRUNT_2_HNS, WayfarerMtMoon_EventScript_Grunt2After
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoParty
WayfarerMtMoon_EventScript_Grunt2TrainerBattle::
	trainerbattle_single TRAINER_MT_MOON_ROCKET_GRUNT_2_HNS, WayfarerMtMoon_Text_Grunt2Intro, WayfarerMtMoon_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerMtMoon_EventScript_EndAfterBattle
WayfarerMtMoon_EventScript_Grunt2After::
	msgbox WayfarerMtMoon_Text_Grunt2After, MSGBOX_AUTOCLOSE
	end

WayfarerMtMoon_EventScript_Grunt3::
	goto_if_defeated TRAINER_MT_MOON_ROCKET_GRUNT_3_HNS, WayfarerMtMoon_EventScript_Grunt3After
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoParty
WayfarerMtMoon_EventScript_Grunt3TrainerBattle::
	trainerbattle_single TRAINER_MT_MOON_ROCKET_GRUNT_3_HNS, WayfarerMtMoon_Text_Grunt3Intro, WayfarerMtMoon_Text_Grunt3Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerMtMoon_EventScript_EndAfterBattle
WayfarerMtMoon_EventScript_Grunt3After::
	msgbox WayfarerMtMoon_Text_Grunt3After, MSGBOX_AUTOCLOSE
	end

WayfarerMtMoon_EventScript_Grunt4::
	goto_if_defeated TRAINER_MT_MOON_ROCKET_GRUNT_4_HNS, WayfarerMtMoon_EventScript_Grunt4After
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoParty
WayfarerMtMoon_EventScript_Grunt4TrainerBattle::
	trainerbattle_single TRAINER_MT_MOON_ROCKET_GRUNT_4_HNS, WayfarerMtMoon_Text_Grunt4Intro, WayfarerMtMoon_Text_Grunt4Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerMtMoon_EventScript_EndAfterBattle
WayfarerMtMoon_EventScript_Grunt4After::
	msgbox WayfarerMtMoon_Text_Grunt4After, MSGBOX_AUTOCLOSE
	end

WayfarerMtMoon_EventScript_NoParty::
	msgbox WayfarerMtMoon_Text_NoUsableParty, MSGBOX_DEFAULT
	end

WayfarerMtMoon_EventScript_EndAfterBattle::
	releaseall
	end

WayfarerMtMoon_EventScript_Miguel::
	lock
	faceplayer
	goto_if_set FLAG_GOT_FOSSIL_FROM_MT_MOON_HNS, WayfarerMtMoon_EventScript_MiguelComplete
	goto_if_defeated TRAINER_MT_MOON_MIGUEL_HNS, WayfarerMtMoon_EventScript_MiguelChoose
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_MiguelNoParty
	msgbox WayfarerMtMoon_Text_MiguelIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_MT_MOON_MIGUEL_HNS, WayfarerMtMoon_Text_MiguelDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerMtMoon_EventScript_EndAfterBattle
	call WayfarerMtMoon_ShowFossils
WayfarerMtMoon_EventScript_MiguelChoose::
	msgbox WayfarerMtMoon_Text_MiguelChoose, MSGBOX_DEFAULT
	release
	end

WayfarerMtMoon_EventScript_MiguelNoParty::
	msgbox WayfarerMtMoon_Text_MiguelNoParty, MSGBOX_DEFAULT
	release
	end

WayfarerMtMoon_EventScript_MiguelComplete::
	msgbox WayfarerMtMoon_Text_MiguelComplete, MSGBOX_DEFAULT
	release
	end

WayfarerMtMoon_EventScript_DomeFossil::
	lock
	faceplayer
	goto_if_set FLAG_GOT_FOSSIL_FROM_MT_MOON_HNS, WayfarerMtMoon_EventScript_FossilUnavailable
	goto_if_not_defeated TRAINER_MT_MOON_MIGUEL_HNS, WayfarerMtMoon_EventScript_FossilUnavailable
	msgbox WayfarerMtMoon_Text_FossilChoice, MSGBOX_DEFAULT
	multichoice 20, 8, MULTI_MT_MOON_FOSSIL_WAYFARER, FALSE
	switch VAR_RESULT
	case 0, WayfarerMtMoon_EventScript_ChooseHelix
	case 1, WayfarerMtMoon_EventScript_ChooseDome
	case 2, WayfarerMtMoon_EventScript_FossilUnavailable
	case MULTI_B_PRESSED, WayfarerMtMoon_EventScript_FossilUnavailable
	end

WayfarerMtMoon_EventScript_HelixFossil::
	goto WayfarerMtMoon_EventScript_DomeFossil

WayfarerMtMoon_EventScript_ChooseHelix::
	checkitem ITEM_HELIX_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, WayfarerMtMoon_EventScript_CommitHelix
	checkitemspace ITEM_HELIX_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoRoom
	additem ITEM_HELIX_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoRoom
	msgreceiveditem WayfarerMtMoon_Text_ReceivedHelix, ITEM_HELIX_FOSSIL, 1, MUS_RG_OBTAIN_KEY_ITEM
	goto WayfarerMtMoon_EventScript_CommitHelix

WayfarerMtMoon_EventScript_ChooseDome::
	checkitem ITEM_DOME_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, WayfarerMtMoon_EventScript_CommitDome
	checkitemspace ITEM_DOME_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoRoom
	additem ITEM_DOME_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, WayfarerMtMoon_EventScript_NoRoom
	msgreceiveditem WayfarerMtMoon_Text_ReceivedDome, ITEM_DOME_FOSSIL, 1, MUS_RG_OBTAIN_KEY_ITEM
	goto WayfarerMtMoon_EventScript_CommitDome

WayfarerMtMoon_EventScript_CommitHelix::
	msgbox WayfarerMtMoon_Text_MiguelTakesDome, MSGBOX_DEFAULT
	goto WayfarerMtMoon_EventScript_CommitChoice

WayfarerMtMoon_EventScript_CommitDome::
	msgbox WayfarerMtMoon_Text_MiguelTakesHelix, MSGBOX_DEFAULT

WayfarerMtMoon_EventScript_CommitChoice::
	setflag FLAG_GOT_FOSSIL_FROM_MT_MOON_HNS
	call WayfarerMtMoon_HideFossils
	release
	end

WayfarerMtMoon_EventScript_NoRoom::
	msgbox WayfarerMtMoon_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

WayfarerMtMoon_EventScript_FossilUnavailable::
	release
	end

WayfarerMtMoon_Text_NoUsableParty::
	.string "Your POKéMON need to be ready\nto battle.$"
WayfarerMtMoon_Text_MiguelNoParty::
	.string "Come back when your POKéMON are\nready to battle.$"
WayfarerMtMoon_Text_MiguelIntro::
	.string "Hey, stop!\pI found these fossils!\nThey're both mine!$"
WayfarerMtMoon_Text_MiguelDefeat::
	.string "Okay!\nI'll share!$"
WayfarerMtMoon_Text_MiguelChoose::
	.string "We'll each take a fossil!\nNo being greedy!$"
WayfarerMtMoon_Text_MiguelComplete::
	.string "That was a fair choice. Take good\ncare of that fossil.$"
WayfarerMtMoon_Text_Grunt1Intro::
	.string "We, TEAM ROCKET, shall find the\n"
	.string "fossils!\p"
	.string "Reviving POKéMON from them will\n"
	.string "earn us huge riches!$"
WayfarerMtMoon_Text_Grunt1Defeat::
	.string "Urgh!\n"
	.string "Now I'm mad!$"
WayfarerMtMoon_Text_Grunt1After::
	.string "You made me mad!\n"
	.string "TEAM ROCKET will blacklist you!$"
WayfarerMtMoon_Text_Grunt2Intro::
	.string "We, TEAM ROCKET, are POKéMON\n"
	.string "gangsters!\l"
	.string "We strike fear with our strength!$"
WayfarerMtMoon_Text_Grunt2Defeat::
	.string "I blew it!$"
WayfarerMtMoon_Text_Grunt2After::
	.string "Darn it all!\n"
	.string "My associates won't stand for this!$"
WayfarerMtMoon_Text_Grunt3Intro::
	.string "We're pulling a big job here!\n"
	.string "Get lost, kid!$"
WayfarerMtMoon_Text_Grunt3Defeat::
	.string "So, you are good…$"
WayfarerMtMoon_Text_Grunt3After::
	.string "If you find a fossil, give it to me\n"
	.string "and scram!$"
WayfarerMtMoon_Text_Grunt4Intro::
	.string "Little kids shouldn't be messing\n"
	.string "around with grown-ups!\p"
	.string "It could be bad news!$"
WayfarerMtMoon_Text_Grunt4Defeat::
	.string "I'm steamed!$"
WayfarerMtMoon_Text_Grunt4After::
	.string "POKéMON lived here long before\n"
	.string "people came.$"
WayfarerMtMoon_Text_FossilChoice::
	.string "Choose one fossil.$"
WayfarerMtMoon_Text_ReceivedHelix::
	.string "Obtained the HELIX FOSSIL!$"
WayfarerMtMoon_Text_ReceivedDome::
	.string "Obtained the DOME FOSSIL!$"
WayfarerMtMoon_Text_MiguelTakesDome::
	.string "Then the DOME FOSSIL is mine.$"
WayfarerMtMoon_Text_MiguelTakesHelix::
	.string "Then the HELIX FOSSIL is mine.$"
WayfarerMtMoon_Text_NoRoom::
	.string "Your ITEMS pocket is full.$"
