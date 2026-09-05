# Wayfarer interregional League circuit

PRD: [Wayfarer interregional League circuit](../prds/wayfarer-interregional-league-circuit.md)
Implemented: No

## Scope

This specification defines Wayfarer's global badge count, badge certification
caps, fixed Kanto to Johto to Hoenn League sequence, League eligibility, and
Trainer Rating inputs. It also establishes Kanto, Johto, and Hoenn as the three
Wayfarer new-game starts and the minimum travel guarantees needed by the
circuit. It supersedes build-specific League entry, Trainer Rating progression,
HNS-only start, and one-way regional-travel assumptions only in the completed
Wayfarer circuit.

Regional badge storage, Champion state, and local story dispatch remain owned
by the Wayfarer runtime foundation and regional content specifications. The
ordinary wild population and level projection remain owned by the Trainer
Rating wild encounter scaling specification.

## Behavior

### Global badge count

The global badge count is the sum of the earned Kanto, Johto, and Hoenn badge
states:

```text
globalBadges = kantoBadges + johtoBadges + hoennBadges
```

Each regional count is between zero and eight, so the global result is between
zero and twenty-four. The calculation reads the region-aware badge helpers and
does not maintain a separate saved count. Re-reading an earned badge or running
its award script again cannot increase the result.

Global aggregation does not change the meaning of a regional badge. Local
story scripts continue to read their regional state unless this specification
explicitly assigns a global circuit check.

### Starting region

Before running a regional opening on a new Wayfarer save, the game offers
Kanto, Johto, and Hoenn as start choices. Each choice initializes all three
regional state banks, then dispatches the approved opening for the selected
region. Control is released at a safe recovery point connected to that
region's open settlement network.

Every choice begins with zero global badges, no regional Champion or game-clear
result, and Trainer Rating 0. The selected start may determine the opening
scene, first settlement, and starter roster, but it grants no circuit progress
and does not alter the fixed League order. Each regional content specification
must define its exact opening and starter transaction before that choice can be
enabled.

Starting in one region cannot permanently lock either other region. The travel
graph may require an authored journey or credential, but not a badge, League
clear, or unrelated regional story chain merely because of the start choice.

### Circuit state and certification cap

The next required League clear determines the current badge certification cap:

| Required League | Prerequisite clears | Certification cap |
| --- | --- | ---: |
| Kanto | None | 8 |
| Johto | Kanto | 16 |
| Hoenn | Kanto and Johto | 24 |
| Circuit complete | Kanto, Johto, and Hoenn | 24 |

Before an unearned badge can start its reward-bearing Gym Leader challenge,
the Gym script checks the global badge count against the current cap. If the
count is below the cap, the challenge proceeds normally. If the count is equal
to or greater than the cap, the challenge is postponed and no Trainer defeat,
badge, reward, Gym completion, or one-time story state is written. This also
makes an invalid over-cap save fail safely until its required League is cleared.

The challenge that awards the badge at the cap proceeds normally. Receiving
that badge triggers the qualified state and next-League presentation after the
badge award has committed.

Regional travel, ordinary Trainer battles, wild encounters, healing, shopping,
and story interactions do not perform this certification check. A regional
story may retain its own prerequisites, but it cannot treat the global cap as a
reason to block travel or unrelated content.

### League eligibility and order

League challenges are enabled only by the following predicates:

| League | Eligibility |
| --- | --- |
| Kanto | At least 8 global badges and Kanto League not cleared |
| Johto | Kanto League cleared, at least 16 global badges, and Johto League not cleared |
| Hoenn | Kanto and Johto Leagues cleared, all 24 global badges, and Hoenn League not cleared |

Badge origin is not part of any predicate. A local badge count, local story
completion, another region's Champion state, or a generic game-clear flag
cannot substitute for or strengthen these requirements.

The applicable League entrance and challenge scripts use these predicates.
Before qualification, they leave the League uncleared and direct the player to
the current badge target. After qualification, no regional story or local
badge check may prevent access to the League challenge.

Before the certification caps are enabled, the travel graph must provide a
usable route from every valid starting region and every possible threshold-
badge location to the assigned League. Completing that League must release the
player at a location connected to the wider regional travel network. The route
may be directional and a direct fast-travel option is not required. Reaching
the League cannot require another badge or the clear currently being pursued.

Completing a League atomically records that region's Champion and game-clear
state, applies only that region's Hall of Fame and cleanup behavior, advances
the global circuit, and preserves every badge and all other regional state.
Out-of-order League completion is unreachable through normal play.

### League and Trainer difficulty

The Kanto League uses its authored Tier 1 parties, the Johto League uses its
authored Tier 2 parties, and the Hoenn League uses its authored Tier 3 parties.
The selected parties do not depend on Trainer Rating, badge distribution,
starting region, current party, or prior losses.

Tier 2 must be authored as a stronger challenge than Tier 1, and Tier 3 as a
stronger challenge than Tier 2. This relative order applies to the complete
League sequence rather than requiring every individual Pokémon level to be
higher than every level in the previous tier.

Ordinary Trainers and Gym Leaders retain their existing authored parties and
never select a party from global badge count, certification tier, League
progress, or Trainer Rating.

### Trainer Rating

Wayfarer Trainer Rating is an integer from zero through eighty. A new Wayfarer
game initializes the saved value to zero. Reads clamp it to that range and
preserve the higher of the saved rating and the rating derived from current
facts.

Let `b` be the global badge count. The badge contribution is:

```text
0 <= b <= 4:   4 * b
5 <= b <= 8:   16 + 6 * (b - 4)
9 <= b <= 24:  40 + (b - 8)
```

League clears add these fixed contributions:

| Clear | Contribution |
| --- | ---: |
| Kanto League | 15 |
| Johto League | 5 |
| Hoenn League | 4 |

The derived result is clamped to eighty. Because certification caps enforce
the League order, the required milestones are:

| Facts | Derived rating |
| --- | ---: |
| No badges or League clears | 0 |
| 4 badges | 16 |
| 8 badges | 40 |
| 8 badges and Kanto clear | 55 |
| 16 badges and Kanto clear | 63 |
| 16 badges and Kanto and Johto clears | 68 |
| 24 badges and Kanto and Johto clears | 76 |
| 24 badges and all three Leagues cleared | 80 |

The value remains an internal high-water mark. It scales ordinary wild
encounters through the existing projection pipeline and does not alter any
Trainer party.

The HNS Chinchou learnsets add `Flash`, `Surf`, and `Whirlpool` at level 5 in
both normal and legacy-moves mode, after any existing level-5 entries. The
later repeat entries remain unchanged. This ensures that the authored level-5
Chinchou fishing sources around Vermilion and Cinnabar still provide the native
Surf user required by Kanto traversal at Rating 0.

### Presentation

Near the start of a new game, the player receives the complete fixed itinerary:
Kanto after eight badges, Johto after sixteen, and Hoenn after twenty-four. The
status highlights Kanto as the current goal. Clearing Kanto updates the active
goal to Johto; clearing Johto updates it to Hoenn.

A repeatable player-facing status reports:

- the global badge count out of twenty-four;
- the current certification cap;
- the next League destination while the circuit is incomplete; and
- whether the player has qualified for that League while one remains.

After the Hoenn League clear, the same status reports `Circuit complete` and
does not show a destination or qualification state.

Regional badge displays continue to identify each badge and its region. The
global status supplements rather than replaces those displays.

### Validation

Deterministic tests must cover:

1. Every distribution of zero through twenty-four badges across the three
   regions and the resulting deduplicated global count.
2. Certification caps of eight, sixteen, and twenty-four before and after the
   applicable League clear.
3. A Gym challenge immediately below and exactly at each cap, including retry
   after the required League clear.
4. Mixed qualification such as four Johto plus four Hoenn badges for Kanto.
5. Rejection of Johto and Hoenn League challenges before their prerequisite
   clears even when the player has enough global badges.
6. Acceptance of each League with the exact threshold and no badges from its
   host region where such a distribution is possible.
7. Preservation of all badges and unrelated regional state after each League
   clear.
8. Trainer Rating derivation at every milestone and clamp boundary, including
   new-game Rating 0 and final Rating 80.
9. High-water behavior after regional cleanup, repeated reads, save and load,
   and a League loss.
10. Static Trainer party selection at several badge distributions, ratings,
    starting regions, and circuit tiers.
11. Access to travel and unrelated story interactions while the badge count is
    at a certification cap.
12. Eighth-badge routes ending in Kanto, Johto, and Hoenn can all reach the
    Kanto League; equivalent sixteenth-badge routes can reach the Johto League;
    and the twenty-fourth-badge route can reach the Hoenn League. Each clear
    releases the player back into the wider regional travel network.
13. Kanto, Johto, and Hoenn new-game choices each initialize a clean three-
    region save at Rating 0, release control in the selected region, and leave
    both other regions reachable through approved travel.
14. At Wayfarer Ratings 0 through 80, the Standard Rod and production encounter
    pipeline keep the named Vermilion and Cinnabar Chinchou sources eligible at
    their required probabilities, and a caught Chinchou knows Surf.
15. The repeatable circuit status shows each pending League and qualification
    state, then shows `Circuit complete` with no next destination after Hoenn.

The ordinary encounter balance audit covers Wayfarer ratings zero through
eighty. Implementation fails acceptance if an approved core-route native
utility source becomes unavailable or loses its required move at any rating.

## References

- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Wayfarer runtime foundation](wayfarer-runtime-foundation.md)
- [Wayfarer Hoenn content port](wayfarer-hoenn-content-port.md)
- [Kanto wild encounters](kanto-wild-encounters.md)
- [Native HM utility learnsets](native-hm-learnsets.md)
- [Standard Rod fishing](standard-rod-fishing.md)
