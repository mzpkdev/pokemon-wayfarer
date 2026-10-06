# Sevii encounters

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. It sets the rules and targets for the Sevii Islands'
encounter tables. The tables themselves are in the
[Sevii table spec](sevii-encounter-tables.md). Every share and count below is a
placeholder for playtesting.

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
| One | Kindle Road, Mt. Ember | Coal country and volcano | Rolycoly, Sizzlipede, Cufant, Koffing as Galarian Weezing, Scorbunny |
| Three | Berry Forest | The fairy forest | Applin, Milcery, Impidimp, Hatenna, Galarian Ponyta, Indeedee, Grookey |
| Four | Four Island, Icefall Cave | The Crown Tundra's snow | Snom, Eiscue, Cufant, Galarian Darumaka, Galarian Mr. Mime |
| Five | Five Island, Five Isle Meadow, Memorial Pillar, Resort Gorgeous, Water Labyrinth, Lost Cave | Haunted Galar, with a meadow and a resort | Sinistea, Dreepy, Galarian Corsola, Galarian Yamask, Impidimp, Hatenna |
| Six | Water Path, Ruin Valley, Green Path, Pattern Bush, Outcast Island, Altering Cave | Ancient sites and the Isle of Armor's wilds | Stonjourner, Galarian Farfetch'd, Galarian Slowpoke, Falinks, Silicobra, Morpeko |
| Seven | Sevault Canyon and its entrance, Tanoby Ruins and its chambers, Trainer Tower grounds | The far, rugged edge | Duraludon, Falinks, Silicobra, Galarian Yamask, Galarian Stunfisk by the canyon's water |

Galarian Darumaka is an Ice type, so it lives in Four Island's snow rather than
Mt. Ember.

Galar has few Ice species of its own, and most of them are rewards. Icefall
Cave is therefore the one dungeon where rewards such as Snom, Eiscue,
Galarian Darumaka and Galarian Mr. Mime may hold common slots. They are
prowlers and take their minimum levels there.

### The blend

A few species from other included generations may appear in Sevii where they
make geographic or thematic sense. They add variety without crowding out
Galar:

- **They must fit the place.** For example: Kanto's sea species, such as
  Tentacool, Horsea, Krabby and Magikarp, drifting into the near islands'
  waters from nearby Kanto; Unown in the Tanoby Ruins; Johto's bugs in Pattern
  Bush; and Koffing in Mt. Ember's coal country, where it evolves into Galarian
  Weezing.
- **Kanto's drift reaches only the near islands.** Magikarp appears only on
  One, Two and Three Islands. On the outer islands the blend is mostly Unown in
  the Tanoby Chambers, Johto's bugs in Pattern Bush, a few ghosts in Lost Cave,
  and a few snow species in Icefall Cave.
- **They stay a minority:** at most ~20% of slot weight on the near islands and
  ~10% on the outer islands, measured per island group across day and night.
  The Kanto drift at sea fades from the near islands outward.
- **They never lead a table.** A blend species is never one of a table's two
  most common slots.
- **They never outshine Galar.** Blend species are everyday species or harmless
  finds, never fierce or dangerous rewards.
- **They don't replace a home.** Every blend species is already catchable in its
  own region.
- **A slot capped at a Galarian form is native.** Koffing–Galarian Weezing
  produces the Galarian form, so it may lead a table and doesn't count toward
  the blend cap.

**Icefall Cave's Alolan forms.** Icefall Cave also holds Alolan Sandshrew,
Sandslash, Vulpix and Ninetales as part of its snow blend. This is an exception
to "regional forms live in their region": Galar's Crown Tundra hosts Alolan
Vulpix and Sandshrew in the source games.

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
  outward, dangerous only in Outlands and on a dungeon's
  [deeper floors](reach-assignments.md#dungeon-floors).
- Stages reached by a non-level evolution (item, trade, friendship, move or
  location) appear only in Outlands and on a dungeon's
  [deeper floors](reach-assignments.md#dungeon-floors).
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
  families for Sevii's surfing and fishing tables. Water follows the surfing
  limit under Water instead. Sea maps differ through their mix, their rarities
  and the blend.

### Water

Every map with surfing or fishing has one water type. The Sevii table spec names
each map's type.

| Water type | Where | Cast |
| --- | --- | --- |
| Ponds and rivers | Cape Brink, Berry Forest, Ruin Valley | Chewtle, Arrokuda, Galarian Stunfisk, Galarian Slowpoke, Sobble, and the blend's fresh-water species: Magikarp, Psyduck, Poliwag, Goldeen, Basculin, Marill, Wooper, Lotad, Dewpider |
| Coast and sea | Every other island coast and sea route | Chewtle, Arrokuda, Pincurchin, Clobbopus, Cramorant, Galarian Corsola, Galarian Slowpoke, Sobble, and the blend's sea species: Magikarp, Tentacool, Horsea, Krabby, Shellder, Staryu, Chinchou, Wingull, Wailmer, Remoraid, Qwilfish, Luvdisc, Frillish, Mareanie, Pyukumuku |
| Cold water | Four Island and Icefall Cave | Eiscue, Arrokuda, Clobbopus, and the blend's cold species: Seel, Shellder, Spheal, Horsea, Tentacool |

- **Fishing:** entries 1 and 2 hold Galar natives, since blend species never
  lead a table. Entries 3–10 hold at least four different families.
- **Surfing:** no family holds a surfing table's first slot on more than
  twelve maps.
- **Night:** at sea, night brings Chinchou near home and Cursola, Galarian
  Corsola's ghostly evolution, further out.

### Dungeons

- **Mt. Ember:** Rolycoly, Sizzlipede, Cufant and Koffing's Galarian Weezing
  fill the tunnels, and its Rock Smash rocks hide Rolycoly and Sizzlipede.
  Stonjourner, which watches the sunset from open ground, appears only on the
  outer slopes.
- **Lost Cave:** graded through its 14 rooms. Impidimp, Nickit and Galarian
  Corsola lead near the entrance, Sinistea and Galarian Yamask rise through the
  middle rooms, and Dreepy's line leads every deep room.
- **Tanoby Chambers:** a stone, ghost and ancient core. Runerigus guards every
  chamber with Silicobra or Sinistea, Unown drifts in from the ruins, and each
  chamber has one accent of its own.

### Day and night

Sevii's night tables are copies of its day tables today, so every night table
is new.

- Every map has its own night table.
- Outdoors, at least 30% of a land table's night weight goes to species that
  don't appear in that map's day table, such as Nickit, Impidimp, Sinistea and
  Dreepy.
- Caves, buildings and surfing change more lightly. Their day and night tables may
  overlap, but each has at least one species that appears only at night or is
  much more common then.
- Fishing and Rock Smash have night tables too, with the same light difference
  as caves.

### Native HM sources

The native HM guarantee covers Sevii's field moves: Surf, Cut, Rock Smash,
Strength and Waterfall, the ones its maps need. The
[catch-window audit's v2 revision](../research/native-hm-windows/revisions/wild-encounters-v2/README.md) checks them across TR 0–160, and checks
that every island offers a Surf carrier within walking distance of its harbor
from two badges on. Galar's Arrokuda–Barraskewda and Chewtle–Drednaw carry
Surf, Chewtle carries Waterfall, and Weepinbell carries Cut.

## Open questions

- The values for the blend caps, the repetition caps and the night share.
