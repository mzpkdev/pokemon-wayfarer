@ Elm's "Has that EGG hatched yet?" call, for Johto-origin players.
@
@ The call belongs to the Togepi Egg beat, not to a town. The Egg handoff in Violet's Pokemon Center
@ owes it, and the first Johto town the player loads after that plays it, in whatever order the towns
@ are reached. Violet itself is left out (the player just got the Egg there) and so is New Bark (Elm
@ is next door). Showing Elm the hatched Togepi in person first settles the call instead.
@
@ Each town's load script calls WayfarerJohto_EventScript_ArmElmEggCall, which sets VAR_TEMP_C, and
@ the town's frame table plays WayfarerJohto_EventScript_ElmEggCall on VAR_TEMP_C 1. The call clears
@ VAR_TEMP_C and the owed flag before anything else, so the frame script never waits.

WayfarerJohto_EventScript_OweElmEggCall::
	specialvar VAR_RESULT, WayfarerIsJohtoOrigin
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_NopReturn
	setflag FLAG_WAYFARER_JOHTO_ELM_EGG_CALL_OWED
	return

WayfarerJohto_EventScript_ArmElmEggCall::
	goto_if_unset FLAG_WAYFARER_JOHTO_ELM_EGG_CALL_OWED, Common_EventScript_NopReturn
	goto_if_set FLAG_SHOWN_ELM_TOGEPI, WayfarerJohto_EventScript_SettleElmEggCall
	setvar VAR_TEMP_C, 1
	return

WayfarerJohto_EventScript_SettleElmEggCall:
	clearflag FLAG_WAYFARER_JOHTO_ELM_EGG_CALL_OWED
	return

WayfarerJohto_EventScript_ElmEggCall::
	setvar VAR_TEMP_C, 0
	clearflag FLAG_WAYFARER_JOHTO_ELM_EGG_CALL_OWED
	lock
	setspeaker WayfarerJohto_Text_ElmName
	@ A Togepi in the party means the Egg has hatched (an Egg itself reads as SPECIES_EGG).
	checkspecies SPECIES_TOGEPI
	goto_if_eq VAR_RESULT, TRUE, WayfarerJohto_EventScript_ElmEggCallHatched
	checkspecies SPECIES_TOGETIC
	goto_if_eq VAR_RESULT, TRUE, WayfarerJohto_EventScript_ElmEggCallHatched
	checkspecies SPECIES_TOGEKISS
	goto_if_eq VAR_RESULT, TRUE, WayfarerJohto_EventScript_ElmEggCallHatched
	pokenavcall WayfarerJohto_Text_ElmEggCall
	goto WayfarerJohto_EventScript_ElmEggCallDone

WayfarerJohto_EventScript_ElmEggCallHatched:
	pokenavcall WayfarerJohto_Text_ElmEggCallHatched
WayfarerJohto_EventScript_ElmEggCallDone:
	waitmessage
	delay 30
	release
	end

WayfarerJohto_Text_ElmName:
	.string "PROF.ELM$"

WayfarerJohto_Text_ElmEggCall:
	.string "PROF.ELM: Hello, {PLAYER}?\n"
	.string "Has that EGG hatched yet?\p"
	.string "Once it hatches, don't forget to\n"
	.string "bring it by the lab! I need to\l"
	.string "see it!$"

WayfarerJohto_Text_ElmEggCallHatched:
	.string "PROF.ELM: Hello, {PLAYER}?\n"
	.string "How's that EGG doing?\p"
	.string "…It hatched? Really?\n"
	.string "That's wonderful!\p"
	.string "Please bring it by the lab!\n"
	.string "I need to see it!$"
