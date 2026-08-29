# Trainer Rating wild encounter scaling

PRD: [Trainer Rating wild encounter scaling](../prds/trainer-rating-wild-encounter-scaling.md)
Implemented: No

## Scope

This specification defines the persistent Trainer Rating, its build-specific progression sources, and the effective ordinary wild population derived from it. It covers the ordinary encounter profiles compiled into Emerald, FireRed/LeafGreen, and the current HNS regions. It does not define new regional content or trainer battle scaling.

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

- Emerald: all ordinary profiles compiled into the ROM.
- FireRed/LeafGreen: Kanto and the Sevii Islands.
- HNS: Johto, Kanto, and all currently compiled miscellaneous regions.

Adding a map connection or warp to HNS does not make a new region eligible. Its ordinary profiles must explicitly target the HNS build. Once that data is compiled into HNS, the existing global Trainer Rating pipeline applies without a new scaling implementation. Any Hoenn badges or story milestones require a separate progression design before they can affect Trainer Rating.

### Effective ordinary population

For a selected authored slot, the game first determines the authored level using the existing encounter rules. It then projects that level through the generated scaling curve for the current Trainer Rating. The projection uses the cumulative highest result through the current rating, so an increased Trainer Rating cannot reduce the projected level. Results are clamped to valid wild levels.

The selected slot and its authored source remain the basis for ordinary encounter selection. Existing selection mechanics stay intact, including encounter weights, rod partitions, time of day, ability effects, lures, Altering Cave, and HNS Hoenn Sound. Pressure, Vital Spirit, and lure-level effects choose the authored level before projection.

If a non-randomized projected encounter would be below a numeric level-evolution threshold, its species resolves through the appropriate predecessor chain. The initial explicit floors require Kecleon to be at least level 20 and Skarmory to be at least level 18. A floor can mark a slot ineligible at the resulting level. Ordinary selection excludes ineligible slots, sums the remaining authored weights, and rolls within that total without rewriting the source table.

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

## Open questions

- The encounter-table audit recommends evaluating explicit floors of 20 for Aerodactyl, 15 for Heracross, and 25 for Bagon. These are not part of the implemented floor set until the balance policy is approved.

## References

- [Implementation pull request](https://github.com/mzpkdev/pokemon-wayfarer/pull/13)
