# PR #96 ROM footprint

PR #96 adds **27,320 bytes (26.68 KiB)** to the Wayfarer release ROM. Fresh local
builds reproduced the current CI numbers exactly: 32,668,400 bytes at base
`ec18cfffd21f31b54a60a73d750863bde30be83d`, and 32,695,720 bytes at PR head
`86518d73d397efe93fb8a34dce3887c90f2303ce`.
The current [budget comment](https://github.com/mzpkdev/pokemon-wayfarer/pull/96#issuecomment-5640831688)
rounds this to **+26.7 KiB**. The previously observed +26.9 KiB is not the current
head's measurement.

The implementation covers a trainer-only wild battle controller, Bag and medicine
handling, capture and retreat rules, party exhaustion and storage, and authored
story recovery across regions. Its ROM cost includes an explicit encounter
allowlist and scene lifecycle machinery as well as the visible battle feature.

| Linked contribution | Bytes added | KiB |
| --- | ---: | ---: |
| Machine code | 12,120 | 11.84 |
| Ordinary encounter registry: 851 × 8 bytes | 6,808 | 6.65 |
| Hoenn scene registry: 49 × 36 bytes | 1,764 | 1.72 |
| Johto/HNS scene registry: 41 × 36 bytes | 1,476 | 1.44 |
| Other read-only data, including text and tables | 2,184 | 2.13 |
| Script section | 2,972 | 2.90 |
| Graphics section | -4 | -0.004 |
| Maps/layouts, audio, trainer parties, wild encounter data | 0 | 0 |
| **Total** | **27,320** | **26.68** |

The three registries consume 10,048 bytes, or 36.8% of the growth. Their sizes are
measured ELF symbols, not estimates from source line counts. The remaining
read-only data is the `other` category delta after subtracting these registries.
The tiny graphics difference is an aggregate section result, not a new asset.

The ordinary registry retains the address of each exact reviewed battle command,
a stable dialogue key, a dialogue class, and policy flags. Of its 851 callers,
787 permit field loss return. This distinction prevents a defeat from running
an encounter's victory script or changing an unaudited challenge's behavior.
Replacing it with a blanket rule for every trainer would change behavior.

An earlier pass already reduced the ordinary row from the 36-byte regional
format to its current 8-byte format. The historical validation document records
a 23.1 KiB saving in matched normal builds. That saving is already reflected in
this PR; it cannot be counted again against the current release delta.

The regional records carry caller, loss and trigger pointers, dialogue and scene
IDs, map/object/activation coordinates, and an eligibility callback. Rival
starter and player-gender variants repeat much of this data. The additional
scripts supply refusal, retreat, retry, and object restoration paths.

The code increase includes new battle actions and integration into existing
battle, item, storage, and field functions. `TrainerOnlyMain` alone occupies
2,384 bytes in the release ELF. `ItemMenu_UseInBattle` grows by 724 bytes.
Release already uses `-O2` and LTO. Inlining changes symbol boundaries, so a
new symbol's entire size is not necessarily newly added functionality; the
12,120-byte linked code-section delta is the reliable total.

TypeScript E2E journeys, product documents, host audit JSON, and Python tests are
not ROM payloads. The ROM E2E mailbox implementation is guarded by
`E2E_TESTING`, which the release build does not define. Removing those tests
will not recover this headroom.

## Implemented reductions

The optimized release uses **32,691,240 bytes**, saving **4,480 bytes (4.38 KiB)**
against the original PR head. The PR's growth falls from **26.68 KiB to 22.30 KiB**,
a **16.4% reduction**, without removing encounter behavior. Static EWRAM and
IWRAM allocations are unchanged.

| Change | Measurement |
| --- | ---: |
| Ordinary registry: aligned callers plus one metadata byte | 2,553 bytes gross data saving |
| Regional registry: 54 shared descriptors plus 90 callers/indices | 1,062 bytes gross data saving |
| Size-optimized controllers and berry bounds check, measured in isolation | 996 bytes net ROM saving |
| **Combined final release, including decoding and alignment costs** | **4,480 bytes net ROM saving** |

The ordinary metadata byte keeps four dialogue classes, the dialogue variant
(`stableKey % 4`), loss-return permission, and completed-trainer text permission.
The host manifest retains complete stable keys and audit fingerprints. Runtime
pointers remain aligned; a packed pointer-containing struct would risk unaligned
loads on the GBA. Regional records share only identical descriptors. Caller
order, callerless scene rows, trigger pointers, eligibility callbacks, and all
other regional fields remain intact. Lookups decode into caller-owned values;
there is no permanent expanded registry cache in EWRAM.

The release Makefile applies `-Os` to `trainer_only_encounter.c` and
`battle_controller_trainer_only.c`; the rest of the engine retains its existing
optimization settings. The feedable-berry function now checks the existing
contiguous item range instead of searching a 67-entry list. An independent
native test retains the original explicit list and checks every item ID,
including exclusion of the following e-Reader berry.

### Why the runtime is still substantial

The original **11.84 KiB is net machine-code growth**, not the size of one small
handler. The feature adds a complete encounter state machine and touches the
existing battle command controller, Bag item actions, capture/storage handling,
and story-object recovery. `TrainerOnlyMain` is 2,384 bytes in the original
release; choosing size optimization brings it to 1,640 bytes. That function
handles the action menu, rock/berry/ball turns, retaliation, fleeing, and exits.
The story system also needs explicit refusal, loss routing, and object/trigger
recovery to avoid awarding victory consequences after a loss.

The existing release uses `-O2` with LTO. It can inline helpers and duplicate
paths to favor speed, so source line counts and individual function growth are
poor estimates of net ROM cost. A measured attempt to force the message helper
out of line made the ROM **112 bytes larger**, and was rejected. The accepted
runtime changes save 996 bytes of total ROM in isolation; after combining them
with registry decoding, the code section is **496 bytes smaller** and read-only
`other` data is **3,984 bytes smaller** than the original head. The remaining
net code growth is **11,624 bytes (11.35 KiB)**.

Further reductions need another measured tradeoff. Repeated script gates, such
as Rustboro's eight rival approaches, may admit a shared helper if it preserves
script-stack and continuation behavior. A broader Bag/controller refactor would
need evidence that the linked output shrinks; much of the apparent per-function
increase is code moved or inlined from other functions. Neither opportunity is
included in the measured saving above.

## Evidence and reproduction

The `pr96-rom-footprint` uberepo task has separate head and `@base` worktrees.
Both completed this command successfully with ARM GCC 13.2.1:

```sh
make -j6 BUILD=wayfarer release UNUSED_ERROR=1 DEPRECATED_ERROR=1
```

The builds also ran their required content and encounter audits. The optimized
release was rebuilt with the same compiler and release flags, including the two
new target-specific `-Os` overrides.

Independent binary validation compares the optimized linked ROM against the
preserved original release, resolving pointers by symbol identity rather than
address. All **851 ordinary callers** retain their runtime metadata, and all
**90 regional records** retain every field and caller/trigger/callback identity.
This check caught and verified the repair of a callerless-scene migration bug;
count-only or generator round-trip tests would not have caught it.

The corrected registry passes all **8 native story tests**. All **13 trainer-only
native tests** pass, including exhaustive berry membership. The host audits pass
**22 generator tests and 9 static contracts**. Independent final review found
no remaining actionable findings.

A freshly built E2E-enabled playable ROM passes **100 tests across 8 emulator
journey files** in SkyEmu: core trainer-only encounters, ordinary story handling,
Hoenn story and objectives, Johto coverage, medicine, Quick Ball, and Safari
regressions. This uses the repository's regular E2E build; release-specific
`-Os` output is measured separately by the release link and binary oracle.

```sh
make -C game -j8 e2e
SKYEMU_ROM="$PWD/game/pokemon-wayfarer-e2e.gba" \
SKYEMU_SYMS="$PWD/game/pokemon-wayfarer-e2e.sym" \
pnpm --dir e2e exec wa test \
  src/journeys/wayfarer-trainer-only.e2e.ts \
  src/journeys/wayfarer-trainer-only-story.e2e.ts \
  src/journeys/wayfarer-trainer-only-hoenn-story.e2e.ts \
  src/journeys/wayfarer-trainer-only-johto-coverage.e2e.ts \
  src/journeys/wayfarer-trainer-only-medicine.e2e.ts \
  src/journeys/wayfarer-trainer-only-quick-ball.e2e.ts \
  src/journeys/wayfarer-trainer-only-hoenn-objectives.e2e.ts \
  src/journeys/wayfarer-safari-regression.e2e.ts
```

Run these E2E commands from the repository root. `native-story-tests.log`,
`native-trainer-only-tests.log`, and
`e2e-journeys.log` retain the raw successful execution evidence.

Task-local `compare_rom.py` compares the release reports and sized ELF symbols.
The adjacent task-level `evidence/` directory contains original binaries, build
logs, metadata, size and symbol comparisons, isolated runtime experiments,
compiled-record equivalence results, native test logs, and review fingerprints.
Existing repository ROM reports compare against a historical accepted baseline;
this investigation explicitly
subtracts the paired PR-base build instead.

The corresponding [CI budget job](https://github.com/mzpkdev/pokemon-wayfarer/actions/runs/34652059770/job/103446098683)
records the same original base and head sizes. The optimized ROM leaves
863,192 bytes before the 32 MiB capacity, including the protected 512 KiB
reserve, and 338,904 bytes (331.0 KiB) before that reserve.
