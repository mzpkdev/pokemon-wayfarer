# Wayfarer Hoenn entry

PRD: [Wayfarer Hoenn integration](../prds/wayfarer-hoenn-integration.md)
Implemented: No

## Scope

This specification defines the first supported journey into Hoenn in
Wayfarer. After the existing HNS S.S. Aqua maiden voyage is complete, the
player may take the S.S. Aqua from Vermilion Port to Slateport Harbor. The trip
uses the same player and save, initializes Hoenn once, and leaves the player at
a safe Hoenn destination.

This milestone is intentionally one-way. It does not provide S.S. Aqua service
from Slateport, another physical return route, cross-region Fly, Town Map
region tabs, or separate healing histories. A future PRD must define those
features before Wayfarer promises free movement among all three regions.

The runtime foundation owns the build, map catalog, persistent-state model,
active-region dispatch, and ROM budget. The Hoenn content port owns the adapted
Birch rescue, starter choice, maps, Trainers, encounters, Gyms, main campaign,
and League. The HNS traversal specification owns the maiden voyage and
completed Olivine and Vermilion ferry service.

## Behavior

### Availability

The Slateport destination exists only in the Wayfarer build and only at
`VermilionCity_PortInside_hns`.

The sailor offers it when both conditions are true:

1. `VAR_SSAQUA_STATE` is at least 8, meaning the maiden voyage has completed.
2. The Bag contains `ITEM_SS_TICKET`.

The route has no Kanto badge, Johto badge, Hoenn badge, Machine Part, Magnet
Train Pass, payment, League, or Hoenn-story requirement. Missing voyage state
keeps the existing pre-completion sailor behavior. Missing ticket uses the
existing no-credentials result and changes no state.

The normal Vermilion menu retains Olivine and every existing HNS destination.
The Wayfarer menu retains the indices and behavior of those travel destinations
and adds Slateport before Exit. The standalone HNS menu remains unchanged.

### One-way confirmation

Selecting Slateport first names the destination and explains that no return
service from Hoenn is available in this milestone. The player must choose Yes
or No after that warning.

No closes the confirmation and returns to the destination menu. Cancel or Exit
releases the player in Vermilion. None of those paths changes the current
region, visited state, respawn location, S.S. Aqua state, ticket, or Hoenn
state.

The warning is required even if the save has visited a Hoenn map through test,
debug, or unsupported means. No hidden flag may suppress it.

### Departure and destination

After confirmation, the script rechecks the completed-voyage state and S.S.
Ticket before committing the trip. It then uses the existing Vermilion S.S.
Aqua boarding and departure presentation.

The destination is `MAP_SLATEPORT_CITY_HARBOR`. The implementation must choose
a Wayfarer arrival coordinate that is walkable, lies outside every coordinate
event, and has an unobstructed path to an ordinary harbor exit. The exact
coordinate is recorded in the travel audit. The trip sets
`HEAL_LOCATION_SLATEPORT_CITY` before the destination receives control.

An invalid destination map, coordinate, or heal location fails static
validation. A runtime preflight failure leaves the player in Vermilion without
changing regional or Hoenn state.

### First-arrival initialization

The confirmed trip checks `WayfarerHoennStateIsInitialized` before departure.
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

### Slateport arrival state

Arrival does not advance `VAR_SLATEPORT_HARBOR_STATE`, the submarine theft,
Birch, rival, team, Gym, legendary, S.S. Tidal, or League state. It does not
show the S.S. Tidal before its normal Hoenn Champion unlock.

The player is a visiting Trainer. No arrival script claims that the player
moved into the Littleroot house, repeats player creation, changes the clock, or
replaces the existing party and inventory.

The player may leave the harbor for Slateport City and use the implemented
Emerald open-world network to reach Littleroot and Route 101. The content
port's adapted Birch rescue and optional starter behavior remain unchanged.

### No return route

This specification adds no S.S. Aqua object, attendant, destination, or
departure script to Slateport or another Hoenn map. It adds no Hoenn destination
to Olivine. It does not turn the S.S. Tidal, Mr. Briney's boat, an event-island
ferry, Fly, Teleport, blackout, or another system into a route to Johto or
Kanto.

The player remains in Hoenn after the trip. Saving, reloading, healing,
blacking out, entering the Hall of Fame, or using an ordinary Hoenn ferry must
not move the player back to HNS content. Any future return path requires a
separate approved PRD and specification.

### S.S. Ticket and S.S. Tidal

The S.S. Ticket granted during the HNS maiden voyage is the shared ticket item
in Wayfarer. The S.S. Aqua checks it but does not consume it.

The S.S. Tidal remains a separate Hoenn ship. Its visibility, service, and
original destinations remain gated by Hoenn Champion state. Owning the S.S.
Ticket before completing Hoenn does not unlock it.

If Hoenn's postgame ticket event runs while the player already owns the S.S.
Ticket, the event treats the item requirement as satisfied, records its
Hoenn-specific completion state, and does not attempt to add a duplicate key
item. The event may announce the S.S. Tidal service. It does not add an HNS
destination to that ship.

### Map, Fly, healing, and blackout

On Hoenn maps, the existing Wayfarer runtime uses the Hoenn map, location,
visited, Fly, and Pokédex-area data selected by map provenance. This milestone
does not add manual region switching or cross-region Fly.

The outbound trip replaces the active heal destination with
`HEAL_LOCATION_SLATEPORT_CITY`. Subsequent Hoenn healing behaves normally. A
blackout after arrival resolves to the current valid Hoenn heal destination
and never to Olivine, Vermilion, or New Bark merely because those locations
were used earlier in the save.

Separate saved heal destinations for Johto, Kanto, and Hoenn are outside this
specification because this milestone has no return travel.

### Save and reload

Saving and reloading after arrival restores the exact Hoenn map, position,
active region, visited state, heal destination, initialization state, and all
Hoenn campaign state. Reloading cannot repeat initialization, grant an item,
reopen a consumed reward, expose the S.S. Tidal, or create a return route.

## Validation

Static, ROM, and focused runtime tests must verify:

1. Standalone HNS retains its original Vermilion menu and behavior.
2. Wayfarer exposes Slateport only when the maiden voyage is complete and the
   S.S. Ticket is present.
3. Olivine and all existing Vermilion destinations remain unchanged.
4. Exit, Cancel, a missing ticket, and No at the warning change no persistent
   or travel state.
5. The warning is shown before every supported outbound trip.
6. The destination coordinate is walkable, outside all coordinate events, and
   has a valid path through the harbor exit to Slateport City.
7. A successful trip leaves `VAR_SSAQUA_STATE` at 8, keeps the S.S. Ticket, and
   preserves a representative snapshot of Johto and Kanto progress. The active
   region and heal destination are the intentional changes.
8. First arrival initializes only Hoenn, commits its initialized value last,
   and runs exactly once.
9. Arrival advances no Hoenn campaign or S.S. Tidal state.
10. Slateport is the active region and safe heal destination before control is
   returned.
11. Saving and reloading in the harbor and in Slateport City preserves the
    exact state.
12. Blackout immediately after arrival recovers in Hoenn.
13. Route 101 retains the content port's adapted Birch rescue and starter
    isolation.
14. No S.S. Aqua, Fly, ferry, blackout, or Hall of Fame path returns the player
    to Johto or Kanto.
15. The S.S. Tidal stays unavailable until the Hoenn Champion result and keeps
    its original destinations afterward.
16. A Hoenn Champion fixture that already owns the HNS S.S. Ticket finishes the
    Hoenn postgame ticket event with one ticket, records the Hoenn receipt state,
    and unlocks the normal S.S. Tidal service and destinations.
17. The release ROM stays within the active Wayfarer size ceiling.

One focused SkyEmu journey may begin from a fixture with the maiden voyage
complete and the S.S. Ticket owned. It must select Slateport, observe and accept
the warning, complete the normal ship departure, arrive in the harbor, exit to
Slateport City, save, reload, and retain the Hoenn location. The journey does
not need to replay the maiden voyage or complete the Hoenn campaign.

## Deferred follow-up

A future interregional-return PRD owns:

- an S.S. Aqua presence and attendant in Slateport;
- Hoenn-to-Olivine and Hoenn-to-Vermilion routes;
- the long-term S.S. Aqua itinerary;
- S.S. Aqua and S.S. Tidal coexistence at Hoenn ports;
- direct destination selection and improved travel presentation;
- cross-region Fly and Town Map region tabs;
- separate regional healing histories and return-aware blackout behavior; and
- any special early transport to Ever Grande.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
