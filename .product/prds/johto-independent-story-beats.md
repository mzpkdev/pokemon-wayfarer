# Johto independent story beats

Status: Draft product design, revised 2026-11-02 against current main and the
Kanto story rules settled since the first draft. Not implemented. The open
questions below need scene-level answers before the implementation
specifications are complete.

HNS Johto is already Wayfarer's live map and story base, so unlike Kanto
there is no separate map-adaptation PRD. This document owns Johto's story
contract, and per-adventure specifications follow it. The
[gate audit](../research/johto-story-beat-gates.md) records what the scripts
do today.

## Intent

Let players discover and complete Johto's local adventures in different orders
while the journey remains coherent. Preserve the original stories wherever
possible: their NPCs, locations, objectives, battles, rewards, and most
dialogue. Remove unrelated prerequisites between adventures rather than replace
the stories.

A story beat here means a complete small adventure, not an individual cutscene.
It may span several locations and contain an ordered sequence. The lighthouse
medicine errand and the Lake of Rage investigation are each one adventure.

## Design

### Independence with minimal changes

An independent adventure introduces its problem where the player discovers it,
gives enough context to act, and ends with a clear local result. Its start does
not require unrelated Gym badges, league wins, distant story flags, the
player's starter choice, or a scene with Silver. Geography and existing
field-move preparation still apply.

Use the smallest change that makes an adventure coherent in the supported
orders:

1. Remove unrelated activation requirements.
2. Reuse existing NPCs and interactions to introduce the adventure locally.
3. Change only dialogue that assumes an unplayed event, contradicts remaining
   adventures, or gives obsolete directions.
4. Keep a necessary narrative dependency when removing it would require a
   substantial rewrite. Hide the successor scene until its predecessor finishes.

A hidden successor must leave travel and unrelated interactions available.
Visiting its location early must not consume it, award its rewards, or mark its
predecessor complete. Returning after the prerequisite is satisfied makes it
available normally. Existing characters may remain present with ordinary
dialogue; only the premature story scene is unavailable.

Do not add NPCs, new factions, new maps, replacement plots, or a
quest-selection interface. Cohesion comes from the existing setting and
characters, truthful callbacks, and visible local consequences.

### Late-game reward exception

Adventures may be discovered early. Final stages granting late-game rewards may
require Trainer Rating (TR), earned through different regional routes. Short,
direct story prerequisites may remain where they preserve the original
narrative with minimal changes. Explain both requirements before the player
commits to the final stage.

Apply this exception deliberately to legendary captures and similarly
consequential rewards. It does not authorize blanket TR gates on ordinary
adventures, useful items, field-move rewards, or Gym access. Each application
must name its reward, its gated stage, and any retained story prerequisites.
Other regional PRDs use this rule without inheriting Johto's storyline; Kanto's
Power Plant set Zapdos's battle threshold at TR 55, which is a reference point
for Johto's thresholds, not a decision.

In Johto the exception covers:

| Reward | Gated stage | Retained story prerequisite |
| --- | --- | --- |
| Main legendary (Lugia or Ho-Oh) | The five-Kimono-Girl trial | Radio Tower completed and its Wing received |
| Other legendary | Pewter's old man giving the other Wing | Main legendary encounter resolved |
| Elm's Master Ball | Elm's handoff after Clair's trial | Clair's trial completed |

The Master Ball is new to this list. Clair's eighth-badge position used to make
it a late reward. Once Clair can be challenged first, it becomes an early one.
Like Silph's Master Ball in Kanto, it waits for TR; Clair's badge and the rest
of her sequence do not.

### Who plays which story

Johto-origin players, who start in New Bark, play Johto's opening and Elm's
errand, and Silver's rivalry is theirs alone. Elm sends them for the Mystery
Egg, Silver steals from the lab, and his scenes follow the rules under
[Silver](#silver).

Visitors, who start in Kanto or Hoenn, receive none of that. They get no
Elm errand, no Johto starter choice or starter gift, no Mystery Egg or Togepi
Egg, and no Silver scenes. Mr. Pokémon is an ordinary resident to them, and
Elm is a professor they have not met. Every other Johto adventure, Gym,
reward, and the legendary ceremony stays available.

Visitors still receive Elm's Master Ball after Clair's trial, under the same
TR threshold. With no errand or phone call behind them, Elm introduces himself
and hands it over in person at his lab, and a local line after Clair's trial
points them there. This mirrors Kanto, where visitors do not play the Parcel opening
or Blue's rivalry.

This supersedes the start-choice PRD's
[visiting-Johto rule](wayfarer-regional-start-choice.md#visiting-another-starting-region)
and its specification's visitor Elm flow, which #75 implemented. Removing that
flow is part of this feature, not a migration: prerelease saves need no
compatibility.

Visitors are the reason the "no starter choice, no Silver" clause appears
throughout this document. A Johto adventure must play the same for a player
who never meets Silver.

### Adventure boundaries

| Adventure | Preserved story | Required change and retained order |
| --- | --- | --- |
| Slowpoke Well | Kurt investigates the harmed Slowpoke; the player confronts the Rockets and resolves the rescue. | Keep the local introduction, rescue, and reward. Bugsy, Silver, Ilex Forest, and the GS Ball story must not rewind or skip the Well, whichever happens first. |
| Burned Tower | Eusine introduces the ruins; exploration leads to the beasts' release. | The release no longer waits on Silver's Burned Tower scene. It plays for visitors and for Johto-origin players whose Burned Tower chapter has been retired. Morty's gym-door sage keeps its local tie to the release. |
| Dance Theater Rocket | The player helps a performer during the Rocket disturbance and receives HM Surf. | Keep it independently available. The ceremony and Elm's calls must not hide the Surf giver or rewind the theater. |
| Lighthouse medicine | Jasmine needs medicine for Ampharos; the player obtains it in Cianwood and returns. | Keep the two-town errand and its tie to Jasmine's Gym. Medicine-first visits get a short local introduction instead of Jasmine acting as if she sent the player. |
| Lake of Rage and Mahogany hideout | The disturbed lake leads to Lance, the shop investigation, the hideout, and shutting down the operation. | Keep the internal sequence. Pryce's Gym, the Radio Tower, and the merchant's road must not rewind or skip it, whichever happens first. |
| Radio Tower takeover | The player discovers the occupation, infiltrates the tower, rescues its staff, and defeats the takeover. | Remove Chuck's, Jasmine's, and Pryce's badges, Pryce's Gym victory, and Elm's phone call as activation requirements. The player starts it in Goldenrod; see [Radio Tower](#radio-tower-takeover). Keep the infiltration, keys, rescue, battles, and resolution in order. |
| Clair and Dragon's Den | Clair challenges the player; the Den trial completes her badge sequence. | Remove the badge 5–7 check. Keep Clair's challenge, the Den, and the deferred badge handoff together. The road to Blackthorn opens independently; see [Mahogany road](#mahogany-road-and-blackthorn). |
| Kimono ceremony and legendary encounter | The dancers, Bell, ceremony, and selected legendary encounter. | Introduce it through Ecruteak characters instead of Clair, Elm, and Silver's Ecruteak scene. Require Radio Tower completion, its Wing, and sufficient TR for the trial. Gate Pewter's other Wing behind resolution of the first encounter and the same TR. |
| Suicune pursuit | Suicune's sightings, leading to the Kanto half of the chase. | Keep the sightings' order. Start the chase however the player first reaches Olivine, including by ship. Eusine's first meeting must not assume an earlier one. |

### Radio Tower takeover

The occupation begins only when the player chooses to start it through an
existing Goldenrod interaction. Before that, the Radio Tower's ordinary
services, radio quiz, and Pokégear radio work. Arriving in Goldenrod, winning
Gyms, or receiving a phone call does not start it. Once started, the
occupation persists through travel and save/reload until the player resolves
it, and the radio quiz cannot regress it.

Elm's takeover call stops being an activator. For Johto-origin players it may
remain as an optional hint while the occupation is running, worded so it does
not deny Rockets the player has already beaten.

Silver's Goldenrod Underground scene is not required to continue. The Director
keeps the Card Key and Wing handoffs. When the Tower is finished before Baoba's
Route 39 choice has picked a branch, the Director keeps the selected Wing for
the player and hands it over once the branch is chosen. That chance is never
lost, and the Wing is never given twice.

### Mahogany road and Blackthorn

The RageCandyBar merchant stops blocking the road east of Mahogany. Route 44,
Ice Path, and Blackthorn follow ordinary traversal and field-move rules, with no
Radio Tower requirement. He keeps his shop, Wayfarer's TR-mart menu, and his
local dialogue. The Ice Path Kimono push remains a one-time local interaction.

This extends the [HNS traversal design](hns-open-world-region-traversal.md),
which left Route 44, Ice Path, and Blackthorn outside its pass. Opening the
road changes no other Mahogany, Lake of Rage, or hideout story.

Pryce's Gym, the Lake of Rage, the hideout, and the takeover share Mahogany's
progress today. Finishing any of them must not rewind another or bring the
merchant's block back.

### Team Rocket

Slowpoke Well, Mahogany, and Radio Tower must support every relative completion
order. Preserve the crimes, organization, characters, and local objectives.
There is no new story about separately named Rocket cells or factions.

First meetings use an introduction that does not claim the player defeated that
character elsewhere. Where the earlier encounter was completed, keep the
recognition line. Examples that need conditional variants:

- Proton's Radio Tower line recalling Slowpoke Well.
- The hideout grunt who remembers the player "from the well".
- Ariana recalling Mahogany in the Radio Tower, and her hideout boast about a
  later broadcast plan.
- The Radio Tower grunt asking whether the player got past the Underground.

Each resolution must remain true while the other operations are unfinished.
Backstory lines such as Rocket's disbandment three years ago are order-safe.
Change claims that Team Rocket is finished, or that a later operation already
happened, when they would contradict the player's remaining adventures.
Archer's "I will disband Team Rocket here today" is the main case. Use a short
neutral line or a completion-conditioned variant; do not invent a replacement
organization history.

Radio Tower's broadcast motive and final dialogue need a scene-level wording
review. Its completion must not silently resolve Mahogany or the Well, unlock
their rewards, or erase their participants.

### Kimono trial and legendary encounters

Keep the branch selection, characters, five-dancer challenge, Bell reward,
ceremony, and legendary encounter. Baoba's Route 39 choice selects Lugia or
Ho-Oh as the main legendary; the other remains available through Pewter's Wing
giver.

The main sequence is:

1. Discover the ceremony through existing Ecruteak characters. They explain
   the Wing and TR requirements without requiring Clair, Elm, or Silver.
2. Complete the independently available Radio Tower adventure and receive the
   selected legendary's Wing from the Director.
3. With that Wing and sufficient TR, take the five-Kimono-Girl challenge.
4. Win the challenge, receive its Bell, and continue to the ceremony and the
   selected legendary encounter. No other readiness gate sits between these
   stages.

Radio Tower completion and its Wing are an explicit retained story
prerequisite. Meeting the TR requirement first does not replace that story;
finishing Radio Tower early does not replace the TR requirement. Both orders
work, and neither requires returning to an unrelated NPC to activate progress.
The encounters currently follow ceremony progress, not Wing inventory, so the
implementation must enforce the Wing and TR conditions itself.

Remove Clair, Dragon's Den, Elm's Master Ball handoff, and Silver's Ecruteak
scene from this sequence. Preserve their own stories and rewards. The Director
keeps the Silver Wing for Lugia or the Rainbow Wing for Ho-Oh; the Kimono Girls
keep the Tidal Bell or Clear Bell.

Pewter's old man grants the other legendary's Wing only after the main
encounter has been resolved and the same TR requirement is met. Finishing the
ceremony without resolving its encounter is insufficient. Catching or
defeating the main legendary counts, as does a non-loss flee or escape that
completes the encounter. Losing or interrupting it does not. Capture is never
mandatory. Before Baoba's choice, Pewter's old man has ordinary dialogue.

Explain an unmet requirement locally and leave the interaction available on
return. A high encounter level is not a substitute for the readiness gate.
Preserve retry after a loss and one-time rewards through travel and save/reload.

The Kimono Girls' speeches recall the Mystery Egg, Ilex Forest, Slowpoke Well,
the Theater Rocket, the Ice Path, the Dragon's Den, the Goldenrod Underground,
and the Radio Tower. Keep each callback when the player lived it; otherwise
omit it or use a brief first-meeting variant. Visitors never hear the Mystery
Egg line. The speeches must not turn the old campaign into hidden
prerequisites.

Under the League circuit, Reception Gate already ignores the ceremony. The
ceremony is not a League prerequisite and does not become one.

### Silver

Silver's rivalry belongs to Johto-origin players and follows the forward
progression of
[Blue's Kanto rivalry](frlg-kanto-story-on-hns-maps.md#blues-origin-specific-role-and-forward-progression).

Keep his scenes at their existing locations in a stable chapter order that
follows the original story and does not depend on map discovery order. His
story moves forward when the player actually starts a later scene with him.
That retires every earlier scene, so returning to an earlier location never
starts an obsolete chapter. Entering a map, passing a deferred trigger, or
declining an optional encounter before it begins does not advance him.

Retiring a scene does not claim the player fought or beat Silver there. It
awards no skipped battle money, experience, items, or victory credit. A loss in
the current scene leaves that scene retryable; earlier scenes stay retired.

His chapters span Johto and Kanto: New Bark, Cherrygrove, Sprout Tower, Azalea,
Burned Tower, Olivine, the Mahogany hideout, the Goldenrod Underground,
Ecruteak, Victory Road, Mt. Moon, and his Indigo rematches. The Route 22 scene stays with
the optional Celebi adventure the
[Kanto story PRD](frlg-kanto-story-on-hns-maps.md#selected-hns-story-adaptations)
owns.

Restore his Victory Road battle. It is his last main-story fight and the turn
in his arc: he loses, admits Lance was right, and sets up his calmer Mt. Moon
and Indigo scenes. Today it never fires, because the League circuit skips the
Reception Gate step it waits for. It plays on a Johto-origin player's first
trip through Victory Road, whether or not they already compete at Indigo, and
follows the same forward-retire rule. Adjust his opening line, which assumes a
first League challenge, when the player has already competed there.

No host adventure waits on Silver. The Burned Tower release, the Radio Tower
Underground, the Mahogany hideout, and the Kimono ceremony all continue
whether his scene there is played, retired, or never offered. Playing or
retiring his scene does not complete, rewind, or reward any of them.

His dialogue refers only to what happened. His Azalea line asks about the Well,
his Underground after-battle line recalls Lance, and his hideout and Ecruteak
lines talk about the player beating Team Rocket. Each needs a variant for
players who have not done those things.

Silver keeps his current battle scaling. Whether he becomes a notable trainer
like Blue is separate work.

### Local Gym relationships

Keep direct story relationships: healing Ampharos before Jasmine returns to her
Gym, completing Clair's Den trial before her badge, and the Burned Tower release
before Morty's gym-door sage steps aside. This feature does not make every Gym
independent of its own local adventure.

Remove the named unrelated badge requirements. Do not replace them with a
global badge total, TR threshold, league win, or another unrelated story check.
The Route 32 guard's Miracle Seed no longer depends on Silver, the Togepi Egg,
or the Violet Gym.

## Boundaries

- Apply this design to Wayfarer's Johto stories. Standalone HNS behavior is
  outside scope.
- Preserve existing NPCs, maps, encounter identities, battles, puzzles, and
  normal rewards. Reward delivery may change where its old activation depended
  on an unrelated story, but duplication or silent loss is not acceptable. The
  one deliberate loss is that visitors no longer receive Elm's errand rewards.
- No broad rewrite of Rocket, Silver, the Kimono Girls, or Johto's mythology.
- No new quest journal, map redesign, regional opening, transportation system,
  battle scaling, or encounter balancing.
- Johto's opening up to Elm's errand stays as the start-choice design defines it
  for Johto-origin players.
- Kanto and Hoenn story designs stay separate. Pewter's Wing gate, Silver's
  Kanto chapters, and the Route 22 boundary are the Kanto touchpoints named
  here.
- The S.S. Aqua maiden voyage and its travel entitlement stay as the
  [S.S. ferries](ss-ferries.md) design keeps them.
- This PRD defines player behavior. Script variables, event placement, state
  ownership, and exact dialogue belong in the specifications.

## Interactions

### State, rewards, and shared characters

Completing one adventure must not move another backward, suppress its start,
replay its completed scenes, or duplicate its rewards. Today several adventures
share one progress value per town, and many scenes overwrite it. The audit
records concrete cases:

- Bugsy, the Well, and Silver share Azalea's progress.
- Pryce, the Lake, the hideout, the merchant, and the takeover share
  Mahogany's.
- The Theater Rocket, the Surf giver, Elm, and the ceremony share the theater's.
- Whitney, the radio quiz, and the takeover share Goldenrod's.

Each of these must work in every supported order.

When adventures share an NPC or location, each unfinished adventure must remain
discoverable. Ceremony staging must not erase an unfinished theater rescue or
its Surf reward. A necessary local sequence may be retained where the same
scene cannot coherently serve both events; document that dependency explicitly.

Leaving for another region, losing a battle, saving, and reloading preserve the
current step. Revisit checks must recognize prerequisites satisfied in any
supported order. A full Bag or interrupted handoff must leave the reward
claimable.

Preserve the original reward owners. The Radio Tower Director awards the main
Wing, the Kimono finale awards the Bell, Pewter's old man awards the other
Wing, and Elm awards the Master Ball. Local ceremony activation must not
silently grant their rewards or mark their adventures complete. Pewter's direct
encounter activation must not bypass its new prerequisites.

### Related product contracts

- [League circuit](wayfarer-interregional-league-circuit.md): this design
  revises its retained Johto story prerequisites, namely Radio Tower activation
  and Clair's three-badge requirement. It does not change League admission,
  the TR calculation, or badge accounting.
- [HNS traversal](hns-open-world-region-traversal.md): this design opens
  Mahogany's east road, which that pass left in place. A hidden story scene
  must not restore an old roadblock, and walking past an adventure must not
  complete it.
- [Regional start choice](wayfarer-regional-start-choice.md): visitors lose
  Elm's errand, the Johto starter choice, and its gift.
- [Kanto story on HNS maps](frlg-kanto-story-on-hns-maps.md): Silver's
  forward progression follows Blue's rule. Silver's Mt. Moon battle stays
  optional on the traversal lane.
- [Notable trainers](notable-trainers.md): unchanged. Gym Leaders' victory
  aftermath and phone contacts keep working when their Gyms are taken in a new
  order.
- [Native HM catch windows](native-hm-catch-windows.md): the Theater's Surf and
  the hideout's Whirlpool remain local rewards. Field preparation for Cianwood
  and the Lugia branch follows the current HM rules.

## Playtesting

Acceptance covers both understandable storytelling and recoverable state:

- Complete the three Rocket adventures in all six relative orders. Each has a
  truthful introduction and ending, and the unfinished operations remain
  available.
- Interleave the Well with Bugsy and Ilex Forest, and the Lake and hideout
  with Pryce and the Radio Tower. Nothing rewinds, skips, or returns the
  merchant's block.
- Start the Radio Tower with no Johto badges and no Mahogany progress. Use its
  services before starting the occupation. Finish without gaining an unrelated
  prerequisite. Finish it before Baoba's choice and collect the Wing later.
- Walk from Mahogany to Blackthorn with no Radio Tower progress and beat Clair
  and the Den without badges 5–7.
- Play as a Hoenn- or Kanto-origin visitor: Elm offers no errand or starter,
  Silver never appears, and every other adventure, including the Burned Tower
  release and Morty's Gym, completes.
- As a Johto-origin player, skip ahead to a later Silver chapter. Every earlier
  chapter retires with no credit, a loss leaves only the current one
  retryable, and his lines mention only what happened.
- As a Johto-origin player, reach Victory Road for the first time both before
  and after competing at Indigo. Silver's battle plays once, with an opener
  that fits.
- Discover the ceremony below the TR threshold without Clair or Elm. The
  characters explain what is needed; discovery grants no Wing, Bell, or
  encounter.
- Test the trial with neither requirement, only the Radio Tower Wing, only
  sufficient TR, and both. Only both unlock it.
- Speak to Pewter's old man before Baoba's choice, before the trial, after the
  trial but before resolving the encounter, and after resolving it. Only the
  last, with sufficient TR, grants the other Wing. Cover both branches and
  each completing encounter outcome.
- Complete Clair's trial below the Master Ball threshold; her badge arrives,
  and Elm's Master Ball arrives once the threshold is reached. As a visitor,
  follow the local lead to Elm's lab and receive it there once.
- Obtain medicine before visiting Jasmine, and also in the original order.
- Reach Olivine first by ship and by Route 39; Suicune's chase starts either
  way.
- Interrupt multi-step adventures with a loss, save/reload, and regional
  travel. The next step remains clear and available.

A player unfamiliar with the original campaign should understand each
adventure's problem, motivation, and conclusion. A player familiar with it
should recognize the same adventure without a new cast or replacement plot.

## Open questions

These are bounded scene decisions, not permission to expand the feature:

1. **Radio Tower start.** Which existing Goldenrod interaction starts the
   occupation by player choice?
2. **Rocket endings.** What are the smallest truthful variants for the
   broadcast motive and Archer's ending in each order? Review shared executives
   and later Kanto dialogue before choosing a global disbandment outcome.
3. **Ceremony introduction.** Which Ecruteak interaction introduces the
   ceremony and explains the Wing and TR gate with the least dialogue change?
4. **Thresholds.** What TR places the shared legendary threshold, and Elm's
   Master Ball threshold, in the 24-badge journey? Both must be decided
   before implementation.

## References

- [Johto story beat gates](../research/johto-story-beat-gates.md)
- [Johto legendary ceremony gates](../research/johto-legendary-ceremony-gates.md)
- [Regional story independence audit](../research/regional-story-beat-independence.md)
- [FRLG Kanto independent story beats](frlg-kanto-independent-story-beats.md)
- [FRLG Kanto story on HNS maps](frlg-kanto-story-on-hns-maps.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Wayfarer regional start choice](wayfarer-regional-start-choice.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
