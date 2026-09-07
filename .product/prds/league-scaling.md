# League scaling

Status: Implementation and automated validation complete; campaign balance acceptance pending.
Implemented: Partial

See [implementation evidence](../research/league-scaling-implementation.md) for
test results and the remaining gameplay validation. TR progression revisions
remain separate pending work.

## Intent

Keep the Kanto, Johto, and Hoenn League challenges appropriate to a player's
career whether they enter as soon as eligible or collect all twenty-four badges
first. Preserve the existing opponents and teams, with a gradual level climb
from the first Elite Four member to the Champion.

## Design

Scale opponent levels from Trainer Rating (TR), captured when the player starts
a League run. Keep that rating and League identity for the entire run, including
save/load and battle reconstruction. A fresh attempt after a loss or departure
uses the player's current TR. Training during a run never raises its difficulty.

Each League retains its existing roster, team size, source order, species and
forms, moves, held items, abilities, IVs, EVs, natures, trainer healing items,
and AI. Kanto and Hoenn retain five-member Elite Four teams and six-member
Champion teams; Johto retains six-member teams throughout. No automatic
evolution, roster expansion, move replacement, or additional rematch access
is part of this change. Preserve existing challenge-mode precedence.

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
| Elite Four 1 | -4 |
| Elite Four 2 | -3 |
| Elite Four 3 | -2 |
| Elite Four 4 | -1 |
| Champion | +1 |

Other members have authored offsets of minus one or minus two from their ace.
Clamp final levels to 1 through 100. Do not use party levels, party size, badge
origin, or original absolute opponent levels as runtime scaling inputs.
Existing battle AI still chooses switches and replacements; the ace designation
does not force it to appear last.

## Scaling examples

These examples use an input TR directly, independent of how the player earned it.

| TR at admission | Baseline | First E4 ace | Champion ace |
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
League rewards, the player soft cap, XP reduction, or obedience. Progression
revisions can be implemented separately; they are not prerequisites for scaling.

## Boundaries and presentation

Keep the fixed Kanto to Johto to Hoenn sequence, 8/16/24 badge admission
minimums, prerequisite clears, unrestricted badge collection, travel, healing
rules, and Hall of Fame flow. The shared Indigo venue still hosts the existing
Tier 1 and Tier 2 opponent identities. No new menu, TR popup, difficulty choice,
or automatic circuit announcement is required.

Initial Gym battles remain governed by the Gym Leader design. Ordinary
Trainers, Gym members, story bosses, facilities, and rematches retain their
existing scaling policies. This design supersedes static levels for the fifteen
circuit opponents only; it preserves their authored non-level content.

## Acceptance and balance

Verify both earliest-entry and all-badges-first routes, plus intermediate
entry timings. Test full runs, loss/retry, save/load, consecutive Indigo tiers,
and return to regional travel. Preserve the existing clear/reward handoff for
the correct League and never change a run's opponents retroactively.

Playtest attrition over five battles, strong authored moves at Kanto's earliest
levels, and the training needed after each clear. Compare remaining Gyms before
and after a clear. Equal TR does not guarantee equal difficulty across authored
teams. Record roster outliers for a separate balance review; do not silently
replace species, moves, items, or AI in this implementation.

## References

- [League scaling specification](../specs/league-scaling.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Player party progression](../specs/trainer-rating-party-progression.md)
