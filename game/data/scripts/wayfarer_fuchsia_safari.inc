@ Wayfarer-only Fuchsia Safari objectives. HNS's existing admission, Safari
@ sessions, research, and competition scripts remain in their HNS map files.

WayfarerFuchsiaSafari_EventScript_GoldTeeth::
	lock
	faceplayer
	checkitem ITEM_GOLD_TEETH
	goto_if_eq VAR_RESULT, TRUE, WayfarerFuchsiaSafari_EventScript_GoldTeethAlreadyOwned
	checkitemspace ITEM_GOLD_TEETH
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_GoldTeethNoRoom
	additem ITEM_GOLD_TEETH
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_GoldTeethNoRoom
	checkitem ITEM_GOLD_TEETH
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_GoldTeethNoRoom
	msgreceiveditem WayfarerFuchsiaSafari_Text_ReceivedGoldTeeth, ITEM_GOLD_TEETH, 1, MUS_RG_OBTAIN_KEY_ITEM
	setflag FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

WayfarerFuchsiaSafari_EventScript_GoldTeethAlreadyOwned::
	msgbox WayfarerFuchsiaSafari_Text_GoldTeethAlreadyOwned, MSGBOX_DEFAULT
	setflag FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS
	removeobject VAR_LAST_TALKED
	release
	end

WayfarerFuchsiaSafari_EventScript_GoldTeethNoRoom::
	msgbox WayfarerFuchsiaSafari_Text_GoldTeethNoRoom, MSGBOX_DEFAULT
	release
	end

WayfarerFuchsiaSafari_EventScript_SurfAttendant::
	lock
	faceplayer
	goto_if_set FLAG_FUCHSIA_SAFARI_SURF_RECEIVED_HNS, WayfarerFuchsiaSafari_EventScript_SurfComplete
	checkitem ITEM_HM03
	goto_if_eq VAR_RESULT, TRUE, WayfarerFuchsiaSafari_EventScript_SurfAlreadyOwned
	msgbox WayfarerFuchsiaSafari_Text_SurfPrize, MSGBOX_DEFAULT
	checkitemspace ITEM_HM03
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_SurfNoRoom
	additem ITEM_HM03
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_SurfNoRoom
	checkitem ITEM_HM03
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_SurfNoRoom
	msgreceiveditem WayfarerFuchsiaSafari_Text_ReceivedSurf, ITEM_HM03, 1
	setflag FLAG_FUCHSIA_SAFARI_SURF_RECEIVED_HNS
	goto WayfarerFuchsiaSafari_EventScript_SurfComplete

WayfarerFuchsiaSafari_EventScript_SurfAlreadyOwned::
	msgbox WayfarerFuchsiaSafari_Text_SurfAlreadyOwned, MSGBOX_DEFAULT
	setflag FLAG_FUCHSIA_SAFARI_SURF_RECEIVED_HNS
	goto WayfarerFuchsiaSafari_EventScript_SurfComplete

WayfarerFuchsiaSafari_EventScript_SurfNoRoom::
	msgbox WayfarerFuchsiaSafari_Text_SurfNoRoom, MSGBOX_DEFAULT
	release
	end

WayfarerFuchsiaSafari_EventScript_SurfComplete::
	msgbox WayfarerFuchsiaSafari_Text_SurfComplete, MSGBOX_DEFAULT
	release
	end

WayfarerFuchsiaSafari_EventScript_Baoba::
	lock
	faceplayer
	goto_if_set FLAG_FUCHSIA_SAFARI_STRENGTH_RECEIVED_HNS, WayfarerFuchsiaSafari_EventScript_BaobaComplete
	checkitem ITEM_GOLD_TEETH
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_BaobaNeedsTeeth
	checkitem ITEM_HM04
	goto_if_eq VAR_RESULT, TRUE, WayfarerFuchsiaSafari_EventScript_BaobaStrengthAlreadyOwned
	msgbox WayfarerFuchsiaSafari_Text_BaobaHasTeeth, MSGBOX_DEFAULT
	checkitemspace ITEM_HM04
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_BaobaNoRoom
	additem ITEM_HM04
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_BaobaNoRoom
	checkitem ITEM_HM04
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_BaobaNoRoom
	msgreceiveditem WayfarerFuchsiaSafari_Text_ReceivedStrength, ITEM_HM04, 1
	goto WayfarerFuchsiaSafari_EventScript_BaobaCommit

WayfarerFuchsiaSafari_EventScript_BaobaStrengthAlreadyOwned::
	msgbox WayfarerFuchsiaSafari_Text_BaobaStrengthAlreadyOwned, MSGBOX_DEFAULT

WayfarerFuchsiaSafari_EventScript_BaobaCommit::
	removeitem ITEM_GOLD_TEETH
	goto_if_eq VAR_RESULT, FALSE, WayfarerFuchsiaSafari_EventScript_BaobaNeedsTeeth
	setflag FLAG_FUCHSIA_SAFARI_GOLD_TEETH_CLAIMED_HNS
	setflag FLAG_FUCHSIA_SAFARI_STRENGTH_RECEIVED_HNS
	goto WayfarerFuchsiaSafari_EventScript_BaobaComplete

WayfarerFuchsiaSafari_EventScript_BaobaNeedsTeeth::
	msgbox WayfarerFuchsiaSafari_Text_BaobaNeedsTeeth, MSGBOX_DEFAULT
	release
	end

WayfarerFuchsiaSafari_EventScript_BaobaNoRoom::
	msgbox WayfarerFuchsiaSafari_Text_BaobaNoRoom, MSGBOX_DEFAULT
	release
	end

WayfarerFuchsiaSafari_EventScript_BaobaComplete::
	msgbox WayfarerFuchsiaSafari_Text_BaobaComplete, MSGBOX_DEFAULT
	release
	end

WayfarerFuchsiaSafari_Text_ReceivedGoldTeeth:
	.string "{PLAYER} received the GOLD TEETH!$"

WayfarerFuchsiaSafari_Text_GoldTeethAlreadyOwned:
	.string "You check your BAG.\p"
	.string "The GOLD TEETH are already inside.$"

WayfarerFuchsiaSafari_Text_GoldTeethNoRoom:
	.string "Your KEY ITEMS POCKET is full.\p"
	.string "Come back after making room.$"

WayfarerFuchsiaSafari_Text_SurfPrize:
	.string "You found DEVON's SAFARI prize!\p"
	.string "This is for reaching this habitat.$"

WayfarerFuchsiaSafari_Text_ReceivedSurf:
	.string "{PLAYER} received HM03\n"
	.string "from the DEVON attendant!$"

WayfarerFuchsiaSafari_Text_SurfAlreadyOwned:
	.string "You already have HM03, SURF.\p"
	.string "DEVON is glad you reached this habitat.$"

WayfarerFuchsiaSafari_Text_SurfNoRoom:
	.string "Your TM CASE is full.\p"
	.string "Come back after making room for\n"
	.string "this prize.$"

WayfarerFuchsiaSafari_Text_SurfComplete:
	.string "HM03 is SURF.\p"
	.string "A compatible POKéMON can ferry you\n"
	.string "across water.$"

WayfarerFuchsiaSafari_Text_BaobaHasTeeth:
	.string "BAOBA: Those are my GOLD TEETH!\p"
	.string "Let me thank you properly for\n"
	.string "bringing them back.$"

WayfarerFuchsiaSafari_Text_ReceivedStrength:
	.string "{PLAYER} received HM04\n"
	.string "from BAOBA!$"

WayfarerFuchsiaSafari_Text_BaobaStrengthAlreadyOwned:
	.string "BAOBA: You already carry HM04.\p"
	.string "Let me take my GOLD TEETH back.$"

WayfarerFuchsiaSafari_Text_BaobaNeedsTeeth:
	.string "BAOBA: I lost my GOLD TEETH while\n"
	.string "inspecting DEVON's SAFARI ZONE.\p"
	.string "Please look for them in the Beach\n"
	.string "habitat.$"

WayfarerFuchsiaSafari_Text_BaobaNoRoom:
	.string "BAOBA: Keep the GOLD TEETH for now.\p"
	.string "Make room for HM04, then return.$"

WayfarerFuchsiaSafari_Text_BaobaComplete:
	.string "BAOBA: The GOLD TEETH fit perfectly!\p"
	.string "Thank you, {PLAYER}!$"
