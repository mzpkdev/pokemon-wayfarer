CinnabarIsland_SeamPoc_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CinnabarIsland_SeamPoc_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, SetTimeEncounters
	.byte 0

CinnabarIsland_SeamPoc_OnTransition::
#if IS_WAYFARER
	setflag FLAG_VISITED_CINNABAR_ISLAND
#endif
	callnative SetTimeBasedEncounters
	end
