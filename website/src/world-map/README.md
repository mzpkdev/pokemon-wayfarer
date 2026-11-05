# World map

The whole overworld in the game's own metatiles: Johto, Kanto, Hoenn, Sinnoh,
Sinjoh, the Sevii Islands and Alola on one sea. It has two parts, a builder
that turns the game's maps into data and a client that draws that data.

The rest of the site uses it only through `index.ts`. The landing page's
RIGHT NOW board (`src/components/RightNow.tsx`) reads the same clock and
simulation from it (`pageMinutes`, `shipsAt`, `trainAt`, `notablesAt` and
the notables' sprite sheet), so it shows the ships, the train and the Gym
Leaders where the map has them. The two parts a visitor meets:

- `WorldTour` is the landing page's world section, one screen tall. When it
  starts, the player rides in on their bike to their front door, with the
  game's map name popups. The first card picks where: PALLET TOWN (RED; the
  default), NEW BARK TOWN (GOLD, out of ROUTE 29's grass) or LITTLEROOT TOWN
  (BRENDAN, down ROUTE 101). Choosing another plays the game's warp on the map: an
  8-step fade to black, a cut, and an 8-step fade in, then the new ride. It
  starts when its section is mostly on screen, or on an event the page sends
  (`startOn`; the landing page sends one when its warp from the title scene
  fades in, see `src/lib/warp.ts`). After that the map holds still beside a
  card, and NEXT and BACK fly the camera between six stops, one journey that
  only pulls out: home; a busy route of the start's region (ROUTE 15, 32 or
  103, `dailyRoute` in `starts.ts`, close in on the stretch holding most of
  its trainer and item spots) changing from day to day (`daily.ts`,
  `dailyView.ts`): the game's own daily draw for a save that has beaten
  everyone there, so its trainers stand in new spots or stay away and its
  item spots roll new items, while the card steps the days (by itself every
  few seconds until a visitor steps it with ◀ and ▶) and lists who and what
  is out; the start's region with its eight Gyms; the
  whole world with its ferries and who lives where; the league circuit (all
  24 Gyms and the three leagues); and the whole map again to explore. A
  journey strip on the card marks each stop with an item or sprite from the
  game (the start's rider, a Poké Ball for the daily route, the region's first badge, the S.S.
  Ticket, the VS Seeker, the Town Map); a tap on a mark goes there. From the second
  stop on the map can be panned in place, the way an embedded map shares a
  page: a mouse drags, and on touch two fingers move it while one still
  scrolls the page (a hint says so). The zoom stays each stop's. At the region
  and circuit stops the Gym pins answer the pointer: each is just its badge over a soft shadow;
  a hover lifts it with a white glow and names its town, a press settles and
  darkens it, and an open card makes it glow gold and bob while the rest dim
  (hidden buttons do the same for a keyboard), and a pin opens its Gym's card:
  the badge, the leader's battle portrait (`leaders/`, from the pic of their
  leader battle in the party files) and what they are up to on the page
  clock, and FIND, which pans onto them and keeps them in view as they walk
  (GO TO the Gym for a leader who doesn't walk the map). NEXT and BACK fly on
  from wherever the map was moved to. It never takes over the page's
  scrolling.
- At the world stop (`wild.ts`, `WildTally.tsx`) icons from `icons.png` pop
  out of the whole world in waves, one per home region (about 4 s each, looping,
  beginning with the start's own region), over the ferries and the time of
  day. Each generation has a home, from the natives table of
  the wild encounters v2 PRD: Gen I from Kanto and II from Johto (one piece of
  land, which they share), III and V in
  Hoenn, Kalos (VI) in the three Safari Zones, Galar (VIII) in the Sevii
  Islands, VII in Alola, Hisui (Wyrdeer to Enamorus) in Sinjoh, and the
  regional forms with their region. A wave pops its home's species only where
  they really live, by the encounter tables (a species picked by how often it
  comes up there, so Hoenn's Gen V share grows to the east), crowding the home
  region's routes where they make up at least 35% of what turns up (Gen
  I crowds Johto too, Gen II keeps mostly to Johto); a small home (Alola, Sinjoh) and
  each Safari Zone, over the map in its middle, huddle a few icons together so
  they still read as a crowd
  at world zoom. Up to three icons per wave are real strays: its species where
  they live away from home. Gen IV waits for Sinnoh, which has no wild
  Pokémon yet: its wave stands black silhouettes with a "?" over Sinnoh, keeping its count so
  the total adds up. The page states the full
  roster, as all of it will be in the final game: every species of Gen I to
  VIII by national dex number (905) and the Alolan, Galarian and Hisuian forms
  (53), from the game's species data (`.natDexNum`; macro-built entries
  expanded with their arguments). The card tallies each home as its wave
  plays and names whose home it is; tapping a chip holds
  its wave on the map, and tapping it again plays on. Gen IX stays off the
  page. Reduced motion shows the full tally and the first wave still; a chip picks
  whose.
- At the region and circuit stops a tap on a pin (or its hidden button,
  `aria-pressed`; at the region stop only its own eight)
  toggles the badge earned: earned pins are bright with a gold glow and a
  sparkle (a burst when earned), the rest fainter, and earning opens that
  Gym's card ("BOULDER BADGE EARNED", the leader, FIND); giving it back closes
  it. A chip shows BADGES n/24. The earned set is the tour's state: it
  persists across the stops and a change of start. The rules are the
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
  follow-up. Accepting flies the tour to the circuit stop and on to that league. It is a modal `<dialog>`
  (Esc or HANG UP closes it, with no answer); reduced motion skips the typing.
  Which league rings when both could is `ChooseCallingLeague`'s order: the one
  that called least recently (none has), then the most badges of its side
  (Indigo counts Kanto and Johto, Hoenn counts Hoenn), then Indigo (`callerFor`).
  A second league rings on a later toggle, once each per visit. At the circuit
  stop each league is a plate in the region labels' style, glowing while it
  calls; the trail has two legs (Indigo and Hoenn to Seven Island) that draw
  in, dotted and marching, once their league calls. The card lists the three
  leagues with whether each is calling; a tap on one flies the map on to it at
  a medium zoom and highlights its plate and leg, and ALL 24 GYMS flies back
  out. For Indigo and
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
  lineup format isn't shipped, so it shows only its halls and how it opens.
  The last card's EXPLORE THE MAP opens the explorer full screen.
  Captions are one per stop, with per-start `variants`; `stopOverlays` lets the
  page draw over the map at any stop.
  `WorldMap` is the explorer. Drag, pinch, wheel, double-click, the arrow keys
  and +/- all pan and zoom it.
  Close in (from 6 CSS pixels a metatile), wild Pokémon pop out of the
  routes (`sightings.ts`): each of a map's sampled encounter cells (tall grass
  and the like on land, the water for surfing and fishing) now and then has
  one of the route's own Pokémon out for a few seconds, picked by its rate in
  the table for the time of day on the page clock (morning and day use the day
  table, evening and night the night one). A land one stands on its cell over
  a shadow; a water one is half under, in a ripple. Most cells are quiet at any
  moment, so a route shows a few, not all. Nothing is stored: a sighting is a
  hash of the map, the cell and the stretch of time.
  Closer still (from 11 CSS pixels a metatile, fully by 14), today's daily
  trainers and items stand on every road (`dailyView.ts` again, with the
  tour's empty-spot rings off): the same draw the tour replays, for the
  visitor's calendar day, so the roads change at midnight and a visit the next
  day finds people elsewhere. Only what is on screen is drawn.
  Tapping a map (or Enter on the one at the center) frames it and opens a place
  card: its name, and a line saying wild Pokémon live there if they do. The
  exact encounter rates are behind the debug menu (`DebugMenu.tsx`), which
  opens with `?debug` in the address or the backquote key: with "Encounter
  rates on place cards" on (kept in this browser), the card lists the wild
  Pokémon by method and time of day, as icon, name and percent. The card is a
  bottom sheet on a phone and a left panel from `sm` up.
  `WorldMap`'s `extra` render prop puts more (a town's Gym, ports) under the
  card's name.

Outside this folder, the module uses the site's pixel CSS utilities
(`pixel-panel`, `pixel-label`, …), `useReducedMotion` from `~/lib/hooks`, and the
PNG/palette helpers in `scripts/lib/`.

## Data

`pnpm world-map` (run from `website/`) builds the data into
`public/world-map/`. It takes about 30 seconds and needs the game tree at
`../game`. Its output is committed, like the rest of the game art, so building
the site doesn't need the game tree. Rebuild it after maps, tilesets or the
builder change.

| File              | What it holds                                                                                                                                                                                                                                                                                                                                                                                 |
| ----------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `atlas.png`       | Every metatile the world uses, 16×16 each, 64 to a row                                                                                                                                                                                                                                                                                                                                        |
| `grid.bin.gz`     | The world as gzipped little-endian u16 atlas indices, row by row; `0xffff` is open sea                                                                                                                                                                                                                                                                                                        |
| `overview.png`    | The far-out view: the tiles averaged, one pixel per metatile, and coarser levels for further out, packed side by side                                                                                                                                                                                                                                                                         |
| `encounters.json` | Per placed map, its name and wild Pokémon (species slot and percent) by method and time of day; its sampled land (`l`) and water (`w`) encounter cells; `gens`, each slot's roster `entry`, the roster by `homes`, and the Safari Zones' Kalos by `safari`                                                                                                                                    |
| `icons.png`       | One 32×32 icon per species used, 16 to a row, indexed by the slot in `encounters.json`                                                                                                                                                                                                                                                                                                        |
| `daily.json`      | The game's daily trainer slots (home trainer name, sprite cells, absence chance, group, placed objects' grid cells), the groups in the game's order, the placed item spots with their reach tier and region, and the item pools; `dailyOn` in `daily.ts` replays a day's draws from them                                                                                                      |
| `trainers.png`    | One 32×32 cell per trainer sprite used (first standing frame, facing down, bottom-centred) and the item ball, 16 to a row, indexed by `daily.json`                                                                                                                                                                                                                                            |
| `notables.json`   | The notables' stops, cycles and routes (see below)                                                                                                                                                                                                                                                                                                                                            |
| `water.bin.gz`    | Which cells are water, a bit each, row by row with the high bit first; the open sea past every map is water, the Magnet Train's viaduct is not, and a cell that only looks like water counts when it touches water (so the sea's tints do and a blue roof doesn't). The living sea keeps its glints and ripples to them                                                                       |
| `meta.json`       | Grid size, atlas columns, the overview's levels, the open ocean's tiles and color, towns, region labels with bounds, `overpasses` (the bridge decks and the viaduct over water, which the ships pass under: no ship is drawn over them), and placed maps, each with the game's `weather` where it has one worth drawing (`rain`, `storm`, `snow`, `sand`, `shade` or `leaves`, from map.json) |

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
  pushed apart (the event islands, off any Town Map in Emerald, are given cells:
  Navel Rock and Birth Island near FRLG's on the Sevii Town Maps, just clear
  of the Five and Six Island pieces, and Faraway Island one south of Hoenn), and the regions are arranged on the sea, wide straits apart.
  Sinnoh sits well north of Kanto; Sinjoh, under it, is bridged to it by land.
  The Safari Zones are enclaves (`ENCLAVES`): the game enters them through a
  gate house, so no connection reaches them and no Town Map cell is theirs.
  Each is set down by hand beside a placed map, after everything else, so
  nothing moves for it: Hoenn's north of Route 121, Johto's west of Cianwood
  City with Routes 47 and 48 and the Safari Zone Gate that lead to it, and
  Kanto's on the shore south of Route 15, the nearest room to Fuchsia City.
  A gate house's door stays on its town's side (`createDoors`).
- `land.mjs`: fills in land between the maps, so no map's rectangle shows.
  A gap between two connected maps is bridged by stretching both edges across
  it (sliding by any misalignment, so the road jogs instead of stopping).
  Coasts are grown with noise, and a few cells of fill always lie in front of
  a map's land edge. Water at a map's edge (rivers, bays, sea routes) spills
  on past it a wandering way in the map's own water, kept off other maps'
  land. The insides of loops are filled, straits keep regions apart, every
  ferry port and Sevii Island dock (a town warp into a `_HARBOR` map) keeps a
  way out to the open sea, cut through the grown land as an inlet that widens
  toward the sea where the land closes its bay (the way may sail a sea
  route's own water and pass under a bridge, `build/bridges.mjs`: a stretch
  of a road map that is a deck over water, like the cycling road's west of
  VERMILION CITY, which is how its harbour reaches the sea; the inlet keeps
  its width clear of the maps, so it never squeezes through a slit between
  two), and each filled cell takes the border blocks of the nearest land-bordered map (an
  island town far from any takes its own edge's ground), counted from that
  map's corner as the game counts them, so its rows of trees carry on past
  its edge in step. Ilex Forest, walled in by rock in the game, lends Azalea
  Town's trees instead. The sea takes the
  nearest map's water by the coasts (a map lends its water only when the
  water runs off its edge, so a pond doesn't paint the sea pale), then deepens in four noisy steps, each a
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
- `rails.mjs`: the Magnet Train's line. Only Route 32 holds its track, so it
  is carried on, in metatiles drawn from Route 32's (turned on their side for
  runs north and south), west to GOLDENROD CITY's edge, deck level with the
  city's own track there, and east into SAFFRON CITY up to its station, the cheapest way by a search that counts the map cells
  covered (towns most) and the turns. It is laid on a copy of the grid that
  is only drawn; the ferries and walkers keep to the ground under it.
- `daily.mjs`: the daily trainer slots and item spots for the tour's daily
  route. It reads the generated slot and item tables
  (`src/data/trainer_slots/slots.h`, `src/data/item_slots/`) and names from
  the party files, `battle_main.c`, `items.h` and `moves_info.h`.
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
the way `tools/mapjson` does. It takes the outdoor ones (towns, cities, routes
and sea routes) and three the game types otherwise: Faraway Island and Navel
Rock, which Emerald calls indoor, and Sinnoh's Pokémon League, between Routes
223 and 224, which has no type. Besides the mainland it places the islands the
boats sail to: Southern Island, Faraway Island and the Battle Frontier off
Hoenn (Lilycove's charters), and Navel Rock and Birth Island among the Sevii
Islands (Vermilion's Seagallop).

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
- `noise.ts`, `water.ts`, `sea.ts`, `clouds.ts`, `weather.ts` and `lights.ts`: the atmosphere over the map, see below.
- `PlaceExtra.tsx`: what the explorer's place cards add for a town: its Gym and badge, its stop on the league circuit, and the ferries that call there.
- `WorldTour.tsx` and `WorldMap.tsx`: the two components.

## The living world

`living.mjs` (builder) writes `living.json`; `living.ts` loads and draws it;
`LivingClock.tsx` is the clock badge. The tour's world stop and the explorer draw it.

- The ferries' loops, port and sea minutes (30 and 360), ship names, the
  Day hours and the time-of-day tint colors are read from the game's source
  (`wayfarer_ferry.c`, `constants/wayfarer_ferry.h`, `constants/rtc.h`,
  `overworld.c`). The sea lanes are not the game's: it has no ship on a map,
  so they are routed over the grid's open water (and under the bridges of
  `build/bridges.mjs` and the viaduct, whose decks `livingOverlay` clips the
  ships out of, `overpasses` in meta.json), kept off coasts where the straits
  allow, and rounded. Each ship's cycle starts at the first moment the
  game's schedule table has it at its first port (Aqua and Tidal 8:00, Anne
  20:00). In the game a voyage starts at boarding, so on the map this is an
  illustration of the loop, not a live position.
- The Magnet Train runs its line (`rails.mjs`) back and forth: half an hour at
  each station, eight hours between, from Goldenrod at 8:00. That timing is
  ours; in the game a ride is a cutscene. Its sprite is the game's two train
  ends side by side (`public/game/sprites/magnet-train.png`, `pnpm assets`),
  drawn twice the game's size so it fills the viaduct's deck, each car turned
  the way the line runs under it.
- A ship at sea leaves a wake: a pale trail tapering back along its lane,
  with flecks of foam drifting off it, and it lifts and settles a game pixel
  on the swell. Both hold still with reduced motion (`still` in
  `livingOverlay`'s options).
- `loadLiving(base)`, `gameClock(rate, start)` (`LIVING_RATE.game` is the game's
  day per 2 hours, `.page` a day per 2 minutes), `livingOverlay(living, minutes)`
  for `View.overlays` (lanes, ships, then the tint as a multiply), `periodAt`,
  `shipsAt`, `trainAt`, and `pickShip` with `shipReach(zoom)` for hover labels.

## The atmosphere

What moves over the world besides its people and ships, drawn by overlays at
every stop of the tour and in the explorer. None of it is stored: a glint, a
raindrop or a cloud is a function of a few whole numbers and the time
(`noise.ts`: `hash`, `roll`, `tiledNoise`), so every frame agrees and nothing
has to be remembered between them. Reduced motion stops the clock they run on,
so each holds at one moment.

- The sea (`sea.ts`, on `water.bin.gz` through `water.ts`). Far out, glints of
  light come and go across the open water on a lattice that doubles in
  metatiles so it stays 36 to 72 CSS pixels apart; each cell's glint has its
  own lag and sits out most periods, so they don't blink together. From 6 CSS
  pixels a metatile, where the game's own water tiles show, a ripple drifts
  down some of the water tiles, a light stroke of a few game pixels and a
  shorter one eight rows under it, the way the overworld cycles its water.
  The glints are gone by 7 and the ripples all in by 10.
- Close in, the map is the game's alone (`extras.ts`): what the map adds that
  the game never draws (the ripples and the surf, the ships' lanes, wakes and
  names and the train's name, the arrow over a Gym Leader's head) fades out from 12 CSS pixels a metatile
  and is gone at 16, a game pixel a pixel, so a close view looks like the game.
- The clouds (`clouds.ts`). Seen from above, as from a satellite: faint,
  irregular masses in hard pixels of four metatiles, stepped into three
  levels of cover (a wisp, the body and the thick of it) and a few flat
  tones lit from the north-west, each casting its shadow 32 metatiles to the
  south-east. The sky is one noise field, sampled every eight metatiles and
  blended between to the pixels: the clouds' noise, curled by a slower one so
  their edges don't follow its lattice, plus a slower one still that gathers
  them into banks with clear sky between. It repeats every 30,720 metatiles,
  far wider than any screen at the explorer's widest. It's baked in chunks of
  512 metatiles as they come into view, a few milliseconds' worth a frame
  (each fading in) or all at once with reduced motion, kept in one cache the
  tour and the explorer share. Each frame the chunks on screen are composed a
  texel to a pixel into one image, which is drawn over the map, so there are
  no seams. Far out, where a pixel of cloud is under two screen pixels, it's
  blended so it doesn't shimmer as it drifts. They drift east and a little
  south. The clouds thin from 1.6 CSS pixels a metatile to nothing at 4.5,
  so the tiles are never under them close in, but their shadows stay at
  every zoom, a little lighter, going from blocks to a soft dimming by 10 (an
  unstepped shadow, baked for a chunk only once it's needed) so they don't
  pass over the towns as squares many game pixels wide.
- The weather (`weather.ts`, from `weather` on the maps in `meta.json`). Each
  map with weather gets a haze in its color from any distance and, from 2 CSS
  pixels a metatile (all there at 4), its drops: rain and a thunderstorm's
  slanting to the south-west with the storm's flashes now and then, snow
  swaying down (two or three game pixels over a shadow, so it shows on
  snow), a sandstorm's grit blowing west, the shade under a forest, and
  leaves tumbling down Bellchime Trail. All of it is clipped to the map's
  rectangle, and the drops hang on a lattice of 8-metatile cells over it,
  so only the cells on screen are drawn and there are as many close in as
  far out. The haze fades in from the map's edges, except along the
  stretch of an edge another map's weather lies on, so Olivine and Route 40
  rain as one (where a join ends partway along a side, the full edge meets
  the fade in a step as long as the feather: each map is feathered on its
  own); a storm's flash fades on every edge.
- The night (`lights.ts`, over the tint). As the time of day's tint darkens
  (a quarter in the evening and morning, about half at night), each town
  glows warm with a brighter core, each ship shows a lamp, and the Olivine
  lighthouse sweeps its beam round every seven seconds. The glows thin out
  from 9 CSS pixels a metatile and are gone by 16: the tiles have no lit
  windows to match them. `darknessAt(living, minutes)` is the share the tint
  takes off.

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
  are dropped (none today); Trainer Hill's courtyard is put on Route 40.
- Routes: A* over the world grid's land (placed maps cost less than filler
  land), simplified. Where no land joins two places, the route crosses water;
  its water segments are marked (`w`), and there the trainer leaves a white
  wake and the label says "crossing the sea". The client walks a route on the
  grid, as the overworld does: a metatile at a time, across or down, a slanted
  run becoming a staircase of steps (`toLeg` in `notables.ts`). Everyone walks at one pace, on
  land and water alike, with no cap on a trip's length, so nobody speeds up
  mid-trip. It's an illustration, like the ferries, not a ferry schedule.
- Sprites: `public/game/sprites/notables.png` (`pnpm assets`), one row of nine
  16x32 frames per trainer in `routines.json` order, from each trainer's
  `OBJ_EVENT_GFX_*` info. Will and Karen have stand frames only; they hop.
- Drawn at least 26 CSS pixels tall (18 on a narrow view), true scale zoomed in.
