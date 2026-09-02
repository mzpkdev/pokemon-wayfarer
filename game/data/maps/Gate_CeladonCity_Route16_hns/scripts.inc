Gate_CeladonCity_Route16_hns_MapScripts::
	.byte 0

Gate_CeladonCity_Route16_TriggerScript::
	lockall
	goto_if_set FLAG_SYS_CYCLING_ROAD, Gate_CeladonCity_Route16_ExitCyclingRoad
	goto Gate_CeladonCity_Route16_EnterCyclingRoad

Gate_CeladonCity_Route16_DeclineLoan::
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkRight1
	waitmovement 0
	releaseall
	end

Gate_CeladonCity_Route16_EnterCyclingRoad::
	msgbox Gate_CeladonCity_Route16_Text_OfferLoan, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Gate_CeladonCity_Route16_DeclineLoan
	specialvar VAR_RESULT, GetPlayerAvatarBike
	goto_if_ne VAR_RESULT, 0, Gate_CeladonCity_Route16_Mounted
	special ForcePlayerOntoBike
Gate_CeladonCity_Route16_Mounted::
	setflag FLAG_SYS_CYCLING_ROAD
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkLeft1
	waitmovement 0
	releaseall
	end

Gate_CeladonCity_Route16_ExitCyclingRoad::
	checkitem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, TRUE, Gate_CeladonCity_Route16_FinishExit
	special ForcePlayerOffBikeAndRestoreMusic
Gate_CeladonCity_Route16_FinishExit::
	clearflag FLAG_SYS_CYCLING_ROAD
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkRight1
	waitmovement 0
	releaseall
	end

Gate_CeladonCity_Route16_Text_OfferLoan:
	.string "CYCLING ROAD bicycles are available\n"
	.string "on loan. Would you like to ride?$"
