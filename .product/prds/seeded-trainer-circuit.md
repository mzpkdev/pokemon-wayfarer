# Seeded Trainer Circuit

Implemented: No
Design status: Draft; recurring regional championships and trainer-owned world
progression are approved directions. D1 is reopened for the growth and snapshot
contract. Numeric balance, qualification, and supporting defaults remain under review.

## World progression revision and balance explorer

The latest direction supersedes the earlier static-strength D1 decision:
trainers have their own starting TR and personal growth as global badges and
first league clears accumulate. Gym battles should use the trainer's TR rather
than the player's TR. Late Gym Leaders also need approachable starting teams;
their original parties inform identity and later targets, not mandatory opening
strength. Blue stays approachable early but reaches Champion strength sooner.

The [trainer balance explorer](../../devtools/ui/README.md#trainer-balance-explorer)
ships with this draft to exercise that direction. It exposes 37 trainers, badge
and first-clear controls, editable growth checkpoints and team stages, original
party references, and saved/exportable experiments. Its curves, level anchors,
party stages, and league-growth amounts are provisional. It models species,
party size, and levels; it does not simulate combat outcomes or generate leagues.

The detailed circuit strength, profile, and edition-snapshot clauses below and
in the companion specs still describe the earlier proposal and need a coordinated
D1/D3/D4 revision before implementation. In particular, resolve when advancing
the world can change a scheduled opponent's strength while retries and reloads
remain deterministic. The explorer does not settle that lifecycle or implement
ROM behavior. Seed keys, home/visitor selection, and soft rotation retain their
approved direction.

## Intent

Make Indigo, Sevii Masters, and Hoenn recurring regional championships with
recognizable competitors and changing fields. Each edition contains all three
venues in a seeded travel order. Every venue offers a comparable competition:
two contender fights, two elite fights, and one headliner. Travel position no
longer makes a whole league weak, intermediate, or strong.

Trainer Rating describes a competitor's strength. Participation changes between
editions without requiring large rating swings. Returning trainers remain part
of the world; an edition should expose a selection of the pool rather than try
to include every established Elite Four member and Champion.

## Design

### Approved direction

- Replace fixed lineups with a shared pool of recognizable Gym Leaders, Elite
  Four members, and Champions. Each canonical person has an authored baseline TR
  that determines competitive eligibility and battle strength.
- All three venues use the same strength roles: two contenders, two elite
  competitors, and one headliner. Roles depend on TR, not historical titles.
  A qualified Gym Leader may be a headliner. Seeded travel order remains mandatory
  for edition results but does not change the role bands or opponent strength.
- Give trainers personal growth from world milestones. Rotation must still
  provide variety when ratings change slowly or reach their mature values;
  merely registering another edition must not create endless strength inflation.
- Apply TR eligibility first. An unsuitable trainer cannot qualify through
  home membership, lack of alternatives, or a desire for roster turnover.
- Give each trainer an authored `homeLeagues` set with lore rationale; several
  home leagues are allowed. After eligibility, choose a home candidate pool with
  85% probability or a visitor pool with 15% probability for each slot, then
  select a trainer within that pool using participation weights. If no eligible
  visitors remain, choose home. This replaces the former outsider drop filter.
- Five distinct canonical people appear within a league. Cross-venue appearances
  within an edition remain possible but receive a soft selection penalty.
  Alternate teams, Trainer IDs, costumes, and titles do not bypass identity.
- Encourage partial turnover between editions. Roughly two returning and three
  different opponents per venue is a tuning target, not a reserved-slot quota.
  Recent participation reduces selection weight; missing an edition at that
  venue removes the previous-participation penalty. No lifetime exclusion exists.
- Complete all three leagues before manually registering for the next edition.
  No calendar wait, automatic rollover, skip, or cancel-to-reroll path applies.
- Derive the whole schedule from the shared playthrough root, edition identity,
  and defined content/history inputs. Retries, replays, and reloads retain it.
  Existing Pokémon gameplay RNG and randomizers keep their current behavior.

### Strength and field composition

Propose three ordered, non-overlapping inclusive NPC TR bands, shared by every
venue and edition. D3 owns their numeric endpoints and the content review.

| Battle slots | Role | Slots per venue |
| --- | --- | ---: |
| 1–2 | Contender | 2 |
| 3–4 | Elite competitor | 2 |
| 5 | Headliner | 1 |

A trainer's baseline places them in a role; their title does not. Sort the
contender pair and elite pair by TR, with stable character identity breaking
ties. The headliner band is above both other bands. Do not use the old whole-league
position targets 40/56/72, or lower a trainer's baseline just to increase attendance.

Each trainer has one reviewed six-member competitive singles profile. TR sets
levels independently of the player's TR or party; authored species, moves,
items, ace, and AI supply the recognizable challenge. The same profile and
saved TR give the same strength across eligible appearances. Initial Gym,
story, rival, and personal rematch encounters remain separate.

Initial membership is supported Kanto, Johto, and Hoenn characters under D3.
Red remains a separate mastery encounter. The circuit uses single battles;
Tate and Liza are explicitly excluded from the initial pool.
Story availability does not remove a trainer from sanctioned competition.

### Home leagues and visitors

Home membership describes where a trainer is a regular competitor, rather than
their current map location. A trainer can have several home leagues when each
membership has a lore rationale. Other venues treat that trainer as a visitor.
For example, a Brock entry with only Indigo in `homeLeagues` joins Hoenn's visitor
pool only when his TR qualifies for the slot. D3 reviews the actual memberships.

The initial 85% home / 15% visitor split applies when both eligible pools contain
candidates. Pool sizes and participation weights do not change that category
chance, so adding more distant trainers cannot overwhelm the home field. This
is neither an individual trainer's appearance chance nor a guaranteed lineup
ratio: a venue may draw zero, one, or several visitors. There is no visitor quota,
reserved guest slot, or maximum guest count.
The draw applies while allocating entrants; sorting each role pair by TR can
then change which room a home trainer or visitor occupies.

Home status removes the regional disadvantage; it does not remove penalties for
recent participation. Visiting never reduces a trainer's TR or battle strength.
Every venue must remain fillable from home candidates alone. Missing home
candidates is a content error, not permission to relax TR or force a visitor.

### Rotation between and within editions

Build the three fields together. A trainer scheduled at another venue receives
less weight, but remains eligible. The immediately preceding completed edition's
lineup at the same venue supplies the returning-participant signal. Missing that
venue for one edition clears that signal, even if the trainer competed elsewhere.
Gym battles, losses, replays, and room visits do not count as participation inputs.

Propose flat base weight inside each approved TR role, replacing the previous
sharp target-proximity weighting. Under D5, initially test these integer factors:

| Condition | Factor |
| --- | ---: |
| Already scheduled at zero / one / two venues in this edition | 16 / 4 / 1 |
| Appeared at this venue in the immediately previous edition | 1 |
| Did not appear there, or this is edition 1 | 2 |

Multiply the applicable factors within the selected home or visitor pool. Every
eligible candidate there has positive weight; no preference overrides a TR band
or changes the 85/15 pool choice.
The pool spec pins the allocation traversal and keys. Earlier allocations can
affect later roster weights, so individual venue rosters are intentionally coupled.

For example, Blue may headline Indigo in edition 1, Lance in edition 2, and
Blue again in edition 3. Neither permanent exclusion nor changed TR is required.
An identical consecutive field is still legal: never redraw or consume a new
edition ID to force the turnover target. These weights are proposed tuning
values, not evidence that the target has been achieved.

Review regional composition and turnover together. Strong home preference can
make a small local pool repetitive even with rotation weights; catalog depth
must support both goals. Do not invent home memberships to meet a turnover target.

### Qualification and progression

A common championship standard replaces the old 8/16/24 admission ladder.
Propose requiring all 24 global badges before entering the first championship
(D4). Subsequent stops require the preceding current-edition result, with no
additional badge or TR threshold. Earning badges and exploring remain independent
of the published seeded itinerary.

Retain the proposed once-per-venue progression reward: each first-ever venue
clear contributes +8 player TR, at most three times for the playthrough. Under
the existing badge curve, the shared 24-badge gate means player TR progresses
56 → 64 → 72 → 80 across those clears. The corresponding soft caps are
62 → 78 → 89 → 100. There is no separate 48/64/80 earliest-clear route in this
proposal. Later editions provide results and ordinary battle rewards, not more TR.

This leaves a player progression advantage: any venue can be the first stop at
cap 62, and later editions face a fully progressed player. D3/D4 must validate
both situations together. The prior illustrative NPC baselines are not adopted,
and qualification does not prove that any particular lineup is balanced. Keep
this as an explicit pre-implementation balance gate; do not hide the difference
through player-adaptive enemies or edition inflation.

### Attempts, replays, and subsequent editions

Losing or leaving resets that competition's room progress. Returning faces the
same people, order, profiles, and assigned TR. Ordinary battle RNG still operates.
Cleared venues remain replayable until registration replaces the edition.
Replays grant ordinary battle rewards but no extra edition result, progression
TR, Hall of Fame entry, Champion Ribbon, or completion credits.

Propose registration at any venue reception after all three results commit and
no run or ceremony is pending. Derive the next schedule using the outgoing
completed edition's lineups as prior-venue history. Atomically commit the new
edition ID, schedule, that history, and fresh current-clear state. Failure keeps
both the old schedule and its old history, and consumes no edition ID. Counter
overflow refuses registration without changing the save.

Persist the current schedule, immediately previous completed edition's canonical
roster IDs by venue, and a bounded completed-edition count. Do not store unlimited
old teams or a lifetime participation blacklist. The history preserves the
selection inputs for reproduction; new-game edition 1 uses empty history.
Badges, lifetime clears, TR high-water state, recognition, regional cleanup,
and Blue/Red access survive rollover.

Propose one new winning-team Hall of Fame and Champion Ribbon flow for Indigo
and Hoenn per edition result. Masters uses its Gallery without regional
recognition, Hall of Fame, or a Ribbon. Apply regional cleanup only on a venue's
first-ever clear. Full story credits follow edition 1's final result; later
editions use a brief completion presentation. Prize currencies/items and more
prestige systems require separate content and balance work.

## Boundaries

This replaces fixed lineups, position-based qualification and difficulty, and
player-entry-TR opponent scaling for Wayfarer's circuit. It preserves the three
venues, independent exploration, global badge accounting, and the player's
badge-and-lifetime-clear TR formula. Future NPC rating updates do not replace
that player progression system.

Ordinary trainers, initial Gyms, wild encounters, player caps, obedience, and
local adventures retain their own policies. Pool membership does not enroll a
character's other encounters in circuit scaling. Standalone Emerald, FRLG, and
HNS remain unchanged.

The [shared seed framework](../specs/playthrough-seed-framework.md) owns the root,
derivation, and unbiased draws. The circuit owns its edition, current schedule,
bounded history, and decision rules. A future dynamic-TR design must define
when ratings change and how they are snapshotted for selection and combat;
it cannot retroactively change a published schedule. Initial baseline ratings
stay fixed until that separate design is adopted.

## Content and balance

Review canonical identities/aliases, home leagues and lore rationale, ratings,
role bands, exact six-member profiles, presentation, and source provenance together.
At equal preparation a higher-rated trainer should usually be harder, while
matchups can still produce upsets. Titles alone do not establish balanced ratings.

Prove every venue can fill its two contender, two elite, and one headliner slots
with distinct TR-qualified home characters even if no visitors are selected.
With the proposed disjoint bands this requires at least 2/2/1 home trainers
in the respective roles per venue. Minimum feasibility is not enough for variety:
a lone eligible home headliner will recur whenever the home pool is chosen for
that role. D3 must author enough credible alternatives and measure the result.
Never repair a shortfall by weakening TR limits, inventing home memberships,
forcing visitors, or substituting a fixed lineup.

Evaluate at least 10,000 documented roots over ten consecutive editions, including:

- adjacent-edition roster overlap by venue, targeting roughly two returning
  opponents where content supports it;
- same-edition cross-venue repeats and role-level variety;
- per-character selection and eligible opportunities, plus absence runs across
  multiple editions so repeatedly overlooked characters are visible;
- regional composition and strength distribution by venue and role, separating
  the 85/15 draws where both pools are available from home-only fallbacks;
- each venue as the first championship at the proposed qualification point,
  and later editions after full player progression.

There is no requirement that every current Elite Four/Champion appears within
one circuit, no 80% individual-per-circuit target, and no complete-attendance
quota. Those earlier experimental targets are superseded. Define quantitative
rotation acceptance under D5 after catalog review; seeded variety is compatible
with occasional repeats and familiar returning opponents.

## Presentation and regional integration

Show the current edition and full itinerary on the Trainer Card from new game,
with qualification and Locked, Available, or Cleared state. A card/reception
roster view shows all five names, assigned TR, specialties, and battle order
before entry, including future stops. Movesets and held items are not revealed
by default. Preview is a read-only view of the saved schedule.

Distinguish player progression TR from opponent strength in context. Actors,
portraits, names, dialogue, battles, and ceremonies follow selected characters.
Rooms must not claim a displaced fixed opponent occupies them. A Gym Leader
headliner is presented as the final opponent without inventing Champion history.

Indigo projects shared Kanto/Johto Champion recognition on its first-ever clear;
Hoenn owns its regional recognition and cleanup; Masters owns its result without
regional Champion status. Edition 1 completes after the third result regardless
of venue. Red unlocks after all three lifetime venue clears and stays available.

Retain the proposed Blue Saffron Dojo unlock after the first committed lifetime
circuit clear, regardless of venue or Blue's participation. Audit dialogue that
assumes he must have been Indigo's final opponent. His other authored encounters
retain their own roles and reward rules.

## Persistence and deterministic decisions

Persist a valid shared root and complete schedule before gameplay can consume
them. The foundation proposes a 64-bit root in two u32 words plus format and
derivation metadata. Do not copy it into circuit state or store a random cursor.

Edition IDs are u32 starting at 1. ORDER, POOL_KIND, and ROSTER use that edition
as occurrence identity, with distinct keys and draft rules versions 1. The
runtime owns schedule schema, versioned profile/TR references, prior-edition
history, and catalog metadata. The pool spec owns stable venue/slot roster keys
and canonical joint allocation. Same root, edition, decision versions, and
content/history inputs reproduce the same unresolved schedule.

Roster changes can propagate through the joint allocation, but cannot change
ORDER or an existing venue/slot's raw POOL_KIND result. Candidate availability
still determines whether a slot needs that draw or falls back to home. Unrelated
seeded features, source enumeration order, queries, elapsed time, gameplay RNG, and
save/load cannot perturb the outcome. Saved schedules remain authoritative.
Missing, corrupt, or incompatible roots, schedules, and required history use
invalid-save handling; loading must not erase history or generate replacements.

Pin the foundation's portable primitive and golden vectors before enabling the
feature. Follow ROM/RAM reserve policies and prerelease save-version policy;
no old prerelease-save migration or direct map.bin editing is required.

### Adoption and related contracts

The existing fixed circuit is implemented; this successor remains unimplemented.
Its approved design direction does not silently rewrite current runtime behavior.
Adoption gives the new specifications precedence for the changes below after
remaining content, qualification, and balance decisions are settled.

| Existing contract | Proposed replacement | Behavior retained |
| --- | --- | --- |
| Interregional circuit [PRD](wayfarer-interregional-league-circuit.md) and [spec](../specs/wayfarer-interregional-league-circuit.md) | Seeded regional championships replace fixed order/lineups and the 8/16/24 position ladder; common role bands and rotating fields replace position difficulty. | Three venues, shared Indigo recognition, regional cleanup ownership, independent travel. |
| League scaling [PRD](league-scaling.md) and [spec](../specs/league-scaling.md) | Snapshot NPC TR and authored role-qualified profiles replace player-entry TR, room offsets, and fixed encounter membership. | Ordinary trainer/Gym policies and explicit randomizer overrides. |
| Player TR [PRD](trainer-rating-wild-encounter-scaling.md) and [spec](../specs/trainer-rating-party-progression.md) | Proposed common 24-badge entry gives TR 56/64/72/80; later editions add no progression TR. | Badge curve, one +8 per lifetime venue clear, high-water rule, ceiling 80, cap/obedience formulas. |
| [Runtime foundation](../specs/wayfarer-runtime-foundation.md) | Add shared root, current edition schedule/results, and bounded prior-venue history. | Lifetime facts, shared save ownership, regional projections, prerelease save policy. |
| [Viridian finale and Blue Dojo](../specs/frlg-kanto-viridian-finale.md) | Dojo access follows the first committed lifetime circuit clear regardless of venue/Blue participation. | Giovanni finale, Blue's authored Dojo battle and rewards. |
| [Kanto story](frlg-kanto-story-on-hns-maps.md) and [Cinnabar/Seafoam](../specs/frlg-cinnabar-seafoam-integration.md) | Fixed Indigo opponent and Indigo-only Dojo assumptions defer to selected profiles and the proposed unlock. | Local quests, independent rival encounters, Gym roles, coast content. |

Historical reports remain evidence for the builds tested, not evidence that
this proposed successor is implemented.

## Decisions before implementation

D1 is reopened: trainer-owned baseline and world progression replace the earlier
static-strength direction. The balance explorer supplies experiments, not final
growth, party-stage, or edition-snapshot contracts.
D2 is resolved: role-qualified headliners do not require a Champion title.
The recurring regional-championship structure, soft rotation, `homeLeagues`,
and initial 85% home / 15% visitor selection are approved. The former 50% outsider
drop gate is removed. Regional frequency and rotation still need balance validation.

| ID | Remaining decision | Proposed default |
| --- | --- | --- |
| D1 | Personal growth, Gym adoption, and strength snapshots during world progression? | Exercise badge checkpoints and first-clear growth in the explorer; reconcile circuit registration, retries, replays, and party stages before implementation. |
| D3 | Concrete catalog, home leagues, role ranges, profiles, strength balance, and sufficient alternatives? | Supported Kanto/Johto/Hoenn singles characters, excluding Tate and Liza; three ordered disjoint TR bands shared by all venues; prove home-only 2/2/1 feasibility and separately validate variety. |
| D4 | Common qualification and first-entry/later-edition balance? | All 24 badges before the first championship; seeded predecessor clears thereafter; retain lifetime +8 per venue and existing player cap formula. |
| D5 | Rotation factors and quantitative acceptance across editions? | Flat in-role weights; factors 16/4/1 for current-edition appearances and 1/2 for previous-venue participation; target roughly 2 returning/3 changed without quotas or rerolls. |

Single battles are confirmed. Supporting defaults remain reviewable: six-member
teams, unchanged-strength replays, reception registration, bounded history and aggregate records, per-edition
Indigo/Hoenn winning-team records and Ribbons, brief later completion presentation,
first-clear Blue Dojo access, separate Red, and advance roster disclosure.
D1 and D3–D5, including the superseded strength clauses identified above, must
be reconciled before implementation.

## References

- [Playthrough-seeded variation](playthrough-seeded-variation.md)
- [Shared playthrough seed framework](../specs/playthrough-seed-framework.md)
- [Trainer pool, generation, and strength](../specs/circuit-trainer-pool.md)
- [Seeded circuit runtime and progression](../specs/seeded-league-circuit.md)
- [Player TR progression](../specs/trainer-rating-party-progression.md)
- [Viridian finale and Blue Dojo](../specs/frlg-kanto-viridian-finale.md)
