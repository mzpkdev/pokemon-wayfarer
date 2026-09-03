SSAqua_RoomSSE_hns_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, SSAqua_RoomSSE_OnFrame
    map_script MAP_SCRIPT_ON_TRANSITION, SSAqua_RoomSSE_OnTransition
	.byte 0

SSAqua_RoomSSE_OnTransition::
    goto_if_ge VAR_SSAQUA_STATE, 4, SSAqua_RoomSSE_SetGirlPos
    end

SSAqua_RoomSSE_OnFrame::
	map_script_2 VAR_SSAQUA_STATE, 3, SSAqua_RoomSSE_EventScript_ReunionScene
	.2byte 0

SSAqua_RoomSSE_SetGirlPos::
    setobjectxyperm LOCALID_SSAQUA_ROOM_SSE_GRANDDAUGTHER, 2, 4
    setobjectmovementtype LOCALID_SSAQUA_ROOM_SSE_GRANDDAUGTHER, MOVEMENT_TYPE_FACE_LEFT
    end

SSAqua_RoomSSE_EventScript_ReunionScene::
	lock
    turnobject OBJ_EVENT_ID_PLAYER, DIR_NORTH
    turnobject LOCALID_SSAQUA_ROOM_SSE_GRANDFATHER, DIR_SOUTH
    playse SE_PIN
    applymovement LOCALID_SSAQUA_ROOM_SSE_GRANDFATHER, Common_Movement_ExclamationMark
    delay 55
    waitmovement LOCALID_SSAQUA_ROOM_SSE_GRANDFATHER
	applymovement LOCALID_SSAQUA_ROOM_SSE_GRANDDAUGTHER, SSAqua_RoomSSE_Movement_GirlEnter
    waitmovement 0
    turnobject LOCALID_SSAQUA_ROOM_SSE_GRANDFATHER, DIR_EAST
    msgbox SSAqua_RoomSSE_Text_GranddaughterWasPlaying, MSGBOX_DEFAULT
    closemessage
    applymovement OBJ_EVENT_ID_PLAYER, SSAqua_RoomSSE_Movement_PlayerEnter
    waitmovement 0
    turnobject LOCALID_SSAQUA_ROOM_SSE_GRANDFATHER, DIR_SOUTH
    msgbox SSAqua_RoomSSE_Text_EntertainedGranddaughter, MSGBOX_DEFAULT
	closemessage
	setvar VAR_SSAQUA_STATE, 4
	call SSAqua_RoomSSE_EventScript_GivePendingReward
	closemessage
	release
	end

SSAqua_RoomSSE_EventScript_Grandpa::
	lock
	faceplayer
	goto_if_eq VAR_SSAQUA_STATE, 4, SSAqua_RoomSSE_EventScript_GrandpaGivePendingReward
	goto_if_eq VAR_SSAQUA_STATE, 5, SSAqua_RoomSSE_EventScript_GrandpaGivePendingReward
	goto_if_ge VAR_SSAQUA_STATE, 6, SSAqua_RoomSSE_EventScript_GrandpaAfter
	msgbox SSAqua_RoomSSE_Text_CantFindGranddaughter, MSGBOX_DEFAULT
	closemessage
	release
	end

SSAqua_RoomSSE_EventScript_GrandpaGivePendingReward::
	call SSAqua_RoomSSE_EventScript_GivePendingReward
	closemessage
	release
	end

SSAqua_RoomSSE_EventScript_GivePendingReward::
	goto_if_eq VAR_SSAQUA_STATE, 4, SSAqua_RoomSSE_EventScript_GiveTicket
	goto_if_eq VAR_SSAQUA_STATE, 5, SSAqua_RoomSSE_EventScript_GiveMetalCoat
	return

SSAqua_RoomSSE_EventScript_GiveTicket::
	checkitem ITEM_SS_TICKET
	goto_if_eq VAR_RESULT, TRUE, SSAqua_RoomSSE_EventScript_TicketCommitted
	checkitemspace ITEM_SS_TICKET
	goto_if_eq VAR_RESULT, FALSE, SSAqua_RoomSSE_EventScript_MakeRoom
	msgbox SSAqua_RoomSSE_Text_GrandpaHaveThis, MSGBOX_DEFAULT
	giveitem ITEM_SS_TICKET
	goto_if_eq VAR_RESULT, FALSE, SSAqua_RoomSSE_EventScript_MakeRoom
SSAqua_RoomSSE_EventScript_TicketCommitted::
	setvar VAR_SSAQUA_STATE, 5
	goto SSAqua_RoomSSE_EventScript_GiveMetalCoat

SSAqua_RoomSSE_EventScript_GiveMetalCoat::
	checkitemspace ITEM_METAL_COAT
	goto_if_eq VAR_RESULT, FALSE, SSAqua_RoomSSE_EventScript_MakeRoom
	msgbox SSAqua_RoomSSE_Text_GrandpaHaveThisToo, MSGBOX_DEFAULT
	giveitem ITEM_METAL_COAT
	goto_if_eq VAR_RESULT, FALSE, SSAqua_RoomSSE_EventScript_MakeRoom
	setvar VAR_SSAQUA_STATE, 6
	return

SSAqua_RoomSSE_EventScript_MakeRoom::
	msgbox SSAqua_RoomSSE_Text_MakeRoom, MSGBOX_DEFAULT
	return

SSAqua_RoomSSE_EventScript_GrandpaAfter::
	msgbox SSAqua_RoomSSE_Text_GrandpaTravelling, MSGBOX_DEFAULT
	closemessage
	release
	end

SSAqua_RoomSSE_EventScript_GrandDaughter::
	msgbox SSAqua_RoomSSE_Text_GranddaughterHadFun, MSGBOX_NPC
	end

SSAqua_RoomSSE_Movement_GirlEnter:
    walk_up
    walk_up
    walk_left
    step_end

SSAqua_RoomSSE_Movement_PlayerEnter:
    walk_up
    walk_left
    walk_left
    walk_up
    step_end

SSAqua_RoomSSE_Text_CantFindGranddaughter:
    .string "Oh, hello…\p"
    .string "I still can't find my\n"
    .string "granddaughter.\p"
    .string "If she's on the ship,\n"
    .string "that's OK.\p"
    .string "She's an energetic child,\n"
    .string "so she may be bugging someone\l"
    .string "I'm worried…$"

SSAqua_RoomSSE_Text_EntertainedGranddaughter:
    .string "{PLAYER}, was it?\n"
    .string "I heard you entertained my\l"
    .string "granddaughter.\p"
    .string "I want to thank you for that.$"

SSAqua_RoomSSE_Text_GrandpaHaveThis:
    .string "I know! I'd like you\n"
    .string "to have this!$"

SSAqua_RoomSSE_Text_GrandpaHaveThisToo:
    .string "I have one more gift for you.$"

SSAqua_RoomSSE_Text_MakeRoom:
    .string "It looks like your BAG is full.\p"
    .string "Please make room and talk to me\n"
    .string "again.$"

SSAqua_RoomSSE_Text_GrandpaTravelling:
    .string "We're traveling around\n"
    .string "the world.$"

SSAqua_RoomSSE_Text_GranddaughterWasPlaying:
    .string "Grandpa, here I am!\n"
    .string "I was playing with\l"
    .string "the CAPTAIN and this person!$"

SSAqua_RoomSSE_Text_GranddaughterHadFun:
    .string "I had lots of fun playing!$"
