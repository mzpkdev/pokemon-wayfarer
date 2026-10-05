# Alola encounters

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. It sets the rules and targets for Alola's encounter
tables. The tables themselves are in the
[Alola table spec](alola-encounter-tables.md). Every share and count below is a
placeholder for playtesting.

## Scope

This spec defines how Alola tables are built:

- the table format;
- how Generation VII and the Alolan forms are spread across the islands;
- the blend of other generations' species;
- what must be catchable in Alola;
- the rules for habitat, reach, rewards, variety, and day and night.

It doesn't cover:

- the per-map tables, which belong to the Alola table spec;
- legendaries, mythicals, Ultra Beasts, Type: Null and Cosmog's line, which
  belong to a later spec;
- reach values, which belong to [Reach assignments](reach-assignments.md).

## Behavior

### Table format

Alola uses the same format as [Kanto and Johto](kanto-johto-encounters.md):

- Each map has a table for each of its methods: land, surfing, and fishing (the
  [Standard Rod](standard-rod-fishing.md)'s ten entries and three qualities).
- Each method has a day table and a night table.
- Each slot holds a species line, written as its stage cap, and a rarity
  weight. Slots hold no levels.

Each island is a single map that holds its village and its wild ground, so one
island has one land table for day and one for night. Island tables are
therefore fuller than a typical route's: about ten different species each. If
the islands are later split into smaller maps, their species spread across the
new maps.

The tables are designed fresh. The old tables are only a reference for what
lived where.

### Natives across the islands

Alola's natives are Generation VII and the Alolan forms of older species. Each
island has its own character, as in the original games:

| Place | Feels like | Example species |
| --- | --- | --- |
| Melemele Isle | The gentle first island | Pikipek, Yungoos, Alolan Rattata, Grubbin, Cutiefly, Crabrawler, Rockruff, Alolan Meowth |
| Akala Isle | Lush hills, beaches and a volcano | Fomantis, Morelull, Mudbray, Stufful, Bounsweet, Dewpider, Comfey, Pa'u Oricorio, Wimpod, Sandygast, Alolan Diglett, Cubone |
| Akala Forest | A deep forest | Fomantis, Morelull, Bounsweet, Comfey, Pom-Pom Oricorio, Passimian, Oranguru |
| Akala Cave | A volcanic cave | Salandit, Alolan Geodude, Alolan Diglett, Turtonator |
| Ula'ula Isle | Mountains, snow and an old power plant | Komala, Togedemaru, Drampa, Turtonator, Oranguru, Passimian, Baile Oricorio, Minior, Alolan Sandshrew, Alolan Vulpix, Alolan Grimer, Crabrawler |
| Ula'ula caves | An icy cave and a haunted one | Alolan Sandshrew, Alolan Vulpix, Crabrawler, Mimikyu, Alolan Grimer |
| Poni Isle | Alola's wildest island | Jangmo-o, Sensu Oricorio, Mudbray, Alolan Exeggutor, Passimian |
| Poni Cave | A deep canyon cave | Jangmo-o, Minior, Lycanroc |
| Alola sea | Warm tropical water | Wishiwashi, Mareanie, Pyukumuku, Bruxish |

Oricorio takes a different style in each place, as in the original games:
Pom-Pom in Akala Forest, Pa'u on Akala Isle, Baile on Ula'ula Isle and Sensu on
Poni Isle. Reach rules come first, so fierce and dangerous species stay off
Melemele, the only Road:

- Pom-Pom Oricorio, a fierce reward, moves from Melemele to Akala Forest.
- Dhelmise, a dangerous reward, lives in Ula'ula's and Poni's water and in Poni
  Cave, not in the Alola sea, which is Wilds.

### The blend

A few species from other included generations may appear in Alola where they
make geographic or thematic sense, as in [Sevii](sevii-encounters.md). Alola's
original Pokédex mixed in many older species, so a light blend suits it:

- **They must fit the place.** For example: Pikachu, Exeggcute and Cubone,
  which evolve into Alolan Raichu, Exeggutor and Marowak there; tropical sea
  species such as Wingull, Corsola, Luvdisc and Lapras; and Magikarp in the
  islands' waters.
- **They stay a minority:** at most ~20% of slot weight on each island,
  measured across day and night.
- **They never lead a table.** A blend species is never one of a table's two
  most common slots.
- **They never outshine Alola's natives.** Blend species are everyday species
  or harmless finds, never fierce or dangerous rewards.
- **They don't replace a home.** Every blend species is already catchable in its
  own region.

### Region-only evolutions

Alola's maps already count as the Alola region, so Alolan evolutions work
there:

- Pikachu evolves into Alolan Raichu with a Thunder Stone.
- Exeggcute evolves into Alolan Exeggutor with a Leaf Stone.
- Cubone evolves into Alolan Marowak at level 28, at night.

### What must be catchable

- **Every Generation VII family can be caught in Alola.** Its lowest
  Generation VII stage appears somewhere in Alola's tables.
- **Every Alolan form can be caught in Alola,** either directly or by evolving
  there, such as Alolan Raichu from Pikachu.
- A later stage reached by level, stone, trade or friendship counts as
  catchable when its base form is.

Other sources count where the PRD says so:

- **Starters:** Rowlet, Litten and Popplio are harmless rewards in Alola.
- **Legendaries, mythicals, Ultra Beasts, Type: Null and Cosmog's line** are
  left to a later spec.

The Alola table spec ends with a coverage checklist that shows where every
family is catchable.

### Reach

- Rewards follow their temperament: harmless anywhere, fierce from Wilds
  outward, dangerous only in Outlands and dungeons. Melemele, the only Road
  island, holds only harmless rewards.
- Stages reached by item, trade or friendship appear only in Outlands and on a
  dungeon's deeper floors.

### Rewards

- Each island holds a few reward slots, more than a typical route, since an
  island is a whole area in one map.
- Every Generation VII and Alolan form reward appears somewhere in Alola.

### Variety

- Each island has a mix of its own.
- No family is one of a table's two most common slots on more than four maps,
  since Alola has only ten maps.

### Water

Every map with surfing or fishing has one water type. The Alola table spec
names each map's type.

| Water type | Where | Cast |
| --- | --- | --- |
| Coast and sea | The four islands' coasts and the Alola sea | Wishiwashi, Mareanie, Pyukumuku, Bruxish, Dhelmise, Popplio, and the blend's sea species: Magikarp, Tentacool, Wingull, Chinchou, Staryu, Shellder, Corsola, Luvdisc, Wailmer, Carvanha, Clamperl |
| Cave water | Poni Cave | Dewpider, Wimpod, Wishiwashi, Bruxish, Dhelmise, and the blend's fresh-water species: Magikarp, Barboach, Psyduck, Basculin, Chinchou, and Zubat on the wing |

- **Fishing:** entries 1 and 2 hold Alola natives, since blend species never
  lead a table. Entry 3 is usually Magikarp. Entries 3–10 hold at least four
  different families.
- **Surfing:** no family holds a surfing table's first slot on more than four
  maps.
- **Native HM crossings:** Alola has none. The boat from Route 13 links
  Melemele to Kanto, so no player is stranded without Surf.

### Day and night

Alola's tables have no night versions today, so every night table is new.

- **Sun by day, Moon by night.** Outdoors, the species split between Pokémon
  Sun and Moon follow the clock: Passimian, Turtonator and Alolan Vulpix's line
  appear by day, and Oranguru, Drampa and Alolan Sandshrew's line by night.
  Caves may hold both.
- **Lycanroc follows the clock too.** Rockruff's slots cap at Midday Lycanroc by
  day and Midnight Lycanroc by night. Own Tempo Rockruff, which becomes Dusk
  Lycanroc, is a rare find in Akala Cave.

- Every map has its own night table.
- Outdoors, at least 30% of a night table's slot weight goes to species that
  don't appear in that map's day table, such as Alolan Rattata, Morelull,
  Mimikyu and Alolan Meowth.
- Caves change more lightly. Their day and night tables may overlap, but each
  has at least one species that appears only at night or is much more common
  then.
- Fishing has night tables too, with the same light difference as caves.

## Open questions

- The values for the blend cap, the repetition cap and the night share.
- How rare a reward slot is.
