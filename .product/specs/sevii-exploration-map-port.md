# Sevii exploration map port

PRD: [Sevii exploration port](../prds/sevii-exploration-port.md)
Implemented: No

## Scope

This specification defines how the FRLG Sevii map catalog becomes selectable
in Wayfarer without changing the standalone FRLG maps. It covers map and layout
selection, event sanitization, ferry integration, retained services,
environmental scripts, completeness validation, and the implementation order.

The separate [Sevii wild encounter specification](sevii-wild-encounters.md)
owns encounter source selection and time-of-day binding. Story, Trainers,
static encounters, and rewards remain outside the new numbered-island maps.
Birth Island and Navel Rock are preexisting Wayfarer content. This port changes
only their ferry connection and preserves everything inside those features.

## Behavior

### Frozen source catalog

The port adds 135 registered FRLG maps backed by 102 unique layouts. The
current source layout and border files total 134,612 bytes before alignment,
code, tables, tilesets, compression, and linker garbage collection. That number
is planning evidence, not a promised ROM increase.

The source catalog is selected by this exact rule:

1. The map appears in `game/data/maps/map_groups.json`.
2. Its map directory begins with `OneIsland_`, `TwoIsland_`, `ThreeIsland_`,
   `FourIsland_`, `FiveIsland_`, `SixIsland_`, `SevenIsland_`, `MtEmber_`, or
   `TrainerTower_`.
3. Its `game_version` is `frlg`.
4. It is not an FRLG Birth Island or Navel Rock variant, or
   `SevenIsland_UnusedHouse`.

The implementation freezes the resolved list in
`game/src/data/wayfarer_sevii_maps.json`; it does not evaluate the prefix rule
at build time. Each record names the source map, layout, category, retained
event set, retained map-script table, encounter methods, and exclusion reason
when applicable. The manifest has a schema version and deterministic ordering.

| Source family | Maps | Included content |
| --- | ---: | --- |
| One Island | 9 | Hub, harbor, two houses, Pokemon Center, Kindle Road, Ember Spa, Treasure Beach |
| Two Island | 8 | Hub, harbor, house, Pokemon Center, Joyful Game Corner, Cape Brink and its house |
| Three Island | 14 | Hub, port, harbor, five houses, Mart, Pokemon Center, Bond Bridge, Berry Forest, Dunsparce area |
| Four Island | 13 | Hub, harbor, houses, Lorelei's house, Mart, Pokemon Center, Day Care, Icefall Cave |
| Five Island | 27 | Hub, harbor, houses, Pokemon Center, Meadow, Memorial Pillar, Water Labyrinth, Resort Gorgeous, Lost Cave, Rocket Warehouse |
| Six Island | 20 | Hub, harbor, house, Mart, Pokemon Center, Green Path, Water Path, Outcast Island, Ruin Valley, Pattern Bush, Altering Cave, Dotted Hole |
| Seven Island | 20 | Hub, harbor, house, Mart, Pokemon Center, Trainer Tower exterior, Sevault Canyon, Tanoby Key, Tanoby Ruins and chambers |
| Mt. Ember | 13 | Exterior, summit, summit paths, Ruby Path and stair maps |
| Trainer Tower interior | 11 | Lobby, elevator, floors 1 through 8, and roof |

The manifest generator fails when the resolved source set is no longer exactly
135 maps and 102 layouts, when a source map or layout disappears, or when an
unreviewed source map begins matching the rule.

### Map catalog adapter

Extend `game/tools/mapjson/mapjson.cpp` and `game/map_data_rules.mk` with an
explicit Wayfarer Sevii manifest input. In `wayfarer` mode only, the adapter:

- treats the manifest's selected FRLG maps and layouts as available alongside
  the existing HNS and Emerald catalogs;
- keeps the original FRLG map IDs, layout IDs, layout symbols, map groups, and
  raw layout paths so existing ferry, region-map, heal-location, and encounter
  references do not need parallel identities;
- emits manifest-selected Wayfarer events and map-script labels instead of the
  source map's full FRLG event table;
- validates every selected warp and connection against the combined Wayfarer
  catalog; and
- leaves nonselected FRLG maps as null catalog entries, as they are now.

Do not copy or edit the 102 `map.bin` files. Any later tile, elevation, or
collision change must be made in Porymap under the repository's FRLG layout
version guidance. The first port should need no layout changes.

The source adapter is allowlist based. It must not change the general meaning
of `game_version: "frlg"` or make all FRLG maps selectable in Wayfarer.
Standalone `firered`, `leafgreen`, `hns`, and `emerald` generation ignores the
Wayfarer manifest and produces byte-equivalent map catalogs.

The adapter rejects every Birth Island and Navel Rock record. Their HNS and
Emerald maps are already selected by Wayfarer and remain outside the manifest.
Their FRLG IDs, layouts, tilesets, events, and scripts must not be imported or
used as substitutes.

### Asset selection

Add a focused `HAS_SEVII_CONTENT` capability for the Wayfarer build rather
than changing `HAS_FRLG_CONTENT`. Use it to select only the primary and
secondary tilesets, door animations, object graphics, music, map sections,
and other data reachable from the 135 imported maps. Audit and update the
selective guards in:

- `game/include/constants/global.h`;
- `game/include/tilesets.h`;
- `game/src/tilesets.c`;
- `game/src/data/tilesets/graphics.h`;
- `game/src/data/tilesets/metatiles.h`;
- `game/src/data/tilesets/headers.h`;
- `game/include/constants/event_objects.h`; and
- `game/src/data/object_events/`.

Do not enable a whole FRLG asset table to satisfy one reference. The generated
audit computes the dependency closure from retained maps and events, then
fails on an unlisted FRLG asset or a missing selected asset. Measure the linked
ROM because the 134,612-byte raw map total does not include tilesets or object
graphics.

### Event and script policy

The manifest uses explicit kept-event lists. No event is retained because its
script name or Trainer type happens to pass a heuristic. Generation records the
source event identity and rejects source drift until the manifest is reviewed.

Apply these output rules:

| Event class | Wayfarer output |
| --- | --- |
| Warps and outdoor connections | Retain, except for an explicit destination repair needed to return to Wayfarer's HNS Vermilion port. |
| Sight and talk Trainers | Remove every object and every coordinate trigger. |
| Story actors and cutscenes | Remove objects, triggers, callbacks, scene tables, rewards, and story transitions. |
| Passive flavor NPCs | Omit from the first port. |
| Signs and fixed labels | Retain only when the script has no story read, story write, item delivery, battle, or movement scene. |
| Pokemon Center and PC service | Retain the ordinary service path with no Bill, Celio, rival, or quest dispatch. |
| Mart service | Retain the ordinary clerk and stock dispatch. |
| Four Island Day Care | Retain ordinary deposit and withdrawal only. Remove story dialogue and gifts. |
| Ferry sailors | Retain the shared all-island menu and a guaranteed HNS Vermilion return. |
| Environmental mechanics | Retain reviewed map-local mechanics needed for collision, currents, ice, holes, boulders, doors, and directional puzzles. |
| Items, gifts, trades, and static Pokemon | Remove from the first port. |

All retained scripts use common or Wayfarer-owned state. They may not read or
write FRLG campaign scenes, Champion state, National Pokedex state, Lostelle,
the Meteorite, Ruby, Sapphire, Rocket passwords, or Trainer defeat state.

Every numbered-island Pokemon Center 2F has source warps to the FRLG Trade
Center and Union Room, which are outside this port. Disable those two upstairs
warps and their doors in the Wayfarer overlay. Do not pull the generic FRLG
multiplayer maps into the allowlist.

### Event script linkage

The source FRLG map scripts are included only inside `.if IS_FRLG` in
`game/data/event_scripts.s`, so selecting their maps does not make their labels
available to Wayfarer. Do not widen that guard, because it would link the full
FRLG story and Trainer script graph.

Keep sanitized, Wayfarer-owned map scripts under
`game/data/scripts/wayfarer_sevii/`. Generate
`game/data/wayfarer_sevii_event_scripts.inc` from the manifest and
include that one file under `#if IS_WAYFARER` in
`game/data/event_scripts.s`. The generated file lists only the map-specific
scripts and common ferry, healing, Mart, Day Care, sign, and environmental
helpers named by the manifest. It contains no copied or filtered source script
bodies.

Add the manifest, the script generator, and every named Wayfarer script file
as Make dependencies of the generated include and `event_scripts.o`. A clean
build must regenerate the include when any of them changes and must reject a
stale checked-in artifact.

Prefer small Wayfarer service wrappers over including a complete FRLG helper
file. If a retained helper is shared directly, the audit must close over every
label it calls and prove that none can battle, grant an item or Pokemon, or
read and write excluded story state. A link test compares every event and
map-script label emitted by mapjson with the symbols in the Wayfarer script
artifact. Standalone event script inclusion remains byte-equivalent.

### Environmental adaptations

Keep the source geometry and make these story-independent adaptations:

- Open Mt. Ember and Ruby Path without Rocket actors or story flags. Retain
  ordinary Rock Smash and Strength terrain.
- Open the Three Island route to Bond Bridge and Berry Forest without the
  biker scene.
- Keep Icefall Cave's cracked-ice, falling-hole, and Waterfall behavior without
  Lorelei or the Rocket confrontation.
- Open Rocket Warehouse without passwords. Retain its floor arrows and allow
  the player to leave, but remove all people, battles, items, and story state.
- Keep Lost Cave's direction puzzle and provide a valid exit from every room.
- Make Dotted Hole enterable whenever the player can perform its retained Cut
  interaction. Keep its floor route and empty Sapphire room without the gem,
  scientist, or theft state.
- Keep Tanoby Key's Strength puzzle and use one Wayfarer-local completion flag
  for boulder persistence and chamber availability. Solving it grants no item
  and starts no story.
- Use Altering Cave's default FRLG state. Do not expose Mystery Gift rotations
  or alter `VAR_ALTERING_CAVE_WILD_SET` from island scripts.
- Make every Trainer Tower floor, roof, stair, and elevator destination
  reachable without starting a timed challenge. Retain no opponents or prizes.

Every environmental script must be retryable after interruption. Save and
reload preserves completed local puzzle state where FRLG normally persists it,
without consuming or advancing unrelated campaign state.

### Ferry and recovery

Add a Wayfarer-only top-level selector to
`VermilionCity_PortInside_hns` with fixed results `0 = SEVII ISLANDS`,
`1 = OTHER DESTINATIONS`, and `2 = CANCEL`. `MULTI_B_PRESSED` has the same
effect as `CANCEL`: close the message, release the player, and leave the player
on the walkable side of the sailor.

Result 0 enters a Wayfarer-owned dynamic Vermilion Sevii selector. Its first
entry is always `ONE ISLAND`, followed by each eligible special island, then
`CANCEL`. Store a destination enum beside each displayed entry so hidden
special entries cannot shift hard-coded result indices. `ONE ISLAND` starts
the ordinary Sevii service and warps to One Island harbor. This branch is
available whenever the player can stand at the dock, is evaluated before
`WayfarerCanUseRegularAqua`, and never calls `WayfarerPrepareHoennEntry`.

Result 1 preserves the existing `VermilionCity_PortInside_hns` service. It
calls `WayfarerCanUseRegularAqua` before opening the unchanged
`MULTI_VERMILION_HARBOR` list. Preserve that list's current Wayfarer mapping
exactly: `0 = Slateport`, `1 = Southern Island`, `2 = Birth Island`,
`3 = Faraway Island`, `4 = Battle Frontier`, and `5 = Cancel`. Preserve all
credential checks, boarding scripts, destination maps, and
`WayfarerPrepareHoennEntry` behavior in this branch. This prevents the Sevii
work from replacing or reindexing any existing non-Sevii Vermilion route.

Update the Seagallop destination data only under `IS_WAYFARER` so its
Vermilion destination is the HNS Vermilion port rather than the absent FRLG
Vermilion map. Use `MAP_VERMILION_CITY_PORT_INSIDE_HNS` at `(8, 9)`, the
existing safe arrival tile used by the HNS S.S. Aqua return paths. Each
numbered-island sailor opens the complete menu for One through Seven Island,
Vermilion, and Cancel from the first visit. The current island is omitted using
the existing menu behavior.

Keep Birth Island and Navel Rock as FRLG-style special routes rather than
adding them to the always-available numbered-island pages. Add read-only
helpers with these authoritative Wayfarer predicates:

- `WayfarerCanSailToBirthIsland` is true only when
  `WayfarerCanUseRegularAqua()` is true and the Bag contains
  `ITEM_AURORA_TICKET`. This preserves the effective parent gate and ticket
  check of the current HNS Vermilion route.
- `WayfarerCanSailToNavelRock` is true only when `FLAG_SYS_GAME_CLEAR` is set
  and the Bag contains `ITEM_MYSTIC_TICKET`. This preserves the effective
  League-clear and ticket ownership requirements of the Emerald ferry while
  making the requested Vermilion connection usable in Wayfarer. Do not read,
  allocate, set, or clear `FLAG_ENABLE_SHIP_NAVEL_ROCK`: it is the constant
  zero in the selected HNS flag catalog and cannot represent persistent
  Wayfarer state.

The dynamic selector includes Birth Island or Navel Rock only when its helper
returns true. The helpers perform no writes. In particular, this feature must
not write any `FLAG_ENABLE_SHIP_*`, `FLAG_SHOWN_*_TICKET`,
`FLAG_RECEIVED_*_TICKET`, ticket item, League, origin, Aqua, or story state.
Keep the current Lilycove and existing Vermilion routes unchanged.

Predicate tests cover the complete truth table. Birth is hidden for
`Aqua = false, Ticket = false`, `false, true`, and `true, false`, and visible
only for `true, true`. Navel Rock is hidden for
`GameClear = false, Ticket = false`, `false, true`, and `true, false`, and
visible only for `true, true`. Each case snapshots the relevant persistent
flags and Bag state before opening and canceling the selector and proves that
none changed.

The Wayfarer Seagallop destination table maps those special routes to the
existing canonical targets: `MAP_BIRTH_ISLAND_HARBOR_HNS` and
`MAP_NAVEL_ROCK_HARBOR`, both at their existing Seagallop landing coordinate
`(8, 5)`. Their harbor sailors return only to
`MAP_VERMILION_CITY_PORT_INSIDE_HNS` at `(8, 9)`, matching the FRLG
event-island topology. They do not open the numbered island destination pages.
Keep every other event-island script, object, encounter, battle, puzzle, item,
reward, and state transition exactly as it was before this port. Standalone
HNS and Emerald keep their current Lilycove return behavior.

Update the assertions in
`game/tools/wayfarer_hoenn_entry/generate.py` and its tests. The audit must
accept the new ungated Sevii branch before the Aqua gate while still proving
that the regular Hoenn route calls `WayfarerCanUseRegularAqua` and
`WayfarerPrepareHoennEntry` in its original order.

No pass, item, badge, League clear, Pokedex milestone, or story flag authorizes
the numbered-island menu. This rule does not relax the existing event-ticket
requirements for Birth Island or Navel Rock. Cancel leaves the player on the
walkable side of the sailor. A departure cannot change S.S. Aqua, S.S. Anne,
Hoenn entry, or regional campaign state.

Blackout from an island uses its Pokemon Center heal location after that hub is
visited. Before a heal location is registered, blackout returns to the last
valid existing heal location. No failure path may resolve to a null FRLG map
header.

### Region map and shared systems

Enable the numbered-island Sevii map pages in Wayfarer when the player first
selects the destination. Do not give a Town Map, pass, or event ticket.
Visiting a numbered island sets only its map visibility and heal-location
state. This milestone is ferry-only and does not allocate numbered-island Fly
flags or add Fly destinations. Existing Birth Island and Navel Rock map and
Fly behavior remains unchanged.

The seven numbered-island heal records in
`game/src/data/heal_locations.json` are currently emitted only when general
FRLG content is enabled. Extend the heal-location generator and
`game/src/data/heal_locations.json.txt` so those exact seven records are also
emitted for `IS_WAYFARER && HAS_SEVII_CONTENT`. Their respawn maps and healer
object IDs must resolve to the sanitized Pokemon Center 1F maps. No other FRLG
heal record may become live in Wayfarer.

Add Wayfarer-specific Sevii rows and classifications to `sMapHealLocations`
and the relevant region-map functions in `game/src/region_map.c`. Do not leave
them behind the current `#if !IS_HNS` guards. Numbered-island map sections must
resolve to the imported generic FRLG map IDs. Do not add, remove, or replace a
Birth Island or Navel Rock heal or region-map record.

Audit every map-ID and layout-ID consumer that currently special-cases FRLG,
including Seagallop travel, region-map sections, map previews, heal locations,
Pokedex area data, DexNav, Nuzlocke areas, Escape Rope, Dig, Fly, field moves,
door animations, tileset animation callbacks, and Altering Cave selection.
Make the smallest Wayfarer-only correction where a compile-time `IS_FRLG`
guard would otherwise suppress behavior needed by an included map.

The map adapter must also use the combined selected-map predicate for heal
location validation and source tracking. A selected FRLG Sevii map is valid in
Wayfarer even though general FRLG content remains disabled.

### Generated audit

Add `game/tools/wayfarer_sevii_port/` with a deterministic audit and unit tests.
Its report includes:

- the 135 selected FRLG maps, 102 layouts, source paths, and categories;
- proof that every Birth Island and Navel Rock map is excluded from the import
  and event-overlay manifests;
- unique layout and tileset dependencies plus raw source byte totals;
- every warp, connection, and reciprocal exterior/interior path;
- retained and removed object, coordinate, and background events;
- every retained script label and the state, battle, item, and special commands
  reachable from it;
- numbered-island heal, ferry, escape, and blackout destinations, plus proof
  that the port adds no numbered-island Fly destination;
- the two special-island ferry destinations, their authoritative read-only
  eligibility predicates, their exact `(8, 5)` arrival tiles, and their exact
  HNS Vermilion `(8, 9)` return target;
- the encounter methods owned by each selected map; and
- intentional exclusions with reasons.

Before the first implementation edit, commit
`game/tools/wayfarer_sevii_port/event_island_baseline.json`. It records the
SHA-256 and normalized map/event structure for every selected HNS Birth Island
and Emerald Navel Rock `map.json`, `scripts.inc`, encounter input, shared event
script, and referenced object or Trainer data file. Files with no approved
ferry edit must retain their exact hash. For each approved script hook, retain
protected-block hashes for every label outside the permitted block.

The only permitted event-island integration diffs are:

- the Wayfarer-only Vermilion selector and its read-only eligibility helpers;
- the Wayfarer destination rows in `game/src/seagallop.c`;
- a Wayfarer-only return target inside
  `BirthIsland_Harbor_hns_EventScript_Sailor`; and
- a Wayfarer-only return target inside `NavelRock_Harbor_EventScript_Sailor`.

The allowlist names files, labels, preprocessor scope, and permitted command
changes. A whole-file exception is forbidden. The audit compares the working
tree to the frozen baseline and fails if any protected hash or normalized
structure changes.

Generation fails on a missing target, null selected map header, event not named
by the manifest, Trainer event, story command, unapproved item or Pokemon grant,
unapproved battle, unavailable tileset, wrong layout version, or standalone
catalog change. It also fails if the port changes any Birth Island or Navel
Rock map, event, encounter, object, battle, puzzle, item, reward, or state
outside the two reviewed ferry connections.

### Delivery plan

Implement the port in bounded milestones. Commit and measure each milestone
before starting the next.

1. Add the frozen manifest, source-catalog adapter, mapjson tests, and a
   generated audit with no map enabled in the release link.
2. Enable the seven hubs, harbors, Pokemon Centers, Marts, houses, Day Care,
   and the Vermilion ferry integration. Connect the existing ticket-gated
   Birth Island route and League-clear, ticket-gated Navel Rock route without
   changing their content. Prove
   numbered-island entry, return, healing, and blackout behavior.
3. Enable outdoor routes and their direct connections, with story and Trainer
   events removed.
4. Enable Mt. Ember, Berry Forest, Icefall Cave, Pattern Bush, Altering Cave,
   Lost Cave, Dotted Hole, Tanoby Key and chambers, Rocket Warehouse, and the
   empty Trainer Tower. Add focused environmental tests.
5. Add the encounter profiles from the companion specification and validate
   day/night identity.
6. Run complete static, mechanics, release-size, and emulator acceptance. Fix
   only port-owned issues before marking either specification implemented.

After every content milestone, build a production-equivalent Wayfarer release
and compare its size report to the pre-port baseline. The final build must fit
32 MiB and retain the configured reserve. If it fails, stop before dropping
content and land a separately reviewed storage optimization first.

### Validation

Required automated validation:

- mapjson unit tests for selected and rejected FRLG maps, layouts, event
  overlays, missing targets, source drift, disabled Pokemon Center 2F
  link-room warps, and unchanged standalone catalogs;
- event-script generation and symbol-closure tests for every retained map
  label and helper dependency;
- `make -C game wayfarer-sevii-port-test` and
  `make -C game wayfarer-sevii-port-audit`;
- the companion encounter tests and audit;
- `make -C game check`;
- `make -C game wayfarer`;
- `make -C game wayfarer release`, with the generated size report checked
  against the active reserve; and
- a Wayfarer E2E ROM journey from Vermilion through all seven numbered islands
  and back, plus ticket-authorized round trips to the existing Birth Island and
  Navel Rock destinations.

The E2E journey must cover all 135 imported maps, enter and exit every
registered interior, traverse every outdoor connection, exercise each retained
environmental mechanic, save and reload on each numbered island, black out once
per heal hub, and prove that no imported Trainer or story scene starts. The
special-island checks exercise every row of both predicate truth tables, both
positive Vermilion departures, both `(8, 5)` harbor arrivals, both cancel paths,
and both `(8, 9)` HNS Vermilion returns. They also exercise every existing Birth
Island and Navel Rock story, Trainer, wild encounter, static battle, puzzle,
item, reward, save, heal, blackout, region-map, and Fly behavior reachable in
the baseline fixtures. The baseline audit, rather than a representative smoke
test alone, proves that unrelated content was not sanitized. Add a
Wayfarer-capable fixture to `e2e/` if the current suite cannot boot the required
build and state.

Do not run different map-version builds concurrently because their generated
map files share the source tree.

## References

- [Sevii wild encounters](sevii-wild-encounters.md)
- [FRLG traversal specification](frlg-open-world-region-traversal.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [Map groups](../../game/data/maps/map_groups.json)
- [Layouts](../../game/data/layouts/layouts.json)
- [Map generator](../../game/tools/mapjson/mapjson.cpp)
- [Seagallop destinations](../../game/src/seagallop.c)
