# Trainer party scaling implementation audit

This audit accompanies the [Trainer scaling specification](../specs/trainer-party-scaling.md).
The implementation consumes `GetTrainerRating()` without changing its progression
inputs. The League circuit remains a separate task.

## Authoring and regeneration

`game/tools/trainer_scaling/classification.json` records the reviewed policy for
each populated Wayfarer opponent ID. The proposal command helps discover entries;
normal generation requires the manifest and rejects missing classifications.

Run these commands from the repository root:

```sh
make -C game trainer-party-scaling-generate
make -C game trainer-party-scaling-test
make -C game trainer-party-scaling-audit
```

Generation runs `trainerproc` over the authored party sources, resolves roster
ownership, and emits the policy table, numeric predecessor table, move-exception
table, and inventory under `game/src/data/trainer_scaling/`. The audit checks that
these checked-in outputs reproduce. Wayfarer ROM and mechanics checks require it.

Trainer predecessor coverage comes from the same validated numeric evolution
graph as wild encounters. The Trainer metadata includes eligible source species
and reachable predecessors. It has no species floors, forward evolution, or
slot filtering. Exact numeric aliases share metadata; distinct forms retain their
own identities.

The default move policy replaces custom moves with the final species' active
level-up moves. The explicit move-exception list starts empty. Future exceptions
need a roster-owner ID, selected difficulty variant, original source slot, and
reason. Runtime retains the entire authored tuple only if the species is unchanged
and every nonempty move is available by the projected level.

## Construction and option order

`CreateNPCTrainerPartyForOpponent` validates an opposing ID and battle context,
then resolves roster overrides. Policy uses the encounter ID; move exceptions use
the final roster owner and original selected pool index. `GetTrainerStructFromId`
retains authority over difficulty selection. Hoenn continues to use its fixed
source roster.

The first eligible opponent reads Rating once. Both opposing Trainers and any
reconstruction within the battle reuse that snapshot. Battle initialization and
cleanup discard it, so a retry reads a fresh value. A boss paired with an ordinary
Trainer retains its authored party.

The selected party size, slot order, pool rules, and two-opponent capacity remain
in effect. For each selected slot, construction applies this order:

1. Calculate the independent Trainer curve and authored-level adjustment.
2. Pass original species, Trainer class, output position, and selected party count
   to the existing Trainer randomizer. If Trainer species randomization is off,
   resolve numeric predecessors against the projected level.
3. Derive personality and gender against the final species, create the Pokémon,
   and apply generated moves or a valid reviewed exception. Existing learnset
   randomization remains active.
4. Restore authored retained fields, select a legal final-species ability, and
   calculate stats. Suppress incompatible species-specific gimmicks while keeping
   the held item.
5. Apply the existing explicit IV and EV challenge options and recalculate stats.

The player party and player soft cap never supply an enemy level. No second enemy
level projection or challenge-cap clamp runs after this transformation. The mirror
challenge retains its existing behavior of copying the constructed opponent party
to the player.

## Excluded construction paths

The shared raw `CreateNPCTrainerPartyFromTrainer` entry point stays unscaled. Its
partner, player-party, and debug callers cannot enroll themselves merely by passing
a Trainer pointer. Debug opponent construction also bypasses policy.

Frontier and its rental sources, Trainer Hill, e-Reader, Secret Base, link,
recorded, tutorial, raid, and externally supplied parties do not enter automatic
scaling. Those context exclusions precede ordinary ID policy. The classification
manifest separately excludes Gym Leaders, rivals, villain bosses/admins, League
members, and other authored story bosses, including their variants.
The special Sinjoh Milkman battle is also excluded: its 99 Moomoo Milk threshold,
bespoke battle script, and six level-70/80 Miltank make it an authored special
challenge rather than an ordinary opponent.

`B_TRAINER_PARTY_SCALING` in `game/include/config/trainer_party_scaling.h` is the
single rollback switch. Disabling it bypasses level, species, and move
transformation together. It requires no save migration.

## Rewards and integration ownership

Battle experience reads the constructed species and level, then follows existing
experience multipliers and the player's soft-cap reduction. Prize money continues
to read authored Trainer data through the existing reward path. Roster aliases
whose encounter record has no party retain the existing fixed fallback reward;
the scaling implementation does not repair or reinterpret those money inputs.
Defeat state, rematch availability, scripts, reward eligibility, and Trainer AI
remain under their existing authorities.

This task owns scaling/classification, opponent construction, shared predecessor
tooling, and scaling tests. It does not edit `GetTrainerRating`, global badges,
League gates, regional starts, or League rosters. This audit is the integration
record for the parallel League task. Any later League party edits must regenerate
the inventory even when those opponents remain excluded.

The shared script edit is limited to `game/data/scripts/set_gym_trainers.inc`:
`Common_EventScript_SetGymTrainers_Frlg` and its eight FRLG handler blocks now
compile only under `IS_FRLG`. Those unused Wayfarer blocks referenced FRLG IDs
without Wayfarer roster authority. HNS and Hoenn Gym handlers are unchanged.

## Validation and balance acceptance

The generated audit evaluates every eligible source slot at Ratings 0 through 80
in normal and legacy learnset modes. It separates structural failures from balance
observations and reports representative parties, move changes, ability fallbacks,
gender adjustments, held-item concerns, and authored reward inputs.

Formula tests and constructed-party tests do not establish playable balance.
Mechanics tests run in mGBA are automated runtime verification; compilation alone
is not emulator playtesting. Representative regional balance playtests remain a
separate acceptance requirement until their actual results are recorded here.
