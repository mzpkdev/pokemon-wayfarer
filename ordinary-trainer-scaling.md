# Ordinary trainer scaling

## Intent

Keep ordinary trainer battles fair when the player explores out of order and
relevant when returning later, without authoring a separate party for every
stage of progression.

## Design

Ordinary trainer parties scale automatically with the same permanent adventure
progress as wild encounters. Each trainer's authored party remains the source
of their identity and relative difficulty.

Ordinary trainers share the active authored starting zone with wild encounter
scaling. Trainers within it receive an additional early-game level reduction
that fades with progress; trainers in other potential starting zones do not.

## Boundaries

This feature covers ordinary trainer battles. Rivals, Gym Leaders, major bosses,
and other deliberate challenges remain authored outside automatic scaling.

Scaling neither creates rematches nor changes whether a trainer has been
defeated. Exact classification, progression rules, values, evolution and move
behavior, rewards, and exceptional cases belong in linked specs.

## References

- [Wild encounter scaling](wild-encounter-scaling.md)
