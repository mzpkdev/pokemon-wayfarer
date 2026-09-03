# Wayfarer Hoenn integration

## Intent

Wayfarer adds Pokémon Emerald's Hoenn region to the HNS-based game. The same
player, party, Bag, Pokédex, storage, money, options, and play time continue
into Hoenn. Entering Hoenn does not start another save or replace the player
with Emerald's protagonist.

This milestone provides the first supported route into Hoenn. It does not yet
provide travel back to Johto or Kanto. The player first completes the existing
S.S. Aqua maiden voyage from Olivine to Vermilion, receives the S.S. Ticket,
and may then take the S.S. Aqua from Vermilion to Slateport.

Hoenn keeps the identity and authored content of Pokémon Emerald. Wayfarer uses
the HNS engine and progression rules around that content, including HNS wild
level scaling, field-move behavior, battle mechanics, and quality-of-life
features. The finished ROM must remain within the standard 32 MiB GBA ROM
limit.

## Design

### Shared player and save

- The HNS opening remains the start of the game.
- The player reaches Kanto through the existing S.S. Aqua maiden voyage.
- Completing the maiden voyage and receiving the S.S. Ticket unlocks the
  outbound Hoenn route at Vermilion Port.
- The outbound route lands inside Slateport Harbor.
- Hoenn uses the same player and global Pokémon and inventory systems while
  keeping its regional story state separate.
- This milestone has no Hoenn-to-Kanto or Hoenn-to-Johto route. The boarding
  interaction must state that return service is unavailable and require
  confirmation before departure.

The one-way boundary is deliberate implementation staging. It is not the final
regional-travel experience. A later PRD will define return shipping, the
coexistence of the S.S. Aqua and S.S. Tidal at Hoenn ports, and any additional
cross-region travel conveniences.

### S.S. Aqua access

The existing HNS S.S. Aqua story remains the authority for Kanto access and the
S.S. Ticket:

1. The player boards the maiden voyage at Olivine.
2. The missing-granddaughter sequence is completed aboard the ship.
3. The reunion grants the S.S. Ticket and its existing rewards.
4. The player disembarks at Vermilion and completes the maiden voyage.
5. The Vermilion attendant then offers Slateport as a Wayfarer-only
   destination alongside the existing HNS destinations.

The Slateport option requires both completed maiden-voyage state and possession
of the S.S. Ticket. It does not require a Kanto badge, the Machine Part, the
Magnet Train Pass, a League result, payment, or Hoenn progress. Declining the
one-way warning changes no state.

The Magnet Train remains a Johto and Kanto service. It never travels to Hoenn.

### First Hoenn arrival

The first successful Slateport trip initializes only Hoenn's local starting
state. It preserves the HNS player, party, Bag, Pokédex, storage, money,
options, play time, Trainer ID, clock, Johto progress, and Kanto progress.

Initialization establishes the pre-campaign Hoenn baseline, records Hoenn as
visited and current, and registers Slateport as a safe recovery destination.
It does not grant a Hoenn starter or advance Birch, rival, Gym, team,
legendary, harbor, ferry, or League state. The player arrives on walkable
ground in Slateport Harbor and can exit normally into Slateport City.

The operation runs once. Saving and reloading in Hoenn cannot repeat it.

### Hoenn campaign start

The player reaches Littleroot and Route 101 through Hoenn's open settlement
network. Birch's adapted rescue uses a Pokémon from the existing party instead
of forcing Emerald's starter-selection bag.

After the rescue, the player selects Treecko, Torchic, or Mudkip for Hoenn's
local rival branches. Birch offers the selected Pokémon as an optional gift,
which may be accepted immediately or left with him. The choice and gift state
remain separate from the HNS starter and Silver's party selection.

Canceling the local starter choice postpones only rival-dependent Hoenn story.
Open exploration remains available. Once the choice is committed, it cannot be
changed, but accepting the gift is not required to continue the campaign.

### Open exploration and authored content

Hoenn uses the implemented Emerald open-world traversal behavior. Its public
Route 104 ferry, road lanes, native Surf crossings, and early-arrival rules
remain available. Visiting a location does not complete its story, award its
reward, or defeat its opponent.

Wayfarer includes the Emerald maps, NPCs, shops, healing facilities, items,
ordinary Trainers, rivals, team encounters, Gym Leaders, Elite Four, Champion,
and main story required to complete Hoenn. Trainer parties retain their
Emerald-authored species, levels, moves, items, AI, and battle formats.

Ordinary Hoenn wild encounters retain Emerald's species, methods, weights, and
locations while using the HNS Trainer Rating level projection. Hoenn progress
does not add Trainer Rating inputs. Fixed, gift, legendary, hidden, and
scripted Pokémon retain their authored levels unless another approved feature
already governs them.

Sootopolis remains a late-game town. Its Dive entrance, weather crisis, Cave
of Origin, Gym, and related rewards retain their original Hoenn progression.
This milestone adds no special transport to Ever Grande or another inland
Hoenn destination.

### Regional progression

- Hoenn has eight independent badge states.
- Hoenn badge-count checks use only Hoenn badges.
- Hoenn badges do not alter HNS Trainer Rating.
- Hoenn Champion completion is independent from Johto and Kanto completion.
- Completing one League cannot start, finish, reset, or unlock another
  region's campaign.
- Hoenn Trainers, NPCs, items, gifts, and story rewards remain consumed through
  saving, reloading, and blacking out.

HNS field-move rules apply throughout Wayfarer. Wayfarer includes the HNS and
Hoenn native utility learnsets together. HM08 remains Whirlpool and HM09 is
Dive. Hoenn Dive spots require the Hoenn authorization granted by Steven's
Mossdeep event.

### Ticket and ship ownership

Wayfarer has one S.S. Ticket item. The HNS maiden-voyage reunion is its normal
source and the item is not consumed by travel.

The S.S. Aqua uses it for the Olivine, Vermilion, and outbound Slateport
service defined by this milestone. The S.S. Tidal remains a separate Emerald
ship with its original Hoenn Champion unlock and original destinations.
Possessing the ticket before the Hoenn League does not reveal or unlock the
S.S. Tidal.

If a Hoenn postgame event would award the S.S. Ticket after the player already
owns it, the event recognizes the existing item instead of attempting a
duplicate grant. It may announce that the S.S. Tidal is available, but it does
not turn that ship into a return route to Johto or Kanto.

### Map, Fly, healing, and blackout

The existing Wayfarer runtime selects Hoenn map art, names, destinations, and
Pokédex area data while the player is on a Hoenn map. This milestone does not
add manual region tabs or cross-region Fly.

The Vermilion-to-Slateport trip sets Slateport as the active safe recovery
location before returning control. Later healing in Hoenn updates the ordinary
active healing location. A blackout in Hoenn must recover at a valid Hoenn
location and must not move the player to Johto or Kanto.

This milestone does not preserve a separate healing history for every region.
That behavior belongs to the future return-travel PRD.

## Boundaries

- There is no supported route from Hoenn back to Johto or Kanto in this
  milestone.
- Slateport does not gain an S.S. Aqua attendant or departure menu.
- Cross-region Fly and selectable Town Map region tabs are not included.
- The S.S. Tidal keeps its original postgame role and destinations.
- This milestone does not add early Ever Grande transport.
- Wayfarer does not redesign or scale Emerald Trainer parties.
- Wayfarer does not add Hoenn-based Trainer Rating milestones.
- Battle Frontier, Contests, Secret Bases, Match Call, television events,
  multiplayer features, event islands, and other optional Emerald systems are
  preservation targets, not requirements for this milestone.
- This PRD does not define Sinnoh integration.
- Compatibility with prerelease saves is not required.

A public release that promises free travel among regions must not ship with
this one-way boundary. Return travel requires its own approved PRD and
implementation.

## Balance

Hoenn's ordinary and boss Trainers use their Emerald-authored levels. The
player may therefore meet battles much stronger or weaker than the current
party. Wayfarer does not correct that mismatch with Trainer scaling.

Wild levels continue to follow HNS Trainer Rating. Hoenn badges and story
milestones do not raise it. Rewards remain attached to their original Hoenn
interactions.

## Presentation

- Player-facing build and save identifiers use the name "Wayfarer".
- The Vermilion attendant identifies the destination as Slateport, Hoenn.
- Before departure, the attendant says that return service is unavailable.
- Canceling or declining leaves the player in Vermilion with no state change.
- Hoenn entry treats the existing player as a visiting Trainer and adds no
  replacement introduction.
- Existing Emerald music, maps, dialogue, and encounter identity remain intact
  unless an approved Wayfarer rule changes them.

## Interactions

The HNS open-world traversal specification continues to own the S.S. Aqua
maiden voyage, the Olivine and Vermilion service, and the Kanto settlement
network. This PRD extends only the completed Vermilion service with an outbound
Slateport destination.

The Emerald open-world traversal specification remains authoritative inside
Hoenn. The Hoenn content port owns the adapted Birch rescue and campaign
content. The Hoenn entry specification owns the outbound trip, first-arrival
boundary, and safe Slateport handoff.

Pokémon Centers, shops, the PC, party storage, the Bag, money, and player
identity remain shared. Hoenn events cannot read an HNS flag or variable merely
because the source games assigned both meanings to the same numeric value.

## Constraints

- The final ROM must be no larger than 32 MiB.
- Release builds must keep at least 512 KiB of unused ROM space during Hoenn
  development.
- Existing HNS map identifiers and persistent meanings cannot be renumbered.
- Map group and map numbers must remain representable by the existing warp
  format.
- Added persistent state and runtime buffers must fit the existing save,
  EWRAM, heap, and decompression limits.
- Every release build reports total ROM use and fails before exceeding its
  active ceiling.

## Playtesting

Automated acceptance is layered. Static and ROM tests own the gate matrix,
state isolation, one-time initialization, destination validity, ticket
behavior, and absence of a reverse route. One focused SkyEmu journey starts
from a completed maiden voyage in Vermilion, boards for Slateport, enters
Hoenn, saves, reloads, and exits the harbor into Slateport City.

Acceptance answers these questions:

- Is Slateport unavailable before the maiden voyage completes?
- Does the route require the S.S. Ticket without adding another story gate?
- Does declining the one-way warning leave all state unchanged?
- Does arrival preserve Johto and Kanto progress?
- Does first-arrival initialization run exactly once?
- Can the player save, reload, heal, and black out safely in Hoenn?
- Does Route 101 retain the adapted Birch rescue afterward?
- Does the S.S. Tidal remain locked until the Hoenn Champion result?
- Does the build remain within the active ROM limit?

## References

- [Wayfarer runtime foundation specification](../specs/wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port specification](../specs/wayfarer-hoenn-content-port.md)
- [Wayfarer Hoenn entry specification](../specs/wayfarer-hoenn-entry.md)
- [Emerald open-world regional traversal](../specs/emerald-open-world-region-traversal.md)
- [HNS open-world regional traversal](../specs/hns-open-world-region-traversal.md)
- [Trainer Rating wild encounter scaling](../specs/trainer-rating-wild-encounter-scaling.md)
- [Badge-free HM field use](../specs/hm-field-use.md)
- [Native HM utility learnsets](../specs/native-hm-learnsets.md)
