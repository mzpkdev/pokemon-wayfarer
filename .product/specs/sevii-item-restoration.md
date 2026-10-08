# Sevii item restoration

PRD: [Daily world slots](../prds/daily-world-slots.md) (prerequisite) and
[Sevii exploration port](../prds/sevii-exploration-port.md)
Implemented: No

## Scope

Restore Sevii's 97 FireRed and LeafGreen overworld items to the Wayfarer
build: 39 item balls and 58 hidden items. Each one is restored exactly as
authored in its FRLG source map, in its original place. It works on its own,
before [daily world slots](daily-world-slots.md) lands, and then
[World items](world-items.md#sevii) decides which spots are prizes and which
are dynamic.

Out of scope, and still dropped by the port: 85 signs, 57 Rock Smash rocks,
16 Cut trees, 26 Strength boulders, 24 coord triggers, the Union Room,
Mystery Gift and Seagallop attendants, and decorative objects. The Town Map and
Tri-Pass first-trip exclusion stays.

## Why they are missing

The Sevii port keeps only the events its manifest lists. The mapjson Sevii
sanitizer (`game/tools/mapjson/mapjson.cpp`) emits a map's `retained_events`
and skips everything else. The 97 items aren't retained. They are recorded as
two manifest exclusions in `game/src/data/wayfarer_sevii_maps.json`:
`exclusion.story.static-visible-pickups` (39) and
`exclusion.story.static-hidden-pickups` (58).

## Behavior

### Manifest

- Remove both exclusions.
- Add one `retained_events` row per item, owned by the exploration domain,
  with a matching `content_id` in the exploration inventory
  (`exploration.<map>.object.N` or `exploration.<map>.bg.N`). The row's source
  is an exact copy of the FRLG event, as the schema requires.

### Flags

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

### Scripts

The 39 FRLG item-ball scripts compile in Wayfarer, but the Sevii audit only
accepts `WayfarerSevii_*` labels. Add a wrapper per item ball in a new
exploration script module, `data/scripts/wayfarer_sevii/items.inc`, each
running `finditem <item>` and ending. Allow `finditem` in that module and
export the labels. Hidden items use the shared hidden-item script and need no
wrapper.

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
- Each pickup persists across a save and reload, and no two items share a flag.
- The Itemfinder finds every Sevii hidden item and nothing after pickup.
- `make check` passes, including `wayfarer-sevii-content-audit` and
  `wayfarer-sevii-port-audit`.
