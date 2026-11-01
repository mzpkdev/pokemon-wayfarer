@ League invitations and results by phone (Wayfarer). Spec: .product/specs/leagues.md (Lifecycle, Presentation).
@ The field starts these when it has control (src/league_phone.c). The caller is the league's front desk, on the
@ POKéNAV call window; the answer is an ordinary yes/no box between the invitation and the reply, with no "later".
@ Call lines fit the call box: two lines of 192 pixels a page.
#if IS_WAYFARER && WAYFARER_LEAGUE_EVENTS
LeaguePhone_EventScript_Invitation::
	lockall
	specialvar VAR_0x8004, LeaguePhone_BufferInvitation
	pokenavcall LeaguePhone_Text_Invitation
	waitmessage
LeaguePhone_EventScript_AnswerPrompt:
	msgbox LeaguePhone_Text_AnswerPrompt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LeaguePhone_EventScript_ConfirmDecline
	specialvar VAR_RESULT, LeaguePhone_Accept
	goto_if_ne VAR_RESULT, LEAGUE_ACCEPT_OK, LeaguePhone_EventScript_AnswerFailed
	special LeaguePhone_SetCaller
	pokenavcall LeaguePhone_Text_Accepted
	waitmessage
	releaseall
	end

@ Declining can't be taken back, so a NO (or a B press) asks once more.
LeaguePhone_EventScript_ConfirmDecline:
	msgbox LeaguePhone_Text_ConfirmDecline, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LeaguePhone_EventScript_AnswerPrompt
	specialvar VAR_RESULT, LeaguePhone_Decline
	goto_if_eq VAR_RESULT, FALSE, LeaguePhone_EventScript_AnswerFailed
	special LeaguePhone_SetCaller
	pokenavcall LeaguePhone_Text_Declined
	waitmessage
	releaseall
	end

@ The lobby staff at Indigo or Hoenn hand over a waiting invitation (from either league) to a player who has
@ no phone, or whose ring is held after a failed answer. The answer and its reply are ordinary message boxes.
LeaguePhone_EventScript_DeskInvitation::
	specialvar VAR_0x8004, LeaguePhone_BufferDeskInvitation
	msgbox LeaguePhone_Text_DeskInvitation
LeaguePhone_EventScript_DeskPrompt:
	msgbox LeaguePhone_Text_AnswerPrompt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, LeaguePhone_EventScript_DeskAccept
	msgbox LeaguePhone_Text_ConfirmDecline, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LeaguePhone_EventScript_DeskPrompt
	specialvar VAR_RESULT, LeaguePhone_Decline
	goto_if_eq VAR_RESULT, FALSE, LeaguePhone_EventScript_AnswerFailed
	msgbox LeaguePhone_Text_Declined
	releaseall
	end

LeaguePhone_EventScript_DeskAccept:
	specialvar VAR_RESULT, LeaguePhone_Accept
	goto_if_ne VAR_RESULT, LEAGUE_ACCEPT_OK, LeaguePhone_EventScript_AnswerFailed
	msgbox LeaguePhone_Text_Accepted
	releaseall
	end

@ The answer wasn't saved: the invitation still waits, and rings again on another map or after a reload.
LeaguePhone_EventScript_AnswerFailed:
	msgbox LeaguePhone_Text_AnswerFailed
	releaseall
	end

LeaguePhone_EventScript_Result::
	lockall
	specialvar VAR_RESULT, LeaguePhone_BufferResult
	goto_if_eq VAR_RESULT, TRUE, LeaguePhone_EventScript_ResultWon
	pokenavcall LeaguePhone_Text_ResultLost
	goto LeaguePhone_EventScript_ResultDone

LeaguePhone_EventScript_ResultWon:
	pokenavcall LeaguePhone_Text_ResultWon
LeaguePhone_EventScript_ResultDone:
	waitmessage
	special LeaguePhone_FinishResult
	releaseall
	end

@ {STR_VAR_1} is the league ("INDIGO LEAGUE", "HOENN LEAGUE"), {STR_VAR_2} its place ("INDIGO PLATEAU",
@ "EVER GRANDE CITY"), {STR_VAR_3} the lineup or the reigning champion.
LeaguePhone_Text_Invitation:
	.string "Hello, {PLAYER}! This is the front\n"
	.string "desk at {STR_VAR_2}.\p"
	.string "The {STR_VAR_1} is holding its\n"
	.string "next event, and you're invited!\p"
	.string "Five top TRAINERS will battle.\n"
	.string "Will you take part?$"

LeaguePhone_Text_DeskInvitation:
	.string "{PLAYER}, an invitation is waiting\n"
	.string "for you from the {STR_VAR_1}!\p"
	.string "Its next event is ready, and\n"
	.string "you're invited to take part.$"

LeaguePhone_Text_AnswerPrompt:
	.string "Accept the {STR_VAR_1}'s\n"
	.string "invitation?$"

LeaguePhone_Text_ConfirmDecline:
	.string "Decline? The event will go ahead\n"
	.string "without you.$"

LeaguePhone_Text_Accepted:
	.string "Wonderful! Your opponents, in\n"
	.string "battle order:\p"
	.string "{STR_VAR_3}\p"
	.string "The event will wait for you at\n"
	.string "{STR_VAR_2}. Good luck!$"

LeaguePhone_Text_Declined:
	.string "Understood. The event will go\n"
	.string "ahead without you.\p"
	.string "…And it's over! {STR_VAR_3} won,\n"
	.string "and reigns as CHAMPION now.\p"
	.string "We'll call with the next\n"
	.string "invitation. Take care!$"

LeaguePhone_Text_ResultWon:
	.string "Congratulations, {PLAYER}!\p"
	.string "You won the event. You're the\n"
	.string "{STR_VAR_1}'s CHAMPION!\p"
	.string "We'll call when the next event\n"
	.string "is ready. Until then!$"

LeaguePhone_Text_ResultLost:
	.string "{PLAYER}, thank you for taking\n"
	.string "part in our event.\p"
	.string "{STR_VAR_3} won it, and reigns\n"
	.string "as CHAMPION now.\p"
	.string "We'll call when the next event\n"
	.string "is ready. Until then!$"

LeaguePhone_Text_AnswerFailed:
	.string "Your answer couldn't be recorded.\n"
	.string "Please try again in a while.$"
#endif
