# Exact ROM graphics and tileset aliases

PRD: [Exact ROM asset aliasing](../prds/exact-rom-asset-aliasing.md)
Implemented: No

## Scope

This specification defines the reviewed C-data aliases for non-Surf Arceus
overworld pixels, live regional water-current frames, battle-environment
graphics and tilemaps, and the HNS Bike Shop payloads. It defines their exact
allowlists, canonical payloads, source and generated inputs, types, owners,
baseline linked inventory, alias mechanics, and rollback.

The [audio specification](exact-rom-asset-aliasing-audio.md) owns assembly song
and voicegroup aliases. The [validation specification](exact-rom-asset-aliasing-validation.md)
owns build gating, decoded comparisons, linked-output proof, release arithmetic,
and smoke coverage.

Surf assets, HNS object-event graphics, other tilesets, and every other excluded
family in the PRD are outside this specification.

## Behavior

### Audit basis and accounting

The tables below were audited at commit `443345f`. Baseline addresses come from
a clean production-equivalent Wayfarer release ELF built at that revision. They
prove that the selected payloads were live and separately addressed before
aliasing. The implementation must refresh the addresses in its evidence report
from a clean paired baseline; addresses are not ABI constants.

All C payloads in this specification are four-byte aligned. Their selected
duplicate ranges total 64,240 bytes:

| Family | Release bytes removed | Linker category |
|---|---:|---|
| Arceus overworld forms | 21,828 | `.rom_other` |
| Johto and Alola water currents | 12,288 | `.rom_other` |
| Battle environments | 21,440 | `.rom_graphics` |
| Bike Shop payloads | 8,684 | `.rom_graphics` |

The totals count removed payload ranges only. Manifests, tests, reports, source
files, generated comparison files, and padded ROM bytes do not count.

### Manifest shape

Add four reviewed X-macro manifests under
`game/src/data/exact_rom_aliases/`:

- `arceus_overworld.def`;
- `water_current.def`;
- `battle_environment.def`; and
- `bike_shop.def`.

Each alias row records `group_id`, `canonical_symbol`, `member_symbol`, the
complete C array shape, `expected_bytes`, canonical and member authored paths,
canonical and member generated paths, owner, source linkage (`external` or
`internal`), expected release ELF binding, visibility, and output section. This
separates source linkage from release-LTO internalization. Group IDs and member
symbols are unique across all four files. Canonical rows are explicit and
cannot also appear as members.

The manifests are the only v1 C allowlists. Preprocessor expansions emit alias
declarations and compile-time type and size assertions at the owning source
location. The host validator parses the same rows. Do not maintain a second
symbol list in a test.

### C array alias contract

For an external `const u32` member with `N` bytes, emit the canonical
`INCBIN_U32` definition once, then declare the member in the same translation
unit as:

```c
extern const u32 Member[N / sizeof(u32)]
    __attribute__((alias("Canonical")));
```

Use the corresponding complete type for `u16` and two-dimensional palette
arrays. A 512-byte palette member is `const u16 Member[16][16]`, not an
incomplete outer dimension. Add compile-time assertions that the member and
canonical types are compatible and both have the manifest size. The target
definition may appear earlier or later in the translation unit, but it must be
defined there exactly once. Preserve existing source visibility declarations;
do not add an export or visibility attribute merely to imitate an LTO result.
The linked verifier enforces each row's expected release binding and visibility.

For a static member, use the same complete declaration with `static` linkage.
The compiler supports a same-translation-unit static object alias and emits both
local symbols with one address and size. Do not promote a static symbol to
external linkage merely for validation.

No member may become a pointer variable, preprocessor spelling substitution,
weak symbol, linker-wide fold, or post-link patch. Public declarations in
`game/include/graphics.h` and other headers keep their existing array types.

### Arceus overworld allowlist

`game/src/data/graphics/pokemon.h` owns all 18 external `const u32[]` symbols in
one translation unit through `game/src/pokemon.c`. Retain
`gObjectEventPic_ArceusNormal` as the only emitted payload. Declare the other 17
symbols as strong aliases with complete `const u32[321]` types. Each payload is
1,284 bytes (`0x504`) of SMOL-compressed 4bpp data. In the audited release ELF,
all 18 are `GLOBAL HIDDEN` objects in `.rom_other`; preserve that binding,
visibility, section, and size in the optimized release.

The canonical source is `game/graphics/pokemon/arceus/overworld.png`, with
generated `overworld.4bpp` and `overworld.4bpp.smol`. Each type member keeps its
own authored `game/graphics/pokemon/arceus/<type>/overworld.png` and generated
`overworld.4bpp` and `overworld.4bpp.smol` comparison outputs. The build must
generate those currently unlinked form outputs before validation. Removing a
row restores that form's independent `INCBIN_COMP` from its own path.

| Role | Symbol | Authored member path | Baseline address | Bytes | Saving |
|---|---|---|---:|---:|---:|
| Canonical | `gObjectEventPic_ArceusNormal` | `graphics/pokemon/arceus/overworld.png` | `0x09563CDC` | 1,284 | 0 |
| Alias | `gObjectEventPic_ArceusFighting` | `graphics/pokemon/arceus/fighting/overworld.png` | `0x0950304C` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusFlying` | `graphics/pokemon/arceus/flying/overworld.png` | `0x09502B48` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusPoison` | `graphics/pokemon/arceus/poison/overworld.png` | `0x09502644` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusGround` | `graphics/pokemon/arceus/ground/overworld.png` | `0x09502140` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusRock` | `graphics/pokemon/arceus/rock/overworld.png` | `0x09501C3C` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusBug` | `graphics/pokemon/arceus/bug/overworld.png` | `0x09501738` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusGhost` | `graphics/pokemon/arceus/ghost/overworld.png` | `0x09501234` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusSteel` | `graphics/pokemon/arceus/steel/overworld.png` | `0x09500D30` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusFire` | `graphics/pokemon/arceus/fire/overworld.png` | `0x0950082C` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusWater` | `graphics/pokemon/arceus/water/overworld.png` | `0x09500328` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusGrass` | `graphics/pokemon/arceus/grass/overworld.png` | `0x094FFE24` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusElectric` | `graphics/pokemon/arceus/electric/overworld.png` | `0x094FF920` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusPsychic` | `graphics/pokemon/arceus/psychic/overworld.png` | `0x094FF41C` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusIce` | `graphics/pokemon/arceus/ice/overworld.png` | `0x094FEF18` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusDragon` | `graphics/pokemon/arceus/dragon/overworld.png` | `0x094FEA14` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusDark` | `graphics/pokemon/arceus/dark/overworld.png` | `0x094FE510` | 1,284 | 1,284 |
| Alias | `gObjectEventPic_ArceusFairy` | `graphics/pokemon/arceus/fairy/overworld.png` | `0x094FE00C` | 1,284 | 1,284 |

The owning `sPicTable_Arceus*` entries in
`game/src/data/object_events/object_event_pic_tables_followers.h` continue to
reference their original public symbols. Every normal and shiny overworld
palette in `pokemon.h`, every form record, animation, frame table, icon, and
species identity remains separate. No Surf Arceus symbol or asset path may
appear in this manifest.

### Regional water-current allowlist

`game/src/tileset_anims.c` owns all current frames. The source audit found three
equal sets of eight 1,536-byte raw 4bpp frames under:

- `data/tilesets/primary/general_frlg/anim/water_current_landwatersedge/`;
- `data/tilesets/primary/johto_general_hns/anim/water_current_landwatersedge/`;
  and
- `data/tilesets/primary/alola_island/anim/water_current_landwatersedge/`.

The production release ELF contains only the Johto and Alola frame sets. It has
already garbage-collected the General FRLG set, so v1 leaves that set independent
and does not count it. For ordinals 0 through 7, retain the source-level external
Alola frame as canonical and declare the Johto frame as a same-translation-unit
static alias with complete `const u16[768]` type. Release LTO currently
internalizes the Alola frame symbols; that optimized ELF binding is not a reason
to change their source linkage. All 16 audited release symbols are
`LOCAL DEFAULT` objects in `.rom_other`; the optimized release must retain that
binding, visibility, section, and size while both source-level Alola definitions
remain external and both Johto members remain static.

| Frame | Canonical Alola symbol and baseline address | Internal Johto alias and baseline address | Bytes | Saving |
|---:|---|---|---:|---:|
| 0 | `gTilesetAnims_AlolaIslands_Water_Frame0` at `0x09EA4044` | `sJohtoGeneral_WaterCurrentLandWatersEdge_Frame0` at `0x09EAB4C4` | 1,536 | 1,536 |
| 1 | `gTilesetAnims_AlolaIslands_Water_Frame1` at `0x09EA3A44` | `sJohtoGeneral_WaterCurrentLandWatersEdge_Frame1` at `0x09EAAEC4` | 1,536 | 1,536 |
| 2 | `gTilesetAnims_AlolaIslands_Water_Frame2` at `0x09EA3444` | `sJohtoGeneral_WaterCurrentLandWatersEdge_Frame2` at `0x09EAA8C4` | 1,536 | 1,536 |
| 3 | `gTilesetAnims_AlolaIslands_Water_Frame3` at `0x09EA2E44` | `sJohtoGeneral_WaterCurrentLandWatersEdge_Frame3` at `0x09EAA2C4` | 1,536 | 1,536 |
| 4 | `gTilesetAnims_AlolaIslands_Water_Frame4` at `0x09EA2844` | `sJohtoGeneral_WaterCurrentLandWatersEdge_Frame4` at `0x09EA9CC4` | 1,536 | 1,536 |
| 5 | `gTilesetAnims_AlolaIslands_Water_Frame5` at `0x09EA2244` | `sJohtoGeneral_WaterCurrentLandWatersEdge_Frame5` at `0x09EA96C4` | 1,536 | 1,536 |
| 6 | `gTilesetAnims_AlolaIslands_Water_Frame6` at `0x09EA1C44` | `sJohtoGeneral_WaterCurrentLandWatersEdge_Frame6` at `0x09EA90C4` | 1,536 | 1,536 |
| 7 | `gTilesetAnims_AlolaIslands_Water_Frame7` at `0x09EA1644` | `sJohtoGeneral_WaterCurrentLandWatersEdge_Frame7` at `0x09EA8AC4` | 1,536 | 1,536 |

Add complete forward declarations for the Alola targets before the Johto block,
then expand the internal aliases where the Johto frame definitions currently
live. Keep the Alola definitions and both regional pointer tables. The callbacks
continue to use 48 tiles at base tile 416. The Johto waterfall path continues to
take its 12-tile slice starting 34 tiles into the same frame. Frame order and
timer division do not change.

### Battle-environment allowlist

`game/src/data/graphics/battle_environment.h` defines all approved external
`const u32[]` arrays in the `game/src/graphics.c` translation unit. Normal
environment ownership is in `gBattleEnvironmentInfo`; modern ownership is in
`sModernBattleGfx`. Runtime consumers in `game/src/battle_bg.c` remain unchanged.
All environment palettes, including morning, night, legendary, Gym, and stadium
variants, remain separate. Every listed release symbol is a `GLOBAL DEFAULT`
object in `.rom_graphics`; preserve that binding, visibility, section, and row
size after aliasing.

The generated-path suffix defines the authored input for every table member:

- `tiles.4bpp.smol` and `anim_tiles.4bpp.smol` come from the same path ending in
  `tiles.png` or `anim_tiles.png`;
- `map.bin.smolTM` and `anim_map.bin.smolTM` come from the same path ending in
  `map.bin` or `anim_map.bin`.

Every path starts at `game/graphics/battle_environment/<directory>/`. The exact
symbol-suffix to directory mapping is:

| Symbol suffix | Directory | Symbol suffix | Directory |
|---|---|---|---|
| `BlueBuilding` | `blue_building` | `BlueBuildingModern` | `blue_building_modern` |
| `Building` | `building` | `BuildingModern` | `building_modern` |
| `Cave` | `cave` | `CaveModern` | `cave_modern` |
| `CaveWaterModern` | `cave_water_modern` | `LongGrass` | `long_grass` |
| `LongGrassModern` | `long_grass_modern` | `MountainSnow` | `mountain_snow` |
| `MountainSnowModern` | `mountain_snow_modern` | `PondWater` | `pond_water` |
| `PondWaterModern` | `pond_water_modern` | `Rayquaza` | `sky` |
| `Rock` | `rock` | `RockModern` | `rock_modern` |
| `RockSnow` | `rock_snow` | `RockSnowModern` | `rock_snow_modern` |
| `Sand` | `sand` | `SandModern` | `sand_modern` |
| `SkyModern` | `sky_modern` | `SnowCave` | `snow_cave` |
| `SnowCaveModern` | `snow_cave_modern` | `Stadium` | `stadium` |
| `TallGrass` | `tall_grass` | `TallGrassModern` | `tall_grass_modern` |
| `Underwater` | `underwater` | `UnderwaterModern` | `underwater_modern` |
| `Volcano` | `volcano` | `VolcanoModern` | `volcano_modern` |
| `Water` | `water` | `WaterModern` | `water_modern` |

Together, the suffix mapping and payload kind in each allowlist row identify the
full authored and generated paths without discovery or wildcard expansion. All
listed symbols are in `.rom_graphics`, are owned by
`game/src/data/graphics/battle_environment.h` through the
`game/src/graphics.c` translation unit, and retain their original
`gBattleEnvironmentInfo` or `sModernBattleGfx` consumers.

The following 25 groups are the complete v1 allowlist. The first symbol in each
row is canonical. Parenthesized hexadecimal values are baseline addresses; all
members have the row's complete `const u32[bytes / 4]` type. `Saving` is
`bytes * (member count - 1)`.

| ID | Payload | Canonical and aliases with baseline addresses | Bytes | Saving |
|---|---|---|---:|---:|
| BE01 | Building entry map | `gBattleEnvironmentAnimTilemap_Building` (`0x086FBFFC`); `BlueBuilding` (`0x086FB698`); `BlueBuildingModern` (`0x086F3740`); `BuildingModern` (`0x086EAC34`) | 152 | 456 |
| BE02 | Long Grass entry map | `gBattleEnvironmentAnimTilemap_LongGrass` (`0x086FF4C8`); `LongGrassModern` (`0x086F11A0`) | 44 | 44 |
| BE03 | Pond Water entry map | `gBattleEnvironmentAnimTilemap_PondWater` (`0x086FEA94`); `PondWaterModern` (`0x086F0770`) | 60 | 60 |
| BE04 | Sky entry map | `gBattleEnvironmentAnimTilemap_Rayquaza` (`0x086FBB64`); `SkyModern` (`0x086EA798`) | 56 | 56 |
| BE05 | Rock entry map | `gBattleEnvironmentAnimTilemap_Rock` (`0x086FE7D4`); `RockSnow` (`0x086FE15C`); `MountainSnow` (`0x086FDAE4`); `RockModern` (`0x086F04B0`); `RockSnowModern` (`0x086EEF1C`); `MountainSnowModern` (`0x086EE274`) | 152 | 760 |
| BE06 | Sand entry map | `gBattleEnvironmentAnimTilemap_Sand` (`0x086FF1F4`); `SandModern` (`0x086F0ECC`) | 208 | 208 |
| BE07 | Tall Grass entry map | `gBattleEnvironmentAnimTilemap_TallGrass` (`0x086FF9BC`); `TallGrassModern` (`0x086F1694`) | 72 | 72 |
| BE08 | Underwater entry map | `gBattleEnvironmentAnimTilemap_Underwater` (`0x086FEF60`); `UnderwaterModern` (`0x086F0C34`) | 332 | 332 |
| BE09 | Cave entry map | `gBattleEnvironmentAnimTilemap_Volcano` (`0x086FD0E0`); `SnowCave` (`0x086FC6DC`); `Cave` (`0x086FC0F4`); `CaveWaterModern` (`0x086F23C4`); `VolcanoModern` (`0x086EC8FC`); `SnowCaveModern` (`0x086EB28C`); `CaveModern` (`0x086EAD2C`) | 164 | 984 |
| BE10 | Water entry map | `gBattleEnvironmentAnimTilemap_Water` (`0x086FEC40`); `WaterModern` (`0x086F091C`) | 76 | 76 |
| BE11 | Building entry tiles | `gBattleEnvironmentAnimTiles_Building` (`0x086FC094`); `BlueBuilding` (`0x086FB730`); `BlueBuildingModern` (`0x086F37D8`); `BuildingModern` (`0x086EACCC`) | 96 | 288 |
| BE12 | Modern cave entry tiles | `gBattleEnvironmentAnimTiles_CaveWaterModern` (`0x086F2468`); `VolcanoModern` (`0x086EC9A0`); `SnowCaveModern` (`0x086EB330`); `CaveModern` (`0x086EADD0`) | 1,212 | 3,636 |
| BE13 | Long Grass entry tiles | `gBattleEnvironmentAnimTiles_LongGrass` (`0x086FF4F4`); `LongGrassModern` (`0x086F11CC`) | 1,224 | 1,224 |
| BE14 | Pond Water entry tiles | `gBattleEnvironmentAnimTiles_PondWater` (`0x086FEAD0`); `PondWaterModern` (`0x086F07AC`) | 368 | 368 |
| BE15 | Rock entry tiles | `gBattleEnvironmentAnimTiles_Rock` (`0x086FE86C`); `RockSnow` (`0x086FE1F4`); `MountainSnow` (`0x086FDB7C`); `RockModern` (`0x086F0548`); `RockSnowModern` (`0x086EEFB4`); `MountainSnowModern` (`0x086EE30C`) | 552 | 2,760 |
| BE16 | Sand entry tiles | `gBattleEnvironmentAnimTiles_Sand` (`0x086FF2C4`); `SandModern` (`0x086F0F9C`) | 516 | 516 |
| BE17 | Tall Grass entry tiles | `gBattleEnvironmentAnimTiles_TallGrass` (`0x086FFA04`); `TallGrassModern` (`0x086F16DC`) | 712 | 712 |
| BE18 | Standard cave entry tiles | `gBattleEnvironmentAnimTiles_Volcano` (`0x086FD184`); `SnowCave` (`0x086FC780`); `Cave` (`0x086FC198`) | 1,348 | 2,696 |
| BE19 | Modern building background map | `gBattleEnvironmentTilemap_BuildingModern` (`0x086F4450`); `BlueBuildingModern` (`0x086F3838`) | 172 | 172 |
| BE20 | Standard flat background map | `gBattleEnvironmentTilemap_Sand` (`0x08702128`); `Building` (`0x087009E4`); `Stadium` (`0x08700504`); `Rayquaza` (`0x086FFCCC`); `BlueBuilding` (`0x086FB790`) | 116 | 464 |
| BE21 | Standard shared background map | `gBattleEnvironmentTilemap_TallGrass` (`0x08702A2C`); `LongGrass` (`0x0870255C`); `Underwater` (`0x08701D34`); `Water` (`0x0870194C`); `PondWater` (`0x0870158C`); `Rock` (`0x087011D4`); `Cave` (`0x08700DB8`); `RockSnow` (`0x086FE41C`); `MountainSnow` (`0x086FDDA4`); `Volcano` (`0x086FD6C8`); `SnowCave` (`0x086FCCC4`); `UnderwaterModern` (`0x086F79D0`) | 116 | 1,276 |
| BE22 | Standard building background tiles | `gBattleEnvironmentTiles_Building` (`0x08700AB8`); `BlueBuilding` (`0x086FB864`) | 768 | 768 |
| BE23 | Modern building background tiles | `gBattleEnvironmentTiles_BuildingModern` (`0x086F455C`); `BlueBuildingModern` (`0x086F3944`) | 352 | 352 |
| BE24 | Standard cave background tiles | `gBattleEnvironmentTiles_Cave` (`0x08700E8C`); `Volcano` (`0x086FD79C`); `SnowCave` (`0x086FCD98`) | 840 | 1,680 |
| BE25 | Standard rock background tiles | `gBattleEnvironmentTiles_Rock` (`0x087012A8`); `RockSnow` (`0x086FE4F0`); `MountainSnow` (`0x086FDE78`) | 740 | 1,480 |

Suffix-only names in this table inherit the canonical symbol prefix shown before
them. For example, `BlueBuilding` in BE01 means
`gBattleEnvironmentAnimTilemap_BlueBuilding`. The manifest contains full names
and full generated paths; it cannot use suffix inference.

Keep each environment entry, palette, decompression mode, background choice,
and time-of-day selector independent. Validation compares both the final
compressed bytes and decoded tile or tilemap bytes so a coincidental compressed
match with incompatible declared semantics cannot pass.

### Bike Shop allowlist

The HNS `BikeShop_Hns` and `JohtoBikeShop_Hns` secondary tilesets are complete
payload matches. Use `BikeShop_Hns` as canonical for the following four pairs.
All definitions share the `game/src/tilesets.c` translation unit through
`graphics.h`, `metatiles.h`, and `headers.h`.

| Payload | Canonical symbol, path, baseline address | Alias symbol, path, baseline address | Complete type | Saving |
|---|---|---|---|---:|
| FastSMOL tiles | `gTilesetTiles_BikeShop_Hns`, `secondary/bike_shop_hns/tiles.4bpp.fastSmol`, `0x088B8B30` | `gTilesetTiles_JohtoBikeShop_Hns`, `secondary/johto_bike_shop_hns/tiles.4bpp.fastSmol`, `0x08887140` | `const u32[1746]` | 6,984 |
| Metatiles | `gMetatiles_BikeShop_Hns`, `secondary/bike_shop_hns/metatiles.bin`, `0x087E0538` | `gMetatiles_JohtoBikeShop_Hns`, `secondary/johto_bike_shop_hns/metatiles.bin`, `0x087B1984` | `const u16[528]` | 1,056 |
| Attributes | `gMetatileAttributes_BikeShop_Hns`, `secondary/bike_shop_hns/metatile_attributes.bin`, `0x087E04B4` | `gMetatileAttributes_JohtoBikeShop_Hns`, `secondary/johto_bike_shop_hns/metatile_attributes.bin`, `0x087B1900` | `const u16[66]` | 132 |
| Palettes | `gTilesetPalettes_BikeShop_Hns`, `secondary/bike_shop_hns/palettes/00..15.gbapal`, `0x088B8930` | `gTilesetPalettes_JohtoBikeShop_Hns`, `secondary/johto_bike_shop_hns/palettes/00..15.gbapal`, `0x08886F40` | `const u16[16][16]` | 512 |

All paths in this table are under `game/data/tilesets/`. The tile source is
`tiles.png`; the other authored inputs are `metatiles.bin`,
`metatile_attributes.bin`, and palette files `00.pal` through `15.pal`. Generate
and compare the two 4bpp streams before comparing and decoding the FastSMOL
outputs. Compare all 16 palette rows and their order, not only the concatenated
size. All eight audited release symbols are `GLOBAL DEFAULT` objects in
`.rom_graphics`; preserve that binding, visibility, section, and complete type.
Change the canonical palette definition itself from incomplete
`const u16[][16]` to `const u16[16][16]`, and declare the member alias with that
same complete type. Both dimensions participate in compile-time type and size
checks.

Keep `gTileset_BikeShop_Hns` and `gTileset_JohtoBikeShop_Hns` as separate
24-byte descriptors pointing through their original field names. Both remain
compressed secondary tilesets with a null callback. Cianwood and Cerulean HNS
layouts continue to select `gTileset_BikeShop_Hns`; Goldenrod continues to
select `gTileset_JohtoBikeShop_Hns`. Do not alias a descriptor, layout, map,
primary tileset, animation callback, or palette outside this pair.

### Excluded HNS object-event family

Do not create an HNS object-event alias manifest in v1. The earlier 14,336-byte
figure has no recoverable reviewed symbol list in repository history. A current
generated-byte scan finds 69 non-Surf HNS duplicate groups totaling 144,640 raw
bytes, but that broad scan does not establish ownership, palette, frame-table,
loaded-size, or release-link compatibility. It cannot substitute for an
allowlist.

The validator treats every `gObjectEventPic_*_hns` row as forbidden outside the
Arceus family. Representative exact pairs such as HNS and non-HNS Biker sprites
remain independent negative controls. A later PRD may admit a smaller reviewed
set after reconstructing its provenance and paired release saving.

### Source changes and rollback

The later implementation changes only payload definitions, the four manifests,
and validation/build wiring. It does not edit authored PNG, palette, map, MIDI,
layout, or script content.

Rollback one group by removing its manifest rows and restoring each member's
original `INCBIN` definition. Restore Johto current frame definitions from their
own generated paths. The owning tables and public declarations do not change in
either direction.

## References

- [Validation and release proof](exact-rom-asset-aliasing-validation.md)
- [Audio aliases](exact-rom-asset-aliasing-audio.md)
- [Battle environment definitions](../../game/src/data/graphics/battle_environment.h)
- [Battle environment owners](../../game/src/data/battle_environment.h)
- [Arceus graphics definitions](../../game/src/data/graphics/pokemon.h)
- [Follower frame tables](../../game/src/data/object_events/object_event_pic_tables_followers.h)
- [Tileset animations](../../game/src/tileset_anims.c)
- [Tileset payloads](../../game/src/data/tilesets/graphics.h)
- [Tileset metatiles](../../game/src/data/tilesets/metatiles.h)
- [Tileset descriptors](../../game/src/data/tilesets/headers.h)
