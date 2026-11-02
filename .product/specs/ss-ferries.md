# S.S. ferries

PRD: [S.S. ferries](../prds/ss-ferries.md)
Implemented: Yes. All three parts are built. Part 1: ports and ships, the
schedule (`Ferry_ShipAt`), the saved voyage state with every field, boarding at
the four dock ports and the three launch ports, the ships' exits and the Anne's
dynamic warp, the story-trip guards, sailing (cast-off and arrival announcements
and catch-up), service, the old menus and the Lilycove charters, and the region
and save changes. Part 2: the cabins and the daily-slot freeze. Boarding draws
two distinct hosts of the ship's pool (`Random()`, saved with a parameter each);
the undrawn hosts' spots in their cabin rooms hide at map load; the talk flow and every v0 request kind work (a host battle
borrows an existing trainer's party and never whites out); sleeping at sea jumps
the clock to `eventAt` and reloads the map, and the arrival then plays from the
field poll; no ship bed heals; and `DailySlots_GetDay` returns `slotDay` while
aboard. Host table: seven hosts on the Aqua (it has seven cabin rooms), eight on
the Anne and eight on the Tidal. Only the Tidal's existing bed event (the
breeder's cabin) is a bed; every other host offers the sleep themselves, so the
extra Tidal bed events are not built. Part 3: the passengers. The Anne, Aqua and
Tidal trainers left the generator's reviewed exclusion and rotate as daily
slots. A new `liner` place type (map-name prefixes SSAnne, SSAqua, SSTidal) holds
the trainers whose lines need a ship (Dale, Wai, Phillip and Leonard); the ones
whose lines name a town or place stay at home; the rest are class-range slots.
Class groups are per region, so a ship's slot is drawn from its class family
there. The Anne's first battles run where they are authored and set the 16
defeat flags as before; the captain does not need any of them. Stanly (hidden by
the maiden voyage's flag) stays fixed.

## Scope

This spec makes the [S.S. ferries](../prds/ss-ferries.md) precise for the
Wayfarer build:
- the schedule;
- the saved voyage state;
- boarding, sailing, arriving and going ashore at each port;
- cabins and their hosts;
- the ships' daily slots;
- what the old menus become;
- how each ship and port is wired.

It builds on the [in-game clock](in-game-clock.md): 12 in-game seconds per
second of play, and the by-day and by-night halves. **It depends on that
clock change** (12×, Day 8–20, Night 0–4) being built first.

**Left to other specs:**
- the trainer-slot generator's rules, beyond the requirement under
  [Passengers](#passengers);
- the maiden voyage of the S.S. Aqua ([HnS traversal](hns-open-world-region-traversal.md));
- the Anne's adventure ([S.S. Anne adventure](frlg-kanto-ss-anne-adventure.md));
- the Seagallop ([Sevii map port](sevii-exploration-map-port.md)), apart
  from the Vermilion sailor's menu and the Birth Island gate below.

**This spec supersedes** these parts of other specs, and each gets a line
pointing here when this ships:
- [S.S. Anne adventure](frlg-kanto-ss-anne-adventure.md): boarding and
  return through the Vermilion menu's "Board S.S. Anne" row, "leaving the ship
  does not start a voyage", and "must not add destinations, voyages or
  schedules";
- [Wayfarer Hoenn entry](wayfarer-hoenn-entry.md): the Aqua circuit as a
  destination menu, "the circuit is the only approved Hoenn-to-HNS route",
  "S.S. Tidal remains unchanged" with its Champion gate, and the validation
  items that check them;
- [Sevii map port](sevii-exploration-map-port.md) "Ferry and recovery": the
  Vermilion selector's fixed rows, the `WayfarerCanSailToBirthIsland`
  predicate, and the audit order;
- [Daily world slots](daily-world-slots.md): ship trainers as one-time
  content.

## Behavior

### Ports and ships

There are three ships (`FERRY_SHIP_AQUA`, `FERRY_SHIP_ANNE` and `FERRY_SHIP_TIDAL`) and seven ports:

| Port | Region | Berth map | Berth kind |
| --- | --- | --- | --- |
| Olivine | Johto | `OlivineCity_PortInside_hns` | dock |
| Vermilion | Kanto | `VermilionCity_PortInside_hns` | dock |
| Slateport | Hoenn | `SlateportCity_Harbor` | dock |
| Lilycove | Hoenn | `LilycoveCity_Harbor` | dock |
| One Island | Sevii | `OneIsland_Harbor_Frlg` | launch |
| Seven Island | Sevii | `SevenIsland_Harbor_Frlg` | launch |
| Melemele | Alola | `MelemeleIsle_hns` | launch |

**Loops.** Each ship sails its loop in order and wraps:
- Aqua: Olivine → Vermilion → Slateport → Lilycove;
- Anne: Vermilion → One Island → Melemele → Seven Island;
- Tidal: Slateport → Lilycove → Seven Island → Olivine.

### The schedule

`Ferry_ShipAt(port, hour)` is a pure function of the port and the in-game
hour:
- by day is `DAY_HOUR_BEGIN` ≤ hour < `DAY_HOUR_END` (8–20);
- by night is every other hour;
- for One Island and Melemele, `NIGHT_HOUR_BEGIN` ≤ hour < `NIGHT_HOUR_END`
  (0–4) returns no ship.

| Port | By day | By night |
| --- | --- | --- |
| Olivine | Aqua | Tidal |
| Vermilion | Aqua | Anne |
| Slateport | Tidal | Aqua |
| Lilycove | Aqua | Tidal |
| Seven Island | Anne | Tidal |
| One Island | Anne | Anne, none 0:00–4:00 |
| Melemele | Anne | Anne, none 0:00–4:00 |

**Ships not yet in service** (see [service](#service)) are returned as none.
The schedule controls **boarding only**: what a berth shows and whether its
gangway opens. It never limits arrival.

**When the table is read:**
- A berth reads the table when its map loads and on every in-game minute while
  the map is shown (the overworld's time-of-day refresh).
- When the answer changes, the ship object switches graphics or hides at once.
- If the player is mid-dialogue with the gangway sailor, the change waits until
  the dialogue ends.

### Voyage state

The voyage state is saved in `PokemonStorage` after `dailySlots`, with a
new-game init. It's packed, at most 16 bytes, and is C-side only, so HnS and
Hoenn maps read the same state through specials.

| Field | Meaning |
| --- | --- |
| `ship` | the ship you're aboard, or none (2 bits) |
| `stop` | index in that ship's loop of the port it's at or sailing to (2 bits) |
| `phase` | in port, or at sea (1 bit) |
| `eventAt` | in-game minute count of the next cast-off (in port) or arrival (at sea): `days × 1440 + hours × 60 + minutes` (u32) |
| `hosts[2]` | the two cabin hosts drawn at boarding (u8 each) |
| `hostParams[2]` | each host's drawn request parameter (u8 each) |
| `cabin` | which host's cabin you've earned, if any (2 bits) |
| `hostsDone` | which hosts' requests are settled, won or lost (2 bits) |
| `slotDay` | the daily-slot day frozen at boarding (u16) |
| `annePending` | the Anne's first departure from Vermilion is armed (1 bit) |

**Lifecycle:**
- **Boarding** sets every field.
- **Going ashore** clears the state to `ship = none`.
- **A blackout** while aboard clears it too. The player returns to their last
  heal location, as on any ship.
- **Saving aboard and continuing** resumes on the same leg. The clock is in the
  save, so `eventAt` still applies.

### Boarding

#### At a dock port

The berth keeps the port's existing ship object and gangway sailor:

| Port | Ship object | Sailor | Arrival tile |
| --- | --- | --- | --- |
| Olivine | the Aqua object at (8,20) | (8,17) | (8,16), or (8,15) while Prof. Oak stands at (8,16) |
| Vermilion | the Aqua object at (8,13) | (8,10) | (8,9) |
| Slateport | the Tidal object at (8,9) | the attendant at (8,10) | (9,11) |
| Lilycove | the Tidal object at (8,9) | the attendant at (8,10) | (8,11) |

**The ship object.** It becomes a variable-graphics object showing the
scheduled ship, or nothing when the berth is empty.
- The Aqua and Tidal sprites are both 96×40.
- The Anne's sprite (`OBJ_EVENT_GFX_SS_ANNE`, 128×64) shows at Vermilion. It
  has to be placed so it sits on the berth's water without covering the
  sailor's tile or the walkway. At Vermilion she is one row further out than
  the Aqua (object at (8,14), rows 11–14 of water, clear of the sailor at
  (8,10)). That still needs a visual check in Porymap.

**Talking to the sailor:**
- **No ship in, or the scheduled ship not in service:** the sailor says which
  ship comes in next and at what hour. Nothing changes.
- **A ship in but no S.S. Ticket:** the sailor asks for one. Nothing changes.
- **A ship in and a Ticket:** the sailor says "Welcome aboard the S.S. Tidal,
  next stop Seven Island." There's no question. The player walks aboard (see
  below), then warps to the ship's [entry tile](#ships).
- **Boarding sets:**
  - `ship`;
  - `stop` = this port's index in that ship's loop;
  - `phase` = in port;
  - `eventAt` = now + 30 in-game minutes;
  - two fresh hosts;
  - `slotDay` = the daily slots' current `stampDay`, so boarding never starts
    a new day by itself.
- **Boarding the Tidal** runs the Hoenn state initialization before the warp
  if Hoenn hasn't been entered yet: the Tidal's maps are Hoenn-source maps
  (see [Region and saves](#region-and-saves)).

**The boarding walk.** A new ferry-only boarding subroutine per dock port
plays the port's existing walk to the ship, without moving the boat sprite
and without `setrespawn`:
- Olivine and Vermilion: the walk from `EnterShip`;
- Slateport: the walk from `SlateportCity_Harbor_EventScript_BoardFerry`;
- Lilycove: the walk from `LilycoveCity_Harbor_EventScript_BoardFerry`.

**Ports with more than one job.** Some sailors already do something else:
- **Vermilion.** `LOCALID_VERMILIONMARINA_SAILOR` is both the S.S. berth's
  sailor and the Seagallop's. Talking to him opens a short menu: "S.S. Aqua"
  or "S.S. Anne" (whichever is in; the row is missing when the berth is
  empty), "Sevii Islands" and Cancel. Choosing the ship boards it as above,
  with no further question. "Sevii Islands" runs today's Seagallop flow.
  Before the captain's reward, the row "Board S.S. Anne" also appears, as
  today, whatever the table says. It's the adventure's entry and runs the
  adventure's own boarding, not the ferry's.
- **Lilycove.** The attendant also owns the [charters](#old-menus-and-event-islands).
  When a charter is available, talking to her opens a menu: the ship in port
  (if any), each available charter, and Cancel. With no charter available,
  she boards with no menu.
- **Slateport during the Aqua escape.** While
  `FLAG_HIDE_SLATEPORT_CITY_HARBOR_PATRONS` is set (from the Aqua escape,
  `VAR_SLATEPORT_HARBOR_STATE` 1, until the Mossdeep Gym clears it), the
  attendant is hidden and the berth is closed: the ship object stays
  hidden whatever the table says, and nobody boards there. Arrivals still land
  at (9,11), which is outside the scene's x=8 triggers.

**The departure board.** A new sign event on a wall tile next to the sailor's
path (none of the docks has a sign) reads the port's by-day and by-night
ships and where each sails next. Melemele has one too, and says the berth is
empty 0:00–4:00. The One Island and Seven Island harbors have no board (their
maps come from the Sevii manifest): their launch sailors announce the next
ship instead. It shows no hosts.

#### At a launch port

At One Island, Seven Island and Melemele, the existing pier gains a **launch
sailor**, and the big ship isn't drawn:

| Port | Launch sailor | Arrival tile |
| --- | --- | --- |
| One Island | (9,5) | (9,4) |
| Seven Island | (9,5) | (9,4) |
| Melemele | a tile near the Melemele captain at (24,33), clear of the boat at (25,35) and of the Samson Oak coordinate trigger at (20,31) and its path | a tile next to him, to the same rules |

The Seagallop's own arrival tile (8,5) stays free. The Melemele tiles are
picked in Porymap: walkable, outside every object, coordinate event and warp,
and on a path to the map's exit.

**Talking to him** gives the same lines and checks as at a dock, with
"anchored offshore" wording. Boarding plays a fade with the ship's horn
(`SE_RG_SS_ANNE_HORN`) instead of a walk.

The Seagallop sailor, the Route 13 captain and the Melemele captain keep their
own menus, checks and arrival tiles.

**One Island's Bill scenes.** One Island Harbor's on-frame scenes for
`VAR_MAP_SCENE_ONE_ISLAND_HARBOR` 1 and 4 walk the player out from the
Seagallop's tile (8,5). They run only after a Seagallop arrival: a ferry
arrival at (9,4) skips them and leaves the var unchanged.

**Alola's first visit.** Going ashore at Melemele while `FLAG_VISITED_ALOLA`
is clear first runs the same first-visit flag block as the Route 13 trip
(hiding the totems, the Tapus, Samson Oak's intro object, the flyover and the
phantoms), moved into a shared script that both paths call.

### Ships

Each ship keeps its own maps. Only the entry, the exit and the cabins change.

| Ship | Maps | Entry tile | Exit |
| --- | --- | --- | --- |
| Aqua | `SSAqua_*_hns` (11 maps) | `SSAqua_1F_hns` (29,3) | the door sailor at (29,2) |
| Anne | `SSAnne_*_Frlg` (25 maps) | `SSAnne_1F_Corridor_Frlg` (19,2) | the gangway warps (19,1), (20,0), already `MAP_DYNAMIC` |
| Tidal | `SSTidalCorridor`, `SSTidalRooms`, `SSTidalLowerDeck` | `SSTidalCorridor` (1,10) | the exit sailor at (1,11) |

**The exit.** While `phase` is in port, the exit takes you to the arrival tile
of the port at `stop`.
- **Anne:** the corridor's on-load `setdynamicwarp` picks that port from a
  special instead of always Vermilion. Cast-off and arrival also refresh it,
  since they can happen while the player stands in the corridor. At sea, the
  dynamic warp points back at the corridor's own (19,2), so taking the
  gangway only returns the player there, and a coordinate event at (19,2)
  says "We're at sea".
- **Aqua and Tidal:** the exit sailor's script warps there, replacing the
  Aqua's fixed Vermilion exit and the Tidal's `VAR_SS_TIDAL_STATE` exit.
  At sea, the exit sailor says "We're at sea" and does nothing.
- **With no voyage** (`ship` is none, for example a save made aboard before
  this change), the exit takes you to a fixed home port: the Aqua and the Anne
  to Vermilion (8,9), the Tidal to Slateport (9,11) with the Hoenn first-entry
  setup.

**Going ashore:**
1. Run `WayfarerPrepareHoennEntry` when the port is in Hoenn and Hoenn hasn't
   been entered yet, just as for today's Vermilion-to-Slateport trip.
2. Set the respawn to the port's heal location (`HEAL_LOCATION_MELEMELE_ISLE_HNS`
   for Melemele).
3. Clear the voyage state.
4. Warp.

**The story trips keep their own scripts.**
- **The maiden voyage is pending** when the player's start has a maiden voyage
  (`WayfarerCanUseAquaMaidenVoyage()`, true only for the Johto start) and
  `VAR_SSAQUA_STATE` < 8. While it's pending, the Olivine sailor runs today's
  maiden-voyage script whatever the table says, the Aqua is out of service
  everywhere, and its interior keeps its maiden-voyage entry, scenes and exit.
- **For every other start, and once the voyage is over,** the maiden voyage
  never runs. The maiden-voyage objects stay hidden on Aqua maps
  (`FLAG_HIDE_SSAQUA_CAPTAINS_ROOM_GRANDDAUGHTER` and
  `FLAG_HIDE_SSAQUA_ROOM_SSE_GRANDDAUGHTER` are set at New Game for the
  Kanto and Hoenn starts), and the Aqua maps' maiden-voyage frame and
  coordinate scripts do nothing while `ship` is set.
- **The Anne's adventure** before the captain's reward keeps its entry (the
  Vermilion menu row) and exit unchanged.
- The ferry's entry, exit and announcements apply only while `ship` is set.

**Retired in Wayfarer:**
- the Tidal's cruise state machine (`VAR_SS_TIDAL_STATE`, `CountSSTidalStep`,
  the porthole walk and the bed's `ProgessCruiseAfterBed`);
- the Aqua's fixed Vermilion exit.

Other builds keep them.

**The Tidal's Town Map location.** The Tidal maps are `MAPSEC_DYNAMIC`, and
`GetSSTidalLocation` read the retired cruise state. A Hoenn-source map's Town
Map shows only Hoenn, so in Wayfarer it reads the voyage state and shows a
Hoenn place: Slateport or Lilycove when the ship is at, or sailing to, that
port; Lilycove when it's at, or sailing to, Seven Island; Slateport when it's
at, or sailing to, Olivine; Slateport with no voyage.

**Scott aboard the Tidal.** `SSTidalCorridor`'s Scott scene ("Congratulations,
League Champion!") runs only once `FLAG_SYS_GAME_CLEAR` (Hoenn's League) is
set. Until then Scott is hidden.

### Sailing

A field task checks the voyage state at least once per in-game minute while
the player is on a ship map with `ship` set, and once on loading a ship map.
The check only acts when the field is free: no script running, no menu, no
battle.

**Catching up.** Events are scheduled, not reset by when they're noticed:
cast-off sets `eventAt += 360` and arrival `eventAt += 30`. If the clock has
passed several events while the player was busy (a long battle, a menu, or a
save loaded later), each elapsed transition is applied in order, and only the
last one is announced.

**Cast-off** happens when `phase` is in port and now ≥ `eventAt`:
1. The PA announces it: "All ashore that's going ashore! Next stop, Seven
   Island."
2. `phase` becomes at sea.
3. `stop` moves to the next port in the loop.
4. `eventAt` += 360.

There's no separate last-call timer: the announcement is the last call, and
the exit closes as it plays.

**Arrival** happens when `phase` is at sea and now ≥ `eventAt`:
1. The PA announces it: "Now arriving at Melemele Island."
2. `phase` becomes in port.
3. `eventAt` += 30.

The Anne's cast-off toward Melemele and her arrival at Melemele both name Alola
("...the Alolan islands!"). She's the only ship that does.

Arrival ignores the schedule.

| Phase | In-game time | Play time |
| --- | --- | --- |
| In port, after boarding or arriving | 30 min | 2.5 min |
| At sea | 6 h | 30 min |

### Cabins

#### Hosts

Each ship has a host pool. Every host has:
- a graphics id, a name line and a cabin room on that ship;
- a request kind, with a list of request parameters.

**At boarding,** two distinct hosts are drawn with `Random()` from the ship's
pool, each with one parameter from its list, and saved.

**Placement.** Drawn hosts appear in their cabin rooms through the same
template rewrite the daily slots use, on a tile in the room clear of the
room's other objects. Hosts not drawn don't appear.

**Talking to a host:**
- **Request settled, cabin not theirs:** they make small talk.
- **Their cabin is earned:** they point at the bed.
- **Otherwise:** they state their request and check it at once, with a
  battle or a menu where the kind needs one.
  - **Met:** the host's cabin becomes the player's (`cabin`), and their
    request is settled.
  - **Failed:** the request stays open, except for kinds marked "one try",
    which are settled either way.

Earning one cabin doesn't settle the other host; a second earned cabin
replaces nothing and is just another bed.

**v0 request kinds:**

| Kind | Check | Parameters | One try |
| --- | --- | --- | --- |
| Battle | win a single trainer battle against the host | the host's party template | yes |
| Double battle | win a double battle against the host pair | the pair's template | yes |
| Bring an item | the Bag holds the item; it's removed on success | Oran, Pecha, Cheri Berry; Antidote, Full Heal; Nugget; Moomoo Milk | no |
| Show a type | a party Pokémon, not an Egg, has the type | any of the 18 types | no |
| Show a species | a party Pokémon is the species | per-host list | no |
| Show a colour | a party Pokémon's species body colour | the body colours | no |
| Field move | a party Pokémon knows the move | Strength, Flash, Cut, Rock Smash | no |
| Fully evolved | a party Pokémon's species has no further evolution and has a pre-evolution | none | no |
| From your region | a party Pokémon's species is in the regional Pokédex of the player's start region | none | no |
| Friendship | a party Pokémon has friendship ≥ 220 | none | no |
| Key item | the Bag holds the item, which isn't removed | Poké Flute | no |
| Quiz | answer a three-choice question about the ferry schedule | a list of questions | yes |
| Coin flip | `Random()` < 50% | none | yes |

Host battles never white out: losing ends the battle, restores every fainted
party Pokémon to 1 HP, settles the host, and leaves the player aboard.

**Host pools:** v0 gives each ship about eight hosts spread across these kinds:
- the Anne: tourists, well-off guests, an Alolan tourist (From your region), a
  gentleman (Battle);
- the Aqua: sailors (Field move), merchants (Bring an item), a gambler (Coin
  flip);
- the Tidal: families (Show a colour or type), fishermen, a breeder
  (Friendship), honeymooners (Double battle).

The exact host table is content and lives in the implementation's data file.

**Hosts deferred to later:**
- the lost brooch, which needs a hidden item placed aboard per boarding;
- the fisherman's "caught with a rod", which needs the catch method, which the
  game doesn't save.

#### Sleeping

The bed in an earned cabin heals nothing.
- **At sea:** the bed asks "Sleep until we reach Lilycove?". Yes fades out,
  moves the clock forward to `eventAt` with `FakeRtc_AdvanceTimeBy`, runs the
  arrival, and reloads the map with the player beside the bed.
- **In port:** the bed says the ship is in port and nothing happens.

Sleeping never moves the clock past `eventAt`, so it never skips time in port.
This is the clock's one sanctioned forward jump. The
[in-game clock](in-game-clock.md#days-only-go-forward) lists it as an
exception, and the daily update and berries catch up as they do for any
passed time.

**Beds.** No ship bed heals in Wayfarer: the Aqua's `SSAqua_PlayersRoom_hns`
and the Tidal's `SSTidalRooms` beds lose their free heal.
- Each host is tied to one cabin room and, where the room has one, one bed
  tile, listed in the host table.
- On the Tidal, all eight cabins are rooms of the single `SSTidalRooms` map;
  each Tidal host owns one cabin of it, and the existing bed at (15,11) and
  (15,12) belongs to that cabin's host. Other Tidal cabins get a bed event on
  their bed metatile.
- The Aqua's `PlayersRoom` belongs to no host and is just a room.
- Where a host's room has no bed, the host offers to let you sleep: talking to
  them again asks the same question.

### Passengers

**Ship trainers rotate.**
- The S.S. Anne, S.S. Aqua and S.S. Tidal trainers leave the reviewed
  "one-time ship content" exclusion in the trainer-slot generator.
- Their decks and rooms become daily slot places of a new `liner` place type,
  separate from the existing `ship` type (the Abandoned Ship), and the
  [daily world slots](daily-world-slots.md) place-type list gains it,
  so they rotate like route trainers.
- Ship slots draw like any other slot: a ship's sailor or gentleman slot is
  filled from that class's regional group, so route trainers of fitting
  classes turn up aboard. `liner` itself only holds the trainers whose own
  lines are about the ship.
- Cabin hosts stand apart from the slots and are never drawn into them.

**The Anne's completion check.**
- `WayfarerSSAnneHasUnfinishedContent` stays for the adventure's own lines.
- Service starts on the captain's reward alone (see [service](#service)).
- The 16 one-time defeat flags keep recording the adventure's battles, but
  the ship's slots draw from the rotation, not from those flags.

**Freezing the daily slots aboard.**
- While `ship` is set, `DailySlots_GetDay` returns `slotDay`, so the day's
  draw (ship and shore alike) doesn't change while you stay aboard.
- When you go ashore, the state clears, and the next map load takes the real
  day as usual.
- `DailySlots_GetDay` has one caller, `DailySlots_OnMapLoad`, so everything
  stamped from it stays on `slotDay` while aboard: the phone's picks and gifts,
  the ready and gift snapshots, and calls that name where a trainer is today.
- The daily update (`DoTimeBasedEvents`: daily flags, berries, `VAR_DAYS`)
  isn't frozen, and neither are leagues, which count `VAR_DAYS`.

### Service

| Ship | In service when |
| --- | --- |
| Aqua | no maiden voyage is pending (see [Ships](#ships)) |
| Anne | `FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED` is set |
| Tidal | always |

- The Ticket gates **boarding only**, never whether a ship is in service: a berth shows its in-service ship even to a
  player with no Ticket, and the sailor then asks for one.
- `WayfarerCanUseRegularAqua` and its per-start policies go. The Ticket is
  the only gate for any start, including Kanto.
- The Seagallop's Birth Island trip, which also read
  `WayfarerCanUseRegularAqua`, now needs only the Aurora Ticket
  (`WayfarerCanSailToBirthIsland` = the Bag holds `ITEM_AURORA_TICKET`), as in
  FireRed and LeafGreen. Navel Rock keeps its league-win and Mystic Ticket
  check.
- The Tidal no longer checks `FLAG_SYS_GAME_CLEAR`. The Slateport and
  Lilycove ship objects no longer hide on it.
- The Ticket is checked with `checkitem ITEM_SS_TICKET` at every gangway and
  launch, and is never removed.
- Ticket sources are unchanged. The Hoenn start's Ticket, which the retired
  Slateport Aqua attendant used to grant, moves to the Slateport gangway
  sailor under the same `WayfarerCanReceiveSlateportTicket` condition.

**The Anne's first departure:**
1. When the captain gives his reward, `annePending` is armed.
2. If the player is aboard the Anne then, the voyage state is set as for
   boarding at Vermilion, the table ignored this once. If they've already
   left, the Vermilion sailor offers "S.S. Anne" the next time they talk to
   him, whatever the table says, and boarding then does the same.
3. The PA announces "We sail for One Island in half an hour."
4. Walking down the gangway before cast-off leaves the player in Vermilion
   with `annePending` cleared. From then on the Anne follows the table.

### Old menus and event islands

**Removed in Wayfarer:**
- The Olivine port menu (`MULTI_OLIVINE_HARBOR`). The maiden voyage's
  boarding at Olivine stays.
- Vermilion's "Other Destinations" row in `DrawWayfarerVermilionPortMenu`.
  The menu becomes the one under
  [Ports with more than one job](#at-a-dock-port).
- The Slateport Wayfarer Aqua attendant.
- The Slateport and Lilycove Tidal menus.
- The Battle Frontier ferry's departures from Slateport.

**Lilycove's charters.** The Lilycove attendant offers each charter while its
condition holds:

| Charter | Condition |
| --- | --- |
| Southern Island | the Bag holds `ITEM_EON_TICKET` |
| Faraway Island | the Bag holds `ITEM_OLD_SEA_MAP` |
| Battle Frontier | `FLAG_MET_SCOTT_ON_SS_TIDAL` (Scott's invite, after Hoenn's League) |

- No `FLAG_SYS_GAME_CLEAR` gate on the attendant and no `FLAG_ENABLE_SHIP_*`
  flags. Wayfarer's real sources for the Eon Ticket and the Old Sea Map
  (Steven at Fuchsia, the Vermilion find) never set those flags.
- The first-time "may I see your ticket" scenes play as today and still set
  their `FLAG_SHOWN_*` flags. Their first-time detection
  (`GetEonTicketState` and its siblings in `LilycoveCity_Harbor`) and the C
  menu builder in `script_menu.c` drop their `FLAG_ENABLE_SHIP_*` checks in
  Wayfarer and read the conditions above.
- The trips use today's scenes. Returns land at Lilycove (8,11).
- The Battle Frontier's ferry attendant returns to Lilycove only; its
  Slateport option goes.

Birth Island and Navel Rock leave the Lilycove and Olivine menus and stay on
the Seagallop only.

### Region and saves

- **Region aboard.** Ship maps carry fixed regions (the Aqua's are Johto, the
  Anne's Kanto, the Tidal's Hoenn-source). While `ship` is set, loading a ship
  map doesn't change the saved current region or mark a region visited
  (`WayfarerUpdateHnsRegionContextForMap` and the Hoenn equivalent skip ship
  maps). Going ashore sets the region through the port's normal map load.
- **Hoenn first entry** is split in two:
  - **Initializing Hoenn's state** (the one-time Hoenn flags and vars) runs
    before the first warp onto any Hoenn-source map: boarding the Tidal, or
    going ashore at Slateport or Lilycove, whichever comes first. It sets
    neither the saved region nor the respawn, so boarding the Tidal at Olivine
    or Seven Island leaves both where they were.
  - **Arriving in Hoenn** (`WayfarerPrepareHoennEntry`, generalized to take the
    port, the arrival tile and the heal location, the shape of
    `Test_WayfarerPrepareHoennEntryAt`) runs when going ashore at Slateport or
    Lilycove: it validates the tile and heal location, sets the saved region
    to Hoenn and sets the respawn.
  - If either step fails, boarding or going ashore is refused with nothing
    changed.
- Ship maps keep their own map sections and music.
- Ferry checks run in C specials, so Hoenn-bank and HnS-bank maps share the
  voyage state without common vars.
- **Saves.** `ship` none is encoded as 0, and the voyage struct is zeroed at
  New Game. Prerelease saves aren't migrated (see the repo's save policy); a
  save made aboard with no voyage leaves by the [home-port exit](#ships). The
  struct has its own size assert.

### Build audits

Some build-time audits enforce the menus this spec removes, and change with it:
- `game/tools/wayfarer_hoenn_entry/generate.py` (run by the build) and its
  `test_generate.py` stop requiring `WayfarerCanUseRegularAqua`,
  `MULTI_VERMILION_HARBOR`, the Slateport Aqua attendant and an unchanged
  Tidal attendant. They assert instead that the Slateport and Lilycove berths
  and the Vermilion and Olivine sailors call the ferry boarding specials.
- `game/tools/league_circuit/test_scripts.py` stops slicing
  `VermilionPort_EventScript_ChoseSlateport` and checks the maiden-voyage
  scripts it still needs.

## Acceptance

**Unit tests:**
- `Ferry_ShipAt` across every port and every hour boundary: 3:59, 4:00, 7:59,
  8:00, 19:59 and 20:00, plus One Island and Melemele at 0:00–3:59.
- Boarding: the state fields.
- Cast-off and arrival: timing, including catching up after a long battle.
- Sleep: jumps exactly to `eventAt` and never across time in port.
- The daily-slot freeze holds while aboard and releases ashore.
- Each request kind passes and fails.
- Service gates for each start with and without the Ticket, and the Tidal
  without game clear.

**E2E:**
- Board each ship at each of its ports and ride a full loop awake and asleep.
- Get off at every stop.
- Save and continue at sea and in port.
- Black out aboard.
- The Anne's first departure, both staying and getting off.
- A Kanto start rides the Aqua with Bill's Ticket.
- The Lilycove charters.
- The berth graphics by day and by night, including the Anne at Vermilion.
- A first Alola visit by the Anne, a host battle lost without whiting out,
  and the Town Map opened aboard the Tidal.
- Update these existing playbooks and journeys:
  - `wayfarer-ports`
  - `hns-ss-aqua-traversal`
  - `wayfarer-hoenn-entry`
  - `wayfarer-ss-anne-adventure`
  - `wayfarer-hoenn-origin-travel`
  - `wayfarer-origin-hoenn-campaign`
  - `hns-standard-rod-givers` (uses `beginWayfarerRegularAquaDeparture`)
  - `wayfarer-sevii-exploration` (the Vermilion menu and Birth Island gate)

**Builds:** the release, `make check` and RAM budget builds pass. The voyage
state stays within `PokemonStorage`'s budget.

## Open questions

- The Anne sprite's placement at Vermilion's berth, and each launch port's
  sailor and arrival tiles, are set in Porymap during implementation.
- The host table's exact names, lines and parameters are content for the
  implementation.

## References

- [S.S. Anne adventure](frlg-kanto-ss-anne-adventure.md)
- [HnS open-world traversal](hns-open-world-region-traversal.md)
- [Wayfarer Hoenn entry](wayfarer-hoenn-entry.md)
- [Sevii map port](sevii-exploration-map-port.md)
- [Daily world slots](daily-world-slots.md)
- [In-game clock](in-game-clock.md)
