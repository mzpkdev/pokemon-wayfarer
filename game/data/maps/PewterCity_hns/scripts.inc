#include "constants/wayfarer_johto_story.h"

PewterCity_hns_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PewterCity_EventScript_CityEnter
	.byte 0

PewterCity_EventScript_CityEnter::
	setflag FLAG_VISITED_PEWTER_CITY
	end

PewterCity_EventScript_CooltrainerF::
	msgbox PewterCity_Text_CooltrainerF, MSGBOX_NPC
	end

PewterCity_EventScript_BugCatcher::
	msgbox PewterCity_Text_BugCatcher, MSGBOX_NPC
	end

@ The old man gives the other branch's Wing once: the main legendary's encounter must be
@ resolved (progress 3) and TR at the legendary threshold. Before a branch is chosen he only
@ chats. The gift is recorded in its own flag, so a tossed Wing is not given again.
PewterCity_EventScript_Gramps::
	lock
	faceplayer
	goto_if_eq VAR_LUGIA_OR_HOOH, 0, PewterCity_EventScript_GrampsDone
	goto_if_set FLAG_WAYFARER_JOHTO_PEWTER_WING_GIVEN, PewterCity_EventScript_GrampsDone
	specialvar VAR_RESULT, WayfarerJohtoPewterWingMissing
	goto_if_eq VAR_RESULT, PEWTER_MISSING_ENCOUNTER, PewterCity_EventScript_GrampsNeedsEncounter
	goto_if_eq VAR_RESULT, PEWTER_MISSING_TR, PewterCity_EventScript_GrampsNeedsTr
	goto_if_eq VAR_RESULT, PEWTER_MISSING_BOTH, PewterCity_EventScript_GrampsNeedsBoth
	goto_if_eq VAR_LUGIA_OR_HOOH, 1, PewterCity_EventScript_GrampsRainbowWing
	goto PewterCity_EventScript_GrampsSilverWing

@ Receipt is recorded only after the add; a full Bag leaves the Wing with him.
PewterCity_EventScript_GrampsRainbowWing::
	msgbox PewterCity_Text_Gramps, MSGBOX_DEFAULT
	giveitem ITEM_RAINBOW_WING
	goto_if_eq VAR_RESULT, FALSE, PewterCity_EventScript_GrampsBagFull
	setflag FLAG_WAYFARER_JOHTO_PEWTER_WING_GIVEN
	setflag FLAG_HIDE_TINTOWER_GUARD
	@ Ho-Oh's level 70 encounter is available unless it has already been resolved.
	goto_if_eq VAR_COMPLETED_HO_OH, 3, PewterCity_EventScript_GrampsGave
	setvar VAR_COMPLETED_HO_OH, 4
	goto PewterCity_EventScript_GrampsGave

PewterCity_EventScript_GrampsSilverWing::
	msgbox PewterCity_Text_Gramps, MSGBOX_DEFAULT
	giveitem ITEM_SILVER_WING
	goto_if_eq VAR_RESULT, FALSE, PewterCity_EventScript_GrampsBagFull
	setflag FLAG_WAYFARER_JOHTO_PEWTER_WING_GIVEN
	setflag FLAG_HIDE_WHIRL_ISLANDS_TENTACRUEL
	@ Lugia's level 70 encounter is available unless it has already been resolved.
	goto_if_eq VAR_COMPLETED_LUGIA, 3, PewterCity_EventScript_GrampsGave
	setvar VAR_COMPLETED_LUGIA, 4
PewterCity_EventScript_GrampsGave::
	closemessage
	release
	end

PewterCity_EventScript_GrampsBagFull::
	call Common_EventScript_BagIsFull
	closemessage
	release
	end

PewterCity_EventScript_GrampsNeedsEncounter::
	msgbox PewterCity_Text_GrampsNeedsEncounter, MSGBOX_DEFAULT
	closemessage
	release
	end

PewterCity_EventScript_GrampsNeedsTr::
	msgbox PewterCity_Text_GrampsNeedsTr, MSGBOX_DEFAULT
	closemessage
	release
	end

PewterCity_EventScript_GrampsNeedsBoth::
	msgbox PewterCity_Text_GrampsNeedsBoth, MSGBOX_DEFAULT
	closemessage
	release
	end

PewterCity_EventScript_GrampsDone::
	msgbox PewterCity_Text_Gramps_GotSilverWing, MSGBOX_DEFAULT
	closemessage
	release
	end

PewterCity_EventScript_Bugcatcher::
	msgbox PewterCity_Text_BugCatcher, MSGBOX_NPC
	end

PewterCity_EventScript_Cooltrainer::
	msgbox PewterCity_Text_CooltrainerF, MSGBOX_NPC
	end

PewterCity_Text_CooltrainerF:
	.string "Have you visited PEWTER GYM?\p"
	.string "The LEADER uses ROCK-type POKéMON.$"

PewterCity_Text_BugCatcher:
	.string "At night, CLEFAIRY come out to play at\n"
	.string "MT.MOON.\p"
	.string "But not every night.$"

PewterCity_Text_Gramps:
	.string "Ah, you came all the way out here from\n"
	.string "JOHTO?\p"
	.string "That brings back memories. When I was\n"
	.string "young, I went to JOHTO to train.\p"
	.string "You remind me so much of what I was like\n"
	.string "as a young man.\p"
	.string "Here. I want you to have this item I\n"
	.string "found in JOHTO.$"

PewterCity_Text_GrampsNeedsEncounter:
	.string "I found something in JOHTO, and I\n"
	.string "mean to hand it to you.\p"
	.string "But first see how your meeting with\n"
	.string "the legendary POKéMON ends.$"

PewterCity_Text_GrampsNeedsTr:
	.string "I found something in JOHTO, and I\n"
	.string "mean to hand it to you.\p"
	.string "But you need more experience as a\n"
	.string "Trainer first.$"

PewterCity_Text_GrampsNeedsBoth:
	.string "I found something in JOHTO, and I\n"
	.string "mean to hand it to you.\p"
	.string "But first see how your meeting with\n"
	.string "the legendary POKéMON ends, and gain\l"
	.string "more experience as a Trainer.$"

PewterCity_Text_Gramps_GotSilverWing:
	.string "Going to new, unknown places and seeing\n"
	.string "new people…\p"
	.string "Those are the joys of travel.$"

PewterCity_EventScript_CitySign::
	msgbox PewterCity_Text_Sign, MSGBOX_NPC
	end

PewterCity_EventScript_GymSign::
	msgbox PewterCity_Text_GymSign, MSGBOX_NPC
	end

PewterCity_EventScript_MuseumSign::
	msgbox PewterCity_Text_MuseumSign, MSGBOX_NPC
	end

PewterCity_EventScript_MtMoonSign::
	msgbox PewterCity_Text_MtMoonGiftShopSign, MSGBOX_NPC
	end

PewterCity_EventScript_WelcomeSign::
	msgbox PewterCity_Text_WelcomeSign, MSGBOX_NPC
	end

PewterCity_Text_Sign:
	.string "PEWTER CITY\n"
	.string "A Stone Gray City$"

PewterCity_Text_GymSign:
	.string "PEWTER CITY POKéMON GYM\n"
	.string "LEADER: BROCK\p"
	.string "The Rock Solid POKéMON Trainer$"

PewterCity_Text_MuseumSign:
	.string "There's a notice here…\p"
	.string "PEWTER MUSEUM OF SCIENCE is closed for\n"
	.string "renovations…$"

PewterCity_Text_MtMoonGiftShopSign:
	.string "There's a notice here…\p"
	.string "MT.MOON GIFT SHOP NOW OPEN!$"

PewterCity_Text_WelcomeSign:
	.string "WELCOME TO PEWTER CITY!$"

