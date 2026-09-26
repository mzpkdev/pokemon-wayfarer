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
- Support recurring circuit editions under that same seed. Completing an
  edition unlocks the player's choice to start the next; retrying or reloading
  does not create another edition.
- Resolve edition 1 order at New Game, then construct the complete field at
  first eligible registration from projected badge/first-clear milestones. Lock
  that edition through retries and replays; registration is the input boundary.
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
the independently keyed venue order or a venue/slot's raw home-or-visitor draw
within the same edition. Candidate availability can change whether that slot
needs a category draw or uses a home-only fallback. Constraints inside one lineup
still apply: selecting a trainer removes that character from its remaining four
places. Cross-league appearances remain possible, with a soft penalty based
on scheduled appearances elsewhere in the edition. This intentionally couples
roster allocations; pure keys do not imply independent selection inputs. The
previous completed edition's lineup at the same venue also affects roster weights.
Neither constraint changes ORDER or raw venue/slot POOL_KIND draws.

### Stable outcomes and meaningful new occurrences

Support these patterns without requiring every future feature to generate all
its content at new game:

| Pattern | Identity and lifetime | Example |
| --- | --- | --- |
| Playthrough-wide choice | One named decision for the entire save | An optional future authored world variant |
| One-time entity choice | Named decision plus a stable content identity | An optional future quest's authored reward variant |
| Repeated event | Named decision, entity, and an explicit progression occurrence | A circuit edition, with its complete itinerary and roster assignment |

The first two examples illustrate framework capability; they do not authorize
or specify those gameplay features. Recurring circuits are the first consumer.

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

The confirmed circuit lifecycle resolves the shared root and edition 1 venue
order at New Game. Before registration, the save has a valid itinerary but no
roster, no active run, zero results/completed count, and empty history. It can be
saved and inspected; it cannot enter a venue or generate a lineup on inspection.
First eligible registration generates the complete edition. D4's common
24-global-badge threshold remains proposed rather than approved here.

Registration captures global badges `B_reg`, lifetime venue-clear mask `L_reg`,
prior-edition history, and generation/content versions. At scheduled stop i,
use `B_i = B_reg` and `C_i = popcount(L_reg union earlier scheduled venue IDs)`.
Resolve each trainer's authored personal badge curve plus bounded first-clear
growth, capped at TR 80, and its team stage/profile before eligibility. Badges
and first clears intentionally affect the world; player party, player TR, XP,
retries, and edition number do not adjust opponent strength. Numeric growth
curves in the balance explorer are provisional.

The venue order, slot home/visitor decision, and roster assignment retain their
separate semantic keys under the shared root and edition identity. Role bands
are authored by world point and shared across venues at the same point; D3 still
owns their endpoints. Check projected TR/stage/content eligibility first, then
partition using authored `homeLeagues`. Choose home with 85% probability or
visitors with 15% when both are available, then apply positive rotation weights
within that pool. No eligible visitors means home; missing eligible homes is
invalid content, never permission to widen bands or substitute visitors. Prove
home-only two-contender/two-elite/one-headliner coverage and variety at every
supported registration/projected point, including C 0–3 and saturation.

Save all fifteen identities, projected effective TR, stage/profile references
and versions, and snapshot inputs together before revealing the field. The
confirmed lifecycle freezes every venue through retries and replays, even after
live progression changes. Numeric progression, role bands, profiles, and
qualification remain provisional. Read-only
verification reproduces plans from registration inputs, never the live world.
Missing or corrupt roster/snapshot state cannot be repaired through regeneration.

After all three current results commit, the player may register for the next
edition. That transition captures current world progress and the outgoing
edition's roster IDs as bounded participation history, then commits the new
identity, complete schedule, snapshots, and history atomically. Failure preserves
the prior edition. Loading a pre-registration save repeats the same outcome for
identical root, next identity, versions, milestones, and history. Losses, replays,
and waiting never create another occurrence or endless strength inflation.

Later keys may yield repeated orders and entrants. Soft weights encourage
roughly two returning and three different opponents per venue without quotas or
novelty rerolls; missing the immediately previous venue lineup clears its return
penalty. Preserve existing semantic IDs: ORDER/POOL_KIND rules remain version 1
and ROSTER rules become version 2. Record an explicit changed schedule schema
for projected snapshot inputs. The framework does not choose trainer curves, role-band numbers,
or circuit qualification, and it does not alter ordinary Pokémon RNG.

All existing circuit rules remain in its PRD, including uniqueness within each
lineup, TR-first home/visitor selection, world point-indexed shared contender/elite/headliner role bands,
and rotating participation. The circuit owns remaining catalog, qualification,
and balance choices; the seed framework does not decide their values.

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
calendar-based seasons, and converting existing random systems are outside this
initial scope. Player-started circuit editions require no real-world schedule.

## Presentation

No additional new-game choice is required. The root seed is an internal save
property, available to debug and reproduction tooling. A future seed-sharing
interface can be added separately.

Features explain their own stable content through their normal interfaces.
For example, the circuit shows its current edition, saved itinerary, and lineups.
A player-facing edition number describes progression; internal hashes, keys,
and draw counters do not belong in ordinary player flows.

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
- Does completing an edition permit a new circuit while reloads before or after
  registration, losses, and replays preserve the appropriate edition's draw?
- Does each feature still feel coherent and fair under its generated content?

## References

- [Playthrough seed framework specification](../specs/playthrough-seed-framework.md)
- [Seeded Trainer Circuit PRD](seeded-trainer-circuit.md)
- [Circuit pool generation](../specs/circuit-trainer-pool.md)
- [Circuit persistence and progression](../specs/seeded-league-circuit.md)
