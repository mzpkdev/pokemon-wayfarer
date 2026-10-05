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

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Sneasel | Hisuian Sneasel |
| 2 | 20% | Hisuian Growlithe | Hisuian Voltorb |
| 3 | 10% | Stantler | Hisuian Zorua–Hisuian Zoroark |
| 4 | 10% | Swinub–Piloswine | Duskull–Dusclops |
| 5 | 10% | Hisuian Voltorb | Hisuian Zorua–Hisuian Zoroark |
| 6 | 10% | Snover–Abomasnow | Swinub–Piloswine |
| 7 | 5% | Stantler | Drifloon–Drifblim |
| 8 | 5% | Scyther | Stantler |
| 9 | 4% | Teddiursa–Ursaring | Snover–Abomasnow |
| 10 | 4% | Abra–Kadabra | Teddiursa–Ursaring |
| 11 | 1% | Hisuian Sneasel | Abra–Kadabra |
| 12 | 1% | Hisuian Growlithe | Teddiursa–Ursaring |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Basculin (White-Striped) | Basculin (White-Striped) |
| 2 | 30% | Hisuian Qwilfish | Hisuian Qwilfish |
| 3 | 5% | Psyduck–Golduck | Finneon–Lumineon |
| 4 | 4% | Buizel–Floatzel | Barboach–Whiscash |
| 5 | 1% | Barboach–Whiscash | Shellos West–Gastrodon |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Basculin (White-Striped) | Basculin (White-Striped) |
| 2 | 22% | 18% | 10% | Hisuian Qwilfish | Hisuian Qwilfish |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Barboach–Whiscash | Barboach–Whiscash |
| 5 | 8% | 9% | 10% | Basculin (White-Striped) | Finneon–Lumineon |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Remoraid–Octillery |
| 7 | 3% | 6% | 10% | Remoraid–Octillery | Basculin (White-Striped) |
| 8 | 3% | 5% | 9% | Buizel–Floatzel | Shellos West–Gastrodon |
| 9 | 2% | 4% | 9% | Hisuian Qwilfish | Hisuian Qwilfish |
| 10 | 2% | 4% | 9% | Barboach–Whiscash | Finneon–Lumineon |

#### Snowswept Cavern

Wilds, Sinjoh.

**`MAP_SNOWSWEPT_CAVERN_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Sneasel | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Zorua–Hisuian Zoroark | Hisuian Sneasel |
| 3 | 10% | Swinub–Piloswine | Zubat–Golbat |
| 4 | 10% | Snover–Abomasnow | Swinub–Piloswine |
| 5 | 10% | Zubat–Golbat | Snover–Abomasnow |
| 6 | 10% | Geodude–Graveler | Geodude–Graveler |
| 7 | 5% | Snorunt–Glalie | Misdreavus |
| 8 | 5% | Spheal | Snorunt–Glalie |
| 9 | 4% | Onix | Onix |
| 10 | 4% | Hisuian Sneasel | Spheal |
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
| 1 | 20% | Hisuian Voltorb | Hisuian Voltorb |
| 2 | 20% | Hisuian Growlithe | Hisuian Growlithe |
| 3 | 10% | Bronzor–Bronzong | Hisuian Zorua–Hisuian Zoroark |
| 4 | 10% | Nosepass | Gastly–Haunter |
| 5 | 10% | Bronzor–Bronzong | Duskull–Dusclops |
| 6 | 10% | Stantler | Bronzor–Bronzong |
| 7 | 5% | Hisuian Voltorb | Misdreavus |
| 8 | 5% | Geodude–Graveler | Drifloon–Drifblim |
| 9 | 4% | Nosepass | Clefairy |
| 10 | 4% | Hisuian Growlithe | Hisuian Zorua–Hisuian Zoroark |
| 11 | 1% | Chingling | Chingling |
| 12 | 1% | Stantler | Clefairy |

#### Route 50

Outlands, Sinjoh.

**`MAP_ROUTE50_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Sneasel | Hisuian Sneasel |
| 2 | 20% | Hisuian Growlithe | Hisuian Growlithe |
| 3 | 10% | Snover–Abomasnow | Hisuian Zorua–Hisuian Zoroark |
| 4 | 10% | Stantler | Duskull–Dusclops |
| 5 | 10% | Teddiursa–Ursaring | Hisuian Zorua–Hisuian Zoroark |
| 6 | 10% | Swinub–Piloswine | Snover–Abomasnow |
| 7 | 5% | Scyther | Drifloon–Drifblim |
| 8 | 5% | Machop–Machoke | Swinub–Piloswine |
| 9 | 4% | Stantler–Wyrdeer | Snorunt–Glalie |
| 10 | 4% | Scyther–Kleavor | Stantler–Wyrdeer |
| 11 | 1% | Hisuian Sneasel–Sneasler | Hisuian Sneasel–Sneasler |
| 12 | 1% | Hisuian Growlithe–Hisuian Arcanine | Riolu |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Hisuian Voltorb | Hisuian Sneasel |
| 2 | 30% | Hisuian Sneasel | Hisuian Voltorb |
| 3 | 5% | Teddiursa–Ursaring | Murkrow |
| 4 | 4% | Scyther | Teddiursa–Ursaring |
| 5 | 1% | Drifloon–Drifblim | Drifloon–Drifblim |

#### New Sinjoh Hot Springs

Dungeon, Sinjoh.

**`MAP_NEWSINJOH_HOTSPRINGS_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Growlithe | Hisuian Growlithe |
| 2 | 20% | Hisuian Growlithe | Hisuian Growlithe |
| 3 | 10% | Croagunk–Toxicroak | Croagunk–Toxicroak |
| 4 | 10% | Ponyta–Rapidash | Hisuian Zorua–Hisuian Zoroark |
| 5 | 10% | Rhyhorn–Rhydon | Rhyhorn–Rhydon |
| 6 | 10% | Swinub–Piloswine | Swinub–Piloswine |
| 7 | 5% | Geodude–Graveler | Geodude–Graveler |
| 8 | 5% | Magmar | Magmar |
| 9 | 4% | Psyduck–Golduck | Psyduck–Golduck |
| 10 | 4% | Teddiursa–Ursaluna | Teddiursa–Ursaluna |
| 11 | 1% | Hisuian Growlithe–Hisuian Arcanine | Ponyta–Rapidash |
| 12 | 1% | Magby | Magby |

#### Sinjoh Ruins chambers

Dungeon, Sinjoh.

**`MAP_SINJOH_RUINS_TEMPLE_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Voltorb | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Zorua–Hisuian Zoroark | Hisuian Voltorb |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Bronzor–Bronzong | Gastly–Haunter |
| 5 | 10% | Nosepass | Duskull–Dusclops |
| 6 | 10% | Unown | Unown |
| 7 | 5% | Hisuian Voltorb–Hisuian Electrode | Bronzor–Bronzong |
| 8 | 5% | Chimecho | Misdreavus |
| 9 | 4% | Unown | Unown |
| 10 | 4% | Bronzor–Bronzong | Hisuian Voltorb–Hisuian Electrode |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Chingling | Chingling |

**`MAP_SINJOH_RUINS_REGICE_ROOM_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Sneasel | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Zorua–Hisuian Zoroark | Hisuian Sneasel |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Snover–Abomasnow | Snover–Abomasnow |
| 5 | 10% | Unown | Unown |
| 6 | 10% | Bronzor–Bronzong | Bronzor–Bronzong |
| 7 | 5% | Snorunt–Glalie | Misdreavus |
| 8 | 5% | Unown | Snorunt–Glalie |
| 9 | 4% | Spheal | Unown |
| 10 | 4% | Unown | Spheal |
| 11 | 1% | Hisuian Sneasel–Sneasler | Hisuian Sneasel–Sneasler |
| 12 | 1% | Unown | Unown |

**`MAP_SINJOH_RUINS_REGIROCK_ROOM_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hisuian Voltorb | Hisuian Zorua–Hisuian Zoroark |
| 2 | 20% | Hisuian Zorua–Hisuian Zoroark | Hisuian Voltorb |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Nosepass | Gastly–Haunter |
| 5 | 10% | Unown | Unown |
| 6 | 10% | Geodude–Graveler | Nosepass |
| 7 | 5% | Unown | Unown |
| 8 | 5% | Bronzor–Bronzong | Duskull–Dusclops |
| 9 | 4% | Unown | Unown |
| 10 | 4% | Onix | Geodude–Graveler |
| 11 | 1% | Hisuian Voltorb–Hisuian Electrode | Hisuian Voltorb–Hisuian Electrode |
| 12 | 1% | Unown | Bronzor–Bronzong |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Abomasnow | Route 49, Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Abra | Route 49 |
| Hisuian Arcanine | New Sinjoh Hot Springs, Route 50 |
| Barboach | Route 49 |
| Basculin (White-Striped) | Route 49 |
| Bronzong | Sinjoh Ruins, Sinjoh Ruins chambers |
| Bronzor | Sinjoh Ruins, Sinjoh Ruins chambers |
| Buizel | Route 49 |
| Chimecho | Sinjoh Ruins chambers |
| Chingling | Sinjoh Ruins, Sinjoh Ruins chambers |
| Clefairy | Sinjoh Ruins |
| Croagunk | New Sinjoh Hot Springs |
| Drifblim | Route 49, Route 50, Sinjoh Ruins |
| Drifloon | Route 49, Route 50, Sinjoh Ruins |
| Dusclops | Route 49, Route 50, Sinjoh Ruins, Sinjoh Ruins chambers |
| Duskull | Route 49, Route 50, Sinjoh Ruins, Sinjoh Ruins chambers |
| Hisuian Electrode | Sinjoh Ruins chambers |
| Finneon | Route 49 |
| Floatzel | Route 49 |
| Gastly | Sinjoh Ruins, Sinjoh Ruins chambers |
| Gastrodon | Route 49 |
| Geodude | New Sinjoh Hot Springs, Sinjoh Ruins, Sinjoh Ruins chambers, Snowswept Cavern |
| Glalie | Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Golbat | Snowswept Cavern |
| Golduck | New Sinjoh Hot Springs, Route 49 |
| Graveler | New Sinjoh Hot Springs, Sinjoh Ruins, Sinjoh Ruins chambers, Snowswept Cavern |
| Hisuian Growlithe | New Sinjoh Hot Springs, Route 49, Route 50, Sinjoh Ruins |
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
| Murkrow | Route 50 |
| Nosepass | Sinjoh Ruins, Sinjoh Ruins chambers, Snowswept Cavern |
| Octillery | Route 49 |
| Onix | Sinjoh Ruins chambers, Snowswept Cavern |
| Piloswine | New Sinjoh Hot Springs, Route 49, Route 50, Snowswept Cavern |
| Ponyta | New Sinjoh Hot Springs |
| Psyduck | New Sinjoh Hot Springs, Route 49 |
| Hisuian Qwilfish | Route 49 |
| Rapidash | New Sinjoh Hot Springs |
| Remoraid | Route 49 |
| Rhydon | New Sinjoh Hot Springs |
| Rhyhorn | New Sinjoh Hot Springs |
| Riolu | Route 50, Snowswept Cavern |
| Scyther | Route 49, Route 50 |
| Shellos West | Route 49 |
| Hisuian Sneasel | Route 49, Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Sneasler | Route 50, Sinjoh Ruins chambers |
| Snorunt | Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Snover | Route 49, Route 50, Sinjoh Ruins chambers, Snowswept Cavern |
| Spheal | Sinjoh Ruins chambers, Snowswept Cavern |
| Stantler | Route 49, Route 50, Sinjoh Ruins |
| Swinub | New Sinjoh Hot Springs, Route 49, Route 50, Snowswept Cavern |
| Teddiursa | New Sinjoh Hot Springs, Route 49, Route 50 |
| Toxicroak | New Sinjoh Hot Springs |
| Unown | Sinjoh Ruins chambers |
| Ursaluna | New Sinjoh Hot Springs |
| Ursaring | New Sinjoh Hot Springs, Route 49, Route 50 |
| Hisuian Voltorb | Route 49, Route 50, Sinjoh Ruins, Sinjoh Ruins chambers |
| Whiscash | Route 49 |
| Wyrdeer | Route 50 |
| Hisuian Zoroark | New Sinjoh Hot Springs, Route 49, Route 50, Sinjoh Ruins and more |
| Hisuian Zorua | New Sinjoh Hot Springs, Route 49, Route 50, Sinjoh Ruins and more |
| Zubat | Snowswept Cavern |
