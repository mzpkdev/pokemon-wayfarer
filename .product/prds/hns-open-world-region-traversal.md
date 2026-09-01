# HNS open-world regional traversal

## Player outcome

After the opening releases the player with a starter, the nine Johto settlements in this pass are reachable without completing the main story, earning badges, or owning an HM. Native Surf users may provide the route to Cianwood. Route 44, Ice Path, and Blackthorn City retain their existing late-game progression.

Kanto opens through the S.S. Aqua maiden voyage, which awards the S.S. Ticket. Once Kanto is unlocked, travel to and between its ten named settlements has no further story requirement. Only the named Cianwood and Cinnabar crossings may require native Surf. Early Alola and Sinjoh access are deferred in full.

## Rules

- Walking, doors, always-available local transport, and the approved native Surf crossings form the core settlement networks in this pass.
- Main-story state, Gym progress, and forced victories cannot close the only route to a settlement named under Settlement coverage. That route cannot require an HM item. Only the named Cianwood and Cinnabar crossings may require a Pokémon that already knows Surf.
- Fly may make settlement travel faster. Surf remains optional outside the approved Cianwood and Cinnabar routes.
- Cut, Rock Smash, Surf, bicycles, and local story events may gate shortcuts, hidden locations, and optional content.
- A Pokémon that already knows another HM move may open an optional route before the matching HM is found. That route does not count toward the core settlement network.
- A self-contained local encounter may occupy the only settlement lane only when it requires no badge, HM item, key item, or earlier story state; declining or losing leaves it available to retry; and winning is unnecessary because fleeing or another non-victory outcome completes it. Every required reward and state transition must be committed before the blocker is removed. If that commit fails, the encounter and reward remain available to retry. Any other story actor or battle must leave another visible lane open.
- This exception applies only when this PRD explicitly names the encounter as the selected core lane. It does not replace an approved bypass or optional-scene requirement.
- A story-gated shortcut must be self-contained. It cannot depend on a badge count or unrelated story flags from other cities.
- First-time travel to Kanto may require a self-contained local story and a key item. It cannot depend on Gym progress or a chain of unrelated campaign flags.
- Travel between Johto and Kanto is frictionless after Kanto is unlocked and always includes a return option.
- A bypass or roadblock removal does not complete its attached story or award skipped rewards. Completing a permitted self-contained encounter may advance only that encounter's local state and grant its normal reward.

## Settlement coverage

| Region | Settlements in scope | Core access contract |
| --- | --- | --- |
| Johto | New Bark Town, Cherrygrove City, Violet City, Azalea Town, Goldenrod City, Ecruteak City, Olivine City, Cianwood City, and Mahogany Town | The existing land network serves the eight settlements other than Cianwood once the listed road actors and turnbacks stop blocking it. Native Surf crosses Routes 40 and 41 to Cianwood. Existing mainland encounters provide Wooper and Chinchou; Cianwood fishing provides native Chinchou by day. Route 44, Ice Path, Blackthorn City, Safari Zone Gate, Lake of Rage, the League corridor, and Mt. Silver are outside this pass. |
| Kanto | Pallet Town, Viridian City, Pewter City, Cerulean City, Vermilion City, Lavender Town, Celadon City, Saffron City, Fuchsia City, and Cinnabar Island | The S.S. Aqua releases the player in Vermilion. Route 6 and Saffron connect Celadon, Lavender, and Cerulean. The nonblocking Mt. Moon route continues to Pewter, Viridian, and Pallet. Celadon's loan-bicycle access to Cycling Road reaches Fuchsia. Native Surf reaches Cinnabar through Route 21, with existing Chinchou fishing encounters around Vermilion and Cinnabar providing directional coverage. Route 20 remains optional. |

A healing point or Fly marker does not make a route or landmark a settlement. This roster, rather than every named map, is the acceptance boundary.

## Approved settlement changes

| Connection | Required behavior |
| --- | --- |
| New Bark Town to Route 29 | Release the west exit as soon as the player has a starter. Keep Elm's errand, the Mystery Egg sequence, the police scene, and Mom's interaction available. |
| Cherrygrove City | Remove only Silver's state-3 trigger at `(56,9)`. Keep the triggers at `(56,10)` and `(56,11)` so the original battle remains available. Taking the upper bypass does not advance Cherrygrove state or hide Silver. |
| Violet to southern Route 32 | Stop the coordinate triggers from turning the player back. The guard may retain conditional dialogue and the Miracle Seed reward. |
| Azalea Town | Remove only Silver's state-5 trigger at `(11,17)`. Keep the trigger at `(11,16)` so the original battle remains available. Taking the lower bypass does not advance Azalea state or hide Silver. |
| Ilex Forest toward Route 34 | Remove the Cut tree from the route choke. Keep the Farfetch'd quest, HM reward, and other Cut interactions. |
| Route 36 junction | Move Sudowoodo into the nearby shrub-ring clearing. Keep the SquirtBottle encounter and retry behavior. Running away must not close the road. |
| Kanto Mt. Moon | On first entry, advance only Mt. Moon's local arrival state and place Silver beside the through-lane. Speaking to him offers the existing battle; crossing the cave does not require accepting or winning it. |
| Celadon Cycling Road gate | Let the player enter without owning the Bicycle. The attendant loans a bicycle for the road segment, and the far gate removes the loaned riding state. The owned Bicycle item and Goldenrod Bike Shop remain unchanged. |

Route 30 requires no map or script change. Its existing east-side walking lane passes the staged Joey, Pidgey, and Rattata objects before Mom is visited.

## Region unlock stories

### Kanto: S.S. Aqua maiden voyage

- Start: the Olivine port sailor offers a one-time maiden-voyage boarding as soon as the player can reach Olivine. Elm and the Johto League are not involved.
- Job: aboard the ship, the grandfather asks the player to find his granddaughter. She is in the captain's room. The blocking sailor is moved off the corridor; speaking to him may still start his battle, but winning it is not required to find the girl.
- Turn-in: escort the granddaughter back to her grandfather. He gives the player the S.S. Ticket as a permanent travel credential and the existing Metal Coat reward. Each item is delivered exactly once, and the reunion remains retryable until both rewards are present.
- First arrival: the ship arrives inside Vermilion port, marks Kanto and Vermilion visited, selects Kanto as an available map region, and leaves the Olivine return sailor active before releasing the player. It sets only the maiden-voyage reunion, Ticket, and minimum region-visited state.
- State isolation: the arrival does not return the Machine Part, upgrade the radio, wake Snorlax, repair the Magnet Train, mark another Kanto settlement visited, complete a Gym, or advance any Kanto campaign scene.
- Repeat travel: the S.S. Ticket alone opens direct Olivine and Vermilion sailings. Remove or ignore the old Machine Part check on both ferry desks. The missing-granddaughter story never repeats, and the Machine Part gates only the Magnet Train.

## Inter-region shortcut story

### Magnet Train restoration

- Start: the Saffron station attendant explains that the train lacks power and directs the player to the Power Plant.
- Stage 1: speak to the Power Plant manager, recover the existing hidden Machine Part from Cerulean Gym, and return it to the manager. No Rocket, badge, or radio story flag is checked.
- Stage 2: speak to Copycat in Saffron, retrieve her lost doll from the Vermilion Fan Club, and return it. She gives the player the Pass.
- Unlock: both Goldenrod and Saffron stations require the returned Machine Part flag and the Pass. Once both are present, the train works in both directions without another check.
- Separation from the ferry: returning the Machine Part is part of restoring this shortcut. It is never required by the S.S. Aqua after the maiden voyage.

Route 44, Ice Path, and Blackthorn retain their existing progression and do not need a bypass in this pass. The Kanto land route deliberately uses Saffron, optionalizes the Mt. Moon Silver battle, and treats Cycling Road as loan-bicycle public access. The Vermilion Snorlax and Underground Paths remain redundant shortcuts on that route.

## Native Surf recovery boundary

The Standard Rod fishing PRD makes the required existing fishing slots eligible with the Old Rod. A player who already has the rod and capture supplies can therefore obtain another native Surf user from the existing encounter tables. This PRD does not change encounters, terrain, rod distribution, fishing probabilities, or capture-supply availability.

Acceptance here covers a prepared player crossing to Cianwood and Cinnabar in both directions. It does not guarantee recovery when the player lacks the Old Rod or Poké Balls, has a full party, or deposits, releases, or forgets Surf on the last user.

A separate traversal-recovery PRD owns those softlock-prevention and emergency-return requirements. This PRD may be accepted independently and must not be cited as proof that Johto's or Kanto's settlement network is softlock-safe.

## Out of scope

- Route 44, Ice Path, and Blackthorn City retain their existing story progression. This pass does not open, bypass, or rebalance them.
- Alola and Sinjoh retain their existing entry, travel, encounter, and reward progression. Early access to either region requires a later PRD.
- Mt. Silver, the League corridor, Victory Road, Cerulean Cave, and other endgame locations are outside this pass.
- The Vermilion Snorlax route, Route 19 coast, Underground Paths, and other redundant connections may remain gated shortcuts.
- This pass does not redesign those redundant shortcuts and does not approve their current broad campaign gates as a pattern for future work.

## Target acceptance

- From a new save after the opening, visit the nine named Johto settlements without badges, HM items, earlier or unrelated story-completion flags, or a required battle victory. Native Surf is allowed only for Cianwood.
- Before Mom is visited, walk Route 30 from Cherrygrove to Route 31 and back without changing its map, scripts, or staged objects.
- Confirm Route 44, Ice Path, and Blackthorn retain their original progression and are not opened by any change in this pass.
- Run every required settlement route with no HM items in the Bag. No party Pokémon may know an HM move other than Surf on the approved Cianwood and Cinnabar crossings.
- Complete the S.S. Aqua maiden voyage exactly as written and receive the S.S. Ticket without unrelated story flags.
- After the maiden voyage, return between Olivine and Vermilion with only the S.S. Ticket and without the Machine Part. Confirm the train remains locked until both the Machine Part is returned and the Pass is obtained.
- From Vermilion, follow the named Kanto land route to the other eight mainland settlements, then use native Surf to reach Cinnabar. Do not battle Silver, wake Snorlax, own a Bicycle, own HM03, or satisfy a Surf badge check.
- Confirm this pass does not change Alola or Sinjoh entry, transport, visit flags, encounters, or rewards.
- Return from Kanto without repeating the maiden-voyage story.
- Use each bypass first, then return and complete its preserved story normally.
- With a native Surf user prepared before each crossing, cross to Cianwood and Cinnabar in both directions using the existing encounter geometry and no HM03 or badge.
- Confirm no native HM other than Surf becomes a prerequisite for settlement access.
- Confirm Cut and Rock Smash still expose shortcuts and optional content where intended.

## References

- [Story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
- [Badge-free HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](standard-rod-fishing.md)
