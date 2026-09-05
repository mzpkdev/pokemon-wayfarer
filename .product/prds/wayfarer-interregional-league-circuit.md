# Wayfarer interregional League circuit

## Intent

Give Wayfarer's open world a clear long-term arc without requiring the player
to finish one region before exploring another. Badges earned in Kanto, Johto,
and Hoenn all advance the same career, while three fixed League destinations
provide authored difficulty peaks after each group of eight badges.

## Design

The player is told the complete League itinerary near the start of the game:

| Career tier | Qualification | League destination |
| --- | ---: | --- |
| Tier 1 | 8 total badges | Kanto League |
| Tier 2 | 16 total badges and Kanto League cleared | Johto League |
| Tier 3 | 24 total badges and Kanto and Johto Leagues cleared | Hoenn League |

Any sanctioned Gym Badge from Kanto, Johto, or Hoenn counts toward these
totals. A player may qualify for Kanto with four Hoenn badges and four Johto
badges, for example. Badge origin never changes its value for League
qualification.

Each tier has a badge certification cap. After receiving the eighth badge, the
player must clear the Kanto League before another Gym can award a badge. The
Kanto clear raises the cap to sixteen. The Johto clear raises it to twenty-four.
Clearing the Hoenn League completes the circuit. Badges remain earned and are
never spent or reset when a League is cleared.

The cap applies only to official badge awards. The player may continue to
travel, catch Pokémon, battle ordinary Trainers, and complete available story
content while qualified for a League. A Gym Leader whose unearned badge would
exceed the current cap postpones the official challenge without consuming the
battle, badge, reward, or related one-time state.

A new game lets the player start in Kanto, Johto, or Hoenn. Every choice begins
with zero badges, no League clears, and Rating 0. Kanto is the first League
destination, not a required starting region. The itinerary supplies a directed
career structure while the player chooses the route, regions, Gym order,
party, and loose story order between League challenges.

## Boundaries

The circuit does not gate regional travel, general exploration, or unrelated
story content. Regional story prerequisites may continue to inspect their own
badge and story states where another approved feature has not removed them.
Only League qualification, badge certification caps, and Trainer Rating use
the global badge total.

Ordinary Trainers and Gym Leaders keep authored, static parties. They do not
scale from Trainer Rating, total badges, or League progress. Each regional
League has one authored party set for its fixed position in the itinerary:
Kanto is Tier 1, Johto is Tier 2, and Hoenn is Tier 3. League parties do not
scale at runtime.

The fixed order describes this campaign's interregional circuit. It does not
establish that one region's League is universally more prestigious than
another outside the circuit.

## Balance

The first eight badges provide the most route freedom because all twenty-four
Gyms are potential choices. Clearing a League removes no options, but the pool
of unearned badges naturally narrows during later tiers. Players may seek
easier badges first or attempt stronger authored Gym teams early.

League difficulty must increase across the fixed itinerary. Kanto is balanced
as the first major test after eight badges, Johto as the second after sixteen,
and Hoenn as the final test after all twenty-four. Exact parties belong to
regional Trainer content, but their intended order cannot be reversed by the
player's starting region or badge route.

## Presentation

At the start of each tier, the game communicates the next League destination,
the total badge requirement, and the current total. The player can review that
goal again without returning to the original announcement NPC or scene.

When the player reaches a certification cap, the game directs them to the
qualified League. A Gym Leader who cannot award another badge explains that
the player must clear the current League tier before continuing the official
Gym circuit.

Regional badge displays retain badge identity and origin. Any career summary
also shows the global total out of twenty-four and the next League goal. After
the Hoenn League clear, it instead reports that the circuit is complete and
shows no further League destination.

## Interactions

Trainer Rating starts at zero and follows the global circuit milestones:

| Progress | Trainer Rating |
| --- | ---: |
| New game | 0 |
| 4 total badges | 16 |
| 8 total badges | 40 |
| Kanto League cleared | 55 |
| 16 total badges | 63 |
| Johto League cleared | 68 |
| 24 total badges | 76 |
| Hoenn League cleared | 80 |

The rating remains a high-water mark used by ordinary wild encounter scaling
and the party's soft level cap and obedience rules. League and Gym Trainer
parties remain authored and static.

Rating 0 must not remove a native utility catch that supplies an approved core
route. In particular, the level-5 Chinchou available around Vermilion and
Cinnabar must know Surf in Wayfarer so Kanto's native-Surf route remains valid.

Every League approach must be reachable once its circuit requirements are met.
It cannot add a local badge minimum or require completion of an unrelated
regional story. Travel to the venue remains part of the journey.

## Constraints

Johto, Kanto, and Hoenn keep separate badge and Champion state for regional
scripts, presentation, and save isolation. The global count is derived from
those twenty-four badge states rather than stored as a second mutable badge
total.

Badge certification caps cannot ship until interregional travel guarantees
that a player can reach the assigned League from any region where the
threshold badge can be earned. After the League, the player must regain control
with access to the wider regional travel network. The journey may use
directional transport and authored routes, but it cannot depend on earning
another badge or clearing the League that the player is trying to reach.

Each regional start needs an approved opening, starter choice, safe recovery
point, and path into its open settlement network before the circuit can ship.
Choosing a start cannot permanently lock the player out of either other
region.

Prerelease save compatibility is not required. This feature does not require
shared systems to preserve behavior in other product builds.

## Playtesting

Playtesting must cover starts in all three regions and mixed badge routes at
each tier. It should confirm that the next League goal remains clear, reaching
a badge cap never blocks exploration, and a postponed Gym challenge remains
available after the required League clear.

The three League encounters should feel like distinct increases in difficulty
for parties built through easier, mixed, and deliberately high-risk badge
routes. Testing should also check the travel time between the eighth,
sixteenth, and twenty-fourth badge locations and their assigned League venues.

## References

- [Technical specification](../specs/wayfarer-interregional-league-circuit.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Wayfarer Hoenn integration](wayfarer-hoenn-integration.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](standard-rod-fishing.md)
