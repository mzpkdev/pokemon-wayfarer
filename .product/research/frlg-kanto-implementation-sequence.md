# FRLG Kanto implementation dependencies

This inventory applies to the merged [story contract](../prds/frlg-kanto-story-on-hns-maps.md) at `7aa8db0557`. The PRD selects behavior; it is not an implementation specification. The [conflict evidence](frlg-hns-kanto-story-conflicts.md) distinguishes accepted adaptations from recommendations.

## Existing specifications

| Owner | Dependency retained by the story port |
| --- | --- |
| [Runtime foundation](../specs/wayfarer-runtime-foundation.md) | Stable composite map IDs, isolated persistent state, active regions, save bounds and release reserve. |
| [FRLG traversal](../specs/frlg-open-world-region-traversal.md) | Standalone FRLG source behavior, including temporary Anne and its Route 12 bypass; not authority to import that dock, departure or exact route geometry into HNS Wayfarer. The new story PRD owns the adaptations. |
| [Trainer-only core](../specs/trainer-only-encounters.md) and [story encounters](../specs/trainer-only-story-encounters.md) | Pending usable-party entry and explicit battle-outcome policies; integrate or coordinate these before enabling imported story battles. |
| [HNS traversal](../specs/hns-open-world-region-traversal.md) | Ordinary travel, Machine Part repair, Misty return, maiden voyage and existing service eligibility. |
| [Aqua entry](../specs/wayfarer-hoenn-entry.md) | Directional circuit; approved future slot 6 Anne addition preserves slots 0–5 and each old service gate. |
| [League circuit](../specs/wayfarer-interregional-league-circuit.md) | Admission, state, recovery and rewards; approved future separate lineups and Giovanni initial badge ownership. |
| [Regional starts](../specs/wayfarer-regional-start-choice.md) | Supported Johto/Hoenn origins and visitor identity; Kanto opening remains separate. |
| [Trainer scaling](../specs/trainer-party-scaling.md), [Gym scaling](../specs/gym-leader-scaling.md), [League scaling](../specs/league-scaling.md) | Imported battles use the appropriate existing scaling consumer. |
| [HM use](../specs/hm-field-use.md) | Existing field-use rules do not consume or auto-complete local adventure rewards. |
| [Map storage](../specs/compressed-map-layout-build-storage.md), [runtime loading](../specs/compressed-map-layout-runtime-loading.md), [rollout](../specs/compressed-map-layout-validation-rollout.md) | Separate space-recovery work, not authority to implement compression in this port. |

The [independent-story PRD](../prds/frlg-kanto-independent-story-beats.md) supplies local adventure dependencies. Its recoverable earlier Blue chapters and temporary Anne departure are superseded by the merged story contract. No FRLG Kanto story implementation specification existed at the inspected base. Existing source maps and scripts do not establish their inclusion or safety in Wayfarer.

## Sequential milestones

Each milestone needs a bounded specification before code, followed by relevant validation, native critic review and a commit. Later rows may be split further after inspecting their code. At the user’s request, Cinnabar is last; other settled story work proceeds first.

1. Measure the current release budget and inventory dependencies. Adapt settled Machine Part chronology without changing its quest state or choosing a new Power Plant layout.
2. Integrate persistent Anne interiors and their state, direct boarding/return, Ticket handling and the appended Vermilion menu entry. Revise the exact-menu audit together with the working feature. Do not expose a menu option with an absent destination. A required following Bill-rescue submilestone integrates his rescue and no-duplicate Ticket reward after the Bill/grandfather placement decision. His rescue remains playable for existing Ticket holders. Anne boarding ships independently of that placement decision and accepts existing Tickets without Bill completion.
3. Integrate Celadon Hideout and independent Silph investigations with isolated state and recoverable rewards. Add Tower/Fuji/physical Flute after radio services and the retained Vermilion Snorlax are reconciled, including placement and keeping radio awakening distinct from the Route 12/16 physical-Flute encounters. Ordinary Silph services follow liberation; Steven reward design stays pending.
4. Integrate remaining local adventures and retained HNS adaptations, including Safari/Teeth, Mt. Moon, burglary and Nugget Bridge. Specify event collisions and reward ownership individually.
5. Integrate Giovanni finale, one Earth Badge, Blue chapter retirement and separate League lineups. Resolve party/leadership choices before dependent content. Add Celebi gates using actual Giovanni and relevant Goldenrod completion, with adapted time/identity dialogue.
6. Port intact FRLG Cinnabar and its interiors, including Mansion/Key/Blaine and fossil services. Validate HNS route seams in Porymap, healing, Fly, exits and release cost.

The user approved content implementation before separately owned space recovery. Track measured usage throughout development and keep the production reserve check intact. Recovery and a passing release budget are merge prerequisites: rebase this task on recovery before merging. Do not implement unrelated optimization or reduce agreed content to fit the current margin.

## Product decisions still required

Steven’s reward, Koga/Janine roles, radio placement, Power Plant exploration layout, and the retained Vermilion Snorlax arrangement remain unresolved. Blue’s visitor Champion party and future starter branches also need specification. Bill/grandfather placement and Master Ball/legendary readiness require bounded decisions before their dependent implementation.

Anne travel routes are separately scoped and do not block local boarding. Kanto opening and Blue’s later Gym succession are outside this feature. Settled content can proceed without choosing these deferred features.

## Budget evidence

The stored accepted report records 32,865,136 used bytes and `__rom_end = 0x09F57B70`, leaving 689,296 unused bytes, of which 524,288 are reserved. Its 165,008-byte margin is historical, not a measurement of this task revision. The authoritative current measurement is a Wayfarer release build and its linker-derived size report, not the padded ROM file size. Space-recovery specifications alone do not establish recovered bytes.


Current task baseline: `make -C game BUILD=wayfarer release -j4` passes at
`7aa8db0557` with GCC 13.2.1 and default release LTO. The report gives
32,888,944 used bytes, `__rom_end = 0x09F5D870`, 665,488 unused bytes and
141,200 bytes above the enforced reserve. This is 23,808 bytes larger than
the stored accepted report. EWRAM is 248,557 bytes and IWRAM 25,616 bytes.
The release emits existing compiler/linker warnings; this measurement does
not claim a warning-free baseline.

No documented recovery project is implemented at this revision. Recovery is
now a merge prerequisite rather than an implementation blocker, following the
user’s approval. The measurement alone does not prove that any particular full
import exceeds the budget. Measure content deltas during development, retain
any failing production-budget result, and revalidate after rebasing on recovery.

## Anne integration findings

The existing ship consists of 25 interior maps plus `SSAnne_Exterior_Frlg`.
The interior graph's two external exit warps are in `SSAnne_1F_Corridor_Frlg`.
The exterior owns the old departure presentation and must not become a second
Vermilion dock. Boarding and both exits need explicit adapted destinations;
the selected return tile `(8, 9)` and its path to the public exit are now covered by static collision/event checks and save/reload emulator coverage.

`game/tools/mapjson/mapjson.cpp` now also accepts explicit Wayfarer opt-ins
for the 25 Anne maps and nine layouts. Catalog tests check preserved source
slots and closed included warps without enabling the entire FRLG catalog.
The mixed-layout renderer supports FRLG layout metadata, but that does not
establish that all required tilesets and scripts are linked correctly.

The FRLG ship's ordinary Trainer IDs are not Wayfarer Trainer IDs. Its item
flags and captain flag also cannot be copied into the HNS namespace, where
several FRLG constants are zero. The import must allocate distinct persistent
reward state and battle identities, include the ordinary parties under existing
scaling rules, and validate every item pickup. Optional Trainers and ordinary
items remain part of the ship adventure; omitting them is not an accepted
way to make the milestone smaller.

The captain needs a separate one-time reward transaction, including capacity
failure and prior Cut ownership, without setting FRLG Vermilion departure
state. Blue's three source corridor triggers must not start rival battles for
visitors. An authored visitor introduction and future Kanto chapter integration
need explicit scene ownership; map discovery cannot advance Blue's chapters.

The exact Vermilion menu audit now checks the appended Anne slot, original
indices, and each old service eligibility check. Its 16 tests pass; standalone
HNS retains its original six-slot expectation.


The Machine Part chronology milestone also passes the same release command:
32,888,848 used bytes, `__rom_end = 0x09F5D810`, and 665,584 unused bytes.
The reported scripts category decreases by 104 bytes and graphics increases
by 8 bytes, for a net reduction of 96 bytes. Other category sizes are unchanged. The remaining margin above reserve is 141,296 bytes.
Standalone HNS Route 24 preprocessing is identical to the baseline, Wayfarer
executable commands are identical, and all 24 existing HNS traversal tests pass.
This validation covers the dialogue substitution, not an emulator playthrough
or the unimplemented story imports.


The final Anne release passes with default release LTO: 33,007,476 used bytes,
`__rom_end = 0x09F7A774`, 546,956 unused bytes, and 22,668 bytes above the
mandatory reserve. This adds 118,628 bytes relative to the Machine Part milestone. The implementation includes the
selected General FRLG and Anne tilesets, all required NPC graphics, the ship door
animation, and valid ship entries in both HNS region-map tables. Eight map-catalog
tests, 29 trainer-scaling generator tests, five native trainer-state tests, ten
Anne emulator cases, and two existing Aqua emulator journeys pass. The native
critic review is clear; E2E lint, type checking, and changed-file formatting pass.


## Celadon Hideout integration findings

The five FRLG interiors use the BuildingFrlg primary and SilphCo secondary
assets. The HNS Game Corner entrance uses a concealed wall panel and source
grunt in existing free space; the source B1 stair exits west and returns to the
panel approach. No layout binary changed. Twelve ordinary trainer entries use
an exact sight resolver so their local entry checks retain normal defeated-trainer
suppression. Separate visibility and receipt flags keep Lift Key and Scope
handoffs retryable, and the elevator's dedicated variable leaves Faraway Island
state untouched.

Fourteen Hideout and ten Anne emulator cases, six native trainer-state tests,
nine catalog tests, 15 trainer-scaling tests and 16 harbor-menu tests pass. The
production release links at 33,067,852 used bytes (`__rom_end = 0x09F8934C`),
60,376 bytes above Anne. Its unchanged reserve check fails by 37,708 bytes,
with 486,580 physical bytes free. EWRAM and IWRAM remain 248,557 and 25,616
bytes. Development continues under the user waiver; recovery and rebase remain
merge prerequisites.


## Fuchsia Safari integration findings

Surf and Gold Teeth occupy opposite accessible branches of the existing HNS
Beach habitat. Baoba visits his former Fuchsia house and exchanges the Teeth
for Strength; a full HM pocket leaves the Teeth with the player. Three local
receipt flags preserve existing HNS research, Safari levels, paid admission,
Steven's welcome, and the other Strength reward sources. No maps, layouts,
assets, or runtime engine code were added.

All 11 Safari emulator cases and five static tests pass, including real paid
sessions in both objective orders, the initial Steven/Pokeblock Case scene,
full-pocket retries, pre-owned rewards, and save/reload. The pre-rebase release
uses 33,069,244 bytes (`__rom_end = 0x09F898BC`), 1,392 bytes above Hideout.
It leaves 485,188 physical bytes free and fails the normal reserve check by
39,100 bytes. The user requests a rebase onto latest main after this milestone's
commit, followed by a fresh combined release measurement.

## Release after recovery rebase

The five completed task commits rebased cleanly onto main `1e2c5c3122`,
which includes Surf pixel deduplication and legacy multiboot removal.
`git range-diff` confirms all five patches are unchanged by the rebase.
The combined production release passes with the normal reserve enforced:
32,429,816 used bytes (`__rom_end = 0x09EED6F8`), 1,124,616 physical bytes
free, and 600,328 bytes above the 512 KiB reserve. This recovers 639,428
bytes relative to the pre-rebase Safari release. EWRAM uses 248,509 bytes
and IWRAM uses 25,556 bytes.

The space-recovery prerequisite is satisfied for this revision. Further
content still needs its own release measurement; this result does not
authorize merging or establish the cost of unimplemented imports.

## Mt. Moon fossil integration findings

The selected HNS cave retains its 15 original object templates, four exits,
Silver interaction, hidden Revive, sign, isolated fossil tableau, and layout
binary. Wayfarer appends four optional Rocket grunts, Miguel, and two local
fossil objects only through event selection. The largest relevant spawn window
contains 12 non-player templates, within the 15-template capacity.

The normal Items pocket owns the Wayfarer Helix and Dome transaction while
standalone HNS retains its configured Key Item metadata. Miguel's win reveals
both loaded fossil objects immediately; a full Items pocket or a selected
pre-owned fossil keeps the local choice safe and retryable. The HNS Ruins of
Alph rewards and revival state remain independent.

The focused static map/state contract, catalog and scaling checks, TypeScript
check, and eleven-case emulator journey pass. The journey covers Miguel's
empty-party refusal, win with immediate fossil interaction, loss/retry, both
fossil choices, Exit, full-pocket retry, pre-owned reconciliation, and Grunt
1's normal-sight, completed-sight, and empty-party paths.
The production release uses
32,433,956 bytes (`__rom_end = 0x09EEE724`), leaves 1,120,476 physical bytes
free, and remains 596,188 bytes above the 512 KiB reserve. EWRAM uses 248,509
bytes and IWRAM uses 25,556 bytes. Existing compiler/linker warnings remain
outside this milestone.

## Cerulean burglary integration findings

Cerulean keeps the HNS city, household, and unrelated story state. Its selected
FRLG robbed-house interior preserves the source rubble and rear hole; all
three front warp records return to the HNS city, while the source center tile
is the active front exit. The Wayfarer city clone appends a stable layout ID.
Porymap changed only the four cells at x34..35, y17..18 to adjacent passable
grass, opening the rear approach without changing the original HNS layout.

One ordinary Rocket battle and one local receipt flag own the Dig reward.
Dig retains the current consumable-TM policy: another source's copy does not
suppress this episode's reward. A full pocket preserves the defeated Rocket
for delivery retry, and receipt hides him only after the local copy is granted.

The production release uses 32,446,992 bytes (`__rom_end = 0x09EF1A10`),
13,036 bytes above Mt. Moon. It leaves 1,107,440 physical bytes free and
583,152 bytes above the unchanged 512 KiB reserve. EWRAM uses 248,509 bytes
and IWRAM uses 25,556 bytes.

All 14 Cerulean emulator cases pass, together with 18 existing Kanto
traversal cases and the Machine Part/Misty/Magnet Train journey. The 35-test
map-generator suite, trainer-scaling checks, two focused native trainer tests,
TypeScript checks, and lint pass. Native critic review is clear. Rear-route
and interior screenshots confirm the new clearing and preserved household.
