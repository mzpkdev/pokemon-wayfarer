# Compressed map layout runtime loading

PRD: [Content-preserving map layout compression](../prds/compressed-map-layouts.md)
Implemented: No

## Scope

This specification defines the runtime boundary for raw and compressed layout data. It
owns descriptor validation, decoding, temporary memory, full and rectangular copies,
connected-map behavior, direct-consumer migration, failure propagation, and save-visible
compatibility.

The [build and storage specification](compressed-map-layout-build-storage.md) defines
the generated descriptor and payload. The
[validation and rollout specification](compressed-map-layout-validation-rollout.md)
defines the oracle, memory, timing, gameplay, and release gates.

## Behavior

### Runtime invariant

`gBackupMapLayout` remains the only mutable loaded-map view used by field gameplay.
Its map pointer continues to reference the existing 20,480-byte `sBackupMapData` array.
The compressed payload is an immutable ROM representation, not a second active map.

Runtime code must not assume that a `MapLayout` payload is raw `u16` ROM data. The
public `MapLayout` shape contains an opaque descriptor reference instead of the current
typed `map` pointer. No public API returns a payload pointer with an unbounded lifetime.

Raw and compressed layouts have identical behavior after crossing the loader boundary.
Callers do not branch on codec, inspect LZ headers, or calculate payload addresses.

### Access API

The layout module provides three classes of operation. Exact function names may follow
project naming conventions, but their ownership and behavior are fixed here.

| Operation | Contract |
| --- | --- |
| Full copy | Copy the logical `width * height` tile prefix into a caller-provided destination with an explicit stride and bounds. Normal map loading uses this operation. |
| Rectangle copy | Copy a checked `(x, y, width, height)` tile rectangle into a caller-provided destination and stride. Connections and Trainer Hill use this operation. |
| Scoped immutable view | Acquire a read-only logical tile view for algorithms that need to scan or revisit source cells, then release it in the same synchronous call scope. |

Every operation takes a `MapLayout`, returns a status, and reports a stable error reason
in test and diagnostic builds. Rectangle arithmetic uses checked unsigned sizes. Zero
or out-of-range dimensions, source rectangles, destination strides, and destination
capacities fail before a copy.

A scoped view is either:

- a non-owning view of verified raw ROM data; or
- a view of verified decoded data owned by a load context.

The view is invalid after release or after the context opens another compressed layout.
It cannot be stored in `gMapHeader`, a task, a callback, saved data, or another global.
Nested compressed views are forbidden. Code that only needs a copy or a metatile search
uses a higher-level operation instead of acquiring a view.

The descriptor structure, codec dispatch, payload address, and unchecked tile pointer
are private to the layout module. A CI source rule rejects descriptor payload access and
the old `mapLayout->map` pattern outside that module. Retyping and renaming the field
makes stale direct dereferences fail at compile time.

### Load context and memory ownership

A synchronous load context owns all temporary decode memory for one map initialization.
Before allocation it inspects the current layout and the layouts referenced by that map's
connections. It requests one buffer large enough for the greatest `decoded file bytes`
among the compressed entries it may open. Raw entries require no decode allocation.

The context follows these rules:

- it allocates at most one full-layout temporary buffer;
- the requested bytes are no greater than the generated catalog maximum and are aligned
  for the selected WRAM decoder;
- it opens and decodes only one layout at a time;
- current-map and neighbor copies reuse the same allocation sequentially;
- no decoded pointer outlives the active copy or scoped view;
- every success and error path releases the allocation exactly once; and
- the allocation is released before secondary tileset loading or any later asynchronous
  heap-backed transfer begins.

The current largest source file is 14,640 bytes. That is a measurement case, not a fixed
capacity. A future larger layout updates the generated maximum and must pass peak-memory
and latency gates before compression is enabled for it.

The production image adds no static full-map buffer. The measured Wayfarer ELF already
allocates 248,484 of 262,144 EWRAM bytes. `gHeap`, sized at `0x1C500`, is part of that
total. Heap capacity cannot be added to the remaining static bytes when reasoning about
EWRAM. The decoder's stack use also counts toward the measured load peak.

### Descriptor validation

The loader validates a descriptor before it calls a decoder or exposes a view. It checks:

1. the layout ID resolves and its descriptor is present;
2. schema version, codec, and required flags are supported;
3. payload address and alignment are valid for the selected operation;
4. stored bytes and decoded file bytes are nonzero, representable, within the generated
   catalog bounds, and contained in the expected ROM section;
5. logical tile bytes equal checked `width * height * sizeof(u16)` and do not exceed
   decoded file bytes;
6. the padded current map dimensions fit `MAX_MAP_DATA_SIZE` before a full load;
7. a compressed payload has header byte `0x10` and its header output size equals decoded
   file bytes;
8. a bounded preflight walk consumes no bytes beyond stored bytes, produces exactly the
   declared output length, contains no invalid backreference, and accepts only the
   schema-defined zero padding through the four-byte boundary;
9. the stored checksum matches before compressed decoding; and
10. the complete decoded checksum matches before logical tiles are committed.

The preflight is required because the existing WRAM LZ77 routines do not take a stored
length or return a decode error. Header checks alone do not make malformed input safe.
Raw descriptors receive the same schema, range, logical-size, ROM-section, and checksum
checks, without LZ-specific validation.

Build-time round trips remain the primary defense against bad generated data. Runtime
checks cover version skew, damaged ROM data, bad integration, and test-injected failures.

### Normal map load

Normal loading preserves the current padded-grid algorithm:

1. Resolve and validate the map header and layout.
2. Fill all of `sBackupMapData` with `MAPGRID_UNDEFINED`.
3. Point `gBackupMapLayout.map` at `sBackupMapData` and calculate width plus 15 and
   height plus 14 with checked arithmetic.
4. Reject a padded grid over 10,240 entries.
5. Start the load context and validate the current layout.
6. Open the current layout, then row-copy its logical tile rectangle into the existing
   backup buffer at the current `(7, 7)` inset and padded stride.
7. Release the current view before opening any neighbor.
8. Apply connections through the same context as specified below.
9. Close the context and free its allocation before the loader returns to tileset work,
   scripts, camera display, or player control.

The loader never decompresses directly into a strided destination. It decodes the
complete source file into the bounded temporary buffer, verifies it, then copies the
logical tile prefix into `sBackupMapData`. Bytes after the logical prefix are verified
but are not copied, matching current runtime behavior.

Save reload follows the same raw or compressed load path before applying the saved map
view. The saved view remains an overlay on the verified backup map, not input to
decompression.

### Outdoor connections

Connected-map borders use bounded temporary neighbor decompression. This is the chosen
first implementation instead of generated edge data.

For each connection in existing order, the loader resolves the neighbor, opens its
layout with the shared context, copies only the legacy source rectangle, and releases the
view before the next neighbor:

| Direction from current map | Neighbor source data copied |
| --- | --- |
| South | First 7 rows after existing horizontal offset and clipping rules. |
| North | Last 7 rows after existing horizontal offset and clipping rules. |
| West | Last 7 columns after existing vertical offset and clipping rules. |
| East | First 8 columns after existing vertical offset and clipping rules. |

The east width remains 8 because the current loader uses `MAP_OFFSET + 1`; it must not be
normalized to the 7 used by other sides. Negative and positive offsets, partial overlap,
destination clipping, connection order, and missing connection behavior remain exactly
as in the raw loader.

One neighbor is decoded at a time. The loader never holds the current decoded map and a
neighbor simultaneously, never holds two neighbors, and never decompresses a region.
Raw neighbors are copied directly through the same rectangle operation.

Generated edge data is deferred because it duplicates authored tiles in ROM, creates a
second generated representation that can drift, and consumes the narrow difference
between gross payload saving and the 1 MiB net gate. The current data is small enough to
make one full neighbor decode bounded by 14,640 bytes, subject to the mandatory live-heap
proof. If that allocation cannot pass the peak-memory gate, rollout stops and the build
or connection representation must be redesigned. It does not add a permanent buffer or
silently omit connected edges.

### Special and direct consumers

The migration covers every known direct raw-layout consumer:

| Consumer | Required access | Initial policy |
| --- | --- | --- |
| `fieldmap.c` normal load | Full copy into the padded backup map. | Compressed layouts allowed after differential tests. |
| `fieldmap.c` connections | Checked neighbor rectangle copies. | Compressed layouts allowed after connection and peak-memory gates. |
| `battle_pyramid.c` | Scoped immutable view while each template square is assembled. | Template layouts may remain raw during the canary stage. |
| `trainer_hill.c` | Rectangle copy of the immutable entrance and exit rows before generated floor data. | Relevant layouts may remain raw during the canary stage. |
| `secret_base.c` | Immutable metatile search. | Relevant layouts remain raw until immutable-view tests pass. |
| `decoration.c` | Immutable source lookup when removing a decoration. | Relevant layouts remain raw until restoration tests pass. |

Secret Base searches and decoration restoration cannot substitute the mutable
`gBackupMapLayout` for authored source data. Decorations and other overlays may already
have changed the backup map. Those paths use the scoped immutable API even while their
descriptors select raw storage.

Battle Pyramid and Trainer Hill procedural output continues to target
`sBackupMapData`. The abstraction changes how template tiles are read, not how a floor
is generated or saved. Each special path must propagate an access failure instead of
continuing with a partly generated floor.

Future code that needs layout data chooses a full copy, rectangle copy, search helper, or
scoped immutable view. A code review must reject a new public raw-pointer accessor, a
codec switch outside the module, or a view stored beyond its scope.

### Failure behavior

Layout access returns this stable schema-1 error enum. Lower-level copy and view
operations return the code without changing the main callback, which lets their caller
finish local cleanup and lets unit tests inspect the result.

| Value | Name | Meaning |
| ---: | --- | --- |
| `0x00` | `MAP_LAYOUT_LOAD_OK` | No error. |
| `0x01` | `MAP_LAYOUT_LOAD_BAD_ID` | Map or layout ID is unresolved, excluded, or has an invalid connection target. |
| `0x02` | `MAP_LAYOUT_LOAD_NO_DESCRIPTOR` | The selected layout has no descriptor or payload. |
| `0x03` | `MAP_LAYOUT_LOAD_BAD_SCHEMA` | Schema version is zero or unsupported. |
| `0x04` | `MAP_LAYOUT_LOAD_BAD_CODEC_FLAGS` | Codec is unsupported or schema-1 flags are nonzero. |
| `0x05` | `MAP_LAYOUT_LOAD_BAD_ROM_RANGE` | Payload alignment, address, length, or linker-section range is invalid. |
| `0x06` | `MAP_LAYOUT_LOAD_BAD_SIZE` | Width, height, logical, stored, decoded, or arithmetic relationship is invalid. |
| `0x07` | `MAP_LAYOUT_LOAD_BAD_BOUNDS` | Padded grid, rectangle, stride, or destination capacity is invalid. |
| `0x08` | `MAP_LAYOUT_LOAD_BAD_STREAM` | LZ header, token, backreference, consumed length, padding, or output length is invalid. |
| `0x09` | `MAP_LAYOUT_LOAD_BAD_STORED_CRC` | Stored payload checksum does not match. |
| `0x0A` | `MAP_LAYOUT_LOAD_BAD_DECODED_CRC` | Complete decoded-file checksum does not match. |
| `0x0B` | `MAP_LAYOUT_LOAD_SCRATCH_LIMIT` | Requested decode bytes exceed the generated or active context capacity. |
| `0x0C` | `MAP_LAYOUT_LOAD_ALLOC_FAILED` | The one bounded temporary allocation failed. |
| `0x0D` | `MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME` | A view was nested, reused, or released outside its ownership contract. |
| `0x0E` | `MAP_LAYOUT_LOAD_INTERNAL` | An invariant not represented above failed. |

The top-level normal, saved-game, Battle Pyramid, and Trainer Hill initializers change
from `void` to this status result and propagate it to their overworld caller. The caller
invokes `AbortMapLayoutLoad(error, mapGroup, mapNum, layoutId)` exactly once and returns
without running the rest of the load sequence. That function stores the four small
diagnostic values in `gMapLayoutLoadError`, disables map scripts and save writes for the
failed transition, clears field input and asynchronous load callbacks, and installs
`CB2_MapLayoutLoadError` as `gMain.callback2`.

`CB2_MapLayoutLoadError` is a terminal, heap-independent screen that displays a fixed
"Map data error" message and the hexadecimal error, map, and layout values from
`gMapLayoutLoadError`. It does not read a layout, tileset, script, or save and does not
attempt to continue or redirect to the title screen. Only the existing hardware or soft
reset path can leave it. Test builds may expose the record directly; production never
writes it into saved data.

On any failure:

- do not call the decoder when preflight, integrity, capacity, or allocation checks fail;
- stop copying immediately and refill the backup map with `MAPGRID_UNDEFINED`;
- release the active view and load context;
- do not run map scripts, apply the saved view, expose player control, or write a save
  based on the failed map;
- install `CB2_MapLayoutLoadError` with the stable enum and identities above, without
  relying on a release-disabled assertion; and
- include layout and map identity plus the failure reason in test and debug output.

The compressed production ROM does not retain a second raw copy as an implicit fallback.
Falling back to unrelated blocks or a partial map would violate content preservation.
The raw-only build setting is the release rollback, not an in-session recovery from
damaged ROM data.

### Instrumentation

Test and profiling builds record at least:

- map and layout identity, storage kind, stored bytes, decoded file bytes, and logical
  tile bytes;
- every current and neighbor decode count and duration;
- temporary bytes requested, allocation result, smallest contiguous free block before
  allocation, heap high-water mark during load, and state after release;
- maximum observed decoder stack use or a proven conservative stack bound;
- full map-load duration through connection completion; and
- stable failure reason counts.

Instrumentation does not add saved fields. Production may compile out detailed counters
after rollout, but release builds keep validation and the deliberate failure path needed
for safe decode.

## Compatibility

`mapLayoutId`, map IDs, dimensions, `gBackupMapLayout` contents, and the saved map-view
overlay retain their existing meanings. Compression adds no save field and changes no
save-structure size. A compatible save created by a raw build must load in a hybrid
build, and the resulting save must load again in the raw rollback build.

The E2E ABI changes only if test instrumentation is explicitly exposed through it. If
that happens, the existing ABI version, C layout assertions, and TypeScript protocol
tests must change together. Compression metadata itself is not part of the save or E2E
ABI.

## Validation

The runtime boundary is complete when:

1. every known direct `MapLayout.map` access has been removed and a source guard prevents
   recurrence;
2. raw and compressed descriptors pass the same full-copy, rectangle-copy, immutable-
   view, lifetime, and error-contract tests;
3. full normal loads and every connection direction, offset, clipping case, and ordering
   case match the legacy raw loader byte for byte;
4. Battle Pyramid, Trainer Hill, Secret Base, and decoration paths use the abstraction
   and pass their feature-specific differential and gameplay tests;
5. all malformed metadata, payload, bounds, and allocation fixtures return the specified
   enum and enter `CB2_MapLayoutLoadError` with an undefined backup map, unchanged save,
   and no leak or partial gameplay state;
6. the largest and connection-heaviest loads use no more than one temporary full-layout
   allocation and release it before tileset heap work;
7. production static EWRAM contains no new full-map buffer, and measured heap and stack
   peaks pass the rollout thresholds; and
8. compatible saves move between raw and hybrid builds without an identifier, map-view,
   or persistent-state difference.

## References

- [Compressed map layout build and storage](compressed-map-layout-build-storage.md)
- [Compressed map layout validation and rollout](compressed-map-layout-validation-rollout.md)
- [`MapLayout` and `BackupMapLayout`](../../game/include/global.fieldmap.h)
- [Map offsets and backup-map limits](../../game/include/fieldmap.h)
- [Field map loading and connection copying](../../game/src/fieldmap.c)
- [Battle Pyramid floor generation](../../game/src/battle_pyramid.c)
- [Trainer Hill floor generation](../../game/src/trainer_hill.c)
- [Secret Base immutable layout search](../../game/src/secret_base.c)
- [Decoration restoration](../../game/src/decoration.c)
- [WRAM decompression interfaces](../../game/include/decompress.h)
- [Heap implementation](../../game/src/malloc.c)
