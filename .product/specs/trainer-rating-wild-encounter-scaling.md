# Trainer Rating wild encounter scaling

For Wayfarer, the approved [Native HM catch windows](native-hm-catch-windows.md)
revision supersedes named native-utility carriers and permanent-retention
assumptions in this document. Its core implementation is complete, while full
route acceptance remains pending. Regional routes, field-use authorization,
scaling and unrelated mechanics remain unchanged; standalone builds retain
their existing contract.

PRD: [Trainer Rating wild encounter scaling](../prds/trainer-rating-wild-encounter-scaling.md)
Implemented: Partial; foundation and Today's circuit producer exist, broader acceptance remains pending.

## Scope

This specification defines persistent Trainer Rating (TR), build-specific
progression sources, and the effective ordinary wild population for Emerald,
FireRed, LeafGreen, and HNS. It does not define new regional content or Trainer
battle scaling. The Wayfarer level cap, experience reduction, and obedience
behavior belong to the separate TR party progression specification. Wayfarer
reuses the encounter projection but replaces the TR's progression inputs through
the interregional League circuit specification.

## Behavior

### Trainer Rating lifecycle

Wayfarer TR is an inclusive value from 0 through 80. A new Wayfarer game starts
at 0. Builds that have not adopted the Wayfarer circuit retain their existing TR
10 floor. Every read clamps the saved value to the range for the active product.

Today's [Wayfarer getter](../../game/src/trainer_rating.c) initializes a new
save to 0 and reads the implemented
[circuit producer](../../game/src/league_circuit.c). The producer derives a
candidate from global badges and the three canonical first-league-win facts,
contributing +8 once for each league. The getter compares that candidate with
the stored TR and saves the higher value, the saved high-water value. Replays
add no TR, and no later read lowers it. Feature-disabled foundation checks can
still seed a stored value; no prerelease save migration is required.

The v0 TR design (rescaled formula, uncapped TR, scalers, and notable
trainers' separate TR) is in [Player Trainer Rating](player-trainer-rating.md);
this document owns the v0 wild level curve in
[v0 wild level curve](#v0-wild-level-curve). Everything else here is Today.

### Progression targets

The TR foundation persists and exposes the shared player TR. Today's circuit
maps global badge count and canonical first league wins to that value. It starts
at 0, reaches 16 after four badges and 40 after eight badges, and reaches 80
after all twenty-four badges and all three first league wins. Circuit starts and
opponent party tiers are not prerequisites for the foundation, its 0 through 80
bounds, or its consumers.

Builds that have not adopted the Wayfarer circuit retain their current
progression. Each of the first four regional badges contributes 4 points, and
each of the next four contributes 6. Their TR 10 floor keeps the first two
badge totals at 10. The first League win raises TR to 55. Their
current substantial postgame target is 65.

| Build | First League | Postgame condition for TR 65 |
| --- | --- | --- |
| Emerald | Champion | Birch's post-Champion National Dex upgrade |
| FireRed/LeafGreen | Hall of Fame clear | Sapphire recovered |
| HNS | Johto Champion | All eight Kanto badges and Kanto Champion |

HNS awards one additional point for each Kanto badge and two for the Kanto
Champion. No progression source outside the Wayfarer circuit currently reaches
the TR 80 cap.

### Eligible profiles

The system resolves ordinary land, water, Rock Smash, and fishing profiles from the active build's wild encounter headers. Fishing keeps its Old, Good, and Super Rod partitions. Time-of-day variants remain distinct profiles.

The active coverage is:

- Emerald: Hoenn.
- FireRed/LeafGreen: Kanto and the Sevii Islands.
- HNS: Johto, Kanto, Alola, Sinjoh, Faraway Island, and Southern Island.

Sinjoh is distinct from Sinnoh. None of the current builds contains a full Sinnoh encounter region.

Adding a map connection or warp to HNS does not make a new region eligible. Its
ordinary profiles must explicitly target the HNS build. Once that data is
compiled into HNS, the existing global TR pipeline applies without a new scaling
implementation. In Wayfarer, Hoenn badges and League completion enter TR through
the interregional circuit rather than this build-specific table.

### Effective ordinary population

For a selected authored slot, the game first determines the authored level using
the existing encounter rules. It then projects that level through the generated
scaling curve for the current TR. The projection uses the cumulative highest
result through the current TR, so an increased TR cannot reduce the projected
level. Results are clamped to valid wild levels.

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
| Mantine | 14 | HNS, Whirl Islands, water/surf, authored 15-19 and projected 14 at TR 10 |
| Bagon | 20 | Emerald, Meteor Falls B1F 2R, land grass, 25-35 |
| Tropius | 20 | Emerald, Route 119, land grass, 25-27 |
| Absol | 20 | Emerald, Route 120, land grass, 25-27 |
| Heracross | 20 | Emerald, Safari Zone North, land grass, 27-29 |

Each example identifies an ordinary encounter that motivated review of the global floor. It does not limit the floor to that map, method, build, or level range.

Mantine's global floor is exactly 14. An authored level-15 Mantine projects to
level 14 at TR 10, so any higher floor would make a protected HNS native-HM
source ineligible. This correction does not create a regional runtime branch,
change authored levels, or add Mantyke or another predecessor rule. Mantine is
protected from TR 10 through 80; it is not an all-TR traversal anchor, and
lower TR may exclude it because of the global floor. HNS Chinchou is the
approved TR 0 through 80 Whirlpool source.

In wild-randomizer mode, the existing randomized species mapping continues to
run from the original selected slot. Its level still projects through TR, but
predecessor resolution and species-floor eligibility filtering are bypassed.

### v0 wild level curve

On the [v0 TR scale](player-trainer-rating.md#formula-v0) the
projection follows the wild level curve below instead of today's anchors (the
level cap minus 10). Anchors are placeholders:

| Badges | v0 TR | Wild level curve | Level cap | Gap to cap |
| ---: | ---: | ---: | ---: | ---: |
| 0 | 0 | 6 | 15 | −9 |
| 4 | 40 | 24 | 28 | −4 |
| 8 | 80 | 40 | 50 | −10 |
| 16 | 120 | 58 | 75 | −17 |
| 24 | 160 | 78 | 100 | −22 |

The curve interpolates as a [scaler](player-trainer-rating.md#scalers) and
stays flat past TR 160. Early wild Pokémon press close to the cap; late ones
fall well behind it. The projection mechanics are unchanged: authored level
projected through the curve, cumulative maximum, evolution downshift, and
eligible-weight selection. Species floors are levels, so their values are
unaffected.

The Mantine and Chinchou protection notes above are stated on today's scale
(Mantine from TR 10 through 80, Chinchou from TR 0 through 80). By badge
equivalence the v0 ranges are TR 25 through 160 for Mantine (today's TR 10 ≈
2.5 badges) and TR 0 through 160 for Chinchou (0 to 24 badges). Both need data
re-verification against the new curve before adoption, including whether 14 is
still the right Mantine floor; this document does not yet claim either
protection on the v0 scale.

### Consumers of the effective population

All ordinary consumers resolve the same effective population:

- Regular land, water, Rock Smash, and fishing encounters.
- Pokédex area checks.
- Match Call and radio species selection.
- Local ambient species selection.
- Ordinary land and water DexNav populations.

Ordinary DexNav selection preserves its ordinary source and level-range
weighting, including fallback lure mirroring. Hidden DexNav entries remain
authored and unscaled.

### Excluded sources

The following retain their authored behavior and do not use TR scaling:

- Hidden DexNav encounters.
- Fixed and scripted encounters.
- Roamers and outbreaks.
- Feebas.
- Battle Pike encounters.
- Battle Pyramid encounters.

### Validation

Validation must include deterministic checks for TR progression, save migration,
level projection, predecessor resolution, species floors, eligible-weight
selection, excluded sources, and ordinary population consumers. Generated
encounter data must reproduce authored profiles before scaling and produce a
balance audit for every covered build.

Compile the affected encounter objects for Emerald, FireRed, LeafGreen, and HNS. Build at least one complete release ROM after generation, then playtest the progression milestones and ordinary encounter mechanics described in the parent PRD.

## References

- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Trainer Rating party progression](trainer-rating-party-progression.md)
- [Implementation pull request](https://github.com/mzpkdev/pokemon-wayfarer/pull/13)
