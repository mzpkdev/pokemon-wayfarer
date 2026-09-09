# FRLG Kanto story on HNS maps

Status: Partially implemented through the bounded [story milestones](../specs/frlg-kanto-story-milestones.md).
PR #86 delivers the completed milestones listed there; future milestones and
their unresolved decisions do not block that scoped PR. The full umbrella
design remains incomplete and is not yet released. FRLG is Kanto's narrative
baseline and HNS is its general geographic base. The full FRLG Cinnabar port is
selected; the S.S. Anne remains permanently available, with travel design
owned by a separate specification. Selected HNS story adaptations are
recorded below; remaining choices stay open.

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
or rebuild the entire overworld to match FRLG. A later Blue takeover of Viridian Gym is a potential future feature, not an
event assumed to have already happened. Cinnabar remains intact; an eruption
is neither planned by this PRD nor required for story coherence.
ROM space recovery is required before merge and does not drive cuts to this
product design. The story task may implement and validate content before that
recovery, then rebase on it before merging.

## Design

### Keep HNS geography and add selected story interiors

HNS remains the base for Kanto's exterior maps, route connections, settlements,
and links to Johto. Preserve existing traversal and landmarks wherever the
story can use them. Adapt NPC positions, movement, encounters, and entrances to
those spaces rather than requiring FRLG's exterior coordinates.

Import the Celadon Rocket Hideout, Pokémon Tower, Silph's missing floors, and
Pokémon Mansion as the principal new story interiors. Use the existing FRLG
layouts and their recognizable puzzles as the starting point. A small entrance
or transition adjustment is acceptable. Cinnabar is the selected exception for a
full city-map replacement. Other wholesale
city or route replacements remain outside this design.

Do not replace these adventures with a single room containing their final
battle and reward. The player should investigate the hideout, navigate Silph,
climb the Tower, and explore the Mansion. Exact layout reuse, shared tilesets,
floor connections, and entrance positions belong in the implementation specs.

### Cinnabar: full FRLG port, intact throughout the adventure

Port the complete FRLG Cinnabar exterior and its Mansion, Gym, laboratory,
Pokémon Center, and Mart into Wayfarer. Keep Blaine in Cinnabar Gym and preserve
Mansion exploration, the Secret Key requirement, and fossil revival at the
laboratory. The island remains intact throughout the scoped adventure.

Retain HNS Routes 20 and 21. Adapt their reciprocal connections and verify
shore alignment, traversable water, and tileset compatibility at seamless
borders. The full island port is selected; exact connection offsets and any
necessary local seam adjustments belong in the implementation specification.
Also adapt interior exits, Fly and healing destinations, map identity,
encounters, and services to Wayfarer. Do not import Bill's Sevii departure as
part of this mainland map port.

Replace HNS dialogue that describes Cinnabar's destruction as completed history.
Blaine's Seafoam relocation is not part of this campaign. Preserve Seafoam's
compatible exploration and encounters without a second active Blaine Gym.

An erupted island may never be implemented. Do not resize the HNS island,
link a second playable island, reserve transition state, or build routing,
evacuation, or reward handling for a hypothetical eruption. Existing HNS source
assets may remain in the repository for standalone HNS; their presence creates
no Wayfarer feature commitment. Any future eruption would need its own design.

### Adventure coverage

| Adventure | Adaptation requirement |
| --- | --- |
| Mt. Moon | Place the Rocket encounters, Super Nerd battle, and fossil choice in the existing HNS cave where practical. Preserve the local sequence and reward choice; reproducing FRLG's full cave topology is not required. |
| Cerulean burglary | Use the HNS city and a suitable house/back-exit arrangement for the theft, Rocket confrontation, and recovered TM. Local doorway or event changes are allowed. |
| Nugget Bridge | Adapt the ordered Trainer challenge, prize, and recruitment attempt to HNS Route 24. Avoid overlapping its existing Rocket scenes. |
| Bill and the S.S. Anne | Preserve Bill's rescue and Ticket reward, plus the captain's adventure and Cut reward aboard a persistent Anne. Any S.S. Ticket permits boarding; Bill is not a prerequisite for players who already have one. Anne travel is specified separately. |
| Celadon Rocket Hideout | Connect imported hideout interiors to the HNS Game Corner. Preserve discovery of the entrance, local key and lift progression, Giovanni, and the Silph Scope reward. Mahogany's HNS Rocket Hideout remains a separate Johto location. |
| Pokémon Tower and Fuji | Restore the Tower adventure in Lavender, including the Scope-dependent ghost identification, Marowak resolution, Rocket rescue, Fuji's return, and Flute handoff. Resolve its relationship to the HNS radio station explicitly. |
| Snorlax | Preserve the two FRLG encounters associated with Routes 12 and 16 and the Flute requirement, adapting event placement to HNS geography. Neither encounter may close the only ordinary travel route. Resolve the existing HNS Vermilion Snorlax separately rather than accidentally duplicating the same encounter. |
| Silph Co. | Extend the existing Saffron destination with the missing interior adventure. Preserve access puzzles, staff rescue, Giovanni, Lapras, and the immediate post-liberation Master Ball reward. |
| Safari Zone and Warden | Use HNS Fuchsia's existing Safari areas for the Surf destination and Gold Teeth search. Keep returning the Teeth for Strength as a separate local objective. Do not import the entire FRLG Safari merely to reproduce coordinates. |
| Mansion and Blaine | Add the Mansion exploration and Secret Key reward. Preserve its causal connection to Blaine's challenge; Blaine belongs in the intact Cinnabar Gym for the FRLG baseline. Use the selected full FRLG island and interiors. |
| Giovanni's finale | Preserve the final confrontation after the two major investigations. Giovanni owns the initial Viridian challenge and Earth Badge. Blue is the rival only for Kanto-origin players and the Kanto Champion for all origins; a later Gym takeover is future work. |
| Legendary sites | Reuse HNS Seafoam, Power Plant, and Cerulean Cave where their spaces support the encounter and exploration requirements. Any missing puzzle space needs a named, bounded addition rather than an automatic full FRLG dungeon import. |

The fossil reward has a usable revival service at the restored Cinnabar
laboratory. Preserve its local reward handling and access independently of
unrelated regional campaign completion.

### Preserve local causes and allow different adventure orders

Adopt the adventure dependencies and reward rules in
[FRLG Kanto independent story beats](frlg-kanto-independent-story-beats.md).
In particular:

- Celadon Hideout leads to the Scope, then the Tower rescue and Flute.
- Silph is independently available without requiring the Tower rescue.
- Both major investigations lead to Giovanni's finale. Smaller Rocket
  incidents and the Snorlax encounters are not finale prerequisites.
- Bill provides a route to the S.S. Ticket and Anne boarding; an existing
  Ticket also permits boarding. The Mansion's Key leads to Blaine's challenge.
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

### One coherent Kanto cast and history

The player experiences one version of each shared character and institution.
Do not leave an HNS NPC describing an event as long finished while the imported
FRLG adventure presents that same event as unresolved.

Preservation is the default. Each overlapping HNS scene must be explicitly retained, adapted, or deferred
in a follow-up content specification. This includes the Machine Part theft,
Misty's absence, the radio upgrade, Vermilion Snorlax, Blue's invitation and Gym
role, Blaine at Seafoam, and dialogue about Cinnabar's condition. Unrelated HNS
flavor, services, and side content remain available unless a named conflict
requires a change. Dropping content requires a named conflict and a product
decision after considering dialogue, event placement, and dependency changes.
The [HNS Kanto conflict inventory](../research/frlg-hns-kanto-story-conflicts.md)
records recommendations separately from accepted decisions.

If the Machine Part or radio story is replaced, specify how Misty becomes
available and how the Magnet Train's power, Copycat's Pass, and any retained
radio functions work afterward. Removing a predecessor scene must not leave
one of these services or rewards permanently waiting for an unreachable event.

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
- Kanto's League lineup is Lorelei, Bruno, Agatha, Lance, then Blue. Johto keeps
  Will, Koga, Bruno, Karen, then Lance. Retain circuit qualification, order,
  scaling, and reward accounting. Blue follows the origin-specific progression
  below; Koga's dual institutional roles still need their own decision.

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
  Cinnabar exception, new parallel Kanto,
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

Giovanni and Blue cannot each award an initial Earth Badge. Preserve one Kanto
badge identity, one first-award contribution to TR, and normal rematch behavior.
For this adaptation, Giovanni replaces the existing League circuit's Blue
invitation route to the initial Earth Badge. Blue's HNS Gym scenes are
candidates for a future succession transition, not concurrent Gym ownership. No new TR,
badge-count, or League-clear condition is introduced for that badge.

Keep ordinary local rewards attached to their adventures even when the player
already owns the corresponding HM or can use the field move through another
region's progression. A prior acquisition must not auto-complete the story.
Repeated conversations cannot repeatedly award one-time items or Pokémon.
Bag and party capacity failures leave rewards claimable later.

The [Silph President Master Ball specification](../specs/silph-president-master-ball.md)
provides an immediate post-liberation reward with no readiness threshold,
success-only receipt, and full-pocket retry. Elm's later reward remains, so
two Master Balls are intentional. An unclaimed Silph Master Ball does not
delay Silph's recovery or Giovanni's finale.

Retain the independent-story design's readiness requirements for legendary
captures, with thresholds decided separately. No Sevii delivery requirement
is imported for Mewtwo.

### Transport and regional access

Keep the S.S. Anne as a persistent, distinct ship. Helping the captain and
receiving Cut complete its local adventure without permanently departing,
removing, or closing the ship. Players may leave and return to its interiors
and unfinished content. Adapt the original farewell dialogue and departure
scene to this persistent presence.

Any S.S. Ticket permits Anne boarding, regardless of its regional source.
Bill remains the normal Ticket source for the planned Kanto opening. Players
who obtained a Ticket elsewhere can board without first rescuing Bill. His
rescue remains independently playable afterward; acknowledge an existing
Ticket without awarding a duplicate or skipping the rescue. Possessing the
Ticket does not complete the captain's adventure or grant Cut.

Retain the Ticket after the captain's reward and all ship visits. Completing
the Anne adventure cannot reset or revoke Aqua travel. Preserve the existing
Aqua circuit: Olivine to Vermilion to Slateport to Olivine. Shared possession
of the Ticket does not authorize this feature to alter Aqua's other existing
travel rules or initialization.

### Vermilion boarding: existing sailor and dock

Add **Board S.S. Anne** to the existing Vermilion dock sailor's harbor menu.
Retain its existing Slateport, Southern Island, Birth Island, Faraway Island,
Battle Frontier, and Exit options. This is an additive entry, not a new
ship-selection submenu or a replacement three-option menu. Preserve existing
option identifiers/indices and route handlers when extending the list; menu
placement and input handling belong in the implementation specification.

Slateport remains Aqua's fixed next regional stop. Aqua continues its
Olivine → Vermilion → Slateport → Olivine circuit. Its existing regular-trip
eligibility, Ticket checks, arrival initialization, and departure presentation
remain intact. The inherited special destinations remain in this same menu
with their existing eligibility and credential rules. They do not become
alternative regional circuit ports or require a separate NPC interaction.

Make Anne boarding available independently of regular Aqua eligibility.
Move any whole-menu Aqua eligibility check to the existing service branches
as needed, so an Anne Ticket holder can reach the new option without gaining
premature access to the old services. Preserve those services' prior eligibility,
including the regular-service prerequisite for the special destinations.
Do not change Olivine's menu or Slateport's Aqua yes/no interaction in this port.

Selecting Anne checks for any S.S. Ticket, then fades and warps directly into
Anne's explorable interior. It must not be hidden behind Aqua's regular-voyage
eligibility check or trigger its maiden-voyage state. Without a Ticket, give
the normal refusal and leave the player at the dock. Cancel leaves the player
at the dock without changing story or transport state.

Leaving Anne returns the player to a safe walkable position in the same
Vermilion dock. Do not automatically sail, reopen the menu, or reboard. Anne
remains available after the captain's reward and on later visits.

Reuse the existing dock terrain, sailor, and displayed Aqua ship graphic.
A brief boarding message and fade are sufficient to represent transfer to the
Anne. This intentionally simple presentation does not require a second visible
ship, a second berth, a dock variant, a new exterior map, or dock `map.bin`
edits. Anne's own interiors and their events still require content integration.
A later presentation improvement is not a prerequisite for this story port.

### Anne travel: separate specification

This PRD settles Vermilion boarding and the persistent onboard story, not
Anne's transport network. A separate travel specification owns its future
destinations, direction, other ports, schedules, travel menus, and arrival
integration. No reverse Aqua circuit or alternate city is selected here.
The additional Vermilion boarding option above is settled, not deferred to that work.
Do not make future travel design a prerequisite for the local boarding flow.

Future travel must preserve onboard state and permit the player to leave
without completing an optional adventure. Sailing must not reset Trainers,
repeat one-time rewards, complete Bill's rescue, or retire Blue's Anne scene.
That scene follows only the origin-specific forward-progression rule above.
The original one-time ship departure must not be used to discard unfinished
content. Exact interior/event integration belongs in the content specification.

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
persists, any existing S.S. Ticket permits boarding, and Bill's rescue remains
available independently. His reward still provides a route to boarding for a
player without the Ticket. Travel routing is outside this story PRD.

### Superseded transport and League requirements

The following are approved future Wayfarer changes, not claims about current
implementation. Their corresponding specifications record the same scoped
overrides; the runtime and existing audits remain unchanged in this docs-only
proposal.

- **Vermilion menu extension:** Supersede the
  [Aqua specification](../specs/wayfarer-hoenn-entry.md#approved-future-vermilion-menu-extension)'s
  restriction that Wayfarer only replaces menu slot 0 and otherwise keeps an
  exact six-entry menu. Preserve slots 0–5, including Exit, and append Anne at
  slot 6. Existing service eligibility and destination behavior remain binding.
  The implementation must revise `game/tools/wayfarer_hoenn_entry/generate.py`
  to accept the additional entry and dispatch case, and to verify preserved
  original slots plus per-service eligibility. Keep its standalone HNS checks
  unchanged. Do not disable the audit or change its acceptance rules before
  implementing the feature it validates.
- **Separate League opponents:** Supersede the
  [League circuit specification](../specs/wayfarer-interregional-league-circuit.md#approved-future-kanto-and-johto-lineups)'s
  shared HNS opponent-lineup requirement. Kanto's authored Tier 1 lineup is
  Lorelei, Bruno, Agatha, Lance, and Blue; Johto's authored Tier 2 lineup remains
  Will, Koga, Bruno, Karen, and Lance. Preserve the shared Indigo venue, circuit
  admission and order, scaling, regional state isolation, save/load recovery,
  loss/retry, rewards, and Hall-of-Fame guarantees. Update affected roster
  validation with the implementation; this proposal does not change ROM code.

The same future port explicitly supersedes the League PRD and specification's
Blue Cinnabar invitation, initial Viridian Gym ownership, and Blue-specific
initial-badge validation. The
[owning specification](../specs/wayfarer-interregional-league-circuit.md#approved-future-viridian-badge-ownership)
assigns those responsibilities to Giovanni's finale while retaining a single
Earth Badge, once-only accounting, and reward recovery. Blue's later succession
is not required. Current runtime continues to use Blue until implementation.

Other conflicts are not implicitly resolved. The decisions below must be
recorded here and reconciled with the affected story, League, or transport
requirements before implementation. Until then, the current runtime behavior
remains the baseline, not proof that an open choice has been made.

## Constraints

Space recovery is a merge prerequisite handled by separate work. Development
on the story task may exceed the 512 KiB reserve while recovery is pending;
this does not waive the production release check or the 32 MiB address-space
limit. Rebase on recovery and validate the complete release budget before merge.
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
- Does the Vermilion sailor retain every existing option and add Anne boarding,
  without changing the directional circuit, old option behavior, or dock terrain?
- Can an Anne-eligible Ticket holder board even without regular Aqua eligibility,
  with cancellation and Ticket refusal leaving both ships' state untouched?
- Does leaving Anne return safely to the existing dock without automatic travel
  or reboarding, and do special services retain their original eligibility?
- Can a player with a Ticket from any origin board Anne, help the captain,
  leave, and revisit afterward without losing unfinished content or the Ticket?
- Can an existing Ticket holder rescue Bill later without a duplicate reward
  or an automatic story completion? Can a player without one obtain it normally?
- Do repeat Anne visits preserve one-time rewards and Blue's actual progression?
- Can the player leave and return to Kanto throughout every adventure without
  affecting the Aqua circuit or unrelated regional progress?
- Does the intact Cinnabar connect correctly to both HNS sea routes, with
  reliable interior exits, Fly/healing arrivals, and no post-eruption dialogue?
- Are the Tower/radio, Cinnabar/Blaine, and Giovanni/Blue choices coherent to a
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
   adaptations for the Power Plant setting, radio services, Snorlax, Copycat,
   Bill's family, Fuchsia leadership, and other overlapping content. Do not
   treat a recommendation to defer or remove content as an accepted cut.
2. **Battle rosters and leadership:** Define Blue's Champion roster for visitors
   and the future Kanto-origin starter branches. Resolve Koga/Janine's roles
   alongside HNS Johto League content; no additional sanctioned badge may be
   introduced.
3. **Bill and his grandfather:** Sharing the house is an available option;
   sequencing grandfather after the rescue is not required. Adjust his
   housesitting/Bill-in-Johto dialogue and the rescue movement space. Keep the
   Pokémon request quest and Galarian-form machine.
4. **Steven's gift:** Decide whether to retain the Hoenn starter choice as an
   additional local gift for every origin or adapt the reward. It must not
   modify origin selection or consume another region's opening reward.
5. **Anne travel: separate specification.** Define the future route, other
   ports, schedules, and travel presentation outside this story PRD. Vermilion
   sailor boarding, shared Ticket eligibility, and persistent onboard content
   are settled. Existing special destinations remain on the Vermilion sailor
   menu; no additional NPC interaction is required.

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
