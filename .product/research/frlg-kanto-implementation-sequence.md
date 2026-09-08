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

Each milestone needs a bounded specification before code, followed by relevant validation, native critic review and a commit. Later rows may be split further after inspecting their code.

1. Measure the current release budget and inventory dependencies. Adapt settled Machine Part chronology without changing its quest state or choosing a new Power Plant layout.
2. Integrate persistent Anne interiors and their state, direct boarding/return, Ticket handling and the appended Vermilion menu entry. Revise the exact-menu audit together with the working feature. Do not expose a menu option with an absent destination. A required following Bill-rescue submilestone integrates his rescue and no-duplicate Ticket reward after the Bill/grandfather placement decision. His rescue remains playable for existing Ticket holders. Anne boarding ships independently of that placement decision and accepts existing Tickets without Bill completion.
3. Port intact FRLG Cinnabar and its interiors, including Mansion/Key/Blaine and fossil services. Validate HNS route seams in Porymap, healing, Fly, exits and release cost.
4. Integrate Celadon Hideout and independent Silph investigations with isolated state and recoverable rewards. Add Tower/Fuji/physical Flute after radio services and the retained Vermilion Snorlax are reconciled, including placement and keeping radio awakening distinct from the Route 12/16 physical-Flute encounters. Ordinary Silph services follow liberation; Steven reward design stays pending.
5. Integrate remaining local adventures and retained HNS adaptations, including Safari/Teeth, Mt. Moon, burglary and Nugget Bridge. Specify event collisions and reward ownership individually.
6. Integrate Giovanni finale, one Earth Badge, Blue chapter retirement and separate League lineups. Resolve party/leadership choices before dependent content. Add Celebi gates using actual Giovanni and relevant Goldenrod completion, with adapted time/identity dialogue.

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
return-tile walkability and an ordinary exit path still need verification.

`game/tools/mapjson/mapjson.cpp` currently selects only HNS and Emerald
sources for Wayfarer. A named selective FRLG manifest must preserve existing
map IDs and close all included warps without enabling the entire FRLG catalog.
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

The exact Vermilion menu audit remains unchanged until the complete boarding
feature is implemented. The feature must keep original indices and all old
eligibility checks while making Anne independently reachable with the Ticket.


The Machine Part chronology milestone also passes the same release command:
32,888,848 used bytes, `__rom_end = 0x09F5D810`, and 665,584 unused bytes.
The reported scripts category decreases by 104 bytes and graphics increases
by 8 bytes, for a net reduction of 96 bytes. Other category sizes are unchanged. The remaining margin above reserve is 141,296 bytes.
Standalone HNS Route 24 preprocessing is identical to the baseline, Wayfarer
executable commands are identical, and all 24 existing HNS traversal tests pass.
This validation covers the dialogue substitution, not an emulator playthrough
or the unimplemented story imports.
