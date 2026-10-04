# Wild encounters v2

Implemented: No

Specifications: [Prowlers](../specs/prowlers.md), [Reach assignments](../specs/reach-assignments.md)

Design status: sketch. It supersedes the authored-level and frozen-table parts
of the [Kanto](kanto-wild-encounters.md), [Johto](johto-wild-encounters.md),
[Trainer Rating scaling](trainer-rating-wild-encounter-scaling.md) and
[Standard Rod](standard-rod-fishing.md) PRDs once accepted. Terms follow the
[glossary](player-trainer-rating.md#glossary) plus the terms below. All
numbers are placeholders for playtesting.

## Intent

Today, wild levels come from the original games' tables, squeezed by a global
Trainer Rating curve. Danger reflects each game's old story order, not the
place itself. Late in the game every area converges to the same level, and
early routes offer little worth the trip.

V2 makes danger something players read from geography, and gives every region
its own natives. Roads are safe and remote places are dangerous. Each region is
home to its own generations, so travelling between regions means meeting
different Pokémon.

## Terms

- **Reach:** the kind of place a map is, which sets how dangerous its wild
  Pokémon are. One of Road, Wilds, Outlands or Dungeon.
- **Reach bonus:** levels an outdoor reach adds on top of the wild level curve.
- **Dungeon:** a named group of maps you go into to explore, such as a cave,
  tower or mansion. It sets its own difficulty.
- **Start / end:** the level bonus on a dungeon's first and deepest floors.
- **Floor:** an optional minimum wild level for a dungeon, regardless of TR.
- **Stage cap:** the highest evolution stage a table slot allows.
- **Stage mix:** the rolled mix of evolution stages near an evolution level, so
  young and grown Pokémon appear together.
- **Prowler:** a rare reward species, a special find. It never appears below
  its own minimum level, so early on it can be fiercer than its surroundings.
- **Temperament:** how threatening a prowler is: harmless, fierce or
  dangerous. It decides which reaches the prowler can appear in.
- **Natives:** the generations a region is home to.
- **Reserve:** a Safari Zone that hosts a generation with no region of its own.
- **Lair:** a remote place that is home to a single legendary or mythical.

## Design

### Reach

Every encounter map has one reach. Players never see reach names. They read
danger from the world: on the road, off the road, far from home, or inside
somewhere.

| Reach | Meaning | Examples |
| --- | --- | --- |
| Road | The way between places | Routes, towns, sea lanes, forests you pass through |
| Wilds | Off the road | Side forests, parks, Safari Zones, open sea near land, caves you pass through |
| Outlands | Outdoors, far from home | The far Hoenn sea, the outer Sevii islands, the approaches to a region's edge |
| Dungeon | A place you go into to explore | Pokémon Tower, Seafoam Islands, Cerulean Cave, Mt. Silver |

**Settlement rule:** any map with a town is Road.

**One safe road:** every town is reachable by at least one path of Road maps.
Other routes take the reach that fits their character, so the long, rugged or
remote way around is the risky choice.

Road, Wilds and Outlands each have a fixed reach bonus, with no per-map numbers.
The starting placeholders are Road +0, Wilds +5 and Outlands +10 levels.
Each also has one distinct mechanic:

- **Road: fewer encounters.** A lower encounter rate keeps travel quick.
- **Wilds: fierce prowlers.** Off the road, rare finds can be stronger than
  their surroundings, but you can still get away.
- **Outlands: no running.** You can't run from wild battles. Escape items
  still work, so preparing for the trip matters. Losing after wandering in
  unprepared is an expected outcome.

Each dungeon instead sets its own difficulty:

- **Start and end:** the bonus on its first and deepest floors, for example
  "Seafoam Islands: +5 → +12". Floors in between climb evenly. Dungeons differ
  freely: a tower in a town stays mild and a cave at the edge of the world
  starts high and ends brutal. Start equal to end gives a flat dungeon.
- **Floor:** optional, for endgame dungeons, so they are clearly too dangerous
  early on.

A dungeon includes its outdoor parts where they belong to the same place, such
as Mt. Silver's slopes. Caves are never Road: a cave you only pass through is
Wilds, and one with floors beyond the path is a dungeon.

How bonuses change over the course of the game is left to later scaling work.

### Encounter tables

- Tables hold species, rarity and day/night variants. **They hold no levels.**
  Every region has day and night encounters.
- **Each slot has a stage cap.** The level picks the stage through the
  [downward rule](player-trainer-rating.md#glossary) and the stage mix. A slot
  capped at an early stage keeps young levels.
- **Item, trade, friendship and choice evolutions** are capped at the stage
  before, except in Outlands and on a dungeon's deeper floors, where tables may
  allow them. Iconic early stages are capped deliberately.
- **Nothing is locked by progress.** Every species is available from the start.
- **Babies are rare finds.** The downward rule never steps into a baby, so a
  low-level Pikachu slot stays Pikachu. Instead, a table can name a baby as a
  rare slot of its own, such as Riolu in Sinjoh or Azurill in Hoenn. Like any
  slot capped at an early stage, it keeps young levels.
- **Prowlers** appear in every reach, sorted by temperament: harmless ones
  anywhere, including Roads; fierce ones from Wilds outward; dangerous ones
  only in Outlands and dungeons. See the [prowlers spec](../specs/prowlers.md).

### Natives

Each included generation is complete. Later evolutions and babies come with
the lines they extend. A family belongs to the region of any of its members,
and regional forms live in their region.

| Generations | Home |
| --- | --- |
| I + II | Kanto and Johto |
| III + V | Hoenn, with no Gen I–II wild species. Gen V grows from west to east |
| IV + the Hisuian part of VIII | Sinjoh |
| VII | Alola |
| VIII (Galar) | Sevii |
| VI (Kalos) | Deferred: possibly a Safari Zone reserve |
| IX | Not included |

**Hoenn's west-to-east gradient.** Gen III is Hoenn's identity, and Gen V's
share of encounter slots rises the further east you travel. Gen V replaces the
Gen I–II species Hoenn drops. Day and night tables follow each species'
behaviour, not its generation. The bands below are placeholders:

| Band | Areas | Gen V share |
| --- | --- | --- |
| West | Littleroot to Rustboro, Dewford, Slateport, Routes 101–109, Petalburg Woods, Granite Cave | ~10–20% |
| Centre | Mauville, Verdanturf, Fallarbor, Lavaridge, Routes 110–118, the desert, Mt. Chimney, Fiery Path, Meteor Falls, Rusturf Tunnel | ~30–40% |
| East | Fortree, Lilycove, Routes 119–123, Mt. Pyre, the Safari Zone | ~40–50% |
| Far east | Mossdeep, Sootopolis, Pacifidlog, Ever Grande, Routes 124–134, Shoal Cave, Seafloor Cavern, Sky Pillar, Cave of Origin, Victory Road | ~50–60% |

Gen III never disappears. Its sea natives still belong in the far east, so it
stays at least a large minority there. Bands follow travel distance from the
western start rather than strict longitude.

### Legendaries

Most legendaries roam. A few stay static behind longer story arcs or puzzles.
Lairs such as Faraway Island, Southern Island, Birth Island, Navel Rock,
Cinnabar's volcano and Seafoam's depths each host a single legendary or
mythical.

## Boundaries

- Covers ordinary wild encounters only. Trainer battles, gifts and scripted
  encounters are separate.
- Trainer Rating, the level cap and obedience keep their current roles.
- Natives must fit existing encounter tables. Porymap edits are deferred.
- Which legendaries roam, and how many roam at once, belong to a later spec.
- How every included species becomes obtainable, including starters,
  mythicals and evolutions by trade or special condition, belongs to later
  specs.
- Wild Pokémon keep whatever abilities they have. There is no special ability
  policy.

## Open questions

- Whether Sinjoh hosts Gen IV, or keeps only its Hisuian species if a Sinnoh
  region arrives later.
- Final values for each reach bonus and dungeon, and the shape of the stage
  mix. The current numbers are placeholders for playtesting.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Native HM catch windows](native-hm-catch-windows.md), which must be
  re-audited for new tables
- [Authored under-level wild encounters](../research/authored-under-level-wild-encounters.md)
