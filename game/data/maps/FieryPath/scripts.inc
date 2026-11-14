FieryPath_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FieryPath_OnTransition
	.byte 0

FieryPath_OnTransition:
	call_if_unset FLAG_LANDMARK_FIERY_PATH, FieryPath_EventScript_MoveScottToFallarbor
	setflag FLAG_LANDMARK_FIERY_PATH
	end

FieryPath_EventScript_MoveScottToFallarbor::
#if IS_WAYFARER
	@ SCOTT leaves VERDANTURF for good here, so a meeting there that has not happened yet is counted now
	call_if_unset FLAG_HIDE_VERDANTURF_TOWN_SCOTT, FieryPath_EventScript_CountClosedVerdanturfScott
#endif
	setflag FLAG_HIDE_VERDANTURF_TOWN_SCOTT
	clearflag FLAG_HIDE_FALLARBOR_TOWN_BATTLE_TENT_SCOTT
	return


#if IS_WAYFARER
FieryPath_EventScript_CountClosedVerdanturfScott::
	goto_if_set FLAG_MET_SCOTT_IN_VERDANTURF, Common_EventScript_NopReturn
	addvar VAR_SCOTT_STATE, 1
	setflag FLAG_MET_SCOTT_IN_VERDANTURF
	return
#endif
