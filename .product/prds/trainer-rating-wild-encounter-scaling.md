# Trainer Rating wild encounter scaling

## Intent

Keep ordinary wild encounters relevant as a player advances through a campaign. One global Trainer Rating should give every supported region in a ROM build the same progression baseline without rewriting its authored encounter tables.

## Design

Trainer Rating is a persistent progression value, and the system never lowers a
saved rating. A new Wayfarer game starts at 0. The rating is internal and has
no player-facing screen.

Every ordinary encounter profile compiled into Emerald, FireRed/LeafGreen, or HNS uses the same rating. The system adjusts encounter levels and can return an over-levelled evolved species to an eligible predecessor. It also respects specific species floors.

Wayfarer uses one global twenty-four-badge circuit across Kanto, Johto, and
Hoenn. Badge origin does not affect League qualification or rating. Its targets
are:

| Progress | Trainer Rating |
| --- | ---: |
| New game | 0 |
| 4 total badges | 16 |
| 8 total badges | 40 |
| Kanto League cleared | 55 |
| 16 total badges | 63 |
| Johto League cleared | 68 |
| 24 total badges | 76 |
| Hoenn League cleared | 80 |

The interregional League circuit defines the exact badge and League
contributions. This replaces the earlier concept of one full regional campaign
plus smaller breadth contributions from the other regions.

## Boundaries

The system only covers ordinary wild encounter profiles. It does not scale hidden DexNav encounters, fixed or scripted encounters, roamers, outbreaks, Feebas, Battle Pike encounters, or Battle Pyramid encounters.

It does not change authored encounter data or create a player-facing Trainer Rating display. It also does not define trainer battle scaling.

Adding an HNS Hoenn warp does not enroll Hoenn encounters in this system. Those profiles must be compiled for the active build first. Once they target
Wayfarer, ordinary profiles use the existing global scaling automatically.
Hoenn badges and the Hoenn League contribute to Wayfarer rating through the
interregional circuit.

## Balance

Wayfarer starts at Rating 0 so the value represents an unproven Trainer before
the first badge. Its encounter curve must still keep starter-area populations
viable at Rating 0. The first eight badges and Kanto League clear create the
largest rating gains. Later badges and League clears provide diminishing gains
as the rating approaches 80.

Rating 0 must not remove a native utility catch that supplies an approved core
route. The Wayfarer circuit owns the level-5 Chinchou compatibility adjustment
needed to preserve Kanto's native-Surf route.

## Content

The scope includes all ordinary encounter profiles that each build currently compiles:

- Emerald: Hoenn.
- FireRed/LeafGreen: Kanto and the Sevii Islands.
- HNS: Johto, Kanto, Alola, Sinjoh, Faraway Island, and Southern Island.

Sinjoh is a distinct region in HNS. No full Sinnoh encounter region is present in the current builds.

Profile identity and encounter mechanics remain authored. This includes land, water, Rock Smash, fishing rods, time of day, ability-based selection, lures, Altering Cave, and HNS Hoenn Sound behavior.

## Interactions

When a projected level is below a level-evolved species' evolution threshold, the encounter uses the appropriate predecessor. This applies to every non-randomized ordinary encounter, including tables that author an evolved form below its natural threshold. Resolution follows the predecessor chain until the projected level supports the stage. Fixed and scripted encounters remain excluded.

Global species floors then apply to every non-randomized ordinary wild population. A slot is ineligible if one of its possible projected outcomes has a resulting species below that species' floor; ordinary selection uses the remaining authored weights without rewriting the source table.

| Species | Minimum level | Example ordinary encounter |
| --- | ---: | --- |
| Kecleon | 20 | Emerald, Route 118, land grass, 25 |
| Skarmory | 18 | Emerald, Route 113, land grass, 16 |
| Scyther | 23 | FireRed, Safari Zone Center, land grass, 23 |
| Pinsir | 23 | LeafGreen, Safari Zone Center, land grass, 23 |
| Chansey | 23 | FireRed, Safari Zone Center, land grass, 23 |
| Kangaskhan | 25 | FireRed, Safari Zone East, land grass, 25 |
| Tauros | 25 | FireRed, Safari Zone West, land grass, 25 |
| Relicanth | 25 | Emerald, Underwater Route 124, water/surf, 30-35 |
| Sneasel | 30 | LeafGreen, Four Island Icefall Cave 1F, land grass, 30 |
| Mantine | 14 | HNS, Whirl Islands, water/surf, authored 15-19 and projected 14 at Rating 10 |
| Bagon | 20 | Emerald, Meteor Falls B1F 2R, land grass, 25-35 |
| Tropius | 20 | Emerald, Route 119, land grass, 25-27 |
| Absol | 20 | Emerald, Route 120, land grass, 25-27 |
| Heracross | 20 | Emerald, Safari Zone North, land grass, 27-29 |

Each example identifies an ordinary encounter that motivated review of the global floor. It does not limit the floor to that map, method, build, or level range.

Mantine's floor is 14 because an authored level-15 Mantine can project to
level 14 at Rating 10. The floor applies globally. It does not add a
Johto-only exception, change authored encounter levels, or introduce Mantyke
predecessor resolution.

Ordinary-population readers use the same effective population as an actual encounter. This includes the Pokédex area display, Match Call, radio, local ambient species selection, and ordinary land or water DexNav populations. Hidden DexNav populations stay authored and unscaled.

In randomizer mode, the selected slot's level still scales, while the existing randomized-species mapping remains in control. Reverse evolution and species-floor filtering do not apply to randomized populations.

## Constraints

Trainer Rating must survive saves and migrations without changing save-block layouts. A migrated save derives an appropriate rating from its existing progression, and future reads preserve the higher of the stored and derived values. Wayfarer clamps the value to the inclusive range 0 through 80.

## Playtesting

Playtesting should confirm that Wayfarer encounters feel appropriate at Rating
0 and at every badge and League milestone through Rating 80. Coverage should
include land, water, Rock Smash, fishing, time-based, ability-influenced, lure,
Altering Cave, and HNS Hoenn Sound encounters, plus the ordinary population
readers.

It should also check that excluded sources remain unchanged, that a later rating never produces a lower projected encounter outcome, and that existing saves migrate without losing progression.

## References

- [Technical specification](../specs/trainer-rating-wild-encounter-scaling.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Implementation pull request](https://github.com/mzpkdev/pokemon-wayfarer/pull/13)
