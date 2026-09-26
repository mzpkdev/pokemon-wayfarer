PalletTown_House2_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_House2_OnTransition
	.byte 0

PalletTown_House2_OnTransition:
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_House2_OnTransitionDone
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, PalletTown_House2_OnTransitionDone
	setobjectmovementtype LOCALID_PALLET_HOUSE2_DAISY, MOVEMENT_TYPE_FACE_RIGHT
	setobjectxyperm LOCALID_PALLET_HOUSE2_DAISY, 5, 4
PalletTown_House2_OnTransitionDone:
#endif
	end

PalletTown_House2_EventScript_Daisy::
	lock
	faceplayer
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_House2_EventScript_KantoDaisy
#else
	goto_if_unset FLAG_HIDE_CINNABAR_BLUE, PalletTown_House2_EventScript_Blue
#endif
	goto_if_unset FLAG_DAILY_DAISY_GROOMED, PalletTown_House2_EventScript_DaisyGrooming
	msgbox PalletTown_House2_Text_AlreadyGroomed, MSGBOX_DEFAULT
	closemessage
	release
	end

#if IS_WAYFARER
@ FRLG PalletTown_RivalsHouse_EventScript_Daisy through the Pokedex handoff;
@ the escort (phase 1+) stands in for lab var 1. Daisy's Town Map gift belongs
@ to a later Kanto specification, so after the handoff she keeps her native
@ HNS tea and grooming service.
PalletTown_House2_EventScript_KantoDaisy::
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, PalletTown_House2_EventScript_KantoDaisyServices
	specialvar VAR_RESULT, WayfarerKanto_GetPhase
	goto_if_ge VAR_RESULT, PALLET_OPENING_OAK_INTERCEPTION_AVAILABLE, PalletTown_House2_EventScript_KantoHeardBattledRival
	msgbox PalletTown_House2_Text_KantoHiBrothersAtLab
	closemessage
	applymovement LOCALID_PALLET_HOUSE2_DAISY, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

PalletTown_House2_EventScript_KantoHeardBattledRival:
	msgbox PalletTown_House2_Text_KantoHeardYouBattledRival
	release
	end

PalletTown_House2_EventScript_KantoDaisyServices:
	goto_if_unset FLAG_DAILY_DAISY_GROOMED, PalletTown_House2_EventScript_DaisyGrooming
	msgbox PalletTown_House2_Text_AlreadyGroomed, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_House2_Text_KantoHiBrothersAtLab:
	.string "DAISY: Hi, {PLAYER}!\p"
	.string "My brother, BLUE, is out at\n"
	.string "Grandpa's LAB.$"

PalletTown_House2_Text_KantoHeardYouBattledRival:
	.string "DAISY: {PLAYER}, I heard you had\n"
	.string "a battle against BLUE.\p"
	.string "I wish I'd seen that!$"

PalletTown_House2_Text_KantoItsBigMapOfKanto:
	.string "It's a big map of the KANTO region.\n"
	.string "Now this would be useful!$"

PalletTown_House2_Text_KantoShelvesCrammedFullOfBooks:
	.string "The shelves are crammed full of\n"
	.string "books on POKéMON.$"

PalletTown_House2_Text_KantoLovelyAndSweetClefairy:
	.string "“The lovely and sweet\n"
	.string "CLEFAIRY”$"
#endif

@ FRLG Town Map object, hosted on the table at (6,4).
PalletTown_House2_EventScript_KantoTownMap::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_House2_EventScript_KantoNone
	msgbox PalletTown_House2_Text_KantoItsBigMapOfKanto, MSGBOX_SIGN
#endif
	end

PalletTown_House2_EventScript_KantoBookshelf::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_House2_EventScript_KantoNone
	msgbox PalletTown_House2_Text_KantoShelvesCrammedFullOfBooks, MSGBOX_SIGN
#endif
	end

PalletTown_House2_EventScript_KantoPicture::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, PalletTown_House2_EventScript_KantoNone
	msgbox PalletTown_House2_Text_KantoLovelyAndSweetClefairy, MSGBOX_SIGN
#endif
	end

#if IS_WAYFARER
PalletTown_House2_EventScript_KantoNone:
	end
#endif

PalletTown_House2_EventScript_Blue::
	msgbox PalletTown_House2_Text_Hello, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_House2_EventScript_DaisyDeclineGrooming::
	msgbox PalletTown_House2_Text_Refused, MSGBOX_DEFAULT
	closemessage
	release
	end
PalletTown_House2_EventScript_Egg::
	msgbox PalletTown_House2_Text_CantGroomEgg, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_House2_EventScript_DaisyGrooming::
	msgbox PalletTown_House2_Text_OfferGrooming, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, PalletTown_House2_EventScript_DaisyDeclineGrooming
	msgbox PalletTown_House2_Text_WhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	lock
	faceplayer
	bufferpartymonnick STR_VAR_1, VAR_0x8004
	goto_if_ge VAR_0x8004, PARTY_SIZE, PalletTown_House2_EventScript_DaisyDeclineGrooming
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, PalletTown_House2_EventScript_Egg
	goto_if_eq VAR_RESULT, FALSE, PalletTown_House2_EventScript_DaisyDeclineGrooming
	
	msgbox PalletTown_House2_Text_Alright
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HaircutBrother1
	fadescreen FADE_FROM_BLACK
	special BufferMonNickname
	msgbox PalletTown_House2_Text_AllDone
	setflag FLAG_DAILY_DAISY_GROOMED
	release
	end

PalletTown_House2_Text_Hello:
#if IS_WAYFARER
	.string "DAISY: My kid brother BLUE is\n"
	.string "always looking for a battle.$"
#else
	.string "DAISY: Hi! My kid brother is the\n"
	.string "GYM LEADER in VIRIDIAN CITY.\p"
	.string "But he goes out of town so\n"
	.string "often, it causes problems for\l"
	.string "the trainers.$"
#endif

PalletTown_House2_Text_OfferGrooming:
	.string "DAISY: Hi! Good timing. I'm about\n"
	.string "to have some tea.\p"
	.string "Would you like to join me?\p"
	.string "Oh, your POKéMON are a bit dirty.\p"
	.string "Would you like me to groom one?$"

PalletTown_House2_Text_WhichMon:
	.string "DAISY: Which one should I groom?$"

PalletTown_House2_Text_Alright:
	.string "DAISY: OK, I'll get it looking\n"
	.string "nice in no time.$"

PalletTown_House2_Text_AllDone:
	.string "DAISY: There you go! All done.\p"
	.string "See? Doesn't {STR_VAR_1} look\n"
	.string "nice?\p"
	.string "It's such a cute POKéMON.$"

PalletTown_House2_Text_AlreadyGroomed:
	.string "DAISY: I always have tea around\n"
	.string "this time. Come join me.$"

PalletTown_House2_Text_Refused:
	.string "DAISY: You don't want to have one\n"
	.string "groomed?\p"
	.string "OK, we'll just have tea.$"

PalletTown_House2_Text_CantGroomEgg:
	.string "DAISY: Oh, sorry. I honestly can't\n"
	.string "groom an EGG.$"
