# Playthrough-seeded variation

Implemented: No
Design status: Framework intent confirmed; technical defaults are proposed below.

## Intent

Give each Wayfarer playthrough a lasting identity. New authored features should
be able to vary between saves while producing a stable answer to the same
decision within a save. Reloading to repeat an unchanged decision should not
offer a better draw.

Living rivals are the first consumers: each save gives familiar trainers their
own growth arc and shifts who appears in each championship field. Future
features can use the same playthrough seed without depending on those consumers
or changing their results.

## Design

### Confirmed requirements

- Generate and retain one shared playthrough seed, rather than a seed owned by
  any single feature.
- Provide a reusable way for future Wayfarer features to derive their authored
  random decisions from that seed.
- Make opted-in decisions stable on reload so repeating the same situation does
  not provide a fresh random outcome.
- Allow more variation between playthroughs as additional features adopt the
  framework.
- Preserve existing Pokémon randomness. Adoption is explicit for each new
  feature; this is not a replacement of the game's existing RNG systems.

### One root, independent decisions

Propose a 64-bit root seed generated once during new-game initialization and
stored as part of Wayfarer's shared save state. Existing saves load their seed;
they never derive another one from the clock, location, or current RNG state.
A copied save belongs to the same playthrough. Starting a new game creates a
new seed; different seeds can still share individual outcomes.

Each decision has a stable identity, for example circuit order or one trainer's
growth arc. The result depends on the root seed, that identity, its declared
occurrence, and its rules. Opening menus, fighting a battle, querying another
feature, or calling the same decision again does not move a shared random
sequence forward.

Separate decisions also stay independent as development continues. Adding a
new feature or trainer must not change existing circuit orders or growth arcs.
Changing the circuit's trainer pool may change newly generated fields, but must
not change the independently keyed venue order. Pure keys do not imply
independent inputs: a feature can deliberately let one decision's outcome, such
as a trainer's arc, feed another's eligibility.

### Stable outcomes and meaningful new occurrences

Support these patterns without requiring every future feature to generate all
its content at new game:

| Pattern | Identity and lifetime | Example |
| --- | --- | --- |
| Playthrough-wide choice | One named decision for the entire save | An optional future authored world variant |
| One-time entity choice | Named decision plus a stable content identity | A trainer's growth arc |
| Repeated event | Named decision, entity, and an explicit progression occurrence | A circuit edition's itinerary and each venue's field |

The world-variant example illustrates framework capability; it does not
authorize or specify that feature.

Each consumer must define which inputs it uses and when they become fixed.
Re-entering a map, inspecting a menu, declining an offer, retrying a failed
attempt, saving, loading, or waiting does not create a new occurrence by default.
An occurrence advances only through that feature's specified gameplay
transition, recorded with its outcome.

Before revealing or applying a generated result, commit it to the owning
feature's logical save state when it has lasting consequences. This does not
require a physical autosave for every decision. Loading a save from before the
first reveal must reproduce the same result from the same key and inputs;
loading afterward reads the committed result.

If an outcome is promised to be fixed for a playthrough, its candidate pool
cannot be silently rebuilt from the player's current party, TR, time, or
location at each reveal. Generate it at new game or freeze its relevant inputs
at a defined earlier milestone. A feature that intentionally responds to
changed progression must describe that behavior to the player and define its
input boundary in its own spec.

The guarantee covers repeating an unchanged decision. A different quest choice
or another explicitly meaningful change of eligibility can lead to a different
valid outcome. The framework does not promise that every route through the
story produces the same events.

### Trainer growth arcs

Each canonical trainer gets one growth arc per save, chosen from a short
authored list of lore-fitting arcs. The arc shapes how their strength moves
relative to the world as badges accumulate and through the first few circuit
editions. Every arc leaves the first
encounter unchanged, and the same person keeps the same arc in Gyms and leagues
for the whole save. [Trainer world progression](trainer-world-progression.md)
and its [specification](../specs/trainer-world-progression.md) own arcs,
standing, and levels.

Which supporting Pokémon a trainer brings also varies per save, through a
stable per-trainer, per-Pokémon draw owned by the same progression docs.

### Circuit fields

Each circuit edition visits the three leagues in a seeded order, and each
(edition, venue) has exactly one competition. Entering it freezes a seeded
five-trainer field; a loss retries that same field, and only a win moves on.
The [Seeded Trainer Circuit PRD](seeded-trainer-circuit.md), the
[circuit persistence spec](../specs/seeded-league-circuit.md), and the
[pool spec](../specs/circuit-trainer-pool.md) own signup, selection, rotation,
and saved state.

## Boundaries

Battle accuracy, damage variation, critical hits, secondary effects, capture
rolls, ordinary wild encounter selection, Pokémon generation, existing item
chance mechanics, and existing randomizer modes retain their current RNG
ownership. This change does not make ordinary Pokémon battles replay
identically or promise to remove all reasons to reload any part of the game.

Newly seeded authored content can affect the situation in which ordinary
mechanics run. Selecting a circuit trainer is seeded; their battle's ordinary
random rolls remain ordinary battle RNG.

The framework supplies deterministic decisions, not a generic quest engine,
world simulator, reward ledger, or universal saved dictionary. Features own
their eligibility, occurrence rules, resolved results, and reward accounting.
Players retain normal saving and loading. Seed entry, seed sharing UI,
converting existing random systems, and a generic calendar are outside this
initial scope. The framework imposes no real-world schedule.

## Presentation

No additional new-game choice is required. The root seed is an internal save
property, available to debug and reproduction tooling. A future seed-sharing
interface can be added separately.

Features explain their own stable content through their normal interfaces.
For example, the circuit shows its current edition, saved itinerary, and the
entered field, and growth arcs surface only as in-world hints. A player-facing
edition number describes progression; internal hashes, keys, and draw counters
do not belong in ordinary player flows.

## Constraints

The same root seed, decision identity, rules version, occurrence, and relevant
inputs produce the same output on supported builds and in test tooling. Candidate
tables use stable identities and canonical ordering so an unrelated source-file
reorder does not change outcomes.

Version decisions individually. Adding unrelated content must not require a
new global content number in every random key. Changing a decision's actual
candidates, weights, or rules can legitimately change unresolved outcomes;
committed results must not silently change when loading a save.

Use the repository's prerelease save policy. An incompatible save or missing
root seed is not an opportunity to reroll a continuing playthrough. No general
backward-compatibility or historical algorithm framework is required before a
public compatibility baseline exists.

Keep storage and computation suitable for the GBA: one compact root record,
small transient derivation state, and fixed save fields owned by each feature.
Measure the implementation's ROM, RAM, and execution cost before enabling it.

## Playtesting

- Can players retry and reload an adopted decision without seeing a new draw,
  including when the save predates its first reveal?
- Does reloading show the same growth arc for each trainer and the same field
  for an entered competition?
- Does a loss clearly retry the same frozen field, with no way to reroll it
  through reloads, menus, or leaving and returning?
- Do different arcs create visible variety between saves ("Clair got scary this
  save") while first encounters stay identical, and do Gym and league
  appearances of the same person clearly share one arc?
- Do different seeds produce useful variation rather than merely different
  numbers attached to otherwise identical playthroughs?
- Does each feature still feel coherent and fair under its generated content?

## References

- [Playthrough seed framework specification](../specs/playthrough-seed-framework.md)
- [Seeded Trainer Circuit PRD](seeded-trainer-circuit.md)
- [Circuit pool generation](../specs/circuit-trainer-pool.md)
- [Circuit persistence and progression](../specs/seeded-league-circuit.md)
- [Trainer world progression PRD](trainer-world-progression.md)
- [Trainer world progression specification](../specs/trainer-world-progression.md)
