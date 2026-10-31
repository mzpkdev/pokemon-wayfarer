# In-game clock

PRD: [In-game clock](../prds/in-game-clock.md)
Implemented: No

## Scope

This spec makes the [in-game clock](../prds/in-game-clock.md) precise: where
the clock lives, how fast it runs, what play time it counts, how a new game
sets it, how a day starts for each daily system, and which ways of setting
time Wayfarer removes. It covers the Wayfarer build only.

Each daily system keeps its own rules for what a new day does; this spec only
says which clock and which day they read. [Daily world slots](daily-world-slots.md#the-day),
[phone rematches](phone-rematches.md) and [Leagues](leagues.md#invitations)
own their day handling. The [notable world simulation](notable-world-simulation.md)
does not run on the clock yet.

## Behavior

### The clock

- The clock is the engine's fake RTC: a date and time saved in
  `gSaveBlock3Ptr->fakeRTC` ([fake_rtc.c](../../game/src/fake_rtc.c)). Wayfarer
  always uses it: `OW_USE_FAKE_RTC` is `IS_WAYFARER`, and `UseFakeRtc()` reads
  only that setting. Nothing in Wayfarer reads the cartridge's real-time clock:
  `RtcInit` returns early, `RtcGetInfo` returns the saved clock, and
  `RtcGetErrorStatus` is always 0, so no clock error screen can show.
- The date counts from the engine's epoch: year 0 is 2000, 1 January 2000 is a
  Saturday, and months and leap years follow the calendar. Weekdays (the
  Bug-Catching Contest, radio shows) follow it.
- **Local time is the clock.** `gSaveBlock2Ptr->localTimeOffset` is always
  zero in Wayfarer, so `gLocalTime` (the time every reader uses, from
  `RtcCalcLocalTime`) equals the saved clock, and the **day number** is the
  same everywhere: `gLocalTime.days`, `RtcGetLocalDayCount()` and, once the
  daily update has run, `VAR_DAYS`. Day 1 is 1 January 2000.

### Rate

The clock moves **16 in-game seconds for every second of play**
(`WAYFARER_CLOCK_SECONDS_RATIO`, returned by `FakeRtc_GetSecondsRatio`). It
advances once per second of play, in one 16-second step, when the play-time
counter's frame count reaches 60.

| In-game | Play time |
| --- | --- |
| 1 minute | 3.75 seconds |
| 1 hour | 3 minutes 45 seconds |
| 1 day | 90 minutes |
| 1 week | 10 hours 30 minutes |

The overworld refreshes the time of day (lighting, time-of-day forms) every
`3600 / 16 = 225` frames, once per in-game minute.

### What play time counts

The clock runs exactly when the play-time counter on the trainer card and save
screen runs ([play_time.c](../../game/src/play_time.c)). The counter is
updated every frame from the main loop, whatever screen is showing, while it
is running:

- It **starts** when a new game reaches the field (`CB2_NewGame`), on Continue
  (`CB2_ContinueSavedGame`), and when a league run's saved game is recovered.
- It **never stops** after that until the game is reset or turned off: it runs
  on the field, in battles, menus, the bag, the party and summary screens, the
  PC, the Pokédex, the Pokégear, shops, cutscenes and scripted scenes, while
  saving, and in link rooms.
- It does **not** run before a game is started or continued: the title screen,
  the main menu, the intro, the origin and appearance pickers and the challenge
  menu, and while the game is off.
- Nothing pauses it. The engine's pause flag (`OW_FLAG_PAUSE_TIME`) is 0, so
  `Script_PauseFakeRtc` and its siblings have no effect; Wayfarer adds no pause.
- The counter's display stops at 999:59 and rolls over as before; the clock
  does not depend on it and keeps running.

Play since the last save is lost when the game is turned off, like everything
else since that save; the clock is what the save holds, and Continue resumes
from it.

### New game

A new game sets the clock to **day 1 at 10:00:00** (`InGameClock_StartNewGame`
in [clock.c](../../game/src/clock.c), from `NewGameInitData`): it resets the
clock and advances it 10 hours, zeroes the local offset, then runs
`InitTimeBasedEvents`, which sets `FLAG_SYS_CLOCK_SET`, stamps the berry
update and sets `VAR_DAYS` to 1. Whatever an earlier file left in the clock or
the offset is overwritten. The first map load stamps day 1 for the daily
world slots.

### Times of day

The hours are the engine's current ones (`OW_TIMES_OF_DAY` is `GEN_LATEST`):

| Time of day | Hours | Play time per day |
| --- | --- | --- |
| Morning | 6:00–10:00 | 15 minutes |
| Day | 10:00–18:00 | 30 minutes |
| Evening | 18:00–19:00 | 3 minutes 45 seconds |
| Night | 19:00–6:00 | 41 minutes 15 seconds |

A new game starts at the start of the day period, with 30 minutes of daylight
ahead.

### A new day

A day starts when the clock passes midnight. Each daily system notices it at
its own moment:

- **The daily update** (`DoTimeBasedEvents` in
  [clock.c](../../game/src/clock.c)) compares `gLocalTime.days` with
  `VAR_DAYS`. When the day is later, it clears the daily flags (both the
  shared bank and Hoenn's), advances the Dewford trend, TV shows, weather,
  Pokérus, Mirage Island, Birch, the Frontier maniac and gambler, the Shoal
  Cave items, the lottery number, the days since a form change and the
  apricorn trees by the number of days passed, then sets `VAR_DAYS`. It also
  grows berry trees by the in-game minutes passed. It runs on every map load
  (warps and camera transitions), on Continue, and on the field every 512
  frames (`TIME_UPDATE_INTERVAL` in
  [field_tasks.c](../../game/src/field_tasks.c), about 2 in-game minutes; the
  other builds keep 8192). So a player standing on a map sees the new day's
  daily flags within about 9 seconds of play.
- **In a Pokémon Center** (and the link rooms, `InPokemonCenter`) the daily
  update does not run, as before. It runs on the first load or field check
  after the player leaves and counts every day passed in between.
- **During a battle, menu or scene** the clock keeps running but the daily
  update waits for the field: it runs on the next map load or field check.
- **Daily world slots** take a new day on the next map load
  ([daily world slots](daily-world-slots.md#the-day)), from
  `RtcGetLocalDayCount()`, read fresh at the load. The Pokémon Center rule
  doesn't apply to them. The phone's day-start ready and gift sets are taken
  then ([phone rematches](phone-rematches.md)).
- **Leagues** count days from `VAR_DAYS` through their forward-only countdown
  ([Leagues](leagues.md#countdown)).

### Days only go forward

Nothing a player can reach sets or moves the clock back:

- The challenge menu's clock-type option is removed, with its saved bit
  (`tx_Features_RTCType`).
- The reset-RTC screen has no entry point: the title screen's
  B + Select + Left chord is gone from Wayfarer.
- The player's-house wall clock (Brendan's and May's 2F, Littleroot) only
  shows the time. Its first-visit "the clock is stopped" scene and the
  "press R to set the clock" line don't run, and R does nothing on the clock
  face (`Task_ViewClock_HandleInput`).
- No Wayfarer script uses the time-changing script commands (`initclock`,
  `addtime`, `adddays`, `addhours`, `addminutes`, `fwdtime`, `fwdweekday`).
- The debug menu keeps its time tools for development; they only move the
  clock forward.

The day number and `VAR_DAYS` therefore only increase. The daily update and
the league countdown keep their guards against a day that goes backwards.
`gLocalTime.days` is an `s16`, so the day number stays valid for 32,767 days,
about 5.6 years of continuous play.

### Other readers

Every other system reads time through `gLocalTime`, `GetTimeOfDay` or
`GetDayOfWeek`, all from the same clock, so they speed up with it:

| System | Reads | At 16× |
| --- | --- | --- |
| Wild tables, battle backgrounds, fishing, DexNav, evolutions and forms by time of day | time of day | as in the table above |
| Lighting and tint | time of day, refreshed every in-game minute | as in the table above |
| Start menu clock | `gLocalTime`, recalculated whenever the window is drawn | the minutes move while the menu is open |
| Map name popup clock | `gLocalTime`, recalculated | |
| Phone random calls | in-game minutes since the last chance (`UpdateMatchCallMinutesCounter`) | a chance every 180 in-game minutes, about 11 minutes of play |
| Shoal Cave tides | hour of day | each 6-hour tide lasts 22.5 minutes of play |
| Bug-Catching Contest | weekday | held Tuesday, Thursday and Saturday, each 90 minutes of play |
| Berry trees | in-game minutes | grow the same number of in-game days as before |

Phone rematch readiness counts steps, not time
([phone rematches](phone-rematches.md)).

### Testing hooks

- The E2E day pin (`gDailySlotsDebugDay`, `game.dailySlots.pin`) still
  overrides only the daily slots' day, as before.
- E2E builds export `gE2EInGameClockOffset`, the offset of the clock in
  SaveBlock3, so a journey can read the clock and set the time of day
  (`game.clock`).

## References

- [Daily world slots](daily-world-slots.md#the-day)
- [Phone rematches](phone-rematches.md)
- [Leagues](leagues.md#invitations)
- [Notable world simulation](notable-world-simulation.md)
