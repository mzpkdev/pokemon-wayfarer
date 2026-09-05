# Wayfarer Surf Pokémon pixel sheet deduplication

PRD: [Wayfarer Surf Pokémon pixel sheet deduplication](../prds/wayfarer-surf-pokemon-pixel-sheet-deduplication.md)
Implemented: No

## Scope

This specification defines how Wayfarer stores one pixel blob for each of 61
normal/shiny Surf Pokémon pairs whose generated 4bpp sheets are byte-identical.
It defines the allowlist, source-level alias, divergence test, linked-output
checks, ROM-size proof, and representative Surf smoke tests.

It does not change Surf gameplay, supported species or forms, source image
assets, palettes, frame tables, sprite templates, or animation behavior. It
does not authorize sharing any of the other 78 normal/shiny pairs.

## Behavior

### Graphics and frame-table sources

The implementation works within the existing Surf mount pipeline:

- `game/graphics/object_events/pics/pokemon/surfable/` contains the authored
  normal and shiny PNG sheets, generated 4bpp sheets, and their separate
  palettes.
- `game/spritesheet_rules.mk` generates each 4bpp sheet from its PNG source.
- `game/src/data/object_events/surfable/surfable_pokemon_graphics.h` declares
  the `gSurfablePokemonPic_*` and `gSurfableShinyPokemonPic_*` pixel symbols
  and the normal, shiny, and modern-shiny palette symbols.
- `game/src/data/object_events/surfable/surfable_pokemon_pic_tables.h` defines
  separate normal and shiny `SpriteFrameImage` tables. Standard sheets have
  six mount frames at indices 0 through 5 and six overlay frames at indices 6
  through 11. Sneasel has eight mount and eight overlay frames, Wailord has
  eight mount frames and no overlay table, and Kyogre has six mount frames and
  no overlay table. The templates suppress overlays for Wailmer, Wailord, and
  Kyogre. These exceptions remain unchanged. Entries use 4x4 tiles for 32x32
  mounts or 8x8 tiles for 64x64 mounts.
- `game/src/data/object_events/surfable/surfable_pokemon_templates.h` keeps the
  normal and shiny palette arrays, normal and shiny mount and overlay template
  arrays, OAM size, and normal or no-flip animation table for every roster
  index.
- `game/src/data/object_events/surfable/surfable_pokemon.h` owns the ordered
  species and form roster. `game/src/surfable.c` resolves that roster, chooses
  normal or shiny palettes and templates, and synchronizes mount and overlay
  movement.

Within the runtime data, only the pixel definitions in
`surfable_pokemon_graphics.h` and the new alias manifest change. The roster,
palettes, picture tables, templates, and runtime selection code remain
structurally and behaviorally unchanged. The implementation also adds the
validation test and its Makefile integration described below.

### Approved aliases

The alias manifest contains exactly the following 61 rows. `Name` is the
suffix shared by `gSurfablePokemonPic_Name` and
`gSurfableShinyPokemonPic_Name`. `Stem` identifies `Stem.4bpp` and
`Stem_shiny.4bpp` under the Surf graphics directory.

| Name | Stem | Bytes |
|---|---|---:|
| Squirtle | `0007_squirtle` | 6,144 |
| Pikachu | `0025_pikachu` | 6,144 |
| Raichu | `0026_raichu` | 6,144 |
| Nidoqueen | `0031_nidoqueen` | 6,144 |
| Psyduck | `0054_psyduck` | 6,144 |
| Poliwag | `0060_poliwag` | 6,144 |
| Poliwhirl | `0061_poliwhirl` | 6,144 |
| Poliwrath | `0062_poliwrath` | 6,144 |
| Tentacool | `0072_tentacool` | 6,144 |
| Tentacruel | `0073_tentacruel` | 6,144 |
| Slowpoke | `0079_slowpoke` | 6,144 |
| Slowbro | `0080_slowbro` | 6,144 |
| Seel | `0086_seel` | 6,144 |
| Kingler | `0099_kingler` | 6,144 |
| Horsea | `0116_horsea` | 6,144 |
| Seadra | `0117_seadra` | 6,144 |
| Goldeen | `0118_goldeen` | 6,144 |
| Omastar | `0139_omastar` | 6,144 |
| Qwilfish | `0211_qwilfish` | 6,144 |
| Mantine | `0226_mantine` | 6,144 |
| Lugia | `0249_lugia` | 24,576 |
| Swampert | `0260_swampert` | 6,144 |
| Lotad | `0270_lotad` | 6,144 |
| Wingull | `0278_wingull` | 6,144 |
| Surskit | `0283_surskit` | 6,144 |
| Exploud | `0295_exploud` | 6,144 |
| Hariyama | `0297_hariyama` | 6,144 |
| Aggron | `0306_aggron` | 6,144 |
| Wailmer | `0320_wailmer` | 6,144 |
| Zangoose | `0335_zangoose` | 6,144 |
| Whiscash | `0340_whiscash` | 6,144 |
| Milotic | `0350_milotic` | 6,144 |
| Spheal | `0363_spheal` | 6,144 |
| Gorebyss | `0368_gorebyss` | 6,144 |
| Latios | `0381_latios` | 6,144 |
| Rayquaza | `0384_rayquaza` | 24,576 |
| Bibarel | `0400_bibarel` | 6,144 |
| Buizel | `0418_buizel` | 6,144 |
| Floatzel | `0419_floatzel` | 6,144 |
| ShellosEast | `0422_shellos_east` | 6,144 |
| ShellosWest | `0422_shellos_west` | 6,144 |
| GastrodonEast | `0423_gastrodon_east` | 6,144 |
| GastrodonWest | `0423_gastrodon_west` | 6,144 |
| Munchlax | `0446_munchlax` | 6,144 |
| Finneon | `0456_finneon` | 6,144 |
| Lumineon | `0457_lumineon` | 6,144 |
| Mantyke | `0458_mantyke` | 6,144 |
| Weavile | `0461_weavile` | 6,144 |
| Lickilicky | `0463_lickilicky` | 6,144 |
| Rhyperior | `0464_rhyperior` | 6,144 |
| Arceus | `0493_arceus` | 24,576 |
| RaichuAlola | `regional_raichu_alola` | 6,144 |
| SlowpokeGalar | `regional_slowpoke_galar` | 6,144 |
| SlowbroGalar | `regional_slowbro_galar` | 6,144 |
| TaurosPaldeaCombat | `regional_tauros_paldea_combat` | 6,144 |
| TaurosPaldeaAqua | `regional_tauros_paldea_aqua` | 6,144 |
| WooperPaldea | `regional_wooper_paldea` | 6,144 |
| SlowkingGalar | `regional_slowking_galar` | 6,144 |
| ObstagoonGalar | `regional_obstagoon` | 6,144 |
| CursolaGalar | `regional_cursola` | 6,144 |
| OverqwilHisui | `regional_overqwil` | 6,144 |

This is 58 times 6,144 bytes plus three times 24,576 bytes, for a total
duplicate payload of 430,080 bytes. The inventory has 139 active pairs in all.
The 78 names absent from this table remain distinct even if a future toolchain
could merge constants opportunistically.

### Alias mechanism

Add
`game/src/data/object_events/surfable/surfable_pokemon_pic_aliases.h` as an
X-macro manifest. Each row records the C symbol suffix and the normal and shiny
asset stems. The manifest is the single reviewed allowlist used by both the C
definitions and the source test.

For each row, `surfable_pokemon_graphics.h` continues to emit the normal pixel
array once with `INCBIN_U32`. It does not emit an `INCBIN_U32` for the shiny
sheet. Instead, a macro expansion declares
`gSurfableShinyPokemonPic_Name` as a strong GCC alias of
`gSurfablePokemonPic_Name` with `__attribute__((alias(...)))`. The repository
already uses this compiler-supported alias facility. Declare the shiny array
with `ARRAY_COUNT(gSurfablePokemonPic_Name)` so both linked symbols have the
same type and size, as well as the external names expected by the shiny picture
tables. Emit each alias after its normal target definition in the same
translation unit. An incomplete `extern const u32 ...[]` alias is not allowed
because the compiler treats it as a one-element array under the project's
warning settings.

A preprocessor name substitution, a second pointer object, identical-code
folding, or a post-link ROM patch does not satisfy this specification. The
linked ELF must contain both named pixel symbols at the same address and
only one copy of their bytes. The 78 non-allowlisted shiny symbols continue to
own their existing `INCBIN_U32` data and link at addresses different from their
normal symbols.

The PNG and palette assets and both normal and shiny conversion rules remain
in place. Generating both 4bpp outputs lets the test detect divergence and lets
an artist remove a row from the allowlist before introducing distinct shiny
pixels.

### Preserved identities and behavior

The implementation must not reorder, combine, or regenerate any of the four
normal/shiny mount and overlay picture-table families. Normal tables continue
to reference `gSurfablePokemonPic_Name`; shiny tables continue to reference
`gSurfableShinyPokemonPic_Name`. Their linked pixel address may match only for
an approved row.

The following data and behavior stay byte-for-byte or entry-for-entry
equivalent unless link-address relocation is unavoidable:

- all normal, shiny, and modern-shiny pixel palettes and palette tags;
- the ordered 139-entry Surf species and form roster;
- all normal and shiny frame offsets, counts, 4x4 or 8x8 dimensions, OAM
  choices, overlay presence, and template indices;
- normal and no-flip animation-table selection;
- shiny detection, modern-shiny option handling, weather palette updates,
  trainer pose, directional frames, movement synchronization, bobbing,
  fishing overlay behavior, map-transition layering, and dismount cleanup; and
- fallback to the ordinary Surf blob for a Pokémon outside the custom mount
  roster.

### Divergence and inventory test

Add a Python standard-library test under
`game/tools/surfable/tests/test_surfable_pokemon_pic_aliases.py` and a focused
Make target named `surfable-pokemon-pixel-alias-test`. Make the test, or a
validation stamp with the same complete inputs, a prerequisite of every
Wayfarer ELF target, including normal, release, mechanics-test, and E2E builds.
Keep the focused target in the Wayfarer `check` path as well. No Wayfarer link
may proceed after an aliased source pair diverges. The test parses active
definitions rather than scanning filenames alone, so commented assets such as
Rhyhorn cannot enter the inventory.

The Make target explicitly depends on the normal and shiny 4bpp output for
every manifest row. Those prerequisites force `spritesheet_rules.mk` to
regenerate both sides from their PNG sources even though the linker no longer
consumes the approved shiny outputs. A clean release build must therefore run
the byte comparison against current generated data rather than stale or
missing files.

The test must fail with the affected symbol and paths when any of these rules
is broken:

1. The manifest does not contain exactly 61 unique names and source pairs.
2. A manifest row does not map to active normal and shiny Surf picture names,
   or either referenced 4bpp file is missing.
3. The two files for any manifest row differ in length or content.
4. The approved set is not 58 6,144-byte pairs plus Lugia, Rayquaza, and Arceus
   as three 24,576-byte pairs, totaling 430,080 bytes.
5. An approved shiny name still has its own `INCBIN_U32`, or does not have one
   strong alias declaration generated from the manifest.
6. The active inventory is not 139 normal names partitioned into 61 aliases
   and 78 separately emitted shiny names.
7. A name outside the manifest uses the alias macro or lacks its independent
   shiny pixel definition.

The test compares raw 4bpp bytes, not PNG pixels or image hashes stored in the
test. This makes any intentional edit actionable: remove the row to keep the
new shiny sheet separate, or make the source pair equal again.

### Linked output and ROM size

Build baseline and optimized Wayfarer release ROMs from clean worktrees at the
same source revision except for this optimization. Use the same compiler,
configuration, release flags, and accepted ROM baseline. Do not run another
map-version build concurrently because generated map files are shared.

Run `make -j"$(nproc)" BUILD=wayfarer release syms` in `game/` for both
worktrees. Preserve `pokewayfarer-release.gba`,
`pokewayfarer-release-size.json`, `pokewayfarer-release.map`, and
`pokewayfarer-release.sym` from each build.

The optimized linked output must satisfy all of these checks:

1. Every approved `gSurfablePokemonPic_Name` and
   `gSurfableShinyPokemonPic_Name` pair appears in the symbol output with
   equal addresses and sizes.
2. All 78 non-allowlisted normal/shiny pixel symbols appear with different
   addresses.
3. No approved pair occupies two data ranges in the linker map. Symbol aliases
   at one range are expected.
4. `rom.used_bytes`, `rom.end_address`, and linked `__rom_end` are exactly
   430,080 bytes (`0x69000`) lower than the baseline. Do not use the `.gba`
   file length for this comparison because the build pads it to 32 MiB.
5. With the current linker classification, the size report's `other` category
   is exactly 430,080 bytes smaller because the Surf picture symbols link
   there. Every other category, including `graphics`, has the same size.
6. The release build still satisfies the Wayfarer ROM limit and reserve checks.

Treat a smaller or larger delta as a failure until unrelated build changes,
alignment, or an accidental extra alias have been identified. The measured
payload is aligned to four bytes, so the approved implementation requires the
exact reduction rather than a minimum threshold.

### Surf smoke tests

Use a Wayfarer release or E2E-enabled ROM that can place a chosen normal or
shiny Pokémon with Surf in the selected party slot. Run both normal and shiny
variants for every row below.

| Mount | Sheet class | Size and identity | Purpose |
|---|---|---|---|
| Squirtle | Aliased | 4x4 base species | Common 32x32 path |
| Alolan Raichu | Aliased | 4x4 regional form | Aliased form identity and palette |
| Wartortle | Separate | 4x4 base species | Non-allowlisted 32x32 control |
| Hisuian Qwilfish | Separate | 4x4 regional form | Non-allowlisted form control |
| Lugia | Aliased | 8x8 base species | Aliased 64x64 path |
| Gyarados | Separate | 8x8 base species | Non-allowlisted 64x64 control |

For each variant, start Surf from land, face and move in all four directions,
pause long enough to observe bobbing, cross a map transition, start and end a
fishing interaction while mounted, dismount, and mount again. Confirm the
correct species or form silhouette, normal or shiny palette, player layering,
overlay, frame order, movement synchronization, and cleanup. No case may show
the fallback Surf blob, borrow another roster entry, retain an overlay after
dismounting, or change animation behavior from the baseline build.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [HM field use](hm-field-use.md)
- [Surf runtime](../../game/src/surfable.c)
- [Surf picture definitions](../../game/src/data/object_events/surfable/surfable_pokemon_graphics.h)
- [Surf frame tables](../../game/src/data/object_events/surfable/surfable_pokemon_pic_tables.h)
- [Surf sprite templates](../../game/src/data/object_events/surfable/surfable_pokemon_templates.h)
- [Surf species and form roster](../../game/src/data/object_events/surfable/surfable_pokemon.h)
