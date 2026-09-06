# Wayfarer Hoenn entry and S.S. Aqua circuit

PRD: [Wayfarer Hoenn integration](../prds/wayfarer-hoenn-integration.md)
Implemented: No

## Scope

This specification defines Wayfarer's permanent S.S. Aqua circuit. After the
existing HNS maiden voyage is complete, the S.S. Ticket permits one
directional next-stop journey at every circuit port:

Olivine to Vermilion to Slateport to Lilycove to Olivine.

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
| `SlateportCity_Harbor` S.S. Tidal hook | Lilycove Harbor |
| `LilycoveCity_Harbor` S.S. Tidal hook | Olivine |

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

At the Slateport and Lilycove harbor hooks, the former S.S. Tidal destination
menus become the Aqua's one-destination next-stop service. They no longer
require Hoenn Champion state or offer Tidal destinations. The circuit is
directional: no hook offers its previous stop or another circuit port.

### Departure and destination

After the player selects the next stop, the script rechecks the completed-
voyage state and S.S. Ticket before committing the trip. It uses the existing
boarding and departure presentation where available.

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
theft, Birch, rival, team, Gym, legendary, or League state. It enables the
repurposed Aqua hook without enabling any S.S. Tidal service.

The player is a visiting Trainer. No arrival script claims that the player
moved into the Littleroot house, repeats player creation, changes the clock, or
replaces the existing party and inventory.

The player may leave the harbor for Slateport City and use the implemented
Emerald open-world network to reach Littleroot and Route 101. The content
port's adapted Birch rescue and optional starter behavior remain unchanged.

### Hoenn departures and inactive S.S. Tidal

The existing S.S. Tidal departure hooks at Slateport and Lilycove become the
Aqua circuit's Slateport-to-Lilycove and Lilycove-to-Olivine legs. They must
not read Tidal Champion state, dispatch a Tidal trip, or expose a Tidal menu.
The visible S.S. Tidal service is inactive in Wayfarer.

Mr. Briney's boat, event-island ferries, Fly, Teleport, blackout, and other
systems do not become an interregional route. Saving, reloading, healing,
blacking out, and Hall of Fame processing preserve the player's current region
and current local heal location.

### S.S. Ticket and S.S. Tidal

The S.S. Ticket granted during the HNS maiden voyage is the shared ticket item
in Wayfarer. The S.S. Aqua checks it but does not consume it.

S.S. Tidal has no operating Wayfarer service. Its original destinations and
Champion-related service are deferred to a future additive feature. Owning the
S.S. Ticket or completing the Hoenn League does not enable it.

The retained Battle Frontier option in the Olivine and Vermilion HNS menus is
an existing special trip, not S.S. Tidal service. Its existing gate and
behavior remain unchanged by S.S. Tidal's inactivity.

If Hoenn's postgame ticket event runs while the player already owns the S.S.
Ticket, the event treats the item requirement as satisfied, records its
Hoenn-specific completion state, and does not attempt to add a duplicate key
item. It does not enable S.S. Tidal service.

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
item, reopen a consumed reward, or enable S.S. Tidal service.

## Validation

Static, ROM, and focused runtime tests must verify:

1. Standalone HNS retains its original Vermilion menu and behavior.
2. Wayfarer exposes Slateport only when the maiden voyage is complete. A
   successful departure additionally requires the S.S. Ticket.
3. At voyage state 8 and later, the four circuit hooks provide only Olivine to
   Vermilion, Vermilion to Slateport, Slateport to Lilycove, and Lilycove to
   Olivine. Every leg is gated only by the S.S. Ticket.
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
16. Slateport and Lilycove expose no S.S. Tidal service before or after the
    Hoenn Champion result. Their former hooks provide the Aqua circuit instead.
    This does not affect the existing HNS Battle Frontier special trip.
17. A Hoenn Champion fixture that already owns the HNS S.S. Ticket finishes the
    Hoenn postgame ticket event with one ticket, records the Hoenn receipt state,
    and leaves S.S. Tidal inactive.
18. The release ROM stays within the active Wayfarer size ceiling.

One focused SkyEmu journey may begin from a fixture with the maiden voyage
complete and the S.S. Ticket owned. It must travel Vermilion to Slateport,
Slateport to Lilycove, Lilycove to Olivine, and Olivine to Vermilion, saving
and reloading at a Hoenn harbor. The journey does not need to replay the maiden
voyage or complete the Hoenn campaign.

## Deferred follow-up

A future S.S. Tidal PRD may add a separate service, destination set, and
Champion-related behavior without changing the Aqua circuit. Any special early
transport to Ever Grande remains separately deferred.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
