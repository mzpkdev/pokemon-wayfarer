# Hoenn encounters

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. It sets the rules and targets for Hoenn's encounter
tables. The tables themselves follow in a Hoenn table spec. Every share and
count below is a placeholder for playtesting.

## Scope

This spec defines how Hoenn tables are built:

- the table format;
- how Generations III and V are spread across Hoenn;
- what must be catchable in Hoenn;
- nostalgia anchors, what every route offers, and water;
- the rules for habitat, reach, rewards, babies, variety, day and night,
  fishing, and Rock Smash rocks.

It doesn't cover:

- the per-map tables, which belong to the Hoenn table spec;
- the Safari Zone, which is deferred;
- legendaries and mythicals, which belong to a later spec;
- reach values, which belong to [Reach assignments](reach-assignments.md).

## Behavior

### Table format

Hoenn uses the same format as [Kanto and Johto](kanto-johto-encounters.md):

- Each map has a table for each of its methods: land, surfing, fishing (the
  [Standard Rod](standard-rod-fishing.md)'s ten entries and three qualities),
  and Rock Smash rocks. Hoenn has no Headbutt trees.
- Each method has a day table and a night table.
- Each slot holds a species line, written as its stage cap, and a rarity
  weight. Slots hold no levels.

The tables are designed fresh. The old tables are only a reference for what
lived where.

### Natives and the west-to-east gradient

Hoenn's natives are Generations III and V. No Generation I or II species
appears in Hoenn's wild tables, except as part of a family that counts as
Generation III, such as Marill's line through Azurill and Wobbuffet's line
through Wynaut.

Gen III is Hoenn's identity. Gen V's share of encounter slots rises the
further east you travel:

| Band | Areas | Gen V share |
| --- | --- | --- |
| West | Petalburg, Dewford, Slateport, Routes 101–109, Petalburg Woods, Granite Cave, Altering Cave, Abandoned Ship | ~10–20% |
| Centre | Routes 110–118, Rusturf Tunnel, Fiery Path, Jagged Pass, Meteor Falls, New Mauville, Desert Underpass, Mirage Tower, Magma Hideout | ~30–40% |
| East | Lilycove, Routes 119–123, Mt. Pyre | ~40–50% |
| Far east | Mossdeep, Sootopolis, Pacifidlog, Ever Grande, Routes 124–134, underwater Routes 124 and 126, Shoal Cave, Seafloor Cavern, Sky Pillar, Cave of Origin, Victory Road, Artisan Cave | ~50–60% |

A share is measured across all of a band's tables, day and night together, by
slot weight. A family counts as the generation of its base form.

Gen III never disappears. Its sea natives, such as Wingull, Wailmer,
Clamperl, Relicanth and Luvdisc, still belong in the far east, so Gen III stays
at least a large minority there.

### What must be catchable

- **Every Generation III family can be caught in Hoenn.** Its lowest
  Generation III stage appears somewhere in Hoenn's tables. This includes
  Azurill and Wynaut, which appear as rare baby finds.
- **Every Generation V family can be caught in Hoenn.** Its lowest Generation V
  stage appears somewhere in Hoenn's tables. Gen V's Galarian and Hisuian forms
  live in Sevii and Sinjoh instead.
- A later stage reached by level, stone, trade or friendship counts as
  catchable when its base form is.

Other sources count where the PRD says so:

- **Starters** are harmless rewards: Hoenn's three and Unova's three, all in
  Hoenn.
- **Lileep, Anorith, Tirtouga and Archen** come from fossils in Hoenn.
- **Legendaries and mythicals** are left to a later spec.

The Hoenn table spec ends with a coverage checklist that shows where every
family is catchable.

### Nostalgia anchors

Hoenn keeps its classic highlights in their original spots, where they are
Generation III species. Emerald is the reference. Anchors include:

| Place | Anchors |
| --- | --- |
| Routes 101–103 | Zigzagoon, Poochyena and Wurmple |
| Route 102 | Ralts, rare |
| Petalburg Woods | Shroomish, Slakoth, Silcoon and Cascoon |
| Route 104 | Taillow and Wingull |
| Granite Cave | Makuhita, Aron, Sableye and Mawile |
| Route 110 | Electrike, Plusle and Minun |
| Route 111's desert | Trapinch, Cacnea and Baltoy |
| Route 113 | Spinda |
| Fiery Path and Jagged Pass | Numel, Torkoal and Spoink |
| Route 117 | Volbeat, Illumise and Roselia |
| Meteor Falls | Lunatone and Solrock, with Bagon deep inside |
| Route 119 | Kecleon, and Feebas as a rare fishing secret |
| Route 120 | Absol and Kecleon |
| Mt. Pyre | Shuppet and Duskull, with Chimecho at the summit |
| Shoal Cave | Spheal and Snorunt |
| Sky Pillar | Claydol, Banette, Sableye and Altaria |
| Victory Road | Hariyama, Lairon and Medicham |

An anchor holds one of its table's common or uncommon slots, unless it was a
rarity in Emerald, such as Ralts or Feebas, where it stays rare but present.
Reach rules come first: a fierce or dangerous reward stays off Roads even where
it lived in Emerald, so Tropius leaves Route 119 for nearby Wilds or dungeons.

Generation I and II species that lived in Emerald's Hoenn, such as Zubat in
caves, Tentacool at sea and Magikarp in the water, don't return. Their places
go to Generation III and V species that fit.

### Classic core, more around it

Every route is worth a trip on its own, including the first routes. A route's
common slots keep its classic Generation III cast, and around that core every
route offers fitting uncommon species, a real night table and at least one
reward slot.

### Habitat

Species live where they belong. Some examples:

| Habitat | Gen III | Gen V |
| --- | --- | --- |
| Desert | Trapinch, Cacnea, Baltoy | Sandile, Darumaka, Maractus, Sigilyph, Yamask |
| Caves | Whismur, Aron, Makuhita, Nosepass, Sableye, Mawile | Woobat, Roggenrola, Drilbur, Durant |
| Volcano and ash | Numel, Torkoal, Spinda | Larvesta, Heatmor |
| Forest | Wurmple, Shroomish, Seedot | Sewaddle, Venipede, Cottonee, Petilil |
| Sea | Wingull, Wailmer, Carvanha, Clamperl, Relicanth | Frillish, Alomomola, Basculin, Ducklett, Tynamo |
| Ice | Snorunt, Spheal | Vanillite, Cubchoo, Cryogonal |
| Towers and ruins | Shuppet, Duskull, Baltoy | Litwick, Golett, Sigilyph |

### Reach

- Rewards follow their temperament: harmless anywhere, fierce from Wilds
  outward, dangerous only in Outlands and dungeons.
- Stages reached by item, trade or friendship appear only in Outlands and on a
  dungeon's deeper floors. This includes Generation IV extensions of Hoenn
  lines, such as Gallade, Froslass, Probopass, Dusknoir and Roserade, and
  Kingambit.
- Deeper dungeon floors hold rarer species than their entrances.

### Rewards and babies

- Most maps hold one to three reward slots. Deep dungeon floors may hold more.
- Every Generation III and Generation V reward appears somewhere in Hoenn.
- Babies appear only as rare slots named directly in a table, never through
  the downward rule. They keep young levels.

### Water

Every map with surfing or fishing has one water type. The Hoenn table spec names
each map's type.

| Water type | Where | Cast |
| --- | --- | --- |
| Ponds and rivers | Fresh water on routes, in towns and in woods | Lotad, Surskit, Barboach, Corphish, Carvanha, Marill, Basculin, Tympole, Ducklett |
| Coast and sea | Coastal towns and sea routes | Wingull, Wailmer, Carvanha, Luvdisc, Corphish, Frillish, Alomomola, Ducklett, Tynamo |
| Underwater | The Dive maps on Routes 124 and 126 | Clamperl, Relicanth, Frillish, Alomomola |
| Cold water | Shoal Cave's icy rooms | Spheal, Wailmer, Carvanha, Frillish |
| Cave water | Water inside caves, such as Meteor Falls and the Seafloor Cavern | Barboach, Corphish, Marill, Tympole, Basculin, and sea species in sea caves |

Rewards, babies, anchors and native HM carriers may appear outside their type's
cast where they fit, such as Feebas on Route 119 and the water starters.

- **Fishing:** entries 1–3, which make up about 70% of Old Rod catches, hold
  the map's most fitting common fish. Entries 3–10 hold at least four different
  families and carry the map's own character, which the better rods reveal.
  Feebas stays a rare secret of Route 119 and never leads a table.
- **Native HM crossings:** fishing entries 1–3 include a local Surf carrier by
  day and by night on Route 118 and at Lilycove, Mossdeep and Pacifidlog. The
  catch-window audit still decides whether each carrier knows the move at its
  level.
- **Surfing:** no family holds a surfing table's first slot on more than twelve
  maps, since Hoenn's sea is large and its water species are fewer. Sea maps
  differ through their mix, their rarities and the gradient.

### Variety

- Each map has a mix of its own, not a copy of its neighbour's.
- No family is one of a table's two most common slots on more than eight maps,
  counted per method for land and for Rock Smash.

### Day and night

Hoenn's tables have no night versions today, so every night table is new.

- Every map has its own night table.
- Outdoors, at least 30% of a night table's slot weight goes to species that
  don't appear in that map's day table, such as Poochyena, Duskull, Shuppet,
  Purrloin, Woobat, Munna and Litwick.
- Caves and buildings change more lightly. Their day and night tables may
  overlap, but each has at least one species that appears only at night or is
  much more common then.
- Fishing and Rock Smash have night tables too, with the same light difference
  as caves.

### Native HM sources

Some of Hoenn's native HM carriers are Generation I or II species, such as
Tentacool, Geodude and Chinchou. They leave Hoenn's tables, so the catch-window
audit must choose Generation III or V carriers when it is re-run. Until then,
the crossings use the Generation III Surf carriers already on the roster, such
as Carvanha, Wingull, Lotad and Marill.

## Open questions

- The values for the Gen V shares, the repetition caps and the night share.
- How rare a reward slot or a baby slot is.
