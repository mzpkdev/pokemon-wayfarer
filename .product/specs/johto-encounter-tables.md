# Johto encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Kanto and Johto encounter rules](kanto-johto-encounters.md). They are the
source of truth for Johto's wild-encounter data: every slot below maps one to one
to a slot in the game's encounter tables. Playtesting may change the picks.

## Scope

This spec lists Johto's wild-encounter tables: 87 maps, each with a day and a
night table for every method it has. The Safari Zone is deferred and not
listed. Legendaries and mythicals belong to a later spec.

## Behavior

### Reading the tables

- Places are ordered by reach: Road, Wilds, Outlands, then dungeons. Each place
  names its reach and its Gen I–II band, and each map is named by its map
  constant.
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

#### New Bark Town

Road, Johto east.

**`MAP_NEW_BARK_TOWN_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sentret–Furret | Oddish–Gloom |
| 2 | 20% | Ledyba–Ledian | Sentret–Furret |
| 3 | 10% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 4 | 10% | Hoppip–Jumpluff | Rattata–Raticate |
| 5 | 10% | Rattata–Raticate | Spinarak–Ariados |
| 6 | 10% | Sunkern | Marill–Azumarill |
| 7 | 5% | Caterpie–Butterfree | Oddish–Gloom |
| 8 | 5% | Marill–Azumarill | Hoothoot–Noctowl |
| 9 | 4% | Pidgey–Pidgeot | Chikorita–Meganium |
| 10 | 4% | Sentret–Furret | Rattata–Raticate |
| 11 | 1% | Togepi | Togepi |
| 12 | 1% | Chikorita–Meganium | Spinarak–Ariados |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 2 | 30% | Chinchou–Lanturn | Tentacool–Tentacruel |
| 3 | 5% | Shellder | Staryu |
| 4 | 4% | Krabby–Kingler | Shellder |
| 5 | 1% | Remoraid–Octillery | Remoraid–Octillery |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chinchou–Lanturn | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Shellder | Staryu |
| 5 | 8% | 9% | 10% | Chinchou–Lanturn | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Shellder |
| 7 | 3% | 6% | 10% | Corsola | Staryu |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Qwilfish | Corsola |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Qwilfish |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Ledyba–Ledian |
| 2 | 30% | Caterpie–Butterfree | Spinarak–Ariados |
| 3 | 5% | Pineco–Forretress | Caterpie–Butterfree |
| 4 | 4% | Exeggcute | Pineco–Forretress |
| 5 | 1% | Aipom | Hoothoot–Noctowl |

#### Cherrygrove City

Road, Johto east.

**`MAP_CHERRYGROVE_CITY_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Pidgey–Pidgeot | Rattata–Raticate |
| 2 | 20% | Spearow–Fearow | Meowth–Persian |
| 3 | 10% | Rattata–Raticate | Hoothoot–Noctowl |
| 4 | 10% | Ledyba–Ledian | Oddish–Gloom |
| 5 | 10% | Hoppip–Jumpluff | Spinarak–Ariados |
| 6 | 10% | Sentret–Furret | Gastly–Haunter |
| 7 | 5% | Sunkern | Rattata–Raticate |
| 8 | 5% | Caterpie–Butterfree | Oddish–Gloom |
| 9 | 4% | Spearow–Fearow | Hoothoot–Noctowl |
| 10 | 4% | Pidgey–Pidgeot | Meowth–Persian |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Sunkern | Gastly–Haunter |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Krabby–Kingler | Krabby–Kingler |
| 3 | 5% | Corsola | Staryu |
| 4 | 4% | Shellder | Shellder |
| 5 | 1% | Qwilfish | Qwilfish |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Corsola | Staryu |
| 5 | 8% | 9% | 10% | Krabby–Kingler | Krabby–Kingler |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 7 | 3% | 6% | 10% | Corsola | Staryu |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 9 | 2% | 4% | 9% | Qwilfish | Staryu |
| 10 | 2% | 4% | 9% | Krabby–Kingler | Krabby–Kingler |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Spearow–Fearow |
| 2 | 30% | Pidgey–Pidgeot | Pidgey–Pidgeot |
| 3 | 5% | Exeggcute | Spinarak–Ariados |
| 4 | 4% | Ledyba–Ledian | Exeggcute |
| 5 | 1% | Aipom | Aipom |

#### Violet City

Road, Johto east.

**`MAP_VIOLET_CITY_HNS`**

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
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Marill–Azumarill | Psyduck–Golduck |
| 4 | 4% | Psyduck–Golduck | Wooper–Quagsire |
| 5 | 1% | Magikarp–Gyarados | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Psyduck–Golduck |

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

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Ledyba–Ledian |
| 2 | 30% | Exeggcute | Exeggcute |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Aipom | Pineco–Forretress |
| 5 | 1% | Caterpie–Butterfree | Murkrow |

#### Ecruteak City

Road, Johto west.

**`MAP_ECRUTEAK_CITY_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sentret–Furret | Gastly–Haunter |
| 2 | 20% | Hoppip–Jumpluff | Sentret–Furret |
| 3 | 10% | Vulpix | Hoothoot–Noctowl |
| 4 | 10% | Pidgey–Pidgeot | Vulpix |
| 5 | 10% | Growlithe | Oddish–Gloom |
| 6 | 10% | Sunkern | Spinarak–Ariados |
| 7 | 5% | Ledyba–Ledian | Misdreavus |
| 8 | 5% | Caterpie–Butterfree | Gastly–Haunter |
| 9 | 4% | Eevee | Eevee |
| 10 | 4% | Sentret–Furret | Oddish–Gloom |
| 11 | 1% | Eevee | Houndour–Houndoom |
| 12 | 1% | Hoppip–Jumpluff | Murkrow |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Psyduck–Golduck |
| 2 | 30% | Psyduck–Golduck | Marill–Azumarill |
| 3 | 5% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 5 | 1% | Wooper–Quagsire | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Marill–Azumarill |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Psyduck–Golduck |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Marill–Azumarill | Magikarp–Gyarados |
| 9 | 2% | 4% | 9% | Poliwag–Poliwhirl | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Psyduck–Golduck |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Exeggcute | Exeggcute |
| 2 | 30% | Ledyba–Ledian | Ledyba–Ledian |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Aipom | Pineco–Forretress |
| 5 | 1% | Caterpie–Butterfree | Hoothoot–Noctowl |

#### Olivine City

Road, Johto west.

**`MAP_OLIVINE_CITY_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Meowth–Persian | Meowth–Persian |
| 2 | 20% | Hoppip–Jumpluff | Magnemite–Magneton |
| 3 | 10% | Magnemite–Magneton | Gastly–Haunter |
| 4 | 10% | Spearow–Fearow | Hoothoot–Noctowl |
| 5 | 10% | Sunkern | Grimer–Muk |
| 6 | 10% | Rattata–Raticate | Oddish–Gloom |
| 7 | 5% | Grimer–Muk | Gastly–Haunter |
| 8 | 5% | Snubbull–Granbull | Meowth–Persian |
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
| 1 | 38% | 25% | 12% | Corsola | Remoraid–Octillery |
| 2 | 22% | 18% | 10% | Remoraid–Octillery | Corsola |
| 3 | 10% | 12% | 11% | Krabby–Kingler | Staryu |
| 4 | 8% | 10% | 10% | Magikarp–Gyarados | Krabby–Kingler |
| 5 | 8% | 9% | 10% | Corsola | Staryu |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Magikarp–Gyarados |
| 7 | 3% | 6% | 10% | Shellder | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Qwilfish | Staryu |
| 9 | 2% | 4% | 9% | Corsola | Qwilfish |
| 10 | 2% | 4% | 9% | Tentacool–Tentacruel | Chinchou–Lanturn |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Exeggcute | Exeggcute |
| 2 | 30% | Spearow–Fearow | Spearow–Fearow |
| 3 | 5% | Pineco–Forretress | Spinarak–Ariados |
| 4 | 4% | Ledyba–Ledian | Pineco–Forretress |
| 5 | 1% | Aipom | Hoothoot–Noctowl |

**`MAP_OLIVINE_CITY_PORT_OUTSIDE_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hoppip–Jumpluff | Krabby–Kingler |
| 2 | 20% | Krabby–Kingler | Meowth–Persian |
| 3 | 10% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 4 | 10% | Snubbull–Granbull | Gastly–Haunter |
| 5 | 10% | Sunkern | Oddish–Gloom |
| 6 | 10% | Meowth–Persian | Snubbull–Granbull |
| 7 | 5% | Grimer–Muk | Slowpoke–Slowbro |
| 8 | 5% | Spearow–Fearow | Hoothoot–Noctowl |
| 9 | 4% | Ditto | Ditto |
| 10 | 4% | Krabby–Kingler | Gastly–Haunter |
| 11 | 1% | Farfetch'd | Farfetch'd |
| 12 | 1% | Slowpoke–Slowbro | Spinarak–Ariados |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Remoraid–Octillery |
| 2 | 30% | Remoraid–Octillery | Tentacool–Tentacruel |
| 3 | 5% | Corsola | Chinchou–Lanturn |
| 4 | 4% | Chinchou–Lanturn | Staryu |
| 5 | 1% | Qwilfish | Corsola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Remoraid–Octillery | Remoraid–Octillery |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Corsola | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Chinchou–Lanturn | Corsola |
| 5 | 8% | 9% | 10% | Qwilfish | Staryu |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Shellder | Staryu |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Chinchou–Lanturn | Qwilfish |
| 10 | 2% | 4% | 9% | Corsola | Magikarp–Gyarados |

#### Cianwood City

Road, Johto west.

**`MAP_CIANWOOD_CITY_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Mankey–Primeape | Machop–Machoke |
| 2 | 20% | Hoppip–Jumpluff | Mankey–Primeape |
| 3 | 10% | Machop–Machoke | Hoothoot–Noctowl |
| 4 | 10% | Krabby–Kingler | Gastly–Haunter |
| 5 | 10% | Sunkern | Oddish–Gloom |
| 6 | 10% | Spearow–Fearow | Spinarak–Ariados |
| 7 | 5% | Tyrogue | Zubat–Golbat |
| 8 | 5% | Geodude–Graveler | Hoothoot–Noctowl |
| 9 | 4% | Tyrogue | Tyrogue |
| 10 | 4% | Mankey–Primeape | Gastly–Haunter |
| 11 | 1% | Ditto | Ditto |
| 12 | 1% | Machop–Machoke | Oddish–Gloom |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chinchou–Lanturn | Chinchou–Lanturn |
| 2 | 30% | Remoraid–Octillery | Remoraid–Octillery |
| 3 | 5% | Shellder | Staryu |
| 4 | 4% | Staryu | Staryu |
| 5 | 1% | Mantyke | Shellder |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chinchou–Lanturn | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Remoraid–Octillery | Remoraid–Octillery |
| 3 | 10% | 12% | 11% | Shellder | Staryu |
| 4 | 8% | 10% | 10% | Corsola | Corsola |
| 5 | 8% | 9% | 10% | Chinchou–Lanturn | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Qwilfish | Staryu |
| 7 | 3% | 6% | 10% | Krabby–Kingler | Shellder |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Staryu | Staryu |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Qwilfish |

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

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Phanpy–Donphan |
| 2 | 20% | Phanpy–Donphan | Oddish–Gloom |
| 3 | 10% | Hoppip–Jumpluff | Hoothoot–Noctowl |
| 4 | 10% | Geodude–Graveler | Gastly–Haunter |
| 5 | 10% | Pidgey–Pidgeot | Zubat–Golbat |
| 6 | 10% | Sunkern | Swinub–Piloswine |
| 7 | 5% | Swinub–Piloswine | Sneasel |
| 8 | 5% | Machop–Machoke | Hoothoot–Noctowl |
| 9 | 4% | Gligar | Zubat–Golbat |
| 10 | 4% | Spearow–Fearow | Gastly–Haunter |
| 11 | 1% | Gligar | Sneasel |
| 12 | 1% | Swinub–Piloswine | Gligar |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 30% | Horsea–Seadra | Horsea–Seadra |
| 3 | 5% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Poliwag–Poliwhirl |
| 5 | 1% | Psyduck–Golduck | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Horsea–Seadra |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 6 | 4% | 7% | 10% | Horsea–Seadra | Horsea–Seadra |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Magikarp–Gyarados |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Spinarak–Ariados |
| 2 | 30% | Spinarak–Ariados | Spearow–Fearow |
| 3 | 5% | Pineco–Forretress | Hoothoot–Noctowl |
| 4 | 4% | Aipom | Pineco–Forretress |
| 5 | 1% | Exeggcute | Aipom |

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
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Marill–Azumarill |
| 5 | 1% | Totodile–Feraligatr | Totodile–Feraligatr |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Spinarak–Ariados |
| 2 | 30% | Spinarak–Ariados | Hoothoot–Noctowl |
| 3 | 5% | Exeggcute | Ledyba–Ledian |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Aipom | Aipom |

#### Route 31

Road, Johto east.

**`MAP_ROUTE31_HNS`**

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
| 1 | 60% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 30% | Marill–Azumarill | Marill–Azumarill |
| 3 | 5% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 5 | 1% | Psyduck–Golduck | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Marill–Azumarill | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Marill–Azumarill |
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
| 1 | 60% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 30% | Qwilfish | Qwilfish |
| 3 | 5% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 4 | 4% | Marill–Azumarill | Tentacool–Tentacruel |
| 5 | 1% | Tentacool–Tentacruel | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Qwilfish | Qwilfish |
| 2 | 22% | 18% | 10% | Qwilfish | Qwilfish |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Qwilfish | Qwilfish |
| 6 | 4% | 7% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 9 | 2% | 4% | 9% | Qwilfish | Qwilfish |
| 10 | 2% | 4% | 9% | Remoraid–Octillery | Magikarp–Gyarados |

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

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Snubbull–Granbull | Drowzee–Hypno |
| 2 | 20% | Pidgey–Pidgeot | Grimer–Muk |
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
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Corsola | Corsola |
| 3 | 5% | Krabby–Kingler | Staryu |
| 4 | 4% | Chinchou–Lanturn | Chinchou–Lanturn |
| 5 | 1% | Tentacool–Tentacruel | Krabby–Kingler |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corsola | Corsola |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Krabby–Kingler | Staryu |
| 4 | 8% | 10% | 10% | Corsola | Krabby–Kingler |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Corsola |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Staryu |
| 7 | 3% | 6% | 10% | Corsola | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Chinchou–Lanturn | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Qwilfish | Corsola |
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
| 2 | 30% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 5% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 4% | Wooper–Quagsire | Wooper–Quagsire |
| 5 | 1% | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Marill–Azumarill |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pidgey–Pidgeot | Pidgey–Pidgeot |
| 2 | 30% | Caterpie–Butterfree | Spinarak–Ariados |
| 3 | 5% | Ledyba–Ledian | Hoothoot–Noctowl |
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
| 7 | 5% | Sudowoodo | Sudowoodo |
| 8 | 5% | Sunkern | Houndour–Houndoom |
| 9 | 4% | Bonsly | Spinarak–Ariados |
| 10 | 4% | Ledyba–Ledian | Hoothoot–Noctowl |
| 11 | 1% | Sudowoodo | Vulpix |
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

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Ledyba–Ledian | Spinarak–Ariados |
| 2 | 30% | Pidgey–Pidgeot | Hoothoot–Noctowl |
| 3 | 5% | Exeggcute | Ledyba–Ledian |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Aipom | Aipom |

#### Route 38

Road, Johto west.

**`MAP_ROUTE38_HNS`**

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

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Corsola | Corsola |
| 3 | 5% | Shellder | Staryu |
| 4 | 4% | Staryu | Chinchou–Lanturn |
| 5 | 1% | Qwilfish | Shellder |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Krabby–Kingler | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Corsola | Corsola |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Staryu |
| 4 | 8% | 10% | 10% | Shellder | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Corsola | Staryu |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Shellder |
| 7 | 3% | 6% | 10% | Tentacool–Tentacruel | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Qwilfish | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Staryu | Tentacool–Tentacruel |
| 10 | 2% | 4% | 9% | Remoraid–Octillery | Staryu |

#### Route 41

Road, Johto west.

**`MAP_ROUTE41_HNS`**

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 2 | 30% | Horsea–Seadra | Horsea–Seadra |
| 3 | 5% | Mantyke | Mantyke |
| 4 | 4% | Chinchou–Lanturn | Chinchou–Lanturn |
| 5 | 1% | Qwilfish | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chinchou–Lanturn | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Horsea–Seadra |
| 3 | 10% | 12% | 11% | Qwilfish | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Chinchou–Lanturn | Staryu |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Horsea–Seadra | Magikarp–Gyarados |
| 7 | 3% | 6% | 10% | Corsola | Staryu |
| 8 | 3% | 5% | 9% | Chinchou–Lanturn | Horsea–Seadra |
| 9 | 2% | 4% | 9% | Krabby–Kingler | Qwilfish |
| 10 | 2% | 4% | 9% | Staryu | Staryu |

#### Route 42

Road, Johto east.

**`MAP_ROUTE42_HNS`**

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
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Remoraid–Octillery | Remoraid–Octillery |
| 3 | 5% | Goldeen–Seaking | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Goldeen–Seaking |
| 5 | 1% | Poliwag–Poliwhirl | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Remoraid–Octillery | Remoraid–Octillery |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Goldeen–Seaking | Goldeen–Seaking |
| 4 | 8% | 10% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Goldeen–Seaking | Remoraid–Octillery |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Magikarp–Gyarados |

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
| 2 | 30% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 5% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Poliwag–Poliwhirl |
| 5 | 1% | Psyduck–Golduck | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 4 | 8% | 10% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Goldeen–Seaking |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Wooper–Quagsire | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Psyduck–Golduck |

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
| 2 | 30% | Remoraid–Octillery | Remoraid–Octillery |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Magikarp–Gyarados |
| 5 | 1% | Marill–Azumarill | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Remoraid–Octillery | Remoraid–Octillery |
| 2 | 22% | 18% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Marill–Azumarill | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

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
| 2 | 30% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 5% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 4 | 4% | Marill–Azumarill | Wooper–Quagsire |
| 5 | 1% | Goldeen–Seaking | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Magikarp–Gyarados |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 9 | 2% | 4% | 9% | Marill–Azumarill | Poliwag–Poliwhirl |
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

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Geodude–Graveler | Geodude–Graveler |
| 2 | 30% | Spearow–Fearow | Spearow–Fearow |
| 3 | 5% | Aipom | Spinarak–Ariados |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Exeggcute | Aipom |

#### Route 48

Road, Johto west.

**`MAP_ROUTE48_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Oddish–Gloom |
| 2 | 20% | Sunkern | Vulpix |
| 3 | 10% | Oddish–Gloom | Hoothoot–Noctowl |
| 4 | 10% | Vulpix | Gastly–Haunter |
| 5 | 10% | Hoppip–Jumpluff | Houndour–Houndoom |
| 6 | 10% | Growlithe | Venonat–Venomoth |
| 7 | 5% | Farfetch'd | Oddish–Gloom |
| 8 | 5% | Pidgey–Pidgeot | Farfetch'd |
| 9 | 4% | Farfetch'd | Yanma |
| 10 | 4% | Nidoran♀–Nidorina | Hoothoot–Noctowl |
| 11 | 1% | Yanma | Houndour–Houndoom |
| 12 | 1% | Sunkern | Spinarak–Ariados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spearow–Fearow | Spearow–Fearow |
| 2 | 30% | Weedle–Beedrill | Weedle–Beedrill |
| 3 | 5% | Aipom | Spinarak–Ariados |
| 4 | 4% | Pineco–Forretress | Pineco–Forretress |
| 5 | 1% | Ledyba–Ledian | Yanma |

#### Ilex Forest

Road, Johto west.

**`MAP_ILEX_FOREST_HNS`**

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
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Marill–Azumarill |

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
| 4 | 4% | Staryu | Totodile–Feraligatr |
| 5 | 1% | Mantine | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Shellder | Shellder |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Shellder | Staryu |
| 6 | 4% | 7% | 10% | Corsola | Krabby–Kingler |
| 7 | 3% | 6% | 10% | Staryu | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Tentacool–Tentacruel | Staryu |
| 9 | 2% | 4% | 9% | Krabby–Kingler | Tentacool–Tentacruel |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Magikarp–Gyarados |

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

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spearow–Fearow | Oddish–Gloom |
| 2 | 20% | Hoppip–Jumpluff | Venonat–Venomoth |
| 3 | 10% | Oddish–Gloom | Hoothoot–Noctowl |
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
| 1 | 60% | Corsola | Seel–Dewgong |
| 2 | 30% | Seel–Dewgong | Corsola |
| 3 | 5% | Chinchou–Lanturn | Chinchou–Lanturn |
| 4 | 4% | Shellder | Staryu |
| 5 | 1% | Mantine | Mantine |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corsola | Corsola |
| 2 | 22% | 18% | 10% | Seel–Dewgong | Seel–Dewgong |
| 3 | 10% | 12% | 11% | Chinchou–Lanturn | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Shellder | Staryu |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Corsola | Magikarp–Gyarados |
| 7 | 3% | 6% | 10% | Staryu | Staryu |
| 8 | 3% | 5% | 9% | Shellder | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Chinchou–Lanturn | Corsola |
| 10 | 2% | 4% | 9% | Remoraid–Octillery | Chinchou–Lanturn |

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
| 1 | 60% | Goldeen–Seaking | Chinchou–Lanturn |
| 2 | 30% | Chinchou–Lanturn | Goldeen–Seaking |
| 3 | 5% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 4% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 5 | 1% | Marill–Azumarill | Wooper–Quagsire |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Chinchou–Lanturn | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Magikarp–Gyarados |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Chinchou–Lanturn | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Wooper–Quagsire |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Goldeen–Seaking |

**`MAP_DARK_CAVE_NORTH_SIDE_HNS`**

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
| 1 | 60% | Magikarp–Gyarados | Chinchou–Lanturn |
| 2 | 30% | Chinchou–Lanturn | Magikarp–Gyarados |
| 3 | 5% | Goldeen–Seaking | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Goldeen–Seaking |
| 5 | 1% | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Chinchou–Lanturn | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Goldeen–Seaking | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Magikarp–Gyarados | Goldeen–Seaking |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Chinchou–Lanturn | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Marill–Azumarill | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Magikarp–Gyarados |

#### Cliff Edge Cave

Wilds, Johto west.

**`MAP_CLIFF_EDGE_GATE_HNS`**

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wooper–Quagsire | Chinchou–Lanturn |
| 2 | 30% | Chinchou–Lanturn | Wooper–Quagsire |
| 3 | 5% | Shellder | Staryu |
| 4 | 4% | Tentacool–Tentacruel | Shellder |
| 5 | 1% | Mantine | Mantine |

**`MAP_CLIFF_EDGE_CAVE_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Krabby–Kingler | Geodude–Graveler |
| 2 | 20% | Geodude–Graveler | Krabby–Kingler |
| 3 | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 4 | 10% | Corsola | Corsola |
| 5 | 10% | Zubat–Golbat | Zubat–Golbat |
| 6 | 10% | Onix | Gastly–Haunter |
| 7 | 5% | Shuckle | Shuckle |
| 8 | 5% | Wooper–Quagsire | Wooper–Quagsire |
| 9 | 4% | Skarmory | Onix |
| 10 | 4% | Seel–Dewgong | Seel–Dewgong |
| 11 | 1% | Skarmory | Skarmory |
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
| 1 | 60% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 30% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Marill–Azumarill | Psyduck–Golduck |
| 5 | 1% | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Psyduck–Golduck |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Magikarp–Gyarados |

#### Ruins of Alph

Wilds, Johto east.

**`MAP_RUINS_OF_ALPH_OUTSIDE_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Wooper–Quagsire |
| 2 | 20% | Unown | Natu–Xatu |
| 3 | 10% | Mareep–Ampharos | Gastly–Haunter |
| 4 | 10% | Hoppip–Jumpluff | Hoothoot–Noctowl |
| 5 | 10% | Pidgey–Pidgeot | Unown |
| 6 | 10% | Sunkern | Oddish–Gloom |
| 7 | 5% | Smeargle | Misdreavus |
| 8 | 5% | Natu–Xatu | Smeargle |
| 9 | 4% | Smeargle | Gastly–Haunter |
| 10 | 4% | Unown | Wooper–Quagsire |
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
| 2 | 22% | 18% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Poliwag–Poliwhirl |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Wooper–Quagsire |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Natu–Xatu | Natu–Xatu |
| 2 | 30% | Natu–Xatu | Natu–Xatu |
| 3 | 5% | Smeargle | Spinarak–Ariados |
| 4 | 4% | Aipom | Smeargle |
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

**`MAP_NATIONAL_PARK_BUG_CONTEST_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Caterpie–Butterfree | Venonat–Venomoth |
| 2 | 20% | Weedle–Beedrill | Spinarak–Ariados |
| 3 | 10% | Ledyba–Ledian | Paras–Parasect |
| 4 | 10% | Paras–Parasect | Pineco–Forretress |
| 5 | 10% | Venonat–Venomoth | Ledyba–Ledian |
| 6 | 10% | Caterpie–Metapod | Weedle–Beedrill |
| 7 | 5% | Scyther | Scyther |
| 8 | 5% | Pinsir | Pinsir |
| 9 | 4% | Yanma | Yanma |
| 10 | 4% | Heracross | Heracross |
| 11 | 1% | Scyther | Scyther |
| 12 | 1% | Pinsir | Spinarak–Ariados |

#### Lake of Rage

Wilds, Johto east.

**`MAP_LAKE_OF_RAGE_HNS`**

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
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Goldeen–Seaking |
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
| 3 | 5% | Staryu–Starmie | Chinchou–Lanturn |
| 4 | 4% | Lapras | Staryu–Starmie |
| 5 | 1% | Mantine | Lapras |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Shellder–Cloyster | Shellder–Cloyster |
| 2 | 22% | 18% | 10% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Staryu–Starmie |
| 5 | 8% | 9% | 10% | Staryu–Starmie | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Corsola | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Magikarp–Gyarados | Staryu–Starmie |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Remoraid–Octillery |

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
| 2 | 30% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 3 | 5% | Goldeen–Seaking | Wooper–Quagsire |
| 4 | 4% | Poliwag–Politoed | Goldeen–Seaking |
| 5 | 1% | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 2 | 22% | 18% | 10% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Poliwag–Politoed | Poliwag–Politoed |
| 9 | 2% | 4% | 9% | Psyduck–Golduck | Goldeen–Seaking |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pineco–Forretress | Teddiursa–Ursaring |
| 2 | 30% | Teddiursa–Ursaring | Pineco–Forretress |
| 3 | 5% | Heracross | Murkrow–Honchkrow |
| 4 | 4% | Scyther–Scizor | Heracross |
| 5 | 1% | Aipom–Ambipom | Scyther–Scizor |

#### Union Cave

Dungeon, Johto east.

**`MAP_UNION_CAVE_1F_HNS`**

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
| 3 | 5% | Goldeen–Seaking | Chinchou–Lanturn |
| 4 | 4% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 1% | Psyduck–Golduck | Poliwag–Poliwhirl |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Wooper–Quagsire | Wooper–Quagsire |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Magikarp–Gyarados |

**`MAP_UNION_CAVE_B1F_HNS`**

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
| 1 | 60% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 30% | Marill–Azumarill | Marill–Azumarill |
| 3 | 5% | Goldeen–Seaking | Chinchou–Lanturn |
| 4 | 4% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 5 | 1% | Psyduck–Golduck | Psyduck–Golduck |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Goldeen–Seaking | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Magikarp–Gyarados |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Goldeen–Seaking |
| 9 | 2% | 4% | 9% | Chinchou–Lanturn | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Wooper–Quagsire |

**`MAP_UNION_CAVE_B2F_HNS`**

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
| 1 | 60% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 30% | Shellder | Shellder |
| 3 | 5% | Lapras | Chinchou–Lanturn |
| 4 | 4% | Tentacool–Tentacruel | Lapras |
| 5 | 1% | Chinchou–Lanturn | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Shellder | Shellder |
| 2 | 22% | 18% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 3 | 10% | 12% | 11% | Chinchou–Lanturn | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Tentacool–Tentacruel | Staryu |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Shellder | Magikarp–Gyarados |
| 7 | 3% | 6% | 10% | Tentacool–Tentacruel | Staryu |
| 8 | 3% | 5% | 9% | Chinchou–Lanturn | Tentacool–Tentacruel |
| 9 | 2% | 4% | 9% | Krabby–Kingler | Staryu |
| 10 | 2% | 4% | 9% | Staryu | Krabby–Kingler |

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
| 5 | 10% | Seel–Dewgong | Sneasel |
| 6 | 10% | Onix | Seel–Dewgong |
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
| 3 | 10% | Seel–Dewgong | Delibird |
| 4 | 10% | Sneasel | Sneasel |
| 5 | 10% | Zubat–Golbat | Seel–Dewgong |
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
| 5 | 10% | Seel–Dewgong | Swinub–Piloswine |
| 6 | 10% | Swinub–Mamoswine | Seel–Dewgong |
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
| 5 | 10% | Seel–Dewgong | Delibird |
| 6 | 10% | Sneasel | Swinub–Mamoswine |
| 7 | 5% | Sneasel–Weavile | Sneasel |
| 8 | 5% | Jynx | Jynx |
| 9 | 4% | Smoochum | Smoochum |
| 10 | 4% | Onix–Steelix | Sneasel–Weavile |
| 11 | 1% | Zubat–Crobat | Zubat–Crobat |
| 12 | 1% | Jynx | Seel–Dewgong |

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
| 4 | 4% | Psyduck–Golduck | Marill–Azumarill |
| 5 | 1% | Wooper–Quagsire | Chinchou–Lanturn |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 22% | 18% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Marill–Azumarill |
| 5 | 8% | 9% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Goldeen–Seaking |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Poliwag–Poliwhirl | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Chinchou–Lanturn |

**`MAP_SLOWPOKE_WELL_B2F_HNS`**

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
| 5 | 1% | Wooper–Quagsire | Chinchou–Lanturn |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 22% | 18% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Wooper–Quagsire |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Marill–Azumarill |
| 5 | 8% | 9% | 10% | Slowpoke–Slowking | Slowpoke–Slowking |
| 6 | 4% | 7% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Magikarp–Gyarados | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 10 | 2% | 4% | 9% | Wooper–Quagsire | Chinchou–Lanturn |

#### Burned Tower

Dungeon, Johto west.

**`MAP_BURNED_TOWER_1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Slugma–Magcargo | Slugma–Magcargo |
| 2 | 20% | Rattata–Raticate | Rattata–Raticate |
| 3 | 10% | Koffing–Weezing | Gastly–Haunter |
| 4 | 10% | Zubat–Golbat | Zubat–Golbat |
| 5 | 10% | Rattata–Raticate | Koffing–Weezing |
| 6 | 10% | Slugma–Magcargo | Houndour–Houndoom |
| 7 | 5% | Houndour–Houndoom | Houndour–Houndoom |
| 8 | 5% | Magby | Magby |
| 9 | 4% | Houndour–Houndoom | Gastly–Haunter |
| 10 | 4% | Koffing–Weezing | Misdreavus |
| 11 | 1% | Magby | Magby |
| 12 | 1% | Vulpix | Vulpix |

**`MAP_BURNED_TOWER_B1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gastly–Haunter | Gastly–Haunter |
| 2 | 20% | Slugma–Magcargo | Slugma–Magcargo |
| 3 | 10% | Koffing–Weezing | Misdreavus |
| 4 | 10% | Misdreavus | Murkrow |
| 5 | 10% | Zubat–Golbat | Houndour–Houndoom |
| 6 | 10% | Houndour–Houndoom | Koffing–Weezing |
| 7 | 5% | Magmar | Misdreavus |
| 8 | 5% | Cyndaquil–Typhlosion | Magmar |
| 9 | 4% | Misdreavus | Gastly–Gengar |
| 10 | 4% | Magby | Magby |
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
| 8 | 5% | Geodude–Graveler | Magnemite–Magneton |
| 9 | 4% | Meowth–Persian | Zubat–Golbat |
| 10 | 4% | Porygon | Porygon |
| 11 | 1% | Porygon–Porygon2 | Porygon–Porygon2 |
| 12 | 1% | Koffing–Weezing | Gastly–Haunter |

#### Mt. Mortar

Dungeon, Johto east.

**`MAP_MT_MORTAR_1F_SOUTH_HNS`**

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
| 5 | 1% | Magikarp–Gyarados | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Psyduck–Golduck |

**`MAP_MT_MORTAR_1F_NORTH_HNS`**

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
| 3 | 5% | Psyduck–Golduck | Goldeen–Seaking |
| 4 | 4% | Goldeen–Seaking | Psyduck–Golduck |
| 5 | 1% | Magikarp–Gyarados | Chinchou–Lanturn |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Marill–Azumarill | Wooper–Quagsire |
| 2 | 22% | 18% | 10% | Wooper–Quagsire | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Chinchou–Lanturn |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Chinchou–Lanturn |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Magikarp–Gyarados |

**`MAP_MT_MORTAR_2F_HNS`**

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
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Poliwag–Poliwhirl | Psyduck–Golduck |
| 5 | 1% | Magikarp–Gyarados | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Marill–Azumarill |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Poliwag–Poliwhirl |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Psyduck–Golduck |

**`MAP_MT_MORTAR_B1F_HNS`**

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
| 2 | 30% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 5% | Psyduck–Golduck | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Psyduck–Golduck |
| 5 | 1% | Poliwag–Poliwhirl | Magikarp–Gyarados |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Marill–Azumarill | Marill–Azumarill |
| 2 | 22% | 18% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Goldeen–Seaking | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Poliwag–Poliwhirl |
| 10 | 2% | 4% | 9% | Marill–Azumarill | Magikarp–Gyarados |

#### Tin Tower

Dungeon, Johto west.

**`MAP_TIN_TOWER_3F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Aipom | Aipom |
| 2 | 20% | Sentret–Furret | Sentret–Furret |
| 3 | 10% | Natu–Xatu | Gastly–Haunter |
| 4 | 10% | Rattata–Raticate | Hoothoot–Noctowl |
| 5 | 10% | Hoothoot–Noctowl | Misdreavus |
| 6 | 10% | Gastly–Haunter | Natu–Xatu |
| 7 | 5% | Misdreavus | Murkrow |
| 8 | 5% | Sentret–Furret | Gastly–Haunter |
| 9 | 4% | Houndour–Houndoom | Misdreavus |
| 10 | 4% | Misdreavus | Houndour–Houndoom |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Stantler | Stantler |

**`MAP_TIN_TOWER_4F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Gastly–Haunter |
| 2 | 20% | Aipom | Natu–Xatu |
| 3 | 10% | Sentret–Furret | Hoothoot–Noctowl |
| 4 | 10% | Hoothoot–Noctowl | Aipom |
| 5 | 10% | Gastly–Haunter | Misdreavus |
| 6 | 10% | Misdreavus | Murkrow |
| 7 | 5% | Aipom–Ambipom | Misdreavus–Mismagius |
| 8 | 5% | Houndour–Houndoom | Houndour–Houndoom |
| 9 | 4% | Natu–Xatu | Misdreavus |
| 10 | 4% | Misdreavus | Gastly–Gengar |
| 11 | 1% | Eevee | Eevee |
| 12 | 1% | Misdreavus–Mismagius | Murkrow–Honchkrow |

**`MAP_TIN_TOWER_5F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Natu–Xatu |
| 2 | 20% | Sentret–Furret | Misdreavus |
| 3 | 10% | Hoothoot–Noctowl | Hoothoot–Noctowl |
| 4 | 10% | Aipom | Murkrow |
| 5 | 10% | Misdreavus | Gastly–Haunter |
| 6 | 10% | Gastly–Haunter | Aipom |
| 7 | 5% | Aipom–Ambipom | Misdreavus–Mismagius |
| 8 | 5% | Houndour–Houndoom | Murkrow–Honchkrow |
| 9 | 4% | Skarmory | Skarmory |
| 10 | 4% | Misdreavus–Mismagius | Houndour–Houndoom |
| 11 | 1% | Eevee–Espeon | Eevee–Umbreon |
| 12 | 1% | Stantler | Gastly–Gengar |

**`MAP_TIN_TOWER_6F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sentret–Furret | Gastly–Haunter |
| 2 | 20% | Natu–Xatu | Natu–Xatu |
| 3 | 10% | Hoothoot–Noctowl | Misdreavus |
| 4 | 10% | Misdreavus | Hoothoot–Noctowl |
| 5 | 10% | Aipom–Ambipom | Murkrow |
| 6 | 10% | Gastly–Haunter | Aipom–Ambipom |
| 7 | 5% | Skarmory | Gastly–Gengar |
| 8 | 5% | Houndour–Houndoom | Murkrow–Honchkrow |
| 9 | 4% | Misdreavus–Mismagius | Misdreavus–Mismagius |
| 10 | 4% | Stantler | Houndour–Houndoom |
| 11 | 1% | Eevee–Espeon | Eevee–Umbreon |
| 12 | 1% | Skarmory | Skarmory |

**`MAP_TIN_TOWER_7F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Natu–Xatu | Hoothoot–Noctowl |
| 2 | 20% | Hoothoot–Noctowl | Natu–Xatu |
| 3 | 10% | Sentret–Furret | Gastly–Haunter |
| 4 | 10% | Aipom–Ambipom | Murkrow |
| 5 | 10% | Misdreavus | Misdreavus |
| 6 | 10% | Gastly–Haunter | Aipom–Ambipom |
| 7 | 5% | Skarmory | Gastly–Gengar |
| 8 | 5% | Misdreavus–Mismagius | Murkrow–Honchkrow |
| 9 | 4% | Eevee–Espeon | Misdreavus–Mismagius |
| 10 | 4% | Houndour–Houndoom | Eevee–Umbreon |
| 11 | 1% | Stantler | Skarmory |
| 12 | 1% | Skarmory | Houndour–Houndoom |

**`MAP_TIN_TOWER_8F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gastly–Haunter | Gastly–Haunter |
| 2 | 20% | Natu–Xatu | Misdreavus |
| 3 | 10% | Hoothoot–Noctowl | Natu–Xatu |
| 4 | 10% | Aipom–Ambipom | Murkrow |
| 5 | 10% | Skarmory | Misdreavus–Mismagius |
| 6 | 10% | Misdreavus–Mismagius | Skarmory |
| 7 | 5% | Eevee–Espeon | Murkrow–Honchkrow |
| 8 | 5% | Houndour–Houndoom | Eevee–Umbreon |
| 9 | 4% | Skarmory | Gastly–Gengar |
| 10 | 4% | Gastly–Gengar | Houndour–Houndoom |
| 11 | 1% | Stantler | Zubat–Crobat |
| 12 | 1% | Misdreavus | Eevee–Espeon |

**`MAP_TIN_TOWER_9F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hoothoot–Noctowl | Hoothoot–Noctowl |
| 2 | 20% | Sentret–Furret | Natu–Xatu |
| 3 | 10% | Natu–Xatu | Murkrow–Honchkrow |
| 4 | 10% | Skarmory | Skarmory |
| 5 | 10% | Aipom–Ambipom | Misdreavus–Mismagius |
| 6 | 10% | Misdreavus–Mismagius | Gastly–Haunter |
| 7 | 5% | Eevee–Espeon | Eevee–Umbreon |
| 8 | 5% | Skarmory | Skarmory |
| 9 | 4% | Houndour–Houndoom | Gastly–Gengar |
| 10 | 4% | Gastly–Gengar | Zubat–Crobat |
| 11 | 1% | Eevee–Espeon | Eevee–Umbreon |
| 12 | 1% | Stantler | Houndour–Houndoom |

#### Dragon's Den

Dungeon, Johto east.

**`MAP_DRAGONS_DEN_CAVERN_HNS`**

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Magikarp–Gyarados | Dratini–Dragonite |
| 2 | 30% | Horsea–Seadra | Magikarp–Gyarados |
| 3 | 5% | Dratini–Dragonite | Horsea–Seadra |
| 4 | 4% | Horsea–Kingdra | Dratini–Dragonite |
| 5 | 1% | Dratini–Dragonite | Horsea–Kingdra |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Magikarp–Gyarados | Dratini–Dragonite |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Dratini–Dragonite |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Horsea–Seadra |
| 5 | 8% | 9% | 10% | Dratini–Dragonite | Dratini–Dragonite |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 7 | 3% | 6% | 10% | Dratini–Dragonite | Horsea–Seadra |
| 8 | 3% | 5% | 9% | Horsea–Seadra | Dratini–Dragonite |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 10 | 2% | 4% | 9% | Dratini–Dragonite | Dratini–Dragonite |

#### Whirl Islands

Dungeon, Johto west.

**`MAP_WHIRL_ISLANDS_1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Zubat–Golbat |
| 2 | 20% | Krabby–Kingler | Seel–Dewgong |
| 3 | 10% | Corsola | Corsola |
| 4 | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 5 | 10% | Natu–Xatu | Krabby–Kingler |
| 6 | 10% | Zubat–Golbat | Gastly–Haunter |
| 7 | 5% | Seel–Dewgong | Seel–Dewgong |
| 8 | 5% | Corsola | Corsola |
| 9 | 4% | Krabby–Kingler | Natu–Xatu |
| 10 | 4% | Slowpoke–Slowbro | Gastly–Haunter |
| 11 | 1% | Skarmory | Skarmory |
| 12 | 1% | Natu–Xatu | Slowpoke–Slowbro |

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
| 1 | 38% | 25% | 12% | Corsola | Krabby–Kingler |
| 2 | 22% | 18% | 10% | Krabby–Kingler | Corsola |
| 3 | 10% | 12% | 11% | Qwilfish | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Qwilfish |
| 5 | 8% | 9% | 10% | Corsola | Staryu |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Corsola |
| 7 | 3% | 6% | 10% | Staryu | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Qwilfish | Staryu |
| 9 | 2% | 4% | 9% | Chinchou–Lanturn | Horsea–Seadra |
| 10 | 2% | 4% | 9% | Horsea–Seadra | Qwilfish |

**`MAP_WHIRL_ISLANDS_B1F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 2 | 20% | Corsola | Corsola |
| 3 | 10% | Seel–Dewgong | Zubat–Golbat |
| 4 | 10% | Marill–Azumarill | Seel–Dewgong |
| 5 | 10% | Wooper–Quagsire | Gastly–Haunter |
| 6 | 10% | Natu–Xatu | Marill–Azumarill |
| 7 | 5% | Zubat–Golbat | Zubat–Crobat |
| 8 | 5% | Slowpoke–Slowking | Slowpoke–Slowking |
| 9 | 4% | Skarmory | Skarmory |
| 10 | 4% | Corsola | Gastly–Haunter |
| 11 | 1% | Slowpoke–Slowking | Gastly–Gengar |
| 12 | 1% | Skarmory | Natu–Xatu |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Horsea–Seadra | Chinchou–Lanturn |
| 2 | 30% | Chinchou–Lanturn | Horsea–Seadra |
| 3 | 5% | Mantine | Mantine |
| 4 | 4% | Corsola | Horsea–Kingdra |
| 5 | 1% | Mantyke | Corsola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Horsea–Seadra | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Chinchou–Lanturn | Horsea–Seadra |
| 3 | 10% | 12% | 11% | Corsola | Corsola |
| 4 | 8% | 10% | 10% | Qwilfish | Staryu |
| 5 | 8% | 9% | 10% | Horsea–Seadra | Qwilfish |
| 6 | 4% | 7% | 10% | Corsola | Staryu |
| 7 | 3% | 6% | 10% | Staryu | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Remoraid–Octillery |
| 9 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Shellder–Cloyster |

**`MAP_WHIRL_ISLANDS_B1F_INNER_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Corsola | Seel–Dewgong |
| 2 | 20% | Seel–Dewgong | Corsola |
| 3 | 10% | Slowpoke–Slowbro | Zubat–Golbat |
| 4 | 10% | Marill–Azumarill | Slowpoke–Slowbro |
| 5 | 10% | Wooper–Quagsire | Gastly–Haunter |
| 6 | 10% | Natu–Xatu | Marill–Azumarill |
| 7 | 5% | Slowpoke–Slowking | Zubat–Crobat |
| 8 | 5% | Zubat–Golbat | Slowpoke–Slowking |
| 9 | 4% | Skarmory | Gastly–Gengar |
| 10 | 4% | Seel–Dewgong | Skarmory |
| 11 | 1% | Skarmory | Gastly–Haunter |
| 12 | 1% | Slowpoke–Slowking | Natu–Xatu |

**`MAP_WHIRL_ISLANDS_B2F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 10% | Corsola | Zubat–Golbat |
| 4 | 10% | Marill–Azumarill | Corsola |
| 5 | 10% | Wooper–Quagsire | Gastly–Haunter |
| 6 | 10% | Skarmory | Skarmory |
| 7 | 5% | Slowpoke–Slowking | Zubat–Crobat |
| 8 | 5% | Natu–Xatu | Gastly–Gengar |
| 9 | 4% | Skarmory | Slowpoke–Slowking |
| 10 | 4% | Zubat–Crobat | Skarmory |
| 11 | 1% | Natu–Xatu | Wooper–Quagsire |
| 12 | 1% | Slowpoke–Slowking | Natu–Xatu |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Horsea–Seadra | Qwilfish |
| 2 | 30% | Qwilfish | Horsea–Seadra |
| 3 | 5% | Lapras | Chinchou–Lanturn |
| 4 | 4% | Mantine | Lapras |
| 5 | 1% | Horsea–Kingdra | Horsea–Kingdra |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Qwilfish | Qwilfish |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Horsea–Seadra |
| 3 | 10% | 12% | 11% | Corsola | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Chinchou–Lanturn | Corsola |
| 5 | 8% | 9% | 10% | Qwilfish | Staryu |
| 6 | 4% | 7% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Horsea–Kingdra | Horsea–Kingdra |
| 8 | 3% | 5% | 9% | Remoraid–Octillery | Staryu |
| 9 | 2% | 4% | 9% | Shellder–Cloyster | Shellder–Cloyster |
| 10 | 2% | 4% | 9% | Staryu | Remoraid–Octillery |

**`MAP_WHIRL_ISLANDS_B3F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Corsola | Corsola |
| 2 | 20% | Seel–Dewgong | Seel–Dewgong |
| 3 | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 4 | 10% | Skarmory | Zubat–Golbat |
| 5 | 10% | Wooper–Quagsire | Gastly–Haunter |
| 6 | 10% | Marill–Azumarill | Skarmory |
| 7 | 5% | Slowpoke–Slowking | Zubat–Crobat |
| 8 | 5% | Skarmory | Gastly–Gengar |
| 9 | 4% | Natu–Xatu | Slowpoke–Slowking |
| 10 | 4% | Zubat–Crobat | Skarmory |
| 11 | 1% | Natu–Xatu | Wooper–Quagsire |
| 12 | 1% | Skarmory | Natu–Xatu |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Qwilfish | Horsea–Seadra |
| 2 | 30% | Horsea–Seadra | Qwilfish |
| 3 | 5% | Lapras | Chinchou–Lanturn |
| 4 | 4% | Mantine | Lapras |
| 5 | 1% | Horsea–Kingdra | Horsea–Kingdra |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Qwilfish | Horsea–Seadra |
| 2 | 22% | 18% | 10% | Horsea–Seadra | Qwilfish |
| 3 | 10% | 12% | 11% | Corsola | Chinchou–Lanturn |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Corsola |
| 5 | 8% | 9% | 10% | Horsea–Seadra | Staryu |
| 6 | 4% | 7% | 10% | Qwilfish | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Horsea–Kingdra | Horsea–Kingdra |
| 8 | 3% | 5% | 9% | Staryu | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Shellder–Cloyster | Shellder–Cloyster |
| 10 | 2% | 4% | 9% | Chinchou–Lanturn | Staryu |

**`MAP_WHIRL_ISLANDS_DESCENT_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seel–Dewgong | Seel–Dewgong |
| 2 | 20% | Natu–Xatu | Natu–Xatu |
| 3 | 10% | Corsola | Corsola |
| 4 | 10% | Slowpoke–Slowking | Slowpoke–Slowking |
| 5 | 10% | Skarmory | Gastly–Gengar |
| 6 | 10% | Zubat–Crobat | Skarmory |
| 7 | 5% | Skarmory | Zubat–Crobat |
| 8 | 5% | Wooper–Quagsire | Skarmory |
| 9 | 4% | Slowpoke–Slowking | Wooper–Quagsire |
| 10 | 4% | Natu–Xatu | Gastly–Gengar |
| 11 | 1% | Marill–Azumarill | Marill–Azumarill |
| 12 | 1% | Zubat–Crobat | Slowpoke–Slowking |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Horsea–Kingdra | Chinchou–Lanturn |
| 2 | 30% | Chinchou–Lanturn | Horsea–Kingdra |
| 3 | 5% | Lapras | Lapras |
| 4 | 4% | Mantine | Tentacool–Tentacruel |
| 5 | 1% | Tentacool–Tentacruel | Mantine |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Horsea–Kingdra | Chinchou–Lanturn |
| 2 | 22% | 18% | 10% | Chinchou–Lanturn | Horsea–Kingdra |
| 3 | 10% | 12% | 11% | Qwilfish | Qwilfish |
| 4 | 8% | 10% | 10% | Shellder–Cloyster | Staryu |
| 5 | 8% | 9% | 10% | Horsea–Kingdra | Shellder–Cloyster |
| 6 | 4% | 7% | 10% | Corsola | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Staryu | Horsea–Kingdra |
| 8 | 3% | 5% | 9% | Chinchou–Lanturn | Staryu |
| 9 | 2% | 4% | 9% | Shellder–Cloyster | Corsola |
| 10 | 2% | 4% | 9% | Remoraid–Octillery | Remoraid–Octillery |

#### Mt. Silver

Dungeon, Border.

**`MAP_MT_SILVER_OUTSIDE_HNS`**

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
| 9 | 4% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 10 | 4% | Skarmory | Houndour–Houndoom |
| 11 | 1% | Snorlax | Snorlax |
| 12 | 1% | Larvitar–Tyranitar | Murkrow |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 3 | 5% | Goldeen–Seaking | Wooper–Quagsire |
| 4 | 4% | Lapras | Lapras |
| 5 | 1% | Marill–Azumarill | Goldeen–Seaking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 2 | 22% | 18% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pineco–Forretress | Teddiursa–Ursaring |
| 2 | 30% | Teddiursa–Ursaring | Pineco–Forretress |
| 3 | 5% | Heracross | Hoothoot–Noctowl |
| 4 | 4% | Scyther | Heracross |
| 5 | 1% | Aipom | Scyther |

**`MAP_MT_SILVER_1F_WATERFALL_ROOM_HNS`**

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
| 9 | 4% | Electabuzz | Magmar |
| 10 | 4% | Magmar | Misdreavus–Mismagius |
| 11 | 1% | Onix–Steelix | Zubat–Crobat |
| 12 | 1% | Elekid | Magby |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Slowpoke–Slowbro |
| 2 | 30% | Slowpoke–Slowbro | Psyduck–Golduck |
| 3 | 5% | Lapras | Lapras |
| 4 | 4% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 1% | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Slowpoke–Slowbro |
| 2 | 22% | 18% | 10% | Slowpoke–Slowbro | Psyduck–Golduck |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Poliwag–Poliwrath | Poliwag–Politoed |
| 8 | 3% | 5% | 9% | Slowpoke–Slowbro | Slowpoke–Slowking |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Psyduck–Golduck |

**`MAP_MT_SILVER_MOUNTAIN_SIDE_HNS`**

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
| 9 | 4% | Electabuzz | Murkrow–Honchkrow |
| 10 | 4% | Skarmory | Gastly–Gengar |
| 11 | 1% | Electabuzz–Electivire | Snorlax |
| 12 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 2 | 30% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 5% | Lapras | Wooper–Quagsire |
| 4 | 4% | Slowpoke–Slowbro | Lapras |
| 5 | 1% | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Poliwag–Poliwrath | Poliwag–Poliwrath |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Psyduck–Golduck | Psyduck–Golduck |
| 4 | 8% | 10% | 10% | Slowpoke–Slowbro | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Poliwag–Poliwrath | Poliwag–Politoed |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Slowpoke–Slowking |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Poliwag–Politoed | Psyduck–Golduck |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pineco–Forretress | Teddiursa–Ursaring |
| 2 | 30% | Teddiursa–Ursaring | Pineco–Forretress |
| 3 | 5% | Heracross | Hoothoot–Noctowl |
| 4 | 4% | Scyther–Scizor | Heracross |
| 5 | 1% | Aipom–Ambipom | Scyther–Scizor |

**`MAP_MT_SILVER_1F_ITEM_ROOM_HNS`**

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
| 8 | 5% | Electabuzz | Magmar |
| 9 | 4% | Magmar | Electabuzz |
| 10 | 4% | Elekid | Gastly–Gengar |
| 11 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 12 | 1% | Magby | Magby |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 30% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 5% | Lapras | Wooper–Quagsire |
| 4 | 4% | Goldeen–Seaking | Lapras |
| 5 | 1% | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Psyduck–Golduck | Psyduck–Golduck |
| 2 | 22% | 18% | 10% | Slowpoke–Slowbro | Slowpoke–Slowbro |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Slowpoke–Slowking | Slowpoke–Slowking |
| 8 | 3% | 5% | 9% | Poliwag–Poliwrath | Poliwag–Politoed |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Psyduck–Golduck |

**`MAP_MT_SILVER_1F_MOLTRES_ROOM_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Paras–Parasect | Paras–Parasect |
| 2 | 20% | Machop–Machamp | Machop–Machamp |
| 3 | 10% | Onix | Zubat–Crobat |
| 4 | 10% | Geodude–Golem | Misdreavus–Mismagius |
| 5 | 10% | Magmar | Magmar |
| 6 | 10% | Electabuzz | Gastly–Gengar |
| 7 | 5% | Onix–Steelix | Onix–Steelix |
| 8 | 5% | Magmar–Magmortar | Magmar–Magmortar |
| 9 | 4% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 10 | 4% | Electabuzz–Electivire | Electabuzz |
| 11 | 1% | Magby | Magby |
| 12 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 30% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 5% | Lapras | Wooper–Quagsire |
| 4 | 4% | Magikarp–Gyarados | Lapras |
| 5 | 1% | Poliwag–Poliwrath | Poliwag–Politoed |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Goldeen–Seaking | Goldeen–Seaking |
| 2 | 22% | 18% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Poliwag–Poliwrath | Wooper–Quagsire |
| 5 | 8% | 9% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Wooper–Quagsire |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Poliwag–Politoed | Poliwag–Politoed |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Goldeen–Seaking | Goldeen–Seaking |

**`MAP_MT_SILVER_2F_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 20% | Machop–Machamp | Machop–Machamp |
| 3 | 10% | Psyduck–Golduck | Zubat–Crobat |
| 4 | 10% | Geodude–Golem | Misdreavus–Mismagius |
| 5 | 10% | Larvitar–Tyranitar | Geodude–Golem |
| 6 | 10% | Zubat–Crobat | Larvitar–Tyranitar |
| 7 | 5% | Electabuzz | Gastly–Gengar |
| 8 | 5% | Magmar | Magmar |
| 9 | 4% | Electabuzz–Electivire | Electabuzz |
| 10 | 4% | Magmar–Magmortar | Psyduck–Golduck |
| 11 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |
| 12 | 1% | Paras–Parasect | Snorlax |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 30% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 5% | Lapras | Lapras |
| 4 | 4% | Psyduck–Golduck | Poliwag–Politoed |
| 5 | 1% | Goldeen–Seaking | Goldeen–Seaking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wooper–Quagsire | Wooper–Quagsire |
| 2 | 22% | 18% | 10% | Magikarp–Gyarados | Magikarp–Gyarados |
| 3 | 10% | 12% | 11% | Psyduck–Golduck | Psyduck–Golduck |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Poliwag–Politoed |
| 5 | 8% | 9% | 10% | Wooper–Quagsire | Wooper–Quagsire |
| 6 | 4% | 7% | 10% | Magikarp–Gyarados | Poliwag–Politoed |
| 7 | 3% | 6% | 10% | Goldeen–Seaking | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Psyduck–Golduck | Psyduck–Golduck |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Lapras | Lapras |

**`MAP_MT_SILVER_SNOW_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Swinub–Mamoswine | Sneasel–Weavile |
| 2 | 20% | Teddiursa–Ursaring | Swinub–Mamoswine |
| 3 | 10% | Sneasel–Weavile | Houndour–Houndoom |
| 4 | 10% | Skarmory | Zubat–Crobat |
| 5 | 10% | Mankey–Annihilape | Misdreavus–Mismagius |
| 6 | 10% | Ponyta–Rapidash | Delibird |
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
| 8 | 5% | Skarmory | Larvitar–Tyranitar |
| 9 | 4% | Electabuzz–Electivire | Sneasel–Weavile |
| 10 | 4% | Magmar–Magmortar | Houndour–Houndoom |
| 11 | 1% | Snorlax | Snorlax |
| 12 | 1% | Larvitar–Tyranitar | Larvitar–Tyranitar |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Abra | Route 30, Route 34, Route 35, Ruins of Alph and more |
| Aipom | Azalea Town, Blackthorn City, Cherrygrove City, Cianwood City and more |
| Ambipom | Mt. Silver, Route 26, Route 28, Tin Tower |
| Ampharos | Route 31, Route 32, Route 43, Ruins of Alph and more |
| Annihilape | Mt. Silver |
| Arbok | Route 26, Route 27, Route 28, Route 32 and more |
| Ariados | Azalea Town, Blackthorn City, Cherrygrove City, Cianwood City and more |
| Azumarill | Blackthorn City, Dark Cave, Ecruteak City, Ilex Forest and more |
| Bayleef | Ilex Forest, New Bark Town, Route 27, Route 29 |
| Beedrill | Azalea Town, Ilex Forest, National Park, Route 30 and more |
| Bellsprout | Route 31, Route 32, Route 36, Route 44 and more |
| Bonsly | Route 36 |
| Butterfree | Azalea Town, Cherrygrove City, Ecruteak City, Goldenrod City and more |
| Caterpie | Azalea Town, Cherrygrove City, Ecruteak City, Goldenrod City and more |
| Chikorita | Ilex Forest, New Bark Town, Route 27, Route 29 |
| Chinchou | Cianwood City, Cliff Edge Cave, Dark Cave, Mt. Mortar and more |
| Cleffa | Mt. Mortar |
| Cloyster | Route 26, Whirl Islands |
| Corsola | Cherrygrove City, Cianwood City, Cliff Edge Cave, New Bark Town and more |
| Crobat | Ice Path, Mt. Mortar, Mt. Silver, Route 28 and more |
| Croconaw | Lake of Rage, Route 27, Route 30 |
| Cubone | Mt. Mortar, Union Cave |
| Cyndaquil | Burned Tower, Route 46 |
| Delibird | Ice Path, Mt. Silver |
| Dewgong | Cliff Edge Cave, Ice Path, Route 47, Whirl Islands |
| Ditto | Cianwood City, Olivine City, Route 34, Route 35 and more |
| Dodrio | Mt. Silver, Route 26, Route 27, Route 28 |
| Doduo | Mt. Silver, Route 26, Route 27, Route 28 |
| Donphan | Blackthorn City, Mt. Silver, Route 26, Route 28 and more |
| Dragonair | Dragon's Den |
| Dragonite | Dragon's Den |
| Dratini | Dragon's Den |
| Drowzee | Route 34, Route 35 |
| Dunsparce | Dark Cave, Mt. Mortar, Route 31, Route 33 and more |
| Eevee | Cherrygrove City, Ecruteak City, Route 34, Route 37 and more |
| Ekans | Route 26, Route 27, Route 28, Route 32 and more |
| Electabuzz | Mt. Silver |
| Electivire | Mt. Silver |
| Electrode | Rocket Hideout |
| Elekid | Mt. Silver, Route 44 |
| Espeon | Tin Tower |
| Exeggcute | Azalea Town, Blackthorn City, Cherrygrove City, Cianwood City and more |
| Farfetch'd | Olivine City, Route 38, Route 39, Route 43 and more |
| Farigiraf | Route 28 |
| Fearow | Blackthorn City, Cherrygrove City, Cianwood City, Mt. Silver and more |
| Feraligatr | Lake of Rage, Route 27, Route 30 |
| Flaaffy | Route 31, Route 32, Route 43, Ruins of Alph and more |
| Forretress | Azalea Town, Blackthorn City, Ecruteak City, Goldenrod City and more |
| Furret | Cherrygrove City, Ecruteak City, New Bark Town, Route 29 and more |
| Gastly | Blackthorn City, Burned Tower, Cherrygrove City, Cianwood City and more |
| Gengar | Burned Tower, Mt. Mortar, Mt. Silver, Route 26 and more |
| Geodude | Blackthorn City, Cianwood City, Cliff Edge Cave, Dark Cave and more |
| Girafarig | National Park, Route 27, Route 28, Route 47 and more |
| Gligar | Blackthorn City, Dark Cave, Route 42, Route 45 and more |
| Gloom | Blackthorn City, Cherrygrove City, Cianwood City, Ecruteak City and more |
| Golbat | Blackthorn City, Burned Tower, Cianwood City, Cliff Edge Cave and more |
| Goldeen | Blackthorn City, Dark Cave, Ecruteak City, Ilex Forest and more |
| Golduck | Blackthorn City, Ecruteak City, Ilex Forest, Lake of Rage and more |
| Golem | Mt. Mortar, Mt. Silver, Route 26 |
| Granbull | National Park, Olivine City, Route 34, Route 35 and more |
| Graveler | Blackthorn City, Cianwood City, Cliff Edge Cave, Dark Cave and more |
| Grimer | Olivine City, Rocket Hideout, Route 34 |
| Growlithe | Ecruteak City, Route 35, Route 36, Route 37 and more |
| Gyarados | Blackthorn City, Cherrygrove City, Dark Cave, Dragon's Den and more |
| Happiny | Route 47 |
| Haunter | Blackthorn City, Burned Tower, Cherrygrove City, Cianwood City and more |
| Heracross | Lake of Rage, Mt. Silver, National Park, Route 26 and more |
| Hitmontop | Mt. Mortar |
| Honchkrow | Mt. Silver, Route 26, Route 28, Tin Tower |
| Hoothoot | Azalea Town, Blackthorn City, Cherrygrove City, Cianwood City and more |
| Hoppip | Blackthorn City, Cherrygrove City, Cianwood City, Ecruteak City and more |
| Horsea | Blackthorn City, Dragon's Den, Route 26, Route 41 and more |
| Houndoom | Burned Tower, Ecruteak City, Mt. Silver, Route 26 and more |
| Houndour | Burned Tower, Ecruteak City, Mt. Silver, Route 26 and more |
| Hypno | Route 34, Route 35 |
| Igglybuff | Route 34 |
| Jigglypuff | Route 34 |
| Jumpluff | Blackthorn City, Cherrygrove City, Cianwood City, Ecruteak City and more |
| Jynx | Ice Path, Mt. Silver |
| Kadabra | Route 34, Route 35 |
| Kakuna | Azalea Town, Ilex Forest, National Park, Route 30 and more |
| Kingdra | Dragon's Den, Route 26, Whirl Islands |
| Kingler | Cherrygrove City, Cianwood City, Cliff Edge Cave, New Bark Town and more |
| Koffing | Burned Tower, Rocket Hideout, Slowpoke Well |
| Krabby | Cherrygrove City, Cianwood City, Cliff Edge Cave, New Bark Town and more |
| Lanturn | Cianwood City, Cliff Edge Cave, Dark Cave, Mt. Mortar and more |
| Lapras | Mt. Silver, Route 26, Union Cave, Whirl Islands |
| Larvitar | Mt. Silver, Route 26, Route 28 |
| Ledian | Cherrygrove City, Ecruteak City, Goldenrod City, Ilex Forest and more |
| Ledyba | Cherrygrove City, Ecruteak City, Goldenrod City, Ilex Forest and more |
| Lickitung | Route 44 |
| Machamp | Mt. Mortar, Mt. Silver |
| Machoke | Blackthorn City, Cianwood City, Dark Cave, Mt. Mortar and more |
| Machop | Blackthorn City, Cianwood City, Dark Cave, Mt. Mortar and more |
| Magby | Burned Tower, Mt. Silver |
| Magcargo | Burned Tower |
| Magikarp | Blackthorn City, Cherrygrove City, Dark Cave, Dragon's Den and more |
| Magmar | Burned Tower, Mt. Silver |
| Magmortar | Burned Tower, Mt. Silver |
| Magnemite | Olivine City, Rocket Hideout, Route 38, Route 39 |
| Magneton | Olivine City, Rocket Hideout, Route 38, Route 39 |
| Mamoswine | Ice Path, Mt. Silver |
| Mankey | Cianwood City, Mt. Mortar, Mt. Silver, Route 26 and more |
| Mantine | Cliff Edge Cave, Route 26, Route 27, Route 47 and more |
| Mantyke | Cianwood City, Olivine City, Route 41, Whirl Islands |
| Mareep | Route 31, Route 32, Route 43, Ruins of Alph and more |
| Marill | Blackthorn City, Dark Cave, Ecruteak City, Ilex Forest and more |
| Marowak | Mt. Mortar, Union Cave |
| Meganium | Ilex Forest, New Bark Town, Route 27, Route 29 |
| Meowth | Cherrygrove City, Olivine City, Rocket Hideout, Route 38 and more |
| Metapod | Azalea Town, Cherrygrove City, Ecruteak City, Goldenrod City and more |
| Miltank | Route 26, Route 27, Route 47 |
| Misdreavus | Burned Tower, Ecruteak City, Ilex Forest, Mt. Silver and more |
| Mismagius | Burned Tower, Mt. Silver, Tin Tower |
| Muk | Olivine City, Rocket Hideout, Route 34 |
| Murkrow | Azalea Town, Burned Tower, Ecruteak City, Goldenrod City and more |
| Natu | Ruins of Alph, Sprout Tower, Tin Tower, Violet City and more |
| Nidoran♀ | National Park, Route 35, Route 48 |
| Nidoran♂ | National Park, Route 35, Route 36 |
| Nidorina | National Park, Route 35, Route 48 |
| Nidorino | National Park, Route 35, Route 36 |
| Noctowl | Azalea Town, Blackthorn City, Cherrygrove City, Cianwood City and more |
| Octillery | Cianwood City, New Bark Town, Olivine City, Route 26 and more |
| Oddish | Blackthorn City, Cherrygrove City, Cianwood City, Ecruteak City and more |
| Onix | Cliff Edge Cave, Dark Cave, Ice Path, Mt. Mortar and more |
| Paras | Ilex Forest, Mt. Silver, National Park |
| Parasect | Ilex Forest, Mt. Silver, National Park |
| Persian | Cherrygrove City, Olivine City, Rocket Hideout, Route 38 and more |
| Phanpy | Blackthorn City, Mt. Silver, Route 26, Route 28 and more |
| Pichu | Ilex Forest, Route 31 |
| Pidgeot | Blackthorn City, Cherrygrove City, Ecruteak City, Mahogany Town and more |
| Pidgeotto | Blackthorn City, Cherrygrove City, Ecruteak City, Mahogany Town and more |
| Pidgey | Blackthorn City, Cherrygrove City, Ecruteak City, Mahogany Town and more |
| Piloswine | Blackthorn City, Ice Path, Mt. Silver |
| Pineco | Azalea Town, Blackthorn City, Ecruteak City, Goldenrod City and more |
| Pinsir | National Park |
| Politoed | Mt. Silver, Route 28 |
| Poliwag | Blackthorn City, Dark Cave, Ecruteak City, Ilex Forest and more |
| Poliwhirl | Blackthorn City, Dark Cave, Ecruteak City, Ilex Forest and more |
| Poliwrath | Mt. Silver, Route 28 |
| Ponyta | Mt. Silver, Route 26, Route 27, Route 28 and more |
| Porygon | Rocket Hideout |
| Porygon2 | Rocket Hideout |
| Primeape | Cianwood City, Mt. Mortar, Mt. Silver, Route 26 and more |
| Psyduck | Blackthorn City, Ecruteak City, Ilex Forest, Lake of Rage and more |
| Pupitar | Mt. Silver, Route 26, Route 28 |
| Quagsire | Blackthorn City, Cliff Edge Cave, Dark Cave, Ecruteak City and more |
| Quilava | Burned Tower, Route 46 |
| Qwilfish | Cherrygrove City, Cianwood City, New Bark Town, Olivine City and more |
| Rapidash | Mt. Silver, Route 26, Route 27, Route 28 and more |
| Raticate | Burned Tower, Cherrygrove City, Mt. Mortar, New Bark Town and more |
| Rattata | Burned Tower, Cherrygrove City, Mt. Mortar, New Bark Town and more |
| Remoraid | Cianwood City, New Bark Town, Olivine City, Route 26 and more |
| Sandshrew | Route 26, Route 27, Union Cave |
| Sandslash | Route 26, Route 27, Union Cave |
| Scizor | Mt. Silver, Route 26, Route 28 |
| Scyther | Mt. Silver, National Park, Route 26, Route 28 |
| Seadra | Blackthorn City, Dragon's Den, Route 26, Route 41 and more |
| Seaking | Blackthorn City, Dark Cave, Ecruteak City, Ilex Forest and more |
| Seel | Cliff Edge Cave, Ice Path, Route 47, Whirl Islands |
| Sentret | Cherrygrove City, Ecruteak City, New Bark Town, Route 29 and more |
| Shellder | Cherrygrove City, Cianwood City, Cliff Edge Cave, New Bark Town and more |
| Shuckle | Cliff Edge Cave, Route 47 |
| Skarmory | Cliff Edge Cave, Mt. Silver, Route 28, Route 47 and more |
| Skiploom | Blackthorn City, Cherrygrove City, Cianwood City, Ecruteak City and more |
| Slowbro | Mt. Silver, Olivine City, Route 33, Slowpoke Well and more |
| Slowking | Mt. Silver, Slowpoke Well, Whirl Islands |
| Slowpoke | Mt. Silver, Olivine City, Route 33, Slowpoke Well and more |
| Slugma | Burned Tower |
| Smeargle | Ruins of Alph |
| Smoochum | Ice Path, Mt. Silver |
| Sneasel | Blackthorn City, Ice Path, Mt. Silver, Route 28 |
| Snorlax | Mt. Silver, Route 28 |
| Snubbull | National Park, Olivine City, Route 34, Route 35 and more |
| Spearow | Blackthorn City, Cherrygrove City, Cianwood City, Mt. Silver and more |
| Spinarak | Azalea Town, Blackthorn City, Cherrygrove City, Cianwood City and more |
| Stantler | National Park, Tin Tower |
| Starmie | Route 26 |
| Staryu | Cherrygrove City, Cianwood City, Cliff Edge Cave, New Bark Town and more |
| Steelix | Ice Path, Mt. Silver, Union Cave |
| Sudowoodo | Route 36 |
| Sunkern | Blackthorn City, Cherrygrove City, Cianwood City, Ecruteak City and more |
| Swinub | Blackthorn City, Ice Path, Mt. Silver |
| Tangela | Route 28, Route 44 |
| Tangrowth | Route 28 |
| Tauros | Mt. Silver, Route 26, Route 27, Route 28 and more |
| Teddiursa | Dark Cave, Mt. Silver, Route 26, Route 28 and more |
| Tentacool | Cherrygrove City, Cianwood City, Cliff Edge Cave, New Bark Town and more |
| Tentacruel | Cherrygrove City, Cianwood City, Cliff Edge Cave, New Bark Town and more |
| Togepi | National Park, New Bark Town |
| Totodile | Lake of Rage, Route 27, Route 30 |
| Typhlosion | Burned Tower, Route 46 |
| Tyranitar | Mt. Silver, Route 26, Route 28 |
| Tyrogue | Cianwood City, Mt. Mortar, Route 42 |
| Umbreon | Tin Tower |
| Unown | Ruins of Alph |
| Ursaring | Dark Cave, Mt. Silver, Route 26, Route 28 and more |
| Venomoth | Ilex Forest, Lake of Rage, National Park, Route 26 and more |
| Venonat | Ilex Forest, Lake of Rage, National Park, Route 26 and more |
| Victreebel | Sprout Tower |
| Voltorb | Rocket Hideout |
| Vulpix | Burned Tower, Ecruteak City, Route 36, Route 37 and more |
| Weavile | Ice Path, Mt. Silver, Route 28 |
| Weedle | Azalea Town, Ilex Forest, National Park, Route 30 and more |
| Weepinbell | Route 31, Route 32, Route 36, Route 44 and more |
| Weezing | Burned Tower, Rocket Hideout, Slowpoke Well |
| Wobbuffet | Dark Cave, Slowpoke Well, Union Cave |
| Wooper | Blackthorn City, Cliff Edge Cave, Dark Cave, Ecruteak City and more |
| Xatu | Ruins of Alph, Sprout Tower, Tin Tower, Violet City and more |
| Yanma | Ilex Forest, Lake of Rage, National Park, Route 35 and more |
| Zubat | Blackthorn City, Burned Tower, Cianwood City, Cliff Edge Cave and more |
