# Gameplay content framework: research

Status: design evidence, 2026-09-09. This investigation supports the [PRD](../prds/gameplay-content-framework.md) and [technical specification](../specs/gameplay-content-framework.md). It does not implement the framework or establish a ROM saving.

## Recommendation

Build a shared host-side content compiler with typed gameplay domain APIs. Existing source content remains authoritative; authors declare missing semantics once, and domain backends generate only the runtime data their consumers need. Start with progression curves, NPC service declarations for rods and marts, and a conservative trainer encounter adapter. Keep species analysis, reachability, and travel as later extensions.

The history supports sharing discovery, identity, validation, and selected operations. It does not support one universal gameplay record or inferring story behavior from trainer class, script names, or map names.

## Sources and limits

The local source baseline is `a5dd5178f098cc3ae80fbca73a59e04da0f6f835`. Historical PR titles, bodies, and merge status were read through GitHub CLI; the source findings below were checked against that baseline. PR descriptions record validation at their own revisions and are not fresh test results.

[PR #85](https://github.com/mzpkdev/pokemon-wayfarer/pull/85) was separately inspected at open head `6799eb7d9f625fefe0c0983104d38e3b13e7d4d5`. Its files are not part of this baseline. Encounter work must reconcile the eventual landed implementation before migration.

The [#85 ROM budget comment](https://github.com/mzpkdev/pokemon-wayfarer/pull/85#issuecomment-5601370188) reports +26.9 KiB used ROM, 30.78 MiB total, and 735.3 KiB before the protected reserve. That is a whole-PR CI measurement, including code, scripts, dialogue, and data. It is not a table-only cost. The earlier conversation's approximately 9.8 KiB registry estimate came from a pre-existing local ELF; this investigation did not reproduce it and does not use it as an acceptance baseline.

## Evidence across additions

| Evidence | What should be reused | Boundary to preserve |
| --- | --- | --- |
| [#13: wild scaling](https://github.com/mzpkdev/pokemon-wayfarer/pull/13) makes ordinary encounter consumers share effective populations. [#16: species floors](https://github.com/mzpkdev/pokemon-wayfarer/pull/16) adds reviewed minimum ordinary wild levels. | Effective-population queries and species relationships. | Fixed/scripted encounters and randomizer behavior have distinct contracts; wild floors are not universal species restrictions. |
| [#19: field HMs](https://github.com/mzpkdev/pokemon-wayfarer/pull/19) consolidates previously divergent eligibility paths. | Typed domain resolvers used by every consumer. | Learned moves, durable HM ownership, compatibility, Eggs, and regional exceptions remain explicit. |
| [#32: Standard Rod](https://github.com/mzpkdev/pokemon-wayfarer/pull/32) shares award transactions and fishing weights across gameplay and tools. [#74: regional fix](https://github.com/mzpkdev/pokemon-wayfarer/pull/74) repairs Hoenn givers omitted from accepted contributors. | Local service declarations generating contributor acceptance and counting from one set. | Distinct giver identity, atomic upgrades, repeat dialogue, tutorial continuation, and registered shortcuts. |
| [#40: runtime foundation](https://github.com/mzpkdev/pokemon-wayfarer/pull/40) generates combined catalogs and source-aware dispatch. | Existing map/trainer identity and source namespaces. | Physical region and source namespace are different; persistent regional state retains its owner. |
| [#41: ecology](https://github.com/mzpkdev/pokemon-wayfarer/pull/41) authors regional encounter profiles with generated audit evidence. [#65: utility catch windows](https://github.com/mzpkdev/pokemon-wayfarer/pull/65) connects learnsets, populations, and acquisition scenarios. | Host inventory, initial-moveset evaluation, and reusable coverage checks. | Populations and utility windows are design choices; coverage alone does not prove practical reachability or balance. |
| [#57: Rating progression](https://github.com/mzpkdev/pokemon-wayfarer/pull/57), [#61: ordinary trainers](https://github.com/mzpkdev/pokemon-wayfarer/pull/61), [#69: Gym Leaders](https://github.com/mzpkdev/pokemon-wayfarer/pull/69), [#72: League scaling](https://github.com/mzpkdev/pokemon-wayfarer/pull/72) combine related progression calculations with distinct roster policies. | Named curve definitions, exact arithmetic, canonical roster references, and structural validation. | Rating production, roster retention, exclusions, feature switches, and snapshot lifetimes are separate policies. #69 shipped disabled pending acceptance work. |
| [#58: regional Pokédex](https://github.com/mzpkdev/pokemon-wayfarer/pull/58) introduces canonical membership with regional views. | Descriptors referencing existing authored data through a facade. | Dex membership, visible catalog, and current catchability answer different questions. |
| [#64: all-badge access](https://github.com/mzpkdev/pokemon-wayfarer/pull/64) separates Gym access from ordered League admission. [#75: regional starts](https://github.com/mzpkdev/pokemon-wayfarer/pull/75) persists origin independently of current region. | Explicit state ownership and eligibility inputs. | Origin, visited state, current geography, badge ownership, and League completion cannot collapse into one region/progress field. |
| [#70: marts](https://github.com/mzpkdev/pokemon-wayfarer/pull/70) composes Rating essentials, town specialties, retained stock, and challenge supplements. | Profiles, bounded composition, and service bindings. | Preserve authored specialties, specialist counters, item ordering, capacity, and fallback behavior. |
| [#85: trainer-only encounters](https://github.com/mzpkdev/pokemon-wayfarer/pull/85) explicitly audits callers and preserves loss/story continuations. | Encounter identity distinct from trainer identity, reviewed outcome contracts, and templates for future content. | Unknown legacy callers must retain existing routing. A trainer classification does not prove that its script can safely return after defeat. |
| [#92: Sevii import](https://github.com/mzpkdev/pokemon-wayfarer/pull/92) projects selected FRLG maps into Wayfarer and resolves encounter sources in Python and TypeScript. | Effective build inventory, import provenance, and one host encounter projection consumed by ROM tooling and Cartographer. | Retain/remove intent and drift guards remain authored and enforced; fixed-stock shops and trainer-free content do not inherit unrelated TR or trainer policies. |

## Regional import and tooling candidate: #92

[PR #92](https://github.com/mzpkdev/pokemon-wayfarer/pull/92) was separately verified OPEN at `85748ebb6731a562b110cfef18034f664518f006`; the Sevii task checkout matched that revision. These findings do not describe the main baseline above.

Its [maps manifest](https://github.com/mzpkdev/pokemon-wayfarer/blob/85748ebb6731a562b110cfef18034f664518f006/game/src/data/wayfarer_sevii_maps.json) selects source content and records retained events and map scripts. The [script generator](https://github.com/mzpkdev/pokemon-wayfarer/blob/85748ebb6731a562b110cfef18034f664518f006/game/tools/wayfarer_sevii_scripts/generate.py) emits Wayfarer script tables and includes reviewed Wayfarer-owned scripts rather than copying FRLG script bodies. The [port audit](https://github.com/mzpkdev/pokemon-wayfarer/blob/85748ebb6731a562b110cfef18034f664518f006/game/tools/wayfarer_sevii_port/audit.py) checks exclusions, exact retained source identity, script ownership, and absence of retained trainers.

The shared inventory should represent the effective Wayfarer overlay alongside source provenance. Reading raw FRLG events alone would report removed trainers and story content as active. Selection, removal, and replacement are author decisions; a future adapter must preserve their drift guards instead of automatically accepting source changes.

There is a concrete tooling duplication: [Python encounter generation](https://github.com/mzpkdev/pokemon-wayfarer/blob/85748ebb6731a562b110cfef18034f664518f006/game/tools/wild_encounters/wild_encounters_to_header.py) resolves frozen FRLG source pairs in `load_wayfarer_sevii_profiles`, while [Cartographer's TypeScript catalog](https://github.com/mzpkdev/pokemon-wayfarer/blob/85748ebb6731a562b110cfef18034f664518f006/devtools/tools/cartographer/src/catalog/encounters.ts) materializes those pairs in `withWayfarerSeviiSource`. A later host projection could provide the resolved slots, identities, and provenance to both consumers, with parity fixtures proving identical species, levels, rates, and day/night aliases. The existing encounter manifest already references source rows rather than authoring a second roster.

Service scope needs care:

- Sevii's [service scripts](https://github.com/mzpkdev/pokemon-wayfarer/blob/85748ebb6731a562b110cfef18034f664518f006/game/data/scripts/wayfarer_sevii/services.inc) use fixed-stock marts. The v1 TR mart pilot must not enroll them or change their stock implicitly.
- Nurse, PC, daycare, and [environmental scripts](https://github.com/mzpkdev/pokemon-wayfarer/blob/85748ebb6731a562b110cfef18034f664518f006/game/data/scripts/wayfarer_sevii/environment.inc) can remain opaque, owned script references in a future inventory. Discovering them does not establish a generic service behavior contract.
- Ferry behavior remains deferred with the travel domain. The import is trainer-free, so trainer encounter contracts offer no direct benefit to these maps.

This is an optional regional-import/tooling pilot, not a required Sevii production migration or a dependency for #92. Its first measurable benefit would be fewer independent resolution implementations and consistent effective-content views. No ROM saving is established or promised.

## Direct source findings

- Progression anchors repeat in [trainer_rating.c](../../game/src/trainer_rating.c), [trainer_party_scaling.c](../../game/src/trainer_party_scaling.c), [gym_leaders.py](../../game/tools/trainer_scaling/gym_leaders.py), and [league.py](../../game/tools/trainer_scaling/league.py). Extracting the definition is practical, but identical anchors do not imply identical policy. [league_circuit.c](../../game/src/league_circuit.c) captures Rating at admission and exposes validated run Rating; [wayfarer_marts.c](../../game/src/wayfarer_marts.c) reads current Rating when opening a shop.
- [item.c](../../game/src/item.c) repeats the Wayfarer rod contributor set in `IsActiveStandardRodContributorFlag` and `CountStandardRodContributors`. One declaration can generate both projections without replacing the award transaction.
- [wayfarer_marts.h](../../game/src/data/wayfarer_marts.h) is production catalog content. [generate_manifest.py](../../game/tools/wayfarer_marts/generate_manifest.py) derives the [catalog manifest](../../game/tools/wayfarer_marts/catalog_manifest.json) from it. The manifest is audit output, not another hand-authored catalog or evidence of equivalent ROM cost.
- [pokedex.c](../../game/src/pokedex.c) defines `DexCatalog` descriptors that explicitly refer to existing authored tables. This is a useful precedent for references instead of copies.
- [league.h](../../game/src/data/trainer_scaling/league.h) contains emitted species and move values. Investigate canonical roster references and smaller exceptional parameters, but retain or replace the current validation contract before dropping fields. No saving has been measured.
- [wayfarer_native_hm_audit.py](../../game/tools/wild_encounters/wayfarer_native_hm_audit.py) explicitly marks area reachability `NOT_VERIFIED`. A shared inventory can improve discovery; it cannot turn a population witness into a route proof.
- At the separate #85 head, the [caller audit README](https://github.com/mzpkdev/pokemon-wayfarer/blob/6799eb7d9f625fefe0c0983104d38e3b13e7d4d5/game/tools/wayfarer_story_encounters/README.md) describes 851 reviewed callers and exact command fingerprints. Adding a script does not enroll it; changes demand review. That safety contract must survive an adapter. New templates may establish a contract by construction; arbitrary legacy scripts cannot be automatically opted in.

## What the compiler should and should not own

The compiler should read existing maps, scripts, trainer data, and build configuration through shared adapters; resolve typed references; validate declarations; and emit deterministic domain-specific runtime data plus richer host reports. It should preserve source provenance so a failure identifies the declaration and source object that disagree.

Authors still choose encounter populations, iconic rosters, town stock, utility windows, and story continuations. Semantics should be authored beside the relevant content or in a clearly owned companion declaration, rather than repeated in unrelated feature inventories. Structural facts should be derived from existing sources.

Runtime APIs should take explicit inputs and return bounded results. A curve evaluator takes a named curve and a supplied Rating. A service resolver takes a service identity and the state its domain requires. An encounter adapter resolves only declared or reviewed behavior. Persistent state remains in its gameplay owner; compiler interned IDs must not silently become save identities.

## Tradeoffs and staged adoption

| Choice | Benefit | Cost or risk |
| --- | --- | --- |
| Shared host inventory | Less repeated parsing and classification; common diagnostics. | Build configurations must match production preprocessing, or tooling can validate the wrong content. |
| Local semantic declarations | New content has one registration point for supported consumers. | Existing content needs migration; genuinely new semantics still require authoring. |
| Domain-specific generated representations | Runtime can use profiles, sparse exceptions, or existing references as appropriate. | More than one backend is needed; host schema size is not a runtime budget. |
| Runtime composition | Avoids materializing every combination. | CPU, stack, capacity, ordering, and failure behavior need explicit limits. |
| Conservative legacy adapters | Preserves story safety and enables gradual migration. | Temporary adapters and reviewed inventories remain until their consumers migrate. |

Pilot progression first, then rod and mart service declarations, then encounter integration after resolving #85's landing status. Evaluate each pilot independently. A pilot that adds complexity without reducing repeated authoring should be revised before expanding the framework. Later species, acquisition, and travel modules should reuse the host infrastructure only when their own concrete consumers justify them.

## Measurements still needed

1. Reproduce same-toolchain, same-build baselines with ELF/map attribution: code, read-only data, script data, initialized RAM, zero-initialized RAM, and release ROM headroom. Report incremental framework cost and migrated-consumer cost separately.
2. Compare declaration effort for adding a curve consumer, a rod giver, a mart binding, and an ordinary encounter. Count independent sources that must be edited; exclude generated reports from manual-authoring totals.
3. Confirm shared curve evaluation matches existing integer rounding and clamping at every supported Rating, including League save/load and retry snapshot boundaries.
4. Establish worst-case lookup/composition work and temporary storage. Determine whether sparse records, existing references, generated switches, or dense arrays are smallest under the actual compiler.
5. Verify generated inventories for every supported build configuration and disabled feature path. Existing history includes mismatched preprocessing and variant-specific failures; sharing parsing does not remove those risks automatically.
6. Test author-facing failures: missing references, duplicate service identities, ambiguous encounters, stale reviewed scripts, unsupported policies, capacity overflow, and non-deterministic outputs.

Documentation and source inspection were completed for this investigation. No ROM build, performance benchmark, mechanics test, or emulator journey was run, and no gameplay balance claim is made.
