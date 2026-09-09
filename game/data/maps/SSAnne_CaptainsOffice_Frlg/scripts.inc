SSAnne_CaptainsOffice_Frlg_MapScripts::
	.byte 0

SSAnne_CaptainsOffice_EventScript_Captain::
#if IS_WAYFARER
	lock
	faceplayer
	goto_if_set FLAG_SS_ANNE_CAPTAIN_REWARD_HNS, SSAnne_CaptainsOffice_EventScript_AlreadyGotCut
	msgbox SSAnne_CaptainsOffice_Text_CaptainIFeelSeasick
	textcolor NPC_TEXT_COLOR_NEUTRAL
	message SSAnne_CaptainsOffice_Text_RubbedCaptainsBack
	waitmessage
	playfanfare MUS_HEAL
	waitfanfare
	call EventScript_RestorePrevTextColor
	delay 50
	applymovement LOCALID_SS_ANNE_CAPTAIN, Common_Movement_FacePlayer
	waitmovement 0
	msgbox SSAnne_CaptainsOffice_Text_ThankYouHaveHMForCut
	checkitem ITEM_HM_CUT
	goto_if_eq VAR_RESULT, TRUE, SSAnne_CaptainsOffice_EventScript_ReconcileCut
	setorcopyvar VAR_0x8000, ITEM_HM_CUT
	setorcopyvar VAR_0x8001, 1
	call Std_ObtainItem
	goto_if_eq VAR_RESULT, FALSE, SSAnne_CaptainsOffice_EventScript_NoRoomForCut
	checkitem ITEM_HM_CUT
	goto_if_eq VAR_RESULT, FALSE, SSAnne_CaptainsOffice_EventScript_NoRoomForCut
	msgbox SSAnne_CaptainsOffice_Text_ExplainCut
	setflag FLAG_SS_ANNE_CAPTAIN_REWARD_HNS
	release
	end

SSAnne_CaptainsOffice_EventScript_ReconcileCut::
	msgbox SSAnne_CaptainsOffice_Text_AlreadyHaveCut
	setflag FLAG_SS_ANNE_CAPTAIN_REWARD_HNS
	release
	end

SSAnne_CaptainsOffice_EventScript_NoRoomForCut::
	msgbox SSAnne_CaptainsOffice_Text_YouHaveNoRoomForThis
	closemessage
	applymovement LOCALID_SS_ANNE_CAPTAIN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

SSAnne_CaptainsOffice_EventScript_AlreadyGotCut::
	msgbox SSAnne_CaptainsOffice_Text_SSAnneRemainsInPort
	release
	end
#else
	lock
	goto_if_set FLAG_GOT_HM01, SSAnne_CaptainsOffice_EventScript_AlreadyGotCut
	msgbox SSAnne_CaptainsOffice_Text_CaptainIFeelSeasick
	textcolor NPC_TEXT_COLOR_NEUTRAL
	message SSAnne_CaptainsOffice_Text_RubbedCaptainsBack
	waitmessage
	playfanfare MUS_HEAL
	waitfanfare
	call EventScript_RestorePrevTextColor
	delay 50
	applymovement LOCALID_SS_ANNE_CAPTAIN, Common_Movement_FacePlayer
	waitmovement 0
	msgbox SSAnne_CaptainsOffice_Text_ThankYouHaveHMForCut
	giveitem_msg SSAnne_CaptainsOffice_Text_ObtainedHM01FromCaptain, ITEM_HM01, 1, MUS_RG_OBTAIN_KEY_ITEM
	msgbox SSAnne_CaptainsOffice_Text_ExplainCut
	setflag FLAG_GOT_HM01
	setvar VAR_MAP_SCENE_VERMILION_CITY, 1
	release
	end

@ Unused
SSAnne_CaptainsOffice_EventScript_NoRoomForCut::
	msgbox SSAnne_CaptainsOffice_Text_YouHaveNoRoomForThis
	closemessage
	applymovement LOCALID_SS_ANNE_CAPTAIN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

SSAnne_CaptainsOffice_EventScript_AlreadyGotCut::
	applymovement LOCALID_SS_ANNE_CAPTAIN, Common_Movement_FacePlayer
	waitmovement 0
	msgbox SSAnne_CaptainsOffice_Text_SSAnneWillSetSailSoon
	release
	end
#endif

SSAnne_CaptainsOffice_EventScript_TrashCan::
	msgbox SSAnne_CaptainsOffice_Text_YuckShouldntHaveLooked, MSGBOX_SIGN
	end

SSAnne_CaptainsOffice_EventScript_Book::
	msgbox SSAnne_CaptainsOffice_Text_HowToConquerSeasickness, MSGBOX_SIGN
	end

SSAnne_CaptainsOffice_Text_CaptainIFeelSeasick::
	.string "CAPTAIN: Ooargh…\n"
	.string "I feel hideous…\l"
	.string "Urrp! Seasick…$"

SSAnne_CaptainsOffice_Text_RubbedCaptainsBack::
	.string "{PLAYER} rubbed the CAPTAIN's\n"
	.string "back!\p"
	.string "Rub-rub…\n"
	.string "Rub-rub…$"

SSAnne_CaptainsOffice_Text_ThankYouHaveHMForCut::
	.string "CAPTAIN: Whew! Thank you!\n"
	.string "I'm feeling much better now.\p"
	.string "You want to see my hidden CUT\n"
	.string "technique?\p"
	.string "I could show you my prized CUT\n"
	.string "technique if I weren't so ill…\p"
	.string "I know! You can have this\n"
	.string "HIDDEN MACHINE for CUT!$"

SSAnne_CaptainsOffice_Text_ObtainedHM01FromCaptain::
	.string "{PLAYER} obtained HM01\n"
	.string "from the CAPTAIN!$"

SSAnne_CaptainsOffice_Text_ExplainCut::
	.string "Using CUT, you can chop down\n"
	.string "small trees.\p"
	.string "You don't need a BADGE to use an\n"
	.string "HM move in the field.\p"
	.string "With its HM in your BAG, a\n"
	.string "compatible POKéMON in your party\l"
	.string "can use the move without\l"
	.string "learning it.\p"
	.string "EGGS can't use HM moves.\p"
	.string "Teaching the move is optional.\n"
	.string "If you do, it can be forgotten or\l"
	.string "replaced like any other move.$"

SSAnne_CaptainsOffice_Text_SSAnneWillSetSailSoon::
	.string "CAPTAIN: …Whew!\p"
	.string "Now that I'm not sick anymore,\n"
	.string "I guess it's time.\p"
	.string "The S.S. ANNE will set sail soon!\p"
	.string "Farewell, until our return to\n"
	.string "VERMILION CITY!$"

#if IS_WAYFARER
SSAnne_CaptainsOffice_Text_SSAnneRemainsInPort::
	.string "CAPTAIN: …Whew!\p"
	.string "I'm feeling much better now.\n"
	.string "Enjoy the rest of your visit aboard\l"
	.string "the S.S. ANNE!$"

SSAnne_CaptainsOffice_Text_AlreadyHaveCut::
	.string "CAPTAIN: Ah, you already carry the\n"
	.string "CUT technique!\p"
	.string "Then put it to good use and enjoy\n"
	.string "your visit aboard the S.S. ANNE!$"
#endif

SSAnne_CaptainsOffice_Text_YouHaveNoRoomForThis::
	.string "Oh, no!\n"
	.string "You have no room for this!$"

SSAnne_CaptainsOffice_Text_YuckShouldntHaveLooked::
	.string "Yuck!\n"
	.string "Shouldn't have looked!$"

SSAnne_CaptainsOffice_Text_HowToConquerSeasickness::
	.string "How to Conquer Seasickness…\n"
	.string "The CAPTAIN's reading this!$"
