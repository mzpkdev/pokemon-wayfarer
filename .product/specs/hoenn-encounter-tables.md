# Hoenn encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Hoenn encounter rules](hoenn-encounters.md). They are the source of truth for
Hoenn's wild-encounter data: every slot below maps one to one to a slot in the
game's encounter tables. Playtesting may change the picks.

## Scope

This spec lists Hoenn's wild-encounter tables: 110 maps, each with a day and a
night table for every method it has. The Safari Zone belongs to the
[Safari table spec](safari-encounter-tables.md). Legendaries and mythicals
belong to a later spec.

## Behavior

### Reading the tables

- Places are ordered by reach: Road, Wilds, Outlands, then dungeons. Each place
  names its reach and its Gen V band, and each map is named by its map
  constant. A map with surfing or fishing also names its water type.
- Each table lists every slot in order, with its weight, and the species for
  day and night.
  - Land has 12 slots weighted 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1 and 1%.
  - Surfing, and Rock Smash rocks, have 5 slots weighted 60, 30, 5, 4 and 1%.
  - Fishing has the [Standard Rod](standard-rod-fishing.md)'s 10 entries, with
    each entry's weight for the Old, Good and Super Rod.
- **A slot's species is its stage cap,** the last name in the cell.
  "Lotad–Lombre" means the slot holds Lombre, and the game steps it down to
  Lotad below Lombre's evolution level. A single name is a single-stage
  species, a baby, or a line capped at its first stage.
- Names map to species constants in capitals: Whismur is `SPECIES_WHISMUR`.
- Slots hold no levels. Levels come from the map's reach, as the PRD defines.

### Tables

#### Petalburg City

Road, West.

**`MAP_PETALBURG_CITY`**

Water type: ponds and rivers.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Lotad–Lombre | Corphish–Crawdaunt |
| 3 | 5% | Surskit–Masquerain | Lotad–Lombre |
| 4 | 4% | Ducklett–Swanna | Barboach–Whiscash |
| 5 | 1% | Azurill | Azurill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 2 | 22% | 18% | 10% | Lotad–Lombre | Barboach–Whiscash |
| 3 | 10% | 12% | 11% | Marill–Azumarill | Marill–Azumarill |
| 4 | 8% | 10% | 10% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 5 | 8% | 9% | 10% | Surskit–Masquerain | Barboach–Whiscash |
| 6 | 4% | 7% | 10% | Barboach–Whiscash | Tympole–Seismitoad |
| 7 | 3% | 6% | 10% | Lotad–Lombre | Lotad–Lombre |
| 8 | 3% | 5% | 9% | Tympole–Seismitoad | Surskit–Masquerain |
| 9 | 2% | 4% | 9% | Marill–Azumarill | Tympole–Seismitoad |
| 10 | 2% | 4% | 9% | Basculin | Basculin |

#### Dewford Town

Road, West.

**`MAP_DEWFORD_TOWN`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Wailmer–Wailord |
| 2 | 30% | Wailmer–Wailord | Carvanha–Sharpedo |
| 3 | 5% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 4 | 4% | Luvdisc | Wingull–Pelipper |
| 5 | 1% | Frillish | Tynamo–Eelektrik |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Carvanha–Sharpedo |
| 4 | 8% | 10% | 10% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 5 | 8% | 9% | 10% | Luvdisc | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Carvanha–Sharpedo | Wailmer–Wailord |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Luvdisc |
| 8 | 3% | 5% | 9% | Wailmer–Wailord | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Luvdisc | Tynamo–Eelektrik |
| 10 | 2% | 4% | 9% | Frillish | Frillish |

#### Slateport City

Road, West.

**`MAP_SLATEPORT_CITY`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Wailmer–Wailord |
| 2 | 30% | Wailmer–Wailord | Corphish–Crawdaunt |
| 3 | 5% | Corphish–Crawdaunt | Tynamo–Eelektrik |
| 4 | 4% | Luvdisc | Wingull–Pelipper |
| 5 | 1% | Oshawott–Samurott | Oshawott–Samurott |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 22% | 18% | 10% | Corphish–Crawdaunt | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Corphish–Crawdaunt |
| 4 | 8% | 10% | 10% | Luvdisc | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Corphish–Crawdaunt | Luvdisc |
| 7 | 3% | 6% | 10% | Luvdisc | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Ducklett–Swanna | Tynamo–Eelektrik |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Frillish |
| 10 | 2% | 4% | 9% | Frillish | Wailmer–Wailord |

#### Lilycove City

Road, East.

**`MAP_LILYCOVE_CITY`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Frillish |
| 2 | 30% | Frillish | Carvanha–Sharpedo |
| 3 | 5% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 4% | Ducklett–Swanna | Wailmer–Wailord |
| 5 | 1% | Luvdisc | Wingull–Pelipper |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Frillish | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Frillish |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Wailmer–Wailord |
| 4 | 8% | 10% | 10% | Frillish | Tynamo–Eelektrik |
| 5 | 8% | 9% | 10% | Wailmer–Wailord | Frillish |
| 6 | 4% | 7% | 10% | Luvdisc | Carvanha–Sharpedo |
| 7 | 3% | 6% | 10% | Ducklett–Swanna | Tynamo–Eelektrik |
| 8 | 3% | 5% | 9% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Tynamo–Eelektrik | Luvdisc |
| 10 | 2% | 4% | 9% | Wailmer–Wailord | Wailmer–Wailord |

#### Mossdeep City

Road, Far east.

**`MAP_MOSSDEEP_CITY`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Tynamo–Eelektrik |
| 2 | 30% | Frillish | Frillish |
| 3 | 5% | Ducklett–Swanna | Wailmer–Wailord |
| 4 | 4% | Wailmer–Wailord | Carvanha–Sharpedo |
| 5 | 1% | Tynamo–Eelektrik | Wingull–Pelipper |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Frillish | Frillish |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Tynamo–Eelektrik | Frillish |
| 5 | 8% | 9% | 10% | Carvanha–Sharpedo | Wailmer–Wailord |
| 6 | 4% | 7% | 10% | Luvdisc | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Frillish | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Corphish–Crawdaunt | Luvdisc |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Corphish–Crawdaunt |
| 10 | 2% | 4% | 9% | Ducklett–Swanna | Tynamo–Eelektrik |

#### Sootopolis City

Road, Far east.

**`MAP_SOOTOPOLIS_CITY`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 30% | Luvdisc | Frillish |
| 3 | 5% | Frillish | Tynamo–Eelektrik |
| 4 | 4% | Ducklett–Swanna | Luvdisc |
| 5 | 1% | Wailmer–Wailord | Frillish–Jellicent |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Luvdisc | Frillish |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Luvdisc |
| 3 | 10% | 12% | 11% | Frillish | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Luvdisc | Frillish |
| 5 | 8% | 9% | 10% | Tynamo–Eelektrik | Wailmer–Wailord |
| 6 | 4% | 7% | 10% | Wailmer–Wailord | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Frillish | Luvdisc |
| 9 | 2% | 4% | 9% | Carvanha–Sharpedo | Corphish–Crawdaunt |
| 10 | 2% | 4% | 9% | Ducklett–Swanna | Frillish–Jellicent |

#### Pacifidlog Town

Road, Far east.

**`MAP_PACIFIDLOG_TOWN`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 2 | 30% | Wingull–Pelipper | Frillish |
| 3 | 5% | Wailmer–Wailord | Wingull–Pelipper |
| 4 | 4% | Frillish | Carvanha–Sharpedo |
| 5 | 1% | Luvdisc | Wailmer–Wailord |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 2 | 22% | 18% | 10% | Tynamo–Eelektrik | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Frillish | Frillish |
| 4 | 8% | 10% | 10% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Luvdisc | Frillish |
| 6 | 4% | 7% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 7 | 3% | 6% | 10% | Tynamo–Eelektrik | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Ducklett–Swanna | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Frillish | Luvdisc |
| 10 | 2% | 4% | 9% | Luvdisc | Tynamo–Eelektrik |

#### Ever Grande City

Road, Far east.

**`MAP_EVER_GRANDE_CITY`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Luvdisc | Luvdisc |
| 2 | 30% | Ducklett–Swanna | Frillish |
| 3 | 5% | Frillish | Tynamo–Eelektrik |
| 4 | 4% | Wingull–Pelipper | Ducklett–Swanna |
| 5 | 1% | Wailmer–Wailord | Wailmer–Wailord |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Luvdisc | Luvdisc |
| 2 | 22% | 18% | 10% | Frillish | Frillish |
| 3 | 10% | 12% | 11% | Luvdisc | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Frillish |
| 5 | 8% | 9% | 10% | Tynamo–Eelektrik | Luvdisc |
| 6 | 4% | 7% | 10% | Ducklett–Swanna | Carvanha–Sharpedo |
| 7 | 3% | 6% | 10% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 8 | 3% | 5% | 9% | Luvdisc | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Corphish–Crawdaunt |
| 10 | 2% | 4% | 9% | Frillish | Frillish–Jellicent |

#### Route 101

Road, West.

**`MAP_ROUTE101`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Zigzagoon–Linoone | Poochyena–Mightyena |
| 2 | 20% | Wurmple | Zigzagoon–Linoone |
| 3 | 10% | Zigzagoon–Linoone | Poochyena–Mightyena |
| 4 | 10% | Wurmple | Zigzagoon–Linoone |
| 5 | 10% | Taillow–Swellow | Seedot–Nuzleaf |
| 6 | 10% | Skitty | Wurmple |
| 7 | 5% | Taillow–Swellow | Purrloin–Liepard |
| 8 | 5% | Skitty | Seedot–Nuzleaf |
| 9 | 4% | Patrat–Watchog | Skitty |
| 10 | 4% | Lillipup–Stoutland | Purrloin–Liepard |
| 11 | 1% | Treecko–Sceptile | Treecko–Sceptile |
| 12 | 1% | Treecko–Sceptile | Munna |

#### Route 102

Road, West.

**`MAP_ROUTE102`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Zigzagoon–Linoone | Poochyena–Mightyena |
| 2 | 20% | Wurmple | Lotad–Lombre |
| 3 | 10% | Lotad–Lombre | Poochyena–Mightyena |
| 4 | 10% | Zigzagoon–Linoone | Seedot–Nuzleaf |
| 5 | 10% | Wurmple | Zigzagoon–Linoone |
| 6 | 10% | Seedot–Nuzleaf | Volbeat |
| 7 | 5% | Taillow–Swellow | Illumise |
| 8 | 5% | Surskit–Masquerain | Seedot–Nuzleaf |
| 9 | 4% | Sewaddle–Swadloon | Purrloin–Liepard |
| 10 | 4% | Pansage | Wurmple |
| 11 | 1% | Ralts–Gardevoir | Ralts–Gardevoir |
| 12 | 1% | Ralts–Gardevoir | Munna |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Lotad–Lombre |
| 2 | 30% | Lotad–Lombre | Marill–Azumarill |
| 3 | 5% | Surskit–Masquerain | Corphish–Crawdaunt |
| 4 | 4% | Azurill | Tympole–Seismitoad |
| 5 | 1% | Mudkip–Swampert | Mudkip–Swampert |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Lotad–Lombre | Corphish–Crawdaunt |
| 2 | 22% | 18% | 10% | Corphish–Crawdaunt | Lotad–Lombre |
| 3 | 10% | 12% | 11% | Lotad–Lombre | Barboach–Whiscash |
| 4 | 8% | 10% | 10% | Marill–Azumarill | Corphish–Crawdaunt |
| 5 | 8% | 9% | 10% | Barboach–Whiscash | Tympole–Seismitoad |
| 6 | 4% | 7% | 10% | Surskit–Masquerain | Barboach–Whiscash |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Marill–Azumarill |
| 8 | 3% | 5% | 9% | Basculin | Tympole–Seismitoad |
| 9 | 2% | 4% | 9% | Tympole–Seismitoad | Basculin |
| 10 | 2% | 4% | 9% | Surskit–Masquerain | Lotad–Lombre |

#### Route 103

Road, West.

**`MAP_ROUTE103`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wingull–Pelipper | Poochyena–Mightyena |
| 2 | 20% | Zigzagoon–Linoone | Zigzagoon–Linoone |
| 3 | 10% | Zigzagoon–Linoone | Poochyena–Mightyena |
| 4 | 10% | Wingull–Pelipper | Munna |
| 5 | 10% | Taillow–Swellow | Seedot–Nuzleaf |
| 6 | 10% | Skitty | Zigzagoon–Linoone |
| 7 | 5% | Wurmple | Purrloin–Liepard |
| 8 | 5% | Ducklett–Swanna | Seedot–Nuzleaf |
| 9 | 4% | Taillow–Swellow | Woobat |
| 10 | 4% | Patrat–Watchog | Purrloin–Liepard |
| 11 | 1% | Oshawott–Samurott | Oshawott–Samurott |
| 12 | 1% | Oshawott–Samurott | Munna |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Carvanha–Sharpedo |
| 2 | 30% | Wailmer–Wailord | Wailmer–Wailord |
| 3 | 5% | Corphish–Crawdaunt | Tynamo–Eelektrik |
| 4 | 4% | Ducklett–Swanna | Wingull–Pelipper |
| 5 | 1% | Oshawott–Samurott | Oshawott–Samurott |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Carvanha–Sharpedo |
| 4 | 8% | 10% | 10% | Corphish–Crawdaunt | Frillish |
| 5 | 8% | 9% | 10% | Luvdisc | Corphish–Crawdaunt |
| 6 | 4% | 7% | 10% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Wailmer–Wailord | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Frillish | Carvanha–Sharpedo |
| 9 | 2% | 4% | 9% | Corphish–Crawdaunt | Frillish |
| 10 | 2% | 4% | 9% | Luvdisc | Luvdisc |

#### Route 104

Road, West.

**`MAP_ROUTE104`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Taillow–Swellow | Poochyena–Mightyena |
| 2 | 20% | Wurmple | Marill–Azumarill |
| 3 | 10% | Wingull–Pelipper | Poochyena–Mightyena |
| 4 | 10% | Marill–Azumarill | Seedot–Nuzleaf |
| 5 | 10% | Zigzagoon–Linoone | Munna |
| 6 | 10% | Taillow–Swellow | Seedot–Nuzleaf |
| 7 | 5% | Skitty | Volbeat |
| 8 | 5% | Cottonee | Purrloin–Liepard |
| 9 | 4% | Pidove–Unfezant | Illumise |
| 10 | 4% | Wingull–Pelipper | Woobat |
| 11 | 1% | Mudkip–Swampert | Mudkip–Swampert |
| 12 | 1% | Azurill | Azurill |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Carvanha–Sharpedo |
| 2 | 30% | Wailmer–Wailord | Wailmer–Wailord |
| 3 | 5% | Ducklett–Swanna | Frillish |
| 4 | 4% | Luvdisc | Wingull–Pelipper |
| 5 | 1% | Corphish–Crawdaunt | Tynamo–Eelektrik |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Luvdisc | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Tynamo–Eelektrik |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 6 | 4% | 7% | 10% | Luvdisc | Wailmer–Wailord |
| 7 | 3% | 6% | 10% | Frillish | Frillish |
| 8 | 3% | 5% | 9% | Wailmer–Wailord | Tynamo–Eelektrik |
| 9 | 2% | 4% | 9% | Corphish–Crawdaunt | Luvdisc |
| 10 | 2% | 4% | 9% | Ducklett–Swanna | Corphish–Crawdaunt |

#### Route 105

Road, West.

**`MAP_ROUTE105`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 30% | Wingull–Pelipper | Wingull–Pelipper |
| 3 | 5% | Luvdisc | Frillish |
| 4 | 4% | Corphish–Crawdaunt | Tynamo–Eelektrik |
| 5 | 1% | Frillish | Luvdisc |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 22% | 18% | 10% | Luvdisc | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Luvdisc |
| 4 | 8% | 10% | 10% | Carvanha–Sharpedo | Frillish |
| 5 | 8% | 9% | 10% | Luvdisc | Carvanha–Sharpedo |
| 6 | 4% | 7% | 10% | Corphish–Crawdaunt | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Wailmer–Wailord | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Frillish | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Carvanha–Sharpedo | Frillish |
| 10 | 2% | 4% | 9% | Tynamo–Eelektrik | Luvdisc |

#### Route 106

Road, West.

**`MAP_ROUTE106`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 2 | 30% | Wingull–Pelipper | Wingull–Pelipper |
| 3 | 5% | Wailmer–Wailord | Carvanha–Sharpedo |
| 4 | 4% | Luvdisc | Wailmer–Wailord |
| 5 | 1% | Ducklett–Swanna | Tynamo–Eelektrik |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 4 | 8% | 10% | 10% | Luvdisc | Tynamo–Eelektrik |
| 5 | 8% | 9% | 10% | Carvanha–Sharpedo | Wailmer–Wailord |
| 6 | 4% | 7% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Frillish |
| 8 | 3% | 5% | 9% | Frillish | Luvdisc |
| 9 | 2% | 4% | 9% | Luvdisc | Tynamo–Eelektrik |
| 10 | 2% | 4% | 9% | Ducklett–Swanna | Corphish–Crawdaunt |

#### Route 109

Road, West.

**`MAP_ROUTE109`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 30% | Wingull–Pelipper | Wingull–Pelipper |
| 3 | 5% | Ducklett–Swanna | Corphish–Crawdaunt |
| 4 | 4% | Luvdisc | Carvanha–Sharpedo |
| 5 | 1% | Oshawott–Samurott | Oshawott–Samurott |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Luvdisc | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Luvdisc | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Carvanha–Sharpedo | Luvdisc |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Ducklett–Swanna | Frillish |
| 9 | 2% | 4% | 9% | Frillish | Tynamo–Eelektrik |
| 10 | 2% | 4% | 9% | Wailmer–Wailord | Corphish–Crawdaunt |

#### Route 110

Road, Centre.

**`MAP_ROUTE110`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Electrike–Manectric | Poochyena–Mightyena |
| 2 | 20% | Gulpin–Swalot | Electrike–Manectric |
| 3 | 10% | Electrike–Manectric | Purrloin–Liepard |
| 4 | 10% | Plusle | Gulpin–Swalot |
| 5 | 10% | Minun | Plusle |
| 6 | 10% | Blitzle–Zebstrika | Minun |
| 7 | 5% | Taillow–Swellow | Trubbish–Garbodor |
| 8 | 5% | Wingull–Pelipper | Woobat |
| 9 | 4% | Trubbish–Garbodor | Illumise |
| 10 | 4% | Patrat–Watchog | Volbeat |
| 11 | 1% | Emolga | Emolga |
| 12 | 1% | Audino | Audino |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 2 | 30% | Wingull–Pelipper | Wailmer–Wailord |
| 3 | 5% | Wailmer–Wailord | Wingull–Pelipper |
| 4 | 4% | Ducklett–Swanna | Carvanha–Sharpedo |
| 5 | 1% | Luvdisc | Frillish |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Wailmer–Wailord |
| 4 | 8% | 10% | 10% | Tynamo–Eelektrik | Frillish |
| 5 | 8% | 9% | 10% | Luvdisc | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Corphish–Crawdaunt | Carvanha–Sharpedo |
| 7 | 3% | 6% | 10% | Wailmer–Wailord | Frillish |
| 8 | 3% | 5% | 9% | Frillish | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Tynamo–Eelektrik | Luvdisc |
| 10 | 2% | 4% | 9% | Corphish–Crawdaunt | Tynamo–Eelektrik |

#### Route 111

Road, Centre.

**`MAP_ROUTE111`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Trapinch–Flygon | Cacnea–Cacturne |
| 2 | 20% | Cacnea–Cacturne | Yamask–Cofagrigus |
| 3 | 10% | Sandile–Krookodile | Baltoy–Claydol |
| 4 | 10% | Baltoy–Claydol | Sandile–Krookodile |
| 5 | 10% | Trapinch–Flygon | Yamask–Cofagrigus |
| 6 | 10% | Darumaka | Trapinch–Flygon |
| 7 | 5% | Scraggy–Scrafty | Woobat |
| 8 | 5% | Dwebble–Crustle | Purrloin–Liepard |
| 9 | 4% | Cacnea–Cacturne | Darumaka |
| 10 | 4% | Sandile–Krookodile | Baltoy–Claydol |
| 11 | 1% | Vullaby–Mandibuzz | Zorua–Zoroark |
| 12 | 1% | Tepig–Emboar | Darumaka–Darmanitan |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 30% | Surskit–Masquerain | Basculin |
| 3 | 5% | Marill–Azumarill | Tympole–Seismitoad |
| 4 | 4% | Basculin | Surskit–Masquerain |
| 5 | 1% | Azurill | Azurill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 22% | 18% | 10% | Basculin | Basculin |
| 3 | 10% | 12% | 11% | Barboach–Whiscash | Tympole–Seismitoad |
| 4 | 8% | 10% | 10% | Surskit–Masquerain | Barboach–Whiscash |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Tympole–Seismitoad |
| 6 | 4% | 7% | 10% | Basculin | Corphish–Crawdaunt |
| 7 | 3% | 6% | 10% | Tympole–Seismitoad | Basculin |
| 8 | 3% | 5% | 9% | Marill–Azumarill | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Barboach–Whiscash | Surskit–Masquerain |
| 10 | 2% | 4% | 9% | Lotad–Lombre | Lotad–Lombre |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Dwebble–Crustle | Roggenrola–Boldore |
| 2 | 30% | Roggenrola–Boldore | Dwebble–Crustle |
| 3 | 5% | Nosepass | Drilbur–Excadrill |
| 4 | 4% | Drilbur–Excadrill | Nosepass |
| 5 | 1% | Roggenrola–Boldore | Aron–Lairon |

#### Route 112

Road, Centre.

**`MAP_ROUTE112`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Numel–Camerupt | Purrloin–Liepard |
| 2 | 20% | Spoink–Grumpig | Numel–Camerupt |
| 3 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 4 | 10% | Timburr–Gurdurr | Marill–Azumarill |
| 5 | 10% | Marill–Azumarill | Numel–Camerupt |
| 6 | 10% | Zigzagoon–Linoone | Woobat |
| 7 | 5% | Mienfoo–Mienshao | Timburr–Gurdurr |
| 8 | 5% | Pansear | Spoink–Grumpig |
| 9 | 4% | Taillow–Swellow | Pansear |
| 10 | 4% | Timburr–Gurdurr | Woobat |
| 11 | 1% | Rufflet–Braviary | Torchic–Blaziken |
| 12 | 1% | Torchic–Blaziken | Azurill |

#### Route 113

Road, Centre.

**`MAP_ROUTE113`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spinda | Spinda |
| 2 | 20% | Numel–Camerupt | Purrloin–Liepard |
| 3 | 10% | Spinda | Duskull–Dusclops |
| 4 | 10% | Pawniard–Bisharp | Spinda |
| 5 | 10% | Trapinch–Flygon | Pawniard–Bisharp |
| 6 | 10% | Cacnea–Cacturne | Numel–Camerupt |
| 7 | 5% | Sandile–Krookodile | Duskull–Dusclops |
| 8 | 5% | Cottonee | Purrloin–Liepard |
| 9 | 4% | Rufflet–Braviary | Yamask–Cofagrigus |
| 10 | 4% | Numel–Camerupt | Cacnea–Cacturne |
| 11 | 1% | Vullaby–Mandibuzz | Zorua–Zoroark |
| 12 | 1% | Spinda | Zorua–Zoroark |

#### Route 115

Road, Centre.

**`MAP_ROUTE115`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Swablu–Altaria | Munna |
| 2 | 20% | Taillow–Swellow | Illumise |
| 3 | 10% | Swablu–Altaria | Woobat |
| 4 | 10% | Pidove–Unfezant | Poochyena–Mightyena |
| 5 | 10% | Wingull–Pelipper | Poochyena–Mightyena |
| 6 | 10% | Minccino | Volbeat |
| 7 | 5% | Taillow–Swellow | Purrloin–Liepard |
| 8 | 5% | Cottonee | Minccino |
| 9 | 4% | Lillipup–Stoutland | Munna |
| 10 | 4% | Pidove–Unfezant | Woobat |
| 11 | 1% | Audino | Audino |
| 12 | 1% | Mudkip–Swampert | Mudkip–Swampert |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Carvanha–Sharpedo |
| 2 | 30% | Ducklett–Swanna | Wailmer–Wailord |
| 3 | 5% | Wailmer–Wailord | Frillish |
| 4 | 4% | Luvdisc | Tynamo–Eelektrik |
| 5 | 1% | Corphish–Crawdaunt | Wingull–Pelipper |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 22% | 18% | 10% | Luvdisc | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Frillish |
| 4 | 8% | 10% | 10% | Carvanha–Sharpedo | Wailmer–Wailord |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Luvdisc | Carvanha–Sharpedo |
| 7 | 3% | 6% | 10% | Frillish | Luvdisc |
| 8 | 3% | 5% | 9% | Corphish–Crawdaunt | Frillish |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Corphish–Crawdaunt |
| 10 | 2% | 4% | 9% | Tynamo–Eelektrik | Tynamo–Eelektrik |

#### Route 116

Road, Centre.

**`MAP_ROUTE116`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Whismur–Exploud | Poochyena–Mightyena |
| 2 | 20% | Taillow–Swellow | Whismur–Exploud |
| 3 | 10% | Nincada–Ninjask | Nincada–Ninjask |
| 4 | 10% | Lillipup–Stoutland | Purrloin–Liepard |
| 5 | 10% | Skitty | Skitty |
| 6 | 10% | Patrat–Watchog | Woobat |
| 7 | 5% | Zigzagoon–Linoone | Munna |
| 8 | 5% | Timburr–Gurdurr | Lillipup–Stoutland |
| 9 | 4% | Solosis–Reuniclus | Poochyena–Mightyena |
| 10 | 4% | Minccino | Solosis–Reuniclus |
| 11 | 1% | Audino | Audino |
| 12 | 1% | Snivy–Serperior | Nincada–Ninjask |

#### Route 117

Road, Centre.

**`MAP_ROUTE117`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Roselia | Volbeat |
| 2 | 20% | Marill–Azumarill | Illumise |
| 3 | 10% | Petilil | Poochyena–Mightyena |
| 4 | 10% | Roselia | Marill–Azumarill |
| 5 | 10% | Seedot–Nuzleaf | Roselia |
| 6 | 10% | Cottonee | Munna |
| 7 | 5% | Lillipup–Stoutland | Purrloin–Liepard |
| 8 | 5% | Taillow–Swellow | Seedot–Nuzleaf |
| 9 | 4% | Patrat–Watchog | Volbeat |
| 10 | 4% | Pidove–Unfezant | Illumise |
| 11 | 1% | Audino | Audino |
| 12 | 1% | Budew | Budew |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Surskit–Masquerain | Surskit–Masquerain |
| 3 | 5% | Lotad–Lombre | Corphish–Crawdaunt |
| 4 | 4% | Ducklett–Swanna | Tympole–Seismitoad |
| 5 | 1% | Azurill | Azurill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Barboach–Whiscash |
| 3 | 10% | 12% | 11% | Surskit–Masquerain | Tympole–Seismitoad |
| 4 | 8% | 10% | 10% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 5 | 8% | 9% | 10% | Barboach–Whiscash | Barboach–Whiscash |
| 6 | 4% | 7% | 10% | Basculin | Marill–Azumarill |
| 7 | 3% | 6% | 10% | Lotad–Lombre | Tympole–Seismitoad |
| 8 | 3% | 5% | 9% | Tympole–Seismitoad | Basculin |
| 9 | 2% | 4% | 9% | Marill–Azumarill | Lotad–Lombre |
| 10 | 2% | 4% | 9% | Ducklett–Swanna | Surskit–Masquerain |

#### Route 118

Road, Centre.

**`MAP_ROUTE118`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Zigzagoon–Linoone | Purrloin–Liepard |
| 2 | 20% | Electrike–Manectric | Electrike–Manectric |
| 3 | 10% | Wingull–Pelipper | Poochyena–Mightyena |
| 4 | 10% | Blitzle–Zebstrika | Zigzagoon–Linoone |
| 5 | 10% | Zigzagoon–Linoone | Blitzle–Zebstrika |
| 6 | 10% | Pidove–Unfezant | Illumise |
| 7 | 5% | Patrat–Watchog | Munna |
| 8 | 5% | Deerling | Kecleon |
| 9 | 4% | Lillipup–Stoutland | Purrloin–Liepard |
| 10 | 4% | Electrike–Manectric | Volbeat |
| 11 | 1% | Kecleon | Zorua–Zoroark |
| 12 | 1% | Kecleon | Kecleon |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 2 | 30% | Wingull–Pelipper | Wingull–Pelipper |
| 3 | 5% | Wailmer–Wailord | Wailmer–Wailord |
| 4 | 4% | Luvdisc | Frillish |
| 5 | 1% | Tynamo–Eelektrik | Carvanha–Sharpedo |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 4 | 8% | 10% | 10% | Frillish–Jellicent | Tynamo–Eelektrik |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Frillish |
| 6 | 4% | 7% | 10% | Wailmer–Wailord | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Tynamo–Eelektrik | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Frillish | Luvdisc |
| 10 | 2% | 4% | 9% | Corphish–Crawdaunt | Frillish |

#### Route 119

Road, East.

**`MAP_ROUTE119`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Zigzagoon–Linoone | Venipede–Scolipede |
| 2 | 20% | Sewaddle–Swadloon | Zigzagoon–Linoone |
| 3 | 10% | Zigzagoon–Linoone | Karrablast |
| 4 | 10% | Pansage | Kecleon |
| 5 | 10% | Petilil | Seedot–Nuzleaf |
| 6 | 10% | Kecleon | Joltik–Galvantula |
| 7 | 5% | Deerling | Sewaddle–Swadloon |
| 8 | 5% | Emolga | Purrloin–Liepard |
| 9 | 4% | Sewaddle–Swadloon | Kecleon |
| 10 | 4% | Kecleon | Shelmet |
| 11 | 1% | Castform | Castform |
| 12 | 1% | Snivy–Serperior | Snivy–Serperior |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Basculin | Carvanha–Sharpedo |
| 2 | 30% | Carvanha–Sharpedo | Basculin |
| 3 | 5% | Barboach–Whiscash | Tympole–Seismitoad |
| 4 | 4% | Ducklett–Swanna | Barboach–Whiscash |
| 5 | 1% | Mudkip–Swampert | Mudkip–Swampert |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Carvanha–Sharpedo | Basculin |
| 2 | 22% | 18% | 10% | Basculin | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Barboach–Whiscash | Tympole–Seismitoad |
| 4 | 8% | 10% | 10% | Basculin | Barboach–Whiscash |
| 5 | 8% | 9% | 10% | Tympole–Seismitoad | Basculin |
| 6 | 4% | 7% | 10% | Marill–Azumarill | Tympole–Seismitoad |
| 7 | 3% | 6% | 10% | Barboach–Whiscash | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Ducklett–Swanna | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Tympole–Seismitoad | Barboach–Whiscash |
| 10 | 2% | 4% | 9% | Feebas | Feebas |

#### Route 121

Road, East.

**`MAP_ROUTE121`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wingull–Pelipper | Shuppet–Banette |
| 2 | 20% | Pidove–Unfezant | Purrloin–Liepard |
| 3 | 10% | Lillipup–Stoutland | Poochyena–Mightyena |
| 4 | 10% | Petilil | Duskull–Dusclops |
| 5 | 10% | Kecleon | Shuppet–Banette |
| 6 | 10% | Patrat–Watchog | Lillipup–Stoutland |
| 7 | 5% | Cottonee | Munna |
| 8 | 5% | Minccino | Kecleon |
| 9 | 4% | Deerling | Litwick–Lampent |
| 10 | 4% | Gothita–Gothitelle | Gothita–Gothitelle |
| 11 | 1% | Kecleon | Litwick–Lampent |
| 12 | 1% | Deerling Spring–Sawsbuck | Kecleon |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Tynamo–Eelektrik |
| 2 | 30% | Ducklett–Swanna | Frillish |
| 3 | 5% | Wailmer–Wailord | Wailmer–Wailord |
| 4 | 4% | Frillish | Carvanha–Sharpedo |
| 5 | 1% | Luvdisc | Wingull–Pelipper |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Frillish |
| 2 | 22% | 18% | 10% | Frillish | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Corphish–Crawdaunt | Frillish |
| 5 | 8% | 9% | 10% | Luvdisc | Carvanha–Sharpedo |
| 6 | 4% | 7% | 10% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Tynamo–Eelektrik | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Frillish | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Luvdisc |
| 10 | 2% | 4% | 9% | Ducklett–Swanna | Carvanha–Sharpedo |

#### Route 122

Road, East.

**`MAP_ROUTE122`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Frillish | Frillish |
| 2 | 30% | Wailmer–Wailord | Tynamo–Eelektrik |
| 3 | 5% | Wingull–Pelipper | Wailmer–Wailord |
| 4 | 4% | Luvdisc | Wingull–Pelipper |
| 5 | 1% | Tynamo–Eelektrik | Frillish–Jellicent |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Frillish | Frillish |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Frillish |
| 4 | 8% | 10% | 10% | Frillish | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Luvdisc | Wailmer–Wailord |
| 7 | 3% | 6% | 10% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Corphish–Crawdaunt | Luvdisc |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Corphish–Crawdaunt |
| 10 | 2% | 4% | 9% | Frillish | Frillish–Jellicent |

#### Route 123

Road, East.

**`MAP_ROUTE123`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Lillipup–Stoutland | Duskull–Dusclops |
| 2 | 20% | Cottonee | Purrloin–Liepard |
| 3 | 10% | Petilil | Shuppet–Banette |
| 4 | 10% | Wingull–Pelipper | Poochyena–Mightyena |
| 5 | 10% | Patrat–Watchog | Lillipup–Stoutland |
| 6 | 10% | Kecleon | Munna |
| 7 | 5% | Minccino | Illumise |
| 8 | 5% | Pidove–Unfezant | Litwick–Lampent |
| 9 | 4% | Roselia | Solosis–Reuniclus |
| 10 | 4% | Solosis–Reuniclus | Kecleon |
| 11 | 1% | Audino | Litwick–Lampent |
| 12 | 1% | Snivy–Serperior | Audino |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Carvanha–Sharpedo |
| 2 | 30% | Ducklett–Swanna | Frillish |
| 3 | 5% | Frillish | Tynamo–Eelektrik |
| 4 | 4% | Wailmer–Wailord | Wailmer–Wailord |
| 5 | 1% | Luvdisc | Wingull–Pelipper |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Frillish | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Frillish |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Frillish | Frillish |
| 5 | 8% | 9% | 10% | Luvdisc | Wailmer–Wailord |
| 6 | 4% | 7% | 10% | Ducklett–Swanna | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Tynamo–Eelektrik | Carvanha–Sharpedo |
| 9 | 2% | 4% | 9% | Carvanha–Sharpedo | Frillish–Jellicent |
| 10 | 2% | 4% | 9% | Luvdisc | Luvdisc |

#### Route 124

Road, Far east.

**`MAP_ROUTE124`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 2 | 30% | Wingull–Pelipper | Frillish |
| 3 | 5% | Wailmer–Wailord | Wailmer–Wailord |
| 4 | 4% | Ducklett–Swanna | Carvanha–Sharpedo |
| 5 | 1% | Frillish | Ducklett–Swanna |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Frillish |
| 3 | 10% | 12% | 11% | Frillish | Carvanha–Sharpedo |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Tynamo–Eelektrik |
| 5 | 8% | 9% | 10% | Tynamo–Eelektrik | Frillish |
| 6 | 4% | 7% | 10% | Luvdisc | Wailmer–Wailord |
| 7 | 3% | 6% | 10% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Frillish | Luvdisc |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Corphish–Crawdaunt |
| 10 | 2% | 4% | 9% | Ducklett–Swanna | Frillish–Jellicent |

#### Route 127

Road, Far east.

**`MAP_ROUTE127`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 30% | Ducklett–Swanna | Frillish |
| 3 | 5% | Frillish | Tynamo–Eelektrik |
| 4 | 4% | Wingull–Pelipper | Ducklett–Swanna |
| 5 | 1% | Luvdisc | Carvanha–Sharpedo |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Frillish | Frillish |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Tynamo–Eelektrik |
| 3 | 10% | 12% | 11% | Ducklett–Swanna | Wailmer–Wailord |
| 4 | 8% | 10% | 10% | Frillish | Frillish |
| 5 | 8% | 9% | 10% | Tynamo–Eelektrik | Carvanha–Sharpedo |
| 6 | 4% | 7% | 10% | Luvdisc | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Wailmer–Wailord | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Luvdisc |
| 9 | 2% | 4% | 9% | Corphish–Crawdaunt | Frillish–Jellicent |
| 10 | 2% | 4% | 9% | Frillish | Corphish–Crawdaunt |

#### Route 128

Road, Far east.

**`MAP_ROUTE128`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Luvdisc | Luvdisc |
| 2 | 30% | Frillish | Frillish |
| 3 | 5% | Ducklett–Swanna | Tynamo–Eelektrik |
| 4 | 4% | Wailmer–Wailord | Wailmer–Wailord |
| 5 | 1% | Wingull–Pelipper | Carvanha–Sharpedo |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Luvdisc | Luvdisc |
| 2 | 22% | 18% | 10% | Frillish | Tynamo–Eelektrik |
| 3 | 10% | 12% | 11% | Luvdisc | Frillish |
| 4 | 8% | 10% | 10% | Tynamo–Eelektrik | Luvdisc |
| 5 | 8% | 9% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 6 | 4% | 7% | 10% | Luvdisc | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Frillish |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Frillish | Wailmer–Wailord |
| 10 | 2% | 4% | 9% | Ducklett–Swanna | Frillish–Jellicent |

#### Petalburg Woods

Road, West.

**`MAP_PETALBURG_WOODS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wurmple | Poochyena–Mightyena |
| 2 | 20% | Shroomish–Breloom | Shroomish–Breloom |
| 3 | 10% | Wurmple–Silcoon | Seedot–Nuzleaf |
| 4 | 10% | Wurmple–Cascoon | Wurmple–Cascoon |
| 5 | 10% | Taillow–Swellow | Venipede–Scolipede |
| 6 | 10% | Shroomish–Breloom | Seedot–Nuzleaf |
| 7 | 5% | Sewaddle–Swadloon | Slakoth–Slaking |
| 8 | 5% | Slakoth–Slaking | Foongus–Amoonguss |
| 9 | 4% | Foongus–Amoonguss | Poochyena–Mightyena |
| 10 | 4% | Sewaddle–Swadloon | Venipede–Scolipede |
| 11 | 1% | Treecko–Sceptile | Budew |
| 12 | 1% | Slakoth–Slaking | Treecko–Sceptile |

#### Route 107

Wilds, West.

**`MAP_ROUTE107`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 30% | Wingull–Pelipper | Frillish |
| 3 | 5% | Frillish | Wingull–Pelipper |
| 4 | 4% | Alomomola | Tynamo–Eelektrik |
| 5 | 1% | Luvdisc | Alomomola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Frillish |
| 4 | 8% | 10% | 10% | Luvdisc | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Frillish | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Corphish–Crawdaunt | Wailmer–Wailord |
| 7 | 3% | 6% | 10% | Carvanha–Sharpedo | Frillish |
| 8 | 3% | 5% | 9% | Wailmer–Wailord | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Alomomola | Alomomola |
| 10 | 2% | 4% | 9% | Luvdisc | Tynamo–Eelektrik |

#### Route 108

Wilds, West.

**`MAP_ROUTE108`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wingull–Pelipper | Carvanha–Sharpedo |
| 2 | 30% | Frillish | Frillish |
| 3 | 5% | Wailmer–Wailord | Wailmer–Wailord |
| 4 | 4% | Alomomola | Tynamo–Eelektrik |
| 5 | 1% | Luvdisc | Alomomola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Frillish |
| 2 | 22% | 18% | 10% | Frillish | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Frillish |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Tynamo–Eelektrik |
| 5 | 8% | 9% | 10% | Luvdisc | Wailmer–Wailord |
| 6 | 4% | 7% | 10% | Frillish | Carvanha–Sharpedo |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Tynamo–Eelektrik |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Alomomola | Alomomola |
| 10 | 2% | 4% | 9% | Luvdisc | Luvdisc |

#### Route 114

Wilds, Centre.

**`MAP_ROUTE114`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Swablu–Altaria | Lotad–Lombre |
| 2 | 20% | Lotad–Lombre | Munna |
| 3 | 10% | Lotad–Lombre | Lotad–Lombre |
| 4 | 10% | Swablu–Altaria | Seedot–Nuzleaf |
| 5 | 10% | Seedot–Nuzleaf | Woobat |
| 6 | 10% | Panpour | Elgyem–Beheeyem |
| 7 | 5% | Deerling | Poochyena–Mightyena |
| 8 | 5% | Tympole–Seismitoad | Illumise |
| 9 | 4% | Elgyem–Beheeyem | Purrloin–Liepard |
| 10 | 4% | Seviper | Zangoose |
| 11 | 1% | Zangoose | Seviper |
| 12 | 1% | Seviper | Shelmet |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Lotad–Lombre | Lotad–Lombre |
| 2 | 30% | Marill–Azumarill | Barboach–Whiscash |
| 3 | 5% | Barboach–Whiscash | Tympole–Seismitoad |
| 4 | 4% | Tympole–Seismitoad | Basculin |
| 5 | 1% | Basculin | Stunfisk |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 22% | 18% | 10% | Lotad–Lombre | Basculin |
| 3 | 10% | 12% | 11% | Basculin | Barboach–Whiscash |
| 4 | 8% | 10% | 10% | Barboach–Whiscash | Tympole–Seismitoad |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Lotad–Lombre |
| 6 | 4% | 7% | 10% | Tympole–Seismitoad | Tympole–Seismitoad |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Basculin | Stunfisk |
| 9 | 2% | 4% | 9% | Surskit–Masquerain | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Stunfisk | Basculin |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Roggenrola–Boldore | Roggenrola–Boldore |
| 2 | 30% | Nosepass | Drilbur–Excadrill |
| 3 | 5% | Dwebble–Crustle | Nosepass |
| 4 | 4% | Drilbur–Excadrill | Dwebble–Crustle |
| 5 | 1% | Durant | Durant |

#### Route 120

Wilds, East.

**`MAP_ROUTE120`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Seedot–Nuzleaf | Poochyena–Mightyena |
| 2 | 20% | Deerling | Seedot–Nuzleaf |
| 3 | 10% | Kecleon | Venipede–Scolipede |
| 4 | 10% | Marill–Azumarill | Kecleon |
| 5 | 10% | Sewaddle–Swadloon | Zorua–Zoroark |
| 6 | 10% | Foongus–Amoonguss | Absol |
| 7 | 5% | Absol | Marill–Azumarill |
| 8 | 5% | Tropius | Foongus–Amoonguss |
| 9 | 4% | Petilil | Purrloin–Liepard |
| 10 | 4% | Karrablast | Shelmet |
| 11 | 1% | Bouffalant | Bouffalant |
| 12 | 1% | Treecko–Sceptile | Tropius |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Tympole–Seismitoad | Basculin |
| 3 | 5% | Lotad–Lombre | Tympole–Seismitoad |
| 4 | 4% | Ducklett–Swanna | Stunfisk |
| 5 | 1% | Mudkip–Swampert | Mudkip–Swampert |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Tympole–Seismitoad |
| 2 | 22% | 18% | 10% | Basculin | Barboach–Whiscash |
| 3 | 10% | 12% | 11% | Tympole–Seismitoad | Basculin |
| 4 | 8% | 10% | 10% | Basculin | Tympole–Seismitoad |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Stunfisk |
| 6 | 4% | 7% | 10% | Barboach–Whiscash | Basculin |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Stunfisk | Barboach–Whiscash |
| 9 | 2% | 4% | 9% | Tympole–Seismitoad | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Surskit–Masquerain | Lotad–Lombre |

#### Route 125

Wilds, Far east.

**`MAP_ROUTE125`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tynamo–Eelektrik | Frillish |
| 2 | 30% | Wingull–Pelipper | Tynamo–Eelektrik |
| 3 | 5% | Wailmer–Wailord | Wailmer–Wailord |
| 4 | 4% | Alomomola | Carvanha–Sharpedo |
| 5 | 1% | Ducklett–Swanna | Alomomola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Frillish |
| 2 | 22% | 18% | 10% | Frillish | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Frillish |
| 5 | 8% | 9% | 10% | Ducklett–Swanna | Carvanha–Sharpedo |
| 6 | 4% | 7% | 10% | Luvdisc | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Frillish | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Tynamo–Eelektrik | Alomomola |
| 9 | 2% | 4% | 9% | Alomomola | Luvdisc |
| 10 | 2% | 4% | 9% | Corphish–Crawdaunt | Frillish–Jellicent |

#### Route 132

Wilds, Far east.

**`MAP_ROUTE132`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 2 | 30% | Ducklett–Swanna | Frillish |
| 3 | 5% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 4% | Alomomola | Alomomola |
| 5 | 1% | Wingull–Pelipper | Wailmer–Wailord |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Frillish | Tynamo–Eelektrik |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Frillish |
| 4 | 8% | 10% | 10% | Tynamo–Eelektrik | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Ducklett–Swanna | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Wailmer–Wailord | Alomomola |
| 7 | 3% | 6% | 10% | Alomomola | Frillish |
| 8 | 3% | 5% | 9% | Luvdisc | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Frillish | Frillish–Jellicent |
| 10 | 2% | 4% | 9% | Corphish–Crawdaunt | Corphish–Crawdaunt |

#### Route 134

Wilds, Far east.

**`MAP_ROUTE134`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tynamo–Eelektrik | Carvanha–Sharpedo |
| 2 | 30% | Wingull–Pelipper | Frillish |
| 3 | 5% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 4% | Alomomola | Wailmer–Wailord |
| 5 | 1% | Ducklett–Swanna | Alomomola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Carvanha–Sharpedo | Frillish |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Frillish | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Frillish |
| 5 | 8% | 9% | 10% | Carvanha–Sharpedo | Ducklett–Swanna |
| 6 | 4% | 7% | 10% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 7 | 3% | 6% | 10% | Luvdisc | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Alomomola | Alomomola |
| 9 | 2% | 4% | 9% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 10 | 2% | 4% | 9% | Luvdisc | Wailmer–Wailord |

#### Jagged Pass

Wilds, Centre.

**`MAP_JAGGED_PASS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spoink–Grumpig | Numel–Camerupt |
| 2 | 20% | Numel–Camerupt | Spoink–Grumpig |
| 3 | 10% | Timburr–Gurdurr | Woobat |
| 4 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 5 | 10% | Spoink–Grumpig | Torkoal |
| 6 | 10% | Torkoal | Purrloin–Liepard |
| 7 | 5% | Mienfoo–Mienshao | Meditite–Medicham |
| 8 | 5% | Pansear | Timburr–Gurdurr |
| 9 | 4% | Meditite–Medicham | Woobat |
| 10 | 4% | Tepig–Emboar | Spoink–Grumpig |
| 11 | 1% | Sawk | Throh |
| 12 | 1% | Tepig–Emboar | Tepig–Emboar |

#### Rusturf Tunnel

Wilds, Centre.

**`MAP_RUSTURF_TUNNEL`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Whismur–Exploud | Whismur–Exploud |
| 2 | 20% | Aron–Aggron | Timburr–Gurdurr |
| 3 | 10% | Timburr–Gurdurr | Woobat |
| 4 | 10% | Whismur–Exploud | Aron–Aggron |
| 5 | 10% | Nosepass | Woobat |
| 6 | 10% | Roggenrola–Boldore | Roggenrola–Boldore |
| 7 | 5% | Woobat | Nosepass |
| 8 | 5% | Drilbur–Excadrill | Drilbur–Excadrill |
| 9 | 4% | Makuhita–Hariyama | Whismur–Exploud |
| 10 | 4% | Aron–Aggron | Sableye |
| 11 | 1% | Durant | Durant |
| 12 | 1% | Mawile | Mawile |

#### Fiery Path

Wilds, Centre.

**`MAP_FIERY_PATH`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Numel–Camerupt | Numel–Camerupt |
| 2 | 20% | Timburr–Gurdurr | Torkoal |
| 3 | 10% | Numel–Camerupt | Numel–Camerupt |
| 4 | 10% | Torkoal | Woobat |
| 5 | 10% | Gulpin–Swalot | Timburr–Gurdurr |
| 6 | 10% | Roggenrola–Boldore | Gulpin–Swalot |
| 7 | 5% | Pansear | Roggenrola–Boldore |
| 8 | 5% | Darumaka | Heatmor |
| 9 | 4% | Torkoal | Darumaka |
| 10 | 4% | Heatmor | Pansear |
| 11 | 1% | Torchic–Blaziken | Heatmor |
| 12 | 1% | Spoink–Grumpig | Torchic–Blaziken |

#### Route 126

Outlands, Far east.

**`MAP_ROUTE126`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Frillish–Jellicent | Frillish–Jellicent |
| 2 | 30% | Wailmer–Wailord | Tynamo–Eelektrik |
| 3 | 5% | Alomomola | Wailmer–Wailord |
| 4 | 4% | Carvanha–Sharpedo | Alomomola |
| 5 | 1% | Wailmer–Wailord | Tynamo–Eelektross |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Frillish–Jellicent | Frillish–Jellicent |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Frillish–Jellicent |
| 4 | 8% | 10% | 10% | Frillish–Jellicent | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Alomomola | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Tynamo–Eelektrik | Alomomola |
| 7 | 3% | 6% | 10% | Luvdisc | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Tynamo–Eelektross |
| 9 | 2% | 4% | 9% | Relicanth | Relicanth |
| 10 | 2% | 4% | 9% | Wailmer–Wailord | Luvdisc |

#### Route 129

Outlands, Far east.

**`MAP_ROUTE129`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 30% | Frillish | Frillish |
| 3 | 5% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 4% | Alomomola | Frillish–Jellicent |
| 5 | 1% | Ducklett–Swanna | Alomomola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Frillish |
| 2 | 22% | 18% | 10% | Frillish | Wailmer–Wailord |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Alomomola | Frillish |
| 6 | 4% | 7% | 10% | Tynamo–Eelektrik | Alomomola |
| 7 | 3% | 6% | 10% | Frillish | Tynamo–Eelektross |
| 8 | 3% | 5% | 9% | Luvdisc | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Frillish–Jellicent | Frillish–Jellicent |
| 10 | 2% | 4% | 9% | Wailmer–Wailord | Luvdisc |

#### Route 130

Outlands, Far east.

**`MAP_ROUTE130`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gothita–Gothitelle | Zorua–Zoroark |
| 2 | 20% | Solosis–Reuniclus | Munna–Musharna |
| 3 | 10% | Wobbuffet | Wobbuffet |
| 4 | 10% | Kecleon | Solosis–Reuniclus |
| 5 | 10% | Castform | Zorua–Zoroark |
| 6 | 10% | Elgyem–Beheeyem | Munna–Musharna |
| 7 | 5% | Wynaut | Wynaut |
| 8 | 5% | Sigilyph | Litwick–Lampent |
| 9 | 4% | Wynaut | Wynaut |
| 10 | 4% | Gothita–Gothitelle | Elgyem–Beheeyem |
| 11 | 1% | Solosis–Reuniclus | Litwick–Chandelure |
| 12 | 1% | Wynaut | Wynaut |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Frillish–Jellicent | Frillish–Jellicent |
| 2 | 30% | Wingull–Pelipper | Tynamo–Eelektrik |
| 3 | 5% | Alomomola | Alomomola |
| 4 | 4% | Wailmer–Wailord | Wailmer–Wailord |
| 5 | 1% | Carvanha–Sharpedo | Carvanha–Sharpedo |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Frillish–Jellicent | Frillish–Jellicent |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Tynamo–Eelektrik |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 4 | 8% | 10% | 10% | Frillish–Jellicent | Frillish–Jellicent |
| 5 | 8% | 9% | 10% | Alomomola | Tynamo–Eelektross |
| 6 | 4% | 7% | 10% | Luvdisc | Alomomola |
| 7 | 3% | 6% | 10% | Tynamo–Eelektrik | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 9 | 2% | 4% | 9% | Relicanth | Relicanth |
| 10 | 2% | 4% | 9% | Corphish–Crawdaunt | Luvdisc |

#### Route 131

Outlands, Far east.

**`MAP_ROUTE131`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 2 | 30% | Wailmer–Wailord | Frillish–Jellicent |
| 3 | 5% | Frillish–Jellicent | Carvanha–Sharpedo |
| 4 | 4% | Alomomola | Alomomola |
| 5 | 1% | Wingull–Pelipper | Tynamo–Eelektross |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Tynamo–Eelektrik | Tynamo–Eelektrik |
| 2 | 22% | 18% | 10% | Frillish–Jellicent | Frillish–Jellicent |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Tynamo–Eelektrik | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Carvanha–Sharpedo | Frillish–Jellicent |
| 6 | 4% | 7% | 10% | Alomomola | Tynamo–Eelektross |
| 7 | 3% | 6% | 10% | Frillish–Jellicent | Alomomola |
| 8 | 3% | 5% | 9% | Luvdisc | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Relicanth | Relicanth |
| 10 | 2% | 4% | 9% | Tynamo–Eelektross | Corphish–Crawdaunt |

#### Route 133

Outlands, Far east.

**`MAP_ROUTE133`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Frillish–Jellicent | Frillish–Jellicent |
| 2 | 30% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 3 | 5% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 4% | Alomomola | Wailmer–Wailord |
| 5 | 1% | Tynamo–Eelektrik | Alomomola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Carvanha–Sharpedo | Frillish–Jellicent |
| 2 | 22% | 18% | 10% | Frillish–Jellicent | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Frillish–Jellicent |
| 5 | 8% | 9% | 10% | Alomomola | Carvanha–Sharpedo |
| 6 | 4% | 7% | 10% | Tynamo–Eelektrik | Alomomola |
| 7 | 3% | 6% | 10% | Frillish–Jellicent | Tynamo–Eelektross |
| 8 | 3% | 5% | 9% | Luvdisc | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Relicanth | Relicanth |
| 10 | 2% | 4% | 9% | Carvanha–Sharpedo | Corphish–Crawdaunt |

#### Underwater Route 124

Outlands, Far east.

**`MAP_UNDERWATER_ROUTE124`**

Water type: underwater.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clamperl | Frillish |
| 2 | 30% | Frillish | Clamperl |
| 3 | 5% | Relicanth | Alomomola |
| 4 | 4% | Alomomola | Relicanth |
| 5 | 1% | Clamperl–Huntail | Clamperl–Gorebyss |

#### Underwater Route 126

Outlands, Far east.

**`MAP_UNDERWATER_ROUTE126`**

Water type: underwater.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Frillish–Jellicent | Frillish–Jellicent |
| 2 | 30% | Clamperl | Relicanth |
| 3 | 5% | Alomomola | Clamperl |
| 4 | 4% | Relicanth | Alomomola |
| 5 | 1% | Clamperl–Gorebyss | Clamperl–Huntail |

#### Granite Cave

Dungeon, West.

**`MAP_GRANITE_CAVE_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Makuhita–Hariyama | Woobat |
| 2 | 20% | Whismur–Exploud | Makuhita–Hariyama |
| 3 | 10% | Makuhita–Hariyama | Whismur–Exploud |
| 4 | 10% | Woobat | Sableye |
| 5 | 10% | Nosepass | Makuhita–Hariyama |
| 6 | 10% | Aron–Aggron | Woobat |
| 7 | 5% | Roggenrola–Boldore | Aron–Aggron |
| 8 | 5% | Sableye | Roggenrola–Boldore |
| 9 | 4% | Mawile | Sableye |
| 10 | 4% | Drilbur–Excadrill | Mawile |
| 11 | 1% | Throh | Throh |
| 12 | 1% | Woobat | Drilbur–Excadrill |

**`MAP_GRANITE_CAVE_B1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Aron–Aggron | Aron–Aggron |
| 2 | 20% | Mawile | Sableye |
| 3 | 10% | Aron–Aggron | Woobat |
| 4 | 10% | Makuhita–Hariyama | Aron–Aggron |
| 5 | 10% | Roggenrola–Boldore | Mawile |
| 6 | 10% | Sableye | Makuhita–Hariyama |
| 7 | 5% | Woobat | Sableye |
| 8 | 5% | Nosepass | Roggenrola–Boldore |
| 9 | 4% | Drilbur–Excadrill | Woobat |
| 10 | 4% | Whismur–Exploud | Nosepass |
| 11 | 1% | Sawk | Sawk |
| 12 | 1% | Throh | Throh |

**`MAP_GRANITE_CAVE_B2F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Aron–Aggron | Sableye |
| 2 | 20% | Sableye | Aron–Aggron |
| 3 | 10% | Mawile | Mawile |
| 4 | 10% | Nosepass | Sableye |
| 5 | 10% | Makuhita–Hariyama | Woobat |
| 6 | 10% | Roggenrola–Boldore | Nosepass |
| 7 | 5% | Woobat | Makuhita–Hariyama |
| 8 | 5% | Drilbur–Excadrill | Roggenrola–Boldore |
| 9 | 4% | Nosepass | Woobat |
| 10 | 4% | Mawile | Drilbur–Excadrill |
| 11 | 1% | Sawk | Sawk |
| 12 | 1% | Nosepass–Probopass | Nosepass–Probopass |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Nosepass | Nosepass |
| 2 | 30% | Roggenrola–Boldore | Roggenrola–Boldore |
| 3 | 5% | Dwebble–Crustle | Sableye |
| 4 | 4% | Aron–Aggron | Dwebble–Crustle |
| 5 | 1% | Drilbur–Excadrill | Aron–Aggron |

**`MAP_GRANITE_CAVE_STEVENS_ROOM`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Mawile | Mawile |
| 2 | 20% | Aron–Aggron | Aron–Aggron |
| 3 | 10% | Makuhita–Hariyama | Sableye |
| 4 | 10% | Sableye | Makuhita–Hariyama |
| 5 | 10% | Nosepass | Sableye |
| 6 | 10% | Roggenrola–Boldore | Woobat |
| 7 | 5% | Mawile | Nosepass |
| 8 | 5% | Drilbur–Excadrill | Roggenrola–Boldore |
| 9 | 4% | Nosepass–Probopass | Woobat–Swoobat |
| 10 | 4% | Roggenrola–Gigalith | Nosepass–Probopass |
| 11 | 1% | Throh | Sawk |
| 12 | 1% | Beldum–Metagross | Beldum–Metagross |

#### Altering Cave

Dungeon, West.

**`MAP_ALTERING_CAVE`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Whismur–Exploud | Woobat |
| 2 | 20% | Nosepass | Whismur–Exploud |
| 3 | 10% | Aron–Lairon | Sableye |
| 4 | 10% | Makuhita–Hariyama | Aron–Lairon |
| 5 | 10% | Woobat | Munna |
| 6 | 10% | Sableye | Makuhita–Hariyama |
| 7 | 5% | Roggenrola–Boldore | Roggenrola–Boldore |
| 8 | 5% | Mawile | Nosepass |
| 9 | 4% | Nosepass–Probopass | Zorua–Zoroark |
| 10 | 4% | Zorua–Zoroark | Mawile |
| 11 | 1% | Chingling | Woobat–Swoobat |
| 12 | 1% | Sableye | Chingling |

#### Abandoned Ship

Dungeon, West.

**`MAP_ABANDONED_SHIP_ROOMS_B1F`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 2 | 30% | Frillish | Frillish |
| 3 | 5% | Corphish–Crawdaunt | Tynamo–Eelektrik |
| 4 | 4% | Wailmer–Wailord | Corphish–Crawdaunt |
| 5 | 1% | Luvdisc | Frillish |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corphish–Crawdaunt | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Corphish–Crawdaunt |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Frillish |
| 4 | 8% | 10% | 10% | Corphish–Crawdaunt | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Frillish | Tynamo–Eelektrik |
| 6 | 4% | 7% | 10% | Luvdisc | Corphish–Crawdaunt |
| 7 | 3% | 6% | 10% | Carvanha–Sharpedo | Frillish |
| 8 | 3% | 5% | 9% | Frillish | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Alomomola |
| 10 | 2% | 4% | 9% | Alomomola | Luvdisc |

**`MAP_ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Frillish–Jellicent | Frillish–Jellicent |
| 2 | 30% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 3 | 5% | Corphish–Crawdaunt | Carvanha–Sharpedo |
| 4 | 4% | Alomomola | Alomomola |
| 5 | 1% | Wailmer–Wailord | Tynamo–Eelektross |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corphish–Crawdaunt | Frillish–Jellicent |
| 2 | 22% | 18% | 10% | Frillish–Jellicent | Carvanha–Sharpedo |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Corphish–Crawdaunt |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Frillish–Jellicent |
| 6 | 4% | 7% | 10% | Luvdisc | Tynamo–Eelektross |
| 7 | 3% | 6% | 10% | Frillish–Jellicent | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Alomomola | Alomomola |
| 10 | 2% | 4% | 9% | Luvdisc | Luvdisc |

#### New Mauville

Dungeon, Centre.

**`MAP_NEW_MAUVILLE_ENTRANCE`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Electrike–Manectric | Klink |
| 2 | 20% | Plusle | Joltik |
| 3 | 10% | Minun | Electrike–Manectric |
| 4 | 10% | Klink | Joltik |
| 5 | 10% | Joltik | Plusle |
| 6 | 10% | Electrike–Manectric | Minun |
| 7 | 5% | Klink | Tynamo–Eelektrik |
| 8 | 5% | Tynamo–Eelektrik | Trubbish–Garbodor |
| 9 | 4% | Trubbish–Garbodor | Woobat |
| 10 | 4% | Ferroseed | Ferroseed |
| 11 | 1% | Stunfisk | Stunfisk |
| 12 | 1% | Emolga | Emolga |

**`MAP_NEW_MAUVILLE_INSIDE`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Klink–Klang | Joltik–Galvantula |
| 2 | 20% | Electrike–Manectric | Klink–Klang |
| 3 | 10% | Joltik–Galvantula | Electrike–Manectric |
| 4 | 10% | Plusle | Tynamo–Eelektrik |
| 5 | 10% | Minun | Plusle |
| 6 | 10% | Tynamo–Eelektrik | Minun |
| 7 | 5% | Electrike–Manectric | Ferroseed–Ferrothorn |
| 8 | 5% | Ferroseed–Ferrothorn | Woobat |
| 9 | 4% | Trubbish–Garbodor | Electrike–Manectric |
| 10 | 4% | Klink | Joltik |
| 11 | 1% | Stunfisk | Stunfisk |
| 12 | 1% | Tynamo–Eelektross | Tynamo–Eelektross |

#### Desert Underpass

Dungeon, Centre.

**`MAP_DESERT_UNDERPASS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Whismur–Exploud | Whismur–Exploud |
| 2 | 20% | Sandile–Krookodile | Yamask–Cofagrigus |
| 3 | 10% | Trapinch–Flygon | Sandile–Krookodile |
| 4 | 10% | Whismur–Exploud | Trapinch–Flygon |
| 5 | 10% | Cacnea–Cacturne | Whismur–Exploud |
| 6 | 10% | Drilbur–Excadrill | Woobat |
| 7 | 5% | Dwebble–Crustle | Drilbur–Excadrill |
| 8 | 5% | Yamask–Cofagrigus | Cacnea–Cacturne |
| 9 | 4% | Nosepass | Yamask–Cofagrigus |
| 10 | 4% | Sandile–Krookodile | Dwebble–Crustle |
| 11 | 1% | Zorua–Zoroark | Zorua–Zoroark |
| 12 | 1% | Durant | Durant |

#### Mirage Tower

Dungeon, Centre.

**`MAP_MIRAGE_TOWER_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Trapinch–Flygon | Cacnea–Cacturne |
| 2 | 20% | Baltoy–Claydol | Yamask–Cofagrigus |
| 3 | 10% | Trapinch–Flygon | Trapinch–Flygon |
| 4 | 10% | Sandile–Krookodile | Baltoy–Claydol |
| 5 | 10% | Cacnea–Cacturne | Sandile–Krookodile |
| 6 | 10% | Baltoy–Claydol | Yamask–Cofagrigus |
| 7 | 5% | Sandile–Krookodile | Cacnea–Cacturne |
| 8 | 5% | Darumaka | Baltoy–Claydol |
| 9 | 4% | Cacnea–Cacturne | Golett–Golurk |
| 10 | 4% | Yamask–Cofagrigus | Woobat |
| 11 | 1% | Vullaby–Mandibuzz | Vullaby–Mandibuzz |
| 12 | 1% | Maractus | Maractus |

**`MAP_MIRAGE_TOWER_2F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Baltoy–Claydol | Yamask–Cofagrigus |
| 2 | 20% | Trapinch–Flygon | Baltoy–Claydol |
| 3 | 10% | Cacnea–Cacturne | Cacnea–Cacturne |
| 4 | 10% | Sandile–Krookodile | Golett–Golurk |
| 5 | 10% | Baltoy–Claydol | Trapinch–Flygon |
| 6 | 10% | Trapinch–Flygon | Sandile–Krookodile |
| 7 | 5% | Yamask–Cofagrigus | Yamask–Cofagrigus |
| 8 | 5% | Darumaka | Woobat |
| 9 | 4% | Dwebble–Crustle | Cacnea–Cacturne |
| 10 | 4% | Golett–Golurk | Darumaka |
| 11 | 1% | Maractus | Sigilyph |
| 12 | 1% | Sigilyph | Maractus |

**`MAP_MIRAGE_TOWER_3F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Baltoy–Claydol | Golett–Golurk |
| 2 | 20% | Cacnea–Cacturne | Baltoy–Claydol |
| 3 | 10% | Golett–Golurk | Yamask–Cofagrigus |
| 4 | 10% | Trapinch–Flygon | Cacnea–Cacturne |
| 5 | 10% | Sandile–Krookodile | Trapinch–Flygon |
| 6 | 10% | Baltoy–Claydol | Baltoy–Claydol |
| 7 | 5% | Yamask–Cofagrigus | Woobat–Swoobat |
| 8 | 5% | Darumaka | Darumaka |
| 9 | 4% | Sigilyph | Sigilyph |
| 10 | 4% | Maractus | Maractus |
| 11 | 1% | Vullaby–Mandibuzz | Vullaby–Mandibuzz |
| 12 | 1% | Sigilyph | Sigilyph |

**`MAP_MIRAGE_TOWER_4F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Trapinch–Flygon | Baltoy–Claydol |
| 2 | 20% | Baltoy–Claydol | Trapinch–Flygon |
| 3 | 10% | Cacnea–Cacturne | Yamask–Cofagrigus |
| 4 | 10% | Golett–Golurk | Cacnea–Cacturne |
| 5 | 10% | Sigilyph | Sigilyph |
| 6 | 10% | Trapinch–Flygon | Golett–Golurk |
| 7 | 5% | Darumaka–Darmanitan | Woobat–Swoobat |
| 8 | 5% | Sandile–Krookodile | Sandile–Krookodile |
| 9 | 4% | Maractus | Sigilyph |
| 10 | 4% | Sigilyph | Maractus |
| 11 | 1% | Vullaby–Mandibuzz | Zorua–Zoroark |
| 12 | 1% | Darumaka–Darmanitan | Darumaka–Darmanitan |

#### Meteor Falls

Dungeon, Centre.

**`MAP_METEOR_FALLS_1F_1R`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Solrock | Lunatone |
| 2 | 20% | Woobat | Woobat |
| 3 | 10% | Solrock | Lunatone |
| 4 | 10% | Roggenrola–Boldore | Roggenrola–Boldore |
| 5 | 10% | Meditite–Medicham | Meditite–Medicham |
| 6 | 10% | Aron–Aggron | Elgyem–Beheeyem |
| 7 | 5% | Elgyem–Beheeyem | Aron–Aggron |
| 8 | 5% | Lunatone | Solrock |
| 9 | 4% | Mawile | Mawile |
| 10 | 4% | Drilbur–Excadrill | Drilbur–Excadrill |
| 11 | 1% | Swablu–Altaria | Swablu–Altaria |
| 12 | 1% | Druddigon | Druddigon |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 30% | Basculin | Tympole–Seismitoad |
| 3 | 5% | Tympole–Seismitoad | Basculin |
| 4 | 4% | Marill–Azumarill | Corphish–Crawdaunt |
| 5 | 1% | Solrock | Lunatone |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 22% | 18% | 10% | Basculin | Tympole–Seismitoad |
| 3 | 10% | 12% | 11% | Barboach–Whiscash | Barboach–Whiscash |
| 4 | 8% | 10% | 10% | Tympole–Seismitoad | Basculin |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Tympole–Seismitoad |
| 6 | 4% | 7% | 10% | Basculin | Corphish–Crawdaunt |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Barboach–Whiscash |
| 8 | 3% | 5% | 9% | Barboach–Whiscash | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Tympole–Seismitoad | Tympole–Seismitoad |
| 10 | 2% | 4% | 9% | Barboach–Whiscash | Basculin |

**`MAP_METEOR_FALLS_1F_2R`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Solrock | Lunatone |
| 2 | 20% | Mawile | Mawile |
| 3 | 10% | Roggenrola–Boldore | Woobat–Swoobat |
| 4 | 10% | Lunatone | Solrock |
| 5 | 10% | Meditite–Medicham | Meditite–Medicham |
| 6 | 10% | Woobat–Swoobat | Roggenrola–Boldore |
| 7 | 5% | Elgyem–Beheeyem | Elgyem–Beheeyem |
| 8 | 5% | Aron–Aggron | Mawile |
| 9 | 4% | Mawile | Aron–Aggron |
| 10 | 4% | Drilbur–Excadrill | Drilbur–Excadrill |
| 11 | 1% | Druddigon | Druddigon |
| 12 | 1% | Mawile | Sableye |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 30% | Tympole–Seismitoad | Tympole–Seismitoad |
| 3 | 5% | Marill–Azumarill | Basculin |
| 4 | 4% | Basculin | Corphish–Crawdaunt |
| 5 | 1% | Solrock | Lunatone |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 22% | 18% | 10% | Marill–Azumarill | Corphish–Crawdaunt |
| 3 | 10% | 12% | 11% | Basculin | Tympole–Seismitoad |
| 4 | 8% | 10% | 10% | Barboach–Whiscash | Barboach–Whiscash |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Basculin |
| 6 | 4% | 7% | 10% | Tympole–Seismitoad | Corphish–Crawdaunt |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Tympole–Seismitoad |
| 8 | 3% | 5% | 9% | Barboach–Whiscash | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Basculin | Stunfisk |
| 10 | 2% | 4% | 9% | Stunfisk | Barboach–Whiscash |

**`MAP_METEOR_FALLS_B1F_1R`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Lunatone | Lunatone |
| 2 | 20% | Solrock | Solrock |
| 3 | 10% | Roggenrola–Boldore | Woobat–Swoobat |
| 4 | 10% | Drilbur–Excadrill | Roggenrola–Boldore |
| 5 | 10% | Woobat–Swoobat | Drilbur–Excadrill |
| 6 | 10% | Meditite–Medicham | Elgyem–Beheeyem |
| 7 | 5% | Elgyem–Beheeyem | Meditite–Medicham |
| 8 | 5% | Aron–Aggron | Sableye |
| 9 | 4% | Drilbur–Excadrill | Roggenrola–Gigalith |
| 10 | 4% | Druddigon | Druddigon |
| 11 | 1% | Bagon–Salamence | Bagon–Salamence |
| 12 | 1% | Axew–Fraxure | Axew–Haxorus |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Marill–Azumarill | Marill–Azumarill |
| 2 | 30% | Barboach–Whiscash | Barboach–Whiscash |
| 3 | 5% | Basculin | Corphish–Crawdaunt |
| 4 | 4% | Tympole–Seismitoad | Tympole–Seismitoad |
| 5 | 1% | Solrock | Lunatone |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 22% | 18% | 10% | Basculin | Tympole–Seismitoad |
| 3 | 10% | 12% | 11% | Tympole–Seismitoad | Basculin |
| 4 | 8% | 10% | 10% | Barboach–Whiscash | Tympole–Seismitoad |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Corphish–Crawdaunt |
| 6 | 4% | 7% | 10% | Corphish–Crawdaunt | Barboach–Whiscash |
| 7 | 3% | 6% | 10% | Basculin | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Tympole–Seismitoad | Basculin |
| 9 | 2% | 4% | 9% | Barboach–Whiscash | Stunfisk |
| 10 | 2% | 4% | 9% | Stunfisk | Marill–Azumarill |

**`MAP_METEOR_FALLS_B1F_2R`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Solrock | Lunatone |
| 2 | 20% | Lunatone | Solrock |
| 3 | 10% | Bagon–Salamence | Bagon–Salamence |
| 4 | 10% | Roggenrola–Boldore | Woobat–Swoobat |
| 5 | 10% | Drilbur–Excadrill | Drilbur–Excadrill |
| 6 | 10% | Woobat–Swoobat | Roggenrola–Boldore |
| 7 | 5% | Elgyem–Beheeyem | Sableye |
| 8 | 5% | Bagon–Salamence | Bagon–Salamence |
| 9 | 4% | Druddigon | Druddigon |
| 10 | 4% | Roggenrola–Gigalith | Elgyem–Beheeyem |
| 11 | 1% | Axew–Haxorus | Axew–Haxorus |
| 12 | 1% | Aron–Aggron | Roggenrola–Gigalith |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 30% | Tympole–Seismitoad | Basculin |
| 3 | 5% | Basculin | Tympole–Seismitoad |
| 4 | 4% | Marill–Azumarill | Corphish–Crawdaunt |
| 5 | 1% | Solrock | Lunatone |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Barboach–Whiscash |
| 2 | 22% | 18% | 10% | Tympole–Seismitoad | Corphish–Crawdaunt |
| 3 | 10% | 12% | 11% | Basculin | Tympole–Seismitoad |
| 4 | 8% | 10% | 10% | Barboach–Whiscash | Basculin |
| 5 | 8% | 9% | 10% | Marill–Azumarill | Corphish–Crawdaunt |
| 6 | 4% | 7% | 10% | Tympole–Seismitoad | Tympole–Seismitoad |
| 7 | 3% | 6% | 10% | Basculin | Barboach–Whiscash |
| 8 | 3% | 5% | 9% | Barboach–Whiscash | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Stunfisk | Stunfisk |
| 10 | 2% | 4% | 9% | Tympole–Seismitoad | Basculin |

**`MAP_METEOR_FALLS_STEVENS_CAVE`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Solrock | Lunatone |
| 2 | 20% | Lunatone | Solrock |
| 3 | 10% | Roggenrola–Gigalith | Elgyem–Beheeyem |
| 4 | 10% | Elgyem–Beheeyem | Roggenrola–Gigalith |
| 5 | 10% | Bagon–Salamence | Bagon–Salamence |
| 6 | 10% | Beldum–Metagross | Beldum–Metagross |
| 7 | 5% | Mawile | Woobat–Swoobat |
| 8 | 5% | Drilbur–Excadrill | Mawile |
| 9 | 4% | Druddigon | Druddigon |
| 10 | 4% | Axew–Haxorus | Axew–Haxorus |
| 11 | 1% | Beldum–Metagross | Beldum–Metagross |
| 12 | 1% | Aron–Aggron | Sableye |

#### Magma Hideout

Dungeon, Centre.

**`MAP_MAGMA_HIDEOUT_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Numel–Camerupt | Numel–Camerupt |
| 2 | 20% | Roggenrola–Boldore | Roggenrola–Boldore |
| 3 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 4 | 10% | Torkoal | Torkoal |
| 5 | 10% | Poochyena–Mightyena | Numel–Camerupt |
| 6 | 10% | Gulpin–Swalot | Poochyena–Mightyena |
| 7 | 5% | Darumaka | Woobat |
| 8 | 5% | Timburr–Gurdurr | Darumaka |
| 9 | 4% | Torkoal | Timburr–Gurdurr |
| 10 | 4% | Poochyena–Mightyena | Torkoal |
| 11 | 1% | Heatmor | Heatmor |
| 12 | 1% | Torchic–Blaziken | Torchic–Blaziken |

**`MAP_MAGMA_HIDEOUT_2F_1R`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Roggenrola–Boldore | Roggenrola–Boldore |
| 2 | 20% | Torkoal | Torkoal |
| 3 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 4 | 10% | Poochyena–Mightyena | Numel–Camerupt |
| 5 | 10% | Darumaka | Darumaka |
| 6 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 7 | 5% | Timburr–Gurdurr | Woobat |
| 8 | 5% | Heatmor | Heatmor |
| 9 | 4% | Torkoal | Numel–Camerupt |
| 10 | 4% | Gulpin–Swalot | Timburr–Gurdurr |
| 11 | 1% | Durant | Durant |
| 12 | 1% | Torkoal | Torkoal |

**`MAP_MAGMA_HIDEOUT_2F_2R`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Numel–Camerupt | Numel–Camerupt |
| 2 | 20% | Torkoal | Torkoal |
| 3 | 10% | Poochyena–Mightyena | Poochyena–Mightyena |
| 4 | 10% | Roggenrola–Boldore | Poochyena–Mightyena |
| 5 | 10% | Numel–Camerupt | Roggenrola–Boldore |
| 6 | 10% | Darumaka | Heatmor |
| 7 | 5% | Heatmor | Woobat |
| 8 | 5% | Timburr–Gurdurr | Darumaka |
| 9 | 4% | Torkoal | Torkoal |
| 10 | 4% | Durant | Timburr–Gurdurr |
| 11 | 1% | Pansear–Simisear | Durant |
| 12 | 1% | Roggenrola–Gigalith | Roggenrola–Gigalith |

**`MAP_MAGMA_HIDEOUT_2F_3R`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Darumaka | Torkoal |
| 2 | 20% | Torkoal | Darumaka |
| 3 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 4 | 10% | Heatmor | Numel–Camerupt |
| 5 | 10% | Poochyena–Mightyena | Heatmor |
| 6 | 10% | Gulpin–Swalot | Woobat |
| 7 | 5% | Numel–Camerupt | Poochyena–Mightyena |
| 8 | 5% | Durant | Durant |
| 9 | 4% | Torkoal | Numel–Camerupt |
| 10 | 4% | Timburr–Conkeldurr | Timburr–Conkeldurr |
| 11 | 1% | Pansear–Simisear | Pansear–Simisear |
| 12 | 1% | Larvesta–Volcarona | Larvesta–Volcarona |

**`MAP_MAGMA_HIDEOUT_3F_1R`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Numel–Camerupt | Numel–Camerupt |
| 2 | 20% | Roggenrola–Boldore | Roggenrola–Boldore |
| 3 | 10% | Torkoal | Poochyena–Mightyena |
| 4 | 10% | Poochyena–Mightyena | Torkoal |
| 5 | 10% | Heatmor | Poochyena–Mightyena |
| 6 | 10% | Numel–Camerupt | Heatmor |
| 7 | 5% | Durant | Woobat |
| 8 | 5% | Roggenrola–Gigalith | Roggenrola–Gigalith |
| 9 | 4% | Torkoal | Durant |
| 10 | 4% | Darumaka–Darmanitan | Darumaka–Darmanitan |
| 11 | 1% | Larvesta–Volcarona | Larvesta–Volcarona |
| 12 | 1% | Pansear–Simisear | Timburr–Conkeldurr |

**`MAP_MAGMA_HIDEOUT_3F_2R`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Torkoal | Torkoal |
| 2 | 20% | Heatmor | Heatmor |
| 3 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 4 | 10% | Poochyena–Mightyena | Numel–Camerupt |
| 5 | 10% | Durant | Durant |
| 6 | 10% | Roggenrola–Gigalith | Poochyena–Mightyena |
| 7 | 5% | Numel–Camerupt | Woobat–Swoobat |
| 8 | 5% | Torkoal | Roggenrola–Gigalith |
| 9 | 4% | Darumaka–Darmanitan | Darumaka–Darmanitan |
| 10 | 4% | Timburr–Conkeldurr | Timburr–Conkeldurr |
| 11 | 1% | Larvesta–Volcarona | Larvesta–Volcarona |
| 12 | 1% | Tepig–Emboar | Tepig–Emboar |

**`MAP_MAGMA_HIDEOUT_3F_3R`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Heatmor | Heatmor |
| 2 | 20% | Torkoal | Torkoal |
| 3 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 4 | 10% | Numel–Camerupt | Numel–Camerupt |
| 5 | 10% | Poochyena–Mightyena | Darumaka–Darmanitan |
| 6 | 10% | Durant | Woobat–Swoobat |
| 7 | 5% | Poochyena–Mightyena | Durant |
| 8 | 5% | Torkoal | Roggenrola–Gigalith |
| 9 | 4% | Larvesta–Volcarona | Larvesta–Volcarona |
| 10 | 4% | Darumaka–Darmanitan | Timburr–Conkeldurr |
| 11 | 1% | Larvesta–Volcarona | Larvesta–Volcarona |
| 12 | 1% | Pansear–Simisear | Poochyena–Mightyena |

**`MAP_MAGMA_HIDEOUT_4F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Numel–Camerupt | Numel–Camerupt |
| 2 | 20% | Torkoal | Torkoal |
| 3 | 10% | Heatmor | Poochyena–Mightyena |
| 4 | 10% | Poochyena–Mightyena | Heatmor |
| 5 | 10% | Roggenrola–Gigalith | Numel–Camerupt |
| 6 | 10% | Numel–Camerupt | Poochyena–Mightyena |
| 7 | 5% | Larvesta–Volcarona | Larvesta–Volcarona |
| 8 | 5% | Darumaka–Darmanitan | Woobat–Swoobat |
| 9 | 4% | Durant | Darumaka–Darmanitan |
| 10 | 4% | Larvesta–Volcarona | Larvesta–Volcarona |
| 11 | 1% | Timburr–Conkeldurr | Durant |
| 12 | 1% | Torchic–Blaziken | Torchic–Blaziken |

#### Mt. Pyre

Dungeon, East.

**`MAP_MT_PYRE_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Shuppet–Banette | Shuppet–Banette |
| 2 | 20% | Yamask–Cofagrigus | Duskull–Dusclops |
| 3 | 10% | Shuppet–Banette | Shuppet–Banette |
| 4 | 10% | Golett–Golurk | Yamask–Cofagrigus |
| 5 | 10% | Duskull–Dusclops | Duskull–Dusclops |
| 6 | 10% | Yamask–Cofagrigus | Golett–Golurk |
| 7 | 5% | Litwick–Lampent | Litwick–Lampent |
| 8 | 5% | Shuppet–Banette | Duskull–Dusclops |
| 9 | 4% | Golett–Golurk | Yamask–Cofagrigus |
| 10 | 4% | Sableye | Sableye |
| 11 | 1% | Litwick–Lampent | Litwick–Lampent |
| 12 | 1% | Sableye | Golett–Golurk |

**`MAP_MT_PYRE_2F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Shuppet–Banette | Shuppet–Banette |
| 2 | 20% | Golett–Golurk | Yamask–Cofagrigus |
| 3 | 10% | Yamask–Cofagrigus | Duskull–Dusclops |
| 4 | 10% | Shuppet–Banette | Golett–Golurk |
| 5 | 10% | Duskull–Dusclops | Duskull–Dusclops |
| 6 | 10% | Golett–Golurk | Shuppet–Banette |
| 7 | 5% | Sableye | Litwick–Lampent |
| 8 | 5% | Litwick–Lampent | Sableye |
| 9 | 4% | Yamask–Cofagrigus | Yamask–Cofagrigus |
| 10 | 4% | Golett–Golurk | Litwick–Lampent |
| 11 | 1% | Litwick–Lampent | Golett–Golurk |
| 12 | 1% | Litwick–Chandelure | Litwick–Chandelure |

**`MAP_MT_PYRE_3F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Golett–Golurk | Duskull–Dusclops |
| 2 | 20% | Shuppet–Banette | Golett–Golurk |
| 3 | 10% | Yamask–Cofagrigus | Shuppet–Banette |
| 4 | 10% | Duskull–Dusclops | Duskull–Dusclops |
| 5 | 10% | Shuppet–Banette | Yamask–Cofagrigus |
| 6 | 10% | Golett–Golurk | Litwick–Lampent |
| 7 | 5% | Sableye | Sableye |
| 8 | 5% | Litwick–Lampent | Golett–Golurk |
| 9 | 4% | Vullaby–Mandibuzz | Vullaby–Mandibuzz |
| 10 | 4% | Yamask–Cofagrigus | Litwick–Lampent |
| 11 | 1% | Litwick–Chandelure | Litwick–Chandelure |
| 12 | 1% | Litwick–Lampent | Shuppet–Banette |

**`MAP_MT_PYRE_4F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Duskull–Dusclops | Duskull–Dusclops |
| 2 | 20% | Shuppet–Banette | Shuppet–Banette |
| 3 | 10% | Golett–Golurk | Duskull–Dusclops |
| 4 | 10% | Yamask–Cofagrigus | Litwick–Lampent |
| 5 | 10% | Duskull–Dusclops | Yamask–Cofagrigus |
| 6 | 10% | Sableye | Golett–Golurk |
| 7 | 5% | Litwick–Lampent | Litwick–Lampent |
| 8 | 5% | Yamask–Cofagrigus | Sableye |
| 9 | 4% | Shuppet–Banette | Yamask–Cofagrigus |
| 10 | 4% | Litwick–Chandelure | Litwick–Chandelure |
| 11 | 1% | Duskull–Dusknoir | Duskull–Dusknoir |
| 12 | 1% | Litwick–Lampent | Litwick–Lampent |

**`MAP_MT_PYRE_5F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Golett–Golurk | Duskull–Dusclops |
| 2 | 20% | Duskull–Dusclops | Golett–Golurk |
| 3 | 10% | Shuppet–Banette | Litwick–Lampent |
| 4 | 10% | Yamask–Cofagrigus | Shuppet–Banette |
| 5 | 10% | Duskull–Dusclops | Duskull–Dusclops |
| 6 | 10% | Sableye | Litwick–Lampent |
| 7 | 5% | Litwick–Lampent | Yamask–Cofagrigus |
| 8 | 5% | Vullaby–Mandibuzz | Sableye |
| 9 | 4% | Golett–Golurk | Vullaby–Mandibuzz |
| 10 | 4% | Litwick–Chandelure | Litwick–Chandelure |
| 11 | 1% | Duskull–Dusknoir | Duskull–Dusknoir |
| 12 | 1% | Shuppet–Banette | Litwick–Lampent |

**`MAP_MT_PYRE_6F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Duskull–Dusclops | Shuppet–Banette |
| 2 | 20% | Shuppet–Banette | Duskull–Dusclops |
| 3 | 10% | Golett–Golurk | Litwick–Lampent |
| 4 | 10% | Yamask–Cofagrigus | Duskull–Dusclops |
| 5 | 10% | Litwick–Lampent | Litwick–Lampent |
| 6 | 10% | Duskull–Dusclops | Yamask–Cofagrigus |
| 7 | 5% | Sableye | Golett–Golurk |
| 8 | 5% | Litwick–Chandelure | Litwick–Chandelure |
| 9 | 4% | Golett–Golurk | Zorua–Zoroark |
| 10 | 4% | Duskull–Dusknoir | Duskull–Dusknoir |
| 11 | 1% | Litwick–Lampent | Litwick–Chandelure |
| 12 | 1% | Litwick–Chandelure | Golett–Golurk |

**`MAP_MT_PYRE_EXTERIOR`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Vullaby–Mandibuzz | Shuppet–Banette |
| 2 | 20% | Swablu–Altaria | Duskull–Dusclops |
| 3 | 10% | Wingull–Pelipper | Zorua–Zoroark |
| 4 | 10% | Meditite–Medicham | Litwick–Lampent |
| 5 | 10% | Golett–Golurk | Yamask–Cofagrigus |
| 6 | 10% | Tropius | Duskull–Dusknoir |
| 7 | 5% | Vullaby–Mandibuzz | Golett–Golurk |
| 8 | 5% | Golett–Golurk | Shuppet–Banette |
| 9 | 4% | Absol | Absol |
| 10 | 4% | Tropius | Duskull–Dusclops |
| 11 | 1% | Rufflet–Braviary | Litwick–Chandelure |
| 12 | 1% | Absol | Tropius |

**`MAP_MT_PYRE_SUMMIT`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Vullaby–Mandibuzz | Shuppet–Banette |
| 2 | 20% | Swablu–Altaria | Duskull–Dusclops |
| 3 | 10% | Meditite–Medicham | Litwick–Lampent |
| 4 | 10% | Golett–Golurk | Shuppet–Banette |
| 5 | 10% | Chimecho | Duskull–Dusclops |
| 6 | 10% | Tropius | Chimecho |
| 7 | 5% | Absol | Litwick–Lampent |
| 8 | 5% | Golett–Golurk | Shuppet–Banette |
| 9 | 4% | Chimecho | Duskull–Dusclops |
| 10 | 4% | Absol | Yamask–Cofagrigus |
| 11 | 1% | Chingling | Chingling |
| 12 | 1% | Rufflet–Braviary | Litwick–Chandelure |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Barboach–Whiscash | Tympole–Seismitoad |
| 2 | 30% | Lotad–Ludicolo | Barboach–Whiscash |
| 3 | 5% | Ducklett–Swanna | Lotad–Ludicolo |
| 4 | 4% | Surskit–Masquerain | Basculin |
| 5 | 1% | Marill–Azumarill | Marill–Azumarill |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Tympole–Seismitoad |
| 2 | 22% | 18% | 10% | Lotad–Ludicolo | Barboach–Whiscash |
| 3 | 10% | 12% | 11% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 4 | 8% | 10% | 10% | Basculin | Basculin |
| 5 | 8% | 9% | 10% | Tympole–Seismitoad | Tympole–Seismitoad |
| 6 | 4% | 7% | 10% | Barboach–Whiscash | Barboach–Whiscash |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Lotad–Ludicolo |
| 8 | 3% | 5% | 9% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 9 | 2% | 4% | 9% | Basculin | Marill–Azumarill |
| 10 | 2% | 4% | 9% | Carvanha–Sharpedo | Carvanha–Sharpedo |

#### Shoal Cave

Dungeon, Far east.

**`MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM`**

Water type: cold water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spheal | Spheal |
| 2 | 20% | Dwebble–Crustle | Woobat |
| 3 | 10% | Spheal | Spheal |
| 4 | 10% | Vanillite | Dwebble–Crustle |
| 5 | 10% | Spheal | Woobat |
| 6 | 10% | Woobat | Vanillite |
| 7 | 5% | Cubchoo | Cubchoo |
| 8 | 5% | Corphish–Crawdaunt | Snorunt |
| 9 | 4% | Snorunt | Corphish–Crawdaunt |
| 10 | 4% | Vanillite | Cubchoo |
| 11 | 1% | Snorunt | Snorunt |
| 12 | 1% | Woobat | Spheal |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spheal | Spheal |
| 2 | 30% | Wailmer–Wailord | Frillish |
| 3 | 5% | Frillish | Wailmer–Wailord |
| 4 | 4% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 5 | 1% | Spheal | Spheal |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Spheal | Spheal |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Frillish |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Frillish | Wailmer–Wailord |
| 6 | 4% | 7% | 10% | Spheal | Frillish |
| 7 | 3% | 6% | 10% | Carvanha–Sharpedo | Spheal |
| 8 | 3% | 5% | 9% | Frillish | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Wailmer–Wailord | Frillish |
| 10 | 2% | 4% | 9% | Spheal | Carvanha–Sharpedo |

**`MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM`**

Water type: cold water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spheal–Sealeo | Spheal–Sealeo |
| 2 | 20% | Vanillite–Vanillish | Cubchoo |
| 3 | 10% | Cubchoo | Woobat–Swoobat |
| 4 | 10% | Spheal–Sealeo | Vanillite–Vanillish |
| 5 | 10% | Dwebble–Crustle | Spheal–Sealeo |
| 6 | 10% | Snorunt | Snorunt |
| 7 | 5% | Woobat–Swoobat | Woobat–Swoobat |
| 8 | 5% | Vanillite–Vanillish | Dwebble–Crustle |
| 9 | 4% | Cubchoo–Beartic | Cubchoo–Beartic |
| 10 | 4% | Snorunt–Glalie | Snorunt–Glalie |
| 11 | 1% | Cryogonal | Cryogonal |
| 12 | 1% | Snorunt | Snorunt–Froslass |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Spheal–Walrein | Spheal–Walrein |
| 2 | 30% | Frillish | Frillish–Jellicent |
| 3 | 5% | Wailmer–Wailord | Frillish |
| 4 | 4% | Frillish–Jellicent | Carvanha–Sharpedo |
| 5 | 1% | Carvanha–Sharpedo | Wailmer–Wailord |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Spheal–Walrein | Frillish–Jellicent |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Spheal–Walrein |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 4 | 8% | 10% | 10% | Spheal–Walrein | Frillish–Jellicent |
| 5 | 8% | 9% | 10% | Wailmer–Wailord | Wailmer–Wailord |
| 6 | 4% | 7% | 10% | Carvanha–Sharpedo | Spheal–Walrein |
| 7 | 3% | 6% | 10% | Spheal–Walrein | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Frillish–Jellicent | Frillish–Jellicent |
| 10 | 2% | 4% | 9% | Wailmer–Wailord | Spheal–Walrein |

**`MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Cubchoo | Snorunt |
| 2 | 20% | Spheal–Sealeo | Cubchoo |
| 3 | 10% | Vanillite–Vanillish | Woobat–Swoobat |
| 4 | 10% | Snorunt | Spheal–Sealeo |
| 5 | 10% | Woobat–Swoobat | Woobat–Swoobat |
| 6 | 10% | Dwebble–Crustle | Vanillite–Vanillish |
| 7 | 5% | Cubchoo–Beartic | Snorunt–Glalie |
| 8 | 5% | Snorunt–Glalie | Cubchoo–Beartic |
| 9 | 4% | Spheal–Walrein | Snorunt–Froslass |
| 10 | 4% | Vanillite–Vanilluxe | Spheal–Walrein |
| 11 | 1% | Cryogonal | Cryogonal |
| 12 | 1% | Snorunt–Froslass | Vanillite–Vanilluxe |

**`MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Vanillite–Vanilluxe | Snorunt |
| 2 | 20% | Snorunt | Vanillite–Vanilluxe |
| 3 | 10% | Spheal–Walrein | Woobat–Swoobat |
| 4 | 10% | Cubchoo–Beartic | Spheal–Walrein |
| 5 | 10% | Woobat–Swoobat | Woobat–Swoobat |
| 6 | 10% | Snorunt–Glalie | Snorunt–Glalie |
| 7 | 5% | Dwebble–Crustle | Snorunt–Froslass |
| 8 | 5% | Cryogonal | Cryogonal |
| 9 | 4% | Snorunt–Froslass | Cubchoo–Beartic |
| 10 | 4% | Vanillite–Vanilluxe | Dwebble–Crustle |
| 11 | 1% | Cryogonal | Cryogonal |
| 12 | 1% | Spheal–Walrein | Snorunt–Froslass |

**`MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Snorunt–Glalie | Snorunt–Froslass |
| 2 | 20% | Snorunt | Snorunt–Glalie |
| 3 | 10% | Vanillite–Vanilluxe | Snorunt |
| 4 | 10% | Cryogonal | Cryogonal |
| 5 | 10% | Cubchoo–Beartic | Vanillite–Vanilluxe |
| 6 | 10% | Spheal–Walrein | Cubchoo–Beartic |
| 7 | 5% | Snorunt–Froslass | Woobat–Swoobat |
| 8 | 5% | Cryogonal | Cryogonal |
| 9 | 4% | Snorunt–Glalie | Spheal–Walrein |
| 10 | 4% | Vanillite–Vanilluxe | Snorunt–Froslass |
| 11 | 1% | Snorunt–Froslass | Cryogonal |
| 12 | 1% | Cryogonal | Cubchoo–Beartic |

#### Seafloor Cavern

Dungeon, Far east.

**`MAP_SEAFLOOR_CAVERN_ENTRANCE`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 2 | 20% | Dwebble–Crustle | Dwebble–Crustle |
| 3 | 10% | Sableye | Poochyena–Mightyena |
| 4 | 10% | Woobat | Sableye |
| 5 | 10% | Roggenrola–Boldore | Woobat |
| 6 | 10% | Dwebble–Crustle | Roggenrola–Boldore |
| 7 | 5% | Mawile | Mawile |
| 8 | 5% | Corphish–Crawdaunt | Stunfisk |
| 9 | 4% | Stunfisk | Poochyena–Mightyena |
| 10 | 4% | Woobat | Woobat |
| 11 | 1% | Oshawott–Samurott | Oshawott–Samurott |
| 12 | 1% | Sableye | Sableye |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Wailmer–Wailord | Wailmer–Wailord |
| 2 | 30% | Frillish | Tynamo–Eelektrik |
| 3 | 5% | Tynamo–Eelektrik | Frillish |
| 4 | 4% | Corphish–Crawdaunt | Carvanha–Sharpedo |
| 5 | 1% | Alomomola | Alomomola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Wailmer–Wailord | Tynamo–Eelektrik |
| 3 | 10% | 12% | 11% | Frillish | Frillish |
| 4 | 8% | 10% | 10% | Corphish–Crawdaunt | Tynamo–Eelektrik |
| 5 | 8% | 9% | 10% | Tynamo–Eelektrik | Wailmer–Wailord |
| 6 | 4% | 7% | 10% | Carvanha–Sharpedo | Corphish–Crawdaunt |
| 7 | 3% | 6% | 10% | Luvdisc | Carvanha–Sharpedo |
| 8 | 3% | 5% | 9% | Frillish | Frillish |
| 9 | 2% | 4% | 9% | Alomomola | Alomomola |
| 10 | 2% | 4% | 9% | Relicanth | Relicanth |

**`MAP_SEAFLOOR_CAVERN_ROOM1`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 2 | 20% | Dwebble–Crustle | Dwebble–Crustle |
| 3 | 10% | Corphish–Crawdaunt | Sableye |
| 4 | 10% | Dwebble–Crustle | Poochyena–Mightyena |
| 5 | 10% | Woobat | Woobat |
| 6 | 10% | Corphish–Crawdaunt | Dwebble–Crustle |
| 7 | 5% | Mawile | Poochyena–Mightyena |
| 8 | 5% | Roggenrola–Boldore | Mawile |
| 9 | 4% | Stunfisk | Roggenrola–Boldore |
| 10 | 4% | Drilbur–Excadrill | Woobat–Swoobat |
| 11 | 1% | Mudkip–Swampert | Mudkip–Swampert |
| 12 | 1% | Sableye | Drilbur–Excadrill |

**`MAP_SEAFLOOR_CAVERN_ROOM2`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Roggenrola–Boldore | Roggenrola–Boldore |
| 2 | 20% | Dwebble–Crustle | Dwebble–Crustle |
| 3 | 10% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 4 | 10% | Aron–Aggron | Poochyena–Mightyena |
| 5 | 10% | Drilbur–Excadrill | Aron–Aggron |
| 6 | 10% | Roggenrola–Boldore | Woobat–Swoobat |
| 7 | 5% | Stunfisk | Sableye |
| 8 | 5% | Sableye | Drilbur–Excadrill |
| 9 | 4% | Roggenrola–Gigalith | Roggenrola–Gigalith |
| 10 | 4% | Durant | Durant |
| 11 | 1% | Oshawott–Samurott | Oshawott–Samurott |
| 12 | 1% | Corphish–Crawdaunt | Poochyena–Mightyena |

**`MAP_SEAFLOOR_CAVERN_ROOM3`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Aron–Aggron | Aron–Aggron |
| 2 | 20% | Mawile | Mawile |
| 3 | 10% | Roggenrola–Gigalith | Woobat–Swoobat |
| 4 | 10% | Drilbur–Excadrill | Roggenrola–Gigalith |
| 5 | 10% | Sableye | Poochyena–Mightyena |
| 6 | 10% | Dwebble–Crustle | Sableye |
| 7 | 5% | Woobat–Swoobat | Drilbur–Excadrill |
| 8 | 5% | Durant | Dwebble–Crustle |
| 9 | 4% | Durant | Durant |
| 10 | 4% | Roggenrola–Gigalith | Durant |
| 11 | 1% | Durant | Poochyena–Mightyena |
| 12 | 1% | Aron–Aggron | Durant |

**`MAP_SEAFLOOR_CAVERN_ROOM4`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Corphish–Crawdaunt | Dwebble–Crustle |
| 2 | 20% | Dwebble–Crustle | Corphish–Crawdaunt |
| 3 | 10% | Sableye | Woobat–Swoobat |
| 4 | 10% | Woobat–Swoobat | Sableye |
| 5 | 10% | Roggenrola–Boldore | Poochyena–Mightyena |
| 6 | 10% | Dwebble–Crustle | Roggenrola–Boldore |
| 7 | 5% | Stunfisk | Sableye |
| 8 | 5% | Mawile | Stunfisk |
| 9 | 4% | Drilbur–Excadrill | Mawile |
| 10 | 4% | Corphish–Crawdaunt | Drilbur–Excadrill |
| 11 | 1% | Mudkip–Swampert | Mudkip–Swampert |
| 12 | 1% | Deino–Hydreigon | Deino–Hydreigon |

**`MAP_SEAFLOOR_CAVERN_ROOM5`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Drilbur–Excadrill | Drilbur–Excadrill |
| 2 | 20% | Roggenrola–Gigalith | Roggenrola–Gigalith |
| 3 | 10% | Aron–Aggron | Woobat–Swoobat |
| 4 | 10% | Durant | Aron–Aggron |
| 5 | 10% | Sableye | Durant |
| 6 | 10% | Drilbur–Excadrill | Poochyena–Mightyena |
| 7 | 5% | Mawile | Sableye |
| 8 | 5% | Roggenrola–Gigalith | Mawile |
| 9 | 4% | Dwebble–Crustle | Roggenrola–Gigalith |
| 10 | 4% | Durant | Dwebble–Crustle |
| 11 | 1% | Druddigon | Druddigon |
| 12 | 1% | Durant | Poochyena–Mightyena |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Corphish–Crawdaunt | Frillish–Jellicent |
| 2 | 30% | Frillish–Jellicent | Corphish–Crawdaunt |
| 3 | 5% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 4% | Tynamo–Eelektrik | Carvanha–Sharpedo |
| 5 | 1% | Alomomola | Tynamo–Eelektross |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corphish–Crawdaunt | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Carvanha–Sharpedo | Frillish–Jellicent |
| 3 | 10% | 12% | 11% | Frillish–Jellicent | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Wailmer–Wailord | Corphish–Crawdaunt |
| 5 | 8% | 9% | 10% | Tynamo–Eelektrik | Tynamo–Eelektross |
| 6 | 4% | 7% | 10% | Corphish–Crawdaunt | Frillish–Jellicent |
| 7 | 3% | 6% | 10% | Luvdisc | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Carvanha–Sharpedo | Luvdisc |
| 9 | 2% | 4% | 9% | Alomomola | Alomomola |
| 10 | 2% | 4% | 9% | Relicanth | Relicanth |

**`MAP_SEAFLOOR_CAVERN_ROOM6`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dwebble–Crustle | Corphish–Crawdaunt |
| 2 | 20% | Sableye | Dwebble–Crustle |
| 3 | 10% | Corphish–Crawdaunt | Woobat–Swoobat |
| 4 | 10% | Sableye | Sableye |
| 5 | 10% | Dwebble–Crustle | Corphish–Crawdaunt |
| 6 | 10% | Mawile | Poochyena–Mightyena |
| 7 | 5% | Roggenrola–Boldore | Mawile |
| 8 | 5% | Aron–Aggron | Roggenrola–Boldore |
| 9 | 4% | Dwebble–Crustle | Aron–Aggron |
| 10 | 4% | Drilbur–Excadrill | Drilbur–Excadrill |
| 11 | 1% | Oshawott–Samurott | Oshawott–Samurott |
| 12 | 1% | Oshawott–Samurott | Oshawott–Samurott |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Frillish | Frillish |
| 2 | 30% | Wailmer–Wailord | Tynamo–Eelektrik |
| 3 | 5% | Corphish–Crawdaunt | Wailmer–Wailord |
| 4 | 4% | Tynamo–Eelektrik | Carvanha–Sharpedo |
| 5 | 1% | Alomomola | Tynamo–Eelektross |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Corphish–Crawdaunt | Frillish–Jellicent |
| 2 | 22% | 18% | 10% | Frillish–Jellicent | Corphish–Crawdaunt |
| 3 | 10% | 12% | 11% | Wailmer–Wailord | Tynamo–Eelektrik |
| 4 | 8% | 10% | 10% | Carvanha–Sharpedo | Carvanha–Sharpedo |
| 5 | 8% | 9% | 10% | Tynamo–Eelektrik | Frillish–Jellicent |
| 6 | 4% | 7% | 10% | Corphish–Crawdaunt | Tynamo–Eelektross |
| 7 | 3% | 6% | 10% | Luvdisc | Corphish–Crawdaunt |
| 8 | 3% | 5% | 9% | Frillish–Jellicent | Wailmer–Wailord |
| 9 | 2% | 4% | 9% | Alomomola | Alomomola |
| 10 | 2% | 4% | 9% | Relicanth | Relicanth |

**`MAP_SEAFLOOR_CAVERN_ROOM7`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sableye | Sableye |
| 2 | 20% | Roggenrola–Gigalith | Roggenrola–Gigalith |
| 3 | 10% | Mawile | Woobat–Swoobat |
| 4 | 10% | Aron–Aggron | Mawile |
| 5 | 10% | Roggenrola–Gigalith | Poochyena–Mightyena |
| 6 | 10% | Dwebble–Crustle | Aron–Aggron |
| 7 | 5% | Drilbur–Excadrill | Roggenrola–Gigalith |
| 8 | 5% | Woobat–Swoobat | Dwebble–Crustle |
| 9 | 4% | Deino–Hydreigon | Deino–Hydreigon |
| 10 | 4% | Durant | Drilbur–Excadrill |
| 11 | 1% | Druddigon | Druddigon |
| 12 | 1% | Deino–Hydreigon | Deino–Hydreigon |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 2 | 30% | Wailmer–Wailord | Frillish–Jellicent |
| 3 | 5% | Frillish–Jellicent | Tynamo–Eelektross |
| 4 | 4% | Tynamo–Eelektross | Carvanha–Sharpedo |
| 5 | 1% | Alomomola | Alomomola |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Wailmer–Wailord | Carvanha–Sharpedo |
| 2 | 22% | 18% | 10% | Corphish–Crawdaunt | Frillish–Jellicent |
| 3 | 10% | 12% | 11% | Carvanha–Sharpedo | Tynamo–Eelektross |
| 4 | 8% | 10% | 10% | Frillish–Jellicent | Corphish–Crawdaunt |
| 5 | 8% | 9% | 10% | Luvdisc | Frillish–Jellicent |
| 6 | 4% | 7% | 10% | Tynamo–Eelektross | Tynamo–Eelektross |
| 7 | 3% | 6% | 10% | Corphish–Crawdaunt | Wailmer–Wailord |
| 8 | 3% | 5% | 9% | Alomomola | Alomomola |
| 9 | 2% | 4% | 9% | Relicanth | Relicanth |
| 10 | 2% | 4% | 9% | Wailmer–Wailord | Luvdisc |

**`MAP_SEAFLOOR_CAVERN_ROOM8`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Mawile | Mawile |
| 2 | 20% | Drilbur–Excadrill | Drilbur–Excadrill |
| 3 | 10% | Roggenrola–Boldore | Sableye |
| 4 | 10% | Sableye | Roggenrola–Boldore |
| 5 | 10% | Roggenrola–Gigalith | Woobat–Swoobat |
| 6 | 10% | Aron–Aggron | Poochyena–Mightyena |
| 7 | 5% | Woobat–Swoobat | Roggenrola–Gigalith |
| 8 | 5% | Drilbur–Excadrill | Aron–Aggron |
| 9 | 4% | Deino–Hydreigon | Deino–Hydreigon |
| 10 | 4% | Druddigon | Druddigon |
| 11 | 1% | Deino–Hydreigon | Deino–Hydreigon |
| 12 | 1% | Durant | Drilbur–Excadrill |

#### Artisan Cave

Dungeon, Far east.

**`MAP_ARTISAN_CAVE_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Drilbur–Excadrill | Klink–Klang |
| 2 | 20% | Klink–Klang | Drilbur–Excadrill |
| 3 | 10% | Roggenrola–Boldore | Baltoy–Claydol |
| 4 | 10% | Baltoy–Claydol | Roggenrola–Boldore |
| 5 | 10% | Nosepass | Sableye |
| 6 | 10% | Drilbur–Excadrill | Nosepass |
| 7 | 5% | Aron–Aggron | Drilbur–Excadrill |
| 8 | 5% | Durant | Woobat |
| 9 | 4% | Mawile | Durant |
| 10 | 4% | Nosepass | Nosepass |
| 11 | 1% | Druddigon | Druddigon |
| 12 | 1% | Klink–Klinklang | Mawile |

**`MAP_ARTISAN_CAVE_B1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Drilbur–Excadrill | Klink–Klinklang |
| 2 | 20% | Klink–Klinklang | Drilbur–Excadrill |
| 3 | 10% | Baltoy–Claydol | Sableye |
| 4 | 10% | Roggenrola–Gigalith | Baltoy–Claydol |
| 5 | 10% | Nosepass–Probopass | Roggenrola–Gigalith |
| 6 | 10% | Drilbur–Excadrill | Drilbur–Excadrill |
| 7 | 5% | Aron–Aggron | Nosepass–Probopass |
| 8 | 5% | Durant | Woobat–Swoobat |
| 9 | 4% | Nosepass | Durant |
| 10 | 4% | Klink–Klinklang | Beldum–Metagross |
| 11 | 1% | Beldum–Metagross | Druddigon |
| 12 | 1% | Druddigon | Elgyem–Beheeyem |

#### Sky Pillar

Dungeon, Far east.

**`MAP_SKY_PILLAR_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Baltoy–Claydol | Sableye |
| 2 | 20% | Sableye | Baltoy–Claydol |
| 3 | 10% | Golett–Golurk | Shuppet–Banette |
| 4 | 10% | Shuppet–Banette | Golett–Golurk |
| 5 | 10% | Woobat | Litwick–Lampent |
| 6 | 10% | Sigilyph | Woobat |
| 7 | 5% | Elgyem–Beheeyem | Sigilyph |
| 8 | 5% | Litwick–Lampent | Elgyem–Beheeyem |
| 9 | 4% | Swablu–Altaria | Duskull–Dusclops |
| 10 | 4% | Sigilyph | Swablu–Altaria |
| 11 | 1% | Druddigon | Druddigon |
| 12 | 1% | Rufflet–Braviary | Rufflet–Braviary |

**`MAP_SKY_PILLAR_3F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sigilyph | Sigilyph |
| 2 | 20% | Baltoy–Claydol | Baltoy–Claydol |
| 3 | 10% | Golett–Golurk | Shuppet–Banette |
| 4 | 10% | Sableye | Sableye |
| 5 | 10% | Shuppet–Banette | Litwick–Lampent |
| 6 | 10% | Swablu–Altaria | Golett–Golurk |
| 7 | 5% | Woobat–Swoobat | Duskull–Dusclops |
| 8 | 5% | Elgyem–Beheeyem | Woobat–Swoobat |
| 9 | 4% | Rufflet–Braviary | Vullaby–Mandibuzz |
| 10 | 4% | Druddigon | Druddigon |
| 11 | 1% | Beldum–Metagross | Beldum–Metagross |
| 12 | 1% | Litwick–Chandelure | Litwick–Chandelure |

**`MAP_SKY_PILLAR_5F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Swablu–Altaria | Sigilyph |
| 2 | 20% | Sigilyph | Swablu–Altaria |
| 3 | 10% | Baltoy–Claydol | Shuppet–Banette |
| 4 | 10% | Golett–Golurk | Baltoy–Claydol |
| 5 | 10% | Shuppet–Banette | Duskull–Dusknoir |
| 6 | 10% | Sableye | Sableye |
| 7 | 5% | Rufflet–Braviary | Litwick–Chandelure |
| 8 | 5% | Beldum–Metagross | Vullaby–Mandibuzz |
| 9 | 4% | Bagon–Salamence | Beldum–Metagross |
| 10 | 4% | Druddigon | Druddigon |
| 11 | 1% | Beldum–Metagross | Bagon–Salamence |
| 12 | 1% | Deino–Hydreigon | Deino–Hydreigon |

#### Cave of Origin

Dungeon, Far east.

**`MAP_CAVE_OF_ORIGIN_ENTRANCE`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Woobat | Woobat |
| 2 | 20% | Mawile | Mawile |
| 3 | 10% | Sableye | Sableye |
| 4 | 10% | Roggenrola–Boldore | Duskull–Dusclops |
| 5 | 10% | Golett–Golurk | Shuppet–Banette |
| 6 | 10% | Shuppet–Banette | Roggenrola–Boldore |
| 7 | 5% | Drilbur–Excadrill | Golett–Golurk |
| 8 | 5% | Duskull–Dusclops | Drilbur–Excadrill |
| 9 | 4% | Duskull–Dusclops | Duskull–Dusclops |
| 10 | 4% | Durant | Duskull–Dusclops |
| 11 | 1% | Golett–Golurk | Durant |
| 12 | 1% | Durant | Durant |

**`MAP_CAVE_OF_ORIGIN_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sableye | Sableye |
| 2 | 20% | Woobat–Swoobat | Woobat–Swoobat |
| 3 | 10% | Roggenrola–Gigalith | Duskull–Dusclops |
| 4 | 10% | Mawile | Litwick–Lampent |
| 5 | 10% | Golett–Golurk | Golett–Golurk |
| 6 | 10% | Shuppet–Banette | Mawile |
| 7 | 5% | Duskull–Dusclops | Shuppet–Banette |
| 8 | 5% | Drilbur–Excadrill | Roggenrola–Gigalith |
| 9 | 4% | Duskull–Dusclops | Duskull–Dusclops |
| 10 | 4% | Litwick–Lampent | Drilbur–Excadrill |
| 11 | 1% | Litwick–Chandelure | Litwick–Chandelure |
| 12 | 1% | Durant | Durant |

**`MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP1`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Mawile | Mawile |
| 2 | 20% | Roggenrola–Gigalith | Roggenrola–Gigalith |
| 3 | 10% | Woobat–Swoobat | Litwick–Lampent |
| 4 | 10% | Shuppet–Banette | Woobat–Swoobat |
| 5 | 10% | Sableye | Shuppet–Banette |
| 6 | 10% | Golett–Golurk | Sableye |
| 7 | 5% | Duskull–Dusclops | Golett–Golurk |
| 8 | 5% | Duskull–Dusclops | Duskull–Dusclops |
| 9 | 4% | Drilbur–Excadrill | Duskull–Dusknoir |
| 10 | 4% | Durant | Drilbur–Excadrill |
| 11 | 1% | Deino–Hydreigon | Deino–Hydreigon |
| 12 | 1% | Duskull–Dusknoir | Durant |

**`MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP2`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Litwick–Lampent | Litwick–Lampent |
| 2 | 20% | Sableye | Litwick–Lampent |
| 3 | 10% | Roggenrola–Gigalith | Sableye |
| 4 | 10% | Litwick–Lampent | Duskull–Dusclops |
| 5 | 10% | Mawile | Roggenrola–Gigalith |
| 6 | 10% | Duskull–Dusclops | Shuppet–Banette |
| 7 | 5% | Woobat–Swoobat | Litwick–Chandelure |
| 8 | 5% | Litwick–Chandelure | Drilbur–Excadrill |
| 9 | 4% | Golett–Golurk | Mawile |
| 10 | 4% | Deino–Hydreigon | Duskull–Dusknoir |
| 11 | 1% | Duskull–Dusknoir | Deino–Hydreigon |
| 12 | 1% | Deino–Hydreigon | Deino–Hydreigon |

**`MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP3`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Roggenrola–Gigalith | Litwick–Chandelure |
| 2 | 20% | Litwick–Lampent | Litwick–Lampent |
| 3 | 10% | Sableye | Duskull–Dusknoir |
| 4 | 10% | Litwick–Chandelure | Sableye |
| 5 | 10% | Mawile | Roggenrola–Gigalith |
| 6 | 10% | Duskull–Dusknoir | Deino–Hydreigon |
| 7 | 5% | Deino–Hydreigon | Shuppet–Banette |
| 8 | 5% | Golett–Golurk | Golett–Golurk |
| 9 | 4% | Drilbur–Excadrill | Mawile |
| 10 | 4% | Woobat–Swoobat | Woobat–Swoobat |
| 11 | 1% | Deino–Hydreigon | Deino–Hydreigon |
| 12 | 1% | Duskull–Dusknoir | Shuppet–Banette |

#### Victory Road

Dungeon, Far east.

**`MAP_VICTORY_ROAD_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Makuhita–Hariyama | Aron–Aggron |
| 2 | 20% | Aron–Aggron | Woobat |
| 3 | 10% | Meditite–Medicham | Makuhita–Hariyama |
| 4 | 10% | Whismur–Exploud | Meditite–Medicham |
| 5 | 10% | Woobat | Whismur–Exploud |
| 6 | 10% | Roggenrola–Boldore | Roggenrola–Boldore |
| 7 | 5% | Timburr–Gurdurr | Sableye |
| 8 | 5% | Mienfoo–Mienshao | Timburr–Gurdurr |
| 9 | 4% | Drilbur–Excadrill | Mienfoo–Mienshao |
| 10 | 4% | Throh | Drilbur–Excadrill |
| 11 | 1% | Sawk | Sawk |
| 12 | 1% | Druddigon | Throh |

**`MAP_VICTORY_ROAD_B1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Aron–Aggron | Meditite–Medicham |
| 2 | 20% | Meditite–Medicham | Aron–Aggron |
| 3 | 10% | Makuhita–Hariyama | Woobat–Swoobat |
| 4 | 10% | Mawile | Mawile |
| 5 | 10% | Roggenrola–Boldore | Makuhita–Hariyama |
| 6 | 10% | Timburr–Conkeldurr | Sableye |
| 7 | 5% | Woobat–Swoobat | Timburr–Conkeldurr |
| 8 | 5% | Mienfoo–Mienshao | Mienfoo–Mienshao |
| 9 | 4% | Throh | Sawk |
| 10 | 4% | Sawk | Throh |
| 11 | 1% | Axew–Haxorus | Axew–Haxorus |
| 12 | 1% | Beldum–Metagross | Beldum–Metagross |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Roggenrola–Gigalith | Roggenrola–Gigalith |
| 2 | 30% | Dwebble–Crustle | Dwebble–Crustle |
| 3 | 5% | Nosepass–Probopass | Sableye |
| 4 | 4% | Drilbur–Excadrill | Nosepass–Probopass |
| 5 | 1% | Durant | Durant |

**`MAP_VICTORY_ROAD_B2F`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sableye | Mawile |
| 2 | 20% | Mawile | Sableye |
| 3 | 10% | Aron–Aggron | Woobat–Swoobat |
| 4 | 10% | Makuhita–Hariyama | Aron–Aggron |
| 5 | 10% | Meditite–Medicham | Makuhita–Hariyama |
| 6 | 10% | Woobat–Swoobat | Woobat–Swoobat |
| 7 | 5% | Timburr–Conkeldurr | Meditite–Medicham |
| 8 | 5% | Axew–Haxorus | Axew–Haxorus |
| 9 | 4% | Druddigon | Druddigon |
| 10 | 4% | Deino–Hydreigon | Deino–Hydreigon |
| 11 | 1% | Mienfoo–Mienshao | Timburr–Conkeldurr |
| 12 | 1% | Axew–Haxorus | Deino–Hydreigon |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Barboach–Whiscash | Tympole–Seismitoad |
| 2 | 30% | Tympole–Seismitoad | Barboach–Whiscash |
| 3 | 5% | Basculin | Basculin |
| 4 | 4% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 5 | 1% | Marill–Azumarill | Tympole–Seismitoad |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Barboach–Whiscash | Tympole–Seismitoad |
| 2 | 22% | 18% | 10% | Basculin | Barboach–Whiscash |
| 3 | 10% | 12% | 11% | Tympole–Seismitoad | Tympole–Seismitoad |
| 4 | 8% | 10% | 10% | Barboach–Whiscash | Basculin |
| 5 | 8% | 9% | 10% | Corphish–Crawdaunt | Corphish–Crawdaunt |
| 6 | 4% | 7% | 10% | Basculin | Tympole–Seismitoad |
| 7 | 3% | 6% | 10% | Marill–Azumarill | Barboach–Whiscash |
| 8 | 3% | 5% | 9% | Tympole–Seismitoad | Marill–Azumarill |
| 9 | 2% | 4% | 9% | Barboach–Whiscash | Stunfisk |
| 10 | 2% | 4% | 9% | Stunfisk | Basculin |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Absol | Mt. Pyre, Route 120 |
| Aggron | Artisan Cave, Granite Cave, Meteor Falls, Rusturf Tunnel and more |
| Alomomola | Abandoned Ship, Route 107, Route 108, Route 125 and more |
| Altaria | Meteor Falls, Mt. Pyre, Route 114, Route 115 and more |
| Amoonguss | Petalburg Woods, Route 120 |
| Aron | Altering Cave, Artisan Cave, Granite Cave, Meteor Falls and more |
| Audino | Route 110, Route 115, Route 116, Route 117 and more |
| Axew | Meteor Falls, Victory Road |
| Azumarill | Meteor Falls, Mt. Pyre, Petalburg City, Route 102 and more |
| Azurill | Petalburg City, Route 102, Route 104, Route 111 and more |
| Bagon | Meteor Falls, Sky Pillar |
| Baltoy | Artisan Cave, Mirage Tower, Route 111, Sky Pillar |
| Banette | Cave of Origin, Mt. Pyre, Route 121, Route 123 and more |
| Barboach | Meteor Falls, Mt. Pyre, Petalburg City, Route 102 and more |
| Basculin | Meteor Falls, Mt. Pyre, Petalburg City, Route 102 and more |
| Beartic | Shoal Cave |
| Beheeyem | Artisan Cave, Meteor Falls, Route 114, Route 130 and more |
| Beldum | Artisan Cave, Granite Cave, Meteor Falls, Sky Pillar and more |
| Bisharp | Route 113 |
| Blaziken | Fiery Path, Magma Hideout, Route 112 |
| Blitzle | Route 110, Route 118 |
| Boldore | Altering Cave, Artisan Cave, Cave of Origin, Fiery Path and more |
| Bouffalant | Route 120 |
| Braviary | Mt. Pyre, Route 112, Route 113, Sky Pillar |
| Breloom | Petalburg Woods |
| Budew | Petalburg Woods, Route 117 |
| Cacnea | Desert Underpass, Mirage Tower, Route 111, Route 113 |
| Cacturne | Desert Underpass, Mirage Tower, Route 111, Route 113 |
| Camerupt | Fiery Path, Jagged Pass, Magma Hideout, Route 112 and more |
| Carvanha | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Cascoon | Petalburg Woods |
| Castform | Route 119, Route 130 |
| Chandelure | Cave of Origin, Mt. Pyre, Route 130, Sky Pillar |
| Chimecho | Mt. Pyre |
| Chingling | Altering Cave, Mt. Pyre |
| Clamperl | Underwater Route 124, Underwater Route 126 |
| Claydol | Artisan Cave, Mirage Tower, Route 111, Sky Pillar |
| Cofagrigus | Desert Underpass, Mirage Tower, Mt. Pyre, Route 111 and more |
| Combusken | Fiery Path, Magma Hideout, Route 112 |
| Conkeldurr | Magma Hideout, Victory Road |
| Corphish | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Cottonee | Route 104, Route 113, Route 115, Route 117 and more |
| Crawdaunt | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Crustle | Desert Underpass, Granite Cave, Mirage Tower, Route 111 and more |
| Cryogonal | Shoal Cave |
| Cubchoo | Shoal Cave |
| Darmanitan | Magma Hideout, Mirage Tower, Route 111 |
| Darumaka | Fiery Path, Magma Hideout, Mirage Tower, Route 111 |
| Deerling | Route 114, Route 118, Route 119, Route 120 and more |
| Deerling Spring | Route 121 |
| Deino | Cave of Origin, Seafloor Cavern, Sky Pillar, Victory Road |
| Dewott | Route 103, Route 109, Seafloor Cavern, Slateport City |
| Drilbur | Artisan Cave, Cave of Origin, Desert Underpass, Granite Cave and more |
| Druddigon | Artisan Cave, Meteor Falls, Seafloor Cavern, Sky Pillar and more |
| Ducklett | Ever Grande City, Lilycove City, Mossdeep City, Mt. Pyre and more |
| Duosion | Route 116, Route 123, Route 130 |
| Durant | Artisan Cave, Cave of Origin, Desert Underpass, Magma Hideout and more |
| Dusclops | Cave of Origin, Mt. Pyre, Route 113, Route 121 and more |
| Dusknoir | Cave of Origin, Mt. Pyre, Sky Pillar |
| Duskull | Cave of Origin, Mt. Pyre, Route 113, Route 121 and more |
| Dwebble | Desert Underpass, Granite Cave, Mirage Tower, Route 111 and more |
| Eelektrik | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Eelektross | Abandoned Ship, New Mauville, Route 126, Route 129 and more |
| Electrike | New Mauville, Route 110, Route 118 |
| Elgyem | Artisan Cave, Meteor Falls, Route 114, Route 130 and more |
| Emboar | Jagged Pass, Magma Hideout, Route 111 |
| Emolga | New Mauville, Route 110, Route 119 |
| Excadrill | Artisan Cave, Cave of Origin, Desert Underpass, Granite Cave and more |
| Exploud | Altering Cave, Desert Underpass, Granite Cave, Route 116 and more |
| Feebas | Route 119 |
| Ferroseed | New Mauville |
| Ferrothorn | New Mauville |
| Flygon | Desert Underpass, Mirage Tower, Route 111, Route 113 |
| Foongus | Petalburg Woods, Route 120 |
| Fraxure | Meteor Falls, Victory Road |
| Frillish | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Froslass | Shoal Cave |
| Galvantula | New Mauville, Route 119 |
| Garbodor | New Mauville, Route 110 |
| Gardevoir | Route 102 |
| Gigalith | Artisan Cave, Cave of Origin, Granite Cave, Magma Hideout and more |
| Glalie | Shoal Cave |
| Golett | Cave of Origin, Mirage Tower, Mt. Pyre, Sky Pillar |
| Golurk | Cave of Origin, Mirage Tower, Mt. Pyre, Sky Pillar |
| Gorebyss | Underwater Route 124, Underwater Route 126 |
| Gothita | Route 121, Route 130 |
| Gothitelle | Route 121, Route 130 |
| Gothorita | Route 121, Route 130 |
| Grovyle | Petalburg Woods, Route 101, Route 120 |
| Grumpig | Fiery Path, Jagged Pass, Route 112 |
| Gulpin | Fiery Path, Magma Hideout, Route 110 |
| Gurdurr | Fiery Path, Jagged Pass, Magma Hideout, Route 112 and more |
| Hariyama | Altering Cave, Granite Cave, Rusturf Tunnel, Victory Road |
| Haxorus | Meteor Falls, Victory Road |
| Heatmor | Fiery Path, Magma Hideout |
| Herdier | Route 101, Route 115, Route 116, Route 117 and more |
| Huntail | Underwater Route 124, Underwater Route 126 |
| Hydreigon | Cave of Origin, Seafloor Cavern, Sky Pillar, Victory Road |
| Illumise | Route 102, Route 104, Route 110, Route 114 and more |
| Jellicent | Abandoned Ship, Ever Grande City, Route 118, Route 122 and more |
| Joltik | New Mauville, Route 119 |
| Karrablast | Route 119, Route 120 |
| Kecleon | Route 118, Route 119, Route 120, Route 121 and more |
| Kirlia | Route 102 |
| Klang | Artisan Cave, New Mauville |
| Klink | Artisan Cave, New Mauville |
| Klinklang | Artisan Cave |
| Krokorok | Desert Underpass, Mirage Tower, Route 111, Route 113 |
| Krookodile | Desert Underpass, Mirage Tower, Route 111, Route 113 |
| Lairon | Altering Cave, Artisan Cave, Granite Cave, Meteor Falls and more |
| Lampent | Cave of Origin, Mt. Pyre, Route 121, Route 123 and more |
| Larvesta | Magma Hideout |
| Liepard | Jagged Pass, Route 101, Route 102, Route 103 and more |
| Lillipup | Route 101, Route 115, Route 116, Route 117 and more |
| Linoone | Route 101, Route 102, Route 103, Route 104 and more |
| Litwick | Cave of Origin, Mt. Pyre, Route 121, Route 123 and more |
| Lombre | Mt. Pyre, Petalburg City, Route 102, Route 111 and more |
| Lotad | Mt. Pyre, Petalburg City, Route 102, Route 111 and more |
| Loudred | Altering Cave, Desert Underpass, Granite Cave, Route 116 and more |
| Ludicolo | Mt. Pyre |
| Lunatone | Meteor Falls |
| Luvdisc | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Makuhita | Altering Cave, Granite Cave, Rusturf Tunnel, Victory Road |
| Mandibuzz | Mirage Tower, Mt. Pyre, Route 111, Route 113 and more |
| Manectric | New Mauville, Route 110, Route 118 |
| Maractus | Mirage Tower |
| Marill | Meteor Falls, Mt. Pyre, Petalburg City, Route 102 and more |
| Marshtomp | Route 102, Route 104, Route 115, Route 119 and more |
| Masquerain | Mt. Pyre, Petalburg City, Route 102, Route 111 and more |
| Mawile | Altering Cave, Artisan Cave, Cave of Origin, Granite Cave and more |
| Medicham | Jagged Pass, Meteor Falls, Mt. Pyre, Victory Road |
| Meditite | Jagged Pass, Meteor Falls, Mt. Pyre, Victory Road |
| Metagross | Artisan Cave, Granite Cave, Meteor Falls, Sky Pillar and more |
| Metang | Artisan Cave, Granite Cave, Meteor Falls, Sky Pillar and more |
| Mienfoo | Jagged Pass, Route 112, Victory Road |
| Mienshao | Jagged Pass, Route 112, Victory Road |
| Mightyena | Jagged Pass, Magma Hideout, Petalburg Woods, Route 101 and more |
| Minccino | Route 115, Route 116, Route 121, Route 123 |
| Minun | New Mauville, Route 110 |
| Mudkip | Route 102, Route 104, Route 115, Route 119 and more |
| Munna | Altering Cave, Route 101, Route 102, Route 103 and more |
| Musharna | Route 130 |
| Nincada | Route 116 |
| Ninjask | Route 116 |
| Nosepass | Altering Cave, Artisan Cave, Desert Underpass, Granite Cave and more |
| Numel | Fiery Path, Jagged Pass, Magma Hideout, Route 112 and more |
| Nuzleaf | Petalburg Woods, Route 101, Route 102, Route 103 and more |
| Oshawott | Route 103, Route 109, Seafloor Cavern, Slateport City |
| Palpitoad | Meteor Falls, Mt. Pyre, Petalburg City, Route 102 and more |
| Panpour | Route 114 |
| Pansage | Route 102, Route 119 |
| Pansear | Fiery Path, Jagged Pass, Magma Hideout, Route 112 |
| Patrat | Route 101, Route 103, Route 110, Route 116 and more |
| Pawniard | Route 113 |
| Pelipper | Dewford Town, Ever Grande City, Lilycove City, Mossdeep City and more |
| Petilil | Route 117, Route 119, Route 120, Route 121 and more |
| Pidove | Route 104, Route 115, Route 117, Route 118 and more |
| Pignite | Jagged Pass, Magma Hideout, Route 111 |
| Plusle | New Mauville, Route 110 |
| Poochyena | Jagged Pass, Magma Hideout, Petalburg Woods, Route 101 and more |
| Probopass | Altering Cave, Artisan Cave, Granite Cave, Victory Road |
| Purrloin | Jagged Pass, Route 101, Route 102, Route 103 and more |
| Ralts | Route 102 |
| Relicanth | Route 126, Route 130, Route 131, Route 133 and more |
| Reuniclus | Route 116, Route 123, Route 130 |
| Roggenrola | Altering Cave, Artisan Cave, Cave of Origin, Fiery Path and more |
| Roselia | Route 117, Route 123 |
| Rufflet | Mt. Pyre, Route 112, Route 113, Sky Pillar |
| Sableye | Altering Cave, Artisan Cave, Cave of Origin, Granite Cave and more |
| Salamence | Meteor Falls, Sky Pillar |
| Samurott | Route 103, Route 109, Seafloor Cavern, Slateport City |
| Sandile | Desert Underpass, Mirage Tower, Route 111, Route 113 |
| Sawk | Granite Cave, Jagged Pass, Victory Road |
| Sawsbuck | Route 121 |
| Sceptile | Petalburg Woods, Route 101, Route 120 |
| Scolipede | Petalburg Woods, Route 119, Route 120 |
| Scrafty | Route 111 |
| Scraggy | Route 111 |
| Sealeo | Shoal Cave |
| Seedot | Petalburg Woods, Route 101, Route 102, Route 103 and more |
| Seismitoad | Meteor Falls, Mt. Pyre, Petalburg City, Route 102 and more |
| Serperior | Route 116, Route 119, Route 123 |
| Servine | Route 116, Route 119, Route 123 |
| Seviper | Route 114 |
| Sewaddle | Petalburg Woods, Route 102, Route 119, Route 120 |
| Sharpedo | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Shelgon | Meteor Falls, Sky Pillar |
| Shelmet | Route 114, Route 119, Route 120 |
| Shroomish | Petalburg Woods |
| Shuppet | Cave of Origin, Mt. Pyre, Route 121, Route 123 and more |
| Sigilyph | Mirage Tower, Route 130, Sky Pillar |
| Silcoon | Petalburg Woods |
| Simisear | Magma Hideout |
| Skitty | Route 101, Route 103, Route 104, Route 116 |
| Slaking | Petalburg Woods |
| Slakoth | Petalburg Woods |
| Snivy | Route 116, Route 119, Route 123 |
| Snorunt | Shoal Cave |
| Solosis | Route 116, Route 123, Route 130 |
| Solrock | Meteor Falls |
| Spheal | Shoal Cave |
| Spinda | Route 113 |
| Spoink | Fiery Path, Jagged Pass, Route 112 |
| Stoutland | Route 101, Route 115, Route 116, Route 117 and more |
| Stunfisk | Meteor Falls, New Mauville, Route 114, Route 120 and more |
| Surskit | Mt. Pyre, Petalburg City, Route 102, Route 111 and more |
| Swablu | Meteor Falls, Mt. Pyre, Route 114, Route 115 and more |
| Swadloon | Petalburg Woods, Route 102, Route 119, Route 120 |
| Swalot | Fiery Path, Magma Hideout, Route 110 |
| Swampert | Route 102, Route 104, Route 115, Route 119 and more |
| Swanna | Ever Grande City, Lilycove City, Mossdeep City, Mt. Pyre and more |
| Swellow | Petalburg Woods, Route 101, Route 102, Route 103 and more |
| Swoobat | Altering Cave, Artisan Cave, Cave of Origin, Granite Cave and more |
| Taillow | Petalburg Woods, Route 101, Route 102, Route 103 and more |
| Tepig | Jagged Pass, Magma Hideout, Route 111 |
| Throh | Granite Cave, Jagged Pass, Victory Road |
| Timburr | Fiery Path, Jagged Pass, Magma Hideout, Route 112 and more |
| Torchic | Fiery Path, Magma Hideout, Route 112 |
| Torkoal | Fiery Path, Jagged Pass, Magma Hideout |
| Tranquill | Route 104, Route 115, Route 117, Route 118 and more |
| Trapinch | Desert Underpass, Mirage Tower, Route 111, Route 113 |
| Treecko | Petalburg Woods, Route 101, Route 120 |
| Tropius | Mt. Pyre, Route 120 |
| Trubbish | New Mauville, Route 110 |
| Tympole | Meteor Falls, Mt. Pyre, Petalburg City, Route 102 and more |
| Tynamo | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Unfezant | Route 104, Route 115, Route 117, Route 118 and more |
| Vanillish | Shoal Cave |
| Vanillite | Shoal Cave |
| Vanilluxe | Shoal Cave |
| Venipede | Petalburg Woods, Route 119, Route 120 |
| Vibrava | Desert Underpass, Mirage Tower, Route 111, Route 113 |
| Vigoroth | Petalburg Woods |
| Volbeat | Route 102, Route 104, Route 110, Route 115 and more |
| Volcarona | Magma Hideout |
| Vullaby | Mirage Tower, Mt. Pyre, Route 111, Route 113 and more |
| Wailmer | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Wailord | Abandoned Ship, Dewford Town, Ever Grande City, Lilycove City and more |
| Walrein | Shoal Cave |
| Watchog | Route 101, Route 103, Route 110, Route 116 and more |
| Whirlipede | Petalburg Woods, Route 119, Route 120 |
| Whiscash | Meteor Falls, Mt. Pyre, Petalburg City, Route 102 and more |
| Whismur | Altering Cave, Desert Underpass, Granite Cave, Route 116 and more |
| Wingull | Dewford Town, Ever Grande City, Lilycove City, Mossdeep City and more |
| Wobbuffet | Route 130 |
| Woobat | Altering Cave, Artisan Cave, Cave of Origin, Desert Underpass and more |
| Wurmple | Petalburg Woods, Route 101, Route 102, Route 103 and more |
| Wynaut | Route 130 |
| Yamask | Desert Underpass, Mirage Tower, Mt. Pyre, Route 111 and more |
| Zangoose | Route 114 |
| Zebstrika | Route 110, Route 118 |
| Zigzagoon | Route 101, Route 102, Route 103, Route 104 and more |
| Zoroark | Altering Cave, Desert Underpass, Mirage Tower, Mt. Pyre and more |
| Zorua | Altering Cave, Desert Underpass, Mirage Tower, Mt. Pyre and more |
| Zweilous | Cave of Origin, Seafloor Cavern, Sky Pillar, Victory Road |
