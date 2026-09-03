# Wayfarer regional travel and Hoenn entry

PRD: [Wayfarer Hoenn integration](../prds/wayfarer-hoenn-integration.md)
Implemented: No

## Scope

This specification defines early travel among Johto, Kanto, and Hoenn, the
adapted Hoenn campaign entry, and the region-aware Map, Fly, healing, blackout,
and return behavior needed to support one continuous Wayfarer save.

The runtime foundation owns the build and saved-state model. The Hoenn content
port owns maps, Trainers, encounters, NPCs, Gyms, the main campaign, and the
League. The existing HNS and Emerald traversal specifications continue to own
ordinary travel inside their respective opening settlement networks.

## Behavior

### Availability

Regional travel becomes available after the HNS opening gives the player a
starter and releases the player into the Johto opening network. It requires no
badge, HM, ticket, payment, story battle, League result, or campaign-completion
flag.

The service remains available for the rest of the save. No regional story may
remove its contacts, disable its menu, or block its return route.

The service has three principal stops:

| Region | Stop | Arrival contract |
| --- | --- | --- |
| Johto | Olivine regional terminal | Place the player on walkable ground beside an active return contact and preserve all Olivine and S.S. Aqua story state. |
| Kanto | Vermilion regional terminal | Mark only Kanto and the terminal destination visited, leave an active return contact, and preserve S.S. Aqua, Power Plant, Snorlax, Gym, and Magnet Train state. |
| Hoenn | Littleroot-area regional terminal | Initialize Hoenn on first arrival, mark only the arrival area visited, and place the player on walkable ground with an active return contact. |

Every contact presents the same destination list:

1. Johto
2. Kanto
3. Hoenn
4. Cancel

Choosing the current region reports that the player is already there and
returns to the menu. Cancel closes the menu without movement or state changes.
A successful trip uses a common state-free travel presentation, sets the active
region and minimum destination visit state, and places the player beside the
destination contact.

Travel must commit destination state before control is returned. If the trip
cannot resolve a valid destination map and coordinate, it leaves the player at
the origin and changes no regional state.

### Relationship to regional story transport

The regional service is separate from the S.S. Aqua, Magnet Train, Mr. Briney,
S.S. Tidal, Slateport and Lilycove ferries, and other story transport.

The player may enter Kanto through the regional service before the S.S. Aqua
maiden voyage. This early entry does not:

- complete or start the missing-granddaughter sequence;
- grant the S.S. Ticket or Metal Coat;
- change S.S. Aqua voyage state;
- return the Machine Part;
- grant the Magnet Train Pass; or
- advance a Kanto Gym or campaign event.

The maiden voyage remains available afterward. Its scenes and rewards occur
once, and first-arrival behavior becomes idempotent when Kanto has already been
visited. The regional service never requires the S.S. Ticket.

Entering Hoenn through the regional service does not change Mr. Briney, Devon
Goods, Letter, team, Gym, rival, legendary, or S.S. Tidal state. Hoenn's public
Route 104, Dewford, and Slateport ferry remains a separate local service.

### First Hoenn arrival

The first successful Hoenn trip checks a dedicated Hoenn-initialized state. If
Hoenn is not initialized, the trip performs one atomic initialization before
releasing the player:

1. Clear and initialize only the Hoenn persistent bank, explicitly setting
   `HOENN_STARTER_CHOICE` to `HOENN_STARTER_CHOICE_NONE` and
   `HOENN_STARTER_RECEIVED` to false.
2. Apply the baseline map and NPC visibility expected before the adapted Route
   101 rescue.
3. Select Brendan or May for the local rival role using the same player-gender
   relationship as Emerald, then preserve that choice.
4. Register the Littleroot-area arrival and safe recovery point.
5. Set Hoenn initialized.

The initializer does not clear or replace the player, party, Bag, Pokédex,
storage, money, options, play time, Trainer ID, HNS home, clock, Johto state, or
Kanto state. It does not grant a Hoenn starter, complete Birch's rescue, start a
rival battle, or advance another Hoenn scene.

If any required initialization step cannot commit, the trip returns to the
origin with Hoenn still uninitialized. Repeating the trip retries the complete
transaction. Once committed, later arrivals do not run the initializer again.

### Adapted Route 101 rescue

Birch's Route 101 rescue remains the entry to the Emerald main campaign, but it
uses the Wayfarer player and existing party.

The rescue follows this sequence:

1. Approaching its retained trigger starts Birch's normal request for help.
2. The battle uses the first non-Egg, non-fainted Pokémon in the player's party.
3. The game does not open Emerald's forced starter-selection bag before the
   battle.
4. A loss or interruption returns through normal recovery and leaves the
   rescue available to retry.
5. A successful rescue advances only the Hoenn opening state needed to visit
   Birch's lab and choose the local starter branch.
6. Birch asks the player to select one of the three Hoenn starters for Hoenn's
   local rival and campaign branches.
7. After the choice is saved, Birch offers the selected Pokémon as an optional
   gift.

Starter choice and starter delivery use separate Hoenn values:

- `HOENN_STARTER_CHOICE` starts at the distinct symbolic value
  `HOENN_STARTER_CHOICE_NONE`. A committed choice records Treecko as 0, Torchic
  as 1, or Mudkip as 2, preserving Emerald's branch mapping.
- `HOENN_STARTER_RECEIVED` records whether Birch successfully delivered the
  selected Pokémon.
- Neither value reads or writes the HNS `VAR_STARTER_MON` or changes Silver's
  party selection.

Every Wayfarer Hoenn consumer of Emerald's `VAR_STARTER_MON` is rewritten or
resolved through a Hoenn-source-scoped symbol that reads
`HOENN_STARTER_CHOICE`. A global alias is forbidden because HNS consumers must
continue to read `VAR_STARTER_MON`. This migration covers rival-party scripts
and non-rival dialogue, rewards, and species helpers. It includes the known
consumers in Route 103, Route 104, Route 110, Route 119, Rustboro City,
Lilycove City, Petalburg City's Pokémon Center, and Mauville City's Game
Corner, plus every other included Hoenn consumer found by the content audit.
Standalone Emerald continues to use its original starter variable and mapping.

The choice prompt may be canceled. Until a choice is committed, all three
choices remain available, the gift remains unclaimed, and rival-dependent
Hoenn story does not advance. Regional travel and open exploration remain
available.

Once a choice is committed, it cannot be changed. The player may accept the
selected Pokémon immediately or leave it with Birch and continue the Hoenn
campaign. No received state is set until that Pokémon is successfully placed in
the party or PC. Failed delivery leaves the same selected Pokémon available to
retry and does not reopen the other two choices.

Receiving or leaving the selected starter with Birch does not change the
player's existing starter, party ownership, Pokédex ownership, home region, or
Trainer identity. The Hoenn campaign cannot require the gifted starter to
remain in the party.

### Skipped Emerald introduction state

Wayfarer does not run Emerald's moving-truck arrival, bedroom setup, clock
setup, player creation, naming, initial money, initial Bag, initial party,
initial PC, or initial Pokédex sequence.

Hoenn scripts that would normally assume those scenes completed use an explicit
Wayfarer visitor baseline. That baseline may establish local NPC placement and
dialogue needed for the adapted rescue, but it cannot claim that the HNS player
moved into the Emerald protagonist's house or replace the player's existing
family and home state.

The local rival is Brendan or May. The rival's house, lab dialogue, Route 103
battle, and later rival encounters use the preserved local-rival choice. They
must not reinterpret the Wayfarer player's sprite or gender after the choice is
saved.

### Active region

Every included map resolves to exactly one primary region: Johto, Kanto, or
Hoenn. The active region is derived from the current map after a map load and
is saved for menus and recovery. A stale saved value cannot override the
current map's region.

Interior maps inherit the region of their owning settlement or entrance.
Transport interiors and other shared spaces declare an explicit origin and
destination context so Map, Fly, and blackout behavior cannot select a region
by accident.

Adding Hoenn must replace two-region boolean assumptions with region-aware
selection. A visited-Kanto flag alone cannot determine whether the current Map
or Fly screen is showing Hoenn.

### Town Map and location display

The Town Map opens on the current region and provides tabs for Johto, Kanto,
and Hoenn once each region has been visited. Before a region is visited, its tab
may remain hidden or disabled, but it cannot expose unvisited Fly destinations.

Each selected region uses its own:

- map artwork and tilemap;
- map-section identifiers and names;
- cursor coordinates;
- player marker or off-region indicator;
- location-entry table;
- visited destinations; and
- Pokédex area coordinates.

Changing tabs changes only the displayed region. It does not move the player,
mark a location visited, or alter campaign state. The next ordinary Town Map
opening returns to the player's current region rather than the last tab viewed
in another region.

### Fly

Fly uses the same regional tabs and location data as the Town Map. Only a
destination whose normal visit state is set may be selected.

The player may Fly to a visited destination in another region when normal HNS
Fly rules permit field use. A successful cross-region Fly updates the active
region before the destination map receives control. It does not initialize an
unvisited region, bypass first-entry initialization, or mark any intermediate
location visited.

The regional service remains available after Fly is obtained. A player without
a Fly user or visited Fly destination always has a physical return option.

### Healing and blackout

Wayfarer keeps one last-heal destination for each region. Healing at a Pokémon
Center or another normal healing point updates only the current region's slot.
Travel to another region does not overwrite the previous region's slot.

On blackout, the game resolves recovery in this order:

1. the current region's valid last-heal destination;
2. the current region's safe regional arrival point; and
3. New Bark Town if the current region or destination cannot be resolved.

The safe regional fallbacks are New Bark Town for Johto, the Vermilion regional
terminal for Kanto, and the Littleroot-area regional terminal for Hoenn.

Whiteout cleanup runs only the current region's applicable battle and story
cleanup. A loss in Johto cannot relocate Mr. Briney or change Hoenn Elite Four
state. A loss in Hoenn cannot reset a Johto or Kanto League sequence.

After recovery, the regional service or an ordinary path to it must remain
available. No blackout destination may strand the player behind a ticket,
badge, HM, field move, payment, or story check.

### Ever Grande and Sootopolis

Wayfarer provides an always-available public connection to Ever Grande's safe
exterior and healing area after regional travel opens. This connection may use
the regional service or an independent Hoenn public contact, but it requires no
badge, HM, field move, or story completion.

Early Ever Grande arrival marks only the exterior destination visited. Victory
Road, the Elite Four entrance, Champion result, and related rewards retain
their Hoenn requirements.

Sootopolis is not part of the early travel network. No regional-service menu,
public contact, Fly destination, or fallback opens it before the original
late-game Dive and weather-crisis progression. Once Sootopolis is reached
normally, its visited, Fly, healing, and blackout behavior works like another
Hoenn destination.

### Save and reload

Saving and reloading on either side of regional travel restores the exact map,
position, active region, visit state, local heal slot, Hoenn initialization,
rival selection, rescue state, starter-gift state, global Trainer Experience,
and its reduced-Experience tutorial state.

Reloading cannot replay first-arrival initialization, duplicate the optional
starter, reopen a consumed reward, remove a return contact, or convert an early
Kanto visit into S.S. Aqua completion.

### Validation

Static and automated checks must verify:

1. Every principal stop and Ever Grande public connection resolves to walkable
   ground beside an active return contact.
2. Regional travel is available after the HNS starter with no badge, HM,
   ticket, payment, battle victory, or campaign completion.
3. Cancel, current-region selection, invalid-destination handling, and every
   pair of successful destinations follow the required state rules.
4. Early Kanto entry leaves S.S. Aqua and all listed Kanto campaign state
   unchanged, and the maiden voyage remains completable once afterward.
5. First Hoenn arrival initializes only Hoenn and does so exactly once.
6. Birch's rescue uses an existing party Pokémon, remains retryable after a
   loss, and never opens forced starter selection before the battle.
7. `HOENN_STARTER_CHOICE_NONE` selects no Hoenn branch, and each committed
   choice selects the expected rival and non-rival branches without changing
   the HNS starter variable or a later Silver battle.
8. A static source audit rejects any included Wayfarer Hoenn script or
   Hoenn-only code path that still reads or writes raw `VAR_STARTER_MON`.
9. The selected starter can be left with Birch, retries after failed delivery,
   and can be received exactly once.
10. Every included map resolves the correct active region, including interiors
   and transport maps.
11. Town Map, Fly, player marker, cursor, destination entries, and Pokédex area
   data use the same selected region.
12. Cross-region Fly works only for visited regions and destinations and never
    bypasses first-entry initialization.
13. Each regional heal slot survives travel and save and reload independently.
14. Blackout returns to the current region and runs no other region's story
    cleanup.
15. Ever Grande is reachable early without opening Victory Road or the League.
16. Sootopolis remains locked until its original late-game progression.

The end-to-end journey starts from a fresh HNS save immediately after receiving
the starter. Without badges, HM items, tickets, payments, required story
battles, or campaign completion, the player must:

1. travel from Johto to Kanto and return;
2. travel to Hoenn, save and reload, and return;
3. re-enter Hoenn without rerunning initialization;
4. choose each Hoenn starter in separate runs, leave the selected gift with
   Birch, complete the Route 103 rival battle, leave Hoenn, return, and receive
   exactly the selected starter without changing a later Silver battle;
5. heal once in each region, travel away, return, and verify each regional heal
   slot;
6. black out once in each region and recover within that region;
7. Fly among visited destinations in all three regions;
8. visit Ever Grande without entering Victory Road or the League; and
9. confirm Sootopolis remains unavailable until its original unlock.

## References

- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Trainer Level progression](trainer-level-progression.md)
