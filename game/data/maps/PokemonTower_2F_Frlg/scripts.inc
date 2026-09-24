PokemonTower_2F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PokemonTower_2F_OnTransition
	.byte 0

PokemonTower_2F_OnTransition::
	setvar VAR_TEMP_0, 0
	setescapewarp MAP_LAVENDER_TOWN_RADIO_STATION_HNS, 17, 5
	end

PokemonTower_2F_EventScript_Stair0::
	lockall
	warp MAP_POKEMON_TOWER_3F, 4, 11
	waitstate
	releaseall
	end


PokemonTower_2F_EventScript_ReturnToLobby::
	lockall
	warp MAP_LAVENDER_TOWN_RADIO_STATION_HNS, 17, 5
	waitstate
	releaseall
	end

PokemonTower_2F_EventScript_Channeler::
	msgbox PokemonTower_2F_Text_SilphScopeCouldUnmaskGhosts, MSGBOX_NPC
	end

PokemonTower_2F_EventScript_Blue::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_TOWER_BLUE_MET, PokemonTower_2F_EventScript_BlueMet
	msgbox PokemonTower_2F_Text_BlueIntro, MSGBOX_DEFAULT
	setflag FLAG_WAYFARER_TOWER_BLUE_MET
	release
	end

PokemonTower_2F_EventScript_BlueMet::
	msgbox PokemonTower_2F_Text_BlueMet, MSGBOX_DEFAULT
	release
	end

PokemonTower_2F_Text_BlueIntro::
	.string "BLUE: I came looking for a\n"
	.string "MAROWAK. The ghosts here sure\l"
	.string "keep to themselves.\p"
	.string "A SILPH SCOPE might show what\n"
	.string "they really are.\p"
	.string "The stairs are open. See how far\n"
	.string "you get.$"

PokemonTower_2F_Text_BlueMet::
	.string "BLUE: Still here?\n"
	.string "Watch out for the ghosts above.$"

PokemonTower_2F_Text_SilphScopeCouldUnmaskGhosts::
	.string "Even we could not identify the\n"
	.string "wayward ghosts.\p"
	.string "A SILPH SCOPE might be able to\n"
	.string "unmask them.$"
