# Cinnabar and Seafoam coastal integration

PRD: [FRLG Cinnabar and Seafoam Islands port](../prds/frlg-cinnabar-seafoam-port.md)
Implemented: No

## Scope

Integrate the two local FRLG map sets with Wayfarer's selected Kanto routes,
state banks, travel services, encounters, Trainer systems, and Viridian Blue.
The town and cave specifications own local gameplay. This specification does
not implement the unrelated Kanto story branch or Sevii campaign.

## Behavior

### Map selection and coastal boundaries

Enable the thirteen Cinnabar maps and five Seafoam floor maps, plus Seafoam's
two stopped-current layouts, for `POKEMON_WAYFARER` only. Include their
required FRLG primary and secondary tilesets, object graphics, door effects,
music, scripts, layouts, events, and wild encounters. The selected catalog
must have unique map IDs and complete warp, connection, and heal references.
Remove the HNS island, HNS Center, HNS Seafoam 1F/B1F, Gym, and Secret Cave
from that catalog. Do not change standalone HNS or FRLG selection.

Keep Wayfarer's HNS Route 20 and single HNS Route 21 as the neighboring route
content. Route 20 is 120×20 and has two existing Seafoam door events; both
must point to the corresponding FRLG 1F exits. Its western connection must
point to FRLG Cinnabar and reciprocate the town's eastern connection. Route
21 is 24×100; its southern connection must point to FRLG Cinnabar and
reciprocate the town's northern connection. Keep Route 20's eastern Route 19
and Route 21's northern Pallet connections working. Do not copy FRLG's zero
offsets onto HNS routes without examining both boundaries.

Inspect the four exterior boundaries, both Seafoam door sites, adjacent
shoreline tiles, elevations, collisions, Surf launch/landing points, and camera
seams in Porymap. Author any static tile, collision, or elevation correction
there. Never byte-patch `map.bin`. Validate walking and Surf transitions in
both directions in an emulator, including repeated crossings and save/reload.
No story, badge, HM item, Blaine, or Articuno flag may close the only
Pallet-to-Cinnabar native-Surf path or strand a player inside Seafoam.

### Saved state and rewards

Inventory every selected FRLG script and object flag and var before enabling
the maps. Allocate disjoint Wayfarer persistence for Mansion switches/items,
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
flags, Route 19 blockers and dialogue, Viridian Gym's Blaine callback, and
Gym guide/statue state. No second Blaine battle or reward remains reachable.
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

### Encounters, services, and release budget

Update the Kanto encounter manifest and day/night source bindings for the
selected Cinnabar and five Seafoam maps. Preserve the approved regional
ecology and native Surf availability; adapt version-specific FRLG species
through the existing encounter policy rather than silently substituting HNS
volcanic or ice-cave tables. Update rod-source contracts that still name
`MAP_CINNABAR_ISLAND_HNS`. Register Mansion and Gym Trainers, including
Blaine, in Wayfarer's scaling, defeat, rematch, and badge audits.

Measure a release build before and after this port from the same revision and
build flags. Report linked used bytes, reserve margin, and category deltas;
checked-in asset sizes and prior Kanto milestones are not the port's ROM cost.
If the reserve fails, resolve the size budget without cutting the agreed town,
five-floor cave, puzzle, services, or rewards by default.

### Acceptance

1. Generate the Wayfarer catalog and content audits with no unavailable map,
   warp, connection, heal, state, Trainer, or encounter reference. Standalone
   HNS and FRLG catalogs remain valid.
2. Traverse Pallet ↔ Route 21 ↔ Cinnabar and Cinnabar ↔ Route 20 ↔ both
   Seafoam entrances ↔ Route 19, by normal field movement and native Surf.
   Check shoreline, camera, music, collision, and return landing points.
3. Verify one Blaine badge/TM path, one Articuno encounter, no Seafoam or
   Dojo Blaine reward, no Seafoam Groudon, and Hoenn Terra Cave's Groudon
   encounter unaffected.
4. Save and reload during Mansion switches, Gym quiz progress, Lab revival,
   Seafoam current progress, Articuno retry, deferred item delivery, and
   Blue's introduction. Verify Fly, whiteout, Center healing, and the Mart.
5. Meet Blue at Viridian before any Cinnabar visit, after a visit, and after
   Blaine. Confirm the Gym gives a truthful first meeting if the exterior
   interaction was skipped, and neither path changes Bill or Sevii state.
6. Build the release ROM and record the true linked delta and reserve margin.

## References

- [Current Wayfarer map selector](../../game/tools/mapjson/mapjson.cpp)
- [Route 20 HNS](../../game/data/maps/Route20_hns/map.json)
- [Route 21 HNS](../../game/data/maps/Route21_hns/map.json)
- [Cinnabar FRLG](../../game/data/maps/CinnabarIsland_Frlg/map.json)
- [Seafoam FRLG 1F](../../game/data/maps/SeafoamIslands_1F_Frlg/map.json)
- [Map editing rule](../../AGENTS.md)
