PokemonTower_6F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PokemonTower_6F_EventScript_SetEscapeWarp
	.byte 0

PokemonTower_6F_EventScript_ItemRareCandy::
	finditem ITEM_RARE_CANDY
	end

PokemonTower_6F_EventScript_Stair0::
	lockall
	warp MAP_POKEMON_TOWER_7F, 11, 15
	waitstate
	releaseall
	end

PokemonTower_6F_EventScript_Stair1::
	lockall
	warp MAP_POKEMON_TOWER_5F, 18, 11
	waitstate
	releaseall
	end


PokemonTower_6F_EventScript_ItemXAccuracy::
	finditem ITEM_X_ACCURACY
	end

PokemonTower_6F_EventScript_SetEscapeWarp::
	setvar VAR_TEMP_0, 0
	setescapewarp MAP_LAVENDER_TOWN_RADIO_STATION_HNS, 17, 5
	end

PokemonTower_6F_EventScript_MarowakGhost::
	goto_if_set FLAG_WAYFARER_TOWER_MAROWAK_CALMED, PokemonTower_6F_EventScript_GhostAlreadyCalmed
	lockall
	textcolor NPC_TEXT_COLOR_MON
	msgbox PokemonTower_6F_Text_BeGoneIntruders
	checkitem ITEM_SILPH_SCOPE
	goto_if_eq VAR_RESULT, FALSE, PokemonTower_6F_EventScript_UnidentifiedGhost
	setwildbattle SPECIES_MAROWAK, 30
	special StartMarowakBattle
	waitstate
	goto_if_eq VAR_RESULT, FALSE, PokemonTower_6F_EventScript_DefeatedMarowakGhost  @ VAR_RESULT set by CB2_EndMarowakBattle
	applymovement LOCALID_PLAYER, PokemonTower_6F_Movement_ForcePlayerUp
	waitmovement 0
	releaseall
	end

PokemonTower_6F_EventScript_GhostAlreadyCalmed::
	end

PokemonTower_6F_EventScript_UnidentifiedGhost::
	msgbox PokemonTower_6F_Text_UnidentifiedGhost
	applymovement LOCALID_PLAYER, PokemonTower_6F_Movement_ForcePlayerUp
	waitmovement 0
	releaseall
	end

PokemonTower_6F_EventScript_DefeatedMarowakGhost::
	message PokemonTower_6F_Text_GhostWasCubonesMother
	waitmessage
	waitse
	playmoncry SPECIES_MAROWAK, CRY_MODE_NORMAL
	waitbuttonpress
	waitmoncry
	msgbox PokemonTower_6F_Text_MothersSpiritWasCalmed
	setflag FLAG_WAYFARER_TOWER_MAROWAK_CALMED
	releaseall
	end

PokemonTower_6F_Movement_ForcePlayerUp::
	walk_up
	step_end

PokemonTower_6F_EventScript_Angelica::
	trainerbattle_single TRAINER_WAYFARER_TOWER_CHANNELER_ANGELICA, PokemonTower_6F_Text_AngelicaIntro, PokemonTower_6F_Text_AngelicaDefeat
	msgbox PokemonTower_6F_Text_AngelicaPostBattle, MSGBOX_AUTOCLOSE
	end

PokemonTower_6F_EventScript_Emilia::
	trainerbattle_single TRAINER_WAYFARER_TOWER_CHANNELER_EMILIA, PokemonTower_6F_Text_EmiliaIntro, PokemonTower_6F_Text_EmiliaDefeat
	msgbox PokemonTower_6F_Text_EmiliaPostBattle, MSGBOX_AUTOCLOSE
	end

PokemonTower_6F_EventScript_Jennifer::
	trainerbattle_single TRAINER_WAYFARER_TOWER_CHANNELER_JENNIFER, PokemonTower_6F_Text_JenniferIntro, PokemonTower_6F_Text_JenniferDefeat
	msgbox PokemonTower_6F_Text_JenniferPostBattle, MSGBOX_AUTOCLOSE
	end

PokemonTower_6F_Text_AngelicaIntro::
	.string "Give…me…\n"
	.string "blood…$"

PokemonTower_6F_Text_AngelicaDefeat::
	.string "Groan!$"

PokemonTower_6F_Text_AngelicaPostBattle::
	.string "I feel anemic and weak…$"

PokemonTower_6F_Text_EmiliaIntro::
	.string "Urff…\n"
	.string "Kwaah!$"

PokemonTower_6F_Text_EmiliaDefeat::
	.string "Something fell out!$"

PokemonTower_6F_Text_EmiliaPostBattle::
	.string "My hair didn't fall out!\n"
	.string "It was a malevolent spirit!$"

PokemonTower_6F_Text_JenniferIntro::
	.string "Ke…ke…ke…\n"
	.string "ke…ke…ke!$"

PokemonTower_6F_Text_JenniferDefeat::
	.string "Keee!$"

PokemonTower_6F_Text_JenniferPostBattle::
	.string "What's going on here?$"

PokemonTower_6F_Text_BeGoneIntruders::
	.string "Be gone…\n"
	.string "Intruders…$"

PokemonTower_6F_Text_UnidentifiedGhost::
	.string "The spirit won't let you pass.\p"
	.string "Without a SILPH SCOPE, you can't\n"
	.string "tell what it is…$"

PokemonTower_6F_Text_GhostWasCubonesMother::
	.string "The ghost was the restless spirit\n"
	.string "of CUBONE's mother!$"

PokemonTower_6F_Text_MothersSpiritWasCalmed::
	.string "The mother's spirit was calmed.\p"
	.string "It departed to the afterlife…$"
