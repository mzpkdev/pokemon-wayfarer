PalletTown_RedsHouse_1F_hns_MapScripts::
	.byte 0

PalletTown_RedsHouse_1F_EventScript_Mom::
	lock
	faceplayer
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_RedsHouse_1F_EventScript_KantoMom
#endif
	goto_if_ge VAR_TEMP_0, 1, PalletTown_RedsHouse_1F_EventScript_MomAfter
	msgbox RedsHouse_Text_Mom1, MSGBOX_DEFAULT
	closemessage
	setvar VAR_TEMP_0, 1
	release
	end

#if IS_WAYFARER
@ FRLG PalletTown_PlayersHouse_1F_EventScript_Mom. FLAG_BEAT_RIVAL_IN_OAKS_LAB is
@ the Wayfarer first-battle receipt, set on either battle outcome.
PalletTown_RedsHouse_1F_EventScript_KantoMom::
	setvar VAR_0x8004, PALLET_RECEIPT_FIRST_BATTLE
	specialvar VAR_RESULT, WayfarerKanto_HasReceipt
	goto_if_eq VAR_RESULT, TRUE, PalletTown_RedsHouse_1F_EventScript_KantoMomHeal
	checkplayergender
	call_if_eq VAR_RESULT, MALE, PalletTown_RedsHouse_1F_EventScript_KantoMomOakLookingForYouMale
	call_if_eq VAR_RESULT, FEMALE, PalletTown_RedsHouse_1F_EventScript_KantoMomOakLookingForYouFemale
	closemessage
	applymovement LOCALID_REDS_HOUSE_1F_MOM, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

PalletTown_RedsHouse_1F_EventScript_KantoMomOakLookingForYouMale:
	msgbox PalletTown_RedsHouse_1F_Text_AllBoysLeaveOakLookingForYou
	return

PalletTown_RedsHouse_1F_EventScript_KantoMomOakLookingForYouFemale:
	msgbox PalletTown_RedsHouse_1F_Text_AllGirlsLeaveOakLookingForYou
	return

PalletTown_RedsHouse_1F_EventScript_KantoMomHeal:
	msgbox PalletTown_RedsHouse_1F_Text_YouShouldTakeQuickRest
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	msgbox PalletTown_RedsHouse_1F_Text_LookingGreatTakeCare
	release
	end

@ FRLG PalletTown_PlayersHouse_1F_EventScript_TV. HNS geometry only exposes the
@ screen from the south, so the wrong-side branch is kept but unreachable.
PalletTown_RedsHouse_1F_EventScript_KantoTV:
	lockall
	goto_if_eq VAR_FACING, DIR_NORTH, PalletTown_RedsHouse_1F_EventScript_KantoTVScreen
	msgbox PalletTown_RedsHouse_1F_Text_OopsWrongSide
	releaseall
	end

PalletTown_RedsHouse_1F_EventScript_KantoTVScreen:
	checkplayergender
	call_if_eq VAR_RESULT, MALE, PalletTown_RedsHouse_1F_EventScript_KantoTVScreenMale
	call_if_eq VAR_RESULT, FEMALE, PalletTown_RedsHouse_1F_EventScript_KantoTVScreenFemale
	releaseall
	end

PalletTown_RedsHouse_1F_EventScript_KantoTVScreenMale:
	msgbox PalletTown_RedsHouse_1F_Text_MovieOnTVFourBoysOnRailroad
	return

PalletTown_RedsHouse_1F_EventScript_KantoTVScreenFemale:
	msgbox PalletTown_RedsHouse_1F_Text_MovieOnTVGirlOnBrickRoad
	return

PalletTown_RedsHouse_1F_Text_AllBoysLeaveOakLookingForYou:
	.string "MOM: …Right.\n"
	.string "All boys leave home someday.\l"
	.string "It said so on TV.\p"
	.string "Oh, yes. PROF. OAK, next door, was\n"
	.string "looking for you.$"

PalletTown_RedsHouse_1F_Text_AllGirlsLeaveOakLookingForYou:
	.string "MOM: …Right.\n"
	.string "All girls dream of traveling.\l"
	.string "It said so on TV.\p"
	.string "Oh, yes. PROF. OAK, next door, was\n"
	.string "looking for you.$"

PalletTown_RedsHouse_1F_Text_YouShouldTakeQuickRest:
	.string "MOM: {PLAYER}!\n"
	.string "You should take a quick rest.$"

PalletTown_RedsHouse_1F_Text_LookingGreatTakeCare:
	.string "MOM: Oh, good! You and your\n"
	.string "POKéMON are looking great.\l"
	.string "Take care now!$"

PalletTown_RedsHouse_1F_Text_MovieOnTVFourBoysOnRailroad:
	.string "There's a movie on TV.\n"
	.string "Four boys are walking on railroad\l"
	.string "tracks.\p"
	.string "…I better go, too.$"

PalletTown_RedsHouse_1F_Text_MovieOnTVGirlOnBrickRoad:
	.string "There's a movie on TV.\n"
	.string "A girl with her hair in pigtails is\l"
	.string "walking up a brick road.\p"
	.string "…I better go, too.$"

PalletTown_RedsHouse_1F_Text_OopsWrongSide:
	.string "Oops, wrong side…$"
#endif

PalletTown_RedsHouse_1F_EventScript_MomAfter::
	msgbox RedsHouse_Text_Mom2, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_RedsHouse_1F_EventScript_TV::
#if IS_WAYFARER
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, PalletTown_RedsHouse_1F_EventScript_KantoTV
#endif
	msgbox RedsHouse_Text_TV, MSGBOX_SIGN
	end

RedsHouse_Text_Mom1:
	.string "Hi!\p"
	.string "RED's been away for a long time.\p"
	.string "He hasn't called either, so I have\n"
	.string "no idea where he is or what he's\l"
	.string "been doing.\p"
	.string "They say that no word is proof that\n"
	.string "he's doing fine, but I do worry\l"
	.string "about him.$"

RedsHouse_Text_Mom2:
	.string "I worry about RED getting hurt or\n"
	.string "sick, but he's a boy.\p"
	.string "I'm proud that he is doing what he\n"
	.string "wants to do.$"

RedsHouse_Text_TV:
	.string "They have programs that aren't\n"
	.string "shown in JOHTO…$"
