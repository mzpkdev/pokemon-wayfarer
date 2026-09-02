# Emerald open-world regional traversal

PRD: [Emerald open-world regional traversal](../prds/emerald-open-world-region-traversal.md)
Implemented: Yes

## Scope

This specification defines the ordinary settlement network that opens after the
Emerald introduction gives the player a starter. It covers the road changes,
the Route 104 public ferry, the approved native Surf crossings, the Route 111
desert survey, the Mt. Chimney story boundary, the Route 120 Kecleon encounter,
and early arrival behavior needed to reach all fourteen opening-network
settlements in the parent PRD.

It does not open Sootopolis or Ever Grande, reorder Gyms or the main campaign,
remove field moves from optional content, change ordinary trainer placement or
sight range, or change battle and wild encounter scaling.

## Behavior

### Availability and state isolation

The settlement network becomes available when the Route 101 rescue releases the
player with a starter. No later badge, HM item, key item, payment, scripted or
story battle victory, or campaign flag may disable a core connection. Only the
named Route 118 and eastern-sea crossings may require a Pokémon that already
knows Surf.

Opening a lane changes only the collision, object visibility, or local transport
needed for that lane. It must not grant an attached item, finish a battle, or set
a broader story state. Dedicated roadblock hide flags may be initialized or
derived without advancing the scene that would normally set them.

A self-contained encounter may remain on the only lane when all of these rules
hold:

- It is available on first approach and checks no badge, HM item, key item, or
  earlier story state.
- Declining or losing leaves the encounter available to retry.
- Defeating the encounter is unnecessary. Fleeing or another non-victory result
  can resolve it.
- The game commits every required reward and local state transition before it
  permanently removes the blocker.
- A failed commit leaves the encounter and reward available to retry.

All other story actors and scripted battles leave a second visible lane open.
Reaching a map early may set its normal visited state and baseline
presentation. It may not start or complete an unrelated team, Gym, rival,
legendary, weather, or reward scene.

Ordinary sight-based trainers may challenge the player on a core route. Keep
their object coordinates, trainer types, and sight ranges unchanged. Their
future player-relative level scaling is outside this specification, and their
battles do not satisfy or advance any travel story state.

This exception applies only to the Route 120 Steven and Kecleon scene in this
specification. It does not relax any bypass or optional-scene decision in the
FireRed/LeafGreen, HNS, or Johto traversal requirements.

### Core road changes

| Connection | Required implementation |
| --- | --- |
| Oldale to Route 102 | Keep the footprints man at his base object position and disable the west-threshold turnback. Do not set `FLAG_ADVENTURE_STARTED`, a Pokedex flag, or a rival flag. His normal later dialogue and visibility changes remain valid. |
| Petalburg west crossing | Remove the tutorial redirect at `(8,13)` so one continuous lane reaches Route 104. Retain the triggers at `(8,10)`, `(8,11)`, and `(8,12)` for Norman and Wally's tutorial. Crossing the free lane changes no Petalburg story state. Verify that the later Scott triggers at `(4,10)` through `(4,13)` do not recreate the roadblock. |
| Petalburg Woods northbound path | Remove the Devon employee trigger at `(27,23)` and retain the story trigger at `(26,23)`. Using the travel lane does not advance the local Aqua encounter. |
| Route 110 Aqua wall | Start a new game with only `FLAG_HIDE_ROUTE_110_TEAM_AQUA` set. The museum encounters and Devon Goods delivery remain unfinished. Their later attempt to set the same hide flag is harmless. |
| Route 110 rival | Remove the rival trigger at `(35,56)` and retain `(33,56)` and `(34,56)`. The battle retains its original victory state and Itemfinder reward. |
| Route 111 northbound road | Move one of the two Rock Smash rocks away from the two-tile choke. Leave the other rock as an optional field-move object without allowing it to occupy the open lane. |
| Route 119 bridge | Start a new game with only `FLAG_HIDE_ROUTE_119_TEAM_AQUA` set. The Weather Institute occupation and Shelly battle retain their original state and rewards. |
| Route 119 rival | Remove the rival trigger at `(26,31)` and retain `(25,31)` for the battle and Fly reward. The second lane crosses without starting or completing the encounter. |
| Lilycove east outlet | The city load script never installs the twelve blocking Wailmer metatiles. It does not set `FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE` or change the submarine, Aqua Hideout, Mt. Pyre, or Slateport theft state. |

Each lane must work from both directions in every state that previously created
or removed its blocker. Saving and reloading on either side must not restore the
blocker or advance its story.

### Route 104 public ferry

A dedicated deckhand stands outside Briney's Cottage beside the existing Route
104 boat departure. Matching fixed contacts stand at the existing Dewford and
Route 109 landing areas used for Slateport. The fixed placements are:

| Stop | Contact | Player arrival |
| --- | --- | --- |
| Route 104 | `(14,51)`, elevation 4, facing down | `(14,52)`, facing up |
| Dewford | `(13,9)`, elevation 3, facing down | `(13,10)`, facing up |
| Route 109 for Slateport | `(22,24)`, elevation 3, facing down | `(22,25)`, facing up |

These contacts are separate from Briney and use no Briney, Peeko, Letter, Devon
Goods, or Slateport campaign state.

The Route 104 placement keeps `(13,51)` clear for Briney's legacy warp and
keeps his movement lane at `x = 12` clear. The deckhand must remain present and
nonblocking while Briney's original departure sequence runs.

Every contact offers the same menu:

1. Route 104
2. Dewford
3. Slateport
4. Cancel

Choosing the current stop reports that the player is already there and returns
to the menu. Choosing another stop uses the existing sailing presentation and
places the player on walkable ground beside that stop's contact. Cancel closes
the menu without movement or state changes.

The service is free and available on first approach. It checks no item, badge,
HM, party move, or story flag. Arrival sets only the destination's normal visited
state. Briney's original ferry and story scripts remain independent and may
appear, disappear, or change destinations without changing the public service.
The service must not call `EventScript_BackupMrBrineyLocation` or write
`VAR_BRINEY_LOCATION` or `VAR_BOARD_BRINEY_BOAT_STATE`. If it reuses the sailing
presentation, that presentation is extracted into a state-free helper.

### Lavaridge and the Mt. Chimney story

The two Route 112 grunts keep their original dialogue and
`FLAG_HIDE_ROUTE_112_TEAM_MAGMA`, but move off at least one ordinary cable-car
lane. Talking to them changes no state. Both cable-car attendants offer travel
whenever approached; `VAR_CABLE_CAR_STATION_STATE` continues to coordinate only
the ride and arrival presentation.

Mt. Chimney has three derived states:

| State | Team visibility and traversal |
| --- | --- |
| Before `FLAG_MET_ARCHIE_METEOR_FALLS` | `FLAG_HIDE_MT_CHIMNEY_TEAM_AQUA` and `FLAG_HIDE_MT_CHIMNEY_TEAM_MAGMA` are set. The summit conflict is absent, Maxie cannot be challenged, and the route to Jagged Pass is open. |
| After Meteor Falls and before `FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY` | Both team hide flags are clear. The original grunts, Tabitha, Archie, and Maxie scenes are available. The four team and Poochyena actors above the Jagged Pass warps stand outside the descent lane. |
| After `FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY` | Both team hide flags are set. The original aftermath, Professor Cozmo state, Lava Cookie seller, and optional Meteorite recovery remain unchanged. Jagged Pass stays open. |

New games initialize both Mt. Chimney team hide flags. The Meteor Falls theft
scene continues to set `FLAG_HIDE_ROUTE_112_TEAM_MAGMA`,
`FLAG_MET_ARCHIE_METEOR_FALLS`, and its other original state. It also clears both
team hide flags to reveal the summit conflict. Loading Mt. Chimney derives the
two team hide flags from the table above so an existing save cannot expose
Maxie's scene before Meteor Falls or restore it after his defeat.

Opening the cable-car lane never sets the Meteor Falls or Maxie completion flags.
Taking the Meteorite remains optional and still requires Maxie's defeat.

### Route 111 desert survey

Repurpose Emerald's unused saved flags `0x264` through `0x267` as the survey
started flag and the three stake flags. Their names become Route 111 survey
constants in Emerald's `flags.h`. Do not add or move saved flag storage. The
FireRed/LeafGreen and HNS flag tables keep their existing meanings at the same
numeric addresses; Emerald survey scripts are compiled only into the Emerald
map set and never reference those build-specific names.

The survey proceeds as follows:

1. The surveyor at the southern desert boundary offers the job without checking
   a badge, HM item, field move, item, battle, or story flag.
2. After acceptance, the three numbered stakes on the non-desert side of Route
   111 may be inspected in any order. Each sets its own flag once and gives
   stable repeat dialogue.
3. Before all three are recorded, the surveyor states how many remain and does
   not give a reward.
4. After all three are recorded, the surveyor gives `ITEM_GO_GOGGLES`. The game
   sets `FLAG_RECEIVED_GO_GOGGLES` only after the item is in the Bag. A full Key
   Items pocket leaves the turn-in available to retry.
5. If the player already owns the Go-Goggles, the surveyor recognizes them, sets
   `FLAG_RECEIVED_GO_GOGGLES` if needed, and uses the normal completed dialogue
   without giving a duplicate.

The Route 111 desert boundary continues to require the Go-Goggles. The normal
post-Lavaridge handoff checks both possession and
`FLAG_RECEIVED_GO_GOGGLES`. If the player already received the surveyor's pair,
the giver acknowledges it and awards no duplicate while the surrounding scene
continues normally, including setting `VAR_LAVARIDGE_TOWN_STATE` to 2. If the
normal handoff happens first, the survey also awards no duplicate.

Go-Goggles reconciliation uses item possession as the authority:

| Item present | Received flag set | Required behavior |
| --- | --- | --- |
| No | No | Award the item only after the survey or normal handoff qualifies, then set the flag. |
| Yes | No | Set the flag, give no duplicate, and complete the qualifying interaction. |
| Yes | Yes | Use completed dialogue and give no duplicate. |
| No | Yes | Treat the reward as still owed. Check Bag space, restore the item, and leave the interaction retryable if delivery fails. |

### Native Surf crossings

The core water routes use the native Surf behavior defined by the HM field-use
and native learnset specifications. They never require HM03 or a badge, but the
player must prepare a Pokémon that already knows Surf.

- Route 118 connects the Mauville-side bank to the eastern bank. Existing Lotad
  encounters serve the western land network, and existing Wailmer fishing
  encounters serve the eastern land network.
- The existing ocean routes connect Lilycove, Mossdeep, and Pacifidlog. Existing
  Wailmer fishing encounters provide the approved directional coverage.
- The Lilycove load behavior described under Core road changes keeps the ocean
  outlet available without advancing the submarine or Aqua story.

This specification does not change encounter tables, Standard Rod odds, rod
distribution, capture supplies, party storage, terrain, or native learnsets. It
also does not guarantee recovery after the player loses access to their last
Surf user. Sootopolis remains outside the opening network, and native Dive opens
only its optional player-controlled route.

### Route 120 Steven and Kecleon

Steven, the bridge Kecleon, and its shadow remain at their original Route 120
positions. The bridge encounter is the permitted self-contained encounter on
this settlement lane.

The interaction follows this result matrix:

| Player result | Required outcome |
| --- | --- |
| Declines Steven's prompt | Set or retain `FLAG_NOT_READY_FOR_BATTLE_ROUTE_120`, leave every actor and reward available, and allow an immediate or later retry. |
| Cannot receive the Devon Scope | Explain that Bag space is needed, do not start the Kecleon encounter, and leave the scene unchanged. |
| Receives `B_OUTCOME_LOST`, `B_OUTCOME_DREW`, or `B_OUTCOME_FORFEITED` | Use normal defeated-player recovery. Do not set `FLAG_RECEIVED_DEVON_SCOPE`, clear the bridge, or permanently remove Steven or Kecleon. The scene is available on return. |
| Receives any non-defeat outcome | Put `ITEM_DEVON_SCOPE` in the Bag, set `FLAG_RECEIVED_DEVON_SCOPE`, remove Steven, Kecleon, and its shadow, and clear the bridge exactly once. This includes `B_OUTCOME_WON`, `B_OUTCOME_RAN`, `B_OUTCOME_PLAYER_TELEPORTED`, `B_OUTCOME_MON_FLED`, `B_OUTCOME_CAUGHT`, and `B_OUTCOME_MON_TELEPORTED`. Any other defined or future non-defeat outcome follows the same transaction instead of falling through to partial state. |

Bag capacity is checked before the encounter starts, so every non-loss result can
commit the Devon Scope and local completion state together. The received flag is
set only after item delivery succeeds. If the player already owns the Devon
Scope but the received flag is unset, the interaction repairs the local flag and
bridge state without awarding another item or replaying the encounter. The
repair sets the persistent hide state for Steven, Kecleon, and the shadow and
applies the same post-completion metatiles as an ordinary success.

When `FLAG_DELIVERED_STEVEN_LETTER` is unset, Steven uses dialogue that does not
claim the player met him in Dewford. When it is set, his original callback to the
Dewford meeting may remain. The script must support interaction and its movement
from both bridge approaches. Existing fixed-coordinate bridge writes are kept
only if they remain correct for both approaches and every completion result.

The Devon Scope retains its normal use on other invisible Kecleon, including the
one outside Fortree Gym. Route 120 travel does not grant Fortree Gym completion.

### Early settlement arrival

Fortree, Lilycove, Mossdeep, and Pacifidlog must initialize a baseline state when
reached before their original campaign order. Baseline arrival may:

- set only that settlement's visited flag;
- select normal player or NPC graphics;
- initialize visual effects and local puzzle presentation that do not confer
  progress;
- preserve a story scene whose own prerequisite is already satisfied.

Baseline arrival may not reveal or hide team actors as though their campaign
scene completed, alter a Gym result, move a legendary, give an HM or key item,
or advance a rival scene.

Pokemon Center healing, saving, reloading, Fly registration after normal visit,
blackout recovery, and return travel must work for every early settlement.

### Validation

Static and automated checks must verify that each changed trigger leaves its
required lane, only dedicated roadblock flags are initialized, the public ferry
writes no campaign state, the approved native Surf coverage remains available,
survey reward delivery is retry-safe, Route 120 implements every result in its
matrix, and ordinary sight-based trainers on changed maps retain their object
coordinates, trainer types, and sight ranges.

Build the Emerald ROM and run `make -C game check`. Then use a fresh save
immediately after the starter to complete this journey without badges, HM
items, earlier campaign completion, or a required scripted or story victory.
Ordinary sight-based trainer battles are permitted. Surf may be known only for
the approved water crossings:

1. Visit and leave all fourteen opening-network settlements in the PRD.
2. Use all three Route 104 public ferry stops in both directions, save at each
   remote stop, reload, and return immediately.
3. Cross every changed road before its original story, while that story is
   active where applicable, and after completing it.
4. Exercise the three Mt. Chimney states and confirm Maxie cannot be challenged
   before Meteor Falls.
5. Exercise Route 120 from both approaches: decline and retry, fail the Bag-space
   precheck, and inject every defined `B_OUTCOME_*` value. Confirm lost, drawn,
   and forfeited battles leave the scene retryable, while every non-defeat
   outcome performs the complete reward-and-removal transaction.
6. Prepare the named native Surf users, cross Route 118 in both directions, and
   travel among Lilycove, Mossdeep, and Pacifidlog without HM03 or a badge.
7. Reach each eastern settlement early and confirm its Gym, team, rival,
   legendary, weather, and reward state remains at baseline.
8. Confirm Emerald survey flags do not alter the FRLG or HNS meanings at
   addresses `0x264` through `0x267`.
9. Complete every preserved story after first using its travel lane and confirm
   that its battle, item, dialogue, and aftermath occur once.

## References

- [Emerald and Hoenn traversal research](../research/emerald-hoenn-story-traversal-blockers.md)
- [Cross-build story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
- [HM field-use specification](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](../prds/standard-rod-fishing.md)
