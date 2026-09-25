# Sevii content overlay

PRDs: [Sevii exploration port](../prds/sevii-exploration-port.md),
[Sevii independent story beats](../prds/sevii-independent-story-beats.md),
[Sevii Trainer Tower](../prds/sevii-trainer-tower.md), and
[Interregional League circuit](../prds/wayfarer-interregional-league-circuit.md)

Implemented: Partial — foundation in [PR #101](https://github.com/mzpkdev/pokemon-wayfarer/pull/101), with the ordinary Trainers, story, and Trainer Tower delivered in [PR #105](https://github.com/mzpkdev/pokemon-wayfarer/pull/105), [PR #104](https://github.com/mzpkdev/pokemon-wayfarer/pull/104), and [PR #102](https://github.com/mzpkdev/pokemon-wayfarer/pull/102). The Masters domain is approved but pending.

## Scope and authority

This specification defines how Wayfarer restores selected FRLG Sevii actors,
Trainers, scripts, rewards, and static encounters on the merged 135-map
exploration port. It owns content selection, source provenance, script linkage,
state isolation, generated audits, and delivery order. The story, ordinary
Trainer, Trainer Tower, and interregional circuit specifications own their
gameplay behavior.

The current Wayfarer exploration port is authoritative. The interregional
circuit may add the explicitly scoped Masters Challenge event layer described
below without changing these access rules:

- One through Seven Island remain available from Vermilion without Bill,
  Celio, a Rainbow Pass, a League clear, or another story credential.
- Every numbered-island harbor retains all seven islands and Vermilion.
- The existing maps, layouts, connections, wild encounters, healing, PCs,
  Marts, Day Care, and environmental puzzles remain unchanged unless another
  specification names an exact extension.
- Birth Island and Navel Rock remain separate existing Wayfarer features.

Do not restore the older Seagallop shakedown, temporary PC shutdown, Ruby travel
gate, National Pokédex gate, or Cinnabar forced trip. Do not include the complete
FRLG event-script graph or make arbitrary FRLG maps available in Wayfarer.

## Map manifest and content domains

Upgrade `game/src/data/wayfarer_sevii_maps.json` to schema version 2 and keep it
as the only map-event projection boundary. The same file continues to own the
frozen geography and service baseline and gains four independently selectable
content domains:

| Domain | Owner |
| --- | --- |
| `ordinary_trainers` | Sevii Trainer restoration specification |
| `story` | Sevii independent story specification |
| `trainer_tower` | Sevii Trainer Tower specification |
| `masters` | Interregional League circuit specification |

Each retained object, coordinate event, background event, or map-script handler
records:

- source map and Wayfarer map ID;
- event kind and zero-based source index;
- the complete source event object or a canonical source hash;
- stable local ID, graphics dependency, source script label, and replacement
  `WayfarerSevii_` script label;
- `owner`: `exploration`, `ordinary_trainer`, `story`, `trainer_tower`, or
  `masters`;
- a stable `content_id` that resolves exactly once in the owner's inventory;
- visibility predicate and permanent hide or defeat flag, when applicable;
- Trainer ID, scaling policy, battle type, and outcome policy for battles;
- state read/write set, item or Pokémon transaction, and reward receipt;
- required source text, movement, trainer, sound, and graphics dependencies; and
- an explicit reason for every adaptation from FRLG behavior.

Map scripts and non-object events use equivalent records. A retained source
identity must match exactly; an index alone is insufficient. Generation fails
when a selected source event changes, disappears, or moves until the manifest is
reviewed.

The domain inventories live beside the projected records in the same manifest;
there is no second event injector. The manifest also contains explicit
exclusions for source actors or scripts that
look selectable but remain outside Wayfarer. At minimum, record exclusions for
old travel gates, PC shutdown, Pokédex/Champion side effects, FRLG multiplayer,
event-island content, and e-Reader-only Trainer Tower data.

## Composition rules

Extend the existing map adapter to compose enabled content domains with the
sanitized events it already produces.

1. The exploration manifest emits the existing baseline.
2. Enabled content-domain records add reviewed events and map scripts.
3. Two records may not own the same source event, local ID, coordinate trigger,
   background event, or map-script type.
4. A deliberate replacement names the displaced baseline record and proves that
   its service, warp, or environmental behavior remains reachable.
5. Story visibility changes use content-owned predicates. They do not overwrite
   ferry, healing, wild-encounter, or environmental state.
6. Domain order does not change output. Conflicts fail generation rather than
   resolve by last writer.

Allow only these Wayfarer overrides in the first content port:

- object event: `script` and visibility `flag`;
- coordinate event: `script`, `var`, and `var_value`; and
- background event: `script`, plus `flag` for a hidden item.

The later `masters` domain adds one narrow map-level override: Room 1 may
select between its existing closed-box and open-box source layouts from the
circuit eligibility predicate. The manifest records both source layout IDs and
the displaced baseline selection. It does not edit layout bytes or authorize a
general layout override for another domain.

Do not override coordinates, graphics, movement, sight radius, item, quantity,
or local ID. Do not author a new actor without exact source-event identity. A
spatial or cast change requires a reviewed amendment to this specification.

Do not edit FRLG source map JSON to make an event visible only in Wayfarer.
Manual changes to the map manifest are allowed. Layout changes remain subject
to the repository's Porymap-only rule.

## Script ownership and linkage

Continue generating the single `game/data/wayfarer_sevii_event_scripts.inc`
artifact from the map manifest and include it only under `IS_WAYFARER`.
Keep authored implementations under:

```text
game/data/scripts/wayfarer_sevii/story/
game/data/scripts/wayfarer_sevii/trainers/
game/data/scripts/wayfarer_sevii/trainer_tower/
game/data/scripts/wayfarer_sevii/masters/
```

The generator owns the one `SourceMap_Frlg_MapScripts` table for each map from
declarative handler rows. Owner modules export handlers, text, movement, and
data labels; they do not emit competing map tables. Reject duplicate handler
types unless an explicitly ordered form is supported. Every selected source
label maps to a Wayfarer-owned entry point. Small shared
helpers may be called directly only when their complete command closure is
available in Wayfarer and contains no unowned state, battle, reward, warp, or
campaign dispatch. Do not widen `.if IS_FRLG` around source map or campaign
scripts.

Reserve the root `common.inc` for universal exploration primitives; story,
ordinary-Trainer, Tower, and Masters modules must not be linked through it. If owner
subdirectories are used, derive Make dependencies from the manifest or emit a
depfile rather than relying on the existing nonrecursive glob.

Copied text and movement data may retain source wording and ordering. Script
control flow is adapted to the Wayfarer contracts. Generation fails on an
undefined label, an unreviewed source include, or a call into a prohibited FRLG
campaign entry point; runtime behavior is covered by focused mechanics and
emulator tests rather than a second script interpreter.

## State and transaction rules

Allocate saved flags and variables in a Wayfarer Sevii namespace. Do not reuse a
same-numbered HNS, Emerald, or standalone-FRLG symbol merely because the source
script used it. The manifest publishes each domain's state table and rejects a
write outside that table.

Use a compact dedicated Wayfarer Sevii state bank, including a dedicated
ordinary-Trainer defeat bitset. The delivered state is initialized as a whole;
prerelease save migration is not required. Publish each cell's owner and use
normal flag/variable APIs rather than raw FRLG state. Keep `SaveBlock3` within
its 1,624-byte bound with compile-time assertions.

Use one-way transitions for objective progress and one-time receipts. A later
objective may read an earlier completion state, but it must not reset it.
Explicitly declared reusable transaction payloads, such as Selphy's pending
request/reward and Trainer Tower's pending prize, may follow their owner's
bounded transition graph, including clearing after a successful claim. These
payloads do not reset objective completion or one-time receipts.
Map entry never awards an item, teaches a
password, defeats a Trainer, hides an undefeated actor, or completes an unseen
scene.

Every item or Pokémon handoff follows this order:

1. establish the prerequisite without consuming it;
2. attempt the Bag, party, or PC transaction;
3. consume the handed-in item only when the destination transaction succeeds;
4. set the reward or completion receipt; and
5. update presentation last.

A full pocket, full party, interrupted script, loss, blackout, ferry departure,
save, or reload leaves a recoverable next interaction. Receipts prevent duplicate
one-time rewards. Actual item ownership remains distinct from offer, recovery,
and delivery state.

## Battles and defeat routing

Every Trainer caller declares one of these policies:

| Policy | Use |
| --- | --- |
| `ordinary` | Independent sight or talk Trainer; normal defeat flag and normal blackout |
| `objective_guard` | Victory advances a local objective; loss restores the pending scene |
| `facility` | Trainer Tower-owned battle and facility loss routing |
| `circuit` | Masters Challenge opponent; circuit-owned run and loss routing |

Unknown or callerless battles fail the Wayfarer content audit. Scripted wild and
static encounters declare their own completion and retry rules; they do not enter
the temporary trainer-only wild mechanic unless a separate authored scenario
explicitly enables it.

Owner-specific validation retains the exploration domain's current prohibition
on battles, gifts, statics, and story state. An `ordinary_trainer` entry permits
exactly one reviewed battle and no story write or reward. A `story` entry may
use only battles, transactions, movements, and static encounters declared by
its objective. A `trainer_tower` entry may use only the facility command and
state surface. Reject raw FRLG flags, scene variables, Trainer IDs, travel-pass
state, Champion state, or National Pokédex state in every domain. A `masters`
entry may replace the caretaker interaction, control the two-room battle-house
transition, and call only the circuit-owned Masters admission, run, completion,
and gallery surface. It must not write Champion, Hall of Fame, regional
game-clear, travel-pass, or local Sevii story state.

Selected source Trainer IDs are provenance only. Generate stable
`TRAINER_WAYFARER_SEVII_*` IDs from an explicit fixed base chosen after a
collision audit; never link the complete FRLG Trainer table or derive the base
from a moving Trainer count. Generate dense selected Trainer and party data,
store normalized source hashes, keep IDs below the partner boundary at 2048,
and dispatch their defeat state through the dedicated Sevii bitset before the
existing appended-HNS remap.

Ordinary Trainers enroll in the existing Wayfarer ordinary scaling policy.
Story bosses, rivals, and facility opponents remain excluded from that policy
unless their owning specification defines another existing scaler. No content
record may silently change a Trainer party, AI, prize money, rematch family, or
battle type.

## Assets and isolation

For each owner module, generate a label/reference graph and require every
external helper, special, native function, asset, and script entry to be
allowlisted. Compute the dependency closure of all enabled content. Select only
the object graphics, trainer pictures, palettes, movements, text, music,
parties, items, and specials reachable from the manifest. A dependency being
present in a standalone FRLG build does not make it available to Wayfarer.

Standalone HNS, FireRed, LeafGreen, and Emerald generation ignores the content
manifest. Their supported behavior and ROM-category boundaries remain unchanged;
whole-ROM byte identity is not claimed because LTO can reorder same-address
interworking thunks. A domain can be disabled for development, but the production
Wayfarer build enables all delivered domains together. The `masters` domain
adapts only `SevenIsland_House_Room1_Frlg` and
`SevenIsland_House_Room2_Frlg`; it does not change the frozen 135-map catalog
or add `SevenIsland_UnusedHouse`.

## Structural audit and delivered validation

`wayfarer-sevii-content-audit` is a deterministic structural gate. It checks the
manifest schema, selected source identity, ownership conflicts, script and asset
closure, state allocation, battle routing, protected exploration content, and
the active ROM reserve. It does not reimplement story control flow, reward
delivery, or C-data parsing; focused mechanics and emulator tests cover those
runtime paths.

The delivered integration was merged as `f5d74b7ef5`, with the validated tree
matching Tower integration commit `11bd87be8b`. Its one combined evidence
snapshot is: 8 Tower, 13 shared Sevii, and 2 rematch mechanics tests; 50 content
checks; and 17/17 emulator tests across four journey files. The production ROM
is 32,916,176 bytes, with 638,256 bytes unused—113,968 bytes above the required
512 KiB reserve—and
`SaveBlock3` is 1,112 of 1,624 bytes. See the [integration coordination
record](../../docs/sevii-tower-implementation/coordination.md) for results and
scope. Keep `__rom_end <= 0x09F80000`; no future documentation-only edit needs
to repeat this historical integration build.

## Validation

The stable validation set includes:

- manifest schema, source-drift, collision, script-closure, and state-ownership
  tests;
- the existing Sevii map and wild-encounter audits unchanged;
- ordinary-Trainer, story-objective, static-encounter, Trainer Tower, and
  Masters Challenge tests named by their owning specifications;
- serial Wayfarer and supported standalone builds;
- a production-equivalent Wayfarer release with the active ROM reserve; and
- representative emulator journeys for discovery, failure and retry, save/load,
  ferry travel, and objective order.

The 135-map exploration sweep remains the authority for catalog coverage.
Representative runtime journeys are not a claim that every story branch, every
battle outcome, or every facility format/floor combination has been exercised.

## References

- [Sevii exploration map port](sevii-exploration-map-port.md)
- [Sevii wild encounters](sevii-wild-encounters.md)
- [Sevii independent story](sevii-independent-story-beats.md)
- [Sevii Trainer restoration](sevii-trainer-restoration.md)
- [Sevii Trainer Tower](sevii-trainer-tower.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Trainer party scaling](trainer-party-scaling.md)
