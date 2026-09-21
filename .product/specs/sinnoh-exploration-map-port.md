# Sinnoh exploration map port

PRD: [Sinnoh exploration foundation](../prds/sinnoh-exploration-port.md)
Implemented: No

## Scope

This specification defines how the frozen Sinnoh geography catalog is added to
the Wayfarer build. It covers source identity, manifest selection, map and
layout provenance, stable identifiers, topology repair, empty event policy,
minimal regional dispatch, tooling, delivery order, and validation.

The [asset reuse and ROM footprint specification](sinnoh-asset-reuse-and-rom-footprint.md)
owns tileset and blockdata classification, exact aliases, compression choices,
and size accounting. Production travel, regional-map art, Fly destinations,
healing services, encounters, Trainers, story, Pokedex behavior, badges, and
League participation require later product and technical documents.

## Behavior

### Frozen source catalog

The sole source baseline is
[`LiderMorti00/Sinnoh-pokeemerald-expansion`](https://github.com/LiderMorti00/Sinnoh-pokeemerald-expansion)
commit `4eed17cc63c4ec8c24fbb20fe49e8d65cb4870d8`.

Create `game/src/data/wayfarer_sinnoh_maps.json` as the checked-in authority.
It has a schema version, source URL, source commit, deterministic ordering, and
one record per selected map. Generation never fetches the donor repository.

The manifest freezes these nine source groups:

| Source group | Count |
| --- | ---: |
| `gMapGroup_SinnohTownsRoutes` | 55 |
| `gMapGroup_SpecialAreasSinnoh` | 5 |
| `gMapGroup_DungeonsSinnoh` | 8 |
| `gMapGroup_IndoorSinnoh` | 14 |
| `gMapGroup_IndoorTwinleaf` | 6 |
| `gMapGroup_IndoorSandgem` | 6 |
| `gMapGroup_IndoorJubilife` | 20 |
| `gMapGroup_IndoorOreburgh` | 13 |
| `gMapGroup_IndoorFloaroma` | 6 |
| **Total** | **133** |

Each record contains at least:

- source map and layout names;
- source group and deterministic order;
- map JSON, layout JSON, `map.bin`, and `border.bin` hashes;
- width, height, source and target tileset pair, and layout format;
- source and target map sections, music, weather, map type, and field-action
  properties;
- every warp and connection with its source and destination coordinates;
- asset-manifest record IDs;
- topology status and any approved repair; and
- inclusion state with an explicit reason for any future exclusion.

Generation fails unless the manifest still resolves to exactly 133 maps and
133 layouts. It also fails on an unreviewed source hash, missing source path,
new source-group member, duplicate symbol, unknown tileset, or topology record
whose reviewed state no longer matches its input.

The source facts below are acceptance fixtures:

| Input | Frozen count |
| --- | ---: |
| Warps | 233 |
| Outdoor connections | 114 |
| Object events | 0 |
| Coordinate events | 0 |
| Background events | 0 |
| Nonempty map-script tables | 0 |
| Wild-encounter profiles | 0 |

### Catalog and provenance

Add `sinnoh` as an explicit map source version. `wayfarer` generation may
select it through the frozen manifest; standalone HNS, Emerald, FireRed, and
LeafGreen generation must not select it. Do not classify a Sinnoh record as
Emerald merely because its metatile format is compatible.

All selected layouts use `layout_version: "emerald"`. This describes their
binary metatile format, not their region or source ownership. Porymap and the
Metatiles tool must open them with Emerald-format behavior while Cartographer
reports their source and physical region as Sinnoh.

Append the nine groups after every existing group. Append their maps and
layouts without changing any existing numeric value. Preserve the donor's
public map, layout, and tileset symbol names when they do not collide; the
manifest records any required rename and keeps the donor name as provenance.
Runtime and generated data use symbols rather than donor numeric IDs.

Every manifest record stores both `source_map_section` and
`target_map_section`. Generation resolves only the target symbol; the source
value is provenance and is never assumed to be safe merely because that symbol
already exists in Wayfarer. Append Sinnoh-specific target sections and labels
without changing existing IDs. At minimum, map the donor's
`MAPSEC_LITTLEROOT_TOWN` values on `TwinleafTown_Haouse1` and
`TwinleafTown_House2` to the appended Twinleaf Town section, and map the donor's
`MAPSEC_POKEMON_LEAGUE` value on `PokmonLeague` to an appended Sinnoh Pokemon
League section. The generator rejects a source/target pair that is absent from
the manifest, an unreviewed collision with an existing section, or a target
whose label and regional ownership do not match the manifest.

The generator enforces all existing signed-byte bounds. After insertion:

- the total map-group count remains at or below 127;
- every group contains at most 127 maps;
- every saved or scripted map group and map number round-trips through its
  declared field type; and
- no existing map, layout, group, map section, heal location, or warp constant
  changes value.

Add a focused `HAS_SINNOH_CONTENT` capability enabled only for Wayfarer. Do not
enable a whole Emerald, FRLG, or HNS asset family to satisfy one Sinnoh
dependency.

### Imported files

Check the selected map JSON, layout blockdata, borders, and reviewed source
assets into `game/`; the build does not read outside the repository. Keep the
unmodified donor inputs reviewable even when generation selects an existing
Wayfarer payload.

Do not hand-edit imported `map.bin` files. A required collision, elevation, or
tile repair is made in Porymap with the declared Emerald layout version, saved
as a new reviewed source hash, and recorded in the manifest. Manual map JSON
repairs remain allowed when schema-valid and recorded.

The source's empty object, coordinate, background, and map-script tables remain
empty. Do not create placeholder NPCs, signs, items, Trainers, service clerks,
or story state. Do not link empty generated event or script payloads when a
null/empty catalog representation is sufficient.

### Warp and connection policy

Generation validates every selected warp and connection against the complete
Wayfarer catalog.

For each outdoor connection, verify:

- the destination is selected;
- the reverse connection exists unless the manifest marks a reviewed one-way
  transition;
- source and destination edges overlap at the declared offset;
- every overlapping walkable tile has compatible collision and elevation;
- camera movement does not expose invalid border data; and
- walking the seam cannot place the player outside the destination layout.

For each warp, verify:

- source and destination map IDs resolve;
- the destination warp index exists or an explicit coordinate is valid;
- return behavior is present unless the source transition is intentionally
  one-way;
- the destination tile is in bounds, walkable, and elevation-compatible; and
- door or cave presentation uses an already selected compatible animation.

A facade with no selected source interior has no live warp. Do not route it to
a generic house. A selected interior with no safe source entrance is retained
for catalog completeness but remains unreachable in production until a later
review supplies an entrance.

Every repair lives in the manifest as structured data naming the original
edge, replacement edge, reason, and validation fixture. Prefix rules and
name-based repair heuristics are forbidden.

### Regional runtime behavior

Every selected map resolves to `REGION_SINNOH` from explicit provenance. Extend
the Wayfarer map-region resolver so it can distinguish:

- the three current core regions used by the League circuit;
- Sinnoh as a valid loaded-map region; and
- HNS auxiliary regions such as Alola, Hisui, and Sinjoh.

Do not add Sinnoh to the Kanto-Johto-Hoenn League circuit or treat it as an HNS
auxiliary map. Existing core-region predicates retain their present meaning;
add a narrower map-region predicate where the loader, diagnostics, or test
harness needs to accept Sinnoh.

The saved current-region field and visited-region mask may represent
`REGION_SINNOH`, but no Sinnoh story bank, badges, Champion state, Trainer
defeat bank, or regional Pokedex state is allocated in this milestone. New-game
reset, save validation, load recovery, and map transitions must preserve or
clear the Sinnoh bit deliberately and must not mask it away as corrupt data.
Compile-time save-size assertions remain unchanged.

Loading a Sinnoh map updates the current map region. Leaving it updates the
region from the destination map. A stale saved value never overrides loaded-map
provenance. Existing Johto/Kanto context handling and Hoenn source-bit behavior
remain unchanged.

### Entry, recovery, and map UI

Production code adds no travel menu entry or public warp into Sinnoh. E2E and
debug builds may expose one test-only dispatch that enters a manifest-selected
map and one test-only escape that returns to the fixture's saved origin. These
helpers compile out of production and write no story, ticket, origin, badge,
League, or encounter state.

Sinnoh maps receive their manifest-mapped target map-section names so map-name
popups and diagnostics are meaningful. Diagnostics also expose the donor
source section when it differs. This milestone adds no Sinnoh Town Map image,
cursor grid, Fly markers, or Pokedex-area page. Attempting to open a
region-specific Town Map or Fly selection while a test is on a Sinnoh map must
fail safely or use an explicit test-only stub; it must not render Hoenn, Johto,
Kanto, Sevii, or Sinjoh coordinates as Sinnoh.

No Sinnoh heal location is registered. Blackout, Teleport, Dig, and Escape
Rope use their existing eligibility rules; when recovery needs a heal location,
the last valid pre-Sinnoh destination remains authoritative. A future public
entry specification must add a local arrival heal and every selected Pokemon
Center service before production access is enabled.

### Shared field behavior

Retain each source map's cycling, running, escaping, Flash, weather, and battle
scene properties when they are valid in Wayfarer. Normal terrain behaviors use
the current badge-free field-action system. No map may assume that a visible
Defog or Rock Climb obstacle grants a new engine mechanic.

The generated audit lists every metatile behavior referenced by the selected
maps and rejects an unknown or format-incompatible behavior. It also lists
every route whose complete navigation currently requires Surf, Cut, Strength,
Rock Smash, Waterfall, Flash, or another supported action. This inventory is
evidence for the later open-world traversal design; it does not silently remove
or grant an obstacle.

### Tooling

Update Cartographer and Metatiles together:

- add Sinnoh source and physical-region classification;
- accept `sinnoh` provenance with Emerald layout format;
- render every selected tileset pair and layout;
- report missing graphics, palettes, metatiles, attributes, borders, and
  animation dependencies;
- show every warp and connection, including reviewed repairs;
- detect null destinations, nonreciprocal edges, bounds failures, seam
  mismatches, and closed facades with live warps; and
- keep standalone and existing Wayfarer catalog output stable.

Porymap project configuration must expose the imported layouts without
changing another layout version. Generated files remain deterministic and
clean after a second generation pass.

### Delivery order

1. Resolve donor asset provenance and freeze the manifest, hashes, counts, and
   proposed symbol names.
2. Add mapjson, constants, Cartographer, Metatiles, and Porymap support for
   Sinnoh provenance without selecting any production content.
3. Land exact asset reuse and independent Sinnoh variants according to the
   asset specification.
4. Import the catalog, append identifiers, and make generation pass with empty
   events and scripts.
5. Resolve every warp, connection, seam, and unsupported dependency; add
   test-only entry and recovery.
6. Run raw and, when approved, compressed builds plus the full topology and
   visual validation suite.
7. Measure a clean paired Wayfarer release. Do not enable production access or
   merge a release configuration that misses the protected reserve.

### Validation

The implementation must provide automated evidence for:

- exact source commit, manifest schema, 133 maps, 133 layouts, nine groups,
  233 warps, 114 connections, and the frozen empty-content counts;
- stable preexisting IDs and signed-byte group/map bounds;
- deterministic generation with no untracked second-pass changes;
- complete header, layout, tileset, map-section, music, weather, warp,
  connection, border, and script-symbol resolution;
- complete manifest-backed source-to-target map-section resolution, including
  collision fixtures for Twinleaf Town and the Sinnoh Pokemon League;
- byte-exact imported blockdata and borders except documented Porymap repairs;
- every connection in both directions and every warp's entry and return;
- explicit Sinnoh region resolution on every selected map and correct region
  changes when entering and leaving;
- save/load and invalid-state handling without new persistent banks or leakage
  into existing regional state;
- safe Town Map, Fly, healing, blackout, Teleport, Dig, and Escape Rope behavior
  while no public Sinnoh services exist;
- Cartographer and Metatiles coverage for every selected layout and tileset
  pair;
- source isolation in standalone HNS, Emerald, FireRed, and LeafGreen builds;
- exact asset-reuse and raw/compressed round-trip gates from the asset spec;
  and
- a production-equivalent Wayfarer size report with `rom.used_bytes`,
  `__rom_end`, category deltas, and at least 512 KiB remaining below 32 MiB.

The E2E route must visit all 133 maps. It walks every connection both ways,
uses every reachable warp in both directions, saves and reloads in each map
class, and samples the largest layouts including Eterna Forest. Tests assert
that no NPC, item, Trainer, encounter, dialogue, service, or story scene appears
from the empty source catalog.

## References

- [Sinnoh asset reuse and ROM footprint](sinnoh-asset-reuse-and-rom-footprint.md)
- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Sevii exploration map port](sevii-exploration-map-port.md)
- [Compressed map layout build and storage](compressed-map-layout-build-storage.md)
- [Compressed map layout runtime loading](compressed-map-layout-runtime-loading.md)
- [Compressed map layout validation and rollout](compressed-map-layout-validation-rollout.md)
- [Badge-free HM field use](../prds/hm-field-use.md)
- [Map catalog](../../game/data/maps/map_groups.json)
- [Layout catalog](../../game/data/layouts/layouts.json)
- [Map generator](../../game/tools/mapjson/mapjson.cpp)
