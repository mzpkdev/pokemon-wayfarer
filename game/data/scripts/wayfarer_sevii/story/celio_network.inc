@ Wayfarer Sevii story: Celio's gems, the rival scenes, and Moltres.
@
@ These scripts intentionally never touch ferry, PC, Pokédex, Champion, or
@ mainland campaign state.  Trainer defeat persistence is owned by the shared
@ generated Trainer router; story scripts only reference its generated IDs.

@ -----------------------------------------------------------------------------
@ Celio's independent two-gem investigation

WayfarerSevii_EventScript_Celio::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_CELIO_REPAIR_COMPLETE, WayfarerSevii_Celio_RepairComplete
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_Celio_OfferInvestigation
	goto_if_set FLAG_WAYFARER_SEVII_RUBY_DELIVERED, WayfarerSevii_Celio_CheckSapphireDelivery
	checkitem ITEM_RUBY, 1
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_Celio_DeliverRuby
WayfarerSevii_Celio_CheckSapphireDelivery:
	goto_if_set FLAG_WAYFARER_SEVII_SAPPHIRE_DELIVERED, WayfarerSevii_Celio_CheckRepair
	checkitem ITEM_SAPPHIRE, 1
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_Celio_DeliverSapphire
WayfarerSevii_Celio_CheckRepair:
	goto_if_unset FLAG_WAYFARER_SEVII_RUBY_DELIVERED, WayfarerSevii_Celio_WaitingForBoth
	goto_if_unset FLAG_WAYFARER_SEVII_SAPPHIRE_DELIVERED, WayfarerSevii_Celio_WaitingForBoth
	setflag FLAG_WAYFARER_SEVII_CELIO_REPAIR_COMPLETE
	msgbox WayfarerSevii_Text_CelioRepairFinished
	release
	end

WayfarerSevii_Celio_OfferInvestigation:
	msgbox WayfarerSevii_Text_CelioOfferInvestigation, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WayfarerSevii_Celio_OfferDeclined
	setflag FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED
	clearflag FLAG_WAYFARER_SEVII_HIDE_DOTTED_HOLE_SCIENTIST
	clearflag FLAG_WAYFARER_SEVII_HIDE_LOCAL_ROCKETS
	clearflag FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS
	msgbox WayfarerSevii_Text_CelioInvestigationAccepted
	release
	end

WayfarerSevii_Celio_OfferDeclined:
	msgbox WayfarerSevii_Text_CelioOfferDeclined
	release
	end

WayfarerSevii_Celio_DeliverRuby:
	setvar VAR_0x8004, ITEM_RUBY
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_RUBY_DELIVERED
	specialvar VAR_RESULT, WayfarerSevii_TryRemoveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Celio_WaitingForBoth
	setflag FLAG_WAYFARER_SEVII_RUBY_DELIVERED
	msgbox WayfarerSevii_Text_CelioRubyDelivered
	goto WayfarerSevii_Celio_AfterDelivery

WayfarerSevii_Celio_DeliverSapphire:
	setvar VAR_0x8004, ITEM_SAPPHIRE
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_SAPPHIRE_DELIVERED
	specialvar VAR_RESULT, WayfarerSevii_TryRemoveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Celio_WaitingForBoth
	setflag FLAG_WAYFARER_SEVII_SAPPHIRE_DELIVERED
	msgbox WayfarerSevii_Text_CelioSapphireDelivered

WayfarerSevii_Celio_AfterDelivery:
	goto_if_unset FLAG_WAYFARER_SEVII_RUBY_DELIVERED, WayfarerSevii_Celio_StillNeedsRuby
	goto_if_unset FLAG_WAYFARER_SEVII_SAPPHIRE_DELIVERED, WayfarerSevii_Celio_StillNeedsSapphire
	setflag FLAG_WAYFARER_SEVII_CELIO_REPAIR_COMPLETE
	msgbox WayfarerSevii_Text_CelioRepairFinished
	release
	end

WayfarerSevii_Celio_StillNeedsRuby:
	msgbox WayfarerSevii_Text_CelioStillNeedsRuby
	release
	end

WayfarerSevii_Celio_StillNeedsSapphire:
	msgbox WayfarerSevii_Text_CelioStillNeedsSapphire
	release
	end

WayfarerSevii_Celio_WaitingForBoth:
	goto_if_set FLAG_WAYFARER_SEVII_RUBY_DELIVERED, WayfarerSevii_Celio_StillNeedsSapphire
	goto_if_set FLAG_WAYFARER_SEVII_SAPPHIRE_DELIVERED, WayfarerSevii_Celio_StillNeedsRuby
	msgbox WayfarerSevii_Text_CelioWaitingForBoth
	release
	end

WayfarerSevii_Celio_RepairComplete:
	msgbox WayfarerSevii_Text_CelioRepairComplete
	release
	end

@ -----------------------------------------------------------------------------
@ Mt. Ember Ruby lead

WayfarerSevii_MtEmberExterior_OnLoad::
	@ Ruby Path is exploration content: the entrance is always open. Only the
	@ Ruby itself waits on Celio's request and the guards.
	setmetatile 42, 39, METATILE_MtEmber_CaveEntrance, 0
	end

WayfarerSevii_EventScript_RubyGuardPasswordScene::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_RubyLead_NotStartedAll
	goto_if_set FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED, WayfarerSevii_RubyLead_PasswordKnown
	msgbox WayfarerSevii_Text_RubyGuardsRevealPassword
	setflag FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED
	call WayfarerSevii_Story_UpdateWarehouseReadiness
WayfarerSevii_RubyLead_PasswordKnown:
	releaseall
	end

WayfarerSevii_EventScript_RubyGuard1::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_RubyLead_NotStarted
	call WayfarerSevii_RubyLead_EnsurePassword
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_43, WayfarerSevii_Text_RubyGuard1Intro, WayfarerSevii_Text_RubyGuard1Defeat
	msgbox WayfarerSevii_Text_RubyGuard1PostBattle
	call WayfarerSevii_RubyLead_DismissGuards
	release
	end

WayfarerSevii_EventScript_RubyGuard2::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_RubyLead_NotStarted
	call WayfarerSevii_RubyLead_EnsurePassword
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_44, WayfarerSevii_Text_RubyGuard2Intro, WayfarerSevii_Text_RubyGuard2Defeat
	msgbox WayfarerSevii_Text_RubyGuard2PostBattle
	call WayfarerSevii_RubyLead_DismissGuards
	release
	end

WayfarerSevii_RubyLead_EnsurePassword:
	goto_if_set FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED, WayfarerSevii_RubyLead_EndEnsurePassword
	msgbox WayfarerSevii_Text_RubyGuardsRevealPassword
	setflag FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED
	call WayfarerSevii_Story_UpdateWarehouseReadiness
WayfarerSevii_RubyLead_EndEnsurePassword:
	return

@ Once both guards are beaten they leave their post, as in FRLG, and stay gone
@ on reload through the shared hide flag.
WayfarerSevii_RubyLead_DismissGuards:
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_43, WayfarerSevii_RubyLead_EndDismissGuards
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_44, WayfarerSevii_RubyLead_EndDismissGuards
	setflag FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS
	removeobject LOCALID_MT_EMBER_GRUNT1
	removeobject LOCALID_MT_EMBER_GRUNT2
WayfarerSevii_RubyLead_EndDismissGuards:
	return

WayfarerSevii_RubyLead_NotStarted:
	msgbox WayfarerSevii_Text_RubyLeadNotStarted
	release
	end

WayfarerSevii_RubyLead_NotStartedAll:
	msgbox WayfarerSevii_Text_RubyLeadNotStarted
	releaseall
	end

WayfarerSevii_EventScript_Ruby::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_Ruby_NotStarted
	goto_if_set FLAG_WAYFARER_SEVII_RUBY_RECOVERED, WayfarerSevii_Ruby_AlreadyRecovered
	goto_if_unset FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED, WayfarerSevii_Ruby_GuardsRemain
	@ Both objective guards must be won before the chamber transaction advances.
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_43, WayfarerSevii_Ruby_GuardsRemain
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_44, WayfarerSevii_Ruby_GuardsRemain
	setvar VAR_0x8004, ITEM_RUBY
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_RUBY_RECOVERED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Ruby_NoRoom
	setflag FLAG_WAYFARER_SEVII_RUBY_RECOVERED
	setflag FLAG_WAYFARER_SEVII_HIDE_RUBY_GUARDS
	msgreceiveditem WayfarerSevii_Text_FoundRuby, ITEM_RUBY, 1, MUS_RG_OBTAIN_KEY_ITEM
	removeobject VAR_LAST_TALKED
	release
	end

WayfarerSevii_Ruby_NotStarted:
	msgbox WayfarerSevii_Text_RubyDormant
	release
	end

WayfarerSevii_Ruby_GuardsRemain:
	msgbox WayfarerSevii_Text_RubyGuardsRemain
	release
	end

WayfarerSevii_Ruby_NoRoom:
	msgbox WayfarerSevii_Text_RubyNoRoom
	release
	end

WayfarerSevii_Ruby_AlreadyRecovered:
	msgbox WayfarerSevii_Text_RubyAlreadyRecovered
	release
	end

@ -----------------------------------------------------------------------------
@ Dotted Hole Sapphire lead

WayfarerSevii_EventScript_SapphireScene::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_Sapphire_NotStarted
	goto_if_set FLAG_WAYFARER_SEVII_SAPPHIRE_STOLEN, WayfarerSevii_Sapphire_AlreadyStolen
	goto_if_set FLAG_WAYFARER_SEVII_SAPPHIRE_FOUND, WayfarerSevii_Sapphire_CommitTheft
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message WayfarerSevii_Text_FoundSapphire
	waitmessage
	waitfanfare
	closemessage
	setflag FLAG_WAYFARER_SEVII_SAPPHIRE_FOUND
WayfarerSevii_Sapphire_CommitTheft:
	msgbox WayfarerSevii_Text_SapphireScientistStealsGem
	removeobject VAR_LAST_TALKED
	setflag FLAG_WAYFARER_SEVII_SAPPHIRE_STOLEN
	setflag FLAG_WAYFARER_SEVII_HIDE_DOTTED_HOLE_SCIENTIST
	msgbox WayfarerSevii_Text_SapphireScientistRevealsPassword
	setflag FLAG_WAYFARER_SEVII_PASSWORD_TWO_LEARNED
	call WayfarerSevii_Story_UpdateWarehouseReadiness
	removeobject LOCALID_DOTTED_HOLE_THIEF
	release
	end

@ The scientist is scene staging, not a second trigger. Only the Sapphire
@ object may start the theft so VAR_LAST_TALKED removes the correct object.
WayfarerSevii_EventScript_SapphireScientist::
	end

WayfarerSevii_Sapphire_NotStarted:
	msgbox WayfarerSevii_Text_SapphireDormant
	release
	end

WayfarerSevii_Sapphire_AlreadyStolen:
	msgbox WayfarerSevii_Text_SapphireAlreadyStolen
	release
	end

@ -----------------------------------------------------------------------------
@ Rocket Warehouse.  The exterior remains open; these checks gate only the
@ confrontation.  Every objective loss returns through normal battle recovery,
@ and the generated Trainer bit leaves the first undefeated opponent pending.

WayfarerSevii_Warehouse_CheckReady:
	setvar VAR_RESULT, FALSE
	goto_if_unset FLAG_WAYFARER_SEVII_SAPPHIRE_STOLEN, WayfarerSevii_Warehouse_CheckReadyDone
	goto_if_unset FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED, WayfarerSevii_Warehouse_CheckReadyDone
	goto_if_unset FLAG_WAYFARER_SEVII_PASSWORD_TWO_LEARNED, WayfarerSevii_Warehouse_CheckReadyDone
	setvar VAR_RESULT, TRUE
WayfarerSevii_Warehouse_CheckReadyDone:
	return

@ Passwords can be learned in either order. The final prerequisite transition
@ presents the first combatant and Gideon; each win reveals the next opponent,
@ and Warehouse clear later hides all five so the Sapphire handoff remains retryable.
WayfarerSevii_Story_UpdateWarehouseReadiness:
	goto_if_unset FLAG_WAYFARER_SEVII_SAPPHIRE_STOLEN, WayfarerSevii_Story_UpdateWarehouseReadinessDone
	goto_if_unset FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED, WayfarerSevii_Story_UpdateWarehouseReadinessDone
	goto_if_unset FLAG_WAYFARER_SEVII_PASSWORD_TWO_LEARNED, WayfarerSevii_Story_UpdateWarehouseReadinessDone
	clearflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_COMBATANTS
	clearflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GIDEON
WayfarerSevii_Story_UpdateWarehouseReadinessDone:
	return

WayfarerSevii_Warehouse_NotReady:
	goto_if_unset FLAG_WAYFARER_SEVII_SAPPHIRE_STOLEN, WayfarerSevii_Warehouse_Empty
	msgbox WayfarerSevii_Text_WarehouseNeedsBothPasswords
	release
	end

WayfarerSevii_Warehouse_Empty:
	msgbox WayfarerSevii_Text_WarehouseEmpty
	release
	end

WayfarerSevii_Warehouse_PredecessorPending:
	msgbox WayfarerSevii_Text_WarehousePredecessorPending
	release
	end

@ The combatants are sight trainers, so each script reaches `trainerbattle` before
@ any effectful command: a beaten Rocket never walks up again. Order is
@ enforced by presentation instead of by dialogue. Each opponent appears only
@ once the previous one falls, so nobody out of turn can stop the player.
WayfarerSevii_EventScript_WarehouseGrunt42::
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_42, WayfarerSevii_Text_WarehouseGruntIntro, WayfarerSevii_Text_WarehouseGruntDefeat
	call WayfarerSevii_Warehouse_RevealAdmin1
	msgbox WayfarerSevii_Text_WarehouseGruntPostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_EventScript_WarehouseAdmin1::
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN, WayfarerSevii_Text_WarehouseAdmin1Intro, WayfarerSevii_Text_WarehouseAdmin1Defeat
	call WayfarerSevii_Warehouse_RevealGrunt47
	msgbox WayfarerSevii_Text_WarehouseAdmin1PostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_EventScript_WarehouseGrunt47::
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_47, WayfarerSevii_Text_WarehouseGruntIntro, WayfarerSevii_Text_WarehouseGruntDefeat
	call WayfarerSevii_Warehouse_RevealGrunt48
	msgbox WayfarerSevii_Text_WarehouseGruntPostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_EventScript_WarehouseGrunt48::
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_48, WayfarerSevii_Text_WarehouseGruntIntro, WayfarerSevii_Text_WarehouseGruntDefeat
	call WayfarerSevii_Warehouse_RevealAdmin2
	msgbox WayfarerSevii_Text_WarehouseGruntPostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_EventScript_WarehouseAdmin2::
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN_2, WayfarerSevii_Text_WarehouseAdmin2Intro, WayfarerSevii_Text_WarehouseAdmin2Defeat, WayfarerSevii_Warehouse_Admin2Victory
	msgbox WayfarerSevii_Text_WarehouseAdmin2PostBattle, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Warehouse_RevealAdmin1:
	goto_if_unset FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_ADMIN_1, WayfarerSevii_Warehouse_RevealDone
	clearflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_ADMIN_1
	addobject LOCALID_WAREHOUSE_ADMIN1
	return

WayfarerSevii_Warehouse_RevealGrunt47:
	goto_if_unset FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GRUNT_47, WayfarerSevii_Warehouse_RevealDone
	clearflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GRUNT_47
	addobject LOCALID_WAREHOUSE_GRUNT2
	return

WayfarerSevii_Warehouse_RevealGrunt48:
	goto_if_unset FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GRUNT_48, WayfarerSevii_Warehouse_RevealDone
	clearflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GRUNT_48
	addobject LOCALID_WAREHOUSE_GRUNT3
	return

WayfarerSevii_Warehouse_RevealAdmin2:
	goto_if_unset FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_ADMIN_2, WayfarerSevii_Warehouse_RevealDone
	clearflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_ADMIN_2
	addobject LOCALID_WAREHOUSE_ADMIN2
WayfarerSevii_Warehouse_RevealDone:
	return

WayfarerSevii_Warehouse_Admin2Victory:
	specialvar VAR_RESULT, WayfarerSevii_IsMainlandGiovanniComplete
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_Warehouse_Admin2GiovanniComplete
	msgbox WayfarerSevii_Text_WarehouseAdmin2LocalEnding, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Warehouse_Admin2GiovanniComplete:
	msgbox WayfarerSevii_Text_WarehouseAdmin2GiovanniEnding, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_EventScript_WarehouseGideon::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_SAPPHIRE_RECOVERED, WayfarerSevii_Warehouse_GideonAfterRecovery
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_Warehouse_TryGiveSapphire
	goto_if_defeated TRAINER_WAYFARER_SEVII_SCIENTIST_GIDEON, WayfarerSevii_Warehouse_TryGiveSapphire
	call WayfarerSevii_Warehouse_CheckReady
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Warehouse_NotReady
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN_2, WayfarerSevii_Warehouse_PredecessorPending
	trainerbattle_single TRAINER_WAYFARER_SEVII_SCIENTIST_GIDEON, WayfarerSevii_Text_WarehouseGideonIntro, WayfarerSevii_Text_WarehouseGideonDefeat, WayfarerSevii_Warehouse_GideonVictory
	goto WayfarerSevii_Warehouse_TryGiveSapphire

WayfarerSevii_Warehouse_GideonVictory:
	@ Clear commits before capacity is checked, while Gideon's separate staging
	@ flag keeps only his reward retry available afterward.
	setflag FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED
	setflag FLAG_WAYFARER_SEVII_HIDE_LOCAL_ROCKETS
	setflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_COMBATANTS
	setflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_ADMIN_1
	setflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GRUNT_47
	setflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_GRUNT_48
	setflag FLAG_WAYFARER_SEVII_HIDE_WAREHOUSE_ADMIN_2
	removeobject LOCALID_WAREHOUSE_GRUNT2
	removeobject LOCALID_WAREHOUSE_GRUNT3
	removeobject LOCALID_WAREHOUSE_ADMIN1
	removeobject LOCALID_WAREHOUSE_ADMIN2
	removeobject LOCALID_WAREHOUSE_GRUNT1
	msgbox WayfarerSevii_Text_WarehouseGideonPostBattle
	goto WayfarerSevii_Warehouse_TryGiveSapphire

WayfarerSevii_Warehouse_TryGiveSapphire:
	setvar VAR_0x8004, ITEM_SAPPHIRE
	setvar VAR_0x8005, FLAG_WAYFARER_SEVII_SAPPHIRE_RECOVERED
	specialvar VAR_RESULT, WayfarerSevii_TryGiveItemThenSetFlag
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Warehouse_SapphireNoRoom
	setflag FLAG_WAYFARER_SEVII_SAPPHIRE_RECOVERED
	msgreceiveditem WayfarerSevii_Text_RecoveredSapphire, ITEM_SAPPHIRE, 1, MUS_RG_OBTAIN_KEY_ITEM
	msgbox WayfarerSevii_Text_WarehouseSapphireRecovered
	release
	end

WayfarerSevii_Warehouse_SapphireNoRoom:
	msgbox WayfarerSevii_Text_WarehouseSapphireNoRoom
	release
	end

WayfarerSevii_Warehouse_GideonAfterRecovery:
	msgbox WayfarerSevii_Text_WarehouseGideonAfterRecovery
	release
	end

@ -----------------------------------------------------------------------------
@ Shared, non-battle rival scene.  The manifest calls the location-specific
@ Try label on arrival and uses the Talk label for the actor afterward.

WayfarerSevii_FourIsland_RivalOnFrame::
	map_script_2 VAR_TEMP_8, 0, WayfarerSevii_FourIsland_RivalTryScene
	.2byte 0

WayfarerSevii_SixIsland_RivalOnFrame::
	map_script_2 VAR_TEMP_8, 0, WayfarerSevii_SixIsland_RivalTryScene
	.2byte 0

WayfarerSevii_FourIsland_RivalTryScene::
	setvar VAR_TEMP_8, 1
	goto_if_set FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN, WayfarerSevii_Rival_NoScene
	specialvar VAR_RESULT, WayfarerSevii_HasAnyLeagueClear
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Rival_NoScene
	call WayfarerSevii_Rival_KnowsBlue
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Rival_NoScene
	lockall
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	setflag FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN
	clearflag FLAG_WAYFARER_SEVII_HIDE_RIVALS
	addobject LOCALID_FOUR_ISLAND_RIVAL
	msgbox WayfarerSevii_Text_FourIslandRivalScene
	@ BLUE is a one-time visitor: he leaves after the scene, so he is never
	@ standing on both islands and both copies stay hidden afterward.
	setflag FLAG_WAYFARER_SEVII_HIDE_RIVALS
	removeobject LOCALID_FOUR_ISLAND_RIVAL
	fadedefaultbgm
	releaseall
WayfarerSevii_Rival_NoScene:
	return

@ BLUE only greets a player he knows: one from PALLET TOWN grew up beside him, anyone else has to have met him
@ somewhere first. VAR_RESULT: TRUE when the scene may play.
WayfarerSevii_Rival_KnowsBlue::
	specialvar VAR_RESULT, WayfarerKanto_IsPalletOrigin
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Rival_KnowsBlueNotPallet
	return

WayfarerSevii_Rival_KnowsBlueNotPallet::
	setvar VAR_0x8004, NOTABLE_TRAINER_BLUE
	specialvar VAR_RESULT, NotableStory_IsIntroduced
	return

WayfarerSevii_SixIsland_RivalTryScene::
	setvar VAR_TEMP_8, 1
	goto_if_set FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN, WayfarerSevii_Rival_NoScene
	specialvar VAR_RESULT, WayfarerSevii_HasAnyLeagueClear
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Rival_NoScene
	call WayfarerSevii_Rival_KnowsBlue
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Rival_NoScene
	lockall
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	setflag FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN
	clearflag FLAG_WAYFARER_SEVII_HIDE_RIVALS
	addobject LOCALID_SIX_ISLAND_RIVAL
	msgbox WayfarerSevii_Text_SixIslandRivalScene
	@ BLUE is a one-time visitor: he leaves after the scene, so he is never
	@ standing on both islands and both copies stay hidden afterward.
	setflag FLAG_WAYFARER_SEVII_HIDE_RIVALS
	removeobject LOCALID_SIX_ISLAND_RIVAL
	fadedefaultbgm
	releaseall
	return

WayfarerSevii_EventScript_FourIslandRival::
	goto_if_unset FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN, WayfarerSevii_Rival_NoSceneTalk
	msgbox WayfarerSevii_Text_FourIslandRivalAfter, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_EventScript_SixIslandRival::
	goto_if_unset FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN, WayfarerSevii_Rival_NoSceneTalk
	msgbox WayfarerSevii_Text_SixIslandRivalAfter, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_Rival_NoSceneTalk:
	end

@ -----------------------------------------------------------------------------
@ Mt. Ember Moltres.  Resolution is written only for catch or knockout.

WayfarerSevii_EventScript_Moltres::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_MOLTRES_RESOLVED, WayfarerSevii_Moltres_Resolved
	specialvar VAR_RESULT, WayfarerSevii_IsMoltresEligible
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Moltres_NotReady
	setwildbattle SPECIES_MOLTRES, 50
	waitse
	playmoncry SPECIES_MOLTRES, CRY_MODE_ENCOUNTER
	message WayfarerSevii_Text_MoltresCry
	waitmessage
	waitmoncry
	delay 10
	playbgm MUS_RG_ENCOUNTER_GYM_LEADER, 0
	waitbuttonpress
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, WayfarerSevii_ResolveMoltresBattle
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_Moltres_CommitResolution
	@ Run, teleport, loss, draw, and interrupted starts keep the encounter pending.
	release
	end

WayfarerSevii_Moltres_CommitResolution:
	setflag FLAG_WAYFARER_SEVII_MOLTRES_RESOLVED
	removeobject VAR_LAST_TALKED
	release
	end

WayfarerSevii_Moltres_NotReady:
	msgbox WayfarerSevii_Text_MoltresNotReady
	release
	end

WayfarerSevii_Moltres_Resolved:
	msgbox WayfarerSevii_Text_MoltresResolved
	release
	end

@ -----------------------------------------------------------------------------
@ Text

WayfarerSevii_Text_CelioOfferInvestigation:
	.string "CELIO: Oh, hello! I'm CELIO. I'm\n"
	.string "trying to repair this Network\l"
	.string "Machine, but it needs two gems.\p"
	.string "A RUBY lies somewhere on MT. EMBER,\n"
	.string "and a SAPPHIRE is hidden on SIX\l"
	.string "ISLAND. Will you investigate?$"

WayfarerSevii_Text_CelioInvestigationAccepted:
	.string "CELIO: Thank you! You can pursue\n"
	.string "either lead first.\p"
	.string "Bring back whatever you find, and\n"
	.string "please be careful.$"

WayfarerSevii_Text_CelioOfferDeclined:
	.string "CELIO: I understand. The offer will\n"
	.string "still be here whenever you're ready.$"

WayfarerSevii_Text_CelioRubyDelivered:
	.string "You found the RUBY! This is exactly\n"
	.string "what the machine needs.$"

WayfarerSevii_Text_CelioSapphireDelivered:
	.string "The SAPPHIRE! Thank you for\n"
	.string "recovering it from TEAM ROCKET.$"

WayfarerSevii_Text_CelioStillNeedsRuby:
	.string "One gem is safely installed.\n"
	.string "The RUBY from MT. EMBER remains.$"

WayfarerSevii_Text_CelioStillNeedsSapphire:
	.string "One gem is safely installed.\n"
	.string "The SAPPHIRE on SIX ISLAND remains.$"

WayfarerSevii_Text_CelioWaitingForBoth:
	.string "The RUBY and SAPPHIRE may be\n"
	.string "investigated in either order.\p"
	.string "I'll be here when you recover one.$"

WayfarerSevii_Text_CelioRepairFinished:
	.string "Both gems are in place! The local\n"
	.string "repair is finally complete.\p"
	.string "You really helped me out, {PLAYER}.\n"
	.string "Thank you.$"

WayfarerSevii_Text_CelioRepairComplete:
	.string "The Network Machine is running\n"
	.string "smoothly thanks to you.\p"
	.string "The islands' other services were\n"
	.string "never tied to this repair.$"

WayfarerSevii_Text_RubyGuardsRevealPassword:
	.string "The ROCKETS let a password slip:\n"
	.string "“GOLDEEN need log.”\p"
	.string "They spot you listening and move to\n"
	.string "defend their dig.$"

WayfarerSevii_Text_RubyGuard1Intro:
	.string "You've been eavesdropping on us,\n"
	.string "haven't you?$"
WayfarerSevii_Text_RubyGuard1Defeat:
	.string "Huh, what?$"
WayfarerSevii_Text_RubyGuard1PostBattle:
	.string "Why'd you have to win?$"
WayfarerSevii_Text_RubyGuard2Intro:
	.string "Trying to horn in on our treasure?\n"
	.string "Don't bet on it!$"
WayfarerSevii_Text_RubyGuard2Defeat:
	.string "Wait! But how?$"
WayfarerSevii_Text_RubyGuard2PostBattle:
	.string "Forget everything you heard!$"
WayfarerSevii_Text_RubyLeadNotStarted:
	.string "The ROCKETS are guarding their dig,\n"
	.string "but you have no local lead yet.$"
WayfarerSevii_Text_FoundRuby:
	.string "{PLAYER} found a RUBY!$"
WayfarerSevii_Text_RubyDormant:
	.string "A red gem glitters here, but CELIO\n"
	.string "hasn't asked you to investigate.$"
WayfarerSevii_Text_RubyGuardsRemain:
	.string "The ROCKETS outside would notice if\n"
	.string "you took it. Deal with the guards.$"
WayfarerSevii_Text_RubyNoRoom:
	.string "The RUBY remains safely here.\n"
	.string "Make room in the KEY ITEMS pocket.$"
WayfarerSevii_Text_RubyAlreadyRecovered:
	.string "The place where the RUBY rested is\n"
	.string "empty.$"

WayfarerSevii_Text_FoundSapphire:
	.string "{PLAYER} discovered the SAPPHIRE!$"
WayfarerSevii_Text_SapphireScientistStealsGem:
	.string "A scientist drops into the chamber\n"
	.string "and snatches the SAPPHIRE!\p"
	.string "He plans to sell it to TEAM ROCKET.$"
WayfarerSevii_Text_SapphireScientistRevealsPassword:
	.string "Before fleeing, he reveals the\n"
	.string "password “Yes, nah, CHANSEY.”$"
WayfarerSevii_Text_SapphireDormant:
	.string "A blue gem glitters here, but CELIO\n"
	.string "hasn't asked you to investigate.$"
WayfarerSevii_Text_SapphireAlreadyStolen:
	.string "The SAPPHIRE was taken to the\n"
	.string "ROCKET WAREHOUSE.$"

WayfarerSevii_Text_WarehouseEmpty:
	.string "The WAREHOUSE is open to explore,\n"
	.string "but no operation is active here.$"
WayfarerSevii_Text_WarehouseNeedsBothPasswords:
	.string "The operation is locked down.\n"
	.string "Both passwords are still needed.$"
WayfarerSevii_Text_WarehousePredecessorPending:
	.string "Another ROCKET blocks the route\n"
	.string "ahead. Deal with them first.$"
WayfarerSevii_Text_WarehouseGruntIntro:
	.string "You won't disrupt TEAM ROCKET's\n"
	.string "WAREHOUSE!$"
WayfarerSevii_Text_WarehouseGruntDefeat:
	.string "What do you think you're doing?!$"
WayfarerSevii_Text_WarehouseGruntPostBattle:
	.string "Keep moving. The ADMINS will stop\n"
	.string "you.$"
WayfarerSevii_Text_WarehouseAdmin1Intro:
	.string "I put my faith in GIOVANNI and do\n"
	.string "as I am told.$"
WayfarerSevii_Text_WarehouseAdmin1Defeat:
	.string "I… I'm shattered…$"
WayfarerSevii_Text_WarehouseAdmin1PostBattle:
	.string "You act according to your beliefs.\n"
	.string "I understand that now.$"
WayfarerSevii_Text_WarehouseAdmin2Intro:
	.string "That's enough of you playing the\n"
	.string "hero. This operation ends you here!$"
WayfarerSevii_Text_WarehouseAdmin2Defeat:
	.string "Urgh… You were too strong…$"
WayfarerSevii_Text_WarehouseAdmin2PostBattle:
	.string "This WAREHOUSE has been abandoned.$"
WayfarerSevii_Text_WarehouseAdmin2LocalEnding:
	.string "This loss ends our Sevii operation.\n"
	.string "TEAM ROCKET abandons the WAREHOUSE.$"
WayfarerSevii_Text_WarehouseAdmin2GiovanniEnding:
	.string "So GIOVANNI was defeated, and TEAM\n"
	.string "ROCKET truly disbanded…\p"
	.string "We abandon this WAREHOUSE.$"
WayfarerSevii_Text_WarehouseGideonIntro:
	.string "I almost sold the SAPPHIRE for a\n"
	.string "fortune. I won't forgive you!$"
WayfarerSevii_Text_WarehouseGideonDefeat:
	.string "Darn! Darn!$"
WayfarerSevii_Text_WarehouseGideonPostBattle:
	.string "If I can't sell it, take it!$"
WayfarerSevii_Text_RecoveredSapphire:
	.string "{PLAYER} recovered the SAPPHIRE!$"
WayfarerSevii_Text_WarehouseSapphireRecovered:
	.string "The stolen SAPPHIRE is safe again.$"
WayfarerSevii_Text_WarehouseSapphireNoRoom:
	.string "Make room in the KEY ITEMS pocket.\n"
	.string "I'll hold the SAPPHIRE until then.$"
WayfarerSevii_Text_WarehouseGideonAfterRecovery:
	.string "You have the SAPPHIRE. Leave me be.$"

WayfarerSevii_Text_FourIslandRivalScene:
	.string "BLUE: Hey, {PLAYER}!\n"
	.string "I came looking for rare POKéMON.\p"
	.string "The islands are bigger than either\n"
	.string "of us expected. Smell ya later!$"
WayfarerSevii_Text_SixIslandRivalScene:
	.string "BLUE: Hey, {PLAYER}!\n"
	.string "Finding every POKéMON here isn't\l"
	.string "easy, even for a LEAGUE winner.\p"
	.string "Keep exploring. Smell ya later!$"
WayfarerSevii_Text_FourIslandRivalAfter:
	.string "BLUE has already moved on from\n"
	.string "FOUR ISLAND.$"
WayfarerSevii_Text_SixIslandRivalAfter:
	.string "BLUE has already continued the\n"
	.string "island search.$"

WayfarerSevii_Text_MoltresNotReady:
	.string "MOLTRES radiates overwhelming heat.\p"
	.string "You need more experience before\n"
	.string "you can challenge it.$"
WayfarerSevii_Text_MoltresResolved:
	.string "Only warm volcanic air remains.$"
WayfarerSevii_Text_MoltresCry:
	.string "Gyaoo!$"

@ The Warehouse pens keep FRLG's text. The source flag that FRLG checks is a
@ stub in Wayfarer, so the unlocked text follows the story's clearance flag.
WayfarerSevii_RocketWarehouse_Cage::
	lockall
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_RocketWarehouse_CageUnlocked
	msgbox FiveIsland_RocketWarehouse_Text_ManyMonsLockedInPen
	releaseall
	end

WayfarerSevii_RocketWarehouse_CageUnlocked::
	msgbox FiveIsland_RocketWarehouse_Text_PenUnlockedMonsFled
	releaseall
	end
