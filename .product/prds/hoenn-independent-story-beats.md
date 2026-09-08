# Hoenn independent story beats

Status: Draft product design. Not implemented. Exact legendary readiness thresholds
and the bounded scene decisions below remain open.

## Intent

Let players complete Hoenn's local adventures in different orders without replaying
Emerald's entire campaign to activate them. Preserve the existing characters,
locations, objectives, battles, rewards, and most dialogue. Keep the main villain
story connected where its events genuinely depend on one another.

An adventure can contain several ordered scenes and span multiple locations.
Independence applies between unrelated adventures, not between every cutscene.

## Design

### Minimal changes and truthful progression

Use existing NPCs to introduce each adventure where the player discovers it. Remove
unrelated badges and campaign flags from its starting requirements. Keep direct
dependencies such as recovering a package before delivering it, or witnessing a
legendary's awakening before the consequences of that awakening.

Preserve original dialogue when it remains true. Use a short first-meeting variant,
conditional callback, or omission when the player has not completed an earlier
event. Do not solve ordering problems with new NPCs, factions, replacement plots,
or a quest-selection interface.

Where the narrative requires a predecessor, keep the successor scene unavailable
until that predecessor finishes. Early visits must still permit ordinary travel
and unrelated interactions. They must not complete unseen events, grant skipped
rewards, or consume a future scene. Returning after a prerequisite is satisfied
makes the next step available without an unrelated activation errand.

### Local adventures

| Adventure | Preserved content | Required independence |
| --- | --- | --- |
| Birch's rescue and introduction | Rescue Birch, visit his lab, and retain the existing optional starter introduction. | Other local adventures do not require this opening. Starter-dependent rival chapters may retain their necessary introduction prerequisite. |
| Peeko and the stolen Devon Goods | Meet the existing employee and Briney, rescue Peeko, recover the Goods, and receive the existing Devon follow-up. | Remove Roxanne's badge as the theft's activation requirement. Keep the rescue locally introduced and internally ordered. |
| Devon deliveries | Deliver the Letter to Steven and the Parts to Stern, including the existing museum confrontation. | Retain the actual items and their original acquisition prerequisites. Once both assignments are available, complete the deliveries in either order. Neither delivery requires the other. |
| Meteor Falls and Mt. Chimney | Discover the meteorite theft, follow it to Mt. Chimney, stop the machine, and recover the meteorite. | Keep this as one ordered adventure, independent of the Devon errands and unrelated Gyms. Preserve the existing return/reward interaction. |
| Weather Institute | Rescue the staff, defeat the occupation, and receive Castform. | Keep the rescue locally available. Its ending must not announce a future Mt. Pyre event that already happened. The nearby rival scene has its own eligibility. |
| Steven and Kecleon | Resolve the Route 120 encounter and receive the Devon Scope. | Preserve its existing early/late introduction and local completion behavior. Steven's Letter is not a prerequisite. |
| Desert survey | Inspect the stakes and receive the Go-Goggles. | Preserve the existing local adventure and duplicate-reward protection with the later handoff. |
| New Mauville | Wattson requests help with the generator and gives the existing reward afterward. | Make the request available locally without Norman's defeat. Preserve access to Wattson's initial Gym challenge throughout the errand. |
| Norman and Wally's introduction | Keep Wally's local introduction and Norman's challenge. | Remove the four-Hoenn-badge requirement for Norman. Retain the local introduction rather than rewriting it. Family dialogue must follow the regional-origin design. |

Early visitors to Steven or Stern receive an appropriate introduction rather than
being credited with a delivery they cannot make. The delivery confrontation and
handover remain unavailable until their actual prerequisite is met. Ordinary
access to the museum, city, harbor, and transport must remain available.

Completing a later harbor event must not delete an outstanding Parts delivery or
its reward. If Stern's existing scene ownership creates a conflict, reconcile it
through his existing interactions rather than requiring an unrelated delivery to
begin the submarine investigation.

### Connected villain adventures

The proposed dependency structure is:

```text
Mt. Pyre -> Magma Hideout
                 |
                 +-> Slateport submarine theft -> Aqua Hideout --+
                 |                                              |
                 +-> Mossdeep / Space Center -> Steven's Dive ---+
                                                                |
                                       Seafloor -> Sootopolis -> Sky Pillar
```

Mt. Pyre introduces the stolen-orb problem and provides the existing Magma Emblem
lead to the hideout. Keep the hideout's Groudon awakening and Maxie encounter.
Neither requires completing the unrelated Devon deliveries, Weather Institute
rescue, or an entire earlier regional itinerary.

After Magma Hideout, the submarine pursuit and Mossdeep episode are sibling
adventures. Players may complete either first, alternate between them, and leave
Hoenn between their steps. Neither branch requires completing the other.

| Chapter | Retained cause and effect | Required change |
| --- | --- | --- |
| Mt. Pyre -> Magma Hideout | The existing Emblem connects the theft to the hideout and Groudon's awakening. | Introduce the adventure locally and remove unrelated activation requirements. |
| Slateport theft -> Aqua Hideout | The stolen submarine leads to the hideout and its escape. | Keep the theft after Magma Hideout. Make the hideout fully playable after the theft without Mossdeep's Gym or Space Center completion. |
| Mossdeep / Space Center -> Steven's Dive | Maxie's rocket-fuel plan follows his failed control of Groudon; Steven helps defeat it and provides Dive. | Keep Magma Hideout as the story prerequisite. Remove any dependency on completing the Aqua branch. Keep the local Mossdeep Gym/invasion sequence and field-use rules. |
| Seafloor -> Sootopolis -> Sky Pillar | Following the submarine leads to Kyogre's awakening, the weather crisis, and seeking Rayquaza's help. | Require both completed branches before starting Seafloor's story. Preserve the existing Cave of Origin, Wallace, Sky Pillar, and return-to-Sootopolis sequence. |

Space Center should not be independently available before Magma Hideout. Its
existing motive explicitly depends on Groudon escaping Maxie's control. Preserve
that connection instead of writing a different reason for the attack.

The physical ability to Dive is not proof that the submarine investigation or
Mossdeep story has happened. Alternate field-move access must not activate Seafloor's
successor scenes prematurely. Keep geographic access and story eligibility distinct.

Do not add a requirement to catch any legendary Pokémon to resolve the crisis.
Juan retains his existing connection to the crisis resolution.

### Earlier villain encounters after later chapters

Meteor Falls/Mt. Chimney, the museum, and the Weather Institute remain available
even when the player first follows the central villain story. Their dialogue and
local outcomes must remain coherent after later encounters with the same teams.

Keep completed-event recognition only when true. Remove or vary references to an
unplayed volcano defeat, an already-completed Mt. Pyre theft, or an organization's
future action that the player already witnessed. A late encounter must not undo
the weather crisis resolution or move a shared character backward through their
development without explanation.

Review the affected Maxie, Archie, and Shelly scenes before specifying the changes.
Prefer brief variants over new premises. If an encounter cannot remain available
after the finale without a substantial rewrite, record the exact conflict as an
open product decision; do not silently delete it or add the entire original
campaign back as a prerequisite.

### Legendary captures and late-game readiness

Apply the [late-game reward exception](johto-independent-story-beats.md) to Hoenn's
later legendary capture opportunities. A Pokémon appearing in the story is not
the same reward as obtaining it for the player's party.

The weather crisis and Rayquaza's intervention remain completable through their
story prerequisites and normal battle/field preparation. Do not apply the legendary
capture TR threshold to Seafloor, the crisis, Rayquaza's story awakening, or Juan's
Gym access.

Rayquaza's later capture opportunity requires the resolved crisis and sufficient
Trainer Rating. The later Groudon and Kyogre capture opportunities also require
their relevant resolved story and sufficient TR. Preserve their existing encounter
identities and presentation; no new capture quest or NPC is introduced.

Every route that activates these capture encounters must enforce the chosen
readiness rule, including revisits and alternate activation paths. Encounter level
alone is not a gate. An unmet requirement leaves the encounter available later,
with local guidance through existing characters or presentation. It does not
consume the encounter or award capture-related completion.

Exact TR thresholds, whether the three encounters share a threshold, and the
treatment of existing postgame activation checks remain explicit balance decisions.
Do not automatically replace every Champion flag with TR or add a new League
requirement. Other Hoenn legendary families and event encounters require their own
inventory and decision before this policy is extended to them.

### Brendan, May, Wally, and the player's origin

Keep genuinely sequential rival development at its existing locations. Later
chapters stay unavailable until the necessary earlier chapter is complete. Do not
move chapters around the region or use unrelated story progress as a substitute
for the actual relationship.

A deferred rival scene must not block an independent local adventure sharing its
map, and completing that adventure must not make the rival chapter unrecoverable.
The exact chapter prerequisites and shared-scene handling belong in the follow-up
specification.

Norman's relationship to the player is owned by the regional-origin design. Preserve
his local role while making dialogue consistent with that identity. This PRD does
not establish new family history or a new Hoenn opening.

## Boundaries

- Apply to Wayfarer's Hoenn content. Standalone Emerald behavior is outside scope.
- Preserve existing NPCs, maps, battles, puzzles, ordinary rewards, and field-use
  mechanics. No new factions, replacement campaign, quest journal, or map redesign.
- Do not redesign regional travel, the public ferry, the S.S. Aqua circuit, or the
  S.S. Tidal service. Travel remains separate from delivery and villain progression.
- No battle-scaling or encounter-level changes. Existing TR determines eligibility
  for the named legendary reward gates; it is not spent. Its calculation and League
  admission remain unchanged.
- Keep direct local Gym relationships. This is not a promise of every Gym or every
  geographic destination being immediately available.
- General Kanto and Johto redesigns remain in their own PRDs. Shared player identity
  follows the regional-origin work.
- Implementation details, exact dialogue variants, and state ownership belong in
  a subsequent specification. No gameplay change is claimed by this document.

## Interactions

Each adventure owns its start, progress, completion, and outstanding rewards.
Completing another adventure must not replay it, regress it, relocate its required
NPC irrecoverably, or imply an item handover that never occurred.

Wattson must remain available for his unearned badge even while New Mauville is
offered or active. Completing Norman or another Gym must not overwrite the errand's
progress. Later Gym victories must not replay the Rustboro theft or Mossdeep invasion
after their adventures are complete.

Save/reload, losses, regional travel, and full-party or full-Bag reward failures
must preserve a recoverable next step. The Parts, Letter, Castform, Scope, Goggles,
and other one-time rewards remain claimable once without advancing unrelated plots.

This design revises the retained campaign ordering in the
[Hoenn content specification](../specs/wayfarer-hoenn-content-port.md) and the
Norman/Wattson conditions in the [League circuit design](wayfarer-interregional-league-circuit.md).
Those documents describe the existing behavior until implementation adopts this
design. Keep the [open-world traversal contract](emerald-open-world-region-traversal.md):
passing through a region does not complete its adventures.

## Playtesting

- Start the Devon rescue without Roxanne's badge. Deliver the Letter and Parts in
  both orders, with travel and save/reload between them. Neither delivery completes
  before the player has its item.
- Visit Stern and Steven before accepting their deliveries. Receive a coherent
  introduction and return later to complete each preserved delivery scene.
- Complete the submarine story before an outstanding Devon delivery, then return
  for that delivery and reward without regressing harbor progress.
- Complete local adventures before and after the central villain story. Check
  first meetings, callbacks, departure directions, and organizational aftermath.
- After Magma Hideout, finish Aqua Hideout before Mossdeep and then test the reverse
  order. Both branches remain playable, including the hideout interior. Neither
  requires an unrelated early adventure.
- Approach Seafloor with neither branch complete and with each branch separately
  complete. Early field-move access cannot activate the finale. Both completed
  branches make it available without another unrelated activation event.
- Finish the crisis and access Juan below the eventual legendary capture threshold.
  Rayquaza's story intervention occurs without awarding or requiring a capture.
- At the eventual TR boundaries, test the later Rayquaza, Groudon, and Kyogre
  encounters, including alternate activation paths and a save made before readiness.
- Begin New Mauville without Norman's defeat, keep Wattson's initial badge available,
  and complete the errand before and after that badge.
- Complete Norman's local introduction and challenge with fewer than four Hoenn
  badges. Verify dialogue against the player's origin.
- Visit later rival locations early, complete unrelated local adventures, then
  return after the rival prerequisites. No chapter disappears or rewinds another plot.
- Interrupt every multi-step adventure with travel, a loss, and save/reload. Test
  deferred item and Pokémon rewards when the initial delivery cannot succeed.

Players unfamiliar with Emerald should understand the local problem and next step.
Players familiar with it should recognize the same adventure without needing a
replacement cast or plot to explain the supported order.

## Open questions

1. Which existing employee interaction starts the Rustboro theft without Roxanne,
   and how do Stern's museum and harbor roles coexist when those stories are reversed?
2. Which minimal dialogue variants keep late museum, Weather Institute, and
   Mt. Chimney encounters coherent after the central finale? Explicitly review
   Maxie's late Mt. Chimney appearance against his Space Center reflection and
   finale aftermath. Identify any scene that needs a retained dependency rather
   than a substantial rewrite.
3. What TR thresholds apply to later Rayquaza, Groudon, and Kyogre captures, and
   which existing postgame checks remain? Crisis resolution and Juan must remain
   independent of those capture thresholds.
4. Which Brendan/May and Wally chapters genuinely require predecessors, and how
   can their shared-map scenes remain recoverable without blocking local adventures?
5. What local introduction and staging let Wattson offer New Mauville while his
   initial Gym challenge remains available? Reuse Wattson and existing locations.

## References

- [Regional story independence audit](../research/regional-story-beat-independence.md)
- [Johto independent story beats and late-game reward exception](johto-independent-story-beats.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [Wayfarer Hoenn integration](wayfarer-hoenn-integration.md)
- [Wayfarer Hoenn content specification](../specs/wayfarer-hoenn-content-port.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
