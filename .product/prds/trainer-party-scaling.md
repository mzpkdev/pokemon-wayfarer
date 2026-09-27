# Regular trainer and Gym member scaling

Implemented: Partial; runtime policies exist, campaign balance acceptance remains pending.

Today's routing includes [League scaling](league-scaling.md) with the player's
TR saved when entering a league; League levels are no longer static. See
[the level resolver](../../game/src/trainer_party_scaling.c) and
[the circuit producer](../../game/src/league_circuit.c). The v0 TR design (a
new badge scale, uncapped TR, scalers, and notable trainers' separate TR) is in
[Player Trainer Rating](player-trainer-rating.md); its effect on regular
trainers is under [v0](#v0).

Today's shared six-slot Gym feature is disabled by default in
[configuration](../../game/include/config/trainer_party_scaling.h); it uses
player TR only when enabled. Giovanni's separate five-slot projection and
other existing Gym/double-battle policies retain their own current behavior.
Neither generated roster data nor an experimental catalog proves enrollment
or activation of a Gym encounter.

## Intent

Let players explore Wayfarer's regions in different orders while regular trainer
battles remain useful and reasonably approachable. Preserve some difference
between easy routes and dangerous places without requiring authors to create a
progression ladder for every Trainer.

## Design

Regular trainers and Gym members scale from the player's permanent Trainer
Rating (TR) when battle begins. Their authored levels contribute a small
authored level bonus on top of the shared regular trainer level curve. A
late-area Trainer remains somewhat stronger than an early-route Trainer at the
same TR, but the original campaign's large level gaps become much smaller.

Player party levels, party composition, starting region, and recent wins or
losses do not affect this calculation. Training and preparation can give the
player a lasting advantage.

The system retains each selected authored roster's size, order, and species
families. It lowers evolved species through supported numeric evolution chains
when the scaled level cannot support their stage. It does not automatically
evolve an authored base species at higher TR. A late-game Youngster can
therefore still have a high-level Rattata.

Regular trainers' moves are generated from the resulting species and level.
Explicit custom moves receive an automated audit and a reviewed policy before
release. The default for covered Trainers is to use level-up moves; bespoke
tactics may be retained through a small, explicit exception list.

Gym members use the regular trainer level curve with a two-level bonus. Their
existing type-themed rosters supply their identity. Gym membership does not
increase party size, improve IVs or EVs, or introduce stronger AI automatically.

## Initial balance

The regular trainer level curve is independently authored. Its initial anchors
are:

| TR | Level |
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

The authored level bonus retains 20 percent of each Pokémon's authored level
difference from level 5, rounded to the nearest integer, with a maximum of
eight levels.
The technical specification defines interpolation and rounding exactly.
These are initial tuning values, not the illustrative numbers used during
design discussion. Tune them against actual Trainer inventories and playtests.

| Authored level | TR 0, regular | TR 0, Gym member | TR 40, regular | TR 80, regular |
| ---: | ---: | ---: | ---: | ---: |
| 5 | 7 | 9 | 34 | 92 |
| 20 | 10 | 12 | 37 | 95 |
| 40 | 14 | 16 | 41 | 99 |
| 60 | 15 | 17 | 42 | 100 |

Final levels cannot exceed 100. The player's level cap is not a limit on enemy
levels. Some early Gym members can exceed the player's cap; party size,
species, moves, and held items must be considered when testing those fights.

### v0

On the new TR scale, regular trainers follow a new level curve. With a few
badges they sit right under your level cap, about one level below it at four
badges, so early routes are real fights. Later they fall behind: with all 24
badges they sit about 18 levels under the cap, and routes are no threat. The
late challenge comes from [notable trainers](notable-trainers.md)
instead. Authored levels still nudge each Pokémon, and Gym members still get
their two-level bonus on top. The exact anchors are in the
[technical specification](../specs/trainer-party-scaling.md#v0-regular-trainer-level-curve).

Evolved species also step back through trade, stone, and friendship
evolutions in v0, each through one shared authored level
([evolution stages](../specs/player-trainer-rating.md#evolution-stages));
today only level evolutions step back. Base species still never evolve.

## Coverage and exclusions

Cover regular opposing trainers throughout the content compiled into Wayfarer,
including Gym members, regular villain grunts, and regular trainer rematches.
Select the existing rematch roster first, then scale it normally. Defeated
Trainers remain defeated under existing rules; this feature adds no rematch
availability or repeatable farming system.

Gym Leaders remain outside this automatic system. Their enrolled initial badge
battles follow the separate
[Gym battle design](notable-trainers.md#gym-battles); the regular trainer scaler
continues to exclude them, and leader rematches retain their authored, static
parties. Rivals, villain bosses and admins, Elite Four members, Champions, other
authored story bosses, tutorials, and battle facilities also remain outside the
automatic system. Player-controlled rental parties, battle partners, link
battles, recorded battles, and imported or externally supplied parties also
remain outside it. Boss variants and rematches retain their exclusion.

Classification must be inspectable and validated. Names and Trainer classes
alone are insufficient to distinguish regular trainers from story bosses. A
regular grunt remains eligible even when a script starts its battle.

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

This design supersedes the interregional League circuit's static-party rule only
for regular trainers and Gym members. Initial Gym Leader badge battles are owned
by the separate [Gym battle design](notable-trainers.md#gym-battles), while
leader rematches remain static. League rosters stay authored and their levels
follow the separate [League scaling design](league-scaling.md). Player TR
advancement belongs to the implemented circuit producer. Host checks may still
seed TR independently; this feature introduces no substitute local-badge or
NPC-personal progression.

## Acceptance

Every covered Trainer has a valid, usable party at every TR from 0 through 80.
Levels never decrease as TR rises. The same source party, battle context, TR,
and existing random inputs produce the same result.

Validation must include party pools, aliases, rematches, two-opponent battles,
custom moves, species reversal, randomizer mode, and excluded boss variants.
No eligible Pokémon may receive an ability that belongs only to its authored
evolution. Generated moves must give it at least one usable move.

Playtest early, middle, and late career stages in each included region. Include
weak and strong regular trainers, Gym members, utility-heavy learnsets, and
the largest eligible parties. Confirm that early exploration is viable,
training provides an advantage, and repeated battles do not become lengthy
solely because every Trainer gains levels.

The first implementation must deliver an inventory and balance report as well
as runtime code. Passing formulas alone does not establish playable balance.

## Later

- Forward evolution for regular trainers: late routes currently show
  high-level unevolved species.

## References

- [Technical specification](../specs/trainer-party-scaling.md)
- [Trainer Rating and party progression](trainer-rating-wild-encounter-scaling.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
