@ Wayfarer owns route transition state while the selected FRLG maps retain
@ their original object, sign, Trainer, and dialogue labels.

#if IS_WAYFARER

WayfarerKantoCoast_Route19_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, WayfarerKantoCoast_Route19_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	.byte 0

WayfarerKantoCoast_Route19_OnTransition::
	callnative SetTimeBasedEncounters
	end

WayfarerKantoCoast_Route20_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, WayfarerKantoCoast_Route20_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	.byte 0

WayfarerKantoCoast_Route20_OnTransition::
	call_if_unset FLAG_WAYFARER_STOPPED_SEAFOAM_B3F_CURRENT, WayfarerKantoCoast_Route20_ResetSeafoamBouldersForB3F
	call_if_unset FLAG_WAYFARER_STOPPED_SEAFOAM_B4F_CURRENT, WayfarerKantoCoast_Route20_ResetSeafoamBouldersForB4F
	callnative SetTimeBasedEncounters
	end

WayfarerKantoCoast_Route20_ResetSeafoamBouldersForB3F::
	clearflag FLAG_WAYFARER_HIDE_SEAFOAM_1F_BOULDER_1
	clearflag FLAG_WAYFARER_HIDE_SEAFOAM_1F_BOULDER_2
	setflag FLAG_WAYFARER_HIDE_SEAFOAM_B1F_BOULDER_1
	setflag FLAG_WAYFARER_HIDE_SEAFOAM_B1F_BOULDER_2
	setflag FLAG_WAYFARER_HIDE_SEAFOAM_B2F_BOULDER_1
	setflag FLAG_WAYFARER_HIDE_SEAFOAM_B2F_BOULDER_2
	setflag FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_1
	setflag FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_2
	return

WayfarerKantoCoast_Route20_ResetSeafoamBouldersForB4F::
	clearflag FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_3
	clearflag FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_4
	clearflag FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_5
	clearflag FLAG_WAYFARER_HIDE_SEAFOAM_B3F_BOULDER_6
	setflag FLAG_WAYFARER_HIDE_SEAFOAM_B4F_BOULDER_1
	setflag FLAG_WAYFARER_HIDE_SEAFOAM_B4F_BOULDER_2
	return

WayfarerKantoCoast_Route21_North_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, WayfarerKantoCoast_Route21_North_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	.byte 0

WayfarerKantoCoast_Route21_North_OnTransition::
	callnative SetTimeBasedEncounters
	end

WayfarerKantoCoast_Route21_South_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, WayfarerKantoCoast_Route21_South_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	.byte 0

WayfarerKantoCoast_Route21_South_OnTransition::
	callnative SetTimeBasedEncounters
	end

@ Selected FRLG route dialogue is deliberately imported as small guarded
@ snippets. The Route 20 map-script state and Camper Fame Checker dependency
@ are replaced below with Wayfarer-owned behavior.
	.include "data/maps/Route19_Frlg/scripts.inc"
	.include "data/maps/Route21_North_Frlg/scripts.inc"
	.include "data/maps/Route21_South_Frlg/scripts.inc"

Route20_EventScript_SeafoamIslandsSign::
	msgbox Route20_Text_SeafoamIslands, MSGBOX_SIGN
	end

Route20_Text_BarryIntro::
	.string "The water is shallow here.\n"
	.string "There are many people swimming.$"

Route20_Text_BarryDefeat::
	.string "Splash!$"

Route20_Text_BarryPostBattle::
	.string "I wish I could ride my POKéMON.\n"
	.string "I bet you're not tired.$"

Route20_Text_ShirleyIntro::
	.string "SEAFOAM is a quiet getaway.\n"
	.string "I'm vacationing here.$"

Route20_Text_ShirleyDefeat::
	.string "Quit it!$"

Route20_Text_ShirleyPostBattle::
	.string "There's a huge cavern underneath\n"
	.string "this island.$"

Route20_Text_TiffanyIntro::
	.string "I love floating with the fishes\n"
	.string "here among the waves.$"

Route20_Text_TiffanyDefeat::
	.string "Yowch!$"

Route20_Text_TiffanyPostBattle::
	.string "Want to float with me?$"

Route20_Text_IreneIntro::
	.string "Are you on vacation, too?$"

Route20_Text_IreneDefeat::
	.string "No mercy at all!$"

Route20_Text_IrenePostBattle::
	.string "SEAFOAM used to be a single island\n"
	.string "in the distant past.$"

Route20_Text_DeanIntro::
	.string "Check out my buff physique!$"

Route20_Text_DeanDefeat::
	.string "Wimpy!$"

Route20_Text_DeanPostBattle::
	.string "I should've been buffing up my\n"
	.string "POKéMON, not me!$"

Route20_Text_DarrinIntro::
	.string "Why are you riding a POKéMON?\n"
	.string "Can't you swim?$"

Route20_Text_DarrinDefeat::
	.string "Ouch!\n"
	.string "Torpedoed!$"

Route20_Text_DarrinPostBattle::
	.string "Riding a POKéMON sure looks fun!$"

Route20_Text_RogerIntro::
	.string "I rode my bird POKéMON here.$"

Route20_Text_RogerDefeat::
	.string "Oh, no!\n"
	.string "Now what am I to do?$"

Route20_Text_RogerPostBattle::
	.string "My birds are exhausted.\n"
	.string "They can't FLY me back!$"

Route20_Text_NoraIntro::
	.string "My boyfriend gave me big pearls.$"

Route20_Text_NoraDefeat::
	.string "Oh, no!\n"
	.string "My pearls were in them!$"

Route20_Text_NoraPostBattle::
	.string "Will my pearls grow bigger inside\n"
	.string "CLOYSTER?$"

Route20_Text_MissyIntro::
	.string "I swam here from CINNABAR ISLAND.\n"
	.string "It wasn't easy, I tell you.$"

Route20_Text_MissyDefeat::
	.string "I'm so disappointed!$"

Route20_Text_MissyPostBattle::
	.string "POKéMON have taken over an\n"
	.string "abandoned mansion on CINNABAR.\p"
	.string "They call it the POKéMON MANSION\n"
	.string "now.$"

Route20_Text_MelissaIntro::
	.string "CINNABAR, in the west, has a LAB\n"
	.string "for POKéMON.\p"
	.string "My daddy works there.$"

Route20_Text_MelissaDefeat::
	.string "Wait!\n"
	.string "You're supposed to wait!$"

Route20_Text_SeafoamIslands::
	.string "SEAFOAM ISLANDS$"

Route20_Text_MistyTrainsHere::
	.string "Strong TRAINERS and WATER POKéMON\n"
	.string "are common sights in these parts.\p"
	.string "They say that MISTY of the\n"
	.string "CERULEAN GYM trains here.$"


Route20_EventScript_Camper::
	lock
	faceplayer
	msgbox Route20_Text_MistyTrainsHere, MSGBOX_DEFAULT
	release
	end

Route19_EventScript_Richard::
	trainerbattle_single TRAINER_SWIMMER_MALE_RICHARD, Route19_Text_RichardIntro, Route19_Text_RichardDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_RichardRematch
	msgbox Route19_Text_RichardPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_RichardRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_RICHARD, Route19_Text_RichardRematchIntro, Route19_Text_RichardDefeat
	msgbox Route19_Text_RichardPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Reece::
	trainerbattle_single TRAINER_SWIMMER_MALE_REECE, Route19_Text_ReeceIntro, Route19_Text_ReeceDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_ReeceRematch
	msgbox Route19_Text_ReecePostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_ReeceRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_REECE, Route19_Text_ReeceRematchIntro, Route19_Text_ReeceDefeat
	msgbox Route19_Text_ReecePostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Matthew::
	trainerbattle_single TRAINER_SWIMMER_MALE_MATTHEW, Route19_Text_MatthewIntro, Route19_Text_MatthewDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_MatthewRematch
	msgbox Route19_Text_MatthewPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_MatthewRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_MATTHEW, Route19_Text_MatthewRematchIntro, Route19_Text_MatthewDefeat
	msgbox Route19_Text_MatthewPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Douglas::
	trainerbattle_single TRAINER_SWIMMER_MALE_DOUGLAS, Route19_Text_DouglasIntro, Route19_Text_DouglasDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_DouglasRematch
	msgbox Route19_Text_DouglasPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_DouglasRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_DOUGLAS, Route19_Text_DouglasRematchIntro, Route19_Text_DouglasDefeat
	msgbox Route19_Text_DouglasPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_David::
	trainerbattle_single TRAINER_SWIMMER_MALE_DAVID, Route19_Text_DavidIntro, Route19_Text_DavidDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_DavidRematch
	msgbox Route19_Text_DavidPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_DavidRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_DAVID, Route19_Text_DavidRematchIntro, Route19_Text_DavidDefeat
	msgbox Route19_Text_DavidPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Tony::
	trainerbattle_single TRAINER_SWIMMER_MALE_TONY, Route19_Text_TonyIntro, Route19_Text_TonyDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_TonyRematch
	msgbox Route19_Text_TonyPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_TonyRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_TONY, Route19_Text_TonyRematchIntro, Route19_Text_TonyDefeat
	msgbox Route19_Text_TonyPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Axle::
	trainerbattle_single TRAINER_SWIMMER_MALE_AXLE, Route19_Text_AxleIntro, Route19_Text_AxleDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_AxleRematch
	msgbox Route19_Text_AxlePostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_AxleRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_AXLE, Route19_Text_AxleRematchIntro, Route19_Text_AxleDefeat
	msgbox Route19_Text_AxlePostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Anya::
	trainerbattle_single TRAINER_SWIMMER_FEMALE_ANYA, Route19_Text_AnyaIntro, Route19_Text_AnyaDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_AnyaRematch
	msgbox Route19_Text_AnyaPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_AnyaRematch::
	trainerbattle_rematch TRAINER_SWIMMER_FEMALE_ANYA, Route19_Text_AnyaRematchIntro, Route19_Text_AnyaDefeat
	msgbox Route19_Text_AnyaPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Alice::
	trainerbattle_single TRAINER_SWIMMER_FEMALE_ALICE, Route19_Text_AliceIntro, Route19_Text_AliceDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_AliceRematch
	msgbox Route19_Text_AlicePostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_AliceRematch::
	trainerbattle_rematch TRAINER_SWIMMER_FEMALE_ALICE, Route19_Text_AliceRematchIntro, Route19_Text_AliceDefeat
	msgbox Route19_Text_AlicePostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Connie::
	trainerbattle_single TRAINER_SWIMMER_FEMALE_CONNIE, Route19_Text_ConnieIntro, Route19_Text_ConnieDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_ConnieRematch
	msgbox Route19_Text_ConniePostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_ConnieRematch::
	trainerbattle_rematch TRAINER_SWIMMER_FEMALE_CONNIE, Route19_Text_ConnieRematchIntro, Route19_Text_ConnieDefeat
	msgbox Route19_Text_ConniePostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Lia::
	trainerbattle_double TRAINER_SIS_AND_BRO_LIA_LUC, Route19_Text_LiaIntro, Route19_Text_LiaDefeat, Route19_Text_LiaNotEnoughMons
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_LiaRematch
	msgbox Route19_Text_LiaPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_LiaRematch::
	trainerbattle_rematch_double TRAINER_SIS_AND_BRO_LIA_LUC, Route19_Text_LiaRematchIntro, Route19_Text_LiaDefeat, Route19_Text_LiaNotEnoughMons
	msgbox Route19_Text_LiaPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_Luc::
	trainerbattle_double TRAINER_SIS_AND_BRO_LIA_LUC, Route19_Text_LucIntro, Route19_Text_LucDefeat, Route19_Text_LucNotEnoughMons
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route19_EventScript_LucRematch
	msgbox Route19_Text_LucPostBattle, MSGBOX_AUTOCLOSE
	end

Route19_EventScript_LucRematch::
	trainerbattle_rematch_double TRAINER_SIS_AND_BRO_LIA_LUC, Route19_Text_LucRematchIntro, Route19_Text_LucDefeat, Route19_Text_LucNotEnoughMons
	msgbox Route19_Text_LucPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Barry::
	trainerbattle_single TRAINER_SWIMMER_MALE_BARRY, Route20_Text_BarryIntro, Route20_Text_BarryDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_BarryRematch
	msgbox Route20_Text_BarryPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_BarryRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_BARRY, Route20_Text_BarryRematchIntro, Route20_Text_BarryDefeat
	msgbox Route20_Text_BarryPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Dean::
	trainerbattle_single TRAINER_SWIMMER_MALE_DEAN, Route20_Text_DeanIntro, Route20_Text_DeanDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_DeanRematch
	msgbox Route20_Text_DeanPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_DeanRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_DEAN, Route20_Text_DeanRematchIntro, Route20_Text_DeanDefeat
	msgbox Route20_Text_DeanPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Darrin::
	trainerbattle_single TRAINER_SWIMMER_MALE_DARRIN, Route20_Text_DarrinIntro, Route20_Text_DarrinDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_DarrinRematch
	msgbox Route20_Text_DarrinPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_DarrinRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_DARRIN, Route20_Text_DarrinRematchIntro, Route20_Text_DarrinDefeat
	msgbox Route20_Text_DarrinPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Tiffany::
	trainerbattle_single TRAINER_SWIMMER_FEMALE_TIFFANY, Route20_Text_TiffanyIntro, Route20_Text_TiffanyDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_TiffanyRematch
	msgbox Route20_Text_TiffanyPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_TiffanyRematch::
	trainerbattle_rematch TRAINER_SWIMMER_FEMALE_TIFFANY, Route20_Text_TiffanyRematchIntro, Route20_Text_TiffanyDefeat
	msgbox Route20_Text_TiffanyPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Nora::
	trainerbattle_single TRAINER_SWIMMER_FEMALE_NORA, Route20_Text_NoraIntro, Route20_Text_NoraDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_NoraRematch
	msgbox Route20_Text_NoraPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_NoraRematch::
	trainerbattle_rematch TRAINER_SWIMMER_FEMALE_NORA, Route20_Text_NoraRematchIntro, Route20_Text_NoraDefeat
	msgbox Route20_Text_NoraPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Shirley::
	trainerbattle_single TRAINER_SWIMMER_FEMALE_SHIRLEY, Route20_Text_ShirleyIntro, Route20_Text_ShirleyDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_ShirleyRematch
	msgbox Route20_Text_ShirleyPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_ShirleyRematch::
	trainerbattle_rematch TRAINER_SWIMMER_FEMALE_SHIRLEY, Route20_Text_ShirleyRematchIntro, Route20_Text_ShirleyDefeat
	msgbox Route20_Text_ShirleyPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Roger::
	trainerbattle_single TRAINER_BIRD_KEEPER_ROGER, Route20_Text_RogerIntro, Route20_Text_RogerDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_RogerRematch
	msgbox Route20_Text_RogerPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_RogerRematch::
	trainerbattle_rematch TRAINER_BIRD_KEEPER_ROGER, Route20_Text_RogerRematchIntro, Route20_Text_RogerDefeat
	msgbox Route20_Text_RogerPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Missy::
	trainerbattle_single TRAINER_PICNICKER_MISSY, Route20_Text_MissyIntro, Route20_Text_MissyDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_MissyRematch
	msgbox Route20_Text_MissyPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_MissyRematch::
	trainerbattle_rematch TRAINER_PICNICKER_MISSY, Route20_Text_MissyRematchIntro, Route20_Text_MissyDefeat
	msgbox Route20_Text_MissyPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_Irene::
	trainerbattle_single TRAINER_PICNICKER_IRENE, Route20_Text_IreneIntro, Route20_Text_IreneDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_IreneRematch
	msgbox Route20_Text_IrenePostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_IreneRematch::
	trainerbattle_rematch TRAINER_PICNICKER_IRENE, Route20_Text_IreneRematchIntro, Route20_Text_IreneDefeat
	msgbox Route20_Text_IrenePostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_Ronald::
	trainerbattle_single TRAINER_FISHERMAN_RONALD, Route21_North_Text_RonaldIntro, Route21_North_Text_RonaldDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_North_EventScript_RonaldRematch
	msgbox Route21_North_Text_RonaldPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_RonaldRematch::
	trainerbattle_rematch TRAINER_FISHERMAN_RONALD, Route21_North_Text_RonaldRematchIntro, Route21_North_Text_RonaldDefeat
	msgbox Route21_North_Text_RonaldPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_Claude::
	trainerbattle_single TRAINER_FISHERMAN_CLAUDE, Route21_South_Text_ClaudeIntro, Route21_South_Text_ClaudeDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_South_EventScript_ClaudeRematch
	msgbox Route21_South_Text_ClaudePostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_ClaudeRematch::
	trainerbattle_rematch TRAINER_FISHERMAN_CLAUDE, Route21_South_Text_ClaudeRematchIntro, Route21_South_Text_ClaudeDefeat
	msgbox Route21_South_Text_ClaudePostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_Wade::
	trainerbattle_single TRAINER_FISHERMAN_WADE, Route21_North_Text_WadeIntro, Route21_North_Text_WadeDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_North_EventScript_WadeRematch
	msgbox Route21_North_Text_WadePostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_WadeRematch::
	trainerbattle_rematch TRAINER_FISHERMAN_WADE, Route21_North_Text_WadeRematchIntro, Route21_North_Text_WadeDefeat
	msgbox Route21_North_Text_WadePostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_Nolan::
	trainerbattle_single TRAINER_FISHERMAN_NOLAN, Route21_South_Text_NolanIntro, Route21_South_Text_NolanDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_South_EventScript_NolanRematch
	msgbox Route21_South_Text_NolanPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_NolanRematch::
	trainerbattle_rematch TRAINER_FISHERMAN_NOLAN, Route21_South_Text_NolanRematchIntro, Route21_South_Text_NolanDefeat
	msgbox Route21_South_Text_NolanPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_Spencer::
	trainerbattle_single TRAINER_SWIMMER_MALE_SPENCER, Route21_North_Text_SpencerIntro, Route21_North_Text_SpencerDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_North_EventScript_SpencerRematch
	msgbox Route21_North_Text_SpencerPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_SpencerRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_SPENCER, Route21_North_Text_SpencerRematchIntro, Route21_North_Text_SpencerDefeat
	msgbox Route21_North_Text_SpencerPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_Jack::
	trainerbattle_single TRAINER_SWIMMER_MALE_JACK, Route21_South_Text_JackIntro, Route21_South_Text_JackDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_South_EventScript_JackRematch
	msgbox Route21_South_Text_JackPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_JackRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_JACK, Route21_South_Text_JackRematchIntro, Route21_South_Text_JackDefeat
	msgbox Route21_South_Text_JackPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_Jerome::
	trainerbattle_single TRAINER_SWIMMER_MALE_JEROME, Route21_South_Text_JeromeIntro, Route21_South_Text_JeromeDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_South_EventScript_JeromeRematch
	msgbox Route21_South_Text_JeromePostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_JeromeRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_JEROME, Route21_South_Text_JeromeRematchIntro, Route21_South_Text_JeromeDefeat
	msgbox Route21_South_Text_JeromePostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_Roland::
	trainerbattle_single TRAINER_SWIMMER_MALE_ROLAND, Route21_South_Text_RolandIntro, Route21_South_Text_RolandDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_South_EventScript_RolandRematch
	msgbox Route21_South_Text_RolandPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_South_EventScript_RolandRematch::
	trainerbattle_rematch TRAINER_SWIMMER_MALE_ROLAND, Route21_South_Text_RolandRematchIntro, Route21_South_Text_RolandDefeat
	msgbox Route21_South_Text_RolandPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_Lil::
	trainerbattle_double TRAINER_SIS_AND_BRO_LIL_IAN, Route21_North_Text_LilIntro, Route21_North_Text_LilDefeat, Route21_North_Text_LilNotEnoughMons
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_North_EventScript_LilRematch
	msgbox Route21_North_Text_LilPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_LilRematch::
	trainerbattle_rematch_double TRAINER_SIS_AND_BRO_LIL_IAN, Route21_North_Text_LilRematchIntro, Route21_North_Text_LilDefeat, Route21_North_Text_LilNotEnoughMons
	msgbox Route21_North_Text_LilPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_Ian::
	trainerbattle_double TRAINER_SIS_AND_BRO_LIL_IAN, Route21_North_Text_IanIntro, Route21_North_Text_IanDefeat, Route21_North_Text_IanNotEnoughMons
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route21_North_EventScript_IanRematch
	msgbox Route21_North_Text_IanPostBattle, MSGBOX_AUTOCLOSE
	end

Route21_North_EventScript_IanRematch::
	trainerbattle_rematch_double TRAINER_SIS_AND_BRO_LIL_IAN, Route21_North_Text_IanRematchIntro, Route21_North_Text_IanDefeat, Route21_North_Text_IanNotEnoughMons
	msgbox Route21_North_Text_IanPostBattle, MSGBOX_AUTOCLOSE
	end

@ FRLG's post-battle line attributes Cinnabar's existence to an eruption.
@ Wayfarer retains the Trainer identity, party, rematch, and other dialogue
@ while excluding that unapproved story assertion.
Route20_EventScript_Melissa::
	trainerbattle_single TRAINER_SWIMMER_FEMALE_MELISSA, Route20_Text_MelissaIntro, Route20_Text_MelissaDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route20_EventScript_MelissaRematch
	msgbox Route20_Text_MelissaPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_EventScript_MelissaRematch::
	trainerbattle_rematch TRAINER_SWIMMER_FEMALE_MELISSA, Route20_Text_MelissaRematchIntro, Route20_Text_MelissaDefeat
	msgbox Route20_Text_MelissaPostBattle, MSGBOX_AUTOCLOSE
	end

Route20_Text_MelissaPostBattle::
	.string "CINNABAR is a volcanic island.\p"
	.string "The POKéMON LAB is on the west side.$"

Route19_Text_RichardRematchIntro::
	.string "I'm almost warmed up to go\n"
	.string "out for a swim.$"

Route19_Text_ReeceRematchIntro::
	.string "Wait! Slow down!\n"
	.string "What if you have a heart attack?$"

Route19_Text_MatthewRematchIntro::
	.string "I love swimming!\n"
	.string "I guess you're the surfing type.$"

Route19_Text_DouglasRematchIntro::
	.string "What's beyond the horizon?\n"
	.string "Have you seen?$"

Route19_Text_DavidRematchIntro::
	.string "I tried diving for POKéMON again,\n"
	.string "but it was a no-go like before.$"

Route19_Text_TonyRematchIntro::
	.string "I look at the sea to forget all\n"
	.string "the bad things that happened.\p"
	.string "…Like losing to you the last time!$"

Route19_Text_AnyaRematchIntro::
	.string "You always get to ride your\n"
	.string "POKéMON…\p"
	.string "It looks so relaxing.\n"
	.string "Can I have it if I win?$"

Route19_Text_AliceRematchIntro::
	.string "Swimming's great!\n"
	.string "Sunburns aren't!$"

Route19_Text_AxleRematchIntro::
	.string "These waters are treacherous!\n"
	.string "You shouldn't come back here!$"

Route19_Text_ConnieRematchIntro::
	.string "I swam here with my friends…\n"
	.string "I'm tired…\l"
	.string "Must we really battle again?$"

Route19_Text_LiaRematchIntro::
	.string "LIA: You know my brother just\n"
	.string "became a TRAINER, right?\p"
	.string "I want to make him better, so I\n"
	.string "need your help again.$"

Route19_Text_LucRematchIntro::
	.string "LUC: My big sis taught me all\n"
	.string "about POKéMON.\p"
	.string "I wonder if I'm better?$"

Route20_Text_BarryRematchIntro::
	.string "The water is shallow here.\n"
	.string "There are many people swimming.$"

Route20_Text_ShirleyRematchIntro::
	.string "Are you jealous that I'm\n"
	.string "vacationing on SEAFOAM?$"

Route20_Text_TiffanyRematchIntro::
	.string "I love floating with the fishes\n"
	.string "here among the waves.$"

Route20_Text_IreneRematchIntro::
	.string "Were you on vacation, too?$"

Route20_Text_DeanRematchIntro::
	.string "Check out my physique!\p"
	.string "It's even more buff than ever\n"
	.string "before!$"

Route20_Text_DarrinRematchIntro::
	.string "Why are you riding a POKéMON?\n"
	.string "Haven't you learned to swim yet?$"

Route20_Text_RogerRematchIntro::
	.string "I rode my bird POKéMON here.$"

Route20_Text_NoraRematchIntro::
	.string "My boyfriend gave me big pearls.\n"
	.string "And, they've grown bigger!$"

Route20_Text_MissyRematchIntro::
	.string "I swam here from CINNABAR ISLAND.\n"
	.string "It wasn't easy, like I said.$"

Route20_Text_MelissaRematchIntro::
	.string "CINNABAR, in the west, has a LAB\n"
	.string "for POKéMON.\p"
	.string "My daddy works there.$"

Route21_North_Text_RonaldRematchIntro::
	.string "You want to know if the fish are\n"
	.string "biting?$"

Route21_North_Text_WadeRematchIntro::
	.string "I got a big haul again!\n"
	.string "Wanna go for it once more?$"

Route21_North_Text_SpencerRematchIntro::
	.string "The sea cleanses my body and soul!$"

Route21_South_Text_JackRematchIntro::
	.string "I caught my POKéMON at sea.\n"
	.string "And that's where I train them.$"

Route21_South_Text_JeromeRematchIntro::
	.string "Right now, I'm in another triathlon\n"
	.string "meet!$"

Route21_South_Text_RolandRematchIntro::
	.string "Ahh!\n"
	.string "Feel the sun and the wind!$"

Route21_South_Text_ClaudeRematchIntro::
	.string "Hey, cut it out already.\n"
	.string "You always scare away the fish!$"

Route21_South_Text_NolanRematchIntro::
	.string "Keep me company till I get a hit.$"

Route21_North_Text_LilRematchIntro::
	.string "LIL: Huh? A battle again?\n"
	.string "IAN, can't you do it alone?$"

Route21_North_Text_IanRematchIntro::
	.string "IAN: My sis is still a slob.\n"
	.string "Help me get her into shape!$"


@ Kanto memorial floors use the Wayfarer event script set alongside the coast maps.
	.include "data/maps/PokemonTower_2F_Frlg/scripts.inc"
	.include "data/maps/PokemonTower_3F_Frlg/scripts.inc"
	.include "data/maps/PokemonTower_4F_Frlg/scripts.inc"
	.include "data/maps/PokemonTower_5F_Frlg/scripts.inc"
	.include "data/maps/PokemonTower_6F_Frlg/scripts.inc"
	.include "data/maps/PokemonTower_7F_Frlg/scripts.inc"

#endif
