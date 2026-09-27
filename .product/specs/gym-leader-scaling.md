# Gym Leader scaling

PRD: [Notable trainers](../prds/notable-trainers.md)
Implemented: No for the v0 model. The earlier player Trainer Rating (TR)
scaler is present in code but `B_GYM_LEADER_SCALING` defaults to `FALSE`;
Giovanni's Wayfarer finale uses a separate implemented projection. Today's
code keeps that behavior until adoption.
Design status: v0 contract for singles badge encounters.

## Scope and authority

This specification owns Gym badge-encounter coverage and is the single
authority for battle construction: its member-metadata, construction, reward,
and override rules apply to every battle with a notable character, not only
Gym battles. [Notable trainers](notable-trainers.md) owns the notable trainer
inventory, the every-battle rule, trainer TR, scalers, rosters, team
resolution, and the battle snapshot; this document does not restate them. A
Gym battle uses the leader's own TR, team, and levels with no Gym-specific
adjustment. Adoption replaces the old player-TR input and prefix-size
selection only for enrolled encounters. Regular trainers, Gym members, wild
encounters, the level cap, TR rewards, rematch availability, and entering a
league keep their owning contracts.

## Coverage and identity

Maintain a versioned inventory of actual Wayfarer initial-badge encounter IDs,
canonical character IDs, and roster references. Exactly one battle policy may
own an encounter. Resolve the ID and context before consulting trainer data;
never infer enrollment from display name, class, region, or a party pointer.

v0 covers the 23 singles badge opponents listed in the
[PRD](../prds/notable-trainers.md#gym-battles), including
Giovanni's Viridian finale through its actual Wayfarer trainer ID. Today's
generated six-slot inventory (23 identities including Tate/Liza, Giovanni
handled separately) is not this manifest and must not be copied without an
explicit mapping.

The twenty-fourth badge remains Tate/Liza's existing double battle. Keep its
current flag-dependent construction and rewards, test both scaling paths where
applicable, and do not convert it to singles or apply two-opponent size limits
to its single party.

Blue's `gymEligible` content metadata creates no badge encounter; his HNS Gym ID
is excluded in Wayfarer. A leader's battles outside their badge encounter are
not in this inventory; the
[notable trainer inventory](notable-trainers.md#notable-trainer-inventory)
routes them to the same TR and roster, built under this document's construction
rules. Facilities, partners, link/recorded/external battles, tutorials, and
other special contexts stay excluded, and raw party entry points cannot bypass
the check. Shared source aliases must not enroll an excluded battle; split or
disambiguate them. Reject missing, duplicated, stale, or unresolved coverage at
generation time, author in the source/generator pipeline rather than generated
C, and report the source and canonical mapping of every covered encounter.

## Battle snapshot resolution

After eligibility resolution, obtain the members from the
[battle snapshot](notable-trainers.md#battle-snapshot): the leader's TR and
resolved team, frozen for the battle. Never read `GetTrainerRating()`, party
levels, badges, historical Gym order, or the old 8/22/34/40 player-TR
thresholds. Each leader has one complete reviewed roster.

## Member metadata

Keep a stable member identity (the roster slot) separate from output position.
The battle snapshot supplies battle order, levels, species/forms, and moves:
`AUTHORED` moves exactly as written, `LEVEL_UP` moves from the existing
constructor. Validate content rather than silently dropping a member.

Copy roster-authored items, abilities, natures, IVs/EVs, trainer inventory,
and AI through the existing constructor; the model never synthesizes those
fields.

Use member identity for member-data lookups, `GeneratePartyHash`, and retained
randomizer/personality inputs; use output positions for party writes and active
slots. Remap gimmick masks explicitly. Reordering must not move moves, items,
abilities, or traits between members. Identity across team sizes is the roster
slot, never an accidental output index.

## Construction and rewards

Build and validate the full battle snapshot before allocating opponent members.
Use its actual count for construction, returned party size, opening slots,
switch candidates, send-out order, and gimmick reconstruction; clear unused
slots and reject references to absent members. The reversed order does not force
switches or replacement AI; reject incompatible ace-lock flags in content
validation.

Preserve each encounter's prize-money basis, including Giovanni's special path,
independently of team size and source levels. Experience follows the actual
opponents. Badge awards, defeat flags, TR rewards, scripts, and access rules
stay with their current systems; the badge is committed after the battle.

## Overrides and enablement

Trainer-species randomization bypasses the new roster and keeps its legacy
source species, indices, count, levels, and constructor. Other challenge and
move randomizer options keep their explicit precedence. A disabled switch
restores existing encounter behavior rather than exposing an unscaled new
roster. Standalone builds are untouched. Leader rematches resolve from the
leader's TR under the every-battle rule, built with these same rules.

Keep the policy disabled until inventory, content, validation, and playtesting
pass. Invalid shipped metadata fails the build; runtime invalidity fails
preparation before any party is replaced, never falling back to player TR,
another trainer, or a random team.

## Validation

Report all 23 singles identities: canonical/source identity, TR, roster version,
member roster slots, output order, count, species, levels, moves, items, and
prize-money basis. Check:

1. The model obligations in
   [Notable trainers](notable-trainers.md#validation) for
   every Gym Leader.
2. Member metadata, move policies at every supported level, non-identity
   ordering, gimmick remapping, actual counts, and unused-slot clearing.
3. Repeated construction in one battle, identical retries, and badge awards
   only after the fight.
4. Giovanni's Viridian finale encounter; Blue's excluded Gym ID; no enrollment
   from `gymEligible` alone.
5. Tate/Liza's double battle and badge, randomizer bypass, disabled-switch
   behavior, unchanged standalone parties, and rematches resolved from the
   leader's TR.
6. Emulator playtests of low- and high-rated leaders, Gym-member comparisons,
   strong species/moves/items, and reward behavior. The
   [explorer](../../devtools/ui/README.md#trainer-balance-explorer) predicts
   parties and levels only.

Run relevant mechanics and Gym journey tests, Wayfarer production builds, and
standalone builds affected by shared construction changes. Record content review
and playtest acceptance separately from implementation status.

## Later

- Gym-specific behavior that follows a changing leader TR is owned by the
  model's [Later](notable-trainers.md#later) list.

## References

- [Notable trainers](notable-trainers.md)
- [Today's Gym scaler switch](../../game/include/config/trainer_party_scaling.h)
- [Today's source inventory generator](../../game/tools/trainer_scaling/gym_leaders.py)
- [Trainer party construction](../../game/src/battle_main.c)
- [Regular trainer scaling](trainer-party-scaling.md)
- [Leagues](leagues.md)
