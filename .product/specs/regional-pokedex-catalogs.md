# Regional Pokédex catalogs and National extensions

PRD: [Regional and expanding National Pokédex](../prds/regional-national-pokedex.md)
Implemented: Yes

## Scope

This specification depends on
[Canonical National Pokédex foundation](canonical-national-pokedex.md). It
adds Kanto, Johto, and Hoenn catalog definitions; persistent selection and
extension state; the final regional and National membership facade; script
grant/query APIs; the catalog-dependent real critical-capture calculation; and
the one approved Oak integration. It does not change the renderer selected by
Spec 1 and does not implement origin choice, Birch, automated travel, or other
regional milestones.

## Behavior

### Declarative catalog registry

Define `enum DexRegionId` with `DEX_REGION_NONE`, `DEX_REGION_KANTO`,
`DEX_REGION_JOHTO`, `DEX_REGION_HOENN`, and `DEX_REGION_COUNT`. This is the
only region identifier. Its nonzero values select a regional catalog and map
directly to the corresponding bit in the `u32` National-extension mask. Kanto,
Johto, and Hoenn must each have a catalog registry record with:

- a stable catalog ID;
- ordered local entry keys numbered from 1 through that catalog's entry count;
- a National ID for every local entry;
- a National-extension bit; and
- a regional-mode display name.

The extension bit for a grantable region is `1u << regionId`. A registered
region ID must be nonzero, less than 32, and below `DEX_REGION_COUNT`.
`DEX_REGION_NONE`, `DEX_REGION_COUNT`, unregistered values, and values that
cannot produce one `u32` bit are invalid for both selection and grants. There is no
`DexNationalExtensionId`, extension-to-region mapping table, or duplicate
extension constant.

Catalog order is authored data, not an arithmetic assumption about National
numbers. Local entry 0 is invalid. A local entry resolves only through its own
catalog. Every declared National ID must pass
`Dex_IsValidNationalId`; duplicate IDs inside a single catalog, zero, and
out-of-range IDs fail a host-side catalog validation test.

The Kanto, Johto, and Hoenn catalog lists are not new authored content. The
registry must preserve the existing tables exactly, in local-entry order and
with the same National ID at every entry:

- Kanto: the 188 valid one-based entries in `sKantoToNationalOrder`;
- Johto: the 282 valid one-based entries in `sJohtoToNationalOrder`; and
- Hoenn: the 214 valid one-based entries in `sHoennToNationalOrder`.

The implementation may move these arrays into the catalog module, but it must
move them verbatim. It must not add, remove, reorder, substitute, deduplicate,
or normalize an entry while registering the initial three catalogs. The
registry owns deduplication only when it composes the National union across
catalogs, never within a source regional list.

The legacy `KANTO_DEX_COUNT` and `JOHTO_DEX_COUNT` array declarations include
one implicit terminal zero beyond their valid local mappings. The registry does
not register that zero as a local entry, does not expose it, and does not copy
it into a baseline. The Hoenn source already has only its 214 valid mappings.

Delete the old parallel generic conversion/count paths once consumers use the
registry. Adding a later region means adding one record with one new
`DexRegionId` value. It must not require a new compile-time feature flag, a
second active-catalog field, a separate extension enum, or a new UI path.

### Persistent state and initialization

Add exactly these new persistent fields to `struct Pokedex` in
`gSaveBlock2Ptr`, alongside the existing Pokédex mode/order metadata:

```c
u8 activeRegion; // enum DexRegionId value
u32 nationalExtensionMask;
```

Initialize shared new-save state to the Johto catalog and an empty extension
mask in `ResetPokedex`. The [starting-origin contract](wayfarer-regional-start-choice.md)
then lets the selected stock profile choose its initial catalog: Johto for
New Bark, Hoenn for Littleroot. This override runs once before play begins;
travel, Continue, and later professor receipts preserve the player's catalog
selection and extension mask. Custom profiles own any initial catalog choice
through their initializer.
Add a save-layout and save/load test that verifies both fields serialize and
restore exactly on a current-build save.

Prerelease saves are unsupported. Do not add migration code, compatibility
sentinels, or branches to interpret old HNS Obtainable-Dex state. An invalid
stored active-region value is treated as a safe initialization failure: common
regional and National queries return an empty set, script mutation refuses it,
and the UI shows an empty list that can be exited. It must not select a
different catalog, access arbitrary data, or write a fallback value during a
query. Save validation or load diagnostics may report the invalid value.

### Active regional facade

Expose these common APIs in the same public Pokédex facade introduced by Spec
1:

```c
enum DexRegionId Dex_GetActiveRegion(void);
bool8 Dex_SetActiveRegion(enum DexRegionId region);
u16 Dex_GetActiveRegionalEntryCount(void);
enum NationalDexOrder Dex_RegionalEntryToNational(u16 localEntry);
u16 Dex_NationalToRegionalEntry(enum NationalDexOrder id);
bool8 Dex_IsRegionalEntryVisible(enum NationalDexOrder id);
u16 Dex_GetRegionalVisibleProgress(u8 flagCase);
bool8 Dex_HasCompletedRegionalVisibleEntries(void);
```

`Dex_SetActiveRegion` accepts only a registered region and changes only the
active catalog. It does not grant an extension, clear global progress,
or modify the National-upgrade flag. The function returns false with no state
change for `NONE`, the count sentinel, or an unregistered value. The current
initiative initializes Johto but does not need to expose selection to ordinary
gameplay scripts.

`Dex_RegionalEntryToNational` accepts only local entries 1 through the active
catalog's entry count. Invalid local entries return `NATIONAL_DEX_NONE`.
`Dex_NationalToRegionalEntry` returns the active catalog's unique local entry
or zero when absent or invalid. Generic regional count, completion, UI list,
search filtering, area presentation, trainer-card count, rating, and number
formatting use this facade. Kanto, Johto, and Hoenn-specific helpers may remain
only for explicitly named legacy content that cannot yet be routed; each such
exception must be documented and have a removal owner. This spec must not
leave a generic consumer selecting a region through `IS_HNS`, `IS_FRLG`, or
`REGIONAL_DEX_COUNT`.

Replace the semantic `DEX_MODE_HOENN` presentation path with generic
`DEX_MODE_REGIONAL`. The standard renderer reads the active catalog's display
name from the registry for its mode label, title, and mode-selection text. It
does not choose a regional name from `IS_HNS` or `IS_FRLG`. Existing regional
background art may remain until an authored visual replacement is approved,
but its label must identify the active Kanto, Johto, or Hoenn catalog. Switching
the active catalog refreshes this presentation with the new catalog name.

### National extension facade

Expose the following APIs:

```c
bool8 Dex_GrantNationalExtension(enum DexRegionId region);
bool8 Dex_HasNationalExtension(enum DexRegionId region);
u32 Dex_GetNationalExtensionMask(void);
bool8 Dex_UpgradeToNational(void);
bool8 Dex_HasNationalUpgrade(void);
```

`Dex_GrantNationalExtension` validates the region ID and its registry record.
A valid call sets exactly `1u << regionId` and returns true whether or not the
bit was already set. An invalid call returns false and leaves all state
unchanged. It does not require and does not grant the National upgrade.
`Dex_UpgradeToNational` enables the existing National mode using its current
project mechanism. It is idempotent, returns true, and never grants an
extension. `Dex_HasNationalUpgrade` reflects that same mode gate.

Replace the Spec 1 temporary visibility policy with this final policy:

- `Dex_IsNationalEntryVisible(id)` returns true exactly when valid `id` appears
  in the active regional catalog or in at least one granted extension catalog.
  It describes National membership even before the upgrade, so extensions can
  be granted in any order.
- Before the National upgrade, player-facing Pokédex consumers use only the
  active regional facade. National mode, its list, and its progress display are
  unavailable.
- After the upgrade, National-mode consumers use the National facade.
- `Dex_GetNationalVisibleEntryCount`, first/next iteration, progress, and
  completion operate on this union, never on a sum of catalog counts.

Numerical National iteration yields ascending National ID. A National sort
order applies its canonical order to the same membership predicate. The
deduplication definition is canonical National ID, including any configured
regional-form entry keys above official 1025 through 1080. A catalog overlap
never makes duplicate UI rows or inflates any total.

The general Pokédex possession flag remains the menu gate. The old National
flag remains the National-mode gate, but its grants and queries route through
the APIs above. Neither a selected catalog nor a granted extension makes the
Pokédex menu available without the possession flag.

### Trade independence and received progress

Pokédex membership is presentation and progress-accounting policy, never trade
eligibility. Remove every active-regional, National-upgrade, extension, or
membership gate from `CheckValidityOfTradeMons`, `CanTradeSelectedMon`,
`GetUnionRoomTradeMessageId`, `CanRegisterMonForTradingBoard`, and
`CanSpinTradeMon`, and remove the legacy origin-progress gate from
`GetGameProgressForLinkTrade` for Wayfarer-to-Wayfarer trades. A player's or
partner's Wayfarer origin, active catalog, National state, extension mask, or
visible membership must not reject a valid trade offer, receipt, registration,
or spin-trade choice. None of these trade paths may call the regional or
National membership facade to decide whether a Pokémon can be traded.

Keep ordinary non-Pokédex restrictions, including `cannotBeTraded`, preserving
one usable party member, requested trade type or Egg pairing, and link protocol
compatibility. These are separate from catalog membership. The legacy
RSE/FRLG-style origin-progress and National-Dex checks are removed for
Wayfarer-to-Wayfarer trades; no equivalent facade check replaces them.

On a completed trade, `UpdatePokedexForReceivedMon` converts a valid received
non-Egg species to its canonical National ID, validates that ID, and records
seen and caught globally. It does not test regional or National membership.
An entry outside the current visible set does not receive a temporary local
row and does not affect current visible progress or completion. When a later
active-catalog selection or National-extension grant makes that ID visible, the
ordinary regional or National facade displays its existing progress once. An
invalid received species or canonical ID follows the existing safe invalid-mon
handling and must not access a Pokédex flag. A trade neither selects a catalog,
grants an extension, nor upgrades National mode.

### Critical-capture progress and real roll

Set `B_CRITICAL_CAPTURE = TRUE` in `include/config/battle.h`. Remove
`B_CRITICAL_CAPTURE_LOCAL_DEX` from Wayfarer's configuration and calculation:
the player's National-upgrade state, not a compile-time local-versus-National
switch, selects the progress set. Keep
`B_CRITICAL_CAPTURE_IF_OWNED = GEN_LATEST` unchanged and separate from the
real roll.

Implement `Dex_GetCriticalCaptureProgress` from Spec 1 as one consistent
snapshot of the current catalog state:

- Before `Dex_HasNationalUpgrade()` is true, `visibleCount` is the active
  regional catalog's distinct visible-entry count from
  `Dex_GetActiveRegionalEntryCount()`, and `caughtCount` is
  `Dex_GetRegionalVisibleProgress(FLAG_GET_CAUGHT)`. Those functions count the
  same distinct active-catalog entries with the global caught flag.
- After the upgrade, `visibleCount` is
  `Dex_GetNationalVisibleEntryCount()` and `caughtCount` is
  `Dex_GetNationalVisibleProgress(FLAG_GET_CAUGHT)`. Both values cover the
  active catalog plus every granted extension catalog, deduplicated by
  canonical National entry ID.

The function returns false and zeroes both outputs when the active region is
invalid, a catalog is malformed, the selected visible set is empty, or a count
cannot be produced safely. It does not count caught entries outside the
selected visible set. It does not mutate any Pokédex, extension, or battle
state.

`CriticalCapture` consumes only this snapshot. If the facade returns false or
the denominator is zero, it returns false before calculating a band or reading
`RNG_BALLTHROW_CRITICAL`. Otherwise it uses the unchanged strict threshold
comparison `caughtCount > floor(visibleCount * threshold / 650)`, in ascending
bands of 30, 150, 300, 450, and 600. The matching base-odds multipliers remain
50%, 100%, 150%, 200%, and 250%; the existing 255 cap, divide by 6, and
`RandomUniform(RNG_BALLTHROW_CRITICAL, 0, MAX_u8) < rollThreshold` remain
unchanged. A real critical capture retains its existing one-shake behavior and
does not guarantee a capture.

If the player gains an extension, a newly visible uncaught entry can increase
the denominator and reduce the band or real critical-capture odds. This is an
intentional consequence of the expanded visible catalog.

`B_CRITICAL_CAPTURE_IF_OWNED` remains solely in successful-capture finalization.
At `GEN_LATEST`, an already globally caught target receives the critical-capture
throw presentation after a successful capture even if the real roll did not
succeed. An unowned target does not receive that presentation unless the real
roll succeeds. A failed capture never receives the owned-species presentation.
The visual rule does not change the numerator, denominator, threshold, RNG
roll, or capture success.

### Script interface and Oak

Register these script specials and document their numeric region constants
beside the command declarations:

| Special | Input | Output in `VAR_RESULT` |
| --- | --- | --- |
| `DexScript_SetActiveRegion` | `VAR_0x8004`: region ID | `TRUE` only for a registered region |
| `DexScript_GetActiveRegion` | none | active region ID, or `DEX_REGION_NONE` for invalid stored state |
| `DexScript_GrantNationalExtension` | `VAR_0x8004`: region ID | `TRUE` for a valid first or repeated grant |
| `DexScript_HasNationalExtension` | `VAR_0x8004`: region ID | `TRUE` only when the valid region bit is set |
| `DexScript_UpgradeToNational` | none | `TRUE` |
| `DexScript_HasNationalUpgrade` | none | `TRUE` only when National mode is enabled |

No script special exposes the full `u32` extension mask. Scripts query one
region at a time, which avoids truncating the mask into a 16-bit script
variable. All mutating specials validate input before touching persistent
state. Invalid input produces `FALSE`, and query specials return the values
above without side effects.

Modify only the current Wayfarer Oak National-Dex scene at
`PalletTown_Lab_hns`. When its existing eligibility condition
selects the National-Dex scene, it retains the existing
`FLAG_SYS_NATIONAL_DEX` set and `EnableNationalPokedex` special. It then puts
`DEX_REGION_KANTO` in `VAR_0x8004` and calls
`DexScript_GrantNationalExtension`.

No atomic award helper, preflight transaction, rollback, or compensating state
is added. If the Kanto grant fails after the National upgrade succeeds, the
upgrade remains in place and National mode shows only the active Johto catalog.
That partial state is valid and harmless. Replaying an already completed scene
is idempotent and cannot add duplicate membership or change the active Johto
catalog.

Oak is the only gameplay integration in this spec. Do not wire origin
selection, Birch, travel, region entry, badges, League milestones, or other
professors.

### Failure handling

Catalog registry validation fails for a malformed record, duplicate local
mapping, duplicate region bit, unsupported extension mask bit, invalid
National ID, or a catalog entry count that differs from its data. Runtime query
failures return safe empty or false values as described above. Invalid script
arguments have no side effects. An active regional catalog or National union
with no entries presents an empty but closable UI, reports zero progress, and
is not complete. The same empty or invalid visible state gives real critical
capture no roll and no critical result.

### Validation

Implementation is accepted only when all of the following pass:

1. Registry tests compare every local entry with a checked-in, element-by-
   element baseline generated from the current source tables. They require the
   exact current order and counts: Kanto 188 from
   `sKantoToNationalOrder`, Johto 282 from `sJohtoToNationalOrder`, and Hoenn
   214 from `sHoennToNationalOrder`. The test rejects a zero or any entry after
   those valid ranges, including the implicit legacy Kanto and Johto terminal
   zeros. A checksum, a representative-entry check, or a regenerated list is
   insufficient. Local entries are contiguous from 1; every mapping is valid;
   no catalog has an internal duplicate; and each region ID produces one
   distinct representable `u32` extension bit. The build contains no
   `DexNationalExtensionId` or separate extension-to-region mapping.
2. A new save starts with Johto active and extension mask zero in
   `gSaveBlock2Ptr->pokedex`, without a new compile-time selection flag. A
   current-build save/load round trip restores both fields exactly. No migration
   code or old-save test fixture is added.
3. The active regional facade lists, numbers, counts, searches, and completes
   only the selected catalog. Switching a valid catalog does not alter global
   seen/caught flags, the National-upgrade state, or the extension mask.
4. Invalid active-catalog and local-entry values return the specified empty
   results, never access a Dex flag, and leave stored state unchanged.
5. Before the upgrade, the UI exposes only active Johto membership. Calling a
   valid extension grant before the upgrade records its bit but does not expose
   National mode or its extension entries.
6. Upgrade alone is valid and exposes exactly active Johto membership in
   National mode. It neither grants Kanto nor changes Johto's regional list.
7. For every subset and every grant-order prefix of valid Kanto, Johto, and
   Hoenn extension grants, assert after each operation that National
   membership, count, progress, and completion equal the deduplicated union of
   active catalog plus the grants received so far. Repeating any grant leaves
   the mask and results unchanged.
8. An overlapping fixture proves that an ID shared by active and extension
   catalogs, or by two extensions, has one list row and one seen/caught and
   completion contribution. A configured entry key above official 1025 through
   1080 follows the same rule.
9. A fixture that sees and catches an ID before its extension is granted shows
   the existing progress after grant without resetting or duplicating it. The
   extension changes the denominator only by newly visible distinct IDs.
10. Generic regional and National UI, search, area, trainer-card count,
    Summary display and shortcut, ratings, completion, start-menu, and credits
    consumers agree with the facade. This covers the rating script and Birch
    PC, diplomas, trainer-card stars, start/continue/save displays, television,
    Match Call, Hall of Fame, debug helpers, Easy Chat, the standard and HNS
    credits roll, and caught-mon battle checks. Credits iterate current visible
    membership through the facade and read only the matching global caught
    state. A source audit finds no remaining generic `REGIONAL_DEX_COUNT`,
    build-identity regional choice, duplicate union loop, or retired Obtainable
    conversion in any Pokédex consumer. Trade code is deliberately excluded
    from this facade audit and must satisfy tests 21 and 22 instead.
11. Switching the active catalog among Kanto, Johto, and Hoenn updates the
    generic regional-mode label and list without selecting a name through
    `IS_HNS` or `IS_FRLG`. Existing visual assets may remain unchanged.
12. Script tests cover every listed special, valid first and repeated grants,
    every invalid region ID, grant-before-upgrade,
    upgrade-before-grant, and query results. Invalid calls preserve all
    persistent state; no script mask result is silently truncated to 16 bits.
13. The Oak eligible-scene journey upgrades National mode and grants Kanto,
    yielding the Johto plus Kanto union. Its existing ineligible paths do not
    alter catalog, extension, or upgrade state; replay is idempotent. A forced
    failed Kanto-grant fixture proves that the National upgrade remains set and
    the extension mask remains unchanged, leaving the valid Johto-only
    National list.
14. The final Wayfarer configuration has `B_CRITICAL_CAPTURE = TRUE`, has no
    live `B_CRITICAL_CAPTURE_LOCAL_DEX` calculation path, and retains
    `B_CRITICAL_CAPTURE_IF_OWNED = GEN_LATEST` as a presentation-only rule.
15. Before the National upgrade, a critical-capture fixture counts only
    distinct caught entries in active Johto and uses Johto's distinct visible
    count as its denominator. A caught Kanto entry outside active Johto does
    not affect either value.
16. After the upgrade, fixtures with Kanto, Johto, and Hoenn grants in every
    subset and order use the deduplicated visible National union for both
    critical-capture values. Overlapping National IDs contribute once to the
    numerator and once to the denominator.
17. A fixture with unchanged caught entries and a newly granted catalog proves
    denominator growth can move the strict threshold band down and lower the
    real critical-capture roll threshold. This is accepted behavior.
18. Empty visible, invalid active-region, malformed-catalog, and zero-
    denominator fixtures return no real critical capture, perform no critical
    RNG read, and do not mutate battle or Pokédex state.
19. Deterministic pure-helper tests cover equality and one-above boundaries for
    all five `threshold / 650` bands, every corresponding multiplier, the
    255 cap, divide-by-6 result, and the Catching Charm's existing multiplier.
    Battle integration pins `RNG_BALLTHROW_CRITICAL` to 0 and `MAX_u8` to prove
    the positive and negative real-roll outcomes, including one-shake behavior.
20. Retained and expanded owned-species visual tests prove that an already
    caught target with `B_CRITICAL_CAPTURE_IF_OWNED = GEN_LATEST` uses the
    critical throw presentation on successful capture even when the real roll
    is disabled or cannot occur; an owned failed capture remains normal; and
    an unowned successful capture remains normal unless the real roll succeeds.
21. Parameterized trade tests cover every ordered pair of distinct Kanto,
    Johto, and Hoenn Wayfarer origins at local link setup and normal local
    link-trade offer and receipt, every National-upgrade state, every
    extension-mask state, and a valid species absent from the active catalog
    and all granted extensions. The link setup reports neither player nor
    partner as origin-progress-ineligible. Union Room offer and receipt,
    trading-board registration, and spin trade also accept that species without
    a membership, origin, National-mode, or extension rejection. Separate
    fixtures retain the existing `cannotBeTraded`, last-usable-party-member,
    requested-type or Egg-pairing, and link-protocol rejection behavior.
22. A completed-trade fixture receives a valid canonical species absent from
    active Johto and all extensions before the National upgrade. It records
    global seen and caught flags, contributes no hidden row or visible progress,
    then appears exactly once with both flags when its catalog becomes visible
    by a later active-catalog selection or extension grant. Repeat the fixture
    after the National upgrade and with an overlapping catalog ID to prove that
    receipt progress is global and later visibility is deduplicated. Invalid
    received species and converted IDs do not access Pokédex flags.

## References

- [Regional and expanding National Pokédex](../prds/regional-national-pokedex.md)
- [Canonical National Pokédex foundation](canonical-national-pokedex.md)
- `game/include/constants/pokedex.h` currently provides Kanto, Johto, Hoenn,
  and separate HNS Obtainable order definitions, plus a build-selected
  `REGIONAL_DEX_COUNT`.
- `game/src/pokemon.c:150-1437` contains the current regional and Obtainable
  conversion tables; `game/src/pokemon.c:7338-7519` selects them through
  compile-time build identity today.
- The exact initial catalog sources are `sKantoToNationalOrder`
  (`game/src/pokemon.c:150-409`), `sHoennToNationalOrder`
  (`game/src/pokemon.c:413-653`), and `sJohtoToNationalOrder`
  (`game/src/pokemon.c:655-939`).
- `game/src/pokedex.c:4513-4699` currently keeps separate generic and
  region-specific progress/completion paths.
- `game/src/battle_script_commands.c:10938-10977` currently calculates real
  critical captures from a compile-time regional or Obtainable denominator,
  while `:10573-10583` applies the separate owned-species presentation rule.
- `game/test/battle/capture.c:141-210` already exercises the owned-species
  successful and failed-capture presentation behavior.
- `game/src/trade.c:1560-1587,2384-2443,2487-2546,2549-2570,2572-2634`
  currently contains the membership-based link, Union Room, trading-board, and
  spin-trade gates that this spec removes.
- `game/src/trade.c:2446-2485` and `game/src/link.c:815-826` currently apply
  the legacy origin-progress result during local trade setup; Wayfarer origins
  must not produce that rejection.
- `game/src/trade.c:3055-3071` already records received non-Egg Pokémon as
  global canonical seen and caught flags; the implementation preserves that
  behavior without a membership check.
- `game/data/maps/PalletTown_Lab_hns/scripts.inc:5-23` contains the current
  Wayfarer Oak National-Dex helper and calls `EnableNationalPokedex`; this is
  the only approved gameplay wiring point.
