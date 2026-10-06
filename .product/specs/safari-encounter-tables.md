# Safari encounter tables

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: v0 approved. These tables follow the
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
| 1 | 20% | Binacle–Barbaracle | Inkay–Malamar |
| 2 | 20% | Scatterbug (Marine)–Vivillon (Marine) | Binacle–Barbaracle |
| 3 | 10% | Bunnelby–Diggersby | Pancham–Pangoro |
| 4 | 10% | Fletchling–Talonflame | Bunnelby–Diggersby |
| 5 | 10% | Litleo–Pyroar | Noibat–Noivern |
| 6 | 10% | Skiddo–Gogoat | Inkay–Malamar |
| 7 | 5% | Binacle–Barbaracle | Pancham–Pangoro |
| 8 | 5% | Scatterbug (Marine)–Vivillon (Marine) | Binacle–Barbaracle |
| 9 | 4% | Furfrou | Litleo–Pyroar |
| 10 | 4% | Bunnelby–Diggersby | Furfrou |
| 11 | 1% | Froakie–Greninja | Froakie–Greninja |
| 12 | 1% | Scatterbug (Meadow)–Vivillon (Meadow) | Pancham–Pangoro |

*Surfing*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Clauncher–Clawitzer | Inkay–Malamar |
| 2 | 30% | Binacle–Barbaracle | Clauncher–Clawitzer |
| 3 | 5% | Skrelp–Dragalge | Skrelp–Dragalge |
| 4 | 4% | Clauncher–Clawitzer | Clauncher–Clawitzer |
| 5 | 1% | Inkay–Malamar | Inkay–Malamar |

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
| 10 | 2% | 4% | 9% | Inkay–Malamar | Inkay–Malamar |

#### Kanto Safari: Brush

Wilds, Kanto Safari.

**`MAP_FUCHSIA_CITY_SAFARI_ZONE_BRUSH_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Litleo–Pyroar | Pancham–Pangoro |
| 2 | 20% | Scatterbug (Meadow)–Vivillon (Meadow) | Litleo–Pyroar |
| 3 | 10% | Bunnelby–Diggersby | Pancham–Pangoro |
| 4 | 10% | Skiddo–Gogoat | Bunnelby–Diggersby |
| 5 | 10% | Fletchling–Talonflame | Noibat–Noivern |
| 6 | 10% | Litleo–Pyroar | Skiddo–Gogoat |
| 7 | 5% | Furfrou | Litleo–Pyroar |
| 8 | 5% | Scatterbug (Meadow)–Vivillon (Meadow) | Pancham–Pangoro |
| 9 | 4% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 10 | 4% | Skiddo–Gogoat | Furfrou |
| 11 | 1% | Scatterbug (Marine)–Vivillon (Marine) | Pancham–Pangoro |
| 12 | 1% | Furfrou | Skiddo–Gogoat |

#### Kanto Safari: Cave

Wilds, Kanto Safari.

**`MAP_FUCHSIA_CITY_SAFARI_ZONE_CAVE_HNS`**

Water type: cave water.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Bergmite–Avalugg | Bergmite–Avalugg |
| 2 | 20% | Bunnelby–Diggersby | Inkay–Malamar |
| 3 | 10% | Binacle–Barbaracle | Bunnelby–Diggersby |
| 4 | 10% | Bergmite–Avalugg | Bergmite–Avalugg |
| 5 | 10% | Carbink | Noibat–Noivern |
| 6 | 10% | Bunnelby–Diggersby | Binacle–Barbaracle |
| 7 | 5% | Noibat–Noivern | Inkay–Malamar |
| 8 | 5% | Bergmite–Avalugg | Carbink |
| 9 | 4% | Binacle–Barbaracle | Bergmite–Avalugg |
| 10 | 4% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 11 | 1% | Noibat–Noivern | Carbink |
| 12 | 1% | Bergmite | Bergmite |

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

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Skiddo–Gogoat | Skiddo–Gogoat |
| 2 | 20% | Scatterbug (Icy Snow)–Vivillon (Icy Snow) | Pancham–Pangoro |
| 3 | 10% | Fletchling–Talonflame | Noibat–Noivern |
| 4 | 10% | Litleo–Pyroar | Litleo–Pyroar |
| 5 | 10% | Bunnelby–Diggersby | Pancham–Pangoro |
| 6 | 10% | Skiddo–Gogoat | Bunnelby–Diggersby |
| 7 | 5% | Carbink | Skiddo–Gogoat |
| 8 | 5% | Scatterbug (Icy Snow)–Vivillon (Icy Snow) | Pancham–Pangoro |
| 9 | 4% | Litleo–Pyroar | Carbink |
| 10 | 4% | Furfrou | Litleo–Pyroar |
| 11 | 1% | Scatterbug (Meadow)–Vivillon (Meadow) | Pancham–Pangoro |
| 12 | 1% | Carbink | Skiddo–Gogoat |

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

#### Johto Safari: Low left

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_LOW_LEFT_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Scatterbug (Sandstorm)–Vivillon (Sandstorm) | Espurr |
| 2 | 20% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 3 | 10% | Fletchling–Talonflame | Espurr |
| 4 | 10% | Bunnelby–Diggersby | Phantump |
| 5 | 10% | Dedenne | Bunnelby–Diggersby |
| 6 | 10% | Scatterbug (Sandstorm)–Vivillon (Sandstorm) | Espurr |
| 7 | 5% | Fletchling–Talonflame | Dedenne |
| 8 | 5% | Spritzee | Espurr |
| 9 | 4% | Bunnelby–Diggersby | Spritzee |
| 10 | 4% | Scatterbug (Sandstorm)–Vivillon (Sandstorm) | Bunnelby–Diggersby |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (Yellow)–Floette (Yellow) | Espurr |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Fletchling–Talonflame | Phantump |
| 2 | 30% | Scatterbug (Sandstorm)–Spewpa (Sandstorm) | Espurr |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Phantump | Swirlix |
| 5 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |

#### Johto Safari: Low middle

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_LOW_MID_HNS`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Flabébé (Yellow)–Floette (Yellow) | Espurr |
| 2 | 20% | Scatterbug (High Plains)–Vivillon (High Plains) | Flabébé (Yellow)–Floette (Yellow) |
| 3 | 10% | Bunnelby–Diggersby | Espurr |
| 4 | 10% | Spritzee | Spritzee |
| 5 | 10% | Fletchling–Talonflame | Bunnelby–Diggersby |
| 6 | 10% | Flabébé (Yellow)–Floette (Yellow) | Espurr |
| 7 | 5% | Spritzee | Swirlix |
| 8 | 5% | Scatterbug (High Plains)–Vivillon (High Plains) | Swirlix |
| 9 | 4% | Swirlix | Espurr |
| 10 | 4% | Bunnelby–Diggersby | Flabébé (Yellow)–Floette (Yellow) |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (Blue)–Floette (Blue) | Espurr |

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
| 2 | 20% | Scatterbug (River)–Vivillon (River) | Flabébé (Blue)–Floette (Blue) |
| 3 | 10% | Bunnelby–Diggersby | Espurr |
| 4 | 10% | Spritzee | Phantump |
| 5 | 10% | Fletchling–Talonflame | Spritzee |
| 6 | 10% | Swirlix | Espurr |
| 7 | 5% | Flabébé (Blue)–Floette (Blue) | Swirlix |
| 8 | 5% | Dedenne | Dedenne |
| 9 | 4% | Scatterbug (River)–Vivillon (River) | Phantump |
| 10 | 4% | Bunnelby–Diggersby | Espurr |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (Yellow)–Floette (Yellow) | Flabébé (Blue)–Floette (Blue) |

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
| 2 | 30% | Scatterbug (River)–Spewpa (River) | Espurr |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Phantump | Swirlix |
| 5 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |

#### Johto Safari: Top left

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_TOP_LEFT_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Flabébé (Red)–Floette (Red) | Flabébé (Red)–Floette (Red) |
| 2 | 20% | Scatterbug (Savanna)–Vivillon (Savanna) | Phantump |
| 3 | 10% | Fletchling–Talonflame | Espurr |
| 4 | 10% | Swirlix | Swirlix |
| 5 | 10% | Spritzee | Phantump |
| 6 | 10% | Bunnelby–Diggersby | Spritzee |
| 7 | 5% | Flabébé (Red)–Floette (Red) | Espurr |
| 8 | 5% | Dedenne | Dedenne |
| 9 | 4% | Scatterbug (Savanna)–Vivillon (Savanna) | Phantump |
| 10 | 4% | Swirlix | Espurr |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (White)–Floette (White) | Flabébé (Red)–Floette (Red) |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Scatterbug (Savanna)–Spewpa (Savanna) | Phantump |
| 2 | 30% | Fletchling–Talonflame | Espurr |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Phantump | Swirlix |
| 5 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |

#### Johto Safari: Top middle

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_TOP_MID_HNS`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Phantump | Phantump |
| 2 | 20% | Scatterbug (Elegant)–Vivillon (Elegant) | Espurr |
| 3 | 10% | Flabébé (White)–Floette (White) | Phantump |
| 4 | 10% | Swirlix | Flabébé (White)–Floette (White) |
| 5 | 10% | Spritzee | Espurr |
| 6 | 10% | Phantump | Swirlix |
| 7 | 5% | Spritzee | Phantump |
| 8 | 5% | Flabébé (White)–Floette (White) | Espurr |
| 9 | 4% | Scatterbug (Elegant)–Vivillon (Elegant) | Swirlix |
| 10 | 4% | Swirlix | Espurr |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (Orange)–Floette (Orange) | Phantump |

*Trees and rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Phantump | Espurr |
| 2 | 30% | Scatterbug (Elegant)–Spewpa (Elegant) | Phantump |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Fletchling–Talonflame | Swirlix |
| 5 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |

#### Johto Safari: Top right

Wilds, Johto Safari.

**`MAP_SAFARI_ZONE_TOP_RIGHT_HNS`**

Water type: coast and sea.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Flabébé (Orange)–Floette (Orange) | Inkay–Malamar |
| 2 | 20% | Scatterbug (Ocean)–Vivillon (Ocean) | Flabébé (Orange)–Floette (Orange) |
| 3 | 10% | Binacle–Barbaracle | Espurr |
| 4 | 10% | Fletchling–Talonflame | Binacle–Barbaracle |
| 5 | 10% | Spritzee | Inkay–Malamar |
| 6 | 10% | Bunnelby–Diggersby | Phantump |
| 7 | 5% | Flabébé (Orange)–Floette (Orange) | Espurr |
| 8 | 5% | Dedenne | Dedenne |
| 9 | 4% | Scatterbug (Ocean)–Vivillon (Ocean) | Spritzee |
| 10 | 4% | Binacle–Barbaracle | Inkay–Malamar |
| 11 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |
| 12 | 1% | Flabébé (Red)–Floette (Red) | Espurr |

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
| 2 | 30% | Scatterbug (Ocean)–Spewpa (Ocean) | Espurr |
| 3 | 5% | Dedenne | Dedenne |
| 4 | 4% | Phantump | Swirlix |
| 5 | 1% | Chespin–Chesnaught | Chespin–Chesnaught |

#### Hoenn Safari: South

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_SOUTH`**

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Helioptile | Pumpkaboo (Large) |
| 2 | 20% | Scatterbug (Garden)–Vivillon (Garden) | Bunnelby–Diggersby |
| 3 | 10% | Bunnelby–Diggersby | Pumpkaboo (Large) |
| 4 | 10% | Fletchling–Talonflame | Honedge–Doublade |
| 5 | 10% | Helioptile | Bunnelby–Diggersby |
| 6 | 10% | Hawlucha | Pumpkaboo (Large) |
| 7 | 5% | Scatterbug (Garden)–Vivillon (Garden) | Klefki |
| 8 | 5% | Bunnelby–Diggersby | Pumpkaboo (Large) |
| 9 | 4% | Fletchling–Talonflame | Bunnelby–Diggersby |
| 10 | 4% | Klefki | Klefki |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Scatterbug (Monsoon)–Vivillon (Monsoon) | Pumpkaboo (Super) |

#### Hoenn Safari: Southwest

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_SOUTHWEST`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Scatterbug (Monsoon)–Vivillon (Monsoon) | Pumpkaboo (Small) |
| 2 | 20% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 3 | 10% | Fletchling–Talonflame | Pumpkaboo (Small) |
| 4 | 10% | Scatterbug (Monsoon)–Vivillon (Monsoon) | Goomy–Goodra |
| 5 | 10% | Goomy–Goodra | Honedge–Doublade |
| 6 | 10% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 7 | 5% | Fletchling–Talonflame | Pumpkaboo (Small) |
| 8 | 5% | Scatterbug (Monsoon)–Vivillon (Monsoon) | Klefki |
| 9 | 4% | Klefki | Bunnelby–Diggersby |
| 10 | 4% | Bunnelby–Diggersby | Pumpkaboo (Small) |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Scatterbug (Jungle)–Vivillon (Jungle) | Pumpkaboo (Small) |

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
| 1 | 20% | Helioptile | Pumpkaboo (Average) |
| 2 | 20% | Scatterbug (Continental)–Vivillon (Continental) | Bunnelby–Diggersby |
| 3 | 10% | Bunnelby–Diggersby | Pumpkaboo (Small) |
| 4 | 10% | Helioptile | Honedge–Doublade |
| 5 | 10% | Fletchling–Talonflame | Bunnelby–Diggersby |
| 6 | 10% | Hawlucha | Pumpkaboo (Average) |
| 7 | 5% | Scatterbug (Continental)–Vivillon (Continental) | Klefki |
| 8 | 5% | Bunnelby–Diggersby | Pumpkaboo (Average) |
| 9 | 4% | Honedge–Doublade | Bunnelby–Diggersby |
| 10 | 4% | Helioptile | Pumpkaboo (Small) |
| 11 | 1% | Klefki | Fennekin–Delphox |
| 12 | 1% | Scatterbug (Sun)–Vivillon (Sun) | Pumpkaboo (Average) |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Helioptile | Bunnelby–Diggersby |
| 2 | 30% | Bunnelby–Diggersby | Pumpkaboo (Average) |
| 3 | 5% | Honedge–Doublade | Honedge–Doublade |
| 4 | 4% | Klefki | Klefki |
| 5 | 1% | Hawlucha | Pumpkaboo (Small) |

#### Hoenn Safari: Northwest

Wilds, Hoenn Safari.

**`MAP_SAFARI_ZONE_NORTHWEST`**

Water type: ponds and rivers.

*Land*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 20% | Scatterbug (Jungle)–Vivillon (Jungle) | Pumpkaboo (Small) |
| 2 | 20% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 3 | 10% | Fletchling–Talonflame | Pumpkaboo (Small) |
| 4 | 10% | Scatterbug (Jungle)–Vivillon (Jungle) | Honedge–Doublade |
| 5 | 10% | Hawlucha | Bunnelby–Diggersby |
| 6 | 10% | Fletchling–Talonflame | Pumpkaboo (Small) |
| 7 | 5% | Bunnelby–Diggersby | Goomy–Goodra |
| 8 | 5% | Scatterbug (Jungle)–Vivillon (Jungle) | Pumpkaboo (Small) |
| 9 | 4% | Goomy–Goodra | Klefki |
| 10 | 4% | Bunnelby–Diggersby | Bunnelby–Diggersby |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Scatterbug (Monsoon)–Vivillon (Monsoon) | Pumpkaboo (Super) |

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
| 1 | 20% | Scatterbug (Archipelago)–Vivillon (Archipelago) | Inkay–Malamar |
| 2 | 20% | Binacle–Barbaracle | Pumpkaboo (Average) |
| 3 | 10% | Helioptile | Binacle–Barbaracle |
| 4 | 10% | Fletchling–Talonflame | Inkay–Malamar |
| 5 | 10% | Bunnelby–Diggersby | Honedge–Doublade |
| 6 | 10% | Helioptile | Pumpkaboo (Average) |
| 7 | 5% | Binacle–Barbaracle | Bunnelby–Diggersby |
| 8 | 5% | Scatterbug (Archipelago)–Vivillon (Archipelago) | Klefki |
| 9 | 4% | Hawlucha | Pumpkaboo (Average) |
| 10 | 4% | Goomy–Goodra | Bunnelby–Diggersby |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Scatterbug (Garden)–Vivillon (Garden) | Pumpkaboo (Super) |

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
| 1 | 20% | Helioptile | Pumpkaboo (Average) |
| 2 | 20% | Scatterbug (Sun)–Vivillon (Sun) | Bunnelby–Diggersby |
| 3 | 10% | Bunnelby–Diggersby | Pumpkaboo (Large) |
| 4 | 10% | Fletchling–Talonflame | Honedge–Doublade |
| 5 | 10% | Helioptile | Pumpkaboo (Average) |
| 6 | 10% | Klefki | Bunnelby–Diggersby |
| 7 | 5% | Scatterbug (Sun)–Vivillon (Sun) | Klefki |
| 8 | 5% | Bunnelby–Diggersby | Pumpkaboo (Large) |
| 9 | 4% | Honedge–Doublade | Bunnelby–Diggersby |
| 10 | 4% | Fletchling–Talonflame | Pumpkaboo (Average) |
| 11 | 1% | Fennekin–Delphox | Fennekin–Delphox |
| 12 | 1% | Scatterbug (Continental)–Vivillon (Continental) | Pumpkaboo (Super) |

*Rock Smash rocks*

| Slot | Weight | Day | Night |
| --- | --- | --- | --- |
| 1 | 60% | Helioptile | Bunnelby–Diggersby |
| 2 | 30% | Bunnelby–Diggersby | Pumpkaboo (Average) |
| 3 | 5% | Klefki | Honedge–Doublade |
| 4 | 4% | Honedge–Doublade | Klefki |
| 5 | 1% | Hawlucha | Pumpkaboo (Large) |

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
| 8 | 5% | Scatterbug (Modern)–Vivillon (Modern) | Pinsir |
| 9 | 4% | Caterpie–Butterfree | Pineco–Forretress |
| 10 | 4% | Heracross | Heracross |
| 11 | 1% | Scyther | Scyther |
| 12 | 1% | Pinsir | Pinsir |

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
| 8 | 5% | Scatterbug (Polar)–Vivillon (Polar) | Illumise |
| 9 | 4% | Wurmple–Beautifly | Joltik–Galvantula |
| 10 | 4% | Durant | Shelmet |
| 11 | 1% | Sewaddle–Swadloon | Nincada–Ninjask |
| 12 | 1% | Karrablast | Karrablast |

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
| Avalugg | Kanto Safari: Cave |
| Barbaracle | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Beautifly | Bug-Catching Contest: Thursday |
| Beedrill | Bug-Catching Contest: Tuesday |
| Bergmite | Kanto Safari: Cave |
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
| Dedenne | Johto Safari: Low left, Johto Safari: Low right, Johto Safari: Top left, Johto Safari: Top middle and more |
| Delphox | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Dewpider | Bug-Catching Contest: Saturday |
| Diggersby | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Dottler | Bug-Catching Contest: Saturday |
| Doublade | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Dragalge | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Durant | Bug-Catching Contest: Thursday |
| Dustox | Bug-Catching Contest: Thursday |
| Dwebble | Bug-Catching Contest: Thursday |
| Espurr | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Fennekin | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Flabébé (Blue) | Johto Safari: Low middle, Johto Safari: Low right |
| Flabébé (Orange) | Johto Safari: Top middle, Johto Safari: Top right |
| Flabébé (Red) | Johto Safari: Top left, Johto Safari: Top right |
| Flabébé (White) | Johto Safari: Top left, Johto Safari: Top middle |
| Flabébé (Yellow) | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right |
| Fletchinder | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Fletchling | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Floette (Blue) | Johto Safari: Low middle, Johto Safari: Low right |
| Floette (Orange) | Johto Safari: Top middle, Johto Safari: Top right |
| Floette (Red) | Johto Safari: Top left, Johto Safari: Top right |
| Floette (White) | Johto Safari: Top left, Johto Safari: Top middle |
| Floette (Yellow) | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right |
| Forretress | Bug-Catching Contest: Tuesday |
| Froakie | Kanto Safari: Beach, Kanto Safari: Mountain |
| Frogadier | Kanto Safari: Beach, Kanto Safari: Mountain |
| Furfrou | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Galvantula | Bug-Catching Contest: Thursday |
| Gogoat | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Goodra | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest |
| Goomy | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest |
| Greninja | Kanto Safari: Beach, Kanto Safari: Mountain |
| Grubbin | Bug-Catching Contest: Saturday |
| Hawlucha | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Helioptile | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: South, Hoenn Safari: Southeast |
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
| Phantump | Johto Safari: Low left, Johto Safari: Low right, Johto Safari: Top left, Johto Safari: Top middle and more |
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
| Scatterbug (Continental) | Hoenn Safari: North, Hoenn Safari: Northeast |
| Scatterbug (Elegant) | Johto Safari: Top middle |
| Scatterbug (Garden) | Hoenn Safari: South, Hoenn Safari: Southeast |
| Scatterbug (High Plains) | Johto Safari: Low middle |
| Scatterbug (Icy Snow) | Kanto Safari: Mountain |
| Scatterbug (Jungle) | Hoenn Safari: Northwest, Hoenn Safari: Southwest |
| Scatterbug (Marine) | Kanto Safari: Beach, Kanto Safari: Brush |
| Scatterbug (Meadow) | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Scatterbug (Modern) | Bug-Catching Contest: Tuesday |
| Scatterbug (Monsoon) | Hoenn Safari: Northwest, Hoenn Safari: South, Hoenn Safari: Southwest |
| Scatterbug (Ocean) | Johto Safari: Top right |
| Scatterbug (Polar) | Bug-Catching Contest: Thursday |
| Scatterbug (River) | Johto Safari: Low right |
| Scatterbug (Sandstorm) | Johto Safari: Low left |
| Scatterbug (Savanna) | Johto Safari: Top left |
| Scatterbug (Sun) | Hoenn Safari: North, Hoenn Safari: Northeast |
| Scatterbug (Tundra) | Bug-Catching Contest: Saturday |
| Scolipede | Bug-Catching Contest: Thursday |
| Scyther | Bug-Catching Contest: Tuesday |
| Sewaddle | Bug-Catching Contest: Thursday |
| Shelmet | Bug-Catching Contest: Thursday |
| Silcoon | Bug-Catching Contest: Thursday |
| Sizzlipede | Bug-Catching Contest: Saturday |
| Skiddo | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Skrelp | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest, Johto Safari: Low middle and more |
| Sliggoo | Hoenn Safari: Northwest, Hoenn Safari: Southeast, Hoenn Safari: Southwest |
| Snom | Bug-Catching Contest: Saturday |
| Spewpa (Archipelago) | Hoenn Safari: Southeast |
| Spewpa (Continental) | Hoenn Safari: North, Hoenn Safari: Northeast |
| Spewpa (Elegant) | Johto Safari: Top middle |
| Spewpa (Garden) | Hoenn Safari: South, Hoenn Safari: Southeast |
| Spewpa (High Plains) | Johto Safari: Low middle |
| Spewpa (Icy Snow) | Kanto Safari: Mountain |
| Spewpa (Jungle) | Hoenn Safari: Northwest, Hoenn Safari: Southwest |
| Spewpa (Marine) | Kanto Safari: Beach, Kanto Safari: Brush |
| Spewpa (Meadow) | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Spewpa (Modern) | Bug-Catching Contest: Tuesday |
| Spewpa (Monsoon) | Hoenn Safari: Northwest, Hoenn Safari: South, Hoenn Safari: Southwest |
| Spewpa (Ocean) | Johto Safari: Top right |
| Spewpa (Polar) | Bug-Catching Contest: Thursday |
| Spewpa (River) | Johto Safari: Low right |
| Spewpa (Sandstorm) | Johto Safari: Low left |
| Spewpa (Savanna) | Johto Safari: Top left |
| Spewpa (Sun) | Hoenn Safari: North, Hoenn Safari: Northeast |
| Spewpa (Tundra) | Bug-Catching Contest: Saturday |
| Spinarak | Bug-Catching Contest: Tuesday |
| Spritzee | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Surskit | Bug-Catching Contest: Thursday |
| Swadloon | Bug-Catching Contest: Thursday |
| Swirlix | Johto Safari: Low left, Johto Safari: Low middle, Johto Safari: Low right, Johto Safari: Top left and more |
| Talonflame | Hoenn Safari: North, Hoenn Safari: Northeast, Hoenn Safari: Northwest, Hoenn Safari: South and more |
| Venipede | Bug-Catching Contest: Thursday |
| Venomoth | Bug-Catching Contest: Tuesday |
| Venonat | Bug-Catching Contest: Tuesday |
| Vivillon (Archipelago) | Hoenn Safari: Southeast |
| Vivillon (Continental) | Hoenn Safari: North, Hoenn Safari: Northeast |
| Vivillon (Elegant) | Johto Safari: Top middle |
| Vivillon (Garden) | Hoenn Safari: South, Hoenn Safari: Southeast |
| Vivillon (High Plains) | Johto Safari: Low middle |
| Vivillon (Icy Snow) | Kanto Safari: Mountain |
| Vivillon (Jungle) | Hoenn Safari: Northwest, Hoenn Safari: Southwest |
| Vivillon (Marine) | Kanto Safari: Beach, Kanto Safari: Brush |
| Vivillon (Meadow) | Kanto Safari: Beach, Kanto Safari: Brush, Kanto Safari: Mountain |
| Vivillon (Modern) | Bug-Catching Contest: Tuesday |
| Vivillon (Monsoon) | Hoenn Safari: Northwest, Hoenn Safari: South, Hoenn Safari: Southwest |
| Vivillon (Ocean) | Johto Safari: Top right |
| Vivillon (Polar) | Bug-Catching Contest: Thursday |
| Vivillon (River) | Johto Safari: Low right |
| Vivillon (Sandstorm) | Johto Safari: Low left |
| Vivillon (Savanna) | Johto Safari: Top left |
| Vivillon (Sun) | Hoenn Safari: North, Hoenn Safari: Northeast |
| Vivillon (Tundra) | Bug-Catching Contest: Saturday |
| Volbeat | Bug-Catching Contest: Thursday |
| Weedle | Bug-Catching Contest: Tuesday |
| Whirlipede | Bug-Catching Contest: Thursday |
| Wurmple | Bug-Catching Contest: Thursday |
| Yanma | Bug-Catching Contest: Tuesday |
