CinnabarIsland_Gym_Port_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, CinnabarIsland_Gym_Port_OnLoad
	.byte 0

CinnabarIsland_Gym_Port_OnLoad::
	call CinnabarIsland_Gym_Port_OpenDoor1
	call CinnabarIsland_Gym_Port_OpenDoor2
	call CinnabarIsland_Gym_Port_OpenDoor3
	call CinnabarIsland_Gym_Port_OpenDoor4
	call CinnabarIsland_Gym_Port_OpenDoor5
	call CinnabarIsland_Gym_Port_OpenDoor6
	end

CinnabarIsland_Gym_Port_OpenDoor1::
	setmetatile 26,  8, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile 27,  8, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile 26,  9, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile 27,  9, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile 26, 10, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 27, 10, METATILE_CinnabarGym_Floor, 0
	setmetatile 28, 10, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return

CinnabarIsland_Gym_Port_OpenDoor2::
	setmetatile 17,  8, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile 18,  8, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile 17,  9, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile 18,  9, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile 17, 10, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 18, 10, METATILE_CinnabarGym_Floor, 0
	setmetatile 19, 10, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return

CinnabarIsland_Gym_Port_OpenDoor3::
	setmetatile 17, 15, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile 18, 15, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile 17, 16, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile 18, 16, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile 17, 17, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 18, 17, METATILE_CinnabarGym_Floor, 0
	setmetatile 19, 17, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return

CinnabarIsland_Gym_Port_OpenDoor4::
	setmetatile 11, 21, METATILE_CinnabarGym_Wall_RetractedBarrier, 1
	setmetatile 11, 22, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 11, 23, METATILE_CinnabarGym_Floor, 0
	return

CinnabarIsland_Gym_Port_OpenDoor5::
	setmetatile  5, 16, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile  6, 16, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile  5, 17, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile  6, 17, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile  5, 18, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile  6, 18, METATILE_CinnabarGym_Floor, 0
	setmetatile  7, 18, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return

CinnabarIsland_Gym_Port_OpenDoor6::
	setmetatile  5,  8, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile  6,  8, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile  5,  9, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile  6,  9, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile  5, 10, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile  6, 10, METATILE_CinnabarGym_Floor, 0
	setmetatile  7, 10, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return

PokemonMansion_1F_Port_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PokemonMansion_1F_Port_OnLoad
	.byte 0

PokemonMansion_1F_Port_OnLoad::
	setmetatile 22, 10, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 23, 10, METATILE_PokemonMansion_Floor, 0
	setmetatile 24, 10, METATILE_PokemonMansion_Floor, 0
	setmetatile 22, 11, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 23, 11, METATILE_PokemonMansion_Floor, 0
	setmetatile 24, 11, METATILE_PokemonMansion_Floor, 0
	setmetatile 27, 25, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 28, 25, METATILE_PokemonMansion_Floor, 0
	setmetatile 29, 25, METATILE_PokemonMansion_Floor, 0
	setmetatile 27, 26, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 28, 26, METATILE_PokemonMansion_Floor, 0
	setmetatile 29, 26, METATILE_PokemonMansion_Floor, 0
	setmetatile 32, 25, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 33, 25, METATILE_PokemonMansion_Floor, 0
	setmetatile 34, 25, METATILE_PokemonMansion_Floor, 0
	setmetatile 32, 26, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 33, 26, METATILE_PokemonMansion_Floor, 0
	setmetatile 34, 26, METATILE_PokemonMansion_Floor, 0
	setmetatile 31, 18, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 32, 18, METATILE_PokemonMansion_Floor, 0
	setmetatile 33, 18, METATILE_PokemonMansion_Floor, 0
	setmetatile 31, 19, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 32, 19, METATILE_PokemonMansion_Floor, 0
	setmetatile 33, 19, METATILE_PokemonMansion_Floor, 0
	end

PokemonMansion_2F_Port_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PokemonMansion_2F_Port_OnLoad
	.byte 0

PokemonMansion_2F_Port_OnLoad::
	setmetatile 24, 14, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 25, 14, METATILE_PokemonMansion_Floor, 0
	setmetatile 26, 14, METATILE_PokemonMansion_Floor, 0
	setmetatile 24, 15, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 25, 15, METATILE_PokemonMansion_Floor, 0
	setmetatile 26, 15, METATILE_PokemonMansion_Floor, 0
	setmetatile 10, 30, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 10, 31, METATILE_PokemonMansion_Floor, 0
	setmetatile 12,  6, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 12,  7, METATILE_PokemonMansion_Floor, 0
	end

PokemonMansion_3F_Port_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PokemonMansion_3F_Port_OnLoad
	.byte 0

PokemonMansion_3F_Port_OnLoad::
	setmetatile 17, 11, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 18, 11, METATILE_PokemonMansion_Floor, 0
	setmetatile 19, 11, METATILE_PokemonMansion_Floor, 0
	setmetatile 17, 12, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 18, 12, METATILE_PokemonMansion_Floor, 0
	setmetatile 19, 12, METATILE_PokemonMansion_Floor, 0
	setmetatile 21,  6, METATILE_PokemonMansion_Floor_ShadeFull, 0
	setmetatile 21,  7, METATILE_PokemonMansion_Floor, 0
	end

PokemonMansion_B1F_Port_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PokemonMansion_B1F_Port_OnLoad
	.byte 0

PokemonMansion_B1F_Port_OnLoad::
	setmetatile 33, 20, METATILE_PokemonMansion_BasementFloor_ShadeFull, 0
	setmetatile 34, 20, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 35, 20, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 33, 21, METATILE_PokemonMansion_BasementFloor_ShadeFull, 0
	setmetatile 34, 21, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 35, 21, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 16, 28, METATILE_PokemonMansion_BasementFloor_ShadeFull, 0
	setmetatile 16, 29, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 12, 10, METATILE_PokemonMansion_BasementFloor_ShadeFull, 0
	setmetatile 12, 11, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 20, 22, METATILE_PokemonMansion_BasementFloor_ShadeFull, 0
	setmetatile 21, 22, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 22, 22, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 20, 23, METATILE_PokemonMansion_BasementFloor_ShadeFull, 0
	setmetatile 21, 23, METATILE_PokemonMansion_BasementFloor, 0
	setmetatile 22, 23, METATILE_PokemonMansion_BasementFloor, 0
	end
