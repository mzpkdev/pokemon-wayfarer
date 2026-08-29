# Trainer Rating wild encounter scaling

## Intent

Keep ordinary wild encounters relevant as a player advances through a campaign. One global Trainer Rating should give every supported region in a ROM build the same progression baseline without rewriting its authored encounter tables.

## Design

Trainer Rating is a persistent progression value. New games start at 10, the lowest supported rating, and the system never lowers a saved rating. The rating is internal and has no player-facing screen.

Every ordinary encounter profile compiled into Emerald, FireRed/LeafGreen, or the HNS regions uses the same rating. The system adjusts encounter levels and can return an over-levelled evolved species to an eligible predecessor. It also respects specific species floors.

The campaign targets are:

| Progress | Trainer Rating |
| --- | ---: |
| New game | 10 |
| Four badges | 16 |
| Eight badges | 40 |
| First League clear | 55 |
| Substantial postgame | 65 |

The rating can reach at most 80, leaving room for later progression.

## Boundaries

The system only covers ordinary wild encounter profiles. It does not scale hidden DexNav encounters, fixed or scripted encounters, roamers, outbreaks, Feebas, Battle Pike encounters, or Battle Pyramid encounters.

It does not change authored encounter data or create a player-facing Trainer Rating display. It also does not define trainer battle scaling.

Adding an HNS Hoenn warp does not enroll Hoenn encounters in this system. Those profiles must be compiled for HNS first. Once they target HNS, ordinary profiles use the existing global scaling automatically. Hoenn-specific progression milestones remain a separate design decision.

## Balance

The early game begins from Rating 10 rather than zero so starter-area encounters do not become trivial on a new save. Progression follows the shared campaign milestones, then leaves headroom above the current 65 postgame target for future content.

## Content

The scope includes all ordinary encounter profiles that each build currently compiles:

- Emerald: its available regions.
- FireRed/LeafGreen: Kanto and the Sevii Islands.
- HNS: Johto, Kanto, and its miscellaneous regions.

Profile identity and encounter mechanics remain authored. This includes land, water, Rock Smash, fishing rods, time of day, ability-based selection, lures, Altering Cave, and HNS Hoenn Sound behavior.

## Interactions

When a projected level is below a level-evolved species' evolution threshold, the encounter uses the appropriate predecessor. This applies to every non-randomized ordinary encounter, including tables that author an evolved form below its natural threshold. Resolution follows the predecessor chain until the projected level supports the stage. Fixed and scripted encounters remain excluded. The initial explicit floors keep Kecleon at level 20 or above and Skarmory at level 18 or above. A floor can make a slot ineligible, after which ordinary selection uses the remaining authored weights.

Ordinary-population readers use the same effective population as an actual encounter. This includes the Pokédex area display, Match Call, radio, local ambient species selection, and ordinary land or water DexNav populations. Hidden DexNav populations stay authored and unscaled.

In randomizer mode, the selected slot's level still scales, while the existing randomized-species mapping remains in control. Reverse evolution and species-floor filtering do not apply to randomized populations.

## Constraints

Trainer Rating must survive saves and migrations without changing save-block layouts. A migrated save derives an appropriate rating from its existing progression, and future reads preserve the higher of the stored and derived values. The value is clamped to the inclusive range 10 through 80.

## Playtesting

Playtesting should confirm that new-game, four-badge, eight-badge, League, and postgame encounters feel appropriate in every currently supported region. It should cover land, water, Rock Smash, fishing, time-based, ability-influenced, lure, Altering Cave, and HNS Hoenn Sound encounters, plus the ordinary population readers.

It should also check that excluded sources remain unchanged, that a later rating never produces a lower projected encounter outcome, and that existing saves migrate without losing progression.

## Open questions

- Should Aerodactyl, Heracross, and Bagon receive explicit ordinary-wild floors of 20, 15, and 25 respectively? A cross-region table audit identified them as the strongest additional candidates.

## References

- [Technical specification](../specs/trainer-rating-wild-encounter-scaling.md)
- [Implementation pull request](https://github.com/mzpkdev/pokemon-wayfarer/pull/13)
