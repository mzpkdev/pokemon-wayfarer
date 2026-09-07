# Johto and Hoenn starting regions

Status: Draft design for implementation. No runtime changes are included.
Specification: [Wayfarer regional start choice](../specs/wayfarer-regional-start-choice.md)

## Intent

Let the player choose where their Wayfarer journey begins. Johto starts with
the New Bark and Elm opening; Hoenn starts with the Littleroot and Birch
opening. These first two origins establish a home and first partner, then
lead into the same three-region adventure.

## Design

### Independently authored origins

An origin is an authored starting scenario, not a region or a fixed sequence
of house, Mom, rival, and professor scenes. The first release offers New Bark
and Littleroot using the region labels below. The foundation must also allow
later Pallet or custom origins, including multiple origins in one region.

A custom origin may begin on a ship, in the wilderness, or during an event,
with an existing party or no Pokémon until its own acquisition scene. It may
have no house, family, rival, professor, or conventional starter choice. Its
author controls its opening sequence, character relationships, rewards, and
transition into the wider world. These are examples of supported structure,
not additional origins included in this release.

Every origin must provide safe entry and recovery, preserve the shared player
and save, and define its interactions with regional stories and travel. It
must make required systems available through its own flow without borrowing
unplayed story flags from a stock opening. Oak's questionnaire, household
setup, and starter handoffs below are the presentation and content selected
for the first two origins, not requirements imposed on future custom flows.

### Professor introduction

Keep the existing Oak introduction, player appearance/name setup, and
challenge settings. Before the final send-off, Oak asks:

> Where will your journey begin?

The choices are `JOHTO` and `HOENN`, in that order. The prompt describes a
starting destination rather than claiming a birthplace, which also fits
Hoenn's moving-house opening. A confirmation names the destination town and
makes clear that the player can visit the other regions later. Choosing No
returns to the region list. The choice becomes permanent when the new game
begins; Continue never asks again.

Use the existing speech presentation and assets. There is one professor
introduction, followed by the selected region's opening. Name, appearance,
and challenge choices are not repeated by the regional scripts.

### Regional openings

| Choice | Opening | First partner under default settings | Home |
| --- | --- | --- | --- |
| Johto | New Bark bedroom, Mom, Elm's lab, then the existing Mr. Pokémon and Silver sequence | Chikorita, Cyndaquil, or Totodile | New Bark player house |
| Hoenn | Moving truck, Littleroot arrival and home/clock setup, local rival introduction, Birch's Route 101 rescue, then Birch's lab and the Route 103 rival sequence | Treecko, Torchic, or Mudkip | The player house selected by the existing Hoenn appearance/gender mapping |

For these two openings, the selected first partner is required to leave
the opening safely. The native Hoenn rescue uses the Pokémon chosen from
Birch's bag. Birch acknowledges that same Pokémon in the lab; he does not
give a second starter.

Keep each region's authored opening scenes and rewards, subject to the
existing Wayfarer traversal rules. Elm's errand and the Route 103 rival remain
available after obtaining a starter, without restoring removed roadblocks.
The choice does not impose a requirement to finish a regional story before
exploring or traveling.

Both starts use the same initial money and challenge settings and begin at
Trainer Rating 0, with no badges or League clears. Existing challenge options
may change starter species through their current rules; the chosen local
starter slot still determines the region's rival branch.

### Visiting the other starting region

A Johto starter visiting Hoenn keeps the existing visitor version of Birch's
rescue: use the current party, choose a local starter branch, and optionally
accept the selected Hoenn starter. Do not run the truck or household setup.

A Hoenn starter visiting Johto can meet Elm and begin the local errand and
Silver story with their existing party. Elm offers a Johto starter choice
for that story and an optional Pokémon gift. The player may postpone the
choice; only story scenes that need it wait. Once chosen, the choice stays
fixed even if the gift is declined. An unclaimed gift remains available later.
Visiting New Bark's house never resets Elm's quest or turns its resident into
the Hoenn player's mother.

Choosing a second regional starter does not replace the first partner or
change the other rival's team. The player's name, party, Bag, money, Pokédex
records, and completed quests survive every regional visit.

### Access to the wider world

Keep the directional S.S. Aqua circuit:

`Olivine → Vermilion → Slateport → Olivine`

Johto starters retain the existing maiden voyage and its ticket handoff.
Hoenn starters can obtain the S.S. Ticket free from the dedicated Slateport
Aqua attendant once they have received their first partner. Their first
departure goes to Olivine, and they use the regular circuit from then on.
They do not play or receive the rewards of Johto's maiden voyage in this scope.

Neither route requires a badge, League clear, fare, timetable, or completion
of the other region's opening. Reaching Slateport remains part of the Hoenn
journey through the existing roads and local ferry routes. S.S. Tidal keeps
its separate service and requirements.

### Home and shared equipment

The selected origin determines its home and family framing, where present.
Hoenn keeps its Mom and Norman relationship; Johto keeps its existing household.
Visitor dialogue avoids giving the player a second family. This is a
targeted correction to affected scenes, not a new biography or dialogue tree.

Johto's Mom retains her equipment handoff. Hoenn's Mom retains the running
shoes handoff; Birch supplies the shared Pokégear functions when acknowledging
the rescued starter in his lab. Native Pokédex receipt stays in each region's
opening. A later professor interaction acknowledges an existing Pokédex
without clearing records or downgrading unlocked modes.

Home is the initial recovery destination. After using a Pokémon Center or
traveling between regions, recovery follows the active local heal destination.
Travel and blackouts never change the player's chosen home.

## Boundaries

This feature exists only in the Wayfarer build. Standalone HNS, Emerald,
FireRed, and LeafGreen retain their current introductions and behavior.

This release adds no Kanto start, custom playable origin, third choice,
random start, origin change after starting, second player identity, new
starter roster, or new difficulty mode.
The feature does not alter Trainer Rating, regional badge accounting, League
eligibility or order, encounter scaling, mart stock, or the open-world field
move rules. It adds no cross-region Fly menu or new transport destination.

Native starts and visitor adaptations must both work at release. A region
selector that merely changes the initial warp does not satisfy this design.
Prerelease save migrations are not required.

Extensibility does not require an in-game origin editor, a scripting language,
or a generic questionnaire builder. Future authors can use the existing map,
script, and engine tooling to register and implement another origin.

## Playtesting

- Can a new player tell where each choice starts and that other regions remain
  available? Can they correct a mistaken choice before committing?
- Does each opening preserve its regional character and deliver exactly one
  first partner, with no empty-party battle or repeated identity setup?
- Can both starters reach all three regions with zero badges and zero League
  clears, using the existing traversal tools and routes?
- Can a player leave before finishing the local errand, return later, and
  continue it without replaying rewards or changing rival choices?
- Do both homes, early blackouts, Pokédex handoffs, and visits to the other
  region make sense for the selected origin?
- Can a Hoenn starter pursue the existing 24-badge journey and League circuit
  without any hidden dependency on New Bark's household or the maiden voyage?

## References

- [Custom origin paper exercise](../research/custom-origin-framework-exercise.md): three hypothetical flows and the remaining authoring-contract gaps.
- [Hoenn integration](wayfarer-hoenn-integration.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [HNS open-world traversal](hns-open-world-region-traversal.md)
- [Emerald open-world traversal](emerald-open-world-region-traversal.md)
