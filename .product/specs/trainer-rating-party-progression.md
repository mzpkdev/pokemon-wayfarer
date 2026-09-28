# Trainer Rating party progression

PRD: [Trainer Rating wild encounter and party progression](../prds/trainer-rating-wild-encounter-scaling.md)
Implemented: Partial

Today's [circuit producer](../../game/src/league_circuit.c) implements +8/+8/+8
first-league-win contributions for Indigo, Masters, and Hoenn. The badge formula
and level cap anchors are unchanged. These circuit inputs are live today; the
Partial marker does not certify completion of all experience, obedience, and
cross-build acceptance listed below.

The v0 TR design (rescaled formula, uncapped TR, scalers, and notable
trainers' separate TR) is in [Player Trainer Rating](player-trainer-rating.md);
this document owns the v0 level cap curve in
[v0 level cap curve](#v0-level-cap-curve). Everything else below is Today.

## Scope

This specification defines the Wayfarer level cap, numerical experience
reduction, and obedience rules derived from Trainer Rating (TR). The TR
foundation owns the shared value and persistence; the interregional League
circuit supplies its global badge and first-league-win inputs. The wild
encounter scaling specification owns ordinary wild levels and species
eligibility. Regional starts and circuit opponent tiers are not prerequisites
for this foundation or for the party-progression mechanics.

The existing missing-badge catch penalty remains active under its existing
rules. This specification does not replace, disable, or remap that penalty.

## Behavior

### Level cap curve

Wayfarer derives one level cap from the current TR. The cap is not saved
separately. Every consumer clamps TR to the inclusive range 0 through 80 and
resolves the cap from this independently authored table:

| TR | Level cap |
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

For a TR between two rows, linearly interpolate between their caps and
round to the nearest integer, with an exact half rounded upward. In equivalent
integer terms, for adjacent rows `(r0, c0)` and `(r1, c1)` and a clamped TR
`r`:

```text
cap = c0 + roundHalfUp((r - r0) * (c1 - c0) / (r1 - r0))
```

The result is clamped to 1 through 100. The curve is monotonic and produces
these global circuit examples. The
[circuit](wayfarer-interregional-league-circuit.md) allows all badges before any
league win. The examples include the approved +8 per first league win, already
implemented by Today's circuit producer. The badge formula and level cap curve
are unchanged:

| Progress | TR | Level cap |
| --- | ---: | ---: |
| New game | 0 | 15 |
| 4 total badges, no league wins | 16 | 23 |
| 8 total badges, no league wins | 40 | 42 |
| 8 total badges, Indigo won | 48 | 52 |
| 16 total badges, Indigo won | 56 | 62 |
| 16 total badges, Indigo and Masters won | 64 | 78 |
| 24 total badges, no league wins | 56 | 62 |
| 24 total badges, Indigo won | 64 | 78 |
| 24 total badges, Indigo and Masters won | 72 | 89 |
| 24 total badges, all three leagues won | 80 | 100 |

These initial values equal the current wild encounter level anchor plus 10.
The party curve remains separate source data. Changing wild encounter anchors
does not change party caps, and changing party caps does not change wild
encounters.

### v0 level cap curve

On the [v0 TR scale](player-trainer-rating.md#formula-v0) the level
cap uses these placeholder anchors, interpolated as a
[scaler](player-trainer-rating.md#scalers) and flat at Lv 100 from TR 160.
Unlike Today's cap, it is not tied to the wild level curve plus 10:

| Badges | v0 TR | Level cap |
| ---: | ---: | ---: |
| 0 | 0 | 15 |
| 4 | 40 | 28 |
| 8 | 80 | 50 |
| 16 | 120 | 75 |
| 24 | 160 | 100 |

In v0, league wins do not move the cap. Experience reduction, obedience, Exp.
Candy, Rare Candy, and the missing-badge catch penalty are unchanged; they
read whatever cap this curve yields.

### Numerical experience

The level cap is soft: its reduction applies independently to each Pokémon
receiving a positive numerical experience award. It covers battle participation,
catch experience, Exp. Share, and Day Care experience. Existing eligibility
rules still decide whether the Pokémon receives an award. Exp. Candy does not
use the reduction.

Calculate the amount the Pokémon would ordinarily receive first, including
trade, held-item, global experience, and challenge-option multipliers. Apply
the level cap reduction to that final amount before adding experience or
processing level gains.

Let `C` be the current level cap, `E` the Pokémon's experience before the award,
`T` the minimum total experience for level `C`, and `A` the otherwise-awarded
amount. Split the award as follows:

```text
full = min(A, max(T - E, 0))
reducedBase = A - full
reduced = 0                              when reducedBase = 0
reduced = max(1, floor(reducedBase / 2)) when reducedBase > 0
granted = full + reduced
```

This gives full experience up to the start of the cap level and half experience
after that boundary. A Pokémon already at or above the cap receives half of
the entire award. An otherwise-positive reduced portion always grants at least
one experience point. A zero award remains zero.

Day Care treats the accumulated experience being applied on withdrawal as one
award for this calculation. Exp. Candy bypasses the reduction entirely: it
grants its normal full numerical experience amount, and its existing display
continues to report that full amount. Rare Candy remains unaffected and grants
its normal level increase. Reaching a level above the cap through either Candy
does not disable later numerical-experience reduction from a covered source.

An enabled challenge level cap keeps its existing behavior. Apply the TR
reduction first, then let the challenge rule further restrict or cancel the
award. The stricter result wins.

### Obedience

Resolve obedience against the current level cap whenever a player-controlled
Pokémon attempts an action in a battle where obedience applies. A Pokémon at
the exact cap obeys.

The reference level depends on ownership:

| Pokémon state | Reference level |
| --- | ---: |
| Egg | Always obeys |
| Same original Trainer as the current player | Met level |
| Different original Trainer | Current level |

If the reference level is at or below the current cap, the Pokémon obeys. If it
is above the cap, use the existing disobedience outcome selection. This keeps
the established chances and outcomes for loafing, choosing another move,
falling asleep, and hurting itself.

A same-OT Pokémon met within the cap remains obedient if training later raises
its current level above the cap. A same-OT Pokémon met above the cap becomes
obedient as soon as the cap reaches its met level. A foreign-OT Pokémon becomes
obedient only while the cap covers its current level.

Link battles, recorded battles, Battle Frontier battles, and the player's
in-game battle partner keep their existing obedience exemptions. Existing
species-specific obedience rules remain in force. Wayfarer does not derive its
obedience threshold from regional badge flags, and a regional eighth badge does
not bypass the TR cap. Other product builds retain their existing obedience
behavior until they explicitly adopt this specification.

### Missing-badge catch penalty

The configured missing-badge catch penalty continues to use its existing badge
count, level thresholds, and cumulative catch-rate reduction. Wild encounter
projection and the level cap do not alter its calculation. A wild Pokémon may
therefore be harder to catch under that rule and may also disobey after capture
when its obedience reference level exceeds the level cap.

Under the current Generation IX rule, the capture runtime counts its existing
eight badge flags and clamps the result to eight. Starting at the current badge
count, each threshold below the wild Pokémon's level multiplies the catch odds
by `4 / 5` using integer arithmetic. The thresholds remain 25, 30, 35, 40, 45,
50, 55, 60, and 100 for indices zero through eight. This feature does not
replace that badge source with global badge count.

### Shared implementation authority

Every covered Wayfarer experience source calls one shared reduction helper so
award ordering and rounding cannot drift. Exp. Candy and Rare Candy never call
that helper. All Wayfarer obedience checks call one shared level cap resolver.
The resolver reads TR rather than a regional badge count or the TR save variable
as though it were already a Pokémon level.

The existing generic `EXP_CAP_SOFT` behavior is not the authority for this
feature. Its progressively smaller divisors do not implement the flat one-half
rule. Wayfarer may reuse its call sites only after they route through the rules
defined here.

The cap is derived from existing saved TR, and obedience uses existing Pokémon
ownership, met-level, and current-level data. The feature does not require a
separately saved cap or new per-Pokémon state.

### Validation

Deterministic tests must cover:

1. Every integer TR from 0 through 80, exact anchor values,
   round-half-up interpolation, range clamping, and monotonic caps.
2. The cap curve's anchor values, including level 15 at TR 0 and level 100
   at TR 80, with controlled seeded TR values for values normally supplied by
   the later circuit.
3. Numerical awards wholly below the cap, ending exactly at it, crossing it,
   and beginning at or above it.
4. Even, odd, one-point, and zero reduced portions, including the positive
   one-point minimum.
5. Battle participation, catch experience, Exp. Share, and Day Care through the
   shared reduction rules.
6. Trade, held-item, global experience, and challenge-option multiplier
   ordering, followed by any stricter challenge level cap.
7. Normal Exp. Candy and Rare Candy use below, at, and above the level cap,
   including the full displayed Exp. Candy award.
8. Same-OT Pokémon met below, exactly at, and above the cap, including a current
   level above the cap in every case.
9. Foreign-OT Pokémon below, exactly at, and above the cap, including a foreign
   Pokémon that grows while the cap is unchanged.
10. Eggs, each retained battle-format exemption, existing species-specific
    rules, and the removal of regional badge and eighth-badge authority in
    Wayfarer.
11. Immediate obedience changes when TR raises the cap, plus save
    and load with no separately persisted cap.
12. Unchanged missing-badge catch odds across representative badge counts and
    wild levels, including a capture that is also above the level cap.
13. Unchanged experience and obedience behavior in builds that have not adopted
    the Wayfarer party progression rules.

Compile the affected battle, party, item, Day Care, capture, and TR objects for
every supported product build. Build at least one complete Wayfarer ROM and
exercise the TR 0, seeded mid-curve, and TR 80 boundaries. The later circuit
specification owns testing its badge and first-league-win inputs.

## References

- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Level-cap configuration](../../game/include/config/caps.h)
- [Level-cap runtime](../../game/src/caps.c)
- [Experience awards](../../game/src/battle_script_commands.c)
- [Obedience runtime](../../game/src/battle_util.c)
