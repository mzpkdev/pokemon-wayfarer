PowerPlant_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, PowerPlant_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, PowerPlant_OnTransition
	.byte 0

PowerPlant_OnResume::
	call_if_set FLAG_SYS_CTRL_OBJ_DELETE, PowerPlant_EventScript_TryRemoveStaticMon
	end

PowerPlant_EventScript_TryRemoveStaticMon::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Common_EventScript_NopReturn
	removeobject VAR_LAST_TALKED
	return

PowerPlant_OnTransition::
	setworldmapflag FLAG_WAYFARER_WORLD_MAP_POWER_PLANT
	setescapewarp MAP_ROUTE10_POWER_PLANT_ENTRANCE_HNS, 4, 20
	call_if_unset FLAG_WAYFARER_POWER_PLANT_ZAPDOS_RESOLVED, PowerPlant_EventScript_ShowZapdos
	call_if_set FLAG_WAYFARER_POWER_PLANT_ZAPDOS_RESOLVED, PowerPlant_EventScript_HideZapdos
	end

PowerPlant_EventScript_ShowZapdos::
	clearflag FLAG_WAYFARER_POWER_PLANT_HIDE_ZAPDOS
	return

PowerPlant_EventScript_HideZapdos::
	setflag FLAG_WAYFARER_POWER_PLANT_HIDE_ZAPDOS
	return

@ The imported FRLG exit metatiles are not uniformly warp-capable in the
@ Wayfarer engine. Matching coordinate triggers preserve all five exits
@ without modifying the source layout binary.
PowerPlant_EventScript_ExitToLobby::
	warp MAP_ROUTE10_POWER_PLANT_ENTRANCE_HNS, 4, 20
	waitstate
	end

PowerPlant_EventScript_Zapdos::
	lock
	faceplayer
	specialvar VAR_RESULT, WayfarerPowerPlant_IsZapdosEligible
	goto_if_eq VAR_RESULT, FALSE, PowerPlant_EventScript_ZapdosNotReady
	seteventmon SPECIES_ZAPDOS, 50
	waitse
	playmoncry SPECIES_ZAPDOS, CRY_MODE_ENCOUNTER
	message Text_Gyaoo
	waitmessage
	waitmoncry
	delay 10
	playbgm MUS_RG_ENCOUNTER_GYM_LEADER, 0
	waitbuttonpress
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, PowerPlant_EventScript_ResolveZapdos
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, PowerPlant_EventScript_ResolveZapdos
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, PowerPlant_EventScript_ZapdosFlewAway
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, PowerPlant_EventScript_ZapdosFlewAway
	release
	end

PowerPlant_EventScript_ResolveZapdos::
	setflag FLAG_WAYFARER_POWER_PLANT_ZAPDOS_RESOLVED
	setflag FLAG_WAYFARER_POWER_PLANT_HIDE_ZAPDOS
	goto Common_EventScript_RemoveStaticPokemon

PowerPlant_EventScript_ZapdosFlewAway::
	setflag FLAG_WAYFARER_POWER_PLANT_HIDE_ZAPDOS
	setvar VAR_0x8004, SPECIES_ZAPDOS
	goto Common_EventScript_LegendaryFlewAway

PowerPlant_EventScript_ZapdosNotReady::
	msgbox PowerPlant_Text_ZapdosNotReady, MSGBOX_DEFAULT
	release
	end

PowerPlant_EventScript_Electrode1::
	lock
	faceplayer
	setwildbattle SPECIES_ELECTRODE, 34
	waitse
	playmoncry SPECIES_ELECTRODE, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	dowildbattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	setflag FLAG_WAYFARER_POWER_PLANT_ELECTRODE_1
	goto Common_EventScript_RemoveStaticPokemon

PowerPlant_EventScript_Electrode2::
	lock
	faceplayer
	setwildbattle SPECIES_ELECTRODE, 34
	waitse
	playmoncry SPECIES_ELECTRODE, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	dowildbattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	setflag FLAG_WAYFARER_POWER_PLANT_ELECTRODE_2
	goto Common_EventScript_RemoveStaticPokemon

PowerPlant_Text_ZapdosNotReady::
	.string "ZAPDOS watches you warily.\p"
	.string "You need more experience before\n"
	.string "you can challenge it.$"
