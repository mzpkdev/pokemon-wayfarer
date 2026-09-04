# Compressed map layout validation and rollout

PRD: [Content-preserving map layout compression](../prds/compressed-map-layouts.md)
Implemented: No

## Scope

This specification defines the evidence required to enable content-preserving layout
compression. It owns build verification, legacy-loader differentials, feature journeys,
failure and bounds tests, memory and timing instrumentation, compatibility, staged
migration, rollback, acceptance records, and the later implementation sequence.

The [build and storage specification](compressed-map-layout-build-storage.md) defines
the generated payload and descriptor. The
[runtime loading specification](compressed-map-layout-runtime-loading.md) defines the
API, decode lifecycle, connection algorithm, and failure contract.

## Behavior

### Validation builds

The implementation provides four deterministic test configurations from the same
source revision:

| Configuration | Purpose |
| --- | --- |
| Legacy size baseline | Compile the pre-feature raw layout record and loader with all compression infrastructure absent. It uses current selected content and production flags only to measure true net ROM change and may supply the test oracle. |
| Raw control | Generate descriptors but link and load every layout as raw. This is the behavior compatibility baseline and release rollback, not the net-size baseline. |
| Hybrid candidate | Apply the current staged raw/compressed policy and production loader. This is the candidate used for net-ROM and playable acceptance. |
| Differential test | Test-only image or mechanics target that retains raw oracle inputs and both loader paths. It never represents production ROM size. |

The legacy size baseline, raw control, and hybrid candidate use the same product,
compiler, linker, release flags, content revision, and non-layout feature settings. The
legacy baseline compiles out every new descriptor, checksum table, codec path, error
path, and other compression-specific linked byte. The differential target may
retain duplicate data only to compare behavior. Reports must never count that test-only
image as the shipping size result.

A dedicated layout-compression setting chooses raw control or staged hybrid behavior.
It does not overload `TESTING`, `E2E`, or `RELEASE`. Test-only oracle and fault-injection
settings cannot be enabled in a production build.

### Build-time verification

Every normal hybrid build performs the complete-file round trip described by the build
specification. For each compressed entry, it decodes the generated payload with an
independent verifier and compares every byte with the unchanged source `map.bin`.
Comparison includes any bytes after the logical tile prefix.

Build tests also cover:

- deterministic regeneration from the same inputs;
- raw and compressed policy selection, including alignment-sensitive no-saving cases;
- catalog additions, exclusions, `NULL` slots, ordering, and stable one-based IDs;
- complete dependencies on every source payload, policy, compressor, verifier, and
  generator input;
- known-answer checksums and LZ77 fixtures;
- source files with trailing bytes after the logical tile data; and
- every generator refusal defined in the build specification.

The round-trip result and manifest are CI artifacts. A summary without the per-layout
machine-readable rows is insufficient for release investigation.

### Full-catalog loader differential

The mechanics test build keeps the current raw loader as a test-only oracle. Production
does not retain the legacy loader or raw copies solely for comparison.

The differential suite has two exhaustive passes:

1. For every selected layout descriptor that satisfies the catalog's runtime bounds,
   construct a synthetic header with no connections. Run the legacy raw loader and new
   loader from separately reset state.
2. For every selected real map header, run both loaders with its real layout and complete
   connection list in the same order.

Each comparison includes:

- `gBackupMapLayout.width` and `height`;
- the expected ownership of `gBackupMapLayout.map`;
- all 20,480 bytes of `sBackupMapData`, including undefined padding, the current map,
  connected strips, clipped areas, and bytes outside the logical grid; and
- connection flags or other loader output consumed after initialization.

An exerciseable catalog layout means one that the selected build can resolve and whose
declared dimensions pass the same checked backup-map bound as production. A selected
entry that cannot be exercised is a test failure unless the manifest identifies a
procedural-only reason and a dedicated oracle below covers it. The report lists every
selected layout and map header as passed, dedicated, or failed. It has no silent skip
category.

Connection differentials cover every real connection plus synthetic cases for north,
south, west, and east; zero, positive, and negative offsets; exact, partial, and no
overlap; minimum dimensions; destination clipping; multiple connections and their
ordering; and missing or invalid neighbors. Tests pin the east strip at 8 columns and
the other border depth at 7.

### Direct-consumer differential

The following tests compare raw control output with the abstraction-backed path:

| Consumer | Required comparison |
| --- | --- |
| Battle Pyramid | Generated backup dimensions, every floor block, entrance and exit placement, collision and elevation, and player position where applicable for every template combination the generator can select. |
| Trainer Hill | Entrance and exit rows, generated floor blocks, dimensions, scripts reached after generation, and every supported floor ID. |
| Secret Bases | Immutable entrance and PC searches before and after appearance and decoration overlays, including saved-base reload. |
| Decorations | Placement and removal for sprite and non-sprite decorations, with restored tiles compared against immutable authored data. |

Keeping one of these layouts raw does not waive its API and differential tests. The
tests prove that hybrid storage policy and runtime access are separate decisions.

### Failure and bounds suite

Fault-injection fixtures cover at least:

- zero, unknown, or excluded layout ID;
- missing descriptor or payload;
- unknown schema, codec, required flag, or policy value;
- null, misaligned, or out-of-ROM-section payload address;
- zero, odd, overflowing, or inconsistent stored, decoded, logical, width, height,
  rectangle, stride, and destination sizes;
- logical tile bytes greater than decoded file bytes;
- a padded backup grid greater than `MAX_MAP_DATA_SIZE`;
- a bad LZ header byte or output size;
- truncated literal or backreference token, source overread, output overrun, invalid
  backreference, early end, and trailing encoded data where the schema forbids it;
- stored and decoded checksum mismatches;
- generated maximum smaller than a descriptor request;
- allocator failure, insufficient contiguous space, and forced failure after an earlier
  connection succeeds; and
- immutable-view misuse, including nesting, use after release, double release, and a
  pointer retained by a task or global test fixture.

Every case asserts the stable `MapLayoutLoadError`, zero decoder calls when preflight
fails, an undefined backup buffer, no scripts or saved-view application, no player
control, and no save write. Top-level tests assert that `gMain.callback2` becomes
`CB2_MapLayoutLoadError`, `gMapLayoutLoadError` contains the expected error and
identities, ordinary input cannot resume play, soft reset still works, and the screen
allocates no heap memory. The mechanics runner must also report an intact heap, no leaked
allocation, and no uncleared task after the case.

### Gameplay and content suite

ROM-backed E2E tests use the repository's isolated `GameSession` and SkyEmu workflow for
functional behavior. Each required journey runs against the hybrid candidate. A focused
raw-control run supplies the rollback comparison where the differential mechanics suite
cannot observe player-visible behavior.

The acceptance matrix includes:

| Area | Required journeys |
| --- | --- |
| Warps | Door, cave, ladder, scripted, and cross-region warps into representative raw and compressed indoor, outdoor, cave, and special layouts. Verify arrival coordinates, facing, collision, and surrounding blocks. |
| Fly | Fly into each supported region and into representative raw and compressed destination layouts. Verify map identity, arrival, scripts, and return transition. |
| Connections | Walk across real connections in all four directions, including offset and connection-heavy cases. Cross back and inspect the seam from both sides. |
| Save reload | Save on raw and compressed maps, reboot, continue through the real save path, and compare map, position, saved map-view changes, objects, decorations, and persistent state. Repeat the resulting save on the raw rollback build. |
| Secret Bases | Create, enter, decorate, save, reload, open or close the entrance, find the PC, and leave without an authored-tile difference. |
| Decorations | Place and remove sprite and non-sprite decorations, including overlap-sensitive restoration and save reload. |
| Trainer Hill | Enter, load every supported floor class, traverse, save or reload where supported, and exit with the same floor and state as raw control. |
| Battle Pyramid | Generate representative floors across template combinations, traverse entrance and exit, trigger battles and items, and confirm layout and saved facility state match raw control. |
| Failure isolation | Test-only corrupt and allocation-failure builds reach the deliberate error state without exposing a partial map or damaging a save. |

The suite compares metatile IDs, collision, and elevation where the E2E facade can read
them. Visual snapshots alone are not enough. Existing campaign, traversal, encounter,
and map-load suites remain required because a catalog or identifier regression can
appear outside the focused journeys.

### Save and binary compatibility

Static assertions and paired artifacts prove that the feature does not change any save
structure, save-sector allocation, map ID, layout ID, persistent identifier, or saved
map-view representation. Tests use at least these paths:

1. Create a save with the raw control, continue it with the hybrid candidate, save again,
   then continue it with the raw control.
2. Repeat on a map with saved map-view edits and on a decorated Secret Base.
3. Compare serialized save fields before and after each load, allowing only state changes
   caused by the explicit test actions.

This contract preserves saves that are compatible with the source revision. It adds no
new promise to migrate unrelated prerelease save-format changes.

If test instrumentation changes the E2E ABI, its version, C size and offset assertions,
and TypeScript protocol tests change together. Production compression descriptors do
not enter that ABI.

### Memory proof

Static and runtime evidence are both required.

The paired production ELF report records `.ewram`, `.ewram.sbss`, IWRAM, stack reserve,
`sBackupMapData`, and `gHeap`. The current evidence is 248,484 static EWRAM bytes out of
256 KiB, with `sBackupMapData` at 20,480 bytes and `gHeap` at `0x1C500` bytes inside that
total. The candidate may add small scalar loader state, but it must add no static array
or reserved region sized to a complete layout.

Test instrumentation records allocator topology and high-water use immediately before
the load context, after its one allocation, after each current or neighbor decode, after
release, and before tileset decompression. The matrix includes:

- the current 14,640-byte largest layout as current map and as connected neighbor where
  a synthetic safe header is needed;
- the real map header with the most connections;
- the smallest observed contiguous free heap under full warp, camera connection,
  Fly, and save-reload paths;
- repeated alternating large and small loads to expose fragmentation;
- each special direct-consumer path; and
- injected allocation failure at and below the requested size.

For every successful case, the single requested buffer fits the smallest contiguous free
block, no second layout buffer exists, heap structure after release matches its pre-load
state, and subsequent tileset allocations still succeed. Stack guards remain intact
through the decoder's peak. The report retains exact requested, free, high-water, and
post-release values. An average or empty-heap measurement does not pass this gate.

### Timing proof

Functional SkyEmu runs do not approve performance. Before each rollout promotion, run
paired raw and hybrid production-equivalent builds on an approved accurate GBA emulator
or physical hardware. Record the ROM revision and checksum, emulator or hardware model,
measurement code revision, timer source, sample count, and raw samples.

Use the existing in-ROM benchmark timer or an equally precise cycle counter around:

- descriptor validation;
- stored checksum and stream preflight;
- decode and decoded checksum;
- current-map copy;
- each connected neighbor; and
- complete layout initialization through connection completion.

Measure at least the largest compressed layout, the connection-heaviest real map, a
representative indoor warp, a Fly arrival, and save reload. Run at least 100 loads per
case after a fixed setup, report median, 95th percentile, and maximum, and compare paired
raw-control samples.

At the 95th percentile, added complete-layout time must be no more than one video frame.
The measured maximum must add no more than two frames. No case may show a visible fade
stall, input-release delay, or audio interruption. Any miss blocks promotion even if ROM
and functional gates pass.

### ROM proof

Use paired production-equivalent Wayfarer builds and the existing ROM report. Retain
total `used_bytes`, final ROM end, and category totals for the legacy size baseline and
hybrid candidate. The `maps_layouts` category diagnoses payload movement; the whole-ROM
difference is the net result. The raw control report is retained for rollback sizing but
does not define net saving because it shares feature overhead with the candidate.

The reference 991-layout audit measured 1,254,526 bytes of gross payload saving, about
72 percent. Release approval does not call this net. The candidate must reduce total
linked ROM use by at least 1,048,576 bytes after code, descriptors, checksums, alignment,
raw exceptions, and all other overhead.

### Instrumentation records

CI retains the build manifest, round-trip results, differential catalog matrix,
mechanics test report, E2E report, paired ROM reports, and paired production ELF memory
reports. Accurate-emulator or hardware timing samples are attached to the release
candidate record because the current CI has no hardware timing lane.

Runtime profiling uses test output rather than save fields or gameplay telemetry. No
player save is modified to collect rollout data.

## Staged migration

### Stage 0: shadow generation

All runtime descriptors select raw. The build generates compressed candidates, verifies
round trips, and emits manifests and paired size estimates. The raw abstraction replaces
every public direct payload access before the stage ends.

Promotion requires deterministic generation, no source-file changes, complete build
tests, a clean source guard, and raw abstraction differentials for all current consumers.

### Stage 1: unconnected canary

A small allowlist of ordinary layouts that appear at neither the source nor destination
end of any selected `MapConnection`, and that have no special immutable consumer,
selects compressed storage. The build derives both connection endpoint sets from the
selected map headers and rejects an ineligible canary rule. The allowlist includes varied
sizes and layout versions. All other layouts remain raw.

Promotion requires complete canary differentials, failure tests, warps, save reload,
memory proof, timing proof, and a successful raw rollback drill.

### Stage 2: connected layouts

Expand the allowlist to real outdoor connections in each direction and then to the
connection-heavy case. Decode neighbors one at a time through the shared context.

Promotion requires exhaustive real and synthetic connection differentials, two-way E2E
crossings, allocation and fragmentation proof, and timing within the stated thresholds.

### Stage 3: broad hybrid

Set eligible ordinary layouts to `auto`. Keep explicit raw exceptions for any special
consumer not yet approved. Migrate Battle Pyramid, Trainer Hill, Secret Base, and
decoration layouts independently after their dedicated differential and gameplay rows
pass.

Promotion requires a full no-skip catalog matrix, every gameplay journey, save round
trip in both build directions, all memory and timing reports, and at least 1 MiB net ROM
saving.

### Stage 4: default on

Hybrid becomes the normal Wayfarer release configuration only after all product
acceptance gates pass. Raw-only remains buildable as rollback until a later product
decision removes it.

## Rollback

Rollback is a build selection, not a source-data restoration:

1. Select raw-only layout storage on the same accepted source revision.
2. Build with production flags and run build, differential, compatibility, and focused
   E2E gates.
3. Continue a hybrid-build save in the raw image and verify the saved map and one
   decorated Secret Base.
4. Publish the raw artifact with its manifest and ROM report.

Trigger rollback for any unexplained loader difference, content or save regression,
unsafe bound, allocation or heap failure, corrupt-payload escape, timing miss, or net
saving below 1 MiB. Rollback does not edit `map.bin`, renumber layouts, or migrate saves.

## Acceptance gates

Release approval requires one evidence bundle showing:

1. unchanged Porymap source files and a complete deterministic build manifest;
2. exact build-time round trips for every compressed entry;
3. a no-skip full-catalog and real-map-header differential matrix, including connected
   edges and direct consumers;
4. passing malformed-input, bounds, ownership, allocation, cleanup, and release-error
   tests;
5. passing warps, Fly, connections, save reload, Secret Bases, decorations, Trainer
   Hill, and Battle Pyramid journeys;
6. unchanged save structures and successful raw to hybrid to raw save compatibility;
7. production ELF and live instrumentation proving no permanent full-map buffer and safe
   heap, stack, lifetime, and fragmentation behavior at peak;
8. accurate-emulator or hardware timing within one frame at the 95th percentile and two
   frames at maximum, without visible or audio disruption;
9. paired production-equivalent legacy-size and hybrid ROM reports showing at least
   1 MiB net saving after all new feature costs and labeling gross payload saving
   separately; and
10. a completed raw rollback drill from the candidate revision.

One failed or missing row blocks promotion. An allowlisted raw exception may remain only
when its rationale and dedicated tests are present and the final build still meets the
net-ROM gate.

## References

- [Compressed map layout build and storage](compressed-map-layout-build-storage.md)
- [Compressed map layout runtime loading](compressed-map-layout-runtime-loading.md)
- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Mechanics test runner](../../game/test/test_runner.c)
- [Mechanics benchmark interface](../../game/include/test/test.h)
- [Wayfarer map-load E2E journey](../../e2e/src/journeys/wayfarer-hoenn-map-load.e2e.ts)
- [E2E harness](../../e2e/README.md)
- [CI workflow](../../.github/workflows/ci.yml)

## Implementation sequence

The later implementation should use these dependency boundaries:

| Order | Workstream | Depends on | Completion output |
| ---: | --- | --- | --- |
| 1 | Baseline and contracts | Approved documentation | Reproducible legacy-size and raw-control catalog, ROM, ELF, heap, timing, and direct-access audit; frozen descriptor and error enums. |
| 2A | Build format and generator | Step 1 | Independent payloads, descriptors, policy manifest, complete-file round trips, generated bounds, and reports. |
| 2B | Raw runtime abstraction | Step 1 | Opaque `MapLayout` payload, raw full and rectangle copies, scoped views, migrated consumers, source guard, and unchanged raw behavior. |
| 2C | Test scaffolding | Step 1 | Legacy oracle seam, catalog enumerator, fault fixtures, heap telemetry, timing hooks, and E2E journey skeletons. |
| 3 | Compressed loader | Steps 2A and 2B | Descriptor preflight, integrity checks, one-buffer load context, full copy, scoped view, release-safe failure path, and cleanup. |
| 4 | Connected-map loading | Step 3 | Sequential neighbor decode with legacy strip and clipping behavior in every direction. |
| 5A | Differential and failure gates | Steps 2C through 4 | No-skip catalog and map-header results, malformed-input results, and memory ownership proof. |
| 5B | Special consumer gates | Steps 2B through 4 | Battle Pyramid, Trainer Hill, Secret Base, and decoration differentials with reviewed raw exceptions. |
| 6 | Canary rollout | Steps 5A and 5B | Stage 1 allowlist, paired ROM and ELF reports, playable journeys, accurate timing, and rollback drill. |
| 7 | Connected and broad rollout | Step 6 | Stage 2 then Stage 3 evidence, exception-by-exception migration, at least 1 MiB linked net saving, and complete acceptance bundle. |
| 8 | Default-on decision | Step 7 | Stage 4 release setting with raw rollback still buildable. |

Steps 2A, 2B, and 2C can run in parallel after the descriptor, ownership, error, and
measurement contracts are frozen. Steps 5A and 5B can run in parallel once connected
loading exists. Each rollout step consumes the retained outputs of the prior step; it
must not infer success from a later aggregate build.
