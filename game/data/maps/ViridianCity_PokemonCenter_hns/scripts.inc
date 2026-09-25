ViridianCity_PokemonCenter_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_PokemonCenter_OnTransition
	.byte 0

ViridianCity_PokemonCenter_OnTransition::
	setrespawn HEAL_LOCATION_VIRIDIAN_CITY_HNS
	end

ViridianCity_PokemonCenter_EventScript_CooltrainerM::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_PokemonCenter_EventScript_KantoVisitor
	goto_if_set FLAG_WAYFARER_VIRIDIAN_BLUE_INTRODUCED_COAST, ViridianCity_PokemonCenter_EventScript_CooltrainerMBlue
	goto_if_set FLAG_DEFEATED_VIRIDIAN_GYM, ViridianCity_PokemonCenter_EventScript_CooltrainerMBlue
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
	.string "BLUE runs VIRIDIAN's GYM.\p"
	.string "I hear he's looking for a new\n"
	.string "challenger.$"
#else
	.string "Where in the world is VIRIDIAN's GYM\n"
	.string "LEADER? I wanted to challenge him.$"
#endif

ViridianCity_PokemonCenter_Text_1FCooltrainerM_BlueReturned:
	.string "There are no GYM TRAINERS at the\n"
	.string "VIRIDIAN GYM.\p"
	.string "The LEADER claims his policy is to\n"
	.string "win without having any underlings.$"

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
