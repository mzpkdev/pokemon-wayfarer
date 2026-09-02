# Kanto wild encounters

## Intent

Make Kanto feel like the home of Generation I while letting nearby Johto leave a
restrained mark on its ecology. Daytime encounters should remain recognizably
FireRed and LeafGreen. Night should change what the player notices without
turning each map into a separate regional Pokédex. Ordinary discovery for
independent Generation III families belongs to Hoenn and is removed from Kanto.

Kanto is a valid starting region, not postgame-only content. On a new save, its
authored levels must support an ordinary FireRed and LeafGreen-style opening.
Species placement communicates habitat and regional identity, while Trainer
Rating keeps encounters relevant later in the open-world campaign.

## Design

FireRed and LeafGreen together are the authority for mainland Kanto's daytime
species ecology and authored level ranges. Where both versions agree, the
combined profile preserves that habitat, relative rarity, and level role. Where
they differ, both version assignments enter the combined population at
comparable rarity. The merge does not concatenate the source arrays or double
the probability assigned to version-exclusive roles.

For example, if FireRed and LeafGreen put different species in the same rare
role, the combined profile gives both species a rare role within the target
method's existing probability budget. It does not append one version's whole
table after the other or make both species as common as the shared slot was by
itself.

Comparable rarity is quantitative. Recalculate each source version with the
target method weights, including each Standard Rod quality for fishing, and
aggregate duplicate slots by species. For counterpart species in one source
role, neither target probability may exceed twice the other. Their combined
target probability must remain within the greater of 2 percentage points or 20
percent of the mean FireRed and LeafGreen source-role probability. A documented
habitat or traversal exception may override the counterpart ratio, but not the
combined probability budget.

When the method's discrete active weights cannot satisfy both rules, preserve
the combined probability budget first. Choose the feasible slot assignment
with the smallest combined-budget error, then the smallest counterpart
imbalance, and include that calculation in the merge report. This discrete-slot
exception may waive the two-to-one ratio only after the report proves that no
active assignment can satisfy the combined-budget tolerance and the ratio at
the same time. The implementation specification must also map
every target profile and method to its FireRed and LeafGreen source profiles so
the comparison set cannot change during acceptance.

Every daytime entry with a direct source counterpart inherits that counterpart's
FireRed or LeafGreen minimum and maximum level. When both versions contribute
different species, each target species retains the range of its own source
role. When the same species occupies the same rarity role with different ranges
between versions, use the complete range from the lower-level source: first
compare midpoint, then maximum level, then choose FireRed if both tie. This
preserves one real source range and favors the starting-region curve over a
synthetic envelope. A Generation II addition inherits the exact level range of
the FireRed or LeafGreen rarity role it occupies rather than importing a Johto
or HNS Kanto level.

A night-only assignment inherits the level range of its mapped daytime rarity
role. A target map and method without a direct source counterpart must use a
documented FireRed or LeafGreen Kanto analog with the same method and closest
habitat. Every slot in that target profile must map to an analog source rarity
role and inherit that role's exact minimum and maximum under the same
lower-level version rule. Sevii profiles use their FireRed and LeafGreen Sevii
sources. These authored ranges are inputs to Trainer Rating scaling, so
effective levels still rise with campaign progress.

Kanto uses these priorities in order:

1. The species must suit the map, terrain, and encounter method.
2. The daytime population should preserve the location's FireRed and LeafGreen
   identity.
3. Generation I should dominate mainland Kanto as a region.
4. Generation II additions should give night and selected habitats their own
   character without displacing the location's defining species.
5. Species from independent Generation III families do not appear in ordinary
   Kanto encounters. Wynaut and Azurill are allowed only as Generation II
   family extensions produced by predecessor resolution.
6. Generation IV and later species remain exceptional and must have a specific
   habitat or family-continuity reason.

Habitat wins when a generation target conflicts with a convincing local
population. Power Plant does not need a Generation II species merely to satisfy
a quota, and a nocturnal cave may exceed the regional Generation II share when
its residents support that choice. The regional report must make such local
exceptions visible.

### Day and night

Every mainland Kanto profile with an ordinary daytime method has a defined night
result. A method may explicitly reuse its daytime population when water,
geology, or another habitat has no useful time distinction. An automatic runtime
fallback does not count as an authored result. Land encounters should provide
most of the noticeable time-based changes.

For each map and method, species available at night must include daytime species
whose combined daytime selection probability is at least 70 percent. Duplicate
slots for one species are combined before this check. The retained species may
receive different weights at night, but species shared with day must also
account for at least 70 percent of the night selection probability.

At least 60 percent of mainland land profiles must have a distinct night
distribution. A night distribution is distinct when the sum of its absolute
species-probability changes from day, divided by two, is at least 10 percentage
points. Explicit day aliases, caves without a useful time distinction, and
other justified unchanged populations remain valid within the other 40
percent.

Generation II species may become locally common where time and habitat justify
it, such as nocturnal birds, insects, and cave residents. They should remain
localized. Repeating the same Johto addition across unrelated Kanto routes does
not satisfy this design.

### Encounter methods

The HNS map and its usable terrain decide which ordinary methods exist on a map.
The FRLG source decides the Generation I anchors and rarity roles where a
comparable method exists. Restrained Generation II additions follow the regional
targets and habitat rules above. A source table does not authorize Surf, Rock
Smash, or fishing on a map that cannot support that method, and the absence of a
source method does not remove a useful method already supported by the HNS map.

- Land populations carry the strongest FRLG identity and most authored night
  changes.
- Surf populations represent visible local water habitat. They should not be a
  generic regional water list copied between every coast and pond.
- Rock Smash populations follow local geology and accessible interaction
  points. Tree or insect species belong there only when that interaction is the
  intended habitat in the target map.
- Fishing populations use all ten authored entries as one Standard Rod
  population. Common, less common, and rare roles follow the Standard Rod
  weights rather than the old rod partitions.

Encounter cadence is separate from species distribution. This PRD does not use
higher step, Surf, Rock Smash, bite, or cast frequency to make a rare species
appear common enough on paper.

### Regional transitions

Route 22, Route 23, and the Kanto side of the League approach may mix Kanto and
Johto more strongly than central Kanto. Route 26, Route 27, Route 28, Tohjo
Falls, and Mt. Silver belong to the Johto encounter PRD, which treats them as a
shared transition ecosystem. The two documents should agree on adjacent
species, but only one owns each encounter profile.

Sevii is broader, later ecology. Its islands may retain or introduce
Generation II, Generation IV, or later species that would be too frequent or
too far from their source habitat on mainland Kanto. Independent Generation III
families remain excluded so Hoenn owns their ordinary discovery. Mainland
generation bands do not apply to Sevii, but each island still needs a coherent
habitat and rarity structure. Sevii must not become an unstructured catch-all
Pokédex.

Before an implementation edits or validates Sevii, its specification must
freeze a separate Sevii map and profile manifest. If an implementation does not
include Sevii, it must leave every Sevii profile unchanged. The ecological and
Generation III family direction above remains binding for its future PRD.

### Profile ownership

Mainland ownership is based on this manifest rather than region-map-section
ranges alone:

- Routes: `MAP_ROUTE1_HNS` through `MAP_ROUTE22_HNS`, plus
  `MAP_ROUTE24_HNS` and `MAP_ROUTE25_HNS`.
- Cities and surface subareas: Pallet Town, Viridian City, Pewter City,
  Cerulean City, Lavender Town, Vermilion City, Vermilion port outside,
  Celadon City, Fuchsia City, Cinnabar Island, and Saffron City.
- Safari subareas: beach, brush, cave, and mountain.
- Forests and caves: Viridian Forest, Mt. Moon Cave, Diglett's Cave tunnel,
  both Rock Tunnel floors, both Seafoam Islands floors, and all three Cerulean
  Cave floors.
- Kanto League caves: all three `MAP_VICTORY_ROAD_KANTO_*_HNS` maps.

Those groups contain 52 currently active map IDs. Route 22 is Kanto-owned
transition ecology. `MAP_ROUTE23_HNS` is a reserved 53rd owner even though it
currently has no HNS encounter profile and carries `MAPSEC_INDIGO_PLATEAU`.
Its target profile stem is `gRoute23_hns`; land, Surf, and fishing are the
expected methods if its population is authored.

Sevii, Routes 26 through 28, Tohjo Falls, Mt. Silver, and the Johto Rocket
Hideout are excluded. Kanto interiors without an active encounter method do
not enter the probability denominator until a later design activates them.
Map labels, ownership, and active methods must be frozen together in the
implementation specification.

## Boundaries

This PRD covers ordinary random land, Surf, Rock Smash, and fishing encounters
on mainland Kanto maps in HNS. It also sets the ecological direction for Sevii
content when those profiles are available in the same product.

It does not redesign fixed encounters, gifts, trades, fossils, Game Corner
prizes, starters, roamers, scripted encounters, hidden DexNav encounters,
facilities, or event islands. It does not change terrain, add encounter surfaces,
move map events, distribute rods, or define region access.

Kanto does not promise all 151 Generation I species as ordinary wild catches.
Starters, fossils, Hitmon choices, Eevee, Porygon, Lapras, trades, statics, and
legendaries should keep their authored acquisition identity unless another PRD
changes that content. An ordinary placement must not be added solely to complete
the Pokédex. Mew remains quest or event content rather than an ordinary wild
encounter.

The redesign targets new saves. Prerelease encounter state does not require a
migration.

## Balance

Mainland Kanto targets the following generation shares among successful
ordinary encounters:

Generation classification normally uses the effective species' base National
Dex number, and a form uses its base species' number. Numbers 252 through 386
inclusive are Generation III. Wynaut and Azurill are the only exception: when
predecessor resolution produces them from an authored Wobbuffet or Marill
family entry, the balance report counts the outcome with its Generation II
family. Neither species may occupy an authored ordinary slot.

In the table, Generation III families means independent Generation III
families, not Wynaut or Azurill as resolved Generation II family extensions.

| Time | Gen I | Gen II families | Gen III families | Gen IV+ |
| --- | ---: | ---: | ---: | ---: |
| Day | 75% to 85% | 10% to 20% | 0% | No more than 5% |
| Night | 60% to 75% | 20% to 35% | 0% | No more than 5% |

These are regional targets, not quotas for every map. For measurement, each
active mainland map and method profile contributes equally within the selected
time period. The report removes empty and ineligible entries, renormalizes the
active probability, aggregates duplicate slots into species probabilities,
then averages the generation shares across profiles. Encounter-rate values
decide whether a profile is active but do not weight the average. Ability
influence, Lures, Hoenn Sound, and randomizer behavior are off for this
baseline. Independent Generation III families must remain exactly zero. The
other generation shares must total 100 percent, so Generation I and Generation
II families must together account for at least 95 percent.

The current ownership manifest contains 126 active profiles per runtime time
period: 40 land, 30 Surf, 31 fishing, and 25 interaction profiles. Thirty-six
night profiles currently rely on automatic daytime fallback. Authoring Route
23 with the three expected methods would make the target denominator 129
profiles across 53 maps. A change to map membership or active profile counts
requires an updated audit rather than an unexplained denominator change.

Fishing uses the effective Standard Rod weights. The regional calculation is
run once for each of Old, Good, and Super Rod quality, and all three results
must meet the generation bands. The report also breaks out land, Surf, Rock
Smash, and fishing as habitat diagnostics. Method breakouts require review but
do not have generation quotas under this PRD.

Night's regional Generation II share must be at least 5 percentage points above
day's share. This makes the authored night layer visible even when both time
periods independently satisfy their wider bands.

The bands apply with Hoenn Sound off. With no authored Generation III species
in the ordinary profiles, Hoenn Sound must not change the encounter
distribution.
The ordinary mainland union across day and night should contain 105 to 120
distinct species. This is a guard against both a narrow FRLG copy and the much
broader HNS catch-all population. Habitat quality takes precedence over filling
the top of this range.

The current Kanto-owned population contains 63 distinct Generation III species
across 36 maps. Removing them leaves 123 distinct ordinary species before the
rest of the redesign, so the 105 to 120 target remains reachable without
replacing Hoenn species one for one.

A Generation II addition may be common in one fitting profile and absent from
most of the region. Every existing Generation III slot, including an authored
Wynaut or Azurill slot, must be removed rather than hidden behind a low
probability. Generation IV and later species should normally occupy rare,
specific habitats or continue an established regional family. They should not
replace a Generation I anchor or a removed Generation III species merely to
increase total species coverage.

Removing a Generation III species must not leave an active slot empty. Reuse
an allowed habitat species at an appropriate rarity when the engine's fixed
method shape requires the slot, while still respecting the regional diversity
ceiling.

## Content

The combined daytime foundation includes the full ecological contribution of
both FireRed and LeafGreen. Version-exclusive pairs and asymmetric assignments,
including Safari Zone and fishing populations, become single-save rarity
counterparts. Shared staples retain the probability needed to define their
routes instead of losing most of their weight to the merge.

Night additions should favor species with a clear Kanto habitat. Hoothoot may
appear on wooded routes, Spinarak in suitable vegetation, and Murkrow or
Misdreavus in a small number of darker locations. These are examples of the
rule, not a required checklist. Sentret, Furret, and other Johto species should
not be repeated throughout Kanto merely because they fit a generation target.
Daytime Generation II additions should usually be uncommon local finds rather
than replacements for a route's Generation I anchors.

The Safari Zone remains the main ordinary habitat for its defining rare
species. Caves, forests, power facilities, coastlines, ponds, and urban edges
retain distinct populations. Rare species should have one or a few memorable
sources rather than a low-probability slot on many unrelated maps.

Vermilion and Cinnabar fishing must retain Chinchou as the native Surf source
for the open-world mainland-to-Cinnabar crossing. At both named sources and at
every supported Trainer Rating, Chinchou must remain exactly as accessible as
the current Standard Rod contract: 11 percent of successful Old Rod encounters
and 2.75 percent of unmodified casts. Its authored level range follows its
mapped FireRed or LeafGreen rarity role. If the resulting effective level can
fall below the native-HM schedule's current level 20 floor, the implementation
must extend Chinchou's Surf schedule to the lowest attainable level and update
the native-HM source record and specification in the same change. Every
qualifying catch must know Surf.

## Interactions

Trainer Rating owns ordinary wild level progression after the encounter author
supplies the FireRed or LeafGreen level range required above. Kanto must not
retain HNS's postgame-authored levels or use permanently high authored levels
to enforce a postgame order. Reverse evolution, species floors, entry
eligibility, and level projection retain the behavior defined by the Trainer
Rating product.

Standard Rod owns fishing entry eligibility, quality weights, bite rates, Lure
behavior, and rod progression. This PRD owns which species occupy Kanto's ten
fishing entries and their rarity roles. Every probability review must use the
effective Standard Rod distribution, not the historical Old, Good, and Super
Rod partitions.

This PRD supersedes the Standard Rod PRD's rule that existing fishing entries
remain unchanged, only for mainland Kanto profiles covered here. It does not
supersede the ten-entry shape, global quality weights, rod progression, Lure
behavior, or exact native Surf accessibility records. The implementation work
must update the Standard Rod specification and its source records when an
authorized table change affects their documented inputs.

Hoenn Sound currently boosts an eligible authored Hoenn species already present
in the selected land or Surf population. It does not unlock a separate
radio-only pool and does not target Wynaut or Azurill when predecessor
resolution produces them from a Generation II family entry. Removing authored
Generation III species from ordinary Kanto profiles therefore makes the
current encounter modifier inert. A future radio-only population requires its
own product decision and must not be simulated by leaving Hoenn species in the
base tables.

The encounter redesign must not ship with player-facing Hoenn Sound text that
promises nearby Hoenn encounters while the modifier is inert. Until a true
radio-only population is approved, the station must be hidden or presented as
music without an encounter claim everywhere it is available under the Kanto or
Sevii exclusion.

Ability-based selection, Lures, randomizer mode, time-of-day fallback, and
ordinary population readers retain their existing mechanics. The Pokédex area
display and other ordinary population readers should expose the same effective
species population as actual encounters for the selected time and eligibility
state.

## Constraints

Each method keeps the target engine's active shape: 12 land entries, 5 Surf
entries, 5 Rock Smash entries, and 10 fishing entries. Inert trailing source
rows do not count as encounters, probability, diversity, or night retention.

The FRLG merge stays within the target method's probability budget. Splitting a
source rarity role between two species is valid. Adding both full source arrays,
creating unreachable trailing rows, or counting slot occurrences without engine
weights is not valid.

Every mainland map profile must have an unambiguous regional owner and time
identity. Transition maps cannot be omitted from reports because of a map-section
or label mismatch.

## Playtesting

Playtesting should answer these questions:

- Does daytime Kanto still feel recognizably like FRLG on its early routes,
  forests, caves, coasts, and Safari Zone?
- Can a new Kanto save progress from its starter through the opening routes and
  first badge without encountering the former HNS postgame level curve?
- Does night reward revisiting maps while leaving familiar daytime residents
  visible?
- Are Johto species memorable finds tied to believable habitats rather than a
  thin layer spread over every route?
- Can a player understand common, uncommon, and rare fishing populations through
  repeated use of each Standard Rod quality?
- Can a prepared player obtain Chinchou from both Vermilion and Cinnabar without
  an unreasonable number of casts, then complete the native Surf crossing in
  both directions?
- Do rare species remain discoverable without external documentation, and do
  defining Safari, cave, coastal, and facility populations stay distinct?
- Does Sevii feel broader than mainland Kanto without feeling random?

## Acceptance

- Produce a deterministic distribution report for every mainland map, method,
  day or night variant, and integer Trainer Rating from 10 through 80. Report
  entry weights, eligible entries, aggregate species probabilities, generation
  shares, and the regional and per-method summaries.
- Confirm the report matches the profile-ownership manifest, includes all three
  Kanto Victory Road maps, reserves Route 23 for Kanto, and excludes the named
  Johto and Sevii profiles. Reconcile the denominator when Route 23 is authored.
- Confirm the regional day and night summaries remain within their generation
  bands at every supported Trainer Rating, for each Standard Rod quality, with
  baseline modifiers off. Confirm night has at least 5 percentage points more
  Generation II probability than day. List any local habitat exceptions without
  excluding them from the regional calculation.
- For every map and method, confirm the night population retains species whose
  daytime selectable probability totals at least 70 percent and that those
  shared species also total at least 70 percent of night probability.
- Confirm at least 60 percent of mainland land profiles have a day-to-night
  species-distribution distance of at least 10 percentage points.
- List every explicit unchanged day alias and confirm that no automatic runtime
  fallback is treated as an authored night result.
- Confirm every FRLG version-exclusive or asymmetric daytime species assignment
  in scope passes the counterpart rule or its documented discrete-slot
  resolution, and that no method exceeds its active entry count.
- Produce an authored-level provenance report for every mainland Kanto slot.
  Confirm every direct daytime counterpart uses its FireRed or LeafGreen source
  minimum and maximum, every introduced species and night assignment inherits
  a mapped source rarity role, and every slot in a method without a direct
  counterpart maps to a named FireRed or LeafGreen analog role and inherits its
  exact minimum and maximum. For a shared species and role with different source
  ranges, confirm the deterministic lower-level source rule selected one
  complete range. List no HNS postgame level as a source.
- Report the effective minimum and maximum levels at Trainer Rating 10 for every
  Kanto profile, then playtest a new Kanto save through the first badge. Confirm
  level projection uses the FireRed and LeafGreen authored inputs and that the
  opening does not behave like the former postgame Kanto curve.
- At Trainer Rating 10, confirm the land populations for Routes 1, 2, 3, and 22,
  Viridian Forest, and every Mt. Moon floor produce no encounter above level
  12. This ceiling is the current projection of the highest level 12 source
  role in that opening set.
- Confirm the mainland day and night union contains 105 to 120 distinct ordinary
  species and Generation IV onward never exceeds 5 percent of regional
  probability.
- Across every Kanto-owned mainland profile, report zero Generation III species
  in active authored slots. In every
  non-randomized effective population, allow only Wynaut or Azurill produced by
  predecessor resolution from an authored Generation II family entry; report
  zero probability for every other Generation III species. Apply this at every
  Trainer Rating, day and night, encounter method, and Standard Rod quality,
  with Hoenn Sound off and on.
- Classify Generation III from base National Dex numbers 252 through 386. Count
  a resolved Wynaut or Azurill with its Generation II family and report its
  provenance separately. Do not count `SPECIES_NONE`, inactive trailing rows,
  gifts, trades, statics, scripted encounters, facilities, event islands, or
  randomized outcomes.
- Report Hoenn Sound off and on for every land and Surf profile and confirm the
  species distributions are identical. Confirm the station is hidden or its
  player-facing text no longer claims that Hoenn species may appear nearby
  everywhere the exclusion applies.
- If the implementation includes Sevii, freeze its map and profile manifest,
  produce authored-level provenance for every slot in it, and apply the direct
  counterpart, source-conflict, and no-source analog rules above. Also run the
  Generation III family and Hoenn Sound checks against every profile in that
  manifest. Otherwise confirm no Sevii encounter profile changed.
- Run the Standard Rod distribution report at all three qualities, with Lure off
  and on, for every Kanto fishing profile. Confirm all ten active entries remain
  selectable when eligible.
- At Vermilion and Cinnabar, confirm Chinchou remains available at every Trainer
  Rating with exactly 11 percent probability per successful Old Rod encounter
  and 2.75 percent per unmodified cast. Confirm every qualifying Chinchou knows
  Surf. If its effective range falls below level 20, confirm the native-HM
  source record, specification, and generated learnset were updated together.
- Inspect representative early route, forest, cave, power facility, Safari Zone,
  coast, pond, and transition profiles in game at day and night, plus Sevii
  profiles when they are included. Compare observed results with the
  deterministic report.
- Confirm gifts, trades, fossils, prizes, statics, scripted encounters,
  legendaries, and Mew acquisition are unchanged.

## References

- [Johto wild encounters](johto-wild-encounters.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [FireRed and LeafGreen open-world regional traversal](frlg-open-world-region-traversal.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Standard Rod fishing](standard-rod-fishing.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Authored wild encounter data](../../game/src/data/wild_encounters.json)
- [Ordinary wild encounter selection](../../game/src/wild_encounter.c)
