#if IS_WAYFARER
#if WAYFARER_LEAGUE_EVENTS
LeagueEvent_EventScript_Entrance::
	lockall
	specialvar VAR_RESULT, LeagueEvent_GetAcceptedLeague
	goto_if_eq VAR_RESULT, LEAGUE_ID_NONE, LeagueEvent_EventScript_EntranceNew
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	goto_if_eq VAR_RESULT, FALSE, LeagueEvent_EventScript_EntranceDenied
	goto LeagueEvent_EventScript_EntranceAccepted

LeagueEvent_EventScript_EntranceNew::
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	goto_if_eq VAR_RESULT, FALSE, LeagueEvent_EventScript_EntranceDenied
	msgbox LeagueEvent_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LeagueEvent_EventScript_EntranceDeclined
	specialvar VAR_RESULT, LeagueEvent_Accept
	goto_if_ne VAR_RESULT, LEAGUE_ACCEPT_OK, LeagueEvent_EventScript_EntranceDenied
	specialvar VAR_RESULT, LeagueEvent_BufferLineup
	goto_if_eq VAR_RESULT, FALSE, LeagueEvent_EventScript_EntranceDenied
	msgbox gStringVar4
	msgbox LeagueEvent_Text_Accepted
	setvar VAR_TEMP_F, 1
	releaseall
	end

LeagueEvent_EventScript_EntranceAccepted::
	special LeagueEvent_BufferChampion
	msgbox LeagueEvent_Text_AlreadyAccepted
	setvar VAR_TEMP_F, 1
	releaseall
	end

LeagueEvent_EventScript_EntranceDeclined::
	msgbox LeagueEvent_Text_Declined
	releaseall
	end

LeagueEvent_EventScript_EntranceDenied::
	special LeagueCircuit_BufferAdmissionDenial
	msgbox gStringVar4
	releaseall
	end

LeagueEvent_EventScript_SetRoomGraphics::
	special LeagueEvent_SetRoomGraphics
	end

LeagueEvent_EventScript_RoomIntro::
	special LeagueEvent_BufferRoomName
	msgbox LeagueEvent_Text_RoomIntro
	return

LeagueEvent_EventScript_ChampionIntro::
	special LeagueEvent_BufferRoomName
	msgbox LeagueEvent_Text_ChampionIntro
	return

LeagueEvent_EventScript_RoomOffer::
	special LeagueEvent_BufferRoomName
	msgbox LeagueEvent_Text_RoomOffer, MSGBOX_YESNO
	return

LeagueEvent_EventScript_ChampionOffer::
	special LeagueEvent_BufferRoomName
	msgbox LeagueEvent_Text_ChampionOffer, MSGBOX_YESNO
	return

LeagueEvent_EventScript_RoomAfter::
	special LeagueEvent_BufferRoomName
	msgbox LeagueEvent_Text_RoomAfter
	return

LeagueEvent_EventScript_ChampionAfter::
	special LeagueEvent_BufferRoomName
	msgbox LeagueEvent_Text_ChampionAfter
	return

LeagueEvent_EventScript_MastersAfter::
	special LeagueEvent_BufferRoomName
	msgbox LeagueEvent_Text_MastersAfter
	return

LeagueEvent_Text_RoomIntro::
	.string "I'm {STR_VAR_1}.\p"
	.string "Your next LEAGUE battle\n"
	.string "starts now!$"

LeagueEvent_Text_ChampionIntro::
	.string "I'm {STR_VAR_1}.\p"
	.string "This is the final round.\n"
	.string "Let's battle!$"

LeagueEvent_Text_RoomOffer::
	.string "I'm {STR_VAR_1}.\p"
	.string "Will you face my team?$"

LeagueEvent_Text_ChampionOffer::
	.string "I'm {STR_VAR_1}.\p"
	.string "Final battle! Are you ready?$"

LeagueEvent_Text_RoomWin::
	.string "You fought well.$"

LeagueEvent_Text_RoomAfter::
	.string "{STR_VAR_1}: Well fought.\p"
	.string "The next LEAGUE battle awaits.$"

LeagueEvent_Text_ChampionAfter::
	.string "{STR_VAR_1}: Well fought.\p"
	.string "The HALL OF FAME awaits you.$"

LeagueEvent_Text_MastersAfter::
	.string "{STR_VAR_1}: Well fought.\p"
	.string "The MASTERS GALLERY awaits.$"

LeagueEvent_Text_Offer::
	.string "An event is ready. Accept this\n"
	.string "LEAGUE challenge?$"

LeagueEvent_Text_Accepted::
	.string "Event accepted!\p"
	.string "Five opponents wait inside.$"

LeagueEvent_Text_AlreadyAccepted::
	.string "Your event is waiting.\p"
	.string "Final opponent: {STR_VAR_1}.$"

LeagueEvent_Text_Declined::
	.string "Come back when you're ready.$"
#endif

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

LeagueCircuit_EventScript_MastersEntryDenied::
	lockall
	special LeagueCircuit_AbandonRun
	warp MAP_SEVEN_ISLAND_HOUSE_ROOM1, 4, 5
	waitstate
	releaseall
	end

LeagueCircuit_EventScript_IndigoHallOfFame::
	specialvar VAR_0x8004, LeagueCircuit_GetRunRegion
	goto_if_eq VAR_0x8004, REGION_HOENN, LeagueCircuit_EventScript_IndigoEntryDenied
	goto_if_eq VAR_0x8004, REGION_NONE, LeagueCircuit_EventScript_IndigoEntryDenied
	specialvar VAR_RESULT, LeagueCircuit_ValidateRun
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
	.string "to enter the INDIGO LEAGUE.$"

LeagueCircuit_Text_NeedIndigoClear::
	.string "Clear the INDIGO LEAGUE first.$"

LeagueCircuit_Text_NeedSixteenBadges::
	.string "You need at least sixteen BADGES\n"
	.string "to enter the MASTERS challenge.$"

LeagueCircuit_Text_NeedMastersClear::
	.string "Clear the SEVII MASTERS first.$"

LeagueCircuit_Text_NeedTwentyFourBadges::
	.string "You need all twenty-four BADGES\n"
	.string "to enter the HOENN LEAGUE.$"

LeagueCircuit_Text_Unavailable::
	.string "There is no LEAGUE challenge\n"
	.string "available here.$"

LeagueCircuit_Text_NeedQualification::
	.string "A TRAINER RATING of at least 80\n"
	.string "is needed for LEAGUE events.$"

LeagueCircuit_Text_NeedRegionalBadge::
	.string "Earn a BADGE in this LEAGUE'S\n"
	.string "region before entering.$"

LeagueCircuit_Text_NeedMaster::
	.string "Clear both INDIGO and HOENN\n"
	.string "before entering the MASTERS.$"

LeagueCircuit_Text_EventElsewhere::
	.string "Another LEAGUE event is active.\n"
	.string "Finish it before entering here.$"

LeagueCircuit_Text_NoAcceptedEvent::
	.string "Accept this LEAGUE event at the\n"
	.string "entrance before going in.$"
#endif
