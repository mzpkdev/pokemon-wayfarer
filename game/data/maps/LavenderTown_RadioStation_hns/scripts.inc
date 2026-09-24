LavenderTown_RadioStation_hns_MapScripts::
	.byte 0

LavenderTown_RadioStation_EventScript_Officer::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_TOWER_FUJI_RESCUED, LavenderTown_RadioStation_EventScript_OfficerAfterRescue
	msgbox LavenderTown_RadioStation_Text_Officer, MSGBOX_DEFAULT
	goto LavenderTown_RadioStation_EventScript_OfficerOfferEntry

LavenderTown_RadioStation_EventScript_OfficerAfterRescue::
	msgbox LavenderTown_RadioStation_Text_OfficerAfterRescue, MSGBOX_DEFAULT

LavenderTown_RadioStation_EventScript_OfficerOfferEntry::
	msgbox LavenderTown_RadioStation_Text_OfficerEnter, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LavenderTown_RadioStation_EventScript_OfficerDecline
	closemessage
	warp MAP_POKEMON_TOWER_2F, 18, 11
	waitstate
	release
	end

LavenderTown_RadioStation_EventScript_OfficerDecline::
	release
	end

LavenderTown_RadioStation_EventScript_Receptionist::
	msgbox LavenderTown_RadioStation_Text_Receptionist, MSGBOX_NPC
	end

LavenderTown_RadioStation_EventScript_SuperNerd::
	msgbox LavenderTown_RadioStation_Text_SuperNerd1, MSGBOX_NPC
	end

LavenderTown_RadioStation_EventScript_Director::
	lock
	faceplayer
	goto_if_set FLAG_KANTO_RADIO_GOT, LavenderTown_RadioStation_EventScript_DirectorGotRadio
	goto_if_set FLAG_RETURNED_MACHINE_PART, LavenderTown_RadioStation_EventScript_DirectorReturnedMachinePart
	msgbox LavenderTown_RadioStation_Text_Gentleman, MSGBOX_NPC
	end
LavenderTown_RadioStation_EventScript_DirectorReturnedMachinePart::
	msgbox LavenderTown_RadioStation_Text_Gentleman_ReturnedMachinePart, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	msgbox LavendertownRadioStation_Text_Upgraded_Radio, MSGBOX_DEFAULT
	waitfanfare
	msgbox LavenderTown_RadioStation_Text_Gentleman_GotExpnCard, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_KANTO_RADIO_GOT
	release
	end
LavenderTown_RadioStation_EventScript_DirectorGotRadio::	
	msgbox LavenderTown_RadioStation_Text_Gentleman_GotExpnCard, MSGBOX_NPC
	end

LavenderTown_RadioStation_EventScript_Host::
	goto_if_set FLAG_KANTO_RADIO_GOT, LavenderTown_RadioStation_EventScript_Host2
	msgbox LavenderTown_RadioStation_Text_SuperNerd2, MSGBOX_NPC
	end
LavenderTown_RadioStation_EventScript_Host2::
	msgbox LavenderTown_RadioStation_Text_SuperNerd2_GotExpnCard, MSGBOX_NPC
	end

LavenderTown_RadioStation_EventScript_Directory::
	msgbox LavenderTown_RadioStation_Text_Directory, MSGBOX_SIGN
	end
LavenderTown_RadioStation_EventScript_Sign::
	msgbox LavenderTown_RadioStation_Text_PokeFluteSign, MSGBOX_SIGN
	end
LavenderTown_RadioStation_EventScript_Library::
	msgbox LavenderTown_RadioStation_Text_ReferenceLibrary, MSGBOX_SIGN
	end

LavenderTown_RadioStation_Text_Receptionist:
	.string "Welcome! Feel free to look around\n"
	.string "anywhere on this floor.$"

LavenderTown_RadioStation_Text_Officer:
	.string "The ground floor is being fitted\n"
	.string "for radio. The memorial floors\l"
	.string "above are still open.\p"
	.string "MR. FUJI was helping move some\n"
	.string "memorials to the SOUL HOUSE, but\l"
	.string "he's gone missing. The work has\l"
	.string "stopped while spirits stir.$"

LavenderTown_RadioStation_Text_OfficerAfterRescue:
	.string "MR. FUJI is safe at his home.\n"
	.string "The memorial floors remain open\l"
	.string "while we finish the radio work.$"

LavenderTown_RadioStation_Text_OfficerEnter:
	.string "Would you like to visit the\n"
	.string "memorial floors upstairs?$"

LavenderTown_RadioStation_Text_SuperNerd1:
	.string "The radio crew works down here\n"
	.string "while the memorial floors stay\l"
	.string "open above us.\p"
	.string "The move to the SOUL HOUSE is\n"
	.string "paused for now.$"

LavenderTown_RadioStation_Text_Gentleman:
	.string "Oh, no, no, no!\p"
	.string "We've been off the air ever since\n"
	.string "the POWER PLANT shut down.\p"
	.string "All my efforts to start this\n"
	.string "station would be wasted if I can't\l"
	.string "broadcast.\p"
	.string "I'll be ruined!$"

LavenderTown_RadioStation_Text_Gentleman_ReturnedMachinePart:
	.string "Ah! So you're the {PLAYER} who\n"
	.string "solved the POWER PLANT's problem?\p"
	.string "Thanks to you, I never lost my\n"
	.string "job.\p"
	.string "I tell you, you're a real\n"
	.string "lifesaver!\p"
	.string "Please take this as my thanks.$"

LavendertownRadioStation_Text_Upgraded_Radio:
	.string "{PLAYER} obtained the\n"
	.string "RADIO UPGRADE!$"

LavenderTown_RadioStation_Text_Gentleman_GotExpnCard:
	.string "With that thing, you can tune into\n"
	.string "the radio programs here in KANTO.\p"
	.string "Gahahahaha!$"

LavenderTown_RadioStation_Text_SuperNerd2:
	.string "Hey there!\p"
	.string "I am the super MUSIC DIRECTOR!\p"
	.string "Huh? Your RADIO can't tune into\n" @POKéGEAR
	.string "my music programs. How unfortunate!\p"
	.string "If you get an EXPN CARD upgrade,\n"
	.string "you can tune in. You'd better get\l"
	.string "one!$"

LavenderTown_RadioStation_Text_SuperNerd2_GotExpnCard:
	.string "Hey there!\p"
	.string "I am the super MUSIC DIRECTOR!\p"
	.string "I'm responsible for the gorgeous\n"
	.string "melodies that go out over the air.\p"
	.string "Don't be square. Grab your music\n"
	.string "off the air!$"

LavenderTown_RadioStation_Text_Directory:
	.string "GROUND FLOOR: RADIO RECEPTION\p"
	.string "2F-7F: POKéMON MEMORIAL\n"
	.string "Access: ask the attendant.$"

LavenderTown_RadioStation_Text_PokeFluteSign:
	.string "Perk Up POKéMON with Mellow\n"
	.string "Sounds of the POKé FLUTE on\l"
	.string "CHANNEL 20$"

LavenderTown_RadioStation_Text_ReferenceLibrary:
	.string "Wow! A full rack of POKéMON CDs\n"
	.string "and videos.\p"
	.string "This must be the reference\n"
	.string "library.$"
