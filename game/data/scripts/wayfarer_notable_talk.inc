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

@ A story scene's introduction of the character in VAR_0x8004 (notable-story-continuity.md#2-introductions), used
@ through the notable_introduce macro, which also puts the scene's first-meeting text in the script data register.
@ A trainer the player has not met yet plays that text and is marked introduced. One already known gives their
@ stage greeting (AGAIN or warmer) in its place. The scene then plays its shared remainder.
NotableStory_EventScript_Introduce::
	goto_if_eq VAR_0x8004, NOTABLE_TRAINER_BLUE, NotableStory_EventScript_IntroduceBlue
	specialvar VAR_RESULT, NotableStory_IsIntroduced
	goto_if_eq VAR_RESULT, TRUE, NotableTalk_EventScript_Greeting
	callstd MSGBOX_DEFAULT
	special NotableStory_MarkIntroduced
	return

@ BLUE grew up beside a player from PALLET TOWN, as their rival: he introduces himself to nobody from there. They
@ count as introduced without a word; everyone else is introduced as usual.
NotableStory_EventScript_IntroduceBlue::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, TRUE, NotableStory_EventScript_RivalKnown
	specialvar VAR_RESULT, NotableStory_IsIntroduced
	goto_if_eq VAR_RESULT, TRUE, NotableTalk_EventScript_Greeting
	callstd MSGBOX_DEFAULT
	special NotableStory_MarkIntroduced
	return

NotableStory_EventScript_RivalKnown:
	special NotableStory_MarkIntroduced
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

@ The number handover: shown by the field when it has control while a trainer at Friend or above is still owed the
@ line. NotableFriendship_TryStartPendingNumberScript puts the character in VAR_0x8004. The saved number-shown bit is
@ set once the line has played, so a save or reset before then shows it again later.
NotableFriendship_EventScript_NumberRegistered::
	lockall
	special NotableFriendship_BufferTrainerName
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox NotableFriendship_Text_NumberRegistered, MSGBOX_DEFAULT
	waitfanfare
	special NotableFriendship_MarkNumberShown
	closemessage
	delay 30
	releaseall
	end

NotableFriendship_Text_NumberRegistered:
	.string "Registered {STR_VAR_1}'s number\n"
	.string "in the POKéGEAR.$"
