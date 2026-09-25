@ The registered Seven Island battle house owns Masters admission.
@ FRLG visitor and e-Reader handlers are not linked in Wayfarer.

WayfarerSevii_Masters_Room1OnTransition::
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	call_if_eq VAR_RESULT, TRUE, WayfarerSevii_Masters_OpenBox
	end

WayfarerSevii_Masters_OpenBox::
	setobjectxyperm LOCALID_SEVEN_ISLAND_HOUSE_OLD_WOMAN, 3, 2
	setobjectmovementtype LOCALID_SEVEN_ISLAND_HOUSE_OLD_WOMAN, MOVEMENT_TYPE_FACE_RIGHT
	setmaplayoutindex LAYOUT_SEVEN_ISLAND_HOUSE_ROOM1_DOOR_OPEN
	return

WayfarerSevii_Masters_OldWoman::
	lock
	faceplayer
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Masters_Denied
	msgbox WayfarerSevii_Masters_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Masters_Declined
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	specialvar VAR_RESULT, LeagueCircuit_BeginRun
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Masters_Denied
	call WayfarerSevii_Masters_OpenBox
	msgbox WayfarerSevii_Masters_Text_Admitted
	warp MAP_SEVEN_ISLAND_HOUSE_ROOM2, 3, 2
	waitstate
	release
	end

WayfarerSevii_Masters_Denied::
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	special LeagueCircuit_BufferAdmissionDenial
	msgbox gStringVar4
	release
	end

WayfarerSevii_Masters_Declined::
	msgbox WayfarerSevii_Masters_Text_Declined
	release
	end

WayfarerSevii_Masters_Box::
	setvar VAR_0x8004, CIRCUIT_STAGE_MASTERS
	specialvar VAR_RESULT, LeagueCircuit_IsEligible
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Masters_BoxClosed
	msgbox WayfarerSevii_Masters_Text_BoxOpen, MSGBOX_SIGN
	end

WayfarerSevii_Masters_BoxClosed::
	msgbox WayfarerSevii_Masters_Text_BoxClosed, MSGBOX_SIGN
	end

WayfarerSevii_Masters_Room2OnFrame::
	map_script_2 VAR_TEMP_1, 0, WayfarerSevii_Masters_Room2Forward
	.2byte 0

WayfarerSevii_Masters_Room2Forward::
	lockall
	setvar VAR_TEMP_1, 1
	specialvar VAR_RESULT, LeagueCircuit_GetRunStage
	goto_if_ne VAR_RESULT, CIRCUIT_STAGE_MASTERS, WayfarerSevii_Masters_Room2Stay
	warp MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS, 6, 12
	waitstate
	releaseall
	end

WayfarerSevii_Masters_Room2Stay::
	releaseall
	end

WayfarerSevii_Masters_Text_Offer::
	.string "Five invited MASTERS await beyond\n"
	.string "the hidden door.\p"
	.string "Would you like to challenge them?$"
WayfarerSevii_Masters_Text_Admitted::
	.string "The box is aside. Good luck!$"
WayfarerSevii_Masters_Text_Declined::
	.string "Come back whenever you're ready.$"
WayfarerSevii_Masters_Text_BoxClosed::
	.string "A heavy box hides a door.$"
WayfarerSevii_Masters_Text_BoxOpen::
	.string "The hidden door is open. Speak to\n"
	.string "the caretaker to enter.$"
