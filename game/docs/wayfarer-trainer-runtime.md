# Wayfarer trainer runtime

Wayfarer uses the v0 player progression and notable-trainer model together,
controlled by `WAYFARER_V0_TRAINERS` in `include/config/notable_trainers.h`.
The default enables it for Wayfarer. Standalone products retain their existing
progression and battles.

League events select five canonical trainers, persist their resolved teams, and
use the existing league rooms and first-win ceremonies. Indigo and Hoenn invite
the player by phone ([leagues](../../.product/specs/leagues.md#invitations)):
from player TR 80 a countdown of seven in-game days runs on the
[in-game clock](../../.product/specs/in-game-clock.md)'s day counter
(`VAR_DAYS`), and the league that knows the player and called least recently
calls. The player accepts or declines on the call; a decline runs the event
without them. Their lobbies admit only an accepted event and name the reigning
champion. The Sevii Masters stays out of the rotation until its format ships:
lifetime wins at both Indigo and Hoenn let the player accept a Masters event at
the Seven Island house, as before. Only one invitation or accepted event may
wait at a time. League wins award no player TR.

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
| `league_events.c` | Invitations (qualification, the countdown, which league calls, accepting and declining, the owed result call), accepted events, saved teams, content validation, reigning champions, reign records, and Gallery counts. |
| `league_phone.c` | The league calls: the field polls `LeaguePhone_TryStartCall` when it has control, and it rings the owed result call or the waiting invitation once the player has the phone and no league run, save, or ceremony is pending. The front desk's call scripts are in `data/scripts/league_phone.inc`. |
| `league_circuit.c` | Room admission, battle progress, event resolution, recovery, and first-win integration. |
| `notable_friendship.c` | The saved friendship score, first-win bit and number-shown bit of each notable (`SaveBlock3`), the first-win and quest events, the number handover (owed while a trainer at Friend or above has no number-shown bit; the field starts its script whenever it has control, and the script's last special sets the bit), and the load validation. `battle_setup.c` credits wins at the end of a trainer, rematch or league event battle. |
| `notable_phone.c` | Notable trainers as phone contacts: who is one (Friend or above, not Tate & Liza), their list entry's trainer and home, and the call (phone greeting, where they are from the world record or a fixed place, farewell). `pokenav_match_call_list.c` lists them after the special headers; they never random-call. Phone lines are the catalog's `phone` bits in `voices.h`. |
| `notable_talk.c` | The talk flow outside haunts: the greeting by stage and fame, the activity lines, the farewell, and the specials that buffer them. Voice lines are game data generated from `tools/notable_trainers/catalog.json` (`voice`, `buddySlot`) into `src/data/notable_trainers/voices.h`. |
| `wayfarer_daily_trainers.c` | Daily trainer slots: who stands in each rotating slot today, the map-load and Continue hook that gives a slot its occupant's sprite and script, the spawn check for empty slots, the day-start phone snapshots and contact index, `WhereIsTrainerToday`, and the battle notes that clear a slot after a win. Tables come from `tools/wayfarer_trainer_slots`. |
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

The countdown saves its days remaining and the last counted day, and counts only
forward advances of `VAR_DAYS`: a counter that went back moves the last counted
day down, and a jump of many days brings one call. Each check runs where the
field polls for calls, so it covers day advances, player TR changes, and loads;
load validation clamps a last counted day ahead of the counter. Declining saves
the champion, reign flag, recent lineup, and restarted countdown before the call
names the champion, as accepting saves the lineup before revealing it. Each
resolution of an Indigo or Hoenn event owes the league's result call in saved
state (`leagueEvent.resultCall`). The Masters Gallery counts are persisted; the
full Gallery presentation and the Masters' calls remain separate work.

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
