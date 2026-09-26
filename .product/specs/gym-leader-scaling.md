# Gym Leader scaling

PRD: [Gym Leader scaling](../prds/gym-leader-scaling.md)
Implemented: No for trainer-owned world progression. The earlier player-TR
scaler is present in code but `B_GYM_LEADER_SCALING` defaults to `FALSE`.
Giovanni's Wayfarer finale uses a separate implemented projection.

## Scope and authority

This is the target contract for initial singles badge encounters in Wayfarer.
Use [trainer world progression](trainer-world-progression.md) for personal
rating, milestones, stages, level calculation, and battle snapshot lifetime.
Replace the old player-TR input and prefix-size selection only for enrolled
encounters when the new policy is adopted. Current code remains unchanged by
this specification update.

Ordinary trainers, Gym members, wild encounters, player caps, TR rewards,
rematch availability, and circuit registration retain their owning contracts.
The shared personal rating evaluator does not authorize changes to those systems.

## Coverage and identity

Maintain a versioned inventory of actual Wayfarer initial-badge encounter IDs,
selectable difficulty variants, roster owners, canonical character IDs, and
stage/profile references. Exactly one battle policy may own an encounter.
Resolve the ID, context, and selected variant before consulting trainer data;
never infer enrollment from display name, class, region, or a party pointer.

The target contains 23 singles badge identities listed in the PRD. Giovanni's
Viridian finale is included through its actual Wayfarer trainer ID; his villain
encounters remain excluded. The current generated six-slot inventory contains
23 identities including Tate/Liza, with Giovanni handled separately. It is not
the new coverage manifest and must not be copied without an explicit mapping.

The twenty-fourth badge remains Tate/Liza's existing double battle, outside
this singles policy. Keep its current flag-dependent construction and reward
behavior; test both current enabled/disabled scaling paths where applicable.
Do not convert the fight to singles or apply two-opponent party-size limits to
its single trainer party.

Blue's experimental `gymEligible` classification is content metadata, not proof
of a current Wayfarer badge encounter. His HNS Gym ID is excluded in Wayfarer;
opening, rival, and Dojo contexts retain their separate policies. Leader
rematches, facilities, partners, player parties, link/recorded/external battles,
tutorials, and other special contexts remain explicitly excluded. Raw party
entry points cannot bypass that check.

Shared source aliases must not enroll an excluded battle. Split or disambiguate
sources when necessary, and reject missing, duplicated, stale, or unresolved
coverage at generation time. Author in the source/generator pipeline rather
than editing generated C output. Report the source and canonical mapping for
every covered variant.

## Rating and stage selection

After eligibility and variant resolution, capture global badge count and the
three-venue lifetime-clear mask from the shared runtime. Evaluate the canonical
trainer's personal curve; do not read player `GetTrainerRating()` as the NPC
rating input. Snapshot effective TR, selected stage/profile, content versions,
and resulting member plan before constructing the opponent.

Select the highest stage threshold not exceeding effective TR. Stage count,
species, and order come from the authored profile; they do not come from the
old 8/22/34/40 player-TR thresholds. Each supported stage and variant is a
complete reviewed team with 2–6 members and at least one explicit ace.
Do not fill missing variants with another difficulty's party or randomly
sample a pool. The shared specification owns curve rounding, saturation,
level anchors, offset validation, and monotonic transition requirements.

All reconstruction in that battle uses the same plan. Clear transient state at
teardown; a new attempt resolves current milestones again. At an unchanged world
point, no new membership, level jitter, or profile choice occurs. Ordinary
Pokémon battle RNG continues as before. Badge/first-clear rewards are committed
after battle and affect only later world encounters.

## Stage member metadata

Keep a stable source-member identity within each stage and variant, separate
from output position. A profile supplies a complete battle-order permutation,
ace flags, signed level offsets, exact species/forms, and `AUTHORED` or
`LEVEL_UP` move policy. Normally place the ace after support members. Stages
may deliberately replace species or moves; these are reviewed source changes,
not automatic runtime evolution or move repair.

`AUTHORED` preserves the reviewed four positions, including empty positions,
and requires at least one usable move. Do not add learnset-level filtering to
that policy. `LEVEL_UP` uses the selected species' normal learnset at effective
level and must yield a usable move throughout its supported range. Validate
content rather than silently dropping an invalid member.

Copy stage-authored items, abilities, IVs, EVs, natures, gender/shiny settings,
balls, trainer inventory, and AI through the existing constructor. TR selects
a stage and level; it does not synthesize those fields. Move randomizers and
other existing challenge overrides retain their explicit precedence.

Use source identity for member-data lookups, `GeneratePartyHash`, and retained
randomizer/personality inputs; use output positions for party writes and active
slots. Remap gimmick masks explicitly. Reordering must not swap moves, items,
abilities, or traits between members. The same member in the same stage and
variant remains stable under reconstruction. Cross-stage changes use explicitly
authored identity, not an accidental output index.

## Construction and rewards

Build and validate the full plan before allocating opponent members. Use its
actual count everywhere: construction, returned party size, opening slots,
switch candidates, send-out order, and gimmick reconstruction. Clear unused
slots; reject references to absent members. Do not return a historical six-slot
count when a stage contains fewer members.

An authored battle order does not force voluntary switches or replacement AI.
Reject incompatible ace-lock flags during content validation rather than silently
stripping them or promising a scripted final Pokémon. Early support dependencies
must fit the complete opening stage.

Preserve the existing encounter's prize-money basis, including Giovanni's
special path, independently of newly authored stage sizes and source levels.
Battle experience follows actual opponents normally. Badge awards, defeat flags,
TR rewards, scripts, and access rules remain owned by their current systems.

## Overrides and enablement

Species randomization bypasses the complete new stage plan and retains the
existing legacy-source inputs and constructor behavior. Preserve source species,
indices, count, and authored levels supplied to that path. A disabled feature
switch must also restore the existing encounter behavior, not expose a new
competitive stage unscaled. Do not overwrite standalone or rematch authorities.

Keep the new policy disabled until inventory, content, validation, and playtesting
pass. Invalid shipped metadata is a build failure; runtime invalidity must fail
preparation before partially replacing a party, rather than substituting player
TR, another trainer, or a random stage. Existing disabled/excluded paths remain
explicit policies, not recovery from malformed new content.

## Validation

Produce a report for all 23 singles badge identities, every selectable variant,
badges 0–24, and first-clear counts 0–3. Include canonical/source identity,
effective TR, stage/profile, member identities, output order, count, species,
levels, moves, items, ace flags, and prize-money basis.

Required checks cover:

1. Personal curve anchors, rounding ties, clamps, monotonic growth, stage
   boundaries and adjacent values, nondecreasing default size/minimum level,
   and unchanged player cap/ordinary trainer policies.
2. Exact member metadata, move policies at every supported level, non-identity
   battle ordering, gimmick remapping, actual counts, and unused-slot clearing.
3. Repeated construction in one battle, unchanged-milestone retries, progression
   earned elsewhere before a retry, and badge awards only after the fight.
4. Giovanni's actual Viridian variant and excluded villain aliases; Blue's
   excluded Wayfarer story/Dojo contexts; no enrollment by `gymEligible` alone.
5. Tate/Liza's existing double battle and twenty-fourth badge, randomizer bypass,
   disabled-switch behavior, and unchanged standalone/rematch parties.
6. Early and postponed leaders, post-league growth, Gym-member comparisons,
   strong species/moves/items, and source-faithful reward behavior in emulator
   playtests. Structural tests and the explorer do not establish combat balance.

Run relevant mechanics and Gym journey tests, Wayfarer production builds, and
standalone builds affected by shared construction changes. Record content
review and playtest acceptance separately from implementation status.

## References

- [Trainer world progression](trainer-world-progression.md)
- [Current Gym scaler switch](../../game/include/config/trainer_party_scaling.h)
- [Current source inventory generator](../../game/tools/trainer_scaling/gym_leaders.py)
- [Trainer party construction](../../game/src/battle_main.c)
- [Ordinary trainer scaling](trainer-party-scaling.md)
- [Seeded circuit pool](circuit-trainer-pool.md)
