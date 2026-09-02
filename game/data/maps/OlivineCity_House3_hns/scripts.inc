OlivineCity_House3_hns_MapScripts::
	.byte 0

OlivineCityHouse3_EventScriptFishingGuru::
	lock
	faceplayer
	goto_if_set FLAG_STANDARD_ROD_OLIVINE_CONTRIBUTED, OlivineCityHouse3_EventScriptFishingGuru2
	msgbox OlivineCity_Text_OfferGoodRod, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, OlivineCityHouse3_EventScriptFishingGuruRefused
	msgbox OlivineCity_Text_GiveGoodRod, MSGBOX_DEFAULT
	setvar VAR_0x8004, FLAG_STANDARD_ROD_OLIVINE_CONTRIBUTED
	special Script_TryAwardStandardRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_ALREADY_CONTRIBUTED, OlivineCityHouse3_EventScriptFishingGuru2
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_NO_SPACE, Common_EventScript_ShowBagIsFull
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_INVALID_STATE, OlivineCityHouse3_EventScriptRodUpgradeFailed
	copyvar VAR_0x8000, VAR_0x8005
	setvar VAR_0x8001, 1
	setvar VAR_0x8007, TRUE
	call EventScript_ObtainItemMessage
	goto_if_eq VAR_0x8005, ITEM_OLD_ROD, OlivineCityHouse3_EventScriptReceivedOldRod
	goto_if_eq VAR_0x8005, ITEM_GOOD_ROD, OlivineCityHouse3_EventScriptReceivedGoodRod
	msgbox OlivineCity_Text_CompletedSuperRod, MSGBOX_DEFAULT
	goto OlivineCityHouse3_EventScriptFishingGuruDone

OlivineCityHouse3_EventScriptReceivedOldRod::
	msgbox OlivineCity_Text_StartedWithOldRod, MSGBOX_DEFAULT
	goto OlivineCityHouse3_EventScriptFishingGuruDone

OlivineCityHouse3_EventScriptReceivedGoodRod::
	msgbox OlivineCity_Text_ImprovedToGoodRod, MSGBOX_DEFAULT

OlivineCityHouse3_EventScriptFishingGuruDone::
	msgbox OlivineCity_Text_GaveGoodRod, MSGBOX_DEFAULT
	closemessage
	release
	end

OlivineCityHouse3_EventScriptRodUpgradeFailed::
	msgbox OlivineCity_Text_RodUpgradeFailed, MSGBOX_DEFAULT
	closemessage
	release
	end

OlivineCityHouse3_EventScriptFishingGuru2::
	msgbox OlivineCity_Text_HaveGoodRod, MSGBOX_DEFAULT
	closemessage
	release
	end

OlivineCityHouse3_EventScriptFishingGuruRefused::
	msgbox OlivineCity_Text_DontWantGoodRod, MSGBOX_DEFAULT
	closemessage
	release
	end

OlivineCity_Text_OfferGoodRod:
	.string "OLIVINE is on the sea!\p"
	.string "And if it's on the sea, there\n"
	.string "are bound to be fish!\p"
	.string "I've fished here for 30 years.\p"
	.string "Would you like to face the sea\n"
	.string "and try your hand at fishing?$"

OlivineCity_Text_GiveGoodRod:
	.string "Ah, hahah! We have ourselves\n"
	.string "an angler who appreciates the sea!$"

OlivineCity_Text_StartedWithOldRod:
	.string "After 30 years, I know a fine starter\n"
	.string "ROD. This OLD ROD can catch any\l"
	.string "POKéMON living in the local waters.$"

OlivineCity_Text_ImprovedToGoodRod:
	.string "You've learned a thing or two! My 30\n"
	.string "years tell me this GOOD ROD will get\l"
	.string "more bites and uncommon catches.$"

OlivineCity_Text_CompletedSuperRod:
	.string "Thirty years on this sea taught me how\n"
	.string "to finish a ROD! Your SUPER ROD gives\l"
	.string "rare catches and bites the best odds.$"

OlivineCity_Text_RodUpgradeFailed:
	.string "Something isn't right with your fishing\n"
	.string "gear. Come back once it's sorted out.$"

OlivineCity_Text_GaveGoodRod:
	.string "Fish aren't only found in the\n"
	.string "sea, they're wherever there's\l"
	.string "water.\p"
	.string "By the way, your POKéDEX shows\n"
	.string "where POKéMON can be fished for.\p"
	.string "Different spots may contain\n"
	.string "similar fish types.$"

OlivineCity_Text_DontWantGoodRod:
	.string "Whaaat? You don't like to fish!?\n"
	.string "That's incomprehensible!$"

OlivineCity_Text_HaveGoodRod:
	.string "How are things?\n"
	.string "Did you land the big one?$"
