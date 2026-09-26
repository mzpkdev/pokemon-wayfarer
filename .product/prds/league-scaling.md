# League scaling

Status: Scaling engine and fixed circuit wiring implemented; campaign balance acceptance pending.
Implemented: Partial

Proposed successor: [Seeded Trainer Circuit](seeded-trainer-circuit.md) uses
each selected NPC's own TR for strength instead of player-entry TR.
Its recurring championships share contender/elite/headliner TR bands rather
than position-based difficulty, and rotate participants with soft history weights.
Trainer-owned baselines and personal growth from badges and first league clears
are the accepted direction; the [balance explorer](../../devtools/ui/README.md#trainer-balance-explorer)
exercises provisional values. Growth and circuit snapshots still need a coordinated
specification revision under reopened D1. This remains a
proposed successor; the scaling contract below remains the
implementation baseline until the successor is adopted.

The fixed Indigo/Masters/Hoenn roster and venue wiring, stage/replay identity,
entry-TR snapshot, and +8-per-venue progression are implemented. See the
[current circuit contract](../specs/wayfarer-interregional-league-circuit.md)
and [runtime producer](../../game/src/league_circuit.c).
The [original scaling evidence](../research/league-scaling-implementation.md)
records earlier automated results and the remaining gameplay validation; its
old progression/wiring description predates the current circuit.

## Intent

Keep the Indigo League, Sevii Masters Challenge, and Hoenn League appropriate to a player's
career whether they enter as soon as eligible or collect all twenty-four badges
first. Preserve the existing opponents and teams, with a gradual level climb
from the first opponent to the final opponent.

## Design

Scale opponent levels from Trainer Rating (TR), captured when the player starts
a circuit run. Keep that rating and stage identity for the entire run, including
save/load and battle reconstruction. A fresh attempt after a loss or departure
uses the player's current TR. Training during a run never raises its difficulty.

Each circuit stage retains its approved roster, team size, source order, species and
forms, moves, held items, abilities, IVs, EVs, natures, trainer healing items,
and AI. Indigo and Hoenn retain five-member preliminary teams and six-member
final teams; Masters retains six-member teams throughout. No automatic
evolution, roster expansion, or move replacement is part of this change.
Preserve existing challenge-mode precedence.

Use a separately tunable League baseline curve, initially seeded with the
player soft-cap anchors. It must not change implicitly when the player cap or
Gym Leader curve changes.

| TR | League baseline |
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

Interpolate between anchors, rounding halves upward. Each opponent has one
explicit ace; the specification identifies its existing source slot.

| Opponent | Ace offset from baseline |
| --- | ---: |
| Opponent 1 | -4 |
| Opponent 2 | -3 |
| Opponent 3 | -2 |
| Opponent 4 | -1 |
| Final opponent | +1 |

Other members have authored offsets of minus one or minus two from their ace.
Clamp final levels to 1 through 100. Do not use party levels, party size, badge
origin, or original absolute opponent levels as runtime scaling inputs.
Existing battle AI still chooses switches and replacements; the ace designation
does not force it to appear last.

## Scaling examples

These examples use an input TR directly, independent of how the player earned it.

| TR at admission | Baseline | First opponent ace | Final opponent ace |
| ---: | ---: | ---: | ---: |
| 40 | 42 | 38 | 43 |
| 56 | 62 | 58 | 63 |
| 64 | 78 | 74 | 79 |
| 72 | 89 | 85 | 90 |
| 80 | 100 | 96 | 100 |

Trainer Rating advancement belongs to the existing
[interregional League circuit](wayfarer-interregional-league-circuit.md) and
[player progression](../specs/trainer-rating-party-progression.md) documents.
This feature consumes the current TR without changing badge contributions,
circuit rewards, the player soft cap, XP reduction, or obedience. Progression
changes remain outside scaling; the current producer already awards +8 for each
of the three canonical first clears.

## Boundaries and presentation

Keep the fixed Indigo to Masters to Hoenn sequence, 8/16/24 badge admission
minimums, prerequisite clears, unrestricted badge collection, travel, and
healing rules. Indigo uses the FRLG rooms and roster, Masters uses the HNS rooms
and roster from Seven Island, and Hoenn retains its Emerald challenge. Indigo
and Hoenn keep their Hall of Fame behavior; the Masters Gallery is not a Hall of
Fame. No new menu, TR popup, difficulty choice, or automatic circuit
announcement is required.

Initial Gym battles remain governed by the Gym Leader design. Ordinary
Trainers, Gym members, story bosses, facilities, and rematches retain their
existing scaling policies. This design supersedes static levels for the fifteen
circuit opponents only; it preserves their authored non-level content.
First-clear and replay runs use the same scaling policy and roster. Replays
take a fresh admission snapshot and do not repeat circuit progression.

## Acceptance and balance

Verify both earliest-entry and all-badges-first routes, plus intermediate
entry timings. Test full runs, loss/retry, save/load, replay, the Seven Island
Masters transition, and return to regional travel. Preserve the clear/reward
handoff for the correct stage and never change a run's opponents retroactively.

Playtest attrition over five battles, strong authored moves at Indigo's earliest
levels, and the training needed after each clear. Compare remaining Gyms before
and after a clear. Equal TR does not guarantee equal difficulty across authored
teams. Record roster outliers for a separate balance review; do not silently
replace species, moves, items, or AI in this implementation.

## References

- [League scaling specification](../specs/league-scaling.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Player party progression](../specs/trainer-rating-party-progression.md)
