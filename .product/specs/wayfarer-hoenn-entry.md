# Wayfarer Hoenn entry

PRD: [Wayfarer Hoenn integration](../prds/wayfarer-hoenn-integration.md)
Implemented: Yes

## Scope

This specification defines the first supported journey into Hoenn in
Wayfarer. After the existing HNS S.S. Aqua maiden voyage is complete, the
player may take the S.S. Aqua from Vermilion Port to Slateport Harbor. The trip
uses the same player and save, initializes Hoenn once, and leaves the player at
a safe Hoenn destination.

This milestone is intentionally one-way. It implements only the
Vermilion-to-Slateport leg, so it provides no departure from Hoenn. The
scheduled-ferry PRD will define Hoenn port service, schedules, and the
return-aware recovery behavior needed for the remaining circuit legs.

The long-term S.S. Aqua circuit is fixed as Olivine to Vermilion to Slateport
to Lilycove to Olivine. Regional travel remains route-based rather than
unrestricted. Wayfarer has no selectable Town Map region tabs, and Fly cannot
cross the HNS and Hoenn boundary.

The runtime foundation owns the build, map catalog, persistent-state model,
active-region dispatch, and ROM budget. The Hoenn content port owns the adapted
Birch rescue, starter choice, maps, Trainers, encounters, Gyms, main campaign,
and League. The HNS traversal specification owns the maiden voyage and
standalone HNS ferry service. This specification supersedes its post-maiden
Vermilion-to-Olivine route only in Wayfarer.

## Behavior

### Availability

The Slateport destination exists only in the Wayfarer build and only at
`VermilionCity_PortInside_hns`.

The sailor offers Slateport when `VAR_SSAQUA_STATE` is at least 8, meaning the
maiden voyage has completed. Selecting Slateport proceeds only when the Bag
contains `ITEM_SS_TICKET`.

The route has no Kanto badge, Johto badge, Hoenn badge, Machine Part, Magnet
Train Pass, payment, League, or Hoenn-story requirement. Missing voyage state
keeps the existing pre-completion sailor behavior. Missing ticket uses the
existing no-credentials result and changes no state.

In the Wayfarer menu, Slateport replaces Olivine as the regular S.S. Aqua
destination and keeps that destination's menu index. Every other existing
special or optional Vermilion destination retains its index and behavior. The
standalone HNS menu, including its Olivine destination, remains unchanged.

After the maiden voyage, Wayfarer provides no S.S. Aqua route from Vermilion
back to Olivine. The Magnet Train remains the bidirectional Johto and Kanto
connection under its existing progression.

At `OlivineCity_PortInside_hns`, voyage state 8 and later retains the regular
Vermilion option at its existing menu index. That option continues to check
only for the S.S. Ticket. Every other Olivine destination retains its existing
index, gate, and behavior.

### Departure and destination

After the player selects Slateport, the script rechecks the completed-voyage
state and S.S. Ticket before committing the trip. It then uses the existing
Vermilion S.S. Aqua boarding and departure presentation.

The destination is `MAP_SLATEPORT_CITY_HARBOR`. The implementation must choose
a Wayfarer arrival coordinate that is walkable, lies outside every coordinate
event, and has an unobstructed path to an ordinary harbor exit. The exact
coordinate is recorded in the travel audit. The trip sets
`HEAL_LOCATION_SLATEPORT_CITY` before the destination receives control.

An invalid destination map, coordinate, or heal location fails static
validation. A runtime preflight failure leaves the player in Vermilion without
changing regional or Hoenn state.

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

### Deferred Hoenn departures

This specification adds no S.S. Aqua object, attendant, or departure script to
Slateport, Lilycove, or another Hoenn map. It does not implement the
Slateport-to-Lilycove or Lilycove-to-Olivine legs. It does not turn the S.S.
Tidal, Mr. Briney's boat, an event-island ferry, Fly, Teleport, blackout, or
another system into a route to Johto or Kanto.

The player remains in Hoenn after the trip. Saving, reloading, healing,
blacking out, entering the Hall of Fame, or using an ordinary Hoenn ferry must
not move the player back to HNS content. The approved Slateport-to-Lilycove and
Lilycove-to-Olivine legs require their scheduled-ferry PRD and specification
before implementation.

### S.S. Ticket and S.S. Tidal

The S.S. Ticket granted during the HNS maiden voyage is the shared ticket item
in Wayfarer. The S.S. Aqua checks it but does not consume it.

The S.S. Tidal remains a separate Hoenn ship. Its visibility, service, and
original destinations remain gated by Hoenn Champion state. Owning the S.S.
Ticket before completing Hoenn does not unlock it.

The retained Battle Frontier option in the Olivine and Vermilion HNS menus is
an existing special trip, not S.S. Tidal service. Its existing gate and
behavior remain unchanged by the Hoenn Champion requirement above.

If Hoenn's postgame ticket event runs while the player already owns the S.S.
Ticket, the event treats the item requirement as satisfied, records its
Hoenn-specific completion state, and does not attempt to add a duplicate key
item. The event may announce the S.S. Tidal service. It does not add an HNS
destination to that ship.

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

The outbound trip replaces the active heal destination with
`HEAL_LOCATION_SLATEPORT_CITY`. Subsequent Hoenn healing behaves normally. A
blackout after arrival resolves to the current valid Hoenn heal destination
and never to Olivine, Vermilion, or New Bark merely because those locations
were used earlier in the save.

Separate saved heal destinations for Johto, Kanto, and Hoenn are outside this
specification because this milestone has no route out of Hoenn. The
scheduled-ferry PRD must define safe healing and blackout behavior for the
completed circuit.

### Save and reload

Saving and reloading after arrival restores the exact Hoenn map, position,
active region, visited state, heal destination, initialization state, and all
Hoenn campaign state. Reloading cannot repeat initialization, grant an item,
reopen a consumed reward, expose the S.S. Tidal, or create a return route.

## Validation

Static, ROM, and focused runtime tests must verify:

1. Standalone HNS retains its original Vermilion menu and behavior.
2. Wayfarer exposes Slateport only when the maiden voyage is complete. A
   successful departure additionally requires the S.S. Ticket.
3. Wayfarer replaces the regular Olivine destination with Slateport at the same
   menu index, provides no Vermilion-to-Olivine S.S. Aqua path, and preserves
   every other existing Vermilion destination, including the existing HNS
   Battle Frontier special trip.
4. At voyage state 8 and later, Wayfarer's Olivine menu retains Vermilion at
   its existing index, gated only by the S.S. Ticket, and preserves every other
   Olivine destination.
5. Exit, Cancel, and a missing ticket change no persistent or travel state.
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
15. On a Hoenn map, the Town Map and Fly interface shows only Hoenn, exposes no
    region selector, and lists no HNS Fly destination. HNS fixtures before and
    after Kanto unlock retain their existing Town Map layouts and Fly behavior,
    expose no Hoenn map selector, and list no Hoenn Fly destination.
16. The Hoenn-port S.S. Tidal service stays unavailable until the Hoenn Champion
    result and keeps its original destinations afterward. This gate does not
    affect the existing HNS Battle Frontier special trip.
17. A Hoenn Champion fixture that already owns the HNS S.S. Ticket finishes the
    Hoenn postgame ticket event with one ticket, records the Hoenn receipt state,
    and unlocks the normal S.S. Tidal service and destinations.
18. The release ROM stays within the active Wayfarer size ceiling.

One focused SkyEmu journey may begin from a fixture with the maiden voyage
complete and the S.S. Ticket owned. It must select Slateport, complete the
normal ship departure, arrive in the harbor, exit to Slateport City, save,
reload, and retain the Hoenn location. The journey does not need to replay the
maiden voyage or complete the Hoenn campaign.

## Deferred follow-up

A future scheduled-ferry PRD owns:

- the Slateport-to-Lilycove S.S. Aqua leg;
- the Lilycove-to-Olivine S.S. Aqua leg;
- the timetable for the complete Olivine-to-Vermilion-to-Slateport-to-Lilycove-
  to-Olivine circuit;
- S.S. Aqua attendants and next-stop presentation at both Hoenn ports;
- S.S. Aqua and S.S. Tidal schedules and berth coexistence;
- the recovery state needed to heal and black out safely after the player can
  leave Hoenn.

Any special early transport to Ever Grande remains separately deferred.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
