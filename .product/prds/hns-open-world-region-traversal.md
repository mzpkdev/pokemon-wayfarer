# HNS open-world regional traversal

## Player outcome

After the opening releases the player with a starter, every town and city in the current unlocked region is reachable without completing the main story, earning badges, or using HMs. The player may take a long route, but determination is enough.

Kanto opens through the S.S. Aqua maiden voyage, Alola through the Mt. Moon keepsake quest, and Sinjoh through Meara's Ruins of Alph survey. Each quest awards its named travel item. Once a region is unlocked, travel to and between its settlements has no further story or HM requirement.

## Rules

- Walking, doors, and always-available local transport form each region's core settlement network.
- Main-story state, Gym progress, forced victories, and HMs cannot close the only route to a town or city in an unlocked region.
- Surf and Fly may make settlement travel faster. They are never required for basic settlement access.
- Cut, Rock Smash, Surf, bicycles, and local story events may gate shortcuts, hidden locations, and optional content.
- A story-gated shortcut must be self-contained. It cannot depend on a badge count or unrelated story flags from other cities.
- First-time travel to another region may require a self-contained local story and a key item. It cannot depend on Gym progress or a chain of unrelated campaign flags.
- After a region is unlocked, repeat travel is frictionless and always includes a return route.
- Opening a travel lane does not complete the story attached to it or award skipped rewards.

## Settlement coverage

| Region | Settlements in scope | Core access contract |
| --- | --- | --- |
| Johto | New Bark Town, Cherrygrove City, Violet City, Azalea Town, Goldenrod City, Ecruteak City, Olivine City, Cianwood City, Mahogany Town, and Blackthorn City | The existing land loop serves nine settlements once the listed road actors and turnbacks stop blocking it. Cianwood requires the ordinary ferry decision below. Safari Zone Gate, Lake of Rage, the League corridor, and Mt. Silver are not towns or cities in this pass. |
| Kanto | Pallet Town, Viridian City, Pewter City, Cerulean City, Vermilion City, Lavender Town, Celadon City, Saffron City, Fuchsia City, and Cinnabar Island | The S.S. Aqua releases the player in Vermilion. Route 6 and Saffron connect Celadon, Lavender, and Cerulean. The nonblocking Mt. Moon route continues to Pewter, Viridian, and Pallet. Celadon's loan-bicycle access to Cycling Road reaches Fuchsia. Cinnabar requires the ordinary transport decision below. |
| Alola | Melemele Isle, Akala Isle, Ula'ula Isle, and Poni Isle | The Route 13 trip releases the player at Melemele. The proposed four-stop Island Hopper service below must connect the four inhabited island hubs without Surf. Caves, forests, ocean routes, and encounter areas are optional destinations rather than settlements. |
| Sinjoh | New Sinjoh and Sinjoh Ruins | Meara releases the player at the Sinjoh branch entrance. The Snowswept Cavern route must provide a normal lane to both inhabited hubs; the exact obstacle check remains listed below. |

A healing point or Fly marker does not make a route or landmark a settlement. This roster, rather than every named map, is the acceptance boundary.

## Approved settlement changes

| Connection | Required behavior |
| --- | --- |
| New Bark Town to Route 29 | Release the west exit as soon as the player has a starter. Keep Elm's errand, the Mystery Egg sequence, the police scene, and Mom's interaction available. |
| Violet to southern Route 32 | Stop the coordinate triggers from turning the player back. The guard may retain conditional dialogue and the Miracle Seed reward. |
| Ilex Forest toward Route 34 | Remove the Cut tree from the route choke. Keep the Farfetch'd quest, HM reward, and other Cut interactions. |
| Route 36 junction | Move Sudowoodo into the nearby shrub-ring clearing. Keep the SquirtBottle encounter and retry behavior. Running away must not close the road. |
| Mahogany to Route 44 | Remove the east-road turnback. Keep the RageCandyBar merchant and Goldenrod Radio Tower story available. |
| New Sinjoh to Route 50 | Remove the state-4 turnback. Visiting the Kimono Hideout may still advance its story. |
| Kanto Mt. Moon | On first entry, advance only Mt. Moon's local arrival state and place Silver beside the through-lane. Speaking to him offers the existing battle; crossing the cave does not require accepting or winning it. |
| Celadon Cycling Road gate | Let the player enter without owning the Bicycle. The attendant loans a bicycle for the road segment, and the far gate removes the loaned riding state. The owned Bicycle item and Goldenrod Bike Shop remain unchanged. |

## Region unlock stories

### Kanto: S.S. Aqua maiden voyage

- Start: the Olivine port sailor offers a one-time maiden-voyage boarding as soon as the player can reach Olivine. Elm and the Johto League are not involved.
- Job: aboard the ship, the grandfather asks the player to find his granddaughter. She is in the captain's room. The blocking sailor is moved off the corridor; speaking to him may still start his battle, but winning it is not required to find the girl.
- Turn-in: escort the granddaughter back to her grandfather. He gives the player the S.S. Ticket as a permanent travel credential.
- First arrival: the ship arrives inside Vermilion port, marks Kanto and Vermilion visited, selects Kanto as an available map region, and leaves the Olivine return sailor active before releasing the player. It sets only the maiden-voyage reunion, Ticket, and minimum region-visited state.
- State isolation: the arrival does not return the Machine Part, upgrade the radio, wake Snorlax, repair the Magnet Train, mark another Kanto settlement visited, complete a Gym, or advance any Kanto campaign scene.
- Repeat travel: the S.S. Ticket alone opens direct Olivine and Vermilion sailings. Remove or ignore the old Machine Part check on both ferry desks. The missing-granddaughter story never repeats, and the Machine Part gates only the Magnet Train.

### Alola: Mt. Moon keepsake

- Start: speak to the lass in the Mt. Moon gift shop. Remove the Strange Souvenir from the shop inventory.
- Job: inspect the existing Moon Stone pickup outside Mt. Moon, then return to the lass. If the player collected it earlier, its existing item flag satisfies the step. The Moon Stone is not consumed.
- Turn-in: the lass gives the player the Strange Souvenir from her private collection.
- First arrival: show it to the Route 13 captain. He sails to the Melemele landing, marks Alola and Melemele visited, runs Samson Oak's one-time welcome, and leaves the Route 13 return boat active before releasing the player.
- State isolation: the welcome does not clear a Totem sign, resolve a noble or island encounter, mark Akala, Ula'ula, or Poni visited, or award any local encounter reward.
- Repeat travel: the captain offers Alola directly while the player owns the Strange Souvenir. The keepsake quest never repeats.

### Sinjoh: Ruins of Alph field survey

- Start: place Meara in the Ruins of Alph Lab from the player's first visit. Her Mt. Silver Pokémon Center placement is no longer used for the unlock.
- Job: Meara asks the player to inspect one new marked inscription beside the lab on the Ruins of Alph exterior map. Reading it sets one dedicated survey flag. No chamber puzzle, HM, badge, battle, or other story flag is required.
- Turn-in: return to Meara. She gives the player the Azure Flute and offers to leave immediately. Remove the duplicate Azure Flute pickup from Mt. Silver 2F.
- First arrival: accepting the trip marks Sinjoh visited, establishes only the baseline map state required for its actors and exits, and sends the player to the Sinjoh branch entrance without requiring Mt. Silver access. Meara accompanies the player and appears beside that arrival point.
- State isolation: arrival does not visit the Kimono Hideout, move the Route 50 story past its baseline, resolve Machamp or its boulder, open a plate or Regi room, or advance Steven, Arceus, or any noble encounter.
- Return: speaking to Meara at the Sinjoh arrival point always offers Return to Ruins of Alph, including before any Sinjoh story, after saving, and after a blackout. Returning places both player and Meara inside the Ruins of Alph Lab.
- Repeat travel: while the player is in Johto, Meara remains in the Ruins of Alph Lab and offers direct travel after the survey. On every trip she reappears at the Sinjoh arrival point as the return contact. The survey never repeats.

## Alola local transport

### Island Hopper

- Required player behavior: after Samson Oak's welcome, a sailor at the Melemele landing offers Melemele, Akala, Ula'ula, Poni, and Return to Route 13. The sailors at the other three island landings offer the same menu.
- Availability: the service checks only that Alola has been unlocked. It does not check Surf, badges, Totem signs, noble encounters, island visits, or wider story state.
- First visit: selecting a new island marks only that island visited and places the player beside its return sailor. It does not start or finish that island's encounter content.
- Persistence: all four island destinations and Route 13 remain available after saving, blacking out, or starting local content.
- Placement still to approve: choose the exact shoreline tile and sailor position on each of the four existing island maps. The landing must be visible from the island's settlement hub and cannot replace an encounter or one-way ledge.

## Inter-region shortcut story

### Magnet Train restoration

- Start: the Saffron station attendant explains that the train lacks power and directs the player to the Power Plant.
- Stage 1: speak to the Power Plant manager, recover the existing hidden Machine Part from Cerulean Gym, and return it to the manager. No Rocket, badge, or radio story flag is checked.
- Stage 2: speak to Copycat in Saffron, retrieve her lost doll from the Vermilion Fan Club, and return it. She gives the player the Pass.
- Unlock: both Goldenrod and Saffron stations require the returned Machine Part flag and the Pass. Once both are present, the train works in both directions without another check.
- Separation from the ferry: returning the Machine Part is part of restoring this shortcut. It is never required by the S.S. Aqua after the maiden voyage.

## Unresolved settlement blocks

| Region | Block | Access affected | Decision needed |
| --- | --- | --- | --- |
| Johto | Cherrygrove Silver battle | Cherrygrove onward route | Add a bypass that keeps the battle available. |
| Johto | Azalea Silver battle | Azalea onward route | Add a bypass that keeps the battle available. |
| Johto | Route 41 Surf | Cianwood City | Add an always-available normal connection. A public ferry is the leading option. |
| Johto | Ice Path Kimono Girl | Blackthorn City | Add a second lane or make her interaction optional in place. |
| Kanto | Route 20 and Route 21 Surf | Cinnabar Island | Choose one ordinary transport connection from Pallet or Fuchsia to Cinnabar, including its operator, boarding point, arrival point, and always-available return. Both Surf routes may remain optional. |
| Alola | Island Hopper placement | Akala, Ula'ula, and Poni | Approve one visible shoreline landing and sailor position on each island for the five-option service defined above. No additional island quest is needed or allowed for basic settlement access. |

These decisions are required before an implementation spec can claim full coverage. The Kanto land route is no longer an open question: it deliberately uses Saffron, optionalizes the Mt. Moon Silver battle, and treats Cycling Road as loan-bicycle public access. The Vermilion Snorlax and Underground Paths remain redundant shortcuts on that route.

## Checks before decision

| Region | Suspected block | Access affected | Check needed |
| --- | --- | --- | --- |
| Johto | Route 30 object | Violet approach | Confirm whether the object closes the only walking route. |
| Sinjoh | Snowswept Cavern rocks and boulder | New Sinjoh | Confirm in the emulator whether Rock Smash or Strength is mandatory. If blocked, choose a clear normal path that keeps Machamp optional. |
| Sinjoh | Snowswept Cavern approach | Sinjoh Ruins | Confirm the ruins do not inherit an HM requirement from the New Sinjoh approach. |

## Out of scope

- Mt. Silver, the League corridor, Victory Road, and other endgame locations are outside this pass.
- Cerulean Cave, plate-count rooms, Regi rooms, noble encounters, and event islands are optional content.
- The Vermilion Snorlax route, Route 19 coast, Underground Paths, and other redundant connections may remain gated shortcuts.
- This pass does not redesign those redundant shortcuts and does not approve their current broad campaign gates as a pattern for future work.

## Target acceptance after open decisions

- Resolve every row under Unresolved settlement blocks and Checks before decision, then record the chosen core lane in Settlement coverage before implementation begins.
- From a new save after the opening, visit the ten named Johto settlements without badges, HMs, story completion flags, or forced victories.
- Complete the S.S. Aqua maiden voyage, Mt. Moon keepsake, and Ruins of Alph survey exactly as written and receive their travel items without unrelated story flags.
- After the maiden voyage, return between Olivine and Vermilion with only the S.S. Ticket and without the Machine Part. Confirm the train remains locked until both the Machine Part is returned and the Pass is obtained.
- From Vermilion, follow the named Kanto land route to the other eight mainland settlements; use the selected Cinnabar service for the tenth. Do not battle Silver, wake Snorlax, own a Bicycle, or use an HM.
- Use the Island Hopper to visit all four named Alola island hubs and return to Route 13 without Surf or island-story progress.
- Visit New Sinjoh and Sinjoh Ruins without Mt. Silver permission or an HM and return through Meara.
- Return from every unlocked region without repeating its unlock story.
- Use each bypass first, then return and complete its preserved story normally.
- Confirm Surf and Fly improve travel without becoming prerequisites for settlement access.
- Confirm Cut and Rock Smash still expose shortcuts and optional content where intended.

## References

- [Story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
