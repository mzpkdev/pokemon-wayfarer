VioletCity_Mart_hns_MapScripts::
	.byte 0

VioletCity_Mart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
	setvar VAR_0x8005, MART_CLERK_FAMILY_VIOLET
	specialvar VAR_0x8004, WayfarerLookupMartProfileForSharedClerk
	goto_if_eq VAR_0x8004, MART_PROFILE_NONE, VioletCity_Mart_EventScript_ClerkLegacy
	special WayfarerOpenMartProfile
	waitstate
	goto VioletCity_Mart_EventScript_ClerkAfterShop
VioletCity_Mart_EventScript_ClerkLegacy:
#endif
	pokemart 0
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
VioletCity_Mart_EventScript_ClerkAfterShop::
#endif
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

VioletCity_Mart_EventScript_Granny::
	lock
	faceplayer
	msgbox VioletCity_Mart_Text_Granny, MSGBOX_DEFAULT
	closemessage
	release
	end

VioletCity_Mart_EventScript_Rocker::
	lock
	faceplayer
	msgbox VioletCity_Mart_Text_CooltrainerM, MSGBOX_DEFAULT
	closemessage
	release
	end
	
VioletCity_Mart_Text_Granny:
	.string "When you first catch a POKéMON,\n"
	.string "it may be weak.\p"
	.string "But it will eventually grow to\n"
	.string "be strong.\p"
	.string "It's important to treat POKéMON\n"
	.string "with love.$"

VioletCity_Mart_Text_CooltrainerM:
	.string "POKéMON can hold items like\n"
	.string "POTION and ANTIDOTE.\p"
	.string "But they don't appear to know\n"
	.string "how to use man-made items.$"
