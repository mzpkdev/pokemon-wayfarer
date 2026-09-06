#if IS_WAYFARER
LeagueCircuit_EventScript_ShowStatus::
	special BufferLeagueCircuitStatus
	message gStringVar4
	waitmessage
	waitbuttonpress
	closemessage
	return

LeagueCircuit_EventScript_Itinerary::
	msgbox LeagueCircuit_Text_Itinerary, MSGBOX_DEFAULT
	call LeagueCircuit_EventScript_ShowStatus
	return

LeagueCircuit_EventScript_PostponeGym::
	lockall
	msgbox LeagueCircuit_Text_Postpone, MSGBOX_DEFAULT
	call LeagueCircuit_EventScript_ShowStatus
	releaseall
	end

LeagueCircuit_EventScript_CheckQualification::
	callnative GetTrainerRating
	specialvar VAR_0x8004, LeagueCircuit_GetRequiredRegion
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	call_if_eq VAR_RESULT, TRUE, LeagueCircuit_EventScript_ShowStatus
	return

LeagueCircuit_EventScript_EntryDenied::
	lockall
	call LeagueCircuit_EventScript_ShowStatus
	releaseall
	end

LeagueCircuit_Text_Postpone:
	.string "Your official challenge must wait.\n"
	.string "Clear your next LEAGUE first.$"

LeagueCircuit_Text_Itinerary:
	.string "Badges from all three regions count!\p"
	.string "KANTO LEAGUE: 8 badges.\n"
	.string "JOHTO LEAGUE: 16 badges.\l"
	.string "HOENN LEAGUE: 24 badges.\p"
	.string "Clear each LEAGUE in that order\n"
	.string "to continue earning badges.$"
LeagueCircuit_EventScript_TravelAllowed::
	end

LeagueCircuit_EventScript_IndigoEntryDenied::
	lockall
	call LeagueCircuit_EventScript_ShowStatus
	warp MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS, 0
	waitstate
	releaseall
	end

LeagueCircuit_EventScript_HoennEntryDenied::
	lockall
	call LeagueCircuit_EventScript_ShowStatus
	warp MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F, 0
	waitstate
	releaseall
	end

LeagueCircuit_EventScript_IndigoHallOfFame::
	lockall
	specialvar VAR_0x8004, LeagueCircuit_GetRequiredRegion
	goto_if_eq VAR_0x8004, REGION_HOENN, LeagueCircuit_EventScript_IndigoEntryDenied
	goto_if_eq VAR_0x8004, REGION_NONE, LeagueCircuit_EventScript_IndigoEntryDenied
	specialvar VAR_RESULT, LeagueCircuit_RecordClear
	goto_if_eq VAR_RESULT, FALSE, LeagueCircuit_EventScript_IndigoEntryDenied
	special GivePartyMonChampionRibbon
	setvar VAR_LEAGUE_STATE, 1
	call LeagueCircuit_EventScript_ShowStatus
	setrespawn HEAL_LOCATION_INDIGO_PLATEAU_HNS
	fadescreenspeed FADE_TO_BLACK, 24
	special GameClear
	waitstate
	releaseall
	end
#endif
