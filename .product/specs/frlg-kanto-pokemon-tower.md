# Pokémon Tower in Lavender's radio conversion

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md#lavender-during-the-radio-conversion)

Supporting requirements: [independent story beats](../prds/frlg-kanto-independent-story-beats.md#celadon-lavender-and-the-flute) and [HNS open-world traversal](hns-open-world-region-traversal.md).

Implemented: No. This is the approved direction for a future Wayfarer port, not a claim about current ROM behavior.

## Outcome and boundary

Lavender is converting Pokémon Tower for radio use while its memorial floors remain in service. The HNS radio lobby and studio occupy the renovated ground floor. Graves and Channelers remain on floors 2F–7F, and the Soul House has already received some memorials. Fuji supervises the remaining move; disturbances and his disappearance pause it. Dialogue describes this local transition without assigning Wayfarer an exact year or asserting that Johto's Radio Tower takeover already happened.

The initial port selects the existing `LavenderTown_RadioStation_hns` as the shared ground floor and FRLG `PokemonTower_2F_Frlg` through `PokemonTower_7F_Frlg` as the upper floor graph. It does not replace Lavender's exterior or add a separate radio annex. Keep the source upper-floor layouts, stairs, graves, Channelers, wild encounters, healing area, items, Rocket confrontation, and Fuji rescue, adapting scripts and battle data to Wayfarer's selected maps, scaling, origin, and persistent state. Source files present in the repository are not proof that those maps or encounters are selected in a Wayfarer build.

The FRLG `PokemonTower_1F_Frlg` layout is not a playable second foyer in this phase. Audit its mourners and clues and carry a bounded selection into the shared lobby or Soul House: at minimum, preserve a mourner's perspective, the warning that spirits are disturbed, and a locally understandable Silph Scope lead. Do not copy every 1F actor into a room that lacks verified free positions. Identify the selected actors, dialogue, and placement in the implementation review.

## Entrance and floor graph

For the initial version, talking to the existing radio guard offers a fade transition to 2F near its downward stair. Declining leaves the player in the lobby. The guard explains the renovation, memorial access, and the paused relocation; no badge, Silph Scope, Power Plant repair, radio upgrade, or other-region campaign is an entrance condition. Adapt the directory and ground-floor NPC lines so they describe the actual playable floors and do not promise accessible sales, personnel, production, or director floors. Keep the director, host, reception, library, and radio interactions reachable.

Redirect 2F's downward stair to a walkable lobby arrival position. Its upward stair and the 3F–7F connections retain the source floor order. Leaving, saving and reloading, escaping where supported, and blacking out must never strand the player upstairs or mark the adventure complete. The implementation must inspect collision, facing, elevation, guard reachability, rival trigger positions, and both arrival paths on the selected layouts before choosing coordinates. No coordinate in this specification is a claim of verified placement.

This first transition changes events and scripts only; it requires no `map.bin` edit. A later Porymap-authored revision may put physical stairs or a doorway into the radio lobby and add visible renovation cues such as barriers or moving supplies. That revision may edit `map.bin` and must prove the entrance, collision, return path, and radio-service reachability again. It preserves the same shared building and story state. The memorial floors stay accessible after rescue; conversion does not erase the dungeon.

## Adventure and state

Players may discover the haunting and explore the lower memorial floors before visiting Celadon. The 2F clue and Lavender dialogue point toward the Silph Scope without granting it. Defeating Celadon's Giovanni and receiving the Scope remain the normal way to identify the ghosts. The Scope gates resolution of the 6F Marowak spirit and passage to Fuji, not entry to the building or independent radio services. Preserve the source encounter's identity and non-capture story outcome; a loss or retreat leaves it retryable. Do not claim the living Marowak in HNS House1 is Cubone's mother: it is a separate shelter resident, whose existing dialogue needs only a conflict check.

After Marowak is calmed, the player confronts the Rockets on 7F and rescues Fuji. Defeated Trainers, collected items, the ghost resolution, and the rescue persist across exits, losses, and reloads without resetting one another. Rescue returns Fuji to his existing HNS home, `LavenderTown_House1_hns`, where he gives the physical Poké Flute once. A full Bag leaves the gift claimable; do not set the claimed state before delivery succeeds. Revisit dialogue reflects only events actually completed. The separate Route 12/16 Snorlax adaptation owns its placement and use of the physical Flute; this spec neither wakes a Snorlax nor makes one a Tower prerequisite.

Fuji's current Soul House actor must be absent during the incident. After rescue, he appears at House1 for the Flute and later dialogue, rather than simultaneously reappearing at Soul House. Keep the Soul House as a public chapel with its other visitors and graves. Update their dialogue and the House1 resident's “Fuji is at the Soul House” direction for missing, rescued, and returned states. Audit the source 7F rescue warp, FRLG Volunteer Pokémon House gift script, HNS House1's living Marowak, and existing Fuji actors so no duplicate Fuji, misleading destination, or unclaimable reward survives.

Blue's source 2F battle assumes the FRLG childhood rivalry and starter roster. Supported visiting origins receive a short optional introduction or context scene with remembered state, without a forced battle or rival progress. A future Kanto origin may use its authored Blue chapter, following the main story PRD's forward-progression rule. Skipping or retiring Blue's scene never gates the stair, Scope clue, Marowak, Rockets, Fuji, or Flute.

## Radio independence

The HNS Machine Part repair remains the director's condition for the Kanto radio upgrade. Tower entry, Scope possession, Marowak, rescue, and Flute do not grant or require that upgrade. The radio's Poké Flute program and Fuji's physical item have separate acquisition and use paths; neither silently completes the other's quest. The radio may remain off air until the existing Power Plant condition is satisfied while its lobby and memorial entrance stay accessible. Preserve the library, dialogue, other radio services, and existing Power Plant consumers.

## Acceptance

- Enter and leave the Tower before Scope acquisition and before Machine Part repair. Verify every stair, save/reload, blackout, and normal return route, and confirm the radio director still follows only his existing repair state.
- Resolve the Scope, Marowak, Rocket, Fuji, and Flute sequence after discovering Lavender first or Celadon first. Test losses, departures, reloads, and a full Bag at the gift; no step advances from merely entering or talking to the guard.
- Verify one Fuji in the world state at a time, with truthful Soul House, House1, radio guard, and directory text. Inspect the living House1 Marowak scene against the ghost story.
- Test visiting origins with no Blue battle, and return after later Blue progress. The adventure and its rewards remain playable. Check ordinary Trainers, items, wild encounters, scaling, and persistent floor access after rescue.
- Inspect the rendered lobby and 2F entrance/return tiles for collision and visual coherence. For a future physical entrance, repeat that check after the Porymap layout edit and confirm radio services remain reachable.

## Source anchors

- [HNS radio station map and script](../../game/data/maps/LavenderTown_RadioStation_hns/map.json)
- [HNS Soul House](../../game/data/maps/LavenderTown_SoulHouse_hns/map.json)
- [HNS House1 and living Marowak](../../game/data/maps/LavenderTown_House1_hns/scripts.inc)
- [FRLG Tower 2F entry and Blue triggers](../../game/data/maps/PokemonTower_2F_Frlg/map.json)
- [FRLG Tower 6F Marowak script](../../game/data/maps/PokemonTower_6F_Frlg/scripts.inc)
- [FRLG Tower 7F rescue script](../../game/data/maps/PokemonTower_7F_Frlg/scripts.inc)
