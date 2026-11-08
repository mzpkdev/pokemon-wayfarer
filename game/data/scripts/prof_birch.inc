@ BIRCH's opening is over once he has given the POKéDEX, or for a visitor once he has been rescued
ProfBirch_EventScript_CheckOpeningOver::
	specialvar VAR_RESULT, WayfarerIsHoennOrigin
	goto_if_eq VAR_RESULT, FALSE, ProfBirch_EventScript_CheckRescued
	goto_if_unset FLAG_RECEIVED_POKEDEX_FROM_BIRCH, ProfBirch_EventScript_OpeningNotOver
	return

ProfBirch_EventScript_CheckRescued::
	setvar VAR_RESULT, TRUE
	goto_if_unset FLAG_RESCUED_BIRCH, ProfBirch_EventScript_OpeningNotOver
	return

ProfBirch_EventScript_OpeningNotOver::
	setvar VAR_RESULT, FALSE
	return

ProfBirch_EventScript_UpdateLocation::
	goto_if_set FLAG_SYS_GAME_CLEAR, ProfBirch_EventScript_MoveToLab
	call ProfBirch_EventScript_CheckOpeningOver
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_NopReturn
	@ A rescued visitor who has not yet been thanked keeps him in the lab, where that scene plays
	goto_if_eq VAR_BIRCH_LAB_STATE, 2, ProfBirch_EventScript_MoveToLab
	call_if_eq VAR_BIRCH_STATE, 0, ProfBirch_EventScript_MoveToLab
	call_if_eq VAR_BIRCH_STATE, 1, ProfBirch_EventScript_MoveToLab
	call_if_eq VAR_BIRCH_STATE, 2, ProfBirch_EventScript_MoveToRoute101
	call_if_eq VAR_BIRCH_STATE, 3, ProfBirch_EventScript_MoveToRoute101
	call_if_eq VAR_BIRCH_STATE, 4, ProfBirch_EventScript_MoveToRoute103
	call_if_eq VAR_BIRCH_STATE, 5, ProfBirch_EventScript_MoveToRoute103
	call_if_eq VAR_BIRCH_STATE, 6, ProfBirch_EventScript_MoveToLab
	call_if_eq VAR_BIRCH_STATE, 7, ProfBirch_EventScript_MoveToLab
	return

ProfBirch_EventScript_MoveToLab::
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_UNKNOWN_0x380
	setflag FLAG_HIDE_ROUTE_101_BIRCH
	setflag FLAG_HIDE_ROUTE_103_BIRCH
	return

ProfBirch_EventScript_MoveToRoute101::
	clearflag FLAG_HIDE_ROUTE_101_BIRCH
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_UNKNOWN_0x380
	setflag FLAG_HIDE_ROUTE_103_BIRCH
	return

ProfBirch_EventScript_MoveToRoute103::
	clearflag FLAG_HIDE_ROUTE_103_BIRCH
	setflag FLAG_HIDE_ROUTE_101_BIRCH
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_UNKNOWN_0x380
	return

ProfBirch_EventScript_RatePokedexOrRegister::
	lock
	faceplayer
#if IS_WAYFARER
	@ Only BIRCH in Hoenn runs this script: ELM rates the POKéDEX over the phone. The phone has no
	@ BIRCH to register, so he goes straight to the rating.
#elif IS_HNS
	goto_if_eq VAR_NEWBARKTOWN_LABSTATE, 3, ProfBirch_EventScript_GoFindMrPokemon
	goto_if_eq VAR_NEWBARKTOWN_LABSTATE, 4, ProfBirch_EventScript_Robbery
#else
	goto_if_unset FLAG_HAS_MATCH_CALL, ProfBirch_EventScript_AskRatePokedex
	goto_if_unset FLAG_ENABLE_PROF_BIRCH_MATCH_CALL, EventScript_RegisterProfBirch
#endif

#if IS_HNS && !IS_WAYFARER
ProfBirch_EventScript_GoFindMrPokemon::
	msgbox gElmDexRatingText_GoFindMrPokemon, MSGBOX_DEFAULT
	release
	end

ProfBirch_EventScript_Robbery::
	msgbox gElmDexRatingText_Robbery, MSGBOX_DEFAULT
	release
	end
#endif

ProfBirch_EventScript_AskRatePokedex::
	msgbox gBirchDexRatingText_AreYouCurious, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ProfBirch_EventScript_DeclineRating
	call ProfBirch_EventScript_RatePokedex
	release
	end

ProfBirch_EventScript_DeclineRating::
	msgbox gBirchDexRatingText_Cancel, MSGBOX_DEFAULT
	release
	end

ProfBirch_EventScript_ShowRatingMessage::
	copyvar VAR_0x8004, VAR_0x8009
	special ShowPokedexRatingMessage
	waitmessage
	waitbuttonpress
	return

ProfBirch_EventScript_RatePokedex::
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, ScriptGetPokedexInfo
	copyvar VAR_0x8008, VAR_0x8005
	copyvar VAR_0x8009, VAR_0x8006
	copyvar VAR_0x800A, VAR_RESULT
	buffernumberstring STR_VAR_1, VAR_0x8008  @ Num Hoenn seen
	buffernumberstring STR_VAR_2, VAR_0x8009  @ Num Hoenn caught
	msgbox gBirchDexRatingText_SoYouveSeenAndCaught, MSGBOX_DEFAULT
	call ProfBirch_EventScript_ShowRatingMessage
	goto_if_eq VAR_0x800A, 0, Common_EventScript_NopReturn  @ National dex not enabled
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, ScriptGetPokedexInfo
	copyvar VAR_0x8008, VAR_0x8005
	copyvar VAR_0x8009, VAR_0x8006
	buffernumberstring STR_VAR_1, VAR_0x8008  @ Num National seen
	buffernumberstring STR_VAR_2, VAR_0x8009  @ Num National caught
	msgbox gBirchDexRatingText_OnANationwideBasis, MSGBOX_DEFAULT
	return
