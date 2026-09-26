# Playthrough-seeded variation

Implemented: No
Design status: Framework intent confirmed; technical defaults are proposed below.

## Intent

Give each Wayfarer playthrough a lasting identity. New authored features should
be able to vary between saves while producing a stable answer to the same
decision within a save. Reloading to repeat an unchanged decision should not
offer a better draw.

The [Seeded Trainer Circuit](seeded-trainer-circuit.md) is the first consumer of
a shared framework. Future features can use the same playthrough seed without
depending on the circuit or changing its results.

## Design

### Confirmed requirements

- Generate and retain one shared playthrough seed, rather than a seed owned only
  by the circuit.
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

Each decision has a stable identity, for example circuit order or the contents
of one particular authored reward. The result depends on the root seed, that
identity, its declared occurrence, and its rules. Opening menus, fighting a
battle, querying another feature, or calling the same decision again does not
move a shared random sequence forward.

Separate decisions also stay independent as development continues. Adding a
new reward feature must not change existing circuit orders. Changing the
circuit's trainer pool may change newly generated rosters, but must not change
the independently keyed venue order or an unchanged trainer/league pair's lore
draw. Explicit constraints inside one lineup still apply: selecting a trainer
removes that character from its remaining four places. It does not remove the
trainer from another league; cross-league appearances are allowed.

### Stable outcomes and meaningful new occurrences

Support these patterns without requiring every future feature to generate all
its content at new game:

| Pattern | Identity and lifetime | Example |
| --- | --- | --- |
| Playthrough-wide choice | One named decision for the entire save | The circuit itinerary and its complete roster assignment |
| One-time entity choice | Named decision plus a stable content identity | An optional future quest's authored reward variant |
| Repeated event | Named decision, entity, and an explicit progression occurrence | An optional future competition edition unlocked by a real milestone |

The latter two examples illustrate framework capability; they do not authorize
or specify those gameplay features.

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

### Circuit adoption

At new game, the circuit derives its venue order, each eligible unaffiliated
trainer/league pair's lore filter, and its roster assignment as separately keyed
decisions from the shared seed. TR eligibility is checked before the lore
filter; authored league affiliations can overlap. Persist the complete schedule before
it is exposed to gameplay. There is no independent saved circuit seed and no
circuit-wide cursor that other features can advance.

All existing circuit rules remain in its PRD, including uniqueness within each lineup,
regional selection preference, rating-based strength, and the unresolved
D1–D3 choices. This framework does not settle those balance/content decisions.

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
Players retain normal saving and loading. Seed entry, seed sharing UI, seasons,
and converting existing random systems are outside this initial scope.

## Presentation

No additional new-game choice is required. The root seed is an internal save
property, available to debug and reproduction tooling. A future seed-sharing
interface can be added separately.

Features explain their own stable content through their normal interfaces.
For example, the circuit shows its saved itinerary and lineups. Do not expose
hashes, internal keys, or generation counters in ordinary player flows.

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
- Do different seeds produce useful variation rather than merely different
  numbers attached to otherwise identical playthroughs?
- Are intentionally progression-dependent choices understandable, with no
  accidental incentive to wait, open menus, or churn failed attempts?
- Does each feature still feel coherent and fair under its generated content?

## References

- [Playthrough seed framework specification](../specs/playthrough-seed-framework.md)
- [Seeded Trainer Circuit PRD](seeded-trainer-circuit.md)
- [Circuit pool generation](../specs/circuit-trainer-pool.md)
- [Circuit persistence and progression](../specs/seeded-league-circuit.md)
