Route10_PowerPlantEntrance_hns_MapScripts::
	.byte 0

Route10_PowerPlantEntrance_EventScript_Cop::
	goto_if_set FLAG_RETURNED_MACHINE_PART, Route10_PowerPlantEntrance_EventScript_CopAfterDone
	msgbox Route10_PowerPlantEntrance_Text_Officer1AThiefBrokeIn, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_EventScript_CopAfterDone::
	msgbox Route10_PowerPlantEntrance_Text_Officer1HaveToBeefUpSecurity, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_EventScript_Engineer1::
	goto_if_set FLAG_RETURNED_MACHINE_PART, Route10_PowerPlantEntrance_EventScript_Engineer1Done
	msgbox Route10_PowerPlantEntrance_Text_Engineer1SomeoneStoleAPart, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_EventScript_Engineer1Done::
	msgbox Route10_PowerPlantEntrance_Text_Engineer1GeneratorUpAndRunning, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_EventScript_Engineer2::
	goto_if_set FLAG_RETURNED_MACHINE_PART, Route10_PowerPlantEntrance_EventScript_Engineer2Done
	msgbox Route10_PowerPlantEntrance_Text_Engineer2PowerPlantUpAndRunning, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_EventScript_Engineer2Done::
	msgbox Route10_PowerPlantEntrance_Text_Engineer2GeneratorIsRunningAgain, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_EventScript_Cop2::
	goto_if_set FLAG_RETURNED_MACHINE_PART, Route10_PowerPlantEntrance_EventScript_Cop2Done
	msgbox Route10_PowerPlantEntrance_Text_Officer2ManagerHasBeenSadAndFurious, MSGBOX_NPC
	end
Route10_PowerPlantEntrance_EventScript_Cop2Done::
	msgbox Route10_PowerPlantEntrance_Text_Officer2ManagerHasBeenCheerful, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_EventScript_Engineer3::
	goto_if_set FLAG_RETURNED_MACHINE_PART, Route10_PowerPlantEntrance_EventScript_Engineer3Done
	msgbox Route10_PowerPlantEntrance_Text_Engineer4MagnetTrainConsumesElectricity, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_EventScript_Engineer3Done::
	msgbox Route10_PowerPlantEntrance_Text_Engineer4WeCanGetMagnetTrainRunning, MSGBOX_NPC
	end

Route10_PowerPlantEntrance_Text_Officer1AThiefBrokeIn:
	.string "The generator is out of service.\p"
	.string "The MANAGER is in the back room\n"
	.string "if you're here to help.$"

Route10_PowerPlantEntrance_Text_Officer1HaveToBeefUpSecurity:
	.string "We'll have to beef up our\n"
	.string "security presence.$"

Route10_PowerPlantEntrance_Text_Engineer1SomeoneStoleAPart:
	.string "Someone made off with a part\n"
	.string "that's essential for the\l"
	.string "generator.\p"
	.string "Without it, the new generator's\n"
	.string "useless!$"

Route10_PowerPlantEntrance_Text_Engineer1GeneratorUpAndRunning:
	.string "The generator's up and running.\n"
	.string "It's making electricity to spare.$"

Route10_PowerPlantEntrance_Text_Engineer2PowerPlantUpAndRunning:
	.string "This POWER PLANT had been\n"
	.string "abandoned in the past.\p"
	.string "The new generator normally powers\n"
	.string "the MAGNET TRAIN, but right now\l"
	.string "it's out of service.$"

Route10_PowerPlantEntrance_Text_Engineer2GeneratorIsRunningAgain:
	.string "The generator's running again!$"

Route10_PowerPlantEntrance_Text_Officer2ManagerHasBeenSadAndFurious:
	.string "The POWER PLANT's MANAGER is in\n"
	.string "the back room with the generator.\p"
	.string "But since someone wrecked the\n"
	.string "generator, he's been both sad and\l"
	.string "furious…$"

Route10_PowerPlantEntrance_Text_Officer2ManagerHasBeenCheerful:
	.string "Since the generator's been fixed,\n"
	.string "the MANAGER has been cheerful.$"

Route10_PowerPlantEntrance_Text_Engineer4MagnetTrainConsumesElectricity:
	.string "The MAGNET TRAIN consumes a lot\n"
	.string "of electricity.\p"
	.string "It can't move if the new\n"
	.string "generator isn't operating.$"

Route10_PowerPlantEntrance_EventScript_Lorenzo::
	lock
	faceplayer
	goto_if_set FLAG_POWERPLANT_NPC_TRADE_COMPLETED, Route10_PowerPlantEntrance_EventScript_TradeCompleted
	ingame_trade INGAME_TRADE_MAGNETON, Route10_PowerPlantEntrance_Text_WantToTrade, Route10_PowerPlantEntrance_EventScript_DeclineTrade, Route10_PowerPlantEntrance_EventScript_NotRequestedMon, Route10_PowerPlantEntrance_Text_TakeCare
	setflag FLAG_POWERPLANT_NPC_TRADE_COMPLETED
	release
	end

Route10_PowerPlantEntrance_EventScript_DeclineTrade::
	msgbox Route10_PowerPlantEntrance_Text_DeclineTrade, MSGBOX_DEFAULT
	release
	end

Route10_PowerPlantEntrance_EventScript_NotRequestedMon::
	bufferspeciesname STR_VAR_1, VAR_0x8009
	msgbox Route10_PowerPlantEntrance_Text_NotRequestedMon, MSGBOX_DEFAULT
	release
	end

Route10_PowerPlantEntrance_EventScript_TradeCompleted::
	msgbox Route10_PowerPlantEntrance_Text_TradeCompleted, MSGBOX_DEFAULT
	release
	end

Route10_PowerPlantEntrance_Text_WantToTrade:
	.string "I work here at the POWER PLANT.\n"
	.string "My MAGNETON's been my partner\l"
	.string "for years.\p"
	.string "But I've always wanted a\n"
	.string "{STR_VAR_1}.\p"
	.string "Would you trade one for my\n"
	.string "{STR_VAR_2}?$"

Route10_PowerPlantEntrance_Text_TakeCare:
	.string "Thanks! Take good care of\n"
	.string "MAGGIE for me!$"

Route10_PowerPlantEntrance_Text_DeclineTrade:
	.string "No? That's a shame.\n"
	.string "My MAGNETON's a hard worker.$"

Route10_PowerPlantEntrance_Text_NotRequestedMon:
	.string "Hmm, that's not a {STR_VAR_1}.\n"
	.string "That's not what I had in mind.$"

Route10_PowerPlantEntrance_Text_TradeCompleted:
	.string "How's MAGGIE doing?\n"
	.string "She loves magnetic fields!$"

Route10_PowerPlantEntrance_Text_Engineer4WeCanGetMagnetTrainRunning:
	.string "All right! We can finally get the\n"
	.string "MAGNET TRAIN running again.$"
