# Sinnoh asset reuse and ROM footprint

PRD: [Sinnoh exploration foundation](../prds/sinnoh-exploration-port.md)
Implemented: No

## Scope

This specification defines how the Sinnoh exploration port selects, reuses,
aliases, or adds map and tileset assets. It owns the exact-match manifest,
component-level tileset decisions, layout blockdata reuse, compression
dependency, ROM accounting, rollback, and release gates.

The [map-port specification](sinnoh-exploration-map-port.md) owns catalog
selection, identifiers, provenance, topology, events, runtime region behavior,
and map validation. Later content documents own object graphics, music, wild
encounters, Trainers, scripts, text, and region-map art; none of those are
included in this estimate.

## Behavior

### Audit identities

The source audit uses donor commit
`4eed17cc63c4ec8c24fbb20fe49e8d65cb4870d8`. The planning comparison used
Wayfarer commit `085c3fdc613c0a9d701065fcc6243d81f4f3492c`.

Implementation must refresh the comparison against its actual base commit and
store both identities in `game/src/data/wayfarer_sinnoh_assets.json`. A planning
match does not authorize an alias after either input changes.

The checked-in manifest contains one row per independently stored payload with:

- stable record ID and asset family;
- donor and Wayfarer source paths;
- donor and canonical generated paths and symbols;
- source, generated, and decoded hashes;
- source and decoded byte lengths;
- array shape, alignment, compression state, layout version, linkage,
  visibility, and output section;
- every consuming Sinnoh layout;
- reuse class, canonical owner, expected linked delta, and rationale; and
- proof command/version and baseline commit.

The manifest is the only reuse allowlist. Equal assets discovered later remain
independent until their row is reviewed. A changed input fails generation and
does not silently fall back to whichever payload happens to be present.

### Reuse classes

Every row has exactly one class:

| Class | Meaning |
| --- | --- |
| `EXISTING_REFERENCE` | The Sinnoh layout references an unchanged Wayfarer tileset or component directly. |
| `EXACT_ALIAS` | A distinct Sinnoh identity shares one byte-identical generated payload with an existing canonical symbol. |
| `SINNOH_VARIANT` | The donor derives from an existing family but differs; preserve a separate Sinnoh symbol and payload. |
| `SINNOH_NEW` | No compatible Wayfarer payload exists; add the reviewed donor asset. |
| `REVIEW_REQUIRED` | Identity, provenance, shape, or runtime meaning is unresolved; generation and release fail. |

Exact equality includes the bytes the runtime reads, decoded length, shape,
alignment, format, and consumer expectations. PNG equality alone is not proof.
Visual similarity, matching names, a common Emerald ancestor, or unused bytes
inside a larger array do not qualify.

Keep authored source paths and public identities even when linked storage is
shared. Use complete array aliases or generator-owned canonical references;
never replace an array with a pointer object or rely on linker-wide identical
code/data folding.

### Selected tileset closure

The 133 layouts use only `General` or `Building` as their primary tileset. Their
secondary usage is frozen below:

| Secondary | Layouts | Secondary | Layouts |
| --- | ---: | --- | ---: |
| Petalburg | 9 | Rustboro | 3 |
| Jubilife | 7 | Mauville | 5 |
| Hearthome | 1 | Celestic | 7 |
| Veilstone | 1 | Lilycove | 3 |
| Canalave | 1 | Snowpoint | 5 |
| Sunnyshore | 3 | Ever Grande | 2 |
| Valor | 13 | Lavaridge | 1 |
| Cave | 7 | Pasos | 14 |
| Brendan/May's House | 6 | Generic Building | 23 |
| Pokemon Center | 8 | Shop | 8 |
| Pokemon School | 1 | Facility | 3 |
| Rustboro Gym | 1 | Pretty Petal Flower Shop | 1 |

Generate the dependency closure from these pairs. Do not enable or import an
unreferenced donor tileset merely because it exists in the source repository.

### Existing references

The following used secondary tilesets match current Wayfarer component-for-
component and are selected directly:

- Brendan/May's House;
- Generic Building;
- Pokemon Center;
- Pokemon School;
- Facility; and
- Pretty Petal Flower Shop.

Rustboro Gym is also reused when the refreshed generated-byte comparison
passes. Every existing reference keeps its current Wayfarer symbol and storage;
the Sinnoh port adds no duplicate descriptor or payload.

For the primary `Building` tileset, graphics, metatiles, and attributes match
Wayfarer and remain shared. Donor palette `04` differs, so any layout needing
that color result uses a reviewed Sinnoh palette/descriptor variant rather than
changing the shared Building palette table.

For `Cave`, graphics match and remain shared, while donor metatiles and
attributes are a Sinnoh variant. For `Shop`, graphics and attributes remain
shared while donor metatiles are a Sinnoh variant. A descriptor may combine
shared and variant components; do not duplicate a matching component merely
because another component differs.

### Sinnoh variants

Keep dedicated Sinnoh identities for every changed component in:

- primary `General`;
- secondary `Petalburg`;
- secondary `Rustboro`;
- secondary `Mauville`;
- secondary `Lilycove`;
- secondary `EverGrande`;
- secondary `Lavaridge`;
- the `Cave` metatiles and attributes;
- the `Shop` metatiles; and
- the changed primary `Building` palette data.

Never point an existing Hoenn, HNS, FRLG, Sevii, Secret Base, or shared
interior descriptor at one of these variants. A Sinnoh variant cannot change
the visible output, collision, animation, or behavior of a preexisting map.

The audit found five 32-byte donor palettes with exact payload matches already
present in Wayfarer: Petalburg `09` and `10`, Rustboro `06` and `12`, and
Lilycove `11`. They represent at most 160 bytes of additional reuse. Count that
saving only if the implementation gives each row a complete, type-safe symbol
or generator-owned canonical reference and the final linker proves one emitted
payload. Otherwise retain the conservative independent palette tables.

### New Sinnoh tilesets

Add these nine secondary families as `SINNOH_NEW` after technical source and
production-byte review:

- Jubilife;
- Hearthome;
- Celestic;
- Veilstone;
- Canalave;
- Snowpoint;
- Sunnyshore;
- Valor; and
- Pasos.

For each family, import only its referenced graphics, palettes, metatiles, and
attributes. The refreshed audit must compare each component against every
current Wayfarer asset, not merely the same pathname. The planning audit found
no byte-identical Wayfarer graphics, metatiles, or attribute payload for these
nine families.

Keep the checked-in source inputs and their donor identity/hashes in the
repository. Provenance is technical traceability, not an attribution or
permission release gate; missing technical source identity or byte proof
blocks import and is not a reason to relabel the asset as Emerald.

### Layout blockdata and borders

The donor's 133 `map.bin` files total 389,886 bytes. Compare decoded raw bytes,
not filenames or layout names.

The planning audit found:

| Classification | Layouts | Raw bytes |
| --- | ---: | ---: |
| Exact existing Wayfarer blockdata | 33 | 7,540 |
| Unique Sinnoh blockdata | 100 | 382,346 |
| **Total** | **133** | **389,886** |

For each exact row, keep an independent `MapLayout` descriptor but point its
blockdata member at one canonical emitted array. The generator must emit the
payload once. Sharing a source filepath while emitting two `incbin` arrays does
not count as reuse.

Ninety-five borders match an existing Wayfarer border. The remaining 38 are
eight bytes each, for 304 unique bytes. Canonicalize exact borders through the
same manifest mechanism. Width, height, tileset pair, map identity, collision
meaning, and layout descriptor remain independent even when blockdata or a
border is shared.

This specification owns only the frozen Sinnoh blockdata rows. It does not
broaden the existing exact-ROM-aliasing allowlist to arbitrary map data.

### Compression dependency

Raw blockdata is the rollback and comparison authority. If the approved hybrid
map-layout storage system is available, store the 100 unique Sinnoh payloads
independently with GBA LZ77 and validate each decoded result against its raw
input.

The planning trial measured:

| Storage | Bytes | KiB |
| --- | ---: | ---: |
| Unique raw blockdata | 382,346 | 373.4 |
| Independently compressed blockdata | 84,008 | 82.0 |
| **Gross blockdata reduction** | **298,338** | **291.3** |

This measurement does not authorize a Sinnoh-only decompressor. Compressed
storage ships only through the common APIs, descriptor format, integrity
checks, memory limits, error handling, rollout, and raw rollback defined by
the three compressed-layout specifications. Every selected layout stays
Porymap-compatible in source control.

### Planning budget

Wayfarer uses `fastSmol` for compressed tileset graphics, so donor `.lz` sizes
are not the production accounting basis. The Wayfarer-native planning audit is:

| Incremental asset category | Bytes | KiB |
| --- | ---: | ---: |
| Nine new tileset families | 141,588 | 138.3 |
| Modified components, with the five palette aliases | 111,230 | 108.6 |
| Unique raw blockdata | 382,346 | 373.4 |
| Unique borders | 304 | 0.3 |
| Layout descriptors and pointer table | 3,724 | 3.6 |
| **Raw planning total** | **639,192** | **624.2** |

Without the five palette aliases, add 160 bytes. With the measured blockdata
compression and no other change, the same planning subtotal becomes 340,854
bytes, or 332.9 KiB. Both figures exclude integration code and all later
gameplay or presentation content.

At the audited Wayfarer baseline, `__rom_end` is `0x09F57B70`, leaving 165,008
bytes before the enforced `0x09F80000` release limit. The raw port cannot fit.
The compressed planning subtotal also exceeds that headroom by 175,846 bytes
before integration code. Therefore implementation must first land enough
approved, content-preserving savings or a broader map-compression rollout. It
must not waive the reserve, use a 64 MiB ROM, or remove selected maps to make
the arithmetic pass.

Refresh all figures with serial, clean, production-equivalent builds on the
implementation base. Report source totals separately from linked ROM deltas.
Only `rom.used_bytes`, linked `__rom_end`, section/category changes, and the
configured limit establish acceptance.

### Build-time verification

Before compilation, a host verifier:

1. regenerates every graphics and palette input using the production tools;
2. decodes compressed inputs where applicable;
3. verifies byte length, array shape, alignment, format, and complete bytes;
4. validates every manifest consumer and rejects an unused imported payload;
5. proves each exact alias has one canonical owner and no alias cycle;
6. proves every variant has a separate symbol and cannot reach a preexisting
   map descriptor;
7. decompresses every selected layout and compares it with raw `map.bin`;
8. rejects a stale baseline identity, missing technical source proof, or
   unresolved row; and
9. writes a deterministic report containing raw, generated, decoded, gross,
   and expected linked bytes without modifying the manifest.

Negative fixtures cover one-byte drift, wrong decoded length, wrong tileset or
layout format, type/shape mismatch, stale generated output, duplicate owner,
alias cycle, missing source, undeclared consumer, incorrect compression flag,
and an attempted alias based only on a matching filename.

### Runtime and visual verification

For every tileset descriptor pair used by Sinnoh, render representative and
edge metatiles in Cartographer and enter at least one consuming map in an
accurate emulator. Compare raw and optimized builds for palettes, animation,
collision, elevation, door behavior, weather, and the complete visible tile
range.

Capture Hoenn maps using every modified donor family name before and after the
port. Their output must remain unchanged. This includes at least one existing
consumer of General, Building, Petalburg, Rustboro, Mauville, Lilycove, Ever
Grande, Lavaridge, Cave, and Shop.

Compressed layout validation covers the largest imported layout, Eterna
Forest at 13,776 raw bytes, plus connection-heavy exteriors, caves, gates, and
shared-blockdata interiors. Measure peak EWRAM, heap fragmentation, stack,
transition frames, and failure behavior under the common compressed-layout
contract.

### Rollback

The narrow rollback disables Sinnoh content selection while retaining the
reviewed source files and manifests. The broader storage rollback keeps Sinnoh
selected but rebuilds its unique layouts from raw blockdata and restores
independent payloads for any new exact alias.

Neither rollback changes existing IDs, save layout, standalone build output,
or authored asset files. A rollback never substitutes a visually similar asset
or drops a map.

### Acceptance gates

1. The manifest identifies the exact donor and Wayfarer baselines and has no
   unresolved row.
2. The selected dependency closure contains every required component and no
   unreachable imported payload.
3. Every exact reference or alias passes generated and decoded byte, length,
   shape, format, and consumer checks.
4. Every changed Emerald-derived component uses a Sinnoh variant; every
   preexisting map renders and behaves identically.
5. All nine new tileset families have reviewed technical source identity and
   production-byte evidence.
6. Exactly 33 blockdata rows and 95 borders reuse canonical Wayfarer payloads
   unless a refreshed audit deliberately updates the manifest and counts.
7. Every unique raw or compressed layout round-trips byte-for-byte; its source
   remains Porymap-compatible.
8. Raw and optimized emulator comparisons cover every selected tileset family,
   the largest layout, every map class, and representative shared payloads.
9. Clean paired production links report each category delta and distinguish
   gross asset savings from net ROM savings.
10. The selected release ends at or below `0x09F80000`, preserving at least
    512 KiB below 32 MiB after code, descriptors, manifests, and validation
    infrastructure.
11. Standalone HNS, Emerald, FireRed, and LeafGreen builds retain their prior
    catalogs, descriptors, symbols, and visible output.

## References

- [Sinnoh exploration map port](sinnoh-exploration-map-port.md)
- [Exact ROM graphics and tileset aliases](exact-rom-asset-aliasing-graphics.md)
- [Exact ROM alias validation](exact-rom-asset-aliasing-validation.md)
- [Compressed map layout build and storage](compressed-map-layout-build-storage.md)
- [Compressed map layout runtime loading](compressed-map-layout-runtime-loading.md)
- [Compressed map layout validation and rollout](compressed-map-layout-validation-rollout.md)
- [Tileset ROM storage optimization](tileset-storage-optimization.md)
- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Tileset graphics table](../../game/src/data/tilesets/graphics.h)
- [Tileset metatile table](../../game/src/data/tilesets/metatiles.h)
- [Tileset headers](../../game/src/data/tilesets/headers.h)
