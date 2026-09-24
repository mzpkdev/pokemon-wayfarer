WayfarerNuggetBridge_EventScript_Cale::
	goto_if_defeated TRAINER_NUGGET_BRIDGE_CALE_HNS, WayfarerNuggetBridge_EventScript_CaleAfter
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_NoParty
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_CaleIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_NUGGET_BRIDGE_CALE_HNS, WayfarerNuggetBridge_Text_CaleDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerNuggetBridge_EventScript_End
WayfarerNuggetBridge_EventScript_CaleAfter::
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_CalePostBattle, MSGBOX_DEFAULT
	goto WayfarerNuggetBridge_EventScript_End

WayfarerNuggetBridge_EventScript_Ali::
	goto_if_defeated TRAINER_NUGGET_BRIDGE_ALI_HNS, WayfarerNuggetBridge_EventScript_AliAfter
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_CALE_HNS, WayfarerNuggetBridge_EventScript_NextCale
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_NoParty
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_AliIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_NUGGET_BRIDGE_ALI_HNS, WayfarerNuggetBridge_Text_AliDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerNuggetBridge_EventScript_End
WayfarerNuggetBridge_EventScript_AliAfter::
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_AliPostBattle, MSGBOX_DEFAULT
	goto WayfarerNuggetBridge_EventScript_End

WayfarerNuggetBridge_EventScript_Timmy::
	goto_if_defeated TRAINER_NUGGET_BRIDGE_TIMMY_HNS, WayfarerNuggetBridge_EventScript_TimmyAfter
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_CALE_HNS, WayfarerNuggetBridge_EventScript_NextCale
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_ALI_HNS, WayfarerNuggetBridge_EventScript_NextAli
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_NoParty
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_TimmyIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_NUGGET_BRIDGE_TIMMY_HNS, WayfarerNuggetBridge_Text_TimmyDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerNuggetBridge_EventScript_End
WayfarerNuggetBridge_EventScript_TimmyAfter::
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_TimmyPostBattle, MSGBOX_DEFAULT
	goto WayfarerNuggetBridge_EventScript_End

WayfarerNuggetBridge_EventScript_Reli::
	goto_if_defeated TRAINER_NUGGET_BRIDGE_RELI_HNS, WayfarerNuggetBridge_EventScript_ReliAfter
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_CALE_HNS, WayfarerNuggetBridge_EventScript_NextCale
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_ALI_HNS, WayfarerNuggetBridge_EventScript_NextAli
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_TIMMY_HNS, WayfarerNuggetBridge_EventScript_NextTimmy
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_NoParty
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_ReliIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_NUGGET_BRIDGE_RELI_HNS, WayfarerNuggetBridge_Text_ReliDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerNuggetBridge_EventScript_End
WayfarerNuggetBridge_EventScript_ReliAfter::
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_ReliPostBattle, MSGBOX_DEFAULT
	goto WayfarerNuggetBridge_EventScript_End

WayfarerNuggetBridge_EventScript_Ethan::
	goto_if_defeated TRAINER_NUGGET_BRIDGE_ETHAN_HNS, WayfarerNuggetBridge_EventScript_EthanAfter
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_CALE_HNS, WayfarerNuggetBridge_EventScript_NextCale
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_ALI_HNS, WayfarerNuggetBridge_EventScript_NextAli
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_TIMMY_HNS, WayfarerNuggetBridge_EventScript_NextTimmy
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_RELI_HNS, WayfarerNuggetBridge_EventScript_NextReli
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_NoParty
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_EthanIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_NUGGET_BRIDGE_ETHAN_HNS, WayfarerNuggetBridge_Text_EthanDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerNuggetBridge_EventScript_End
WayfarerNuggetBridge_EventScript_EthanAfter::
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_EthanPostBattle, MSGBOX_DEFAULT
	goto WayfarerNuggetBridge_EventScript_End

WayfarerNuggetBridge_EventScript_Rocket::
	goto_if_defeated TRAINER_NUGGET_BRIDGE_ROCKET_HNS, WayfarerNuggetBridge_EventScript_RocketAfter
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_CALE_HNS, WayfarerNuggetBridge_EventScript_NextCale
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_ALI_HNS, WayfarerNuggetBridge_EventScript_NextAli
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_TIMMY_HNS, WayfarerNuggetBridge_EventScript_NextTimmy
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_RELI_HNS, WayfarerNuggetBridge_EventScript_NextReli
	goto_if_not_defeated TRAINER_NUGGET_BRIDGE_ETHAN_HNS, WayfarerNuggetBridge_EventScript_NextEthan
	goto_if_set FLAG_NUGGET_BRIDGE_NUGGET_RECEIVED_HNS, WayfarerNuggetBridge_EventScript_Recruitment
	checkitemspace ITEM_NUGGET
	goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_NoRoom
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_JustEarnedFabulousPrize, MSGBOX_DEFAULT
	additem ITEM_NUGGET
	goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_NoRoomLocked
	msgreceiveditem WayfarerNuggetBridge_Text_ReceivedNuggetFromMysteryTrainer, ITEM_NUGGET, 1
	setflag FLAG_NUGGET_BRIDGE_NUGGET_RECEIVED_HNS
	releaseall
WayfarerNuggetBridge_EventScript_Recruitment::
	specialvar VAR_RESULT, WayfarerCanStartOrdinaryBattleForScript
	goto_if_eq VAR_RESULT, FALSE, WayfarerNuggetBridge_EventScript_RecruiterNoParty
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_JoinTeamRocket, MSGBOX_DEFAULT
	playbgm MUS_RG_ENCOUNTER_ROCKET, FALSE
	trainerbattle_no_intro TRAINER_NUGGET_BRIDGE_ROCKET_HNS, WayfarerNuggetBridge_Text_RocketDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, WayfarerNuggetBridge_EventScript_End
WayfarerNuggetBridge_EventScript_RocketAfter::
	lock
	faceplayer
	msgbox WayfarerNuggetBridge_Text_YoudBecomeTopRocketLeader, MSGBOX_DEFAULT
WayfarerNuggetBridge_EventScript_End::
	releaseall
	end

WayfarerNuggetBridge_EventScript_NoRoom::
	msgbox WayfarerNuggetBridge_Text_NoRoom, MSGBOX_NPC
	end

WayfarerNuggetBridge_EventScript_NoRoomLocked::
	msgbox WayfarerNuggetBridge_Text_NoRoom, MSGBOX_DEFAULT
	goto WayfarerNuggetBridge_EventScript_End

WayfarerNuggetBridge_EventScript_NoParty::
	msgbox WayfarerNuggetBridge_Text_NoParty, MSGBOX_NPC
	end

WayfarerNuggetBridge_EventScript_RecruiterNoParty::
	msgbox WayfarerNuggetBridge_Text_RecruiterNoParty, MSGBOX_NPC
	end

WayfarerNuggetBridge_EventScript_NextCale::
	msgbox WayfarerNuggetBridge_Text_NextCale, MSGBOX_NPC
	end

WayfarerNuggetBridge_EventScript_NextAli::
	msgbox WayfarerNuggetBridge_Text_NextAli, MSGBOX_NPC
	end

WayfarerNuggetBridge_EventScript_NextTimmy::
	msgbox WayfarerNuggetBridge_Text_NextTimmy, MSGBOX_NPC
	end

WayfarerNuggetBridge_EventScript_NextReli::
	msgbox WayfarerNuggetBridge_Text_NextReli, MSGBOX_NPC
	end

WayfarerNuggetBridge_EventScript_NextEthan::
	msgbox WayfarerNuggetBridge_Text_NextEthan, MSGBOX_NPC
	end

WayfarerNuggetBridge_Text_CaleIntro::
	.string "People call this the NUGGET\n"
	.string "BRIDGE!\p"
	.string "Beat us five TRAINERS and win\n"
	.string "a fabulous prize!\p"
	.string "Think you got what it takes?$"

WayfarerNuggetBridge_Text_CaleDefeat::
	.string "Whoo!\n"
	.string "Good stuff!$"

WayfarerNuggetBridge_Text_CalePostBattle::
	.string "I did my best. I have no regrets!$"

WayfarerNuggetBridge_Text_AliIntro::
	.string "I'm second!\n"
	.string "Now it's serious!$"

WayfarerNuggetBridge_Text_AliDefeat::
	.string "How could I lose?$"

WayfarerNuggetBridge_Text_AliPostBattle::
	.string "I did my best. I have no regrets!$"

WayfarerNuggetBridge_Text_TimmyIntro::
	.string "Here's No. 3!\n"
	.string "I won't be easy!$"

WayfarerNuggetBridge_Text_TimmyDefeat::
	.string "Ow!\n"
	.string "Stomped flat!$"

WayfarerNuggetBridge_Text_TimmyPostBattle::
	.string "I did my best. I have no regrets!$"

WayfarerNuggetBridge_Text_ReliIntro::
	.string "I'm No. 4!\n"
	.string "Getting tired?$"

WayfarerNuggetBridge_Text_ReliDefeat::
	.string "I lost, too!$"

WayfarerNuggetBridge_Text_ReliPostBattle::
	.string "I did my best, so I've no regrets!$"

WayfarerNuggetBridge_Text_EthanIntro::
	.string "Okay! I'm No. 5!\n"
	.string "I'll stomp you!$"

WayfarerNuggetBridge_Text_EthanDefeat::
	.string "Whoa!\n"
	.string "Too much!$"

WayfarerNuggetBridge_Text_EthanPostBattle::
	.string "I did my best. I have no regrets!$"

WayfarerNuggetBridge_Text_JustEarnedFabulousPrize::
	.string "Congratulations! You beat our\n"
	.string "five contest TRAINERS!\p"
	.string "You just earned a fabulous prize!$"

WayfarerNuggetBridge_Text_ReceivedNuggetFromMysteryTrainer::
	.string "{PLAYER} received a NUGGET\n"
	.string "from the mystery TRAINER!$"

WayfarerNuggetBridge_Text_JoinTeamRocket::
	.string "By the way, how would you like to\n"
	.string "join TEAM ROCKET?\p"
	.string "We're a group of professional\n"
	.string "criminals specializing in POKéMON!\p"
	.string "Want to join?\p"
	.string "Are you sure?\p"
	.string "Come on, join us!\p"
	.string "I'm telling you to join!\p"
	.string "…Okay, you need convincing!\p"
	.string "I'll make you an offer you can't\n"
	.string "refuse!$"

WayfarerNuggetBridge_Text_RocketDefeat::
	.string "Arrgh!\n"
	.string "You are good!$"

WayfarerNuggetBridge_Text_YoudBecomeTopRocketLeader::
	.string "With your ability, you'd become\n"
	.string "a top leader in TEAM ROCKET.\p"
	.string "Come on, think of the opportunity!\n"
	.string "Don't let this chance go to waste.$"

WayfarerNuggetBridge_Text_NextCale::
	.string "Please battle CALE next.\n"
	.string "We take this challenge in order!$"

WayfarerNuggetBridge_Text_NextAli::
	.string "Please battle ALI next.\n"
	.string "We take this challenge in order!$"

WayfarerNuggetBridge_Text_NextTimmy::
	.string "Please battle TIMMY next.\n"
	.string "We take this challenge in order!$"

WayfarerNuggetBridge_Text_NextReli::
	.string "Please battle RELI next.\n"
	.string "We take this challenge in order!$"

WayfarerNuggetBridge_Text_NextEthan::
	.string "Please battle ETHAN next.\n"
	.string "We take this challenge in order!$"

WayfarerNuggetBridge_Text_NoParty::
	.string "Come back when you have a POKéMON\n"
	.string "that can battle.$"

WayfarerNuggetBridge_Text_RecruiterNoParty::
	.string "Come back with a POKéMON ready\n"
	.string "to battle. Then we'll talk.$"

WayfarerNuggetBridge_Text_NoRoom::
	.string "Make room in your ITEMS pocket\n"
	.string "for your NUGGET prize!$"
