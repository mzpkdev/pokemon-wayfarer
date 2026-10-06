# Safari Zones and the Bug-Catching Contest

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: v0 approved. It sets the rules for the three Safari Zones and the
Bug-Catching Contest. The tables themselves are in the
[Safari table spec](safari-encounter-tables.md). Every count below is a
placeholder for playtesting.

## Scope

This spec defines:

- the Kalos reserve, and how Kalos splits across the Kanto, Johto and Hoenn
  Safari Zones;
- which Vivillon pattern, Flabébé colour and Pumpkaboo size lives where;
- the rules Safari tables follow on temperament, stages, water and night;
- the Bug-Catching Contest's three contest days.

It doesn't cover:

- the per-map tables, which belong to the Safari table spec;
- the Safari Zones' story events, such as the Kanto Safari's Surf and Strength
  quests;
- Kalos's legendaries, mythicals and fossils (Tyrunt and Amaura), which belong
  to later specs;
- Safari mode itself: entry fees, step limits, bait and rocks stay as they are.

## Behavior

### The Kalos reserve

Generation VI (Kalos) has no region of its own. It lives only in the three
Safari Zones, apart from three Vivillon patterns that belong to the
Bug-Catching Contest, and each Safari Zone holds its own third of it. Completing
Kalos means visiting all three, which gives players a reason to travel between
regions.

- **Each Safari Zone has its own land species and its own Kalos starter.**
  These appear in no other Safari Zone.
- **A few species are shared** so every Safari Zone feels Kalosian: Fletchling,
  Bunnelby and Scatterbug on land, and the water quartet in every Safari
  Zone's water.
- **The forms are regional collectibles.** Each Safari area holds its own
  Vivillon pattern as its signature common, and Flabébé's colours and
  Pumpkaboo's sizes each live in one Safari Zone.

| Safari Zone | Feels like | Its own species | Starter |
| --- | --- | --- | --- |
| Kanto (Beach, Brush, Cave, Mountain) | Coast, brush and mountain fields, and a sea cave | Litleo, Pancham, Skiddo, Furfrou, Carbink, Noibat, Bergmite | Froakie |
| Johto (six areas) | Meadows, sweets and a haunted forest | Flabébé (all five colours), Spritzee, Swirlix, Dedenne, Phantump, Espurr | Chespin |
| Hoenn (six areas) | Sun-baked plains and rocks, Goomy's wetland and Hawlucha's river jungle | Helioptile, Hawlucha, Honedge, Klefki, Pumpkaboo (all four sizes), Goomy | Fennekin |
| All three | | Fletchling, Bunnelby, Scatterbug; Binacle, Clauncher, Skrelp and Inkay in the water | |

### Forms

**Vivillon patterns.** Each outdoor Safari area holds one pattern, and each
contest day holds another. Together they cover all 18 of Vivillon's regular
patterns. The two event patterns, Fancy and Poké Ball, stay out. In the Kanto
and Hoenn Safari Zones, an area's 1% slot holds a stray pattern from a
neighbouring area.

| Place | Areas and patterns |
| --- | --- |
| Kanto Safari | Beach: Marine. Brush: Meadow. Mountain: Icy Snow. The Cave has none |
| Johto Safari | Low left: Sandstorm. Low middle: High Plains. Low right: River. Top left: Savanna. Top middle: Elegant. Top right: Ocean |
| Hoenn Safari | South: Garden. Southwest: Monsoon. North: Continental. Northwest: Jungle. Southeast: Archipelago. Northeast: Sun |
| Bug-Catching Contest | Tuesday: Modern. Thursday: Polar. Saturday: Tundra |

**Flabébé colours** live in the Johto Safari, one per area: red in the top
left, yellow in the low middle, blue in the low right, white in the top middle
and orange in the top right. Each Johto area's 1% slot holds a stray colour from
a neighbouring area, so the low left, which has no colour of its own, has a
yellow stray.

**Pumpkaboo sizes** live in the Hoenn Safari at night: small in the southwest,
northwest and north, average in the north, southeast and northeast, and large in
the south and northeast. Super size is a 1% find in the south, northwest,
southeast and northeast.

**Meowstic** comes from Espurr, which the tables cap at Espurr, so its gender
decides which Meowstic it becomes.

### Rewards are residents

Gen VI's reward species are the reserve's residents, not prowlers:

- **They have no minimum level** inside a Safari Zone, so they appear at the
  Wilds level like their neighbours.
- **They never lead a table** and take at most 10% of any one table. Each area
  keeps one or two as its showpiece, such as Goomy in the southwest wetland,
  Carbink in the Kanto cave or Hawlucha in the river jungle, while ordinary
  species hold the common slots.

### Temperament and stages

- **Any temperament may appear in a Safari Zone.** Safari mode has no battles to
  lose, so Goomy, a dangerous species, can live there although the Safari Zones
  are Wilds.
- **Stages follow the Wilds rules.** A slot is capped before any stage reached
  by a non-level evolution, so Florges, Aromatisse, Slurpuff, Trevenant,
  Gourgeist, Aegislash and Heliolisk come from evolving.
- Levels follow the Wilds reach, as the PRD defines.

### Water

Kalos has only four water families, so the Safari Zones share them: Binacle,
Clauncher, Skrelp and Inkay. Each area's water mixes them in its own order,
and Inkay rises at night. Froakie, the Kanto Safari's starter, takes the rare
slot in the Mountain's pond, and Bergmite floats on the Kanto Safari's cave
water.

### Trees and rocks

The Johto Safari's Headbutt trees hold Fletchling, Spewpa in the area's pattern,
Dedenne and Phantump, with Chespin as a rare find. The Hoenn Safari's Rock Smash
rocks hide Helioptile and Bunnelby, with Honedge, Klefki and Hawlucha as rarer
finds. At night, when the day-only species rest, Espurr and Swirlix take the
trees' Spewpa and Fletchling slots, and Pumpkaboo takes the rocks'.

### Day and night

- Every Safari area has its own night table.
- Outdoors, at least 30% of a land table's night weight goes to species that
  don't appear in that map's day table, such as Noibat, Inkay, Pancham, Espurr,
  Pumpkaboo and Honedge wherever they are absent by day.
- Day species rest at night: Fletchling, Helioptile and Hawlucha appear only by
  day outdoors, and Scatterbug's patterns only by day.
- Surfing, Headbutt trees, Rock Smash rocks and the Kanto Safari's Cave
  change more lightly, as caves do.

### The Bug-Catching Contest

The contest in National Park is held only on Tuesday, Thursday and Saturday, as
in the original games. On other days, the gate attendant says it isn't on. Each
contest day shows a different set of generations:

| Contest day | Generations | Bugs |
| --- | --- | --- |
| Tuesday | I and II | Caterpie, Weedle, Paras, Venonat, Ledyba, Spinarak, Yanma, Pineco, Scyther, Pinsir, Heracross |
| Thursday | III and V | Wurmple, Nincada, Surskit, Volbeat, Illumise, Sewaddle, Venipede, Dwebble, Karrablast, Shelmet, Joltik, and Durant as the day's showpiece |
| Saturday | VII and VIII | Grubbin, Cutiefly, Dewpider, Blipbug, Sizzlipede, Snom |

Each day also has its own Vivillon pattern, as listed under Forms. These three
patterns are found only in the contest.

- **The contest is exempt from Johto's natives.** It is a showcase, so its bugs
  come from every included generation.
- **It isn't any species' only source.** Every contest species is also
  catchable in the wild in its own region. Only the three Vivillon patterns are
  contest-only.
- **Temperament and stages follow the Wilds rules,** since the contest has
  battles.

### Implementation notes

- **The Safari tables replace the current Safari Zone tables.** The old
  FireRed and LeafGreen Safari Zone maps aren't reachable in Wayfarer, so they
  get no tables.
- **The contest needs three day tables and a weekday check.** The engine has
  one table for the contest map today, and the gate runs the contest every day,
  although its dialogue names Tuesday, Thursday and Saturday. The
  implementation must hold the contest only on those days and choose the table
  by day.

## Open questions

- Whether the three Safari Zones should acknowledge the shared Kalos goal, for
  example through a conservation programme in the story.
