# Wayfarer Hoenn integration

Implemented: Outdated

The League policy now requires TR-based levels with unchanged Tier 3 rosters.
The implementation still uses static League levels.

## Intent

Wayfarer adds Pokémon Emerald's Hoenn region to the HNS-based game. The same
player, party, Bag, Pokédex, storage, money, options, and play time continue
into Hoenn. Entering Hoenn does not start another save or replace the player
with Emerald's protagonist.

The first S.S. Aqua maiden voyage remains the authored trip from Olivine to
Vermilion. Its reunion grants the S.S. Ticket. Once that voyage is complete,
the ticket unlocks Wayfarer's permanent directional circuit:

Olivine to Vermilion to Slateport to Olivine.

Each port offers only its next stop. The circuit has no timetable, regional
lockout, payment, badge, League, or story gate beyond completing the maiden
voyage and holding the S.S. Ticket. It is the normal route between the three
regions, not a temporary Hoenn-entry exception.

At Slateport, a separate Wayfarer S.S. Aqua attendant in the existing harbor
provides the next-stop service to Olivine. This addition does not replace or
alter the existing S.S. Tidal attendant, ship, destinations, postgame gates,
or scripts. S.S. Tidal is deferred to a separate PRD.

Hoenn keeps the identity and authored content of Pokémon Emerald. Wayfarer uses
the HNS engine and progression rules around that content, including HNS wild
level scaling, field-move behavior, battle mechanics, and quality-of-life
features. The finished ROM must remain within the standard 32 MiB GBA ROM
limit.

## Design

### Shared player and save

- The HNS opening remains the start of this milestone.
- The player reaches Kanto through the existing S.S. Aqua maiden voyage.
- Completing the maiden voyage and receiving the S.S. Ticket unlocks every
  next-stop leg of the S.S. Aqua circuit.
- Olivine sends the player to Vermilion, Vermilion to Slateport, and Slateport
  to Olivine.
- Each trip lands inside its destination harbor.
- The Magnet Train remains the bidirectional Johto and Kanto connection.
- Hoenn uses the same player and global Pokémon and inventory systems while
  keeping its regional story state separate.
- Entering Hoenn never prevents the player from continuing around the circuit
  to Johto or Kanto.

### S.S. Aqua access

The existing HNS S.S. Aqua story remains the authority for Kanto access and the
S.S. Ticket:

1. The player boards the maiden voyage at Olivine.
2. The missing-granddaughter sequence is completed aboard the ship.
3. The reunion grants the S.S. Ticket and its existing rewards.
4. The player disembarks at Vermilion and completes the maiden voyage.
5. In Wayfarer, every circuit port offers its one next regular stop.

Every circuit option appears after the maiden voyage is complete. A departure
requires the S.S. Ticket; selecting a route without it uses the existing
no-credentials response and changes no state. The circuit does not require a
Kanto badge, the Machine Part, the Magnet Train Pass, a League result, payment,
or regional story progress.

The circuit is directional, not a destination picker: Vermilion does not offer
Olivine, Slateport does not offer Vermilion, and Olivine does not offer
Slateport. The Magnet Train remains an independent bidirectional Johto and
Kanto shortcut. Other special or optional Vermilion destinations, including
the HNS Battle Frontier option, keep their existing behavior and are not S.S.
Tidal service.

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
and main story required to complete Hoenn. Non-League Trainer parties other
than enrolled initial Gym Leader badge battles retain their Emerald-authored
source rosters, items, AI, and battle formats. Ordinary Trainers and Gym
members apply the separate [Trainer-party scaling design](trainer-party-scaling.md)
to those rosters; rivals and bosses retain authored battle parties. Initial
Gym Leader badge battles follow the separate [Gym Leader scaling design](gym-leader-scaling.md),
while leader rematches retain authored, static parties.
The Hoenn Elite Four and Champion use their existing Tier 3 rosters, with
levels governed by the [League scaling design](league-scaling.md).

Ordinary Hoenn wild encounters retain Emerald's species, methods, weights, and
locations while using the HNS Trainer Rating level projection. Hoenn badges
count toward Wayfarer's global badge total, and the Hoenn League contributes
its circuit milestone. Fixed, gift, legendary, hidden, and scripted Pokémon
retain their authored levels unless another approved feature already governs
them.

Sootopolis remains a late-game town. Its Dive entrance, weather crisis, Cave
of Origin, Gym, and related rewards retain their original Hoenn progression.
This milestone adds no special transport to Ever Grande or another inland
Hoenn destination.

### Regional progression

- Hoenn has eight independent badge states.
- Hoenn-local story checks use only Hoenn badges unless another approved
  feature changes that story.
- Hoenn badges count toward global League qualification and Trainer Rating.
- Hoenn Champion completion is independent from Johto and Kanto completion.
- Hoenn is the fixed Tier 3 League after the Kanto and Johto clears and all
  twenty-four badges. Completing it finishes the interregional circuit without
  finishing or resetting another region's local campaign.
- Hoenn Trainers, NPCs, items, gifts, and story rewards remain consumed through
  saving, reloading, and blacking out.

HNS field-move rules apply throughout Wayfarer. Wayfarer includes the HNS and
Hoenn native utility learnsets together. HM08 remains Whirlpool and HM09 is
Dive. Hoenn Dive spots require the Hoenn authorization granted by Steven's
Mossdeep event.

### Ticket and ship ownership

Wayfarer has one S.S. Ticket item. The HNS maiden-voyage reunion is its normal
source and the item is not consumed by travel.

Every circuit leg checks the ticket without consuming it. The Slateport Aqua
attendant and its route are separate from S.S. Tidal. This milestone makes no
change to S.S. Tidal's Champion gate, destinations, ticket event, ship object,
attendant, or scripts.

### Map, Fly, healing, and blackout

On a Hoenn map, the Town Map and Fly interface use only Hoenn map art, names,
visited state, and destinations. The player cannot select an HNS map or Fly
destination while in Hoenn. On an HNS map, the existing HNS Town Map and Fly
behavior remains unchanged. Wayfarer has no region tabs or other manual map
switcher, and Fly never crosses the HNS and Hoenn boundary.

Each circuit trip sets the destination's valid local recovery location before
returning control. Later healing updates the ordinary active healing location.
A blackout recovers at that location, even after circuit travel. Wayfarer does
not keep a separate healing history for every region.

## Boundaries

- The S.S. Aqua circuit has no timetable, bidirectional port menu, fare, or
  regional, badge, League, or story gate after the maiden voyage.
- Slateport's S.S. Aqua service is a separate Wayfarer interaction in the
  existing Harbor map. It is a fixed attendant placement, not a new map, port
  layout, collision change, map-warp event, or Tidal hook.
- Wayfarer has no selectable Town Map region tabs and Fly cannot cross the HNS
  and Hoenn boundary.
- S.S. Tidal remains entirely unchanged and is not part of the Aqua circuit.
- This milestone does not add early Ever Grande transport.
- Ordinary Trainer and Gym-member scaling is owned by the
  [Trainer-party scaling design](trainer-party-scaling.md). Enrolled initial
  Gym Leader badge battles are separately owned by the
  [Gym Leader scaling design](gym-leader-scaling.md); leader rematches retain
  static parties.
- The Hoenn League keeps its authored species, party sizes, moves, items,
  abilities, and AI; its levels scale using TR locked for the run under the
  [League scaling design](league-scaling.md).
- Battle Frontier, Contests, Secret Bases, Match Call, television events,
  multiplayer features, event islands, and other optional Emerald systems are
  preservation targets, not requirements for this milestone.
- This PRD does not define Sinnoh integration.
- Compatibility with prerelease saves is not required.

Wayfarer's regional transport is route-based. The Magnet Train links Johto and
Kanto in both directions, while the S.S. Aqua follows its permanent directional
circuit. The circuit does not add region tabs or cross-boundary Fly.

## Balance

Hoenn's ordinary and boss Trainers use their Emerald-authored levels. The
player may therefore meet battles much stronger or weaker than the current
party. Wayfarer does not correct that mismatch with Trainer scaling.

Wild levels continue to follow Trainer Rating. Hoenn badges raise the global
Wayfarer rating, and clearing the Hoenn League completes it at Rating 80.
Other Hoenn story milestones do not raise the rating. Rewards remain attached
to their original Hoenn interactions.

## Presentation

- Player-facing build and save identifiers use the name "Wayfarer".
- Each Aqua circuit attendant identifies the circuit's next destination.
- Canceling leaves the player at the current port with no state change.
- Hoenn entry treats the existing player as a visiting Trainer and adds no
  replacement introduction.
- Existing Emerald music, maps, dialogue, and encounter identity remain intact
  unless an approved Wayfarer rule changes them.

## Interactions

The HNS open-world traversal specification continues to own the S.S. Aqua
maiden voyage and standalone HNS ferry behavior. This PRD owns the permanent
post-maiden directional circuit, including the separate Slateport Harbor Aqua
interaction. The HNS specification continues to own the Kanto settlement
network.

The Emerald open-world traversal specification remains authoritative inside
Hoenn. The Hoenn content port owns the adapted Birch rescue and campaign
content. The Hoenn entry specification owns the circuit, first arrival, and
safe destination handoffs.

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
behavior, every directional circuit leg, the separate Slateport Aqua
interaction, and the unchanged Tidal wiring. One focused SkyEmu journey starts
from a completed maiden voyage in Vermilion, travels the complete circuit,
saves and reloads at Slateport Harbor, and returns to Olivine.

Acceptance answers these questions:

- Is Slateport unavailable before the maiden voyage completes?
- Does the route require the S.S. Ticket without adding another story gate?
- Does every port offer only its next circuit stop with the S.S. Ticket as the
  only post-maiden requirement?
- Do the three legs remain Olivine to Vermilion, Vermilion to Slateport, and
  Slateport to Olivine?
- Does arrival preserve Johto and Kanto progress?
- Does first-arrival initialization run exactly once?
- Can the player save, reload, heal, and black out safely in Hoenn?
- While the player is in Hoenn, does the Town Map show only Hoenn and does Fly
  exclude every HNS destination?
- On HNS maps before and after Kanto unlock, do the existing Town Map and Fly
  behavior remain unchanged and exclude every Hoenn destination?
- Does Route 101 retain the adapted Birch rescue afterward?
- Does Slateport's separate Aqua interaction provide Olivine without changing
  any S.S. Tidal route, gate, menu, ship object, attendant, or script, and does
  the existing HNS Battle Frontier option remain unchanged?
- Does the build remain within the active ROM limit?

## References

- [Wayfarer runtime foundation specification](../specs/wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port specification](../specs/wayfarer-hoenn-content-port.md)
- [Wayfarer Hoenn entry specification](../specs/wayfarer-hoenn-entry.md)
- [Emerald open-world regional traversal](../specs/emerald-open-world-region-traversal.md)
- [HNS open-world regional traversal](../specs/hns-open-world-region-traversal.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Trainer Rating wild encounter scaling](../specs/trainer-rating-wild-encounter-scaling.md)
- [Badge-free HM field use](../specs/hm-field-use.md)
- [Native HM utility learnsets](../specs/native-hm-learnsets.md)
