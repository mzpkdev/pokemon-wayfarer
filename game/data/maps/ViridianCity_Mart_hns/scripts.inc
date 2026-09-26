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

@ FRLG ViridianCity_Mart_EventScript_ParcelScene. The Wayfarer receipt special
@ adds the Parcel exactly once; msgreceiveditem is the rest of giveitem_msg.
ViridianCity_Mart_KantoParcelScene::
	lockall
	setvar VAR_TEMP_1, 0
	textcolor NPC_TEXT_COLOR_MALE
	applymovement LOCALID_VIRIDIAN_MART_KANTO_CLERK, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox ViridianCity_Mart_Text_KantoYouCameFromPallet
	closemessage
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_0x8004, 4, ViridianCity_Mart_KantoParcelSceneAtCounter
	goto_if_ne VAR_0x8005, 7, ViridianCity_Mart_KantoParcelSceneAtCounter
	applymovement LOCALID_VIRIDIAN_MART_KANTO_CLERK, ViridianCity_Mart_Movement_KantoFacePlayer
	applymovement LOCALID_PLAYER, ViridianCity_Mart_Movement_KantoApproachCounter
	waitmovement 0
ViridianCity_Mart_KantoParcelSceneAtCounter:
	msgbox ViridianCity_Mart_Text_KantoTakeThisToProfOak
	call ViridianCity_Mart_KantoGiveParcel
	releaseall
	end

ViridianCity_Mart_KantoGiveParcel:
	specialvar VAR_RESULT, WayfarerKanto_TryReceiveParcel
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Mart_KantoParcelFull
	msgreceiveditem ViridianCity_Mart_Text_KantoReceivedOaksParcelFromClerk, ITEM_OAKS_PARCEL, 1, MUS_RG_OBTAIN_KEY_ITEM
	return

@ Not FRLG: FRLG never checks Bag space for the Parcel. The spec requires a
@ retry, which the clerk offers on the next talk.
ViridianCity_Mart_KantoParcelFull:
	textcolor NPC_TEXT_COLOR_NEUTRAL
	msgbox gText_TooBadBagIsFull
	call EventScript_RestorePrevTextColor
	return

ViridianCity_Mart_Movement_KantoApproachCounter:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_in_place_faster_left
	step_end

ViridianCity_Mart_Movement_KantoFacePlayer:
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_right
	step_end

@ FRLG ViridianCity_Mart_EventScript_Clerk. Holding the Parcel (Mart var 1) gives
@ SayHiToOakForMe; before the Parcel is received the clerk retries the grant.
ViridianCity_Mart_EventScript_Clerk::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, Cherrygrove_Pokemart_EventScript_Clerk
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, Cherrygrove_Pokemart_EventScript_Clerk
	setvar VAR_0x8004, PALLET_RECEIPT_PARCEL_RECEIVED
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_Mart_EventScript_KantoSayHiToOak
	setvar VAR_0x8004, PALLET_RECEIPT_FIRST_BATTLE
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, FALSE, Cherrygrove_Pokemart_EventScript_Clerk
	lock
	faceplayer
	msgbox ViridianCity_Mart_Text_KantoTakeThisToProfOak
	call ViridianCity_Mart_KantoGiveParcel
	release
	end

ViridianCity_Mart_EventScript_KantoSayHiToOak:
	lock
	faceplayer
	msgbox ViridianCity_Mart_Text_KantoSayHiToOakForMe
	release
	end

ViridianCity_Mart_Text_KantoYouCameFromPallet:
	.string "Hey!\n"
	.string "You came from PALLET TOWN?$"

ViridianCity_Mart_Text_KantoTakeThisToProfOak:
	.string "You know PROF. OAK, right?\p"
	.string "His order came in.\n"
	.string "Can I get you to take it to him?$"

ViridianCity_Mart_Text_KantoReceivedOaksParcelFromClerk:
	.string "{PLAYER} received OAK'S PARCEL\n"
	.string "from the POKéMON MART clerk.$"

ViridianCity_Mart_Text_KantoSayHiToOakForMe:
	.string "Okay, thanks! Please say hi to\n"
	.string "PROF. OAK for me, too.$"

ViridianMart_Text_KantoShopDoesGoodBusinessInAntidotes:
	.string "This shop does good business in\n"
	.string "ANTIDOTES, I've heard.$"

ViridianMart_Text_KantoGotToBuySomePotions:
	.string "I've got to buy some POTIONS.\p"
	.string "You never know when your POKéMON\n"
	.string "will need quick healing.$"
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
	msgbox ViridianMart_Text_KantoShopDoesGoodBusinessInAntidotes, MSGBOX_NPC
	end

ViridianCity_Mart_EventScript_KantoYoungster::
	msgbox ViridianMart_Text_KantoGotToBuySomePotions, MSGBOX_NPC
	end
#endif

ViridianCity_Mart_EventScript_Cooltrainer::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_Mart_EventScript_KantoYoungster
#endif
	msgbox ViridianMart_Text_CooltrainerM, MSGBOX_NPC
	end

ViridianMart_Text_Lass:
	.string "The GYM LEADER here is totally cool.$"

ViridianMart_Text_CooltrainerM:
	.string "Have you been to CINNABAR?\p"
	.string "It's an island way south of here.$"
