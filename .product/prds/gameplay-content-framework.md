# Shared gameplay content framework

## Intent

Make gameplay additions fit Wayfarer's connected world without requiring each
feature to maintain its own inventory of trainers, NPCs, locations, and progression
facts. Authors should describe an interaction once, and existing systems should
apply their documented behavior consistently across regions.

Players should keep the same encounters, difficulty, shop specialties, rewards,
and story outcomes during adoption. The benefit is fewer omissions when content
is added, less repeated authoring, and lower linked ROM usage after migration.

## Design

### Shared facts with domain-owned behavior

Use a shared content compilation layer and small gameplay APIs. Reuse existing
map, trainer, species, item, and script sources. Add explicit declarations only
for meaning those sources cannot express, such as an NPC's service or a future
encounter profile's explicitly approved continuation.

Each fact has one authoritative owner. A trainer's roster, a future encounter
profile's story consequences, and a map's region are separate facts. Features may
interpret shared facts differently, but must not maintain competing copies of them.

Resolve routine cases through named profiles and explicit local differences.
Keep local specialties, ecological choices, iconic teams, dialogue, and story
continuations authored. Generation must not infer story intent from trainer
class, graphics, map names, or an absence of known exceptions.

### Required first adoption

| Area | Authoring outcome | Player outcome |
| --- | --- | --- |
| Progression | One definition for each named Rating curve, consumed by gameplay and audits | Existing levels, rounding, offsets, and timing remain exact |
| NPC services | One declaration per rod contributor or converted mart interaction; membership and bindings derive from it | All currently supported regional contributors work, rewards remain repeat-safe, and towns retain their stock |
| Encounters (future) | Separately approved shared encounter and story profiles supply only their declared behavior | Each adopted encounter retains its explicitly validated consequences |

The first adoption includes the shared compiler, progression consumers, all
existing active Standard Rod contributors in supported builds, and all currently
converted Wayfarer mart bindings. These are required acceptance phases, not
interchangeable demonstrations.

Encounter and story profiles are future authored capabilities. Each requires its
own product decision, explicit data contract, behavior validation, and resource
measurement; none is a prerequisite for the current adoption. The current
trainer-only mechanic is separate: it changes eligible wild encounters only when
an author sets its flag and the player party count is exactly zero. It supplies no
trainer, NPC, story, refusal, deferral, loss, or recovery policy to this framework.

### State has a defined meaning and lifetime

Physical region, source-game namespace, starting origin, visited locations,
regional campaign state, and active League run remain separate concepts.
Geography cannot determine which persistence bank a script uses. Total badges
cannot substitute for a story completion flag or League admission rule.

Each consumer declares when it samples changing state. A mart resolves its stock
when opened. A trainer party uses its encounter snapshot. A League uses its saved
admission Rating throughout the run and samples again on a new admission.

### Adding content

Adding a rod giver using an existing service requires one local declaration and
its normal interaction script. It must not require another contributor whitelist,
counter implementation, region list, or hand-authored test inventory.

Adding a mart using an existing profile requires one binding. A new local stock
choice belongs to that profile's authored data. Common stock thresholds remain
shared.

Adding a future encounter through a supported template will require its identity,
profile, and unique dialogue or continuation data. Unsupported legacy scripts
remain explicitly classified until reviewed; discovering a script does not
authorize a new outcome for it.

## Boundaries

This work does not change Rating production, League rewards, badge access,
feature enablement, encounter odds, rosters, learnsets, catalog membership, or
dialogue. Existing feature specifications continue to own those decisions.
Where a specification describes pending behavior, migration preserves the
selected production baseline and records that difference rather than silently
implementing the pending change.

The first adoption does not include a generic runtime rule interpreter, a new
scripting language for arbitrary logic, a universal object table, procedural
ecology, automatic story classification, or a rewrite of all legacy scripts.

Species-fact sharing, acquisition and reachability queries, travel route profiles,
and additional reward services are extension directions. They require their own
consumer contracts and measurements before production migration. Existing
Pokédex, effective encounter, field-move, and origin APIs remain authoritative.

[Sevii exploration, PR #92](https://github.com/mzpkdev/pokemon-wayfarer/pull/92),
is a concrete later adoption candidate for regional content imports and consistent
tooling. Shared inventory must describe the resulting Wayfarer maps after source
events are removed or replaced, while keeping the reviewed import choices and
source provenance. Its fixed-stock shops, healing/PC/daycare services, environmental
scripts, and ferries keep their existing behavior until an appropriate domain
adapter is specified. V1 does not convert those shops to TR stock or require the
Sevii port to wait for this framework. Its trainer-free maps do not imply
encounter-profile adoption.

## Interactions

Preserve randomizer behavior, challenge rules, authored boss exclusions, rematch
identity, standalone builds, and regional state isolation. A context eligible for
trainer scaling is not automatically eligible for a field defeat return.

Service completion must retain existing transaction behavior. Failed rod upgrades
must not consume a contribution; repeat visits must not grant another upgrade;
registered shortcuts and tutorial follow-up must survive.

Any future encounter or story profile must define and validate its own outcome
policy. The current trainer-only wild mechanic has no trainer or story outcome
policy; it must not be treated as a source of refusal, deferral, loss, recovery,
or continuation behavior.

## Constraints

- Generated audit reports are development artifacts. Their line counts are not
  measurements of ROM usage or manual authoring.
- Every runtime adoption must compare equivalent linked builds, including code,
  data, alignment, and retained legacy paths. The complete first adoption must
  reduce used ROM relative to its equivalent baseline. No KiB saving is promised
  before that measurement.
- Host-only infrastructure must emit no new runtime payload. Runtime phases must
  not increase used ROM, static EWRAM, or static IWRAM against their phase baseline.
  Existing ROM capacity and protected-reserve gates remain mandatory.
- New queries use bounded work and caller-owned or existing scratch storage.
  Adoption must not introduce heap allocation, persistent caches, new save fields,
  or extra per-frame whole-world scans.
- New prerelease save migrations are not required. The first adoption preserves
  existing persistent fields and contributor flags because it changes ownership
  and representation rather than player state.
- A feature extension must add only facts or policy it owns. Shared infrastructure
  must allow independent feature switches and product-specific output.

## Acceptance

The first adoption is complete when all required phases pass the specification's
behavior, authoring, resource, and build checks. Generated views must replace
superseded manual inventories for the migrated facts. Keeping both writable
copies does not satisfy this design.

An authoring exercise must add and remove a temporary giver and bind a mart to an
existing profile without editing consumer membership tables or runtime dispatch.
These fixtures do not add shipped gameplay content. A future encounter-profile
adoption must define its own authoring exercise and reject invalid or contradictory
declarations.

Representative journeys must preserve mixed-region rod upgrades, mart stock at
thresholds, and League save/load stability. A future encounter-profile adoption
must add representative journeys for every behavior it owns. Mechanical
equivalence establishes preservation; playtesting checks that transitions,
dialogue, and interaction timing remain coherent.

## References

- [Implementation specification](../specs/gameplay-content-framework.md)
- [Historical evidence and alternatives](../research/gameplay-content-framework.md)
- [Global TR Poké Marts](../specs/global-tr-pokemarts.md)
- [Standard Rod fishing](../specs/standard-rod-fishing.md)
- [Trainer party scaling](../specs/trainer-party-scaling.md)
- [League scaling](../specs/league-scaling.md)
- [Trainer-only wild mechanic boundary](../specs/trainer-only-story-encounters.md)
