@ Source-free environmental mechanics for the Wayfarer Sevii overlays.
@
@ These helpers deliberately use only common field mechanics and the one
@ Wayfarer-local Tanoby completion flag. They do not reference FRLG scene,
@ quest, Champion, National Dex, Rocket, Ruby, Sapphire, or Trainer state.

@ Icefall Cave: the field-task callback owns the thin-ice transition; the
@ map table installs it and handles the resulting fall exactly once.
FourIsland_IcefallCave_1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, WayfarerSevii_IcefallCave1F_OnResume
	map_script MAP_SCRIPT_ON_LOAD, WayfarerSevii_IcefallCave1F_OnLoad
	map_script MAP_SCRIPT_ON_FRAME_TABLE, WayfarerSevii_IcefallCave1F_OnFrame
	.byte 0

WayfarerSevii_IcefallCave1F_OnResume::
	setstepcallback STEP_CB_ICEFALL_CAVE
	end

WayfarerSevii_IcefallCave1F_OnLoad::
	setvar VAR_WAYFARER_SEVII_ICEFALL_FALL, 0
	special SetIcefallCaveCrackedIceMetatiles
	end

WayfarerSevii_IcefallCave1F_OnFrame::
	map_script_2 VAR_WAYFARER_SEVII_ICEFALL_FALL, 1, WayfarerSevii_IcefallCave1F_FallDownHole
	.2byte 0

WayfarerSevii_IcefallCave1F_FallDownHole::
	lockall
	delay 20
	applymovement LOCALID_PLAYER, WayfarerSevii_Movement_SetInvisible
	waitmovement 0
	playse SE_FALL
	delay 60
	warphole MAP_FOUR_ISLAND_ICEFALL_CAVE_B1F
	waitstate
	releaseall
	end

WayfarerSevii_Movement_SetInvisible::
	set_invisible
	step_end

@ Dotted Hole's floor route is represented by its retained static warps. The
@ four braille markers are the sole retained background-event scripts.
WayfarerSevii_DottedHole_BrailleUp::
	lockall
	braillemsgbox Braille_Text_Up
	releaseall
	end

WayfarerSevii_DottedHole_BrailleLeft::
	lockall
	braillemsgbox Braille_Text_Left
	releaseall
	end

WayfarerSevii_DottedHole_BrailleRight::
	lockall
	braillemsgbox Braille_Text_Right
	releaseall
	end

WayfarerSevii_DottedHole_BrailleDown::
	lockall
	braillemsgbox Braille_Text_Down
	releaseall
	end

@ The retained Cut interaction opens the Ruin Valley doorway. The field-move
@ implementation owns setting the Wayfarer-local flag; this load hook restores
@ its collision state after a save, reload, or map transition.
SixIsland_RuinValley_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, WayfarerSevii_RuinValley_OnLoad
	.byte 0

WayfarerSevii_RuinValley_OnLoad::
	call_if_set FLAG_WAYFARER_SEVII_DOTTED_HOLE_OPEN, WayfarerSevii_RuinValley_OpenDottedHoleDoor
	end

WayfarerSevii_RuinValley_OpenDottedHoleDoor::
	setmetatile 24, 24, METATILE_SeviiIslands67_DottedHoleDoor_Open, 0
	return

@ Rocket Warehouse: the Meadow load hook opens the exterior entrance without
@ passwords. The interior hook keeps the source floor-arrow geometry usable
@ for re-entry and exit, with no people or scene state.
FiveIsland_Meadow_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, WayfarerSevii_Meadow_OnLoad
	.byte 0

WayfarerSevii_Meadow_OnLoad::
	setmetatile 12, 21, METATILE_SeviiIslands45_RocketWarehouseDoor_Unlocked, 1
	end

FiveIsland_RocketWarehouse_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, WayfarerSevii_RocketWarehouse_OnLoad
	.byte 0

WayfarerSevii_RocketWarehouse_OnLoad::
	setmetatile 26, 20, METATILE_SilphCo_Arrow_Up, 0
	setmetatile 27, 20, METATILE_SilphCo_Arrow_Up, 0
	end

@ Tanoby Key: boulder placement is re-established from a Wayfarer-local
@ completion flag after loading, while the seven push destinations retain
@ their ordinary Strength-puzzle behavior. Temporary variables record the
@ current visit only; completion is the sole persistent state.
.equ WAYFARER_SEVII_TANOBY_SWITCH_COUNT, VAR_TEMP_8

SevenIsland_SevaultCanyon_TanobyKey_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, WayfarerSevii_TanobyKey_OnTransition
	.byte 0

WayfarerSevii_TanobyKey_OnTransition::
	call_if_set FLAG_WAYFARER_SEVII_TANOBY_COMPLETE, WayfarerSevii_TanobyKey_MoveBouldersToSolvedPos
	setvar VAR_TEMP_1, 0
	setvar VAR_TEMP_2, 0
	setvar VAR_TEMP_3, 0
	setvar VAR_TEMP_4, 0
	setvar VAR_TEMP_5, 0
	setvar VAR_TEMP_6, 0
	setvar VAR_TEMP_7, 0
	setvar WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 0
	end

WayfarerSevii_TanobyKey_MoveBouldersToSolvedPos::
	setobjectxyperm LOCALID_TANOBY_KEY_BOULDER1, 7, 2
	setobjectxyperm LOCALID_TANOBY_KEY_BOULDER2, 4, 4
	setobjectxyperm LOCALID_TANOBY_KEY_BOULDER3, 10, 4
	setobjectxyperm LOCALID_TANOBY_KEY_BOULDER4, 5, 6
	setobjectxyperm LOCALID_TANOBY_KEY_BOULDER5, 9, 6
	setobjectxyperm LOCALID_TANOBY_KEY_BOULDER6, 5, 8
	setobjectxyperm LOCALID_TANOBY_KEY_BOULDER7, 9, 8
	return

WayfarerSevii_TanobyKey_Switch1::
	lockall
	goto_if_eq VAR_TEMP_1, 100, WayfarerSevii_TanobyKey_AlreadyPressed
	call WayfarerSevii_TanobyKey_SwitchPressed
	setvar VAR_TEMP_1, 100
	goto_if_eq WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 7, WayfarerSevii_TanobyKey_PuzzleSolved
	releaseall
	end

WayfarerSevii_TanobyKey_Switch2::
	lockall
	goto_if_eq VAR_TEMP_2, 100, WayfarerSevii_TanobyKey_AlreadyPressed
	call WayfarerSevii_TanobyKey_SwitchPressed
	setvar VAR_TEMP_2, 100
	goto_if_eq WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 7, WayfarerSevii_TanobyKey_PuzzleSolved
	releaseall
	end

WayfarerSevii_TanobyKey_Switch3::
	lockall
	goto_if_eq VAR_TEMP_3, 100, WayfarerSevii_TanobyKey_AlreadyPressed
	call WayfarerSevii_TanobyKey_SwitchPressed
	setvar VAR_TEMP_3, 100
	goto_if_eq WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 7, WayfarerSevii_TanobyKey_PuzzleSolved
	releaseall
	end

WayfarerSevii_TanobyKey_Switch4::
	lockall
	goto_if_eq VAR_TEMP_4, 100, WayfarerSevii_TanobyKey_AlreadyPressed
	call WayfarerSevii_TanobyKey_SwitchPressed
	setvar VAR_TEMP_4, 100
	goto_if_eq WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 7, WayfarerSevii_TanobyKey_PuzzleSolved
	releaseall
	end

WayfarerSevii_TanobyKey_Switch5::
	lockall
	goto_if_eq VAR_TEMP_5, 100, WayfarerSevii_TanobyKey_AlreadyPressed
	call WayfarerSevii_TanobyKey_SwitchPressed
	setvar VAR_TEMP_5, 100
	goto_if_eq WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 7, WayfarerSevii_TanobyKey_PuzzleSolved
	releaseall
	end

WayfarerSevii_TanobyKey_Switch6::
	lockall
	goto_if_eq VAR_TEMP_6, 100, WayfarerSevii_TanobyKey_AlreadyPressed
	call WayfarerSevii_TanobyKey_SwitchPressed
	setvar VAR_TEMP_6, 100
	goto_if_eq WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 7, WayfarerSevii_TanobyKey_PuzzleSolved
	releaseall
	end

WayfarerSevii_TanobyKey_Switch7::
	lockall
	goto_if_eq VAR_TEMP_7, 100, WayfarerSevii_TanobyKey_AlreadyPressed
	call WayfarerSevii_TanobyKey_SwitchPressed
	setvar VAR_TEMP_7, 100
	goto_if_eq WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 7, WayfarerSevii_TanobyKey_PuzzleSolved
	releaseall
	end

WayfarerSevii_TanobyKey_SwitchPressed::
	addvar WAYFARER_SEVII_TANOBY_SWITCH_COUNT, 1
	playse SE_TRUCK_MOVE
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 3
	special ShakeScreen
	delay 60
	return

WayfarerSevii_TanobyKey_AlreadyPressed::
	releaseall
	end

WayfarerSevii_TanobyKey_PuzzleSolved::
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 10
	setvar VAR_0x8007, 3
	special ShakeScreen
	delay 60
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 3
	setvar VAR_0x8006, 12
	setvar VAR_0x8007, 2
	special ShakeScreen
	delay 60
	playse SE_LAVARIDGE_FALL_WARP
	waitse
	setflag FLAG_WAYFARER_SEVII_TANOBY_COMPLETE
	releaseall
	end
