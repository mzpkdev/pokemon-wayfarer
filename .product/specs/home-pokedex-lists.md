# Home Pokédex lists

PRD: [Home Pokédex lists and the National upgrade](../prds/home-pokedex-lists.md)
Implemented: No

## Scope

This specification revises
[Regional Pokédex catalogs and National extensions](regional-pokedex-catalogs.md)
for the Wayfarer build. It replaces the three regional catalogs with two home
lists, makes National mode show every enabled species, retires the per-region
extension grants, and adds the professors' first-meeting upgrade. The canonical
National foundation, global seen and caught records, trade independence, and
the Pokédex screens are unchanged. Line references are to main at
`6591781ef1`.

## Behavior

### The two home lists

| Home list | Home generations | Selected by |
| --- | --- | --- |
| Kanto–Johto | Gen I and Gen II | `ORIGIN_PALLET`, `ORIGIN_NEW_BARK` |
| Hoenn | Gen III and Gen V | `ORIGIN_LITTLEROOT` |

**Membership.** A species is on a home list when its evolutionary family has a
member of one of that list's home generations. That is the rule
[Wild encounters v2](../prds/wild-encounters-v2.md#natives) uses to place
families in the world, so a home list holds exactly the families that roam the
home land: later evolutions (Kingambit with Pawniard, Magnezone with Magnemite),
babies (Budew with Roselia, Munchlax with Snorlax), and the odd family that
spans both lands (Azurill with Marill, Wynaut with Wobbuffet) appear on every
list whose generations they touch. A family is traced through the `.evolutions`
of each species' standard form (`game/src/data/pokemon/species_info/gen_*_families.h`).
An evolution reached only from a regional form (Obstagoon, Perrserker,
Sirfetch'd, Mr. Rime, Cursola, Runerigus, Kleavor, Wyrdeer, Ursaluna,
Basculegion, Sneasler, Overqwil) is not part of the family and is National-only,
because those forms live in Sevii and Sinjoh and a home list must be
completable at home. Only enabled species are listed; a family member whose
species is disabled in this build is left out.

**Order.** The home generations' species first, in National order, then the
family members from other generations, in National order. The Kanto–Johto list
therefore opens with Bulbasaur and the Hoenn list with Treecko, with Victini
following Deoxys.

**Forms.** The lists hold base species only. Regional-form entry keys above
`NATIONAL_DEX_PECHARUNT` (`game/include/constants/pokedex.h:1045`) are
National-only, as today.

**Tables and registry.**

- The lists are hand-written tables in `game/src/pokemon.c`, as the three
  catalogs are today (`sKantoToNationalOrder` at line 151,
  `sHoennToNationalOrder` at 414, `sJohtoToNationalOrder` at 656). One table
  and one local-number enum serve the Kanto–Johto list in place of the Kanto
  and Johto pairs; the Hoenn pair is rebuilt. Derive each list from the
  species data's evolution families, check it against the rule above with a
  test, and record the two counts in the PR. Keep the
  `P_NEW_EVOS_IN_REGIONAL_DEX` guards today's tables use.
- Every helper that converts between a species and a Kanto or Johto local
  number reads the Kanto–Johto list: `SpeciesToKantoPokedexNum` and
  `SpeciesToJohtoPokedexNum` (`game/src/pokemon.c:6893`, `6967`), the
  `KantoToNationalOrder`, `JohtoToNationalOrder`, `NationalToKantoOrder`, and
  `NationalToJohtoOrder` helpers (`6850-6990`), and so the ferry passenger's
  home-region request (`game/src/wayfarer_ferry_cabins.c:258-269`).
- Each list holds each National ID at most once; a test asserts it.
- The registry (`sDexCatalogs`, `game/src/pokedex.c:4572`) keeps a row per
  `DEX_REGION_*` value, each with its own `.region` so `GetDexCatalog`'s index
  check (`:4583`) still passes. The `DEX_REGION_KANTO` and `DEX_REGION_JOHTO`
  rows describe the Kanto–Johto list and share its table, name, and title;
  the `DEX_REGION_HOENN` row describes the Hoenn list.
  `Dex_RegionEntryToNational` (`game/src/pokemon.c:6910`) maps Kanto and Johto
  to the same table. `Dex_GetActiveRegionName` and `Dex_GetActiveRegionTitle`
  (`pokedex.c:4714`, `4729`) return the list's name and title.
- The origin initializers in `game/src/wayfarer_origin.c` (lines 42, 63, 75)
  keep setting the region they set today. `ResetPokedex` (`pokedex.c:1502`)
  keeps `DEX_REGION_JOHTO` as the pre-origin default.

### Persistent state

- `struct Pokedex` (`game/include/global.h:453-466`) does not change:
  `activeRegion`, `nationalExtensionMask`, `nationalMagic`, `mode`, and
  `order` keep their offsets and meanings, and `SAVE_VERSION`
  (`game/include/save.h:20`) does not change. A Wayfarer build rejects any
  other save version (`game/src/save.c:1170-1190`), so this work must not
  touch the layout.
- `activeRegion` values `DEX_REGION_KANTO` and `DEX_REGION_JOHTO` both mean
  the Kanto–Johto home list; `DEX_REGION_HOENN` means the Hoenn list. Any
  other value means no list, as today.
- `nationalExtensionMask` is no longer read for visibility. It is left as it
  is in old saves and no longer written. `Dex_GrantNationalExtension`,
  `Dex_HasNationalExtension`, and the `DexScript_*Extension` specials
  (`game/src/event_data.c:207-235`, `game/data/specials.inc:540-545`) stay
  callable: a grant of a valid region returns true and changes nothing, and a
  query of a valid region returns whether the National upgrade is held.
- The National upgrade stays the triple that `EnableNationalPokedex`
  (`game/src/event_data.c:188`) writes: `nationalMagic`, `VAR_NATIONAL_DEX`,
  and `FLAG_SYS_NATIONAL_DEX`. `IsNationalPokedexEnabled` and
  `Dex_HasNationalUpgrade` keep requiring all three.
- An old save may carry a `FLAG_SYS_ACK_COMPLETE_JOHTO_DEX` set against the
  shorter list it had, or a pending Birch upgrade scene. Both are harmless: the
  phone's switch to the National rating keys on the home list being complete,
  not on that flag alone (see [Counts and completion](#counts-and-completion)),
  and the scene skips an upgrade already held (see
  [Who gives the upgrade](#who-gives-the-upgrade)).

### Visibility

- **Regional mode** shows the home list: `Dex_IsRegionalEntryVisible`,
  `Dex_GetRegionalVisibleProgress` (`pokedex.c:4654`), and the regional list
  builder keep working on the active catalog, which is now the home list.
- **National mode** shows every enabled species: `Dex_IsNationalEntryVisible`
  (`pokedex.c:4741`) returns true for a valid National ID whose species is
  enabled (`IsSpeciesEnabled(NationalPokedexNumToSpecies(id))`), and
  `Dex_GetNationalVisibleEntryCount`, `Dex_GetFirstVisibleNationalEntry`,
  `Dex_GetNextVisibleNationalEntry`, and `Dex_GetNationalVisibleProgress`
  iterate `1..NATIONAL_DEX_COUNT` with that predicate. Gen IX is switched off
  in this build (`game/include/config/species_enabled.h:17`) while
  `NATIONAL_DEX_COUNT` still spans its numbers, so the enabled check is what
  keeps those ~110 numbers out of the list and out of completion. Enabled
  regional-form entry keys are included, as the canonical foundation defines
  them.
- National mode is available only with the upgrade, as today
  (`pokedex.c:1618-1619`, `5599-5600`): before it, the Pokédex forces regional
  mode and every National-else-regional consumer (`game/src/trainer_card.c:1227`,
  `game/src/main_menu.c:2224`, `game/src/menu.c:1907`, `game/src/tv.c:1863`,
  `game/src/easy_chat.c:5120`, `game/src/start_menu.c:740`) sees the home list.
- Catch registration (`Cmd_trysetcaughtmondexflags`,
  `game/src/battle_script_commands.c:11303`) is unchanged: the caught flag is
  always recorded; the registration page shows only for a visible species.
  With the upgrade held every enabled species is visible, so the page always
  shows.

### Counts and completion

Every count and completion helper keeps its name and its callers; only what it
counts changes:

| Helper | Today | With this work |
| --- | --- | --- |
| `GetRegionalPokedexCount` (`pokedex.c:4850`), `HasAllRegionalMons` (`4921`), `Dex_HasCompletedRegionalVisibleEntries` (`4667`) | Active catalog | Home list |
| `HasAllMons` (`4969`), `Dex_HasCompletedNationalVisibleEntries` (`4806`) | Visible union | Every enabled species |
| `GetHoennPokedexCount`, `HasAllHoennMons` (`4855`, `4926`) | Emerald's Hoenn list | The Hoenn home list |
| `GetKantoPokedexCount`, `HasAllKantoMons`, `GetJohtoPokedexCount`, `HasAllJohotoMons` (`4877-4955`) | Kanto or Johto list | The Kanto–Johto home list |
| `ScriptGetPokedexInfo`, `GetFrlgPokedexCount` (`game/src/birch_pc.c:8`, `268`) | Active or fixed Kanto, then National | Home list, then every enabled species |

- Completion keeps today's rules: a species marked `dexNotRequired` is not
  needed, and a mythical is needed only when `dexForceRequired`. Before this
  work ships, check every enabled species against the encounter, gift, trade,
  fossil, and evolution tables, and mark each species with no way to be
  obtained `dexNotRequired`, so neither a home-list reward nor a National one
  (the Shiny Charm, the National diploma) is out of reach. Gen IV is already
  marked so; the Gen V species with no source found so far are Tirtouga,
  Carracosta, Archen, Archeops, Cobalion, Terrakion, Virizion, Tornadus,
  Thundurus, Landorus, Reshiram, Zekrom, and Kyurem, and most Gen VI–VIII
  legendaries are unchecked. Record the outcome in the PR.
- Consumers whose meaning follows from the table: the Lilycove game designers
  (`game/data/maps/LilycoveCity_CoveLilyMotel_2F/scripts.inc:5-33`; the
  diploma and Oval Charm need the home list, the Shiny Charm needs every
  enabled species), the diploma text (`game/src/diploma.c:129`), Birch's Johto
  starter (`game/data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc:33`,
  the Hoenn home list complete), the Trainer Card star
  (`game/src/trainer_card.c:858`), and the ferry passenger who asks for a
  Pokémon from the player's home region (see
  [The two home lists](#the-two-home-lists)).
- Ratings keep today's shape through the helpers above. In person
  (`game/data/scripts/prof_birch.inc:80-97`) the professor rates the home list
  and, once the upgrade is held, reports the National seen and caught counts.
  By phone (`BufferPokedexRatingForMatchCall`, `game/src/match_call.c:2562`)
  the home list is rated until it is complete, then the National count is.
  Today that switch reads `FLAG_SYS_ACK_COMPLETE_JOHTO_DEX` alone
  (`match_call.c:2591-2593`), which an old save may hold from a shorter list;
  the switch now requires the home list to be complete
  (`Dex_HasCompletedRegionalVisibleEntries`) as well, and the flag stays the
  one-time marker that the completion line was heard. The National rating's
  wording bands (`game/src/birch_pc.c:203-224`, which run to 475) are
  stretched in proportion to the enabled species count, with "complete" at the
  required count.

### Who gives the upgrade

| Site | Today | With this work |
| --- | --- | --- |
| Birch's lab, traveller's thanks after the Route 101 rescue, the scene the Hoenn rival revision (task `hoenn-rival-origin`, which must land first) adds | No upgrade | For `ORIGIN_PALLET` and `ORIGIN_NEW_BARK`: after the thanks and the Pokédex handover if the traveller had none, and before the rating, Birch upgrades the Pokédex with the lab's existing `..._Text_PokedexUpgradedToNational` line, unless it is already held. The rating that follows reports the National counts. A traveller whose save is already past the rescue gets the same upgrade and line at the start of their next lab talk with Birch. |
| New Bark lab, `VisitorElm` (`game/data/maps/NewBarkTown_Lab_hns/scripts.inc:1086-1095`) | No upgrade; no record of the first talk | For `ORIGIN_LITTLEROOT`: at the end of the first talk, after Elm's own lines on whichever branch ran (the Master Ball, the S.S. ticket, or the plain greeting), a shared subroutine upgrades the Pokédex with a short line of Elm's, unless it is already held or the player has no Pokédex. The upgrade flag is the record. |
| Pallet lab, `OakWelcomeKanto` branch (`game/data/maps/PalletTown_Lab_hns/scripts.inc:116-130`) | Sets `VAR_PALLETTOWN_LABSTATE` to 1; no upgrade | For `ORIGIN_LITTLEROOT`: the welcome ends with the upgrade and a short line of Oak's, before the eight-badge jump at `:128-130`, unless already held or the player has no Pokédex. Oak in the Johto Champion's room is not a lab talk and does not upgrade. |
| Pallet lab, `OakDex` at eight Kanto badges (`PalletTown_Lab_hns/scripts.inc:1033-1038`, `1077`) | Upgrade plus the Kanto grant | Upgrade only, still guarded by `FLAG_SYS_NATIONAL_DEX`. The grant call goes. Reached by New Bark and Littleroot origins; a Pallet origin's lab talk is the opening script (`:119-122`), which never comes here. |
| Olivine port Oak (`game/data/maps/OlivineCity_PortInside_hns/scripts.inc:12-23`) | Upgrade | Unchanged, except that a held upgrade replaces lines 12-23 (the greeting, the walk-in-place, the flag and special, the fanfare) with a short acknowledgement; the exit at 24-41 is unchanged. This is the stay-at-home upgrade for a Pallet origin. |
| Birch's lab after the Hoenn Hall of Fame (`game/data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc:283-316`, armed by `game/data/scripts/hall_of_fame.inc:25,51`) | Upgrade scene, then the Johto-starter chain through `VAR_DEX_UPGRADE_JOHTO_STARTER_STATE` | The scene plays as today, since it also arms Scott's Battle Frontier call (`:322-323`) and the Johto-starter chain. When the upgrade is already held, one `goto_if_set FLAG_SYS_NATIONAL_DEX` at `:283` skips the upgrade block (lines, Birch's walk to the machine, flag and special) for a short acknowledgement that leads into "But listen." at `:317`; Birch's walk is skipped whole, so he stays in place. Littleroot's "I have something to show you" line (`game/data/maps/LittlerootTown/scripts.inc:1015-1017`) gets a guarded variant that fits. |
| Mr. Pokémon's house, randomizer path (`game/data/maps/Route30_MrPokemonsHouse_hns/scripts.inc:84-87`) | Upgrade, unguarded | Guarded by `FLAG_SYS_NATIONAL_DEX` like the others. |

- A home professor never upgrades a traveller: Birch's first-meeting upgrade
  is for Pallet and New Bark origins only; Elm's and Oak's are for Littleroot
  only. Kanto and Johto are one land, so a New Bark player meeting Oak is at
  home, and Oak upgrades them at eight Kanto badges as today.
- The upgrade needs a Pokédex (`FLAG_SYS_POKEDEX_GET`). Elm's and Oak's
  first-meeting upgrade waits for it (a Littleroot origin receives the Pokédex
  only after Route 103, `LittlerootTown_ProfessorBirchsLab/scripts.inc:689`)
  and happens on the next lab talk instead; Birch's thanks gives one to a
  traveller who has none before upgrading it.
- Every upgrade site is idempotent through `FLAG_SYS_NATIONAL_DEX`, and none
  of them reads or writes the extension mask.
- New text: Elm's and Oak's first-meeting upgrade lines, and a short
  acknowledgement for each existing site that may find the upgrade held.

### Critical captures

`Dex_GetCriticalCaptureProgress` (`pokedex.c:4825`) returns the home list's
caught count over the home list's visible entry count, whether or not the
upgrade is held. The bands in `game/src/battle_script_commands.c:11002` and
the Catching Charm multiplier are unchanged.

### Presentation

- The regional name and title (`gText_Dex*Title`, `gText_Dex*Description`,
  `gText_<Region>`; `Dex_GetActiveRegionName` and `Dex_GetActiveRegionTitle`
  at `pokedex.c:4714`, `4729`) read "KANTO-JOHTO" for both Kanto and Johto
  region values and "HOENN" for Hoenn. The in-game text uses a hyphen, since
  the character map has no en dash; the documents write "Kanto–Johto". Check
  the longer name fits the rating and diploma messages that print it.
- The search screen's mode option takes its title from
  `Dex_GetActiveRegionTitle` (`pokedex.c:6057`), so it follows.
- Regional numbers run from 1 in list order; National mode shows National
  numbers. `SpeciesToPokedexNum` (`game/src/pokemon.c:7433`) returns the
  National ID for any enabled species once the upgrade is held.

### Related documents updated with this work

- [Regional and expanding National Pokédex](../prds/regional-national-pokedex.md):
  its status line points here; its "Initial integration" and the extension
  boundaries describe today.
- [Regional Pokédex catalogs and National extensions](regional-pokedex-catalogs.md):
  its "National extension facade" and "Script interface and Oak" sections
  describe today; note the supersession at the top.
- [Johto, Hoenn, and Kanto starting regions](../specs/wayfarer-regional-start-choice.md),
  where it names the catalog each origin selects.
- This specification's `Implemented` marker and the PRD's status line.

## Validation

Mechanical checks extend `game/test/pokedex_catalog.c`, `game/test/wayfarer_origin.c`,
and the E2E journeys. Cover:

- **Lists:** each home list holds each National ID once; every member's
  family has a member of a home generation, and every enabled species whose
  family does is a member; the Kanto–Johto list starts at 1 and the Hoenn list
  at 252, with 494 after 386; every member is a valid, enabled National ID;
  no regional-form key is listed; the recorded counts match.
- **Resolution:** `DEX_REGION_KANTO` and `DEX_REGION_JOHTO` resolve to the
  same table, name, and title; `DEX_REGION_HOENN` to the Hoenn list; each
  origin's initializer selects its land's list.
- **Visibility:** before the upgrade, a Gen V species is visible to a
  Littleroot origin and invisible to a New Bark origin, and a Kalos species is
  invisible to both; after the upgrade every enabled species is visible, no
  disabled number is, the National count equals the enabled species count, and
  a species caught while invisible shows as caught.
- **Grants:** `Dex_GrantNationalExtension` changes nothing and returns true;
  an old save's mask bits change no count.
- **Counts:** `GetHoennPokedexCount` counts the Hoenn home list;
  `GetKantoPokedexCount` and `GetJohtoPokedexCount` count the Kanto–Johto
  list; `HasAllRegionalMons` with every required home-list species caught is
  true and with one missing is false; `HasAllMons` is reachable with every
  required enabled species caught.
- **Critical capture:** the progress does not change when the upgrade is
  granted.
- **Upgrades (E2E):** a New Bark origin catches a Hoenn species, rescues
  Birch, and sees it in National mode after his thanks; a Littleroot origin
  talks to Elm first and Oak second, and only Elm upgrades; a Littleroot origin
  who never leaves Hoenn gets the upgrade after the Hall of Fame, and one who
  already holds it hears the acknowledgement, still arms Scott's call, and
  still reaches the Johto-starter chain; a New Bark origin at eight Kanto
  badges gets Oak's upgrade with no grant; a Pallet origin gets it at the
  Olivine port; each professor's second visit does not upgrade again.
- **Saves:** a save written before this work loads, keeps its origin's home
  list and its progress, and its mask bits are ignored.
- **Obtainability:** the audit of every enabled species against the world's
  sources is recorded, and every species it could not place is
  `dexNotRequired`.

## References

- `game/src/pokemon.c`: catalog tables at 151, 414, 656; `Dex_RegionEntryToNational` at 6910; `SpeciesToPokedexNum` at 7433
- `game/src/pokedex.c`: `ResetPokedex` at 1502; `sDexCatalogs` at 4572; `GetDexCatalog` at 4579; `Dex_GetActiveRegion` at 4605; visibility, progress, and completion at 4649–4848; counts at 4850–4969; `Dex_GetActiveRegionName` at 4714; `Dex_GetActiveRegionTitle` at 4729
- `game/src/event_data.c`: `EnableNationalPokedex` at 188; Pokédex specials at 207–235
- `game/include/global.h:453-466`, `game/include/save.h:20`, `game/src/save.c:1170-1190`, `game/include/config/species_enabled.h:17`
- `game/test/pokedex_catalog.c`, `game/test/wayfarer_origin.c`
- Upgrade sites listed in [Who gives the upgrade](#who-gives-the-upgrade)
