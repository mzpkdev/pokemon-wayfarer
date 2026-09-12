# Trainer-only wild encounters

PRD: [Trainer-only encounters](../prds/trainer-only-encounters.md)
Status: implemented, author-owned safe-scenario mechanic.

## Activation

The custom trainer-only wild-encounter controller starts only when both of these
conditions are true at wild-battle admission:

1. An authored script has set `FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS`.
2. `CalculatePlayerPartyCount()` is exactly zero.

The flag aliases unused map-local `FLAG_TEMP_B` and is cleared by normal map-load
temporary-flag cleanup. A scenario sets it only after entering its safe map and
clears it before any local exit or unsafe interaction. No production map enables
the flag. The condition is literal party count: a nonempty party, including an
all-fainted, Eggs-only, or mixed unusable party, always keeps native behavior.

The mode is not a global empty-party policy. With the flag clear, an empty party
also follows native behavior. Native storage still prohibits depositing the last
party Pokémon.

## Supported mechanic

An enabled empty-party ordinary wild encounter uses the existing dedicated
controller. It preserves its Ball, Bag (Berry), Rock, Go Near, and Run actions,
proximity/flee flow, capture handling, and encounter-local state. Existing wild
sources that initialize this controller retain their current behavior.

No party recovery, replacement, special blackout, warp, or loss policy is part of
this mechanic. An ordinary party loss uses the game’s native blackout/recovery.

## Authoring boundary

Authors own the complete safe scenario. They must ensure that its enabled,
empty-party window cannot reach trainer battles, scripted battles, special battle
formats, scene transitions, or other unsupported flows. Those flows have no
trainer-only conversion, interception, refusal dialogue, rollback, or fallback;
they use native behavior.

Pokémon Center/healing interactions are unsafe while the flag is set with an empty
party and must not be exposed by a safe scenario. Clear the flag before leaving
the authored scope; a map transfer clears it as a backstop, not as scenario logic.

## Validation contract

Focused tests cover the two-condition gate, map-load clearing, and rejection of
nonempty/fainted/Egg parties. End-to-end coverage exercises explicit-on entry,
explicit-off native routing, and the retained controller journey.
