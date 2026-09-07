# Wayfarer interregional League circuit

Implemented: Outdated

The implementation still uses +15/+5/+4 League rewards and static League
levels. This revision requires +8/+8/+8 and the separate League scaling design.

## Intent

Give Wayfarer's open world a clear long-term arc without requiring the player
to finish one region before exploring another. Badges earned in Kanto, Johto,
and Hoenn all advance the same career, while three fixed League destinations
provide optional challenges unlocked by badge progress. Players can earn all
twenty-four badges before attempting any League.

## Design

The Trainer Card presents the complete League itinerary on demand:

| Career tier | Qualification | League destination |
| --- | ---: | --- |
| Tier 1 | 8 total badges | Kanto League |
| Tier 2 | 16 total badges and Kanto League cleared | Johto League |
| Tier 3 | 24 total badges and Kanto and Johto Leagues cleared | Hoenn League |

Any sanctioned Gym Badge from Kanto, Johto, or Hoenn counts toward these
totals. A player may qualify for Kanto with four Hoenn badges and four Johto
badges, for example. Badge origin never changes its value for League
qualification.

These badge requirements are minimums, with no badge certification caps. League
participation never gates earning another badge. A player with twenty-four
badges and no clears can challenge Kanto, then Johto, then Hoenn consecutively.
Clearing Hoenn completes the circuit. Badges remain earned and are never spent
or reset when a League is cleared.

This release has one new-game start: the existing Johto opening. It begins with
zero badges, no League clears, and Rating 0. Kanto is the first League
destination, not a required starting region. The itinerary supplies a directed
career structure while the player chooses the route, regions, available Gyms,
party, and when to attempt Leagues. Kanto and Hoenn starts
are future features; they do not constrain this circuit release.

## Boundaries

The circuit does not gate regional travel, general exploration, or badge
collection. Coherent regional story prerequisites may remain, including
Jasmine's medicine errand, Misty's Power Plant sequence, and Juan's weather
storyline. A quest may span towns or a region, provided the player can discover
and complete its prerequisites without having missed an earlier event or
returning merely to activate an unrelated story flag. League clears must never
be prerequisites for an initial badge.

Regional prerequisites mean that not every Gym is immediately available in any
order. Clair may still require Chuck, Jasmine, and Pryce's badges, and Norman
may still require four Hoenn badges and Wally's tutorial. Blue's Cinnabar
interaction alone invites him back to his Gym; the requirement to earn the
other fifteen Kanto/Johto badges is removed.

Story events must recognize satisfied regional prerequisites on a revisit,
without replaying completed events or undoing later progress. In particular,
the Rocket takeover must not depend on a shared badge counter being exactly
seven, or on which region supplied that badge. Its trigger must use explicit
regional story prerequisites, rather than a replacement global badge threshold.

Undefeated Leaders must remain accessible for their initial badge. Wattson's
New Mauville relocation requires both Norman's defeat and the Dynamo Badge,
with either completion order recognized. The
existing initial-badge protections for Chuck, Blue, Clair, and Blaine remain.
Only League qualification and Trainer Rating use the global badge total.

Ordinary Trainers and Gym members follow the separate
[Trainer-party scaling design](trainer-party-scaling.md). Initial Gym Leader
badge battles follow the separate [Gym Leader scaling design](gym-leader-scaling.md);
leader rematches retain authored, static parties. Each regional
League has one authored party set for its fixed position in the itinerary:
Kanto is Tier 1, Johto is Tier 2, and Hoenn is Tier 3. Their levels follow the
[League scaling design](league-scaling.md), using TR locked on run admission.
Species, party sizes, moves, held items, abilities, and AI remain authored.

The fixed order describes this campaign's interregional circuit. It does not
establish that one region's League is universally more prestigious than
another outside the circuit.

## Balance

Players may seek easier available badges first or attempt stronger authored Gym
teams early, subject to the retained regional prerequisites. League levels follow the
[League scaling design](league-scaling.md), with a gradual climb through each
run and the same authored rosters. Playtest early and postponed attempts, plus
the training needed between successive runs after each +8 reward.

## Presentation

The player presses Select on the local Trainer Card to review the circuit.
The view shows the badge total out of twenty-four and all three Leagues, each
with its badge minimum, prerequisite clears, and locked, available, or cleared
state. After Hoenn, it retains the completed itinerary and reports
`Circuit complete`. Regional badge displays retain badge identity and origin.

There are no automatic circuit announcements: no opening itinerary, badge
qualification popups, or post-clear status messages. NPCs do not duplicate the
circuit overview. A League admission refusal may state its immediate unmet
requirement. Dialogue must not assume the player is a Champion merely because
they have many badges.

## Interactions

Trainer Rating keeps its existing badge contribution. Each first-time League
clear adds +8 TR, for +24 across the circuit. Record the reward after the
successful run; losses, repeated completion callbacks, and repeat clears award
no additional TR. These examples show taking each League as soon as its badge
minimum is reached:

| Progress | Trainer Rating |
| --- | ---: |
| New game | 0 |
| 4 total badges | 16 |
| 8 total badges | 40 |
| Kanto League cleared | 48 |
| 16 total badges and Kanto cleared | 56 |
| Johto League cleared | 64 |
| 24 total badges and Kanto and Johto cleared | 72 |
| Hoenn League cleared | 80 |

Collecting all badges first is also valid:

| With 24 badges | Trainer Rating | Soft level cap |
| --- | ---: | ---: |
| No League clears | 56 | 62 |
| Kanto cleared | 64 | 78 |
| Kanto and Johto cleared | 72 | 89 |
| All three cleared | 80 | 100 |

The rating remains a high-water mark used by ordinary wild encounter scaling
and the party's soft level cap and obedience rules. It also drives ordinary
Trainer and Gym-member scaling, plus the separate [initial Gym Leader badge
battle scaling](gym-leader-scaling.md). League levels use their separate
[scaling design](league-scaling.md); leader rematches remain static.

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

Interregional travel must guarantee that a player can reach an eligible League
from any region, including after collecting all badges. After the League, the
player must regain control with access to the wider regional travel network.
The journey may use
directional transport and authored routes, but it cannot depend on earning
another badge or clearing the League that the player is trying to reach.

The existing Johto opening and its S.S. Aqua maiden voyage are this release's
entry contract. They must provide a route to Kanto, then the completed Aqua
circuit must provide the Vermilion-to-Slateport and Slateport-to-Olivine legs.
Future Kanto and Hoenn starts need their own approved openings when they are
scoped; they do not gate League eligibility.

Prerelease save compatibility is not required. This feature does not require
shared systems to preserve behavior in other product builds.

## Playtesting

Playtesting must prove a Johto-start journey to all twenty-four badges with zero
League clears, including mixed regional orders and recoverable story events.
Cover deferred Whitney and Clair badge rewards, Blue's invitation, and Wattson
remaining available when Norman is defeated first. Verify that a mixed badge
order which misses the old exact-seven Rocket trigger still reaches the story
and Clair without replaying completed events.

With all badges earned, test consecutive Kanto, Johto, and Hoenn clears,
including the shared Indigo venue's tier reset, loss/retry, save/load, and
return to regional travel. Check all Trainer Card states and confirm that
opening, badge awards, and League clears produce no circuit announcements.

## References

- [Technical specification](../specs/wayfarer-interregional-league-circuit.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Wayfarer Hoenn integration](wayfarer-hoenn-integration.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](standard-rod-fishing.md)
