# Sinjoh encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Sinjoh encounter rules](sinjoh-encounters.md). They are the source of truth for
Sinjoh's wild-encounter data: every slot below maps one to one to a slot in the
game's encounter tables. Playtesting may change the picks.

## Scope

This spec lists Sinjoh's wild-encounter tables: 8 maps, each with a day and a
night table for every method it has. Enamorus, the Regi rooms' legendaries and
Arceus's events belong to a later spec.

The Sinjoh Ruins lose their current Rock Smash table, since the map has no
breakable rocks, and Route 50's becomes a Headbutt-tree table, since its
trees are what trigger it.

## Behavior

### Reading the tables

- Places are ordered by reach: Road, Wilds, Outlands, then dungeons. Each place
  names its reach, and each map is named by its map
  constant. A map with surfing or fishing also names its water type.
- Each table lists every slot in order, with its weight, and the species for
  day and night.
  - Land has 12 slots weighted 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1 and 1%.
  - Surfing, and trees and rocks, have 5 slots weighted 60, 30, 5, 4 and 1%.
  - Fishing has the [Standard Rod](standard-rod-fishing.md)'s 10 entries, with
    each entry's weight for the Old, Good and Super Rod.
- **A slot's species is its stage cap,** the last name in the cell.
  "Teddiursa–Ursaring" means the slot holds Ursaring, and the game steps it
  down to Teddiursa below Ursaring's evolution level. A single name is a single-stage
  species, a baby, or a line capped at its first stage.
- Names map to species constants in capitals, with spaces and hyphens as
  underscores and other punctuation dropped: Abomasnow is
  `SPECIES_ABOMASNOW`.
  - "Hisuian" marks a Hisuian form: Hisuian Sneasel is
    `SPECIES_SNEASEL_HISUI`.
  - A form in brackets follows the name: Basculin (White-Striped) is
    `SPECIES_BASCULIN_WHITE_STRIPED`.
- Slots hold no levels. Levels come from the map's reach, as the PRD defines.

### Tables

#### Route 49

Wilds, Sinjoh.

**`MAP_ROUTE49_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Growlithe | Hisuian Sneasel |
| 2 | 20% | Hisuian Sneasel | Hisuian Zorua–Hisuian Zoroark |
| 3 | 10% | Stantler | Hisuian Voltorb |
| 4 | 10% | Hisuian Voltorb | Teddiursa–Ursaring |
| 5 | 10% | Teddiursa–Ursaring | Hisuian Zorua–Hisuian Zoroark |
| 6 | 10% | Ponyta–Rapidash | Stantler |
| 7 | 5% | Hisuian Growlithe | Drifloon–Drifblim |
| 8 | 5% | Scyther | Hisuian Growlithe |
| 9 | 4% | Abra–Kadabra | Abra–Kadabra |
| 10 | 4% | Yanma | Teddiursa–Ursaring |
| 11 | 1% | Hisuian Sneasel | Hisuian Sneasel |
| 12 | 1% | Stantler | Drifloon–Drifblim |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Hisuian Qwilfish | Hisuian Qwilfish |
| 2 | 30% | Basculin (White-Striped) | Basculin (White-Striped) |
| 3 | 5% | Buizel–Floatzel | Finneon–Lumineon |
| 4 | 4% | Shellos West–Gastrodon | Shellos West–Gastrodon |
| 5 | 1% | Tentacool–Tentacruel | Buizel–Floatzel |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Hisuian Qwilfish | Basculin (White-Striped) |
| 2 | 22% | 18% | 10% | Basculin (White-Striped) | Hisuian Qwilfish |
| 3 | 10% | 12% | 11% | Magikarp–Gyarados | Magikarp–Gyarados |
| 4 | 8% | 10% | 10% | Hisuian Qwilfish | Finneon–Lumineon |
| 5 | 8% | 9% | 10% | Remoraid–Octillery | Hisuian Qwilfish |
| 6 | 4% | 7% | 10% | Buizel–Floatzel | Remoraid–Octillery |
| 7 | 3% | 6% | 10% | Shellos West–Gastrodon | Finneon–Lumineon |
| 8 | 3% | 5% | 9% | Basculin (White-Striped) | Shellos West–Gastrodon |
| 9 | 2% | 4% | 9% | Tentacool–Tentacruel | Basculin (White-Striped) |
| 10 | 2% | 4% | 9% | Magikarp–Gyarados | Buizel–Floatzel |

#### Snowswept Cavern

Wilds, Sinjoh.

**`MAP_SNOWSWEPT_CAVERN_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Sneasel | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Zorua–Hisuian Zoroark | Hisuian Sneasel |
| 3 | 10% | Swinub–Piloswine | Zubat–Golbat |
| 4 | 10% | Snorunt–Glalie | Swinub–Piloswine |
| 5 | 10% | Snover–Abomasnow | Snorunt–Glalie |
| 6 | 10% | Zubat–Golbat | Snover–Abomasnow |
| 7 | 5% | Geodude–Graveler | Hisuian Zorua–Hisuian Zoroark |
| 8 | 5% | Spheal–Walrein | Geodude–Graveler |
| 9 | 4% | Onix | Onix |
| 10 | 4% | Hisuian Sneasel | Misdreavus |
| 11 | 1% | Riolu | Riolu |
| 12 | 1% | Hisuian Zorua–Hisuian Zoroark | Hisuian Zorua–Hisuian Zoroark |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Hisuian Zorua–Hisuian Zoroark | Hisuian Sneasel |
| 2 | 30% | Hisuian Sneasel | Hisuian Zorua–Hisuian Zoroark |
| 3 | 5% | Geodude–Graveler | Geodude–Graveler |
| 4 | 4% | Nosepass | Nosepass |
| 5 | 1% | Onix | Onix |

#### Sinjoh Ruins

Wilds, Sinjoh.

**`MAP_SINJOH_RUINS_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Voltorb | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Growlithe | Hisuian Voltorb |
| 3 | 10% | Bronzor–Bronzong | Misdreavus |
| 4 | 10% | Chimecho | Gastly–Haunter |
| 5 | 10% | Nosepass | Duskull–Dusclops |
| 6 | 10% | Clefairy | Bronzor–Bronzong |
| 7 | 5% | Hisuian Voltorb | Drifloon–Drifblim |
| 8 | 5% | Stantler | Hisuian Zorua–Hisuian Zoroark |
| 9 | 4% | Bronzor–Bronzong | Chimecho |
| 10 | 4% | Hisuian Growlithe | Misdreavus |
| 11 | 1% | Chingling | Chingling |
| 12 | 1% | Clefairy | Clefairy |

#### Route 50

Outlands, Sinjoh.

**`MAP_ROUTE50_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Sneasel | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Growlithe | Hisuian Sneasel |
| 3 | 10% | Snover–Abomasnow | Teddiursa–Ursaring |
| 4 | 10% | Stantler | Hisuian Zorua–Hisuian Zoroark |
| 5 | 10% | Teddiursa–Ursaring | Snover–Abomasnow |
| 6 | 10% | Swinub–Piloswine | Swinub–Piloswine |
| 7 | 5% | Scyther | Drifloon–Drifblim |
| 8 | 5% | Machop–Machoke | Stantler |
| 9 | 4% | Stantler–Wyrdeer | Snorunt–Glalie |
| 10 | 4% | Scyther–Kleavor | Stantler–Wyrdeer |
| 11 | 1% | Hisuian Sneasel–Sneasler | Hisuian Sneasel–Sneasler |
| 12 | 1% | Hisuian Growlithe–Hisuian Arcanine | Riolu |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Hisuian Voltorb | Hisuian Sneasel |
| 2 | 30% | Hisuian Sneasel | Hisuian Voltorb |
| 3 | 5% | Teddiursa–Ursaring | Teddiursa–Ursaring |
| 4 | 4% | Scyther | Drifloon–Drifblim |
| 5 | 1% | Drifloon–Drifblim | Scyther |

#### New Sinjoh Hot Springs

Dungeon, Sinjoh.

**`MAP_NEWSINJOH_HOTSPRINGS_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Growlithe | Hisuian Growlithe |
| 2 | 20% | Hisuian Growlithe–Hisuian Arcanine | Hisuian Growlithe–Hisuian Arcanine |
| 3 | 10% | Magmar | Croagunk–Toxicroak |
| 4 | 10% | Ponyta–Rapidash | Magmar |
| 5 | 10% | Rhyhorn–Rhydon | Rhyhorn–Rhydon |
| 6 | 10% | Croagunk–Toxicroak | Hisuian Zorua–Hisuian Zoroark |
| 7 | 5% | Geodude–Graveler | Geodude–Graveler |
| 8 | 5% | Magby | Magby |
| 9 | 4% | Psyduck–Golduck | Psyduck–Golduck |
| 10 | 4% | Teddiursa–Ursaluna | Teddiursa–Ursaluna |
| 11 | 1% | Croagunk–Toxicroak | Ponyta–Rapidash |
| 12 | 1% | Teddiursa–Ursaluna | Teddiursa–Ursaluna |

#### Sinjoh Ruins chambers

Dungeon, Sinjoh.

**`MAP_SINJOH_RUINS_TEMPLE_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Zorua–Hisuian Zoroark | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Voltorb | Hisuian Voltorb |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Bronzor–Bronzong | Gastly–Haunter |
| 5 | 10% | Baltoy–Claydol | Misdreavus |
| 6 | 10% | Unown | Unown |
| 7 | 5% | Hisuian Voltorb–Hisuian Electrode | Bronzor–Bronzong |
| 8 | 5% | Chimecho | Baltoy–Claydol |
| 9 | 4% | Unown | Unown |
| 10 | 4% | Bronzor–Bronzong | Hisuian Voltorb–Hisuian Electrode |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Chingling | Chingling |

**`MAP_SINJOH_RUINS_REGICE_ROOM_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Zorua–Hisuian Zoroark | Hisuian Sneasel |
| 2 | 20% | Hisuian Sneasel | Hisuian Zorua–Hisuian Zoroark |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Snorunt–Glalie | Misdreavus |
| 5 | 10% | Unown | Unown |
| 6 | 10% | Bronzor–Bronzong | Snorunt–Glalie |
| 7 | 5% | Unown | Unown |
| 8 | 5% | Hisuian Sneasel–Sneasler | Bronzor–Bronzong |
| 9 | 4% | Unown | Unown |
| 10 | 4% | Snover–Abomasnow | Snover–Abomasnow |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Hisuian Zorua–Hisuian Zoroark | Hisuian Sneasel–Sneasler |

**`MAP_SINJOH_RUINS_REGIROCK_ROOM_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Voltorb | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Zorua–Hisuian Zoroark | Hisuian Voltorb |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Baltoy–Claydol | Gastly–Haunter |
| 5 | 10% | Unown | Unown |
| 6 | 10% | Nosepass | Baltoy–Claydol |
| 7 | 5% | Unown | Unown |
| 8 | 5% | Bronzor–Bronzong | Nosepass |
| 9 | 4% | Unown | Unown |
| 10 | 4% | Geodude–Graveler | Bronzor–Bronzong |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Hisuian Voltorb–Hisuian Electrode | Hisuian Voltorb–Hisuian Electrode |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Abomasnow | Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Abra | Route 49 |
| Hisuian Arcanine | New Sinjoh Hot Springs, Route 50 |
| Baltoy | Sinjoh Ruins chambers |
| Basculin (White-Striped) | Route 49 |
| Bronzong | Sinjoh Ruins, Sinjoh Ruins chambers |
| Bronzor | Sinjoh Ruins, Sinjoh Ruins chambers |
| Buizel | Route 49 |
| Chimecho | Sinjoh Ruins, Sinjoh Ruins chambers |
| Chingling | Sinjoh Ruins, Sinjoh Ruins chambers |
| Claydol | Sinjoh Ruins chambers |
| Clefairy | Sinjoh Ruins |
| Croagunk | New Sinjoh Hot Springs |
| Drifblim | Route 49, Route 50, Sinjoh Ruins |
| Drifloon | Route 49, Route 50, Sinjoh Ruins |
| Dusclops | Sinjoh Ruins |
| Duskull | Sinjoh Ruins |
| Hisuian Electrode | Sinjoh Ruins chambers |
| Finneon | Route 49 |
| Floatzel | Route 49 |
| Gastly | Sinjoh Ruins, Sinjoh Ruins chambers |
| Gastrodon | Route 49 |
| Geodude | New Sinjoh Hot Springs, Sinjoh Ruins chambers, Snowswept Cavern |
| Glalie | Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Golbat | Snowswept Cavern |
| Golduck | New Sinjoh Hot Springs |
| Graveler | New Sinjoh Hot Springs, Sinjoh Ruins chambers, Snowswept Cavern |
| Hisuian Growlithe | New Sinjoh Hot Springs, Route 49, Route 50, Sinjoh Ruins |
| Gyarados | Route 49 |
| Haunter | Sinjoh Ruins, Sinjoh Ruins chambers |
| Kadabra | Route 49 |
| Kleavor | Route 50 |
| Lumineon | Route 49 |
| Machoke | Route 50 |
| Machop | Route 50 |
| Magby | New Sinjoh Hot Springs |
| Magikarp | Route 49 |
| Magmar | New Sinjoh Hot Springs |
| Misdreavus | Sinjoh Ruins, Sinjoh Ruins chambers, Snowswept Cavern |
| Nosepass | Sinjoh Ruins, Sinjoh Ruins chambers, Snowswept Cavern |
| Octillery | Route 49 |
| Onix | Snowswept Cavern |
| Piloswine | Route 50, Snowswept Cavern |
| Ponyta | New Sinjoh Hot Springs, Route 49 |
| Psyduck | New Sinjoh Hot Springs |
| Hisuian Qwilfish | Route 49 |
| Rapidash | New Sinjoh Hot Springs, Route 49 |
| Remoraid | Route 49 |
| Rhydon | New Sinjoh Hot Springs |
| Rhyhorn | New Sinjoh Hot Springs |
| Riolu | Route 50, Snowswept Cavern |
| Scyther | Route 49, Route 50 |
| Sealeo | Snowswept Cavern |
| Shellos West | Route 49 |
| Hisuian Sneasel | Route 49, Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Sneasler | Route 50, Sinjoh Ruins chambers |
| Snorunt | Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Snover | Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Spheal | Snowswept Cavern |
| Stantler | Route 49, Route 50, Sinjoh Ruins |
| Swinub | Route 50, Snowswept Cavern |
| Teddiursa | New Sinjoh Hot Springs, Route 49, Route 50 |
| Tentacool | Route 49 |
| Tentacruel | Route 49 |
| Toxicroak | New Sinjoh Hot Springs |
| Unown | Sinjoh Ruins chambers |
| Ursaluna | New Sinjoh Hot Springs |
| Ursaring | New Sinjoh Hot Springs, Route 49, Route 50 |
| Hisuian Voltorb | Route 49, Route 50, Sinjoh Ruins, Sinjoh Ruins chambers |
| Walrein | Snowswept Cavern |
| Wyrdeer | Route 50 |
| Yanma | Route 49 |
| Hisuian Zoroark | New Sinjoh Hot Springs, Route 49, Route 50, Sinjoh Ruins and more |
| Hisuian Zorua | New Sinjoh Hot Springs, Route 49, Route 50, Sinjoh Ruins and more |
| Zubat | Snowswept Cavern |
