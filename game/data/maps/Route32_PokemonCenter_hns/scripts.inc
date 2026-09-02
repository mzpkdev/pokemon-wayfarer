Route32_PokemonCenter_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route32_PokemonCenter_OnTransition	
	.byte 0

Route32_PokemonCenter_OnTransition::
	setrespawn HEAL_LOCATION_ROUTE_32_HNS
	end

Route32_PokemonCenter_EventScript_Lass::
	lock
	faceplayer
	msgbox Route32_Pokecenter_Text_CooltrainerF, MSGBOX_DEFAULT
	closemessage
	release
	end

Route32_PokemonCenter_EventScript_FishingGuru::
	lock
	faceplayer
	goto_if_set FLAG_STANDARD_ROD_ROUTE32_CONTRIBUTED, Route32_PokemonCenter_EventScript_FishingGuru3
	msgbox Route32_Pokecenter_Text_FishingGuruQuestion, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, TRUE, Route32_PokemonCenter_EventScript_FishingGuru2
	msgbox Route32_Pokecenter_Text_FishingGuruNo, MSGBOX_DEFAULT
	closemessage
	release
	end
Route32_PokemonCenter_EventScript_FishingGuru2::
	msgbox Route32_Pokecenter_Text_FishingGuruYes, MSGBOX_DEFAULT
	setvar VAR_0x8004, FLAG_STANDARD_ROD_ROUTE32_CONTRIBUTED
	special Script_TryAwardStandardRod
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_ALREADY_CONTRIBUTED, Route32_PokemonCenter_EventScript_FishingGuru3
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_NO_SPACE, Common_EventScript_ShowBagIsFull
	goto_if_eq VAR_RESULT, STANDARD_ROD_AWARD_INVALID_STATE, Route32_PokemonCenter_EventScript_RodUpgradeFailed
	copyvar VAR_0x8000, VAR_0x8005
	setvar VAR_0x8001, 1
	setvar VAR_0x8007, TRUE
	call EventScript_ObtainItemMessage
	goto_if_eq VAR_0x8005, ITEM_OLD_ROD, Route32_PokemonCenter_EventScript_ReceivedOldRod
	goto_if_eq VAR_0x8005, ITEM_GOOD_ROD, Route32_PokemonCenter_EventScript_ReceivedGoodRod
	msgbox Route32_Pokecenter_Text_CompletedSuperRod, MSGBOX_DEFAULT
	goto Route32_PokemonCenter_EventScript_FishingGuruDone

Route32_PokemonCenter_EventScript_ReceivedOldRod::
	msgbox Route32_Pokecenter_Text_StartedWithOldRod, MSGBOX_DEFAULT
	goto Route32_PokemonCenter_EventScript_FishingGuruDone

Route32_PokemonCenter_EventScript_ReceivedGoodRod::
	msgbox Route32_Pokecenter_Text_ImprovedToGoodRod, MSGBOX_DEFAULT

Route32_PokemonCenter_EventScript_FishingGuruDone::
	msgbox Route32_Pokecenter_Text_FishingGuruGiveRod, MSGBOX_DEFAULT
	closemessage
	release
	end

Route32_PokemonCenter_EventScript_RodUpgradeFailed::
	msgbox Route32_Pokecenter_Text_RodUpgradeFailed, MSGBOX_DEFAULT
	closemessage
	release
	end
Route32_PokemonCenter_EventScript_FishingGuru3::
	msgbox Route32_Pokecenter_Text_FishingGuruAfter, MSGBOX_DEFAULT
	release
	end



Route32_Pokecenter_Text_FishingGuruQuestion:
	.string "This is a great fishing spot.\p"
	.string "You saw people fishing?\n"
	.string "How about you?\p"
	.string "Would you like one of my RODS?$"

Route32_Pokecenter_Text_FishingGuruYes:
	.string "Heh, that's good to hear.\p"
	.string "Now you're an angler too!$"

Route32_Pokecenter_Text_FishingGuruGiveRod:
	.string "Fishing is great!\p"
	.string "If there's water, be it the sea or\n"
	.string "a stream, try out your ROD.$"

Route32_Pokecenter_Text_StartedWithOldRod:
	.string "That OLD ROD can catch any POKéMON\n"
	.string "living in the local waters.$"

Route32_Pokecenter_Text_ImprovedToGoodRod:
	.string "You've got some angling experience!\n"
	.string "The GOOD ROD gets more bites and gives\l"
	.string "uncommon catches better odds.$"

Route32_Pokecenter_Text_CompletedSuperRod:
	.string "A seasoned angler! The SUPER ROD gives\n"
	.string "rare catches and bites their best odds.$"

Route32_Pokecenter_Text_RodUpgradeFailed:
	.string "Something isn't right with your fishing\n"
	.string "gear. Come back once it's sorted out.$"

Route32_Pokecenter_Text_FishingGuruNo:
	.string "Oh. That's rather disappointing…$"

Route32_Pokecenter_Text_FishingGuruAfter:
	.string "Yo, kid. How are they biting?$"

Route32_Pokecenter_Text_CooltrainerF:
	.string "What should I make my POKéMON hold?\p"
	.string "Maybe an item that increases\n"
	.string "ATTACK power…$"
