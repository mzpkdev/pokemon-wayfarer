# Emerald open-world regional traversal

## Player outcome

After the opening releases the player with a starter, every ordinary Hoenn town and city except Sootopolis is reachable without completing the main story, earning badges, or owning an HM. Native Surf users may provide the route across natural water. The player may take a long route, but determination is enough.

Sootopolis remains unlockable content. Its enclosed geography, Dive entrance, Gym, and crisis story do not belong to the opening settlement network.

## Rules

- Walking, doors, always-available local transport, and the approved native Surf crossings form Hoenn's core settlement network.
- Main-story state, Gym progress, and forced scripted or story victories cannot close the only route to a town or city in the opening network. That route cannot require an HM item. Only the named water crossings may require a Pokémon that already knows Surf.
- Ordinary sight-based trainers may challenge the player on a core route. They retain their existing positions, sight ranges, and authored parties and do not count as story-gated traversal. They do not scale with player progress.
- Native Cut, Flash, Strength, Rock Smash, Waterfall, and Dive users may open optional routes before the matching HM is found. Those routes do not count toward the core settlement network.
- A self-contained local encounter may occupy the only settlement lane only when it requires no badge, HM item, key item, or earlier story state; declining or losing leaves it available to retry; and winning is unnecessary because fleeing or another non-victory outcome completes it. Every required reward and state transition must be committed before the blocker is removed. If that commit fails, the encounter and reward remain available to retry. Any other story actor or scripted battle must leave another visible lane open.
- This exception applies only when this PRD explicitly names the encounter as the selected core lane. It does not replace an approved bypass or optional-scene requirement.
- A story-gated shortcut must be self-contained. It cannot depend on a badge count or unrelated story flags from other cities.
- A bypass or roadblock removal does not complete its attached story or award skipped rewards. Completing a permitted self-contained encounter may advance only that encounter's local state and grant its normal reward.
- Maps and scripts must tolerate visits in an unexpected campaign order.

## Settlement coverage

The required opening roster is Littleroot Town, Oldale Town, Petalburg City, Rustboro City, Dewford Town, Slateport City, Mauville City, Verdanturf Town, Fallarbor Town, Lavaridge Town, Fortree City, Lilycove City, Mossdeep City, and Pacifidlog Town. Sootopolis is unlockable content, while Ever Grande remains an endgame destination.

| Network segment | Settlements served | Core access contract |
| --- | --- | --- |
| Southwest and central land network | Littleroot, Oldale, Petalburg, Rustboro, Slateport, Mauville, Verdanturf, and Fallarbor | Existing roads, Petalburg Woods, the public ferry, Route 110, and the Route 111/113 loop remain passable under the approved changes below. Rusturf Tunnel remains an optional Rock Smash shortcut. |
| Route 104 public ferry | Dewford and Slateport, with a return to Route 104 | A new deckhand outside Briney's Cottage uses the existing Briney boat departure and offers Route 104, Dewford, and Slateport from the player's first visit. The same three-stop menu appears at all three landings. Briney, Peeko, the Letter, and the Devon Goods story never start, stop, or remove this service. No new route geometry or pier is required. |
| Lavaridge branch | Lavaridge | The Route 112 grunts leave one lane to an always-operating cable car. Mt. Chimney remains passable to Jagged Pass before, during, and after its team conflict. The conflict appears only after the Meteor Falls theft and retains its normal completion state. |
| Eastern mainland | Fortree and Lilycove | Native Surf crosses Route 118 without HM03 or a badge. Existing Lotad encounters serve the western land network and existing Wailmer fishing encounters serve the eastern land network. Route 119 then reaches Fortree. Steven and Kecleon retain their Route 120 bridge scene, which requires no earlier progress or victory and opens the bridge after the encounter resolves. |
| Eastern sea network | Mossdeep and Pacifidlog | Native Wailmer users connect Lilycove, Mossdeep, and Pacifidlog through the existing ocean routes and fishing encounters. Prepared travel is in scope; recovery after losing access to Surf is not. |

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
| Route 112 and Mt. Chimney | Move the two Route 112 grunts aside so one cable-car lane is always open. Hide the summit conflict before the Meteor Falls theft, reveal it when that scene completes, and move the four actors above Jagged Pass out of the descent lane. Keep Maxie's battle and its normal aftermath available only after Meteor Falls. |
| Route 119 bridge | Initialize only the dedicated Aqua roadblock hide state. Keep the Weather Institute occupation and Shelly battle available. |
| Route 119 rival | Leave one trigger lane for the battle and one lane for travel. Fly remains a battle reward. |
| Route 120 Steven and Kecleon | Keep the original bridge placement. The player may decline and retry, or accept and defeat, catch, flee from, or escape Kecleon through a battle move. Every non-loss resolution grants the Devon Scope exactly once and permanently opens the bridge. Losing leaves the scene available after recovery. Reward delivery must succeed before the completion state is committed. |
| Early eastern arrivals | Fortree, Lilycove, Mossdeep, and Pacifidlog may run baseline presentation and set their own visited state. Arrival does not advance a team, Gym, legendary, weather, rival, or other campaign scene. Route 118's Steven introduction may remain as baseline presentation. |
| Lilycove east ocean outlet | Do not install the blocking Wailmer metatiles. Keep the submarine, hideout, and Team Aqua story state unchanged. This is the native Surf entrance to the eastern sea network. |

## Shortcut story

### Route 111 desert survey

- Start: speak to a new desert surveyor at the southern desert boundary on Route 111.
- Job: inspect three numbered weather stakes along the non-desert side of Route 111, then return to the surveyor. No battle, HM item, field move, badge, or earlier story flag is required.
- Reward: the surveyor gives the player the Go-Goggles.
- Result: the desert boundary accepts the Go-Goggles permanently. The desert remains an optional shortcut and exploration area rather than part of the core settlement route.
- Later story: the post-Lavaridge handoff checks whether the player already owns the Go-Goggles. If so, the giver acknowledges the surveyor's pair and gives no duplicate. The surrounding scene and all unrelated rewards still run normally.

## Native Surf recovery boundary

The Standard Rod fishing PRD makes the existing Wailmer slots eligible with the Old Rod. A player who already has the rod and capture supplies can therefore obtain another native Surf user from the existing encounter tables. This PRD does not change encounters, terrain, rod distribution, fishing probabilities, or capture-supply availability.

Acceptance here covers a prepared player crossing each approved route in both directions. It does not guarantee recovery when the player lacks the Old Rod or Poké Balls, has a full party, or deposits, releases, or forgets Surf on the last user.

A separate traversal-recovery PRD owns those softlock-prevention and emergency-return requirements. This PRD may be accepted independently and must not be cited as proof that Hoenn's opening settlement network is softlock-safe.

## Out of scope

- Ever Grande's League area, Victory Road, and the Battle Frontier are endgame locations.
- Sootopolis is unlockable content. This pass does not bypass its Dive entrance, Gym state, Cave of Origin state, or crisis progression.
- New Mauville, hideouts, legendary rooms, Regi chambers, Safari expansion, and event islands are optional content.
- Rusturf Tunnel may retain Rock Smash because it is a shortcut between already reachable settlements.
- The Route 111 desert uses the survey story above and remains a gated shortcut.
- This pass does not redesign shortcut rewards or optional encounters unless they block the only settlement route.

## Acceptance

- From a new save after the opening, visit all fourteen named opening-network settlements without badges, HM items, earlier or unrelated story-completion flags, or a required scripted or story battle victory. Ordinary sight-based trainer battles are permitted. Native Surf is allowed only on the approved water crossings.
- Run every required core settlement route with no HM items in the Bag. No party Pokémon may know an HM move other than Surf on the approved crossings.
- Use the public ferry from Route 104 to Dewford and Slateport and return from both destinations while the Peeko and Letter stories are untouched and while either story is active.
- Reach Lavaridge before Meteor Falls, during the Mt. Chimney conflict, and after defeating Maxie. The road stays open in all three states, and the conflict cannot be completed before Meteor Falls.
- Approach Steven from both directions. Decline and retry, lose and retry after recovery, then defeat, catch, flee from, or escape Kecleon through a battle move. Each non-loss completion opens the bridge and grants the Devon Scope exactly once. A failed reward delivery leaves the scene retryable.
- Use each bypass or roadblock removal first, then return and complete its preserved story normally.
- Test every changed connection from both directions and after saving on either side.
- With a native Surf user prepared before each crossing, cross Route 118 in both directions, then travel among Lilycove, Mossdeep, and Pacifidlog in both directions without HM03 or a badge.
- Confirm no native HM other than Surf becomes a prerequisite for opening-network settlement access.
- Confirm Cut and Rock Smash still expose shortcuts and optional content where intended.
- Complete the Route 111 desert survey without a badge, HM item, field-move use, or unrelated story flag; confirm the Go-Goggles keep the boundary open and the later handoff does not duplicate them.
- Confirm this pass does not change the position, trainer type, or sight range of any ordinary sight-based trainer.

## References

- [Story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Badge-free HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](standard-rod-fishing.md)
