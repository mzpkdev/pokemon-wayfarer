# Wayfarer trainer runtime

Wayfarer uses the v0 player progression and notable-trainer model together,
controlled by `WAYFARER_V0_TRAINERS` in `include/config/notable_trainers.h`.
The default enables it for Wayfarer. Standalone products retain their existing
progression and battles.

League events select five canonical trainers, persist their resolved teams, and
use the existing league rooms and first-win ceremonies. This slice offers an
explicit acceptance prompt at each lobby: player TR 80 and a badge from that
league's regions qualify for Indigo or Hoenn, and lifetime wins at both qualify
for the Masters. Only one accepted event may wait at a time. Phone invitations
and the in-game day clock remain follow-up work; lobby acceptance does not wait
for a countdown. League wins award no player TR.

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
| `league_selection.c` | League registry, willingness, fatigue, aloof eligibility, Masters seats, and deterministic lineup order. |
| `league_halls.c` | Ordered hall locations, names, conditions, and their battle starting statuses. |
| `league_events.c` | Accepted events, saved teams, content validation, reigning champions, reign records, and Gallery counts. |
| `league_circuit.c` | Room admission, battle progress, event resolution, recovery, and first-win integration. |
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

## Saved league events

Acceptance composes all five teams through `ResolveNotableTrainerSnapshot` and
the production Pokémon constructor, then commits a full save before revealing
the lineup. Saved team records contain values rather than runtime pointers.
Challenge and randomizer choices that determine those values are captured at
acceptance; changing options later does not recompose the accepted team.

Event metadata occupies `SaveBlock3.leagueEvent`; the compact team payload uses
the unused tail of `PokemonStorage`, without reducing box capacity. State and
payload checksums bind them to one event generation. Wayfarer event saves use
complete slots, including save paths that formerly wrote only a few sectors.
Load rejects mixed-generation slots and can recover the older complete slot.

Entry and room victories queue a save at a settled overworld point. A loss or
exit crowns the strongest saved participant; a win crowns the player. Both
release the accepted event and update the most recent resolved lineup exactly
once. Battle completion carries the event identity and match index, so an old
callback cannot advance a new event. First Indigo Hall of Fame rollback restores
the event metadata and saved teams together with its existing ceremony state.

The future clock integration must initialize countdown timestamps from its own
in-game day count; this slice does not read RTC days or schedule calls. The
Masters Gallery counts are persisted; the full Gallery presentation and phone
results remain separate work.

## League halls

Each league's five matches use fixed halls. Their conditions belong to the
location, independent of the selected trainer and saved team. A prepared league
battle replaces trainer-authored starting statuses with its hall's statuses;
the three neutral Champion's Rooms clear those statuses. Other battle contexts
keep the engine's normal starting-status sources.

The engine applies temporary rooms and terrains for five turns, Tailwind and
Sea of Fire to both sides for four turns, and Sticky Web or Stealth Rock to
both sides until cleared. Starting hazards affect the opening Pokémon too.
Lorelei's and Indigo Bruno's maps carry snow and sandstorm respectively, using
normal overworld weather and battle-weather handling. Those shared maps also
have weather in standalone FRLG. Optional lobby boards describe the halls and
their conditions before entry.
