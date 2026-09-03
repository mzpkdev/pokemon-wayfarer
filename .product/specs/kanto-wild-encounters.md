# Kanto wild encounters

PRD: [Kanto wild encounters](../prds/kanto-wild-encounters.md)
Implemented: Yes

## Scope

This specification defines the HNS mainland Kanto ordinary wild encounter
portfolio, its FireRed and LeafGreen source provenance, authored day and night
behavior, and the deterministic checks that keep it suitable as a starting
region. It covers land, Surf, the shared Rock Smash and Headbutt interaction
data, and all ten Standard Rod entries.

It does not redesign encounter runtime selection, Trainer Rating, rod quality,
special acquisitions, terrain, or region access. Sevii remains out of scope for
this implementation. Its existing encounter records must remain unchanged.

## Behavior

### Authored sources and generated policy

`game/src/data/wild_encounters.json` remains the only authored source for
species, level ranges, encounter rates, and active slot order. Do not copy the
target encounter tables into another editable file.

The encounter author chooses the exact target species assignments under the
closed rules and numeric bounds below. Once committed, those assignments are
the implemented behavior. They do not require a second mirrored roster in this
document or another product decision.

Add `game/src/data/wild_encounter_regions.json` as the authored source for
regional ownership and provenance. The wild encounter generator consumes it
when validating `wild_encounters.json` and producing the balance audit. The
file has exactly `schemaVersion` and `regions` at the top level.
`schemaVersion` is 1. `regions` has exactly `KANTO` and `JOHTO`. The `KANTO`
object has exactly `product`, `profiles`, and `changes`. The `JOHTO` object's
exact fields are defined by its own specification.

Every Kanto `profiles` record has these fields:

- `map`, the exact HNS map constant.
- `method`, one of `land_mons`, `water_mons`, `rock_smash_mons`, or
  `fishing_mons`.
- `dayBaseLabel` and `nightBaseLabel`, the resolved target identities.
- `nightMode`, either `AUTHORED` or `DAY_ALIAS`.
- `activeSlotCount`, equal to 12, 5, 5, or 10 for the four methods.
- `sourceKind`, either `DIRECT`, `EQUIVALENT`, or `ANALOG`.
- `fireRedSource` and `leafGreenSource`, each an array of one or more exact
  source base labels.
- `habitat`, one of `ROUTE_GRASS`, `FOREST`, `CAVE`, `MOUNTAIN`, `URBAN_EDGE`,
  `POND`, `COAST`, `OFFSHORE`, `FACILITY`, or `SAFARI`.

Every active Kanto slot has one matching provenance record nested under its
profile. A provenance record contains `targetTime`, `targetSlot`,
`ecologySourceGroup`, `levelSource`, and `reason`. `ecologySourceGroup`
identifies the source method and the arrays of grouped numeric slots in each
contributing version. `levelSource` identifies one version, base label, method,
slot, minimum level, and maximum level. `reason` is one of `FRLG_SHARED`,
`FRLG_VERSION_COUNTERPART`, `FRLG_DUPLICATE`, `GEN2_LOCAL_ADDITION`,
`LATER_FAMILY_CONTINUITY`, or `NIGHT_REWEIGHT`.

`FRLG_DUPLICATE` marks a surplus copy from a shared FRLG ecology group, where
FireRed and LeafGreen have the same species and the selected assignment already
retains that species in another target slot. The counterpart solver leaves the
surplus slot in its unassigned source-group state. The record keeps the shared
group and level range for traceability, but the slot does not count as another
selected group allocation in the exhaustive proof. A live slot containing a
species from a differing counterpart group must be mapped to that group and
counted in its probability budget. `FRLG_DUPLICATE` does not permit a new
species, a level change, or an omission that the solver did not select.

The generator rejects duplicate profile identities, duplicate target slots,
missing active slots, unknown source labels, source and target method
mismatches, incorrect active counts, or provenance whose recorded species or
levels do not match `wild_encounters.json`. An `AUTHORED` night label must exist
in that file. A `DAY_ALIAS` night label must be unique and absent from it because
the generator creates that binding. The manifest is policy and traceability
data. It does not change runtime selection.

`levelSource` must name a slot inside its `ecologySourceGroup`. For a copied
FireRed or LeafGreen species, that exact source slot must author the same
species. Additions may inherit either version's slot range under the level
selection rule below.

Each `changes` record identifies a target profile, method, time, slot, before
species, after species, change kind, and reason. Change kind is `MERGE`,
`REWEIGHT`, `ADDITION`, `FORBIDDEN_REMOVAL`, or `NIGHT_AUTHORING`. A reason must
name the habitat or source role that makes the change valid. Generated reports
show these records, but `changes` are not compiled into the ROM.

### Mainland ownership

The Kanto manifest contains these 53 map IDs and no others:

| Group | Map IDs |
| --- | --- |
| Routes | `MAP_ROUTE1_HNS` through `MAP_ROUTE25_HNS` |
| Cities and ports | `MAP_PALLET_TOWN_HNS`, `MAP_VIRIDIAN_CITY_HNS`, `MAP_PEWTER_CITY_HNS`, `MAP_CERULEAN_CITY_HNS`, `MAP_LAVENDER_TOWN_HNS`, `MAP_VERMILION_CITY_HNS`, `MAP_VERMILION_CITY_PORT_OUTSIDE_HNS`, `MAP_CELADON_CITY_HNS`, `MAP_FUCHSIA_CITY_HNS`, `MAP_CINNABAR_ISLAND_HNS`, `MAP_SAFFRON_CITY_HNS` |
| Safari | `MAP_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS`, `MAP_FUCHSIA_CITY_SAFARI_ZONE_BRUSH_HNS`, `MAP_FUCHSIA_CITY_SAFARI_ZONE_CAVE_HNS`, `MAP_FUCHSIA_CITY_SAFARI_ZONE_MOUNTAIN_HNS` |
| Forests and caves | `MAP_VIRIDIAN_FOREST_HNS`, `MAP_MT_MOON_CAVE_HNS`, `MAP_DIGLETTS_CAVE_TUNNEL_HNS`, `MAP_ROCK_TUNNEL_1F_HNS`, `MAP_ROCK_TUNNEL_B1F_HNS`, `MAP_SEAFOAM_ISLANDS_1F_HNS`, `MAP_SEAFOAM_ISLANDS_B1F_HNS`, `MAP_CERULEAN_CAVE_1F_HNS`, `MAP_CERULEAN_CAVE_B1F_HNS`, `MAP_CERULEAN_CAVE_B2F_HNS` |
| League caves | `MAP_VICTORY_ROAD_KANTO_1F_HNS`, `MAP_VICTORY_ROAD_KANTO_B1F_HNS`, `MAP_VICTORY_ROAD_KANTO_B2F_HNS` |

The route range in the first row includes Routes 1 through 25. This makes
`MAP_ROUTE23_HNS` an active Kanto map instead of a reservation. Add
`gRoute23_hns_Day` and `gRoute23_hns_Night` with land, Surf, and fishing data.
Their encounter rates are 21, 2, and 20 respectively, copied from the FireRed
and LeafGreen Route 23 sources. Route 23 has no interaction profile.

The resolved mainland topology is 129 profiles at day and 129 at night: 41
land, 31 Surf, 25 interaction, and 32 fishing profiles per time. A profile is a
map and method pair. `DAY_ALIAS` still contributes a night profile because the
manifest binds it explicitly. Automatic runtime fallback does not.

The original 52 maps retain their current nonzero encounter rates. Adding or
removing a method, changing one of those rates, or turning a zero-rate source
row into an active profile is outside this specification. Route 23 is the only
topology and encounter-rate addition.

Routes 26 through 28, Tohjo Falls, Mt. Silver, the Johto Rocket Hideout, Sinjoh,
Alola, event islands, and every Sevii map are rejected from the Kanto manifest.
The three Kanto Victory Road maps remain Kanto-owned regardless of map section.
`MAP_ROUTE10_POWER_PLANT_ENTRANCE_HNS` and
`MAP_ROUTE10_POWER_PLANT_BACK_ROOM_HNS` have no ordinary encounter profiles and
remain encounter-free. Use `MAP_ROUTE10_HNS` as the representative Power Plant
approach for ecology inspection and playtesting.

### FireRed and LeafGreen source mapping

A source role is one active numeric slot under the production weights for its
method. FireRed and LeafGreen slots with the same method and index form one
paired role. Source rows beyond the active runtime count are never roles.

For ecology budgeting, combine every paired role in the same source profile
whose ordered `(FireRed species, LeafGreen species)` pair is identical. This is
one ecology source group, and its budget is the sum of all grouped role weights.
The group retains the ordered arrays of source slot indices. Level provenance
does not aggregate: every target slot still selects one exact source slot from
its ecology source group as `levelSource`.

Use the same numbered FireRed and LeafGreen route, city, forest, or cave as the
`DIRECT` source when that source has the target method. The following target
maps use named `EQUIVALENT` sources:

| HNS target | FireRed and LeafGreen source |
| --- | --- |
| Route 21 | `sRoute21North_FireRed` and `sRoute21North_LeafGreen`; the generator also confirms the corresponding South profiles are identical |
| Vermilion port outside | `sSSAnneExterior_FireRed` and `sSSAnneExterior_LeafGreen` |
| Safari beach | `sSafariZoneCenter_FireRed` and `sSafariZoneCenter_LeafGreen` |
| Safari brush | `sSafariZoneEast_FireRed` and `sSafariZoneEast_LeafGreen` |
| Safari cave | `sSafariZoneWest_FireRed` and `sSafariZoneWest_LeafGreen` |
| Safari mountain | `sSafariZoneNorth_FireRed` and `sSafariZoneNorth_LeafGreen` |
| Mt. Moon cave | `sMtMoon1F_FireRed` and `sMtMoon1F_LeafGreen` |
| Diglett's Cave tunnel | `sDiglettsCaveB1F_FireRed` and `sDiglettsCaveB1F_LeafGreen` |
| Cerulean Cave 1F | `sCeruleanCave1F_FireRed` and `sCeruleanCave1F_LeafGreen` |
| Cerulean Cave B1F | `sCeruleanCave2F_FireRed` and `sCeruleanCave2F_LeafGreen` |
| Cerulean Cave B2F | `sCeruleanCaveB1F_FireRed` and `sCeruleanCaveB1F_LeafGreen` |
| Kanto Victory Road 1F | `sVictoryRoad1F_FireRed` and `sVictoryRoad1F_LeafGreen` |
| Kanto Victory Road B1F | `sVictoryRoad2F_FireRed` and `sVictoryRoad2F_LeafGreen` |
| Kanto Victory Road B2F | `sVictoryRoad3F_FireRed` and `sVictoryRoad3F_LeafGreen` |

When one of those sources lacks an active target method, use these `ANALOG`
sources. This table is exhaustive, so the implementation does not choose an
analog by proximity at generation time.

| Target maps and methods | FireRed and LeafGreen analog |
| --- | --- |
| Route 1 Surf and fishing | `sPalletTown_FireRed` and `sPalletTown_LeafGreen`, same method |
| Route 2 Surf and fishing | `sViridianCity_FireRed` and `sViridianCity_LeafGreen`, same method |
| Route 9 Surf and fishing | `sRoute10_FireRed` and `sRoute10_LeafGreen`, same method |
| Routes 14 and 15 Surf and fishing | `sFuchsiaCity_FireRed` and `sFuchsiaCity_LeafGreen`, same method |
| Kanto Victory Road 1F and B1F Surf or fishing without a floor source | `sCeruleanCave1F_FireRed` and `sCeruleanCave1F_LeafGreen`, same method |
| Cinnabar land | `sRoute21North_FireRed` and `sRoute21North_LeafGreen` land |
| Every active mainland interaction profile without a direct FRLG interaction source | `sRockTunnelB1F_FireRed` and `sRockTunnelB1F_LeafGreen` Rock Smash |

An active method not covered by a direct, equivalent, or analog rule is a
generation error. Analog selection is never inferred from label sorting.

### Daytime merge

Build each daytime profile from the ecology source groups under the target
method's production weights. The active weights are 20, 20, 10, 10, 10, 10, 5,
5, 4, 4, 1, and 1 for land; 60, 30, 5, 4, and 1 for Surf and interaction; and
the three Standard Rod vectors for fishing.

When both versions author the same species in an ecology source group, preserve
that species as the group's anchor. When they differ, both species must remain
in the target profile. Aggregate all target slots mapped to that group before
checking its budget. Route 23 Surf therefore forms one group containing source
slots 0 through 4: FireRed contributes Psyduck and LeafGreen contributes
Slowpoke. Its five target slots divide the complete group budget between those
two species.

For different counterpart species, neither target probability may exceed
twice the other. Their combined target probability must differ from the mean
combined FireRed and LeafGreen source probability by no more than the greater
of 2 percentage points or 20 percent of that source mean. Fishing applies both
checks independently under Old, Good, and Super Rod weights.

First enumerate assignments that retain every distinct FireRed and LeafGreen
source species and both members of every differing counterpart group. If at
least one full-retention assignment satisfies the fixed production slot count,
method or rod weights, and every combined probability-budget tolerance, an
omission is a generation error.

An omission is allowed only when exhaustive enumeration proves that no
full-retention assignment satisfies those fixed constraints. Enumerate the
valid reduced assignments without changing the production slot count, weights,
or budget tolerances. Select the assignment that retains the greatest number
of distinct FireRed and LeafGreen source species, then the one with the
smallest combined probability-budget error, then the smallest absolute
counterpart difference, then the lexicographically smallest sequence of target
slot indices, then the lexicographically smallest sequence of species
constants. The schema-version-3 balance audit names every omitted counterpart and
includes a deterministic, machine-verifiable certificate of the exhaustive
search. The certificate records the solver version and canonical ordering,
then derives the complete candidate space only from the frozen FireRed and
LeafGreen source profiles, every production target slot and weight, and
explicitly named protected constraints. The source-group allocation domain
covers every canonical source ecology group and member across every target
slot, plus an unassigned source-group state for slots used by duplicates or
permitted additions. Independently justified fixed bindings may come only from
those frozen inputs and named constraints. No fixed binding or domain
restriction may derive from the proposed final manifest, its provenance, or
the selected assignment. The certificate records exact full-retention and
reduced candidate counts, exact rejection counts, the canonical enumeration
digest, the selected assignment, and its exact objective values under the
ordered selection rules. It also records enough reachable states and
transitions or equivalent dynamic-programming classes for an independent
implementation to recompute the counts, verify that no valid full-retention
assignment exists, and reproduce the selected winner without enumerating every
leaf into the audit file. A full-retention witness is sufficient when no
omission is claimed. A summarized infeasibility claim without the certificate
is not proof. This is not a general waiver for a profile where full retention
is feasible.

When full retention can meet every combined-budget tolerance but no retained
assignment satisfies the two-to-one counterpart ratio, choose the retained
assignment with the smallest combined-budget error, then the smallest absolute
counterpart difference, followed by the same slot-index and species-constant
tie-breaks. Record the same exhaustive-search certificate and the selected
assignment's exact objective values in the schema-version-3 balance audit. Only
this proved case may exceed the two-to-one counterpart ratio.

Generation II additions may replace only a duplicate occurrence of a shared
FRLG species. They may not remove the last occurrence of any FRLG daytime
species in that profile, remove a version counterpart outside the proved
reduced-assignment exception above, or occupy a role needed for a native-HM or
Chinchou guarantee. Generation IV and later additions obey the same rule and
also require `LATER_FAMILY_CONTINUITY` provenance.

### Authored levels

A target species copied from FireRed or LeafGreen uses that source slot's exact
minimum and maximum. If both versions assign the same species to the paired
role with different ranges, choose one complete range by the following ordered
comparison:

1. Lower midpoint, compared as the integer sum of minimum and maximum.
2. Lower maximum.
3. FireRed when both values tie.

A Generation II addition, later-family addition, or night assignment inherits
the exact range of its mapped source role. It never imports the HNS Kanto range
that occupied the slot before this redesign. Every provenance record repeats
the selected source range, and generation fails on a mismatch. When an
addition's paired FireRed and LeafGreen roles have different ranges, apply the
same midpoint, maximum, and FireRed tie breaks above even though the added
species does not occur in either source.

Trainer Rating rolls within the authored range, projects the level, resolves a
supported predecessor, and applies species-floor eligibility as defined by the
Trainer Rating specification. The redesign adds no Kanto profile offset.

At Rating 10, effective land encounters on Routes 1, 2, 3, and 22, Viridian
Forest, and every manifest-owned Mt. Moon profile must be level 12 or lower.
The check enumerates every authored level in every slot, including ability and
Lure level selection inputs before projection.

### Night authoring

Every day profile has an explicit night binding. `AUTHORED` points to a
terminal `_Night` encounter row in `wild_encounters.json`. `DAY_ALIAS` points
to the day data through generated night header data and does not rely on
`OW_TIME_OF_DAY_FALLBACK`. Morning and evening keep their existing configured
fallback behavior.

For one profile, let `D(s)` and `N(s)` be the normalized selectable probability
of species `s` by day and night. Duplicate slots are aggregated first. Fishing
calculates separate `D` and `N` values for each rod quality.

The shared species set must satisfy both `sum D(s) >= 0.70` and
`sum N(s) >= 0.70`. Land profiles marked `AUTHORED` are materially distinct
when `0.5 * sum |D(s) - N(s)| >= 0.10`. At least 25 of the 41 land profiles
must be materially distinct. `DAY_ALIAS` profiles count in the denominator but
not the numerator.

Night additions follow the daytime replacement restrictions. A night profile
may shift a species between source roles, but its slot keeps the mapped source
role's exact range. A repeated Johto species across unrelated habitat classes
requires a separate local reason for every placement.

### Generation portfolios

Classify each species by its own National Dex number. A regional or alternate
form uses the National Dex number of its base form. Generation I is 1 through
151, Generation II is 152 through 251, independent Generation III is 252
through 386, and Generation IV onward is 387 or greater.

An authored Wobbuffet or Marill family slot remains a Generation II family. If
production predecessor resolution yields Wynaut or Azurill, record that outcome
as a Generation II family extension. Neither baby is required to resolve, and
neither may be authored directly. Every other effective species numbered 252
through 386 is forbidden.

For each profile, remove `SPECIES_NONE` and slots locked by the current Trainer
Rating, renormalize the remaining production weights, aggregate by effective
species, and then by generation. Every map and method profile has equal weight
in the 129-profile regional mean. Encounter rates do not weight profiles. For a
given calculation, the selected rod quality supplies the weights for all 32
fishing profiles. Report land, Surf, interaction, and fishing breakouts as
diagnostics, but do not apply generation quotas to those breakouts. Ability
attraction, Lures, randomizer behavior, and Hoenn Sound are off for the
baseline portfolio.

Run the effective regional portfolio for every integer Rating from 10 through
80. Every rating and every rod quality must satisfy:

| Time | Generation I | Generation II families | Independent Generation III | Generation IV onward |
| --- | ---: | ---: | ---: | ---: |
| Day | 75% to 85% | 10% to 20% | 0% | At most 5% |
| Night | 60% to 75% | 20% to 35% | 0% | At most 5% |

At each rating, night Generation II probability is at least 5 percentage
points above day. Exact rational values decide pass or failure. Decimal display
rounding uses half-up rounding to two places.

The authored day and night union contains 105 through 120 distinct nonempty
species. This union uses active authored slots before Trainer Rating resolution
and counts each species once. Wynaut and Azurill do not enter the union unless
they are authored, which this specification forbids.

### Fishing and native Surf

All Kanto fishing profiles retain ten active entries and use the Standard Rod
weights. Species placement may change, but rod bite rates, eligibility,
renormalization, Lure mirroring, and the Feebas exception do not.

The following six HNS records remain in
`game/src/data/standard_rod_fishing.json`: Vermilion City, Vermilion port
outside, and Cinnabar Island by day and night. For each record, Chinchou is
exactly 11 percent of successful Old Rod encounters with Lure off at every
Rating from 10 through 80. The 25 percent bite rate makes it exactly 2.75
percent per unmodified cast. Kanto changes must not alter the Johto-owned
Olivine and Cianwood accessibility records in the same file.

Every qualifying Kanto Chinchou catch knows Surf. If the lowest effective level
falls below 20, lower the HNS Chinchou native-HM schedule to that level in both
learnset modes, preserve the move through level 100, and update the native-HM
specification and tests in the same implementation.

### Radio and ordinary population readers

With independent Generation III species absent, Hoenn Sound must produce the
same land and Surf distributions with the modifier on and off. Keep the station
as a music channel, but change `game/src/data/text/radio_strings.h` so it does
not promise nearby Hoenn encounters. The station name may remain Hoenn Sound.
No radio-only encounter pool is added.

Pokédex area checks, DexNav, ambient species, Match Call, Oak's Pokemon Talk,
and Cartographer must consume the same effective population as normal
encounters for the selected time, method, rod quality, and eligibility state.
Hidden DexNav entries and the randomizer retain their existing exclusions.

### Generated audit

Extend `game/tools/wild_encounters/wild_encounters_to_header.py` to consume the
regional manifest. Bump the balance audit to schema version 3. In addition to
the existing per-slot Trainer Rating data, it contains:

- The complete resolved Kanto ownership manifest and 129-profile denominator
  for each time.
- Authored and effective species probabilities as exact fractions, with the
  authored slot and resolved species both present for every outcome.
- Day and night generation portfolios for every Rating and rod quality.
- Per-profile shared-species retention and total-variation distance.
- FRLG ecology-group budgets, counterpart ratios, selected source level ranges,
  and every exhaustive discrete-slot proof in the schema-version-3 audit. Each
  proof names omitted counterparts and carries the deterministic compact search
  certificate defined above, including exact candidate and rejection counts,
  the enumeration digest, and the selected assignment's exact objective values.
- The authored species union, forbidden-species results, Hoenn Sound comparison,
  Chinchou accessibility, and Rating 10 opening-level checks.
- The manifest `changes` ledger and a list of every `DAY_ALIAS`.

Generation fails when any required report invariant fails. The report may
contain local habitat diagnostics, but no profile is removed from a portfolio
because it is an exception.

## Validation

Add regional-manifest and Kanto portfolio cases to
`game/tools/wild_encounters/tests/test_scaling.py`. Fixtures must cover direct,
equivalent, and analog mapping; a valid and invalid version-counterpart merge;
the discrete-slot tie breaks; source-range selection; explicit day aliases;
night retention and distance boundaries; generation classification; forbidden
authored and effective species; and deterministic report ordering.

Update existing HNS profile-count fixtures for Route 23 and the explicit night
bindings. Run `make wild-encounter-scaling-test` and
`make wild-encounter-balance-audit`. The generated audit must pass every Rating
from 10 through 80 and all three rod qualities.

Keep the production runtime tests for eligibility, predecessor resolution,
weighted selection, Lures, Hoenn Sound, randomizer handoff, Pokédex, DexNav,
and fishing passing. Add integration assertions for Hoenn Sound equality and
every Kanto Chinchou record. Run the native-HM test if the Chinchou schedule
changes. Run the devtools catalog tests and checks after the audit or
Cartographer schema changes.

Compile the affected encounter, radio, Pokédex, and DexNav objects for HNS,
then build one HNS release ROM. Playtest a new Kanto start through the first
badge at day and night, all three rod qualities, both Chinchou coasts, one
unchanged cave night, one materially changed route night, Safari Zone, Power
Plant approach on Route 10, and Route 23. Confirm special acquisitions and
Sevii encounters did not change.

## References

- [Johto wild encounter specification](johto-wild-encounters.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Standard Rod fishing](standard-rod-fishing.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Wild encounter generator](../../game/tools/wild_encounters/wild_encounters_to_header.py)
- [Wild encounter runtime](../../game/src/wild_encounter.c)
- [Radio runtime](../../game/src/pokenav_radio.c)
