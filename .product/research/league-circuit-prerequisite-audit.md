# Johto-start League circuit route audit

## Evidence scope

The implementation observations below describe commit `a1334dd634`. The
[revised circuit specification](../specs/wayfarer-interregional-league-circuit.md)
removes certification caps and automatic circuit announcements. Those revisions
and the regional fixes identified here are pending implementation. Existing
cap tests and starter announcements are historical implementation evidence,
not requirements for the revised design.

The production circuit uses the existing Johto opening as its only new-game
entry. Kanto and Hoenn new-game starts are future features and do not gate
`WAYFARER_LEAGUE_CIRCUIT_ENABLED`, certification caps, or League eligibility.
The switch defaults to enabled and remains a separate compile-time rollback.

This audit records source-level route and state evidence. It does not claim an
emulator journey, collision walkthrough, or balance playtest.

## Johto entry and interregional travel

The normal Johto starter flow releases the player into the HNS settlement
network and presents the circuit itinerary in `NewBarkTown_Lab_hns`. Its
existing S.S. Aqua progression provides the sole required travel entitlement:

1. `OlivineCity_PortInside_hns` offers the maiden voyage at state 0 without an
   S.S. Ticket and sets `VAR_SSAQUA_STATE` to 1.
2. The existing Aqua reunion grants `ITEM_SS_TICKET`; arrival and
   disembarkation set state 8 and the Kanto visit state.
3. At state 8, Olivine can return to Vermilion with the Ticket. Wayfarer's
   Vermilion selection checks the same state and Ticket, prepares the safe
   Hoenn entry, and warps to Slateport Harbor.
4. The dedicated Slateport attendant checks state 8 and the Ticket, then
   returns the player to Olivine.

The resulting Johto -> Kanto -> Hoenn -> Johto loop is directional by design
and has no badge, League, Machine Part, or future-start requirement. The
League circuit source test covers each of these state and warp links.

## Certification approaches and releases

All 24 badge award paths receive the cap check before a battle, reward, or
one-time state write, including Whitney's deferred reward and Clair's Dragon's
Den reward. Badge origin is not used for the 8, 16, or 24 qualification totals.

At Tier 1 and Tier 2, a qualified player reaches the shared HNS League through
the existing Kanto/Johto approach. The circuit branch bypasses Reception Gate's
local Johto Badge 8 and Ecruteak-theater checks and skips the forced Victory
Road Silver scene. The HNS rooms select the pending Kanto or Johto authored
tier and explicitly reject Hoenn and a completed circuit.

At Tier 3, the completed Aqua loop puts any Kanto or Johto threshold route
back at Slateport; the existing Hoenn approach leads through Ever Grande and
Victory Road. The Ever Grande entrance replaces its local badge predicate with
the circuit's Hoenn eligibility predicate. At 24 badges every regional badge
is present. Field-move preparation still matters: `src/field_move.c` permits
Waterfall without a badge predicate, while Dive uses the regional authorization
provided by Steven's reward. Badges alone are not travel credentials.

The Hall of Fame handoff records the explicit cleared tier instead of deriving
one from the shared HNS map. Kanto and Johto continuations recover at Indigo
Plateau Center; Hoenn recovers at Ever Grande League Center. The source test
checks both continuation anchors and the HNS/Hoenn Hall of Fame record calls.

## Retained regional campaign conditions

Sootopolis Gym's outer door still requires
`FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE`. That condition governs obtaining Juan's
regional badge, including when Juan is the twenty-fourth badge; it does not
strengthen the Hoenn League predicate after the badge exists and is unrelated
to regional start selection. The late Hoenn ocean approach also still relies on
the normal prepared Dive and Waterfall field-move journey.

These are existing regional campaign and field-use conditions. They remain
within the revised scope, subject to the recoverability requirements below.

## Free badge collection findings

A read-only audit of all 24 initial badge paths found no additional confirmed
Champion requirement after the explicit cap checks are removed. This does not
prove every route through the maps. The following script dependencies need
implementation work or regression coverage:

- Chuck, Jasmine, and Pryce schedule the Rocket takeover only when
  `VAR_NUM_BADGES == 7`. Kanto awards increment the same counter. A seventh
  badge from another Gym, followed by these three awards above seven, misses
  the event. See `CianwoodGym_hns/scripts.inc:82`,
  `OlivineCity_Gym_hns/scripts.inc:253`, and
  `MahoganyTown_Gym_hns/scripts.inc:99`. The event in
  `Mahoganytown_hns/scripts.inc:19` removes the Radio Tower policeman, whose
  coordinates overlap the upstairs warp in
  `GoldenrodCity_RadioTower_2F_hns/map.json:123`. Tower completion advances
  Mahogany to state 17, releasing the normal eastern route. The missed event
  is confirmed in source; a complete navigation softlock has not been
  reproduced in an emulator. Use explicit regional prerequisites and a
  repeatable eligibility check that cannot regress completed story state.
- Blue's Cinnabar interaction currently requires exactly 15 HNS badges
  (`CinnabarIsland_hns/scripts.inc:36`). Hoenn awards do not increment that
  counter. The revised decision removes this condition: meeting Blue is enough
  to invite him back to Viridian Gym. His initial dialogue must stop assuming
  the player has completed Johto or become Champion.
- Norman's victory unconditionally relocates Wattson
  (`PetalburgCity_Gym/scripts.inc:406`). If the Dynamo Badge is unearned,
  New Mauville becomes an extra prerequisite before Wattson returns. The
  revised decision keeps him in his Gym until his badge is earned; any later
  relocation must remain available once both badge prerequisites are met.
- Keep the existing unearned-badge branches for Chuck, Blue, and Clair, and
  Blue's check before moving Blaine. They protect initial badge awards from
  legacy rematch, Champion-dialogue, and relocation behavior. They are distinct
  from the certification guards despite sharing the feature switch.
- Whitney and Clair award badges in separate follow-up interactions. These
  must remain resumable after other badge awards, regional travel, and League
  clears, with rewards granted once.

Coherent regional quests remain: Jasmine's medicine errand, Misty's Power
Plant sequence, and Juan's weather storyline are examples. Norman still needs
four Hoenn badges; Clair's Gym still checks the Chuck, Jasmine, and Pryce
badges. League independence does not promise arbitrary Gym order.

Circuit presentation moves entirely to the Trainer Card. Remove the starter
itinerary and badge/clear status popups rather than changing their frequency.
The current formatter and tests cover only the next League; the revised card
must show all three requirements and their locked, available, or cleared states.

The Rating formula remains unchanged. With all 24 badges, successive clear
states produce Ratings 56, 71, 76, and 80, with soft caps 62, 88, 95, and 100.
Hoenn script game-clear flags are namespaced into Hoenn's own saved state;
Kanto and Johto clears do not grant Hoenn postgame access.

## Validation boundary

Deterministic native tests cover aggregation, caps, actual Gym guard arguments,
League clear handoff, continuation warps, Trainer Rating, fixed League parties,
and Chinchou utility moves. Python source tests cover the port progression,
League admission wiring, all 24 Gym guard insertions, and retained HNS
traversal contracts.

Runtime navigation through late Sootopolis, Ever Grande, and Victory Road has
not been emulator-tested in this task. If later route acceptance finds a
collision, elevation, or field-move defect, report that exact route and fix it
within the Johto-start circuit scope. Do not reintroduce a future Kanto or
Hoenn-start dependency.

## Integration note

The fixed League inventory contains the stable Kanto, Johto, and Hoenn roster
IDs and aliases. At the audited commit, ordinary Trainer scaling excludes
these League participants. Their party strength remains a separate future
PRD concern; this revision changes badge access and circuit presentation.
