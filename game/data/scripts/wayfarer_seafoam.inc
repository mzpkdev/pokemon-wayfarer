@ Wayfarer-local Seafoam selection. The FRLG source scripts remain linked only
@ by standalone FRLG; these labels satisfy the selected Wayfarer map events.

#if IS_WAYFARER

.equ WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, VAR_TEMP_2

SeafoamIslands_1F_Frlg_MapScripts::
	.byte 0

SeafoamIslands_B1F_Frlg_MapScripts::
	.byte 0

SeafoamIslands_B2F_Frlg_MapScripts::
	.byte 0

SeafoamIslands_B3F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SeafoamIslands_B3F_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, SeafoamIslands_B3F_OnFrame
	.byte 0

SeafoamIslands_B3F_OnTransition::
	call_if_unset FLAG_WAYFARER_STOPPED_SEAFOAM_B3F_CURRENT, SeafoamIslands_B3F_EventScript_CheckStoppedCurrent
	call_if_set FLAG_WAYFARER_STOPPED_SEAFOAM_B3F_CURRENT, SeafoamIslands_B3F_EventScript_SetNoCurrentLayout
	end

SeafoamIslands_B3F_EventScript_CheckStoppedCurrent::
	setvar WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 0
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_1, SeafoamIslands_B3F_EventScript_AddBoulderPresent
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_2, SeafoamIslands_B3F_EventScript_AddBoulderPresent
	call_if_eq WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 2, SeafoamIslands_B3F_EventScript_StoppedCurrent
	return

SeafoamIslands_B3F_EventScript_StoppedCurrent::
	setflag FLAG_WAYFARER_STOPPED_SEAFOAM_B3F_CURRENT
	return

SeafoamIslands_B3F_EventScript_SetNoCurrentLayout::
	setmaplayoutindex LAYOUT_SEAFOAM_ISLANDS_B3F_CURRENT_STOPPED
	return

SeafoamIslands_B3F_OnFrame::
	map_script_2 VAR_TEMP_1, 1, SeafoamIslands_B3F_EventScript_EnterByFalling
	.2byte 0

SeafoamIslands_B3F_EventScript_EnterByFalling::
	lockall
	setvar WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 0
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_1, SeafoamIslands_B3F_EventScript_AddBoulderPresent
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_2, SeafoamIslands_B3F_EventScript_AddBoulderPresent
	goto_if_eq WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 2, SeafoamIslands_B3F_EventScript_CurrentBlocked
	getplayerxy VAR_0x8008, VAR_0x8009
	call_if_lt VAR_0x8008, 24, SeafoamIslands_B3F_EventScript_RideCurrentFar
	call_if_ge VAR_0x8008, 24, SeafoamIslands_B3F_EventScript_RideCurrentClose
	setvar VAR_WAYFARER_SEAFOAM_B4F_CURRENT_ENTRY, 1
	warp MAP_SEAFOAM_ISLANDS_B4F, 27, 21
	waitstate
	releaseall
	end

SeafoamIslands_B3F_EventScript_AddBoulderPresent::
	addvar WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 1
	return

SeafoamIslands_B3F_EventScript_RideCurrentFar::
	applymovement LOCALID_PLAYER, SeafoamIslands_B3F_Movement_RideCurrentFar
	waitmovement 0
	return

SeafoamIslands_B3F_EventScript_RideCurrentClose::
	applymovement LOCALID_PLAYER, SeafoamIslands_B3F_Movement_RideCurrentClose
	waitmovement 0
	return

SeafoamIslands_B3F_EventScript_CurrentBlocked::
	setvar VAR_TEMP_1, 0
	releaseall
	end

SeafoamIslands_B3F_Movement_RideCurrentFar::
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

SeafoamIslands_B3F_Movement_RideCurrentClose::
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

SeafoamIslands_B4F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SeafoamIslands_B4F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SeafoamIslands_B4F_OnResume
	map_script MAP_SCRIPT_ON_LOAD, SeafoamIslands_B4F_OnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, SeafoamIslands_B4F_OnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, SeafoamIslands_B4F_OnFrame
	.byte 0

SeafoamIslands_B4F_OnResume::
	call_if_set FLAG_SYS_CTRL_OBJ_DELETE, SeafoamIslands_B4F_EventScript_TryRemoveArticuno
	end

SeafoamIslands_B4F_EventScript_TryRemoveArticuno::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Common_EventScript_NopReturn
	removeobject VAR_LAST_TALKED
	return

SeafoamIslands_B4F_OnTransition::
	call_if_unset FLAG_WAYFARER_FOUGHT_ARTICUNO, SeafoamIslands_B4F_EventScript_ShowArticuno
	call_if_unset FLAG_WAYFARER_STOPPED_SEAFOAM_B4F_CURRENT, SeafoamIslands_B4F_EventScript_CheckStoppedCurrent
	call_if_set FLAG_WAYFARER_STOPPED_SEAFOAM_B4F_CURRENT, SeafoamIslands_B4F_EventScript_SetNoCurrentLayout
	end

SeafoamIslands_B4F_EventScript_CheckStoppedCurrent::
	setvar WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 0
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B4F_BOULDER_1, SeafoamIslands_B4F_EventScript_AddBoulderPresent
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B4F_BOULDER_2, SeafoamIslands_B4F_EventScript_AddBoulderPresent
	call_if_eq WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 2, SeafoamIslands_B4F_EventScript_StoppedCurrent
	return

SeafoamIslands_B4F_EventScript_StoppedCurrent::
	setflag FLAG_WAYFARER_STOPPED_SEAFOAM_B4F_CURRENT
	return

SeafoamIslands_B4F_EventScript_SetNoCurrentLayout::
	setmaplayoutindex LAYOUT_SEAFOAM_ISLANDS_B4F_CURRENT_STOPPED
	return

SeafoamIslands_B4F_EventScript_ShowArticuno::
	clearflag FLAG_WAYFARER_HIDE_ARTICUNO
	return

SeafoamIslands_B4F_OnLoad::
	setvar WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 0
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B4F_BOULDER_1, SeafoamIslands_B4F_EventScript_AddBoulderPresent
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B4F_BOULDER_2, SeafoamIslands_B4F_EventScript_AddBoulderPresent
	goto_if_eq WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 2, SeafoamIslands_B4F_EventScript_SetCalmWaterNearStairs
	end

SeafoamIslands_B4F_EventScript_SetCalmWaterNearStairs::
	setmetatile 12, 14, METATILE_GeneralFrlg_CalmWater, 0
	setmetatile 13, 14, METATILE_GeneralFrlg_CalmWater, 0
	end

SeafoamIslands_B4F_OnWarp::
	map_script_2 VAR_WAYFARER_SEAFOAM_B4F_CURRENT_ENTRY, 1, SeafoamIslands_B4F_EventScript_WarpInOnCurrent
	.2byte 0

SeafoamIslands_B4F_EventScript_WarpInOnCurrent::
	turnobject LOCALID_PLAYER, DIR_NORTH
	special ForcePlayerToStartSurfing
	end

SeafoamIslands_B4F_OnFrame::
	map_script_2 VAR_WAYFARER_SEAFOAM_B4F_CURRENT_ENTRY, 1, SeafoamIslands_B4F_EventScript_EnterOnCurrent
	map_script_2 VAR_TEMP_1, 1, SeafoamIslands_B4F_EventScript_EnterByFalling
	.2byte 0

SeafoamIslands_B4F_EventScript_EnterOnCurrent::
	lockall
	applymovement LOCALID_PLAYER, SeafoamIslands_B4F_Movement_EnterOnCurrent
	waitmovement 0
	setvar VAR_WAYFARER_SEAFOAM_B4F_CURRENT_ENTRY, 0
	releaseall
	end

SeafoamIslands_B4F_Movement_EnterOnCurrent::
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

SeafoamIslands_B4F_EventScript_EnterByFalling::
	lockall
	setvar WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 0
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B4F_BOULDER_1, SeafoamIslands_B4F_EventScript_AddBoulderPresent
	call_if_unset FLAG_WAYFARER_HIDE_SEAFOAM_B4F_BOULDER_2, SeafoamIslands_B4F_EventScript_AddBoulderPresent
	goto_if_eq WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 2, SeafoamIslands_B4F_EventScript_CurrentBlocked
	getplayerxy VAR_0x8008, VAR_0x8009
	call_if_lt VAR_0x8008, 9, SeafoamIslands_B4F_EventScript_RideCurrentFar
	call_if_ge VAR_0x8008, 9, SeafoamIslands_B4F_EventScript_RideCurrentClose
	special SeafoamIslandsB4F_CurrentDumpsPlayerOnLand
	setvar VAR_TEMP_1, 0
	releaseall
	end

SeafoamIslands_B4F_EventScript_AddBoulderPresent::
	addvar WAYFARER_SEAFOAM_NUM_BOULDERS_PRESENT, 1
	return

SeafoamIslands_B4F_EventScript_RideCurrentFar::
	applymovement LOCALID_PLAYER, SeafoamIslands_B4F_Movement_RideCurrentFar
	waitmovement 0
	return

SeafoamIslands_B4F_EventScript_RideCurrentClose::
	applymovement LOCALID_PLAYER, SeafoamIslands_B4F_Movement_RideCurrentClose
	waitmovement 0
	return

SeafoamIslands_B4F_EventScript_CurrentBlocked::
	setvar VAR_TEMP_1, 0
	releaseall
	end

SeafoamIslands_B4F_Movement_RideCurrentFar::
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	step_end

SeafoamIslands_B4F_Movement_RideCurrentClose::
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_up
	step_end

SeafoamIslands_B4F_EventScript_UpwardCurrent::
	lockall
	applymovement LOCALID_PLAYER, SeafoamIslands_B4F_Movement_WalkUp
	waitmovement 0
	releaseall
	end

SeafoamIslands_B4F_Movement_WalkUp::
	walk_up
	step_end

SeafoamIslands_B4F_EventScript_Articuno::
	lock
	faceplayer
	specialvar VAR_RESULT, WayfarerCoast_IsArticunoEligible
	goto_if_eq VAR_RESULT, FALSE, SeafoamIslands_B4F_EventScript_ArticunoNotReady
	setwildbattle SPECIES_ARTICUNO, 50
	waitse
	playmoncry SPECIES_ARTICUNO, CRY_MODE_ENCOUNTER
	message Text_Gyaoo
	waitmessage
	waitmoncry
	delay 10
	playbgm MUS_RG_ENCOUNTER_GYM_LEADER, 0
	waitbuttonpress
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, SeafoamIslands_B4F_EventScript_RanFromArticuno
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, SeafoamIslands_B4F_EventScript_RanFromArticuno
	setflag FLAG_WAYFARER_FOUGHT_ARTICUNO
	setflag FLAG_WAYFARER_HIDE_ARTICUNO
	goto Common_EventScript_RemoveStaticPokemon

SeafoamIslands_B4F_EventScript_RanFromArticuno::
	setvar VAR_0x8004, SPECIES_ARTICUNO
	goto Common_EventScript_LegendaryFlewAway

SeafoamIslands_B4F_EventScript_ArticunoNotReady::
	msgbox SeafoamIslands_B4F_Text_ArticunoNotReady
	release
	end

SeafoamIslands_B4F_EventScript_BoulderHintSign::
	msgbox SeafoamIslands_B4F_Text_BouldersMightChangeWaterFlow, MSGBOX_SIGN
	end

SeafoamIslands_B4F_EventScript_FastCurrentSign::
	msgbox SeafoamIslands_B4F_Text_DangerFastCurrent, MSGBOX_SIGN
	end

SeafoamIslands_1F_EventScript_ItemIceHeal::
	finditem ITEM_ICE_HEAL
	end

SeafoamIslands_B1F_EventScript_ItemWaterStone::
	finditem ITEM_WATER_STONE
	end

SeafoamIslands_B1F_EventScript_ItemRevive::
	finditem ITEM_REVIVE
	end

SeafoamIslands_B2F_EventScript_ItemBigPearl::
	finditem ITEM_BIG_PEARL
	end

SeafoamIslands_B4F_EventScript_ItemUltraBall::
	finditem ITEM_ULTRA_BALL
	end

SeafoamIslands_B4F_Text_BouldersMightChangeWaterFlow::
	.string "Hint: Boulders might change the\n"
	.string "flow of water.$"

SeafoamIslands_B4F_Text_DangerFastCurrent::
	.string "DANGER\n"
	.string "Fast current!$"

SeafoamIslands_B4F_Text_ArticunoNotReady::
	.string "ARTICUNO is watching you warily.\p"
	.string "You need more experience before\n"
	.string "you can challenge it.$"

#endif
