NavelRock_Harbor_MapScripts::
	.byte 0

NavelRock_Harbor_EventScript_Sailor::
	lock
	faceplayer
	msgbox NavelRock_Harbor_Text_SailorReturn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NavelRock_Harbor_EventScript_AsYouLike
	msgbox EventTicket_Text_SailHome, MSGBOX_DEFAULT
	closemessage
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	delay 30
	hideobjectat LOCALID_NAVEL_ROCK_SAILOR, MAP_NAVEL_ROCK_HARBOR
	setvar VAR_0x8004, LOCALID_NAVEL_ROCK_SS_TIDAL
	call Common_EventScript_FerryDepartIsland
#if IS_WAYFARER
	warp MAP_VERMILION_CITY_PORT_INSIDE_HNS, 8, 9
#else
	warp MAP_LILYCOVE_CITY_HARBOR, 8, 11
#endif
	waitstate
	release
	end

NavelRock_Harbor_EventScript_AsYouLike::
	msgbox EventTicket_Text_AsYouLike, MSGBOX_DEFAULT
	release
	end

