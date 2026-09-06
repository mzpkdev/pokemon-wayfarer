# Ordinary Trainer and Gym-member scaling

## Intent

Let players explore Wayfarer's regions in different orders while ordinary
Trainer battles remain useful and reasonably approachable. Preserve some
difference between easy routes and dangerous places without requiring authors
to create a progression ladder for every Trainer.

## Design

Ordinary Trainers and Gym members scale from the player's permanent Trainer
Rating when battle begins. Their authored levels contribute a small adjustment
to a shared baseline. A late-area Trainer remains somewhat stronger than an
early-route Trainer at the same Rating, but the original campaign's large
level gaps become much smaller.

Player party levels, party composition, starting region, and recent wins or
losses do not affect this calculation. Training and preparation can give the
player a lasting advantage.

The system retains each selected authored roster's size, order, and species
families. It lowers evolved species through supported numeric evolution chains
when the scaled level cannot support their stage. It does not automatically
evolve an authored base species at higher Ratings. A late-game Youngster can
therefore still have a high-level Rattata.

Ordinary moves are generated from the resulting species and level. Explicit
custom moves receive an automated audit and a reviewed policy before release.
The default for covered Trainers is to use level-up moves; bespoke tactics may
be retained through a small, explicit exception list.

Gym members use the ordinary curve with a two-level bonus. Their existing
type-themed rosters supply their identity. Gym membership does not increase
party size, improve IVs or EVs, or introduce stronger AI automatically.

## Initial balance

The Trainer baseline is independently authored. Its initial anchors are:

| Trainer Rating | Baseline level |
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

Retain 20 percent of each Pokémon's authored level difference from level 5,
rounded to the nearest integer, with a maximum adjustment of eight levels.
The technical specification defines interpolation and rounding exactly.
These are initial tuning values, not the illustrative numbers used during
design discussion. Tune them against actual Trainer inventories and playtests.

| Authored level | Rating 0, ordinary | Rating 0, Gym member | Rating 40, ordinary | Rating 80, ordinary |
| ---: | ---: | ---: | ---: | ---: |
| 5 | 7 | 9 | 34 | 92 |
| 20 | 10 | 12 | 37 | 95 |
| 40 | 14 | 16 | 41 | 99 |
| 60 | 15 | 17 | 42 | 100 |

Final levels cannot exceed 100. The party soft cap is not a ceiling on enemy
levels. Some early Gym members can exceed the player's cap; party size,
species, moves, and held items must be considered when testing those fights.

## Coverage and exclusions

Cover ordinary opposing Trainers throughout the content compiled into
Wayfarer, including Gym members, ordinary villain grunts, and ordinary
rematches. Select the existing rematch roster first, then scale it normally.
Defeated Trainers remain defeated under existing rules; this feature adds no
rematch availability or repeatable farming system.

Gym Leaders, rivals, villain bosses and admins, Elite Four members, Champions,
other authored story bosses, tutorials, and battle facilities remain outside
the automatic system. Player-controlled rental parties, battle partners,
link battles, recorded battles, and imported or externally supplied parties
also remain outside it. Boss variants and rematches retain their exclusion.

Classification must be inspectable and validated. Names and Trainer classes
alone are insufficient to distinguish ordinary Trainers from story bosses.
An ordinary grunt remains eligible even when a script starts its battle.

## Authoring and interactions

Runtime scaling handles progression. Authors keep the existing roster source
and review a generated report of classification, custom moves, and problematic
species or item combinations. AI-assisted batch edits may propose corrections,
but the accepted source records and validation remain deterministic and
reviewable. No AI model runs in the game or is required to build it.

Keep existing items, AI, IVs, EVs, rewards, and encounter triggers unless a
reviewed exception identifies a concrete incompatibility. Changes in battle
levels naturally affect battle experience; prize-money formulas remain under
their existing rules. The audit must identify any difference between scaled
experience rewards and authored-level money rewards.

Trainer randomization retains its existing species mapping and receives the
original authored inputs. Levels still scale for eligible Trainers; automatic
predecessor resolution is bypassed while Trainer species randomization is
active. Moves must be valid for the resulting randomized species and level.

This design supersedes the interregional League circuit's static-party rule
only for ordinary Trainers and Gym members. Gym Leaders and League parties
remain separately authored. Trainer Rating advancement belongs to the circuit;
this feature can be implemented and tested with seeded Ratings before that
producer exists. It introduces no substitute local-badge progression.

## Acceptance

Every covered Trainer has a valid, usable party at every Rating from 0 through
80. Levels never decrease as Rating rises. The same source party, battle
context, Rating, and existing random inputs produce the same result.

Validation must include party pools, aliases, rematches, two-opponent battles,
custom moves, species reversal, randomizer mode, and excluded boss variants.
No eligible Pokémon may receive an ability that belongs only to its authored
evolution. Generated moves must give it at least one usable move.

Playtest early, middle, and late career stages in each included region. Include
weak and strong ordinary Trainers, Gym members, utility-heavy learnsets, and
the largest eligible parties. Confirm that early exploration is viable,
training provides an advantage, and repeated battles do not become lengthy
solely because every Trainer gains levels.

The first implementation must deliver an inventory and balance report as well
as runtime code. Passing formulas alone does not establish playable balance.

## References

- [Technical specification](../specs/trainer-party-scaling.md)
- [Trainer Rating and party progression](trainer-rating-wild-encounter-scaling.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
