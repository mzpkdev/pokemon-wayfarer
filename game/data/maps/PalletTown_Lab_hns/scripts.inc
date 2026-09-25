PalletTown_Lab_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_Lab_KantoOnTransition
	map_script MAP_SCRIPT_ON_LOAD, PalletTown_Lab_KantoOnLoad
	.byte 0

#if IS_WAYFARER
PalletTown_Lab_KantoOnTransition:
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_KantoTransitionDone
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_ne VAR_RESULT, PALLET_OPENING_OAK_INTERCEPTION_AVAILABLE, PalletTown_Lab_KantoTransitionDone
	special WayfarerKanto_CommitLabStarterChoice
PalletTown_Lab_KantoTransitionDone:
	end

PalletTown_Lab_KantoOnLoad:
	clearflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_KantoLoadDone
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_lt VAR_RESULT, PALLET_OPENING_LAB_STARTER_CHOICE, PalletTown_Lab_KantoHideOak
	goto_if_ge VAR_RESULT, PALLET_OPENING_STARTER_RECEIVED, PalletTown_Lab_KantoLoadBlue
	specialvar VAR_RESULT, WayfarerKanto_GetStarterSlot
	goto_if_ne VAR_RESULT, KANTO_STARTER_SLOT_NONE, PalletTown_Lab_KantoLoadBlue
	clearflag FLAG_TEMP_3
PalletTown_Lab_KantoLoadBlue:
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_lt VAR_RESULT, PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED, PalletTown_Lab_KantoShowBlue
	setvar VAR_0x8004, PALLET_RECEIPT_BLUE_ARRIVED
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_KantoLoadDone
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_KantoLoadDone
PalletTown_Lab_KantoShowBlue:
	clearflag FLAG_TEMP_2
	end

PalletTown_Lab_KantoHideOak:
	setflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2 @ Blue is waiting even before Oak's escort.
PalletTown_Lab_KantoLoadDone:
	end
#else
PalletTown_Lab_KantoOnTransition:
	end
PalletTown_Lab_KantoOnLoad:
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	end
#endif

PalletTown_Lab_EventScript_Oak::
	lock
	faceplayer
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoOak
#endif
	goto_if_eq VAR_PALLETTOWN_LABSTATE, 1, PalletTown_Lab_EventScript_OakBadgeCheck
	goto_if_eq VAR_PALLETTOWN_LABSTATE, 2, PalletTown_Lab_EventScript_OakDex
	msgbox PalletTown_Lab_Text_OakWelcomeKanto, MSGBOX_DEFAULT
	msgbox PalletTown_Lab_Text_OakLabGoodbye, MSGBOX_DEFAULT
	setvar VAR_PALLETTOWN_LABSTATE, 1
	goto_if_ge VAR_NUM_BADGES, 16, PalletTown_Lab_EventScript_OakBadgeCheck
	closemessage
	release
	end

#if IS_WAYFARER
PalletTown_Lab_EventScript_KantoOak::
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_ge VAR_RESULT, PALLET_OPENING_PARCEL_RECEIVED, PalletTown_Lab_EventScript_KantoParcel
	goto_if_ge VAR_RESULT, PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED, PalletTown_Lab_EventScript_KantoAfterBattle
	goto_if_ge VAR_RESULT, PALLET_OPENING_STARTER_RECEIVED, PalletTown_Lab_EventScript_KantoAfterStarter
	specialvar VAR_RESULT, WayfarerKanto_GetStarterSlot
	goto_if_ne VAR_RESULT, KANTO_STARTER_SLOT_NONE, PalletTown_Lab_EventScript_KantoRetryStarter
	msgbox PalletTown_Lab_Text_KantoChooseStarter, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoAfterStarter::
	msgbox PalletTown_Lab_Text_KantoAfterStarter, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoAfterBattle::
	msgbox PalletTown_Lab_Text_KantoAfterBattle, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoParcel::
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoComplete
	setvar VAR_0x8004, PALLET_RECEIPT_PARCEL_ACCEPTED
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoBlueArrival
	msgbox PalletTown_Lab_Text_KantoParcelWelcome, MSGBOX_DEFAULT
	specialvar VAR_RESULT, WayfarerKanto_TryAcceptParcel
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoParcelMissing
	msgbox PalletTown_Lab_Text_KantoParcelAccepted, MSGBOX_DEFAULT

PalletTown_Lab_EventScript_KantoBlueArrival:
	setvar VAR_0x8004, PALLET_RECEIPT_BLUE_ARRIVED
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoDex
	clearflag FLAG_TEMP_2
	setobjectxyperm LOCALID_PALLET_KANTO_BLUE, 13, 19
	addobject LOCALID_PALLET_KANTO_BLUE
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_BlueArrives
	waitmovement 0
	msgbox PalletTown_Lab_Text_KantoBlueArrives, MSGBOX_DEFAULT
	special WayfarerKanto_CommitBlueArrival

PalletTown_Lab_EventScript_KantoDex:
	setvar VAR_0x8004, PALLET_RECEIPT_POKEDEX
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoBalls
	msgbox PalletTown_Lab_Text_KantoDexRequest, MSGBOX_DEFAULT
	specialvar VAR_RESULT, WayfarerKanto_TryGrantPokedex
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoRewardRetry
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	msgbox PalletTown_Lab_Text_KantoDexReceived, MSGBOX_DEFAULT
	waitfanfare

PalletTown_Lab_EventScript_KantoBalls:
	setvar VAR_0x8004, PALLET_RECEIPT_POKE_BALLS
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoPresentation
	msgbox PalletTown_Lab_Text_KantoBallOffer, MSGBOX_DEFAULT
	specialvar VAR_RESULT, WayfarerKanto_TryGrantPokeBalls
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoBagFull
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	msgbox PalletTown_Lab_Text_KantoBallsReceived, MSGBOX_DEFAULT
	waitfanfare

PalletTown_Lab_EventScript_KantoPresentation:
	setvar VAR_0x8004, PALLET_RECEIPT_PRESENTATION_DONE
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoFinish
	msgbox PalletTown_Lab_Text_KantoCatchExplanation, MSGBOX_DEFAULT
	msgbox PalletTown_Lab_Text_KantoBlueTownMap, MSGBOX_DEFAULT
	removeobject LOCALID_PALLET_KANTO_BLUE
	setflag FLAG_TEMP_2
	special WayfarerKanto_CommitPresentationDone

PalletTown_Lab_EventScript_KantoFinish:
	specialvar VAR_RESULT, WayfarerKanto_CompleteOpening
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoRewardRetry
	msgbox PalletTown_Lab_Text_KantoComplete, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoComplete:
	msgbox PalletTown_Lab_Text_KantoComplete, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoParcelMissing:
	msgbox PalletTown_Lab_Text_KantoParcelMissing, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoBagFull:
	msgbox PalletTown_Lab_Text_KantoBagFull, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoRewardRetry:
	msgbox PalletTown_Lab_Text_KantoRewardRetry, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_Movement_BlueArrives:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

PalletTown_Lab_EventScript_KantoBlue::
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_lt VAR_RESULT, PALLET_OPENING_LAB_STARTER_CHOICE, PalletTown_Lab_EventScript_KantoBlueWaiting
	goto_if_lt VAR_RESULT, PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED, PalletTown_Lab_EventScript_KantoBlueReady
	msgbox PalletTown_Lab_Text_KantoBlueDex, MSGBOX_NPC
	end

PalletTown_Lab_EventScript_KantoBlueWaiting:
	msgbox PalletTown_Lab_Text_KantoBlueWaiting, MSGBOX_NPC
	end

PalletTown_Lab_EventScript_KantoBlueReady:
	msgbox PalletTown_Lab_Text_KantoBlueReady, MSGBOX_NPC
	end

PalletTown_Lab_EventScript_KantoBulbasaurBall::
	setvar VAR_0x8004, KANTO_STARTER_SLOT_BULBASAUR
	goto PalletTown_Lab_EventScript_KantoBall

PalletTown_Lab_EventScript_KantoCharmanderBall::
	setvar VAR_0x8004, KANTO_STARTER_SLOT_CHARMANDER
	goto PalletTown_Lab_EventScript_KantoBall

PalletTown_Lab_EventScript_KantoSquirtleBall::
	setvar VAR_0x8004, KANTO_STARTER_SLOT_SQUIRTLE
	goto PalletTown_Lab_EventScript_KantoBall

PalletTown_Lab_EventScript_KantoBall:
	lock
	faceplayer
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_ne VAR_RESULT, PALLET_OPENING_LAB_STARTER_CHOICE, PalletTown_Lab_EventScript_KantoBallUnavailable
	specialvar VAR_RESULT, WayfarerKanto_GetStarterSlot
	goto_if_ne VAR_RESULT, KANTO_STARTER_SLOT_NONE, PalletTown_Lab_EventScript_KantoRetryStarter
	specialvar VAR_0x8005, WayfarerKanto_GetSelectedStarterSpecies
	bufferspeciesname STR_VAR_1, VAR_0x8005
	special WayfarerKanto_BufferSelectedStarterType
	showmonpic VAR_0x8005, 10, 3
	msgbox PalletTown_Lab_Text_KantoStarterPreview, MSGBOX_YESNO

PalletTown_Lab_EventScript_KantoPreviewResult:
	hidemonpic
	goto_if_eq VAR_RESULT, NO, PalletTown_Lab_EventScript_KantoCancelStarter
	specialvar VAR_RESULT, WayfarerKanto_CommitStarterSlot
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoBallUnavailable
PalletTown_Lab_EventScript_KantoRetryStarter:
	specialvar VAR_RESULT, WayfarerKanto_TryGiveStarter
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoStarterFull
	setflag FLAG_TEMP_3
	removeobject LOCALID_PALLET_KANTO_BULBASAUR_BALL
	removeobject LOCALID_PALLET_KANTO_CHARMANDER_BALL
	removeobject LOCALID_PALLET_KANTO_SQUIRTLE_BALL
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	msgbox PalletTown_Lab_Text_KantoStarterReceived, MSGBOX_DEFAULT
	waitfanfare
	msgbox PalletTown_Lab_Text_KantoBlueTakesCounter, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoCancelStarter:
	msgbox PalletTown_Lab_Text_KantoChooseAnother, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoStarterFull:
	setflag FLAG_TEMP_3
	removeobject LOCALID_PALLET_KANTO_BULBASAUR_BALL
	removeobject LOCALID_PALLET_KANTO_CHARMANDER_BALL
	removeobject LOCALID_PALLET_KANTO_SQUIRTLE_BALL
	msgbox PalletTown_Lab_Text_KantoStarterFull, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoBallUnavailable:
	msgbox PalletTown_Lab_Text_KantoBallUnavailable, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_KantoExitBattle::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoNoExitBattle
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_ne VAR_RESULT, PALLET_OPENING_STARTER_RECEIVED, PalletTown_Lab_EventScript_KantoNoExitBattle
	setvar VAR_0x8004, PALLET_RECEIPT_STARTER
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, FALSE, PalletTown_Lab_EventScript_KantoNoExitBattle
	specialvar VAR_RESULT, CountPartyAliveNonEggMons
	goto_if_eq VAR_RESULT, 0, PalletTown_Lab_EventScript_KantoNoUsableMon
	lockall
	setobjectxyperm LOCALID_PALLET_KANTO_BLUE, 13, 16
	applymovement LOCALID_PALLET_KANTO_BLUE, PalletTown_Lab_Movement_BlueChallenges
	waitmovement 0
	msgbox PalletTown_Lab_Text_KantoBlueChallenge, MSGBOX_DEFAULT
	specialvar VAR_RESULT, WayfarerKanto_GetCounterSlot
	goto_if_eq VAR_RESULT, KANTO_STARTER_SLOT_BULBASAUR, PalletTown_Lab_EventScript_KantoBattleBulbasaur
	goto_if_eq VAR_RESULT, KANTO_STARTER_SLOT_CHARMANDER, PalletTown_Lab_EventScript_KantoBattleCharmander
	goto PalletTown_Lab_EventScript_KantoBattleSquirtle

PalletTown_Lab_EventScript_KantoBattleBulbasaur:
	trainerbattle_earlyrival TRAINER_WAYFARER_KANTO_BLUE_BULBASAUR, RIVAL_BATTLE_TUTORIAL, PalletTown_Lab_Text_KantoBlueWon, PalletTown_Lab_Text_KantoBlueLost
	goto PalletTown_Lab_EventScript_KantoBattleEnd

PalletTown_Lab_EventScript_KantoBattleCharmander:
	trainerbattle_earlyrival TRAINER_WAYFARER_KANTO_BLUE_CHARMANDER, RIVAL_BATTLE_TUTORIAL, PalletTown_Lab_Text_KantoBlueWon, PalletTown_Lab_Text_KantoBlueLost
	goto PalletTown_Lab_EventScript_KantoBattleEnd

PalletTown_Lab_EventScript_KantoBattleSquirtle:
	trainerbattle_earlyrival TRAINER_WAYFARER_KANTO_BLUE_SQUIRTLE, RIVAL_BATTLE_TUTORIAL, PalletTown_Lab_Text_KantoBlueWon, PalletTown_Lab_Text_KantoBlueLost
	goto PalletTown_Lab_EventScript_KantoBattleEnd

PalletTown_Lab_EventScript_KantoBattleEnd:
	special WayfarerKanto_ResolveFirstBattle
	msgbox PalletTown_Lab_Text_KantoBlueLeaves, MSGBOX_DEFAULT
	removeobject LOCALID_PALLET_KANTO_BLUE
	setflag FLAG_TEMP_2
	releaseall
	end

PalletTown_Lab_Movement_BlueChallenges:
	walk_down
	walk_down
	step_end

PalletTown_Lab_EventScript_KantoNoUsableMon:
	lockall
	msgbox PalletTown_Lab_Text_KantoNoUsableMon, MSGBOX_DEFAULT
	releaseall
	end

PalletTown_Lab_EventScript_KantoNoExitBattle:
	end
#else
PalletTown_Lab_EventScript_KantoBlue::
PalletTown_Lab_EventScript_KantoBulbasaurBall::
PalletTown_Lab_EventScript_KantoCharmanderBall::
PalletTown_Lab_EventScript_KantoSquirtleBall::
PalletTown_Lab_EventScript_KantoExitBattle::
	end
#endif

PalletTown_Lab_EventScript_OakGiveNatDex_ForPlaytesters::
	setflag FLAG_SYS_NATIONAL_DEX
	special EnableNationalPokedex
	setvar VAR_0x8004, 1 @ DEX_REGION_KANTO
	special DexScript_GrantNationalExtension
	return

PalletTown_Lab_EventScript_OakBadgeCheck::
	goto_if_ge VAR_NUM_BADGES, 16, PalletTown_Lab_EventScript_Oak16Badges
	goto_if_ge VAR_NUM_BADGES, 9, PalletTown_Lab_EventScript_Oak9Badges
	msgbox PalletTown_Lab_Text_OakNoKantoBadges, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_Oak16Badges::
	msgbox PalletTown_Lab_Text_OakOpenMtSilver, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_INDIGOJUNCTION_HIDE_SILVER_GUARD
	setvar VAR_PALLETTOWN_LABSTATE, 2
#if !(IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED)
	call_if_unset FLAG_IS_KANTO_CHAMPION, PalletTown_Lab_EventScript_OakKantoLeague
#endif
	release
	end

PalletTown_Lab_EventScript_OakKantoLeague::
	msgbox PalletTown_Lab_Text_OakOpenIndigoPlateau, MSGBOX_DEFAULT
	closemessage
	return

PalletTown_Lab_EventScript_Oak9Badges::
	msgbox PalletTown_Lab_Text_OakYesKantoBadges, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_OakDex::
	call_if_unset FLAG_SYS_NATIONAL_DEX, PalletTown_Lab_EventScript_OakGiveNatDex_ForPlaytesters
	msgbox PalletTown_Lab_Text_OakLabDexCheck, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Lab_EventScript_Assistant1::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoAssistant
#endif
	msgbox PalletTown_Lab_Text_OaksAssistant1, MSGBOX_NPC
	end

PalletTown_Lab_EventScript_Assistant2::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoAssistant
#endif
	msgbox PalletTown_Lab_Text_OaksAssistant2, MSGBOX_NPC
	end

PalletTown_Lab_EventScript_Assistant3::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_Lab_EventScript_KantoAssistant
#endif
	msgbox PalletTown_Lab_Text_OaksAssistant3, MSGBOX_NPC
	end

#if IS_WAYFARER
PalletTown_Lab_EventScript_KantoAssistant::
	msgbox PalletTown_Lab_Text_KantoAssistant, MSGBOX_NPC
	end

PalletTown_Lab_Text_KantoAssistant:
	.string "PROF. OAK studies POKéMON from\n"
	.string "all over the world.$"
#endif

PalletTown_Lab_EventScript_PC::
	msgbox PalletTown_Lab_Text_OaksLabPC, MSGBOX_SIGN
	end

#if IS_WAYFARER
PalletTown_Lab_Text_KantoChooseStarter:
	.string "OAK: POKéMON are waiting in those\n"
	.string "three BALLS. Choose your partner.$"

PalletTown_Lab_Text_KantoAfterStarter:
	.string "OAK: Treat your new partner well.\p"
	.string "BLUE has chosen one too.$"

PalletTown_Lab_Text_KantoAfterBattle:
	.string "OAK: You're ready for ROUTE 1.\p"
	.string "VIRIDIAN CITY is just beyond it.$"

PalletTown_Lab_Text_KantoParcelWelcome:
	.string "OAK: Ah, {PLAYER}!\p"
	.string "Is that the PARCEL I ordered?$"

PalletTown_Lab_Text_KantoParcelAccepted:
	.string "OAK: Wonderful! This is what I\n"
	.string "was waiting for.$"

PalletTown_Lab_Text_KantoBlueArrives:
	.string "BLUE: Gramps! You called for me?$"

PalletTown_Lab_Text_KantoDexRequest:
	.string "OAK: I have a request for you two.\p"
	.string "Record every POKéMON you find\n"
	.string "with this POKéDEX.$"

PalletTown_Lab_Text_KantoDexReceived:
	.string "{PLAYER} received a POKéDEX\n"
	.string "from PROF. OAK!$"

PalletTown_Lab_Text_KantoBallOffer:
	.string "OAK: Here are five POKé BALLS\n"
	.string "to help you begin.$"

PalletTown_Lab_Text_KantoBallsReceived:
	.string "{PLAYER} received five\n"
	.string "POKé BALLS!$"

PalletTown_Lab_Text_KantoCatchExplanation:
	.string "OAK: When a wild POKéMON is weak,\n"
	.string "try throwing a POKé BALL.\p"
	.string "Keep exploring and fill the\n"
	.string "POKéDEX together.$"

PalletTown_Lab_Text_KantoBlueTownMap:
	.string "BLUE: I'll get a TOWN MAP from\n"
	.string "DAISY. Don't fall behind!$"

PalletTown_Lab_Text_KantoComplete:
	.string "OAK: Your journey starts now.\p"
	.string "Good luck, {PLAYER}!$"

PalletTown_Lab_Text_KantoParcelMissing:
	.string "OAK: I can't find the PARCEL.\p"
	.string "Bring it here and we'll continue.$"

PalletTown_Lab_Text_KantoBagFull:
	.string "OAK: Make room for five POKé BALLS,\n"
	.string "then speak to me again.$"

PalletTown_Lab_Text_KantoRewardRetry:
	.string "OAK: Let's try that handoff again.\p"
	.string "Speak to me when you're ready.$"

PalletTown_Lab_Text_KantoBlueWaiting:
	.string "BLUE: Where's Gramps?\p"
	.string "He said there would be POKéMON\n"
	.string "for us here.$"

PalletTown_Lab_Text_KantoBlueReady:
	.string "BLUE: Choose already! I want to\n"
	.string "see how strong my POKéMON is.$"

PalletTown_Lab_Text_KantoBlueDex:
	.string "BLUE: I'll fill my POKéDEX\n"
	.string "before you do.$"

PalletTown_Lab_Text_KantoStarterPreview:
	.string "{STR_VAR_1}, a {STR_VAR_2}-type\n"
	.string "POKéMON. Choose this one?$"

PalletTown_Lab_Text_KantoStarterReceived:
	.string "{PLAYER} received a first\n"
	.string "POKéMON from PROF. OAK!$"

PalletTown_Lab_Text_KantoBlueTakesCounter:
	.string "BLUE: I'll take this one!\p"
	.string "We can test them before you leave.$"

PalletTown_Lab_Text_KantoChooseAnother:
	.string "OAK: Take your time. You can\n"
	.string "inspect the others.$"

PalletTown_Lab_Text_KantoStarterFull:
	.string "OAK: We can't finish the handoff\n"
	.string "yet. Speak to me to try again.$"

PalletTown_Lab_Text_KantoBallUnavailable:
	.string "The BALL is no longer available.$"

PalletTown_Lab_Text_KantoBlueChallenge:
	.string "BLUE: Wait, {PLAYER}!\p"
	.string "Let's see whose POKéMON is\n"
	.string "stronger!$"

PalletTown_Lab_Text_KantoBlueWon:
	.string "BLUE: What? Unbelievable!$"

PalletTown_Lab_Text_KantoBlueLost:
	.string "BLUE: Yes! My POKéMON is great!$"

PalletTown_Lab_Text_KantoBlueLeaves:
	.string "BLUE: I'll get stronger.\p"
	.string "See you later, {PLAYER}!$"

PalletTown_Lab_Text_KantoNoUsableMon:
	.string "BLUE: Heal your POKéMON first.\p"
	.string "Then we can battle.$"
#endif

PalletTown_Lab_Text_OakLabDexCheck:
	.string "How is your POKéDEX coming?$"

PalletTown_Lab_Text_OakWelcomeKanto:
	.string "OAK: Ah, {PLAYER}!\n"
	.string "It's good of you to come all this\l"
	.string "way to KANTO.\p"
	.string "What do you think of the trainers\n"
	.string "out here? Pretty tough, huh?$"

PalletTown_Lab_Text_OakLabGoodbye:
	.string "If you're in the area, I hope you\n"
	.string "come visit again.$"

PalletTown_Lab_Text_OakOpenMtSilver:
	.string "OAK: Wow! That's excellent!\p"
	.string "You collected the BADGES of GYMS\n"
	.string "in KANTO. Well done!\p"
	.string "I was right in my assessment of\n"
	.string "you.\p"
	.string "Tell you what, {PLAYER}. I'll make\n"
	.string "arrangements so that you can go\l"
	.string "to MT.SILVER.\p"
	.string "MT.SILVER is a big mountain that\n"
	.string "is home to many wild POKéMON.\p"
	.string "It's too dangerous for your average\n"
	.string "trainer, so it's off limits. But\l"
	.string "we can make an exception in your\l"
	.string "case, {PLAYER}.\p"
	.string "Go up to INDIGO PLATEAU. You can\n"
	.string "reach MT.SILVER from there.$"

PalletTown_Lab_Text_OakOpenIndigoPlateau:
	.string "Oh, and speaking of the INDIGO\n"
	.string "PLATEAU, now that you've earned\l"
	.string "16 badges, the KANTO LEAGUE\l"
	.string "will be ready to accept\l"
	.string "your challenge once again.\p"
	.string "But be warned, LANCE and his\n"
	.string "ELITE FOUR won't go easy on you\l"
	.string "this time.\p"
	.string "Good luck, {PLAYER}!$"

PalletTown_Lab_Text_OakNoKantoBadges:
	.string "OAK: Hmm? You're not collecting\n"
	.string "KANTO GYM BADGES?\p"
	.string "The GYM LEADERS in KANTO are as\n"
	.string "tough as any you battled in JOHTO.\p"
	.string "I recommend that you challenge them.$"

PalletTown_Lab_Text_OakYesKantoBadges:
	.string "OAK: Ah, you're collecting KANTO\n"
	.string "GYM BADGES.\p"
	.string "I imagine that it's hard, but the\n"
	.string "experience is sure to help you.\p"
	.string "Come see me when you get them all.\p"
	.string "I'll have a gift for you.\p"
	.string "Keep trying hard, {PLAYER}!$"

PalletTown_Lab_Text_OaksAssistant1:
	.string "The PROF's POKéMON TALK radio\n"
	.string "program isn't aired here in KANTO.\p"
	.string "It's a shame--I'd like to hear it.$"

PalletTown_Lab_Text_OaksAssistant2:
	.string "Thanks to your work on the POKéDEX,\n"
	.string "the PROF's research is coming along\l"
	.string "great.$"

PalletTown_Lab_Text_OaksAssistant3:
	.string "Don't tell anyone, but PROF.OAK'S\n"
	.string "POKéMON TALK isn't a live broadcast.$"

PalletTown_Lab_Text_OaksLabPC:
	.string "There's an e-mail message on the PC.\p"
	.string "…\p"
	.string "PROF.OAK, how is your research\n"
	.string "coming along?\p"
	.string "I'm still plugging away.\p"
	.string "I heard rumors that {PLAYER} is\n"
	.string "getting quite a reputation.\p"
	.string "I'm delighted to hear that.\p"
	.string "ELM in NEW BARK TOWN 8-)$"
