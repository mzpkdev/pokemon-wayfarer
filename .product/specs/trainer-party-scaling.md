# Ordinary Trainer and Gym-member scaling

PRD: [Ordinary Trainer and Gym-member scaling](../prds/trainer-party-scaling.md)
Implemented: No

## Scope and authority

Implement for `IS_WAYFARER`. Standalone builds retain their existing behavior.
This specification owns automatic party transformation for ordinary opposing
Trainers and Gym members. It does not implement Trainer Rating advancement,
boss scaling, new rematches, or new rosters.

Use `GetTrainerRating()` as the only progression input. Store no per-Trainer
scaled roster, cap, level, or historical Rating in save data. Keep the source
Trainer records immutable.

## Classification contract

Generate a compact policy table indexed by the active Wayfarer Trainer ID.
Use three policies: `ORDINARY`, `GYM_MEMBER`, and `EXCLUDED`. Every populated
Trainer ID must have exactly one policy. Unclassified IDs fail generation;
invalid runtime IDs fail closed to existing unscaled behavior.

A populated ID has a nonempty party in at least one selectable difficulty
variant after roster overrides are resolved. Zero-initialized table holes do
not require manifest records. A script or rematch reference to a hole or an
empty resolved roster is a validation failure, not an implicit exclusion.

Keep the authoring inventory in a versioned machine-readable manifest. Each
record contains the symbolic Trainer ID, policy, and evidence references to
maps, scripts, rematch tables, or an explicit authored role. Excluded entries
also carry a reason. A batch inventory tool should propose the initial records
from existing data, so authors review classifications rather than rewrite
parties. Reject duplicate IDs, unknown IDs, stale references, and missing IDs.

Gym-map references are discovery evidence, not the runtime classifier. Use the
opposing Trainer ID at runtime; do not apply a Gym bonus merely because a
battle takes place on a Gym map. Require explicit review when one ID serves
both Gym-member and ordinary roles. Select one documented policy for that ID,
or split its source ID before enrolling it; never infer a context-dependent
policy silently.

Exclude Gym Leaders, all rival variants, villain bosses and admins, Elite Four,
Champions, other story bosses, and tutorial opponents. Enumerate associated
rematches, starter variants, aliases, and scripted variants. Shared classes
must not cause boss enrollment. Ordinary scripted battles and villain grunts
remain eligible.

Facility, link, recorded, external-party, partner, and player-party construction
paths are excluded by battle context before ID policy is considered. Include
Frontier, Trainer Hill, e-Reader, Secret Base, and rental-party sources in that
context audit. A raw `struct Trainer *` without a validated opposing ID and
eligible context does not authorize scaling.

## Level projection

Author the following data independently from the wild and player-cap curves:

| Rating | Trainer baseline |
| ---: | ---: |
| 0 | 7 |
| 4 | 8 |
| 8 | 10 |
| 16 | 15 |
| 30 | 22 |
| 40 | 34 |
| 55 | 52 |
| 65 | 72 |
| 80 | 92 |

Clamp Rating to 0 through 80. Interpolate adjacent baseline anchors linearly,
rounding exact halves upward. For integer division define `roundSigned(n/d)`
as nearest integer with exact halves away from zero and positive denominator.
For authored level `L` in 1 through 100:

```text
identityAdjustment = clamp(roundSigned((L - 5) / 5), -1, 8)
roleBonus = 2 for GYM_MEMBER, otherwise 0
effectiveLevel = clamp(baseline(Rating) + identityAdjustment + roleBonus, 1, 100)
```

Project each selected slot independently. Party order and size do not change;
the compression and level-100 clamp may make previously distinct levels equal.
No random level jitter, player-party matching, wild profile offsets, species
floors, or enemy clamp to the player soft cap applies. Because the baseline is
monotonic and adjustments are constant, no cumulative high-water projection
loop is necessary.

Read Rating once when constructing an eligible battle's opponents. Both
opponents in a two-Trainer battle share this snapshot but retain independent
policies. A boss paired with an ordinary Trainer remains unscaled while the
ordinary Trainer scales. Reuse the snapshot if setup reconstructs a party;
discard it after the battle. Retry after a loss starts a new snapshot.

## Species, moves, and per-Pokémon fields

Resolve numeric level-evolution predecessors until the effective level supports
the resulting stage. Reuse the validated numeric predecessor authority behind
wild encounters, or extract it into a shared generator/helper. Extend metadata
coverage to every eligible Trainer species and all reachable predecessors;
the existing ordinary-wild species inventory is not sufficient by assumption.
Resolve ambiguous ancestry explicitly and reject cycles during generation.

Do not apply wild species floors or remove a party slot. Non-level evolutions
have no inferred reverse relationship. Do not forward-evolve base species.
Preserve exact forms unless a validated predecessor edge specifies otherwise.
Report powerful species with no numeric predecessor for balance review.

The default move policy for every eligible slot is `LEVEL_UP`: create its
normal four-move set for the final species and effective level using the active
learnset mode. Bypass `CustomTrainerPartyAssignMoves` for these slots, including
slots that originally authored custom moves. Do not retain late-game custom
moves as an accidental fallback. Honor existing move-randomizer policy after
this baseline where that option explicitly applies.

A reviewed `AUTHORED_MOVES` exception is keyed by roster-owner ID and authored
slot index, with a reason. It retains the original move tuple only when the
final species equals the authored species and every nonempty move appears in
that species' active level-up learnset at or below the effective level.
Otherwise regenerate the entire set. This conservative rule intentionally
does not preserve TM, tutor, or egg moves without a level-up justification.
Validate both normal and legacy learnsets. No slot may end with zero usable
moves; generation must identify such outcomes and require a reviewed source
correction before release.

Retain authored held items, nature, IVs, EVs, friendship, nickname, ball, shiny
intent, and AI. Derive gender and ability against the final species. Preserve
an explicit ability only if present on that species; otherwise use its first
nonempty standard ability. Existing random-ability selection must select a
valid slot from the final species. A reversed species that cannot support an
authored gender uses its legal species gender behavior.

Audit form-specific held items and gimmick flags. If transformation invalidates
a species-specific form or gimmick, suppress that incompatible activation while
retaining the held item. List every affected slot in the report. Do not copy
an ability index from the authored species into a different final species.

## Runtime integration

The current central seams are `CreateNPCTrainerParty` and
`CreateNPCTrainerPartyFromTrainer` in `game/src/battle_main.c`. The former knows
the opposing ID and resolves `overrideTrainer`; the latter also serves debug
and player-party construction, so it must not unconditionally scale all callers.
Pass an explicit context or perform an equivalent validated transformation.

Resolve the existing rematch ID and roster override before selecting party-pool
entries. Preserve the original encounter ID for policy, and roster-owner ID
plus original selected slot index for move exceptions. Alias chains must have
validated termination and unambiguous ownership. Preserve the engine's selected
party count and two-opponent storage limits.

Consume the roster selected by `GetTrainerStructFromId`, including any existing
difficulty variants. Preserve the Hoenn content contract's fixed source-roster
selection. Where multiple difficulty variants exist, key move exceptions by
variant as well as roster-owner ID and slot, and validate every selectable
variant. Inventory the `.party` sources and `trainerproc` output rather than
assuming all Trainers use one literal C array.

For each selected slot, determine effective level and species before
`CreateMon`, personality constraints, ability assignment, move creation, and
stat calculation. Keep the original slot available for retained authored data.
Do not mutate or reconstruct the shared ROM roster in place. Avoid extra RNG
draws in level projection, predecessor resolution, and policy lookup.

When Trainer species randomization is active, pass original species, class,
selected-party position, and count to the existing randomizer in its established
order. Bypass predecessor resolution, scale the level, and create moves and
valid abilities for the randomized species. Do not use wild-randomizer state
as the Trainer-randomizer switch.

Existing explicit IV/EV challenge options run after base party creation as they
do today. They do not replace the Rating level calculation. Inventory any other
level-changing options or post-creation hooks and prevent double scaling for
eligible opponents. A player challenge level cap does not clamp enemy levels.
Document option precedence in the implementation audit before shipping.

Battle XP uses actual constructed Pokémon under the existing XP and player
soft-cap rules. Preserve prize money, rematch flags, defeat flags, scripts,
Trainer AI selection, and reward eligibility. Do not rewrite authored levels
merely to make reward readers observe projected values.

## Generation and audit

The implementation must deliver a deterministic host command that inventories
all compiled Wayfarer Trainer records and emits policy data plus a reviewable
report. Validate all selectable pool slots and resolved overrides, not only
the first party-size entries. Enumerate every Rating 0 through 80 and both
learnset modes for each eligible source slot. Cache equivalent calculations
where useful; emit summarized intervals rather than duplicate rows.

Report coverage counts by policy and region, excluded reasons, unresolved
classification candidates, custom-move replacements, move exceptions, species
changes, invalid or falling-back abilities, gender adjustments, gimmick
suppression, empty move sets, held-item concerns, and parties above the player
soft cap. Separate fatal structural failures from balance-review observations.

Include baseline and effective level ranges, party sizes, and representative
full parties at Ratings 0, 4, 8, 16, 30, 40, 55, 63, 65, 68, 76, and 80.
Identify the highest-level and largest early parties and high-stat species
that remain unevolved by policy. Show authored prize-money inputs alongside
effective levels and XP inputs. No claim that those populations are balanced
may be inferred solely from passing generation.

Generated output must reproduce from checked-in inputs without an AI service.
Batch source corrections are separate, reviewable edits. Do not silently turn
all custom-move Trainers into excluded opponents to satisfy the audit.

## Validation and rollout

1. Test every Rating and authored level against an independent integer oracle,
   exact anchors, signed rounding, bonuses, bounds, and monotonicity.
2. Test ordinary, Gym-member, and excluded policies, shared-class bosses,
   aliases, rematch variants, sparse IDs, and missing manifest records.
3. Test multi-stage reversal, ambiguous ancestry rejection, forms, non-level
   evolutions, no forward evolution, and no wild floor filtering.
4. Test move regeneration and exceptions below and at thresholds, changed
   species, both learnset modes, utility-heavy schedules, and usable moves.
5. Test legal ability and gender assignment after reversal and randomization,
   species-specific items and gimmicks, and retained IV/EV and AI behavior.
6. Test real battle construction, pools, two opponents with mixed policies,
   stable Rating snapshots, retries, and no extra projection RNG consumption.
7. Test randomizer and challenge precedence, player/partner/debug construction,
   facility exclusions, and recorded/link battle bypasses.
8. Verify defeat and rematch persistence, existing money rules, and battle XP
   from effective levels. Verify that player party levels never affect output.

Compile affected paths for every supported product and build a complete
Wayfarer release ROM. Run focused battle integration tests and the generated
inventory audit. Playtest representative parties across included regions at
Rating 0, a middle milestone, and Rating 80, including Gym-member doubles and
the highest-risk early parties reported by the audit.

Land classification and audit tooling before enabling runtime scaling. Enable
the shared feature only after all IDs are classified and structural validation
passes. Keep one build-time feature switch that restores authored construction
for rollback; it must bypass level, species, and move transformation together.
No save migration is required. Formula changes must regenerate the report and
repeat affected balance checks.

## References

- [Trainer Rating foundation and wild scaling](trainer-rating-wild-encounter-scaling.md)
- [Party progression](trainer-rating-party-progression.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Battle party construction](../../game/src/battle_main.c)
- [Trainer Rating runtime](../../game/src/trainer_rating.c)
- [Wild metadata generator](../../game/tools/wild_encounters/wild_encounters_to_header.py)
