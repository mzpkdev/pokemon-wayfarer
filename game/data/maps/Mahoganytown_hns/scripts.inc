Mahoganytown_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Mahoganytown_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, Mahoganytown_OnLoad
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Mahoganytown_OnFrame
	.byte 0

Mahoganytown_OnTransition::
	call_if_unset FLAG_VISITED_MAHOGANY_TOWN, MahoganyTown_EventScript_Setup
	end

Mahoganytown_OnLoad::
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
	call MahoganyTown_Gym_EventScript_TryArmRocketTakeover
#endif
	call_if_ge VAR_MAHOGANY_TOWN_STATE, 13, MahoganyTown_EventScript_FixTree
	end

MahoganyTown_EventScript_FixTree::
	setmetatile 18, 8, 0x378, FALSE
	setmetatile 19, 8, 0x379, FALSE
	end

Mahoganytown_OnFrame::
	map_script_2 VAR_TRIGGER_ELM_ROCKET_CALL, 1, Mahoganytown_EventScript_RocketTrigger
	.2byte 0

Mahoganytown_EventScript_RocketTrigger::
	lock
	setspeaker Route30_Text_ElmName
	pokenavcall ElmPhoneRadioTowerRocketTakeoverText
	waitmessage
	setvar_forward VAR_GOLDENROD_CITY_STATE, 6
	clearflag FLAG_HIDE_GOLDENROD_ROCKETS
	setflag FLAG_HIDE_GOLDENROD_ROCKET_SCOUT
	setflag FLAG_HIDE_GOLDENROD_NPCS
	clearflag FLAG_HIDE_ROCKET_GOLDENROD_RADIOTOWER_1
	setvar VAR_TRIGGER_ELM_ROCKET_CALL, 2
	delay 10
	release
	end

@ First-visit setup, run once from the map load however the player arrives.
@ New-game defaults are not restated, and Blackthorn's own state is set up by
@ Blackthorn itself.
MahoganyTown_EventScript_Setup::
	setflag FLAG_VISITED_MAHOGANY_TOWN
	setflag FLAG_HIDE_LAKE_OF_RAGE_LANCE
	setflag FLAG_HIDE_MAHOGANY_TOWN_SHOP_NPCS
	setflag FLAG_HIDE_MAHOGANY_COP
	setflag FLAG_HIDE_ROCKETHIDEOUT_ROCKETS
	setflag FLAG_HIDE_ROCKETHIDEOUT3_SILVER
	setflag FLAG_HIDE_ROCKETHIDEOUT3_PETREL
	setflag FLAG_HIDE_ROCKETHIDEOUT2_MURKROWSTAIRS
	setflag FLAG_HIDE_ROCKETHIDEOUT2_MURKROWGATE
	setflag FLAG_HIDE_ROCKETHIDEOUT3_MURKROWSTAIRS
	setflag FLAG_HIDE_ROCKETHIDEOUT2_ARIANA_AND_GRUNT
	setvar_forward VAR_MAHOGANY_TOWN_STATE, 1
	return

MahoganyTown_EventScript_Merchant::
#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
	lock
	faceplayer
MahoganyTown_EventScript_MerchantMenu::
	message MahoganyTown_Text_MerchantMenu
	waitmessage
	multichoice 18, 6, MULTI_MAHOGANY_MERCHANT, FALSE
	switch VAR_RESULT
	case 0, MahoganyTown_EventScript_MerchantSupplies
	case 1, MahoganyTown_EventScript_MerchantRageCandyBar
	case 2, MahoganyTown_EventScript_MerchantExit
	case MULTI_B_PRESSED, MahoganyTown_EventScript_MerchantExit
	goto MahoganyTown_EventScript_MerchantExit

MahoganyTown_EventScript_MerchantSupplies::
	setvar VAR_0x8004, MART_PROFILE_MAHOGANY
	special WayfarerOpenMartProfile
	waitstate
	goto MahoganyTown_EventScript_MerchantMenu

MahoganyTown_EventScript_MerchantRageCandyBar::
	goto_if_set FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED, MahoganyTown_EventScript_MerchantAfter
	showmoneybox 0, 0
	msgbox MahoganyTown_Text_RageCandyBarMerchantTryOne, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, MahoganyTown_EventScript_MerchantRefused
	checkmoney 300, 0
	goto_if_eq VAR_RESULT, FALSE, MahoganyTown_EventScript_MerchantBroke
	removemoney 300
	updatemoneybox 0
	msgbox MahoganyTown_Text_RageCandyBarMerchantSavorIt, MSGBOX_DEFAULT
	giveitem ITEM_RAGE_CANDY_BAR
	hidemoneybox
	closemessage
	release
	end

MahoganyTown_EventScript_MerchantExit::
	closemessage
	release
	end
#else
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_JOHTO_RADIO_TOWER_RESOLVED, MahoganyTown_EventScript_MerchantAfter
	showmoneybox 0, 0
	msgbox MahoganyTown_Text_RageCandyBarMerchantTryOne, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, MahoganyTown_EventScript_MerchantRefused
	checkmoney 300, 0
	goto_if_eq VAR_RESULT, FALSE, MahoganyTown_EventScript_MerchantBroke
	removemoney 300
	updatemoneybox 0
	msgbox MahoganyTown_Text_RageCandyBarMerchantSavorIt, MSGBOX_DEFAULT
	giveitem ITEM_RAGE_CANDY_BAR
	hidemoneybox
	closemessage
	release
	end
#endif

MahoganyTown_EventScript_MerchantRefused::
	msgbox MahoganyTown_Text_RageCandyBarMerchantRefused, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	closemessage
	release
	end

MahoganyTown_EventScript_MerchantBroke::
	msgbox MahoganyTown_Text_RageCandyBarMerchantNotEnoughMoney, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	closemessage
	release
	end

MahoganyTown_EventScript_NotEnoughSpace:
	msgbox MahoganyTown_Text_RageCandyBarMerchantRefused, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	closemessage
	release
	return

MahoganyTown_EventScript_MerchantAfter::
	msgbox MahoganyTown_Text_RageCandyBarMerchantSoldOut, MSGBOX_DEFAULT
	closemessage
	release
	end

MahoganyTown_EventScript_Gramps::
	lock
	faceplayer
	goto_if_eq VAR_MAHOGANY_TOWN_STATE, 6, MahoganyTown_EventScript_GrampsAfter
	msgbox MahoganyTown_Text_Gramps, MSGBOX_DEFAULT
	closemessage
	release
	end

MahoganyTown_EventScript_GrampsAfter::
	msgbox MahoganyTown_Text_GrampsClearedRocketHideout, MSGBOX_DEFAULT
	closemessage
	release
	end

MahoganyTown_EventScript_Fisher::
	lock
	faceplayer
	msgbox MahoganyTown_Text_Fisher, MSGBOX_DEFAULT
	closemessage
	release
	end

MahoganyTown_EventScript_Lass::
	lock
	faceplayer
	msgbox MahoganyTown_Text_Lass, MSGBOX_DEFAULT
	closemessage
	release
	end

MahoganyTown_EventScript_Sign::
	lock
	msgbox MahoganyTown_Text_Sign, MSGBOX_SIGN
	closemessage
	release
	end

MahoganyTown_EventScript_CandybarSign::
	lock
	msgbox MahoganyTown_Text_RagecandybarSign, MSGBOX_SIGN
	closemessage
	release
	end

MahoganyTown_EventScript_GymSign::
	lock
	msgbox MahoganyTown_Text_GymSign, MSGBOX_SIGN
	closemessage
	release
	end

MahoganyTown_Text_RageCandyBarMerchantTryOne:
	.string "Hiya, kid!\p"
	.string "I see you're new in MAHOGANY TOWN.\p"
	.string "Since you're new, you should try a\n"
	.string "yummy RAGECANDYBAR!\p"
	.string "Right now, it can be yours for just\n"
	.string "¥300! Want one?$"

MahoganyTown_Text_RageCandyBarMerchantSavorIt:
    .string "Good! Savor it!$"

MahoganyTown_Text_RageCandyBarMerchantNotEnoughMoney:
    .string "You don't have enough money.$"

MahoganyTown_Text_RageCandyBarMerchantRefused:
    .string "Oh, fine then…$"

MahoganyTown_Text_RageCandyBarMerchantSoldOut:
    .string "RAGECANDYBAR's sold out.\p"
    .string "I'm packing up. Don't bother me,\n"
    .string "kiddo.$"

#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED
MahoganyTown_Text_MerchantMenu:
	.string "Need supplies or a RAGE CANDY BAR?$"
#endif

MahoganyTown_Text_Gramps:
    .string "Are you off to see the GYARADOS\n"
    .string "rampage at the LAKE?$"

MahoganyTown_Text_GrampsClearedRocketHideout:
    .string "MAGIKARP have returned to LAKE\n"
    .string "OF RAGE.\p"
    .string "That should be good news for the\n"
    .string "anglers there.$"

MahoganyTown_Text_Fisher:
    .string "Since you came this far, take the\n"
    .string "time to do some sightseeing.\p"
    .string "You should head north and check\n"
    .string "out LAKE OF RAGE right now.$"

MahoganyTown_Text_Lass:
    .string "Visit Grandma's shop. She sells\n"
    .string "stuff that nobody else has.$"

MahoganyTown_Text_Sign:
    .string "MAHOGANY TOWN\p"
    .string "Welcome to the Home of the Ninja$"

MahoganyTown_Text_RagecandybarSign:
    .string "While visiting MAHOGANY TOWN, try\n"
    .string "a RAGECANDYBAR!$"

MahoganyTown_Text_GymSign:
    .string "MAHOGANY TOWN POKéMON GYM\p"
    .string "LEADER: PRYCE\p"
    .string "The Teacher of Winter's Harshness$"

ElmPhoneRadioTowerRocketTakeoverText:
	.string "PROF.ELM: {PLAYER}, how are\n"
	.string "things going?\p"
	.string "I called because something weird\n"
	.string "is happening with the radio\l"
	.string "broadcasts.\p"
	.string "They were talking about TEAM\n"
	.string "ROCKET.\p"
	.string "{PLAYER}, do you know anything\n"
	.string "about it?\p"
	.string "Maybe TEAM ROCKET has returned.\n"
	.string "No, that just can't be true.\p"
	.string "Sorry to bug you.\n"
	.string "Take care!$"
