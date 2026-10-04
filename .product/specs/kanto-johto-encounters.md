# Kanto and Johto encounters

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. It sets the rules and targets for Kanto's and Johto's
encounter tables. The tables themselves follow in a Kanto spec and a Johto
spec. Every share and count below is a placeholder for playtesting.

## Scope

This spec defines how Kanto and Johto tables are built:

- the table format;
- how Generations I and II are spread between the two regions;
- what must be catchable in each region;
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
- Every Generation I reward appears in Kanto, and every Generation II reward
  appears in Johto. A reward may also appear in the other region.
- Babies appear only as rare slots named directly in a table, never through
  the downward rule. They keep young levels.

### Variety

- Each map has a mix of its own, not a copy of its neighbour's.
- No family is one of a table's two most common slots on more than eight maps
  in a region.

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
