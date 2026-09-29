# Trainer AI

PRD: [Trainer AI](../prds/trainer-ai.md)
Implemented: No. Today, every trainer battles with the AI line authored in
its party data; the
[balance explorer](../../devtools/ui/README.md#trainer-balance-explorer)
resolves the flags below as placeholder tooling.
Design status: v0 contract, fitted to the engine's existing AI flags. The
model (base bundle, one play style per trainer, the AI skill scaler, ace
protection, the boss flag) and the play style assignments are approved; the
AI skill tier thresholds and tier names are placeholders tuned by
playtesting. Terms follow the
[glossary](../prds/player-trainer-rating.md#glossary).

## Scope and ownership

This specification is the single owner of the AI flags every
[notable trainer](notable-trainers.md) battles with: the base bundle, the
play styles and their assignments, the AI skill scaler, ace protection, the
boss flag, the flag hazards, runtime resolution and the override point, and
their validation. Consumers link here rather than restating it.

- [Notable trainers](notable-trainers.md) owns the inventory, trainer TR,
  rosters, aces, team resolution, and the battle snapshot this spec reads.
- [Gym Leader scaling](gym-leader-scaling.md) owns battle construction and
  randomizer and challenge option precedence; it takes the resolved flags
  from here.
- [Leagues](leagues.md) owns lineups; a league match is a notable battle and
  resolves its flags here like any other.
- [Player Trainer Rating](player-trainer-rating.md) owns the
  [scaler definition](player-trainer-rating.md#scalers) the AI skill scaler
  uses.

Regular trainers, Gym members, wild Pokémon, facilities, partners, and
link or recorded battles keep their current AI flags; the one exception is a
notable trainer partnering the player at the Sevii Masters, who resolves
here like any notable trainer. Standalone builds are
unchanged.

## Engine model

The engine's AI flags are bits in `game/include/constants/battle_ai.h`. A
trainer's AI line in its party data compiles to `struct Trainer.aiFlags`,
and `BattleAI_SetupFlags()` in `game/src/battle_ai_main.c` reads it once per
battle into `gAiThinkingStruct->aiFlags[battler]`. Every flag only adjusts
move scores; the AI picks the highest-scoring move, breaking ties at random,
so a flag shifts preferences but never scripts a move. This spec names flags
by their display names (Check Bad Move for `AI_FLAG_CHECK_BAD_MOVE`, and so
on).

## Base bundle

Every notable trainer always has the **Basic** bundle: Check Bad Move, Try To
Faint, and Check Viability (the engine's `AI_FLAG_BASIC_TRAINER`). Style
flags are read inside the Basic bundle's scoring, so a style without it does
nothing; no resolution may drop any of the three.

## Play styles

A **play style** is a notable trainer's battle identity: authored per entry,
exactly one per trainer, stored as a lowercase identifier (`field_marshal`
for Field marshal). Its flags are added on top of Basic:

| Play style | Flags on top of Basic | Plays like |
| --- | --- | --- |
| Gambler | Risky | Swings for big damage over accuracy and takes risks. |
| Bomber | Risky, Will Suicide | A gambler who also trades Pokémon away with Explosion-style moves. |
| Sweeper | Force Setup First Turn | Sets up on the first turn, then sweeps. |
| Field marshal | Powerful Status | Controls the field first: weather, hazards, screens, and terrain. |
| Hexer | Prefer Status Moves, HP Aware | Wears you down with status moves, timed by HP. |
| Turtle | Conservative, HP Aware | Plays safe for the long game: expects low rolls and manages HP. |
| Brawler | Try To 2HKO, Prefer Highest Damage Move | Hits as hard as it can to knock you out in two. |
| Tactician | HP Aware | No gimmick: reads both sides' HP and picks the move for the moment. |

Assignments follow each trainer's gimmick and are reviewable content:

| Play style | Trainers (gimmick) |
| --- | --- |
| Gambler | Blaine (all-in fire), Lt. Surge (electric blitz), Flannery (hot-headed fire), Winona (high-risk aerial offence) |
| Bomber | Wattson (Voltorb and Electrode self-destructing) |
| Sweeper | Norman (one big hitter), Lorelei (ice setups), Clair and Drake (dragon dances), Sidney (a dark showman's sweep), Bugsy (Scyther's swords) |
| Field marshal | Brock (rocks and sand), Steven (hazards), Misty (rain), Will (screens), Tate & Liza (psychic field control), Falkner (winds and weather) |
| Hexer | Erika (powders), Morty, Agatha, and Phoebe (ghostly curses), Sabrina (psychic mind games), Juan (graceful status play), Janine and Koga (poison and ninja tricks), Karen (dark tricks) |
| Turtle | Whitney (Miltank's milk-drink stall), Pryce (an old master's patience), Wallace (Milotic's recovery), Jasmine (steel walls), Glacia (ice walls), Roxanne (textbook caution) |
| Brawler | Bruno, Chuck, and Brawly (fighting power), Giovanni (ground brute force) |
| Tactician | Lance (the boss Champion), Blue (the adaptable rival) |

The play style never changes with TR; only the AI skill and ace protection
do.

## AI skill

**AI skill** is a [step scaler](player-trainer-rating.md#scalers) over the
trainer's own TR (never player TR), with anchors `(0,0) (30,1) (70,2)
(110,3)` giving a tier. Each tier adds its flags to every tier below it, on
top of the style (placeholder thresholds and names):

| Trainer TR | Tier | Adds |
| --- | --- | --- |
| 0–29 | 0, None | nothing |
| 30–69 | 1, Aware | Smart Mon Choices, Assume STAB |
| 70–109 | 2, Smart | Smart Switching, Assume Status Moves, Weigh Ability Prediction |
| 110+ | 3, Predictive | Predict Switch, Predict Incoming Mon, Predict Move |

Flat past TR 110. Stronger trainers therefore choose switch-ins better, guess
the player's moves and abilities, and at the top predict switches and moves.

## Ace protection

The engine protects aces by party position: Ace Pokemon keeps the **last**
party member back until it is the last one remaining, and Double Ace Pokemon
the last two. Battle order puts the aces last
([battle order](notable-trainers.md#rosters)), so the party's last slots are
the aces. Ace protection counts the aces in the **resolved team** (not the
roster):

- exactly one ace (always roster slot 1, the signature Pokémon): Ace Pokemon;
- two or more: Double Ace Pokemon.

A resolved team always has at least one ace, so one of the two is always set
(except under trainer-species randomization, which turns ace protection off;
see [Options and double battles](#options-and-double-battles)).
The engine has no three-ace flag: with three aces the earliest-fought ace is
unprotected (a known limit; Later). A trainer whose team grows past a second
ace switches from Ace to Double Ace at that team size (Brock when Aerodactyl
joins).

## Boss flag

The **boss flag** is authored per entry (`bossOmniscient`, default `false`)
and adds Omniscient: full knowledge of the player's moves, abilities, and
held items. v0 sets it for Lance only; Red is the planned next boss once he
is a notable trainer.

## Resolution

For one battle, with `tr` the trainer's TR and `aces` the aces in the
resolved team, both from the
[battle snapshot](notable-trainers.md#battle-snapshot), and
`speciesRandomized` whether trainer-species randomization is on:

```text
flags = Basic
      | styleFlags(playStyle)
      | skillFlags(tier 0..aiSkill(tr))
      | (speciesRandomized ? 0 : aces == 1 ? Ace Pokemon : Double Ace Pokemon)
      | (bossOmniscient ? Omniscient : 0)
      | (double battle ? Double Battle : 0)
      | (Smart Switching ? Smart Mon Choices : 0)
      | (Predict Incoming Mon ? Predict Switch : 0)
```

The last three lines are the engine's own additions in its flag setup (the
automatic double-battle flag and the two auto-includes); the resolver
reproduces them because it writes after that setup. Resolution is a pure
function of play style, trainer TR, aces, the boss flag, the battle type, and
trainer-species randomization: no save seed, RNG, player party, or call history.

Examples (placeholder growth; badges → world progress 0 / 40 / 80 / 120 / 160):

| Trainer | 0 badges | 4 badges | 8 badges | 16 badges | 24 badges |
| --- | --- | --- | --- | --- | --- |
| Brock (Field marshal) | TR 25, None, 1 ace | TR 44, Aware, 1 ace | TR 63, Aware, 1 ace | TR 81, Smart, 2 aces | TR 100, Smart, 2 aces |
| Lance (Tactician, boss) | TR 200, Predictive, 3 aces, Omniscient | same | same | same | same |

## Flag hazards

- **Risky and Conservative** cancel each other (Risky plays for damage and
  accepts risk, Conservative assumes every move low-rolls); never combine
  them. Each style sets at most
  one, and no other layer adds either, so this holds by construction.
- **Stall** is marked unfinished in the engine; never use it.
- **Basic** must stay present (see [base bundle](#base-bundle)).
- **Randomize Party Indices, Randomize Switchin, and Sequence Switching**
  would break or override the authored battle order that places aces last;
  never set them for a notable trainer. Randomize Party Indices is also read
  at party construction from `struct Trainer.aiFlags`, so an enrolled
  encounter's authored AI line must not carry it.
- **Prediction slots.** The engine also copies the opponents' flags into the
  player-side battlers for its prediction logic; the override writes the
  resolved flags there too, so prediction and choice agree.

## Runtime and the override point

Resolve the flags at battle start, alongside the battle snapshot, once the
team (and so its aces) and the trainer's TR are fixed. Write them once per
battle through the engine's per-battle AI flag override point: directly
after `BattleAI_SetupFlags()` sets `gAiThinkingStruct->aiFlags[battler]`
from the trainer's `struct Trainer.aiFlags`, and before the first AI
decision. The debug-battle path in `BattleAI_SetupFlags()` already overrides
the flags this way. Write every opponent battler the notable trainer controls
(both in Tate & Liza's double battle) and the prediction slots above.

The resolved flags belong to the battle: reconstruction within the battle
reuses them, TR gained mid-battle changes nothing, and a retry at the same
world progress resolves the same flags. An invalid play style or resolution
fails preparation like any invalid snapshot; never fall back to the authored
AI line or another trainer's flags. A disabled scaling switch restores the
encounter's authored AI line along with its party
([enablement](gym-leader-scaling.md#overrides-and-enablement)).

## Options and double battles

- **Randomizer and challenge options** keep their existing precedence
  ([overrides](gym-leader-scaling.md#overrides-and-enablement)). They change
  species, moves, items, IVs/EVs, or levels, not AI flags; the resolved flags
  still come from the snapshot's TR and aces, except that trainer-species
  randomization turns ace protection off (Ace Pokémon and Double Ace
  Pokémon): the roster is bypassed and the legacy party keeps its own order,
  so its last slots need not be aces.
- **Double battles** keep the engine's automatic Double Battle flag. Tate &
  Liza share one play style (Field marshal), one TR, and one resolved flag
  set, written for both opponent battlers.
- **Tag matches** at the Sevii Masters
  ([Leagues](leagues.md#tag-matches)) put three notable trainers in one
  battle: two opponents and the player's partner. Each resolves their own
  flags from their own snapshot (their TR, and the aces among the three they
  bring), and the override writes all three: opponent A to battler 1,
  opponent B to battler 3, and the partner to battler 2, where
  `BattleAI_SetupFlags()` puts `gPartnerTrainerId`'s flags. With a partner,
  the engine copies no flags into the player's side for prediction, and the
  override adds none. Ace protection counts each trainer's own three.
  Every other league match is singles.

## Validation

- Every notable trainer has exactly one play style, one of the eight; the
  catalog script rejects an unknown style and a trainer with none or more
  than one. Only Lance has the boss flag.
- No resolution combines Risky with Conservative, uses Stall, or lacks any
  Basic flag, for every style, tier, ace count, and boss flag.
- AI skill boundaries: TR 29/30, 69/70, and 109/110 change tier; nothing
  changes past 110.
- Ace protection: one ace gives Ace Pokemon, two or more Double Ace Pokemon,
  never both; zero aces is invalid content (slot 1 is always an ace); neither
  flag under trainer-species randomization.
- A resolved-flags report per trainer at world progress 0, 40, 80, 120, and
  160: TR, AI skill tier, aces in the team, and the flag list, matching the
  examples above.
- ROM checks once implemented: the override runs after the standard flag
  setup and before the first decision, writes every controlled battler and
  the prediction slots, keeps Double Battle in a double battle, reproduces
  identically on reconstruction and retry, and leaves regular trainers and
  standalone builds unchanged. Playtesting judges how each style feels.

## Open questions

None in v0.

## Later

- Regular trainers reading player TR for their AI skill.
- AI-driven item use in tiers (trainer inventories that scale with TR).
- More play styles as new trainers need them.
- A three-ace engine tier (protecting the last three party members).

## References

- [Notable trainers](notable-trainers.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Leagues](leagues.md)
- [Player Trainer Rating](player-trainer-rating.md)
- [AI flag constants](../../game/include/constants/battle_ai.h)
- [AI flag setup](../../game/src/battle_ai_main.c)
