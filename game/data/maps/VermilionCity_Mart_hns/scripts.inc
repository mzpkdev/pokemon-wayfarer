VermilionCity_Mart_hns_MapScripts::
	.byte 0

VermilionCity_Mart_EventScript_BirdMan::
	goto_if_set FLAG_VIRIDIAN_GIOVANNI_DEPARTED_HNS, VermilionCity_Mart_EventScript_BirdManRocketGone
	msgbox VermilionMart_Text_SuperNerdRocketAbout, MSGBOX_NPC
	end

VermilionCity_Mart_EventScript_BirdManRocketGone::
	msgbox VermilionMart_Text_SuperNerd, MSGBOX_NPC
	end

VermilionCity_Mart_EventScript_Woman::
	msgbox VermilionMart_Text_Beauty, MSGBOX_NPC
	end

VermilionMart_Text_SuperNerdRocketAbout:
	.string "TEAM ROCKET is still up to no good\n"
	.string "in KANTO.\p"
	.string "I just wish somebody would stop\n"
	.string "them.$"

VermilionMart_Text_SuperNerd:
	.string "TEAM ROCKET is no longer in\n"
	.string "KANTO.\p"
	.string "That alone makes me happy.$"

VermilionMart_Text_Beauty:
	.string "I'm thinking about going shopping\n"
	.string "in SAFFRON.$"
