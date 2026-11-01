# S.S. ferries

PRD: [S.S. ferries](../prds/ss-ferries.md)
Implemented: No

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
second of play, and the by-day and by-night halves.

**Left to other specs:**
- the trainer-slot generator's rules, beyond the requirement under
  [Passengers](#passengers);
- the maiden voyage of the S.S. Aqua ([HnS traversal](hns-open-world-region-traversal.md));
- the Anne's adventure ([S.S. Anne adventure](frlg-kanto-ss-anne-adventure.md));
- the Seagallop ([Sevii map port](sevii-exploration-map-port.md)).

**This spec supersedes** the parts of those specs that say:
- the Anne never transports anyone;
- the Aqua circuit is a destination menu ([Wayfarer Hoenn entry](wayfarer-hoenn-entry.md#s-s-ticket-and-s-s-tidal));
- the Tidal is unchanged;
- ship trainers are one-time ([daily world slots](daily-world-slots.md)).

Each of those specs gets a line pointing here when this ships.

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
| Olivine | the Aqua object at (8,20) | (8,17) | (8,16) |
| Vermilion | the Aqua object at (8,13) | (8,10) | (8,9) |
| Slateport | the Tidal object at (8,9) | (8,10) | (9,11) |
| Lilycove | the Tidal object at (8,9) | (8,10) | (8,11) |

**The ship object.** It becomes a variable-graphics object showing the
scheduled ship, or nothing when the berth is empty.
- The Aqua and Tidal sprites are both 96×40.
- The Anne's sprite (`OBJ_EVENT_GFX_SS_ANNE`, 128×64) shows at Vermilion. It
  has to be placed so it sits on the berth's water without covering the
  sailor's tile or the walkway. That needs a visual check in Porymap; this
  spec doesn't fix pixel offsets.

**Talking to the sailor:**
- **No ship in, or the scheduled ship not in service:** the sailor says which
  ship comes in next and at what hour. Nothing changes.
- **A ship in but no S.S. Ticket:** the sailor asks for one. Nothing changes.
- **A ship in and a Ticket:** the sailor says "Welcome aboard the S.S. Tidal,
  next stop Seven Island." There's no question. The player plays the
  port's existing boarding walk (the Olivine and Vermilion `EnterShip`
  movement or Lilycove's `BoardFerry`, without sailing the ship sprite off),
  then warps to the ship's [entry tile](#ships).
- **Boarding sets:**
  - `ship`;
  - `stop` = this port's index in that ship's loop;
  - `phase` = in port;
  - `eventAt` = now + 30 in-game minutes;
  - two fresh hosts;
  - `slotDay` = today's daily-slot day.

**The departure board.** Each dock port's existing sign, or a new sign event
next to the sailor if there is none, reads the port's by-day and by-night
ships and where each sails next. At One Island and Melemele it also says the
berth is empty 0:00–4:00. It shows no hosts.

#### At a launch port

At One Island, Seven Island and Melemele, the existing pier gains a **launch
sailor**.

**Where he stands:**
- One Island and Seven Island: in the Seagallop harbor room, on a tile beside
  the Seagallop sailor at (8,6).
- Melemele: near the Route 13 captain at (24,33), clear of the boat at (25,35).
- In all three: on a walkable tile, outside every existing object, coordinate
  event and warp, and on a path to the map's exit.
- Each port has a fixed arrival tile next to him, chosen to the same rules.
- The big ship isn't drawn.

**Talking to him** gives the same lines and checks as at a dock, with
"anchored offshore" wording. Boarding plays a fade with the ship's horn
(`SE_RG_SS_ANNE_HORN`) instead of a walk.

The Seagallop sailor, the Route 13 captain and the Melemele captain keep their
own menus, checks and arrival tiles unchanged.

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
  special instead of always Vermilion.
- **Aqua and Tidal:** the exit sailor's script warps there, replacing the
  Aqua's fixed Vermilion exit and the Tidal's `VAR_SS_TIDAL_STATE` exit.
- **While at sea,** the exit says "We're at sea" and does nothing.

**Going ashore:**
1. Run `WayfarerPrepareHoennEntry` when the port is in Hoenn and Hoenn hasn't
   been entered yet, just as for today's Vermilion-to-Slateport trip.
2. Set the respawn to the port's heal location (`HEAL_LOCATION_MELEMELE_ISLE_HNS`
   for Melemele).
3. Clear the voyage state.
4. Warp.

**Outside a voyage, ship interiors are not reachable.** The Aqua's maiden
voyage (`VAR_SSAQUA_STATE` < 8) and the Anne's adventure before the captain's
reward keep their current entry and exit scripts unchanged. The ferry's
entry, exit and announcements apply only while `ship` is set.

**Retired in Wayfarer:**
- the Tidal's cruise state machine (`VAR_SS_TIDAL_STATE`, `CountSSTidalStep`,
  the porthole walk and the bed's `ProgessCruiseAfterBed`);
- the Aqua's fixed Vermilion exit.

Other builds keep them.

### Sailing

A field task checks the voyage state at least once per in-game minute while
the player is on a ship map with `ship` set. The check only acts when the
field is free: no script running, no menu, no battle. If the clock has passed
`eventAt` by more than one step while the player was busy, the event still
happens once and catches up to the current time.

**Cast-off** happens when `phase` is in port and now ≥ `eventAt`:
1. The PA announces it: "All ashore that's going ashore! Next stop, Seven
   Island."
2. `phase` becomes at sea.
3. `stop` moves to the next port in the loop.
4. `eventAt` = the cast-off minute + 360.

There's no separate last-call timer: the announcement is the last call, and
the exit closes as it plays.

**Arrival** happens when `phase` is at sea and now ≥ `eventAt`:
1. The PA announces it: "Now arriving at Melemele Island."
2. `phase` becomes in port.
3. `eventAt` = the arrival minute + 30.

The Anne's arrival at Melemele names Alola ("...the Alolan islands!"). She's
the only ship that does.

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

**Which bed:**
- Cabins that already have a bed (the Aqua's `SSAqua_PlayersRoom_hns` and the
  Tidal's `SSTidalRooms` beds) reuse it.
- A cabin with no bed uses the host's room bed metatile, if it has one, with a
  new bed event. Otherwise the host offers to let you sleep: talking to them
  again asks the same question.

### Passengers

**Ship trainers rotate.**
- The S.S. Anne, S.S. Aqua and S.S. Tidal trainers leave the reviewed
  "one-time ship content" exclusion in the trainer-slot generator.
- Their decks and rooms become daily slot places of a new `liner` place type,
  so they rotate like route trainers.
- Other trainers whose class fits `liner` (sailors, gentlemen, ladies,
  swimmers, fishermen, tubers, beauties, rich boys) may be drawn there.
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
- The daily update (`DoTimeBasedEvents`: daily flags, berries, `VAR_DAYS`)
  isn't frozen, and neither are leagues or phone calls that count days.

### Service

| Ship | In service when |
| --- | --- |
| Aqua | the Bag holds `ITEM_SS_TICKET` and `VAR_SSAQUA_STATE` is not between 1 and 7, so a maiden voyage that has started has also finished |
| Anne | the captain's reward has been received |
| Tidal | always, for a Ticket holder |

- `WayfarerCanUseRegularAqua` and its per-start policies go. The Ticket is
  the only gate for any start, including Kanto.
- The Tidal no longer checks `FLAG_SYS_GAME_CLEAR`. The Slateport and
  Lilycove ship objects no longer hide on it.
- The Ticket is checked with `checkitem ITEM_SS_TICKET` at every gangway and
  launch, and is never removed.
- Ticket sources are unchanged. The Hoenn start's Ticket, which the retired
  Slateport Aqua attendant used to grant, moves to the Slateport gangway
  sailor under the same `WayfarerCanReceiveSlateportTicket` condition.

**The Anne's first departure:**
1. When the captain gives his reward, `annePending` is armed.
2. If the player is aboard the Anne then, or the next time they board her at
   Vermilion, the voyage state is set as for boarding at Vermilion. The table
   is ignored this once.
3. The PA announces "We sail for One Island in half an hour."
4. Walking down the gangway before cast-off leaves the player in Vermilion
   with `annePending` cleared. From then on the Anne follows the table.

### Old menus and event islands

**Removed in Wayfarer:**
- The Olivine port menu (`MULTI_OLIVINE_HARBOR`). The maiden voyage's
  boarding at Olivine stays.
- Vermilion's "Other Destinations" and "Board S.S. Anne" rows in
  `DrawWayfarerVermilionPortMenu`. The Vermilion sailor keeps Sevii Islands
  and Cancel, and the Anne is boarded at the berth like any ship.
- The Slateport Wayfarer Aqua attendant.
- The Slateport and Lilycove Tidal menus.
- The Battle Frontier ferry's departures from Slateport.

**Lilycove's charters.** The Lilycove sailor keeps a charter menu that appears
only when at least one charter is available. It has the same conditions,
scenes and returns as today's Lilycove menu:
- Southern Island with the Eon Ticket;
- Faraway Island with the Old Sea Map;
- the Battle Frontier.

Returns from those trips land at Lilycove (8,11), as today. Birth Island and
Navel Rock leave the Lilycove and Olivine menus and stay on the Seagallop
only.

### Region and saves

- Arriving changes region through the normal map-load region context
  (`WayfarerSetSavedCurrentRegion`).
- Ship maps keep their own map sections and music.
- Ferry checks run in C specials, so Hoenn-bank and HnS-bank maps share the
  voyage state without common vars.
- The Hoenn first-entry setup runs on the first ashore step into Hoenn, from
  any ship.

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
- Update these existing playbooks and journeys:
  - `wayfarer-ports`
  - `hns-ss-aqua-traversal`
  - `wayfarer-hoenn-entry`
  - `wayfarer-ss-anne-adventure`
  - `wayfarer-hoenn-origin-travel`
  - `wayfarer-origin-hoenn-campaign`

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
