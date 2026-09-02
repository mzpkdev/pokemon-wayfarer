MossdeepCity_House3_MapScripts::
	.byte 0

MossdeepCity_House3_EventScript_SuperRodFisherman::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_SUPER_ROD, MossdeepCity_House3_EventScript_ReceivedSuperRod
	msgbox MossdeepCity_House3_Text_YouWantSuperRod, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MossdeepCity_House3_EventScript_DeclineSuperRod
	msgbox MossdeepCity_House3_Text_SuperRodIsSuper, MSGBOX_DEFAULT
	setvar VAR_0x8004, FLAG_RECEIVED_SUPER_ROD
	special Script_TryAwardStandardRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_ALREADY_CONTRIBUTED, MossdeepCity_House3_EventScript_ReceivedSuperRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_NO_SPACE, Common_EventScript_ShowBagIsFull
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_INVALID_STATE, MossdeepCity_House3_EventScript_RodUpgradeFailed
	copyvar VAR_0x8000, VAR_0x8005
	setvar VAR_0x8001, 1
	setvar VAR_0x8007, TRUE
	call EventScript_ObtainItemMessage
	goto_if_eq VAR_0x8005, ITEM_OLD_ROD, MossdeepCity_House3_EventScript_ReceivedOldRod
	goto_if_eq VAR_0x8005, ITEM_GOOD_ROD, MossdeepCity_House3_EventScript_ReceivedGoodRod
	msgbox MossdeepCity_House3_Text_CompletedSuperRod, MSGBOX_DEFAULT
	release
	end

MossdeepCity_House3_EventScript_ReceivedOldRod::
	msgbox MossdeepCity_House3_Text_StartedWithOldRod, MSGBOX_DEFAULT
	release
	end

MossdeepCity_House3_EventScript_ReceivedGoodRod::
	msgbox MossdeepCity_House3_Text_ImprovedToGoodRod, MSGBOX_DEFAULT
	release
	end

MossdeepCity_House3_EventScript_RodUpgradeFailed::
	msgbox MossdeepCity_House3_Text_RodUpgradeFailed, MSGBOX_DEFAULT
	release
	end

MossdeepCity_House3_EventScript_ReceivedSuperRod::
	msgbox MossdeepCity_House3_Text_GoAfterSeafloorPokemon, MSGBOX_DEFAULT
	release
	end

MossdeepCity_House3_EventScript_DeclineSuperRod::
	msgbox MossdeepCity_House3_Text_DontYouLikeToFish, MSGBOX_DEFAULT
	release
	end

MossdeepCity_House3_Text_YouWantSuperRod:
	.string "Hey there, TRAINER!\n"
	.string "A well-tuned fishing ROD is super!\p"
	.string "It gets more bites and gives uncommon\n"
	.string "catches better odds!\p"
	.string "What do you think?\n"
	.string "You want it, don't you?$"

MossdeepCity_House3_Text_SuperRodIsSuper:
	.string "You bet, you bet!\n"
	.string "Let's make your fishing ROD super!$"

MossdeepCity_House3_Text_StartedWithOldRod:
	.string "Your OLD ROD can catch any POKéMON\n"
	.string "living in the local waters!$"

MossdeepCity_House3_Text_ImprovedToGoodRod:
	.string "You know your fishing! Your GOOD ROD\n"
	.string "gets more bites and better odds for\l"
	.string "uncommon catches!$"

MossdeepCity_House3_Text_CompletedSuperRod:
	.string "You bet! Your SUPER ROD gives rare\n"
	.string "catches and bites their best odds!$"

MossdeepCity_House3_Text_RodUpgradeFailed:
	.string "Something isn't right with your fishing\n"
	.string "gear. Come back once it's sorted out.$"

MossdeepCity_House3_Text_TryDroppingRodInWater:
	.string "If there's any water, try dropping in\n"
	.string "your ROD and see what bites!$"

MossdeepCity_House3_Text_DontYouLikeToFish:
	.string "Hunh?\n"
	.string "Don't you like to fish?$"

MossdeepCity_House3_Text_GoAfterSeafloorPokemon:
	.string "Try your ROD wherever there's water,\n"
	.string "and see what bites!$"
