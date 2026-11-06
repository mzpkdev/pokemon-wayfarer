# Hoenn independent story beats

Status: Agreed product design, revised 2026-11-06 with the owner's decisions on
the five questions the draft left open. Not implemented.
Specification: [Hoenn independent story beats](../specs/hoenn-independent-story-beats.md)

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
| Meteor Falls and Mt. Chimney | Discover the meteorite theft, follow it to Mt. Chimney, stop the machine, and recover the meteorite. | Keep this as one ordered adventure, independent of the Devon errands and unrelated Gyms. Preserve the existing return/reward interaction. It comes before the central villain story (see [Connected villain adventures](#connected-villain-adventures)). |
| Weather Institute | Rescue the staff, defeat the occupation, and receive Castform. | Keep the rescue locally available. Its ending must not announce a future Mt. Pyre event that already happened. The nearby rival scene has its own eligibility. |
| Steven and Kecleon | Resolve the Route 120 encounter and receive the Devon Scope. | Preserve its existing early/late introduction and local completion behavior. Steven's Letter is not a prerequisite. |
| Desert survey | Inspect the stakes and receive the Go-Goggles. | Preserve the existing local adventure and duplicate-reward protection with the later handoff. |
| New Mauville | Wattson requests help with the generator and gives the existing reward afterward. | Wattson steps outside and makes the request once the player holds his badge, as in Emerald. Norman's defeat is not needed. He never leaves the Gym before his first challenge is won. |
| Norman and Wally's introduction | Keep Wally's local introduction and Norman's challenge. | Remove the four-Hoenn-badge requirement for Norman. Retain the local introduction rather than rewriting it. Family dialogue must follow the regional-origin design. |

Early visitors to Steven or Stern receive an appropriate introduction rather than
being credited with a delivery they cannot make. The delivery confrontation and
handover remain unavailable until their actual prerequisite is met. Ordinary
access to the museum, city, harbor, and transport must remain available.

Completing a later harbor event must not delete an outstanding Parts delivery or
its reward. The submarine investigation never requires the delivery. Once the
harbor scene has played, Archie has already stolen the submarine, so the museum
confrontation is no longer told: Stern takes the Parts with a short line and no
fight. Before the harbor scene, the delivery keeps the full confrontation.

### Connected villain adventures

The proposed dependency structure is:

```text
Meteor Falls -> Mt. Chimney
                 |
Mt. Pyre -> Magma Hideout
                 |
                 +-> Slateport submarine theft -> Aqua Hideout --+
                 |                                              |
                 +-> Mossdeep / Space Center -> Steven's Dive ---+
                                                                |
                                       Seafloor -> Sootopolis -> Sky Pillar
```

The central villain story starts only after Meteor Falls and Mt. Chimney.
Maxie's purpose on Mt. Chimney is to wake what sleeps in the volcano. That
cannot be told once Groudon has woken, and no short variant repairs it. So the
Mt. Pyre theft waits until Mt. Chimney is resolved, and Mt. Chimney's scenes
never play late. This is the one dependency kept between a local adventure and
the central story.

Mt. Pyre introduces the stolen-orb problem and provides the existing Magma Emblem
lead to the hideout. Keep the hideout's Groudon awakening and Maxie encounter.
Neither requires completing the unrelated Devon deliveries, Weather Institute
rescue, or an entire earlier regional itinerary.

After Magma Hideout, the submarine pursuit and Mossdeep episode are sibling
adventures. Players may complete either first, alternate between them, and leave
Hoenn between their steps. Neither branch requires completing the other.

| Chapter | Retained cause and effect | Required change |
| --- | --- | --- |
| Mt. Pyre -> Magma Hideout | The existing Emblem connects the theft to the hideout and Groudon's awakening. | Start the theft only after Mt. Chimney. Before that, Mt. Pyre is open and Team Aqua is not on the summit. Add no other requirement. |
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

The museum and the Weather Institute remain available even when the player
first follows the central villain story. Their dialogue and local outcomes must
remain coherent after later encounters with the same teams. Meteor Falls and
Mt. Chimney always come first, as above, so they have no late form.

Keep completed-event recognition only when true. Remove or vary references to
an unplayed museum confrontation, an already-completed Mt. Pyre theft, or an
organization's future action that the player already witnessed. A late
encounter must not undo the weather crisis resolution or move a shared
character backward through their development without explanation.

Prefer brief variants over new premises. At the Weather Institute, Shelly and
her grunt withdraw without announcing Mt. Pyre once that theft has happened.
At the museum, a late Parts delivery is a plain handover.

### Legendary captures and late-game readiness

Apply the [late-game reward exception](johto-independent-story-beats.md) to Hoenn's
later legendary capture opportunities. A Pokémon appearing in the story is not
the same reward as obtaining it for the player's party.

The weather crisis and Rayquaza's intervention remain completable through their
story prerequisites and normal battle/field preparation. Do not apply the legendary
capture TR threshold to Seafloor, the crisis, Rayquaza's story awakening, or Juan's
Gym access.

Rayquaza's later capture opportunity requires the resolved crisis and sufficient
Trainer Rating (TR). The later Groudon and Kyogre capture opportunities also
require their relevant resolved story and sufficient TR. Preserve their existing
encounter identities and presentation; no new capture quest or NPC is
introduced. Hoenn's Terra Cave remains Groudon's capture site; the Marine Cave
remains Kyogre's capture site. The [coastal port](frlg-cinnabar-seafoam-port.md)
removes the separate HNS Seafoam Secret Cave Groudon and Route 19 Cave Kyogre
encounters from Wayfarer.

Every route that activates these capture encounters must enforce the chosen
readiness rule, including revisits and alternate activation paths. Encounter level
alone is not a gate. An unmet requirement leaves the encounter available later,
with local guidance through existing characters or presentation. It does not
consume the encounter or award capture-related completion.

The three captures share one threshold, TR 55, the number Johto's legendaries
use. Each needs the resolved crisis and TR 55, and nothing else. Terra Cave and
Marine Cave no longer need a Hall of Fame entry, which today a Kanto or Johto
win satisfies and Hoenn's own may not. Other Hoenn legendary families and event
encounters require their own inventory and decision before this policy is
extended to them.

### Brendan, May, Wally, and the player's origin

Keep genuinely sequential rival development at its existing locations. Later
chapters stay unavailable until the necessary earlier chapter is complete. Do not
move chapters around the region or use unrelated story progress as a substitute
for the actual relationship.

A deferred rival scene must not block an independent local adventure sharing its
map, and completing that adventure must not make the rival chapter unrecoverable.

Every Brendan or May battle picks its team from the player's Hoenn starter. A
visitor who never took Birch's starter does not meet Brendan or May: their
scenes wait and play once the player has a starter. The Itemfinder and HM Fly
stay rewards of those scenes. There is no variant with a borrowed team.

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

Wattson offers New Mauville only after his badge is won, so his first challenge
is never out of reach. Completing Norman or another Gym must not overwrite the errand's
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
- Complete the museum delivery and the Weather Institute before and after the
  central villain story. Check first meetings, callbacks, departure directions,
  and organizational aftermath.
- Climb Mt. Pyre before Mt. Chimney. The theft is not available and ordinary
  access is unaffected. Resolve Mt. Chimney and return for the theft.
- After Magma Hideout, finish Aqua Hideout before Mossdeep and then test the reverse
  order. Both branches remain playable, including the hideout interior. Neither
  requires an unrelated early adventure.
- Approach Seafloor with neither branch complete and with each branch separately
  complete. Early field-move access cannot activate the finale. Both completed
  branches make it available without another unrelated activation event.
- Finish the crisis and access Juan below the eventual legendary capture threshold.
  Rayquaza's story intervention occurs without awarding or requiring a capture.
- Below and at TR 55, test the later Rayquaza, Groudon, and Kyogre encounters,
  with and without a Hall of Fame entry in another region, and with a save made
  before readiness.
- Win Wattson's badge without Norman's defeat, then begin and complete New
  Mauville.
- As a visitor with no Hoenn starter, walk Routes 110 and 119 and visit Rustboro
  and Lilycove. No rival scene starts and nothing is blocked. Take Birch's
  starter and return for each scene.
- Complete Norman's local introduction and challenge with fewer than four Hoenn
  badges. Verify dialogue against the player's origin.
- Visit later rival locations early, complete unrelated local adventures, then
  return after the rival prerequisites. No chapter disappears or rewinds another plot.
- Interrupt every multi-step adventure with travel, a loss, and save/reload. Test
  deferred item and Pokémon rewards when the initial delivery cannot succeed.

Players unfamiliar with Emerald should understand the local problem and next step.
Players familiar with it should recognize the same adventure without needing a
replacement cast or plot to explain the supported order.

## Settled questions

The draft left five questions open. The owner settled them on 2026-11-06, and
the [specification](../specs/hoenn-independent-story-beats.md) carries the
detail.

1. **The Rustboro theft and Stern.** The existing street scene outside Devon
   starts the theft the first time the player passes, with no badge. Stern and
   Steven accept a delivery only from a player who holds its item. A Parts
   delivery after the harbor scene is a plain handover.
2. **Early villain scenes played late.** Meteor Falls and Mt. Chimney come
   before the central story. The Weather Institute gets a withdraw variant.
3. **Capture thresholds.** TR 55 for Rayquaza, Groudon, and Kyogre, with the
   resolved crisis and no Hall of Fame check.
4. **Rival chapters.** Brendan and May wait for the Hoenn starter. Wally's
   later chapters wait for his Petalburg tutorial.
5. **Wattson.** He makes the New Mauville request outside once the player
   holds his badge. Norman is not involved.

## References

- [Hoenn independent story beats specification](../specs/hoenn-independent-story-beats.md)
- [Hoenn story beat gates](../research/hoenn-story-beat-gates.md)
- [Regional story independence audit](../research/regional-story-beat-independence.md)
- [Johto independent story beats and late-game reward exception](johto-independent-story-beats.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [Wayfarer Hoenn integration](wayfarer-hoenn-integration.md)
- [Wayfarer Hoenn content specification](../specs/wayfarer-hoenn-content-port.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
