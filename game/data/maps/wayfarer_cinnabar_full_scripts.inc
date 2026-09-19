@ Full Wayfarer Cinnabar script set.  This deliberately owns no HNS or FRLG
@ story state: the active-map selector includes this file only for Wayfarer.

@ The original mansion renderer is data-only.  Bind its toggle to the
@ Wayfarer persistence flag before importing the four authored floor scripts.
#define FLAG_POKEMON_MANSION_SWITCH_STATE FLAG_WAYFARER_CINNABAR_MANSION_SWITCH_STATE
	.include "data/scripts/pokemon_mansion.inc"
	.include "data/maps/PokemonMansion_1F_Frlg/scripts.inc"
	.include "data/maps/PokemonMansion_2F_Frlg/scripts.inc"
	.include "data/maps/PokemonMansion_3F_Frlg/scripts.inc"
	.include "data/maps/PokemonMansion_B1F_Frlg/scripts.inc"
#undef FLAG_POKEMON_MANSION_SWITCH_STATE

@ Mansion item balls retain the normal Find Item behavior.  The event objects
@ carry their own Wayfarer hide flags; only the Secret Key has additional
@ gameplay state because the Gym door must remain unlocked after Bag changes.
PokemonMansion_1F_EventScript_ItemCarbos::
	finditem ITEM_CARBOS
	end
PokemonMansion_1F_EventScript_ItemEscapeRope::
	finditem ITEM_ESCAPE_ROPE
	end
PokemonMansion_1F_EventScript_ItemProtein::
	finditem ITEM_PROTEIN
	end
PokemonMansion_2F_EventScript_ItemCalcium::
	finditem ITEM_CALCIUM
	end
PokemonMansion_2F_EventScript_ItemZinc::
	finditem ITEM_ZINC
	end
PokemonMansion_2F_EventScript_ItemHPUp::
	finditem ITEM_HP_UP
	end
PokemonMansion_3F_EventScript_ItemMaxPotion::
	finditem ITEM_MAX_POTION
	end
PokemonMansion_3F_EventScript_ItemIron::
	finditem ITEM_IRON
	end
PokemonMansion_B1F_EventScript_ItemTM14::
	finditem ITEM_TM14
	end
PokemonMansion_B1F_EventScript_ItemTM22::
	finditem ITEM_TM22
	end
PokemonMansion_B1F_EventScript_ItemFullRestore::
	finditem ITEM_FULL_RESTORE
	end
PokemonMansion_B1F_EventScript_ItemSecretKey::
	finditem ITEM_SECRET_KEY
	goto_if_eq VAR_RESULT, FALSE, PokemonMansion_B1F_EventScript_ItemSecretKeyEnd
	setflag FLAG_WAYFARER_CINNABAR_SECRET_KEY
PokemonMansion_B1F_EventScript_ItemSecretKeyEnd::
	end

@ Cinnabar exterior: only the town state and Mansion-Key Gym gate are active.
@ Bill, the Seagallop, and every Sevii frame script are intentionally absent.
CinnabarIsland_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CinnabarIsland_OnTransition
	.byte 0

CinnabarIsland_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_CINNABAR_ISLAND
	setflag FLAG_VISITED_CINNABAR_ISLAND
	call CinnabarIsland_EventScript_CheckUnlockGym
	end

CinnabarIsland_EventScript_CheckUnlockGym::
	goto_if_unset FLAG_WAYFARER_CINNABAR_SECRET_KEY, CinnabarIsland_EventScript_CheckUnlockGymEnd
	setvar VAR_TEMP_1, 1
CinnabarIsland_EventScript_CheckUnlockGymEnd::
	return

CinnabarIsland_EventScript_GymDoorLocked::
	lockall
	goto_if_set FLAG_WAYFARER_CINNABAR_SECRET_KEY, CinnabarIsland_EventScript_GymDoorOpen
	msgbox CinnabarIsland_Text_GymDoorLocked, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, WayfarerCinnabar_Movement_ForcePlayerFromGymDoor
	waitmovement 0
	releaseall
	end
CinnabarIsland_EventScript_GymDoorOpen::
	setvar VAR_TEMP_1, 1
	releaseall
	end

WayfarerCinnabar_Movement_ForcePlayerFromGymDoor::
	walk_down
	step_end

CinnabarIsland_EventScript_Woman::
	msgbox CinnabarIsland_Text_Woman, MSGBOX_NPC
	end
CinnabarIsland_EventScript_OldMan::
	msgbox CinnabarIsland_Text_OldMan, MSGBOX_NPC
	end
CinnabarIsland_EventScript_IslandSign::
	msgbox CinnabarIsland_Text_IslandSign, MSGBOX_SIGN
	end
CinnabarIsland_EventScript_PokemonLabSign::
	msgbox CinnabarIsland_Text_LabSign, MSGBOX_SIGN
	end
CinnabarIsland_EventScript_GymSign::
	msgbox CinnabarIsland_Text_GymSign, MSGBOX_SIGN
	end
CinnabarIsland_Text_GymDoorLocked::
	.string "The GYM door is locked.\n"
	.string "A key may be in the MANSION.$"
CinnabarIsland_Text_Woman::
	.string "CINNABAR is a quiet island\n"
	.string "of research and discovery.$"
CinnabarIsland_Text_OldMan::
	.string "The POKéMON MANSION has many\n"
	.string "old secrets.$"
CinnabarIsland_Text_IslandSign::
	.string "CINNABAR ISLAND\n"
	.string "The Fiery Town of Burning Desire$"
CinnabarIsland_Text_LabSign::
	.string "CINNABAR ISLAND\n"
	.string "POKéMON LAB$"
CinnabarIsland_Text_GymSign::
	.string "CINNABAR ISLAND POKéMON GYM\n"
	.string "LEADER: BLAINE$"

@ Gym: quiz state is local to Wayfarer and every opened door is reconstructed
@ on load.  Wrong answers retain the FRLG trainer route without a story gate.
CinnabarIsland_Gym_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, CinnabarIsland_Gym_OnLoad
	.byte 0
CinnabarIsland_Gym_OnLoad::
	goto_if_set FLAG_WAYFARER_CINNABAR_BLAINE_DEFEATED, CinnabarIsland_Gym_OnLoadOpenAllDoors
	call_if_set FLAG_WAYFARER_CINNABAR_GYM_QUIZ_1, CinnabarIsland_Gym_OnLoadOpenDoor1
	call_if_set FLAG_WAYFARER_CINNABAR_GYM_QUIZ_2, CinnabarIsland_Gym_OnLoadOpenDoor2
	call_if_set FLAG_WAYFARER_CINNABAR_GYM_QUIZ_3, CinnabarIsland_Gym_OnLoadOpenDoor3
	call_if_set FLAG_WAYFARER_CINNABAR_GYM_QUIZ_4, CinnabarIsland_Gym_OnLoadOpenDoor4
	call_if_set FLAG_WAYFARER_CINNABAR_GYM_QUIZ_5, CinnabarIsland_Gym_OnLoadOpenDoor5
	call_if_set FLAG_WAYFARER_CINNABAR_GYM_QUIZ_6, CinnabarIsland_Gym_OnLoadOpenDoor6
	end
CinnabarIsland_Gym_OnLoadOpenAllDoors::
	call CinnabarIsland_Gym_EventScript_OpenDoor1
	call CinnabarIsland_Gym_EventScript_OpenDoor2
	call CinnabarIsland_Gym_EventScript_OpenDoor3
	call CinnabarIsland_Gym_EventScript_OpenDoor4
	call CinnabarIsland_Gym_EventScript_OpenDoor5
	call CinnabarIsland_Gym_EventScript_OpenDoor6
	end
CinnabarIsland_Gym_OnLoadOpenDoor1::
	call CinnabarIsland_Gym_EventScript_OpenDoor1
	return
CinnabarIsland_Gym_OnLoadOpenDoor2::
	call CinnabarIsland_Gym_EventScript_OpenDoor2
	return
CinnabarIsland_Gym_OnLoadOpenDoor3::
	call CinnabarIsland_Gym_EventScript_OpenDoor3
	return
CinnabarIsland_Gym_OnLoadOpenDoor4::
	call CinnabarIsland_Gym_EventScript_OpenDoor4
	return
CinnabarIsland_Gym_OnLoadOpenDoor5::
	call CinnabarIsland_Gym_EventScript_OpenDoor5
	return
CinnabarIsland_Gym_OnLoadOpenDoor6::
	call CinnabarIsland_Gym_EventScript_OpenDoor6
	return

CinnabarIsland_Gym_EventScript_Blaine::
	trainerbattle_single TRAINER_LEADER_BLAINE, CinnabarIsland_Gym_Text_BlaineIntro, CinnabarIsland_Gym_Text_BlaineDefeat, CinnabarIsland_Gym_EventScript_DefeatedBlaine, NO_MUSIC
	goto_if_unset FLAG_WAYFARER_CINNABAR_TM38_CLAIMED, CinnabarIsland_Gym_EventScript_GiveTM38
	msgbox CinnabarIsland_Gym_Text_BlainePostBattle
	release
	end
CinnabarIsland_Gym_EventScript_DefeatedBlaine::
	setflag FLAG_WAYFARER_CINNABAR_BLAINE_DEFEATED
	setflag FLAG_DEFEATED_CINNABAR_ISLAND_GYM
	special WayfarerGrantKantoBadge15
	setvar VAR_0x8008, 15
	message CinnabarIsland_Gym_Text_ReceivedVolcanoBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	goto CinnabarIsland_Gym_EventScript_GiveTM38
	end
CinnabarIsland_Gym_EventScript_GiveTM38::
	msgbox CinnabarIsland_Gym_Text_ExplainVolcanoBadge
	checkitemspace ITEM_TM38
	goto_if_eq VAR_RESULT, FALSE, CinnabarIsland_Gym_EventScript_NoRoomForTM38
	giveitem_msg CinnabarIsland_Gym_Text_ReceivedTM38FromBlaine, ITEM_TM38
	setflag FLAG_WAYFARER_CINNABAR_TM38_CLAIMED
	msgbox CinnabarIsland_Gym_Text_FireBlast
	release
	end
CinnabarIsland_Gym_EventScript_NoRoomForTM38::
	msgbox CinnabarIsland_Gym_Text_MakeSpace
	release
	end

CinnabarIsland_Gym_EventScript_Erik::
	trainerbattle_single TRAINER_SUPER_NERD_ERIK, CinnabarIsland_Gym_Text_TrainerIntro, CinnabarIsland_Gym_Text_TrainerDefeat
	msgbox CinnabarIsland_Gym_Text_TrainerPost, MSGBOX_AUTOCLOSE
	end
CinnabarIsland_Gym_EventScript_Avery::
	trainerbattle_single TRAINER_SUPER_NERD_AVERY, CinnabarIsland_Gym_Text_TrainerIntro, CinnabarIsland_Gym_Text_TrainerDefeat, CinnabarIsland_Gym_EventScript_DefeatedAvery
	msgbox CinnabarIsland_Gym_Text_TrainerPost, MSGBOX_AUTOCLOSE
	end
CinnabarIsland_Gym_EventScript_DefeatedAvery::
	call_if_unset FLAG_WAYFARER_CINNABAR_GYM_QUIZ_2, CinnabarIsland_Gym_EventScript_Quiz2Complete
	release
	end
CinnabarIsland_Gym_EventScript_Derek::
	trainerbattle_single TRAINER_SUPER_NERD_DEREK, CinnabarIsland_Gym_Text_TrainerIntro, CinnabarIsland_Gym_Text_TrainerDefeat, CinnabarIsland_Gym_EventScript_DefeatedDerek
	msgbox CinnabarIsland_Gym_Text_TrainerPost, MSGBOX_AUTOCLOSE
	end
CinnabarIsland_Gym_EventScript_DefeatedDerek::
	call_if_unset FLAG_WAYFARER_CINNABAR_GYM_QUIZ_4, CinnabarIsland_Gym_EventScript_Quiz4Complete
	release
	end
CinnabarIsland_Gym_EventScript_Zac::
	trainerbattle_single TRAINER_SUPER_NERD_ZAC, CinnabarIsland_Gym_Text_TrainerIntro, CinnabarIsland_Gym_Text_TrainerDefeat, CinnabarIsland_Gym_EventScript_DefeatedZac
	msgbox CinnabarIsland_Gym_Text_TrainerPost, MSGBOX_AUTOCLOSE
	end
CinnabarIsland_Gym_EventScript_DefeatedZac::
	call_if_unset FLAG_WAYFARER_CINNABAR_GYM_QUIZ_6, CinnabarIsland_Gym_EventScript_Quiz6Complete
	release
	end
CinnabarIsland_Gym_EventScript_Quinn::
	trainerbattle_single TRAINER_BURGLAR_QUINN, CinnabarIsland_Gym_Text_TrainerIntro, CinnabarIsland_Gym_Text_TrainerDefeat, CinnabarIsland_Gym_EventScript_DefeatedQuinn
	msgbox CinnabarIsland_Gym_Text_TrainerPost, MSGBOX_AUTOCLOSE
	end
CinnabarIsland_Gym_EventScript_DefeatedQuinn::
	call_if_unset FLAG_WAYFARER_CINNABAR_GYM_QUIZ_1, CinnabarIsland_Gym_EventScript_Quiz1Complete
	release
	end
CinnabarIsland_Gym_EventScript_Ramon::
	trainerbattle_single TRAINER_BURGLAR_RAMON, CinnabarIsland_Gym_Text_TrainerIntro, CinnabarIsland_Gym_Text_TrainerDefeat, CinnabarIsland_Gym_EventScript_DefeatedRamon
	msgbox CinnabarIsland_Gym_Text_TrainerPost, MSGBOX_AUTOCLOSE
	end
CinnabarIsland_Gym_EventScript_DefeatedRamon::
	call_if_unset FLAG_WAYFARER_CINNABAR_GYM_QUIZ_3, CinnabarIsland_Gym_EventScript_Quiz3Complete
	release
	end
CinnabarIsland_Gym_EventScript_Dusty::
	trainerbattle_single TRAINER_BURGLAR_DUSTY, CinnabarIsland_Gym_Text_TrainerIntro, CinnabarIsland_Gym_Text_TrainerDefeat, CinnabarIsland_Gym_EventScript_DefeatedDusty
	msgbox CinnabarIsland_Gym_Text_TrainerPost, MSGBOX_AUTOCLOSE
	end
CinnabarIsland_Gym_EventScript_DefeatedDusty::
	call_if_unset FLAG_WAYFARER_CINNABAR_GYM_QUIZ_5, CinnabarIsland_Gym_EventScript_Quiz5Complete
	release
	end

CinnabarIsland_Gym_EventScript_GymGuy::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_CINNABAR_BLAINE_DEFEATED, CinnabarIsland_Gym_EventScript_GymGuyWon
	msgbox CinnabarIsland_Gym_Text_GymGuy
	release
	end
CinnabarIsland_Gym_EventScript_GymGuyWon::
	msgbox CinnabarIsland_Gym_Text_GymGuyWon
	release
	end
CinnabarIsland_Gym_EventScript_GymStatue::
	lockall
	goto_if_set FLAG_WAYFARER_CINNABAR_BLAINE_DEFEATED, CinnabarIsland_Gym_EventScript_GymStatueWon
	msgbox CinnabarIsland_Gym_Text_Statue
	releaseall
	end
CinnabarIsland_Gym_EventScript_GymStatueWon::
	msgbox CinnabarIsland_Gym_Text_StatueWon
	releaseall
	end
CinnabarIsland_Gym_EventScript_BlaineFujiPhoto::
	msgbox CinnabarIsland_Gym_Text_Photo, MSGBOX_SIGN
	end

CinnabarIsland_Gym_EventScript_Quz1Left::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz1
CinnabarIsland_Gym_EventScript_Quz1Right::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz1
CinnabarIsland_Gym_EventScript_Quiz1::
	msgbox CinnabarIsland_Gym_Text_Q1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, CinnabarIsland_Gym_EventScript_Quiz1CompleteAndEnd
	goto CinnabarIsland_Gym_EventScript_BattleQuinn
CinnabarIsland_Gym_EventScript_BattleQuinn::
	trainerbattle_no_intro TRAINER_BURGLAR_QUINN, CinnabarIsland_Gym_Text_TrainerDefeat
	call CinnabarIsland_Gym_EventScript_Quiz1Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz1CompleteAndEnd::
	call CinnabarIsland_Gym_EventScript_Quiz1Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz1Complete::
	call CinnabarIsland_Gym_EventScript_Correct
	call CinnabarIsland_Gym_EventScript_OpenDoor1
	setflag FLAG_WAYFARER_CINNABAR_GYM_QUIZ_1
	return

CinnabarIsland_Gym_EventScript_Quiz2Left::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz2
CinnabarIsland_Gym_EventScript_Quiz2Right::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz2
CinnabarIsland_Gym_EventScript_Quiz2::
	msgbox CinnabarIsland_Gym_Text_Q2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinnabarIsland_Gym_EventScript_Quiz2CompleteAndEnd
	goto CinnabarIsland_Gym_EventScript_BattleAvery
CinnabarIsland_Gym_EventScript_BattleAvery::
	trainerbattle_no_intro TRAINER_SUPER_NERD_AVERY, CinnabarIsland_Gym_Text_TrainerDefeat
	call CinnabarIsland_Gym_EventScript_Quiz2Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz2CompleteAndEnd::
	call CinnabarIsland_Gym_EventScript_Quiz2Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz2Complete::
	call CinnabarIsland_Gym_EventScript_Correct
	call CinnabarIsland_Gym_EventScript_OpenDoor2
	setflag FLAG_WAYFARER_CINNABAR_GYM_QUIZ_2
	return

CinnabarIsland_Gym_EventScript_Quiz3Left::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz3
CinnabarIsland_Gym_EventScript_Quiz3Right::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz3
CinnabarIsland_Gym_EventScript_Quiz3::
	msgbox CinnabarIsland_Gym_Text_Q3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinnabarIsland_Gym_EventScript_Quiz3CompleteAndEnd
	goto CinnabarIsland_Gym_EventScript_BattleRamon
CinnabarIsland_Gym_EventScript_BattleRamon::
	trainerbattle_no_intro TRAINER_BURGLAR_RAMON, CinnabarIsland_Gym_Text_TrainerDefeat
	call CinnabarIsland_Gym_EventScript_Quiz3Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz3CompleteAndEnd::
	call CinnabarIsland_Gym_EventScript_Quiz3Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz3Complete::
	call CinnabarIsland_Gym_EventScript_Correct
	call CinnabarIsland_Gym_EventScript_OpenDoor3
	setflag FLAG_WAYFARER_CINNABAR_GYM_QUIZ_3
	return

CinnabarIsland_Gym_EventScript_Quiz4Left::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz4
CinnabarIsland_Gym_EventScript_Quiz4Right::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz4
CinnabarIsland_Gym_EventScript_Quiz4::
	msgbox CinnabarIsland_Gym_Text_Q4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinnabarIsland_Gym_EventScript_Quiz4CompleteAndEnd
	goto CinnabarIsland_Gym_EventScript_BattleDerek
CinnabarIsland_Gym_EventScript_BattleDerek::
	trainerbattle_no_intro TRAINER_SUPER_NERD_DEREK, CinnabarIsland_Gym_Text_TrainerDefeat
	call CinnabarIsland_Gym_EventScript_Quiz4Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz4CompleteAndEnd::
	call CinnabarIsland_Gym_EventScript_Quiz4Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz4Complete::
	call CinnabarIsland_Gym_EventScript_Correct
	call CinnabarIsland_Gym_EventScript_OpenDoor4
	setflag FLAG_WAYFARER_CINNABAR_GYM_QUIZ_4
	return

CinnabarIsland_Gym_EventScript_Quiz5Left::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz5
CinnabarIsland_Gym_EventScript_Quiz5Right::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz5
CinnabarIsland_Gym_EventScript_Quiz5::
	msgbox CinnabarIsland_Gym_Text_Q5, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, CinnabarIsland_Gym_EventScript_Quiz5CompleteAndEnd
	goto CinnabarIsland_Gym_EventScript_BattleDusty
CinnabarIsland_Gym_EventScript_BattleDusty::
	trainerbattle_no_intro TRAINER_BURGLAR_DUSTY, CinnabarIsland_Gym_Text_TrainerDefeat
	call CinnabarIsland_Gym_EventScript_Quiz5Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz5CompleteAndEnd::
	call CinnabarIsland_Gym_EventScript_Quiz5Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz5Complete::
	call CinnabarIsland_Gym_EventScript_Correct
	call CinnabarIsland_Gym_EventScript_OpenDoor5
	setflag FLAG_WAYFARER_CINNABAR_GYM_QUIZ_5
	return

CinnabarIsland_Gym_EventScript_Quiz6Left::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz6
CinnabarIsland_Gym_EventScript_Quiz6Right::
	lockall
	goto CinnabarIsland_Gym_EventScript_Quiz6
CinnabarIsland_Gym_EventScript_Quiz6::
	msgbox CinnabarIsland_Gym_Text_Q6, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinnabarIsland_Gym_EventScript_Quiz6CompleteAndEnd
	goto CinnabarIsland_Gym_EventScript_BattleZac
CinnabarIsland_Gym_EventScript_BattleZac::
	trainerbattle_no_intro TRAINER_SUPER_NERD_ZAC, CinnabarIsland_Gym_Text_TrainerDefeat
	call CinnabarIsland_Gym_EventScript_Quiz6Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz6CompleteAndEnd::
	call CinnabarIsland_Gym_EventScript_Quiz6Complete
	releaseall
	end
CinnabarIsland_Gym_EventScript_Quiz6Complete::
	call CinnabarIsland_Gym_EventScript_Correct
	call CinnabarIsland_Gym_EventScript_OpenDoor6
	setflag FLAG_WAYFARER_CINNABAR_GYM_QUIZ_6
	return

CinnabarIsland_Gym_EventScript_Correct::
	playfanfare MUS_LEVEL_UP
	waitfanfare
	msgbox CinnabarIsland_Gym_Text_Correct
	return

@ Authoritative FRLG door tile changes.
CinnabarIsland_Gym_EventScript_OpenDoor1::
	setmetatile 26, 8, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile 27, 8, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile 26, 9, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile 27, 9, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile 26, 10, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 27, 10, METATILE_CinnabarGym_Floor, 0
	setmetatile 28, 10, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return
CinnabarIsland_Gym_EventScript_OpenDoor2::
	setmetatile 17, 8, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile 18, 8, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile 17, 9, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile 18, 9, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile 17, 10, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 18, 10, METATILE_CinnabarGym_Floor, 0
	setmetatile 19, 10, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return
CinnabarIsland_Gym_EventScript_OpenDoor3::
	setmetatile 17, 15, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile 18, 15, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile 17, 16, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile 18, 16, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile 17, 17, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 18, 17, METATILE_CinnabarGym_Floor, 0
	setmetatile 19, 17, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return
CinnabarIsland_Gym_EventScript_OpenDoor4::
	setmetatile 11, 21, METATILE_CinnabarGym_Wall_RetractedBarrier, 1
	setmetatile 11, 22, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 11, 23, METATILE_CinnabarGym_Floor, 0
	return
CinnabarIsland_Gym_EventScript_OpenDoor5::
	setmetatile 5, 16, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile 6, 16, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile 5, 17, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile 6, 17, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile 5, 18, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 6, 18, METATILE_CinnabarGym_Floor, 0
	setmetatile 7, 18, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return
CinnabarIsland_Gym_EventScript_OpenDoor6::
	setmetatile 5, 8, METATILE_CinnabarGym_Floor_WallLeftCorner, 0
	setmetatile 6, 8, METATILE_CinnabarGym_Floor_WallRightCorner, 0
	setmetatile 5, 9, METATILE_CinnabarGym_Floor_WallLeftEdge, 0
	setmetatile 6, 9, METATILE_CinnabarGym_Floor_WallRightEdge, 0
	setmetatile 5, 10, METATILE_CinnabarGym_Floor_ShadeFull, 0
	setmetatile 6, 10, METATILE_CinnabarGym_Floor, 0
	setmetatile 7, 10, METATILE_CinnabarGym_Floor_ShadeDiagonal, 0
	return

CinnabarIsland_Gym_Text_BlaineIntro::
	.string "I am BLAINE, the red-hot LEADER\n"
	.string "of CINNABAR GYM!$"
CinnabarIsland_Gym_Text_BlaineDefeat::
	.string "I have burned down to nothing!\p"
	.string "You have earned the VOLCANO BADGE.$"
CinnabarIsland_Gym_Text_BlainePostBattle::
	.string "TM38 contains FIRE BLAST.$"
CinnabarIsland_Gym_Text_ReceivedVolcanoBadge::
	.string "{PLAYER} received the VOLCANO BADGE!$"
CinnabarIsland_Gym_Text_ExplainVolcanoBadge::
	.string "Take this, too!$"
CinnabarIsland_Gym_Text_ReceivedTM38FromBlaine::
	.string "{PLAYER} received TM38\n"
	.string "from BLAINE.$"
CinnabarIsland_Gym_Text_FireBlast::
	.string "FIRE BLAST is a powerful fire\n"
	.string "technique.$"
CinnabarIsland_Gym_Text_MakeSpace::
	.string "Make room for TM38, then return.$"
CinnabarIsland_Gym_Text_TrainerIntro::
	.string "My FIRE POKéMON will burn you!$"
CinnabarIsland_Gym_Text_TrainerDefeat::
	.string "Too hot to handle!$"
CinnabarIsland_Gym_Text_TrainerPost::
	.string "The quiz doors also open when\n"
	.string "their TRAINER is beaten.$"
CinnabarIsland_Gym_Text_GymGuy::
	.string "Water attacks can douse BLAINE's\n"
	.string "fire!$"
CinnabarIsland_Gym_Text_GymGuyWon::
	.string "You beat that firebrand!$"
CinnabarIsland_Gym_Text_Statue::
	.string "CINNABAR POKéMON GYM\n"
	.string "LEADER: BLAINE$"
CinnabarIsland_Gym_Text_StatueWon::
	.string "CINNABAR POKéMON GYM\n"
	.string "WINNER: {PLAYER}$"
CinnabarIsland_Gym_Text_Photo::
	.string "It's a photo of BLAINE and\n"
	.string "MR. FUJI.$"
CinnabarIsland_Gym_Text_Correct::
	.string "You're absolutely correct!$"
CinnabarIsland_Gym_Text_Q1::
	.string "CATERPIE evolves into METAPOD?$"
CinnabarIsland_Gym_Text_Q2::
	.string "There are nine certified POKéMON\n"
	.string "LEAGUE BADGES?$"
CinnabarIsland_Gym_Text_Q3::
	.string "POLIWAG evolves three times?$"
CinnabarIsland_Gym_Text_Q4::
	.string "Are electric moves effective\n"
	.string "against GROUND-type POKéMON?$"
CinnabarIsland_Gym_Text_Q5::
	.string "POKéMON of the same kind and level\n"
	.string "are not identical?$"
CinnabarIsland_Gym_Text_Q6::
	.string "TM28 contains TOMBSTONY?$"

@ Lab entrance changes a pending fossil only after the authored leave/return.
CinnabarIsland_PokemonLab_Entrance_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CinnabarIsland_PokemonLab_Entrance_OnTransition
	.byte 0
CinnabarIsland_PokemonLab_Entrance_OnTransition::
	call_if_eq VAR_WAYFARER_CINNABAR_LAB_FOSSIL_STATE, 1, CinnabarIsland_PokemonLab_Entrance_EventScript_ReadyFossil
	end
CinnabarIsland_PokemonLab_Entrance_EventScript_ReadyFossil::
	setvar VAR_WAYFARER_CINNABAR_LAB_FOSSIL_STATE, 2
	return

@ The FRLG Lab's authored door coordinates lack warp metatile behaviors.
@ Wayfarer map events trigger these transitions at the same coordinates.
WayfarerCinnabar_Lab_WarpLounge::
	warp MAP_CINNABAR_ISLAND_POKEMON_LAB_LOUNGE, 7, 8
	waitstate
	end
WayfarerCinnabar_Lab_WarpResearch::
	warp MAP_CINNABAR_ISLAND_POKEMON_LAB_RESEARCH_ROOM, 7, 8
	waitstate
	end
WayfarerCinnabar_Lab_WarpExperiment::
	warp MAP_CINNABAR_ISLAND_POKEMON_LAB_EXPERIMENT_ROOM, 7, 8
	waitstate
	end
WayfarerCinnabar_Lab_WarpFromLounge::
	warp MAP_CINNABAR_ISLAND_POKEMON_LAB_ENTRANCE, 13, 6
	waitstate
	end
WayfarerCinnabar_Lab_WarpFromResearch::
	warp MAP_CINNABAR_ISLAND_POKEMON_LAB_ENTRANCE, 19, 6
	waitstate
	end
WayfarerCinnabar_Lab_WarpFromExperiment::
	warp MAP_CINNABAR_ISLAND_POKEMON_LAB_ENTRANCE, 25, 6
	waitstate
	end
CinnabarIsland_PokemonLab_Entrance_EventScript_Scientist::
	msgbox CinnabarIsland_PokemonLab_Entrance_Text_Scientist, MSGBOX_NPC
	end
CinnabarIsland_PokemonLab_Entrance_EventScript_DrFujiPhoto::
	msgbox CinnabarIsland_PokemonLab_Entrance_Text_Photo, MSGBOX_SIGN
	end
CinnabarIsland_PokemonLab_Entrance_EventScript_MeetingRoomSign::
	msgbox CinnabarIsland_PokemonLab_Entrance_Text_Meeting, MSGBOX_SIGN
	end
CinnabarIsland_PokemonLab_Entrance_EventScript_RAndDRoomSign::
	msgbox CinnabarIsland_PokemonLab_Entrance_Text_RD, MSGBOX_SIGN
	end
CinnabarIsland_PokemonLab_Entrance_EventScript_TestingRoomSign::
	msgbox CinnabarIsland_PokemonLab_Entrance_Text_Testing, MSGBOX_SIGN
	end
CinnabarIsland_PokemonLab_Entrance_Text_Scientist::
	.string "We study POKéMON extensively\n"
	.string "every day.$"
CinnabarIsland_PokemonLab_Entrance_Text_Photo::
	.string "A photo of the LAB's founder…\n"
	.string "DR. FUJI?!$"
CinnabarIsland_PokemonLab_Entrance_Text_Meeting::
	.string "POKéMON LAB\nMeeting Room$"
CinnabarIsland_PokemonLab_Entrance_Text_RD::
	.string "POKéMON LAB\nR & D Room$"
CinnabarIsland_PokemonLab_Entrance_Text_Testing::
	.string "POKéMON LAB\nTesting Room$"

CinnabarIsland_PokemonLab_Lounge_Frlg_MapScripts::
	.byte 0
CinnabarIsland_PokemonLab_Lounge_EventScript_Scientist::
	msgbox CinnabarIsland_PokemonLab_Lounge_Text_Scientist, MSGBOX_NPC
	end
CinnabarIsland_PokemonLab_Lounge_EventScript_Clifton::
	setvar VAR_0x8008, INGAME_TRADE_ELECTRODE
	goto CinnabarIsland_PokemonLab_Lounge_EventScript_TradeElectrode
CinnabarIsland_PokemonLab_Lounge_EventScript_TradeElectrode::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_CINNABAR_LAB_TRADE_ELECTRODE, CinnabarIsland_PokemonLab_Lounge_EventScript_TradeDone
	call EventScript_GetInGameTradeSpeciesInfo
	msgbox CinnabarIsland_PokemonLab_Lounge_Text_TradeOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinnabarIsland_PokemonLab_Lounge_EventScript_TradeNo
	call EventScript_ChooseMonForInGameTrade
	goto_if_ge VAR_0x8004, PARTY_SIZE, CinnabarIsland_PokemonLab_Lounge_EventScript_TradeNo
	call EventScript_GetInGameTradeSpecies
	goto_if_ne VAR_RESULT, VAR_0x8009, CinnabarIsland_PokemonLab_Lounge_EventScript_TradeWrong
	call EventScript_DoInGameTrade
	setflag FLAG_WAYFARER_CINNABAR_LAB_TRADE_ELECTRODE
	msgbox CinnabarIsland_PokemonLab_Lounge_Text_TradeThanks
	release
	end
CinnabarIsland_PokemonLab_Lounge_EventScript_Norma::
	setvar VAR_0x8008, INGAME_TRADE_TANGELA
	goto CinnabarIsland_PokemonLab_Lounge_EventScript_TradeTangela
CinnabarIsland_PokemonLab_Lounge_EventScript_TradeTangela::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_CINNABAR_LAB_TRADE_TANGELA, CinnabarIsland_PokemonLab_Lounge_EventScript_TradeDone
	call EventScript_GetInGameTradeSpeciesInfo
	msgbox CinnabarIsland_PokemonLab_Lounge_Text_TradeOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinnabarIsland_PokemonLab_Lounge_EventScript_TradeNo
	call EventScript_ChooseMonForInGameTrade
	goto_if_ge VAR_0x8004, PARTY_SIZE, CinnabarIsland_PokemonLab_Lounge_EventScript_TradeNo
	call EventScript_GetInGameTradeSpecies
	goto_if_ne VAR_RESULT, VAR_0x8009, CinnabarIsland_PokemonLab_Lounge_EventScript_TradeWrong
	call EventScript_DoInGameTrade
	setflag FLAG_WAYFARER_CINNABAR_LAB_TRADE_TANGELA
	msgbox CinnabarIsland_PokemonLab_Lounge_Text_TradeThanks
	release
	end
CinnabarIsland_PokemonLab_Lounge_EventScript_TradeDone::
	msgbox CinnabarIsland_PokemonLab_Lounge_Text_TradeDone
	release
	end
CinnabarIsland_PokemonLab_Lounge_EventScript_TradeNo::
	msgbox CinnabarIsland_PokemonLab_Lounge_Text_TradeNo
	release
	end
CinnabarIsland_PokemonLab_Lounge_EventScript_TradeWrong::
	bufferspeciesname STR_VAR_1, VAR_0x8009
	msgbox CinnabarIsland_PokemonLab_Lounge_Text_TradeWrong
	release
	end
CinnabarIsland_PokemonLab_Lounge_Text_Scientist::
	.string "I found a remarkable fossil\n"
	.string "inside MT. MOON.$"
CinnabarIsland_PokemonLab_Lounge_Text_TradeOffer::
	.string "Do you have the POKéMON I need?\n"
	.string "Would you trade?$"
CinnabarIsland_PokemonLab_Lounge_Text_TradeThanks::
	.string "Thanks! Take good care of it.$"
CinnabarIsland_PokemonLab_Lounge_Text_TradeDone::
	.string "How is my old POKéMON doing?$"
CinnabarIsland_PokemonLab_Lounge_Text_TradeNo::
	.string "Maybe another time.$"
CinnabarIsland_PokemonLab_Lounge_Text_TradeWrong::
	.string "That isn't the {STR_VAR_1} I need.$"

CinnabarIsland_PokemonLab_ResearchRoom_Frlg_MapScripts::
	.byte 0
CinnabarIsland_PokemonLab_ResearchRoom_EventScript_MetronomeTutor::
	lock
	faceplayer
	move_tutor MOVE_METRONOME, CinnabarIsland_PokemonLab_ResearchRoom_Text_Teach, CinnabarIsland_PokemonLab_ResearchRoom_Text_Which, CinnabarIsland_PokemonLab_ResearchRoom_EventScript_MetronomeNo, CinnabarIsland_PokemonLab_ResearchRoom_EventScript_MetronomeDone, FLAG_WAYFARER_CINNABAR_LAB_METRONOME
	release
	end
CinnabarIsland_PokemonLab_ResearchRoom_EventScript_MetronomeNo::
	msgbox CinnabarIsland_PokemonLab_ResearchRoom_Text_No
	release
	end
CinnabarIsland_PokemonLab_ResearchRoom_EventScript_MetronomeDone::
	msgbox CinnabarIsland_PokemonLab_ResearchRoom_Text_Done
	release
	end
CinnabarIsland_PokemonLab_ResearchRoom_EventScript_Scientist::
	msgbox CinnabarIsland_PokemonLab_ResearchRoom_Text_Scientist, MSGBOX_NPC
	end
CinnabarIsland_PokemonLab_ResearchRoom_EventScript_Computer::
	msgbox CinnabarIsland_PokemonLab_ResearchRoom_Text_Computer, MSGBOX_SIGN
	end
CinnabarIsland_PokemonLab_ResearchRoom_EventScript_AmberPipe::
	msgbox CinnabarIsland_PokemonLab_ResearchRoom_Text_Amber, MSGBOX_SIGN
	end
CinnabarIsland_PokemonLab_ResearchRoom_Text_Teach::
	.string "I'll teach a nifty move called\nMETRONOME. Interested?$"
CinnabarIsland_PokemonLab_ResearchRoom_Text_Which::
	.string "Which POKéMON should learn it?$"
CinnabarIsland_PokemonLab_ResearchRoom_Text_No::
	.string "It is a lot of fun!$"
CinnabarIsland_PokemonLab_ResearchRoom_Text_Done::
	.string "Listen: tch-tch-tch!$"
CinnabarIsland_PokemonLab_ResearchRoom_Text_Scientist::
	.string "EEVEE can evolve into one of\nthree kinds of POKéMON.$"
CinnabarIsland_PokemonLab_ResearchRoom_Text_Computer::
	.string "There are three legendary birds:\nARTICUNO, ZAPDOS, and MOLTRES.$"
CinnabarIsland_PokemonLab_ResearchRoom_Text_Amber::
	.string "An amber pipe!$"

@ The third FRLG trade is kept in the testing room beside the fossil doctor.
CinnabarIsland_PokemonLab_ExperimentRoom_Frlg_MapScripts::
	.byte 0
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_Garett::
	lock
	faceplayer
	goto_if_set FLAG_WAYFARER_CINNABAR_LAB_TRADE_SEEL, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_TradeDone
	setvar VAR_0x8008, INGAME_TRADE_SEEL
	call EventScript_GetInGameTradeSpeciesInfo
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_TradeNo
	call EventScript_ChooseMonForInGameTrade
	goto_if_ge VAR_0x8004, PARTY_SIZE, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_TradeNo
	call EventScript_GetInGameTradeSpecies
	goto_if_ne VAR_RESULT, VAR_0x8009, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_TradeWrong
	call EventScript_DoInGameTrade
	setflag FLAG_WAYFARER_CINNABAR_LAB_TRADE_SEEL
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeThanks
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_TradeDone::
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeDone
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_TradeNo::
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeNo
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_TradeWrong::
	bufferspeciesname STR_VAR_1, VAR_0x8009
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeWrong
	release
	end

CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_FossilScientist::
	lock
	faceplayer
	goto_if_eq VAR_WAYFARER_CINNABAR_LAB_FOSSIL_STATE, 2, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DeliverFossil
	goto_if_eq VAR_WAYFARER_CINNABAR_LAB_FOSSIL_STATE, 1, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_FossilWaiting
	checkitem ITEM_HELIX_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_CheckDome
	goto_if_unset FLAG_WAYFARER_CINNABAR_LAB_REVIVED_HELIX, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferHelix
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_CheckDome::
	checkitem ITEM_DOME_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_CheckAmber
	goto_if_unset FLAG_WAYFARER_CINNABAR_LAB_REVIVED_DOME, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferDome
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_CheckAmber::
	checkitem ITEM_OLD_AMBER
	goto_if_eq VAR_RESULT, FALSE, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_NoFossil
	goto_if_unset FLAG_WAYFARER_CINNABAR_LAB_REVIVED_AMBER, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferAmber
	goto CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_NoFossil
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_NoFossil::
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_NoFossil
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferHelix::
	setvar VAR_0x8004, ITEM_HELIX_FOSSIL
	setvar VAR_WAYFARER_CINNABAR_LAB_FOSSIL_KIND, 1
	goto CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferFossil
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferDome::
	setvar VAR_0x8004, ITEM_DOME_FOSSIL
	setvar VAR_WAYFARER_CINNABAR_LAB_FOSSIL_KIND, 2
	goto CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferFossil
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferAmber::
	setvar VAR_0x8004, ITEM_OLD_AMBER
	setvar VAR_WAYFARER_CINNABAR_LAB_FOSSIL_KIND, 3
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_OfferFossil::
	bufferitemname STR_VAR_1, VAR_0x8004
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_OfferFossil, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_FossilNo
	removeitem VAR_0x8004
	setvar VAR_WAYFARER_CINNABAR_LAB_FOSSIL_STATE, 1
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilWait
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_FossilNo::
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilNo
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_FossilWaiting::
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilWait
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DeliverFossil::
	goto_if_eq VAR_WAYFARER_CINNABAR_LAB_FOSSIL_KIND, 1, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveOmanyte
	goto_if_eq VAR_WAYFARER_CINNABAR_LAB_FOSSIL_KIND, 2, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveKabuto
	goto CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveAerodactyl
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveOmanyte::
	setvar VAR_TEMP_1, SPECIES_OMANYTE
	goto CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveFossilMon
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveKabuto::
	setvar VAR_TEMP_1, SPECIES_KABUTO
	goto CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveFossilMon
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveAerodactyl::
	setvar VAR_TEMP_1, SPECIES_AERODACTYL
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveFossilMon::
	bufferspeciesname STR_VAR_1, VAR_TEMP_1
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilReady
	givemon VAR_TEMP_1, 5
	goto_if_eq VAR_RESULT, 2, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_PartyFull
	goto_if_eq VAR_WAYFARER_CINNABAR_LAB_FOSSIL_KIND, 1, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_MarkHelix
	goto_if_eq VAR_WAYFARER_CINNABAR_LAB_FOSSIL_KIND, 2, CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_MarkDome
	setflag FLAG_WAYFARER_CINNABAR_LAB_REVIVED_AMBER
	goto CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ClearFossilPending
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_MarkHelix::
	setflag FLAG_WAYFARER_CINNABAR_LAB_REVIVED_HELIX
	goto CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ClearFossilPending
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_MarkDome::
	setflag FLAG_WAYFARER_CINNABAR_LAB_REVIVED_DOME
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ClearFossilPending::
	setvar VAR_WAYFARER_CINNABAR_LAB_FOSSIL_STATE, 0
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilReceived
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_PartyFull::
	msgbox CinnabarIsland_PokemonLab_ExperimentRoom_Text_PartyFull
	release
	end
CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeOffer::
	.string "Would you trade the POKéMON I need?$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeThanks::
	.string "Thanks for the trade!$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeDone::
	.string "How is my old POKéMON doing?$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeNo::
	.string "Maybe another time.$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_TradeWrong::
	.string "That isn't the {STR_VAR_1} I need.$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_NoFossil::
	.string "Have you a fossil for me?$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_OfferFossil::
	.string "That {STR_VAR_1} can be brought\nback to life. Shall I begin?$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilNo::
	.string "You come again.$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilWait::
	.string "It takes time. Go for a walk\nand return later.$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilReady::
	.string "Your fossil is back to life!\nIt is {STR_VAR_1}.$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_FossilReceived::
	.string "{PLAYER} received {STR_VAR_1}\nfrom the doctor.$"
CinnabarIsland_PokemonLab_ExperimentRoom_Text_PartyFull::
	.string "Your party is full. Return when\nyou have room for {STR_VAR_1}.$"

@ Center and Mart services are active before Blaine.  The selected map is
@ responsible for any supported 2F facility objects; this script adds no Bill.
CinnabarIsland_PokemonCenter_1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CinnabarIsland_PokemonCenter_1F_OnTransition
	.byte 0
CinnabarIsland_PokemonCenter_1F_OnTransition::
	setrespawn HEAL_LOCATION_CINNABAR_ISLAND
	end
CinnabarIsland_PokemonCenter_1F_EventScript_Nurse::
	lock
	faceplayer
	special HealPlayerParty
	msgbox CinnabarIsland_PokemonCenter_1F_Text_Nurse
	release
	end
CinnabarIsland_PokemonCenter_1F_EventScript_Gentleman::
	msgbox CinnabarIsland_PokemonCenter_1F_Text_Gentleman, MSGBOX_NPC
	end
CinnabarIsland_PokemonCenter_1F_EventScript_CooltrainerF::
	msgbox CinnabarIsland_PokemonCenter_1F_Text_GymLocked, MSGBOX_NPC
	end
CinnabarIsland_PokemonCenter_1F_EventScript_Youngster::
	msgbox CinnabarIsland_PokemonCenter_1F_Text_Youngster, MSGBOX_NPC
	end
CinnabarIsland_PokemonCenter_1F_EventScript_PokemonJournalMrFuji::
	msgbox CinnabarIsland_PokemonCenter_1F_Text_Journal, MSGBOX_SIGN
	end
CinnabarIsland_PokemonCenter_1F_Text_Nurse::
	.string "Your POKéMON have been restored\nto full health.$"
CinnabarIsland_PokemonCenter_1F_Text_Gentleman::
	.string "The upstairs facilities are\navailable when supported.$"
CinnabarIsland_PokemonCenter_1F_Text_GymLocked::
	.string "The GYM needs a key from the\nburned-out MANSION.$"
CinnabarIsland_PokemonCenter_1F_Text_Youngster::
	.string "POKéMON can still learn moves\nafter canceling evolution.$"
CinnabarIsland_PokemonCenter_1F_Text_Journal::
	.string "The POKéMON JOURNAL is open to\na feature on MR. FUJI.$"

CinnabarIsland_PokemonCenter_2F_Frlg_MapScripts::
	.byte 0
CinnabarIsland_PokemonCenter_2F_EventScript_Colosseum::
	msgbox CinnabarIsland_PokemonCenter_2F_Text_Unavailable, MSGBOX_DEFAULT
	end
CinnabarIsland_PokemonCenter_2F_EventScript_TradeCenter::
	msgbox CinnabarIsland_PokemonCenter_2F_Text_Unavailable, MSGBOX_DEFAULT
	end
CinnabarIsland_PokemonCenter_2F_EventScript_RecordCorner::
	msgbox CinnabarIsland_PokemonCenter_2F_Text_Unavailable, MSGBOX_DEFAULT
	end
CinnabarIsland_PokemonCenter_2F_EventScript_UnionRoomAttendant::
	msgbox CinnabarIsland_PokemonCenter_2F_Text_Unavailable, MSGBOX_DEFAULT
	end
CinnabarIsland_PokemonCenter_2F_EventScript_WirelessClubAttendant::
	msgbox CinnabarIsland_PokemonCenter_2F_Text_Unavailable, MSGBOX_DEFAULT
	end
CinnabarIsland_PokemonCenter_2F_EventScript_DirectCornerAttendant::
	msgbox CinnabarIsland_PokemonCenter_2F_Text_Unavailable, MSGBOX_DEFAULT
	end
CinnabarIsland_PokemonCenter_2F_EventScript_MysteryGiftMan::
	msgbox CinnabarIsland_PokemonCenter_2F_Text_Unavailable, MSGBOX_DEFAULT
	end
CinnabarIsland_PokemonCenter_2F_Text_Unavailable::
	.string "This facility is not available\nright now.$"

CinnabarIsland_Mart_Frlg_MapScripts::
	.byte 0
CinnabarIsland_Mart_EventScript_Clerk::
	lock
	faceplayer
	setvar VAR_0x8004, MART_PROFILE_CINNABAR
	special WayfarerOpenMartProfile
	waitstate
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end
CinnabarIsland_Mart_EventScript_Woman::
	msgbox CinnabarIsland_Mart_Text_Woman, MSGBOX_NPC
	end
CinnabarIsland_Mart_EventScript_Scientist::
	msgbox CinnabarIsland_Mart_Text_Scientist, MSGBOX_NPC
	end
CinnabarIsland_Mart_Text_Woman::
	.string "I like items that boost a\nPOKéMON's strength.$"
CinnabarIsland_Mart_Text_Scientist::
	.string "It never hurts to carry extra\nitems.$"
