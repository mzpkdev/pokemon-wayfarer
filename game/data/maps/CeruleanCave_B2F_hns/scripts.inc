CeruleanCave_B2F_hns_MapScripts::
	.byte 0

CeruleanCave3_EventScript_MewtwoNew::
	lock
	faceplayer
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerCeruleanCave_IsMewtwoEligible
	goto_if_eq VAR_RESULT, FALSE, CeruleanCave3_EventScript_MewtwoNotReady
#endif
	playmoncry SPECIES_MEWTWO, CRY_MODE_ENCOUNTER
	waitmoncry
	seteventmon SPECIES_MEWTWO, 80
	special BattleSetup_StartLegendaryBattle
waitstate
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, CeruleanCave3_EventScript_MewtwoRan
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject LOCALID_CERULEANCAVE_MEWTWO
	setflag FLAG_HIDE_MEWTWO
	fadescreenswapbuffers FADE_FROM_BLACK
	release
	end

CeruleanCave3_EventScript_MewtwoRan::
	release
	end

#if IS_WAYFARER
CeruleanCave3_EventScript_MewtwoNotReady::
	msgbox CeruleanCave3_Text_MewtwoNotReady
	release
	end

CeruleanCave3_Text_MewtwoNotReady:
	.string "MEWTWO is watching you warily.\p"
	.string "You need more experience before\n"
	.string "you can challenge it.$"
#endif

CeruleanCave3_EventScript_MaxRevive::
    finditem ITEM_MAX_REVIVE
    end

CeruleanCave3_EventScript_UltraBall::
    finditem ITEM_ULTRA_BALL
    end

