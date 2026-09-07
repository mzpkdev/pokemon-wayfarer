# Canonical National Pokédex foundation

PRD: [Regional and expanding National Pokédex](../prds/regional-national-pokedex.md)
Implemented: Yes

## Scope

This is the first dependency of the regional-catalog feature. It removes the
HNS Obtainable Dex abstraction, activates the standard Pokéemerald Expansion
style renderer already present in the tree, and establishes one safe canonical
National-ID facade. It does not add regional catalogs, extension persistence,
or Oak's Kanto grant. Those are owned by
[Regional Pokédex catalogs and National extensions](regional-pokedex-catalogs.md).

This spec also removes the HNS Obtainable-Dex dependency from the real
critical-capture calculation and creates the common progress snapshot it will
consume. Spec 2 supplies the active-catalog and National-union policy, enables
the final real critical-capture configuration, and tests catalog-dependent
odds.

Trade eligibility is not a consumer of this facade. Spec 2 removes the old
Pokédex-membership trade gates and specifies receipt progress for catalog-hidden
Pokémon.

The resulting foundation may be developed and tested before the second spec,
but it is not a separate player-facing release milestone. Until Spec 2 changes
the facade policy, its foundation policy considers every valid canonical ID
visible. The final product behavior is the regional and extension union from
the parent PRD.

## Behavior

### Renderer and retired architecture

Set `POKEDEX_PLUS_HGSS` to `FALSE`, so `CB2_OpenPokedex` and new-entry display
take the standard renderer in `game/src/pokedex.c`. Keep its existing normal
list, search, information, area, cry, and size flows. Delete the HNS-specific
HGSS renderer and its build references after every caller uses the standard
path. Remove the HNS-only Summary shortcut that opens an HGSS entry, or adapt
it to open the standard renderer at the selected canonical National ID. It
must never retain an HGSS-only entry-point dependency.

The later import of the current official pokeemerald-expansion HGSS renderer
is explicitly deferred. It is a renderer replacement only and must consume the
facade defined here. Do not port it in this specification.

### Canonical identifiers and bounds

Delete `enum ObtainableDexOrder`, `OBTAINABLE_DEX_COUNT`, the conditional
`SEPARATE_OBTAINABLE_DEX` configuration, Obtainable conversion declarations,
tables, and all call sites. Replace them with `enum NationalDexOrder` and
`NATIONAL_DEX_COUNT`. Remove the duplicate HNS conversion implementation.

`NATIONAL_DEX_COUNT` is the inclusive maximum valid canonical ID. All loops
over canonical IDs use `for (id = 1; id <= NATIONAL_DEX_COUNT; id++)` or an
equivalent inclusive iterator. No code may use `< NATIONAL_DEX_COUNT` when it
means to cover the canonical ID domain. Buffers use `NATIONAL_DEX_COUNT + 1`
only where slot 0 is deliberately unused.

Add one internal validation helper and make every public facade function use
it before reading or writing a Dex flag. It accepts only `1 <= id <=
NATIONAL_DEX_COUNT`. Invalid IDs, `NATIONAL_DEX_NONE`, zero, sentinels, and
IDs above the configured count return false or zero for queries and do nothing
for flag writes. They must not index a flag array, a species table, or a sort
table.

The first executable operation in `GetSetPokedexFlag` is
`if (!Dex_IsValidNationalId(nationalDexNo)) return 0;`. It occurs before any
decrement, index, bit, mask, switch, or Dex-flag pointer access. Caller-side
validation is additional defense, not a substitute. For an invalid ID it
returns zero, makes no flag write, and leaves both seen and caught arrays
unchanged for every get, set, or unknown case value. This prevents ID zero from
underflowing before the bit-array access.

All species-to-Dex conversion consumers validate their result before flag
access. The implementation must not assume that a species conversion, a
catalog conversion, or a sort table always produced a valid ID.

### Common National visibility facade

Introduce the following public, renderer-independent interface in
`include/pokedex.h` or a focused new Pokédex common header. The implementation
may add internal helpers, but consumers use this interface rather than their
own membership loops.

```c
bool8 Dex_IsValidNationalId(enum NationalDexOrder id);
bool8 Dex_IsNationalEntryVisible(enum NationalDexOrder id);
u16 Dex_GetNationalVisibleEntryCount(void);
enum NationalDexOrder Dex_GetFirstVisibleNationalEntry(void);
enum NationalDexOrder Dex_GetNextVisibleNationalEntry(enum NationalDexOrder after);
u16 Dex_GetNationalVisibleProgress(u8 flagCase);
bool8 Dex_HasCompletedNationalVisibleEntries(void);
```

`Dex_GetFirstVisibleNationalEntry` returns `NATIONAL_DEX_NONE` for an empty
set. `Dex_GetNextVisibleNationalEntry(NATIONAL_DEX_NONE)` returns the first
visible ID and is the only restart operation. For a valid nonzero `after`, it
returns the least visible ID strictly greater than `after`, or
`NATIONAL_DEX_NONE` when none exists. Any nonzero invalid `after`, including a
value above `NATIONAL_DEX_COUNT` or a sentinel, returns `NATIONAL_DEX_NONE`.
It never restarts, wraps, or returns the first entry for an invalid iterator.
This rule also applies when the cursor came from a malformed catalog conversion
or sort-table value. All current and future common-facade iterators use only
`NATIONAL_DEX_NONE` as their explicit restart token.

At this stage, the visibility policy returns true for every valid canonical ID.
Spec 2 replaces only that policy and the count/iteration implementation with
catalog union membership. Keep the interface, callers, and validation rules
unchanged.

`Dex_GetNationalVisibleProgress` accepts only `FLAG_GET_SEEN` and
`FLAG_GET_CAUGHT`; any other case returns zero. It counts each visible ID once.
`Dex_HasCompletedNationalVisibleEntries` evaluates precisely the visible set
and uses the existing `dexNotRequired` and mythical requirement exceptions.
The empty set is not a completion result.

### Critical-capture foundation

Retire `B_CRITICAL_CAPTURE_LOCAL_DEX` from Wayfarer's critical-capture
calculation. It must not select a compile-time regional count, a National
count, or an Obtainable-Dex count. Delete the project dependency on
`OBTAINABLE_DEX_COUNT` and `GetNationalPokedexCount` from
`CriticalCapture`.

Add a common facade that takes one consistent caught and visible-count snapshot:

```c
bool8 Dex_GetCriticalCaptureProgress(u16 *caughtCount, u16 *visibleCount);
```

It returns `FALSE` and writes zero to both outputs when the relevant visible
set is invalid or empty. It returns `TRUE` only when both values describe the
same valid visible set. Spec 2 defines that set from the current upgrade state.
The battle code treats a `FALSE` result as no real critical capture and does
not perform a threshold calculation or random roll.

Extract the existing threshold and odds transformation into a pure,
test-visible helper. It receives the caught count, visible count, base catch
odds, and Catching Charm state, and returns whether a real critical roll is
possible plus the final roll threshold. It preserves the existing strict bands
at `30`, `150`, `300`, `450`, and `600` over `650`; the existing `50%`,
`100%`, `150%`, `200%`, and `250%` multipliers; the 255 cap; and the division
by 6. The Catching Charm continues to use
`(100 + B_CATCHING_CHARM_BOOST) / 100` exactly as it does now.

Spec 1 must not turn on a behavior that uses its temporary all-canonical
foundation policy. Spec 2 sets `B_CRITICAL_CAPTURE = TRUE` when the catalog
facade has its final regional and National-union semantics.

`B_CRITICAL_CAPTURE_IF_OWNED = GEN_LATEST` stays unchanged. It is not an
input to the helper or real probability roll. `FinalizeCapture` retains it as
the separate Gen 9 rule that gives a successful capture of an already caught
species the critical presentation.

Route all National list creation, National search filtering, progress displays,
trainer-card count, ratings, completion logic, Summary display and shortcut
handling, credits, and National completion consumers through this facade. The
start-menu Pokédex action must always enter the standard renderer and retain no
HGSS dispatch. Trade eligibility must not query this facade or a replacement
membership predicate. Trade receipt may validate a converted canonical ID and
record its global flags, as Spec 2 defines. The generic regional consumers may
continue to use their existing regional path until Spec 2 replaces it with the
active-catalog facade.

### Lists, sort tables, and UI capacity

The standard UI uses `NATIONAL_DEX_COUNT` and the common iterator for every
National list order. Numerical order lists visible entries in increasing
canonical ID. Alphabetical, weight, and height order walk their canonical
tables, discard invalid table values, and include an ID only when the common
visibility predicate permits it. Seen and caught filters still apply to the
corresponding sorted list operation.

Repair `gPokedexOrder_Alphabetical`, `gPokedexOrder_Height`, and
`gPokedexOrder_Weight` in `game/src/data/pokemon/pokedex_orders.h`. Each table
must have exactly `NATIONAL_DEX_COUNT` entries, contain every valid canonical
ID exactly once, contain no zero, no out-of-range entry, and no duplicate. A
table error must fail a host-side validation test rather than silently produce
a missing, repeated, or unsafe UI row.

Add focused host tests under `game/test/` for flag bounds, canonical iteration,
sort-table validation, progress, completion, and randomizer separation. The
current `pokedex_area.c` coverage is not a replacement for these tests.

Use `NATIONAL_DEX_COUNT` for every list allocation, sentinel initialization,
scroll boundary, digit-width decision, and Summary display. A four-digit
display is required when the count exceeds 999. The Summary screen must use
canonical IDs and `NATIONAL_DEX_COUNT`, never the retired Obtainable count.

Add a focused memory-capacity test for the standard renderer. In the configured
Wayfarer build, it must open National mode with a
`NATIONAL_DEX_COUNT + 1` list while the normal standard-renderer graphics,
windows, sprites, tasks, and search state are allocated. Instrument the test
build so an EWRAM heap-allocation failure fails the test. The test must open,
navigate, enter and exit an entry page, enter and exit search, and close the
Pokédex without allocation failure, memory corruption, or a leaked allocation.
It records the peak heap use and verifies that it does not exceed the active
EWRAM heap limit of `0x1C500` bytes. The HNS build also emits the linker's
`--print-memory-usage` report; the test record includes its EWRAM headroom.

### Randomizer separation

Remove randomizer dependencies on Obtainable Dex membership and do not replace
them with National visibility checks. Randomizer candidate selection stays
governed by its own enabled-species and randomizer policy. A test must prove
that altering the visible-membership predicate cannot alter randomizer
candidate eligibility, and that randomizer code no longer names an Obtainable
symbol.

### Failure handling

Malformed sort data is a build or host-test failure. A runtime invalid ID is a
safe empty result as specified above. A UI list that receives no visible entry
must render an empty list and remain navigable to exit; it must not calculate a
negative last selection, dereference a sentinel, or open an information page.
Deleting the HGSS renderer requires removal of its assets only when no build
target references them. If a retained non-Dex asset still has another owner,
leave that asset in place.

### Validation

Implementation is accepted only when all of the following pass:

1. A clean HNS Wayfarer build compiles with `POKEDEX_PLUS_HGSS` false and has
   no live `SEPARATE_OBTAINABLE_DEX`, `OBTAINABLE_DEX_COUNT`,
   `ObtainableDexOrder`, `OBTAINABLE_DEX_*`, `NationalToObtainableOrder`,
   `ObtainableToNationalOrder`, `SpeciesToObtainablePokedexNum`,
   `sObtainableToNationalOrder`, or `OBTAINABLE_TO_NATIONAL` symbol. An
   intentional removal test may name a retired symbol only as text, never as a
   compiled compatibility path.
2. Opening the start-menu Pokédex and a caught-entry Pokédex page reaches the
   standard renderer. The old `pokedex_plus_hgss.c` implementation is absent
   from the build and no Summary shortcut calls it.
3. Numerical National iteration contains IDs 1 and `NATIONAL_DEX_COUNT`, in
   order, exactly once. It contains neither zero nor an ID above the count.
4. The three canonical sort tables each contain exactly the valid canonical
   domain once. A fixture with a zero, duplicate, missing final ID, or
   out-of-range ID fails the table validator.
5. `Dex_IsValidNationalId`, flag access, first/next iteration, visible count,
    progress, and completion safely reject 0, `NATIONAL_DEX_NONE`, a sentinel,
    and `NATIONAL_DEX_COUNT + 1`; none reads or writes a Dex flag. Direct
   `GetSetPokedexFlag` tests verify the same outcome for every get and set
   case, plus an unknown case, without relying on caller validation. They
   compare the seen and caught arrays byte-for-byte before and after and use an
   instrumented accessor to prove that invalid IDs cause no array read or
   write.
6. With the Spec 1 policy, the visible count is `NATIONAL_DEX_COUNT`; each
   valid canonical ID is visible once; global seen and caught progress uses the
   matching canonical flag.
7. Search and each numerical, alphabetical, height, and weight list include
   only visible entries and preserve the relevant seen/caught filter. The last
   enabled canonical entry and a configured entry above official 1025 can be
   displayed safely.
8. Summary displays the canonical number with the correct width and uses
   question marks for an invalid conversion without accessing a Dex flag.
9. Trainer-card National count and National completion agree with the common
   facade for fixtures with required, exempt, mythical, seen, and caught
   entries.
10. Randomizer candidate tests pass unchanged when a test seam suppresses a
    visible Pokédex entry, proving membership is not its availability policy.
11. The start-menu action opens the standard renderer; Summary, ratings,
    completion, credits, and trainer-card fixtures all use canonical IDs and
    the common facade, with no Obtainable or build-identity regional rule in
    their Pokédex paths. Trade eligibility is excluded from this facade and is
    covered by Spec 2.
12. The instrumented memory-capacity test opens and exercises the full standard
    National renderer as specified above. Every allocation succeeds, peak heap
    use stays within the `0x1C500`-byte EWRAM heap limit, the HNS link report
    records nonnegative EWRAM headroom, and teardown returns the heap to its
    pre-open state.
13. `CriticalCapture` and its helper name no `OBTAINABLE_*`, `Obtainable*`,
    `GetNationalPokedexCount`, or `REGIONAL_DEX_COUNT` symbol, and
    `B_CRITICAL_CAPTURE_LOCAL_DEX` cannot alter their denominator. A helper
    fixture with an invalid or zero visible count returns no roll without
    division or RNG use.
14. Pure-helper fixtures preserve every existing strict threshold boundary,
    multiplier, Catching Charm arithmetic, 255 cap, and divide-by-6 result for
    a supplied valid progress snapshot. They do not test probability through
    uncontrolled RNG.
15. Iterator tests verify that `NATIONAL_DEX_NONE` returns the first visible
    entry, a valid last entry returns none, and every nonzero invalid iterator
    value returns none without restarting the list. The invalid fixtures
    include `NATIONAL_DEX_COUNT + 1`, sentinels, malformed catalog conversions,
    and malformed sort-table values; a valid penultimate entry still returns
    the valid last entry.

## References

- [Regional and expanding National Pokédex](../prds/regional-national-pokedex.md)
- [Regional Pokédex catalogs and National extensions](regional-pokedex-catalogs.md)
- `game/include/config/pokedex_plus_hgss.h:4,12-16` currently enables HGSS and
  the separate Obtainable Dex for HNS.
- `game/src/pokedex.c:1588-1594` dispatches between renderers, while
  `game/src/pokedex.c:173` already allocates a National-sized standard list.
- `game/src/pokedex_plus_hgss.c:2483-2619` owns the duplicate HNS list and
  checks Obtainable or regional membership during sorting.
- `game/src/pokedex.c:4513-4699` contains current flag, count, and completion
  paths, including Obtainable-based National counting.
- `game/src/pokedex.c:4513-4539` currently decrements a National ID before it
  derives a flag-array index, so an invalid zero requires the in-function guard
  specified above.
- `game/src/trade.c:3055-3071` currently maps a received non-Egg Pokémon to a
  canonical ID and records seen and caught flags. Spec 2 keeps that global
  progress behavior while removing membership-based trade eligibility gates.
- `game/src/pokemon_summary_screen.c:3437-3457` currently uses
  `OBTAINABLE_DEX_COUNT` for number width.
- `game/src/pokemon.c:150-1437` holds current regional and Obtainable mapping
  tables; `game/src/randomizer.c:253-273` currently couples randomizer
  candidates to an Obtainable conversion.
- `game/include/malloc.h:44-45` defines the `0x1C500`-byte heap and
  `game/Makefile:695` requests the linker memory-usage report.
- `game/src/battle_script_commands.c:10938-10977` currently combines a
  compile-time regional or Obtainable denominator with an Obtainable-based
  caught count. `game/include/config/battle.h:352,356-358` contains the
  related critical-capture configuration.
