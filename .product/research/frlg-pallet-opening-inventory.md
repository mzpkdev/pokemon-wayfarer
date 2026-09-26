# FRLG Pallet opening inventory

Authoritative oracle for porting the FireRed/LeafGreen Pallet Town opening onto
the Wayfarer HNS Pallet maps and for the verbatim-dialogue e2e journey. It covers
a new game from the bedroom through Oak's Pokédex and Poké Ball handoff after the
Parcel delivery, plus the ambient content on that path.

Inventory size: 76 event scripts, 126 distinct FRLG messages,
71 port gaps (section 13).

## 1. Sources and conventions

Every FRLG message below is generated from the repository sources, not typed by
hand:

- Map scripts: `game/data/maps/{PalletTown_Frlg, PalletTown_PlayersHouse_1F_Frlg,
  PalletTown_PlayersHouse_2F_Frlg, PalletTown_RivalsHouse_Frlg,
  PalletTown_ProfessorOaksLab_Frlg, Route1_Frlg, ViridianCity_Frlg,
  ViridianCity_Mart_Frlg}/scripts.inc` and each `map.json`.
- Shared script text: `game/data/text/pc_transfer.inc` (nickname prompt),
  `game/data/text/obtain_item.inc`, `game/data/text/mart_clerk.inc`,
  `game/data/event_scripts.s` (PC boot, heal, bag-full, naming helpers).
- Battle tutorial: `game/src/battle_controller_oak_old_man.c` (Oak's in-battle
  lines), `game/src/strings.c` (Oak's party-menu lines),
  `game/src/battle_controllers.c`, `game/src/party_menu.c`,
  `game/src/battle_setup.c`, `game/src/battle_main.c`.
- Trainer parties: `game/src/data/trainers_frlg.party`.
- Constants: `game/include/constants/flags_frlg.h`, `vars_frlg.h`, `vars.h`,
  `map_event_ids.h`, `battle.h`.

Conventions:

- **Verbatim text** is the concatenation of the label's `.string` lines (or the C
  literal) with every control code kept: `\n` (next line), `\l` (scroll line),
  `\p` (new page), `{PLAYER}`, `{RIVAL}`, `{STR_VAR_n}`, `{B_PLAYER_NAME}`,
  `{PAUSE_UNTIL_PRESS}`. The only change is that the trailing `$` terminator is
  dropped. A straight `'` in source renders as `’` (charmap maps `'` to `B4`).
- **Speaker** comes from the name prefix in the text when there is one, otherwise
  from the talking object and its `textcolor` (male = blue, female = red,
  neutral = system/narration).
- **Coordinates** are map-local tiles from the FRLG `map.json`, written `(x,y)`.
  Movement descriptions give the net path.
- `msgbox ..., MSGBOX_NPC` = lock + face player + message + release.
  `MSGBOX_SIGN` = lockall + message. `MSGBOX_YESNO` = message + YES/NO, result in
  `VAR_RESULT`.
- **Repository modification:** `ViridianCity_Frlg` is not vanilla. Commit
  `14f5e09e5c` ("open FRLG regional traversal") removed the old man's road block.
  Section 10 records both the current repo state and the vanilla original, which
  the git diff preserves.

## 2. State model

| Symbol | ID (FRLG build) | Role in this opening |
| --- | --- | --- |
| `VAR_MAP_SCENE_PALLET_TOWN_OAK` | `0x4050` | 0 = Oak trigger armed at Pallet north exit; 1 = escort done (set by the town trigger). |
| `VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB` | `0x4055` | 0 pre-escort; 1 escort arriving (lab `OnFrame` arrival scene); 2 choose starter; 3 both starters taken (exit triggers start the battle); 4 battle done; 5 Parcel held (set by the Mart); 6 Pokédex and Balls handed off. 7-9 are post-game National Dex states. |
| `VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY` | `0x4070` | 0 not shown; 1 Trainer Tips seen (set by the sign, the lady's copy, or starter receipt if START was already opened); 2 done (promoted from 1 on the next Pallet transition). |
| `VAR_MAP_SCENE_PALLET_TOWN_PLAYERS_HOUSE_2F` | `0x4056` | 0 first bedroom arrival (face north, `setrespawn HEAL_LOCATION_PALLET_TOWN`); 1 afterwards. |
| `VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE` | `0x4058` | 0 pre-handoff; 1 set at the Pokédex handoff (Daisy gives the Town Map); 2 map received. |
| `VAR_MAP_SCENE_VIRIDIAN_CITY_MART` | `0x4057` | 0 Parcel scene armed on Mart entry; 1 Parcel held ("say hi" clerk, and Oak's delivery gate is `>= 1`); 2 set at the handoff (ordinary shop). |
| `VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN` | `0x4051` | 0 grumpy/blocking old man; 1 set at the handoff (catching tutorial armed); 2 tutorial done. |
| `VAR_MAP_SCENE_VIRIDIAN_CITY_GYM_DOOR` | `0x405A` | 0 locked gym door trigger. |
| `VAR_MAP_SCENE_ROUTE22` | `0x4054` | Set to 1 at the handoff (arms the Route 22 rival). |
| `VAR_MAP_SCENE_POKEMON_CENTER_TEALA` | `0x407C` | Set to 1 at the handoff (Pokémon Center 2F attendant). |
| `VAR_STARTER_MON` | `0x4023` | Copied from the chosen slot: 0 Bulbasaur, 1 Squirtle, 2 Charmander. It selects the rival's battle roster. |
| `VAR_TEMP_1..4` (lab) | temp | `PLAYER_STARTER_NUM`, `PLAYER_STARTER_SPECIES`, `RIVAL_STARTER_SPECIES`, `RIVAL_STARTER_ID`, set by the ball scripts. `VAR_TEMP_2` is reused as the exit lane (1/2/3 = x 5/6/7) by the battle triggers. |
| `VAR_TEMP_1` (Pallet) | temp | Oak trigger side: 0 left (x 12), 1 right (x 13). |
| `VAR_TEMP_2` (Pallet, `SIGN_LADY_READY`) | temp | TRUE when the lady waits at the route entrance to show the sign. |
| `VAR_TEMP_1` (rival house, `RECEIVED_TOWN_MAP`) | temp | TRUE on entry when the house var is 2 or more. |
| `FLAG_HIDE_OAK_IN_PALLET_TOWN` | `0x02C` | Set at new game; set again after the escort. |
| `FLAG_HIDE_OAK_IN_HIS_LAB` | `0x02B` | Set at new game; cleared by the town trigger and the lab arrival. |
| `FLAG_HIDE_RIVAL_IN_LAB` | `0x02D` | Clear at new game (rival visible in the lab from the start). |
| `FLAG_HIDE_{BULBASAUR,SQUIRTLE,CHARMANDER}_BALL` | `0x028-0x02A` | Clear at new game; set by `removeobject` when a ball is taken. |
| `FLAG_HIDE_POKEDEX` | `0x03A` | Clear at new game (two Pokédex units on the desk); set by `removeobject` at the handoff. |
| `FLAG_HIDE_TOWN_MAP` | `0x039` | Clear at new game (map displayed in the rival's house); set when Daisy gives it (post-handoff). |
| `FLAG_SYS_POKEMON_GET` | `SYS+0x28` | Set on starter receipt. |
| `FLAG_SYS_POKEDEX_GET` | `SYS+0x29` | Set at the handoff (also `special SetUnlockedPokedexFlags`). |
| `FLAG_PALLET_LADY_NOT_BLOCKING_SIGN` | `0x291` | Set on starter receipt; moves the sign lady off the Trainer Tips sign. |
| `FLAG_OPENED_START_MENU` | `SYS+0x3E` | Set by `field_control_avatar.c` whenever START is pressed, and by the lady's sign copy. |
| `FLAG_BEAT_RIVAL_IN_OAKS_LAB` | `0x258` | Set after the lab battle on **either** outcome; switches Mom to healing. |
| `FLAG_GOT_POTION_ON_ROUTE_1` | `0x230` | Route 1 clerk sample given. |
| `FLAG_VISITED_OAKS_LAB` | `0x2CF` | Set on every lab transition. |
| `FLAG_DONT_TRANSITION_MUSIC` | special | Set by the escort, cleared in the lab arrival scene. |
| `FLAG_WORLD_MAP_PALLET_TOWN`, `FLAG_WORLD_MAP_VIRIDIAN_CITY` | `SYS+0x90/0x91` | Set on map transition. |
| `FLAG_TEMP_2` | temp | Pallet: the sign lady has stepped aside. Lab: `SHOWED_OAK_COMPLETE_DEX` (post-game). |

Local IDs: Pallet `LOCALID_PALLET_SIGN_LADY` 1, `LOCALID_PALLET_FAT_MAN` 2,
`LOCALID_PALLET_PROF_OAK` 3. Lab `LOCALID_OAKS_LAB_PROF_OAK` 4,
`LOCALID_BULBASAUR_BALL` 5, `LOCALID_SQUIRTLE_BALL` 6, `LOCALID_CHARMANDER_BALL`
7, `LOCALID_OAKS_LAB_RIVAL` 8, `LOCALID_POKEDEX_1` 9, `LOCALID_POKEDEX_2` 10.
Rival's house `LOCALID_DAISY` 1, `LOCALID_TOWN_MAP` 2. Mom
`LOCALID_PLAYERS_HOUSE_1F_MOM` 1. Viridian `LOCALID_TUTORIAL_MAN` 4,
`LOCALID_VIRIDIAN_WOMAN` 5, `LOCALID_VIRIDIAN_OLD_MAN` (gym old man). Mart
`LOCALID_VIRIDIAN_MART_CLERK` 1.

## 3. Critical path at a glance

| # | Beat | Map, trigger | Gate, then state written |
| --- | --- | --- | --- |
| 1 | Wake in bedroom | 2F, new-game warp (6,6) | 2F var 0, then 1; respawn is Pallet |
| 2 | Mom's send-off | 1F, Mom (8,4) | `FLAG_BEAT_RIVAL_IN_OAKS_LAB` clear; gender branch |
| 3 | Oak stops the player | Pallet coord (12,1)/(13,1) | Oak var 0, then Oak 1, lab 1; warp to lab (6,12) |
| 4 | Lab arrival exchange | Lab `OnFrame` | lab 1, then 2 |
| 5 | Choose starter, nickname, rival picks | Ball (8/9/10,4) | lab 2, then 3; mon given; lady flag |
| 6 | Rival battle on the way out | Lab coord (5..7,8) | lab 3, then 4; `FLAG_BEAT_RIVAL_IN_OAKS_LAB` |
| 7 | Route 1 sample | Route 1 clerk (6,28) | `FLAG_GOT_POTION_ON_ROUTE_1` |
| 8 | Parcel | Mart `OnFrame` | Mart 0, then 1; lab 5; `ITEM_OAKS_PARCEL` +1 |
| 9 | Delivery, Pokédex, 5 Balls | Talk to Oak | Mart var 1 or more; lab 6, Mart 2, old man 1, rival house 1, Route 22 1 |

## 4. Bedroom: `PalletTown_PlayersHouse_2F_Frlg`

Geometry: new game spawns at (6,6) (`src/new_game.c`). BG: NES (6,5), PC (1,1),
posted notice (11,1). Stairs warp (10,2) to 1F.

- `PalletTown_PlayersHouse_2F_OnTransition`: if 2F var is 0,
  `setrespawn HEAL_LOCATION_PALLET_TOWN`.
- `PalletTown_PlayersHouse_2F_FirstWarpIn` (on warp-in, 2F var 0): the player
  turns north, then 2F var is set to 1. No text.
- `PalletTown_PlayersHouse_2F_EventScript_NES` (sign).
- `PalletTown_PlayersHouse_2F_EventScript_Sign` (sign).
- `EventScript_PalletTown_PlayersHouse_2F_TurnOnPC`: PC-on effect, `SE_PC_ON`,
  message, then `special BedroomPC` (FRLG item storage, mailbox and turn-off menu).
  Off: `EventScript_PalletTown_PlayersHouse_2F_ShutDownPC`.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_PlayersHouse_2F_Text_PlayedWithNES` | sign | `{PLAYER} played with the NES.\p…Okay!\nIt's time to go!` |
| `PalletTown_PlayersHouse_2F_Text_PressLRForHelp` | sign | `It's a posted notice…\pIf you're confused, ask for HELP!\nPress the L or R Button!` |
| `gText_PlayerHouseBootPC` | narration | `{PLAYER} booted up the PC.` |

## 5. Home 1F: `PalletTown_PlayersHouse_1F_Frlg`

Geometry: Mom (8,4) faces left. TV bg (6,1). Exits (3,9), (4,8), (5,8) lead to
Pallet (6,7). Stairs (10,2).

`PalletTown_PlayersHouse_1F_EventScript_Mom` (lock, faceplayer):

1. If `FLAG_BEAT_RIVAL_IN_OAKS_LAB` is set, go to
   `PalletTown_PlayersHouse_1F_EventScript_MomHeal`:
   `YouShouldTakeQuickRest`, closemessage, `Common_EventScript_OutOfCenterPartyHeal`
   (fade to black, `MUS_HEAL` fanfare, `HealPlayerParty`, fade in), then
   `LookingGreatTakeCare`.
2. Otherwise `checkplayergender`: male shows `AllBoysLeaveOakLookingForYou`,
   female shows `AllGirlsLeaveOakLookingForYou`. Then closemessage and Mom turns
   back to her original direction.

`PalletTown_PlayersHouse_1F_EventScript_TV` (lockall): if the player faces
north (in front of the screen), branch on gender: male
`MovieOnTVFourBoysOnRailroad`, female `MovieOnTVGirlOnBrickRoad`. From any other
side it shows `OopsWrongSide`.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_PlayersHouse_1F_Text_AllBoysLeaveOakLookingForYou` | Mom (male player) | `MOM: …Right.\nAll boys leave home someday.\lIt said so on TV.\pOh, yes. PROF. OAK, next door, was\nlooking for you.` |
| `PalletTown_PlayersHouse_1F_Text_AllGirlsLeaveOakLookingForYou` | Mom (female player) | `MOM: …Right.\nAll girls dream of traveling.\lIt said so on TV.\pOh, yes. PROF. OAK, next door, was\nlooking for you.` |
| `PalletTown_PlayersHouse_1F_Text_YouShouldTakeQuickRest` | Mom (after lab battle) | `MOM: {PLAYER}!\nYou should take a quick rest.` |
| `PalletTown_PlayersHouse_1F_Text_LookingGreatTakeCare` | Mom (after heal) | `MOM: Oh, good! You and your\nPOKéMON are looking great.\lTake care now!` |
| `PalletTown_PlayersHouse_1F_Text_MovieOnTVFourBoysOnRailroad` | TV (male, facing north) | `There's a movie on TV.\nFour boys are walking on railroad\ltracks.\p…I better go, too.` |
| `PalletTown_PlayersHouse_1F_Text_MovieOnTVGirlOnBrickRoad` | TV (female, facing north) | `There's a movie on TV.\nA girl with her hair in pigtails is\lwalking up a brick road.\p…I better go, too.` |
| `PalletTown_PlayersHouse_1F_Text_OopsWrongSide` | TV (not facing north) | `Oops, wrong side…` |

## 6. Pallet Town: `PalletTown_Frlg`

Geometry:

| Object/event | Position | Notes |
| --- | --- | --- |
| Sign lady (`WOMAN_1`), local 1 | (3,10) wander | Repositioned on transition (see below). |
| Fat man, local 2 | (13,17) wander | `FatMan` script. |
| Prof. Oak, local 3 | (10,8) face up | Hidden by `FLAG_HIDE_OAK_IN_PALLET_TOWN`. |
| Coord: Oak trigger left | (12,1), Oak var 0 | `OakTriggerLeft` |
| Coord: Oak trigger right | (13,1), Oak var 0 | `OakTriggerRight` |
| Coord: sign lady trigger | (13,2), `VAR_TEMP_2` = 1 | `SignLadyTrigger` |
| BG signs | lab (16,16), player's house (4,7), rival's house (13,7), town (9,11), Trainer Tips (5,14) | |
| Warps | player's house (6,7), rival's house (15,7), lab door (16,13) | |

### 6.1 Map scripts

`PalletTown_OnTransition`: `setworldmapflag FLAG_WORLD_MAP_PALLET_TOWN`, then
in order:

1. If `FLAG_PALLET_LADY_NOT_BLOCKING_SIGN` is set, run
   `PalletTown_EventScript_TryReadySignLady`: when `FLAG_OPENED_START_MENU` is
   set and the lady var is below 1, set the lady var to 1.
2. If the lady var is 0, run `PalletTown_EventScript_SetSignLadyPos`. Before the
   starter (flag clear) she moves to (5,15) facing up, directly below the Trainer
   Tips sign at (5,14), which blocks it from the south. After the starter
   (`PalletTown_EventScript_MoveSignLadyToRouteEntrance`) she moves to (12,2)
   facing down and `SIGN_LADY_READY` (`VAR_TEMP_2`) becomes TRUE, which arms
   coord (13,2).
3. If the lady var is 1, it is set to 2 (done) and she stays at her default
   wander spot (3,10).

`OnFrame` only holds the post-Elite Four `OakRatingScene` (Oak var 2), which is
out of scope.

### 6.2 Oak interception

`PalletTown_EventScript_OakTriggerLeft` / `PalletTown_EventScript_OakTriggerRight`
(lockall; `VAR_TEMP_1` = 0 or 1) run `PalletTown_EventScript_OakTrigger`:

1. Fame Checker Oak update, male textcolor, delay 30, `playbgm MUS_RG_OAK`.
2. `message OakDontGoOut`, waitmessage, delay 85, closemessage. The box
   auto-dismisses without a button press.
3. The player turns down, `SE_PIN`, exclamation mark, delay 30.
4. `addobject` Oak at (10,8). Left: up 2, right, up 2, right, up 2, ending at
   (12,2) below the player at (12,1). Right: right, up 2, right, up 2, right,
   up 2, ending at (13,2).
5. Delay 30, `msgbox OakGrassUnsafeNeedMon`, closemessage, delay 30.
6. Walk to the lab. Oak goes down one tile, (right side only: left 1), left,
   down 11, right 5, faces up at (16,14). The player follows one tile behind:
   down 2, (right side: left 1), left, down 11, right 4.
7. `opendoor 16,13`. Oak steps up into the door and becomes invisible; the player
   steps right, up, and becomes invisible. `closedoor`.
8. State: lab var = 1, `clearflag FLAG_HIDE_OAK_IN_HIS_LAB`, Oak var = 1,
   `setflag FLAG_HIDE_OAK_IN_PALLET_TOWN`, `setflag FLAG_DONT_TRANSITION_MUSIC`.
9. `warp MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6, 12`.

The two coord tiles are the whole north exit. The south edge is water, so FRLG
has no other exit before Surf.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_Text_OakDontGoOut` | Oak (off-screen, auto-close) | `OAK: Hey! Wait!\nDon't go out!` |
| `PalletTown_Text_OakGrassUnsafeNeedMon` | Oak | `OAK: It's unsafe!\nWild POKéMON live in tall grass!\pYou need your own POKéMON for\nyour protection.\pI know!\nHere, come with me!` |

### 6.3 Sign lady (all branches)

`PalletTown_EventScript_SignLady` (lock). Checks run in this order:

1. Lady var 2: face player, `RaisingMonsToo`.
2. Lady var 1: face player, `SignsAreUsefulArentThey`.
3. `SIGN_LADY_READY` TRUE (post-starter, at the route entrance): face player and
   `call SignLadyShowSign` (below).
4. `FLAG_TEMP_2` set (she already stepped aside on this map load): face player,
   `ReadItReadIt`.
5. Otherwise (pre-starter, blocking the sign): `HmmIsThatRight` is shown while she
   still faces the sign. Then she faces the player, `SE_PIN`, exclamation, a
   48-frame delay, and `OhLookLook`. She steps right then faces left if the
   player faces east, otherwise steps left then faces right.
   `copyobjectxytoperm`, `setflag FLAG_TEMP_2`.

`PalletTown_EventScript_SignLadyTrigger` (coord (13,2) with
`SIGN_LADY_READY`): the lady faces right and the player faces left, then the
same show-sign subroutine runs.

`PalletTown_EventScript_SignLadyShowSign`: female textcolor,
`LookCopiedTrainerTipsSign`, closemessage, delay 20, neutral textcolor,
`setflag FLAG_OPENED_START_MENU`, lady var = 1, `SIGN_LADY_READY` = FALSE,
`special SetWalkingIntoSignVars`, `special DisableMsgBoxWalkaway`, then `signmsg`
(sign-style box) `PressStartToOpenMenuCopy` and `normalmsg`.

Starter receipt (section 8.4) also sets the lady var to 1 when
`FLAG_OPENED_START_MENU` is already set. A player who opened START before
choosing never sees the route-entrance scene, and the next transition makes
the lady "done".

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_Text_HmmIsThatRight` | Sign lady (pre-starter, first talk) | `Hmm…\nIs that right…` |
| `PalletTown_Text_OhLookLook` | Sign lady (after exclamation) | `Oh!\nLook, look!` |
| `PalletTown_Text_ReadItReadIt` | Sign lady (stepped aside) | `Read it, read it!` |
| `PalletTown_Text_LookCopiedTrainerTipsSign` | Sign lady (post-starter show) | `Look, look!\pI copied what it said on one of\nthose TRAINER TIPS signs!` |
| `PalletTown_Text_PressStartToOpenMenuCopy` | Sign lady's copy (sign box) | `TRAINER TIPS!\pPress START to open the MENU!` |
| `PalletTown_Text_SignsAreUsefulArentThey` | Sign lady (lady var 1) | `Signs are useful, aren't they?` |
| `PalletTown_Text_RaisingMonsToo` | Sign lady (lady var 2) | `I'm raising POKéMON, too.\pWhen they get strong, they can\nprotect me.` |

### 6.4 Other NPCs and signs

- `PalletTown_EventScript_FatMan` (`MSGBOX_NPC`).
- `PalletTown_EventScript_OaksLabSign` (lockall, Fame Checker Oak 0).
- `PalletTown_EventScript_PlayersHouseSign`, `PalletTown_EventScript_RivalsHouseSign`,
  `PalletTown_EventScript_TownSign` (`MSGBOX_SIGN`).
- `PalletTown_EventScript_TrainerTips` (lockall): message, then lady var = 1.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_Text_CanStoreItemsAndMonsInPC` | Fat man | `Technology is incredible!\pYou can now store and recall items\nand POKéMON as data via PC.` |
| `PalletTown_Text_OakPokemonResearchLab` | Lab sign (16,16) | `OAK POKéMON RESEARCH LAB` |
| `PalletTown_Text_PlayersHouse` | House sign (4,7) | `{PLAYER}'s house` |
| `PalletTown_Text_RivalsHouse` | House sign (13,7) | `{RIVAL}'s house` |
| `PalletTown_Text_TownSign` | Town sign (9,11) | `PALLET TOWN\nShades of your journey await!` |
| `PalletTown_Text_PressStartToOpenMenu` | Trainer Tips sign (5,14) | `TRAINER TIPS\pPress START to open the MENU!` |

## 7. Rival's house: `PalletTown_RivalsHouse_Frlg`

Geometry: Daisy, local 1, template position (10,6), wander. Town Map object,
local 2, at (6,4), hidden by `FLAG_HIDE_TOWN_MAP`, which is clear at new game.
Bookshelves (11,1) and (12,1), picture (9,1). Exits (3..5,8).

`PalletTown_RivalsHouse_OnTransition`: while the house var is below 2, Daisy
is moved to (5,4) facing right (seated at the table). At 2 or more,
`RECEIVED_TOWN_MAP` (`VAR_TEMP_1`) becomes TRUE.

`PalletTown_RivalsHouse_EventScript_Daisy` (lock, faceplayer, Fame Checker).
First match wins:

1. `FLAG_SYS_GAME_CLEAR`: grooming and friendship (post-game, out of scope).
2. `RECEIVED_TOWN_MAP`: `PleaseGiveMonsRest` (post-handoff).
3. House var 2: `ExplainTownMap` (post-handoff, same visit as the gift).
4. House var 1: `PalletTown_RivalsHouse_EventScript_GiveTownMap`
   (**post-handoff**): `ErrandForGrandpaThisWillHelp`, closemessage,
   `checkitemspace ITEM_TOWN_MAP`. If full: `DontHaveSpaceForThis`. Otherwise Daisy
   faces right, `removeobject LOCALID_TOWN_MAP`, house var = 2, delay 15, faces
   player, delay 12, `giveitem_msg ReceivedTownMapFromDaisy, ITEM_TOWN_MAP`
   (`MUS_RG_OBTAIN_KEY_ITEM`).
5. Lab var 1 or more (from the moment Oak escorts the player, **not** from the
   battle): `HeardYouBattledRival`.
6. Otherwise: `HiBrothersAtLab`, closemessage, Daisy faces her original
   direction.

`PalletTown_RivalsHouse_EventScript_TownMap` (`MSGBOX_NPC`, visible until the
gift), `PalletTown_RivalsHouse_EventScript_Bookshelf`,
`PalletTown_RivalsHouse_EventScript_Picture`.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_RivalsHouse_Text_HiBrothersAtLab` | Daisy (lab var 0) | `DAISY: Hi, {PLAYER}!\pMy brother, {RIVAL}, is out at\nGrandpa's LAB.` |
| `PalletTown_RivalsHouse_Text_HeardYouBattledRival` | Daisy (lab var 1 or more, pre-handoff) | `DAISY: {PLAYER}, I heard you had\na battle against {RIVAL}.\pI wish I'd seen that!` |
| `PalletTown_RivalsHouse_Text_ItsBigMapOfKanto` | Town Map object | `It's a big map of the KANTO region.\nNow this would be useful!` |
| `PalletTown_RivalsHouse_Text_ShelvesCrammedFullOfBooks` | Bookshelf | `The shelves are crammed full of\nbooks on POKéMON.` |
| `PalletTown_RivalsHouse_Text_LovelyAndSweetClefairy` | Picture | `“The lovely and sweet\nCLEFAIRY”` |
| `PalletTown_RivalsHouse_Text_ErrandForGrandpaThisWillHelp` | Daisy (post-handoff) | `Grandpa asked you to run an\nerrand?\pGee, that's lazy of him.\nHere, this will help you.` |
| `PalletTown_RivalsHouse_Text_ReceivedTownMapFromDaisy` | narration (post-handoff) | `{PLAYER} received a TOWN MAP\nfrom DAISY.` |
| `PalletTown_RivalsHouse_Text_DontHaveSpaceForThis` | Daisy (post-handoff, bag full) | `You don't have space for this in\nyour BAG.` |
| `PalletTown_RivalsHouse_Text_ExplainTownMap` | Daisy (post-handoff) | `You can use the TOWN MAP to find\nout where you are, or check the\lnames of places.` |
| `PalletTown_RivalsHouse_Text_PleaseGiveMonsRest` | Daisy (post-handoff) | `DAISY: Just like people, POKéMON\nare living things.\pWhen they get tired, please give\nthem a rest.` |

## 8. Oak's lab: `PalletTown_ProfessorOaksLab_Frlg`

Geometry:

| Object/event | Position | Visibility |
| --- | --- | --- |
| Aide 1 (scientist) | (3,11) look around | always |
| Aide 3 (worker F) | (2,10) wander up/down | always |
| Aide 2 (scientist) | (11,10) look around | always |
| Prof. Oak, local 4 | (6,3) face down | `FLAG_HIDE_OAK_IN_HIS_LAB` (set at new game) |
| Bulbasaur ball, local 5 | (8,4) | `FLAG_HIDE_BULBASAUR_BALL` |
| Squirtle ball, local 6 | (9,4) | `FLAG_HIDE_SQUIRTLE_BALL` |
| Charmander ball, local 7 | (10,4) | `FLAG_HIDE_CHARMANDER_BALL` |
| Rival, local 8 | (5,4) face down | `FLAG_HIDE_RIVAL_IN_LAB` (clear at new game) |
| Pokédex 1 and 2, locals 9 and 10 | (4,1), (5,1) | `FLAG_HIDE_POKEDEX` (clear at new game) |
| Coord (5,8), (6,8), (7,8), lab var 2 | | `LeaveStarterSceneTrigger` |
| Coord (5,8), (6,8), (7,8), lab var 3 | | `RivalBattleTrigger{Left,Mid,Right}` |
| BG computer | (2,1), (3,1) | `Computer` |
| BG posters | (6,1) left, (7,1) right | `LeftSign`, `RightSign` |
| Warps | (5..7,12) to Pallet door | |

### 8.1 Map scripts

- `PalletTown_ProfessorOaksLab_OnTransition`: `setflag FLAG_VISITED_OAKS_LAB`.
  At lab var 1, `PalletTown_ProfessorOaksLab_EventScript_ReadyOakForStarterScene`
  puts Oak at (6,11) facing up and runs `savebgm MUS_RG_OAK`. Vars 7 and 8 are
  post-game.
- `OnWarp` at lab var 1: the player faces north.
- `OnFrame` at lab var 1: `PalletTown_ProfessorOaksLab_ChooseStarterScene`.

### 8.2 Arrival exchange (lab var 1 to 2)

`PalletTown_ProfessorOaksLab_ChooseStarterScene` (lockall, male textcolor):

1. Oak walks up 6 from (6,11), `removeobject`, template moved to (6,3) facing
   down, `clearflag FLAG_HIDE_OAK_IN_HIS_LAB`. He re-spawns behind the desk.
2. The player walks up 8 from (6,12) to (6,4). The rival at (5,4) is to the
   player's left.
3. The rival faces up. `clearflag FLAG_DONT_TRANSITION_MUSIC`, `savebgm MUS_DUMMY`,
   `fadedefaultbgm`.
4. `RivalFedUpWithWaiting`, closemessage, delay 60.
5. `OakThreeMonsChooseOne`, closemessage, delay 30.
6. The rival walks in place up twice, then `RivalNoFairWhatAboutMe` and
   `OakBePatientRival`.
7. Lab var = 2, releaseall.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_ProfessorOaksLab_Text_RivalFedUpWithWaiting` | Rival | `{RIVAL}: Gramps!\nI'm fed up with waiting!` |
| `PalletTown_ProfessorOaksLab_Text_OakThreeMonsChooseOne` | Oak | `OAK: {RIVAL}?\nLet me think…\pOh, that's right, I told you to\ncome! Just wait!\pHere, {PLAYER}.\pThere are three POKéMON here.\pHaha!\pThe POKéMON are held inside\nthese POKé BALLS.\pWhen I was young, I was a serious\nPOKéMON TRAINER.\pBut now, in my old age, I have\nonly these three left.\pYou can have one.\nGo on, choose!` |
| `PalletTown_ProfessorOaksLab_Text_RivalNoFairWhatAboutMe` | Rival | `{RIVAL}: Hey! Gramps! No fair!\nWhat about me?` |
| `PalletTown_ProfessorOaksLab_Text_OakBePatientRival` | Oak | `OAK: Be patient, {RIVAL}.\nYou can have one, too!` |

### 8.3 Talk scripts by lab state

`PalletTown_ProfessorOaksLab_EventScript_Rival` (lock, faceplayer): lab var 3
gives `RivalMyMonLooksTougher`, var 2 gives `RivalGoChoosePlayer`, anything else
(var 0, and var 1 before the scene) gives `RivalGrampsIsntAround`. After the
battle the rival is removed.

`PalletTown_ProfessorOaksLab_EventScript_ProfOak` (lock, faceplayer). The
in-scope checks, in source order:

1. Post-game checks (`SHOWED_OAK_COMPLETE_DEX`, lab var 9/8, `FLAG_SYS_GAME_CLEAR`,
   Cerulean rival var 1) are out of scope.
2. Lab var 6: `RatePokedexOrTryGiveBalls` (post-handoff). With only the starter
   caught it usually gives `OakMonsAroundWorldWait`; otherwise the Pokédex rating.
3. `VAR_MAP_SCENE_VIRIDIAN_CITY_MART >= 1`: **delivery scene** (section 8.8).
   It is keyed on the Mart var, not on lab var 5.
4. Lab var 4: `OakBattleMonForItToGrow`.
5. Lab var 3: `OakCanReachNextTownWithMon`.
6. Otherwise (var 2): `OakWhichOneWillYouChoose`.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_ProfessorOaksLab_Text_RivalGrampsIsntAround` | Rival (lab var 0) | `{RIVAL}: What, it's only {PLAYER}?\nGramps isn't around.` |
| `PalletTown_ProfessorOaksLab_Text_RivalGoChoosePlayer` | Rival (lab var 2) | `{RIVAL}: Heh, I don't need to be\ngreedy like you. I'm mature!\pGo ahead and choose, {PLAYER}!` |
| `PalletTown_ProfessorOaksLab_Text_RivalMyMonLooksTougher` | Rival (lab var 3) | `{RIVAL}: My POKéMON looks a lot\ntougher than yours.` |
| `PalletTown_ProfessorOaksLab_Text_OakWhichOneWillYouChoose` | Oak (lab var 2) | `OAK: Now, {PLAYER}.\pInside those three POKé BALLS are\nPOKéMON.\pWhich one will you choose for\nyourself?` |
| `PalletTown_ProfessorOaksLab_Text_OakCanReachNextTownWithMon` | Oak (lab var 3) | `OAK: If a wild POKéMON appears,\nyour POKéMON can battle it.\pWith it at your side, you should be\nable to reach the next town.` |
| `PalletTown_ProfessorOaksLab_Text_OakBattleMonForItToGrow` | Oak (lab var 4) | `OAK: {PLAYER}, raise your young\nPOKéMON by making it battle.\pIt has to battle for it to grow.` |
| `PalletTown_ProfessorOaksLab_Text_OakMonsAroundWorldWait` | Oak (lab var 6, post-handoff) | `POKéMON around the world wait for\nyou, {PLAYER}!` |

### 8.4 Starter choice (lab var 2 to 3)

Ball scripts `PalletTown_ProfessorOaksLab_EventScript_BulbasaurBall`,
`PalletTown_ProfessorOaksLab_EventScript_SquirtleBall` and
`PalletTown_ProfessorOaksLab_EventScript_CharmanderBall` (lock, faceplayer) set:

| Ball | `PLAYER_STARTER_NUM` | Player species | Rival species | Rival ball |
| --- | --- | --- | --- | --- |
| (8,4) Bulbasaur | 0 | `SPECIES_BULBASAUR` | `SPECIES_CHARMANDER` | Charmander ball (10,4) |
| (9,4) Squirtle | 1 | `SPECIES_SQUIRTLE` | `SPECIES_BULBASAUR` | Bulbasaur ball (8,4) |
| (10,4) Charmander | 2 | `SPECIES_CHARMANDER` | `SPECIES_SQUIRTLE` | Squirtle ball (9,4) |

Branches: lab var 3 or more gives `OaksLastMon` (the one remaining ball). Var 2
goes to `PalletTown_ProfessorOaksLab_EventScript_ConfirmStarterChoice`. Var 0
or 1 gives `ThoseArePokeBalls`.

ConfirmStarterChoice: Oak faces right, `showmonpic` of the species at (10,3), male
textcolor, then the per-species `MSGBOX_YESNO` (`OakChoosingBulbasaur`,
`OakChoosingSquirtle` or `OakChoosingCharmander`).

- **NO**: `PalletTown_ProfessorOaksLab_EventScript_DeclinedStarter`:
  `hidemonpic`, release. No text, nothing committed. Any ball can be inspected
  again.
- **YES**: `PalletTown_ProfessorOaksLab_EventScript_ChoseStarter`:
  1. `hidemonpic`, `removeobject VAR_LAST_TALKED` (the chosen ball).
  2. `OakThisMonIsEnergetic` (Oak), restore textcolor.
  3. `setflag FLAG_SYS_POKEMON_GET`, `setflag FLAG_PALLET_LADY_NOT_BLOCKING_SIGN`.
  4. `givemon <species>, 5`. The result is not checked; a new game always has
     room.
  5. `copyvar VAR_STARTER_MON, PLAYER_STARTER_NUM`,
     `bufferspeciesname STR_VAR_1`.
  6. `message ReceivedMonFromOak`, waitmessage, fanfare `MUS_RG_OBTAIN_KEY_ITEM`.
  7. `msgbox gText_NicknameThisPokemon, MSGBOX_YESNO`. YES:
     `EventScript_GiveNicknameToStarter`, which runs
     `Common_EventScript_NameReceivedPartyMon` (fade, `ChangePokemonNickname`
     naming screen for party slot 0). NO: continue.
  8. `PalletTown_ProfessorOaksLab_EventScript_RivalPicksStarter`: closemessage,
     then the rival walks from (5,4). For Charmander: down 2, right 5, up, ending
     at (10,5). For Squirtle: down, right 4, face up at (9,5). For Bulbasaur:
     down, right 3, face up at (8,5).
  9. `PalletTown_ProfessorOaksLab_EventScript_RivalTakesStarter`: male textcolor,
     `RivalIllTakeThisOneThen`, `removeobject RIVAL_STARTER_ID`, neutral textcolor,
     `bufferspeciesname STR_VAR_1, RIVAL_STARTER_SPECIES`,
     `message RivalReceivedMonFromOak`, fanfare `MUS_RG_OBTAIN_KEY_ITEM`.
  10. Lab var = 3. If `FLAG_OPENED_START_MENU` is set, lady var = 1.

`PalletTown_ProfessorOaksLab_EventScript_LeaveStarterSceneTrigger` (coords
y=8, lab var 2): Oak faces down, `OakHeyDontGoAwayYet`, and the player is pushed
up 1 tile. This is how FRLG prevents leaving without a starter.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_ProfessorOaksLab_Text_ThoseArePokeBalls` | ball (lab var 0 or 1) | `Those are POKé BALLS.\nThey contain POKéMON!` |
| `PalletTown_ProfessorOaksLab_Text_OakChoosingBulbasaur` | Oak (YES/NO) | `I see! BULBASAUR is your choice.\nIt's very easy to raise.\pSo, {PLAYER}, you want to go with\nthe GRASS POKéMON BULBASAUR?` |
| `PalletTown_ProfessorOaksLab_Text_OakChoosingSquirtle` | Oak (YES/NO) | `Hm! SQUIRTLE is your choice.\nIt's one worth raising.\pSo, {PLAYER}, you've decided on the\nWATER POKéMON SQUIRTLE?` |
| `PalletTown_ProfessorOaksLab_Text_OakChoosingCharmander` | Oak (YES/NO) | `Ah! CHARMANDER is your choice.\nYou should raise it patiently.\pSo, {PLAYER}, you're claiming the\nFIRE POKéMON CHARMANDER?` |
| `PalletTown_ProfessorOaksLab_Text_OakThisMonIsEnergetic` | Oak | `This POKéMON is really quite\nenergetic!` |
| `PalletTown_ProfessorOaksLab_Text_ReceivedMonFromOak` | narration (+ fanfare) | `{PLAYER} received the {STR_VAR_1}\nfrom PROF. OAK!` |
| `gText_NicknameThisPokemon` | system (YES/NO) | `Do you want to give a nickname to\nthis {STR_VAR_1}?` |
| `PalletTown_ProfessorOaksLab_Text_RivalIllTakeThisOneThen` | Rival | `{RIVAL}: I'll take this one, then!` |
| `PalletTown_ProfessorOaksLab_Text_RivalReceivedMonFromOak` | narration (+ fanfare) | `{RIVAL} received the {STR_VAR_1}\nfrom PROF. OAK!` |
| `PalletTown_ProfessorOaksLab_Text_OaksLastMon` | remaining ball (lab var 3 or more) | `That's PROF. OAK's last POKéMON.` |
| `PalletTown_ProfessorOaksLab_Text_OakHeyDontGoAwayYet` | Oak (exit coord, lab var 2) | `OAK: Hey!\nDon't go away yet!` |

### 8.5 Rival battle (lab var 3 to 4)

`PalletTown_ProfessorOaksLab_EventScript_RivalBattleTriggerLeft`,
`PalletTown_ProfessorOaksLab_EventScript_RivalBattleTriggerMid` and
`PalletTown_ProfessorOaksLab_EventScript_RivalBattleTriggerRight` (coords (5,8),
(6,8), (7,8); `VAR_TEMP_2` = 1, 2 or 3) run
`PalletTown_ProfessorOaksLab_EventScript_RivalBattle`:

1. Male textcolor, `playbgm MUS_RG_ENCOUNTER_RIVAL`, the rival faces down, the
   player faces up.
2. `RivalLetsCheckOutMons`, closemessage, Oak faces down.
3. Branch on `VAR_STARTER_MON`, then on lane. The rival walks left to the
   player's column, then down 2, stopping directly above the player at
   (lane x, 7).
4. `trainerbattle_earlyrival <trainer>, RIVAL_BATTLE_TUTORIAL,
   PalletTown_ProfessorOaksLab_Text_RivalDefeat, Text_RivalVictory`. The third
   argument is shown when the rival loses; the fourth when the rival wins.

| `VAR_STARTER_MON` (player) | Trainer | Party |
| --- | --- | --- |
| 0 Bulbasaur | `TRAINER_RIVAL_OAKS_LAB_CHARMANDER` (228) | Charmander Lv5, Scratch, Growl |
| 1 Squirtle | `TRAINER_RIVAL_OAKS_LAB_BULBASAUR` (227) | Bulbasaur Lv5, Tackle, Growl |
| 2 Charmander | `TRAINER_RIVAL_OAKS_LAB_SQUIRTLE` (226) | Squirtle Lv5, Tackle, Tail Whip |

All three are class `Rival Early Frlg` with 0 IVs. The party-file name is `TERRY`
but is never shown: for rival classes, `battle_message.c` substitutes
`PLACEHOLDER_ID_RIVAL`.

Battle mechanics (`battle_setup.c`, `battle_main.c`):

- `RIVAL_BATTLE_TUTORIAL` = 3 (`RIVAL_BATTLE_HEAL_AFTER` 1 plus the tutorial bit)
  adds `BATTLE_TYPE_FIRST_BATTLE`. Running is forbidden and routed to Oak's line.
- **Loss** is not a game over. `VAR_RESULT` = TRUE, `HealPlayerParty`, no
  white-out text (`MULTISTRING_CHOOSER` 1), and the script continues. **Win** sets
  `VAR_RESULT` = FALSE. Both set the trainer flag and return to the script.

`PalletTown_ProfessorOaksLab_EventScript_EndRivalBattle` (both outcomes):
`special HealPlayerParty`, `RivalGoToughenMyMon`, closemessage,
`playbgm MUS_RG_RIVAL_EXIT`, and the rival exits by lane. Left: right, down 5.
Mid: right, down 3, left, down 2. Right: left, down 5. The player watches
(turns right, or left for the right lane, then down). `removeobject` rival,
`SE_EXIT`, `fadedefaultbgm`, lab var = 4, `setflag FLAG_BEAT_RIVAL_IN_OAKS_LAB`.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_ProfessorOaksLab_Text_RivalLetsCheckOutMons` | Rival | `{RIVAL}: Wait, {PLAYER}!\nLet's check out our POKéMON!\pCome on, I'll take you on!` |
| `PalletTown_ProfessorOaksLab_Text_RivalDefeat` | Rival (in battle, player wins) | `WHAT?\nUnbelievable!\lI picked the wrong POKéMON!` |
| `Text_RivalVictory` | Rival (in battle, player loses) | `{RIVAL}: Yeah!\nAm I great or what?` |
| `PalletTown_ProfessorOaksLab_Text_RivalGoToughenMyMon` | Rival (after either outcome) | `{RIVAL}: Okay! I'll make my\nPOKéMON battle to toughen it up!\p{PLAYER}! Gramps!\nSmell you later!` |

### 8.6 Oak's in-battle tutorial (FRLG controller)

In an FRLG build, `BATTLE_TYPE_FIRST_BATTLE` selects `SetControllerToOakOrOldMan`
for the player (`battle_controllers.c`: `IS_FRLG && FIRST_BATTLE`). Each hook
fires once per battle (state2 flags `FIRST_BATTLE_MSG_FLAG_*`):

| Hook | When | Text |
| --- | --- | --- |
| Intro (3 boxes, main BG darkened) | After the send-out and health box | `ForPetesSake`, then `TheTrainerThat`, then `TryBattling` |
| `INFLICT_DMG` | First HP change on the opponent | `InflictingDamageIsKey` |
| `STAT_CHG` | First `STRINGID_DEFENDERSSTATFELL` | `LoweringStats` |
| `HP_RESTORE` | First Potion used from the in-battle Bag | `KeepAnEyeOnHP` |
| `PARTY_MENU` | First time the party menu opens | `OakImportantToGetToKnowPokemonThroughly`, then `OakThisIsListOfPokemon` |
| Run attempt | `STRINGID_DONTLEAVEBIRCH` (`BATTLE_RUN_FORBIDDEN`) | `OakNoRunningFromATrainer` |
| Win | `STRINGID_PLAYERGOTMONEY` | `WinEarnsPrizeMoney` |
| Loss | `STRINGID_TRAINER1WINTEXT` (after `Text_RivalVictory`) | `HowDissapointing` (sic in source) |

| Label | Speaker | Text |
| --- | --- | --- |
| `sText_ForPetesSake` | Oak | `OAK: Oh, for Pete's sake…\nSo pushy, as always.\p{B_PLAYER_NAME}.\pYou've never had a POKéMON battle\nbefore, have you?\pA POKéMON battle is when TRAINERS\npit their POKéMON against each\lother.\p` |
| `sText_TheTrainerThat` | Oak | `The TRAINER that makes the other\nTRAINER's POKéMON faint by lowering\ltheir HP to “0,” wins.\p` |
| `sText_TryBattling` | Oak | `But rather than talking about it,\nyou'll learn more from experience.\pTry battling and see for yourself.\p` |
| `sText_InflictingDamageIsKey` | Oak | `OAK: Inflicting damage on the foe\nis the key to any battle.\p` |
| `sText_LoweringStats` | Oak | `OAK: Lowering the foe's stats\nwill put you at an advantage.\p` |
| `sText_KeepAnEyeOnHP` | Oak | `OAK: Keep your eyes on your\nPOKéMON's HP.\pIt will faint if the HP drops to\n“0.”\p` |
| `gText_OakImportantToGetToKnowPokemonThroughly` | Oak (party menu) | `OAK: It's important to get to know\nyour POKéMON thoroughly.\p` |
| `gText_OakThisIsListOfPokemon` | Oak (party menu) | `This is a list of your POKéMON,\n{PLAYER}.\pOpen this to check the skills\nand moves of your POKéMON.\pYou also choose POKéMON here if\nyou want to use an item on one.{PAUSE_UNTIL_PRESS}` |
| `sText_OakNoRunningFromATrainer` | Oak | `OAK: No! There's no running away\nfrom a TRAINER POKéMON battle!\p` |
| `sText_WinEarnsPrizeMoney` | Oak (win) | `OAK: Hm! Excellent!\pIf you win, you earn prize money,\nand your POKéMON will grow!\pBattle other TRAINERS and make\nyour POKéMON strong!\p` |
| `sText_HowDissapointing` | Oak (loss) | `OAK: Hm…\nHow disappointing…\pIf you win, you earn prize money,\nand your POKéMON grow.\pBut if you lose, {B_PLAYER_NAME}, you end\nup paying prize money…\pHowever, since you had no warning\nthis time, I'll pay for you.\pBut things won't be this way once\nyou step outside these doors.\pThat's why you must strengthen your\nPOKéMON by battling wild POKéMON.\p` |

The ordinary engine battle strings ("... would like to battle!", send-out, prize
money, "... was defeated!") come from `battle_message.c` and are not transcribed
here.

### 8.7 Lab ambient objects

- `PalletTown_ProfessorOaksLab_EventScript_Aide1` and
  `PalletTown_ProfessorOaksLab_EventScript_Aide2`: `StudyAsOaksAide` (their
  `FLAG_SYS_GAME_CLEAR` branches are post-game).
- `PalletTown_ProfessorOaksLab_EventScript_Aide3`: `OakIsAuthorityOnMons`.
- `PalletTown_ProfessorOaksLab_EventScript_Pokedex` (both desk units, until the
  handoff removes them).
- `PalletTown_ProfessorOaksLab_EventScript_Computer`.
- `PalletTown_ProfessorOaksLab_EventScript_LeftSign`.
- `PalletTown_ProfessorOaksLab_EventScript_RightSign`: lab var 6 or more shows
  the types line; otherwise the SAVE line.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_ProfessorOaksLab_Text_StudyAsOaksAide` | Aide 1 (3,11), Aide 2 (11,10) | `I study POKéMON as PROF. OAK's\nAIDE.` |
| `PalletTown_ProfessorOaksLab_Text_OakIsAuthorityOnMons` | Aide 3 (2,10) | `PROF. OAK may not look like much,\nbut he's the authority on POKéMON.\pMany POKéMON TRAINERS hold him in\nhigh regard.` |
| `PalletTown_ProfessorOaksLab_Text_BlankEncyclopedia` | Pokédex unit (4,1)/(5,1) | `It's like an encyclopedia, but the\npages are blank.` |
| `PalletTown_ProfessorOaksLab_Text_EmailMessage` | Computer (2,1)/(3,1) | `There's an e-mail message here.\p…\pFinally!\nThe ultimate TRAINERS of the\lPOKéMON LEAGUE are ready to\ltake on all comers!\pBring your best POKéMON and see\nhow you rate as a TRAINER!\pPOKéMON LEAGUE HQ\nINDIGO PLATEAU\pPROF. OAK, please visit us!\n…` |
| `PalletTown_ProfessorOaksLab_Text_PressStartToOpenMenu` | Left poster (6,1) | `Press START to open the MENU!` |
| `PalletTown_ProfessorOaksLab_Text_SaveOptionInMenu` | Right poster (7,1), lab var below 6 | `The SAVE option is on the MENU.\nUse it regularly.` |
| `PalletTown_ProfessorOaksLab_Text_AllMonTypesHaveStrongAndWeakPoints` | Right poster (7,1), lab var 6 or more | `All POKéMON types have strong and\nweak points against others.` |

### 8.8 Parcel delivery, Pokédex, Poké Balls (lab var 5 to 6)

`PalletTown_ProfessorOaksLab_EventScript_ReceiveDexScene` (from the Oak talk
script, which already ran lock and faceplayer). `VAR_FACING` is the player's
facing when talking to Oak at (6,3): north means the player stands at (6,4);
east (5,3); west (7,3); south (6,2).

1. `OakHaveSomethingForMe`.
2. Neutral textcolor, fanfare `MUS_OBTAIN_TMHM`, `message DeliveredOaksParcel`.
   Restore textcolor. `removeitem ITEM_OAKS_PARCEL`.
3. `OakCustomBallIOrdered`.
4. `playbgm MUS_RG_ENCOUNTER_RIVAL`, `RivalGramps`, closemessage.
5. The rival enters, `addobject` at the door, walking up 6. North: from (5,10) to
   (5,4), next to the player; the player turns down, then left. South, east or
   west: from (6,10) to (6,4); Oak faces down, and the player turns down for
   east and west.
6. `fadedefaultbgm`, `RivalWhatDidYouCallMeFor`, closemessage, delay 30.
7. Oak: `SE_PIN`, exclamation, delay 48. Facing adjustments: Oak faces the
   player, then down, and the player faces Oak.
8. `OakHaveRequestForYouTwo`, closemessage.
9. Oak walks to the desk: up, left, faces down. For south he goes left 2, up,
   faces right. The player and rival watch.
10. `OakPokedexOnDesk`, closemessage, delay 40.
11. `OakTakeTheseWithYou`, closemessage. Oak faces up, `removeobject
    LOCALID_POKEDEX_1`, delay 10, `removeobject LOCALID_POKEDEX_2`, delay 25.
    Oak walks back (right, down; south: down, right). Delay 10.
12. Neutral textcolor, fanfare `MUS_RG_OBTAIN_KEY_ITEM`,
    `message ReceivedPokedexFromOak`. `setflag FLAG_SYS_POKEDEX_GET`,
    `special SetUnlockedPokedexFlags`, `VAR_MAP_SCENE_POKEMON_CENTER_TEALA` = 1.
13. `OakCatchMonsForDataTakeThese`.
14. `giveitem_msg ReceivedFivePokeBalls, ITEM_POKE_BALL, 5`: `additem`, then the
    message with the default `MUS_LEVEL_UP` fanfare. There is **no bag-full
    branch**.
15. `OakExplainCatching`, Fame Checker Oak 1, `OakCompleteMonGuideWasMyDream`.
16. `RivalLeaveItToMeGramps`. The rival faces the player (facing-dependent turns),
    then `RivalTellSisNotToGiveYouMap`, closemessage.
17. `playbgm MUS_RG_RIVAL_EXIT`, the rival walks down 6 (north case: the player
    turns down first), `removeobject`, `fadedefaultbgm`.
18. State: lab var = 6, Mart var = 2, old man var = 1, rival's house var = 1,
    Route 22 var = 1. Release.

| Label | Speaker | Text |
| --- | --- | --- |
| `PalletTown_ProfessorOaksLab_Text_OakHaveSomethingForMe` | Oak | `OAK: Oh, {PLAYER}!\nHow is my old POKéMON?\pWell, it seems to be growing more\nattached to you.\pYou must be talented as a POKéMON\nTRAINER.\pWhat's that?\nYou have something for me?` |
| `PalletTown_ProfessorOaksLab_Text_DeliveredOaksParcel` | narration (+ fanfare) | `{PLAYER} delivered OAK'S PARCEL.` |
| `PalletTown_ProfessorOaksLab_Text_OakCustomBallIOrdered` | Oak | `Ah! \nIt's the custom POKé BALL!\pI had it on order.\nThank you!` |
| `PalletTown_ProfessorOaksLab_Text_RivalGramps` | Rival (off-screen) | `{RIVAL}: Gramps!` |
| `PalletTown_ProfessorOaksLab_Text_RivalWhatDidYouCallMeFor` | Rival | `{RIVAL}: I almost forgot!\nWhat did you call me for?` |
| `PalletTown_ProfessorOaksLab_Text_OakHaveRequestForYouTwo` | Oak | `OAK: Oh, right!\nI have a request for you two.` |
| `PalletTown_ProfessorOaksLab_Text_OakPokedexOnDesk` | Oak | `On the desk there is my invention,\nthe POKéDEX!\pIt automatically records data on\nPOKéMON you've seen or caught.\pIt's a high-tech encyclopedia!` |
| `PalletTown_ProfessorOaksLab_Text_OakTakeTheseWithYou` | Oak | `OAK: {PLAYER} and {RIVAL}.\nTake these with you.` |
| `PalletTown_ProfessorOaksLab_Text_ReceivedPokedexFromOak` | narration (+ fanfare) | `{PLAYER} received the POKéDEX\nfrom PROF. OAK.` |
| `PalletTown_ProfessorOaksLab_Text_OakCatchMonsForDataTakeThese` | Oak | `OAK: You can't get detailed data\non POKéMON by just seeing them.\pYou must catch them to obtain\ncomplete data.\pSo, here are some tools for\ncatching wild POKéMON.` |
| `PalletTown_ProfessorOaksLab_Text_ReceivedFivePokeBalls` | narration (+ fanfare) | `{PLAYER} received five POKé BALLS.` |
| `PalletTown_ProfessorOaksLab_Text_OakExplainCatching` | Oak | `When a wild POKéMON appears,\nit's fair game.\pJust throw a POKé BALL at it and\ntry to catch it!\pThis won't always work, however.\pA healthy POKéMON can escape.\nYou have to be lucky!` |
| `PalletTown_ProfessorOaksLab_Text_OakCompleteMonGuideWasMyDream` | Oak | `To make a complete guide on all\nthe POKéMON in the world…\pThat was my dream!\pBut, I'm too old.\nI can't get the job done.\pSo, I want you two to fulfill my\ndream for me.\pGet moving, you two.\pThis is a great undertaking in\nPOKéMON history!` |
| `PalletTown_ProfessorOaksLab_Text_RivalLeaveItToMeGramps` | Rival | `{RIVAL}: All right, Gramps!\nLeave it all to me!` |
| `PalletTown_ProfessorOaksLab_Text_RivalTellSisNotToGiveYouMap` | Rival (no name prefix) | `{PLAYER}, I hate to say it, but you\nwon't be necessary for this.\pI know! I'll borrow a TOWN MAP\nfrom my sis!\pI'll tell her not to lend you one,\n{PLAYER}! Hahaha!\pDon't bother coming around to\nmy place after this!` |

## 9. Route 1: `Route1_Frlg`

Geometry: Mart clerk (6,28) wanders up and down near the Pallet end. Boy (19,16)
wanders left and right. Route sign (9,31). No trainers, no scene gates. The
clerk works on both legs of the errand.

`Route1_EventScript_MartClerk` (lock, faceplayer):

1. If `FLAG_GOT_POTION_ON_ROUTE_1` is set: `ComeSeeUsIfYouNeedPokeBalls`.
2. Otherwise `WorkAtPokeMartTakeSample`, neutral textcolor,
   `checkitemspace ITEM_POTION`. If full, `EventScript_BagIsFull`
   (`gText_TooBadBagIsFull`, release) and the flag stays clear.
3. Otherwise `bufferitemname STR_VAR_2`, fanfare `MUS_LEVEL_UP`,
   `message gText_ObtainedTheItem`, `additem ITEM_POTION`, `PutPotionAway`,
   `setflag FLAG_GOT_POTION_ON_ROUTE_1`.

`Route1_EventScript_Boy` (`MSGBOX_NPC`), `Route1_EventScript_RouteSign`.

| Label | Speaker | Text |
| --- | --- | --- |
| `Route1_Text_WorkAtPokeMartTakeSample` | Mart clerk | `Hi!\nI work at a POKéMON MART.\pIt's part of a convenient chain\nselling all sorts of items.\pPlease, visit us in VIRIDIAN CITY.\pI know, I'll give you a sample.\nHere you go!` |
| `gText_ObtainedTheItem` | narration (+ fanfare), STR_VAR_2 = POTION | `Obtained the {STR_VAR_2}!` |
| `Route1_Text_PutPotionAway` | narration | `{PLAYER} put the POTION away in\nthe BAG's ITEMS POCKET.` |
| `gText_TooBadBagIsFull` | narration (bag full) | `Too bad!\nThe BAG is full…` |
| `Route1_Text_ComeSeeUsIfYouNeedPokeBalls` | Mart clerk (repeat) | `Please come see us if you need\nPOKé BALLS for catching POKéMON.` |
| `Route1_Text_CanJumpFromLedges` | Boy | `See those ledges along the road?\pIt's a bit scary, but you can jump\nfrom them.\pYou can get back to PALLET TOWN\nquicker that way.` |
| `Route1_Text_RouteSign` | Route sign | `ROUTE 1\nPALLET TOWN - VIRIDIAN CITY` |

## 10. Viridian City during the Parcel phase: `ViridianCity_Frlg`

### 10.1 Old man (grandpa) and granddaughter

Current repository state (after commit `14f5e09e5c`):

- Tutorial old man, local 4 (`OBJ_EVENT_GFX_VAR_0`), template (21,8), look
  around. `ViridianCity_OnTransition` sets the graphics to `OLD_MAN_1` at
  (21,8), look around, for old man var 0 and 1.
- Only one tutorial coord remains: (20,8) at old man var 1 runs
  `TutorialTriggerLeft`. There is **no** road block during the Parcel phase.

Vanilla FRLG (removed by `14f5e09e5c`; this is the behavior the product oracle
most likely wants):

- Old man var 0: graphics `OBJ_EVENT_GFX_OLD_MAN_LYING_DOWN` at (21,11) facing
  down, lying across the north road.
- Coord (22,11) at old man var 0 runs `ViridianCity_EventScript_RoadBlocked`:
  lockall, male textcolor, `ThisIsPrivateProperty`, closemessage, the player is
  pushed down 1.
- Coords (20,8) and (22,8) at old man var 1 run `TutorialTriggerLeft` /
  `TutorialTriggerRight`. The template position was (21,6).

`ViridianCity_EventScript_TutorialOldMan` (lock, faceplayer) branches:
`FLAG_BADGE01_GET` gives the Teachy TV question (post-game, out of scope);
old man var 2 or more gives `WeakenMonsFirstToCatch`; var 1 runs the tutorial
(post-handoff); var 0 (`ViridianCity_EventScript_TutorialNotReady`) gives
`ThisIsPrivateProperty` and closemessage.

`ViridianCity_EventScript_Woman` (granddaughter, local 5, (20,12) facing up;
lock, faceplayer): old man var 0 gives `GrandpaHasntHadCoffeeYet`, closemessage,
then she faces her original direction. Otherwise `GoShoppingInPewterOccasionally`.

Post-handoff tutorial, for reference only:
`ViridianCity_EventScript_DoTutorialBattle` shows `ShowYouHowToCatchMons`,
runs `special StartOldManTutorialBattle`, then `ThatWasEducationalTakeThis`, sets
old man var = 2, `giveitem ITEM_TEACHY_TV`, and shows `WatchThatToLearnBasics`.

| Label | Speaker | Text |
| --- | --- | --- |
| `ViridianCity_Text_ThisIsPrivateProperty` | Old man (var 0: talk, or vanilla road-block coord) | `I absolutely forbid you from\ngoing through here!\pThis is private property!` |
| `ViridianCity_Text_GrandpaHasntHadCoffeeYet` | Granddaughter (var 0) | `Oh, Grandpa!\nDon't be so mean!\pI'm so sorry.\nHe hasn't had his coffee yet.` |
| `ViridianCity_Text_GoShoppingInPewterOccasionally` | Granddaughter (var 1 or more, post-handoff) | `I go shopping in PEWTER CITY\noccasionally.\pI have to take the winding trail in\nVIRIDIAN FOREST when I go.` |
| `ViridianCity_Text_ShowYouHowToCatchMons` | Old man (var 1, post-handoff) | `Well, now, I've had my coffee, and\nthat's what I need to get going!\pHm?\nWhat is that red box you have?\pAh, so you're working on your\nPOKéDEX.\pThen let me give you a word of\nadvice.\pWhenever you catch a POKéMON,\nthe POKéDEX automatically updates\lits data.\p…You don't know how to catch\na POKéMON?\pI suppose I had better show you\nthen!` |
| `ViridianCity_Text_ThatWasEducationalTakeThis` | Old man (post-handoff) | `There! Now tell me, that was\neducational, was it not?\pAnd here, take this, too.` |
| `ViridianCity_Text_WatchThatToLearnBasics` | Old man (post-handoff) | `If there's something you don't\nunderstand, watch that.\pIt will teach you about the basics\nof being a POKéMON TRAINER.` |
| `ViridianCity_Text_WeakenMonsFirstToCatch` | Old man (var 2) | `Well, now, I've had my coffee, and\nthat's what I need to get going!\pBut I made it too strong.\nIt gave me a headache…\pIncidentally, are you filling your\nPOKéDEX?\pAt first, focus on weakening the\nPOKéMON before trying to catch it.` |

### 10.2 Other Viridian ambient content on the errand path

- Gym old man `ViridianCity_EventScript_OldMan` (34,11), facing up: gym-door
  var 1 gives `ViridiansGymLeaderReturned` (late game). Otherwise
  `GymClosedWonderWhoLeaderIs`, then he faces his original direction.
- Gym door `ViridianCity_EventScript_GymDoorLocked`: coord (36,11), gym-door
  var 0. Lockall, the player faces up, delay 20, `GymDoorsAreLocked`, then the
  player jumps 2 down the ledge. The door bg (36,10) shows the same line.
- Boy `ViridianCity_EventScript_Boy` (16,22) `MSGBOX_NPC`.
- Youngster `ViridianCity_EventScript_Youngster` (33,26): YES/NO. YES gives
  `ExplainCaterpieWeedle`; NO gives `OhOkayThen`.
- Signs: city (20,16), Trainer Tips 1 (23,1), Trainer Tips 2 (20,31), gym (32,10).
- Not transcribed: the Dream Eater tutor fat man (8,26), a shared move-tutor
  script, and the Potion item ball (17,5) behind the cut tree
  (`FLAG_HIDE_VIRIDIAN_CITY_POTION`).

| Label | Speaker | Text |
| --- | --- | --- |
| `ViridianCity_Text_GymClosedWonderWhoLeaderIs` | Gym old man | `This POKéMON GYM is always closed.\pI wonder who the LEADER is?` |
| `ViridianCity_Text_GymDoorsAreLocked` | Gym door coord/bg | `VIRIDIAN GYM's doors are locked…` |
| `ViridianCity_Text_CanCarryMonsAnywhere` | Boy | `Those POKé BALLS at your waist!\nYou have POKéMON, don't you?\pIt's great that you can carry and\nuse POKéMON anytime, anywhere.` |
| `ViridianCity_Text_WantToKnowAboutCaterpillarMons` | Youngster (YES/NO) | `You want to know about the two\nkinds of caterpillar POKéMON?` |
| `ViridianCity_Text_ExplainCaterpieWeedle` | Youngster (YES) | `CATERPIE has no poison,\nbut WEEDLE does.\pWatch that your POKéMON aren't\nstabbed by WEEDLE's POISON STING.` |
| `ViridianCity_Text_OhOkayThen` | Youngster (NO) | `Oh, okay then!` |
| `ViridianCity_Text_CitySign` | City sign (20,16) | `VIRIDIAN CITY \nThe Eternally Green Paradise` |
| `ViridianCity_Text_CatchMonsForEasierBattles` | Trainer Tips (23,1) | `TRAINER TIPS\pCatch POKéMON and expand your\ncollection.\pThe more you have, the easier it\nis to battle.` |
| `ViridianCity_Text_MovesLimitedByPP` | Trainer Tips (20,31) | `TRAINER TIPS\pThe battle moves of POKéMON are\nlimited by their POWER POINTS, PP.\pTo replenish PP, rest your tired\nPOKéMON at a POKéMON CENTER.` |
| `ViridianCity_Text_GymSign` | Gym sign (32,10) | `VIRIDIAN CITY POKéMON GYM` |

## 11. Viridian Mart: `ViridianCity_Mart_Frlg`

Geometry: clerk, local 1, (2,3) facing right behind the counter. Youngster (6,2)
wanders. Woman (9,5) wanders up and down. Door warps (3..5,7).

- `ViridianCity_Mart_OnLoad`: without `FLAG_SYS_POKEDEX_GET`, the
  questionnaire is replaced with counter metatiles at (1,3) and (1,4).
- `ViridianCity_Mart_EventScript_ParcelScene` (`OnFrame`, Mart var 0, so it
  fires on the first entry at any time): lockall, male textcolor, the clerk
  faces down, `YouCameFromPallet`, closemessage. The clerk waits 4×16 frames and
  faces right while the player walks up 4 and faces left (to the counter).
  `TakeThisToProfOak`. Mart var = 1.
  `giveitem_msg ReceivedOaksParcelFromClerk, ITEM_OAKS_PARCEL, 1,
  MUS_RG_OBTAIN_KEY_ITEM` (additem, no bag-full branch). Lab var = 5.
  Releaseall.
- `ViridianCity_Mart_EventScript_Clerk` (lock, faceplayer): Mart var 1 gives
  `SayHiToOakForMe`. Otherwise `message gText_HowMayIServeYou`, `pokemart`
  (Poké Ball, Potion, Antidote, Paralyze Heal), then `gText_PleaseComeAgain`.
- `ViridianCity_Mart_EventScript_Woman` and
  `ViridianCity_Mart_EventScript_Youngster` (`MSGBOX_NPC`).

| Label | Speaker | Text |
| --- | --- | --- |
| `ViridianCity_Mart_Text_YouCameFromPallet` | Clerk | `Hey!\nYou came from PALLET TOWN?` |
| `ViridianCity_Mart_Text_TakeThisToProfOak` | Clerk | `You know PROF. OAK, right?\pHis order came in.\nCan I get you to take it to him?` |
| `ViridianCity_Mart_Text_ReceivedOaksParcelFromClerk` | narration (+ fanfare) | `{PLAYER} received OAK'S PARCEL\nfrom the POKéMON MART clerk.` |
| `ViridianCity_Mart_Text_SayHiToOakForMe` | Clerk (Mart var 1) | `Okay, thanks! Please say hi to\nPROF. OAK for me, too.` |
| `gText_HowMayIServeYou` | Clerk (Mart var 0 after scene, or 2) | `Welcome!\pHow may I serve you?` |
| `gText_PleaseComeAgain` | Clerk (after shop) | `Please come again!` |
| `ViridianCity_Mart_Text_ShopDoesGoodBusinessInAntidotes` | Woman | `This shop does good business in\nANTIDOTES, I've heard.` |
| `ViridianCity_Mart_Text_GotToBuySomePotions` | Youngster | `I've got to buy some POTIONS.\pYou never know when your POKéMON\nwill need quick healing.` |

## 12. Mapping onto the Wayfarer HNS maps

Port sources: `game/data/maps/{PalletTown_hns, PalletTown_RedsHouse_1F_hns,
PalletTown_RedsHouse_2F_hns, PalletTown_House2_hns, PalletTown_Lab_hns,
Route1_hns, ViridianCity_hns, ViridianCity_Mart_hns}`,
`game/src/wayfarer_kanto_opening.c`,
`game/include/constants/wayfarer_kanto_opening.h`, and the spec
`.product/specs/wayfarer-kanto-origin-opening.md`. The working tree was read as
it stood while this was written. Other writers may be changing these files.

Status key: **V** verbatim, **P** paraphrased or rewritten, **O** omitted, **X**
the HNS map shows different non-FRLG content in that slot.

### 12.1 Rival name resolution (`{RIVAL}` and Blue)

- `ExpandPlaceholder_RivalName` (`game/src/string_util.c`), in a Wayfarer build
  (`IS_HNS` = 1, `IS_FRLG` = 0, `IS_WAYFARER` = 1): on a Hoenn-source map it
  returns MAY or BRENDAN. Otherwise it returns `gSaveBlock2Ptr->rivalName`. New
  game (`new_game.c`, `#if IS_HNS`) sets that field to `SILVER`, and the Johto
  naming screen can rename it. **`{RIVAL}` on the Kanto HNS maps therefore prints
  the Johto rival (SILVER), never BLUE.**
- The current port avoids this by hardcoding the literal `BLUE` in every Kanto
  string (for example `BLUE: Gramps! You called for me?`).
- To stay verbatim, each FRLG `{RIVAL}` has to become `BLUE`, either through a
  mechanical substitution when transcribing or through a Wayfarer-owned
  placeholder or buffer. Nothing in the tree provides `BLUE` today. Affected
  labels: `RivalsHouse`, `HiBrothersAtLab`, `HeardYouBattledRival`, every lab
  `Rival*` line, `OakThreeMonsChooseOne`, `OakBePatientRival`,
  `OakTakeTheseWithYou`, `Text_RivalVictory`, `RivalReceivedMonFromOak`, and the
  post-game lines.
- **In-battle name (likely defect):** `BattleStringGetOpponentNameByTrainerId`
  (`battle_message.c`) replaces the trainer name with
  `GetExpandedPlaceholder(PLACEHOLDER_ID_RIVAL)` for class `RIVAL_EARLY_FRLG`,
  whatever the build. `TRAINER_WAYFARER_KANTO_BLUE_*` use that class, so their
  `Name: BLUE` is ignored and the battle most likely reads "RIVAL SILVER". This
  is inferred from source; confirm it in the emulator.

### 12.2 Oak's battle tutorial under Wayfarer

`BATTLE_TYPE_FIRST_BATTLE` is still set, because `RIVAL_BATTLE_TUTORIAL` goes
through `battle_setup.c`. But the Oak/old-man player controller is selected only
when `IS_FRLG` (`battle_controllers.c`), and the damage, stat, Potion and
party-menu hooks are also gated on `IS_FRLG`. Inferred Wayfarer behavior:

- Intro (`ForPetesSake`, `TheTrainerThat`, `TryBattling`), `WinEarnsPrizeMoney`,
  `InflictingDamageIsKey`, `LoweringStats`, `KeepAnEyeOnHP` and both party-menu
  lines are **absent**.
- `HowDissapointing` (loss) and `OakNoRunningFromATrainer` (run) still hook
  through the opponent controller's print-string path, which is not
  `IS_FRLG`-gated, so they probably still appear. A run attempt first prints the
  engine's `PROF. BIRCH: Don't leave me like this!\p`
  (`STRINGID_DONTLEAVEBIRCH`).
- `SetUpBattleVarsAndBirchZigzagoon` creates a Zigzagoon when
  `!IS_FRLG && FIRST_BATTLE`, but the trainer party is created afterwards and
  overwrites it.
- The AI uses `AI_FLAG_FIRST_BATTLE`.

None of this was run; confirm it in the emulator before writing assertions.

### 12.3 Beat-by-beat host and status

| FRLG beat | HNS host (map, object/event, coords) | Current port | Status |
| --- | --- | --- | --- |
| Bedroom spawn, face north, Pallet respawn | `PalletTown_RedsHouse_2F_hns`, origin entry (spec) | Origin launcher (not in these files) | n/a |
| NES `PlayedWithNES` | 2F bg (4,2) `PalletTown_RedsHouse_2F_EventScript_NES` | `{PLAYER} played the NES.\pBetter get going--no time to lose!` (not origin-gated) | P |
| Posted notice `PressLRForHelp` | 2F: no bg; needs a new wall bg event | none | O |
| Bedroom PC boot and item storage | 2F bg (2,1) `..._EventScript_PC` | `It looks like it hasn't been used\nin a long time…`, no PC menu | X |
| Mom gender send-off | 1F Mom (7,4) `PalletTown_RedsHouse_1F_EventScript_Mom` | `MOM: All kids leave home someday.\pPROF. OAK is waiting to help you\nbegin your journey, {PLAYER}.` (one gender-neutral line) | P |
| Mom heal after battle | same Mom | `MOM: Your POKéMON is counting on\nyou, {PLAYER}.\pCome home whenever you need to\nrest.`, no heal, gated on starter not battle | O |
| TV (gender, wrong-side) | 1F bg (4,1) `..._EventScript_TV` | `They have programs that aren't\nshown in JOHTO…` (not origin-gated) | X |
| Oak "Hey! Wait!" auto-close | `PalletTown_hns` coords (12,1), (13,1) `KantoNorthExit` | none | O |
| Oak grass warning and escort walk | Oak `LOCALID_PALLET_KANTO_OAK` (12,3), walks up 1 | `OAK: Wait, {PLAYER}!\pWild POKéMON live in the grass.\nYou need a partner to be safe.` + `OAK: Come with me to my LAB.\pI have a POKéMON for you.`, then warp to lab (13,14), no walk or door | P |
| Sign lady (7 lines, sign-blocking, route-entrance show) | Beauty (3,11) `PalletTown_EventScript_Woman`, which matches the FRLG lady (3,10); Trainer Tips needs a new bg near FRLG (5,14) and a coord for FRLG (13,2) | `I'm raising POKéMON too.\pThey serve as my private\nguards.` only | P (final line) / O (rest) |
| Fat man | Fat man (19,17) `PalletTown_EventScript_Fatman` | `Technology is incredible!\pYou can now trade POKéMON\nacross time like e-mail.` | P |
| Lab sign | bg (16,16) | `OAK POKéMON\nRESEARCH LAB` (extra line break) | P |
| House signs `{PLAYER}'s house` / `{RIVAL}'s house` | scripts `RedsHouseSign`/`BluesHouseSign` exist but `map.json` has no bg events; FRLG (4,7) and (13,7) | `RED'S HOUSE` / `BLUE'S HOUSE`, unreachable | O |
| Town sign | bg (9,11) (same tile as FRLG) | `PALLET TOWN\pA Tranquil Setting of Peace\n& Purity` | P |
| Trainer Tips sign | none (FRLG (5,14)) | none | O |
| South coast gate | coords (7..10,18..19) `KantoSouthCoast` | `The southern sea can wait.\pI should finish PROF. OAK's\nerrand first.` (HNS-only; FRLG has no coast exit) | n/a (spec-required addition) |
| Daisy pre-escort | `PalletTown_House2_hns` Daisy (5,4), which matches the FRLG seated spot (5,4) | `DAISY: It's a fine day to begin\na journey. Take care, {PLAYER}!` | P |
| Daisy after escort (`HeardYouBattledRival`) | same | After starter receipt the port jumps to HNS grooming (`OfferGrooming`), which FRLG only offers post-game | O + X |
| Displayed Town Map object | House2: no object (FRLG (6,4)) | none | O |
| Bookshelves, Clefairy picture | House2: no bg events | none | O |
| Town Map gift branch (post-handoff) | Daisy | Not in scope for the spec (later Kanto work) | O (deferred) |
| Lab pre-escort rival `GrampsIsntAround` | `PalletTown_Lab_hns` Blue (16,14) `KantoBlue` | `BLUE: Where's Gramps?\pHe said there would be POKéMON\nfor us here.` | P |
| Unopened balls `ThoseArePokeBalls` | Balls (10,18), (16,18), (17,18) | Hidden (`FLAG_TEMP_3`) until the choice phase | O |
| Arrival exchange (4 messages) | Lab: Oak (13,11), Blue | none (`OnTransition` just commits the phase) | O |
| Oak/rival choice-phase talk | Oak, Blue | `OAK: POKéMON are waiting in those\nthree BALLS. Choose your partner.` / `BLUE: Choose already! I want to\nsee how strong my POKéMON is.` | P |
| Per-species confirm YES/NO | Ball objects, `showmonpic` | `{STR_VAR_1}, a {STR_VAR_2}-type\nPOKéMON. Choose this one?` (generic, STR_VAR type) | P |
| NO: silent release | same | `OAK: Take your time. You can\ninspect the others.` | X |
| `OakThisMonIsEnergetic` | Oak | none | O |
| Received mon | narration | `{PLAYER} received a first\nPOKéMON from PROF. OAK!` | P |
| Nickname prompt | shared `gText_NicknameThisPokemon` | none | O |
| Rival walks to counter ball, takes it | Blue, counter ball | `BLUE: I'll take this one!\pWe can test them before you leave.`; all 3 balls removed at once, no walk | P |
| `RivalReceivedMonFromOak` | narration | none | O |
| Last ball `OaksLastMon` | remaining ball | removed | O |
| Leave-without-starter guard | Lab exit coords (13,19), (12,20), (14,20) | Only active at `STARTER_RECEIVED`; before that, leaving hits Pallet's `PROF. OAK is waiting in his LAB.\pI should choose my partner first.` and a warp back | O (+ X) |
| Rival post-starter `MyMonLooksTougher` | Blue | Still `BLUE: Choose already! I want to\nsee how strong my POKéMON is.` ("Choose already!") | X |
| Oak post-starter | Oak | `OAK: Treat your new partner well.\pBLUE has chosen one too.` | P |
| Battle challenge | Lab exit coords | `BLUE: Wait, {PLAYER}!\pLet's see whose POKéMON is\nstronger!` | P |
| Rival defeat / victory | `trainerbattle_earlyrival` texts | `BLUE: What? Unbelievable!` / `BLUE: Yes! My POKéMON is great!` | P |
| Rival parties by counter | `TRAINER_WAYFARER_KANTO_BLUE_{BULBASAUR,CHARMANDER,SQUIRTLE}` | Same species, levels, moves and IVs as FRLG; the counter map matches (Bulbasaur to Charmander, Charmander to Squirtle, Squirtle to Bulbasaur) | V (data) |
| Post-battle `GoToughenMyMon` + heal | Blue | `BLUE: I'll get stronger.\pSee you later, {PLAYER}!`; no `HealPlayerParty` on a win (a loss heals via `HEAL_AFTER`) | P |
| Oak in-battle tutorial | engine | see 12.2 | O (mostly) |
| Oak post-battle | Oak | `OAK: You're ready for ROUTE 1.\pVIRIDIAN CITY is just beyond it.` | P |
| Aides (2× `StudyAsOaksAide`, 1× `OakIsAuthorityOnMons`) | `Assistant2` (9,18) M, `Assistant1` (16,17) F, `Assistant3` (17,10) F | `PROF. OAK studies POKéMON from\nall over the world.` for all three | P |
| Pokédex desk units | none | none | O |
| Lab e-mail computer | bg (10,9) `PalletTown_Lab_EventScript_PC` | HNS Elm e-mail (not origin-gated) | X |
| Lab posters (START, SAVE, types) | none | none | O |
| Route 1 Potion clerk (5 messages, bag-full) | `Route1_hns`: no object; needs a new origin-gated object near the Pallet end | none | O |
| Route 1 ledge boy | none | none | O |
| Route 1 sign | bg (21,31) | `ROUTE 1\pPALLET TOWN - VIRIDIAN CITY` (`\p` for `\n`) | P |
| Route 1 trainers suppressed | Quinn (29,11), Danny (20,27) hidden with `FLAG_TEMP_1` | spec-required; FRLG has none | n/a |
| Viridian old man blocks the road / forbid line | `ViridianCity_hns` Coffee Gramps (25,20) is the closest host; a north-road block needs HNS coords | Coffee Gramps espresso YES/NO (HNS, not gated) | O + X |
| Granddaughter | none | none | O |
| Gym old man | `GymGramps` (34,11), the same tile as FRLG | `The GYM is closed today.\pThere's plenty to see in town.` | P |
| Gym door locked | HNS gym warp (43,14) | not gated here | O |
| Boy, caterpillar youngster, Trainer Tips signs | HNS Youngster (27,30) etc. | HNS texts | O / X |
| City sign | bg (21,21) `CitySign2` | `VIRIDIAN CITY\pThe Eternally Green Paradise` (`\p` for `\n`, no trailing space) | P |
| Gym sign | bg (45,14) | `VIRIDIAN CITY POKéMON GYM\pTemporarily closed.` | P |
| Mart Parcel scene | `ViridianCity_Mart_hns` `OnTransition`+`OnFrame` (`VAR_TEMP_1`), clerk (2,3), which matches FRLG | `CLERK: Hey! You came from PALLET\nTOWN, right?\pPROF. OAK's order came in.\nCould you take it to him?` (merges 2 messages, no walk-up) | P |
| Parcel received | narration | `{PLAYER} received OAK'S PARCEL\nfrom the MART clerk.` | P |
| Clerk "say hi" while holding the Parcel | clerk | `CLERK: Please take that straight\nback to PROF. OAK.` once; later the ordinary Cherrygrove clerk | O + X |
| Clerk shop greeting | clerk | `gText_HowMayIServeYou` / `gText_PleaseComeAgain` via the Cherrygrove clerk (inventory differs) | V (text) |
| Mart woman | Woman (9,5) `..._Lass`, the same tile as FRLG | `This shop is stocked with\nsupplies for new trainers.` | P |
| Mart youngster | Youngster (6,2) `..._Cooltrainer`, the same tile as FRLG | `Have you been to CINNABAR?\pIt's an island way south of here.` (not gated) | X |
| Delivery: `OakHaveSomethingForMe` | Oak (13,11) `KantoParcel` | `OAK: Ah, {PLAYER}!\pIs that the PARCEL I ordered?` | P |
| `DeliveredOaksParcel` | narration | none | O |
| `OakCustomBallIOrdered` | Oak | `OAK: Wonderful! This is what I\nwas waiting for.` | P |
| `RivalGramps` + `RivalWhatDidYouCallMeFor` | Blue from door (13,19), walks up 4 | `BLUE: Gramps! You called for me?` | P |
| `OakHaveRequestForYouTwo` + `OakPokedexOnDesk` | Oak | `OAK: I have a request for you two.\pRecord every POKéMON you find\nwith this POKéDEX.` | P |
| `OakTakeTheseWithYou` | Oak | none | O |
| Pokédex received | narration | `{PLAYER} received a POKéDEX\nfrom PROF. OAK!` | P |
| `OakCatchMonsForDataTakeThese` | Oak | `OAK: Here are five POKé BALLS\nto help you begin.` | P |
| Five Poké Balls received | narration | `{PLAYER} received five\nPOKé BALLS!` | P |
| `OakExplainCatching` | Oak | `OAK: When a wild POKéMON is weak,\ntry throwing a POKé BALL.\pKeep exploring and fill the\nPOKéDEX together.` | P |
| `OakCompleteMonGuideWasMyDream` | Oak | none | O |
| `RivalLeaveItToMeGramps` | Blue | none | O |
| `RivalTellSisNotToGiveYouMap` | Blue | `BLUE: I'll get a TOWN MAP from\nDAISY. Don't fall behind!` | P |
| Post-delivery state (old man, Mart 2, rival's house 1, Route 22) | origin state | `PALLET_OPENING_COMPLETE` only | n/a (spec defers) |
| HNS-only retry and closing lines | Oak | `OAK: Your journey starts now.\pGood luck, {PLAYER}!`, `KantoParcelMissing`, `KantoBagFull`, `KantoRewardRetry`, `KantoStarterFull`, `KantoBallUnavailable`, `KantoNoUsableMon` | X (spec retry paths; FRLG has no equivalent) |

## 13. Gap table (first implementation)

Columns: FRLG content, HNS host, gap type (O omitted, P paraphrased,
X wrong or non-FRLG content), note.

| ID | FRLG content (labels) | HNS host | Type | Note |
| --- | --- | --- | --- | --- |
| G01 | `{RIVAL}` placeholder in all rival lines | all | P | `{RIVAL}` resolves to SILVER on HNS; the port hardcodes BLUE and paraphrases. Decide on literal `BLUE` substitution versus a placeholder. |
| G02 | In-battle rival name | `TRAINER_WAYFARER_KANTO_BLUE_*` | X | Class `RIVAL_EARLY_FRLG` forces `PLACEHOLDER_ID_RIVAL`, so SILVER is likely shown (unverified). |
| G03 | Oak in-battle tutorial (11 strings) | engine | O | The controller and 4 hooks are `IS_FRLG`-gated. Loss and run lines probably remain. |
| G04 | `PlayedWithNES` | 2F NES (4,2) | P | HNS GSC line. |
| G05 | `PressLRForHelp` notice | 2F | O | No bg event. |
| G06 | `gText_PlayerHouseBootPC` + BedroomPC | 2F PC (2,1) | X | "hasn't been used" line, no storage. |
| G07 | Mom `AllBoys…` / `AllGirls…` | 1F Mom (7,4) | P | No gender branch. |
| G08 | Mom heal (`YouShouldTakeQuickRest`, `LookingGreatTakeCare`, heal) | 1F Mom | O | Gate should be the battle resolved, not the starter. |
| G09 | TV 3 variants | 1F TV (4,1) | X | Johto TV line, not origin-gated. |
| G10 | `OakDontGoOut` | Pallet north coords | O | Auto-close "Hey! Wait!" line. |
| G11 | `OakGrassUnsafeNeedMon` + escort walk and door | Pallet Oak (12,3) | P | Two HNS lines, instant warp. |
| G12 | Sign lady 6 lines + sign-block + route-entrance show | Pallet Beauty (3,11) | O | Needs a Trainer Tips bg, coord, and lady state. |
| G13 | `RaisingMonsToo` | Pallet Beauty | P | GSC "private guards" text. |
| G14 | `CanStoreItemsAndMonsInPC` | Pallet fat man (19,17) | P | GSC "trade across time" text. |
| G15 | `OakPokemonResearchLab` | bg (16,16) | P | Line break added. |
| G16 | `PlayersHouse`, `RivalsHouse` signs | FRLG (4,7), (13,7) | O | Scripts exist, no bg events, wrong text. |
| G17 | `TownSign` | bg (9,11) | P | "Tranquil Setting" text. |
| G18 | Trainer Tips sign | FRLG (5,14) | O | |
| G19 | Daisy `HiBrothersAtLab` | House2 Daisy (5,4) | P | |
| G20 | Daisy `HeardYouBattledRival` | House2 Daisy | O/X | The port offers grooming instead (FRLG post-game only). |
| G21 | Town Map object, bookshelves, picture | House2 | O | No objects or bg events. |
| G22 | `RivalGrampsIsntAround` | Lab Blue | P | |
| G23 | `ThoseArePokeBalls` | Lab balls | O | Balls hidden before the choice phase. |
| G24 | Arrival exchange (4 messages) | Lab | O | No scene. |
| G25 | `OakWhichOneWillYouChoose`, `RivalGoChoosePlayer` | Lab Oak/Blue | P | |
| G26 | Per-species confirm (3) | Lab balls | P | Generic type preview. |
| G27 | Silent NO release | Lab balls | X | The port adds `KantoChooseAnother`. |
| G28 | `OakThisMonIsEnergetic` | Lab | O | |
| G29 | `ReceivedMonFromOak` | Lab | P | Species name dropped. |
| G30 | Nickname prompt | Lab | O | |
| G31 | `RivalIllTakeThisOneThen` + walk to the ball | Lab Blue | P | |
| G32 | `RivalReceivedMonFromOak` | Lab | O | |
| G33 | `OaksLastMon` remaining ball | Lab | O | All balls removed. |
| G34 | `OakHeyDontGoAwayYet` exit guard | Lab exit coords | O | Replaced by the Pallet north bounce. |
| G35 | `RivalMyMonLooksTougher` | Lab Blue | X | Still says "Choose already!". |
| G36 | `OakCanReachNextTownWithMon` | Lab Oak | P | |
| G37 | `RivalLetsCheckOutMons` | Lab exit | P | |
| G38 | `RivalDefeat`, `Text_RivalVictory` | battle | P | |
| G39 | `RivalGoToughenMyMon` + unconditional heal + exit walk | Lab Blue | P | No heal on a win. |
| G40 | `OakBattleMonForItToGrow` | Lab Oak | P | |
| G41 | Aides (2 texts) | Lab assistants | P | One generic line. |
| G42 | Pokédex desk units `BlankEncyclopedia` | Lab | O | |
| G43 | Lab computer e-mail | Lab PC (10,9) | X | Elm e-mail. |
| G44 | Lab posters (START, SAVE) | Lab | O | |
| G45 | Route 1 Potion clerk (5 messages incl. bag-full) + `FLAG_GOT_POTION_ON_ROUTE_1` | Route1_hns | O | Needs a new object. |
| G46 | Route 1 ledge boy | Route1_hns | O | |
| G47 | Route 1 sign | bg (21,31) | P | `\p` for `\n`. |
| G48 | Viridian old man forbid/road block (`ThisIsPrivateProperty`) | ViridianCity_hns | O | No north-road block; see A2. |
| G49 | Granddaughter `GrandpaHasntHadCoffeeYet` | ViridianCity_hns | O | |
| G50 | Gym old man `GymClosedWonderWhoLeaderIs` | GymGramps (34,11) | P | The FRLG line is already neutral, as the spec requires. |
| G51 | Gym door locked | Viridian gym | O | |
| G52 | Boy, caterpillar youngster, 2 Trainer Tips | Viridian | O/X | |
| G53 | City sign, gym sign | Viridian bg (21,21), (45,14) | P | |
| G54 | Mart `YouCameFromPallet` + `TakeThisToProfOak` + walk to counter | Mart clerk (2,3) | P | Merged into one message. |
| G55 | `ReceivedOaksParcelFromClerk` | Mart | P | "MART clerk" vs "POKéMON MART clerk". |
| G56 | `SayHiToOakForMe` | Mart clerk | O/X | Replaced by `KantoParcelReturn`. |
| G57 | Mart woman `…ANTIDOTES…` | Mart woman (9,5) | P | |
| G58 | Mart youngster `…POTIONS…` | Mart youngster (6,2) | X | HNS Cinnabar line. |
| G59 | `OakHaveSomethingForMe` | Lab Oak | P | |
| G60 | `DeliveredOaksParcel` (+ `MUS_OBTAIN_TMHM`) | Lab | O | |
| G61 | `OakCustomBallIOrdered` | Lab Oak | P | |
| G62 | `RivalGramps`, `RivalWhatDidYouCallMeFor` | Lab Blue | P | Merged. |
| G63 | `OakHaveRequestForYouTwo`, `OakPokedexOnDesk` | Lab Oak | P | Merged. |
| G64 | `OakTakeTheseWithYou` | Lab Oak | O | |
| G65 | `ReceivedPokedexFromOak` | Lab | P | "a POKéDEX … !" vs "the POKéDEX … ." |
| G66 | `OakCatchMonsForDataTakeThese` | Lab Oak | P | |
| G67 | `ReceivedFivePokeBalls` | Lab | P | Line break and "!" differ. |
| G68 | `OakExplainCatching` | Lab Oak | P | |
| G69 | `OakCompleteMonGuideWasMyDream` | Lab Oak | O | |
| G70 | `RivalLeaveItToMeGramps` | Lab Blue | O | |
| G71 | `RivalTellSisNotToGiveYouMap` | Lab Blue | P | |

## 14. e2e fixture check (`e2e/src/fixtures/kanto-frlg-contract-fixture.ts`)

Method: the fixture was loaded with `bun` (107 strings). Each string was compared
with the sources after the fixture's documented normalization: `\n`, `\l` and `\p`
become a newline, `'` becomes `’`, `{PLAYER}` becomes RED, `{RIVAL}` becomes
GREEN, and `STR_VAR` holds species or POTION. 105 matched exactly.

Mismatches:

1. `frlgPalletDialogue.daisy.beforeBattle` uses a straight apostrophe in
   `Grandpa's LAB.`. Every other fixture string uses `’`, and the game renders
   `'` as `’`. An exact comparison will fail.
2. `frlgPalletDialogue.parcelReturn[2]` is `"Ah!\nIt’s the custom POKé BALL!…"`.
   The source `Ah! \nIt's the custom POKé BALL!\pI had it on order.\nThank you!` has a
   trailing space: `Ah! \n`. It only matches if the harness trims line ends.

Semantic notes (the strings match, but the labels or coverage mislead):

3. `daisy.beforeBattle` and `daisy.afterBattle` are gated in source on lab var 0
   versus lab var 1 or more. Var 1 is set by Oak's escort, not by the battle.
   Daisy already says `HeardYouBattledRival` during the starter choice, before
   any battle. The journey's use (fresh intro, then post-battle) is consistent;
   only the names are misleading.
4. `town.oakWait` is a `message` with delay 85 and auto-close, not a `msgbox`.
   The harness must not expect a button-gated box.
5. `frlgNames` (RED/GREEN) are the FireRed presets (`sMaleNameChoices`,
   `sRivalNameChoices` under `FIRERED`). LeafGreen presets differ.
6. Missing from the fixture (FRLG content on the path):
   `RivalGrampsIsntAround`, `gText_PlayerHouseBootPC`, both party-menu Oak lines
   (`gText_OakImportantToGetToKnowPokemonThroughly`, `gText_OakThisIsListOfPokemon`),
   Route 1 `gText_TooBadBagIsFull`, Mart `gText_HowMayIServeYou` and
   `gText_PleaseComeAgain`, all Viridian lines (`ThisIsPrivateProperty`,
   `GrandpaHasntHadCoffeeYet`, gym old man, signs, boy, youngster), and
   post-handoff `OakMonsAroundWorldWait` and `AllMonTypesHaveStrongAndWeakPoints`.
7. `frlgStarterMatrix` (slot 0/1/2 = Bulbasaur/Squirtle/Charmander, ball x
   8/9/10, rival counters) and `frlgLabExitLanes` [5,6,7] match the source. The
   HNS slot enum is ordered differently (`KANTO_STARTER_SLOT_BULBASAUR` 0,
   `CHARMANDER` 1, `SQUIRTLE` 2), so do not reuse FRLG slot numbers as HNS slot
   values.

## 15. Ambiguities and decisions for the owner

- **A1, rival name.** `{RIVAL}` cannot be kept literally on HNS (it prints
  SILVER). Choose between transcribing it as `BLUE` and adding a Wayfarer
  placeholder. The in-battle name (G02) needs a code fix either way.
- **A2, Viridian road block.** The repo's `ViridianCity_Frlg` no longer blocks the
  north road (commit `14f5e09e5c`). Vanilla FRLG blocks it with the lying old man
  and coord (22,11). Decide which is the oracle, and whether HNS Viridian should
  block its Route 2 exit during the Parcel phase. The spec is silent.
- **A3, battle tutorial.** Porting Oak's in-battle tutorial means enabling the
  FRLG controller (or equivalent hooks) for this Wayfarer battle only, which is
  engine work beyond script text.
- **A4, spec conflicts with FRLG branches.** The spec's retry and bag-full paths
  (`KantoBagFull`, `KantoParcelFull` and others) have no FRLG text; FRLG never
  checks bag space for the Parcel or Balls. The spec also requires non-FRLG
  neutral variants in Viridian (gym gramps, sign, Mart NPC); FRLG's own lines
  (`GymClosedWonderWhoLeaderIs`, `GymSign`, `ShopDoesGoodBusinessInAntidotes`)
  already satisfy "does not identify Blue".
- **A5, post-handoff state.** FRLG sets the old man, Mart 2, rival's house 1 and
  Route 22 1 at the handoff. The spec defers Town Map and later beats, so the
  Daisy Town Map branch and the Viridian catching tutorial stay out of this port.
- **A6, geometry.** HNS Pallet, the Mart and several Viridian NPCs share FRLG
  tile coordinates, so reusing them is straightforward. The HNS lab and Route 1
  do not, and the new objects they need (Route 1 clerk and boy, Pokédex units,
  Town Map, posters, notice) require coordinate choices that were not checked
  against the HNS layouts here.
- **A7, speakers.** The ball-confirm lines and `RivalTellSisNotToGiveYouMap` have
  no name prefix. They are attributed by textcolor and object (Oak and rival).

## 16. Port status

The port that followed this inventory closes every gap in section 13 except
the divergences below. `e2e/src/journeys/kanto-frlg-contract.e2e.ts` plays the
opening on foot from a new game on all four appearances, covering all three
starters, all three lab exit lanes, and forced-win, forced-loss, and naturally
fought first battles. It asserts every field message verbatim and in order
(the ROM counts message boxes, so an unasserted message fails the run) and
every first-battle message, Oak's tutorial window included.

Remaining divergences, each deliberate or forced by HNS geometry:

- Daisy's Town Map gift stays out of scope, as the specification's scope
  section requires. After the handoff she keeps her native HNS services, and
  the table's Town Map is text only because Wayfarer has no Town Map object
  graphic.
- The two Pokédex units are FRLG objects, drawn with FRLG's NPC blue palette,
  at (12,9) and (13,9) in front of the lab's back wall. The HNS layout has no
  FRLG-style table there yet, and the right unit covers the START poster's read
  spot until Oak removes both in the handoff. The unchosen Poké Ball stays on
  the table afterwards, as in FRLG. The START and SAVE posters sit on wall tiles, since the HNS tileset has
  no poster art.
- The Trainer Tips sign sits on Pallet's garden fence at (5,11). Viridian's two
  Trainer Tips signs have no free sign tile and are not placed.
- The Viridian Gym door refusal steps the player back instead of FRLG's ledge
  jump, and fires only on a northward approach, because HNS has no ledge there.
- The shared receive-item path adds the engine's put-away line after the
  Parcel ("PARCEL"). This repo's FRLG build shows the same line.
- Oak's in-battle "no running" line is unreachable, as in this repo's FRLG
  build: the engine's trainer-battle run refusal fires first. The journey
  asserts Oak's introduction, damage, win, and loss lines. The stat, HP, and
  party-menu lessons were verified once in the emulator during the engine port.
- A challenge setting that substitutes the starter species uses a generic
  typed preview, because the FRLG confirmation would name the wrong species.
- Paths FRLG never reaches (full Bag, failed gift, interrupted handoff) keep the
  specification's Wayfarer text. `textcolor` has no effect in this engine, and
  the Fame Checker calls are dropped.

