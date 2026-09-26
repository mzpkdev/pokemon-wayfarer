ViridianCity_hns_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, ViridianCity_KantoOnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_EventScript_CityEnter
	map_script MAP_SCRIPT_ON_FRAME_TABLE, ViridianCity_OnFrame
	.byte 0

#if IS_WAYFARER
@ FLAG_TEMP_4 hides the FRLG residents from other origins. VAR_TEMP_1 arms the
@ FRLG locked-Gym-door trigger while the Pallet-origin opening is active.
ViridianCity_KantoOnLoad:
	setflag FLAG_TEMP_4
	setvar VAR_TEMP_1, 0
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_KantoLoadDone
	clearflag FLAG_TEMP_4
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_KantoLoadDone
	setvar VAR_TEMP_1, 1
ViridianCity_KantoLoadDone:
	end
#else
ViridianCity_KantoOnLoad:
	setflag FLAG_TEMP_4
	end
#endif

ViridianCity_EventScript_CityEnter::
	setflag FLAG_VISITED_VIRIDIAN_CITY
	end

ViridianCity_OnFrame::
	map_script_2 VAR_OAK_MT_SILVER_CALL, 1, ViridianCity_EventScript_OakCall
	.2byte 0

ViridianCity_EventScript_OakCall::
	lock
	setspeaker ViridianCity_Text_OakName
	pokenavcall ViridianCity_Text_OakCall
	waitmessage
	delay 30
	setvar VAR_OAK_MT_SILVER_CALL, 2
	release
	end

ViridianCity_Text_OakName:
	.string "OAK$"

ViridianCity_Text_OakCall:
	.string "OAK: Ah, {PLAYER}!\p"
	.string "Come and see my at my\n"
	.string "LAB in PALLET TOWN\l"
	.string "when you get the chance.$"

ViridianCity_EventScript_Gramps::
	lock
	faceplayer
#if IS_WAYFARER
	call ViridianCity_EventScript_KantoIsOpeningActive
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_EventScript_KantoTutorialNotReady
#endif
	msgbox ViridianCity_Text_CoffeeGramps_Question, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_EventScript_GrampsDoubted
	msgbox ViridianCity_Text_CoffeeGramps_Believed, MSGBOX_DEFAULT
	closemessage
	release
	end

ViridianCity_EventScript_GrampsDoubted::
	msgbox ViridianCity_Text_CoffeeGramps_Doubted, MSGBOX_DEFAULT
	closemessage
	release
	end

VirianCity_EventScript_GymGramps::
#if IS_WAYFARER
	@ The opening keeps FRLG's closed-Gym line; afterwards #133's Giovanni
	@ state decides for every origin.
	call ViridianCity_EventScript_KantoIsOpeningActive
	goto_if_eq VAR_RESULT, TRUE, VirianCity_EventScript_KantoGymGramps
	goto_if_set FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS, VirianCity_EventScript_GymGrampsBlue
#else
	goto_if_set FLAG_HIDE_CINNABAR_BLUE, VirianCity_EventScript_GymGrampsBlue
#endif
	msgbox ViridianCity_Text_GrampsNearGym, MSGBOX_NPC
	end

#if IS_WAYFARER
@ FRLG ViridianCity_EventScript_OldMan (gym-door var 0 branch).
VirianCity_EventScript_KantoGymGramps::
	lock
	faceplayer
	msgbox ViridianCity_Text_KantoGymClosedWonderWhoLeaderIs
	closemessage
	applymovement LOCALID_VIRIDIAN_KANTO_GYM_OLD_MAN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

@ VAR_RESULT = Pallet origin with the opening still active.
ViridianCity_EventScript_KantoIsOpeningActive:
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_EventScript_KantoReturn
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_EventScript_KantoOpeningDone
	setvar VAR_RESULT, TRUE
	return
ViridianCity_EventScript_KantoOpeningDone:
	setvar VAR_RESULT, FALSE
ViridianCity_EventScript_KantoReturn:
	return

@ FRLG ViridianCity_EventScript_TutorialNotReady (old man var 0) on Coffee Gramps.
ViridianCity_EventScript_KantoTutorialNotReady:
	msgbox ViridianCity_Text_KantoThisIsPrivateProperty
	closemessage
	release
	end

@ FRLG ViridianCity_EventScript_Woman. The opening stands in for old man var 0.
ViridianCity_EventScript_KantoGranddaughter::
	lock
	faceplayer
	call ViridianCity_EventScript_KantoIsOpeningActive
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_EventScript_KantoGranddaughterRoadBlocked
	msgbox ViridianCity_Text_KantoGoShoppingInPewterOccasionally
	release
	end

ViridianCity_EventScript_KantoGranddaughterRoadBlocked:
	msgbox ViridianCity_Text_KantoGrandpaHasntHadCoffeeYet
	closemessage
	applymovement LOCALID_VIRIDIAN_KANTO_GRANDDAUGHTER, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

ViridianCity_EventScript_KantoBoy::
	msgbox ViridianCity_Text_KantoCanCarryMonsAnywhere, MSGBOX_NPC
	end

@ FRLG ViridianCity_EventScript_Youngster on the HNS youngster.
ViridianCity_EventScript_KantoYoungster:
	lock
	faceplayer
	msgbox ViridianCity_Text_KantoWantToKnowAboutCaterpillarMons, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, ViridianCity_EventScript_KantoYoungsterExplainCaterpillars
	goto_if_eq VAR_RESULT, NO, ViridianCity_EventScript_KantoYoungsterDeclineExplanation
	end

ViridianCity_EventScript_KantoYoungsterExplainCaterpillars:
	msgbox ViridianCity_Text_KantoExplainCaterpieWeedle
	release
	end

ViridianCity_EventScript_KantoYoungsterDeclineExplanation:
	msgbox ViridianCity_Text_KantoOhOkayThen
	release
	end

ViridianCity_EventScript_KantoCitySign:
	msgbox ViridianCity_Text_KantoCitySign, MSGBOX_SIGN
	end

ViridianCity_Text_KantoGymClosedWonderWhoLeaderIs:
	.string "This POKéMON GYM is always closed.\p"
	.string "I wonder who the LEADER is?$"

ViridianCity_Text_KantoThisIsPrivateProperty:
	.string "I absolutely forbid you from\n"
	.string "going through here!\p"
	.string "This is private property!$"

ViridianCity_Text_KantoGrandpaHasntHadCoffeeYet:
	.string "Oh, Grandpa!\n"
	.string "Don't be so mean!\p"
	.string "I'm so sorry.\n"
	.string "He hasn't had his coffee yet.$"

ViridianCity_Text_KantoGoShoppingInPewterOccasionally:
	.string "I go shopping in PEWTER CITY\n"
	.string "occasionally.\p"
	.string "I have to take the winding trail in\n"
	.string "VIRIDIAN FOREST when I go.$"

ViridianCity_Text_KantoCanCarryMonsAnywhere:
	.string "Those POKé BALLS at your waist!\n"
	.string "You have POKéMON, don't you?\p"
	.string "It's great that you can carry and\n"
	.string "use POKéMON anytime, anywhere.$"

ViridianCity_Text_KantoWantToKnowAboutCaterpillarMons:
	.string "You want to know about the two\n"
	.string "kinds of caterpillar POKéMON?$"

ViridianCity_Text_KantoOhOkayThen:
	.string "Oh, okay then!$"

ViridianCity_Text_KantoExplainCaterpieWeedle:
	.string "CATERPIE has no poison,\n"
	.string "but WEEDLE does.\p"
	.string "Watch that your POKéMON aren't\n"
	.string "stabbed by WEEDLE's POISON STING.$"

ViridianCity_Text_KantoCitySign:
	.string "VIRIDIAN CITY \n"
	.string "The Eternally Green Paradise$"

ViridianCity_Text_KantoGymDoorsAreLocked:
	.string "VIRIDIAN GYM's doors are locked…$"
#endif

#if !IS_WAYFARER
ViridianCity_EventScript_KantoGranddaughter::
ViridianCity_EventScript_KantoBoy::
	end
#endif

@ FRLG ViridianCity_EventScript_GymDoorLocked, coord (43,15) below the HNS Gym
@ door while VAR_TEMP_1 is 1. HNS has no ledge here, so the player steps back
@ one tile instead of FRLG's jump_2_down.
ViridianCity_EventScript_KantoGymDoorLocked::
#if IS_WAYFARER
	lockall
	@ HNS has no ledge below the door, so only a northward approach is refused.
	specialvar VAR_RESULT, GetPlayerFacingDirection
	goto_if_ne VAR_RESULT, DIR_NORTH, ViridianCity_EventScript_KantoGymDoorPassing
	textcolor NPC_TEXT_COLOR_NEUTRAL
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	delay 20
	msgbox ViridianCity_Text_KantoGymDoorsAreLocked
	closemessage
	applymovement LOCALID_PLAYER, ViridianCity_Movement_KantoStepBackFromGym
	waitmovement 0
	releaseall
#endif
	end

#if IS_WAYFARER
ViridianCity_EventScript_KantoGymDoorPassing:
	releaseall
	end

ViridianCity_Movement_KantoStepBackFromGym:
	walk_down
	step_end
#endif

VirianCity_EventScript_GymGrampsBlue::
#if IS_WAYFARER
	msgbox ViridianCity_Text_GrampsNearGym_GiovanniDeparted, MSGBOX_NPC
#else
	msgbox ViridianCity_Text_GrampsNearGym_BlueReturned, MSGBOX_NPC
#endif
	end

#if !IS_WAYFARER
ViridianCity_EventScript_BlueIntro::
	lock
	faceplayer
	msgbox ViridianCity_Text_BlueIntro, MSGBOX_DEFAULT
	setflag FLAG_WAYFARER_VIRIDIAN_BLUE_INTRODUCED
	setflag FLAG_HIDE_VIRIDIAN_BLUE_INTRO
	removeobject LOCALID_VIRIDIAN_BLUE_INTRO
	release
	end

ViridianCity_Text_BlueIntro:
	.string "BLUE: Haven't seen you around.\p"
	.string "I'm BLUE, the GYM LEADER here.\p"
	.string "Looking for a battle? Come inside\n"
	.string "and show me what you've got.$"
#endif

ViridianCity_EventScript_Fisher::
	goto_if_set FLAG_GOT_VIRIDIAN_TM_DREAM_EATER, ViridianCity_EventScript_FisherAfter
	msgbox ViridianCity_Text_DreamEaterFisher, MSGBOX_NPC
	giveitem ITEM_TM_DREAM_EATER
	setflag FLAG_GOT_VIRIDIAN_TM_DREAM_EATER
	end

ViridianCity_EventScript_FisherAfter::
	msgbox ViridianCity_Text_DreamEaterFisher_GotTM, MSGBOX_NPC
	end

ViridianCity_EventScript_Youngster::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_EventScript_KantoYoungster
#endif
	msgbox ViridianCity_Text_Youngster, MSGBOX_NPC
	end

ViridianCity_EventScript_CitySign::
	msgbox ViridianCity_Text_WelcomeSign, MSGBOX_SIGN
	end

ViridianCity_EventScript_CitySign2::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_EventScript_KantoCitySign
#endif
	msgbox ViridianCity_Text_Sign, MSGBOX_SIGN
	end

ViridianCity_EventScript_GymSign::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_EventScript_KantoGymSign
#endif
	msgbox ViridianCity_Text_GymSign, MSGBOX_SIGN
	end

#if IS_WAYFARER
ViridianCity_EventScript_KantoGymSign::
	msgbox ViridianCity_Text_KantoGymSign, MSGBOX_SIGN
	end

ViridianCity_Text_KantoGymSign:
	.string "VIRIDIAN CITY POKéMON GYM$"
#endif

ViridianCity_Text_CoffeeGramps_Question:
	.string "Hey, kid! I just had a double shot\n"
	.string "of espresso, and I am wired!\p"
	.string "I need to talk to someone, so you'll\n"
	.string "have to do!\p"
	.string "I might not look like much now, but\n"
	.string "I was an expert at catching POKéMON.\p"
	.string "Do you believe me?$"

ViridianCity_Text_CoffeeGramps_Believed:
	.string "Good, good. Yes, I was something\n"
	.string "out of the ordinary, let me tell\l"
	.string "you!$"

ViridianCity_Text_CoffeeGramps_Doubted:
	.string "What? You little whelp!\p"
	.string "If I were just a bit younger, I'd\n"
	.string "show you a thing or two. Humph!$"

ViridianCity_Text_GrampsNearGym:
#if IS_WAYFARER
	.string "GIOVANNI leads the GYM here.\p"
	.string "The doors are open, so take a\n"
	.string "look inside.$"
#else
	.string "This GYM didn't have a LEADER\n"
	.string "until recently.\p"
	.string "A young man from PALLET became\n"
	.string "the LEADER, but he's often away.$"
#endif

#if !IS_WAYFARER
ViridianCity_Text_GrampsNearGym_BlueReturned:
	.string "Are you going to battle the\n"
	.string "LEADER?\p"
	.string "Good luck to you. You'll need it.$"
#endif

#if IS_WAYFARER
ViridianCity_Text_GrampsNearGym_GiovanniDeparted:
	.string "GIOVANNI has left the GYM.\p"
	.string "You can still go inside.$"
#endif

ViridianCity_Text_DreamEaterFisherNew:
	.string "…Zzzzz…\p" 
	.string "Yawn!\p"
	.string "I must have dozed off in the sun.\p"
	.string "What? I remind you of somebody?\p"
	.string "I don't know, here, have this.$"


ViridianCity_Text_DreamEaterFisher_GotTM::
	.string "…Zzzzz…$"

ViridianCity_Text_DreamEaterFisher:
	.string "Yawn!\p"
	.string "I must have dozed off in the sun.\p"
	.string "…I had this dream about a DROWZEE\n"
	.string "eating my dream. Weird, huh?\p"
	.string "Huh? What's this?\p"
	.string "Where did this TM come from?\p"
	.string "This is spooky! Here, you can have\n"
	.string "this TM.$"

ViridianCity_Text_DreamEaterFisher_GotDreamEater:
	.string "TM42 contains DREAM EATER…\p"
	.string "…Zzzzz…$"

ViridianCity_Text_Youngster:
	.string "I heard that there are many items\n"
	.string "on the ground in VIRIDIAN FOREST.$"

ViridianCity_Text_Sign:
	.string "VIRIDIAN CITY\p"
	.string "The Eternally Green Paradise$"

ViridianCity_Text_GymSign:
	.string "VIRIDIAN CITY POKéMON GYM\n"
#if IS_WAYFARER
	.string "LEADER: GIOVANNI$"
#else
	.string "LEADER: …\p"
	.string "The rest of the text is illegible…$"
#endif

ViridianCity_Text_WelcomeSign:
	.string "WELCOME TO VIRIDIAN CITY,\p"
	.string "THE GATEWAY TO INDIGO PLATEAU$"

ViridianCity_Text_TrainerHouseSign:
	.string "TRAINER HOUSE\p"
	.string "The Club for Top Trainer Battles$"
