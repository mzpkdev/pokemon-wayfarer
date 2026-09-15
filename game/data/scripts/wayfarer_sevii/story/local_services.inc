@ Wayfarer-owned Sevii local adventures, rewards, and services.
@
@ Restored here: the Tectonix Lemonade/TM42 transaction, the Water
@ Labyrinth Togepi Egg, the Water Path Heracross size challenge, the five
@ local move services (Explosion, Body Slam, Swords Dance, Cape Brink's
@ starter moves, and the Two Island Move Maniac), the four optional local
@ Rocket confrontations, Dunsparce Tunnel's prospector, Chansey Dance, and the
@ complete campaign-free passive inventory selected by the story manifest.
@
@ Adapted: every one-time grant records receipt only after delivery succeeds;
@ Tectonix records the accepted/consumed offering separately from TM receipt;
@ an offered Egg remains pending while the party is full; Heracross keeps a
@ pending Nest Ball claim after the size record is committed.
@
@ Explicit exclusions: Sticker Man depends on Hall-of-Fame sticker totals;
@ Seven Island's visiting Trainer depends on e-Reader data; Mystery Gift
@ deliverymen and upstairs link attendants remain owned by the shared
@ communications layer. The 135-map source inventory contains no in-game NPC
@ trade; ordinary link trades therefore remain a shared-service concern rather
@ than inventing a new local trade. Item balls, field-obstacle objects, ordinary
@ Trainers, Trainer Tower actors, Bill/Celio, Lostelle, Lorelei, Selphy's
@ rescue, rival scenes, Moltres, and the gem/Warehouse objective chain are
@ owned by their respective overlay modules rather than this file.

@ ---------------------------------------------------------------------------
@ Memorial Pillar: consume the accepted Lemonade first, then keep TM42 as a
@ separate retryable receipt. The caretaker is also a reachable retry point.

WayfarerSevii_Story_TectonixCaretaker::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_TECTONIX_REWARD_RECEIVED, WayfarerSevii_Story_TectonixCaretakerComplete
	goto_if_set FLAG_WAYFARER_SEVII_TECTONIX_OFFERING_COMPLETE, WayfarerSevii_Story_TectonixTryGiveReward
	msgbox FiveIsland_MemorialPillar_Text_ScrubScrub
	msgbox FiveIsland_MemorialPillar_Text_YourMonsLookHealthy
	msgbox FiveIsland_MemorialPillar_Text_ThisIsWhereIBuriedMyOnix
	release
	end

WayfarerSevii_Story_TectonixMemorial::
	lockall
	goto_if_set FLAG_WAYFARER_SEVII_TECTONIX_OFFERING_COMPLETE, WayfarerSevii_Story_TectonixMemorialComplete
	msgbox WayfarerSevii_Story_Text_TectonixMemorial
	checkitem ITEM_LEMONADE
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_TectonixNoLemonade
	msgbox FiveIsland_MemorialPillar_Text_LeaveAnotherLemonadeOffering, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Story_TectonixDeclined
	removeitem ITEM_LEMONADE
	setflag FLAG_WAYFARER_SEVII_TECTONIX_OFFERING_COMPLETE
	msgbox FiveIsland_MemorialPillar_Text_PlacedCanOfLemonade
	textcolor NPC_TEXT_COLOR_MALE
	msgbox FiveIsland_MemorialPillar_Text_ThankYouPleaseTakeThis
	goto WayfarerSevii_Story_TectonixTryGiveReward

WayfarerSevii_Story_TectonixTryGiveReward::
	giveitem ITEM_TM42
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_TectonixRewardFull
	setflag FLAG_WAYFARER_SEVII_TECTONIX_REWARD_RECEIVED
	msgbox FiveIsland_MemorialPillar_Text_BeGoodToYourMonsToo
	releaseall
	end

WayfarerSevii_Story_TectonixRewardFull::
	msgbox FiveIsland_MemorialPillar_Text_DontHaveRoomForIt
	releaseall
	end

WayfarerSevii_Story_TectonixCaretakerComplete::
	msgbox FiveIsland_MemorialPillar_Text_ScrubScrubTectonix
	release
	end

WayfarerSevii_Story_TectonixMemorialComplete::
	msgbox FiveIsland_MemorialPillar_Text_HereLiesTectonix
	releaseall
	end

WayfarerSevii_Story_TectonixNoLemonade::
	msgbox WayfarerSevii_Story_Text_TectonixNeedsLemonade
	releaseall
	end

WayfarerSevii_Story_TectonixDeclined::
	releaseall
	end

WayfarerSevii_Story_Text_TectonixMemorial::
	.string "“Here lies TECTONIX” is engraved\n"
	.string "on the memorial.\p"
	.string "A fresh LEMONADE would make a\n"
	.string "fitting offering.$"

WayfarerSevii_Story_Text_TectonixNeedsLemonade::
	.string "The memorial is carefully tended.\p"
	.string "You don't have a LEMONADE to\n"
	.string "leave as an offering.$"

@ ---------------------------------------------------------------------------
@ Water Labyrinth: the friendship test is paid only before the offer becomes
@ active. Once offered, capacity failures return directly to Egg delivery.

WayfarerSevii_Story_WaterLabyrinthEggGiver::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_EGG_RECEIVED, WayfarerSevii_Story_WaterLabyrinthEggPost
	goto_if_set FLAG_WAYFARER_SEVII_EGG_OFFERED, WayfarerSevii_Story_WaterLabyrinthTryGiveEgg
	msgbox FiveIsland_WaterLabyrinth_Text_LetMeTakeLookAtMons
	specialvar VAR_RESULT, GetLeadMonFriendship
	goto_if_ne VAR_RESULT, 6, WayfarerSevii_Story_WaterLabyrinthEggNotReady
	msgbox FiveIsland_WaterLabyrinth_Text_TreatMonRightHaveThis
	setflag FLAG_WAYFARER_SEVII_EGG_OFFERED

WayfarerSevii_Story_WaterLabyrinthTryGiveEgg::
	setvar VAR_0x8004, SPECIES_TOGEPI
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_EGG_RECEIVED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveEggThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_WaterLabyrinthEggPartyFull
	@ Keep the receipt visible to the host script audit; the atomic helper has
	@ already committed it only after the Egg exists in the party.
	setflag FLAG_WAYFARER_SEVII_EGG_RECEIVED
	textcolor NPC_TEXT_COLOR_NEUTRAL
	playfanfare MUS_LEVEL_UP
	message FiveIsland_WaterLabyrinth_Text_ReceivedEggFromMan
	waitfanfare
	waitmessage
	call EventScript_RestorePrevTextColor
	goto WayfarerSevii_Story_WaterLabyrinthEggPost

WayfarerSevii_Story_WaterLabyrinthEggNotReady::
	msgbox FiveIsland_WaterLabyrinth_Text_HmmISeeIsee
	release
	end

WayfarerSevii_Story_WaterLabyrinthEggPartyFull::
	msgbox FiveIsland_WaterLabyrinth_Text_DontHaveSpaceInYourParty
	release
	end

WayfarerSevii_Story_WaterLabyrinthEggPost::
	msgbox FiveIsland_WaterLabyrinth_Text_GladIMetSomeoneLikeYou
	release
	end

@ ---------------------------------------------------------------------------
@ Water Path Heracross measurement. CompareHeracrossSize commits a new record
@ before returning 3, so a failed Nest Ball delivery is recovered explicitly
@ on the later tied/smaller paths rather than requiring another record.

WayfarerSevii_Story_HeracrossJudge::
	lock
	faceplayer
	setvar VAR_0x8004, SPECIES_HERACROSS
	specialvar VAR_RESULT, DoesPlayerPartyContainSpecies
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_HeracrossAbsent
	special GetHeracrossSizeRecordInfo
	msgbox SixIsland_WaterPath_House1_Text_MayIMeasureHeracross
	special ChoosePartyMon
	waitstate
	copyvar VAR_RESULT, VAR_0x8004
	goto_if_ge VAR_RESULT, PARTY_SIZE, WayfarerSevii_Story_HeracrossCancel
	special CompareHeracrossSize
	goto_if_eq VAR_RESULT, 1, WayfarerSevii_Story_HeracrossWrongSpecies
	goto_if_eq VAR_RESULT, 3, WayfarerSevii_Story_HeracrossNewRecord
	goto_if_unset FLAG_WAYFARER_SEVII_HERACROSS_REWARD_RECEIVED, WayfarerSevii_Story_HeracrossTryGiveReward
	msgbox SixIsland_WaterPath_House1_Text_ItsXInchesYInchesWasBiggest
	release
	end

WayfarerSevii_Story_HeracrossNewRecord::
	msgbox SixIsland_WaterPath_House1_Text_ItsXInchesDeserveReward

WayfarerSevii_Story_HeracrossTryGiveReward::
	giveitem ITEM_NEST_BALL
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_HeracrossRewardFull
	setflag FLAG_WAYFARER_SEVII_HERACROSS_REWARD_RECEIVED
	msgbox SixIsland_WaterPath_House1_Text_WantToSeeBiggerOne
	release
	end

WayfarerSevii_Story_HeracrossAbsent::
	msgbox SixIsland_WaterPath_House1_Text_LoveItNeedItHeracross
	release
	end

WayfarerSevii_Story_HeracrossWrongSpecies::
	msgbox SixIsland_WaterPath_House1_Text_ThisWontDo
	release
	end

WayfarerSevii_Story_HeracrossRewardFull::
	msgbox SixIsland_WaterPath_House1_Text_YourBagIsFull
	release
	end

WayfarerSevii_Story_HeracrossCancel::
	release
	end

WayfarerSevii_Story_HeracrossSizeRecord::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_HERACROSS_REWARD_RECEIVED, WayfarerSevii_Story_HeracrossSizeRecordBlank
	special GetHeracrossSizeRecordInfo
	msgbox SixIsland_WaterPath_House1_Text_BiggestHeracrossIsXInches
	releaseall
	end

WayfarerSevii_Story_HeracrossSizeRecordBlank::
	msgbox SixIsland_WaterPath_House1_Text_BlankChartOfSomeSort
	releaseall
	end

WayfarerSevii_Story_WaterPathNeighbor::
	msgbox SixIsland_WaterPath_House2_Text_MyNeighborMeasuredMe, MSGBOX_NPC
	end

@ Dunsparce Tunnel: the source National Dex gate is unrelated to this local
@ discovery. The Nugget remains pending until it actually fits in the Bag.
WayfarerSevii_Story_DunsparceProspector::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_DUNSPARCE_NUGGET_RECEIVED, WayfarerSevii_Story_DunsparceProspectorComplete
	msgbox ThreeIsland_DunsparceTunnel_Text_StruckGoldThisIsForYou
	giveitem ITEM_NUGGET
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_DunsparceProspectorFull
	setflag FLAG_WAYFARER_SEVII_DUNSPARCE_NUGGET_RECEIVED

WayfarerSevii_Story_DunsparceProspectorComplete::
	msgbox ThreeIsland_DunsparceTunnel_Text_ThatsANugget
	release
	end

WayfarerSevii_Story_DunsparceProspectorFull::
	msgbox ThreeIsland_DunsparceTunnel_Text_WhoopsBagCrammedFull
	release
	end

@ ---------------------------------------------------------------------------
@ One-time move tutors. These use Wayfarer-specific receipt flags because the
@ imported FRLG flags are intentionally zero in the Wayfarer constants view.

WayfarerSevii_Story_ExplosionTutor::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_TUTOR_EXPLOSION, EventScript_ExplosionTaught
	msgbox Text_ExplosionTeach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_ExplosionDeclined
	call EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, EventScript_ExplosionDeclined
	msgbox Text_ExplosionWhichMon
	setvar VAR_0x8005, MOVE_EXPLOSION
	call EventScript_ChooseMoveTutorMon
	goto_if_eq VAR_RESULT, FALSE, EventScript_ExplosionDeclined
	setflag FLAG_WAYFARER_SEVII_TUTOR_EXPLOSION
	goto EventScript_ExplosionTaught

WayfarerSevii_Story_BodySlamTutor::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_TUTOR_BODY_SLAM, EventScript_BodySlamTaught
	msgbox Text_BodySlamTeach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_BodySlamDeclined
	call EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, EventScript_BodySlamDeclined
	msgbox Text_BodySlamWhichMon
	setvar VAR_0x8005, MOVE_BODY_SLAM
	call EventScript_ChooseMoveTutorMon
	goto_if_eq VAR_RESULT, FALSE, EventScript_BodySlamDeclined
	setflag FLAG_WAYFARER_SEVII_TUTOR_BODY_SLAM
	goto EventScript_BodySlamTaught

WayfarerSevii_Story_SwordsDanceTutor::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_TUTOR_SWORDS_DANCE, EventScript_SwordsDanceTaught
	msgbox Text_SwordsDanceTeach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_SwordsDanceDeclined
	call EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, EventScript_SwordsDanceDeclined
	msgbox Text_SwordsDanceWhichMon
	setvar VAR_0x8005, MOVE_SWORDS_DANCE
	call EventScript_ChooseMoveTutorMon
	goto_if_eq VAR_RESULT, FALSE, EventScript_SwordsDanceDeclined
	setflag FLAG_WAYFARER_SEVII_TUTOR_SWORDS_DANCE
	goto EventScript_SwordsDanceTaught

@ CapeBrinkGetMoveToTeachLeadPokemon is a shared eligibility special whose
@ Wayfarer branch reads the three receipt flags named below. This script owns
@ the receipt write and commits it only after move teaching succeeds.
WayfarerSevii_Story_CapeBrinkTutor::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_TUTOR_FRENZY_PLANT, WayfarerSevii_Story_CapeBrinkCheckLead
	goto_if_unset FLAG_WAYFARER_SEVII_TUTOR_BLAST_BURN, WayfarerSevii_Story_CapeBrinkCheckLead
	goto_if_set FLAG_WAYFARER_SEVII_TUTOR_HYDRO_CANNON, WayfarerSevii_Story_CapeBrinkAllAlreadyTaught

WayfarerSevii_Story_CapeBrinkCheckLead::
	goto_if_set FLAG_TEMP_2, WayfarerSevii_Story_CapeBrinkMoveJustTaught
	bufferleadmonspeciesname STR_VAR_1
	msgbox Text_UltimateMoveThatMon
	specialvar VAR_RESULT, CapeBrinkGetMoveToTeachLeadPokemon
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_CapeBrinkNoEligibleLead
	copyvar VAR_0x8009, VAR_0x8005
	msgbox Text_AllowMeToTeachMonUltimateMove, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Story_CapeBrinkDeclined
	call EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Story_CapeBrinkDeclined
	msgbox Text_LetMeConferUltimateMove
	closemessage
	fadescreen FADE_TO_BLACK
	call EventScript_ChooseMoveTutorMon
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_CapeBrinkDeclined
	copyvar VAR_0x8005, VAR_0x8009
	call WayfarerSevii_Story_CapeBrinkCommitReceipt
	goto_if_unset FLAG_WAYFARER_SEVII_TUTOR_FRENZY_PLANT, WayfarerSevii_Story_CapeBrinkTaughtOne
	goto_if_unset FLAG_WAYFARER_SEVII_TUTOR_BLAST_BURN, WayfarerSevii_Story_CapeBrinkTaughtOne
	goto_if_set FLAG_WAYFARER_SEVII_TUTOR_HYDRO_CANNON, WayfarerSevii_Story_CapeBrinkAllTaught

WayfarerSevii_Story_CapeBrinkTaughtOne::
	msgbox Text_TaughtMove
	setflag FLAG_TEMP_2
	release
	end

WayfarerSevii_Story_CapeBrinkCommitReceipt::
	goto_if_eq VAR_0x8005, MOVE_FRENZY_PLANT, WayfarerSevii_Story_CapeBrinkCommitFrenzyPlant
	goto_if_eq VAR_0x8005, MOVE_BLAST_BURN, WayfarerSevii_Story_CapeBrinkCommitBlastBurn
	setflag FLAG_WAYFARER_SEVII_TUTOR_HYDRO_CANNON
	return

WayfarerSevii_Story_CapeBrinkCommitFrenzyPlant::
	setflag FLAG_WAYFARER_SEVII_TUTOR_FRENZY_PLANT
	return

WayfarerSevii_Story_CapeBrinkCommitBlastBurn::
	setflag FLAG_WAYFARER_SEVII_TUTOR_BLAST_BURN
	return

WayfarerSevii_Story_CapeBrinkAllTaught::
	msgbox Text_PassedOnEverythingIKnow
	release
	end

WayfarerSevii_Story_CapeBrinkAllAlreadyTaught::
	msgbox Text_FeelRejuvenated
	release
	end

WayfarerSevii_Story_CapeBrinkMoveJustTaught::
	msgbox Text_MakeBestUseOfMove
	release
	end

WayfarerSevii_Story_CapeBrinkNoEligibleLead::
	msgbox Text_JustMistaken
	release
	end

WayfarerSevii_Story_CapeBrinkDeclined::
	msgbox Text_YouRejectIt
	release
	end

@ The Move Maniac is repeatable and consumes mushrooms only after the player
@ has successfully selected and taught a relearnable move.
.equ WAYFARER_SEVII_CHECKED_MUSHROOMS,  FLAG_TEMP_1
.equ WAYFARER_SEVII_HAS_BIG_MUSHROOM,   FLAG_TEMP_2
.equ WAYFARER_SEVII_HAS_TINY_MUSHROOMS, FLAG_TEMP_3
.equ WAYFARER_SEVII_HAS_BOTH_MUSHROOMS, FLAG_TEMP_4

WayfarerSevii_Story_MoveManiac::
	lock
	faceplayer
	clearflag WAYFARER_SEVII_HAS_BIG_MUSHROOM
	clearflag WAYFARER_SEVII_HAS_TINY_MUSHROOMS
	clearflag WAYFARER_SEVII_HAS_BOTH_MUSHROOMS
	goto_if_set WAYFARER_SEVII_CHECKED_MUSHROOMS, WayfarerSevii_Story_MoveManiacCheckMushrooms
	msgbox TwoIsland_House_Text_TeachMonMoveForMushroom
	setflag WAYFARER_SEVII_CHECKED_MUSHROOMS

WayfarerSevii_Story_MoveManiacCheckMushrooms::
	checkitem ITEM_BIG_MUSHROOM
	call_if_eq VAR_RESULT, TRUE, WayfarerSevii_Story_MoveManiacHasBigMushroom
	checkitem ITEM_TINY_MUSHROOM, 2
	call_if_eq VAR_RESULT, TRUE, WayfarerSevii_Story_MoveManiacHasTinyMushrooms
	goto_if_set WAYFARER_SEVII_HAS_BIG_MUSHROOM, WayfarerSevii_Story_MoveManiacCheckBothFromBig
	goto_if_set WAYFARER_SEVII_HAS_TINY_MUSHROOMS, WayfarerSevii_Story_MoveManiacCheckBothFromTiny
	goto WayfarerSevii_Story_MoveManiacEnd

WayfarerSevii_Story_MoveManiacCheckBothFromBig::
	call_if_set WAYFARER_SEVII_HAS_TINY_MUSHROOMS, WayfarerSevii_Story_MoveManiacHasBoth
	goto WayfarerSevii_Story_MoveManiacAsk

WayfarerSevii_Story_MoveManiacCheckBothFromTiny::
	call_if_set WAYFARER_SEVII_HAS_BIG_MUSHROOM, WayfarerSevii_Story_MoveManiacHasBoth

WayfarerSevii_Story_MoveManiacAsk::
	msgbox TwoIsland_House_Text_WantMeToTeachMove, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Story_MoveManiacEnd
	msgbox TwoIsland_House_Text_TutorWhichMon
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, WayfarerSevii_Story_MoveManiacEnd
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_Story_MoveManiacEgg
	goto_if_eq VAR_0x8005, 0, WayfarerSevii_Story_MoveManiacNoMove
	msgbox TwoIsland_House_Text_TeachWhichMove
	special TeachMoveRelearnerMove
	waitstate
	goto_if_eq VAR_0x8004, 0, WayfarerSevii_Story_MoveManiacAsk
	goto_if_set WAYFARER_SEVII_HAS_BOTH_MUSHROOMS, WayfarerSevii_Story_MoveManiacChoosePayment
	goto_if_set WAYFARER_SEVII_HAS_BIG_MUSHROOM, WayfarerSevii_Story_MoveManiacPayBig
	goto WayfarerSevii_Story_MoveManiacPayTiny

WayfarerSevii_Story_MoveManiacChoosePayment::
	message TwoIsland_House_Text_IllNeedMushroomOrTwo
	waitmessage
	multichoice 0, 0, MULTI_MUSHROOMS, TRUE
	goto_if_eq VAR_RESULT, 0, WayfarerSevii_Story_MoveManiacPayTiny
	goto_if_eq VAR_RESULT, 1, WayfarerSevii_Story_MoveManiacPayBig
	goto WayfarerSevii_Story_MoveManiacEnd

WayfarerSevii_Story_MoveManiacPayBig::
	removeitem ITEM_BIG_MUSHROOM
	msgbox TwoIsland_House_Text_HandedOverOneBigMushroom
	goto WayfarerSevii_Story_MoveManiacEnd

WayfarerSevii_Story_MoveManiacPayTiny::
	removeitem ITEM_TINY_MUSHROOM, 2
	msgbox TwoIsland_House_Text_HandedOverTwoTinyMushrooms
	goto WayfarerSevii_Story_MoveManiacEnd

WayfarerSevii_Story_MoveManiacNoMove::
	msgbox TwoIsland_House_Text_NoMoveICanTeachThatMon
	goto WayfarerSevii_Story_MoveManiacAsk

WayfarerSevii_Story_MoveManiacEgg::
	msgbox TwoIsland_House_Text_CantTeachAnEgg
	goto WayfarerSevii_Story_MoveManiacAsk

WayfarerSevii_Story_MoveManiacEnd::
	msgbox TwoIsland_House_Text_NeedMushroomOrTwoToLearnMove
	release
	end

WayfarerSevii_Story_MoveManiacHasBigMushroom::
	setflag WAYFARER_SEVII_HAS_BIG_MUSHROOM
	return

WayfarerSevii_Story_MoveManiacHasTinyMushrooms::
	setflag WAYFARER_SEVII_HAS_TINY_MUSHROOMS
	return

WayfarerSevii_Story_MoveManiacHasBoth::
	setflag WAYFARER_SEVII_HAS_BOTH_MUSHROOMS
	return

@ ---------------------------------------------------------------------------
@ Optional local Rockets: these appear only during Celio's investigation and
@ never teach passwords or advance either gem. Generated Trainer IDs preserve
@ source parties and route defeat state through the Trainer-owned slot router.

WayfarerSevii_Story_MeadowRocket1::
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_Story_LocalRocketInactive
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_Story_LocalRocketAbandoned
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_49, FiveIsland_Meadow_Text_Rocket1Intro, FiveIsland_Meadow_Text_Rocket1Defeat
	msgbox FiveIsland_Meadow_Text_Rocket1PostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Story_MeadowRocket2::
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_Story_LocalRocketInactive
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_Story_LocalRocketAbandoned
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_50, FiveIsland_Meadow_Text_Rocket2Intro, FiveIsland_Meadow_Text_Rocket2Defeat
	msgbox FiveIsland_Meadow_Text_Rocket2PostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Story_MeadowRocket3::
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_Story_LocalRocketInactive
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_Story_LocalRocketAbandoned
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_51, FiveIsland_Meadow_Text_Rocket3Intro, FiveIsland_Meadow_Text_Rocket3Defeat
	msgbox FiveIsland_Meadow_Text_Rocket3PostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Story_OutcastRocket::
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_Story_LocalRocketInactive
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_Story_LocalRocketAbandoned
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_46, SixIsland_OutcastIsland_Text_RocketIntro, SixIsland_OutcastIsland_Text_RocketDefeat
	msgbox SixIsland_OutcastIsland_Text_RocketPostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Story_LocalRocketInactive::
	msgbox WayfarerSevii_Story_Text_LocalRocketInactive, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Story_LocalRocketAbandoned::
	msgbox WayfarerSevii_Story_Text_LocalRocketAbandoned, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Story_Text_LocalRocketInactive::
	.string "The stranger keeps watch without\n"
	.string "saying what the local crew wants.$"

WayfarerSevii_Story_Text_LocalRocketAbandoned::
	.string "The SEVII operation is finished.\n"
	.string "There's nothing left here for us.$"

@ ---------------------------------------------------------------------------
@ Campaign-free passive actors retained with their source-local dialogue.

WayfarerSevii_Story_FourIslandHouseFatMan::
	msgbox FourIsland_House1_Text_YoureAwfullyHeavy, MSGBOX_NPC
	end

WayfarerSevii_Story_SevenIslandOldWoman::
	msgbox SevenIsland_Text_IslandsMadeInSevenDays, MSGBOX_NPC
	end

WayfarerSevii_Story_SevenIslandScientist::
	msgbox SevenIsland_Text_IslandVirtuallyUntouched, MSGBOX_NPC
	end

WayfarerSevii_Story_SevenIslandSign::
	msgbox SevenIsland_Text_IslandSign, MSGBOX_SIGN
	end

WayfarerSevii_Story_ChanseyDanceMan::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_2, WayfarerSevii_Story_ChanseyDanceAlreadyDone
	msgbox SevenIsland_SevaultCanyon_House_Text_ChanseyDanceJoinIn
	textcolor NPC_TEXT_COLOR_NEUTRAL
	msgbox SevenIsland_SevaultCanyon_House_Text_WouldYouLikeToDance, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Story_ChanseyDanceDeclined
	msgbox SevenIsland_SevaultCanyon_House_Text_DancedChanseyDance
	closemessage
	playbgm MUS_SCHOOL, 0
	applymovement LOCALID_PLAYER, WayfarerSevii_Story_Movement_ChanseyDance
	waitmovement 0
	fadedefaultbgm
	call Common_EventScript_OutOfCenterPartyHeal
	msgbox SevenIsland_SevaultCanyon_House_Text_YoureAllChipperNow
	setflag FLAG_TEMP_2
	release
	end

WayfarerSevii_Story_ChanseyDanceAlreadyDone::
	msgbox SevenIsland_SevaultCanyon_House_Text_ChanseyDanceJoinIn
	release
	end

WayfarerSevii_Story_ChanseyDanceDeclined::
	textcolor NPC_TEXT_COLOR_MALE
	msgbox SevenIsland_SevaultCanyon_House_Text_ComeOnDance
	release
	end

WayfarerSevii_Story_Chansey::
	lock
	faceplayer
	playmoncry SPECIES_CHANSEY, CRY_MODE_NORMAL
	msgbox SevenIsland_SevaultCanyon_House_Text_Chansey
	waitmoncry
	release
	end

WayfarerSevii_Story_Movement_ChanseyDance::
	walk_in_place_down
	delay_16
	walk_in_place_down
	delay_16
	walk_in_place_faster_right
	delay_4
	walk_in_place_faster_up
	delay_4
	walk_in_place_faster_left
	delay_4
	walk_in_place_faster_down
	step_end

WayfarerSevii_Story_FiveIslandHouseWoman::
	msgbox FiveIsland_House1_Text_HusbandWentOffFishing, MSGBOX_NPC
	end

WayfarerSevii_Story_FiveIslandHouseOldMan::
	msgbox FiveIsland_House2_Text_MeadowBelongedToFamily, MSGBOX_NPC
	end

@ ---------------------------------------------------------------------------
@ Complete passive inventory selected by the story manifest. Stable export
@ names include the source map and zero-based object index so the generated
@ audit can account for each actor without depending on display text.

WayfarerSevii_Story_Passive_OneIslandKindleRoadEmberSpa_0::
	msgbox OneIsland_KindleRoad_EmberSpa_Text_WaterWarmsMeToCore, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_OneIslandKindleRoadEmberSpa_1::
	msgbox OneIsland_KindleRoad_EmberSpa_Text_EnjoyBowlOfChowder, MSGBOX_NPC
	end

@ The Bruno Fame Checker dependency is presentation-only and removed here.
WayfarerSevii_Story_Passive_OneIslandKindleRoadEmberSpa_2::
	msgbox OneIsland_KindleRoad_EmberSpa_Text_BrunoVisitsSpaOnOccasion, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_OneIslandKindleRoadEmberSpa_3::
	msgbox OneIsland_KindleRoad_EmberSpa_Text_SeeHowSmoothMySkinIs, MSGBOX_NPC
	end

@ Rock Smash is a local one-time gift, independent of campaign progression.
@ A full Key Items pocket leaves both the item and its receipt pending.
WayfarerSevii_Story_Passive_OneIslandKindleRoadEmberSpa_4::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_EMBER_SPA_ROCK_SMASH_RECEIVED, WayfarerSevii_Story_EmberSpaRockSmashReceived
	msgbox OneIsland_KindleRoad_EmberSpa_Text_UsedThisToMakeEmberSpa
	giveitem ITEM_HM06
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Story_EmberSpaRockSmashFull
	setflag FLAG_WAYFARER_SEVII_EMBER_SPA_ROCK_SMASH_RECEIVED

WayfarerSevii_Story_EmberSpaRockSmashReceived::
	msgbox OneIsland_KindleRoad_EmberSpa_Text_ExplainHM06
	release
	end

WayfarerSevii_Story_EmberSpaRockSmashFull::
	msgbox WayfarerSevii_Story_Text_EmberSpaRockSmashFull
	release
	end

WayfarerSevii_Story_Text_EmberSpaRockSmashFull::
	.string "Your KEY ITEMS pocket is full.\p"
	.string "I'll keep this for you until you\n"
	.string "have room.$"

WayfarerSevii_Story_Passive_OneIslandKindleRoadEmberSpa_5::
	msgbox OneIsland_KindleRoad_EmberSpa_Text_HotSpringIsTherapeutic, MSGBOX_NPC
	end

@ This One Island elder always uses his local Celio introduction. The source
@ post-Champion/network variants would claim communications changes that the
@ independent repair objective explicitly does not make.
WayfarerSevii_Story_Passive_OneIsland_1::
	msgbox OneIsland_Text_LuckyToHaveCelioHere, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_OneIsland_2::
	msgbox OneIsland_Text_IsntWarmClimateHereGreat, MSGBOX_NPC
	end


@ Two Island's outdoor shop exposes its complete local stock without tying
@ expansion to Lostelle, Champion, National Dex, or network state.
WayfarerSevii_Story_Passive_TwoIsland_0::
	lock
	faceplayer
	message TwoIsland_Text_BringingItemsFromDistantLands
	waitmessage
	pokemart WayfarerSevii_Story_TwoIslandShopItems
	msgbox gText_PleaseComeAgain
	release
	end

	.align 2
WayfarerSevii_Story_TwoIslandShopItems::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_REPEAT_BALL
	.2byte ITEM_TIMER_BALL
	.2byte ITEM_LEMONADE
	.2byte ITEM_SODA_POP
	.2byte ITEM_FRESH_WATER
	.2byte ITEM_MOOMOO_MILK
	.2byte ITEM_LAVA_COOKIE
	.2byte ITEM_NONE

WayfarerSevii_Story_Passive_TwoIsland_1::
	msgbox TwoIsland_Text_ShopkeepersBrotherWorksGameCorner, MSGBOX_NPC
	end

@ The Bruno gossip is local flavor; the source Fame Checker side effect is
@ omitted because it is unrelated to the conversation.
WayfarerSevii_Story_Passive_TwoIsland_2::
	msgbox TwoIsland_Text_BrunoCameToIslandWhileBack, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_TwoIsland_3::
	msgbox TwoIsland_Text_FellowMovedFromCeladonCity, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_TwoIsland_4::
	msgbox TwoIsland_Text_BuyRareItemsHere, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_TwoIsland_7::
	msgbox TwoIsland_Text_HaveYouTriedJumpingGame, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_TwoIsland_8::
	msgbox TwoIsland_Text_OldWomanLivesOutOnCape, MSGBOX_NPC
	end


WayfarerSevii_Story_Passive_TwoIslandJoyfulGameCorner_3::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED, WayfarerSevii_Story_Passive_JoyfulGameCornerAfterRescue
	msgbox TwoIsland_JoyfulGameCorner_Text_NotRunningAnyGamesToday
	release
	end

WayfarerSevii_Story_Passive_JoyfulGameCornerAfterRescue::
	msgbox TwoIsland_JoyfulGameCorner_Text_GetGamesGoingSoon
	release
	end


WayfarerSevii_Story_Passive_ThreeIsland_10::
	msgbox ThreeIsland_Text_WouldntWantToSeeBikersHereAgain, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_ThreeIsland_11::
	msgbox ThreeIsland_Text_WhenDodouEvolvesGoingToPlayGame, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_ThreeIsland_12::
	lock
	faceplayer
	playmoncry SPECIES_DODUO, CRY_MODE_NORMAL
	msgbox ThreeIsland_Text_Doduo
	waitmoncry
	release
	end

WayfarerSevii_Story_Passive_ThreeIslandPort_0::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_LOSTELLE_RESCUED, WayfarerSevii_Story_Passive_ThreeIslandPortResolved
	goto_if_set FLAG_WAYFARER_SEVII_BIKERS_CLEARED, WayfarerSevii_Story_Passive_ThreeIslandPortBikersGone
	msgbox ThreeIsland_Port_Text_IllCallThePolice
	release
	end

WayfarerSevii_Story_Passive_ThreeIslandPortBikersGone::
	msgbox ThreeIsland_Port_Text_ThankGoodnessBikersGone
	release
	end

WayfarerSevii_Story_Passive_ThreeIslandPortResolved::
	msgbox ThreeIsland_Port_Text_EverythingTurnedOutForBest
	release
	end


@ Four Island's old woman keys her local observation to Lorelei's actual
@ independent objective instead of Champion state. The child's Fame Checker
@ write is likewise omitted while preserving her doll dialogue.
WayfarerSevii_Story_Passive_FourIsland_5::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_LORELEI_COMPLETE, WayfarerSevii_Story_Passive_FourIslandLoreleiComplete
	msgbox FourIsland_Text_LoreleiHasReturned
	release
	end

WayfarerSevii_Story_Passive_FourIslandLoreleiComplete::
	msgbox FourIsland_Text_LoreleiMetLaprasAsChild
	release
	end

WayfarerSevii_Story_Passive_FourIsland_10::
	msgbox FourIsland_Text_MadeSlushieUsingIcefallCaveIce, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_FourIsland_11::
	msgbox FourIsland_Text_LoreleiHasLotsOfStuffedDolls, MSGBOX_NPC
	end


WayfarerSevii_Story_Passive_FiveIsland_0::
	msgbox FiveIsland_Text_BeenGettingBusierAroundHere, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_FiveIsland_1::
	msgbox FiveIsland_Text_WeirdBuildingInMeadow, MSGBOX_NPC
	end


WayfarerSevii_Story_Passive_SixIsland_0::
	msgbox SixIsland_Text_SkyAtNightIsFantastic, MSGBOX_NPC
	end

WayfarerSevii_Story_Passive_SixIsland_1::
	msgbox SixIsland_Text_ThatWayToWaterPathRuinValley, MSGBOX_NPC
	end
