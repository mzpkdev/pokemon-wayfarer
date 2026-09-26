Route1_hns_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Route1_KantoOnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, Route1_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	.byte 0

#if IS_WAYFARER
@ FLAG_TEMP_1 hides the HNS Trainers during the opening; FLAG_TEMP_2 hides the
@ FRLG Route 1 residents (Mart clerk, ledge boy) from every other origin.
Route1_KantoOnLoad:
	setflag FLAG_TEMP_2
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, Route1_KantoShowTrainers
	clearflag FLAG_TEMP_2
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, Route1_KantoShowTrainers
	setflag FLAG_TEMP_1
	end

Route1_KantoShowTrainers:
	clearflag FLAG_TEMP_1
	end
#else
Route1_KantoOnLoad:
	setflag FLAG_TEMP_2
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
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, Route1_EventScript_SignNative
	msgbox Route1_Text_KantoRouteSign, MSGBOX_SIGN
	end
Route1_EventScript_SignNative:
#endif
	msgbox Route1_Text_Sign, MSGBOX_SIGN
	end

@ FRLG Route1_EventScript_MartClerk. The Wayfarer town state replaces
@ FLAG_GOT_POTION_ON_ROUTE_1, which is zero on HNS.
Route1_EventScript_KantoMartClerk::
#if IS_WAYFARER
	lock
	faceplayer
	callnative WayfarerKantoTown_HasRoute1Potion
	goto_if_eq VAR_RESULT, TRUE, Route1_EventScript_KantoAlreadyGotPotion
	msgbox Route1_Text_KantoWorkAtPokeMartTakeSample
	textcolor NPC_TEXT_COLOR_NEUTRAL
	checkitemspace ITEM_POTION
	goto_if_eq VAR_RESULT, FALSE, EventScript_BagIsFull
	bufferitemname STR_VAR_2, ITEM_POTION
	playfanfare MUS_LEVEL_UP
	message gText_ObtainedTheItem
	waitmessage
	waitfanfare
	additem ITEM_POTION
	msgbox Route1_Text_KantoPutPotionAway
	call EventScript_RestorePrevTextColor
	callnative WayfarerKantoTown_MarkRoute1Potion
	release
#endif
	end

Route1_EventScript_KantoBoy::
#if IS_WAYFARER
	msgbox Route1_Text_KantoCanJumpFromLedges, MSGBOX_NPC
#endif
	end

#if IS_WAYFARER
Route1_EventScript_KantoAlreadyGotPotion:
	msgbox Route1_Text_KantoComeSeeUsIfYouNeedPokeBalls
	release
	end

Route1_Text_KantoWorkAtPokeMartTakeSample:
	.string "Hi!\n"
	.string "I work at a POKéMON MART.\p"
	.string "It's part of a convenient chain\n"
	.string "selling all sorts of items.\p"
	.string "Please, visit us in VIRIDIAN CITY.\p"
	.string "I know, I'll give you a sample.\n"
	.string "Here you go!$"

Route1_Text_KantoComeSeeUsIfYouNeedPokeBalls:
	.string "Please come see us if you need\n"
	.string "POKé BALLS for catching POKéMON.$"

Route1_Text_KantoPutPotionAway:
	.string "{PLAYER} put the POTION away in\n"
	.string "the BAG's ITEMS POCKET.$"

Route1_Text_KantoCanJumpFromLedges:
	.string "See those ledges along the road?\p"
	.string "It's a bit scary, but you can jump\n"
	.string "from them.\p"
	.string "You can get back to PALLET TOWN\n"
	.string "quicker that way.$"

Route1_Text_KantoRouteSign:
	.string "ROUTE 1\n"
	.string "PALLET TOWN - VIRIDIAN CITY$"
#endif

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
