# Trainer Rating party progression

PRD: [Trainer Rating wild encounter and party progression](../prds/trainer-rating-wild-encounter-scaling.md)
Implemented: No

## Scope

This specification defines the Wayfarer soft level cap, numerical experience
reduction, and obedience rules derived from Trainer Rating. The interregional
League circuit owns Trainer Rating derivation and persistence. The wild
encounter scaling specification owns ordinary wild levels and species
eligibility.

The existing missing-badge catch penalty remains active under its existing
rules. This specification does not replace, disable, or remap that penalty.

## Behavior

### Soft level-cap curve

Wayfarer derives one soft level cap from the current Trainer Rating. The cap is
not saved separately. Every consumer clamps the rating to the inclusive range
0 through 80 and resolves the cap from this independently authored table:

| Trainer Rating | Soft level cap |
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

For a rating between two rows, linearly interpolate between their caps and
round to the nearest integer, with an exact half rounded upward. In equivalent
integer terms, for adjacent rows `(r0, c0)` and `(r1, c1)` and a clamped rating
`r`:

```text
cap = c0 + roundHalfUp((r - r0) * (c1 - c0) / (r1 - r0))
```

The result is clamped to 1 through 100. The curve is monotonic and produces
these global circuit milestones:

| Progress | Trainer Rating | Soft level cap |
| --- | ---: | ---: |
| New game | 0 | 15 |
| 4 total badges | 16 | 23 |
| 8 total badges | 40 | 42 |
| Kanto League cleared | 55 | 60 |
| 16 total badges | 63 | 76 |
| Johto League cleared | 68 | 84 |
| 24 total badges | 76 | 95 |
| Hoenn League cleared | 80 | 100 |

These initial values equal the current wild encounter level anchor plus 10.
The party curve remains separate source data. Changing wild encounter anchors
does not change party caps, and changing party caps does not change wild
encounters.

### Numerical experience

The soft-cap reduction applies independently to each Pokémon receiving a
positive numerical experience award. It covers battle participation, catch
experience, Exp. Share, Exp. Candy, and Day Care experience. Existing
eligibility rules still decide whether the Pokémon receives an award.

Calculate the amount the Pokémon would ordinarily receive first, including
trade, held-item, global experience, and challenge-option multipliers. Apply
the soft-cap reduction to that final amount before adding experience or
processing level gains.

Let `C` be the current soft cap, `E` the Pokémon's experience before the award,
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
award for this calculation. Exp. Candy treats the item's numerical experience
as one award. Rare Candy remains unaffected and grants its normal level
increase. Reaching a level above the cap through Rare Candy does not disable
the later numerical-experience reduction.

An enabled challenge level cap keeps its existing behavior. Apply the
Trainer Rating reduction first, then let the challenge rule further restrict
or cancel the award. The stricter result wins.

### Obedience

Resolve obedience against the current soft cap whenever a player-controlled
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
obedience threshold from regional badge flags, and a regional eighth badge
does not bypass the Trainer Rating cap. Other product builds retain their
existing obedience behavior until they explicitly adopt this specification.

### Missing-badge catch penalty

The configured missing-badge catch penalty continues to use its existing badge
count, level thresholds, and cumulative catch-rate reduction. Wild encounter
projection and the soft cap do not alter its calculation. A wild Pokémon may
therefore be harder to catch under that rule and may also disobey after capture
when its obedience reference level exceeds the soft cap.

Under the current Generation IX rule, the capture runtime counts its existing
eight badge flags and clamps the result to eight. Starting at the current badge
count, each threshold below the wild Pokémon's level multiplies the catch odds
by `4 / 5` using integer arithmetic. The thresholds remain 25, 30, 35, 40, 45,
50, 55, 60, and 100 for indices zero through eight. This feature does not
replace that badge source with global badge count.

### Shared implementation authority

All Wayfarer experience sources call one shared reduction helper so award
ordering and rounding cannot drift. All Wayfarer obedience checks call one
shared soft-cap resolver. The resolver reads Trainer Rating rather than a
regional badge count or the Trainer Rating save variable as though it were
already a Pokémon level.

The existing generic `EXP_CAP_SOFT` behavior is not the authority for this
feature. Its progressively smaller divisors do not implement the flat one-half
rule. Wayfarer may reuse its call sites only after they route through the rules
defined here.

The cap is derived from existing saved Trainer Rating, and obedience uses
existing Pokémon ownership, met-level, and current-level data. The feature does
not require a separately saved cap or new per-Pokémon state.

### Validation

Deterministic tests must cover:

1. Every integer Trainer Rating from 0 through 80, exact anchor values,
   round-half-up interpolation, range clamping, and monotonic caps.
2. The eight global circuit milestone results, including level 15 at Rating 0
   and level 100 at Rating 80.
3. Numerical awards wholly below the cap, ending exactly at it, crossing it,
   and beginning at or above it.
4. Even, odd, one-point, and zero reduced portions, including the positive
   one-point minimum.
5. Battle participation, catch experience, Exp. Share, Exp. Candy, and Day Care
   through the shared reduction rules.
6. Trade, held-item, global experience, and challenge-option multiplier
   ordering, followed by any stricter challenge level cap.
7. Normal Rare Candy use below, at, and above the soft cap.
8. Same-OT Pokémon met below, exactly at, and above the cap, including a current
   level above the cap in every case.
9. Foreign-OT Pokémon below, exactly at, and above the cap, including a foreign
   Pokémon that grows while the cap is unchanged.
10. Eggs, each retained battle-format exemption, existing species-specific
    rules, and the removal of regional badge and eighth-badge authority in
    Wayfarer.
11. Immediate obedience changes when Trainer Rating raises the cap, plus save
    and load with no separately persisted cap.
12. Unchanged missing-badge catch odds across representative badge counts and
    wild levels, including a capture that is also above the soft cap.
13. Unchanged experience and obedience behavior in builds that have not adopted
    the Wayfarer party progression rules.

Compile the affected battle, party, item, Day Care, capture, and Trainer Rating
objects for every supported product build. Build at least one complete Wayfarer
ROM and exercise the Rating 0, League-clear, and Rating 80 boundaries.

## References

- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Level-cap configuration](../../game/include/config/caps.h)
- [Level-cap runtime](../../game/src/caps.c)
- [Experience awards](../../game/src/battle_script_commands.c)
- [Obedience runtime](../../game/src/battle_util.c)
