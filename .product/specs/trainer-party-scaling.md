# Regular trainer and Gym member scaling

PRD: [Regular trainer and Gym member scaling](../prds/trainer-party-scaling.md)
Implemented: Partial; runtime policies exist, campaign balance acceptance remains pending.
[v0 final-stage rosters](#v0-final-stage-rosters) are implemented (#157), and so are
the [v0 levels](#v0-levels).

Today's routing includes [League scaling](league-scaling.md) with the player's
TR saved when entering a league, so League levels scale from that saved TR. See
[the level resolver](../../game/src/trainer_party_scaling.c) and
[the circuit producer](../../game/src/league_circuit.c). The v0 TR design
(rescaled formula, uncapped TR, scalers, and notable trainers' separate TR) is
in [Player Trainer Rating](player-trainer-rating.md); this document owns the
v0 levels in [v0 levels](#v0-levels) and the v0 rosters in
[v0 final-stage rosters](#v0-final-stage-rosters). Everything else here is
Today.

## Scope and authority

Implement for `IS_WAYFARER`. Standalone builds retain their existing behavior.
This specification owns automatic party transformation for regular opposing
trainers and Gym members. It does not implement Trainer Rating (TR) advancement,
boss scaling, new rematches, or new rosters.

Use `GetTrainerRating()` as the only progression input. Store no per-Trainer
scaled roster, cap, level, or historical TR in save data. Keep the source
Trainer records immutable.

## Classification contract

Generate a compact policy table indexed by the active Wayfarer Trainer ID.
Use five policies: `ORDINARY`, `GYM_MEMBER`, `GYM_LEADER`, `LEAGUE`, and
`EXCLUDED`.
Every populated Trainer ID must have exactly one policy. `GYM_LEADER` routes
only enrolled initial badge battles to the separate
[Gym Leader scaling specification](gym-leader-scaling.md); it never receives
this specification's regular trainer transformation. The dedicated six-slot Gym
feature is disabled by `B_GYM_LEADER_SCALING` in the default configuration; its
compiled plans would use player TR when enabled. Disabled paths retain existing
authored behavior. Giovanni's initial Viridian battle separately uses a bespoke
five-slot player-TR projection in
[party construction](../../game/src/battle_main.c). The v0 notable trainer TR
and rosters belong to the [Notable trainers](notable-trainers.md) and Gym
specifications. `LEAGUE` currently routes the fifteen enrolled circuit runtime
IDs to the separate [League scaling specification](league-scaling.md), including
its run-context validation and authored fallback when scaling is disabled. Those
fifteen positions cover seventeen possible source parties: Blue's one Wayfarer
runtime ID may resolve to any of three reviewed FRLG source variants. Raw FRLG
IDs are provenance and must not index the active policy table. Unclassified IDs
fail generation; invalid runtime IDs fail closed to existing unscaled behavior.

A populated ID has a nonempty party in at least one selectable difficulty
variant after roster overrides are resolved. Zero-initialized table holes do
not require manifest records. A script or rematch reference to a hole or an
empty resolved roster is a validation failure, not an implicit exclusion.

Keep the authoring inventory in a versioned machine-readable manifest. Each
record contains the symbolic Trainer ID, policy, and evidence references to
maps, scripts, rematch tables, or an explicit authored role. Excluded entries
also carry a reason. A batch inventory tool should propose the initial records
from existing data, so authors review classifications rather than rewrite
parties. Reject duplicate IDs, unknown IDs, stale references, and missing IDs.

Gym-map references are discovery evidence, not the runtime classifier. Use the
opposing Trainer ID at runtime; do not apply a Gym bonus merely because a battle
takes place on a Gym map. Require explicit review when one ID serves both
Gym-member and regular trainer roles. Select one documented policy for that ID,
or split its source ID before enrolling it; never infer a context-dependent
policy silently.

Exclude all Gym Leaders from the regular trainer transformation. Only IDs
explicitly enrolled as initial badge battles may use the `GYM_LEADER` routing
policy in the [Gym Leader scaling specification](gym-leader-scaling.md).
Enumerate their aliases and scripted variants explicitly: enrolled initial-badge
variants use that policy, while rematch and other story variants remain
excluded Today; on adoption a leader's rematch and story battles follow the
notable model through the
[every-battle rule](notable-trainers.md#trainer-rating). Exclude all rival variants, villain bosses and admins, Elite Four,
Champions, other story bosses, and tutorial opponents from regular trainer
transformation. Explicitly enrolled circuit IDs use `LEAGUE` routing; other
Elite Four and Champion variants remain `EXCLUDED`. Shared classes must not
cause boss enrollment. Regular trainers' scripted battles and villain grunts
remain eligible.

Facility, link, recorded, external-party, partner, and player-party construction
paths are excluded by battle context before ID policy is considered. Include
Frontier, Trainer Hill, e-Reader, Secret Base, and rental-party sources in that
context audit. A raw `struct Trainer *` without a validated opposing ID and
eligible context does not authorize scaling.

## Level projection

Author the following regular trainer level curve independently from the wild
level curve and the level cap curve:

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

Clamp TR to 0 through 80. Interpolate adjacent curve anchors linearly,
rounding exact halves upward. For integer division define `roundSigned(n/d)`
as nearest integer with exact halves away from zero and positive denominator.
For authored level `L` in 1 through 100, with the authored level bonus as
`identityAdjustment` and the curve as `baseline`:

```text
identityAdjustment = clamp(roundSigned((L - 5) / 5), -1, 8)
roleBonus = 2 for GYM_MEMBER, otherwise 0
effectiveLevel = clamp(baseline(Rating) + identityAdjustment + roleBonus, 1, 100)
```

Project each selected slot independently. Party order and size do not change;
the compression and level-100 clamp may make previously distinct levels equal.
No random level jitter, player-party matching, wild profile offsets, species
floors, or enemy clamp to the player's level cap applies. Because the curve is
monotonic and bonuses are constant, no cumulative maximum projection loop is
necessary.

Read TR once when constructing an eligible battle's opponents. Both opponents in
a two-Trainer battle share this snapshot but retain independent policies. A boss
paired with a regular trainer remains unscaled while the regular trainer scales.
Reuse the snapshot if setup reconstructs a party; discard it after the battle.
Retry after a loss starts a new snapshot.

## v0 levels

Implemented: Yes. The runtime is `GetTrainerScalingLevel` and `GetTrainerPlaceLevel`
in [the level resolver](../../game/src/trainer_party_scaling.c); the map table is
generated by `game/tools/trainer_scaling/reach.py` into
`game/src/data/trainer_scaling/trainer_places.h`, and the report of which rule
resolved each map is `game/src/data/trainer_scaling/reach_report.md`.

On the [v0 TR scale](player-trainer-rating.md#formula-v0), `ORDINARY` and
`GYM_MEMBER` slots no longer use the Today curve above. All values are
placeholders for playtesting.

### Reach levels for ORDINARY

An `ORDINARY` slot's level comes from the reach of the map where the battle
starts, the same reach and dungeon levels wild Pokémon use:

```text
placeLevel = reachLevel(battleMap, Rating)
effectiveLevel = clamp(placeLevel + TRAINER_REACH_BONUS, 1, 100)
TRAINER_REACH_BONUS = 3
```

`reachLevel` is the [wild level scaling](wild-level-scaling.md#reach-levels)
level for a Road, Wilds or Outlands map, or the
[dungeon level](wild-level-scaling.md#dungeon-levels) of the map's floor,
floor minimum included. Take the level before any wild-only adjustment: no
stage mix, encounter roll, prowler minimum or random jitter. Every slot of a
party gets the same `effectiveLevel`. The authored level no longer adjusts it
(`identityAdjustment` is dropped), so party order is the only remaining
difference between slots.

The battle map is `gSaveBlock1Ptr->location` at battle construction. A
wandering trainer from [daily world slots](../prds/daily-world-slots.md)
therefore takes the level of the spot they stand on that day, and both
opponents of a two-Trainer battle share one place level.

### Maps without a reach of their own

[Reach assignments](reach-assignments.md) only cover maps with wild
encounters. Every other map that hosts a covered Trainer resolves its reach at
generation time, in this order:

1. The map is listed in reach assignments: use that reach or dungeon floor.
2. Otherwise use the
   [places without wild encounters](reach-assignments.md#places-without-wild-encounters)
   rules: extra dungeon floors, story-site dungeons such as Silph Co. or the
   S.S. Anne, ferries as Road, and interiors taking the map they open onto.

Generation emits a compact map-to-reach table for every map that hosts a
covered Trainer, plus a report of which rule resolved each map. A covered Trainer on a map with no resolvable entry fails generation; it never
falls back silently at runtime.

How `reach.py` applies this:

- **Hosting.** A map hosts a Trainer when a script reachable from its events or
  its map script table battles that ID (`trainerbattle*`, following gotos,
  calls and fall-through). Every stage of a rematch family stands on the map of
  its first stage, or on the rematch table's map when no script places it. IDs
  no compiled Wayfarer map runs, such as the Hoenn rematch stages the ROM does not
  compile and retired source Trainers, are listed in the report and need no entry.
- **Listed maps** use the wild generator's data
  (`wild_encounters_v2/meta.json` and the reach-assignments intents), the same
  records the wild headers use.
- **Joining an existing place** and **new dungeons** are the reviewed rule data
  in `game/tools/trainer_scaling/reach_places.json`, one entry per row of the
  two tables in reach assignments. A joined dungeon is one floor order: the
  listed floors of Sprout Tower, the Johto Rocket Hideout, Abandoned Ship and
  Seafloor Cavern take their step in the extended order (and the Hideout its
  higher intent) for Trainer levels, so the new floor sits where the table puts it.
  Wild levels keep the listed floors.
- **Ferries** are the S.S. Aqua and S.S. Tidal maps. **Interiors** take the nearest
  placed map by warps; when one opens onto both a town or route and a dungeon
  floor, the outdoor map wins, and any other disagreement needs a reviewed
  `interior_overrides` entry.
- **Gyms and facilities** never get an entry: an `ORDINARY` Trainer on one fails
  generation (none exists today).
- A map missing from the table counts as Road at runtime. That is a defensive
  default for tests and debug battles: generation guarantees every map that hosts a covered Trainer is in the table.

### Gym members

`GYM_MEMBER` slots keep a TR curve, because Gyms aren't on the danger map:

| Badges | v0 TR | Gym member base level | Level cap | Gap to cap |
| ---: | ---: | ---: | ---: | ---: |
| 0 | 0 | 9 | 15 | −6 |
| 4 | 40 | 27 | 28 | −1 |
| 8 | 80 | 44 | 50 | −6 |
| 16 | 120 | 62 | 75 | −13 |
| 24 | 160 | 82 | 100 | −18 |

The curve interpolates as a [scaler](player-trainer-rating.md#scalers) and stays
flat past TR 160. The Gym-member +2 and the 1–100 clamp apply on top. The
authored level adjustment is dropped here too.

### Unchanged

The battle-start TR snapshot, retry behavior, two-opponent rules and the
level-100 clamp work as above. No enemy level is clamped to the player's level
cap.

## v0 final-stage rosters

Implemented: Yes (#157). Rosters are authored by
`game/tools/trainer_scaling/final_stage.py`; its report is
`game/src/data/trainer_scaling/final_stage_report.md`.

In v0, regular trainers' Pokémon evolve as their level rises. This replaces
"Do not forward-evolve base species".

**Authoring.** Every `ORDINARY` and `GYM_MEMBER` roster, rematch teams and the
ordinary Sevii, Kanto coast and S.S. Anne rosters included, names each slot at
the highest stage it may reach. A reproducible authoring tool proposes the
source edits from these rules, and authors review its report. Levels, IVs,
natures, party size and order don't change.

1. **Final stage.** A slot names the final stage of its line, as compiled for
   Wayfarer. Later-generation extensions count, such as Magnezone, Togekiss,
   Annihilape or Kingambit, matching the
   [natives rule](../prds/wild-encounters-v2.md#natives) that later evolutions
   come with the lines they extend.
2. **Regional forms stay home.** A regional form, or an evolution that only
   happens in one region, is chosen only for a trainer in that region: Alolan
   forms in Alola, Galarian forms on Sevii, Hisuian forms and evolutions in
   Sinjoh. Elsewhere the standard form is used, and a line whose only further
   step is region-locked stops before it.
3. **Branches follow the trainer.** A branching line takes one final form per
   slot, chosen deterministically from the trainer's class, theme and gender:
   for example Poliwrath for fighting classes and Politoed otherwise, Bellossom
   for Aroma Ladies and Beauties, Gallade for male fighters, Froslass for female
   trainers, and Eevee's form by the trainer's type theme. Branches with no
   theme are chosen by trainer ID. A gender-specific evolution sets the slot's
   gender.
4. **One evolved copy per line.** Within one team, only the highest-level copy
   of a line reaches its final stage (ties go to the later slot). A copy
   already authored at the final stage uses up that one evolved copy. When a
   team has three or more copies of a three-stage line, the next copy may
   reach the middle stage. Every other copy keeps its authored species. Six
   Magikarp become one Gyarados and five Magikarp, two Zubat become one Crobat
   and one Zubat, and five Geodude become one Golem, one Graveler and three
   Geodude.
5. **Identity exceptions.** A slot keeps its authored species when it holds an
   Everstone or Eviolite, holds a booster that only works on its current stage
   (Light Ball on Pikachu, Lucky Punch on Chansey, Deep Sea Tooth or Scale on
   Clamperl), belongs to a child class (Tuber, School Kid, Twins, Sis and Bro,
   Preschooler), belongs to a team made only of babies, or is a legendary,
   mythical or special line such as Cosmog or Type: Null. Youngsters aren't a
   child class: a Youngster's Rattata evolves.
6. **Never past a step-back.** A slot never goes past a stage the step-back
   rule can't lower again to its authored species. Babies therefore keep their
   species: the step-back has no edge from Snorlax to Munchlax.

**Step-back.** The scaler lowers each slot through the shared
[downward rule](player-trainer-rating.md#evolution-stages) until the effective
level supports its stage, for every evolution method. Level evolutions step
back at their evolution level, and stone, trade, friendship and other
evolutions through one shared authored level. Reuse the notable trainers'
evolution-stage table rather than a second copy. An authored early stage is a
ceiling: the scaler never evolves a slot past what its roster names.

**Report.** The authoring report lists every slot's before and after species,
the rule that applied, branch choices, duplicate caps and exceptions. The
scaling audit adds representative parties at low, middle and high TR showing
non-level step-back.

## Species, moves, and per-Pokémon fields

Resolve numeric level-evolution predecessors until the effective level supports
the resulting stage. Reuse the validated numeric predecessor authority behind
wild encounters, or extract it into a shared generator/helper. Extend metadata
coverage to every eligible Trainer species and all reachable predecessors;
the existing ordinary-wild species inventory is not sufficient by assumption.
Resolve ambiguous ancestry explicitly and reject cycles during generation.

Do not apply wild species floors or remove a party slot. Non-level evolutions
have no inferred reverse relationship. Do not forward-evolve base species. In
v0 rosters are authored at their final stages instead; see
[v0 final-stage rosters](#v0-final-stage-rosters).
Preserve exact forms unless a validated predecessor edge specifies otherwise.
Report powerful species with no numeric predecessor for balance review.

**v0.** The paragraphs above describe Today. In v0 predecessor resolution
follows the shared
[downward rule](player-trainer-rating.md#evolution-stages), which also steps
down non-level evolutions through the shared evolution-level table (an
effective Lv 30 Alakazam becomes Kadabra). Forms, wild-floor exclusion, and no
forward evolution are unchanged.

The default move policy for every eligible slot is `LEVEL_UP`: create its
normal four-move set for the final species and effective level using the current
learnset. Bypass `CustomTrainerPartyAssignMoves` for these slots, including
slots that originally authored custom moves. Do not retain late-game custom
moves as an accidental fallback. Honor existing move-randomizer policy after
this default where that option explicitly applies.

A reviewed `AUTHORED_MOVES` exception is keyed by roster-owner ID and authored
slot index, with a reason. It retains the original move tuple only when the
final species equals the authored species and every nonempty move appears in
that species' active level-up learnset at or below the effective level.
Otherwise regenerate the entire set. This conservative rule intentionally
does not preserve TM, tutor, or egg moves without a level-up justification.
Validate the current learnset. No slot may end with zero usable
moves; generation must identify such outcomes and require a reviewed source
correction before release.

Retain authored held items, nature, IVs, EVs, friendship, nickname, ball, shiny
intent, and AI. Derive gender and ability against the final species. Preserve
an explicit ability only if present on that species; otherwise use its first
nonempty standard ability. Existing random-ability selection must select a
valid slot from the final species. A reversed species that cannot support an
authored gender uses its legal species gender behavior.

Audit form-specific held items and gimmick flags. If transformation invalidates
a species-specific form or gimmick, suppress that incompatible activation while
retaining the held item. List every affected slot in the report. Do not copy
an ability index from the authored species into a different final species.

## Runtime integration

The current central seams are `CreateNPCTrainerParty` and
`CreateNPCTrainerPartyFromTrainer` in `game/src/battle_main.c`. The former knows
the opposing ID and resolves `overrideTrainer`; the latter also serves debug
and player-party construction, so it must not unconditionally scale all callers.
Pass an explicit context or perform an equivalent validated transformation.

Resolve the existing rematch ID and roster override before selecting party-pool
entries. Preserve the original encounter ID for policy, and roster-owner ID
plus original selected slot index for move exceptions. Alias chains must have
validated termination and unambiguous ownership. Preserve the engine's selected
party count and two-opponent storage limits.

Consume the roster selected by `GetTrainerStructFromId`, including any existing
difficulty variants. Preserve the Hoenn content contract's fixed source-roster
selection. Where multiple difficulty variants exist, key move exceptions by
variant as well as roster-owner ID and slot, and validate every selectable
variant. Inventory the `.party` sources and `trainerproc` output rather than
assuming all Trainers use one literal C array.

For each selected slot, determine effective level and species before
`CreateMon`, personality constraints, ability assignment, move creation, and
stat calculation. Keep the original slot available for retained authored data.
Do not mutate or reconstruct the shared ROM roster in place. Avoid extra RNG
draws in level projection, predecessor resolution, and policy lookup.

When Trainer species randomization is active, pass original species, class,
selected-party position, and count to the existing randomizer in its established
order. Bypass predecessor resolution, scale the level, and create moves and
valid abilities for the randomized species. Do not use wild-randomizer state
as the Trainer-randomizer switch.

Existing explicit IV/EV challenge options run after base party creation as they
do today. They do not replace the TR level calculation. Inventory any other
level-changing options or post-creation hooks and prevent double scaling for
eligible opponents. A player challenge level cap does not clamp enemy levels.
Document option precedence in the implementation audit before shipping.

Battle XP uses actual constructed Pokémon under the existing XP and level cap
rules. Preserve prize money, rematch flags, defeat flags, scripts, Trainer AI
selection, and reward eligibility. Do not rewrite authored levels merely to make
reward readers observe projected values.

## Generation and audit

The implementation must deliver a deterministic host command that inventories
all compiled Wayfarer Trainer records and emits policy data plus a reviewable
report. Validate all selectable pool slots and resolved overrides, not only
the first party-size entries. Enumerate every TR 0 through 80 using the
current learnset for each eligible source slot. Cache equivalent calculations
where useful; emit summarized intervals rather than duplicate rows.

Report coverage counts by policy and region, excluded reasons, unresolved
classification candidates, custom-move replacements, move exceptions, species
changes, invalid or falling-back abilities, gender adjustments, gimmick
suppression, empty move sets, held-item concerns, and parties above the player's
level cap. Separate fatal structural failures from balance-review observations.

Include curve and effective level ranges, party sizes, and representative
full parties at TR 0, 4, 8, 16, 30, 40, 55, 63, 65, 68, 76, and 80.
Identify the highest-level and largest early parties and high-stat species
that remain unevolved by policy. Show authored prize-money inputs alongside
effective levels and XP inputs. No claim that those populations are balanced
may be inferred solely from passing generation.

Generated output must reproduce from checked-in inputs without an AI service.
Batch source corrections are separate, reviewable edits. Do not silently turn
all custom-move Trainers into excluded opponents to satisfy the audit.

## Validation and rollout

1. Test every TR and authored level against an independent integer oracle,
   exact anchors, signed rounding, bonuses, bounds, and monotonicity.
2. Test `ORDINARY`, `GYM_MEMBER`, `GYM_LEADER` and `LEAGUE` routing, and
      excluded policies, shared-class bosses, aliases, rematch variants, sparse
   IDs, and missing manifest records.
3. Test multi-stage reversal, ambiguous ancestry rejection, forms, non-level
   evolutions, no forward evolution, and no wild floor filtering.
4. Test move regeneration and exceptions below and at thresholds, changed
   species, the current learnset, utility-heavy schedules, and usable moves.
5. Test legal ability and gender assignment after reversal and randomization,
   species-specific items and gimmicks, and retained IV/EV and AI behavior.
6. Test real battle construction, pools, two opponents with mixed policies,
      stable TR snapshots, retries, and no extra projection RNG consumption.
7. Test randomizer and challenge precedence, player/partner/debug construction,
   facility exclusions, and recorded/link battle bypasses.
8. Verify defeat and rematch persistence, existing money rules, and battle XP
   from effective levels. Verify that player party levels never affect output.

Compile affected paths for every supported product and build a complete Wayfarer
release ROM. Run focused battle integration tests and the generated inventory
audit. Playtest representative parties across included regions at TR 0, a middle
milestone, and TR 80, including Gym-member doubles and the highest-risk early
parties reported by the audit.

Land classification and audit tooling before enabling runtime scaling. Enable
the shared feature only after all IDs are classified and structural validation
passes. Keep one build-time feature switch that restores authored construction
for rollback; it must bypass level, species, and move transformation together.
No save migration is required. Formula changes must regenerate the report and
repeat affected balance checks.

## References

- [Trainer Rating foundation and wild scaling](trainer-rating-wild-encounter-scaling.md)
- [Party progression](trainer-rating-party-progression.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Battle party construction](../../game/src/battle_main.c)
- [Trainer Rating runtime](../../game/src/trainer_rating.c)
- [Wild metadata generator](../../game/tools/wild_encounters/wild_encounters_to_header.py)
- [Gym Leader scaling](gym-leader-scaling.md)
