# Emerald Lati roaming

PRD: [Emerald Lati roaming](../prds/emerald-lati-roaming.md)
Implemented: No

## Scope

This specification defines Emerald's post-Hall-of-Fame Latias or Latios choice,
the Hoenn roaming territory, coexistence with HNS roamers, and the corresponding
Hoenn Southern Island encounter in Wayfarer.

The existing roaming engine remains authoritative for movement frequency, wild
encounter substitution, battle creation, fleeing, and persisted Pokémon data
except where this specification names regional ownership. The Hoenn content port
owns the surrounding Hall of Fame, player-house, S.S. Ticket, and ferry content.
Eon Ticket acquisition remains owned by its existing Wayfarer content.

## Behavior

### Build isolation

The union behavior in this specification applies when `IS_WAYFARER` is true.
Standalone builds retain their source behavior:

- standalone Emerald keeps one Hoenn location table, one saved roamer, and its
  existing `InitRoamer` meaning;
- standalone HNS keeps its Johto and Kanto tables and existing initializer
  meanings; and
- Wayfarer compiles the Johto, Kanto, and Hoenn location tables together.

Wayfarer must not select regional roaming behavior solely from the broad
`IS_HNS` build predicate or from the player's current map.

### Regional location tables

Wayfarer's roaming table IDs are stable and ordered as follows:

| ID | Territory | Map group | Authored route set |
| --- | --- | --- | --- |
| `0` | Johto | `MAP_GROUP(MAP_ROUTE29_HNS)` | Existing HNS Johto table |
| `1` | Kanto | `MAP_GROUP(MAP_ROUTE1_HNS)` | Existing HNS Kanto table |
| `2` | Hoenn | `MAP_GROUP(MAP_ROUTE110)` | Existing Emerald Hoenn table |

Preserve the existing numeric IDs for Johto and Kanto. Add a `mapGroup` field to
each location-table definition. Every operation that assigns or compares a
roamer's location uses the group belonging to that roamer's saved
`locationTableId`; no operation assumes map group zero.

The Hoenn table retains Emerald's complete Route 110 through Route 134 adjacency
data, including its existing empty entries and number of location sets. Attraction
and ordinary movement may change the map number only within that table and must
retain its Hoenn map group. A coincident map number in another group is never a
match and cannot attract or encounter the Hoenn roamer.

Reject an active roamer whose saved location-table ID is outside the compiled
table range before indexing table data. It must not read beyond the table or be
encountered on an arbitrary map.

### Saved roamer capacity and ownership

Wayfarer retains four saved `struct Roamer` records. The build must assert that
the configured capacity can simultaneously hold two Johto roamers, one Kanto
roamer, and one Hoenn roamer.

Regional initialization owns records by `locationTableId`, not by current slot
number or species. Reinitializing a territory may deactivate and replace active
records from that same table. It must not deactivate records belonging to any
other table. The first available inactive record may be used, so activation order
does not define permanent slot identities.

`TryAddRoamer` failure is observable to its caller through `VAR_RESULT`, where
`TRUE` means the selected roamer exists and `FALSE` means no record was added. A
script must not commit a choice or completion flag after a failed addition. Under
valid production state, the intended four-roamer set must always fit regardless
of activation order.

### Hoenn initialization

Expose an explicit script special for Hoenn, named `InitHoennRoamer` or an
equivalent region-specific name. Its input is `VAR_0x8004`:

| Input | Species | Level | Location table |
| --- | --- | --- | --- |
| `0` (`Red`) | Latias | 40 | Hoenn |
| `1` (`Blue`) | Latios | 40 | Hoenn |

Any other input fails without modifying saved roamers. For a valid input, the
special removes only an active Hoenn-table record, creates the selected Pokémon,
and writes `TRUE` or `FALSE` to `VAR_RESULT`. The selected color remains in
`VAR_0x8004`; callers must not treat the result value as the color. Creation
preserves the existing roamer rules for
personality, IVs, initial moves, HP, status, contest conditions, and shininess.

Initialization does not set a Pokédex seen flag. The television report alone is
not a Pokémon sighting.

Keep the existing HNS APIs region-specific in Wayfarer:

- `InitRoamer` continues to initialize level-40 Raikou and Entei in Johto; and
- `InitKantoRoamers` continues to initialize the selected level-60 Lati in Kanto.

Register the Hoenn special explicitly in the special table. Do not dispatch a
shared initializer according to the current map.

### Player-house television transaction

The post-Hall-of-Fame player-house scene retains its existing S.S. Ticket handoff,
television presentation, Mom movement, color prompt, and closing dialogue.

After the player selects a color, process the Lati state in this order:

1. Set a dedicated Hoenn `LATI_CHOICE_PENDING` state before presenting the
   prompt.
2. Copy the answer to `VAR_0x8004`.
3. Call the explicit Hoenn initializer, which writes success to `VAR_RESULT`
   without overwriting `VAR_0x8004`.
4. On success, copy `VAR_0x8004` into Hoenn's source-scoped
   `VAR_ROAMER_POKEMON`.
5. Set Hoenn's source-scoped roaming flag, clear `LATI_CHOICE_PENDING`, advance
   both player-house state variables, and complete the scene.

If initialization fails, do not store the answer, set the roaming flag, or
clear `LATI_CHOICE_PENDING`. Advance the two house variables past the automatic
cutscene and release the player so the map-frame script cannot loop. While the
pending state is set, Mom's normal interaction presents the color prompt again
without replaying Norman, the ticket handoff, or the television sequence. A
successful retry performs steps 2 through 5 above. The S.S. Ticket transaction
remains independently committed if it already succeeded.

The general player-house Lati news script contains an original redundant
`InitRoamer` call made before Mom records the color. In Wayfarer, remove or guard
that call so watching the broadcast cannot initialize any regional roamer. The
choice transaction above is the only player-house path that creates the Hoenn
roamer.

A committed Hoenn choice is one-time. Later Hall of Fame clears, ordinary TV
interactions, and player-house transitions do not call the initializer again.

### Roaming encounter lifecycle

The existing lifecycle applies independently to each active record:

- map transitions update and move all active roamers;
- an eligible normal land or water encounter may become the active roamer on the
  exact matching map group and map number under the existing probability;
- creating the battle uses the saved species, level, personality, IVs, HP,
  status, contest conditions, and shiny state;
- fleeing or another nonresolving outcome writes current HP and status back and
  relocates only the encountered record; and
- catching or defeating the Pokémon deactivates only the encountered record.

Roamers retain their authored fixed levels and do not pass through ordinary
Trainer Rating wild-level projection. The engine must continue to track the
encountered record index so two Lati of the same species cannot update or resolve
one another.

### Regional story state

Use the generated Hoenn source namespace for the Emerald choice, roaming flag,
Southern Island encounter flags, and S.S. Tidal presentation state. Do not alias
them to HNS's `VAR_ROAMER_POKEMON_HNS` or its Southern Island state.

The two regional Lati choices are independent. Any combination is valid:

| HNS choice | Hoenn choice | Required result |
| --- | --- | --- |
| Latias | Latias | Two independent Latias records in their own territories |
| Latias | Latios | One regional record of each species |
| Latios | Latias | One regional record of each species |
| Latios | Latios | Two independent Latios records in their own territories |

Defeat or capture state is also independent. Species identity must never be used
as a substitute for regional encounter ownership.

### Hoenn Southern Island access

In Wayfarer, ownership of `ITEM_EON_TICKET` is the shared item prerequisite for
each existing Southern Island ferry route. The Key Item is not consumed. The
HNS ports continue to select the HNS Southern Island. The Emerald S.S. Tidal
menu selects Hoenn's Southern Island only when the item is owned and Hoenn's
source-scoped roaming flag confirms that the television choice committed.

The Lilycove menu must not read HNS placeholder flag value zero for Emerald's
`FLAG_ENABLE_SHIP_SOUTHERN_ISLAND`, `FLAG_SHOWN_EON_TICKET`, or related
source-owned presentation state. Use an explicit Wayfarer-safe ticket predicate
and banked Hoenn flags where presentation state remains necessary.

This specification does not add or relocate an Eon Ticket giver. Any existing or
future valid Wayfarer grant can unlock both regional ferry destinations through
the same owned item, subject to each route's regional story prerequisite. The
default value of `VAR_ROAMER_POKEMON` never authorizes Hoenn island access.

### Hoenn Southern Island encounter

On entry, select the static encounter only from Hoenn's saved television choice:

| `VAR_ROAMER_POKEMON` | Object and encounter | Held item |
| --- | --- | --- |
| `0` (roaming Latias) | Level 50 Latios | Soul Dew |
| `1` (roaming Latios) | Level 50 Latias | Soul Dew |

Do not read HNS's Lati choice and do not inspect which roaming records are active,
caught, or defeated. Southern Island remains available whether the selected
Hoenn roamer is still roaming or has been resolved.

Running, teleporting, losing, or interrupting the battle before a resolving
outcome leaves the encounter available. Catching sets the Hoenn caught state;
defeating sets the Hoenn defeated state. Either resolving state hides the Hoenn
island encounter on later visits without changing the HNS island or any roaming
record. In Wayfarer, later Hoenn Hall of Fame clears must not clear the Hoenn
Southern Island defeated flag. Other event-legendary reset behavior remains
unchanged.

## Validation

Automated validation must cover:

- Wayfarer compiling exactly three regional location tables with stable Johto
  and Kanto IDs and the correct map group for every table;
- standalone Emerald and HNS retaining their prior table sets and initializer
  behavior;
- both television answers producing the specified species, level, and Hoenn
  table without setting its Pokédex seen flag;
- invalid input and exhausted capacity leaving every choice and completion state
  uncommitted, exiting the frame event without a loop, and remaining retryable
  through Mom's interaction;
- all four intended roamers coexisting in either regional activation order;
- movement, attraction, and encounter matching remaining inside the owning map
  group, including colliding map-number fixtures from another group;
- HP, status, personality, IV, shiny state, and encounter ownership persisting
  after a nonresolving battle and save/reload;
- catching or defeating one record deactivating only that record, including when
  Kanto and Hoenn contain the same species;
- repeated Hall of Fame and general-TV paths leaving a committed or resolved
  Hoenn roamer unchanged;
- an owned Eon Ticket exposing Hoenn Southern Island only after a committed
  Hoenn choice and without relying on a placeholder HNS flag; and
- both Hoenn choices producing the opposite level-50 Soul Dew encounter, with
  retry and one-time resolution behavior across later Hall of Fame clears.

At least one emulator journey must exercise each television choice. A combined
journey must activate the Johto, Kanto, and Hoenn roamers; cross a regional
boundary; verify a damaged or statused Hoenn roamer retains its battle state; and
resolve one same-species regional Lati without affecting the other. A Southern
Island journey must first verify that an early Eon Ticket does not expose the
destination, then commit the Hoenn choice, show the ticket at Lilycove, flee and
retry once, resolve the encounter, clear the Hoenn League again, and confirm the
encounter remains absent after save and reload. A capacity-failure fixture must
exercise the Mom retry interaction before a successful choice.

## References

- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [Emerald open-world region traversal](emerald-open-world-region-traversal.md)
- Emerald player-house event: `game/data/scripts/players_house.inc`
- Roaming engine: `game/src/roamer.c`
- Hoenn Southern Island: `game/data/maps/SouthernIsland_Interior/scripts.inc`
- S.S. Tidal menu: `game/src/script_menu.c`
