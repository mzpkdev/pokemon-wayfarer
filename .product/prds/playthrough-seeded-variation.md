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
  edition permits the next; losing a league competition ends that event and
  requires waiting for another at the same venue, without advancing the edition.
- Resolve edition 1's complete venue order at New Game. Generate and lock only
  the single league competition being entered, using actual live badges/first
  clears. Future participants remain unresolved until their own entry.
- Give each canonical trainer one seeded growth percentage per save, initially
  an integer 90–110%. Keep their authored baseline unchanged; scale later badge
  and first-clear growth together. Gyms and leagues share the same percentage,
  which never rerolls as milestones or competitions advance.
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
for the same competition identity. Candidate availability can change whether that slot
needs a category draw or uses a home-only fallback. Constraints inside one lineup
still apply: selecting a trainer removes that character from its remaining four
places. Cross-league appearances remain possible, with a soft penalty based
on completed appearances at earlier venues in the circuit. The immediately
previous completed competition at the same venue also affects roster weights,
including a lost competition. Pure keys do not imply independent eligibility
or selection inputs.
Neither constraint changes ORDER or raw venue/slot POOL_KIND draws.

### Stable outcomes and meaningful new occurrences

Support these patterns without requiring every future feature to generate all
its content at new game:

| Pattern | Identity and lifetime | Example |
| --- | --- | --- |
| Playthrough-wide choice | One named decision for the entire save | An optional future authored world variant |
| One-time entity choice | Named decision plus a stable content identity | An optional future quest's authored reward variant |
| Repeated event | Named decision, entity, and an explicit progression occurrence | A circuit edition's itinerary; each league competition has its own event identity |

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

### Save-specific trainer growth

Keep each trainer's authored baseline the same across playthroughs, while their
later growth is a little faster or slower in each save. Derive one integer
percentage from 90% through 110% inclusive using the root seed and canonical
trainer identity. The same person has the same percentage in their Gym and
league appearances; aliases, new badges, travel, losses and recurring editions
do not draw another value.

Pin the growth policy when the save starts, before Gyms use it. Values can be
calculated lazily with pure keyed lookups rather than storing a large roster
array. Use a separate TRAINER_GROWTH domain so this addition leaves existing
circuit order and category/person raw keys unchanged. Changed growth can still
legitimately change which people qualify for a new competition.

After existing interpolation of authored badge checkpoints, combine growth
above baseline with first-clear growth, scale that sum, round half up once,
then add the unchanged baseline and cap at TR 80. At the starting world point,
every save has the authored baseline. A 100% trainer follows the previous growth
formula exactly. The accepted 90–110% range still needs balance testing against
the provisional curves/teams. Production home coverage must hold for heterogeneous
modifiers over the full range, including intermediate stage transitions; checking
only both ends or a sample of seeds is insufficient.

### Circuit adoption

At New Game, save the root and edition 1's entire venue order. The waiting state
has no active competition or participant lineup; future venues remain unresolved.
The itinerary can be inspected without generating trainers.

Only entering an available single league competition captures and locks its five
trainers and teams. Use actual current badges and lifetime venue first clears,
with each trainer's personal growth curve and fixed save-specific percentage. Do not project future clears or badges.
Milestones earned between competitions legitimately affect new teams. Player
party/TR/XP, event ordinal and edition number do not increase trainer strength.
The balance explorer's numeric curves remain provisional.

The seeded order identifies the circuit traversal. Each competition has a stable
identity combining circuit edition, venue and competition ordinal. The runtime's
persisted availability rule must allocate it once; opening signup, cancelling,
previewing or reloading cannot produce another identity. Exact signup/entry gates
(D4) and the waiting/availability mechanism (D6) are undecided. First entries near 8/16/24
badges are tentative guidance, not adopted thresholds. No real-time/calendar cadence has been chosen.

Check trainer TR/stage/content eligibility first, then classify authored
`homeLeagues`. When both pools exist, choose home with 85% probability and visitor
with 15%, then use positive rotation weights inside that category. Empty visitors
means home; missing homes is invalid content. All venues share authored role
bands at the same actual world point. Prove home-only two-contender/two-elite/
one-headliner feasibility and variety at every supported entry badge/clear point,
including intermediate badges, heterogeneous full-range growth modifiers, stage
transitions and saturation. Concrete endpoints/content remain
D3 decisions; no outsider exclusion, forced guest or automatic band widening exists.

Rotation considers the immediately previous completed competition at this venue,
and participation at distinct earlier other venues during the current circuit.
Both wins and losses count as completed fields. Store bounded latest fields and
per-venue participation sets, not every historical team. Positive proposed factors
keep repeats possible; no person is exhausted for future circuits. Roughly two
returning and three changed opponents is a soft target, never a quota or reroll.

Save the active event's five identities, actual captured milestones, effective
TR, fixed growth percentages/policy, stages/profiles, versions and pre-entry history before revealing opponents.
Their plans remain fixed through that event and save reconstruction. Loading
before entry reproduces the same field for identical identity and inputs;
loading afterward reads its saved plan. Future venues have no locked participants.
Corrupt active snapshots cannot trigger regeneration.

Losing ends that competition. Retire its active snapshot, record the completed
field in participation history and keep the player at the same circuit venue,
waiting for the next available competition. There is no immediate retry or replay
of the lost event. A later available competition gets a fresh stable ordinal and
uses then-current milestones/history; it may still select the same trainers.
Winning ends the event, updates history and first-clear accounting, and advances
to the next venue. Only completing the three-venue traversal permits a new edition.
Reloads, menus and arbitrary registration calls cannot skip either event or venue.

ORDER keeps rules version 1. POOL_KIND version 2 and ROSTER version 3 include
edition/venue/event ordinal in their keys; a new explicit competition schema
replaces whole-edition snapshots. Pure verification reproduces only the active
field from its captured inputs. Preserve the shared root and ordinary Pokémon RNG.
Growth policy has its own version and does not change those circuit rules.
The circuit owns remaining availability, qualification, content and tuning choices.

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
converting existing random systems and a generic calendar are outside this
initial scope. The circuit's competition availability/waiting design remains open;
this framework imposes no real-world schedule.

## Presentation

No additional new-game choice is required. The root seed is an internal save
property, available to debug and reproduction tooling. A future seed-sharing
interface can be added separately.

Features explain their own stable content through their normal interfaces.
For example, the circuit shows its current edition, saved itinerary, competition availability
and the entered event's lineup.
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
- Does loss clearly end the competition and require waiting for the next, while
  reloads of the same event preserve its lineup? Can players understand why later
  entries reflect milestone growth and why menus cannot manufacture another event?
- Does completing the traversal permit a new edition while preserving bounded
  participation history, lifetime first clears and the shared seed?
- Do faster/slower growth percentages create variety without changing starting
  baselines or causing missing eligible home fields? Do Gym and league appearances
  of the same person clearly share that growth?
- Does each feature still feel coherent and fair under its generated content?

## References

- [Playthrough seed framework specification](../specs/playthrough-seed-framework.md)
- [Seeded Trainer Circuit PRD](seeded-trainer-circuit.md)
- [Circuit pool generation](../specs/circuit-trainer-pool.md)
- [Circuit persistence and progression](../specs/seeded-league-circuit.md)
