CherrygroveCity_Mart_hns_MapScripts::
	.byte 0

Cherrygrove_Pokemart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
	setvar VAR_0x8005, MART_CLERK_FAMILY_CHERRYGROVE
	specialvar VAR_0x8004, WayfarerLookupMartProfileForSharedClerk
	goto_if_eq VAR_0x8004, MART_PROFILE_NONE, Cherrygrove_Pokemart_EventScript_ClerkLegacy
	special WayfarerOpenMartProfile
	waitstate
	goto Cherrygrove_Pokemart_EventScript_ClerkAfterShop
Cherrygrove_Pokemart_EventScript_ClerkLegacy:
#endif
	goto_if_ge VAR_NEWBARK_TOWN_STATE, 5, Cherrygrove_Pokemart_EventScript_Clerk2
	pokemart Cherrygrove_Pokemart_Pokemart
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
Cherrygrove_Pokemart_EventScript_ClerkAfterShop::
#endif
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Cherrygrove_Pokemart_EventScript_Youngster::
	lock
	faceplayer
	msgbox Cherrygrove_Pokemart_Text_Youngster, MSGBOX_DEFAULT
	closemessage
	release
	end

Cherrygrove_Pokemart_EventScript_Bugcatcher::
	lock
	faceplayer
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
	msgbox Cherrygrove_Pokemart_Text_CooltrainerM_PokeBallsInStock, MSGBOX_DEFAULT
	closemessage
	release
	end
#else
	goto_if_eq VAR_NEWBARK_TOWN_STATE, 5, Cherrygrove_Pokemart_EventScript_Bugcatcher2
	msgbox Cherrygrove_Pokemart_Text_CooltrainerM, MSGBOX_DEFAULT
	closemessage
	release
	end
	
Cherrygrove_Pokemart_EventScript_Bugcatcher2::
	msgbox Cherrygrove_Pokemart_Text_CooltrainerM_PokeBallsInStock, MSGBOX_DEFAULT
	closemessage
	release
	end
#endif

Cherrygrove_Pokemart_EventScript_Clerk2::
	pokemart 0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 3
Cherrygrove_Pokemart_Pokemart:
	.2byte ITEM_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_NONE
	release
	end

Cherrygrove_Pokemart_Text_CooltrainerM:
	.string "They're fresh out of POKé BALLS!\p"
	.string "When will they get more of them?$"

Cherrygrove_Pokemart_Text_CooltrainerM_PokeBallsInStock:
	.string "POKé BALLS are in stock!\n"
	.string "Now I can catch POKéMON!$"

Cherrygrove_Pokemart_Text_Youngster:
	.string "When I was walking in the grass, a\n"
	.string "bug POKéMON poisoned my POKéMON!\p"
	.string "I just kept going, but then my\n"
	.string "POKéMON fainted.\p"
	.string "You should keep an ANTIDOTE\n"
	.string "with you.$"
