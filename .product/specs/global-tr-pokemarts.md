# Global TR Poké Marts

PRD: [Poké Marts across the three regions](../prds/global-tr-pokemarts.md)
Implemented: Outdated

The League progression example now assumes +8 per first-time clear. That
reward change is pending implementation; the current circuit uses the old rewards.

## Scope

Implement shared Trainer Rating essentials and permanent local stock for the
Wayfarer counters enumerated below. Kanto means the active HNS Kanto maps;
Johto uses HNS maps and Hoenn uses imported Emerald maps. This specification
defines the catalog, bindings, script changes, runtime contract and acceptance.
It does not authorize changes to Pokémon species, teams, movesets or learnsets.

Source baseline: `f7a2b95b7bde184141cf5b91215dc4e7ab9068a6`. The core production
implementation is merged and enabled by default. Emulator validation and release
acceptance remain pending. Reconcile changed map bindings and authored shop lists
with the retention rules below before extending the implementation.

## Behavior

### Essentials and global progression

Read `GetTrainerRating()` once when a converted counter opens. Use the greatest
threshold less than or equal to that value. The getter's existing saved high-water
behavior and League calculation remain authoritative. Never count badge flags,
infer TR from the current region, or require a particular League clear.

| Tier | Minimum TR | Newly included item constants |
| --- | --- | --- |
| 0 | 0 | `ITEM_POKE_BALL`, `ITEM_POTION`, `ITEM_ANTIDOTE`, `ITEM_PARALYZE_HEAL`, `ITEM_AWAKENING`, `ITEM_BURN_HEAL`, `ITEM_ICE_HEAL`, `ITEM_REPEL`, `ITEM_ESCAPE_ROPE` |
| 1 | 4 | `ITEM_GREAT_BALL`, `ITEM_SUPER_POTION` |
| 2 | 16 | `ITEM_SUPER_REPEL` |
| 3 | 30 | `ITEM_ULTRA_BALL`, `ITEM_HYPER_POTION`, `ITEM_REVIVE` |
| 4 | 40 | `ITEM_FULL_HEAL`, `ITEM_MAX_REPEL` |
| 5 | 55 | `ITEM_MAX_POTION`, `ITEM_FULL_RESTORE` |

The cumulative ordinary counts are 9, 11, 12, 15, 17 and 19. TR 55 through 80
uses tier 5. Lower-tier entries never disappear. A resupply location means
one full-service counter, except Lilycove 2F, where it means the union of the
two co-located counters. At equal TR, each location offers the complete same
common catalog; the two Lilycove profiles individually offer their defined
subsets. Local or retained stock
must never reintroduce a common item before its threshold.

Use this display order, skipping locked entries: Poké Ball, Great Ball, Ultra
Ball; Potion, Super Potion, Hyper Potion, Max Potion, Full Restore; Antidote,
Paralyze Heal, Awakening, Burn Heal, Ice Heal, Full Heal, Revive; Repel, Super
Repel, Max Repel, Escape Rope. Append challenge supplements, signature goods
in table order, then retained goods in their original list order. De-duplicate
by item ID, preserving the first occurrence. Finish with `ITEM_NONE`.

### Pokémon Center challenge

When `gSaveBlock3Ptr->challengeSettings.tx_Challenges_PkmnCenter` is nonzero,
append this cumulative PP catalog only to profiles with `supportsPpRecovery`
set. Set it true for every converted full-service profile and Lilycove's right
profile, and false for Lilycove's left profile. Preserved specialists do not
consume this supplement. Match the existing nonzero-setting interpretation.

| Minimum TR | Additional challenge stock |
| --- | --- |
| 0 | `ITEM_ETHER` |
| 16 | `ITEM_ELIXIR` |
| 30 | `ITEM_MAX_ETHER` |
| 55 | `ITEM_MAX_ELIXIR` |

These explicit TR thresholds replace the old badge thresholds for converted
counters. Normal play does not receive the supplement. Existing challenge
item-use restrictions, Expensive pricing and all specialist stock stay intact.

### Local stock authority and retention

The tables below are the complete v1 conversion registry. A map name means
`game/data/maps/<name>/map.json`. Profile IDs are new symbolic constants, one
per table row, named from that map and counter. All listed signatures are
available at TR 0 and at every later tier, independent of story and time.

For each existing literal ordinary-mart inventory, retain the union of its
authored nonessential goods across its inventory branches. Remove all 19 common
IDs from those literals before merging, so they cannot bypass TR. Keep X items,
vitamins, stones, mail and Fluffy Tails that were explicitly sold there. Retained
goods are unconditional in a converted counter; story conditions continue only
at the separately preserved specialist services. For the old shared NULL
catalog, retain no extra goods: its Fluffy Tail becomes a local item, not a
universal essential. All other former NULL-catalog items are represented by the
new essentials or the challenge supplement.

During implementation, record the resulting retained IDs per profile in a
checked-in catalog manifest, with the source list labels and map/clerk binding.
The extraction rule above is normative, not permission to drop inherited goods
to meet the two-to-four signature target. That target applies to signatures;
inherited extras and facility vitamin selections may make the total longer.
Every new signature ID must exist in the compiled Wayfarer item table and be
buyable under the existing item rules.

### Johto town profiles

| Map / counter | Existing binding | Permanent signature IDs |
| --- | --- | --- |
| `CherrygroveCity_Mart_hns` | Cherrygrove clerk | `ITEM_HEAL_BALL`, `ITEM_POKE_DOLL` |
| `VioletCity_Mart_hns` | Violet clerk | `ITEM_NEST_BALL`, `ITEM_X_ACCURACY` |
| `AzaleaTown_Mart_hns` | Cherrygrove clerk | `ITEM_NET_BALL`, `ITEM_NEST_BALL`, `ITEM_WOOD_MAIL` |
| `GoldenrodCity_DepartmentStore_2F_hns` | `GoldenrodDeptStore2_EventScript_Clerk` | `ITEM_FIRE_STONE`, `ITEM_WATER_STONE`, `ITEM_THUNDER_STONE`, `ITEM_LUXURY_BALL` |
| `EcruteakCity_Mart_hns` | Cherrygrove clerk | `ITEM_DUSK_BALL`, `ITEM_POKE_DOLL`, `ITEM_RETRO_MAIL` |
| `OlivineCity_Mart_hns` | Cherrygrove clerk | `ITEM_NET_BALL`, `ITEM_HEAL_BALL`, `ITEM_HARBOR_MAIL` |
| `BlackthornCity_Mart_hns` | Cherrygrove clerk | `ITEM_DUSK_BALL`, `ITEM_TIMER_BALL`, `ITEM_GUARD_SPEC` |
| `Mahoganytown_hns` / exterior merchant | `MahoganyTown_EventScript_Merchant` | `ITEM_NET_BALL`, `ITEM_DUSK_BALL` |

The names "Cherrygrove clerk" and "Violet clerk" mean
`Cherrygrove_Pokemart_EventScript_Clerk` and
`VioletCity_Mart_EventScript_Clerk`. They currently serve multiple towns;
do not give all their users one profile. Existing ordinary HNS mart employees
stand at (2,3); Goldenrod's counter employee stands at (14,11).

Mahogany is a new essentials service at an existing NPC, not replacement stock
for the interior evolution shop. Cianwood's pharmacist keeps the Secret Potion
service; no new essentials service is added there. New Bark has no mart to
convert. These are explicit coverage boundaries, not missing profiles.

### Kanto town profiles

| Map / counter | Existing binding | Permanent signature IDs |
| --- | --- | --- |
| `ViridianCity_Mart_hns` | Cherrygrove clerk | `ITEM_NEST_BALL`, `ITEM_POKE_DOLL`, `ITEM_ORANGE_MAIL` |
| `PewterCity_Mart_hns` | Cherrygrove clerk | `ITEM_DUSK_BALL`, `ITEM_X_DEFENSE` |
| `CeruleanCity_Mart_hns` | Cherrygrove clerk | `ITEM_NET_BALL`, `ITEM_HEAL_BALL`, `ITEM_X_SPEED` |
| `VermilionCity_Mart_hns` | Cherrygrove clerk | `ITEM_NET_BALL`, `ITEM_DIVE_BALL`, `ITEM_HARBOR_MAIL` |
| `LavenderTown_Mart_hns` | Cherrygrove clerk | `ITEM_DUSK_BALL`, `ITEM_HEAL_BALL`, `ITEM_SHADOW_MAIL` |
| `CeladonCity_DepartmentStore_2F_hns` | Violet clerk at (12,11) | `ITEM_LUXURY_BALL`, `ITEM_POKE_DOLL`, `ITEM_RETRO_MAIL` |
| `SaffronCity_Mart_hns` | Cherrygrove clerk | `ITEM_X_SP_ATK`, `ITEM_X_SP_DEF`, `ITEM_GUARD_SPEC` |
| `FuchsiaCity_Mart_hns` | Cherrygrove clerk | `ITEM_NET_BALL`, `ITEM_NEST_BALL`, `ITEM_FLUFFY_TAIL` |

The ordinary Kanto employees stand at (2,3). Preserve Mt. Moon's Moon Stones,
drinks and souvenirs, and all Celadon specialist floors. Pallet and active
HNS Cinnabar have no mart to convert. Do not add FRLG or Sevii marts to claim
Kanto coverage. Existing alternatives to these signature items remain available.

### Hoenn town profiles

| Map / counter | Permanent signature IDs |
| --- | --- |
| `OldaleTown_Mart` | `ITEM_HEAL_BALL`, `ITEM_NEST_BALL` |
| `PetalburgCity_Mart` | `ITEM_NEST_BALL`, `ITEM_X_DEFENSE`, `ITEM_ORANGE_MAIL` |
| `RustboroCity_Mart` | `ITEM_TIMER_BALL`, `ITEM_REPEAT_BALL` |
| `SlateportCity_Mart` | `ITEM_NET_BALL`, `ITEM_DIVE_BALL`, `ITEM_HARBOR_MAIL`, `ITEM_LUXURY_BALL` |
| `MauvilleCity_Mart` | `ITEM_X_SPEED`, `ITEM_X_SP_ATK`, `ITEM_X_ACCURACY`, `ITEM_MECH_MAIL` |
| `VerdanturfTown_Mart` | `ITEM_NEST_BALL`, `ITEM_FLUFFY_TAIL` |
| `FallarborTown_Mart` | `ITEM_DUSK_BALL`, `ITEM_DIRE_HIT`, `ITEM_X_DEFENSE` |
| `LavaridgeTown_Mart` | `ITEM_HEAL_BALL`, `ITEM_GUARD_SPEC`, `ITEM_X_SP_DEF` |
| `FortreeCity_Mart` | `ITEM_NEST_BALL`, `ITEM_NET_BALL`, `ITEM_WOOD_MAIL`, `ITEM_X_SPEED` |
| `MossdeepCity_Mart` | `ITEM_NET_BALL`, `ITEM_DIVE_BALL` |
| `SootopolisCity_Mart` | `ITEM_DIVE_BALL`, `ITEM_DUSK_BALL`, `ITEM_SHADOW_MAIL` |
| `LilycoveCity_DepartmentStore_2F` / left | `ITEM_LUXURY_BALL`, `ITEM_FLUFFY_TAIL` |
| `LilycoveCity_DepartmentStore_2F` / right | `ITEM_WAVE_MAIL`, `ITEM_MECH_MAIL` |

The eleven ordinary mart clerks use `<Map>_EventScript_Clerk`, at (1,3), with
existing `LOCALID_<TOWN>_MART_CLERK` bindings. Lilycove left/right use
`LilycoveCity_DepartmentStore_2F_EventScript_ClerkLeft` at (7,6) and
`...ClerkRight` at (10,6). Preserve their division: left sells the unlocked
common balls, Escape Rope and status cures; right sells unlocked HP medicine,
Revive and repels, plus the challenge supplement. Their union equals a
full-service common catalog. Keep Full Restore on the right and Full Heal on
the left. Each clerk keeps only the retained extras from its own source list.

Littleroot, Dewford and Pacifidlog have no ordinary mart to convert. The
`LilycoveCity_UnusedMart` and prototype maps are excluded. The outdoor Slateport
market, herb shop and other specialist vendors remain separate services.

### League and facility cash counters

These exceptions to the town signature size retain their complete authored
nonessential stock, but receive the same tiered essentials. They do not gain
new signature goods.

| Map / counter | Existing binding and treatment |
| --- | --- |
| `IndigoPlateau_PokemonCenter_hns` | Employee at (10,10), `BattleFrontier_Mart_EventScript_Clerk`; full essentials plus retained vitamins |
| `EverGrandeCity_PokemonLeague_1F` | `EverGrandeCity_PokemonLeague_1F_EventScript_Clerk`; full essentials |
| `BattleFrontier_Mart_hns` | `BattleFrontier_Mart_EventScript_Clerk_hns`; full essentials plus retained vitamins |
| `BattleFrontier_Mart` | `BattleFrontier_Mart_EventScript_Clerk`; full essentials plus retained vitamins |
| `TrainerHill_Entrance_hns` / clerk 2 at (12,9) | `TrainerHill_Entrance_hns_EventScript_Clerk2`; full essentials |
| `TrainerHill_Entrance` / clerk at (14,9) | `TrainerHill_Entrance_EventScript_Clerk`; full essentials plus union of authored X/battle items |

HNS Trainer Hill's first clerk at (14,9) remains a battle-item specialist.
BP exchanges, décor and challenge-admission NPCs are excluded. Venue access
and facility rules stay authored; once a listed clerk is accessible, its stock
has no additional game-clear gate. Any listed facility map excluded by the
Wayfarer build receives an explicit inactive classification in the manifest,
not a fabricated reachable shop or an unreferenced runtime profile.

### Baseline retained stock

The baseline retained sets below make the union rule concrete. Items already
present in a signature appear only once after de-duplication. All converted
profiles not named here have an empty retained set. Each row's source is the
literal ordinary `pokemart` list or lists in that map's `scripts.inc`, except
Indigo, which explicitly reuses the Frontier source.

| Profile | Retained item IDs in source-union order |
| --- | --- |
| Goldenrod 2F | `ITEM_FIRE_STONE`, `ITEM_WATER_STONE`, `ITEM_THUNDER_STONE` |
| Petalburg | `ITEM_X_SPEED`, `ITEM_X_ATTACK`, `ITEM_X_DEFENSE`, `ITEM_ORANGE_MAIL` |
| Rustboro | `ITEM_X_SPEED`, `ITEM_X_ATTACK`, `ITEM_X_DEFENSE`, `ITEM_TIMER_BALL`, `ITEM_REPEAT_BALL` |
| Slateport mart | `ITEM_HARBOR_MAIL` |
| Mauville | `ITEM_X_SPEED`, `ITEM_X_ATTACK`, `ITEM_X_DEFENSE`, `ITEM_GUARD_SPEC`, `ITEM_DIRE_HIT`, `ITEM_X_ACCURACY` |
| Verdanturf | `ITEM_NEST_BALL`, `ITEM_X_SP_ATK`, `ITEM_FLUFFY_TAIL` |
| Fallarbor | `ITEM_X_SP_ATK`, `ITEM_X_SPEED`, `ITEM_X_ATTACK`, `ITEM_X_DEFENSE`, `ITEM_DIRE_HIT`, `ITEM_GUARD_SPEC` |
| Lavaridge mart | `ITEM_X_SPEED` |
| Fortree | `ITEM_WOOD_MAIL` |
| Mossdeep | `ITEM_NET_BALL`, `ITEM_DIVE_BALL`, `ITEM_X_ATTACK`, `ITEM_X_DEFENSE` |
| Sootopolis | `ITEM_X_ATTACK`, `ITEM_X_DEFENSE`, `ITEM_SHADOW_MAIL` |
| Lilycove 2F left | `ITEM_FLUFFY_TAIL` |
| Lilycove 2F right | `ITEM_WAVE_MAIL`, `ITEM_MECH_MAIL` |
| Indigo, both Frontier marts | `ITEM_PROTEIN`, `ITEM_CALCIUM`, `ITEM_IRON`, `ITEM_ZINC`, `ITEM_CARBOS`, `ITEM_HP_UP` |
| Hoenn Trainer Hill | `ITEM_X_SPEED`, `ITEM_X_SP_ATK`, `ITEM_X_ATTACK`, `ITEM_X_DEFENSE`, `ITEM_DIRE_HIT`, `ITEM_GUARD_SPEC`, `ITEM_X_ACCURACY` |

### Clerk interactions

Keep every existing map-event `script` binding. This feature must not edit map
JSON or introduce wrapper labels. Instead, put one guarded Wayfarer branch in
each existing converted clerk entry point and leave its original `pokemart`
path as the `#else` or sentinel fallback. Every new or replaced script branch,
including inventory-gate bypasses, dialogue changes, profile lookup, opener
call and post-shop continuation, must use exactly
`#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED`. Do not use a bare
`#if IS_WAYFARER` branch for mart behavior. With that condition false, the
unmodified legacy interaction, labels and map bindings remain in use.

Use one deterministic dispatch architecture. Direct-profile scripts set
`VAR_0x8004` to their constant `MART_PROFILE_*` ID, call
`special WayfarerOpenMartProfile`, then immediately `waitstate`, before their
existing "Please come again" continuation. This covers the eleven ordinary
unique Hoenn labels, `OldaleTown_Mart_EventScript_Clerk`,
`PetalburgCity_Mart_EventScript_Clerk`, `RustboroCity_Mart_EventScript_Clerk`,
`SlateportCity_Mart_EventScript_Clerk`, `MauvilleCity_Mart_EventScript_Clerk`,
`VerdanturfTown_Mart_EventScript_Clerk`, `FallarborTown_Mart_EventScript_Clerk`,
`LavaridgeTown_Mart_EventScript_Clerk`, `FortreeCity_Mart_EventScript_Clerk`,
`MossdeepCity_Mart_EventScript_Clerk` and `SootopolisCity_Mart_EventScript_Clerk`,
plus unique department-store, League, Trainer Hill, Lilycove-left and
Lilycove-right entries. They must not use a current-map lookup.

The only shared-label path is a new explicit
`WayfarerLookupMartProfileForSharedClerk` special. The script puts one of
`MART_CLERK_FAMILY_CHERRYGROVE`, `MART_CLERK_FAMILY_VIOLET` or
`MART_CLERK_FAMILY_FRONTIER` in `VAR_0x8005`, then uses
`specialvar VAR_0x8004, WayfarerLookupMartProfileForSharedClerk`. The special
looks up the pair `(family, compiled current map)`, with the current map taken
from `gSaveBlock1Ptr->location.mapGroup/mapNum` and table keys expressed as the
matching compiled `MAP_GROUP(MAP_...)` and `MAP_NUM(MAP_...)` values. It returns
the matching profile in `VAR_0x8004` or the reserved
`MART_PROFILE_NONE` sentinel. The script branches to its original `pokemart`
path on that sentinel; otherwise it invokes the normal opener and `waitstate`.

`Cherrygrove_Pokemart_EventScript_Clerk` uses the Cherrygrove family for the
listed Cherrygrove, Azalea, Ecruteak, Olivine, Blackthorn, Viridian, Pewter,
Cerulean, Vermilion, Lavender, Saffron and Fuchsia maps.
`VioletCity_Mart_EventScript_Clerk` uses the Violet family for Violet and
Celadon 2F. `BattleFrontier_Mart_EventScript_Clerk` and
`BattleFrontier_Mart_EventScript_Clerk_hns` use the Frontier family for their
respective Frontier maps and Indigo Plateau. No profile may be inferred from a
shared label alone. A listed converted map that returns `MART_PROFILE_NONE` is
a release-blocking binding-audit failure; a map outside the conversion registry
that reaches one of these labels deliberately keeps its legacy branch. This
single existing-entry dispatcher path replaces both per-map wrapper and
conditional-JSON alternatives.

At Cherrygrove, remove the `VAR_NEWBARK_TOWN_STATE >= 5` inventory restriction.
At Oldale, remove the `FLAG_ADVENTURE_STARTED` stock restriction. At Petalburg,
remove `FLAG_PETALBURG_MART_EXPANDED_ITEMS` from stock selection. At Rustboro,
remove `FLAG_MET_DEVON_EMPLOYEE` from stock selection. Rustboro's Timer/Repeat
Balls are now available on arrival. At Hoenn Trainer Hill, remove the
`FLAG_SYS_GAME_CLEAR` inventory branch. Preserve every unrelated writer or
reader of these flags and vars. Do not change story state to make a shop work.
Each removal exists only inside its
`#if IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED` mart branch; the false branch
retains the original gate and stock selection.

Update Cherrygrove and Oldale NPC text/branches that describe sold-out Poké Balls.
Update Mossdeep's "only made in MOSSDEEP" sales claim to acknowledge its
specialization without implying exclusive distribution. Review other listed
clerks' stock-specific dialogue against their actual catalogs and thresholds.

Mahogany uses the exterior merchant at (30,11),
`LOCALID_MAHOGANY_MERCHANT`, with a zero visibility flag. On direct interaction,
offer `Supplies`, `Rage Candy Bar`, `Cancel`. Supplies opens the Mahogany profile
at every story state, including 17. Rage Candy Bar follows the existing manual
sale and state-17 sold-out behavior; Cancel releases the player without spending
money or moving them. Keep the interior granny's state-14 evolution shop, Rocket
scenes, movement/collision and coordinate-trigger behavior intact. Do not expose
the new menu through the automatic merchant trigger. After closing Supplies,
return to the merchant's choice menu without replaying story movement. Confirm
direct access from the town side at the story states in acceptance testing.

## Runtime contract

Introduce a profile-based normal mart opener. The suggested files are
`game/include/wayfarer_marts.h`, `game/include/constants/wayfarer_marts.h`,
`game/src/wayfarer_marts.c` and `game/src/data/wayfarer_marts.h`; these are new
implementation files, not existing APIs. Keep membership separate from price,
currency and item-effect policy.

Add `game/include/config/wayfarer_marts.h` as the sole mart feature-switch
header. It must provide the overridable default
`#ifndef WAYFARER_TR_MARTS_ENABLED` / `#define WAYFARER_TR_MARTS_ENABLED 1` /
`#endif`; the mandatory `IS_WAYFARER &&` conjunction makes it inactive in
non-Wayfarer builds. Include this header in every production C source and C
test that uses mart profiles, the opener, lookup special or switch, before its
first use. Add `#include "config/wayfarer_marts.h"` to the top configuration
includes in `game/data/event_scripts.s`, before any imported Hoenn block, so
every imported mart script sees the same macro. Do not duplicate this setting
in JSON, script constants or a test-local default.

1. Each direct-profile clerk branch sets `VAR_0x8004` to a stable
   `MART_PROFILE_*` ID and invokes
   `special WayfarerOpenMartProfile`, followed immediately by `waitstate`.
   The shared-clerk lookup described above supplies that same `VAR_0x8004`
   profile before the opener. Reserve `MART_PROFILE_NONE` outside the valid
   profile range. Register both
   `WayfarerOpenMartProfile` and `WayfarerLookupMartProfileForSharedClerk` in
   `game/data/specials.inc` without renumbering existing special IDs. Include
   the new constants through the normal script build path.
2. The opener snapshots profile, TR and challenge setting, resolves the ordered
   inventory, and calls `CreatePokemartMenu(nonNullItems)`. Use `MART_TYPE_NORMAL`
   through that function; it supplies Buy/Sell/Quit and resumes the script on
   final exit. Do not modify the existing `pokemart` bytecode format.
3. The resolver assembles a maximum of 63 unique item IDs plus `ITEM_NONE` in
   a dedicated static EWRAM `u16[64]` buffer (128 bytes). Use explicit source
   counts and bounded append operations. Never hand the UI a stack array.
4. The buffer belongs to the currently open mart and remains unchanged through
   Buy, Sell, cancel and return-to-shop transitions. Rebuild only after the prior
   shop has fully closed; the locked script flow prevents concurrent opens.
   Snapshotting means a catalog cannot change beneath a selected item.
5. Reject invalid source IDs and capacity overflow in host validation. At runtime,
   invalid profiles/data fall back to a valid common catalog for the captured
   tier and challenge setting, with no signatures, rather than exposing a partial
   list or reading past a terminator. Include a debug diagnostic. The fallback
   is defensive only: an unknown binding or profile is a release-blocking audit
   failure. When possible, resolve and validate before modifying the active buffer.
6. Keep legacy `SetShopItemsForSale(NULL)` behavior for standalone or excluded
   callers. Every listed Wayfarer counter must use an explicit profile. Do not
   select a profile by shared script label or intercept every normal mart on a
   map; multi-counter maps and shared specialty labels make either ambiguous.

The existing-entry dispatch described in Clerk interactions is mandatory. Its
lookup table must be keyed by the clerk family and compiled map, not by label,
source-map name or JSON. It covers Celadon's Violet family profile and Indigo's
Frontier family profile. An unknown `(family, compiled map)` must return
`MART_PROFILE_NONE` and take the original script branch; it must never inherit
Cherrygrove, Violet, Frontier or a generic common profile. The audit must fail
for an unknown map that is listed as converted, while an explicitly
out-of-scope shared-label user remains legacy. No conditional map-event binding
or per-map wrapper is permitted.

The resolver stores no new save data. TR is read through its public API and
shop state is transient. No prerelease save migration is required. Gate every
mart-specific script branch behind `IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED`.
With the switch false, all converted interactions, including Mahogany and
dialogue, take their original branches. Preserve non-Wayfarer code paths and
numeric script/map compatibility.

### Prices, purchases and selling

Profiles contain only item membership. Reuse the existing `GetItemPrice`,
Expensive challenge multiplier, PokéNews sale calculation, quantity limits,
bag-full checks, purchase history, Premier Ball bonus and Sell behavior. A
PokéNews sale is an existing engine exception to the common base price, not
a newly introduced regional discount. Do not change zero-price or important-item
rules, item effects, sale factors, money caps or currency. New signatures in
this spec are ordinary buyable items; zero-price/important entries are rejected
by the profile audit unless an explicitly retained authored service handles them.

## Implementation and validation

### Catalog and binding audit

Generate a checked-in manifest from the authored profile definitions, including
map name, compiled map ID, NPC/script binding, profile ID, category mask,
`supportsPpRecovery`,
signature IDs, retained source lists and IDs, and output IDs for all six tiers
and both challenge states. Retained stock must be reviewed against the baseline
source lists; generated output alone does not prove the extraction was correct.

Walk the active Wayfarer map NPC bindings and reachable script calls, not merely
files containing `pokemart`. Classify each shop entry as converted, preserved
specialist, or excluded/inactive with a reason. Check every row in this spec and
every discovered active mart; reject missing profiles, unclassified counters,
duplicate bindings and residual legacy stock paths at converted counters.
Include shared-script users and both Lilycove counters. Outlying-region NULL
callers must be classified and retain legacy behavior, not be reprofiled.

For each profile and every integer TR 0 through 80, test exact expected item
membership, no duplicates, correct order/terminator, capacity and monotonic
common stock. Assert that each of the 29 town-counter signature sets has two
to four IDs and that those sets are pairwise distinct; the six facility
profiles are explicit exceptions with no new signatures. The selected catalog's
largest list is Mauville in challenge mode at TR 55 or above: 31 unique IDs,
including eight distinct signature/retained goods. Treat future capacity
growth as a checked data change. Cover threshold minus one, threshold and threshold plus one;
normal/challenge switching; equal TR with Johto-only, Kanto-only, Hoenn-only
and mixed progress; 24 badges without League clears (TR 56) and eight badges
plus the first League clear (TR 48). The TR 56 case has all essentials; the
TR 48 case has the TR 45 tier and excludes Max Potion and Full Restore, which
unlock at TR 55. Mart thresholds are unchanged.
Use the real global TR path in integration tests, not only a stubbed tier input.

Verify the 19 common items cannot appear early through signatures or retained
lists. Verify the challenge's four PP items appear at their exact thresholds
only in the supplement, and do not remove authored specialist PP supplies.
Check Lilycove's union equals a full common catalog and its split is disjoint
for common items. Check all existing specialist inventory and availability
snapshots remain unchanged, including mints, evolution items, Kurt and BP.

### Script and runtime acceptance

- Exercise each converted clerk binding in a deterministic script harness;
  verify the profile received by the opener and the required `waitstate`/resume
  behavior. Test shared-script dispatch and unknown-map handling separately.
- Test Oldale, Petalburg, Rustboro and Cherrygrove before/after their former
  stock gates at fixed TR. Stock and successful purchase must match; story
  flags/vars must be unchanged by opening, buying and closing the mart.
- Test Mahogany states 1, 5, 6, 13, 14, 16 and 17, both challenge settings,
  Supplies purchase/cancel, Rage Candy sale/sold-out and direct NPC access.
  Confirm the automatic route trigger and interior specialist still behave
  as before. Opening Supplies must not fire story, move the player or sell candy.
- In a Wayfarer emulator build, visit at least one town mart in each region at
  low and high TR; both Lilycove counters; Goldenrod and Celadon essentials;
  Mahogany; and one League/Frontier counter. Open Buy, scroll to the final local
  item, buy multiple quantities, test insufficient money and full bag, enter
  Sell, return to Buy, exit and immediately visit a differently profiled mart.
  Check no stale list, duplicate, crash, frozen script or incorrect money change.
- Validate normal/challenge PP stock and Expensive/PokéNews price handling with
  the actual purchase path. Verify excluded specialist item lists, costs and
  currency after visiting an ordinary mart, catching shared-buffer/state leaks.
- Build Wayfarer with the feature on and off, and standalone HNS and Emerald
  serially because map generation shares outputs. Check FRLG compilation/script
  compatibility as well. Run the existing Hoenn content tests/audit
  (`make -C game wayfarer-hoenn-content-test` and
  `make -C game wayfarer-hoenn-content-audit`) and the relevant script tests.
  Update only intentionally affected audit manifests. Record the actual commands,
  results and ROM/RAM size change in the implementation PR.

The core profile data, host catalog checks, script bindings and production build
are implemented. Emulator and release-acceptance checks remain required,
including the no-tile-edit boundary. Record the actual commands, results and
ROM/RAM size change in the implementation PR.

## References

- [Existing shop tiers, list storage and normal opener](../../game/src/shop.c)
- [Existing mart bytecode command](../../game/src/scrcmd.c)
- [Trainer Rating getter and level-cap anchors](../../game/src/trainer_rating.c)
- [Global badge and League TR calculation](../../game/src/league_circuit.c)
- [Current map groups](../../game/data/maps/map_groups.json)
- [Mahogany merchant and story scripts](../../game/data/maps/Mahoganytown_hns/scripts.inc)
- [Hoenn content integration](wayfarer-hoenn-content-port.md)
- [Trainer Rating and party progression](trainer-rating-party-progression.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
