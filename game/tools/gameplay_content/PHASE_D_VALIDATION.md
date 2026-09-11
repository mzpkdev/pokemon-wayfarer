# Phase D validation

The measured implementation is `a655387c45ebd54061778c7443545c1adf23168c`.
The evidence commit changes documentation only.

The unrefactored baseline is `f4f602d0dd5e00edae4b49f1f4992d217ed02ed2`,
rebased onto framework main `d99050250df47fe785736a7932d0dc33ef22031a`.
The task preserves that revision and its release artifacts separately from this
refactor. This record covers phase D; the earlier A-C measurements remain in
[RELEASE_PROFILING.md](RELEASE_PROFILING.md).

## Migrated content

The compiler owns 931 reviewed caller declarations: all 851 ordinary callers,
78 regional legacy adapters, and full declarations for the Rusturf Aqua guard
and the Route 110 deferred rival. Ordinary caller order, dialogue keys, rematch
identity, and the 787 field-loss redirects remain the baseline values. The 64
ordinary double/rematch-double callers retain their existing defeat routing.

The production script pipeline discovers 1,147 emitted battle invocations.
The remaining 216 unreviewed calls receive no outcome authorization. Discovery
uses real CPP and assembler selection, including inactive assembly branches;
source fingerprints include the compiler-discovered include closure. The final
ELF check validates each declared command's opcode, mode, trainer, and argument
offset. Continuation modes come from assembled bytes, not macro-name guesses.

All 1,513 populated scaling classifications move to the shared declaration
projection. Complete caller coverage permits removing 785 redundant fallbacks;
728 reviewed fallbacks remain. The six Rustboro rival variants retain fallbacks
because their Route 104 calls remain unreviewed. Shared actors must agree on
scaling policy, and a newly discovered undeclared caller prevents treating a
fallback-free actor as fully migrated. Unsupported trainer operands fail closed.
Standalone products retain discovery without consuming Wayfarer's scaling policy.

The frozen ordinary snapshot and original regional table hashes are independent
migration oracles. Scene tests expand the two generated rows back into the original
Hoenn table positions before comparing its original hash. Scaling policy bytes,
predecessors, move exceptions, and balance projections retain their baseline
values. The compatibility inventory changes only the explanation for 63 excluded
actors whose classifications now derive from complete reviewed caller coverage.

## Review and checks

The permanent host suite passes 74 tests; the temporary migration suite passes
five. Clean-generated bootstrap was exercised before the aggregate. CI workflow
lint and all 12 change-classification tests pass. The saved baseline ELF validates
all 931 declared caller addresses and assembled argument contracts.

Independent review covered script source selection, digest closure, build and
post-link integration, encounter bindings, and scaling completeness. Accepted
findings were fixed: generated-header bootstrap dependencies, standalone scaling
ownership, raw macro operand positions, duplicate caller bindings, rematch dialogue
identity, and continuation modes. Unknown operands and incomplete fallback removal
fail generation rather than silently dropping discovery evidence.

The Wayfarer release passes its reserve gate and validates all 931 linked
callers. Its GBA and ELF are byte-identical to the baseline; the map differs only
in temporary LTO object names. Used ROM is 32,692,136 bytes, static EWRAM 248,641,
and static IWRAM 25,564: all deltas are zero.

The unchanged optimized `WayfarerStoryFindCaller.part.0` is 196 bytes and retains
its fixed 851-row ordinary lookup (eight bytes per row), followed by the bounded
regional registries. `WayfarerStoryReconcileCurrentMap` is 304 bytes and retains
the existing map/eligibility checks. The refactor adds no runtime routine, callback,
save field, heap allocation, recursion, or variable-size stack object. Full ELF
identity establishes that these optimized routines and their callers are unchanged;
no new timing or peak-stack measurement is claimed. Static RAM alone is not used
as peak-stack evidence.

The matched E2E build passes all 931 linked caller checks. With explicit saved
ROM/symbol paths, `LIBGL_ALWAYS_SOFTWARE=1`, and four Vitest workers, all 95 tests
pass in 188.48 seconds across these seven journey files:

- `wayfarer-trainer-only-story.e2e.ts`
- `wayfarer-trainer-only-hoenn-story.e2e.ts`
- `wayfarer-trainer-only-hoenn-objectives.e2e.ts`
- `wayfarer-trainer-only-johto-coverage.e2e.ts`
- `wayfarer-trainer-only-johto-hosts.e2e.ts`
- `wayfarer-trainer-only-radio-host.e2e.ts`
- `wayfarer-trainer-only.e2e.ts`

These include actual ordinary and objective losses, actual Azalea and Rusturf
victories, completed aftertext, rematch refusal, recovery/leave/re-entry rival
restoration, and existing challenge/objective exclusions. Forced outcomes remain
limited to existing rare lifecycle checks; they do not replace those real battles.

Full mechanics validation succeeds with the baseline totals: 4,359 passed,
349 known failing, nine failed assumptions, 629 TODOs, and six expected failures
across 5,352 cases. There are no unexpected failures. The default configuration's
existing trainer-scaling rollback assumption remains an expected skip.

The final standalone report correction does not change either Wayfarer runtime
include. An independent final-source generation exactly matches both includes
used for the saved release and E2E builds. Their behavior/resource evidence is
retained on that equality; standalone products are rebuilt with the correction.
## Paired release resources

Each refactored ROM and ELF is byte-identical to its same-product baseline.
Every used-ROM, static-EWRAM, and static-IWRAM delta is zero. All products pass
their existing capacity/reserve checks. Figures below are bytes.

| Product | Used ROM | Static EWRAM | Static IWRAM |
| --- | ---: | ---: | ---: |
| Wayfarer | 32,692,136 | 248,641 | 25,564 |
| HNS | 30,811,588 | 247,577 | 25,616 |
| Emerald | 28,543,724 | 247,513 | 25,648 |
| FireRed | 28,638,924 | 247,625 | 25,864 |
| LeafGreen | 28,639,328 | 247,625 | 25,864 |

The separate PR #85 feature increment above exact framework main is **27,456 ROM
bytes, 132 EWRAM bytes, and eight IWRAM bytes**. Framework main uses 32,664,680
ROM bytes, 248,509 EWRAM bytes, and 25,556 IWRAM bytes. Those feature costs are
not refactor growth, and the earlier framework savings are not counted again.

[phase-d-resource-evidence.json](phase-d-resource-evidence.json) records the measured
implementation revision, exact build commands, toolchain, feature configurations,
paired ELF/map hashes, linked categories, static RAM, E2E hashes, and final-source
runtime-generation equivalence. Original baseline artifacts were not modified.

## Reproduction

From the repository root:

```sh
make -C game BUILD=wayfarer gameplay-content-test
TEST=1 UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -C game -j4 BUILD=wayfarer check
UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -C game -j8 BUILD=wayfarer release
make -C game -j8 e2e
make -C game clean-assets
make -C game -j8 BUILD=<product> release
```

Build products sequentially and save each artifact before the next product's
clean-assets step. Standalone products use the baseline's default warning policy.
The feature-configuration digest covers release gates and configuration headers;
diagnostic-only warning switches are reflected in commands rather than treated
as gameplay features.

From `e2e/`, invoke the installed Vitest CLI with `run --config webanvil.config.ts
--maxWorkers 4` and the seven `src/journeys/` paths above. Set `SKYEMU_ROM` and
`SKYEMU_SYMS` to the saved matched artifacts and `LIBGL_ALWAYS_SOFTWARE=1`.
