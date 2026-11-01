SSAnne_1F_Corridor_Frlg_MapScripts::
#if IS_WAYFARER
	map_script MAP_SCRIPT_ON_LOAD, SSAnne_1F_Corridor_EventScript_WayfarerInitialize
#endif
	.byte 0

#if IS_WAYFARER
SSAnne_1F_Corridor_EventScript_WayfarerInitialize::
	@ The gangway leads to the port the Anne is at; with no voyage (the adventure) it is Vermilion (8,9).
	special Ferry_RefreshAnneWarp
	end
#endif

SSAnne_1F_Corridor_EventScript_WorkerM::
	msgbox SSAnne_1F_Corridor_Text_LeStrongSilentType, MSGBOX_NPC
	end

SSAnne_1F_Corridor_EventScript_Sailor::
	msgbox SSAnne_1F_Corridor_Text_PassengersAreRestless, MSGBOX_NPC
	end

SSAnne_1F_Corridor_Text_LeStrongSilentType::
	.string "Bonjour!\n"
	.string "I am le waiter on this ship!\p"
	.string "I will be happy to serve you\n"
	.string "anything you please!\p"
	.string "… … …\n"
	.string "Ah! Le strong silent type!$"

SSAnne_1F_Corridor_Text_PassengersAreRestless::
	.string "The passengers are restless from\n"
	.string "this long voyage.\p"
	.string "You might be challenged by the\n"
	.string "more bored ones!$"
