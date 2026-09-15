@ Wayfarer-owned Sevii local adventures (delivery milestones 1-3).
@
@ These scripts intentionally use only the compact Wayfarer Sevii state bank.
@ Trainer parties and defeat routing are supplied by the Trainer generator.

@ ---------------------------------------------------------------------------
@ Lostelle, the Three Island bikers, and Bill's Meteorite
@ ---------------------------------------------------------------------------

WayfarerSeviiStory_EventScript_BillMeteorite::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_METEORITE_RECEIVED, WayfarerSeviiStory_EventScript_BillMeteoriteAccepted
	msgbox WayfarerSeviiStory_Text_BillMeteoriteOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSeviiStory_EventScript_BillMeteoriteDeclined
	setvar VAR_0x8004, ITEM_METEORITE
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_METEORITE_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_BillMeteoriteNoRoom
	msgbox WayfarerSeviiStory_Text_BillMeteoriteReceived
	release
	end

WayfarerSeviiStory_EventScript_BillMeteoriteAccepted::
	goto_if_set FLAG_WAYFARER_SEVII_METEORITE_DELIVERED, WayfarerSeviiStory_EventScript_BillMeteoriteDelivered
	msgbox WayfarerSeviiStory_Text_BillMeteoriteReminder
	release
	end

WayfarerSeviiStory_EventScript_BillMeteoriteDelivered::
	msgbox WayfarerSeviiStory_Text_BillMeteoriteThanks
	release
	end

WayfarerSeviiStory_EventScript_BillMeteoriteDeclined::
	msgbox WayfarerSeviiStory_Text_BillMeteoriteDeclined
	release
	end

WayfarerSeviiStory_EventScript_BillMeteoriteNoRoom::
	msgbox WayfarerSeviiStory_Text_BillMeteoriteNoRoom
	release
	end

WayfarerSeviiStory_EventScript_LostellesFather::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED, WayfarerSeviiStory_EventScript_LostellesFatherAfterRescue
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSeviiStory_EventScript_LostellesFatherReminder
	setflag FLAG_WAYFARER_SEVII_LOSTELLE_STARTED
	checkitem ITEM_METEORITE
	goto_if_eq VAR_RESULT, TRUE, WayfarerSeviiStory_EventScript_LostellesFatherStartWithMeteorite
	msgbox WayfarerSeviiStory_Text_LostelleStart
	release
	end

WayfarerSeviiStory_EventScript_LostellesFatherReminder::
	checkitem ITEM_METEORITE
	goto_if_eq VAR_RESULT, TRUE, WayfarerSeviiStory_EventScript_LostellesFatherReminderWithMeteorite
	msgbox WayfarerSeviiStory_Text_LostelleReminder
	release
	end

WayfarerSeviiStory_EventScript_LostellesFatherStartWithMeteorite::
	msgbox WayfarerSeviiStory_Text_LostelleStartWithMeteorite
	release
	end

WayfarerSeviiStory_EventScript_LostellesFatherReminderWithMeteorite::
	msgbox WayfarerSeviiStory_Text_LostelleReminderWithMeteorite
	release
	end

WayfarerSeviiStory_EventScript_LostellesFatherAfterRescue::
	goto_if_set FLAG_WAYFARER_SEVII_METEORITE_DELIVERED, WayfarerSeviiStory_EventScript_TryMoonStone
	checkitem ITEM_METEORITE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_LostellesFatherRescueThanks
	msgbox WayfarerSeviiStory_Text_MeteoriteHandoff, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSeviiStory_EventScript_LostellesFatherRescueThanks
	setvar VAR_0x8004, ITEM_METEORITE
	setvar VAR_0x8005, ITEM_MOON_STONE
	setvar VAR_0x8006, FLAG_WAYFARER_SEVII_METEORITE_DELIVERED
	setvar VAR_0x8007, FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryExchangeItemForRewardThenSetFlags
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_MoonStoneNoRoom
	msgbox WayfarerSeviiStory_Text_MeteoriteDelivered
	goto WayfarerSeviiStory_EventScript_LostellesFatherComplete

WayfarerSeviiStory_EventScript_LostellesFatherNoMeteorite::
	msgbox WayfarerSeviiStory_Text_NoMeteorite
	release
	end

WayfarerSeviiStory_EventScript_LostellesFatherRescueThanks::
	msgbox WayfarerSeviiStory_Text_LostelleRescueThanks
	release
	end

WayfarerSeviiStory_EventScript_TryMoonStone::
	goto_if_set FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED, WayfarerSeviiStory_EventScript_LostellesFatherComplete
	setvar VAR_0x8004, ITEM_MOON_STONE
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_MoonStoneNoRoom
	msgbox WayfarerSeviiStory_Text_MoonStoneReceived
	release
	end

WayfarerSeviiStory_EventScript_MoonStoneNoRoom::
	msgbox WayfarerSeviiStory_Text_MoonStoneNoRoom
	release
	end

WayfarerSeviiStory_EventScript_LostellesFatherComplete::
	msgbox WayfarerSeviiStory_Text_LostelleFatherComplete
	release
	end

@ Each selected actor reaches exactly one allocated Trainer. Predecessor checks
@ preserve the source order, while defeat state makes blackout retries stable.
WayfarerSeviiStory_EventScript_BikerConfrontation::
	goto WayfarerSeviiStory_EventScript_BikerGoon1

WayfarerSeviiStory_EventScript_BikerGoon1::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSeviiStory_EventScript_BikersNotStarted
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSeviiStory_EventScript_BikersCleared
	goto_if_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON, WayfarerSeviiStory_EventScript_BikerOneDefeated
	msgbox WayfarerSeviiStory_Text_BikerOneIntro
	trainerbattle_no_intro TRAINER_WAYFARER_SEVII_BIKER_GOON, WayfarerSeviiStory_Text_BikerOneDefeat
	msgbox WayfarerSeviiStory_Text_BikerOneAfter
	releaseall
	end

WayfarerSeviiStory_EventScript_BikerOneDefeated::
	msgbox WayfarerSeviiStory_Text_BikerOneAfter
	releaseall
	end

WayfarerSeviiStory_EventScript_BikerGoon2::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSeviiStory_EventScript_BikersNotStarted
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSeviiStory_EventScript_BikersCleared
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON, WayfarerSeviiStory_EventScript_BikersEarlierOpponent
	goto_if_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON_2, WayfarerSeviiStory_EventScript_BikerTwoDefeated
	msgbox WayfarerSeviiStory_Text_BikerTwoIntro
	trainerbattle_no_intro TRAINER_WAYFARER_SEVII_BIKER_GOON_2, WayfarerSeviiStory_Text_BikerTwoDefeat
	msgbox WayfarerSeviiStory_Text_BikerTwoAfter
	releaseall
	end

WayfarerSeviiStory_EventScript_BikerTwoDefeated::
	msgbox WayfarerSeviiStory_Text_BikerTwoAfter
	releaseall
	end

WayfarerSeviiStory_EventScript_BikerGoon3::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSeviiStory_EventScript_BikersNotStarted
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSeviiStory_EventScript_BikersCleared
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON_2, WayfarerSeviiStory_EventScript_BikersEarlierOpponent
	goto_if_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON_3, WayfarerSeviiStory_EventScript_BikerThreeDefeated
	msgbox WayfarerSeviiStory_Text_BikerThreeIntro
	trainerbattle_no_intro TRAINER_WAYFARER_SEVII_BIKER_GOON_3, WayfarerSeviiStory_Text_BikerThreeDefeat
	msgbox WayfarerSeviiStory_Text_BikerThreeAfter
	releaseall
	end

WayfarerSeviiStory_EventScript_BikerThreeDefeated::
	msgbox WayfarerSeviiStory_Text_BikerThreeAfter
	releaseall
	end

WayfarerSeviiStory_EventScript_CueBallPaxton::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSeviiStory_EventScript_BikersNotStarted
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSeviiStory_EventScript_BikersCleared
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON_3, WayfarerSeviiStory_EventScript_BikersEarlierOpponent
	goto_if_defeated TRAINER_WAYFARER_SEVII_CUE_BALL_PAXTON, WayfarerSeviiStory_EventScript_CommitBikersCleared
	msgbox WayfarerSeviiStory_Text_PaxtonIntro
	trainerbattle_no_intro TRAINER_WAYFARER_SEVII_CUE_BALL_PAXTON, WayfarerSeviiStory_Text_PaxtonDefeat

WayfarerSeviiStory_EventScript_CommitBikersCleared::
	setflag FLAG_WAYFARER_SEVII_BIKERS_CLEARED
	msgbox WayfarerSeviiStory_Text_BikersLeave
	releaseall
	end

WayfarerSeviiStory_EventScript_BikersNotStarted::
	msgbox WayfarerSeviiStory_Text_BikersLocalTrouble
	releaseall
	end

WayfarerSeviiStory_EventScript_BikersEarlierOpponent::
	msgbox WayfarerSeviiStory_Text_BikersEarlierOpponent
	releaseall
	end

WayfarerSeviiStory_EventScript_BikersCleared::
	msgbox WayfarerSeviiStory_Text_BikersGone
	releaseall
	end

@ Non-battling gang members use a separate closure so their manifest entries
@ do not claim or statically reach another actor's Trainer allocation.
WayfarerSeviiStory_EventScript_BikerOnlooker::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSeviiStory_EventScript_BikerOnlookerGone
	msgbox WayfarerSeviiStory_Text_BikerOnlooker
	release
	end

WayfarerSeviiStory_EventScript_BikerOnlookerGone::
	msgbox WayfarerSeviiStory_Text_BikersGone
	release
	end

WayfarerSeviiStory_EventScript_ThreeIslandDefender::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSeviiStory_EventScript_DefenderBeforeBikers
	goto_if_set FLAG_WAYFARER_SEVII_THREE_ISLAND_FULL_RESTORE_RECEIVED, WayfarerSeviiStory_EventScript_DefenderAfterReward
	setvar VAR_0x8004, ITEM_FULL_RESTORE
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_THREE_ISLAND_FULL_RESTORE_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_FullRestoreNoRoom
	msgbox WayfarerSeviiStory_Text_FullRestoreReceived
	release
	end

WayfarerSeviiStory_EventScript_FullRestoreNoRoom::
	msgbox WayfarerSeviiStory_Text_FullRestoreNoRoom
	release
	end

WayfarerSeviiStory_EventScript_DefenderBeforeBikers::
	msgbox WayfarerSeviiStory_Text_DefenderBeforeBikers
	release
	end

WayfarerSeviiStory_EventScript_DefenderAfterReward::
	msgbox WayfarerSeviiStory_Text_DefenderAfterReward
	release
	end

WayfarerSeviiStory_EventScript_LostelleForest::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSeviiStory_EventScript_LostelleBeforeBikers
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_FOUND, WayfarerSeviiStory_EventScript_LostelleAfterHypno
	msgbox WayfarerSeviiStory_Text_LostelleScared
	waitse
	playmoncry SPECIES_HYPNO, CRY_MODE_ENCOUNTER
	waitmoncry
	setwildbattle SPECIES_HYPNO, 30
	dowildbattle
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, WayfarerSeviiStory_EventScript_HypnoResolved
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, WayfarerSeviiStory_EventScript_HypnoResolved
	release
	end

WayfarerSeviiStory_EventScript_HypnoResolved::
	setflag FLAG_WAYFARER_SEVII_LOSTELLE_FOUND

WayfarerSeviiStory_EventScript_LostelleAfterHypno::
	goto_if_set FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED, WayfarerSeviiStory_EventScript_CommitLostelleRescue
	setvar VAR_0x8004, ITEM_IAPAPA_BERRY
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_IapapaNoRoom
	msgbox WayfarerSeviiStory_Text_IapapaReceived
	goto WayfarerSeviiStory_EventScript_CommitLostelleRescue

WayfarerSeviiStory_EventScript_IapapaNoRoom::
	msgbox WayfarerSeviiStory_Text_IapapaNoRoom
	goto WayfarerSeviiStory_EventScript_CommitLostelleRescue

WayfarerSeviiStory_EventScript_CommitLostelleRescue::
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED, WayfarerSeviiStory_EventScript_LostelleReadyHome
	setflag FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED
	msgbox WayfarerSeviiStory_Text_LostelleReadyHome
	release
	end

WayfarerSeviiStory_EventScript_LostelleReadyHome::
	msgbox WayfarerSeviiStory_Text_LostelleReadyHome
	release
	end

WayfarerSeviiStory_EventScript_LostelleBeforeBikers::
	msgbox WayfarerSeviiStory_Text_LostelleBeforeBikers
	release
	end

@ Lostelle's returned actor owns the retry if her Berry Pouch was full in the
@ forest. The Hypno and reunion cannot replay.
WayfarerSeviiStory_EventScript_LostelleReturned::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED, WayfarerSeviiStory_EventScript_LostelleNotHome
	goto_if_set FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED, WayfarerSeviiStory_EventScript_LostelleHomeThanks
	setvar VAR_0x8004, ITEM_IAPAPA_BERRY
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_ReturnedIapapaNoRoom
	msgbox WayfarerSeviiStory_Text_IapapaReceived
	release
	end

WayfarerSeviiStory_EventScript_ReturnedIapapaNoRoom::
	msgbox WayfarerSeviiStory_Text_IapapaNoRoom
	release
	end

WayfarerSeviiStory_EventScript_LostelleHomeThanks::
	msgbox WayfarerSeviiStory_Text_LostelleHomeThanks
	release
	end

WayfarerSeviiStory_EventScript_LostelleNotHome::
	msgbox WayfarerSeviiStory_Text_LostelleNotHome
	release
	end

@ ---------------------------------------------------------------------------
@ Lorelei and Icefall Cave
@ ---------------------------------------------------------------------------

WayfarerSeviiStory_EventScript_IcefallLorelei::
	lockall
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_COMPLETE, WayfarerSeviiStory_EventScript_LoreleiCompleteDialogue
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_ROCKETS_DEFEATED, WayfarerSeviiStory_EventScript_CommitLoreleiComplete
	setflag FLAG_WAYFARER_SEVII_LORELEI_STARTED
	msgbox WayfarerSeviiStory_Text_LoreleiNeedsHelp
	releaseall
	end

WayfarerSeviiStory_EventScript_IcefallRocket::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LORELEI_STARTED, WayfarerSeviiStory_EventScript_IcefallRocketBeforeLorelei
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_ROCKETS_DEFEATED, WayfarerSeviiStory_EventScript_IcefallRocketDefeated
	msgbox WayfarerSeviiStory_Text_IcefallRocketIntro
	trainerbattle_no_intro TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_45, WayfarerSeviiStory_Text_IcefallRocketDefeat
	setflag FLAG_WAYFARER_SEVII_LORELEI_ROCKETS_DEFEATED
	goto WayfarerSeviiStory_EventScript_CommitLoreleiComplete

WayfarerSeviiStory_EventScript_CommitLoreleiComplete::
	setflag FLAG_WAYFARER_SEVII_LORELEI_COMPLETE
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSeviiStory_EventScript_LoreleiWarehouseDone
	msgbox WayfarerSeviiStory_Text_LoreleiWarehouseLead
	releaseall
	end

WayfarerSeviiStory_EventScript_LoreleiWarehouseDone::
	msgbox WayfarerSeviiStory_Text_LoreleiWarehouseDone
	releaseall
	end

WayfarerSeviiStory_EventScript_LoreleiCompleteDialogue::
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSeviiStory_EventScript_LoreleiWarehouseDone
	msgbox WayfarerSeviiStory_Text_LoreleiWarehouseLead
	releaseall
	end

WayfarerSeviiStory_EventScript_IcefallRocketBeforeLorelei::
	msgbox WayfarerSeviiStory_Text_IcefallRocketBusy
	releaseall
	end

WayfarerSeviiStory_EventScript_IcefallRocketDefeated::
	msgbox WayfarerSeviiStory_Text_IcefallRocketGone
	releaseall
	end

WayfarerSeviiStory_EventScript_IcefallRocketLookout::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_COMPLETE, WayfarerSeviiStory_EventScript_IcefallRocketLookoutGone
	msgbox WayfarerSeviiStory_Text_IcefallRocketLookout
	release
	end

WayfarerSeviiStory_EventScript_IcefallRocketLookoutGone::
	msgbox WayfarerSeviiStory_Text_IcefallRocketGone
	release
	end

@ ---------------------------------------------------------------------------
@ Selphy and her repeatable showing request
@ ---------------------------------------------------------------------------

WayfarerSeviiStory_EventScript_SelphyLostCave::
	lockall
	goto_if_set FLAG_WAYFARER_SEVII_SELPHY_RETURNED, WayfarerSeviiStory_EventScript_SelphyAlreadyReturned
	goto_if_set FLAG_WAYFARER_SEVII_SELPHY_DEFEATED, WayfarerSeviiStory_EventScript_ReturnSelphy
	setflag FLAG_WAYFARER_SEVII_SELPHY_FOUND
	msgbox WayfarerSeviiStory_Text_SelphyIntro
	trainerbattle_no_intro TRAINER_WAYFARER_SEVII_LADY_SELPHY, WayfarerSeviiStory_Text_SelphyDefeat
	setflag FLAG_WAYFARER_SEVII_SELPHY_DEFEATED

WayfarerSeviiStory_EventScript_ReturnSelphy::
	setflag FLAG_WAYFARER_SEVII_SELPHY_RETURNED
	msgbox WayfarerSeviiStory_Text_SelphyReturn
	releaseall
	end

WayfarerSeviiStory_EventScript_SelphyAlreadyReturned::
	msgbox WayfarerSeviiStory_Text_SelphyAlreadyHome
	releaseall
	end

WayfarerSeviiStory_EventScript_SelphyRequest::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_SELPHY_RETURNED, WayfarerSeviiStory_EventScript_SelphyNotReturned
	specialvar VAR_RESULT, WayfarerSevii_HasActiveSelphyRequest
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_SelphySampleRequest
	goto WayfarerSeviiStory_EventScript_SelphyCheckRequest

WayfarerSeviiStory_EventScript_SelphySampleRequest::
	specialvar VAR_RESULT, WayfarerSevii_SampleSelphyRequest
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_SelphyRequestUnavailable
	bufferspeciesname STR_VAR_1, VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES
	msgbox WayfarerSeviiStory_Text_SelphyNewRequest
	release
	end

WayfarerSeviiStory_EventScript_SelphyCheckRequest::
	copyvar VAR_0x8004, VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES
	bufferspeciesname STR_VAR_1, VAR_0x8004
	specialvar VAR_RESULT, WayfarerSevii_PartyHasSpecies
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_SelphyWrongSpecies
	bufferitemname STR_VAR_2, VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD
	msgbox WayfarerSeviiStory_Text_SelphyRewardIntro
	specialvar VAR_RESULT, WayfarerSevii_TryClaimSelphyPendingReward
	goto_if_eq VAR_RESULT, FALSE, WayfarerSeviiStory_EventScript_SelphyRewardNoRoom
	msgbox WayfarerSeviiStory_Text_SelphyRewardDelivered
	release
	end

WayfarerSeviiStory_EventScript_SelphyWrongSpecies::
	msgbox WayfarerSeviiStory_Text_SelphyWrongSpecies
	release
	end

WayfarerSeviiStory_EventScript_SelphyRewardNoRoom::
	msgbox WayfarerSeviiStory_Text_SelphyRewardNoRoom
	release
	end

WayfarerSeviiStory_EventScript_SelphyNotReturned::
	msgbox WayfarerSeviiStory_Text_SelphyNotReturned
	release
	end

WayfarerSeviiStory_EventScript_SelphyRequestUnavailable::
	msgbox WayfarerSeviiStory_Text_SelphyRequestUnavailable
	release
	end

WayfarerSeviiStory_EventScript_SelphyButler::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_SELPHY_RETURNED, WayfarerSeviiStory_EventScript_SelphyButlerWaiting
	msgbox WayfarerSeviiStory_Text_SelphyButlerHome
	release
	end

WayfarerSeviiStory_EventScript_SelphyButlerWaiting::
	msgbox WayfarerSeviiStory_Text_SelphyButlerWaiting
	release
	end

@ ---------------------------------------------------------------------------
@ Dialogue
@ ---------------------------------------------------------------------------

WayfarerSeviiStory_Text_BillMeteoriteOffer::
	.string "BILL: A friend on TWO ISLAND studies\n"
	.string "rare stones. Will you take this\l"
	.string "METEORITE to him for me?$"
WayfarerSeviiStory_Text_BillMeteoriteReceived::
	.string "BILL handed you the METEORITE.\p"
	.string "His friend runs TWO ISLAND's\n"
	.string "GAME CORNER.$"
WayfarerSeviiStory_Text_BillMeteoriteReminder::
	.string "BILL: Please take the METEORITE to\n"
	.string "LOSTELLE's father on TWO ISLAND.$"
WayfarerSeviiStory_Text_BillMeteoriteThanks::
	.string "BILL: The delivery went through?\n"
	.string "Thanks for helping my friend!$"
WayfarerSeviiStory_Text_BillMeteoriteDeclined::
	.string "BILL: No problem. Ask me again if\n"
	.string "you feel like making the delivery.$"
WayfarerSeviiStory_Text_BillMeteoriteNoRoom::
	.string "BILL: Your KEY ITEMS pocket is full.\n"
	.string "Make room and I'll hold it for you.$"
WayfarerSeviiStory_Text_LostelleStart::
	.string "LOSTELLE's father: LOSTELLE hasn't\n"
	.string "come home! Please search THREE\l"
	.string "ISLAND, BOND BRIDGE, and the forest.$"
WayfarerSeviiStory_Text_LostelleReminder::
	.string "Please find LOSTELLE. The bikers on\n"
	.string "THREE ISLAND may know something.$"
WayfarerSeviiStory_Text_LostelleStartWithMeteorite::
	.string "That METEORITE is for me, but my\n"
	.string "LOSTELLE is missing! Please search\l"
	.string "THREE ISLAND for her first.$"
WayfarerSeviiStory_Text_LostelleReminderWithMeteorite::
	.string "I'll receive BILL's METEORITE after\n"
	.string "LOSTELLE is safely home. Please find\l"
	.string "her on THREE ISLAND.$"
WayfarerSeviiStory_Text_MeteoriteHandoff::
	.string "That METEORITE is from BILL!\n"
	.string "Will you hand it over?$"
WayfarerSeviiStory_Text_MeteoriteDelivered::
	.string "You handed over the METEORITE.\p"
	.string "Please take this rare stone in\n"
	.string "return.$"
WayfarerSeviiStory_Text_NoMeteorite::
	.string "I thought I saw a METEORITE, but it\n"
	.string "isn't in your BAG.$"
WayfarerSeviiStory_Text_LostelleRescueThanks::
	.string "Thank you for bringing LOSTELLE\n"
	.string "home safely!$"
WayfarerSeviiStory_Text_MoonStoneReceived::
	.string "You received a MOON STONE!$"
WayfarerSeviiStory_Text_MoonStoneNoRoom::
	.string "Your BAG is full. I'll keep the\n"
	.string "MOON STONE safe until you return.$"
WayfarerSeviiStory_Text_LostelleFatherComplete::
	.string "The GAME CORNER will be lively again.\n"
	.string "Come visit with a friend!$"
WayfarerSeviiStory_Text_BikerOneIntro::
	.string "Quit staring, or we'll make you!$"
WayfarerSeviiStory_Text_BikerOneDefeat::
	.string "W-what is this kid?$"
WayfarerSeviiStory_Text_BikerOneAfter::
	.string "Hey! Somebody else deal with this!$"
WayfarerSeviiStory_Text_BikerTwoIntro::
	.string "Our gang owns this island now!$"
WayfarerSeviiStory_Text_BikerTwoDefeat::
	.string "You beat me down!$"
WayfarerSeviiStory_Text_BikerTwoAfter::
	.string "You're tougher than you look.$"
WayfarerSeviiStory_Text_BikerThreeIntro::
	.string "You won't get past all of us!$"
WayfarerSeviiStory_Text_BikerThreeDefeat::
	.string "Stop fooling around!$"
WayfarerSeviiStory_Text_BikerThreeAfter::
	.string "Boss PAXTON will flatten you!$"
WayfarerSeviiStory_Text_PaxtonIntro::
	.string "PAXTON: You want us off this island?\n"
	.string "Make me.$"
WayfarerSeviiStory_Text_PaxtonDefeat::
	.string "All right, enough!$"
WayfarerSeviiStory_Text_BikersLeave::
	.string "PAXTON: We're leaving. Don't think\n"
	.string "we'll forget this!$"
WayfarerSeviiStory_Text_BikersLocalTrouble::
	.string "The bikers are making trouble, but\n"
	.string "you don't know who needs help yet.$"
WayfarerSeviiStory_Text_BikersEarlierOpponent::
	.string "Another biker steps between you and\n"
	.string "this opponent.$"
WayfarerSeviiStory_Text_BikersGone::
	.string "The biker gang has left the island.$"
WayfarerSeviiStory_Text_BikerOnlooker::
	.string "Our boss will put you in your place.$"
WayfarerSeviiStory_Text_DefenderBeforeBikers::
	.string "Those bikers have everyone on edge.$"
WayfarerSeviiStory_Text_FullRestoreReceived::
	.string "Thanks for driving them away!\n"
	.string "You received a FULL RESTORE.$"
WayfarerSeviiStory_Text_FullRestoreNoRoom::
	.string "Your BAG is full. I'll hold this\n"
	.string "FULL RESTORE for you.$"
WayfarerSeviiStory_Text_DefenderAfterReward::
	.string "Your POKéMON are really impressive.$"
WayfarerSeviiStory_Text_LostelleScared::
	.string "LOSTELLE: A scary POKéMON keeps\n"
	.string "coming back! Please help me!$"
WayfarerSeviiStory_Text_IapapaReceived::
	.string "LOSTELLE gave you an IAPAPA BERRY.$"
WayfarerSeviiStory_Text_IapapaNoRoom::
	.string "Your BERRY pocket is full. LOSTELLE\n"
	.string "will keep the IAPAPA BERRY for you.$"
WayfarerSeviiStory_Text_LostelleReadyHome::
	.string "LOSTELLE: Thank you! Now I can go\n"
	.string "home to my daddy.$"
WayfarerSeviiStory_Text_LostelleBeforeBikers::
	.string "The bikers' commotion makes the\n"
	.string "trail ahead hard to investigate.$"
WayfarerSeviiStory_Text_LostelleHomeThanks::
	.string "LOSTELLE: Thank you for bringing me\n"
	.string "home!$"
WayfarerSeviiStory_Text_LostelleNotHome::
	.string "LOSTELLE hasn't returned home yet.$"
WayfarerSeviiStory_Text_LoreleiNeedsHelp::
	.string "LORELEI: These poachers are taking\n"
	.string "the cave's POKéMON! Help me stop\l"
	.string "them.$"
WayfarerSeviiStory_Text_IcefallRocketIntro::
	.string "We catch whatever we want!\n"
	.string "Stay out of TEAM ROCKET's business!$"
WayfarerSeviiStory_Text_IcefallRocketDefeat::
	.string "We didn't plan on this!$"
WayfarerSeviiStory_Text_LoreleiWarehouseLead::
	.string "LORELEI: Thank you. They mentioned a\n"
	.string "ROCKET WAREHOUSE on FIVE ISLAND.\l"
	.string "Their operation may still be active.$"
WayfarerSeviiStory_Text_LoreleiWarehouseDone::
	.string "LORELEI: Thank you. I heard you also\n"
	.string "ended their WAREHOUSE operation.$"
WayfarerSeviiStory_Text_IcefallRocketBusy::
	.string "We're busy. Keep moving.$"
WayfarerSeviiStory_Text_IcefallRocketGone::
	.string "The poachers have fled ICEFALL CAVE.$"
WayfarerSeviiStory_Text_IcefallRocketLookout::
	.string "LORELEI can't stop all of us.$"
WayfarerSeviiStory_Text_SelphyIntro::
	.string "SELPHY: W-what do you intend to do\n"
	.string "with me?$"
WayfarerSeviiStory_Text_SelphyDefeat::
	.string "Oh… I was afraid you meant me harm.$"
WayfarerSeviiStory_Text_SelphyReturn::
	.string "SELPHY: I have grown tired of walking.\n"
	.string "Please take me home.$"
WayfarerSeviiStory_Text_SelphyAlreadyHome::
	.string "SELPHY has already returned home.$"
WayfarerSeviiStory_Text_SelphyNewRequest::
	.string "SELPHY: I wish to see a {STR_VAR_1}.\n"
	.string "Please show one to me.$"
WayfarerSeviiStory_Text_SelphyRewardIntro::
	.string "SELPHY: A {STR_VAR_1}! Splendid!\n"
	.string "Please accept this {STR_VAR_2}.$"
WayfarerSeviiStory_Text_SelphyRewardDelivered::
	.string "SELPHY: A fitting token of my\n"
	.string "appreciation.$"
WayfarerSeviiStory_Text_SelphyWrongSpecies::
	.string "SELPHY: I am still waiting to see a\n"
	.string "{STR_VAR_1}.$"
WayfarerSeviiStory_Text_SelphyRewardNoRoom::
	.string "Your BAG is full. The {STR_VAR_2}\n"
	.string "will remain here for you.$"
WayfarerSeviiStory_Text_SelphyNotReturned::
	.string "LADY SELPHY is away from home.$"
WayfarerSeviiStory_Text_SelphyRequestUnavailable::
	.string "SELPHY: Let us speak of another wish\n"
	.string "a little later.$"
WayfarerSeviiStory_Text_SelphyButlerHome::
	.string "My lady's smile has regained the\n"
	.string "brilliance of the sun.$"
WayfarerSeviiStory_Text_SelphyButlerWaiting::
	.string "Please bring LADY SELPHY home safely.$"
