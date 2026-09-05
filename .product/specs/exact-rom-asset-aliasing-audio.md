# Exact ROM battle-audio aliases

PRD: [Exact ROM asset aliasing](../prds/exact-rom-asset-aliasing.md)
Implemented: No

## Scope

This specification defines the only v1 audio aliases: the byte-identical
Rayquaza and Kyogre/Groudon battle-song sequence objects and their byte-identical
voicegroups. It defines exact assembly boundaries, labels, source ownership,
build-object selection, table identities, validation, and rollback.

It does not authorize general audio compression, song deduplication, sample
deduplication, cry changes, or any other equal audio data. C graphics and tileset
aliases are owned by the [graphics specification](exact-rom-asset-aliasing-graphics.md).
Cross-family release proof is owned by the [validation specification](exact-rom-asset-aliasing-validation.md).

## Behavior

### Approved audio groups

The two approved groups total 3,304 bytes in the release `audio` category.
Baseline addresses were audited in a clean production-equivalent Wayfarer
release ELF built at commit `443345f`; the implementation must refresh them in
its paired report. Both retained ranges are in `.rom_audio`.

| ID | Canonical payload | Alias payload | Source | Baseline spans | Bytes saved |
|---|---|---|---|---|---:|
| AU01 | `mus_vs_rayquaza` sequence object | `mus_vs_kyogre_groudon` sequence object | `sound/songs/midi/mus_vs_rayquaza.mid` and `mus_vs_kyogre_groudon.mid` | Rayquaza `0x0944065C..0x09440D44`; Kyogre/Groudon `0x0943EAC0..0x0943F1A8` | 1,768 |
| AU02 | `voicegroup_vs_rayquaza` | `voicegroup_vs_kyogre_groudon` | `sound/voicegroups/vs_rayquaza.inc` and `vs_kyogre_groudon.inc` | Rayquaza `0x08956DE0..0x089573E0`; Kyogre/Groudon `0x0895987C..0x08959E7C` | 1,536 |

Both MIDI files are 6,553-byte exact source matches. Their `midi.cfg` rows use
the same conversion settings except for the public song and voicegroup names:
`-E -R50`, volume 80, priority 1, and the family-specific `-G` value. Normalized
generated assembly and separately assembled `.rodata` payloads are exact
matches. Each sequence object is `0x6E8` bytes.

Both voicegroup include files contain 128 records of 12 bytes. After normalizing
the public label, their source records, sample references, keysplits, assembled
bytes, and relocation layout match. Each voicegroup is `0x600` bytes.

No other MIDI, generated song assembly, voicegroup, sample, keysplit, cry, or
song-table row may appear in the v1 audio allowlist.

### Sequence boundaries

The sequence payload is the complete generated object, not only the public song
header. Relative to the start of either sequence object's `.rodata` range:

| Item | Offset |
|---|---:|
| Track 1 | `0x000` |
| Track 2 | `0x10E` |
| Track 3 | `0x263` |
| Track 4 | `0x378` |
| Track 5 | `0x42D` |
| Track 6 | `0x510` |
| Public song header | `0x6C8` |
| End of header and object | `0x6E8` |

The 32-byte header contains the track count, block count, priority, reverb,
voicegroup address, and six track addresses. An implementation that aliases the
header while leaving a second set of track bytes does not satisfy this
specification.

The canonical physical object keeps Rayquaza's track-local labels and relocation
targets. `mus_vs_kyogre_groudon` becomes a strong linked label at the same header
address as `mus_vs_rayquaza`. No external caller refers to the omitted
Kyogre/Groudon track-local labels, so they need not remain in the linked ELF.
Validation still regenerates and compares the full Kyogre/Groudon source object.

### Song object and label mechanism

Add `game/sound/exact_rom_audio_aliases.mk` as the reviewed build manifest for
AU01. It records the canonical and member MIDI stems, expected `.rodata` size
`0x6E8`, header offset `0x6C8`, header size `0x20`, expected track offsets, and
public labels.

For `POKEMON_WAYFARER` builds only:

1. Generate both `.s` files from their separate `.mid` files using their existing
   `midi.cfg` rows.
2. Assemble both files for source validation.
3. Exclude only `$(MID_BUILDDIR)/mus_vs_kyogre_groudon.o` from the final
   `MID_OBJS` list.
4. Link `mus_vs_rayquaza.o` once.
5. Add the strong linker symbol assignment
   `mus_vs_kyogre_groudon=mus_vs_rayquaza` through the Wayfarer link flags or a
   generated linker fragment consumed from the same manifest.

The linker assignment is permitted here because both names are assembly labels
used only as addresses in the song table. It is not a C object alias and does not
change an array type. The final symbol table must contain both global labels at
the same header address. The validation report assigns both a logical 32-byte
header size and proves that the one retained physical span is `0x6E8` bytes.

Do not use a pointer object, weak alias, runtime table rewrite, second header,
post-link ROM patch, or linker-wide identical-data folding. The duplicate MIDI
object must still be built for comparison but must not be passed to the final
Wayfarer link.

Standalone Emerald, FireRed, LeafGreen, and HNS builds retain their current two
independent objects unless a later product decision deliberately broadens the
scope and supplies equivalent validation.

### Voicegroup mechanism

`game/data/sound_data.s` includes `game/sound/voice_groups.inc`, all voicegroup
payloads, and then the song table in one assembly translation unit. Keep the
existing `voicegroup_vs_rayquaza` include as canonical. Under
`POKEMON_WAYFARER`, replace the later duplicate include with a strong shared
label:

```asm
    .global voicegroup_vs_kyogre_groudon
    .set voicegroup_vs_kyogre_groudon, voicegroup_vs_rayquaza
```

For non-Wayfarer builds, retain the existing
`sound/voicegroups/vs_kyogre_groudon.inc` include. The audio manifest records
both source include paths, the 128-record count, 12-byte record size, and total
`0x600` bytes. If the assembler permits explicit object sizes for the shared
labels, emit matching `.type` and `.size` metadata. The linked verifier must not
depend on that optional metadata; it also checks the manifest-bounded span and
the next voicegroup label.

No instrument, sample, keysplit, envelope, pan, pitch, or priority data changes.
The duplicate source include remains tracked and is assembled in a comparison
fixture so future sound work can diverge it deliberately.

### Preserved IDs, tables, and callers

Keep both song constants and table rows:

| Identity | Song ID | Song-table row | Required label |
|---|---:|---:|---|
| `MUS_VS_RAYQUAZA` | 470 | 479 | `mus_vs_rayquaza` |
| `MUS_VS_KYOGRE_GROUDON` | 480 | 489 | `mus_vs_kyogre_groudon` |

`game/sound/song_table.inc` continues to contain both entries with their current
music-player and unknown fields. `game/src/battle_setup.c` and
`game/src/pokemon.c` continue to choose the same IDs: Rayquaza selects
`MUS_VS_RAYQUAZA`; Kyogre and Groudon select `MUS_VS_KYOGRE_GROUDON`.

The two table entries may contain the same linked song-header address after
aliasing. Do not merge the constants, delete a table row, change an index, or
rewrite callers to a different ID.

### Audio validation

Before every Wayfarer link that consumes the manifest, the audio test must:

1. verify that the two MIDI files are byte-identical inputs;
2. regenerate both `.s` files with their current `midi.cfg` options and reject a
   stale checked-in or build output;
3. parse exactly six tracks at offsets `0x000`, `0x10E`, `0x263`, `0x378`,
   `0x42D`, and `0x510`, with the header at `0x6C8` and the end at `0x6E8`;
4. assemble both sources in isolation and compare `.rodata` bytes after
   normalizing self-relocations and public names;
5. compare relocation counts, offsets, types, addends, and normalized targets;
6. parse both voicegroup includes as exactly 128 12-byte records and compare
   normalized source, assembled bytes, relocations, samples, and keysplits;
7. confirm both song IDs, both song-table positions, both labels, and the known
   callers remain present; and
8. reject any audio manifest row or filtered MIDI object outside AU01 and AU02.

The linked checks then prove one `0x6E8` sequence range and one `0x600`
voicegroup range, equal public addresses, retained IDs and rows, and an exact
3,304-byte release reduction.

Negative fixtures cover changed MIDI events, changed conversion flags, a moved
track or header boundary, a changed relocation target, a changed voicegroup
record, a missing label, a duplicate manifest row, an extra filtered MIDI
object, and an attempted third audio alias.

### Rollback

Remove AU01 from the Wayfarer build manifest, restore
`mus_vs_kyogre_groudon.o` to `MID_OBJS`, and remove its linker assignment. For
AU02, restore the duplicate voicegroup include and remove the `.set`. Both source
MIDI files and voicegroup includes already remain in place. Song IDs, tables,
and callers need no rollback change.

## References

- [Validation and release proof](exact-rom-asset-aliasing-validation.md)
- [MIDI conversion configuration](../../game/sound/songs/midi/midi.cfg)
- [Song table](../../game/sound/song_table.inc)
- [Voicegroup includes](../../game/sound/voice_groups.inc)
- [Sound-data owner](../../game/data/sound_data.s)
- [Song constants](../../game/include/constants/songs.h)
- [Battle music selection](../../game/src/battle_setup.c)
- [Legendary music selection](../../game/src/pokemon.c)
