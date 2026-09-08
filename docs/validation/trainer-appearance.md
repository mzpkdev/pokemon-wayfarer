# Trainer appearance validation

The authoritative requirements are the [PRD](../../.product/prds/trainer-appearance-styles.md) and [specification](../../.product/specs/trainer-appearance-styles.md), merged in PR #87. The implementation starts from `583308c817`.

Acceptance requires emulator evidence as well as mechanics tests and serial builds. An asset inventory or successful compile does not establish visual correctness.

| Acceptance area | Evidence |
| --- | --- |
| Six picker previews and input behavior | Pending emulator validation |
| Naming, challenge callbacks, fresh setup reset and shrink | Pending emulator validation |
| Twelve appearance/origin combinations | Pending state and emulator validation |
| Save, Continue, invalid appearance and gender pair | Pending validation |
| Field actions, palettes, reflections and movement restoration | Pending asset and emulator validation |
| Naming, battle, card, Hall of Fame and facility rendering | Pending emulator validation |
| Live local link appearance and legacy remote/recorded actors | Pending validation |
| Regional travel and story gender mapping | Pending validation |
| Wayfarer and standalone builds | HNS, FireRed, LeafGreen and Emerald releases passed; Wayfarer blocked by missing FRLG action descriptors |
| Production ROM reserve and save/sprite budgets | Pending measurement |

The production release limit is `0x09F80000`, retaining 512 KiB before the physical 32 MiB ROM limit. The accepted ROM-report baseline records 32,865,136 used bytes. Build measurements must retain the normal reserve enforcement and category comparison.

## Recorded checks

- `make -C game -j8 tools check-tools`: passed.
- `make -C game -j8 BUILD=hns release`: passed; 30,804,368 ROM bytes used. Existing linker/newlib and LTO warnings remain in the build log.
- `make -C game -j8 BUILD=firered release`: passed; 28,616,300 ROM bytes used.
- `make -C game -j8 BUILD=emerald release`: passed; 28,521,460 ROM bytes used.
- `make -C game -j8 BUILD=leafgreen release`: passed; 28,616,704 ROM bytes used.
- `make -C game -k -j8 BUILD=wayfarer UNUSED_ERROR=1 DEPRECATED_ERROR=1 release`: failed on six missing FRLG graphics identifiers. The contest script change also required an updated reviewed content fingerprint; its 10-test audit suite passed after that update. No Wayfarer ROM size or asset delta is available.
- `make -C game -k -j8 BUILD=wayfarer check TESTS=Wayfarer`: test sources compiled; linking/execution blocked by missing FRLG identifiers. This compile exposed a nonzero `.sbss` initializer, which was corrected to explicit runtime initialization. Mechanics tests have not executed. A subsequent targeted build of the corrected picker, decoration and region-map test objects passed.
- `python3 -m unittest discover -s game/tools/wayfarer_hoenn_content -p 'test_*.py' -q`: 10 tests passed after reviewing the contest script change and updating its content fingerprint.
- `python3 -m unittest discover -s game/tools/rom_report/tests -q`: 21 tests passed.
- `python3 -m unittest discover -s game/tools/trainer_appearance -p 'test_*.py' -q`: six tests passed. The checked manifest itself reports six missing-art failures across 54 combinations.
- `python3 game/tools/wayfarer_origin/test_scripts.py`: 20 tests passed (identity worker).
- `pnpm --dir e2e exec wa test src/harness`: five files, 22 tests passed.
- `pnpm --dir e2e exec wa typecheck`: passed. Changed-file lint passed in the E2E worker. No emulator run yet.

The E2E ROM build also failed on the same six missing FRLG identifiers after compiling its telemetry and picker code. No playable Wayfarer E2E ROM or emulator captures were produced.

All four standalone release targets were rerun serially after the review fixes and passed. The byte counts above are from those final builds. Build logs are retained locally under `/tmp/appearance-*-build.log`, with final standalone logs under `/tmp/appearance-final-*-build.log`.

## Missing action art

Two imagegen attempts failed the required frame dimensions, palette and pose order. Both were rejected; see the [candidate report](trainer-appearance-art-blocker.md). Red and Leaf Acro, underwater and watering support remains incomplete. No substitute protagonist or incompatible generated sheet is accepted as completed art.
