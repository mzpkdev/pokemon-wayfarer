SilphCo_2F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_2F_OnLoad
	.byte 0

SilphCo_2F_OnLoad::
	call_if_unset FLAG_SILPH_2F_DOOR_1, EventScript_Close2FDoor1
	call_if_unset FLAG_SILPH_2F_DOOR_2, EventScript_Close2FDoor2
	end

SilphCo_2F_EventScript_ThunderWaveTutor::
	goto EventScript_ThunderWaveTutor
	end

SilphCo_2F_EventScript_FloorSign::
	msgbox SilphCo_2F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_2F_EventScript_Connor::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_CONNOR_HNS, SilphCo_2F_EventScript_Connor_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_2F_EventScript_Connor_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_CONNOR_HNS, SilphCo_2F_Text_ConnorIntro, SilphCo_2F_Text_ConnorDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_2F_EventScript_Connor_PostDialogue::
	msgbox SilphCo_2F_Text_ConnorPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_2F_EventScript_Jerry::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_JERRY_HNS, SilphCo_2F_EventScript_Jerry_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_2F_EventScript_Jerry_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_JERRY_HNS, SilphCo_2F_Text_JerryIntro, SilphCo_2F_Text_JerryDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_2F_EventScript_Jerry_PostDialogue::
	msgbox SilphCo_2F_Text_JerryPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_2F_EventScript_Grunt1::
	goto_if_defeated TRAINER_SILPH_GRUNT_23_HNS, SilphCo_2F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_2F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_23_HNS, SilphCo_2F_Text_Grunt1Intro, SilphCo_2F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_2F_EventScript_Grunt1_PostDialogue::
	msgbox SilphCo_2F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_2F_EventScript_Grunt2::
	goto_if_defeated TRAINER_SILPH_GRUNT_24_HNS, SilphCo_2F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_2F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_24_HNS, SilphCo_2F_Text_Grunt2Intro, SilphCo_2F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_2F_EventScript_Grunt2_PostDialogue::
	msgbox SilphCo_2F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_2F_Text_ConnorIntro::
	.string "Help!\n"
	.string "I'm a SILPH employee.$"

SilphCo_2F_Text_ConnorDefeat::
	.string "How did you know\n"
	.string "I was a ROCKET?$"

SilphCo_2F_Text_ConnorPostBattle::
	.string "I work for both SILPH and TEAM\n"
	.string "ROCKET.$"

SilphCo_2F_Text_JerryIntro::
	.string "It's off-limits here!\n"
	.string "Go home!$"

SilphCo_2F_Text_JerryDefeat::
	.string "You're good.$"

SilphCo_2F_Text_JerryPostBattle::
	.string "Can you solve the maze in here?\n"
	.string "It's not easy, I tell you!$"

SilphCo_2F_Text_Grunt1Intro::
	.string "No kids are allowed in here!$"

SilphCo_2F_Text_Grunt1Defeat::
	.string "Tough!$"

SilphCo_2F_Text_Grunt1PostBattle::
	.string "Diamond-shaped tiles are teleport\n"
	.string "blocks.\p"
	.string "They're transporters for this\n"
	.string "high-tech building.$"

SilphCo_2F_Text_Grunt2Intro::
	.string "Hey, kid!\n"
	.string "What are you wandering here for?$"

SilphCo_2F_Text_Grunt2Defeat::
	.string "I goofed!$"

SilphCo_2F_Text_Grunt2PostBattle::
	.string "SILPH CO. will be associated with\n"
	.string "TEAM ROCKET!$"

Text_ThunderWaveTeach::
	.string "Eeek!\n"
	.string "No! Stop! Help!\p"
	.string "Oh, you're not with TEAM ROCKET.\n"
	.string "I'm sorry, I thought…\p"
	.string "Will you forgive me if I teach\n"
	.string "you the THUNDER WAVE technique?$"

Text_ThunderWaveDeclined::
	.string "Oh…\n"
	.string "But THUNDER WAVE is convenient…$"

Text_ThunderWaveWhichMon::
	.string "Which POKéMON should I teach\n"
	.string "THUNDER WAVE?$"

Text_ThunderWaveTaught::
	.string "THUNDER WAVE causes paralysis in\n"
	.string "the target POKéMON.\p"
	.string "A paralyzed POKéMON may be unable\n"
	.string "to move in battle.\p"
	.string "It's an ELECTRIC-type move, so it\n"
	.string "won't work on some POKéMON.$"

SilphCo_2F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "2F$"


SilphCo_3F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_3F_OnLoad
	.byte 0

SilphCo_3F_OnLoad::
	call_if_unset FLAG_SILPH_3F_DOOR_1, EventScript_Close3FDoor1
	call_if_unset FLAG_SILPH_3F_DOOR_2, EventScript_Close3FDoor2
	end

SilphCo_3F_EventScript_WorkerM::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_3F_EventScript_WorkerMRocketsGone
	msgbox SilphCo_3F_Text_WhatAmIToDo
	release
	end

SilphCo_3F_EventScript_WorkerMRocketsGone::
	msgbox SilphCo_3F_Text_YouAndYourMonsSavedUs
	release
	end

SilphCo_3F_EventScript_FloorSign::
	msgbox SilphCo_3F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_3F_EventScript_Jose::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_JOSE_HNS, SilphCo_3F_EventScript_Jose_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_3F_EventScript_Jose_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_JOSE_HNS, SilphCo_3F_Text_JoseIntro, SilphCo_3F_Text_JoseDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_3F_EventScript_Jose_PostDialogue::
	msgbox SilphCo_3F_Text_JosePostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_3F_EventScript_Grunt::
	goto_if_defeated TRAINER_SILPH_GRUNT_25_HNS, SilphCo_3F_EventScript_Grunt_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_3F_EventScript_Grunt_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_25_HNS, SilphCo_3F_Text_GruntIntro, SilphCo_3F_Text_GruntDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_3F_EventScript_Grunt_PostDialogue::
	msgbox SilphCo_3F_Text_GruntPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_3F_Text_WhatAmIToDo::
	.string "I work for SILPH CO.\n"
	.string "What am I to do?$"

SilphCo_3F_Text_YouAndYourMonsSavedUs::
	.string "{PLAYER}!\n"
	.string "You and your POKéMON saved us!$"

SilphCo_3F_Text_GruntIntro::
	.string "Quit messing with us, kid!$"

SilphCo_3F_Text_GruntDefeat::
	.string "I give up!$"

SilphCo_3F_Text_GruntPostBattle::
	.string "A hint? You can open doors with\n"
	.string "a CARD KEY!$"

SilphCo_3F_Text_JoseIntro::
	.string "I support TEAM ROCKET more than\n"
	.string "I support SILPH CO.!$"

SilphCo_3F_Text_JoseDefeat::
	.string "You really got me!$"

SilphCo_3F_Text_JosePostBattle::
	.string "Humph…\p"
	.string "TEAM ROCKET said that if I helped\n"
	.string "them, they'd let me study POKéMON.$"

SilphCo_3F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "3F$"


SilphCo_4F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_4F_OnLoad
	.byte 0

SilphCo_4F_OnLoad::
	call_if_unset FLAG_SILPH_4F_DOOR_1, EventScript_Close4FDoor1
	call_if_unset FLAG_SILPH_4F_DOOR_2, EventScript_Close4FDoor2
	end

SilphCo_4F_EventScript_WorkerM::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_4F_EventScript_WorkerMRocketsGone
	msgbox SilphCo_4F_Text_CantYouSeeImHiding
	release
	end

SilphCo_4F_EventScript_WorkerMRocketsGone::
	msgbox SilphCo_4F_Text_TeamRocketIsGone
	release
	end

SilphCo_4F_EventScript_FloorSign::
	msgbox SilphCo_4F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_4F_EventScript_Rodney::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_RODNEY_HNS, SilphCo_4F_EventScript_Rodney_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_4F_EventScript_Rodney_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_RODNEY_HNS, SilphCo_4F_Text_RodneyIntro, SilphCo_4F_Text_RodneyDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_4F_EventScript_Rodney_PostDialogue::
	msgbox SilphCo_4F_Text_RodneyPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_4F_EventScript_Grunt1::
	goto_if_defeated TRAINER_SILPH_GRUNT_26_HNS, SilphCo_4F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_4F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_26_HNS, SilphCo_4F_Text_Grunt1Intro, SilphCo_4F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_4F_EventScript_Grunt1_PostDialogue::
	msgbox SilphCo_4F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_4F_EventScript_Grunt2::
	goto_if_defeated TRAINER_SILPH_GRUNT_27_HNS, SilphCo_4F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_4F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_27_HNS, SilphCo_4F_Text_Grunt2Intro, SilphCo_4F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_4F_EventScript_Grunt2_PostDialogue::
	msgbox SilphCo_4F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_4F_Text_CantYouSeeImHiding::
	.string "Sssh!\n"
	.string "Can't you see I'm hiding?$"

SilphCo_4F_Text_TeamRocketIsGone::
	.string "Huh?\n"
	.string "TEAM ROCKET is gone?$"

SilphCo_4F_Text_Grunt1Intro::
	.string "TEAM ROCKET has taken control\n"
	.string "of SILPH CO.!$"

SilphCo_4F_Text_Grunt1Defeat::
	.string "Arrgh!$"

SilphCo_4F_Text_Grunt1PostBattle::
	.string "Fwahahaha!\n"
	.string "My BOSS has been after this place!$"

SilphCo_4F_Text_RodneyIntro::
	.string "My POKéMON is my loyal servant.$"

SilphCo_4F_Text_RodneyDefeat::
	.string "Darn!\n"
	.string "You weak POKéMON!$"

SilphCo_4F_Text_RodneyPostBattle::
	.string "The doors are electronically\n"
	.string "locked.\p"
	.string "It takes a CARD KEY to open the\n"
	.string "doors here.$"

SilphCo_4F_Text_Grunt2Intro::
	.string "Intruder spotted!$"

SilphCo_4F_Text_Grunt2Defeat::
	.string "Who are you?$"

SilphCo_4F_Text_Grunt2PostBattle::
	.string "I better tell the BOSS on 11F!$"

SilphCo_4F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "4F$"


SilphCo_5F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_5F_OnLoad
	.byte 0

SilphCo_5F_OnLoad::
	call_if_unset FLAG_SILPH_5F_DOOR_1, EventScript_Close5FDoor1
	call_if_unset FLAG_SILPH_5F_DOOR_2, EventScript_Close5FDoor2
	call_if_unset FLAG_SILPH_5F_DOOR_3, EventScript_Close5FDoor3
	end

SilphCo_5F_EventScript_WorkerM::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_5F_EventScript_WorkerMRocketsGone
	msgbox SilphCo_5F_Text_RocketsInUproarAboutIntruder
	release
	end

SilphCo_5F_EventScript_WorkerMRocketsGone::
	msgbox SilphCo_5F_Text_YoureOurHeroThankYou
	release
	end

SilphCo_5F_EventScript_PokemonReport1::
	msgbox SilphCo_5F_Text_PorygonFirstVRMon, MSGBOX_SIGN
	end

SilphCo_5F_EventScript_PokemonReport2::
	msgbox SilphCo_5F_Text_Over350TechniquesConfirmed, MSGBOX_SIGN
	end

SilphCo_5F_EventScript_PokemonReport3::
	msgbox SilphCo_5F_Text_SomeMonsEvolveWhenTraded, MSGBOX_SIGN
	end

SilphCo_5F_EventScript_FloorSign::
	msgbox SilphCo_5F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_5F_EventScript_Beau::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_BEAU_HNS, SilphCo_5F_EventScript_Beau_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_5F_EventScript_Beau_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_BEAU_HNS, SilphCo_5F_Text_BeauIntro, SilphCo_5F_Text_BeauDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_5F_EventScript_Beau_PostDialogue::
	msgbox SilphCo_5F_Text_BeauPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_5F_EventScript_Grunt1::
	goto_if_defeated TRAINER_SILPH_GRUNT_28_HNS, SilphCo_5F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_5F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_28_HNS, SilphCo_5F_Text_Grunt1Intro, SilphCo_5F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_5F_EventScript_Grunt1_PostDialogue::
	msgbox SilphCo_5F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_5F_EventScript_Grunt2::
	goto_if_defeated TRAINER_SILPH_GRUNT_29_HNS, SilphCo_5F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_5F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_29_HNS, SilphCo_5F_Text_Grunt2Intro, SilphCo_5F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_5F_EventScript_Grunt2_PostDialogue::
	msgbox SilphCo_5F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_5F_EventScript_Dalton::
	goto_if_defeated TRAINER_SILPH_DALTON_HNS, SilphCo_5F_EventScript_Dalton_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_5F_EventScript_Dalton_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_DALTON_HNS, SilphCo_5F_Text_DaltonIntro, SilphCo_5F_Text_DaltonDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_5F_EventScript_Dalton_PostDialogue::
	msgbox SilphCo_5F_Text_DaltonPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_5F_Text_RocketsInUproarAboutIntruder::
	.string "TEAM ROCKET is in an uproar over\n"
	.string "some intruder.\p"
	.string "That's you, right?$"

SilphCo_5F_Text_YoureOurHeroThankYou::
	.string "TEAM ROCKET took off!\n"
	.string "You're our hero! Thank you!$"

SilphCo_5F_Text_Grunt1Intro::
	.string "I heard a kid was wandering\n"
	.string "around.$"

SilphCo_5F_Text_Grunt1Defeat::
	.string "Boom!$"

SilphCo_5F_Text_Grunt1PostBattle::
	.string "It's not smart to pick a fight with\n"
	.string "TEAM ROCKET!$"

SilphCo_5F_Text_BeauIntro::
	.string "We study POKé BALL technology on\n"
	.string "this floor.$"

SilphCo_5F_Text_BeauDefeat::
	.string "Dang!\n"
	.string "Blast it!$"

SilphCo_5F_Text_BeauPostBattle::
	.string "We worked on the ultimate POKé\n"
	.string "BALL which would catch anything.$"

SilphCo_5F_Text_DaltonIntro::
	.string "Whaaat?\p"
	.string "There shouldn't be any children\n"
	.string "here!$"

SilphCo_5F_Text_DaltonDefeat::
	.string "Oh, goodness!$"

SilphCo_5F_Text_DaltonPostBattle::
	.string "You're only on 5F.\n"
	.string "It's a long way to my BOSS!$"

SilphCo_5F_Text_Grunt2Intro::
	.string "Show TEAM ROCKET a little respect!$"

SilphCo_5F_Text_Grunt2Defeat::
	.string "Cough…\n"
	.string "Cough…$"

SilphCo_5F_Text_Grunt2PostBattle::
	.string "Which reminds me.\p"
	.string "KOFFING evolves into WEEZING!$"

SilphCo_5F_Text_PorygonFirstVRMon::
	.string "It's a POKéMON REPORT!\p"
	.string "POKéMON LAB created PORYGON,\n"
	.string "the first virtual-reality POKéMON.$"

SilphCo_5F_Text_Over350TechniquesConfirmed::
	.string "It's a POKéMON REPORT!\p"
	.string "Over 350 POKéMON techniques have\n"
	.string "been confirmed.$"

SilphCo_5F_Text_SomeMonsEvolveWhenTraded::
	.string "It's a POKéMON REPORT!\p"
	.string "Some POKéMON have been confirmed\n"
	.string "to evolve when they are traded.$"

SilphCo_5F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "5F$"

SilphCo_5F_Text_RocketBossLookingForStrongMons::
	.string "Those thugs that took over our\n"
	.string "building…\p"
	.string "Their BOSS said he was looking for\n"
	.string "strong POKéMON.\p"
	.string "I hope our PRESIDENT managed to\n"
	.string "avoid trouble…$"

SilphCo_6F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_6F_OnLoad
	.byte 0

SilphCo_6F_OnLoad::
	call_if_unset FLAG_SILPH_6F_DOOR, EventScript_Close6FDoor
	end

SilphCo_6F_EventScript_WorkerM2::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_6F_EventScript_WorkerM2RocketsGone
	msgbox SilphCo_6F_Text_TargetedSilphForOurMonProducts
	release
	end

SilphCo_6F_EventScript_WorkerM2RocketsGone::
	msgbox SilphCo_6F_Text_ComeWorkForSilphWhenYoureOlder
	release
	end

SilphCo_6F_EventScript_WorkerM3::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_6F_EventScript_WorkerM3RocketsGone
	msgbox SilphCo_6F_Text_RocketsTookOverBuilding
	release
	end

SilphCo_6F_EventScript_WorkerM3RocketsGone::
	msgbox SilphCo_6F_Text_BetterGetBackToWork
	release
	end

SilphCo_6F_EventScript_WorkerM1::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_6F_EventScript_WorkerM1RocketsGone
	msgbox SilphCo_6F_Text_HelpMePlease
	release
	end

SilphCo_6F_EventScript_WorkerM1RocketsGone::
	msgbox SilphCo_6F_Text_WeGotEngaged
	release
	end

SilphCo_6F_EventScript_WorkerF1::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_6F_EventScript_WorkerF1RocketsGone
	msgbox SilphCo_6F_Text_ThatManIsSuchACoward
	release
	end

SilphCo_6F_EventScript_WorkerF1RocketsGone::
	msgbox SilphCo_6F_Text_NeedsMeToLookAfterHim
	release
	end

SilphCo_6F_EventScript_WorkerF2::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_6F_EventScript_WorkerF2RocketsGone
	msgbox SilphCo_6F_Text_RocketsTryingToConquerWorld
	release
	end

SilphCo_6F_EventScript_WorkerF2RocketsGone::
	msgbox SilphCo_6F_Text_RocketsRanAwayBecauseOfYou
	release
	end

SilphCo_6F_EventScript_FloorSign::
	msgbox SilphCo_6F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_6F_EventScript_Taylor::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_TAYLOR_HNS, SilphCo_6F_EventScript_Taylor_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_6F_EventScript_Taylor_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_TAYLOR_HNS, SilphCo_6F_Text_TaylorIntro, SilphCo_6F_Text_TaylorDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_6F_EventScript_Taylor_PostDialogue::
	msgbox SilphCo_6F_Text_TaylorPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_6F_EventScript_Grunt1::
	goto_if_defeated TRAINER_SILPH_GRUNT_30_HNS, SilphCo_6F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_6F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_30_HNS, SilphCo_6F_Text_Grunt1Intro, SilphCo_6F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_6F_EventScript_Grunt1_PostDialogue::
	msgbox SilphCo_6F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_6F_EventScript_Grunt2::
	goto_if_defeated TRAINER_SILPH_GRUNT_31_HNS, SilphCo_6F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_6F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_31_HNS, SilphCo_6F_Text_Grunt2Intro, SilphCo_6F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_6F_EventScript_Grunt2_PostDialogue::
	msgbox SilphCo_6F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_6F_Text_RocketsTookOverBuilding::
	.string "The ROCKETS came and took over\n"
	.string "the building!$"

SilphCo_6F_Text_BetterGetBackToWork::
	.string "Well, better get back to work.$"

SilphCo_6F_Text_HelpMePlease::
	.string "Oh dear, oh dear.\n"
	.string "Help me, please!$"

SilphCo_6F_Text_WeGotEngaged::
	.string "We got engaged.\n"
	.string "Heheh!$"

SilphCo_6F_Text_ThatManIsSuchACoward::
	.string "That man next to me…\n"
	.string "He's such a coward!$"

SilphCo_6F_Text_NeedsMeToLookAfterHim::
	.string "He's so helpless, he needed\n"
	.string "someone like me to look after him.$"

SilphCo_6F_Text_RocketsTryingToConquerWorld::
	.string "TEAM ROCKET is trying to conquer\n"
	.string "the world with POKéMON.$"

SilphCo_6F_Text_RocketsRanAwayBecauseOfYou::
	.string "TEAM ROCKET ran away, and it's all\n"
	.string "because of you!$"

SilphCo_6F_Text_TargetedSilphForOurMonProducts::
	.string "They must have targeted SILPH for\n"
	.string "our POKéMON products.$"

SilphCo_6F_Text_ComeWorkForSilphWhenYoureOlder::
	.string "Come work for SILPH when you\n"
	.string "get older.$"

SilphCo_6F_Text_Grunt1Intro::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

SilphCo_6F_Text_Grunt1Defeat::
	.string "Flame out!$"

SilphCo_6F_Text_Grunt1PostBattle::
	.string "No matter!\n"
	.string "My brothers will avenge me!$"

SilphCo_6F_Text_TaylorIntro::
	.string "That rotten PRESIDENT!\p"
	.string "Serves him right for shipping me\n"
	.string "off to the TIKSI BRANCH!\p"
	.string "That's why TEAM ROCKET came after\n"
	.string "us, I'm sure of it!$"

SilphCo_6F_Text_TaylorDefeat::
	.string "Shoot!$"

SilphCo_6F_Text_TaylorPostBattle::
	.string "TIKSI BRANCH?\n"
	.string "It's in Russian no-man's-land!$"

SilphCo_6F_Text_Grunt2Intro::
	.string "You dare betray TEAM ROCKET?$"

SilphCo_6F_Text_Grunt2Defeat::
	.string "You traitor!$"

SilphCo_6F_Text_Grunt2PostBattle::
	.string "If you stand for justice, you\n"
	.string "betray us bad guys!$"

SilphCo_6F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "6F$"


SilphCo_7F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_7F_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, SilphCo_7F_OnTransition
	.byte 0

SilphCo_7F_OnLoad::
	call_if_unset FLAG_SILPH_7F_DOOR_1, EventScript_Close7FDoor1
	call_if_unset FLAG_SILPH_7F_DOOR_2, EventScript_Close7FDoor2
	call_if_unset FLAG_SILPH_7F_DOOR_3, EventScript_Close7FDoor3
	end

SilphCo_7F_OnTransition::
	call_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_7F_EventScript_SetObjRocketsGone
	end

SilphCo_7F_EventScript_SetObjRocketsGone::
	setobjectmovementtype LOCALID_SILPH_CO_7F_WORKER_M2, MOVEMENT_TYPE_FACE_UP
	return

SilphCo_7F_EventScript_LaprasGuy::
	lock
	faceplayer
	goto_if_set FLAG_GOT_LAPRAS_FROM_SILPH, SilphCo_7F_EventScript_AlreadyGotLapras
	msgbox SilphCo_7F_Text_HaveMonForSavingUs
	setvar VAR_TEMP_1, SPECIES_LAPRAS
	givemon SPECIES_LAPRAS, 25
	goto_if_eq VAR_RESULT, 0, SilphCo_7F_EventScript_ReceiveLaprasParty
	goto_if_eq VAR_RESULT, 1, SilphCo_7F_EventScript_ReceiveLaprasPC
	goto_if_eq VAR_RESULT, 2, Common_EventScript_NoMoreRoomForPokemon
	release
	end

SilphCo_7F_EventScript_ReceiveLaprasParty::
	textcolor NPC_TEXT_COLOR_NEUTRAL
	playfanfare MUS_LEVEL_UP
	message SilphCo_7F_Text_ObtainedLaprasFromEmployee
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_LAPRAS
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SilphCo_7F_EventScript_EndReceiveLapras
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto SilphCo_7F_EventScript_EndReceiveLapras
	end

SilphCo_7F_EventScript_ReceiveLaprasPC::
	textcolor NPC_TEXT_COLOR_NEUTRAL
	playfanfare MUS_LEVEL_UP
	message SilphCo_7F_Text_ObtainedLaprasFromEmployee
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_LAPRAS
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SilphCo_7F_EventScript_LaprasTransferredToPC
	call Common_EventScript_NameReceivedBoxMon
	goto SilphCo_7F_EventScript_LaprasTransferredToPC
	end

SilphCo_7F_EventScript_LaprasTransferredToPC::
	call Common_EventScript_TransferredToPC
	goto SilphCo_7F_EventScript_EndReceiveLapras
	end

SilphCo_7F_EventScript_EndReceiveLapras::
	textcolor NPC_TEXT_COLOR_MALE
	msgbox SilphCo_7F_Text_ExplainLapras
	setflag FLAG_GOT_LAPRAS_FROM_SILPH
	release
	end

SilphCo_7F_EventScript_AlreadyGotLapras::
	msgbox SilphCo_7F_Text_RocketBossWentToBoardroom
	release
	end

SilphCo_7F_EventScript_WorkerM1::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_7F_EventScript_WorkerM1RocketsGone
	msgbox SilphCo_7F_Text_RocketsAfterMasterBall
	release
	end

SilphCo_7F_EventScript_WorkerM1RocketsGone::
	msgbox SilphCo_7F_Text_CanceledMasterBallProject
	release
	end

SilphCo_7F_EventScript_WorkerM2::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_7F_EventScript_WorkerM2RocketsGone
	msgbox SilphCo_7F_Text_BadIfTeamRocketTookOver
	release
	end

SilphCo_7F_EventScript_WorkerM2RocketsGone::
	msgbox SilphCo_7F_Text_WowYouChasedOffTeamRocket
	release
	end

SilphCo_7F_EventScript_WorkerF::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_7F_EventScript_WorkerFRocketsGone
	msgbox SilphCo_7F_Text_ReallyDangerousHere
	release
	end

SilphCo_7F_EventScript_WorkerFRocketsGone::
	msgbox SilphCo_7F_Text_ThankYouSoMuch
	release
	end

SilphCo_7F_EventScript_FloorSign::
	msgbox SilphCo_7F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_7F_EventScript_Joshua::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_JOSHUA_HNS, SilphCo_7F_EventScript_Joshua_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_7F_EventScript_Joshua_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_JOSHUA_HNS, SilphCo_7F_Text_JoshuaIntro, SilphCo_7F_Text_JoshuaDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_7F_EventScript_Joshua_PostDialogue::
	msgbox SilphCo_7F_Text_JoshuaPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_7F_EventScript_Grunt1::
	goto_if_defeated TRAINER_SILPH_GRUNT_33_HNS, SilphCo_7F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_7F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_33_HNS, SilphCo_7F_Text_Grunt1Intro, SilphCo_7F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_7F_EventScript_Grunt1_PostDialogue::
	msgbox SilphCo_7F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_7F_EventScript_Grunt2::
	goto_if_defeated TRAINER_SILPH_GRUNT_34_HNS, SilphCo_7F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_7F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_34_HNS, SilphCo_7F_Text_Grunt2Intro, SilphCo_7F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_7F_EventScript_Grunt2_PostDialogue::
	msgbox SilphCo_7F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_7F_EventScript_Grunt3::
	goto_if_defeated TRAINER_SILPH_GRUNT_35_HNS, SilphCo_7F_EventScript_Grunt3_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_7F_EventScript_Grunt3_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_35_HNS, SilphCo_7F_Text_Grunt3Intro, SilphCo_7F_Text_Grunt3Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_7F_EventScript_Grunt3_PostDialogue::
	msgbox SilphCo_7F_Text_Grunt3PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_7F_Text_HaveMonForSavingUs::
	.string "Oh! Hi! You're not a ROCKET!\n"
	.string "You came to save us?\l"
	.string "Why, thank you!\p"
	.string "I want you to have this POKéMON\n"
	.string "for saving us.$"

SilphCo_7F_Text_ObtainedLaprasFromEmployee::
	.string "{PLAYER} obtained a LAPRAS from\n"
	.string "the SILPH employee!$"

SilphCo_7F_Text_ExplainLapras::
	.string "It's a LAPRAS.\n"
	.string "It's a very intelligent POKéMON.\p"
	.string "We kept it in our lab, but it will\n"
	.string "be much better off with you.\p"
	.string "I think you will be a good TRAINER\n"
	.string "for LAPRAS!\p"
	.string "It's a good swimmer.\n"
	.string "It'll give you a lift across water!$"

SilphCo_7F_Text_RocketBossWentToBoardroom::
	.string "TEAM ROCKET's BOSS went to the\n"
	.string "boardroom!\p"
	.string "Is our PRESIDENT okay?\n"
	.string "I'm worried.$"

SilphCo_7F_Text_RocketsAfterMasterBall::
	.string "TEAM ROCKET was after the MASTER\n"
	.string "BALL, which catches any POKéMON.$"

SilphCo_7F_Text_CanceledMasterBallProject::
	.string "We canceled the MASTER BALL\n"
	.string "project because of TEAM ROCKET.$"

SilphCo_7F_Text_BadIfTeamRocketTookOver::
	.string "It would be bad if TEAM ROCKET\n"
	.string "took over SILPH or our POKéMON.$"

SilphCo_7F_Text_WowYouChasedOffTeamRocket::
	.string "Wow!\p"
	.string "You chased off TEAM ROCKET all by\n"
	.string "yourself?$"

SilphCo_7F_Text_ReallyDangerousHere::
	.string "You!\n"
	.string "It's really dangerous here!\p"
	.string "You came to save me?\n"
	.string "You can't!$"

SilphCo_7F_Text_ThankYouSoMuch::
	.string "Thank you so much!$"

SilphCo_7F_Text_Grunt3Intro::
	.string "Uh-oh!\n"
	.string "I smell a little mouse!$"

SilphCo_7F_Text_Grunt3Defeat::
	.string "Lights out!$"

SilphCo_7F_Text_Grunt3PostBattle::
	.string "You won't find my BOSS by just\n"
	.string "scurrying around!$"

SilphCo_7F_Text_JoshuaIntro::
	.string "Heheh!\p"
	.string "You mistook me for a SILPH worker?$"

SilphCo_7F_Text_JoshuaDefeat::
	.string "I'm done!$"

SilphCo_7F_Text_JoshuaPostBattle::
	.string "Despite your age, you are a\n"
	.string "skilled TRAINER!$"

SilphCo_7F_Text_Grunt1Intro::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

SilphCo_7F_Text_Grunt1Defeat::
	.string "Aack!\n"
	.string "Brothers, I lost!$"

SilphCo_7F_Text_Grunt1PostBattle::
	.string "Doesn't matter.\n"
	.string "My brothers will repay the favor!$"

SilphCo_7F_Text_Grunt2Intro::
	.string "A child intruder?\n"
	.string "That must be you!$"

SilphCo_7F_Text_Grunt2Defeat::
	.string "Fine!\n"
	.string "I lost!$"

SilphCo_7F_Text_Grunt2PostBattle::
	.string "Go on home before my BOSS gets\n"
	.string "ticked off!$"

SilphCo_7F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "7F$"


SilphCo_8F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_8F_OnLoad
	.byte 0

SilphCo_8F_OnLoad::
	call_if_unset FLAG_SILPH_8F_DOOR, EventScript_Close8FDoor
	end

SilphCo_8F_EventScript_WorkerM::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_8F_EventScript_WorkerMRocketsGone
	msgbox SilphCo_8F_Text_WonderIfSilphIsFinished
	release
	end

SilphCo_8F_EventScript_WorkerMRocketsGone::
	msgbox SilphCo_8F_Text_ThanksForSavingUs
	release
	end

SilphCo_8F_EventScript_FloorSign::
	msgbox SilphCo_8F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_8F_EventScript_Parker::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_PARKER_HNS, SilphCo_8F_EventScript_Parker_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_8F_EventScript_Parker_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_PARKER_HNS, SilphCo_8F_Text_ParkerIntro, SilphCo_8F_Text_ParkerDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_8F_EventScript_Parker_PostDialogue::
	msgbox SilphCo_8F_Text_ParkerPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_8F_EventScript_Grunt1::
	goto_if_defeated TRAINER_SILPH_GRUNT_32_HNS, SilphCo_8F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_8F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_32_HNS, SilphCo_8F_Text_Grunt1Intro, SilphCo_8F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_8F_EventScript_Grunt1_PostDialogue::
	msgbox SilphCo_8F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_8F_EventScript_Grunt2::
	goto_if_defeated TRAINER_SILPH_GRUNT_36_HNS, SilphCo_8F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_8F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_36_HNS, SilphCo_8F_Text_Grunt2Intro, SilphCo_8F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_8F_EventScript_Grunt2_PostDialogue::
	msgbox SilphCo_8F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_8F_Text_WonderIfSilphIsFinished::
	.string "I wonder if SILPH is finished…$"

SilphCo_8F_Text_ThanksForSavingUs::
	.string "Thanks for saving us!$"

SilphCo_8F_Text_Grunt1Intro::
	.string "That's as far as you'll go!$"

SilphCo_8F_Text_Grunt1Defeat::
	.string "Not enough grit!$"

SilphCo_8F_Text_Grunt1PostBattle::
	.string "If you don't turn back…\n"
	.string "I'll call for backup!$"

SilphCo_8F_Text_ParkerIntro::
	.string "You're causing us problems!$"

SilphCo_8F_Text_ParkerDefeat::
	.string "Huh?\n"
	.string "I lost?$"

SilphCo_8F_Text_ParkerPostBattle::
	.string "So, what do you think of SILPH\n"
	.string "BUILDING's maze?$"

SilphCo_8F_Text_Grunt2Intro::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

SilphCo_8F_Text_Grunt2Defeat::
	.string "Whoo!\n"
	.string "Oh, Brothers!$"

SilphCo_8F_Text_Grunt2PostBattle::
	.string "I'll leave you up to my brothers.$"

SilphCo_8F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "8F$"

SilphCo_8F_Text_ToRocketBossMonsAreTools::
	.string "TEAM ROCKET's BOSS is terribly\n"
	.string "cruel!\p"
	.string "To him, POKéMON are just tools to\n"
	.string "be used.\p"
	.string "What will happen if that tyrant\n"
	.string "takes over our company…$"

SilphCo_9F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_9F_OnLoad
	.byte 0

SilphCo_9F_OnLoad::
	call_if_unset FLAG_SILPH_9F_DOOR_1, EventScript_Close9FDoor1
	call_if_unset FLAG_SILPH_9F_DOOR_2, EventScript_Close9FDoor2
	call_if_unset FLAG_SILPH_9F_DOOR_3, EventScript_Close9FDoor3
	call_if_unset FLAG_SILPH_9F_DOOR_4, EventScript_Close9FDoor4
	end

SilphCo_9F_EventScript_HealWoman::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_9F_EventScript_HealWomanRocketsGone
	msgbox SilphCo_9F_Text_YouShouldTakeQuickNap
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	msgbox SilphCo_9F_Text_DontGiveUp
	release
	end

SilphCo_9F_EventScript_HealWomanRocketsGone::
	msgbox SilphCo_9F_Text_ThankYouSoMuch
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	release
	end

SilphCo_9F_EventScript_FloorSign::
	msgbox SilphCo_9F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_9F_EventScript_Ed::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_ED_HNS, SilphCo_9F_EventScript_Ed_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_9F_EventScript_Ed_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_ED_HNS, SilphCo_9F_Text_EdIntro, SilphCo_9F_Text_EdDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_9F_EventScript_Ed_PostDialogue::
	msgbox SilphCo_9F_Text_EdPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_9F_EventScript_Grunt1::
	goto_if_defeated TRAINER_SILPH_GRUNT_37_HNS, SilphCo_9F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_9F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_37_HNS, SilphCo_9F_Text_Grunt1Intro, SilphCo_9F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_9F_EventScript_Grunt1_PostDialogue::
	msgbox SilphCo_9F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_9F_EventScript_Grunt2::
	goto_if_defeated TRAINER_SILPH_GRUNT_38_HNS, SilphCo_9F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_9F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_38_HNS, SilphCo_9F_Text_Grunt2Intro, SilphCo_9F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_9F_EventScript_Grunt2_PostDialogue::
	msgbox SilphCo_9F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_9F_Text_YouShouldTakeQuickNap::
	.string "You look tired.\n"
	.string "You should take a quick nap.$"

SilphCo_9F_Text_DontGiveUp::
	.string "Don't give up!$"

SilphCo_9F_Text_ThankYouSoMuch::
	.string "Thank you so much.$"

SilphCo_9F_Text_Grunt1Intro::
	.string "Your POKéMON seem to adore you,\n"
	.string "kid!$"

SilphCo_9F_Text_Grunt1Defeat::
	.string "Ghaaah!$"

SilphCo_9F_Text_Grunt1PostBattle::
	.string "If I had started as a TRAINER at\n"
	.string "your age…$"

SilphCo_9F_Text_EdIntro::
	.string "Your POKéMON have weak points!\n"
	.string "I can nail them!$"

SilphCo_9F_Text_EdDefeat::
	.string "You hammered me!$"

SilphCo_9F_Text_EdPostBattle::
	.string "Exploiting weak spots does work.\n"
	.string "Think about type advantages.$"

SilphCo_9F_Text_Grunt2Intro::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

SilphCo_9F_Text_Grunt2Defeat::
	.string "Warg!\n"
	.string "Brothers, I lost!$"

SilphCo_9F_Text_Grunt2PostBattle::
	.string "My brothers will avenge me!$"

SilphCo_9F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "9F$"


SilphCo_10F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_10F_OnLoad
	.byte 0

SilphCo_10F_OnLoad::
	call_if_unset FLAG_SILPH_10F_DOOR, EventScript_Close10FDoor
	end

SilphCo_10F_EventScript_WorkerF::
	lock
	faceplayer
	goto_if_ge VAR_SILPH_GIOVANNI_SCENE_HNS, 1, SilphCo_10F_EventScript_WorkerFRocketsGone
	msgbox SilphCo_10F_Text_WaaaImScared
	release
	end

SilphCo_10F_EventScript_WorkerFRocketsGone::
	msgbox SilphCo_10F_Text_KeepMeCryingASecret
	release
	end

SilphCo_10F_EventScript_FloorSign::
	msgbox SilphCo_10F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_10F_EventScript_Travis::
	goto_if_defeated TRAINER_SILPH_SCIENTIST_TRAVIS_HNS, SilphCo_10F_EventScript_Travis_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OrdinaryNoParty
SilphCo_10F_EventScript_Travis_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_SCIENTIST_TRAVIS_HNS, SilphCo_10F_Text_TravisIntro, SilphCo_10F_Text_TravisDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_10F_EventScript_Travis_PostDialogue::
	msgbox SilphCo_10F_Text_TravisPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_10F_EventScript_Grunt::
	goto_if_defeated TRAINER_SILPH_GRUNT_39_HNS, SilphCo_10F_EventScript_Grunt_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_10F_EventScript_Grunt_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_39_HNS, SilphCo_10F_Text_GruntIntro, SilphCo_10F_Text_GruntDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_10F_EventScript_Grunt_PostDialogue::
	msgbox SilphCo_10F_Text_GruntPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_10F_Text_GruntIntro::
	.string "Welcome to 10F!\n"
	.string "So good of you to join me!$"

SilphCo_10F_Text_GruntDefeat::
	.string "I'm stunned!$"

SilphCo_10F_Text_GruntPostBattle::
	.string "Nice try, but the boardroom is up\n"
	.string "one more floor.$"

SilphCo_10F_Text_TravisIntro::
	.string "Enough of your silly games!$"

SilphCo_10F_Text_TravisDefeat::
	.string "No continues left!$"

SilphCo_10F_Text_TravisPostBattle::
	.string "Are you satisfied with beating me?\n"
	.string "Then go on home!$"

SilphCo_10F_Text_WaaaImScared::
	.string "Waaaaa!\n"
	.string "I'm scared!$"

SilphCo_10F_Text_KeepMeCryingASecret::
	.string "About me crying…\n"
	.string "Please keep it a secret.$"

SilphCo_10F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "10F$"


SilphCo_11F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_11F_OnLoad
	.byte 0

SilphCo_11F_OnLoad::
	call_if_unset FLAG_SILPH_11F_DOOR, EventScript_Close11FDoor
	end

SilphCo_11F_EventScript_President::
	goto_if_set FLAG_SILPH_LIBERATED_HNS, WayfarerSilph_EventScript_PresidentLiberated
	msgbox WayfarerSilph_Text_PresidentOccupied, MSGBOX_NPC
	end
WayfarerSilph_EventScript_PresidentLiberated::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_MASTER_BALL_RECEIVED_HNS, WayfarerSilph_EventScript_PresidentMasterBallReceived
	checkplayergender
	call_if_eq VAR_RESULT, MALE, WayfarerSilph_EventScript_PresidentThanksMale
	call_if_eq VAR_RESULT, FEMALE, WayfarerSilph_EventScript_PresidentThanksFemale
	checkitemspace ITEM_MASTER_BALL
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_PresidentNoRoomForMasterBall
	giveitem_msg SilphCo_11F_Text_ObtainedMasterBallFromPresident, ITEM_MASTER_BALL, 1, MUS_RG_OBTAIN_KEY_ITEM
	msgbox SilphCo_11F_Text_ThatsOurSecretPrototype
	setflag FLAG_SILPH_MASTER_BALL_RECEIVED_HNS
	clearflag FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS
	release
	end

WayfarerSilph_EventScript_PresidentThanksMale::
	msgbox SilphCo_11F_Text_ThanksForSavingMeDearBoy
	return

WayfarerSilph_EventScript_PresidentThanksFemale::
	msgbox SilphCo_11F_Text_ThanksForSavingMeDearGirl
	return

WayfarerSilph_EventScript_PresidentNoRoomForMasterBall::
	msgbox SilphCo_11F_Text_YouHaveNoRoomForThis
	release
	end

WayfarerSilph_EventScript_PresidentMasterBallReceived::
	msgbox SilphCo_11F_Text_ThatsOurSecretPrototype
	release
	end

SilphCo_11F_EventScript_Secretary::
	goto_if_unset FLAG_SILPH_LIBERATED_HNS, WayfarerSilph_EventScript_SecretaryOccupied
	msgbox SilphCo_11F_Text_ThanksForRescuingUs, MSGBOX_NPC
	end

SilphCo_11F_EventScript_GiovanniTriggerLeft::
	lockall
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
	setvar VAR_TEMP_1, 0
	goto SilphCo_11F_EventScript_BattleGiovanni

SilphCo_11F_EventScript_GiovanniTriggerRight::
	lockall
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
	setvar VAR_TEMP_1, 1
	goto SilphCo_11F_EventScript_BattleGiovanni

SilphCo_11F_EventScript_BattleGiovanni::
	textcolor NPC_TEXT_COLOR_MALE
	applymovement LOCALID_SILPH_CO_GIOVANNI, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox WayfarerSilph_Text_GiovanniIntro, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 0, SilphCo_11F_EventScript_GiovanniApproachLeft
	call_if_eq VAR_TEMP_1, 1, SilphCo_11F_EventScript_GiovanniApproachRight
	setvar VAR_LAST_TALKED, LOCALID_SILPH_CO_GIOVANNI
	trainerbattle_no_intro TRAINER_SILPH_GIOVANNI_HNS, WayfarerSilph_Text_GiovanniDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_GiovanniRetry
	msgbox SilphCo_11F_Text_GiovanniPostBattle, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_SILPH_LIBERATED_HNS
	setflag FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS
	setvar VAR_SILPH_GIOVANNI_SCENE_HNS, 1
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_SILPH_CO_GIOVANNI
	removeobject LOCALID_SILPH_CO_11F_GRUNT1
	removeobject LOCALID_SILPH_CO_11F_GRUNT2
	fadescreen FADE_FROM_BLACK
	releaseall
	end

SilphCo_11F_EventScript_GiovanniApproachLeft::
	applymovement LOCALID_SILPH_CO_GIOVANNI, SilphCo_11F_Movement_GiovanniApproachLeft
	applymovement LOCALID_PLAYER, SilphCo_11F_Movement_PlayerFaceGiovanni
	waitmovement 0
	return

SilphCo_11F_EventScript_GiovanniApproachRight::
	applymovement LOCALID_SILPH_CO_GIOVANNI, SilphCo_11F_Movement_GiovanniApproachRight
	waitmovement 0
	return

SilphCo_11F_Movement_GiovanniApproachLeft::
	walk_down
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_left
	step_end

SilphCo_11F_Movement_GiovanniApproachRight::
	walk_down
	walk_down
	walk_down
	step_end

SilphCo_11F_Movement_PlayerFaceGiovanni::
	delay_16
	delay_16
	delay_16
	delay_8
	delay_4
	walk_in_place_faster_right
	step_end

@ Unused
SilphCo_11F_EventScript_Monitor::
	msgbox SilphCo_11F_Text_MonitorHasMonsOnIt, MSGBOX_SIGN
	end

SilphCo_11F_EventScript_FloorSign::
	msgbox SilphCo_11F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_11F_EventScript_Grunt1::
	goto_if_defeated TRAINER_SILPH_GRUNT_40_HNS, SilphCo_11F_EventScript_Grunt1_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_11F_EventScript_Grunt1_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_40_HNS, SilphCo_11F_Text_Grunt1Intro, SilphCo_11F_Text_Grunt1Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_11F_EventScript_Grunt1_PostDialogue::
	msgbox SilphCo_11F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_11F_EventScript_Grunt2::
	goto_if_defeated TRAINER_SILPH_GRUNT_41_HNS, SilphCo_11F_EventScript_Grunt2_PostDialogue
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_RocketNoParty
SilphCo_11F_EventScript_Grunt2_TrainerBattle::
	trainerbattle_single TRAINER_SILPH_GRUNT_41_HNS, SilphCo_11F_Text_Grunt2Intro, SilphCo_11F_Text_Grunt2Defeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerSilph_EventScript_End
SilphCo_11F_EventScript_Grunt2_PostDialogue::
	msgbox SilphCo_11F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_11F_Text_ThanksForSavingMeDearBoy::
	.string "PRESIDENT: Oh, dear boy!\n"
	.string "Thank you for saving SILPH.\p"
	.string "I will never forget you saved us in\n"
	.string "our moment of peril.\p"
	.string "I have to thank you in some way.\p"
	.string "Because I am rich, I can give you\n"
	.string "anything.\p"
	.string "Here, maybe this will do.$"

SilphCo_11F_Text_ThanksForSavingMeDearGirl::
	.string "PRESIDENT: Oh, dear girl!\n"
	.string "Thank you for saving SILPH.\p"
	.string "I will never forget you saved us in\n"
	.string "our moment of peril.\p"
	.string "I have to thank you in some way.\p"
	.string "Because I am rich, I can give you\n"
	.string "anything.\p"
	.string "Here, maybe this will do.$"

SilphCo_11F_Text_ObtainedMasterBallFromPresident::
	.string "{PLAYER} obtained a MASTER BALL\n"
	.string "from the PRESIDENT!$"

SilphCo_11F_Text_ThatsOurSecretPrototype::
	.string "PRESIDENT: You can't buy that\n"
	.string "anywhere.\p"
	.string "It's our secret prototype MASTER\n"
	.string "BALL.\p"
	.string "It will catch any POKéMON without\n"
	.string "fail!\p"
	.string "You should be quiet about using it,\n"
	.string "though.$"

SilphCo_11F_Text_YouHaveNoRoomForThis::
	.string "You have no room for this.$"

SilphCo_11F_Text_ThanksForRescuingUs::
	.string "SECRETARY: Thank you for rescuing\n"
	.string "all of us.\p"
	.string "We, from the PRESIDENT down,\n"
	.string "are indebted to you.$"

SilphCo_11F_Text_GiovanniIntro::
	.string "Ah, {PLAYER}!\n"
	.string "So we meet again!\p"
	.string "The PRESIDENT and I are discussing\n"
	.string "a vital business proposition.\p"
	.string "Keep your nose out of grown-up\n"
	.string "matters…\p"
	.string "Or, experience a world of pain!$"

SilphCo_11F_Text_GiovanniDefeat::
	.string "Arrgh!!\n"
	.string "I lost again!?$"

SilphCo_11F_Text_GiovanniPostBattle::
	.string "Blast it all!\n"
	.string "You ruined our plans for SILPH!\p"
	.string "But, TEAM ROCKET will never fall!\p"
	.string "{PLAYER}! Never forget that all\n"
	.string "POKéMON exist for TEAM ROCKET!\p"
	.string "I must go, but I shall return!$"

SilphCo_11F_Text_Grunt2Intro::
	.string "Stop right there!\n"
	.string "Don't you move!$"

SilphCo_11F_Text_Grunt2Defeat::
	.string "Don't…\n"
	.string "Please!$"

SilphCo_11F_Text_Grunt2PostBattle::
	.string "So, you want to see my BOSS?$"

SilphCo_11F_Text_Grunt1Intro::
	.string "Halt! Do you have an appointment\n"
	.string "with my BOSS?$"

SilphCo_11F_Text_Grunt1Defeat::
	.string "Gaah!\n"
	.string "Demolished!$"

SilphCo_11F_Text_Grunt1PostBattle::
	.string "Watch your step…\n"
	.string "My BOSS likes his POKéMON tough!$"

SilphCo_11F_Text_MonitorHasMonsOnIt::
	.string "The monitor has POKéMON on it!$"

SilphCo_11F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "11F$"


SilphCo_Elevator_Frlg_MapScripts::
	.byte 0

SilphCo_Elevator_EventScript_FloorSelect::
	lockall
	setvar VAR_0x8004, 1
	special WayfarerInitSilphElevatorForScript
	copyvar VAR_0x8005, VAR_SILPH_ELEVATOR_FLOOR_HNS
	special DrawElevatorCurrentFloorWindow
	message gText_WantWhichFloor
	waitmessage
	setvar VAR_0x8004, SCROLL_MULTI_SILPHCO_FLOORS
	special ShowScrollableMultichoice
	waitstate
	switch VAR_RESULT
	case 0, SilphCo_Elevator_EventScript_To11F
	case 1, SilphCo_Elevator_EventScript_To10F
	case 2, SilphCo_Elevator_EventScript_To9F
	case 3, SilphCo_Elevator_EventScript_To8F
	case 4, SilphCo_Elevator_EventScript_To7F
	case 5, SilphCo_Elevator_EventScript_To6F
	case 6, SilphCo_Elevator_EventScript_To5F
	case 7, SilphCo_Elevator_EventScript_To4F
	case 8, SilphCo_Elevator_EventScript_To3F
	case 9, SilphCo_Elevator_EventScript_To2F
	case 10, SilphCo_Elevator_EventScript_To1F
	case 11, SilphCo_Elevator_EventScript_ExitFloorSelect
	case 127, SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To1F::
	setvar VAR_0x8006, 4
	setdynamicwarp MAP_SAFFRON_CITY_SILPH_CO_HNS, 255, 22, 4
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 4, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 4
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To2F::
	setvar VAR_0x8006, 5
	setdynamicwarp MAP_SILPH_CO_2F, 255, 22, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 5, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 5
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To3F::
	setvar VAR_0x8006, 6
	setdynamicwarp MAP_SILPH_CO_3F, 255, 22, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 6, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 6
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To4F::
	setvar VAR_0x8006, 7
	setdynamicwarp MAP_SILPH_CO_4F, 255, 22, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 7, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 7
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To5F::
	setvar VAR_0x8006, 8
	setdynamicwarp MAP_SILPH_CO_5F, 255, 22, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 8, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 8
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To6F::
	setvar VAR_0x8006, 9
	setdynamicwarp MAP_SILPH_CO_6F, 255, 20, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 9, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 9
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To7F::
	setvar VAR_0x8006, 10
	setdynamicwarp MAP_SILPH_CO_7F, 255, 23, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 10, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 10
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To8F::
	setvar VAR_0x8006, 11
	setdynamicwarp MAP_SILPH_CO_8F, 255, 22, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 11, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 11
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To9F::
	setvar VAR_0x8006, 12
	setdynamicwarp MAP_SILPH_CO_9F, 255, 24, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 12, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 12
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To10F::
	setvar VAR_0x8006, 13
	setdynamicwarp MAP_SILPH_CO_10F, 255, 13, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 13, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 13
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_To11F::
	setvar VAR_0x8006, 14
	setdynamicwarp MAP_SILPH_CO_11F, 255, 13, 3
	goto_if_eq VAR_SILPH_ELEVATOR_FLOOR_HNS, 14, SilphCo_Elevator_EventScript_ExitFloorSelect
	call SilphCo_Elevator_EventScript_MoveElevator
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 14
	goto SilphCo_Elevator_EventScript_ExitFloorSelect
	end

SilphCo_Elevator_EventScript_ExitFloorSelect::
	special CloseElevatorCurrentFloorWindow
	releaseall
	end

SilphCo_Elevator_EventScript_MoveElevator::
	special CloseElevatorCurrentFloorWindow
	closemessage
	waitse
	special AnimateElevator
	waitstate
	setflag FLAG_TEMP_2
	return

SilphCo_3F_EventScript_ItemHyperPotion::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_3F_HYPER_POTION_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_HYPER_POTION
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_HYPER_POTION
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_HYPER_POTION
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_HYPER_POTION, 1
	setflag FLAG_SILPH_3F_HYPER_POTION_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_4F_EventScript_ItemMaxRevive::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_4F_MAX_REVIVE_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_MAX_REVIVE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_MAX_REVIVE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_MAX_REVIVE
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_MAX_REVIVE, 1
	setflag FLAG_SILPH_4F_MAX_REVIVE_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_4F_EventScript_ItemEscapeRope::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_4F_ESCAPE_ROPE_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_ESCAPE_ROPE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_ESCAPE_ROPE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_ESCAPE_ROPE
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_ESCAPE_ROPE, 1
	setflag FLAG_SILPH_4F_ESCAPE_ROPE_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_4F_EventScript_ItemFullHeal::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_4F_FULL_HEAL_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_FULL_HEAL
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_FULL_HEAL
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_FULL_HEAL
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_FULL_HEAL, 1
	setflag FLAG_SILPH_4F_FULL_HEAL_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_4F_EventScript_ItemTM41::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_4F_TM41_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_TM_TORMENT
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_TM_TORMENT
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_TM_TORMENT
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_TM_TORMENT, 1
	setflag FLAG_SILPH_4F_TM41_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_5F_EventScript_ItemProtein::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_5F_PROTEIN_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_PROTEIN
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_PROTEIN
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_PROTEIN
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_PROTEIN, 1
	setflag FLAG_SILPH_5F_PROTEIN_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_5F_EventScript_ItemTM01::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_5F_TM01_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_TM_FOCUS_PUNCH
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_TM_FOCUS_PUNCH
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_TM_FOCUS_PUNCH
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_TM_FOCUS_PUNCH, 1
	setflag FLAG_SILPH_5F_TM01_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_5F_EventScript_ItemCardKey::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_CARD_KEY_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_SILPH_CARD_KEY
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_SILPH_CARD_KEY
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_SILPH_CARD_KEY
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_SILPH_CARD_KEY, 1
	setflag FLAG_SILPH_CARD_KEY_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_6F_EventScript_ItemHPUp::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_6F_HP_UP_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_HP_UP
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_HP_UP
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_HP_UP
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_HP_UP, 1
	setflag FLAG_SILPH_6F_HP_UP_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_6F_EventScript_ItemXSpecial::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_6F_X_SPECIAL_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_X_SPECIAL
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_X_SPECIAL
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_X_SPECIAL
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_X_SPECIAL, 1
	setflag FLAG_SILPH_6F_X_SPECIAL_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_7F_EventScript_ItemCalcium::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_7F_CALCIUM_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_CALCIUM
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_CALCIUM
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_CALCIUM
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_CALCIUM, 1
	setflag FLAG_SILPH_7F_CALCIUM_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_7F_EventScript_ItemTM08::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_7F_TM08_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_TM_BULK_UP
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_TM_BULK_UP
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_TM_BULK_UP
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_TM_BULK_UP, 1
	setflag FLAG_SILPH_7F_TM08_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_8F_EventScript_ItemIron::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_8F_IRON_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_IRON
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_IRON
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_IRON
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_IRON, 1
	setflag FLAG_SILPH_8F_IRON_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_10F_EventScript_ItemCarbos::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_10F_CARBOS_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_CARBOS
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_CARBOS
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_CARBOS
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_CARBOS, 1
	setflag FLAG_SILPH_10F_CARBOS_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_10F_EventScript_ItemUltraBall::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_10F_ULTRA_BALL_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_ULTRA_BALL
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_ULTRA_BALL
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_ULTRA_BALL
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_ULTRA_BALL, 1
	setflag FLAG_SILPH_10F_ULTRA_BALL_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_10F_EventScript_ItemRareCandy::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_10F_RARE_CANDY_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_RARE_CANDY
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_RARE_CANDY
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_RARE_CANDY
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_RARE_CANDY, 1
	setflag FLAG_SILPH_10F_RARE_CANDY_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

SilphCo_11F_EventScript_ItemZinc::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_11F_ZINC_RECEIVED_HNS, WayfarerSilph_EventScript_End
	checkitemspace ITEM_ZINC
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	additem ITEM_ZINC
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_ItemNoRoom
	bufferitemname STR_VAR_1, ITEM_ZINC
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_ZINC, 1
	setflag FLAG_SILPH_11F_ZINC_RECEIVED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

WayfarerSilph_EventScript_End::
	releaseall
	end

WayfarerSilph_EventScript_RocketNoParty::
	msgbox WayfarerSilph_Text_RocketNoParty, MSGBOX_DEFAULT
	releaseall
	end

WayfarerSilph_EventScript_OrdinaryNoParty::
	msgbox WayfarerSilph_Text_OrdinaryNoParty, MSGBOX_DEFAULT
	releaseall
	end

WayfarerSilph_EventScript_ItemNoRoom::
	msgbox WayfarerSilph_Text_ItemNoRoom, MSGBOX_DEFAULT
	releaseall
	end

WayfarerSilph_EventScript_Officer::
	lock
	faceplayer
	goto_if_unset FLAG_SILPH_LIBERATED_HNS, WayfarerSilph_EventScript_OfficerOccupied
	goto_if_set FLAG_GOT_UP_GRADE, WayfarerSilph_EventScript_OfficerAfterGift
	msgbox WayfarerSilph_Text_OfficerGift, MSGBOX_DEFAULT
	checkitemspace ITEM_UP_GRADE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OfficerNoRoom
	additem ITEM_UP_GRADE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_OfficerNoRoom
	bufferitemname STR_VAR_1, ITEM_UP_GRADE
	msgreceiveditem WayfarerSilph_Text_ReceivedItem, ITEM_UP_GRADE, 1
	setflag FLAG_GOT_UP_GRADE
WayfarerSilph_EventScript_OfficerAfterGift::
	msgbox SilphCo_Text_Officer_GotUpGrade, MSGBOX_DEFAULT
	goto WayfarerSilph_EventScript_OfficerOfferUpstairs
WayfarerSilph_EventScript_OfficerNoRoom::
	msgbox WayfarerSilph_Text_OfficerNoRoom, MSGBOX_DEFAULT
	goto WayfarerSilph_EventScript_OfficerOfferUpstairs
WayfarerSilph_EventScript_OfficerOccupied::
	msgbox WayfarerSilph_Text_OfficerOccupied, MSGBOX_DEFAULT
WayfarerSilph_EventScript_OfficerOfferUpstairs::
	msgbox WayfarerSilph_Text_OfficerUpstairs, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSilph_EventScript_End
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	warp MAP_SILPH_CO_2F, 30, 3
	waitstate
	releaseall
	end

WayfarerSilph_EventScript_ElevatorEntry::
	lockall
	msgbox WayfarerSilph_Text_ElevatorEntry, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSilph_EventScript_End
	setdynamicwarp MAP_SAFFRON_CITY_SILPH_CO_HNS, 255, 22, 4
	setvar VAR_SILPH_ELEVATOR_FLOOR_HNS, 4
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	warp MAP_SILPH_CO_ELEVATOR, 2, 4
	waitstate
	releaseall
	end

WayfarerSilph_EventScript_Receptionist::
	goto_if_set FLAG_SILPH_LIBERATED_HNS, WayfarerSilph_EventScript_ReceptionistRecovered
	msgbox WayfarerSilph_Text_ReceptionistOccupied, MSGBOX_NPC
	end
WayfarerSilph_EventScript_ReceptionistRecovered::
	msgbox SilphCo_Text_Receptionist, MSGBOX_NPC
	end

WayfarerSilph_EventScript_Steven::
	goto_if_set FLAG_SILPH_LIBERATED_HNS, WayfarerSilph_EventScript_StevenRecovered
	msgbox WayfarerSilph_Text_StevenOccupied, MSGBOX_NPC
	end
WayfarerSilph_EventScript_StevenRecovered::
	msgbox WayfarerSilph_Text_StevenRecovered, MSGBOX_NPC
	end

WayfarerSilph_EventScript_ElevatorSign::
	msgbox WayfarerSilph_Text_ElevatorSign, MSGBOX_SIGN
	end

EventScript_ThunderWaveTutor::
	lock
	faceplayer
	goto_if_set FLAG_SILPH_THUNDER_WAVE_TUTOR_HNS, WayfarerSilph_EventScript_TutorTaught
	msgbox Text_ThunderWaveTeach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSilph_EventScript_TutorDeclined
	msgbox WayfarerSilph_Text_TutorOnce, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSilph_EventScript_TutorDeclined
	msgbox Text_ThunderWaveWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, MOVE_THUNDER_WAVE
	special ChooseMonForMoveTutor
	waitstate
	lock
	faceplayer
	goto_if_eq VAR_RESULT, FALSE, WayfarerSilph_EventScript_TutorDeclined
	setflag FLAG_SILPH_THUNDER_WAVE_TUTOR_HNS
WayfarerSilph_EventScript_TutorTaught::
	msgbox Text_ThunderWaveTaught, MSGBOX_DEFAULT
	releaseall
	end
WayfarerSilph_EventScript_TutorDeclined::
	msgbox Text_ThunderWaveDeclined, MSGBOX_DEFAULT
	releaseall
	end

WayfarerSilph_Text_ReceivedItem::
	.string "Obtained {STR_VAR_1}!$"
WayfarerSilph_Text_ItemNoRoom::
	.string "There's no room in your BAG\nfor this item.$"
WayfarerSilph_Text_RocketNoParty::
	.string "You've got no POKéMON to stop us.\nGet lost!$"
WayfarerSilph_Text_OrdinaryNoParty::
	.string "Come back when you have a POKéMON\nthat can battle.$"
WayfarerSilph_Text_GiovanniIntro::
	.string "GIOVANNI: The PRESIDENT and I\nare discussing business.\p"
	.string "Keep your nose out of our affairs,\nor face my POKéMON!$"
WayfarerSilph_Text_GiovanniDefeat::
	.string "Arrgh!\nYou've ruined my plans!$"
WayfarerSilph_Text_PresidentOccupied::
	.string "TEAM ROCKET is holding us here!\nPlease drive them out!$"
WayfarerSilph_Text_PresidentLiberated::
	.string "PRESIDENT: Thank you for saving\nSILPH CO. and all our staff!$"
WayfarerSilph_Text_OfficerOccupied::
	.string "TEAM ROCKET has taken over\nthe upper floors!\p"
	.string "Our staff are trapped up there.\nPlease help them!$"
WayfarerSilph_Text_OfficerGift::
	.string "Thank you for helping SILPH CO.!\nPlease take this souvenir.$"
WayfarerSilph_Text_OfficerNoRoom::
	.string "Your BAG has no room for it.\nI'll keep it here for you.$"
WayfarerSilph_Text_OfficerUpstairs::
	.string "Would you like to go upstairs?$"
WayfarerSilph_Text_ElevatorEntry::
	.string "Call the elevator?$"
WayfarerSilph_Text_ElevatorSign::
	.string "SILPH CO. ELEVATOR\nUse the doors to the left.$"
WayfarerSilph_Text_ReceptionistOccupied::
	.string "TEAM ROCKET is upstairs!\nPlease be careful.$"
WayfarerSilph_Text_StevenOccupied::
	.string "STEVEN: Something's wrong here.\nThe staff need help upstairs.$"
WayfarerSilph_Text_StevenRecovered::
	.string "STEVEN: You helped the people\nhere. Thank you.\p"
	.string "I'd like to thank you properly.\nWe'll talk again another time.$"
WayfarerSilph_Text_TutorOnce::
	.string "I can teach this move only once.\nWould you like me to teach it now?$"

WayfarerSilph_EventScript_GiovanniRetry::
	fadescreen FADE_TO_BLACK
	setobjectxy LOCALID_SILPH_CO_GIOVANNI, 6, 11
	setobjectmovementtype LOCALID_SILPH_CO_GIOVANNI, MOVEMENT_TYPE_FACE_RIGHT
	setvar VAR_TEMP_1, 0
	fadescreen FADE_FROM_BLACK
	releaseall
	end

WayfarerSilph_EventScript_SecretaryOccupied::
	msgbox WayfarerSilph_Text_SecretaryOccupied, MSGBOX_NPC
	end

WayfarerSilph_Text_SecretaryOccupied::
	.string "Please help us! TEAM ROCKET\n"
	.string "has taken over our company!$"

SilphCo_5F_EventScript_Scientist::
	goto_if_set FLAG_SILPH_LIBERATED_HNS, WayfarerSilph_EventScript_Scientist5FRecovered
	msgbox SilphCo_5F_Text_RocketBossLookingForStrongMons, MSGBOX_NPC
	end

WayfarerSilph_EventScript_Scientist5FRecovered::
	msgbox SilphCo_5F_Text_YoureOurHeroThankYou, MSGBOX_NPC
	end

SilphCo_8F_EventScript_Scientist::
	goto_if_set FLAG_SILPH_LIBERATED_HNS, WayfarerSilph_EventScript_Scientist8FRecovered
	msgbox SilphCo_8F_Text_ToRocketBossMonsAreTools, MSGBOX_NPC
	end

WayfarerSilph_EventScript_Scientist8FRecovered::
	msgbox SilphCo_8F_Text_ThanksForSavingUs, MSGBOX_NPC
	end
