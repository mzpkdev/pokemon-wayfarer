#if IS_WAYFARER
LeagueCircuit_EventScript_EntryDenied::
	lockall
	special LeagueCircuit_BufferAdmissionDenial
	msgbox gStringVar4, MSGBOX_DEFAULT
	releaseall
	end

LeagueCircuit_EventScript_TravelAllowed::
	end

LeagueCircuit_EventScript_IndigoEntryDenied::
	lockall
	special LeagueCircuit_BufferAdmissionDenial
	msgbox gStringVar4, MSGBOX_DEFAULT
	warp MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS, 0
	waitstate
	releaseall
	end

LeagueCircuit_EventScript_HoennEntryDenied::
	lockall
	special LeagueCircuit_BufferAdmissionDenial
	msgbox gStringVar4, MSGBOX_DEFAULT
	warp MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F, 0
	waitstate
	releaseall
	end

LeagueCircuit_EventScript_IndigoHallOfFame::
	specialvar VAR_0x8004, LeagueCircuit_GetRequiredRegion
	goto_if_eq VAR_0x8004, REGION_HOENN, LeagueCircuit_EventScript_IndigoEntryDenied
	goto_if_eq VAR_0x8004, REGION_NONE, LeagueCircuit_EventScript_IndigoEntryDenied
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_IndigoEntryDenied
	specialvar VAR_RESULT, LeagueCircuit_RecordClear
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_IndigoEntryDenied
	goto_if_eq VAR_0x8004, REGION_KANTO, LeagueCircuit_EventScript_IndigoHallOfFameKantoCleanup
	call PokemonLeague_HallOfFame_EventScript_SetFirstGameClearFlags
	goto LeagueCircuit_EventScript_IndigoHallOfFameFinish

LeagueCircuit_EventScript_IndigoHallOfFameKantoCleanup:
	call PokemonLeague_HallOfFame_EventScript_SetGameClearFlags

LeagueCircuit_EventScript_IndigoHallOfFameFinish:
	setvar VAR_LEAGUE_STATE, 1
	setrespawn HEAL_LOCATION_INDIGO_PLATEAU_HNS
	fadescreenspeed FADE_TO_BLACK, 24
	special GameClear
	waitstate
	releaseall
	end

LeagueCircuit_Text_NeedEightBadges::
	.string "You need at least eight BADGES\n"
	.string "to enter the KANTO LEAGUE.$"

LeagueCircuit_Text_NeedKantoClear::
	.string "You must clear the KANTO LEAGUE\n"
	.string "before taking this challenge.$"

LeagueCircuit_Text_NeedSixteenBadges::
	.string "You need at least sixteen BADGES\n"
	.string "to enter the JOHTO LEAGUE.$"

LeagueCircuit_Text_NeedJohtoClear::
	.string "You must clear the JOHTO LEAGUE\n"
	.string "before taking this challenge.$"

LeagueCircuit_Text_NeedTwentyFourBadges::
	.string "You need all twenty-four BADGES\n"
	.string "to enter the HOENN LEAGUE.$"

LeagueCircuit_Text_Unavailable::
	.string "There is no LEAGUE challenge\n"
	.string "available here.$"
#endif
