PalletTown_hns_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PalletTown_KantoOnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_EventScript_CityEnter
	.byte 0

PalletTown_KantoOnLoad:
	setflag FLAG_TEMP_1
#if IS_WAYFARER
	setvar VAR_TEMP_2, 1
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_KantoOnLoadDone
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, PalletTown_KantoOnLoadDone
	setvar VAR_TEMP_2, 0
PalletTown_KantoOnLoadDone:
#endif
	end

PalletTown_EventScript_CityEnter::
	setflag FLAG_VISITED_PALLET_TOWN
#if IS_WAYFARER
	call PalletTown_EventScript_KantoSignLadyTransition
#endif
	end

#if IS_WAYFARER
@ FRLG PalletTown_OnTransition sign lady staging. VAR_TEMP_3 is FRLG SIGN_LADY_READY.
@ FLAG_OPENED_START_MENU is zero on HNS, so the START-menu shortcut is not ported.
PalletTown_EventScript_KantoSignLadyTransition:
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_KantoReturn
	callnative WayfarerKantoTown_GetSignLadyState
	goto_if_eq VAR_RESULT, 0, PalletTown_EventScript_KantoSetSignLadyPos
	goto_if_eq VAR_RESULT, 1, PalletTown_EventScript_KantoSetSignLadyDone
PalletTown_EventScript_KantoReturn:
	return

PalletTown_EventScript_KantoSetSignLadyDone:
	setvar VAR_0x8004, 2
	callnative WayfarerKantoTown_SetSignLadyState
	return

@ Before the starter she stands north of the Trainer Tips sign on the garden fence.
PalletTown_EventScript_KantoSetSignLadyPos:
	setvar VAR_0x8004, PALLET_RECEIPT_STARTER
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_EventScript_KantoMoveSignLadyToRouteEntrance
	setobjectxyperm LOCALID_PALLET_KANTO_SIGN_LADY, 5, 10
	setobjectmovementtype LOCALID_PALLET_KANTO_SIGN_LADY, MOVEMENT_TYPE_FACE_DOWN
	return

PalletTown_EventScript_KantoMoveSignLadyToRouteEntrance:
	setobjectxyperm LOCALID_PALLET_KANTO_SIGN_LADY, 12, 2
	setobjectmovementtype LOCALID_PALLET_KANTO_SIGN_LADY, MOVEMENT_TYPE_FACE_DOWN
	setvar VAR_TEMP_3, TRUE
	return
#endif

@ FRLG PalletTown_EventScript_OakTriggerLeft/Right. VAR_TEMP_1 is the side.
PalletTown_EventScript_KantoNorthExitLeft::
#if IS_WAYFARER
	setvar VAR_TEMP_1, 0
	goto PalletTown_EventScript_KantoNorthExit
#else
	end
#endif

PalletTown_EventScript_KantoNorthExitRight::
#if IS_WAYFARER
	setvar VAR_TEMP_1, 1
	goto PalletTown_EventScript_KantoNorthExit
#else
	end
#endif

#if IS_WAYFARER
PalletTown_EventScript_KantoNorthExit:
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
	textcolor NPC_TEXT_COLOR_MALE
	delay 30
	playbgm MUS_RG_OAK, FALSE
	message PalletTown_Text_KantoOakDontGoOut
	waitmessage
	delay 85
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	clearflag FLAG_TEMP_1
	addobject LOCALID_PALLET_KANTO_OAK
	call_if_eq VAR_TEMP_1, 0, PalletTown_EventScript_KantoOakEnterLeft
	call_if_eq VAR_TEMP_1, 1, PalletTown_EventScript_KantoOakEnterRight
	delay 30
	msgbox PalletTown_Text_KantoOakGrassUnsafeNeedMon
	closemessage
	delay 30
	call_if_eq VAR_TEMP_1, 0, PalletTown_EventScript_KantoOakLeadPlayerToLabLeft
	call_if_eq VAR_TEMP_1, 1, PalletTown_EventScript_KantoOakLeadPlayerToLabRight
	opendoor 16, 13
	waitdooranim
	applymovement LOCALID_PALLET_KANTO_OAK, PalletTown_Movement_KantoOakEnterLab
	applymovement LOCALID_PLAYER, PalletTown_Movement_KantoPlayerEnterLab
	waitmovement 0
	closedoor 16, 13
	waitdooranim
	setflag FLAG_TEMP_1
	warp MAP_PALLET_TOWN_LAB_HNS, 13, 20
	waitstate
	releaseall
	end

@ Safety net only: the lab keeps an uncommitted player inside. Not FRLG text.
PalletTown_EventScript_KantoReturnToLab:
	lockall
	msgbox PalletTown_Text_KantoChooseStarter, MSGBOX_DEFAULT
	closemessage
	warp MAP_PALLET_TOWN_LAB_HNS, 13, 14
	waitstate
PalletTown_EventScript_KantoReleaseExit:
	releaseall
	end

PalletTown_EventScript_KantoOakEnterLeft:
	applymovement LOCALID_PALLET_KANTO_OAK, PalletTown_Movement_KantoOakEnterLeft
	waitmovement 0
	return

PalletTown_EventScript_KantoOakEnterRight:
	applymovement LOCALID_PALLET_KANTO_OAK, PalletTown_Movement_KantoOakEnterRight
	waitmovement 0
	return

PalletTown_EventScript_KantoOakLeadPlayerToLabLeft:
	applymovement LOCALID_PALLET_KANTO_OAK, PalletTown_Movement_KantoOakWalkToLabLeft
	applymovement LOCALID_PLAYER, PalletTown_Movement_KantoPlayerWalkToLabLeft
	waitmovement 0
	return

PalletTown_EventScript_KantoOakLeadPlayerToLabRight:
	applymovement LOCALID_PALLET_KANTO_OAK, PalletTown_Movement_KantoOakWalkToLabRight
	applymovement LOCALID_PLAYER, PalletTown_Movement_KantoPlayerWalkToLabRight
	waitmovement 0
	return

@ HNS Pallet matches FRLG here: Oak starts at (10,8) and the lab door is (16,13).
PalletTown_Movement_KantoOakEnterLeft:
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	step_end

PalletTown_Movement_KantoOakEnterRight:
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	step_end

	.macro kanto_walk_to_lab
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	.endm

PalletTown_Movement_KantoOakWalkToLabLeft:
	walk_down
	kanto_walk_to_lab
	walk_right
	walk_in_place_faster_up
	step_end

PalletTown_Movement_KantoOakWalkToLabRight:
	walk_down
	walk_left
	kanto_walk_to_lab
	walk_right
	walk_in_place_faster_up
	step_end

PalletTown_Movement_KantoOakEnterLab:
	walk_up
	set_invisible
	step_end

PalletTown_Movement_KantoPlayerWalkToLabLeft:
	walk_down
	walk_down
	kanto_walk_to_lab
	step_end

PalletTown_Movement_KantoPlayerWalkToLabRight:
	walk_down
	walk_down
	walk_left
	kanto_walk_to_lab
	step_end

PalletTown_Movement_KantoPlayerEnterLab:
	walk_right
	walk_up
	set_invisible
	step_end
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

@ Oak is only visible inside the locked escort scene.
PalletTown_EventScript_KantoOak::
	end

#if IS_WAYFARER
@ Coord scripts that end without releaseall leave the player frozen on the
@ trigger tile, so the pass-through path must release explicitly.
PalletTown_EventScript_KantoExitAllowed:
	lockall
	releaseall
	end

PalletTown_Movement_KantoStepFromCoast:
	walk_up
	step_end

@ FRLG PalletTown_EventScript_SignLady.
PalletTown_EventScript_KantoSignLady:
	lock
	callnative WayfarerKantoTown_GetSignLadyState
	goto_if_eq VAR_RESULT, 2, PalletTown_EventScript_KantoSignLadyDone
	goto_if_eq VAR_RESULT, 1, PalletTown_EventScript_KantoSignLadyJustShowedSign
	goto_if_eq VAR_TEMP_3, TRUE, PalletTown_EventScript_KantoSignLadyStartShowSign
	goto_if_set FLAG_TEMP_2, PalletTown_EventScript_KantoSignLadyGoReadSign
	msgbox PalletTown_Text_KantoHmmIsThatRight
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, Common_Movement_Delay48
	waitmovement 0
	msgbox PalletTown_Text_KantoOhLookLook
	closemessage
	call_if_eq VAR_FACING, DIR_EAST, PalletTown_EventScript_KantoSignLadyMoveOutOfWayRight
	call_if_ne VAR_FACING, DIR_EAST, PalletTown_EventScript_KantoSignLadyMoveOutOfWayLeft
	copyobjectxytoperm LOCALID_PALLET_KANTO_SIGN_LADY
	setflag FLAG_TEMP_2
	release
	end

PalletTown_EventScript_KantoSignLadyMoveOutOfWayRight:
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, PalletTown_Movement_KantoSignLadyMoveOutOfWayRight
	waitmovement 0
	return

PalletTown_EventScript_KantoSignLadyMoveOutOfWayLeft:
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, PalletTown_Movement_KantoSignLadyMoveOutOfWayLeft
	waitmovement 0
	return

PalletTown_EventScript_KantoSignLadyDone:
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_KantoRaisingMonsToo
	release
	end

PalletTown_EventScript_KantoSignLadyGoReadSign:
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_KantoReadItReadIt
	release
	end

PalletTown_EventScript_KantoSignLadyJustShowedSign:
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_KantoSignsAreUsefulArentThey
	release
	end

PalletTown_Movement_KantoSignLadyMoveOutOfWayRight:
	walk_right
	walk_in_place_faster_left
	step_end

PalletTown_Movement_KantoSignLadyMoveOutOfWayLeft:
	walk_left
	walk_in_place_faster_right
	step_end

PalletTown_EventScript_KantoSignLadyShowSign:
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox PalletTown_Text_KantoLookCopiedTrainerTipsSign
	closemessage
	delay 20
	textcolor NPC_TEXT_COLOR_NEUTRAL
	setvar VAR_0x8004, 1
	callnative WayfarerKantoTown_SetSignLadyState
	setvar VAR_TEMP_3, FALSE
	special SetWalkingIntoSignVars
	special DisableMsgBoxWalkaway
	signmsg
	msgbox PalletTown_Text_KantoPressStartToOpenMenuCopy
	normalmsg
	return

PalletTown_EventScript_KantoSignLadyStartShowSign:
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	call PalletTown_EventScript_KantoSignLadyShowSign
	release
	end
#endif

@ FRLG PalletTown_EventScript_SignLadyTrigger, coord (13,2) while VAR_TEMP_3 is TRUE.
PalletTown_EventScript_KantoSignLadyTrigger::
#if IS_WAYFARER
	lockall
	applymovement LOCALID_PALLET_KANTO_SIGN_LADY, Common_Movement_WalkInPlaceFasterRight
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	call PalletTown_EventScript_KantoSignLadyShowSign
	releaseall
#endif
	end

@ FRLG PalletTown_EventScript_TrainerTips, hosted on the garden fence at (5,11).
PalletTown_EventScript_KantoTrainerTips::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_KantoExitAllowed
	lockall
	msgbox PalletTown_Text_KantoPressStartToOpenMenu
	setvar VAR_0x8004, 1
	callnative WayfarerKantoTown_SetSignLadyState
	releaseall
#endif
	end

@ The HNS mailboxes stand where FRLG has the house signs.
PalletTown_EventScript_KantoPlayersHouseSign::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_KantoExitAllowed
	msgbox PalletTown_Text_KantoPlayersHouse, MSGBOX_SIGN
#endif
	end

PalletTown_EventScript_KantoRivalsHouseSign::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_KantoExitAllowed
	msgbox PalletTown_Text_KantoRivalsHouse, MSGBOX_SIGN
#endif
	end

#if IS_WAYFARER
PalletTown_Text_KantoOakDontGoOut:
	.string "OAK: Hey! Wait!\n"
	.string "Don't go out!$"

PalletTown_Text_KantoOakGrassUnsafeNeedMon:
	.string "OAK: It's unsafe!\n"
	.string "Wild POKéMON live in tall grass!\p"
	.string "You need your own POKéMON for\n"
	.string "your protection.\p"
	.string "I know!\n"
	.string "Here, come with me!$"

PalletTown_Text_KantoChooseStarter:
	.string "PROF. OAK is waiting in his LAB.\p"
	.string "I should choose my partner first.$"

PalletTown_Text_KantoCoastClosed:
	.string "The southern sea can wait.\p"
	.string "I should finish PROF. OAK's\n"
	.string "errand first.$"

PalletTown_Text_KantoRaisingMonsToo:
	.string "I'm raising POKéMON, too.\p"
	.string "When they get strong, they can\n"
	.string "protect me.$"

PalletTown_Text_KantoCanStoreItemsAndMonsInPC:
	.string "Technology is incredible!\p"
	.string "You can now store and recall items\n"
	.string "and POKéMON as data via PC.$"

PalletTown_Text_KantoOakPokemonResearchLab:
	.string "OAK POKéMON RESEARCH LAB$"

PalletTown_Text_KantoPlayersHouse:
	.string "{PLAYER}'s house$"

PalletTown_Text_KantoRivalsHouse:
	.string "BLUE's house$"

PalletTown_Text_KantoTownSign:
	.string "PALLET TOWN\n"
	.string "Shades of your journey await!$"

PalletTown_Text_KantoHmmIsThatRight:
	.string "Hmm…\n"
	.string "Is that right…$"

PalletTown_Text_KantoOhLookLook:
	.string "Oh!\n"
	.string "Look, look!$"

PalletTown_Text_KantoReadItReadIt:
	.string "Read it, read it!$"

PalletTown_Text_KantoPressStartToOpenMenu:
	.string "TRAINER TIPS\p"
	.string "Press START to open the MENU!$"

PalletTown_Text_KantoSignsAreUsefulArentThey:
	.string "Signs are useful, aren't they?$"

PalletTown_Text_KantoLookCopiedTrainerTipsSign:
	.string "Look, look!\p"
	.string "I copied what it said on one of\n"
	.string "those TRAINER TIPS signs!$"

PalletTown_Text_KantoPressStartToOpenMenuCopy:
	.string "TRAINER TIPS!\p"
	.string "Press START to open the MENU!$"
#endif

PalletTown_EventScript_Woman::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_EventScript_KantoSignLady
#endif
	msgbox PalletTown_Text_Teacher, MSGBOX_NPC
	end
PalletTown_EventScript_Fatman::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_FatmanNative
	msgbox PalletTown_Text_KantoCanStoreItemsAndMonsInPC, MSGBOX_NPC
	end
PalletTown_EventScript_FatmanNative:
#endif
	msgbox PalletTown_Text_Fisher, MSGBOX_NPC
	end
PalletTown_EventScript_TownSign::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_TownSignNative
	msgbox PalletTown_Text_KantoTownSign, MSGBOX_SIGN
	end
PalletTown_EventScript_TownSignNative:
#endif
	msgbox PalletTown_Text_Sign, MSGBOX_SIGN
	end	
PalletTown_EventScript_RedsHouseSign::
	msgbox PalletTown_Text_RedsHouseSign, MSGBOX_SIGN
	end
PalletTown_EventScript_BluesHouseSign::
	msgbox PalletTown_Text_BluesHouseSign, MSGBOX_SIGN
	end	
PalletTown_EventScript_OaksLabSign::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_EventScript_OaksLabSignNative
	lockall
	msgbox PalletTown_Text_KantoOakPokemonResearchLab
	releaseall
	end
PalletTown_EventScript_OaksLabSignNative:
#endif
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
