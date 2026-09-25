# FRLG Kanto story on HNS maps

Status: Draft product design. Not implemented. FRLG is Kanto's narrative
baseline and HNS is its general geographic base. The full FRLG Cinnabar,
Seafoam, and Routes 19–21 port is owned by
[its dedicated PRD](frlg-cinnabar-seafoam-port.md).
The [S.S. Anne adventure port](../specs/frlg-kanto-ss-anne-adventure.md)
defines its permanently recoverable, one-time adventure. Selected HNS story
adaptations are recorded below; remaining choices stay open.

## Intent

Bring FireRed and LeafGreen's recognizable Kanto adventures to Wayfarer's
existing HNS Kanto. Keep the routes, towns, and interregional connections that
players already explore, while restoring the story spaces needed for the Rocket
investigations, Fuji's rescue, and the other mainland adventures.

This is an adaptation to one shared Kanto. Its date is deliberately imprecise:
players can experience FRLG Kanto and HNS Johto adventures in the same world.
FRLG is the narrative baseline for Kanto. Conflicting HNS Kanto content must
be adapted to that baseline, rather than treated as an equally binding later
history. Preserve as much HNS Kanto story content as possible through coherent
adaptations; do not remove a quest merely because it originated in HNS.

This release does not implement transitions between historical world states
or rebuild the entire overworld to match FRLG. Cinnabar remains intact; an
eruption is outside the dedicated coastal port.
ROM space recovery will happen before implementation and does not drive cuts
to this product design.

## Design

### Keep HNS geography and add selected story interiors

HNS remains the base for Kanto's exterior maps, route connections, settlements,
and links to Johto. Preserve existing traversal and landmarks wherever the
story can use them. Adapt NPC positions, movement, encounters, and entrances to
those spaces rather than requiring FRLG's exterior coordinates.

Import the Celadon Rocket Hideout, Pokémon Tower, and Silph's missing floors
as the principal story interiors owned here. Use the existing FRLG
layouts and their recognizable puzzles as the starting point. A small entrance
or transition adjustment is acceptable. Cinnabar, Seafoam, and Routes 19–21
are selected exceptions owned by the [coastal port](frlg-cinnabar-seafoam-port.md). Other
wholesale city or route replacements remain outside this design.

Do not replace these adventures with a single room containing their final
battle and reward. The player should investigate the hideout, navigate Silph,
climb the Tower, and explore the separately specified Mansion. Exact layout reuse, shared tilesets,
floor connections, and entrance positions belong in the implementation specs.

### Coastal port ownership

The [FRLG coastal port](frlg-cinnabar-seafoam-port.md) owns Cinnabar, Seafoam,
Routes 19–21, their local puzzles, services, encounters, rewards, the current
coastal port's Blue Viridian introduction, and the coastal connections. The
Viridian finale removes that introduction. The Mansion Secret
Key leads to Blaine's Cinnabar Gym. Its Seafoam cave contains the FRLG current
puzzle and Articuno. Bill's Sevii story remains independent, and Groudon is
catchable through Hoenn's Terra Cave rather than the removed HNS Seafoam
Secret Cave. This broader story PRD adds no second island, cave, Gym, or
eruption state.

### Adventure coverage

| Adventure | Adaptation requirement |
| --- | --- |
| Mt. Moon | Place the Rocket encounters, Super Nerd battle, and fossil choice in the existing HNS cave where practical. Preserve the local sequence and reward choice; reproducing FRLG's full cave topology is not required. |
| Cerulean burglary | Use the HNS city and a suitable house/back-exit arrangement for the theft, Rocket confrontation, and recovered TM. Local doorway or event changes are allowed. |
| Nugget Bridge | Adapt the ordered Trainer challenge, prize, and recruitment attempt to HNS Route 24. Avoid overlapping its existing Rocket scenes. |
| Bill and the S.S. Anne | Preserve Bill's rescue and Ticket reward, plus the captain's adventure and Cut reward aboard a persistent Anne. Any S.S. Ticket permits boarding; Bill is not a prerequisite for players who already have one. Persistence prevents missable content; the ship provides no transport or recurring utility. |
| Celadon Rocket Hideout | Connect imported hideout interiors to the HNS Game Corner. Preserve discovery of the entrance, local key and lift progression, Giovanni, and the Silph Scope reward. Mahogany's HNS Rocket Hideout remains a separate Johto location. |
| Pokémon Tower and Fuji | Restore the upper memorial floors while the ground floor is being converted for radio use. Preserve the Scope-dependent ghosts, Marowak resolution, Rocket rescue, Fuji's return, and physical Flute handoff. The [Tower specification](../specs/frlg-kanto-pokemon-tower.md) owns the shared building and transition. |
| Snorlax | Preserve the two FRLG encounters associated with Routes 12 and 16 and the Flute requirement, adapting event placement to HNS geography. Neither encounter may close the only ordinary travel route. Resolve the existing HNS Vermilion Snorlax separately rather than accidentally duplicating the same encounter. |
| Silph Co. | Extend the existing Saffron destination with the missing interior adventure. Preserve access puzzles, staff rescue, Giovanni, Lapras, and the deferred Master Ball reward. |
| Safari Zone and Warden | Use HNS Fuchsia's existing Safari areas for the Surf destination and Gold Teeth search. Keep returning the Teeth for Strength as a separate local objective. Do not import the entire FRLG Safari merely to reproduce coordinates. |
| Mansion and Blaine | The dedicated coastal port owns the intact town, Mansion, Secret Key, Gym, Lab, Center, Mart, and Blaine's local rewards. No broader Rocket or League prerequisite is added. |
| Giovanni's finale | After Celadon Hideout and Silph liberation, use the full FRLG Viridian Gym for Giovanni's one-time Earth Badge and Earthquake TM. Blue leaves Viridian entirely; his repeatable Dojo battle unlocks on the first committed Indigo victory. The [finale specification](../specs/frlg-kanto-viridian-finale.md) owns both roles. |
| Legendary sites | The dedicated coastal port owns full FRLG Seafoam and Articuno. Keep the operating HNS Power Plant and add its FRLG old generating hall for the sole Zapdos encounter, as specified [here](../specs/frlg-kanto-power-plant.md). Reuse HNS Cerulean Cave where its spaces support Mewtwo; any missing puzzle space needs a named, bounded addition. Groudon belongs to Hoenn. |

The coastal port owns fossil revival at Cinnabar's laboratory, independently
of unrelated regional campaign completion.

### Preserve local causes and allow different adventure orders

Adopt the adventure dependencies and reward rules in
[FRLG Kanto independent story beats](frlg-kanto-independent-story-beats.md).
In particular:

- Celadon Hideout leads to the Scope, then the Tower rescue and Flute.
- Silph is independently available without requiring the Tower rescue.
- Celadon Hideout and Silph liberation lead to Giovanni's finale in either
  order. Tower/Fuji, smaller Rocket incidents, and Snorlax are not prerequisites.
- Bill provides a route to the S.S. Ticket and Anne boarding; an existing
  Ticket also permits boarding. The coastal port owns the Mansion's local Key
  and Blaine connection without triggering Bill's Sevii trip.
- Unrelated badges, League clears, or regional campaigns do not activate these
  adventures. Retain ordinary field preparation and necessary local puzzles.

An early visit introduces the problem without completing it. Returning later
with the required tool or completed predecessor makes the next scene playable.
Travelling to another region must not reset an adventure. Blue's earlier
rival scenes may be retired only through his explicit progression rule below. Players can alternate between the Rocket investigations.

Preserve the original objectives, important characters, battle identities,
ordinary rewards, and dialogue where it remains true. Use short variants for
different encounter orders and for the HNS setting. Do not invent replacement
plots merely to avoid importing the selected interiors.

### Lavender during the radio conversion

Lavender is partway through converting Pokémon Tower for radio use. Its ground
floor is the existing HNS radio lobby and studio; the old memorial
floors, graves, and Channelers remain above. The Soul House is already open as
a public chapel and has received some memorials. Mr. Fuji supervises the
remaining relocation, which pauses when disturbances begin and he disappears.
This is Wayfarer's local continuity, without a fixed year or a claim that all
Gen 1 events preceded all Gen 2 events.

Keep both purposes legible and independently playable. Visitors can reach the
memorial floors before obtaining the Scope or restoring Power Plant service.
The Scope remains necessary to resolve the ghost and rescue Fuji. The radio
director's existing Machine Part upgrade, services, and broadcasts keep their
own conditions; Tower progress does not grant or disable them. The physical
Poké Flute from Fuji remains a separate reward from the radio's Poké Flute
program. The memorial floors remain accessible after the rescue.

The port uses a guard-led fade transition from the radio lobby to 2F; the
downward stair returns to the lobby. It reuses the FRLG upper-floor layouts
without `map.bin` changes. The [Tower specification](../specs/frlg-kanto-pokemon-tower.md)
owns the floor graph, Fuji staging, source-content audit, and entrance proof.

### Power Plant old generating hall

Keep the HNS Power Plant's staffed entrance, back room, Machine Part repair,
trade, and power-dependent services. An existing entrance-hall worker offers a
scripted fade into the complete FRLG old generating hall; all of its exits
return safely to the HNS lobby. The hall's maze, encounters, items, and two
Electrode are available independently of the repair. The back room's west exit
keeps its repair condition. No exterior replacement or `map.bin` edit is part
of this port.

Move the only Route 10 Zapdos into the old hall. Players can explore it before
Trainer Rating 55; the fixed level 50 battle becomes available at TR 55. Zapdos
uses FRLG outcomes: catch or KO resolves it permanently, while running,
teleporting, or blacking out leaves it available on return. League completion
does not respawn it. The [Power Plant specification](../specs/frlg-kanto-power-plant.md)
owns the entrance, returns, source content, and encounter state.

### One coherent Kanto cast and history

The player experiences one version of each shared character and institution.
Do not leave an HNS NPC describing an event as long finished while the imported
FRLG adventure presents that same event as unresolved.

Preservation is the default. Each overlapping HNS scene must be explicitly retained, adapted, or deferred
in a follow-up content specification. This includes the Machine Part theft,
Misty's absence, the radio upgrade, Vermilion Snorlax, and Blue's broader story
role. The coastal port resolves Blaine's location and Blue's current introduction;
the Viridian finale supersedes that introduction. The coastal port also resolves
Cinnabar and Seafoam's condition, and Routes 19–21. Unrelated HNS
flavor, services, and side content remain available unless a named conflict
requires a change. Dropping content requires a named conflict and a product
decision after considering dialogue, event placement, and dependency changes.
The [HNS Kanto conflict inventory](../research/frlg-hns-kanto-story-conflicts.md)
records recommendations separately from accepted decisions.

Keep the Machine Part and radio story's current causal chain: repair enables
Misty's return, the Magnet Train's power, Copycat's Pass, and the radio upgrade.
The old generating hall and Zapdos neither require nor advance that chain.

Retained HNS side quests must not become new prerequisites for the FRLG
investigations or restore removed restrictions on interregional travel. Their
unfinished scenes and rewards must remain coherent after Giovanni's finale.

### Selected HNS story adaptations

- Devon already operates Fuchsia Safari. Baoba visits his former home and
  loses his Gold Teeth while inspecting the Safari. Preserve the Teeth/Strength
  adventure and his existing Safari research/competition quest, with dialogue
  reflecting this arrangement rather than a closed Fuchsia Safari.
- Make the normal Silph lobby interactions and gifts available after
  liberation. Preserve the Up-Grade interaction. Steven's gift reward design
  remains open, including how it serves a Hoenn-origin player.
- Preserve the Celebi scenes as an optional adventure: Route 22 after the
  player's actual Giovanni finale, and Tohjo after that finale and the relevant
  Goldenrod story. Adapt exact-year references, Giovanni's recognition of the
  player, and time-travel framing. These prerequisites gate only this optional
  adventure, not either regional campaign.
- The shared Kanto/Johto Indigo lineup is Lorelei, Bruno, Agatha, Lance, then
  Blue. Will, Koga, Bruno, Karen, and Lance appear in the non-League Sevii
  Masters Challenge. Retain circuit qualification, order, scaling, and reward
  accounting. Blue follows the origin-specific progression below; Koga's dual
  institutional roles still need their own decision.

### Misty's current sequence

Preserve the selected existing sequence: return the Machine Part and repair
the Power Plant, meet Misty on her Cape date, then return her and the Gym
Trainers to Cerulean Gym. This is the prerequisite for her initial Cascade
Badge, not for ordinary Kanto settlement traversal. The existing League
circuit PRD explicitly permits this regional prerequisite.

Do not move the date to a post-badge outing. Adapt conflicting dialogue while
keeping the chain free of unrelated prerequisites and preventing a circular
dependency on Misty's own badge or completion of another regional campaign.

### Blue's origin-specific role and forward progression

Only Kanto-origin players receive Blue's personal rival campaign. Johto- and
Hoenn-origin players meet him through an appropriate introduction and face him
as Kanto Champion, without inheriting the Kanto-origin rival battle itinerary
or being told they grew up together. They keep access to the same local
adventures, ordinary gifts, and League circuit. Other origins retain their own
rivals; visiting Kanto does not change the player's rival identity.

The Kanto opening is planned as separate work. This PRD defines how its future
origin participates; it does not implement that opening or select its starter
roster. Until it exists, supported visiting origins use Blue's visitor role.

For Kanto-origin players, Blue's story moves forward when the player actually
starts a later authored encounter with him. Retire all earlier Blue encounters
so returning to an earlier location never starts an obsolete rivalry chapter.
Merely entering a map, passing a hidden trigger, or declining an optional
encounter before it begins does not advance the rivalry. Maintain a stable
chapter order independent of map discovery order.

Retiring an earlier encounter means it is no longer playable; it does not
claim the player fought or defeated Blue there. Award no skipped battle money,
experience, items, or victory credit. Dialogue may reflect Blue's current
relationship with the player without inventing previous meetings or victories.
A loss in the current encounter leaves that encounter retryable; earlier
chapters remain retired rather than returning after a loss.

Starting Blue's Champion encounter follows the same forward-progression rule.
Winning records the normal League completion and leaves all earlier rival
encounters retired. League entry remains governed by the existing circuit,
without requiring the earlier rivalry or the Kanto regional story. A visitor
meeting Blue for the first time at the League receives an introduction; a
Kanto-origin player receives dialogue appropriate to the encounters actually
played.

Only Blue's scenes advance this way. Retiring a Tower or Silph rival encounter
does not rescue Fuji, identify Marowak, defeat Giovanni, liberate Silph, award
Lapras, grant an HM, or complete the ship adventure. Remove any dependency
that would make a skipped rival scene necessary for its host adventure or an
ordinary reward. Those adventures remain playable in their own state.

For visitors, shared-location Blue appearances may provide introductions or
context where useful, but must not activate the Kanto-origin rival campaign.
The specification must identify which shared-location scenes remain and how
the first introduction is remembered. Do not silently repurpose a scripted
rival battle as a required visitor battle.

## Boundaries

- Wayfarer only. Standalone HNS and FRLG keep their existing campaigns.
- Mainland Kanto adventures are in scope. Sevii, its transport, Celio's
  deliveries, Lostelle, and Mt. Ember are separate work.
- Kanto origin is planned separately. This feature defines its Blue rivalry
  contract but does not implement starter selection or the Kanto opening.
  Visiting players do not need an Oak's Parcel opening to access Kanto stories.
- No wholesale replacement of HNS city or route layouts beyond the selected
  Cinnabar, Seafoam, and Routes 19–21 exceptions, new parallel Kanto,
  or general import of every FRLG house, gate, cave, tileset, or soundtrack.
- No new quest-selection interface or quest journal is required.
- No change to the global TR formula, scaling rules, League order, or total
  of 24 sanctioned Gym Badges.
- No implementation of compression or asset deduplication in this feature.

## Interactions

### Badges, battles, and rewards

Use Wayfarer's established Trainer, Gym Leader, and League scaling systems.
Importing a battle must not silently bypass them by copying a standalone FRLG
party or progression check.

Giovanni replaces Blue as Viridian's sole Earth Badge giver. Preserve one Kanto
badge identity and one first-award contribution to TR; Giovanni departs after
the Earthquake TM is delivered and has no rematch or successor. Blue's existing
repeatable Dojo battle instead unlocks on the first committed Indigo Champion
victory, independently of Giovanni and player origin. No new TR, badge-count,
or League-clear condition is introduced for the Earth Badge. The delivered
coastal port's Blue Gym remains current runtime until this story port lands;
the [finale specification](../specs/frlg-kanto-viridian-finale.md) supersedes
its Viridian invitation and Gym requirements then.

Keep ordinary local rewards attached to their adventures even when the player
already owns the corresponding HM or can use the field move through another
region's progression. A prior acquisition must not auto-complete the story.
Repeated conversations cannot repeatedly award one-time items or Pokémon.
Bag and party capacity failures leave rewards claimable later.

Retain the independent-story design's readiness requirements for the Master
Ball and legendary captures. Zapdos's battle threshold is TR 55; other
unsettled thresholds are decided separately. A deferred Master Ball does not
delay Silph's rescue or Giovanni's finale. No Sevii
delivery requirement is imported for Mewtwo.

### S.S. Anne ownership

The [S.S. Anne adventure port](../specs/frlg-kanto-ss-anne-adventure.md) is the
single implementation authority for its 25 imported interiors, Vermilion
boarding, return path, Trainers, items, Blue scene, captain, Cut reward, and
saved state. Any S.S. Ticket permits boarding. Bill remains the normal Kanto
source, but his rescue stays playable when the player already owns a Ticket.

The Anne remains accessible so players can recover unfinished one-time
content. It does not depart after the captain's reward, reset on revisit, sail
to another port, or provide recurring services, battles, or rewards. S.S. Aqua
and Seagallop remain the travel networks. The port must preserve their menus,
eligibility, state, and destinations.

Keep the current open-traversal behavior and supported Johto/Hoenn origins.
Tower, Silph, ship, and Gym completion must not newly gate ordinary city access,
regional return travel, or an unrelated region's adventure. Local optional
shortcuts may retain their separately approved requirements.

### Relationship to the existing story PRD

This PRD selects the HNS geography and selective-interior approach previously
deferred by the FRLG independent-story design. For this adaptation, references
to original locations mean their corresponding HNS destinations. The explicit
Mt. Moon and Safari adaptations relax exact FRLG map and puzzle reproduction;
the major imported story interiors retain their recognizable progression.

For Blue, this PRD explicitly supersedes the independent-story PRD's rule
that unseen rival chapters remain recoverable and that later chapters wait
for all necessary predecessors. Later encounters instead retire earlier
Blue chapters, and only Kanto-origin players receive his rivalry. Host
adventure and reward recovery requirements remain unchanged.

For the Anne, this PRD supersedes the independent-story design's temporary
ship/departure requirement and strict Bill-before-boarding sequence. The Anne
persists only to keep its one-time adventure recoverable, any existing S.S.
Ticket permits boarding, and Bill's rescue remains available independently.
His reward still provides a route to boarding for a player without the Ticket.

### Superseded transport and circuit requirements

The following are approved future Wayfarer changes, not claims about current
implementation. Their corresponding specifications record the same scoped
overrides; the runtime and existing audits remain unchanged in this docs-only
proposal.

- **Vermilion menu extension:** The
  [S.S. Anne specification](../specs/frlg-kanto-ss-anne-adventure.md#boarding-and-return)
  owns the exact additive menu contract and the required Aqua audit changes.
- **Circuit opponents:** Indigo is the one shared Kanto/Johto League and uses
  the FRLG rooms with Lorelei, Bruno, Agatha, Lance, and Blue. The HNS sequence
  of Will, Koga, Bruno, Karen, and Lance moves to the non-League Sevii Masters
  Challenge in the existing Seven Island battle house. The
  [circuit specification](../specs/wayfarer-interregional-league-circuit.md)
  owns admission, order, scaling, save/load recovery, loss/retry, rewards, and
  presentation for both stages. Update affected roster validation with the
  implementation; this proposal does not change ROM code.

The [Viridian finale specification](../specs/frlg-kanto-viridian-finale.md)
supersedes the coastal port's Blue invitation and Gym ownership and the League
specification's older Blue badge path. Giovanni awards one Earth Badge and
departs permanently after the TM handoff; Blue has no Gym succession. Indigo's
shared admission and Champion role are unchanged. Current runtime continues to
use Blue until implementation.

Other conflicts are not implicitly resolved. The decisions below must be
recorded here and reconciled with the affected story, League, or transport
requirements before implementation. Until then, the current runtime behavior
remains the baseline, not proof that an open choice has been made.

## Constraints

Space recovery is an implementation prerequisite handled by separate work.
The finished feature must fit Wayfarer's standard 32 MiB release budget and
reserve policy, but this PRD makes no byte estimate or claim that a port fits
today. Measure the selected content in release builds during implementation.

Prefer shared existing assets and selectively enabled interiors. Existing FRLG
source files do not imply that their data or scripts are already included in
the Wayfarer ROM.

Adventure state must remain distinct from unrelated HNS and Hoenn progression.
The implementation specification must cover mapped flags, battle identities,
rewards, and all scene entry points; copying standalone FRLG scripts unchanged
is not sufficient. Saving within imported interiors must preserve progress and
provide a valid return to the HNS world.

## Playtesting

- Does Kanto still feel like the same HNS region, with clear entrances and
  consistent building presentation for the added adventures?
- Can a player unfamiliar with FRLG discover the Celadon/Tower connection and
  complete Silph first without confusing dialogue or missing scenes?
- Do the imported dungeons preserve exploration and puzzle progression rather
  than feeling like abbreviated reward rooms?
- Do Mt. Moon and Safari retain their intended objectives in the HNS spaces?
- Can both supported origins complete the adventures in varied orders,
  including with HMs already obtained elsewhere?
- Does the dedicated Anne specification pass its boarding, return, one-time
  content, revisit, isolation, and no-recurring-utility acceptance checks?
- Can an existing Ticket holder rescue Bill later without a duplicate reward
  or an automatic story completion? Can a player without one obtain it normally?
- Can the player leave and return to Kanto throughout every adventure without
  affecting the Aqua circuit or unrelated regional progress?
- Does the separately specified coastal port pass its route, town, service,
  Gym, and cave acceptance checks?
- Are the Tower/radio and Giovanni/Blue choices coherent to a
  player who knows either original game?
- Does a later Blue encounter retire all earlier chapters across save/reload,
  while a loss leaves the current encounter retryable and grants no skipped rewards?
- Can the Tower, Silph, and ship adventures and their ordinary rewards still be
  completed after their Blue scenes have been retired?
- Do visitors receive an appropriate introduction without acquiring Blue as
  their personal rival or changing their original rival?
- Are ordinary rewards recoverable after losses, save/reload, and full Bag or
  party conditions?
- Does the Earth Badge count exactly once, with all 24 badges and the existing
  League circuit still reachable?

## Open questions

1. **HNS story preservation:** Review the linked conflict inventory and select
   adaptations for Vermilion Snorlax, Copycat,
   Bill's family, Fuchsia leadership, and other overlapping content. Lavender's
   Tower/radio and Power Plant choices are settled above. Do not treat a
   recommendation to defer or remove content as an accepted cut.
2. **Battle rosters and leadership:** Define Blue's future Kanto-origin starter
   branches while preserving the circuit's fixed Blastoise roster for visiting
   origins. Resolve Koga/Janine's roles alongside their Sevii Masters Challenge
   appearances; no additional sanctioned badge may be introduced.
3. **Bill and his grandfather:** Sharing the house is an available option;
   sequencing grandfather after the rescue is not required. Adjust his
   housesitting/Bill-in-Johto dialogue and the rescue movement space. Keep the
   Pokémon request quest and Galarian-form machine.
4. **Steven's gift:** Decide whether to retain the Hoenn starter choice as an
   additional local gift for every origin or adapt the reward. It must not
   modify origin selection or consume another region's opening reward.
Exact entrance tiles, per-floor event placement, state allocation, reward
thresholds, and any required supplementary puzzle rooms belong in follow-up
specifications once these product choices are resolved.

## References

- [FRLG Kanto independent story beats](frlg-kanto-independent-story-beats.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Wayfarer regional start choice](wayfarer-regional-start-choice.md)
- [HM field use](hm-field-use.md)
- [S.S. Aqua circuit specification](../specs/wayfarer-hoenn-entry.md)
- [HNS Kanto story traversal audit](../research/hns-kanto-story-traversal-blockers.md)
- [Sevii independent story beats](sevii-independent-story-beats.md)
