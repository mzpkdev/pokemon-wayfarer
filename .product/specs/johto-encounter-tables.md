# Johto encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Kanto and Johto encounter rules](kanto-johto-encounters.md). Every pick is a
placeholder for playtesting.

## Scope

This spec lists Johto's wild-encounter tables: 87 maps, each with a day and a
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

#### New Bark Town

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Sentret–Furret, Ledyba–Ledian | Oddish–Gloom, Sentret–Furret |
| Uncommon | Pidgey–Pidgeot, Hoppip–Jumpluff, Rattata–Raticate, Sunkern | Hoothoot–Noctowl, Rattata–Raticate, Spinarak–Ariados, Marill–Azumarill |
| Rare | Caterpie–Butterfree, Marill–Azumarill, Pidgey–Pidgeot, Sentret–Furret | Oddish–Gloom, Hoothoot–Noctowl, Chikorita–Meganium, Rattata–Raticate |
| Very rare | Togepi, Chikorita–Meganium | Togepi, Spinarak–Ariados |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Chinchou–Lanturn |
| Uncommon | Chinchou–Lanturn | Tentacool–Tentacruel |
| Rare | Shellder, Krabby–Kingler | Staryu, Shellder |
| Very rare | Remoraid–Octillery | Remoraid–Octillery |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Chinchou–Lanturn, Tentacool–Tentacruel | Chinchou–Lanturn, Tentacool–Tentacruel |
| Uncommon | Magikarp–Gyarados, Shellder, Chinchou–Lanturn | Magikarp–Gyarados, Staryu, Chinchou–Lanturn |
| Rare | Krabby–Kingler, Corsola, Tentacool–Tentacruel | Shellder, Staryu, Krabby–Kingler |
| Very rare | Qwilfish, Chinchou–Lanturn | Corsola, Qwilfish |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian | Ledyba–Ledian |
| Uncommon | Caterpie–Butterfree | Spinarak–Ariados |
| Rare | Pineco–Forretress, Exeggcute | Caterpie–Butterfree, Pineco–Forretress |
| Very rare | Aipom | Hoothoot–Noctowl |

#### Cherrygrove City

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot, Spearow–Fearow | Rattata–Raticate, Meowth–Persian |
| Uncommon | Rattata–Raticate, Ledyba–Ledian, Hoppip–Jumpluff, Sentret–Furret | Hoothoot–Noctowl, Oddish–Gloom, Spinarak–Ariados, Gastly–Haunter |
| Rare | Sunkern, Caterpie–Butterfree, Spearow–Fearow, Pidgey–Pidgeot | Rattata–Raticate, Oddish–Gloom, Hoothoot–Noctowl, Meowth–Persian |
| Very rare | Eevee, Sunkern | Eevee, Gastly–Haunter |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Krabby–Kingler | Krabby–Kingler |
| Rare | Corsola, Shellder | Staryu, Shellder |
| Very rare | Qwilfish | Qwilfish |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler, Tentacool–Tentacruel | Krabby–Kingler, Tentacool–Tentacruel |
| Uncommon | Magikarp–Gyarados, Corsola, Krabby–Kingler | Magikarp–Gyarados, Staryu, Krabby–Kingler |
| Rare | Magikarp–Gyarados, Corsola, Tentacool–Tentacruel | Magikarp–Gyarados, Staryu, Tentacool–Tentacruel |
| Very rare | Qwilfish, Krabby–Kingler | Staryu, Krabby–Kingler |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow | Spearow–Fearow |
| Uncommon | Pidgey–Pidgeot | Pidgey–Pidgeot |
| Rare | Exeggcute, Ledyba–Ledian | Spinarak–Ariados, Exeggcute |
| Very rare | Aipom | Aipom |

#### Violet City

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell, Ledyba–Ledian | Bellsprout–Weepinbell, Gastly–Haunter |
| Uncommon | Pidgey–Pidgeot, Rattata–Raticate, Mareep–Ampharos, Hoppip–Jumpluff | Rattata–Raticate, Oddish–Gloom, Spinarak–Ariados, Hoothoot–Noctowl |
| Rare | Sunkern, Natu–Xatu, Bellsprout–Weepinbell, Ledyba–Ledian | Zubat–Golbat, Gastly–Haunter, Hoothoot–Noctowl, Oddish–Gloom |
| Very rare | Abra, Sunkern | Abra, Gastly–Haunter |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Marill–Azumarill, Psyduck–Golduck | Psyduck–Golduck, Wooper–Quagsire |
| Very rare | Magikarp–Gyarados | Magikarp–Gyarados |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp–Gyarados ×2, Marill–Azumarill | Magikarp–Gyarados ×2, Wooper–Quagsire |
| Rare | Poliwag–Poliwhirl, Goldeen–Seaking, Psyduck–Golduck | Wooper–Quagsire, Goldeen–Seaking, Marill–Azumarill |
| Very rare | Poliwag–Poliwhirl, Wooper–Quagsire | Poliwag–Poliwhirl, Psyduck–Golduck |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian | Bellsprout–Weepinbell |
| Uncommon | Bellsprout–Weepinbell | Ledyba–Ledian |
| Rare | Exeggcute, Pineco–Forretress | Spinarak–Ariados, Hoothoot–Noctowl |
| Very rare | Aipom | Aipom |

#### Azalea Town

Road, Johto west.

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pineco–Forretress | Pineco–Forretress |
| Uncommon | Aipom | Aipom |
| Rare | Caterpie–Butterfree, Weedle–Beedrill | Spinarak–Ariados, Hoothoot–Noctowl |
| Very rare | Exeggcute | Murkrow |

#### Goldenrod City

Road, Johto west.

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian | Ledyba–Ledian |
| Uncommon | Exeggcute | Exeggcute |
| Rare | Pineco–Forretress, Aipom | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Caterpie–Butterfree | Murkrow |

#### Ecruteak City

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Sentret–Furret, Hoppip–Jumpluff | Gastly–Haunter, Sentret–Furret |
| Uncommon | Vulpix, Pidgey–Pidgeot, Growlithe, Sunkern | Hoothoot–Noctowl, Vulpix, Oddish–Gloom, Spinarak–Ariados |
| Rare | Ledyba–Ledian, Caterpie–Butterfree, Eevee, Sentret–Furret | Misdreavus, Gastly–Haunter, Eevee, Oddish–Gloom |
| Very rare | Eevee, Hoppip–Jumpluff | Houndour–Houndoom, Murkrow |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill | Psyduck–Golduck |
| Uncommon | Psyduck–Golduck | Marill–Azumarill |
| Rare | Poliwag–Poliwhirl, Magikarp–Gyarados | Wooper–Quagsire, Poliwag–Poliwhirl |
| Very rare | Wooper–Quagsire | Magikarp–Gyarados |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck, Marill–Azumarill | Marill–Azumarill, Psyduck–Golduck |
| Uncommon | Magikarp–Gyarados, Poliwag–Poliwhirl, Goldeen–Seaking | Magikarp–Gyarados, Wooper–Quagsire, Goldeen–Seaking |
| Rare | Psyduck–Golduck, Magikarp–Gyarados, Marill–Azumarill | Poliwag–Poliwhirl, Wooper–Quagsire, Magikarp–Gyarados |
| Very rare | Poliwag–Poliwhirl, Wooper–Quagsire | Marill–Azumarill, Psyduck–Golduck |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Exeggcute | Exeggcute |
| Uncommon | Ledyba–Ledian | Ledyba–Ledian |
| Rare | Pineco–Forretress, Aipom | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Caterpie–Butterfree | Hoothoot–Noctowl |

#### Olivine City

Road, Johto west.

**`MAP_OLIVINE_CITY_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Meowth–Persian, Hoppip–Jumpluff | Meowth–Persian, Magnemite–Magneton |
| Uncommon | Magnemite–Magneton, Spearow–Fearow, Sunkern, Rattata–Raticate | Gastly–Haunter, Hoothoot–Noctowl, Grimer–Muk, Oddish–Gloom |
| Rare | Grimer–Muk, Snubbull–Granbull, Spearow–Fearow, Sunkern | Gastly–Haunter, Meowth–Persian, Hoothoot–Noctowl, Rattata–Raticate |
| Very rare | Farfetch'd, Magnemite–Magneton | Farfetch'd, Spinarak–Ariados |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Remoraid–Octillery | Remoraid–Octillery |
| Uncommon | Corsola | Corsola |
| Rare | Tentacool–Tentacruel, Shellder | Staryu, Chinchou–Lanturn |
| Very rare | Mantyke | Tentacool–Tentacruel |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Corsola, Remoraid–Octillery | Remoraid–Octillery, Corsola |
| Uncommon | Krabby–Kingler, Magikarp–Gyarados, Corsola | Staryu ×2, Krabby–Kingler |
| Rare | Krabby–Kingler, Shellder, Qwilfish | Magikarp–Gyarados, Chinchou–Lanturn, Staryu |
| Very rare | Corsola, Tentacool–Tentacruel | Qwilfish, Chinchou–Lanturn |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Exeggcute | Exeggcute |
| Uncommon | Spearow–Fearow | Spearow–Fearow |
| Rare | Pineco–Forretress, Ledyba–Ledian | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Aipom | Hoothoot–Noctowl |

**`MAP_OLIVINE_CITY_PORT_OUTSIDE_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Hoppip–Jumpluff, Krabby–Kingler | Krabby–Kingler, Meowth–Persian |
| Uncommon | Pidgey–Pidgeot, Snubbull–Granbull, Sunkern, Meowth–Persian | Hoothoot–Noctowl, Gastly–Haunter, Oddish–Gloom, Snubbull–Granbull |
| Rare | Grimer–Muk, Spearow–Fearow, Ditto, Krabby–Kingler | Slowpoke–Slowbro, Hoothoot–Noctowl, Ditto, Gastly–Haunter |
| Very rare | Farfetch'd, Slowpoke–Slowbro | Farfetch'd, Spinarak–Ariados |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Remoraid–Octillery |
| Uncommon | Remoraid–Octillery | Tentacool–Tentacruel |
| Rare | Corsola, Chinchou–Lanturn | Chinchou–Lanturn, Staryu |
| Very rare | Qwilfish | Corsola |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Remoraid–Octillery, Tentacool–Tentacruel | Remoraid–Octillery, Tentacool–Tentacruel |
| Uncommon | Corsola, Chinchou–Lanturn, Qwilfish | Chinchou–Lanturn, Corsola, Staryu |
| Rare | Krabby–Kingler, Shellder, Tentacool–Tentacruel | Chinchou–Lanturn, Staryu, Krabby–Kingler |
| Very rare | Chinchou–Lanturn, Corsola | Qwilfish, Magikarp–Gyarados |

#### Cianwood City

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Mankey–Primeape, Hoppip–Jumpluff | Machop–Machoke, Mankey–Primeape |
| Uncommon | Machop–Machoke, Krabby–Kingler, Sunkern, Spearow–Fearow | Hoothoot–Noctowl, Gastly–Haunter, Oddish–Gloom, Spinarak–Ariados |
| Rare | Tyrogue ×2, Geodude–Graveler, Mankey–Primeape | Zubat–Golbat, Hoothoot–Noctowl, Tyrogue, Gastly–Haunter |
| Very rare | Ditto, Machop–Machoke | Ditto, Oddish–Gloom |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Chinchou–Lanturn | Chinchou–Lanturn |
| Uncommon | Remoraid–Octillery | Remoraid–Octillery |
| Rare | Shellder, Staryu | Staryu ×2 |
| Very rare | Mantyke | Shellder |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Chinchou–Lanturn, Remoraid–Octillery | Chinchou–Lanturn, Remoraid–Octillery |
| Uncommon | Shellder, Corsola, Chinchou–Lanturn | Staryu, Corsola, Chinchou–Lanturn |
| Rare | Qwilfish, Krabby–Kingler, Tentacool–Tentacruel | Staryu, Shellder, Krabby–Kingler |
| Very rare | Staryu, Chinchou–Lanturn | Staryu, Qwilfish |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler | Krabby–Kingler |
| Uncommon | Mankey–Primeape | Mankey–Primeape |
| Rare | Geodude–Graveler, Aipom | Spinarak–Ariados, Geodude–Graveler |
| Very rare | Exeggcute | Aipom |

#### Mahogany Town

Road, Johto east.

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Aipom | Aipom |
| Uncommon | Pineco–Forretress | Pineco–Forretress |
| Rare | Ledyba–Ledian, Exeggcute | Spinarak–Ariados, Hoothoot–Noctowl |
| Very rare | Pidgey–Pidgeot | Murkrow |

#### Blackthorn City

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow, Phanpy–Donphan | Phanpy–Donphan, Oddish–Gloom |
| Uncommon | Hoppip–Jumpluff, Geodude–Graveler, Pidgey–Pidgeot, Sunkern | Hoothoot–Noctowl, Gastly–Haunter, Zubat–Golbat, Swinub–Piloswine |
| Rare | Swinub–Piloswine, Machop–Machoke, Gligar, Spearow–Fearow | Sneasel, Hoothoot–Noctowl, Zubat–Golbat, Gastly–Haunter |
| Very rare | Gligar, Swinub–Piloswine | Sneasel, Gligar |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados | Magikarp–Gyarados |
| Uncommon | Horsea–Seadra | Horsea–Seadra |
| Rare | Poliwag–Poliwhirl, Marill–Azumarill | Wooper–Quagsire, Poliwag–Poliwhirl |
| Very rare | Psyduck–Golduck | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl, Horsea–Seadra | Poliwag–Poliwhirl, Horsea–Seadra |
| Uncommon | Magikarp–Gyarados ×2, Poliwag–Poliwhirl | Magikarp–Gyarados, Wooper–Quagsire, Poliwag–Poliwhirl |
| Rare | Horsea–Seadra, Poliwag–Poliwhirl, Goldeen–Seaking | Horsea–Seadra, Wooper–Quagsire, Magikarp–Gyarados |
| Very rare | Magikarp–Gyarados, Marill–Azumarill | Goldeen–Seaking, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow | Spinarak–Ariados |
| Uncommon | Spinarak–Ariados | Spearow–Fearow |
| Rare | Pineco–Forretress, Aipom | Hoothoot–Noctowl, Pineco–Forretress |
| Very rare | Exeggcute | Aipom |

#### Safari Zone Gate

Road, Johto west.

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow | Exeggcute |
| Uncommon | Exeggcute | Venonat–Venomoth |
| Rare | Aipom, Pineco–Forretress | Aipom, Pineco–Forretress |
| Very rare | Ledyba–Ledian | Murkrow |

#### Route 29

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot, Sentret–Furret | Hoothoot–Noctowl, Rattata–Raticate |
| Uncommon | Rattata–Raticate, Hoppip–Jumpluff, Ledyba–Ledian, Spearow–Fearow | Oddish–Gloom, Spinarak–Ariados, Venonat–Venomoth, Hoothoot–Noctowl |
| Rare | Caterpie–Butterfree, Sunkern, Pidgey–Pidgeot, Sentret–Furret | Gastly–Haunter ×2, Rattata–Raticate, Oddish–Gloom |
| Very rare | Chikorita–Meganium, Hoppip–Jumpluff | Chikorita–Meganium, Spinarak–Ariados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian | Hoothoot–Noctowl |
| Uncommon | Hoothoot–Noctowl | Ledyba–Ledian |
| Rare | Pineco–Forretress, Exeggcute | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Aipom | Aipom |

#### Route 30

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian, Pidgey–Pidgeot | Hoothoot–Noctowl, Spinarak–Ariados |
| Uncommon | Caterpie–Butterfree, Weedle–Beedrill, Hoppip–Jumpluff, Rattata–Raticate | Poliwag–Poliwhirl, Oddish–Gloom, Rattata–Raticate, Venonat–Venomoth |
| Rare | Sunkern, Sentret–Furret, Ledyba–Ledian, Weedle–Beedrill | Spinarak–Ariados, Gastly–Haunter, Hoothoot–Noctowl, Zubat–Golbat |
| Very rare | Abra, Pidgey–Pidgeot | Abra, Gastly–Haunter |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Marill–Azumarill, Magikarp–Gyarados | Wooper–Quagsire, Marill–Azumarill |
| Very rare | Totodile–Feraligatr | Totodile–Feraligatr |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl ×2 | Poliwag–Poliwhirl ×2 |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Magikarp–Gyarados ×2, Wooper–Quagsire |
| Rare | Poliwag–Poliwhirl, Marill–Azumarill, Goldeen–Seaking | Poliwag–Poliwhirl, Wooper–Quagsire, Goldeen–Seaking |
| Very rare | Psyduck–Golduck, Poliwag–Poliwhirl | Marill–Azumarill, Poliwag–Poliwhirl |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian | Spinarak–Ariados |
| Uncommon | Spinarak–Ariados | Hoothoot–Noctowl |
| Rare | Exeggcute, Pineco–Forretress | Ledyba–Ledian, Pineco–Forretress |
| Very rare | Aipom | Aipom |

#### Route 31

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell, Ledyba–Ledian | Spinarak–Ariados, Oddish–Gloom |
| Uncommon | Caterpie–Butterfree, Weedle–Beedrill, Pidgey–Pidgeot, Mareep–Ampharos | Bellsprout–Weepinbell, Gastly–Haunter, Zubat–Golbat, Hoothoot–Noctowl |
| Rare | Sunkern, Hoppip–Jumpluff, Bellsprout–Weepinbell, Weedle–Beedrill | Spinarak–Ariados, Poliwag–Poliwhirl, Gastly–Haunter, Hoothoot–Noctowl |
| Very rare | Pichu, Mareep–Ampharos | Zubat–Golbat, Dunsparce |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Marill–Azumarill | Marill–Azumarill |
| Rare | Poliwag–Poliwhirl, Magikarp–Gyarados | Wooper–Quagsire, Poliwag–Poliwhirl |
| Very rare | Psyduck–Golduck | Magikarp–Gyarados |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl, Marill–Azumarill | Poliwag–Poliwhirl, Marill–Azumarill |
| Uncommon | Magikarp–Gyarados ×2, Poliwag–Poliwhirl | Magikarp–Gyarados ×2, Wooper–Quagsire |
| Rare | Goldeen–Seaking, Poliwag–Poliwhirl, Marill–Azumarill | Poliwag–Poliwhirl, Wooper–Quagsire, Goldeen–Seaking |
| Very rare | Psyduck–Golduck, Poliwag–Poliwhirl | Marill–Azumarill, Poliwag–Poliwhirl |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian | Spinarak–Ariados |
| Uncommon | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| Rare | Exeggcute, Pineco–Forretress | Hoothoot–Noctowl, Pineco–Forretress |
| Very rare | Aipom | Aipom |

#### Route 32

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Mareep–Ampharos, Wooper–Quagsire | Wooper–Quagsire, Mareep–Ampharos |
| Uncommon | Ekans–Arbok, Pidgey–Pidgeot, Bellsprout–Weepinbell, Hoppip–Jumpluff | Zubat–Golbat, Gastly–Haunter, Spinarak–Ariados, Oddish–Gloom |
| Rare | Mareep–Ampharos, Sunkern, Ekans–Arbok, Wooper–Quagsire | Gastly–Haunter, Zubat–Golbat, Hoothoot–Noctowl, Oddish–Gloom |
| Very rare | Mareep–Ampharos, Hoppip–Jumpluff | Murkrow, Ekans–Arbok |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Wooper–Quagsire |
| Uncommon | Qwilfish | Qwilfish |
| Rare | Tentacool–Tentacruel, Marill–Azumarill | Chinchou–Lanturn, Tentacool–Tentacruel |
| Very rare | Tentacool–Tentacruel | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Qwilfish ×2 | Qwilfish ×2 |
| Uncommon | Magikarp–Gyarados, Tentacool–Tentacruel, Qwilfish | Magikarp–Gyarados, Chinchou–Lanturn, Qwilfish |
| Rare | Wooper–Quagsire, Magikarp–Gyarados, Tentacool–Tentacruel | Wooper–Quagsire, Chinchou–Lanturn, Tentacool–Tentacruel |
| Very rare | Qwilfish, Remoraid–Octillery | Qwilfish, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ekans–Arbok | Ekans–Arbok |
| Uncommon | Ekans–Arbok | Ekans–Arbok |
| Rare | Pineco–Forretress, Exeggcute | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Aipom | Hoothoot–Noctowl |

#### Route 33

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Machop–Machoke, Rattata–Raticate | Rattata–Raticate, Machop–Machoke |
| Uncommon | Ekans–Arbok, Spearow–Fearow, Hoppip–Jumpluff, Geodude–Graveler | Zubat–Golbat, Gastly–Haunter, Hoothoot–Noctowl, Oddish–Gloom |
| Rare | Wooper–Quagsire, Machop–Machoke, Ekans–Arbok, Rattata–Raticate | Zubat–Golbat, Spinarak–Ariados, Gastly–Haunter, Hoothoot–Noctowl |
| Very rare | Slowpoke–Slowbro, Hoppip–Jumpluff | Dunsparce, Wooper–Quagsire |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Aipom | Aipom |
| Uncommon | Ekans–Arbok | Ekans–Arbok |
| Rare | Pineco–Forretress, Exeggcute | Venonat–Venomoth, Pineco–Forretress |
| Very rare | Spearow–Fearow | Spinarak–Ariados |

#### Route 34

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Snubbull–Granbull, Pidgey–Pidgeot | Drowzee–Hypno, Grimer–Muk |
| Uncommon | Jigglypuff, Drowzee–Hypno, Sunkern, Hoppip–Jumpluff | Hoothoot–Noctowl, Snubbull–Granbull, Oddish–Gloom, Spinarak–Ariados |
| Rare | Abra–Kadabra, Ditto, Igglybuff, Snubbull–Granbull | Abra–Kadabra, Ditto, Igglybuff, Gastly–Haunter |
| Very rare | Ditto, Eevee | Ditto, Hoothoot–Noctowl |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Corsola | Corsola |
| Rare | Krabby–Kingler, Chinchou–Lanturn | Staryu, Chinchou–Lanturn |
| Very rare | Tentacool–Tentacruel | Krabby–Kingler |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Corsola, Tentacool–Tentacruel | Corsola, Tentacool–Tentacruel |
| Uncommon | Krabby–Kingler, Corsola, Magikarp–Gyarados | Staryu, Krabby–Kingler, Corsola |
| Rare | Krabby–Kingler, Corsola, Chinchou–Lanturn | Staryu, Chinchou–Lanturn, Krabby–Kingler |
| Very rare | Qwilfish, Corsola | Corsola, Staryu |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot | Exeggcute |
| Uncommon | Exeggcute | Pidgey–Pidgeot |
| Rare | Ledyba–Ledian, Pineco–Forretress | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Aipom | Hoothoot–Noctowl |

#### Route 35

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Nidoran♀–Nidorina, Snubbull–Granbull | Psyduck–Golduck, Nidoran♂–Nidorino |
| Uncommon | Pidgey–Pidgeot, Hoppip–Jumpluff, Nidoran♂–Nidorino, Sunkern | Drowzee–Hypno, Hoothoot–Noctowl, Oddish–Gloom, Venonat–Venomoth |
| Rare | Yanma, Abra–Kadabra, Growlithe, Ditto | Yanma, Abra–Kadabra, Hoothoot–Noctowl, Ditto |
| Very rare | Yanma, Growlithe | Oddish–Gloom, Psyduck–Golduck |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Magikarp–Gyarados | Magikarp–Gyarados |
| Rare | Marill–Azumarill, Wooper–Quagsire | Wooper–Quagsire ×2 |
| Very rare | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados, Psyduck–Golduck | Magikarp–Gyarados, Psyduck–Golduck |
| Uncommon | Marill–Azumarill, Magikarp–Gyarados, Wooper–Quagsire | Wooper–Quagsire ×2, Magikarp–Gyarados |
| Rare | Poliwag–Poliwhirl, Marill–Azumarill, Magikarp–Gyarados | Marill–Azumarill, Wooper–Quagsire, Poliwag–Poliwhirl |
| Very rare | Psyduck–Golduck, Goldeen–Seaking | Psyduck–Golduck, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pidgey–Pidgeot | Pidgey–Pidgeot |
| Uncommon | Caterpie–Butterfree | Spinarak–Ariados |
| Rare | Ledyba–Ledian, Pineco–Forretress | Hoothoot–Noctowl, Pineco–Forretress |
| Very rare | Aipom | Aipom |

#### Route 36

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell, Growlithe | Hoothoot–Noctowl, Vulpix |
| Uncommon | Hoppip–Jumpluff, Pidgey–Pidgeot, Vulpix, Sentret–Furret | Gastly–Haunter, Houndour–Houndoom, Oddish–Gloom, Rattata–Raticate |
| Rare | Sudowoodo, Sunkern, Bonsly, Ledyba–Ledian | Sudowoodo, Houndour–Houndoom, Spinarak–Ariados, Hoothoot–Noctowl |
| Very rare | Sudowoodo, Nidoran♂–Nidorino | Vulpix, Gastly–Haunter |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell | Hoothoot–Noctowl |
| Uncommon | Hoothoot–Noctowl | Bellsprout–Weepinbell |
| Rare | Exeggcute, Pineco–Forretress | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Aipom | Aipom |

#### Route 37

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian, Pidgey–Pidgeot | Hoothoot–Noctowl, Spinarak–Ariados |
| Uncommon | Hoppip–Jumpluff, Sunkern, Vulpix, Growlithe | Venonat–Venomoth, Vulpix, Gastly–Haunter, Oddish–Gloom |
| Rare | Sentret–Furret, Ledyba–Ledian, Eevee, Pidgey–Pidgeot | Houndour–Houndoom, Hoothoot–Noctowl ×2, Spinarak–Ariados |
| Very rare | Vulpix, Growlithe | Misdreavus, Gastly–Haunter |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ledyba–Ledian | Spinarak–Ariados |
| Uncommon | Pidgey–Pidgeot | Hoothoot–Noctowl |
| Rare | Exeggcute, Pineco–Forretress | Ledyba–Ledian, Pineco–Forretress |
| Very rare | Aipom | Aipom |

#### Route 38

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Rattata–Raticate, Snubbull–Granbull | Hoothoot–Noctowl, Magnemite–Magneton |
| Uncommon | Magnemite–Magneton, Pidgey–Pidgeot, Sentret–Furret, Meowth–Persian | Meowth–Persian, Rattata–Raticate, Gastly–Haunter, Oddish–Gloom |
| Rare | Farfetch'd, Sunkern, Magnemite–Magneton, Pidgey–Pidgeot | Hoothoot–Noctowl, Gastly–Haunter ×2, Snubbull–Granbull |
| Very rare | Farfetch'd, Snubbull–Granbull | Farfetch'd, Venonat–Venomoth |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Hoothoot–Noctowl | Hoothoot–Noctowl |
| Uncommon | Caterpie–Butterfree | Venonat–Venomoth |
| Rare | Exeggcute, Pineco–Forretress | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Aipom | Exeggcute |

#### Route 39

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ponyta–Rapidash, Snubbull–Granbull | Hoothoot–Noctowl, Magnemite–Magneton |
| Uncommon | Magnemite–Magneton, Pidgey–Pidgeot, Sentret–Furret, Meowth–Persian | Gastly–Haunter, Meowth–Persian, Rattata–Raticate, Oddish–Gloom |
| Rare | Magnemite–Magneton, Ponyta–Rapidash, Farfetch'd, Sunkern | Hoothoot–Noctowl, Gastly–Haunter, Spinarak–Ariados, Ponyta–Rapidash |
| Very rare | Eevee, Magnemite–Magneton | Eevee, Venonat–Venomoth |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Hoothoot–Noctowl | Hoothoot–Noctowl |
| Uncommon | Weedle–Beedrill | Weedle–Beedrill |
| Rare | Exeggcute, Pineco–Forretress | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Ledyba–Ledian | Exeggcute |

#### Route 40

Road, Johto west.

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Corsola | Corsola |
| Rare | Shellder, Staryu | Staryu, Chinchou–Lanturn |
| Very rare | Qwilfish | Shellder |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler, Corsola | Krabby–Kingler, Corsola |
| Uncommon | Magikarp–Gyarados, Shellder, Corsola | Staryu ×2, Magikarp–Gyarados |
| Rare | Krabby–Kingler, Tentacool–Tentacruel, Qwilfish | Shellder, Chinchou–Lanturn, Krabby–Kingler |
| Very rare | Staryu, Remoraid–Octillery | Tentacool–Tentacruel, Staryu |

#### Route 41

Road, Johto west.

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Horsea–Seadra | Horsea–Seadra |
| Rare | Mantyke, Chinchou–Lanturn | Mantyke, Chinchou–Lanturn |
| Very rare | Qwilfish | Staryu |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Chinchou–Lanturn, Horsea–Seadra | Chinchou–Lanturn, Horsea–Seadra |
| Uncommon | Qwilfish, Chinchou–Lanturn, Magikarp–Gyarados | Chinchou–Lanturn ×2, Staryu |
| Rare | Horsea–Seadra, Corsola, Chinchou–Lanturn | Magikarp–Gyarados, Staryu, Horsea–Seadra |
| Very rare | Krabby–Kingler, Staryu | Qwilfish, Staryu |

#### Route 42

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow, Mankey–Primeape | Marill–Azumarill, Mankey–Primeape |
| Uncommon | Ekans–Arbok, Rattata–Raticate, Marill–Azumarill, Geodude–Graveler | Zubat–Golbat, Hoothoot–Noctowl, Gastly–Haunter, Spinarak–Ariados |
| Rare | Machop–Machoke, Ekans–Arbok, Gligar, Mankey–Primeape | Zubat–Golbat, Oddish–Gloom, Gligar, Hoothoot–Noctowl |
| Very rare | Tyrogue, Spearow–Fearow | Zubat–Golbat, Tyrogue |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill | Marill–Azumarill |
| Uncommon | Remoraid–Octillery | Remoraid–Octillery |
| Rare | Goldeen–Seaking, Magikarp–Gyarados | Wooper–Quagsire, Goldeen–Seaking |
| Very rare | Poliwag–Poliwhirl | Magikarp–Gyarados |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Remoraid–Octillery, Marill–Azumarill | Remoraid–Octillery, Marill–Azumarill |
| Uncommon | Goldeen–Seaking ×2, Magikarp–Gyarados | Goldeen–Seaking, Wooper–Quagsire, Magikarp–Gyarados |
| Rare | Poliwag–Poliwhirl, Magikarp–Gyarados, Remoraid–Octillery | Wooper–Quagsire, Goldeen–Seaking, Poliwag–Poliwhirl |
| Very rare | Goldeen–Seaking, Marill–Azumarill | Remoraid–Octillery, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow | Aipom |
| Uncommon | Aipom | Spearow–Fearow |
| Rare | Pineco–Forretress, Exeggcute | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Ledyba–Ledian | Hoothoot–Noctowl |

#### Route 43

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Mareep–Ampharos, Sentret–Furret | Venonat–Venomoth, Mareep–Ampharos |
| Uncommon | Pidgey–Pidgeot ×2, Hoppip–Jumpluff, Rattata–Raticate | Hoothoot–Noctowl, Rattata–Raticate, Oddish–Gloom, Gastly–Haunter |
| Rare | Farfetch'd, Mareep–Ampharos, Sentret–Furret, Sunkern | Venonat–Venomoth, Hoothoot–Noctowl ×2, Spinarak–Ariados |
| Very rare | Farfetch'd, Hoppip–Jumpluff | Farfetch'd, Zubat–Golbat |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados | Magikarp–Gyarados |
| Uncommon | Magikarp–Gyarados | Magikarp–Gyarados |
| Rare | Poliwag–Poliwhirl, Marill–Azumarill | Wooper–Quagsire, Poliwag–Poliwhirl |
| Very rare | Psyduck–Golduck | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados ×2 | Magikarp–Gyarados ×2 |
| Uncommon | Poliwag–Poliwhirl, Magikarp–Gyarados, Goldeen–Seaking | Poliwag–Poliwhirl, Wooper–Quagsire, Magikarp–Gyarados |
| Rare | Psyduck–Golduck, Marill–Azumarill, Poliwag–Poliwhirl | Goldeen–Seaking, Wooper–Quagsire, Poliwag–Poliwhirl |
| Very rare | Wooper–Quagsire, Poliwag–Poliwhirl | Marill–Azumarill, Psyduck–Golduck |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Exeggcute | Venonat–Venomoth |
| Uncommon | Venonat–Venomoth | Exeggcute |
| Rare | Pineco–Forretress, Aipom | Hoothoot–Noctowl, Pineco–Forretress |
| Very rare | Ledyba–Ledian | Aipom |

#### Route 44

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tangela, Bellsprout–Weepinbell | Poliwag–Poliwhirl, Bellsprout–Weepinbell |
| Uncommon | Lickitung, Pidgey–Pidgeot, Hoppip–Jumpluff, Poliwag–Poliwhirl | Oddish–Gloom, Hoothoot–Noctowl, Lickitung, Venonat–Venomoth |
| Rare | Sunkern, Tangela, Lickitung, Bellsprout–Weepinbell | Oddish–Gloom, Gastly–Haunter ×2, Hoothoot–Noctowl |
| Very rare | Elekid, Farfetch'd | Elekid, Tangela |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Uncommon | Remoraid–Octillery | Remoraid–Octillery |
| Rare | Psyduck–Golduck, Magikarp–Gyarados | Wooper–Quagsire, Magikarp–Gyarados |
| Very rare | Marill–Azumarill | Psyduck–Golduck |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Remoraid–Octillery, Poliwag–Poliwhirl | Remoraid–Octillery, Poliwag–Poliwhirl |
| Uncommon | Magikarp–Gyarados, Goldeen–Seaking, Poliwag–Poliwhirl | Magikarp–Gyarados, Wooper–Quagsire, Poliwag–Poliwhirl |
| Rare | Magikarp–Gyarados, Psyduck–Golduck, Remoraid–Octillery | Magikarp–Gyarados, Wooper–Quagsire, Goldeen–Seaking |
| Very rare | Marill–Azumarill, Poliwag–Poliwhirl | Psyduck–Golduck, Poliwag–Poliwhirl |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell | Bellsprout–Weepinbell |
| Uncommon | Weedle–Beedrill | Weedle–Beedrill |
| Rare | Pineco–Forretress, Exeggcute | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Ledyba–Ledian | Hoothoot–Noctowl |

#### Route 45

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler, Phanpy–Donphan | Teddiursa–Ursaring, Geodude–Graveler |
| Uncommon | Teddiursa–Ursaring, Spearow–Fearow, Mankey–Primeape, Machop–Machoke | Zubat–Golbat, Hoothoot–Noctowl, Gastly–Haunter, Gligar |
| Rare | Gligar ×2, Geodude–Graveler, Phanpy–Donphan | Zubat–Golbat ×2, Gastly–Haunter, Hoothoot–Noctowl |
| Very rare | Dunsparce, Teddiursa–Ursaring | Dunsparce, Gligar |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados | Magikarp–Gyarados |
| Uncommon | Magikarp–Gyarados | Magikarp–Gyarados |
| Rare | Poliwag–Poliwhirl, Marill–Azumarill | Poliwag–Poliwhirl, Wooper–Quagsire |
| Very rare | Goldeen–Seaking | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados ×2 | Magikarp–Gyarados ×2 |
| Uncommon | Poliwag–Poliwhirl, Goldeen–Seaking, Magikarp–Gyarados | Poliwag–Poliwhirl, Wooper–Quagsire, Magikarp–Gyarados |
| Rare | Goldeen–Seaking, Poliwag–Poliwhirl, Magikarp–Gyarados | Goldeen–Seaking, Wooper–Quagsire, Magikarp–Gyarados |
| Very rare | Marill–Azumarill, Goldeen–Seaking | Poliwag–Poliwhirl, Goldeen–Seaking |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler | Geodude–Graveler |
| Uncommon | Aipom | Aipom |
| Rare | Pineco–Forretress, Spearow–Fearow | Hoothoot–Noctowl, Pineco–Forretress |
| Very rare | Exeggcute | Spinarak–Ariados |

#### Route 46

Road, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler, Spearow–Fearow | Geodude–Graveler, Phanpy–Donphan |
| Uncommon | Phanpy–Donphan, Rattata–Raticate, Hoppip–Jumpluff, Pidgey–Pidgeot | Zubat–Golbat, Hoothoot–Noctowl, Gastly–Haunter, Oddish–Gloom |
| Rare | Machop–Machoke, Spearow–Fearow, Cyndaquil–Typhlosion, Rattata–Raticate | Zubat–Golbat, Cyndaquil–Typhlosion, Hoothoot–Noctowl, Gastly–Haunter |
| Very rare | Gligar, Phanpy–Donphan | Spinarak–Ariados, Oddish–Gloom |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler | Geodude–Graveler |
| Uncommon | Spearow–Fearow | Spearow–Fearow |
| Rare | Aipom, Pineco–Forretress | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Exeggcute | Aipom |

#### Route 48

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow, Sunkern | Oddish–Gloom, Vulpix |
| Uncommon | Oddish–Gloom, Vulpix, Hoppip–Jumpluff, Growlithe | Hoothoot–Noctowl, Gastly–Haunter, Houndour–Houndoom, Venonat–Venomoth |
| Rare | Farfetch'd ×2, Pidgey–Pidgeot, Nidoran♀–Nidorina | Oddish–Gloom, Farfetch'd, Yanma, Hoothoot–Noctowl |
| Very rare | Yanma, Sunkern | Houndour–Houndoom, Spinarak–Ariados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow | Spearow–Fearow |
| Uncommon | Weedle–Beedrill | Weedle–Beedrill |
| Rare | Aipom, Pineco–Forretress | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Ledyba–Ledian | Yanma |

#### Ilex Forest

Road, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Caterpie–Metapod, Weedle–Kakuna | Oddish–Gloom, Venonat–Venomoth |
| Uncommon | Ledyba–Ledian, Paras–Parasect, Oddish–Gloom, Sunkern | Spinarak–Ariados, Hoothoot–Noctowl, Paras–Parasect, Wooper–Quagsire |
| Rare | Yanma, Chikorita–Meganium, Pichu, Ledyba–Ledian | Spinarak–Ariados, Hoothoot–Noctowl, Pichu, Chikorita–Meganium |
| Very rare | Chikorita–Meganium, Pichu | Misdreavus, Weedle–Kakuna |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Marill–Azumarill, Wooper–Quagsire | Wooper–Quagsire ×2 |
| Very rare | Magikarp–Gyarados | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl, Psyduck–Golduck | Poliwag–Poliwhirl, Psyduck–Golduck |
| Uncommon | Magikarp–Gyarados ×2, Poliwag–Poliwhirl | Magikarp–Gyarados ×2, Wooper–Quagsire |
| Rare | Goldeen–Seaking, Marill–Azumarill, Poliwag–Poliwhirl | Wooper–Quagsire, Goldeen–Seaking, Poliwag–Poliwhirl |
| Very rare | Psyduck–Golduck, Wooper–Quagsire | Psyduck–Golduck, Marill–Azumarill |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Weedle–Beedrill | Weedle–Beedrill |
| Uncommon | Caterpie–Butterfree | Caterpie–Butterfree |
| Rare | Pineco–Forretress, Exeggcute | Spinarak–Ariados, Pineco–Forretress |
| Very rare | Aipom | Hoothoot–Noctowl |

#### Route 27

Wilds, Border.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ponyta–Rapidash, Rattata–Raticate | Rattata–Raticate, Ekans–Arbok |
| Uncommon | Ekans–Arbok, Doduo–Dodrio, Sandshrew–Sandslash, Spearow–Fearow | Hoothoot–Noctowl, Zubat–Golbat, Gastly–Haunter, Venonat–Venomoth |
| Rare | Miltank, Doduo–Dodrio, Girafarig, Ponyta–Rapidash | Murkrow, Oddish–Gloom, Houndour–Houndoom, Hoothoot–Noctowl |
| Very rare | Chikorita–Meganium, Tauros | Chikorita–Meganium, Murkrow |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Shellder | Shellder |
| Rare | Totodile–Feraligatr, Staryu | Chinchou–Lanturn, Totodile–Feraligatr |
| Very rare | Mantine | Staryu |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Shellder, Tentacool–Tentacruel | Shellder, Tentacool–Tentacruel |
| Uncommon | Magikarp–Gyarados, Krabby–Kingler, Shellder | Magikarp–Gyarados, Chinchou–Lanturn, Staryu |
| Rare | Corsola, Staryu, Tentacool–Tentacruel | Krabby–Kingler, Chinchou–Lanturn, Staryu |
| Very rare | Krabby–Kingler, Chinchou–Lanturn | Tentacool–Tentacruel, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ekans–Arbok | Ekans–Arbok |
| Uncommon | Pineco–Forretress | Pineco–Forretress |
| Rare | Heracross, Exeggcute | Murkrow, Heracross |
| Very rare | Aipom | Hoothoot–Noctowl |

#### Route 47

Wilds, Johto west.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Spearow–Fearow, Hoppip–Jumpluff | Oddish–Gloom, Venonat–Venomoth |
| Uncommon | Oddish–Gloom, Rattata–Raticate, Snubbull–Granbull, Pidgey–Pidgeot | Hoothoot–Noctowl, Gastly–Haunter, Rattata–Raticate, Spinarak–Ariados |
| Rare | Miltank, Farfetch'd, Girafarig, Ditto | Miltank, Ditto, Farfetch'd, Hoothoot–Noctowl |
| Very rare | Tauros, Skarmory | Happiny, Skarmory |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Corsola | Seel–Dewgong |
| Uncommon | Seel–Dewgong | Corsola |
| Rare | Chinchou–Lanturn, Shellder | Chinchou–Lanturn, Staryu |
| Very rare | Mantine | Mantine |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Corsola, Seel–Dewgong | Corsola, Seel–Dewgong |
| Uncommon | Chinchou–Lanturn, Shellder, Magikarp–Gyarados | Chinchou–Lanturn ×2, Staryu |
| Rare | Corsola, Staryu, Shellder | Magikarp–Gyarados, Staryu, Krabby–Kingler |
| Very rare | Chinchou–Lanturn, Remoraid–Octillery | Corsola, Chinchou–Lanturn |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler | Krabby–Kingler |
| Uncommon | Spearow–Fearow | Spearow–Fearow |
| Rare | Shuckle, Heracross | Hoothoot–Noctowl, Shuckle |
| Very rare | Geodude–Graveler | Heracross |

#### Dark Cave

Wilds, Johto east.

**`MAP_DARK_CAVE_SOUTH_SIDE_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler, Zubat–Golbat | Zubat–Golbat, Geodude–Graveler |
| Uncommon | Teddiursa–Ursaring, Machop–Machoke, Onix, Zubat–Golbat | Teddiursa–Ursaring, Machop–Machoke, Onix, Gastly–Haunter |
| Rare | Dunsparce ×2, Geodude–Graveler, Wobbuffet | Dunsparce ×2, Wobbuffet ×2 |
| Very rare | Wobbuffet, Teddiursa–Ursaring | Gastly–Haunter, Teddiursa–Ursaring |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking | Chinchou–Lanturn |
| Uncommon | Chinchou–Lanturn | Goldeen–Seaking |
| Rare | Magikarp–Gyarados, Poliwag–Poliwhirl | Magikarp–Gyarados, Poliwag–Poliwhirl |
| Very rare | Marill–Azumarill | Wooper–Quagsire |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking, Chinchou–Lanturn | Chinchou–Lanturn, Goldeen–Seaking |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Chinchou–Lanturn, Magikarp–Gyarados, Goldeen–Seaking |
| Rare | Poliwag–Poliwhirl, Chinchou–Lanturn, Goldeen–Seaking | Chinchou–Lanturn, Magikarp–Gyarados, Poliwag–Poliwhirl |
| Very rare | Magikarp–Gyarados, Marill–Azumarill | Wooper–Quagsire, Goldeen–Seaking |

**`MAP_DARK_CAVE_NORTH_SIDE_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Teddiursa–Ursaring, Onix | Teddiursa–Ursaring, Onix |
| Uncommon | Geodude–Graveler ×2, Zubat–Golbat, Machop–Machoke | Zubat–Golbat, Machop–Machoke, Geodude–Graveler, Gastly–Haunter |
| Rare | Wobbuffet ×2, Zubat–Golbat, Geodude–Graveler | Wobbuffet, Gastly–Haunter, Dunsparce, Zubat–Golbat |
| Very rare | Gligar, Dunsparce | Gligar, Dunsparce |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados | Chinchou–Lanturn |
| Uncommon | Chinchou–Lanturn | Magikarp–Gyarados |
| Rare | Goldeen–Seaking, Marill–Azumarill | Wooper–Quagsire, Goldeen–Seaking |
| Very rare | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados, Chinchou–Lanturn | Chinchou–Lanturn, Magikarp–Gyarados |
| Uncommon | Goldeen–Seaking ×2, Magikarp–Gyarados | Chinchou–Lanturn, Goldeen–Seaking, Magikarp–Gyarados |
| Rare | Poliwag–Poliwhirl, Chinchou–Lanturn, Magikarp–Gyarados | Wooper–Quagsire, Chinchou–Lanturn, Goldeen–Seaking |
| Very rare | Marill–Azumarill, Goldeen–Seaking | Poliwag–Poliwhirl, Magikarp–Gyarados |

#### Cliff Edge Cave

Wilds, Johto west.

**`MAP_CLIFF_EDGE_GATE_HNS`**

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Chinchou–Lanturn |
| Uncommon | Chinchou–Lanturn | Wooper–Quagsire |
| Rare | Shellder, Tentacool–Tentacruel | Staryu, Shellder |
| Very rare | Mantine | Mantine |

**`MAP_CLIFF_EDGE_CAVE_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler, Geodude–Graveler | Geodude–Graveler, Krabby–Kingler |
| Uncommon | Wooper–Quagsire, Corsola, Zubat–Golbat, Onix | Wooper–Quagsire, Corsola, Zubat–Golbat, Gastly–Haunter |
| Rare | Shuckle, Wooper–Quagsire, Skarmory, Seel–Dewgong | Shuckle, Wooper–Quagsire, Onix, Seel–Dewgong |
| Very rare | Skarmory, Shuckle | Skarmory, Shuckle |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Krabby–Kingler | Geodude–Graveler |
| Uncommon | Geodude–Graveler | Krabby–Kingler |
| Rare | Shuckle, Krabby–Kingler | Shuckle, Onix |
| Very rare | Shuckle | Shuckle |

#### Tohjo Falls

Wilds, Border.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro, Zubat–Golbat | Zubat–Golbat, Slowpoke–Slowbro |
| Uncommon | Rattata–Raticate, Geodude–Graveler, Onix, Marill–Azumarill | Rattata–Raticate, Geodude–Graveler, Gastly–Haunter, Marill–Azumarill |
| Rare | Psyduck–Golduck, Slowpoke–Slowbro, Dunsparce, Rattata–Raticate | Psyduck–Golduck, Onix, Dunsparce, Zubat–Golbat |
| Very rare | Dunsparce, Onix | Gastly–Haunter, Slowpoke–Slowbro |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Psyduck–Golduck, Marill–Azumarill | Wooper–Quagsire, Psyduck–Golduck |
| Very rare | Poliwag–Poliwhirl | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking, Slowpoke–Slowbro | Goldeen–Seaking, Slowpoke–Slowbro |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Magikarp–Gyarados ×2, Wooper–Quagsire |
| Rare | Poliwag–Poliwhirl, Slowpoke–Slowbro, Psyduck–Golduck | Wooper–Quagsire, Slowpoke–Slowbro, Poliwag–Poliwhirl |
| Very rare | Magikarp–Gyarados, Marill–Azumarill | Psyduck–Golduck, Magikarp–Gyarados |

#### Ruins of Alph

Wilds, Johto east.

**`MAP_RUINS_OF_ALPH_OUTSIDE_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Natu–Xatu, Unown | Wooper–Quagsire, Natu–Xatu |
| Uncommon | Mareep–Ampharos, Hoppip–Jumpluff, Pidgey–Pidgeot, Sunkern | Gastly–Haunter, Hoothoot–Noctowl, Unown, Oddish–Gloom |
| Rare | Smeargle ×2, Natu–Xatu, Unown | Misdreavus, Smeargle, Gastly–Haunter, Wooper–Quagsire |
| Very rare | Abra, Girafarig | Abra, Girafarig |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Wooper–Quagsire |
| Uncommon | Wooper–Quagsire | Wooper–Quagsire |
| Rare | Poliwag–Poliwhirl, Marill–Azumarill | Psyduck–Golduck, Poliwag–Poliwhirl |
| Very rare | Magikarp–Gyarados | Marill–Azumarill |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire ×2 | Wooper–Quagsire ×2 |
| Uncommon | Magikarp–Gyarados ×2, Poliwag–Poliwhirl | Magikarp–Gyarados ×2, Psyduck–Golduck |
| Rare | Marill–Azumarill, Poliwag–Poliwhirl, Goldeen–Seaking | Poliwag–Poliwhirl, Psyduck–Golduck, Marill–Azumarill |
| Very rare | Magikarp–Gyarados, Wooper–Quagsire | Goldeen–Seaking, Wooper–Quagsire |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Natu–Xatu | Natu–Xatu |
| Uncommon | Natu–Xatu | Natu–Xatu |
| Rare | Smeargle, Aipom | Spinarak–Ariados, Smeargle |
| Very rare | Heracross | Heracross |

**`MAP_RUINS_OF_ALPH_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Unown ×2 | Unown ×2 |
| Uncommon | Unown ×4 | Unown ×4 |
| Rare | Unown ×2, Natu–Xatu ×2 | Misdreavus, Gastly–Haunter, Natu–Xatu, Unown |
| Very rare | Smeargle, Unown | Smeargle, Unown |

#### National Park

Wilds, Johto west.

**`MAP_NATIONAL_PARK_NORMAL_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Sunkern, Nidoran♀–Nidorina | Spinarak–Ariados, Nidoran♂–Nidorino |
| Uncommon | Nidoran♂–Nidorino, Caterpie–Butterfree, Weedle–Beedrill, Snubbull–Granbull | Hoothoot–Noctowl, Venonat–Venomoth, Psyduck–Golduck, Murkrow |
| Rare | Stantler, Scyther, Girafarig, Togepi | Stantler, Scyther, Gastly–Haunter, Togepi |
| Very rare | Scyther, Pinsir | Scyther, Misdreavus |

**`MAP_NATIONAL_PARK_BUG_CONTEST_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Caterpie–Butterfree, Weedle–Beedrill | Venonat–Venomoth, Spinarak–Ariados |
| Uncommon | Ledyba–Ledian, Paras–Parasect, Venonat–Venomoth, Caterpie–Metapod | Paras–Parasect, Pineco–Forretress, Ledyba–Ledian, Weedle–Beedrill |
| Rare | Scyther, Pinsir, Yanma, Heracross | Scyther, Pinsir, Yanma, Heracross |
| Very rare | Scyther, Pinsir | Scyther, Spinarak–Ariados |

#### Lake of Rage

Wilds, Johto east.

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados | Magikarp–Gyarados |
| Uncommon | Magikarp–Gyarados | Magikarp–Gyarados |
| Rare | Totodile–Feraligatr, Marill–Azumarill | Wooper–Quagsire, Totodile–Feraligatr |
| Very rare | Wooper–Quagsire | Psyduck–Golduck |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados ×2 | Magikarp–Gyarados ×2 |
| Uncommon | Poliwag–Poliwhirl, Goldeen–Seaking, Magikarp–Gyarados | Wooper–Quagsire, Poliwag–Poliwhirl, Magikarp–Gyarados |
| Rare | Wooper–Quagsire, Marill–Azumarill, Magikarp–Gyarados | Wooper–Quagsire, Goldeen–Seaking, Psyduck–Golduck |
| Very rare | Totodile–Feraligatr, Goldeen–Seaking | Totodile–Feraligatr, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Exeggcute | Venonat–Venomoth |
| Uncommon | Pineco–Forretress | Pineco–Forretress |
| Rare | Heracross, Aipom | Hoothoot–Noctowl, Heracross |
| Very rare | Venonat–Venomoth | Yanma |

#### Route 26

Outlands, Border.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Doduo–Dodrio, Ekans–Arbok | Rattata–Raticate, Ekans–Arbok |
| Uncommon | Ponyta–Rapidash, Rattata–Raticate, Sandshrew–Sandslash, Spearow–Fearow | Houndour–Houndoom, Hoothoot–Noctowl, Zubat–Golbat, Sandshrew–Sandslash |
| Rare | Tauros, Phanpy–Donphan, Miltank, Mankey–Primeape | Murkrow–Honchkrow, Tauros, Venonat–Venomoth, Houndour–Houndoom |
| Very rare | Larvitar–Tyranitar, Teddiursa–Ursaring | Larvitar–Tyranitar, Gastly–Gengar |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Tentacool–Tentacruel |
| Uncommon | Shellder–Cloyster | Shellder–Cloyster |
| Rare | Staryu–Starmie, Lapras | Chinchou–Lanturn, Staryu–Starmie |
| Very rare | Mantine | Lapras |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Shellder–Cloyster, Tentacool–Tentacruel | Shellder–Cloyster, Tentacool–Tentacruel |
| Uncommon | Magikarp–Gyarados, Krabby–Kingler, Staryu–Starmie | Chinchou–Lanturn, Staryu–Starmie, Magikarp–Gyarados |
| Rare | Corsola, Magikarp–Gyarados, Remoraid–Octillery | Chinchou–Lanturn, Staryu–Starmie, Krabby–Kingler |
| Very rare | Horsea–Kingdra, Chinchou–Lanturn | Horsea–Kingdra, Remoraid–Octillery |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Aipom–Ambipom | Aipom–Ambipom |
| Uncommon | Pineco–Forretress | Pineco–Forretress |
| Rare | Heracross, Geodude–Golem | Hoothoot–Noctowl, Heracross |
| Very rare | Scyther–Scizor | Murkrow–Honchkrow |

#### Route 28

Outlands, Border.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ponyta–Rapidash, Tangela–Tangrowth | Teddiursa–Ursaring, Poliwag–Poliwrath |
| Uncommon | Doduo–Dodrio, Teddiursa–Ursaring, Phanpy–Donphan, Ekans–Arbok | Zubat–Crobat, Houndour–Houndoom, Tangela–Tangrowth, Murkrow–Honchkrow |
| Rare | Skarmory, Larvitar–Tyranitar, Tauros, Girafarig–Farigiraf | Skarmory, Larvitar–Tyranitar, Houndour–Houndoom, Sneasel–Weavile |
| Very rare | Larvitar–Tyranitar, Snorlax | Larvitar–Tyranitar, Poliwag–Politoed |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwrath | Poliwag–Poliwrath |
| Uncommon | Poliwag–Poliwrath | Poliwag–Poliwrath |
| Rare | Goldeen–Seaking, Poliwag–Politoed | Wooper–Quagsire, Goldeen–Seaking |
| Very rare | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwrath ×2 | Poliwag–Poliwrath ×2 |
| Uncommon | Magikarp–Gyarados, Goldeen–Seaking, Poliwag–Poliwrath | Magikarp–Gyarados, Wooper–Quagsire, Poliwag–Poliwrath |
| Rare | Magikarp–Gyarados, Goldeen–Seaking, Poliwag–Politoed | Wooper–Quagsire, Psyduck–Golduck, Poliwag–Politoed |
| Very rare | Psyduck–Golduck, Magikarp–Gyarados | Goldeen–Seaking, Magikarp–Gyarados |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pineco–Forretress | Teddiursa–Ursaring |
| Uncommon | Teddiursa–Ursaring | Pineco–Forretress |
| Rare | Heracross, Scyther–Scizor | Murkrow–Honchkrow, Heracross |
| Very rare | Aipom–Ambipom | Scyther–Scizor |

#### Union Cave

Dungeon, Johto east.

**`MAP_UNION_CAVE_1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler, Zubat–Golbat | Zubat–Golbat, Geodude–Graveler |
| Uncommon | Onix, Sandshrew–Sandslash, Rattata–Raticate, Wooper–Quagsire | Onix, Wooper–Quagsire, Rattata–Raticate, Sandshrew–Sandslash |
| Rare | Cubone–Marowak, Geodude–Graveler, Dunsparce, Onix | Gastly–Haunter, Cubone–Marowak, Dunsparce, Wooper–Quagsire |
| Very rare | Dunsparce, Cubone–Marowak | Onix, Dunsparce |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Wooper–Quagsire |
| Uncommon | Wooper–Quagsire | Wooper–Quagsire |
| Rare | Goldeen–Seaking, Poliwag–Poliwhirl | Chinchou–Lanturn, Goldeen–Seaking |
| Very rare | Psyduck–Golduck | Poliwag–Poliwhirl |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking, Wooper–Quagsire | Goldeen–Seaking, Wooper–Quagsire |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Magikarp–Gyarados ×2, Chinchou–Lanturn |
| Rare | Poliwag–Poliwhirl, Goldeen–Seaking, Wooper–Quagsire | Chinchou–Lanturn, Goldeen–Seaking, Wooper–Quagsire |
| Very rare | Magikarp–Gyarados, Chinchou–Lanturn | Poliwag–Poliwhirl, Magikarp–Gyarados |

**`MAP_UNION_CAVE_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Onix, Marill–Azumarill | Onix, Marill–Azumarill |
| Uncommon | Geodude–Graveler, Zubat–Golbat, Sandshrew–Sandslash, Rattata–Raticate | Zubat–Golbat, Geodude–Graveler, Wooper–Quagsire, Rattata–Raticate |
| Rare | Wooper–Quagsire, Cubone–Marowak, Dunsparce, Machop–Machoke | Gastly–Haunter ×2, Cubone–Marowak, Dunsparce |
| Very rare | Onix–Steelix, Dunsparce | Onix–Steelix, Sandshrew–Sandslash |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Wooper–Quagsire |
| Uncommon | Marill–Azumarill | Marill–Azumarill |
| Rare | Goldeen–Seaking, Poliwag–Poliwhirl | Chinchou–Lanturn, Goldeen–Seaking |
| Very rare | Psyduck–Golduck | Psyduck–Golduck |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire, Marill–Azumarill | Wooper–Quagsire, Marill–Azumarill |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Magikarp–Gyarados, Chinchou–Lanturn, Goldeen–Seaking |
| Rare | Goldeen–Seaking, Poliwag–Poliwhirl, Magikarp–Gyarados | Chinchou–Lanturn, Magikarp–Gyarados, Goldeen–Seaking |
| Very rare | Chinchou–Lanturn, Wooper–Quagsire | Poliwag–Poliwhirl, Wooper–Quagsire |

**`MAP_UNION_CAVE_B2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire, Onix | Wooper–Quagsire, Onix |
| Uncommon | Zubat–Golbat, Geodude–Graveler, Rattata–Raticate, Cubone–Marowak | Zubat–Golbat, Geodude–Graveler, Gastly–Haunter, Cubone–Marowak |
| Rare | Marill–Azumarill, Dunsparce, Onix–Steelix, Machop–Machoke | Marill–Azumarill, Dunsparce, Onix–Steelix, Gastly–Haunter |
| Very rare | Wobbuffet, Dunsparce | Wobbuffet, Zubat–Crobat |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Wooper–Quagsire |
| Uncommon | Shellder | Shellder |
| Rare | Lapras, Tentacool–Tentacruel | Chinchou–Lanturn, Lapras |
| Very rare | Chinchou–Lanturn | Staryu |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Shellder, Wooper–Quagsire | Shellder, Wooper–Quagsire |
| Uncommon | Chinchou–Lanturn, Tentacool–Tentacruel, Magikarp–Gyarados | Chinchou–Lanturn ×2, Staryu |
| Rare | Shellder, Tentacool–Tentacruel, Chinchou–Lanturn | Magikarp–Gyarados, Staryu, Tentacool–Tentacruel |
| Very rare | Krabby–Kingler, Staryu | Staryu, Krabby–Kingler |

#### Ice Path

Dungeon, Johto east.

**`MAP_ICE_PATH_1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Swinub–Piloswine, Delibird | Sneasel, Delibird |
| Uncommon | Zubat–Golbat, Geodude–Graveler, Onix, Swinub–Piloswine | Zubat–Golbat, Swinub–Piloswine, Geodude–Graveler, Sneasel |
| Rare | Sneasel, Zubat–Golbat, Smoochum, Delibird | Onix, Zubat–Golbat, Smoochum, Delibird |
| Very rare | Jynx, Sneasel | Jynx, Gastly–Haunter |

**`MAP_ICE_PATH_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Delibird, Swinub–Piloswine | Sneasel, Delibird |
| Uncommon | Zubat–Golbat, Sneasel, Seel–Dewgong, Onix | Swinub–Piloswine, Zubat–Golbat, Sneasel, Seel–Dewgong |
| Rare | Jynx, Zubat–Golbat, Smoochum, Delibird | Jynx, Smoochum, Zubat–Golbat, Onix |
| Very rare | Swinub–Mamoswine, Jynx | Sneasel–Weavile, Jynx |

**`MAP_ICE_PATH_B2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Swinub–Piloswine, Delibird | Sneasel, Swinub–Piloswine |
| Uncommon | Seel–Dewgong, Sneasel, Zubat–Golbat, Geodude–Graveler | Delibird, Sneasel, Seel–Dewgong, Zubat–Golbat |
| Rare | Jynx ×2, Smoochum, Swinub–Mamoswine | Jynx, Sneasel–Weavile, Smoochum, Swinub–Mamoswine |
| Very rare | Sneasel–Weavile, Onix–Steelix | Zubat–Crobat, Jynx |

**`MAP_ICE_PATH_B3F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Swinub–Piloswine, Delibird | Sneasel, Delibird |
| Uncommon | Jynx, Sneasel, Seel–Dewgong, Swinub–Mamoswine | Jynx, Sneasel–Weavile, Swinub–Piloswine, Seel–Dewgong |
| Rare | Zubat–Golbat, Smoochum, Jynx, Sneasel–Weavile | Sneasel, Smoochum, Swinub–Mamoswine, Jynx |
| Very rare | Onix–Steelix, Zubat–Crobat | Zubat–Crobat, Sneasel–Weavile |

**`MAP_ICE_PATH_B4F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Delibird, Swinub–Piloswine | Sneasel, Swinub–Piloswine |
| Uncommon | Jynx, Swinub–Mamoswine, Seel–Dewgong, Sneasel | Sneasel–Weavile, Jynx, Delibird, Swinub–Mamoswine |
| Rare | Sneasel–Weavile, Jynx, Smoochum, Onix–Steelix | Sneasel, Jynx, Smoochum, Sneasel–Weavile |
| Very rare | Zubat–Crobat, Jynx | Zubat–Crobat, Seel–Dewgong |

#### Sprout Tower

Dungeon, Johto east.

**`MAP_SPROUT_TOWER_2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Rattata–Raticate, Bellsprout–Weepinbell | Gastly–Haunter, Rattata–Raticate |
| Uncommon | Gastly–Haunter, Rattata–Raticate, Bellsprout–Weepinbell, Zubat–Golbat | Bellsprout–Weepinbell, Gastly–Haunter, Rattata–Raticate, Zubat–Golbat |
| Rare | Gastly–Haunter, Natu–Xatu, Abra, Misdreavus | Misdreavus ×2, Gastly–Haunter, Hoothoot–Noctowl |
| Very rare | Abra, Hoothoot–Noctowl | Abra, Bellsprout–Weepinbell |

**`MAP_SPROUT_TOWER_3F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Bellsprout–Weepinbell ×2 | Gastly–Haunter, Bellsprout–Weepinbell |
| Uncommon | Gastly–Haunter, Zubat–Golbat, Natu–Xatu, Rattata–Raticate | Misdreavus, Rattata–Raticate, Zubat–Golbat, Gastly–Haunter |
| Rare | Misdreavus, Gastly–Haunter, Abra, Natu–Xatu | Misdreavus, Hoothoot–Noctowl, Abra, Gastly–Gengar |
| Very rare | Bellsprout–Victreebel, Misdreavus | Bellsprout–Victreebel, Misdreavus |

#### Slowpoke Well

Dungeon, Johto west.

**`MAP_SLOWPOKE_WELL_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro ×2 | Slowpoke–Slowbro ×2 |
| Uncommon | Wooper–Quagsire ×2, Marill–Azumarill, Zubat–Golbat | Zubat–Golbat, Wooper–Quagsire, Gastly–Haunter, Marill–Azumarill |
| Rare | Koffing–Weezing, Marill–Azumarill, Zubat–Golbat, Rattata–Raticate | Misdreavus, Gastly–Haunter, Wooper–Quagsire, Koffing–Weezing |
| Very rare | Wobbuffet, Wooper–Quagsire | Wobbuffet, Misdreavus |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Marill–Azumarill, Psyduck–Golduck | Wooper–Quagsire, Marill–Azumarill |
| Very rare | Wooper–Quagsire | Chinchou–Lanturn |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro ×2 | Slowpoke–Slowbro ×2 |
| Uncommon | Marill–Azumarill, Goldeen–Seaking, Wooper–Quagsire | Wooper–Quagsire ×2, Marill–Azumarill |
| Rare | Magikarp–Gyarados, Marill–Azumarill, Poliwag–Poliwhirl | Goldeen–Seaking, Chinchou–Lanturn, Marill–Azumarill |
| Very rare | Slowpoke–Slowbro, Wooper–Quagsire | Slowpoke–Slowbro, Chinchou–Lanturn |

**`MAP_SLOWPOKE_WELL_B2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro ×2 | Slowpoke–Slowbro ×2 |
| Uncommon | Wooper–Quagsire, Marill–Azumarill, Zubat–Golbat, Slowpoke–Slowking | Zubat–Golbat, Wooper–Quagsire, Misdreavus, Gastly–Haunter |
| Rare | Wobbuffet, Wooper–Quagsire, Zubat–Crobat, Marill–Azumarill | Misdreavus, Slowpoke–Slowking, Wobbuffet, Zubat–Crobat |
| Very rare | Slowpoke–Slowking, Misdreavus | Gastly–Haunter, Slowpoke–Slowking |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Slowpoke–Slowking, Marill–Azumarill | Wooper–Quagsire, Slowpoke–Slowking |
| Very rare | Wooper–Quagsire | Chinchou–Lanturn |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro ×2 | Slowpoke–Slowbro ×2 |
| Uncommon | Marill–Azumarill, Goldeen–Seaking, Slowpoke–Slowking | Wooper–Quagsire, Marill–Azumarill, Slowpoke–Slowking |
| Rare | Wooper–Quagsire, Marill–Azumarill, Magikarp–Gyarados | Wooper–Quagsire, Chinchou–Lanturn, Marill–Azumarill |
| Very rare | Slowpoke–Slowbro, Wooper–Quagsire | Slowpoke–Slowbro, Chinchou–Lanturn |

#### Burned Tower

Dungeon, Johto west.

**`MAP_BURNED_TOWER_1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slugma–Magcargo, Rattata–Raticate | Slugma–Magcargo, Rattata–Raticate |
| Uncommon | Koffing–Weezing, Zubat–Golbat, Rattata–Raticate, Slugma–Magcargo | Gastly–Haunter, Zubat–Golbat, Koffing–Weezing, Houndour–Houndoom |
| Rare | Houndour–Houndoom ×2, Magby, Koffing–Weezing | Houndour–Houndoom, Magby, Gastly–Haunter, Misdreavus |
| Very rare | Magby, Vulpix | Magby, Vulpix |

**`MAP_BURNED_TOWER_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Gastly–Haunter, Slugma–Magcargo | Gastly–Haunter, Slugma–Magcargo |
| Uncommon | Koffing–Weezing, Misdreavus, Zubat–Golbat, Houndour–Houndoom | Misdreavus, Murkrow, Houndour–Houndoom, Koffing–Weezing |
| Rare | Magmar, Cyndaquil–Typhlosion, Misdreavus, Magby | Misdreavus, Magmar, Gastly–Gengar, Magby |
| Very rare | Magmar–Magmortar, Cyndaquil–Typhlosion | Misdreavus–Mismagius, Cyndaquil–Typhlosion |

#### Rocket Hideout

Dungeon, Johto east.

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Koffing–Weezing, Voltorb–Electrode | Koffing–Weezing, Voltorb–Electrode |
| Uncommon | Rattata–Raticate, Grimer–Muk, Magnemite–Magneton, Meowth–Persian | Rattata–Raticate, Grimer–Muk, Meowth–Persian, Gastly–Haunter |
| Rare | Porygon ×2, Geodude–Graveler, Meowth–Persian | Porygon ×2, Magnemite–Magneton, Zubat–Golbat |
| Very rare | Porygon–Porygon2, Koffing–Weezing | Porygon–Porygon2, Gastly–Haunter |

#### Mt. Mortar

Dungeon, Johto east.

**`MAP_MT_MORTAR_1F_SOUTH_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Machop–Machoke, Zubat–Golbat | Zubat–Golbat, Machop–Machoke |
| Uncommon | Geodude–Graveler, Marill–Azumarill, Rattata–Raticate, Mankey–Primeape | Marill–Azumarill, Geodude–Graveler, Rattata–Raticate, Gastly–Haunter |
| Rare | Cubone–Marowak, Onix, Tyrogue, Machop–Machoke | Cubone–Marowak, Onix, Tyrogue, Marill–Azumarill |
| Very rare | Hitmontop, Dunsparce | Hitmontop, Cleffa |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill | Marill–Azumarill |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Psyduck–Golduck, Poliwag–Poliwhirl | Wooper–Quagsire, Psyduck–Golduck |
| Very rare | Magikarp–Gyarados | Magikarp–Gyarados |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking, Marill–Azumarill | Goldeen–Seaking, Marill–Azumarill |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Magikarp–Gyarados ×2, Wooper–Quagsire |
| Rare | Poliwag–Poliwhirl, Goldeen–Seaking, Psyduck–Golduck | Wooper–Quagsire, Goldeen–Seaking, Poliwag–Poliwhirl |
| Very rare | Magikarp–Gyarados, Marill–Azumarill | Magikarp–Gyarados, Psyduck–Golduck |

**`MAP_MT_MORTAR_1F_NORTH_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Graveler, Marill–Azumarill | Marill–Azumarill, Geodude–Graveler |
| Uncommon | Machop–Machoke, Zubat–Golbat, Rattata–Raticate, Cubone–Marowak | Zubat–Golbat, Machop–Machoke, Gastly–Haunter, Cubone–Marowak |
| Rare | Onix, Tyrogue, Dunsparce, Hitmontop | Onix, Tyrogue, Cleffa, Hitmontop |
| Very rare | Machop–Machamp, Geodude–Golem | Zubat–Crobat, Dunsparce |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill | Wooper–Quagsire |
| Uncommon | Wooper–Quagsire | Marill–Azumarill |
| Rare | Psyduck–Golduck, Goldeen–Seaking | Goldeen–Seaking, Psyduck–Golduck |
| Very rare | Magikarp–Gyarados | Chinchou–Lanturn |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill, Wooper–Quagsire | Wooper–Quagsire, Marill–Azumarill |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Magikarp–Gyarados ×2, Chinchou–Lanturn |
| Rare | Poliwag–Poliwhirl, Psyduck–Golduck, Goldeen–Seaking | Wooper–Quagsire, Goldeen–Seaking, Chinchou–Lanturn |
| Very rare | Magikarp–Gyarados, Marill–Azumarill | Poliwag–Poliwhirl, Magikarp–Gyarados |

**`MAP_MT_MORTAR_2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Machop–Machoke, Mankey–Primeape | Machop–Machoke, Marill–Azumarill |
| Uncommon | Geodude–Graveler, Zubat–Golbat, Marill–Azumarill, Rattata–Raticate | Zubat–Golbat, Mankey–Primeape, Geodude–Graveler, Gastly–Haunter |
| Rare | Machop–Machamp, Hitmontop, Tyrogue, Onix | Hitmontop, Machop–Machamp, Tyrogue, Gastly–Haunter |
| Very rare | Geodude–Golem, Dunsparce | Zubat–Crobat, Cleffa |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill | Marill–Azumarill |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Psyduck–Golduck, Poliwag–Poliwhirl | Wooper–Quagsire, Psyduck–Golduck |
| Very rare | Magikarp–Gyarados | Magikarp–Gyarados |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking, Marill–Azumarill | Goldeen–Seaking, Marill–Azumarill |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Magikarp–Gyarados ×2, Wooper–Quagsire |
| Rare | Poliwag–Poliwhirl, Goldeen–Seaking, Psyduck–Golduck | Wooper–Quagsire, Goldeen–Seaking, Poliwag–Poliwhirl |
| Very rare | Magikarp–Gyarados, Marill–Azumarill | Magikarp–Gyarados, Psyduck–Golduck |

**`MAP_MT_MORTAR_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Onix, Geodude–Graveler | Geodude–Graveler, Onix |
| Uncommon | Machop–Machoke, Cubone–Marowak, Zubat–Golbat, Marill–Azumarill | Machop–Machoke, Zubat–Golbat, Marill–Azumarill, Gastly–Haunter |
| Rare | Hitmontop ×2, Tyrogue, Machop–Machamp | Hitmontop ×2, Tyrogue, Cleffa |
| Very rare | Geodude–Golem, Zubat–Crobat | Zubat–Crobat, Gastly–Gengar |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill | Marill–Azumarill |
| Uncommon | Goldeen–Seaking | Goldeen–Seaking |
| Rare | Psyduck–Golduck, Magikarp–Gyarados | Wooper–Quagsire, Psyduck–Golduck |
| Very rare | Poliwag–Poliwhirl | Magikarp–Gyarados |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Marill–Azumarill, Goldeen–Seaking | Marill–Azumarill, Goldeen–Seaking |
| Uncommon | Magikarp–Gyarados ×2, Goldeen–Seaking | Magikarp–Gyarados ×2, Wooper–Quagsire |
| Rare | Poliwag–Poliwhirl, Psyduck–Golduck, Goldeen–Seaking | Wooper–Quagsire, Goldeen–Seaking, Psyduck–Golduck |
| Very rare | Magikarp–Gyarados, Marill–Azumarill | Poliwag–Poliwhirl, Magikarp–Gyarados |

#### Tin Tower

Dungeon, Johto west.

**`MAP_TIN_TOWER_3F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Aipom, Sentret–Furret | Aipom, Sentret–Furret |
| Uncommon | Natu–Xatu, Rattata–Raticate, Hoothoot–Noctowl, Gastly–Haunter | Gastly–Haunter, Hoothoot–Noctowl, Misdreavus, Natu–Xatu |
| Rare | Misdreavus ×2, Sentret–Furret, Houndour–Houndoom | Murkrow, Gastly–Haunter, Misdreavus, Houndour–Houndoom |
| Very rare | Eevee, Stantler | Eevee, Stantler |

**`MAP_TIN_TOWER_4F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Natu–Xatu, Aipom | Gastly–Haunter, Natu–Xatu |
| Uncommon | Sentret–Furret, Hoothoot–Noctowl, Gastly–Haunter, Misdreavus | Hoothoot–Noctowl, Aipom, Misdreavus, Murkrow |
| Rare | Aipom–Ambipom, Houndour–Houndoom, Natu–Xatu, Misdreavus | Misdreavus–Mismagius, Houndour–Houndoom, Misdreavus, Gastly–Gengar |
| Very rare | Eevee, Misdreavus–Mismagius | Eevee, Murkrow–Honchkrow |

**`MAP_TIN_TOWER_5F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Natu–Xatu, Sentret–Furret | Natu–Xatu, Misdreavus |
| Uncommon | Hoothoot–Noctowl, Aipom, Misdreavus, Gastly–Haunter | Hoothoot–Noctowl, Murkrow, Gastly–Haunter, Aipom |
| Rare | Aipom–Ambipom, Houndour–Houndoom, Skarmory, Misdreavus–Mismagius | Misdreavus–Mismagius, Murkrow–Honchkrow, Skarmory, Houndour–Houndoom |
| Very rare | Eevee–Espeon, Stantler | Eevee–Umbreon, Gastly–Gengar |

**`MAP_TIN_TOWER_6F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Sentret–Furret, Natu–Xatu | Gastly–Haunter, Natu–Xatu |
| Uncommon | Hoothoot–Noctowl, Misdreavus, Aipom–Ambipom, Gastly–Haunter | Misdreavus, Hoothoot–Noctowl, Murkrow, Aipom–Ambipom |
| Rare | Skarmory, Houndour–Houndoom, Misdreavus–Mismagius, Stantler | Gastly–Gengar, Murkrow–Honchkrow, Misdreavus–Mismagius, Houndour–Houndoom |
| Very rare | Eevee–Espeon, Skarmory | Eevee–Umbreon, Skarmory |

**`MAP_TIN_TOWER_7F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Natu–Xatu, Hoothoot–Noctowl | Hoothoot–Noctowl, Natu–Xatu |
| Uncommon | Sentret–Furret, Aipom–Ambipom, Misdreavus, Gastly–Haunter | Gastly–Haunter, Murkrow, Misdreavus, Aipom–Ambipom |
| Rare | Skarmory, Misdreavus–Mismagius, Eevee–Espeon, Houndour–Houndoom | Gastly–Gengar, Murkrow–Honchkrow, Misdreavus–Mismagius, Eevee–Umbreon |
| Very rare | Stantler, Skarmory | Skarmory, Houndour–Houndoom |

**`MAP_TIN_TOWER_8F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Gastly–Haunter, Natu–Xatu | Gastly–Haunter, Misdreavus |
| Uncommon | Hoothoot–Noctowl, Aipom–Ambipom, Skarmory, Misdreavus–Mismagius | Natu–Xatu, Murkrow, Misdreavus–Mismagius, Skarmory |
| Rare | Eevee–Espeon, Houndour–Houndoom, Skarmory, Gastly–Gengar | Murkrow–Honchkrow, Eevee–Umbreon, Gastly–Gengar, Houndour–Houndoom |
| Very rare | Stantler, Misdreavus | Zubat–Crobat, Eevee–Espeon |

**`MAP_TIN_TOWER_9F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Hoothoot–Noctowl, Sentret–Furret | Hoothoot–Noctowl, Natu–Xatu |
| Uncommon | Natu–Xatu, Skarmory, Aipom–Ambipom, Misdreavus–Mismagius | Murkrow–Honchkrow, Skarmory, Misdreavus–Mismagius, Gastly–Haunter |
| Rare | Eevee–Espeon, Skarmory, Houndour–Houndoom, Gastly–Gengar | Eevee–Umbreon, Skarmory, Gastly–Gengar, Zubat–Crobat |
| Very rare | Eevee–Espeon, Stantler | Eevee–Umbreon, Houndour–Houndoom |

#### Dragon's Den

Dungeon, Johto east.

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados | Dratini–Dragonite |
| Uncommon | Horsea–Seadra | Magikarp–Gyarados |
| Rare | Dratini–Dragonite, Horsea–Kingdra | Horsea–Seadra, Dratini–Dragonite |
| Very rare | Dratini–Dragonite | Horsea–Kingdra |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Magikarp–Gyarados, Horsea–Seadra | Dratini–Dragonite, Magikarp–Gyarados |
| Uncommon | Magikarp–Gyarados, Horsea–Seadra, Dratini–Dragonite | Dratini–Dragonite ×2, Horsea–Seadra |
| Rare | Magikarp–Gyarados, Dratini–Dragonite, Horsea–Seadra | Magikarp–Gyarados, Horsea–Seadra, Dratini–Dragonite |
| Very rare | Horsea–Kingdra, Dratini–Dragonite | Horsea–Kingdra, Dratini–Dragonite |

#### Whirl Islands

Dungeon, Johto west.

**`MAP_WHIRL_ISLANDS_1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong, Krabby–Kingler | Zubat–Golbat, Seel–Dewgong |
| Uncommon | Corsola, Wooper–Quagsire, Natu–Xatu, Zubat–Golbat | Corsola, Wooper–Quagsire, Krabby–Kingler, Gastly–Haunter |
| Rare | Seel–Dewgong, Corsola, Krabby–Kingler, Slowpoke–Slowbro | Seel–Dewgong, Corsola, Natu–Xatu, Gastly–Haunter |
| Very rare | Skarmory, Natu–Xatu | Skarmory, Slowpoke–Slowbro |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Tentacool–Tentacruel | Corsola |
| Uncommon | Corsola | Tentacool–Tentacruel |
| Rare | Mantine, Horsea–Seadra | Chinchou–Lanturn, Mantine |
| Very rare | Mantyke | Horsea–Seadra |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Corsola, Krabby–Kingler | Krabby–Kingler, Corsola |
| Uncommon | Qwilfish, Horsea–Seadra, Corsola | Chinchou–Lanturn, Qwilfish, Staryu |
| Rare | Krabby–Kingler, Staryu, Qwilfish | Corsola, Chinchou–Lanturn, Staryu |
| Very rare | Chinchou–Lanturn, Horsea–Seadra | Horsea–Seadra, Qwilfish |

**`MAP_WHIRL_ISLANDS_B1F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Slowpoke–Slowbro, Corsola | Slowpoke–Slowbro, Corsola |
| Uncommon | Seel–Dewgong, Marill–Azumarill, Wooper–Quagsire, Natu–Xatu | Zubat–Golbat, Seel–Dewgong, Gastly–Haunter, Marill–Azumarill |
| Rare | Zubat–Golbat, Slowpoke–Slowking, Skarmory, Corsola | Zubat–Crobat, Slowpoke–Slowking, Skarmory, Gastly–Haunter |
| Very rare | Slowpoke–Slowking, Skarmory | Gastly–Gengar, Natu–Xatu |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra | Chinchou–Lanturn |
| Uncommon | Chinchou–Lanturn | Horsea–Seadra |
| Rare | Mantine, Corsola | Mantine, Horsea–Kingdra |
| Very rare | Mantyke | Corsola |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra, Chinchou–Lanturn | Chinchou–Lanturn, Horsea–Seadra |
| Uncommon | Corsola, Qwilfish, Horsea–Seadra | Corsola, Staryu, Qwilfish |
| Rare | Corsola, Staryu, Remoraid–Octillery | Staryu, Chinchou–Lanturn, Remoraid–Octillery |
| Very rare | Horsea–Kingdra, Chinchou–Lanturn | Horsea–Kingdra, Shellder–Cloyster |

**`MAP_WHIRL_ISLANDS_B1F_INNER_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Corsola, Seel–Dewgong | Seel–Dewgong, Corsola |
| Uncommon | Slowpoke–Slowbro, Marill–Azumarill, Wooper–Quagsire, Natu–Xatu | Zubat–Golbat, Slowpoke–Slowbro, Gastly–Haunter, Marill–Azumarill |
| Rare | Slowpoke–Slowking, Zubat–Golbat, Skarmory, Seel–Dewgong | Zubat–Crobat, Slowpoke–Slowking, Gastly–Gengar, Skarmory |
| Very rare | Skarmory, Slowpoke–Slowking | Gastly–Haunter, Natu–Xatu |

**`MAP_WHIRL_ISLANDS_B2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong, Slowpoke–Slowbro | Seel–Dewgong, Slowpoke–Slowbro |
| Uncommon | Corsola, Marill–Azumarill, Wooper–Quagsire, Skarmory | Zubat–Golbat, Corsola, Gastly–Haunter, Skarmory |
| Rare | Slowpoke–Slowking, Natu–Xatu, Skarmory, Zubat–Crobat | Zubat–Crobat, Gastly–Gengar, Slowpoke–Slowking, Skarmory |
| Very rare | Natu–Xatu, Slowpoke–Slowking | Wooper–Quagsire, Natu–Xatu |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Seadra | Qwilfish |
| Uncommon | Qwilfish | Horsea–Seadra |
| Rare | Lapras, Mantine | Chinchou–Lanturn, Lapras |
| Very rare | Horsea–Kingdra | Horsea–Kingdra |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Qwilfish, Horsea–Seadra | Qwilfish, Horsea–Seadra |
| Uncommon | Corsola, Chinchou–Lanturn, Qwilfish | Chinchou–Lanturn, Corsola, Staryu |
| Rare | Horsea–Seadra, Horsea–Kingdra, Remoraid–Octillery | Chinchou–Lanturn, Horsea–Kingdra, Staryu |
| Very rare | Shellder–Cloyster, Staryu | Shellder–Cloyster, Remoraid–Octillery |

**`MAP_WHIRL_ISLANDS_B3F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Corsola, Seel–Dewgong | Corsola, Seel–Dewgong |
| Uncommon | Slowpoke–Slowbro, Skarmory, Wooper–Quagsire, Marill–Azumarill | Slowpoke–Slowbro, Zubat–Golbat, Gastly–Haunter, Skarmory |
| Rare | Slowpoke–Slowking, Skarmory, Natu–Xatu, Zubat–Crobat | Zubat–Crobat, Gastly–Gengar, Slowpoke–Slowking, Skarmory |
| Very rare | Natu–Xatu, Skarmory | Wooper–Quagsire, Natu–Xatu |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Qwilfish | Horsea–Seadra |
| Uncommon | Horsea–Seadra | Qwilfish |
| Rare | Lapras, Mantine | Chinchou–Lanturn, Lapras |
| Very rare | Horsea–Kingdra | Horsea–Kingdra |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Qwilfish, Horsea–Seadra | Horsea–Seadra, Qwilfish |
| Uncommon | Corsola, Krabby–Kingler, Horsea–Seadra | Chinchou–Lanturn, Corsola, Staryu |
| Rare | Qwilfish, Horsea–Kingdra, Staryu | Chinchou–Lanturn, Horsea–Kingdra, Krabby–Kingler |
| Very rare | Shellder–Cloyster, Chinchou–Lanturn | Shellder–Cloyster, Staryu |

**`MAP_WHIRL_ISLANDS_DESCENT_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Seel–Dewgong, Natu–Xatu | Seel–Dewgong, Natu–Xatu |
| Uncommon | Corsola, Slowpoke–Slowking, Skarmory, Zubat–Crobat | Corsola, Slowpoke–Slowking, Gastly–Gengar, Skarmory |
| Rare | Skarmory, Wooper–Quagsire, Slowpoke–Slowking, Natu–Xatu | Zubat–Crobat, Skarmory, Wooper–Quagsire, Gastly–Gengar |
| Very rare | Marill–Azumarill, Zubat–Crobat | Marill–Azumarill, Slowpoke–Slowking |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Kingdra | Chinchou–Lanturn |
| Uncommon | Chinchou–Lanturn | Horsea–Kingdra |
| Rare | Lapras, Mantine | Lapras, Tentacool–Tentacruel |
| Very rare | Tentacool–Tentacruel | Mantine |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Horsea–Kingdra, Chinchou–Lanturn | Chinchou–Lanturn, Horsea–Kingdra |
| Uncommon | Qwilfish, Shellder–Cloyster, Horsea–Kingdra | Qwilfish, Staryu, Shellder–Cloyster |
| Rare | Corsola, Staryu, Chinchou–Lanturn | Chinchou–Lanturn, Horsea–Kingdra, Staryu |
| Very rare | Shellder–Cloyster, Remoraid–Octillery | Corsola, Remoraid–Octillery |

#### Mt. Silver

Dungeon, Border.

**`MAP_MT_SILVER_OUTSIDE_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Phanpy–Donphan, Ponyta–Rapidash | Teddiursa–Ursaring, Phanpy–Donphan |
| Uncommon | Teddiursa–Ursaring, Geodude–Graveler, Spearow–Fearow, Doduo–Dodrio | Zubat–Golbat, Houndour–Houndoom, Wooper–Quagsire, Sneasel |
| Rare | Skarmory ×2, Tauros, Larvitar–Tyranitar | Misdreavus, Skarmory, Larvitar–Tyranitar, Houndour–Houndoom |
| Very rare | Snorlax, Larvitar–Tyranitar | Snorlax, Murkrow |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Poliwag–Poliwhirl | Poliwag–Poliwhirl |
| Rare | Goldeen–Seaking, Lapras | Wooper–Quagsire, Lapras |
| Very rare | Marill–Azumarill | Goldeen–Seaking |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwhirl, Psyduck–Golduck | Poliwag–Poliwhirl, Psyduck–Golduck |
| Uncommon | Magikarp–Gyarados, Goldeen–Seaking, Poliwag–Poliwhirl | Magikarp–Gyarados, Wooper–Quagsire, Poliwag–Poliwhirl |
| Rare | Magikarp–Gyarados, Goldeen–Seaking, Psyduck–Golduck | Wooper–Quagsire, Goldeen–Seaking, Psyduck–Golduck |
| Very rare | Magikarp–Gyarados, Poliwag–Poliwhirl | Magikarp–Gyarados, Poliwag–Poliwhirl |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pineco–Forretress | Teddiursa–Ursaring |
| Uncommon | Teddiursa–Ursaring | Pineco–Forretress |
| Rare | Heracross, Scyther | Hoothoot–Noctowl, Heracross |
| Very rare | Aipom | Scyther |

**`MAP_MT_SILVER_1F_WATERFALL_ROOM_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Onix, Teddiursa–Ursaring | Onix, Teddiursa–Ursaring |
| Uncommon | Zubat–Golbat, Geodude–Graveler, Machop–Machoke, Paras–Parasect | Zubat–Golbat, Geodude–Graveler, Misdreavus, Paras–Parasect |
| Rare | Psyduck–Golduck, Larvitar–Tyranitar, Electabuzz, Magmar | Psyduck–Golduck, Larvitar–Tyranitar, Magmar, Misdreavus–Mismagius |
| Very rare | Onix–Steelix, Elekid | Zubat–Crobat, Magby |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Slowpoke–Slowbro |
| Uncommon | Slowpoke–Slowbro | Psyduck–Golduck |
| Rare | Lapras, Goldeen–Seaking | Lapras, Wooper–Quagsire |
| Very rare | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck, Slowpoke–Slowbro | Slowpoke–Slowbro, Psyduck–Golduck |
| Uncommon | Magikarp–Gyarados, Goldeen–Seaking, Psyduck–Golduck | Magikarp–Gyarados, Wooper–Quagsire, Goldeen–Seaking |
| Rare | Magikarp–Gyarados, Poliwag–Poliwrath, Slowpoke–Slowbro | Wooper–Quagsire, Poliwag–Politoed, Slowpoke–Slowking |
| Very rare | Magikarp–Gyarados, Goldeen–Seaking | Magikarp–Gyarados, Psyduck–Golduck |

**`MAP_MT_SILVER_MOUNTAIN_SIDE_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Ponyta–Rapidash, Teddiursa–Ursaring | Teddiursa–Ursaring, Ponyta–Rapidash |
| Uncommon | Skarmory, Machop–Machamp, Phanpy–Donphan, Geodude–Golem | Houndour–Houndoom, Zubat–Crobat, Misdreavus–Mismagius, Sneasel–Weavile |
| Rare | Larvitar–Tyranitar, Tauros, Electabuzz, Skarmory | Larvitar–Tyranitar, Houndour–Houndoom, Murkrow–Honchkrow, Gastly–Gengar |
| Very rare | Electabuzz–Electivire, Larvitar–Tyranitar | Snorlax, Larvitar–Tyranitar |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwrath | Poliwag–Poliwrath |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Lapras, Slowpoke–Slowbro | Wooper–Quagsire, Lapras |
| Very rare | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Poliwag–Poliwrath, Magikarp–Gyarados | Poliwag–Poliwrath, Magikarp–Gyarados |
| Uncommon | Psyduck–Golduck, Slowpoke–Slowbro, Goldeen–Seaking | Psyduck–Golduck, Wooper–Quagsire, Goldeen–Seaking |
| Rare | Magikarp–Gyarados, Poliwag–Poliwrath, Psyduck–Golduck | Wooper–Quagsire, Poliwag–Politoed, Slowpoke–Slowking |
| Very rare | Magikarp–Gyarados, Poliwag–Politoed | Magikarp–Gyarados, Psyduck–Golduck |

*Trees and rocks*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Pineco–Forretress | Teddiursa–Ursaring |
| Uncommon | Teddiursa–Ursaring | Pineco–Forretress |
| Rare | Heracross, Scyther–Scizor | Hoothoot–Noctowl, Heracross |
| Very rare | Aipom–Ambipom | Scyther–Scizor |

**`MAP_MT_SILVER_1F_ITEM_ROOM_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Geodude–Golem, Machop–Machamp | Geodude–Golem, Machop–Machamp |
| Uncommon | Teddiursa–Ursaring, Onix–Steelix, Zubat–Crobat, Psyduck–Golduck | Zubat–Crobat, Onix–Steelix, Misdreavus–Mismagius, Teddiursa–Ursaring |
| Rare | Larvitar–Tyranitar, Electabuzz, Magmar, Elekid | Larvitar–Tyranitar, Magmar, Electabuzz, Gastly–Gengar |
| Very rare | Larvitar–Tyranitar, Magby | Larvitar–Tyranitar, Magby |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck | Psyduck–Golduck |
| Uncommon | Slowpoke–Slowbro | Slowpoke–Slowbro |
| Rare | Lapras, Goldeen–Seaking | Wooper–Quagsire, Lapras |
| Very rare | Magikarp–Gyarados | Poliwag–Politoed |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Psyduck–Golduck, Slowpoke–Slowbro | Psyduck–Golduck, Slowpoke–Slowbro |
| Uncommon | Magikarp–Gyarados, Goldeen–Seaking, Psyduck–Golduck | Magikarp–Gyarados, Wooper–Quagsire, Goldeen–Seaking |
| Rare | Magikarp–Gyarados, Slowpoke–Slowking, Poliwag–Poliwrath | Wooper–Quagsire, Slowpoke–Slowking, Poliwag–Politoed |
| Very rare | Magikarp–Gyarados, Goldeen–Seaking | Magikarp–Gyarados, Psyduck–Golduck |

**`MAP_MT_SILVER_1F_MOLTRES_ROOM_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Paras–Parasect, Machop–Machamp | Paras–Parasect, Machop–Machamp |
| Uncommon | Onix, Geodude–Golem, Magmar, Electabuzz | Zubat–Crobat, Misdreavus–Mismagius, Magmar, Gastly–Gengar |
| Rare | Onix–Steelix, Magmar–Magmortar, Larvitar–Tyranitar, Electabuzz–Electivire | Onix–Steelix, Magmar–Magmortar, Larvitar–Tyranitar, Electabuzz |
| Very rare | Magby, Larvitar–Tyranitar | Magby, Larvitar–Tyranitar |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking | Goldeen–Seaking |
| Uncommon | Psyduck–Golduck | Psyduck–Golduck |
| Rare | Lapras, Magikarp–Gyarados | Wooper–Quagsire, Lapras |
| Very rare | Poliwag–Poliwrath | Poliwag–Politoed |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Goldeen–Seaking, Psyduck–Golduck | Goldeen–Seaking, Psyduck–Golduck |
| Uncommon | Magikarp–Gyarados, Poliwag–Poliwrath, Goldeen–Seaking | Magikarp–Gyarados, Wooper–Quagsire, Goldeen–Seaking |
| Rare | Magikarp–Gyarados, Psyduck–Golduck, Poliwag–Politoed | Wooper–Quagsire, Psyduck–Golduck, Poliwag–Politoed |
| Very rare | Magikarp–Gyarados, Goldeen–Seaking | Magikarp–Gyarados, Goldeen–Seaking |

**`MAP_MT_SILVER_2F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire, Machop–Machamp | Wooper–Quagsire, Machop–Machamp |
| Uncommon | Psyduck–Golduck, Geodude–Golem, Larvitar–Tyranitar, Zubat–Crobat | Zubat–Crobat, Misdreavus–Mismagius, Geodude–Golem, Larvitar–Tyranitar |
| Rare | Electabuzz, Magmar, Electabuzz–Electivire, Magmar–Magmortar | Gastly–Gengar, Magmar, Electabuzz, Psyduck–Golduck |
| Very rare | Larvitar–Tyranitar, Paras–Parasect | Larvitar–Tyranitar, Snorlax |

*Surfing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire | Wooper–Quagsire |
| Uncommon | Magikarp–Gyarados | Magikarp–Gyarados |
| Rare | Lapras, Psyduck–Golduck | Lapras, Poliwag–Politoed |
| Very rare | Goldeen–Seaking | Goldeen–Seaking |

*Fishing*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Wooper–Quagsire, Magikarp–Gyarados | Wooper–Quagsire, Magikarp–Gyarados |
| Uncommon | Psyduck–Golduck, Goldeen–Seaking, Wooper–Quagsire | Psyduck–Golduck, Poliwag–Politoed, Wooper–Quagsire |
| Rare | Magikarp–Gyarados, Goldeen–Seaking, Psyduck–Golduck | Poliwag–Politoed, Goldeen–Seaking, Psyduck–Golduck |
| Very rare | Magikarp–Gyarados, Lapras | Magikarp–Gyarados, Lapras |

**`MAP_MT_SILVER_SNOW_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Swinub–Mamoswine, Teddiursa–Ursaring | Sneasel–Weavile, Swinub–Mamoswine |
| Uncommon | Sneasel–Weavile, Skarmory, Mankey–Annihilape, Ponyta–Rapidash | Houndour–Houndoom, Zubat–Crobat, Misdreavus–Mismagius, Delibird |
| Rare | Larvitar–Tyranitar, Jynx, Delibird, Skarmory | Gastly–Gengar, Larvitar–Tyranitar, Jynx, Murkrow–Honchkrow |
| Very rare | Snorlax, Larvitar–Tyranitar | Smoochum, Sneasel–Weavile |

**`MAP_MT_SILVER_3F_HNS`**

*Land*

| Tier | Day | Night |
| --- | --- | --- |
| Common | Teddiursa–Ursaring, Zubat–Crobat | Zubat–Crobat, Teddiursa–Ursaring |
| Uncommon | Onix–Steelix, Geodude–Golem, Larvitar–Tyranitar, Machop–Machamp | Onix–Steelix, Geodude–Golem, Larvitar–Tyranitar, Misdreavus–Mismagius |
| Rare | Larvitar–Tyranitar, Skarmory, Electabuzz–Electivire, Magmar–Magmortar | Gastly–Gengar, Larvitar–Tyranitar, Sneasel–Weavile, Houndour–Houndoom |
| Very rare | Snorlax, Larvitar–Tyranitar | Snorlax, Larvitar–Tyranitar |


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
