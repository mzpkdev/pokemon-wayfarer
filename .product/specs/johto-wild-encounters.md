# Johto wild encounters

PRD: [Johto wild encounter ecology](../prds/johto-wild-encounters.md)
Implemented: No

## Scope

This specification defines the HNS Johto ordinary wild encounter rebalance.
It freezes the included map, time, and method profiles; preserves the HNS
authored level curve; raises Generation II probability by rearranging the
existing weighted slots; and defines the reports and tests that protect time
identity, native-HM access, and the Kanto border ecology.

It does not change encounter runtime selection, encounter rates, Trainer
Rating, rod quality, region access, special acquisitions, terrain, hidden
encounters, facilities outside the frozen profile manifest, or randomized
species behavior.

## Behavior

### Regional policy source

Use the shared `game/src/data/wild_encounter_regions.json` file defined by the
Kanto wild encounter specification. `game/src/data/wild_encounters.json`
remains the only authored source for Johto species, level ranges, encounter
rates, and slot order.

The encounter author chooses the exact target species assignments under the
closed rules and numeric bounds below. Once committed, those assignments are
the implemented behavior. They do not require a second mirrored roster in this
document or another product decision.

The `JOHTO` object has exactly `product`, `profiles`, `fallbacks`,
`protectedAnchors`, and `changes`. `product` equals `POKEMON_HNS`; the other
four fields are arrays. Every profile record has:

- `map`, `baseLabel`, `runtimeTime`, and `method`.
- `activeSlotCount`, equal to the production count for the method.
- `selectable`, which is false only when all active slots are `SPECIES_NONE`.
- `habitat`, using the same closed habitat values as Kanto.
- `transition`, true only for Routes 26 through 28, Tohjo Falls, and Mt. Silver.

The manifest lists authored source profiles. It does not synthesize a second
portfolio row for runtime fallback. Each `fallbacks` record contains `map`,
`method`, `runtimeTime`, and `sourceBaseLabel`. It records which missing
runtime times use an authored day profile so Cartographer and the day-and-night
diff can expose them.

Every `changes` record identifies a profile, target slot, before species, after
species, and one of these ordered change kinds:

1. `REWEIGHT_EXISTING`, where species already present in the profile move among
   fixed slot weights.
2. `CONSOLIDATE_DUPLICATE`, where a repeated species loses a slot and another
   species already present receives it.
3. `ADD_LOCAL_SPECIES`, where a duplicate slot receives a species not previously
   present in that profile.
4. `REMOVE_FORBIDDEN`, used only to remove a directly authored independent
   Generation III species.

Each change also records `reason`, `habitatEvidence`, and `targetFailureBefore`.
`reason` is one of `METHOD_IDENTITY`, `TIME_IDENTITY`, `LOCAL_FAMILY`,
`TRANSITION_ECOLOGY`, `NATIVE_HM`, or `FORBIDDEN_GENERATION`.
`habitatEvidence` names at least one unchanged neighboring profile or another
time variant that already supports the species. `targetFailureBefore` lists
the numeric portfolio or single-species cap that the previous stages could not
meet. It is empty for `REWEIGHT_EXISTING` and `REMOVE_FORBIDDEN`.

The generator reconstructs the baseline from the change ledger, replays the
ordered changes, and calculates a portfolio snapshot after each stage. It
rejects `ADD_LOCAL_SPECIES` unless at least one named portfolio target or
single-species cap still fails after all recorded reweighting and consolidation
changes. This makes species replacement a measured exception rather than an
unreviewed authoring shortcut.

### Frozen ownership and topology

The Johto manifest contains these 93 map IDs:

| Group | Map IDs |
| --- | --- |
| Routes | `MAP_ROUTE26_HNS` through `MAP_ROUTE48_HNS` |
| Settlements and gates | `MAP_NEW_BARK_TOWN_HNS`, `MAP_CHERRYGROVE_CITY_HNS`, `MAP_VIOLET_CITY_HNS`, `MAP_AZALEA_TOWN_HNS`, `MAP_GOLDENROD_CITY_HNS`, `MAP_ECRUTEAK_CITY_HNS`, `MAP_OLIVINE_CITY_HNS`, `MAP_OLIVINE_CITY_PORT_OUTSIDE_HNS`, `MAP_CIANWOOD_CITY_HNS`, `MAP_MAHOGANYTOWN_HNS`, `MAP_BLACKTHORN_CITY_HNS`, `MAP_CLIFF_EDGE_GATE_HNS`, `MAP_SAFARI_ZONE_GATE_HNS` |
| Early and central dungeons | both Dark Cave sides; Sprout Tower 2F and 3F; Ruins of Alph outside and B1F; Union Cave 1F, B1F, and B2F; Slowpoke Well B1F and B2F; Ilex Forest; Burned Tower 1F and B1F; Cliff Edge Cave |
| Eastern dungeons | Mt. Mortar 1F South, 1F North, B1F, and 2F; Lake of Rage; Ice Path 1F and B1F through B4F; Dragon's Den cavern; Tohjo Falls cavern; Rocket Hideout B1F |
| Whirl Islands | 1F, B1F, B1F Inner, B2F, B3F, and Descent |
| Mt. Silver | outside, 1F Item Room, 1F Moltres Room, 1F Waterfall Room, 2F, 3F, Mountain Side, and Snow |
| Parks and towers | National Park normal and Bug Contest; Tin Tower 3F through 9F |
| Johto Safari | top middle, top left, top right, low middle, low left, and low right |

The named dungeon rows expand to the exact `MAP_*_HNS` constants in
`wild_encounters.json`; aliases or map-section matches are not accepted. The
generator stores the expanded constant list in the audit so it can be compared
without interpreting the prose grouping.

The manifest contains 149 authored time rows and 366 nonzero-rate method
profiles: 123 land, 90 Surf, 89 fishing, and 64 interaction. These three Surf
profiles are present but not selectable because every active slot is
`SPECIES_NONE`:

- `gMtSilver_1F_ItemRoom_hns_Day`
- `gMtSilver_1F_ItemRoom_hns_Night`
- `gMtSilver_MountainSide_hns_Day`

They remain in the topology report but not the probability denominator. The
selectable denominator is 363 profiles, including 87 Surf profiles. The
National Park Bug Contest and Rocket Hideout profiles are included because
they use the same authored ordinary population data and selection path. This
specification changes only their encounter table composition, not when or how
the player enters them.

The three Kanto Victory Road maps and Route 23 are Kanto-owned and rejected
from Johto. Kanto, Sevii, Sinjoh, Alola, Faraway Island, and Southern Island
profiles are also rejected.

Any membership or active-method change must update this specification, the
manifest, the recorded baseline, and the denominator formula together. A
label rename is also a manifest change because labels participate in runtime
time binding.

### Fixed-slot rebalance

Johto keeps every baseline encounter rate, minimum level, and maximum level at
the same profile and numeric slot. Only the species value may move or change.
This keeps the HNS authored level curve intact even when a species moves to a
different probability role.

The production weights are 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, and 1 for
land; 60, 30, 5, 4, and 1 for Surf and interaction; and the Standard Rod
quality vectors for fishing. Reweighting means assigning the existing species
multiset to different fixed weights. Consolidation or addition changes that
multiset but not the weight vector.

For each method, apply changes in manifest order:

1. Apply forbidden-generation removals.
2. Apply reweighting of the existing species multiset within each profile.
3. Apply duplicate consolidations without adding a new local species.
4. Permit recorded local additions only while a named target or cap still
   fails.

Within one stage, authors choose the smallest practical change set. The ledger
must order changes by profile manifest order, target slot index, and species
constant. A slot changed more than once forms a chain whose `afterSpecies`
equals the next record's `beforeSpecies`. Reversing every chain must reproduce
the recorded baseline, and replaying it must reproduce the committed
`wild_encounters.json` data. The generator reports total probability moved,
changed slots, and changed profiles at each stage. It does not solve the
content portfolio or choose species.

An addition must already exist in the same habitat and time category on an
unchanged neighboring Johto map or the target map's other authored time
variant. Starters, babies, gifts, prizes, statics, legendaries, and species
available only through a special acquisition are not candidates. A transition
profile may use a species from either adjacent regional ecology, but its
`habitatEvidence` must still name a qualifying profile.

The rebalance does not require every map to meet a generation quota. It must
not spread one Generation II species across unrelated habitats merely because
that is a numerically cheap solution.

### Authored portfolio calculation

For one profile, take the active slot prefix, remove `SPECIES_NONE`, aggregate
duplicate slots by authored species, and normalize by the remaining production
weight. Give every selectable profile equal weight inside its method
portfolio. Encounter rate and map size do not weight a profile.

Classify each species by its own National Dex number. A regional or alternate
form uses the National Dex number of its base form. Generation I is 1 through
151, Generation II is 152 through 251, independent Generation III is 252
through 386, and Generation IV onward is 387 or greater. Directly authored
Wynaut and Azurill are forbidden. A later effective resolution to either baby
is allowed and counted with its Generation II family.

Let `L`, `W`, `F`, and `I` be the Generation II shares of the 123 land, 87
selectable Surf, 89 fishing, and 64 interaction portfolios. Fishing first
averages Old, Good, and Super Rod results equally within each profile. The
overall authored share is exactly:

`R = (123L + 87W + 89F + 64I) / 363`

The completed authored portfolio must satisfy all of these exact, unrounded
requirements:

| Portfolio | Required Generation II share |
| --- | ---: |
| Overall | 35% to 40% |
| Land | 40% to 50% |
| Surf | 20% to 30% |
| Equal-quality fishing | 15% to 25% |
| Interaction | 50% to 65% |
| Each individual fishing quality | At least 10% |

Overall Generation I is 55 to 65 percent, independent Generation III is zero,
and Generation IV onward is at most 5 percent. All generation shares sum to
one before display rounding. Reports display percentages with half-up rounding
to two decimal places, but exact fractions decide acceptance.

Aggregate each interaction species across the 64 normalized profiles. Pineco
and every other individual species must be no more than 25 percent of the
regional interaction portfolio. This cap does not prevent a strong local
Pineco profile.

### Effective portfolio calculation

Run each authored slot through production Trainer Rating eligibility, level
projection, and predecessor resolution. The report retains the authored slot,
authored species, projected level outcomes, eligibility, and resulting species
so every probability can be traced back to its source.

At Ratings 10, 16, 40, 55, 65, and 80, renormalize eligible weights and repeat
the authored portfolio calculation by effective species. The results must meet:

| Portfolio | Required effective Generation II share |
| --- | ---: |
| Overall | 30% to 45% |
| Land | 35% to 55% |
| Surf | 15% to 35% |
| Equal-quality fishing | 10% to 30% |
| Interaction | 45% to 70% |
| Each individual fishing quality | At least 5% |

Independent Generation III remains exactly zero. Wynaut may resolve through
the existing numeric predecessor path. Azurill is permitted if a future
production predecessor rule resolves it from an authored Marill-family slot,
but this rebalance does not require or introduce that runtime behavior.

Ability attraction, Lures, and randomizer behavior are off for portfolio
acceptance. Produce a separate comparison with Hoenn Sound off and on. Every
land and Surf species probability must be identical in both states at every
sampled Rating.

### Time identity and fallback

Do not add or remove authored Johto time rows solely to improve the portfolio.
Existing day and night rows remain the source topology. The report identifies
each method that uses configured day fallback for night, morning, or evening.
A fallback is never reported as authored night data.

For every profile with authored day and night variants, report the leading
species, the shared-species probability in each direction, and total-variation
distance before and after the rebalance. No numeric per-profile threshold is
imposed, but a change fails if day and night become identical when they were
different in the baseline or if a time-exclusive species disappears without a
`TIME_IDENTITY` change reason.

### Protected native-HM anchors

Copy only the Johto-owned HNS rows from the native-HM specification into
`protectedAnchors` as exact machine-readable records. The Kanto Chinchou rows
at Vermilion and Cinnabar belong to the Kanto specification and do not enter
this array. Each record names species, utility moves, qualifying base labels,
method, applicable times, qualifying authored level ranges, and required
Rating range 10 through 80.

The protected species are Gligar, Aipom, Chinchou, Mareep, Wooper, Snubbull,
Miltank, Marill, and Mantine. For every named slot and authored level, the
production projection must yield an eligible family member with the required
utility moves at every Rating. The species must remain obtainable in every
qualifying place and time named by the current native-HM inventory.

Aipom remains available through the Rock Smash-backed Headbutt profiles for
Azalea Town and Route 33 at authored level 10. Its set of qualifying Headbutt
profiles may not shrink. Its aggregate normalized probability across that
fixed baseline set may not fall below the recorded baseline. Rock Smash access
does not count as the acquisition path for Aipom.

The Standard Rod source keeps Chinchou at exactly 11 percent of successful Old
Rod encounters in every Olivine port day and night profile and exactly 12
percent in Cianwood by day, with Lure off and every slot eligible. Those values
are exactly 2.75 and 3 percent per unmodified cast. All affected records in
`game/src/data/standard_rod_fishing.json` remain exact at every Rating from 10
through 80.

### Radio and ordinary readers

Hoenn Sound remains a music-only station under the Kanto specification. A
persisted Hoenn track does not change a Johto encounter because no independent
Generation III species is authored. The runtime does not treat resolved Wynaut
or Azurill as a Hoenn Sound candidate.

Pokédex area checks, DexNav, ambient species, Match Call, Oak's Pokemon Talk,
and Cartographer consume the same effective Johto population as normal
encounters. Oak's Pokemon Talk keeps its existing route and time selection
logic, but its announced species must come from an eligible effective outcome
after the rebalance. Hidden DexNav data and randomizer output remain excluded.

### Generated audit

The schema version 3 balance audit defined by the Kanto specification also
contains:

- The expanded 93-map manifest, all 149 authored time rows, all 366 nonzero
  profiles, and the three nonselectable Surf profiles.
- The recorded pre-change baseline and final authored slot data under identical
  membership, weights, classification, and normalization.
- Exact authored and effective species and generation portfolios, with each
  rod quality and all six Rating milestones.
- The ordered change ledger, portfolio snapshots after each stage, additions
  and removals, habitat evidence, and protected-anchor effects.
- Day and night leading species, shared probabilities, distances, and every
  runtime fallback.
- Interaction species shares, Aipom coverage and baseline comparison,
  Chinchou accessibility, native-HM coverage, forbidden species, and Hoenn
  Sound equality.

The recorded baseline must reproduce 72.8 percent Generation I, 25.9 percent
Generation II, 0 percent Generation III, and 1.3 percent Generation IV onward
when displayed to one decimal place. It must also reproduce the PRD's 27.9
percent land, 11.9 percent Surf, and 3.7 percent equal-quality fishing
Generation II shares to one decimal place. A mismatch means the manifest or
calculation changed and generation stops before evaluating the final data.

## Validation

Add Johto manifest, rebalance-stage, portfolio, time identity, and protected
anchor cases to `game/tools/wild_encounters/tests/test_scaling.py`. Tests cover
all weight boundaries, exact fraction aggregation, the 363-profile formula,
each rod quality, empty Surf exclusion, deterministic ledger ordering, a
rejected premature addition, habitat evidence, Generation III classification,
Wynaut family reporting, optional Azurill reporting, and Hoenn Sound equality.

Extend `game/test/native_hm_learnsets.c` only if the manifest-driven coverage
requires new test access. Keep its existing HNS anchor assertions passing.
Keep the ordinary runtime, fishing, Pokédex, and DexNav tests passing. Add an
Oak's Pokemon Talk assertion that a rebalanced slot announces an eligible
effective species.

Run `make wild-encounter-scaling-test`, `make wild-encounter-balance-audit`,
and the HNS mechanics tests. Run the devtools catalog tests and checks after
the audit or Cartographer schema changes. Compile the affected encounter,
radio, Pokédex, and DexNav objects, then build one HNS release ROM.

Playtest fresh Johto starts at day and night, Surf, all three rod qualities,
Headbutt, Rock Smash, the Olivine and Cianwood Chinchou sources, every native-HM
anchor route, one transition route, Mt. Silver, National Park, and the Johto
Safari Zone. Record map, time, method, rod quality, Rating, attempts, and
observed species. Confirm special acquisitions and encounter rates did not
change.

## References

- [Kanto wild encounter specification](kanto-wild-encounters.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Standard Rod fishing](standard-rod-fishing.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Wild encounter generator](../../game/tools/wild_encounters/wild_encounters_to_header.py)
- [Wild encounter runtime](../../game/src/wild_encounter.c)
- [Radio runtime](../../game/src/pokenav_radio.c)
