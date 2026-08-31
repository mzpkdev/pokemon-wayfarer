# FireRed and LeafGreen open-world regional traversal

## Player outcome

After the opening releases the player with a starter, every Kanto town and city is reachable without completing the main story, earning badges, owning an HM, or using an HM move in the field. The player may take a long route, but determination is enough.

The Sevii Islands require the Seagallop shakedown in Vermilion. It awards the Rainbow Pass. Once unlocked, travel to and between its towns and cities has no further story requirement and never requires HM field use.

## Rules

- Walking, doors, and always-available local transport form the core settlement network.
- Main-story state, Gym progress, and forced victories cannot close the only route to a town or city in an unlocked region. That route also cannot require an HM item or a Pokémon that knows an HM move.
- Surf and Fly may make settlement travel faster. They are never required for basic settlement access.
- Cut, Rock Smash, Surf, bicycles, and local story events may gate shortcuts, hidden locations, and optional content.
- A Pokémon that already knows an HM move may open an optional route before the matching HM is found. That route does not count toward the core settlement network.
- A story-gated shortcut must be self-contained. It cannot depend on a badge count or unrelated story flags from other cities.
- First-time travel to another region may require a self-contained local story and a key item. It cannot depend on Gym progress or a chain of unrelated campaign flags.
- After a region is unlocked, repeat travel is frictionless and always includes a return route.
- Opening a travel lane does not complete the story attached to it or award skipped rewards.

## Settlement coverage

| Region | Settlements in scope | Core access contract |
| --- | --- | --- |
| Kanto | Pallet Town, Viridian City, Pewter City, Cerulean City, Vermilion City, Lavender Town, Celadon City, Saffron City, Fuchsia City, and Cinnabar Island | Pallet through Cerulean use the Route 1, Route 2, Viridian Forest, Route 3, Mt. Moon, and Route 4 land spine. Cerulean's south and east exits plus the four open Saffron gates connect Vermilion, Lavender, Celadon, and Saffron. The Fuchsia and Cinnabar lanes remain the two route-design decisions listed below. |
| Sevii Islands | The settlement hubs on One, Two, Three, Four, Five, Six, and Seven Island | The Seagallop service is the core network. The unlock trip introduces One Island; immediately afterward, every Sevii port offers all seven islands and Vermilion. |

Indigo Plateau and event-only islands are not settlements in this pass. A route, cave, or optional landmark does not enter scope merely because it has a healing point.

## Approved Kanto changes

| Connection | Required behavior |
| --- | --- |
| Viridian north road | After the player receives a starter, place the old man beside the road instead of across it. Oak's Parcel, the Pokédex scene, coffee dialogue, and catching tutorial remain available, but none may block Route 2. |
| Cerulean east and south exits | Stop the transition logic from moving the policeman and Slowbro pair into blocking positions. Bill, Nugget Bridge, the rival battle, and the northern story route remain available. |
| Four Saffron gates | Allow direct passage without Tea. Each gate may still advance its local scene when first crossed. |

## Sevii unlock story

### Seagallop shakedown

- Start: speak to the port builder beside Machop's construction lot in Vermilion. This is available on the first visit to the city.
- Job: inspect three marked soft spots in the construction lot. Machop follows the player to each marker and tamps it down. This uses no battle, item, field move, badge, or earlier story flag.
- Turn-in: report to the builder, who certifies the pier for Seagallop service. The ferry sailor gives the player the Rainbow Pass and adds Sevii Islands to the shared dock's service choices. This does not change the S.S. Anne scene.
- Shared dock: while the S.S. Anne is present, the sailor offers S.S. Anne and Sevii Islands. The S.S. Anne choice keeps its existing Ticket, boarding, captain, rival, Cut, and departure behavior. Sevii Islands never advances or departs the S.S. Anne. After the player causes the ship's normal departure, the sailor offers only Sevii Islands.
- First Sevii trip: choosing Sevii Islands for the first time takes the player to One Island. Celio gives the existing regional introduction and expands the Town Map, but does not start the Meteorite delivery unless the player separately accepts it.
- Permanent service: as soon as Celio's introduction ends, the One Island sailor offers One through Seven Island and Vermilion. Every other Sevii port uses that same eight-destination menu from its first visit. The Rainbow Pass is the only credential.
- State isolation: visiting an island marks that destination visited and initializes only the baseline NPC and transport state needed to enter and leave it. It does not complete Lostelle, the biker invasion, Hypno, the Meteorite, the Ruby, the Champion, or the National Pokédex progression.
- Return guarantee: the player can always select Vermilion or another island. Starting any local island story never removes the ferry menu.

## Unresolved settlement blocks

| Block | Access affected | Decision needed |
| --- | --- | --- |
| Pewter escort | Pewter's east exit | Move the escort or leave a second lane open. |
| Mt. Moon Super Nerd | Pewter to Cerulean | Add a bypass that preserves the Fossil battle and reward. |
| Route 12 and Route 16 Snorlax | Fuchsia City | Choose the core lane: either leave a walkable lane around Route 12 Snorlax, or leave a walkable lane around Route 16 Snorlax and allow Cycling Road entry without owning the Bicycle. Preserve both encounters; the unused approach may remain a shortcut. |
| Route 20 and Route 21 Surf | Cinnabar Island | Choose one ordinary transport connection from Pallet or Fuchsia to Cinnabar, including its operator, boarding point, arrival point, and always-available return trip. Seafoam and both Surf routes may remain optional. |

These four decisions are required before an implementation spec can claim complete Kanto coverage. They are not permission to invent story-completion flags; the chosen lane must preserve the associated story and reward.

## Out of scope

- Indigo Plateau, Route 23's badge challenge, and Victory Road are endgame content.
- Birth Island, Navel Rock, legendary encounters, optional caves, and Trainer Tower are optional destinations.
- Cerulean to Route 9 and the unused Fuchsia approach may remain gated shortcuts. Cycling Road may remain Bicycle-gated only if Route 12 is selected as Fuchsia's core lane.
- This pass does not redesign shortcut rewards or optional encounters unless they block the only settlement route.

## Target acceptance after open decisions

- Resolve every row under Unresolved settlement blocks, then record the chosen core lane in Settlement coverage before implementation begins.
- From a new save after the opening, visit the ten named Kanto settlements without badges, HM items, story completion flags, field-move use, or forced victories.
- Run every required core settlement route with no HM items in the Bag and a party in which no Pokémon knows an HM move.
- Use each bypass first, then return and complete its preserved story normally.
- Complete the Seagallop shakedown and receive the Rainbow Pass without leaving Vermilion or satisfying another story flag.
- Open Sevii service while the S.S. Anne is still present, use both dock choices, and confirm neither choice changes the other's independent story state.
- Confirm the first Sevii trip goes to One Island, Celio's introduction does not start the Meteorite delivery, and the ferry then exposes the permanent eight-destination menu.
- Visit all seven named Sevii settlement hubs and return to Vermilion without further story checks, HM items, known HM moves, field-move use, forced battles, or a lost return option.
- Confirm FireRed and LeafGreen behave the same.

## References

- [Story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
- [Badge-free HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
