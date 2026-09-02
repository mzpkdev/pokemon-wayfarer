FuchsiaCity_House2_Frlg_MapScripts::
	.byte 0

FuchsiaCity_House2_EventScript_FishingGurusBrother::
	lock
	faceplayer
	goto_if_set FLAG_GOT_GOOD_ROD, FuchsiaCity_House2_EventScript_AlreadyGotGoodRod
	msgbox FuchsiaCity_House2_Text_DoYouLikeToFish, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, FuchsiaCity_House2_EventScript_GiveGoodRod
	msgbox FuchsiaCity_House2_Text_OhThatsDisappointing
	release
	end

FuchsiaCity_House2_EventScript_AlreadyGotGoodRod::
	msgbox FuchsiaCity_House2_Text_HowAreTheFishBiting
	release
	end

FuchsiaCity_House2_EventScript_GiveGoodRod::
	msgbox FuchsiaCity_House2_Text_LikeYourStyleTakeThis
	setvar VAR_0x8004, FLAG_GOT_GOOD_ROD
	special Script_TryAwardStandardRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_ALREADY_CONTRIBUTED, FuchsiaCity_House2_EventScript_AlreadyGotGoodRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_NO_SPACE, Common_EventScript_ShowBagIsFull
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_INVALID_STATE, FuchsiaCity_House2_EventScript_RodUpgradeFailed
	copyvar VAR_0x8000, VAR_0x8005
	setvar VAR_0x8001, 1
	setvar VAR_0x8007, TRUE
	call EventScript_ObtainItemMessage
	goto_if_eq VAR_0x8005, ITEM_OLD_ROD, FuchsiaCity_House2_EventScript_ReceivedOldRod
	goto_if_eq VAR_0x8005, ITEM_GOOD_ROD, FuchsiaCity_House2_EventScript_ReceivedUpgradedGoodRod
	msgbox FuchsiaCity_House2_Text_CompletedSuperRod
	release
	end

FuchsiaCity_House2_EventScript_ReceivedOldRod::
	msgbox FuchsiaCity_House2_Text_StartedWithOldRod
	release
	end

FuchsiaCity_House2_EventScript_ReceivedUpgradedGoodRod::
	msgbox FuchsiaCity_House2_Text_ImprovedToGoodRod
	release
	end

FuchsiaCity_House2_EventScript_RodUpgradeFailed::
	msgbox FuchsiaCity_House2_Text_RodUpgradeFailed
	release
	end

FuchsiaCity_House2_Text_DoYouLikeToFish::
	.string "I'm the FISHING GURU's older\n"
	.string "brother.\p"
	.string "I simply looove fishing!\n"
	.string "I can't bear to go without.\p"
	.string "Tell me, do you like to fish?$"

FuchsiaCity_House2_Text_LikeYourStyleTakeThis::
	.string "Grand! I like your style.\n"
	.string "I think we can be friends.\p"
	.string "Take this and fish, young friend!$"

FuchsiaCity_House2_Text_StartedWithOldRod::
	.string "Fishing is a way of life!\n"
	.string "It is like the finest poetry.\p"
	.string "Your OLD ROD can catch any POKéMON\n"
	.string "living in the local waters.$"

FuchsiaCity_House2_Text_ImprovedToGoodRod::
	.string "You have an angler's spirit! Your GOOD\n"
	.string "ROD gets more bites and gives uncommon\l"
	.string "catches better odds.$"

FuchsiaCity_House2_Text_CompletedSuperRod::
	.string "A fine angler indeed! Your SUPER ROD\n"
	.string "gives rare catches and bites their\l"
	.string "best odds.$"

FuchsiaCity_House2_Text_RodUpgradeFailed::
	.string "Something isn't right with your fishing\n"
	.string "gear. Come back once it's sorted out.$"

FuchsiaCity_House2_Text_OhThatsDisappointing::
	.string "Oh…\n"
	.string "That's so disappointing…$"

FuchsiaCity_House2_Text_HowAreTheFishBiting::
	.string "Hello there, {PLAYER}!\p"
	.string "How are the fish biting?$"

