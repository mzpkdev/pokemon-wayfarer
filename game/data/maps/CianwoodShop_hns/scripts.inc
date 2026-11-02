CianwoodShop_hns_MapScripts::
	.byte 0

CianwoodShop_EventScriptMan::	
	setvar VAR_OLIVINE_CITY_STATE, 4
	giveitem ITEM_SECRET_POTION
	end

CianwoodCity_EventScript_Pharmacist::
	lock
	faceplayer
	checkitem ITEM_SECRET_POTION
	goto_if_eq VAR_RESULT, TRUE, CianwoodCity_EventScript_PharmacistAfter
	goto_if_ge VAR_OLIVINE_CITY_STATE, 5, CianwoodCity_EventScript_PharmacistAfter
	goto_if_lt VAR_OLIVINE_CITY_STATE, 3, CianwoodCity_EventScript_PharmacistMedicineFirst
	msgbox CianwoodCity_Text_PharmacistGivePotion, MSGBOX_DEFAULT
	goto CianwoodCity_EventScript_PharmacistGive

@ Jasmine has not asked yet: the potion is for the sick POKéMON at the lighthouse, nobody sent the player.
CianwoodCity_EventScript_PharmacistMedicineFirst::
	msgbox CianwoodCity_Text_PharmacistGivePotionFirst, MSGBOX_DEFAULT
CianwoodCity_EventScript_PharmacistGive::
	giveitem ITEM_SECRET_POTION
	goto_if_eq VAR_RESULT, FALSE, CianwoodCity_EventScript_PharmacistBagFull
	closemessage
	call_if_lt VAR_OLIVINE_CITY_STATE, 3, CianwoodCity_EventScript_NoteMedicineFirst
	setvar VAR_OLIVINE_CITY_STATE, 4
	release
	end

CianwoodCity_EventScript_NoteMedicineFirst::
	setflag FLAG_WAYFARER_JOHTO_LIGHTHOUSE_MEDICINE_FIRST
	return

CianwoodCity_EventScript_PharmacistBagFull::
	call Common_EventScript_BagIsFull
	closemessage
	release
	end

CianwoodCity_EventScript_PharmacistAfter::
	msgbox CianwoodCity_Text_PharmacistDescribePotion, MSGBOX_DEFAULT
	closemessage
	release
	end


CianwoodCity_Text_PharmacistGivePotion:
	.string "Your POKéMON appear to be fine.\p"
	.string "Is something worrying you?\p"
	.string "…\p"
	.string "The LIGHTHOUSE POKéMON is in\n"
	.string "trouble?\p"
	.string "I got it!\p"
	.string "This ought to do the trick.$"

CianwoodCity_Text_PharmacistGivePotionFirst:
	.string "Your POKéMON appear to be fine.\p"
	.string "…You're not here about a sick\n"
	.string "POKéMON?\p"
	.string "Then take this anyway.\p"
	.string "My SECRET POTION treats the sick\n"
	.string "POKéMON at the OLIVINE LIGHTHOUSE.\p"
	.string "This ought to do the trick.$"

CianwoodCity_Text_ReceivedSecretpotion:
	.string "{PLAYER} received SECRET POTION.$"

CianwoodCity_Text_PharmacistDescribePotion:
	.string "My SECRET POTION is a tad too\n"
	.string "strong, so I only offer it in an\l"
	.string "emergency.$"
