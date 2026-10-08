# World items

PRD: [Daily world slots](../prds/daily-world-slots.md#item-slots)
Implemented: No

Design status: draft. Every weight and odds value is a placeholder for
playtesting.

## Scope

This spec authors Wayfarer's overworld item spots: item balls and hidden items.
It defines the dynamic pools and lists every region's static prizes and trails,
Sevii included as if its item port were already done.

It doesn't cover the daily roll itself, the save state for cleared spots or
the map-load hook. Those belong to the daily world slots spec. Berry trees, gift
NPCs, Poké Marts and Pickup are unchanged.

## Reach of an item spot

A spot uses its map's reach from [reach assignments](reach-assignments.md). A
dungeon's entrance floor counts as Wilds and every deeper floor as Outlands. A
map without a reach of its own resolves one with the
[places without wild encounters](reach-assignments.md#places-without-wild-encounters)
rules, the same resolution trainers use
([trainer reach levels](trainer-party-scaling.md#maps-without-a-reach-of-their-own)).

## Dynamic pools

### Tier odds

A dynamic find first rolls a tier from its spot's reach, then an item within
the tier by weight. An empty day is rolled before this, at the flat 25% from
the PRD.

| Reach | Regular | Better | Special |
| --- | ---: | ---: | ---: |
| Road | 70% | 25% | 5% |
| Wilds, dungeon entrance | 50% | 38% | 12% |
| Outlands, deeper dungeon floor | 30% | 45% | 25% |

Odds never depend on Trainer Rating, badges or the day.

### Regular

| Item | Weight |
| --- | ---: |
| `ITEM_POTION` | 10 |
| `ITEM_SUPER_POTION` | 12 |
| `ITEM_POKE_BALL` | 10 |
| `ITEM_GREAT_BALL` | 10 |
| `ITEM_ANTIDOTE` | 5 |
| `ITEM_PARALYZE_HEAL` | 5 |
| `ITEM_AWAKENING` | 4 |
| `ITEM_BURN_HEAL` | 4 |
| `ITEM_ICE_HEAL` | 4 |
| `ITEM_REPEL` | 6 |
| `ITEM_SUPER_REPEL` | 5 |
| `ITEM_ESCAPE_ROPE` | 5 |
| `ITEM_ORAN_BERRY` | 6 |
| `ITEM_FRESH_WATER` | 5 |
| `ITEM_SODA_POP` | 4 |
| `ITEM_TINY_MUSHROOM` | 3 |
| `ITEM_EXP_CANDY_S` | 2 |

### Better

| Item | Weight |
| --- | ---: |
| `ITEM_HYPER_POTION` | 12 |
| `ITEM_MAX_POTION` | 6 |
| `ITEM_FULL_HEAL` | 8 |
| `ITEM_REVIVE` | 10 |
| `ITEM_ULTRA_BALL` | 10 |
| `ITEM_ETHER` | 6 |
| `ITEM_ELIXIR` | 4 |
| `ITEM_MAX_REPEL` | 5 |
| `ITEM_SITRUS_BERRY` | 6 |
| `ITEM_LUM_BERRY` | 4 |
| `ITEM_LEPPA_BERRY` | 3 |
| `ITEM_MOOMOO_MILK` | 4 |
| `ITEM_X_ATTACK` | 2 |
| `ITEM_X_DEFENSE` | 2 |
| `ITEM_X_SPEED` | 2 |
| `ITEM_X_SP_ATK` | 2 |
| `ITEM_DIVE_BALL` | 2 |
| `ITEM_NET_BALL` | 2 |
| `ITEM_QUICK_BALL` | 2 |
| `ITEM_DUSK_BALL` | 2 |
| `ITEM_TIMER_BALL` | 2 |
| `ITEM_BIG_MUSHROOM` | 2 |
| `ITEM_PEARL` | 3 |
| `ITEM_STARDUST` | 3 |
| `ITEM_EXP_CANDY_M` | 3 |

### Special

| Item | Weight |
| --- | ---: |
| `ITEM_RARE_CANDY` | 6 |
| `ITEM_PP_UP` | 5 |
| `ITEM_PP_MAX` | 1 |
| `ITEM_HP_UP` | 2 |
| `ITEM_PROTEIN` | 2 |
| `ITEM_IRON` | 2 |
| `ITEM_CALCIUM` | 2 |
| `ITEM_ZINC` | 2 |
| `ITEM_CARBOS` | 2 |
| `ITEM_FULL_RESTORE` | 5 |
| `ITEM_MAX_REVIVE` | 5 |
| `ITEM_MAX_ELIXIR` | 3 |
| `ITEM_NUGGET` | 6 |
| `ITEM_BIG_NUGGET` | 1 |
| `ITEM_STAR_PIECE` | 3 |
| `ITEM_BIG_PEARL` | 3 |
| `ITEM_COMET_SHARD` | 1 |
| `ITEM_HEART_SCALE` | 4 |
| `ITEM_BOTTLE_CAP` | 1 |
| `ITEM_EXP_CANDY_L` | 2 |
| Evolution stone | 6 |
| TM | 3 |

- **Evolution stone:** one of `ITEM_FIRE_STONE`, `ITEM_WATER_STONE`,
  `ITEM_THUNDER_STONE`, `ITEM_LEAF_STONE`, `ITEM_MOON_STONE`, `ITEM_SUN_STONE`,
  `ITEM_ICE_STONE`, `ITEM_SHINY_STONE`, `ITEM_DUSK_STONE` and
  `ITEM_DAWN_STONE`, equally likely.
- **TM:** any TM in the game except those used as a Legend prize anywhere,
  equally likely. TMs are single-use, so this keeps them renewable but rare.

### Regional flavour

Optional. A region may add these to its pools on top of the shared tables,
with the listed weights:

| Region | Tier | Items |
| --- | --- | --- |
| Johto | Regular | Each Apricorn (`ITEM_RED_APRICORN` and the other colours), weight 2 |
| Hoenn | Better | `ITEM_RED_SHARD`, `ITEM_BLUE_SHARD`, `ITEM_YELLOW_SHARD`, `ITEM_GREEN_SHARD`, weight 2 each; `ITEM_HEART_SCALE` moves up to Better at weight 3 |
| Sevii | Better | `ITEM_PEARL` weight +3, `ITEM_BIG_PEARL` moves up to Better at weight 2 |
| Kanto | Regular | `ITEM_MOOMOO_MILK` moves down to Regular at weight 4 |

## Static prizes

Static prizes are items found exactly once per save. Each one is placed in an existing item spot (ball or hidden) in Wilds, Outlands or a dungeon, or hidden on a Road. A visible ball on an open Road is never a prize. Prizes follow a ladder matched to danger: **Find** (Wilds, dungeon entrances, hidden Road spots: solid TMs, type boosters, utility and evolution items), **Treasure** (Outlands, deep dungeon floors: strong TMs, Leftovers, Life Orb, Choice items, Focus Sash, Assault Vest, rare evolution items) and **Legend** (the deepest, most hidden spots in a region). Spots are named by map directory and either the object's local id with its (x,y), or the hidden item's x,y. Rules that apply across all regions:

- **Legends are unique game-wide.** Each Legend item appears exactly once among all static prizes, at any tier. Story rewards are separate: Elm's and Silph's Master Balls and Viridian Gym's TM Earthquake stay as they are.
- **Treasures and Finds may repeat across regions** (so each region can offer, say, a Choice item) but never twice within one region.
- **No static prize is a TM that a mart sells.** The mart-sold list is: Avalanche, Blizzard, Brine, Captivate, Dark Pulse, Double Team, Dream Eater, Endure, Fire Blast, Flamethrower, Focus Blast, Frustration, Giga Impact, Gyro Ball, Hyper Beam, Ice Beam, Light Screen, Natural Gift, Protect, Psychic, Reflect, Rest, Return, Safeguard, Solar Beam, Stealth Rock, Thunder, Thunderbolt.
- A spot that loses its authored item becomes a dynamic spot (daily pool). A prize may be moved to any existing spot in its region, and a dynamic spot may be promoted to a prize spot.
- Sevii spots assume the [Sevii exploration restoration](sevii-exploration-restoration.md) is done (the current Wayfarer build drops those balls and hidden items). A Sevii spot is identified by its exploration row id (`exploration.<map>.object.N` for a ball, `exploration.<map>.bg.N` for a hidden item), where N is the FRLG source index, or by coordinates. The "local id" in the Sevii tables is that FRLG source index; the generator maps it to the compiled local id after the restoration.

- The S.S. Tidal and the Battle Frontier open on the global game-clear flag,
  which Wayfarer sets on the first Indigo League win, not Hoenn's. Their
  prizes are endgame Treasures.

### Legends

| Region | Place | Item |
|---|---|---|
| Kanto | Seafoam Islands B4F, beside the Articuno chamber | Ability Capsule |
| Kanto | Cerulean Cave B2F, hidden corner | Ability Patch |
| Sevii | Five Island, Lost Cave room 14 (end of the maze) | Lucky Egg |
| Sevii | Six Island, Outcast Island (hidden) | Amulet Coin |
| Johto | Mt. Silver Snow, Red's mountain (hidden) | Gold Bottle Cap |
| Johto | Dragon's Den cavern (hidden) | Exp. Charm |
| Johto | Whirl Islands B2F, before Lugia (hidden) | Catching Charm |
| Alola | None (the region is small; its best finds are Treasures) | - |
| Sinjoh | Route 50, far corner (hidden) | Adamant Orb |
| Sinjoh | Route 50, western edge (hidden) | Lustrous Orb |
| Sinjoh | New Sinjoh Hot Springs, buried in the springs (hidden) | Griseous Orb |
| Hoenn | Aqua Hideout B1F | Master Ball |
| Hoenn | Seafloor Cavern, Room 9 | TM Earthquake |
| Hoenn | Underwater Route 126, far corner | Soul Dew |

Totals: Kanto 54 (33 Find, 19 Treasure, 2 Legend), Sevii 32 (13 Find, 17 Treasure, 2 Legend), Johto 62 (40 Find, 19 Treasure, 3 Legend), Alola 10 (5 Find, 5 Treasure, 0 Legend), Sinjoh 8 (3 Find, 2 Treasure, 3 Legend), Hoenn 44 (22 Find, 19 Treasure, 3 Legend). Overall 210 prizes.

### Kanto

#### The Weather Run

**Build identity:** Weather / bulky stall. **Start:** Pallet Town, by sea (Route 21).

Take the sea lane to Cinnabar and dig up all four weather rocks (Damp Rock on Route 21, Heat and Smooth Rock in the Mansion, Icy Rock in Seafoam). The same trail carries the stall kit: Black Sludge, Assault Vest, Eviolite, Leftovers, Light Clay for Aurora Veil. It ends beside Articuno with an Ability Capsule. It is the longest trail and the one for players who want a team that wins slowly.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route21_North_Frlg | hidden 17,42 | Hidden | Road | `ITEM_DAMP_ROCK` | Find | Opens the set of weather rocks. A hidden find on the sea lane, the first thing a Pallet-by-sea player digs up. |
| 2 | PokemonMansion_1F_Frlg | hidden 2,21 | Hidden | Dungeon: Pokemon Mansion, floor 1 of 4 | `ITEM_MAGMARIZER` | Find | Cinnabar is Magmar country; a hidden evolution item for Magby on the first Mansion floor. |
| 3 | PokemonMansion_3F_Frlg | hidden 36,13 | Hidden | Dungeon: Pokemon Mansion, floor 3 of 4 | `ITEM_SMOOTH_ROCK` | Find | Top Mansion floor, hidden in the corner; sandstorm rock (replaces a Rare Candy). |
| 4 | PokemonMansion_B1F_Frlg | local id 1 (6,21) | Visible | Dungeon: Pokemon Mansion, floor 4 of 4 | `ITEM_HEAT_ROCK` | Find | Sun rock in the Mansion basement, where the Cinnabar fires burn. |
| 5 | PokemonMansion_B1F_Frlg | hidden 35,5; moved from CeruleanCave_B2F_hns hidden 19,17 | Hidden | Dungeon: Pokemon Mansion, floor 4 of 4 | `ITEM_BLACK_SLUDGE` | Treasure | Poison-type sustain, hidden on the deepest Mansion floor. |
| 6 | Route20_Frlg | hidden 23,6 | Hidden | Outlands | `ITEM_ASSAULT_VEST` | Treasure | Hidden on open sea far from shore: the trail's bulky special wall piece (replaces Stardust). |
| 7 | SeafoamIslands_B1F_Frlg | local id 3 (19,18) | Visible | Dungeon: Seafoam Islands, floor 2 of 5 | `ITEM_ICY_ROCK` | Find | Hail rock where the ice begins (replaces a Water Stone that Goldenrod sells). |
| 8 | SeafoamIslands_B2F_Frlg | local id 3 (18,15); moved from Route9_hns hidden 44,54 | Visible | Dungeon: Seafoam Islands, floor 3 of 5 | `ITEM_LIGHT_CLAY` | Find | Hail plus screens plus Aurora Veil. Moved here from a Route 9 hidden spot so it sits on the cold chain. |
| 9 | SeafoamIslands_B3F_Frlg | hidden 5,12 | Hidden | Dungeon: Seafoam Islands, floor 4 of 5 | `ITEM_EVIOLITE` | Treasure | Hidden on a low floor: bulk for unevolved Seafoam natives (replaces a Nugget). |
| 10 | SeafoamIslands_B4F_Frlg | hidden 13,8; moved from CeladonCity_House1_hns hidden 11,8 | Hidden | Dungeon: Seafoam Islands, floor 5 of 5 | `ITEM_LEFTOVERS` | Treasure | Deepest hidden spot in a Moderate-to-hard dungeon. |
| 11 | SeafoamIslands_B4F_Frlg | local id 4 (22,19) | Visible | Dungeon: Seafoam Islands, floor 5 of 5 | `ITEM_ABILITY_CAPSULE` | Legend | Next to the Articuno chamber, the end of the trail: the talk-about reward (replaces an Ultra Ball). |

#### Thunder Road

**Build identity:** Fast special attacker. **Start:** Cerulean City, east (Route 9).

The overland trail from Cerulean through the rugged Routes 9 and 10 into the Power Plant. Flash Cannon, Wise Glasses and Charge Beam come early; the plant pays out Thunder Wave and a choice of Choice Specs or Choice Scarf. Rock Polish handles speed on the way.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route9_hns | local id 12 (32,43) | Visible | Wilds | `ITEM_TM_FLASH_CANNON` | Find | Authored TM; a rugged Wilds route earns it. |
| 2 | Route9_hns | hidden 44,54; moved from PewterCity_hns hidden 7,7 | Hidden | Wilds | `ITEM_WISE_GLASSES` | Find | Special-attack boost. Moved from a hidden spot inside Pewter City (Road) to rugged Route 9. |
| 3 | Route10_hns | local id 1 (14,74) | Visible | Wilds | `ITEM_TM_ROCK_POLISH` | Find | Authored TM; the trail's speed-control piece. |
| 4 | Route10_hns | hidden 1,12 | Hidden | Wilds | `ITEM_TM_CHARGE_BEAM` | Find | Replaces a second Rock Polish (duplicate TM). |
| 5 | PowerPlant_Frlg | local id 3 (46,37) | Visible | Dungeon: Power Plant, single floor | `ITEM_TM_THUNDER_WAVE` | Find | Replaces TM Thunder (sold in Goldenrod). Paralysis support deep in the plant. |
| 6 | PowerPlant_Frlg | hidden 29,16 | Hidden | Dungeon: Power Plant, single floor | `ITEM_CHOICE_SPECS` | Treasure | Hidden in the maze (replaces a Max Elixir): the special-attacker capstone. |
| 7 | PowerPlant_Frlg | local id 4 (45,4) | Visible | Dungeon: Power Plant, single floor | `ITEM_CHOICE_SCARF` | Treasure | Far corner by Zapdos (replaces a Thunder Stone that Goldenrod sells). |

#### The Cerulean Cave Gauntlet

**Build identity:** Physical sweeper. **Start:** Cerulean City, north (Nugget Bridge).

Start with a Muscle Band past Nugget Bridge and a Swords Dance TM at the Cape, then enter the Brutal cave. Every floor pays out more: Stone Edge, Choice Band, Expert Belt, then rare evolution items, a Life Orb and the most hidden prize in Kanto, an Ability Patch. The strongest trail and the most dangerous.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route24_hns | hidden 24,20 | Hidden | Road | `ITEM_MUSCLE_BAND` | Find | Hidden past Nugget Bridge (replaces a Pecha Berry). |
| 2 | Route25_hns | hidden 50,12 | Hidden | Wilds | `ITEM_TM_SWORDS_DANCE` | Find | The Cerulean Cape dead end is the classic hidden spot (replaces a Revive). |
| 3 | CeruleanCave_1F_hns | hidden 26,21 | Hidden | Dungeon: Cerulean Cave, floor 1 of 3 | `ITEM_TM_STONE_EDGE` | Find | Entrance floor of a Brutal dungeon (replaces Sea Incense). |
| 4 | CeruleanCave_B1F_hns | local id 11 (30,17) | Visible | Dungeon: Cerulean Cave, floor 2 of 3 | `ITEM_CHOICE_BAND` | Treasure | Mid-cave (replaces a Big Pearl). |
| 5 | CeruleanCave_B1F_hns | hidden 6,4 | Hidden | Dungeon: Cerulean Cave, floor 2 of 3 | `ITEM_EXPERT_BELT` | Treasure | Hidden in a corner of B1F (replaces Odd Incense). |
| 6 | CeruleanCave_B2F_hns | hidden 31,16 | Hidden | Dungeon: Cerulean Cave, floor 3 of 3 | `ITEM_DUSK_STONE` | Treasure | Authored; rare evolution stone (Murkrow, Misdreavus, Lampent, Doublade). |
| 7 | CeruleanCave_B2F_hns | hidden 39,7 | Hidden | Dungeon: Cerulean Cave, floor 3 of 3 | `ITEM_ELECTIRIZER` | Treasure | Authored; trade evolution item for Electabuzz. |
| 8 | CeruleanCave_B2F_hns | local id 1 (31,10) | Visible | Dungeon: Cerulean Cave, floor 3 of 3 | `ITEM_LIFE_ORB` | Treasure | Signature sweeper item on the Brutal floor (Life Orb is a Treasure, not a Legend). |
| 9 | CeruleanCave_B2F_hns | hidden 19,17 | Hidden | Dungeon: Cerulean Cave, floor 3 of 3 | `ITEM_ABILITY_PATCH` | Legend | The most hidden spot in the most dangerous map. |

#### The League Road

**Build identity:** Mixed breaker / endgame. **Start:** Viridian City, west: Route 22 and 23 to the gauntlet before the Indigo Plateau.

Victory Road is Kanto's last dungeon and the only way to the League, so its prizes are the final tune-up: a Poison TM and Scope Lens on the entry floor, X-Scissor and a Weakness Policy in the middle, and Rocky Helmet and Heavy-Duty Boots on the bottom floor. These are held items for the team that has to win several battles in a row.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | VictoryRoadKanto_1F_hns | local id 1 (23,16) | Visible | Dungeon: Victory Road, floor 1 of 3 | `ITEM_TM_POISON_JAB` | Find | Replaces TM Dark Pulse (sold in Celadon). Physical Poison coverage. |
| 2 | VictoryRoadKanto_1F_hns | hidden 11,20 | Hidden | Dungeon: Victory Road, floor 1 of 3 | `ITEM_SCOPE_LENS` | Find | Hidden in the entry floor (replaces an Ultra Ball). Crit support for the climb. |
| 3 | VictoryRoadKanto_B1F_hns | hidden 37,14 | Hidden | Dungeon: Victory Road, floor 2 of 3 | `ITEM_TM_X_SCISSOR` | Find | Hidden mid-gauntlet (replaces a PP Up). Bug coverage for physical attackers. |
| 4 | VictoryRoadKanto_B1F_hns | local id 11 (42,8) | Visible | Dungeon: Victory Road, floor 2 of 3 | `ITEM_WEAKNESS_POLICY` | Treasure | Replaces a Full Restore. Rewards a team that expects to be hit. |
| 5 | VictoryRoadKanto_B2F_hns | local id 10 (19,11) | Visible | Dungeon: Victory Road, floor 3 of 3 | `ITEM_ROCKY_HELMET` | Treasure | Replaces TM Earthquake (now Hoenn's Legend). Contact chip damage for a wall. |
| 6 | VictoryRoadKanto_B2F_hns | hidden 8,38 | Hidden | Dungeon: Victory Road, floor 3 of 3 | `ITEM_HEAVY_DUTY_BOOTS` | Treasure | Hidden on the last floor (replaces Zinc). Hazard immunity; Stealth Rock is a mart TM. |

#### Moon to Lavender

**Build identity:** Collector / evolution and status utility. **Start:** Viridian City, north (Viridian Forest).

A long walk from Viridian Forest through Mt. Moon and Rock Tunnel to Lavender Town's tower. It collects evolution stones (Leaf, Moon, Shiny, Oval, Reaper Cloth) and status tools (Will-O-Wisp, Toxic Orb) with a Focus Sash on Rock Tunnel's dark lower floor.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | ViridianForest_hns | hidden 42,13 | Hidden | Road | `ITEM_LEAF_STONE` | Find | Authored hidden Leaf Stone, the only Leaf Stone source. |
| 2 | MtMoon_Outside_hns | local id 1 (6,24) | Visible | Wilds | `ITEM_MOON_STONE` | Find | Authored; on the Mt. |
| 3 | MtMoon_Cave_hns | hidden 26,21 | Hidden | Wilds | `ITEM_SHINY_STONE` | Find | Rare evolution stone hidden in the cave (replaces a Revive). |
| 4 | RockTunnel_1F_hns | hidden 5,45 | Hidden | Wilds | `ITEM_TM_ROCK_SLIDE` | Find | Flinch coverage in a dark cave that needs Flash (replaces Iron). |
| 5 | RockTunnel_B1F_hns | local id 1 (45,33) | Visible | Wilds | `ITEM_FOCUS_SASH` | Treasure | Deep in the dark cave (replaces a junk TM Fling). |
| 6 | RockTunnel_B1F_hns | hidden 53,19 | Hidden | Wilds | `ITEM_OVAL_STONE` | Find | Authored; the Chansey line's stone. |
| 7 | PokemonTower_5F_Frlg | hidden 7,3 | Hidden | Dungeon: Pokemon Tower, floor 3 of 5 | `ITEM_TOXIC_ORB` | Find | Status-orb for Guts, Poison Heal and Facade users (replaces a Big Mushroom). |
| 8 | PokemonTower_6F_Frlg | local id 4 (5,15) | Visible | Dungeon: Pokemon Tower, floor 4 of 5 | `ITEM_TM_WILL_O_WISP` | Find | Status TM in the haunted tower (replaces a Rare Candy). |
| 9 | PokemonTower_7F_Frlg | hidden 11,4 | Hidden | Dungeon: Pokemon Tower, floor 5 of 5 | `ITEM_REAPER_CLOTH` | Treasure | Top of the tower, hidden (replaces a Soothe Bell). |

#### The Celadon Underground

**Build identity:** Disruption / trick control. **Start:** Celadon City.

Rocket's cellar and Silph Co.'s towers. These are story dungeons with trainers on every floor, so a player who beats them first earns Taunt, Black Glasses, Razor Claw, Trick Room and a Dubious Disc. Expect a tricky, tempo-stealing team.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | RocketHideout_B2F_Frlg | local id 4 (5,7) | Visible | Story dungeon: Celadon Rocket Hideout, floor 2 of 4 | `ITEM_TM_TAUNT` | Find | Authored (TM12); disruption TM from a Rocket cellar. |
| 2 | RocketHideout_B3F_Frlg | local id 5 (14,24) | Visible | Story dungeon: Celadon Rocket Hideout, floor 3 of 4 | `ITEM_BLACK_GLASSES` | Find | Authored; Dark boost. |
| 3 | RocketHideout_B4F_Frlg | local id 7 (1,6) | Visible | Story dungeon: Celadon Rocket Hideout, floor 4 of 4 | `ITEM_RAZOR_CLAW` | Treasure | Replaces TM Snatch (junk). Rare evolution item for Sneasel at the bottom of the hideout. |
| 4 | SilphCo_4F_Frlg | local id 8 (30,18) | Visible | Story dungeon: Silph Co., floor 3 of 10 | `ITEM_TM_TRICK_ROOM` | Treasure | Replaces TM Torment. Silph tech: speed-control TM that only this trail sells... |
| 5 | SilphCo_8F_Frlg | hidden 29,10 | Hidden | Story dungeon: Silph Co., floor 7 of 10 | `ITEM_DUBIOUS_DISC` | Treasure | Hidden in the upper offices (replaces a Nugget). |

#### The Grinder's Cycleway

**Build identity:** Sustain / grinder. **Start:** Lavender Town, south (Routes 14 and 15) or Celadon (Cycling Road).

A chain of hidden finds on the southern routes ending at the south tip of Cycling Road: Lucky Punch, Shell Bell, Big Root and Metronome. No danger spike, so it suits a first journey.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route14_hns | hidden 21,22 | Hidden | Wilds | `ITEM_LUCKY_PUNCH` | Find | Authored hidden Lucky Punch on a wild route. |
| 2 | Route15_hns | hidden 77,8 | Hidden | Wilds | `ITEM_SHELL_BELL` | Find | Far east end of Route 15 (replaces Rose Incense). |
| 3 | Route17_hns | hidden 16,156 | Hidden | Road | `ITEM_BIG_ROOT` | Find | Draining sustain at the dead end of Cycling Road (replaces a Max Ether). |
| 4 | Route17_hns | hidden 22,157 | Hidden | Road | `ITEM_METRONOME` | Find | Rewards a team that commits to one move; same dead end (replaces a Max Elixir). |

#### Standalone prizes

Prizes outside any trail.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | SSAnne_1F_Room2_Frlg | local id 4 (5,7) | Visible | Story dungeon: S.S. Anne cabin | `ITEM_TM_BRICK_BREAK` | Find | Authored (TM31) in a cabin of the one-time ship. |
| 2 | SSAnne_B1F_Room2_Frlg | local id 2 (3,2) | Visible | Story dungeon: S.S. Anne cabin | `ITEM_TM_SUBSTITUTE` | Find | Replaces TM Rest, which the Game Corner sells. A cabin of the one-time ship. |
| 3 | Route8_hns | hidden 18,12 | Hidden | Road | `ITEM_TM_SLEEP_TALK` | Find | Hidden on Route 8. Pairs with TM Rest, which the Game Corner sells. |

### Sevii

Sevii spots are identified by their exploration row id (`exploration.<map>.object.N` or `exploration.<map>.bg.N`, N being the FRLG source index) or by coordinates. The "local id" below is that FRLG source index. The generator maps each spot to the compiled local id after the [restoration](sevii-exploration-restoration.md), and fails on one it can't find.

#### Ember and Orchard

**Build identity:** Sustain / utility. **Start:** One Island (arrival by Seagallop ferry).

A north-isles chain through One, Two and Three Island: Shell Bell on Treasure Beach, Overheat at Mt. Ember, PP Max on the Cape, Substitute in the tunnel, then Lum Berry and Big Root in the Berry Forest. Moderate danger the whole way, a good first journey on Sevii.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | OneIsland_TreasureBeach_Frlg | hidden 15,29 | Hidden | Wilds | `ITEM_SHELL_BELL` | Find | Treasure Beach is the first dig on arrival (replaces Star Piece). |
| 2 | MtEmber_Exterior_Frlg | local id 19 (38,10) | Visible | Dungeon: Mt. Ember, floor 1 of 10: exterior | `ITEM_TM_OVERHEAT` | Find | Replaces a Fire Stone (sold in Goldenrod). Mt. |
| 3 | TwoIsland_CapeBrink_Frlg | hidden 16,28 | Hidden | Wilds | `ITEM_PP_MAX` | Treasure | Authored; hidden at the end of Two Island's cape. |
| 4 | ThreeIsland_DunsparceTunnel_Frlg | hidden 21,3 | Hidden | Road (Dunsparce Tunnel has no wild encounters; it takes the map it opens onto) | `ITEM_TM_SUBSTITUTE` | Find | Replaces a Nugget. Hidden in the tunnel. |
| 5 | ThreeIsland_BerryForest_Frlg | hidden 8,5 | Hidden | Wilds | `ITEM_LUM_BERRY` | Find | Authored hidden Lum Berry. |
| 6 | ThreeIsland_BerryForest_Frlg | hidden 47,5 | Hidden | Wilds | `ITEM_BIG_ROOT` | Find | Forest sustain (replaces a Chesto Berry). |

#### The Frozen Frontier

**Build identity:** Bulky wall / tricky defence. **Start:** Four Island (Icefall Cave).

Four and Five Island: Icefall's Never-Melt Ice and Ice Stone, a Prism Scale in the resort, the Rocket Warehouse's Sludge Bomb and Up-Grade, Metal Coat on Memorial Pillar, then the Lost Cave maze with Rocky Helmet, Weakness Policy and Protector, ending in a Lucky Egg.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | FourIsland_IcefallCave_B1F_Frlg | local id 2 (21,7) | Visible | Dungeon: Icefall Cave, floor 3 of 4 | `ITEM_NEVER_MELT_ICE` | Find | Authored. |
| 2 | FourIsland_IcefallCave_B1F_Frlg | local id 1 (10,14) | Visible | Dungeon: Icefall Cave, floor 3 of 4 | `ITEM_ICE_STONE` | Treasure | Replaces a Full Restore. Rare evolution stone. |
| 3 | FiveIsland_ResortGorgeous_Frlg | hidden 40,12 | Hidden | Wilds | `ITEM_PRISM_SCALE` | Find | Replaces Star Piece. Sea-resort flavour (Feebas line). |
| 4 | FiveIsland_RocketWarehouse_Frlg | local id 8 (17,3) | Visible | Story dungeon: Rocket Warehouse, single floor; Five Island | `ITEM_TM_SLUDGE_BOMB` | Find | Authored (TM36). |
| 5 | FiveIsland_RocketWarehouse_Frlg | local id 10 (4,5) | Visible | Story dungeon: Rocket Warehouse, single floor; Five Island | `ITEM_UP_GRADE` | Treasure | Authored. |
| 6 | FiveIsland_MemorialPillar_Frlg | local id 5 (4,47) | Visible | Outlands | `ITEM_METAL_COAT` | Treasure | Authored; far up the pillar. |
| 7 | FiveIsland_LostCave_Room11_Frlg | local id 1 (5,5) | Visible | Dungeon: Lost Cave, room 11 of 14 | `ITEM_ROCKY_HELMET` | Treasure | Replaces Lax Incense. |
| 8 | FiveIsland_LostCave_Room12_Frlg | local id 1 (5,5) | Visible | Dungeon: Lost Cave, room 12 of 14 | `ITEM_WEAKNESS_POLICY` | Treasure | Replaces Sea Incense. |
| 9 | FiveIsland_LostCave_Room13_Frlg | local id 1 (5,5) | Visible | Dungeon: Lost Cave, room 13 of 14 | `ITEM_PROTECTOR` | Treasure | Replaces Max Revive. Rhydon evolution item. |
| 10 | FiveIsland_LostCave_Room14_Frlg | local id 1 (5,5) | Visible | Dungeon: Lost Cave, room 14 of 14 | `ITEM_LUCKY_EGG` | Legend | End of the 14-room maze; Sevii's Legend (replaces a Rare Candy). |

#### Dragon and Sun

**Build identity:** Physical sweeper (dragons). **Start:** Six Island (Water Path).

Six Island's outer routes: a Dragon Scale at the end of Water Path, Dragon Pulse on Green Path, Dragon Claw, Dragon Fang and a Sun Stone in Ruin Valley, and an Amulet Coin on remote Outcast Island.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | SixIsland_WaterPath_Frlg | local id 9 (17,87) | Visible | Wilds | `ITEM_DRAGON_SCALE` | Treasure | Authored; at the far end of the path. |
| 2 | SixIsland_GreenPath_Frlg | hidden 12,9 | Hidden | Outlands | `ITEM_TM_DRAGON_PULSE` | Treasure | Replaces an Ultra Ball. |
| 3 | SixIsland_RuinValley_Frlg | local id 17 (43,32) | Visible | Outlands | `ITEM_SUN_STONE` | Treasure | Authored. |
| 4 | SixIsland_RuinValley_Frlg | local id 16 (19,11) | Visible | Outlands | `ITEM_TM_DRAGON_CLAW` | Treasure | Replaces a Full Restore. |
| 5 | SixIsland_RuinValley_Frlg | local id 15 (5,33) | Visible | Outlands | `ITEM_DRAGON_FANG` | Find | Replaces HP Up. |
| 6 | SixIsland_OutcastIsland_Frlg | hidden 6,24 | Hidden | Outlands | `ITEM_AMULET_COIN` | Legend | The most remote island; Sevii's second Legend (replaces a Net Ball). |

#### Seven Island Training Grounds

**Build identity:** EV training / mixed attacker. **Start:** Seven Island (Sevault Canyon, Trainer Tower).

All six Power items are scattered over Seven Island (bracer in the canyon entrance, three on the Trainer Tower grounds, belt and weight in the canyon), plus King's Rock, Deep Sea Tooth and Deep Sea Scale in Tanoby Ruins and a Life Orb at the far end of the ruins.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | SevenIsland_SevaultCanyon_Entrance_Frlg | hidden 8,29 | Hidden | Wilds | `ITEM_POWER_BRACER` | Find | Replaces a Rawst Berry. |
| 2 | SevenIsland_TrainerTower_Frlg | hidden 49,27 | Hidden | Wilds | `ITEM_POWER_LENS` | Find | Replaces a Big Pearl. |
| 3 | SevenIsland_TrainerTower_Frlg | hidden 47,30 | Hidden | Wilds | `ITEM_POWER_BAND` | Find | Replaces a Pearl. |
| 4 | SevenIsland_TrainerTower_Frlg | hidden 59,32 | Hidden | Wilds | `ITEM_POWER_ANKLET` | Find | Replaces a Nanab Berry. |
| 5 | SevenIsland_SevaultCanyon_Frlg | local id 18 (7,38) | Visible | Outlands | `ITEM_POWER_BELT` | Treasure | Replaces a Max Elixir. |
| 6 | SevenIsland_SevaultCanyon_Frlg | local id 19 (17,23) | Visible | Outlands | `ITEM_POWER_WEIGHT` | Treasure | Replaces a Nugget. |
| 7 | SevenIsland_SevaultCanyon_Frlg | local id 17 (18,45) | Visible | Outlands | `ITEM_KINGS_ROCK` | Treasure | Authored. |
| 8 | SevenIsland_TanobyRuins_Frlg | hidden 33,10 | Hidden | Outlands | `ITEM_DEEP_SEA_TOOTH` | Treasure | Replaces a Heart Scale. |
| 9 | SevenIsland_TanobyRuins_Frlg | hidden 86,9 | Hidden | Outlands | `ITEM_DEEP_SEA_SCALE` | Treasure | Replaces a Heart Scale. |
| 10 | SevenIsland_TanobyRuins_Frlg | hidden 125,5 | Hidden | Outlands | `ITEM_LIFE_ORB` | Treasure | Far end of the ruins; a mixed attacker's capstone (replaces a Heart Scale). |

### Johto

#### Sage's Trail

**Build identity:** Special attacker and setup support. **Start:** New Bark Town, south-west: Route 29/32, then Union Cave and the Ruins of Alph.

The quiet western road from New Bark. Almost nothing here is dangerous, so the loot is mostly clever: a Calm Mind TM at the bottom of a Mild cave, Wise Glasses in a dead end, and at the end the Ruins of Alph, where the Choice Specs sit in a hidden corner and the Ho-Oh chamber hands over a Life Orb. If you want a special attacker that sets up, start here. It is also the easiest trail to finish early.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route32_hns | hidden 30,97 | Hidden | Road | `ITEM_SHELL_BELL` | Find | Authored Shell Bell kept; it sits at the far south end of a long route, a real detour for a hidden item. |
| 2 | IlexForest_hns | hidden 57,8 | Hidden | Road (passage) | `ITEM_MIRACLE_SEED` | Find | Replaces a Big Mushroom (sell junk). Celebi's forest gives out a Grass booster, tucked in the maze. |
| 3 | UnionCave_B2F_hns | local id 3 (16,21) | Visible | Dungeon: Union Cave B2F (3 of 3) | `ITEM_TM_CALM_MIND` | Find | Upgraded from a Max Revive. B2F is the dead end below the cave's path, so a good setup TM is earned. |
| 4 | UnionCave_B2F_hns | hidden 4,34 | Hidden | Dungeon: Union Cave B2F (3 of 3) | `ITEM_WISE_GLASSES` | Find | Upgraded from Calcium. Special-attack booster at the far corner of the dead end. |
| 5 | RuinsOfAlph_Outside_hns | hidden 7,32 | Hidden | Wilds | `ITEM_TWISTED_SPOON` | Find | Replaces a Nugget. Psychic booster fits the ruins and the Unown. |
| 6 | RuinsOfAlph_Outside_hns | hidden 4,9; moved from LakeOfRage_hns hidden 20,19 | Hidden | Wilds | `ITEM_CHOICE_SPECS` | Treasure | Moved here from Lake of Rage so the special-attacker trail owns its signature item. |
| 7 | RuinsOfAlph_PuzzleAndRewardChambers_hns | local id 2 (3,38) | Visible (puzzle) | Wilds (Ruins chambers, puzzle-gated) | `ITEM_LIFE_ORB` | Treasure | Authored Life Orb kept. Only reached by finishing the Ho-Oh puzzle, which is the earn. |

#### Tidewalker

**Build identity:** Weather and sea control. **Start:** Olivine Port, by the S.S. Aqua from Kanto or by Surf from Cherrygrove; then Route 40/41 and Cianwood.

Johto's sea road. The loot is weather rocks (Damp, Smooth, Heat), Wave Incense, a Roost TM and Safety Goggles for the team that sets the weather, then Whirl Islands, which turn out an excellent finish: Light Clay for screens and, at the very bottom beside Lugia's whirlpool, a Catching Charm. You need Surf, Whirlpool and Waterfall for the last step, so this is a trail you get to late even if you pass Olivine early.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route40_hns | hidden 15,18 | Hidden | Road (sea lane) | `ITEM_TM_ROOST` | Find | Upgraded from TM Pluck. A hidden item in open water. |
| 2 | Route41_hns | hidden 28,79 | Hidden | Road (sea lane) | `ITEM_DAMP_ROCK` | Find | Replaces Stardust. Rain extender inside the whirlpool field. |
| 3 | Route47_hns | hidden 86,48 | Hidden | Wilds | `ITEM_WAVE_INCENSE` | Find | Authored Wave Incense kept. Water booster at the far end of the Cianwood cliffs. |
| 4 | Route47_hns | hidden 53,9 | Hidden | Wilds | `ITEM_SMOOTH_ROCK` | Find | Replaces a Lagging Tail. Sandstorm extender on the cliffs. |
| 5 | Route47_hns | hidden 31,13 | Hidden | Wilds | `ITEM_TM_WATER_PULSE` | Find | Replaces Lucky Egg (now a Sevii Legend). Water coverage for the sea trail. |
| 6 | WhirlIslands_1F_hns | local id 10 (8,41) | Visible | Dungeon: Whirl Islands 1F (1 of 6) | `ITEM_SAFETY_GOGGLES` | Find | Replaces a Full Restore (sold at TR 55). Weather-immunity utility for a weather team. |
| 7 | WhirlIslands_B1F_hns | local id 2 (29,16) | Visible | Dungeon: Whirl Islands B1F (2 of 6) | `ITEM_HEAT_ROCK` | Find | Replaces a Full Restore. Completes the weather rocks. |
| 8 | WhirlIslands_B1F_hns | local id 1 (65,62) | Visible | Dungeon: Whirl Islands B1F (2 of 6) | `ITEM_LIGHT_CLAY` | Treasure | Replaces a Big Nugget. Far corner of a Hard dungeon floor. |
| 9 | WhirlIslands_B2F_hns | hidden 34,18 | Hidden | Dungeon: Whirl Islands B2F (4 of 6) | `ITEM_CATCHING_CHARM` | Legend | Deepest spot before Lugia; a Legend that replaces a Revive. |

#### Goldenrod Circuit

**Build identity:** Speed control and status utility. **Start:** New Bark to Violet, then Route 31 into Dark Cave, or Azalea toward Goldenrod; Routes 34, 38, 39 and the National Park.

The crossroads trail, mostly hidden items on the busy central routes and then two caves. Orbs for Guts and Poison Heal, Trick Room with a Lagging Tail, Thunder Wave, Taunt and U-turn, and a Choice Scarf at the far end of Dark Cave. It is the build-your-own-tempo trail: easy to start, wide to complete.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route34_hns | hidden 17,17 | Hidden | Road | `ITEM_FLAME_ORB` | Find | Replaces a Rare Candy. Guts or burn-based builds. |
| 2 | Route38_hns | hidden 41,7 | Hidden | Road | `ITEM_LAGGING_TAIL` | Find | Replaces an HP Up. Pairs with the Trick Room TM in the Park. |
| 3 | Route38_hns | hidden 8,24 | Hidden | Road | `ITEM_QUICK_CLAW` | Find | Replaces a Max Potion. Cheeky speed-control item. |
| 4 | Route39_hns | hidden 29,45 | Hidden | Road | `ITEM_TOXIC_ORB` | Find | Replaces a PP Up. Poison Heal and status builds. |
| 5 | NationalPark_Normal_hns | local id 12 (3,43) | Visible | Wilds | `ITEM_TM_TRICK_ROOM` | Find | Upgraded from TM Dig. A corner ball in the park. |
| 6 | NationalPark_Normal_hns | local id 13 (39,15) | Visible | Wilds | `ITEM_TM_U_TURN` | Find | Replaces a Soothe Bell. Pivoting for a fast team. |
| 7 | DarkCave_NorthSide_hns | local id 2 (31,31) | Visible | Wilds | `ITEM_TM_TAUNT` | Find | Replaces a Star Piece. Dark cave, far from the path. |
| 8 | DarkCave_NorthSide_hns | hidden 7,21 | Hidden | Wilds | `ITEM_TM_THUNDER_WAVE` | Find | Replaces a Revive. Hidden at the west wall. |
| 9 | DarkCave_SouthSide_hns | hidden 33,20 | Hidden | Wilds | `ITEM_BLACK_GLASSES` | Find | Replaces a Black Flute. Dark boost for Taunt users. |
| 10 | DarkCave_SouthSide_hns | local id 7 (51,41) | Visible | Wilds | `ITEM_CHOICE_SCARF` | Treasure | Replaces a Max Revive. The south-east end of the biggest cave in the area, a long unlit walk from either door. |

#### Fists of Mortar

**Build identity:** Physical sweeper (Fighting and close quarters). **Start:** Ecruteak City, then Route 42 and Mt. Mortar.

Mt. Mortar is the dojo cave, so the loot is physical: Black Belt, a Bulk Up TM, Drain Punch, Scope Lens for crit work, a Muscle Band and at the bottom a Choice Band. A Shadow Claw TM waits in a hidden spot on Route 42 as the trail marker. It is the standard pick for Fighting types and a decent one for anything with Attack.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route42_hns | hidden 63,6; moved from Route42_hns local id 18 (60,6) | Hidden | Road | `ITEM_TM_SHADOW_CLAW` | Find | Moved from the visible Route 42 ball so the Road spot is hidden. |
| 2 | MtMortar_1F_North_hns | local id 7 (20,59) | Visible | Dungeon: Mt. Mortar 1F North (2 of 4) | `ITEM_BLACK_BELT` | Find | Replaces a King's Rock (sold at Mahogany). Fighting booster at the far south of the floor. |
| 3 | MtMortar_1F_North_hns | local id 4 (65,35); moved from Route39_hns local id 10 (11,13) | Visible | Dungeon: Mt. Mortar 1F North (2 of 4) | `ITEM_TM_DRAIN_PUNCH` | Find | Moved from Route 39 (a visible Road ball); replaces a Protector (sold at Mahogany). |
| 4 | MtMortar_1F_North_hns | hidden 41,45 | Hidden | Dungeon: Mt. Mortar 1F North (2 of 4) | `ITEM_TM_BULK_UP` | Find | Replaces an Iron Ball. Setup move for the dojo cave. |
| 5 | MtMortar_2F_hns | local id 5 (44,26) | Visible | Dungeon: Mt. Mortar 2F (3 of 4) | `ITEM_SCOPE_LENS` | Find | Replaces a Dragon Scale (sold at Mahogany). Crit support. |
| 6 | MtMortar_B1F_hns | local id 5 (35,47) | Visible | Dungeon: Mt. Mortar B1F (4 of 4) | `ITEM_MUSCLE_BAND` | Find | Replaces a Carbos. Deepest floor, so it can be the baseline here. |
| 7 | MtMortar_B1F_hns | local id 6 (50,48) | Visible | Dungeon: Mt. Mortar B1F (4 of 4) | `ITEM_CHOICE_BAND` | Treasure | Replaces a Max Ether. The trail's signature at the bottom of the cave. |

#### Rage and Ice

**Build identity:** Bulky wall and stall. **Start:** Mahogany Town by Route 42, then Route 43 to Lake of Rage; the Rocket Hideout and Ice Path follow.

The cold, slow north-east. Lake of Rage hides Heavy-Duty Boots, Rocky Helmet and Will-O-Wisp; the Rocket Hideout gives out Toxic and a Black Sludge; the Ice Path is a long way down for Big Root and finally an Assault Vest. Everything here helps a team that outlasts the other side. The Hideout is short; the Ice Path loops on its lower floors.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | LakeOfRage_hns | local id 11 (37,1) | Visible | Wilds | `ITEM_TM_WILL_O_WISP` | Find | Upgraded from TM Secret Power. Burn for physical walls. |
| 2 | LakeOfRage_hns | hidden 20,19 | Hidden | Wilds | `ITEM_ROCKY_HELMET` | Treasure | Takes the spot Choice Specs left. Contact punishment for a wall. |
| 3 | LakeOfRage_hns | local id 10 (21,3) | Visible | Wilds | `ITEM_HEAVY_DUTY_BOOTS` | Treasure | Replaces a Full Restore. Hazard immunity for a bulky lead. |
| 4 | RocketHideout_B2F_hns | local id 13 (3,17) | Visible | Dungeon: Rocket Hideout B2F (2 of 3) | `ITEM_TM_TOXIC` | Find | Upgraded from TM Thief. |
| 5 | RocketHideout_B3F_hns | local id 9 (3,16) | Visible | Dungeon: Rocket Hideout B3F (3 of 3) | `ITEM_BLACK_SLUDGE` | Treasure | Replaces a Guard Spec (sold). Poison sustain at the bottom of the Hideout. |
| 6 | IcePath_B2F_hns | local id 4 (7,6) | Visible | Dungeon: Ice Path B2F (3 of 5) | `ITEM_BIG_ROOT` | Find | Replaces a Full Heal. Draining stall support. |
| 7 | IcePath_B4F_hns | local id 1 (11,20) | Visible | Dungeon: Ice Path B4F (5 of 5) | `ITEM_ASSAULT_VEST` | Treasure | Replaces a Never-Melt Ice (sold at Mahogany). |

#### Dragon's Road to Silver

**Build identity:** Endgame physical breaker. **Start:** Blackthorn City for the Den, then the east: Routes 26 to 28 and Mt. Silver.

The long finish. Dragon's Den is the first test (Dragon Fang, PP Max, and an Exp. Charm hidden in the dark), then the eastern routes give Dragon Claw and Swords Dance, and Mt. Silver pays out everything an endgame team could want: Weakness Policy, Stone Edge, Expert Belt, Focus Sash, and at the very end of the snow, a Gold Bottle Cap. Brutal throughout; it is the answer to "what do I save until I can handle it".

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | DragonsDen_Cavern_hns | local id 8 (54,28) | Visible | Dungeon: Dragon's Den (single, Hard) | `ITEM_DRAGON_FANG` | Treasure | Authored Dragon Fang kept (not sold anywhere). |
| 2 | DragonsDen_Cavern_hns | local id 6 (54,50) | Visible | Dungeon: Dragon's Den (single, Hard) | `ITEM_PP_MAX` | Treasure | Authored PP Max kept. |
| 3 | DragonsDen_Cavern_hns | hidden 15,16 | Hidden | Dungeon: Dragon's Den (single, Hard) | `ITEM_EXP_CHARM` | Legend | Hidden far from the water path; a Legend that replaces a Max Elixir. |
| 4 | Route27_hns | local id 19 (133,17) | Visible | Wilds | `ITEM_TM_DRAGON_CLAW` | Treasure | Authored Dragon Claw kept at the far east end of the route. |
| 5 | Route28_hns | local id 16 (41,10) | Visible | Outlands | `ITEM_TM_SWORDS_DANCE` | Treasure | Replaces TM Flamethrower (sold in marts). Setup for the endgame physical breaker. |
| 6 | MtSilver_Outside_hns | hidden 16,22 | Hidden | Dungeon: Mt. Silver Outside (1) | `ITEM_WEAKNESS_POLICY` | Treasure | Replaces a Full Restore. Punishes breakers. |
| 7 | MtSilver_1F_WaterfallRoom_hns | hidden 28,28 | Hidden | Dungeon: Mt. Silver Waterfall Room (2) | `ITEM_EXPERT_BELT` | Treasure | Authored Expert Belt kept. |
| 8 | MtSilver_MountainSide_hns | local id 23 (31,29) | Visible | Dungeon: Mt. Silver Mountain Side (3) | `ITEM_TM_STONE_EDGE` | Treasure | Upgraded from TM Stealth Rock (sold in department stores). |
| 9 | MtSilver_1F_ItemRoom_hns | local id 1 (10,7) | Visible | Dungeon: Mt. Silver Item Room (3) | `ITEM_FOCUS_SASH` | Treasure | Replaces a Spell Tag. The item room is the reward room of the mountain. |
| 10 | MtSilver_Snow_hns | hidden 14,52 | Hidden | Dungeon: Mt. Silver Snow (5) | `ITEM_GOLD_BOTTLE_CAP` | Legend | Replaces a Pure Incense. The deepest hidden spot of Johto. |

#### Standalone prizes

Prizes outside any trail.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | UnionCave_B1F_hns | local id 3 (14,3); moved from IlexForest_hns local id 37 (71,42) | Visible | Dungeon: Union Cave B1F (2 of 3) | `ITEM_TM_FALSE_SWIPE` | Find | Moved from Ilex Forest (a visible Road ball). |
| 2 | Route26_hns | hidden 3,53 | Hidden | Outlands | `ITEM_EVIOLITE` | Treasure | Replaces a Tiny Mushroom. A far Outlands hidden spot earns a Treasure. |
| 3 | Route27_hns | local id 7 (69,14) | Visible | Wilds | `ITEM_MOON_STONE` | Find | Authored Moon Stone kept (not in the Mahogany shop or Goldenrod counter). |
| 4 | Route27_hns | local id 20 (93,14) | Visible | Wilds | `ITEM_DESTINY_KNOT` | Treasure | Authored Destiny Knot kept; breeders will talk about it. |
| 5 | TohjoFalls_Cavern_hns | local id 1 (15,11) | Visible | Wilds | `ITEM_WIDE_LENS` | Find | Replaces a Heart Scale. |
| 6 | RuinsOfAlph_PuzzleAndRewardChambers_hns | local id 3 (6,38) | Visible (puzzle) | Wilds (Ruins chambers) | `ITEM_CHARCOAL` | Find | Authored Charcoal kept. |
| 7 | RuinsOfAlph_PuzzleAndRewardChambers_hns | local id 5 (6,41) | Visible (puzzle) | Wilds (Ruins chambers) | `ITEM_SACRED_ASH` | Find | Authored Sacred Ash kept (party-wide revive). |
| 8 | RuinsOfAlph_PuzzleAndRewardChambers_hns | local id 6 (19,55) | Visible (puzzle) | Wilds (Ruins chambers) | `ITEM_SHARP_BEAK` | Find | Replaces a Heal Powder. |
| 9 | RuinsOfAlph_PuzzleAndRewardChambers_hns | local id 9 (22,58) | Visible (puzzle) | Wilds (Ruins chambers) | `ITEM_SPELL_TAG` | Find | Replaces a duplicate Moon Stone (Johto keeps the Route 27 one). |
| 10 | RuinsOfAlph_PuzzleAndRewardChambers_hns | local id 12 (19,38) | Visible (puzzle) | Wilds (Ruins chambers) | `ITEM_HARD_STONE` | Find | Replaces a Heal Powder. |
| 11 | RuinsOfAlph_PuzzleAndRewardChambers_hns | local id 14 (6,58) | Visible (puzzle) | Wilds (Ruins chambers) | `ITEM_MYSTIC_WATER` | Find | Replaces a Water Stone (sold at Goldenrod). |
| 12 | RuinsOfAlph_PuzzleAndRewardChambers_hns | local id 17 (3,58) | Visible (puzzle) | Wilds (Ruins chambers) | `ITEM_LUM_BERRY` | Find | Authored Lum Berry kept; the only status cure worth a puzzle. |

### Alola

#### Sun Isles

**Build identity:** Collector and evolution (Melemele to Akala). **Start:** Boat from Route 13 to Melemele Isle.

The gentle half of the Alola chain. A Sun Stone hidden on the arrival island, a Light Ball and Silver Powder on Akala, and an Eviolite in Akala Cave for the team of unevolved Pokemon. Everything is within an easy Surf of the harbour; a first-journey pick for collectors.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | MelemeleIsle_hns | hidden 42,25 | Hidden | Road | `ITEM_SUN_STONE` | Find | Replaces a Rare Candy. Sun Stone is not in the Mahogany evolution shop or the Goldenrod stone counter. |
| 2 | AkalaIsle_hns | local id 13 (42,12) | Visible | Wilds | `ITEM_LIGHT_BALL` | Find | Replaces an Iron (vitamin). |
| 3 | Akala_Forest_hns | hidden 11,14 | Hidden | Wilds | `ITEM_SILVER_POWDER` | Find | Replaces a Full Restore. Bug booster for a bug forest. |
| 4 | Akala_Cave_hns | local id 2 (23,17) | Visible | Dungeon: Akala Cave (single, Moderate) | `ITEM_EVIOLITE` | Treasure | Replaces an HP Up. Great for the NFE-rich Alola species. |

#### Far Isles

**Build identity:** Bulky and legendary finds (Ula'ula and Poni). **Start:** Surf from Akala Isle to Ula'ula, then Poni.

The remote half. Ula'ula Isle, its forest and cave hold a Rock Slide TM, a Bottle Cap and a Black Sludge; Poni Cave holds a Leaf Stone and a Choice Scarf; and Poni Isle, the wildest island, holds Leftovers. This is a trail you reach late in a first journey, and the most memorable one in Alola.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | UlaulaIsle_hns | local id 9 (32,65) | Visible | Outlands | `ITEM_TM_ROCK_SLIDE` | Find | Replaces a Protein. |
| 2 | UlaUla_Forest_hns | hidden 19,3 | Hidden | Outlands (no wild encounters of its own) | `ITEM_BOTTLE_CAP` | Treasure | Replaces a Max Revive. Hidden in the forest. |
| 3 | UlaUla_Cave_hns | local id 3 (25,27) | Visible | Dungeon: Ula'ula Cave (single, Hard) | `ITEM_BLACK_SLUDGE` | Treasure | Replaces an Ice Stone (sold). Alolan Muk and Grimer live here. |
| 4 | Poni_Cave_hns | local id 2 (8,6) | Visible | Dungeon: Poni Cave (single, Hard) | `ITEM_LEAF_STONE` | Find | Authored Leaf Stone kept (not in the Mahogany shop or Goldenrod counter). |
| 5 | Poni_Cave_hns | local id 3 (13,10) | Visible | Dungeon: Poni Cave (single, Hard) | `ITEM_CHOICE_SCARF` | Treasure | Deepest ball in Poni Cave; replaces a Thunder Stone. |
| 6 | PoniIsle_hns | local id 11 (24,17) | Visible | Outlands | `ITEM_LEFTOVERS` | Treasure | The wildest island holds the classic sustain item. |

### Sinjoh

#### Beyond the Silver

**Build identity:** Ice and endgame (hail and heavy hitters). **Start:** Through Mt. Silver 1F and Snowswept Cavern into Sinjoh.

Sinjoh lies beyond Mt. Silver, so the prizes only get reached by players who finished Johto. Icy Rock and a Focus Band in the cavern, an X-Scissor TM on Route 49, and a Life Orb on Route 50. Three hidden orbs are the reason to cross the whole region: Adamant and Lustrous on Route 50, Griseous buried in the hot springs, next to an Assault Vest.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | SnowsweptCavern_hns | local id 18 (27,18) | Visible | Wilds | `ITEM_ICY_ROCK` | Find | Authored Icy Rock kept. Hail extender at the first step into Sinjoh. |
| 2 | SnowsweptCavern_hns | hidden 44,23 | Hidden | Wilds | `ITEM_FOCUS_BAND` | Find | Replaces a Never-Melt Ice (sold at Mahogany). |
| 3 | Route49_hns | local id 23 (26,58) | Visible | Wilds | `ITEM_TM_X_SCISSOR` | Find | Upgraded from TM Rock Climb. |
| 4 | Route50_hns | local id 7 (33,37) | Visible | Outlands | `ITEM_LIFE_ORB` | Treasure | Replaces a Big Nugget. |
| 5 | NewSinjoh_HotSprings_hns | local id 6 (4,39) | Visible | Dungeon: New Sinjoh Hot Springs (single, Moderate) | `ITEM_ASSAULT_VEST` | Treasure | Replaces a Lava Cookie. The far end of the springs. |
| 6 | Route50_hns | hidden 68,54 | Hidden | Outlands | `ITEM_ADAMANT_ORB` | Legend | Far corner of the Outlands approach to the Ruins; replaces an Enigma Berry. |
| 7 | Route50_hns | hidden 9,37 | Hidden | Outlands | `ITEM_LUSTROUS_ORB` | Legend | Western edge of Route 50; promoted from a dynamic spot (Ice Stone). Sinjoh Ruins lore. |
| 8 | NewSinjoh_HotSprings_hns | hidden 28,37 | Hidden | Dungeon: New Sinjoh Hot Springs (single, Moderate) | `ITEM_GRISEOUS_ORB` | Legend | Buried in the springs; replaces a Dawn Stone. Sinjoh Ruins lore. |

### Hoenn

#### The Western Waters

**Build identity:** Bulky wall / stall. **Start:** Littleroot, west: Petalburg, then the Dewford sea lane into Granite Cave.

The gentle starter trail. A player who walks west out of Littleroot and ferries to Dewford collects the pieces of a durable, patient team: a Shell Bell tucked in the south end of the Route 105 sea lane, then Granite Cave's last floor, which hides three separate defensive finds. It is mild everywhere, so it is the safest first journey, and it ends in Eviolite, the item that makes Hoenn's evolving walls (Nosepass line, Chansey, Porygon2) actually tanky.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route105 | hidden 5,56 | Hidden | Road (sea lane) | `ITEM_SHELL_BELL` | Find | Sustain for a Dewford-bound swimmer; a hidden pickup at the far south of the lane. |
| 2 | GraniteCave_B2F | hidden 28,6 | Hidden | Dungeon: Granite Cave, floor 3 of 4 (B2F) | `ITEM_EVERSTONE` | Find | Keeps a Nosepass or Aron line from evolving; utility staple that stays where Emerald put it. |
| 3 | GraniteCave_B2F | local id 2 (29,4) | Visible | Dungeon: Granite Cave, floor 3 of 4 (B2F) | `ITEM_ROCKY_HELMET` | Find | Chip damage on contact for the trail's physical-wall pieces. |
| 4 | GraniteCave_B2F | hidden 15,11 | Hidden | Dungeon: Granite Cave, floor 3 of 4 (B2F) | `ITEM_EVIOLITE` | Treasure | The trail's capstone: hidden on the cave's deepest item floor, turns every not-fully-evolved Hoenn native into a wall. |

#### Meteor Run

**Build identity:** Physical sweeper. **Start:** Littleroot, north-west: Petalburg Woods, Rustboro, then east along Routes 116 and 115.

Out of Rustboro the road climbs toward Meteor Falls, and the prizes get steadily more dragon-shaped. A dark-type boost on Route 116 is the opener, Iron Tail waits in the first cave room, and the Wilds corner of Route 114 hides a Choice Band for anyone willing to cross the river. The deepest basin of Meteor Falls holds Dragon Claw. A run that finishes the trail owns a hard-hitting Salamence-or-Flygon-style attacker before the first gym circuit is done.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route116 | hidden 70,13 | Hidden | Road | `ITEM_BLACK_GLASSES` | Find | Dark boost for physical Crunch and Sucker Punch users. |
| 2 | MeteorFalls_1F_1R | local id 1 (2,4) | Visible | Dungeon: Meteor Falls, floor 1 of 5 | `ITEM_TM_IRON_TAIL` | Find | Solid physical coverage in the cave's far west corner. |
| 3 | Route114 | local id 9 (7,6) | Visible | Wilds | `ITEM_CHOICE_BAND` | Treasure | The river-locked north-west corner of a rugged Wilds route earns a Choice Item. |
| 4 | MeteorFalls_B1F_2R | local id 1 (5,3) | Visible | Dungeon: Meteor Falls, floor 4 of 5 | `ITEM_TM_DRAGON_CLAW` | Treasure | The best physical dragon move in the region, on the deepest basin before Steven's cave. |

#### Four Winds

**Build identity:** Weather. **Start:** Slateport (S.S. Aqua arrival), north through Mauville and the Route 111 desert.

Hoenn is the weather region, and this trail is the full set: Sandstorm, Sun, Rain and Hail, each with its matching rock. It starts in the desert, picks up a Heat Rock on the ash route, then spreads out: the Jagged Pass TM, the Scorched Slab off Route 120, the Abandoned Ship's hidden floor and finally the frozen heart of Shoal Cave. The danger climbs with every leg, and the last two are the first genuinely hard dungeon rooms in the region.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route111 | hidden 26,70 | Hidden | Road (desert interior) | `ITEM_SMOOTH_ROCK` | Find | Lengthens Sandstorm; hidden in the desert, so it is only found by sweeping the sand. |
| 2 | Route113 | hidden 22,5 | Hidden | Road (ash route) | `ITEM_HEAT_ROCK` | Find | Lengthens Sun on the volcanic ash route; hidden deep in the grass. |
| 3 | JaggedPass | local id 3 (23,24); moved from Route111 local id 18 (33,104) | Visible | Wilds | `ITEM_TM_SANDSTORM` | Find | The Sandstorm TM moves off the open desert to a rugged Wilds trail so it is actually earned. |
| 4 | ScorchedSlab | local id 1 (7,5) | Visible | Wilds (end of the Route 120 branch) | `ITEM_TM_SUNNY_DAY` | Find | Dead-end of the jungle branch; the fitting home for the Sun TM. |
| 5 | AbandonedShip_HiddenFloorRooms | local id 1 (41,4) | Visible | Dungeon: Abandoned Ship, floor 3 of 3 (hidden floor) | `ITEM_DAMP_ROCK` | Find | Lengthens Rain, paired with the ship's Rain TM. |
| 6 | AbandonedShip_HiddenFloorRooms | local id 3 (5,11) | Visible | Dungeon: Abandoned Ship, floor 3 of 3 (hidden floor) | `ITEM_TM_RAIN_DANCE` | Find | Behind the key-room puzzle on the hidden floor. |
| 7 | ShoalCave_LowTideIceRoom | local id 1 (12,8) | Visible | Dungeon: Shoal Cave, floor 5 of 5 (ice room) | `ITEM_TM_HAIL` | Treasure | Deepest Shoal Cave room, tide-gated; the Hail TM finishes the set. |
| 8 | ShoalCave_LowTideIceRoom | local id 2 (12,21) | Visible | Dungeon: Shoal Cave, floor 5 of 5 (ice room) | `ITEM_ICY_ROCK` | Treasure | Lengthens Hail; replaces the weaker type-boost at the same spot. |

#### Ghost Mountain

**Build identity:** Special attacker. **Start:** Mauville, east: Routes 118 and 123, then Route 122 to Mt. Pyre.

Mt. Pyre is the region's special-attacking pilgrimage. The climb rewards a Spell Tag on the second floor and the Shadow Ball TM at the top of the tower, then the open slopes outside hide a Choice Specs for those who wander off the path. It is a short trail with a very clear identity: a ghost/dark special attacker with its boost item, its STAB TM and its Choice.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | MtPyre_2F | local id 2 (0,10) | Visible | Dungeon: Mt. Pyre, floor 2 of 8 | `ITEM_SPELL_TAG` | Find | Ghost boost placed at the first real floor, in a side corner. |
| 2 | MtPyre_6F | local id 2 (6,9) | Visible | Dungeon: Mt. Pyre, floor 6 of 8 | `ITEM_TM_SHADOW_BALL` | Treasure | Top of the interior climb; the TM that defines the build. |
| 3 | MtPyre_Exterior | hidden 9,8 | Hidden | Dungeon: Mt. Pyre, floor 7 of 8 (exterior) | `ITEM_CHOICE_SPECS` | Treasure | Hidden on the outdoor slopes; the Special Choice item. |

#### Magma Descent

**Build identity:** Status / speed control. **Start:** Lavaridge and Fallarbor, north-central: Fiery Path and Jagged Pass into the Magma Hideout.

Trade power for control. Fiery Path and Jagged Pass hide the Hoenn status set: Toxic, a Fire Stone, a Toxic Orb (Breloom's Poison Heal) and a Flame Orb (Swellow's Guts). Then the Magma Hideout pays out for those willing to fight through to its upper floors: a Black Sludge, a PP Max and a Choice Scarf on the last floor. A trainer who walks this trail gets the toolkit for a utility attacker or a status-spreading support player.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | FieryPath | local id 1 (8,3) | Visible | Wilds | `ITEM_TM_TOXIC` | Find | Poison for stall and support teams; the north end of the tunnel. |
| 2 | FieryPath | local id 8 (7,32) | Visible | Wilds | `ITEM_FIRE_STONE` | Find | Evolution stone in the cave's south dead end. |
| 3 | JaggedPass | hidden 8,10 | Hidden | Wilds | `ITEM_TOXIC_ORB` | Find | Poison Heal fuel for Breloom; hidden on the mountain trail. |
| 4 | JaggedPass | hidden 7,29 | Hidden | Wilds | `ITEM_FLAME_ORB` | Find | Guts fuel for Swellow-style attackers; a second hidden item on the same trail. |
| 5 | MagmaHideout_3F_1R | local id 3 (9,16) | Visible | Dungeon: Magma Hideout, floor 5 of 8 | `ITEM_BLACK_SLUDGE` | Treasure | Replaces a Nugget. Poison sustain for a status team. |
| 6 | MagmaHideout_3F_2R | local id 2 (5,9) | Visible | Dungeon: Magma Hideout, floor 6 of 8 | `ITEM_PP_MAX` | Treasure | Authored in place and still earns its spot on the way down. |
| 7 | MagmaHideout_4F | local id 8 (3,7) | Visible | Dungeon: Magma Hideout, floor 8 of 8 | `ITEM_CHOICE_SCARF` | Treasure | The final floor of the hideout; speed control is the trail's payoff. |

#### The Long Way to Sootopolis

**Build identity:** Collector / evolution, with a glass-cannon kit. **Start:** Slateport harbor (S.S. Aqua arrival), south-east across the far sea, then Dive.

The long route nobody takes by accident. Past Slateport, Route 133's currents drop a Dragon Scale (Seadra to Kingdra), a Focus Sash and an Expert Belt on separate islets. Beyond them the player must Dive: Deep Sea Tooth and Scale for Clamperl's two evolutions, and, in the very last corner of Underwater Route 126, the Soul Dew. It is the longest and most dangerous trail, so every piece is a Treasure or better.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | Route133 | local id 4 (53,12) | Visible | Outlands | `ITEM_DRAGON_SCALE` | Treasure | Kingdra evolution item for the Horsea line on the far-sea islets. |
| 2 | Route133 | local id 5 (8,10) | Visible | Outlands | `ITEM_FOCUS_SASH` | Treasure | Glass-cannon insurance on the far west islet. |
| 3 | Route133 | local id 10 (48,28) | Visible | Outlands | `ITEM_EXPERT_BELT` | Treasure | Rewards the player who crosses the currents. |
| 4 | Underwater_Route124 | hidden 64,54 | Hidden | Outlands | `ITEM_DEEP_SEA_TOOTH` | Treasure | Huntail evolution item; Clamperl lives here. |
| 5 | Underwater_Route126 | hidden 63,19 | Hidden | Outlands | `ITEM_DEEP_SEA_SCALE` | Treasure | Gorebyss evolution item; the pair to the Tooth. |
| 6 | Underwater_Route126 | hidden 9,77 | Hidden | Outlands | `ITEM_SOUL_DEW` | Legend | The farthest corner of the deepest dive map; the Lati twins' signature item. |

#### Standalone prizes

Prizes outside any trail.

| # | Map | Spot | Shown | Reach | Item | Tier | Why |
|---|---|---|---|---|---|---|---|
| 1 | AquaHideout_B1F | local id 5 (15,9) | Visible | Dungeon: Aqua Hideout, floor 2 of 3 | `ITEM_MASTER_BALL` | Legend | The one Master Ball in Hoenn, at the end of a warp-pad puzzle room. |
| 2 | SeafloorCavern_Room9 | local id 6 (14,5) | Visible | Dungeon: Seafloor Cavern, floor 10 of 10 (room 9) | `ITEM_TM_EARTHQUAKE` | Legend | The best Ground TM, in the last hidden room of the deepest cave. |
| 3 | SSTidalLowerDeck | hidden 0,2 | Hidden | Road (ferry) | `ITEM_LEFTOVERS` | Treasure | The classic hidden Leftovers, behind a ferry ride. |
| 4 | ArtisanCave_B1F | hidden 7,5 | Hidden | Dungeon: Artisan Cave, floor 2 of 2 | `ITEM_WEAKNESS_POLICY` | Treasure | Replaces a Protein. Hidden in a post-game cave. |
| 5 | VictoryRoad_B1F | local id 18 (42,8) | Visible | Dungeon: Victory Road, floor 2 of 3 | `ITEM_ASSAULT_VEST` | Treasure | TM Psychic is bought at the Mauville Game Corner. |
| 6 | VictoryRoad_B2F | hidden 37,1 | Hidden | Dungeon: Victory Road, floor 3 of 3 | `ITEM_LIFE_ORB` | Treasure | The far hidden corner of the deepest floor. |
| 7 | MeteorFalls_1F_1R | local id 2 (2,14) | Visible | Dungeon: Meteor Falls, floor 1 of 5 | `ITEM_MOON_STONE` | Find | Evolution stone kept at its Emerald location. |
| 8 | AbandonedShip_HiddenFloorRooms | local id 4 (31,11) | Visible | Dungeon: Abandoned Ship, floor 3 of 3 (hidden floor) | `ITEM_WATER_STONE` | Find | Evolution stone on the ship's hidden floor (Lombre line). |
| 9 | NewMauville_Inside | local id 3 (39,4) | Visible | Dungeon: New Mauville, floor 2 of 2 | `ITEM_THUNDER_STONE` | Find | Evolution stone in the power plant's inner hall. |
| 10 | Route120 | hidden 0,86; moved from Route119 local id 20 (25,76) | Hidden | Wilds | `ITEM_LEAF_STONE` | Find | A visible ball on a Road route is not a prize. |
| 11 | SafariZone_Northwest | local id 2 (33,7) | Visible | Wilds (Safari preserve) | `ITEM_TM_ENERGY_BALL` | Find | Replaces TM Solar Beam (sold in marts). Special Grass coverage inside the preserve. |
| 12 | Route119 | hidden 38,63 | Hidden | Road | `ITEM_PRISM_SCALE` | Find | Hidden near Feebas waters; Feebas to Milotic. |

### Fixed story items

These are not prizes and are never moved or replaced.

- **Kanto:** Old Sea Map (VermilionCity_hns local id 22); Gold Teeth (FuchsiaCity_SafariZoneBeach_hns local id 9); Silph Scope and Lift Key (RocketHideout_B4F_Frlg local ids 2 and 4); Silph Card Key (SilphCo_5F_Frlg local id 8); Secret Key (PokemonMansion_B1F_Frlg local id 6); starter Poke Balls (PalletTown_Lab_hns local ids 6 to 8); Electrode decoys in PowerPlant_Frlg. NPC rewards are untouched: the Silph President's Master Ball, Poke Flute, the Nugget Bridge Nugget, the Safari HMs and the Viridian Gym TM Earthquake (a story reward, not a spot; it duplicates the Hoenn Legend and is left as it is).
- **Sevii:** HM07 (FourIsland_IcefallCave_1F_Frlg local id 2). Other rewards are given by NPCs.
- **Johto:** Coin Case (GoldenrodCity_UndergroundTunnel_hns local id 8); GS Ball (RuinsOfAlph_B1F_hns local id 15); HM Waterfall (IcePath_1F_hns local id 2); Azure Flute (MtSilver_2F_hns local id 18).
- **Alola:** UlaUla_Cave_2_hns local id 1 has no script or flag and is left alone.
- **Sinjoh:** the Growlithe Poke Ball in SinjohRuins_House1_hns local id 7 (a script-driven Pokemon gift).
- **Hoenn:** starter Poke Balls in Birch's lab and the Brendan/May houses; the Beldum Poke Ball in MossdeepCity_StevensHouse; Electrode and Voltorb battle balls in AquaHideout_B1F and NewMauville_Inside; Storage Key (AbandonedShip_CaptainsOffice); Scanner and the Room keys on AbandonedShip_HiddenFloorRooms; Sacred Ash on NavelRock_Top; the Mirage Tower and Desert Underpass fossils; Steven's Dive HM and the Devon, Wally and Scope quest gifts.

### Demoted to dynamic

Every authored item spot that is not listed above becomes a dynamic spot in the daily pool. Counts are spots per region; Kanto and Sevii counts are exact (from the region authors' tables), the others are approximate (spots in the compiled map set minus prizes and fixed items).

| Region | Dynamic spots | Notable demoted items |
|---|---|---|
| Kanto | 128 | Nuggets, Pearls, Big Pearls and Heart Scales (sell items); Escape Ropes; vitamins and Rare Candies; Revives and Max Revives; Ultra Balls; TMs Recycle, Silver Wind, Psych Up, Grass Knot, Frustration, Protect, Blizzard; the Pewter and Celadon-house spots vacated by Wise Glasses and Leftovers; the second and third Moon Stones and the Goldenrod-sold Thunder, Water and Fire Stones |
| Sevii | 64 | Ultra, Net and Heart Scale items; Fire Stones, Pearls, Star Pieces; berries; Lax and Sea Incense; the Sevault Canyon House Lucky Punch |
| Johto | about 81 | Restoratives and X items; vitamins and Rare Candies; sell items (Heart Scale, Nugget, Pearl, Star Piece); flutes; Mahogany-shop items (King's Rock, Dragon Scale, Protector, Never-Melt Ice, Reaper Cloth, Dubious Disc, Dawn Stone, Ice Stone); weak TMs (Bullet Seed, Rock Tomb, Secret Power, Dig, Charge Beam, Embargo, Thief, Pluck, Rock Climb) and the mart-sold Flamethrower and Stealth Rock; Mint |
| Alola | about 6 | Restoratives, a Rare Candy and sell items |
| Sinjoh | about 13 | Ultra Ball, Lure, Star Dust, King's Rock, Snowball, Nugget, Dawn Stone, Max Revive and similar; a Route 50 hidden spot (Ice Stone) and the Hot Springs hidden Dawn Stone were promoted to Legends instead |
| Hoenn | about 280 | Vitamins, Rare Candies, PP Ups; Heart Scales, shards and Nuggets; marts' Balls (Luxury, Dive, Nest, Net) and Escape Ropes; TMs Focus Punch, Skill Swap and Ice Beam; Sea and Lax Incense; mail and Revival Herb; the second Everstone; Battle Frontier and Pyramid items are out of scope |

New dynamic spots created by reconciliation: none beyond the Kanto Victory Road spots above (Max Ether, Rare Candy, two Max Revives, HP Up, Potion) and the spots already vacated by moved prizes. The authored Lucky Egg (Route 47 hidden) and TM Earthquake (Kanto Victory Road B2F) spots were given other prizes (Water Pulse and Rocky Helmet) rather than demoted. The Amulet Coin in the Goldenrod Department Store basement becomes a dynamic spot. The Ability Capsule, Ability Patch, Gold Bottle Cap, Exp. Charm, Catching Charm and the Sevii and Sinjoh Legends are placed at spots that held other items, so no authored copy of them exists to replace.

## Validation

- Every item constant in this spec exists in the Wayfarer build and is
  obtainable (no Gen IX-only items).
- Every static prize names an existing spot in a map Wayfarer compiles, or one
  of Sevii's restored spots.
- No static prize sits as a visible ball on a Road map.
- Key items, HMs and script-dependent items keep their authored spots.
- Each region's Legend items appear nowhere else as static prizes, and the
  dynamic TM list excludes them.
