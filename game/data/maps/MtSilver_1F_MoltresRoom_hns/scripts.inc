MtSilver_1F_MoltresRoom_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MtSilver_1F_MoltresRoom_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, MtSilver_1F_MoltresRoom_OnResume
	.byte 0

MtSilver_1F_MoltresRoom_OnResume::
	setescapewarp MAP_MT_SILVER_OUTSIDE_HNS, 17, 8
	end

MtSilver_1F_MoltresRoom_OnTransition: 
	callnative SetTimeBasedEncounters
	end


MtSilver_1F_MoltresRoom_EventScript_Moltres::
	lock
	faceplayer
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerMtSilver_IsMoltresEligible
	goto_if_eq VAR_RESULT, FALSE, MtSilver_1F_MoltresRoom_EventScript_MoltresNotReady
#endif
	playmoncry SPECIES_MOLTRES, CRY_MODE_ENCOUNTER
	msgbox Text_Gyaoo
	delay 40
	waitmoncry
	seteventmon SPECIES_MOLTRES, 50
	special BattleSetup_StartLegendaryBattle
	waitstate
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, MtSilver_1F_MoltresRoom_EventScript_MoltresRan
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject LOCALID_MTSILVER_MOLTRES
	setflag FLAG_HIDE_MOLTRES
	fadescreenswapbuffers FADE_FROM_BLACK
	release
	end

MtSilver_1F_MoltresRoom_EventScript_MoltresRan::
	release
	end

#if IS_WAYFARER
MtSilver_1F_MoltresRoom_EventScript_MoltresNotReady::
	msgbox MtSilver_1F_MoltresRoom_Text_MoltresNotReady
	release
	end

MtSilver_1F_MoltresRoom_Text_MoltresNotReady:
	.string "MOLTRES is watching you warily.\p"
	.string "You need more experience before\n"
	.string "you can challenge it.$"
#endif
