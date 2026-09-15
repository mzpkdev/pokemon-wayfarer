@ Wayfarer-owned Sevii local adventures (delivery milestones 1-3).
@
@ These scripts intentionally use only the compact Wayfarer Sevii state bank.
@ Trainer parties and defeat routing are supplied by the Trainer generator.

@ ---------------------------------------------------------------------------
@ Lostelle, the Three Island bikers, and Bill's Meteorite
@ ---------------------------------------------------------------------------

WayfarerSevii_Story_EventScript_BillMeteorite::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_METEORITE_RECEIVED, WayfarerSevii_Story_EventScript_BillMeteoriteAccepted
	msgbox WayfarerSevii_Story_Text_BillMeteoriteOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Story_EventScript_BillMeteoriteDeclined
	setvar VAR_0x8004, ITEM_METEORITE
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_METEORITE_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_BillMeteoriteNoRoom
	setflag FLAG_WAYFARER_SEVII_METEORITE_RECEIVED
	msgbox WayfarerSevii_Story_Text_BillMeteoriteReceived
	release
	end

WayfarerSevii_Story_EventScript_BillMeteoriteAccepted::
	goto_if_set FLAG_WAYFARER_SEVII_METEORITE_DELIVERED, WayfarerSevii_Story_EventScript_BillMeteoriteDelivered
	msgbox WayfarerSevii_Story_Text_BillMeteoriteReminder
	release
	end

WayfarerSevii_Story_EventScript_BillMeteoriteDelivered::
	msgbox WayfarerSevii_Story_Text_BillMeteoriteThanks
	release
	end

WayfarerSevii_Story_EventScript_BillMeteoriteDeclined::
	msgbox WayfarerSevii_Story_Text_BillMeteoriteDeclined
	release
	end

WayfarerSevii_Story_EventScript_BillMeteoriteNoRoom::
	msgbox WayfarerSevii_Story_Text_BillMeteoriteNoRoom
	release
	end

WayfarerSevii_Story_EventScript_LostellesFather::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED, WayfarerSevii_Story_EventScript_LostellesFatherAfterRescue
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSevii_Story_EventScript_LostellesFatherReminder
	setflag FLAG_WAYFARER_SEVII_LOSTELLE_STARTED
	checkitem ITEM_METEORITE
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_Story_EventScript_LostellesFatherStartWithMeteorite
	msgbox WayfarerSevii_Story_Text_LostelleStart
	release
	end

WayfarerSevii_Story_EventScript_LostellesFatherReminder::
	checkitem ITEM_METEORITE
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_Story_EventScript_LostellesFatherReminderWithMeteorite
	msgbox WayfarerSevii_Story_Text_LostelleReminder
	release
	end

WayfarerSevii_Story_EventScript_LostellesFatherStartWithMeteorite::
	msgbox WayfarerSevii_Story_Text_LostelleStartWithMeteorite
	release
	end

WayfarerSevii_Story_EventScript_LostellesFatherReminderWithMeteorite::
	msgbox WayfarerSevii_Story_Text_LostelleReminderWithMeteorite
	release
	end

WayfarerSevii_Story_EventScript_LostellesFatherAfterRescue::
	goto_if_set FLAG_WAYFARER_SEVII_METEORITE_DELIVERED, WayfarerSevii_Story_EventScript_TryMoonStone
	checkitem ITEM_METEORITE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_LostellesFatherRescueThanks
	msgbox WayfarerSevii_Story_Text_MeteoriteHandoff, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Story_EventScript_LostellesFatherRescueThanks
	setvar VAR_0x8004, ITEM_METEORITE
	setvar VAR_0x8005, ITEM_MOON_STONE
	setvar VAR_0x8006, FLAG_WAYFARER_SEVII_METEORITE_DELIVERED
	setvar VAR_0x8007, FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryExchangeItemForRewardThenSetFlags
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_MoonStoneNoRoom
	setflag FLAG_WAYFARER_SEVII_METEORITE_DELIVERED
	setflag FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED
	msgbox WayfarerSevii_Story_Text_MeteoriteDelivered
	goto WayfarerSevii_Story_EventScript_LostellesFatherComplete

WayfarerSevii_Story_EventScript_LostellesFatherNoMeteorite::
	msgbox WayfarerSevii_Story_Text_NoMeteorite
	release
	end

WayfarerSevii_Story_EventScript_LostellesFatherRescueThanks::
	msgbox WayfarerSevii_Story_Text_LostelleRescueThanks
	release
	end

WayfarerSevii_Story_EventScript_TryMoonStone::
	goto_if_set FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED, WayfarerSevii_Story_EventScript_LostellesFatherComplete
	setvar VAR_0x8004, ITEM_MOON_STONE
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_MoonStoneNoRoom
	setflag FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED
	msgbox WayfarerSevii_Story_Text_MoonStoneReceived
	release
	end

WayfarerSevii_Story_EventScript_MoonStoneNoRoom::
	msgbox WayfarerSevii_Story_Text_MoonStoneNoRoom
	release
	end

WayfarerSevii_Story_EventScript_LostellesFatherComplete::
	msgbox WayfarerSevii_Story_Text_LostelleFatherComplete
	release
	end

@ Each selected actor reaches exactly one allocated Trainer. Predecessor checks
@ preserve the source order, while defeat state makes blackout retries stable.
WayfarerSevii_Story_EventScript_BikerConfrontation::
	goto WayfarerSevii_Story_EventScript_BikerGoon1

WayfarerSevii_Story_EventScript_BikerGoon1::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSevii_Story_EventScript_BikersNotStarted
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSevii_Story_EventScript_BikersCleared
	goto_if_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON, WayfarerSevii_Story_EventScript_BikerOneDefeated
	trainerbattle_single TRAINER_WAYFARER_SEVII_BIKER_GOON, WayfarerSevii_Story_Text_BikerOneIntro, WayfarerSevii_Story_Text_BikerOneDefeat
	msgbox WayfarerSevii_Story_Text_BikerOneAfter
	releaseall
	end

WayfarerSevii_Story_EventScript_BikerOneDefeated::
	msgbox WayfarerSevii_Story_Text_BikerOneAfter
	releaseall
	end

WayfarerSevii_Story_EventScript_BikerGoon2::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSevii_Story_EventScript_BikersNotStarted
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSevii_Story_EventScript_BikersCleared
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON, WayfarerSevii_Story_EventScript_BikersEarlierOpponent
	goto_if_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON_2, WayfarerSevii_Story_EventScript_BikerTwoDefeated
	trainerbattle_single TRAINER_WAYFARER_SEVII_BIKER_GOON_2, WayfarerSevii_Story_Text_BikerTwoIntro, WayfarerSevii_Story_Text_BikerTwoDefeat
	msgbox WayfarerSevii_Story_Text_BikerTwoAfter
	releaseall
	end

WayfarerSevii_Story_EventScript_BikerTwoDefeated::
	msgbox WayfarerSevii_Story_Text_BikerTwoAfter
	releaseall
	end

WayfarerSevii_Story_EventScript_BikerGoon3::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSevii_Story_EventScript_BikersNotStarted
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSevii_Story_EventScript_BikersCleared
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON_2, WayfarerSevii_Story_EventScript_BikersEarlierOpponent
	goto_if_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON_3, WayfarerSevii_Story_EventScript_BikerThreeDefeated
	trainerbattle_single TRAINER_WAYFARER_SEVII_BIKER_GOON_3, WayfarerSevii_Story_Text_BikerThreeIntro, WayfarerSevii_Story_Text_BikerThreeDefeat
	msgbox WayfarerSevii_Story_Text_BikerThreeAfter
	releaseall
	end

WayfarerSevii_Story_EventScript_BikerThreeDefeated::
	msgbox WayfarerSevii_Story_Text_BikerThreeAfter
	releaseall
	end

WayfarerSevii_Story_EventScript_CueBallPaxton::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_STARTED, WayfarerSevii_Story_EventScript_BikersNotStarted
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSevii_Story_EventScript_BikersCleared
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_BIKER_GOON_3, WayfarerSevii_Story_EventScript_BikersEarlierOpponent
	goto_if_defeated TRAINER_WAYFARER_SEVII_CUE_BALL_PAXTON, WayfarerSevii_Story_EventScript_CommitBikersCleared
	trainerbattle_single TRAINER_WAYFARER_SEVII_CUE_BALL_PAXTON, WayfarerSevii_Story_Text_PaxtonIntro, WayfarerSevii_Story_Text_PaxtonDefeat

WayfarerSevii_Story_EventScript_CommitBikersCleared::
	setflag FLAG_WAYFARER_SEVII_BIKERS_CLEARED
	msgbox WayfarerSevii_Story_Text_BikersLeave
	releaseall
	end

WayfarerSevii_Story_EventScript_BikersNotStarted::
	msgbox WayfarerSevii_Story_Text_BikersLocalTrouble
	releaseall
	end

WayfarerSevii_Story_EventScript_BikersEarlierOpponent::
	msgbox WayfarerSevii_Story_Text_BikersEarlierOpponent
	releaseall
	end

WayfarerSevii_Story_EventScript_BikersCleared::
	msgbox WayfarerSevii_Story_Text_BikersGone
	releaseall
	end

@ Non-battling gang members use a separate closure so their manifest entries
@ do not claim or statically reach another actor's Trainer allocation.
WayfarerSevii_Story_EventScript_BikerOnlooker::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSevii_Story_EventScript_BikerOnlookerGone
	msgbox WayfarerSevii_Story_Text_BikerOnlooker
	release
	end

WayfarerSevii_Story_EventScript_BikerOnlookerGone::
	msgbox WayfarerSevii_Story_Text_BikersGone
	release
	end

WayfarerSevii_Story_EventScript_ThreeIslandDefender::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSevii_Story_EventScript_DefenderBeforeBikers
	goto_if_set FLAG_WAYFARER_SEVII_THREE_ISLAND_FULL_RESTORE_RECEIVED, WayfarerSevii_Story_EventScript_DefenderAfterReward
	setvar VAR_0x8004, ITEM_FULL_RESTORE
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_THREE_ISLAND_FULL_RESTORE_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_FullRestoreNoRoom
	setflag FLAG_WAYFARER_SEVII_THREE_ISLAND_FULL_RESTORE_RECEIVED
	msgbox WayfarerSevii_Story_Text_FullRestoreReceived
	release
	end

WayfarerSevii_Story_EventScript_FullRestoreNoRoom::
	msgbox WayfarerSevii_Story_Text_FullRestoreNoRoom
	release
	end

WayfarerSevii_Story_EventScript_DefenderBeforeBikers::
	msgbox WayfarerSevii_Story_Text_DefenderBeforeBikers
	release
	end

WayfarerSevii_Story_EventScript_DefenderAfterReward::
	msgbox WayfarerSevii_Story_Text_DefenderAfterReward
	release
	end

WayfarerSevii_Story_EventScript_LostelleForest::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSevii_Story_EventScript_LostelleBeforeBikers
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_FOUND, WayfarerSevii_Story_EventScript_LostelleAfterHypno
	msgbox WayfarerSevii_Story_Text_LostelleScared
	waitse
	playmoncry SPECIES_HYPNO, CRY_MODE_ENCOUNTER
	waitmoncry
	setwildbattle SPECIES_HYPNO, 30
	dowildbattle
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, WayfarerSevii_Story_EventScript_HypnoResolved
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, WayfarerSevii_Story_EventScript_HypnoResolved
	release
	end

WayfarerSevii_Story_EventScript_HypnoResolved::
	setflag FLAG_WAYFARER_SEVII_LOSTELLE_FOUND

WayfarerSevii_Story_EventScript_LostelleAfterHypno::
	goto_if_set FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED, WayfarerSevii_Story_EventScript_CommitLostelleRescue
	setvar VAR_0x8004, ITEM_IAPAPA_BERRY
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_IapapaNoRoom
	setflag FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED
	msgbox WayfarerSevii_Story_Text_IapapaReceived
	goto WayfarerSevii_Story_EventScript_CommitLostelleRescue

WayfarerSevii_Story_EventScript_IapapaNoRoom::
	msgbox WayfarerSevii_Story_Text_IapapaNoRoom
	goto WayfarerSevii_Story_EventScript_CommitLostelleRescue

WayfarerSevii_Story_EventScript_CommitLostelleRescue::
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED, WayfarerSevii_Story_EventScript_LostelleReadyHome
	setflag FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED
	msgbox WayfarerSevii_Story_Text_LostelleReadyHome
	release
	end

WayfarerSevii_Story_EventScript_LostelleReadyHome::
	msgbox WayfarerSevii_Story_Text_LostelleReadyHome
	release
	end

WayfarerSevii_Story_EventScript_LostelleBeforeBikers::
	msgbox WayfarerSevii_Story_Text_LostelleBeforeBikers
	release
	end

@ Lostelle's returned actor owns the retry if her Berry Pouch was full in the
@ forest. The Hypno and reunion cannot replay.
WayfarerSevii_Story_EventScript_LostelleReturned::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED, WayfarerSevii_Story_EventScript_LostelleNotHome
	goto_if_set FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED, WayfarerSevii_Story_EventScript_LostelleHomeThanks
	setvar VAR_0x8004, ITEM_IAPAPA_BERRY
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_ReturnedIapapaNoRoom
	setflag FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED
	msgbox WayfarerSevii_Story_Text_IapapaReceived
	release
	end

WayfarerSevii_Story_EventScript_ReturnedIapapaNoRoom::
	msgbox WayfarerSevii_Story_Text_IapapaNoRoom
	release
	end

WayfarerSevii_Story_EventScript_LostelleHomeThanks::
	msgbox WayfarerSevii_Story_Text_LostelleHomeThanks
	release
	end

WayfarerSevii_Story_EventScript_LostelleNotHome::
	msgbox WayfarerSevii_Story_Text_LostelleNotHome
	release
	end

@ ---------------------------------------------------------------------------
@ Lorelei and Icefall Cave
@ ---------------------------------------------------------------------------

WayfarerSevii_Story_EventScript_IcefallLorelei::
	lockall
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_COMPLETE, WayfarerSevii_Story_EventScript_LoreleiCompleteDialogue
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_ROCKETS_DEFEATED, WayfarerSevii_Story_EventScript_CommitLoreleiComplete
	setflag FLAG_WAYFARER_SEVII_LORELEI_STARTED
	msgbox WayfarerSevii_Story_Text_LoreleiNeedsHelp
	releaseall
	end

WayfarerSevii_Story_EventScript_IcefallRocket::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_LORELEI_STARTED, WayfarerSevii_Story_EventScript_IcefallRocketBeforeLorelei
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_ROCKETS_DEFEATED, WayfarerSevii_Story_EventScript_IcefallRocketDefeated
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_45, WayfarerSevii_Story_Text_IcefallRocketIntro, WayfarerSevii_Story_Text_IcefallRocketDefeat
	setflag FLAG_WAYFARER_SEVII_LORELEI_ROCKETS_DEFEATED
	goto WayfarerSevii_Story_EventScript_CommitLoreleiComplete

WayfarerSevii_Story_EventScript_CommitLoreleiComplete::
	setflag FLAG_WAYFARER_SEVII_LORELEI_COMPLETE
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_Story_EventScript_LoreleiWarehouseDone
	msgbox WayfarerSevii_Story_Text_LoreleiWarehouseLead
	releaseall
	end

WayfarerSevii_Story_EventScript_LoreleiWarehouseDone::
	msgbox WayfarerSevii_Story_Text_LoreleiWarehouseDone
	releaseall
	end

WayfarerSevii_Story_EventScript_LoreleiCompleteDialogue::
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_Story_EventScript_LoreleiWarehouseDone
	msgbox WayfarerSevii_Story_Text_LoreleiWarehouseLead
	releaseall
	end

WayfarerSevii_Story_EventScript_IcefallRocketBeforeLorelei::
	msgbox WayfarerSevii_Story_Text_IcefallRocketBusy
	releaseall
	end

WayfarerSevii_Story_EventScript_IcefallRocketDefeated::
	msgbox WayfarerSevii_Story_Text_IcefallRocketGone
	releaseall
	end

WayfarerSevii_Story_EventScript_IcefallRocketLookout::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_COMPLETE, WayfarerSevii_Story_EventScript_IcefallRocketLookoutGone
	msgbox WayfarerSevii_Story_Text_IcefallRocketLookout
	release
	end

WayfarerSevii_Story_EventScript_IcefallRocketLookoutGone::
	msgbox WayfarerSevii_Story_Text_IcefallRocketGone
	release
	end

@ ---------------------------------------------------------------------------
@ Selphy and her repeatable showing request
@ ---------------------------------------------------------------------------

WayfarerSevii_Story_EventScript_SelphyLostCave::
	lockall
	goto_if_set FLAG_WAYFARER_SEVII_SELPHY_RETURNED, WayfarerSevii_Story_EventScript_SelphyAlreadyReturned
	goto_if_set FLAG_WAYFARER_SEVII_SELPHY_DEFEATED, WayfarerSevii_Story_EventScript_ReturnSelphy
	setflag FLAG_WAYFARER_SEVII_SELPHY_FOUND
	trainerbattle_single TRAINER_WAYFARER_SEVII_LADY_SELPHY, WayfarerSevii_Story_Text_SelphyIntro, WayfarerSevii_Story_Text_SelphyDefeat
	setflag FLAG_WAYFARER_SEVII_SELPHY_DEFEATED

WayfarerSevii_Story_EventScript_ReturnSelphy::
	setflag FLAG_WAYFARER_SEVII_SELPHY_RETURNED
	msgbox WayfarerSevii_Story_Text_SelphyReturn
	releaseall
	end

WayfarerSevii_Story_EventScript_SelphyAlreadyReturned::
	msgbox WayfarerSevii_Story_Text_SelphyAlreadyHome
	releaseall
	end

WayfarerSevii_Story_EventScript_SelphyRequest::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_SELPHY_RETURNED, WayfarerSevii_Story_EventScript_SelphyNotReturned
	specialvar VAR_RESULT, WayfarerSevii_HasActiveSelphyRequest
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_SelphySampleRequest
	goto WayfarerSevii_Story_EventScript_SelphyCheckRequest

WayfarerSevii_Story_EventScript_SelphySampleRequest::
	specialvar VAR_RESULT, WayfarerSevii_SampleSelphyRequest
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_SelphyRequestUnavailable
	bufferspeciesname STR_VAR_1, VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES
	msgbox WayfarerSevii_Story_Text_SelphyNewRequest
	release
	end

WayfarerSevii_Story_EventScript_SelphyCheckRequest::
	copyvar VAR_0x8004, VAR_WAYFARER_SEVII_SELPHY_REQUESTED_SPECIES
	bufferspeciesname STR_VAR_1, VAR_0x8004
	specialvar VAR_RESULT, WayfarerSevii_PartyHasSpecies
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_SelphyWrongSpecies
	bufferitemname STR_VAR_2, VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD
	msgbox WayfarerSevii_Story_Text_SelphyRewardIntro
	specialvar VAR_RESULT, WayfarerSevii_TryClaimSelphyPendingReward
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EventScript_SelphyRewardNoRoom
	msgbox WayfarerSevii_Story_Text_SelphyRewardDelivered
	release
	end

WayfarerSevii_Story_EventScript_SelphyWrongSpecies::
	msgbox WayfarerSevii_Story_Text_SelphyWrongSpecies
	release
	end

WayfarerSevii_Story_EventScript_SelphyRewardNoRoom::
	msgbox WayfarerSevii_Story_Text_SelphyRewardNoRoom
	release
	end

WayfarerSevii_Story_EventScript_SelphyNotReturned::
	msgbox WayfarerSevii_Story_Text_SelphyNotReturned
	release
	end

WayfarerSevii_Story_EventScript_SelphyRequestUnavailable::
	msgbox WayfarerSevii_Story_Text_SelphyRequestUnavailable
	release
	end

WayfarerSevii_Story_EventScript_SelphyButler::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_SELPHY_RETURNED, WayfarerSevii_Story_EventScript_SelphyButlerWaiting
	msgbox WayfarerSevii_Story_Text_SelphyButlerHome
	release
	end

WayfarerSevii_Story_EventScript_SelphyButlerWaiting::
	msgbox WayfarerSevii_Story_Text_SelphyButlerWaiting
	release
	end

@ ---------------------------------------------------------------------------
@ Dialogue
@ ---------------------------------------------------------------------------

WayfarerSevii_Story_Text_BillMeteoriteOffer::
	.string "BILL: A friend on TWO ISLAND studies\n"
	.string "rare stones. Will you take this\l"
	.string "METEORITE to him for me?$"
WayfarerSevii_Story_Text_BillMeteoriteReceived::
	.string "BILL handed you the METEORITE.\p"
	.string "His friend runs TWO ISLAND's\n"
	.string "GAME CORNER.$"
WayfarerSevii_Story_Text_BillMeteoriteReminder::
	.string "BILL: Please take the METEORITE to\n"
	.string "LOSTELLE's father on TWO ISLAND.$"
WayfarerSevii_Story_Text_BillMeteoriteThanks::
	.string "BILL: The delivery went through?\n"
	.string "Thanks for helping my friend!$"
WayfarerSevii_Story_Text_BillMeteoriteDeclined::
	.string "BILL: No problem. Ask me again if\n"
	.string "you feel like making the delivery.$"
WayfarerSevii_Story_Text_BillMeteoriteNoRoom::
	.string "BILL: Your KEY ITEMS pocket is full.\n"
	.string "Make room and I'll hold it for you.$"
WayfarerSevii_Story_Text_LostelleStart::
	.string "LOSTELLE's father: LOSTELLE hasn't\n"
	.string "come home! Please search THREE\l"
	.string "ISLAND, BOND BRIDGE, and the forest.$"
WayfarerSevii_Story_Text_LostelleReminder::
	.string "Please find LOSTELLE. The bikers on\n"
	.string "THREE ISLAND may know something.$"
WayfarerSevii_Story_Text_LostelleStartWithMeteorite::
	.string "That METEORITE is for me, but my\n"
	.string "LOSTELLE is missing! Please search\l"
	.string "THREE ISLAND for her first.$"
WayfarerSevii_Story_Text_LostelleReminderWithMeteorite::
	.string "I'll receive BILL's METEORITE after\n"
	.string "LOSTELLE is safely home. Please find\l"
	.string "her on THREE ISLAND.$"
WayfarerSevii_Story_Text_MeteoriteHandoff::
	.string "That METEORITE is from BILL!\n"
	.string "Will you hand it over?$"
WayfarerSevii_Story_Text_MeteoriteDelivered::
	.string "You handed over the METEORITE.\p"
	.string "Please take this rare stone in\n"
	.string "return.$"
WayfarerSevii_Story_Text_NoMeteorite::
	.string "I thought I saw a METEORITE, but it\n"
	.string "isn't in your BAG.$"
WayfarerSevii_Story_Text_LostelleRescueThanks::
	.string "Thank you for bringing LOSTELLE\n"
	.string "home safely!$"
WayfarerSevii_Story_Text_MoonStoneReceived::
	.string "You received a MOON STONE!$"
WayfarerSevii_Story_Text_MoonStoneNoRoom::
	.string "Your BAG is full. I'll keep the\n"
	.string "MOON STONE safe until you return.$"
WayfarerSevii_Story_Text_LostelleFatherComplete::
	.string "The GAME CORNER will be lively again.\n"
	.string "Come visit with a friend!$"
WayfarerSevii_Story_Text_BikerOneIntro::
	.string "Quit staring, or we'll make you!$"
WayfarerSevii_Story_Text_BikerOneDefeat::
	.string "W-what is this kid?$"
WayfarerSevii_Story_Text_BikerOneAfter::
	.string "Hey! Somebody else deal with this!$"
WayfarerSevii_Story_Text_BikerTwoIntro::
	.string "Our gang owns this island now!$"
WayfarerSevii_Story_Text_BikerTwoDefeat::
	.string "You beat me down!$"
WayfarerSevii_Story_Text_BikerTwoAfter::
	.string "You're tougher than you look.$"
WayfarerSevii_Story_Text_BikerThreeIntro::
	.string "You won't get past all of us!$"
WayfarerSevii_Story_Text_BikerThreeDefeat::
	.string "Stop fooling around!$"
WayfarerSevii_Story_Text_BikerThreeAfter::
	.string "Boss PAXTON will flatten you!$"
WayfarerSevii_Story_Text_PaxtonIntro::
	.string "PAXTON: You want us off this island?\n"
	.string "Make me.$"
WayfarerSevii_Story_Text_PaxtonDefeat::
	.string "All right, enough!$"
WayfarerSevii_Story_Text_BikersLeave::
	.string "PAXTON: We're leaving. Don't think\n"
	.string "we'll forget this!$"
WayfarerSevii_Story_Text_BikersLocalTrouble::
	.string "The bikers are making trouble, but\n"
	.string "you don't know who needs help yet.$"
WayfarerSevii_Story_Text_BikersEarlierOpponent::
	.string "Another biker steps between you and\n"
	.string "this opponent.$"
WayfarerSevii_Story_Text_BikersGone::
	.string "The biker gang has left the island.$"
WayfarerSevii_Story_Text_BikerOnlooker::
	.string "Our boss will put you in your place.$"
WayfarerSevii_Story_Text_DefenderBeforeBikers::
	.string "Those bikers have everyone on edge.$"
WayfarerSevii_Story_Text_FullRestoreReceived::
	.string "Thanks for driving them away!\n"
	.string "You received a FULL RESTORE.$"
WayfarerSevii_Story_Text_FullRestoreNoRoom::
	.string "Your BAG is full. I'll hold this\n"
	.string "FULL RESTORE for you.$"
WayfarerSevii_Story_Text_DefenderAfterReward::
	.string "Your POKéMON are really impressive.$"
WayfarerSevii_Story_Text_LostelleScared::
	.string "LOSTELLE: A scary POKéMON keeps\n"
	.string "coming back! Please help me!$"
WayfarerSevii_Story_Text_IapapaReceived::
	.string "LOSTELLE gave you an IAPAPA BERRY.$"
WayfarerSevii_Story_Text_IapapaNoRoom::
	.string "Your BERRY pocket is full. LOSTELLE\n"
	.string "will keep the IAPAPA BERRY for you.$"
WayfarerSevii_Story_Text_LostelleReadyHome::
	.string "LOSTELLE: Thank you! Now I can go\n"
	.string "home to my daddy.$"
WayfarerSevii_Story_Text_LostelleBeforeBikers::
	.string "The bikers' commotion makes the\n"
	.string "trail ahead hard to investigate.$"
WayfarerSevii_Story_Text_LostelleHomeThanks::
	.string "LOSTELLE: Thank you for bringing me\n"
	.string "home!$"
WayfarerSevii_Story_Text_LostelleNotHome::
	.string "LOSTELLE hasn't returned home yet.$"
WayfarerSevii_Story_Text_LoreleiNeedsHelp::
	.string "LORELEI: These poachers are taking\n"
	.string "the cave's POKéMON! Help me stop\l"
	.string "them.$"
WayfarerSevii_Story_Text_IcefallRocketIntro::
	.string "We catch whatever we want!\n"
	.string "Stay out of TEAM ROCKET's business!$"
WayfarerSevii_Story_Text_IcefallRocketDefeat::
	.string "We didn't plan on this!$"
WayfarerSevii_Story_Text_LoreleiWarehouseLead::
	.string "LORELEI: Thank you. They mentioned a\n"
	.string "ROCKET WAREHOUSE on FIVE ISLAND.\l"
	.string "Their operation may still be active.$"
WayfarerSevii_Story_Text_LoreleiWarehouseDone::
	.string "LORELEI: Thank you. I heard you also\n"
	.string "ended their WAREHOUSE operation.$"
WayfarerSevii_Story_Text_IcefallRocketBusy::
	.string "We're busy. Keep moving.$"
WayfarerSevii_Story_Text_IcefallRocketGone::
	.string "The poachers have fled ICEFALL CAVE.$"
WayfarerSevii_Story_Text_IcefallRocketLookout::
	.string "LORELEI can't stop all of us.$"
WayfarerSevii_Story_Text_SelphyIntro::
	.string "SELPHY: W-what do you intend to do\n"
	.string "with me?$"
WayfarerSevii_Story_Text_SelphyDefeat::
	.string "Oh… I was afraid you meant me harm.$"
WayfarerSevii_Story_Text_SelphyReturn::
	.string "SELPHY: I have grown tired of walking.\n"
	.string "Please take me home.$"
WayfarerSevii_Story_Text_SelphyAlreadyHome::
	.string "SELPHY has already returned home.$"
WayfarerSevii_Story_Text_SelphyNewRequest::
	.string "SELPHY: I wish to see a {STR_VAR_1}.\n"
	.string "Please show one to me.$"
WayfarerSevii_Story_Text_SelphyRewardIntro::
	.string "SELPHY: A {STR_VAR_1}! Splendid!\n"
	.string "Please accept this {STR_VAR_2}.$"
WayfarerSevii_Story_Text_SelphyRewardDelivered::
	.string "SELPHY: A fitting token of my\n"
	.string "appreciation.$"
WayfarerSevii_Story_Text_SelphyWrongSpecies::
	.string "SELPHY: I am still waiting to see a\n"
	.string "{STR_VAR_1}.$"
WayfarerSevii_Story_Text_SelphyRewardNoRoom::
	.string "Your BAG is full. The {STR_VAR_2}\n"
	.string "will remain here for you.$"
WayfarerSevii_Story_Text_SelphyNotReturned::
	.string "LADY SELPHY is away from home.$"
WayfarerSevii_Story_Text_SelphyRequestUnavailable::
	.string "SELPHY: Let us speak of another wish\n"
	.string "a little later.$"
WayfarerSevii_Story_Text_SelphyButlerHome::
	.string "My lady's smile has regained the\n"
	.string "brilliance of the sun.$"
WayfarerSevii_Story_Text_SelphyButlerWaiting::
	.string "Please bring LADY SELPHY home safely.$"
