# Open Johto city traversal

Status: Draft

## Product goal

After receiving a starter, the player can enter and leave every Johto city with
zero badges and without completing a story quest, obtaining an HM, carrying a
key item, or winning a forced battle.

This requirement covers New Bark Town, Cherrygrove City, Violet City, Azalea
Town, Goldenrod City, Ecruteak City, Olivine City, Cianwood City, Mahogany Town,
and Blackthorn City.

## Player promise

Johto is available for exploration from the start of the adventure. Players can
choose where to travel, which local stories to pursue, and when to challenge
Gyms. Reaching a new city does not imply that its Gym, story event, or optional
area is ready to complete.

Obstacles may still mark side paths, conceal items, guard optional dungeons, or
create shortcuts that open later. They must not occupy the only route into a
city.

## Current problem

Johto's roads use badges, story flags, objects, field moves, and forced
encounters to enforce the original campaign order. Several of these gates sit
on the only practical route between cities:

- New Bark Town closes its west exit during two parts of the opening story.
- Route 30 may be physically obstructed until the player finishes the opening
  return to New Bark Town.
- Route 32 blocks travel from Violet City to Azalea Town behind Sprout Tower,
  the Violet Gym, and the Togepi Egg handoff.
- Ilex Forest requires the Azalea rival encounter, the Farfetch'd quest, and Cut
  before the player can continue to Route 34.
- Sudowoodo closes the Route 36 junction until the player earns access to the
  SquirtBottle.
- Cianwood City has no non-Surf connection to the rest of Johto.
- The RageCandyBar merchant closes Mahogany Town's east road until the Goldenrod
  Radio Tower story is complete.
- A Kimono Girl occupies the Ice Path corridor leading to Blackthorn City.

Removing badge checks from field moves does not solve the whole problem. It
still leaves city access dependent on HM acquisition, party compatibility, key
items, story state, and physical roadblocks.

## Design rules

### City routes are always open

After onboarding, at least one route into and out of each Johto city must be
available in every story state. The route must work in both directions and must
not depend on a badge, quest, field move, key item, payment, or forced victory.

### Story state does not control collision

Story flags may change characters, dialogue, encounters, rewards, and scenery.
They must not add collision to the only city route or turn the player back at a
city connection.

Opening a road must not be implemented by marking its related quest complete.
Travel and quest progression remain separate so that players can return and
play local stories in any supported order.

### Obstacles become optional content

When practical, existing characters and encounters should be moved beside the
main route instead of removed. Cut trees, unusual Pokémon, and story NPCs can
still draw attention to optional content without forcing the player to engage.

### Early arrival is a valid state

City initialization, visited flags, Pokémon Centers, saving, blackout recovery,
and return travel must work when the player arrives before the original story
expected them.

## Required changes

### 1. End mandatory onboarding after the starter

The player may be held in New Bark Town until they receive their starter. Once
they have a Pokémon, the west exit to Route 29 remains open in every
`VAR_NEWBARK_TOWN_STATE` value.

The Mystery Egg errand, rival theft, police report, Elm handoff, and conversation
with Mom remain available as story content. None of them may close the town exit
or control access to Route 30.

The Route 30 battle tableau must leave a clear northbound path before
`FLAG_MOM_VISITED` is set. If runtime validation shows that its actors close the
route, move them off the travel lane or change their visibility condition. Do
not set `FLAG_MOM_VISITED` early solely to clear the road.

Acceptance criteria:

- The player cannot leave New Bark Town without a starter.
- After receiving a starter, the player can reach Cherrygrove and Violet without
  returning the Mystery Egg or speaking to Mom.
- Returning to New Bark later still allows the unfinished opening story to
  continue without duplicated rewards or blocked scripts.

### 2. Open Route 32 south of Violet

Move the Route 32 guard out of the travel lane and remove the coordinate
turn-back behavior. The road to Union Cave remains open regardless of Sprout
Tower, Violet Gym, and Togepi Egg state.

The guard may still comment on local events and give the Miracle Seed when its
existing conditions are met. Those conditions affect the gift, not passage.

Acceptance criteria:

- A zero-badge player can travel from Violet City through Route 32, Union Cave,
  and Route 33 to Azalea Town.
- The route remains open before and after each former prerequisite.
- Declining or missing the Miracle Seed does not affect traversal.

### 3. Make the Ilex Forest quest optional

Keep the Farfetch'd quest and HM Cut reward, but remove Cut from the main path
between Azalea Town and Route 34. Move the Cut tree at `(32,40)` to a side branch
or provide a permanent walkable path around it.

The Silver encounter must not occupy Azalea Town's only southern exit. Move the
encounter to a nearby optional space or let the player initiate it through
dialogue. Defeating Bugsy may make the encounter available, but may not close
the road until the player wins.

Acceptance criteria:

- The player can travel between Azalea Town and Goldenrod City without Cut,
  completing the Farfetch'd quest, defeating Bugsy, or battling Silver.
- The Farfetch'd quest remains completable from either side of the forest.
- The Cut reward and any later quest state remain available after the player has
  already crossed the forest.

### 4. Move Sudowoodo off the Route 36 junction

Move the existing Sudowoodo object from `(39,19)` to the shrub-ring clearing at
`(37,17)`. Every arm of the Route 36 junction remains walkable while Sudowoodo
is present.

The Goldenrod flower-shop owner gives the SquirtBottle without checking the
Plain Badge and without requiring the player to visit Route 36 first. Sudowoodo
remains an optional level 20 encounter. Catching or defeating it clears the
encounter, while running leaves it available for another attempt.

Dialogue must not claim that Sudowoodo blocks the road or that Whitney's badge
is required to receive the SquirtBottle. Inspecting Sudowoodo without the item
should provide a light watering hint.

Acceptance criteria:

- The player can travel among Route 35, Route 37, Violet City, and the Ruins of
  Alph while Sudowoodo is present.
- Reaching Ecruteak from the south does not require the SquirtBottle, a badge, or
  the Sudowoodo battle.
- The existing catch, defeat, run, retry, object flag, and respawn behavior is
  preserved.

### 5. Add baseline transport to Cianwood

Add a free, bidirectional local ferry between the Olivine waterfront and
Cianwood City. The ferry is available after onboarding and requires no ticket,
badge, story flag, payment, HM, or party move.

Surf remains the direct player-controlled route across Routes 40 and 41. It may
continue to control access to the Whirl Islands and other water content, but it
is not the only way to enter or leave Cianwood.

The local ferry must remain distinct from the S.S. Aqua and must not advance or
depend on the Johto to Kanto ferry story.

Acceptance criteria:

- A zero-badge player with no HMs can travel from Olivine to Cianwood and back.
- Both ferry endpoints are accessible in every local story state.
- Using the ferry does not set S.S. Aqua, medicine, Gym, or Kanto story flags.
- Players who later obtain Surf can still cross Routes 40 and 41 normally.

### 6. Open Mahogany's east road

Move the RageCandyBar merchant beside the Route 44 road and remove the trigger
strips that push the player west during `VAR_MAHOGANY_TOWN_STATE` values 1
through 16.

The merchant may continue selling RageCandyBars and may change dialogue or
leave after the Radio Tower story. Buying, declining, lacking money, or having a
full bag must never affect passage.

Acceptance criteria:

- The player can leave Mahogany for Route 44 in every Mahogany story state.
- The merchant remains interactable without occupying the travel lane.
- The Rocket Hideout and Radio Tower stories retain their own progression
  state.

### 7. Remove the forced Ice Path obstruction

Move the Kimono Girl out of the only corridor between Route 44 and Blackthorn
City. Helping her may remain as an optional interaction in a nearby alcove or
side lane.

The player must not be trapped in a repeating yes/no prompt, stepped backward,
or required to complete the interaction before crossing Ice Path.

Acceptance criteria:

- A player entering Ice Path from either side can pass without speaking to the
  Kimono Girl.
- Helping her once still resolves the scene and its flags normally.
- Declining the interaction leaves both the route and the interaction available.

## Region-level acceptance

Starting from a fresh save immediately after receiving the starter, with zero
badges, no HMs, and no optional story completion, the player can make each of
these trips and return by the same connection:

| Destination | Required open route |
| --- | --- |
| Cherrygrove City | New Bark Town to Route 29 |
| Violet City | Cherrygrove City to Routes 30 and 31 |
| Azalea Town | Violet City to Route 32, Union Cave, and Route 33 |
| Goldenrod City | Azalea Town to Ilex Forest and Route 34 |
| Ecruteak City | Violet or Goldenrod to Routes 35, 36, and 37 |
| Olivine City | Ecruteak City to Routes 38 and 39 |
| Cianwood City | Local ferry from Olivine |
| Mahogany Town | Ecruteak City to Route 42, using the existing non-HM path |
| Blackthorn City | Mahogany Town to Route 44 and Ice Path |

For every destination:

- Arrival sets only the expected location and visit state.
- Pokémon Center healing, blackout recovery, saving, and reloading work.
- The player can leave without completing local content.
- Returning later does not duplicate or skip local story rewards.
- No NPC dialogue describes the open route as closed.
- No unavoidable fixed story battle prevents travel.

The same matrix must be repeated at the major story states that previously
created or removed roadblocks.

## Out of scope

- Choosing a starting city other than New Bark Town
- Opening Gyms in any order
- Gym Leader, trainer, or wild encounter level scaling
- Reordering or rewriting the main Johto story
- Changing the Pokémon League, Tohjo Falls, Reception Gate, Victory Road, or
  Indigo Plateau requirements
- Opening Kanto before its intended regional transition
- Removing the need to teach field moves to compatible Pokémon
- Redesigning optional HM branches, side dungeons, or item paths
- Automatically completing story quests when their original location is
  reached from a different direction

These systems will need separate product decisions before the full game supports
open Gym and story progression. This PRD establishes the city network they can
build on.
