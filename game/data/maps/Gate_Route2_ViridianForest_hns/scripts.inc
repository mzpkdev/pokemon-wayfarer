Gate_Route2_ViridianForest_hns_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Gate_Route2_ViridianForest_WalkerPocOnLoad
	.byte 0

Gate_Route2_ViridianForest_WalkerPocOnLoad:
	setflag FLAG_TEMP_6
	end

Gate_Route2_ViridianForest_EventScript_WalkerPoc::
	lock
	faceplayer
	goto_if_eq VAR_TEMP_2, 13, Gate_Route2_ViridianForest_WalkerPocExit
	msgbox Gate_Route2_ViridianForest_Text_WalkerPocSpot, MSGBOX_DEFAULT
	goto Gate_Route2_ViridianForest_WalkerPocDone
Gate_Route2_ViridianForest_WalkerPocExit:
	msgbox Gate_Route2_ViridianForest_Text_WalkerPocExit, MSGBOX_DEFAULT
Gate_Route2_ViridianForest_WalkerPocDone:
	release
	end

Gate_Route2_ViridianForest_Text_WalkerPocSpot:
	.string "Taking a breather at the gate!$"
Gate_Route2_ViridianForest_Text_WalkerPocExit:
	.string "Heading back to ROUTE 2!$"
