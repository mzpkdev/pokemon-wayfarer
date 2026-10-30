@ Notable trainer talks and the friendship number handover (Wayfarer only).
@ Specs: notable-trainers.md#friendship, notable-spots.md#talk-flow, notable-world-simulation.md#interactions.
@ C side: src/notable_talk.c, src/notable_friendship.c. See include/notable_talk.h for how a Gym script calls
@ the greeting and the farewell around its own line.

@ A walker actor the player talked to: greeting by friendship, the line of the activity it is doing or heading for,
@ and the farewell. Talking adds no friendship. Attached to walker actors in field_control_avatar.c.
NotableTalk_EventScript_WalkerTalk::
	lock
	faceplayer
	specialvar VAR_RESULT, NotableTalk_PrepareWalker
	goto_if_eq VAR_RESULT, FALSE, NotableTalk_EventScript_WalkerTalkEnd
	call NotableTalk_EventScript_Greeting
	special NotableTalk_BufferWalkerActivityLine
	msgbox gStringVar4, MSGBOX_DEFAULT
	call NotableTalk_EventScript_Farewell
NotableTalk_EventScript_WalkerTalkEnd:
	release
	end

@ The stage greeting of the character in VAR_0x8004 (MEET or HEARD for a Stranger, AGAIN, HELLO, CLOSE).
NotableTalk_EventScript_Greeting::
	specialvar VAR_RESULT, NotableTalk_BufferGreeting
	goto_if_eq VAR_RESULT, FALSE, NotableTalk_EventScript_NothingToSay
	msgbox gStringVar4, MSGBOX_DEFAULT
	return

@ The farewell (BYE) of the character in VAR_0x8004, then the message closes.
NotableTalk_EventScript_Farewell::
	specialvar VAR_RESULT, NotableTalk_BufferFarewell
	goto_if_eq VAR_RESULT, FALSE, NotableTalk_EventScript_NothingToSay
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	return

NotableTalk_EventScript_NothingToSay:
	return

@ The number handover: shown by the field when it regains control after a score crossed the Friend threshold.
@ NotableFriendship_TryStartPendingNumberScript puts the character in VAR_0x8004.
NotableFriendship_EventScript_NumberRegistered::
	lockall
	special NotableFriendship_BufferTrainerName
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox NotableFriendship_Text_NumberRegistered, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	releaseall
	end

NotableFriendship_Text_NumberRegistered:
	.string "Registered {STR_VAR_1}'s number\n"
	.string "in the POKéGEAR.$"
