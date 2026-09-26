# HNS Cinnabar and Seafoam map retirement

PRD: [FRLG Cinnabar and Seafoam Islands port](../prds/frlg-cinnabar-seafoam-port.md)
Implemented: Yes

The interim retirement preview merged in
[PR #110](https://github.com/mzpkdev/pokemon-wayfarer/pull/110). The final
route retirement and full coastal port merged in
[PR #112](https://github.com/mzpkdev/pokemon-wayfarer/pull/112).

## Scope

The approved full coastal port retires the superseded HNS Cinnabar, Seafoam,
Route 19 through Route 21, and Route 19 Kyogre Cave content from the active
Wayfarer selection. Hoenn Marine Cave remains the later Kyogre capture site.
Wayfarer instead selects FRLG Cinnabar, Seafoam, Route 19, Route 20, and Route
21 North and South with their route layouts, ordinary NPCs, Trainers, items,
and wild encounter profiles. The final port does not retain HNS Route 19, 20,
or 21 content as active Wayfarer content; standalone HNS retains its own maps,
scripts, and encounters unchanged.

The interim preview retired seven superseded or unreachable HNS maps from the
Wayfarer ROM: Cinnabar Island, Cinnabar Pokémon Center, Seafoam Islands 1F and
B1F, Seafoam Gym, Seafoam Secret Cave, and Route 21. In that preview, Pallet
and Cinnabar use the CoastPoc route while HNS Route 20 remains reachable
through HNS Route 19 and Seafoam's two exits. Those preview selections are
historical and are superseded by the full coastal port.

## Historical preview behavior

In the historical preview, entering the Cinnabar exterior set
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
introduction. Neither path hides an undefeated Gym leader. Viridian and Pallet
dialogue does not claim a Cinnabar meeting or eruption. Retiring Cinnabar's
Blaine script also removes its Fighting Dojo relocation trigger. No retired
Seafoam Gym or Secret Cave path grants another Blaine or Groudon encounter.

This Blue introduction records the historical coastal port. The implemented
[Viridian finale](frlg-kanto-viridian-finale.md) now removes Blue from the exterior
and Gym; its acceptance checks supersede this document's Blue check. Proposed
[trainer world progression](trainer-world-progression.md) may author a separate
Blue Gym profile, but that does not restore Viridian actors or enroll his rival,
Dojo, or rematch variants. The retirement boundary remains unchanged.

The Wayfarer release link map contains no layout, event, script, or wild
encounter payload for the seven retired HNS maps. The generated standalone HNS
catalog retains the seven source maps and layouts; this retirement does not edit
their scripts or encounter profiles. Map IDs may remain defined for shared
source code, but Wayfarer must not route the player or Fly to their retired
content.

### Historical preview acceptance

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

## Full coastal port direction

The final port must preserve native-Surf access between Pallet and Cinnabar
through the selected FRLG Route 21 North and South maps. Its ordinary coastal
wild ecology follows the existing Kanto FireRed and LeafGreen merge, authored
day-and-night, and Trainer Rating policies. The full port's implementation and
acceptance requirements remain owned by the coastal and route specifications.

## References

- [Coastal integration specification](frlg-cinnabar-seafoam-integration.md)
- [Unlocked interior preview](frlg-cinnabar-unlocked-interiors.md)
