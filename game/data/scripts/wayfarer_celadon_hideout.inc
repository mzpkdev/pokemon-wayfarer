@ Wayfarer-only Celadon Rocket Hideout. The FRLG map scripts remain compiled
@ only by the FRLG target; these labels supply the selected interiors here.

CeladonGameCorner_EventScript_HideoutGrunt::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS, CeladonGameCorner_EventScript_HideoutGrunt_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, CeladonGameCorner_EventScript_HideoutGrunt_NoParty
CeladonGameCorner_EventScript_HideoutGrunt_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS, CeladonGameCorner_Text_HideoutGruntIntro, CeladonGameCorner_Text_HideoutGruntDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox CeladonGameCorner_Text_HideoutGruntPostBattle, MSGBOX_AUTOCLOSE
	end

CeladonGameCorner_EventScript_HideoutGrunt_NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

CeladonGameCorner_EventScript_RocketHideoutPoster::
	lockall
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GRUNT_7_HNS, CeladonGameCorner_EventScript_RocketHideoutPanelGuarded
	goto_if_set FLAG_CELADON_ROCKET_HIDEOUT_ENTRANCE_OPEN_HNS, CeladonGameCorner_EventScript_RocketHideoutPanelOpen
	msgbox CeladonGameCorner_Text_HideoutPanelFound, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CeladonGameCorner_EventScript_RocketHideoutPanelCancel
	playse SE_UNLOCK
	setflag FLAG_CELADON_ROCKET_HIDEOUT_ENTRANCE_OPEN_HNS
	msgbox CeladonGameCorner_Text_HideoutPanelOpened, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	warp MAP_ROCKET_HIDEOUT_B1F, 12, 2
	waitstate
	releaseall
	end

CeladonGameCorner_EventScript_RocketHideoutPanelOpen::
	msgbox CeladonGameCorner_Text_HideoutPanelEnter, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CeladonGameCorner_EventScript_RocketHideoutPanelCancel
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	warp MAP_ROCKET_HIDEOUT_B1F, 12, 2
	waitstate
	releaseall
	end

CeladonGameCorner_EventScript_RocketHideoutPanelGuarded::
	msgbox CeladonGameCorner_Text_HideoutPanelGuarded, MSGBOX_DEFAULT

CeladonGameCorner_EventScript_RocketHideoutPanelCancel::
	releaseall
	end

RocketHideout_B1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, WayfarerRocketHideout_B1F_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, RocketHideout_B1F_OnTransition
	.byte 0

WayfarerRocketHideout_B1F_OnLoad::
	setdynamicwarp MAP_CELADON_CITY_GAME_CORNER_HNS, 255, 11, 2
	call_if_not_defeated TRAINER_CELADON_HIDEOUT_GRUNT_12_HNS, RocketHideout_B1F_EventScript_SetBarrier
	end

RocketHideout_B1F_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_ROCKET_HIDEOUT_B1F
	end

RocketHideout_B1F_EventScript_Grunt1::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_8_HNS, RocketHideout_B1F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B1F_EventScript_Grunt1NoParty
RocketHideout_B1F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_8_HNS, RocketHideout_B1F_Text_Grunt1Intro, RocketHideout_B1F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B1F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt1NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B1F_EventScript_Grunt2::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_9_HNS, RocketHideout_B1F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B1F_EventScript_Grunt2NoParty
RocketHideout_B1F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_9_HNS, RocketHideout_B1F_Text_Grunt2Intro, RocketHideout_B1F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B1F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt2NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B1F_EventScript_Grunt3::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_10_HNS, RocketHideout_B1F_EventScript_Grunt3_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B1F_EventScript_Grunt3NoParty
RocketHideout_B1F_EventScript_Grunt3_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_10_HNS, RocketHideout_B1F_Text_Grunt3Intro, RocketHideout_B1F_Text_Grunt3Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B1F_Text_Grunt3PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt3NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B1F_EventScript_Grunt4::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_11_HNS, RocketHideout_B1F_EventScript_Grunt4_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B1F_EventScript_Grunt4NoParty
RocketHideout_B1F_EventScript_Grunt4_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_11_HNS, RocketHideout_B1F_Text_Grunt4Intro, RocketHideout_B1F_Text_Grunt4Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B1F_Text_Grunt4PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt4NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B1F_EventScript_Grunt5::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_12_HNS, RocketHideout_B1F_EventScript_Grunt5_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B1F_EventScript_Grunt5NoParty
RocketHideout_B1F_EventScript_Grunt5_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_12_HNS, RocketHideout_B1F_Text_Grunt5Intro, RocketHideout_B1F_Text_Grunt5Defeat, RocketHideout_B1F_EventScript_DefeatedGrunt5
	end

RocketHideout_B1F_EventScript_Grunt5NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B1F_EventScript_DefeatedGrunt5::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	call RocketHideout_B1F_EventScript_RemoveBarrier
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	release
	end

RocketHideout_B1F_EventScript_SetBarrier::
	setmetatile 20, 19, METATILE_SilphCo_HideoutBarrier_TopLeft_Floor, 1
	setmetatile 21, 19, METATILE_SilphCo_HideoutBarrier_TopRight_Floor, 1
	setmetatile 20, 20, METATILE_SilphCo_HideoutBarrier_BottomLeft, 1
	setmetatile 21, 20, METATILE_SilphCo_HideoutBarrier_BottomRight, 1
	setmetatile 20, 21, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	setmetatile 21, 21, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	return

RocketHideout_B1F_EventScript_RemoveBarrier::
	setmetatile 20, 19, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	setmetatile 21, 19, METATILE_SilphCo_HideoutFloor, 0
	setmetatile 20, 20, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	setmetatile 21, 20, METATILE_SilphCo_HideoutFloor, 0
	setmetatile 20, 21, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	setmetatile 21, 21, METATILE_SilphCo_HideoutFloor, 0
	return

RocketHideout_B2F_Frlg_MapScripts::
	.byte 0

RocketHideout_B2F_EventScript_Grunt::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_13_HNS, RocketHideout_B2F_EventScript_Grunt_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B2F_EventScript_GruntNoParty
RocketHideout_B2F_EventScript_Grunt_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_13_HNS, RocketHideout_B2F_Text_GruntIntro, RocketHideout_B2F_Text_GruntDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B2F_Text_GruntPostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B2F_EventScript_GruntNoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B3F_Frlg_MapScripts::
	.byte 0

RocketHideout_B3F_EventScript_Grunt1::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_14_HNS, RocketHideout_B3F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B3F_EventScript_Grunt1NoParty
RocketHideout_B3F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_14_HNS, RocketHideout_B3F_Text_Grunt1Intro, RocketHideout_B3F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B3F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B3F_EventScript_Grunt1NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B3F_EventScript_Grunt2::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_15_HNS, RocketHideout_B3F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B3F_EventScript_Grunt2NoParty
RocketHideout_B3F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_15_HNS, RocketHideout_B3F_Text_Grunt2Intro, RocketHideout_B3F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B3F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B3F_EventScript_Grunt2NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

.equ NUM_DOOR_GRUNTS_DEFEATED, VAR_TEMP_1

RocketHideout_B4F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, RocketHideout_B4F_OnLoad
	.byte 0

RocketHideout_B4F_OnLoad::
	setvar NUM_DOOR_GRUNTS_DEFEATED, 0
	call_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_16_HNS, RocketHideout_B4F_EventScript_CountGruntDefeated
	call_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_17_HNS, RocketHideout_B4F_EventScript_CountGruntDefeated
	call_if_ne NUM_DOOR_GRUNTS_DEFEATED, 2, RocketHideout_B4F_EventScript_SetBarrier
	call RocketHideout_B4F_EventScript_RefreshLiftKey
	call RocketHideout_B4F_EventScript_RefreshSilphScope
	end

RocketHideout_B4F_EventScript_CountGruntDefeated::
	addvar NUM_DOOR_GRUNTS_DEFEATED, 1
	return

RocketHideout_B4F_EventScript_Giovanni::
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GRUNT_16_HNS, RocketHideout_B4F_EventScript_GiovanniGuarded
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GRUNT_17_HNS, RocketHideout_B4F_EventScript_GiovanniGuarded
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B4F_EventScript_GiovanniNoParty
	lock
	faceplayer
	message RocketHideout_B4F_Text_GiovanniIntro
	waitmessage
	playbgm MUS_RG_ENCOUNTER_ROCKET, 0
	waitbuttonpress
	trainerbattle_no_intro TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS, RocketHideout_B4F_Text_GiovanniDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B4F_Text_GiovanniPostBattle
	fadescreen FADE_TO_BLACK
	closemessage
	setflag FLAG_HIDE_HIDEOUT_GIOVANNI
	removeobject LOCALID_HIDEOUT_GIOVANNI
	call RocketHideout_B4F_EventScript_RefreshSilphScope
	fadescreen FADE_FROM_BLACK
	release
	end

RocketHideout_B4F_EventScript_GiovanniGuarded::
	msgbox RocketHideout_B4F_Text_GiovanniGuarded, MSGBOX_DEFAULT
	end

RocketHideout_B4F_EventScript_GiovanniNoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B4F_EventScript_SilphScope::
	lock
	faceplayer
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS, RocketHideout_B4F_EventScript_SilphScopeNotReady
	checkitem ITEM_SILPH_SCOPE
	goto_if_eq VAR_RESULT, TRUE, RocketHideout_B4F_EventScript_SilphScopeCommit
	additem ITEM_SILPH_SCOPE
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B4F_EventScript_RewardBagFull
	msgreceiveditem RocketHideout_B4F_Text_ReceivedSilphScope, ITEM_SILPH_SCOPE, 1, MUS_RG_OBTAIN_KEY_ITEM

RocketHideout_B4F_EventScript_SilphScopeCommit::
	setflag FLAG_CELADON_ROCKET_HIDEOUT_SILPH_SCOPE_RECEIVED_HNS
	setflag FLAG_HIDE_SILPH_SCOPE
	removeobject LOCALID_SILPH_SCOPE
	release
	end

RocketHideout_B4F_EventScript_SilphScopeNotReady::
	msgbox RocketHideout_B4F_Text_SilphScopeNotReady, MSGBOX_DEFAULT
	release
	end

RocketHideout_B4F_EventScript_Grunt1::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_18_HNS, RocketHideout_B4F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B4F_EventScript_Grunt1NoParty
RocketHideout_B4F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_18_HNS, RocketHideout_B4F_Text_Grunt1Intro, RocketHideout_B4F_Text_Grunt1Defeat, RocketHideout_B4F_EventScript_DefeatedGrunt1
	end

RocketHideout_B4F_EventScript_Grunt1NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B4F_EventScript_DefeatedGrunt1::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	msgbox RocketHideout_B4F_Text_Grunt1PostBattle
	call RocketHideout_B4F_EventScript_RefreshLiftKey
	release
	end

RocketHideout_B4F_EventScript_LiftKey::
	lock
	faceplayer
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GRUNT_18_HNS, RocketHideout_B4F_EventScript_LiftKeyNotReady
	checkitem ITEM_LIFT_KEY
	goto_if_eq VAR_RESULT, TRUE, RocketHideout_B4F_EventScript_LiftKeyCommit
	additem ITEM_LIFT_KEY
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B4F_EventScript_RewardBagFull
	msgreceiveditem RocketHideout_B4F_Text_ReceivedLiftKey, ITEM_LIFT_KEY, 1, MUS_RG_OBTAIN_KEY_ITEM

RocketHideout_B4F_EventScript_LiftKeyCommit::
	setflag FLAG_CELADON_ROCKET_HIDEOUT_LIFT_KEY_RECEIVED_HNS
	setflag FLAG_CAN_USE_ROCKET_HIDEOUT_LIFT
	setflag FLAG_HIDE_LIFT_KEY
	removeobject LOCALID_LIFT_KEY
	release
	end

RocketHideout_B4F_EventScript_LiftKeyNotReady::
	msgbox RocketHideout_B4F_Text_LiftKeyNotReady, MSGBOX_DEFAULT
	release
	end

RocketHideout_B4F_EventScript_RewardBagFull::
	call Common_EventScript_BagIsFull
	release
	end

RocketHideout_B4F_EventScript_Grunt2::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_16_HNS, RocketHideout_B4F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B4F_EventScript_Grunt2NoParty
RocketHideout_B4F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_16_HNS, RocketHideout_B4F_Text_Grunt2Intro, RocketHideout_B4F_Text_Grunt2Defeat, RocketHideout_B4F_EventScript_DefeatedGrunt2
	end

RocketHideout_B4F_EventScript_Grunt2NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B4F_EventScript_DefeatedGrunt2::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	goto RocketHideout_B4F_EventScript_UpdatePairedBarrier

RocketHideout_B4F_EventScript_Grunt3::
	goto_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_17_HNS, RocketHideout_B4F_EventScript_Grunt3_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, RocketHideout_B4F_EventScript_Grunt3NoParty
RocketHideout_B4F_EventScript_Grunt3_TrainerBattle::
	trainerbattle_single TRAINER_CELADON_HIDEOUT_GRUNT_17_HNS, RocketHideout_B4F_Text_Grunt3Intro, RocketHideout_B4F_Text_Grunt3Defeat, RocketHideout_B4F_EventScript_DefeatedGrunt3
	end

RocketHideout_B4F_EventScript_Grunt3NoParty::
	msgbox WayfarerCeladonHideout_Text_NoUsableParty, MSGBOX_DEFAULT
	end

RocketHideout_B4F_EventScript_DefeatedGrunt3::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerCeladonHideout_EventScript_EndAfterBattle
	goto RocketHideout_B4F_EventScript_UpdatePairedBarrier

RocketHideout_B4F_EventScript_UpdatePairedBarrier::
	setvar NUM_DOOR_GRUNTS_DEFEATED, 0
	call_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_16_HNS, RocketHideout_B4F_EventScript_CountGruntDefeated
	call_if_defeated TRAINER_CELADON_HIDEOUT_GRUNT_17_HNS, RocketHideout_B4F_EventScript_CountGruntDefeated
	call_if_eq NUM_DOOR_GRUNTS_DEFEATED, 2, RocketHideout_B4F_EventScript_RemoveBarrier
	call_if_eq NUM_DOOR_GRUNTS_DEFEATED, 2, RocketHideout_B4F_EventScript_DrawMapForBarrierRemoval
	release
	end

RocketHideout_B4F_EventScript_DrawMapForBarrierRemoval::
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	return

RocketHideout_B4F_EventScript_SetBarrier::
	setmetatile 17, 12, METATILE_SilphCo_HideoutBarrier_TopLeft_Floor, 1
	setmetatile 18, 12, METATILE_SilphCo_HideoutBarrier_TopRight_Floor, 1
	setmetatile 17, 13, METATILE_SilphCo_HideoutBarrier_BottomLeft, 1
	setmetatile 18, 13, METATILE_SilphCo_HideoutBarrier_BottomRight, 1
	setmetatile 17, 14, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	setmetatile 18, 14, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	return

RocketHideout_B4F_EventScript_RemoveBarrier::
	setmetatile 17, 12, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	setmetatile 18, 12, METATILE_SilphCo_HideoutFloor, 0
	setmetatile 17, 13, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	setmetatile 18, 13, METATILE_SilphCo_HideoutFloor, 0
	setmetatile 17, 14, METATILE_SilphCo_HideoutFloor_ShadeFull, 0
	setmetatile 18, 14, METATILE_SilphCo_HideoutFloor, 0
	return

RocketHideout_B4F_EventScript_RefreshLiftKey::
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GRUNT_18_HNS, RocketHideout_B4F_EventScript_HideLiftKey
	goto_if_set FLAG_CELADON_ROCKET_HIDEOUT_LIFT_KEY_RECEIVED_HNS, RocketHideout_B4F_EventScript_HideLiftKey
	clearflag FLAG_HIDE_LIFT_KEY
	addobject LOCALID_LIFT_KEY
	return

RocketHideout_B4F_EventScript_HideLiftKey::
	setflag FLAG_HIDE_LIFT_KEY
	removeobject LOCALID_LIFT_KEY
	return

RocketHideout_B4F_EventScript_RefreshSilphScope::
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS, RocketHideout_B4F_EventScript_HideSilphScope
	goto_if_set FLAG_CELADON_ROCKET_HIDEOUT_SILPH_SCOPE_RECEIVED_HNS, RocketHideout_B4F_EventScript_HideSilphScope
	clearflag FLAG_HIDE_SILPH_SCOPE
	addobject LOCALID_SILPH_SCOPE
	return

RocketHideout_B4F_EventScript_HideSilphScope::
	setflag FLAG_HIDE_SILPH_SCOPE
	removeobject LOCALID_SILPH_SCOPE
	return

RocketHideout_Elevator_Frlg_MapScripts::
	.byte 0

RocketHideout_Elevator_EventScript_FloorSelect::
	lockall
	goto_if_unset FLAG_CAN_USE_ROCKET_HIDEOUT_LIFT, RocketHideout_Elevator_EventScript_NeedKey
	specialvar VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR, WayfarerGetRocketHideoutElevatorFloorForScript
	copyvar VAR_0x8005, VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR
	special DrawElevatorCurrentFloorWindow
	message gText_WantWhichFloor
	waitmessage
	specialvar VAR_RESULT, InitElevatorFloorSelectMenuPos
	switch VAR_RESULT
	case 0, RocketHideout_Elevator_EventScript_FloorSelectFromB1F
	case 1, RocketHideout_Elevator_EventScript_FloorSelectFromB2F
	case 2, RocketHideout_Elevator_EventScript_FloorSelectFromB4F
	end

RocketHideout_Elevator_EventScript_FloorSelectFromB1F::
	multichoicedefault 0, 0, MULTI_ROCKET_HIDEOUT_ELEVATOR, 0, FALSE
	goto RocketHideout_Elevator_EventScript_ChooseFloor
	end

RocketHideout_Elevator_EventScript_FloorSelectFromB2F::
	multichoicedefault 0, 0, MULTI_ROCKET_HIDEOUT_ELEVATOR, 1, FALSE
	goto RocketHideout_Elevator_EventScript_ChooseFloor
	end

RocketHideout_Elevator_EventScript_FloorSelectFromB4F::
	multichoicedefault 0, 0, MULTI_ROCKET_HIDEOUT_ELEVATOR, 2, FALSE
	goto RocketHideout_Elevator_EventScript_ChooseFloor
	end

RocketHideout_Elevator_EventScript_ChooseFloor::
	switch VAR_RESULT
	case 0, RocketHideout_Elevator_EventScript_ToB1F
	case 1, RocketHideout_Elevator_EventScript_ToB2F
	case 2, RocketHideout_Elevator_EventScript_ToB4F
	case 3, RocketHideout_Elevator_EventScript_ExitFloorSelect
	case 127, RocketHideout_Elevator_EventScript_ExitFloorSelect
	end

RocketHideout_Elevator_EventScript_ToB1F::
	setvar VAR_0x8006, 3
	setdynamicwarp MAP_ROCKET_HIDEOUT_B1F, 255, 24, 25
	goto_if_eq VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR, 3, RocketHideout_Elevator_EventScript_ExitFloorSelect
	call RocketHideout_Elevator_EventScript_MoveElevator
	setvar VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR, 3
	goto RocketHideout_Elevator_EventScript_ExitFloorSelect
	end

RocketHideout_Elevator_EventScript_ToB2F::
	setvar VAR_0x8006, 2
	setdynamicwarp MAP_ROCKET_HIDEOUT_B2F, 255, 28, 16
	goto_if_eq VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR, 2, RocketHideout_Elevator_EventScript_ExitFloorSelect
	call RocketHideout_Elevator_EventScript_MoveElevator
	setvar VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR, 2
	goto RocketHideout_Elevator_EventScript_ExitFloorSelect
	end

RocketHideout_Elevator_EventScript_ToB4F::
	setvar VAR_0x8006, 0
	setdynamicwarp MAP_ROCKET_HIDEOUT_B4F, 255, 20, 23
	goto_if_eq VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR, 0, RocketHideout_Elevator_EventScript_ExitFloorSelect
	call RocketHideout_Elevator_EventScript_MoveElevator
	setvar VAR_CELADON_ROCKET_HIDEOUT_ELEVATOR_FLOOR, 0
	goto RocketHideout_Elevator_EventScript_ExitFloorSelect
	end

RocketHideout_Elevator_EventScript_ExitFloorSelect::
	special CloseElevatorCurrentFloorWindow
	releaseall
	end

RocketHideout_Elevator_EventScript_MoveElevator::
	special CloseElevatorCurrentFloorWindow
	closemessage
	waitse
	special AnimateElevator
	waitstate
	return

RocketHideout_Elevator_EventScript_NeedKey::
	msgbox RocketHideout_Elevator_Text_AppearsToNeedAKey
	releaseall
	end

RocketHideout_Elevator_Text_AppearsToNeedAKey::
	.string "It appears to need a key.$"

WayfarerCeladonHideout_EventScript_EndAfterBattle::
	releaseall
	end

WayfarerCeladonHideout_Text_NoUsableParty::
	.string "You have no POKéMON ready to\n"
	.string "battle. Get lost!$"

CeladonGameCorner_Text_HideoutGruntIntro::
	.string "Hey! What are you snooping\n"
	.string "around for?$"

CeladonGameCorner_Text_HideoutGruntDefeat::
	.string "Ouch!$"

CeladonGameCorner_Text_HideoutGruntPostBattle::
	.string "Our secret is safe...\n"
	.string "Or so I thought!$"

CeladonGameCorner_Text_HideoutPanelGuarded::
	.string "A seam in the wall catches your\n"
	.string "eye, but it will not move.$"

CeladonGameCorner_Text_HideoutPanelFound::
	.string "A concealed panel hides a switch.\p"
	.string "Push it?$"

CeladonGameCorner_Text_HideoutPanelOpened::
	.string "The wall slides aside, revealing\n"
	.string "a hidden passage.$"

CeladonGameCorner_Text_HideoutPanelEnter::
	.string "The hidden passage is open.\p"
	.string "Enter it?$"

RocketHideout_B1F_Text_Grunt1Intro::
	.string "Who are you?\n"
	.string "How did you get here?$"

RocketHideout_B1F_Text_Grunt1Defeat::
	.string "Oww!\n"
	.string "Beaten!$"

RocketHideout_B1F_Text_Grunt1PostBattle::
	.string "Blast it... You're dissing TEAM\n"
	.string "ROCKET, aren't you?$"

RocketHideout_B1F_Text_Grunt2Intro::
	.string "You broke into TEAM ROCKET's\n"
	.string "operation? What nerve!$"

RocketHideout_B1F_Text_Grunt2Defeat::
	.string "Boom!$"

RocketHideout_B1F_Text_Grunt2PostBattle::
	.string "You're not going to get away with\n"
	.string "this, brat!$"

RocketHideout_B1F_Text_Grunt3Intro::
	.string "Intruder alert!$"

RocketHideout_B1F_Text_Grunt3Defeat::
	.string "I can't do it!$"

RocketHideout_B1F_Text_Grunt3PostBattle::
	.string "SILPH SCOPE? Humph!\n"
	.string "I don't know where it is.$"

RocketHideout_B1F_Text_Grunt4Intro::
	.string "Why did you come here?$"

RocketHideout_B1F_Text_Grunt4Defeat::
	.string "This won't do!$"

RocketHideout_B1F_Text_Grunt4PostBattle::
	.string "Okay, I'll talk...\n"
	.string "Take the elevator to see my BOSS.$"

RocketHideout_B1F_Text_Grunt5Intro::
	.string "Are you lost, you little mouse?$"

RocketHideout_B1F_Text_Grunt5Defeat::
	.string "Why...?$"

RocketHideout_B1F_Text_Grunt5PostBattle::
	.string "Uh-oh, that ruckus somehow opened\n"
	.string "the door!$"

RocketHideout_B2F_Text_GruntIntro::
	.string "BOSS said you can see ghosts with\n"
	.string "the SILPH SCOPE.$"

RocketHideout_B2F_Text_GruntDefeat::
	.string "I surrender!$"

RocketHideout_B2F_Text_GruntPostBattle::
	.string "The TEAM ROCKET HQ has four\n"
	.string "underground floors.\p"
	.string "Think you can reach the BOSS?$"

RocketHideout_B3F_Text_Grunt1Intro::
	.string "Stop meddling in TEAM ROCKET's\n"
	.string "affairs or else!$"

RocketHideout_B3F_Text_Grunt1Defeat::
	.string "Oof!\n"
	.string "Taken down!$"

RocketHideout_B3F_Text_Grunt1PostBattle::
	.string "SILPH SCOPE?\n"
	.string "Oh, that machine the BOSS stole.\p"
	.string "It's here somewhere.$"

RocketHideout_B3F_Text_Grunt2Intro::
	.string "We got word from upstairs that you\n"
	.string "were coming!$"

RocketHideout_B3F_Text_Grunt2Defeat::
	.string "What?\n"
	.string "I lost? No!$"

RocketHideout_B3F_Text_Grunt2PostBattle::
	.string "Go ahead and go!\p"
	.string "But you can't run the elevator\n"
	.string "without the LIFT KEY.$"

RocketHideout_B4F_Text_GiovanniIntro::
	.string "So! I must say, I am impressed you\n"
	.string "got here.\p"
	.string "TEAM ROCKET captures POKéMON from\n"
	.string "around the world.\p"
	.string "They're important tools for keeping\n"
	.string "our criminal enterprise going.\p"
	.string "I am the leader, GIOVANNI!\p"
	.string "For your insolence, you will feel a\n"
	.string "world of pain!$"

RocketHideout_B4F_Text_GiovanniDefeat::
	.string "WHAT!\n"
	.string "This can't be!$"

RocketHideout_B4F_Text_GiovanniPostBattle::
	.string "I see that you raise POKéMON with\n"
	.string "utmost care.\p"
	.string "A child like you would never\n"
	.string "understand what I hope to achieve.\p"
	.string "I shall step aside this time!\p"
	.string "I hope we meet again...$"

RocketHideout_B4F_Text_GiovanniGuarded::
	.string "The guards refuse to let you\n"
	.string "approach their BOSS.$"

RocketHideout_B4F_Text_SilphScopeNotReady::
	.string "The SILPH SCOPE is secured\n"
	.string "elsewhere.$"

RocketHideout_B4F_Text_ReceivedSilphScope::
	.string "{PLAYER} received the\n"
	.string "SILPH SCOPE!$"

RocketHideout_B4F_Text_Grunt1Intro::
	.string "The elevator doesn't work?\n"
	.string "Who has the LIFT KEY?$"

RocketHideout_B4F_Text_Grunt1Defeat::
	.string "No!$"

RocketHideout_B4F_Text_Grunt1PostBattle::
	.string "Oh, no!\n"
	.string "I dropped the LIFT KEY!$"

RocketHideout_B4F_Text_LiftKeyNotReady::
	.string "The LIFT KEY is held by a\n"
	.string "nearby ROCKET member.$"

RocketHideout_B4F_Text_ReceivedLiftKey::
	.string "{PLAYER} received the\n"
	.string "LIFT KEY!$"

RocketHideout_B4F_Text_Grunt2Intro::
	.string "I know you!\n"
	.string "You're meddling in our plans!$"

RocketHideout_B4F_Text_Grunt2Defeat::
	.string "Burned again!$"

RocketHideout_B4F_Text_Grunt2PostBattle::
	.string "Do you have something against TEAM\n"
	.string "ROCKET?$"

RocketHideout_B4F_Text_Grunt3Intro::
	.string "How can you not see the beauty of\n"
	.string "our evil?$"

RocketHideout_B4F_Text_Grunt3Defeat::
	.string "Ayaya!$"

RocketHideout_B4F_Text_Grunt3PostBattle::
	.string "BOSS!\n"
	.string "I'm sorry I failed you!$"

CeladonGameCorner_EventScript_HideoutGrunt_PostDialogue::
	msgbox CeladonGameCorner_Text_HideoutGruntPostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt1_PostDialogue::
	msgbox RocketHideout_B1F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt2_PostDialogue::
	msgbox RocketHideout_B1F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt3_PostDialogue::
	msgbox RocketHideout_B1F_Text_Grunt3PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt4_PostDialogue::
	msgbox RocketHideout_B1F_Text_Grunt4PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B1F_EventScript_Grunt5_PostDialogue::
	msgbox RocketHideout_B1F_Text_Grunt5PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B2F_EventScript_Grunt_PostDialogue::
	msgbox RocketHideout_B2F_Text_GruntPostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B3F_EventScript_Grunt1_PostDialogue::
	msgbox RocketHideout_B3F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B3F_EventScript_Grunt2_PostDialogue::
	msgbox RocketHideout_B3F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B4F_EventScript_Grunt1_PostDialogue::
	msgbox RocketHideout_B4F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B4F_EventScript_Grunt2_PostDialogue::
	msgbox RocketHideout_B4F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

RocketHideout_B4F_EventScript_Grunt3_PostDialogue::
	msgbox RocketHideout_B4F_Text_Grunt3PostBattle, MSGBOX_AUTOCLOSE
	end
