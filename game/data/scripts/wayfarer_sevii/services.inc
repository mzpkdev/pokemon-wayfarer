@ Source-free ordinary services for the Wayfarer Sevii overlay.
@
@ The labels at the end of this file intentionally match only reviewed source
@ event labels.  They are aliases, not imported FRLG script bodies.

WayfarerSevii_EventScript_Nurse::
	lock
	faceplayer
	msgbox gText_WouldYouLikeToRestYourPkmn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, WayfarerSevii_EventScript_NurseHeal
	msgbox gText_WeHopeToSeeYouAgain, MSGBOX_DEFAULT
	release
	end

WayfarerSevii_EventScript_NurseHeal::
	incrementgamestat GAME_STAT_USED_POKECENTER
	message gText_IllTakeYourPkmn
	waitmessage
	hidefollower 0
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	dofieldeffect FLDEFF_POKECENTER_HEAL
	waitfieldeffect FLDEFF_POKECENTER_HEAL
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	special HealPlayerParty
	message gText_RestoredPkmnToFullHealth
	waitmessage
	applymovement VAR_LAST_TALKED, Movement_PkmnCenterNurse_Bow
	waitmovement 0
	message gText_WeHopeToSeeYouAgain
	closemessage
	callnative UpdateFollowingPokemon
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceDown
	waitmovement 0
	release
	end

WayfarerSevii_EventScript_PC::
	goto EventScript_PC

WayfarerSevii_EventScript_ThreeIslandMart::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart WayfarerSevii_ThreeIslandMartItems
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

WayfarerSevii_EventScript_FourIslandMart::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart WayfarerSevii_FourIslandMartItems
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

WayfarerSevii_EventScript_SixIslandMart::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart WayfarerSevii_SixIslandMartItems
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

WayfarerSevii_EventScript_SevenIslandMart::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart WayfarerSevii_SevenIslandMartItems
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
WayfarerSevii_ThreeIslandMartItems::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_NONE

WayfarerSevii_FourIslandMartItems::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_MAX_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_NONE

WayfarerSevii_SixIslandMartItems::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_MAX_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_DREAM_MAIL
	.2byte ITEM_NONE

WayfarerSevii_SevenIslandMartItems::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_MAX_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_NONE

@ The Day Care uses Wayfarer's ordinary two-slot Day Care state.  Egg pickup,
@ gifts, and every FRLG campaign branch are deliberately absent.
WayfarerSevii_EventScript_FourIslandDaycareWoman::
	lock
	faceplayer
	specialvar VAR_RESULT, GetDaycareState
	goto_if_eq VAR_RESULT, DAYCARE_EGG_WAITING, WayfarerSevii_EventScript_DaycareRetrieve
	goto_if_eq VAR_RESULT, DAYCARE_ONE_MON, WayfarerSevii_EventScript_DaycareOneMon
	goto_if_eq VAR_RESULT, DAYCARE_TWO_MONS, WayfarerSevii_EventScript_DaycareRetrieve
	msgbox Route117_PokemonDayCare_Text_WouldYouLikeUsToRaiseAMon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, WayfarerSevii_EventScript_DaycareDeposit
	goto WayfarerSevii_EventScript_DaycareComeAgain

WayfarerSevii_EventScript_DaycareOneMon::
	msgbox Route117_PokemonDayCare_Text_WeCanRaiseOneMore, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, WayfarerSevii_EventScript_DaycareDeposit
	goto WayfarerSevii_EventScript_DaycareRetrieve

WayfarerSevii_EventScript_DaycareDeposit::
	specialvar VAR_RESULT, CountPartyNonEggMons
	goto_if_eq VAR_RESULT, 1, WayfarerSevii_EventScript_DaycareOnlyOneMon
	msgbox Route117_PokemonDayCare_Text_WhichMonShouldWeRaise, MSGBOX_DEFAULT
	chooseboxmon SELECT_PC_MON_DAYCARE
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, WayfarerSevii_EventScript_DaycareComeAgain
	specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
	goto_if_eq VAR_RESULT, 0, WayfarerSevii_EventScript_DaycareOnlyOneAliveMon
	specialvar VAR_0x8005, GetSelectedMonNicknameAndSpecies
	waitse
	playmoncry VAR_0x8005, CRY_MODE_NORMAL
	msgbox Route117_PokemonDayCare_Text_WellRaiseYourMon, MSGBOX_DEFAULT
	waitmoncry
	special StoreSelectedPokemonInDaycare
	callnative UpdateFollowingPokemon
	incrementgamestat GAME_STAT_USED_DAYCARE
	goto WayfarerSevii_EventScript_DaycareComeAgain

WayfarerSevii_EventScript_DaycareRetrieve::
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, PARTY_SIZE, WayfarerSevii_EventScript_DaycareNoRoom
	specialvar VAR_RESULT, GetDaycareState
	setvar VAR_0x8004, 0
	goto_if_eq VAR_RESULT, DAYCARE_ONE_MON, WayfarerSevii_EventScript_DaycareCostPrompt
	special ShowDaycareLevelMenu
	waitstate
	copyvar VAR_0x8004, VAR_RESULT
	goto_if_eq VAR_RESULT, DAYCARE_EXITED_LEVEL_MENU, WayfarerSevii_EventScript_DaycareComeAgain

WayfarerSevii_EventScript_DaycareCostPrompt::
	special GetDaycareCost
	msgbox Route117_PokemonDayCare_Text_ItWillCostX, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, WayfarerSevii_EventScript_DaycareCheckMoney
	goto WayfarerSevii_EventScript_DaycareComeAgain

WayfarerSevii_EventScript_DaycareCheckMoney::
	specialvar VAR_RESULT, IsEnoughForCostInVar0x8005
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_EventScript_DaycareReturnMon
	msgbox Route117_PokemonDayCare_Text_NotEnoughMoney, MSGBOX_DEFAULT
	goto WayfarerSevii_EventScript_DaycareComeAgain

WayfarerSevii_EventScript_DaycareReturnMon::
	specialvar VAR_RESULT, TakePokemonFromDaycare
	special SubtractMoneyFromVar0x8005
	playse SE_SHOP
	msgbox Route117_PokemonDayCare_Text_HeresYourMon, MSGBOX_DEFAULT
	waitse
	playmoncry VAR_RESULT, CRY_MODE_NORMAL
	msgbox Route117_PokemonDayCare_Text_TookBackMon, MSGBOX_DEFAULT
	waitmoncry
	goto WayfarerSevii_EventScript_DaycareComeAgain

WayfarerSevii_EventScript_DaycareOnlyOneMon::
	msgbox Route117_PokemonDayCare_Text_YouHaveJustOneMon, MSGBOX_DEFAULT
	goto WayfarerSevii_EventScript_DaycareComeAgain

WayfarerSevii_EventScript_DaycareOnlyOneAliveMon::
	msgbox Route117_PokemonDayCare_Text_WhatWillYouBattleWith, MSGBOX_DEFAULT
	goto WayfarerSevii_EventScript_DaycareComeAgain

WayfarerSevii_EventScript_DaycareNoRoom::
	msgbox Route117_PokemonDayCare_Text_YourTeamIsFull, MSGBOX_DEFAULT

WayfarerSevii_EventScript_DaycareComeAgain::
	msgbox Route117_PokemonDayCare_Text_ComeAgain, MSGBOX_DEFAULT
	release
	end

@ One Island is the only numbered-island Center with source PC event tiles.
WayfarerSevii_EventScript_OneIslandHarborSailor::
	lock
	faceplayer
	setvar VAR_0x8004, SEAGALLOP_ONE_ISLAND
	goto EventScript_SeviiDestinationsPage1

WayfarerSevii_EventScript_TwoIslandHarborSailor::
	lock
	faceplayer
	setvar VAR_0x8004, SEAGALLOP_TWO_ISLAND
	goto EventScript_SeviiDestinationsPage1

WayfarerSevii_EventScript_ThreeIslandHarborSailor::
	lock
	faceplayer
	setvar VAR_0x8004, SEAGALLOP_THREE_ISLAND
	goto EventScript_SeviiDestinationsPage1

WayfarerSevii_EventScript_FourIslandHarborSailor::
	lock
	faceplayer
	setvar VAR_0x8004, SEAGALLOP_FOUR_ISLAND
	goto EventScript_SeviiDestinationsPage1

WayfarerSevii_EventScript_FiveIslandHarborSailor::
	lock
	faceplayer
	setvar VAR_0x8004, SEAGALLOP_FIVE_ISLAND
	goto EventScript_SeviiDestinationsPage1

WayfarerSevii_EventScript_SixIslandHarborSailor::
	lock
	faceplayer
	setvar VAR_0x8004, SEAGALLOP_SIX_ISLAND
	goto EventScript_SeviiDestinationsPage1

WayfarerSevii_EventScript_SevenIslandHarborSailor::
	lock
	faceplayer
	setvar VAR_0x8004, SEAGALLOP_SEVEN_ISLAND
	goto EventScript_SeviiDestinationsPage1
