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
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_MtEmberExterior_EndOnLoad
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_43, WayfarerSevii_MtEmberExterior_EndOnLoad
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_44, WayfarerSevii_MtEmberExterior_EndOnLoad
	setmetatile 42, 39, METATILE_MtEmber_CaveEntrance, 0
WayfarerSevii_MtEmberExterior_EndOnLoad:
	end

WayfarerSevii_EventScript_RubyGuardPasswordScene::
	lockall
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_RubyLead_NotStartedAll
	goto_if_set FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED, WayfarerSevii_RubyLead_PasswordKnown
	msgbox WayfarerSevii_Text_RubyGuardsRevealPassword
	setflag FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED
WayfarerSevii_RubyLead_PasswordKnown:
	releaseall
	end

WayfarerSevii_EventScript_RubyGuard1::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_RubyLead_NotStarted
	call WayfarerSevii_RubyLead_EnsurePassword
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_43, WayfarerSevii_Text_RubyGuard1Intro, WayfarerSevii_Text_RubyGuard1Defeat
	call WayfarerSevii_RubyLead_TryOpenCave
	msgbox WayfarerSevii_Text_RubyGuard1PostBattle
	release
	end

WayfarerSevii_EventScript_RubyGuard2::
	lock
	faceplayer
	goto_if_unset FLAG_WAYFARER_SEVII_CELIO_GEMS_STARTED, WayfarerSevii_RubyLead_NotStarted
	call WayfarerSevii_RubyLead_EnsurePassword
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_44, WayfarerSevii_Text_RubyGuard2Intro, WayfarerSevii_Text_RubyGuard2Defeat
	call WayfarerSevii_RubyLead_TryOpenCave
	msgbox WayfarerSevii_Text_RubyGuard2PostBattle
	release
	end

WayfarerSevii_RubyLead_EnsurePassword:
	goto_if_set FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED, WayfarerSevii_RubyLead_EndEnsurePassword
	msgbox WayfarerSevii_Text_RubyGuardsRevealPassword
	setflag FLAG_WAYFARER_SEVII_PASSWORD_ONE_LEARNED
WayfarerSevii_RubyLead_EndEnsurePassword:
	return

WayfarerSevii_RubyLead_TryOpenCave:
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_43, WayfarerSevii_RubyLead_EndTryOpenCave
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_44, WayfarerSevii_RubyLead_EndTryOpenCave
	setmetatile 42, 39, METATILE_MtEmber_CaveEntrance, 0
	special DrawWholeMapView
WayfarerSevii_RubyLead_EndTryOpenCave:
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
	msgbox WayfarerSevii_Text_SapphireScientistRevealsPassword
	setflag FLAG_WAYFARER_SEVII_PASSWORD_TWO_LEARNED
	removeobject LOCALID_DOTTED_HOLE_THIEF
	release
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

WayfarerSevii_EventScript_WarehouseGrunt42::
	lock
	faceplayer
	call WayfarerSevii_Warehouse_CheckReady
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Warehouse_NotReady
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_42, WayfarerSevii_Text_WarehouseGruntIntro, WayfarerSevii_Text_WarehouseGruntDefeat
	msgbox WayfarerSevii_Text_WarehouseGruntPostBattle
	release
	end

WayfarerSevii_EventScript_WarehouseAdmin1::
	lock
	faceplayer
	call WayfarerSevii_Warehouse_CheckReady
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Warehouse_NotReady
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_42, WayfarerSevii_Warehouse_PredecessorPending
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN, WayfarerSevii_Text_WarehouseAdmin1Intro, WayfarerSevii_Text_WarehouseAdmin1Defeat
	msgbox WayfarerSevii_Text_WarehouseAdmin1PostBattle
	release
	end

WayfarerSevii_EventScript_WarehouseGrunt47::
	lock
	faceplayer
	call WayfarerSevii_Warehouse_CheckReady
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Warehouse_NotReady
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN, WayfarerSevii_Warehouse_PredecessorPending
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_47, WayfarerSevii_Text_WarehouseGruntIntro, WayfarerSevii_Text_WarehouseGruntDefeat
	msgbox WayfarerSevii_Text_WarehouseGruntPostBattle
	release
	end

WayfarerSevii_EventScript_WarehouseGrunt48::
	lock
	faceplayer
	call WayfarerSevii_Warehouse_CheckReady
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Warehouse_NotReady
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_47, WayfarerSevii_Warehouse_PredecessorPending
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_48, WayfarerSevii_Text_WarehouseGruntIntro, WayfarerSevii_Text_WarehouseGruntDefeat
	msgbox WayfarerSevii_Text_WarehouseGruntPostBattle
	release
	end

WayfarerSevii_EventScript_WarehouseAdmin2::
	lock
	faceplayer
	call WayfarerSevii_Warehouse_CheckReady
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Warehouse_NotReady
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_GRUNT_48, WayfarerSevii_Warehouse_PredecessorPending
	trainerbattle_single TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN_2, WayfarerSevii_Text_WarehouseAdmin2Intro, WayfarerSevii_Text_WarehouseAdmin2Defeat, WayfarerSevii_Warehouse_Admin2Victory
	msgbox WayfarerSevii_Text_WarehouseAdmin2PostBattle
	release
	end

WayfarerSevii_Warehouse_Admin2Victory:
	specialvar VAR_RESULT, WayfarerSevii_IsMainlandGiovanniComplete
	goto_if_eq VAR_RESULT, TRUE, WayfarerSevii_Warehouse_Admin2GiovanniComplete
	msgbox WayfarerSevii_Text_WarehouseAdmin2LocalEnding
	return

WayfarerSevii_Warehouse_Admin2GiovanniComplete:
	msgbox WayfarerSevii_Text_WarehouseAdmin2GiovanniEnding
	return

WayfarerSevii_EventScript_WarehouseGideon::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_SEVII_SAPPHIRE_RECOVERED, WayfarerSevii_Warehouse_GideonAfterRecovery
	goto_if_set FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED, WayfarerSevii_Warehouse_TryGiveSapphire
	call WayfarerSevii_Warehouse_CheckReady
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Warehouse_NotReady
	goto_if_not_defeated TRAINER_WAYFARER_SEVII_TEAM_ROCKET_ADMIN_2, WayfarerSevii_Warehouse_PredecessorPending
	trainerbattle_single TRAINER_WAYFARER_SEVII_SCIENTIST_GIDEON, WayfarerSevii_Text_WarehouseGideonIntro, WayfarerSevii_Text_WarehouseGideonDefeat, WayfarerSevii_Warehouse_GideonVictory
	goto WayfarerSevii_Warehouse_TryGiveSapphire

WayfarerSevii_Warehouse_GideonVictory:
	@ The clear commits before reward capacity is checked.  A full Key Items
	@ pocket therefore retries only this handoff and never the ordered battles.
	setflag FLAG_WAYFARER_SEVII_WAREHOUSE_CLEARED
	msgbox WayfarerSevii_Text_WarehouseGideonPostBattle
	return

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
	goto_if_set FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN, WayfarerSevii_Rival_NoScene
	specialvar VAR_RESULT, WayfarerSevii_HasAnyLeagueClear
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Rival_NoScene
	lockall
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	msgbox WayfarerSevii_Text_FourIslandRivalScene
	setflag FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN
	fadedefaultbgm
	releaseall
WayfarerSevii_Rival_NoScene:
	return

WayfarerSevii_SixIsland_RivalTryScene::
	goto_if_set FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN, WayfarerSevii_Rival_NoScene
	specialvar VAR_RESULT, WayfarerSevii_HasAnyLeagueClear
	goto_if_eq VAR_RESULT, FALSE, WayfarerSevii_Rival_NoScene
	lockall
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	msgbox WayfarerSevii_Text_SixIslandRivalScene
	setflag FLAG_WAYFARER_SEVII_RIVAL_SCENE_SEEN
	fadedefaultbgm
	releaseall
	return

WayfarerSevii_EventScript_FourIslandRival::
	msgbox WayfarerSevii_Text_FourIslandRivalAfter, MSGBOX_AUTOCLOSE
	end

WayfarerSevii_EventScript_SixIslandRival::
	msgbox WayfarerSevii_Text_SixIslandRivalAfter, MSGBOX_AUTOCLOSE
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
	special BattleSetup_StartLegendaryBattle
	waitstate
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
	.string "I'm trying to repair this Network\n"
	.string "Machine, but it needs two gems.\p"
	.string "A RUBY lies somewhere on MT. EMBER,\n"
	.string "and a SAPPHIRE is hidden on SIX\l"
	.string "ISLAND. Will you investigate?$"

WayfarerSevii_Text_CelioInvestigationAccepted:
	.string "Thank you! You can pursue either\n"
	.string "lead first. Bring back whatever you\l"
	.string "find, and please be careful.$"

WayfarerSevii_Text_CelioOfferDeclined:
	.string "I understand. The offer will still\n"
	.string "be here whenever you're ready.$"

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
	.string "defend the chamber.$"

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
	.string "The ROCKET guards still bar the\n"
	.string "chamber.$"
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
	.string "{RIVAL}: Hey, {PLAYER}!\n"
	.string "I came looking for rare POKéMON.\p"
	.string "The islands are bigger than either\n"
	.string "of us expected. Smell ya later!$"
WayfarerSevii_Text_SixIslandRivalScene:
	.string "{RIVAL}: Hey, {PLAYER}!\n"
	.string "Finding every POKéMON here isn't\l"
	.string "easy, even for a LEAGUE winner.\p"
	.string "Keep exploring. Smell ya later!$"
WayfarerSevii_Text_FourIslandRivalAfter:
	.string "{RIVAL} has already moved on from\n"
	.string "FOUR ISLAND.$"
WayfarerSevii_Text_SixIslandRivalAfter:
	.string "{RIVAL} has already continued the\n"
	.string "island search.$"

WayfarerSevii_Text_MoltresNotReady:
	.string "MOLTRES radiates overwhelming heat.\n"
	.string "It is too dangerous to challenge\l"
	.string "before Trainer Rating 55.$"
WayfarerSevii_Text_MoltresResolved:
	.string "Only warm volcanic air remains.$"
WayfarerSevii_Text_MoltresCry:
	.string "Gyaoo!$"
