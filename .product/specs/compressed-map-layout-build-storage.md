# Compressed map layout build and storage

PRD: [Content-preserving map layout compression](../prds/compressed-map-layouts.md)
Implemented: Yes

## Scope

This specification owns build-time layout selection, compression, round-trip
verification, generated descriptors, payload bounds, and the compact storage report.
Runtime behavior is defined in
[compressed-map-layout-runtime-loading.md](compressed-map-layout-runtime-loading.md).

## Inputs and modes

`game/data/layouts/layouts.json` and its referenced `map.bin` and `border.bin` files are
the authored source of truth. Generated storage never rewrites them.

The build exposes exactly two modes:

- `raw`: store every selected layout verbatim;
- `hybrid`: independently GBA-LZ77-compress every selected layout and use the compressed
  form only when its aligned stored size is smaller.

Wayfarer defaults to `hybrid`. Standalone products default to `raw`.

There is no policy file, canary stage, minimum-savings threshold, raw exception list,
or legacy mode. A future exception must be justified by a reproduced product defect.

## Generated format

Every Wayfarer `MapLayout.mapData` points to this 16-byte, four-byte-aligned descriptor:

| Offset | Type | Meaning |
| ---: | --- | --- |
| `0x00` | pointer | Stored payload in the generated payload section. |
| `0x04` | `u32` | Stored bytes, including alignment padding. |
| `0x08` | `u32` | Complete decoded source-file bytes. |
| `0x0c` | `u8` | Codec: `0` raw, `1` GBA LZ77. |
| `0x0d` | 3 bytes | Zero padding. |

Raw and hybrid modes emit this same shape. The linker exposes the start and end of the
payload section so the loader can bound descriptor references.

The generated constants also record maximum stored and decoded sizes. They are compile-
time/runtime allocation bounds, not versioned format metadata.

## Build verification

For every selected layout, the generator must:

1. read the exact source bytes;
2. generate an independent GBA LZ77 candidate in hybrid mode;
3. decode that candidate with the bounded build-time decoder;
4. require byte-for-byte equality with the source; and
5. select the smaller aligned representation.

Any read, encode, decode, or equality failure stops the build.

## Report

The JSON report contains only information useful for size review: mode, raw/stored
sizes, selected codec, hybrid round-trip result, totals, and maximum bounds. Raw mode
does not claim a compression round trip that did not run. The report does not contain
CRCs, Git revisions, policy hashes, rollout stages, or a duplicate text report.

## Build integration

Storage mode participates in generated-output and object-directory identity so raw and
hybrid artifacts cannot be confused. Changing modes invalidates generated layout data.
Unrelated Git revisions do not.

The source audit rejects new production code that directly dereferences layout payloads
instead of using the map-layout access API.
