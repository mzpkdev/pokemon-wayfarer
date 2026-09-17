CinnabarIsland_PokemonCenter_1F_Port_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CinnabarIsland_PokemonCenter_1F_Port_OnTransition
	.byte 0

CinnabarIsland_PokemonCenter_1F_Port_OnTransition::
	setrespawn HEAL_LOCATION_CINNABAR_ISLAND_HNS
	end

CinnabarIsland_PokemonCenter_1F_Port_EventScript_Nurse::
	lock
	faceplayer
	message CinnabarIsland_PokemonCenter_1F_Port_Text_Nurse
	waitmessage
	special HealPlayerParty
	msgbox CinnabarIsland_PokemonCenter_1F_Port_Text_NurseDone, MSGBOX_DEFAULT
	release
	end

CinnabarIsland_PokemonCenter_1F_Port_Text_Nurse:
	.string "Welcome to our POKéMON CENTER!\p"
	.string "Your POKéMON have been restored\n"
	.string "to full health.$"

CinnabarIsland_PokemonCenter_1F_Port_Text_NurseDone:
	.string "We hope to see you again!$"

CinnabarIsland_PokemonCenter_2F_Port_MapScripts::
	.byte 0

CinnabarIsland_Mart_Port_MapScripts::
	.byte 0

CinnabarIsland_Mart_Port_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart CinnabarIsland_Mart_Port_Items
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
CinnabarIsland_Mart_Port_Items::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_NONE

CinnabarIsland_PokemonLab_Entrance_Port_MapScripts::
	.byte 0

CinnabarIsland_PokemonLab_Lounge_Port_MapScripts::
	.byte 0

CinnabarIsland_PokemonLab_ResearchRoom_Port_MapScripts::
	.byte 0

CinnabarIsland_PokemonLab_ExperimentRoom_Port_MapScripts::
	.byte 0
