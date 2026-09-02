VermilionCity_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VermilionCity_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, VermilionCity_OnFrame
	.byte 0

VermilionCity_OnFrame::
	map_script_2 VAR_MAP_SCENE_VERMILION_CITY, 2, VermilionCity_EventScript_ExitSSAnne
	.2byte 0

VermilionCity_EventScript_ExitSSAnne::
	lockall
	applymovement LOCALID_PLAYER, VermilionCity_Movement_ExitSSAnne
	waitmovement 0
	setvar VAR_MAP_SCENE_VERMILION_CITY, 3
	releaseall
	end

VermilionCity_Movement_ExitSSAnne::
	walk_up
	walk_up
	step_end

VermilionCity_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_VERMILION_CITY
	call_if_set FLAG_TALKED_TO_OAKS_AIDE_IN_VERMILION, VermilionCity_EventScript_HideOaksAide
	end

VermilionCity_EventScript_HideOaksAide::
	setflag FLAG_HIDE_VERMILION_CITY_OAKS_AIDE
	return

VermilionCity_EventScript_Woman::
	msgbox VermilionCity_Text_GrimerMultipliesInSludge, MSGBOX_NPC
	end

VermilionCity_EventScript_OldMan1::
	lock
	faceplayer
	goto_if_eq VAR_MAP_SCENE_VERMILION_CITY, 3, VermilionCity_EventScript_OldMan1SSAnneLeft
	msgbox VermilionCity_Text_DidYouSeeSSAnneInHarbor
	release
	end

VermilionCity_EventScript_OldMan1SSAnneLeft::
	msgbox VermilionCity_Text_SSAnneHasDepartedForYear
	release
	end

VermilionCity_EventScript_FerrySailor::
	lock
	faceplayer
	call VermilionCity_EventScript_CheckShakedownPass
	goto_if_eq VAR_RESULT, TRUE, VermilionCity_EventScript_SharedDockDispatch
	goto_if_eq VAR_MAP_SCENE_VERMILION_CITY, 3, VermilionCity_EventScript_CheckSeagallopPresent
	msgbox VermilionCity_Text_WelcomeToTheSSAnne
	release
	end

VermilionCity_EventScript_CheckShakedownPass::
	checkitem ITEM_RAINBOW_PASS
	goto_if_eq VAR_RESULT, FALSE, EventScript_SetResultFalse
	goto_if_unset FLAG_SEVII_SHAKEDOWN_COMPLETE, EventScript_SetResultFalse
	goto EventScript_SetResultTrue
	end

VermilionCity_EventScript_SharedDockDispatch::
	message VermilionCity_Text_WhichService
	waitmessage
	goto_if_eq VAR_MAP_SCENE_VERMILION_CITY, 3, VermilionCity_EventScript_SharedDockSeviiOnly
	multichoice 13, 6, MULTI_VERMILION_SHARED_DOCK, FALSE
	switch VAR_RESULT
	case 0, VermilionCity_EventScript_CheckSSTicket
	case 1, VermilionCity_EventScript_ChooseSeviiService
	case 2, EventScript_CancelSail
	case MULTI_B_PRESSED, EventScript_CancelSail
	end
VermilionCity_EventScript_SharedDockSeviiOnly::
	multichoice 13, 6, MULTI_VERMILION_SEVII_DOCK, FALSE
	switch VAR_RESULT
	case 0, VermilionCity_EventScript_ChooseSeviiService
	case 1, EventScript_CancelSail
	case MULTI_B_PRESSED, EventScript_CancelSail
	end

VermilionCity_EventScript_ChooseSeviiService::
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	goto_if_ge VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 5, VermilionCity_EventScript_ChooseSeagallopDestRainbowPass
	goto_if_eq VAR_MAP_SCENE_ONE_ISLAND_HARBOR, 5, VermilionCity_EventScript_ReturnToPendingSeviiIntro
	goto_if_unset FLAG_SEVII_TRAVEL_INTRO_SEEN, VermilionCity_EventScript_FirstSeviiTrip
	goto VermilionCity_EventScript_ChooseSeagallopDestRainbowPass
	end

VermilionCity_EventScript_ReturnToPendingSeviiIntro::
	setvar VAR_0x8006, SEAGALLOP_ONE_ISLAND
	goto EventScript_SailToDest
	end

VermilionCity_EventScript_FirstSeviiTrip::
	msgbox VermilionCity_Text_FirstSeviiTrip
	setflag FLAG_HIDE_ONE_ISLAND_BILL
	setflag FLAG_HIDE_ONE_ISLAND_POKECENTER_BILL
	setvar VAR_MAP_SCENE_ONE_ISLAND_HARBOR, 4
	setvar VAR_0x8006, SEAGALLOP_ONE_ISLAND
	goto EventScript_SailToDest
	end

VermilionCity_EventScript_CheckHasMysticTicket::
	goto_if_unset FLAG_ENABLE_SHIP_NAVEL_ROCK, EventScript_SetResultFalse
	checkitem ITEM_MYSTIC_TICKET
	goto_if_eq VAR_RESULT, FALSE, EventScript_SetResultFalse
	goto EventScript_SetResultTrue
	end

VermilionCity_EventScript_CheckHasAuroraTicket::
	goto_if_unset FLAG_ENABLE_SHIP_BIRTH_ISLAND, EventScript_SetResultFalse
	checkitem ITEM_AURORA_TICKET
	goto_if_eq VAR_RESULT, FALSE, EventScript_SetResultFalse
	goto EventScript_SetResultTrue
	end

VermilionCity_EventScript_CheckSeagallopPresent::
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	checkitem ITEM_RAINBOW_PASS
	goto_if_eq VAR_RESULT, FALSE, VermilionCity_EventScript_CheckSeagallopPresentOriginal
	goto_if_ge VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 5, VermilionCity_EventScript_ChooseSeagallopDestRainbowPass

VermilionCity_EventScript_CheckSeagallopPresentOriginal::
	goto_if_ge VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 1, VermilionCity_EventScript_ChooseSeagallopDestTriPass
	msgbox VermilionCity_Text_TheShipSetSail
	release
	end

VermilionCity_EventScript_ChooseSeagallopDestRainbowPass::
	call VermilionCity_EventScript_CheckHasMysticTicket
	goto_if_eq VAR_RESULT, TRUE, VermilionCity_EventScript_HasMysticTicket
	call VermilionCity_EventScript_CheckHasAuroraTicket
	goto_if_eq VAR_RESULT, TRUE, VermilionCity_EventScript_HasAuroraTicket
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	message VermilionCity_Text_BoardSeagallopRainbowPass
	waitmessage
	goto EventScript_SeviiDestinationsPage1
	end

VermilionCity_EventScript_HasMysticTicket::
	call VermilionCity_EventScript_CheckHasAuroraTicket
	goto_if_eq VAR_RESULT, TRUE, VermilionCity_EventScript_HasMysticAndAuroraTickets
	call_if_unset FLAG_SHOWN_MYSTIC_TICKET, VermilionCity_EventScript_ShowMysticTicket
	message VermilionCity_Text_BoardSeagallopFerry
	waitmessage
	multichoice 17, 6, MULTI_SEVII_NAVEL, FALSE
	switch VAR_RESULT
	case 0, EventScript_SeviiDestinationsPage1
	case 1, EventScript_SailToNavelRock
	case 2, EventScript_CancelSail
	case 127, EventScript_CancelSail
	end

VermilionCity_EventScript_ShowMysticTicket::
	setflag FLAG_SHOWN_MYSTIC_TICKET
	msgbox VermilionCity_Text_OhMysticTicketTakeYouToNavelRock
	return

VermilionCity_EventScript_HasAuroraTicket::
	call_if_unset FLAG_SHOWN_AURORA_TICKET, VermilionCity_EventScript_ShowAuroraTicket
	message VermilionCity_Text_BoardSeagallopFerry
	waitmessage
	multichoice 13, 6, MULTI_SEVII_BIRTH, FALSE
	switch VAR_RESULT
	case 0, EventScript_SeviiDestinationsPage1
	case 1, EventScript_SailToBirthIsland
	case 2, EventScript_CancelSail
	case 127, EventScript_CancelSail
	end

VermilionCity_EventScript_ShowAuroraTicket::
	setflag FLAG_SHOWN_AURORA_TICKET
	msgbox VermilionCity_Text_OhAuroraTicketTakeYouToBirthIsland
	return

VermilionCity_EventScript_HasMysticAndAuroraTickets::
	call_if_unset FLAG_SHOWN_MYSTIC_TICKET, VermilionCity_EventScript_ShowMysticTicket
	call_if_unset FLAG_SHOWN_AURORA_TICKET, VermilionCity_EventScript_ShowAuroraTicket
	message VermilionCity_Text_BoardSeagallopFerry
	waitmessage
	multichoice 13, 5, MULTI_SEVII_NAVEL_BIRTH, FALSE
	switch VAR_RESULT
	case 0, EventScript_SeviiDestinationsPage1
	case 1, EventScript_SailToNavelRock
	case 2, EventScript_SailToBirthIsland
	case 3, EventScript_CancelSail
	case 127, EventScript_CancelSail
	end

EventScript_SailToNavelRock::
	msgbox VermilionCity_Text_Seagallop10Departing
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	setvar VAR_0x8006, SEAGALLOP_NAVEL_ROCK
	goto EventScript_SetSail
	end

EventScript_SailToBirthIsland::
	msgbox VermilionCity_Text_Seagallop12Departing
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	setvar VAR_0x8006, SEAGALLOP_BIRTH_ISLAND
	goto EventScript_SetSail
	end

VermilionCity_EventScript_ChooseSeagallopDestTriPass::
	message VermilionCity_Text_BoardSeagallopTriPass
	waitmessage
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	multichoice 19, 5, MULTI_SEAGALLOP_123, FALSE
	switch VAR_RESULT
	case 0, EventScript_SailToOneIsland2
	case 1, EventScript_SailToTwoIsland2
	case 2, EventScript_SailToThreeIsland2
	case 3, EventScript_CancelSail
	case 127, EventScript_CancelSail
	end

@ Unreachable
Vermilion_EventScript_Unused::
	release
	end

VermilionCity_EventScript_CheckTicketLeft::
	lockall
	goto VermilionCity_EventScript_CheckTicket
	end

VermilionCity_EventScript_CheckTicketRight::
	lockall
	goto VermilionCity_EventScript_CheckTicket
	end

VermilionCity_EventScript_ExitedTicketCheck::
	lockall
	setvar VAR_VERMILION_CITY_TICKET_CHECK_TRIGGER, 0
	releaseall
	end

VermilionCity_EventScript_CheckTicket::
	textcolor NPC_TEXT_COLOR_MALE
	call VermilionCity_EventScript_CheckShakedownPass
	goto_if_eq VAR_RESULT, TRUE, VermilionCity_EventScript_SharedDockDispatch

VermilionCity_EventScript_CheckSSTicket::
	textcolor NPC_TEXT_COLOR_MALE
	applymovement LOCALID_VERMILION_FERRY_SAILOR, Common_Movement_WalkInPlaceFasterLeft
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	goto_if_eq VAR_MAP_SCENE_VERMILION_CITY, 3, VermilionCity_EventScript_CheckSeagallopPresentTrigger
	msgbox VermilionCity_Text_DoYouHaveATicket
	goto_if_unset FLAG_GOT_SS_TICKET, VermilionCity_EventScript_DontHaveSSTicket
	msgbox VermilionCity_Text_FlashedSSTicket
	setvar VAR_VERMILION_CITY_TICKET_CHECK_TRIGGER, 1
	releaseall
	end

VermilionCity_EventScript_DontHaveSSTicket::
	msgbox VermilionCity_Text_DontHaveNeededSSTicket
	closemessage
	goto VermilionCity_EventScript_WalkUpPier
	end

VermilionCity_EventScript_CheckSeagallopPresentTrigger::
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	checkitem ITEM_RAINBOW_PASS
	goto_if_eq VAR_RESULT, FALSE, VermilionCity_EventScript_CheckSeagallopPresentTriggerOriginal
	goto_if_ge VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 5, VermilionCity_EventScript_ChooseSeagallopDestRainbowPass

VermilionCity_EventScript_CheckSeagallopPresentTriggerOriginal::
	goto_if_ge VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 1, VermilionCity_EventScript_ChooseSeagallopDestTriPass
	msgbox VermilionCity_Text_TheShipSetSail
	closemessage
	goto VermilionCity_EventScript_WalkUpPier
	end

VermilionCity_EventScript_Sailor::
	msgbox VermilionCity_Text_SSAnneVisitsOnceAYear, MSGBOX_NPC
	end

VermilionCity_EventScript_OaksAide::
	lock
	faceplayer
	setflag FLAG_TALKED_TO_OAKS_AIDE_IN_VERMILION
	msgbox VermilionCity_Text_Route2AideHasPackageForYou
	release
	end

VermilionCity_EventScript_PortBuilder::
	lock
	faceplayer
	checkitem ITEM_RAINBOW_PASS
	goto_if_eq VAR_RESULT, TRUE, VermilionCity_EventScript_PortBuilderHasPassState
	goto_if_set FLAG_SEVII_SHAKEDOWN_COMPLETE, VermilionCity_EventScript_ShakedownReward
	goto VermilionCity_EventScript_PortBuilderIncomplete
	end

VermilionCity_EventScript_PortBuilderHasPassState::
	goto_if_set FLAG_SEVII_SHAKEDOWN_COMPLETE, VermilionCity_EventScript_ShakedownCompleted

VermilionCity_EventScript_PortBuilderIncomplete::
	goto_if_unset FLAG_SEVII_SHAKEDOWN_STARTED, VermilionCity_EventScript_OfferShakedown
	setvar VAR_0x8004, 3
	call_if_set FLAG_SEVII_SHAKEDOWN_SPOT_1, VermilionCity_EventScript_CountShakedownSpot
	call_if_set FLAG_SEVII_SHAKEDOWN_SPOT_2, VermilionCity_EventScript_CountShakedownSpot
	call_if_set FLAG_SEVII_SHAKEDOWN_SPOT_3, VermilionCity_EventScript_CountShakedownSpot
	goto_if_ne VAR_0x8004, 0, VermilionCity_EventScript_ShakedownReportRemaining
	checkitem ITEM_RAINBOW_PASS
	goto_if_eq VAR_RESULT, TRUE, VermilionCity_EventScript_PortBuilderHasRainbowPass
	goto VermilionCity_EventScript_ShakedownReward
	end

VermilionCity_EventScript_ShakedownReportRemaining::
	buffernumberstring STR_VAR_1, VAR_0x8004
	msgbox VermilionCity_Text_ShakedownSpotsRemaining
	release
	end

VermilionCity_EventScript_CountShakedownSpot::
	subvar VAR_0x8004, 1
	return

VermilionCity_EventScript_OfferShakedown::
	message VermilionCity_Text_OfferShakedown
	waitmessage
	multichoice 18, 8, MULTI_SEVII_SHAKEDOWN_START, FALSE
	goto_if_ne VAR_RESULT, 0, VermilionCity_EventScript_DeclineShakedown
	setflag FLAG_SEVII_SHAKEDOWN_STARTED
	msgbox VermilionCity_Text_ShakedownAccepted
	release
	end

VermilionCity_EventScript_DeclineShakedown::
	msgbox VermilionCity_Text_ShakedownDeclined
	release
	end

VermilionCity_EventScript_PortBuilderHasRainbowPass::
	goto_if_set FLAG_SEVII_SHAKEDOWN_COMPLETE, VermilionCity_EventScript_ShakedownCompleted
	setflag FLAG_SEVII_SHAKEDOWN_COMPLETE
	call VermilionCity_EventScript_ReconcileSeviiIntroduction
	msgbox VermilionCity_Text_ShakedownPassReconciled
	release
	end

VermilionCity_EventScript_ShakedownReward::
	checkitemspace ITEM_RAINBOW_PASS
	goto_if_eq VAR_RESULT, FALSE, VermilionCity_EventScript_ShakedownRewardNoRoom
	additem ITEM_RAINBOW_PASS
	goto_if_eq VAR_RESULT, FALSE, VermilionCity_EventScript_ShakedownRewardNoRoom
	checkitem ITEM_RAINBOW_PASS
	goto_if_eq VAR_RESULT, FALSE, VermilionCity_EventScript_ShakedownRewardNoRoom
	msgreceiveditem VermilionCity_Text_ReceivedRainbowPass, ITEM_RAINBOW_PASS, 1, MUS_RG_OBTAIN_KEY_ITEM
	setflag FLAG_SEVII_SHAKEDOWN_COMPLETE
	call VermilionCity_EventScript_ReconcileSeviiIntroduction
	msgbox VermilionCity_Text_ShakedownCertified
	release
	end

VermilionCity_EventScript_ShakedownRewardNoRoom::
	msgbox VermilionCity_Text_ShakedownNoRoom
	release
	end

VermilionCity_EventScript_ShakedownCompleted::
	call VermilionCity_EventScript_ReconcileSeviiIntroduction
	msgbox VermilionCity_Text_ShakedownCompleted
	release
	end

VermilionCity_EventScript_ReconcileSeviiIntroduction::
	goto_if_lt VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 1, Common_EventScript_NopReturn
	setflag FLAG_SYS_SEVII_MAP_123
	setflag FLAG_SYS_SEVII_MAP_4567
	setflag FLAG_SEVII_TRAVEL_INTRO_SEEN
	return

VermilionCity_EventScript_ShakedownSpot1::
	lockall
	goto_if_unset FLAG_SEVII_SHAKEDOWN_STARTED, VermilionCity_EventScript_ShakedownSpotNotStarted
	goto_if_set FLAG_SEVII_SHAKEDOWN_SPOT_1, VermilionCity_EventScript_ShakedownSpotComplete
	msgbox VermilionCity_Text_ShakedownSpot1Soft
	closemessage
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopToSpot1
	waitmovement 0
	playse SE_M_STRENGTH
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopTampDown
	waitmovement 0
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopFromSpot1
	waitmovement 0
	setflag FLAG_SEVII_SHAKEDOWN_SPOT_1
	msgbox VermilionCity_Text_ShakedownSpotFinished
	releaseall
	end

VermilionCity_EventScript_ShakedownSpot2::
	lockall
	goto_if_unset FLAG_SEVII_SHAKEDOWN_STARTED, VermilionCity_EventScript_ShakedownSpotNotStarted
	goto_if_set FLAG_SEVII_SHAKEDOWN_SPOT_2, VermilionCity_EventScript_ShakedownSpotComplete
	msgbox VermilionCity_Text_ShakedownSpot2Soft
	closemessage
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopToSpot2
	waitmovement 0
	playse SE_M_STRENGTH
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopTampDown
	waitmovement 0
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopFromSpot2
	waitmovement 0
	setflag FLAG_SEVII_SHAKEDOWN_SPOT_2
	msgbox VermilionCity_Text_ShakedownSpotFinished
	releaseall
	end

VermilionCity_EventScript_ShakedownSpot3::
	lockall
	goto_if_unset FLAG_SEVII_SHAKEDOWN_STARTED, VermilionCity_EventScript_ShakedownSpotNotStarted
	goto_if_set FLAG_SEVII_SHAKEDOWN_SPOT_3, VermilionCity_EventScript_ShakedownSpotComplete
	msgbox VermilionCity_Text_ShakedownSpot3Soft
	closemessage
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopToSpot3
	waitmovement 0
	playse SE_M_STRENGTH
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopTampUp
	waitmovement 0
	applymovement LOCALID_VERMILION_SHAKEDOWN_MACHOP, VermilionCity_Movement_MachopFromSpot3
	waitmovement 0
	setflag FLAG_SEVII_SHAKEDOWN_SPOT_3
	msgbox VermilionCity_Text_ShakedownSpotFinished
	releaseall
	end

VermilionCity_EventScript_ShakedownSpotNotStarted::
	msgbox VermilionCity_Text_ShakedownSpotMarked
	releaseall
	end

VermilionCity_EventScript_ShakedownSpotComplete::
	msgbox VermilionCity_Text_ShakedownSpotFirm
	releaseall
	end

VermilionCity_Movement_MachopToSpot1::
	walk_up
	walk_up
	walk_left
	walk_left
	step_end

VermilionCity_Movement_MachopFromSpot1::
	walk_right
	walk_right
	walk_down
	walk_down
	face_down
	step_end

VermilionCity_Movement_MachopToSpot2::
	walk_up
	walk_up
	walk_right
	walk_right
	step_end

VermilionCity_Movement_MachopFromSpot2::
	walk_left
	walk_left
	walk_down
	walk_down
	face_down
	step_end

VermilionCity_Movement_MachopToSpot3::
	walk_down
	walk_down
	walk_right
	walk_right
	step_end

VermilionCity_Movement_MachopFromSpot3::
	walk_left
	walk_left
	walk_up
	walk_up
	face_down
	step_end

VermilionCity_Movement_MachopTampDown::
	jump_in_place_down
	jump_in_place_down
	step_end

VermilionCity_Movement_MachopTampUp::
	jump_in_place_up
	jump_in_place_up
	step_end

VermilionCity_EventScript_Machop::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MACHOP, CRY_MODE_NORMAL
	msgbox VermilionCity_Text_Machop
	waitmoncry
	msgbox VermilionCity_Text_MachopStompingLandFlat
	release
	end

VermilionCity_EventScript_CitySign::
	msgbox VermilionCity_Text_CitySign, MSGBOX_SIGN
	end

VermilionCity_EventScript_SnorlaxNotice::
	msgbox VermilionCity_Text_SnorlaxBlockingRoute12, MSGBOX_SIGN
	end

VermilionCity_EventScript_PokemonFanClubSign::
	msgbox VermilionCity_Text_PokemonFanClubSign, MSGBOX_SIGN
	end

VermilionCity_EventScript_GymSign::
	lockall
	famechecker FAMECHECKER_LTSURGE, 0
	msgbox VermilionCity_Text_GymSign
	releaseall
	end

VermilionCity_EventScript_HarborSign::
	msgbox VermilionCity_Text_VermilionHarbor, MSGBOX_SIGN
	end

VermilionCity_Text_GrimerMultipliesInSludge::
	.string "We're careful about pollution here.\p"
	.string "We've heard GRIMER multiplies in\n"
	.string "toxic sludge.$"

VermilionCity_Text_DidYouSeeSSAnneInHarbor::
	.string "Did you see the S.S. ANNE moored\n"
	.string "in the harbor?$"

VermilionCity_Text_SSAnneHasDepartedForYear::
	.string "So, the S.S. ANNE has departed?\p"
	.string "She'll be back in VERMILION about\n"
	.string "this time next year.$"

VermilionCity_Text_BuildingOnThisLand::
	.string "I'm putting up a building on this\n"
	.string "plot of land. I own it all.\p"
	.string "My POKéMON is tamping the land\n"
	.string "to lay the foundation.$"

VermilionCity_Text_OfferShakedown::
	.string "I'm checking this lot for soft\n"
	.string "soil.\p"
	.string "Inspect the three marked spots,\n"
	.string "and MACHOP will tamp them down.\p"
	.string "Will you start the inspection?$"

VermilionCity_Text_ShakedownAccepted::
	.string "Good! Check the marked spots in\n"
	.string "any order, then report back.$"

VermilionCity_Text_ShakedownDeclined::
	.string "All right. The job will be here\n"
	.string "when you're ready.$"

VermilionCity_Text_ShakedownSpotsRemaining::
	.string "{STR_VAR_1} marked spots still\n"
	.string "need work.\p"
	.string "Check them, then report back.$"

VermilionCity_Text_ReceivedRainbowPass::
	.string "{PLAYER} received the RAINBOW PASS\n"
	.string "from the ferry sailor.$"

VermilionCity_Text_ShakedownCertified::
	.string "The ferry sailor certified\n"
	.string "the pier!\p"
	.string "This dock now serves the\n"
	.string "SEVII ISLANDS.$"

VermilionCity_Text_ShakedownNoRoom::
	.string "Your KEY ITEMS POCKET is full.\p"
	.string "Make room, then report back.$"

VermilionCity_Text_ShakedownPassReconciled::
	.string "You already have the RAINBOW PASS.\p"
	.string "The ferry sailor certified the\n"
	.string "pier. SEVII service is ready.$"

VermilionCity_Text_ShakedownCompleted::
	.string "The pier is firm and certified.\p"
	.string "SEVII service is ready\n"
	.string "at the dock.$"

VermilionCity_Text_ShakedownSpotMarked::
	.string "A numbered stake marks soft soil.\n"
	.string "The builder is watching the lot.$"

VermilionCity_Text_ShakedownSpot1Soft::
	.string "Spot 1 is soft.\n"
	.string "MACHOP comes to tamp it down!$"

VermilionCity_Text_ShakedownSpot2Soft::
	.string "Spot 2 is soft.\n"
	.string "MACHOP comes to tamp it down!$"

VermilionCity_Text_ShakedownSpot3Soft::
	.string "Spot 3 is soft.\n"
	.string "MACHOP comes to tamp it down!$"

VermilionCity_Text_ShakedownSpotFinished::
	.string "The soil is packed firm.$"

VermilionCity_Text_ShakedownSpotFirm::
	.string "MACHOP already packed this spot\n"
	.string "firm.$"

VermilionCity_Text_Machop::
	.string "MACHOP: Guoh! Gogogoh!$"

VermilionCity_Text_MachopStompingLandFlat::
	.string "A MACHOP is stomping the land flat.$"

VermilionCity_Text_SSAnneVisitsOnceAYear::
	.string "The S.S. ANNE is a famous luxury\n"
	.string "cruise ship.\p"
	.string "It visits VERMILION once a year.$"

VermilionCity_Text_CitySign::
	.string "VERMILION CITY\n"
	.string "The Port of Exquisite Sunsets$"

VermilionCity_Text_SnorlaxBlockingRoute12::
	.string "NOTICE!\p"
	.string "ROUTE 12 may be blocked off by a\n"
	.string "sleeping POKéMON.\p"
	.string "Detour through ROCK TUNNEL to\n"
	.string "LAVENDER TOWN.\p"
	.string "VERMILION POLICE$"

VermilionCity_Text_PokemonFanClubSign::
	.string "POKéMON FAN CLUB\n"
	.string "All POKéMON fans welcome!$"

VermilionCity_Text_GymSign::
	.string "VERMILION CITY POKéMON GYM\n"
	.string "LEADER: LT. SURGE\l"
	.string "The Lightning American!$"

VermilionCity_Text_VermilionHarbor::
	.string "VERMILION HARBOR$"

VermilionCity_Text_WelcomeToTheSSAnne::
	.string "Welcome to the S.S. ANNE!$"

VermilionCity_Text_WhichService::
	.string "Which service would you like?$"

VermilionCity_Text_FirstSeviiTrip::
	.string "Your first SEVII stop is\n"
	.string "ONE ISLAND.\p"
	.string "CELIO will explain the ferry service\n"
	.string "there.$"

VermilionCity_Text_DoYouHaveATicket::
	.string "Welcome to the S.S. ANNE!\p"
	.string "Excuse me, do you have a ticket?$"

VermilionCity_Text_FlashedSSTicket::
	.string "{FONT_NORMAL}{PLAYER} flashed the S.S. TICKET!\p"
	.string "{FONT_MALE}Great!\n"
	.string "Welcome to the S.S. ANNE!$"

VermilionCity_Text_DontHaveNeededSSTicket::
	.string "{FONT_NORMAL}{PLAYER} doesn't have the needed\n"
	.string "S.S. TICKET.\p"
	.string "{FONT_MALE}Sorry!\p"
	.string "You need a ticket to get aboard.$"

VermilionCity_Text_TheShipSetSail::
	.string "The ship set sail.$"

VermilionCity_Text_BoardSeagallopTriPass::
	.string "Ah, you have a TRI-PASS.\p"
	.string "Would you like to board\n"
	.string "a SEAGALLOP ferry?$"

VermilionCity_Text_Seagallop7Departing::
	.string "Okay, everything's in order.\p"
	.string "SEAGALLOP HI-SPEED 7 will be\n"
	.string "departing immediately.$"

VermilionCity_Text_BoardSeagallopRainbowPass::
	.string "Ah, you have a RAINBOW PASS.\p"
	.string "Would you like to board\n"
	.string "a SEAGALLOP ferry?$"

VermilionCity_Text_OhMysticTicketTakeYouToNavelRock::
	.string "Oh! That's a MYSTICTICKET!\n"
	.string "Now that is rare.\p"
	.string "We'll be happy to take you to\n"
	.string "NAVEL ROCK anytime.$"

VermilionCity_Text_OhAuroraTicketTakeYouToBirthIsland::
	.string "Oh! That's an AURORATICKET!\n"
	.string "Now that is rare.\p"
	.string "We'll be happy to take you to\n"
	.string "BIRTH ISLAND anytime.$"

VermilionCity_Text_BoardSeagallopFerry::
	.string "Would you like to board\n"
	.string "a SEAGALLOP ferry?$"

VermilionCity_Text_Seagallop10Departing::
	.string "Okay, everything's in order for you\n"
	.string "to board a special ferry.\p"
	.string "SEAGALLOP HI-SPEED 10 will be\n"
	.string "departing immediately.$"

VermilionCity_Text_Seagallop12Departing::
	.string "Okay, everything's in order for you\n"
	.string "to board a special ferry.\p"
	.string "SEAGALLOP HI-SPEED 12 will be\n"
	.string "departing immediately.$"

VermilionCity_Text_Route2AideHasPackageForYou::
	.string "Oh, hello, {PLAYER}!\n"
	.string "How are you doing?\p"
	.string "It's me, one of PROF. OAK's AIDES.\p"
	.string "Did you meet the other AIDE?\p"
	.string "He had a package from PROF. OAK\n"
	.string "for you, {PLAYER}.\p"
	.string "He said he'd look for you around\n"
	.string "ROUTE 2, {PLAYER}.\p"
	.string "If you're in the ROUTE 2 area,\n"
	.string "please look for him.$"
