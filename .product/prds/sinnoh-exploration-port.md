# Sinnoh geography foundation

## Intent

Preserve the available Sinnoh geography in Wayfarer as checked-in source material for a
future player-facing region. This milestone imports and compiles the donor catalog; it
does not pretend that an empty map scaffold is a playable Sinnoh product.

The donor baseline is
[`LiderMorti00/Sinnoh-pokeemerald-expansion`](https://github.com/LiderMorti00/Sinnoh-pokeemerald-expansion)
commit `4eed17cc63c4ec8c24fbb20fe49e8d65cb4870d8`.

## Delivered catalog

The Wayfarer-only catalog contains 133 maps and 133 layouts across nine Sinnoh groups,
with 233 warps and 114 connections. It contains no object, coordinate, or background
events, no authored map scripts, and no wild-encounter profiles.

All imported maps, layouts, blockdata, borders, tilesets, and effective topology repairs
remain checked in. Existing map/layout IDs are not renumbered. Standalone HNS, Emerald,
FireRed, and LeafGreen builds do not select Sinnoh content.

## Current product boundary

Sinnoh has no production entry route, guaranteed return, encounters, services, story,
regional map, Fly page, progression, or League role. Therefore this milestone does not
add:

- a test-only warp/return runtime;
- saved current/visited Sinnoh region state;
- Town Map, Fly, item, or party-menu guards for an unreachable region; or
- emulator traversal requirements for all 133 empty maps.

`REGION_SINNOH` and source classification remain because the checked-in maps carry that
identity. Player-facing runtime state is deferred until a separate product proposal
defines arrival, return, healing/blackout behavior, and progression.

Sinnoh remains unavailable in player-facing gameplay. The Wayfarer Devtools
Cartographer and Metatiles browsers may inspect its checked-in maps, layouts, and
tilesets without adding runtime reachability or saved region state.

## Authority and maintenance

One compact map manifest fixes donor identity, catalog counts/order, stable target IDs,
effective topology, and empty-content facts. Checked-in layout and tileset references are
the asset authority; there is no duplicate asset inventory or release-selection gate.

Normal builds trust these checked-in outputs. They do not replay the donor import,
rehash every asset, or run a provenance pipeline. An explicit offline maintainer audit
may compare a supplied donor checkout when the catalog is intentionally refreshed.

The 34 effective topology repairs remain represented once. Duplicate input/output/return
fixtures and repeated per-map selection prose are not product requirements.

## Storage

Sinnoh layouts participate in the common Wayfarer map-layout storage system. Hybrid may
compress any profitable layout automatically; Sinnoh does not maintain a separate
compression policy or exception list.

The complete Wayfarer release must remain inside the existing ROM and reserve limits.
Content-preserving aliasing and compression are preferred to removing imported geography.

## Acceptance

- Compact manifest/tooling tests pass.
- Map generation validates catalog counts, stable IDs, topology totals, and empty content.
- The default hybrid Wayfarer build links the catalog within the 32 MiB ROM limit; raw
  mode remains available for generator and runtime mechanics tests.
- Existing regions and standalone builds remain unaffected.
- The optional maintainer audit succeeds when provided the pinned donor checkout.

These checks establish a reliable source foundation. They do not approve Sinnoh as a
playable region.

## Future product decisions

A later proposal must choose a playable slice and define its entry/return route, events,
encounters, services, regional UI, persistence, recovery behavior, and validation. Only
then should runtime region state or map-specific safety branches be introduced.

## References

- [Sinnoh geography catalog](../specs/sinnoh-exploration-map-port.md)
- [Sinnoh asset authority and footprint](../specs/sinnoh-asset-reuse-and-rom-footprint.md)
- [Compressed map layouts](compressed-map-layouts.md)
