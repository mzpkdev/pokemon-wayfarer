LavenderTown_House1_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, LavenderTown_House1_OnTransition
	.byte 0

LavenderTown_House1_OnTransition::
	goto_if_unset FLAG_WAYFARER_TOWER_FUJI_RESCUED, LavenderTown_House1_HideFuji
	clearflag FLAG_WAYFARER_TOWER_HIDE_HOUSE1_FUJI
	end

LavenderTown_House1_HideFuji::
	setflag FLAG_WAYFARER_TOWER_HIDE_HOUSE1_FUJI
	end

LavenderTown_House1_EventScript_Girl::
	msgbox LavenderTown_House1_Text_Girl, MSGBOX_NPC
	end

LavenderTown_House1_EventScript_Guy::
	goto_if_set FLAG_WAYFARER_TOWER_FUJI_RESCUED, LavenderTown_House1_EventScript_GuyAfterRescue
	msgbox LavenderTown_House1_Text_GuyMissing, MSGBOX_NPC
	end

LavenderTown_House1_EventScript_GuyAfterRescue::
	msgbox LavenderTown_House1_Text_GuyReturned, MSGBOX_NPC
	end

LavenderTown_House1_EventScript_MrFuji::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_TOWER_FLUTE_CLAIMED, LavenderTown_House1_EventScript_FujiAfterGift
	checkitem ITEM_POKE_FLUTE
	goto_if_eq VAR_RESULT, TRUE, LavenderTown_House1_EventScript_FujiAlreadyOwned
	msgbox LavenderTown_House1_Text_FujiOffersFlute, MSGBOX_DEFAULT
	checkitemspace ITEM_POKE_FLUTE
	goto_if_eq VAR_RESULT, FALSE, LavenderTown_House1_EventScript_FujiBagFull
	additem ITEM_POKE_FLUTE
	goto_if_eq VAR_RESULT, FALSE, LavenderTown_House1_EventScript_FujiBagFull
	setflag FLAG_WAYFARER_TOWER_FLUTE_CLAIMED
	msgreceiveditem LavenderTown_House1_Text_ReceivedPokeFlute, ITEM_POKE_FLUTE, 1, MUS_RG_OBTAIN_KEY_ITEM
	msgbox LavenderTown_House1_Text_FujiExplainsFlute, MSGBOX_DEFAULT
	release
	end

LavenderTown_House1_EventScript_FujiAlreadyOwned::
	setflag FLAG_WAYFARER_TOWER_FLUTE_CLAIMED
	msgbox LavenderTown_House1_Text_FujiAlreadyOwned, MSGBOX_DEFAULT
	release
	end

LavenderTown_House1_EventScript_FujiBagFull::
	msgbox LavenderTown_House1_Text_FujiBagFull, MSGBOX_DEFAULT
	release
	end

LavenderTown_House1_EventScript_FujiAfterGift::
	msgbox LavenderTown_House1_Text_FujiAfterGift, MSGBOX_DEFAULT
	release
	end

LavenderTown_House1_EventScript_Marowak::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MAROWAK, CRY_MODE_NORMAL
	msgbox LavenderTown_House1_Text_Marowak, MSGBOX_DEFAULT
	waitmoncry
	msgbox LavenderTown_House1_Text_Marowak2, MSGBOX_DEFAULT
	closemessage
	release
	end

LavenderTown_House1_Text_Girl:
	.string "Can you believe it?\p"
	.string "Some people get bored with taking\n"
	.string "care of POKéMON.\p"
	.string "MR. FUJI takes in such homeless\n"
	.string "POKéMON and takes care of them.$"
LavenderTown_House1_Text_GuyMissing:
	.string "MR. FUJI went up to the memorial\n"
	.string "floors of POKéMON TOWER and\l"
	.string "hasn't come home.\p"
	.string "The radio attendant can let you\n"
	.string "upstairs.$"
LavenderTown_House1_Text_GuyReturned:
	.string "MR. FUJI is home safely. He was\n"
	.string "very worried about the spirits\l"
	.string "in the TOWER.$"
LavenderTown_House1_Text_FujiOffersFlute:
	.string "MR. FUJI: Thank you for helping\n"
	.string "me and the restless spirit.\p"
	.string "Your kindness to POKéMON means\n"
	.string "a great deal to me. Please take\l"
	.string "this POKé FLUTE.$"
LavenderTown_House1_Text_ReceivedPokeFlute:
	.string "{PLAYER} received a POKé FLUTE\n"
	.string "from MR. FUJI.$"
LavenderTown_House1_Text_FujiExplainsFlute:
	.string "Its music wakes sleepy POKéMON.\n"
	.string "Try it when one blocks your way.$"
LavenderTown_House1_Text_FujiAlreadyOwned:
	.string "MR. FUJI: You already have a\n"
	.string "POKé FLUTE. Take care of it.$"
LavenderTown_House1_Text_FujiBagFull:
	.string "MR. FUJI: Make some room in your\n"
	.string "BAG, then come back for the\l"
	.string "POKé FLUTE.$"
LavenderTown_House1_Text_FujiAfterGift:
	.string "MR. FUJI: Has my POKé FLUTE\n"
	.string "helped you? The memorial floors\l"
	.string "are open whenever you visit.$"
LavenderTown_House1_Text_Marowak:
	.string ".............................$"

LavenderTown_House1_Text_Marowak2:
	.string "This shelter MAROWAK is quiet,\n"
	.string "though it appears quite happy.$"
