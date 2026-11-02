@ The S.S. ferries (Wayfarer only). Spec: .product/specs/ss-ferries.md.
@ C side: src/wayfarer_ferry.c. The schedule, the saved voyage state and the checks live there; these
@ scripts only talk, walk and call the specials. They are shared by the HnS-bank and Hoenn-bank maps.

@ ---- The gangway sailor ----

@ A sailor's talk, shared by every dock and launch port. The caller has run lock and faceplayer, and sets
@ VAR_0x8004 to FERRY_BERTH_DOCK or FERRY_BERTH_LAUNCH. Returns VAR_RESULT TRUE once the voyage state is set and
@ the caller should walk the player aboard (the offered ship is snapshotted in VAR_0x8005); FALSE after any refusal (the sailor has already said why).
Ferry_EventScript_SailorTalk::
	specialvar VAR_RESULT, Ferry_GetBerthShip
	copyvar VAR_0x8005, VAR_RESULT
	goto_if_eq VAR_RESULT, FERRY_SHIP_NONE, Ferry_EventScript_SailorNoShip
	checkitem ITEM_SS_TICKET
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_SailorNoTicket
	specialvar VAR_RESULT, Ferry_BufferBoarding
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_SailorCantBoard
	goto_if_eq VAR_0x8004, FERRY_BERTH_LAUNCH, Ferry_EventScript_SailorWelcomeLaunch
	msgbox Ferry_Text_WelcomeAboard, MSGBOX_DEFAULT
	goto Ferry_EventScript_SailorBoard

Ferry_EventScript_SailorWelcomeLaunch::
	msgbox Ferry_Text_WelcomeAboardLaunch, MSGBOX_DEFAULT
Ferry_EventScript_SailorBoard::
	closemessage
	specialvar VAR_RESULT, Ferry_Board
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_SailorCantBoard
	return

Ferry_EventScript_SailorNoShip::
	specialvar VAR_RESULT, Ferry_BufferNextShip
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_SailorNoService
	goto_if_eq VAR_0x8004, FERRY_BERTH_LAUNCH, Ferry_EventScript_SailorNoShipLaunch
	msgbox Ferry_Text_NextShip, MSGBOX_DEFAULT
	goto Ferry_EventScript_SailorRefused

Ferry_EventScript_SailorNoShipLaunch::
	msgbox Ferry_Text_NextShipLaunch, MSGBOX_DEFAULT
	goto Ferry_EventScript_SailorRefused

Ferry_EventScript_SailorNoService::
	msgbox Ferry_Text_NoService, MSGBOX_DEFAULT
	goto Ferry_EventScript_SailorRefused

Ferry_EventScript_SailorNoTicket::
	bufferitemname STR_VAR_1, ITEM_SS_TICKET
	msgbox Ferry_Text_NeedTicket, MSGBOX_DEFAULT
	goto Ferry_EventScript_SailorRefused

Ferry_EventScript_SailorCantBoard::
	msgbox Ferry_Text_CantBoard, MSGBOX_DEFAULT
Ferry_EventScript_SailorRefused::
	closemessage
	setvar VAR_RESULT, FALSE
	return

@ A launch port's boarding: no walk, a fade with the ship's horn.
Ferry_EventScript_LaunchBoardingFade::
	playse SE_RG_SS_ANNE_HORN
	fadescreenswapbuffers FADE_TO_BLACK
	waitse
	return

@ The whole launch sailor: Seven Island, One Island and Melemele call this from their sailor object.
Ferry_EventScript_LaunchSailor::
	lock
	faceplayer
	setvar VAR_0x8004, FERRY_BERTH_LAUNCH
	call Ferry_EventScript_SailorTalk
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_SailorEnd
	call Ferry_EventScript_LaunchBoardingFade
	special Ferry_WarpAboard
	waitstate
	release
	end

Ferry_EventScript_SailorEnd::
	release
	end

@ ---- Going ashore ----

@ The exit sailor of the Aqua and the Tidal. Aboard at sea he says so and nothing happens; in port (or with no
@ voyage, a fixed home port) the player goes ashore.
Ferry_EventScript_ExitSailor::
	specialvar VAR_RESULT, Ferry_IsAtSeaForScript
	goto_if_eq VAR_RESULT, TRUE, Ferry_EventScript_ExitAtSea
	specialvar VAR_RESULT, Ferry_CanDisembark
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_ExitAtSea
	special Ferry_BufferArrivalPort
	msgbox Ferry_Text_GoAshore, MSGBOX_DEFAULT
	closemessage
	specialvar VAR_RESULT, Ferry_Disembark
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_ExitRefused
	waitstate
	release
	end

Ferry_EventScript_ExitAtSea::
	msgbox Ferry_Text_AtSea, MSGBOX_DEFAULT
	release
	end

Ferry_EventScript_ExitRefused::
	msgbox Ferry_Text_CantGoAshore, MSGBOX_DEFAULT
	release
	end

@ The Anne's gangway (19,2): at sea the dynamic warp only returns the player here.
Ferry_EventScript_Nop::
	end

Ferry_EventScript_AtSeaTrigger::
	specialvar VAR_RESULT, Ferry_IsAtSeaForScript
	goto_if_eq VAR_RESULT, FALSE, Ferry_EventScript_Nop
	lockall
	msgbox Ferry_Text_AtSea, MSGBOX_DEFAULT
	releaseall
	end

@ ---- The PA ----

Ferry_EventScript_PA_CastOff::
	lockall
	playse SE_DING_DONG
	msgbox Ferry_Text_PA_CastOff, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Ferry_EventScript_PA_CastOffAlola::
	lockall
	playse SE_DING_DONG
	msgbox Ferry_Text_PA_CastOffAlola, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Ferry_EventScript_PA_Arrival::
	lockall
	playse SE_DING_DONG
	msgbox Ferry_Text_PA_Arrival, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Ferry_EventScript_PA_ArrivalAlola::
	lockall
	playse SE_DING_DONG
	msgbox Ferry_Text_PA_ArrivalAlola, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

@ The Anne's first departure: the captain's reward arms it, and the PA announces it.
Ferry_EventScript_AnneFirstDeparture::
	special Ferry_ArmAnneDepartureScript
	playse SE_DING_DONG
	msgbox Ferry_Text_PA_AnneFirstDeparture, MSGBOX_DEFAULT
	return

@ ---- Texts ----

Ferry_Text_WelcomeAboard:
	.string "Welcome aboard the {STR_VAR_1},\n"
	.string "next stop {STR_VAR_2}.$"

Ferry_Text_WelcomeAboardLaunch:
	.string "The launch is ready for the\n"
	.string "{STR_VAR_1}, anchored offshore.\p"
	.string "Next stop {STR_VAR_2}. Welcome aboard!$"

Ferry_Text_NextShip:
	.string "No ship is in port right now.\p"
	.string "The next to call here is the\n"
	.string "{STR_VAR_1}, at {STR_VAR_2}.$"

Ferry_Text_NextShipLaunch:
	.string "The ship's anchored offshore\n"
	.string "right now.\p"
	.string "The {STR_VAR_1} is back at {STR_VAR_2}.$"

Ferry_Text_NoService:
	.string "No ship calls here at the moment.$"

Ferry_Text_NeedTicket:
	.string "I'm sorry, you need the\n"
	.string "{STR_VAR_1} to board.$"

Ferry_Text_CantBoard:
	.string "Sorry. You can't board now.$"

Ferry_Text_GoAshore:
	.string "We're in {STR_VAR_1}.\n"
	.string "Mind your step on the gangway.$"

Ferry_Text_AtSea:
	.string "We're at sea.\p"
	.string "We'll make an announcement when\n"
	.string "we reach port.$"

Ferry_Text_CantGoAshore:
	.string "Sorry. You can't go ashore now.$"

Ferry_Text_PA_CastOff:
	.string "All ashore that's going ashore!\p"
	.string "Next stop, {STR_VAR_1}.$"

Ferry_Text_PA_CastOffAlola:
	.string "All ashore that's going ashore!\p"
	.string "Next stop, {STR_VAR_1}, in the\n"
	.string "Alolan islands!$"

Ferry_Text_PA_Arrival:
	.string "Now arriving at {STR_VAR_1}.$"

Ferry_Text_PA_ArrivalAlola:
	.string "Now arriving at {STR_VAR_1}.\p"
	.string "Welcome to the Alolan islands!$"

Ferry_Text_PA_AnneFirstDeparture:
	.string "We sail for ONE ISLAND in half\n"
	.string "an hour.$"
