# Alola encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: Yes (Wayfarer)

Design status: v0 approved. These tables follow the
[Alola encounter rules](alola-encounters.md). They are the source of truth for
Alola's wild-encounter data: every slot below maps one to one to a slot in the
game's encounter tables. Playtesting may change the picks.

## Scope

This spec lists Alola's wild-encounter tables: 10 maps, each with a day and a
night table for every method it has. Legendaries, mythicals, Ultra Beasts,
Type: Null and Cosmog's line belong to a later spec.

## Behavior

### Reading the tables

- Places are ordered by reach: Road, Wilds, Outlands, then dungeons. Each place
  names its reach and its island, and each map is named by its map
  constant. A map with surfing or fishing also names its water type.
- Each table lists every slot in order, with its weight, and the species for
  day and night.
  - Land has 12 slots weighted 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1 and 1%.
  - Surfing has 5 slots weighted 60, 30, 5, 4 and 1%.
  - Fishing has the [Standard Rod](standard-rod-fishing.md)'s 10 entries, with
    each entry's weight for the Old, Good and Super Rod.
- **A slot's species is its stage cap,** the last name in the cell.
  "Fomantis–Lurantis" means the slot holds Lurantis, and the game steps it down
  to Fomantis below Lurantis's evolution level. A single name is a single-stage
  species, a baby, or a line capped at its first stage.
- Names map to species constants in capitals, with spaces and hyphens as
  underscores and other punctuation dropped: Jangmo-o is `SPECIES_JANGMO_O`.
  - "Alolan" marks an Alolan form: Alolan Raticate is
    `SPECIES_RATICATE_ALOLA`.
  - A form in brackets follows the name: Oricorio (Pa'u) is
    `SPECIES_ORICORIO_PAU`, Lycanroc (Midnight) is `SPECIES_LYCANROC_MIDNIGHT`
    and Rockruff (Own Tempo) is `SPECIES_ROCKRUFF_OWN_TEMPO`.
- Slots hold no levels. Levels come from the map's reach, as the PRD defines.

### Tables

#### Melemele Isle

Road, Melemele.

**`MAP_MELEMELE_ISLE_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Pikipek–Toucannon | Alolan Rattata–Alolan Raticate |
| 2 | 20% | Yungoos–Gumshoos | Alolan Meowth |
| 3 | 10% | Grubbin–Charjabug | Grubbin–Charjabug |
| 4 | 10% | Cutiefly–Ribombee | Alolan Grimer–Alolan Muk |
| 5 | 10% | Crabrawler | Spinarak–Ariados |
| 6 | 10% | Pikachu | Crabrawler |
| 7 | 5% | Rockruff–Lycanroc (Midday) | Rockruff–Lycanroc (Midnight) |
| 8 | 5% | Wingull–Pelipper | Alolan Rattata–Alolan Raticate |
| 9 | 4% | Alolan Grimer–Alolan Muk | Alolan Grimer–Alolan Muk |
| 10 | 4% | Bounsweet–Steenee | Pikachu |
| 11 | 1% | Rowlet–Decidueye | Rowlet–Decidueye |
| 12 | 1% | Pichu | Pichu |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Mareanie–Toxapex | Pyukumuku |
| 2 | 30% | Pyukumuku | Mareanie–Toxapex |
| 3 | 5% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 4 | 4% | Wingull–Pelipper | Chinchou–Lanturn |
| 5 | 1% | Popplio–Primarina | Popplio–Primarina |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Mareanie–Toxapex | Pyukumuku |
| 2 | 22% | 18% | 10% | Pyukumuku | Mareanie–Toxapex |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Wishiwashi | Chinchou |
| 5 | 8% | 9% | 10% | Wishiwashi | Wishiwashi |
| 6 | 4% | 7% | 10% | Luvdisc | Staryu |
| 7 | 3% | 6% | 10% | Pyukumuku | Pyukumuku |
| 8 | 3% | 5% | 9% | Corsola | Mareanie–Toxapex |
| 9 | 2% | 4% | 9% | Mareanie–Toxapex | Corsola |
| 10 | 2% | 4% | 9% | Popplio–Primarina | Popplio–Primarina |

#### Akala Isle

Wilds, Akala.

**`MAP_AKALA_ISLE_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Mudbray–Mudsdale | Morelull–Shiinotic |
| 2 | 20% | Fomantis–Lurantis | Mudbray–Mudsdale |
| 3 | 10% | Bounsweet–Steenee | Alolan Rattata–Alolan Raticate |
| 4 | 10% | Alolan Diglett–Alolan Dugtrio | Fomantis–Lurantis |
| 5 | 10% | Sandygast–Palossand | Cubone–Alolan Marowak |
| 6 | 10% | Dewpider–Araquanid | Sandygast–Palossand |
| 7 | 5% | Stufful–Bewear | Salandit–Salazzle |
| 8 | 5% | Cubone–Alolan Marowak | Wimpod–Golisopod |
| 9 | 4% | Oricorio (Pa'u) | Dewpider–Araquanid |
| 10 | 4% | Wimpod–Golisopod | Oricorio (Pa'u) |
| 11 | 1% | Comfey | Litten–Incineroar |
| 12 | 1% | Litten–Incineroar | Comfey |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wishiwashi | Mareanie–Toxapex |
| 2 | 30% | Mareanie–Toxapex | Wishiwashi |
| 3 | 5% | Wingull–Pelipper | Chinchou–Lanturn |
| 4 | 4% | Pyukumuku | Pyukumuku |
| 5 | 1% | Bruxish | Bruxish |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wishiwashi | Mareanie–Toxapex |
| 2 | 22% | 18% | 10% | Mareanie–Toxapex | Wishiwashi |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Pyukumuku | Chinchou |
| 5 | 8% | 9% | 10% | Mareanie–Toxapex | Pyukumuku |
| 6 | 4% | 7% | 10% | Wishiwashi | Wishiwashi |
| 7 | 3% | 6% | 10% | Clamperl | Clamperl |
| 8 | 3% | 5% | 9% | Corsola | Corsola |
| 9 | 2% | 4% | 9% | Luvdisc | Staryu |
| 10 | 2% | 4% | 9% | Bruxish | Bruxish |

#### Alola sea

Wilds, Alola sea.

**`MAP_ALOLA_WATER_HNS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wishiwashi | Wishiwashi |
| 2 | 30% | Mareanie–Toxapex | Mareanie–Toxapex |
| 3 | 5% | Wailmer | Chinchou–Lanturn |
| 4 | 4% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 5 | 1% | Bruxish | Bruxish |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wishiwashi | Wishiwashi |
| 2 | 22% | 18% | 10% | Mareanie–Toxapex | Mareanie–Toxapex |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Mareanie–Toxapex | Chinchou |
| 5 | 8% | 9% | 10% | Wishiwashi | Mareanie–Toxapex |
| 6 | 4% | 7% | 10% | Carvanha | Wishiwashi |
| 7 | 3% | 6% | 10% | Mareanie–Toxapex | Staryu |
| 8 | 3% | 5% | 9% | Clamperl | Mareanie–Toxapex |
| 9 | 2% | 4% | 9% | Wailmer | Wailmer |
| 10 | 2% | 4% | 9% | Bruxish | Bruxish |

#### Akala Forest

Wilds, Akala.

**`MAP_AKALA_FOREST_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bounsweet–Steenee | Morelull–Shiinotic |
| 2 | 20% | Fomantis–Lurantis | Fomantis–Lurantis |
| 3 | 10% | Cutiefly–Ribombee | Spinarak–Ariados |
| 4 | 10% | Grubbin–Charjabug | Alolan Rattata–Alolan Raticate |
| 5 | 10% | Pikipek–Toucannon | Venonat–Venomoth |
| 6 | 10% | Paras–Parasect | Grubbin–Charjabug |
| 7 | 5% | Morelull–Shiinotic | Bounsweet–Steenee |
| 8 | 5% | Exeggcute | Hoothoot–Noctowl |
| 9 | 4% | Passimian | Oranguru |
| 10 | 4% | Oricorio (Pom-Pom) | Oricorio (Pom-Pom) |
| 11 | 1% | Comfey | Rowlet–Decidueye |
| 12 | 1% | Rowlet–Decidueye | Comfey |

#### Ula'ula Isle

Outlands, Ula'ula.

**`MAP_ULAULA_ISLE_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Togedemaru | Togedemaru |
| 2 | 20% | Crabrawler–Crabominable | Alolan Grimer–Alolan Muk |
| 3 | 10% | Alolan Grimer–Alolan Muk | Alolan Meowth–Alolan Persian |
| 4 | 10% | Pikipek–Toucannon | Crabrawler–Crabominable |
| 5 | 10% | Stufful–Bewear | Alolan Rattata–Alolan Raticate |
| 6 | 10% | Yungoos–Gumshoos | Morelull–Shiinotic |
| 7 | 5% | Alolan Vulpix–Alolan Ninetales | Alolan Sandshrew–Alolan Sandslash |
| 8 | 5% | Turtonator | Drampa |
| 9 | 4% | Minior | Oranguru |
| 10 | 4% | Oricorio (Baile) | Oricorio (Baile) |
| 11 | 1% | Komala | Komala |
| 12 | 1% | Passimian | Minior |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wishiwashi | Mareanie–Toxapex |
| 2 | 30% | Pyukumuku | Wishiwashi |
| 3 | 5% | Wingull–Pelipper | Chinchou–Lanturn |
| 4 | 4% | Shellder–Cloyster | Wingull–Pelipper |
| 5 | 1% | Bruxish | Dhelmise |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wishiwashi | Mareanie–Toxapex |
| 2 | 22% | 18% | 10% | Mareanie–Toxapex | Wishiwashi |
| 3 | 10% | 12% | 11% | Pyukumuku | Pyukumuku |
| 4 | 8% | 10% | 10% | Magikarp | Chinchou |
| 5 | 8% | 9% | 10% | Mareanie–Toxapex | Magikarp |
| 6 | 4% | 7% | 10% | Luvdisc | Shellder |
| 7 | 3% | 6% | 10% | Wailmer | Clamperl |
| 8 | 3% | 5% | 9% | Corsola | Wailmer |
| 9 | 2% | 4% | 9% | Bruxish | Bruxish |
| 10 | 2% | 4% | 9% | Dhelmise | Dhelmise |

#### Poni Isle

Outlands, Poni.

**`MAP_PONI_ISLE_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Mudbray–Mudsdale | Mudbray–Mudsdale |
| 2 | 20% | Exeggcute–Alolan Exeggutor | Exeggcute–Alolan Exeggutor |
| 3 | 10% | Wimpod–Golisopod | Morelull–Shiinotic |
| 4 | 10% | Sandygast–Palossand | Alolan Rattata–Alolan Raticate |
| 5 | 10% | Yungoos–Gumshoos | Alolan Meowth–Alolan Persian |
| 6 | 10% | Fomantis–Lurantis | Wimpod–Golisopod |
| 7 | 5% | Jangmo-o–Kommo-o | Jangmo-o–Kommo-o |
| 8 | 5% | Oricorio (Sensu) | Oricorio (Sensu) |
| 9 | 4% | Passimian | Oranguru |
| 10 | 4% | Bounsweet–Tsareena | Rockruff–Lycanroc (Midnight) |
| 11 | 1% | Grubbin–Vikavolt | Salandit–Salazzle |
| 12 | 1% | Pikachu–Alolan Raichu | Sandygast–Palossand |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Mareanie–Toxapex | Wishiwashi |
| 2 | 30% | Wishiwashi | Pyukumuku |
| 3 | 5% | Wingull–Pelipper | Chinchou–Lanturn |
| 4 | 4% | Wailmer | Carvanha–Sharpedo |
| 5 | 1% | Dhelmise | Dhelmise |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Mareanie–Toxapex | Wishiwashi |
| 2 | 22% | 18% | 10% | Wishiwashi | Mareanie–Toxapex |
| 3 | 10% | 12% | 11% | Pyukumuku | Pyukumuku |
| 4 | 8% | 10% | 10% | Magikarp | Chinchou |
| 5 | 8% | 9% | 10% | Corsola | Magikarp |
| 6 | 4% | 7% | 10% | Carvanha | Carvanha |
| 7 | 3% | 6% | 10% | Staryu | Corsola |
| 8 | 3% | 5% | 9% | Clamperl | Staryu |
| 9 | 2% | 4% | 9% | Bruxish | Bruxish |
| 10 | 2% | 4% | 9% | Dhelmise | Dhelmise |

#### Akala Cave

Dungeon, Akala.

**`MAP_AKALA_CAVE_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Salandit–Salazzle | Salandit–Salazzle |
| 2 | 20% | Alolan Geodude–Alolan Graveler | Alolan Geodude–Alolan Graveler |
| 3 | 10% | Alolan Diglett–Alolan Dugtrio | Zubat–Golbat |
| 4 | 10% | Cubone–Alolan Marowak | Cubone–Alolan Marowak |
| 5 | 10% | Zubat–Golbat | Alolan Diglett–Alolan Dugtrio |
| 6 | 10% | Slugma–Magcargo | Slugma–Magcargo |
| 7 | 5% | Alolan Geodude–Alolan Golem | Woobat–Swoobat |
| 8 | 5% | Woobat–Swoobat | Alolan Geodude–Alolan Golem |
| 9 | 4% | Turtonator | Numel–Camerupt |
| 10 | 4% | Numel–Camerupt | Misdreavus |
| 11 | 1% | Litten–Incineroar | Litten–Incineroar |
| 12 | 1% | Rockruff (Own Tempo)–Lycanroc (Dusk) | Rockruff (Own Tempo)–Lycanroc (Dusk) |

#### Ula'ula Cave

Dungeon, Ula'ula.

**`MAP_ULA_ULA_CAVE_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Crabrawler–Crabominable | Crabrawler–Crabominable |
| 2 | 20% | Alolan Vulpix–Alolan Ninetales | Alolan Sandshrew–Alolan Sandslash |
| 3 | 10% | Alolan Sandshrew–Alolan Sandslash | Alolan Vulpix–Alolan Ninetales |
| 4 | 10% | Snorunt–Glalie | Snorunt–Glalie |
| 5 | 10% | Vanillite–Vanilluxe | Vanillite–Vanilluxe |
| 6 | 10% | Drampa | Mimikyu |
| 7 | 5% | Mimikyu | Drampa |
| 8 | 5% | Alolan Geodude–Alolan Graveler | Zubat–Golbat |
| 9 | 4% | Zubat–Golbat | Alolan Geodude–Alolan Graveler |
| 10 | 4% | Drampa | Misdreavus |
| 11 | 1% | Alolan Vulpix–Alolan Ninetales | Alolan Sandshrew–Alolan Sandslash |
| 12 | 1% | Jangmo-o–Kommo-o | Jangmo-o–Kommo-o |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Dewpider–Araquanid | Dewpider–Araquanid |
| 2 | 30% | Wishiwashi | Wishiwashi |
| 3 | 5% | Magikarp | Barboach–Whiscash |
| 4 | 4% | Barboach–Whiscash | Zubat–Golbat |
| 5 | 1% | Dewpider–Araquanid | Basculin |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Dewpider–Araquanid | Wishiwashi |
| 2 | 22% | 18% | 10% | Wishiwashi | Dewpider–Araquanid |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Barboach–Whiscash | Barboach–Whiscash |
| 5 | 8% | 9% | 10% | Dewpider–Araquanid | Basculin |
| 6 | 4% | 7% | 10% | Wishiwashi | Wishiwashi |
| 7 | 3% | 6% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 8 | 3% | 5% | 9% | Dewpider–Araquanid | Dewpider–Araquanid |
| 9 | 2% | 4% | 9% | Wishiwashi | Barboach–Whiscash |
| 10 | 2% | 4% | 9% | Dewpider–Araquanid | Basculin |

#### Ula'ula Cave 2

Dungeon, Ula'ula.

**`MAP_ULA_ULA_CAVE_2_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Alolan Grimer–Alolan Muk | Alolan Grimer–Alolan Muk |
| 2 | 20% | Alolan Rattata–Alolan Raticate | Alolan Rattata–Alolan Raticate |
| 3 | 10% | Mimikyu | Mimikyu |
| 4 | 10% | Zubat–Golbat | Gastly–Haunter |
| 5 | 10% | Gastly–Haunter | Zubat–Golbat |
| 6 | 10% | Cubone–Alolan Marowak | Cubone–Alolan Marowak |
| 7 | 5% | Alolan Geodude–Alolan Golem | Alolan Geodude–Alolan Golem |
| 8 | 5% | Alolan Geodude–Alolan Graveler | Misdreavus |
| 9 | 4% | Grubbin–Vikavolt | Grubbin–Vikavolt |
| 10 | 4% | Grubbin–Charjabug | Grubbin–Charjabug |
| 11 | 1% | Alolan Grimer–Alolan Muk | Mimikyu |
| 12 | 1% | Mimikyu | Cubone–Alolan Marowak |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Dewpider–Araquanid | Dewpider–Araquanid |
| 2 | 30% | Wishiwashi | Wishiwashi |
| 3 | 5% | Psyduck–Golduck | Zubat–Golbat |
| 4 | 4% | Barboach–Whiscash | Barboach–Whiscash |
| 5 | 1% | Dewpider–Araquanid | Basculin |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wishiwashi | Dewpider–Araquanid |
| 2 | 22% | 18% | 10% | Dewpider–Araquanid | Wishiwashi |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Psyduck–Golduck | Barboach–Whiscash |
| 5 | 8% | 9% | 10% | Wishiwashi | Basculin |
| 6 | 4% | 7% | 10% | Dewpider–Araquanid | Dewpider–Araquanid |
| 7 | 3% | 6% | 10% | Wishiwashi | Wishiwashi |
| 8 | 3% | 5% | 9% | Dewpider–Araquanid | Dewpider–Araquanid |
| 9 | 2% | 4% | 9% | Wishiwashi | Basculin |
| 10 | 2% | 4% | 9% | Dewpider–Araquanid | Wishiwashi |

#### Poni Cave

Dungeon, Poni.

**`MAP_PONI_CAVE_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Alolan Geodude–Alolan Golem | Alolan Geodude–Alolan Golem |
| 2 | 20% | Rockruff–Lycanroc (Midday) | Rockruff–Lycanroc (Midnight) |
| 3 | 10% | Jangmo-o–Kommo-o | Jangmo-o–Kommo-o |
| 4 | 10% | Grubbin–Vikavolt | Mimikyu |
| 5 | 10% | Zubat–Golbat | Zubat–Golbat |
| 6 | 10% | Alolan Diglett–Alolan Dugtrio | Alolan Diglett–Alolan Dugtrio |
| 7 | 5% | Roggenrola–Gigalith | Roggenrola–Gigalith |
| 8 | 5% | Alolan Geodude–Alolan Graveler | Alolan Geodude–Alolan Graveler |
| 9 | 4% | Cubone–Alolan Marowak | Cubone–Alolan Marowak |
| 10 | 4% | Alolan Geodude–Alolan Graveler | Sableye |
| 11 | 1% | Jangmo-o–Kommo-o | Jangmo-o–Kommo-o |
| 12 | 1% | Mimikyu | Alolan Diglett–Alolan Dugtrio |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Dewpider–Araquanid | Dewpider–Araquanid |
| 2 | 30% | Wishiwashi | Wishiwashi |
| 3 | 5% | Barboach–Whiscash | Barboach–Whiscash |
| 4 | 4% | Psyduck–Golduck | Zubat–Golbat |
| 5 | 1% | Dewpider–Araquanid | Basculin |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wishiwashi | Dewpider–Araquanid |
| 2 | 22% | 18% | 10% | Dewpider–Araquanid | Wishiwashi |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Barboach–Whiscash | Barboach–Whiscash |
| 5 | 8% | 9% | 10% | Psyduck–Golduck | Basculin |
| 6 | 4% | 7% | 10% | Dewpider–Araquanid | Psyduck–Golduck |
| 7 | 3% | 6% | 10% | Wishiwashi | Wishiwashi |
| 8 | 3% | 5% | 9% | Barboach–Whiscash | Dewpider–Araquanid |
| 9 | 2% | 4% | 9% | Dewpider–Araquanid | Basculin |
| 10 | 2% | 4% | 9% | Wishiwashi | Wishiwashi |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Araquanid | Akala Isle, Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Ariados | Akala Forest, Melemele Isle |
| Barboach | Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Basculin | Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Bewear | Akala Isle, Ula'ula Isle |
| Boldore | Poni Cave |
| Bounsweet | Akala Forest, Akala Isle, Melemele Isle, Poni Isle |
| Brionne | Melemele Isle |
| Bruxish | Akala Isle, Alola sea, Poni Isle, Ula'ula Isle |
| Camerupt | Akala Cave |
| Carvanha | Alola sea, Poni Isle |
| Charjabug | Akala Forest, Melemele Isle, Poni Cave, Poni Isle and more |
| Chinchou | Akala Isle, Alola sea, Melemele Isle, Poni Isle and more |
| Clamperl | Akala Isle, Alola sea, Poni Isle, Ula'ula Isle |
| Cloyster | Ula'ula Isle |
| Comfey | Akala Forest, Akala Isle |
| Corsola | Akala Isle, Melemele Isle, Poni Isle, Ula'ula Isle |
| Crabominable | Ula'ula Cave, Ula'ula Isle |
| Crabrawler | Melemele Isle, Ula'ula Cave, Ula'ula Isle |
| Cubone | Akala Cave, Akala Isle, Poni Cave, Ula'ula Cave 2 |
| Cutiefly | Akala Forest, Melemele Isle |
| Dartrix | Akala Forest, Melemele Isle |
| Decidueye | Akala Forest, Melemele Isle |
| Dewpider | Akala Isle, Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Dhelmise | Poni Isle, Ula'ula Isle |
| Alolan Diglett | Akala Cave, Akala Isle, Poni Cave |
| Drampa | Ula'ula Cave, Ula'ula Isle |
| Alolan Dugtrio | Akala Cave, Akala Isle, Poni Cave |
| Exeggcute | Akala Forest, Poni Isle |
| Alolan Exeggutor | Poni Isle |
| Fomantis | Akala Forest, Akala Isle, Poni Isle |
| Gastly | Ula'ula Cave 2 |
| Alolan Geodude | Akala Cave, Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Gigalith | Poni Cave |
| Glalie | Ula'ula Cave |
| Golbat | Akala Cave, Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Golduck | Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Alolan Golem | Akala Cave, Poni Cave, Ula'ula Cave 2 |
| Golisopod | Akala Isle, Poni Isle |
| Alolan Graveler | Akala Cave, Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Alolan Grimer | Melemele Isle, Ula'ula Cave 2, Ula'ula Isle |
| Grubbin | Akala Forest, Melemele Isle, Poni Cave, Poni Isle and more |
| Gumshoos | Melemele Isle, Poni Isle, Ula'ula Isle |
| Hakamo-o | Poni Cave, Poni Isle, Ula'ula Cave |
| Haunter | Ula'ula Cave 2 |
| Hoothoot | Akala Forest |
| Incineroar | Akala Cave, Akala Isle |
| Jangmo-o | Poni Cave, Poni Isle, Ula'ula Cave |
| Komala | Ula'ula Isle |
| Kommo-o | Poni Cave, Poni Isle, Ula'ula Cave |
| Lanturn | Akala Isle, Alola sea, Melemele Isle, Poni Isle and more |
| Litten | Akala Cave, Akala Isle |
| Lurantis | Akala Forest, Akala Isle, Poni Isle |
| Luvdisc | Akala Isle, Melemele Isle, Ula'ula Isle |
| Lycanroc (Dusk) | Akala Cave |
| Lycanroc (Midday) | Melemele Isle, Poni Cave |
| Lycanroc (Midnight) | Melemele Isle, Poni Cave, Poni Isle |
| Magcargo | Akala Cave |
| Magikarp | Akala Isle, Alola sea, Melemele Isle, Poni Cave and more |
| Mareanie | Akala Isle, Alola sea, Melemele Isle, Poni Isle and more |
| Alolan Marowak | Akala Cave, Akala Isle, Poni Cave, Ula'ula Cave 2 |
| Alolan Meowth | Melemele Isle, Poni Isle, Ula'ula Isle |
| Mimikyu | Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Minior | Ula'ula Isle |
| Misdreavus | Akala Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Morelull | Akala Forest, Akala Isle, Poni Isle, Ula'ula Isle |
| Mudbray | Akala Isle, Poni Isle |
| Mudsdale | Akala Isle, Poni Isle |
| Alolan Muk | Melemele Isle, Ula'ula Cave 2, Ula'ula Isle |
| Alolan Ninetales | Ula'ula Cave, Ula'ula Isle |
| Noctowl | Akala Forest |
| Numel | Akala Cave |
| Oranguru | Akala Forest, Poni Isle, Ula'ula Isle |
| Oricorio (Baile) | Ula'ula Isle |
| Oricorio (Pa'u) | Akala Isle |
| Oricorio (Pom-Pom) | Akala Forest |
| Oricorio (Sensu) | Poni Isle |
| Palossand | Akala Isle, Poni Isle |
| Paras | Akala Forest |
| Parasect | Akala Forest |
| Passimian | Akala Forest, Poni Isle, Ula'ula Isle |
| Pelipper | Akala Isle, Melemele Isle, Poni Isle, Ula'ula Isle |
| Alolan Persian | Poni Isle, Ula'ula Isle |
| Pichu | Melemele Isle |
| Pikachu | Melemele Isle, Poni Isle |
| Pikipek | Akala Forest, Melemele Isle, Ula'ula Isle |
| Popplio | Melemele Isle |
| Primarina | Melemele Isle |
| Psyduck | Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Pyukumuku | Akala Isle, Melemele Isle, Poni Isle, Ula'ula Isle |
| Alolan Raichu | Poni Isle |
| Alolan Raticate | Akala Forest, Akala Isle, Melemele Isle, Poni Isle and more |
| Alolan Rattata | Akala Forest, Akala Isle, Melemele Isle, Poni Isle and more |
| Ribombee | Akala Forest, Melemele Isle |
| Rockruff | Melemele Isle, Poni Cave, Poni Isle |
| Rockruff (Own Tempo) | Akala Cave |
| Roggenrola | Poni Cave |
| Rowlet | Akala Forest, Melemele Isle |
| Sableye | Poni Cave |
| Salandit | Akala Cave, Akala Isle, Poni Isle |
| Salazzle | Akala Cave, Akala Isle, Poni Isle |
| Alolan Sandshrew | Ula'ula Cave, Ula'ula Isle |
| Alolan Sandslash | Ula'ula Cave, Ula'ula Isle |
| Sandygast | Akala Isle, Poni Isle |
| Sharpedo | Poni Isle |
| Shellder | Ula'ula Isle |
| Shiinotic | Akala Forest, Akala Isle, Poni Isle, Ula'ula Isle |
| Slugma | Akala Cave |
| Snorunt | Ula'ula Cave |
| Spinarak | Akala Forest, Melemele Isle |
| Staryu | Akala Isle, Alola sea, Melemele Isle, Poni Isle |
| Steenee | Akala Forest, Akala Isle, Melemele Isle, Poni Isle |
| Stufful | Akala Isle, Ula'ula Isle |
| Swoobat | Akala Cave |
| Tentacool | Alola sea, Melemele Isle |
| Tentacruel | Alola sea, Melemele Isle |
| Togedemaru | Ula'ula Isle |
| Torracat | Akala Cave, Akala Isle |
| Toucannon | Akala Forest, Melemele Isle, Ula'ula Isle |
| Toxapex | Akala Isle, Alola sea, Melemele Isle, Poni Isle and more |
| Trumbeak | Akala Forest, Melemele Isle, Ula'ula Isle |
| Tsareena | Poni Isle |
| Turtonator | Akala Cave, Ula'ula Isle |
| Vanillish | Ula'ula Cave |
| Vanillite | Ula'ula Cave |
| Vanilluxe | Ula'ula Cave |
| Venomoth | Akala Forest |
| Venonat | Akala Forest |
| Vikavolt | Poni Cave, Poni Isle, Ula'ula Cave 2 |
| Alolan Vulpix | Ula'ula Cave, Ula'ula Isle |
| Wailmer | Alola sea, Poni Isle, Ula'ula Isle |
| Whiscash | Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
| Wimpod | Akala Isle, Poni Isle |
| Wingull | Akala Isle, Melemele Isle, Poni Isle, Ula'ula Isle |
| Wishiwashi | Akala Isle, Alola sea, Melemele Isle, Poni Cave and more |
| Woobat | Akala Cave |
| Yungoos | Melemele Isle, Poni Isle, Ula'ula Isle |
| Zubat | Akala Cave, Poni Cave, Ula'ula Cave, Ula'ula Cave 2 |
