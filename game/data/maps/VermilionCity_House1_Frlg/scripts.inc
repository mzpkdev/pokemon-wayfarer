VermilionCity_House1_Frlg_MapScripts::
	.byte 0

VermilionCity_House1_EventScript_FishingGuru::
	lock
	faceplayer
	goto_if_set FLAG_GOT_OLD_ROD, VermilionCity_House1_EventScript_AlreadyGotOldRod
	msgbox VermilionCity_House1_Text_ImFishingGuruDoYouLikeToFish, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, VermilionCity_House1_EventScript_GiveOldRod
	msgbox VermilionCity_House1_Text_OhThatsSoDisappointing
	release
	end

VermilionCity_House1_EventScript_AlreadyGotOldRod::
	msgbox VermilionCity_House1_Text_HowAreTheFishBiting
	release
	end

VermilionCity_House1_EventScript_GiveOldRod::
	msgbox VermilionCity_House1_Text_TakeThisAndFish
	setvar VAR_0x8004, FLAG_GOT_OLD_ROD
	special Script_TryAwardStandardRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_ALREADY_CONTRIBUTED, VermilionCity_House1_EventScript_AlreadyGotOldRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_NO_SPACE, Common_EventScript_ShowBagIsFull
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_INVALID_STATE, VermilionCity_House1_EventScript_RodUpgradeFailed
	copyvar VAR_0x8000, VAR_0x8005
	setvar VAR_0x8001, 1
	setvar VAR_0x8007, TRUE
	call EventScript_ObtainItemMessage
	goto_if_eq VAR_0x8005, ITEM_OLD_ROD, VermilionCity_House1_EventScript_ReceivedOldRod
	goto_if_eq VAR_0x8005, ITEM_GOOD_ROD, VermilionCity_House1_EventScript_ReceivedGoodRod
	msgbox VermilionCity_House1_Text_CompletedSuperRod
	release
	end

VermilionCity_House1_EventScript_ReceivedOldRod::
	msgbox VermilionCity_House1_Text_StartedWithOldRod
	release
	end

VermilionCity_House1_EventScript_ReceivedGoodRod::
	msgbox VermilionCity_House1_Text_ImprovedToGoodRod
	release
	end

VermilionCity_House1_EventScript_RodUpgradeFailed::
	msgbox VermilionCity_House1_Text_RodUpgradeFailed
	release
	end

VermilionCity_House1_Text_ImFishingGuruDoYouLikeToFish::
	.string "I'm the FISHING GURU!\p"
	.string "I simply looove fishing!\n"
	.string "I can't bear to go without.\p"
	.string "Tell me, do you like to fish?$"

VermilionCity_House1_Text_TakeThisAndFish::
	.string "Grand! I like your style.\n"
	.string "I think we can be friends.\p"
	.string "Take this and fish, young friend!$"

VermilionCity_House1_Text_StartedWithOldRod::
	.string "You're an angler now, young friend!\n"
	.string "That OLD ROD can catch any POKéMON\l"
	.string "living in the local waters.$"

VermilionCity_House1_Text_ImprovedToGoodRod::
	.string "I see you're already an angler! Your\n"
	.string "GOOD ROD will get more bites and give\l"
	.string "uncommon catches better odds.$"

VermilionCity_House1_Text_CompletedSuperRod::
	.string "A true fishing devotee! Your SUPER ROD\n"
	.string "gives rare catches and bites their\l"
	.string "best odds.$"

VermilionCity_House1_Text_RodUpgradeFailed::
	.string "Something isn't right with your fishing\n"
	.string "gear. Come back once it's sorted out.$"

VermilionCity_House1_Text_FishingIsAWayOfLife::
	.string "Fishing is a way of life!\n"
	.string "It is like the finest poetry.\p"
	.string "From the seas to rivers, go out\n"
	.string "and land the big one, my friend.$"

VermilionCity_House1_Text_OhThatsSoDisappointing::
	.string "Oh…\n"
	.string "That's so disappointing…$"

VermilionCity_House1_Text_HowAreTheFishBiting::
	.string "Hello there, {PLAYER}!\p"
	.string "How are the fish biting?$"

