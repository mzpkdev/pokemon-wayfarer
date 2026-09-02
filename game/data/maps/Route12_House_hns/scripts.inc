Route12_House_hns_MapScripts::
	.byte 0

Route12_House_EventScript_SuperRod::
	lock
	faceplayer
	goto_if_set FLAG_STANDARD_ROD_ROUTE12_CONTRIBUTED, Route12_House_EventScript_SuperRodGot
	msgbox Route12_House_Text_OfferSuperRod, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, Route12_House_EventScript_SuperRodRefused
	msgbox Route12_House_Text_GiveSuperRod, MSGBOX_DEFAULT
	setvar VAR_0x8004, FLAG_STANDARD_ROD_ROUTE12_CONTRIBUTED
	special Script_TryAwardStandardRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_ALREADY_CONTRIBUTED, Route12_House_EventScript_SuperRodGot
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_NO_SPACE, Common_EventScript_ShowBagIsFull
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_INVALID_STATE, Route12_House_EventScript_RodUpgradeFailed
	copyvar VAR_0x8000, VAR_0x8005
	setvar VAR_0x8001, 1
	setvar VAR_0x8007, TRUE
	call EventScript_ObtainItemMessage
	goto_if_eq VAR_0x8005, ITEM_OLD_ROD, Route12_House_EventScript_ReceivedOldRod
	goto_if_eq VAR_0x8005, ITEM_GOOD_ROD, Route12_House_EventScript_ReceivedGoodRod
	msgbox Route12_House_Text_CompletedSuperRod, MSGBOX_DEFAULT
	goto Route12_House_EventScript_RodAwardDone

Route12_House_EventScript_ReceivedOldRod::
	msgbox Route12_House_Text_StartedWithOldRod, MSGBOX_DEFAULT
	goto Route12_House_EventScript_RodAwardDone

Route12_House_EventScript_ReceivedGoodRod::
	msgbox Route12_House_Text_ImprovedToGoodRod, MSGBOX_DEFAULT

Route12_House_EventScript_RodAwardDone::
	msgbox Route12_House_Text_GaveSuperRod, MSGBOX_DEFAULT
	closemessage
	release
	end

Route12_House_EventScript_RodUpgradeFailed::
	msgbox Route12_House_Text_RodUpgradeFailed, MSGBOX_DEFAULT
	closemessage
	release
	end

Route12_House_EventScript_SuperRodRefused::
	msgbox Route12_House_Text_DontWantSuperRod, MSGBOX_DEFAULT
	closemessage
	release
	end

Route12_House_EventScript_SuperRodGot::
	msgbox Route12_House_Text_GaveSuperRod, MSGBOX_NPC
	release
	end

Route12_House_Text_OfferSuperRod:
	.string "I'm the FISHING GURU's younger\n"
	.string "brother.\p"
	.string "I can see that you like fishing.\p"
	.string "There's no doubt in my mind at\n"
	.string "all!\p"
	.string "So? I know I'm right.$"

Route12_House_Text_GiveSuperRod:
	.string "Yes, yes. Just as I thought!\p"
	.string "Here, fishing fan! Let me tune up\n"
	.string "your ROD.$"

Route12_House_Text_StartedWithOldRod:
	.string "You're an angler now! That OLD ROD can\n"
	.string "catch any POKéMON living in the local\l"
	.string "waters.$"

Route12_House_Text_ImprovedToGoodRod:
	.string "I knew you were experienced! The GOOD\n"
	.string "ROD gets more bites and gives uncommon\l"
	.string "catches better odds.$"

Route12_House_Text_CompletedSuperRod:
	.string "My eyes were right! The SUPER ROD gives\n"
	.string "rare catches and bites their best odds.$"

Route12_House_Text_RodUpgradeFailed:
	.string "Something isn't right with your fishing\n"
	.string "gear. Come back once it's sorted out.$"

Route12_House_Text_GaveSuperRod:
	.string "Try your hand at fishing wherever\n"
	.string "there is water.\p"
	.string "Better RODS get more bites and improve\n"
	.string "the odds of uncommon catches.$"

Route12_House_Text_DontWantSuperRod:
	.string "Huh? My own eyes deceived me?$"
