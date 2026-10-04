# Kanto and Johto encounters

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. It sets the rules and targets for Kanto's and Johto's
encounter tables. The tables themselves are in the
[Kanto](kanto-encounter-tables.md) and [Johto](johto-encounter-tables.md)
table specs. Every share and count below is a placeholder for playtesting.

## Scope

This spec defines how Kanto and Johto tables are built:

- the table format;
- how Generations I and II are spread between the two regions;
- what must be catchable in each region;
- nostalgia anchors, crossovers, and what every route offers;
- the rules for habitat, reach, rewards, babies, variety, day and night,
  fishing, and Headbutt trees and Rock Smash rocks.

It doesn't cover:

- the per-map tables, which belong to the Kanto and Johto table specs;
- the Safari Zones, which are deferred;
- legendaries and mythicals, which belong to a later spec;
- reach values, which belong to [Reach assignments](reach-assignments.md).

## Behavior

### Table format

Each map has a table for each of its encounter methods, and each method has a
day table and a night table. The methods are:

- **Land:** tall grass, caves and floors.
- **Surfing.**
- **Fishing:** the [Standard Rod](standard-rod-fishing.md)'s ten entries and
  three qualities.
- **Trees and rocks:** one shared table for Headbutt trees and Rock Smash
  rocks. In the HNS maps both use the same encounter.

Each slot holds:

- a species line, written as its **stage cap**, the highest stage allowed;
- a **rarity weight**.

Slots hold no levels. Levels come from the map's reach, and the stage comes
from the level through the downward rule and the stage mix.

The tables are designed fresh. The old tables are only a reference for what
lived where.

### The Generation I–II gradient

Kanto and Johto share Generations I and II. Johto lies west and Kanto east,
and they meet where Johto's eastern edge touches Kanto's western edge. Gen II's
share of encounter slots falls the further east you travel:

| Band | Areas | Gen II share |
| --- | --- | --- |
| Johto west | Cianwood, Olivine, Ecruteak, Goldenrod, Azalea, Routes 34–41 and 47–48, Safari Zone Gate, Ilex Forest, National Park, Slowpoke Well, Cliff Edge Cave, Whirl Islands, Tin Tower, Burned Tower | ~50% |
| Johto east | Violet, Cherrygrove, New Bark, Mahogany, Blackthorn, Routes 29–33 and 42–46, Lake of Rage, Ruins of Alph, Union Cave, Dark Cave, Ice Path, Mt. Mortar, Dragon's Den, Sprout Tower, Rocket Hideout | ~40% |
| Border | Routes 22, 23 and 26–28, Tohjo Falls, Victory Road, Mt. Silver | ~30% |
| Kanto west | Pallet, Viridian, Pewter, Cinnabar, Routes 1–4, 20 and 21, Viridian Forest, Mt. Moon, Diglett's Cave, Seafoam Islands, Pokémon Mansion | ~20% |
| Kanto east | Cerulean, Saffron, Celadon, Vermilion, Lavender, Fuchsia, Routes 5–19 and 24–25, Rock Tunnel, Power Plant, Pokémon Tower, Cerulean Cave | ~10–15% |

A share is measured across all of a band's tables, day and night together,
by slot weight. A family counts as the generation of its own base form, so
Zubat's line counts as Gen I even where it reaches Crobat.

### What must be catchable

- **Every Generation I family can be caught in Kanto.** Its lowest
  Generation I stage appears somewhere in Kanto's tables. Pikachu counts for
  its line, and Pichu isn't needed in Kanto.
- **Every Generation II family can be caught in Johto.** Its lowest
  Generation II stage appears somewhere in Johto's tables. This includes the
  Generation II babies, which appear as rare finds.
- **Generation II species from Generation I lines need that line in Johto.**
  Crobat, Bellossom, Politoed, Slowking, Steelix, Scizor, Kingdra, Porygon2,
  Blissey, Espeon and Umbreon each need their line's base catchable in Johto.
- A baby slot also covers the stage it evolves into. Chansey stays in Kanto as a
  Safari species, so Happiny in Johto covers Blissey's line there.
- A later stage reached by level, stone, trade or friendship counts as
  catchable when its base form is.
- A species may also live in the other region. The guarantee only says where
  it must live.

Other sources count where the PRD says so:

- **Starters** are harmless rewards: Kanto's three in Kanto and Johto's three
  in Johto.
- **Omanyte and Kabuto** come from fossils in Kanto.
- **Legendaries and mythicals** are left to a later spec.

The Kanto and Johto table specs each end with a coverage checklist that shows
where every family is catchable.

### Nostalgia anchors

Each region keeps its classic highlights in their original spots, so players
of the original games find them where they remember. FireRed, LeafGreen,
HeartGold and SoulSilver are the reference. Anchors include:

| Region | Anchors |
| --- | --- |
| Kanto | Pidgey and Rattata on Route 1, Pikachu in Viridian Forest, Clefairy in Mt. Moon, Onix in Rock Tunnel, Diglett in Diglett's Cave, Magnemite and Voltorb in the Power Plant, Gastly and Cubone in the Pokémon Tower, Grimer and Koffing in the Pokémon Mansion, Seel and Jynx in Seafoam Islands, Ditto in Cerulean Cave |
| Johto | Sentret on Route 29, Mareep and Wooper on Route 32, Wooper in Union Cave, Lapras in Union Cave's depths, Slowpoke in Slowpoke Well, Natu and Unown at the Ruins of Alph, Sneasel at night in Ice Path, Dratini in Dragon's Den, Aipom and Heracross in Headbutt trees |

An anchor holds one of its table's common or uncommon slots, unless it was a
rarity in the original games, such as Pikachu or Clefairy, where it stays rare
but present.

Reach rules come first. A fierce or dangerous reward stays off Roads even where
it lived in the original games, so Stantler, Miltank, Tauros, Girafarig,
Skarmory, Shuckle and Mantine move from their old Road spots to nearby Wilds,
Outlands or dungeons.

### Crossovers

- **A crossover follows the remakes first.** A Generation II species in Kanto,
  or a Generation I species in Johto, appears where HeartGold and SoulSilver
  put it. Sentret by day and Hoothoot at night on Kanto's Route 1 are examples.
- **Otherwise it must fit the place thematically,** such as forest bugs in a
  forest.
- **Signature species stay at home.** A region's signature species don't cross
  over: for Johto, for example, Larvitar, Mareep, Hoppip, Stantler, Mantine and
  Aipom; for Kanto, Safari Zone species such as Kangaskhan and Chansey.
- **Crossovers stay light.** A crossover family appears on only a few maps of
  the other region, near the border or where the remakes put it. Wooper's line,
  for example, appears on at most four Kanto maps.

### Classic core, more around it

Every route is worth a trip on its own in this open world, including the first
routes:

- A route's common slots keep its classic cast, so Route 1 still feels like
  Route 1.
- Around that core, every route offers more than it did in the original games:
  fitting uncommon species, a real night table and at least one reward slot.
- The extra species fit the route and follow the crossover rules, so a route
  gains variety without losing its identity.

### Habitat

Species live where they belong: Zubat, Geodude and Onix in caves; water
species in water; Paras and Oddish in forests; birds and bugs in Headbutt
trees; and Geodude and Shuckle in rocks. A map's tables hold only species that
fit what the map has.

### Reach

- Rewards follow their temperament: harmless anywhere, fierce from Wilds
  outward, dangerous only in Outlands and dungeons.
- Stages reached by item, trade or friendship appear only in Outlands and on
  a dungeon's deeper floors. This includes later-generation extensions such as
  Electivire, Magmortar, Weavile and Togekiss.
- Deeper dungeon floors hold rarer species than their entrances.

### Rewards and babies

- Most maps hold one to three reward slots. Deep dungeon floors may hold more.
  A Road map with only surfing and fishing, such as Cinnabar Island or the sea
  lanes on Routes 40 and 41, may hold none.
- Every Generation I reward appears in Kanto, and every Generation II reward
  appears in Johto. A reward may also appear in the other region.
- Babies appear only as rare slots named directly in a table, never through
  the downward rule. They keep young levels.

### Variety

- Each map has a mix of its own, not a copy of its neighbour's.
- No family is one of a table's two most common slots on more than eight maps
  in a region, counted per method for land and for trees and rocks. Water has
  its own limits, under Water.

### Water

Every map with surfing or fishing has one water type. The table specs name each
map's type.

| Water type | Where | Cast |
| --- | --- | --- |
| Ponds and rivers | Fresh water on routes, in towns and in forests | Magikarp, Poliwag, Psyduck, Goldeen, Slowpoke, Marill, and Wooper in Johto |
| Coast and sea | Coastal towns and sea routes | Magikarp, Tentacool, Krabby, Shellder, Staryu, Horsea, Chinchou, Corsola, Qwilfish, Remoraid, Slowpoke, Psyduck, and Mantine in Johto |
| Cold water | Seafoam Islands' icy depths | Magikarp, Seel, Shellder, Horsea, Krabby, Tentacool, Slowpoke, Psyduck, Lapras |
| Cave water | Water inside caves, such as Mt. Moon, Union Cave, Cerulean Cave and the Whirl Islands | Magikarp, Zubat, Goldeen, Psyduck, Slowpoke, Poliwag, Marill, Wooper in Johto, and sea species in sea caves such as the Whirl Islands |

Rewards, babies, anchors and native HM carriers may appear outside their type's
cast where they fit, such as Lapras in Union Cave's depths, Dratini and Seadra
in Dragon's Den, and the water starters.

- **Fishing:** entries 1–3, which make up about 70% of Old Rod catches, hold
  the map's most fitting common fish. Magikarp stays common in ponds and lakes
  but isn't required anywhere. Entries 3–10 hold at least four different
  families and carry the map's own character, which the better rods reveal.
- **Native HM crossings:** where a player on shore must be able to catch a
  Pokémon that knows Surf or Whirlpool, fishing entries 1–3 include a local
  carrier by day and by night. In Kanto and Johto that means Vermilion City and
  its port, Cinnabar Island, Olivine City and its port, Cianwood City and
  Blackthorn City for Surf, and Dragon's Den for Whirlpool. The catch-window
  audit still decides whether each carrier knows the move at its level.
- **Surfing:** no family holds a surfing table's first slot on more than eight
  maps in a region, so sea routes don't all lead with Tentacool.
- **Night:** at sea, night brings its own species, such as Chinchou, Staryu
  and Qwilfish.
- **Kanto east:** Generation II sea species that HeartGold and SoulSilver put in
  Kanto, such as Qwilfish, Remoraid and Chinchou, refresh the eastern sea and
  bring Kanto east's Gen II share towards ~10–15%.

### Day and night

- Every map has its own night table.
- Outdoors, at least 30% of a night table's slot weight goes to species that
  don't appear in that map's day table, such as Hoothoot, Gastly, Murkrow,
  Houndour and Oddish.
- Caves and buildings change more lightly. Their day and night tables may
  overlap, but each has at least one species that appears only at night or is
  much more common then.
- Fishing and trees and rocks have night tables too, with the same light
  difference as caves.

### Native HM sources

The native HM catch-window carriers stay available where those docs place
them until the catch-window audit is re-run.

## Open questions

- The values for the Gen II shares, the family repetition cap and the night
  share.
- How rare a reward slot or a baby slot is.
