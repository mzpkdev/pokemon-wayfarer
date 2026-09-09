# Framework validation record

This record covers phases A through C at commit
`6cfae19a450fd3e821ecdf35b494aa1434591a01`, compared with the merged
PR #91 squash baseline
`f4d6861481ee8ac1f7d06351f0e7cab9fafb1ab4`. Phase D is not included.

Each product was built sequentially from a clean release state with
`make -j8 BUILD=<product> release`. The captured configuration has `RELEASE=1`,
`USE_LTO_ON_RELEASE=1`, `O_LEVEL=2`, `TEST=0`, `DEBUG=0`, and `E2E=0`.
The compiler was `arm-none-eabi-gcc (15:13.2.rel1-2) 13.2.1 20231009`.

`../gameplay_content_resources/report.py` measured each matched ELF and map pair,
checked its ROM boundaries against both artifacts, reconciled category totals with
used ROM, and compared it to the same product's baseline. The complete machine
reports are ignored build artifacts at
`build/gameplay-content-resources/<product>.json`; they contain the full artifact
and configuration hashes. The command rejects a comparison whose product,
configuration, or toolchain does not match.

[resource-evidence.json](resource-evidence.json) commits the compact selection of
those reports and the captured effective configuration hashes. Its shared
configuration block applies to every product entry. The report selections contain
each product's exact configuration hash. This keeps the evidence reviewable without
depending on the task-local reports.

## Paired release measurements

All figures are bytes. Category deltas not named in the last column are zero.

| Product | Used ROM, baseline to current | Delta | EWRAM, baseline to current | IWRAM, baseline to current | Category deltas |
| --- | ---: | ---: | ---: | ---: | --- |
| Wayfarer | 32,249,612 to 32,249,312 | -300 | 248,509 to 248,509 | 25,556 to 25,556 | code -360; graphics -8; other +68 |
| HNS | 30,803,868 to 30,803,868 | 0 | 247,569 to 247,569 | 25,612 to 25,612 | all zero |
| Emerald | 28,521,156 to 28,521,156 | 0 | 247,493 to 247,493 | 25,648 to 25,648 | all zero |
| FireRed | 28,616,076 to 28,616,076 | 0 | 247,601 to 247,601 | 25,864 to 25,864 | all zero |
| LeafGreen | 28,616,480 to 28,616,480 | 0 | 247,601 to 247,601 | 25,864 to 25,864 | all zero |

Phase A's Wayfarer inventory and build integration measured zero ROM and static-RAM
change. Phase B measured a 300-byte reduction. After phase C, the padded Wayfarer
ROM SHA-256 was still
`780f53c159aecb3f48489d58b006e221aede856c4b94034c2bf4e129c67f68a3`,
which is byte-identical to the Phase B ROM. This confirms the generated service
projections did not change the final Wayfarer ROM. None of these measurements use
generated JSON size as a ROM-size claim.

The eight-byte graphics movement is linker alignment, not a graphics asset change.
The 68-byte `other` increase is the interned progression curve data: a 32-byte
descriptor table and two 18-byte point arrays. In the matched Wayfarer release,
`EvaluateGameplayCurve` is 0x54 bytes. LTO removes that evaluator and the
progression symbols from the HNS, Emerald, FireRed, and LeafGreen releases.

The current report fingerprints below are SHA-256 prefixes. The paired reports
named above retain the complete values.

| Product | Configuration | ELF | Map |
| --- | --- | --- | --- |
| Wayfarer | `f7de40d8e3ffa3c8` | `b6f10d674a961ca7` | `50fc795634805325` |
| HNS | `9c5af62607e92012` | `0e89776a80cc6e02` | `129fba8799e81d2c` |
| Emerald | `6d424165c9b59de4` | `1f848a01100cc772` | `e22331484b1bede6` |
| FireRed | `29aabdc3ff1aa245` | `34a8d99ca81acae` | `f672371fd6dec9c` |
| LeafGreen | `8652e038e944861a` | `46355ca348c7fd85` | `f1937b1265808121` |

`resource-evidence.json` includes the baseline, phase A, phase B, and final head
selections. The ignored reports remain useful for local inspection, but are not
the sole source of the committed evidence.

## Checks run

The generated-content host suite passed 35 checks. The existing map JSON suite
passed 5, trainer host tests passed 28, and wild generator tests passed 46. The
mart catalog generator checked 5,670 projections: 35 profiles, 81 ratings, and two
challenge states each.

Default-product mechanics checks passed: gameplay progression 3, bag 19,
trainer rating 9, trainer-party scaling 23 with one expected `ASSUME` skip,
Wayfarer marts 5, and league circuit 17. The matched E2E-enabled ROM and symbol
file were built together with `make -j8 e2e`. Their SHA-256 values are
`391129495bbeec0f8d9d880f31b06cd96ef9c09dce0ac31491e166c24f4d4f4e` for
the ROM and `a3a38cad24e0ed0fc301fab7d7206eb0dc192f18cf5ee9b8eee0764f5ad6fc5d`
for the symbol file. The HNS standard rod-giver journey passed 6 of 6 tests in
36.49 seconds with a fresh writable XDG directory. No whole-suite E2E result is
recorded here.

The main reproducible entry points were:

```sh
make BUILD=wayfarer gameplay-content-test
make BUILD=wayfarer TEST=1 gameplay-content-check
make -j8 BUILD=wayfarer check TESTS='Gameplay progression'
make -j8 BUILD=<product> release
```

From `e2e/`, the rod journey used the directly matched paths and writable runtime
directories:

```sh
XDG_DATA_HOME=<writable>/data XDG_CACHE_HOME=<writable>/cache \
LIBGL_ALWAYS_SOFTWARE=1 \
SKYEMU_ROM=../game/pokemon-wayfarer-e2e.gba \
SKYEMU_SYMS=../game/pokemon-wayfarer-e2e.sym \
./node_modules/.bin/wa test src/journeys/hns-standard-rod-givers.e2e.ts
```

The disabled trainer-scaling rollback check fails at
`test/trainer_party_scaling.c:592` with `EXPECT_EQ(1, 63)`. The same failure was
reproduced in a clean git archive of baseline `f4d6861`, so it is recorded as a
pre-existing failure. It does not establish disabled-feature acceptance for this
work.

## Acceptance boundary

These paired builds show no static-RAM increase, and phases A through C introduce
no save fields or heap allocation. They do not provide released-ROM cycle or stack
profiling. A test-only timing diagnostic observed maximum additional cycles of 69
for league baseline, 49 for league level, 54 for ordinary, and 75 for Gym; that
instrumented result is not release CPU or stack evidence.

Matched release disassembly bounds the evaluator's own frame at 16 bytes: it saves
`r4`, `r5`, `r6`, and `lr`; the division helper has no stack adjustment. The curve
loop has at most eight intervals and nine points. This does not bound callers,
interrupt nesting, or released-ROM execution time, so it is not a substitute for
the missing dynamic acceptance evidence.

Trainer scaling and encounter semantic adapters remain outside this record. Their
phase D prerequisite is the trainer-only integration in open PR #85, which has not
been imported here. The remaining runtime evidence and that prerequisite mean the
overall gameplay content framework specification is not yet claimed as implemented.
