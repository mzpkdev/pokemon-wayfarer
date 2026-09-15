@ Wayfarer-owned Trainer Tower map entry points.
@
@ The facility runtime owns all persistent records and every active-run field.
@ These wrappers deliberately contain no ordinary Trainer battle, defeat-flag,
@ campaign, e-Reader, or raw saved-state command.

.equ HIDE_TRAINER_DOUBLES1, FLAG_TEMP_2
.equ HIDE_TRAINER_SINGLES, FLAG_TEMP_3
.equ HIDE_TRAINER_KNOCKOUT, FLAG_TEMP_4
.equ HIDE_TRAINER_DOUBLES2, FLAG_TEMP_5
.equ HIDE_OWNER, FLAG_TEMP_6
.equ DISABLE_SINGLES_TRIGGER, VAR_TEMP_E
.equ DISABLE_DOUBLES_TRIGGER, VAR_TEMP_F

WayfarerSevii_TrainerTower_Floor_OnResume::
	setvar VAR_TEMP_2, 0
	ttower_resumetimer
	end

WayfarerSevii_TrainerTower_Floor_OnTransition::
	ttower_initfloor
	switch VAR_RESULT
	case CHALLENGE_TYPE_SINGLE, WayfarerSevii_TrainerTower_Floor_SetObjectsSingles
	case CHALLENGE_TYPE_DOUBLE, WayfarerSevii_TrainerTower_Floor_SetObjectsDoubles
	case CHALLENGE_TYPE_KNOCKOUT, WayfarerSevii_TrainerTower_Floor_SetObjectsKnockout
	setflag HIDE_TRAINER_DOUBLES1
	setflag HIDE_TRAINER_SINGLES
	setflag HIDE_TRAINER_KNOCKOUT
	setflag HIDE_TRAINER_DOUBLES2
	setvar DISABLE_SINGLES_TRIGGER, TRUE
	setvar DISABLE_DOUBLES_TRIGGER, TRUE
	end

WayfarerSevii_TrainerTower_Floor_SetObjectsSingles::
	setflag HIDE_TRAINER_DOUBLES1
	setflag HIDE_TRAINER_KNOCKOUT
	setflag HIDE_TRAINER_DOUBLES2
	setvar DISABLE_DOUBLES_TRIGGER, TRUE
	setobjectxyperm LOCALID_TOWER_TRAINER_SINGLES, 15, 13
	setobjectmovementtype LOCALID_TOWER_TRAINER_SINGLES, MOVEMENT_TYPE_FACE_LEFT
	goto WayfarerSevii_TrainerTower_Floor_HideOwner

WayfarerSevii_TrainerTower_Floor_SetObjectsDoubles::
	setflag HIDE_TRAINER_SINGLES
	setflag HIDE_TRAINER_KNOCKOUT
	setvar DISABLE_SINGLES_TRIGGER, TRUE
	ttower_isfloorcleared
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_TrainerTower_Floor_SetObjectsDoublesCleared
	setobjectxyperm LOCALID_TOWER_TRAINER_DOUBLES1, 10, 12
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES1, MOVEMENT_TYPE_FACE_LEFT
	setobjectxyperm LOCALID_TOWER_TRAINER_DOUBLES2, 10, 13
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES2, MOVEMENT_TYPE_FACE_LEFT
	goto WayfarerSevii_TrainerTower_Floor_HideOwner

WayfarerSevii_TrainerTower_Floor_SetObjectsDoublesCleared::
	setobjectxyperm LOCALID_TOWER_TRAINER_DOUBLES1, 10, 12
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES1, MOVEMENT_TYPE_FACE_DOWN
	setobjectxyperm LOCALID_TOWER_TRAINER_DOUBLES2, 11, 12
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES2, MOVEMENT_TYPE_FACE_DOWN
	goto WayfarerSevii_TrainerTower_Floor_HideOwner

WayfarerSevii_TrainerTower_Floor_SetObjectsKnockout::
	setflag HIDE_TRAINER_DOUBLES2
	setvar DISABLE_DOUBLES_TRIGGER, TRUE
	setobjectxyperm LOCALID_TOWER_TRAINER_DOUBLES1, 10, 10
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES1, MOVEMENT_TYPE_FACE_DOWN
	setobjectxyperm LOCALID_TOWER_TRAINER_SINGLES, 14, 13
	setobjectmovementtype LOCALID_TOWER_TRAINER_SINGLES, MOVEMENT_TYPE_FACE_LEFT
	setobjectxyperm LOCALID_TOWER_TRAINER_KNOCKOUT, 10, 16
	setobjectmovementtype LOCALID_TOWER_TRAINER_KNOCKOUT, MOVEMENT_TYPE_FACE_UP

WayfarerSevii_TrainerTower_Floor_HideOwner::
	setflag HIDE_OWNER
	end

WayfarerSevii_TrainerTower_Floor_OnFrame::
	map_script_2 VAR_TEMP_2, 0, WayfarerSevii_TrainerTower_Floor_Enter
	.2byte 0

WayfarerSevii_TrainerTower_Floor_Enter::
	setvar VAR_TEMP_2, 1
	ttower_isfloorcleared
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_TrainerTower_Floor_EnterEnd
	setvar DISABLE_SINGLES_TRIGGER, TRUE
	setvar DISABLE_DOUBLES_TRIGGER, TRUE

WayfarerSevii_TrainerTower_Floor_EnterEnd::
	end

WayfarerSevii_TrainerTower_Floor_DoublesTrainer1::
	ttower_getchallengetype
	goto_if_eq VAR_RESULT, CHALLENGE_TYPE_KNOCKOUT, WayfarerSevii_TrainerTower_Floor_PostBattleKnockout2
	setvar VAR_0x8006, 0
	goto WayfarerSevii_TrainerTower_Floor_PostBattleSpeech

WayfarerSevii_TrainerTower_Floor_SinglesTrainer::
	ttower_getchallengetype
	goto_if_eq VAR_RESULT, CHALLENGE_TYPE_KNOCKOUT, WayfarerSevii_TrainerTower_Floor_PostBattleKnockout3
	setvar VAR_0x8006, 0
	goto WayfarerSevii_TrainerTower_Floor_PostBattleSpeech

WayfarerSevii_TrainerTower_Floor_KnockoutTrainer::
	setvar VAR_0x8006, 0
	goto WayfarerSevii_TrainerTower_Floor_PostBattleSpeech

WayfarerSevii_TrainerTower_Floor_DoublesTrainer2::
	setvar VAR_0x8006, 1
	goto WayfarerSevii_TrainerTower_Floor_PostBattleSpeech

WayfarerSevii_TrainerTower_Floor_PostBattleKnockout2::
	setvar VAR_0x8006, 1
	goto WayfarerSevii_TrainerTower_Floor_PostBattleSpeech

WayfarerSevii_TrainerTower_Floor_PostBattleKnockout3::
	setvar VAR_0x8006, 2

WayfarerSevii_TrainerTower_Floor_PostBattleSpeech::
	lock
	faceplayer
	ttower_getspeech TRAINER_TOWER_TEXT_AFTER, VAR_0x8006
	msgbox gStringVar4
	release
	end

WayfarerSevii_TrainerTower_Floor_Owner::
	goto WayfarerSevii_TrainerTower_Roof_Owner

WayfarerSevii_TrainerTower_Floor_SingleBattleTrigger::
	lockall
	setvar VAR_TEMP_3, 0
	goto WayfarerSevii_TrainerTower_Floor_TriggerBattle

WayfarerSevii_TrainerTower_Floor_DoubleBattleTriggerTop::
	lockall
	setvar VAR_TEMP_3, 0
	goto WayfarerSevii_TrainerTower_Floor_CheckDoubleEligibility

WayfarerSevii_TrainerTower_Floor_DoubleBattleTriggerBottom::
	lockall
	setvar VAR_TEMP_3, 1
	goto WayfarerSevii_TrainerTower_Floor_CheckDoubleEligibility

WayfarerSevii_TrainerTower_Floor_CheckDoubleEligibility::
	ttower_checkdoubles
	goto_if_eq VAR_RESULT, PLAYER_HAS_TWO_USABLE_MONS, WayfarerSevii_TrainerTower_Floor_TriggerBattle
	msgbox WayfarerSevii_TrainerTower_Text_NeedTwoMons
	releaseall
	end

WayfarerSevii_TrainerTower_Floor_TriggerBattle::
	ttower_getchallengetype
	switch VAR_RESULT
	case CHALLENGE_TYPE_SINGLE, WayfarerSevii_TrainerTower_Floor_DoSingleBattle
	case CHALLENGE_TYPE_DOUBLE, WayfarerSevii_TrainerTower_Floor_DoDoubleBattle
	case CHALLENGE_TYPE_KNOCKOUT, WayfarerSevii_TrainerTower_Floor_DoKnockoutBattle

WayfarerSevii_TrainerTower_Floor_DoSingleBattle::
	ttower_encountermusic
	ttower_getspeech TRAINER_TOWER_TEXT_INTRO, 0
	goto WayfarerSevii_TrainerTower_Floor_DoBattle

WayfarerSevii_TrainerTower_Floor_DoDoubleBattle::
	ttower_encountermusic
	ttower_getspeech TRAINER_TOWER_TEXT_INTRO, VAR_TEMP_3
	msgbox gStringVar4
	addvar VAR_TEMP_3, 1
	goto_if_eq VAR_TEMP_3, 1, WayfarerSevii_TrainerTower_Floor_DoDoubleSecondIntro
	setvar VAR_TEMP_3, 0

WayfarerSevii_TrainerTower_Floor_DoDoubleSecondIntro::
	ttower_getspeech TRAINER_TOWER_TEXT_INTRO, VAR_TEMP_3
	goto WayfarerSevii_TrainerTower_Floor_DoBattle

WayfarerSevii_TrainerTower_Floor_DoKnockoutBattle::
	ttower_encountermusic
	ttower_getspeech TRAINER_TOWER_TEXT_INTRO, VAR_TEMP_1

WayfarerSevii_TrainerTower_Floor_DoBattle::
	msgbox gStringVar4
	ttower_dobattle
	waitstate
	switch VAR_RESULT
	case B_OUTCOME_WON, WayfarerSevii_TrainerTower_Floor_WonBattle
	case B_OUTCOME_LOST, WayfarerSevii_TrainerTower_Floor_LostBattle
	case B_OUTCOME_DREW, WayfarerSevii_TrainerTower_Floor_LostBattle
	case B_OUTCOME_FORFEITED, WayfarerSevii_TrainerTower_Floor_LostBattle

WayfarerSevii_TrainerTower_Floor_WonBattle::
	ttower_getchallengetype
	goto_if_ne VAR_RESULT, CHALLENGE_TYPE_KNOCKOUT, WayfarerSevii_TrainerTower_Floor_SetFloorCleared
	switch VAR_TEMP_1
	case 0, WayfarerSevii_TrainerTower_Floor_NextKnockoutBattle
	case 1, WayfarerSevii_TrainerTower_Floor_NextKnockoutBattle

WayfarerSevii_TrainerTower_Floor_SetFloorCleared::
	ttower_clearedfloor
	ttower_getchallengetype
	goto_if_eq VAR_RESULT, CHALLENGE_TYPE_DOUBLE, WayfarerSevii_TrainerTower_Floor_MoveDoublesOutOfWay
	goto_if_eq VAR_RESULT, CHALLENGE_TYPE_KNOCKOUT, WayfarerSevii_TrainerTower_Floor_MoveLastKnockoutOutOfWay
	releaseall
	end

WayfarerSevii_TrainerTower_Floor_MoveDoublesOutOfWay::
	applymovement LOCALID_TOWER_TRAINER_DOUBLES2, WayfarerSevii_TrainerTower_Movement_DoublesTrainer2OutOfWay
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_DOUBLES1, WayfarerSevii_TrainerTower_Movement_FaceDown
	waitmovement 0
	releaseall
	end

WayfarerSevii_TrainerTower_Floor_MoveLastKnockoutOutOfWay::
	applymovement LOCALID_TOWER_TRAINER_SINGLES, WayfarerSevii_TrainerTower_Movement_LastKnockoutOutOfWay
	waitmovement 0
	releaseall
	end

WayfarerSevii_TrainerTower_Floor_NextKnockoutBattle::
	addvar VAR_TEMP_1, 1
	goto WayfarerSevii_TrainerTower_Floor_DoKnockoutBattle

WayfarerSevii_TrainerTower_Floor_LostBattle::
	ttower_setlost
	warp MAP_TRAINER_TOWER_LOBBY, 9, 7
	waitstate
	end

WayfarerSevii_TrainerTower_Roof_Owner::
	lock
	faceplayer
	ttower_getownerstate
	goto_if_eq VAR_RESULT, 0, WayfarerSevii_TrainerTower_Roof_Arrived
	goto_if_eq VAR_RESULT, 1, WayfarerSevii_TrainerTower_Roof_GivePrize
	msgbox WayfarerSevii_TrainerTower_Text_ComeAgain
	release
	end

WayfarerSevii_TrainerTower_Roof_Arrived::
	msgbox WayfarerSevii_TrainerTower_Text_OwnerGreeting

WayfarerSevii_TrainerTower_Roof_GivePrize::
	ttower_checkfinaltime
	ttower_giveprize
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_TrainerTower_Roof_PrizeReceived
	msgbox WayfarerSevii_TrainerTower_Text_PrizePending
	goto WayfarerSevii_TrainerTower_Roof_End

WayfarerSevii_TrainerTower_Roof_PrizeReceived::
	msgbox WayfarerSevii_TrainerTower_Text_PrizeReceived

WayfarerSevii_TrainerTower_Roof_End::
	release
	end

WayfarerSevii_TrainerTower_Lobby_OnResume::
	ttower_resumetimer
	end

WayfarerSevii_TrainerTower_Lobby_OnReturnToField::
	end

WayfarerSevii_TrainerTower_Lobby_OnTransition::
	end

WayfarerSevii_TrainerTower_Lobby_OnLoad::
	end

WayfarerSevii_TrainerTower_Lobby_OnFrame::
	end

WayfarerSevii_TrainerTower_Lobby_Nurse::
	lock
	faceplayer
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CHECK_ACTIVE
	special CallTrainerTowerFunc
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_TrainerTower_Lobby_NurseHeal
	msgbox WayfarerSevii_TrainerTower_Text_AbandonPrompt, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, WayfarerSevii_TrainerTower_Lobby_NurseEnd
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_ABANDON_CHALLENGE
	special CallTrainerTowerFunc

WayfarerSevii_TrainerTower_Lobby_NurseHeal::
	special HealPlayerParty
	msgbox WayfarerSevii_TrainerTower_Text_Healed

WayfarerSevii_TrainerTower_Lobby_NurseEnd::
	release
	end

WayfarerSevii_TrainerTower_Lobby_MartClerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart WayfarerSevii_TrainerTower_LobbyMartItems
	msgbox gText_PleaseComeAgain
	release
	end

WayfarerSevii_TrainerTower_Lobby_Receptionist::
	lock
	faceplayer
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CHECK_PENDING_PRIZE
	special CallTrainerTowerFunc
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_TrainerTower_Lobby_ClaimPendingPrize
	goto WayfarerSevii_TrainerTower_Lobby_OfferChallenge

WayfarerSevii_TrainerTower_Lobby_ClaimPendingPrize::
	msgbox WayfarerSevii_TrainerTower_Text_PendingPrize, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, WayfarerSevii_TrainerTower_Lobby_ReceptionistEnd
	giveitem VAR_WAYFARER_SEVII_TRAINER_TOWER_PENDING_PRIZE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_TrainerTower_Lobby_ClaimNoRoom
	setvar VAR_WAYFARER_SEVII_TRAINER_TOWER_PENDING_PRIZE, ITEM_NONE
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CLAIM_PENDING_PRIZE
	special CallTrainerTowerFunc
	goto WayfarerSevii_TrainerTower_Lobby_ReceptionistEnd

WayfarerSevii_TrainerTower_Lobby_ClaimNoRoom::
	msgbox WayfarerSevii_TrainerTower_Text_PrizePending
	goto WayfarerSevii_TrainerTower_Lobby_ReceptionistEnd

WayfarerSevii_TrainerTower_Lobby_OfferChallenge::
	msgbox WayfarerSevii_TrainerTower_Text_ChallengePrompt, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, WayfarerSevii_TrainerTower_Lobby_ReceptionistEnd
	multichoice 13, 3, MULTI_TRAINER_TOWER_MODE, FALSE
	goto_if_eq VAR_RESULT, 4, WayfarerSevii_TrainerTower_Lobby_ReceptionistEnd
	copyvar VAR_0x8005, VAR_RESULT
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CHECK_ELIGIBILITY
	special CallTrainerTowerFunc
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_TrainerTower_Lobby_StartChallenge
	msgbox WayfarerSevii_TrainerTower_Text_Ineligible
	goto WayfarerSevii_TrainerTower_Lobby_ReceptionistEnd

WayfarerSevii_TrainerTower_Lobby_StartChallenge::
	special HealPlayerParty
	msgbox WayfarerSevii_TrainerTower_Text_StartClock
	ttower_startchallenge
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_TrainerTower_Lobby_ReceptionistEnd
	msgbox WayfarerSevii_TrainerTower_Text_StartFailed

WayfarerSevii_TrainerTower_Lobby_ReceptionistEnd::
	release
	end

WayfarerSevii_TrainerTower_Lobby_EntryTrigger::
	goto WayfarerSevii_TrainerTower_Lobby_OfferChallenge

WayfarerSevii_TrainerTower_Lobby_ShowRecords::
	lockall
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_SHOW_RESULTS
	special CallTrainerTowerFunc
	waitbuttonpress
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CLOSE_RESULTS
	special CallTrainerTowerFunc
	releaseall
	end

WayfarerSevii_TrainerTower_Lobby_CooltrainerF::
	msgbox WayfarerSevii_TrainerTower_Text_FlavorOne, MSGBOX_NPC
	end

WayfarerSevii_TrainerTower_Lobby_BaldingMan::
	msgbox WayfarerSevii_TrainerTower_Text_FlavorTwo, MSGBOX_NPC
	end

WayfarerSevii_TrainerTower_Elevator_FloorSelect::
	lockall
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CHECK_ACTIVE
	special CallTrainerTowerFunc
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_TrainerTower_Elevator_ReturnLobby
	msgbox WayfarerSevii_TrainerTower_Text_AbandonPrompt, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, WayfarerSevii_TrainerTower_Elevator_End
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_ABANDON_CHALLENGE
	special CallTrainerTowerFunc

WayfarerSevii_TrainerTower_Elevator_ReturnLobby::
	warp MAP_TRAINER_TOWER_LOBBY, 17, 8
	waitstate

WayfarerSevii_TrainerTower_Elevator_End::
	releaseall
	end

WayfarerSevii_TrainerTower_Lobby_ConfirmExit::
	lockall
	msgbox WayfarerSevii_TrainerTower_Text_AbandonPrompt, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, WayfarerSevii_TrainerTower_Lobby_ConfirmExitEnd
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_ABANDON_CHALLENGE
	special CallTrainerTowerFunc
	warp MAP_SEVEN_ISLAND_TRAINER_TOWER, 0
	waitstate
	end

WayfarerSevii_TrainerTower_Lobby_ConfirmExitEnd::
	releaseall
	end

WayfarerSevii_TrainerTower_Movement_DoublesTrainer2OutOfWay::
	walk_right
	step_end

WayfarerSevii_TrainerTower_Movement_LastKnockoutOutOfWay::
	walk_up
	face_down
	step_end

WayfarerSevii_TrainerTower_Movement_FaceDown::
	face_down
	step_end

	.align 2
WayfarerSevii_TrainerTower_LobbyMartItems::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_MAX_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_NONE

WayfarerSevii_TrainerTower_Text_Healed::
	.string "Your POKéMON are fighting fit.$"

WayfarerSevii_TrainerTower_Text_NeedTwoMons::
	.string "You need two usable POKéMON for\nthis battle.$"

WayfarerSevii_TrainerTower_Text_OwnerGreeting::
	.string "You've reached the roof.\nAn excellent challenge!$"

WayfarerSevii_TrainerTower_Text_PrizeReceived::
	.string "Thank you for taking the prize.$"

WayfarerSevii_TrainerTower_Text_PrizePending::
	.string "Your prize is waiting with the\nreceptionist until you make room.$"

WayfarerSevii_TrainerTower_Text_ComeAgain::
	.string "Come back with an even better time.$"

WayfarerSevii_TrainerTower_Text_PendingPrize::
	.string "You have a Trainer Tower prize waiting.\nWould you like to claim it?$"

WayfarerSevii_TrainerTower_Text_ChallengePrompt::
	.string "Would you like to challenge the\nwaiting TRAINERS?$"

WayfarerSevii_TrainerTower_Text_Ineligible::
	.string "Your current party cannot enter\nthat format.$"

WayfarerSevii_TrainerTower_Text_StartClock::
	.string "On your marks… Get set… Go!$"

WayfarerSevii_TrainerTower_Text_StartFailed::
	.string "The challenge could not be started.$"

WayfarerSevii_TrainerTower_Text_FlavorOne::
	.string "I wonder what kinds of TRAINERS\nare waiting upstairs.$"

WayfarerSevii_TrainerTower_Text_FlavorTwo::
	.string "Those stairs are tougher than\nany battle.$"

WayfarerSevii_TrainerTower_Text_AbandonPrompt::
	.string "Leave the challenge and return\nto the lobby?$"
