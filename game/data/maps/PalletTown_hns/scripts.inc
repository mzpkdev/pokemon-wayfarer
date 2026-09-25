PalletTown_hns_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PalletTown_KantoOnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_EventScript_CityEnter
	.byte 0

PalletTown_KantoOnLoad:
	setflag FLAG_TEMP_1
	end

PalletTown_EventScript_CityEnter::
	setflag FLAG_VISITED_PALLET_TOWN
	end

PalletTown_EventScript_KantoNorthExit::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_KantoExitAllowed
	specialvar VAR_RESULT, GetPlayerFacingDirection
	goto_if_eq VAR_RESULT, DIR_SOUTH, PalletTown_EventScript_KantoExitAllowed
	setvar VAR_0x8004, PALLET_RECEIPT_STARTER
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_EventScript_KantoExitAllowed
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_ge VAR_RESULT, PALLET_OPENING_LAB_STARTER_CHOICE, PalletTown_EventScript_KantoReturnToLab
	lockall
	specialvar VAR_RESULT, WayfarerKanto_TryStageInterception
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_KantoReleaseExit
	clearflag FLAG_TEMP_1
	addobject LOCALID_PALLET_KANTO_OAK
	applymovement LOCALID_PALLET_KANTO_OAK, PalletTown_Movement_KantoOakApproach
	waitmovement 0
	msgbox PalletTown_Text_KantoOakWarning, MSGBOX_DEFAULT
	msgbox PalletTown_Text_KantoOakLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_PALLET_TOWN_LAB_HNS, 13, 14
	waitstate
PalletTown_EventScript_KantoReturnToLab:
	lockall
	msgbox PalletTown_Text_KantoChooseStarter, MSGBOX_DEFAULT
	closemessage
	warp MAP_PALLET_TOWN_LAB_HNS, 13, 14
	waitstate
PalletTown_EventScript_KantoReleaseExit:
	releaseall
	end
#else
	end
#endif

PalletTown_EventScript_KantoSouthCoast::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_KantoExitAllowed
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, PalletTown_EventScript_KantoExitAllowed
	lockall
	msgbox PalletTown_Text_KantoCoastClosed, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, PalletTown_Movement_KantoStepFromCoast
	waitmovement 0
	releaseall
#endif
	end

PalletTown_EventScript_KantoOak::
#if IS_WAYFARER
	msgbox PalletTown_Text_KantoOakWarning, MSGBOX_NPC
#endif
	end

#if IS_WAYFARER
PalletTown_EventScript_KantoExitAllowed:
	end

PalletTown_Movement_KantoOakApproach:
	walk_up
	step_end

PalletTown_Movement_KantoStepFromCoast:
	walk_up
	step_end

PalletTown_Text_KantoOakWarning:
	.string "OAK: Wait, {PLAYER}!\p"
	.string "Wild POKéMON live in the grass.\n"
	.string "You need a partner to be safe.$"

PalletTown_Text_KantoOakLead:
	.string "OAK: Come with me to my LAB.\p"
	.string "I have a POKéMON for you.$"

PalletTown_Text_KantoChooseStarter:
	.string "PROF. OAK is waiting in his LAB.\p"
	.string "I should choose my partner first.$"

PalletTown_Text_KantoCoastClosed:
	.string "The southern sea can wait.\p"
	.string "I should finish PROF. OAK's\n"
	.string "errand first.$"
#endif

PalletTown_EventScript_Woman::
	msgbox PalletTown_Text_Teacher, MSGBOX_NPC
	end
PalletTown_EventScript_Fatman::
	msgbox PalletTown_Text_Fisher, MSGBOX_NPC
	end
PalletTown_EventScript_TownSign::
	msgbox PalletTown_Text_Sign, MSGBOX_SIGN
	end	
PalletTown_EventScript_RedsHouseSign::
	msgbox PalletTown_Text_RedsHouseSign, MSGBOX_SIGN
	end
PalletTown_EventScript_BluesHouseSign::
	msgbox PalletTown_Text_BluesHouseSign, MSGBOX_SIGN
	end	
PalletTown_EventScript_OaksLabSign::
	msgbox PalletTown_Text_OaksLabSign, MSGBOX_SIGN
	end	

PalletTown_Text_Teacher:
	.string "I'm raising POKéMON too.\p"
	.string "They serve as my private\n"
	.string "guards.$"

PalletTown_Text_Fisher:
	.string "Technology is incredible!\p"
	.string "You can now trade POKéMON\n"
	.string "across time like e-mail.$"

PalletTown_Text_Sign:
	.string "PALLET TOWN\p"
	.string "A Tranquil Setting of Peace\n"
	.string "& Purity$"

PalletTown_Text_RedsHouseSign:
	.string "RED'S HOUSE$"

PalletTown_Text_OaksLabSign:
	.string "OAK POKéMON\n"
	.string "RESEARCH LAB$"

PalletTown_Text_BluesHouseSign:
	.string "BLUE'S HOUSE$"
