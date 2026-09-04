# Content-preserving map layout compression

## Intent

Pokemon Wayfarer's combined map catalog consumes enough ROM that future required
content is at risk of competing with existing maps for space. Map layout block data is
a strong compression target because it is repetitive, read-only source content that the
engine already copies into a mutable map buffer when a map loads.

This feature reduces the ROM occupied by layout `map.bin` data without removing,
simplifying, or changing any authored content. A player must encounter the same maps,
tiles, collision, elevation, connections, warps, and persistent world state before and
after the migration. Compression is successful only if it creates at least 1 MiB of net
ROM space after the loader, metadata, checksums, alignment, raw exceptions, and other
shipping overhead are counted.

## Design

### User problem

Wayfarer needs ROM headroom for required content. Deleting maps or reducing their detail
would turn a storage problem into a smaller game. That trade is not acceptable: maps are
gameplay data, not disposable packaging, and even a visually minor block change can alter
collision, elevation, encounters, scripted positioning, Secret Bases, or a connection
seam.

The project therefore stores eligible layout block data more compactly in the ROM and
reconstructs the exact bytes when the engine needs them. Source `map.bin` files remain
unchanged and usable in Porymap.

### Player-visible behavior

Players should not be able to tell whether a layout is raw or compressed. In both modes:

- every map has the same dimensions, metatiles, collision, elevation, border, tilesets,
  events, and scripts;
- normal warps, Fly, outdoor map connections, save reload, and camera transitions arrive
  at the same position and display the same surrounding blocks;
- Secret Bases and decorations keep their authored and saved behavior;
- Trainer Hill and Battle Pyramid generate the same playable floors; and
- save data, map identifiers, map layout identifiers, and persistent state keep their
  meanings.

No menu, option, save migration prompt, or new player-facing terminology is part of this
feature. A corrupt or unsupported layout payload must fail before player control reaches
a partial map. It must produce a deliberate diagnostic failure rather than allow the
player to walk on missing or unverified blocks.

### Storage behavior

Each layout is independently addressable and may be stored raw or with the project's GBA
LZ77 format. A regional table may organize those entries, but loading one map never
decompresses an entire region. The build chooses the storage kind per layout and records
enough generated metadata to validate and load it safely.

The initial migration is hybrid. Ordinary layouts can move to compressed storage first.
Procedural templates and layouts whose callers still need a scoped immutable source view
may remain raw until their access path and tests are ready. Raw status is not permission
to bypass the common layout API.

Outdoor connections are part of map loading, not a follow-up optimization. The loader
uses one bounded temporary buffer for a compressed current layout or one compressed
neighbor at a time. It copies the required connection strip into the existing
`gBackupMapLayout`, reuses the allocation sequentially, and frees it before later
tileset work. It never holds decompressed layouts for a whole region and never adds a
permanent full-map EWRAM buffer.

### Measured opportunity

The reference audit measured the following 991-layout catalog with the project's GBA
LZ77 compressor:

| Measure | Result |
| --- | ---: |
| Layout `map.bin` blobs | 991 |
| Raw bytes | 1,742,526 |
| Compressed payload bytes | 488,000 |
| Gross payload saving | 1,254,526 bytes |
| Gross payload reduction | 71.99%, about 72% |
| Largest current `map.bin` | 14,640 bytes |

The 1,254,526-byte result is gross, not net. It excludes loader code, descriptors,
checksums, alignment, raw exceptions, and any retained rollback data. The catalog is
mutable, so implementation and release builds must regenerate the report from their own
selected layouts. Neither this baseline nor a later compressor total can substitute for
the final linked-ROM comparison.

The isolated documentation branch selects 987 of these layouts: 1,741,356 raw bytes and
487,320 compressed bytes. The four-entry difference is 1,170 raw bytes and 680 compressed
bytes, which exactly reconciles it with the 991-layout linked audit. Both reproductions
round-trip byte for byte. Release evidence must identify its exact catalog manifest so a
reader never has to infer which count a size result covers. The build specification
records the four paths, per-file totals, and reproduction method.

## Boundaries

### In scope

- Wayfarer's generated layout catalog and the `map.bin` payloads it links.
- Per-layout raw or GBA LZ77 storage selected at build time.
- Generated format, size, integrity, and storage-kind metadata.
- One runtime abstraction for full copies, rectangular copies, and tightly scoped
  immutable access.
- Normal map loading, connected-map borders, Battle Pyramid, Trainer Hill, Secret Bases,
  and decorations.
- Build-time round trips, old-loader versus new-loader differential tests, memory and
  latency measurement, failure tests, instrumentation, staged rollout, and rollback.

### Non-goals

- Editing, repacking, or replacing source `map.bin` files.
- Removing maps, details, events, scripts, connections, encounters, NPCs, or story
  content to save space.
- Compressing a region as one unit or keeping a decompressed regional cache.
- Adding a permanent full-map EWRAM buffer.
- Changing map geometry, Porymap workflows, save structures, save identifiers, or the
  meaning of saved map state.
- Replacing GBA LZ77 with a new general-purpose compression algorithm in the first
  migration.
- Compressing borders, tilesets, scripts, graphics, or unrelated assets under this
  feature.
- Requiring every unusual or procedural layout to be compressed in the first release.

## Constraints

`MAX_MAP_DATA_SIZE` is 10,240 `u16` entries. The existing `sBackupMapData` allocation is
20,480 bytes and remains the sole permanent full loaded-map buffer. A measured Wayfarer
ELF reports 248,484 bytes of static EWRAM allocation out of 256 KiB. The `gHeap` array is
`0x1C500` bytes within that allocation, not spare memory outside it. The 14,640-byte
largest current layout is larger than the remaining static EWRAM headroom, so another
permanent map-sized allocation is forbidden.

A temporary decode allocation must be exact-size or generated-bound-size, measured at
the real map-load peak, reused rather than multiplied for connections, and freed before
the next heap-heavy loading phase. Total heap capacity is not evidence that the
allocation is safe. The acceptance measurement must cover live field allocations,
fragmentation, the largest layout, and a connection-heavy transition.

The engine already provides WRAM LZ77 decompression functions. Their presence does not
remove the need to validate descriptor bounds, payload integrity, output size, and
failure propagation before using their output.

## Success metrics

The feature succeeds when all of these statements are true:

- the shipping compressed build is at least 1,048,576 ROM bytes smaller than a
  production-equivalent legacy raw size baseline made from the same content revision and
  configuration, with all new loader, descriptor, checksum, format, and policy overhead
  absent from the baseline;
- every compressed payload round-trips to byte-for-byte equality with its source
  `map.bin` during the build;
- every exerciseable catalog layout produces the same complete loaded backup map under
  the old and new loaders, including connection edges;
- the required gameplay journeys have no layout, collision, persistence, or transition
  regressions;
- the production build adds no permanent map-sized EWRAM allocation and the measured
  map-load peak has enough contiguous heap for the bounded temporary allocation;
- temporary memory is released on success and every failure path, with no heap damage or
  leak;
- on accurate emulation or hardware, the added map-load work stays within one frame at
  the 95th percentile and two frames at the measured maximum, with no visible fade or
  audio disruption; and
- the raw rollback build remains buildable and passes its compatibility suite.

ROM reduction is measured from linked production ROMs, not by summing source files.
Functional emulator runs can prove behavior but do not satisfy the hardware-timing gate.

## Risks

### Raw-pointer regression

The current `MapLayout.map` type invites code to assume that every payload is raw ROM
`u16` data. A future direct dereference could work for raw exceptions and fail only on a
compressed layout. The migration must remove public typed payload access, route all
consumers through the layout API, and add an automated source guard. Hybrid support
makes this guard more important, not less.

### Connection seams

The normal loader reads neighboring layouts directly, including asymmetric strip widths
at the east edge and offset clipping in every direction. Testing only the center of the
current map can miss a wrong row, wrong stride, or one-tile seam. Actual connection data
and synthetic boundary cases both need differential coverage.

### Peak memory and fragmentation

A 14,640-byte allocation may fit an empty `gHeap` and still fail during a live camera
transition. Holding it through tileset decompression would make the peak worse. Runtime
ownership must be scoped to layout copying, and rollout stops if instrumentation cannot
prove adequate contiguous space at every measured load path.

### Corrupt or mismatched metadata

The existing LZ77 routines trust their input. Bad sizes, a truncated payload, an unknown
format version, or a damaged backreference can otherwise produce an out-of-bounds write
or a partial map. The generated descriptor and loader must fail closed before unsafe
decode and verify the reconstructed bytes before committing them to the live map.

### Misleading space reports

The gross audit leaves about 205,950 bytes between its payload saving and the 1 MiB net
gate. Duplicated edge data, large descriptors, retained raw payloads, or code growth can
consume that margin. Only a paired linked-ROM report can approve rollout.

### Maintenance drift

New map generators, procedural facilities, or content tools may introduce another raw
consumer. The API contract, generated manifest, review checklist, and CI source guard
must make such additions fail visibly. A permanent exception list without owners and
rationale would hide drift, so each raw exception requires a reason and a test that
proves it still needs special handling.

## Staged rollout and rollback

| Stage | Runtime selection | Entry gate | Exit gate |
| --- | --- | --- | --- |
| 0. Shadow generation | All layouts remain raw. The build emits descriptors, compressed candidates, reports, and round-trip checks. | Generator and format review. | Deterministic output and complete round-trip report. |
| 1. Canary | A small reviewed set of ordinary layouts that appear at neither end of any selected map connection uses compressed storage. Special direct-access layouts remain raw. | Raw abstraction is active for every caller; negative tests pass. | Differential, memory, timing, and representative warp tests pass. |
| 2. Connected maps | Ordinary connected layouts migrate by allowlist. Neighbors decode one at a time. | All connection directions, offsets, and clipping pass differential tests. | Connection-heavy E2E, peak-memory, and accurate-timing gates pass. |
| 3. Broad hybrid | Eligible catalog layouts default to compressed. Battle Pyramid, Trainer Hill, Secret Base, or decoration exceptions remain raw until their own tests pass. | Full catalog differential suite and required gameplay journeys pass. | At least 1 MiB measured net ROM saving and all acceptance gates pass. |
| 4. Default on | The validated hybrid policy is the normal Wayfarer release setting. | Release approval. | Ongoing CI and release reports remain green. |

A dedicated build setting selects raw or hybrid layout storage without changing source
content. Rollback rebuilds the same commit in raw mode. It requires no save conversion
because layouts, identifiers, and serialized structures do not change. Any byte
difference, bounds failure, heap failure, unexplained transition regression, or net
saving below 1 MiB blocks promotion and returns the next build to the last accepted
stage.

## Acceptance gates

1. The selected catalog is complete, every entry has one valid descriptor, and source
   `map.bin` files are unchanged and Porymap-compatible.
2. Every compressed entry passes build-time decompression and exact byte comparison.
3. Generated format version, codec, raw size, stored size, dimensions, and integrity
   values agree; malformed fixtures fail deterministically.
4. The public runtime API has no operation that returns an unbounded, permanent raw map
   pointer. All current direct consumers use the common abstraction, even when their
   layout remains raw.
5. The old and new loaders produce identical dimensions and all 20,480 bytes of the
   loaded backup buffer for every exerciseable layout and map header, including all real
   connections.
6. Warps, Fly, map connections, save and reload, Secret Bases, decorations, Trainer Hill,
   and Battle Pyramid pass targeted tests in raw and hybrid builds.
7. Invalid identifiers, dimensions, rectangles, formats, versions, headers, lengths,
   checksums, allocation failures, and output bounds fail without a partial live map,
   leak, or out-of-bounds write.
8. A current production ELF and runtime instrumentation prove the EWRAM, heap, stack,
   lifetime, and fragmentation constraints at map-load peak. No permanent full-map
   buffer is added.
9. Accurate-emulator or hardware results meet the latency metric and are retained with
   the tested ROM identity and measurement method.
10. Paired linked production-equivalent builds compare the hybrid candidate with a
    legacy raw size baseline that excludes all new compression infrastructure, proving
    at least 1 MiB net ROM saving after every code and data cost. The report labels gross
    payload reduction separately.
11. Save layout and persistent identifiers are unchanged, an existing compatible save
    reloads on the compressed build, and that save can return to the raw rollback build.
12. The raw build setting remains supported until the default-on stage has passed and a
    later product decision explicitly removes it.

## References

- [Compressed map layout build and storage](../specs/compressed-map-layout-build-storage.md)
- [Compressed map layout runtime loading](../specs/compressed-map-layout-runtime-loading.md)
- [Compressed map layout validation and rollout](../specs/compressed-map-layout-validation-rollout.md)
- [Wayfarer runtime foundation](../specs/wayfarer-runtime-foundation.md)
- [`MapLayout` and `BackupMapLayout`](../../game/include/global.fieldmap.h)
- [Current field map loader](../../game/src/fieldmap.c)
- [Current layout generator](../../game/tools/mapjson/mapjson.cpp)
