# Regional and expanding National Pokédex

## Intent

Wayfarer needs one Pokédex that can describe the Pokémon the player can pursue
now without losing the canonical National identity of every supported species.
The player begins with one regional Pokédex catalog. A National Pokédex upgrade
then shows that catalog together with each regional catalog that has been
granted as a National extension. Seen and caught records remain one global
record, so discovering a Pokémon before it becomes visible still counts when
its catalog is later added.

This replaces the HNS Obtainable Dex. It is a catalog and progression feature,
not an encounter-selection feature.

## Design

### Catalogs and identifiers

The game has exactly one active regional Pokédex catalog. A catalog is an
ordered list of local entry keys, each of which resolves to one canonical
National ID. It controls the regional mode's list, numbers, progress totals,
and completion check. It does not create a second seen or caught bitset.

Canonical National IDs cover every enabled official National entry through Gen
9, ending at official National ID 1025. The enabled registry also has separate
regional-form entry keys above the official range, through 1080. Those keys
are valid catalog entries where configured and are not renumbered into the
official 1 through 1025 sequence. UI, sorting, storage, and bounds checks use
`NATIONAL_DEX_COUNT`, rather than a literal 1025 or 1080.

The first shipped active catalog is Johto. Kanto, Johto, and Hoenn catalogs are
authored declaratively. One region ID identifies a catalog everywhere: it
selects the active regional catalog and, when granted, identifies the same
catalog as a National extension. The extension mask uses that region ID's bit;
there is no second matching extension enum. Later region work registers one
region ID and catalog without replacing the Pokédex architecture.

The first Kanto, Johto, and Hoenn registrations preserve their existing
authored local orders and National mappings exactly. This initiative does not
retune, reorder, add, or remove a regional entry while turning those tables
into catalogs.

### National progression

The player has one National upgrade. Before the upgrade, the Pokédex presents
only the active regional catalog. After the upgrade, National mode is available
and its visible membership is the deduplicated union of:

1. the active regional catalog; and
2. every granted National extension catalog.

An upgrade by itself is valid. It shows only the active regional catalog until
an extension is granted. An extension can be granted before or after the
upgrade, in any order. Granting an extension more than once changes nothing.
If two catalogs contain the same National ID, it appears once in National mode
and contributes once to National seen, caught, and completion totals.

The active regional catalog remains the regional-mode authority after a
National upgrade. Extensions do not change the selected regional list or its
local numbering.

### Progress and completion

Seen and caught state is global and keyed by canonical National ID. Catalog
membership only determines whether an entry is displayed and counted in the
current regional or National view. It never clears, copies, or reassigns
progress when a catalog is selected or an extension is granted.

For every visible list, the seen and caught totals count distinct visible
entries with the corresponding global flag. Completion evaluates the same
deduplicated visible set, applying the existing `dexNotRequired` and mythical
requirement rules to that set. A newly granted extension can reveal progress
the player already earned and can increase the denominator.

### Critical captures

Wayfarer enables real critical captures. The critical-capture probability uses
the same current visible Pokédex set for both progress values. Before the
National upgrade, the numerator is the number of distinct caught entries in
the active regional catalog and the denominator is that catalog's distinct
visible-entry count. After the upgrade, the numerator is the number of
distinct caught entries in the visible National union and the denominator is
that union's distinct visible-entry count.

The National union is the active catalog plus granted extension catalogs,
deduplicated by canonical National entry ID. Caught entries outside the current
visible set do not contribute to the critical-capture numerator. An extension
can increase the denominator without adding caught entries, which can
temporarily lower the critical-capture odds. That is intentional.

An empty or invalid visible set never produces a real critical capture. The
existing Catching Charm multiplier remains part of the probability calculation.
The Gen 9 owned-capture presentation rule remains separate: a successful
capture of an already caught species can appear critical under
`B_CRITICAL_CAPTURE_IF_OWNED = GEN_LATEST`, even when the real
critical-capture probability roll did not succeed. It does not change that
roll's probability.

### Initial integration

The current Wayfarer Professor Oak interaction is the only gameplay wiring in
this initiative. Its existing National Dex award becomes an idempotent
National upgrade, then it grants Kanto with the normal extension API. With
Johto active, Oak's resulting National list is the Johto and Kanto union. If
the Kanto grant fails after a successful upgrade, National mode remains
enabled with the active regional catalog only. This partial state is harmless
and no special transaction or rollback is required.

The standard pokeemerald-expansion-style Pokédex UI already in the tree is the
active presentation. It retains the normal list, search, information, area,
cry, size, and sorting flows. The HNS-specific HGSS renderer is retired. A
future backport of the current official pokeemerald-expansion HGSS renderer is
a separate, postponed visual improvement that must consume this same catalog
facade.

## Boundaries

- This initiative does not select the player's origin or active catalog from a
  new-game choice. Johto is initialized as the active catalog for now.
- It does not wire Birch, automated travel, region arrival, badge milestones,
  or other professor interactions to grant an extension.
- It does not change wild encounters, randomizer species availability, species
  data, encounter-selection behavior, or battle mechanics other than the real
  critical-capture behavior defined here.
- It does not preserve prerelease HNS behavior or save layouts. No migration,
  compatibility branch, or obsolete-save conversion is required.
- It does not restore the old HNS HGSS presentation or promise feature parity
  with it while the standard UI is active.

## Presentation

The player sees familiar regional and National modes. Regional mode means the
one selected regional catalog. National mode means the current visible union,
not an all-species list unless the active catalog and granted extensions happen
to cover all enabled entries. The interface calls these "regional Pokédex
catalogs" and does not reuse terms from another feature.

Unknown, out-of-range, disabled, or malformed catalog entries are never shown
as a Pokémon. A failed script grant leaves the player's current catalog,
extensions, National-upgrade state, and global progress unchanged. Repeating a
valid grant is successful and leaves the same state in place.

## Interactions

The general Pokédex possession flag remains the start-menu gate. The National
upgrade remains the mode gate. The active-catalog selection and extension mask
decide membership behind common APIs. The standard UI, Summary screen, ratings,
completion checks, trades, start menu, credits, trainer-card counts, search,
and future renderers must not reimplement union logic or retain an old
hardcoded rule.

Critical-capture code obtains its caught and total values from the same common
facade. It does not choose a denominator from the build identity, the retired
Obtainable Dex, or a separate local-versus-National configuration switch.

Randomizer policy remains independent. A randomizer may choose an enabled
species that is absent from the current Pokédex membership, and a visible
Pokédex entry does not make that species randomizer-eligible.

## Constraints

- Persistent state is one active regional catalog identifier and one `u32`
  National-extension mask, plus the pre-existing Pokédex state. No per-region
  seen or caught arrays are added.
- Every use of a Pokédex flag validates a canonical ID first. Valid canonical
  IDs are inclusive from 1 through `NATIONAL_DEX_COUNT`; 0, sentinels, and
  values above the count are invalid.
- Catalog, sort, list, and completion iteration use inclusive canonical
  bounds and preserves the end entry.
- The canonical alphabetical, height, and weight tables must contain each
  enabled canonical ID exactly once and no invalid or duplicate ID.
- The standard renderer's National-sized allocations must fit the active
  EWRAM heap with its normal graphics, windows, sprites, and task allocations.
  A test must demonstrate this rather than treating the existing allocation as
  evidence of safety.
- `B_CRITICAL_CAPTURE` is `TRUE`. `B_CRITICAL_CAPTURE_IF_OWNED` remains
  `GEN_LATEST` as a successful-capture presentation rule, not a replacement
  for or input to the real critical-capture roll.

## Playtesting

Playtesting should verify that a fresh Johto catalog is understandable before
the upgrade, that Oak visibly changes the National list to the Johto plus
Kanto union, and that an entry seen before its extension is granted appears
with its existing progress. It should also check that the standard UI's search,
sort, information, area, cry, and size screens remain usable with four-digit
National numbers and regional-form entry keys.
It should also check that a regional extension can visibly reduce real
critical-capture odds by increasing the progress denominator, while an
already-owned successful capture still uses its separate presentation rule.

## References

- [Canonical National Pokédex foundation](../specs/canonical-national-pokedex.md)
- [Regional Pokédex catalogs and National extensions](../specs/regional-pokedex-catalogs.md)
- Current standard UI allocation: `game/src/pokedex.c`, `struct PokedexView`
  at line 173 allocates `NATIONAL_DEX_COUNT + 1` entries.
- Current HNS configuration: `game/include/config/pokedex_plus_hgss.h` enables
  `POKEDEX_PLUS_HGSS` and forces `SEPARATE_OBTAINABLE_DEX` for HNS.
- Current HNS renderer: `game/src/pokedex_plus_hgss.c`, `CreatePokedexList`,
  iterates the Obtainable Dex and contains its own membership checks.
- Current mapping and persistence paths: `game/src/pokemon.c` contains the
  regional and Obtainable conversion tables; `game/src/event_data.c` owns the
  current National-upgrade persistence helper.
- Current critical-capture calculation and presentation: `game/src/battle_script_commands.c`
  contains `CriticalCapture` and `FinalizeCapture`; `game/include/config/battle.h`
  currently defines the critical-capture and Catching Charm configuration.
