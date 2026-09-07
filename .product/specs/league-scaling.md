# League scaling

PRD: [League scaling](../prds/league-scaling.md)
Implemented: Partial

The code contract is implemented. See [implementation evidence](../research/league-scaling-implementation.md)
for automated results and pending campaign balance acceptance. This status does
not implement the separate TR progression revisions.

## Scope and current code

Implement TR-based levels for the fifteen fixed circuit opponents in
`IS_WAYFARER`. Preserve all existing non-level team content and admission rules.
This specification supersedes the static League level policy only.

Consume `GetTrainerRating()` at admission. Trainer Rating production, badge and
League contributions, high-water storage, and player progression remain owned
by the existing [circuit specification](wayfarer-interregional-league-circuit.md)
and [party progression specification](trainer-rating-party-progression.md).
Do not change their formulas or reward amounts as part of implementing scaling.
Their pending revisions can be implemented independently; scaling works with
whatever valid TR the current producer returns.

Current integration points:

- `game/src/league_circuit.c`: admission, regional clear recording, and the
  transient recorded-clear handoff. Its TR formula is outside this feature.
- `game/src/league_circuit_scripts.c` and
  `game/data/scripts/league_circuit.inc`: script-facing admission and completion.
- `game/data/maps/PokemonLeague_*_hns/scripts.inc`: shared Indigo rooms and
  Hall of Fame, currently resolving the required region repeatedly.
- `game/data/maps/EverGrandeCity_*/scripts.inc`: Hoenn entry and room flow.
- `game/src/trainer_party_scaling.c` and the Trainer party constructor: policy
  dispatch, effective levels, and battle reconstruction.
- `game/src/data/trainers_hns.party` and `game/src/data/trainers.party`:
  authoritative existing parties. `game/test/league_tiers.c` currently asserts
  their static levels and non-level identity across difficulty settings.

The transient recorded-clear region is not a persisted run snapshot. Keep its
existing Hall of Fame handoff role; do not reuse it as the run state.

## Opponent metadata

Add a distinct League policy and an explicit allowlist keyed by circuit region,
Trainer ID, resolved roster owner, and resolved difficulty. Do not enroll a
trainer from class, map, party pointer, or a rematch-looking name alone. Indigo's
`_2_HNS` IDs are the initial Johto circuit teams and are in scope.

Use the following source-slot metadata, with zero-based indices. Offset vectors
are in existing battle order and are relative to that opponent's ace. All
members stay present and in source order. These are authored constants, not
values recomputed from old levels at runtime.

| Circuit | Trainer ID (prefix `TRAINER_`) | Ace slot / species | Slot offsets |
| --- | --- | --- | --- |
| Kanto | WILL_1_HNS | 4 / Xatu | -2, -1, -2, -1, 0 |
| Kanto | KOGA_1_HNS | 4 / Crobat | -2, -1, -2, -1, 0 |
| Kanto | BRUNO_1_HNS | 4 / Machamp | -1, -2, -1, -1, 0 |
| Kanto | KAREN_1_HNS | 4 / Houndoom | -1, -2, -2, -2, 0 |
| Kanto | LANCE_1_HNS | 5 / Dragonite | -2, -1, -2, -1, -2, 0 |
| Johto | WILL_2_HNS | 5 / Xatu | -2, -1, -1, -1, -2, 0 |
| Johto | KOGA_2_HNS | 5 / Crobat | -1, -1, -1, -1, -1, 0 |
| Johto | BRUNO_2_HNS | 5 / Machamp | -1, -1, -1, -1, -1, 0 |
| Johto | KAREN_2_HNS | 5 / Houndoom | -1, -2, -1, -1, -2, 0 |
| Johto | LANCE_2_HNS | 5 / Altaria | -1, -2, -1, -2, -1, 0 |
| Hoenn | SIDNEY | 4 / Absol | -1, -1, -1, -1, 0 |
| Hoenn | PHOEBE | 4 / Dusclops | -1, -1, -1, -1, 0 |
| Hoenn | GLACIA | 4 / Walrein | -1, -1, -1, -1, 0 |
| Hoenn | DRAKE | 4 / Salamence | -1, -1, -1, -1, 0 |
| Hoenn | WALLACE | 5 / Milotic | -1, -2, -1, -1, -1, 0 |

Preserve the existing difficulty resolver, including its normal-party selection
for the circuit. Audit every selectable difficulty against the resolved source;
metadata mismatch is a validation failure, not permission to choose a different
team. Validate exact count, species/form at each slot, ace slot, moves, and
held items against the sources before enabling production scaling. Lance's
Johto ace remains the existing final Altaria; do not substitute Dragonite.

## Level function

Expose a pure `GetLeagueScalingBaseline(rating)` function with its own anchors:

```
(0,15), (4,16), (8,18), (16,23), (30,30),
(40,42), (55,60), (65,80), (80,100)
```

Clamp input TR to 0 through 80. Between adjacent anchors `(r0,l0)` and
`(r1,l1)`, use integer arithmetic wide enough for the intermediate product:

```
width = r1 - r0
rise = (rating - r0) * (l1 - l0)
baseline = l0 + (2 * rise + width) / (2 * width)
```

Division truncates, giving nearest-integer rounding with halves upward. Do not
call the player's cap resolver or Gym Leader resolver for this baseline.

The encounter offsets in circuit order are `[-4, -3, -2, -1, +1]`.
For each source slot compute, using signed arithmetic:

```
level = clamp(baseline + encounterOffset + slotOffset, 1, 100)
```

Clamp once after adding both offsets. At TR 80 a Champion ace is 100 and its
minus-one support is also 100; this intentional saturation must not become
99 through premature ace clamping. Levels must be monotonic across every TR.

The source roster's absolute levels have no influence on this calculation.
Player party size/levels, region badge counts, historical Gym order, and global
difficulty do not introduce new offsets.

## Persistent run state and lifecycle

Add a save-backed Wayfarer run record containing `active`, `region`, and
`ratingAtEntry`. Use the existing Wayfarer save ownership and initialization
mechanisms; it must not be a region-swapped event variable. Store only these
facts, not derived levels or a copied roster. Existing room progression remains
the authority for which opponents have been defeated.

1. At successful admission into the first room, before locking the entrance,
   validate the existing circuit requirements and capture `GetTrainerRating()`
   plus the admitted circuit region. Failed admission creates no run. Reset
   room defeat state using the existing new-run flow.
2. First-room resume and map load must recognize an active matching run and
   never overwrite its rating or reset defeated rooms. Ordinary room transitions
   use the same record. Resolve Indigo Tier 1/2 from its saved circuit region,
   not the geographic region of the shared venue.
3. Each enrolled battle requires an active matching record, the expected source
   identity, and valid room progression. Use `ratingAtEntry` for construction
   and reconstruction. A per-battle snapshot may copy this value but must never
   replace it with live TR. Battle teardown does not end the League run.
4. A loss/whiteout or departure from the run clears the record and resets the
   existing room progression for the next attempt. Enumerate both venues' exit,
   warp, whiteout, and return paths. Ordinary room and Hall of Fame transitions
   stay within the run until completion handling finishes.
5. Champion victory proceeds through the existing Hall of Fame completion path.
   Validate the captured region and completed room progression, record its
   first clear through the existing producer without changing its TR policy.
   Preserve the recorded-clear handoff used by GameClear. Clear the active run
   and reset venue progression before the completion save/return to overworld.
   Keep the captured region locally for cleanup after setting the clear flag:
   `GetRequiredLeagueRegion()` now names the next League.
6. Save/load within an unfinished run preserves its record and room progression.
   Repeated completion processing must not award another contribution, switch
   the recorded result to the next League, or skip its opponents.

Initialize an inactive record on new game. Validate region, rating range, and
venue/room consistency on load. Invalid or absent run state inside a League
must safely reset the attempt and return to that venue's lobby without a clear
or reward; do not silently capture a new rating halfway through a run. Validate
before dispatching room scripts. Do not add migrations for prerelease saves.

Existing admission restrictions still apply. No League replay access or
post-circuit run mode is introduced. An out-of-context debug Trainer battle
must use the existing authored fallback, never create a real circuit run or
award a clear. Real room scripts deny/recover invalid state before battle.

## Party construction and other modes

Apply only the effective level override at the existing resolved-party
construction boundary. Preserve count, source indices, species/forms, custom
move tuples, held items, abilities, IVs, EVs, natures, genders, balls, gimmick
configuration, trainer healing inventory, and AI. Do not pass League members
through ordinary evolution reversal or custom-move replacement. Keep source
party data immutable. All enrolled source slots currently have authored moves;
if that assumption changes, require explicit metadata review before enabling
the new source rather than quietly replacing its moves.

Keep existing Trainer species-randomizer behavior as a complete party-path
bypass, consistent with the Gym Leader boundary. Other move randomizers and
challenge modes retain existing precedence. Run lifecycle and first-clear
rewards still apply in those modes. Do not claim roster identity preservation
when the player explicitly enables a content randomizer.

Battle XP follows actual species and effective levels. Prize money retains the
existing authored-level and trainer-class basis. No change to rewards, healing,
catch rules, or player stats is implied by scaling opponent levels.

## Validation and release

Update static-level assertions in `game/test/league_tiers.c` while retaining
its authored identity checks. Extend `game/test/league_circuit.c`, script tests,
and the League E2E journey. Required evidence:

- All integer TR values 0 through 80 for all fifteen parties: exact formula,
  anchors, interpolation ties, signed offsets, saturation, monotonicity, size,
  unchanged order, species/forms, moves/items and remaining authored fields.
- Actual party construction and reconstruction for every resolved difficulty,
  including duplicate species, source slot identity, randomizer bypass, and
  unchanged ordinary/Gym/story/rematch behavior.
- Preserve valid admission order and existing completion behavior for first
  clear, failed clear, repeat clear, and duplicate completion handling. Tests
  use the current producer contract; scaling introduces no reward calculation.
- Persistent run admission, denied admission, room transitions, save/load,
  loss/retry, exit/re-entry, invalid record recovery, and changing live TR during
  a run without changing its opponents. No reward before successful completion.
- Earliest-entry and all-badges-first routes, intermediate League timing,
  consecutive Kanto/Johto runs at Indigo, completion saves and reloads, and
  return to regional travel without stale room or clear state.
- Seeded input ratings from the PRD examples and all curve boundaries. Assert
  that scaling does not mutate live TR or change its producer. Campaign tests
  derive expected levels from the actual admission TR, without assuming a
  particular League reward schedule.

Build the production Wayfarer configuration and relevant mechanics/E2E ROM
configuration. If the shared engine is changed, compile affected standalone
configurations and prove their authored League behavior remains unchanged.
Keep level scaling behind an independent compile-time switch that restores
existing authored party construction when disabled. Toggling this switch must
not change TR progression or rewards. Exercise run completion with either setting.

Produce a deterministic roster/level inventory from resolved source data and
record emulator playtesting separately from structural checks. Do not report
playable balance from unit tests alone. Acceptance includes early Kanto
attrition and both routes' between-League training demands. Keep any proposed
roster rebalance outside this implementation and obtain a separate design
revision for it.

## Related documents

- [Circuit specification](wayfarer-interregional-league-circuit.md)
- [Gym Leader specification](gym-leader-scaling.md)
- [Player progression specification](trainer-rating-party-progression.md)
