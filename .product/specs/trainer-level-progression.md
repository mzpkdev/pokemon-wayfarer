# Trainer Level progression

PRD: [Trainer Level progression](../prds/trainer-level-progression.md)
Implemented: No

## Scope

This specification defines Wayfarer's Trainer Experience and Trainer Level
lifecycle, accomplishment rewards, ordinary wild projection input, Pokemon
battle-Experience attenuation, direct-growth exceptions, persistence,
presentation, and validation.

It defines Trainer Level as the sole progression input available to Wayfarer
Trainer and Gym scaling. A separate specification owns Trainer party level
projection, roster tiers, moves, held items, AI, and battle formats. This
specification does not enroll battle facilities, link content, fixed Pokemon,
or other excluded wild sources in dynamic scaling.

The separate Trainer scaling specification and the complete regional reward
manifest are release prerequisites. Wayfarer may implement and test this
system behind its product gate, but it must not enable Trainer-Level Experience
attenuation in a release build until both prerequisites pass their validation.

Standalone Emerald, FireRed, LeafGreen, and HNS continue to use their existing
Trainer Rating and level-cap behavior. Every rule below is selected only by the
Wayfarer product.

## Behavior

### State and level calculation

Wayfarer stores one unsigned cumulative Trainer Experience value. Its valid
range is 0 through 9,000 inclusive.

Trainer Level and the player-facing Experience remainder are derived as:

```text
TrainerLevel = min(10 + floor(TrainerExperience / 100), 100)
ExperienceIntoLevel = TrainerExperience mod 100
```

At Trainer Level 100, `ExperienceIntoLevel` is treated as zero and the
interface displays MAX. Any attempted award beyond 9,000 is consumed without
changing saved state.

A new game initializes Trainer Experience to zero and Trainer Level to 10.
Trainer Experience never decreases during normal play. No party inspection or
regional switch can change it.

### Reward classes

The Wayfarer Trainer Experience manifest contains stable records with:

- a stable milestone identifier;
- one monotonic completion flag or equivalent permanent completion fact;
- one reward class;
- its source region; and
- a player-facing reward name;
- its prerequisite expression over durable completion facts; and
- the script owner that commits its completion fact.

The reward classes are:

| Reward class | Trainer Experience |
| --- | ---: |
| Gym Badge | 300 |
| Regional Champion | 600 |
| Major story | 100 |
| Substantial quest | 50 |

The initial manifest includes all eight Johto badges, all eight Kanto badges,
all eight Hoenn badges, and the first Johto, Kanto, and Hoenn Champion clears.
Each badge and Champion fact is independent even when source games reuse a
numeric flag or variable.

Every registered story or quest record must use a permanent completion fact
that cannot become false during ordinary story cleanup. A transient story
stage, visited-location state, Trainer defeat bit, item possession check, or
repeatable result is not a valid completion fact.

The complete shipped manifest must satisfy all of these conditions:

1. Its badge and Champion records provide exactly 9,000 Trainer Experience.
2. Registered story and quest records provide alternate progress beyond that
   amount, totaling at least 1,800 Trainer Experience with at least 600 from
   each region whose prerequisite closure excludes that region's Champion
   fact, so level 100 does not require every eligible accomplishment or every
   regional League clear.
3. Gym Badge rewards alone contribute more Experience than all registered
   story and quest rewards combined.
4. No repeatable battle, rematch, catch, item pickup, location visit, travel
   action, daily event, facility result, or multiplayer action is registered.

Each regional content specification owns the exact records and source facts
for its story and quest entries. Before release, those records must be
materialized in the checked-in generated-data input consumed by the Wayfarer
build. Placeholder milestones, unresolved fact names, and records without a
named script owner fail acceptance. Prerequisites use normalized all-of and
any-of expressions over durable facts, contain no cycles, and must describe
the actual script gates for reaching the accomplishment.

### Award ordering and idempotence

The derived Trainer Experience total is the sum of every completed manifest
record, clamped to 9,000. Reading progression reconciles saved state as:

```text
TrainerExperience = max(SavedTrainerExperience, DerivedTrainerExperience)
```

The result is clamped to 9,000 before it is used or stored. This high-water
rule recovers an interrupted award after its completion fact was committed and
prevents a later cleanup or data correction from lowering Trainer Level.
Manifest totals are accumulated in an unsigned type at least 32 bits wide
before the result is clamped.

An accomplishment becomes eligible only when its permanent completion fact is
committed. The award path then:

1. Reads and reconciles the previous saved Trainer Experience.
2. Commits the accomplishment's permanent completion fact.
3. Recalculates derived Trainer Experience from all manifest facts.
4. Stores the greater reconciled total, clamped to 9,000.
5. Reports the accepted increase and the final Trainer Level before returning
   control.

Re-entering the script, reloading after the committed save, winning a rematch,
or reaching the same fact through another script produces no additional award.
A loss, draw, flee, interruption before the permanent fact commit, or failed
fact commit produces no Trainer Experience. An interruption after the fact
commit is recovered by reconciliation and grants the accepted increase exactly
once, even if its presentation was interrupted.

A static audit must prove one-to-one ownership between manifest records and
durable facts. The stored high-water total plus full derivation from those facts
must recover a missing award without duplicating an accounted one.

Pokemon Experience from the battle that enables a badge or Champion reward
uses the Trainer Level that was active when the battle began. Trainer
Experience is committed after the victory and progression fact, so the higher
Trainer Level applies only to later Pokemon Experience and encounters.

### Trainer Level consumer contract

Wayfarer exposes one canonical Trainer Level query. Ordinary wild scaling and
every future ordinary Trainer, rival, team boss, Gym Leader, Elite Four,
Champion, and eligible rematch scaling implementation must read that query.

No world-scaling consumer may derive its progression baseline from the current
party, party average, highest party member, boxed Pokemon, Pokedex completion,
money, play time, or battle count. A consumer samples Trainer Level when it
creates an encounter and does not change the encounter after creation.

Battle facilities, link battles, secret-base parties, scripted rental teams,
and formats with their own level-normalization rules do not use Trainer Level.

### Ordinary wild projection

Wayfarer retains the current ordinary wild projection as a function of a
projection coordinate from 10 through 80. The coordinate is transient and is
not saved, displayed, awarded, or called Trainer Rating.

For Trainer Level `L`, calculate it with round-half-up integer arithmetic:

```text
ProjectionCoordinate = 10 + floor((((L - 10) * 70) + 45) / 90)
```

This maps Trainer Level 10 to coordinate 10 and Trainer Level 100 to coordinate
80. Every intermediate result is monotonic and remains within 10 through 80.
Every integer coordinate in that range is reached. Because 91 Trainer Levels
map onto 71 coordinates, some adjacent Trainer Levels deliberately produce the
same wild projection.

After calculating the coordinate, ordinary wild processing is identical to
the current Trainer Rating implementation. It retains:

- the existing level anchors and authored-level retention curve;
- cumulative high-water projection so a higher Trainer Level cannot lower an
  outcome;
- profile level offsets and the valid Pokemon-level clamp;
- predecessor resolution for nonrandomized ordinary encounters;
- global species minimum levels and ineligible-weight renormalization;
- authored slot weights, encounter methods, rod partitions, time variants,
  ability effects, lure behavior, and Altering Cave behavior; and
- randomized-species ownership with projected levels but without predecessor
  or species-floor filtering.

The projection data must retain points 0 through 80 even though Wayfarer never
supplies a coordinate below 10. The current algorithm uses point 0 as the
authored-level baseline and calculates a cumulative high-water result through
the selected coordinate. An exactly equivalent precomputed seed at coordinate
10 is acceptable, but rebasing or truncating the curve at coordinate 10 is not.

The ordinary consumers remain regular land, water, Rock Smash, and fishing
encounters; ordinary land and water DexNav populations; Pokedex area checks;
Match Call and radio selection; and local ambient species selection.

Hidden DexNav encounters, fixed and scripted encounters, roamers, outbreaks,
Feebas, Battle Pike, and Battle Pyramid populations remain excluded. Gift and
legendary Pokemon remain excluded unless another specification enrolls them.

Generated Wayfarer data and developer tools expose Trainer Level 10 through
100. They may retain the projection coordinate as implementation data, but
must not expose it as a second progression stat.

### Pokemon battle Experience

Calculate each participating or Exp. Share Pokemon's normal final Experience
award first, including all existing participation, sharing, trade, item,
affection, level-difference, and global Experience modifiers. Apply Trainer
Level attenuation to that per-Pokemon result afterward.

The multiplier is selected from the Pokemon's current effective level for each
portion of the award:

| Pokemon level | Multiplier |
| --- | ---: |
| Less than Trainer Level | 1 / 1 |
| Equal to Trainer Level | 1 / 4 |
| Trainer Level plus 1 | 1 / 10 |
| Trainer Level plus 2 or more | 1 / 20 |

If an award crosses one or more level boundaries, consume the unmodified award
piecewise using a simulated level and Experience total:

```text
remainingRaw = NormalFinalExperienceAward
granted = 0

while remainingRaw > 0 and simulatedLevel < 100:
    D = denominatorFor(simulatedLevel, TrainerLevel)
    expToNextLevel = experienceNeededForNextLevel(simulatedPokemon)
    rawCost = expToNextLevel * D

    if remainingRaw >= rawCost:
        granted += expToNextLevel
        remainingRaw -= rawCost
        advance simulatedPokemon to the next level
    else:
        granted += floor(remainingRaw / D)
        remainingRaw = 0
```

`rawCost` and the other intermediate values use an unsigned type wide enough
to prevent overflow. An award that reaches level 100 discards all remaining
raw Experience. The final incomplete portion discards its division remainder.
A single large award therefore cannot receive the starting level's rate after
crossing into a reduced band.

Each rational result rounds down. If the complete normal award is positive and
attenuation would produce zero total Experience, award one Experience point.
This deliberate minimum can exceed the table's nominal percentage and can
cross a level boundary when the Pokemon needs exactly one Experience point.
After that one point, no remainder continues into the next band. No Pokemon at
level 100 receives Experience.

The calculation is independent for each party member. Pokemon below Trainer
Level receive their normal award even when another party member is over the
limit. Party rotation and catch-up are intended behavior.

### Direct growth and received Pokemon

Rare Candies and Experience Candies bypass battle-Experience attenuation.
Calculate the item's normal result before consuming it. Show a confirmation
when that result has a numeric level above Trainer Level, including any
effective Candy use on a Pokemon that is already above Trainer Level. Landing
exactly at Trainer Level does not warn. A use that has no effect, including a
level-100 use, follows its normal rejection path without a confirmation.
Cancel leaves the item and Pokemon unchanged. Confirm applies the item's
normal effect, including the level-100 clamp.

Daycare Pokemon gain Experience normally below Trainer Level and stop at the
exact Experience threshold for Trainer Level. Wayfarer stores a separate
eligible-growth counter for each occupied daycare slot: the two breeding
daycare slots and the Route 5 single daycare slot. These three counters live in
global Wayfarer saved state rather than enlarging `SaveBlock1`.

The existing `DaycareMon.steps` counters continue to advance so breeding and
egg production retain their timing. On each step, the corresponding Wayfarer
eligible-growth counter advances only until the deposited Pokemon's original
Experience plus that counter reaches the current Trainer Level threshold.
Surplus growth is discarded as it occurs and cannot be banked for a later
Trainer Level increase. Raising Trainer Level allows growth from new steps to
resume without changing the breeding clock.

Deposit resets the matching eligible-growth counter. Slot compaction moves it
with its Pokemon, and withdrawal or removal clears it. Daycare level previews,
withdrawal Experience, learned moves, and the level-based withdrawal fee use
eligible growth rather than `DaycareMon.steps`. Waiting at the threshold does
not increase the fee.

Caught, hatched, gifted, scripted, and traded Pokemon retain their assigned or
received level and Experience. The system never lowers them. Future battle
Experience uses the multiplier for their individual level.

Scripted level assignment, battle-facility normalization, evolution, move
learning, EVs, friendship, and stat calculation remain unchanged.

### Challenge settings

Wayfarer does not present the HNS OFF, NORMAL, or HARD level-cap choice. The
saved HNS challenge bitfield does not affect Trainer Level, battle Experience,
Rare Candy, Experience Candy, daycare, or encounter scaling in Wayfarer.

Standalone HNS retains its existing challenge menu, badge-derived cap tables,
hard enforcement, and compile-time cap configuration.

### Presentation

The Wayfarer Trainer Card displays:

- `TRAINER Lv.` followed by Trainer Level;
- `TRAINER EXP` followed by `ExperienceIntoLevel / 100`; and
- `TRAINING LIMIT` followed by Trainer Level.

At Trainer Level 100, the Experience field displays `MAX`.

After an eligible completion, show the reward name and the accepted Experience
increase after the 9,000-point clamp. For example, a nominal 100-point reward
accepted at 8,950 reports 50. If one or more levels are crossed, show the final
Trainer Level. A single reward does not require one message per intermediate
level. If the player is already at 9,000, show `TRAINER EXP MAX` instead of a
zero-point award.

The first battle in which any Pokemon receives attenuated Experience sets a
permanent tutorial flag after explaining that Pokemon at or above Trainer
Level gain less battle Experience. Subsequent battles do not repeat it. The
ordinary Experience message still reports the actual awarded amount.

### Persistence and product isolation

Trainer Experience and the reduced-Experience tutorial flag are global
Wayfarer fields. They do not belong to the Hoenn persistent bank and are not
reset by regional initialization, Hall of Fame cleanup, whiteout, travel,
credits, daily reset, or starting another regional story.

The Wayfarer save initialization path clears Trainer Experience and the
tutorial flag for a new game. Save and load round-trip both fields and reject
or clamp an out-of-range Trainer Experience value to 9,000.

Wayfarer is prerelease software, so this change requires no migration from the
old Trainer Rating variable or an earlier Wayfarer save layout. The obsolete
Wayfarer Trainer Rating state is not consulted. Standalone products retain
their current save fields and migration behavior.

### Validation

Static validation must prove:

- every manifest identifier and permanent completion fact is unique;
- all 24 regional badges and all three first Champion clears are registered;
- badge and Champion rewards total exactly 9,000 Trainer Experience;
- Gym Badge rewards exceed the combined registered story and quest rewards;
- registered story and quest rewards total at least 1,800, including at least
  600 from each region whose prerequisite closure excludes that region's
  Champion fact, with no placeholder record or unresolved script owner;
- manifest prerequisite expressions are acyclic and match their owning
  scripts, and at least one prerequisite-closed set of records reaches 9,000
  Trainer Experience with no more than two Champion facts;
- no transient or repeatable fact is registered;
- every Trainer Level from 10 through 100 maps to a coordinate from 10 through
  80, endpoints are exact, every coordinate is reached, the mapping never
  decreases, and its expected adjacent-level plateaus are retained;
- the ordinary wild projection retains points 0 through 80 or an exactly
  equivalent coordinate-10 seed;
- Wayfarer wild outcomes equal current Trainer Rating outcomes at the mapped
  coordinate for every authored level, configured offset, and eligible
  profile;
- a higher Trainer Level never lowers a projected outcome; and
- the Trainer Experience fields fit the bounded Wayfarer save layout.

Runtime and integration tests must cover:

- new-game initialization and every reward class;
- rewards that cross zero, one, and several Trainer Levels;
- level-100 saturation and ignored excess rewards;
- victory, loss, draw, flee, repeat interaction, rematch, save and reload, and
  two scripts sharing one completion fact;
- travel among Johto, Kanto, and Hoenn before and after rewards;
- one complete prerequisite-valid progression route that reaches Trainer Level
  100 while leaving at least one regional Champion fact unset;
- ordinary wild populations at Trainer Levels 10 through 100, including every
  current consumer and excluded source;
- one battle award wholly below the limit, at each reduced band, and crossing
  every boundary with Exp. Share, trade, item, and other modifiers;
- separate party members below, at, and above Trainer Level in one award;
- Rare Candy and each Experience Candy confirmation, cancellation, and bypass;
- daycare stopping and resuming at the exact threshold;
- discarded daycare steps remaining discarded after Trainer Level rises;
- breeding and egg-production timing continuing while daycare growth is
  stopped, plus correct eligible-growth reset, slot movement, preview, fee,
  withdrawal, and save round-trip behavior;
- the one-Experience minimum at all three reduced bands, including a
  one-point level crossing;
- caught, hatched, gifted, scripted, and traded over-level Pokemon; and
- absence of the HNS level-cap choice and challenge-bit influence in Wayfarer,
  with standalone HNS regression coverage.

Release validation must also exercise every required Trainer and boss path
defined by the companion Trainer scaling specification. No supported route may
require grinding through an attenuated band to meet that specification's
expected battle level.

Generated balance audits must identify every ordinary wild profile whose
possible projected level exceeds Trainer Level. These records are review data,
not automatic failures: exact preservation of the current curve allows
high-authored areas to remain dangerous. Playtesting must confirm that the
resulting regional and area progression is understandable and that no profile
exceeds its mathematically equivalent current Trainer Rating outcome.

## References

- [Trainer Rating wild encounter scaling specification](trainer-rating-wild-encounter-scaling.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Standard Rod fishing](standard-rod-fishing.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
