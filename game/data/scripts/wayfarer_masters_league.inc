@ Wayfarer-only HNS room dispatch for the invited Sevii Masters.
@ Each victory is recorded by the circuit runtime before a passage opens.

WayfarerMasters_EventScript_Will::
	lock
	faceplayer
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 0
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerMasters_EventScript_AlreadyWon
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	msgbox WayfarerMasters_Text_Will, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerMasters_EventScript_Withdraw
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_WILL_2_HNS, WayfarerMasters_Text_Win
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 0
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	goto WayfarerMasters_EventScript_OpenDoor

WayfarerMasters_EventScript_Koga::
	lock
	faceplayer
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 1
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerMasters_EventScript_AlreadyWon
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	msgbox WayfarerMasters_Text_Koga, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerMasters_EventScript_Withdraw
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_KOGA_2_HNS, WayfarerMasters_Text_Win
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 1
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	goto WayfarerMasters_EventScript_OpenDoor

WayfarerMasters_EventScript_Bruno::
	lock
	faceplayer
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 2
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerMasters_EventScript_AlreadyWon
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	msgbox WayfarerMasters_Text_Bruno, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerMasters_EventScript_Withdraw
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_BRUNO_2_HNS, WayfarerMasters_Text_Win
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 2
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	goto WayfarerMasters_EventScript_OpenDoor

WayfarerMasters_EventScript_Karen::
	lock
	faceplayer
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 3
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerMasters_EventScript_AlreadyWon
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	msgbox WayfarerMasters_Text_Karen, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerMasters_EventScript_Withdraw
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_KAREN_2_HNS, WayfarerMasters_Text_Win
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 3
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	goto WayfarerMasters_EventScript_OpenDoor

WayfarerMasters_EventScript_OpenDoor::
	setmetatile 6, 1, 0x28E, FALSE
	setmetatile 6, 2, 0x296, FALSE
	playse SE_BANG
	special DrawWholeMapView
	msgbox WayfarerMasters_Text_Next
	release
	end

WayfarerMasters_EventScript_Lance::
	lockall
	faceplayer
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 4
	specialvar VAR_RESULT, LeagueCircuit_IsCurrentRoomDefeated
	goto_if_eq VAR_RESULT, TRUE, WayfarerMasters_EventScript_ChampionResume
	specialvar VAR_RESULT, LeagueCircuit_ValidateRoomBattle
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	msgbox WayfarerMasters_Text_Lance, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerMasters_EventScript_Withdraw
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	trainerbattle_no_intro TRAINER_LANCE_2_HNS, WayfarerMasters_Text_Win
	clearflag FLAG_TEMP_5
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	setvar VAR_0x8005, 4
	specialvar VAR_RESULT, LeagueCircuit_RecordRoomVictory
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	msgbox WayfarerMasters_Text_LanceAfter

WayfarerMasters_EventScript_ChampionResume::
	warpsilent MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS, 0
	waitstate
	releaseall
	end

WayfarerMasters_EventScript_Gallery::
	lockall
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	specialvar VAR_RESULT, LeagueCircuit_CanCompleteRun
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_MastersEntryDenied
	msgbox WayfarerMasters_Text_Gallery
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	specialvar VAR_RESULT, LeagueCircuit_CommitRunClear
	goto_if_eq VAR_RESULT, CIRCUIT_COMMIT_INVALID, LeagueCircuit_EventScript_MastersEntryDenied
	setvar VAR_LEAGUE_STATE, 1
	warp MAP_SEVEN_ISLAND_HOUSE_ROOM1, 4, 5
	waitstate
	releaseall
	end

WayfarerMasters_EventScript_AlreadyWon::
	msgbox WayfarerMasters_Text_Next
	release
	end

WayfarerMasters_EventScript_Withdraw::
	special LeagueCircuit_AbandonRun
	warp MAP_SEVEN_ISLAND_HOUSE_ROOM1, 4, 5
	waitstate
	releaseall
	end

WayfarerMasters_Text_Will::
	.string "I am WILL, an invited MASTER.\p"
	.string "Ready to face my team?$"
WayfarerMasters_Text_Koga::
	.string "KOGA awaits as the next MASTER.\p"
	.string "Will you face my team?$"
WayfarerMasters_Text_Bruno::
	.string "I am BRUNO, invited to this\n"
	.string "challenge. Battle me?$"
WayfarerMasters_Text_Karen::
	.string "I am KAREN. Only skill matters\n"
	.string "here. Battle me?$"
WayfarerMasters_Text_Lance::
	.string "I once held the Champion title.\p"
	.string "I am the final MASTER. Ready?$"
WayfarerMasters_Text_Win::
	.string "You have won this battle.$"
WayfarerMasters_Text_Next::
	.string "The next MASTER awaits.$"
WayfarerMasters_Text_LanceAfter::
	.string "The MASTERS GALLERY is ahead.$"
WayfarerMasters_Text_Gallery::
	.string "Welcome to the MASTERS GALLERY.\p"
	.string "Your challenge is complete.$"
