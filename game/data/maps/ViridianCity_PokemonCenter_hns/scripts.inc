ViridianCity_PokemonCenter_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_PokemonCenter_OnTransition
	.byte 0

ViridianCity_PokemonCenter_OnTransition::
	setrespawn HEAL_LOCATION_VIRIDIAN_CITY_HNS
	end

ViridianCity_PokemonCenter_EventScript_CooltrainerM::
#if IS_WAYFARER
	@ Only the Pallet-origin opening swaps in its Route 1 line.
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_PokemonCenter_EventScript_CooltrainerMNative
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_PokemonCenter_EventScript_KantoVisitor
ViridianCity_PokemonCenter_EventScript_CooltrainerMNative:
	goto_if_set FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS, ViridianCity_PokemonCenter_EventScript_CooltrainerMBlue
#else
	goto_if_set FLAG_HIDE_CINNABAR_BLUE, ViridianCity_PokemonCenter_EventScript_CooltrainerMBlue
#endif
	msgbox ViridianCity_PokemonCenter_Text_1FCooltrainerM, MSGBOX_NPC
	end

#if IS_WAYFARER
ViridianCity_PokemonCenter_EventScript_KantoVisitor::
	msgbox ViridianCity_PokemonCenter_Text_KantoVisitor, MSGBOX_NPC
	end

ViridianCity_PokemonCenter_Text_KantoVisitor:
	.string "This POKéMON CENTER is a good\n"
	.string "place to rest before ROUTE 1.$"
#endif

ViridianCity_PokemonCenter_EventScript_CooltrainerMBlue::
	msgbox ViridianCity_PokemonCenter_Text_1FCooltrainerM_BlueReturned, MSGBOX_NPC
	end

ViridianCity_PokemonCenter_EventScript_CooltrainerF::
	msgbox ViridianCity_PokemonCenter_Text_1FCooltrainerF, MSGBOX_NPC
	end

ViridianCity_PokemonCenter_EventScript_BugCatcher::
	msgbox ViridianCity_PokemonCenter_Text_1FBugCatcher, MSGBOX_NPC
	end

ViridianCity_PokemonCenter_Text_1FCooltrainerM:
#if IS_WAYFARER
	.string "VIRIDIAN's GYM has a strange\n"
	.string "floor full of moving arrows.\p"
	.string "I hear GIOVANNI leads it.$"
#else
	.string "Where in the world is VIRIDIAN's GYM\n"
	.string "LEADER? I wanted to challenge him.$"
#endif

ViridianCity_PokemonCenter_Text_1FCooltrainerM_BlueReturned:
#if IS_WAYFARER
	.string "GIOVANNI left the GYM, but the\n"
	.string "Trainers are still there.$"
#else
	.string "There are no GYM TRAINERS at the\n"
	.string "VIRIDIAN GYM.\p"
	.string "The LEADER claims his policy is to\n"
	.string "win without having any underlings.$"
#endif

ViridianCity_PokemonCenter_Text_1FCooltrainerF:
#if IS_WAYFARER
	.string "BLAINE runs the GYM on CINNABAR.\p"
	.string "I hear he still has a real knack\n"
	.string "for FIRE-type POKéMON.$"
#else
	.string "I heard that the GYM in CINNABAR is\n"
	.string "gone.\p"
	.string "I wonder what became of BLAINE, the\n"
	.string "GYM LEADER.$"
#endif

ViridianCity_PokemonCenter_Text_1FBugCatcher:
	.string "My dream is to become a GYM LEADER.$"
