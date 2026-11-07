# Pokédex obtainability audit

## Evidence scope

This audit backs the "Obtainability" rule of the
[home Pokédex lists spec](../specs/home-pokedex-lists.md): every enabled species
the Pokédex requires must have a way to be obtained in the Wayfarer build, and
every species without one is `dexNotRequired`. It describes `main` at
`4d25e06d0e`.

The result is computed and pinned by `game/tools/wayfarer_pokedex/`:

- `obtainability.py` reads the sources below and closes over evolution and
  breeding. With no flag it prints the unobtainable entries; `--explain SPECIES`
  prints how one species is obtained; `--check` fails on drift; `--write`
  re-records `obtainability.json` and the table in
  `game/test/pokedex_obtainability.c`.
- `test_obtainability.py` pins the counts, checks the record against the world
  and the species flags, and proves that removing a species' only source fails
  with its name.
- `make BUILD=wayfarer check` runs both through `wayfarer-pokedex-audit`. The
  tool needs the Wayfarer map outputs (`data/maps/groups.inc`), which the target
  builds first; standalone, run `make BUILD=wayfarer generated` once. A full
  pass takes about six seconds.

The audit reads source data. It does not claim an emulator walkthrough: a
source counts when its map is compiled into the Wayfarer ROM and its script
label is reachable from a map event, another script, or C.

## Sources read

Each is a separate check in the tool.

| Source | What counts |
| --- | --- |
| Wild | Every species in `src/data/wild_encounters_v2/{kanto,johto,hoenn,alola,sevii,safari,sinjoh}.json`, day and night, all methods. This covers the Safari Zones and the three Bug-Catching Contest days. Every table's map must be compiled. |
| Scripts | `givemon`, `giveegg`, `setwildbattle` (both species of a double), `setwildbattleshiny` and `seteventmon` in the Wayfarer script assembly, read as the build sees it: `data/event_scripts.s` through cpp with its gas `.if IS_FRLG` block dropped, and each included file's own `#if` branches evaluated. They must be on compiled maps and under a label something references. `givemon VAR_x` counts every species that `setvar VAR_x, SPECIES_*` names in the same file (Game Corner prizes, fossil revival). |
| Roamers | `TryAddRoamer` in the `src/roamer.c` function that a compiled script calls as a special (Raikou and Entei from Burned Tower; Latias or Latios from `InitKantoRoamers`). |
| In-game trades | Entries of `src/data/trade.h` that a compiled script names, counted only once the Pokémon they ask for is obtainable. |
| Evolution | Every `.evolutions` row of an obtainable species. Items must be handed out somewhere: map item balls and hidden items, `giveitem`/`finditem`/marts, the daily world item slots and pools, or a held item of a species caught in the wild. A map section, map or region condition must belong to a compiled map. A move condition must be learnable (level-up, egg move, or a teachable listed in `all_learnables.json`). Trade evolutions, trade-partner and script-trigger evolutions do not count. |
| Breeding | An obtainable parent outside the Undiscovered egg group lays its first stage, form included. Genderless and male-only parents need Ditto, which is wild. The Day Care's region turns a foreign regional form into its own, and the Nidoran and Volbeat/Illumise gender swaps and the Manaphy-to-Phione rule apply (`DetermineEggSpeciesAndParentSlots`). The three compiled Day Cares are Route 34, Route 117 and Four Island. |

Not counted: link trades with a second cartridge, the randomizer, `data/scripts/debug.inc`, and assembled
scripts on maps the Wayfarer build deselects (for example the FRLG Celadon Prize Room and Saffron Dojo).
Two unreferenced test labels that hand out five Pokémon at once (`Route25_BillsHouse_Test`,
`SafariZoneGate_SafariZoneEntrance_EventScript_Test`) are dead code and give nothing.

### Build branches

Every C and script source is read through cpp with `POKEMON_WAYFARER` and `POKEMON_HNS` defined, so
`IS_WAYFARER` and `IS_HNS` are true and `IS_FRLG` is false, as in the ROM. That drops the FRLG-only
block of `event_scripts.s` (the FRLG Power Plant, Mt. Ember, Seafoam B4F and Cinnabar Lab copies, among
others) and branches such as Route 10's `#if !IS_WAYFARER` Zapdos. Wayfarer's own replacements
(`wayfarer_power_plant.inc`, `wayfarer_seafoam.inc`, `wayfarer_cinnabar_full_scripts.inc`) are read
instead. The live branches that matter:

- `src/roamer.c`: the `IS_HNS` branch. `InitRoamer` adds Raikou and Entei; `InitKantoRoamers` adds Latias
  or Latios, whichever the player picked. `SouthernIsland_Interior_hns` then shows the other one, so both
  are obtainable.
- `src/data/trade.h`: the non-FireRed branch of the Nidoran trades. Their FRLG maps are deselected anyway.
- `src/starter_choose.c`: the `IS_WAYFARER` starter tables. Birch's bag holds the Hoenn trio, and Elm uses
  `GetJohtoStarterPokemon` (see the open decision below).
- Species data: `config/pokemon.h` selects the Gen VII level-up learnsets, and P_GEN_9_POKEMON is off.
  Gen IX evolutions of older families (Kingambit, Dudunsparce, Farigiraf, Annihilape, Clodsire,
  Archaludon, Dipplin, Hydrapple) stay enabled through their families.

## Result

968 National entries are enabled. 128 have no obtainable form:

| Group | Entries | Handling |
| --- | --- | --- |
| Newly marked by this audit | 51 | `dexNotRequired` added |
| Already `dexNotRequired` | 64 | kept |
| Mythical (excused by `isMythical`) | 13 | unchanged |

The new flag goes on every form of each entry, including the `SILVALLY_SPECIES_INFO` macro that builds
Silvally's eighteen forms.

### Newly marked (51)

All 51 have no wild, static, gift, trade or family source. No obtainable species fails to evolve into
any of them, so none is blocked only by a missing item or place.

| Generation | Species | Why |
| --- | --- | --- |
| V (13) | Tirtouga, Carracosta, Archen, Archeops | No Cover or Plume Fossil, and no revival, anywhere in the world |
| V | Cobalion, Terrakion, Virizion, Tornadus, Thundurus, Landorus, Reshiram, Zekrom, Kyurem | No encounter or gift |
| VI (7) | Tyrunt, Tyrantrum, Amaura, Aurorus | No Jaw or Sail Fossil, and no revival |
| VI | Xerneas, Yveltal, Zygarde | No encounter or gift |
| VII (18) | Type: Null, Silvally | No gift (Silvally only evolves from Type: Null) |
| VII | Cosmog, Cosmoem, Solgaleo, Lunala, Necrozma | No encounter or gift |
| VII | Nihilego, Buzzwole, Pheromosa, Xurkitree, Celesteela, Kartana, Guzzlord, Poipole, Naganadel, Stakataka, Blacephalon | No Ultra Beast encounter |
| VIII (13) | Dracozolt, Arctozolt, Dracovish, Arctovish | No fossil halves, and no revival |
| VIII | Zacian, Zamazenta, Eternatus, Glastrier, Spectrier, Calyrex, Enamorus | No encounter or gift |
| VIII | Kubfu, Urshifu | No gift; Urshifu only evolves from Kubfu |

The PRD's thirteen Gen V species are confirmed exactly. Every Gen I–III legendary and fossil has a live
source. Ruins of Alph gives the Helix Fossil, the Dome Fossil and the Old Amber separately. Desert
Underpass gives whichever of the Root and Claw Fossils Mirage Tower did not.

### Already marked and still unobtainable (64)

These are the 61 Gen IV entries with no source and the three Galarian legendary birds. Gen IV ones:
Turtwig, Grotle, Torterra, Chimchar, Monferno, Infernape, Piplup, Prinplup, Empoleon, Starly, Staravia,
Staraptor, Bidoof, Bibarel, Kricketot, Kricketune, Shinx, Luxio, Luxray, Cranidos, Rampardos, Shieldon,
Bastiodon, Burmy, Wormadam, Mothim, Combee, Vespiquen, Pachirisu, Cherubi, Cherrim, Buneary, Lopunny,
Glameow, Purugly, Stunky, Skuntank, Chatot, Spiritomb, Gible, Gabite, Garchomp, Hippopotas, Hippowdon,
Skorupi, Drapion, Carnivine, Rotom, Uxie, Mesprit, Azelf, Dialga, Palkia, Heatran, Giratina,
Cresselia, Phione, Manaphy, Darkrai, Shaymin and Arceus.

Mythical entries with no source, excused without a flag: Victini, Keldeo, Meloetta, Genesect, Diancie,
Hoopa, Volcanion, Magearna, Marshadow, Zeraora, Meltan, Melmetal and Zarude.

### Excused but obtainable (88)

These entries were `dexNotRequired` before this audit, yet the world offers them. By decision
(2026-11-07) they stay marked; the record lists them, and `--check` fails if another obtainable species
gets the flag. Clearing them is a separate product choice. It would add these species to the Shiny Charm,
and the Gen III Regis to the Hoenn list.

- Hoenn-list members: Regirock, Regice and Registeel. They have statics in the Emerald tombs and in the
  Sinjoh Ruins.
- Gen IV in the wild: Buizel, Floatzel, Shellos, Gastrodon, Drifloon, Drifblim, Bronzor, Bronzong, Riolu,
  Lucario, Croagunk, Toxicroak, Finneon, Lumineon, Snover and Abomasnow. Regigigas has a Sinjoh Ruins
  static.
- Statics and gifts: Regieleki and Regidrago (Sinjoh Ruins), the four Tapus (Alola), and the Noibat egg
  (Ula'ula), which brings Noivern with it.
- Regional-form evolutions: Obstagoon, Perrserker, Cursola, Sirfetch'd, Mr. Rime, Sneasler, Overqwil, and
  Clodsire (from the New Sinjoh Paldean Wooper gift).
- Every regional form entry: 18 Alolan, 16 Galarian (plus the three birds above, which are unobtainable),
  16 Hisuian, and Paldean Tauros (Melemele trades) and Wooper.

## Open decision: the starter trios are pick-one gifts

The tool counts a pick-one gift as a source for each species it offers. The starters are such gifts:

| Trio | Sources | Most a player can hold |
| --- | --- | --- |
| Chikorita, Cyndaquil, Totodile lines | Elm's pick (Johto origin only, `NewBarkTown_Lab_hns`), plus Birch's reward pick (`LittlerootTown_ProfessorBirchsLab`, one of three, `VAR_DEX_UPGRADE_JOHTO_STARTER_STATE`) | 2 |
| Treecko, Torchic, Mudkip lines | Birch's bag pick (Hoenn origin only, `battle_setup.c`), plus Silph Co.'s pick (`SaffronCity_SilphCo_hns`, one of three, `FLAG_GOT_HOENN_STARTER`) | 2 |
| Bulbasaur, Charmander, Squirtle lines | Final stages are wild; breed down | 3 |

Without a second cartridge nobody completes the Hoenn list, so Birch's Johto-starter reward is never
earned. That caps every player at one Johto starter, so the Kanto–Johto diploma is out of reach as well.
A static flag cannot excuse "the two you did not pick". The owner decides between handing out the
remaining starters (Birch's lab and Silph Co.), placing starters in the world, or excusing all eighteen
starter-line species. Until then the record treats each starter as obtainable, so settling the decision
does not change it.

## Required counts

Home-list membership follows the PRD's family rule. A family is traced through standard forms. It joins
Kanto–Johto when it has a Gen I or II member, and Hoenn when it has a Gen III or V member. Regional form
entries are National only. The core list implementation is authoritative if its counts differ.

| List | Members | Mythical | `dexNotRequired` | Required | Required but unobtainable |
| --- | --- | --- | --- | --- | --- |
| Kanto–Johto | 282 | 2 | 0 | 280 | 0 |
| Hoenn | 303 | 6 | 16 (the Gen III Regis, the 13 Gen V species) | 281 | 0 |
| National | 968 | 17 | 203 | 748 | 0 |

"Required but unobtainable" counts species with no source at all. It ignores the starter decision above,
which caps what one player can hold, not what the world offers.

## Assumptions and limits

- A compiled map counts as reachable. The tool does not walk the world graph. Every species whose only
  source is a script, roamer or trade was reviewed by hand. Of those, the ones on the closed Emerald event
  islands (Birth, Navel, Faraway and Southern Island) are either mythical or also offered by an HNS map
  (Lugia, Ho-Oh, Latias and Latios).
- A move condition passes when the move is a teachable listed in `all_learnables.json`. The tool assumes
  the TM or tutor exists. No result depends on this today. The only such case is Dipplin's Dragon Cheer,
  and Hydrapple is also reached by Shiny Stone (sold in the Mahogany shop).
- These required species have exactly one live source, so a playtest of it is worthwhile: Mewtwo
  (CeruleanCave_B2F_hns), Suicune (Route25_hns), Groudon (TerraCave_End), Kyogre (MarineCave_End), and
  Raikou and Entei (`special InitRoamer` from BurnedTower_B1F_hns).
- One-time statics count as a source. A legendary knocked out without being caught is a player choice,
  not a missing source.
- The Route 101 rescue battle counts as a Zigzagoon source, though it cannot be caught there. Zigzagoon is
  also bred from wild Linoone.
