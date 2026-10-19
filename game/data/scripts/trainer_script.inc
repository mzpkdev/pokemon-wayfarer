#if IS_WAYFARER
@ Wayfarer asks before it registers (phone-rematches.md, Getting a number). The trainer in VAR_0x8000 (any team) offers its
@ number: yes registers it with the usual message, no leaves it unregistered and the offer comes again after the next win.
@ Both register_matchcall and the after-battle offer below end up here.
Std_RegisteredInMatchCall::
	specialvar VAR_RESULT, PhoneOffer_CanOffer
	goto_if_eq VAR_RESULT, FALSE, Std_OfferMatchCall_End
	buffertrainerclassname STR_VAR_1, VAR_0x8000
	buffertrainername STR_VAR_2, VAR_0x8000
	msgbox gText_OfferTrainerNumber, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, FALSE, Std_OfferMatchCall_Declined
	special PhoneOffer_Accept
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox gText_RegisteredTrainerinPokeNav, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	return

Std_OfferMatchCall_Declined:
	closemessage
Std_OfferMatchCall_End:
	return

@ The offer after a win whose trainer has no after-battle script of its own: a wanderer, or a contact without one.
PhoneOffer_EventScript_AfterBattle::
	special PlayerFaceTrainerAfterBattle
	waitmovement 0
	special PhoneOffer_BufferTarget
	callstd STD_REGISTER_MATCH_CALL
	releaseall
	end

gText_OfferTrainerNumber::
	.string "Register {STR_VAR_1} {STR_VAR_2}\n"
	.string "in the POKéGEAR?$"
#else
Std_RegisteredInMatchCall::
	buffertrainerclassname STR_VAR_1, VAR_0x8000
	buffertrainername STR_VAR_2, VAR_0x8000
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox gText_RegisteredTrainerinPokeNav, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	return
#endif

EventScript_TryGetTrainerScript::
	special ShouldTryGetTrainerScript
	goto_if_eq VAR_RESULT, TRUE, EventScript_GotoTrainerScript
	releaseall
	end

EventScript_GotoTrainerScript::
	gotobeatenscript
	releaseall
	end

EventScript_ObjectApproachPlayer::
	lock
	special DoTrainerApproach
	waitstate
	gotonative LoadTrainerObjectScript
