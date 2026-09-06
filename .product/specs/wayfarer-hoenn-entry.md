# Wayfarer Hoenn entry and S.S. Aqua circuit

PRD: [Wayfarer Hoenn integration](../prds/wayfarer-hoenn-integration.md)
Implemented: No

## Scope

This specification defines Wayfarer's permanent S.S. Aqua circuit. After the
existing HNS maiden voyage is complete, the S.S. Ticket permits one
directional next-stop journey at every circuit port:

Olivine to Vermilion to Slateport to Olivine.

The Vermilion-to-Slateport leg initializes Hoenn once. Every leg uses the same
player and save, leaves the player at a safe destination, and remains available
without a timetable, fare, regional lockout, badge, League, or story gate.
Regional travel is route-based rather than unrestricted. Wayfarer has no
selectable Town Map region tabs, and Fly cannot cross the HNS and Hoenn
boundary.

The interregional League circuit separately owns new-game start selection and
the availability of this circuit from those starts. It may define the necessary
opening entitlement without changing the circuit's direction or its port hooks.

The runtime foundation owns the build, map catalog, persistent-state model,
active-region dispatch, and ROM budget. The Hoenn content port owns the adapted
Birch rescue, starter choice, maps, Trainers, encounters, Gyms, main campaign,
and League. The HNS traversal specification owns the maiden voyage and
standalone HNS ferry service. This specification supersedes its post-maiden
Vermilion-to-Olivine route only in Wayfarer.

## Behavior

### Availability

The circuit exists only in the Wayfarer build. Its routes are:

| Departure hook | Destination |
| --- | --- |
| `OlivineCity_PortInside_hns` | Vermilion |
| `VermilionCity_PortInside_hns` | Slateport Harbor |
| Dedicated Wayfarer Aqua attendant in `SlateportCity_Harbor` | Olivine |

Each hook offers its destination when `VAR_SSAQUA_STATE` is at least 8, meaning
the maiden voyage has completed. A departure proceeds only when the Bag
contains `ITEM_SS_TICKET`.

The circuit has no Kanto badge, Johto badge, Hoenn badge, Machine Part, Magnet
Train Pass, payment, League, or regional-story requirement. Missing voyage
state keeps the existing pre-completion behavior. A missing ticket uses the
existing no-credentials result and changes no state.

In Wayfarer, Slateport replaces Olivine at the regular Vermilion Aqua menu
index. Olivine keeps Vermilion at its existing Aqua menu index. Every other
special or optional HNS destination retains its index and behavior. The
standalone HNS menus remain unchanged.

At Slateport, a dedicated Wayfarer Aqua attendant offers the Aqua's
one-destination next-stop service to Olivine. The attendant is a new object
event on the existing `MAP_SLATEPORT_CITY_HARBOR` map, uses its own script, and
is visible only in Wayfarer. Its fixed event record is:

| Field | Value |
| --- | --- |
| Local ID | `LOCALID_SLATEPORT_HARBOR_WAYFARER_AQUA_ATTENDANT` |
| Graphics and facing | `OBJ_EVENT_GFX_SAILOR`, facing left |
| Position | `(15,11)`, elevation `3` |
| Movement | stationary |
| Script | `WayfarerHoennEntry_EventScript_SlateportAquaAttendant` |
| Visibility | no visibility flag |

`(15,11)` is an existing walkable tile. It is outside every object, warp, and
coordinate event, has a path to the ordinary harbor exits, and is outside the
positions and movement lanes used by the Harbor Aqua-escape scene. The event
does not require a new map, port layout, collision edit, or static map-warp
event. It remains visually separate from the S.S. Tidal attendant and ship.

The circuit is directional: no Aqua hook offers its previous stop or another
circuit port. The dedicated attendant does not call, replace, guard, or modify
an S.S. Tidal script or state.

### Departure and destination

After the player selects the next stop, the script rechecks the completed-
voyage state and S.S. Ticket before committing the trip. The existing HNS
Olivine and Vermilion legs keep their current boarding and departure
presentation. Slateport uses the separate presentation below.

The Slateport Aqua attendant uses a dedicated departure presentation. After
the two checks and confirmation, it locks interaction, fades to black, commits
the Olivine local heal location, and performs the scripted Olivine warp. It
must not call `SlateportCity_Harbor_EventScript_BoardFerry` or
`Common_EventScript_FerryDepart`, hide or move the Tidal ship or attendant, or
reference `LOCALID_SLATEPORT_HARBOR_SS_TIDAL`. The existing Slateport Tidal
boarding presentation remains reachable only from its original Tidal scripts.

Every destination coordinate must be walkable, outside every coordinate event,
and have an unobstructed path to an ordinary harbor exit. The travel audit
records the coordinate and local heal location for each leg. A trip sets its
destination's local heal location before control returns.

An invalid destination map, coordinate, or heal location fails static
validation. A runtime preflight failure leaves the player at the departure
port without changing regional or Hoenn state.

### First-arrival initialization

The outbound trip checks `WayfarerHoennStateIsInitialized` before departure.
If Hoenn is uninitialized, one dedicated entry routine prepares the Hoenn
baseline before the warp:

1. Preserve the player, party, Bag, Pokédex, storage, money, options, play
   time, Trainer ID, clock, and all Johto and Kanto state.
2. Initialize only uncommitted Hoenn entry state, including
   `HOENN_STARTER_CHOICE_NONE` and an unreceived optional starter.
3. Apply the pre-campaign map and NPC visibility expected before Birch's Route
   101 rescue without advancing a Hoenn story variable.
4. Record Hoenn as visited and set the saved current region to Hoenn.
5. Register Slateport as the active safe recovery destination.
6. Set the Hoenn-initialized value last.

The routine returns success or failure. Failure leaves Hoenn uninitialized and
leaves the player in Vermilion. A later attempt retries the entire operation.
Once initialized, later calls are no-ops and cannot clear Hoenn progress.

The trip may encounter an already initialized Hoenn state only through tests,
debugging, or a future feature. In that case it skips initialization, updates
the current region and Slateport recovery destination, and completes the warp
without altering existing Hoenn progress.

### Slateport first-arrival state

First arrival does not advance `VAR_SLATEPORT_HARBOR_STATE`, the submarine
theft, Birch, rival, team, Gym, legendary, or League state. It makes the
separate Aqua attendant available without affecting S.S. Tidal service.

The player is a visiting Trainer. No arrival script claims that the player
moved into the Littleroot house, repeats player creation, changes the clock, or
replaces the existing party and inventory.

The player may leave the harbor for Slateport City and use the implemented
Emerald open-world network to reach Littleroot and Route 101. The content
port's adapted Birch rescue and optional starter behavior remain unchanged.

### Hoenn departures and S.S. Tidal separation

The dedicated Slateport Aqua attendant provides the circuit's
Slateport-to-Olivine leg. It rechecks the completed maiden-voyage state and
S.S. Ticket, then dispatches the Aqua trip. It does not call, replace, guard,
or write S.S. Tidal state.

S.S. Tidal remains unchanged at Slateport and Lilycove, including its ship
objects, attendants, scripts, Champion gate, destinations, and Battle Frontier
flow. It is not part of the Aqua circuit. A future S.S. Tidal PRD may change
that service after its complete design is decided.

Mr. Briney's boat, event-island ferries, Fly, Teleport, blackout, and other
systems do not become an interregional route. Saving, reloading, healing,
blacking out, and Hall of Fame processing preserve the player's current region
and current local heal location.

### S.S. Ticket and S.S. Tidal

The S.S. Ticket granted during the HNS maiden voyage is the shared ticket item
in Wayfarer. The S.S. Aqua checks it but does not consume it.

S.S. Tidal keeps its original Slateport and Lilycove behavior. Owning the S.S.
Ticket unlocks the Aqua circuit but does not change a Tidal gate, route, menu,
or event.

The retained Battle Frontier option in the Olivine and Vermilion HNS menus is
an existing special trip, not S.S. Tidal service. Its existing gate and
behavior remain unchanged by the Aqua circuit.

This specification makes no change to Hoenn's postgame ticket event.

### Town Map, Fly, healing, and blackout

On a Hoenn map, opening the Town Map or Fly interface renders only the Hoenn
map and uses only Hoenn location, visited, Fly, and Pokédex-area data. The
interface has no region tab or other control that can display an HNS map. Its
destination list contains only valid visited Hoenn Fly destinations, so Fly
cannot leave Hoenn.

On an HNS map, the existing HNS Town Map and Fly behavior remains unchanged.
No Fly destination or map control crosses the HNS and Hoenn boundary in either
direction. Regional transport across that boundary uses the physical S.S.
Aqua route.

Each circuit trip replaces the active heal destination with the valid local
destination location. Subsequent healing behaves normally. A blackout resolves
to the current valid local heal destination and never to an earlier port merely
because it was used before circuit travel.

### Save and reload

Saving and reloading after a circuit trip restores the exact map, position,
active region, visited state, heal destination, initialization state, and all
regional campaign state. Reloading cannot repeat initialization, grant an
item, or reopen a consumed reward. It also cannot change S.S. Tidal state.

## Validation

Static, ROM, and focused runtime tests must verify:

1. Standalone HNS retains its original Vermilion menu and behavior.
2. Wayfarer exposes Slateport only when the maiden voyage is complete. A
   successful departure additionally requires the S.S. Ticket.
3. At voyage state 8 and later, the three circuit hooks provide only Olivine to
   Vermilion, Vermilion to Slateport, and Slateport to Olivine. Every leg is
   gated only by the S.S. Ticket.
4. Wayfarer preserves every other existing HNS special destination, including
   the HNS Battle Frontier trip, and standalone HNS retains its original menus.
5. Exit, Cancel, and a missing ticket change no persistent or travel state.
6. Every destination coordinate is walkable, outside all coordinate events, and
   has a valid path through its ordinary harbor exit.
7. A successful trip leaves `VAR_SSAQUA_STATE` at 8, keeps the S.S. Ticket, and
   preserves a representative snapshot of Johto and Kanto progress. The active
   region and heal destination are the intentional changes.
8. First arrival initializes only Hoenn, commits its initialized value last,
   and runs exactly once.
9. First Slateport arrival advances no Hoenn campaign or S.S. Tidal state.
10. Every destination becomes the active region and safe local heal destination
    before control is returned.
11. Saving and reloading at each harbor preserves the exact state.
12. Blackout after every circuit leg recovers in the destination region.
13. Route 101 retains the content port's adapted Birch rescue and starter
    isolation.
14. The circuit is the only approved Hoenn-to-HNS route. Fly, other ferries,
    blackout, and Hall of Fame processing do not create another route.
15. On a Hoenn map, the Town Map and Fly interface shows only Hoenn, exposes no
    region selector, and lists no HNS Fly destination. HNS fixtures before and
    after Kanto unlock retain their existing Town Map layouts and Fly behavior,
    expose no Hoenn map selector, and list no Hoenn Fly destination.
16. The dedicated Slateport Aqua attendant uses an existing walkable tile that
    exactly matches its specified local ID, graphics, facing, position,
    elevation, movement, script, and visibility. Its tile is outside every
    existing object, coordinate event, and warp, has an unobstructed path to a
    harbor exit, and does not overlap an Aqua-escape-scene position or movement
    lane. It does not change a map layout or collision value.
17. Before and after the Aqua trip, S.S. Tidal retains its original Slateport
    and Lilycove attendants, ship objects, scripts, Champion gate,
    destinations, `FLAG_MET_SCOTT_ON_SS_TIDAL` progression, and Battle Frontier
    flow. This does not affect the existing HNS Battle Frontier special trip.
18. The dedicated Aqua script rechecks `VAR_SSAQUA_STATE` and
    `ITEM_SS_TICKET`, uses its independent fade-and-warp presentation, and
    does not call a Tidal script, `Common_EventScript_FerryDepart`, or reference
    `LOCALID_SLATEPORT_HARBOR_SS_TIDAL`.
19. The Hoenn-entry static audit allows and validates only this named Slateport
    Aqua event and its dedicated script. It continues to reject Aqua content in
    every other Emerald map and in the original Slateport Tidal script graph,
    and it verifies the original Tidal object, attendant, visibility gate, and
    reachable script graph independently.
20. The release ROM stays within the active Wayfarer size ceiling.

One focused SkyEmu journey may begin from a fixture with the maiden voyage
complete and the S.S. Ticket owned. It must travel Vermilion to Slateport,
Slateport to Olivine, and Olivine to Vermilion, saving and reloading at
Slateport Harbor. The journey does not need to replay the maiden voyage or
complete the Hoenn campaign.

## Deferred follow-up

A future S.S. Tidal PRD may revise Tidal service without changing the Aqua
circuit. Any special early transport to Ever Grande remains separately
deferred.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
