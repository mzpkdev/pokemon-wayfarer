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

### Variety

- Each map has a mix of its own, not a copy of its neighbour's.
- No family is one of a table's two most common slots on more than eight maps.
- Open water may repeat more, since Hoenn's sea is large and its water species
  are fewer. For surfing and fishing tables the limit is twelve maps. Sea maps
  differ through their mix, their rarities and the gradient instead.

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
audit must choose Generation III or V carriers when it is re-run.

## Open questions

- The values for the Gen V shares, the repetition caps and the night share.
- How rare a reward slot or a baby slot is.
