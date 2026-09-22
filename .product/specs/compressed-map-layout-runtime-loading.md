# Compressed map layout runtime loading

PRD: [Content-preserving map layout compression](../prds/compressed-map-layouts.md)
Implemented: Yes

## Scope

This specification owns the runtime descriptor boundary, decompression lifetime, map
and connection copying, special consumers, and failure behavior.

## Invariant

`gBackupMapLayout` and `sBackupMapData` remain the mutable field-gameplay map. ROM
payloads are immutable inputs only. Consumers do not inspect codecs, LZ headers, or
payload addresses.

Both raw and hybrid Wayfarer builds use the same 16-byte descriptor contract. Standalone
games keep their existing raw layout representation.

## Validation boundary

Before use, the Wayfarer loader checks:

- non-null, aligned descriptor and payload pointers;
- payload containment in the generated linker section;
- known codec and nonzero, bounded sizes;
- decoded size sufficient for the layout geometry; and
- for LZ77, a bounded token walk that cannot read past stored bytes or produce past the
  declared decoded size.

The preflight accepts alignment padding only after the decoded stream. It runs before
the BIOS-compatible decompressor. The loader does not calculate CRCs or retain a
verification cache because linked ROM is trusted and immutable.

## Memory lifetime

A map load first determines the largest decoded payload needed by the current layout or
any connected layout. It allocates one scratch buffer of that size, reuses it
sequentially, and releases it after the current map and connection strips have been
copied.

Raw layouts require no decode buffer. Short-lived view APIs used by special consumers
may allocate and release a buffer around one operation. No decompressed layout remains
resident during ordinary field play.

## Access API

Production code uses the map-layout module for:

- full copies into a caller-owned grid;
- rectangular copies with source and destination bounds;
- one-tile reads; and
- scoped read-only views for consumers that need several source reads.

The field loader, all four connection directions, Battle Pyramid, Trainer Hill, Secret
Bases, decorations, origin/persistence helpers, and any future direct consumer use this
boundary. The source audit prevents new direct payload dereferences.

## Failure behavior

Invalid geometry, descriptor ranges, sizes, streams, bounds, allocation failures, and
invalid view lifetime return a small `MapLayoutLoadError`. A field-load failure clears
the partial backup grid, stops scripts and player input, and enters a terminal blank
screen rather than exposing a partially decoded map.

Failures are fail-stop. There is no fallback from compressed bytes to unrelated raw
bytes in the same image, and no elaborate diagnostic UI.

## Save compatibility

Storage mode is not saved. Layout IDs, map IDs, coordinates, tiles, and persistent world
state are unchanged, so a save created by hybrid must continue under raw and vice versa.
