SSAnne_2F_Room2_Frlg_MapScripts::
	.byte 0

SSAnne_2F_Room2_EventScript_Dale::
#if IS_WAYFARER
	trainerbattle_single TRAINER_SS_ANNE_FISHERMAN_DALE_HNS, SSAnne_2F_Room2_Text_DaleIntro, SSAnne_2F_Room2_Text_DaleDefeat
#else
	trainerbattle_single TRAINER_FISHERMAN_DALE, SSAnne_2F_Room2_Text_DaleIntro, SSAnne_2F_Room2_Text_DaleDefeat
#endif
	msgbox SSAnne_2F_Room2_Text_DalePostBattle, MSGBOX_AUTOCLOSE
	end

SSAnne_2F_Room2_EventScript_Brooks::
#if IS_WAYFARER
	trainerbattle_single TRAINER_SS_ANNE_GENTLEMAN_BROOKS_HNS, SSAnne_2F_Room2_Text_BrooksIntro, SSAnne_2F_Room2_Text_BrooksDefeat
#else
	trainerbattle_single TRAINER_GENTLEMAN_BROOKS, SSAnne_2F_Room2_Text_BrooksIntro, SSAnne_2F_Room2_Text_BrooksDefeat
#endif
	msgbox SSAnne_2F_Room2_Text_BrooksPostBattle, MSGBOX_AUTOCLOSE
	end

SSAnne_2F_Room2_Text_BrooksIntro::
	.string "Competing against the young keeps\n"
	.string "me youthful.$"

SSAnne_2F_Room2_Text_BrooksDefeat::
	.string "Good match!\n"
	.string "Ah, I feel young again!$"

SSAnne_2F_Room2_Text_BrooksPostBattle::
	.string "Fifteen years ago, I would\n"
	.string "have won!$"

SSAnne_2F_Room2_Text_DaleIntro::
	.string "Check out what I fished up!$"

SSAnne_2F_Room2_Text_DaleDefeat::
	.string "I'm all out!$"

SSAnne_2F_Room2_Text_DalePostBattle::
	.string "Party?\p"
	.string "The cruise ship's party should be\n"
	.string "over by now.$"

