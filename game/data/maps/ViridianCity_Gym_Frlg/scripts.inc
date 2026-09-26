ViridianCity_Gym_Frlg_MapScripts::
#if IS_WAYFARER
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_Gym_KantoOnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, ViridianCity_Gym_KantoOnFrame
#endif
	.byte 0

#if IS_WAYFARER
@ The Pallet-origin opening keeps Viridian's Gym closed. The town's door
@ trigger refuses a northward approach; this catches any other entry and
@ returns the player to the doorstep. Not FRLG text: FRLG never reaches it.
ViridianCity_Gym_KantoOnTransition:
	setvar VAR_TEMP_1, 0
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_KantoTransitionDone
	specialvar VAR_RESULT, WayfarerKanto_IsOpeningComplete
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_Gym_KantoTransitionDone
	setvar VAR_TEMP_1, 1
ViridianCity_Gym_KantoTransitionDone:
	end

ViridianCity_Gym_KantoOnFrame:
	map_script_2 VAR_TEMP_1, 1, ViridianCity_Gym_KantoRefuseEntry
	.2byte 0

ViridianCity_Gym_KantoRefuseEntry:
	lockall
	setvar VAR_TEMP_1, 0
	msgbox ViridianCity_Gym_Text_KantoClosed, MSGBOX_DEFAULT
	closemessage
	warp MAP_VIRIDIAN_CITY_HNS, 43, 15
	waitstate
	releaseall
	end

ViridianCity_Gym_Text_KantoClosed:
	.string "The GYM is closed for now.\p"
	.string "Come back another time.$"
#endif

#if IS_WAYFARER
ViridianCity_Gym_EventScript_Giovanni::
ViridianCity_Gym_EventScript_GiovanniWayfarer::
	lock
	faceplayer
	goto_if_set FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS, ViridianCity_Gym_EventScript_GiovanniDeparted
	goto_if_defeated TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS, ViridianCity_Gym_EventScript_GiveEarthquake
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS, ViridianCity_Gym_EventScript_GiovanniHideoutMissing
	goto_if_unset FLAG_CELADON_ROCKET_HIDEOUT_SILPH_SCOPE_RECEIVED_HNS, ViridianCity_Gym_EventScript_GiovanniScopeMissing
	goto_if_unset FLAG_SILPH_LIBERATED_HNS, ViridianCity_Gym_EventScript_GiovanniSilphMissing
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_NoUsableParty
	msgbox ViridianCity_Gym_Text_GiovanniWayfarerIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS, ViridianCity_Gym_Text_GiovanniWayfarerDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, ViridianCity_Gym_EventScript_EndAfterBattle
	setflag FLAG_DEFEATED_VIRIDIAN_GYM
	goto_if_set FLAG_BADGE16_GET, ViridianCity_Gym_EventScript_GiveEarthquake
	message ViridianCity_Gym_Text_ReceivedEarthBadgeWayfarer
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE16_GET
	addvar VAR_NUM_BADGES, 1
	setvar VAR_0x8008, 16
	call Common_EventScript_SetGymTrainers_hns
	goto ViridianCity_Gym_EventScript_GiveEarthquake

ViridianCity_Gym_EventScript_GiveEarthquake::
	msgbox ViridianCity_Gym_Text_EarthquakeOffer, MSGBOX_DEFAULT
	checkitemspace ITEM_TM_EARTHQUAKE
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_NoRoomForEarthquake
	additem ITEM_TM_EARTHQUAKE
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_NoRoomForEarthquake
	msgreceiveditem ViridianCity_Gym_Text_ReceivedEarthquake, ITEM_TM_EARTHQUAKE, 1, MUS_OBTAIN_ITEM
	msgbox ViridianCity_Gym_Text_GiovanniDeparture, MSGBOX_DEFAULT
	setflag FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_VIRIDIAN_GIOVANNI
	fadescreen FADE_FROM_BLACK
	release
	end

ViridianCity_Gym_EventScript_GiovanniHideoutMissing::
	goto_if_unset FLAG_SILPH_LIBERATED_HNS, ViridianCity_Gym_EventScript_GiovanniBothMissing
	msgbox ViridianCity_Gym_Text_HideoutMissing, MSGBOX_DEFAULT
	release
	end

ViridianCity_Gym_EventScript_GiovanniBothMissing::
	msgbox ViridianCity_Gym_Text_BothMissing, MSGBOX_DEFAULT
	release
	end

ViridianCity_Gym_EventScript_GiovanniScopeMissing::
	msgbox ViridianCity_Gym_Text_ScopeMissing, MSGBOX_DEFAULT
	release
	end

ViridianCity_Gym_EventScript_GiovanniSilphMissing::
	msgbox ViridianCity_Gym_Text_SilphMissing, MSGBOX_DEFAULT
	release
	end

ViridianCity_Gym_EventScript_NoUsableParty::
	msgbox ViridianCity_Gym_Text_NoUsableParty, MSGBOX_DEFAULT
	release
	end

ViridianCity_Gym_EventScript_NoRoomForEarthquake::
	msgbox ViridianCity_Gym_Text_NoRoomForEarthquake, MSGBOX_DEFAULT
	release
	end

ViridianCity_Gym_EventScript_GiovanniDeparted::
	release
	end

ViridianCity_Gym_EventScript_EndAfterBattle::
	release
	end

ViridianCity_Gym_EventScript_Jason::
	goto_if_defeated TRAINER_VIRIDIAN_GYM_JASON_HNS, ViridianCity_Gym_EventScript_JasonPost
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_OrdinaryNoParty
ViridianCity_Gym_EventScript_Jason_TrainerBattle::
	trainerbattle_single TRAINER_VIRIDIAN_GYM_JASON_HNS, ViridianCity_Gym_Text_JasonIntro, ViridianCity_Gym_Text_JasonDefeat
ViridianCity_Gym_EventScript_JasonPost::
	msgbox ViridianCity_Gym_Text_JasonPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Cole::
	goto_if_defeated TRAINER_VIRIDIAN_GYM_COLE_HNS, ViridianCity_Gym_EventScript_ColePost
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_OrdinaryNoParty
ViridianCity_Gym_EventScript_Cole_TrainerBattle::
	trainerbattle_single TRAINER_VIRIDIAN_GYM_COLE_HNS, ViridianCity_Gym_Text_ColeIntro, ViridianCity_Gym_Text_ColeDefeat
ViridianCity_Gym_EventScript_ColePost::
	msgbox ViridianCity_Gym_Text_ColePostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Atsushi::
	goto_if_defeated TRAINER_VIRIDIAN_GYM_ATSUSHI_HNS, ViridianCity_Gym_EventScript_AtsushiPost
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_OrdinaryNoParty
ViridianCity_Gym_EventScript_Atsushi_TrainerBattle::
	trainerbattle_single TRAINER_VIRIDIAN_GYM_ATSUSHI_HNS, ViridianCity_Gym_Text_AtsushiIntro, ViridianCity_Gym_Text_AtsushiDefeat
ViridianCity_Gym_EventScript_AtsushiPost::
	msgbox ViridianCity_Gym_Text_AtsushiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Kiyo::
	goto_if_defeated TRAINER_VIRIDIAN_GYM_KIYO_HNS, ViridianCity_Gym_EventScript_KiyoPost
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_OrdinaryNoParty
ViridianCity_Gym_EventScript_Kiyo_TrainerBattle::
	trainerbattle_single TRAINER_VIRIDIAN_GYM_KIYO_HNS, ViridianCity_Gym_Text_KiyoIntro, ViridianCity_Gym_Text_KiyoDefeat
ViridianCity_Gym_EventScript_KiyoPost::
	msgbox ViridianCity_Gym_Text_KiyoPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Takashi::
	goto_if_defeated TRAINER_VIRIDIAN_GYM_TAKASHI_HNS, ViridianCity_Gym_EventScript_TakashiPost
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_OrdinaryNoParty
ViridianCity_Gym_EventScript_Takashi_TrainerBattle::
	trainerbattle_single TRAINER_VIRIDIAN_GYM_TAKASHI_HNS, ViridianCity_Gym_Text_TakashiIntro, ViridianCity_Gym_Text_TakashiDefeat
ViridianCity_Gym_EventScript_TakashiPost::
	msgbox ViridianCity_Gym_Text_TakashiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Samuel::
	goto_if_defeated TRAINER_VIRIDIAN_GYM_SAMUEL_HNS, ViridianCity_Gym_EventScript_SamuelPost
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_OrdinaryNoParty
ViridianCity_Gym_EventScript_Samuel_TrainerBattle::
	trainerbattle_single TRAINER_VIRIDIAN_GYM_SAMUEL_HNS, ViridianCity_Gym_Text_SamuelIntro, ViridianCity_Gym_Text_SamuelDefeat
ViridianCity_Gym_EventScript_SamuelPost::
	msgbox ViridianCity_Gym_Text_SamuelPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Yuji::
	goto_if_defeated TRAINER_VIRIDIAN_GYM_YUJI_HNS, ViridianCity_Gym_EventScript_YujiPost
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_OrdinaryNoParty
ViridianCity_Gym_EventScript_Yuji_TrainerBattle::
	trainerbattle_single TRAINER_VIRIDIAN_GYM_YUJI_HNS, ViridianCity_Gym_Text_YujiIntro, ViridianCity_Gym_Text_YujiDefeat
ViridianCity_Gym_EventScript_YujiPost::
	msgbox ViridianCity_Gym_Text_YujiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Warren::
	goto_if_defeated TRAINER_VIRIDIAN_GYM_WARREN_HNS, ViridianCity_Gym_EventScript_WarrenPost
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_OrdinaryNoParty
ViridianCity_Gym_EventScript_Warren_TrainerBattle::
	trainerbattle_single TRAINER_VIRIDIAN_GYM_WARREN_HNS, ViridianCity_Gym_Text_WarrenIntro, ViridianCity_Gym_Text_WarrenDefeat
ViridianCity_Gym_EventScript_WarrenPost::
	msgbox ViridianCity_Gym_Text_WarrenPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_OrdinaryNoParty::
	msgbox ViridianCity_Gym_Text_NoUsableParty, MSGBOX_AUTOCLOSE
	end

ViridianCity_FrlgGym_EventScript_GymGuy::
	lock
	faceplayer
	goto_if_set FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS, ViridianCity_Gym_EventScript_GymGuyPostVictory
	goto_if_set FLAG_DEFEATED_VIRIDIAN_GYM, ViridianCity_Gym_EventScript_GymGuyBadgeWon
	goto_if_not_defeated TRAINER_CELADON_HIDEOUT_GIOVANNI_HNS, ViridianCity_Gym_EventScript_GymGuyEarly
	goto_if_unset FLAG_CELADON_ROCKET_HIDEOUT_SILPH_SCOPE_RECEIVED_HNS, ViridianCity_Gym_EventScript_GymGuyEarly
	goto_if_unset FLAG_SILPH_LIBERATED_HNS, ViridianCity_Gym_EventScript_GymGuyEarly
	msgbox ViridianCity_Gym_Text_GymGuyReadyWayfarer, MSGBOX_DEFAULT
	release
	end
ViridianCity_Gym_EventScript_GymGuyEarly::
	msgbox ViridianCity_Gym_Text_GymGuyEarlyWayfarer, MSGBOX_DEFAULT
	release
	end
ViridianCity_Gym_EventScript_GymGuyBadgeWon::
	msgbox ViridianCity_Gym_Text_GymGuyRewardPending, MSGBOX_DEFAULT
	release
	end
ViridianCity_Gym_EventScript_GymGuyPostVictory::
	msgbox ViridianCity_Gym_Text_GymGuyPostVictoryWayfarer, MSGBOX_DEFAULT
	release
	end

ViridianCity_FrlgGym_EventScript_GymStatue::
	lockall
	goto_if_set FLAG_BADGE16_GET, ViridianCity_FrlgGym_EventScript_GymStatuePostVictory
	msgbox ViridianCity_Gym_Text_GymStatueWayfarer, MSGBOX_DEFAULT
	releaseall
	end
ViridianCity_FrlgGym_EventScript_GymStatuePostVictory::
	msgbox ViridianCity_Gym_Text_GymStatuePlayerWonWayfarer, MSGBOX_DEFAULT
	releaseall
	end

#else
ViridianCity_Gym_EventScript_Giovanni::
	famechecker FAMECHECKER_GIOVANNI, FCPICKSTATE_COLORED, UpdatePickStateFromSpecialVar8005
	trainerbattle_single TRAINER_LEADER_GIOVANNI, ViridianCity_Gym_Text_GiovanniIntro, ViridianCity_Gym_Text_GiovanniDefeat, ViridianCity_Gym_EventScript_DefeatedGiovanni, NO_MUSIC
	goto_if_unset FLAG_GOT_TM26_FROM_GIOVANNI, ViridianCity_Gym_EventScript_GiveTM26
	msgbox ViridianCity_Gym_Text_GiovanniPostBattle
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_VIRIDIAN_GIOVANNI
	fadescreen FADE_FROM_BLACK
	release
	end

ViridianCity_Gym_EventScript_DefeatedGiovanni::
	famechecker FAMECHECKER_GIOVANNI, 3
	setflag FLAG_HIDE_MISC_KANTO_ROCKETS
	setflag FLAG_DEFEATED_LEADER_GIOVANNI
	setflag FLAG_BADGE08_GET
	setvar VAR_MAP_SCENE_ROUTE22, 3
	set_gym_trainers_frlg 8
	goto ViridianCity_Gym_EventScript_GiveTM26
	end

ViridianCity_Gym_EventScript_GiveTM26::
	msgbox ViridianCity_Gym_Text_ExplainEarthBadgeTakeThis
	checkitemspace ITEM_TM26
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_NoRoomForTM26
	giveitem_msg ViridianCity_Gym_Text_ReceivedTM26FromGiovanni, ITEM_TM26
	setflag FLAG_GOT_TM26_FROM_GIOVANNI
	msgbox ViridianCity_Gym_Text_ExplainTM26
	release
	end

ViridianCity_Gym_EventScript_NoRoomForTM26::
	msgbox ViridianCity_Gym_Text_YouDoNotHaveSpace
	release
	end

ViridianCity_Gym_EventScript_Jason::
	trainerbattle_single TRAINER_TAMER_JASON, ViridianCity_Gym_Text_JasonIntro, ViridianCity_Gym_Text_JasonDefeat
	msgbox ViridianCity_Gym_Text_JasonPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Cole::
	trainerbattle_single TRAINER_TAMER_COLE, ViridianCity_Gym_Text_ColeIntro, ViridianCity_Gym_Text_ColeDefeat
	msgbox ViridianCity_Gym_Text_ColePostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Atsushi::
	trainerbattle_single TRAINER_BLACK_BELT_ATSUSHI, ViridianCity_Gym_Text_AtsushiIntro, ViridianCity_Gym_Text_AtsushiDefeat
	msgbox ViridianCity_Gym_Text_AtsushiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Kiyo::
	trainerbattle_single TRAINER_BLACK_BELT_KIYO, ViridianCity_Gym_Text_KiyoIntro, ViridianCity_Gym_Text_KiyoDefeat
	msgbox ViridianCity_Gym_Text_KiyoPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Takashi::
	trainerbattle_single TRAINER_BLACK_BELT_TAKASHI, ViridianCity_Gym_Text_TakashiIntro, ViridianCity_Gym_Text_TakashiDefeat
	msgbox ViridianCity_Gym_Text_TakashiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Samuel::
	trainerbattle_single TRAINER_COOLTRAINER_SAMUEL, ViridianCity_Gym_Text_SamuelIntro, ViridianCity_Gym_Text_SamuelDefeat
	msgbox ViridianCity_Gym_Text_SamuelPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Yuji::
	trainerbattle_single TRAINER_COOLTRAINER_YUJI, ViridianCity_Gym_Text_YujiIntro, ViridianCity_Gym_Text_YujiDefeat
	msgbox ViridianCity_Gym_Text_YujiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Warren::
	trainerbattle_single TRAINER_COOLTRAINER_WARREN, ViridianCity_Gym_Text_WarrenIntro, ViridianCity_Gym_Text_WarrenDefeat
	msgbox ViridianCity_Gym_Text_WarrenPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_GymGuy::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_LEADER_GIOVANNI, ViridianCity_Gym_EventScript_GymGuyPostVictory
	msgbox ViridianCity_Gym_Text_GymGuyAdvice
	release
	end

ViridianCity_Gym_EventScript_GymGuyPostVictory::
	famechecker FAMECHECKER_GIOVANNI, 4
	msgbox ViridianCity_Gym_Text_GymGuyPostVictory
	release
	end

ViridianCity_Gym_EventScript_GymStatue::
	lockall
	goto_if_set FLAG_BADGE08_GET, ViridianCity_Gym_EventScript_GymStatuePostVictory
	msgbox ViridianCity_Gym_Text_GymStatue
	releaseall
	end

ViridianCity_Gym_EventScript_GymStatuePostVictory::
	msgbox ViridianCity_Gym_Text_GymStatuePlayerWon
	releaseall
	end


#endif

#if IS_WAYFARER
ViridianCity_Gym_Text_GiovanniWayfarerIntro::
	.string "GIOVANNI: We meet again.\p"
	.string "The VIRIDIAN GYM is my final\n"
	.string "challenge to you. Show me all\l"
	.string "you have learned!$"
ViridianCity_Gym_Text_GiovanniWayfarerDefeat::
	.string "GIOVANNI: An intense battle.\n"
	.string "You have earned your victory.$"
ViridianCity_Gym_Text_ReceivedEarthBadgeWayfarer::
	.string "GIOVANNI gave you the\n"
	.string "EARTHBADGE!$"
ViridianCity_Gym_Text_EarthquakeOffer::
	.string "GIOVANNI: The EARTHBADGE is\n"
	.string "yours. Take this TM as well.\p"
	.string "It teaches EARTHQUAKE.$"
ViridianCity_Gym_Text_ReceivedEarthquake::
	.string "{PLAYER} received a TM for\n"
	.string "EARTHQUAKE from GIOVANNI.$"
ViridianCity_Gym_Text_GiovanniDeparture::
	.string "GIOVANNI: My time as this GYM's\n"
	.string "LEADER is over. Farewell.$"
ViridianCity_Gym_Text_HideoutMissing::
	.string "GIOVANNI: Settle our business\n"
	.string "in CELADON's ROCKET HIDEOUT\l"
	.string "before you challenge me here.$"
ViridianCity_Gym_Text_BothMissing::
	.string "GIOVANNI: Our business in the\n"
	.string "CELADON ROCKET HIDEOUT and\l"
	.string "SILPH CO. is unfinished.$"
ViridianCity_Gym_Text_ScopeMissing::
	.string "GIOVANNI: The SILPH SCOPE is\n"
	.string "still waiting in my hideout.\l"
	.string "Claim it before we battle.$"
ViridianCity_Gym_Text_SilphMissing::
	.string "GIOVANNI: SILPH CO. is still\n"
	.string "occupied. Free it, then return.$"
ViridianCity_Gym_Text_NoUsableParty::
	.string "Bring a POKéMON able to battle\n"
	.string "before you challenge me.$"
ViridianCity_Gym_Text_NoRoomForEarthquake::
	.string "GIOVANNI: Make room in your BAG.\n"
	.string "Your EARTHQUAKE TM awaits.$"
ViridianCity_Gym_Text_GymGuyEarlyWayfarer::
	.string "Yo! Champ in the making!\p"
	.string "The LEADER is waiting on matters\n"
	.string "in CELADON and SILPH CO.\p"
	.string "You can still explore this GYM's\n"
	.string "arrow maze and challenge us.$"
ViridianCity_Gym_Text_GymGuyReadyWayfarer::
	.string "The LEADER is ready now!\n"
	.string "Watch the floor arrows and keep\l"
	.string "a GROUND-type counter handy.$"
ViridianCity_Gym_Text_GymGuyRewardPending::
	.string "You won the EARTHBADGE!\n"
	.string "GIOVANNI still has a TM for you.$"
ViridianCity_Gym_Text_GymGuyPostVictoryWayfarer::
	.string "GIOVANNI has left, but this\n"
	.string "GYM and its maze remain open.$"
ViridianCity_Gym_Text_GymStatueWayfarer::
	.string "VIRIDIAN POKéMON GYM\n"
	.string "LEADER: GIOVANNI$"
ViridianCity_Gym_Text_GymStatuePlayerWonWayfarer::
	.string "VIRIDIAN POKéMON GYM\n"
	.string "LEADER: GIOVANNI\p"
	.string "WINNING TRAINER: {PLAYER}$"
#endif

ViridianCity_Gym_Text_GiovanniIntro::
	.string "Fwahahaha!\n"
	.string "Welcome to my hideout!\p"
	.string "It shall be so until I can restore\n"
	.string "TEAM ROCKET to its former glory.\p"
	.string "But, you have found me again.\n"
	.string "So be it.\l"
	.string "This time, I'm not holding back!\p"
	.string "Once more, you shall face\n"
	.string "GIOVANNI, the greatest TRAINER!{PLAY_BGM}{MUS_RG_ENCOUNTER_ROCKET}$"

ViridianCity_Gym_Text_GiovanniDefeat::
	.string "Ha!\n"
	.string "That was a truly intense fight.\l"
	.string "You have won!\p"
	.string "As proof, here is the EARTHBADGE!\n"
	.string "{PAUSE_MUSIC}{PLAY_BGM}{MUS_OBTAIN_BADGE}{PAUSE 0xFE}{PAUSE 0x56}{RESUME_MUSIC}$"

ViridianCity_Gym_Text_GiovanniPostBattle::
	.string "Having lost in this fashion, \n"
	.string "I can't face my followers.\l"
	.string "I have betrayed their trust.\p"
	.string "As of today, TEAM ROCKET is\n"
	.string "finished forever!\p"
	.string "As for myself, I shall dedicate\n"
	.string "my life to training again.\p"
	.string "Let us meet again someday!\n"
	.string "Farewell!$"

ViridianCity_Gym_Text_ExplainEarthBadgeTakeThis::
	.string "The EARTHBADGE makes POKéMON of\n"
	.string "any level obey without question.\p"
	.string "It is evidence of your mastery as\n"
	.string "a POKéMON TRAINER.\p"
	.string "With it, you can challenge the\n"
	.string "POKéMON LEAGUE.\p"
	.string "Also, take this TM.\p"
	.string "Consider it a gift for your POKéMON\n"
	.string "LEAGUE challenge.$"

ViridianCity_Gym_Text_ReceivedTM26FromGiovanni::
	.string "{PLAYER} received TM26\n"
	.string "from GIOVANNI.$"

ViridianCity_Gym_Text_ExplainTM26::
	.string "TM26 contains EARTHQUAKE.\p"
	.string "It is a powerful attack that causes\n"
	.string "a massive tremor.\p"
	.string "I made it when I ran the GYM here,\n"
	.string "far too long ago… $"

ViridianCity_Gym_Text_YouDoNotHaveSpace::
	.string "You do not have space for this!$"

ViridianCity_Gym_Text_YujiIntro::
	.string "Heh!\n"
	.string "You must be running out of steam\l"
	.string "by now!$"

ViridianCity_Gym_Text_YujiDefeat::
	.string "I ran out of gas!$"

ViridianCity_Gym_Text_YujiPostBattle::
	.string "You'll need power to keep up with\n"
	.string "our GYM LEADER.$"

ViridianCity_Gym_Text_AtsushiIntro::
	.string "Rrrroar!\n"
	.string "I'm working myself into a rage!$"

ViridianCity_Gym_Text_AtsushiDefeat::
	.string "Wargh!$"

ViridianCity_Gym_Text_AtsushiPostBattle::
	.string "I'm still not worthy!$"

ViridianCity_Gym_Text_JasonIntro::
	.string "POKéMON and I, we make wonderful\n"
	.string "music together!$"

ViridianCity_Gym_Text_JasonDefeat::
	.string "You are in perfect harmony!$"

ViridianCity_Gym_Text_JasonPostBattle::
	.string "Do you know the identity of our\n"
	.string "GYM LEADER?$"

ViridianCity_Gym_Text_KiyoIntro::
	.string "Karate is the ultimate form of\n"
	.string "martial arts!$"

ViridianCity_Gym_Text_KiyoDefeat::
	.string "Aiyah!$"

ViridianCity_Gym_Text_KiyoPostBattle::
	.string "If my POKéMON were as good at\n"
	.string "karate as I…$"

ViridianCity_Gym_Text_WarrenIntro::
	.string "The truly talented win with style.$"

ViridianCity_Gym_Text_WarrenDefeat::
	.string "I lost my grip!$"

ViridianCity_Gym_Text_WarrenPostBattle::
	.string "The LEADER will scold me for\n"
	.string "losing this way…$"

ViridianCity_Gym_Text_TakashiIntro::
	.string "I'm the KARATE KING!\n"
	.string "Your fate rests with me!$"

ViridianCity_Gym_Text_TakashiDefeat::
	.string "Ayah!$"

ViridianCity_Gym_Text_TakashiPostBattle::
	.string "The POKéMON LEAGUE?\n"
	.string "You? Don't get cocky!$"

ViridianCity_Gym_Text_ColeIntro::
	.string "Your POKéMON will cower at the\n"
	.string "crack of my whip!$"

ViridianCity_Gym_Text_ColeDefeat::
	.string "Yowch!\n"
	.string "Whiplash!$"

ViridianCity_Gym_Text_ColePostBattle::
	.string "Wait!\n"
	.string "I was just careless!$"

ViridianCity_Gym_Text_SamuelIntro::
	.string "VIRIDIAN GYM was closed for a\n"
	.string "long time.\p"
	.string "But now, our LEADER is back!$"

ViridianCity_Gym_Text_SamuelDefeat::
	.string "I was beaten?$"

ViridianCity_Gym_Text_SamuelPostBattle::
	.string "You can go on to the POKéMON\n"
	.string "LEAGUE only by defeating our GYM\l"
	.string "LEADER!$"

ViridianCity_Gym_Text_GymGuyAdvice::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "Even I don't know the VIRIDIAN\n"
	.string "LEADER's identity.\p"
	.string "But one thing's certain.\n"
	.string "This will be the toughest of all\l"
	.string "the GYM LEADERS.\p"
	.string "Also, I heard that the TRAINERS\n"
	.string "here like GROUND-type POKéMON.$"

ViridianCity_Gym_Text_GymGuyPostVictory::
	.string "Blow me away! GIOVANNI was the\n"
	.string "GYM LEADER of VIRIDIAN?$"

#if !IS_WAYFARER
ViridianCity_Gym_Text_GymStatue::
	.string "VIRIDIAN POKéMON GYM\n"
	.string "LEADER: ?\p"
	.string "WINNING TRAINERS:\n"
	.string "{RIVAL}$"

ViridianCity_Gym_Text_GymStatuePlayerWon::
	.string "VIRIDIAN POKéMON GYM\n"
	.string "LEADER: GIOVANNI\p"
	.string "WINNING TRAINERS:\n"
	.string "{RIVAL}, {PLAYER}$"
#endif
