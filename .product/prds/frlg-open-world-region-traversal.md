# FireRed and LeafGreen open-world regional traversal

## Player outcome

After the opening releases the player with a starter, every Kanto town and city is reachable without completing the main story, earning badges, or owning an HM. Native Surf users may provide the route to Cinnabar. The player may take a long route, but determination is enough.

The Sevii Islands require the Seagallop shakedown in Vermilion. It awards the Rainbow Pass. Once unlocked, travel to and between its towns and cities has no further story requirement and never requires HM field use.

## Rules

- Walking, doors, always-available local transport, and the approved native Surf crossing form the core settlement network.
- Main-story state, Gym progress, and forced victories cannot close the only route to a town or city in an unlocked region. That route cannot require an HM item. Only the Cinnabar crossing may require a Pokémon that already knows Surf.
- Fly may make settlement travel faster. Surf remains optional except for the approved Cinnabar route.
- Cut, Rock Smash, Surf, bicycles, and local story events may gate shortcuts, hidden locations, and optional content.
- A Pokémon that already knows another HM move may open an optional route before the matching HM is found. That route does not count toward the core settlement network.
- A story-gated shortcut must be self-contained. It cannot depend on a badge count or unrelated story flags from other cities.
- First-time travel to another region may require a self-contained local story and a key item. It cannot depend on Gym progress or a chain of unrelated campaign flags.
- Every inter-region travel service is frictionless after its region is unlocked and always includes a return option.
- Opening a travel lane does not complete the story attached to it or award skipped rewards.

## Settlement coverage

| Region | Settlements in scope | Core access contract |
| --- | --- | --- |
| Kanto | Pallet Town, Viridian City, Pewter City, Cerulean City, Vermilion City, Lavender Town, Celadon City, Saffron City, Fuchsia City, and Cinnabar Island | Pallet through Cerulean use the Route 1, Route 2, Viridian Forest, Route 3, Mt. Moon, and Route 4 land spine. Cerulean's south and east exits plus the four open Saffron gates connect Vermilion, Lavender, Celadon, and Saffron. Fuchsia retains one unresolved land approach. Native Surf connects Pallet to Cinnabar through Route 21. Existing Horsea and Krabby fishing encounters provide both species on both sides in FireRed and LeafGreen. Route 20 and Seafoam remain optional. |
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

These three decisions are required before an implementation spec can claim complete Kanto coverage. They are not permission to invent story-completion flags; the chosen lane must preserve the associated story and reward.

## Native Surf recovery boundary

The Standard Rod fishing PRD makes the existing Horsea and Krabby slots eligible with the Old Rod. A player who already has the rod and capture supplies can therefore obtain another native Surf user from the existing encounter tables. This PRD does not change encounters, terrain, rod distribution, fishing probabilities, or capture-supply availability.

Acceptance here covers a prepared player crossing Route 21 in both directions. It does not guarantee recovery when the player lacks the Old Rod or Poké Balls, has a full party, or deposits, releases, or forgets Surf on the last user.

A separate traversal-recovery PRD owns those softlock-prevention and emergency-return requirements. This PRD may be accepted independently and must not be cited as proof that Kanto's settlement network is softlock-safe.

## Out of scope

- Indigo Plateau, Route 23's badge challenge, and Victory Road are endgame content.
- Birth Island, Navel Rock, legendary encounters, optional caves, and Trainer Tower are optional destinations.
- Cerulean to Route 9 and the unused Fuchsia approach may remain gated shortcuts. Cycling Road may remain Bicycle-gated only if Route 12 is selected as Fuchsia's core lane.
- This pass does not redesign shortcut rewards or optional encounters unless they block the only settlement route.

## Target acceptance after open decisions

- Resolve every row under Unresolved settlement blocks, then record the chosen core lane in Settlement coverage before implementation begins.
- From a new save after the opening, visit the ten named Kanto settlements without badges, HM items, story completion flags, or forced victories. Native Surf is allowed only for Cinnabar.
- Run every required core settlement route with no HM items in the Bag. No party Pokémon may know an HM move other than Surf on the Cinnabar crossing.
- Use each bypass first, then return and complete its preserved story normally.
- Complete the Seagallop shakedown and receive the Rainbow Pass without leaving Vermilion or satisfying another story flag.
- Open Sevii service while the S.S. Anne is still present, use both dock choices, and confirm neither choice changes the other's independent story state.
- Confirm the first Sevii trip goes to One Island, Celio's introduction does not start the Meteorite delivery, and the ferry then exposes the permanent eight-destination menu.
- Visit all seven named Sevii settlement hubs and return to Vermilion without further story checks, HM items, known HM moves, field-move use, forced battles, or a lost return option.
- In both versions, confirm native Horsea and Krabby remain in the existing fishing tables on the Pallet and Cinnabar sides. With a native Surf user prepared before each crossing, cross Route 21 in both directions without HM03 or a badge.
- Confirm FireRed and LeafGreen behave the same.

## References

- [Story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
- [Badge-free HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](standard-rod-fishing.md)
