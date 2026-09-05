# Tileset ROM storage optimization

## Intent

Reduce the Wayfarer release ROM footprint without removing or changing game
content. The first version compresses two full-size tileset graphics payloads
that are still stored raw and makes the existing production-link exclusion of
eight unreachable definitions an enforced invariant.

This is a storage change only. Players must see the same maps and graphics and
must retain the same Secret Base, Cable Club, link, script, and save behavior.

## Design

### Content-preservation policy

The decoded bytes loaded into VRAM are the product contract. For each selected
active asset, the build must decode the generated payload and compare all
16,384 bytes with the active raw input. A mismatch, truncated decode, oversized
decode, legacy-asset substitution, or missing comparison fails the build.

The optimization must preserve every accessible map, Secret Base style, Cable
Club behavior, tile, palette, metatile, metatile attribute, animation, script,
and save behavior. It must not change map IDs, save data, or player-visible
timing beyond the technical specification's one-frame transition limit.

Source PNGs and other source assets remain in the repository. An asset may be
absent from the production ROM only when the build proves that no live code or
data references its definition.

### Active payloads

Version 1 uses GBA LZ77 for both selected assets. It does not choose a codec
dynamically or add a generic best-codec framework.

| Active symbol | Raw bytes | v1 codec | Measured stored bytes | Gross saving |
| --- | ---: | --- | ---: | ---: |
| `gTilesetTiles_SecretBase` | 16,384 | GBA LZ77 | 9,140 | 7,244 |
| `gTilesetTiles_CableClub` | 16,384 | GBA LZ77 | 4,028 | 12,356 |
| Total | 32,768 | GBA LZ77 | 13,168 | 19,600 |

GBA LZ77 is the smaller measured codec for the active Secret Base primary
sheet: its fastSmol result is 10,980 bytes. fastSmol produces the smallest
Cable Club payload at 3,872 bytes, 156 bytes less than GBA LZ77. Version 1
still selects GBA LZ77 to keep this two-asset change on one decoder path and
one host round-trip procedure. The 156-byte difference does not justify adding
a second codec to the version 1 validation and rollback matrix.

The retained Wayfarer headers with raw graphics are the Secret Base primary
sheet, its six style sheets, and Emerald Cable Club. Version 1 addresses only
the two full-size sheets and does not propose a catalog-wide conversion.

### Short Secret Base sheets

The six active Secret Base secondary sheets remain raw:

- Brown Cave
- Tree
- Shrub
- Blue Cave
- Yellow Cave
- Red Cave

Each active sheet is 2,656 bytes, for 15,936 bytes total. Emerald layouts copy
a 16 KiB secondary tileset slot. Compressing the short bytes as-is would make
the existing decompression helpers allocate only 2,656 decoded bytes before a
16,384-byte VRAM copy, which risks a heap overread.

Padding each sheet to a 16,384-byte decoded length avoids that overread, but
GBA LZ77 produces 17,340 stored bytes in total, a 1,404-byte regression from
the current raw payloads. A separate audit found that padded fastSmol output
would total 8,856 bytes. That option is still outside version 1 because it
would add six assets and a new source-padding contract for another 7,080 gross
bytes. It requires its own proposal and full safety evidence. It must not be
silently added to this work.

The similarly named legacy
`gTilesetTiles_SecretBase*Compressed` definitions are not replacements. Their
generated inputs contain 82 tiles, or 2,624 bytes, from `unused_tiles`, while
the active inputs contain 83 tiles, or 2,656 bytes. The source PNG pairs happen
to match on this task base, but their generated payloads do not: the legacy
outputs omit the final active tile.

### Unreachable definitions

The audited no-reference inventory is:

| Definitions | Count | Input bytes |
| --- | ---: | ---: |
| Legacy `gTilesetTiles_SecretBase*Compressed` arrays | 6 | 8,820 |
| `gTilesetTiles_UnknownSecretBase` | 1 | 16,384 |
| `gTilesetTiles_UnknownCableClub` | 1 | 3,840 |
| Total | 8 | 29,044 |

These definitions remain available to the build from preserved repository
sources, but none may occupy a production Wayfarer ELF output section or ROM
range unless a later content decision deliberately restores one.

The task-base production release already satisfies this rule. Its linker map
lists all eight definitions only under discarded input sections at address
zero, and none appears in the final ELF symbol table. Version 1 therefore adds
a regression guard; it does not count these bytes as a new production ROM
saving and does not delete their source files.

### Corrected opportunity

The earlier 48,800-byte figure added the best measured active compression
result, 19,756 bytes, to the 29,044-byte unreachable inventory. It is a valid
gross inventory figure only for a link that retains those unreachable input
sections and uses the mixed-codec choice for Cable Club. It is not the version
1 opportunity on the current production-equivalent release, which already
discards the unreachable definitions. The selected GBA LZ77 policy has a
current expected gross opportunity of 19,600 bytes before final-link effects.

An even earlier 27,936-byte estimate is withdrawn. It counted short Secret
Base compression without first proving decoded length and active-input
identity. That was unsafe because the loader could copy 16,384 bytes from a
2,656-byte allocation and because the legacy `unused_tiles` payloads omit an
active tile. Neither issue can be treated as an acceptable storage tradeoff.

The task-base release at commit `443345f62949e37375325d02d9a75bd18e1c68fc`
reports `__rom_end` at `0x09F5B110` and `rom.used_bytes` of 32,878,864. It
leaves 675,568 bytes unused, which is 151,280 bytes more than the current 512
KiB reserve requirement. The previously audited 55,236-byte reserve deficit
is therefore historical rather than current on this base. Even in that older
state, 48,800 gross bytes would have fallen at least 6,436 bytes short before
alignment or build overhead, so this feature must never claim that it restores
the full historical deficit by itself.

## Presentation

There is no new menu, message, setting, animation, or player choice. Before and
after screenshots and VRAM tile-range captures must be identical for the
affected maps. Save files do not change format or meaning.

## Interactions

Normal map loading remains the only runtime consumer of the two selected
payloads. The existing tileset loader reads each selected header's
`isCompressed` field and dispatches through its existing decompression paths.
The change adds no permanent RAM allocation and no new runtime ownership
model.

Secret Base style selection, entrances, decorations, record mixing, and
save/reload behavior continue to use their current maps and data. Cable Club
and link-room flows continue to use their current scripts and link behavior.

## Boundaries

- Compress only `gTilesetTiles_SecretBase` and
  `gTilesetTiles_CableClub` in version 1.
- Keep all six active short Secret Base secondary sheets raw.
- Preserve every source asset, including `unused_tiles` and unknown tiles.
- Do not substitute a legacy `gTilesetTiles_SecretBase*Compressed` payload for
  an active asset.
- Do not remove accessible or potentially accessible content.
- Do not modify map layouts, palettes, metatiles, metatile attributes, map
  compression, Surf graphics, Trainer graphics, text, audio, animations,
  scripts, or save data.
- Do not redesign the generic tileset loader unless new verification disproves
  the audited runtime assumptions. Such a redesign requires separate approval.
- Do not add a generic codec-selection framework.
- Do not claim the discarded 29,044-byte inventory as a new production ROM
  saving.

## Constraints

The optimized release must use the existing compression formats and loader
ownership rules. Both selected payloads must declare and decode to exactly
16,384 bytes because their Emerald layout paths request full 16 KiB tile
ranges.

The optimized build must retain a narrow raw build mode that restores the two
raw inputs and sets both headers to `isCompressed = FALSE`. It is the rollback
path and the paired-link baseline. It must not change other production build
flags or content.

Outer allocation failure must queue no copy and must not produce an
out-of-bounds read, use-after-free, double free, or DMA from released memory.
The explicit heap route also needs one task slot per compressed copy so it can
free the decoded buffer after DMA. The existing task API has no recoverable
release failure when all slots are occupied. Affected routes must prove at
least two free task slots before the paired primary and secondary copy, and
task exhaustion blocks the optimized release.

Malformed compressed input is different: the release decompressor returns no
status and its diagnostic is disabled, so version 1 cannot promise safe
runtime recovery without a loader redesign. Generation-time size and
round-trip checks must prevent malformed selected payloads from shipping, and
test or debug builds must exercise the existing diagnostic. Version 1 adds no
new recovery screen or generic fallback. If those controls fail, the optimized
mode does not ship and the raw mode remains active.

## Rollout and rollback

Rollout starts with the raw mode as a production-equivalent baseline. The
optimized build then enables only the two fixed payload mappings and the two
header flags. Static, link, runtime, emulator, and hardware gates must pass
before the optimized mode becomes the production default.

Rollback sets the narrow build mode to raw, which selects the original
`.4bpp` inputs and clears the two compression flags. A rollback rebuild must
not depend on generated compressed files and must preserve saves. Any content
mismatch, unsafe memory result, unexplained ROM delta, transition regression,
or Cable Club failure triggers rollback.

## Playtesting

Playtesting covers all 24 Secret Base layouts: variants 1 through 4 of all six
styles, plus every cave, tree, and shrub entrance type. It includes entering
and leaving bases, placing and removing decorations, saving inside or after
using a base, reloading, and repeating the relevant transitions.

Cable Club coverage includes all seven layouts currently selecting
`gTileset_CableClub`, entry and exit, link-room transitions, and the supported
trade, battle, and record-mixing flows. At least one accurate emulator run and
one GBA hardware or hardware-accurate validation run must confirm visuals,
transition timing, and DMA completion behavior.

## Acceptance

The optimized mode is accepted only when all of these gates pass:

1. Each selected raw input and decoded generated output is exactly 16,384
   bytes, and a byte-for-byte round trip matches.
2. `gTileset_SecretBase` and `gTileset_CableClub` have
   `isCompressed = TRUE` and point to compressed outputs generated from their
   intended active `tiles.4bpp` inputs.
3. All six short active Secret Base sheets remain raw, and no legacy
   `unused_tiles` payload is substituted.
4. An identifier-aware source audit finds no use of the eight unreachable
   symbols outside their definitions. The production linker map assigns them
   no output address, and the final ELF contains none of the symbols. Their
   source files remain present.
5. Serial raw and optimized production-equivalent release builds compare
   `__rom_end` and `rom.used_bytes`, not the padded `.gba` file length. The
   optimized build saves at least 19,500 net bytes.
6. The temporary-buffer and explicit heap tileset copy paths both load the
   intended 16 KiB primary or secondary VRAM range and release memory only
   after DMA completion.
7. Tests cover decompression errors, allocation failure, DMA completion and
   freeing, task-slot headroom and exhaustion, peak temporary memory,
   transition latency, and identical used VRAM tile ranges. The repeatable
   latency protocol in the technical specification permits at most a one-frame
   increase and no additional missed VBlank.
8. Secret Base and Cable Club playtesting passes with no map, content, visual,
   script, link, or save change.
9. The production release stays within the active Wayfarer ROM limit and does
   not claim that this feature alone restores the historical reserve deficit.

## References

- [Technical specification](../specs/tileset-storage-optimization.md)
- [Wayfarer runtime foundation specification](../specs/wayfarer-runtime-foundation.md)
