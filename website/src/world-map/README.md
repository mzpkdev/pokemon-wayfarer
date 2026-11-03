# World map

The whole overworld in the game's own metatiles: Johto, Kanto, Hoenn, Sinnoh,
Sinjoh, the Sevii Islands and Alola on one sea. It has two parts, a builder
that turns the game's maps into data and a client that draws that data.

The rest of the site uses it only through `index.ts`:

- `WorldTour` is the landing page's world section, one screen tall. When it
  starts, the player rides in on their bike to their front door, with the
  game's map name popups. The first card picks where: PALLET TOWN (RED; the
  default), NEW BARK TOWN (GOLD, out of ROUTE 29's grass) or LITTLEROOT TOWN
  (BRENDAN, down ROUTE 101). Choosing another plays the game's warp on the map: an
  8-step fade to black, a cut, and an 8-step fade in, then the new ride. It
  starts when its section is mostly on screen, or on an event the page sends
  (`startOn`; the landing page sends one when its warp from the title scene
  fades in, see `src/lib/warp.ts`). After that the map holds still beside a
  card, and NEXT and BACK fly the camera between eight stops: the town, its
  one region (Hoenn, Johto or Kanto), the whole world (wherever you start,
  you can go anywhere), who lives on its routes, the 24 Gyms, and each league in turn (Indigo
  Plateau, the Hoenn League, the Sevii Masters). From the second
  stop on the map can be panned in place, the way an embedded map shares a
  page: a mouse drags, and on touch two fingers move it while one still
  scrolls the page (a hint says so). The zoom stays each stop's. At the Gym
  stop the pins answer the pointer: each is just its badge over a soft shadow;
  a hover lifts it with a white glow and names its town, a press settles and
  darkens it, and an open card makes it glow gold and bob while the rest dim
  (hidden buttons do the same for a keyboard), and a pin opens its Gym's card:
  the badge, the leader's battle portrait (`leaders/`, from the pic of their
  leader battle in the party files) and what they are up to on the page
  clock, and FIND, which pans onto them and keeps them in view as they walk
  (GO TO the Gym for a leader who doesn't walk the map). NEXT and BACK fly on
  from wherever the map was moved to. It never takes over the page's
  scrolling.
- At the wild stop (`wild.ts`, `WildTally.tsx`) the camera stays on the whole
  world and icons from `icons.png` pop out of routes in waves, Gen I to VIII,
  the regional forms, then a mix (about 14 s, looping). The page states the
  full roster, as all of it will be in the final game: every species of Gen I
  to VIII by national dex number (905) and the Alolan, Galarian and Hisuian
  forms (53), from the game's species data (`.natDexNum`; macro-built entries
  expanded with their arguments). A wave pops that generation's species over
  routes spread by farthest-point sampling, an illustration, not who lives
  on each route (the explorer's place cards say that). The card tallies each
  generation as its wave plays, ending on the total; tapping a chip holds its
  wave on the map, and tapping it again plays on. Gen IX stays off the page.
  Reduced motion shows the full tally and a still scatter; a chip picks whose.
- At the Gym stop a tap on a pin (or its hidden button, `aria-pressed`)
  toggles the badge earned: earned pins are bright with a gold glow and a
  sparkle (a burst when earned), the rest fainter, and earning opens that
  Gym's card ("BOULDER BADGE EARNED", the leader, FIND); giving it back closes
  it. A chip shows BADGES n/24. The earned set is the tour's state: it
  persists across the league stops and a change of start. The rules are the
  game's (`league_circuit.c`): badges from every region count together,
  Trainer Rating is `10 * badges` up to 8 and `80 + 5 * (badges - 8)` after,
  and 80 (any 8 badges) qualifies; Indigo also needs one Kanto or Johto badge,
  the Hoenn League one Hoenn badge (`callingFor` in `landmarks.ts`). The first
  time the set qualifies a league that hasn't phoned yet on the page, its call
  rings (`LeagueCall.tsx`): a pulsing phone, then a GBA text window with the
  league's name box (INDIGO LEAGUE, HOENN LEAGUE), the game's lines
  (`LeaguePhone_Text_*` in `data/scripts/league_phone.inc`, the league and
  place as `league_phone.c` buffers them, the lineup as the first event's five),
  YES/NO with the game's decline confirmation, and the accept or decline
  follow-up. Accepting flies the tour to that league. It is a modal `<dialog>`
  (Esc or HANG UP closes it, with no answer); reduced motion skips the typing.
  Which league rings when both could is `ChooseCallingLeague`'s order: the one
  that called least recently (none has), then the most badges of its side
  (Indigo counts Kanto and Johto, Hoenn counts Hoenn), then Indigo (`callerFor`).
  A second league rings on a later toggle, once each per visit. At the league
  stops each league is a plate in the region labels' style, glowing while it
  calls; the trail has two legs (Indigo and Hoenn to Seven Island) that draw
  in, dotted and marching, once their league calls. Each league stop flies to
  its league at a medium zoom and highlights its plate and leg. For Indigo and
  Hoenn the five that fight there are the hero (`LineupPanel.tsx`): five seats
  in battle order, one per hall (name and battle condition from
  `game/src/league_halls.c`, `circuit[].halls`), the Champion's Room last. The
  whole roster's portraits shuffle past, then five settle in order, and a few
  seconds later the next event picks again (reduced motion: just the first
  five). The picks are not random: `lineup.ts` replays `SelectLeagueLineup`
  (`league_selection.c`) on the builder's `roster`: score = Trainer Rating at
  the player's rating x willingness / 100, willingness is 100 less travel
  (home region vs the league's: 0 at home, 10 for a traveller away, else 80;
  Indigo is Kanto and Johto, Hoenn is Hoenn) less 50 for whoever fought in the
  last event (floor 5), the top five are seated, aloof trainers only when
  within 10 team levels of the best five of the rest, and the battle order is
  ascending rating. Successive events feed the last five forward as the tired
  ones, so the page alternates between two fives for a given rating. The
  pool is every enabled registry trainer: Gym Leaders, Elite Four and
  Champions (Tate and Liza are doubles-only and out). The Sevii Masters'
  lineup format isn't shipped, so its stop shows only its halls and how it
  opens. The last card's EXPLORE THE MAP opens the explorer full screen.
  Captions are one per stop, with per-start `variants`; `stopOverlays` lets the
  page draw over the map at any stop (the hook for a living-world overlay on
  the world stop).
  `WorldMap` is the explorer. Drag, pinch, wheel, double-click, the arrow keys
  and +/- all pan and zoom it.
  Tapping a map (or Enter on the one at the center) frames it and opens a place
  card: its name and wild Pokémon by method and time of day, as icon, name and
  percent. The card is a bottom sheet on a phone and a left panel from `sm` up.
  `WorldMap`'s `extra` render prop puts more (a town's Gym, ports) under the
  card's name.

Outside this folder, the module uses the site's pixel CSS utilities
(`pixel-panel`, `pixel-label`, …), `useReducedMotion` from `~/lib/hooks`, and the
PNG/palette helpers in `scripts/lib/`.

## Data

`pnpm world-map` (run from `website/`) builds the data into
`public/world-map/`. It takes about 5 seconds and needs the game tree at
`../game`. Its output is committed, like the rest of the game art, so building
the site doesn't need the game tree. Rebuild it after maps, tilesets or the
builder change.

| File              | What it holds                                                                                                                                  |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| `atlas.png`       | Every metatile the world uses, 16×16 each, 64 to a row                                                                                         |
| `grid.bin.gz`     | The world as gzipped little-endian u16 atlas indices, row by row; `0xffff` is open sea                                                         |
| `overview.png`    | The far-out view: the tiles averaged, one pixel per metatile, and coarser levels for further out, packed side by side                          |
| `encounters.json` | Per placed map, its name and wild Pokémon (species slot and percent) by method and time of day; `gens` (each species' generation) and `counts` |
| `icons.png`       | One 32×32 icon per species used, 16 to a row, indexed by the slot in `encounters.json`                                                         |
| `notables.json`   | The notables' stops, cycles and routes (see below)                                                                                             |
| `meta.json`       | Grid size, atlas columns, the overview's levels, the open ocean's tiles and color, towns, region labels with bounds, and placed maps           |

## Builder (`build/`)

Plain Node scripts. Each stage gets a module:

- `index.mjs`: the entry point, which writes the outputs.
- `game.mjs`: reads the game's maps, layouts, tilesets and Town Map cells, and
  draws metatiles.
- `layout.mjs`: places the maps. Maps are joined along their connections.
  Where a loop of connections doesn't add up, its error goes across the edges
  (a gap, or a few rows of overlap), never along them, so roads crossing an
  edge line up; a connection that would lay maps deep over each other is given
  up.
  Pieces no connection reaches are placed by their Town Map cells, overlaps are
  pushed apart, and the regions are arranged on the sea, wide straits apart.
  Sinnoh sits well north of Kanto; Sinjoh, under it, is bridged to it by land.
- `land.mjs`: fills in land between the maps, so no map's rectangle shows.
  A gap between two connected maps is bridged by stretching both edges across
  it (sliding by any misalignment, so the road jogs instead of stopping).
  Coasts are grown with noise, and a few cells of fill always lie in front of
  a map's land edge. Water at a map's edge (rivers, bays, sea routes) spills
  on past it a wandering way in the map's own water, kept off other maps'
  land. The insides of loops are filled, straits keep regions apart, and each
  filled cell takes the border blocks of the nearest land-bordered map (an
  island town far from any takes its own edge's ground). The sea takes the
  nearest map's water by the coasts, then deepens in four noisy steps, each a
  tinted copy of that water a quarter nearer one deep ocean, so every game's
  water meets out at sea in one color and tile (`DEEP_SEA`). The tints are new
  metatiles in the atlas, so the tiles and the far-out view agree.
- `encounters.mjs`: the wild Pokémon and icons. The wayfarer build compiles only
  `game/src/data/wild_encounters_v2/*.json` (not `wild_encounters.json`, which
  is for the standalone builds). Those hold a day table (morning and day) and
  a night table (evening and night) per method, no seasons. Shares come from
  the slot weights: `wild_encounters.json` field rates for land, surf and rock
  smash, `standard_rod_fishing.json` for each rod. Names are the Town Map
  sections' (`region_map_sections.json`) and species names and icon palettes
  (`.iconPalIndex`, `graphics/pokemon/icon_palettes`) come from the species info.
- `overview.mjs`: the far-out view, each pixel the average of the metatile
  under it in the GBA's 5-bit colors, and four coarser levels. A coarser pixel
  is land or sea by the majority under it and averages only those, so coasts
  stay crisp at any level.
- `landmarks.mjs`: finds each gym by the maps whose scripts set a `FLAG_BADGEnn_GET`
  and follows warps out to the town (the badge's place in its region is the flag
  number less one, modulo 8), the league doors from `league_circuit.c`, and the league `roster` (every enabled trainer in `tools/notable_trainers/catalog.json`: home region, traveller, aloof, TR growth curves from `notable_trainers.c`, and battle portrait via the party files, or the `.h` trainer tables for the Indigo Plateau's). Portraits for the roster and the Gym Leaders go to `leaders/`.
  INDIGO PLATEAU isn't a placed map, so its pin sits at its Town Map cell
  relative to ROUTE 22.
- `source/`: Sinnoh's Town Map cells, borrowed from the project its maps came
  from. Its README gives the credit.

The builder only reads the maps the wayfarer build compiles, and connects them
the way `tools/mapjson` does.

## Client

- `data.ts`: loads and caches the data, shared by the tour and the explorer.
- `encounters.ts` and `PlaceCard.tsx`: the encounters, loaded the first time a place is picked, and the card.
- `camera.ts`: the camera (a world point and CSS pixels per metatile), fitting
  bounds into a view, and the eased flight between two cameras.
- `render.ts`: draws a view. Far out it draws the overview level whose pixels
  are at least 1.5 CSS pixels, blown up without smoothing like the GBA's
  mosaic. Close in it draws 32×32-metatile chunks (4 pixels a metatile, then
  16), cross-fading in from the overview. Past the grid's edges the chunks
  tile the deep ocean, so the world has no edge. Chunks are drawn a few per
  frame, and the overview shows through until they're done. It also draws the
  region and town names.
- `starts.ts`: the three starts: town, player sprite, ride path (in each map's own tiles, so it follows the maps if the layout moves), opening shot and popup names.
- `rider.ts`: draws the player on their bike along a ride.
- `fade.ts`: the game's 8-step fade, shared with `src/lib/warp.ts`.
- `landmarks.ts`: loads `landmarks.json`; draws the badge pins, the league plates and the two-leg dotted trail as overlays, and says which leagues a set of badges calls and which rings first.
- `lineup.ts`, `LineupPanel.tsx`, `LeagueCall.tsx`, `ui.ts`: the league lineup rule, its draft panel, the phone call, and the shared yellow button. Badge art is `public/game/badges.png` (from the trainer card, `pnpm assets`).
- `clock.ts`: one game clock for the page (a day every two minutes from 8:00), so the tour's world stop and the explorer show the same time of day and the same ships at sea.
- `PlaceExtra.tsx`: what the explorer's place cards add for a town: its Gym and badge, its stop on the league circuit, and the ferries that call there.
- `WorldTour.tsx` and `WorldMap.tsx`: the two components.

## The living world

`living.mjs` (builder) writes `living.json`; `living.ts` loads and draws it;
`LivingClock.tsx` is the clock badge. The tour's world stop and the explorer draw it.

- The ferries' loops, port and sea minutes (30 and 360), ship names, the
  Day hours and the time-of-day tint colors are read from the game's source
  (`wayfarer_ferry.c`, `constants/wayfarer_ferry.h`, `constants/rtc.h`,
  `overworld.c`). The sea lanes are not the game's: it has no ship on a map,
  so they are routed over the grid's open water, kept off coasts where the
  straits allow, and rounded. Each ship's cycle starts at the first moment the
  game's schedule table has it at its first port (Aqua and Tidal 8:00, Anne
  20:00). In the game a voyage starts at boarding, so on the map this is an
  illustration of the loop, not a live position.
- `loadLiving(base)`, `gameClock(rate, start)` (`LIVING_RATE.game` is the game's
  day per 2 hours, `.page` a day per 2 minutes), `livingOverlay(living, minutes)`
  for `View.overlays` (lanes, ships, then the tint as a multiply), `periodAt`,
  `shipsAt`, and `pickShip` with `shipReach(zoom)` for hover labels.

## The notables

`build/notables.mjs` writes `notables.json`; `notables.ts` loads it and draws the
25 simulated Gym Leaders and Elite Four walking the map on the page clock
(`pageMinutes()`), and the Gym Leaders whose overworld sprite only stands
(Roxanne, Brawly, Wattson, Flannery, Winona, and Tate beside Liza) waiting at
their Gym (`build/source/stationary-leaders.json`). The tour's world stop and the explorer draw them (under the
time-of-day tint at the world stop); tapping one in the explorer shows a
toast like "MISTY · fishing at Nugget Bridge", and a town's place card lists who
lives there ("HOME OF BROCK") and who is there now.

This is a page-side simulation of the game's routines on the page's clock, not
live game state: the game advances them on map changes (see
`.product/specs/notable-world-simulation.md`, `src/wayfarer_world_sim.c`).

- Game-faithful, read from `game/tools/wayfarer_world` (`routines.json`,
  `named_spots.json`) and `tools/notable_trainers/catalog.json`: who they are,
  their homes and Gyms, activity cycles, favourites per activity, dwell beats per
  activity (`sDwell`). Indoor places sit at the door of the outdoor map that
  leads in (`createDoors` in `landmarks.mjs`).
- Ours: the timing (a beat is 45 game minutes; dwell is stretched to fill a
  round of the cycle; each trainer has a phase offset; the favourite for a step
  is picked by a hash of trainer, round and step), the walking pace, and the
  routes. Steps with no favourite are spent in the home town (the Gym for a
  Gym Leader's home and training). Favourites in places the grid doesn't hold
  (the Battle Frontier) are dropped; Trainer Hill's courtyard is put on Route 40.
- Routes: A* over the world grid's land (placed maps cost less than filler
  land), simplified. Where no land joins two places, the route crosses water;
  its water segments are marked (`w`), and there the trainer leaves a white
  wake and the label says "crossing the sea". Everyone walks at one pace, on
  land and water alike, with no cap on a trip's length, so nobody speeds up
  mid-trip. It's an illustration, like the ferries, not a ferry schedule.
- Sprites: `public/game/sprites/notables.png` (`pnpm assets`), one row of nine
  16x32 frames per trainer in `routines.json` order, from each trainer's
  `OBJ_EVENT_GFX_*` info. Will and Karen have stand frames only; they hop.
- Drawn at least 26 CSS pixels tall (18 on a narrow view), true scale zoomed in.
