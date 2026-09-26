# Seeded League circuit runtime

PRD: [Seeded trainer circuit](../prds/seeded-trainer-circuit.md)

Implemented: No

Draft successor to the fixed-order [interregional circuit](wayfarer-interregional-league-circuit.md).
The parent PRD distinguishes confirmed requirements from proposed defaults and
owns resolved initial fixed strength (D1) and title-agnostic headliner (D2),
unresolved D3–D5, and supporting defaults.
The recurring championship and rotation direction is approved; the registration
interface, bounded history layout, aggregate record storage,
ceremony rewards, and presentation defaults below remain proposed. This document
does not authorize implementation of unresolved defaults.

## Scope

This specification owns edition registration, schedule persistence, admission,
active runs, edition-result and lifetime-clear transactions, retries and replays,
recovery, player-facing information, regional
integration, and journey acceptance. The sibling [trainer pool specification](circuit-trainer-pool.md)
owns canonical trainer identity, eligible team profiles, authored home leagues,
keyed circuit order/allocation decisions, and the NPC TR-to-level calculation.
The [shared playthrough seed framework](playthrough-seed-framework.md) owns
the root seed, its initialization/persistence and version, pure keyed draws,
and unbiased bounded sampling. This runtime owns the committed circuit outcome.

The three venues retain their selected rooms and public entrances: FRLG Indigo,
the Seven Island Masters House leading into HNS rooms, and Emerald Hoenn. Their
position in the circuit and their five opponents come from the saved schedule.
Regional stories, badge awards, ordinary battles, and travel keep their own
owners except for the explicit dependencies below.

## Behavior

### Venue and position identity

Use a stable venue identity distinct from geographic region and schedule
position. Each edition's saved schedule contains Indigo, Masters, and Hoenn once.
Positions 1, 2, and 3 determine mandatory travel order, not competitive tier.
All venues host comparable championship fields using the same ordered,
disjoint TR role bands: two contenders, two elite trainers, and one headliner.
These roles express TR suitability, not historical titles. Numeric endpoints
remain under D3. Venue
identity determines the map chain, ceremony, regional recognition, and return
location. Trainer identity determines the opponent presented in each room.

Masters continues to resolve to Sevii/Kanto for ordinary regional systems. Its
reused HNS rooms must have a Wayfarer-only map-context override independent of
active-run validity; standalone HNS retains its source map identity. The final
HNS ceremony room is the Masters Gallery. Geographic region, map name, and
an opponent's historical title cannot substitute for saved venue or position.

### Saved schedule

Before new-game circuit state can be saved or displayed, initialize and validate
the foundation root, then generate and validate one complete schedule through
the sibling specification. The proposed foundation stores one 64-bit root as
two `u32` words with seed-format/derivation-version metadata in shared Wayfarer
persistence. Circuit initialization must not consume or reseed Pokémon RNG.
Save the resolved schedule alongside the valid root, with circuit metadata:

- one-based `editionId` (`u32`, initialized to 1);
- schedule schema version, active ORDER/POOL_KIND/ROSTER rules versions, and
  trainer catalog/content version;
- three ordered venue identities; and
- five ordered slots per venue, each containing slot role, canonical character ID, stable
  team-profile reference, and assigned baseline NPC TR.

The root and derivation metadata have one shared owner; do not copy them into
the circuit schedule or save an independent circuit seed or random cursor.
The schedule has fifteen slots, with five distinct canonical character IDs
within each venue. A character may recur across venues when independently
TR/content-qualified and selected from its eligible home or visitor bucket,
using the same fixed profile/TR and therefore
strength unless an explicit challenge override applies. Do not store full parties or
derived levels. Stable profile references resolve into versioned authored
content. Multiple runtime/source Trainer IDs for one person do not create
additional characters within a lineup. Ordinary Gym, story, or Dojo appearances
remain unaffected.

For each slot, first resolve role TR/content eligibility and remove characters
already selected in that venue. Partition the remaining candidates by versioned
`homeLeagues` and authored rationale: home when the set includes the venue,
visitor otherwise. Multiple sensible home leagues are allowed; current map
region does not establish membership. Home membership cannot override TR or
content requirements, and visiting does not change NPC TR or strength.

When both buckets are nonempty, resolve that slot's POOL_KIND `Uniform(100)`:
0–84 selects home, 85–99 selects visitors. When no eligible visitor remains,
use home without a POOL_KIND draw. An empty home bucket is invalid content,
never a reason to substitute a visitor or widen eligibility. Select the trainer
only inside the chosen bucket using strictly positive repeat/history weights.
Home trainers receive those same penalties; category selection prevents a large
global visitor pool from crowding out home entrants through raw pool size.

The initial 85/15 tuning is a conditional category chance when both buckets
exist, not an individual trainer probability or a guaranteed lineup composition.
Multiple visitors may occupy a field; there is no quota, cap, separately mutable
visitor count, or retry/redraw rule. The pool specification owns category keys,
rotation weights, candidate
constraints, and whole-circuit allocation.

Save a bounded history record alongside the current schedule: history schema,
the immediately previous completed edition ID, and its five canonical character
IDs for each stable venue. Edition 1 has explicitly empty history, even after
completion; for every later current edition, `history.editionId = editionId - 1`.
History is keyed by venue, not its former travel position. It is an authoritative
input to current-edition generation, not a display archive. Do not save old full
parties, old profiles, or unlimited schedules.

The pool spec proposes soft rotation from flat base weight 1, a within-edition
scheduled-count factor `[16,4,1]` for counts 0/1/2, and a prior-same-venue factor
1 for a returning character or 2 otherwise. Counts use assigned canonical
characters in generation, never battle wins or persistent encounter flags.
About two returners and three new entrants per venue is a soft target, not a
quota, guarantee, exclusion, or reason to redraw. Missing prior history at
edition 1 applies no returning penalty. Cross-venue repeats remain permitted;
the weights softly discourage repeated appointments. D5 owns numerical tuning.

Generate edition 1 at new game and later schedules only through registration.
The sibling's semantic keys use `occurrenceId = editionId` for ORDER (ID 1),
POOL_KIND (ID 5), and ROSTER (ID 2); all active draft rules versions remain 1.
IDs 3 (VISITOR_POLICY) and 4 (LORE_FILTER) are retired and cannot be reused.
The active keys determine the
same unresolved outcome for the same root, edition, decision versions, and inputs, even
before first resolution. Once resolved, the saved schedule is authoritative.
Saving, loading, ordinary gameplay RNG, admission, losing, leaving, completion,
and replay cannot reorder venues or reroll slots, profiles, or assigned ratings.
A content update cannot silently
reinterpret an existing schedule through a different trainer-content version.
Unrelated opted-in seeded features, their calls/content, menu opens, fights,
and queries cannot perturb circuit outcomes. Roster catalog changes may alter
new-edition roster draws but cannot change raw ORDER or POOL_KIND keys for the
same root, edition, and respective rules versions. Effective bucket fallback
and jointly weighted roster outcomes depend on their actual inputs; changing
eligibility, history, or earlier allocations may change them.
Global/catalog versions must not be added
to every random key.

Store separate canonical current-edition venue results and lifetime first-clear
facts, independent of Champion projections and room defeat state. New game
initializes both three-venue masks false, no active run, and completed count 0.
Derive the next position from the current-edition contiguous cleared prefix;
a later venue result without its predecessor is invalid. Lifetime facts remain
true across rollover and provide progression rewards, recognition, and unlocks.

Persist the current schedule and immediately prior completed roster IDs.
Propose a bounded `u32` aggregate completed-edition count, without an unlimited
archive of old schedules/teams. Count exactly once
when the current edition's third result commits. Consistent state has
`completedCount = editionId - 1` while unfinished and `completedCount = editionId`
when complete. This count tracks edition results, never player TR.

### Registering the next edition

After all three current results commit, the player manually registers for the
next edition. There is no calendar, waiting period, automatic rollover, skip,
next-edition preview, or cancel-to-reroll path. Propose offering registration
at every venue reception while preserving current-edition replay until rollover.
Refuse registration if the edition is incomplete, a run is active, or a ceremony
or result transaction is pending.

1. Bind the registration request to its expected current edition ID. Validate
   that it still matches the saved complete edition, and reject
   `editionId = 0xFFFFFFFF` without changing the save. Derive
   `nextEditionId = editionId + 1` without committing it.
2. Stage a new history snapshot of the completed current edition's five
   character IDs per stable venue and its edition ID. Pass that snapshot to
   generation of the next complete schedule using the same root and next ID.
   Failure retains both the complete current edition and existing history,
   consuming no edition ID. No staged schedule is presented to the player.
3. Revalidate the expected source edition and registration eligibility, then
   atomically persist new history, next ID, full schedule, and empty current-edition result
   mask; reset attempt progress. A duplicate or stale request cannot act on a
   newer edition. Preserve lifetime clears, completed count, badges, player
   TR/high-water, regional titles/cleanup, and Blue/Red access.
4. Return with the committed current itinerary. A crash/save interruption exposes
   either the prior complete edition or the full new edition, never mixed IDs,
   partial rosters, mixed history, inherited room wins, or a counter advanced
   without its schedule. The completed-count invariant remains valid.

Registration retries, including loading a pre-registration save, resolve the
same next-edition keys, staged history, and other inputs. A new edition may coincidentally repeat order or
entrants; do not reroll to guarantee novelty. No alternate occurrence may be
used to recover from a failed transaction. No prerelease migration is required.

### Qualification and travel

Derive global badges from the three existing regional badge sets. A badge is
counted once, never spent, and never gated by circuit progress. D4 proposes a
common 24-global-badge admission gate before the first edition's opening venue;
the same badge requirement applies throughout, replacing 8/16/24 staging.
The edition-result predicates under that proposal are:

| Schedule position | Admission |
| --- | --- |
| 1 | All 24 global badges; no result for this venue in current edition |
| 2 | All 24 global badges; current position 1 result; no current venue result |
| 3 | All 24 badges; current positions 1–2 results; no current venue result |

Only the next uncleared scheduled venue admits an edition-result run. Visiting a
later venue early reveals its requirements without starting a run. Badge
origin, player origin, local quest state, Champion status, or a generic
game-clear flag cannot replace the qualification facts. Cleared venues offer
replays regardless of later progress; replay mode is selected explicitly. Lifetime
clears do not satisfy current-edition prerequisites. Later editions already have
24 badges but still require the three venues in their newly seeded order, using
the same role TR bands at every venue; travel position and later edition are
not difficulty tiers. The common 24-badge gate remains proposed, not approved.

Under D4, player TR is 56 on opening admission, then 64, 72, and 80 after the
three first-ever venue clears. Corresponding soft caps are 62, 78, 89, and 100.
D3 must validate every venue as the first stop against cap 62 and later editions
against cap 100, with fixed NPC strength. A higher player cap does not strengthen
the following venue automatically.

All twenty-four badges remain obtainable before any circuit clear. At 24
badges the player still completes the three scheduled venues in order. Existing
regional badge prerequisites and once-only deferred rewards remain intact.

Every venue must be reachable at its earliest eligible position. Preserve the
S.S. Aqua maiden-voyage/Ticket flow and its Olivine–Vermilion–Slateport service.
Numbered Sevii-island service from Vermilion remains independent of badge count,
clears, Rainbow Pass, Bill/Celio, National Pokédex, and Sevii quests. Masters'
caretaker and hidden door read its scheduled qualification, which may be the
opening stop under the shared gate. Entrance requirements apply at the challenge door,
not to island travel. Returns after completion, refusal, loss, exit, or
recovery must connect to the ordinary travel network.

### Active-run record and battle dispatch

Persist an active-run record containing active status, edition ID, schedule
position, venue, and edition-result/replay mode. Persist sufficient room defeat state to identify a
contiguous prefix of the five actual scheduled opponent slots. Existing room
progression may remain its authority if it validates against this record.
Progression is keyed by edition, venue, and actual slot, never a global character defeat
flag or shared Trainer-ID defeat flag. Defeating the same person in another
league cannot skip this slot, complete this run, or grant this venue's clear/reward.

There is no difficulty input from player `ratingAtEntry`. If retained for
diagnostics, it must not affect team selection or effective opponent levels.
Under resolved D1, the saved NPC baseline TR and selected profile determine
strength through the sibling specification, including on replays and party
reconstruction. Reusing a map or changing live player TR cannot change them.
Future modest dynamic NPC TR is outside this initial contract and requires
separate rating/profile snapshot and replay rules before implementation.

1. Validate schedule, qualification, mode, and destination before locking an
   entrance or changing room state. Denied admission creates no active run.
2. Accepted admission resets only that venue's attempt progress and creates
   the matching run before entering its first room.
3. Resume and normal room transitions preserve the run and defeated prefix.
   They cannot perform admission again or regenerate content.
4. Each battle validates edition, run, venue, position, room, and expected slot, then
   resolves the scheduled character/profile. The slot supplies party, class,
   portrait, sprite, name, introduction, defeat text, music, and authored AI
   through audited content references. Fixed room-owner Trainer IDs do not
   select opponents.
5. A victory advances that slot's progression exactly once. A loss does not
   advance it. Final victory must validate all five scheduled victories before
   a completion transaction can begin.
6. Room and ceremony transitions remain inside the run until completion or
   explicit failure handling ends it.

The approved format is five singles: contender slots 0–1, elite slots 2–3,
then headliner slot 4. Roles use the same disjoint ordered TR bands at every
venue/edition. Sort only within the contender and elite pairs by TR then
canonical character ID. The headliner is title-agnostic under resolved D2;
there is no reserved Champion appointment. The pool spec generates in battle-slot
priority `[4,2,3,0,1]`, visiting each venue in saved travel order for each slot.
POOL_KIND and ROSTER keys use stable venue entities (Indigo 1, Masters 2, Hoenn 3)
and battle-slot draw IDs 0–4. Joint scheduled counts
and prior-venue history couple roster selection across the edition, even though
keyed draws remain pure and unrelated-feature isolation still holds. Room entry
cannot reorder or reroll slots.

Preserve existing randomizer precedence. An explicitly enabled party randomizer
may replace authored Pokémon according to its own contract, but it cannot
reroll the schedule, change circuit qualification, or create duplicate
character appointments within a lineup. Out-of-context debug battles cannot create runs,
advance slots, or record clears.

### Edition-result and lifetime-clear transaction

Commit a venue result only for a valid edition-result run at the next current
position after all five victories. The transaction records that edition/venue
result, applies required once-only effects, and resets run/progression before
the completion save/return. If this venue's lifetime fact is still false, commit
it and its first-ever effects in the same transaction. A lifetime fact cannot
replace this edition's result. Reject callbacks from a stale edition.

The following ceremony/record defaults remain proposed:

| Venue, at any position | Once per edition venue result | First-ever lifetime effects |
| --- | --- | --- |
| Indigo | One FRLG winning-team Hall of Fame registration and Champion Ribbon flow | Shared Kanto/Johto Champion and game-clear recognition; +8 player TR |
| Masters | Gallery result/presentation; no Hall of Fame or Champion Ribbon | Lifetime Masters clear; +8 player TR; no regional Champion/game-clear result or story cleanup |
| Hoenn | One Emerald winning-team Hall of Fame registration and Champion Ribbon flow | Hoenn Champion/game-clear recognition, Hoenn-owned regional cleanup, and +8 player TR |

Masters cannot grant regional status, Hall of Fame, Ribbon, or cleanup in any
edition, even as the final venue. Later Indigo/Hoenn results may record a new
winning team but cannot repeat lifetime recognition or story cleanup. Existing
Ribbon ownership rules apply; this does not create stackable Ribbon copies.
Ordinary battle rewards remain available on every attempt and replay. New
currencies, prizes/items, and detailed prestige UI require separate design.

Only each venue's first-ever clear contributes +8 player TR, at most three
contributions for the playthrough. Preserve badge contribution, ceiling 80,
and high-water behavior. Derive rewards from lifetime facts; current-edition
mask resets cannot lower or re-award TR. Regional projections, later editions,
repeat callbacks, individual victories, replays, and Red add nothing.

The third committed result completes the current edition and increments its
aggregate completed count exactly once. Full story credits follow only edition
1's third result, whichever venue it is. Hoenn early still performs its own
first-ever regional cleanup without full credits. Masters final still runs
Gallery and credits without regional awards. Later editions use a proposed
brief completion presentation and leave full story credits and cleanup alone.

Completion processing must be idempotent across callbacks, loads, interrupted
ceremonies, and saves. Retained current results cannot lose their required
record effects; retained lifetime facts cannot lose their TR or recognition;
unfinished attempts cannot acquire either. Any pending result/ceremony marker
must identify edition, venue, position, and completion phase. Finish the same
transaction before admitting another result or registration. A stale marker
cannot attach to a new schedule or grant its rewards. The implementation must
document the actual save/ceremony commit boundary; no forced autosave is required
solely to prevent rerolls because loading an earlier boundary reuses the same keys.

### Loss, exit, and replay

A loss/blackout or departure ends the attempt, clears its active record, resets
only its room progress, and returns to its own connected lobby: Indigo's League
lobby, Room 1 of the Masters House, or Hoenn's native lobby. Audit every back
warp, voluntary exit, healing/blackout target, Dig, Escape Rope, and ceremony
return. Never route Masters to the HNS Indigo lobby. These outcomes preserve
edition ID, schedule, current results, lifetime clears, titles, and player TR.

Retry starts at the first battle (slot 0) with the same five participants, profiles, battle order,
and levels. Loading a valid unfinished run resumes its saved progress rather
than restarting or granting victories.

A replay uses that venue's scheduled role field, roster, and levels. Replay
victory may show the ordinary exit presentation, but records no new venue result,
circuit TR, circuit Hall of Fame registration, Champion Ribbon, full credits,
regional cleanup, or Blue unlock reward. It does not move or consume the next
edition-result position. Replay cannot register the next edition. The current
schedule remains replayable until manual rollover; only previous roster IDs are
retained for rotation, not a playable old schedule. Battles, defeats, replay,
completion presentation, and ordinary gameplay cannot mutate that history.
Proposed stronger postgame rematches are outside scope.

### Load validation and damaged state

Validate the shared root and its format/derivation version before circuit
dispatch. Validate edition ID, schedule schema, active ORDER/POOL_KIND/ROSTER rules and catalog
versions, venue permutation, role layout, slot/profile eligibility, five-character
uniqueness within each venue, rating ranges, valid home/visitor category resolution,
and resolvable content references. Validate
current-clear-prefix consistency, lifetime facts, completed count, and any pending
completion. Validate an active run against the saved edition, position, venue, mode,
current room, actual opponent slot, and contiguous defeat progress.

Verify category resolution by purely reproducing the canonical allocation and
pair sorting from the saved root, edition, active rules/catalog versions, catalog
baseline ratings, and validated prior-venue history. Compare the complete result
with the saved schedule without modifying either saved inputs or outputs.
POOL_KIND and ROSTER draw IDs identify pre-sort allocation slots; a displayed
room can contain the other slot's selection after sorting. Never validate its
category by comparing the room index directly with a POOL_KIND draw. Missing
inputs or a mismatch follow invalid-save handling, never roster replacement.

Every current venue result requires that venue's lifetime fact. In committed
edition-1 state, the current and lifetime masks must match; editions after 1
require all three lifetime facts already true. Invalid aggregate or edition
relationships use invalid-save handling, not invented completion or rollover.

Validate bounded-history schema, canonical character references, five distinct
IDs within each prior venue, complete stable-venue mapping, and edition relation.
Edition 1 requires empty history; later editions require all three prior venue
lists and `history.editionId = editionId - 1`. A character may recur across
history's different venues. Do not demand current-role eligibility from a past
identity or interpret IDs as battle-defeat flags. The saved versioned current
schedule and assigned baseline TR remain authoritative; history preserves its
generation inputs for read-only verification and never authorizes replacing the
current roster on load.

With a valid schedule and clear state, missing or invalid active-run state
inside a challenge resets that attempt and returns to its own lobby without
rewards. An active record outside its permitted venue/ceremony locations ends
the attempt. Recovery preserves valid history, edition/schedule, current results, and lifetime clears and
must not replace the missing record with a fresh live-rating snapshot.

A missing, damaged, or unsupported root cannot be initialized or replaced on
load, including when an existing schedule appears structurally valid. Follow
the foundation's invalid-save/new-game policy without consuming Pokémon RNG
or attempting a new root. A damaged, incomplete, or unsupported history cannot
be silently cleared, reconstructed, or used to redraw a roster. It follows
invalid-save handling just like a damaged schedule, including when current
lineups appear valid. A damaged, incomplete, or unsupported schedule cannot
silently regenerate, especially once clears exist. Use the project's standard invalid-save handling
and block circuit dispatch until a valid save is recovered or a new game is
started. A valid root alone does not authorize rebuilding different content
under acquired clears. Prerelease save migration is not required; do not add a
compatibility path solely to retain old fixed-order saves.

### Itinerary, lineups, and dialogue

Provide a local Trainer Card circuit view available from the beginning that
shows global badges, current edition number, the complete seeded order, qualification for each position,
and each venue's Locked/Available/Cleared state. Mark the next required stop and
append Edition complete only after all three current results. Propose showing
the bounded completed-edition total. Registration becomes available only when
that edition is complete, with no preview of uncommitted future schedules.

Provide each venue's complete five-person lineup in battle order, with trainer
name, authored specialty, and NPC TR, before entry. It must remain inspectable
after clears and on replay. These views read the saved schedule and cannot reveal a separately
generated preview. Distinguish the player's TR from opponent TR. Public TR
does not require revealing moves, held items, or full parties.

Venue staff state the immediate unmet requirement using actual scheduled venue
names. Masters' caretaker must support first, second, and third position rather
than always requiring Indigo. Fixed Champion/Elite Four room assets may remain,
but displayed opponent graphics, portraits, names, dialogue, and battle metadata
must match the scheduled character. Room labels may describe the venue or slot
instead of asserting a displaced resident is present.

Historical titles describe the trainer's background; the final slot describes
this competition's finalist. Dialogue cannot assume Blue is always the Indigo
opponent, Lance always concludes Masters, or Hoenn always ends the circuit.
Masters never calls its winner a regional Champion. No new automatic itinerary
announcements are required at opening, badge awards, or unrelated interactions.

### Story dependencies and integration audit

Red remains outside the pool. His Mt. Silver admission reads all three lifetime
venue clears, independent of current edition results, order, or projected regional flags. Keep
his authored encounter and reward/repeat behavior unchanged.

The proposed Blue Dojo dependency is the first committed lifetime circuit clear,
regardless of venue or whether Blue was drawn anywhere. Keep its authored party
and existing Battle Point rules; circuit completion grants no extra Dojo reward.
Starting or losing a challenge, a raw battle victory, and an unfinished ceremony
do not unlock it. Blue and Red access persist through all rollovers. Audit dialogue that assumes defeating Blue or Indigo, while
preserving Giovanni's Earth Badge ownership and local finale.

Audit existing checks that equate Indigo with position 1, Masters with position
2, Hoenn with position 3, regional Champion flags with circuit order, or Hoenn
game clear with full credits. Classify each as position-dependent qualification,
venue-specific recognition/cleanup, or unrelated regional story behavior before
changing it. Hoenn's own completion cleanup stays attached to Hoenn; it cannot
be moved wholesale to the final venue. Optional quests, deferred rewards,
voyage state, initial Gym access, and other regions' unfinished stories must
survive every completion order.

Existing integration surfaces to review, not newly available APIs:

- [Save ownership and initialization](../../game/src/wayfarer_persistence.c)
  and [run/save structures](../../game/include/global.h).
- [Current circuit admission and lifecycle](../../game/src/league_circuit.c),
  [script wrappers](../../game/src/league_circuit_scripts.c),
  [script entry points](../../game/data/scripts/league_circuit.inc), and
  [warp/blackout handling](../../game/src/overworld.c).
- [Resolved battle construction](../../game/src/battle_main.c),
  [player TR producer](../../game/src/trainer_rating.c), and
  [post-battle ceremony handling](../../game/src/post_battle_event_funcs.c).
- [Trainer Card](../../game/src/trainer_card.c),
  [circuit status](../../game/src/league_circuit_status.c),
  [Sevii content manifest](../../game/src/data/wayfarer_sevii_maps.json), and
  [Blue Dojo scripts](../../game/data/maps/SaffronCity_FightingDojoVIP_hns/scripts.inc).

### Acceptance and release

Implementation must provide the following evidence; this draft does not claim
that these checks have run:

1. Exercise all six venue permutations, not only seeds that retain the old
   order. Under proposed D4, cover badge boundary 23/24, missing prior
   clears, later-venue refusal, cleared-venue replay, and contiguous-prefix
   completion. Test mixed badge origins and duplicate badge awards.
2. For all six orders, collect all 24 badges before the first circuit attempt
   under proposed D4. Reach every opening venue without another badge, circuit clear,
   or unrelated story gate. Finish in order and return to public travel after
   loss, voluntary exit, each clear, and credits.
3. Prove schedule/profile/rating stability through save/load in every room,
   entry/re-entry, losses at each of five slots, replay, ordinary RNG use,
   changing live player TR, and reconstruction of an enemy party. Verify the
   five-character per-venue uniqueness rule using characters with multiple
   source IDs. Allow a qualified character across multiple lineups; its
   profile/TR strength stays fixed and victories/rewards are accounted by
   edition, venue, and slot independently. A previous win cannot skip its next appearance.
   Resolve identical circuit keys/inputs before and after unrelated seeded
   feature calls/content, menu opens, fights, and queries; assert identical
   unresolved outcomes as well as committed schedules. Change roster content
   and prove the same root, edition, and respective rules versions retain venue order
   and raw venue/slot POOL_KIND values, while allowing input-dependent fallback
   and jointly weighted roster changes. Cover all-zero
   and all-one 64-bit roots (`lo = hi = 0` and `lo = hi = 0xFFFFFFFF`) and
   verify ordinary Pokémon RNG is untouched by root/schedule initialization.
4. Inject wrong edition/venue/position/mode, missing records, noncontiguous defeat flags,
   and invalid room-slot combinations. Recover valid schedules to the right
   lobby without rewards. Inject schedule/version/reference/clear-prefix
   damage and verify invalid-save handling without silent generation. Inject
   missing/corrupt/unsupported root or derivation metadata, including with a
   valid-looking schedule and acquired clears; loading cannot create a root,
   redraw a schedule, or change committed outcomes.
5. Interrupt and repeat completion handling around the commit boundary. Verify
   exactly one current venue result and, only if first-ever, one lifetime fact
   and +8 contribution. No partial/duplicate recognition or ceremony effects,
   completed-count increments, or accidental next-position result is allowed.
   Verify edition-1 TR progression 56/64/72/80 and caps 62/78/89/100 under D4;
   later editions never change progression TR.
6. Complete each venue in each position. Indigo projects one shared Kanto/Johto
   result and one regional ceremony; Hoenn performs only its own regional
   cleanup; Masters never grants regional status, Ribbon, or Hall of Fame.
   Full credits appear only after edition 1 position 3 and never on later
   editions or replays. Later Indigo/Hoenn results apply proposed winning-team
   records/Ribbon flows once per edition, never repeat lifetime cleanup.
7. Prove Blue unlocks after the first committed clear with each opening venue,
   including schedules without Blue, independently of Giovanni. Prove Red
   remains locked with any missing clear and unlocks after all three.
8. Verify saved lineup names, ratings, portraits, sprites, room introductions,
   defeat text, music, and battle parties agree at every slot. Cover visiting
   trainers, Gym Leader finalists, and characters whose map room retains a
   different historical owner's name. Inspect itinerary and lineups before
   qualification, during progression, and after completion.
   Validate each venue's two contenders, two elite trainers, and headliner
   against the same ordered disjoint role bands, independent of travel position.
   Prove home-only feasibility: at least two distinct home contenders, two
   home elites, and one home headliner per venue. Cover authored multi-home
   membership, unsuitable home TR exclusion, no eligible visitors (home with
   no category draw), missing home (invalid catalog), category boundaries 84/85,
   and multiple visitors selected without a quota or cap. Apply eligibility and
   within-venue alias deduplication before partitioning. Trainer weights may
   select only from the chosen bucket and remain positive for home and visitors.
   A retry cannot introduce or remove a participant or rerun category selection.
   Cover a home/visitor pair swapping positions under TR sorting: read-only
   canonical allocation verification accepts it and rejects tampering without
   replacing the saved lineup.
9. Audit deferred badge rewards, regional story branches, Hoenn cleanup,
   S.S. Aqua/ferry state, optional Sevii content, and ordinary battles for
   regressions caused by venue reordering. Shared-engine changes must preserve
   standalone FRLG, HNS, and Emerald authored League behavior.
10. Finish an edition in every final venue and register at each reception.
    Refuse incomplete/active-run/pending-ceremony registration, edition skip,
    future preview, and overflow. Inject failures before generation, after
    staging, and around commit/save boundaries. Preserve either the entire
    old complete edition with old history or entire new edition with a snapshot
    of the completed predecessor's IDs, never consume a counter on failure.
    Loading a pre-registration save and registering again must yield
    the same next schedule; cancellation cannot provide another candidate.
    A duplicate or delayed registration request bound to a prior edition cannot
    start another edition, including after that newer edition is completed.
11. Verify all three decisions use the one-based edition occurrence and respond
    to a changed edition key, without asserting every order/roster must differ.
    Permit identical consecutive schedules and prohibit forced-novelty rerolls.
    Loss, abandonment, retry, replay, and loading cannot increment edition.
    Rollover resets only current results/attempt state; retain lifetime facts,
    badges, TR/high-water, Blue/Red access, regional titles/cleanup, and completed
    count. Reject stale-edition battle/result callbacks and pending markers.
    Later editions remain sequential under D4, with the same role bands,
    profiles/TR, and no automatic difficulty growth. Verify aggregate count
    consistency and once-only increment across duplicate final callbacks.
    Reject edition-1 saves whose current and lifetime clear masks differ.
12. Verify history empty for edition 1 and exactly previous-completed-edition
    IDs by stable venue thereafter, across all six old/new travel orders.
    Gameplay wins/losses, retries, replays, and save/load cannot mutate it.
    Reject corrupt/missing history, wrong schema/IDs/venue mapping, duplicates
    within a venue, and wrong edition relation without clearing history or
    regenerating current choices. Keep cross-venue repeats valid. Derive the
    same staged next schedule from unchanged completed roster/history inputs
    around interrupted registration, including duplicate/stale requests.
13. Validate deterministic allocation priority `[4,2,3,0,1]`, then venues in
    saved travel order, using ROSTER venue entities and battle-slot draw IDs.
    Verify within-edition occurrence counts and prior-same-venue history affect
    proposed weights, never battle results. A relevant roster change can affect
    other venue selections through those counts; raw ORDER and POOL_KIND keys
    remain isolated. Soft repeated appointments and arbitrary returner counts
    are valid; never impose a two-returner/three-new quota or novelty reroll.

Extend relevant [mechanics coverage](../../game/test/league_circuit.c),
[script coverage](../../game/test/league_circuit_scripts.c),
[status coverage](../../game/test/league_circuit_status.c), and
[the League E2E journey](../../e2e/src/journeys/wayfarer-league-circuit.e2e.ts).
Build the production Wayfarer configuration and applicable test ROMs; compile
affected standalone configurations when shared code changes. The E2E suite
requires explicit prebuilt ROM and symbol paths and does not build the game.

Release remains blocked on resolving parent PRD decisions, the foundation's
fixed derivation/hash encoding and golden vectors, an audited feasible
pool for every venue/role pairing, and successful save/history/transaction,
travel, presentation, and journey evidence. Emulator playtesting must assess
actual battle attrition for every venue first against cap 62, later-edition
challenge against cap 100, and rotation variety under proposed D5 weights.
This balance requirement is conditional on D4's proposed 24-badge admission.
NPC strength stays fixed under initial D1; structural
checks alone do not establish balance.

## Open questions

See resolved parent PRD D1 for initial fixed player-independent strength and D2
for title-agnostic headliners; open D3 for catalog/role bands, D4 for common
qualification, and D5 for rotation weights. The parent
also owns review of supplementary defaults: edition fixation of all slots,
admission thresholds, replay policy, Blue unlock, regional
ceremonies, credits, itinerary visibility, reception registration, bounded
aggregate records, and per-edition winning-team records/Ribbon flows. Any revision must update this
runtime contract and the sibling pool specification together.
