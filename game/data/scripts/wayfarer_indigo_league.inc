@ The selected FRLG League rooms are a separate Wayfarer stage. Their source
@ event labels collide with the active HNS rooms, so only these entry points
@ and the shared FRLG door helper are linked in the Wayfarer build.

PokemonLeague_LoreleisRoom_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, WayfarerIndigo_LoreleiOnResume
	map_script MAP_SCRIPT_ON_LOAD, WayfarerIndigo_LoreleiOnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, WayfarerIndigo_LoreleiOnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, WayfarerIndigo_LoreleiOnFrame
	.byte 0

WayfarerIndigo_LoreleiOnResume::
	setvar VAR_0x8004, 0
	call PokemonLeague_EventScript_DoLightingEffect
	end

WayfarerIndigo_LoreleiOnLoad::
	call_if_eq VAR_LEAGUE_STATE, 1, PokemonLeague_EventScript_CloseEntry
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	call_if_eq VAR_RESULT, TRUE, PokemonLeague_EventScript_SetDoorOpen
	end

WayfarerIndigo_LoreleiOnWarp::
	map_script_2 VAR_TEMP_1, 0, WayfarerIndigo_TurnPlayerNorth
	.2byte 0

WayfarerIndigo_LoreleiOnFrame::
	map_script_2 VAR_LEAGUE_STATE, 0, WayfarerIndigo_EnterLoreleiRoom
	.2byte 0

WayfarerIndigo_EventScript_Lorelei::
	lock
	faceplayer
	specialvar VAR_RESULT, LeagueCircuit_ValidateRun
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerIndigo_LoreleiDefeated
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 0
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	msgbox WayfarerIndigo_Text_LoreleiIntro
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_WAYFARER_INDIGO_LORELEI, WayfarerIndigo_Text_LoreleiWin
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 0
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	call PokemonLeague_EventScript_OpenDoor
WayfarerIndigo_LoreleiDefeated::
	msgbox WayfarerIndigo_Text_LoreleiAfter
	release
	end

PokemonLeague_BrunosRoom_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, WayfarerIndigo_BrunoOnResume
	map_script MAP_SCRIPT_ON_LOAD, WayfarerIndigo_BrunoOnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, WayfarerIndigo_BrunoOnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, WayfarerIndigo_BrunoOnFrame
	.byte 0

WayfarerIndigo_BrunoOnResume::
	setvar VAR_0x8004, 1
	call PokemonLeague_EventScript_DoLightingEffect
	end

WayfarerIndigo_BrunoOnLoad::
	call_if_eq VAR_LEAGUE_STATE, 2, PokemonLeague_EventScript_CloseEntry
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	call_if_eq VAR_RESULT, TRUE, PokemonLeague_EventScript_SetDoorOpen
	end

WayfarerIndigo_BrunoOnWarp::
	map_script_2 VAR_TEMP_1, 0, WayfarerIndigo_TurnPlayerNorth
	.2byte 0

WayfarerIndigo_BrunoOnFrame::
	map_script_2 VAR_LEAGUE_STATE, 1, WayfarerIndigo_EnterBrunoRoom
	.2byte 0

WayfarerIndigo_EventScript_Bruno::
	lock
	faceplayer
	specialvar VAR_RESULT, LeagueCircuit_ValidateRun
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerIndigo_BrunoDefeated
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 1
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	msgbox WayfarerIndigo_Text_BrunoIntro
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_WAYFARER_INDIGO_BRUNO, WayfarerIndigo_Text_BrunoWin
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 1
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	call PokemonLeague_EventScript_OpenDoor
WayfarerIndigo_BrunoDefeated::
	msgbox WayfarerIndigo_Text_BrunoAfter
	release
	end

PokemonLeague_AgathasRoom_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, WayfarerIndigo_AgathaOnResume
	map_script MAP_SCRIPT_ON_LOAD, WayfarerIndigo_AgathaOnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, WayfarerIndigo_AgathaOnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, WayfarerIndigo_AgathaOnFrame
	.byte 0

WayfarerIndigo_AgathaOnResume::
	setvar VAR_0x8004, 2
	call PokemonLeague_EventScript_DoLightingEffect
	end

WayfarerIndigo_AgathaOnLoad::
	call_if_eq VAR_LEAGUE_STATE, 3, PokemonLeague_EventScript_CloseEntry
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	call_if_eq VAR_RESULT, TRUE, PokemonLeague_EventScript_SetDoorOpen
	end

WayfarerIndigo_AgathaOnWarp::
	map_script_2 VAR_TEMP_1, 0, WayfarerIndigo_TurnPlayerNorth
	.2byte 0

WayfarerIndigo_AgathaOnFrame::
	map_script_2 VAR_LEAGUE_STATE, 2, WayfarerIndigo_EnterAgathaRoom
	.2byte 0

WayfarerIndigo_EventScript_Agatha::
	lock
	faceplayer
	specialvar VAR_RESULT, LeagueCircuit_ValidateRun
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerIndigo_AgathaDefeated
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 2
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	msgbox WayfarerIndigo_Text_AgathaIntro
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_WAYFARER_INDIGO_AGATHA, WayfarerIndigo_Text_AgathaWin
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 2
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	call PokemonLeague_EventScript_OpenDoor
WayfarerIndigo_AgathaDefeated::
	msgbox WayfarerIndigo_Text_AgathaAfter
	release
	end

PokemonLeague_LancesRoom_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, WayfarerIndigo_LanceOnResume
	map_script MAP_SCRIPT_ON_LOAD, WayfarerIndigo_LanceOnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, WayfarerIndigo_LanceOnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, WayfarerIndigo_LanceOnFrame
	.byte 0

WayfarerIndigo_LanceOnResume::
	setvar VAR_0x8004, 3
	call PokemonLeague_EventScript_DoLightingEffect
	end

WayfarerIndigo_LanceOnLoad::
	call_if_eq VAR_LEAGUE_STATE, 4, WayfarerIndigo_LanceSetEntryClosed
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	call_if_eq VAR_RESULT, TRUE, PokemonLeague_EventScript_SetDoorOpenLance
	end

WayfarerIndigo_LanceOnWarp::
	map_script_2 VAR_TEMP_1, 0, WayfarerIndigo_TurnPlayerNorth
	.2byte 0

WayfarerIndigo_LanceOnFrame::
	map_script_2 VAR_LEAGUE_STATE, 3, WayfarerIndigo_LanceEnterRoom
	.2byte 0

WayfarerIndigo_LanceEnterRoom::
	lockall
	applymovement LOCALID_PLAYER, WayfarerIndigo_LanceWalkThroughCorridor
	waitmovement 0
	setflag FLAG_TEMP_2
	call WayfarerIndigo_LanceSetEntryClosed
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	setvar VAR_LEAGUE_STATE, 4
	releaseall
	end

WayfarerIndigo_LanceSetEntryClosed::
	setmetatile 5, 14, METATILE_PokemonLeague_Entry_TopLeft_Closed, 1
	setmetatile 6, 14, METATILE_PokemonLeague_Entry_TopMid_Closed, 1
	setmetatile 7, 14, METATILE_PokemonLeague_Entry_TopRight_Closed, 1
	setmetatile 5, 15, METATILE_PokemonLeague_Entry_BottomLeft_Closed, 1
	setmetatile 6, 15, METATILE_PokemonLeague_Entry_BottomMid_Closed, 1
	setmetatile 7, 15, METATILE_PokemonLeague_Entry_BottomRight_Closed, 1
	setmetatile 6, 16, METATILE_PokemonLeague_Floor_ShadeFull_Lance, 0
	setmetatile 7, 16, METATILE_PokemonLeague_Floor_ShadeFull_Lance, 0
	return

WayfarerIndigo_LanceWalkThroughCorridor::
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

WayfarerIndigo_EventScript_Lance::
	lock
	faceplayer
	specialvar VAR_RESULT, LeagueCircuit_ValidateRun
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerIndigo_LanceDefeated
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 3
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	msgbox WayfarerIndigo_Text_LanceIntro
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_WAYFARER_INDIGO_LANCE, WayfarerIndigo_Text_LanceWin
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 3
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	call PokemonLeague_EventScript_OpenDoorLance
WayfarerIndigo_LanceDefeated::
	msgbox WayfarerIndigo_Text_LanceAfter
	release
	end

PokemonLeague_ChampionsRoom_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, WayfarerIndigo_ChampionOnResume
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, WayfarerIndigo_ChampionOnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, WayfarerIndigo_ChampionOnFrame
	.byte 0

WayfarerIndigo_ChampionOnResume::
	setvar VAR_0x8004, 4
	call PokemonLeague_EventScript_DoLightingEffect
	end

WayfarerIndigo_ChampionOnWarp::
	map_script_2 VAR_TEMP_1, 0, WayfarerIndigo_TurnPlayerNorth
	.2byte 0

WayfarerIndigo_ChampionOnFrame::
	map_script_2 VAR_TEMP_1, 0, WayfarerIndigo_ChampionBattle
	.2byte 0

WayfarerIndigo_ChampionBattle::
	lockall
	setflag FLAG_TEMP_2
	applymovement LOCALID_PLAYER, WayfarerIndigo_ChampionEnterMovement
	waitmovement 0
	delay 20
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerIndigo_ChampionResumeCompleted
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 4
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	msgbox WayfarerIndigo_Text_BlueIntro
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_WAYFARER_INDIGO_BLUE, WayfarerIndigo_Text_BlueWin
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	setvar VAR_0x8005, 4
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	setvar VAR_TEMP_1, 1
	msgbox WayfarerIndigo_Text_BlueAfter

WayfarerIndigo_ChampionResumeCompleted::
	warp MAP_POKEMON_LEAGUE_HALL_OF_FAME, 5, 12
	waitstate
	releaseall
	end

WayfarerIndigo_ChampionEnterMovement::
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

WayfarerIndigo_TurnPlayerNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

WayfarerIndigo_EnterLoreleiRoom::
	lockall
	call PokemonLeague_EventScript_EnterRoom
	setvar VAR_LEAGUE_STATE, 1
	releaseall
	end

WayfarerIndigo_EnterBrunoRoom::
	lockall
	call PokemonLeague_EventScript_EnterRoom
	setvar VAR_LEAGUE_STATE, 2
	releaseall
	end

WayfarerIndigo_EnterAgathaRoom::
	lockall
	call PokemonLeague_EventScript_EnterRoom
	setvar VAR_LEAGUE_STATE, 3
	releaseall
	end

PokemonLeague_HallOfFame_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, WayfarerIndigo_HallOnFrame
	.byte 0

WayfarerIndigo_HallOnFrame::
	map_script_2 VAR_TEMP_1, 0, WayfarerIndigo_HallCeremony
	.2byte 0

WayfarerIndigo_HallCeremony::
	lockall
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	specialvar VAR_RESULT, LeagueCircuit_CanCompleteRun
	goto_if_eq VAR_RESULT, FALSE, WayfarerIndigo_InvalidRun
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	specialvar VAR_RESULT, LeagueCircuit_HasClearedStage
	goto_if_eq VAR_RESULT, TRUE, WayfarerIndigo_HallReplay
	msgbox WayfarerIndigo_Text_HallFirstClear
	dofieldeffect FLDEFF_HALL_OF_FAME_RECORD_FRLG
	waitfieldeffect FLDEFF_HALL_OF_FAME_RECORD_FRLG
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	special LeagueCircuit_CommitAndRegisterIndigo
	waitstate
	releaseall
	end

WayfarerIndigo_HallReplay::
	msgbox WayfarerIndigo_Text_HallReplay

WayfarerIndigo_HallCommit::
	setvar VAR_0x8004, CIRCUIT_STAGE_INDIGO
	specialvar VAR_RESULT, LeagueCircuit_CommitRunClear
	goto_if_eq VAR_RESULT, CIRCUIT_COMMIT_INVALID, WayfarerIndigo_InvalidRun
	setvar VAR_TEMP_1, 1
	warp MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS, 32, 3
	waitstate
	releaseall
	end

WayfarerIndigo_InvalidRun::
	special LeagueCircuit_AbandonRun
	clearflag FLAG_TEMP_5
	warp MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS, 32, 3
	waitstate
	releaseall
	end

WayfarerIndigo_Text_LoreleiIntro::
	.string "I am LORELEI of the ELITE FOUR.\p"
	.string "Show me what your POKéMON can do!$"
WayfarerIndigo_Text_LoreleiWin::
	.string "Your team has earned this win.$"
WayfarerIndigo_Text_LoreleiAfter::
	.string "The next room awaits you.$"
WayfarerIndigo_Text_BrunoIntro::
	.string "I am BRUNO of the ELITE FOUR.\p"
	.string "Let us test our strength!$"
WayfarerIndigo_Text_BrunoWin::
	.string "Your strength is undeniable.$"
WayfarerIndigo_Text_BrunoAfter::
	.string "Go on to your next challenge.$"
WayfarerIndigo_Text_AgathaIntro::
	.string "I am AGATHA of the ELITE FOUR.\p"
	.string "Let's see how you handle my team!$"
WayfarerIndigo_Text_AgathaWin::
	.string "Well fought, child.$"
WayfarerIndigo_Text_AgathaAfter::
	.string "LANCE is waiting ahead.$"
WayfarerIndigo_Text_LanceIntro::
	.string "I am LANCE of the ELITE FOUR.\p"
	.string "Can you defeat my dragons?$"
WayfarerIndigo_Text_LanceWin::
	.string "You have surpassed me.$"
WayfarerIndigo_Text_LanceAfter::
	.string "BLUE awaits in the final room.$"
WayfarerIndigo_Text_BlueIntro::
	.string "I'm BLUE, the INDIGO CHAMPION.\p"
	.string "Show me the team that got you here!$"
WayfarerIndigo_Text_BlueWin::
	.string "You won. You really did it.$"
WayfarerIndigo_Text_BlueAfter::
	.string "You have earned your place in the\n"
	.string "HALL OF FAME.$"
WayfarerIndigo_Text_HallFirstClear::
	.string "Your first INDIGO victory will be\n"
	.string "recorded in the HALL OF FAME.$"
WayfarerIndigo_Text_HallReplay::
	.string "Welcome back, INDIGO CHAMPION!$"
