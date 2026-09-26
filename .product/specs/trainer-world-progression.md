# Trainer world progression

PRD: [Trainer world progression](../prds/trainer-world-progression.md)
Implemented: No; the browser explorer implements an experimental model only.
Design status: Target contract. Numeric catalog values and production balance
require review; projected whole-edition snapshots are confirmed.

## Ownership and scope

Own the personal NPC rating evaluator, world milestones, authored team stages,
and the NPC TR-to-level resolver for enrolled Wayfarer encounters. The
[Gym spec](gym-leader-scaling.md) owns initial-badge coverage and battle
construction. The [pool spec](circuit-trainer-pool.md) owns circuit eligibility,
regional weighting, rotation, and competitive profiles. The
[circuit runtime](seeded-league-circuit.md) owns registration, saved schedules,
transactions, admission, and replay.

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
effectiveTR = clamp(badgeTR + leagueGrowth * C, 0, 80)
```

At 24 badges use the final checkpoint directly. Use sufficiently wide integer
intermediates and nearest-integer rounding with halves upward. Validate inputs
before evaluation. Runtime corruption is an error, not a reason to substitute
player TR or another trainer's curve. The evaluator is pure: it consumes no seed
or Pokémon RNG, saves no personal random cursor, and has no player-party input.

The badge curve is authored per trainer. Gym eligibility, title, source Trainer
ID, home league, current location, and original Gym order do not choose a curve
implicitly. Aliases of one character share the same personal rating; separately
enrolled encounter profiles can still differ in authored content.

The explorer currently uses six TR per first clear by default and Blue's
6/54/57/59 checkpoints. Other values are catalog data. None of those numbers is
a production acceptance claim. Saturation at 80 must be visible in balance
reports; registering another edition must not reset or extend the growth budget.

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

After resolving encounter identity and variant, capture `(B,L)`, personal TR,
stage/profile IDs, and content versions before constructing the opponent. Keep
the complete plan for battle reconstruction. Clear the transient plan at teardown.
At unchanged milestones, subsequent attempts resolve identical authored
membership and levels; existing battle RNG may still differ. An intervening
badge or first clear legitimately changes the next encounter's plan.

### Circuit editions

Fix the complete edition at eligible registration. New game
creates the root and first travel order, but has no registered trainer lineup.
Capture `B_reg`, `L_reg`, catalog/growth/band versions, and required history.
For the venue at zero-based position `i`, let `P_i` contain earlier venues in
that order:

```text
B_i = B_reg
L_i = L_reg union P_i
C_i = popcount(L_i)
```

Resolve every candidate at `(B_i,C_i)` before role eligibility and home/visitor
selection. The first fresh edition therefore uses C=0/1/2 at its three stops;
later editions use C=3 at every stop. The union avoids counting a lifetime clear
twice. Home status never changes effective TR. A returning character can have
different TR at different first-edition stops because their world point differs.

Persist registration inputs, projected world points, and selected effective TR,
stage/profile IDs, and versions with the entire schedule. Retries, reconstructions,
and exhibition replays use those saved values, never live milestones. The same
effective TR and same authored profile produce the same ordinary strength.
Current world Gym opponents continue to use live milestones separately.

This projection assumes the proposed common 24-badge first-registration gate,
so badges cannot change during an edition. A different admission design needs
an explicit badge-projection rule and corresponding validation; it cannot be
enabled by changing a threshold alone. Registration failure changes neither
edition identity nor saved history. Corrupt snapshots are not regenerated from
the current world state.

Role bands are authored for world point `(B,C)` and shared by all venues at the
same point. Use actual effective TR for admission to those bands, not baseline
TR, historical title, or a player's cap. Bounds must remain ordered and disjoint
through TR saturation. The catalog must prove home-only 2/2/1 feasibility at all
supported registration projections before shipping; empty roles block content
enablement rather than widening bands or forcing visitors.

## Validation and adoption

Exhaust 37 prototype records × 25 badge counts × four first-clear counts in the
explorer, and all approved production records in the content validator. Check
rounding, clamps, monotonic growth, stage boundaries, profile completeness,
source provenance, and explicit encounter enrollment. Add fixtures for Blue's
distinct curve, unchanged player caps, and Tate/Liza's unenrolled double battle.

Test snapshot reuse, unchanged-milestone retries, genuine milestone progression,
all six circuit orders, registration rollback, union-based projected clears,
same-character cross-venue strength, capped later editions, and replay after
world progression. Test seed isolation separately from ordinary battle RNG.

The browser tool cannot establish production moves/items/AI balance or ROM
integration. Finalize personal curves, stage content, NPC anchors, circuit bands,
qualification, and playtest evidence before enabling the proposed policies.
The existing Gym and League implementations remain active until that adoption;
their current policy descriptions are not alternative target requirements.
