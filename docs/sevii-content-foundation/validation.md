# Foundation validation

## Identities and environment

- Exact base: `7aeb01c62108f005981297d5d83d0a8826981c10`.
- Base source diff: empty. Build inputs were copied before implementation edits
  to `/tmp/sevii-foundation-base/game`; generated files were rebuilt there.
- Compiler: `arm-none-eabi-gcc 13.2.1`.
- GNU Make: `4.4.1`, built from the GNU release archive under
  `/tmp/sevii-build-tools`. Make 4.3 attempts are historical failed setup runs.
- Production command: `make -C game -j8 BUILD=wayfarer release`.
- Raw logs, ROMs, ELFs and linker maps: `/tmp/sevii-foundation-evidence/`.
- [Candidate source identity](evidence/source-identity.json) fingerprints every changed
  game source and host test against the exact base.
- Source owners: schema/projection, script closure/port audit, shared contracts,
  content audit, and root integration. Root owns final validation and product builds.

## Baseline

The exact-base Wayfarer release uses **32,673,336 bytes** (`__rom_end =
0x09F28E38`), with **881,096 bytes free** and **356,808 bytes above the 512 KiB
reserve**. Its `gSaveblock3` symbol is **904 bytes**. The existing compile-time
save-sector bound passed.

ROM SHA-256: `4a5efc952c50e4095bd8c3d9aff3582a37276573ed0e495058db74fcdc97ce6f`.
The checked-in [base report](evidence/base-wayfarer-release.json) includes its
category comparison against the repository's older accepted baseline. Candidate
comparison must use this exact-base report instead.

## Candidate and product isolation

The candidate uses **32,673,336 bytes**, exactly matching the base. Every category
in the [candidate report](evidence/candidate-wayfarer-release.json) has a zero-byte
delta against the exact starting commit. `__rom_end` remains `0x09F28E38`, leaving
881,096 bytes free and 356,808 bytes above the required reserve. `gSaveblock3`
remains **904 bytes**, below its 1,624-byte bound.

The Wayfarer binary hash changes because centrally generated map-script tables
move within the script section. The retained event records and handler bodies
are unchanged; see [baseline equivalence](evidence/baseline-equivalence.json).

Serial release builds passed for Wayfarer, FireRed, LeafGreen, Emerald and HNS.
FireRed, LeafGreen and Emerald are byte-identical to the exact base. The exact
base cannot compile standalone HNS because of missing Sevii map-section aliases;
HNS is byte-identical to the exact base plus the isolated prerequisite fix in
`23d2b8eb8f`. [ROM identities](evidence/rom-identities.json) record every hash.
The raw evidence retains the failed exact-base HNS log and reference patch.

## Verification

- Normal Wayfarer build, `BUILD=wayfarer check`, and the E2E ROM build passed.
- Headless SkyEmu Sevii exploration journey: all 6 tests passed, including
  loading every imported map, ungated ferry travel/cancellation and Birth
  Island/Navel Rock predicates (48.40 seconds).
- The full mechanics run reported 4,357 passed, 6 expected-failing, 349
  known-failing, 629 TODO and 9 assumption failures out of 5,350 cases; the
  repository check exited successfully.
- Wild encounter scaling: 61 tests passed; balance audit passed 2,205 method rows.
- Trainer party scaling: 28 tests passed; league scaling: 5 tests passed.
  Trainer classification audited 1,513 populated rows; league audit checked
  15 fixed rosters and 81 rating inventories.
- Final mapjson projection suite: 11 passed; generator/environment suite:
  14 passed; existing Sevii port suite: 13 passed. Content schema, closure,
  contract and audit suite: 43 passed. The audit was run twice with the measured
  candidate report and produced byte-identical JSON.
- Independent review findings were reproduced and fixed, covering source drift,
  owner collisions, state declarations, repeatable claims, typed battle forms,
  reference closure and dependency paths.

The mechanics suite retains its pre-existing known-failing/TODO cases. This
foundation adds no new runtime primitives or save fields, so its new regression
coverage is in host-side positive and negative fixtures.

## Reproduction

Use GNU Make 4.4.1 on `PATH`, the repository toolchain, and serial product builds:

```sh
make -C game -j8 BUILD=wayfarer release
make -C game -j8 BUILD=wayfarer check
make -C game wayfarer-sevii-content-test wayfarer-sevii-content-audit wayfarer-sevii-port-audit
python3 game/tools/wayfarer_sevii_scripts/generate.py --root game --check
python3 game/tools/rom_report/rom_report.py --symbols game/pokewayfarer-release.map \
  --build wayfarer --release --baseline docs/sevii-content-foundation/evidence/base-wayfarer-release.json \
  --output /tmp/sevii-candidate-size.json
python3 game/tools/wayfarer_sevii_content/audit.py --root game \
  --rom-report /tmp/sevii-candidate-size.json --output /tmp/sevii-content-audit.json
```

Repeat release builds with `BUILD=firered`, `leafgreen`, `emerald`, and `hns`.
Recreate the exact baseline in a separate checkout at the recorded base commit;
apply only the alias prerequisite to the separate HNS reference. Do not switch
products concurrently in one tree. ROM artifacts remain local evidence and are
not checked into source control.

Run the emulator journey against explicitly built artifacts:

```sh
make -C game -j8 e2e
SKYEMU_ROM="$PWD/game/pokemon-wayfarer-e2e.gba" \
SKYEMU_SYMS="$PWD/game/pokemon-wayfarer-e2e.sym" \
  pnpm --filter @wayfarer/skyemu-e2e exec wa test \
  src/journeys/wayfarer-sevii-exploration.e2e.ts
```
