# HNS open-world regional traversal

PRD: [HNS open-world regional traversal](../prds/hns-open-world-region-traversal.md)
Implemented: Yes

## Scope

This specification defines the ordinary settlement network that opens after
the HNS introduction gives the player a starter. It covers the nine Johto
settlements through Mahogany, the S.S. Aqua maiden voyage that unlocks Kanto,
the ten named Kanto settlements, permanent Olivine and Vermilion ferry travel,
the Magnet Train restoration, and the approved native Surf crossings to
Cianwood and Cinnabar.

It does not open Route 44, Ice Path, Blackthorn, the League corridor, Mt.
Silver, Alola, or Sinjoh early. It does not change wild encounters, fishing
odds, native learnsets, optional field-move routes, ordinary trainer placement
or sight range, battle scaling, or recovery after the player loses access to
their last Surf user.

## Behavior

### Availability and state isolation

The Johto settlement network becomes available when the player receives a
starter. No later badge, HM item, key item, payment, forced scripted or story
victory, or campaign flag may close a core connection through Mahogany. Only
the Routes 40 and 41 crossing to Cianwood may require a Pokemon that already
knows Surf.

Kanto unlocks through the maiden voyage described below. After that voyage,
the S.S. Ticket alone keeps the Olivine and Vermilion ferry connection open.
No Kanto badge, Power Plant, radio, Snorlax, or Magnet Train state may close
that ferry or the named Kanto settlement network. Only Route 21 to Cinnabar may
require a Pokemon that already knows Surf.

Opening a lane changes only the coordinate event, object position, local
transport state, or baseline regional presentation needed for travel. It must
not finish an attached battle, grant a skipped reward, or synthesize unrelated
story progress. Reaching a settlement early may set its normal visited flag and
healing location. It may not complete a Gym, rival, Rocket, legendary, or
reward scene.

No encounter uses the sole-lane exception in this specification. The retained
Silver battles, Sudowoodo, Snorlax, and other story encounters remain optional
because a separate travel lane stays open.

Ordinary sight-based trainers may challenge the player on a core route. Keep
their object coordinates, trainer types, and sight ranges unchanged. Their
future player-relative level scaling is outside this specification, and their
battles do not satisfy or advance any travel story state.

### Johto road changes

| Connection | Required implementation |
| --- | --- |
| New Bark west exit | Delete the six coordinate events at `(0,12)`, `(0,13)`, and `(0,14)` for `VAR_NEWBARK_TOWN_STATE` values 2 and 4 from `NewBarkTown_hns/map.json`. Do not change the variable. Elm still advances state 2 to 3 for the Mr. Pokemon errand, the Mystery Egg and police sequence still enters state 4, and Mom still enters state 5. The now-unreferenced turnback scripts, movements, and text may be removed. |
| Cherrygrove Silver | Delete only the state-3 coordinate event at `(56,9)`, `CherryGroveCity_EventScript_TriggerSilver_Top`. Retain the events at `(56,10)` and `(56,11)`. The upper row changes neither `VAR_CHERRYGROVE_CITY_STATE` nor `FLAG_HIDE_SILVER_CHERRYGROVE`. Winning the retained battle keeps its original state-4 and hide-flag commit. |
| Route 30 | Make no map or script change. Keep Joey at `(23,25)`, Pidgey at `(23,23)`, and Rattata at `(23,24)` before `FLAG_MOM_VISITED`. The existing east-side lane remains the core route. |
| Route 32 south road | Delete all nine coordinate events at `x = 27..29`, `y = 10` for `VAR_VIOLET_CITY_STATE` values 1, 2, and 4. Keep the guard at `(26,10)` as an optional interaction. He checks the Sprout Tower Silver flag, Violet Gym completion, and Togepi Egg receipt in the existing order and gives the existing conditional dialogue without moving the player. Once all three pass, he gives the Miracle Seed and enters state 5 only after item delivery succeeds. A full Bag leaves the reward and state available to retry. |
| Azalea Silver | Delete only the state-5 coordinate event at `(11,17)`, `AzaleaTown_EventScript_SilverTriggerBot`. Retain the event at `(11,16)`. The lower row changes neither `VAR_AZALEA_TOWN_STATE` nor `FLAG_HIDE_AZALEA_TOWN_SILVER`. Winning the retained battle keeps its original state-6 and hide-flag commit. |
| Ilex Forest | Delete only the Cut tree object at `(32,40)`, which uses `EventScript_CutTree` and `FLAG_TEMP_1`. Keep the Farfetch'd state, master and apprentice events, HM Cut reward, and every other Cut tree unchanged. |
| Route 36 junction | Move `LOCALID_ROUTE36_SUDOWOODO` from `(39,19)` to `(37,17)`. Keep `FLAG_HIDE_SUDOWOODO` and `Route36_EventScript_SudoWoodo` unchanged. The original junction tile stays open before and after the encounter. Declining or running keeps Sudowoodo available; winning or catching keeps the original hide behavior. Replace dialogue that says the tree blocks the road with neutral shrub-clearing text. |

Each changed lane must work from both directions before its original scene,
while that scene remains active, and after completing it. Saving and reloading
on either side must not restore a deleted trigger or advance the attached
story.

### Saved state and Kanto initialization

The Standard Rod specification owns `0x304` through `0x306`. Keep `0x307` as
`FLAG_UNUSED_39` and keep `HNS_UNUSED_COUNT` at 40. Magnet Train restoration
uses the existing `VAR_KANTO_ROCKET_STORY_STATE` and consumes no new content
flag or variable.

Move the following initial visibility defaults from the S.S. Aqua arrival
script to `EventScript_ResetAllMapFlagsHnS`:

- `FLAG_HIDE_COPYCAT_CLEFAIRY_DOLL`
- `FLAG_HIDE_CERULEAN_GYM_TRAINERS`
- `FLAG_HIDE_CERULEAN_CAPE_ROCKET`
- `FLAG_HIDE_CERULEAN_GYM_ROCKET`
- `FLAG_HIDDEN_ITEM_MACHINE_PART`
- `FLAG_HIDE_ROUTE25_MISTY`
- `FLAG_HIDE_ROUTE25_EUSINE`
- `FLAG_HIDE_ROUTE25_SUICUNE`
- `FLAG_HIDE_CELADON_EUSINE`
- `FLAG_HIDE_SEAFOAM_GYMGUY`
- `FLAG_HIDE_VIRIDIAN_BLUE`
- `FLAG_HIDE_ROUTE14_EUSINE`
- `FLAG_HIDE_ROUTE14_SUICUNE`
- `FLAG_MT_SILVER_1F_HIDE_SCIENTIST`

Keep `FLAG_HIDE_MTMOON_SILVER` and
`FLAG_HIDE_FAN_CLUB_CLEFAIRY_DOLL` clear at new game so both optional objects
start visible. Existing reset entries for the Viridian Forest Pichu objects,
the Route 13 boat, and the Fighting Dojo remain unchanged. Do not move any
progress clear, badge write, or story variable assignment into first Kanto
arrival.

### S.S. Aqua maiden voyage

`VAR_SSAQUA_STATE` remains the only voyage state. Give its values these HNS
meanings:

| Value | Meaning |
| --- | --- |
| 0 | Maiden voyage not started. |
| 1 | Maiden voyage boarded and ship actors initialized. |
| 2 | Grandfather has asked the player to find his granddaughter. |
| 3 | Granddaughter escorted back; reunion presentation pending. |
| 4 | Reunion complete; S.S. Ticket delivery pending. |
| 5 | S.S. Ticket committed; one additional Metal Coat delivery pending. |
| 6 | Both rewards committed; arrival announcement pending. |
| 7 | Vermilion arrival announced; disembarkation ready. |
| 8 | Maiden voyage complete; direct ferry service unlocked. |

Apply the following script changes:

1. In `OlivineCity_PortInside_hns`, state 0 offers the maiden voyage without an
   S.S. Ticket. Accepting initializes the existing ship object flags once and
   sets state 1. States 1 through 7 board or resume the maiden voyage without a
   Ticket and without reinitializing its actors. State 8 and later use the
   normal destination menu, whose Vermilion option checks only for the Ticket.
2. In `SSAqua_B1F_hns`, move `LOCALID_SSAQUA_B1F_SAILORLOOKING` at `(28,8)`
   away from the corridor and remove the state-2 coordinate event at `(29,8)`
   that pushes the player back. Stanley at `(2,6)` in
   `SSAqua_RoomNW_hns` remains an optional interaction and battle. His victory
   may retain its trainer and hide flags but must not write state 3. The
   captain-room granddaughter remains reachable directly from state 2.
3. The captain-room scene sets state 3. The `SSAqua_RoomSSE_hns` on-frame
   table runs the reunion presentation only at state 3, then sets state 4
   before attempting either reward. This makes a failed delivery leave player
   control available instead of replaying the cutscene. Change the room's
   reunited actor-layout threshold from state 5 to state 4 so re-entering while
   a reward is pending does not restore the pre-reunion positions.
4. At state 4, the shared reunion reward routine treats an already-owned S.S.
   Ticket as its unique key-item reward being satisfied. Otherwise it
   preflights the Key Items pocket, gives the Ticket, and verifies `VAR_RESULT`.
   Only a present or successfully granted Ticket advances the voyage to state
   5. At state 5, preflight the Items pocket and grant exactly one Metal Coat,
   regardless of how many the player already owns. Verify `VAR_RESULT`, then
   set state 6. A full pocket or failed grant leaves the current state,
   displays make-room dialogue, and releases control. Speaking to the
   grandfather at state 4 or 5 retries only that state's pending reward. At
   state 6 or later, he uses his normal post-reunion dialogue.
5. Remove the later S.S. Ticket grant from Elm in `NewBarkTown_Lab_hns` so the
   maiden reunion is the only Ticket source. Replace its Ticket-specific copy,
   remove `giveitem ITEM_SS_TICKET`, and do not clear
   `FLAG_HIDE_SSAQUA_1F_GRANDPA`. Preserve the unrelated late-story commits to
   `VAR_NEWBARKTOWN_LABSTATE` and `FLAG_HIDE_OLIVINE_PORT_OAK`.
6. In `PokemonLeague_HallOfFame_hns`, remove the
   `VAR_SSAQUA_STATE = 0` write from the first Johto League clear. The League
   must not change any voyage state. Preserve the other first-clear effects.
7. In `SSAqua_1F_hns`, delete the Kanto progress flag heap from `LeaveBoat`.
   Run the arrival announcement at state 6 and set state 7 after it finishes.
   The door sailor permits disembarkation at state 7. Disembarking sets only
   `FLAG_VISITED_KANTO`,
   `FLAG_VISITED_VERMILION_CITY`, and state 8 before warping to Vermilion port.
   It never writes `VAR_NUM_BADGES`, badge flags, Gym state, Rocket state,
   radio state, Snorlax state, Machine Part state, Copycat state, or another
   settlement's visited flag.
8. In `VermilionCity_PortInside_hns`, remove the
   `FLAG_RETURNED_MACHINE_PART` ferry gate. At state 8 or later, the Olivine
   destination checks only for the S.S. Ticket. Repeat travel never resets or
   replays the granddaughter story.

Whiteout during states 1 through 7 may return the player to Olivine, but the
resume branch above must always let that state reboard. A save and reload at
state 4 or 5 must permit that state's missing reward to be retried through the
grandfather.

### Kanto regional map presentation

`FLAG_VISITED_KANTO` selects both the combined Johto and Kanto map layout and
its matching location entries. The generated `gRegionMapEntries` table holds
the Johto-only HNS coordinates. In `GetActiveRegionMapEntries`, return it while
the flag is clear, and return the static combined table
`sRegionMapEntries_JK` after the flag is set. Keep the matching
`GetRegionMapType` and `GetMapSecIdAt` selection. The layout and location-entry
selection must never use opposite flag branches.

Before Kanto unlock, the region map, player marker, cursor, Fly map, and
Pokedex area display use the Johto-only coordinates. After unlock, all of them
use the combined coordinates. Only Vermilion becomes a newly available Kanto
Fly destination on first arrival because no other Kanto visited flag is set.

### Kanto land connections

#### Mt. Moon Silver

Remove the two on-frame entries that force the Silver scene for
`VAR_PEWTER_CITY_STATE` values 0 and 1. On the first cave transition, advance
state 0 to 1 without movement, dialogue, or battle. Move Silver from `(9,12)`
to `(9,11)`, face him down, and assign `MtMoon_Cave_EventScript_Silver`
directly to the object.

Interaction presents a yes or no battle offer. Declining, losing, or leaving
the cave keeps state 1 and Silver visible. Winning keeps the original
`FLAG_HIDE_MTMOON_SILVER` and Indigo Plateau Silver reveal changes, then sets
state 2. Rewrite the interaction as a stationary lock, face, offer, and battle
sequence; do not reuse the forced scene's movement paths, which assume Silver
starts at `(9,12)`. On victory, fade and remove him before committing the
original flags and state. Crossing the cave never invokes the interaction
script.

#### Existing open routes

Do not edit the Route 6 to Saffron or Route 21 to Cinnabar map geometry. The
Route 6 gate officer already stands beside the lane, and the engineer guards
only the redundant Underground Path entrance. The Route 21 water connection
is already continuous. Acceptance, rather than a map change, proves each
route works with its unrelated campaign state unset.

#### Cycling Road loan

Both the Celadon Route 16 and Fuchsia Route 18 gate coordinate events continue
to call the shared `Gate_CeladonCity_Route16_TriggerScript`. Replace its
Bicycle-item denial with this symmetric behavior:

- When entering and `FLAG_SYS_CYCLING_ROAD` is clear, offer the road bicycle.
  On acceptance, call the existing `ForcePlayerOntoBike` special if the player
  is on foot, set `FLAG_SYS_CYCLING_ROAD`, and move the player onto the road.
- When exiting and the system flag is set, check `ITEM_BICYCLE`. If the item is
  absent, force the avatar onto foot and restore normal map music before
  moving through the gate. If the item is present, preserve the owned riding
  state. Always clear `FLAG_SYS_CYCLING_ROAD` on exit.
- Never add or remove `ITEM_BICYCLE` and never write `FLAG_RECEIVED_BIKE`.
  The system cycling flag plus absence of the Bicycle identifies a loan, so no
  persistent loan flag is required.

Add a narrow registered special for the forced on-foot transition if no
existing script-callable special performs both the avatar and music reset.
The existing Fly, Teleport, Dig, Escape Rope, and whiteout reset paths already
force the avatar to foot and clear `FLAG_SYS_CYCLING_ROAD`; preserve that
behavior. Saving and reloading on Routes 16 through 18 preserves the mounted
loan until a gate or one of those reset paths ends it.

### Magnet Train restoration

This is the self-contained HNS Power Plant theft story. It uses
`VAR_KANTO_ROCKET_STORY_STATE`, `FLAG_HIDDEN_ITEM_MACHINE_PART`, the Machine
Part item, `FLAG_RETURNED_MACHINE_PART`, `VAR_CERULEAN_CITY_STATE`,
`VAR_FAN_CLUB_CLEFAIRY`, the Lost Item, and the Pass. The sequence requires no
badge, HM item, radio progress, Gym completion, or unrelated story flag. Its
required Rocket victory gates only this story, its rewards, and optional
shortcuts. It never closes a core route to a named Kanto settlement.

1. The Saffron station attendant may report the power failure and direct the
   player to the Power Plant while `FLAG_RETURNED_MACHINE_PART` is clear. This
   is optional guidance. It does not write story state or gate the manager.
   Goldenrod may report that the train is down without starting the Kanto
   errand.
2. The Power Plant manager starts the theft sequence directly. At
   `VAR_KANTO_ROCKET_STORY_STATE = 0`, his initial dialogue sets state 1.
   States 1 through 5 use the existing culprit dialogue. State 6 checks for
   the Machine Part, and state 7 or later uses terminal restored dialogue.
3. Keep the `Route10_PowerPlantEntrance_hns` coordinate event at `(6,15)` for
   state 1. The officer reports the Cerulean suspect, sets state 2, and clears
   `FLAG_HIDE_CERULEAN_GYM_ROCKET`. Officer and engineer dialogue continues to
   follow the active Rocket state before completion and the restored state
   afterward.
4. At state 2, entering Cerulean Gym runs the Rocket movement scene.
   The grunt flees, `FLAG_HIDE_CERULEAN_GYM_ROCKET` is set,
   `FLAG_HIDE_CERULEAN_CAPE_ROCKET` is cleared, and the sequence enters state
   3. This scene does not check or complete Misty's Gym.
5. Keep the three Route 24 coordinate events for state 3. They reposition the
   nearby actors and enter state 4. The grunt interaction at state 4 runs the
   battle. A loss leaves state 4 and the grunt available to retry. A
   victory hides the grunt, enters state 5, and clears
   `FLAG_HIDDEN_ITEM_MACHINE_PART` so the Gym pickup becomes available. The
   encounter is on the optional Route 24 branch and does not block travel
   between named settlements.
6. The Cerulean Gym sparkle and both pickup surfaces are active only at state
   5 while `FLAG_HIDDEN_ITEM_MACHINE_PART` is clear and the Machine Part is
   absent from the Bag. The background event at `(0,13)` repeats the same
   checks because an object flag cannot hide it. Give and verify
   `ITEM_MACHINE_PART` before setting the hidden-item flag and state 6. A full
   Key Items pocket leaves state 5, the hide flag clear, and both pickup
   surfaces available to retry.
7. At the state-6 manager turn-in, an existing `ITEM_TM_THUNDER` counts as the
   normal reward already satisfied. Otherwise, preflight its pocket, grant it,
   and verify success before removing the Machine Part. Only after the TM is
   present and Machine Part removal succeeds may the script set state 7,
   `FLAG_HIDDEN_ITEM_MACHINE_PART`, and `FLAG_RETURNED_MACHINE_PART`; clear
   `FLAG_HIDE_ROUTE25_MISTY`; set `VAR_CERULEAN_CITY_STATE` to 2; and set
   `FLAG_HIDE_POWER_PLANT_ENGINEER`. A full pocket or failed item operation
   changes none of these completion states and leaves the handoff retryable.
8. Clearing `FLAG_HIDE_ROUTE25_MISTY` exposes Misty and her date on Route 25.
   The existing state-2 Route 25 scene removes both actors, sets
   `FLAG_HIDE_ROUTE25_MISTY`, clears `FLAG_HIDE_CERULEAN_GYM_TRAINERS`, sets
   `FLAG_HIDE_CERULEAN_GYM_POKEMON`, and advances
   `VAR_CERULEAN_CITY_STATE` to 3. This returns Misty and the Gym Trainers to
   Cerulean Gym without completing or requiring the Gym battle.
9. Keep `FLAG_RETURNED_MACHINE_PART` as the shared power-restoration state.
   Existing consumers retain their original behavior: Power Plant and city
   dialogue switches to restored text, the Lavender Radio director can grant
   the Kanto radio upgrade, Copycat's doll quest becomes available, the Route
   5 and Route 6 Underground Path blockers disappear, and both Magnet Train
   stations recognize restored power. The radio upgrade may then support its
   existing Poke Flute and Snorlax sequence. Neither S.S. Aqua desk may read
   this flag.
10. Copycat retains her existing sequence after the returned-part flag is set:
   state 1 requests the doll, state 2 means the Lost Item is held, and state 3
   means the Pass was awarded. In the Vermilion Fan Club, check space, grant
   and verify `ITEM_LOST_ITEM` before hiding the doll or setting state 2.
11. Returning the Lost Item to Copycat atomically replaces it with `ITEM_PASS`.
   Use a small tested helper if ordinary script commands cannot guarantee the
   swap. On success, the Lost Item is absent, the Pass is present, the Copycat
   doll is shown, and state 3 is committed. On any failure, the Lost Item and
   state 2 remain available for retry.
12. Both station attendants check `FLAG_RETURNED_MACHINE_PART` first and
   `ITEM_PASS` second. Only the true and present combination boards. Keep the
   existing `VAR_TRAIN` arrival animation and warps in both directions.

### Deferred-content invariants

The implementation must not otherwise edit or bypass the Mahogany merchant
that guards Route 44, the Ice Path Kimono scene, the Route 13 Alola boat,
Meara, New Sinjoh, Snowswept Cavern, the League corridor, Mt. Silver,
Vermilion Snorlax, or either Underground Path. The returned-part consumers
described above keep their authored state changes. Source assertions should
pin the late Johto, Alola, and Sinjoh gates that are most vulnerable to broad
state cleanup.

### Native Surf crossings

The core water routes use the native Surf behavior defined by the HM field-use
and native learnset specifications. They never require the HM item or a badge,
but the player must prepare a Pokemon that already knows Surf.

- Routes 40 and 41 connect Olivine to Cianwood. Route 32 land encounters supply
  Wooper, Olivine fishing supplies Chinchou, and Cianwood daytime fishing
  supplies a return-side Chinchou. HNS Krabby does not receive native Surf and
  is not part of the recovery contract.
- Route 21 connects Pallet to Cinnabar. Existing Chinchou fishing encounters on
  the Kanto network provide the approved directional coverage.

This specification does not change encounters, Standard Rod probabilities,
rod distribution, capture supplies, party storage, terrain, or learnsets.
Acceptance covers a prepared player crossing each route in both directions. A
separate traversal-recovery feature owns full-party, no-Ball, no-Rod, blackout,
and lost-last-user recovery.

## Validation

Extend the existing HNS `GameSession` E2E suite and map catalog. Catalog-only
map additions do not require an ABI revision. Add a focused save-and-reload
helper that keeps the test ROM and save isolation so state-4 and state-5 voyage
rewards and an active Cycling Road loan can be verified across reload. Add a
synthetic full-pocket fixture or a narrow unit test for the atomic Lost Item to
Pass helper because the existing eight-entry Bag arrangement cannot fill the
60-slot Key Items pocket.

Use one focused runtime path for the restored Rocket sequence. Cover exhaustive
state guards, full-pocket retry branches, and returned-part consumers with
narrow unit or source assertions instead of separate full-story E2E journeys.

The acceptance suite must cover:

- New Bark in states 2 and 4; all three Cherrygrove rows; Route 30 in both
  directions before Mom; all three Route 32 lanes in states 1, 2, and 4;
  both Azalea rows; the Ilex choke; and Route 36 before and after declining,
  running, winning, and catching Sudowoodo.
- The Route 32 guard with each prerequisite missing, all prerequisites met,
  and a full-pocket reward retry. Each optional Silver battle remains
  available after taking its bypass.
- Maiden boarding at state 0 without a Ticket, reboarding in every state 1
  through 7, optional Stanley, full-pocket failure and retry for the Ticket at
  state 4 and Metal Coat at state 5, both rewards exactly once, arrival at
  state 8, and first-arrival state isolation. A run that begins with one Metal
  Coat must end with two. Elm's later scene must not grant the Ticket.
- Repeat ferry travel in both directions with Ticket present and Machine Part
  unset. Ticket absent denies repeat travel. The voyage must not change badge
  count, Rocket state, radio state, Gym completion, Snorlax state, Copycat
  state, or unrelated visited flags.
- After completing the maiden voyage before the Johto League, finish the first
  League clear. Confirm state 8 and direct ferry travel survive unchanged and
  the maiden voyage does not repeat.
- Exercise the region map before and after `FLAG_VISITED_KANTO`. Confirm the
  selected layout and location-entry table agree, every Johto and Kanto cursor
  position uses the expected coordinates, the player marker matches the
  current map, the Pokedex area display uses the active coordinates, and only
  visited settlements appear as Fly destinations.
- Mt. Moon crossing without interaction, Silver decline and loss without
  state change, and Silver victory with only the original local commits.
- Cycling Road entry from each gate without a Bicycle, save and reload while
  loaned, normal far-gate cleanup, Fly or whiteout cleanup, and an owned
  Bicycle run whose item and riding state remain unchanged.
- The full returned-part flag by Pass-presence matrix at both train stations.
  Both Copycat item handoffs must also cover full-pocket retry without lost
  items, duplicated rewards, or premature state. After completion, every
  Power Plant interaction must remain terminal and must not expose another
  Machine Part.
- The complete `VAR_KANTO_ROCKET_STORY_STATE` progression from 0 through 7:
  manager start, Power Plant officer report, Cerulean Gym escape, Route 24
  staging, grunt battle, Machine Part pickup, and manager turn-in. Run it with
  no badges, HM items, radio progress, Gym completion, or unrelated story
  flags. A grunt loss must preserve state 4 and allow a retry.
- Machine Part pickup and manager turn-in with full pockets, save and reload,
  and successful retry. A full-pocket failure must not hide or lose the
  Machine Part, grant the TM, advance Rocket state, reveal Misty, hide the
  rear-exit engineer, or set `FLAG_RETURNED_MACHINE_PART`.
- Successful manager turn-in must reach Rocket state 7, reveal the Route 25
  Misty scene, remove the Power Plant rear-exit engineer, and set the returned
  part flag. The Route 25 scene must advance Cerulean state 2 to 3, remove
  Misty and her date from Route 25, reveal Misty and the Gym Trainers, and hide
  the temporary Gym Pokemon without completing the Gym.
- Source assertions for each existing `FLAG_RETURNED_MACHINE_PART` consumer
  must cover restored NPC dialogue, the Lavender radio and its Poke Flute path,
  Copycat's quest, Route 5 and Route 6 Underground Path access, and both Magnet
  Train stations. Both S.S. Aqua desks must remain independent of the flag.
- Vermilion through Route 6 to Saffron and onward through Routes 5, 7, and 8
  with Machine Part unset; Pallet through Route 21 to Cinnabar and back with a
  prepared native Surf user; and Olivine through Routes 40 and 41 to Cianwood
  and back with a prepared native Surf user.
- Regression checks that Route 44, Ice Path, Blackthorn, Route 13 Alola access,
  Snowswept Cavern, New Sinjoh, the League corridor, and Mt. Silver retain
  their current progression gates.
- Source assertions that every ordinary sight-based trainer on a changed map
  retains its object coordinates, trainer type, and sight range.

## References

- [Johto story traversal blockers](../research/johto-story-traversal-blockers.md)
- [HNS Kanto story traversal blockers](../research/hns-kanto-story-traversal-blockers.md)
- [Cross-build story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
- [HM field-use specification](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](../prds/standard-rod-fishing.md)
