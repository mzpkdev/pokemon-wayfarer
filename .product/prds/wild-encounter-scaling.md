# Wild encounter scaling

## Intent

Make open-world exploration practical from different starting points while
preserving natural differences in danger. Wild encounters should stay relevant
as the player progresses without erasing each area's identity. Returning to an
earlier area should offer useful catches and new discoveries rather than only
trivial encounters. Early exploration may reveal species beyond the player's
current capabilities, creating reasons to prepare and return without making
those species reliable shortcuts to immediate power.

## Design

Wild encounter scaling uses Trainer Rating, a permanent measure of adventure
progress, rather than the player's current party. Each encounter's potential
level comes from Trainer Rating and the area's level curve.

Species remain present in their authored habitats throughout progression. When
an encounter's potential level is below the original species' intended range,
that species appears at a reduced frequency. If it appears, it retains its
authored natural level instead of scaling down to the potential level and
receives an encounter-scoped catch-rate penalty. The frequency reduction and
catch-rate penalty fade as the potential level approaches the intended range
and disappear once it reaches that range. These adjustments do not change the
species' global catch data.

Each selectable starting location has an authored starting-zone marker applied
to a list of existing maps. The marker exists only to apply an extra early-game
level reduction. It does not alter map layout or connections. Starting zones
are not inferred from map connections and may overlap or be shared by multiple
starting locations.

Only the chosen location's zone becomes active. Its wild encounters receive an
additional early-game level reduction that fades with progress; other potential
starting zones do not. Early frequency and catch-rate adjustments use the
resulting potential level, including this reduction.

## Balance

Across the world, scaling compresses level differences early, restores more
identity to dangerous areas as the player becomes established, then lets weaker
areas catch up later.

Early compression keeps routine travel from becoming repeatedly punishing, but
it does not remove every encounter beyond the player's current capabilities.
Rare species above the current potential range retain their danger and remain
possible to catch. Reduced frequency and temporary catch resistance prevent
them from becoming reliable sources of early power. An unusually lucky or
well-prepared early capture is an intentional open-world outcome.

Catch resistance must leave preparation meaningful. Weakening the encounter,
using status conditions, and bringing better capture tools should improve the
player's chances substantially. Reduced frequency and catch resistance should
not combine into an opportunity that feels effectively impossible or rewards
repetitive encounter grinding.

Late catch-up keeps earlier areas useful by letting returning players catch new
team members closer to their current strength instead of grinding them from very
low levels. It also reduces trivial encounters during routine travel.

The boundary of the active starting zone may create a discrete level step. The
global early-game compression limits the danger outside that boundary, so the
design does not require additional smoothing between marked and unmarked maps.

## Playtesting

Treat the narrow-wide-narrow pattern of level differences as the baseline until
representative areas and populations can be tested. Compare simpler curves using
early travel pressure, the frequency and success rate of above-range encounters,
whether early failures create reasons to return, the value of revisiting earlier
areas, the effort required to train new team members, and the frequency of
trivial encounters or repetitive encounter grinding.

## Boundaries

This feature covers ordinary wild encounters only. Ordinary trainers, Gym
Leaders, and all other trainer battles are outside its scope. It does not alter
maps or make every location a suitable starting point.

Exact Trainer Rating inputs, potential-level formulas, curve values,
starting-zone membership, intended species ranges, frequency and catch-rate
adjustments, exceptions, and encounter edge cases belong in linked specs.
