#if IS_WAYFARER
#if WAYFARER_LEAGUE_EVENTS
@ Indigo and Hoenn lobbies: the staff name the reigning champion, then admit an event accepted by phone or turn
@ the player away with the immediate reason. Their invitations come only by phone (league_phone.inc).
LeagueEvent_EventScript_Entrance::
	lockall
	call LeagueEvent_EventScript_ReigningChampion
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	goto_if_eq VAR_RESULT, FALSE, LeagueEvent_EventScript_EntranceDenied
	special LeagueEvent_BufferChampion
	msgbox LeagueEvent_Text_AlreadyAccepted
	setvar VAR_TEMP_F, 1
	releaseall
	end

@ The Hoenn door guards' refusal: the reigning champion, then the immediate reason.
LeagueEvent_EventScript_TurnedAway::
	lockall
	call LeagueEvent_EventScript_ReigningChampion
	goto LeagueEvent_EventScript_EntranceDenied

LeagueEvent_EventScript_EntranceDenied::
	@ A waiting invitation the player can't take by phone is handed over here instead.
	specialvar VAR_RESULT, LeaguePhone_ShouldDeliverAtDesk
	goto_if_eq VAR_RESULT, TRUE, LeaguePhone_EventScript_DeskInvitation
	special LeagueCircuit_BufferAdmissionDenial
	msgbox gStringVar4
	releaseall
	end

@ VAR_0x8004 is the league's circuit stage, which is its league id.
LeagueEvent_EventScript_ReigningChampion::
	specialvar VAR_RESULT, LeaguePhone_BufferReigningChampion
	goto_if_eq VAR_RESULT, LEAGUE_PHONE_CHAMPION_PLAYER, LeagueEvent_EventScript_ReigningChampionPlayer
	goto_if_eq VAR_RESULT, LEAGUE_PHONE_CHAMPION_TRAINER, LeagueEvent_EventScript_ReigningChampionTrainer
	msgbox LeagueEvent_Text_NoReigningChampion
	return

LeagueEvent_EventScript_ReigningChampionPlayer:
	msgbox LeagueEvent_Text_PlayerReigns
	return

LeagueEvent_EventScript_ReigningChampionTrainer:
	msgbox LeagueEvent_Text_TrainerReigns
	return

LeagueEvent_EventScript_SetRoomGraphics::
	special LeagueEvent_SetRoomGraphics
	end

LeagueEvent_EventScript_HallBoard::
	specialvar VAR_RESULT, LeagueEvent_BufferHallBoard
	goto_if_eq VAR_RESULT, FALSE, LeagueEvent_EventScript_HallBoardUnavailable
	msgbox gStringVar4, MSGBOX_SIGN
	return

LeagueEvent_EventScript_HallBoardUnavailable:
	msgbox LeagueCircuit_Text_Unavailable, MSGBOX_SIGN
	return

LeagueEvent_EventScript_OfferHallBoard::
	msgbox LeagueEvent_Text_HallsOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LeagueEvent_EventScript_HallBoardDeclined
	call LeagueEvent_EventScript_HallBoard
LeagueEvent_EventScript_HallBoardDeclined:
	return

@ The host introduces themselves once (notable-story-continuity.md#2-introductions): a notable trainer the player
@ knows gives their greeting in place of "I'm X.", one met here is marked introduced, and any other host, whose
@ name only the room knows, always says it. The rest of the line follows in each script.
LeagueEvent_EventScript_IntroduceHost::
	specialvar VAR_0x8004, LeagueEvent_GetRoomHostCharacter
	goto_if_eq VAR_0x8004, NOTABLE_TRAINER_NONE, LeagueEvent_EventScript_IntroduceStranger
	loadword 0, LeagueEvent_Text_ImHost
	goto NotableStory_EventScript_Introduce

LeagueEvent_EventScript_IntroduceStranger::
	msgbox LeagueEvent_Text_ImHost
	return

LeagueEvent_EventScript_RoomIntro::
	special LeagueEvent_BufferRoomName
	call LeagueEvent_EventScript_IntroduceHost
	msgbox LeagueEvent_Text_RoomIntro
	return

LeagueEvent_EventScript_ChampionIntro::
	special LeagueEvent_BufferRoomName
	call LeagueEvent_EventScript_IntroduceHost
	msgbox LeagueEvent_Text_ChampionIntro
	return

LeagueEvent_EventScript_RoomOffer::
	special LeagueEvent_BufferRoomName
	call LeagueEvent_EventScript_IntroduceHost
	msgbox LeagueEvent_Text_RoomOffer, MSGBOX_YESNO
	return

LeagueEvent_EventScript_ChampionOffer::
	special LeagueEvent_BufferRoomName
	call LeagueEvent_EventScript_IntroduceHost
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

LeagueEvent_Text_ImHost::
	.string "I'm {STR_VAR_1}.$"

LeagueEvent_Text_RoomIntro::
	.string "Your next LEAGUE battle\n"
	.string "starts now!$"

LeagueEvent_Text_ChampionIntro::
	.string "This is the final round.\n"
	.string "Let's battle!$"

LeagueEvent_Text_RoomOffer::
	.string "Will you face my team?$"

LeagueEvent_Text_ChampionOffer::
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

LeagueEvent_Text_AlreadyAccepted::
	.string "Your event is waiting.\p"
	.string "Final opponent: {STR_VAR_1}.$"

LeagueEvent_Text_NoReigningChampion::
	.string "Welcome! The {STR_VAR_1} has\n"
	.string "no reigning CHAMPION yet.$"

LeagueEvent_Text_PlayerReigns::
	.string "Welcome back, CHAMPION!\p"
	.string "You are the reigning CHAMPION\n"
	.string "of the {STR_VAR_1}.$"

LeagueEvent_Text_TrainerReigns::
	.string "Welcome! The reigning CHAMPION of\n"
	.string "the {STR_VAR_1} is {STR_VAR_3}.$"

LeagueEvent_Text_HallsOffer::
	.string "Want to see each HALL'S\n"
	.string "battle condition?$"
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

LeagueCircuit_Text_NeedMaster::
	.string "Clear both INDIGO and HOENN\n"
	.string "before entering the MASTERS.$"

LeagueCircuit_Text_EventElsewhere::
	.string "Another LEAGUE event is active.\n"
	.string "Finish it before entering here.$"

LeagueCircuit_Text_NoAcceptedEvent::
	.string "Accept this LEAGUE event at the\n"
	.string "entrance before going in.$"

LeagueCircuit_Text_CallPending::
	.string "A LEAGUE invitation is waiting for\n"
	.string "you. Please answer it first.$"

LeagueCircuit_Text_NoInvitation::
	.string "Only invited TRAINERS may enter.\p"
	.string "The next LEAGUE invitation should\n"
	.string "come in {STR_VAR_1} days.$"

LeagueCircuit_Text_NoInvitationTomorrow::
	.string "Only invited TRAINERS may enter.\p"
	.string "The next LEAGUE invitation should\n"
	.string "come tomorrow.$"

LeagueCircuit_Text_NotKnown::
	.string "Only invited TRAINERS may enter.\p"
	.string "This LEAGUE invites TRAINERS with\n"
	.string "a BADGE from its region.$"

LeagueCircuit_Text_NoInvitationDue::
	.string "Only invited TRAINERS may enter.\p"
	.string "The next LEAGUE invitation should\n"
	.string "come any time now.$"
#endif

@ The invitation and result calls (src/league_phone.c).
	.include "data/scripts/league_phone.inc"
