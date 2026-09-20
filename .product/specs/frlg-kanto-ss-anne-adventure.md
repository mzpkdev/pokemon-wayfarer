# S.S. Anne adventure port

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)

Supporting requirements:

- [FRLG Kanto independent story beats](../prds/frlg-kanto-independent-story-beats.md): local adventures remain independently playable;
- [Wayfarer Hoenn entry and S.S. Aqua circuit](wayfarer-hoenn-entry.md): existing Vermilion services remain unchanged; and
- [HM field use](hm-field-use.md): the captain explains Wayfarer's badge-free Cut behavior.

Implemented: No

## Outcome

Wayfarer imports the FireRed and LeafGreen S.S. Anne as one self-contained
Vermilion adventure. The player boards with an S.S. Ticket, explores the ship,
battles its Trainers, collects its items, meets Blue, helps the captain, and
receives Cut.

The ship remains available after the captain's reward so optional rooms and
missed rewards cannot be lost. This persistence is a recovery guarantee, not a
repeatable activity. The Anne never becomes transport, a service hub, or a
source of renewable content.

## Port boundary

Import exactly these 25 interiors and the layouts, scripts, graphics, text, and
audio they require:

| Area | Maps |
| --- | --- |
| First floor | `SSAnne_1F_Corridor_Frlg`, `SSAnne_1F_Room1_Frlg` through `SSAnne_1F_Room7_Frlg` |
| Second floor | `SSAnne_2F_Corridor_Frlg`, `SSAnne_2F_Room1_Frlg` through `SSAnne_2F_Room6_Frlg` |
| Other guest areas | `SSAnne_3F_Corridor_Frlg`, `SSAnne_B1F_Corridor_Frlg`, `SSAnne_B1F_Room1_Frlg` through `SSAnne_B1F_Room5_Frlg` |
| Service and deck | `SSAnne_CaptainsOffice_Frlg`, `SSAnne_Deck_Frlg`, `SSAnne_Kitchen_Frlg` |

Preserve the source interior graph, ordinary NPCs, dialogue, doors, Trainer
battles, visible items, included hidden items, and room rewards unless this
specification explicitly adapts them. Saving in any imported interior and
reloading must leave a valid route back to Vermilion.

Do not import `SSAnne_Exterior_Frlg` or its Lava Cookie. Boarding transfers
directly between the existing Vermilion dock and the 1F corridor. No second
berth, visible Anne object, dock variant, new exterior, or dock `map.bin` change
is part of this port.

The map catalog must opt in only the listed maps and their required layouts.
Keep excluded source-group slots stable, and do not renumber existing HNS or
Emerald maps, layouts, map sections, Trainers, flags, or variables. FRLG and
standalone HNS builds retain their existing selections and behavior.

Select the imported event scripts explicitly for Wayfarer and verify symbol
closure for every referenced script, text, movement, sound, and object graphic.
Import the source S.S. Anne door metatile animation and its required tile and
palette assets for Wayfarer only. Give the selected interiors a named
`S.S. ANNE` map section classified as Kanto in every applicable region-map
mode; do not expose an unnamed or zero-sized entry or change existing concrete
map-section IDs in another build.

## Player flow

### Boarding and return

Extend the Wayfarer-only top-level Vermilion selector to these fixed results:

| Result | Choice |
| --- | --- |
| 0 | Sevii Islands |
| 1 | Other Destinations |
| 2 | Board S.S. Anne |
| 3 | Cancel |

Preserve the first two branches exactly. **Sevii Islands** remains available
before regular Aqua eligibility is tested. **Other Destinations** remains
behind `WayfarerCanUseRegularAqua()` and opens its existing six-entry menu:

| Result | Existing service |
| --- | --- |
| 0 | Slateport |
| 1 | Southern Island |
| 2 | Birth Island |
| 3 | Faraway Island |
| 4 | Battle Frontier |
| 5 | Exit |

**Board S.S. Anne** is evaluated at the top level, before the regular Aqua
gate, and requires only possession of `ITEM_SS_TICKET`. It must remain
selectable when the player is ineligible for regular Aqua travel, without
making **Other Destinations** available early. A missing Ticket uses the
established refusal and leaves the player at the dock. Cancel, B, and the
submenu's Exit change no story or travel state.

Selecting Anne fades directly to the 1F corridor at `(19, 2)`. Both corridor
exit warps return to the walkable Vermilion dock position `(8, 9)`. Leaving the
ship does not reopen the harbor menu or start a voyage. Boarding and leaving
must not change S.S. Aqua, Sevii, Hoenn-entry, respawn, or regional-travel
state. The Ticket is never consumed.

The ship does not register a heal or respawn location. Blackout follows the
existing last-valid-heal behavior, and the player can board again afterward
without reset or loss of unfinished content.

The implementation must extend the existing exact-menu audit rather than
weakening it. Standalone HNS keeps its current menu and eligibility behavior.

### One adventure, permanently recoverable

The captain's reward marks the adventure's principal objective complete, but
does not remove or depart the ship. The player may leave before completion and
return later, or revisit after completion to collect any still-unclaimed
one-time content. Save and reload must preserve each completed encounter and
reward independently.

Revisiting never resets the ship. In particular, it does not restore defeated
Trainers, collected items, Blue's scene, or the captain's reward. Completed
NPCs use stable post-event dialogue.

Adapt the captain, deck, and any other source dialogue that promises imminent
departure. Persistent-port variants may acknowledge that the adventure is
complete, but must not promise a voyage, schedule, or service that does not
exist.

The port must not add any of the following:

- destinations, voyages, schedules, fares, or arrival ports;
- healing, shopping, storage, trade, daycare, or other repeat services;
- Trainer rematches, repeat battles, daily events, renewable items, or
  recurring rewards;
- random or wild encounters introduced to create a repeatable activity;
- a second completion reward or a reason to farm the completed ship.

S.S. Aqua and Seagallop remain Wayfarer's travel ships. No future Anne travel
design is implied by this specification.

## Adventure content

### Trainers

Import all 16 ordinary FRLG ship Trainers and their authored parties and
rewards: Tyler, Ann, Arthur, Thomas, Dale, Brooks, Lamar, Dawn, Barny, Phillip,
Huey, Dylan, Leonard, Duncan, Edmond, and Trevor. Each has a distinct,
Wayfarer-owned defeated state. Losing leaves that Trainer available; winning
sets the state once and prevents another battle or reward.

Allocate appended Wayfarer Trainer constants and party data without reusing
raw FRLG IDs or changing standalone HNS tables. Register every imported party
with the existing Trainer-scaling classification. The generator must prove
that the runtime IDs and defeated-state slots are unique and do not overlap
HNS, Emerald, system, or partner ranges.

### Blue

The source approach triggers converge on one nonblocking Wayfarer visitor
introduction for every currently supported origin. It plays at most once,
describes Blue as a visiting Trainer, does not start a battle, and does not
claim a childhood rivalry, replace the player's own rival, set unrelated Kanto
story completion, or block the captain.

A future Kanto-origin implementation may replace this scene for that origin
under the broader story's forward-progression rules. That upgrade is outside
this port and must preserve captain access and the one-time visitor state.

### Items

Preserve the seven interior item balls and four interior hidden items as
one-time rewards:

- TM31, Stardust, X Attack, TM44, Ether, Super Potion, and Great Ball;
- the B1F Hyper Potion; and
- the Kitchen's Chesto, Pecha, and Cheri Berries.

Give each reward a distinct nonzero Wayfarer-owned saved flag. Do not allocate
a flag or substitute reward for the excluded exterior Lava Cookie. A failed
Bag-capacity check leaves the reward available.

### Captain and Cut

Helping the captain is a retry-safe, one-time transaction. If Cut is not owned,
check capacity, grant it, verify possession, and only then set the captain
completion flag. A capacity failure leaves the reward and completion available
for retry. If Cut is already owned, play the local story resolution and set
completion without granting a duplicate.

After completion, the captain gives stable post-adventure dialogue. Do not run
the source departure sequence, set a ship-departed scene, consume the Ticket,
or disable boarding. The reward dialogue must teach the badge-free Cut model
defined by the HM field-use specification.

## State and isolation

Allocate named Wayfarer states for the eleven included items, captain
completion, Blue scene, and 16 Trainer victories. Use currently free saved
capacity selected at implementation time and verify it against the then-current
main branch; numeric allocations from an unmerged branch are not authoritative.
No save-layout migration is expected.

Do not alias source constants whose zero or reused values are meaningful in
another build. Imported map scripts must use the new Wayfarer names under the
Wayfarer build guard. Existing concrete HNS and Emerald identifiers remain
stable.

Anne state is isolated from Bill's rescue, S.S. Aqua, Seagallop, Hoenn entry,
regional origins, badges, and unrelated Kanto adventures. Bill remains a normal
route to the Ticket, but a player who already owns one may board first and
rescue Bill later. Possessing the Ticket does not complete the captain's
objective. Bill must not award a duplicate Ticket when the player already owns
one, and his rescue remains playable.

## Acceptance

Implementation is complete only when automated checks and emulator coverage
demonstrate all of the following:

1. Exactly the 25 named interiors and their required layouts are selected in
   Wayfarer; the exterior and unrelated FRLG maps remain excluded. Event-script
   symbols, object graphics, the Anne door animation and assets, and the named
   Kanto map section have complete generated coverage. Existing HNS and Emerald
   generated IDs remain stable.
2. Every interior warp forms the intended closed ship graph, and every save
   location has a valid path to one of the two corridor exits. Boarding at
   `(19, 2)` and returning to dock `(8, 9)` are safe. Save/reload and blackout
   preserve consumed and unfinished content and permit re-entry.
3. The top-level selector keeps Sevii and Other Destinations at results 0 and
   1, adds Anne at result 2, and moves Cancel to result 3. The six-entry Aqua
   submenu retains its labels, indices, eligibility, and destinations. Anne
   accepts any S.S. Ticket independently of Aqua eligibility. Refusal,
   cancellation, boarding, and return change no other transport state.
4. All 16 Trainers have unique parties and persistent one-time defeat state.
   All eleven interior items have unique saved flags and remain claimable after
   capacity failure.
5. Blue's visitor scene plays once without a battle or blocking the captain.
   The captain handles normal award, full-pocket retry, prior ownership, save
   and reload, and repeat interaction without duplicate Cut. No dialogue
   promises that the Anne will depart or provide travel.
6. Leaving at multiple points and revisiting preserves unfinished content;
   revisiting after completion exposes only unclaimed one-time content and
   stable dialogue. Nothing resets or becomes renewable.
7. The ship never offers transport or a recurring service, and neither Anne
   entry nor completion alters Aqua, Seagallop, Hoenn-entry, Bill, badge,
   origin, or unrelated story state.
8. Standalone FRLG and HNS builds retain their original map selection, menus,
   scripts, counts, and state definitions. A coordinated Wayfarer release build
   passes the repository's ROM reserve requirement.
