# Emerald open-world regional traversal

## Player outcome

After the opening releases the player with a starter, every Hoenn town and city is reachable without completing the main story, earning badges, or using HMs. The player may take a long route, but determination is enough.

Surf and Fly make travel faster. Cut, Rock Smash, Surf, bicycles, and local stories may still unlock shortcuts, hidden locations, and optional content.

## Rules

- Walking, doors, and always-available local transport form Hoenn's core settlement network.
- Main-story state, Gym progress, forced victories, and HMs cannot close the only route to a town or city.
- A story actor or battle may occupy one approach only when another visible lane remains open.
- A story-gated shortcut must be self-contained. It cannot depend on a badge count or unrelated story flags from other cities.
- A bypass does not complete its story or award skipped badges, HMs, items, or rewards.
- Maps and scripts must tolerate visits in an unexpected campaign order.

## Settlement coverage

The required roster is Littleroot Town, Oldale Town, Petalburg City, Rustboro City, Dewford Town, Slateport City, Mauville City, Verdanturf Town, Fallarbor Town, Lavaridge Town, Fortree City, Lilycove City, Mossdeep City, Sootopolis City, and Pacifidlog Town. Ever Grande is an endgame destination and is not part of this roster.

| Network segment | Settlements served | Core access contract |
| --- | --- | --- |
| Southwest and central land network | Littleroot, Oldale, Petalburg, Rustboro, Slateport, Mauville, Verdanturf, and Fallarbor | Existing roads, Petalburg Woods, the public ferry, Route 110, and the Route 111/113 loop remain passable under the approved changes below. Rusturf Tunnel remains an optional Rock Smash shortcut. |
| Route 104 public ferry | Dewford and Slateport, with a return to Route 104 | A new deckhand outside Briney's Cottage uses the existing Briney boat departure and offers Route 104, Dewford, and Slateport from the player's first visit. The same three-stop menu appears at all three landings. Briney, Peeko, the Letter, and the Devon Goods story never start, stop, or remove this service. No new route geometry or pier is required. |
| Lavaridge branch | Lavaridge | One ordinary lane through Route 112, the cable car, Mt. Chimney, or Jagged Pass must be selected below. |
| Eastern mainland | Fortree and Lilycove | One HM-free Route 118 crossing must connect Mauville's side to the eastern bank. Route 119 then reaches Fortree; a Kecleon bypass on Route 120 continues to Lilycove. |
| Eastern sea network | Mossdeep, Sootopolis, and Pacifidlog | One public transport network with visible boarding and return points must be selected below. Removing Lilycove's Wailmer keeps the Surf route open but does not satisfy this HM-free lane. |

## Approved changes

| Connection | Required behavior |
| --- | --- |
| Oldale to Route 102 | Leave the footprints man at his base position and disable the west-threshold turnback. Do not set adventure, Pokédex, or rival flags. |
| Petalburg to Route 104 | Leave one lane through the west crossing free of Norman and Wally's tutorial. Keep another approach available for the story. |
| Petalburg Woods northbound path | Leave one lane around the Devon employee and Aqua encounter. Keep another trigger lane for the story. |
| Route 104, Dewford, and Slateport ferry | Add the three-stop public ferry described above. Its deckhand and menu are independent of Briney, Peeko, Steven, the Letter, and the Devon Goods. |
| Route 110 Aqua wall | Initialize only the dedicated roadblock hide state. Do not complete the museum battles or Devon Goods delivery. |
| Route 110 rival | Leave one trigger lane for the battle and one lane for travel. The Itemfinder remains a battle reward. |
| Route 111 northbound road | Move one Rock Smash rock off the choke. Keep the other rock as an optional field-move interaction. |
| Route 119 bridge | Initialize only the dedicated Aqua roadblock hide state. Keep the Weather Institute occupation and Shelly battle available. |
| Route 119 rival | Leave one trigger lane for the battle and one lane for travel. Fly remains a battle reward. |
| Lilycove east ocean outlet | Do not install the blocking Wailmer metatiles. Keep the submarine, hideout, and Team Aqua story state unchanged. This preserves Surf as an optional direct route; it is not the ordinary HM-free sea connection. |

## Shortcut story

### Route 111 desert survey

- Start: speak to a new desert surveyor at the southern desert boundary on Route 111.
- Job: inspect three numbered weather stakes along the non-desert side of Route 111, then return to the surveyor. No battle, HM, badge, or earlier story flag is required.
- Reward: the surveyor gives the player the Go-Goggles.
- Result: the desert boundary accepts the Go-Goggles permanently. The desert remains an optional shortcut and exploration area rather than part of the core settlement route.
- Later story: the post-Lavaridge handoff checks whether the player already owns the Go-Goggles. If so, the giver acknowledges the surveyor's pair and gives no duplicate. The surrounding scene and all unrelated rewards still run normally.

## Unresolved settlement blocks

| Block | Access affected | Decision needed |
| --- | --- | --- |
| Lavaridge approach | Lavaridge | Choose one core lane. Either keep the Route 112 cable car operating before the team conflict, or leave a walkable descent through the Mt. Chimney staging and Jagged Pass. Specify which team actors move and where their scenes remain available. |
| Route 118 water choke | Fortree and Lilycove | Choose an HM-free crossing between Mauville's Route 118 bank and the eastern bank. Specify whether it is a bridge, a short ferry, or another visible transport; include both boarding points and unrestricted return behavior. |
| Route 120 Kecleon and Steven | Lilycove from the Fortree side | Choose the bypass lane or object placement that lets the player cross without the Devon Scope while keeping Steven's scene and the Kecleon encounter available. |
| Eastern sea service | Mossdeep and Pacifidlog | Choose the operator, boarding points, and menu for ordinary travel among Lilycove, Mossdeep, and Pacifidlog. Every stop must offer an immediate return, and no stop may check Surf, Dive, badges, or story state. |
| Sootopolis service | Sootopolis | Choose a believable surface arrival point and connect it to the eastern sea service. The player must enter and leave without Dive, the Mind Badge, the Space Center story, or the crisis story. The underwater entrance remains optional. |

These five route-design decisions are required before an implementation spec can claim complete Hoenn coverage. Each chosen service must be visible and understandable in the world; a hidden warp is not an ordinary travel solution.

## Checks before decision

| Suspected problem | Check needed |
| --- | --- |
| Early eastern-city arrival | Enter Fortree, Lilycove, Mossdeep, Sootopolis, and Pacifidlog before their campaign order. Record any arrival script that advances a team, Gym, legendary, weather, or rival state; convert it to baseline presentation only before approving the associated lane. |

## Out of scope

- Ever Grande's League area, Victory Road, and the Battle Frontier are endgame locations.
- New Mauville, hideouts, legendary rooms, Regi chambers, Safari expansion, and event islands are optional content.
- Rusturf Tunnel may retain Rock Smash because it is a shortcut between already reachable settlements.
- The Route 111 desert uses the survey story above and remains a gated shortcut.
- This pass does not redesign shortcut rewards or optional encounters unless they block the only settlement route.

## Target acceptance after open decisions

- Resolve every row under Unresolved settlement blocks, then record the chosen lane or service in Settlement coverage before implementation begins.
- From a new save after the opening, visit all fifteen named Hoenn settlements without badges, HMs, story completion flags, or forced victories.
- Use the public ferry from Route 104 to Dewford and Slateport and return from both destinations while the Peeko and Letter stories are untouched and while either story is active.
- Use each bypass first, then return and complete its preserved story normally.
- Test every changed connection from both directions and after saving on either side.
- Confirm Surf and Fly improve travel without becoming prerequisites for settlement access.
- Confirm Cut and Rock Smash still expose shortcuts and optional content where intended.
- Complete the Route 111 desert survey without a badge, HM, or unrelated story flag; confirm the Go-Goggles keep the boundary open and the later handoff does not duplicate them.

## References

- [Story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
