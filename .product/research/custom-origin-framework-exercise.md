# Custom origin framework exercise

## Evidence and scope

A native Codex runner read the [regional-start PRD](../prds/wayfarer-regional-start-choice.md)
and [implementation specification](../specs/wayfarer-regional-start-choice.md),
then independently designed three hypothetical origins against that contract.
The root agent reviewed and synthesized the designs below, clarifying reward
retries and the unfinished stock-story handoff in the evacuation example.

This is a paper exercise. None of these maps, characters, rewards, or origins
is claimed to exist in the game, and none is approved release content. The
framework itself is a draft. No implementation, emulator journey, balance
test, or save-state test was performed for these scenarios.

The brief required an existing-party origin, an empty-party origin with
acquisition outside a professor/starter-selection scene, and two origins in
the same region. Each design had to account for flags, recovery, travel, and
later contact with the stock regional stories.

## Three starting scenarios

| Hypothetical origin | Entry region | Initial party | Opening | Recovery |
| --- | --- | --- | --- | --- |
| Survey camp | Johto | One expedition partner | Field assignment and a samples decision | Camp healer |
| Storm evacuation | Hoenn | None | Relief work and a rescued Pokémon | Shelter clinic |
| Deckhand's shift | Johto | One work partner | Shipboard cargo incident and disembarkation | Ship infirmary, then a port clinic |

These symbolic origin names are examples, not allocated persistent IDs. The
two Johto scenarios need distinct IDs even though both enter `REGION_JOHTO`.
Neither should inherit New Bark's home, family, Elm setup, or maiden voyage
merely because of that geography. None requires a house, parent, professor,
rival, or conventional starter-selection screen.

### Survey camp

The player is part of a field expedition. They begin at a temporary camp
with their partner already present, inspect a damaged route marker, then
decide whether to return collected samples or leave them with the field crew.
A quartermaster supplies equipment and explains regional travel. The player
can then leave on their own journey.

`ORIGIN_JOHTO_SURVEY_CAMP` supplies a camp map/warp, Johto entry region, a
camp-healer recovery point, and the one-time partner grant. Example state:

| State | Meaning and transition |
| --- | --- |
| `FLAG_SURVEY_BRIEFED` | Set after the initial assignment. Later interaction gives a reminder. |
| `FLAG_SURVEY_MARKER_INSPECTED` | Set by inspecting the damaged marker. |
| `VAR_SURVEY_SAMPLES_STATUS` | Unresolved, returned, or left with the crew. Both resolved outcomes allow departure. |
| `FLAG_SURVEY_KIT_ISSUED` | Set only after the equipment handoff succeeds. |
| `FLAG_SURVEY_RELEASED` | Set once the assignment and equipment handoff are resolved. |

Saving at an allowed checkpoint preserves those facts. On reload, the map
script selects the appropriate scene from them; it does not reinitialize the
profile or grant another partner. A failed equipment delivery is retried at
the quartermaster. The sample decision and already delivered rewards stay
committed. A battle loss uses the camp healer until a newer valid local
recovery point has been established.

The quartermaster enables shared navigation/contact functions without Mom
or Birch. The profile's Aqua eligibility reads `FLAG_SURVEY_RELEASED`; its
port interaction supplies the Ticket once eligible. Actual departure still
requires the Ticket in the Bag. Elm's starter state and the maiden-voyage
state remain untouched.

At New Bark, this player is an established trainer from a different Johto
origin. A visitor handler suppresses household assumptions and offers Elm's
local choice/errand with optional gift delivery. In Hoenn, the profile selects
the existing-party visitor rescue and optional local starter, without the
truck or household opening.

Profile setup, separate identity, owned state, and travel queries are covered
by the draft. Camp content and these transactions are authored work. The
dispatch and first-contact lifecycle for a third origin need greater precision.

### Storm evacuation

The player begins in a Hoenn emergency shelter after a storm, without Pokémon.
They register with relief workers and carry supplies through a safe interior
task. A later scene involves a trapped Pokémon; a ranger entrusts that rescued
Pokémon to the player. Relief logistics supplies shoes, navigation/contact
functions, and Pokédex access. The player leaves when the shelter's safety
checks are satisfied.

`ORIGIN_HOENN_EVACUEE` supplies a shelter map/warp and clinic recovery point.
The acquisition is an authored rescue handoff, not Birch's bag selection.
Example state:

| State | Meaning and transition |
| --- | --- |
| `FLAG_EVAC_REGISTERED` | Relief registration completed. |
| `VAR_EVAC_SUPPLY_RUN` | Not started, carrying supplies, or delivered. |
| `FLAG_EVAC_RESCUE_RESOLVED` | The non-battle rescue scene has finished. |
| `FLAG_EVAC_PARTNER_RECEIVED` | Set only after delivery of the rescued Pokémon succeeds. |
| `FLAG_EVAC_SERVICE_KIT` | The shared service/equipment handoff is complete. |
| `FLAG_EVAC_RELEASED` | The exit into ordinary exploration is open. |

Before delivery, map gates and battle entry checks prevent ordinary encounters
and forced battles. That protection is derived from the profile's actual
progress and usable-party state, including on reload. A local event flag alone
is not evidence that every battle entry path is safe.

A save during the supply task resumes at its last permitted checkpoint. If
partner delivery fails, the ranger retries it without replaying the rescue.
After successful delivery, a later battle loss heals the existing partner
and resumes the current step; it never returns the receipt flag to false or
grants another Pokémon. Partial equipment handoffs need separate receipt
tracking if they can succeed independently.

The profile may permit Aqua travel after relief release and partner receipt,
with its own Slateport Ticket handoff. No Hoenn starter-choice, Birch rescue,
truck, or maiden-voyage milestone is written to grant that access.

The runner proposed bypassing the stock Birch rescue when this player later
visits Littleroot. That exposes an unfinished design decision: the custom
handler must provide a coherent entry into Birch's lab, the local starter
choice that selects the Hoenn rival, and later campaign progression. Simply
suppressing Route 101's rescue does not establish those prerequisites.
Alternatively, the author can retain the ordinary existing-party visitor
rescue as a separate later event. The scenario's own rescue and Birch's event
must remain distinct in either design.

New Bark can use an established-trainer visitor flow, preserving the rescue
partner and existing Pokédex. Declining a Johto starter gift must not undo
the choice needed by Silver. The shelter and rescue content are authored
work; the engine-wide empty-party protection and custom-to-stock campaign
handoff remain specification gaps.

### Deckhand's shift

The player begins aboard a hypothetical docked service vessel during a cargo
incident, with one work partner already present. They inspect a manifest,
resolve the incident with that partner, decide the cargo's disposition, then
receive a navigation device and Ticket from the purser. The opening ends at
a safe disembarkation point. This example keeps the partner; it does not
assume a rental-party or Pokémon-return system.

`ORIGIN_JOHTO_AQUA_DECKHAND` enters a Johto ship-interior map and initially
uses its infirmary for recovery. It is a separately authored vessel, not a
claim that the existing S.S. Aqua quest maps can be reused unchanged.
Example state:

| State | Meaning and transition |
| --- | --- |
| `VAR_DECKHAND_SHIFT` | Briefing, incident, or cleared. |
| `FLAG_DECKHAND_INCIDENT_RESOLVED` | The cargo outcome is committed. |
| `FLAG_DECKHAND_NAV_ISSUED` | Navigation service handoff succeeded. |
| `FLAG_DECKHAND_TICKET_ISSUED` | Ticket delivery succeeded. |
| `FLAG_DECKHAND_DISEMBARKED` | The player reached the port and its local recovery point is active. |

Initial party setup executes once. A failed Ticket grant leaves the player
at the purser with the incident still resolved. Reloading an allowed ship
checkpoint selects the current shift state; a loss uses the infirmary and
preserves completed handoffs.

Before disabling access to the opening ship, disembarkation must establish
the destination port's local recovery point. If the origin's fallback still
names the ship after it becomes unavailable, later recovery could strand the
player. The author needs a supported replacement/fallback policy for that
transition, not just an initial heal-location field.

Its Aqua policy reads disembarkation and Ticket receipt, with actual Bag
possession still checked at departure. It does not set the stock maiden
voyage complete or award its reunion rewards. New Bark uses an established
trainer visitor handler; Hoenn can use the normal existing-party visitor
rescue. Like Survey Camp, this is a Johto origin with no New Bark identity.

## Framework findings

The profile model expresses the three player flows without inventing a Mom,
professor, or starter ceremony. Separating identity from region also prevents
the two Johto profiles from collapsing into the New Bark opening. That is
useful evidence of conceptual flexibility, not proof that registering a
profile alone makes its content work.

The runner identified six areas that need a more precise contract before
claiming general custom-origin authoring is ready:

| Area | What the draft already provides | Missing detail exposed by the exercise |
| --- | --- | --- |
| Regional story dispatch | Profile-owned regional integration and no geography-based native fallback | Named interception points, handler results, and how custom setup yields to later normal campaign progression. Skipping a stock event needs an authored prerequisite alternative. |
| Empty-party openings | No ordinary battles before a valid participant is available | A defined battle-gate interface, coverage of wild/forced/scripted encounters and callbacks, and safe behavior after reload or unexpected entry. |
| Rewards and service handoffs | Exact-once starter examples and profile-owned equipment handoffs | A common transaction contract for arbitrary partner/item/service grants, partial success, full Bag/PC, retry interaction, and saved receipt state. |
| First contact | Origin-owned persistent milestones | Ordering between custom first-contact scenes, reusable visitor scenes, and later stock progression; completion/retry state must not overload stock quest flags. |
| Recovery lifecycle | Required safe initial recovery and precedence for valid local recovery | Replacement before an origin-only location closes, including a still-valid fallback after the transition. |
| Selection and registration | Stable IDs and a launcher separated from the v1 Oak menu | How registered profiles become selectable or remain test-only, how another front end confirms an ID, and which validation runs before custom profile entry. |

The root review distinguishes framework gaps from ordinary authoring work.
Every custom camp, scene, reward, and dialogue branch will still need to be
written. The framework need not generate those. It does need to define where
they attach, which state they may change, and how failure or reload returns
control safely. Arbitrary callbacks alone are not an implementation contract.

These findings are design follow-ups, not implemented fixes or an expansion
of the first release's two playable origins. A future implementation should
use a small test-only custom profile to verify these boundaries before making
broader claims about authoring support.
