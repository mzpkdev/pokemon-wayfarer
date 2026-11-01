VermilionCity_PortInside_hns_MapScripts::
	.byte 0

VermilionPortIndoor_EventScript_Teacher::
	msgbox VermilionPortPassage_Text_Teacher, MSGBOX_NPC
	end
VermilionPortIndoor_EventScript_Nerd::
	msgbox VermilionPort_Text_SuperNerd, MSGBOX_NPC
	end

VermilionPortPassage_Text_Teacher:
	.string "The FAST SHIP sails every day!$"

VermilionPort_Text_SuperNerd:
	.string "You came from JOHTO?\p"
	.string "I hear many rare POKéMON live\n"
	.string "over there.$"

VermilionPort_EventScript_Sailor::
	lock
	faceplayer
#if IS_WAYFARER
	@ One sailor runs the S.S. berth, the Seagallop, and (before the captain's reward) the S.S. Anne adventure.
	special DrawWayfarerVermilionPortMenu
	waitstate
	specialvar VAR_RESULT, GetWayfarerVermilionPortChoice
	switch VAR_RESULT
	case 0, VermilionPort_EventScript_Sailor_Ferry
	case 1, VermilionPort_EventScript_Sailor_Sevii
	case 2, VermilionPort_EventScript_Sailor_BoardSSAnne
	case 3, VermilionPort_EventScript_Sailor_Cancel
	case MULTI_B_PRESSED, VermilionPort_EventScript_Sailor_Cancel
	release
	end

@ The ship in port (or the Anne's first departure): boards with no further question.
VermilionPort_EventScript_Sailor_Ferry::
	setvar VAR_0x8004, FERRY_BERTH_DOCK
	call Ferry_EventScript_SailorTalk
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_SailorEnd
	call VermilionPort_EventScript_FerryBoardingWalk
	special Ferry_WarpAboard
	waitstate
	release
	end

@ The ferry-only boarding walk: the gangway walk of EnterShip, without moving the boat and without setrespawn.
VermilionPort_EventScript_FerryBoardingWalk::
	applymovement LOCALID_VERMILIONMARINA_SAILOR, VermilionPort_Movement_SailorEnterBoat
	waitmovement 0
	playse SE_EXIT
	applymovement OBJ_EVENT_ID_PLAYER, VermilionPort_Movement_PlayerEnterBoat
	waitmovement 0
	playse SE_EXIT
	delay 20
	return

VermilionPort_EventScript_Sailor_Sevii::
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	special DrawWayfarerVermilionSeviiDestinationMenu
	waitstate
	specialvar VAR_0x8006, GetWayfarerVermilionSeviiDestination
	switch VAR_0x8006
	case SEAGALLOP_ONE_ISLAND, VermilionPort_EventScript_Sailor_SetSailToSeviiDestination
	case SEAGALLOP_BIRTH_ISLAND, VermilionPort_EventScript_Sailor_SetSailToSeviiDestination
	case SEAGALLOP_NAVEL_ROCK, VermilionPort_EventScript_Sailor_SetSailToSeviiDestination
	case MULTI_B_PRESSED, VermilionPort_EventScript_Sailor_Cancel
	release
	end

VermilionPort_EventScript_Sailor_SetSailToSeviiDestination::
	goto WayfarerSevii_EventScript_SailToSelectedDestination
	end

@ The adventure's own entry, before the captain's reward: it is not the ferry's boarding.
VermilionPort_EventScript_Sailor_BoardSSAnne::
	specialvar VAR_RESULT, WayfarerSSAnne_HasUnfinishedContent
	goto_if_eq VAR_RESULT, FALSE, VermilionPort_EventScript_Sailor_SSAnneComplete
	checkitem ITEM_SS_TICKET
	bufferitemname STR_VAR_1, ITEM_SS_TICKET
	goto_if_eq VAR_RESULT, FALSE, VermilionPort_EventScript_Sailor_NoCredentials
	msgbox VermilionPort_Text_BoardSSAnne, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	warp MAP_SSANNE_1F_CORRIDOR, 19, 2
	waitstate
	release
	end

VermilionPort_EventScript_Sailor_SSAnneComplete::
	msgbox VermilionPort_Text_SSAnneComplete, MSGBOX_DEFAULT
	closemessage
	release
	end

VermilionPort_EventScript_Sailor_Cancel::
	closemessage
	release
	end
#else
	goto_if_ge VAR_SSAQUA_STATE, 8, VermilionPort_EventScript_Sailor_AfterKanto
	goto VermilionPort_EventScript_SailorCantSail

VermilionPort_EventScript_SailorCantSail::
	msgbox VermilionPort_Text_SailorCantSail, MSGBOX_NPC
	closemessage
	release
	end

VermilionPort_EventScript_Sailor_AfterKanto::
	message VermilionPort_Text_WhereSail
	multichoice 20, 0, MULTI_VERMILION_HARBOR, FALSE
	switch VAR_RESULT
	case 0, VermilionPort_EventScript_ChoseOlivine
	case 1, VermilionPort_EventScript_ChoseSouthernIsland
	case 2, VermilionPort_EventScript_ChoseBirthIsland
	case 3, VermilionPort_EventScript_ChoseFarawayIsland
	case 4, VermilionPort_EventScript_ChoseBattleFrontier
	case 5, VermilionPort_EventScript_Sailor_Refused
	case MULTI_B_PRESSED, VermilionPort_EventScript_Sailor_Refused
	release
	end

VermilionPort_EventScript_ChoseOlivine::
	closemessage
	delay 20
	checkitem ITEM_SS_TICKET
	bufferitemname STR_VAR_1, ITEM_SS_TICKET
	goto_if_eq VAR_RESULT, FALSE, VermilionPort_EventScript_Sailor_NoCredentials
	msgbox VermilionPort_Text_SailorGetOnBoard, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	setrespawn HEAL_LOCATION_OLIVINE_CITY_HNS
	warp MAP_OLIVINE_CITY_PORT_INSIDE_HNS, 8, 16
	waitstate
	release
	end

VermilionPort_EventScript_ChoseSouthernIsland::
	closemessage
	delay 20
	checkitem ITEM_EON_TICKET
	bufferitemname STR_VAR_1, ITEM_EON_TICKET
	goto_if_eq VAR_RESULT, FALSE, VermilionPort_EventScript_Sailor_NoCredentials
	call VermilionPort_EventScript_EnterShip
	warpsilent MAP_SOUTHERN_ISLAND_EXTERIOR_HNS, 13, 22
	waitstate
	release
	end

VermilionPort_EventScript_ChoseBirthIsland::
	closemessage
	delay 20
	checkitem ITEM_AURORA_TICKET
	bufferitemname STR_VAR_1, ITEM_AURORA_TICKET
	goto_if_eq VAR_RESULT, FALSE, VermilionPort_EventScript_Sailor_NoCredentials
	call VermilionPort_EventScript_EnterShip
	warpsilent MAP_BIRTH_ISLAND_EXTERIOR_HNS, 13, 23
	waitstate
	release
	end

VermilionPort_EventScript_ChoseFarawayIsland::
	closemessage
	delay 20
	checkitem ITEM_OLD_SEA_MAP
	bufferitemname STR_VAR_1, ITEM_OLD_SEA_MAP
	goto_if_eq VAR_RESULT, FALSE, VermilionPort_EventScript_Sailor_NoCredentials
	call VermilionPort_EventScript_EnterShip
	warpsilent MAP_FARAWAY_ISLAND_ENTRANCE_HNS, 13, 38
	waitstate
	release
	end	

VermilionPort_EventScript_ChoseBattleFrontier::
	closemessage
	delay 20
	call VermilionPort_EventScript_EnterShip
	warpsilent MAP_BATTLE_FRONTIER_OUTSIDE_WEST_HNS, 20, 67
	waitstate
	release
	end	

VermilionPort_EventScript_EnterShip::
	msgbox VermilionPort_Text_SailorGetOnBoard, MSGBOX_DEFAULT
	closemessage
	delay 20
	applymovement LOCALID_VERMILIONMARINA_SAILOR, VermilionPort_Movement_SailorEnterBoat
	waitmovement 0
	playse SE_EXIT
	applymovement OBJ_EVENT_ID_PLAYER, VermilionPort_Movement_PlayerEnterBoat
	waitmovement 0
	special SpawnCameraObject
	playse SE_EXIT
	removeobject OBJ_EVENT_ID_PLAYER
	delay 60
	applymovement LOCALID_VERMILIONMARINA_SAILOR, VermilionPort_Movement_BoatLeave
	applymovement LOCALID_VERMILIONMARINA_BOAT, VermilionPort_Movement_BoatLeave
	waitmovement LOCALID_VERMILIONMARINA_BOAT
	setrespawn HEAL_LOCATION_VERMILION_CITY_HNS
	return
#endif

VermilionPort_EventScript_Sailor_Refused::
	msgbox VermilionPort_Text_ComeAgain, MSGBOX_DEFAULT
	closemessage
	release
	end

VermilionPort_EventScript_Sailor_NoCredentials::
	msgbox VermilionPort_Text_NoPass, MSGBOX_DEFAULT
	closemessage
	release
	end

#if IS_WAYFARER
VermilionPort_Text_BoardSSAnne:
	.string "We'll take you aboard the\n"
	.string "S.S. ANNE.$"

VermilionPort_Text_SSAnneComplete:
	.string "The S.S. ANNE's visiting hours are\n"
	.string "over. Thank you for your visit.$"
#endif

VermilionPort_Movement_SailorEnterBoat:
	walk_down
	walk_down
	step_end

VermilionPort_Movement_PlayerEnterBoat:
	walk_down
	walk_down
	walk_in_place_down
	step_end

VermilionPort_Movement_BoatLeave:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

VermilionPort_Text_SailorCantSail:
	.string "The S.S.AQUA isn't ready to sail.$"

VermilionPort_Text_NoPass:
    .string "{PLAYER} tried to show the\n"
    .string "{STR_VAR_1}…\p"
    .string "…But no {STR_VAR_1}!\p"
    .string "Sorry!\n"
    .string "You may board only if you have an\l"
    .string "{STR_VAR_1}.$"

VermilionPort_Text_SailorGetOnBoard:
    .string "We're departing soon.\n"
    .string "Please get on board.$"

VermilionPort_Text_CantBoard:
    .string "Sorry. You can't board now.$"

VermilionPort_Text_WhereSail:
    .string "Where would you like to sail?$"

VermilionPort_Text_AskBoard:
    .string "Welcome to FAST SHIP S.S.AQUA.\p"
    .string "Will you be boarding today?$"

VermilionPort_Text_AskTicket:
    .string "May I see your S.S.TICKET?$"

VermilionPort_Text_ComeAgain:
    .string "We hope to see you again!$"

VermilionPort_Text_FlashTicket:
    .string "{PLAYER} flashed the S.S.TICKET.\p"
    .string "That's it. Thank you!$"

VermilionPort_Text_NoTicket:
    .string "{PLAYER} tried to show the\n"
    .string "S.S.TICKET…\p"
    .string "…But no TICKET!\p"
    .string "Sorry!\n"
    .string "You may board only\l"
    .string "if you have an S.S.TICKET.$"

#if IS_WAYFARER
VermilionPort_EventScript_DepartureBoard::
	msgbox VermilionPort_Text_DepartureBoard, MSGBOX_SIGN
	end

VermilionPort_Text_DepartureBoard:
	.string "DEPARTURES - VERMILION CITY\p"
	.string "By day (8:00-20:00):\n"
	.string "S.S. AQUA for SLATEPORT CITY\p"
	.string "By night (20:00-8:00):\n"
	.string "S.S. ANNE for ONE ISLAND$"
#endif
