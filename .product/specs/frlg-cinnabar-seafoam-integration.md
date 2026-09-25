# Cinnabar and Seafoam coastal integration

PRD: [FRLG Cinnabar, Seafoam Islands, and coastal routes port](../prds/frlg-cinnabar-seafoam-port.md)
Implemented: Yes

Implemented in [PR #112](https://github.com/mzpkdev/pokemon-wayfarer/pull/112).

## Scope

Integrate FRLG Cinnabar, Seafoam, and Routes 19–21 with Wayfarer's selected
Kanto towns, state banks, travel services, encounters, Trainer systems, and
Viridian Blue. The town, cave, and
[route](frlg-kanto-coastal-routes-port.md) specifications own local gameplay.
This specification does not implement the unrelated Kanto story branch or
Sevii campaign.

## Behavior

### Map selection and coastal boundaries

Enable the thirteen Cinnabar maps, five Seafoam floor maps, and four FRLG
coastal route maps, plus Seafoam's two stopped-current layouts, for
`POKEMON_WAYFARER` only. Include their required FRLG primary and secondary
tilesets, object graphics, door effects,
music, scripts, layouts, events, and wild encounters. The selected catalog
must have unique map IDs and complete warp, connection, and heal references.
Remove the HNS island, HNS Center, HNS Seafoam 1F/B1F, Gym, Secret Cave, and
HNS Routes 19–21 and the HNS Route 19 Kyogre Cave from the final selected
Wayfarer catalog. The navigation-only
`CoastPoc` route maps must be replaced or filled with the complete FRLG route
events and encounter bindings; they cannot remain as a parallel playable path.
Do not change standalone HNS or FRLG selection.
The unconnected HNS Fuchsia Route 19 Gate still has an exit warp to
`MAP_ROUTE19_HNS`; retire that gate from Wayfarer's final catalog rather
than leaving a reference into the removed route. Fuchsia connects directly
to the selected FRLG Route 19 exterior.

Use the FRLG Route 19, Route 20, Route 21 North, and Route 21 South layouts
and full route content. Route 19 connects Fuchsia to Route 20; Route 20 connects
Route 19 to FRLG Cinnabar and has two Seafoam 1F doors, each returning to its
corresponding coast side. Route 21 South connects Cinnabar to Route 21 North,
which connects to Pallet. Both Cinnabar connections reciprocate the town's
east and north edges. Resolve offsets against the selected Fuchsia and Pallet
layouts; source FRLG offsets alone do not prove a correct HNS-town seam.

Inspect all six route and town boundaries, both Seafoam door sites, adjacent
shoreline tiles, elevations, collisions, Surf launch/landing points, and camera
seams in Porymap. Author any static tile, collision, or elevation correction
there. Never byte-patch `map.bin`. Validate walking and Surf transitions in
both directions in an emulator, including repeated crossings and save/reload.
No story, badge, HM item, Blaine, or Articuno flag may close the only
Pallet-to-Cinnabar native-Surf path or strand a player inside Seafoam.

Run FRLG Route 20's Seafoam reset behavior on the selected Route 20
transition without losing its time-of-day encounter setup. On reentering Route
20, reset the unsolved B3F boulder chain to its 1F starts and the unsolved B4F
chain to its B3F starts, using the new Wayfarer flag identities; do not reset a
chain whose current is already stopped. Keep the FRLG Seafoam signs; do not
carry the HNS Seafoam Gym sign or its Blaine text. Remove the HNS Route 19
Kingler blockade and its Blaine-dependent dialogue from the selected path.
Keep Hoenn Marine Cave's Kyogre capture reachable; the retired HNS Route 19
Cave must not provide a duplicate or an orphaned warp.

### Saved state and rewards

Inventory every selected FRLG script, object flag, and variable before
enabling the maps. Include route Trainers, hidden items, the Route 21/Pallet
boundary actor, and Route 20's transition reset. Allocate disjoint Wayfarer
persistence for Mansion switches/items,
the Secret Key, Gym quiz doors, Blaine victory and TM, Lab trades, tutor and
fossil processing, Seafoam boulders/currents/items, Articuno, and Blue's
introduction. Map one-time Trainer defeat state into the selected Wayfarer
trainer system. Reuse an existing Wayfarer identity only when it denotes the
same reward or encounter. Do not use raw FRLG constants that resolve to zero
or numeric slots already owned by HNS or Emerald. New-game initialization and
save validation must recognize each selected state and leave other regional
adventures unchanged.

Blaine's single initial award sets Kanto badge 15 and the existing global
badge/TR contribution once. Audit all consumers of
`FLAG_DEFEATED_CINNABAR_ISLAND_GYM`, `FLAG_BADGE15_GET`, old Seafoam actor
flags, Route 19 blockers and dialogue, retired HNS route references,
Viridian Gym's Blaine callback, and Gym guide/statue state. No second Blaine
battle or reward remains reachable.
Include Kanto roamer adjacency, HNS Route 20 rematch entries, battle
environment routing, native-HM coverage, and encounter-generator retirement
in that route-reference audit.
Select `HEAL_LOCATION_CINNABAR_ISLAND` for the FRLG Center and update region
map, Fly, whiteout, and respawn records from HNS map IDs and coordinates.
Arriving before visiting the Center must not create an invalid recovery point.

### Blue at Viridian

Place one Blue introduction on the walkable Viridian City exterior beside the
Gym entrance. The player can speak to him without visiting Cinnabar or
beating Blaine. He gives a first-meeting line and invites the player inside;
the introduction records once, then his exterior object leaves. If the player
enters the Gym first, its encounter supplies a truthful first meeting and
retires the unused exterior introduction. Neither path duplicates a battle,
badge, item, or Trainer Rating reward. The exterior interaction cannot block
the Gym doorway or ordinary city travel.

Initialize the Gym leader visible on a new Wayfarer save; do not wait for the
removed Cinnabar script to clear `FLAG_HIDE_VIRIDIAN_BLUE`. Give the exterior
introduction its own one-time state and hide flag, separate from the Gym
leader's visibility and defeat state. Speaking to exterior Blue records that
state and hides only his exterior object. Entering the Gym first records the
same introduction state, hides the unused exterior object, and selects the
Gym's first-meeting line. Neither path hides an undefeated Gym leader.

This port keeps Blue as Wayfarer's initial Viridian Gym Leader and sole Earth
Badge giver. His invitation is local to Viridian and has no fifteen-badge,
Cinnabar, or Blaine prerequisite. A future Kanto story may separately assign
that Gym and badge to Giovanni; that unmerged change is not required here.

Update Viridian City, Center, Gym, Pallet, and Dojo dialogue and actor flags
that currently infer Blue's return from `FLAG_HIDE_CINNABAR_BLUE`. They must
read actual Viridian introduction and Gym state. Remove lines that refer to a
Cinnabar meeting, eruption, or a prerequisite set of fifteen badges. Blue's
later Dojo and League content remains independently reachable under its own
specifications; no step here advances an unplayed rival chapter.

These Viridian Blue requirements describe the delivered coastal port. The
[future Viridian finale](frlg-kanto-viridian-finale.md) supersedes the exterior
introduction, Blue Gym role, and related acceptance checks when implemented.
Blue's Dojo appearance then follows Indigo's committed first Champion victory,
independently of Giovanni.

### Encounters, services, and release budget

Update the Kanto encounter manifest and day/night source bindings for the
selected four coastal route maps, Cinnabar, and five Seafoam maps. Preserve
the approved regional ecology and native Surf availability. Reuse the HNS
encounter tables as donors where the coast maps have matching HNS maps;
adapt FRLG tables for the remaining Seafoam and Mansion floors. Update rod-source contracts that still name
`MAP_CINNABAR_ISLAND_HNS`. Register the 32 FRLG route Trainer objects,
including two shared-identity double-battle pairs, and Mansion and Gym
Trainers, including Blaine, in Wayfarer's scaling, defeat, rematch, and
badge audits. Keep route encounter methods and one-time items attached to
the selected playable maps, without duplicate HNS route content.

Measure a release build before and after this port from the same revision and
build flags. Report linked used bytes, reserve margin, and category deltas;
checked-in asset sizes and prior Kanto milestones are not the port's ROM cost.
If the reserve fails, resolve the size budget without cutting the agreed four
routes, town, five-floor cave, puzzle, services, or rewards by default.

### Acceptance

1. Generate the Wayfarer catalog and content audits with no unavailable map,
   warp, connection, heal, state, Trainer, or encounter reference. Standalone
   HNS and FRLG catalogs remain valid. The final catalog has one playable
   route set and no orphaned HNS or navigation-only route payload, including
   the old Fuchsia gate and Route 19 Cave warps.
2. Traverse Pallet ↔ Route 21 North ↔ Route 21 South ↔ Cinnabar and
   Cinnabar ↔ Route 20 ↔ both Seafoam entrances ↔ Route 19 ↔ Fuchsia,
   by normal field movement and native Surf.
   Check shoreline, camera, music, collision, and return landing points.
   Leaving either Seafoam door resets each unfinished boulder path but leaves
   stopped currents solved; Route 20 still sets time-based encounters and its
   signs do not advertise a Seafoam Gym. Verify route Trainers, NPCs, hidden
   items, and encounter methods under the companion route specification.
3. Verify one Blaine badge/TM path, one Articuno encounter, no Seafoam or
   Dojo Blaine reward, no Seafoam Groudon, and Hoenn Terra Cave's Groudon
   encounter unaffected.
4. Save and reload during Mansion switches, Gym quiz progress, Lab revival,
   Seafoam current progress, Articuno's run/teleport reentry behavior,
   deferred item delivery, and Blue's introduction. Verify Fly, whiteout,
   Center healing, and the Mart.
5. Meet Blue at Viridian before any Cinnabar visit, after a visit, and after
   Blaine. Confirm the leader is visible from a new save, the Gym gives a
   truthful first meeting if the exterior interaction was skipped, and both
   orders retire only the exterior introduction without changing Bill or
   Sevii state.
6. Build the release ROM and record the true linked delta and reserve margin.

## References

- [Current Wayfarer map selector](../../game/tools/mapjson/mapjson.cpp)
- [FRLG Route 19](../../game/data/maps/Route19_Frlg/map.json)
- [FRLG Route 20](../../game/data/maps/Route20_Frlg/map.json)
- [FRLG Route 21 North](../../game/data/maps/Route21_North_Frlg/map.json)
- [FRLG Route 21 South](../../game/data/maps/Route21_South_Frlg/map.json)
- [Cinnabar FRLG](../../game/data/maps/CinnabarIsland_Frlg/map.json)
- [Seafoam FRLG 1F](../../game/data/maps/SeafoamIslands_1F_Frlg/map.json)
- [Map editing rule](../../AGENTS.md)
