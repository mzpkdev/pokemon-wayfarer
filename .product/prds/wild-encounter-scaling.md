# Wild encounter scaling

## Intent

Make open-world exploration practical from different starting points while
preserving natural differences in danger. Wild encounters should stay relevant
as the player progresses without erasing each area's identity.

## Design

Wild encounter levels scale automatically with permanent adventure progress,
independent of the player's current party. Scaling uses existing encounter
populations, so ordinary areas need neither progression-specific populations
nor individual tuning.

Each selectable starting location has an authored starting zone. A starting
zone is a group of nearby maps intended for early play, including surrounding
routes, caves, and dungeons. Zones are not inferred from map connections and
may overlap or be shared by multiple starting locations.

Only the chosen location's zone becomes active. Its wild encounters receive an
additional early-game level reduction that fades with progress; other potential
starting zones do not.

Across the world, scaling compresses level differences early, restores more
identity to dangerous areas as the player becomes established, then lets weaker
areas catch up later.

## Boundaries

This feature covers ordinary wild encounter progression. It does not scale
trainer battles or make every location a suitable starting point.

Exact progression inputs, level curves, values, starting-zone membership,
exceptions, and encounter edge cases belong in linked specs.
