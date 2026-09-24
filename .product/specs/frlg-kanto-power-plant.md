# Power Plant old generating hall

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md#power-plant-old-generating-hall)

Supporting requirements: [FRLG Kanto independent story beats](../prds/frlg-kanto-independent-story-beats.md#late-game-rewards) and [HNS open-world traversal](hns-open-world-region-traversal.md#magnet-train-restoration).

Implemented: No. This specifies the selected Wayfarer port; current source maps and scripts do not establish a playable Wayfarer hall.

## Place and content

The HNS Power Plant remains an operating facility with its existing Route 10 exterior, entrance hall, back room, workers, manager, Magneton trade, and Machine Part repair. Its older generating hall remains behind the staffed section. One existing worker in `Route10_PowerPlantEntrance_hns` offers a fade transition into the FRLG `PowerPlant_Frlg` interior. The old hall is explorable before or after repair, without a badge, Machine Part state, radio upgrade, another Rocket investigation, or League clear. Declining the worker's offer changes no state. Choose the worker and arrival tiles only after checking collision, facing, movement, and existing interactions; this specification does not assert coordinates.

Reuse the complete 49×40 FRLG hall layout and maze, its ordinary wild encounters, five visible items (Max Potion, TM17, TM25, Thunder Stone, Elixir), two hidden items (Max Elixir, Thunder Stone), and two Electrode decoys. Preserve their one-time item and encounter state across exits, save/reload, and failed Bag handoffs. Select the map, encounters, required assets, scripts, and map identity for Wayfarer explicitly; their source presence is not proof of runtime inclusion. This port reuses the existing layout binaries: it edits no `map.bin` and imports no FRLG Route 10 exterior.

Every one of the FRLG hall's five exit warp records must lead to a verified walkable arrival in the HNS entrance hall, including its shortcut exits. The entry transition must land at the hall's playable entrance without triggering an exit loop. Escape and blackout use valid Wayfarer return or heal behavior and never strand the player. Audit both directions, map music/section, encounter selection, and rendered collision before assigning coordinates. Keep the HNS entrance-to-back-room door and its reciprocal warp; the back room's west Route 10 exit remains blocked by its existing engineer until repair and opens afterward. The old hall offers no bypass that completes or changes that repair gate.

## Independent repair and services

The existing manager starts and completes the Machine Part sequence. Its Cerulean suspect, Rocket battle, Gym pickup, Thunder TM handoff, engineer removal, and `FLAG_RETURNED_MACHINE_PART` state retain the [current traversal contract](hns-open-world-region-traversal.md#magnet-train-restoration). Exploring the old hall, taking items, fighting Electrode, or meeting Zapdos does not start, advance, or repair the generator. A full Bag or failed item handoff must leave repair retryable.

Repair continues to enable its existing consumers: Misty's Route 25 date and Gym return, the Lavender radio upgrade and its broadcasts, Copycat's doll and Pass, Route 5/6 Underground Path access, and Magnet Train service. The existing Magneton trade remains independent. These services do not gate old-hall exploration or Zapdos readiness. Preserve the hall worker's existing clue/service dialogue when adding the entrance offer.

## Zapdos

Move the single Wayfarer Route 10 exterior Zapdos encounter into the FRLG hall's source position. Remove its exterior object and interaction so there is only one Zapdos. Exploration and approach remain available below Trainer Rating (TR) 55. Below TR 55, interaction gives a brief readiness message without starting battle or changing Zapdos state. At TR 55 or above, start the fixed level 50 static Zapdos battle; TR is an eligibility check, not a level-scaling or capture-rate change. No Articuno, Rocket, Machine Part, badge, or League condition is added.

Use FRLG's Zapdos outcome policy, with Wayfarer's TR gate added before battle:

| Outcome | Encounter state |
| --- | --- |
| Caught | Resolve permanently; Zapdos stays absent. |
| Defeated (KO) | Resolve permanently; Zapdos stays absent. |
| Run or player teleports | Remove the current object; Zapdos returns on map reentry. |
| Player blacks out | Whiteout bypasses the script continuation; Zapdos remains available after return. |

Give the hall's Zapdos a dedicated persistent resolved state and visibility state. The current shared `FLAG_FOUGHT_ZAPDOS` and other imported FRLG Power Plant aliases are zero in Wayfarer's flag table, so implementation must allocate real Wayfarer state without prescribing IDs here. A Hall of Fame script currently clears the HNS exterior hide flag; remove or isolate that reset for this encounter so a caught or defeated Zapdos never reappears after League completion. Running, teleporting, blacking out, saving, or reentering must not turn a resolved encounter into a second capture. This outcome choice applies to Zapdos here; it does not change Articuno, Moltres, or other legendary encounters.

## Acceptance

- Visit the hall before initiating repair, during the Machine Part errand, and after repair. Inspect the worker interaction, all five FRLG exit records, both HNS rooms and doors, the repair-gated west exit, Escape, blackout, and save/reload. No path bypasses or advances the repair sequence.
- Traverse the full maze and verify ordinary encounters, five visible items, two hidden items, and both Electrode decoys. Items and resolved decoys do not reset on reentry; Bag failures remain claimable.
- Approach Zapdos below TR 55 and at TR 55. Confirm fixed level 50, one indoor object, no exterior duplicate, and no requirement from repair, other birds, Rockets, or League.
- Exercise catch, KO, run, teleport, and blackout separately, then leave/reenter, save/reload, and finish the League. Only run, teleport, and blackout permit another battle; completion never respawns.
- Complete the existing repair with and without prior hall exploration. Confirm Thunder TM/Part failure recovery and the Misty, radio, Copycat, Underground Path, train, and Magneton trade behaviors.

## Evidence

- [FRLG Power Plant map](../../game/data/maps/PowerPlant_Frlg/map.json) and [scripts](../../game/data/maps/PowerPlant_Frlg/scripts.inc)
- [HNS entrance hall](../../game/data/maps/Route10_PowerPlantEntrance_hns/map.json), [back room](../../game/data/maps/Route10_PowerPlantBackRoom_hns/map.json), and [Route 10 Zapdos](../../game/data/maps/Route10_hns/scripts.inc)
- [Battle return callback](../../game/src/battle_setup.c) and [Hall of Fame reset](../../game/data/maps/PokemonLeague_HallOfFame_hns/scripts.inc)
- [Original FRLG Power Plant script](https://github.com/pret/pokefirered/blob/master/data/maps/PowerPlant/scripts.inc) and [battle callback](https://github.com/pret/pokefirered/blob/master/src/battle_setup.c)
