# Sinnoh geography catalog

PRD: [Sinnoh geography foundation](../prds/sinnoh-exploration-port.md)
Implemented: Yes

## Scope

This specification owns the checked-in Sinnoh map/layout catalog, stable identities,
effective topology, source classification, and offline audit. It does not own a playable
entry route or regional gameplay.

## Catalog contract

`game/src/data/wayfarer_sinnoh_maps.json` records the pinned donor, nine source groups,
133 maps/layouts, 233 warps, 114 connections, stable target map/layout IDs, and
empty-content facts.

There is no release-selection gate: the full source catalog is compiled, and the absence
of a player entry route is the reachability boundary. Map generation requires complete
counts, unique IDs, expected group order, Emerald layout format, and zero authored
events/scripts/encounters.

All catalog maps are compiled in Wayfarer and excluded from standalone products. They
remain unreachable during ordinary play until a later feature adds a production entry.

## Topology

Checked-in map JSON is the effective runtime topology. The compact repair list documents
the 34 donor edges that required correction. It contains only source edge, replacement,
and reason; it does not duplicate complete before/after/return fixtures.

Generation verifies destination IDs and frozen aggregate topology. A future playable
slice must separately validate collision, elevations, seams, warps, and guaranteed
return paths for the maps it exposes.

## Runtime boundary

Maps retain explicit Sinnoh provenance and `REGION_SINNOH`. No saved Sinnoh current/
visited flags, test traversal helper, or Town Map/Fly UI branches are present because no
player can enter the region.

When a production entry is approved, add only the persistence and recovery behavior
required by that route. Do not infer a complete regional system from source names or
map-section ranges.

## Tooling

Normal generation reads checked-in source without network or donor replay.
`make wayfarer-sinnoh-port-audit` runs compact structural tests and the local catalog
audit. A maintainer may pass the pinned donor checkout directly to the audit when
reviewing a deliberate refresh.

The removed importer/freeze pipeline is not part of routine development. Updating the
donor baseline is an explicit migration that may introduce a purpose-built one-time
import script if needed.

## Validation

- Compact tooling unit tests pass.
- Mapjson catalog tests cover counts, identity, topology, and empty content.
- The default hybrid Wayfarer image links all generated map/layout data within the ROM
  limit. Raw mode remains a generator and runtime mechanics-test baseline.
- Standalone builds link without Sinnoh selection.

Full-map emulator traversal, Sinnoh save/reload, regional UI, healing, blackout, and Fly
tests are deferred with the player-facing feature that would make them meaningful.
