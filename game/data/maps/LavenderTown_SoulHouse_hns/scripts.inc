LavenderTown_SoulHouse_hns_MapScripts::
	.byte 0

LavenderTownSoulHouse_EventScript_ClefairyMourner::
	msgbox SoulHouse_Text_ClefairyMourner, MSGBOX_NPC
	end
LavenderTownSoulHouse_EventScript_Lass::
	msgbox SoulHouse_Text_Lass, MSGBOX_NPC
	end
LavenderTownSoulHouse_EventScript_Teacher::
	goto_if_set FLAG_WAYFARER_TOWER_FUJI_RESCUED, LavenderTownSoulHouse_EventScript_TeacherAfterRescue
	msgbox SoulHouse_Text_Teacher, MSGBOX_NPC
	end
LavenderTownSoulHouse_EventScript_TeacherAfterRescue::
	msgbox SoulHouse_Text_TeacherAfterRescue, MSGBOX_NPC
	end
LavenderTownSoulHouse_EventScript_Granny::
	msgbox SoulHouse_Text_Granny, MSGBOX_NPC
	end

SoulHouse_Text_ClefairyMourner:
	.string "I came to pray for my dearly\n"
	.string "departed CLEFAIRY.\p"
	.string "Some memorials have moved here\n"
	.string "from the TOWER. Her grave is\l"
	.string "still upstairs. Sniff…$"

SoulHouse_Text_Teacher:
	.string "A CHANNELER says spirits are\n"
	.string "disturbed high in the TOWER.\p"
	.string "The SILPH SCOPE from CELADON can\n"
	.string "show what those ghosts are.\l"
	.string "MR. FUJI went to help, but he\l"
	.string "hasn't come back.$"

SoulHouse_Text_TeacherAfterRescue:
	.string "MR. FUJI is home again. Some\n"
	.string "memorials have moved here, and\l"
	.string "you can still pray upstairs.$"

SoulHouse_Text_Lass:
	.string "I came with my mom to visit\n"
	.string "POKéMON graves…$"

SoulHouse_Text_Granny:
	.string "The POKéMON that lived with me…\p"
	.string "I loved them like my\n"
	.string "grandchildren…$"
