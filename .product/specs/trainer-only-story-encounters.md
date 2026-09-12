# Trainer-only story encounters

PRD: [Trainer-only encounters](../prds/trainer-only-encounters.md)
Status: no story-scene integration.

Trainer-only is only the explicit, map-local wild-encounter mechanic described in
[the core specification](trainer-only-encounters.md). This project has no
ordinary-trainer caller registry, refusal dialogue, rival deferral, scene
continuation, rollback table, recovery policy, or story-specific empty-party
handling.

A human-authored safe scenario may set `FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS` only
after it is on the scenario map, with an exactly empty party, and must clear it
before its own exit or any unsafe interaction. Map loading clears the temporary
flag as a backstop. The scenario must not expose trainers, scripted/special
battles, scene triggers, Pokémon Center/healing, or other unsupported routes
while the flag is set and the party is empty.

If an author permits one of those routes, the game follows its native behavior;
there is no conversion to trainer-only, no refusal, deferral, restoration, warp,
or recovery fallback. A nonempty party—including a fainted or Eggs-only party—is
never trainer-only and follows native behavior as well.
