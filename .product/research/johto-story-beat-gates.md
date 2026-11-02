# Johto story beat gates

Static source audit of Wayfarer's HNS Johto story scripts at `2e70a3ef35`
(2026-11-02), done for the revised
[Johto independent story beats PRD](../prds/johto-independent-story-beats.md).
No ROM, journey, or collision run. Paths are relative to `game/data/maps/`.
This refreshes and partly corrects the
[traversal blocker inventory](johto-story-traversal-blockers.md) and the
[ceremony gate audit](johto-legendary-ceremony-gates.md), both taken at
`9ab8058a6e`.

Build facts: `WAYFARER_LEAGUE_CIRCUIT_ENABLED` is 1
(`game/include/config/league_circuit.h:10`). In the Wayfarer branch of
`game/src/field_move.c`, Surf, Strength, Waterfall, Whirlpool, Fly, and Cut
carry no badge checks.

## Three cross-cutting problems

1. **Adventures share linear state variables, and many writes are
   unconditional.** `VAR_MAHOGANY_TOWN_STATE` carries the Lake of Rage, the
   hideout, Pryce, the RageCandyBar merchant, and the Radio Tower finale.
   `VAR_AZALEA_TOWN_STATE` carries the Well, Bugsy, Silver, Ilex, and the GS
   Ball. `VAR_ECRUTEAK_CITY_THEATER` carries both the Theater Rocket and the
   Kimono ceremony. `VAR_GOLDENROD_CITY_STATE` carries Whitney, the radio
   quiz, and the takeover. A write that assigns a value without checking the
   current one lets one adventure rewind another.
2. **Silver's Wayfarer deferral keys on the starter choice, not on story
   progress.** Every deferred Silver trigger starts with
   `goto_if_unset FLAG_JOHTO_STARTER_CHOICE_COMMITTED → releaseall; end`.
   When a Silver scene carries a host adventure's next step, a player without
   that flag is stuck there. The flag has other readers: Jessica's Tauros
   trade on Melemele (`Melemele_House_hns:67-69`) and the starter preview
   (`game/src/scrcmd.c:2664`).
3. **Town setup runs from one approach road.** Violet's first-visit script is
   a coordinate trigger in the Route 31 gate (`VioletCity_hns:16-46`), and
   resets the Togepi Egg, Sprout Tower, and Ruins of Alph state when it runs
   late. Azalea's runs from Route 33. Goldenrod's first-visit script
   is a coordinate trigger on Route 34, Ecruteak's on Route 37, Olivine's on
   Route 39, and Mahogany's on Route 42, each at town state 0. Entering by
   another road, ship, or train skips it; crossing the trigger later runs it
   then and overwrites live progress. Goldenrod's resets `VAR_LUGIA_OR_HOOH`
   and clears gift flags such as the Bike and Eevee (`GoldenrodCity_hns:4-41`).
   Ecruteak's resets the theater, both legendaries' progress, and the beasts'
   release (`EcruteakCity_hns:19-57`). Olivine's resets Olivine, Cianwood, and
   Suicune progress. Mahogany's also resets Blackthorn and Dragon's Den state
   and `FLAG_GOT_DRATINI` (`Mahoganytown_hns:38-90`).

## Activation gates

| Adventure | Start today | Unrelated gate or hazard |
| --- | --- | --- |
| Slowpoke Well | Reach Azalea, talk to Kurt (`AzaleaTown_KurtsHouse_hns:14-26`), beat Proton. | None to start. Proton sets Azalea state 3 (`SlowpokeWell_B1F_hns:69`) and Kurt sets 4 (`AzaleaTown_KurtsHouse_hns:95`) unconditionally, and Bugsy sets 5 (`AzaleaTown_Gym_hns:542`) unconditionally. Doing the Well after Bugsy, Silver, or Ilex rewinds Azalea; doing Bugsy first skips states 2-4. |
| Burned Tower | Eusine on 1F at Ecruteak state 1 (`BurnedTower_1F_hns:21-36`). Silver's 1F trigger at state 2 drops the player to B1F (state 3), where the beasts release and Morty's gym-door sage is hidden (state 4; `BurnedTower_B1F_hns:12,118,121`). | The Silver trigger is deferred without the Johto starter flag (`BurnedTower_1F_hns:102-103`), so the release and Morty's Gym access wait on Elm's lab choice. |
| Theater Rocket and Surf | Enter the theater at theater state 0; beating the Rocket sets 2; the Surf man then gives HM Surf and sets 3 (`EcruteakCity_Theater_hns:265-293`). | The Surf man's talk script sets 3 whenever Surf is unreceived and state is not 1 (`:258-272`), which can rewind a ceremony state. Elm's Master Ball scene sets theater 4 and hides the theater NPCs unconditionally (`NewBarkTown_Lab_hns:537`). |
| Lighthouse medicine | Jasmine at the lighthouse sets Olivine 3 (`OlivineCity_Lighthouse_hns:255-263`); the Cianwood pharmacist sets 4 below 5 (`CianwoodShop_hns:9-18`); delivery sets 5. | Medicine-first works mechanically, but Jasmine's delivery prompt assumes her request. Cianwood needs Surf. |
| Lake of Rage and hideout | Mahogany's setup, triggered on Route 42 (`Route42_hns/map.json:327-360`), sets state 1 (`Mahoganytown_hns:38-90`); red Gyarados 2 (`LakeOfRage_hns:110`); Lance 3-4; shop raid 5; hideout 6-14 (finale `RocketHideout_B2F_hns:520`). Route 43 toll at states 1-5. | No badge gate. Pryce's win sets 15 unconditionally (`MahoganyTown_Gym_hns:88`), and the Gyarados win sets 2 unconditionally, rewinding 15-17. After a Radio Tower-first run, a later Lake run ends at 14 and nothing restores 17, so the merchant block returns for good. |
| Radio Tower takeover | `MahoganyTown_Gym_EventScript_TryArmRocketTakeover` (`MahoganyTown_Gym_hns:103-112`) arms Elm's call only with badges 5, 6, and 7 and Mahogany state exactly 15 (Pryce's win). It runs from Mahogany OnLoad and the Chuck, Jasmine, and Pryce victory scripts; the call fires in Mahogany, Olivine, or the Cianwood Fly handoff (`Mahoganytown_hns:22-37`). | Chuck, Jasmine's whole errand, and Pryce are unrelated. A Mahogany rewind below 15 before arming stops the takeover from ever arming. The 1F radio quiz sets Goldenrod 2 unconditionally (`GoldenrodCity_RadioTower_1F_hns:143`) and can regress a running takeover (states 6-9). |
| Radio Tower internals | Petrel on 5F gives the Basement Key (state 7); Underground Kimono (8); Underground Silver (9; `GoldenrodCity_UndergroundSwitches_hns:379,415`); Director's Card Key; Archer (10, `GoldenrodCity_RadioTower_5F_hns:160`); Wing; Mahogany 17 (`:248`). | The Underground Silver scene is deferred without the starter flag and leaves state at 8. The Director's Wing depends on `VAR_LUGIA_OR_HOOH` (`:182-183`); finishing the Tower before Baoba's Route 39 choice plays the Wing line but gives nothing, once. |
| Clair and Dragon's Den | Gym boy blocks Blackthorn Gym without badges 5, 6, and 7 (`BlackthornCity_hns:8-16,80-101`). Clair, then the Den; badge 8 recovery is resumable (`DragonsDen_Shrine_hns`). | The badge check is the only gate on Clair. |
| Kimono ceremony | Still Clair → Elm: the Master Ball scene sets theater 4 (`NewBarkTown_Lab_hns:522-537`); an Ecruteak Silver trigger at theater 4 sets 5 (`EcruteakCity_hns:61`, not deferred); then the five dancers, Bell, and ceremony (state 7). Encounters dispatch on legendary progress, not Wing or Bell inventory (`TinTower_RoofDay_hns:5`, `WhirlIslands_LugiaChamber_hns:5`). | Clair, the Den, Elm, and the Ecruteak Silver scene are all unrelated to the ceremony. Reception Gate's theater ≥ 8 check is skipped under the circuit (`ReceptionGate_hns:31-60`). |
| Pewter alternate Wing | `PewterCity_EventScript_Gramps` (`PewterCity_hns:17`) checks only the branch choice and Wing ownership. | No TR, badge, League, or ceremony check. With no branch chosen the script ends with the lock held. A tossed Wing is given again, and each gift resets the legendary's progress to 4 (`:24-41`). |
| Suicune pursuit | Olivine's first arrival via Route 39 sets sightings to 1 (`OlivineCity_hns:~54-66`); Cianwood and Route 42 in either order, then the Kanto half. | No Burned Tower prerequisite. Arriving in Olivine by ship may skip the init (unverified). Eusine greets as if already met (`CianwoodCity_hns:271`). |

Elm's Master Ball handoff fires from Blackthorn state 4 (Clair's Den
cavern), not a conversation. Lance at the Lake appears after the Gyarados win
with no badge check. The Route 32 guard no longer turns players back; his
Miracle Seed still checks the Sprout Tower Silver flag, Violet Gym, and the
Togepi Egg.

## Mahogany road

The RageCandyBar merchant still blocks Route 44: coordinate triggers at
`(30,12-14)` exist for Mahogany states 1 through 16 and none at 17
(`Mahoganytown_hns:95`, all 48 events checked in `map.json`). Buying never
clears it. Only `GoldenrodCity_RadioTower_5F_hns:248` sets 17. The traversal
specification deliberately left it in place
(`.product/specs/hns-open-world-region-traversal.md:339`). Route 44 and Ice
Path are reachable only through this road or from Blackthorn. Routes 46 and
45 connect Route 29 to Blackthorn without scripts, but an earlier audit notes
Route 45's ledges descend southward
(`native-hm-windows/revisions/nearby-access/blackthorn-routes.md:68`), so it
is probably one-way. Unconfirmed.

## Silver's scenes

Silver has no notable-trainer entry (`game/include/constants/notable_trainers.h`)
and no retirement logic. Deferred scenes are skipped until the starter flag is
set, and a scene lost to a state change never returns.

| # | Location | Trigger | Battle | What it claims |
| --- | --- | --- | --- | --- |
| 1 | New Bark | Static NPC (`NewBarkTown_hns:164-181`) | No | First meeting. |
| 2 | Cherrygrove | Coords (56,10-11), city state 3; deferred | Yes | "You got a Pokémon at the lab." |
| 3 | Sprout Tower 3F | Coords, `VAR_SPROUT_TOWER` 0 | No | Elder lecture. |
| 4 | Azalea | Coord (11,16), Azalea state 5; deferred | Yes | Asserts the Well ("You beat them?", `AzaleaTown_hns:349`). |
| 5 | Burned Tower 1F | Coord (15,12), Ecruteak state 2; deferred | Yes | Carries the beast release. |
| 6 | Goldenrod Underground | Coords (26,5-6), Goldenrod state 8; deferred | Yes | "You beat me before"; after-text presupposes Lance's hideout scene (`GoldenrodCity_UndergroundSwitches_hns:530,547`). Inside the takeover. |
| 7 | Olivine | Coords (15,28-30), city state 1 | No | "You again?"; mentions Jasmine at the lighthouse. |
| 8 | Mahogany hideout B3F | Coord (8,13), Mahogany state 7 | No | "Didn't I tell you I'd destroy Team Rocket?" Inside the hideout. |
| 9 | Ecruteak | Coord (39,41), theater state 4 | No | "Don't get cocky just because you defeated Team Rocket!" On the Kimono chain. |
| 10 | Route 22 | Celebi check and Route 22 Giovanni state 0 | No | Giovanni flashback; tied to Celebi. |
| 11 | Victory Road 1F | Coords (27-29,7), Route 27 state 2; deferred | Yes | Lance's lesson. Never fires under the circuit: only the skipped Reception Gate trigger sets Route 27 state 2, and the trigger also skips Silver once `LeagueCircuit_IsKnownThere` (pinned by `game/tools/league_circuit/test_scripts.py:287-294`). A forced fight with no prompt. |
| 12 | Mt. Moon | Static NPC; deferred | Yes | "Since I lost to you..." |
| 13 | Indigo Center | Static NPC after Mt. Moon; daily (`FLAG_DAILY_BEAT_SILVER`); deferred | Yes | Daily rematch. |

The Dragon's Den cavern Silver is dead: nothing clears its hide flag.

## Rocket and Kimono lines that break under reordering

Rocket lines that assert an earlier operation:

- Radio Tower 4F Proton: "Aren't you the one who got in our way at Slowpoke Well?" (`GoldenrodCity_RadioTower_4F_hns:90`).
- Hideout grunt: "I remember you from the well!" (`RocketHideout_B3F_hns:648`).
- Hideout Ariana's retreat: "The broadcast experiment was a total success... much bigger plans" (`RocketHideout_B2F_hns:597`).
- Radio Tower Ariana: "Remember me from the hideout in Mahogany?" (`GoldenrodCity_RadioTower_5F_hns:325`).
- Archer: "I will disband Team Rocket here today" (`:358`), false while the Well or hideout remains.
- Radio Tower 3F grunt: "You made it past our men in the Underground?" (`GoldenrodCity_RadioTower_3F_hns:218`).
- Elm's takeover call: "Maybe Team Rocket has returned. No, that can't be true" (`Mahoganytown_hns:355-369`).

Backstory lines, such as "broken up three years ago" (Proton, Petrel), are
order-safe.

Kimono ceremony callbacks (`EcruteakCity_Theater_hns`):

- Zuki recalls Violet and the Mystery Egg (`:970`).
- Naoko recalls Ilex Forest and the Well (`:995`).
- Miki recalls the Theater Rocket (`:1006`).
- Sayo recalls the Ice Path push and the Dragon's Den test (`:1018`).
- Kuni recalls the Goldenrod Underground and the Radio Tower (`:1028`).

## Corrections to the September inventory

- Field moves were never badge-checked in the Wayfarer branch. The
  inventory's Surf, Waterfall, and Whirlpool badge gates are standalone HNS.
- Under the circuit, Reception Gate does not check the ceremony, and the
  takeover's former exact-seven-badge trigger is gone.
- Traversal #37 had already removed the New Bark exit, Cherrygrove (56,9),
  Route 32 turn-back, Azalea (11,17), and Ilex (32,40) blocks, and moved
  Sudowoodo off the Route 36 junction.
- #75 added the starter-flag Silver deferrals and the visitor Elm path.

## Not confirmed

- Whether Routes 45/46 can be climbed to Blackthorn.
- Whether Baoba's Route 39 choice lane can be avoided, and whether ship
  arrivals in Olivine skip it or the Suicune init.
- Which rewind paths ordinary play reaches.
- Whether a native Surf source makes the Theater HM unnecessary for Cianwood.
- The first Lugia encounter's flee behavior.
