# S.S. ferries

Specifications: [S.S. ferries](../specs/ss-ferries.md)

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

**Where the ships berth.** At Vermilion, Olivine, Slateport and Lilycove the
ship ties up at the existing berth, and the berth shows whichever ship is in:
Vermilion shows the Aqua by day and the Anne by night, so the Anne is seen at
her dock for the first time. One Island, Seven Island and Melemele were built
for small boats, so there the big ship anchors offshore and a launch takes you
out from the existing pier.

### Boarding

You board the ship that is in port by walking up its gangway. The sailor
there checks your Ticket and tells you the next stop; there is no question
to answer. Where the same sailor also runs the Seagallop (Vermilion) or the
event-island charters (Lilycove), a short menu picks which. Once you're aboard, the ship stays in port for a short while and
then casts off. Until it does, you can walk back down the gangway. An empty
berth's sailor says which ship comes next and when.

### Riding

Sailing is a commitment. Each leg to the next port takes **6 in-game hours**,
a quarter of a day, so a whole loop takes a day. While you sail, the world
moves on: you arrive at another time of day, maybe after your connection's
window has closed, and the day's slots and rematches may have turned over.

You spend a leg one of two ways:

- **Awake on deck.** The 6 hours play out in real time, about 30 minutes of
  play. You walk the decks and battle the passengers. The clock keeps running.
- **In a cabin.** You sleep and wake as the ship reaches the next port. The
  clock jumps to the arrival time, so the world still moves on 6 hours, but it
  costs you only seconds of play. Sleeping never skips time in port.

**Earning a cabin.** Cabins aren't sold. When you board, **two random
passengers** aboard each offer their cabin, and each wants something in
return. Meet either request and that cabin is yours until you go ashore: you
can sleep in it every leg until then. Leave the ship and it's gone; the next
time you board, two new hosts are drawn. If you can't meet either request, you
spend the trip awake on deck. A cabin gives nothing but the sleep: no healing,
no gifts.

Each ship draws its hosts from its own crowd: the Anne carries tourists and
well-off guests, the Aqua sailors and merchants, the Tidal families and
fishermen. Requests vary widely and don't need to be equally hard, for
example:

- a gentleman: "Beat me, and my cabin is yours";
- a chef: bring a particular berry for tomorrow's menu;
- a little girl: show her a Pokémon of a given colour, type or species;
- a sailor: help load cargo with a Pokémon that knows Strength, or light the
  hold with Flash;
- a seasick passenger: bring an Antidote or a Full Heal;
- a tourist from Alola: show a Pokémon from your own region;
- a lost-and-found lady: find the brooch she dropped somewhere on deck;
- a fisherman: show a Pokémon you caught with a rod;
- a scientist: show a fully evolved Pokémon;
- a rich kid: trade the cabin for a Nugget;
- an old sea dog: answer a sailor's quiz about the ports;
- an insomniac: play the Poké Flute;
- a gambler: win a coin flip;
- a stowaway kid: bring a snack and share the hiding spot;
- a breeder: show a Pokémon with high friendship;
- honeymooners: win a double battle against them.

Nothing outside the ship tells you who's aboard: you find the hosts by
walking the corridors.

**Passengers rotate.** The ships' trainers join the daily trainer rotation
like route trainers: each day different passengers are aboard and you can
battle them again, and other trainers who fit a ship may turn up among them.

**No farming in your sleep.** Staying aboard asleep, round and round, skips a
day every 4 legs. So a ship's own daily trainers and items (its
[daily world slots](daily-world-slots.md)) turn over only after you've been
ashore: while you stay aboard, the ship keeps the passengers and finds it had
when you boarded.

**Arriving.** When the ship reaches a port, an announcement names it and the
ship stays in port for a short while. Nothing asks you anything: to get off,
walk down the gangway yourself. A last call comes before the ship casts off;
if you're still aboard when it does, you're on the next trip, and so on round
the loop. Arrival is never limited by the table: once aboard, you can ride
the whole loop and get off anywhere on it, at any hour.

**In port and at sea.** Each leg is 6 in-game hours at sea (30 minutes of
play) followed by half an in-game hour in port (2.5 minutes), and a ship
you've just boarded stays the same half hour before it casts off.

### Ticket and service

One **S.S. Ticket** works on all three ships, and it's the only thing that
guards them: get it in any region and you can ride all three, whichever
region you started in. It's checked at every gangway and never used up. Its
sources stay as they are (the S.S. Aqua's maiden voyage, Bill in Kanto, and
the Hoenn start's gifts).

A ship joins the network when its story is done:

- **S.S. Aqua:** in service from the start for anyone holding the Ticket. A
  Johto start gets the Ticket on the maiden voyage, which stays as it is, so
  for them the Aqua joins once that voyage is over.
- **S.S. Anne:** as soon as the captain gives his reward. Until then she
  stays at Vermilion for the adventure, as today. When he rewards you, she
  casts off for One Island half an in-game hour later, whatever the time of
  day: walk down the gangway to stay in Vermilion, or stay aboard and sail.
  After that she follows the table. Trainers you skipped are still aboard.
- **S.S. Tidal:** from the start, for anyone holding the Ticket. It no longer
  waits for Hoenn's League.

Before a ship is in service, its slots in the table stay empty.

### Event islands

The ships' destination menus go, but the special trips to the event islands
stay where their own games had them. Lilycove's harbor sailor offers a
charter to Southern Island (with the Eon Ticket), Faraway Island (with the Old
Sea Map) and the Battle Frontier, each shown only when you qualify, as in
Emerald. Birth Island and Navel Rock stay on the Seagallop from Vermilion and
the Sevii harbors, as in FireRed and LeafGreen. The copies of these trips at
Olivine and Slateport go.

### Departure boards

Every port with an S.S. berth has a departure board: which ship is in port by
day and by night, the hours a single-ship port's berth is empty, and where
each ship sails next. That is the only timetable; the Pokégear and Town Map
don't show ships, and nothing ashore shows who's aboard.

## Boundaries

- **The other boats stay** as they are: the Seagallop between Vermilion and
  the Sevii Islands (with its Birth Island and Navel Rock tickets), the Route
  104 boat, and the Route 13 boat to Alola, which keeps its Strange Souvenir
  check. They are extras, not part of the network.
- **The ships' menus go.** The S.S. Aqua's Olivine, Vermilion and Slateport
  destination menu and the S.S. Tidal's Slateport, Lilycove and Battle
  Frontier menu are replaced by the loops, apart from Lilycove's event-island
  charters. The Battle Frontier isn't a stop on any loop.
- **No new ports.** Ships call at existing harbors and landings: Vermilion's,
  Olivine's, Slateport's and Lilycove's ports, the One Island and Seven Island
  Seagallop harbors, and Melemele's boat landing. Cinnabar gets no pier.
- **No new ship maps.** Each ship keeps its own rooms; only its gangway now
  leads to whichever port it's at. Its trainers rotate daily and its item
  spots stay ([world items](../specs/world-items.md), [daily world
  slots](daily-world-slots.md)). The Anne's Alolan flavour is lines, not new
  trainers.
- **Sleeping in a cabin is the only time skip**, and it only skips the rest of a
  leg. There is no waiting bench at the harbors.
- **No change to the S.S. Ticket's source**, the Anne's captain's adventure or
  the Aqua's maiden voyage, other than what happens after them.
- Only the Wayfarer build.

## Balance

- A leg is 6 in-game hours at sea and half an hour in port. Awake, that is
  about 30 minutes of play at sea; in a cabin, seconds. Either way the world
  moves on a quarter of a day per leg.

  | Trip | Legs | In-game time | Awake | In a cabin |
  | --- | --- | --- | --- | --- |
  | Vermilion → One Island | 1 | about 6.5 h | about 33 min | about 3 min |
  | Vermilion → Melemele | 2 | about 13 h | about 1 h 5 min | about 5 min |
  | Olivine → Melemele, changing at Vermilion | 3 | about 20 h | about 1 h 40 min | about 8 min |
  | A whole loop | 4 | about a day | about 2 h | about 10 min |

  The minutes in a cabin are the stops in port along the way. Waiting for a
  connection comes on top.
- The longest wait for a particular ship at a shared port is an hour of play;
  at One Island or Melemele it is 20 minutes. Five shared ports mean another
  ship is often in that gets you somewhere useful, so a wait is rarely forced.
- The challenge is planning, not reflexes: the table never changes, so
  missing a ship is a choice you can see coming.

## Presentation

- Gangway sailor, as you walk aboard: "Welcome aboard the S.S. Tidal, next
  stop Seven Island." Without a Ticket, the sailor stops you and asks for one.
- Empty berth: "The S.S. Anne's anchored offshore tonight. She's back at 4:00."
- Arrival: "Now arriving at Melemele Island." Last call: "All ashore that's
  going ashore! Next stop, Seven Island."
- The Anne's next-stop announcement for Melemele names Alola outright, the
  first time anyone does.

## References

- [In-game clock](in-game-clock.md)
- [S.S. Anne adventure](../specs/frlg-kanto-ss-anne-adventure.md), which
  promised this ferry change
- [HnS open-world traversal](../specs/hns-open-world-region-traversal.md), for
  the Aqua's maiden voyage
- [Wayfarer Hoenn entry](../specs/wayfarer-hoenn-entry.md), whose Aqua circuit
  and Slateport attendant this replaces
- [Sevii exploration](sevii-exploration-port.md), for the Seagallop
