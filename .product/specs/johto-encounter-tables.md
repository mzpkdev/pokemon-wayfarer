# Johto encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Kanto and Johto encounter rules](kanto-johto-encounters.md). They are the
source of truth for Johto's wild-encounter data: every slot below maps one to one
to a slot in the game's encounter tables. Playtesting may change the picks.

## Scope

This spec lists Johto's wild-encounter tables: 88 maps, each with a day and a
night table for every method it has. The Safari Zone and the Bug-Catching
Contest belong to the [Safari table spec](safari-encounter-tables.md).
Legendaries and mythicals belong to a later spec.

The list includes Faraway Island and Southern Island, the two remote islands
described [below](#remote-islands).

## Behavior

### Reading the tables

- Places are ordered by reach: Road, Wilds, Outlands, then dungeons. Each place
  names its reach and its Gen I–II band, and each map is named by its map
  constant. A map with surfing or fishing also names its water type.
- Each table lists every slot in order, with its weight, and the species for
  day and night.
  - Land has 12 slots weighted 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1 and 1%.
  - Surfing, and trees and rocks, have 5 slots weighted 60, 30, 5, 4 and 1%.
  - Fishing has the [Standard Rod](standard-rod-fishing.md)'s 10 entries, with
    each entry's weight for the Old, Good and Super Rod.
- **A slot's species is its stage cap,** the last name in the cell. "Caterpie–
  Metapod" means the slot holds Metapod, and the game steps it down to
  Caterpie below Metapod's evolution level. A single name is a single-stage
  species, a baby, or a line capped at its first stage.
- Names map to species constants in capitals, with spaces and hyphens as
  underscores and other punctuation dropped: Mr. Mime is `SPECIES_MR_MIME`,
  Farfetch'd is `SPECIES_FARFETCHD` and Porygon-Z is `SPECIES_PORYGON_Z`. The
  exceptions are Nidoran♀ (`SPECIES_NIDORAN_F`) and Nidoran♂
  (`SPECIES_NIDORAN_M`).
- Slots hold no levels. Levels come from the map's reach, as the PRD defines.

### Remote islands

Faraway Island and Southern Island are Outlands, reached by boat from the
Olivine and Vermilion ports, so their tables hold Kanto and Johto species in
the Border band. Both are a legendary's lair, and their ordinary tables are
Gen I–II natives that fit the island, with dangerous prowlers allowed as in any
Outlands.

- **Faraway Island** (Mew's lair) is a jungle of grass and ancient or mimicking
  species such as Tangela, Exeggutor, Ditto and Aerodactyl, with a night shift
  to Venomoth, Noctowl and Ariados. Its tall grass lies in many small patches,
  but the map holds 43 grass tiles, so it gets a land table. Its sea has
  surfing and fishing tables.
- **Southern Island** (Latias and Latios's lair) is a bare rock in the open
  sea. Its rocky outcrops are Emerald's mountain-top terrain, which never had
  encounters, so it has surfing and fishing tables only, of psychic and
  draconic sea species.

### Tables

#### New Bark Town

Road, Johto east.

**`MAP_NEW_BARK_TOWN_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 2 | 30% | Shellder | Tentacool–Tentacruel |
| 3 | 5% | Krabby–Kingler | Staryu |
| 4 | 4% | Qwilfish | Qwilfish |
| 5 | 1% | Corsola | Shellder |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Shellder | Staryu |
| 5 | 8% | 9% | 10% | Qwilfish | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Corsola | Shellder |
| 7 | 3% | 6% | 10% | Shellder | Staryu |
| 8 | 3% | 5% | 9% | Krabby–Kingler | Qwilfish |
| 9 | 2% | 4% | 9% | Qwilfish | Krabby–Kingler |
| 10 | 2% | 4% | 9% | Remoraid–Octillery | Chinchou–Lanturn |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Spinarak–Ariados |
| 2 | 30% | Caterpie–Butterfree | Venonat–Venomoth |
| 3 | 5% | Pineco–Forretress | Hoothoot–Noctowl |
| 4 | 4% | Exeggcute | Pineco–Forretress |
| 5 | 1% | Aipom | Ledyba–Ledian |

#### Cherrygrove City

Road, Johto east.

**`MAP_CHERRYGROVE_CITY_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Krabby–Kingler | Krabby–Kingler |
| 3 | 5% | Corsola | Staryu |
| 4 | 4% | Shellder | Qwilfish |
| 5 | 1% | Qwilfish | Corsola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Krabby–Kingler | Staryu |
| 4 | 8% | 10% | 10% | Corsola | Krabby–Kingler |
| 5 | 8% | 9% | 10% | Tentacool–Tentacruel | Corsola |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Staryu |
| 7 | 3% | 6% | 10% | Corsola | Qwilfish |
| 8 | 3% | 5% | 9% | Shellder | Tentacool–Tentacruel |
| 9 | 2% | 4% | 9% | Qwilfish | Staryu |
| 10 | 2% | 4% | 9% | Krabby–Kingler | Krabby–Kingler |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Spinarak–Ariados |
| 2 | 30% | Pidgey–Pidgeot | Murkrow |
| 3 | 5% | Exeggcute | Spearow–Fearow |
| 4 | 4% | Ledyba–Ledian | Exeggcute |
| 5 | 1% | Aipom | Hoothoot–Noctowl |

#### Violet City

Road, Johto east.

**`MAP_VIOLET_CITY_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| 2 | 20% | Ledyba–Ledian | Gastly–Haunter |
| 3 | 10% | Pidgey–Pidgeot | Rattata–Raticate |
| 4 | 10% | Rattata–Raticate | Oddish–Gloom |
| 5 | 10% | Mareep–Ampharos | Spinarak–Ariados |
| 6 | 10% | Hoppip–Jumpluff | Hoothoot–Noctowl |
| 7 | 5% | Sunkern | Zubat–Golbat |
| 8 | 5% | Natu–Xatu | Gastly–Haunter |
| 9 | 4% | Bellsprout–Weepinbell | Hoothoot–Noctowl |
| 10 | 4% | Ledyba–Ledian | Oddish–Gloom |
| 11 | 1% | Abra | Abra |
| 12 | 1% | Sunkern | Gastly–Haunter |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Marill–Azumarill | Wooper–Quagsire |
| 3 | 5% | Psyduck–Golduck | Marill–Azumarill |
| 4 | 4% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 5 | 1% | Magikarp–Gyarados | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Marill–Azumarill |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Bellsprout–Weepinbell |
| 2 | 30% | Bellsprout–Weepinbell | Ledyba–Ledian |
| 3 | 5% | Exeggcute | Spinarak–Ariados |
| 4 | 4% | Pineco–Forretress | Hoothoot–Noctowl |
| 5 | 1% | Aipom | Aipom |

#### Azalea Town

Road, Johto west.

**`MAP_AZALEA_TOWN_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 20% | Ledyba–Ledian | Spinarak–Ariados |
| 3 | 10% | Caterpie–Butterfree | Hoothoot–Noctowl |
| 4 | 10% | Pidgey–Pidgeot | Oddish–Gloom |
| 5 | 10% | Weedle–Beedrill | Venonat–Venomoth |
| 6 | 10% | Hoppip–Jumpluff | Paras–Parasect |
| 7 | 5% | Pineco–Forretress | Gastly–Haunter |
| 8 | 5% | Sunkern | Murkrow |
| 9 | 4% | Paras–Parasect | Pineco–Forretress |
| 10 | 4% | Sentret–Furret | Hoothoot–Noctowl |
| 11 | 1% | Igglybuff | Igglybuff |
| 12 | 1% | Pichu | Pichu |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pineco–Forretress | Pineco–Forretress |
| 2 | 30% | Aipom | Aipom |
| 3 | 5% | Caterpie–Butterfree | Spinarak–Ariados |
| 4 | 4% | Weedle–Beedrill | Hoothoot–Noctowl |
| 5 | 1% | Exeggcute | Murkrow |

#### Goldenrod City

Road, Johto west.

**`MAP_GOLDENROD_CITY_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Corsola | Chinchou–Lanturn |
| 2 | 30% | Tentacool–Tentacruel | Corsola |
| 3 | 5% | Qwilfish | Qwilfish |
| 4 | 4% | Remoraid–Octillery | Tentacool–Tentacruel |
| 5 | 1% | Krabby–Kingler | Remoraid–Octillery |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corsola | Corsola |
| 2 | 22% | 18% | 10% | Remoraid–Octillery | Chinchou–Lanturn |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Qwilfish | Qwilfish |
| 5 | 8% | 9% | 10% | Krabby–Kingler | Remoraid–Octillery |
| 6 | 4% | 7% | 10% | Corsola | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Remoraid–Octillery | Corsola |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Staryu |
| 9 | 2% | 4% | 9% | Qwilfish | Qwilfish |
| 10 | 2% | 4% | 9% | Horsea–Seadra | Krabby–Kingler |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Spinarak–Ariados |
| 2 | 30% | Exeggcute | Murkrow |
| 3 | 5% | Pineco–Forretress | Exeggcute |
| 4 | 4% | Aipom | Pineco–Forretress |
| 5 | 1% | Caterpie–Butterfree | Hoothoot–Noctowl |

#### Ecruteak City

Road, Johto west.

**`MAP_ECRUTEAK_CITY_HNS`**

Water type: ponds and rivers.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 5% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 5 | 1% | Slowpoke–Slowbro | Slowpoke–Slowbro |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Marill–Azumarill |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Magikarp–Gyarados |

#### Olivine City

Road, Johto west.

**`MAP_OLIVINE_CITY_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Magnemite–Magneton | Meowth–Persian |
| 2 | 20% | Hoppip–Jumpluff | Magnemite–Magneton |
| 3 | 10% | Spearow–Fearow | Gastly–Haunter |
| 4 | 10% | Rattata–Raticate | Hoothoot–Noctowl |
| 5 | 10% | Sunkern | Grimer–Muk |
| 6 | 10% | Snubbull–Granbull | Oddish–Gloom |
| 7 | 5% | Grimer–Muk | Gastly–Haunter |
| 8 | 5% | Pidgey–Pidgeot | Meowth–Persian |
| 9 | 4% | Spearow–Fearow | Hoothoot–Noctowl |
| 10 | 4% | Sunkern | Rattata–Raticate |
| 11 | 1% | Farfetch'd | Farfetch'd |
| 12 | 1% | Magnemite–Magneton | Spinarak–Ariados |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Remoraid–Octillery | Remoraid–Octillery |
| 2 | 30% | Corsola | Corsola |
| 3 | 5% | Tentacool–Tentacruel | Staryu |
| 4 | 4% | Shellder | Chinchou–Lanturn |
| 5 | 1% | Mantyke | Tentacool–Tentacruel |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corsola | Corsola |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Horsea–Seadra |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Corsola | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Remoraid–Octillery | Remoraid–Octillery |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Staryu |
| 7 | 3% | 6% | 10% | Corsola | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Shellder | Staryu |
| 9 | 2% | 4% | 9% | Qwilfish | Qwilfish |
| 10 | 2% | 4% | 9% | Tentacool–Tentacruel | Corsola |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Exeggcute | Exeggcute |
| 2 | 30% | Spearow–Fearow | Spearow–Fearow |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Ledyba–Ledian | Pineco–Forretress |
| 5 | 1% | Aipom | Hoothoot–Noctowl |

**`MAP_OLIVINE_CITY_PORT_OUTSIDE_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Remoraid–Octillery | Remoraid–Octillery |
| 2 | 30% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 5% | Corsola | Chinchou–Lanturn |
| 4 | 4% | Chinchou–Lanturn | Staryu |
| 5 | 1% | Slowpoke–Slowbro | Corsola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Remoraid–Octillery | Remoraid–Octillery |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Corsola | Staryu |
| 6 | 4% | 7% | 10% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Krabby–Kingler | Staryu |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Chinchou–Lanturn | Shellder |
| 10 | 2% | 4% | 9% | Slowpoke–Slowbro | Qwilfish |

#### Cianwood City

Road, Johto west.

**`MAP_CIANWOOD_CITY_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 2 | 30% | Shellder | Tentacool–Tentacruel |
| 3 | 5% | Krabby–Kingler | Staryu |
| 4 | 4% | Remoraid–Octillery | Staryu |
| 5 | 1% | Mantyke | Qwilfish |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Horsea–Seadra |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Shellder | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Corsola | Shellder |
| 7 | 3% | 6% | 10% | Remoraid–Octillery | Staryu |
| 8 | 3% | 5% | 9% | Qwilfish | Qwilfish |
| 9 | 2% | 4% | 9% | Shellder | Chinchou–Lanturn |
| 10 | 2% | 4% | 9% | Krabby–Kingler | Staryu |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Krabby–Kingler | Krabby–Kingler |
| 2 | 30% | Mankey–Primeape | Mankey–Primeape |
| 3 | 5% | Geodude–Graveler | Spinarak–Ariados |
| 4 | 4% | Aipom | Geodude–Graveler |
| 5 | 1% | Exeggcute | Aipom |

#### Mahogany Town

Road, Johto east.

**`MAP_MAHOGANYTOWN_HNS`**

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Aipom | Aipom |
| 2 | 30% | Pineco–Forretress | Pineco–Forretress |
| 3 | 5% | Ledyba–Ledian | Spinarak–Ariados |
| 4 | 4% | Exeggcute | Hoothoot–Noctowl |
| 5 | 1% | Pidgey–Pidgeot | Murkrow |

#### Blackthorn City

Road, Johto east.

**`MAP_BLACKTHORN_CITY_HNS`**

Water type: ponds and rivers.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 4% | Psyduck–Golduck | Marill–Azumarill |
| 5 | 1% | Goldeen–Seaking | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wooper–Quagsire | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Poliwag–Poliwhirl | Slowpoke–Slowbro |
| 4 | 8% | 10% | 10% | Slowpoke–Slowbro | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Poliwag–Poliwhirl |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Magikarp–Gyarados |

#### Safari Zone Gate

Road, Johto west.

**`MAP_SAFARI_ZONE_GATE_HNS`**

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Exeggcute |
| 2 | 30% | Exeggcute | Venonat–Venomoth |
| 3 | 5% | Aipom | Aipom |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Ledyba–Ledian | Murkrow |

#### Route 29

Road, Johto east.

**`MAP_ROUTE29_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 2 | 20% | Sentret–Furret | Rattata–Raticate |
| 3 | 10% | Rattata–Raticate | Oddish–Gloom |
| 4 | 10% | Hoppip–Jumpluff | Spinarak–Ariados |
| 5 | 10% | Ledyba–Ledian | Venonat–Venomoth |
| 6 | 10% | Spearow–Fearow | Hoothoot–Noctowl |
| 7 | 5% | Caterpie–Butterfree | Gastly–Haunter |
| 8 | 5% | Sunkern | Rattata–Raticate |
| 9 | 4% | Pidgey–Pidgeot | Gastly–Haunter |
| 10 | 4% | Sentret–Furret | Oddish–Gloom |
| 11 | 1% | Chikorita–Meganium | Chikorita–Meganium |
| 12 | 1% | Hoppip–Jumpluff | Spinarak–Ariados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Hoothoot–Noctowl |
| 2 | 30% | Hoothoot–Noctowl | Ledyba–Ledian |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Exeggcute | Pineco–Forretress |
| 5 | 1% | Aipom | Aipom |

#### Route 30

Road, Johto east.

**`MAP_ROUTE30_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ledyba–Ledian | Hoothoot–Noctowl |
| 2 | 20% | Pidgey–Pidgeot | Spinarak–Ariados |
| 3 | 10% | Caterpie–Butterfree | Poliwag–Poliwhirl |
| 4 | 10% | Weedle–Beedrill | Oddish–Gloom |
| 5 | 10% | Hoppip–Jumpluff | Rattata–Raticate |
| 6 | 10% | Rattata–Raticate | Venonat–Venomoth |
| 7 | 5% | Sunkern | Spinarak–Ariados |
| 8 | 5% | Sentret–Furret | Gastly–Haunter |
| 9 | 4% | Ledyba–Ledian | Hoothoot–Noctowl |
| 10 | 4% | Weedle–Beedrill | Zubat–Golbat |
| 11 | 1% | Abra | Abra |
| 12 | 1% | Pidgey–Pidgeot | Gastly–Haunter |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Marill–Azumarill | Wooper–Quagsire |
| 3 | 5% | Psyduck–Golduck | Marill–Azumarill |
| 4 | 4% | Magikarp–Gyarados | Psyduck–Golduck |
| 5 | 1% | Totodile–Feraligatr | Totodile–Feraligatr |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Marill–Azumarill |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Goldeen–Seaking |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Spinarak–Ariados |
| 2 | 30% | Caterpie–Butterfree | Hoothoot–Noctowl |
| 3 | 5% | Exeggcute | Ledyba–Ledian |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Aipom | Aipom |

#### Route 31

Road, Johto east.

**`MAP_ROUTE31_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bellsprout–Weepinbell | Spinarak–Ariados |
| 2 | 20% | Ledyba–Ledian | Oddish–Gloom |
| 3 | 10% | Caterpie–Butterfree | Bellsprout–Weepinbell |
| 4 | 10% | Weedle–Beedrill | Gastly–Haunter |
| 5 | 10% | Pidgey–Pidgeot | Zubat–Golbat |
| 6 | 10% | Mareep–Ampharos | Hoothoot–Noctowl |
| 7 | 5% | Sunkern | Spinarak–Ariados |
| 8 | 5% | Hoppip–Jumpluff | Poliwag–Poliwhirl |
| 9 | 4% | Bellsprout–Weepinbell | Gastly–Haunter |
| 10 | 4% | Weedle–Beedrill | Hoothoot–Noctowl |
| 11 | 1% | Pichu | Zubat–Golbat |
| 12 | 1% | Mareep–Ampharos | Dunsparce |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Psyduck–Golduck |
| 5 | 1% | Goldeen–Seaking | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Marill–Azumarill | Marill–Azumarill |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Spinarak–Ariados |
| 2 | 30% | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| 3 | 5% | Exeggcute | Hoothoot–Noctowl |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Aipom | Aipom |

#### Route 32

Road, Johto east.

**`MAP_ROUTE32_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Mareep–Ampharos | Wooper–Quagsire |
| 2 | 20% | Wooper–Quagsire | Mareep–Ampharos |
| 3 | 10% | Ekans–Arbok | Zubat–Golbat |
| 4 | 10% | Pidgey–Pidgeot | Gastly–Haunter |
| 5 | 10% | Bellsprout–Weepinbell | Spinarak–Ariados |
| 6 | 10% | Hoppip–Jumpluff | Oddish–Gloom |
| 7 | 5% | Mareep–Ampharos | Gastly–Haunter |
| 8 | 5% | Sunkern | Zubat–Golbat |
| 9 | 4% | Ekans–Arbok | Hoothoot–Noctowl |
| 10 | 4% | Wooper–Quagsire | Oddish–Gloom |
| 11 | 1% | Mareep–Ampharos | Murkrow |
| 12 | 1% | Hoppip–Jumpluff | Ekans–Arbok |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Qwilfish | Qwilfish |
| 2 | 30% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 5% | Slowpoke–Slowbro | Chinchou–Lanturn |
| 4 | 4% | Corsola | Slowpoke–Slowbro |
| 5 | 1% | Tentacool–Tentacruel | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Qwilfish | Qwilfish |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Qwilfish | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Tentacool–Tentacruel | Qwilfish |
| 6 | 4% | 7% | 10% | Qwilfish | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Slowpoke–Slowbro | Qwilfish |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Staryu |
| 9 | 2% | 4% | 9% | Qwilfish | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Corsola | Qwilfish |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ekans–Arbok | Ekans–Arbok |
| 2 | 30% | Ekans–Arbok | Ekans–Arbok |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Exeggcute | Pineco–Forretress |
| 5 | 1% | Aipom | Hoothoot–Noctowl |

#### Route 33

Road, Johto east.

**`MAP_ROUTE33_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Machop–Machoke | Rattata–Raticate |
| 2 | 20% | Rattata–Raticate | Machop–Machoke |
| 3 | 10% | Ekans–Arbok | Zubat–Golbat |
| 4 | 10% | Spearow–Fearow | Gastly–Haunter |
| 5 | 10% | Hoppip–Jumpluff | Hoothoot–Noctowl |
| 6 | 10% | Geodude–Graveler | Oddish–Gloom |
| 7 | 5% | Wooper–Quagsire | Zubat–Golbat |
| 8 | 5% | Machop–Machoke | Spinarak–Ariados |
| 9 | 4% | Ekans–Arbok | Gastly–Haunter |
| 10 | 4% | Rattata–Raticate | Hoothoot–Noctowl |
| 11 | 1% | Slowpoke–Slowbro | Dunsparce |
| 12 | 1% | Hoppip–Jumpluff | Wooper–Quagsire |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wooper–Quagsire | Poliwag–Poliwhirl |
| 2 | 30% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 3 | 5% | Psyduck–Golduck | Psyduck–Golduck |
| 4 | 4% | Marill–Azumarill | Marill–Azumarill |
| 5 | 1% | Slowpoke–Slowbro | Slowpoke–Slowbro |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Wooper–Quagsire | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Psyduck–Golduck |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Marill–Azumarill |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Aipom | Aipom |
| 2 | 30% | Ekans–Arbok | Ekans–Arbok |
| 3 | 5% | Pineco–Forretress | Venonat–Venomoth |
| 4 | 4% | Exeggcute | Pineco–Forretress |
| 5 | 1% | Spearow–Fearow | Spinarak–Ariados |

#### Route 34

Road, Johto west.

**`MAP_ROUTE34_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Snubbull–Granbull | Drowzee–Hypno |
| 2 | 20% | Pidgey–Pidgeot | Meowth–Persian |
| 3 | 10% | Jigglypuff | Hoothoot–Noctowl |
| 4 | 10% | Drowzee–Hypno | Snubbull–Granbull |
| 5 | 10% | Sunkern | Oddish–Gloom |
| 6 | 10% | Hoppip–Jumpluff | Spinarak–Ariados |
| 7 | 5% | Abra–Kadabra | Abra–Kadabra |
| 8 | 5% | Ditto | Ditto |
| 9 | 4% | Igglybuff | Igglybuff |
| 10 | 4% | Snubbull–Granbull | Gastly–Haunter |
| 11 | 1% | Ditto | Ditto |
| 12 | 1% | Eevee | Hoothoot–Noctowl |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Corsola | Corsola |
| 2 | 30% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 5% | Krabby–Kingler | Staryu |
| 4 | 4% | Chinchou–Lanturn | Chinchou–Lanturn |
| 5 | 1% | Slowpoke–Slowbro | Krabby–Kingler |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corsola | Corsola |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Krabby–Kingler |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Qwilfish | Corsola |
| 6 | 4% | 7% | 10% | Corsola | Staryu |
| 7 | 3% | 6% | 10% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Qwilfish | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Chinchou–Lanturn | Corsola |
| 10 | 2% | 4% | 9% | Corsola | Staryu |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pidgey–Pidgeot | Exeggcute |
| 2 | 30% | Exeggcute | Pidgey–Pidgeot |
| 3 | 5% | Ledyba–Ledian | Spinarak–Ariados |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Aipom | Hoothoot–Noctowl |

#### Route 35

Road, Johto west.

**`MAP_ROUTE35_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Nidoran♀–Nidorina | Psyduck–Golduck |
| 2 | 20% | Snubbull–Granbull | Nidoran♂–Nidorino |
| 3 | 10% | Pidgey–Pidgeot | Drowzee–Hypno |
| 4 | 10% | Hoppip–Jumpluff | Hoothoot–Noctowl |
| 5 | 10% | Nidoran♂–Nidorino | Oddish–Gloom |
| 6 | 10% | Sunkern | Venonat–Venomoth |
| 7 | 5% | Yanma | Yanma |
| 8 | 5% | Abra–Kadabra | Abra–Kadabra |
| 9 | 4% | Growlithe | Hoothoot–Noctowl |
| 10 | 4% | Ditto | Ditto |
| 11 | 1% | Yanma | Oddish–Gloom |
| 12 | 1% | Growlithe | Psyduck–Golduck |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Marill–Azumarill | Wooper–Quagsire |
| 3 | 5% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 4% | Wooper–Quagsire | Marill–Azumarill |
| 5 | 1% | Magikarp–Gyarados | Poliwag–Poliwhirl |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Psyduck–Golduck |
| 7 | 3% | 6% | 10% | Wooper–Quagsire | Marill–Azumarill |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Poliwag–Poliwhirl |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 2 | 30% | Caterpie–Butterfree | Spinarak–Ariados |
| 3 | 5% | Ledyba–Ledian | Pidgey–Pidgeot |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Aipom | Aipom |

#### Route 36

Road, Johto west.

**`MAP_ROUTE36_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bellsprout–Weepinbell | Hoothoot–Noctowl |
| 2 | 20% | Growlithe | Vulpix |
| 3 | 10% | Hoppip–Jumpluff | Gastly–Haunter |
| 4 | 10% | Pidgey–Pidgeot | Houndour–Houndoom |
| 5 | 10% | Vulpix | Oddish–Gloom |
| 6 | 10% | Sentret–Furret | Rattata–Raticate |
| 7 | 5% | Nidoran♀–Nidorina | Bonsly |
| 8 | 5% | Sunkern | Houndour–Houndoom |
| 9 | 4% | Bonsly | Spinarak–Ariados |
| 10 | 4% | Ledyba–Ledian | Hoothoot–Noctowl |
| 11 | 1% | Growlithe | Vulpix |
| 12 | 1% | Nidoran♂–Nidorino | Gastly–Haunter |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Bellsprout–Weepinbell | Hoothoot–Noctowl |
| 2 | 30% | Hoothoot–Noctowl | Bellsprout–Weepinbell |
| 3 | 5% | Exeggcute | Spinarak–Ariados |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Aipom | Aipom |

#### Route 37

Road, Johto west.

**`MAP_ROUTE37_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ledyba–Ledian | Hoothoot–Noctowl |
| 2 | 20% | Pidgey–Pidgeot | Spinarak–Ariados |
| 3 | 10% | Hoppip–Jumpluff | Venonat–Venomoth |
| 4 | 10% | Sunkern | Vulpix |
| 5 | 10% | Vulpix | Gastly–Haunter |
| 6 | 10% | Growlithe | Oddish–Gloom |
| 7 | 5% | Sentret–Furret | Houndour–Houndoom |
| 8 | 5% | Ledyba–Ledian | Hoothoot–Noctowl |
| 9 | 4% | Eevee | Spinarak–Ariados |
| 10 | 4% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 11 | 1% | Vulpix | Misdreavus |
| 12 | 1% | Growlithe | Gastly–Haunter |

#### Route 38

Road, Johto west.

**`MAP_ROUTE38_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rattata–Raticate | Hoothoot–Noctowl |
| 2 | 20% | Snubbull–Granbull | Magnemite–Magneton |
| 3 | 10% | Magnemite–Magneton | Meowth–Persian |
| 4 | 10% | Pidgey–Pidgeot | Rattata–Raticate |
| 5 | 10% | Sentret–Furret | Gastly–Haunter |
| 6 | 10% | Meowth–Persian | Oddish–Gloom |
| 7 | 5% | Farfetch'd | Hoothoot–Noctowl |
| 8 | 5% | Sunkern | Gastly–Haunter |
| 9 | 4% | Magnemite–Magneton | Snubbull–Granbull |
| 10 | 4% | Pidgey–Pidgeot | Gastly–Haunter |
| 11 | 1% | Farfetch'd | Farfetch'd |
| 12 | 1% | Snubbull–Granbull | Venonat–Venomoth |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Wooper–Quagsire |
| 2 | 30% | Psyduck–Golduck | Marill–Azumarill |
| 3 | 5% | Wooper–Quagsire | Poliwag–Poliwhirl |
| 4 | 4% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 5 | 1% | Goldeen–Seaking | Goldeen–Seaking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Marill–Azumarill | Wooper–Quagsire |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Wooper–Quagsire | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Goldeen–Seaking |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Marill–Azumarill |
| 8 | 3% | 5% | 9% | Wooper–Quagsire | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Hoothoot–Noctowl | Hoothoot–Noctowl |
| 2 | 30% | Caterpie–Butterfree | Venonat–Venomoth |
| 3 | 5% | Exeggcute | Spinarak–Ariados |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Aipom | Exeggcute |

#### Route 39

Road, Johto west.

**`MAP_ROUTE39_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ponyta–Rapidash | Hoothoot–Noctowl |
| 2 | 20% | Snubbull–Granbull | Magnemite–Magneton |
| 3 | 10% | Magnemite–Magneton | Gastly–Haunter |
| 4 | 10% | Pidgey–Pidgeot | Meowth–Persian |
| 5 | 10% | Sentret–Furret | Rattata–Raticate |
| 6 | 10% | Meowth–Persian | Oddish–Gloom |
| 7 | 5% | Magnemite–Magneton | Hoothoot–Noctowl |
| 8 | 5% | Ponyta–Rapidash | Gastly–Haunter |
| 9 | 4% | Farfetch'd | Spinarak–Ariados |
| 10 | 4% | Sunkern | Ponyta–Rapidash |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Magnemite–Magneton | Venonat–Venomoth |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Wooper–Quagsire |
| 2 | 30% | Poliwag–Poliwhirl | Marill–Azumarill |
| 3 | 5% | Psyduck–Golduck | Psyduck–Golduck |
| 4 | 4% | Wooper–Quagsire | Poliwag–Poliwhirl |
| 5 | 1% | Goldeen–Seaking | Goldeen–Seaking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Wooper–Quagsire |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Wooper–Quagsire | Wooper–Quagsire |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Hoothoot–Noctowl | Hoothoot–Noctowl |
| 2 | 30% | Weedle–Beedrill | Weedle–Beedrill |
| 3 | 5% | Exeggcute | Spinarak–Ariados |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Ledyba–Ledian | Exeggcute |

#### Route 40

Road, Johto west.

**`MAP_ROUTE40_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Corsola | Corsola |
| 2 | 30% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 5% | Shellder | Staryu |
| 4 | 4% | Remoraid–Octillery | Chinchou–Lanturn |
| 5 | 1% | Qwilfish | Shellder |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Corsola | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Corsola | Corsola |
| 5 | 8% | 9% | 10% | Qwilfish | Staryu |
| 6 | 4% | 7% | 10% | Corsola | Shellder |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Qwilfish | Qwilfish |
| 10 | 2% | 4% | 9% | Remoraid–Octillery | Staryu |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Krabby–Kingler | Krabby–Kingler |
| 2 | 30% | Pidgey–Pidgeot | Murkrow |
| 3 | 5% | Geodude–Graveler | Geodude–Graveler |
| 4 | 4% | Spearow–Fearow | Spearow–Fearow |
| 5 | 1% | Ledyba–Ledian | Hoothoot–Noctowl |

#### Route 41

Road, Johto west.

**`MAP_ROUTE41_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Horsea–Seadra | Horsea–Seadra |
| 3 | 5% | Mantyke | Mantyke |
| 4 | 4% | Shellder | Chinchou–Lanturn |
| 5 | 1% | Qwilfish | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Shellder | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Qwilfish | Horsea–Seadra |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Staryu |
| 8 | 3% | 5% | 9% | Corsola | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Shellder | Shellder |
| 10 | 2% | 4% | 9% | Qwilfish | Staryu |

#### Route 42

Road, Johto east.

**`MAP_ROUTE42_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Marill–Azumarill |
| 2 | 20% | Mankey–Primeape | Mankey–Primeape |
| 3 | 10% | Ekans–Arbok | Zubat–Golbat |
| 4 | 10% | Rattata–Raticate | Hoothoot–Noctowl |
| 5 | 10% | Marill–Azumarill | Gastly–Haunter |
| 6 | 10% | Geodude–Graveler | Spinarak–Ariados |
| 7 | 5% | Machop–Machoke | Zubat–Golbat |
| 8 | 5% | Ekans–Arbok | Oddish–Gloom |
| 9 | 4% | Gligar | Gligar |
| 10 | 4% | Mankey–Primeape | Hoothoot–Noctowl |
| 11 | 1% | Tyrogue | Zubat–Golbat |
| 12 | 1% | Spearow–Fearow | Tyrogue |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Marill–Azumarill | Marill–Azumarill |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Psyduck–Golduck |
| 5 | 1% | Poliwag–Poliwhirl | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Marill–Azumarill |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Marill–Azumarill | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Poliwag–Poliwhirl |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Aipom |
| 2 | 30% | Aipom | Spearow–Fearow |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Exeggcute | Pineco–Forretress |
| 5 | 1% | Ledyba–Ledian | Hoothoot–Noctowl |

#### Route 43

Road, Johto east.

**`MAP_ROUTE43_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Mareep–Ampharos | Venonat–Venomoth |
| 2 | 20% | Sentret–Furret | Mareep–Ampharos |
| 3 | 10% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 4 | 10% | Hoppip–Jumpluff | Rattata–Raticate |
| 5 | 10% | Rattata–Raticate | Oddish–Gloom |
| 6 | 10% | Pidgey–Pidgeot | Gastly–Haunter |
| 7 | 5% | Farfetch'd | Venonat–Venomoth |
| 8 | 5% | Mareep–Ampharos | Hoothoot–Noctowl |
| 9 | 4% | Sentret–Furret | Spinarak–Ariados |
| 10 | 4% | Sunkern | Hoothoot–Noctowl |
| 11 | 1% | Farfetch'd | Farfetch'd |
| 12 | 1% | Hoppip–Jumpluff | Zubat–Golbat |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 4% | Psyduck–Golduck | Marill–Azumarill |
| 5 | 1% | Slowpoke–Slowbro | Slowpoke–Slowbro |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Marill–Azumarill |
| 8 | 3% | 5% | 9% | Slowpoke–Slowbro | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Exeggcute | Venonat–Venomoth |
| 2 | 30% | Venonat–Venomoth | Exeggcute |
| 3 | 5% | Pineco–Forretress | Hoothoot–Noctowl |
| 4 | 4% | Aipom | Pineco–Forretress |
| 5 | 1% | Ledyba–Ledian | Aipom |

#### Route 44

Road, Johto east.

**`MAP_ROUTE44_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Tangela | Poliwag–Poliwhirl |
| 2 | 20% | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| 3 | 10% | Lickitung | Oddish–Gloom |
| 4 | 10% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 5 | 10% | Hoppip–Jumpluff | Lickitung |
| 6 | 10% | Poliwag–Poliwhirl | Venonat–Venomoth |
| 7 | 5% | Sunkern | Oddish–Gloom |
| 8 | 5% | Tangela | Gastly–Haunter |
| 9 | 4% | Lickitung | Hoothoot–Noctowl |
| 10 | 4% | Bellsprout–Weepinbell | Gastly–Haunter |
| 11 | 1% | Elekid | Elekid |
| 12 | 1% | Farfetch'd | Tangela |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Psyduck–Golduck | Wooper–Quagsire |
| 3 | 5% | Marill–Azumarill | Psyduck–Golduck |
| 4 | 4% | Magikarp–Gyarados | Marill–Azumarill |
| 5 | 1% | Goldeen–Seaking | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 4 | 8% | 10% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Psyduck–Golduck | Goldeen–Seaking |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| 2 | 30% | Weedle–Beedrill | Weedle–Beedrill |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Exeggcute | Pineco–Forretress |
| 5 | 1% | Ledyba–Ledian | Hoothoot–Noctowl |

#### Route 45

Road, Johto east.

**`MAP_ROUTE45_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Geodude–Graveler | Teddiursa–Ursaring |
| 2 | 20% | Phanpy–Donphan | Geodude–Graveler |
| 3 | 10% | Teddiursa–Ursaring | Zubat–Golbat |
| 4 | 10% | Spearow–Fearow | Hoothoot–Noctowl |
| 5 | 10% | Mankey–Primeape | Gastly–Haunter |
| 6 | 10% | Machop–Machoke | Gligar |
| 7 | 5% | Gligar | Zubat–Golbat |
| 8 | 5% | Geodude–Graveler | Zubat–Golbat |
| 9 | 4% | Gligar | Gastly–Haunter |
| 10 | 4% | Phanpy–Donphan | Hoothoot–Noctowl |
| 11 | 1% | Dunsparce | Dunsparce |
| 12 | 1% | Teddiursa–Ursaring | Gligar |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Goldeen–Seaking | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Goldeen–Seaking |
| 5 | 1% | Psyduck–Golduck | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Poliwag–Poliwhirl |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Goldeen–Seaking |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Geodude–Graveler |
| 2 | 30% | Aipom | Aipom |
| 3 | 5% | Pineco–Forretress | Hoothoot–Noctowl |
| 4 | 4% | Spearow–Fearow | Pineco–Forretress |
| 5 | 1% | Exeggcute | Spinarak–Ariados |

#### Route 46

Road, Johto east.

**`MAP_ROUTE46_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Geodude–Graveler | Geodude–Graveler |
| 2 | 20% | Spearow–Fearow | Phanpy–Donphan |
| 3 | 10% | Phanpy–Donphan | Zubat–Golbat |
| 4 | 10% | Rattata–Raticate | Hoothoot–Noctowl |
| 5 | 10% | Hoppip–Jumpluff | Gastly–Haunter |
| 6 | 10% | Pidgey–Pidgeot | Oddish–Gloom |
| 7 | 5% | Machop–Machoke | Zubat–Golbat |
| 8 | 5% | Spearow–Fearow | Cyndaquil–Typhlosion |
| 9 | 4% | Cyndaquil–Typhlosion | Hoothoot–Noctowl |
| 10 | 4% | Rattata–Raticate | Gastly–Haunter |
| 11 | 1% | Gligar | Spinarak–Ariados |
| 12 | 1% | Phanpy–Donphan | Oddish–Gloom |

#### Route 48

Road, Johto west.

**`MAP_ROUTE48_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Oddish–Gloom |
| 2 | 20% | Sunkern | Vulpix |
| 3 | 10% | Bellsprout–Weepinbell | Hoothoot–Noctowl |
| 4 | 10% | Vulpix | Gastly–Haunter |
| 5 | 10% | Hoppip–Jumpluff | Houndour–Houndoom |
| 6 | 10% | Growlithe | Venonat–Venomoth |
| 7 | 5% | Farfetch'd | Oddish–Gloom |
| 8 | 5% | Pidgey–Pidgeot | Farfetch'd |
| 9 | 4% | Farfetch'd | Yanma |
| 10 | 4% | Nidoran♀–Nidorina | Hoothoot–Noctowl |
| 11 | 1% | Yanma | Houndour–Houndoom |
| 12 | 1% | Sunkern | Spinarak–Ariados |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Qwilfish | Chinchou–Lanturn |
| 2 | 30% | Tentacool–Tentacruel | Qwilfish |
| 3 | 5% | Corsola | Corsola |
| 4 | 4% | Shellder | Staryu |
| 5 | 1% | Remoraid–Octillery | Tentacool–Tentacruel |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Qwilfish | Qwilfish |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Corsola | Corsola |
| 5 | 8% | 9% | 10% | Remoraid–Octillery | Remoraid–Octillery |
| 6 | 4% | 7% | 10% | Shellder | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Qwilfish | Staryu |
| 8 | 3% | 5% | 9% | Corsola | Qwilfish |
| 9 | 2% | 4% | 9% | Horsea–Seadra | Corsola |
| 10 | 2% | 4% | 9% | Remoraid–Octillery | Krabby–Kingler |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Spinarak–Ariados |
| 2 | 30% | Weedle–Beedrill | Spearow–Fearow |
| 3 | 5% | Aipom | Hoothoot–Noctowl |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Ledyba–Ledian | Yanma |

#### Ilex Forest

Road, Johto west.

**`MAP_ILEX_FOREST_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Caterpie–Metapod | Oddish–Gloom |
| 2 | 20% | Weedle–Kakuna | Venonat–Venomoth |
| 3 | 10% | Ledyba–Ledian | Spinarak–Ariados |
| 4 | 10% | Paras–Parasect | Hoothoot–Noctowl |
| 5 | 10% | Oddish–Gloom | Paras–Parasect |
| 6 | 10% | Sunkern | Wooper–Quagsire |
| 7 | 5% | Yanma | Spinarak–Ariados |
| 8 | 5% | Chikorita–Meganium | Hoothoot–Noctowl |
| 9 | 4% | Pichu | Pichu |
| 10 | 4% | Ledyba–Ledian | Chikorita–Meganium |
| 11 | 1% | Chikorita–Meganium | Misdreavus |
| 12 | 1% | Pichu | Weedle–Kakuna |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 4% | Wooper–Quagsire | Wooper–Quagsire |
| 5 | 1% | Magikarp–Gyarados | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Marill–Azumarill |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Slowpoke–Slowbro | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Slowpoke–Slowbro |
| 9 | 2% | 4% | 9% | Wooper–Quagsire | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Poliwag–Poliwhirl |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Weedle–Beedrill | Weedle–Beedrill |
| 2 | 30% | Caterpie–Butterfree | Caterpie–Butterfree |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Exeggcute | Pineco–Forretress |
| 5 | 1% | Aipom | Hoothoot–Noctowl |

#### Route 27

Wilds, Border.

**`MAP_ROUTE27_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ponyta–Rapidash | Rattata–Raticate |
| 2 | 20% | Rattata–Raticate | Ekans–Arbok |
| 3 | 10% | Ekans–Arbok | Hoothoot–Noctowl |
| 4 | 10% | Doduo–Dodrio | Zubat–Golbat |
| 5 | 10% | Sandshrew–Sandslash | Gastly–Haunter |
| 6 | 10% | Spearow–Fearow | Venonat–Venomoth |
| 7 | 5% | Miltank | Murkrow |
| 8 | 5% | Doduo–Dodrio | Oddish–Gloom |
| 9 | 4% | Girafarig | Houndour–Houndoom |
| 10 | 4% | Ponyta–Rapidash | Hoothoot–Noctowl |
| 11 | 1% | Chikorita–Meganium | Chikorita–Meganium |
| 12 | 1% | Tauros | Murkrow |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Shellder | Shellder |
| 3 | 5% | Totodile–Feraligatr | Chinchou–Lanturn |
| 4 | 4% | Shellder | Totodile–Feraligatr |
| 5 | 1% | Mantine | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Shellder | Shellder |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Shellder | Staryu |
| 6 | 4% | 7% | 10% | Corsola | Krabby–Kingler |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Slowpoke–Slowbro | Staryu |
| 9 | 2% | 4% | 9% | Krabby–Kingler | Qwilfish |
| 10 | 2% | 4% | 9% | Horsea–Seadra | Horsea–Seadra |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ekans–Arbok | Ekans–Arbok |
| 2 | 30% | Pineco–Forretress | Pineco–Forretress |
| 3 | 5% | Heracross | Murkrow |
| 4 | 4% | Exeggcute | Heracross |
| 5 | 1% | Aipom | Hoothoot–Noctowl |

#### Route 47

Wilds, Johto west.

**`MAP_ROUTE47_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Oddish–Gloom |
| 2 | 20% | Hoppip–Jumpluff | Venonat–Venomoth |
| 3 | 10% | Bellsprout–Weepinbell | Hoothoot–Noctowl |
| 4 | 10% | Rattata–Raticate | Gastly–Haunter |
| 5 | 10% | Snubbull–Granbull | Rattata–Raticate |
| 6 | 10% | Pidgey–Pidgeot | Spinarak–Ariados |
| 7 | 5% | Miltank | Miltank |
| 8 | 5% | Farfetch'd | Ditto |
| 9 | 4% | Girafarig | Farfetch'd |
| 10 | 4% | Ditto | Hoothoot–Noctowl |
| 11 | 1% | Tauros | Happiny |
| 12 | 1% | Skarmory | Skarmory |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Corsola | Corsola |
| 2 | 30% | Shellder | Staryu |
| 3 | 5% | Horsea–Seadra | Shellder |
| 4 | 4% | Slowpoke–Slowbro | Chinchou–Lanturn |
| 5 | 1% | Mantine | Mantine |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Shellder | Shellder |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Krabby–Kingler |
| 3 | 10% | 12% | 11% | Corsola | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Chinchou–Lanturn | Corsola |
| 6 | 4% | 7% | 10% | Corsola | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Staryu |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 9 | 2% | 4% | 9% | Remoraid–Octillery | Chinchou–Lanturn |
| 10 | 2% | 4% | 9% | Corsola | Qwilfish |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Krabby–Kingler | Krabby–Kingler |
| 2 | 30% | Spearow–Fearow | Spearow–Fearow |
| 3 | 5% | Shuckle | Hoothoot–Noctowl |
| 4 | 4% | Heracross | Shuckle |
| 5 | 1% | Geodude–Graveler | Heracross |

#### Dark Cave

Wilds, Johto east.

**`MAP_DARK_CAVE_SOUTH_SIDE_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Geodude–Graveler | Zubat–Golbat |
| 2 | 20% | Zubat–Golbat | Geodude–Graveler |
| 3 | 10% | Teddiursa–Ursaring | Teddiursa–Ursaring |
| 4 | 10% | Machop–Machoke | Machop–Machoke |
| 5 | 10% | Onix | Onix |
| 6 | 10% | Zubat–Golbat | Gastly–Haunter |
| 7 | 5% | Dunsparce | Dunsparce |
| 8 | 5% | Geodude–Graveler | Wobbuffet |
| 9 | 4% | Wobbuffet | Wobbuffet |
| 10 | 4% | Dunsparce | Dunsparce |
| 11 | 1% | Wobbuffet | Gastly–Haunter |
| 12 | 1% | Teddiursa–Ursaring | Teddiursa–Ursaring |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Zubat–Golbat | Zubat–Golbat |
| 3 | 5% | Magikarp–Gyarados | Wooper–Quagsire |
| 4 | 4% | Poliwag–Poliwhirl | Magikarp–Gyarados |
| 5 | 1% | Marill–Azumarill | Poliwag–Poliwhirl |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Goldeen–Seaking | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Poliwag–Poliwhirl | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Dunsparce |
| 2 | 30% | Dunsparce | Geodude–Graveler |
| 3 | 5% | Geodude | Geodude |
| 4 | 4% | Shuckle | Shuckle |
| 5 | 1% | Geodude–Graveler | Geodude–Graveler |

**`MAP_DARK_CAVE_NORTH_SIDE_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Teddiursa–Ursaring | Teddiursa–Ursaring |
| 2 | 20% | Onix | Onix |
| 3 | 10% | Geodude–Graveler | Zubat–Golbat |
| 4 | 10% | Zubat–Golbat | Machop–Machoke |
| 5 | 10% | Machop–Machoke | Geodude–Graveler |
| 6 | 10% | Geodude–Graveler | Gastly–Haunter |
| 7 | 5% | Wobbuffet | Wobbuffet |
| 8 | 5% | Zubat–Golbat | Gastly–Haunter |
| 9 | 4% | Wobbuffet | Dunsparce |
| 10 | 4% | Geodude–Graveler | Zubat–Golbat |
| 11 | 1% | Gligar | Gligar |
| 12 | 1% | Dunsparce | Dunsparce |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 30% | Goldeen–Seaking | Zubat–Golbat |
| 3 | 5% | Zubat–Golbat | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Goldeen–Seaking |
| 5 | 1% | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Poliwag–Poliwhirl |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Goldeen–Seaking |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Geodude–Graveler |
| 2 | 30% | Geodude | Dunsparce |
| 3 | 5% | Dunsparce | Geodude |
| 4 | 4% | Geodude–Graveler | Geodude–Graveler |
| 5 | 1% | Shuckle | Shuckle |

#### Cliff Edge Cave

Wilds, Johto west.

**`MAP_CLIFF_EDGE_GATE_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Krabby–Kingler | Krabby–Kingler |
| 2 | 20% | Wobbuffet | Wobbuffet |
| 3 | 10% | Geodude–Graveler | Geodude–Graveler |
| 4 | 10% | Dunsparce | Misdreavus |
| 5 | 10% | Zubat–Golbat | Zubat–Golbat |
| 6 | 10% | Slowpoke–Slowbro | Dunsparce |
| 7 | 5% | Shuckle | Shuckle |
| 8 | 5% | Onix | Slowpoke–Slowbro |
| 9 | 4% | Dunsparce | Misdreavus |
| 10 | 4% | Wobbuffet | Onix |
| 11 | 1% | Shuckle | Seel–Dewgong |
| 12 | 1% | Seel–Dewgong | Shuckle |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 30% | Shellder | Chinchou–Lanturn |
| 3 | 5% | Corsola | Shellder |
| 4 | 4% | Tentacool–Tentacruel | Staryu |
| 5 | 1% | Mantine | Mantine |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 22% | 18% | 10% | Shellder | Shellder |
| 3 | 10% | 12% | 11% | Krabby–Kingler | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Corsola | Krabby–Kingler |
| 5 | 8% | 9% | 10% | Tentacool–Tentacruel | Corsola |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Staryu |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Tentacool–Tentacruel |
| 8 | 3% | 5% | 9% | Qwilfish | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Shellder | Qwilfish |
| 10 | 2% | 4% | 9% | Corsola | Horsea–Seadra |

**`MAP_CLIFF_EDGE_CAVE_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Krabby–Kingler | Geodude–Graveler |
| 2 | 20% | Geodude–Graveler | Krabby–Kingler |
| 3 | 10% | Dunsparce | Misdreavus |
| 4 | 10% | Wobbuffet | Wobbuffet |
| 5 | 10% | Zubat–Golbat | Zubat–Golbat |
| 6 | 10% | Onix | Dunsparce |
| 7 | 5% | Shuckle | Shuckle |
| 8 | 5% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 9 | 4% | Dunsparce | Onix |
| 10 | 4% | Seel–Dewgong | Misdreavus |
| 11 | 1% | Wobbuffet | Seel–Dewgong |
| 12 | 1% | Shuckle | Shuckle |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Krabby–Kingler | Geodude–Graveler |
| 2 | 30% | Geodude–Graveler | Krabby–Kingler |
| 3 | 5% | Shuckle | Shuckle |
| 4 | 4% | Krabby–Kingler | Onix |
| 5 | 1% | Shuckle | Shuckle |

#### Tohjo Falls

Wilds, Border.

**`MAP_TOHJO_FALLS_CAVERN_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Slowpoke–Slowbro | Zubat–Golbat |
| 2 | 20% | Zubat–Golbat | Slowpoke–Slowbro |
| 3 | 10% | Rattata–Raticate | Rattata–Raticate |
| 4 | 10% | Geodude–Graveler | Geodude–Graveler |
| 5 | 10% | Onix | Gastly–Haunter |
| 6 | 10% | Marill–Azumarill | Marill–Azumarill |
| 7 | 5% | Psyduck–Golduck | Psyduck–Golduck |
| 8 | 5% | Slowpoke–Slowbro | Onix |
| 9 | 4% | Dunsparce | Dunsparce |
| 10 | 4% | Rattata–Raticate | Zubat–Golbat |
| 11 | 1% | Dunsparce | Gastly–Haunter |
| 12 | 1% | Onix | Slowpoke–Slowbro |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Psyduck–Golduck |
| 5 | 1% | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Marill–Azumarill | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Zubat–Golbat |

#### Ruins of Alph

Wilds, Johto east.

**`MAP_RUINS_OF_ALPH_OUTSIDE_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Wooper–Quagsire |
| 2 | 20% | Smeargle | Natu–Xatu |
| 3 | 10% | Mareep–Ampharos | Gastly–Haunter |
| 4 | 10% | Hoppip–Jumpluff | Hoothoot–Noctowl |
| 5 | 10% | Pidgey–Pidgeot | Spinarak–Ariados |
| 6 | 10% | Sunkern | Oddish–Gloom |
| 7 | 5% | Sentret–Furret | Misdreavus |
| 8 | 5% | Natu–Xatu | Smeargle |
| 9 | 4% | Smeargle | Gastly–Haunter |
| 10 | 4% | Natu | Wooper–Quagsire |
| 11 | 1% | Abra | Abra |
| 12 | 1% | Girafarig | Girafarig |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 30% | Wooper–Quagsire | Wooper–Quagsire |
| 3 | 5% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 4 | 4% | Marill–Azumarill | Poliwag–Poliwhirl |
| 5 | 1% | Magikarp–Gyarados | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Psyduck–Golduck |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Marill–Azumarill |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Wooper–Quagsire | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Poliwag–Poliwhirl |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Natu–Xatu | Natu–Xatu |
| 2 | 30% | Natu–Xatu | Natu–Xatu |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Aipom | Pineco–Forretress |
| 5 | 1% | Heracross | Heracross |

**`MAP_RUINS_OF_ALPH_B1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Unown | Unown |
| 2 | 20% | Unown | Unown |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Unown | Unown |
| 5 | 10% | Unown | Unown |
| 6 | 10% | Unown | Unown |
| 7 | 5% | Unown | Misdreavus |
| 8 | 5% | Unown | Gastly–Haunter |
| 9 | 4% | Natu–Xatu | Natu–Xatu |
| 10 | 4% | Natu–Xatu | Unown |
| 11 | 1% | Smeargle | Smeargle |
| 12 | 1% | Unown | Unown |

#### National Park

Wilds, Johto west.

**`MAP_NATIONAL_PARK_NORMAL_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sunkern | Spinarak–Ariados |
| 2 | 20% | Nidoran♀–Nidorina | Nidoran♂–Nidorino |
| 3 | 10% | Nidoran♂–Nidorino | Hoothoot–Noctowl |
| 4 | 10% | Caterpie–Butterfree | Venonat–Venomoth |
| 5 | 10% | Weedle–Beedrill | Psyduck–Golduck |
| 6 | 10% | Snubbull–Granbull | Murkrow |
| 7 | 5% | Stantler | Stantler |
| 8 | 5% | Scyther | Scyther |
| 9 | 4% | Girafarig | Gastly–Haunter |
| 10 | 4% | Togepi | Togepi |
| 11 | 1% | Scyther | Scyther |
| 12 | 1% | Pinsir | Misdreavus |

#### Lake of Rage

Wilds, Johto east.

**`MAP_LAKE_OF_RAGE_HNS`**

Water type: ponds and rivers.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 30% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 5% | Totodile–Feraligatr | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Totodile–Feraligatr |
| 5 | 1% | Wooper–Quagsire | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Wooper–Quagsire | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Totodile–Feraligatr | Totodile–Feraligatr |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Exeggcute | Venonat–Venomoth |
| 2 | 30% | Pineco–Forretress | Pineco–Forretress |
| 3 | 5% | Heracross | Hoothoot–Noctowl |
| 4 | 4% | Aipom | Heracross |
| 5 | 1% | Venonat–Venomoth | Yanma |

#### Route 26

Outlands, Border.

**`MAP_ROUTE26_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Doduo–Dodrio | Rattata–Raticate |
| 2 | 20% | Ekans–Arbok | Ekans–Arbok |
| 3 | 10% | Ponyta–Rapidash | Houndour–Houndoom |
| 4 | 10% | Rattata–Raticate | Hoothoot–Noctowl |
| 5 | 10% | Sandshrew–Sandslash | Zubat–Golbat |
| 6 | 10% | Spearow–Fearow | Sandshrew–Sandslash |
| 7 | 5% | Tauros | Murkrow–Honchkrow |
| 8 | 5% | Phanpy–Donphan | Tauros |
| 9 | 4% | Miltank | Venonat–Venomoth |
| 10 | 4% | Mankey–Primeape | Houndour–Houndoom |
| 11 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 12 | 1% | Teddiursa–Ursaring | Gastly–Gengar |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Shellder–Cloyster | Shellder–Cloyster |
| 3 | 5% | Horsea–Seadra | Chinchou–Lanturn |
| 4 | 4% | Lapras | Staryu–Starmie |
| 5 | 1% | Mantine | Lapras |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Shellder–Cloyster | Shellder–Cloyster |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Remoraid–Octillery | Staryu–Starmie |
| 6 | 4% | 7% | 10% | Corsola | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Remoraid–Octillery | Staryu–Starmie |
| 8 | 3% | 5% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 9 | 2% | 4% | 9% | Shellder–Cloyster | Qwilfish |
| 10 | 2% | 4% | 9% | Slowpoke–Slowbro | Remoraid–Octillery |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Aipom–Ambipom | Aipom–Ambipom |
| 2 | 30% | Pineco–Forretress | Pineco–Forretress |
| 3 | 5% | Heracross | Hoothoot–Noctowl |
| 4 | 4% | Geodude–Golem | Heracross |
| 5 | 1% | Scyther–Scizor | Murkrow–Honchkrow |

#### Route 28

Outlands, Border.

**`MAP_ROUTE28_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ponyta–Rapidash | Teddiursa–Ursaring |
| 2 | 20% | Tangela–Tangrowth | Poliwag–Poliwrath |
| 3 | 10% | Doduo–Dodrio | Zubat–Crobat |
| 4 | 10% | Teddiursa–Ursaring | Houndour–Houndoom |
| 5 | 10% | Phanpy–Donphan | Tangela–Tangrowth |
| 6 | 10% | Ekans–Arbok | Murkrow–Honchkrow |
| 7 | 5% | Skarmory | Skarmory |
| 8 | 5% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 9 | 4% | Tauros | Houndour–Houndoom |
| 10 | 4% | Girafarig–Farigiraf | Sneasel–Weavile |
| 11 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 12 | 1% | Snorlax | Poliwag–Politoed |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 2 | 30% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 5% | Goldeen–Seaking | Wooper–Quagsire |
| 4 | 4% | Poliwag–Politoed | Poliwag–Politoed |
| 5 | 1% | Slowpoke–Slowbro | Slowpoke–Slowking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Goldeen–Seaking | Goldeen–Seaking |
| 4 | 8% | 10% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwrath | Psyduck–Golduck |
| 6 | 4% | 7% | 10% | Slowpoke–Slowbro | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Poliwag–Politoed | Poliwag–Politoed |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Slowpoke–Slowking |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Poliwag–Poliwrath |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pineco–Forretress | Teddiursa–Ursaring |
| 2 | 30% | Teddiursa–Ursaring | Pineco–Forretress |
| 3 | 5% | Heracross | Murkrow–Honchkrow |
| 4 | 4% | Scyther–Scizor | Heracross |
| 5 | 1% | Aipom–Ambipom | Scyther–Scizor |

#### Faraway Island

Outlands, Border.

**`MAP_FARAWAY_ISLAND_ENTRANCE_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Tangela | Venonat–Venomoth |
| 2 | 20% | Exeggcute–Exeggutor | Tangela |
| 3 | 10% | Bellsprout–Victreebel | Hoothoot–Noctowl |
| 4 | 10% | Oddish–Bellossom | Spinarak–Ariados |
| 5 | 10% | Pineco–Forretress | Exeggcute–Exeggutor |
| 6 | 10% | Hoppip–Jumpluff | Oddish–Vileplume |
| 7 | 5% | Ditto | Murkrow |
| 8 | 5% | Smeargle | Ditto |
| 9 | 4% | Scyther | Misdreavus |
| 10 | 4% | Pinsir | Houndour–Houndoom |
| 11 | 1% | Aerodactyl | Aerodactyl |
| 12 | 1% | Cleffa | Cleffa |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Staryu–Starmie | Chinchou–Lanturn |
| 2 | 30% | Tentacool–Tentacruel | Staryu–Starmie |
| 3 | 5% | Corsola | Tentacool–Tentacruel |
| 4 | 4% | Mantine | Qwilfish |
| 5 | 1% | Lapras | Lapras |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Qwilfish | Chinchou–Lanturn |
| 3 | 10% | 12% | 11% | Corsola | Qwilfish |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Horsea–Seadra | Horsea–Seadra |
| 6 | 4% | 7% | 10% | Staryu–Starmie | Staryu–Starmie |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Shellder–Cloyster | Shellder–Cloyster |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 10 | 2% | 4% | 9% | Dratini–Dragonair | Dratini–Dragonair |

#### Southern Island

Outlands, Border.

**`MAP_SOUTHERN_ISLAND_EXTERIOR_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 30% | Staryu–Starmie | Chinchou–Lanturn |
| 3 | 5% | Horsea–Seadra | Staryu–Starmie |
| 4 | 4% | Dratini–Dragonair | Horsea–Kingdra |
| 5 | 1% | Lapras | Lapras |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corsola | Corsola |
| 2 | 22% | 18% | 10% | Slowpoke–Slowbro | Chinchou–Lanturn |
| 3 | 10% | 12% | 11% | Staryu–Starmie | Staryu–Starmie |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Horsea–Seadra |
| 5 | 8% | 9% | 10% | Mantine | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 7 | 3% | 6% | 10% | Horsea–Kingdra | Horsea–Kingdra |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 9 | 2% | 4% | 9% | Dratini–Dragonair | Dratini–Dragonair |
| 10 | 2% | 4% | 9% | Lapras | Lapras |

#### Union Cave

Dungeon, Johto east.

**`MAP_UNION_CAVE_1F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Geodude–Graveler | Zubat–Golbat |
| 2 | 20% | Zubat–Golbat | Geodude–Graveler |
| 3 | 10% | Onix | Onix |
| 4 | 10% | Sandshrew–Sandslash | Wooper–Quagsire |
| 5 | 10% | Rattata–Raticate | Rattata–Raticate |
| 6 | 10% | Wooper–Quagsire | Sandshrew–Sandslash |
| 7 | 5% | Cubone–Marowak | Gastly–Haunter |
| 8 | 5% | Geodude–Graveler | Cubone–Marowak |
| 9 | 4% | Dunsparce | Dunsparce |
| 10 | 4% | Onix | Wooper–Quagsire |
| 11 | 1% | Dunsparce | Onix |
| 12 | 1% | Cubone–Marowak | Dunsparce |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 30% | Wooper–Quagsire | Wooper–Quagsire |
| 3 | 5% | Goldeen–Seaking | Zubat–Golbat |
| 4 | 4% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 1% | Psyduck–Golduck | Poliwag–Poliwhirl |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Zubat–Golbat |
| 5 | 8% | 9% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Goldeen–Seaking |
| 7 | 3% | 6% | 10% | Wooper–Quagsire | Zubat–Golbat |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Wooper–Quagsire |

**`MAP_UNION_CAVE_B1F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Onix | Onix |
| 2 | 20% | Marill–Azumarill | Marill–Azumarill |
| 3 | 10% | Geodude–Graveler | Zubat–Golbat |
| 4 | 10% | Zubat–Golbat | Geodude–Graveler |
| 5 | 10% | Sandshrew–Sandslash | Wooper–Quagsire |
| 6 | 10% | Rattata–Raticate | Rattata–Raticate |
| 7 | 5% | Wooper–Quagsire | Gastly–Haunter |
| 8 | 5% | Cubone–Marowak | Cubone–Marowak |
| 9 | 4% | Dunsparce | Dunsparce |
| 10 | 4% | Machop–Machoke | Gastly–Haunter |
| 11 | 1% | Onix–Steelix | Onix–Steelix |
| 12 | 1% | Dunsparce | Sandshrew–Sandslash |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Wooper–Quagsire | Wooper–Quagsire |
| 3 | 5% | Goldeen–Seaking | Zubat–Golbat |
| 4 | 4% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 1% | Psyduck–Golduck | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Marill–Azumarill | Marill–Azumarill |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Zubat–Golbat |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Zubat–Golbat |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Marill–Azumarill |

**`MAP_UNION_CAVE_B2F_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 20% | Onix | Onix |
| 3 | 10% | Zubat–Golbat | Zubat–Golbat |
| 4 | 10% | Geodude–Graveler | Geodude–Graveler |
| 5 | 10% | Rattata–Raticate | Gastly–Haunter |
| 6 | 10% | Cubone–Marowak | Cubone–Marowak |
| 7 | 5% | Marill–Azumarill | Marill–Azumarill |
| 8 | 5% | Dunsparce | Dunsparce |
| 9 | 4% | Onix–Steelix | Onix–Steelix |
| 10 | 4% | Machop–Machoke | Gastly–Haunter |
| 11 | 1% | Wobbuffet | Wobbuffet |
| 12 | 1% | Dunsparce | Zubat–Crobat |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Shellder | Shellder |
| 2 | 30% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 3 | 5% | Lapras | Lapras |
| 4 | 4% | Chinchou–Lanturn | Tentacool–Tentacruel |
| 5 | 1% | Slowpoke–Slowbro | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Shellder | Shellder |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Chinchou–Lanturn | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Shellder | Staryu |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Chinchou–Lanturn | Staryu |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Remoraid–Octillery | Chinchou–Lanturn |
| 10 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |

#### Ice Path

Dungeon, Johto east.

**`MAP_ICE_PATH_1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Swinub–Piloswine | Sneasel |
| 2 | 20% | Delibird | Delibird |
| 3 | 10% | Zubat–Golbat | Zubat–Golbat |
| 4 | 10% | Geodude–Graveler | Swinub–Piloswine |
| 5 | 10% | Onix | Geodude–Graveler |
| 6 | 10% | Swinub–Piloswine | Sneasel |
| 7 | 5% | Sneasel | Onix |
| 8 | 5% | Zubat–Golbat | Zubat–Golbat |
| 9 | 4% | Smoochum | Smoochum |
| 10 | 4% | Delibird | Delibird |
| 11 | 1% | Jynx | Jynx |
| 12 | 1% | Sneasel | Gastly–Haunter |

**`MAP_ICE_PATH_B1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Delibird | Sneasel |
| 2 | 20% | Swinub–Piloswine | Delibird |
| 3 | 10% | Zubat–Golbat | Swinub–Piloswine |
| 4 | 10% | Sneasel | Zubat–Golbat |
| 5 | 10% | Swinub–Piloswine | Sneasel |
| 6 | 10% | Onix | Swinub–Piloswine |
| 7 | 5% | Jynx | Jynx |
| 8 | 5% | Zubat–Golbat | Smoochum |
| 9 | 4% | Smoochum | Zubat–Golbat |
| 10 | 4% | Delibird | Onix |
| 11 | 1% | Swinub–Mamoswine | Sneasel–Weavile |
| 12 | 1% | Jynx | Jynx |

**`MAP_ICE_PATH_B2F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Swinub–Piloswine | Sneasel |
| 2 | 20% | Delibird | Swinub–Piloswine |
| 3 | 10% | Delibird | Delibird |
| 4 | 10% | Sneasel | Sneasel |
| 5 | 10% | Zubat–Golbat | Swinub–Piloswine |
| 6 | 10% | Geodude–Graveler | Zubat–Golbat |
| 7 | 5% | Jynx | Jynx |
| 8 | 5% | Smoochum | Sneasel–Weavile |
| 9 | 4% | Swinub–Mamoswine | Smoochum |
| 10 | 4% | Jynx | Swinub–Mamoswine |
| 11 | 1% | Sneasel–Weavile | Zubat–Crobat |
| 12 | 1% | Onix–Steelix | Jynx |

**`MAP_ICE_PATH_B3F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Swinub–Piloswine | Sneasel |
| 2 | 20% | Delibird | Delibird |
| 3 | 10% | Jynx | Jynx |
| 4 | 10% | Sneasel | Sneasel–Weavile |
| 5 | 10% | Delibird | Swinub–Piloswine |
| 6 | 10% | Swinub–Mamoswine | Delibird |
| 7 | 5% | Zubat–Golbat | Sneasel |
| 8 | 5% | Smoochum | Smoochum |
| 9 | 4% | Jynx | Swinub–Mamoswine |
| 10 | 4% | Sneasel–Weavile | Jynx |
| 11 | 1% | Onix–Steelix | Zubat–Crobat |
| 12 | 1% | Zubat–Crobat | Sneasel–Weavile |

**`MAP_ICE_PATH_B4F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Delibird | Sneasel |
| 2 | 20% | Swinub–Piloswine | Swinub–Piloswine |
| 3 | 10% | Jynx | Sneasel–Weavile |
| 4 | 10% | Swinub–Mamoswine | Jynx |
| 5 | 10% | Swinub–Piloswine | Delibird |
| 6 | 10% | Sneasel | Swinub–Mamoswine |
| 7 | 5% | Sneasel–Weavile | Sneasel |
| 8 | 5% | Jynx | Jynx |
| 9 | 4% | Smoochum | Smoochum |
| 10 | 4% | Onix–Steelix | Sneasel–Weavile |
| 11 | 1% | Zubat–Crobat | Zubat–Crobat |
| 12 | 1% | Jynx | Delibird |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Geodude–Golem |
| 2 | 30% | Geodude–Golem | Geodude–Graveler |
| 3 | 5% | Shuckle | Shuckle |
| 4 | 4% | Geodude–Graveler | Dunsparce |
| 5 | 1% | Dunsparce | Geodude–Graveler |

#### Sprout Tower

Dungeon, Johto east.

**`MAP_SPROUT_TOWER_2F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rattata–Raticate | Gastly–Haunter |
| 2 | 20% | Bellsprout–Weepinbell | Rattata–Raticate |
| 3 | 10% | Gastly–Haunter | Bellsprout–Weepinbell |
| 4 | 10% | Rattata–Raticate | Gastly–Haunter |
| 5 | 10% | Bellsprout–Weepinbell | Rattata–Raticate |
| 6 | 10% | Zubat–Golbat | Zubat–Golbat |
| 7 | 5% | Gastly–Haunter | Misdreavus |
| 8 | 5% | Natu–Xatu | Gastly–Haunter |
| 9 | 4% | Abra | Hoothoot–Noctowl |
| 10 | 4% | Misdreavus | Misdreavus |
| 11 | 1% | Abra | Abra |
| 12 | 1% | Hoothoot–Noctowl | Bellsprout–Weepinbell |

**`MAP_SPROUT_TOWER_3F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bellsprout–Weepinbell | Gastly–Haunter |
| 2 | 20% | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| 3 | 10% | Gastly–Haunter | Misdreavus |
| 4 | 10% | Zubat–Golbat | Rattata–Raticate |
| 5 | 10% | Natu–Xatu | Zubat–Golbat |
| 6 | 10% | Rattata–Raticate | Gastly–Haunter |
| 7 | 5% | Misdreavus | Misdreavus |
| 8 | 5% | Gastly–Haunter | Hoothoot–Noctowl |
| 9 | 4% | Abra | Abra |
| 10 | 4% | Natu–Xatu | Gastly–Gengar |
| 11 | 1% | Bellsprout–Victreebel | Bellsprout–Victreebel |
| 12 | 1% | Misdreavus | Misdreavus |

#### Slowpoke Well

Dungeon, Johto west.

**`MAP_SLOWPOKE_WELL_B1F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 10% | Wooper–Quagsire | Zubat–Golbat |
| 4 | 10% | Marill–Azumarill | Wooper–Quagsire |
| 5 | 10% | Zubat–Golbat | Gastly–Haunter |
| 6 | 10% | Wooper–Quagsire | Marill–Azumarill |
| 7 | 5% | Koffing–Weezing | Misdreavus |
| 8 | 5% | Marill–Azumarill | Gastly–Haunter |
| 9 | 4% | Zubat–Golbat | Wooper–Quagsire |
| 10 | 4% | Rattata–Raticate | Koffing–Weezing |
| 11 | 1% | Wobbuffet | Wobbuffet |
| 12 | 1% | Wooper–Quagsire | Misdreavus |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 30% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 5% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 4% | Psyduck–Golduck | Zubat–Golbat |
| 5 | 1% | Wooper–Quagsire | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Wooper–Quagsire | Marill–Azumarill |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Zubat–Golbat |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Poliwag–Poliwhirl |

**`MAP_SLOWPOKE_WELL_B2F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 10% | Wooper–Quagsire | Zubat–Golbat |
| 4 | 10% | Marill–Azumarill | Wooper–Quagsire |
| 5 | 10% | Zubat–Golbat | Misdreavus |
| 6 | 10% | Slowpoke–Slowking | Gastly–Haunter |
| 7 | 5% | Wobbuffet | Misdreavus |
| 8 | 5% | Wooper–Quagsire | Slowpoke–Slowking |
| 9 | 4% | Zubat–Crobat | Wobbuffet |
| 10 | 4% | Marill–Azumarill | Zubat–Crobat |
| 11 | 1% | Slowpoke–Slowking | Gastly–Haunter |
| 12 | 1% | Misdreavus | Slowpoke–Slowking |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 30% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 5% | Slowpoke–Slowking | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Slowpoke–Slowking |
| 5 | 1% | Wooper–Quagsire | Zubat–Golbat |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Slowpoke–Slowking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Slowpoke–Slowking |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Wooper–Quagsire | Zubat–Golbat |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Psyduck–Golduck |

#### Burned Tower

Dungeon, Johto west.

**`MAP_BURNED_TOWER_1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rattata–Raticate | Houndour–Houndoom |
| 2 | 20% | Houndour–Houndoom | Rattata–Raticate |
| 3 | 10% | Koffing–Weezing | Gastly–Haunter |
| 4 | 10% | Zubat–Golbat | Zubat–Golbat |
| 5 | 10% | Houndour–Houndoom | Koffing–Weezing |
| 6 | 10% | Rattata–Raticate | Houndour–Houndoom |
| 7 | 5% | Magby | Misdreavus |
| 8 | 5% | Koffing–Weezing | Magby |
| 9 | 4% | Slugma–Magcargo | Gastly–Haunter |
| 10 | 4% | Misdreavus | Slugma–Magcargo |
| 11 | 1% | Magby | Magby |
| 12 | 1% | Vulpix | Vulpix |

**`MAP_BURNED_TOWER_B1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gastly–Haunter | Gastly–Haunter |
| 2 | 20% | Houndour–Houndoom | Houndour–Houndoom |
| 3 | 10% | Koffing–Weezing | Misdreavus |
| 4 | 10% | Misdreavus | Murkrow |
| 5 | 10% | Zubat–Golbat | Rattata–Raticate |
| 6 | 10% | Rattata–Raticate | Koffing–Weezing |
| 7 | 5% | Magmar | Misdreavus |
| 8 | 5% | Cyndaquil–Typhlosion | Magmar |
| 9 | 4% | Misdreavus | Gastly–Gengar |
| 10 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 11 | 1% | Magmar–Magmortar | Misdreavus–Mismagius |
| 12 | 1% | Cyndaquil–Typhlosion | Cyndaquil–Typhlosion |

#### Rocket Hideout

Dungeon, Johto east.

**`MAP_ROCKET_HIDEOUT_B1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Koffing–Weezing | Koffing–Weezing |
| 2 | 20% | Voltorb–Electrode | Voltorb–Electrode |
| 3 | 10% | Rattata–Raticate | Rattata–Raticate |
| 4 | 10% | Grimer–Muk | Grimer–Muk |
| 5 | 10% | Magnemite–Magneton | Meowth–Persian |
| 6 | 10% | Meowth–Persian | Gastly–Haunter |
| 7 | 5% | Porygon | Porygon |
| 8 | 5% | Voltorb | Magnemite–Magneton |
| 9 | 4% | Meowth–Persian | Zubat–Golbat |
| 10 | 4% | Porygon | Porygon |
| 11 | 1% | Porygon–Porygon2 | Porygon–Porygon2 |
| 12 | 1% | Koffing–Weezing | Gastly–Haunter |

#### Mt. Mortar

Dungeon, Johto east.

**`MAP_MT_MORTAR_1F_SOUTH_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Machop–Machoke | Zubat–Golbat |
| 2 | 20% | Zubat–Golbat | Machop–Machoke |
| 3 | 10% | Geodude–Graveler | Marill–Azumarill |
| 4 | 10% | Marill–Azumarill | Geodude–Graveler |
| 5 | 10% | Rattata–Raticate | Rattata–Raticate |
| 6 | 10% | Mankey–Primeape | Gastly–Haunter |
| 7 | 5% | Cubone–Marowak | Cubone–Marowak |
| 8 | 5% | Onix | Onix |
| 9 | 4% | Tyrogue | Tyrogue |
| 10 | 4% | Machop–Machoke | Marill–Azumarill |
| 11 | 1% | Hitmontop | Hitmontop |
| 12 | 1% | Dunsparce | Cleffa |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 5 | 1% | Magikarp–Gyarados | Zubat–Golbat |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Marill–Azumarill |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Zubat–Golbat |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

**`MAP_MT_MORTAR_1F_NORTH_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Geodude–Graveler | Marill–Azumarill |
| 2 | 20% | Marill–Azumarill | Geodude–Graveler |
| 3 | 10% | Machop–Machoke | Zubat–Golbat |
| 4 | 10% | Zubat–Golbat | Machop–Machoke |
| 5 | 10% | Rattata–Raticate | Gastly–Haunter |
| 6 | 10% | Cubone–Marowak | Cubone–Marowak |
| 7 | 5% | Onix | Onix |
| 8 | 5% | Tyrogue | Tyrogue |
| 9 | 4% | Dunsparce | Cleffa |
| 10 | 4% | Hitmontop | Hitmontop |
| 11 | 1% | Machop–Machamp | Zubat–Crobat |
| 12 | 1% | Geodude–Golem | Dunsparce |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Wooper–Quagsire |
| 2 | 30% | Wooper–Quagsire | Marill–Azumarill |
| 3 | 5% | Psyduck–Golduck | Zubat–Golbat |
| 4 | 4% | Goldeen–Seaking | Psyduck–Golduck |
| 5 | 1% | Magikarp–Gyarados | Goldeen–Seaking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Marill–Azumarill | Marill–Azumarill |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Wooper–Quagsire | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Zubat–Golbat |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Marill–Azumarill | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |

**`MAP_MT_MORTAR_2F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Machop–Machoke | Machop–Machoke |
| 2 | 20% | Mankey–Primeape | Marill–Azumarill |
| 3 | 10% | Geodude–Graveler | Zubat–Golbat |
| 4 | 10% | Zubat–Golbat | Mankey–Primeape |
| 5 | 10% | Marill–Azumarill | Geodude–Graveler |
| 6 | 10% | Rattata–Raticate | Gastly–Haunter |
| 7 | 5% | Machop–Machamp | Hitmontop |
| 8 | 5% | Hitmontop | Machop–Machamp |
| 9 | 4% | Tyrogue | Tyrogue |
| 10 | 4% | Onix | Gastly–Haunter |
| 11 | 1% | Geodude–Golem | Zubat–Crobat |
| 12 | 1% | Dunsparce | Cleffa |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Marill–Azumarill | Marill–Azumarill |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 5 | 1% | Slowpoke–Slowbro | Zubat–Golbat |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Goldeen–Seaking | Goldeen–Seaking |
| 4 | 8% | 10% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Marill–Azumarill |
| 6 | 4% | 7% | 10% | Slowpoke–Slowbro | Zubat–Golbat |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Marill–Azumarill | Slowpoke–Slowbro |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Marill–Azumarill |

**`MAP_MT_MORTAR_B1F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Onix | Geodude–Graveler |
| 2 | 20% | Geodude–Graveler | Onix |
| 3 | 10% | Machop–Machoke | Machop–Machoke |
| 4 | 10% | Cubone–Marowak | Zubat–Golbat |
| 5 | 10% | Zubat–Golbat | Marill–Azumarill |
| 6 | 10% | Marill–Azumarill | Gastly–Haunter |
| 7 | 5% | Hitmontop | Hitmontop |
| 8 | 5% | Tyrogue | Tyrogue |
| 9 | 4% | Machop–Machamp | Cleffa |
| 10 | 4% | Hitmontop | Hitmontop |
| 11 | 1% | Geodude–Golem | Zubat–Crobat |
| 12 | 1% | Zubat–Crobat | Gastly–Gengar |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Goldeen–Seaking | Zubat–Golbat |
| 3 | 5% | Slowpoke–Slowbro | Goldeen–Seaking |
| 4 | 4% | Psyduck–Golduck | Wooper–Quagsire |
| 5 | 1% | Poliwag–Poliwhirl | Slowpoke–Slowbro |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Marill–Azumarill | Marill–Azumarill |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Slowpoke–Slowbro | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Zubat–Golbat |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Slowpoke–Slowbro |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Goldeen–Seaking |

#### Tin Tower

Dungeon, Johto west.

**`MAP_TIN_TOWER_3F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Aipom | Aipom |
| 2 | 20% | Natu–Xatu | Murkrow |
| 3 | 10% | Misdreavus | Gastly–Haunter |
| 4 | 10% | Rattata–Raticate | Hoothoot–Noctowl |
| 5 | 10% | Houndour–Houndoom | Misdreavus |
| 6 | 10% | Natu–Xatu | Natu–Xatu |
| 7 | 5% | Misdreavus | Murkrow |
| 8 | 5% | Aipom | Gastly–Haunter |
| 9 | 4% | Houndour–Houndoom | Misdreavus |
| 10 | 4% | Gastly–Haunter | Houndour–Houndoom |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Misdreavus | Misdreavus |

**`MAP_TIN_TOWER_4F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Gastly–Haunter |
| 2 | 20% | Aipom | Natu–Xatu |
| 3 | 10% | Misdreavus | Hoothoot–Noctowl |
| 4 | 10% | Houndour–Houndoom | Aipom |
| 5 | 10% | Aipom–Ambipom | Misdreavus |
| 6 | 10% | Rattata–Raticate | Murkrow |
| 7 | 5% | Natu–Xatu | Misdreavus–Mismagius |
| 8 | 5% | Gastly–Haunter | Houndour–Houndoom |
| 9 | 4% | Misdreavus | Misdreavus |
| 10 | 4% | Aipom | Gastly–Gengar |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Misdreavus–Mismagius | Murkrow–Honchkrow |

**`MAP_TIN_TOWER_5F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Natu–Xatu |
| 2 | 20% | Misdreavus | Misdreavus |
| 3 | 10% | Aipom | Hoothoot–Noctowl |
| 4 | 10% | Houndour–Houndoom | Murkrow |
| 5 | 10% | Aipom–Ambipom | Gastly–Haunter |
| 6 | 10% | Rattata–Raticate | Aipom |
| 7 | 5% | Natu–Xatu | Misdreavus–Mismagius |
| 8 | 5% | Gastly–Haunter | Murkrow–Honchkrow |
| 9 | 4% | Misdreavus | Murkrow |
| 10 | 4% | Misdreavus–Mismagius | Houndour–Houndoom |
| 11 | 1% | Eevee–Espeon | Eevee–Umbreon |
| 12 | 1% | Aipom | Gastly–Gengar |

**`MAP_TIN_TOWER_6F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Gastly–Haunter |
| 2 | 20% | Aipom–Ambipom | Natu–Xatu |
| 3 | 10% | Misdreavus | Misdreavus |
| 4 | 10% | Houndour–Houndoom | Hoothoot–Noctowl |
| 5 | 10% | Gastly–Haunter | Murkrow |
| 6 | 10% | Natu–Xatu | Aipom–Ambipom |
| 7 | 5% | Misdreavus–Mismagius | Gastly–Gengar |
| 8 | 5% | Rattata–Raticate | Murkrow–Honchkrow |
| 9 | 4% | Misdreavus | Misdreavus–Mismagius |
| 10 | 4% | Aipom | Houndour–Houndoom |
| 11 | 1% | Eevee–Espeon | Eevee–Umbreon |
| 12 | 1% | Eevee | Murkrow |

**`MAP_TIN_TOWER_7F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Hoothoot–Noctowl |
| 2 | 20% | Aipom–Ambipom | Natu–Xatu |
| 3 | 10% | Misdreavus | Gastly–Haunter |
| 4 | 10% | Houndour–Houndoom | Murkrow |
| 5 | 10% | Natu–Xatu | Misdreavus |
| 6 | 10% | Rattata–Raticate | Aipom–Ambipom |
| 7 | 5% | Misdreavus–Mismagius | Gastly–Gengar |
| 8 | 5% | Gastly–Haunter | Murkrow–Honchkrow |
| 9 | 4% | Eevee–Espeon | Misdreavus–Mismagius |
| 10 | 4% | Misdreavus | Eevee–Umbreon |
| 11 | 1% | Eevee | Murkrow |
| 12 | 1% | Misdreavus–Mismagius | Houndour–Houndoom |

**`MAP_TIN_TOWER_8F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Gastly–Haunter |
| 2 | 20% | Misdreavus–Mismagius | Misdreavus |
| 3 | 10% | Aipom–Ambipom | Natu–Xatu |
| 4 | 10% | Houndour–Houndoom | Murkrow |
| 5 | 10% | Natu–Xatu | Misdreavus–Mismagius |
| 6 | 10% | Misdreavus | Hoothoot–Noctowl |
| 7 | 5% | Eevee–Espeon | Murkrow–Honchkrow |
| 8 | 5% | Rattata–Raticate | Eevee–Umbreon |
| 9 | 4% | Gastly–Haunter | Gastly–Gengar |
| 10 | 4% | Gastly–Gengar | Houndour–Houndoom |
| 11 | 1% | Eevee | Zubat–Crobat |
| 12 | 1% | Misdreavus | Eevee–Espeon |

**`MAP_TIN_TOWER_9F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Hoothoot–Noctowl |
| 2 | 20% | Aipom–Ambipom | Natu–Xatu |
| 3 | 10% | Misdreavus–Mismagius | Murkrow–Honchkrow |
| 4 | 10% | Houndour–Houndoom | Murkrow |
| 5 | 10% | Misdreavus | Misdreavus–Mismagius |
| 6 | 10% | Natu–Xatu | Gastly–Haunter |
| 7 | 5% | Eevee–Espeon | Eevee–Umbreon |
| 8 | 5% | Gastly–Haunter | Misdreavus |
| 9 | 4% | Houndour–Houndoom | Gastly–Gengar |
| 10 | 4% | Gastly–Gengar | Zubat–Crobat |
| 11 | 1% | Eevee–Espeon | Eevee–Umbreon |
| 12 | 1% | Eevee | Houndour–Houndoom |

#### Dragon's Den

Dungeon, Johto east.

**`MAP_DRAGONS_DEN_CAVERN_HNS`**

Water type: cave water.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Magikarp–Gyarados | Dratini–Dragonite |
| 2 | 30% | Poliwag–Poliwhirl | Magikarp–Gyarados |
| 3 | 5% | Dratini–Dragonite | Poliwag–Poliwhirl |
| 4 | 4% | Slowpoke–Slowbro | Dratini–Dragonite |
| 5 | 1% | Dratini–Dragonite | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Goldeen–Seaking | Poliwag–Poliwhirl |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Dratini–Dragonite | Dratini–Dragonite |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Magikarp–Gyarados |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Dratini–Dragonite |
| 8 | 3% | 5% | 9% | Dratini–Dragonite | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Wooper–Quagsire |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Dratini–Dragonite |

#### Whirl Islands

Dungeon, Johto west.

**`MAP_WHIRL_ISLANDS_1F_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Krabby–Kingler | Krabby–Kingler |
| 2 | 20% | Seel–Dewgong | Seel–Dewgong |
| 3 | 10% | Zubat–Golbat | Misdreavus |
| 4 | 10% | Wobbuffet | Zubat–Golbat |
| 5 | 10% | Dunsparce | Wobbuffet |
| 6 | 10% | Slowpoke–Slowbro | Dunsparce |
| 7 | 5% | Wobbuffet | Slowpoke–Slowbro |
| 8 | 5% | Shuckle | Misdreavus |
| 9 | 4% | Seel–Dewgong | Shuckle |
| 10 | 4% | Dunsparce | Zubat–Golbat |
| 11 | 1% | Shuckle | Seel–Dewgong |
| 12 | 1% | Krabby–Kingler | Dunsparce |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Corsola |
| 2 | 30% | Corsola | Tentacool–Tentacruel |
| 3 | 5% | Mantine | Chinchou–Lanturn |
| 4 | 4% | Horsea–Seadra | Mantine |
| 5 | 1% | Mantyke | Horsea–Seadra |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Corsola | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Qwilfish | Corsola |
| 5 | 8% | 9% | 10% | Qwilfish | Staryu |
| 6 | 4% | 7% | 10% | Corsola | Qwilfish |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Krabby–Kingler | Staryu |
| 9 | 2% | 4% | 9% | Chinchou–Lanturn | Horsea–Seadra |
| 10 | 2% | 4% | 9% | Horsea–Seadra | Corsola |

**`MAP_WHIRL_ISLANDS_B1F_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 20% | Krabby–Kingler | Misdreavus |
| 3 | 10% | Dunsparce | Wobbuffet |
| 4 | 10% | Wobbuffet | Zubat–Golbat |
| 5 | 10% | Seel–Dewgong | Krabby–Kingler |
| 6 | 10% | Zubat–Golbat | Dunsparce |
| 7 | 5% | Slowpoke–Slowking | Zubat–Crobat |
| 8 | 5% | Shuckle | Slowpoke–Slowking |
| 9 | 4% | Wobbuffet | Misdreavus |
| 10 | 4% | Dunsparce | Seel–Dewgong |
| 11 | 1% | Slowpoke–Slowking | Gastly–Gengar |
| 12 | 1% | Shuckle | Shuckle |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Horsea–Seadra | Chinchou–Lanturn |
| 2 | 30% | Tentacool–Tentacruel | Horsea–Seadra |
| 3 | 5% | Mantine | Staryu |
| 4 | 4% | Corsola | Mantine |
| 5 | 1% | Mantyke | Corsola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Corsola | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Qwilfish | Staryu |
| 6 | 4% | 7% | 10% | Horsea–Seadra | Qwilfish |
| 7 | 3% | 6% | 10% | Shellder | Staryu |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Remoraid–Octillery |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Shellder–Cloyster |

**`MAP_WHIRL_ISLANDS_B1F_INNER_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Krabby–Kingler | Misdreavus |
| 3 | 10% | Wobbuffet | Wobbuffet |
| 4 | 10% | Dunsparce | Zubat–Golbat |
| 5 | 10% | Slowpoke–Slowbro | Dunsparce |
| 6 | 10% | Zubat–Golbat | Slowpoke–Slowbro |
| 7 | 5% | Slowpoke–Slowking | Zubat–Crobat |
| 8 | 5% | Shuckle | Slowpoke–Slowking |
| 9 | 4% | Wobbuffet | Misdreavus |
| 10 | 4% | Zubat–Crobat | Krabby–Kingler |
| 11 | 1% | Slowpoke–Slowking | Gastly–Gengar |
| 12 | 1% | Shuckle | Shuckle |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Horsea–Seadra | Chinchou–Lanturn |
| 2 | 30% | Tentacool–Tentacruel | Horsea–Seadra |
| 3 | 5% | Corsola | Staryu |
| 4 | 4% | Mantine | Corsola |
| 5 | 1% | Horsea–Kingdra | Horsea–Kingdra |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Qwilfish | Qwilfish |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Horsea–Seadra | Horsea–Seadra |
| 6 | 4% | 7% | 10% | Remoraid–Octillery | Staryu |
| 7 | 3% | 6% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Corsola | Remoraid–Octillery |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 10 | 2% | 4% | 9% | Qwilfish | Corsola |

**`MAP_WHIRL_ISLANDS_B2F_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Krabby–Kingler | Misdreavus |
| 3 | 10% | Wobbuffet | Zubat–Golbat |
| 4 | 10% | Dunsparce | Wobbuffet |
| 5 | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 6 | 10% | Zubat–Golbat | Dunsparce |
| 7 | 5% | Slowpoke–Slowking | Misdreavus–Mismagius |
| 8 | 5% | Shuckle | Shuckle |
| 9 | 4% | Zubat–Crobat | Zubat–Crobat |
| 10 | 4% | Wobbuffet | Misdreavus |
| 11 | 1% | Shuckle | Slowpoke–Slowking |
| 12 | 1% | Slowpoke–Slowking | Shuckle |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Horsea–Seadra | Horsea–Seadra |
| 2 | 30% | Qwilfish | Qwilfish |
| 3 | 5% | Lapras | Chinchou–Lanturn |
| 4 | 4% | Mantine | Lapras |
| 5 | 1% | Horsea–Kingdra | Horsea–Kingdra |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Qwilfish | Qwilfish |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Krabby–Kingler |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Corsola | Qwilfish |
| 6 | 4% | 7% | 10% | Qwilfish | Staryu |
| 7 | 3% | 6% | 10% | Horsea–Kingdra | Horsea–Kingdra |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Staryu |
| 9 | 2% | 4% | 9% | Shellder–Cloyster | Shellder–Cloyster |
| 10 | 2% | 4% | 9% | Slowpoke–Slowbro | Chinchou–Lanturn |

**`MAP_WHIRL_ISLANDS_B3F_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Slowpoke–Slowbro | Misdreavus |
| 3 | 10% | Wobbuffet | Zubat–Crobat |
| 4 | 10% | Shuckle | Wobbuffet |
| 5 | 10% | Dunsparce | Slowpoke–Slowbro |
| 6 | 10% | Zubat–Crobat | Shuckle |
| 7 | 5% | Slowpoke–Slowking | Misdreavus–Mismagius |
| 8 | 5% | Krabby–Kingler | Slowpoke–Slowking |
| 9 | 4% | Wobbuffet | Dunsparce |
| 10 | 4% | Shuckle | Gastly–Gengar |
| 11 | 1% | Slowpoke–Slowking | Misdreavus |
| 12 | 1% | Zubat–Crobat | Shuckle |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Qwilfish | Qwilfish |
| 2 | 30% | Horsea–Seadra | Horsea–Seadra |
| 3 | 5% | Lapras | Chinchou–Lanturn |
| 4 | 4% | Mantine | Lapras |
| 5 | 1% | Horsea–Kingdra | Horsea–Kingdra |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Qwilfish | Qwilfish |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Corsola | Qwilfish |
| 6 | 4% | 7% | 10% | Horsea–Kingdra | Staryu |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Horsea–Kingdra |
| 8 | 3% | 5% | 9% | Shellder–Cloyster | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Slowpoke–Slowking | Shellder–Cloyster |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Staryu |

**`MAP_WHIRL_ISLANDS_DESCENT_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Slowpoke–Slowking | Misdreavus–Mismagius |
| 3 | 10% | Wobbuffet | Slowpoke–Slowking |
| 4 | 10% | Shuckle | Zubat–Crobat |
| 5 | 10% | Zubat–Crobat | Wobbuffet |
| 6 | 10% | Krabby–Kingler | Shuckle |
| 7 | 5% | Dunsparce | Misdreavus |
| 8 | 5% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 9 | 4% | Slowpoke–Slowking | Gastly–Gengar |
| 10 | 4% | Wobbuffet | Dunsparce |
| 11 | 1% | Shuckle | Shuckle |
| 12 | 1% | Zubat–Crobat | Misdreavus–Mismagius |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Horsea–Kingdra | Horsea–Kingdra |
| 2 | 30% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 3 | 5% | Lapras | Lapras |
| 4 | 4% | Mantine | Tentacool–Tentacruel |
| 5 | 1% | Slowpoke–Slowking | Mantine |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Horsea–Kingdra | Horsea–Kingdra |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Krabby–Kingler |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Shellder–Cloyster | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Qwilfish | Staryu–Starmie |
| 6 | 4% | 7% | 10% | Corsola | Shellder–Cloyster |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Slowpoke–Slowking | Qwilfish |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Staryu–Starmie |
| 10 | 2% | 4% | 9% | Remoraid–Octillery | Horsea–Kingdra |

#### Mt. Silver

Dungeon, Border.

**`MAP_MT_SILVER_OUTSIDE_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Phanpy–Donphan | Teddiursa–Ursaring |
| 2 | 20% | Ponyta–Rapidash | Phanpy–Donphan |
| 3 | 10% | Teddiursa–Ursaring | Zubat–Golbat |
| 4 | 10% | Geodude–Graveler | Houndour–Houndoom |
| 5 | 10% | Spearow–Fearow | Wooper–Quagsire |
| 6 | 10% | Doduo–Dodrio | Sneasel |
| 7 | 5% | Skarmory | Misdreavus |
| 8 | 5% | Tauros | Skarmory |
| 9 | 4% | Rhyhorn–Rhydon | Geodude–Graveler |
| 10 | 4% | Skarmory | Houndour–Houndoom |
| 11 | 1% | Miltank | Wobbuffet |
| 12 | 1% | Machop–Machoke | Murkrow |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Goldeen–Seaking | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Magikarp–Gyarados |
| 5 | 1% | Marill–Azumarill | Goldeen–Seaking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Psyduck–Golduck | Psyduck–Golduck |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pineco–Forretress | Teddiursa–Ursaring |
| 2 | 30% | Teddiursa–Ursaring | Pineco–Forretress |
| 3 | 5% | Heracross | Hoothoot–Noctowl |
| 4 | 4% | Scyther | Heracross |
| 5 | 1% | Aipom | Scyther |

**`MAP_MT_SILVER_1F_WATERFALL_ROOM_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Onix | Onix |
| 2 | 20% | Teddiursa–Ursaring | Teddiursa–Ursaring |
| 3 | 10% | Zubat–Golbat | Zubat–Golbat |
| 4 | 10% | Geodude–Graveler | Geodude–Graveler |
| 5 | 10% | Machop–Machoke | Misdreavus |
| 6 | 10% | Paras–Parasect | Paras–Parasect |
| 7 | 5% | Psyduck–Golduck | Psyduck–Golduck |
| 8 | 5% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 9 | 4% | Sneasel | Magmar |
| 10 | 4% | Magmar | Misdreavus–Mismagius |
| 11 | 1% | Onix–Steelix | Zubat–Crobat |
| 12 | 1% | Smoochum | Magby |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 5% | Poliwag–Poliwhirl | Slowpoke–Slowking |
| 4 | 4% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 1% | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Slowpoke–Slowbro | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwrath | Slowpoke–Slowking |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Poliwag–Politoed |
| 8 | 3% | 5% | 9% | Slowpoke–Slowbro | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Marill–Azumarill | Zubat–Golbat |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Goldeen–Seaking |

**`MAP_MT_SILVER_MOUNTAIN_SIDE_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ponyta–Rapidash | Teddiursa–Ursaring |
| 2 | 20% | Teddiursa–Ursaring | Ponyta–Rapidash |
| 3 | 10% | Skarmory | Houndour–Houndoom |
| 4 | 10% | Machop–Machamp | Zubat–Crobat |
| 5 | 10% | Phanpy–Donphan | Misdreavus–Mismagius |
| 6 | 10% | Geodude–Golem | Sneasel–Weavile |
| 7 | 5% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 8 | 5% | Tauros | Houndour–Houndoom |
| 9 | 4% | Mankey–Annihilape | Murkrow–Honchkrow |
| 10 | 4% | Skarmory | Gastly–Gengar |
| 11 | 1% | Sneasel–Weavile | Snorlax |
| 12 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pineco–Forretress | Teddiursa–Ursaring |
| 2 | 30% | Teddiursa–Ursaring | Pineco–Forretress |
| 3 | 5% | Heracross | Hoothoot–Noctowl |
| 4 | 4% | Scyther–Scizor | Heracross |
| 5 | 1% | Aipom–Ambipom | Scyther–Scizor |

**`MAP_MT_SILVER_1F_ITEM_ROOM_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Geodude–Golem | Geodude–Golem |
| 2 | 20% | Machop–Machamp | Machop–Machamp |
| 3 | 10% | Teddiursa–Ursaring | Zubat–Crobat |
| 4 | 10% | Onix–Steelix | Onix–Steelix |
| 5 | 10% | Zubat–Crobat | Misdreavus–Mismagius |
| 6 | 10% | Psyduck–Golduck | Teddiursa–Ursaring |
| 7 | 5% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 8 | 5% | Sneasel | Magmar |
| 9 | 4% | Magmar | Sneasel–Weavile |
| 10 | 4% | Smoochum | Gastly–Gengar |
| 11 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 12 | 1% | Magby | Magby |

**`MAP_MT_SILVER_1F_MOLTRES_ROOM_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Paras–Parasect | Paras–Parasect |
| 2 | 20% | Machop–Machamp | Machop–Machamp |
| 3 | 10% | Onix | Zubat–Crobat |
| 4 | 10% | Geodude–Golem | Misdreavus–Mismagius |
| 5 | 10% | Magmar | Magmar |
| 6 | 10% | Houndour–Houndoom | Gastly–Gengar |
| 7 | 5% | Onix–Steelix | Onix–Steelix |
| 8 | 5% | Magmar–Magmortar | Magmar–Magmortar |
| 9 | 4% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 10 | 4% | Sneasel–Weavile | Houndour–Houndoom |
| 11 | 1% | Magby | Magby |
| 12 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |

**`MAP_MT_SILVER_2F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 20% | Machop–Machamp | Machop–Machamp |
| 3 | 10% | Psyduck–Golduck | Zubat–Crobat |
| 4 | 10% | Geodude–Golem | Misdreavus–Mismagius |
| 5 | 10% | Larvitar–Tyranitar | Geodude–Golem |
| 6 | 10% | Zubat–Crobat | Larvitar–Tyranitar |
| 7 | 5% | Sneasel | Gastly–Gengar |
| 8 | 5% | Magmar | Magmar |
| 9 | 4% | Sneasel–Weavile | Sneasel–Weavile |
| 10 | 4% | Magmar–Magmortar | Psyduck–Golduck |
| 11 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 12 | 1% | Paras–Parasect | Snorlax |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 30% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 5% | Poliwag–Poliwhirl | Slowpoke–Slowking |
| 4 | 4% | Psyduck–Golduck | Poliwag–Politoed |
| 5 | 1% | Goldeen–Seaking | Zubat–Golbat |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Slowpoke–Slowbro | Poliwag–Politoed |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Wooper–Quagsire | Slowpoke–Slowbro |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Zubat–Golbat |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Slowpoke–Slowking | Slowpoke–Slowking |
| 10 | 2% | 4% | 9% | Psyduck–Golduck | Poliwag–Politoed |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Golem | Dunsparce |
| 2 | 30% | Geodude–Graveler | Geodude–Golem |
| 3 | 5% | Shuckle | Geodude–Graveler |
| 4 | 4% | Dunsparce | Shuckle |
| 5 | 1% | Geodude–Golem | Geodude–Golem |

**`MAP_MT_SILVER_SNOW_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Swinub–Mamoswine | Sneasel–Weavile |
| 2 | 20% | Teddiursa–Ursaring | Swinub–Mamoswine |
| 3 | 10% | Sneasel–Weavile | Houndour–Houndoom |
| 4 | 10% | Skarmory | Zubat–Crobat |
| 5 | 10% | Delibird | Misdreavus–Mismagius |
| 6 | 10% | Swinub–Piloswine | Delibird |
| 7 | 5% | Larvitar–Tyranitar | Gastly–Gengar |
| 8 | 5% | Jynx | Larvitar–Tyranitar |
| 9 | 4% | Delibird | Jynx |
| 10 | 4% | Skarmory | Murkrow–Honchkrow |
| 11 | 1% | Snorlax | Smoochum |
| 12 | 1% | Larvitar–Tyranitar | Sneasel–Weavile |

**`MAP_MT_SILVER_3F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Teddiursa–Ursaring | Zubat–Crobat |
| 2 | 20% | Zubat–Crobat | Teddiursa–Ursaring |
| 3 | 10% | Onix–Steelix | Onix–Steelix |
| 4 | 10% | Geodude–Golem | Geodude–Golem |
| 5 | 10% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 6 | 10% | Machop–Machamp | Misdreavus–Mismagius |
| 7 | 5% | Larvitar–Tyranitar | Gastly–Gengar |
| 8 | 5% | Phanpy–Donphan | Larvitar–Tyranitar |
| 9 | 4% | Sneasel–Weavile | Sneasel–Weavile |
| 10 | 4% | Magmar–Magmortar | Houndour–Houndoom |
| 11 | 1% | Snorlax | Snorlax |
| 12 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Abra | Route 30, Route 34, Route 35, Ruins of Alph and more |
| Aerodactyl | Faraway Island |
| Aipom | Azalea Town, Cherrygrove City, Cianwood City, Goldenrod City and more |
| Ambipom | Mt. Silver, Route 26, Route 28, Tin Tower |
| Ampharos | Route 31, Route 32, Route 43, Ruins of Alph and more |
| Annihilape | Mt. Silver |
| Arbok | Route 26, Route 27, Route 28, Route 32 and more |
| Ariados | Azalea Town, Cherrygrove City, Cianwood City, Faraway Island and more |
| Azumarill | Blackthorn City, Dark Cave, Ecruteak City, Ilex Forest and more |
| Bayleef | Ilex Forest, Route 27, Route 29 |
| Beedrill | Azalea Town, Ilex Forest, National Park, Route 30 and more |
| Bellossom | Faraway Island |
| Bellsprout | Faraway Island, Route 31, Route 32, Route 36 and more |
| Bonsly | Route 36 |
| Butterfree | Azalea Town, Goldenrod City, Ilex Forest, National Park and more |
| Caterpie | Azalea Town, Goldenrod City, Ilex Forest, National Park and more |
| Chikorita | Ilex Forest, Route 27, Route 29 |
| Chinchou | Cianwood City, Cliff Edge Cave, Faraway Island, Goldenrod City and more |
| Cleffa | Faraway Island, Mt. Mortar |
| Cloyster | Faraway Island, Route 26, Whirl Islands |
| Corsola | Cherrygrove City, Cianwood City, Cliff Edge Cave, Faraway Island and more |
| Crobat | Ice Path, Mt. Mortar, Mt. Silver, Route 28 and more |
| Croconaw | Lake of Rage, Route 27, Route 30 |
| Cubone | Mt. Mortar, Union Cave |
| Cyndaquil | Burned Tower, Route 46 |
| Delibird | Ice Path, Mt. Silver |
| Dewgong | Cliff Edge Cave, Whirl Islands |
| Ditto | Faraway Island, Route 34, Route 35, Route 47 |
| Dodrio | Mt. Silver, Route 26, Route 27, Route 28 |
| Doduo | Mt. Silver, Route 26, Route 27, Route 28 |
| Donphan | Mt. Silver, Route 26, Route 28, Route 45 and more |
| Dragonair | Dragon's Den, Faraway Island, Southern Island |
| Dragonite | Dragon's Den |
| Dratini | Dragon's Den, Faraway Island, Southern Island |
| Drowzee | Route 34, Route 35 |
| Dunsparce | Cliff Edge Cave, Dark Cave, Ice Path, Mt. Mortar and more |
| Eevee | Route 34, Route 37, Route 39, Tin Tower |
| Ekans | Route 26, Route 27, Route 28, Route 32 and more |
| Electrode | Rocket Hideout |
| Elekid | Route 44 |
| Espeon | Tin Tower |
| Exeggcute | Azalea Town, Cherrygrove City, Cianwood City, Faraway Island and more |
| Exeggutor | Faraway Island |
| Farfetch'd | Olivine City, Route 38, Route 39, Route 43 and more |
| Farigiraf | Route 28 |
| Fearow | Cherrygrove City, Mt. Silver, Olivine City, Route 26 and more |
| Feraligatr | Lake of Rage, Route 27, Route 30 |
| Flaaffy | Route 31, Route 32, Route 43, Ruins of Alph and more |
| Forretress | Azalea Town, Faraway Island, Goldenrod City, Ilex Forest and more |
| Furret | Azalea Town, Route 29, Route 30, Route 36 and more |
| Gastly | Azalea Town, Burned Tower, Dark Cave, Ice Path and more |
| Gengar | Burned Tower, Mt. Mortar, Mt. Silver, Route 26 and more |
| Geodude | Cianwood City, Cliff Edge Cave, Dark Cave, Ice Path and more |
| Girafarig | National Park, Route 27, Route 28, Route 47 and more |
| Gligar | Dark Cave, Route 42, Route 45, Route 46 |
| Gloom | Azalea Town, Faraway Island, Ilex Forest, Olivine City and more |
| Golbat | Burned Tower, Cliff Edge Cave, Dark Cave, Ice Path and more |
| Goldeen | Blackthorn City, Dark Cave, Dragon's Den, Ecruteak City and more |
| Golduck | Blackthorn City, Dark Cave, Dragon's Den, Ecruteak City and more |
| Golem | Ice Path, Mt. Mortar, Mt. Silver, Route 26 |
| Granbull | National Park, Olivine City, Route 34, Route 35 and more |
| Graveler | Cianwood City, Cliff Edge Cave, Dark Cave, Ice Path and more |
| Grimer | Olivine City, Rocket Hideout |
| Growlithe | Route 35, Route 36, Route 37, Route 48 |
| Gyarados | Blackthorn City, Cherrygrove City, Cianwood City, Cliff Edge Cave and more |
| Happiny | Route 47 |
| Haunter | Azalea Town, Burned Tower, Dark Cave, Ice Path and more |
| Heracross | Lake of Rage, Mt. Silver, Route 26, Route 27 and more |
| Hitmontop | Mt. Mortar |
| Honchkrow | Mt. Silver, Route 26, Route 28, Tin Tower |
| Hoothoot | Azalea Town, Cherrygrove City, Faraway Island, Goldenrod City and more |
| Hoppip | Azalea Town, Faraway Island, Olivine City, Route 29 and more |
| Horsea | Cianwood City, Cliff Edge Cave, Faraway Island, Goldenrod City and more |
| Houndoom | Burned Tower, Faraway Island, Mt. Silver, Route 26 and more |
| Houndour | Burned Tower, Faraway Island, Mt. Silver, Route 26 and more |
| Hypno | Route 34, Route 35 |
| Igglybuff | Azalea Town, Route 34 |
| Jigglypuff | Route 34 |
| Jumpluff | Azalea Town, Faraway Island, Olivine City, Route 29 and more |
| Jynx | Ice Path, Mt. Silver |
| Kadabra | Route 34, Route 35 |
| Kakuna | Azalea Town, Ilex Forest, National Park, Route 30 and more |
| Kingdra | Faraway Island, Route 26, Southern Island, Whirl Islands |
| Kingler | Cherrygrove City, Cianwood City, Cliff Edge Cave, Faraway Island and more |
| Koffing | Burned Tower, Rocket Hideout, Slowpoke Well |
| Krabby | Cherrygrove City, Cianwood City, Cliff Edge Cave, Faraway Island and more |
| Lanturn | Cianwood City, Cliff Edge Cave, Faraway Island, Goldenrod City and more |
| Lapras | Faraway Island, Route 26, Southern Island, Union Cave and more |
| Larvitar | Mt. Silver, Route 26, Route 28 |
| Ledian | Azalea Town, Cherrygrove City, Goldenrod City, Ilex Forest and more |
| Ledyba | Azalea Town, Cherrygrove City, Goldenrod City, Ilex Forest and more |
| Lickitung | Route 44 |
| Machamp | Mt. Mortar, Mt. Silver |
| Machoke | Dark Cave, Mt. Mortar, Mt. Silver, Route 33 and more |
| Machop | Dark Cave, Mt. Mortar, Mt. Silver, Route 33 and more |
| Magby | Burned Tower, Mt. Silver |
| Magcargo | Burned Tower |
| Magikarp | Blackthorn City, Cherrygrove City, Cianwood City, Cliff Edge Cave and more |
| Magmar | Burned Tower, Mt. Silver |
| Magmortar | Burned Tower, Mt. Silver |
| Magnemite | Olivine City, Rocket Hideout, Route 38, Route 39 |
| Magneton | Olivine City, Rocket Hideout, Route 38, Route 39 |
| Mamoswine | Ice Path, Mt. Silver |
| Mankey | Cianwood City, Mt. Mortar, Mt. Silver, Route 26 and more |
| Mantine | Cliff Edge Cave, Faraway Island, Route 26, Route 27 and more |
| Mantyke | Cianwood City, Olivine City, Route 41, Whirl Islands |
| Mareep | Route 31, Route 32, Route 43, Ruins of Alph and more |
| Marill | Blackthorn City, Dark Cave, Ecruteak City, Ilex Forest and more |
| Marowak | Mt. Mortar, Union Cave |
| Meganium | Ilex Forest, Route 27, Route 29 |
| Meowth | Olivine City, Rocket Hideout, Route 34, Route 38 and more |
| Metapod | Azalea Town, Goldenrod City, Ilex Forest, National Park and more |
| Miltank | Mt. Silver, Route 26, Route 27, Route 47 |
| Misdreavus | Burned Tower, Cliff Edge Cave, Faraway Island, Ilex Forest and more |
| Mismagius | Burned Tower, Mt. Silver, Tin Tower, Whirl Islands |
| Muk | Olivine City, Rocket Hideout |
| Murkrow | Azalea Town, Burned Tower, Cherrygrove City, Faraway Island and more |
| Natu | Ruins of Alph, Sprout Tower, Tin Tower, Violet City |
| Nidoran♀ | National Park, Route 35, Route 36, Route 48 |
| Nidoran♂ | National Park, Route 35, Route 36 |
| Nidorina | National Park, Route 35, Route 36, Route 48 |
| Nidorino | National Park, Route 35, Route 36 |
| Noctowl | Azalea Town, Cherrygrove City, Faraway Island, Goldenrod City and more |
| Octillery | Cianwood City, Goldenrod City, New Bark Town, Olivine City and more |
| Oddish | Azalea Town, Faraway Island, Ilex Forest, Olivine City and more |
| Onix | Cliff Edge Cave, Dark Cave, Ice Path, Mt. Mortar and more |
| Paras | Azalea Town, Ilex Forest, Mt. Silver |
| Parasect | Azalea Town, Ilex Forest, Mt. Silver |
| Persian | Olivine City, Rocket Hideout, Route 34, Route 38 and more |
| Phanpy | Mt. Silver, Route 26, Route 28, Route 45 and more |
| Pichu | Azalea Town, Ilex Forest, Route 31 |
| Pidgeot | Azalea Town, Cherrygrove City, Mahogany Town, Olivine City and more |
| Pidgeotto | Azalea Town, Cherrygrove City, Mahogany Town, Olivine City and more |
| Pidgey | Azalea Town, Cherrygrove City, Mahogany Town, Olivine City and more |
| Piloswine | Ice Path, Mt. Silver |
| Pineco | Azalea Town, Faraway Island, Goldenrod City, Ilex Forest and more |
| Pinsir | Faraway Island, National Park |
| Politoed | Mt. Silver, Route 28 |
| Poliwag | Blackthorn City, Dark Cave, Dragon's Den, Ecruteak City and more |
| Poliwhirl | Blackthorn City, Dark Cave, Dragon's Den, Ecruteak City and more |
| Poliwrath | Mt. Silver, Route 28 |
| Ponyta | Mt. Silver, Route 26, Route 27, Route 28 and more |
| Porygon | Rocket Hideout |
| Porygon2 | Rocket Hideout |
| Primeape | Cianwood City, Mt. Mortar, Mt. Silver, Route 26 and more |
| Psyduck | Blackthorn City, Dark Cave, Dragon's Den, Ecruteak City and more |
| Pupitar | Mt. Silver, Route 26, Route 28 |
| Quagsire | Blackthorn City, Dark Cave, Dragon's Den, Ecruteak City and more |
| Quilava | Burned Tower, Route 46 |
| Qwilfish | Cherrygrove City, Cianwood City, Cliff Edge Cave, Faraway Island and more |
| Rapidash | Mt. Silver, Route 26, Route 27, Route 28 and more |
| Raticate | Burned Tower, Mt. Mortar, Olivine City, Rocket Hideout and more |
| Rattata | Burned Tower, Mt. Mortar, Olivine City, Rocket Hideout and more |
| Remoraid | Cianwood City, Goldenrod City, New Bark Town, Olivine City and more |
| Rhydon | Mt. Silver |
| Rhyhorn | Mt. Silver |
| Sandshrew | Route 26, Route 27, Union Cave |
| Sandslash | Route 26, Route 27, Union Cave |
| Scizor | Mt. Silver, Route 26, Route 28 |
| Scyther | Faraway Island, Mt. Silver, National Park, Route 26 and more |
| Seadra | Cianwood City, Cliff Edge Cave, Faraway Island, Goldenrod City and more |
| Seaking | Blackthorn City, Dark Cave, Dragon's Den, Ecruteak City and more |
| Seel | Cliff Edge Cave, Whirl Islands |
| Sentret | Azalea Town, Route 29, Route 30, Route 36 and more |
| Shellder | Cherrygrove City, Cianwood City, Cliff Edge Cave, Faraway Island and more |
| Shuckle | Cliff Edge Cave, Dark Cave, Ice Path, Mt. Silver and more |
| Skarmory | Mt. Silver, Route 28, Route 47 |
| Skiploom | Azalea Town, Faraway Island, Olivine City, Route 29 and more |
| Slowbro | Azalea Town, Blackthorn City, Cliff Edge Cave, Dragon's Den and more |
| Slowking | Mt. Silver, Route 28, Slowpoke Well, Whirl Islands |
| Slowpoke | Azalea Town, Blackthorn City, Cliff Edge Cave, Dragon's Den and more |
| Slugma | Burned Tower |
| Smeargle | Faraway Island, Ruins of Alph |
| Smoochum | Ice Path, Mt. Silver |
| Sneasel | Ice Path, Mt. Silver, Route 28 |
| Snorlax | Mt. Silver, Route 28 |
| Snubbull | National Park, Olivine City, Route 34, Route 35 and more |
| Spearow | Cherrygrove City, Mt. Silver, Olivine City, Route 26 and more |
| Spinarak | Azalea Town, Cherrygrove City, Cianwood City, Faraway Island and more |
| Stantler | National Park |
| Starmie | Faraway Island, Route 26, Southern Island, Whirl Islands |
| Staryu | Cherrygrove City, Cianwood City, Cliff Edge Cave, Faraway Island and more |
| Steelix | Ice Path, Mt. Silver, Union Cave |
| Sunkern | Azalea Town, Ilex Forest, National Park, Olivine City and more |
| Swinub | Ice Path, Mt. Silver |
| Tangela | Faraway Island, Route 28, Route 44 |
| Tangrowth | Route 28 |
| Tauros | Mt. Silver, Route 26, Route 27, Route 28 and more |
| Teddiursa | Dark Cave, Mt. Silver, Route 26, Route 28 and more |
| Tentacool | Cherrygrove City, Cianwood City, Cliff Edge Cave, Faraway Island and more |
| Tentacruel | Cherrygrove City, Cianwood City, Cliff Edge Cave, Faraway Island and more |
| Togepi | National Park |
| Totodile | Lake of Rage, Route 27, Route 30 |
| Typhlosion | Burned Tower, Route 46 |
| Tyranitar | Mt. Silver, Route 26, Route 28 |
| Tyrogue | Mt. Mortar, Route 42 |
| Umbreon | Tin Tower |
| Unown | Ruins of Alph |
| Ursaring | Dark Cave, Mt. Silver, Route 26, Route 28 and more |
| Venomoth | Azalea Town, Faraway Island, Ilex Forest, Lake of Rage and more |
| Venonat | Azalea Town, Faraway Island, Ilex Forest, Lake of Rage and more |
| Victreebel | Faraway Island, Sprout Tower |
| Vileplume | Faraway Island |
| Voltorb | Rocket Hideout |
| Vulpix | Burned Tower, Route 36, Route 37, Route 48 |
| Weavile | Ice Path, Mt. Silver, Route 28 |
| Weedle | Azalea Town, Ilex Forest, National Park, Route 30 and more |
| Weepinbell | Faraway Island, Route 31, Route 32, Route 36 and more |
| Weezing | Burned Tower, Rocket Hideout, Slowpoke Well |
| Wobbuffet | Cliff Edge Cave, Dark Cave, Mt. Silver, Slowpoke Well and more |
| Wooper | Blackthorn City, Dark Cave, Dragon's Den, Ecruteak City and more |
| Xatu | Ruins of Alph, Sprout Tower, Tin Tower, Violet City |
| Yanma | Ilex Forest, Lake of Rage, Route 35, Route 48 |
| Zubat | Burned Tower, Cliff Edge Cave, Dark Cave, Ice Path and more |
