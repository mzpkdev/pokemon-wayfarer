ViridianCity_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_EventScript_CityEnter
	map_script MAP_SCRIPT_ON_FRAME_TABLE, ViridianCity_OnFrame
	.byte 0

ViridianCity_EventScript_CityEnter::
	setflag FLAG_VISITED_VIRIDIAN_CITY
#if IS_WAYFARER
	// Saves made before the HNS Cinnabar retirement may still hide Blue.
	goto_if_set FLAG_DEFEATED_VIRIDIAN_GYM, ViridianCity_EventScript_CityEnterDone
	clearflag FLAG_HIDE_VIRIDIAN_BLUE
ViridianCity_EventScript_CityEnterDone::
#endif
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
	goto_if_set FLAG_DEFEATED_VIRIDIAN_GYM, VirianCity_EventScript_GymGrampsBlue
	goto_if_set FLAG_WAYFARER_VIRIDIAN_BLUE_INTRODUCED, VirianCity_EventScript_GymGrampsBlue
#else
	goto_if_set FLAG_HIDE_CINNABAR_BLUE, VirianCity_EventScript_GymGrampsBlue
#endif
	msgbox ViridianCity_Text_GrampsNearGym, MSGBOX_NPC
	end

VirianCity_EventScript_GymGrampsBlue::
	msgbox ViridianCity_Text_GrampsNearGym_BlueReturned, MSGBOX_NPC
	end

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
	msgbox ViridianCity_Text_Youngster, MSGBOX_NPC
	end

ViridianCity_EventScript_CitySign::
	msgbox ViridianCity_Text_WelcomeSign, MSGBOX_SIGN
	end

ViridianCity_EventScript_CitySign2::
	msgbox ViridianCity_Text_Sign, MSGBOX_SIGN
	end

ViridianCity_EventScript_GymSign::
	msgbox ViridianCity_Text_GymSign, MSGBOX_SIGN
	end

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
	.string "BLUE leads the GYM here.\p"
	.string "He's right inside if you want to\n"
	.string "challenge him.$"
#else
	.string "This GYM didn't have a LEADER\n"
	.string "until recently.\p"
	.string "A young man from PALLET became\n"
	.string "the LEADER, but he's often away.$"
#endif

ViridianCity_Text_GrampsNearGym_BlueReturned:
	.string "Are you going to battle the\n"
	.string "LEADER?\p"
	.string "Good luck to you. You'll need it.$"

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
	.string "LEADER: …\p"
	.string "The rest of the text is illegible…$"

ViridianCity_Text_WelcomeSign:
	.string "WELCOME TO VIRIDIAN CITY,\p"
	.string "THE GATEWAY TO INDIGO PLATEAU$"

ViridianCity_Text_TrainerHouseSign:
	.string "TRAINER HOUSE\p"
	.string "The Club for Top Trainer Battles$"
