# Wild encounter scaling

## Intent

Make open-world exploration practical from different starting points while
preserving natural differences in danger. Wild encounters should stay relevant
as the player progresses without erasing each area's identity. Returning to an
earlier area should offer useful catches and new discoveries rather than only
trivial encounters.

## Design

Wild encounter scaling uses Trainer Rating, a permanent measure of adventure
progress, rather than the player's current party. Each encounter's potential
level comes from Trainer Rating and the area's level curve.

A manually authored list maps original species to lower-level replacements.
When an encounter's potential level is below the original species' intended
range, the mapped replacement appears. Once the potential level reaches that
range, the original species replaces it completely. Species absent from the
list remain part of the area's core population throughout progression.

Each selectable starting location has an authored starting-zone marker applied
to a list of existing maps. The marker exists only to apply an extra early-game
level reduction. It does not alter map layout or connections. Starting zones
are not inferred from map connections and may overlap or be shared by multiple
starting locations.

Only the chosen location's zone becomes active. Its wild encounters receive an
additional early-game level reduction that fades with progress; other potential
starting zones do not. Population replacement follows the resulting scaled
level, including this reduction.

## Balance

Across the world, scaling compresses level differences early, restores more
identity to dangerous areas as the player becomes established, then lets weaker
areas catch up later.

Early compression prevents dangerous areas from immediately offering
disproportionately high-level captures or making travel repeatedly punishing.
Late catch-up keeps earlier areas useful by letting returning players catch new
team members closer to their current strength instead of grinding them from very
low levels. It also reduces trivial encounters during routine travel.

The boundary of the active starting zone may create a discrete level step. The
global early-game compression limits the danger outside that boundary, so the
design does not require additional smoothing between marked and unmarked maps.

## Playtesting

Treat the narrow-wide-narrow pattern of level differences as the baseline until
representative areas and populations can be tested. Compare simpler curves using
early travel pressure, access to strong captures, the value of revisiting earlier
areas, the effort required to train new team members, and the frequency of
trivial encounters.

## Boundaries

This feature covers ordinary wild encounters only. Ordinary trainers, Gym
Leaders, and all other trainer battles are outside its scope. It does not alter
maps or make every location a suitable starting point.

Exact Trainer Rating inputs, potential-level formulas, curve values,
starting-zone membership, replacement lists, exceptions, and encounter edge
cases belong in linked specs.
