# FRLG Cinnabar town port

PRD: [FRLG Cinnabar and Seafoam Islands port](../prds/frlg-cinnabar-seafoam-port.md)
Implemented: No

The [unlocked interior preview](frlg-cinnabar-unlocked-interiors.md) is a
separate interim milestone. It makes the twelve rooms traversable and supplies
Center healing and Mart shopping, but does not implement the local content and
rewards specified here.

## Scope

Select the intact FRLG Cinnabar exterior and its twelve local interior maps for
Wayfarer: Mansion 1F, 2F, 3F, B1F; Gym; Lab entrance, lounge, research room,
experiment room; Pokémon Center 1F and 2F; and Mart. This spec owns their
local events and rewards. The
[route specification](frlg-kanto-coastal-routes-port.md) owns FRLG Route 20
and both Route 21 sections. The companion integration spec owns route seams,
state allocation, Fly/respawn routing, and Blue's Viridian introduction.

## Behavior

### Town and entrances

Use `CinnabarIsland_Frlg` with its 24×20 layout, town map identity, FRLG
graphics, Cinnabar music, outdoor objects, and five building entrances. Each
entrance and each interior exit returns to the matching exterior door. Select
the listed interior layouts, events, and scripts as a closed set. Remove the
HNS island and HNS Pokémon Center from the selected Wayfarer catalog; do not
carry over the New Bark warp, eruption actors, or relocated-Gym sign. The
center, Lab, Mart, and Mansion are accessible without the Secret Key. Visiting
town does not alter another adventure's state.

### Mansion and Secret Key

Keep all four FRLG floors, their stairs, falls, statue switches, diary
interactions, seven Trainers, visible and hidden items, and wild encounters.
Switches must change the intended barriers on all affected floors and restore
their state on load. The B1F Secret Key is a one-time pickup. Obtaining it
unlocks the Cinnabar Gym door; reaching the island, defeating another Leader,
or owning a different key does not. An already collected Key continues to
unlock the door after save/reload, regardless of whether it remains in the Bag
under the game's key-item policy.

### Gym and Blaine

Keep the FRLG Gym layout, six quiz doors, seven Trainers, Gym Guy, statues,
photo, and Blaine. Each quiz door opens by its authored correct answer or
Trainer outcome and restores its state on reentry. Blaine has one initial
reward-bearing battle at this Gym. His victory awards Wayfarer's Kanto
Volcano Badge identity (`FLAG_BADGE15_GET`), increments badge/accounting once
through the regional helpers, records the one Blaine defeat, opens all doors,
and makes TM38 Fire Blast claimable exactly once. A full Bag leaves the TM
claimable on return; repeated dialogue does not award another badge or TM.
His party and the seven Gym Trainers participate in Wayfarer's scaling and
trainer-defeat systems rather than using unadjusted standalone FRLG progression.

The selected Wayfarer build contains no reward-bearing Blaine battle in
Seafoam and no relocation to the Fighting Dojo. Replace stock FRLG Gym lines
that set `FLAG_BADGE07_GET`, use a conflicting defeat flag, or start the Bill
scene. Blaine's victory does not change Sevii travel, Meteorite, Bill, or
Seagallop state.

### Lab, Center, and Mart

Keep the Lab entrance and all three rooms, including three distinct one-time
in-game trades and the Metronome tutor. Each trade checks the requested
species, records completion only after a successful exchange, and cannot be
repeated after travel or reload. The fossil scientist accepts Helix Fossil,
Dome Fossil, or Old Amber when the player owns an eligible one. Acceptance
consumes only that fossil and saves which species is pending. After the
authored leave-and-return step, the player receives the corresponding
Omanyte, Kabuto, or Aerodactyl once. A full party or other delivery failure
keeps the revived Pokémon pending and recoverable. Existing fossil rewards
from other regions must neither be duplicated nor treated as already revived.

Center 1F heals, sets the FRLG Cinnabar heal location, and is the town's
whiteout/Fly destination. Center 2F remains a reachable service floor with
working supported facilities and clear unavailable responses for facilities
the product does not support. The Mart retains its FRLG room, counter, and
clerk interaction; its inventory follows the Wayfarer
[global Mart profile](global-tr-pokemarts.md) for Cinnabar.
None of these services requires defeating Blaine, completing the Mansion, or
starting Bill's story.

### Town encounters and presentation

Select FRLG Cinnabar's Surf and fishing ecology through Wayfarer's encounter
pipeline; remove the selected HNS volcanic land/water/day/night bindings.
If Wayfarer's active encounter policy requires a native Surf source or
day/night coverage, preserve it explicitly under the new town map identity.
Keep building objects, signs, doors, sounds, and collision consistent with
the FRLG layout. Rewrite or suppress HNS and FRLG dialogue that asserts an
eruption, Blaine's relocation, a prior Cinnabar meeting with Blue, or a forced
Bill/Sevii voyage.

### Acceptance

1. Every town door has the correct reciprocal exit; all four Mansion and Lab
   rooms, both Center floors, and the Mart are reachable and escapable.
2. The Mansion switches, falls, Trainers, items, Secret Key, six quiz doors,
   Blaine battle, badge, and deferred TM survive travel and save/reload.
3. Exercise all three Lab trades, the tutor, and all three fossils, including
   a full party or Bag, repeated interaction, and save/reload while a revival
   is pending.
4. Heal, whiteout, and Fly to the new town; use its Mart and Center before
   Blaine. Confirm no Cinnabar action starts Bill's Sevii trip or reveals a
   second Blaine encounter.
5. Check wild methods, trainer scaling, door/collision art, music, and the
   town's visible condition in a playable Wayfarer ROM.

## References

- [FRLG town map](../../game/data/maps/CinnabarIsland_Frlg/map.json)
- [Mansion switch script](../../game/data/scripts/pokemon_mansion.inc)
- [FRLG Gym script](../../game/data/maps/CinnabarIsland_Gym_Frlg/scripts.inc)
- [Fossil scientist script](../../game/data/maps/CinnabarIsland_PokemonLab_ExperimentRoom_Frlg/scripts.inc)
- [FRLG Center heal location](../../game/src/data/heal_locations.json)
- [Coastal and state integration](frlg-cinnabar-seafoam-integration.md)
