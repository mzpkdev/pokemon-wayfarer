PalletTown_Lab_hns_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PalletTown_Lab_KantoOnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PalletTown_Lab_KantoOnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PalletTown_Lab_KantoOnFrame
	.byte 0

@ The Pallet-origin opening ports FRLG's PalletTown_ProfessorOaksLab_Frlg.
@ The HNS lab is the FRLG lab translated by (+7, +8): Oak (13,11), Blue
@ (12,12), balls (15..17,12), exit lanes (12..14,16), door (13,20).
@ Progress lives in the Wayfarer opening state, never in FRLG scene vars.
@ FRLG {RIVAL} is transcribed as BLUE because HNS {RIVAL} prints SILVER.
#if IS_WAYFARER
PalletTown_Lab_KantoOnLoad:
	clearflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_4
	setflag FLAG_TEMP_5
	setflag FLAG_TEMP_6
	specialvar VAR_RESULT, WayfarerKanto_StageLab
	copyvar VAR_TEMP_1, VAR_RESULT
	end

PalletTown_Lab_KantoOnWarp:
	map_script_2 VAR_TEMP_1, TRUE, PalletTown_Lab_EventScript_KantoReadyPlayerForArrival
	.2byte 0

PalletTown_Lab_EventScript_KantoReadyPlayerForArrival:
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

PalletTown_Lab_KantoOnFrame:
	map_script_2 VAR_TEMP_1, TRUE, PalletTown_Lab_EventScript_KantoArrival
	.2byte 0

@ FRLG ChooseStarterScene. Oak's escort warps the player onto the door (13,20).
PalletTown_Lab_EventScript_KantoArrival:
	lockall
	setvar VAR_TEMP_1, FALSE
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_0x8004, 13, PalletTown_Lab_EventScript_KantoArrivalStaged
	goto_if_ne VAR_0x8005, 20, PalletTown_Lab_EventScript_KantoArrivalStaged
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, PalletTown_Lab_Movement_KantoOakEnter
	waitmovement 0
	call PalletTown_Lab_EventScript_KantoPlaceOakAtDesk
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoPlayerEnter
	waitmovement 0
	goto PalletTown_Lab_EventScript_KantoArrivalExchange

@ Defensive: an arrival away from the door skips the walk-in, not the exchange.
PalletTown_Lab_EventScript_KantoArrivalStaged:
	call PalletTown_Lab_EventScript_KantoPlaceOakAtDesk
PalletTown_Lab_EventScript_KantoArrivalExchange:
	applymovement LOCALID_PALLET_KANTO_BLUE, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	msgbox PalletTown_Lab_Text_KantoRivalFedUpWithWaiting
	closemessage
	delay 60
	msgbox PalletTown_Lab_Text_KantoOakThreeMonsChooseOne
	closemessage
	delay 30
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalReact
	waitmovement 0
	msgbox PalletTown_Lab_Text_KantoRivalNoFairWhatAboutMe
	msgbox PalletTown_Lab_Text_KantoOakBePatientRival
	special WayfarerKanto_CommitLabStarterChoice
	releaseall
	end

PalletTown_Lab_EventScript_KantoPlaceOakAtDesk:
	removeobject LOCALID_PALLET_KANTO_LAB_OAK
	setobjectxyperm LOCALID_PALLET_KANTO_LAB_OAK, 13, 11
	setobjectmovementtype LOCALID_PALLET_KANTO_LAB_OAK, MOVEMENT_TYPE_FACE_DOWN
	clearflag FLAG_TEMP_1
	addobject LOCALID_PALLET_KANTO_LAB_OAK
	return

PalletTown_Lab_Movement_KantoOakEnter:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

PalletTown_Lab_Movement_KantoPlayerEnter:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

PalletTown_Lab_Movement_KantoRivalReact:
	walk_in_place_up
	walk_in_place_up
	step_end
#else
PalletTown_Lab_KantoOnLoad:
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_4
	setflag FLAG_TEMP_5
	setflag FLAG_TEMP_6
	end

PalletTown_Lab_KantoOnWarp:
PalletTown_Lab_KantoOnFrame:
	.2byte 0
#endif

PalletTown_Lab_EventScript_Oak::
	lock
	faceplayer
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoOak
#endif
	goto_if_eq VAR_PALLETTOWN_LABSTATE, 1, PalletTown_Lab_EventScript_OakBadgeCheck
	goto_if_eq VAR_PALLETTOWN_LABSTATE, 2, PalletTown_Lab_EventScript_OakDex
	msgbox PalletTown_Lab_Text_OakWelcomeKanto, MSGBOX_DEFAULT
	msgbox PalletTown_Lab_Text_OakLabGoodbye, MSGBOX_DEFAULT
	setvar VAR_PALLETTOWN_LABSTATE, 1
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
	specialvar VAR_RESULT, WayfarerGetKantoBadgeCountForScript
	goto_if_ge VAR_RESULT, 8, PalletTown_Lab_EventScript_Oak16Badges
#else
	goto_if_ge VAR_NUM_BADGES, 16, PalletTown_Lab_EventScript_OakBadgeCheck
#endif
	closemessage
	release
	end

#if IS_WAYFARER
@ FRLG EventScript_ProfOak, by opening phase.
PalletTown_Lab_EventScript_KantoOak::
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_ge VAR_RESULT, PALLET_OPENING_PARCEL_RECEIVED, PalletTown_Lab_EventScript_KantoParcel
	goto_if_eq VAR_RESULT, PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED, PalletTown_Lab_EventScript_KantoOakAfterBattle
	goto_if_eq VAR_RESULT, PALLET_OPENING_STARTER_RECEIVED, PalletTown_Lab_EventScript_KantoOakAfterStarter
	specialvar VAR_0x8004, WayfarerKanto_GetStarterSlot
	goto_if_ne VAR_0x8004, KANTO_STARTER_SLOT_NONE, PalletTown_Lab_EventScript_KantoGiveStarter
	msgbox PalletTown_Lab_Text_KantoOakWhichOneWillYouChoose
	release
	end

PalletTown_Lab_EventScript_KantoOakAfterStarter:
	msgbox PalletTown_Lab_Text_KantoOakCanReachNextTownWithMon
	release
	end

PalletTown_Lab_EventScript_KantoOakAfterBattle:
	msgbox PalletTown_Lab_Text_KantoOakBattleMonForItToGrow
	release
	end

@ FRLG ReceiveDexScene as a resumable transaction: each receipt is committed
@ once, and a retry resumes at the first unfinished step.
PalletTown_Lab_EventScript_KantoParcel::
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoComplete
	setvar VAR_0x8004, PALLET_RECEIPT_PARCEL_ACCEPTED
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoBlueArrival
	msgbox PalletTown_Lab_Text_KantoOakHaveSomethingForMe
	specialvar VAR_RESULT, WayfarerKanto_TryAcceptParcel
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoParcelMissing
	playfanfare MUS_OBTAIN_TMHM
	message PalletTown_Lab_Text_KantoDeliveredOaksParcel
	waitmessage
	waitfanfare
	msgbox PalletTown_Lab_Text_KantoOakCustomBallIOrdered

PalletTown_Lab_EventScript_KantoBlueArrival:
	setvar VAR_0x8004, PALLET_RECEIPT_BLUE_ARRIVED
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoDex
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	msgbox PalletTown_Lab_Text_KantoRivalGramps
	closemessage
	call_if_eq VAR_FACING, DIR_NORTH, PalletTown_Lab_EventScript_KantoDexSceneRivalEnterNorth
	call_if_eq VAR_FACING, DIR_SOUTH, PalletTown_Lab_EventScript_KantoDexSceneRivalEnterSouth
	call_if_eq VAR_FACING, DIR_EAST, PalletTown_Lab_EventScript_KantoDexSceneRivalEnterEastWest
	call_if_eq VAR_FACING, DIR_WEST, PalletTown_Lab_EventScript_KantoDexSceneRivalEnterEastWest
	fadedefaultbgm
	msgbox PalletTown_Lab_Text_KantoRivalWhatDidYouCallMeFor
	closemessage
	delay 30
	playse SE_PIN
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_Delay48
	waitmovement 0
	call_if_eq VAR_FACING, DIR_SOUTH, PalletTown_Lab_EventScript_KantoDexSceneOakFacePlayerAndRivalSouth
	call_if_eq VAR_FACING, DIR_EAST, PalletTown_Lab_EventScript_KantoDexSceneOakFacePlayerAndRivalEast
	call_if_eq VAR_FACING, DIR_WEST, PalletTown_Lab_EventScript_KantoDexSceneOakFacePlayerAndRivalWest
	call_if_eq VAR_FACING, DIR_NORTH, PalletTown_Lab_EventScript_KantoDexSceneOakFacePlayerAndRivalNorth
	special WayfarerKanto_CommitBlueArrival
	msgbox PalletTown_Lab_Text_KantoOakHaveRequestForYouTwo
	closemessage
	call_if_eq VAR_FACING, DIR_NORTH, PalletTown_Lab_EventScript_KantoDexSceneOakWalkToDesk
	call_if_eq VAR_FACING, DIR_SOUTH, PalletTown_Lab_EventScript_KantoDexSceneOakWalkToDeskSouth
	call_if_eq VAR_FACING, DIR_EAST, PalletTown_Lab_EventScript_KantoDexSceneOakWalkToDeskEast
	call_if_eq VAR_FACING, DIR_WEST, PalletTown_Lab_EventScript_KantoDexSceneOakWalkToDesk
	msgbox PalletTown_Lab_Text_KantoOakPokedexOnDesk
	closemessage
	delay 40
	msgbox PalletTown_Lab_Text_KantoOakTakeTheseWithYou
	closemessage
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	removeobject LOCALID_PALLET_KANTO_POKEDEX_1
	delay 10
	removeobject LOCALID_PALLET_KANTO_POKEDEX_2
	delay 25
	call_if_eq VAR_FACING, DIR_NORTH, PalletTown_Lab_EventScript_KantoOakBringDexesOver
	call_if_eq VAR_FACING, DIR_SOUTH, PalletTown_Lab_EventScript_KantoOakBringDexesOverSouth
	call_if_eq VAR_FACING, DIR_EAST, PalletTown_Lab_EventScript_KantoOakBringDexesOverEast
	call_if_eq VAR_FACING, DIR_WEST, PalletTown_Lab_EventScript_KantoOakBringDexesOver
	delay 10

PalletTown_Lab_EventScript_KantoDex:
	setvar VAR_0x8004, PALLET_RECEIPT_POKEDEX
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoBalls
	specialvar VAR_RESULT, WayfarerKanto_TryGrantPokedex
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoRewardRetry
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message PalletTown_Lab_Text_KantoReceivedPokedexFromOak
	waitmessage
	waitfanfare

PalletTown_Lab_EventScript_KantoBalls:
	setvar VAR_0x8004, PALLET_RECEIPT_POKE_BALLS
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoPresentation
	msgbox PalletTown_Lab_Text_KantoOakCatchMonsForDataTakeThese
	specialvar VAR_RESULT, WayfarerKanto_TryGrantPokeBalls
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoBagFull
	playfanfare MUS_LEVEL_UP
	message PalletTown_Lab_Text_KantoReceivedFivePokeBalls
	waitmessage
	waitfanfare
	waitbuttonpress

PalletTown_Lab_EventScript_KantoPresentation:
	setvar VAR_0x8004, PALLET_RECEIPT_PRESENTATION_DONE
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoFinish
	msgbox PalletTown_Lab_Text_KantoOakExplainCatching
	msgbox PalletTown_Lab_Text_KantoOakCompleteMonGuideWasMyDream
	msgbox PalletTown_Lab_Text_KantoRivalLeaveItToMeGramps
	call_if_eq VAR_FACING, DIR_NORTH, PalletTown_Lab_EventScript_KantoDexSceneRivalFacePlayerNorth
	call_if_eq VAR_FACING, DIR_SOUTH, PalletTown_Lab_EventScript_KantoDexSceneRivalFacePlayerSouth
	call_if_eq VAR_FACING, DIR_EAST, PalletTown_Lab_EventScript_KantoDexSceneRivalFacePlayerEastWest
	call_if_eq VAR_FACING, DIR_WEST, PalletTown_Lab_EventScript_KantoDexSceneRivalFacePlayerEastWest
	msgbox PalletTown_Lab_Text_KantoRivalTellSisNotToGiveYouMap
	closemessage
	playbgm MUS_RG_RIVAL_EXIT, 0
	call_if_eq VAR_FACING, DIR_NORTH, PalletTown_Lab_EventScript_KantoDexSceneRivalExitNorth
	call_if_ne VAR_FACING, DIR_NORTH, PalletTown_Lab_EventScript_KantoDexSceneRivalExit
	removeobject LOCALID_PALLET_KANTO_BLUE
	fadedefaultbgm
	special WayfarerKanto_CommitPresentationDone

PalletTown_Lab_EventScript_KantoFinish:
	specialvar VAR_RESULT, WayfarerKanto_CompleteOpening
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoRewardRetry
	release
	end

PalletTown_Lab_EventScript_KantoComplete:
	msgbox PalletTown_Lab_Text_KantoComplete, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoParcelMissing:
	msgbox PalletTown_Lab_Text_KantoParcelMissing, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoBagFull:
	msgbox PalletTown_Lab_Text_KantoBagFull, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoRewardRetry:
	msgbox PalletTown_Lab_Text_KantoRewardRetry, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoDexSceneRivalEnterNorth:
	clearflag FLAG_TEMP_2
	setobjectxyperm LOCALID_PALLET_KANTO_BLUE, 12, 18
	addobject LOCALID_PALLET_KANTO_BLUE
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoWatchRivalEnterNorth
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalEnter
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneRivalEnterEastWest:
	clearflag FLAG_TEMP_2
	setobjectxyperm LOCALID_PALLET_KANTO_BLUE, 13, 18
	addobject LOCALID_PALLET_KANTO_BLUE
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoWatchRivalEnterEastWest
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalEnter
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneRivalEnterSouth:
	clearflag FLAG_TEMP_2
	setobjectxyperm LOCALID_PALLET_KANTO_BLUE, 13, 18
	addobject LOCALID_PALLET_KANTO_BLUE
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalEnter
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneOakFacePlayerAndRivalSouth:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_FacePlayer
	waitmovement 0
	delay 15
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneOakFacePlayerAndRivalWest:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_FacePlayer
	waitmovement 0
	delay 15
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneOakFacePlayerAndRivalEast:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_FacePlayer
	waitmovement 0
	delay 15
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneOakFacePlayerAndRivalNorth:
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneOakWalkToDesk:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, PalletTown_Lab_Movement_KantoOakWalkToDesk
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneOakWalkToDeskSouth:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, PalletTown_Lab_Movement_KantoOakWalkToDeskSouth
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoWatchOakWalkToDesk
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoWatchOakWalkToDesk
	waitmovement LOCALID_PALLET_KANTO_LAB_OAK
	return

PalletTown_Lab_EventScript_KantoDexSceneOakWalkToDeskEast:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, PalletTown_Lab_Movement_KantoOakWalkToDesk
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoWatchOakWalkToDeskEast
	waitmovement LOCALID_PALLET_KANTO_LAB_OAK
	return

PalletTown_Lab_EventScript_KantoOakBringDexesOver:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, PalletTown_Lab_Movement_KantoOakBringDexesOver
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoOakBringDexesOverSouth:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, PalletTown_Lab_Movement_KantoOakBringDexesOverSouth
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoOakBringDexesOverEast:
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, PalletTown_Lab_Movement_KantoOakBringDexesOver
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoPlayerFaceOakForDexEast
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneRivalFacePlayerNorth:
	applymovement LOCALID_PALLET_KANTO_BLUE, Common_Movement_FacePlayer
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneRivalFacePlayerSouth:
	applymovement LOCALID_PALLET_KANTO_BLUE, Common_Movement_WalkInPlaceFasterUp
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneRivalFacePlayerEastWest:
	applymovement LOCALID_PALLET_KANTO_BLUE, Common_Movement_FacePlayer
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneRivalExitNorth:
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoWatchRivalEnterEastWest
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalExit
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoDexSceneRivalExit:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalExit
	waitmovement 0
	return

PalletTown_Lab_Movement_KantoWatchRivalEnterNorth:
	walk_in_place_faster_down
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_8
	walk_in_place_faster_left
	step_end

PalletTown_Lab_Movement_KantoWatchRivalEnterEastWest:
	delay_4
	walk_in_place_faster_down
	step_end

PalletTown_Lab_Movement_KantoRivalEnter:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

PalletTown_Lab_Movement_KantoRivalExit:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

PalletTown_Lab_Movement_KantoOakWalkToDesk:
	walk_up
	walk_left
	delay_16
	walk_in_place_faster_down
	step_end

PalletTown_Lab_Movement_KantoOakWalkToDeskSouth:
	walk_left
	walk_left
	walk_up
	delay_16
	walk_in_place_faster_right
	step_end

PalletTown_Lab_Movement_KantoWatchOakWalkToDesk:
	delay_16
	delay_8
	walk_in_place_faster_left
	step_end

PalletTown_Lab_Movement_KantoWatchOakWalkToDeskEast:
	delay_16
	delay_8
	walk_in_place_faster_up
	step_end

PalletTown_Lab_Movement_KantoOakBringDexesOver:
	walk_right
	walk_down
	step_end

PalletTown_Lab_Movement_KantoOakBringDexesOverSouth:
	walk_down
	walk_right
	step_end

PalletTown_Lab_Movement_KantoPlayerFaceOakForDexEast:
	delay_16
	delay_8
	walk_in_place_faster_right
	step_end

@ FRLG EventScript_Rival, by opening phase.
PalletTown_Lab_EventScript_KantoBlue::
	lock
	faceplayer
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_eq VAR_RESULT, PALLET_OPENING_STARTER_RECEIVED, PalletTown_Lab_EventScript_KantoRivalChoseStarter
	goto_if_eq VAR_RESULT, PALLET_OPENING_LAB_STARTER_CHOICE, PalletTown_Lab_EventScript_KantoRivalWaitingForStarter
	goto_if_gt VAR_RESULT, PALLET_OPENING_STARTER_RECEIVED, PalletTown_Lab_EventScript_KantoBlueDex
	msgbox PalletTown_Lab_Text_KantoRivalGrampsIsntAround
	release
	end

PalletTown_Lab_EventScript_KantoRivalWaitingForStarter:
	msgbox PalletTown_Lab_Text_KantoRivalGoChoosePlayer
	release
	end

PalletTown_Lab_EventScript_KantoRivalChoseStarter:
	msgbox PalletTown_Lab_Text_KantoRivalMyMonLooksTougher
	release
	end

@ Only reachable while resuming an interrupted Parcel handoff.
PalletTown_Lab_EventScript_KantoBlueDex:
	msgbox PalletTown_Lab_Text_KantoBlueDex
	release
	end

PalletTown_Lab_EventScript_KantoBulbasaurBall::
	lock
	faceplayer
	setvar VAR_0x8004, KANTO_STARTER_SLOT_BULBASAUR
	goto PalletTown_Lab_EventScript_KantoBall

PalletTown_Lab_EventScript_KantoSquirtleBall::
	lock
	faceplayer
	setvar VAR_0x8004, KANTO_STARTER_SLOT_SQUIRTLE
	goto PalletTown_Lab_EventScript_KantoBall

PalletTown_Lab_EventScript_KantoCharmanderBall::
	lock
	faceplayer
	setvar VAR_0x8004, KANTO_STARTER_SLOT_CHARMANDER
	goto PalletTown_Lab_EventScript_KantoBall

PalletTown_Lab_EventScript_KantoBall:
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_ge VAR_RESULT, PALLET_OPENING_STARTER_RECEIVED, PalletTown_Lab_EventScript_KantoLastPokeBall
	goto_if_eq VAR_RESULT, PALLET_OPENING_LAB_STARTER_CHOICE, PalletTown_Lab_EventScript_KantoConfirmStarterChoice
	msgbox PalletTown_Lab_Text_KantoThoseArePokeBalls
	release
	end

PalletTown_Lab_EventScript_KantoLastPokeBall:
	msgbox PalletTown_Lab_Text_KantoOaksLastMon
	release
	end

PalletTown_Lab_EventScript_KantoConfirmStarterChoice:
	specialvar VAR_RESULT, WayfarerKanto_GetStarterSlot
	goto_if_ne VAR_RESULT, KANTO_STARTER_SLOT_NONE, PalletTown_Lab_EventScript_KantoRetryStarter
	specialvar VAR_0x8005, WayfarerKanto_GetSelectedStarterSpecies
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_FaceRight
	waitmovement 0
	showmonpic VAR_0x8005, 10, 3
	goto_if_eq VAR_0x8004, KANTO_STARTER_SLOT_BULBASAUR, PalletTown_Lab_EventScript_KantoConfirmBulbasaur
	goto_if_eq VAR_0x8004, KANTO_STARTER_SLOT_SQUIRTLE, PalletTown_Lab_EventScript_KantoConfirmSquirtle
	goto PalletTown_Lab_EventScript_KantoConfirmCharmander

PalletTown_Lab_EventScript_KantoConfirmBulbasaur:
	goto_if_ne VAR_0x8005, SPECIES_BULBASAUR, PalletTown_Lab_EventScript_KantoConfirmSubstitute
	msgbox PalletTown_Lab_Text_KantoOakChoosingBulbasaur, MSGBOX_YESNO
	goto PalletTown_Lab_EventScript_KantoConfirmResult

PalletTown_Lab_EventScript_KantoConfirmSquirtle:
	goto_if_ne VAR_0x8005, SPECIES_SQUIRTLE, PalletTown_Lab_EventScript_KantoConfirmSubstitute
	msgbox PalletTown_Lab_Text_KantoOakChoosingSquirtle, MSGBOX_YESNO
	goto PalletTown_Lab_EventScript_KantoConfirmResult

PalletTown_Lab_EventScript_KantoConfirmCharmander:
	goto_if_ne VAR_0x8005, SPECIES_CHARMANDER, PalletTown_Lab_EventScript_KantoConfirmSubstitute
	msgbox PalletTown_Lab_Text_KantoOakChoosingCharmander, MSGBOX_YESNO
	goto PalletTown_Lab_EventScript_KantoConfirmResult

@ Wayfarer-only: a challenge setting replaced this slot's species, so FRLG's
@ species-specific line would misname it.
PalletTown_Lab_EventScript_KantoConfirmSubstitute:
	bufferspeciesname STR_VAR_1, VAR_0x8005
	special WayfarerKanto_BufferSelectedStarterType
	msgbox PalletTown_Lab_Text_KantoStarterPreview, MSGBOX_YESNO

PalletTown_Lab_EventScript_KantoConfirmResult:
	hidemonpic
	goto_if_ne VAR_RESULT, YES, PalletTown_Lab_EventScript_KantoDeclinedStarter
	specialvar VAR_RESULT, WayfarerKanto_CommitStarterSlot
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoBallUnavailable
	removeobject VAR_LAST_TALKED
	msgbox PalletTown_Lab_Text_KantoOakThisMonIsEnergetic
	goto PalletTown_Lab_EventScript_KantoGiveStarter

@ FRLG DeclinedStarter: nothing is said or committed.
PalletTown_Lab_EventScript_KantoDeclinedStarter:
	release
	end

@ Wayfarer-only retry after a failed delivery of the committed slot.
PalletTown_Lab_EventScript_KantoRetryStarter:
	specialvar VAR_0x8004, WayfarerKanto_GetStarterSlot
PalletTown_Lab_EventScript_KantoGiveStarter:
	call PalletTown_Lab_EventScript_KantoRemovePlayerBall
	specialvar VAR_RESULT, WayfarerKanto_TryGiveStarter
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoStarterFull
	specialvar VAR_0x8004, WayfarerKanto_GetStarterSlot
	specialvar VAR_0x8005, WayfarerKanto_GetSelectedStarterSpecies
	bufferspeciesname STR_VAR_1, VAR_0x8005
	message PalletTown_Lab_Text_KantoReceivedMonFromOak
	waitmessage
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	waitfanfare
	specialvar VAR_RESULT, IsNuzlockeNicknamingActive
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoForceNickname
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, PalletTown_Lab_EventScript_KantoRivalPicksStarter
	call Common_EventScript_NameReceivedPartyMonFull
	goto PalletTown_Lab_EventScript_KantoRivalPicksStarter

PalletTown_Lab_EventScript_KantoForceNickname:
	message gText_NicknameThisPokemon
	waitmessage
	call Common_EventScript_NameReceivedPartyMonFull

PalletTown_Lab_EventScript_KantoRivalPicksStarter:
	closemessage
	specialvar VAR_RESULT, WayfarerKanto_GetCounterSlot
	goto_if_eq VAR_RESULT, KANTO_STARTER_SLOT_CHARMANDER, PalletTown_Lab_EventScript_KantoRivalWalksToCharmander
	goto_if_eq VAR_RESULT, KANTO_STARTER_SLOT_SQUIRTLE, PalletTown_Lab_EventScript_KantoRivalWalksToSquirtle
	goto PalletTown_Lab_EventScript_KantoRivalWalksToBulbasaur

PalletTown_Lab_EventScript_KantoRivalWalksToCharmander:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalWalksToCharmander
	waitmovement 0
	setobjectxyperm LOCALID_PALLET_KANTO_BLUE, 17, 13
	setvar VAR_0x8006, LOCALID_PALLET_KANTO_CHARMANDER_BALL
	setvar VAR_0x8007, SPECIES_CHARMANDER
	goto PalletTown_Lab_EventScript_KantoRivalTakesStarter

PalletTown_Lab_EventScript_KantoRivalWalksToSquirtle:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalWalksToSquirtle
	waitmovement 0
	setobjectxyperm LOCALID_PALLET_KANTO_BLUE, 16, 13
	setvar VAR_0x8006, LOCALID_PALLET_KANTO_SQUIRTLE_BALL
	setvar VAR_0x8007, SPECIES_SQUIRTLE
	goto PalletTown_Lab_EventScript_KantoRivalTakesStarter

PalletTown_Lab_EventScript_KantoRivalWalksToBulbasaur:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalWalksToBulbasaur
	waitmovement 0
	setobjectxyperm LOCALID_PALLET_KANTO_BLUE, 15, 13
	setvar VAR_0x8006, LOCALID_PALLET_KANTO_BULBASAUR_BALL
	setvar VAR_0x8007, SPECIES_BULBASAUR
	goto PalletTown_Lab_EventScript_KantoRivalTakesStarter

PalletTown_Lab_EventScript_KantoRivalTakesStarter:
	setobjectmovementtype LOCALID_PALLET_KANTO_BLUE, MOVEMENT_TYPE_FACE_UP
	msgbox PalletTown_Lab_Text_KantoRivalIllTakeThisOneThen
	removeobject VAR_0x8006
	bufferspeciesname STR_VAR_1, VAR_0x8007
	message PalletTown_Lab_Text_KantoRivalReceivedMonFromOak
	waitmessage
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	waitfanfare
	release
	end

PalletTown_Lab_EventScript_KantoRemovePlayerBall:
	goto_if_eq VAR_0x8004, KANTO_STARTER_SLOT_BULBASAUR, PalletTown_Lab_EventScript_KantoRemoveBulbasaurBall
	goto_if_eq VAR_0x8004, KANTO_STARTER_SLOT_SQUIRTLE, PalletTown_Lab_EventScript_KantoRemoveSquirtleBall
	removeobject LOCALID_PALLET_KANTO_CHARMANDER_BALL
	return

PalletTown_Lab_EventScript_KantoRemoveBulbasaurBall:
	removeobject LOCALID_PALLET_KANTO_BULBASAUR_BALL
	return

PalletTown_Lab_EventScript_KantoRemoveSquirtleBall:
	removeobject LOCALID_PALLET_KANTO_SQUIRTLE_BALL
	return

PalletTown_Lab_EventScript_KantoStarterFull:
	msgbox PalletTown_Lab_Text_KantoStarterFull, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoBallUnavailable:
	msgbox PalletTown_Lab_Text_KantoBallUnavailable, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_Movement_KantoRivalWalksToCharmander:
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	step_end

PalletTown_Lab_Movement_KantoRivalWalksToSquirtle:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

PalletTown_Lab_Movement_KantoRivalWalksToBulbasaur:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

@ FRLG exit coords (5..7,8) map to the bookshelf gap (12..14,16). VAR_TEMP_2
@ holds the lane (1 left, 2 middle, 3 right) exactly as FRLG does.
PalletTown_Lab_EventScript_KantoExitLeft::
	setvar VAR_TEMP_2, 1
	goto PalletTown_Lab_EventScript_KantoExitTrigger

PalletTown_Lab_EventScript_KantoExitMid::
	setvar VAR_TEMP_2, 2
	goto PalletTown_Lab_EventScript_KantoExitTrigger

PalletTown_Lab_EventScript_KantoExitRight::
	setvar VAR_TEMP_2, 3
	goto PalletTown_Lab_EventScript_KantoExitTrigger

PalletTown_Lab_EventScript_KantoExitTrigger:
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoExitInert
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_eq VAR_RESULT, PALLET_OPENING_LAB_STARTER_CHOICE, PalletTown_Lab_EventScript_KantoLeaveStarterScene
	goto_if_eq VAR_RESULT, PALLET_OPENING_STARTER_RECEIVED, PalletTown_Lab_EventScript_KantoRivalBattle
@ Coord scripts that end without releaseall leave the player frozen.
PalletTown_Lab_EventScript_KantoExitInert:
	lockall
	setvar VAR_TEMP_2, 0
	releaseall
	end

@ FRLG LeaveStarterSceneTrigger.
PalletTown_Lab_EventScript_KantoLeaveStarterScene:
	lockall
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_FaceDown
	waitmovement 0
	msgbox PalletTown_Lab_Text_KantoOakHeyDontGoAwayYet
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkUp
	waitmovement 0
	setvar VAR_TEMP_2, 0
	releaseall
	end

@ FRLG RivalBattle. Blue waits at (15..17,13) under his ball, walks left to
@ the player's lane, then down 2 to stand directly above the player.
PalletTown_Lab_EventScript_KantoRivalBattle:
	lockall
	setvar VAR_0x8004, PALLET_RECEIPT_STARTER
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoExitReleased
	specialvar VAR_RESULT, CountPartyAliveNonEggMons
	goto_if_eq VAR_RESULT, 0, PalletTown_Lab_EventScript_KantoNoUsableMon
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	applymovement LOCALID_PALLET_KANTO_BLUE, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	msgbox PalletTown_Lab_Text_KantoRivalLetsCheckOutMons
	closemessage
	applymovement LOCALID_PALLET_KANTO_LAB_OAK, Common_Movement_FaceDown
	waitmovement 0
	specialvar VAR_RESULT, WayfarerKanto_GetCounterSlot
	setvar VAR_0x8005, 4 @ Blue's column (15, 16, or 17) minus 11
	call_if_eq VAR_RESULT, KANTO_STARTER_SLOT_SQUIRTLE, PalletTown_Lab_EventScript_KantoRivalColumnSquirtle
	call_if_eq VAR_RESULT, KANTO_STARTER_SLOT_CHARMANDER, PalletTown_Lab_EventScript_KantoRivalColumnCharmander
	subvar VAR_0x8005, VAR_TEMP_2 @ steps left to the player's lane (1..5)
	call_if_eq VAR_0x8005, 1, PalletTown_Lab_EventScript_KantoRivalApproach1
	call_if_eq VAR_0x8005, 2, PalletTown_Lab_EventScript_KantoRivalApproach2
	call_if_eq VAR_0x8005, 3, PalletTown_Lab_EventScript_KantoRivalApproach3
	call_if_eq VAR_0x8005, 4, PalletTown_Lab_EventScript_KantoRivalApproach4
	call_if_eq VAR_0x8005, 5, PalletTown_Lab_EventScript_KantoRivalApproach5
	specialvar VAR_RESULT, WayfarerKanto_GetCounterSlot
	goto_if_eq VAR_RESULT, KANTO_STARTER_SLOT_BULBASAUR, PalletTown_Lab_EventScript_KantoRivalBattleBulbasaur
	goto_if_eq VAR_RESULT, KANTO_STARTER_SLOT_CHARMANDER, PalletTown_Lab_EventScript_KantoRivalBattleCharmander
	goto PalletTown_Lab_EventScript_KantoRivalBattleSquirtle

PalletTown_Lab_EventScript_KantoRivalColumnSquirtle:
	setvar VAR_0x8005, 5
	return

PalletTown_Lab_EventScript_KantoRivalColumnCharmander:
	setvar VAR_0x8005, 6
	return

PalletTown_Lab_EventScript_KantoRivalApproach1:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalApproach1
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoRivalApproach2:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalApproach2
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoRivalApproach3:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalApproach3
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoRivalApproach4:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalApproach4
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoRivalApproach5:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalApproach5
	waitmovement 0
	return

PalletTown_Lab_EventScript_KantoRivalBattleBulbasaur:
	trainerbattle_earlyrival TRAINER_WAYFARER_KANTO_BLUE_BULBASAUR, RIVAL_BATTLE_TUTORIAL, PalletTown_Lab_Text_KantoRivalDefeat, PalletTown_Lab_Text_KantoRivalVictory
	goto PalletTown_Lab_EventScript_KantoEndRivalBattle

PalletTown_Lab_EventScript_KantoRivalBattleCharmander:
	trainerbattle_earlyrival TRAINER_WAYFARER_KANTO_BLUE_CHARMANDER, RIVAL_BATTLE_TUTORIAL, PalletTown_Lab_Text_KantoRivalDefeat, PalletTown_Lab_Text_KantoRivalVictory
	goto PalletTown_Lab_EventScript_KantoEndRivalBattle

PalletTown_Lab_EventScript_KantoRivalBattleSquirtle:
	trainerbattle_earlyrival TRAINER_WAYFARER_KANTO_BLUE_SQUIRTLE, RIVAL_BATTLE_TUTORIAL, PalletTown_Lab_Text_KantoRivalDefeat, PalletTown_Lab_Text_KantoRivalVictory
	goto PalletTown_Lab_EventScript_KantoEndRivalBattle

@ FRLG EndRivalBattle: both outcomes heal and converge here.
PalletTown_Lab_EventScript_KantoEndRivalBattle:
	special WayfarerKanto_ResolveFirstBattle
	special HealPlayerParty
	msgbox PalletTown_Lab_Text_KantoRivalGoToughenMyMon
	closemessage
	playbgm MUS_RG_RIVAL_EXIT, 0
	call_if_eq VAR_TEMP_2, 1, PalletTown_Lab_EventScript_KantoRivalExitAfterBattleLeft
	call_if_eq VAR_TEMP_2, 2, PalletTown_Lab_EventScript_KantoRivalExitAfterBattleMid
	call_if_eq VAR_TEMP_2, 3, PalletTown_Lab_EventScript_KantoRivalExitAfterBattleRight
	removeobject LOCALID_PALLET_KANTO_BLUE
	playse SE_EXIT
	fadedefaultbgm
	releaseall
	end

PalletTown_Lab_EventScript_KantoRivalExitAfterBattleLeft:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalExitAfterBattleLeft
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoPlayerWatchRivalExitAfterBattle
	waitmovement LOCALID_PALLET_KANTO_BLUE
	return

PalletTown_Lab_EventScript_KantoRivalExitAfterBattleMid:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalExitAfterBattleMid
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoPlayerWatchRivalExitAfterBattle
	waitmovement LOCALID_PALLET_KANTO_BLUE
	return

PalletTown_Lab_EventScript_KantoRivalExitAfterBattleRight:
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_KantoRivalExitAfterBattleRight
	applymovement LOCALID_PLAYER, PalletTown_Lab_Movement_KantoPlayerWatchRivalExitAfterBattleRight
	waitmovement LOCALID_PALLET_KANTO_BLUE
	return

@ Wayfarer-only guard; FRLG's fresh starter can always battle.
PalletTown_Lab_EventScript_KantoNoUsableMon:
	msgbox PalletTown_Lab_Text_KantoNoUsableMon, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkUp
	waitmovement 0
PalletTown_Lab_EventScript_KantoExitReleased:
	setvar VAR_TEMP_2, 0
	releaseall
	end

PalletTown_Lab_Movement_KantoRivalApproach1:
	walk_left
	walk_down
	walk_down
	step_end

PalletTown_Lab_Movement_KantoRivalApproach2:
	walk_left
	walk_left
	walk_down
	walk_down
	step_end

PalletTown_Lab_Movement_KantoRivalApproach3:
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	step_end

PalletTown_Lab_Movement_KantoRivalApproach4:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	step_end

PalletTown_Lab_Movement_KantoRivalApproach5:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	step_end

PalletTown_Lab_Movement_KantoRivalExitAfterBattleLeft:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	delay_8
	step_end

PalletTown_Lab_Movement_KantoRivalExitAfterBattleRight:
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	delay_8
	step_end

PalletTown_Lab_Movement_KantoRivalExitAfterBattleMid:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_left
	walk_down
	walk_down
	delay_8
	step_end

PalletTown_Lab_Movement_KantoPlayerWatchRivalExitAfterBattle:
	delay_16
	delay_8
	walk_in_place_faster_right
	delay_16
	walk_in_place_faster_down
	step_end

PalletTown_Lab_Movement_KantoPlayerWatchRivalExitAfterBattleRight:
	delay_16
	delay_8
	walk_in_place_faster_left
	delay_16
	walk_in_place_faster_down
	step_end

@ Lab ambient content (FRLG Computer, Pokedex, LeftSign, RightSign) on the
@ translated back-wall tiles; inert for other origins.
PalletTown_Lab_EventScript_KantoComputer::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoAmbientInert
	msgbox PalletTown_Lab_Text_KantoEmailMessage, MSGBOX_SIGN
	end

PalletTown_Lab_EventScript_KantoPokedex::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoAmbientInert
	setvar VAR_0x8004, PALLET_RECEIPT_POKEDEX
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoAmbientInert
	msgbox PalletTown_Lab_Text_KantoBlankEncyclopedia, MSGBOX_SIGN
	end

PalletTown_Lab_EventScript_KantoLeftPoster::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoAmbientInert
	msgbox PalletTown_Lab_Text_KantoPressStartToOpenMenu, MSGBOX_SIGN
	end

PalletTown_Lab_EventScript_KantoRightPoster::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoAmbientInert
	lockall
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoRightPosterAlt
	msgbox PalletTown_Lab_Text_KantoSaveOptionInMenu
	releaseall
	end

PalletTown_Lab_EventScript_KantoRightPosterAlt:
	msgbox PalletTown_Lab_Text_KantoAllMonTypesHaveStrongAndWeakPoints
	releaseall
	end

PalletTown_Lab_EventScript_KantoAmbientInert:
	end
#else
PalletTown_Lab_EventScript_KantoBlue::
PalletTown_Lab_EventScript_KantoBulbasaurBall::
PalletTown_Lab_EventScript_KantoCharmanderBall::
PalletTown_Lab_EventScript_KantoSquirtleBall::
PalletTown_Lab_EventScript_KantoExitLeft::
PalletTown_Lab_EventScript_KantoExitMid::
PalletTown_Lab_EventScript_KantoExitRight::
PalletTown_Lab_EventScript_KantoComputer::
PalletTown_Lab_EventScript_KantoPokedex::
PalletTown_Lab_EventScript_KantoLeftPoster::
PalletTown_Lab_EventScript_KantoRightPoster::
	end
#endif

PalletTown_Lab_EventScript_OakGiveNatDex_ForPlaytesters::
	setflag FLAG_SYS_NATIONAL_DEX
	special EnableNationalPokedex
	setvar VAR_0x8004, 1 @ DEX_REGION_KANTO
	special DexScript_GrantNationalExtension
	return

PalletTown_Lab_EventScript_OakBadgeCheck::
#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
	specialvar VAR_RESULT, WayfarerGetKantoBadgeCountForScript
	goto_if_ge VAR_RESULT, 8, PalletTown_Lab_EventScript_Oak16Badges
	goto_if_ge VAR_RESULT, 1, PalletTown_Lab_EventScript_Oak9Badges
#else
	goto_if_ge VAR_NUM_BADGES, 16, PalletTown_Lab_EventScript_Oak16Badges
	goto_if_ge VAR_NUM_BADGES, 9, PalletTown_Lab_EventScript_Oak9Badges
#endif
	msgbox PalletTown_Lab_Text_OakNoKantoBadges, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_Oak16Badges::
	msgbox PalletTown_Lab_Text_OakOpenMtSilver, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_INDIGOJUNCTION_HIDE_SILVER_GUARD
	setvar VAR_PALLETTOWN_LABSTATE, 2
#if !(IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED)
	call_if_unset FLAG_IS_KANTO_CHAMPION, PalletTown_Lab_EventScript_OakKantoLeague
#endif
	release
	end

PalletTown_Lab_EventScript_OakKantoLeague::
	msgbox PalletTown_Lab_Text_OakOpenIndigoPlateau, MSGBOX_DEFAULT
	closemessage
	return

PalletTown_Lab_EventScript_Oak9Badges::
	msgbox PalletTown_Lab_Text_OakYesKantoBadges, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_OakDex::
	call_if_unset FLAG_SYS_NATIONAL_DEX, PalletTown_Lab_EventScript_OakGiveNatDex_ForPlaytesters
	msgbox PalletTown_Lab_Text_OakLabDexCheck, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_Assistant1::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoAideAuthority
#endif
	msgbox PalletTown_Lab_Text_OaksAssistant1, MSGBOX_NPC
	end

PalletTown_Lab_EventScript_Assistant2::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoAide
#endif
	msgbox PalletTown_Lab_Text_OaksAssistant2, MSGBOX_NPC
	end

PalletTown_Lab_EventScript_Assistant3::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoAide
#endif
	msgbox PalletTown_Lab_Text_OaksAssistant3, MSGBOX_NPC
	end

#if IS_WAYFARER
@ FRLG Aide1 and Aide2.
PalletTown_Lab_EventScript_KantoAide::
	msgbox PalletTown_Lab_Text_KantoStudyAsOaksAide, MSGBOX_NPC
	end

@ FRLG Aide3.
PalletTown_Lab_EventScript_KantoAideAuthority::
	msgbox PalletTown_Lab_Text_KantoOakIsAuthorityOnMons, MSGBOX_NPC
	end
#endif

PalletTown_Lab_EventScript_PC::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoComputer
#endif
	msgbox PalletTown_Lab_Text_OaksLabPC, MSGBOX_SIGN
	end

#if IS_WAYFARER
@ FRLG PalletTown_ProfessorOaksLab_Frlg text, {RIVAL} transcribed as BLUE.
PalletTown_Lab_Text_KantoRivalGrampsIsntAround:
	.string "BLUE: What, it's only {PLAYER}?\n"
	.string "Gramps isn't around.$"

PalletTown_Lab_Text_KantoRivalFedUpWithWaiting:
	.string "BLUE: Gramps!\n"
	.string "I'm fed up with waiting!$"

PalletTown_Lab_Text_KantoRivalNoFairWhatAboutMe:
	.string "BLUE: Hey! Gramps! No fair!\n"
	.string "What about me?$"

PalletTown_Lab_Text_KantoRivalGoChoosePlayer:
	.string "BLUE: Heh, I don't need to be\n"
	.string "greedy like you. I'm mature!\p"
	.string "Go ahead and choose, {PLAYER}!$"

PalletTown_Lab_Text_KantoRivalIllTakeThisOneThen:
	.string "BLUE: I'll take this one, then!$"

PalletTown_Lab_Text_KantoRivalReceivedMonFromOak:
	.string "BLUE received the {STR_VAR_1}\n"
	.string "from PROF. OAK!$"

PalletTown_Lab_Text_KantoRivalMyMonLooksTougher:
	.string "BLUE: My POKéMON looks a lot\n"
	.string "tougher than yours.$"

PalletTown_Lab_Text_KantoRivalLetsCheckOutMons:
	.string "BLUE: Wait, {PLAYER}!\n"
	.string "Let's check out our POKéMON!\p"
	.string "Come on, I'll take you on!$"

PalletTown_Lab_Text_KantoRivalDefeat:
	.string "WHAT?\n"
	.string "Unbelievable!\l"
	.string "I picked the wrong POKéMON!$"

PalletTown_Lab_Text_KantoRivalVictory:
	.string "BLUE: Yeah!\n"
	.string "Am I great or what?$"

PalletTown_Lab_Text_KantoRivalGoToughenMyMon:
	.string "BLUE: Okay! I'll make my\n"
	.string "POKéMON battle to toughen it up!\p"
	.string "{PLAYER}! Gramps!\n"
	.string "Smell you later!$"

PalletTown_Lab_Text_KantoRivalGramps:
	.string "BLUE: Gramps!$"

PalletTown_Lab_Text_KantoRivalWhatDidYouCallMeFor:
	.string "BLUE: I almost forgot!\n"
	.string "What did you call me for?$"

PalletTown_Lab_Text_KantoRivalLeaveItToMeGramps:
	.string "BLUE: All right, Gramps!\n"
	.string "Leave it all to me!$"

PalletTown_Lab_Text_KantoRivalTellSisNotToGiveYouMap:
	.string "{PLAYER}, I hate to say it, but you\n"
	.string "won't be necessary for this.\p"
	.string "I know! I'll borrow a TOWN MAP\n"
	.string "from my sis!\p"
	.string "I'll tell her not to lend you one,\n"
	.string "{PLAYER}! Hahaha!\p"
	.string "Don't bother coming around to\n"
	.string "my place after this!$"

PalletTown_Lab_Text_KantoOakThreeMonsChooseOne:
	.string "OAK: BLUE?\n"
	.string "Let me think…\p"
	.string "Oh, that's right, I told you to\n"
	.string "come! Just wait!\p"
	.string "Here, {PLAYER}.\p"
	.string "There are three POKéMON here.\p"
	.string "Haha!\p"
	.string "The POKéMON are held inside\n"
	.string "these POKé BALLS.\p"
	.string "When I was young, I was a serious\n"
	.string "POKéMON TRAINER.\p"
	.string "But now, in my old age, I have\n"
	.string "only these three left.\p"
	.string "You can have one.\n"
	.string "Go on, choose!$"

PalletTown_Lab_Text_KantoOakBePatientRival:
	.string "OAK: Be patient, BLUE.\n"
	.string "You can have one, too!$"

PalletTown_Lab_Text_KantoOakWhichOneWillYouChoose:
	.string "OAK: Now, {PLAYER}.\p"
	.string "Inside those three POKé BALLS are\n"
	.string "POKéMON.\p"
	.string "Which one will you choose for\n"
	.string "yourself?$"

PalletTown_Lab_Text_KantoOakHeyDontGoAwayYet:
	.string "OAK: Hey!\n"
	.string "Don't go away yet!$"

PalletTown_Lab_Text_KantoOakChoosingCharmander:
	.string "Ah! CHARMANDER is your choice.\n"
	.string "You should raise it patiently.\p"
	.string "So, {PLAYER}, you're claiming the\n"
	.string "FIRE POKéMON CHARMANDER?$"

PalletTown_Lab_Text_KantoOakChoosingSquirtle:
	.string "Hm! SQUIRTLE is your choice.\n"
	.string "It's one worth raising.\p"
	.string "So, {PLAYER}, you've decided on the\n"
	.string "WATER POKéMON SQUIRTLE?$"

PalletTown_Lab_Text_KantoOakChoosingBulbasaur:
	.string "I see! BULBASAUR is your choice.\n"
	.string "It's very easy to raise.\p"
	.string "So, {PLAYER}, you want to go with\n"
	.string "the GRASS POKéMON BULBASAUR?$"

PalletTown_Lab_Text_KantoOakThisMonIsEnergetic:
	.string "This POKéMON is really quite\n"
	.string "energetic!$"

PalletTown_Lab_Text_KantoReceivedMonFromOak:
	.string "{PLAYER} received the {STR_VAR_1}\n"
	.string "from PROF. OAK!$"

PalletTown_Lab_Text_KantoOakCanReachNextTownWithMon:
	.string "OAK: If a wild POKéMON appears,\n"
	.string "your POKéMON can battle it.\p"
	.string "With it at your side, you should be\n"
	.string "able to reach the next town.$"

PalletTown_Lab_Text_KantoOakBattleMonForItToGrow:
	.string "OAK: {PLAYER}, raise your young\n"
	.string "POKéMON by making it battle.\p"
	.string "It has to battle for it to grow.$"

PalletTown_Lab_Text_KantoOakHaveSomethingForMe:
	.string "OAK: Oh, {PLAYER}!\n"
	.string "How is my old POKéMON?\p"
	.string "Well, it seems to be growing more\n"
	.string "attached to you.\p"
	.string "You must be talented as a POKéMON\n"
	.string "TRAINER.\p"
	.string "What's that?\n"
	.string "You have something for me?$"

PalletTown_Lab_Text_KantoDeliveredOaksParcel:
	.string "{PLAYER} delivered OAK'S PARCEL.$"

PalletTown_Lab_Text_KantoOakCustomBallIOrdered:
	.string "Ah! \n"
	.string "It's the custom POKé BALL!\p"
	.string "I had it on order.\n"
	.string "Thank you!$"

PalletTown_Lab_Text_KantoOakHaveRequestForYouTwo:
	.string "OAK: Oh, right!\n"
	.string "I have a request for you two.$"

PalletTown_Lab_Text_KantoOakPokedexOnDesk:
	.string "On the desk there is my invention,\n"
	.string "the POKéDEX!\p"
	.string "It automatically records data on\n"
	.string "POKéMON you've seen or caught.\p"
	.string "It's a high-tech encyclopedia!$"

PalletTown_Lab_Text_KantoOakTakeTheseWithYou:
	.string "OAK: {PLAYER} and BLUE.\n"
	.string "Take these with you.$"

PalletTown_Lab_Text_KantoReceivedPokedexFromOak:
	.string "{PLAYER} received the POKéDEX\n"
	.string "from PROF. OAK.$"

PalletTown_Lab_Text_KantoOakCatchMonsForDataTakeThese:
	.string "OAK: You can't get detailed data\n"
	.string "on POKéMON by just seeing them.\p"
	.string "You must catch them to obtain\n"
	.string "complete data.\p"
	.string "So, here are some tools for\n"
	.string "catching wild POKéMON.$"

PalletTown_Lab_Text_KantoReceivedFivePokeBalls:
	.string "{PLAYER} received five POKé BALLS.$"

PalletTown_Lab_Text_KantoOakExplainCatching:
	.string "When a wild POKéMON appears,\n"
	.string "it's fair game.\p"
	.string "Just throw a POKé BALL at it and\n"
	.string "try to catch it!\p"
	.string "This won't always work, however.\p"
	.string "A healthy POKéMON can escape.\n"
	.string "You have to be lucky!$"

PalletTown_Lab_Text_KantoOakCompleteMonGuideWasMyDream:
	.string "To make a complete guide on all\n"
	.string "the POKéMON in the world…\p"
	.string "That was my dream!\p"
	.string "But, I'm too old.\n"
	.string "I can't get the job done.\p"
	.string "So, I want you two to fulfill my\n"
	.string "dream for me.\p"
	.string "Get moving, you two.\p"
	.string "This is a great undertaking in\n"
	.string "POKéMON history!$"

PalletTown_Lab_Text_KantoBlankEncyclopedia:
	.string "It's like an encyclopedia, but the\n"
	.string "pages are blank.$"

PalletTown_Lab_Text_KantoThoseArePokeBalls:
	.string "Those are POKé BALLS.\n"
	.string "They contain POKéMON!$"

PalletTown_Lab_Text_KantoOaksLastMon:
	.string "That's PROF. OAK's last POKéMON.$"

PalletTown_Lab_Text_KantoPressStartToOpenMenu:
	.string "Press START to open the MENU!$"

PalletTown_Lab_Text_KantoSaveOptionInMenu:
	.string "The SAVE option is on the MENU.\n"
	.string "Use it regularly.$"

PalletTown_Lab_Text_KantoAllMonTypesHaveStrongAndWeakPoints:
	.string "All POKéMON types have strong and\n"
	.string "weak points against others.$"

PalletTown_Lab_Text_KantoEmailMessage:
	.string "There's an e-mail message here.\p"
	.string "…\p"
	.string "Finally!\n"
	.string "The ultimate TRAINERS of the\l"
	.string "POKéMON LEAGUE are ready to\l"
	.string "take on all comers!\p"
	.string "Bring your best POKéMON and see\n"
	.string "how you rate as a TRAINER!\p"
	.string "POKéMON LEAGUE HQ\n"
	.string "INDIGO PLATEAU\p"
	.string "PROF. OAK, please visit us!\n"
	.string "…$"

PalletTown_Lab_Text_KantoStudyAsOaksAide:
	.string "I study POKéMON as PROF. OAK's\n"
	.string "AIDE.$"

PalletTown_Lab_Text_KantoOakIsAuthorityOnMons:
	.string "PROF. OAK may not look like much,\n"
	.string "but he's the authority on POKéMON.\p"
	.string "Many POKéMON TRAINERS hold him in\n"
	.string "high regard.$"

@ Wayfarer-only lines for states FRLG never reaches (post-opening, retries).
PalletTown_Lab_Text_KantoComplete:
	.string "OAK: Your journey starts now.\p"
	.string "Good luck, {PLAYER}!$"

PalletTown_Lab_Text_KantoParcelMissing:
	.string "OAK: I can't find the PARCEL.\p"
	.string "Bring it here and we'll continue.$"

PalletTown_Lab_Text_KantoBagFull:
	.string "OAK: Make room for five POKé BALLS,\n"
	.string "then speak to me again.$"

PalletTown_Lab_Text_KantoRewardRetry:
	.string "OAK: Let's try that handoff again.\p"
	.string "Speak to me when you're ready.$"

PalletTown_Lab_Text_KantoBlueDex:
	.string "BLUE: I'll fill my POKéDEX\n"
	.string "before you do.$"

PalletTown_Lab_Text_KantoStarterPreview:
	.string "{STR_VAR_1}, a {STR_VAR_2}-type\n"
	.string "POKéMON. Choose this one?$"

PalletTown_Lab_Text_KantoStarterFull:
	.string "OAK: We can't finish the handoff\n"
	.string "yet. Speak to me to try again.$"

PalletTown_Lab_Text_KantoBallUnavailable:
	.string "The BALL is no longer available.$"

PalletTown_Lab_Text_KantoNoUsableMon:
	.string "BLUE: Heal your POKéMON first.\p"
	.string "Then we can battle.$"
#endif

PalletTown_Lab_Text_OakLabDexCheck:
	.string "How is your POKéDEX coming?$"

PalletTown_Lab_Text_OakWelcomeKanto:
	.string "OAK: Ah, {PLAYER}!\n"
	.string "It's good of you to come all this\l"
	.string "way to KANTO.\p"
	.string "What do you think of the trainers\n"
	.string "out here? Pretty tough, huh?$"

PalletTown_Lab_Text_OakLabGoodbye:
	.string "If you're in the area, I hope you\n"
	.string "come visit again.$"

PalletTown_Lab_Text_OakOpenMtSilver:
	.string "OAK: Wow! That's excellent!\p"
	.string "You collected the BADGES of GYMS\n"
	.string "in KANTO. Well done!\p"
	.string "I was right in my assessment of\n"
	.string "you.\p"
	.string "Tell you what, {PLAYER}. I'll make\n"
	.string "arrangements so that you can go\l"
	.string "to MT.SILVER.\p"
	.string "MT.SILVER is a big mountain that\n"
	.string "is home to many wild POKéMON.\p"
	.string "It's too dangerous for your average\n"
	.string "trainer, so it's off limits. But\l"
	.string "we can make an exception in your\l"
	.string "case, {PLAYER}.\p"
	.string "Go up to INDIGO PLATEAU. You can\n"
	.string "reach MT.SILVER from there.$"

PalletTown_Lab_Text_OakOpenIndigoPlateau:
	.string "Oh, and speaking of the INDIGO\n"
	.string "PLATEAU, now that you've earned\l"
	.string "16 badges, the KANTO LEAGUE\l"
	.string "will be ready to accept\l"
	.string "your challenge once again.\p"
	.string "But be warned, LANCE and his\n"
	.string "ELITE FOUR won't go easy on you\l"
	.string "this time.\p"
	.string "Good luck, {PLAYER}!$"

PalletTown_Lab_Text_OakNoKantoBadges:
	.string "OAK: Hmm? You're not collecting\n"
	.string "KANTO GYM BADGES?\p"
	.string "The GYM LEADERS in KANTO are as\n"
	.string "tough as any you battled in JOHTO.\p"
	.string "I recommend that you challenge them.$"

PalletTown_Lab_Text_OakYesKantoBadges:
	.string "OAK: Ah, you're collecting KANTO\n"
	.string "GYM BADGES.\p"
	.string "I imagine that it's hard, but the\n"
	.string "experience is sure to help you.\p"
	.string "Come see me when you get them all.\p"
	.string "I'll have a gift for you.\p"
	.string "Keep trying hard, {PLAYER}!$"

PalletTown_Lab_Text_OaksAssistant1:
	.string "The PROF's POKéMON TALK radio\n"
	.string "program isn't aired here in KANTO.\p"
	.string "It's a shame--I'd like to hear it.$"

PalletTown_Lab_Text_OaksAssistant2:
	.string "Thanks to your work on the POKéDEX,\n"
	.string "the PROF's research is coming along\l"
	.string "great.$"

PalletTown_Lab_Text_OaksAssistant3:
	.string "Don't tell anyone, but PROF.OAK'S\n"
	.string "POKéMON TALK isn't a live broadcast.$"

PalletTown_Lab_Text_OaksLabPC:
	.string "There's an e-mail message on the PC.\p"
	.string "…\p"
	.string "PROF.OAK, how is your research\n"
	.string "coming along?\p"
	.string "I'm still plugging away.\p"
	.string "I heard rumors that {PLAYER} is\n"
	.string "getting quite a reputation.\p"
	.string "I'm delighted to hear that.\p"
	.string "ELM in NEW BARK TOWN 8-)$"
