# Safari encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. These tables follow the
[Safari Zones rules](safari-zones.md). They are the source of truth for the
Safari Zones' and the Bug-Catching Contest's wild-encounter data: every slot
below maps one to one to a slot in the game's encounter tables. Playtesting may
change the picks.

## Scope

This spec lists the tables for the Kanto, Johto and Hoenn Safari Zones (16
areas) and the Bug-Catching Contest's three contest days. Each has a day and a
night table for every method it has.

## Behavior

### Reading the tables

- Places are ordered Kanto, Johto and Hoenn Safari, then the contest days. Each
  place names its reach and its Safari Zone, and each map is named by its map
  constant. A map with surfing or fishing also names its water type.
- A contest day is named by the contest map's constant followed by the day,
  such as `MAP_NATIONAL_PARK_BUG_CONTEST_HNS:TUESDAY`. The implementation
  chooses the contest's table by contest day.
- Each table lists every slot in order, with its weight, and the species for
  day and night.
  - Land has 12 slots weighted 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1 and 1%.
  - Surfing, Headbutt trees (Johto) and Rock Smash rocks (Hoenn) have 5 slots
    weighted 60, 30, 5, 4 and 1%.
  - Fishing has the [Standard Rod](standard-rod-fishing.md)'s 10 entries, with
    each entry's weight for the Old, Good and Super Rod.
- **A slot's species is its stage cap,** the last name in the cell.
  "Bunnelby–Diggersby" means the slot holds Diggersby, and the game steps it
  down to Bunnelby below Diggersby's evolution level.
- Names map to species constants in capitals, with spaces and hyphens as
  underscores, accents dropped and other punctuation removed: Flabébé is
  `SPECIES_FLABEBE`. A form in brackets follows the name: Vivillon (High
  Plains) is `SPECIES_VIVILLON_HIGH_PLAINS` and Floette (Red) is
  `SPECIES_FLOETTE_RED`.
- Slots hold no levels. Levels come from the Wilds reach, as the PRD defines.

### Tables

#### Kanto Safari: Beach

Wilds, Kanto Safari.

**`MAP_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 2 | 20% | Fletchling–Talonflame | Binacle–Barbaracle |
| 3 | 10% | Binacle–Barbaracle | Inkay–Malamar |
| 4 | 10% | Scatterbug (Marine)–Vivillon (Marine) | Noibat–Noivern |
| 5 | 10% | Litleo–Pyroar | Litleo–Pyroar |
| 6 | 10% | Furfrou | Inkay–Malamar |
| 7 | 5% | Pancham–Pangoro | Pancham–Pangoro |
| 8 | 5% | Scatterbug (Marine)–Vivillon (Marine) | Noibat–Noivern |
| 9 | 4% | Bunnelby–Diggersby | Furfrou |
| 10 | 4% | Binacle–Barbaracle | Binacle–Barbaracle |
| 11 | 1% | Froakie–Greninja | Froakie–Greninja |
| 12 | 1% | Furfrou | Noibat–Noivern |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 30% | Binacle–Barbaracle | Clauncher–Clawitzer |
| 3 | 5% | Skrelp–Dragalge | Skrelp–Dragalge |
| 4 | 4% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 5 | 1% | Froakie–Greninja | Froakie–Greninja |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Binacle–Barbaracle | Clauncher–Clawitzer |
| 3 | 10% | 12% | 11% | Skrelp–Dragalge | Skrelp–Dragalge |
| 4 | 8% | 10% | 10% | Clauncher–Clawitzer | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 6 | 4% | 7% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 7 | 3% | 6% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 8 | 3% | 5% | 9% | Clauncher–Clawitzer | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 10 | 2% | 4% | 9% | Froakie–Greninja | Froakie–Greninja |

#### Kanto Safari: Brush

Wilds, Kanto Safari.

**`MAP_FUCHSIA_CITY_SAFARI_ZONE_BRUSH_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Litleo–Pyroar | Litleo–Pyroar |
| 2 | 20% | Bunnelby–Diggersby | Noibat–Noivern |
| 3 | 10% | Pancham–Pangoro | Pancham–Pangoro |
| 4 | 10% | Scatterbug (Meadow)–Vivillon (Meadow) | Noibat–Noivern |
| 5 | 10% | Fletchling–Talonflame | Bunnelby–Diggersby |
| 6 | 10% | Skiddo–Gogoat | Skiddo–Gogoat |
| 7 | 5% | Furfrou | Litleo–Pyroar |
| 8 | 5% | Litleo–Pyroar | Noibat–Noivern |
| 9 | 4% | Scatterbug (Meadow)–Vivillon (Meadow) | Pancham–Pangoro |
| 10 | 4% | Pancham–Pangoro | Furfrou |
| 11 | 1% | Furfrou | Noibat–Noivern |
| 12 | 1% | Bunnelby–Diggersby | Bunnelby–Diggersby |

#### Kanto Safari: Cave

Wilds, Kanto Safari.

**`MAP_FUCHSIA_CITY_SAFARI_ZONE_CAVE_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Noibat–Noivern | Noibat–Noivern |
| 2 | 20% | Bergmite–Avalugg | Bergmite–Avalugg |
| 3 | 10% | Carbink | Noibat–Noivern |
| 4 | 10% | Bunnelby–Diggersby | Carbink |
| 5 | 10% | Noibat–Noivern | Bunnelby–Diggersby |
| 6 | 10% | Bunnelby–Diggersby | Inkay–Malamar |
| 7 | 5% | Carbink | Noibat–Noivern |
| 8 | 5% | Bergmite–Avalugg | Carbink |
| 9 | 4% | Binacle–Barbaracle | Bergmite–Avalugg |
| 10 | 4% | Carbink | Bunnelby–Diggersby |
| 11 | 1% | Carbink | Carbink |
| 12 | 1% | Noibat–Noivern | Noibat–Noivern |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Binacle–Barbaracle | Inkay–Malamar |
| 2 | 30% | Clauncher–Clawitzer | Binacle–Barbaracle |
| 3 | 5% | Skrelp–Dragalge | Skrelp–Dragalge |
| 4 | 4% | Binacle–Barbaracle | Binacle–Barbaracle |
| 5 | 1% | Bergmite–Avalugg | Bergmite–Avalugg |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Binacle–Barbaracle | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Clauncher–Clawitzer | Binacle–Barbaracle |
| 3 | 10% | 12% | 11% | Skrelp–Dragalge | Skrelp–Dragalge |
| 4 | 8% | 10% | 10% | Binacle–Barbaracle | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 6 | 4% | 7% | 10% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 7 | 3% | 6% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 8 | 3% | 5% | 9% | Binacle–Barbaracle | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Binacle–Barbaracle | Binacle–Barbaracle |
| 10 | 2% | 4% | 9% | Bergmite–Avalugg | Bergmite–Avalugg |

#### Kanto Safari: Mountain

Wilds, Kanto Safari.

**`MAP_FUCHSIA_CITY_SAFARI_ZONE_MOUNTAIN_HNS`**

Water type: cold water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Skiddo–Gogoat | Skiddo–Gogoat |
| 2 | 20% | Fletchling–Talonflame | Bergmite–Avalugg |
| 3 | 10% | Bergmite–Avalugg | Noibat–Noivern |
| 4 | 10% | Scatterbug (Icy Snow)–Vivillon (Icy Snow) | Pancham–Pangoro |
| 5 | 10% | Pancham–Pangoro | Noibat–Noivern |
| 6 | 10% | Bunnelby–Diggersby | Noibat |
| 7 | 5% | Skiddo–Gogoat | Noibat–Noivern |
| 8 | 5% | Bergmite–Avalugg | Bergmite–Avalugg |
| 9 | 4% | Litleo–Pyroar | Litleo–Pyroar |
| 10 | 4% | Carbink | Carbink |
| 11 | 1% | Scatterbug (Icy Snow)–Vivillon (Icy Snow) | Noibat–Noivern |
| 12 | 1% | Furfrou | Scatterbug (Icy Snow)–Vivillon (Icy Snow) |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 30% | Bergmite–Avalugg | Clauncher–Clawitzer |
| 3 | 5% | Binacle–Barbaracle | Binacle–Barbaracle |
| 4 | 4% | Skrelp–Dragalge | Skrelp–Dragalge |
| 5 | 1% | Froakie–Greninja | Froakie–Greninja |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Bergmite–Avalugg | Clauncher–Clawitzer |
| 3 | 10% | 12% | 11% | Binacle–Barbaracle | Binacle–Barbaracle |
| 4 | 8% | 10% | 10% | Clauncher–Clawitzer | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 6 | 4% | 7% | 10% | Bergmite–Avalugg | Bergmite–Avalugg |
| 7 | 3% | 6% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 8 | 3% | 5% | 9% | Clauncher–Clawitzer | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Skrelp–Dragalge | Skrelp–Dragalge |
| 10 | 2% | 4% | 9% | Froakie–Greninja | Froakie–Greninja |

#### Johto Safari: Low left

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_LOW_LEFT_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bunnelby–Diggersby | Espurr |
| 2 | 20% | Espurr | Bunnelby–Diggersby |
| 3 | 10% | Fletchling–Talonflame | Phantump |
| 4 | 10% | Scatterbug (Sandstorm)–Vivillon (Sandstorm) | Dedenne |
| 5 | 10% | Dedenne | Phantump |
| 6 | 10% | Bunnelby–Diggersby | Phantump |
| 7 | 5% | Espurr | Phantump |
| 8 | 5% | Scatterbug (Sandstorm)–Vivillon (Sandstorm) | Espurr |
| 9 | 4% | Fletchling–Talonflame | Dedenne |
| 10 | 4% | Dedenne | Phantump |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Espurr | Espurr |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Fletchling–Talonflame | Phantump |
| 2 | 30% | Scatterbug (Sandstorm)–Spewpa (Sandstorm) | Scatterbug (Sandstorm)–Spewpa (Sandstorm) |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Phantump | Fletchling–Talonflame |
| 5 | 1% | Scatterbug (Sandstorm)–Vivillon (Sandstorm) | Scatterbug (Sandstorm)–Vivillon (Sandstorm) |

#### Johto Safari: Low middle

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_LOW_MID_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Espurr | Flabébé (Yellow)–Floette (Yellow) |
| 2 | 20% | Flabébé (Yellow)–Floette (Yellow) | Espurr |
| 3 | 10% | Fletchling–Talonflame | Phantump |
| 4 | 10% | Scatterbug (High Plains)–Vivillon (High Plains) | Bunnelby–Diggersby |
| 5 | 10% | Bunnelby–Diggersby | Phantump |
| 6 | 10% | Dedenne | Phantump |
| 7 | 5% | Flabébé (Yellow)–Floette (Yellow) | Espurr |
| 8 | 5% | Bunnelby–Diggersby | Phantump |
| 9 | 4% | Spritzee | Spritzee |
| 10 | 4% | Scatterbug (High Plains)–Vivillon (High Plains) | Phantump |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (Yellow)–Floette (Yellow) | Espurr |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 30% | Skrelp–Dragalge | Clauncher–Clawitzer |
| 3 | 5% | Binacle–Barbaracle | Binacle–Barbaracle |
| 4 | 4% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 5 | 1% | Inkay–Malamar | Inkay–Malamar |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Skrelp–Dragalge | Clauncher–Clawitzer |
| 3 | 10% | 12% | 11% | Binacle–Barbaracle | Binacle–Barbaracle |
| 4 | 8% | 10% | 10% | Clauncher–Clawitzer | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 6 | 4% | 7% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 7 | 3% | 6% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 8 | 3% | 5% | 9% | Clauncher–Clawitzer | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 10 | 2% | 4% | 9% | Inkay–Malamar | Inkay–Malamar |

#### Johto Safari: Low right

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_LOW_RIGHT_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Flabébé (Blue)–Floette (Blue) | Phantump |
| 2 | 20% | Bunnelby–Diggersby | Flabébé (Blue)–Floette (Blue) |
| 3 | 10% | Fletchling–Talonflame | Espurr |
| 4 | 10% | Scatterbug (River)–Vivillon (River) | Phantump |
| 5 | 10% | Dedenne | Dedenne |
| 6 | 10% | Spritzee | Phantump |
| 7 | 5% | Flabébé (Blue)–Floette (Blue) | Spritzee |
| 8 | 5% | Espurr | Espurr |
| 9 | 4% | Scatterbug (River)–Vivillon (River) | Swirlix |
| 10 | 4% | Swirlix | Phantump |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (Blue)–Floette (Blue) | Espurr |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Skrelp–Dragalge | Inkay–Malamar |
| 2 | 30% | Clauncher–Clawitzer | Skrelp–Dragalge |
| 3 | 5% | Binacle–Barbaracle | Binacle–Barbaracle |
| 4 | 4% | Skrelp–Dragalge | Skrelp–Dragalge |
| 5 | 1% | Inkay–Malamar | Inkay–Malamar |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Skrelp–Dragalge | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Clauncher–Clawitzer | Skrelp–Dragalge |
| 3 | 10% | 12% | 11% | Binacle–Barbaracle | Binacle–Barbaracle |
| 4 | 8% | 10% | 10% | Skrelp–Dragalge | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 6 | 4% | 7% | 10% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 7 | 3% | 6% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 8 | 3% | 5% | 9% | Skrelp–Dragalge | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Skrelp–Dragalge | Skrelp–Dragalge |
| 10 | 2% | 4% | 9% | Inkay–Malamar | Inkay–Malamar |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Fletchling–Talonflame | Phantump |
| 2 | 30% | Scatterbug (River)–Spewpa (River) | Scatterbug (River)–Spewpa (River) |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Phantump | Fletchling–Talonflame |
| 5 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |

#### Johto Safari: Top left

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_TOP_LEFT_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Flabébé (Red)–Floette (Red) | Flabébé (Red)–Floette (Red) |
| 2 | 20% | Fletchling–Talonflame | Espurr |
| 3 | 10% | Bunnelby–Diggersby | Phantump |
| 4 | 10% | Scatterbug (Savanna)–Vivillon (Savanna) | Swirlix |
| 5 | 10% | Spritzee | Phantump |
| 6 | 10% | Swirlix | Spritzee |
| 7 | 5% | Flabébé (Red)–Floette (Red) | Espurr |
| 8 | 5% | Dedenne | Phantump |
| 9 | 4% | Scatterbug (Savanna)–Vivillon (Savanna) | Espurr |
| 10 | 4% | Swirlix | Flabébé (Red)–Floette (Red) |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (Red)–Floette (Red) | Phantump |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Scatterbug (Savanna)–Spewpa (Savanna) | Phantump |
| 2 | 30% | Fletchling–Talonflame | Scatterbug (Savanna)–Spewpa (Savanna) |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Phantump | Fletchling–Talonflame |
| 5 | 1% | Scatterbug (Savanna)–Vivillon (Savanna) | Scatterbug (Savanna)–Vivillon (Savanna) |

#### Johto Safari: Top middle

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_TOP_MID_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Phantump | Phantump |
| 2 | 20% | Swirlix | Espurr |
| 3 | 10% | Flabébé (White)–Floette (White) | Phantump |
| 4 | 10% | Scatterbug (Elegant)–Vivillon (Elegant) | Flabébé (White)–Floette (White) |
| 5 | 10% | Dedenne | Espurr |
| 6 | 10% | Spritzee | Swirlix |
| 7 | 5% | Phantump | Phantump |
| 8 | 5% | Swirlix | Espurr |
| 9 | 4% | Scatterbug (Elegant)–Vivillon (Elegant) | Dedenne |
| 10 | 4% | Flabébé (White)–Floette (White) | Espurr |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Dedenne | Phantump |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Phantump | Scatterbug (Elegant)–Spewpa (Elegant) |
| 2 | 30% | Scatterbug (Elegant)–Spewpa (Elegant) | Phantump |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Fletchling–Talonflame | Fletchling–Talonflame |
| 5 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |

#### Johto Safari: Top right

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_TOP_RIGHT_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Spritzee | Phantump |
| 2 | 20% | Flabébé (Orange)–Floette (Orange) | Inkay–Malamar |
| 3 | 10% | Fletchling–Talonflame | Bunnelby–Diggersby |
| 4 | 10% | Scatterbug (Ocean)–Vivillon (Ocean) | Flabébé (Orange)–Floette (Orange) |
| 5 | 10% | Binacle–Barbaracle | Inkay–Malamar |
| 6 | 10% | Bunnelby–Diggersby | Espurr |
| 7 | 5% | Dedenne | Phantump |
| 8 | 5% | Flabébé (Orange)–Floette (Orange) | Spritzee |
| 9 | 4% | Scatterbug (Ocean)–Vivillon (Ocean) | Inkay–Malamar |
| 10 | 4% | Swirlix | Phantump |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Binacle–Barbaracle | Espurr |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Binacle–Barbaracle | Inkay–Malamar |
| 2 | 30% | Skrelp–Dragalge | Binacle–Barbaracle |
| 3 | 5% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 4 | 4% | Binacle–Barbaracle | Binacle–Barbaracle |
| 5 | 1% | Inkay–Malamar | Inkay–Malamar |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Binacle–Barbaracle | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Skrelp–Dragalge | Binacle–Barbaracle |
| 3 | 10% | 12% | 11% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 4 | 8% | 10% | 10% | Binacle–Barbaracle | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 6 | 4% | 7% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 7 | 3% | 6% | 10% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 8 | 3% | 5% | 9% | Binacle–Barbaracle | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Binacle–Barbaracle | Binacle–Barbaracle |
| 10 | 2% | 4% | 9% | Inkay–Malamar | Inkay–Malamar |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Fletchling–Talonflame | Phantump |
| 2 | 30% | Scatterbug (Ocean)–Spewpa (Ocean) | Scatterbug (Ocean)–Spewpa (Ocean) |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Phantump | Fletchling–Talonflame |
| 5 | 1% | Scatterbug (Ocean)–Vivillon (Ocean) | Scatterbug (Ocean)–Vivillon (Ocean) |

#### Hoenn Safari: South

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_SOUTH`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Helioptile | Pumpkaboo (Large) |
| 2 | 20% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 3 | 10% | Fletchling–Talonflame | Honedge–Doublade |
| 4 | 10% | Scatterbug (Garden)–Vivillon (Garden) | Pumpkaboo (Large) |
| 5 | 10% | Helioptile | Honedge–Doublade |
| 6 | 10% | Bunnelby–Diggersby | Klefki |
| 7 | 5% | Scatterbug (Garden)–Vivillon (Garden) | Pumpkaboo (Large) |
| 8 | 5% | Fletchling–Talonflame | Helioptile |
| 9 | 4% | Hawlucha | Klefki |
| 10 | 4% | Klefki | Honedge–Doublade |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Hawlucha | Pumpkaboo (Super) |

#### Hoenn Safari: Southwest

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_SOUTHWEST`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Goomy–Goodra | Goomy–Goodra |
| 2 | 20% | Helioptile | Pumpkaboo (Small) |
| 3 | 10% | Fletchling–Talonflame | Honedge–Doublade |
| 4 | 10% | Scatterbug (Monsoon)–Vivillon (Monsoon) | Pumpkaboo (Small) |
| 5 | 10% | Bunnelby–Diggersby | Goomy–Goodra |
| 6 | 10% | Goomy–Goodra | Honedge–Doublade |
| 7 | 5% | Helioptile | Pumpkaboo (Small) |
| 8 | 5% | Scatterbug (Monsoon)–Vivillon (Monsoon) | Honedge–Doublade |
| 9 | 4% | Bunnelby–Diggersby | Klefki |
| 10 | 4% | Klefki | Goomy–Goodra |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Goomy–Goodra | Pumpkaboo (Small) |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Skrelp–Dragalge | Inkay–Malamar |
| 2 | 30% | Binacle–Barbaracle | Skrelp–Dragalge |
| 3 | 5% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 4 | 4% | Skrelp–Dragalge | Skrelp–Dragalge |
| 5 | 1% | Inkay–Malamar | Inkay–Malamar |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Skrelp–Dragalge | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Binacle–Barbaracle | Skrelp–Dragalge |
| 3 | 10% | 12% | 11% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 4 | 8% | 10% | 10% | Skrelp–Dragalge | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 6 | 4% | 7% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 7 | 3% | 6% | 10% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 8 | 3% | 5% | 9% | Skrelp–Dragalge | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Skrelp–Dragalge | Skrelp–Dragalge |
| 10 | 2% | 4% | 9% | Inkay–Malamar | Inkay–Malamar |

#### Hoenn Safari: North

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_NORTH`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Hawlucha | Honedge–Doublade |
| 2 | 20% | Helioptile | Hawlucha |
| 3 | 10% | Bunnelby–Diggersby | Pumpkaboo (Average) |
| 4 | 10% | Scatterbug (Continental)–Vivillon (Continental) | Pumpkaboo (Small) |
| 5 | 10% | Fletchling–Talonflame | Klefki |
| 6 | 10% | Honedge–Doublade | Pumpkaboo (Average) |
| 7 | 5% | Hawlucha | Honedge–Doublade |
| 8 | 5% | Klefki | Pumpkaboo (Average) |
| 9 | 4% | Scatterbug (Continental)–Vivillon (Continental) | Hawlucha |
| 10 | 4% | Bunnelby–Diggersby | Klefki |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Hawlucha | Honedge–Doublade |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Helioptile | Honedge–Doublade |
| 2 | 30% | Honedge–Doublade | Helioptile |
| 3 | 5% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 4 | 4% | Klefki | Klefki |
| 5 | 1% | Hawlucha | Hawlucha |

#### Hoenn Safari: Northwest

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_NORTHWEST`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bunnelby–Diggersby | Goomy–Goodra |
| 2 | 20% | Goomy–Goodra | Pumpkaboo (Small) |
| 3 | 10% | Hawlucha | Honedge–Doublade |
| 4 | 10% | Scatterbug (Jungle)–Vivillon (Jungle) | Pumpkaboo (Small) |
| 5 | 10% | Fletchling–Talonflame | Goomy–Goodra |
| 6 | 10% | Helioptile | Hawlucha |
| 7 | 5% | Goomy–Goodra | Pumpkaboo (Small) |
| 8 | 5% | Scatterbug (Jungle)–Vivillon (Jungle) | Honedge–Doublade |
| 9 | 4% | Hawlucha | Klefki |
| 10 | 4% | Klefki | Goomy–Goodra |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Bunnelby–Diggersby | Pumpkaboo (Super) |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 30% | Skrelp–Dragalge | Clauncher–Clawitzer |
| 3 | 5% | Binacle–Barbaracle | Binacle–Barbaracle |
| 4 | 4% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 5 | 1% | Inkay–Malamar | Inkay–Malamar |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Skrelp–Dragalge | Clauncher–Clawitzer |
| 3 | 10% | 12% | 11% | Binacle–Barbaracle | Binacle–Barbaracle |
| 4 | 8% | 10% | 10% | Clauncher–Clawitzer | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 6 | 4% | 7% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 7 | 3% | 6% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 8 | 3% | 5% | 9% | Clauncher–Clawitzer | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 10 | 2% | 4% | 9% | Inkay–Malamar | Inkay–Malamar |

#### Hoenn Safari: Southeast

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_SOUTHEAST`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Fletchling–Talonflame | Bunnelby–Diggersby |
| 2 | 20% | Bunnelby–Diggersby | Inkay–Malamar |
| 3 | 10% | Helioptile | Pumpkaboo (Average) |
| 4 | 10% | Scatterbug (Archipelago)–Vivillon (Archipelago) | Honedge–Doublade |
| 5 | 10% | Binacle–Barbaracle | Inkay–Malamar |
| 6 | 10% | Klefki | Pumpkaboo (Average) |
| 7 | 5% | Scatterbug (Archipelago)–Vivillon (Archipelago) | Klefki |
| 8 | 5% | Helioptile | Inkay–Malamar |
| 9 | 4% | Goomy–Goodra | Pumpkaboo (Average) |
| 10 | 4% | Binacle–Barbaracle | Honedge–Doublade |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Hawlucha | Pumpkaboo (Super) |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Binacle–Barbaracle | Inkay–Malamar |
| 2 | 30% | Clauncher–Clawitzer | Binacle–Barbaracle |
| 3 | 5% | Skrelp–Dragalge | Skrelp–Dragalge |
| 4 | 4% | Binacle–Barbaracle | Binacle–Barbaracle |
| 5 | 1% | Inkay–Malamar | Inkay–Malamar |

*Fishing*

| Entry | Old Rod | Good Rod | Super Rod | Day | Night |
| --- | --- | --- | --- | --- | --- |
| 1 | 38% | 25% | 12% | Binacle–Barbaracle | Inkay–Malamar |
| 2 | 22% | 18% | 10% | Clauncher–Clawitzer | Binacle–Barbaracle |
| 3 | 10% | 12% | 11% | Skrelp–Dragalge | Skrelp–Dragalge |
| 4 | 8% | 10% | 10% | Binacle–Barbaracle | Inkay–Malamar |
| 5 | 8% | 9% | 10% | Binacle–Barbaracle | Binacle–Barbaracle |
| 6 | 4% | 7% | 10% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 7 | 3% | 6% | 10% | Skrelp–Dragalge | Skrelp–Dragalge |
| 8 | 3% | 5% | 9% | Binacle–Barbaracle | Inkay–Malamar |
| 9 | 2% | 4% | 9% | Binacle–Barbaracle | Binacle–Barbaracle |
| 10 | 2% | 4% | 9% | Inkay–Malamar | Inkay–Malamar |

#### Hoenn Safari: Northeast

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_NORTHEAST`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Klefki | Honedge–Doublade |
| 2 | 20% | Honedge–Doublade | Klefki |
| 3 | 10% | Bunnelby–Diggersby | Pumpkaboo (Average) |
| 4 | 10% | Scatterbug (Sun)–Vivillon (Sun) | Pumpkaboo (Large) |
| 5 | 10% | Fletchling–Talonflame | Honedge–Doublade |
| 6 | 10% | Helioptile | Pumpkaboo (Average) |
| 7 | 5% | Klefki | Klefki |
| 8 | 5% | Scatterbug (Sun)–Vivillon (Sun) | Pumpkaboo (Large) |
| 9 | 4% | Hawlucha | Hawlucha |
| 10 | 4% | Honedge–Doublade | Honedge–Doublade |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Klefki | Pumpkaboo (Super) |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Honedge–Doublade | Klefki |
| 2 | 30% | Helioptile | Honedge–Doublade |
| 3 | 5% | Klefki | Helioptile |
| 4 | 4% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 5 | 1% | Hawlucha | Hawlucha |

#### Bug-Catching Contest: Tuesday

Wilds, Bug-Catching Contest.

**`MAP_NATIONAL_PARK_BUG_CONTEST_HNS:TUESDAY`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Caterpie–Butterfree | Venonat–Venomoth |
| 2 | 20% | Weedle–Beedrill | Spinarak–Ariados |
| 3 | 10% | Ledyba–Ledian | Paras–Parasect |
| 4 | 10% | Paras–Parasect | Pineco–Forretress |
| 5 | 10% | Yanma | Venonat–Venomoth |
| 6 | 10% | Scatterbug (Modern)–Vivillon (Modern) | Spinarak–Ariados |
| 7 | 5% | Scyther | Scyther |
| 8 | 5% | Pinsir | Pinsir |
| 9 | 4% | Caterpie–Butterfree | Pineco–Forretress |
| 10 | 4% | Heracross | Heracross |
| 11 | 1% | Scyther | Scyther |
| 12 | 1% | Pinsir | Scatterbug (Modern)–Vivillon (Modern) |

#### Bug-Catching Contest: Thursday

Wilds, Bug-Catching Contest.

**`MAP_NATIONAL_PARK_BUG_CONTEST_HNS:THURSDAY`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Wurmple–Beautifly | Volbeat |
| 2 | 20% | Nincada–Ninjask | Illumise |
| 3 | 10% | Surskit–Masquerain | Wurmple–Dustox |
| 4 | 10% | Sewaddle–Swadloon | Joltik–Galvantula |
| 5 | 10% | Dwebble–Crustle | Shelmet |
| 6 | 10% | Scatterbug (Polar)–Vivillon (Polar) | Venipede–Scolipede |
| 7 | 5% | Venipede–Scolipede | Volbeat |
| 8 | 5% | Karrablast | Illumise |
| 9 | 4% | Wurmple–Beautifly | Joltik–Galvantula |
| 10 | 4% | Nincada–Ninjask | Shelmet |
| 11 | 1% | Sewaddle–Swadloon | Nincada–Ninjask |
| 12 | 1% | Scatterbug (Polar)–Vivillon (Polar) | Karrablast |

#### Bug-Catching Contest: Saturday

Wilds, Bug-Catching Contest.

**`MAP_NATIONAL_PARK_BUG_CONTEST_HNS:SATURDAY`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Cutiefly–Ribombee | Dewpider–Araquanid |
| 2 | 20% | Blipbug–Orbeetle | Sizzlipede–Centiskorch |
| 3 | 10% | Grubbin–Charjabug | Snom |
| 4 | 10% | Scatterbug (Tundra)–Vivillon (Tundra) | Blipbug–Orbeetle |
| 5 | 10% | Cutiefly–Ribombee | Dewpider–Araquanid |
| 6 | 10% | Blipbug–Orbeetle | Sizzlipede–Centiskorch |
| 7 | 5% | Grubbin–Charjabug | Grubbin–Charjabug |
| 8 | 5% | Scatterbug (Tundra)–Vivillon (Tundra) | Snom |
| 9 | 4% | Cutiefly–Ribombee | Dewpider–Araquanid |
| 10 | 4% | Grubbin–Charjabug | Sizzlipede–Centiskorch |
| 11 | 1% | Scatterbug (Tundra)–Spewpa (Tundra) | Snom |
| 12 | 1% | Blipbug–Orbeetle | Grubbin–Charjabug |


### Coverage checklist

| Species | Catchable at |
| --- | --- |
| Araquanid | Bug-Catching Contest: Saturday |
| Ariados | Bug-Catching Contest: Tuesday |
| Avalugg | Kanto Safari: Cave, Kanto Safari: Mountain |
| Barbaracle | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Beautifly | Bug-Catching Contest: Thursday |
| Beedrill | Bug-Catching Contest: Tuesday |
| Bergmite | Kanto Safari: Cave, Kanto Safari: Mountain |
| Binacle | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Blipbug | Bug-Catching Contest: Saturday |
| Braixen | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Bunnelby | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Butterfree | Bug-Catching Contest: Tuesday |
| Carbink | Kanto Safari: Cave, Kanto Safari: Mountain |
| Cascoon | Bug-Catching Contest: Thursday |
| Caterpie | Bug-Catching Contest: Tuesday |
| Centiskorch | Bug-Catching Contest: Saturday |
| Charjabug | Bug-Catching Contest: Saturday |
| Chesnaught | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Chespin | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Clauncher | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Clawitzer | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Crustle | Bug-Catching Contest: Thursday |
| Cutiefly | Bug-Catching Contest: Saturday |
| Dedenne | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Delphox | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Dewpider | Bug-Catching Contest: Saturday |
| Diggersby | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Dottler | Bug-Catching Contest: Saturday |
| Doublade | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Dragalge | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Dustox | Bug-Catching Contest: Thursday |
| Dwebble | Bug-Catching Contest: Thursday |
| Espurr | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Fennekin | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Flabébé (Blue) | Johto Safari: Low right |
| Flabébé (Orange) | Johto Safari: Top right |
| Flabébé (Red) | Johto Safari: Top left |
| Flabébé (White) | Johto Safari: Top middle |
| Flabébé (Yellow) | Johto Safari: Low middle |
| Fletchinder | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Fletchling | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Floette (Blue) | Johto Safari: Low right |
| Floette (Orange) | Johto Safari: Top right |
| Floette (Red) | Johto Safari: Top left |
| Floette (White) | Johto Safari: Top middle |
| Floette (Yellow) | Johto Safari: Low middle |
| Forretress | Bug-Catching Contest: Tuesday |
| Froakie | Kanto Safari: Beach, Kanto Safari: Mountain |
| Frogadier | Kanto Safari: Beach, Kanto Safari: Mountain |
| Furfrou | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Galvantula | Bug-Catching Contest: Thursday |
| Gogoat | Kanto Safari: Brush, Kanto Safari: Mountain |
| Goodra | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest |
| Goomy | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest |
| Greninja | Kanto Safari: Beach, Kanto Safari: Mountain |
| Grubbin | Bug-Catching Contest: Saturday |
| Hawlucha | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Helioptile | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Heracross | Bug-Catching Contest: Tuesday |
| Honedge | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Illumise | Bug-Catching Contest: Thursday |
| Inkay | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Joltik | Bug-Catching Contest: Thursday |
| Kakuna | Bug-Catching Contest: Tuesday |
| Karrablast | Bug-Catching Contest: Thursday |
| Klefki | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Ledian | Bug-Catching Contest: Tuesday |
| Ledyba | Bug-Catching Contest: Tuesday |
| Litleo | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Malamar | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Masquerain | Bug-Catching Contest: Thursday |
| Metapod | Bug-Catching Contest: Tuesday |
| Nincada | Bug-Catching Contest: Thursday |
| Ninjask | Bug-Catching Contest: Thursday |
| Noibat | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Cave, Kanto Safari: Mountain |
| Noivern | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Cave, Kanto Safari: Mountain |
| Orbeetle | Bug-Catching Contest: Saturday |
| Pancham | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Pangoro | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Paras | Bug-Catching Contest: Tuesday |
| Parasect | Bug-Catching Contest: Tuesday |
| Phantump | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Pineco | Bug-Catching Contest: Tuesday |
| Pinsir | Bug-Catching Contest: Tuesday |
| Pumpkaboo (Average) | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Southeast |
| Pumpkaboo (Large) | Hoenn Safari: Northeast, Hoenn Safari: South |
| Pumpkaboo (Small) | Hoenn Safari: North, Hoenn Safari: Northwest, Hoenn Safari: Southwest |
| Pumpkaboo (Super) | Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South, Hoenn Safari: Southeast |
| Pyroar | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Quilladin | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Ribombee | Bug-Catching Contest: Saturday |
| Scatterbug (Archipelago) | Hoenn Safari: Southeast |
| Scatterbug (Continental) | Hoenn Safari: North |
| Scatterbug (Elegant) | Johto Safari: Top middle |
| Scatterbug (Garden) | Hoenn Safari: South |
| Scatterbug (High Plains) | Johto Safari: Low middle |
| Scatterbug (Icy Snow) | Kanto Safari: Mountain |
| Scatterbug (Jungle) | Hoenn Safari: Northwest |
| Scatterbug (Marine) | Kanto Safari: Beach |
| Scatterbug (Meadow) | Kanto Safari: Brush |
| Scatterbug (Modern) | Bug-Catching Contest: Tuesday |
| Scatterbug (Monsoon) | Hoenn Safari: Southwest |
| Scatterbug (Ocean) | Johto Safari: Top right |
| Scatterbug (Polar) | Bug-Catching Contest: Thursday |
| Scatterbug (River) | Johto Safari: Low right |
| Scatterbug (Sandstorm) | Johto Safari: Low left |
| Scatterbug (Savanna) | Johto Safari: Top left |
| Scatterbug (Sun) | Hoenn Safari: Northeast |
| Scatterbug (Tundra) | Bug-Catching Contest: Saturday |
| Scolipede | Bug-Catching Contest: Thursday |
| Scyther | Bug-Catching Contest: Tuesday |
| Sewaddle | Bug-Catching Contest: Thursday |
| Shelmet | Bug-Catching Contest: Thursday |
| Silcoon | Bug-Catching Contest: Thursday |
| Sizzlipede | Bug-Catching Contest: Saturday |
| Skiddo | Kanto Safari: Brush, Kanto Safari: Mountain |
| Skrelp | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Sliggoo | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest |
| Snom | Bug-Catching Contest: Saturday |
| Spewpa (Archipelago) | Hoenn Safari: Southeast |
| Spewpa (Continental) | Hoenn Safari: North |
| Spewpa (Elegant) | Johto Safari: Top middle |
| Spewpa (Garden) | Hoenn Safari: South |
| Spewpa (High Plains) | Johto Safari: Low middle |
| Spewpa (Icy Snow) | Kanto Safari: Mountain |
| Spewpa (Jungle) | Hoenn Safari: Northwest |
| Spewpa (Marine) | Kanto Safari: Beach |
| Spewpa (Meadow) | Kanto Safari: Brush |
| Spewpa (Modern) | Bug-Catching Contest: Tuesday |
| Spewpa (Monsoon) | Hoenn Safari: Southwest |
| Spewpa (Ocean) | Johto Safari: Top right |
| Spewpa (Polar) | Bug-Catching Contest: Thursday |
| Spewpa (River) | Johto Safari: Low right |
| Spewpa (Sandstorm) | Johto Safari: Low left |
| Spewpa (Savanna) | Johto Safari: Top left |
| Spewpa (Sun) | Hoenn Safari: Northeast |
| Spewpa (Tundra) | Bug-Catching Contest: Saturday |
| Spinarak | Bug-Catching Contest: Tuesday |
| Spritzee | Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left, Johto Safari: Top middle and more |
| Surskit | Bug-Catching Contest: Thursday |
| Swadloon | Bug-Catching Contest: Thursday |
| Swirlix | Johto Safari: Low right, Johto Safari: Top left, Johto Safari: Top middle, Johto Safari: Top right |
| Talonflame | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Venipede | Bug-Catching Contest: Thursday |
| Venomoth | Bug-Catching Contest: Tuesday |
| Venonat | Bug-Catching Contest: Tuesday |
| Vivillon (Archipelago) | Hoenn Safari: Southeast |
| Vivillon (Continental) | Hoenn Safari: North |
| Vivillon (Elegant) | Johto Safari: Top middle |
| Vivillon (Garden) | Hoenn Safari: South |
| Vivillon (High Plains) | Johto Safari: Low middle |
| Vivillon (Icy Snow) | Kanto Safari: Mountain |
| Vivillon (Jungle) | Hoenn Safari: Northwest |
| Vivillon (Marine) | Kanto Safari: Beach |
| Vivillon (Meadow) | Kanto Safari: Brush |
| Vivillon (Modern) | Bug-Catching Contest: Tuesday |
| Vivillon (Monsoon) | Hoenn Safari: Southwest |
| Vivillon (Ocean) | Johto Safari: Top right |
| Vivillon (Polar) | Bug-Catching Contest: Thursday |
| Vivillon (River) | Johto Safari: Low right |
| Vivillon (Sandstorm) | Johto Safari: Low left |
| Vivillon (Savanna) | Johto Safari: Top left |
| Vivillon (Sun) | Hoenn Safari: Northeast |
| Vivillon (Tundra) | Bug-Catching Contest: Saturday |
| Volbeat | Bug-Catching Contest: Thursday |
| Weedle | Bug-Catching Contest: Tuesday |
| Whirlipede | Bug-Catching Contest: Thursday |
| Wurmple | Bug-Catching Contest: Thursday |
| Yanma | Bug-Catching Contest: Tuesday |
