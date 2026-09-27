# Gym Leader scaling

PRD: [Gym Leader scaling](../prds/gym-leader-scaling.md)
Implemented: No for world progression. The earlier player-TR scaler is present
in code but `B_GYM_LEADER_SCALING` defaults to `FALSE`; Giovanni's Wayfarer
finale uses a separate implemented projection. Current code keeps that
behavior until adoption.
Design status: Target contract for initial singles badge encounters.

## Scope and authority

This specification owns Gym badge-encounter coverage and battle construction.
[Trainer world progression](trainer-world-progression.md) owns the world cap,
standing, arcs, rosters and team composition, level resolution, and the Gym
snapshot; this document does not restate them. Adoption replaces the old
player-TR input and prefix-size selection only for enrolled encounters.
Ordinary trainers, Gym members, wild encounters, player caps, TR rewards,
rematch availability, and circuit registration keep their owning contracts.

## Coverage and identity

Maintain a versioned inventory of actual Wayfarer initial-badge encounter IDs,
selectable difficulty variants, roster owners, canonical character IDs, and
roster references. Exactly one battle policy may own an encounter.
Resolve the ID, context, and variant before consulting trainer data; never infer
enrollment from display name, class, region, or a party pointer.

The target covers the 23 singles badge opponents listed in the PRD, including
Giovanni's Viridian finale through its actual Wayfarer trainer ID; his villain
encounters stay excluded. The current generated six-slot inventory (23
identities including Tate/Liza, Giovanni handled separately) is not this
manifest and must not be copied without an explicit mapping.

The twenty-fourth badge remains Tate/Liza's existing double battle. Keep its
current flag-dependent construction and rewards, test both scaling paths where
applicable, and do not convert it to singles or apply two-opponent size limits
to its single party.

Blue's `gymEligible` content metadata creates no badge encounter; his HNS Gym
ID is excluded in Wayfarer. Opening, rival, and Dojo battles, leader rematches,
facilities, partners, link/recorded/external battles, tutorials, and other
special contexts stay excluded, and raw party entry points cannot bypass the
check. Shared source aliases must not enroll an excluded battle; split or
disambiguate them. Reject missing, duplicated, stale, or unresolved coverage at
generation time, author in the source/generator pipeline rather than generated
C, and report the source and canonical mapping of every covered variant.

## Plan resolution

After eligibility and variant resolution, obtain the member plan from
[trainer world progression](trainer-world-progression.md#gym-encounters): world
point, arc, standing, and the composed roster team (size, aces, fillers
including any traded filler, forms, levels, moves, and battle order), frozen
for the battle.
Early teams still start at two and grow with the leader's strength. Never read
`GetTrainerRating()`, party levels, historical Gym order, or the old 8/22/34/40
player-TR thresholds. Each variant references a complete reviewed roster; do
not fill a missing variant from another difficulty.

## Member metadata

Keep a stable member identity (`aceId` or `fillerId`) separate from output
position. The composed plan supplies the battle order, ace flags, levels, forms
from the authored lines, and moves: authored per form for aces, `LEVEL_UP` for
authored fillers, and `TRADED` for a traded filler. Validate content rather than
silently dropping a member.

Copy roster-authored items, abilities, natures, trainer inventory, and AI
through the existing constructor, and a traded filler's persisted identity
from its trade record; the growth model never synthesizes those fields.

Use member identity for member-data lookups, `GeneratePartyHash`, and retained
randomizer/personality inputs; use output positions for party writes and active
slots. Remap gimmick masks explicitly. Reordering must not move moves, items,
abilities, or traits between members. Identity across team sizes is the
member reference, never an accidental output index.

## Construction and rewards

Build and validate the full plan before allocating opponent members. Use its
actual count for construction, returned party size, opening slots, switch
candidates, send-out order, and gimmick reconstruction; clear unused slots and
reject references to absent members. An authored order does not force switches
or replacement AI; reject incompatible ace-lock flags in content validation.

Preserve each encounter's prize-money basis, including Giovanni's special path,
independently of team size and source levels. Experience follows the actual
opponents. Badge awards, defeat flags, TR rewards, scripts, and access rules
stay with their current systems; the badge is committed after the battle.

## Overrides and enablement

Trainer-species randomization bypasses the new roster plan and keeps its legacy
source species, indices, count, levels, and constructor. Other challenge and
move randomizer options keep their explicit precedence. A disabled switch
restores existing encounter behavior rather than exposing an unscaled new
roster. Standalone and rematch authorities are untouched.

Keep the policy disabled until inventory, content, validation, and playtesting
pass. Invalid shipped metadata fails the build; runtime invalidity fails
preparation before any party is replaced, never falling back to player TR,
another trainer, or a random team.

## Validation

Report all 23 singles identities × every variant × B 0–24 × C 0–3 × each allowed
arc: canonical/source identity, arc, standing, worldCap, roster version, member
identities, output order, count, species, levels, moves, items, ace flags, and
prize-money basis. Check:

1. The growth-model obligations in
   [trainer world progression](trainer-world-progression.md#validation) for
   every Gym Leader, including the ace-minus-cap report (mean near -2, spread
   within about ±3, no widening with clears) and early team sizes growing
   from 2 to 6.
2. Member metadata, move policies at every supported level, non-identity
   ordering, gimmick remapping, actual counts, and unused-slot clearing.
3. Repeated construction in one battle, identical retries at unchanged
   milestones, progression earned elsewhere before a retry, and badge awards
   only after the fight.
4. Giovanni's Viridian variant and excluded villain aliases; Blue's excluded
   contexts; no enrollment from `gymEligible` alone.
5. Tate/Liza's double battle and badge, randomizer bypass, disabled-switch
   behavior, and unchanged standalone/rematch parties.
6. Emulator playtests of early and postponed leaders, Gym-member comparisons,
   strong species/moves/items, and reward behavior. The
   [explorer](../../devtools/ui/README.md#trainer-balance-explorer), being
   reworked to model arcs and the cap gap, predicts parties and levels only.

Run relevant mechanics and Gym journey tests, Wayfarer production builds, and
standalone builds affected by shared construction changes. Record content review
and playtest acceptance separately from implementation status.

## References

- [Trainer world progression](trainer-world-progression.md)
- [Current Gym scaler switch](../../game/include/config/trainer_party_scaling.h)
- [Current source inventory generator](../../game/tools/trainer_scaling/gym_leaders.py)
- [Trainer party construction](../../game/src/battle_main.c)
- [Ordinary trainer scaling](trainer-party-scaling.md)
- [Circuit trainer pool](circuit-trainer-pool.md)
