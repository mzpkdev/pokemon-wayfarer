# Tileset ROM storage optimization

PRD: [Tileset ROM storage optimization](../prds/tileset-storage-optimization.md)
Implemented: No

## Scope

This specification defines the version 1 asset manifest, deterministic build
rules, tileset header changes, production-link guard, runtime ownership checks,
measurement method, tests, rollout, and rollback for the two active full-size
tileset graphics payloads selected by the parent PRD.

It does not change the generic decompressor or tileset loader. It does not
compress the six short active Secret Base secondary sheets, change authored
content, or remove source assets. It also does not claim a new saving for
unreachable definitions that the task-base release already discards.

## Behavior

### Version 1 asset manifest

The implementation uses this closed manifest. Adding another asset requires a
separate reviewed scope change.

| Header | Symbol | Active raw input | Generated input selected in optimized mode | Codec | Decoded bytes |
| --- | --- | --- | --- | --- | ---: |
| `gTileset_SecretBase` | `gTilesetTiles_SecretBase` | `data/tilesets/primary/secret_base/tiles.4bpp` | `data/tilesets/primary/secret_base/tiles.4bpp.lz` | GBA LZ77 | 16,384 |
| `gTileset_CableClub` | `gTilesetTiles_CableClub` | `data/tilesets/secondary/cable_club/tiles.4bpp` | `data/tilesets/secondary/cable_club/tiles.4bpp.lz` | GBA LZ77 | 16,384 |

The symbol names remain unchanged to match the tileset catalog convention.
Only their `INCBIN_U32` input paths differ between raw and optimized modes.
The optimized headers set `isCompressed = TRUE`; the raw rollback headers set
it to `FALSE`.

No selected `INCBIN_U32` path may contain `unused_tiles` or `unknown_tiles`.
In particular, none of these legacy symbols may satisfy an active header:

- `gTilesetTiles_SecretBaseBrownCaveCompressed`
- `gTilesetTiles_SecretBaseTreeCompressed`
- `gTilesetTiles_SecretBaseShrubCompressed`
- `gTilesetTiles_SecretBaseBlueCaveCompressed`
- `gTilesetTiles_SecretBaseYellowCaveCompressed`
- `gTilesetTiles_SecretBaseRedCaveCompressed`

### Build mode and rollback switch

Add one narrow Boolean make option named
`TILESET_STORAGE_OPTIMIZATION`. Reject values other than `0` and `1`, and pass
the value to C preprocessing for the tileset data and headers. Keep the
rollout baseline explicitly on `0`; change the production default to `1` only
after all acceptance gates pass.

- Value `1` selects the two compressed paths and sets both selected headers to
  `isCompressed = TRUE`.
- Value `0` selects the two existing raw paths and sets both selected headers
  to `isCompressed = FALSE`.

The option must not affect another tileset, build flag, object list, content
manifest, or linker option. Because both modes share the normal Wayfarer
release output directory, paired measurements must use clean serial builds and
copy the ELF, map, and size report out before switching modes. Do not compare
stale objects from two option values.

### Compression rules

Use the repository's existing deterministic suffix rules:

1. Generate Secret Base LZ77 with the existing `%.lz: %` rule, which invokes
   `$(GFX) $< $@`.
2. Generate Cable Club LZ77 with the same existing `%.lz: %` rule.
3. Do not run several codecs and choose the smaller output during a normal
   build. The manifest fixes GBA LZ77 for both assets.
4. Make the compressed output depend on its active raw `tiles.4bpp` input.
   The raw file continues to be generated from the existing `tiles.png`
   source.
5. Before either compressed output can be used by compilation, verify the raw
   input size is exactly 16,384 bytes.
6. Decode each generated output to a temporary file with `gbagfx`.
7. Verify the decoded file is exactly 16,384 bytes, then perform a binary
   comparison against the active raw input. Any failure removes or withholds
   the generated target and stops the build.

The task-base measurements are:

| Input | GBA LZ77 | fastSmol | Selected |
| --- | ---: | ---: | ---: |
| Secret Base, 16,384 raw bytes | 9,140 | 10,980 | 9,140 |
| Cable Club, 16,384 raw bytes | 4,028 | 3,872 | 4,028 |
| Selected total |  |  | 13,168 |

The host-tool round trips for both selected outputs must compare byte for byte.
The selected 19,600-byte payload reduction is gross until paired final links
confirm the net result. The 3,872-byte Cable Club fastSmol result is smaller,
but version 1 gives up the 156-byte difference to keep both assets on one
decoder path and one host round-trip procedure. It does not add a second codec
to the validation and rollback matrix.

### Short active sheets remain raw

The following active symbols retain their current raw `tiles.4bpp` definitions
and `isCompressed = FALSE` headers:

| Active symbol | Raw bytes |
| --- | ---: |
| `gTilesetTiles_SecretBaseBrownCave` | 2,656 |
| `gTilesetTiles_SecretBaseTree` | 2,656 |
| `gTilesetTiles_SecretBaseShrub` | 2,656 |
| `gTilesetTiles_SecretBaseBlueCave` | 2,656 |
| `gTilesetTiles_SecretBaseYellowCave` | 2,656 |
| `gTilesetTiles_SecretBaseRedCave` | 2,656 |

For Emerald layouts, `GetNumTilesInPrimary` returns 512 and the loader copies
`NUM_TILES_TOTAL - 512`, also 512 tiles, for the secondary sheet. That is a
16,384-byte request. A compressed 2,656-byte decode is therefore forbidden.

The reproduced 17,340-byte padded result is specifically the combined GBA
LZ77 result and is 1,404 bytes larger than the 15,936 raw bytes. A padded
fastSmol experiment totals 8,856 bytes, but it remains outside this manifest
and must not affect version 1 code or acceptance.

### Unreachable-definition link invariant

Use this exact denylist:

1. `gTilesetTiles_SecretBaseBrownCaveCompressed`
2. `gTilesetTiles_SecretBaseTreeCompressed`
3. `gTilesetTiles_SecretBaseShrubCompressed`
4. `gTilesetTiles_SecretBaseBlueCaveCompressed`
5. `gTilesetTiles_SecretBaseYellowCaveCompressed`
6. `gTilesetTiles_SecretBaseRedCaveCompressed`
7. `gTilesetTiles_UnknownSecretBase`
8. `gTilesetTiles_UnknownCableClub`

The production Wayfarer release keeps its existing LTO and
`--gc-sections` behavior. Do not add any denylisted section to a linker
`KEEP()` rule. The task-base linker already places all eight definitions in
discarded input sections at address zero, and the final ELF symbol table omits
them. Their 29,044 input bytes are not a new release saving.

Add a build guard with these checks:

1. An identifier-aware scan of compiled source expects one definition of each
   denylisted symbol and no identifier use outside that definition. Ignore
   comments, documentation, build outputs, and string literals.
2. The production release linker map may mention a denylisted input section
   only as discarded. It must not assign one an address in any ROM output
   section.
3. `arm-none-eabi-nm` on the final production ELF must find none of the eight
   symbols.
4. The eight source PNG paths remain present: the six
   `secondary/secret_base/*/unused_tiles.png` files,
   `primary/secret_base/unknown_tiles.png`, and
   `secondary/cable_club/unknown_tiles.png`.

If a future change references or deliberately restores one of these symbols,
the guard fails. That change must remove the symbol from the denylist in an
explicit content review and update the ROM baseline. The implementation must
not weaken the guard merely to make a new reference link.

### Header and reference checks

An optimized-build static or ELF-backed test must prove:

- `gTileset_SecretBase.isCompressed` is `TRUE` and its `tiles` pointer is
  `gTilesetTiles_SecretBase`.
- `gTileset_CableClub.isCompressed` is `TRUE` and its `tiles` pointer is
  `gTilesetTiles_CableClub`.
- The two symbol definitions include the manifest's exact compressed paths.
- The compressed files decode to the active raw inputs, not a legacy or
  unknown input.
- Each selected asset has no direct code or data consumer other than its
  definition and tileset header.
- Every Secret Base and Cable Club layout that uses the selected assets retains
  `layout_version: emerald`.

The raw-mode version of the same test expects the two flags to be `FALSE`, the
same header pointers, and the exact active raw paths.

### Runtime path

`struct Tileset` already contains the `isCompressed` bit. No structure layout,
map header, map ID, or save representation changes.

The existing paths remain authoritative:

1. `CopyTilesetToVram` sends a raw asset to `LoadBgTiles` and a compressed
   asset to `DecompressAndCopyTileDataToVram`.
2. `CopyTilesetToVramUsingHeap` sends a raw asset to `LoadBgTiles` and a
   compressed asset to `DecompressAndLoadBgGfxUsingHeap`.
3. `CopyPrimaryTilesetToVram` and `CopySecondaryTilesetToVram` exercise the
   temporary-buffer path.
4. `CopySecondaryTilesetToVramUsingHeap` and `CopyMapTilesetsToVram` exercise
   the explicit free-after-DMA task path.

The observed callers form separate validation routes:

- `InitMapView` calls `CopyMapTilesetsToVram` for both tilesets.
- `LoadMapInStepsLocal`, `LoadMapInStepsLink`, and `ReturnToFieldLink` call the
  primary and secondary temporary-buffer functions and then wait for
  `FreeTempTileDataBuffersIfPossible`.
- The field-resume path after `InitMap` calls
  `CopySecondaryTilesetToVramUsingHeap`.

Tests must exercise each route with an affected header in the primary or
secondary position it actually occupies. The implementation must not assume
that `CopyMapTilesetsToVram` is the only normal map-loading route.

The commonly called "non-heap" path is not allocation-free for compressed
data. It allocates through `malloc_and_decompress` and records the buffer in
the temporary tile-data table. In this specification, "temporary-buffer path"
means that ownership model; "explicit heap path" means the free-after-DMA task
model.

No selected symbol has a direct runtime consumer outside its tileset header.
Normal map loading must remain the only asset-loading route.

### Memory ownership and copy bounds

`malloc_and_decompress` reads the decoded length from the compression header,
allocates exactly that many WRAM bytes, and decompresses into the allocation.
For both selected assets the length must be 16,384.

The selected GBA LZ77 decoder writes directly to that output allocation. Tests
must still calculate peak temporary memory as the current decoded output plus
every earlier output still owned by a queued DMA request.

The requested VRAM ranges are also exactly 16,384 bytes:

- On the audited Emerald Secret Base layouts, the primary copy is tiles 0
  through 511 at byte offset 0.
- On the audited Emerald Cable Club layouts, the secondary copy is tiles 512
  through 1023 at byte offset 16,384.

The temporary-buffer path queues the copy, retains the allocation in
`sTempTileDataBuffer`, and frees it only after
`IsDma3ManagerBusyWithBgCopy()` becomes false. The explicit heap path creates
`task_free_buf_after_copying_tile_data_to_vram`, stores the queued request and
pointer in the task, and frees the allocation only after the DMA manager has
space, which indicates the queued transfer has completed.

Tests must account for overlapping lifetimes when normal map loading queues
primary and secondary tilesets in sequence. Record peak heap use for affected
transitions and prove all allocations succeed under the same game state in
which the raw baseline succeeds. The change adds no permanent allocation.

The explicit heap helper creates one free-after-DMA task after each successful
compressed allocation. `CreateTask` does not provide safe release recovery
when all 16 task slots are occupied: its assertion is disabled in release and
the returned task ID can overwrite task 0 state. Instrument every affected
explicit-heap route immediately before the copy calls. Require at least as
many free task slots as compressed tilesets about to be queued, which is two
for the paired primary and secondary route. Record the active count and prove
the required headroom in the highest-task affected state. Task exhaustion is a
release blocker. Safe runtime recovery would require a separately approved
task or loader contract change; version 1 rolls back to raw mode instead.

If the decoded-output allocation fails, the existing helper queues no copy.
Fault injection must prove that this outer-allocation path does not read, DMA,
or free invalid memory. Any stale-VRAM visual outcome remains a release
blocker.

Malformed compressed input has no recoverable release status.
`DecompressDataWithHeaderWram` returns `void`, and the release build compiles
the existing decompression diagnostic to a no-op. The helper may continue and
queue the output buffer after the decompressor returns. Version 1 therefore
handles payload integrity at generation time: exact size, host decode, and
byte comparison are release prerequisites. Corruption tests run in test or
debug mode and must observe the existing diagnostic and block release. A
runtime no-copy guarantee for malformed data would require a separately
approved loader contract change. The response in this project is to keep or
restore raw mode.

### Production-equivalent measurement

Run baseline and optimized builds serially from the same commit, toolchain,
configuration, and generated content. The only allowed difference is
`TILESET_STORAGE_OPTIMIZATION` and the resulting two payloads and header bits.
Use clean build state between modes.

For each mode, archive:

- the final ELF;
- the linker map;
- the ROM size report;
- the commit and toolchain identity;
- the two generated payload sizes;
- the round-trip comparison result;
- the denylist guard result.

Read `__rom_end` from the linker map and `rom.used_bytes` from the generated
size report. Both deltas must agree. Do not compare `.gba` file sizes because
the release image is padded to 32 MiB.

The optimized production link must reduce both `__rom_end - 0x08000000` and
`rom.used_bytes` by at least 19,500 bytes. The threshold allows up to 100 bytes
of alignment or build metadata against the 19,600-byte gross payload result.
An unexplained difference between the two metrics, a smaller net saving, or a
delta attributed to unrelated sections fails acceptance.

For reference, the task-base raw release reports:

| Metric | Value |
| --- | ---: |
| `__rom_end` | `0x09F5B110` |
| `rom.used_bytes` | 32,878,864 |
| `rom.unused_bytes` | 675,568 |
| Unused bytes above 512 KiB reserve | 151,280 |

The former 48,800-byte gross estimate applies only to a link that also retains
the 29,044-byte denylisted inventory. The current production release does not.
The implementation report must keep that distinction explicit.

### Rollout and failure handling

The optimized mode becomes the default only after every validation gate below
passes. Keep raw mode buildable in CI or a release rehearsal until at least one
accepted emulator and hardware validation cycle completes.

Rollback uses `TILESET_STORAGE_OPTIMIZATION=0` and a clean production release
build. It restores the two raw `INCBIN_U32` paths and clears the two header
flags. It changes no save data and needs no migration. After rollback, repeat
the header, round-trip input, link-denylist, ROM-limit, and focused runtime
checks for the raw build.

Build validation fails closed. A bad size, decode error, byte mismatch,
unexpected reference, retained denylisted symbol, wrong header flag, wrong
pointer, wrong input path, or insufficient net delta must stop the optimized
release.

## Validation

### Static and build tests

1. Verify the closed two-entry manifest, exact source paths, fixed codecs, and
   16,384-byte decoded lengths.
2. Generate, host-decode, and compare each selected asset byte for byte on a
   clean build.
3. Verify optimized and raw header flags and pointers.
4. Reject any selected path containing `unused_tiles` or `unknown_tiles`.
5. Verify all six short active Secret Base sheets remain raw and 2,656 bytes.
6. Verify the eight-symbol denylist has no live source reference, no production
   output address, and no final ELF symbol while all eight source assets remain.
7. Verify the affected layouts remain Emerald layouts and therefore request
   exactly 16 KiB for the selected primary or secondary range. Enumerate all
   24 Secret Base layouts, consisting of Red Cave, Brown Cave, Blue Cave,
   Yellow Cave, Tree, and Shrub variants 1 through 4, and all seven Cable Club
   users: `BattleColosseum_2P_Layout`, `TradeCenter_Layout`,
   `TradeCenter_hns_Layout`, `BattleColosseum_2P_hns_Layout`,
   `RecordCorner_Layout`, `BattleColosseum_4P_Layout`, and
   `EverGrandeCity_HallOfFame_Layout`.

### Runtime and memory tests

1. Exercise `LoadMapInStepsLocal`, `LoadMapInStepsLink`, and
   `ReturnToFieldLink` with affected layouts, covering the primary and
   secondary temporary-buffer functions and their wait-for-free step.
2. Exercise `InitMapView` through `CopyMapTilesetsToVram` and the field-resume
   path through `CopySecondaryTilesetToVramUsingHeap` with Cable Club as the
   affected secondary tileset.
3. Capture the used primary or secondary BG VRAM tile range after DMA and
   compare every byte between raw and optimized builds.
4. Instrument allocation size, decoded size, queued copy size, DMA request
   completion, free count, active task count, free task slots, and peak
   overlapping heap use. Expect one free per successful allocation and no free
   before completion. Require two free task slots before a paired explicit-heap
   copy.
5. Inject decoded-output allocation failure before each selected decode.
   Assert no overread, overwrite, use-after-free, double free, leaked task, or
   DMA from invalid memory.
6. Fill all task slots in a test or debug build before an affected explicit
   heap route and verify the existing diagnostic fires. Treat the condition as
   a release blocker; do not claim safe runtime recovery from task exhaustion.
7. Corrupt a test-only copy of each payload in a test or debug build and verify
   that the existing decompression diagnostic fires. This test documents that
   release has no recoverable status and must not claim a runtime no-copy
   guarantee.
8. Measure latency for a Red Cave Secret Base entry, Tree Secret Base entry,
   Cable Club entry, and `ReturnToFieldLink`. Use the same production build
   settings, save, deterministic input trace, emulator or hardware setup, and
   instrumentation in both modes. For each scene, discard one warm-up and run
   30 trials from the frame the load state starts through the first frame that
   field input is enabled and the BG copy DMA manager is idle. The optimized
   median and worst trial may be at most one frame above the corresponding raw
   result, with no additional missed VBlank or multi-frame stall.

### Content and integration tests

1. Load all 24 affected Secret Base layouts: variants 1 through 4 of Brown
   Cave, Tree, Shrub, Blue Cave, Yellow Cave, and Red Cave. Cover every cave,
   tree, and shrub entrance type across that matrix.
2. Enter and exit, place and remove decorations, save, reload, and repeat
   relevant Secret Base transitions.
3. Load all seven affected Cable Club users named in the static layout test.
   Cover Cable Club entry, exit, room transitions, trade, battle, and record
   mixing with the supported link setup.
4. Compare screenshots or frame captures and used VRAM ranges between raw and
   optimized builds. Tiles, palettes, metatiles, attributes, and animations
   must match.
5. Run on an accurate emulator and on GBA hardware or a hardware-accurate
   platform. Check decompression diagnostics, heap telemetry, DMA completion,
   transition timing, and link behavior.
6. Confirm no map ID, map content, script, save format, or persistent behavior
   changes.

### Link and size tests

1. Produce clean serial raw and optimized Wayfarer release links.
2. Compare `__rom_end` and `rom.used_bytes`; require matching deltas of at
   least 19,500 bytes.
3. Confirm the final `.gba` padding does not enter the comparison.
4. Attribute the delta to the two selected payloads. The denylisted 29,044
   bytes remain absent in both modes and contribute zero new saving.
5. Run the normal Wayfarer release ROM-limit report and retain at least the
   active reserve required by the runtime foundation specification.

## Implementation sequence

Implementation happens later, in this order:

1. Add the two-entry audit manifest and host-side validation that checks raw
   size, decoded size, active input identity, and byte-for-byte round trips.
2. Add the narrow `TILESET_STORAGE_OPTIMIZATION` build option and validate its
   value without changing other build modes.
3. Wire the two existing suffix rules to the selected active inputs. Update
   only the two `INCBIN_U32` definitions and two `isCompressed` values behind
   the build option.
4. Add the eight-symbol identifier and production-link guard. Preserve every
   source asset and existing release garbage collection.
5. Add focused temporary-buffer, explicit heap, DMA-lifetime, failure, VRAM,
   task-headroom, and content-preservation tests.
6. Produce clean serial raw and optimized release artifacts and record the net
   `__rom_end` and `rom.used_bytes` deltas.
7. Run the Secret Base and Cable Club emulator and hardware validation matrix.
8. Make optimized mode the release default only after all gates pass. Use raw
   mode for rollback if any later regression appears.

No production implementation or pull request is part of this documentation
change.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
