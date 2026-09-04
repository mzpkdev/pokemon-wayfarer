# Compressed map layout build and storage

PRD: [Content-preserving map layout compression](../prds/compressed-map-layouts.md)
Implemented: No

## Scope

This specification defines how the build selects, compresses, verifies, describes, and
links map layout block data. It owns source-file compatibility, catalog stability,
per-layout storage policy, generated descriptors, format versioning, integrity metadata,
build reports, and the gross and net size calculations.

The [runtime loading specification](compressed-map-layout-runtime-loading.md) owns
decode lifetime, memory, connection copying, and consumer APIs. The
[validation and rollout specification](compressed-map-layout-validation-rollout.md) owns
differential tests, playable journeys, performance signoff, staged enablement, and
rollback approval.

## Behavior

### Source of truth

`game/data/layouts/layouts.json` and each referenced `border.bin` and `map.bin` remain
the authored inputs. The build must not rewrite, replace, or require a compressed
version of a source file. Porymap continues to read and write the same files.

Compression is a generated build product. Generated outputs belong in the existing
build or generated-data flow and must be reproducible from the source catalog, storage
policy, and project compressor. Deleting generated compressed output and rebuilding it
must produce identical bytes and metadata with the same toolchain.

The first release applies to the Wayfarer catalog. Standalone product builds remain raw
unless a later change opts them into the same descriptor contract and validation gates.

### Catalog selection and identity

The layout generator preserves the current catalog contract:

- `gMapLayouts` follows `layouts.json` order;
- layout IDs remain one-based and `GetMapLayout` continues to resolve ID minus one;
- entries excluded from the selected product retain their `NULL` table slots;
- map headers retain their existing layout IDs;
- adding compression does not reorder, deduplicate, renumber, or merge layouts; and
- each selected layout resolves to exactly one independently addressable payload
  descriptor.

Blob organization may group descriptor tables or ROM sections by region for build and
reporting purposes. It must not combine several maps into a compression stream. A
single layout can be decoded without reading or decoding another layout or a complete
region.

### Storage policy

A checked-in storage-policy manifest, separate from Porymap-authored `layouts.json`,
selects the rollout mode and explicit exceptions. It supports at least these policies:

| Policy | Generated behavior |
| --- | --- |
| `raw` | Link the complete source `map.bin` unchanged. |
| `gba_lz77` | Link an independently compressed form of the complete source file. |
| `auto` | Use GBA LZ77 only when the aligned compressed payload is smaller than the aligned raw payload. Otherwise use raw. |

The global build setting selects raw-only or hybrid generation. Raw-only emits the same
descriptor shape but chooses raw storage for every entry. Hybrid applies the manifest
and stage allowlist. It does not alter source files or catalog identifiers.

A separate `legacy_size` measurement configuration compiles the pre-feature raw layout
record and loader without descriptors, checksum tables, codec dispatch, policy data,
new failure-path code, or other compression infrastructure. It uses the exact selected
map sources and production flags of the candidate. It exists only to measure net ROM
change and may also supply the test oracle; it is not the supported rollback image.

Every explicit raw exception records a short reason. Initial exceptions may include
Battle Pyramid templates, Trainer Hill layouts, Secret Base or player-room layouts used
for immutable restoration, other not-yet-migrated direct-access cases, and files that do
not shrink after alignment. An exception does not expose a pointer to callers and does
not bypass the common runtime API.

The manifest must reject an unknown layout name, duplicate rule, contradictory rule, or
exception that no longer exists. A default policy covers new selected layouts so a
catalog addition cannot silently omit its descriptor or verification. During staged
rollout, the default is raw. Before default-on release, the reviewed default becomes
`auto` and special raw exceptions remain explicit.

### Descriptor contract

The generated `MapLayout` record no longer contains a public typed `const u16 *map`
payload. That field becomes an opaque map-data descriptor reference. Only the layout
storage and loader module may inspect the descriptor's payload pointer or codec.

Schema version 1 uses this exact 28-byte GBA little-endian record:

| Offset | Width | Field | Meaning |
| ---: | ---: | --- | --- |
| `0x00` | 4 | `payload` | Four-byte-aligned ROM pointer to this layout's independent stored bytes. |
| `0x04` | 4 | `storedBytes` | Exact encoder output length, including codec-required padding but excluding inter-record alignment. |
| `0x08` | 4 | `decodedFileBytes` | Exact byte length of the original `map.bin`. |
| `0x0C` | 4 | `logicalTileBytes` | `width * height * sizeof(u16)`, the prefix consumed by the field engine. |
| `0x10` | 4 | `storedCrc32` | CRC-32 of exactly `storedBytes`, checked before compressed decoding. |
| `0x14` | 4 | `decodedCrc32` | CRC-32 of the complete original file, checked after decoding and during build verification. |
| `0x18` | 1 | `schemaVersion` | `1` for this record layout. `0` and all unknown values are invalid. |
| `0x19` | 1 | `codec` | `0` for raw and `1` for the project's GBA LZ77 encoding. All other values are invalid in schema 1. |
| `0x1A` | 2 | `flags` | Must be zero in schema 1. Any nonzero bit is invalid. |

The GBA target's pointer and integer fields are 32-bit little-endian values. The build
adds compile-time assertions for record size, alignment, and every offset above. The
generator emits descriptors on a four-byte boundary and `MapLayout` stores a pointer to
the descriptor at the former payload-pointer offset. Schema and codec values are
independent. A changed record layout uses a new schema value; a changed compression
bitstream uses a new codec value.

GBA LZ77 payloads use the project's existing format: header byte `0x10`, a 24-bit
decoded length, and the compressor's four-byte output alignment. Every payload address
is aligned to at least four bytes before it is passed to a WRAM decoder.

Both checksum fields use CRC-32/ISO-HDLC over bytes in increasing address order:

```text
width       = 32
polynomial  = 0x04C11DB7
init        = 0xFFFFFFFF
refin       = true
refout      = true
xorout      = 0xFFFFFFFF
check       = CRC32("123456789") == 0xCBF43926
```

A reflected implementation may use polynomial `0xEDB88320`. The generator, independent
verifier, and runtime share known-answer fixtures but do not share the same checksum
implementation in the round-trip test.

The linker brackets map-layout payloads with
`__map_layout_payloads_start` and `__map_layout_payloads_end` inside the existing
`maps_layouts` ROM category. Runtime range validation treats them as a half-open byte
range. It rejects arithmetic overflow and requires:

```text
payload >= __map_layout_payloads_start
payload < __map_layout_payloads_end
storedBytes <= __map_layout_payloads_end - payload
```

Descriptors may live in the same linker category but outside the bracketed payload
subrange. Raw and compressed modes emit the same descriptor ABI.

`Decoded file bytes` and `Logical tile bytes` must not be conflated. The current catalog
contains source files with bytes after the `width * height * sizeof(u16)` tile prefix.
Those bytes are part of the source file and therefore part of compression and the
round-trip comparison even though the current field engine does not read them. The
descriptor requires:

```text
decoded file bytes >= logical tile bytes
logical tile bytes is even
```

The build reports any trailing byte count by layout. A new or changed trailing region
requires review rather than being silently stripped. Runtime tile copies use only the
logical prefix; integrity checks cover the complete decoded file.

The checksum algorithm, initial value, reflection rules, and byte order are fixed as
part of schema version 1 and covered by known-answer tests. CRC-32 is the default for
the generated format. Substituting an existing project checksum is allowed only if it
covers both stored and decoded lengths, has known-answer tests, and changes the schema
version before any release payload is produced.

### Generated layout output

For each selected layout, generation performs this sequence:

1. Resolve the same version-selection rules used by map and layout generation.
2. Read the complete source `map.bin` and its declared width and height.
3. Compute logical tile bytes with checked arithmetic and validate that the source is at
   least that long.
4. Apply the stage policy. If compression is considered, run the project GBA LZ77
   compressor separately for this file.
5. Decode the candidate with an independent verifier and compare the entire result with
   the source file byte for byte.
6. Select raw or compressed storage, taking payload alignment into account.
7. Emit the aligned payload, descriptor, `MapLayout` reference, and one manifest row.
8. Verify that every selected catalog entry has one emitted descriptor and no emitted
   descriptor lacks a selected entry.

The round-trip comparison is a hard dependency of the object that links map layouts. A
normal Wayfarer build cannot bypass it. The generation rule depends explicitly on all
selected `map.bin` files, `layouts.json`, the storage policy, compressor, verifier, and
generator sources so that changing any input invalidates the output.

Schema version 1 permits only the GBA LZ77 encoder's zero padding through its four-byte
boundary. The bounded stream parser must consume the declared output and accept only
that defined padding within stored bytes. Other trailing encoded data is invalid.

The build fails on any of the following:

- missing or duplicate catalog entries, payloads, descriptors, or policy rows;
- a source shorter than its logical tile bytes;
- odd logical tile bytes, integer overflow, or dimensions that cannot fit the runtime
  backup-map bounds;
- an unknown schema, codec, flag, or policy;
- a raw, stored, decoded, or LZ header length that exceeds its defined field;
- an unaligned compressed payload;
- an LZ header decoded length that differs from decoded file bytes;
- an encode/decode error, truncated output, trailing decoded output, checksum mismatch,
  or byte-comparison mismatch; or
- nondeterministic output from identical inputs.

### Generated constants and ownership

Generation publishes the maximum decoded file size and maximum logical tile size for
the selected catalog. Runtime allocation uses a requested descriptor size and checks it
against the generated maximum. The current 14,640-byte largest file is evidence, not a
permanent hard-coded limit. A larger future file changes the generated bound and must
pass backup-grid, memory-peak, and latency gates before it can ship compressed.

Payload labels are private to the generated storage module. Other generated assembly or
C headers expose `MapLayout` records and descriptor references, not raw block-data
symbols. `gMapLayouts` remains the public lookup table because existing ID semantics
depend on it.

### Build report

Every hybrid and raw-only Wayfarer build emits a machine-readable manifest and a concise
human report containing:

- build identity, source revision, product selection, schema version, codec version,
  compressor identity, and storage-policy identity;
- selected layout count and all excluded or missing catalog entries;
- per layout: ID, name, source path, source version, dimensions, storage kind, raw file
  bytes, logical tile bytes, stored bytes, alignment cost, trailing bytes, both checksums,
  and raw-exception reason;
- totals for raw payloads, compressed payloads, descriptors, checksums, padding, and raw
  exceptions;
- the largest decoded file and the generated decode bound;
- the count and size of files for which compression was not profitable; and
- exact round-trip status for every compressed entry.

The reference 991-layout audit measured 1,742,526 raw bytes, 488,000 compressed payload
bytes, and 1,254,526 bytes of gross payload saving. The report labels these as baseline
evidence only. It regenerates its own figures from the exact selected catalog instead of
asserting that the count or total stays constant.

### Reference audit reproduction

The documentation branch independently selects 987 layouts and produces 1,741,356 raw
bytes and 487,320 compressed bytes. The supplied 991-layout linked audit used an
augmented catalog with these four additional HNS records. Each record declares
`game_version: "hns"` and `layout_version: "frlg"`, so it reuses an existing FRLG
source layout while the Wayfarer selection rule includes the new record.

| Added layout record | Source path under `game/` | Raw bytes | GBA LZ77 bytes |
| --- | --- | ---: | ---: |
| `House1_FRLG_hns_Layout` | `data/layouts/House1_Frlg/map.bin` | 198 | 148 |
| `CeladonCity_Hotel_FRLG_hns_Layout` | `data/layouts/CeladonCity_Hotel_Frlg/map.bin` | 374 | 200 |
| `Entrance_1F_FRLG_hns_Layout` | `data/layouts/Entrance_1F_Frlg/map.bin` | 312 | 192 |
| `Entrance_2F_FRLG_hns_Layout` | `data/layouts/Entrance_2F_Frlg/map.bin` | 286 | 140 |
| Four-entry total | | 1,170 | 680 |

On the documentation branch, the existing records for these source paths are FRLG-only
and are correctly absent from the 987-entry Wayfarer selection. The 991 audit first adds
the four HNS records above, then selects default Emerald and explicit HNS layouts with
the same version rule as `mapjson`. It runs the repository-built
`tools/gbagfx/gbagfx` once per source file with a `.lz` output, runs the same tool from
each `.lz` file to a temporary `.bin`, uses `cmp` over the complete source and decoded
files, and sums the source and `.lz` byte sizes. All 991 comparisons matched. No source
file was changed. This method yields:

```text
987 branch entries + 4 audited entries = 991 entries
1,741,356 + 1,170 = 1,742,526 raw bytes
487,320 + 680 = 488,000 compressed bytes
1,742,526 - 488,000 = 1,254,526 gross saved bytes
```

The four-entry table is the retained baseline manifest for this proposal. Implementation
replaces it with the full generated machine-readable manifest. A manifest identity is
part of every reported total.

### Net ROM accounting

Gross payload saving is:

```text
selected raw payload bytes - selected stored payload bytes
```

It is useful for compressor diagnostics but is not a product result. Net ROM saving is
the difference in total linked `used_bytes` between the production-equivalent
`legacy_size` baseline and hybrid candidate. Both use the same content revision,
toolchain, non-layout features, and release flags. The legacy baseline contains none of
the new compression infrastructure, so the difference charges the candidate for loader
code, descriptors, checksum fields, alignment, any other format metadata linked into
ROM, and every raw exception. Build manifests and storage policy are retained artifacts,
not ROM costs, unless a later implementation explicitly links some part of them.

The raw-only descriptor build is the compatibility baseline and release rollback. It is
not the size baseline because its shared feature overhead would cancel and overstate net
saving.

The existing ROM report's `maps_layouts` category explains where space moved, but only
the whole-ROM `used_bytes` delta approves release. The hybrid build must save at least
1,048,576 bytes net. The paired reports and their configuration identities are retained
as rollout evidence.

## Compatibility

The storage schema is private to a single ROM image. Save data never stores a payload
pointer, codec, descriptor version, checksum, or compressed offset. The build preserves
layout IDs, map IDs, dimensions, and serialized structures, so switching between raw
and hybrid builds does not require a save migration.

The loader rejects descriptor or codec versions it does not implement. A future format
change increments the applicable version, adds verifier fixtures, and keeps raw-only
generation available through its staged rollout. Old and new descriptor interpretations
must not share a version number.

## Validation

The build/storage boundary is complete when:

1. clean raw-only and hybrid builds generate deterministic manifests from unchanged
   Porymap source files;
2. every selected compressed layout round-trips over the complete source file and
   compares byte for byte;
3. raw entries link their complete source bytes and carry correct descriptor metadata;
4. catalog order, `NULL` slots, map layout constants, and map header references match the
   raw control build;
5. schema, codec, alignment, length, checksum, policy, dependency, and malformed-input
   tests cover every build failure above;
6. the report separates logical tile bytes, decoded file bytes, stored payload bytes,
   gross saving, overhead, and linked net saving; and
7. paired production-equivalent legacy-size and hybrid ROM reports prove at least 1 MiB
   net saving before the default-on stage.

## References

- [Compressed map layout runtime loading](compressed-map-layout-runtime-loading.md)
- [Compressed map layout validation and rollout](compressed-map-layout-validation-rollout.md)
- [Current layout generator](../../game/tools/mapjson/mapjson.cpp)
- [Current layout generation rules](../../game/map_data_rules.mk)
- [Current generated layout include point](../../game/data/maps.s)
- [GBA LZ77 compressor](../../game/tools/gbagfx/lz.c)
- [ROM report](../../game/tools/rom_report/rom_report.py)
