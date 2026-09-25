ViridianCity_Mart_hns_MapScripts::
#if IS_WAYFARER
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_Mart_KantoOnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, ViridianCity_Mart_KantoOnFrame
#endif
	.byte 0

#if IS_WAYFARER
ViridianCity_Mart_KantoOnTransition:
	setvar VAR_TEMP_1, 0
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Mart_KantoEntryDone
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_lt VAR_RESULT, PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED, ViridianCity_Mart_KantoEntryDone
	goto_if_ge VAR_RESULT, PALLET_OPENING_PARCEL_RECEIVED, ViridianCity_Mart_KantoEntryDone
	setvar VAR_TEMP_1, 1
ViridianCity_Mart_KantoEntryDone:
	end

ViridianCity_Mart_KantoOnFrame:
	map_script_2 VAR_TEMP_1, 1, ViridianCity_Mart_KantoParcelScene
	.2byte 0

ViridianCity_Mart_KantoParcelScene::
	lockall
	setvar VAR_TEMP_1, 0
	call ViridianCity_Mart_KantoParcelHandoff
	releaseall
	end

ViridianCity_Mart_KantoParcelHandoff:
	msgbox ViridianCity_Mart_Text_KantoParcelOffer, MSGBOX_DEFAULT
	specialvar VAR_RESULT, WayfarerKanto_TryReceiveParcel
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Mart_KantoParcelFull
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	msgbox ViridianCity_Mart_Text_KantoParcelReceived, MSGBOX_DEFAULT
	waitfanfare
	msgbox ViridianCity_Mart_Text_KantoParcelReturn, MSGBOX_DEFAULT
	return

ViridianCity_Mart_KantoParcelFull:
	msgbox ViridianCity_Mart_Text_KantoParcelFull, MSGBOX_DEFAULT
	return

ViridianCity_Mart_EventScript_Clerk::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, Cherrygrove_Pokemart_EventScript_Clerk
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_lt VAR_RESULT, PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED, Cherrygrove_Pokemart_EventScript_Clerk
	goto_if_ge VAR_RESULT, PALLET_OPENING_PARCEL_RECEIVED, Cherrygrove_Pokemart_EventScript_Clerk
	lock
	faceplayer
	call ViridianCity_Mart_KantoParcelHandoff
	release
	end

ViridianCity_Mart_Text_KantoParcelOffer:
	.string "CLERK: Hey! You came from PALLET\n"
	.string "TOWN, right?\p"
	.string "PROF. OAK's order came in.\n"
	.string "Could you take it to him?$"

ViridianCity_Mart_Text_KantoParcelReceived:
	.string "{PLAYER} received OAK'S PARCEL\n"
	.string "from the MART clerk.$"

ViridianCity_Mart_Text_KantoParcelReturn:
	.string "CLERK: Please take that straight\n"
	.string "back to PROF. OAK.$"

ViridianCity_Mart_Text_KantoParcelFull:
	.string "CLERK: You can't carry the PARCEL\n"
	.string "right now. Come back when you can.$"
#else
ViridianCity_Mart_EventScript_Clerk::
	goto Cherrygrove_Pokemart_EventScript_Clerk
#endif

ViridianCity_Mart_EventScript_Lass::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_Mart_EventScript_KantoLass
#endif
	msgbox ViridianMart_Text_Lass, MSGBOX_NPC
	end

#if IS_WAYFARER
ViridianCity_Mart_EventScript_KantoLass::
	msgbox ViridianMart_Text_KantoLass, MSGBOX_NPC
	end

ViridianMart_Text_KantoLass:
	.string "This shop is stocked with\n"
	.string "supplies for new trainers.$"
#endif

ViridianCity_Mart_EventScript_Cooltrainer::
	msgbox ViridianMart_Text_CooltrainerM, MSGBOX_NPC
	end

ViridianMart_Text_Lass:
	.string "The GYM LEADER here is totally cool.$"

ViridianMart_Text_CooltrainerM:
	.string "Have you been to CINNABAR?\p"
	.string "It's an island way south of here.$"
