# FireRed and LeafGreen open-world regional traversal

## Player outcome

After the opening releases the player with a starter, every Kanto town and city is reachable without completing the main story, earning badges, or owning an HM. Native Surf users may provide the route to Cinnabar. The player may take a long route, but determination is enough.

The Sevii Islands require the Seagallop shakedown in Vermilion. It awards the Rainbow Pass. Once unlocked, travel to and between its towns and cities has no further story requirement and never requires HM field use.

## Rules

- Walking, doors, always-available local transport, and the approved native Surf crossing form the core settlement network.
- Main-story state, Gym progress, and forced scripted or story victories cannot close the only route to a town or city in an unlocked region. That route cannot require an HM item. Only the Cinnabar crossing may require a Pokémon that already knows Surf.
- Ordinary sight-based trainers may challenge the player on a core route. They retain their existing positions, sight ranges, and authored parties and do not count as story-gated traversal. They do not scale with player progress.
- Fly may make settlement travel faster. Surf remains optional except for the approved Cinnabar route.
- Cut, Rock Smash, Surf, bicycles, and local story events may gate shortcuts, hidden locations, and optional content.
- A Pokémon that already knows another HM move may open an optional route before the matching HM is found. That route does not count toward the core settlement network.
- A self-contained local encounter may occupy the only settlement lane only when it requires no badge, HM item, key item, or earlier story state; declining or losing leaves it available to retry; and winning is unnecessary because fleeing or another non-victory outcome completes it. Every required reward and state transition must be committed before the blocker is removed. If that commit fails, the encounter and reward remain available to retry. Any other story actor or scripted battle must leave another visible lane open.
- This exception applies only when this PRD explicitly names the encounter as the selected core lane. It does not replace an approved bypass or optional-scene requirement.
- A story-gated shortcut must be self-contained. It cannot depend on a badge count or unrelated story flags from other cities.
- First-time travel to another region may require a self-contained local story and a key item. It cannot depend on Gym progress or a chain of unrelated campaign flags.
- Every inter-region travel service is frictionless after its region is unlocked and always includes a return option.
- A bypass or roadblock removal does not complete its attached story or award skipped rewards. Completing a permitted self-contained encounter may advance only that encounter's local state and grant its normal reward.

## Settlement coverage

| Region | Settlements in scope | Core access contract |
| --- | --- | --- |
| Kanto | Pallet Town, Viridian City, Pewter City, Cerulean City, Vermilion City, Lavender Town, Celadon City, Saffron City, Fuchsia City, and Cinnabar Island | Pallet through Cerulean use the Route 1, Route 2, Viridian Forest, Route 3, Mt. Moon, and Route 4 land spine. Cerulean's south and east exits plus the four open Saffron gates connect Vermilion, Lavender, Celadon, and Saffron. Route 12 is the core Fuchsia approach; its Snorlax remains asleep beside an open lane. Route 16 and Cycling Road remain an optional Bicycle shortcut. Native Surf connects Pallet to Cinnabar through Route 21. Existing Horsea and Krabby fishing encounters provide both species on both sides in FireRed and LeafGreen. Route 20 and Seafoam remain optional. |
| Sevii Islands | The settlement hubs on One, Two, Three, Four, Five, Six, and Seven Island | The Seagallop service is the core network. The unlock trip introduces One Island; immediately afterward, every Sevii port offers all seven islands and Vermilion. |

Indigo Plateau and event-only islands are not settlements in this pass. A route, cave, or optional landmark does not enter scope merely because it has a healing point.

## Approved Kanto changes

| Connection | Required behavior |
| --- | --- |
| Viridian north road | After the player receives a starter, place the old man beside the road instead of across it. Oak's Parcel, the Pokédex scene, coffee dialogue, and catching tutorial remain available, but none may block Route 2. |
| Pewter east road | Keep the Gym guide and the upper two escort triggers. Remove the bottom and right triggers so the player may leave through the lower lane without starting the escort. The guide's dialogue, movement, visibility, and scene state remain available through the retained triggers. |
| Mt. Moon Fossil room | Keep Super Nerd Miguel and both Fossils in place. Remove the automatic approach trigger beside Miguel so the player may pass through the open tile. Speaking to Miguel still starts the original battle, and victory still unlocks the original Fossil choice. |
| Cerulean east and south exits | Stop the transition logic from moving the policeman and Slowbro pair into blocking positions. Bill, Nugget Bridge, the rival battle, and the northern story route remain available. |
| Four Saffron gates | Allow direct passage without Tea. Each gate may still advance its local scene when first crossed. |
| Route 12 Snorlax | Move Snorlax and its underfoot Leftovers one tile east, leaving the north-south lane open. Keep the Poké Flute interaction, encounter, hide flag, wake flag, and Leftovers reward unchanged at the new tile. Do not change Route 16 or Cycling Road. |

## Sevii unlock story

### Seagallop shakedown

- Start: speak to the port builder beside Machop's construction lot in Vermilion. This is available on the first visit to the city.
- Job: inspect three marked soft spots in the construction lot. Machop follows the player to each marker and tamps it down. This uses no battle, item, field move, badge, or earlier story flag.
- Turn-in: report to the builder, who certifies the pier for Seagallop service. The ferry sailor gives the player the Rainbow Pass and adds Sevii Islands to the shared dock's service choices. This does not change the S.S. Anne scene.
- Shared dock: while the S.S. Anne is present, the sailor offers S.S. Anne and Sevii Islands. The S.S. Anne choice keeps its existing Ticket, boarding, captain, rival, Cut, and departure behavior. Sevii Islands never advances or departs the S.S. Anne. After the player causes the ship's normal departure, the sailor offers only Sevii Islands.
- First Sevii trip: choosing Sevii Islands for the first time takes the player to One Island. Celio gives a new travel-only introduction and expands the Town Map. Bill remains hidden until his normal Cinnabar trip. This introduction does not give the Meteorite or Tri-Pass, disable PC storage, move Bill, or advance the existing One Island quest state.
- Existing Meteorite story: Bill's normal Cinnabar invitation remains the entry to the original Meteorite delivery after early Sevii travel. That later trip may reuse the player's Rainbow Pass, but it still starts the original local quest and preserves its rewards and state transitions without giving a duplicate pass.
- Permanent service: as soon as Celio's introduction ends, the One Island sailor offers One through Seven Island and Vermilion. Every other Sevii port uses that same eight-destination menu from its first visit. The Rainbow Pass is the only credential.
- State isolation: visiting an island marks that destination visited and initializes only the baseline NPC and transport state needed to enter and leave it. It does not complete Lostelle, the biker invasion, Hypno, the Meteorite, the Ruby, the Champion, or the National Pokédex progression. The shared rival scene on Four or Six Island remains deferred until its original Sevii prerequisite is met and plays exactly once at whichever location resolves first.
- Return guarantee: the player can always select Vermilion or another island. Starting any local island story never removes the ferry menu.

## Native Surf recovery boundary

The Standard Rod fishing PRD makes the existing Horsea and Krabby slots eligible with the Old Rod. A player who already has the rod and capture supplies can therefore obtain another native Surf user from the existing encounter tables. This PRD does not change encounters, terrain, rod distribution, fishing probabilities, or capture-supply availability.

Acceptance here covers a prepared player crossing Route 21 in both directions. It does not guarantee recovery when the player lacks the Old Rod or Poké Balls, has a full party, or deposits, releases, or forgets Surf on the last user.

A separate traversal-recovery PRD owns those softlock-prevention and emergency-return requirements. This PRD may be accepted independently and must not be cited as proof that Kanto's settlement network is softlock-safe.

## Out of scope

- Indigo Plateau, Route 23's badge challenge, and Victory Road are endgame content.
- Birth Island, Navel Rock, legendary encounters, optional caves, and Trainer Tower are optional destinations.
- Cerulean to Route 9 and Route 16 may remain gated shortcuts. Cycling Road remains Bicycle-gated because Route 12 is Fuchsia's core lane.
- This pass does not redesign shortcut rewards or optional encounters unless they block the only settlement route.

## Acceptance

- From a new save after the opening, visit the ten named Kanto settlements without badges, HM items, earlier or unrelated story-completion flags, or a required scripted or story battle victory. Ordinary sight-based trainer battles are permitted. Native Surf is allowed only for Cinnabar.
- Run every required core settlement route with no HM items in the Bag. No party Pokémon may know an HM move other than Surf on the Cinnabar crossing.
- Leave Pewter through the free lower lane, pass Miguel without a battle, and walk past Route 12 Snorlax. Then return and complete each preserved scene and reward normally.
- Complete the Seagallop shakedown and receive the Rainbow Pass without leaving Vermilion or satisfying another story flag.
- Open Sevii service while the S.S. Anne is still present, use both dock choices, and confirm neither choice changes the other's independent story state.
- Confirm the first Sevii trip goes to One Island, Celio's travel introduction does not start the Meteorite delivery, and the ferry then exposes the permanent eight-destination menu.
- After early Sevii travel, take Bill's normal Cinnabar trip and confirm the Meteorite story starts without replacing or duplicating the Rainbow Pass.
- Visit all seven named Sevii settlement hubs and return to Vermilion without further story checks, HM items, known HM moves, field-move use, forced battles, or a lost return option.
- Enter Two Island's Game Corner and Three Island's port before starting the original detour. Confirm Lostelle, the bikers, PC storage, and detour-completion state remain unchanged.
- Reach Four and Six Island before the postgame Sevii story in both visit orders and confirm the shared rival scene remains pending. Satisfy the original prerequisite later and confirm it plays exactly once at whichever location resolves first.
- In both versions, confirm native Horsea and Krabby remain in the existing fishing tables on the Pallet and Cinnabar sides. With a native Surf user prepared before each crossing, cross Route 21 in both directions without HM03 or a badge.
- Confirm FireRed and LeafGreen behave the same.
- Confirm this pass does not change the position, trainer type, or sight range of any ordinary sight-based trainer.

## References

- [Story-blocking traversal audit](../research/story-blocking-traversal-audit.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Badge-free HM field use](hm-field-use.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](standard-rod-fishing.md)
