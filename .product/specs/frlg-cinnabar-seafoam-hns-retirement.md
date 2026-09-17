# HNS Cinnabar and Seafoam map retirement

PRD: [FRLG Cinnabar and Seafoam Islands port](../prds/frlg-cinnabar-seafoam-port.md)
Implemented: No

This slice is being implemented in [PR #110](https://github.com/mzpkdev/pokemon-wayfarer/pull/110).
Change the marker to `Yes` once it lands on `main`.

## Scope

Retire seven superseded or unreachable HNS maps from the Wayfarer ROM: Cinnabar Island,
Cinnabar Pokémon Center, Seafoam Islands 1F and B1F, Seafoam Gym, and Seafoam
Secret Cave, plus Route 21. Remove their map, layout, script, and wild encounter
payloads from Wayfarer while retaining them for standalone HNS. HNS Route 21
has no playable incoming connection in this preview: Pallet and Cinnabar use
the CoastPoc route instead. Its three Trainers, hidden items, and encounters
remain in source for the final coastal integration, which must place that
content on a playable route. HNS Route 20 remains reachable through Route 19
and Seafoam's two exits, preserving its authored Trainers and fishing.

## Behavior

Entering the current Cinnabar exterior preview sets
`FLAG_VISITED_CINNABAR_ISLAND`, which unlocks Fly. Fly lands on that exterior;
Center healing and whiteout use its reachable 1F nurse and Cinnabar heal
location. An arrival before visiting the Center must not create an invalid
recovery point. The two Seafoam doors on retained HNS Route 20 lead to a
reachable 1F preview and return to retained HNS Route 20. No route connection,
warp, fall, battle environment, or heal record in Wayfarer points into a
retired map. Shared scripts for other legendary birds must resolve their
encounter text without loading the retired Seafoam B1F script.

Retarget Cinnabar's day and night Surf and fishing encounters to the preview
exterior. Keep Chinchou as a native Surf source and preserve Wayfarer's
existing Kingler fishing override. Encounter headers must not point at the
retired Cinnabar or Seafoam maps.

Wayfarer's Viridian Blue introduction and Gym access no longer depend on the
retired HNS Cinnabar script clearing `FLAG_HIDE_VIRIDIAN_BLUE`. Exterior Blue
is available for a one-time introduction beside the Gym; entering the Gym
first provides a truthful first meeting and retires the unused exterior
introduction. Existing saves that still hide an undefeated Blue recover on
entering Viridian. Neither path hides an undefeated Gym leader. Viridian and Pallet
dialogue does not claim a Cinnabar meeting or eruption. Retiring Cinnabar's
Blaine script also removes its Fighting Dojo relocation trigger. No retired
Seafoam Gym or Secret Cave path grants another Blaine or Groudon encounter.

The Wayfarer release link map contains no layout, event, script, or wild
encounter payload for the seven retired HNS maps. The generated standalone HNS
catalog retains the seven source maps and layouts; this retirement does not edit
their scripts or encounter profiles. Map IDs may remain defined for shared
source code, but Wayfarer must not route the player or Fly to their retired
content.

### Acceptance

1. Visit Cinnabar and Fly back; heal and whiteout at its preview Center. Check
   a visit before Center activation does not move the recovery point.
2. Enter and exit both retained Route 20 Seafoam doors, returning to Route 20.
   Check Cinnabar day and night fishing, native Surf availability, and battle
   environment selection.
3. Meet Blue on the Viridian exterior and, in a separate run, enter his Gym
   first. Check that each path leaves one accessible Gym leader and truthful
   local dialogue, without enabling old Blaine or Groudon paths.
4. Confirm the seven old payloads are absent from the Wayfarer release link map,
   remain in the generated HNS catalog, and the Wayfarer release reserve still
   passes.

## References

- [Coastal integration specification](frlg-cinnabar-seafoam-integration.md)
- [Unlocked interior preview](frlg-cinnabar-unlocked-interiors.md)
