MeteorFalls_StevensCave_MapScripts::
	.byte 0

MeteorFalls_StevensCave_EventScript_Steven::
	lock
	goto_if_set FLAG_DEFEATED_METEOR_FALLS_STEVEN, MeteorFalls_StevensCave_EventScript_Defeated
	waitse
	playse SE_PIN
	applymovement LOCALID_METEOR_FALLS_STEVEN, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_METEOR_FALLS_STEVEN, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_METEOR_FALLS_STEVEN, Common_Movement_FacePlayer
	waitmovement 0
#if IS_WAYFARER
	call MeteorFalls_StevensCave_EventScript_Greet
	@ He only recalls the SPACE CENTER if the player fought beside him there
	call_if_set FLAG_DEFEATED_MAGMA_SPACE_CENTER, MeteorFalls_StevensCave_EventScript_RecallSpaceCenter
	msgbox MeteorFalls_StevensCave_Text_ExpectWorst, MSGBOX_DEFAULT
#else
	msgbox MeteorFalls_StevensCave_Text_ShouldKnowHowGoodIAmExpectWorst, MSGBOX_DEFAULT
#endif
	trainerbattle_no_intro TRAINER_STEVEN, MeteorFalls_StevensCave_Text_StevenDefeat
#if IS_WAYFARER
	call MeteorFalls_StevensCave_EventScript_MyPredictionCameTrue
#else
	msgbox MeteorFalls_StevensCave_Text_MyPredictionCameTrue, MSGBOX_DEFAULT
#endif
	setflag FLAG_DEFEATED_METEOR_FALLS_STEVEN
	release
	end

MeteorFalls_StevensCave_EventScript_Defeated::
	applymovement LOCALID_METEOR_FALLS_STEVEN, Common_Movement_FacePlayer
	waitmovement 0
#if IS_WAYFARER
	call MeteorFalls_StevensCave_EventScript_MyPredictionCameTrue
#else
	msgbox MeteorFalls_StevensCave_Text_MyPredictionCameTrue, MSGBOX_DEFAULT
#endif
	release
	end

#if IS_WAYFARER
@ A player may reach his cave without ever meeting him: a stranger hears his name and is not told they were
@ expected; someone he knows gets his greeting and "I'm amazed you knew where to find me".
MeteorFalls_StevensCave_EventScript_Greet::
	setvar VAR_0x8004, NOTABLE_TRAINER_STEVEN
	specialvar VAR_RESULT, NotableStory_IsIntroduced
	goto_if_eq VAR_RESULT, TRUE, MeteorFalls_StevensCave_EventScript_GreetAcquaintance
	notable_introduce NOTABLE_TRAINER_STEVEN, MeteorFalls_StevensCave_Text_StevenIntro
	msgbox MeteorFalls_StevensCave_Text_RockManiacStranger, MSGBOX_DEFAULT
	return

MeteorFalls_StevensCave_EventScript_GreetAcquaintance::
	notable_introduce NOTABLE_TRAINER_STEVEN, MeteorFalls_StevensCave_Text_StevenIntro
	msgbox MeteorFalls_StevensCave_Text_ImAmazedYouFoundMe, MSGBOX_DEFAULT
	return

MeteorFalls_StevensCave_EventScript_RecallSpaceCenter::
	msgbox MeteorFalls_StevensCave_Text_BattledAtSpaceCenter, MSGBOX_DEFAULT
	return

@ GRANITE CAVE is only recalled if the LETTER reached him there. His cave opens on HOENN's own League win
@ (MeteorFalls_1F_1R's OnLoad reads HOENN's FLAG_SYS_GAME_CLEAR), so the CHAMPION line always holds here.
MeteorFalls_StevensCave_EventScript_MyPredictionCameTrue::
	goto_if_set FLAG_DELIVERED_STEVEN_LETTER, MeteorFalls_StevensCave_EventScript_PredictionFromGraniteCave
	msgbox MeteorFalls_StevensCave_Text_PredictionFromHearingOfYou, MSGBOX_DEFAULT
	msgbox MeteorFalls_StevensCave_Text_MyPredictionCameTrue, MSGBOX_DEFAULT
	return

MeteorFalls_StevensCave_EventScript_PredictionFromGraniteCave::
	msgbox MeteorFalls_StevensCave_Text_PredictionFromGraniteCave, MSGBOX_DEFAULT
	msgbox MeteorFalls_StevensCave_Text_MyPredictionCameTrue, MSGBOX_DEFAULT
	return

MeteorFalls_StevensCave_Text_StevenIntro:
	.string "STEVEN: I'm STEVEN. You must be\n"
	.string "{PLAYER}{KUN}.$"

MeteorFalls_StevensCave_Text_RockManiacStranger:
	.string "Not many find their way in here.\p"
	.string "Do you, uh…maybe think of me as\n"
	.string "just a rock maniac?\p"
	.string "No, that can't be right.$"

MeteorFalls_StevensCave_Text_ImAmazedYouFoundMe:
	.string "STEVEN: Oh, wow, {PLAYER}{KUN}.\n"
	.string "I'm amazed you knew where to find me.\p"
	.string "Do you, uh…maybe think of me as\n"
	.string "just a rock maniac?\p"
	.string "No, that can't be right.$"

MeteorFalls_StevensCave_Text_BattledAtSpaceCenter:
	.string "We battled alongside each other at\n"
	.string "the MOSSDEEP SPACE CENTER.\p"
	.string "You should have a very good idea\n"
	.string "about how good I am.$"

MeteorFalls_StevensCave_Text_ExpectWorst:
	.string "Okay, {PLAYER}{KUN}, if you're going to mount\n"
	.string "a serious challenge, expect the worst!$"

MeteorFalls_StevensCave_Text_StevenDefeat:
	.string "You…\n"
	.string "I had no idea you'd become so strong…$"

MeteorFalls_StevensCave_Text_PredictionFromGraniteCave:
	.string "STEVEN: Come to think of it, ever since\n"
	.string "our paths first crossed in GRANITE\l"
	.string "CAVE in DEWFORD, I had this feeling.$"

MeteorFalls_StevensCave_Text_PredictionFromHearingOfYou:
	.string "STEVEN: Come to think of it, ever since\n"
	.string "I first heard your name, I had this\l"
	.string "feeling.$"

MeteorFalls_StevensCave_Text_MyPredictionCameTrue:
	.string "I thought that you would eventually\n"
	.string "become the CHAMPION.\p"
	.string "My predictions usually come true.\p"
	.string "And where will you go from here?\p"
	.string "… … … … … …\n"
	.string "… … … … … …\p"
	.string "Fufufu, even I couldn't tell you that.$"
#else
MeteorFalls_StevensCave_Text_ShouldKnowHowGoodIAmExpectWorst:
	.string "STEVEN: Oh, wow, {PLAYER}{KUN}.\n"
	.string "I'm amazed you knew where to find me.\p"
	.string "Do you, uh…maybe think of me as\n"
	.string "just a rock maniac?\p"
	.string "No, that can't be right.\p"
	.string "We battled alongside each other at\n"
	.string "the SOOTOPOLIS SPACE CENTER.\p"
	.string "You should have a very good idea\n"
	.string "about how good I am.\p"
	.string "Okay, {PLAYER}{KUN}, if you're going to mount\n"
	.string "a serious challenge, expect the worst!$"

MeteorFalls_StevensCave_Text_StevenDefeat:
	.string "You…\n"
	.string "I had no idea you'd become so strong…$"

MeteorFalls_StevensCave_Text_MyPredictionCameTrue:
	.string "STEVEN: Come to think of it, ever since\n"
	.string "our paths first crossed in GRANITE\l"
	.string "CAVE in DEWFORD, I had this feeling.\p"
	.string "I thought that you would eventually\n"
	.string "become the CHAMPION.\p"
	.string "My predictions usually come true.\p"
	.string "And where will you go from here?\p"
	.string "… … … … … …\n"
	.string "… … … … … …\p"
	.string "Fufufu, even I couldn't tell you that.$"
#endif
