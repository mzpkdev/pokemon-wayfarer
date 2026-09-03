# Trainer Rating wild encounter scaling

PRD: [Trainer Rating wild encounter scaling](../prds/trainer-rating-wild-encounter-scaling.md)
Implemented: Yes

## Scope

This specification defines the persistent Trainer Rating, its build-specific progression sources, and the effective ordinary wild population derived from it. It covers the ordinary encounter profiles compiled into standalone Emerald, FireRed/LeafGreen, and HNS. It does not define new regional content or trainer battle scaling.

Wayfarer replaces the saved Trainer Rating with Trainer Experience and Trainer
Level. The [Trainer Level progression](trainer-level-progression.md)
specification owns its progression lifecycle and adapts this specification's
ordinary-wild projection through a transient projection coordinate.

## Behavior

### Trainer Rating lifecycle

Trainer Rating is an inclusive value from 10 to 80. A new game starts at 10. The saved value is clamped to that range whenever it is read.

The game derives a rating from current progression facts, compares it with the saved value, and stores the higher value. This is a high-water mark: progression can increase the rating but no later read can reduce it. Save migration initializes the stored value from the derived rating, also within the 10 to 80 range, without changing save-block layouts.

The rating is not shown in the player interface.

### Progression targets

Each of the first four badges contributes 4 points, and each of the next four contributes 6. The rating floor keeps the first two badge totals at 10. The shared badge contribution reaches Rating 16 after four badges and Rating 40 after eight badges. The first League clear raises the rating to 55. The current substantial postgame target is 65.

| Build | First League | Postgame condition for Rating 65 |
| --- | --- | --- |
| Emerald | Champion | Birch's post-Champion National Dex upgrade |
| FireRed/LeafGreen | Hall of Fame clear | Sapphire recovered |
| HNS | Johto Champion | All eight Kanto badges and Kanto Champion |

HNS awards one additional point for each Kanto badge and two for the Kanto Champion. No current progression source reaches the Rating 80 cap.

### Eligible profiles

The system resolves ordinary land, water, Rock Smash, and fishing profiles from the active build's wild encounter headers. Fishing keeps its Old, Good, and Super Rod partitions. Time-of-day variants remain distinct profiles.

The active coverage is:

- Emerald: Hoenn.
- FireRed/LeafGreen: Kanto and the Sevii Islands.
- HNS: Johto, Kanto, Alola, Sinjoh, Faraway Island, and Southern Island.

Sinjoh is distinct from Sinnoh. None of the current builds contains a full Sinnoh encounter region.

Adding a map connection or warp to HNS does not make a new region eligible. Its ordinary profiles must explicitly target the HNS build. Once that data is compiled into HNS, the existing global Trainer Rating pipeline applies without a new scaling implementation. Any Hoenn badges or story milestones require a separate progression design before they can affect Trainer Rating.

### Effective ordinary population

For a selected authored slot, the game first determines the authored level using the existing encounter rules. It then projects that level through the generated scaling curve for the current Trainer Rating. The projection uses the cumulative highest result through the current rating, so an increased Trainer Rating cannot reduce the projected level. Results are clamped to valid wild levels.

The selected slot and its authored source remain the basis for ordinary encounter selection. Existing selection mechanics stay intact, including encounter weights, rod partitions, time of day, ability effects, lures, Altering Cave, and HNS Hoenn Sound. Pressure, Vital Spirit, and lure-level effects choose the authored level before projection.

If a non-randomized projected encounter would be below a numeric level-evolution threshold, its species resolves through the appropriate predecessor chain until the projected level supports the stage. This rule depends on the projected level, not on whether the source table authored the evolved species below its threshold. It applies only to ordinary populations; fixed and scripted encounters remain excluded.

Global species floors apply to every non-randomized ordinary wild population after predecessor resolution. A slot is ineligible if one of its possible projected outcomes has a resulting species below that species' floor. Ordinary selection excludes ineligible slots, sums the remaining authored weights, and rolls within that total without rewriting the source table.

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
| Mantine | 35 | LeafGreen, Seven Island Trainer Tower, water/surf, 35-40 |
| Bagon | 20 | Emerald, Meteor Falls B1F 2R, land grass, 25-35 |
| Tropius | 20 | Emerald, Route 119, land grass, 25-27 |
| Absol | 20 | Emerald, Route 120, land grass, 25-27 |
| Heracross | 20 | Emerald, Safari Zone North, land grass, 27-29 |

Each example identifies an ordinary encounter that motivated review of the global floor. It does not limit the floor to that map, method, build, or level range.

In wild-randomizer mode, the existing randomized species mapping continues to run from the original selected slot. Its level still projects through Trainer Rating, but predecessor resolution and species-floor eligibility filtering are bypassed.

### Consumers of the effective population

All ordinary consumers resolve the same effective population:

- Regular land, water, Rock Smash, and fishing encounters.
- Pokédex area checks.
- Match Call and radio species selection.
- Local ambient species selection.
- Ordinary land and water DexNav populations.

Ordinary DexNav selection preserves its ordinary source and level-range weighting, including fallback lure mirroring. Hidden DexNav entries remain authored and unscaled.

### Excluded sources

The following retain their authored behavior and do not use Trainer Rating scaling:

- Hidden DexNav encounters.
- Fixed and scripted encounters.
- Roamers and outbreaks.
- Feebas.
- Battle Pike encounters.
- Battle Pyramid encounters.

### Validation

Validation must include deterministic checks for Trainer Rating progression, save migration, level projection, predecessor resolution, species floors, eligible-weight selection, excluded sources, and ordinary population consumers. Generated encounter data must reproduce authored profiles before scaling and produce a balance audit for every covered build.

Compile the affected encounter objects for Emerald, FireRed, LeafGreen, and HNS. Build at least one complete release ROM after generation, then playtest the progression milestones and ordinary encounter mechanics described in the parent PRD.

## References

- [Wayfarer Trainer Level progression](trainer-level-progression.md)
- [Implementation pull request](https://github.com/mzpkdev/pokemon-wayfarer/pull/13)
