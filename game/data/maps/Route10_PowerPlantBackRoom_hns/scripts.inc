Route10_PowerPlantBackRoom_hns_MapScripts::
	.byte 0


Route10_PowerPlantBackRoom_EventScript_Manager::
	lock
	faceplayer
	goto_if_set FLAG_RETURNED_MACHINE_PART, Route10_PowerPlantBackRoom_EventScript_ManagerDone
	goto_if_unset FLAG_HNS_MAGNET_TRAIN_RESTORATION_STARTED, Route10_PowerPlantBackRoom_EventScript_ManagerInitial
	checkitem ITEM_MACHINE_PART
	goto_if_eq VAR_RESULT, TRUE, Route10_PowerPlantBackRoom_EventScript_ManagerCheckPart
	clearflag FLAG_HIDDEN_ITEM_MACHINE_PART
	msgbox Route10_PowerPlantBackRoom_Text_ManagerSearchCeruleanGym, MSGBOX_DEFAULT
	closemessage
	release
	end

Route10_PowerPlantBackRoom_EventScript_ManagerInitial::
	msgbox Route10_PowerPlantBackRoom_Text_ManagerWhoWouldRuinMyGenerator, MSGBOX_DEFAULT
	closemessage
	release
	end

Route10_PowerPlantBackRoom_EventScript_ManagerCheckPart::
	msgbox Route10_PowerPlantBackRoom_Text_ManagerThatsThePart, MSGBOX_DEFAULT
	checkitem ITEM_TM_THUNDER
	goto_if_eq VAR_RESULT, TRUE, Route10_PowerPlantBackRoom_EventScript_ManagerCommitRepair
	checkitemspace ITEM_TM_THUNDER
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox Route10_PowerPlantBackRoom_Text_ManagerTakeThisTM, MSGBOX_DEFAULT
	giveitem ITEM_TM_THUNDER
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	checkitem ITEM_TM_THUNDER
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox Route10_PowerPlantBackRoom_Text_ManagerTM07IsZapCannon, MSGBOX_DEFAULT
	closemessage

Route10_PowerPlantBackRoom_EventScript_ManagerCommitRepair::
	removeitem ITEM_MACHINE_PART
	goto_if_eq VAR_RESULT, FALSE, Route10_PowerPlantBackRoom_EventScript_ManagerPartRemovalFailed
	setflag FLAG_RETURNED_MACHINE_PART
	setflag FLAG_HIDDEN_ITEM_MACHINE_PART
	msgbox Route10_PowerPlantBackRoom_Text_ManagerGeneratorRestored, MSGBOX_DEFAULT
	closemessage
	release
	end

Route10_PowerPlantBackRoom_EventScript_ManagerPartRemovalFailed::
	msgbox Route10_PowerPlantBackRoom_Text_ManagerBringMachinePart, MSGBOX_DEFAULT
	closemessage
	release
	end

Route10_PowerPlantBackRoom_EventScript_ManagerDone::
	msgbox Route10_PowerPlantBackRoom_Text_ManagerMyBelovedGenerator, MSGBOX_DEFAULT
	closemessage
	release
	end

Route10_PowerPlantBackRoom_EventScript_Engineer::
	msgbox Route10_PowerPlantBackRoom_Text_Engineer, MSGBOX_NPC
	end

Route10_PowerPlantBackRoom_Text_ManagerWhoWouldRuinMyGenerator:
	.string "MANAGER: The generator is down,\n"
	.string "so the MAGNET TRAIN has no power.\p"
	.string "The SAFFRON STATION attendant has\n"
	.string "the latest repair information.$"

Route10_PowerPlantBackRoom_Text_ManagerSearchCeruleanGym:
	.string "MANAGER: The missing MACHINE PART\n"
	.string "should still be in CERULEAN GYM.\p"
	.string "Please search around the pool and\n"
	.string "bring it back here.$"

Route10_PowerPlantBackRoom_Text_ManagerThatsThePart:
	.string "MANAGER: Ah! Yeah!\p"
	.string "That's the missing PART from my\n"
	.string "beloved generator! You found it?$"

Route10_PowerPlantBackRoom_Text_ManagerTakeThisTM:
	.string "Wahah! Thanks!\p"
	.string "Here! Take this TM as a reward!$"

Route10_PowerPlantBackRoom_Text_ManagerTM07IsZapCannon:
	.string "MANAGER: That TM is THUNDER.\p"
	.string "It's a powerful technique!\p"
	.string "It's not what anyone would\n"
	.string "consider accurate, but it packs a\l"
	.string "wallop!$"

Route10_PowerPlantBackRoom_Text_ManagerGeneratorRestored:
	.string "MANAGER: The missing part is back!\p"
	.string "The generator and MAGNET TRAIN can\n"
	.string "run again. Wahah!$"

Route10_PowerPlantBackRoom_Text_ManagerBringMachinePart:
	.string "MANAGER: Please bring me the\n"
	.string "MACHINE PART from CERULEAN GYM.$"

Route10_PowerPlantBackRoom_Text_ManagerMyBelovedGenerator:
	.string "MANAGER: My beloved generator!\p"
	.string "Keep pumping the electricity out!$"

Route10_PowerPlantBackRoom_Text_Engineer:
	.string "This door is unsafe without power.$"
