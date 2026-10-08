# Sevii exploration restoration

PRD: [Daily world slots](../prds/daily-world-slots.md) (prerequisite) and
[Sevii exploration port](../prds/sevii-exploration-port.md)
Implemented: No

## Scope

The [Sevii exploration port](../prds/sevii-exploration-port.md) says to keep
breakable rocks, boulders, field-move terrain and fixed signs, but the build
only kept the events its manifest lists one by one. This spec restores what
the port meant to keep, exactly as authored in the FRLG source maps:

| Content | Count | Notes |
| --- | ---: | --- |
| Item balls | 39 | [Items](#items) |
| Hidden items | 58 | [Items](#items) |
| Rock Smash rocks | 58 | Kindle Road, Mt. Ember and its Ruby Path, Four Island, Sevault Canyon, Ember Spa |
| Cut trees | 16 | Berry Forest, Two, Three and Five Island, Bond Bridge |
| Strength boulders | 26 | Mt. Ember, its summit and Ruby Path, Ruin Valley, Sevault Canyon. Tanoby Key's 7 are already kept |
| Signs | 85 | Town, route and landmark signs and Tanoby's Braille texts; 19 are already kept |
| Ember Spa heal | 1 | The coord trigger that heals the party in the hot spring |
| Lorelei's dolls | 5 | Seel, Pikachu, Slowpoke, Jigglypuff and Lapras dolls in Lorelei's house |

It works on its own, before [daily world slots](daily-world-slots.md) lands.
[World items](world-items.md#sevii) then decides which item spots are prizes
and which are dynamic.

Still out of scope:

- 24 story coord triggers (the Three Island bikers, the Mt. Ember password,
  Lorelei's poacher scene, the Warehouse admin, and Bill's One Island
  departure check). The [Sevii story port](../prds/sevii-independent-story-beats.md)
  owns those scenes with its own scripts.
- The Union Room, Mystery Gift and Seagallop attendants, which are link
  features Wayfarer doesn't have or ferry logic owned elsewhere.
- Lorelei herself, who belongs to [notable trainers](../prds/notable-trainers.md).
- The Town Map and Tri-Pass first-trip exclusion.

## Why they are missing

The Sevii port keeps only the events its manifest lists. The mapjson Sevii
sanitizer (`game/tools/mapjson/mapjson.cpp`) emits a map's `retained_events`
and skips everything else. The items are recorded as two manifest exclusions in
`game/src/data/wayfarer_sevii_maps.json`:
`exclusion.story.static-visible-pickups` (39) and
`exclusion.story.static-hidden-pickups` (58). The terrain, signs, spa trigger
and dolls were simply never listed.

## Behavior

### Items

#### Manifest

- Remove both item exclusions.
- Add one `retained_events` row per item, owned by the exploration domain,
  with a matching `content_id` in the exploration inventory
  (`exploration.<map>.object.N` or `exploration.<map>.bg.N`). The row's source
  is an exact copy of the FRLG event, as the schema requires.

#### Flags

The FRLG flag constants can't be used. In Wayfarer they resolve to HNS stubs:
every `FLAG_HIDE_*` item flag is 0, which never persists a pickup, and every
`FLAG_HIDDEN_ITEM_*` aliases one shared flag.

- **Item balls (39):** each row overrides its object flag with a new Sevii bank
  flag, `FLAG_WAYFARER_SEVII_ITEM_<MAP>_<ITEM>`, in free Sevii slots (2–9 and
  61 upward are free; the bank has 256 slots and is already sized in
  SaveBlock3). Picking the item up runs `removeobject`, which sets the flag.
- **Hidden items (58):** a hidden item event stores its flag as a 13-bit offset
  from `FLAG_HIDDEN_ITEMS_START`, so a Sevii bank flag doesn't fit. Add a
  Sevii hidden-item marker, following Hoenn's
  `WAYFARER_HOENN_HIDDEN_ITEM_MARKER`: the encoded value carries the marker
  plus a Sevii bank slot, `GetHiddenItemFlagId` decodes it to the Sevii flag,
  and mapjson emits it for Sevii rows. Each hidden item gets
  `FLAG_WAYFARER_SEVII_HIDDEN_<MAP>_<N>`. The Itemfinder must see them.

Allocate the 97 slots together, record them where the Sevii state contracts
expect, and check that none overlaps another Sevii or Wayfarer flag.

#### Scripts

The 39 FRLG item-ball scripts compile in Wayfarer, but the Sevii audit only
accepts `WayfarerSevii_*` labels. Add a wrapper per item ball in a new
exploration script module, `data/scripts/wayfarer_sevii/items.inc`, each
running `finditem <item>` and ending. Allow `finditem` in that module and
export the labels. Hidden items use the shared hidden-item script and need no
wrapper.

### Field-move terrain

Retain every Rock Smash rock, Cut tree and Strength boulder as an exploration
row with its FRLG position, graphics and movement. They use the shared
`EventScript_RockSmash`, `EventScript_CutTree` and `EventScript_StrengthBoulder`
helpers; add the first two to the approved exploration helpers beside the
Strength one. Rocks and trees keep FRLG's behavior of coming back when the map
reloads, and need no flags.

Field use follows Wayfarer's [badge-free HM rules](../prds/hm-field-use.md).
Smashing a rock can start a wild encounter from the map's Rock Smash table,
which makes Sevii's Rock Smash tables reachable. Ferry access to every island
hub still needs no field move, as the port requires.

### Signs

Retain the 85 signs as exploration bg rows. Each needs a `WayfarerSevii_*`
wrapper that shows the original text, in a new `signs.inc` exploration script
module that allows only message commands. Braille signs keep their Braille
text. A sign whose text points to story that the Wayfarer story port changed is
reworded to stay accurate, or left out with a reason.

### Ember Spa

Retain the spa's coord trigger with a `WayfarerSevii_*` wrapper that heals the
party as FRLG does, in the exploration module, which allows the heal command.

### Lorelei's dolls

Retain the five dolls in Lorelei's house with a shared `WayfarerSevii_*`
wrapper showing FRLG's doll text. They need no flags.

### Pins and audits

- The exploration projection digest changes. Refresh
  `baseline.projection_sha256` in the manifest and
  `manifest_baseline.projection_sha256` in
  `game/tools/wayfarer_sevii_content/baseline.json` together, from the audit.
  The map, layout and wild-encounter digests don't change.
- Extend the hidden-item tests in `game/tools/mapjson/tests/` for the Sevii
  marker, and the Sevii content, port and script generator tests for the new
  rows and module.

## Validation

- All 97 items appear in Wayfarer's generated events at their FRLG
  coordinates, with the authored item and quantity.
- Every restored rock, tree, boulder, sign, the spa trigger and the dolls
  appear at their FRLG coordinates. Mt. Ember's Ruby Path and Ruin Valley can be
  completed with field moves, and nowhere strands the player.
- A Rock Smash encounter triggers on a Sevii map with a Rock Smash table.
- Each pickup persists across a save and reload, and no two items share a flag.
- The Itemfinder finds every Sevii hidden item and nothing after pickup.
- `make check` passes, including `wayfarer-sevii-content-audit` and
  `wayfarer-sevii-port-audit`.
