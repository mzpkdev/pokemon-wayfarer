# Johto wild encounter ecology

## Intent

Make Generation II define Johto's local identity without turning the Kanto
border into a hard ecological boundary. The existing HNS encounters remain the
foundation:
maps keep their recognizable habitats, day and night identities, and regional
families. Probability shifts make Generation II species part of ordinary play
instead of occasional decoration, while familiar Generation I species still
connect Johto to its neighboring region.

The result should suit an open-world campaign. Players may enter an area from
several directions and at different stages, so encounter identity comes from
species, habitat, method, time of day, and rarity. Trainer Rating continues to
own encounter levels and progression scaling.

## Design

The HNS Johto encounter profiles are the starting point. Rebalancing follows
this order:

1. Change the probability of species already authored for the map, method, and
   time of day.
2. Consolidate duplicate slots when repeated species prevent a useful rarity
   curve.
3. Replace a duplicate with a habitat-appropriate Generation II species only
   when weighting alone cannot meet the regional targets or would make one
   species unreasonably repetitive.

The native HM coverage inventory constrains every reweighting and replacement.
Gligar, Aipom, Chinchou, Mareep, Wooper, Snubbull, Miltank, Marill, and Mantine
must retain every qualifying map, time variant, authored level, and Trainer
Rating outcome named by that design. A slot that supplies required coverage is
not a replacement candidate unless the same change preserves the complete
coverage contract.

Species replacement is the exception. It must preserve the area's habitat and
day or night identity. A forest, cave, coast, lake, route, and settlement edge
should not become interchangeable collections of the same favored species.

Johto has no hard generation border. Common Generation I species such as cave,
rodent, bird, and coastal families may remain common when they are part of the
area's established ecology. More distinctive Kanto species should be
localized or uncommon in Johto. Generation II species should supply the
region's most visible local character.

Cross-generation families count as one ecological family for placement. A
Generation I base species does not need to disappear from Johto because it
evolves into a Generation II species. The authored stage, Trainer Rating
resolution, and the habitat should decide which member appears.

Day and night variants retain their current purpose. A time change may alter
the leading species and reveal nocturnal or diurnal finds, but it should not
erase the habitat shared by both variants. Existing HNS day and night rosters
remain intact unless a documented substitution is needed under the hierarchy
above. Missing time variants and runtime fallback must be reported explicitly;
fallback data does not count as an authored night profile.

Routes 26 through 28, Tohjo Falls, and Mt. Silver are transition habitats.
Their populations may mix the two regional generations more evenly than the
rest of Johto. They still count in the regional report, but they do not need to
meet the method targets on each individual map.

Species from independent Generation III families do not appear in ordinary
Johto encounters. Hoenn owns their ordinary discovery. Wynaut and Azurill may
appear only as Generation II family extensions produced by predecessor
resolution; neither may occupy an authored slot. Generation IV and later
species may remain as rare, habitat-specific finds when they continue an
established family or serve a deliberate local role. They must not displace
Johto's Generation II identity or become a generic filler pool.

## Boundaries

This design covers ordinary HNS wild encounter profiles whose maps belong to
Johto, including land, Surf, fishing, and the shared interaction data used by
Rock Smash and Headbutt. It does not change Kanto, Sinjoh, Alola, Faraway
Island, Southern Island, hidden encounters, outbreaks, roamers, scripted
battles, fixed encounters, facilities, or randomizer behavior.

The three `MAP_VICTORY_ROAD_KANTO_*_HNS` maps and reserved
`MAP_ROUTE23_HNS` belong to the Kanto encounter PRD even when their current map
sections suggest otherwise. They do not enter Johto's profile count or
probability denominator.

The rebalance does not edit authored encounter levels. It also does not use
base encounter rates, bite rates, or interaction trigger rates to manufacture
the target generation shares. Those rates may change only under a separate
design that addresses encounter frequency.

This design does not make every Generation II species an ordinary wild catch.
Starters, babies, gifts, game-corner prizes, fossils, statics, and legendaries
keep their existing acquisition identity. Rebalancing must not add one of
those species to an ordinary table merely to improve a generation ratio.
Existing special acquisitions and scripted encounters are outside the
probability portfolio.

The work does not promise Pokédex completion within Johto alone. Regional and
whole-game catchability remain separate product decisions.

The independent Generation III family exclusion applies to ordinary wild
encounters. It does not remove evolution, breeding, gifts, trades, fixed
encounters, or other special acquisition methods that another design owns.

## Balance

The current HNS Johto portfolio is heavily weighted toward Generation I. The
audited baseline and target ranges are:

Generation classification normally uses the effective species' base National
Dex number, and a form uses its base species' number. Numbers 252 through 386
inclusive are Generation III. Wynaut and Azurill are the only exception: when
predecessor resolution produces them from an authored Wobbuffet or Marill
family entry, the balance report counts the outcome with its Generation II
family. Neither species may occupy an authored ordinary slot.

In the table, Generation III families means independent Generation III
families, not Wynaut or Azurill as resolved Generation II family extensions.

| Portfolio | Gen I | Gen II families | Gen III families | Gen IV+ | Gen II target |
| --- | ---: | ---: | ---: | ---: | ---: |
| All ordinary Johto profiles | 72.8% | 25.9% | 0% | 1.3% | 35% to 40% |
| Land | Not separately required | 27.9% | 0% | Not separately required | 40% to 50% |
| Surf | Not separately required | 11.9% | 0% | Not separately required | 20% to 30% |
| Fishing | Not separately required | 3.7% | 0% | Not separately required | 15% to 25% |

The completed overall portfolio must place Generation I between 55% and 65%,
Generation II families between 35% and 40%, independent Generation III
families at exactly 0%, and Generation IV onward at 5% or less. The values must
total exactly 100% before display rounding. Acceptance uses the exact calculated
probabilities; a report may show rounded values for reading, but rounding
cannot change a pass or failure.

Rock Smash and Headbutt use the same authored interaction profiles in HNS.
Generation II should account for 50% to 65% of their regional probability
mass. Pineco currently accounts for 47.1% of that mass. The rebalance must
reduce Pineco to no more than 25% and must keep every other single species at
or below the same cap. This is a regional cap, not a rule against a
Pineco-heavy forest profile.

These ranges are regional portfolio targets. They are not quotas for every
map, method profile, or time variant. A cave may remain mostly Generation I, a
night route may be dominated by one Generation II nocturnal species, and a
transition area may be evenly mixed. The aggregate result must meet the
targets without flattening those local identities.

The acceptance report measures authored selection probability before Trainer
Rating changes levels or resolves a species to a predecessor. It includes only
active runtime slots, drops `SPECIES_NONE`, renormalizes the remaining entries,
and uses the real slot weights for each encounter method. Each active map,
time, and method profile contributes one normalized distribution to its method
portfolio so duplicate source rows do not receive accidental extra weight.

The audited Johto topology contains 366 nonzero-rate records: 123 land, 90
Surf, 89 fishing, and 64 Rock Smash or Headbutt interaction records. Three Surf
records contain only `SPECIES_NONE`, leaving 363 selectable profiles and 87
selectable Surf profiles. Let `L`, `W`, `F`, and `I` be the Generation II share
in the four selectable method portfolios. The regional authored Generation II
share is:

`R = (123L + 87W + 89F + 64I) / 363`

The 35% to 40% overall target is coupled to the method ranges through this
formula. A change to map membership or active profile counts requires a
recorded audit update and a recalculation of the compatible overall range.

Fishing is reported separately for Old Rod, Good Rod, and Super Rod because
Standard Rod quality changes the ten slot weights. The fishing portfolio `F`
uses the equal average of those three quality distributions for each profile
and must place Generation II between 15% and 25%. The report also shows the
three individual quality results. Each quality must assign at least 10% of its
regional successful-catch probability to Generation II so a valid average
cannot hide an unusable rod tier.

A second report samples the effective species outcomes at Trainer Ratings 10,
16, 40, 55, 65, and 80. At every sampled rating, effective Generation II
probability must remain between 30% and 45% overall, 35% and 55% for land, 15%
and 35% for Surf, 10% and 30% for equal-quality fishing, and 45% and 70% for
interaction profiles. Each individual rod quality must retain at least 5%
effective Generation II fishing probability. Independent Generation III
families must remain at exactly 0% in every effective report. These results do
not replace the authored targets; they ensure that entry eligibility,
predecessor resolution, and species floors do not reverse Johto's identity
during campaign milestones.

## Content

### Land

Land profiles should make Generation II species routine across early routes,
forests, grassland, caves, and mountain paths. Reweight existing local species
before adding placements. Common Generation I species remain useful ecological
background, but repeated Rattata, Pidgey, Zubat, and Geodude slots are the
first candidates for consolidation when they crowd out an established local
species.

Rare species should stay rare and geographically meaningful. Prefer a better
chance in one suitable habitat or one additional suitable habitat over broad
distribution across unrelated maps.

### Surf and fishing

Johto's water should no longer read as an almost entirely Generation I
population. Existing placements for Chinchou, Wooper, Marill, Qwilfish,
Remoraid, Corsola, and Mantine are the first candidates for additional weight
where their habitat fits. Magikarp, Poliwag, Tentacool, Goldeen, Krabby, and
other established Generation I water species remain, but repeated entries may
be consolidated to create room in the probability curve.

The ten fishing entries retain meaningful common, less common, and rare
positions under Standard Rod. Generation II species must appear across the
sequence rather than being confined to the last entries, because every rod
quality uses the same population with different weights.

### Rock Smash and Headbutt

The shared interaction profiles need more than one dominant Generation II
species. Reweight or consolidate Pineco slots and distribute probability among
the other habitat-appropriate authored species. Headbutt trees should still
feel different from grass even though they share the Rock Smash data path.

Aipom's existing Headbutt coverage must not shrink. The set of Johto maps and
time variants where Aipom is available through Headbutt must remain at least as
broad as the current HNS baseline, and its aggregate chance within those
profiles must not decrease. This encounter route continues to depend on
Headbutt being obtainable independently of Cut and Rock Smash, as required by
the native HM design.

### Special and transition populations

Special acquisitions do not contribute to the portfolio targets. Their
species, locations, choice structures, and progression conditions remain
unchanged.

Routes 26 through 28, Tohjo Falls, and Mt. Silver should communicate movement
between regions. Their common populations may draw from both generations, and
their rare slots may contain species associated with either side of the
border. They must still fit their individual grassland, water, cave, or
mountain habitat.

## Interactions

- Trainer Rating selects eligible entries, projects levels, and resolves
  unsupported evolved species after authored slot selection. This PRD does not
  duplicate or override that logic.
- Standard Rod keeps one ten-entry fishing population per profile. Old, Good,
  and Super Rod quality changes probability and bite reliability, not species
  eligibility.
- This PRD supersedes the Standard Rod PRD's rule that existing fishing entries
  remain unchanged, only for Johto profiles covered here. It does not supersede
  the ten-entry shape, global quality weights, rod progression, Lure behavior,
  or exact native Surf accessibility records. Implementation must update the
  Standard Rod specification and source records for authorized table changes.
- This PRD also supersedes the native HM specification's blanket prohibition on
  encounter edits, but it does not supersede any named coverage outcome. The
  full Gligar, Aipom, Chinchou, Mareep, Wooper, Snubbull, Miltank, Marill, and
  Mantine inventory remains binding.
- Mantine uses the global minimum ordinary wild level of 14. This is the
  minimum floor that keeps an authored level-15 Mantine eligible when it
  projects to level 14 at Rating 10. The correction does not change its
  authored levels or add Mantyke predecessor behavior.
- The Olivine and Cianwood Chinchou sources remain traversal-safe. Every
  required source must preserve the exact Standard Rod records: 11% of
  successful Old Rod encounters for every Olivine day and night profile, and
  12% for the Cianwood daytime profile. With the 25% Old Rod bite rate, these
  remain 2.75% and 3% per unmodified cast respectively.
- Time-of-day selection, ability-based slot influence, Lures, repel checks,
  and ordinary population readers continue to operate on the rebalanced
  profiles.
- Hoenn Sound must not change an ordinary Johto distribution, including when a
  persisted Hoenn track reaches Johto. It does not target Wynaut or Azurill when
  predecessor resolution produces them from a Generation II family entry. A
  future radio-only Hoenn population requires a separate superseding product
  decision.
- Pokédex area data and other ordinary population readers must expose the same
  eligible species that an encounter can produce.
- Randomizer mode continues to receive the selected authored entry and raw
  slot identity. Regional generation targets apply to the non-randomized
  authored population only.

## Constraints

The encounter data keeps its existing runtime shape: 12 active land entries,
5 active Surf entries, 5 active Rock Smash or Headbutt entries, and 10 active
fishing entries where the method exists. Source rows beyond an active runtime
count do not contribute to balance or acceptance.

Day and night profiles must remain valid under the configured time fallback.
The balance report must flag a missing method variant, a fallback-only night
population, and any profile that the engine cannot bind to its intended map or
time.

Changes must preserve valid species identifiers, level ranges, method entry
counts, encounter-rate fields, generated header bindings, Trainer Rating
metadata, Standard Rod accessibility records, and deterministic generator
output.

The implementation specification must freeze the report's map membership,
normalization, generation classification, time fallback treatment, and
rounding rules. The same calculation must produce the recorded baseline and
the final result so acceptance cannot move through a change in methodology.

## Acceptance

The Johto rebalance is accepted when all of the following are true:

- A deterministic report lists every included map, time variant, method,
  active slot, authored species probability, and generation.
- The overall, land, Surf, equal-quality fishing, and interaction portfolios
  meet their target ranges, and all three individual fishing-quality results
  are reported and meet the 10% Generation II floor.
- Pineco and every other interaction species meet the 25% regional cap.
- A before-and-after roster diff documents every species addition, removal,
  and duplicate-slot consolidation with a habitat reason.
- A day-and-night diff confirms that existing time identities remain visible
  and identifies every runtime fallback.
- Effective outcome reports at the six Trainer Rating milestones meet the
  numeric overall, method, and individual-rod guardrails.
- Across every Johto-owned profile, report zero Generation III species in
  active authored slots. In every non-randomized effective population, allow
  only Wynaut or Azurill produced by predecessor resolution from an authored
  Generation II family entry; report zero probability for every other
  Generation III species. Apply this at every Trainer Rating, day and night,
  encounter method, and Standard Rod quality, with Hoenn Sound off and on.
- Classify Generation III from base National Dex numbers 252 through 386. Count
  a resolved Wynaut or Azurill with its Generation II family and report its
  provenance separately. Do not count `SPECIES_NONE`, inactive trailing rows,
  gifts, trades, statics, scripted encounters, facilities, event islands, or
  randomized outcomes.
- The Chinchou accessibility rows pass Standard Rod validation, and Aipom's
  Headbutt map coverage and aggregate probability do not regress.
- Every qualifying native HM profile still contains its named Gligar, Aipom,
  Chinchou, Mareep, Wooper, Snubbull, Miltank, Marill, or Mantine anchor at the
  required authored levels and at every Trainer Rating from 10 through 80.
- Gifts, statics, babies, starters, fossils, prizes, and legendaries have not
  been added to ordinary encounter tables merely to satisfy a target.
- Encounter generation and its existing deterministic tests pass with no
  malformed, unreachable, or empty ordinary profile.

## Playtesting

Test fresh open-world runs that begin through different Johto routes and at
day and night. Each run should answer these questions:

- Do common catches make the region recognizable as Johto within the first few
  areas, without making every route use the same Generation II species?
- Do Generation I encounters feel like shared ecology or localized finds
  rather than leftover filler?
- Do day and night change the leading encounters while preserving the map's
  habitat?
- Does Surf reveal a Johto water population often enough to be noticed?
- Can each rod quality produce Generation II catches during ordinary fishing,
  and do upgrades still make uncommon and rare entries more likely?
- Do Headbutt and Rock Smash produce varied results, or does Pineco or another
  species still dominate repeated interactions?
- Can a player obtain Chinchou for the approved Surf crossings and Aipom for
  native HM use without unreasonable repetition or circular field-move
  requirements?
- Do Routes 26 through 28, Tohjo Falls, and Mt. Silver feel like a gradual
  ecological transition rather than an abrupt roster swap?
- At early, midgame, League, and postgame Trainer Ratings, do the same places
  keep their identity while levels remain appropriate?

Playtest notes should record the map, time, method, rod quality when relevant,
Trainer Rating, number of attempts, and observed species. Probability reports
prove the authored distribution; playtesting decides whether that distribution
is noticeable and enjoyable.

## References

- [Kanto wild encounters](kanto-wild-encounters.md)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Trainer Rating wild encounter scaling specification](../specs/trainer-rating-wild-encounter-scaling.md)
- [Standard Rod fishing](standard-rod-fishing.md)
- [Standard Rod fishing specification](../specs/standard-rod-fishing.md)
- [Native HM learnsets](native-hm-learnsets.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Johto story traversal research](../research/johto-story-traversal-blockers.md)
- [Kanto and Johto inter-region travel research](../research/kanto-johto-inter-region-travel.md)
