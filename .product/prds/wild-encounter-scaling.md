# Wild encounter scaling

## Intent

Make open-world exploration practical from different starting points while
preserving natural differences in danger. Wild encounters should stay relevant
as the player progresses without erasing each area's identity. Returning to an
earlier area should offer useful catches and new discoveries rather than only
trivial encounters.

## Design

Wild encounter levels and species availability change automatically with
permanent adventure progress, independent of the player's current party.

When scaling produces encounters below a species' intended level range, that
species is replaced by a habitat-appropriate common Pokémon. As the area's
scaled levels rise, the original species enters its encounter population. Each
area retains a core population throughout progression so its identity remains
recognizable.

Each selectable starting location has an authored starting zone. A starting
zone is a group of nearby maps intended for early play, including surrounding
routes, caves, and dungeons. Zones are not inferred from map connections and
may overlap or be shared by multiple starting locations.

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

## Playtesting

Treat the narrow-wide-narrow pattern of level differences as the baseline until
representative areas and populations can be tested. Compare simpler curves using
early travel pressure, access to strong captures, the value of revisiting earlier
areas, the effort required to train new team members, and the frequency of
trivial encounters.

## Boundaries

This feature covers ordinary wild encounter progression. It does not scale
trainer battles or make every location a suitable starting point.

Exact progression inputs, level curves, values, starting-zone membership,
population tiers, replacement pools, exceptions, and encounter edge cases
belong in linked specs.
