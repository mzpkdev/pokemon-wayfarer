PalletTown_RedsHouse_2F_hns_MapScripts::
	.byte 0

PalletTown_RedsHouse_2F_EventScript_NES::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_RedsHouse_2F_EventScript_KantoNES
#endif
	msgbox RedsHouse_Text_NES, MSGBOX_SIGN
	end

PalletTown_RedsHouse_2F_EventScript_PC::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_RedsHouse_2F_EventScript_KantoTurnOnPC
#endif
	msgbox RedsHouse_Text_PC, MSGBOX_SIGN
	end

PalletTown_RedsHouse_2F_EventScript_Notice::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_RedsHouse_2F_EventScript_KantoNotice
#endif
	end

#if IS_WAYFARER
PalletTown_RedsHouse_2F_EventScript_KantoNES:
	msgbox PalletTown_RedsHouse_2F_Text_PlayedWithNES, MSGBOX_SIGN
	end

PalletTown_RedsHouse_2F_EventScript_KantoNotice:
	msgbox PalletTown_RedsHouse_2F_Text_PressLRForHelp, MSGBOX_SIGN
	end

@ FRLG EventScript_PalletTown_PlayersHouse_2F_TurnOnPC on the HNS bedroom PC.
@ The HNS player-house PC effect and turn-off script match this tileset.
PalletTown_RedsHouse_2F_EventScript_KantoTurnOnPC:
	lockall
	setvar VAR_0x8004, PC_LOCATION_PLAYER_HOUSE_HNS
	special DoPCTurnOnEffect
	playse SE_PC_ON
	msgbox gText_PlayerHouseBootPC
	special BedroomPC
	waitstate
	releaseall
	end

PalletTown_RedsHouse_2F_Text_PlayedWithNES:
	.string "{PLAYER} played with the NES.\p"
	.string "…Okay!\n"
	.string "It's time to go!$"

PalletTown_RedsHouse_2F_Text_PressLRForHelp:
	.string "It's a posted notice…\p"
	.string "If you're confused, ask for HELP!\n"
	.string "Press the L or R Button!$"
#endif

RedsHouse_Text_NES:
	.string "{PLAYER} played the NES.\p"
	.string "Better get going--no time to lose!$"

RedsHouse_Text_PC:
	.string "It looks like it hasn't been used\n"
	.string "in a long time…$"
