# Kanto encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Kanto and Johto encounter rules](kanto-johto-encounters.md). Every pick is a
placeholder for playtesting.

## Scope

This spec lists Kanto's wild-encounter tables: 63 maps, each with a day and a
night table for every method it has. The Safari Zone is deferred and not
listed. Legendaries and mythicals belong to a later spec.

## Behavior

### Reading the tables

- Places are ordered by reach: Road, Wilds, Outlands, then dungeons. Each place
  names its reach and its Gen I–II band.
- Each method's slots are grouped into tiers by their weight:

| Method | Common | Uncommon | Rare | Very rare |
| --- | --- | --- | --- | --- |
| Land | Slots 1–2 (20% each) | Slots 3–6 (10% each) | Slots 7–10 (4–5% each) | Slots 11–12 (1% each) |
| Surfing, trees and rocks | Slot 1 (60%) | Slot 2 (30%) | Slots 3–4 (5% and 4%) | Slot 5 (1%) |
| Fishing | Entries 1–2 | Entries 3–5 | Entries 6–8 | Entries 9–10 |

- Fishing uses the Standard Rod's ten entries, which are weighted differently
  by rod quality.
- A line is written from its base to its stage cap, such as Pidgey–Pidgeot. The
  game picks the stage from the level. A single name is a single-stage species,
  a baby, or a line capped at that stage.
- "×2" means the species fills two slots in that tier.

### Tables

#### Pallet Town

Road, Kanto west.

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp, Horsea–Seadra | Magikarp, Horsea–Seadra |
| Uncommon | Tentacool–Tentacruel, Horsea–Seadra, Krabby–Kingler | Tentacool–Tentacruel, Chinchou–Lanturn, Krabby–Kingler |
| Rare | Remoraid–Octillery, Staryu, Magikarp–Gyarados | Remoraid–Octillery, Staryu, Magikarp–Gyarados |
| Very rare | Shellder, Squirtle–Blastoise | Chinchou–Lanturn, Squirtle–Blastoise |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot | Pidgey–Pidgeot |
| Uncommon | Caterpie–Butterfree | Hoothoot–Noctowl |
| Rare | Ledyba–Ledian, Exeggcute | Spinarak–Ariados, Exeggcute |
| Very rare | Pineco | Venonat–Venomoth |

#### Viridian City

Road, Kanto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Nidoran♀–Nidorina, Nidoran♂–Nidorino | Nidoran♀–Nidorina, Nidoran♂–Nidorino |
| Uncommon | Spearow–Fearow, Rattata–Raticate, Ledyba–Ledian, Mankey–Primeape | Hoothoot–Noctowl, Oddish–Gloom, Spinarak–Ariados, Meowth–Persian |
| Rare | Pidgey–Pidgeot, Pikachu, Caterpie–Butterfree, Sentret–Furret | Venonat–Venomoth, Rattata–Raticate, Mankey–Primeape, Oddish–Gloom |
| Very rare | Eevee, Pichu | Eevee, Pichu |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Marill–Azumarill, Psyduck–Golduck | Goldeen–Seaking, Marill–Azumarill |
| Very rare | Slowpoke–Slowbro | Psyduck–Golduck |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp, Goldeen–Seaking, Marill–Azumarill | Magikarp, Goldeen–Seaking, Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Goldeen–Seaking | Marill–Azumarill, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Slowpoke–Slowbro, Magikarp–Gyarados | Slowpoke–Slowbro, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Weedle–Beedrill | Weedle–Beedrill |
| Uncommon | Spearow–Fearow | Spearow–Fearow |
| Rare | Caterpie–Butterfree, Geodude–Graveler | Hoothoot–Noctowl, Venonat–Venomoth |
| Very rare | Exeggcute | Caterpie–Butterfree |

#### Pewter City

Road, Kanto west.

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler | Geodude–Graveler |
| Uncommon | Geodude–Graveler | Geodude–Graveler |
| Rare | Sandshrew–Sandslash, Dunsparce | Spinarak–Ariados, Dunsparce |
| Very rare | Spearow–Fearow | Sandshrew–Sandslash |

#### Cerulean City

Road, Kanto east.

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking | Goldeen–Seaking |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Psyduck–Golduck, Staryu | Slowpoke–Slowbro, Staryu |
| Very rare | Squirtle–Blastoise | Squirtle–Blastoise |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Magikarp, Psyduck–Golduck, Poliwag–Poliwhirl | Magikarp, Psyduck–Golduck, Slowpoke–Slowbro |
| Rare | Staryu, Magikarp–Gyarados, Krabby–Kingler | Staryu, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Goldeen–Seaking, Magikarp–Gyarados | Goldeen–Seaking, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Caterpie–Butterfree | Caterpie–Butterfree |
| Uncommon | Weedle–Beedrill | Weedle–Beedrill |
| Rare | Pineco, Exeggcute | Hoothoot–Noctowl, Venonat–Venomoth |
| Very rare | Geodude–Graveler | Exeggcute |

#### Vermilion City

Road, Kanto east.

**`MAP_VERMILION_CITY_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Meowth–Persian, Magnemite–Magneton | Meowth–Persian, Magnemite–Magneton |
| Uncommon | Drowzee–Hypno, Spearow–Fearow, Pidgey–Pidgeot, Diglett–Dugtrio | Drowzee–Hypno, Zubat–Golbat, Gastly–Haunter, Hoothoot–Noctowl |
| Rare | Squirtle–Blastoise, Voltorb–Electrode, Farfetch'd, Sandshrew–Sandslash | Squirtle–Blastoise, Voltorb–Electrode, Farfetch'd, Ekans–Arbok |
| Very rare | Squirtle–Blastoise, Elekid | Squirtle–Blastoise, Elekid |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Rare | Krabby–Kingler, Staryu | Chinchou–Lanturn, Staryu |
| Very rare | Shellder | Krabby–Kingler |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler ×2 | Krabby–Kingler ×2 |
| Uncommon | Magikarp, Horsea–Seadra, Tentacool–Tentacruel | Magikarp, Horsea–Seadra, Chinchou–Lanturn |
| Rare | Chinchou–Lanturn, Magikarp–Gyarados, Shellder | Chinchou–Lanturn, Magikarp–Gyarados, Shellder |
| Very rare | Horsea–Seadra, Staryu | Chinchou–Lanturn, Staryu |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler | Krabby–Kingler |
| Uncommon | Krabby–Kingler | Krabby–Kingler |
| Rare | Geodude–Graveler, Pidgey–Pidgeot | Geodude–Graveler, Hoothoot–Noctowl |
| Very rare | Spearow–Fearow | Spearow–Fearow |

**`MAP_VERMILION_CITY_PORT_OUTSIDE_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Machop–Machoke, Voltorb–Electrode | Machop–Machoke, Voltorb–Electrode |
| Uncommon | Spearow–Fearow, Meowth–Persian, Pidgey–Pidgeot, Drowzee–Hypno | Zubat–Golbat, Hoothoot–Noctowl, Gastly–Haunter, Meowth–Persian |
| Rare | Grimer–Muk, Magnemite–Magneton, Farfetch'd, Pikachu | Murkrow, Magnemite–Magneton, Farfetch'd, Grimer–Muk |
| Very rare | Squirtle–Blastoise, Elekid | Squirtle–Blastoise, Elekid |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Rare | Krabby–Kingler, Staryu | Chinchou–Lanturn ×2 |
| Very rare | Horsea–Seadra | Staryu |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler ×2 | Krabby–Kingler ×2 |
| Uncommon | Magikarp, Tentacool–Tentacruel, Chinchou–Lanturn | Magikarp, Tentacool–Tentacruel, Chinchou–Lanturn |
| Rare | Horsea–Seadra, Magikarp–Gyarados, Shellder | Horsea–Seadra, Magikarp–Gyarados, Chinchou–Lanturn |
| Very rare | Staryu, Horsea–Seadra | Staryu, Chinchou–Lanturn |

#### Lavender Town

Road, Kanto east.

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot | Pidgey–Pidgeot |
| Uncommon | Pidgey–Pidgeot | Murkrow |
| Rare | Geodude–Graveler, Exeggcute | Geodude–Graveler, Gastly–Haunter |
| Very rare | Caterpie–Butterfree | Exeggcute |

#### Celadon City

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Oddish–Gloom, Bellsprout–Weepinbell | Oddish–Gloom, Bellsprout–Weepinbell |
| Uncommon | Meowth–Persian, Grimer–Muk, Koffing–Weezing, Pidgey–Pidgeot | Gastly–Haunter, Murkrow, Meowth–Persian, Grimer–Muk |
| Rare | Tangela, Exeggcute, Caterpie–Butterfree, Jigglypuff | Venonat–Venomoth, Hoothoot–Noctowl, Koffing–Weezing, Houndour |
| Very rare | Eevee, Porygon | Eevee, Porygon |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Grimer–Muk | Grimer–Muk ×2 |
| Very rare | Koffing–Weezing | Poliwag–Poliwhirl |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking, Poliwag–Poliwhirl | Goldeen–Seaking, Poliwag–Poliwhirl |
| Uncommon | Magikarp, Slowpoke–Slowbro, Goldeen–Seaking | Magikarp, Slowpoke–Slowbro, Psyduck–Golduck |
| Rare | Grimer–Muk, Magikarp–Gyarados, Psyduck–Golduck | Grimer–Muk, Magikarp–Gyarados, Psyduck–Golduck |
| Very rare | Grimer–Muk, Magikarp–Gyarados | Grimer–Muk, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Exeggcute | Exeggcute |
| Uncommon | Exeggcute | Exeggcute |
| Rare | Caterpie–Butterfree, Geodude–Graveler | Venonat–Venomoth, Hoothoot–Noctowl |
| Very rare | Weedle–Beedrill | Murkrow |

#### Saffron City

Road, Kanto east.

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot | Pidgey–Pidgeot |
| Uncommon | Pidgey–Pidgeot | Hoothoot–Noctowl |
| Rare | Abra–Kadabra, Exeggcute | Abra–Kadabra, Exeggcute |
| Very rare | Geodude–Graveler | Murkrow |

#### Fuchsia City

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Venonat–Venomoth, Exeggcute | Venonat–Venomoth, Exeggcute |
| Uncommon | Nidoran♀–Nidorina, Nidoran♂–Nidorino, Doduo–Dodrio, Paras–Parasect | Zubat–Golbat, Oddish–Gloom, Nidoran♀–Nidorina, Nidoran♂–Nidorino |
| Rare | Rhyhorn, Koffing–Weezing, Farfetch'd, Tangela | Gastly–Haunter, Koffing–Weezing, Hoothoot–Noctowl, Spinarak–Ariados |
| Very rare | Farfetch'd, Exeggcute | Farfetch'd, Hoothoot–Noctowl |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Slowpoke–Slowbro, Goldeen–Seaking | Poliwag–Poliwhirl, Slowpoke–Slowbro |
| Very rare | Poliwag–Poliwhirl | Goldeen–Seaking |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Magikarp ×2, Poliwag–Poliwhirl | Magikarp, Poliwag–Poliwhirl, Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Slowpoke–Slowbro | Psyduck–Golduck, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Goldeen–Seaking, Magikarp–Gyarados | Goldeen–Seaking, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Venonat–Venomoth | Venonat–Venomoth |
| Uncommon | Exeggcute | Exeggcute |
| Rare | Paras–Parasect, Geodude–Graveler | Spinarak–Ariados, Paras–Parasect |
| Very rare | Caterpie–Butterfree | Hoothoot–Noctowl |

#### Cinnabar Island

Road, Kanto west.

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Corsola | Corsola |
| Rare | Staryu, Shellder | Chinchou–Lanturn, Staryu |
| Very rare | Tentacool–Tentacruel | Shellder |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler ×2 | Krabby–Kingler ×2 |
| Uncommon | Corsola, Horsea–Seadra, Shellder | Corsola, Horsea–Seadra, Chinchou–Lanturn |
| Rare | Magikarp, Staryu, Magikarp–Gyarados | Magikarp, Staryu, Magikarp–Gyarados |
| Very rare | Remoraid–Octillery, Horsea–Seadra | Chinchou–Lanturn, Horsea–Seadra |

#### Route 1

Road, Kanto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot, Rattata–Raticate | Rattata–Raticate, Hoothoot–Noctowl |
| Uncommon | Sentret–Furret ×2, Pidgey–Pidgeot, Rattata–Raticate | Hoothoot–Noctowl, Rattata–Raticate, Meowth–Persian, Oddish–Gloom |
| Rare | Caterpie–Butterfree, Ledyba–Ledian, Nidoran♀–Nidorina, Nidoran♂–Nidorino | Pidgey–Pidgeot, Spinarak–Ariados, Venonat–Venomoth, Nidoran♂–Nidorino |
| Very rare | Eevee, Igglybuff | Eevee, Igglybuff |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Marill–Azumarill, Poliwag–Poliwhirl | Goldeen–Seaking, Marill–Azumarill |
| Very rare | Slowpoke–Slowbro | Poliwag–Poliwhirl |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp ×2 | Magikarp ×2 |
| Uncommon | Poliwag–Poliwhirl, Psyduck–Golduck, Marill–Azumarill | Poliwag–Poliwhirl, Psyduck–Golduck, Slowpoke–Slowbro |
| Rare | Goldeen–Seaking, Magikarp–Gyarados, Psyduck–Golduck | Marill–Azumarill, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Slowpoke–Slowbro, Magikarp–Gyarados | Slowpoke–Slowbro, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot | Pidgey–Pidgeot |
| Uncommon | Pidgey–Pidgeot | Hoothoot–Noctowl |
| Rare | Exeggcute, Caterpie–Butterfree | Exeggcute, Venonat–Venomoth |
| Very rare | Pineco | Spinarak–Ariados |

#### Route 2

Road, Kanto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Caterpie–Butterfree, Weedle–Beedrill | Caterpie–Butterfree, Weedle–Beedrill |
| Uncommon | Pidgey–Pidgeot, Rattata–Raticate, Ledyba–Ledian, Paras–Parasect | Hoothoot–Noctowl, Spinarak–Ariados, Venonat–Venomoth, Rattata–Raticate |
| Rare | Pikachu, Abra–Kadabra, Yanma, Caterpie–Metapod | Paras–Parasect, Pidgey–Pidgeot, Oddish–Gloom, Murkrow |
| Very rare | Abra–Kadabra, Pichu | Abra–Kadabra, Pichu |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Marill–Azumarill | Goldeen–Seaking, Psyduck–Golduck |
| Very rare | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Magikarp, Poliwag–Poliwhirl, Marill–Azumarill | Magikarp, Poliwag–Poliwhirl, Slowpoke–Slowbro |
| Rare | Slowpoke–Slowbro, Magikarp–Gyarados, Poliwag–Poliwhirl | Slowpoke–Slowbro ×2, Magikarp–Gyarados |
| Very rare | Psyduck–Golduck, Magikarp–Gyarados | Psyduck–Golduck, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Weedle–Beedrill | Weedle–Beedrill |
| Uncommon | Caterpie–Butterfree | Caterpie–Butterfree |
| Rare | Pineco, Exeggcute | Hoothoot–Noctowl, Spinarak–Ariados |
| Very rare | Geodude–Graveler | Exeggcute |

#### Route 3

Road, Kanto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow, Jigglypuff | Spearow–Fearow, Jigglypuff |
| Uncommon | Nidoran♀–Nidorina, Nidoran♂–Nidorino, Mankey–Primeape, Sandshrew–Sandslash | Zubat–Golbat, Meowth–Persian, Nidoran♀–Nidorina, Oddish–Gloom |
| Rare | Ekans–Arbok, Sunkern, Rattata–Raticate, Jigglypuff | Clefairy, Nidoran♂–Nidorino, Ekans–Arbok, Sandshrew–Sandslash |
| Very rare | Charmander–Charizard, Igglybuff | Charmander–Charizard, Igglybuff |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill | Marill–Azumarill |
| Uncommon | Marill–Azumarill | Marill–Azumarill |
| Rare | Psyduck–Golduck, Poliwag–Poliwhirl | Goldeen–Seaking, Psyduck–Golduck |
| Very rare | Slowpoke–Slowbro | Poliwag–Poliwhirl |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp, Marill–Azumarill | Magikarp, Marill–Azumarill |
| Uncommon | Poliwag–Poliwhirl, Marill–Azumarill, Goldeen–Seaking | Poliwag–Poliwhirl, Marill–Azumarill, Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Poliwag–Poliwhirl | Psyduck–Golduck, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Goldeen–Seaking, Magikarp–Gyarados | Goldeen–Seaking, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow | Spearow–Fearow |
| Uncommon | Spearow–Fearow | Spearow–Fearow |
| Rare | Geodude–Graveler, Sandshrew–Sandslash | Hoothoot–Noctowl, Geodude–Graveler |
| Very rare | Exeggcute | Spinarak–Ariados |

#### Route 4

Road, Kanto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ekans–Arbok, Sandshrew–Sandslash | Ekans–Arbok, Sandshrew–Sandslash |
| Uncommon | Rattata–Raticate, Spearow–Fearow, Mankey–Primeape, Jigglypuff | Zubat–Golbat, Hoothoot–Noctowl, Meowth–Persian, Rattata–Raticate |
| Rare | Pidgey–Pidgeot, Geodude–Graveler, Charmander–Charizard, Nidoran♂–Nidorino | Oddish–Gloom, Mankey–Primeape, Charmander–Charizard, Jigglypuff |
| Very rare | Abra–Kadabra, Clefairy | Clefairy, Abra–Kadabra |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Rare | Remoraid–Octillery, Staryu | Chinchou–Lanturn, Remoraid–Octillery |
| Very rare | Shellder | Staryu |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra ×2 | Horsea–Seadra ×2 |
| Uncommon | Remoraid–Octillery, Krabby–Kingler, Shellder | Remoraid–Octillery, Krabby–Kingler, Chinchou–Lanturn |
| Rare | Magikarp, Magikarp–Gyarados, Krabby–Kingler | Magikarp, Magikarp–Gyarados, Chinchou–Lanturn |
| Very rare | Staryu, Magikarp–Gyarados | Staryu, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Sandshrew–Sandslash | Sandshrew–Sandslash |
| Uncommon | Sandshrew–Sandslash | Sandshrew–Sandslash |
| Rare | Geodude–Graveler, Dunsparce | Hoothoot–Noctowl, Dunsparce |
| Very rare | Exeggcute | Geodude–Graveler |

#### Route 5

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot, Meowth–Persian | Meowth–Persian, Pidgey–Pidgeot |
| Uncommon | Jigglypuff, Abra–Kadabra, Oddish–Gloom, Rattata–Raticate | Hoothoot–Noctowl, Venonat–Venomoth, Jigglypuff, Gastly–Haunter |
| Rare | Snubbull–Granbull, Abra–Kadabra, Pidgey–Pidgeot, Jigglypuff | Abra–Kadabra, Snubbull–Granbull, Drowzee, Oddish–Gloom |
| Very rare | Mime Jr., Eevee | Mime Jr., Clefairy |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Psyduck–Golduck, Goldeen–Seaking | Slowpoke–Slowbro, Psyduck–Golduck |
| Very rare | Slowpoke–Slowbro | Slowpoke–Slowbro |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp ×2, Goldeen–Seaking | Magikarp, Goldeen–Seaking, Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Goldeen–Seaking | Psyduck–Golduck, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Slowpoke–Slowbro, Magikarp–Gyarados | Slowpoke–Slowbro, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot | Pidgey–Pidgeot |
| Uncommon | Pidgey–Pidgeot | Hoothoot–Noctowl |
| Rare | Exeggcute, Caterpie–Butterfree | Exeggcute, Venonat–Venomoth |
| Very rare | Pineco | Pidgey–Pidgeot |

#### Route 6

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Oddish–Gloom, Magnemite–Magneton | Oddish–Gloom, Magnemite–Magneton |
| Uncommon | Pidgey–Pidgeot, Meowth–Persian, Bellsprout–Weepinbell, Rattata–Raticate | Meowth–Persian, Venonat–Venomoth, Drowzee, Hoothoot–Noctowl |
| Rare | Abra–Kadabra, Snubbull–Granbull, Farfetch'd, Jigglypuff | Abra–Kadabra, Snubbull–Granbull, Farfetch'd, Zubat–Golbat |
| Very rare | Abra–Kadabra, Elekid | Abra–Kadabra, Elekid |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Slowpoke–Slowbro, Squirtle–Blastoise | Slowpoke–Slowbro, Poliwag–Poliwhirl |
| Very rare | Slowpoke–Slowbro | Squirtle–Blastoise |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck, Krabby–Kingler | Psyduck–Golduck, Krabby–Kingler |
| Uncommon | Magikarp ×2, Poliwag–Poliwhirl | Magikarp ×2, Poliwag–Poliwhirl |
| Rare | Goldeen–Seaking, Magikarp–Gyarados, Krabby–Kingler | Slowpoke–Slowbro, Magikarp–Gyarados, Krabby–Kingler |
| Very rare | Slowpoke–Slowbro, Magikarp–Gyarados | Slowpoke–Slowbro, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Caterpie–Butterfree | Caterpie–Butterfree |
| Uncommon | Weedle–Beedrill | Weedle–Beedrill |
| Rare | Ledyba–Ledian, Geodude–Graveler | Hoothoot–Noctowl, Venonat–Venomoth |
| Very rare | Exeggcute | Exeggcute |

#### Route 7

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Meowth–Persian, Vulpix | Meowth–Persian, Vulpix |
| Uncommon | Growlithe, Pidgey–Pidgeot, Jigglypuff, Oddish–Gloom | Murkrow, Houndour, Gastly–Haunter, Growlithe |
| Rare | Snubbull–Granbull, Rattata–Raticate, Abra–Kadabra, Eevee | Murkrow, Hoothoot–Noctowl, Abra–Kadabra, Houndour |
| Very rare | Eevee, Pichu | Eevee, Pichu |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Psyduck–Golduck, Slowpoke–Slowbro | Goldeen–Seaking, Slowpoke–Slowbro |
| Very rare | Goldeen–Seaking | Goldeen–Seaking |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp ×2, Goldeen–Seaking | Magikarp, Goldeen–Seaking, Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Goldeen–Seaking | Psyduck–Golduck, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Poliwag–Poliwhirl, Magikarp–Gyarados | Poliwag–Poliwhirl, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Exeggcute | Exeggcute |
| Uncommon | Exeggcute | Exeggcute |
| Rare | Pidgey–Pidgeot, Pineco | Murkrow, Venonat–Venomoth |
| Very rare | Geodude–Graveler | Hoothoot–Noctowl |

#### Route 8

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Growlithe, Ekans–Arbok | Growlithe, Ekans–Arbok |
| Uncommon | Vulpix, Sandshrew–Sandslash, Pidgey–Pidgeot, Meowth–Persian | Gastly–Haunter ×2, Hoothoot–Noctowl, Houndour |
| Rare | Abra–Kadabra ×2, Snubbull–Granbull, Ponyta | Meowth–Persian, Snubbull–Granbull, Abra–Kadabra, Houndour |
| Very rare | Growlithe, Ponyta | Vulpix, Gastly |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking | Goldeen–Seaking |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Psyduck–Golduck, Poliwag–Poliwhirl | Psyduck–Golduck, Slowpoke–Slowbro |
| Very rare | Slowpoke–Slowbro | Slowpoke–Slowbro |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Magikarp ×2, Poliwag–Poliwhirl | Magikarp, Poliwag–Poliwhirl, Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Poliwag–Poliwhirl | Psyduck–Golduck, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Goldeen–Seaking, Magikarp–Gyarados | Goldeen–Seaking, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Sandshrew–Sandslash | Sandshrew–Sandslash |
| Uncommon | Sandshrew–Sandslash | Sandshrew–Sandslash |
| Rare | Pidgey–Pidgeot, Exeggcute | Hoothoot–Noctowl, Exeggcute |
| Very rare | Geodude–Graveler | Venonat–Venomoth |

#### Route 11

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Drowzee, Spearow–Fearow | Drowzee, Spearow–Fearow |
| Uncommon | Rattata–Raticate, Ekans–Arbok, Sandshrew–Sandslash, Diglett | Rattata–Raticate, Hoothoot–Noctowl, Meowth–Persian, Zubat–Golbat |
| Rare | Magnemite–Magneton ×2, Drowzee–Hypno, Farfetch'd | Gastly–Haunter, Drowzee–Hypno, Farfetch'd, Venonat–Venomoth |
| Very rare | Farfetch'd, Elekid | Farfetch'd, Elekid |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler | Krabby–Kingler |
| Uncommon | Krabby–Kingler | Krabby–Kingler |
| Rare | Tentacool–Tentacruel, Shellder | Tentacool–Tentacruel, Chinchou–Lanturn |
| Very rare | Horsea–Seadra | Shellder |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler ×2 | Krabby–Kingler ×2 |
| Uncommon | Magikarp, Horsea–Seadra, Shellder | Magikarp, Horsea–Seadra, Chinchou–Lanturn |
| Rare | Horsea–Seadra, Magikarp–Gyarados, Tentacool–Tentacruel | Horsea–Seadra, Magikarp–Gyarados, Chinchou–Lanturn |
| Very rare | Krabby–Kingler, Staryu | Krabby–Kingler, Staryu |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow | Spearow–Fearow |
| Uncommon | Spearow–Fearow | Spearow–Fearow |
| Rare | Geodude–Graveler, Pidgey–Pidgeot | Hoothoot–Noctowl, Geodude–Graveler |
| Very rare | Exeggcute | Spinarak–Ariados |

#### Route 16

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow, Doduo–Dodrio | Spearow–Fearow, Doduo–Dodrio |
| Uncommon | Rattata–Raticate, Grimer, Doduo–Dodrio, Ponyta | Grimer–Muk, Murkrow, Rattata–Raticate, Zubat–Golbat |
| Rare | Spearow–Fearow, Rattata–Raticate, Lickitung, Ponyta–Rapidash | Hoothoot–Noctowl, Houndour, Lickitung, Murkrow |
| Very rare | Lickitung, Eevee | Lickitung, Eevee |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Psyduck–Golduck, Slowpoke–Slowbro | Goldeen–Seaking, Psyduck–Golduck |
| Very rare | Goldeen–Seaking | Slowpoke–Slowbro |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp ×2, Goldeen–Seaking | Magikarp, Goldeen–Seaking, Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Goldeen–Seaking | Psyduck–Golduck, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Poliwag–Poliwhirl, Magikarp–Gyarados | Poliwag–Poliwhirl, Magikarp–Gyarados |

#### Route 17

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Doduo–Dodrio, Spearow–Fearow | Doduo–Dodrio, Spearow–Fearow |
| Uncommon | Rattata–Raticate, Ponyta, Spearow–Fearow, Ponyta–Rapidash | Grimer–Muk, Murkrow, Zubat–Golbat, Hoothoot–Noctowl |
| Rare | Doduo–Dodrio, Grimer, Rattata–Raticate, Ponyta | Houndour, Hoothoot–Noctowl, Grimer–Muk, Grimer |
| Very rare | Lickitung, Farfetch'd | Lickitung, Farfetch'd |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Poliwag–Poliwhirl, Slowpoke–Slowbro | Slowpoke–Slowbro ×2 |
| Very rare | Goldeen–Seaking | Poliwag–Poliwhirl |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp, Psyduck–Golduck | Magikarp, Psyduck–Golduck |
| Uncommon | Poliwag–Poliwhirl, Magikarp, Goldeen–Seaking | Poliwag–Poliwhirl, Magikarp, Slowpoke–Slowbro |
| Rare | Slowpoke–Slowbro, Magikarp–Gyarados, Poliwag–Poliwhirl | Slowpoke–Slowbro ×2, Magikarp–Gyarados |
| Very rare | Psyduck–Golduck, Magikarp–Gyarados | Psyduck–Golduck, Magikarp–Gyarados |

#### Route 18

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow, Rattata–Raticate | Spearow–Fearow, Rattata–Raticate |
| Uncommon | Doduo–Dodrio ×2, Grimer, Spearow–Fearow | Grimer–Muk, Murkrow, Zubat–Golbat, Hoothoot–Noctowl |
| Rare | Rattata–Raticate, Lickitung, Grimer, Grimer–Muk | Houndour, Lickitung, Hoothoot–Noctowl, Grimer |
| Very rare | Lickitung, Eevee | Lickitung, Eevee |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra | Horsea–Seadra |
| Uncommon | Horsea–Seadra | Horsea–Seadra |
| Rare | Tentacool–Tentacruel, Krabby–Kingler | Chinchou–Lanturn, Tentacool–Tentacruel |
| Very rare | Shellder | Krabby–Kingler |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra ×2 | Horsea–Seadra ×2 |
| Uncommon | Magikarp, Krabby–Kingler, Shellder | Magikarp, Krabby–Kingler, Chinchou–Lanturn |
| Rare | Krabby–Kingler, Magikarp–Gyarados, Tentacool–Tentacruel | Krabby–Kingler, Magikarp–Gyarados, Chinchou–Lanturn |
| Very rare | Horsea–Seadra, Staryu | Horsea–Seadra, Staryu |

#### Route 21

Road, Kanto west.

**`MAP_ROUTE21_NORTH`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tangela ×2 | Tangela ×2 |
| Uncommon | Pidgey–Pidgeot, Exeggcute, Rattata–Raticate, Sunkern | Hoothoot–Noctowl, Oddish–Gloom, Venonat–Venomoth, Exeggcute |
| Rare | Yanma, Paras–Parasect, Tangela, Sentret–Furret | Spinarak–Ariados, Paras–Parasect, Tangela, Oddish–Gloom |
| Very rare | Bulbasaur–Ivysaur, Tangela | Bulbasaur–Ivysaur, Tangela |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Rare | Shellder, Corsola | Chinchou–Lanturn, Corsola |
| Very rare | Staryu | Staryu |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Shellder ×2 | Shellder ×2 |
| Uncommon | Qwilfish, Krabby–Kingler, Horsea–Seadra | Qwilfish, Krabby–Kingler, Chinchou–Lanturn |
| Rare | Magikarp, Staryu, Magikarp–Gyarados | Magikarp, Staryu, Magikarp–Gyarados |
| Very rare | Horsea–Seadra, Corsola | Chinchou–Lanturn, Corsola |

**`MAP_ROUTE21_SOUTH`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tangela ×2 | Tangela ×2 |
| Uncommon | Exeggcute, Pidgey–Pidgeot, Sentret–Furret, Rattata–Raticate | Exeggcute, Hoothoot–Noctowl, Oddish–Gloom, Venonat–Venomoth |
| Rare | Paras–Parasect, Yanma, Tangela, Sunkern | Spinarak–Ariados, Paras–Parasect, Tangela, Oddish–Gloom |
| Very rare | Farfetch'd, Exeggcute | Farfetch'd, Tangela |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Staryu | Staryu |
| Uncommon | Staryu | Staryu |
| Rare | Tentacool–Tentacruel, Corsola | Chinchou–Lanturn, Tentacool–Tentacruel |
| Very rare | Shellder | Corsola |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler ×2 | Krabby–Kingler ×2 |
| Uncommon | Remoraid–Octillery, Shellder, Horsea–Seadra | Remoraid–Octillery, Shellder, Chinchou–Lanturn |
| Rare | Magikarp, Staryu, Magikarp–Gyarados | Magikarp, Staryu, Magikarp–Gyarados |
| Very rare | Horsea–Seadra, Corsola | Chinchou–Lanturn, Corsola |

#### Route 22

Road, Border.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Rattata–Raticate, Mankey–Primeape | Rattata–Raticate, Mankey–Primeape |
| Uncommon | Spearow–Fearow, Ponyta, Nidoran♀–Nidorina, Nidoran♂–Nidorino | Hoothoot–Noctowl, Murkrow, Houndour, Nidoran♂–Nidorino |
| Rare | Doduo, Sentret–Furret, Ledyba–Ledian, Ponyta | Zubat–Golbat, Spinarak–Ariados, Hoothoot–Noctowl, Houndour |
| Very rare | Eevee, Igglybuff | Eevee, Murkrow |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Marill–Azumarill, Wooper–Quagsire | Wooper–Quagsire ×2 |
| Very rare | Psyduck–Golduck | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp, Marill–Azumarill, Goldeen–Seaking | Magikarp, Marill–Azumarill, Wooper–Quagsire |
| Rare | Wooper–Quagsire, Magikarp–Gyarados, Remoraid–Octillery | Wooper–Quagsire, Magikarp–Gyarados, Remoraid–Octillery |
| Very rare | Marill–Azumarill, Magikarp–Gyarados | Wooper–Quagsire, Magikarp–Gyarados |

#### Route 24

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell, Caterpie–Butterfree | Bellsprout–Weepinbell, Oddish–Gloom |
| Uncommon | Abra–Kadabra, Oddish–Gloom, Pidgey–Pidgeot, Weedle–Beedrill | Venonat–Venomoth, Hoothoot–Noctowl, Abra–Kadabra, Paras–Parasect |
| Rare | Abra–Kadabra, Caterpie–Metapod, Sunkern, Ledyba–Ledian | Abra–Kadabra, Spinarak–Ariados, Caterpie–Butterfree, Meowth–Persian |
| Very rare | Charmander–Charmeleon, Pichu | Charmander–Charmeleon, Pichu |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking | Goldeen–Seaking |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Psyduck–Golduck, Slowpoke–Slowbro | Poliwag–Poliwhirl, Psyduck–Golduck |
| Very rare | Remoraid–Octillery | Slowpoke–Slowbro |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Magikarp, Poliwag–Poliwhirl, Psyduck–Golduck | Magikarp, Poliwag–Poliwhirl, Slowpoke–Slowbro |
| Rare | Krabby–Kingler, Magikarp–Gyarados, Goldeen–Seaking | Krabby–Kingler, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Remoraid–Octillery, Magikarp–Gyarados | Remoraid–Octillery, Magikarp–Gyarados |

#### Viridian Forest

Road, Kanto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Caterpie–Metapod, Weedle–Kakuna | Caterpie–Metapod, Weedle–Kakuna |
| Uncommon | Caterpie–Metapod, Weedle–Kakuna, Paras–Parasect, Pidgey–Pidgeotto | Spinarak, Venonat, Paras–Parasect, Hoothoot |
| Rare | Pikachu ×2, Ledyba, Bulbasaur–Ivysaur | Pikachu ×2, Oddish–Gloom, Bulbasaur–Ivysaur |
| Very rare | Bulbasaur–Ivysaur, Pichu | Bulbasaur–Ivysaur, Pichu |

#### Route 12

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Oddish–Gloom, Bellsprout–Weepinbell | Oddish–Gloom, Bellsprout–Weepinbell |
| Uncommon | Venonat–Venomoth, Pidgey–Pidgeot, Farfetch'd, Bellsprout–Weepinbell | Venonat–Venomoth, Hoothoot–Noctowl, Gastly–Haunter, Drowzee–Hypno |
| Rare | Oddish–Gloom, Nidoran♂–Nidorino, Yanma, Nidoran♀–Nidorina | Spinarak–Ariados, Zubat–Golbat, Yanma, Nidoran♂–Nidorino |
| Very rare | Farfetch'd, Yanma | Farfetch'd, Murkrow |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Rare | Krabby–Kingler, Slowpoke–Slowbro | Chinchou–Lanturn, Krabby–Kingler |
| Very rare | Shellder | Slowpoke–Slowbro |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Qwilfish ×2 | Qwilfish ×2 |
| Uncommon | Magikarp, Krabby–Kingler, Horsea–Seadra | Magikarp, Krabby–Kingler, Chinchou–Lanturn |
| Rare | Krabby–Kingler, Magikarp–Gyarados, Horsea–Seadra | Krabby–Kingler, Magikarp–Gyarados, Chinchou–Lanturn |
| Very rare | Qwilfish, Magikarp–Gyarados | Qwilfish, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Venonat–Venomoth | Venonat–Venomoth |
| Uncommon | Venonat–Venomoth | Venonat–Venomoth |
| Rare | Exeggcute, Pidgey–Pidgeot | Hoothoot–Noctowl, Exeggcute |
| Very rare | Pineco | Spinarak–Ariados |

#### Route 13

Road, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell, Oddish–Gloom | Bellsprout–Weepinbell, Oddish–Gloom |
| Uncommon | Pidgey–Pidgeot, Venonat–Venomoth, Nidoran♀–Nidorina, Nidoran♂–Nidorino | Venonat–Venomoth, Hoothoot–Noctowl, Gastly–Haunter, Zubat–Golbat |
| Rare | Ditto ×2, Farfetch'd, Oddish–Gloom | Ditto ×2, Spinarak–Ariados, Murkrow |
| Very rare | Ditto, Pidgey–Pidgeot | Ditto, Nidoran♂–Nidorino |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra | Horsea–Seadra |
| Uncommon | Horsea–Seadra | Horsea–Seadra |
| Rare | Tentacool–Tentacruel, Qwilfish | Chinchou–Lanturn, Tentacool–Tentacruel |
| Very rare | Slowpoke–Slowbro | Qwilfish |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra ×2 | Horsea–Seadra ×2 |
| Uncommon | Magikarp, Krabby–Kingler, Qwilfish | Magikarp, Krabby–Kingler, Chinchou–Lanturn |
| Rare | Krabby–Kingler, Magikarp–Gyarados, Corsola | Krabby–Kingler, Magikarp–Gyarados, Chinchou–Lanturn |
| Very rare | Horsea–Seadra, Magikarp–Gyarados | Horsea–Seadra, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pineco | Pineco |
| Uncommon | Pineco | Pineco |
| Rare | Exeggcute, Geodude–Graveler | Hoothoot–Noctowl, Exeggcute |
| Very rare | Pidgey–Pidgeot | Venonat–Venomoth |

#### Route 9

Wilds, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Rattata–Raticate, Ekans–Arbok | Rattata–Raticate, Ekans–Arbok |
| Uncommon | Spearow–Fearow, Sandshrew–Sandslash, Rhyhorn, Rattata–Raticate | Venonat–Venomoth, Hoothoot–Noctowl, Rhyhorn, Zubat–Golbat |
| Rare | Mankey–Primeape, Kangaskhan, Onix, Sandshrew–Sandslash | Gastly–Haunter, Kangaskhan, Onix, Cubone–Marowak |
| Very rare | Kangaskhan, Cubone–Marowak | Kangaskhan, Murkrow |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Slowpoke–Slowbro, Goldeen–Seaking | Poliwag–Poliwhirl, Slowpoke–Slowbro |
| Very rare | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Magikarp, Poliwag–Poliwhirl, Psyduck–Golduck | Magikarp, Poliwag–Poliwhirl, Slowpoke–Slowbro |
| Rare | Magikarp, Magikarp–Gyarados, Poliwag–Poliwhirl | Magikarp, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Magikarp–Gyarados ×2 | Magikarp–Gyarados ×2 |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler | Geodude–Graveler |
| Uncommon | Geodude–Graveler | Geodude–Graveler |
| Rare | Spearow–Fearow, Pinsir | Hoothoot–Noctowl, Pinsir |
| Very rare | Shuckle | Shuckle |

#### Route 10

Wilds, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Voltorb–Electrode, Sandshrew–Sandslash | Voltorb–Electrode, Sandshrew–Sandslash |
| Uncommon | Spearow–Fearow, Ekans–Arbok, Magnemite–Magneton, Rhyhorn | Gastly–Haunter, Zubat–Golbat, Hoothoot–Noctowl, Magnemite–Magneton |
| Rare | Electabuzz ×2, Tauros, Pikachu | Electabuzz ×2, Houndour, Pikachu |
| Very rare | Tauros, Elekid | Tauros, Elekid |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Goldeen–Seaking, Psyduck–Golduck | Slowpoke–Slowbro ×2 |
| Very rare | Slowpoke–Slowbro | Goldeen–Seaking |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp ×2, Goldeen–Seaking | Magikarp, Goldeen–Seaking, Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Goldeen–Seaking | Psyduck–Golduck, Magikarp–Gyarados, Slowpoke–Slowbro |
| Very rare | Poliwag–Poliwhirl, Magikarp–Gyarados | Poliwag–Poliwhirl, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Sandshrew–Sandslash | Sandshrew–Sandslash |
| Uncommon | Sandshrew–Sandslash | Sandshrew–Sandslash |
| Rare | Geodude–Graveler, Shuckle | Spinarak–Ariados, Shuckle |
| Very rare | Voltorb–Electrode | Geodude–Graveler |

#### Route 14

Wilds, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Oddish–Gloom, Venonat–Venomoth | Oddish–Gloom, Venonat–Venomoth |
| Uncommon | Bellsprout–Weepinbell, Pidgey–Pidgeot, Nidoran♀–Nidorina, Nidoran♂–Nidorino | Hoothoot–Noctowl, Gastly–Haunter, Spinarak–Ariados, Nidoran♂–Nidorino |
| Rare | Ditto, Tauros, Chansey, Mr. Mime | Ditto, Zubat–Golbat, Chansey, Murkrow |
| Very rare | Ditto, Chansey | Ditto, Chansey |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Poliwag–Poliwhirl | Psyduck–Golduck, Goldeen–Seaking |
| Very rare | Goldeen–Seaking | Goldeen–Seaking |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp, Goldeen–Seaking | Magikarp, Goldeen–Seaking |
| Uncommon | Slowpoke–Slowbro, Magikarp, Poliwag–Poliwhirl | Slowpoke–Slowbro, Magikarp, Psyduck–Golduck |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Goldeen–Seaking | Psyduck–Golduck ×2, Magikarp–Gyarados |
| Very rare | Poliwag–Poliwhirl, Magikarp–Gyarados | Poliwag–Poliwhirl, Magikarp–Gyarados |

#### Route 15

Wilds, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell, Oddish–Gloom | Bellsprout–Weepinbell, Oddish–Gloom |
| Uncommon | Pidgey–Pidgeot, Venonat–Venomoth, Nidoran♀–Nidorina, Ditto | Venonat–Venomoth, Hoothoot–Noctowl, Gastly–Haunter, Ditto |
| Rare | Nidoran♂–Nidorino, Scyther, Kangaskhan, Chansey | Zubat–Golbat, Spinarak–Ariados, Kangaskhan, Chansey |
| Very rare | Ditto, Scyther | Ditto, Murkrow |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking | Goldeen–Seaking |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Slowpoke–Slowbro, Psyduck–Golduck | Slowpoke–Slowbro, Poliwag–Poliwhirl |
| Very rare | Poliwag–Poliwhirl | Poliwag–Poliwhirl |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Magikarp, Poliwag–Poliwhirl, Psyduck–Golduck | Magikarp, Poliwag–Poliwhirl, Slowpoke–Slowbro |
| Rare | Slowpoke–Slowbro, Magikarp–Gyarados, Goldeen–Seaking | Slowpoke–Slowbro ×2, Magikarp–Gyarados |
| Very rare | Poliwag–Poliwhirl, Magikarp–Gyarados | Poliwag–Poliwhirl, Magikarp–Gyarados |

#### Route 19

Wilds, Kanto east.

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Rare | Remoraid–Octillery, Shellder | Chinchou–Lanturn, Staryu |
| Very rare | Squirtle–Wartortle | Squirtle–Wartortle |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler ×2 | Krabby–Kingler ×2 |
| Uncommon | Magikarp, Horsea–Seadra, Shellder | Magikarp, Horsea–Seadra, Chinchou–Lanturn |
| Rare | Remoraid–Octillery, Magikarp–Gyarados, Horsea–Seadra | Remoraid–Octillery, Magikarp–Gyarados, Chinchou–Lanturn |
| Very rare | Staryu, Magikarp–Gyarados | Staryu, Magikarp–Gyarados |

#### Mt. Moon

Wilds, Kanto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Zubat–Golbat, Geodude–Graveler | Zubat–Golbat, Geodude–Graveler |
| Uncommon | Zubat–Golbat, Paras–Parasect, Geodude–Graveler, Sandshrew–Sandslash | Zubat–Golbat, Clefairy, Paras–Parasect, Geodude–Graveler |
| Rare | Paras–Parasect, Onix, Clefairy, Marill–Azumarill | Clefairy ×2, Onix, Marill–Azumarill |
| Very rare | Clefairy, Cleffa | Cleffa ×2 |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill | Marill–Azumarill |
| Uncommon | Marill–Azumarill | Marill–Azumarill |
| Rare | Psyduck–Golduck, Zubat–Golbat | Zubat–Golbat ×2 |
| Very rare | Poliwag–Poliwhirl | Psyduck–Golduck |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp ×2 | Magikarp ×2 |
| Uncommon | Poliwag–Poliwhirl, Psyduck–Golduck, Magikarp | Poliwag–Poliwhirl, Psyduck–Golduck, Magikarp |
| Rare | Marill–Azumarill, Magikarp–Gyarados, Psyduck–Golduck | Marill–Azumarill, Magikarp–Gyarados, Psyduck–Golduck |
| Very rare | Magikarp–Gyarados, Poliwag–Poliwhirl | Slowpoke–Slowbro ×2 |

#### Rock Tunnel

Wilds, Kanto east.

**`MAP_ROCK_TUNNEL_1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Zubat–Golbat, Geodude–Graveler | Zubat–Golbat, Geodude–Graveler |
| Uncommon | Machop–Machoke, Onix, Zubat–Golbat, Mankey–Primeape | Machop–Machoke, Onix, Zubat–Golbat, Gastly–Haunter |
| Rare | Onix, Geodude–Graveler, Hitmonchan, Dunsparce | Onix, Dunsparce ×2, Hitmonchan |
| Very rare | Hitmonlee, Tyrogue | Hitmonlee, Tyrogue |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Zubat–Golbat, Slowpoke–Slowbro | Zubat–Golbat ×2 |
| Very rare | Poliwag–Poliwhirl | Zubat–Golbat |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp ×2 | Magikarp ×2 |
| Uncommon | Poliwag–Poliwhirl, Psyduck–Golduck, Magikarp | Poliwag–Poliwhirl, Psyduck–Golduck, Magikarp |
| Rare | Goldeen–Seaking, Magikarp–Gyarados, Poliwag–Poliwhirl | Slowpoke–Slowbro, Magikarp–Gyarados, Poliwag–Poliwhirl |
| Very rare | Magikarp–Gyarados, Psyduck–Golduck | Slowpoke–Slowbro, Psyduck–Golduck |

**`MAP_ROCK_TUNNEL_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Onix, Machop–Machoke | Onix, Machop–Machoke |
| Uncommon | Geodude–Graveler, Zubat–Golbat, Onix, Cubone–Marowak | Geodude–Graveler, Zubat–Golbat, Gastly–Haunter, Cubone–Marowak |
| Rare | Geodude–Graveler, Rhyhorn, Hitmonlee, Kangaskhan | Misdreavus ×2, Rhyhorn, Hitmonlee |
| Very rare | Hitmonchan, Tyrogue | Hitmonchan, Tyrogue |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Zubat–Golbat, Psyduck–Golduck | Zubat–Golbat ×2 |
| Very rare | Poliwag–Poliwhirl | Zubat–Golbat |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp ×2 | Magikarp ×2 |
| Uncommon | Poliwag–Poliwhirl, Slowpoke–Slowbro, Goldeen–Seaking | Poliwag–Poliwhirl, Slowpoke–Slowbro, Goldeen–Seaking |
| Rare | Slowpoke–Slowbro, Magikarp–Gyarados, Poliwag–Poliwhirl | Psyduck–Golduck, Magikarp–Gyarados, Poliwag–Poliwhirl |
| Very rare | Magikarp–Gyarados, Goldeen–Seaking | Psyduck–Golduck, Goldeen–Seaking |

#### Diglett's Cave

Wilds, Kanto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Diglett–Dugtrio ×2 | Diglett–Dugtrio ×2 |
| Uncommon | Diglett ×2, Geodude–Graveler, Sandshrew–Sandslash | Diglett ×2, Zubat–Golbat, Geodude–Graveler |
| Rare | Diglett–Dugtrio, Onix, Diglett, Dunsparce | Zubat–Golbat, Onix, Diglett, Dunsparce |
| Very rare | Diglett–Dugtrio, Dunsparce | Dunsparce ×2 |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Goldeen–Seaking, Zubat–Golbat | Zubat–Golbat ×2 |
| Very rare | Psyduck–Golduck | Goldeen–Seaking |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp ×2 | Magikarp ×2 |
| Uncommon | Poliwag–Poliwhirl ×2, Goldeen–Seaking | Poliwag–Poliwhirl ×2, Goldeen–Seaking |
| Rare | Magikarp, Magikarp–Gyarados, Goldeen–Seaking | Slowpoke–Slowbro, Magikarp–Gyarados, Goldeen–Seaking |
| Very rare | Magikarp–Gyarados, Poliwag–Poliwhirl | Slowpoke–Slowbro, Poliwag–Poliwhirl |

#### Route 25

Wilds, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Abra–Kadabra, Weedle–Beedrill | Abra–Kadabra, Weedle–Beedrill |
| Uncommon | Oddish–Gloom, Bellsprout–Weepinbell, Pidgey–Pidgeot, Caterpie–Butterfree | Venonat–Venomoth, Hoothoot–Noctowl, Oddish–Gloom, Gastly–Haunter |
| Rare | Abra–Kadabra, Scyther, Pinsir, Bulbasaur–Ivysaur | Abra–Kadabra, Spinarak–Ariados, Pinsir, Bulbasaur–Ivysaur |
| Very rare | Bulbasaur–Ivysaur, Scyther | Bulbasaur–Ivysaur, Scyther |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Staryu | Staryu |
| Uncommon | Staryu | Staryu |
| Rare | Tentacool–Tentacruel, Psyduck–Golduck | Chinchou–Lanturn, Tentacool–Tentacruel |
| Very rare | Slowpoke–Slowbro | Psyduck–Golduck |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Staryu ×2 | Staryu ×2 |
| Uncommon | Magikarp, Goldeen–Seaking, Krabby–Kingler | Magikarp, Goldeen–Seaking, Chinchou–Lanturn |
| Rare | Poliwag–Poliwhirl, Magikarp–Gyarados, Horsea–Seadra | Poliwag–Poliwhirl, Magikarp–Gyarados, Chinchou–Lanturn |
| Very rare | Staryu, Magikarp–Gyarados | Staryu, Magikarp–Gyarados |

#### Route 20

Outlands, Kanto west.

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Rare | Staryu–Starmie, Seel–Dewgong | Chinchou–Lanturn, Staryu–Starmie |
| Very rare | Lapras | Lapras |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Shellder ×2 | Shellder ×2 |
| Uncommon | Krabby–Kingler, Horsea–Seadra, Remoraid–Octillery | Krabby–Kingler, Horsea–Seadra, Chinchou–Lanturn |
| Rare | Shellder–Cloyster, Horsea–Kingdra, Magikarp–Gyarados | Shellder–Cloyster, Horsea–Kingdra, Chinchou–Lanturn |
| Very rare | Dratini–Dragonair, Staryu–Starmie | Dratini–Dragonair, Staryu–Starmie |

#### Route 23

Outlands, Border.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow, Mankey–Primeape | Spearow–Fearow, Mankey–Primeape |
| Uncommon | Ekans–Arbok, Sandshrew–Sandslash, Gligar, Phanpy–Donphan | Zubat–Golbat, Houndour–Houndoom, Teddiursa–Ursaring, Gligar |
| Rare | Rhyhorn–Rhydon, Skarmory, Tauros, Teddiursa–Ursaring | Murkrow, Sneasel, Sandshrew–Sandslash, Skarmory |
| Very rare | Snorlax, Mankey–Annihilape | Snorlax, Mankey–Annihilape |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking | Goldeen–Seaking |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Wooper–Quagsire, Marill–Azumarill | Wooper–Quagsire ×2 |
| Very rare | Poliwag–Politoed | Wooper–Quagsire |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Poliwag–Poliwhirl, Wooper–Quagsire, Poliwag–Poliwrath | Poliwag–Poliwhirl, Wooper–Quagsire ×2 |
| Rare | Marill–Azumarill, Magikarp–Gyarados, Poliwag–Politoed | Marill–Azumarill, Magikarp–Gyarados, Poliwag–Politoed |
| Very rare | Magikarp–Gyarados, Dratini–Dragonair | Wooper–Quagsire, Dratini–Dragonair |

#### Pokémon Tower

Dungeon, Kanto east.

**`MAP_POKEMON_TOWER_3F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Gastly–Haunter ×2 | Gastly–Haunter ×2 |
| Uncommon | Cubone–Marowak ×2, Gastly–Haunter ×2 | Cubone–Marowak ×2, Misdreavus, Gastly–Haunter |
| Rare | Misdreavus ×2, Gastly–Haunter, Cubone–Marowak | Misdreavus ×2, Murkrow, Cubone–Marowak |
| Very rare | Gastly–Haunter, Misdreavus | Gastly–Haunter, Misdreavus |

**`MAP_POKEMON_TOWER_4F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Gastly–Haunter, Cubone–Marowak | Gastly–Haunter, Cubone–Marowak |
| Uncommon | Gastly–Haunter ×2, Cubone–Marowak, Misdreavus | Gastly–Haunter ×2, Misdreavus ×2 |
| Rare | Gastly–Haunter ×2, Cubone–Marowak, Misdreavus | Murkrow, Cubone–Marowak, Misdreavus, Gastly–Gengar |
| Very rare | Misdreavus, Gastly–Gengar | Gastly–Gengar, Misdreavus–Mismagius |

**`MAP_POKEMON_TOWER_5F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Gastly–Haunter ×2 | Gastly–Haunter ×2 |
| Uncommon | Cubone–Marowak ×2, Gastly–Haunter, Misdreavus | Cubone–Marowak, Misdreavus ×2, Gastly–Haunter |
| Rare | Gastly–Gengar ×2, Cubone–Marowak, Misdreavus | Gastly–Gengar ×2, Murkrow, Misdreavus–Mismagius |
| Very rare | Misdreavus–Mismagius, Gastly–Gengar | Misdreavus–Mismagius, Gastly–Gengar |

**`MAP_POKEMON_TOWER_6F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Gastly–Haunter, Cubone–Marowak | Gastly–Haunter, Cubone–Marowak |
| Uncommon | Gastly–Haunter, Cubone–Marowak, Gastly–Gengar, Misdreavus | Gastly–Haunter, Misdreavus ×2, Gastly–Gengar |
| Rare | Gastly–Gengar ×2, Cubone–Marowak, Misdreavus | Murkrow, Cubone–Marowak, Misdreavus–Mismagius, Gastly–Gengar |
| Very rare | Misdreavus–Mismagius, Gastly–Gengar | Murkrow–Honchkrow, Misdreavus–Mismagius |

**`MAP_POKEMON_TOWER_7F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Gastly–Gengar, Cubone–Marowak | Gastly–Gengar, Cubone–Marowak |
| Uncommon | Gastly–Haunter, Cubone–Marowak, Gastly–Gengar, Misdreavus | Gastly–Haunter, Misdreavus, Gastly–Gengar, Misdreavus–Mismagius |
| Rare | Gastly–Haunter, Misdreavus–Mismagius, Cubone–Marowak, Gastly–Gengar | Murkrow–Honchkrow, Misdreavus–Mismagius, Cubone–Marowak, Gastly–Gengar |
| Very rare | Misdreavus–Mismagius, Gastly–Gengar | Murkrow–Honchkrow, Gastly–Gengar |

#### Pokémon Mansion

Dungeon, Kanto west.

**`MAP_POKEMON_MANSION_1F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Grimer–Muk, Koffing–Weezing | Grimer–Muk, Koffing–Weezing |
| Uncommon | Rattata–Raticate, Growlithe, Vulpix, Slugma–Magcargo | Rattata–Raticate, Growlithe, Vulpix, Gastly–Haunter |
| Rare | Ponyta, Koffing, Charmander–Charmeleon, Rattata–Raticate | Houndour ×2, Slugma–Magcargo, Charmander–Charmeleon |
| Very rare | Magby, Charmander–Charmeleon | Magby, Charmander–Charmeleon |

**`MAP_POKEMON_MANSION_2F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Growlithe, Vulpix | Growlithe, Vulpix |
| Uncommon | Grimer–Muk, Koffing–Weezing, Slugma–Magcargo, Ponyta–Rapidash | Grimer–Muk, Koffing–Weezing, Gastly–Haunter, Houndour |
| Rare | Rattata–Raticate, Ponyta, Charmander–Charmeleon, Magmar | Slugma–Magcargo, Ponyta–Rapidash, Charmander–Charmeleon, Magmar |
| Very rare | Magby, Ditto | Magby, Ditto |

**`MAP_POKEMON_MANSION_3F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Koffing–Weezing, Grimer–Muk | Koffing–Weezing, Grimer–Muk |
| Uncommon | Growlithe–Arcanine, Vulpix–Ninetales, Slugma–Magcargo, Ponyta–Rapidash | Growlithe–Arcanine, Vulpix–Ninetales, Gastly–Haunter, Houndour–Houndoom |
| Rare | Magmar, Rattata–Raticate, Charmander–Charmeleon, Ditto | Magmar, Slugma–Magcargo, Charmander–Charmeleon, Ditto |
| Very rare | Ditto, Magby | Ditto, Magby |

**`MAP_POKEMON_MANSION_B1F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ditto, Grimer–Muk | Ditto, Grimer–Muk |
| Uncommon | Koffing–Weezing, Ditto, Growlithe–Arcanine, Vulpix–Ninetales | Koffing–Weezing, Ditto, Growlithe–Arcanine, Gastly–Gengar |
| Rare | Magmar, Ponyta–Rapidash, Charmander–Charizard, Porygon | Magmar, Houndour–Houndoom, Charmander–Charizard, Porygon |
| Very rare | Aerodactyl, Magmar–Magmortar | Aerodactyl, Magmar–Magmortar |

#### Power Plant

Dungeon, Kanto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magnemite–Magneton, Voltorb | Magnemite–Magneton, Voltorb |
| Uncommon | Magnemite–Magneton, Voltorb–Electrode, Pikachu, Electabuzz | Magnemite–Magneton, Voltorb–Electrode, Pikachu, Electabuzz |
| Rare | Pikachu, Magnemite–Magnezone, Electabuzz, Porygon | Pikachu, Magnemite–Magnezone, Elekid, Porygon |
| Very rare | Pikachu–Raichu, Electabuzz–Electivire | Pikachu–Raichu, Electabuzz–Electivire |

#### Seafoam Islands

Dungeon, Kanto west.

**`MAP_SEAFOAM_ISLANDS_1F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong, Psyduck–Golduck | Seel–Dewgong, Psyduck–Golduck |
| Uncommon | Zubat–Golbat, Slowpoke–Slowbro, Seel–Dewgong, Swinub | Zubat–Golbat ×2, Slowpoke–Slowbro, Sneasel |
| Rare | Zubat–Golbat, Shellder, Slowpoke–Slowbro, Smoochum | Swinub, Shellder, Sneasel, Smoochum |
| Very rare | Jynx, Smoochum | Jynx, Smoochum |

**`MAP_SEAFOAM_ISLANDS_B1F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong, Slowpoke–Slowbro | Seel–Dewgong, Slowpoke–Slowbro |
| Uncommon | Psyduck–Golduck, Jynx, Seel–Dewgong, Swinub | Psyduck–Golduck, Jynx, Sneasel, Swinub |
| Rare | Delibird, Zubat–Golbat, Shellder, Psyduck–Golduck | Delibird, Zubat–Crobat, Sneasel, Zubat–Golbat |
| Very rare | Smoochum, Shellder–Cloyster | Smoochum, Shellder–Cloyster |

**`MAP_SEAFOAM_ISLANDS_B2F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong, Zubat–Crobat | Seel–Dewgong, Zubat–Crobat |
| Uncommon | Slowpoke–Slowbro, Jynx, Swinub–Piloswine, Delibird | Slowpoke–Slowbro, Jynx, Sneasel, Swinub–Piloswine |
| Rare | Psyduck–Golduck, Shellder, Jynx, Swinub | Delibird, Sneasel, Jynx, Sneasel–Weavile |
| Very rare | Slowpoke–Slowking, Shellder–Cloyster | Slowpoke–Slowking, Sneasel–Weavile |

**`MAP_SEAFOAM_ISLANDS_B3F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong, Jynx | Seel–Dewgong, Jynx |
| Uncommon | Seel–Dewgong, Shellder–Cloyster, Swinub–Piloswine, Slowpoke–Slowbro | Seel–Dewgong, Shellder–Cloyster, Sneasel, Swinub–Piloswine |
| Rare | Delibird, Psyduck–Golduck, Zubat–Crobat, Slowpoke–Slowking | Sneasel–Weavile, Psyduck–Golduck, Zubat–Crobat, Slowpoke–Slowking |
| Very rare | Lapras, Swinub–Mamoswine | Lapras, Swinub–Mamoswine |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong | Seel–Dewgong |
| Uncommon | Seel–Dewgong | Seel–Dewgong |
| Rare | Horsea–Seadra, Shellder–Cloyster | Chinchou–Lanturn, Shellder–Cloyster |
| Very rare | Lapras | Lapras |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Shellder ×2 | Shellder ×2 |
| Uncommon | Horsea–Seadra ×2, Krabby–Kingler | Horsea–Seadra, Krabby–Kingler, Chinchou–Lanturn |
| Rare | Magikarp–Gyarados, Seel–Dewgong, Horsea–Kingdra | Magikarp–Gyarados, Seel–Dewgong, Horsea–Kingdra |
| Very rare | Dratini–Dragonair, Shellder–Cloyster | Dratini–Dragonair, Chinchou–Lanturn |

**`MAP_SEAFOAM_ISLANDS_B4F`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong, Jynx | Seel–Dewgong, Jynx |
| Uncommon | Shellder–Cloyster, Slowpoke–Slowking, Swinub–Piloswine, Delibird | Shellder–Cloyster, Slowpoke–Slowking, Sneasel, Sneasel–Weavile |
| Rare | Lapras, Jynx, Swinub–Mamoswine, Zubat–Crobat | Lapras, Jynx, Swinub–Mamoswine, Zubat–Crobat |
| Very rare | Lapras, Sneasel–Weavile | Lapras, Sneasel–Weavile |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong | Seel–Dewgong |
| Uncommon | Seel–Dewgong | Seel–Dewgong |
| Rare | Lapras, Shellder–Cloyster | Lapras, Chinchou–Lanturn |
| Very rare | Horsea–Kingdra | Horsea–Kingdra |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra ×2 | Horsea–Seadra ×2 |
| Uncommon | Shellder–Cloyster, Seel–Dewgong, Krabby–Kingler | Shellder–Cloyster, Seel–Dewgong, Chinchou–Lanturn |
| Rare | Magikarp–Gyarados, Dratini–Dragonair, Horsea–Kingdra | Magikarp–Gyarados, Dratini–Dragonair, Horsea–Kingdra |
| Very rare | Dratini–Dragonite, Dratini–Dragonair | Chinchou–Lanturn, Dratini–Dragonair |

#### Victory Road

Dungeon, Border.

**`MAP_VICTORY_ROAD_KANTO_1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Machop–Machoke, Geodude–Graveler | Machop–Machoke, Geodude–Graveler |
| Uncommon | Onix, Zubat–Golbat, Cubone–Marowak, Gligar | Onix, Zubat–Golbat, Misdreavus, Gligar |
| Rare | Sandshrew–Sandslash, Machop–Machoke, Hitmonlee, Hitmonchan | Sandshrew–Sandslash, Misdreavus, Hitmonlee, Hitmonchan |
| Very rare | Tyrogue, Skarmory | Tyrogue, Misdreavus |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Wooper–Quagsire |
| Uncommon | Wooper–Quagsire | Wooper–Quagsire |
| Rare | Psyduck–Golduck, Zubat–Golbat | Zubat–Golbat ×2 |
| Very rare | Slowpoke–Slowbro | Psyduck–Golduck |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp, Poliwag–Poliwhirl | Magikarp, Poliwag–Poliwhirl |
| Uncommon | Wooper–Quagsire, Magikarp, Goldeen–Seaking | Wooper–Quagsire, Magikarp, Marill–Azumarill |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Wooper–Quagsire | Psyduck–Golduck, Magikarp–Gyarados, Marill–Azumarill |
| Very rare | Goldeen–Seaking, Magikarp–Gyarados | Goldeen–Seaking, Magikarp–Gyarados |

**`MAP_VICTORY_ROAD_KANTO_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Onix, Zubat–Golbat | Onix, Zubat–Golbat |
| Uncommon | Geodude–Graveler, Machop–Machoke, Rhyhorn–Rhydon, Cubone–Marowak | Geodude–Graveler, Machop–Machoke, Misdreavus, Cubone–Marowak |
| Rare | Onix–Steelix, Skarmory, Hitmontop, Machop–Machamp | Zubat–Crobat, Skarmory, Hitmontop, Machop–Machamp |
| Very rare | Aerodactyl, Hitmonlee | Aerodactyl, Hitmonchan |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Poliwag–Poliwhirl | Psyduck–Golduck, Zubat–Crobat |
| Very rare | Slowpoke–Slowking | Zubat–Crobat |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp, Goldeen–Seaking, Psyduck–Golduck | Magikarp, Goldeen–Seaking, Psyduck–Golduck |
| Rare | Poliwag–Poliwrath, Magikarp–Gyarados, Poliwag–Politoed | Marill–Azumarill, Magikarp–Gyarados, Poliwag–Politoed |
| Very rare | Magikarp–Gyarados, Dratini–Dragonair | Marill–Azumarill, Dratini–Dragonair |

**`MAP_VICTORY_ROAD_KANTO_B2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Machop–Machamp, Cubone–Marowak | Machop–Machamp, Cubone–Marowak |
| Uncommon | Geodude–Golem, Onix–Steelix, Rhyhorn–Rhydon, Skarmory | Geodude–Golem, Onix–Steelix, Zubat–Crobat, Skarmory |
| Rare | Gligar–Gliscor, Rhyhorn–Rhyperior, Aerodactyl, Snorlax | Gligar–Gliscor, Misdreavus–Mismagius, Aerodactyl, Snorlax |
| Very rare | Dratini–Dragonite, Hitmontop | Dratini–Dragonite, Hitmontop |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Wooper–Quagsire |
| Uncommon | Wooper–Quagsire | Wooper–Quagsire |
| Rare | Slowpoke–Slowking, Psyduck–Golduck | Zubat–Crobat, Psyduck–Golduck |
| Very rare | Poliwag–Politoed | Zubat–Crobat |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados ×2 | Magikarp–Gyarados ×2 |
| Uncommon | Poliwag–Poliwrath, Wooper–Quagsire, Goldeen–Seaking | Poliwag–Poliwrath, Wooper–Quagsire ×2 |
| Rare | Poliwag–Politoed, Horsea–Kingdra, Goldeen–Seaking | Poliwag–Politoed, Horsea–Kingdra, Wooper–Quagsire |
| Very rare | Dratini–Dragonair, Dratini–Dragonite | Dratini–Dragonair, Dratini–Dragonite |

#### Cerulean Cave

Dungeon, Kanto east.

**`MAP_CERULEAN_CAVE_1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Drowzee–Hypno, Magnemite–Magneton | Drowzee–Hypno, Magnemite–Magneton |
| Uncommon | Zubat–Golbat, Venonat–Venomoth, Doduo–Dodrio, Paras–Parasect | Zubat–Golbat, Venonat–Venomoth, Wobbuffet, Paras–Parasect |
| Rare | Abra–Kadabra, Sandshrew–Sandslash, Ditto, Wobbuffet | Abra–Kadabra, Gastly–Haunter, Ditto, Wobbuffet |
| Very rare | Mr. Mime, Pikachu | Mr. Mime, Misdreavus |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Slowpoke–Slowbro, Goldeen–Seaking | Slowpoke–Slowbro, Poliwag–Poliwhirl |
| Very rare | Slowpoke–Slowbro | Poliwag–Poliwhirl |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados, Goldeen–Seaking | Magikarp–Gyarados, Goldeen–Seaking |
| Uncommon | Poliwag–Poliwhirl, Psyduck–Golduck, Magikarp–Gyarados | Poliwag–Poliwhirl, Psyduck–Golduck, Magikarp–Gyarados |
| Rare | Psyduck–Golduck, Goldeen–Seaking, Magikarp–Gyarados | Slowpoke–Slowbro ×2, Goldeen–Seaking |
| Very rare | Poliwag–Poliwhirl, Magikarp–Gyarados | Poliwag–Poliwhirl, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler | Geodude–Graveler |
| Uncommon | Geodude–Graveler | Geodude–Graveler |
| Rare | Sandshrew–Sandslash, Geodude–Graveler | Spinarak–Ariados, Sandshrew–Sandslash |
| Very rare | Shuckle | Shuckle |

**`MAP_CERULEAN_CAVE_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Rhyhorn–Rhydon, Zubat–Golbat | Rhyhorn–Rhydon, Zubat–Golbat |
| Uncommon | Magnemite–Magneton, Voltorb–Electrode, Cubone–Marowak, Ditto | Magnemite–Magneton, Voltorb–Electrode, Zubat–Crobat, Ditto |
| Rare | Chansey, Jigglypuff–Wigglytuff, Abra–Alakazam, Ditto | Chansey, Wobbuffet, Abra–Alakazam, Ditto |
| Very rare | Kangaskhan, Chansey–Blissey | Kangaskhan, Chansey–Blissey |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Psyduck–Golduck, Slowpoke–Slowking | Psyduck–Golduck, Poliwag–Poliwhirl |
| Very rare | Goldeen–Seaking | Slowpoke–Slowking |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Goldeen–Seaking, Psyduck–Golduck, Magikarp–Gyarados | Goldeen–Seaking, Psyduck–Golduck, Magikarp–Gyarados |
| Rare | Poliwag–Poliwrath, Magikarp–Gyarados, Slowpoke–Slowbro | Slowpoke–Slowbro, Magikarp–Gyarados, Poliwag–Poliwrath |
| Very rare | Goldeen–Seaking, Dratini–Dragonair | Slowpoke–Slowbro, Dratini–Dragonair |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Golem | Geodude–Golem |
| Uncommon | Geodude–Graveler | Geodude–Graveler |
| Rare | Rhyhorn–Rhydon, Shuckle | Spinarak–Ariados, Shuckle |
| Very rare | Geodude–Golem | Rhyhorn–Rhydon |

**`MAP_CERULEAN_CAVE_B2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ditto, Abra–Alakazam | Ditto, Abra–Alakazam |
| Uncommon | Magnemite–Magnezone, Rhyhorn–Rhyperior, Lickitung–Lickilicky, Voltorb–Electrode | Magnemite–Magnezone, Rhyhorn–Rhyperior, Zubat–Crobat, Gastly–Gengar |
| Rare | Chansey, Chansey–Blissey, Snorlax, Aerodactyl | Chansey, Chansey–Blissey, Snorlax, Aerodactyl |
| Very rare | Dratini–Dragonite, Mr. Mime | Dratini–Dragonite, Mr. Mime |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowking | Slowpoke–Slowking |
| Uncommon | Slowpoke–Slowking | Slowpoke–Slowking |
| Rare | Psyduck–Golduck, Staryu–Starmie | Psyduck–Golduck, Goldeen–Seaking |
| Very rare | Poliwag–Politoed | Poliwag–Politoed |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking ×2 | Goldeen–Seaking ×2 |
| Uncommon | Magikarp–Gyarados, Poliwag–Poliwrath, Slowpoke–Slowbro | Magikarp–Gyarados, Poliwag–Poliwrath, Psyduck–Golduck |
| Rare | Horsea–Kingdra, Magikarp–Gyarados, Dratini–Dragonair | Horsea–Kingdra, Magikarp–Gyarados, Dratini–Dragonair |
| Very rare | Poliwag–Politoed, Dratini–Dragonite | Psyduck–Golduck, Dratini–Dragonite |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Golem | Geodude–Golem |
| Uncommon | Geodude–Golem | Geodude–Golem |
| Rare | Rhyhorn–Rhyperior, Shuckle | Spinarak–Ariados, Shuckle |
| Very rare | Onix–Steelix | Rhyhorn–Rhyperior |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Abra | Cerulean Cave, Route 2, Route 24, Route 25 and more |
| Aerodactyl | Cerulean Cave, Pokémon Mansion, Victory Road |
| Alakazam | Cerulean Cave |
| Annihilape | Route 23 |
| Arbok | Route 10, Route 11, Route 23, Route 3 and more |
| Arcanine | Pokémon Mansion |
| Ariados | Cerulean Cave, Fuchsia City, Pallet Town, Pewter City and more |
| Azumarill | Mt. Moon, Route 1, Route 2, Route 22 and more |
| Beedrill | Celadon City, Cerulean City, Route 2, Route 24 and more |
| Bellsprout | Celadon City, Route 12, Route 13, Route 14 and more |
| Blastoise | Cerulean City, Pallet Town, Route 6, Vermilion City |
| Blissey | Cerulean Cave |
| Bulbasaur | Route 21, Route 25, Viridian Forest |
| Butterfree | Celadon City, Cerulean City, Fuchsia City, Lavender Town and more |
| Caterpie | Celadon City, Cerulean City, Fuchsia City, Lavender Town and more |
| Chansey | Cerulean Cave, Route 14, Route 15 |
| Charizard | Pokémon Mansion, Route 3, Route 4 |
| Charmander | Pokémon Mansion, Route 24, Route 3, Route 4 |
| Charmeleon | Pokémon Mansion, Route 24, Route 3, Route 4 |
| Chinchou | Cinnabar Island, Pallet Town, Route 11, Route 12 and more |
| Clefairy | Mt. Moon, Route 3, Route 4, Route 5 |
| Cleffa | Mt. Moon |
| Cloyster | Route 20, Seafoam Islands |
| Corsola | Cinnabar Island, Route 13, Route 21 |
| Crobat | Cerulean Cave, Seafoam Islands, Victory Road |
| Cubone | Cerulean Cave, Pokémon Tower, Rock Tunnel, Route 9 and more |
| Delibird | Seafoam Islands |
| Dewgong | Route 20, Seafoam Islands |
| Diglett | Diglett's Cave, Route 11, Vermilion City |
| Ditto | Cerulean Cave, Pokémon Mansion, Route 13, Route 14 and more |
| Dodrio | Cerulean Cave, Fuchsia City, Route 16, Route 17 and more |
| Doduo | Cerulean Cave, Fuchsia City, Route 16, Route 17 and more |
| Donphan | Route 23 |
| Dragonair | Cerulean Cave, Route 20, Route 23, Seafoam Islands and more |
| Dragonite | Cerulean Cave, Seafoam Islands, Victory Road |
| Dratini | Cerulean Cave, Route 20, Route 23, Seafoam Islands and more |
| Drowzee | Cerulean Cave, Route 11, Route 12, Route 5 and more |
| Dugtrio | Diglett's Cave, Vermilion City |
| Dunsparce | Diglett's Cave, Pewter City, Rock Tunnel, Route 4 |
| Eevee | Celadon City, Route 1, Route 16, Route 18 and more |
| Ekans | Route 10, Route 11, Route 23, Route 3 and more |
| Electabuzz | Power Plant, Route 10 |
| Electivire | Power Plant |
| Electrode | Cerulean Cave, Power Plant, Route 10, Vermilion City |
| Elekid | Power Plant, Route 10, Route 11, Route 6 and more |
| Exeggcute | Celadon City, Cerulean City, Fuchsia City, Lavender Town and more |
| Farfetch'd | Fuchsia City, Route 11, Route 12, Route 13 and more |
| Fearow | Pewter City, Route 10, Route 11, Route 16 and more |
| Furret | Route 1, Route 21, Route 22, Viridian City |
| Gastly | Celadon City, Cerulean Cave, Fuchsia City, Lavender Town and more |
| Gengar | Cerulean Cave, Pokémon Mansion, Pokémon Tower |
| Geodude | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Gligar | Route 23, Victory Road |
| Gliscor | Victory Road |
| Gloom | Celadon City, Fuchsia City, Route 1, Route 12 and more |
| Golbat | Cerulean Cave, Diglett's Cave, Fuchsia City, Mt. Moon and more |
| Goldeen | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Golduck | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Golem | Cerulean Cave, Victory Road |
| Granbull | Route 5, Route 6, Route 7, Route 8 |
| Graveler | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Grimer | Celadon City, Pokémon Mansion, Route 16, Route 17 and more |
| Growlithe | Pokémon Mansion, Route 7, Route 8 |
| Gyarados | Celadon City, Cerulean Cave, Cerulean City, Cinnabar Island and more |
| Haunter | Celadon City, Cerulean Cave, Fuchsia City, Lavender Town and more |
| Hitmonchan | Rock Tunnel, Victory Road |
| Hitmonlee | Rock Tunnel, Victory Road |
| Hitmontop | Victory Road |
| Honchkrow | Pokémon Tower |
| Hoothoot | Celadon City, Cerulean City, Fuchsia City, Pallet Town and more |
| Horsea | Cerulean Cave, Cinnabar Island, Pallet Town, Route 11 and more |
| Houndoom | Pokémon Mansion, Route 23 |
| Houndour | Celadon City, Pokémon Mansion, Route 10, Route 16 and more |
| Hypno | Cerulean Cave, Route 11, Route 12, Vermilion City |
| Igglybuff | Route 1, Route 22, Route 3 |
| Ivysaur | Route 21, Route 25, Viridian Forest |
| Jigglypuff | Celadon City, Cerulean Cave, Route 3, Route 4 and more |
| Jynx | Seafoam Islands |
| Kadabra | Cerulean Cave, Route 2, Route 24, Route 25 and more |
| Kakuna | Celadon City, Cerulean City, Route 2, Route 24 and more |
| Kangaskhan | Cerulean Cave, Rock Tunnel, Route 15, Route 9 |
| Kingdra | Cerulean Cave, Route 20, Seafoam Islands, Victory Road |
| Kingler | Cerulean City, Cinnabar Island, Pallet Town, Route 11 and more |
| Koffing | Celadon City, Fuchsia City, Pokémon Mansion |
| Krabby | Cerulean City, Cinnabar Island, Pallet Town, Route 11 and more |
| Lanturn | Cinnabar Island, Pallet Town, Route 11, Route 12 and more |
| Lapras | Route 20, Seafoam Islands |
| Ledian | Pallet Town, Route 1, Route 2, Route 22 and more |
| Ledyba | Pallet Town, Route 1, Route 2, Route 22 and more |
| Lickilicky | Cerulean Cave |
| Lickitung | Cerulean Cave, Route 16, Route 17, Route 18 |
| Machamp | Victory Road |
| Machoke | Rock Tunnel, Vermilion City, Victory Road |
| Machop | Rock Tunnel, Vermilion City, Victory Road |
| Magby | Pokémon Mansion |
| Magcargo | Pokémon Mansion |
| Magikarp | Celadon City, Cerulean Cave, Cerulean City, Cinnabar Island and more |
| Magmar | Pokémon Mansion |
| Magmortar | Pokémon Mansion |
| Magnemite | Cerulean Cave, Power Plant, Route 10, Route 11 and more |
| Magneton | Cerulean Cave, Power Plant, Route 10, Route 11 and more |
| Magnezone | Cerulean Cave, Power Plant |
| Mamoswine | Seafoam Islands |
| Mankey | Rock Tunnel, Route 22, Route 23, Route 3 and more |
| Marill | Mt. Moon, Route 1, Route 2, Route 22 and more |
| Marowak | Cerulean Cave, Pokémon Tower, Rock Tunnel, Route 9 and more |
| Meowth | Celadon City, Route 1, Route 11, Route 24 and more |
| Metapod | Celadon City, Cerulean City, Fuchsia City, Lavender Town and more |
| Mime Jr. | Route 5 |
| Misdreavus | Cerulean Cave, Pokémon Tower, Rock Tunnel, Victory Road |
| Mismagius | Pokémon Tower, Victory Road |
| Mr. Mime | Cerulean Cave, Route 14 |
| Muk | Celadon City, Pokémon Mansion, Route 16, Route 17 and more |
| Murkrow | Celadon City, Lavender Town, Pokémon Tower, Route 12 and more |
| Nidoran♀ | Fuchsia City, Route 1, Route 12, Route 13 and more |
| Nidoran♂ | Fuchsia City, Route 1, Route 12, Route 13 and more |
| Nidorina | Fuchsia City, Route 1, Route 12, Route 13 and more |
| Nidorino | Fuchsia City, Route 1, Route 12, Route 13 and more |
| Ninetales | Pokémon Mansion |
| Noctowl | Celadon City, Cerulean City, Fuchsia City, Pallet Town and more |
| Octillery | Cinnabar Island, Pallet Town, Route 19, Route 20 and more |
| Oddish | Celadon City, Fuchsia City, Route 1, Route 12 and more |
| Onix | Cerulean Cave, Diglett's Cave, Mt. Moon, Rock Tunnel and more |
| Paras | Cerulean Cave, Fuchsia City, Mt. Moon, Route 2 and more |
| Parasect | Cerulean Cave, Fuchsia City, Mt. Moon, Route 2 and more |
| Persian | Celadon City, Route 1, Route 11, Route 24 and more |
| Phanpy | Route 23 |
| Pichu | Route 2, Route 24, Route 7, Viridian City and more |
| Pidgeot | Celadon City, Lavender Town, Pallet Town, Route 1 and more |
| Pidgeotto | Celadon City, Lavender Town, Pallet Town, Route 1 and more |
| Pidgey | Celadon City, Lavender Town, Pallet Town, Route 1 and more |
| Pikachu | Cerulean Cave, Power Plant, Route 10, Route 2 and more |
| Piloswine | Seafoam Islands |
| Pineco | Cerulean City, Pallet Town, Route 1, Route 12 and more |
| Pinsir | Route 25, Route 9 |
| Politoed | Cerulean Cave, Route 23, Victory Road |
| Poliwag | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Poliwhirl | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Poliwrath | Cerulean Cave, Route 23, Victory Road |
| Ponyta | Pokémon Mansion, Route 16, Route 17, Route 22 and more |
| Porygon | Celadon City, Pokémon Mansion, Power Plant |
| Primeape | Rock Tunnel, Route 22, Route 23, Route 3 and more |
| Psyduck | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Quagsire | Route 22, Route 23, Victory Road |
| Qwilfish | Route 12, Route 13, Route 21 |
| Raichu | Power Plant |
| Rapidash | Pokémon Mansion, Route 16, Route 17 |
| Raticate | Pokémon Mansion, Route 1, Route 11, Route 16 and more |
| Rattata | Pokémon Mansion, Route 1, Route 11, Route 16 and more |
| Remoraid | Cinnabar Island, Pallet Town, Route 19, Route 20 and more |
| Rhydon | Cerulean Cave, Route 23, Victory Road |
| Rhyhorn | Cerulean Cave, Fuchsia City, Rock Tunnel, Route 10 and more |
| Rhyperior | Cerulean Cave, Victory Road |
| Sandshrew | Cerulean Cave, Diglett's Cave, Mt. Moon, Pewter City and more |
| Sandslash | Cerulean Cave, Diglett's Cave, Mt. Moon, Pewter City and more |
| Scyther | Route 15, Route 25 |
| Seadra | Cerulean Cave, Cinnabar Island, Pallet Town, Route 11 and more |
| Seaking | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Seel | Route 20, Seafoam Islands |
| Sentret | Route 1, Route 21, Route 22, Viridian City |
| Shellder | Cinnabar Island, Pallet Town, Route 11, Route 12 and more |
| Shuckle | Cerulean Cave, Route 10, Route 9 |
| Skarmory | Route 23, Victory Road |
| Slowbro | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Slowking | Cerulean Cave, Seafoam Islands, Victory Road |
| Slowpoke | Celadon City, Cerulean Cave, Cerulean City, Diglett's Cave and more |
| Slugma | Pokémon Mansion |
| Smoochum | Seafoam Islands |
| Sneasel | Route 23, Seafoam Islands |
| Snorlax | Cerulean Cave, Route 23, Victory Road |
| Snubbull | Route 5, Route 6, Route 7, Route 8 |
| Spearow | Pewter City, Route 10, Route 11, Route 16 and more |
| Spinarak | Cerulean Cave, Fuchsia City, Pallet Town, Pewter City and more |
| Squirtle | Cerulean City, Pallet Town, Route 19, Route 6 and more |
| Starmie | Cerulean Cave, Route 20 |
| Staryu | Cerulean Cave, Cerulean City, Cinnabar Island, Pallet Town and more |
| Steelix | Cerulean Cave, Victory Road |
| Sunkern | Route 21, Route 24, Route 3 |
| Swinub | Seafoam Islands |
| Tangela | Celadon City, Fuchsia City, Route 21 |
| Tauros | Route 10, Route 14, Route 23 |
| Teddiursa | Route 23 |
| Tentacool | Cinnabar Island, Pallet Town, Route 11, Route 12 and more |
| Tentacruel | Cinnabar Island, Pallet Town, Route 11, Route 12 and more |
| Tyrogue | Rock Tunnel, Victory Road |
| Ursaring | Route 23 |
| Venomoth | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Venonat | Celadon City, Cerulean Cave, Cerulean City, Fuchsia City and more |
| Voltorb | Cerulean Cave, Power Plant, Route 10, Vermilion City |
| Vulpix | Pokémon Mansion, Route 7, Route 8 |
| Wartortle | Cerulean City, Pallet Town, Route 19, Route 6 and more |
| Weavile | Seafoam Islands |
| Weedle | Celadon City, Cerulean City, Route 2, Route 24 and more |
| Weepinbell | Celadon City, Route 12, Route 13, Route 14 and more |
| Weezing | Celadon City, Fuchsia City, Pokémon Mansion |
| Wigglytuff | Cerulean Cave |
| Wobbuffet | Cerulean Cave |
| Wooper | Route 22, Route 23, Victory Road |
| Yanma | Route 12, Route 2, Route 21 |
| Zubat | Cerulean Cave, Diglett's Cave, Fuchsia City, Mt. Moon and more |
