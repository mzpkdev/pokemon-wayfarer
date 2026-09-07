# Johto-start League circuit route audit

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
is present, so the native Dive and Waterfall route permissions are available to
a prepared party.

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

These are existing regional campaign and field-use conditions. They are not a
reason to disable the circuit. No source-level route defect was found that
requires a map edit.

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
IDs and aliases. The parallel ordinary Trainer-scaling task must rerun its
generated classification inventory after integrating these League sources.
