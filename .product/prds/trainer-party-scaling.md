# Regular trainer and Gym member scaling

Implemented: Partial; runtime policies exist, campaign balance acceptance remains pending.

Today's routing includes [League scaling](league-scaling.md) with the player's
TR saved when entering a league, so League levels scale from that saved TR. See
[the level resolver](../../game/src/trainer_party_scaling.c) and
[the circuit producer](../../game/src/league_circuit.c). The v0 TR design (a
v0 badge scale, uncapped TR, scalers, and notable trainers' separate TR) is in
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

On the v0 TR scale, regular trainers take their strength from the place they
stand, the same way [wild Pokémon](wild-encounters-v2.md#reach) do. Danger
reads the same for trainers and wild Pokémon: roads are easy, remote places
are hard.

- **Reach level.** A regular trainer fights at their map's reach level, using
  the same Road, Wilds, Outlands and dungeon floor levels as
  [wild level scaling](../specs/wild-level-scaling.md#reach-levels), plus a
  small trainer bonus (placeholder: 3 levels). A map with no reach of its own,
  such as a building, resolves one with the
  [places without wild encounters](../specs/reach-assignments.md#places-without-wild-encounters)
  rules.
- **What that means.** Road trainers start close to your level and fall behind
  as you earn badges, so routes stop being a threat and the late challenge
  comes from [notable trainers](notable-trainers.md). Wilds trainers stay a
  step up, Outlands trainers meet you near your level cap all game, and
  dungeon trainers follow their floor.
- **Authored levels** no longer nudge a Pokémon's level. The place replaces
  that role.
- **Gym members** stand inside Gyms, which have no place on the danger map.
  They keep the v0 regular level curve with their two-level bonus. The exact
  anchors are in the
  [technical specification](../specs/trainer-party-scaling.md#gym-members).
- **Wandering trainers** under [daily world slots](daily-world-slots.md) take
  the level of the spot they stand in that day.

#### Final-stage rosters

Regular trainers' Pokémon evolve as their level rises. Every covered regular
roster, rematch teams included, lists each Pokémon at the final stage of its
line. A branching line, such as Poliwag's, names one final form per trainer.
The step-back rule then picks the stage the trainer's level supports: level
evolutions step back at their evolution level, and trade, stone and friendship
evolutions each step back through one shared authored level
([evolution stages](../specs/player-trainer-rating.md#evolution-stages)).

A Youngster's Rattata on an early road becomes a Raticate on a late one. This
replaces today's rule that base species never evolve.

A short reviewed list keeps an earlier stage as the highest stage for trainers
whose identity is that stage, such as a Tuber's baby Pokémon. Their Pokémon
step back as usual but never go past that stage.

## Coverage and exclusions

Cover regular opposing trainers throughout the content compiled into Wayfarer,
including Gym members, regular villain grunts, and regular trainer rematches.
Select the existing rematch roster first, then scale it normally. Defeated
Trainers remain defeated under existing rules; this feature adds no rematch
availability or repeatable farming system. [Daily world
slots](daily-world-slots.md) separately lets regular trainer spots rotate
daily, and its trainers scale under this design.

Gym Leaders remain outside this automatic system. Their enrolled initial badge
battles follow the separate
[Gym battle design](notable-trainers.md#gym-battles); the regular trainer scaler
continues to exclude them. Leader rematches keep their authored, static
parties Today; on adoption they follow the notable model through the
[every-battle rule](../specs/notable-trainers.md#trainer-rating). Rivals, villain bosses and admins, Elite Four members, Champions, other
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
by the separate [Gym battle design](notable-trainers.md#gym-battles), and
leader rematches stay static until they follow that design on adoption. League rosters stay authored and their levels
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

## References

- [Technical specification](../specs/trainer-party-scaling.md)
- [Trainer Rating and party progression](trainer-rating-wild-encounter-scaling.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
