# FireRed and LeafGreen open-world regional traversal

PRD: [FireRed and LeafGreen open-world regional traversal](../prds/frlg-open-world-region-traversal.md)
Implemented: Yes

## Scope

This specification defines the ordinary Kanto settlement network that opens
after the player receives a starter and the Seagallop network unlocked by the
Vermilion shakedown. It covers the Viridian, Pewter, Mt. Moon, Cerulean,
Saffron, and Route 12 lane changes; the shakedown and shared Vermilion dock;
the travel-only One Island introduction; all-island ferry service; and the
state isolation needed for early Sevii arrival.

It does not open Indigo Plateau or event-only islands, remove HM requirements
from optional content, redesign the S.S. Anne or Sevii campaign stories, change
wild encounters, ordinary trainer placement or sight range, or provide
emergency recovery after the player loses access to their last native Surf
user.

## Behavior

### Availability and state isolation

The Kanto settlement network becomes available when the opening releases the
player with a starter. No later badge, HM item, key item, payment, forced
scripted or story victory, or campaign flag may close a core land connection.
Only the Route 21 crossing to Cinnabar may require a Pokémon that already
knows Surf.

Opening a lane changes only the object position, coordinate event, or local
transport state needed for that lane. It must not grant an attached item,
finish a battle, or set a broader story state. Reaching a Sevii map early may
set its world-map or visited flag and its normal healing location. It may not
start or complete an island quest, rival scene, team encounter, Pokédex
milestone, or postgame reward.

No encounter uses the sole-lane exception in this specification. Snorlax and
Miguel remain optional because the selected travel lanes go around their
interactions.

Ordinary sight-based trainers may challenge the player on a core route. Keep
their object coordinates, trainer types, and sight ranges unchanged. Their
parties remain authored and static, and their battles do not satisfy or advance
any travel story state.

### Kanto road changes

| Connection | Required implementation |
| --- | --- |
| Viridian north road | For `VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN` values 0 and 1, place `LOCALID_TUTORIAL_MAN` at `(21,8)` with his standing graphics and nonblocking movement. Remove the state-0 turnback at `(22,11)` and the state-1 tutorial trigger at `(22,8)`. Retain the tutorial trigger at `(20,8)` and the old man's interaction so the coffee dialogue, catching tutorial, Teachy TV, Parcel, and Pokédex flow remain available. Do not change the old-man scene variable. |
| Pewter east road | Keep `LOCALID_PEWTER_GYM_GUIDE` at `(42,20)` and retain the escort triggers at `(42,21)` and `(42,22)`. Remove only the triggers at `(42,23)` and `(43,23)`. The lower lane changes no Pewter scene, Brock state, guide flag, or Running Shoes state. |
| Mt. Moon B2F | Remove the coordinate event at `(14,11)`. Keep `LOCALID_MIGUEL` at `(13,11)`, both Fossil objects, and every existing script and flag. Passing Miguel changes no trainer, Fossil, or map-scene state. Speaking to him still starts the original battle, and only victory sets `VAR_MAP_SCENE_MT_MOON_B2F` to 1 and permits the original Fossil choice. |
| Cerulean east and south exits | Remove the transition call to `CeruleanCity_EventScript_BlockExits`. Keep the policeman, Slowbro, and Lass at their base object positions. Do not set `FLAG_GOT_SS_TICKET` or alter Bill, the northern rival, Nugget Bridge, Rocket, robbed-house, or S.S. Anne state. |
| Four Saffron gates | In the state-0 trigger for `Route5_SouthEntrance_Frlg`, `Route6_NorthEntrance_Frlg`, `Route7_EastEntrance_Frlg`, and `Route8_WestEntrance_Frlg`, keep the normal Tea handoff when `ITEM_TEA` is present. Otherwise skip the turnback, use the existing approach movement, allow passage, and set `VAR_MAP_SCENE_ROUTE5_ROUTE6_ROUTE7_ROUTE8_GATES` to 1. After an early crossing, speaking to any guard still accepts and removes Tea once if the player later obtains it. Do not set `FLAG_GOT_TEA`, add Tea, or change another Saffron story flag. |
| Route 12 Snorlax | Move the Snorlax object and the underfoot `ITEM_LEFTOVERS` hidden item from `(14,70)` to `(15,70)`. Keep `FLAG_HIDE_ROUTE_12_SNORLAX`, `FLAG_WOKE_UP_ROUTE_12_SNORLAX`, `FLAG_HIDDEN_ITEM_ROUTE12_LEFTOVERS`, the Poké Flute check, and the encounter script unchanged. Make no Route 16, Route 18, or Cycling Road change. |

Every changed lane must work from both directions before, during, and after its
preserved scene. Saving and reloading on either side must not restore a removed
trigger or advance the attached story.

### Shakedown state

Rename FRLG's unused saved flags `0x4A7` through `0x4AC` as follows:

| Address | Constant | Meaning |
| --- | --- | --- |
| `0x4A7` | `FLAG_SEVII_SHAKEDOWN_STARTED` | The builder accepted the player's help. |
| `0x4A8` | `FLAG_SEVII_SHAKEDOWN_SPOT_1` | The northwest soft spot is complete. |
| `0x4A9` | `FLAG_SEVII_SHAKEDOWN_SPOT_2` | The northeast soft spot is complete. |
| `0x4AA` | `FLAG_SEVII_SHAKEDOWN_SPOT_3` | The southeast soft spot is complete. |
| `0x4AB` | `FLAG_SEVII_SHAKEDOWN_COMPLETE` | The Rainbow Pass reward was delivered. |
| `0x4AC` | `FLAG_SEVII_TRAVEL_INTRO_SEEN` | Celio completed the travel-only introduction. |

These names exist only in `flags_frlg.h`. The same numeric addresses retain
their existing meanings in Emerald and HNS.

The old man at `(36,10)` becomes the port builder. Machop remains at `(35,11)`.
Add three marked background interactions inside the existing lot at `(33,9)`,
`(37,9)`, and `(37,13)`. They are numbered 1 through 3 in the order shown, but
the player may inspect them in any order.

The shakedown behaves as follows:

1. Before acceptance, the builder explains the inspection and offers Start or
   Not now. Declining changes no state.
2. Acceptance sets only `FLAG_SEVII_SHAKEDOWN_STARTED`. Each incomplete marker
   then calls Machop over with a fixed, map-local movement sequence, plays a
   tamping animation and sound, and sets its spot flag after the presentation
   finishes. The player cannot leave the lot or open a menu while the movement
   runs. The sequence must not enable the global NPC-follower feature.
3. An inspected marker gives stable completed dialogue and never replays its
   movement. The builder reports how many spots remain.
4. Once all three spot flags are set, the builder allows turn-in. Before the
   reward presentation, check whether `ITEM_RAINBOW_PASS` can be placed in the
   Key Items pocket. A full pocket leaves the turn-in available to retry.
5. Add `ITEM_RAINBOW_PASS`, verify possession, then set
   `FLAG_SEVII_SHAKEDOWN_COMPLETE`. The sailor certifies the pier and explains
   that the shared dock now serves the Sevii Islands. The completion flag must
   never be set before item delivery succeeds.
6. If the original One Island Center introduction has already advanced its
   scene to 1 or later, also set both Sevii Town Map page flags and
   `FLAG_SEVII_TRAVEL_INTRO_SEEN`. Celio has already introduced the region in
   this case, so the next ferry use may open the full menu immediately.

Reward reconciliation uses possession as the authority:

| Rainbow Pass present | Completion flag set | Required behavior |
| --- | --- | --- |
| No | No | Award it only after all three spots and a successful capacity check. |
| Yes | No | Set the completion flag during builder turn-in and give no duplicate. |
| Yes | Yes | Use completed dialogue and give no duplicate. |
| No | Yes | Treat the reward as still owed. Restore the pass when space is available and keep turn-in retryable otherwise. |

### Shared Vermilion dock

Keep `LOCALID_VERMILION_FERRY_SAILOR`, the two ticket-check triggers, and
`VAR_MAP_SCENE_VERMILION_CITY`. In particular, only the original S.S. Anne
departure may set the Vermilion scene to 3.

When the Rainbow Pass is not available through the shakedown, the sailor and
ticket triggers retain their existing S.S. Anne behavior. Once the player owns
the Rainbow Pass and `FLAG_SEVII_SHAKEDOWN_COMPLETE` is set, both entry paths
use this dispatch:

| S.S. Anne state | Dock behavior |
| --- | --- |
| `VAR_MAP_SCENE_VERMILION_CITY < 3` | Offer S.S. Anne, Sevii Islands, and Cancel. S.S. Anne enters the existing ticket check and boarding flow. Sevii Islands does not run any S.S. Anne script. |
| `VAR_MAP_SCENE_VERMILION_CITY == 3` | Offer Sevii Islands and Cancel. Do not restore the S.S. Anne choice. |

Cancel returns the player to the walkable side of the sailor without changing
state. Selecting the S.S. Anne preserves the Ticket check, ship rival, captain,
Cut reward, departure, and all existing movement. Selecting Sevii Islands never
writes the Vermilion scene, S.S. Anne variables, Ticket flags, or ship object
state.

If `FLAG_SEVII_TRAVEL_INTRO_SEEN` is unset, the Sevii choice always sails to One
Island and does not show another destination. Once that flag is set, it routes
through the existing Rainbow Pass dispatch, including the Mystic Ticket and
Aurora Ticket branches when those branches are independently available.

### Travel-only One Island introduction

The early trip uses dedicated travel states without changing
`VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F`. It must not reuse scene value 5,
which belongs to the original Ruby and Rainbow Pass handoff.

Use `VAR_MAP_SCENE_ONE_ISLAND_HARBOR` value 4 for the automatic early arrival
and value 5 when the travel introduction is waiting for Town Map space. The
harbor and One Island scripts walk the player from the ferry to the Pokémon
Network Center without Bill and enter with value 4. The existing vanilla values
1, 2, and 3 remain reserved for Bill's Cinnabar trip, and 0 remains the neutral
travel value.

Before the travel-only ferry departs Vermilion, set
`FLAG_HIDE_ONE_ISLAND_POKECENTER_BILL`. Keep it set throughout harbor states 4
and 5 and after the travel introduction returns the harbor to state 0. This
keeps Bill absent from the early Center visit and from later ordinary ferry
visits. After Bill's Cinnabar invitation passes all item-capacity preflights,
clear the hide flag immediately before setting the original harbor arrival
state. The vanilla scene can then place and move Bill normally. A failed
preflight must leave both the hide flag and the vanilla arrival state
unchanged.

At Center scene 0, the frame script chooses between three cases:

| Condition | Behavior |
| --- | --- |
| Vanilla harbor state 3 | Run the existing Bill and Celio introduction that starts the Meteorite story. |
| Harbor state 4, shakedown complete, and travel introduction unseen | Run the new Celio travel introduction. |
| Harbor state 5 and travel introduction unseen | Run no automatic scene. Talking to Celio retries the travel introduction. |
| Travel introduction seen and no vanilla arrival pending | Run no automatic scene. Celio remains available for ordinary dialogue. |

The travel introduction uses the existing Celio object. It explains the seven
islands and the Rainbow Pass, then expands the Town Map. It gives no Meteorite,
Tri-Pass, or other item except a Town Map when the player does not already own
one. It does not set `FLAG_SYS_PC_STORAGE_DISABLED`, move or reveal Bill, or
change the One Island Center, Cinnabar, Mt. Ember, Lostelle, biker, Hypno,
National Pokédex, Champion, Ruby, or Sapphire state.

If the player needs a Town Map, Celio checks Key Items capacity before the
introduction commits. A full pocket changes the harbor state from 4 to 5,
releases the player, and leaves the introduction pending without retriggering
the frame script. The One Island sailor offers Vermilion and Cancel while state
5 is active so the player can make room and return. Talking to Celio retries the
same capacity check. After the Town Map is present, set
`FLAG_SYS_SEVII_MAP_123` and `FLAG_SYS_SEVII_MAP_4567`, then set
`FLAG_SEVII_TRAVEL_INTRO_SEEN` and restore the harbor state to 0. This order
makes the introduction retry-safe.

### Permanent Seagallop service

After the travel introduction, every Sevii harbor routes to the existing
two-page `EventScript_SeviiDestinationsPage1` and
`EventScript_SeviiDestinationsPage2` menu when the player owns the Rainbow Pass.
The menu contains Vermilion and One through Seven Island, excludes the current
stop, and retains Cancel and More navigation. The existing ferry presentation,
destination IDs, and arrival coordinates remain unchanged.

Full service is also available through the original path when
`VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F` is at least 5. The new shakedown
flag must not make a debug-added Rainbow Pass or an unfinished reward look like
a valid unlock. A missing Rainbow Pass always sends the player back to the
appropriate incomplete or original branch.

The ferry menu remains available during Lostelle, the biker invasion,
Meteorite delivery, Ruby and Sapphire recovery, and every later island state.
No local quest may replace it with a no-return menu.

### Compatibility with the original Sevii story

Bill's normal Cinnabar invitation remains available after early travel. Before
the boat departs, preflight Key Items space for every missing item that the
forced introduction will deliver: the Town Map, the Meteorite, and the Tri-Pass
when the player has neither pass. A failed preflight leaves the invitation at
Cinnabar and changes no harbor or Center state. A successful trip sets the
original One Island harbor states and runs the original Bill and Celio
introduction. That scene still gives the Meteorite, enters One Island Center
scene 1, disables PC storage for the original detour, and preserves the Lostelle
and Meteorite story.

When the player already owns the Rainbow Pass:

- the introduction does not give a Tri-Pass or replay either Town Map page;
- the existing Rainbow Pass continues to authorize the full ferry menu during
  the detour;
- the introduction sets both Sevii map-page flags and
  `FLAG_SEVII_TRAVEL_INTRO_SEEN` when the shakedown is complete;
- the Meteorite is delivered only after a capacity check, and a failed check
  leaves Bill's Cinnabar trip available to retry;
- the later Ruby handoff removes the Ruby, advances the Center to scene 5, and
  sets `FLAG_SYS_SEVII_MAP_4567`, but does not remove a nonexistent Tri-Pass or
  add a duplicate Rainbow Pass.

For a save that reaches the original story without the shakedown, the existing
Tri-Pass and later Tri-Pass-to-Rainbow-Pass conversion remain unchanged. Item
delivery must complete before the corresponding scene value advances in either
path.

### Early island arrival

Ordinary island transition scripts may set their existing world-map flags,
`FLAG_VISITED_TWO_ISLAND`, and healing locations. They may initialize only the
baseline objects and transport needed to enter, heal, save, reload, and leave.

Two Island's Joyful Game Corner uses scene value 5 as an early-travel deferred
state. On transition, change scene 0 to 5 when the One Island Center is below
scene 1. Scene 5 suppresses the automatic Lostelle opener and gives the Daddy
and attendant neutral, pre-quest dialogue. It must not clear
`FLAG_HIDE_THREE_ISLAND_LONE_BIKER` or change
`VAR_MAP_SCENE_THREE_ISLAND`. Once the One Island Center reaches scene 1 or
later, a Game Corner transition changes deferred scene 5 back to 0 so the
original Lostelle opener runs normally and advances the original states.

`ThreeIsland_Port_OnTransition` clears `FLAG_SYS_PC_STORAGE_DISABLED` and sets
`FLAG_SEVII_DETOUR_FINISHED` only when the One Island Center is at scene 1 and
the storage-disable flag is currently set. Direct early ferry travel therefore
changes neither flag. Later transitions after the vanilla detour has already
finished are idempotent.

Four Island and the Six Island Pokémon Center are two locations for one rival
scene. They defer it until the original full-Sevii prerequisite is met:

- If the local rival scene is 0 while the One Island Center is below scene 5,
  set the local scene to a new deferred value 2 and keep the rival hidden.
- On a later transition with the local scene at 2 and the One Island Center at
  scene 5 or later, first inspect both rival scene variables. If either is
  already 1, set both to 1 and keep the local rival hidden. Otherwise restore
  only the current location from 2 to 0 and expose its original rival scene.
- Completion at either location sets both
  `VAR_MAP_SCENE_FOUR_ISLAND` and
  `VAR_MAP_SCENE_SIX_ISLAND_POKEMON_CENTER_1F` to 1. The scene therefore plays
  exactly once even when both locations were visited early and deferred. Early
  arrival never advances either variable to 1.

The same isolation applies to Lostelle, the Three Island bikers, Hypno, the
Meteorite, Mt. Ember, Ruby, Sapphire, Rocket Warehouse, Dotted Hole, Tanoby Key,
and postgame Pokédex state. This specification does not open optional inland
content that still requires Surf, Rock Smash, Strength, Waterfall, or a story
credential.

### Native Surf crossing

Route 21 keeps its existing water and encounter tables. The HM field-use and
native learnset specifications allow a prepared Horsea or Krabby to Surf
without HM03 or the Soul Badge. No part of this specification changes rods,
fishing odds, capture supplies, party storage, terrain, or learnsets.

Acceptance covers a player who prepares a native Surf user before each Pallet
to Cinnabar crossing. It does not guarantee recovery when the player lacks an
Old Rod or Poké Balls, has no party space, or loses access to the last Surf
user.

### Validation

Static checks must verify the exact trigger removals and object moves, the six
FRLG-only flag assignments, item-before-flag reward ordering, dock dispatch,
full-menu reachability, early-island scene guards, and unchanged coordinates,
trainer types, and sight ranges for ordinary sight-based trainers on changed
maps.

Build both FireRed and LeafGreen ROMs and run `make -C game check`. Then use a
fresh save immediately after the starter to complete this journey without
badges, HM items, unrelated campaign completion, or a required scripted or
story victory. Ordinary sight-based trainer battles are permitted. Surf may be
known only for the Route 21 crossing:

1. Visit and leave all ten Kanto settlements named by the PRD.
2. Cross each changed Kanto lane in both directions before its original story,
   while that story is active where applicable, and after completing it.
3. Trigger the retained Viridian and Pewter interactions, battle Miguel and
   take each Fossil on separate saves, hand Tea to a guard after an early gate
   crossing, and wake Route 12 Snorlax. Confirm every preserved reward occurs
   once.
4. Complete the three shakedown spots in every order. Save and reload after
   each spot, exercise a full Key Items pocket at turn-in, and inject every row
   of the Rainbow Pass reconciliation table.
5. While the S.S. Anne is present, select Cancel, fail and pass its Ticket
   check, board it, and use Sevii travel. Confirm neither branch changes the
   other's state. Repeat after the ship's normal departure.
6. Interrupt the first Sevii introduction with a full Key Items pocket, return
   to Vermilion, make room, and finish it. Confirm no Meteorite, Tri-Pass,
   storage-disable flag, visible Bill, or One Island quest state changed. Then
   take the normal Cinnabar trip and confirm its successful preflight restores
   Bill before the vanilla Center scene begins.
7. Use all eight permanent ferry stops in both directions. Save, reload, heal,
   black out, and return from every island without a story check or lost menu.
8. Enter Two Island's Game Corner before the vanilla trip and confirm its scene
   becomes deferred without changing Lostelle or Three Island state. Walk
   through Three Island's port and confirm PC storage and detour-completion
   flags remain unchanged. Start the vanilla detour later and confirm both
   original transitions still occur once.
9. Reach Four Island and Six Island's Center before scene 5 in both visit
   orders and confirm both rival scenes remain pending. Reach scene 5 later,
   restore from Four first on one save and Six first on another, and confirm
   the rival scene plays exactly once in each case. Both scene variables must
   end at 1.
10. After early travel, take Bill's Cinnabar trip, finish the Meteorite detour,
   deliver the Ruby, and complete the later Sevii story. Confirm no duplicate
   Tri-Pass, Rainbow Pass, Town Map, or map-page reward appears.
11. Prepare the named native Surf users on both sides of Route 21 and cross in
    both directions without HM03 or a badge.
12. Repeat the complete suite in FireRed and LeafGreen and confirm identical
    traversal and state behavior.

## References

- [Cross-build story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [HM field-use specification](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](../prds/standard-rod-fishing.md)
