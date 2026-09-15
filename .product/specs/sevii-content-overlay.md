# Sevii content overlay

PRDs: [Sevii exploration port](../prds/sevii-exploration-port.md),
[Sevii independent story beats](../prds/sevii-independent-story-beats.md), and
[Sevii Trainer Tower](../prds/sevii-trainer-tower.md)

Implemented: No

## Scope and authority

This specification defines how Wayfarer restores selected FRLG Sevii actors,
Trainers, scripts, rewards, and static encounters on the merged 135-map
exploration port. It owns content selection, source provenance, script linkage,
state isolation, generated audits, and delivery order. The story, ordinary
Trainer, and Trainer Tower specifications own their gameplay behavior.

The current Wayfarer exploration port is authoritative:

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
frozen geography and service baseline and gains three independently selectable
content domains:

| Domain | Owner |
| --- | --- |
| `ordinary_trainers` | Sevii Trainer restoration specification |
| `story` | Sevii independent story specification |
| `trainer_tower` | Sevii Trainer Tower specification |

Each retained object, coordinate event, background event, or map-script handler
records:

- source map and Wayfarer map ID;
- event kind and zero-based source index;
- the complete source event object or a canonical source hash;
- stable local ID, graphics dependency, source script label, and replacement
  `WayfarerSevii_` script label;
- `owner`: `exploration`, `ordinary_trainer`, `story`, or `trainer_tower`;
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
ordinary-Trainer, and Tower modules must not be linked through it. If owner
subdirectories are used, derive Make dependencies from the manifest or emit a
depfile rather than relying on the existing nonrecursive glob.

Copied text and movement data may retain source wording and ordering. Script
control flow must be adapted explicitly to the Wayfarer contracts. Generation
emits a symbol-closure report and fails on an undefined label, an unreviewed
source include, or a call into a prohibited FRLG campaign entry point.

## State and transaction rules

Allocate saved flags and variables in a Wayfarer Sevii namespace. Do not reuse a
same-numbered HNS, Emerald, or standalone-FRLG symbol merely because the source
script used it. The manifest publishes each domain's state table and rejects a
write outside that table.

Use a compact dedicated Wayfarer Sevii state bank, including a dedicated
ordinary-Trainer defeat bitset. Move the existing prerelease exploration flags
into this bank when story state lands; no save migration is required before
release. Publish each cell's owner, initial value, legal transitions, and
consumers, use nonzero constants through normal flag/variable APIs, and keep
`SaveBlock3` within its 1,624-byte bound with compile-time assertions.

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
state, Champion state, or National Pokédex state in every domain.

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
manifest and remains byte-equivalent. A domain can be disabled for development,
but the production Wayfarer build enables all delivered domains together.

## Generated audit

Add a deterministic `wayfarer-sevii-content-audit` target. Its report includes:

- counts by domain, map, event kind, actor role, battle policy, and reward type;
- every selected and explicitly excluded source identity;
- source hashes and generated output identities;
- state reads, writes, ownership, and cross-objective dependencies;
- every Trainer ID, caller, party owner, scaling classification, battle type,
  defeat flag, and outcome route;
- every item/Pokémon transaction and one-time receipt;
- script and asset dependency closure;
- unchanged hashes for the exploration baseline, wild profiles, Birth Island,
  Navel Rock, and standalone products; and
- ROM usage and remaining production reserve.

Fail on source drift, duplicate ownership, an unknown state write, an unowned
battle, a missing receipt, an unavailable dependency, an accidental FRLG script
include, a changed baseline hash, or an unexplained output record.

## Delivery order

1. Land schema-v2, generator, and audit support with no enabled new content and
   no material linked-ROM increase.
2. Restore ordinary Trainers and validate scaling, defeat persistence, and map
   traversal.
3. Add independent story objectives in bounded groups, rebuilding and measuring
   after each group.
4. Add Trainer Tower as a separate facility milestone.
5. Enable all domains together and run full integration and release validation.

The accepted baseline uses 32,865,136 bytes, leaving 165,008 bytes above the
required 512 KiB reserve. Retain exact-base and candidate ROM reports plus
category deltas for every milestone, and keep `__rom_end <= 0x09F80000`. Do not
weaken that reserve or delete accepted gameplay content. If a milestone does not
fit, stop and land a separately reviewed content-preserving storage optimization.

## Validation

Required checks include:

- manifest schema, source-drift, collision, script-closure, state-ownership, and
  transaction-order tests;
- the existing Sevii map and wild-encounter audits unchanged;
- ordinary-Trainer, story-objective, static-encounter, and Trainer Tower tests
  named by their owning specifications;
- serial Wayfarer and supported standalone builds;
- a production-equivalent Wayfarer release with the active ROM reserve; and
- emulator journeys covering first discovery, every battle outcome, reward
  capacity failure, save/reload, blackout, ferry departure, and objective orders.

The final exploration journey must still load all 135 maps and prove that every
unselected FRLG actor and script remains absent.

## References

- [Sevii exploration map port](sevii-exploration-map-port.md)
- [Sevii wild encounters](sevii-wild-encounters.md)
- [Sevii independent story](sevii-independent-story-beats.md)
- [Sevii Trainer restoration](sevii-trainer-restoration.md)
- [Sevii Trainer Tower](sevii-trainer-tower.md)
- [Trainer party scaling](trainer-party-scaling.md)
