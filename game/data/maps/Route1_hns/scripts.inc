Route1_hns_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Route1_KantoOnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, Route1_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	.byte 0

#if IS_WAYFARER
Route1_KantoOnLoad:
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, Route1_KantoShowTrainers
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, Route1_KantoShowTrainers
	setflag FLAG_TEMP_1
	end

Route1_KantoShowTrainers:
	clearflag FLAG_TEMP_1
	end
#else
Route1_KantoOnLoad:
	end
#endif

Route1_OnTransition: 
	callnative SetTimeBasedEncounters
	end

Route1_EventScript_SchoolboyDanny::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, Route1_EventScript_DannyBattle
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, FALSE, Route1_EventScript_KantoNoBattle
Route1_EventScript_DannyBattle::
#endif
	trainerbattle_single TRAINER_DANNY_HNS, Route1_Text_SchoolboyDanny_Seen, Route1_Text_SchoolboyDanny_Beaten
	msgbox Route1_Text_SchoolboyDanny_AfterBattle, MSGBOX_AUTOCLOSE
	end

Route1_EventScript_Quinn::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, Route1_EventScript_QuinnBattle
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, FALSE, Route1_EventScript_KantoNoBattle
Route1_EventScript_QuinnBattle::
#endif
	trainerbattle_single TRAINER_QUINN_HNS, Route1_Text_CooltrainerFQuinn_Seen, Route1_Text_CooltrainerFQuinn_Beaten
	msgbox Route1_Text_CooltrainerFQuinn_AfterBattle, MSGBOX_AUTOCLOSE
	end

#if IS_WAYFARER
Route1_EventScript_KantoNoBattle::
	msgbox Route1_Text_KantoNoBattle, MSGBOX_NPC
	end

Route1_Text_KantoNoBattle:
	.string "The road to VIRIDIAN CITY is\n"
	.string "usually quiet this time of day.$"
#endif

Route1_EventScript_Sign::
	msgbox Route1_Text_Sign, MSGBOX_SIGN
	end

Route1_Text_SchoolboyDanny_Seen:
	.string "If trainers meet, the first\n"
	.string "thing to do is battle.$"

Route1_Text_SchoolboyDanny_Beaten:
	.string "Awww… I've got a losing record…$"

Route1_Text_SchoolboyDanny_AfterBattle:
	.string "For trainers, it's a given that\n"
	.string "we'll battle whenever we meet.$"

Route1_Text_CooltrainerFQuinn_Seen:
	.string "You there! Want to battle?$"

Route1_Text_CooltrainerFQuinn_Beaten:
	.string "Down and out…$"

Route1_Text_CooltrainerFQuinn_AfterBattle:
	.string "You're strong.\p"
	.string "You obviously must have trained\n"
	.string "hard.$"

Route1_Text_Sign:
	.string "ROUTE 1\p"
	.string "PALLET TOWN - VIRIDIAN CITY$"
