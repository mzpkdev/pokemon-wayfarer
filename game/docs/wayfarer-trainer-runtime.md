# Wayfarer trainer runtime

Wayfarer uses the v0 player progression and notable-trainer model together,
controlled by `WAYFARER_V0_TRAINERS` in `include/config/notable_trainers.h`.
The default enables it for Wayfarer. Standalone products retain their existing
progression and battles.

The fixed League circuit still owns admission, room order, wins, and ceremonies.
Its opponents now use their canonical notable teams at the world progress saved
on entry. Invitations, dynamic lineups, and persisted event teams are separate
follow-up work. League wins no longer award player TR.

Wild encounters use the v0 curve and shared evolution policy. Their encounter
content must preserve the existing regional HM acquisition requirements.

## Runtime ownership

| Code | Owns |
| --- | --- |
| `trainer_scaler.c` | Interpolated and step curves, half-up rounding, and flat ceilings. |
| `trainer_rating.c` | Saved player TR and the level cap. Badges supply the progression value; consumers keep their own curves. |
| `notable_trainers.c` | Encounter-to-character mapping, growth, roster selection, and team resolution. |
| `notable_moves.c` | Shared evolution step-down and per-trainer move pools, using game learnsets. |
| `notable_ai.c` | Play styles, skill tiers, ace protection, and boss flags. |
| `battle_main.c` | Eligible opponent preparation, battle snapshot lifetime, and construction through the existing Pokémon constructor. |
| `battle_ai_main.c` | Applying frozen notable flags after the engine's normal AI setup, including prediction slots. |

`CreateNPCTrainerPartyForOpponent` resolves the encounter before selecting its
battle policy. A notable encounter prepares its whole team before construction.
Snapshot members retain roster-slot identity; `battleOrder[]` maps output positions
to those slots. Items, moves, abilities, and personality inputs must follow the
member when the party is reordered. Raw player, partner, facility, link, and
recorded-party entry points do not enroll notable trainers.

Player TR is stored as a 32-bit high-water value in shared Wayfarer save state.
It is independent of notable TR. Changing region preserves it. The save schema
was bumped for the new layout; older prerelease saves are unsupported.

## Content and validation

`tools/notable_trainers/catalog.json` owns the 38 characters, explicit encounter
aliases, growth values, rosters, styles, and move pools. Its generator emits the
game's C tables and stable character constants. The browser balance explorer is
a design reference, not a build dependency.
`slug` names a catalog entry for generation; `characterId` is its stable identity,
and `encounterIds` lists the engine Trainer IDs mapped to it.

`tools/notable_trainers/evolution.json` owns nonlevel evolution thresholds and
explicit ancestry choices. Numeric evolution thresholds come from game data.
Growth, move pools, and additional nonlevel thresholds remain tuning content;
passing structural checks does not establish combat balance.

Run `make notable-trainers-audit` to check both catalogs and generated files.
The ordinary Wayfarer ROM and test builds also run this audit. Native mechanics
tests cover the model, moves, AI, production constructor, and progression;
`make BUILD=wayfarer check TESTS=Notable` selects the notable tests.

`tools/wild_encounters/wayfarer_native_hm_v0_audit.py --check` validates the
v0 HM windows and their generated regional witnesses. Regional witnesses prove
encounter-table presence; the directional scenarios check acquisition from
their approved sources. Native ROM tests verify both against actual movesets.

Future league events should compose teams through `ResolveNotableTrainerSnapshot`
and persist their resolved values before revealing the lineup. Its runtime
pointers are transient and must not be written directly into a save. The event
layer owns selection and lifetime; the battle layer owns construction.
