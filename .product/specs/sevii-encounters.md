# Sevii encounters

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. It sets the rules and targets for the Sevii Islands'
encounter tables. The tables themselves follow in a Sevii table spec. Every
share and count below is a placeholder for playtesting.

## Scope

This spec defines how Sevii tables are built:

- the table format;
- how Galar's species are spread across the islands;
- the blend of other generations' species;
- what must be catchable in Sevii;
- the rules for habitat, reach, rewards, babies, variety, and day and night;
- the change of Sevii's region to Galar.

It doesn't cover:

- the per-map tables, which belong to the Sevii table spec;
- legendaries and mythicals, which belong to a later spec;
- reach values, which belong to [Reach assignments](reach-assignments.md).

## Behavior

### Table format

Sevii uses the same format as [Kanto and Johto](kanto-johto-encounters.md):

- Each map has a table for each of its methods: land, surfing, fishing (the
  [Standard Rod](standard-rod-fishing.md)'s ten entries and three qualities),
  and Rock Smash rocks.
- Each method has a day table and a night table.
- Each slot holds a species line, written as its stage cap, and a rarity
  weight. Slots hold no levels.

The tables are designed fresh. The old FireRed and LeafGreen tables are only a
reference for what lived where.

### Natives: Galar across the islands

Sevii's natives are Galar's Generation VIII species and the Galarian forms of
older species. The Hisuian part of Generation VIII lives in Sinjoh instead.

The island chain follows how Galar feels, from its pastoral countryside on the
near islands to its wild, rugged lands on the outer ones. Everyday Galar
species live near home, and rare, strong ones further out.

| Islands | Places | Feels like | Example species |
| --- | --- | --- | --- |
| One, Two and Three | One Island, Treasure Beach, Cape Brink, Three Isle Port, Bond Bridge | Galar's countryside: farms, hedgerows, beaches | Wooloo, Skwovet, Rookidee, Blipbug, Nickit, Gossifleur, Yamper, Chewtle, Galarian Zigzagoon, Galarian Meowth |
| One | Kindle Road, Mt. Ember | Coal country and volcano | Rolycoly, Sizzlipede, Galarian Darumaka |
| Three | Berry Forest | The fairy forest | Applin, Milcery, Impidimp, Hatenna, Galarian Ponyta |
| Four | Four Island, Icefall Cave | The Crown Tundra's snow | Snom, Eiscue, Cufant |
| Five | Five Island, Five Isle Meadow, Memorial Pillar, Resort Gorgeous, Water Labyrinth, Lost Cave | Haunted Galar, with a meadow and a resort | Sinistea, Dreepy, Galarian Corsola, Galarian Yamask |
| Six | Water Path, Ruin Valley, Green Path, Pattern Bush, Outcast Island, Altering Cave | Ancient sites and the Isle of Armor's wilds | Stonjourner, Galarian Farfetch'd, Galarian Slowpoke, Falinks |
| Seven | Sevault Canyon and its entrance, Tanoby Ruins and its chambers, Trainer Tower grounds | The far, rugged edge | Duraludon, Falinks, Galarian Mr. Mime, Galarian Stunfisk |

### The blend

A few species from other included generations may appear in Sevii where they
make geographic or thematic sense. They add variety without crowding out
Galar:

- **They must fit the place.** For example: Kanto's sea species, such as
  Tentacool, Horsea, Krabby and Magikarp, drifting into the near islands'
  waters from nearby Kanto; Unown in the Tanoby Ruins; Johto's bugs in Pattern
  Bush; and Koffing in Mt. Ember's coal country, where it evolves into Galarian
  Weezing.
- **They stay a minority:** at most ~20% of slot weight on the near islands and
  ~10% on the outer islands, measured per island group across day and night.
  The Kanto drift at sea fades from the near islands outward.
- **They never lead a table.** A blend species is never one of a table's two
  most common slots.
- **They never outshine Galar.** Blend species are everyday species or harmless
  finds, never fierce or dangerous rewards.
- **They don't replace a home.** Every blend species is already catchable in its
  own region.

### Sevii's region

Sevii's maps change from the Kanto region to the **Galar region**. Galar-only
evolutions then work there, as Hisui-only evolutions already do in Sinjoh:

- Koffing evolves into Galarian Weezing.
- Mime Jr. evolves into Galarian Mr. Mime when it knows Mimic.

The implementation must check anything else that reads the region, such as the
Pokédex or the Town Map, before changing it.

### What must be catchable

- **Every Galar family can be caught in Sevii.** Its lowest Generation VIII
  stage appears somewhere in Sevii's tables.
- **Every Galarian form can be caught in Sevii,** either directly or by
  evolving there, such as Galarian Weezing from Koffing.
- A later stage reached by level, stone, trade or friendship counts as
  catchable when its base form is.

Other sources count where the PRD says so:

- **Starters:** Grookey, Scorbunny and Sobble are harmless rewards in Sevii.
- **Toxel** appears as a rare baby find.
- **Dracozolt, Arctozolt, Dracovish and Arctovish** come from fossils in Sevii.
- **Legendaries and mythicals** are left to a later spec.

The Sevii table spec ends with a coverage checklist that shows where every
family is catchable.

### Reach

- Rewards follow their temperament: harmless anywhere, fierce from Wilds
  outward, dangerous only in Outlands and dungeons.
- Stages reached by item, trade or friendship appear only in Outlands and on a
  dungeon's deeper floors.
- Deeper dungeon floors hold rarer species than their entrances.

### Rewards and babies

- Most maps hold one to three reward slots. Deep dungeon floors may hold more.
- Every Galar reward appears somewhere in Sevii.
- Babies appear only as rare slots named directly in a table, never through
  the downward rule. They keep young levels.

### Variety

- Each map has a mix of its own, not a copy of its neighbour's.
- No family is one of a table's two most common slots on more than eight maps.
- Open water repeats more, since Galar has few water species: about six water
  families for Sevii's surfing and fishing tables. For those tables the limit
  is twelve maps. Sea maps differ through their mix, their rarities and the
  blend.

### Day and night

Sevii's night tables are copies of its day tables today, so every night table
is new.

- Every map has its own night table.
- Outdoors, at least 30% of a night table's slot weight goes to species that
  don't appear in that map's day table, such as Nickit, Impidimp, Sinistea and
  Dreepy.
- Caves and buildings change more lightly. Their day and night tables may
  overlap, but each has at least one species that appears only at night or is
  much more common then.
- Fishing and Rock Smash have night tables too, with the same light difference
  as caves.

### Native HM sources

The catch-window audit must check Sevii's native HM carriers when it is
re-run, since Sevii's species change completely.

## Open questions

- The values for the blend caps, the repetition caps and the night share.
- How rare a reward slot or a baby slot is.
