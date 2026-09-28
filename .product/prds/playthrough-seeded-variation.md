# Playthrough-seeded variation

Implemented: No
Design status: v0: one consumer (league lineups). Other seeded features stay
Later.

## Intent

Give each Wayfarer playthrough a lasting identity. Authored features could
vary between saves while giving a stable answer to the same decision within a
save, so reloading to repeat an unchanged decision never offers a better draw.

## Design

### Requirements

- One shared playthrough seed, not a seed owned by any single feature.
- A reusable way for features to derive their authored random decisions from
  it.
- Opted-in decisions stay stable on reload.
- Existing Pokémon randomness is preserved. Adoption is explicit per feature;
  this does not replace the game's existing RNG systems.

### One root, independent decisions

A 64-bit root seed is generated once at new game and stored in Wayfarer's
shared save state. Existing saves load their seed and never derive another
from the clock, location, or current RNG state. A copied save belongs to the
same playthrough; a new game creates a new seed.

Each decision has a stable identity. Its result depends only on the root, that
identity, its declared occurrence, and its rules. Opening menus, fighting,
querying another feature, or repeating a query never moves a shared random
sequence. Adding a feature or content must not change existing decisions,
though a feature may deliberately feed one decision's outcome into another's
inputs.

### Stable outcomes

| Pattern | Identity and lifetime |
| --- | --- |
| Playthrough-wide choice | One named decision for the whole save |
| One-time entity choice | Named decision plus a stable content identity |
| Repeated event | Named decision, entity, and an explicit progression occurrence |

Each consumer defines its inputs and when they become fixed. Re-entering a map,
inspecting a menu, declining an offer, retrying, saving, loading, or waiting
does not create a new occurrence. A result with lasting consequences is
committed to the owning feature's save state before it is revealed; loading from
before the reveal reproduces it from the same key and inputs. A promised fixed
outcome cannot be rebuilt from the player's current party, Trainer Rating (TR),
time, or location at each reveal. A meaningful change of eligibility, such as a
different quest choice, can legitimately lead to a different outcome.

### First use: league lineups

In v0 the seed decides one thing: which five contenders make each league
lineup ([Leagues](leagues.md)). The root seed is created at new game, and each
lineup is one decision keyed on the league and which lineup this is there
(the first, then one more for each replay). It is drawn on first entry and saved before the
lineup is revealed, so reloading, losing, or waiting never re-rolls it. A
replay after winning is the league's next occurrence and draws afresh. There
is no running random sequence behind it, so nothing else the player does
changes the draw.

## Boundaries

Battle accuracy, damage, critical hits, secondary effects, capture rolls,
ordinary wild encounters, Pokémon generation, existing item chances, and
randomizer modes keep their current RNG. The framework supplies deterministic
decisions, not a quest engine, world simulator, reward ledger, or universal
saved dictionary. Seed entry, seed-sharing UI, and converting existing random
systems are out of scope.

## Presentation

No new-game choice. The root is an internal save property, available to debug
and reproduction tooling. Features explain their own content in-world;
internal hashes, keys, and draw counters stay out of player flows.

## Constraints

The same root, identity, rules version, occurrence, and inputs produce the same
output on every supported build and in test tooling, with canonical ordering so
a source reorder changes nothing. Decisions are versioned individually; a
committed result never changes silently on load. A missing or incompatible
seed follows prerelease invalid-save handling and is never an excuse to
reroll. Storage stays GBA-sized: one compact root record plus fixed fields
owned by each feature.

## Later

- Seeded league order, rotation, and recurring editions.
- Per-save trainer growth arcs.
- Per-save variation in which supporting Pokémon fill a trainer's filler
  slots ([Trainer roster influence](trainer-roster-influence.md)).
- Other per-save world variants, each approved as its own feature.

## References

- [Playthrough seed framework specification](../specs/playthrough-seed-framework.md)
- [Leagues PRD](leagues.md)
- [Notable trainers PRD](notable-trainers.md)
