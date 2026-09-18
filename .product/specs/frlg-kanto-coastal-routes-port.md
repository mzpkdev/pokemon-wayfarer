# FRLG Kanto coastal routes port

PRD: [FRLG Cinnabar, Seafoam Islands, and coastal routes port](../prds/frlg-cinnabar-seafoam-port.md)
Implemented: No

The current `CoastPoc` maps prove navigation with empty object and encounter
events. This specification replaces that preview with the complete FRLG Route
19, Route 20, Route 21 North, and Route 21 South content in Wayfarer. The
[coastal integration specification](frlg-cinnabar-seafoam-integration.md)
owns links to Cinnabar, Seafoam, Fuchsia, and Pallet, shared state allocation,
and the release budget.

## Selected maps and traversal

Select the four FRLG route layouts, tilesets, music, events, and scripts for
Wayfarer under `MAP_ROUTE19`, `MAP_ROUTE20`, `MAP_ROUTE21_NORTH`, and
`MAP_ROUTE21_SOUTH`. Wayfarer-specific variants may adapt source events and
layouts while retaining these final logical map IDs. Route 19 runs from
Fuchsia to Route 20. Route 20 runs from Route 19
to Cinnabar and has both Seafoam 1F entrances. Route 21 South and North form
one uninterrupted Cinnabar-to-Pallet route. Keep the source FRLG maps and
standalone HNS maps unchanged; a Wayfarer variant may adapt their identifiers
and events. The final Wayfarer catalog has one playable version of each route
section. It must not leave a second HNS or empty preview path that duplicates
Trainers, items, or encounters.

Fuchsia and Pallet remain Wayfarer's selected HNS towns. Inspect their
boundaries with the adjacent FRLG routes in Porymap. Fix shoreline tiles,
elevation, collision, Surf launch and landing points, and camera previews as
needed. Check the Route 19/Fuchsia, Route 19/20, Route 20/Cinnabar, Route 21
South/Cinnabar, Route 21 South/North, and Route 21 North/Pallet connections in
both directions, including saving and reloading near each seam. Mixed primary
or secondary tilesets must not corrupt the entering frame or its outgoing
preview. Preserve the approved native-Surf access from Pallet to Cinnabar
without a badge, HM item, or story victory.

Apply Wayfarer's existing native-Surf field-use rule to all selected coastal
water, including Routes 19 and 20; no route-specific badge or HM-item check
may be introduced. The Pallet-to-Cinnabar Route 21 crossing remains the
required settlement access path.

Remove the selected HNS Route 19 Kingler blockade and dialogue that treats
Blaine's defeat as the condition for opening the Fuchsia coast. No FRLG route
Trainer or NPC may close the only open-world route. Ordinary sight-based
Trainer battles remain possible. Route 20's Seafoam entrances stay usable
from both sides independently of Mansion, Gym, or Articuno progress.

Do not add a Route 19 warp to `Route19_UnusedHouse_Frlg`: its source map has
no entrance, objects, or reward. Retire the HNS Route 19 Cave entrance and
its Kyogre encounter from active Wayfarer content. Hoenn's Marine Cave remains
the later Kyogre capture site under the Hoenn story contract; the route port
must not create a second capture or leave a reachable orphan cave. Standalone
HNS retains its cave and encounter.
Retire the unconnected HNS Fuchsia Route 19 Gate from the final Wayfarer
catalog; its old exit still names `MAP_ROUTE19_HNS`. The selected Fuchsia
exterior connects directly to FRLG Route 19.

## Objects, Trainers, and items

Carry the FRLG route object and background-event content, adapting source
references to the selected Wayfarer maps. The source roster contains twelve
Trainer objects on Route 19, ten on Route 20, five on Route 21 North, and
five on Route 21 South. Route 19's Lia/Luc and Route 21 North's Lil/Ian are
paired double battles: either sibling starts the pair's shared battle and
defeat identity, including its authored two-Pokémon party requirement and
rematch path. Do not register each sibling as a separate victory or reward.
Retain the other Trainers' authored party identities, approach
geometry, sight ranges, intro and post-battle dialogue, and one-time defeat
state. Map their parties through Wayfarer's Trainer Rating scaling and trainer
defeat systems. Preserve source rematches where supported by Wayfarer's
rematch rules; do not create a second battle identity for an HNS counterpart.

Retain Route 20's Camper and two Seafoam signs, Route 19's route sign, Route
20's hidden Stardust, and Route 21 North's hidden Pearl. Each hidden item has
one collision-safe placement and a persistent one-time receipt. Adapt Route
21 North's off-map Pallet NPC clone to the selected HNS Pallet actor if that
actor can safely be shared; otherwise render one equivalent actor at the
boundary without duplicating an interaction or leaving an invalid FRLG Pallet
reference. Preserve any other source object needed for the authored route
presentation. Remove lines that refer to a Seafoam Gym, Cinnabar eruption,
forced Bill trip, or another event absent from the selected coast.

Route 20 runs FRLG's Seafoam boulder reset on every map transition into the
route, including from either Seafoam entrance, Cinnabar, or Route 19. Restart
only unfinished B3F and B4F boulder chains and leave each stopped current
solved. Compose this with Wayfarer's route time-of-day wild setup. The
[Seafoam specification](frlg-seafoam-islands-port.md) owns the boulder and
current behavior inside the cave.

Inventory every selected object flag, hidden-item flag, Trainer ID, rematch
entry, map script, and referenced text before enabling the content. Allocate
Wayfarer state where raw FRLG IDs are zero, collide with HNS/Emerald, or denote
a different event. Preserve the same encounter or reward identity only when
it truly represents the same one-time result. Standalone FRLG and HNS retain
their original flags and scripts.

Audit non-map consumers of the retired HNS route IDs. In particular, rematch
entries must not target an unreachable HNS Route 20, and Kanto roamer
adjacency must use the selected FRLG Route 19, Route 20, and both Route 21
sections without sending roamers to retired maps. Update encounter-generator
retirement lists, native-HM coverage fixtures, and battle-environment records
that still name the old route IDs. Preserve standalone HNS behavior.

## Ordinary wild encounters

Use `game/src/data/wild_encounters.json` as the authored source. Register each
selected route map in the Kanto encounter manifest with separate map identity
and complete day/night source binding. Use `sRoute19_{FireRed,LeafGreen}` and
`sRoute20_{FireRed,LeafGreen}` for Surf and fishing, and the matching
`sRoute21{North,South}_{FireRed,LeafGreen}` sources for land, Surf, and
fishing. If the two Route 21 source profiles are identical for a method,
record and validate that equivalence; retain both playable map bindings.

Apply the existing Kanto FireRed/LeafGreen species-merge, level provenance,
night-authoring, Standard Rod, and Trainer Rating rules. Preserve every
authored method on its playable terrain and do not silently carry an HNS route
encounter header into the FRLG layouts. The retired HNS route profiles remain
available to standalone HNS. No encounter header in the Wayfarer release
points to a route that is absent or unreachable.

## Acceptance

1. Generate the Wayfarer catalog and Kanto encounter audit with four selected
   route map identities, 32 ordinary route Trainer objects, source-equivalent
   non-Trainer objects, and no duplicate active HNS route content. Route 19
   Cave and the disconnected FRLG unused house are not Wayfarer destinations;
   Hoenn Marine Cave's Kyogre encounter remains reachable through its owner.
   Standalone FRLG and HNS catalogs keep their own routes.
2. From Fuchsia, travel Route 19 → Route 20 → Cinnabar and back. From Pallet,
   travel Route 21 North → South → Cinnabar and back. Cross every seam while
   Surfing and check camera preview, graphics, music, collision, native-Surf
   launch and landing, save/reload, and Fly or whiteout recovery at Cinnabar.
3. Challenge every route Trainer by sight and by direct talk where possible.
   Trigger both paired double battles from either sibling, with enough and
   too few usable Pokémon. Verify scaling, shared defeat persistence,
   after-battle dialogue, supported rematches, and no HNS counterpart or
   Kingler blockade on the selected path.
   Talk to the Camper and read the signs; collect Stardust and Pearl once,
   including after travel and save/reload.
4. Verify land, Surf, and each rod method on the maps that author them, in day
   and night conditions and at representative Trainer Ratings. Confirm the
   FRLG version species merge and encounter provenance audit still pass.
5. Enter and leave Seafoam through both Route 20 doors. Unfinished boulder
   chains reset on Route 20; stopped currents remain solved. Neither door nor
   the Fuchsia and Pallet crossings depends on Blaine or another story win.
6. Build the production Wayfarer ROM and measure the linked size against the
   unchanged reserve, alongside the full coastal port. No POC-only map,
   orphaned HNS route payload, or duplicate route event is part of the final
   release.

## References

- [FRLG Route 19](../../game/data/maps/Route19_Frlg/map.json)
- [FRLG Route 20](../../game/data/maps/Route20_Frlg/map.json)
- [FRLG Route 21 North](../../game/data/maps/Route21_North_Frlg/map.json)
- [FRLG Route 21 South](../../game/data/maps/Route21_South_Frlg/map.json)
- [Kanto wild encounters](kanto-wild-encounters.md)
- [Coastal and state integration](frlg-cinnabar-seafoam-integration.md)
