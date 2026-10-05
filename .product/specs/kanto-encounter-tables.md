# Kanto encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Kanto and Johto encounter rules](kanto-johto-encounters.md). They are the
source of truth for Kanto's wild-encounter data: every slot below maps one to one
to a slot in the game's encounter tables. Playtesting may change the picks.

## Scope

This spec lists Kanto's wild-encounter tables: 63 maps, each with a day and a
night table for every method it has. The Safari Zone is deferred and not
listed. Legendaries and mythicals belong to a later spec.

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

### Tables

#### Pallet Town

Road, Kanto west.

**`MAP_PALLET_TOWN_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Krabby–Kingler | Krabby–Kingler |
| 2 | 30% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 5% | Shellder | Chinchou–Lanturn |
| 4 | 4% | Horsea–Seadra | Horsea–Seadra |
| 5 | 1% | Squirtle–Blastoise | Squirtle–Blastoise |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Krabby–Kingler |
| 3 | 10% | 12% | 11% | Shellder | Shellder |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Krabby–Kingler |
| 7 | 3% | 6% | 10% | Krabby | Staryu |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 9 | 2% | 4% | 9% | Horsea–Seadra | Chinchou–Lanturn |
| 10 | 2% | 4% | 9% | Squirtle–Blastoise | Squirtle–Blastoise |

#### Viridian City

Road, Kanto west.

**`MAP_VIRIDIAN_CITY_HNS`**

Water type: ponds and rivers.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Psyduck–Golduck | Slowpoke–Slowbro |
| 3 | 5% | Marill–Azumarill | Psyduck–Golduck |
| 4 | 4% | Goldeen–Seaking | Marill–Azumarill |
| 5 | 1% | Slowpoke–Slowbro | Slowpoke–Slowbro |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Slowpoke–Slowbro |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Marill–Azumarill |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Weedle–Beedrill | Weedle–Beedrill |
| 2 | 30% | Spearow–Fearow | Spearow–Fearow |
| 3 | 5% | Caterpie–Butterfree | Hoothoot–Noctowl |
| 4 | 4% | Caterpie–Metapod | Venonat–Venomoth |
| 5 | 1% | Exeggcute | Caterpie–Butterfree |

#### Pewter City

Road, Kanto west.

**`MAP_PEWTER_CITY_HNS`**

#### Cerulean City

Road, Kanto east.

**`MAP_CERULEAN_CITY_HNS`**

Water type: ponds and rivers.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 5% | Goldeen–Seaking | Slowpoke–Slowbro |
| 4 | 4% | Poliwag–Poliwhirl | Slowpoke–Slowbro |
| 5 | 1% | Squirtle–Blastoise | Squirtle–Blastoise |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Slowpoke–Slowbro |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Vermilion City

Road, Kanto east.

**`MAP_VERMILION_CITY_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Krabby–Kingler | Chinchou–Lanturn |
| 3 | 5% | Tentacool–Tentacruel | Krabby–Kingler |
| 4 | 4% | Shellder | Staryu |
| 5 | 1% | Remoraid | Chinchou–Lanturn |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Chinchou–Lanturn | Chinchou–Lanturn |
| 3 | 10% | 12% | 11% | Horsea–Seadra | Horsea–Seadra |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Shellder | Shellder |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Horsea–Seadra | Staryu |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Krabby–Kingler | Krabby–Kingler |
| 2 | 30% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 3 | 5% | Geodude–Graveler | Geodude–Graveler |
| 4 | 4% | Spearow–Fearow | Spearow–Fearow |
| 5 | 1% | Caterpie–Butterfree | Caterpie–Butterfree |

**`MAP_VERMILION_CITY_PORT_OUTSIDE_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Remoraid–Octillery | Chinchou–Lanturn |
| 3 | 5% | Krabby–Kingler | Chinchou–Lanturn |
| 4 | 4% | Remoraid–Octillery | Krabby–Kingler |
| 5 | 1% | Staryu | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Remoraid–Octillery | Remoraid–Octillery |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Krabby–Kingler |
| 3 | 10% | 12% | 11% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Remoraid–Octillery | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Shellder | Shellder |
| 9 | 2% | 4% | 9% | Chinchou–Lanturn | Staryu |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Lavender Town

Road, Kanto east.

**`MAP_LAVENDER_TOWN_HNS`**

#### Celadon City

Road, Kanto east.

**`MAP_CELADON_CITY_HNS`**

Water type: ponds and rivers.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 30% | Slowpoke–Slowbro | Psyduck–Golduck |
| 3 | 5% | Psyduck–Golduck | Slowpoke–Slowbro |
| 4 | 4% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 1% | Goldeen–Seaking | Goldeen–Seaking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 5 | 8% | 9% | 10% | Slowpoke–Slowbro | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Saffron City

Road, Kanto east.

**`MAP_SAFFRON_CITY_HNS`**

#### Fuchsia City

Road, Kanto east.

**`MAP_FUCHSIA_CITY_HNS`**

Water type: ponds and rivers.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Goldeen–Seaking | Psyduck–Golduck |
| 3 | 5% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 4 | 4% | Psyduck–Golduck | Psyduck–Golduck |
| 5 | 1% | Poliwag–Poliwhirl | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Slowpoke–Slowbro | Psyduck–Golduck |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Venonat–Venomoth | Venonat–Venomoth |
| 2 | 30% | Exeggcute | Exeggcute |
| 3 | 5% | Paras–Parasect | Spinarak–Ariados |
| 4 | 4% | Weedle–Kakuna | Paras–Parasect |
| 5 | 1% | Caterpie–Butterfree | Hoothoot–Noctowl |

#### Cinnabar Island

Road, Kanto west.

**`MAP_CINNABAR_ISLAND`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Corsola | Staryu |
| 3 | 5% | Horsea–Seadra | Corsola |
| 4 | 4% | Shellder | Chinchou–Lanturn |
| 5 | 1% | Tentacool–Tentacruel | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Corsola | Corsola |
| 3 | 10% | 12% | 11% | Horsea–Seadra | Horsea–Seadra |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Shellder | Shellder |
| 7 | 3% | 6% | 10% | Tentacool–Tentacruel | Staryu |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 9 | 2% | 4% | 9% | Corsola | Chinchou–Lanturn |
| 10 | 2% | 4% | 9% | Horsea–Seadra | Horsea–Seadra |

#### Route 1

Road, Kanto west.

**`MAP_ROUTE1_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Pidgey–Pidgeot | Rattata–Raticate |
| 2 | 20% | Rattata–Raticate | Hoothoot–Noctowl |
| 3 | 10% | Sentret–Furret | Hoothoot–Noctowl |
| 4 | 10% | Pidgey–Pidgeot | Rattata–Raticate |
| 5 | 10% | Rattata–Raticate | Meowth–Persian |
| 6 | 10% | Sentret–Furret | Oddish–Gloom |
| 7 | 5% | Caterpie–Butterfree | Rattata |
| 8 | 5% | Ledyba–Ledian | Spinarak–Ariados |
| 9 | 4% | Nidoran♀–Nidorina | Venonat–Venomoth |
| 10 | 4% | Nidoran♂–Nidorino | Nidoran♂–Nidorino |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Igglybuff | Igglybuff |

#### Route 2

Road, Kanto west.

**`MAP_ROUTE2_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Caterpie–Butterfree | Caterpie–Butterfree |
| 2 | 20% | Weedle–Beedrill | Weedle–Beedrill |
| 3 | 10% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 4 | 10% | Rattata–Raticate | Spinarak–Ariados |
| 5 | 10% | Ledyba–Ledian | Venonat–Venomoth |
| 6 | 10% | Paras–Parasect | Rattata–Raticate |
| 7 | 5% | Pikachu | Paras–Parasect |
| 8 | 5% | Abra–Kadabra | Hoothoot–Noctowl |
| 9 | 4% | Yanma | Oddish–Gloom |
| 10 | 4% | Caterpie–Metapod | Murkrow |
| 11 | 1% | Abra–Kadabra | Abra–Kadabra |
| 12 | 1% | Pichu | Pichu |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 5% | Goldeen–Seaking | Marill–Azumarill |
| 4 | 4% | Psyduck–Golduck | Marill–Azumarill |
| 5 | 1% | Marill–Azumarill | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Slowpoke–Slowbro | Marill–Azumarill |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Slowpoke–Slowbro |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 3

Road, Kanto west.

**`MAP_ROUTE3_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Spearow–Fearow |
| 2 | 20% | Jigglypuff | Jigglypuff |
| 3 | 10% | Nidoran♀–Nidorina | Zubat–Golbat |
| 4 | 10% | Nidoran♂–Nidorino | Meowth–Persian |
| 5 | 10% | Mankey–Primeape | Nidoran♀–Nidorina |
| 6 | 10% | Sandshrew–Sandslash | Oddish–Gloom |
| 7 | 5% | Ekans–Arbok | Clefairy |
| 8 | 5% | Sunkern | Nidoran♂–Nidorino |
| 9 | 4% | Rattata–Raticate | Ekans–Arbok |
| 10 | 4% | Jigglypuff | Sandshrew–Sandslash |
| 11 | 1% | Charmander–Charizard | Charmander–Charizard |
| 12 | 1% | Igglybuff | Igglybuff |

#### Route 4

Road, Kanto west.

**`MAP_ROUTE4_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ekans–Arbok | Ekans–Arbok |
| 2 | 20% | Sandshrew–Sandslash | Sandshrew–Sandslash |
| 3 | 10% | Rattata–Raticate | Zubat–Golbat |
| 4 | 10% | Spearow–Fearow | Hoothoot–Noctowl |
| 5 | 10% | Mankey–Primeape | Meowth–Persian |
| 6 | 10% | Jigglypuff | Rattata–Raticate |
| 7 | 5% | Pidgey–Pidgeot | Oddish–Gloom |
| 8 | 5% | Geodude–Graveler | Mankey–Primeape |
| 9 | 4% | Charmander–Charizard | Charmander–Charizard |
| 10 | 4% | Nidoran♂–Nidorino | Jigglypuff |
| 11 | 1% | Abra–Kadabra | Clefairy |
| 12 | 1% | Clefairy | Abra–Kadabra |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Remoraid–Octillery | Chinchou–Lanturn |
| 3 | 5% | Tentacool–Tentacruel | Remoraid–Octillery |
| 4 | 4% | Shellder | Staryu |
| 5 | 1% | Krabby–Kingler | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Horsea–Seadra | Horsea–Seadra |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Remoraid–Octillery | Remoraid–Octillery |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Krabby–Kingler |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Shellder | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Remoraid–Octillery | Remoraid–Octillery |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 5

Road, Kanto east.

**`MAP_ROUTE5_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Pidgey–Pidgeot | Meowth–Persian |
| 2 | 20% | Meowth–Persian | Hoothoot–Noctowl |
| 3 | 10% | Jigglypuff | Oddish–Gloom |
| 4 | 10% | Abra–Kadabra | Venonat–Venomoth |
| 5 | 10% | Oddish–Gloom | Jigglypuff |
| 6 | 10% | Rattata–Raticate | Gastly–Haunter |
| 7 | 5% | Snubbull–Granbull | Abra–Kadabra |
| 8 | 5% | Abra–Kadabra | Snubbull–Granbull |
| 9 | 4% | Pidgey–Pidgeot | Drowzee |
| 10 | 4% | Jigglypuff | Oddish–Gloom |
| 11 | 1% | Mime Jr. | Mime Jr. |
| 12 | 1% | Eevee | Clefairy |

#### Route 6

Road, Kanto east.

**`MAP_ROUTE6_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Oddish–Gloom | Oddish–Gloom |
| 2 | 20% | Magnemite–Magneton | Magnemite–Magneton |
| 3 | 10% | Pidgey–Pidgeot | Meowth–Persian |
| 4 | 10% | Meowth–Persian | Venonat–Venomoth |
| 5 | 10% | Bellsprout–Weepinbell | Drowzee |
| 6 | 10% | Rattata–Raticate | Hoothoot–Noctowl |
| 7 | 5% | Abra–Kadabra | Abra–Kadabra |
| 8 | 5% | Snubbull–Granbull | Snubbull–Granbull |
| 9 | 4% | Farfetch'd | Farfetch'd |
| 10 | 4% | Jigglypuff | Gastly–Haunter |
| 11 | 1% | Abra–Kadabra | Abra–Kadabra |
| 12 | 1% | Elekid | Elekid |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Psyduck–Golduck | Slowpoke–Slowbro |
| 3 | 5% | Slowpoke–Slowbro | Goldeen–Seaking |
| 4 | 4% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 1% | Squirtle–Blastoise | Squirtle–Blastoise |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 5 | 8% | 9% | 10% | Slowpoke–Slowbro | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Caterpie–Butterfree | Caterpie–Butterfree |
| 2 | 30% | Weedle–Beedrill | Weedle–Beedrill |
| 3 | 5% | Ledyba–Ledian | Hoothoot–Noctowl |
| 4 | 4% | Caterpie–Metapod | Venonat–Venomoth |
| 5 | 1% | Exeggcute | Exeggcute |

#### Route 7

Road, Kanto east.

**`MAP_ROUTE7_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Meowth–Persian | Meowth–Persian |
| 2 | 20% | Vulpix | Vulpix |
| 3 | 10% | Growlithe | Murkrow |
| 4 | 10% | Pidgey–Pidgeot | Houndour |
| 5 | 10% | Jigglypuff | Gastly–Haunter |
| 6 | 10% | Oddish–Gloom | Growlithe |
| 7 | 5% | Snubbull–Granbull | Murkrow |
| 8 | 5% | Rattata–Raticate | Hoothoot–Noctowl |
| 9 | 4% | Abra–Kadabra | Abra–Kadabra |
| 10 | 4% | Eevee | Houndour |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Pichu | Pichu |

#### Route 8

Road, Kanto east.

**`MAP_ROUTE8_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Growlithe | Growlithe |
| 2 | 20% | Ekans–Arbok | Ekans–Arbok |
| 3 | 10% | Vulpix | Gastly–Haunter |
| 4 | 10% | Sandshrew–Sandslash | Gastly–Haunter |
| 5 | 10% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 6 | 10% | Meowth–Persian | Houndour |
| 7 | 5% | Abra–Kadabra | Meowth–Persian |
| 8 | 5% | Snubbull–Granbull | Snubbull–Granbull |
| 9 | 4% | Abra–Kadabra | Abra–Kadabra |
| 10 | 4% | Ponyta | Houndour |
| 11 | 1% | Growlithe | Vulpix |
| 12 | 1% | Ponyta | Gastly |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Goldeen–Seaking | Psyduck–Golduck |
| 3 | 5% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 4 | 4% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 5 | 1% | Psyduck–Golduck | Poliwag–Poliwhirl |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Slowpoke–Slowbro | Psyduck–Golduck |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 11

Road, Kanto east.

**`MAP_ROUTE11_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Drowzee | Drowzee |
| 2 | 20% | Spearow–Fearow | Spearow–Fearow |
| 3 | 10% | Rattata–Raticate | Rattata–Raticate |
| 4 | 10% | Ekans–Arbok | Hoothoot–Noctowl |
| 5 | 10% | Sandshrew–Sandslash | Meowth–Persian |
| 6 | 10% | Diglett | Oddish–Gloom |
| 7 | 5% | Magnemite–Magneton | Gastly–Haunter |
| 8 | 5% | Drowzee–Hypno | Drowzee–Hypno |
| 9 | 4% | Farfetch'd | Farfetch'd |
| 10 | 4% | Magnemite–Magneton | Venonat–Venomoth |
| 11 | 1% | Farfetch'd | Farfetch'd |
| 12 | 1% | Elekid | Elekid |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Spearow–Fearow |
| 2 | 30% | Spearow–Fearow | Spearow–Fearow |
| 3 | 5% | Weedle–Kakuna | Hoothoot–Noctowl |
| 4 | 4% | Pidgey–Pidgeot | Weedle–Kakuna |
| 5 | 1% | Exeggcute | Spinarak–Ariados |

#### Route 16

Road, Kanto east.

**`MAP_ROUTE16_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Spearow–Fearow |
| 2 | 20% | Doduo–Dodrio | Doduo–Dodrio |
| 3 | 10% | Rattata–Raticate | Grimer–Muk |
| 4 | 10% | Grimer | Murkrow |
| 5 | 10% | Doduo–Dodrio | Rattata–Raticate |
| 6 | 10% | Ponyta | Slugma–Magcargo |
| 7 | 5% | Spearow–Fearow | Hoothoot–Noctowl |
| 8 | 5% | Rattata–Raticate | Houndour |
| 9 | 4% | Lickitung | Lickitung |
| 10 | 4% | Ponyta–Rapidash | Murkrow |
| 11 | 1% | Lickitung | Lickitung |
| 12 | 1% | Eevee | Eevee |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 5% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 4 | 4% | Psyduck–Golduck | Goldeen–Seaking |
| 5 | 1% | Goldeen–Seaking | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp | Magikarp |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 4 | 8% | 10% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Slowpoke–Slowbro | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 17

Road, Kanto east.

**`MAP_ROUTE17_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Doduo–Dodrio | Doduo–Dodrio |
| 2 | 20% | Spearow–Fearow | Spearow–Fearow |
| 3 | 10% | Rattata–Raticate | Grimer–Muk |
| 4 | 10% | Ponyta | Murkrow |
| 5 | 10% | Spearow–Fearow | Slugma–Magcargo |
| 6 | 10% | Ponyta–Rapidash | Hoothoot–Noctowl |
| 7 | 5% | Doduo–Dodrio | Houndour |
| 8 | 5% | Grimer | Hoothoot–Noctowl |
| 9 | 4% | Rattata–Raticate | Grimer–Muk |
| 10 | 4% | Ponyta | Grimer |
| 11 | 1% | Lickitung | Lickitung |
| 12 | 1% | Farfetch'd | Farfetch'd |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Remoraid–Octillery | Remoraid–Octillery |
| 2 | 30% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 5% | Remoraid–Octillery | Chinchou–Lanturn |
| 4 | 4% | Shellder | Staryu |
| 5 | 1% | Krabby–Kingler | Shellder |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Remoraid–Octillery | Remoraid–Octillery |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Krabby–Kingler |
| 3 | 10% | 12% | 11% | Remoraid–Octillery | Remoraid–Octillery |
| 4 | 8% | 10% | 10% | Shellder | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Horsea–Seadra | Horsea–Seadra |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Krabby–Kingler | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Remoraid–Octillery | Staryu |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 18

Road, Kanto east.

**`MAP_ROUTE18_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Spearow–Fearow |
| 2 | 20% | Rattata–Raticate | Rattata–Raticate |
| 3 | 10% | Doduo–Dodrio | Grimer–Muk |
| 4 | 10% | Grimer | Murkrow |
| 5 | 10% | Spearow–Fearow | Slugma–Magcargo |
| 6 | 10% | Doduo–Dodrio | Hoothoot–Noctowl |
| 7 | 5% | Rattata–Raticate | Houndour |
| 8 | 5% | Lickitung | Lickitung |
| 9 | 4% | Grimer | Hoothoot–Noctowl |
| 10 | 4% | Grimer–Muk | Grimer |
| 11 | 1% | Lickitung | Lickitung |
| 12 | 1% | Eevee | Eevee |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Horsea–Seadra | Horsea–Seadra |
| 2 | 30% | Remoraid–Octillery | Tentacool–Tentacruel |
| 3 | 5% | Horsea–Seadra | Chinchou–Lanturn |
| 4 | 4% | Krabby–Kingler | Staryu |
| 5 | 1% | Tentacool–Tentacruel | Krabby–Kingler |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Horsea–Seadra | Horsea–Seadra |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Krabby–Kingler | Krabby–Kingler |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Remoraid–Octillery | Remoraid–Octillery |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Shellder | Staryu |
| 9 | 2% | 4% | 9% | Horsea–Seadra | Chinchou–Lanturn |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Hoothoot–Noctowl |
| 2 | 30% | Pidgey–Pidgeot | Spearow–Fearow |
| 3 | 5% | Caterpie–Butterfree | Venonat–Venomoth |
| 4 | 4% | Weedle–Beedrill | Weedle–Beedrill |
| 5 | 1% | Exeggcute | Exeggcute |

#### Route 21

Road, Kanto west.

**`MAP_ROUTE21_NORTH`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Tangela | Tangela |
| 2 | 20% | Tangela | Tangela |
| 3 | 10% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 4 | 10% | Exeggcute | Oddish–Gloom |
| 5 | 10% | Rattata–Raticate | Venonat–Venomoth |
| 6 | 10% | Sunkern | Exeggcute |
| 7 | 5% | Yanma | Spinarak–Ariados |
| 8 | 5% | Paras–Parasect | Paras–Parasect |
| 9 | 4% | Tangela | Tangela |
| 10 | 4% | Sentret–Furret | Oddish–Gloom |
| 11 | 1% | Bulbasaur–Ivysaur | Bulbasaur–Ivysaur |
| 12 | 1% | Tangela | Tangela |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Shellder | Staryu |
| 3 | 5% | Tentacool–Tentacruel | Shellder |
| 4 | 4% | Corsola | Chinchou–Lanturn |
| 5 | 1% | Horsea–Seadra | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Shellder | Shellder |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Shellder | Shellder |
| 4 | 8% | 10% | 10% | Qwilfish | Qwilfish |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Horsea–Seadra | Horsea–Seadra |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Krabby–Kingler | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Corsola | Staryu |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

**`MAP_ROUTE21_SOUTH`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Shellder | Staryu |
| 2 | 30% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 5% | Corsola | Chinchou–Lanturn |
| 4 | 4% | Horsea–Seadra | Corsola |
| 5 | 1% | Slowpoke–Slowbro | Chinchou–Lanturn |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Corsola | Corsola |
| 3 | 10% | 12% | 11% | Shellder | Shellder |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Remoraid–Octillery | Remoraid–Octillery |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Shellder | Staryu |
| 9 | 2% | 4% | 9% | Corsola | Chinchou–Lanturn |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 22

Road, Border.

**`MAP_ROUTE22_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rattata–Raticate | Rattata–Raticate |
| 2 | 20% | Mankey–Primeape | Mankey–Primeape |
| 3 | 10% | Spearow–Fearow | Hoothoot–Noctowl |
| 4 | 10% | Ponyta | Murkrow |
| 5 | 10% | Nidoran♀–Nidorina | Houndour |
| 6 | 10% | Nidoran♂–Nidorino | Nidoran♂–Nidorino |
| 7 | 5% | Doduo | Zubat–Golbat |
| 8 | 5% | Sentret–Furret | Spinarak–Ariados |
| 9 | 4% | Ledyba–Ledian | Hoothoot–Noctowl |
| 10 | 4% | Ponyta | Houndour |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Igglybuff | Murkrow |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Marill–Azumarill | Wooper–Quagsire |
| 3 | 5% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 4% | Psyduck–Golduck | Marill–Azumarill |
| 5 | 1% | Wooper–Quagsire | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Marill–Azumarill |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Wooper–Quagsire |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Hoothoot–Noctowl |
| 2 | 30% | Ledyba–Ledian | Spinarak–Ariados |
| 3 | 5% | Pidgey–Pidgeot | Spearow–Fearow |
| 4 | 4% | Caterpie–Butterfree | Venonat–Venomoth |
| 5 | 1% | Pineco–Forretress | Pineco–Forretress |

#### Route 24

Road, Kanto east.

**`MAP_ROUTE24_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| 2 | 20% | Caterpie–Butterfree | Oddish–Gloom |
| 3 | 10% | Abra–Kadabra | Venonat–Venomoth |
| 4 | 10% | Oddish–Gloom | Hoothoot–Noctowl |
| 5 | 10% | Pidgey–Pidgeot | Abra–Kadabra |
| 6 | 10% | Weedle–Beedrill | Paras–Parasect |
| 7 | 5% | Abra–Kadabra | Abra–Kadabra |
| 8 | 5% | Caterpie–Metapod | Spinarak–Ariados |
| 9 | 4% | Sunkern | Caterpie–Butterfree |
| 10 | 4% | Ledyba–Ledian | Meowth–Persian |
| 11 | 1% | Charmander–Charmeleon | Charmander–Charmeleon |
| 12 | 1% | Pichu | Pichu |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 5% | Psyduck–Golduck | Slowpoke–Slowbro |
| 4 | 4% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 5 | 1% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Slowpoke–Slowbro |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Slowpoke–Slowbro |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Viridian Forest

Road, Kanto west.

**`MAP_VIRIDIAN_FOREST_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Caterpie–Metapod | Caterpie–Metapod |
| 2 | 20% | Weedle–Kakuna | Weedle–Kakuna |
| 3 | 10% | Caterpie–Metapod | Spinarak |
| 4 | 10% | Weedle–Kakuna | Venonat |
| 5 | 10% | Paras–Parasect | Paras–Parasect |
| 6 | 10% | Pidgey–Pidgeotto | Hoothoot |
| 7 | 5% | Pikachu | Pikachu |
| 8 | 5% | Ledyba | Oddish–Gloom |
| 9 | 4% | Pikachu | Pikachu |
| 10 | 4% | Bulbasaur–Ivysaur | Bulbasaur–Ivysaur |
| 11 | 1% | Bulbasaur–Ivysaur | Bulbasaur–Ivysaur |
| 12 | 1% | Pichu | Pichu |

#### Route 12

Road, Kanto east.

**`MAP_ROUTE12_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Qwilfish | Qwilfish |
| 3 | 5% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 4 | 4% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 5 | 1% | Krabby–Kingler | Chinchou–Lanturn |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 22% | 18% | 10% | Qwilfish | Qwilfish |
| 3 | 10% | 12% | 11% | Qwilfish | Qwilfish |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Horsea–Seadra | Horsea–Seadra |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Slowpoke–Slowbro | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Qwilfish | Staryu |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Venonat–Venomoth | Venonat–Venomoth |
| 2 | 30% | Venonat–Venomoth | Venonat–Venomoth |
| 3 | 5% | Exeggcute | Hoothoot–Noctowl |
| 4 | 4% | Pidgey–Pidgeot | Exeggcute |
| 5 | 1% | Pineco | Spinarak–Ariados |

#### Route 13

Road, Kanto east.

**`MAP_ROUTE13_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| 2 | 20% | Oddish–Gloom | Oddish–Gloom |
| 3 | 10% | Pidgey–Pidgeot | Venonat–Venomoth |
| 4 | 10% | Venonat–Venomoth | Hoothoot–Noctowl |
| 5 | 10% | Nidoran♀–Nidorina | Gastly–Haunter |
| 6 | 10% | Nidoran♂–Nidorino | Drowzee–Hypno |
| 7 | 5% | Ditto | Ditto |
| 8 | 5% | Ditto | Spinarak–Ariados |
| 9 | 4% | Farfetch'd | Murkrow |
| 10 | 4% | Oddish–Gloom | Ditto |
| 11 | 1% | Ditto | Ditto |
| 12 | 1% | Pidgey–Pidgeot | Nidoran♂–Nidorino |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Qwilfish | Qwilfish |
| 2 | 30% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 5% | Qwilfish | Chinchou–Lanturn |
| 4 | 4% | Horsea–Seadra | Horsea–Seadra |
| 5 | 1% | Slowpoke–Slowbro | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Horsea–Seadra | Horsea–Seadra |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Qwilfish | Qwilfish |
| 4 | 8% | 10% | 10% | Corsola | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Krabby–Kingler |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Horsea–Seadra | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Qwilfish | Staryu |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 9

Wilds, Kanto east.

**`MAP_ROUTE9_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rattata–Raticate | Rattata–Raticate |
| 2 | 20% | Ekans–Arbok | Ekans–Arbok |
| 3 | 10% | Spearow–Fearow | Venonat–Venomoth |
| 4 | 10% | Sandshrew–Sandslash | Hoothoot–Noctowl |
| 5 | 10% | Rhyhorn | Rhyhorn |
| 6 | 10% | Rattata–Raticate | Zubat–Golbat |
| 7 | 5% | Mankey–Primeape | Gastly–Haunter |
| 8 | 5% | Kangaskhan | Kangaskhan |
| 9 | 4% | Onix | Onix |
| 10 | 4% | Sandshrew–Sandslash | Cubone–Marowak |
| 11 | 1% | Kangaskhan | Kangaskhan |
| 12 | 1% | Cubone–Marowak | Murkrow |

#### Route 10

Wilds, Kanto east.

**`MAP_ROUTE10_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Voltorb–Electrode | Voltorb–Electrode |
| 2 | 20% | Sandshrew–Sandslash | Sandshrew–Sandslash |
| 3 | 10% | Spearow–Fearow | Gastly–Haunter |
| 4 | 10% | Ekans–Arbok | Zubat–Golbat |
| 5 | 10% | Magnemite–Magneton | Hoothoot–Noctowl |
| 6 | 10% | Rhyhorn | Magnemite–Magneton |
| 7 | 5% | Electabuzz | Electabuzz |
| 8 | 5% | Cubone–Marowak | Houndour |
| 9 | 4% | Electabuzz | Electabuzz |
| 10 | 4% | Pikachu | Pikachu |
| 11 | 1% | Cubone–Marowak | Cubone–Marowak |
| 12 | 1% | Elekid | Elekid |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 5% | Psyduck–Golduck | Slowpoke–Slowbro |
| 4 | 4% | Goldeen–Seaking | Slowpoke–Slowbro |
| 5 | 1% | Poliwag–Poliwhirl | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Slowpoke–Slowbro |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 14

Wilds, Kanto east.

**`MAP_ROUTE14_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Oddish–Gloom | Oddish–Gloom |
| 2 | 20% | Venonat–Venomoth | Venonat–Venomoth |
| 3 | 10% | Bellsprout–Weepinbell | Hoothoot–Noctowl |
| 4 | 10% | Pidgey–Pidgeot | Gastly–Haunter |
| 5 | 10% | Nidoran♀–Nidorina | Spinarak–Ariados |
| 6 | 10% | Nidoran♂–Nidorino | Nidoran♂–Nidorino |
| 7 | 5% | Ditto | Ditto |
| 8 | 5% | Tauros | Drowzee–Hypno |
| 9 | 4% | Chansey | Chansey |
| 10 | 4% | Mr. Mime | Murkrow |
| 11 | 1% | Ditto | Ditto |
| 12 | 1% | Chansey | Chansey |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 30% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 5% | Slowpoke–Slowbro | Psyduck–Golduck |
| 4 | 4% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 5 | 1% | Psyduck–Golduck | Poliwag–Poliwhirl |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Slowpoke–Slowbro | Psyduck–Golduck |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Route 15

Wilds, Kanto east.

**`MAP_ROUTE15_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| 2 | 20% | Oddish–Gloom | Oddish–Gloom |
| 3 | 10% | Pidgey–Pidgeot | Venonat–Venomoth |
| 4 | 10% | Venonat–Venomoth | Hoothoot–Noctowl |
| 5 | 10% | Nidoran♀–Nidorina | Gastly–Haunter |
| 6 | 10% | Ditto | Ditto |
| 7 | 5% | Nidoran♂–Nidorino | Drowzee–Hypno |
| 8 | 5% | Scyther | Spinarak–Ariados |
| 9 | 4% | Kangaskhan | Kangaskhan |
| 10 | 4% | Chansey | Chansey |
| 11 | 1% | Ditto | Ditto |
| 12 | 1% | Scyther | Murkrow |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Goldeen–Seaking | Psyduck–Golduck |
| 4 | 4% | Slowpoke–Slowbro | Psyduck–Golduck |
| 5 | 1% | Poliwag–Poliwhirl | Slowpoke–Slowbro |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Psyduck–Golduck |
| 6 | 4% | 7% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Weedle–Beedrill | Hoothoot–Noctowl |
| 2 | 30% | Caterpie–Butterfree | Venonat–Venomoth |
| 3 | 5% | Pidgey–Pidgeot | Weedle–Beedrill |
| 4 | 4% | Exeggcute | Exeggcute |
| 5 | 1% | Spearow–Fearow | Spearow–Fearow |

#### Route 19

Wilds, Kanto east.

**`MAP_ROUTE19`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Horsea–Seadra | Staryu |
| 3 | 5% | Remoraid–Octillery | Chinchou–Lanturn |
| 4 | 4% | Shellder | Staryu |
| 5 | 1% | Squirtle–Wartortle | Squirtle–Wartortle |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Staryu |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Horsea–Seadra | Horsea–Seadra |
| 4 | 8% | 10% | 10% | Shellder | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Corsola | Corsola |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Krabby–Kingler | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Shellder | Staryu |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

#### Mt. Moon

Wilds, Kanto west.

**`MAP_MT_MOON_CAVE_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Zubat–Golbat | Zubat–Golbat |
| 2 | 20% | Geodude–Graveler | Geodude–Graveler |
| 3 | 10% | Zubat–Golbat | Zubat–Golbat |
| 4 | 10% | Paras–Parasect | Clefairy |
| 5 | 10% | Geodude–Graveler | Paras–Parasect |
| 6 | 10% | Sandshrew–Sandslash | Geodude–Graveler |
| 7 | 5% | Paras–Parasect | Clefairy |
| 8 | 5% | Onix | Onix |
| 9 | 4% | Clefairy | Clefairy |
| 10 | 4% | Marill–Azumarill | Marill–Azumarill |
| 11 | 1% | Clefairy | Cleffa |
| 12 | 1% | Cleffa | Cleffa |

#### Rock Tunnel

Wilds, Kanto east.

**`MAP_ROCK_TUNNEL_1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Zubat–Golbat | Zubat–Golbat |
| 2 | 20% | Geodude–Graveler | Geodude–Graveler |
| 3 | 10% | Machop–Machoke | Machop–Machoke |
| 4 | 10% | Onix | Onix |
| 5 | 10% | Zubat–Golbat | Zubat–Golbat |
| 6 | 10% | Mankey–Primeape | Gastly–Haunter |
| 7 | 5% | Onix | Onix |
| 8 | 5% | Geodude–Graveler | Dunsparce |
| 9 | 4% | Hitmonchan | Hitmonchan |
| 10 | 4% | Dunsparce | Dunsparce |
| 11 | 1% | Hitmonlee | Hitmonlee |
| 12 | 1% | Tyrogue | Tyrogue |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Geodude–Graveler |
| 2 | 30% | Geodude | Dunsparce |
| 3 | 5% | Dunsparce | Geodude |
| 4 | 4% | Geodude–Graveler | Geodude–Graveler |
| 5 | 1% | Shuckle | Shuckle |

**`MAP_ROCK_TUNNEL_B1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Onix | Onix |
| 2 | 20% | Machop–Machoke | Machop–Machoke |
| 3 | 10% | Geodude–Graveler | Geodude–Graveler |
| 4 | 10% | Zubat–Golbat | Zubat–Golbat |
| 5 | 10% | Onix | Gastly–Haunter |
| 6 | 10% | Cubone–Marowak | Cubone–Marowak |
| 7 | 5% | Geodude–Graveler | Misdreavus |
| 8 | 5% | Rhyhorn | Rhyhorn |
| 9 | 4% | Hitmonlee | Hitmonlee |
| 10 | 4% | Kangaskhan | Misdreavus |
| 11 | 1% | Hitmonchan | Hitmonchan |
| 12 | 1% | Tyrogue | Tyrogue |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Geodude–Graveler |
| 2 | 30% | Geodude | Dunsparce |
| 3 | 5% | Geodude–Graveler | Geodude |
| 4 | 4% | Dunsparce | Geodude–Graveler |
| 5 | 1% | Shuckle | Shuckle |

#### Diglett's Cave

Wilds, Kanto west.

**`MAP_DIGLETTS_CAVE_TUNNEL_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Diglett–Dugtrio | Diglett–Dugtrio |
| 2 | 20% | Diglett–Dugtrio | Diglett–Dugtrio |
| 3 | 10% | Diglett | Diglett |
| 4 | 10% | Geodude–Graveler | Zubat–Golbat |
| 5 | 10% | Diglett | Diglett |
| 6 | 10% | Sandshrew–Sandslash | Geodude–Graveler |
| 7 | 5% | Diglett–Dugtrio | Zubat–Golbat |
| 8 | 5% | Onix | Onix |
| 9 | 4% | Diglett | Diglett |
| 10 | 4% | Dunsparce | Dunsparce |
| 11 | 1% | Diglett–Dugtrio | Dunsparce |
| 12 | 1% | Dunsparce | Dunsparce |

#### Route 25

Wilds, Kanto east.

**`MAP_ROUTE25_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Abra–Kadabra | Abra–Kadabra |
| 2 | 20% | Weedle–Beedrill | Weedle–Beedrill |
| 3 | 10% | Oddish–Gloom | Venonat–Venomoth |
| 4 | 10% | Bellsprout–Weepinbell | Hoothoot–Noctowl |
| 5 | 10% | Pidgey–Pidgeot | Oddish–Gloom |
| 6 | 10% | Caterpie–Butterfree | Gastly–Haunter |
| 7 | 5% | Abra–Kadabra | Abra–Kadabra |
| 8 | 5% | Scyther | Spinarak–Ariados |
| 9 | 4% | Pinsir | Pinsir |
| 10 | 4% | Bulbasaur–Ivysaur | Bulbasaur–Ivysaur |
| 11 | 1% | Bulbasaur–Ivysaur | Bulbasaur–Ivysaur |
| 12 | 1% | Scyther | Scyther |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowbro | Staryu |
| 2 | 30% | Krabby–Kingler | Slowpoke–Slowbro |
| 3 | 5% | Shellder | Chinchou–Lanturn |
| 4 | 4% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 5 | 1% | Corsola | Chinchou–Lanturn |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Staryu |
| 2 | 22% | 18% | 10% | Slowpoke–Slowbro | Krabby |
| 3 | 10% | 12% | 11% | Krabby | Slowpoke–Slowbro |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Shellder | Shellder |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Krabby–Kingler | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Corsola | Staryu |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Caterpie–Butterfree | Hoothoot–Noctowl |
| 2 | 30% | Weedle–Beedrill | Venonat–Venomoth |
| 3 | 5% | Pidgey–Pidgeot | Weedle–Beedrill |
| 4 | 4% | Exeggcute | Exeggcute |
| 5 | 1% | Pinsir | Pinsir |

#### Route 20

Outlands, Kanto west.

**`MAP_ROUTE20`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Staryu–Starmie | Staryu–Starmie |
| 3 | 5% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 4 | 4% | Shellder–Cloyster | Chinchou–Lanturn |
| 5 | 1% | Lapras | Lapras |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Shellder–Cloyster | Shellder–Cloyster |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Horsea–Kingdra | Horsea–Kingdra |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Staryu–Starmie | Staryu–Starmie |
| 7 | 3% | 6% | 10% | Dratini–Dragonair | Dratini–Dragonair |
| 8 | 3% | 5% | 9% | Shellder–Cloyster | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 10 | 2% | 4% | 9% | Dratini–Dragonair | Dratini–Dragonair |

#### Route 23

Outlands, Border.

**`MAP_ROUTE23_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Spearow–Fearow |
| 2 | 20% | Mankey–Primeape | Mankey–Primeape |
| 3 | 10% | Ekans–Arbok | Zubat–Golbat |
| 4 | 10% | Sandshrew–Sandslash | Houndour–Houndoom |
| 5 | 10% | Gligar | Teddiursa–Ursaring |
| 6 | 10% | Phanpy–Donphan | Gligar |
| 7 | 5% | Rhyhorn–Rhydon | Murkrow |
| 8 | 5% | Skarmory | Sneasel |
| 9 | 4% | Tauros | Sandshrew–Sandslash |
| 10 | 4% | Teddiursa–Ursaring | Skarmory |
| 11 | 1% | Snorlax | Snorlax |
| 12 | 1% | Mankey–Annihilape | Mankey–Annihilape |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 3 | 5% | Wooper–Quagsire | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Poliwag–Poliwhirl |
| 5 | 1% | Poliwag–Politoed | Poliwag–Politoed |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Poliwag–Politoed | Wooper–Quagsire |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Dratini–Dragonair | Dratini–Dragonair |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Hoothoot–Noctowl |
| 2 | 30% | Ledyba–Ledian | Spinarak–Ariados |
| 3 | 5% | Pidgey–Pidgeot | Spearow–Fearow |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Heracross | Heracross |

#### Pokémon Tower

Dungeon, Kanto east.

**`MAP_POKEMON_TOWER_3F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gastly–Haunter | Gastly–Haunter |
| 2 | 20% | Gastly–Haunter | Gastly–Haunter |
| 3 | 10% | Cubone–Marowak | Cubone–Marowak |
| 4 | 10% | Gastly–Haunter | Misdreavus |
| 5 | 10% | Cubone–Marowak | Cubone–Marowak |
| 6 | 10% | Gastly–Haunter | Gastly–Haunter |
| 7 | 5% | Misdreavus | Misdreavus |
| 8 | 5% | Gastly–Haunter | Murkrow |
| 9 | 4% | Cubone–Marowak | Cubone–Marowak |
| 10 | 4% | Misdreavus | Misdreavus |
| 11 | 1% | Gastly–Haunter | Gastly–Haunter |
| 12 | 1% | Misdreavus | Misdreavus |

**`MAP_POKEMON_TOWER_4F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gastly–Haunter | Gastly–Haunter |
| 2 | 20% | Cubone–Marowak | Cubone–Marowak |
| 3 | 10% | Gastly–Haunter | Gastly–Haunter |
| 4 | 10% | Cubone–Marowak | Misdreavus |
| 5 | 10% | Gastly–Haunter | Misdreavus |
| 6 | 10% | Misdreavus | Gastly–Haunter |
| 7 | 5% | Gastly–Haunter | Murkrow |
| 8 | 5% | Cubone–Marowak | Cubone–Marowak |
| 9 | 4% | Misdreavus | Misdreavus |
| 10 | 4% | Gastly–Haunter | Gastly–Gengar |
| 11 | 1% | Misdreavus | Gastly–Gengar |
| 12 | 1% | Gastly–Gengar | Misdreavus–Mismagius |

**`MAP_POKEMON_TOWER_5F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gastly–Haunter | Gastly–Haunter |
| 2 | 20% | Gastly–Haunter | Gastly–Haunter |
| 3 | 10% | Cubone–Marowak | Cubone–Marowak |
| 4 | 10% | Gastly–Haunter | Misdreavus |
| 5 | 10% | Cubone–Marowak | Misdreavus |
| 6 | 10% | Misdreavus | Gastly–Haunter |
| 7 | 5% | Gastly–Gengar | Gastly–Gengar |
| 8 | 5% | Cubone–Marowak | Murkrow |
| 9 | 4% | Misdreavus | Misdreavus–Mismagius |
| 10 | 4% | Gastly–Gengar | Gastly–Gengar |
| 11 | 1% | Misdreavus–Mismagius | Misdreavus–Mismagius |
| 12 | 1% | Gastly–Gengar | Gastly–Gengar |

**`MAP_POKEMON_TOWER_6F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gastly–Haunter | Gastly–Haunter |
| 2 | 20% | Cubone–Marowak | Cubone–Marowak |
| 3 | 10% | Gastly–Haunter | Gastly–Haunter |
| 4 | 10% | Cubone–Marowak | Misdreavus |
| 5 | 10% | Gastly–Gengar | Gastly–Gengar |
| 6 | 10% | Misdreavus | Misdreavus |
| 7 | 5% | Gastly–Gengar | Murkrow |
| 8 | 5% | Cubone–Marowak | Cubone–Marowak |
| 9 | 4% | Misdreavus | Misdreavus–Mismagius |
| 10 | 4% | Gastly–Gengar | Gastly–Gengar |
| 11 | 1% | Misdreavus–Mismagius | Murkrow–Honchkrow |
| 12 | 1% | Gastly–Gengar | Misdreavus–Mismagius |

**`MAP_POKEMON_TOWER_7F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gastly–Gengar | Gastly–Gengar |
| 2 | 20% | Cubone–Marowak | Cubone–Marowak |
| 3 | 10% | Gastly–Haunter | Gastly–Haunter |
| 4 | 10% | Cubone–Marowak | Misdreavus |
| 5 | 10% | Gastly–Gengar | Gastly–Gengar |
| 6 | 10% | Misdreavus | Misdreavus–Mismagius |
| 7 | 5% | Gastly–Haunter | Murkrow–Honchkrow |
| 8 | 5% | Misdreavus–Mismagius | Misdreavus–Mismagius |
| 9 | 4% | Cubone–Marowak | Cubone–Marowak |
| 10 | 4% | Gastly–Gengar | Gastly–Gengar |
| 11 | 1% | Misdreavus–Mismagius | Murkrow–Honchkrow |
| 12 | 1% | Gastly–Gengar | Gastly–Gengar |

#### Pokémon Mansion

Dungeon, Kanto west.

**`MAP_POKEMON_MANSION_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Grimer–Muk | Grimer–Muk |
| 2 | 20% | Koffing–Weezing | Koffing–Weezing |
| 3 | 10% | Rattata–Raticate | Rattata–Raticate |
| 4 | 10% | Growlithe | Growlithe |
| 5 | 10% | Vulpix | Vulpix |
| 6 | 10% | Slugma–Magcargo | Gastly–Haunter |
| 7 | 5% | Ponyta | Houndour |
| 8 | 5% | Koffing | Slugma–Magcargo |
| 9 | 4% | Charmander–Charmeleon | Charmander–Charmeleon |
| 10 | 4% | Rattata–Raticate | Houndour |
| 11 | 1% | Magby | Magby |
| 12 | 1% | Charmander–Charmeleon | Charmander–Charmeleon |

**`MAP_POKEMON_MANSION_2F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Growlithe | Growlithe |
| 2 | 20% | Vulpix | Vulpix |
| 3 | 10% | Grimer–Muk | Grimer–Muk |
| 4 | 10% | Koffing–Weezing | Koffing–Weezing |
| 5 | 10% | Slugma–Magcargo | Gastly–Haunter |
| 6 | 10% | Ponyta–Rapidash | Houndour |
| 7 | 5% | Rattata–Raticate | Slugma–Magcargo |
| 8 | 5% | Ponyta | Ponyta–Rapidash |
| 9 | 4% | Charmander–Charmeleon | Charmander–Charmeleon |
| 10 | 4% | Magmar | Magmar |
| 11 | 1% | Magby | Magby |
| 12 | 1% | Ditto | Ditto |

**`MAP_POKEMON_MANSION_3F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Koffing–Weezing | Koffing–Weezing |
| 2 | 20% | Grimer–Muk | Grimer–Muk |
| 3 | 10% | Growlithe–Arcanine | Growlithe–Arcanine |
| 4 | 10% | Vulpix–Ninetales | Vulpix–Ninetales |
| 5 | 10% | Slugma–Magcargo | Gastly–Haunter |
| 6 | 10% | Ponyta–Rapidash | Houndour–Houndoom |
| 7 | 5% | Magmar | Magmar |
| 8 | 5% | Rattata–Raticate | Slugma–Magcargo |
| 9 | 4% | Charmander–Charmeleon | Charmander–Charmeleon |
| 10 | 4% | Ditto | Ditto |
| 11 | 1% | Ditto | Ditto |
| 12 | 1% | Magby | Magby |

**`MAP_POKEMON_MANSION_B1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ditto | Ditto |
| 2 | 20% | Grimer–Muk | Grimer–Muk |
| 3 | 10% | Koffing–Weezing | Koffing–Weezing |
| 4 | 10% | Ditto | Ditto |
| 5 | 10% | Growlithe–Arcanine | Growlithe–Arcanine |
| 6 | 10% | Vulpix–Ninetales | Gastly–Gengar |
| 7 | 5% | Magmar | Magmar |
| 8 | 5% | Ponyta–Rapidash | Houndour–Houndoom |
| 9 | 4% | Charmander–Charizard | Charmander–Charizard |
| 10 | 4% | Porygon | Porygon |
| 11 | 1% | Aerodactyl | Aerodactyl |
| 12 | 1% | Magmar–Magmortar | Magmar–Magmortar |

#### Power Plant

Dungeon, Kanto east.

**`MAP_POWER_PLANT`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Magnemite–Magneton | Magnemite–Magneton |
| 2 | 20% | Voltorb | Voltorb |
| 3 | 10% | Magnemite–Magneton | Magnemite–Magneton |
| 4 | 10% | Voltorb–Electrode | Voltorb–Electrode |
| 5 | 10% | Pikachu | Pikachu |
| 6 | 10% | Electabuzz | Electabuzz |
| 7 | 5% | Pikachu | Pikachu |
| 8 | 5% | Magnemite–Magnezone | Magnemite–Magnezone |
| 9 | 4% | Electabuzz | Elekid |
| 10 | 4% | Porygon | Porygon |
| 11 | 1% | Pikachu–Raichu | Pikachu–Raichu |
| 12 | 1% | Electabuzz–Electivire | Electabuzz–Electivire |

#### Seafoam Islands

Dungeon, Kanto west.

**`MAP_SEAFOAM_ISLANDS_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 10% | Zubat–Golbat | Zubat–Golbat |
| 4 | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 5 | 10% | Seel–Dewgong | Sneasel |
| 6 | 10% | Swinub | Zubat–Golbat |
| 7 | 5% | Zubat–Golbat | Swinub |
| 8 | 5% | Seel–Dewgong | Seel–Dewgong |
| 9 | 4% | Slowpoke–Slowbro | Sneasel |
| 10 | 4% | Smoochum | Smoochum |
| 11 | 1% | Jynx | Jynx |
| 12 | 1% | Smoochum | Smoochum |

**`MAP_SEAFOAM_ISLANDS_B1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 4 | 10% | Jynx | Jynx |
| 5 | 10% | Seel–Dewgong | Sneasel |
| 6 | 10% | Swinub | Swinub |
| 7 | 5% | Delibird | Delibird |
| 8 | 5% | Zubat–Golbat | Zubat–Crobat |
| 9 | 4% | Slowpoke–Slowbro | Sneasel |
| 10 | 4% | Psyduck–Golduck | Zubat–Golbat |
| 11 | 1% | Smoochum | Smoochum |
| 12 | 1% | Jynx | Jynx |

**`MAP_SEAFOAM_ISLANDS_B2F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Zubat–Crobat | Zubat–Crobat |
| 3 | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 4 | 10% | Jynx | Jynx |
| 5 | 10% | Swinub–Piloswine | Sneasel |
| 6 | 10% | Delibird | Swinub–Piloswine |
| 7 | 5% | Psyduck–Golduck | Delibird |
| 8 | 5% | Seel–Dewgong | Sneasel |
| 9 | 4% | Jynx | Jynx |
| 10 | 4% | Swinub | Sneasel–Weavile |
| 11 | 1% | Slowpoke–Slowking | Slowpoke–Slowking |
| 12 | 1% | Smoochum | Sneasel–Weavile |

**`MAP_SEAFOAM_ISLANDS_B3F`**

Water type: cold water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Jynx | Jynx |
| 3 | 10% | Seel–Dewgong | Seel–Dewgong |
| 4 | 10% | Seel | Seel |
| 5 | 10% | Swinub–Piloswine | Sneasel |
| 6 | 10% | Slowpoke–Slowbro | Swinub–Piloswine |
| 7 | 5% | Delibird | Sneasel–Weavile |
| 8 | 5% | Psyduck–Golduck | Psyduck–Golduck |
| 9 | 4% | Zubat–Crobat | Zubat–Crobat |
| 10 | 4% | Slowpoke–Slowking | Slowpoke–Slowking |
| 11 | 1% | Smoochum | Smoochum |
| 12 | 1% | Swinub–Mamoswine | Swinub–Mamoswine |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Seel–Dewgong | Seel–Dewgong |
| 2 | 30% | Seel–Dewgong | Shellder–Cloyster |
| 3 | 5% | Slowpoke–Slowbro | Seel–Dewgong |
| 4 | 4% | Shellder–Cloyster | Horsea–Seadra |
| 5 | 1% | Lapras | Lapras |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Shellder | Shellder |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Horsea–Seadra |
| 3 | 10% | 12% | 11% | Krabby–Kingler | Krabby–Kingler |
| 4 | 8% | 10% | 10% | Seel–Dewgong | Slowpoke–Slowbro |
| 5 | 8% | 9% | 10% | Magikarp | Magikarp |
| 6 | 4% | 7% | 10% | Shellder | Shellder |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Slowpoke–Slowbro |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 10 | 2% | 4% | 9% | Horsea–Seadra | Horsea–Seadra |

**`MAP_SEAFOAM_ISLANDS_B4F`**

Water type: cold water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Jynx | Jynx |
| 3 | 10% | Seel | Seel |
| 4 | 10% | Slowpoke–Slowking | Slowpoke–Slowking |
| 5 | 10% | Swinub–Piloswine | Sneasel |
| 6 | 10% | Delibird | Sneasel–Weavile |
| 7 | 5% | Seel–Dewgong | Seel–Dewgong |
| 8 | 5% | Jynx | Jynx |
| 9 | 4% | Swinub–Mamoswine | Swinub–Mamoswine |
| 10 | 4% | Zubat–Crobat | Zubat–Crobat |
| 11 | 1% | Swinub–Mamoswine | Swinub–Mamoswine |
| 12 | 1% | Sneasel–Weavile | Sneasel–Weavile |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Seel–Dewgong | Seel–Dewgong |
| 2 | 30% | Shellder–Cloyster | Shellder–Cloyster |
| 3 | 5% | Horsea–Seadra | Slowpoke–Slowking |
| 4 | 4% | Horsea–Kingdra | Horsea–Kingdra |
| 5 | 1% | Lapras | Lapras |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Seel–Dewgong | Seel–Dewgong |
| 2 | 22% | 18% | 10% | Shellder–Cloyster | Shellder–Cloyster |
| 3 | 10% | 12% | 11% | Horsea–Seadra | Horsea–Seadra |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Krabby–Kingler |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Shellder–Cloyster | Shellder–Cloyster |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Horsea–Seadra |
| 8 | 3% | 5% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 9 | 2% | 4% | 9% | Seel–Dewgong | Seel–Dewgong |
| 10 | 2% | 4% | 9% | Psyduck–Golduck | Slowpoke–Slowking |

#### Victory Road

Dungeon, Border.

**`MAP_VICTORY_ROAD_KANTO_1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Machop–Machoke | Machop–Machoke |
| 2 | 20% | Geodude–Graveler | Geodude–Graveler |
| 3 | 10% | Onix | Onix |
| 4 | 10% | Zubat–Golbat | Zubat–Golbat |
| 5 | 10% | Cubone–Marowak | Misdreavus |
| 6 | 10% | Gligar | Gligar |
| 7 | 5% | Sandshrew–Sandslash | Sandshrew–Sandslash |
| 8 | 5% | Machop–Machoke | Misdreavus |
| 9 | 4% | Hitmonlee | Hitmonlee |
| 10 | 4% | Hitmonchan | Hitmonchan |
| 11 | 1% | Tyrogue | Tyrogue |
| 12 | 1% | Gligar | Misdreavus |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Geodude–Graveler |
| 2 | 30% | Sandshrew–Sandslash | Dunsparce |
| 3 | 5% | Geodude | Sandshrew–Sandslash |
| 4 | 4% | Shuckle | Shuckle |
| 5 | 1% | Dunsparce | Geodude |

**`MAP_VICTORY_ROAD_KANTO_B1F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Onix | Onix |
| 2 | 20% | Zubat–Golbat | Zubat–Golbat |
| 3 | 10% | Geodude–Graveler | Geodude–Graveler |
| 4 | 10% | Machop–Machoke | Machop–Machoke |
| 5 | 10% | Rhyhorn–Rhydon | Misdreavus |
| 6 | 10% | Cubone–Marowak | Cubone–Marowak |
| 7 | 5% | Onix–Steelix | Zubat–Crobat |
| 8 | 5% | Mankey–Primeape | Mankey–Primeape |
| 9 | 4% | Hitmontop | Hitmontop |
| 10 | 4% | Machop–Machamp | Machop–Machamp |
| 11 | 1% | Aerodactyl | Aerodactyl |
| 12 | 1% | Hitmonlee | Hitmonchan |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 30% | Psyduck–Golduck | Zubat–Crobat |
| 3 | 5% | Slowpoke–Slowbro | Psyduck–Golduck |
| 4 | 4% | Poliwag–Poliwhirl | Zubat–Crobat |
| 5 | 1% | Slowpoke–Slowking | Slowpoke–Slowking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Psyduck–Golduck | Psyduck–Golduck |
| 4 | 8% | 10% | 10% | Magikarp | Magikarp |
| 5 | 8% | 9% | 10% | Slowpoke–Slowbro | Zubat–Crobat |
| 6 | 4% | 7% | 10% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Zubat–Crobat |
| 9 | 2% | 4% | 9% | Poliwag–Politoed | Poliwag–Politoed |
| 10 | 2% | 4% | 9% | Dratini–Dragonair | Dratini–Dragonair |

**`MAP_VICTORY_ROAD_KANTO_B2F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Machop–Machamp | Machop–Machamp |
| 2 | 20% | Cubone–Marowak | Cubone–Marowak |
| 3 | 10% | Geodude–Golem | Geodude–Golem |
| 4 | 10% | Onix–Steelix | Onix–Steelix |
| 5 | 10% | Rhyhorn–Rhydon | Zubat–Crobat |
| 6 | 10% | Sandshrew–Sandslash | Sandshrew–Sandslash |
| 7 | 5% | Gligar–Gliscor | Gligar–Gliscor |
| 8 | 5% | Rhyhorn–Rhyperior | Misdreavus–Mismagius |
| 9 | 4% | Aerodactyl | Aerodactyl |
| 10 | 4% | Snorlax | Snorlax |
| 11 | 1% | Aerodactyl | Aerodactyl |
| 12 | 1% | Hitmontop | Hitmontop |

#### Cerulean Cave

Dungeon, Kanto east.

**`MAP_CERULEAN_CAVE_1F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Drowzee–Hypno | Drowzee–Hypno |
| 2 | 20% | Magnemite–Magneton | Magnemite–Magneton |
| 3 | 10% | Zubat–Golbat | Zubat–Golbat |
| 4 | 10% | Venonat–Venomoth | Venonat–Venomoth |
| 5 | 10% | Doduo–Dodrio | Wobbuffet |
| 6 | 10% | Paras–Parasect | Paras–Parasect |
| 7 | 5% | Abra–Kadabra | Abra–Kadabra |
| 8 | 5% | Sandshrew–Sandslash | Gastly–Haunter |
| 9 | 4% | Ditto | Ditto |
| 10 | 4% | Wobbuffet | Wobbuffet |
| 11 | 1% | Mr. Mime | Mr. Mime |
| 12 | 1% | Pikachu | Misdreavus |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Slowpoke–Slowbro | Zubat–Golbat |
| 3 | 5% | Psyduck–Golduck | Slowpoke–Slowbro |
| 4 | 4% | Goldeen–Seaking | Zubat–Golbat |
| 5 | 1% | Poliwag–Poliwhirl | Goldeen–Seaking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 4 | 8% | 10% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Zubat–Golbat |
| 6 | 4% | 7% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Zubat–Golbat |
| 9 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Geodude–Graveler |
| 2 | 30% | Geodude–Graveler | Geodude–Graveler |
| 3 | 5% | Sandshrew–Sandslash | Dunsparce |
| 4 | 4% | Geodude–Graveler | Sandshrew–Sandslash |
| 5 | 1% | Shuckle | Shuckle |

**`MAP_CERULEAN_CAVE_B1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rhyhorn–Rhydon | Rhyhorn–Rhydon |
| 2 | 20% | Zubat–Golbat | Zubat–Golbat |
| 3 | 10% | Magnemite–Magneton | Magnemite–Magneton |
| 4 | 10% | Voltorb–Electrode | Voltorb–Electrode |
| 5 | 10% | Cubone–Marowak | Zubat–Crobat |
| 6 | 10% | Ditto | Ditto |
| 7 | 5% | Chansey | Chansey |
| 8 | 5% | Jigglypuff–Wigglytuff | Wobbuffet |
| 9 | 4% | Abra–Alakazam | Abra–Alakazam |
| 10 | 4% | Ditto | Ditto |
| 11 | 1% | Kangaskhan | Kangaskhan |
| 12 | 1% | Chansey–Blissey | Chansey–Blissey |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Golem | Geodude–Golem |
| 2 | 30% | Geodude–Graveler | Geodude–Graveler |
| 3 | 5% | Geodude–Graveler | Dunsparce |
| 4 | 4% | Shuckle | Shuckle |
| 5 | 1% | Geodude–Golem | Geodude–Golem |

**`MAP_CERULEAN_CAVE_B2F_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Ditto | Ditto |
| 2 | 20% | Abra–Alakazam | Abra–Alakazam |
| 3 | 10% | Magnemite–Magnezone | Magnemite–Magnezone |
| 4 | 10% | Rhyhorn–Rhyperior | Rhyhorn–Rhyperior |
| 5 | 10% | Lickitung–Lickilicky | Zubat–Crobat |
| 6 | 10% | Voltorb–Electrode | Gastly–Gengar |
| 7 | 5% | Chansey | Chansey |
| 8 | 5% | Chansey–Blissey | Chansey–Blissey |
| 9 | 4% | Snorlax | Snorlax |
| 10 | 4% | Aerodactyl | Aerodactyl |
| 11 | 1% | Jigglypuff–Wigglytuff | Jigglypuff–Wigglytuff |
| 12 | 1% | Mr. Mime | Mr. Mime |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Slowpoke–Slowking | Slowpoke–Slowking |
| 2 | 30% | Psyduck–Golduck | Zubat–Crobat |
| 3 | 5% | Slowpoke–Slowking | Psyduck–Golduck |
| 4 | 4% | Poliwag–Politoed | Zubat–Crobat |
| 5 | 1% | Marill–Azumarill | Poliwag–Politoed |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Slowpoke–Slowking | Slowpoke–Slowking |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 4 | 8% | 10% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Zubat–Crobat |
| 6 | 4% | 7% | 10% | Poliwag–Politoed | Poliwag–Politoed |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Zubat–Crobat |
| 9 | 2% | 4% | 9% | Dratini–Dragonair | Dratini–Dragonair |
| 10 | 2% | 4% | 9% | Dratini–Dragonite | Dratini–Dragonite |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Golem | Geodude–Golem |
| 2 | 30% | Geodude–Golem | Geodude–Golem |
| 3 | 5% | Geodude–Graveler | Dunsparce |
| 4 | 4% | Shuckle | Shuckle |
| 5 | 1% | Geodude–Golem | Geodude–Graveler |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Abra | Cerulean Cave, Route 2, Route 24, Route 25 and more |
| Aerodactyl | Cerulean Cave, Pokémon Mansion, Victory Road |
| Alakazam | Cerulean Cave |
| Annihilape | Route 23 |
| Arbok | Route 10, Route 11, Route 23, Route 3 and more |
| Arcanine | Pokémon Mansion |
| Ariados | Fuchsia City, Route 1, Route 11, Route 12 and more |
| Azumarill | Cerulean Cave, Mt. Moon, Route 2, Route 22 and more |
| Beedrill | Route 15, Route 18, Route 2, Route 24 and more |
| Bellsprout | Route 13, Route 14, Route 15, Route 24 and more |
| Blastoise | Cerulean City, Pallet Town, Route 6 |
| Blissey | Cerulean Cave |
| Bulbasaur | Route 21, Route 25, Viridian Forest |
| Butterfree | Fuchsia City, Route 1, Route 15, Route 18 and more |
| Caterpie | Fuchsia City, Route 1, Route 15, Route 18 and more |
| Chansey | Cerulean Cave, Route 14, Route 15 |
| Charizard | Pokémon Mansion, Route 3, Route 4 |
| Charmander | Pokémon Mansion, Route 24, Route 3, Route 4 |
| Charmeleon | Pokémon Mansion, Route 24, Route 3, Route 4 |
| Chinchou | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Clefairy | Mt. Moon, Route 3, Route 4, Route 5 |
| Cleffa | Mt. Moon |
| Cloyster | Route 20, Seafoam Islands |
| Corsola | Cinnabar Island, Route 13, Route 19, Route 21 and more |
| Crobat | Cerulean Cave, Seafoam Islands, Victory Road |
| Cubone | Cerulean Cave, Pokémon Tower, Rock Tunnel, Route 10 and more |
| Delibird | Seafoam Islands |
| Dewgong | Seafoam Islands |
| Diglett | Diglett's Cave, Route 11 |
| Ditto | Cerulean Cave, Pokémon Mansion, Route 13, Route 14 and more |
| Dodrio | Cerulean Cave, Route 16, Route 17, Route 18 |
| Doduo | Cerulean Cave, Route 16, Route 17, Route 18 and more |
| Donphan | Route 23 |
| Dragonair | Cerulean Cave, Route 20, Route 23, Victory Road |
| Dragonite | Cerulean Cave |
| Dratini | Cerulean Cave, Route 20, Route 23, Victory Road |
| Drowzee | Cerulean Cave, Route 11, Route 13, Route 14 and more |
| Dugtrio | Diglett's Cave |
| Dunsparce | Cerulean Cave, Diglett's Cave, Rock Tunnel, Victory Road |
| Eevee | Route 1, Route 16, Route 18, Route 22 and more |
| Ekans | Route 10, Route 11, Route 23, Route 3 and more |
| Electabuzz | Power Plant, Route 10 |
| Electivire | Power Plant |
| Electrode | Cerulean Cave, Power Plant, Route 10 |
| Elekid | Power Plant, Route 10, Route 11, Route 6 |
| Exeggcute | Fuchsia City, Route 11, Route 12, Route 15 and more |
| Farfetch'd | Route 11, Route 13, Route 17, Route 6 |
| Fearow | Route 10, Route 11, Route 15, Route 16 and more |
| Forretress | Route 22, Route 23 |
| Furret | Route 1, Route 21, Route 22 |
| Gastly | Cerulean Cave, Pokémon Mansion, Pokémon Tower, Rock Tunnel and more |
| Gengar | Cerulean Cave, Pokémon Mansion, Pokémon Tower |
| Geodude | Cerulean Cave, Diglett's Cave, Mt. Moon, Rock Tunnel and more |
| Gligar | Route 23, Victory Road |
| Gliscor | Victory Road |
| Gloom | Route 1, Route 11, Route 13, Route 14 and more |
| Golbat | Cerulean Cave, Diglett's Cave, Mt. Moon, Rock Tunnel and more |
| Goldeen | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Golduck | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Golem | Cerulean Cave, Victory Road |
| Granbull | Route 5, Route 6, Route 7, Route 8 |
| Graveler | Cerulean Cave, Diglett's Cave, Mt. Moon, Rock Tunnel and more |
| Grimer | Pokémon Mansion, Route 16, Route 17, Route 18 |
| Growlithe | Pokémon Mansion, Route 7, Route 8 |
| Gyarados | Celadon City, Cerulean Cave, Cerulean City, Cinnabar Island and more |
| Haunter | Cerulean Cave, Pokémon Mansion, Pokémon Tower, Rock Tunnel and more |
| Heracross | Route 23 |
| Hitmonchan | Rock Tunnel, Victory Road |
| Hitmonlee | Rock Tunnel, Victory Road |
| Hitmontop | Victory Road |
| Honchkrow | Pokémon Tower |
| Hoothoot | Fuchsia City, Route 1, Route 10, Route 11 and more |
| Horsea | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Houndoom | Pokémon Mansion, Route 23 |
| Houndour | Pokémon Mansion, Route 10, Route 16, Route 17 and more |
| Hypno | Cerulean Cave, Route 11, Route 13, Route 14 and more |
| Igglybuff | Route 1, Route 22, Route 3 |
| Ivysaur | Route 21, Route 25, Viridian Forest |
| Jigglypuff | Cerulean Cave, Route 3, Route 4, Route 5 and more |
| Jynx | Seafoam Islands |
| Kadabra | Cerulean Cave, Route 2, Route 24, Route 25 and more |
| Kakuna | Fuchsia City, Route 11, Route 15, Route 18 and more |
| Kangaskhan | Cerulean Cave, Rock Tunnel, Route 15, Route 9 |
| Kingdra | Route 20, Seafoam Islands |
| Kingler | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Koffing | Pokémon Mansion |
| Krabby | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Lanturn | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Lapras | Route 20, Seafoam Islands |
| Ledian | Route 1, Route 2, Route 22, Route 23 and more |
| Ledyba | Route 1, Route 2, Route 22, Route 23 and more |
| Lickilicky | Cerulean Cave |
| Lickitung | Cerulean Cave, Route 16, Route 17, Route 18 |
| Machamp | Victory Road |
| Machoke | Rock Tunnel, Victory Road |
| Machop | Rock Tunnel, Victory Road |
| Magby | Pokémon Mansion |
| Magcargo | Pokémon Mansion, Route 16, Route 17, Route 18 |
| Magikarp | Celadon City, Cerulean Cave, Cerulean City, Cinnabar Island and more |
| Magmar | Pokémon Mansion |
| Magmortar | Pokémon Mansion |
| Magnemite | Cerulean Cave, Power Plant, Route 10, Route 11 and more |
| Magneton | Cerulean Cave, Power Plant, Route 10, Route 11 and more |
| Magnezone | Cerulean Cave, Power Plant |
| Mamoswine | Seafoam Islands |
| Mankey | Rock Tunnel, Route 22, Route 23, Route 3 and more |
| Marill | Cerulean Cave, Mt. Moon, Route 2, Route 22 and more |
| Marowak | Cerulean Cave, Pokémon Tower, Rock Tunnel, Route 10 and more |
| Meowth | Route 1, Route 11, Route 24, Route 3 and more |
| Metapod | Fuchsia City, Route 1, Route 15, Route 18 and more |
| Mime Jr. | Route 5 |
| Misdreavus | Cerulean Cave, Pokémon Tower, Rock Tunnel, Victory Road |
| Mismagius | Pokémon Tower, Victory Road |
| Mr. Mime | Cerulean Cave, Route 14 |
| Muk | Pokémon Mansion, Route 16, Route 17, Route 18 |
| Murkrow | Pokémon Tower, Route 13, Route 14, Route 15 and more |
| Nidoran♀ | Route 1, Route 13, Route 14, Route 15 and more |
| Nidoran♂ | Route 1, Route 13, Route 14, Route 15 and more |
| Nidorina | Route 1, Route 13, Route 14, Route 15 and more |
| Nidorino | Route 1, Route 13, Route 14, Route 15 and more |
| Ninetales | Pokémon Mansion |
| Noctowl | Fuchsia City, Route 1, Route 10, Route 11 and more |
| Octillery | Route 17, Route 18, Route 19, Route 21 and more |
| Oddish | Route 1, Route 11, Route 13, Route 14 and more |
| Onix | Diglett's Cave, Mt. Moon, Rock Tunnel, Route 9 and more |
| Paras | Cerulean Cave, Fuchsia City, Mt. Moon, Route 2 and more |
| Parasect | Cerulean Cave, Fuchsia City, Mt. Moon, Route 2 and more |
| Persian | Route 1, Route 11, Route 24, Route 3 and more |
| Phanpy | Route 23 |
| Pichu | Route 2, Route 24, Route 7, Viridian Forest |
| Pidgeot | Route 1, Route 11, Route 12, Route 13 and more |
| Pidgeotto | Route 1, Route 11, Route 12, Route 13 and more |
| Pidgey | Route 1, Route 11, Route 12, Route 13 and more |
| Pikachu | Cerulean Cave, Power Plant, Route 10, Route 2 and more |
| Piloswine | Seafoam Islands |
| Pineco | Route 12, Route 22, Route 23 |
| Pinsir | Route 25 |
| Politoed | Cerulean Cave, Route 23, Victory Road |
| Poliwag | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Poliwhirl | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Poliwrath | Cerulean Cave, Route 23, Victory Road |
| Ponyta | Pokémon Mansion, Route 16, Route 17, Route 22 and more |
| Porygon | Pokémon Mansion, Power Plant |
| Primeape | Rock Tunnel, Route 22, Route 23, Route 3 and more |
| Psyduck | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Quagsire | Route 22, Route 23 |
| Qwilfish | Route 12, Route 13, Route 21 |
| Raichu | Power Plant |
| Rapidash | Pokémon Mansion, Route 16, Route 17 |
| Raticate | Pokémon Mansion, Route 1, Route 11, Route 16 and more |
| Rattata | Pokémon Mansion, Route 1, Route 11, Route 16 and more |
| Remoraid | Route 17, Route 18, Route 19, Route 21 and more |
| Rhydon | Cerulean Cave, Route 23, Victory Road |
| Rhyhorn | Cerulean Cave, Rock Tunnel, Route 10, Route 23 and more |
| Rhyperior | Cerulean Cave, Victory Road |
| Sandshrew | Cerulean Cave, Diglett's Cave, Mt. Moon, Route 10 and more |
| Sandslash | Cerulean Cave, Diglett's Cave, Mt. Moon, Route 10 and more |
| Scyther | Route 15, Route 25 |
| Seadra | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Seaking | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Seel | Seafoam Islands |
| Sentret | Route 1, Route 21, Route 22 |
| Shellder | Cinnabar Island, Pallet Town, Route 17, Route 18 and more |
| Shuckle | Cerulean Cave, Rock Tunnel, Victory Road |
| Skarmory | Route 23 |
| Slowbro | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Slowking | Cerulean Cave, Seafoam Islands, Victory Road |
| Slowpoke | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Slugma | Pokémon Mansion, Route 16, Route 17, Route 18 |
| Smoochum | Seafoam Islands |
| Sneasel | Route 23, Seafoam Islands |
| Snorlax | Cerulean Cave, Route 23, Victory Road |
| Snubbull | Route 5, Route 6, Route 7, Route 8 |
| Spearow | Route 10, Route 11, Route 15, Route 16 and more |
| Spinarak | Fuchsia City, Route 1, Route 11, Route 12 and more |
| Squirtle | Cerulean City, Pallet Town, Route 19, Route 6 |
| Starmie | Route 20 |
| Staryu | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Steelix | Victory Road |
| Sunkern | Route 21, Route 24, Route 3 |
| Swinub | Seafoam Islands |
| Tangela | Route 21 |
| Tauros | Route 14, Route 23 |
| Teddiursa | Route 23 |
| Tentacool | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Tentacruel | Cinnabar Island, Pallet Town, Route 12, Route 13 and more |
| Tyrogue | Rock Tunnel, Victory Road |
| Ursaring | Route 23 |
| Venomoth | Cerulean Cave, Fuchsia City, Route 1, Route 11 and more |
| Venonat | Cerulean Cave, Fuchsia City, Route 1, Route 11 and more |
| Voltorb | Cerulean Cave, Power Plant, Route 10 |
| Vulpix | Pokémon Mansion, Route 7, Route 8 |
| Wartortle | Cerulean City, Pallet Town, Route 19, Route 6 |
| Weavile | Seafoam Islands |
| Weedle | Fuchsia City, Route 11, Route 15, Route 18 and more |
| Weepinbell | Route 13, Route 14, Route 15, Route 24 and more |
| Weezing | Pokémon Mansion |
| Wigglytuff | Cerulean Cave |
| Wobbuffet | Cerulean Cave |
| Wooper | Route 22, Route 23 |
| Yanma | Route 2, Route 21 |
| Zubat | Cerulean Cave, Diglett's Cave, Mt. Moon, Rock Tunnel and more |
