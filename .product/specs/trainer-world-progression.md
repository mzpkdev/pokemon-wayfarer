# Trainer world progression

PRD: [Trainer world progression](../prds/trainer-world-progression.md)
Implemented: No; the browser explorer implements an experimental model only.
Design status: Target contract. Numeric catalog values and production balance
require review. A stable seeded growth modifier per canonical trainer is approved,
initially 90–110%; individual competition snapshots replace whole-edition locks.

## Ownership and scope

Own the personal NPC rating evaluator, world milestones, authored team stages,
and the NPC TR-to-level resolver for enrolled Wayfarer encounters. The
[Gym spec](gym-leader-scaling.md) owns initial-badge coverage and battle
construction. The [pool spec](circuit-trainer-pool.md) owns circuit eligibility,
regional weighting, rotation, and competitive profiles. The
[circuit runtime](seeded-league-circuit.md) owns registration, saved schedules,
transactions, admission, event termination, and waiting.

This is the authority for proposed NPC growth. The implemented player rating
and soft cap remain owned by [player progression](trainer-rating-party-progression.md).
NPC growth never calls `GetTrainerRating()` to obtain an opponent rating, never
changes its high-water mark, and does not replace wild, shop, ordinary-trainer,
or Gym-member policies. Standalone builds keep their existing contracts.

## World point

Use `B`, the count of distinct global badges, in 0–24, and a lifetime-clear mask
`L` keyed by Indigo, Sevii Masters, and Hoenn. Let `C = popcount(L)`, in 0–3.
The shared runtime's canonical facts own these values. Do not infer them from
local game-clear flags, Champion titles, room wins, current-edition results,
encounter counts, or the player's saved TR.

Commit badges and first clears through their existing reward transactions.
The award affects subsequent world encounters, never the fight that earned it.
Later edition clears, losses, replays, reloads, and duplicate award requests do
not add milestone growth. Clearing a venue already in `L` leaves `C` unchanged.

## Seeded personal growth rate

Each canonical trainer has one immutable `growthPercent` per save. The initial
range is the 21 integers 90–110, uniformly sampled; 100 means the authored growth
rate. Use the [shared seed framework](playthrough-seed-framework.md), with
`TRAINER_GROWTH` domain 2, `GROWTH_RATE` decision 1, rules version 1,
`entityLo = characterId`, `entityHi = 0`, `occurrenceId = 0`, and `drawId = 0`:

```text
growthPercent = 90 + Uniform(21, trainerGrowthKey)
```

The shared root and saved growth-policy version must exist from new game, before
any enrolled Gym encounter. Resolve a trainer's rate on demand from that root,
stable canonical ID and pinned policy version. This is one logical assignment,
not a fresh roll on lookup. No mutable random cursor or eager full-catalog save
is required. All encounter aliases, Gym profiles and league appearances of one
character use the same rate. Separate trainers draw independently; different
saves may coincidentally assign the same rate.

Never key this modifier by badge count, venue, circuit/event identity, appearance
count, source Trainer ID, catalog ordering, party, or current TR. New trainers
must not perturb existing rates. Loading, losing, changing region, earning a
badge, or entering another competition cannot change the assigned rate. Use no
ordinary Pokémon RNG. Pin the policy version for the save; changing the range or
distribution requires a new version and cannot silently reinterpret existing
snapshots. Follow prerelease save rejection policy rather than inventing migration.

Keep `baselineTR` unchanged. Apply the percentage only to authored badge and
first-clear growth, with the integer order below. Future story/training influences
must declare their inputs and how they interact with this modifier before being
added; this version introduces no such events or arbitrary TR drift.

## Trainer records and effective TR

Each canonical trainer record supplies versioned identity, `baselineTR`, a
personal `badgeTRCheckpoints` tuple at badges 0/8/16/24, `leagueGrowth`, and
authored stage/profile references. The first checkpoint equals `baselineTR`.
Checkpoints are integers in 0–80, nondecreasing; the initial authoring range for
integer `leagueGrowth` is 0–20, matching the explorer. Reject invalid content at
build time. The baseline is the starting
rating, not the rating permanently used for eligibility or combat.

For neighboring checkpoints `(b0,t0)` and `(b1,t1)` surrounding `B`:

```text
d = b1 - b0
badgeTR = t0 + floor((2 * (B - b0) * (t1 - t0) + d) / (2 * d))
authoredGrowth = (badgeTR - baselineTR) + leagueGrowth * C
scaledGrowth = floor((authoredGrowth * growthPercent + 50) / 100)
effectiveTR = clamp(baselineTR + scaledGrowth, 0, 80)
```

At 24 badges use the final authored checkpoint as `badgeTR`, then still apply
the growth modifier. First round badge interpolation as above; then scale the
combined nonnegative badge/first-clear growth and round once with halves upward.
Do not round each badge reward or clear bonus separately and accumulate it.
Use sufficiently wide integer intermediates and validate all inputs. At 100%,
the result exactly matches the prior unmodified formula. At B=0 and C=0, every
seed returns the authored baseline. TR never exceeds 80; not every trainer must
reach 80, and some faster-growing trainers may saturate earlier.

The rating evaluator is pure given world point, authored curve and resolved
`growthPercent`. Only immutable rate assignment uses a keyed seed decision; no
milestone award consumes a new random draw. Runtime corruption is an error,
not a reason to substitute player TR, a neutral modifier, or another curve.
There is no player-party input or persistent personal random cursor.

For a baseline of 10 and authored growth of 30, rates 90/100/110 produce TR
37/40/43. Experimental Blue at 24 badges and three first clears has baseline 6
and authored growth `(59 - 6) + 6 * 3 = 71`: the same rates produce TR 70/77/80
(the last clamps from 84). These are examples, not accepted combat balance.

The badge curve is authored per trainer. Gym eligibility, title, source Trainer
ID, home league, current location, and original Gym order do not choose a curve
implicitly. Aliases of one character share the same personal rating; separately
enrolled encounter profiles can still differ in authored content.

The explorer currently uses six TR per first clear by default and Blue's
6/54/57/59 checkpoints. Other values are catalog data. None of those numbers is
a production acceptance claim. The current explorer applies a neutral 100%
rate and does not yet derive or expose seeded modifiers. Saturation at 80 must
be visible in balance reports; registering another edition must not reset or extend the growth budget.

## Stages and levels

Select the stage with the greatest authored `minTR <= effectiveTR`. Stage
thresholds are unique and increasing, cover TR 0, and stay within 0–80. Each
stage references a complete authored profile with 2–6 members, explicit
species/forms, ace designation, battle order, signed level offsets, moves
policy, held items, abilities, stats, and trainer AI/inventory. Each consumer
adds its own eligibility requirements; the circuit currently proposes reviewed
six-member competitive profiles. A valid opening Gym stage is not automatically
a valid championship entry.

Preserve stable source-member identity through ordering and profile selection.
Author changes between stages explicitly. Do not infer evolution from level,
reverse species, pad a team, sample new members, or copy incomplete source
parties as a production fallback. Each stage and encounter variant must be
complete independently. Stage changes can alter species and count; one immutable
six-member roster filtered by player TR is not the target selection rule.

The NPC level curve is separate from the player cap and ordinary trainer curve.
The current experimental anchors are:

```text
(0,12), (4,16), (8,18), (16,23), (30,30),
(40,42), (55,60), (65,80), (80,100)
```

For adjacent level anchors `(r0,l0)` and `(r1,l1)`, resolve:

```text
d = r1 - r0
aceLevel = l0 + floor((2 * (effectiveTR - r0) * (l1 - l0) + d) / (2 * d))
memberLevel = clamp(aceLevel + member.levelOffset, 1, 100)
```

Use exact endpoints and wide signed intermediates. Curves cover 0–80 with
strictly increasing rating anchors and nondecreasing levels in 1–100. Require
at least one ace with offset 0 and support offsets in the initial range -30–0.
Review source
level gaps rather than imposing the old universal minus-one/minus-two rule.
Within stages, levels cannot decrease as TR rises. Across default stage changes,
party size and the minimum member level must not decrease. Validate authored
offsets against those rules; do not silently repair production content.

The lower NPC starting anchor allows the experimental Brock TR 2 profile to
produce Geodude 12 / Onix 14. It does not lower the player's TR-0 cap of 15.
Source parties are provenance and balance references; their absolute levels
do not override the selected curve at runtime.

## Snapshot boundaries

### World Gym encounters

After resolving canonical encounter identity and variant, derive its saved-policy
growth rate. Capture `(B,L)`, `growthPercent`, growth-policy version, personal TR,
stage/profile IDs, and content versions before constructing the opponent. Keep
the complete plan for battle reconstruction. Clear the transient plan at teardown.
At unchanged milestones, subsequent attempts resolve identical authored
membership and levels; existing battle RNG may still differ. An intervening
badge or first clear legitimately changes the next encounter's plan.

### Individual league competitions

New game creates the root and first circuit travel order, with no registered
trainer lineup. A circuit edition describes the three-venue traversal; a
competition is one event at one venue. Only entry into an available competition
captures the actual world point:

```text
B_event = current global badge count
L_event = current lifetime venue-clear mask
C_event = popcount(L_event)
```

Capture catalog/growth/band/resolver versions, the saved growth-policy version,
and required participation history. Derive each candidate’s immutable modifier
from its canonical ID. Resolve candidates at `(B_event,C_event)` before role eligibility and home/visitor
selection. Generate only this competition's five slots. Do not select future
venues, assume future clears, or hold the badge count fixed across the circuit.
Home status never changes effective TR. The same character may appear in later
competitions with a higher rating after actual world progression.

Persist this event's entry inputs, selected effective TR, growth percentages,
stage/profile IDs, and versions, including growth-policy version. Verification
checks each rate against the saved root and policy before checking TR/eligibility. While the event is active, battle reconstruction, saves, reloads,
and departures retain that plan. A changed live world point never mutates an
active event. World Gym opponents separately use live milestones at battle setup.

A loss terminates the competition and releases its active participant/strength
lock. It grants no venue clear or milestone growth. Preserve the event identity,
result, and bounded participation history so reopening registration cannot revive
or reroll it. The player must wait for a later available competition, whose
entry captures then-current milestones. A win commits the venue result and any
first-lifetime-clear growth before a later competition can capture its inputs.
The five-person lineup is fixed within an event, not for all three venues or all
attempts at one venue.

The circuit runtime owns stable competition ordinals, availability, atomic entry
and result commits. Repeated entry requests for the same event do not consume a
new identity. A new competition is permitted only by the adopted availability
rule, not by loading, previewing, or cancelling registration. The waiting rule
and exact signup thresholds remain unresolved. Around 8/16/24 badges for circuit
positions is tentative guidance; the former all-24-badges prerequisite is removed.
Neither a failed entry transaction nor a corrupt snapshot authorizes regeneration
from live progress. Exhibition replays are outside this revised lifecycle.

Role bands are authored for world point `(B,C)` and shared by all venues at the
same point. Use actual effective TR for admission to those bands, not baseline
TR, historical title, or a player's cap. Bounds must remain ordered and disjoint
through TR saturation. The catalog must prove home-only 2/2/1 feasibility at all
supported entry world points and allowed per-trainer modifier combinations before
shipping; empty roles block content enablement rather than widening bands,
forcing visitors, or rerolling growth rates. Checking all trainers at 90% and
then all at 110% is insufficient: different trainers have different rates, and
interior rates can cross role or stage boundaries. Establish a conservative
coverage proof (for example sufficient home candidates whose role/profile
eligibility survives every allowed rate), or an exhaustive equivalent. Sampled
seeds establish variety, not universal feasibility.

## Validation and adoption

Validate all approved production records × 25 badge counts × four first-clear
counts × 21 growth percentages. Extend the explorer before using it as evidence
for seeded variation; its current 37 prototypes cover only the neutral rate. Check
rounding, clamps, monotonic growth, stage boundaries, profile completeness,
source provenance, baseline invariance, 100% equivalence, and explicit encounter
enrollment. Check immutable rate derivation across reloads/aliases/events,
independence from lookup order/catalog additions/other domains, and distinct
seed examples without requiring every seed to give different rates. Add fixtures for Blue's
distinct curve, unchanged player caps, and Tate/Liza's unenrolled double battle.

Test world-Gym retries at unchanged milestones, active-event snapshot reuse,
all six circuit orders, entry rollback, loss termination and waiting, duplicate
result recovery, and genuine milestone progression before the next competition.
Check that future stops remain unlocked, first-clear rewards affect only later
events, and event counts do not inflate TR. Test seed isolation separately from
ordinary battle RNG.

The browser tool cannot establish production moves/items/AI balance or ROM
integration. Finalize personal curves, stage content, NPC anchors, circuit bands,
qualification, and playtest evidence before enabling the proposed policies.
The existing Gym and League implementations remain active until that adoption;
their current policy descriptions are not alternative target requirements.
