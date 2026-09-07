# Gym Leader scaling

PRD: [Gym Leader scaling](../prds/gym-leader-scaling.md)
Implemented: Core implementation merged; structural acceptance and Wayfarer ROM playtesting pending.

## Scope

Implement the parent PRD for `IS_WAYFARER`: explicit initial-badge coverage,
six-slot authored rosters, deterministic selection, leader levels, and battle
construction. Ordinary Trainer scaling, TR advancement, player caps, and
rematch availability remain owned by their existing systems.

## Behavior

### Coverage and source authority

Add a distinct `GYM_LEADER` scaling policy alongside the ordinary, Gym-member,
and excluded policies. An opposing Trainer ID must receive exactly one policy.
Resolve the existing Trainer ID, roster owner, and difficulty variant before
looking up leader metadata. Do not infer enrollment from trainer class, name,
map, regional badge flags, or a party pointer alone.

Maintain a versioned, machine-readable coverage inventory for the 24 initial
badge encounters named in the PRD. Each entry records the encounter identity,
symbolic Trainer IDs, selectable difficulty variants, resolved roster owner,
badge-battle script references, and distinct excluded rematch or story IDs.
Enumerate aliases and runtime roster overrides. If an ID/owner is shared with
an excluded encounter, split or explicitly disambiguate that source before
enrollment; do not turn the excluded encounter into a scaled fight.

Every selectable initial-battle variant must resolve to exactly six authored
members and matching metadata. Shared source rosters may share metadata only
when all six slots and their policies match. Do not fill missing variants by
silently substituting another difficulty's team. Author in the existing party
source/generation pipeline; generated C output is not the editing authority.

The inventory and roster content must be reviewed before enabling the feature.
Report the exact six species/forms, move tuples or level-up policy, items,
aces, retention order, battle order, and level offsets per resolved variant.
Missing, duplicate, stale, or unknown IDs and unresolved script references
fail generation. All 24 encounter identities must be covered, with Tate/Liza
counted once. Existing rematch variants remain explicitly excluded.

### Per-roster data

Use source-array position 0 through 5 as retention order. Attach these fields
to each source slot without conflating them with its final party index:

| Field | Contract |
| --- | --- |
| `battleOrder` | Unique integer 0 through 5; ascending construction order. |
| `isAce` | Explicit boolean; all aces lie within the first two source slots. |
| `levelOffset` | Zero for an ace; minus one or minus two for every other member. |
| `movePolicy` | `AUTHORED` or `LEVEL_UP`; never inferred from scaled level. |

Slot 0 must be an ace. Normally a roster has one ace; two are allowed when
explicitly authored, with both in slots 0 and 1. Tate/Liza require Lunatone and
Solrock in those slots, both aces, and a double-battle marker. Species/forms
must match the reviewed roster exactly. Require a full permutation for
`battleOrder`; require ace entries after all non-aces in that ordering. With
two aces, their relative order is authored. Early retained prefixes inherit
the ordering by filtering; they do not get separate randomized orders.

`AUTHORED` preserves the original four move positions, including empty
positions, and requires at least one nonempty valid move. Existing custom
tuples must be copied unchanged, without a learnset or minimum-level gate.
`LEVEL_UP` applies to members without custom moves and generates their normal
four-move set at the effective level using the active learnset mode. Require
at least one usable move at every supported TR and learnset mode; report an
invalid low-level learnset as an authoring error, not a reason to drop a slot.

Retain the source slot identity for move tuples, abilities, items, gimmick
metadata, and other per-member fields during reordering. Where existing code
uses an index as a stable member input (including personality generation),
use the original source index rather than the new output position. In
particular, pass the source index to `GeneratePartyHash` and any retained
per-member seeded/randomizer inputs. Audit `CustomTrainerPartyAssignMoves`
and gimmick masks explicitly: member-data lookup uses the source index while
party writes and active-slot references use the output index. The same
retained member must not change authored traits when another member joins.
Test a non-identity battle order under fixed random inputs, including
personality-derived ability, gender, and shiny/OT behavior.

### Rating and party size

Reuse the existing battle-wide Trainer scaling snapshot. Read
`GetTrainerRating()` once per eligible battle setup, clamped to 0 through 80.
Reconstruction shares the snapshot; battle teardown clears it. A retry takes
a fresh snapshot. No scaled roster, historical rating, or level is saved.

For snapshot `r`, select the count:

```text
r < 8:   2
r < 22:  3
r < 34:  4
r < 40:  5
otherwise: 6
```

Keep source indices `[0, count)` and sort only those indices by `battleOrder`.
Do not use Trainer party pools to sample, replace, or reorder a covered roster.
There is no random jitter, player-party matching, regional adjustment, or
additional badge condition.

### Level calculation

Store an independent leader baseline table:

| Rating | Baseline |
| ---: | ---: |
| 0 | 15 |
| 4 | 16 |
| 8 | 18 |
| 16 | 23 |
| 30 | 30 |
| 40 | 42 |
| 55 | 60 |
| 65 | 80 |
| 80 | 100 |

For adjacent anchors `(r0, l0)` and `(r1, l1)`, let `d = r1 - r0` and
`n = (r - r0) * (l1 - l0)`. Use integer arithmetic with enough width:

```text
baseline = l0 + floor((2 * n + d) / (2 * d))
level = clamp(baseline + signed(levelOffset), 1, 100)
```

Exact halves round upward. Exact anchors produce their listed value. Do not
call the player soft-cap or ordinary Trainer level resolver to obtain this
baseline. Do not use original authored levels as an adjustment. A retained
member's level must never decrease as TR rises. At TR 0 the allowed levels
are 13, 14, 15; at TR 40 they are 40, 41, 42; at TR 80 they are 98, 99, 100.

### Species and other battle fields

Create the exact authored species/form at its effective level. Bypass numeric
predecessor resolution, wild species floors, and automatic forward evolution.
Run the leader move policy instead of ordinary scaling's custom-move fallback.
Later explicit move-randomizer options keep their existing behavior; the
unchanged-custom-move guarantee applies with move randomization off.

Copy authored held items, abilities, IVs, EVs, natures, balls, gender/shiny
settings, and other applicable member metadata through the existing constructor.
Keep trainer AI flags and trainer-use items. TR does not alter those fields;
existing explicit challenge options still apply in their normal order.

Expanding a source roster must preserve the pre-feature prize-money basis.
Capture each covered variant's current money inputs before editing its party,
and retain that basis explicitly if the existing formula derives it from
party size or authored levels. Constructed battle levels must not become
money inputs. Calculate battle experience normally from actual opponents.
Badge rewards, defeat flags, and reward scripts retain their current behavior.

### Battle construction and exclusions

Build a leader plan containing the selected source indices, effective levels,
and actual count after roster/variant resolution and before allocating or
constructing opponent members. Consume that plan in the existing opponent
constructor. Use the actual count for construction, returned party size, and
all downstream battle setup consumers; returning the original six is wrong
for a reduced team. Clear unused party slots using the existing setup path.

Audit all call sites that obtain opponent party size, initial active slots,
switch candidates, send-out order, and gimmick-slot reconstruction. Remap
source-indexed gimmicks to selected output positions and suppress metadata
for omitted members. Do not leave a six-member count or an omitted-slot
reference in a second setup path. Validate a plan fully before mutating the
opponent party. Invalid runtime metadata falls back to the complete existing
unscaled path using the legacy initial-party record defined below; malformed
shipped metadata is a build failure, so fallback is
not an acceptable production content fix.

Tate/Liza use one trainer in a double battle and may bring all six members.
The first two constructed entries are their opening pair; at count two these
must be Lunatone and Solrock. Do not convert the fight to singles. A true
two-opponent battle has a separate three-members-per-trainer capacity and is
outside leader scaling in this version. Assert that no initial badge script
uses that excluded format; do not accidentally apply its cap to Tate/Liza.

Party ordering does not constrain AI replacement selection or voluntary
switching. Do not add an ace-lock rule, battle AI tier, or scripted finale.
Reject `AI_FLAG_ACE_POKEMON` and `AI_FLAG_DOUBLE_ACE_POKEMON` on enrolled
rosters during generation: those existing flags reserve the last member(s)
and conflict with this ordering contract. Do not silently strip AI flags.

Exclude rematches, facilities, link/recorded/external parties, battle partners,
player parties, tutorials, and other special contexts before ID policy. Raw
party-pointer entry points do not authorize scaling. When trainer-species
randomization is active, bypass the complete leader plan before selection,
reordering, or leader-specific index changes, and preserve the
existing species-randomizer path, including its existing size and levels.
Select the same legacy initial-party record used by disabled-switch mode
before entering that path. Feed the randomizer the legacy source species,
source indices, count, and authored levels, never the expanded six-slot roster.
Do not modify rematch parties when their initial roster has shared aliases.

### Enablement and authority

Provide one Wayfarer build-time switch for the whole leader transformation.
Keep it disabled until all roster, validation, and playtest requirements are
met. Standalone builds must retain their pre-feature party sources and behavior.
Preserve the original initial-party records as a separate fallback authority:
disabling the switch must restore original size, species, levels, order, moves,
and money inputs, not expose the expanded six-slot authoring roster unscaled.
An implementation can select between legacy and leader roster definitions
before construction; do not overwrite a shared standalone/rematch source.

This spec supersedes static initial-Gym-party statements in the League circuit
and regional content documents only for its enrolled contexts. Ordinary
scaling still excludes leaders from its own transformation; its inventory and
audit tools must recognize the new policy without treating it as ordinary.

## Validation

Generate an inspectable report for all 24 encounters, every selectable variant,
and every integer TR from 0 through 80. Include exact selected source indices,
output order, count, species/forms, levels, moves, items, aces, and money basis.
Compare authored fields against the reviewed source inventory. Separate fatal
structural errors from balance observations about strong species or moves.

Required automated checks cover:

1. All size thresholds and adjacent values (7/8, 21/22, 33/34, 39/40), all level
   anchors, interpolation ties, clamping, and monotonic levels and membership.
2. Every retained prefix, ace retention, ordered construction, absent-slot
   clearing, actual returned count, and all party-size/gimmick consumers.
3. Exact species/forms, custom tuples above normal learning levels, held items,
   stable source-slot identity, normal and legacy level-up moves, and abilities.
4. Tate/Liza at two and six members, opening pairs, both aces at baseline level,
   double-battle semantics, AI freedom to switch or select replacements, and
   generation rejection of ace-lock AI flags.
5. Shared snapshot and reconstruction, reset after battle, retry after a TR
   change, and unchanged badge rewards and pre-feature money inputs.
6. Difficulty variants, aliases, runtime overrides, excluded rematches and
   contexts, trainer-species randomizer bypass, and move-randomizer precedence.
   For every leader/variant, compare randomizer inputs and outputs with the
   disabled-switch legacy path under fixed random inputs, including source
   species, count, indices, and levels.
7. Disabled-switch restoration and standalone build parity, including original
   party sources, plus rejection of missing inventory or malformed metadata.

Compile the affected engine/generator paths and run targeted battle-mechanics
tests. Playtest the cases in the PRD on a Wayfarer ROM, including size
transitions and remaining Gyms before and after a League clear. The core
implementation and targeted automated validation are merged, but
`B_GYM_LEADER_SCALING` remains disabled by default pending structural acceptance
and Wayfarer ROM playtesting. Do not claim balance acceptance from generated
tables alone.

## References

- [Ordinary Trainer scaling specification](trainer-party-scaling.md)
- [Trainer Rating and League circuit](wayfarer-interregional-league-circuit.md)
- [Player soft-cap curve](trainer-rating-party-progression.md)
- [Opponent construction](../../game/src/battle_main.c)
- [Trainer scaling and snapshot](../../game/src/trainer_party_scaling.c)
- [Trainer party selection](../../game/src/trainer_pools.c)
