# Sevii encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Sevii encounter rules](sevii-encounters.md). They are the source of truth for
Sevii's wild-encounter data: every slot below maps one to one to a slot in the
game's encounter tables. Playtesting may change the picks.

## Scope

This spec lists the Sevii Islands' wild-encounter tables: 58 maps, each with a
day and a night table for every method it has. Legendaries and mythicals belong
to a later spec.

Each map's tables replace all of its current tables: the separate FireRed and
LeafGreen versions, and Altering Cave's eight event variants, become the one
table set listed here.

## Behavior

### Reading the tables

- Places are ordered by reach: Road, Wilds, Outlands, then dungeons. Each place
  names its reach and its island group, and each map is named by its map
  constant. A map with surfing or fishing also names its water type.
- Each table lists every slot in order, with its weight, and the species for
  day and night.
  - Land has 12 slots weighted 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1 and 1%.
  - Surfing, and Rock Smash rocks, have 5 slots weighted 60, 30, 5, 4 and 1%.
  - Fishing has the [Standard Rod](standard-rod-fishing.md)'s 10 entries, with
    each entry's weight for the Old, Good and Super Rod.
- **A slot's species is its stage cap,** the last name in the cell.
  "Rookidee–Corviknight" means the slot holds Corviknight, and the game steps
  it down through Corvisquire to Rookidee below each evolution level. A single name is a single-stage
  species, a baby, or a line capped at its first stage.
- Names map to species constants in capitals, with spaces and hyphens as
  underscores and other punctuation dropped: Mr. Rime is `SPECIES_MR_RIME` and
  Sirfetch'd is `SPECIES_SIRFETCHD`.
  - "Galarian" marks a Galarian form: Galarian Weezing is
    `SPECIES_WEEZING_GALAR`.
  - "Alolan" marks an Alolan form: Alolan Sandshrew is
    `SPECIES_SANDSHREW_ALOLA`.
- Slots hold no levels. Levels come from the map's reach, as the PRD defines.

### Tables

#### One Island

Road, Near islands.

**`MAP_ONE_ISLAND`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 2 | 30% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 3 | 5% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 4 | 4% | Wingull–Pelipper | Chinchou–Lanturn |
| 5 | 1% | Sobble–Inteleon | Sobble–Inteleon |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 2 | 22% | 18% | 10% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Horsea–Seadra | Pincurchin |
| 5 | 8% | 9% | 10% | Pincurchin | Horsea–Seadra |
| 6 | 4% | 7% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Arrokuda–Barraskewda | Clobbopus |
| 8 | 3% | 5% | 9% | Shellder | Krabby–Kingler |
| 9 | 2% | 4% | 9% | Clobbopus | Shellder |
| 10 | 2% | 4% | 9% | Sobble–Inteleon | Sobble–Inteleon |

#### Four Island

Road, Outer islands.

**`MAP_FOUR_ISLAND`**

Water type: cold water.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Arrokuda–Barraskewda | Clobbopus |
| 2 | 30% | Clobbopus | Arrokuda–Barraskewda |
| 3 | 5% | Seel–Dewgong | Seel–Dewgong |
| 4 | 4% | Arrokuda–Barraskewda | Clobbopus |
| 5 | 1% | Shellder | Shellder |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Clobbopus | Arrokuda–Barraskewda |
| 3 | 10% | 12% | 11% | Clobbopus | Arrokuda–Barraskewda |
| 4 | 8% | 10% | 10% | Spheal–Walrein | Shellder |
| 5 | 8% | 9% | 10% | Arrokuda–Barraskewda | Clobbopus |
| 6 | 4% | 7% | 10% | Shellder | Seel–Dewgong |
| 7 | 3% | 6% | 10% | Arrokuda–Barraskewda | Clobbopus |
| 8 | 3% | 5% | 9% | Clobbopus | Arrokuda–Barraskewda |
| 9 | 2% | 4% | 9% | Clobbopus | Tentacool |
| 10 | 2% | 4% | 9% | Spheal–Walrein | Arrokuda–Barraskewda |

#### Five Island

Road, Outer islands.

**`MAP_FIVE_ISLAND`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pincurchin | Arrokuda–Barraskewda |
| 2 | 30% | Arrokuda–Barraskewda | Pincurchin |
| 3 | 5% | Wingull–Pelipper | Galarian Corsola–Cursola |
| 4 | 4% | Galarian Corsola–Cursola | Galarian Corsola–Cursola |
| 5 | 1% | Chewtle–Drednaw | Chewtle–Drednaw |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Pincurchin | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Arrokuda–Barraskewda | Pincurchin |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Clobbopus | Galarian Corsola–Cursola |
| 5 | 8% | 9% | 10% | Galarian Corsola | Clobbopus |
| 6 | 4% | 7% | 10% | Chewtle–Drednaw | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Pincurchin | Chewtle–Drednaw |
| 8 | 3% | 5% | 9% | Qwilfish | Qwilfish |
| 9 | 2% | 4% | 9% | Arrokuda–Barraskewda | Pincurchin |
| 10 | 2% | 4% | 9% | Galarian Corsola–Cursola | Galarian Corsola |

#### Three Isle Port

Road, Near islands.

**`MAP_THREE_ISLAND_PORT`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wooloo–Dubwool | Nickit–Thievul |
| 2 | 20% | Rookidee–Corviknight | Galarian Zigzagoon–Obstagoon |
| 3 | 10% | Yamper–Boltund | Galarian Meowth–Perrserker |
| 4 | 10% | Skwovet–Greedent | Morpeko |
| 5 | 10% | Galarian Meowth–Perrserker | Wooloo–Dubwool |
| 6 | 10% | Morpeko | Rattata–Raticate |
| 7 | 5% | Gossifleur–Eldegoss | Impidimp–Grimmsnarl |
| 8 | 5% | Wingull–Pelipper | Hoothoot–Noctowl |
| 9 | 4% | Pidgey–Pidgeotto | Yamper–Boltund |
| 10 | 4% | Blipbug–Orbeetle | Skwovet–Greedent |
| 11 | 1% | Milcery | Milcery |
| 12 | 1% | Toxel | Toxel |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Pincurchin |
| 2 | 30% | Pincurchin | Chewtle–Drednaw |
| 3 | 5% | Clobbopus | Clobbopus |
| 4 | 4% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 5 | 1% | Sobble–Inteleon | Chinchou–Lanturn |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Pincurchin |
| 2 | 22% | 18% | 10% | Pincurchin | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Clobbopus | Clobbopus |
| 5 | 8% | 9% | 10% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 6 | 4% | 7% | 10% | Chewtle–Drednaw | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Pincurchin | Pincurchin |
| 8 | 3% | 5% | 9% | Krabby–Kingler | Chewtle–Drednaw |
| 9 | 2% | 4% | 9% | Clobbopus | Krabby–Kingler |
| 10 | 2% | 4% | 9% | Sobble–Inteleon | Sobble–Inteleon |

#### Kindle Road

Road, Near islands.

**`MAP_ONE_ISLAND_KINDLE_ROAD`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rolycoly–Coalossal | Nickit–Thievul |
| 2 | 20% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 10% | Rookidee–Corviknight | Sizzlipede–Centiskorch |
| 4 | 10% | Yamper–Boltund | Koffing–Galarian Weezing |
| 5 | 10% | Chewtle–Drednaw | Galarian Meowth–Perrserker |
| 6 | 10% | Skwovet–Greedent | Galarian Zigzagoon–Obstagoon |
| 7 | 5% | Spearow–Fearow | Rattata–Raticate |
| 8 | 5% | Ponyta–Rapidash | Yamper–Boltund |
| 9 | 4% | Koffing–Galarian Weezing | Chewtle–Drednaw |
| 10 | 4% | Galarian Meowth–Perrserker | Hoothoot–Noctowl |
| 11 | 1% | Scorbunny–Cinderace | Scorbunny–Cinderace |
| 12 | 1% | Toxel | Toxel |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 2 | 30% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 3 | 5% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 4 | 4% | Horsea–Seadra | Chinchou–Lanturn |
| 5 | 1% | Pincurchin | Pincurchin |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Krabby–Kingler | Pincurchin |
| 5 | 8% | 9% | 10% | Pincurchin | Krabby–Kingler |
| 6 | 4% | 7% | 10% | Horsea–Seadra | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Clobbopus | Clobbopus |
| 8 | 3% | 5% | 9% | Shellder | Horsea–Seadra |
| 9 | 2% | 4% | 9% | Arrokuda–Barraskewda | Shellder |
| 10 | 2% | 4% | 9% | Staryu | Staryu |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pincurchin | Clobbopus |
| 2 | 30% | Clobbopus | Pincurchin |
| 3 | 5% | Geodude–Graveler | Geodude–Graveler |
| 4 | 4% | Rolycoly–Coalossal | Rolycoly–Coalossal |
| 5 | 1% | Chewtle–Drednaw | Chewtle–Drednaw |

#### Bond Bridge

Road, Near islands.

**`MAP_THREE_ISLAND_BOND_BRIDGE`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gossifleur–Eldegoss | Nickit–Thievul |
| 2 | 20% | Blipbug–Orbeetle | Blipbug–Orbeetle |
| 3 | 10% | Skwovet–Greedent | Venonat–Venomoth |
| 4 | 10% | Rookidee–Corviknight | Galarian Zigzagoon–Obstagoon |
| 5 | 10% | Wooloo–Dubwool | Oddish–Gloom |
| 6 | 10% | Yamper–Boltund | Skwovet–Greedent |
| 7 | 5% | Bellsprout–Weepinbell | Impidimp–Grimmsnarl |
| 8 | 5% | Galarian Meowth–Perrserker | Gossifleur–Eldegoss |
| 9 | 4% | Galarian Zigzagoon–Obstagoon | Hoothoot–Noctowl |
| 10 | 4% | Yamper–Boltund | Galarian Meowth–Perrserker |
| 11 | 1% | Applin | Applin |
| 12 | 1% | Milcery | Milcery |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Arrokuda–Barraskewda | Pincurchin |
| 2 | 30% | Pincurchin | Arrokuda–Barraskewda |
| 3 | 5% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 4 | 4% | Wingull–Pelipper | Chinchou–Lanturn |
| 5 | 1% | Staryu | Staryu |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Arrokuda–Barraskewda | Pincurchin |
| 2 | 22% | 18% | 10% | Pincurchin | Arrokuda–Barraskewda |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Clobbopus | Clobbopus |
| 5 | 8% | 9% | 10% | Krabby–Kingler | Chinchou–Lanturn |
| 6 | 4% | 7% | 10% | Chewtle–Drednaw | Chewtle–Drednaw |
| 7 | 3% | 6% | 10% | Tentacool–Tentacruel | Krabby–Kingler |
| 8 | 3% | 5% | 9% | Horsea–Seadra | Horsea–Seadra |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Remoraid |
| 10 | 2% | 4% | 9% | Staryu | Staryu |

#### Treasure Beach

Wilds, Near islands.

**`MAP_ONE_ISLAND_TREASURE_BEACH`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Chewtle–Drednaw | Chewtle–Drednaw |
| 2 | 20% | Clobbopus | Clobbopus |
| 3 | 10% | Silicobra–Sandaconda | Nickit–Thievul |
| 4 | 10% | Yamper–Boltund | Galarian Zigzagoon–Obstagoon |
| 5 | 10% | Skwovet–Greedent | Silicobra–Sandaconda |
| 6 | 10% | Slowpoke–Slowbro | Staryu |
| 7 | 5% | Krabby–Kingler | Krabby–Kingler |
| 8 | 5% | Cramorant | Slowpoke–Slowbro |
| 9 | 4% | Pincurchin | Pincurchin |
| 10 | 4% | Wingull–Pelipper | Galarian Corsola–Cursola |
| 11 | 1% | Sobble–Inteleon | Sobble–Inteleon |
| 12 | 1% | Spearow–Fearow | Cramorant |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Clobbopus |
| 2 | 30% | Clobbopus | Chewtle–Drednaw |
| 3 | 5% | Tentacool–Tentacruel | Tentacool–Tentacruel |
| 4 | 4% | Wingull–Pelipper | Staryu |
| 5 | 1% | Cramorant | Cramorant |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Clobbopus |
| 2 | 22% | 18% | 10% | Clobbopus | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 5 | 8% | 9% | 10% | Krabby–Kingler | Pincurchin |
| 6 | 4% | 7% | 10% | Pincurchin | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Horsea–Seadra | Krabby–Kingler |
| 8 | 3% | 5% | 9% | Shellder | Horsea–Seadra |
| 9 | 2% | 4% | 9% | Staryu | Staryu |
| 10 | 2% | 4% | 9% | Galarian Corsola | Galarian Corsola |

#### Cape Brink

Wilds, Near islands.

**`MAP_TWO_ISLAND_CAPE_BRINK`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wooloo–Dubwool | Nickit–Thievul |
| 2 | 20% | Gossifleur–Eldegoss | Wooloo–Dubwool |
| 3 | 10% | Rookidee–Corviknight | Galarian Zigzagoon–Obstagoon |
| 4 | 10% | Yamper–Boltund | Hoothoot–Noctowl |
| 5 | 10% | Skwovet–Greedent | Psyduck–Golduck |
| 6 | 10% | Blipbug–Orbeetle | Galarian Meowth–Perrserker |
| 7 | 5% | Psyduck–Golduck | Impidimp–Grimmsnarl |
| 8 | 5% | Spearow–Fearow | Galarian Meowth–Perrserker |
| 9 | 4% | Morpeko | Blipbug–Orbeetle |
| 10 | 4% | Galarian Meowth–Perrserker | Morpeko |
| 11 | 1% | Sobble–Inteleon | Sobble–Inteleon |
| 12 | 1% | Toxel | Toxel |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Chewtle–Drednaw |
| 2 | 30% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 3 | 5% | Psyduck–Golduck | Psyduck–Golduck |
| 4 | 4% | Poliwag–Poliwhirl | Basculin |
| 5 | 1% | Galarian Stunfisk | Galarian Stunfisk |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Poliwag–Poliwhirl |
| 5 | 8% | 9% | 10% | Poliwag–Poliwhirl | Goldeen–Seaking |
| 6 | 4% | 7% | 10% | Psyduck–Golduck | Psyduck–Golduck |
| 7 | 3% | 6% | 10% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 8 | 3% | 5% | 9% | Chewtle–Drednaw | Basculin |
| 9 | 2% | 4% | 9% | Galarian Stunfisk | Galarian Stunfisk |
| 10 | 2% | 4% | 9% | Sobble–Inteleon | Sobble–Inteleon |

#### Berry Forest

Wilds, Near islands.

**`MAP_THREE_ISLAND_BERRY_FOREST`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Skwovet–Greedent | Impidimp–Grimmsnarl |
| 2 | 20% | Hatenna–Hatterene | Hatenna–Hatterene |
| 3 | 10% | Galarian Ponyta–Galarian Rapidash | Venonat–Venomoth |
| 4 | 10% | Blipbug–Orbeetle | Oddish–Gloom |
| 5 | 10% | Gossifleur–Eldegoss | Skwovet–Greedent |
| 6 | 10% | Exeggcute | Galarian Ponyta–Galarian Rapidash |
| 7 | 5% | Applin | Morelull–Shiinotic |
| 8 | 5% | Bellsprout–Weepinbell | Drowzee–Hypno |
| 9 | 4% | Milcery | Applin |
| 10 | 4% | Drowzee–Hypno | Indeedee |
| 11 | 1% | Grookey–Rillaboom | Grookey–Rillaboom |
| 12 | 1% | Indeedee | Milcery |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 2 | 30% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 3 | 5% | Marill–Azumarill | Marill–Azumarill |
| 4 | 4% | Lotad–Lombre | Dewpider–Araquanid |
| 5 | 1% | Sobble–Inteleon | Sobble–Inteleon |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Magikarp | Magikarp |
| 4 | 8% | 10% | 10% | Marill–Azumarill | Marill–Azumarill |
| 5 | 8% | 9% | 10% | Lotad–Lombre | Lotad–Lombre |
| 6 | 4% | 7% | 10% | Chewtle–Drednaw | Dewpider–Araquanid |
| 7 | 3% | 6% | 10% | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| 8 | 3% | 5% | 9% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 9 | 2% | 4% | 9% | Magikarp–Gyarados | Magikarp–Gyarados |
| 10 | 2% | 4% | 9% | Sobble–Inteleon | Sobble–Inteleon |

#### Five Isle Meadow

Wilds, Outer islands.

**`MAP_FIVE_ISLAND_MEADOW`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gossifleur–Eldegoss | Nickit–Thievul |
| 2 | 20% | Wooloo–Dubwool | Wooloo–Dubwool |
| 3 | 10% | Morpeko | Galarian Zigzagoon–Obstagoon |
| 4 | 10% | Yamper–Boltund | Impidimp–Grimmsnarl |
| 5 | 10% | Blipbug–Orbeetle | Morpeko |
| 6 | 10% | Skwovet–Greedent | Hatenna–Hatterene |
| 7 | 5% | Hoppip–Jumpluff | Hoothoot–Noctowl |
| 8 | 5% | Galarian Ponyta–Galarian Rapidash | Gossifleur–Eldegoss |
| 9 | 4% | Falinks | Blipbug–Orbeetle |
| 10 | 4% | Scorbunny–Cinderace | Falinks |
| 11 | 1% | Cramorant | Sinistea |
| 12 | 1% | Milcery | Cramorant |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Clobbopus |
| 2 | 30% | Clobbopus | Chewtle–Drednaw |
| 3 | 5% | Wingull–Pelipper | Wingull–Pelipper |
| 4 | 4% | Cramorant | Galarian Corsola–Cursola |
| 5 | 1% | Galarian Corsola–Cursola | Cramorant |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Clobbopus |
| 2 | 22% | 18% | 10% | Clobbopus | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Arrokuda–Barraskewda | Pincurchin |
| 5 | 8% | 9% | 10% | Pincurchin | Arrokuda–Barraskewda |
| 6 | 4% | 7% | 10% | Galarian Corsola | Galarian Corsola–Cursola |
| 7 | 3% | 6% | 10% | Chewtle–Drednaw | Galarian Corsola |
| 8 | 3% | 5% | 9% | Wailmer | Galarian Corsola–Cursola |
| 9 | 2% | 4% | 9% | Pyukumuku | Pyukumuku |
| 10 | 2% | 4% | 9% | Clobbopus | Chewtle–Drednaw |

#### Resort Gorgeous

Wilds, Outer islands.

**`MAP_FIVE_ISLAND_RESORT_GORGEOUS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Arrokuda–Barraskewda | Pincurchin |
| 2 | 30% | Pincurchin | Arrokuda–Barraskewda |
| 3 | 5% | Luvdisc | Luvdisc |
| 4 | 4% | Cramorant | Galarian Corsola–Cursola |
| 5 | 1% | Galarian Corsola | Cramorant |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Arrokuda–Barraskewda | Pincurchin |
| 2 | 22% | 18% | 10% | Pincurchin | Arrokuda–Barraskewda |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Luvdisc | Luvdisc |
| 5 | 8% | 9% | 10% | Galarian Corsola | Galarian Corsola–Cursola |
| 6 | 4% | 7% | 10% | Wailmer | Wailmer |
| 7 | 3% | 6% | 10% | Clobbopus | Chinchou–Lanturn |
| 8 | 3% | 5% | 9% | Chewtle–Drednaw | Clobbopus |
| 9 | 2% | 4% | 9% | Luvdisc | Chewtle–Drednaw |
| 10 | 2% | 4% | 9% | Sobble–Inteleon | Sobble–Inteleon |

#### Water Path

Wilds, Outer islands.

**`MAP_SIX_ISLAND_WATER_PATH`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rookidee–Corviknight | Nickit–Thievul |
| 2 | 20% | Clobbopus | Clobbopus |
| 3 | 10% | Galarian Slowpoke | Impidimp–Grimmsnarl |
| 4 | 10% | Galarian Farfetch'd | Galarian Zigzagoon–Obstagoon |
| 5 | 10% | Skwovet–Greedent | Galarian Slowpoke |
| 6 | 10% | Chewtle–Drednaw | Galarian Meowth–Perrserker |
| 7 | 5% | Falinks | Rookidee–Corviknight |
| 8 | 5% | Pyukumuku | Galarian Farfetch'd |
| 9 | 4% | Yamper–Boltund | Falinks |
| 10 | 4% | Silicobra–Sandaconda | Chewtle–Drednaw |
| 11 | 1% | Stonjourner | Stonjourner |
| 12 | 1% | Cramorant | Hatenna–Hatterene |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 2 | 30% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 3 | 5% | Pyukumuku | Mareanie–Toxapex |
| 4 | 4% | Cramorant | Cramorant |
| 5 | 1% | Galarian Slowpoke | Galarian Slowpoke |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Clobbopus | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Arrokuda–Barraskewda | Clobbopus |
| 3 | 10% | 12% | 11% | Arrokuda–Barraskewda | Clobbopus |
| 4 | 8% | 10% | 10% | Galarian Slowpoke | Pincurchin |
| 5 | 8% | 9% | 10% | Pincurchin | Galarian Slowpoke |
| 6 | 4% | 7% | 10% | Chewtle–Drednaw | Galarian Corsola–Cursola |
| 7 | 3% | 6% | 10% | Mareanie–Toxapex | Chewtle–Drednaw |
| 8 | 3% | 5% | 9% | Clobbopus | Mareanie–Toxapex |
| 9 | 2% | 4% | 9% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 10 | 2% | 4% | 9% | Pyukumuku | Pyukumuku |

#### Sevault Canyon Entrance

Wilds, Outer islands.

**`MAP_SEVEN_ISLAND_SEVAULT_CANYON_ENTRANCE`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Silicobra–Sandaconda | Nickit–Thievul |
| 2 | 20% | Rookidee–Corviknight | Silicobra–Sandaconda |
| 3 | 10% | Rolycoly–Coalossal | Galarian Zigzagoon–Obstagoon |
| 4 | 10% | Cufant–Copperajah | Cufant–Copperajah |
| 5 | 10% | Falinks | Galarian Meowth–Perrserker |
| 6 | 10% | Galarian Zigzagoon–Obstagoon | Impidimp–Grimmsnarl |
| 7 | 5% | Phanpy–Donphan | Falinks |
| 8 | 5% | Galarian Farfetch'd | Galarian Farfetch'd |
| 9 | 4% | Galarian Yamask | Hoothoot–Noctowl |
| 10 | 4% | Galarian Meowth–Perrserker | Galarian Yamask |
| 11 | 1% | Scorbunny–Cinderace | Hatenna–Hatterene |
| 12 | 1% | Toxel | Toxel |

#### Trainer Tower grounds

Wilds, Outer islands.

**`MAP_SEVEN_ISLAND_TRAINER_TOWER`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Pincurchin |
| 2 | 30% | Pincurchin | Chewtle–Drednaw |
| 3 | 5% | Wailmer | Galarian Corsola–Cursola |
| 4 | 4% | Cramorant | Cramorant |
| 5 | 1% | Clobbopus | Clobbopus |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Pincurchin |
| 2 | 22% | 18% | 10% | Pincurchin | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 5 | 8% | 9% | 10% | Clobbopus | Clobbopus |
| 6 | 4% | 7% | 10% | Wailmer | Galarian Corsola–Cursola |
| 7 | 3% | 6% | 10% | Galarian Corsola | Galarian Corsola–Cursola |
| 8 | 3% | 5% | 9% | Chewtle–Drednaw | Wailmer |
| 9 | 2% | 4% | 9% | Qwilfish | Pyukumuku |
| 10 | 2% | 4% | 9% | Pyukumuku | Chewtle–Drednaw |

#### Memorial Pillar

Outlands, Outer islands.

**`MAP_FIVE_ISLAND_MEMORIAL_PILLAR`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Gossifleur–Eldegoss | Galarian Yamask–Runerigus |
| 2 | 20% | Galarian Yamask–Runerigus | Galarian Corsola–Cursola |
| 3 | 10% | Sinistea | Sinistea |
| 4 | 10% | Rookidee–Corviknight | Nickit–Thievul |
| 5 | 10% | Galarian Corsola–Cursola | Impidimp–Grimmsnarl |
| 6 | 10% | Skwovet–Greedent | Dreepy–Dragapult |
| 7 | 5% | Dreepy–Dragapult | Gastly–Haunter |
| 8 | 5% | Hatenna–Hatterene | Hatenna–Hatterene |
| 9 | 4% | Alcremie | Misdreavus |
| 10 | 4% | Falinks | Gossifleur–Eldegoss |
| 11 | 1% | Hoppip–Jumpluff | Galarian Zigzagoon–Obstagoon |
| 12 | 1% | Milcery | Alcremie |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pincurchin | Arrokuda–Barraskewda |
| 2 | 30% | Arrokuda–Barraskewda | Pincurchin |
| 3 | 5% | Galarian Corsola–Cursola | Galarian Corsola–Cursola |
| 4 | 4% | Clobbopus–Grapploct | Clobbopus–Grapploct |
| 5 | 1% | Cramorant | Cramorant |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Arrokuda–Barraskewda | Pincurchin |
| 2 | 22% | 18% | 10% | Pincurchin | Arrokuda–Barraskewda |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Galarian Corsola | Galarian Corsola–Cursola |
| 5 | 8% | 9% | 10% | Clobbopus–Grapploct | Frillish |
| 6 | 4% | 7% | 10% | Chewtle–Drednaw | Clobbopus–Grapploct |
| 7 | 3% | 6% | 10% | Galarian Corsola–Cursola | Galarian Corsola |
| 8 | 3% | 5% | 9% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 9 | 2% | 4% | 9% | Pincurchin | Galarian Corsola–Cursola |
| 10 | 2% | 4% | 9% | Clobbopus–Grapploct | Pincurchin |

#### Water Labyrinth

Outlands, Outer islands.

**`MAP_FIVE_ISLAND_WATER_LABYRINTH`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clobbopus–Grapploct | Chewtle–Drednaw |
| 2 | 30% | Chewtle–Drednaw | Clobbopus–Grapploct |
| 3 | 5% | Cramorant | Galarian Corsola–Cursola |
| 4 | 4% | Wingull–Pelipper | Wingull–Pelipper |
| 5 | 1% | Galarian Corsola–Cursola | Cramorant |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Clobbopus–Grapploct | Chewtle–Drednaw |
| 2 | 22% | 18% | 10% | Chewtle–Drednaw | Clobbopus–Grapploct |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Arrokuda–Barraskewda | Pincurchin |
| 5 | 8% | 9% | 10% | Pincurchin | Arrokuda–Barraskewda |
| 6 | 4% | 7% | 10% | Galarian Corsola | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Clobbopus–Grapploct | Galarian Corsola–Cursola |
| 8 | 3% | 5% | 9% | Wailmer | Wailmer |
| 9 | 2% | 4% | 9% | Chewtle–Drednaw | Clobbopus–Grapploct |
| 10 | 2% | 4% | 9% | Galarian Corsola–Cursola | Galarian Corsola |

#### Ruin Valley

Outlands, Outer islands.

**`MAP_SIX_ISLAND_RUIN_VALLEY`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Silicobra–Sandaconda | Blipbug–Orbeetle |
| 2 | 20% | Blipbug–Orbeetle | Silicobra–Sandaconda |
| 3 | 10% | Stonjourner | Impidimp–Grimmsnarl |
| 4 | 10% | Galarian Yamask–Runerigus | Nickit–Thievul |
| 5 | 10% | Hatenna–Hatterene | Galarian Yamask–Runerigus |
| 6 | 10% | Cufant–Copperajah | Morpeko |
| 7 | 5% | Natu–Xatu | Stonjourner |
| 8 | 5% | Indeedee | Galarian Zigzagoon–Obstagoon |
| 9 | 4% | Rookidee–Corviknight | Sinistea |
| 10 | 4% | Falinks | Natu–Xatu |
| 11 | 1% | Duraludon | Dreepy–Dragapult |
| 12 | 1% | Galarian Yamask–Runerigus | Indeedee |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Chewtle–Drednaw |
| 2 | 30% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 3 | 5% | Galarian Stunfisk | Wooper–Quagsire |
| 4 | 4% | Wooper–Quagsire | Galarian Stunfisk |
| 5 | 1% | Marill–Azumarill | Basculin |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 4 | 8% | 10% | 10% | Goldeen–Seaking | Galarian Stunfisk |
| 5 | 8% | 9% | 10% | Galarian Stunfisk | Poliwag–Poliwhirl |
| 6 | 4% | 7% | 10% | Poliwag–Poliwhirl | Basculin |
| 7 | 3% | 6% | 10% | Arrokuda–Barraskewda | Goldeen–Seaking |
| 8 | 3% | 5% | 9% | Chewtle–Drednaw | Chewtle–Drednaw |
| 9 | 2% | 4% | 9% | Wooper–Quagsire | Wooper–Quagsire |
| 10 | 2% | 4% | 9% | Galarian Stunfisk | Arrokuda–Barraskewda |

#### Green Path

Outlands, Outer islands.

**`MAP_SIX_ISLAND_GREEN_PATH`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clobbopus–Grapploct | Pincurchin |
| 2 | 30% | Pincurchin | Clobbopus–Grapploct |
| 3 | 5% | Galarian Slowpoke–Galarian Slowbro | Galarian Slowpoke |
| 4 | 4% | Cramorant | Cramorant |
| 5 | 1% | Galarian Slowpoke–Galarian Slowking | Galarian Slowpoke–Galarian Slowking |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Pincurchin | Clobbopus–Grapploct |
| 2 | 22% | 18% | 10% | Clobbopus–Grapploct | Pincurchin |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Galarian Slowpoke | Galarian Slowpoke |
| 5 | 8% | 9% | 10% | Arrokuda–Barraskewda | Galarian Corsola–Cursola |
| 6 | 4% | 7% | 10% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 7 | 3% | 6% | 10% | Galarian Slowpoke–Galarian Slowbro | Chewtle–Drednaw |
| 8 | 3% | 5% | 9% | Mareanie–Toxapex | Galarian Slowpoke–Galarian Slowbro |
| 9 | 2% | 4% | 9% | Pincurchin | Mareanie–Toxapex |
| 10 | 2% | 4% | 9% | Galarian Slowpoke–Galarian Slowking | Galarian Slowpoke–Galarian Slowking |

#### Pattern Bush

Outlands, Outer islands.

**`MAP_SIX_ISLAND_PATTERN_BUSH`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Blipbug–Orbeetle | Blipbug–Orbeetle |
| 2 | 20% | Gossifleur–Eldegoss | Sizzlipede–Centiskorch |
| 3 | 10% | Falinks | Spinarak–Ariados |
| 4 | 10% | Sizzlipede–Centiskorch | Nickit–Thievul |
| 5 | 10% | Ledyba–Ledian | Impidimp–Grimmsnarl |
| 6 | 10% | Applin–Flapple | Falinks |
| 7 | 5% | Pineco–Forretress | Hatenna–Hatterene |
| 8 | 5% | Skwovet–Greedent | Pineco–Forretress |
| 9 | 4% | Venipede–Scolipede | Venonat–Venomoth |
| 10 | 4% | Cutiefly–Ribombee | Applin–Flapple |
| 11 | 1% | Applin–Appletun | Morelull–Shiinotic |
| 12 | 1% | Galarian Ponyta–Galarian Rapidash | Applin–Appletun |

#### Outcast Island

Outlands, Outer islands.

**`MAP_SIX_ISLAND_OUTCAST_ISLAND`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clobbopus–Grapploct | Arrokuda–Barraskewda |
| 2 | 30% | Arrokuda–Barraskewda | Clobbopus–Grapploct |
| 3 | 5% | Cramorant | Galarian Corsola–Cursola |
| 4 | 4% | Wailmer | Wailmer |
| 5 | 1% | Galarian Corsola–Cursola | Cramorant |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Clobbopus–Grapploct | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Arrokuda–Barraskewda | Clobbopus–Grapploct |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Chewtle–Drednaw | Pincurchin |
| 5 | 8% | 9% | 10% | Pincurchin | Chewtle–Drednaw |
| 6 | 4% | 7% | 10% | Wailmer | Galarian Corsola–Cursola |
| 7 | 3% | 6% | 10% | Arrokuda–Barraskewda | Wailmer |
| 8 | 3% | 5% | 9% | Galarian Corsola | Galarian Corsola–Cursola |
| 9 | 2% | 4% | 9% | Clobbopus–Grapploct | Clobbopus–Grapploct |
| 10 | 2% | 4% | 9% | Chewtle–Drednaw | Arrokuda–Barraskewda |

#### Sevault Canyon

Outlands, Outer islands.

**`MAP_SEVEN_ISLAND_SEVAULT_CANYON`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Cufant–Copperajah | Cufant–Copperajah |
| 2 | 20% | Rookidee–Corviknight | Silicobra–Sandaconda |
| 3 | 10% | Silicobra–Sandaconda | Galarian Zigzagoon–Obstagoon |
| 4 | 10% | Falinks | Impidimp–Grimmsnarl |
| 5 | 10% | Rolycoly–Coalossal | Galarian Meowth–Perrserker |
| 6 | 10% | Phanpy–Donphan | Dreepy–Dragapult |
| 7 | 5% | Duraludon | Duraludon |
| 8 | 5% | Galarian Farfetch'd | Galarian Farfetch'd |
| 9 | 4% | Stonjourner | Cubone–Marowak |
| 10 | 4% | Cubone–Marowak | Stonjourner |
| 11 | 1% | Galarian Stunfisk | Hatenna–Hatterene |
| 12 | 1% | Galarian Farfetch'd–Sirfetch'd | Falinks |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Chewtle–Drednaw | Chewtle–Drednaw |
| 2 | 30% | Arrokuda–Barraskewda | Galarian Stunfisk |
| 3 | 5% | Galarian Stunfisk | Arrokuda–Barraskewda |
| 4 | 4% | Sobble–Inteleon | Galarian Slowpoke |
| 5 | 1% | Galarian Slowpoke | Sobble–Inteleon |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 2 | 22% | 18% | 10% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 3 | 10% | 12% | 11% | Chewtle–Drednaw | Galarian Stunfisk |
| 4 | 8% | 10% | 10% | Galarian Stunfisk | Chewtle–Drednaw |
| 5 | 8% | 9% | 10% | Arrokuda–Barraskewda | Arrokuda–Barraskewda |
| 6 | 4% | 7% | 10% | Galarian Slowpoke | Galarian Stunfisk |
| 7 | 3% | 6% | 10% | Chewtle–Drednaw | Galarian Slowpoke |
| 8 | 3% | 5% | 9% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 9 | 2% | 4% | 9% | Galarian Stunfisk | Basculin |
| 10 | 2% | 4% | 9% | Sobble–Inteleon | Sobble–Inteleon |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Silicobra–Sandaconda | Galarian Yamask–Runerigus |
| 2 | 30% | Galarian Yamask–Runerigus | Silicobra–Sandaconda |
| 3 | 5% | Stonjourner | Stonjourner |
| 4 | 4% | Geodude–Golem | Geodude–Golem |
| 5 | 1% | Duraludon | Duraludon |

#### Tanoby Ruins

Outlands, Outer islands.

**`MAP_SEVEN_ISLAND_TANOBY_RUINS`**

Water type: coast and sea.

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Pincurchin | Clobbopus–Grapploct |
| 2 | 30% | Clobbopus–Grapploct | Pincurchin |
| 3 | 5% | Cramorant | Galarian Corsola–Cursola |
| 4 | 4% | Wailmer | Galarian Corsola–Cursola |
| 5 | 1% | Galarian Corsola–Cursola | Cramorant |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Pincurchin | Clobbopus–Grapploct |
| 2 | 22% | 18% | 10% | Clobbopus–Grapploct | Pincurchin |
| 3 | 10% | 12% | 11% | Pincurchin | Pincurchin |
| 4 | 8% | 10% | 10% | Arrokuda–Barraskewda | Chewtle–Drednaw |
| 5 | 8% | 9% | 10% | Chewtle–Drednaw | Arrokuda–Barraskewda |
| 6 | 4% | 7% | 10% | Galarian Corsola | Chinchou–Lanturn |
| 7 | 3% | 6% | 10% | Wailmer | Galarian Corsola–Cursola |
| 8 | 3% | 5% | 9% | Pincurchin | Wailmer |
| 9 | 2% | 4% | 9% | Pyukumuku | Pyukumuku |
| 10 | 2% | 4% | 9% | Galarian Corsola–Cursola | Galarian Corsola |

#### Icefall Cave

Dungeon, Outer islands.

**`MAP_FOUR_ISLAND_ICEFALL_CAVE_ENTRANCE`**

Water type: cold water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Cufant–Copperajah | Snom |
| 2 | 20% | Snom | Cufant–Copperajah |
| 3 | 10% | Galarian Darumaka | Galarian Mr. Mime–Mr. Rime |
| 4 | 10% | Eiscue | Galarian Darumaka |
| 5 | 10% | Cufant–Copperajah | Alolan Vulpix |
| 6 | 10% | Galarian Mr. Mime–Mr. Rime | Cufant–Copperajah |
| 7 | 5% | Swinub–Piloswine | Eiscue |
| 8 | 5% | Snom | Swinub–Piloswine |
| 9 | 4% | Snorunt–Glalie | Snom |
| 10 | 4% | Galarian Darumaka | Snorunt–Glalie |
| 11 | 1% | Alolan Sandshrew | Galarian Mr. Mime–Mr. Rime |
| 12 | 1% | Eiscue | Eiscue |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Arrokuda–Barraskewda | Clobbopus |
| 2 | 30% | Clobbopus | Arrokuda–Barraskewda |
| 3 | 5% | Eiscue | Eiscue |
| 4 | 4% | Seel–Dewgong | Seel–Dewgong |
| 5 | 1% | Clobbopus | Spheal |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Arrokuda–Barraskewda | Clobbopus |
| 2 | 22% | 18% | 10% | Clobbopus | Arrokuda–Barraskewda |
| 3 | 10% | 12% | 11% | Clobbopus | Arrokuda–Barraskewda |
| 4 | 8% | 10% | 10% | Arrokuda–Barraskewda | Clobbopus |
| 5 | 8% | 9% | 10% | Shellder | Eiscue |
| 6 | 4% | 7% | 10% | Eiscue | Shellder |
| 7 | 3% | 6% | 10% | Clobbopus | Arrokuda–Barraskewda |
| 8 | 3% | 5% | 9% | Arrokuda–Barraskewda | Clobbopus |
| 9 | 2% | 4% | 9% | Seel–Dewgong | Spheal |
| 10 | 2% | 4% | 9% | Eiscue | Eiscue |

**`MAP_FOUR_ISLAND_ICEFALL_CAVE_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Cufant–Copperajah | Galarian Mr. Mime–Mr. Rime |
| 2 | 20% | Galarian Mr. Mime–Mr. Rime | Cufant–Copperajah |
| 3 | 10% | Snom | Snom |
| 4 | 10% | Galarian Darumaka | Eiscue |
| 5 | 10% | Eiscue | Galarian Darumaka |
| 6 | 10% | Cufant–Copperajah | Alolan Vulpix–Alolan Ninetales |
| 7 | 5% | Swinub–Piloswine | Cufant–Copperajah |
| 8 | 5% | Snom–Frosmoth | Snom–Frosmoth |
| 9 | 4% | Galarian Mr. Mime–Mr. Rime | Swinub–Piloswine |
| 10 | 4% | Snom | Snom |
| 11 | 1% | Galarian Darumaka–Galarian Darmanitan | Galarian Darumaka–Galarian Darmanitan |
| 12 | 1% | Vanillite–Vanilluxe | Galarian Mr. Mime–Mr. Rime |

**`MAP_FOUR_ISLAND_ICEFALL_CAVE_B1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Snom | Eiscue |
| 2 | 20% | Eiscue | Snom |
| 3 | 10% | Cufant–Copperajah | Galarian Mr. Mime–Mr. Rime |
| 4 | 10% | Galarian Mr. Mime–Mr. Rime | Cufant–Copperajah |
| 5 | 10% | Galarian Darumaka | Galarian Darumaka |
| 6 | 10% | Snom–Frosmoth | Snom–Frosmoth |
| 7 | 5% | Snorunt–Glalie | Alolan Vulpix–Alolan Ninetales |
| 8 | 5% | Snom | Snom |
| 9 | 4% | Cufant–Copperajah | Galarian Darumaka–Galarian Darmanitan |
| 10 | 4% | Alolan Sandshrew–Alolan Sandslash | Snorunt–Glalie |
| 11 | 1% | Galarian Darumaka–Galarian Darmanitan | Duraludon |
| 12 | 1% | Duraludon | Galarian Mr. Mime–Mr. Rime |

**`MAP_FOUR_ISLAND_ICEFALL_CAVE_BACK`**

Water type: cold water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Eiscue | Galarian Mr. Mime–Mr. Rime |
| 2 | 20% | Galarian Mr. Mime–Mr. Rime | Eiscue |
| 3 | 10% | Snom–Frosmoth | Snom–Frosmoth |
| 4 | 10% | Galarian Darumaka–Galarian Darmanitan | Galarian Darumaka–Galarian Darmanitan |
| 5 | 10% | Cufant–Copperajah | Cufant–Copperajah |
| 6 | 10% | Snom | Alolan Vulpix–Alolan Ninetales |
| 7 | 5% | Galarian Darumaka | Snom |
| 8 | 5% | Spheal–Walrein | Galarian Darumaka |
| 9 | 4% | Cufant–Copperajah | Cufant–Copperajah |
| 10 | 4% | Alolan Sandshrew–Alolan Sandslash | Spheal–Walrein |
| 11 | 1% | Snom–Frosmoth | Duraludon |
| 12 | 1% | Duraludon | Snom–Frosmoth |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Eiscue | Arrokuda–Barraskewda |
| 2 | 30% | Arrokuda–Barraskewda | Eiscue |
| 3 | 5% | Clobbopus–Grapploct | Clobbopus–Grapploct |
| 4 | 4% | Spheal–Walrein | Spheal–Walrein |
| 5 | 1% | Eiscue | Arrokuda–Barraskewda |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Arrokuda–Barraskewda | Clobbopus–Grapploct |
| 2 | 22% | 18% | 10% | Clobbopus–Grapploct | Arrokuda–Barraskewda |
| 3 | 10% | 12% | 11% | Eiscue | Eiscue |
| 4 | 8% | 10% | 10% | Clobbopus–Grapploct | Arrokuda–Barraskewda |
| 5 | 8% | 9% | 10% | Shellder–Cloyster | Clobbopus–Grapploct |
| 6 | 4% | 7% | 10% | Arrokuda–Barraskewda | Shellder–Cloyster |
| 7 | 3% | 6% | 10% | Eiscue | Eiscue |
| 8 | 3% | 5% | 9% | Clobbopus–Grapploct | Arrokuda–Barraskewda |
| 9 | 2% | 4% | 9% | Arrokuda–Barraskewda | Spheal |
| 10 | 2% | 4% | 9% | Horsea–Kingdra | Horsea–Kingdra |

#### Altering Cave

Dungeon, Outer islands.

**`MAP_SIX_ISLAND_ALTERING_CAVE`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rolycoly–Coalossal | Cufant–Copperajah |
| 2 | 20% | Cufant–Copperajah | Rolycoly–Coalossal |
| 3 | 10% | Zubat–Golbat | Nickit–Thievul |
| 4 | 10% | Sizzlipede–Centiskorch | Zubat–Golbat |
| 5 | 10% | Rolycoly–Carkol | Rolycoly–Carkol |
| 6 | 10% | Nickit–Thievul | Misdreavus |
| 7 | 5% | Dreepy–Dragapult | Impidimp–Grimmsnarl |
| 8 | 5% | Impidimp–Grimmsnarl | Dreepy–Dragapult |
| 9 | 4% | Sizzlipede–Centiskorch | Sizzlipede–Centiskorch |
| 10 | 4% | Rolycoly–Carkol | Rolycoly–Carkol |
| 11 | 1% | Morpeko | Morpeko |
| 12 | 1% | Toxel | Toxel |

#### Tanoby Chambers

Dungeon, Outer islands.

**`MAP_SEVEN_ISLAND_TANOBY_RUINS_MONEAN_CHAMBER`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Yamask–Runerigus | Silicobra–Sandaconda |
| 2 | 20% | Silicobra–Sandaconda | Galarian Yamask–Runerigus |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Sinistea | Impidimp–Grimmsnarl |
| 5 | 10% | Galarian Corsola–Cursola | Dreepy–Dragapult |
| 6 | 10% | Silicobra–Sandaconda | Galarian Corsola–Cursola |
| 7 | 5% | Galarian Yamask | Sinistea |
| 8 | 5% | Stonjourner | Galarian Yamask |
| 9 | 4% | Galarian Corsola–Cursola | Stonjourner |
| 10 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |

**`MAP_SEVEN_ISLAND_TANOBY_RUINS_LIPTOO_CHAMBER`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Yamask–Runerigus | Sinistea |
| 2 | 20% | Sinistea | Galarian Yamask–Runerigus |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Silicobra–Sandaconda | Impidimp–Grimmsnarl |
| 5 | 10% | Rolycoly–Coalossal | Dreepy–Dragapult |
| 6 | 10% | Sinistea | Rolycoly–Coalossal |
| 7 | 5% | Galarian Yamask | Silicobra–Sandaconda |
| 8 | 5% | Stonjourner | Galarian Yamask |
| 9 | 4% | Rolycoly–Coalossal | Stonjourner |
| 10 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |

**`MAP_SEVEN_ISLAND_TANOBY_RUINS_WEEPTH_CHAMBER`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Yamask–Runerigus | Silicobra–Sandaconda |
| 2 | 20% | Silicobra–Sandaconda | Galarian Yamask–Runerigus |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Sinistea | Impidimp–Grimmsnarl |
| 5 | 10% | Nickit–Thievul | Dreepy–Dragapult |
| 6 | 10% | Silicobra–Sandaconda | Nickit–Thievul |
| 7 | 5% | Galarian Yamask | Sinistea |
| 8 | 5% | Stonjourner | Galarian Yamask |
| 9 | 4% | Nickit–Thievul | Stonjourner |
| 10 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |

**`MAP_SEVEN_ISLAND_TANOBY_RUINS_DILFORD_CHAMBER`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Yamask–Runerigus | Sinistea |
| 2 | 20% | Sinistea | Galarian Yamask–Runerigus |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Silicobra–Sandaconda | Impidimp–Grimmsnarl |
| 5 | 10% | Galarian Corsola | Dreepy–Dragapult |
| 6 | 10% | Sinistea | Galarian Corsola |
| 7 | 5% | Galarian Yamask | Silicobra–Sandaconda |
| 8 | 5% | Stonjourner | Galarian Yamask |
| 9 | 4% | Galarian Corsola | Stonjourner |
| 10 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |

**`MAP_SEVEN_ISLAND_TANOBY_RUINS_SCUFIB_CHAMBER`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Yamask–Runerigus | Silicobra–Sandaconda |
| 2 | 20% | Silicobra–Sandaconda | Galarian Yamask–Runerigus |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Sinistea | Nickit–Thievul |
| 5 | 10% | Impidimp–Grimmsnarl | Dreepy–Dragapult |
| 6 | 10% | Silicobra–Sandaconda | Impidimp–Grimmsnarl |
| 7 | 5% | Galarian Yamask | Sinistea |
| 8 | 5% | Stonjourner | Galarian Yamask |
| 9 | 4% | Impidimp–Grimmsnarl | Stonjourner |
| 10 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |

**`MAP_SEVEN_ISLAND_TANOBY_RUINS_RIXY_CHAMBER`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Yamask–Runerigus | Sinistea |
| 2 | 20% | Sinistea | Galarian Yamask–Runerigus |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Silicobra–Sandaconda | Impidimp–Grimmsnarl |
| 5 | 10% | Rolycoly–Carkol | Dreepy–Dragapult |
| 6 | 10% | Sinistea | Rolycoly–Carkol |
| 7 | 5% | Galarian Yamask | Silicobra–Sandaconda |
| 8 | 5% | Stonjourner | Galarian Yamask |
| 9 | 4% | Rolycoly–Carkol | Stonjourner |
| 10 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |

**`MAP_SEVEN_ISLAND_TANOBY_RUINS_VIAPOIS_CHAMBER`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Yamask–Runerigus | Silicobra–Sandaconda |
| 2 | 20% | Silicobra–Sandaconda | Galarian Yamask–Runerigus |
| 3 | 10% | Unown | Unown |
| 4 | 10% | Sinistea | Impidimp–Grimmsnarl |
| 5 | 10% | Galarian Meowth–Perrserker | Dreepy–Dragapult |
| 6 | 10% | Silicobra–Sandaconda | Galarian Meowth–Perrserker |
| 7 | 5% | Galarian Yamask | Sinistea |
| 8 | 5% | Stonjourner | Galarian Yamask |
| 9 | 4% | Galarian Meowth–Perrserker | Stonjourner |
| 10 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 11 | 1% | Unown | Unown |
| 12 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |

#### Mt. Ember

Dungeon, Near islands.

**`MAP_MT_EMBER_EXTERIOR`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 20% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 10% | Rookidee–Corviknight | Koffing–Galarian Weezing |
| 4 | 10% | Rolycoly–Carkol | Nickit–Thievul |
| 5 | 10% | Koffing–Galarian Weezing | Zubat–Golbat |
| 6 | 10% | Geodude–Graveler | Salandit–Salazzle |
| 7 | 5% | Cufant–Copperajah | Impidimp–Grimmsnarl |
| 8 | 5% | Slugma–Magcargo | Slugma–Magcargo |
| 9 | 4% | Scorbunny–Cinderace | Hoothoot–Noctowl |
| 10 | 4% | Salandit–Salazzle | Misdreavus |
| 11 | 1% | Toxel | Toxel |
| 12 | 1% | Stonjourner | Scorbunny–Cinderace |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 30% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 5% | Geodude–Graveler | Geodude–Graveler |
| 4 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 5 | 1% | Stonjourner | Stonjourner |

**`MAP_MT_EMBER_SUMMIT_PATH_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rolycoly–Coalossal | Koffing–Galarian Weezing |
| 2 | 20% | Koffing–Galarian Weezing | Rolycoly–Coalossal |
| 3 | 10% | Sizzlipede–Centiskorch | Zubat–Golbat |
| 4 | 10% | Geodude–Graveler | Sizzlipede–Centiskorch |
| 5 | 10% | Cufant–Copperajah | Geodude–Graveler |
| 6 | 10% | Slugma–Magcargo | Salandit–Salazzle |
| 7 | 5% | Salandit–Salazzle | Slugma–Magcargo |
| 8 | 5% | Rolycoly–Carkol | Cufant–Copperajah |
| 9 | 4% | Zubat–Golbat | Rolycoly–Carkol |
| 10 | 4% | Salandit–Salazzle | Misdreavus |
| 11 | 1% | Toxel | Toxel |
| 12 | 1% | Scorbunny–Cinderace | Scorbunny–Cinderace |

**`MAP_MT_EMBER_SUMMIT_PATH_2F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sizzlipede–Centiskorch | Koffing–Galarian Weezing |
| 2 | 20% | Koffing–Galarian Weezing | Sizzlipede–Centiskorch |
| 3 | 10% | Rolycoly–Coalossal | Rolycoly–Coalossal |
| 4 | 10% | Cufant–Copperajah | Zubat–Golbat |
| 5 | 10% | Geodude–Golem | Cufant–Copperajah |
| 6 | 10% | Slugma–Magcargo | Geodude–Golem |
| 7 | 5% | Salandit–Salazzle | Salandit–Salazzle |
| 8 | 5% | Rolycoly–Carkol | Misdreavus |
| 9 | 4% | Zubat–Golbat | Slugma–Magcargo |
| 10 | 4% | Salandit–Salazzle | Rolycoly–Carkol |
| 11 | 1% | Rolycoly–Carkol | Toxel |
| 12 | 1% | Toxel | Rolycoly–Carkol |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 30% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 5% | Geodude–Golem | Geodude–Golem |
| 4 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 5 | 1% | Rolycoly–Carkol | Rolycoly–Carkol |

**`MAP_MT_EMBER_SUMMIT_PATH_3F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Koffing–Galarian Weezing | Cufant–Copperajah |
| 2 | 20% | Cufant–Copperajah | Koffing–Galarian Weezing |
| 3 | 10% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 4 | 10% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 5 | 10% | Rolycoly–Carkol | Zubat–Golbat |
| 6 | 10% | Geodude–Golem | Geodude–Golem |
| 7 | 5% | Slugma–Magcargo | Salandit–Salazzle |
| 8 | 5% | Salandit–Salazzle | Misdreavus |
| 9 | 4% | Rolycoly–Carkol | Slugma–Magcargo |
| 10 | 4% | Geodude–Graveler | Rolycoly–Carkol |
| 11 | 1% | Toxel | Toxel |
| 12 | 1% | Scorbunny–Cinderace | Scorbunny–Cinderace |

**`MAP_MT_EMBER_RUBY_PATH_1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 20% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 10% | Cufant–Copperajah | Koffing–Galarian Weezing |
| 4 | 10% | Koffing–Galarian Weezing | Cufant–Copperajah |
| 5 | 10% | Slugma–Magcargo | Zubat–Golbat |
| 6 | 10% | Geodude–Golem | Slugma–Magcargo |
| 7 | 5% | Rolycoly–Carkol | Geodude–Golem |
| 8 | 5% | Salandit–Salazzle | Salandit–Salazzle |
| 9 | 4% | Zubat–Golbat | Misdreavus |
| 10 | 4% | Sizzlipede–Centiskorch | Rolycoly–Carkol |
| 11 | 1% | Toxel | Toxel |
| 12 | 1% | Rolycoly–Carkol | Rolycoly–Carkol |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 30% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 5% | Geodude–Graveler | Geodude–Graveler |
| 4 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 5 | 1% | Cufant–Copperajah | Cufant–Copperajah |

**`MAP_MT_EMBER_RUBY_PATH_B1F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Koffing–Galarian Weezing | Sizzlipede–Centiskorch |
| 2 | 20% | Sizzlipede–Centiskorch | Koffing–Galarian Weezing |
| 3 | 10% | Rolycoly–Coalossal | Salandit–Salazzle |
| 4 | 10% | Slugma–Magcargo | Rolycoly–Coalossal |
| 5 | 10% | Cufant–Copperajah | Slugma–Magcargo |
| 6 | 10% | Salandit–Salazzle | Cufant–Copperajah |
| 7 | 5% | Geodude–Golem | Zubat–Golbat |
| 8 | 5% | Rolycoly–Carkol | Geodude–Golem |
| 9 | 4% | Sizzlipede–Centiskorch | Misdreavus |
| 10 | 4% | Zubat–Golbat | Rolycoly–Carkol |
| 11 | 1% | Rolycoly–Carkol | Rolycoly–Carkol |
| 12 | 1% | Toxel | Toxel |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 30% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 5% | Geodude–Golem | Geodude–Golem |
| 4 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 5 | 1% | Toxel | Toxel |

**`MAP_MT_EMBER_RUBY_PATH_B1F_STAIRS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sizzlipede–Centiskorch | Cufant–Copperajah |
| 2 | 20% | Cufant–Copperajah | Sizzlipede–Centiskorch |
| 3 | 10% | Rolycoly–Coalossal | Rolycoly–Coalossal |
| 4 | 10% | Slugma–Magcargo | Koffing–Galarian Weezing |
| 5 | 10% | Koffing–Galarian Weezing | Slugma–Magcargo |
| 6 | 10% | Sizzlipede–Centiskorch | Salandit–Salazzle |
| 7 | 5% | Salandit–Salazzle | Zubat–Golbat |
| 8 | 5% | Geodude–Golem | Sizzlipede–Centiskorch |
| 9 | 4% | Rolycoly–Carkol | Geodude–Golem |
| 10 | 4% | Rolycoly–Carkol | Rolycoly–Carkol |
| 11 | 1% | Toxel | Toxel |
| 12 | 1% | Scorbunny–Cinderace | Scorbunny–Cinderace |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 30% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 5% | Geodude–Golem | Geodude–Golem |
| 4 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 5 | 1% | Cufant–Copperajah | Cufant–Copperajah |

**`MAP_MT_EMBER_RUBY_PATH_B2F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rolycoly–Coalossal | Koffing–Galarian Weezing |
| 2 | 20% | Koffing–Galarian Weezing | Rolycoly–Coalossal |
| 3 | 10% | Sizzlipede–Centiskorch | Salandit–Salazzle |
| 4 | 10% | Cufant–Copperajah | Sizzlipede–Centiskorch |
| 5 | 10% | Salandit–Salazzle | Cufant–Copperajah |
| 6 | 10% | Slugma–Magcargo | Zubat–Golbat |
| 7 | 5% | Sizzlipede–Centiskorch | Slugma–Magcargo |
| 8 | 5% | Geodude–Golem | Misdreavus |
| 9 | 4% | Rolycoly–Carkol | Rolycoly–Carkol |
| 10 | 4% | Rolycoly–Carkol | Sizzlipede–Centiskorch |
| 11 | 1% | Scorbunny–Cinderace | Scorbunny–Cinderace |
| 12 | 1% | Toxel | Toxel |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 30% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 5% | Geodude–Golem | Geodude–Golem |
| 4 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 5 | 1% | Rolycoly–Carkol | Rolycoly–Carkol |

**`MAP_MT_EMBER_RUBY_PATH_B2F_STAIRS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 2 | 20% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 3 | 10% | Salandit–Salazzle | Salandit–Salazzle |
| 4 | 10% | Cufant–Copperajah | Cufant–Copperajah |
| 5 | 10% | Koffing–Galarian Weezing | Zubat–Golbat |
| 6 | 10% | Sizzlipede–Centiskorch | Koffing–Galarian Weezing |
| 7 | 5% | Slugma–Magcargo | Sizzlipede–Centiskorch |
| 8 | 5% | Rolycoly–Carkol | Rolycoly–Carkol |
| 9 | 4% | Geodude–Golem | Misdreavus |
| 10 | 4% | Rolycoly–Carkol | Slugma–Magcargo |
| 11 | 1% | Scorbunny–Cinderace | Scorbunny–Cinderace |
| 12 | 1% | Toxel | Toxel |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 30% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 5% | Geodude–Golem | Geodude–Golem |
| 4 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 5 | 1% | Toxel | Toxel |

**`MAP_MT_EMBER_RUBY_PATH_B3F`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Rolycoly–Coalossal | Cufant–Copperajah |
| 2 | 20% | Cufant–Copperajah | Rolycoly–Coalossal |
| 3 | 10% | Sizzlipede–Centiskorch | Sizzlipede–Centiskorch |
| 4 | 10% | Salandit–Salazzle | Salandit–Salazzle |
| 5 | 10% | Sizzlipede–Centiskorch | Koffing–Galarian Weezing |
| 6 | 10% | Koffing–Galarian Weezing | Zubat–Golbat |
| 7 | 5% | Koffing–Galarian Weezing | Sizzlipede–Centiskorch |
| 8 | 5% | Slugma–Magcargo | Koffing–Galarian Weezing |
| 9 | 4% | Geodude–Golem | Misdreavus |
| 10 | 4% | Scorbunny–Cinderace | Slugma–Magcargo |
| 11 | 1% | Toxel | Toxel |
| 12 | 1% | Rolycoly–Carkol | Scorbunny–Cinderace |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Rolycoly–Coalossal | Sizzlipede–Centiskorch |
| 2 | 30% | Sizzlipede–Centiskorch | Rolycoly–Coalossal |
| 3 | 5% | Geodude–Golem | Geodude–Golem |
| 4 | 4% | Slugma–Magcargo | Slugma–Magcargo |
| 5 | 1% | Cufant–Copperajah | Cufant–Copperajah |

#### Lost Cave

Dungeon, Outer islands.

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM1`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Corsola–Cursola | Impidimp–Grimmsnarl |
| 2 | 20% | Impidimp–Grimmsnarl | Galarian Corsola–Cursola |
| 3 | 10% | Galarian Yamask | Galarian Yamask |
| 4 | 10% | Nickit–Thievul | Sinistea |
| 5 | 10% | Sinistea | Nickit–Thievul |
| 6 | 10% | Galarian Corsola–Cursola | Misdreavus |
| 7 | 5% | Zubat–Golbat | Gastly–Haunter |
| 8 | 5% | Nickit–Thievul | Nickit–Thievul |
| 9 | 4% | Galarian Yamask | Galarian Yamask |
| 10 | 4% | Impidimp–Grimmsnarl | Impidimp–Grimmsnarl |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Sinistea | Sinistea |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM2`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Impidimp–Grimmsnarl | Nickit–Thievul |
| 2 | 20% | Nickit–Thievul | Impidimp–Grimmsnarl |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Sinistea | Galarian Corsola–Cursola |
| 5 | 10% | Galarian Corsola–Cursola | Sinistea |
| 6 | 10% | Impidimp–Grimmsnarl | Misdreavus |
| 7 | 5% | Gastly–Haunter | Gastly–Haunter |
| 8 | 5% | Sinistea | Sinistea |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Nickit–Thievul | Nickit–Thievul |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Galarian Corsola–Cursola | Galarian Corsola–Cursola |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM3`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Corsola–Cursola | Impidimp–Grimmsnarl |
| 2 | 20% | Impidimp–Grimmsnarl | Galarian Corsola–Cursola |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Nickit–Thievul | Sinistea |
| 5 | 10% | Sinistea | Nickit–Thievul |
| 6 | 10% | Galarian Corsola–Cursola | Misdreavus |
| 7 | 5% | Zubat–Golbat | Gastly–Haunter |
| 8 | 5% | Nickit–Thievul | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Impidimp–Grimmsnarl | Impidimp–Grimmsnarl |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Sinistea | Sinistea |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM4`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Galarian Corsola–Cursola | Impidimp–Grimmsnarl |
| 2 | 20% | Impidimp–Grimmsnarl | Galarian Corsola–Cursola |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Nickit–Thievul | Sinistea |
| 5 | 10% | Sinistea | Nickit–Thievul |
| 6 | 10% | Galarian Corsola–Cursola | Misdreavus |
| 7 | 5% | Gastly–Haunter | Gastly–Haunter |
| 8 | 5% | Nickit–Thievul | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Impidimp–Grimmsnarl | Impidimp–Grimmsnarl |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Sinistea | Sinistea |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM5`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sinistea | Galarian Corsola–Cursola |
| 2 | 20% | Galarian Corsola–Cursola | Sinistea |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Impidimp–Grimmsnarl |
| 5 | 10% | Impidimp–Grimmsnarl | Galarian Yamask–Runerigus |
| 6 | 10% | Nickit–Thievul | Misdreavus |
| 7 | 5% | Dreepy–Dragapult | Nickit–Thievul |
| 8 | 5% | Gastly–Haunter | Dreepy–Dragapult |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Impidimp–Grimmsnarl | Gastly–Haunter |
| 11 | 1% | Sinistea | Sinistea |
| 12 | 1% | Nickit–Thievul | Impidimp–Grimmsnarl |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM6`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Sinistea | Impidimp–Grimmsnarl |
| 2 | 20% | Impidimp–Grimmsnarl | Sinistea |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Galarian Corsola–Cursola |
| 5 | 10% | Galarian Corsola–Cursola | Galarian Yamask–Runerigus |
| 6 | 10% | Nickit–Thievul | Misdreavus |
| 7 | 5% | Dreepy–Dragapult | Nickit–Thievul |
| 8 | 5% | Zubat–Golbat | Dreepy–Dragapult |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Galarian Corsola–Cursola | Gastly–Haunter |
| 11 | 1% | Sinistea | Sinistea |
| 12 | 1% | Nickit–Thievul | Galarian Corsola–Cursola |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM7`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dreepy–Dragapult | Sinistea |
| 2 | 20% | Sinistea | Dreepy–Dragapult |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Galarian Corsola–Cursola |
| 5 | 10% | Galarian Corsola–Cursola | Galarian Yamask–Runerigus |
| 6 | 10% | Impidimp–Grimmsnarl | Misdreavus |
| 7 | 5% | Nickit–Thievul | Impidimp–Grimmsnarl |
| 8 | 5% | Gastly–Haunter | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Galarian Corsola–Cursola | Gastly–Haunter |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Impidimp–Grimmsnarl | Galarian Corsola–Cursola |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM8`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dreepy–Dragapult | Galarian Corsola–Cursola |
| 2 | 20% | Galarian Corsola–Cursola | Dreepy–Dragapult |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Sinistea |
| 5 | 10% | Sinistea | Galarian Yamask–Runerigus |
| 6 | 10% | Impidimp–Grimmsnarl | Misdreavus |
| 7 | 5% | Nickit–Thievul | Impidimp–Grimmsnarl |
| 8 | 5% | Zubat–Golbat | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Sinistea | Gastly–Haunter |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Impidimp–Grimmsnarl | Sinistea |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM9`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dreepy–Dragapult | Sinistea |
| 2 | 20% | Sinistea | Dreepy–Dragapult |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Galarian Corsola–Cursola |
| 5 | 10% | Galarian Corsola–Cursola | Galarian Yamask–Runerigus |
| 6 | 10% | Impidimp–Grimmsnarl | Misdreavus |
| 7 | 5% | Nickit–Thievul | Impidimp–Grimmsnarl |
| 8 | 5% | Gastly–Haunter | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Galarian Corsola–Cursola | Gastly–Haunter |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Impidimp–Grimmsnarl | Galarian Corsola–Cursola |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM10`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dreepy–Dragapult | Galarian Corsola–Cursola |
| 2 | 20% | Galarian Corsola–Cursola | Dreepy–Dragapult |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Sinistea |
| 5 | 10% | Sinistea | Galarian Yamask–Runerigus |
| 6 | 10% | Impidimp–Grimmsnarl | Misdreavus |
| 7 | 5% | Nickit–Thievul | Impidimp–Grimmsnarl |
| 8 | 5% | Zubat–Golbat | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Sinistea | Gastly–Haunter |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Impidimp–Grimmsnarl | Sinistea |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM11`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dreepy–Dragapult | Impidimp–Grimmsnarl |
| 2 | 20% | Impidimp–Grimmsnarl | Dreepy–Dragapult |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Sinistea |
| 5 | 10% | Sinistea | Galarian Yamask–Runerigus |
| 6 | 10% | Galarian Corsola–Cursola | Misdreavus |
| 7 | 5% | Nickit–Thievul | Galarian Corsola–Cursola |
| 8 | 5% | Gastly–Haunter | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Sinistea | Gastly–Haunter |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Galarian Corsola–Cursola | Sinistea |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM12`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dreepy–Dragapult | Sinistea |
| 2 | 20% | Sinistea | Dreepy–Dragapult |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Galarian Corsola–Cursola |
| 5 | 10% | Galarian Corsola–Cursola | Galarian Yamask–Runerigus |
| 6 | 10% | Impidimp–Grimmsnarl | Misdreavus |
| 7 | 5% | Nickit–Thievul | Impidimp–Grimmsnarl |
| 8 | 5% | Zubat–Golbat | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Galarian Corsola–Cursola | Gastly–Haunter |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Impidimp–Grimmsnarl | Galarian Corsola–Cursola |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM13`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dreepy–Dragapult | Galarian Corsola–Cursola |
| 2 | 20% | Galarian Corsola–Cursola | Dreepy–Dragapult |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Sinistea |
| 5 | 10% | Sinistea | Galarian Yamask–Runerigus |
| 6 | 10% | Impidimp–Grimmsnarl | Misdreavus |
| 7 | 5% | Nickit–Thievul | Impidimp–Grimmsnarl |
| 8 | 5% | Gastly–Haunter | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Sinistea | Gastly–Haunter |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Impidimp–Grimmsnarl | Sinistea |

**`MAP_FIVE_ISLAND_LOST_CAVE_ROOM14`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Dreepy–Dragapult | Impidimp–Grimmsnarl |
| 2 | 20% | Impidimp–Grimmsnarl | Dreepy–Dragapult |
| 3 | 10% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 4 | 10% | Galarian Yamask–Runerigus | Sinistea |
| 5 | 10% | Sinistea | Galarian Yamask–Runerigus |
| 6 | 10% | Galarian Corsola–Cursola | Misdreavus |
| 7 | 5% | Nickit–Thievul | Galarian Corsola–Cursola |
| 8 | 5% | Zubat–Golbat | Nickit–Thievul |
| 9 | 4% | Galarian Yamask–Runerigus | Galarian Yamask–Runerigus |
| 10 | 4% | Sinistea | Gastly–Haunter |
| 11 | 1% | Dreepy–Dragapult | Dreepy–Dragapult |
| 12 | 1% | Galarian Corsola–Cursola | Sinistea |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Alcremie | Memorial Pillar |
| Appletun | Pattern Bush |
| Applin | Berry Forest, Bond Bridge, Pattern Bush |
| Araquanid | Berry Forest |
| Ariados | Pattern Bush |
| Arrokuda | Berry Forest, Bond Bridge, Cape Brink, Five Island and more |
| Azumarill | Berry Forest, Ruin Valley |
| Barraskewda | Berry Forest, Bond Bridge, Cape Brink, Five Island and more |
| Basculin | Cape Brink, Ruin Valley, Sevault Canyon |
| Bellsprout | Berry Forest, Bond Bridge |
| Blipbug | Berry Forest, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Boltund | Bond Bridge, Cape Brink, Five Isle Meadow, Kindle Road and more |
| Carkol | Altering Cave, Kindle Road, Mt. Ember, Sevault Canyon and more |
| Centiskorch | Altering Cave, Kindle Road, Mt. Ember, Pattern Bush |
| Chewtle | Berry Forest, Bond Bridge, Cape Brink, Five Island and more |
| Chinchou | Bond Bridge, Five Island, Kindle Road, One Island and more |
| Cinderace | Five Isle Meadow, Kindle Road, Mt. Ember, Sevault Canyon Entrance |
| Clobbopus | Bond Bridge, Five Island, Five Isle Meadow, Four Island and more |
| Cloyster | Icefall Cave |
| Coalossal | Altering Cave, Kindle Road, Mt. Ember, Sevault Canyon and more |
| Copperajah | Altering Cave, Icefall Cave, Mt. Ember, Ruin Valley and more |
| Galarian Corsola | Five Island, Five Isle Meadow, Green Path, Lost Cave and more |
| Corviknight | Bond Bridge, Cape Brink, Kindle Road, Memorial Pillar and more |
| Corvisquire | Bond Bridge, Cape Brink, Kindle Road, Memorial Pillar and more |
| Cramorant | Five Isle Meadow, Green Path, Memorial Pillar, Outcast Island and more |
| Cubone | Sevault Canyon |
| Cufant | Altering Cave, Icefall Cave, Mt. Ember, Ruin Valley and more |
| Cursola | Five Island, Five Isle Meadow, Green Path, Lost Cave and more |
| Cutiefly | Pattern Bush |
| Galarian Darmanitan | Icefall Cave |
| Galarian Darumaka | Icefall Cave |
| Dewgong | Four Island, Icefall Cave |
| Dewpider | Berry Forest |
| Donphan | Sevault Canyon, Sevault Canyon Entrance |
| Dottler | Berry Forest, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Dragapult | Altering Cave, Lost Cave, Memorial Pillar, Ruin Valley and more |
| Drakloak | Altering Cave, Lost Cave, Memorial Pillar, Ruin Valley and more |
| Drednaw | Berry Forest, Bond Bridge, Cape Brink, Five Island and more |
| Dreepy | Altering Cave, Lost Cave, Memorial Pillar, Ruin Valley and more |
| Drizzile | Berry Forest, Cape Brink, One Island, Resort Gorgeous and more |
| Drowzee | Berry Forest |
| Dubwool | Bond Bridge, Cape Brink, Five Isle Meadow, Three Isle Port |
| Duraludon | Icefall Cave, Ruin Valley, Sevault Canyon |
| Eiscue | Icefall Cave |
| Eldegoss | Berry Forest, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Exeggcute | Berry Forest |
| Falinks | Five Isle Meadow, Memorial Pillar, Pattern Bush, Ruin Valley and more |
| Galarian Farfetch'd | Sevault Canyon, Sevault Canyon Entrance, Water Path |
| Fearow | Cape Brink, Kindle Road, Treasure Beach |
| Flapple | Pattern Bush |
| Forretress | Pattern Bush |
| Frillish | Memorial Pillar |
| Frosmoth | Icefall Cave |
| Gastly | Lost Cave, Memorial Pillar |
| Geodude | Kindle Road, Mt. Ember, Sevault Canyon |
| Glalie | Icefall Cave |
| Gloom | Berry Forest, Bond Bridge |
| Golbat | Altering Cave, Lost Cave, Mt. Ember |
| Goldeen | Cape Brink, Ruin Valley |
| Golduck | Cape Brink |
| Golem | Mt. Ember, Sevault Canyon |
| Gossifleur | Berry Forest, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Grapploct | Green Path, Icefall Cave, Memorial Pillar, Outcast Island and more |
| Graveler | Kindle Road, Mt. Ember, Sevault Canyon |
| Greedent | Berry Forest, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Grimmsnarl | Altering Cave, Berry Forest, Bond Bridge, Cape Brink and more |
| Grookey | Berry Forest |
| Gyarados | Berry Forest, Bond Bridge |
| Hatenna | Berry Forest, Five Isle Meadow, Memorial Pillar, Pattern Bush and more |
| Hatterene | Berry Forest, Five Isle Meadow, Memorial Pillar, Pattern Bush and more |
| Hattrem | Berry Forest, Five Isle Meadow, Memorial Pillar, Pattern Bush and more |
| Haunter | Lost Cave, Memorial Pillar |
| Hoothoot | Bond Bridge, Cape Brink, Five Isle Meadow, Kindle Road and more |
| Hoppip | Five Isle Meadow, Memorial Pillar |
| Horsea | Bond Bridge, Icefall Cave, Kindle Road, One Island and more |
| Hypno | Berry Forest |
| Impidimp | Altering Cave, Berry Forest, Bond Bridge, Cape Brink and more |
| Indeedee | Berry Forest, Ruin Valley |
| Inteleon | Berry Forest, Cape Brink, One Island, Resort Gorgeous and more |
| Jumpluff | Five Isle Meadow, Memorial Pillar |
| Kingdra | Icefall Cave |
| Kingler | Bond Bridge, Kindle Road, One Island, Three Isle Port and more |
| Koffing | Kindle Road, Mt. Ember |
| Krabby | Bond Bridge, Kindle Road, One Island, Three Isle Port and more |
| Lanturn | Bond Bridge, Five Island, Kindle Road, One Island and more |
| Ledian | Pattern Bush |
| Ledyba | Pattern Bush |
| Galarian Linoone | Bond Bridge, Cape Brink, Five Isle Meadow, Kindle Road and more |
| Lombre | Berry Forest |
| Lotad | Berry Forest |
| Luvdisc | Resort Gorgeous |
| Magcargo | Mt. Ember |
| Magikarp | Berry Forest, Bond Bridge, Cape Brink, Kindle Road and more |
| Mareanie | Green Path, Water Path |
| Marill | Berry Forest, Ruin Valley |
| Marowak | Sevault Canyon |
| Galarian Meowth | Bond Bridge, Cape Brink, Kindle Road, Sevault Canyon and more |
| Milcery | Berry Forest, Bond Bridge, Five Isle Meadow, Memorial Pillar and more |
| Misdreavus | Altering Cave, Lost Cave, Memorial Pillar, Mt. Ember |
| Morelull | Berry Forest, Pattern Bush |
| Morgrem | Altering Cave, Berry Forest, Bond Bridge, Cape Brink and more |
| Morpeko | Altering Cave, Cape Brink, Five Isle Meadow, Ruin Valley and more |
| Galarian Mr. Mime | Icefall Cave |
| Mr. Rime | Icefall Cave |
| Natu | Ruin Valley |
| Nickit | Altering Cave, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Alolan Ninetales | Icefall Cave |
| Noctowl | Bond Bridge, Cape Brink, Five Isle Meadow, Kindle Road and more |
| Obstagoon | Bond Bridge, Cape Brink, Five Isle Meadow, Kindle Road and more |
| Oddish | Berry Forest, Bond Bridge |
| Orbeetle | Berry Forest, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Pelipper | Bond Bridge, Five Island, Five Isle Meadow, One Island and more |
| Perrserker | Bond Bridge, Cape Brink, Kindle Road, Sevault Canyon and more |
| Phanpy | Sevault Canyon, Sevault Canyon Entrance |
| Pidgeotto | Three Isle Port |
| Pidgey | Three Isle Port |
| Piloswine | Icefall Cave |
| Pincurchin | Bond Bridge, Five Island, Five Isle Meadow, Green Path and more |
| Pineco | Pattern Bush |
| Poliwag | Berry Forest, Cape Brink, Ruin Valley |
| Poliwhirl | Berry Forest, Cape Brink, Ruin Valley |
| Ponyta | Kindle Road |
| Galarian Ponyta | Berry Forest, Five Isle Meadow, Pattern Bush |
| Psyduck | Cape Brink |
| Pyukumuku | Five Isle Meadow, Tanoby Ruins, Trainer Tower grounds, Water Path |
| Quagsire | Ruin Valley |
| Qwilfish | Five Island, Trainer Tower grounds |
| Raboot | Five Isle Meadow, Kindle Road, Mt. Ember, Sevault Canyon Entrance |
| Rapidash | Kindle Road |
| Galarian Rapidash | Berry Forest, Five Isle Meadow, Pattern Bush |
| Raticate | Kindle Road, Three Isle Port |
| Rattata | Kindle Road, Three Isle Port |
| Remoraid | Bond Bridge |
| Ribombee | Pattern Bush |
| Rillaboom | Berry Forest |
| Rolycoly | Altering Cave, Kindle Road, Mt. Ember, Sevault Canyon and more |
| Rookidee | Bond Bridge, Cape Brink, Kindle Road, Memorial Pillar and more |
| Runerigus | Lost Cave, Memorial Pillar, Ruin Valley, Sevault Canyon and more |
| Salandit | Mt. Ember |
| Salazzle | Mt. Ember |
| Sandaconda | Ruin Valley, Sevault Canyon, Sevault Canyon Entrance, Tanoby Chambers and more |
| Alolan Sandshrew | Icefall Cave |
| Alolan Sandslash | Icefall Cave |
| Scolipede | Pattern Bush |
| Scorbunny | Five Isle Meadow, Kindle Road, Mt. Ember, Sevault Canyon Entrance |
| Seadra | Bond Bridge, Icefall Cave, Kindle Road, One Island and more |
| Seaking | Cape Brink, Ruin Valley |
| Sealeo | Four Island, Icefall Cave |
| Seel | Four Island, Icefall Cave |
| Shellder | Four Island, Icefall Cave, Kindle Road, One Island and more |
| Shiinotic | Berry Forest, Pattern Bush |
| Silicobra | Ruin Valley, Sevault Canyon, Sevault Canyon Entrance, Tanoby Chambers and more |
| Sinistea | Five Isle Meadow, Lost Cave, Memorial Pillar, Ruin Valley and more |
| Sirfetch'd | Sevault Canyon |
| Sizzlipede | Altering Cave, Kindle Road, Mt. Ember, Pattern Bush |
| Skiploom | Five Isle Meadow, Memorial Pillar |
| Skwovet | Berry Forest, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Slowbro | Treasure Beach |
| Galarian Slowbro | Green Path |
| Galarian Slowking | Green Path |
| Slowpoke | Treasure Beach |
| Galarian Slowpoke | Green Path, Sevault Canyon, Water Path |
| Slugma | Mt. Ember |
| Snom | Icefall Cave |
| Snorunt | Icefall Cave |
| Sobble | Berry Forest, Cape Brink, One Island, Resort Gorgeous and more |
| Spearow | Cape Brink, Kindle Road, Treasure Beach |
| Spheal | Four Island, Icefall Cave |
| Spinarak | Pattern Bush |
| Staryu | Bond Bridge, Kindle Road, Treasure Beach |
| Stonjourner | Mt. Ember, Ruin Valley, Sevault Canyon, Tanoby Chambers and more |
| Galarian Stunfisk | Cape Brink, Ruin Valley, Sevault Canyon |
| Swinub | Icefall Cave |
| Tentacool | Bond Bridge, Four Island, Kindle Road, One Island and more |
| Tentacruel | Bond Bridge, Kindle Road, One Island, Treasure Beach |
| Thievul | Altering Cave, Bond Bridge, Cape Brink, Five Isle Meadow and more |
| Thwackey | Berry Forest |
| Toxapex | Green Path, Water Path |
| Toxel | Altering Cave, Cape Brink, Kindle Road, Mt. Ember and more |
| Unown | Tanoby Chambers |
| Vanillish | Icefall Cave |
| Vanillite | Icefall Cave |
| Vanilluxe | Icefall Cave |
| Venipede | Pattern Bush |
| Venomoth | Berry Forest, Bond Bridge, Pattern Bush |
| Venonat | Berry Forest, Bond Bridge, Pattern Bush |
| Alolan Vulpix | Icefall Cave |
| Wailmer | Five Isle Meadow, Outcast Island, Resort Gorgeous, Tanoby Ruins and more |
| Walrein | Four Island, Icefall Cave |
| Weepinbell | Berry Forest, Bond Bridge |
| Galarian Weezing | Kindle Road, Mt. Ember |
| Whirlipede | Pattern Bush |
| Wingull | Bond Bridge, Five Island, Five Isle Meadow, One Island and more |
| Wooloo | Bond Bridge, Cape Brink, Five Isle Meadow, Three Isle Port |
| Wooper | Ruin Valley |
| Xatu | Ruin Valley |
| Galarian Yamask | Lost Cave, Memorial Pillar, Ruin Valley, Sevault Canyon and more |
| Yamper | Bond Bridge, Cape Brink, Five Isle Meadow, Kindle Road and more |
| Galarian Zigzagoon | Bond Bridge, Cape Brink, Five Isle Meadow, Kindle Road and more |
| Zubat | Altering Cave, Lost Cave, Mt. Ember |
