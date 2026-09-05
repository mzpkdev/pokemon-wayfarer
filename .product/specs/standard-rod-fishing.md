# Standard Rod fishing progression

PRD: [Standard Rod fishing progression](../prds/standard-rod-fishing.md)
Implemented: Yes

## Scope

This specification defines the three global Standard Rod weight profiles, the
runtime selection order, order-independent upgrades from the nine existing rod
givers, and the validation needed for Emerald, FireRed, LeafGreen, and HNS. It
also defines the changes required in ordinary fishing population readers and
development tools.

It does not change authored fishing entries or levels, giver locations and
access gates, the fishing animation or reeling minigame, the special Route 119
Feebas encounter, or pre-feature save compatibility.

## Behavior

### Canonical quality profiles

Every fishing quality uses all ten entries in the active map and time-of-day
fishing table. Entries 0 and 1 remain the common band, entries 2 through 4 the
less-common band, and entries 5 through 9 the rare band. These bands are
metadata and do not limit selection.

The exact weights are:

| Quality | Entry 0 | Entry 1 | Entry 2 | Entry 3 | Entry 4 | Entry 5 | Entry 6 | Entry 7 | Entry 8 | Entry 9 | Total |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Old Rod | 38 | 22 | 10 | 8 | 8 | 4 | 3 | 3 | 2 | 2 | 100 |
| Good Rod | 25 | 18 | 12 | 10 | 9 | 7 | 6 | 5 | 4 | 4 | 100 |
| Super Rod | 12 | 10 | 11 | 10 | 10 | 10 | 10 | 9 | 9 | 9 | 100 |

Add `game/src/data/standard_rod_fishing.json` as the single authored source for
these profiles and the recovery cases defined below. Its top-level shape is:

- `schemaVersion`, which must equal 1.
- `qualityWeights`, with exactly the keys `OLD_ROD`, `GOOD_ROD`, and
  `SUPER_ROD` and the corresponding vectors above.
- `nativeSurfAccessibility`, the exact recovery-case records defined below.

Keep the existing `fishing_mons.groups` object in `wild_encounters.json` as
rarity-band metadata. Its single legacy `encounter_rates` vector no longer
drives fishing selection or display. The encounter generator must validate
that each quality vector has exactly ten positive integer values that fit in
`u8` and totals 100.

The generator emits the validated profiles for the ROM and uses the same parsed
values for its balance report and Cartographer projection. The profiles must
not also be maintained as handwritten C or Python constants.

### Runtime population selection

Resolving a fishing profile requires a valid Old, Good, or Super Rod quality.
For every valid quality, the profile view covers authored entries 0 through 9
and points to that quality's generated weights. The rod identity remains part
of the profile context for quality selection and any existing fishing level
offsets.

For a successful normal fishing encounter, runtime selection proceeds in this
order:

1. Resolve the active map, time-of-day fishing table, and the current rod's
   ten-entry profile.
2. Calculate Trainer Rating eligibility for every entry. An entry with
   `SPECIES_NONE` is ineligible in both normal and randomized modes.
3. Sum the current quality's weights for eligible entries only. If the total is
   zero, report that the current spot has no fishing encounter and do not enter
   the hooked sequence.
4. Roll once directly over that eligible total. There is no preliminary roll
   for a former rod partition.
5. If a Lure is active, retain the existing 20 percent reversal check. Mirror
   the selected position across the ordered sequence of eligible entries, not
   across fixed raw indices. An ineligible entry can never be restored by the
   mirror.
6. Roll the selected entry's authored level, apply Trainer Rating level
   projection and predecessor resolution, and create the encounter through the
   existing ordinary fishing path.

Removing ineligible entries therefore renormalizes the remaining profile
weights. A selected empty or locked entry never consumes a successful bite.

Randomized mode continues to bypass predecessor and species-floor filtering,
apart from always excluding `SPECIES_NONE`. It receives the selected authored
species and the raw entry index from 0 through 9. The index is never converted
to a rod-relative value.

The existing Route 119 Feebas check remains before normal profile selection.
When it succeeds, every rod quality creates the existing authored Feebas
encounter without consulting or changing the unified selector.

### Bite chance and fishing modes

The base bite chances remain 25 percent for Old Rod, 50 percent for Good Rod,
and 75 percent for Super Rod. Existing follower, proximity, time, Sticky Hold,
Suction Cups, and configured fishing modifiers retain their current order and
100 percent cap.

Easier Fishing continues to skip the reactive reeling sequence. Manual fishing
retains the timing and repeat-round behavior associated with the displayed rod
quality. Item use continues to derive that quality from the existing Old Rod,
Good Rod, or Super Rod item.

The fishing-data availability check must use the current rod profile. It
returns false when the table is absent, its encounter rate is zero, or its
eligible weight total is zero. The player must not reach the hooked message at
such a spot.

### Givers and contribution flags

The existing three givers in each build are the only contributors:

| Build | Contributor | Permanent contribution flag |
| --- | --- | --- |
| Emerald | Dewford Town fisherman | `FLAG_RECEIVED_OLD_ROD` |
| Emerald | Route 118 fisherman | `FLAG_RECEIVED_GOOD_ROD` |
| Emerald | Mossdeep City House 3 fisherman | `FLAG_RECEIVED_SUPER_ROD` |
| FireRed and LeafGreen | Vermilion City House 1 Fishing Guru | `FLAG_GOT_OLD_ROD` |
| FireRed and LeafGreen | Fuchsia City House 2 Fishing Guru's brother | `FLAG_GOT_GOOD_ROD` |
| FireRed and LeafGreen | Route 12 Fishing House Fishing Guru's brother | `FLAG_GOT_SUPER_ROD` |
| HNS | Route 32 Pokémon Center Fishing Guru | `FLAG_STANDARD_ROD_ROUTE32_CONTRIBUTED` at `0x304` |
| HNS | Olivine City House 3 fisherman | `FLAG_STANDARD_ROD_OLIVINE_CONTRIBUTED` at `0x305` |
| HNS | Route 12 house fisherman | `FLAG_STANDARD_ROD_ROUTE12_CONTRIBUTED` at `0x306` |

Emerald and FireRed/LeafGreen retain their current giver flags. HNS renames the
three adjacent unused general-purpose flags at `0x304` through `0x306`. It must
not use rod possession or `FLAG_GOT_SUPER_ROD` as contribution state. The S.S.
Aqua transition currently clears `FLAG_GOT_SUPER_ROD`; it must not clear any of
the three new contribution flags.

Each giver script checks only its own contribution flag to decide whether it is
unused. If it is already set, the giver uses repeat dialogue or its existing
follow-up activity. If it is unset, the giver offers a contribution. Declining
changes nothing.

The number of set contribution flags in the active build determines a
successful award:

| Flags already set | Required current rod | Award |
| ---: | --- | --- |
| 0 | None | Add Old Rod |
| 1 | Old Rod | Replace it with Good Rod |
| 2 | Good Rod | Replace it with Super Rod |

The giver's original rod tier does not affect the award. All six visit orders
within a build produce Old, then Good, then Super quality.

### Atomic rod transaction

One C helper owns the complete award transaction. A giver supplies its
build-specific contribution flag, and the helper returns one of these outcomes
plus the awarded item ID on success:

- Success.
- Giver already contributed.
- No Key Items space for the first award.
- Invalid rod state.

Before changing state, the helper validates the contribution flag, contributor
count, required current rod, and absence of either other rod item. It does not
repair or migrate an inconsistent save.

For the first contribution, the helper adds one Old Rod through the ordinary
Bag API. If no Key Items slot is available, it returns the no-space result and
changes no state.

For the second or third contribution, the helper locates the current lower rod
in the Key Items pocket and validates that its decoded quantity is exactly one.
It replaces that slot through `BagPocket_SetSlotItemIdAndCount`, or an equivalent
helper that preserves the Bag's quantity encryption, with the next rod and
quantity one. It does not write the encrypted slot fields directly, compact the
pocket, remove and then add through separate commands, or require an empty Key
Items slot.

If either `registeredItem` or `registeredItemHold` equals the replaced rod, the
helper changes that field to the awarded rod. If both fields match, it changes
both. It then sets the contributor flag last. All validation happens before the
Bag, registration, and flag writes, so an error leaves all three unchanged.

Scripts use the returned item ID for the normal item-received presentation and
quality-specific success text. A no-space result uses the ordinary Bag-full
message and remains retryable. An invalid state uses a neutral failure message,
sets no flag, and remains retryable after the state is corrected.

### Content and follow-up behavior

Update the three rod item descriptions so they describe one shared population:

- Old Rod can catch any Pokémon in the local fishing population.
- Good Rod improves bite reliability and the chance of uncommon catches.
- Super Rod provides the best bite reliability and chance of rare catches.

Every giver keeps their regional introduction and personality, but their offer
and success branches must work for all three possible awards. Dialogue must not
name the location as a fixed tier, claim that Old Rod catches only Magikarp, or
claim that different rods unlock exclusive species.

Keep Dewford's fishing tutorial and make its mechanical explanation correct at
any awarded quality. Keep Olivine's seaside setting and the fisherman's 30 years
of experience. In FireRed and LeafGreen, a successful contribution from the
Route 12 giver unlocks the Magikarp size-record activity through that giver's
existing contribution flag, regardless of the quality awarded.

The player-facing item names, icons, fanfares, fishing animations, and item-use
flow remain Old Rod, Good Rod, and Super Rod. The term Standard Rod is not added
to the interface.

### Population readers and tools

Pokédex area checks treat all ten eligible fishing entries as one location
population. They may resolve that population through one rod quality rather
than repeating identical roster checks for all three qualities. They do not
show exact probabilities.

Cartographer keeps separate Old, Good, and Super views because their odds
differ. Each view contains all ten entries, applies its quality's generated
weights, shows eligibility and renormalized probability, and labels the former
groups as rarity bands rather than exclusive rod partitions.

Bump the generated Cartographer projection to schema version 2. Every fishing
profile row includes a `weights` array copied from its quality profile and has
`runtimeSlotCount` equal to 10. Non-fishing profile rows keep their existing
shape. The catalog validates the ten weights and the slot count, then the UI
calculates eligible totals and displayed probabilities from that profile row.
It no longer derives fishing slots or odds from `fishing_mons.groups` or the
legacy shared `encounter_rates` vector.

Each `nativeSurfAccessibility` record has exactly these fields:

- `product`: `EMERALD`, `FIRERED`, `LEAFGREEN`, or `POKEMON_HNS`.
- `baseLabel`: the exact `base_label` in `wild_encounters.json`.
- `timeOfDay`: the resolved runtime time constant.
- `species`: the exact species constant to aggregate across duplicate entries.
- `expectedOldRodSuccessfulEncounterPercent`: the exact Lure-off result with
  all required entries eligible.
- `minimumOldRodSuccessfulEncounterPercent`: 8.
- `minimumOldRodUnmodifiedCastPercent`: 2.

The file contains one record for every label, time, and species combination in
this table. A row with several labels, times, or species expands to their full
Cartesian product except where separate expected values are shown.

| Product | Base labels | Times | Species and expected successful Old Rod chance |
| --- | --- | --- | --- |
| `FIRERED` | `sPalletTown_FireRed`, `sCinnabarIsland_FireRed` | `TIME_DAY` | `SPECIES_HORSEA` 14%, `SPECIES_KRABBY` 8% |
| `LEAFGREEN` | `sPalletTown_LeafGreen`, `sCinnabarIsland_LeafGreen` | `TIME_DAY` | `SPECIES_HORSEA` 8%, `SPECIES_KRABBY` 14% |
| `POKEMON_HNS` | `gOlivineCity_PortOutside_hns_Day`, `gOlivineCity_PortOutside_hns_Night`, `gVermilionCity_hns_Day`, `gVermilionCity_hns_Night`, `gVermilionCity_PortOutside_hns_Day`, `gVermilionCity_PortOutside_hns_Night`, `gCinnabarIsland_hns_Day`, `gCinnabarIsland_hns_Night` | Time encoded by the label | `SPECIES_CHINCHOU` 11% |
| `POKEMON_HNS` | `gCianwoodCity_hns_Day` | `TIME_DAY` | `SPECIES_CHINCHOU` 12% |
| `EMERALD` | `gLilycoveCity` | `TIME_DAY` | `SPECIES_WAILMER` 19% |
| `EMERALD` | `gMossdeepCity`, `gPacifidlogTown` | `TIME_DAY` | `SPECIES_WAILMER` 18% |

For HNS labels ending in `_Day` or `_Night`, each record stores the resolved
`TIME_DAY` or `TIME_NIGHT` value respectively. The generator rejects duplicate
records, unknown profile identities, a product or time mismatch, a species not
authored in the profile, or a computed Lure-off result that differs from the
recorded expected percentage.

For Wayfarer, the wild encounter generator's deterministic balance report
covers every included version, map profile, and time-of-day variant at every
integer Trainer Rating from 0 through 80. For every quality it reports:

- Raw entry weights and eligible entries.
- Renormalized entry and aggregate species probabilities.
- Lure-off and Lure-on probabilities using the production eligible-sequence
  mirror rule.
- Probability per successful encounter and per unmodified cast using the 25,
  50, and 75 percent base bite rates.
- Expected unmodified casts for Horsea and Krabby at the named FireRed and
  LeafGreen sources, Chinchou at the named HNS sources, and Wailmer at the named
  Emerald sources.

The report consumes `nativeSurfAccessibility` rather than hardcoded profile
names. Generation fails if profile shape or totals drift, an eligible entry has
zero weight, any Old Rod entry falls below the PRD's minimum, or a listed
native Surf species differs from its expected result or falls below 8 percent
per successful Old Rod encounter or 2 percent per unmodified cast.

### Validation

Automated validation must cover:

- Exact profile values, totals, ten-entry views, and every weighted-roll
  boundary for all three qualities.
- Empty-entry exclusion, Trainer Rating filtering, renormalization, and Lure
  mirroring for both full and filtered profiles.
- Raw slot handoff to the randomizer, zero-data spots, rod-independent Feebas,
  and the unchanged bite probabilities and modifier cap.
- Pokédex and Cartographer inclusion of species from each former rarity band
  for every quality.
- All six giver visit orders in each build, refusal, repeat interaction, and
  quality-specific presentation.
- A full-pocket first award failure, full-pocket Good and Super upgrades, Bag
  slot reuse, both registered-item shortcuts, invalid-state rollback, and
  contributor flag ordering.
- Persistence of all three HNS contributions through the S.S. Aqua transition
  and the FireRed/LeafGreen Route 12 Magikarp activity after any successful
  award.
- Deterministic generator output and all accessibility thresholds in the
  parent PRD.

Run generator and mechanics tests for Emerald, FireRed, LeafGreen, and HNS
sequentially because the builds share generated map files. Compile the affected
encounter, fishing, Bag, item-use, Pokédex, and map-script objects for all four
builds, then build at least one complete release ROM. Playtest each quality,
one nonstandard giver order per build, the two registered-item shortcuts, the
FRLG Magikarp follow-up, the HNS regional transition, a filtered fishing table,
and Route 119 Feebas.

## References

- [Trainer Rating wild encounter scaling specification](trainer-rating-wild-encounter-scaling.md)
- [Authored under-level wild encounters](../research/authored-under-level-wild-encounters.md)
- [Wild encounter data](../../game/src/data/wild_encounters.json)
- [Wild encounter runtime](../../game/src/wild_encounter.c)
- [Fishing runtime](../../game/src/fishing.c)
- [Bag implementation](../../game/src/item.c)
