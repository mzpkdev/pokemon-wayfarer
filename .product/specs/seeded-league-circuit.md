# Seeded League circuit runtime

PRD: [Seeded trainer circuit](../prds/seeded-trainer-circuit.md)

Implemented: No

Draft successor to the fixed-order [interregional circuit](wayfarer-interregional-league-circuit.md).
The parent PRD distinguishes confirmed requirements from proposed defaults and
owns unresolved decisions D1–D3. The behavior below specifies the proposed
runtime contract; writing it does not approve those defaults or authorize implementation.

## Scope

This specification owns schedule persistence, admission, active runs, first-clear
transactions, retries and replays, recovery, player-facing information, regional
integration, and journey acceptance. The sibling [trainer pool specification](circuit-trainer-pool.md)
owns canonical trainer identity, eligible team profiles, region affinity,
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
position. The saved schedule contains each of Indigo, Masters, and Hoenn once.
Positions 1, 2, and 3 determine qualification and competitive tier. Venue
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

- schedule schema version, separate ORDER/LORE_FILTER/ROSTER rules versions, and
  trainer catalog/content version;
- three ordered venue identities; and
- five ordered slots per venue, each containing canonical character ID, stable
  team-profile reference, and assigned baseline NPC TR.

The root and derivation metadata have one shared owner; do not copy them into
the circuit schedule or save an independent circuit seed or random cursor.
The schedule has fifteen slots, with five distinct canonical character IDs
within each venue. A character may recur across venues when independently
TR-qualified and lore-admitted, using the same fixed profile/TR and therefore
strength unless an explicit challenge override applies. Do not store full parties or
derived levels. Stable profile references resolve into versioned authored
content. Multiple runtime/source Trainer IDs for one person do not create
additional characters within a lineup. Ordinary Gym, story, or Dojo appearances
remain unaffected.

The saved lineup is selected from TR-eligible candidates after the confirmed
lore filter: affiliated characters pass, and each unaffiliated character is
retained or dropped with exactly equal seeded probability per venue. Derive
affiliation from the versioned `leagueAffiliations` and rationale, rather than
map region. Multiple unaffiliated participants may be selected; there is no
visitor quota or separately mutable visitor count. The pool specification owns
filter keys, rating weights, candidate constraints, and independent venue allocation.

Generation is a new-game operation. The sibling's semantic keys determine the
same unresolved outcome for the same root, decision versions, and inputs, even
before first resolution. Once resolved, the saved schedule is authoritative.
Saving, loading, ordinary gameplay RNG, admission, losing, leaving, completion,
and replay cannot reorder venues or reroll slots, profiles, or assigned ratings.
A content update cannot silently
reinterpret an existing schedule through a different trainer-content version.
Unrelated opted-in seeded features, their calls/content, menu opens, fights,
and queries cannot perturb circuit outcomes. Roster catalog changes may alter
new-game roster draws but cannot change ORDER or existing character/venue
LORE_FILTER draws for the same
root and respective rules versions. Global/catalog versions must not be added
to every random key.

Store canonical first-clear facts by venue, separate from Champion projections
and room defeat state. New game initializes all three false and no active run.
Derive the next position from the contiguous cleared prefix of the saved order;
do not keep a separately mutable progress index or reward count. A cleared
later venue with an uncleared predecessor is invalid circuit state.

### Qualification and travel

Derive global badges from the three existing regional badge sets. A badge is
counted once, never spent, and never gated by circuit progress. The proposed
first-clear predicates are:

| Schedule position | Admission |
| --- | --- |
| 1 | At least 8 global badges; venue not cleared |
| 2 | At least 16 global badges; position 1 cleared; venue not cleared |
| 3 | All 24 badges; positions 1 and 2 cleared; venue not cleared |

Only the next uncleared scheduled venue admits a first-clear run. Visiting a
later venue early reveals its requirements without starting a run. Badge
origin, player origin, local quest state, Champion status, or a generic
game-clear flag cannot replace the qualification facts. Cleared venues offer
replays regardless of later progress; replay mode is selected explicitly.

All twenty-four badges remain obtainable before any circuit clear. At 24
badges the player still completes the three scheduled venues in order. Existing
regional badge prerequisites and once-only deferred rewards remain intact.

Every venue must be reachable at its earliest eligible position. Preserve the
S.S. Aqua maiden-voyage/Ticket flow and its Olivine–Vermilion–Slateport service.
Numbered Sevii-island service from Vermilion remains independent of badge count,
clears, Rainbow Pass, Bill/Celio, National Pokédex, and Sevii quests. Masters'
caretaker and hidden door read its scheduled qualification, which may be the
eight-badge opening stop. Entrance requirements apply at the challenge door,
not to island travel. Returns after completion, refusal, loss, exit, or
recovery must connect to the ordinary travel network.

### Active-run record and battle dispatch

Persist an active-run record containing active status, schedule position, venue,
and first-clear/replay mode. Persist sufficient room defeat state to identify a
contiguous prefix of the five actual scheduled opponent slots. Existing room
progression may remain its authority if it validates against this record.
Progression is keyed by venue and actual slot, never a global character defeat
flag or shared Trainer-ID defeat flag. Defeating the same person in another
league cannot skip this slot, complete this run, or grant this venue's clear/reward.

There is no difficulty input from player `ratingAtEntry`. If retained for
diagnostics, it must not affect team selection or effective opponent levels.
Under proposed D1, the saved NPC baseline TR and selected profile determine
strength through the sibling specification, including on replays and party
reconstruction. Reusing a map or changing live player TR cannot change them.

1. Validate schedule, qualification, mode, and destination before locking an
   entrance or changing room state. Denied admission creates no active run.
2. Accepted admission resets only that venue's attempt progress and creates
   the matching run before entering its first room.
3. Resume and normal room transitions preserve the run and defeated prefix.
   They cannot perform admission again or regenerate content.
4. Each battle validates run, venue, position, room, and expected slot, then
   resolves the scheduled character/profile. The slot supplies party, class,
   portrait, sprite, name, introduction, defeat text, music, and authored AI
   through audited content references. Fixed room-owner Trainer IDs do not
   select opponents.
5. A victory advances that slot's progression exactly once. A loss does not
   advance it. Final victory must validate all five scheduled victories before
   a completion transaction can begin.
6. Room and ceremony transitions remain inside the run until completion or
   explicit failure handling ends it.

The proposed format is five single battles in ascending NPC TR; the fifth
opponent is the finalist regardless of historical role, subject to D2. No
additional battle order randomness is introduced by room entry. Pool selection
and tie ordering belong to the sibling specification.

Preserve existing randomizer precedence. An explicitly enabled party randomizer
may replace authored Pokémon according to its own contract, but it cannot
reroll the schedule, change circuit qualification, or create duplicate
character appointments within a lineup. Out-of-context debug battles cannot create runs,
advance slots, or record clears.

### First-clear and ceremony transaction

Commit a first clear only for a valid first-clear run at the next scheduled
position after all five victories. The transaction records the canonical venue
clear, establishes its required regional projections, applies its once-only
ceremony effects, and resets its run/progression before the completion save and
return. Derived player TR reads the committed clear facts.

| Venue cleared, at any position | First-clear effects |
| --- | --- |
| Indigo | One canonical clear; shared Kanto/Johto Champion and game-clear recognition; one FRLG Hall of Fame registration and Champion Ribbon flow |
| Masters | One canonical Masters clear and Gallery presentation; no regional Champion/game-clear result, Hall of Fame registration, Champion Ribbon, or regional story cleanup |
| Hoenn | One canonical clear; Hoenn Champion/game-clear recognition; one Emerald Hall of Fame registration and Champion Ribbon flow; Hoenn-owned regional cleanup |

Every first clear contributes +8 player TR exactly once. Preserve the existing
badge contribution, ceiling of 80, and high-water behavior. Regional Champion
projections are not additional clears. Repeated callbacks, ceremony re-entry,
replays, individual victories, and Red add no circuit TR.

Full completion credits follow the third committed first clear, whichever venue
it is. A first or second Hoenn clear runs Hoenn's regional ceremony and cleanup
without full credits. A third Indigo clear runs its regional ceremony followed
by full credits. A third Masters clear runs its Gallery and full credits while
still awarding no regional title, Ribbon, or Hall of Fame record. The full
credits trigger cannot replace, repeat, or relocate regional cleanup.

Completion processing must be idempotent across duplicate callbacks, map loads,
save/load, and interrupted ceremony handling. Retained clear state cannot lose
its +8 contribution or required once-only ceremony result; unfinished attempts
cannot acquire rewards. A pending completion/ceremony marker may be used if
needed for reliable resume, but it must identify the same venue and schedule
position and cannot admit the next venue before the transaction is complete.
The implementation must document the actual save/ceremony commit boundary.

### Loss, exit, and replay

A loss/blackout or departure ends the attempt, clears its active record, resets
only its room progress, and returns to its own connected lobby: Indigo's League
lobby, Room 1 of the Masters House, or Hoenn's native lobby. Audit every back
warp, voluntary exit, healing/blackout target, Dig, Escape Rope, and ceremony
return. Never route Masters to the HNS Indigo lobby. These outcomes preserve
schedule, committed clears, titles, and player TR.

Retry starts at slot 1 with the same five participants, profiles, battle order,
and levels. Loading a valid unfinished run resumes its saved progress rather
than restarting or granting victories.

A replay uses that venue's original scheduled tier, roster, and levels. Replay
victory may show the ordinary exit presentation, but records no new first clear,
circuit TR, circuit Hall of Fame registration, Champion Ribbon, full credits,
regional cleanup, or Blue unlock reward. It does not move or consume the next
first-clear position. Proposed stronger postgame rematches are outside scope.

### Load validation and damaged state

Validate the shared root and its format/derivation version before circuit
dispatch. Validate schedule schema, ORDER/LORE_FILTER/ROSTER rules and catalog
versions, venue permutation, slot/profile eligibility, five-character
uniqueness within each venue, rating ranges, TR-first lore-filter admission,
and resolvable content references. Validate
clear-prefix consistency and any pending
completion. Validate an active run against the saved position, venue, mode,
current room, actual opponent slot, and contiguous defeat progress.

With a valid schedule and clear state, missing or invalid active-run state
inside a challenge resets that attempt and returns to its own lobby without
rewards. An active record outside its permitted venue/ceremony locations ends
the attempt. Recovery preserves the valid schedule and canonical clears and
must not replace the missing record with a fresh live-rating snapshot.

A missing, damaged, or unsupported root cannot be initialized or replaced on
load, including when an existing schedule appears structurally valid. Follow
the foundation's invalid-save/new-game policy without consuming Pokémon RNG
or attempting a new root. A damaged, incomplete, or unsupported schedule cannot
silently regenerate, especially once clears exist. Use the project's standard invalid-save handling
and block circuit dispatch until a valid save is recovered or a new game is
started. A valid root alone does not authorize rebuilding different content
under acquired clears. Prerelease save migration is not required; do not add a
compatibility path solely to retain old fixed-order saves.

### Itinerary, lineups, and dialogue

Provide a local Trainer Card circuit view available from the beginning that
shows global badges, the complete seeded order, qualification for each position,
and each venue's Locked/Available/Cleared state. Mark the next required stop and
append Circuit complete only after all three canonical clears.

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

Red remains outside the pool. His Mt. Silver admission reads all three canonical
venue clears, independent of schedule order or projected regional flags. Keep
his authored encounter and reward/repeat behavior unchanged.

The proposed Blue Dojo dependency is the first committed scheduled clear,
regardless of venue or whether Blue was drawn anywhere. Keep its authored party
and existing Battle Point rules; circuit completion grants no extra Dojo reward.
Starting or losing a challenge, a raw battle victory, and an unfinished ceremony
do not unlock it. Audit dialogue that assumes defeating Blue or Indigo, while
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
   order. For each, cover badge boundaries 7/8, 15/16, and 23/24, missing prior
   clears, later-venue refusal, cleared-venue replay, and contiguous-prefix
   completion. Test mixed badge origins and duplicate badge awards.
2. For all six orders, run earliest-entry journeys and all-24-badges-first
   journeys. Reach every opening venue without another badge, circuit clear,
   or unrelated story gate. Finish in order and return to public travel after
   loss, voluntary exit, each clear, and credits.
3. Prove schedule/profile/rating stability through save/load in every room,
   entry/re-entry, losses at each of five slots, replay, ordinary RNG use,
   changing live player TR, and reconstruction of an enemy party. Verify the
   five-character per-venue uniqueness rule using characters with multiple
   source IDs. Allow a qualified character across multiple lineups; its
   profile/TR strength stays fixed and victories/rewards are accounted by
   venue and slot independently. A previous win cannot skip its next appearance.
   Resolve identical circuit keys/inputs before and after unrelated seeded
   feature calls/content, menu opens, fights, and queries; assert identical
   unresolved outcomes as well as committed schedules. Change roster content
   and prove the same root and respective rules versions retain venue order
   and existing character/venue lore-filter outcomes. Cover all-zero
   and all-one 64-bit roots (`lo = hi = 0` and `lo = hi = 0xFFFFFFFF`) and
   verify ordinary Pokémon RNG is untouched by root/schedule initialization.
4. Inject wrong venue/position/mode, missing records, noncontiguous defeat flags,
   and invalid room-slot combinations. Recover valid schedules to the right
   lobby without rewards. Inject schedule/version/reference/clear-prefix
   damage and verify invalid-save handling without silent generation. Inject
   missing/corrupt/unsupported root or derivation metadata, including with a
   valid-looking schedule and acquired clears; loading cannot create a root,
   redraw a schedule, or change committed outcomes.
5. Interrupt and repeat completion handling around the commit boundary. Verify
   exactly one canonical clear and +8 contribution, no partial/duplicate
   regional recognition or ceremony effects, and no accidental next-position
   clear. Verify player TR milestones 48/64/80 at earliest first clears and
   56/64/72/80 on the all-badges-first route.
6. Complete each venue in each position. Indigo projects one shared Kanto/Johto
   result and one regional ceremony; Hoenn performs only its own regional
   cleanup; Masters never grants regional status, Ribbon, or Hall of Fame.
   Full credits appear only after position 3 and never after a replay.
7. Prove Blue unlocks after the first committed clear with each opening venue,
   including schedules without Blue, independently of Giovanni. Prove Red
   remains locked with any missing clear and unlocks after all three.
8. Verify saved lineup names, ratings, portraits, sprites, room introductions,
   defeat text, music, and battle parties agree at every slot. Cover visiting
   trainers, Gym Leader finalists, and characters whose map room retains a
   different historical owner's name. Inspect itinerary and lineups before
   qualification, during progression, and after completion.
   Validate all six venue orders with at least five distinct TR-qualified
   affiliated candidates per venue/position, including all unaffiliated
   candidates dropping. Cover multi-league
   affiliations, unsuitable affiliated TR exclusion, gate outcomes 0/1, and
   several unaffiliated participants surviving and being selected without a
   quota. A retry cannot introduce or remove a participant or rerun the gate.
9. Audit deferred badge rewards, regional story branches, Hoenn cleanup,
   S.S. Aqua/ferry state, optional Sevii content, and ordinary battles for
   regressions caused by venue reordering. Shared-engine changes must preserve
   standalone FRLG, HNS, and Emerald authored League behavior.

Extend relevant [mechanics coverage](../../game/test/league_circuit.c),
[script coverage](../../game/test/league_circuit_scripts.c),
[status coverage](../../game/test/league_circuit_status.c), and
[the League E2E journey](../../e2e/src/journeys/wayfarer-league-circuit.e2e.ts).
Build the production Wayfarer configuration and applicable test ROMs; compile
affected standalone configurations when shared code changes. The E2E suite
requires explicit prebuilt ROM and symbol paths and does not build the game.

Release remains blocked on resolving parent PRD decisions, the foundation's
fixed derivation/hash encoding and golden vectors, an audited feasible
pool for every position/venue permutation, and successful save/transaction,
travel, presentation, and journey evidence. Emulator playtesting must assess
actual battle attrition and the player's between-stop training demands under
D1; structural checks alone do not establish balance.

## Open questions

See parent PRD D1 for player-independent strength, D2 for finalist eligibility,
and D3 for the initial roster and ratings. The parent also owns review of the
supplementary defaults: new-game fixation of all slots,
admission thresholds, replay policy, Blue unlock, regional
ceremonies, credits, and itinerary visibility. Any revision must update this
runtime contract and the sibling pool specification together.
