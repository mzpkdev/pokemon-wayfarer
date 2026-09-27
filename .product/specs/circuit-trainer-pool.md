# Circuit trainer pool and selection

PRD: [Trainer Circuit](../prds/seeded-trainer-circuit.md)
Implemented: No
Design status: v0 approved: one global pool, top five by TR, ascending battle
order, and a field captured at entry and locked until the venue is won.

## Scope

Own the circuit registry, eligibility, field selection, battle ordering, the
frozen field's contents, and battle construction for each `IS_WAYFARER`
league venue.

- [Well-known trainer rating](trainer-world-progression.md) owns well-known
  trainers, their TR, the team-level and team-size scalers, rosters, and team
  composition. This spec reads a trainer's TR and composed team; it never
  restates how they are computed.
- The [circuit runtime](seeded-league-circuit.md) owns entry, the field lock,
  the win commit, persistence, and presentation.

Upon adoption this replaces the fixed lineups and player-entry-TR level policy
in [League scaling](league-scaling.md).

## Registry and eligibility

Author a versioned, machine-readable registry:

| Field | Contract |
| --- | --- |
| `characterId` | Stable `u32` identity for one person, shared with the progression registry and independent of Trainer IDs, title, party, or region. |
| `displayName` | Existing localized name or an authored circuit name. |
| `presentationId` | Audited graphics, introduction, defeat, and after-battle text. |
| `enabled` | Build-time inclusion, with an authored reason when disabled. |

Maintain an alias inventory linking existing Trainer IDs and source roster
owners to canonical characters. Gym, rival, Champion, rematch, and regional
encounters of one person share one `characterId` and one roster, so aliases
cannot enter a field twice. People with similar names remain distinct.

A trainer is **eligible** when they are a well-known trainer with an authored
TR and a valid roster, fight in singles, are enabled, and have validated
presentation. Red and Tate & Liza are not well-known in v0, so they are
league-ineligible and keep their current policies: Red his separate mastery
encounter, Tate & Liza their double battle. Region, title, and story
availability neither add nor remove a trainer. Circuit eligibility does not
change story battles, which follow the
[every-battle rule](trainer-world-progression.md#trainer-rating).

## Selection

One global pool of eligible trainers serves every venue. At entry:

1. Sort the pool by TR, highest first.
2. Take the first five. Equal TRs keep whatever order the registry iteration
   and sort produce; there is no tie-break rule.
3. Order the five by ascending TR for battle, so the highest TR fights last.
   Equal TRs again take whatever order the sort produces.

Venue, region, home membership, title, player TR, party, and history play no
part. Every venue may therefore field the same five, and v0 accepts that. The
procedure consumes no randomness and reads no seed.

Because ties have no rule, a changed registry order or sort can reorder tied
trainers in a field not yet entered. The frozen field below keeps any entered
field stable.

## Frozen field

Entry captures, for each of the five in battle order: `characterId`, their TR,
and their composed team, plus the registry and roster content versions. Per
member, the team holds the roster entry index and every resolved battle field
the plan uses: species/form, level, moves, item, ability, nature, IVs/EVs, and
battle order. The runtime saves this atomically before reveal. The field stays
locked until the venue is won; a loss or leaving keeps it. Every retry at that
venue, including after reload, reconstructs battles from the saved field and
never reselects or recomposes. With fixed TRs this equals locking after a loss.

If the content version changes while a field is locked, the lock is dropped
and the next entry captures a new field (prerelease policy; the save stays
valid).

## Battle construction

Each slot fights with the trainer's own team at their own TR, as
[well-known trainer rating](trainer-world-progression.md) composes it for any
battle: there is no league-specific level offset, role adjustment, or
six-member competitive profile. The old `[-4,-3,-2,-1,+1]` room offsets are
removed.

Construct from the saved field. Resolve the selected trainer and roster owner
before applying circuit policy; never identify enrollment from class, map, or
a shared party pointer. Record runtime Trainer ID, source roster owner, and
content version, and preserve member identity through ordering and gimmick
remapping. Content is immutable at runtime; do not edit shared Gym or story
parties. Debug and ordinary battles cannot create circuit records.

Challenge settings keep their overrides. The trainer species randomizer may
bypass authored parties as it does today but still uses the saved people and
order. XP uses actual species and levels. Prize money uses the trainer's
inventoried source reward basis and class, not the old room occupant, and team
size must not shift it.

## Validation

Check in an inventory: aliases, roster references, presentation coverage,
exclusion reasons, provenance, and content versions.

- Reject duplicate characters or aliases, unresolved source IDs, missing
  assets, double-battle flags, and trainers without an authored TR or valid
  roster. The build must hold at least five eligible trainers.
- **Selection.** Fixtures for the top five over distinct TRs, ties at the
  fifth-place boundary, and ties inside the field; the battle order is
  non-decreasing in TR with the highest last; excluded and disabled trainers
  never appear; aliases never appear twice.
- **Frozen field.** Enter, lose, leave, retry, and reload: the saved field and
  teams are reused unchanged. Change the content version while a field is
  locked: the lock is dropped and the next entry captures a new field.
- **Construction.** Build every eligible trainer's team, preserve member
  identity and metadata, reconstruct identically from the saved field, and
  verify money, XP, AI, graphics, and dialogue follow the selected trainer.

Run the trainer/scaling mechanics and circuit E2E suites, build Wayfarer and
affected standalone configurations, and measure ROM/RAM against the reserve
policy. Report balance playtesting separately from structural checks.

## Later

- Seeded keys and draws for selection.
- Role windows and standing-based slots.
- Rotation weights between editions.
- Home leagues and the 85/15 home/visitor draw.
- Nearest-standing fallback for empty role windows.
- Seeded venue order.

## References

- [Trainer Circuit PRD](../prds/seeded-trainer-circuit.md)
- [Well-known trainer rating](trainer-world-progression.md)
- [Circuit runtime](seeded-league-circuit.md)
- [Existing League scaling contract](league-scaling.md)
- [Party construction](../../game/src/battle_main.c)
- [Scaling policy and roster validation](../../game/src/trainer_party_scaling.c)
- [Current fixed League metadata](../../game/src/data/trainer_scaling/league.h)
