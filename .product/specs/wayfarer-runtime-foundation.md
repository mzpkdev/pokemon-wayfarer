# Wayfarer runtime foundation

PRD: [Wayfarer Hoenn integration](../prds/wayfarer-hoenn-integration.md)
Implemented: No

## Scope

This specification defines the build, map catalog, persistent-state model, and
ROM budget for the Wayfarer product. It keeps HNS as the engine while making
the HNS and Emerald content sets available in one ROM.

It does not define the Hoenn content manifest, the adapted Hoenn introduction,
or player-facing regional travel. Those behaviors belong to the Wayfarer Hoenn
content port and regional travel specifications.

## Behavior

### Build identity

Wayfarer is a first-class build named `wayfarer`. Its player-facing title and
save identity use "Wayfarer". Internal conditionals continue to select HNS
engine behavior for battle, field moves, encounters, and shared systems.

Engine selection and content selection are separate concerns:

- HNS engine behavior remains selected for Wayfarer.
- HNS map content remains included.
- Emerald map content is included alongside HNS map content.
- HNS and Hoenn native utility learnset rows are both included.
- Standalone Emerald, FireRed, LeafGreen, and HNS builds keep their current
  content and behavior.
- Adding Emerald content to Wayfarer does not make standalone HNS include that
  content.

Code that chooses an engine rule may continue to branch on the HNS engine.
Code that chooses regional maps, tilesets, scripts, encounters, or other
content must use a content-availability rule that can include more than one
source product.

### Composite map and layout catalog

The Wayfarer map generator accepts both `hns` and `emerald` maps and layouts.
Each included layout retains its declared layout version so the existing mixed
layout renderer selects the correct dimensions, palettes, tiles, and
metatiles.

The generated catalog follows these rules:

- Every map included by the standalone HNS build keeps the same map group and
  map number in Wayfarer.
- Hoenn maps receive stable, noncolliding group and map numbers from the union
  catalog.
- Every included map header references a present layout, events table, scripts
  table, connections table, and tileset pair.
- Every warp and map connection resolves to a present Wayfarer map.
- Hoenn and HNS heal locations referenced by included maps remain in the
  generated heal-location catalog.
- No map or map group number may exceed the range accepted by the existing
  warp representation. The build fails instead of truncating an identifier.
- Later content additions append identifiers or occupy explicitly reserved
  space. They do not reorder existing HNS or Hoenn identifiers.

Wayfarer includes both HNS and Emerald tileset headers, graphics, metatiles,
palettes, animations, and door data when referenced. Mutually exclusive source
build conditionals must not suppress one region's tileset metadata in the
union build.

### Trainer ID capacity

Wayfarer separates the number of populated ordinary Trainer records from the
reserved boundary used by partner Trainer IDs:

- `TRAINERS_COUNT_WAYFARER` is one greater than the highest populated ordinary
  HNS or Hoenn Trainer ID.
- `MAX_TRAINERS_COUNT_WAYFARER` is 2,048.
- Every ordinary HNS and Hoenn Trainer ID is less than
  `MAX_TRAINERS_COUNT_WAYFARER`.
- Partner Trainer IDs begin at `MAX_TRAINERS_COUNT_WAYFARER` and occupy the
  following partner-only range.
- The populated Trainer table is sized by `TRAINERS_COUNT_WAYFARER`, not by the
  reserved partner boundary.
- Trainer-defeat storage is sized for populated ordinary Trainers and does not
  reserve one event flag for every value below the partner boundary.

All Trainer IDs, partner-ID calculations, script operands, battle setup paths,
and Trainer table lookups must represent the Wayfarer ranges without
truncation. Standalone builds retain their existing counts and partner
boundaries.

### Persistent namespaces

Existing HNS persistent identifiers keep their current meanings. Hoenn content
does not reuse an HNS flag, variable, badge, Trainer-defeat bit, visited bit, or
Champion state merely because Emerald used the same numeric value.

Wayfarer provides a dedicated Hoenn persistent bank in saved data. It contains:

- a full Hoenn script-variable bank;
- Hoenn story, NPC visibility, item, gift, daily, and system flags;
- eight Hoenn badge bits;
- Hoenn visited-location bits;
- Hoenn Trainer-defeat state;
- Hoenn initialization and Champion state; and
- any Hoenn-local counters required by the included main campaign.

Hoenn persistent constants must be nonzero, stable, and distinguishable from
HNS constants. The standard flag and variable accessors route Hoenn constants
to the Hoenn bank. Scripts do not need to know which save structure holds the
value.

The following behaviors are forbidden:

- defining a reachable Hoenn persistent flag as zero;
- aliasing multiple unrelated Hoenn items, NPCs, or story events to one bit;
- indexing directly into HNS flag or variable arrays with a Hoenn identifier;
- deriving a Hoenn badge by adding an offset to the sixteen HNS badges; and
- deriving Trainer-defeat flags from a global Trainer ID when that calculation
  can enter another persistent range.

Region-aware helpers expose badge count, Champion state, game-clear state,
visited state, and Trainer-defeat state. Hoenn scripts use the Hoenn result.
Existing Johto and Kanto callers keep their current result.

### Save storage and lifecycle

The additional Hoenn banks live in `SaveBlock3` or an equivalent separately
bounded saved structure. The implementation must not enlarge `SaveBlock1`
past its safe allocation or shift existing HNS fields merely to create Hoenn
space.

The complete saved structure must fit its existing save-sector allocation.
`SaveBlock3` must remain no larger than 1,624 bytes. Compile-time assertions
must cover the saved structure, flag bank, variable bank, Trainer bitset, and
all region-indexed arrays.

A new game clears all Wayfarer regional state before running any regional
initializer. The first Hoenn arrival may apply the Hoenn baseline defined by
the travel specification, but it must not clear HNS progress. Starting a new
game after an existing save must not retain a Hoenn item, badge, visited bit,
Trainer result, or story value.

Saving and loading must round-trip every new field. Corrupt, absent, or
out-of-range region values fall back to the region implied by the saved map,
then to Johto if the saved map cannot be resolved.

Prerelease save compatibility is not required. Introducing the Wayfarer save
layout increments the save version and initializes the new fields as a unit.
Once a public Wayfarer save format exists, later changes require an explicit
migration or a deliberate compatibility break.

### Regional state isolation

Johto, Kanto, and Hoenn have separate badge and Champion meanings. Completing
or resetting one region's League cannot alter another region's badges,
Champion state, Trainers, items, NPCs, or campaign values.

Global engine state may record that some League has been completed only where
an engine feature truly needs that fact. Hoenn scripts that reveal postgame
content must check Hoenn Champion or Hoenn game-clear state, not a global flag
that Johto or Kanto can set.

Whiteout, Hall of Fame, daily reset, and new-game code must dispatch regional
story cleanup to the applicable region. A whiteout in Johto cannot relocate a
Hoenn NPC. Entering the Hoenn Hall of Fame cannot run the Johto League reset.

### ROM budget

The final ROM must fit the standard 32 MiB GBA address space. Wayfarer must not
depend on a 64 MiB mapping, emulator-side storage, bank switching, or a custom
cartridge mapper.

Every release build produces a size report containing at least:

- total used and unused ROM bytes;
- the final `__rom_end` address;
- code, scripts, maps and layouts, graphics, audio, Trainer data, and encounter
  data totals; and
- change in each category from the previous accepted Wayfarer baseline.

During Hoenn development, the release build fails when `__rom_end` is greater
than `0x09F80000`. This preserves at least 512 KiB below the 32 MiB end address
at `0x0A000000`. A 1 MiB reserve is preferred until the required Hoenn campaign
is complete, but the enforceable minimum is 512 KiB.

Space recovery follows this order unless a separate product decision approves
a content cut:

1. Remove empty generated data and release-only debugging data.
2. Alias exact duplicate assets.
3. Compress data transparently without changing its runtime result.
4. Reassess optional systems outside the required Hoenn milestone.

Required maps, NPCs, encounters, Trainers, Gyms, or main-story content cannot
be removed solely to satisfy the reserve.

### Runtime memory

The combined build must remain within GBA EWRAM, IWRAM, stack, heap, save-sector,
and decompression-buffer limits. The Hoenn state bank must add less than 1 KiB
of persistent runtime memory unless a later audited design changes this bound.

Map and tileset loading should continue to decompress into the existing
destination buffers. A union catalog must not require HNS and Hoenn map assets
to be decompressed simultaneously.

### Validation

Static and automated checks must prove all of the following:

1. Wayfarer selects the HNS engine and includes both HNS and Emerald map sets.
2. Standalone product builds keep their previous map and content selection.
3. Every Wayfarer map, layout, tileset, warp, connection, and heal location
   resolves.
4. Existing HNS map identifiers and persistent constants are unchanged.
5. Every ordinary Trainer ID is below 2,048, every partner Trainer ID is at or
   above 2,048, and both ranges resolve through battle setup without collision.
6. Every reachable Hoenn persistent constant is nonzero, in bounds, unique
   where required, and disjoint from HNS state.
7. Badge, Champion, game-clear, Trainer-defeat, and visited-state helpers return
   the requested region's value.
8. New game, save, reload, and save replacement initialize and preserve the
   Hoenn bank correctly.
9. Compile-time size assertions pass for saved and runtime structures.
10. The release build stays at or below `0x09F80000` and reports each required
   size category.
11. Representative map loads and transitions run without heap corruption or a
    second simultaneous map decompression buffer.

## References

- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [Wayfarer regional travel and Hoenn entry](wayfarer-regional-travel-and-hoenn-entry.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
