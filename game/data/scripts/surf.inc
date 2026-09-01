EventScript_UseSurf::
	lockall
	checkfieldmove FIELD_MOVE_SURF, TRUE
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_FieldMoveFailure
	bufferpartymonnick STR_VAR_1, VAR_RESULT
	setfieldeffectargument 0, VAR_RESULT
	msgbox gText_WantToUseSurf, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_ReleaseUseSurf
	msgbox gText_PlayerUsedSurf, MSGBOX_DEFAULT
	dofieldeffect FLDEFF_USE_SURF
EventScript_ReleaseUseSurf::
	releaseall
EventScript_EndUseSurf::
	end

EventScript_CurrentTooFast::
	lockall
	checkfieldmove FIELD_MOVE_SURF
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_FieldMoveFailure
	playse SE_SELECT
	msgbox Text_CurrentTooFast
EventScript_CurrentTooFastEndMsg::
	releaseall
	end

EventScript_FieldMoveFailure::
	goto_if_eq VAR_0x8008, FIELD_MOVE_USER_MISSING_ITEM, EventScript_FieldMoveMissingHM
	msgbox Text_NoPokemonCanUseFieldMove, MSGBOX_DEFAULT
	releaseall
	end

EventScript_FieldMoveMissingHM::
	msgbox Text_FieldMoveNeedsHM, MSGBOX_DEFAULT
	releaseall
	end

Text_FieldMoveNeedsHM:
	.string "You need the matching HM to use\n"
	.string "that move.$"

Text_NoPokemonCanUseFieldMove:
	.string "None of your party POKéMON can use\n"
	.string "that move.$"
