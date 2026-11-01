# S.S. ferries

Specifications: none yet.

Design status: agreed for v0. Times build on the
[in-game clock](in-game-clock.md#design)'s times of day and its by-day and
by-night halves.

## Intent

Wayfarer's regions are linked by a patchwork of boats, each with its own
destination menu: the S.S. Aqua's three-way menu, the Seagallop's island
pages, a deckhand's Route 104 hop, a captain's one boat to Alola. Picking a
destination from a list makes the sea a loading screen.

The three big ships, the **S.S. Aqua**, the **S.S. Anne** and the **S.S. Tidal**,
become the backbone instead: real ships on short loops, each calling at its
own ports in order. Between them they reach every region. No single ship goes
everywhere, so crossing the world means reading the harbor board, catching
the right ship and changing ships at a shared port. Ports trade ships between
day and night, so the harbors feel alive and the trip is a small plan of its
own. The other boats stay as extra, slower ways around.

## Design

### Ships and routes

Each ship sails a one-way loop of four ports and then starts over:

| Ship | Loop |
| --- | --- |
| **S.S. Aqua** | Olivine (Johto) → Vermilion (Kanto) → Slateport (Hoenn) → Lilycove (Hoenn) → Olivine |
| **S.S. Anne** | Vermilion (Kanto) → One Island (Sevii) → Melemele (Alola) → Seven Island (Sevii) → Vermilion |
| **S.S. Tidal** | Slateport (Hoenn) → Lilycove (Hoenn) → Seven Island (Sevii) → Olivine (Johto) → Slateport |

Every region is on at least one loop, and Alola only on the Anne's. Five of
the seven ports are on two loops; those are where you change ships.

**The Anne's secret.** In the old games nobody knew where the S.S. Anne went
after Vermilion. In Wayfarer it sails to Alola. The captain, the crew and the
passengers let it show: the next-stop announcement, a holiday-maker heading
home to Melemele, a gentleman back from the islands.

### By day and by night

A port hosts one ship at a time. Ports on two loops trade ships at 8:00 and
20:00, the [clock's](in-game-clock.md#design) by-day and by-night halves, an
hour of play each. One Island and Melemele have only the Anne; she stays all
day but anchors offshore at night (0:00–4:00, 20 minutes of play), when the
berth is empty.

| Port | By day (8:00–20:00) | By night (20:00–8:00) |
| --- | --- | --- |
| Olivine (Johto) | Aqua | Tidal |
| Vermilion (Kanto) | Aqua | Anne |
| Slateport (Hoenn) | Tidal | Aqua |
| Lilycove (Hoenn) | Aqua | Tidal |
| Seven Island (Sevii) | Anne | Tidal |
| One Island (Sevii) | Anne | Anne, away 0:00–4:00 |
| Melemele (Alola) | Anne | Anne, away 0:00–4:00 |

The table is the whole schedule, every day the same. It isn't a simulation:
the Anne can be in Vermilion and Melemele at the same hour, and nobody minds.
It only says which ship you can **board** where and when.

### Boarding

You board the ship that is in port by talking to its sailor at the gangway.
Boarding means sailing: the sailor names the next stop and asks whether
you're coming aboard. Yes takes you aboard and the ship casts off; No leaves
you on the pier. An empty berth's sailor says which ship comes next and when.

### Riding

The trip is played out aboard, in real time. Each leg to the next port takes
an in-game hour, 5 minutes of play, and you can walk the decks, visit the
cabins and battle the passengers on the way. The clock keeps running.

**The cabin bed.** Every ship has a cabin with a bed. It heals your party,
and while the ship is at sea it lets you sleep until the next port, which
moves the clock to the arrival time. It never skips more than the rest of the
current leg.

**Arriving.** When the ship reaches a port, the announcement names it and you
choose: **go ashore** or **stay aboard**. Going ashore puts you on that port's
pier. Staying aboard sails on to the next stop, and so on round the loop.
Arrival is never limited by the table: once aboard, you can ride the whole
loop and get off anywhere on it, at any hour.

### Ticket and service

One **S.S. Ticket** works on all three ships. It's checked at every gangway
and never used up. Its sources stay as they are: the S.S. Aqua's maiden
voyage, and Bill in Kanto.

A ship joins the network when its story is done:

- **S.S. Aqua:** after its maiden voyage, as today. Until then it keeps its
  maiden-voyage boarding at Olivine.
- **S.S. Anne:** after the captain's adventure. Until then she stays docked
  at Vermilion for the adventure, as today; afterwards the "visiting hours are
  over" refusal is gone and she sails her loop.
- **S.S. Tidal:** from the start, for anyone holding the Ticket.

Before a ship is in service, its slots in the table stay empty.

### Departure boards

Every port with an S.S. berth has a departure board: which ship is in port by
day and by night, the hours a single-ship port's berth is empty, and where
each ship sails next. That is the only timetable; the Pokégear and Town Map
don't show ships.

## Boundaries

- **The other boats stay** as they are: the Seagallop between Vermilion and
  the Sevii Islands (with its Birth Island and Navel Rock tickets), the Route
  104 boat, and the Route 13 boat to Alola. They are extras, not part of the
  network.
- **The ships' menus go.** The S.S. Aqua's Olivine, Vermilion and Slateport
  destination menu and the S.S. Tidal's Slateport, Lilycove and Battle
  Frontier menu are replaced by the loops. The Battle Frontier isn't a stop.
- **No new ports.** Ships call at existing harbors and landings: Vermilion's,
  Olivine's, Slateport's and Lilycove's ports, the One Island and Seven Island
  Seagallop harbors, and Melemele's boat landing. Cinnabar gets no pier.
- **No new content aboard.** The ships keep their existing rooms, trainers and
  daily world slots ([world items](../specs/world-items.md), [daily world
  slots](daily-world-slots.md)). The Anne's Alolan flavour is lines, not new
  trainers.
- **The cabin bed is the only time skip**, and it only skips the rest of a
  leg. There is no waiting bench at the harbors.
- **No change to the S.S. Ticket's source**, the Anne's captain's adventure or
  the Aqua's maiden voyage, other than what happens after them.
- Only the Wayfarer build.

## Balance

- A leg is 5 minutes of play aboard. The longest loop trip, three legs, is
  15 minutes, or less with the cabin bed.
- The longest wait for a particular ship at a shared port is an hour of play;
  at One Island or Melemele it is 20 minutes. Five shared ports mean another
  ship is often in that gets you somewhere useful, so a wait is rarely forced.
- The challenge is planning, not reflexes: the table never changes, so
  missing a ship is a choice you can see coming.

## Presentation

- Gangway sailor: "This is the S.S. Tidal, sailing for Seven Island. Coming
  aboard?" Without a Ticket: the sailor asks for one.
- Empty berth: "The S.S. Anne's anchored offshore tonight. She's back at 4:00."
- Arrival: "Now arriving at Melemele Island. Going ashore?"
- The Anne's next-stop announcement for Melemele names Alola outright, the
  first time anyone does.

## Open questions

- How a big ship shows at the One Island and Seven Island Seagallop harbors
  and at Melemele's landing, which were drawn for small boats.
- Whether the Route 13 boat to Alola keeps its Strange Souvenir check now that
  the Anne also sails there.
- Which interior maps each ship uses for the voyage, and how the decks look at
  sea.

## References

- [In-game clock](in-game-clock.md)
- [S.S. Anne adventure](../specs/frlg-kanto-ss-anne-adventure.md), which
  promised this ferry change
- [HnS open-world traversal](../specs/hns-open-world-region-traversal.md), for
  the Aqua's maiden voyage
- [Wayfarer Hoenn entry](../specs/wayfarer-hoenn-entry.md), whose Aqua circuit
  and Slateport attendant this replaces
- [Sevii exploration](sevii-exploration-port.md), for the Seagallop
