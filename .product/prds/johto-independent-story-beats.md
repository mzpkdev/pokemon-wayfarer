# Johto independent story beats

Status: Draft product design. Not implemented. The open questions below require
scene-level design before an implementation specification is complete.

## Intent

Let players discover and complete Johto's local adventures in different orders
while the journey remains coherent. Preserve the original stories wherever
possible: their NPCs, locations, objectives, battles, rewards, and most dialogue.
Remove unrelated prerequisites between adventures rather than replace the stories.

A story beat here means a complete small adventure, not an individual cutscene.
It may span several locations and contain an ordered sequence. The lighthouse
medicine errand and the Lake of Rage investigation are each one adventure.

## Design

### Independence with minimal changes

An independent adventure introduces its problem where the player discovers it,
provides enough context to act, and ends with a clear local result. Its starting
conditions do not require unrelated Gym badges, League clears, or distant story
flags. Geography and existing field-move preparation still apply unless another
approved traversal requirement removes them.

Use the smallest change that makes an adventure coherent in the supported orders:

1. Remove unrelated activation requirements.
2. Reuse existing NPCs and interactions to introduce the adventure locally.
3. Change only dialogue that assumes an unplayed event, contradicts remaining
   adventures, or gives obsolete directions.
4. Keep a necessary narrative dependency when removing it would require a
   substantial rewrite. Hide the successor scene until its predecessor finishes.

A hidden successor must leave travel and unrelated interactions available. Visiting
its location early must not consume it, award its rewards, or mark its predecessor
complete. Returning after the prerequisite is satisfied makes it available normally.
Existing characters may remain present with appropriate ordinary dialogue; it is
the premature story scene that must be unavailable.

Do not add NPCs, new factions, new maps, replacement plots, or a quest-selection
interface. Cohesion comes from the existing setting and characters, truthful
callbacks, and visible local consequences.

### Late-game reward exception

Adventures may be discovered early. Final stages granting late-game rewards may
require Trainer Rating (TR), earned through different regional routes. Short,
direct story prerequisites may remain where they preserve the original narrative
with minimal changes. Explain both requirements before the player commits to the
final stage.

Apply this exception deliberately to legendary captures and similarly consequential
rewards. It does not authorize blanket TR gates on ordinary adventures, useful
items, field-move rewards, or Gym access. Each application must name its reward,
its gated stage, and any retained story prerequisites. Future regional PRDs can
use this rule without inheriting Johto's particular storyline.

### Kimono trial and legendary encounters

Keep the existing branch selection, characters, five-dancer challenge, Bell reward,
ceremony, and legendary encounter. The selected main legendary is Lugia or Ho-Oh;
the other legendary remains available through Pewter's existing Wing giver.

The main sequence is:

1. Discover the ceremony through existing Ecruteak characters early. They explain
   the Wing and TR requirements without requiring Clair or Elm's late-story visit.
2. Complete the independently available Radio Tower adventure and receive the
   selected legendary's Wing from the Director.
3. With that Wing and sufficient TR, take the existing five-Kimono-Girl challenge.
4. Win the challenge, receive its Bell, and continue to the existing ceremony and
   selected legendary encounter. Do not introduce another unrelated readiness gate
   between these stages.

Radio Tower completion and receipt of its selected Wing are an explicit retained
story prerequisite. Meeting the TR requirement first does not replace that story;
finishing Radio Tower early does not replace the TR requirement. Both orders work,
and neither requires returning to an unrelated NPC merely to activate progress.

Remove Clair, Dragon's Den, and Elm's Master Ball handoff as activation requirements
for this sequence. Preserve their own stories and rewards. The Radio Tower Director
keeps the Silver Wing for Lugia or Rainbow Wing for Ho-Oh; the Kimono Girls keep the
Tidal Bell or Clear Bell respectively.

Pewter's existing old man grants the other legendary's Wing only after the selected
main legendary encounter has been resolved and the same TR requirement is met.
Finishing the ceremony alone, before resolving its encounter, is insufficient.
Catching or defeating the main legendary counts; a non-loss flee/escape outcome
that completes the encounter also counts. Losing or interrupting the encounter does
not count. Capture is never mandatory. This unlock preserves the existing second
encounter rather than adding another ceremony or moving its reward giver.

Explain an unmet requirement locally and leave the interaction available on return.
A high encounter level is not a substitute for the readiness gate. Preserve existing
retry behavior after a loss and one-time rewards through travel and save/reload.

The exact TR threshold remains an open balance decision against the full 24-badge
journey. Use one shared threshold for this main trial and the Pewter follow-up;
do not invent a separate badge or League requirement.

### Adventure boundaries

| Adventure | Preserved story | Required change and retained order |
| --- | --- | --- |
| Slowpoke Well | Kurt investigates the harmed Slowpoke; the player confronts the Rockets and resolves the rescue. | Keep the local introduction, rescue, and reward sequence. Do not require another adventure. Later references to this rescue depend on actual completion. |
| Burned Tower | Eusine introduces the ruins and legendary Pokémon; exploration leads to the beasts' release. | Keep the discovery locally available. A later Silver chapter must not block the discovery when that chapter is not yet eligible. Beast appearances that depend on their release retain that prerequisite. |
| Dance Theater Rocket | The player helps a performer during the existing Rocket disturbance. | Keep it independently available with its existing reward. Do not require earlier Kimono Girl meetings. |
| Lighthouse medicine | Jasmine needs medicine for Ampharos; the player obtains it in Cianwood and returns. | Keep this complete two-town errand and its connection to Jasmine's Gym return. Support medicine-first visits with a short, locally understandable introduction rather than pretending Jasmine already sent the player. |
| Lake of Rage and Mahogany hideout | The disturbed lake leads to Lance, the shop investigation, the hideout, and shutting down the operation. | Keep these scenes in their existing internal sequence. Begin through the existing local introduction without unrelated adventures or badges. |
| Radio Tower takeover | The player discovers the occupation, infiltrates the tower, rescues its staff, and defeats the takeover. | Remove Chuck, Jasmine, Pryce, and Mahogany completion as activation requirements. Introduce the available crisis through existing Goldenrod interactions. Keep the infiltration, keys, rescue, battles, and resolution in order. |
| Clair and Dragon's Den | Clair challenges the player; the local trial resolves her badge sequence. | Remove the requirement for Chuck, Jasmine, and Pryce's badges. Keep Clair's challenge, Dragon's Den, and the deferred badge/reward sequence together. |
| Kimono ceremony and legendary encounter | The existing dancers, ceremony, and associated legendary encounter remain the adventure. | Make its introduction available through existing Ecruteak characters instead of Clair's completion and Elm's handoff. Require Radio Tower completion, its selected Wing, and sufficient TR for the trial, then preserve the Bell and ceremony sequence. Gate Pewter's other Wing behind resolution of the first encounter and the same TR. |

### Team Rocket

Slowpoke Well, Mahogany, and Radio Tower must support every relative completion
order. Preserve the existing crimes, organization, characters, and local objectives.
There is no new story about separately named Rocket cells or factions.

First meetings use an introduction that does not claim the player defeated that
character elsewhere. Where the earlier encounter was completed, retain the
recognition line. For example, Proton may mention Slowpoke Well only after the
player has completed that encounter.

Each resolution must remain true while the other operations are unfinished.
Preserve local defeat and rescue outcomes. Change claims that all of Team Rocket
has disappeared, or that a later operation has already occurred, when those claims
would contradict the player's remaining adventures. Use a short neutral line or a
completion-conditioned variant; do not invent a replacement organization history.

Radio Tower's original broadcast motive and final dialogue need a scene-level
wording review. The desired outcome is the same takeover adventure with truthful
context, not a new plot. Its completion must not silently resolve Mahogany or the
Well, unlock their rewards, or erase their participants.

### Silver and necessary continuing stories

Keep Silver's existing scenes at their existing locations. Do not move chapters
between locations or select a different chapter dynamically on arrival.

Later scenes that require his earlier losses, recognition, or character growth
remain unavailable until those narrative prerequisites are complete. Retain only
the dependencies needed for that story; unrelated Gym and local-adventure flags
must not substitute for Silver's actual progress.

An ineligible Silver scene must not block an independent adventure sharing its
location. If Radio Tower can be played before an eligible underground Silver
encounter, the takeover remains completable and the deferred rival scene remains
recoverable later. The same principle applies to Burned Tower and travel routes.

The exact chapter chain and treatment of encounters embedded in temporary story
scenes belong in the implementation specification. Do not promise independence by
silently deleting a chapter. If preserving a particular scene would require moving
it or substantially rewriting it, record that conflict for a product decision.

Releasing a legendary Pokémon before following its appearances is a valid retained
dependency. The Suicune pursuit remains a continuing adventure rather than several
independent sightings. Its Johto prerequisites must remain recoverable after travel.

The Kimono Girls' existing speeches recall help in several earlier towns and
adventures. Preserve those lines when the referenced events happened; otherwise
omit the callback or use a brief first-meeting variant. Those speeches must not
reintroduce the old campaign as a set of hidden prerequisites.

### Local Gym relationships

Keep direct story relationships such as healing Ampharos before Jasmine returns
to her Gym and completing Clair's local trial before receiving her badge. This
feature does not make every Gym independent of its own local adventure.

Remove the named unrelated badge prerequisites above. Do not replace them with a
global badge total, Trainer Rating threshold, League clear, or another unrelated
story check. Existing access to Blackthorn and other locations is governed by the
traversal design; removing Clair's Gym check alone does not promise unrestricted
geographic access.

## Boundaries

- Apply this design to Wayfarer's Johto stories. Standalone HNS behavior is outside
  scope.
- Preserve existing NPCs, maps, encounter identities, battles, puzzles, and normal
  rewards. Reward delivery may need adjustment when its old activation depended
  on an unrelated story, but duplication or silent loss is not acceptable.
- No broad rewrite of Rocket, Silver, the Kimono Girls, or Johto's mythology.
- No new quest journal, map redesign, regional opening, transportation system,
  battle scaling, or encounter balancing.
- General Kanto and Hoenn adventure redesigns remain separate. Pewter's alternate
  legendary Wing gate is the explicit Kanto exception in this PRD. Shared characters and
  cross-region callbacks must be identified so this change does not claim full
  journey cohesion while leaving known contradictions unaddressed.
- The S.S. Aqua maiden voyage and its established travel entitlement remain intact.
- This PRD defines player behavior. Script variables, event placement, state
  ownership, and exact revised dialogue belong in a subsequent specification.

## Interactions

### State, rewards, and shared characters

Completing one adventure must not move another adventure backward, suppress its
start, replay its completed scenes, or duplicate its rewards. This includes
finishing Slowpoke Well after later Azalea activity and completing Radio Tower
before the Mahogany investigation.

When adventures share an NPC or location, each unfinished adventure must remain
discoverable. Ceremony staging must not erase an unfinished theater rescue or its
reward. A necessary local sequence may be retained where the same scene cannot
coherently serve both events; document that dependency explicitly.

Leaving for another region, losing a battle, saving, and reloading preserve the
current step. Revisit checks must recognize prerequisites satisfied in any supported
order. A full Bag or interrupted reward handoff must leave a way to claim the reward.

Preserve the original reward owners: the Radio Tower Director awards the main
Wing, the Kimono finale awards the Bell, Pewter's old man awards the other Wing,
and Elm awards the Master Ball. Local ceremony activation must not silently grant
their rewards or mark their adventures complete.

The [ceremony gate audit](../research/johto-legendary-ceremony-gates.md) confirms
that the current main encounter uses ceremony progress rather than checking
Wing/Bell inventory. The implementation must explicitly enforce the approved
Radio Tower/Wing and TR readiness conditions when replacing the Clair/Elm bridge.
Pewter's existing direct encounter activation must not bypass its new prerequisites.

Silver's chapter completion must not stand in for progress in the hideout, Radio
Tower, or ceremony. Those adventures can advance while his scene is deferred;
playing it later cannot move their state backward. Shared Kimono Girl appearances
likewise remain individually available or completed after another adventure changes
their staging.

### Related product contracts

This proposed design revises the retained Johto story prerequisites described by
the [League circuit PRD](wayfarer-interregional-league-circuit.md), specifically
Radio Tower activation and Clair's three-badge requirement. It does not alter
League admission, the Trainer Rating calculation, or badge accounting. The
legendary readiness gate consumes existing TR. The older specifications
still describe the current implementation until this design is implemented.

The [HNS traversal design](hns-open-world-region-traversal.md) opens routes without
completing their stories. Preserve that separation. A hidden story scene must not
restore an old roadblock, and walking past an adventure must not complete it.

## Playtesting

Acceptance should cover both understandable storytelling and recoverable state:

- Complete the three Rocket adventures in all six relative orders. Each has a
  truthful introduction and ending, and all unfinished operations remain available.
- Begin Radio Tower without Chuck, Jasmine, Pryce, or Mahogany completion. Finish
  it without acquiring an unrelated prerequisite during the adventure.
- Reach Clair's Gym without those three badges and complete the preserved local
  challenge and badge handoff, subject to the existing geographic access rules.
- Discover the ceremony below the TR threshold without Clair or Elm's handoff.
  The characters explain what is needed; discovery grants no Wing, Bell, or encounter.
- Test the trial with neither requirement, only Radio Tower/Wing, only sufficient
  TR, and both. Only both unlock it. Exercise both completion orders and the final
  chosen TR boundary. Radio Tower itself remains playable below that boundary.
- Complete the trial, Bell handoff, ceremony, and main encounter without Clair,
  Dragon's Den, Elm's Master Ball handoff, or another unrelated activation event.
- Speak to Pewter's old man before the trial, after the trial but before resolving
  the encounter, and after resolving it. Only the last state with sufficient TR
  grants the other Wing and activates its encounter. Cover both legendary branches.
- Verify first-encounter capture, defeat, and a completing flee/escape outcome all
  permit the follow-up. A loss or interruption does not. No capture requirement,
  duplicate Wing, or early second-encounter activation may appear after save/reload.
- Obtain medicine before visiting Jasmine, and also follow the original order.
  Both versions explain the problem without asserting an earlier conversation.
- Visit a later Silver location before its prerequisite chapter, complete the
  available local adventure, then return after progressing Silver. His scene still
  works and refers only to events that happened.
- Release the beasts before their dependent appearances; visiting those locations
  too early neither starts nor consumes a later encounter.
- Complete unfinished local quests after unrelated Gym wins, regional travel, and
  other story resolutions. Rewards and local aftermath occur once.
- Interrupt multi-step adventures with a loss, save/reload, and regional travel.
  The next step remains clear and available.

A player unfamiliar with the original campaign should understand each adventure's
problem, motivation, and conclusion. A player familiar with it should recognize
the same adventure without needing a new cast or replacement plot to explain it.

## Open questions

These are bounded scene decisions, not permission to expand the feature:

1. Which existing Goldenrod interaction activates the takeover without making
   ordinary first-visit Radio Tower services inaccessible? Preserve both normal
   services and a locally discoverable start.
2. What are the smallest truthful variants for the broadcast motive and Rocket
   ending in each supported order? Review shared executives and later Kanto
   dialogue before choosing a global disbandment outcome.
3. Which existing Ecruteak interaction introduces the ceremony and explains the
   agreed Wing/TR gate with the least dialogue change?
4. What exact TR threshold places these legendary rewards appropriately in the
   24-badge journey? This number must be decided before implementation; the story
   sequence and use of the same threshold for both unlocks are settled.
5. Which Silver chapters require which predecessors, and how can scenes inside
   temporary occupations remain playable afterward at their existing locations?
   Include the later Kanto appearances in the dependency review, without expanding
   this into a general Kanto story rewrite.

## References

- [Regional story independence audit](../research/regional-story-beat-independence.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [HNS traversal specification](../specs/hns-open-world-region-traversal.md)
