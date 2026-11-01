# In-game clock

PRD: [In-game clock](../prds/in-game-clock.md)
Implemented: Yes. The clock, what play time counts, the new game, a new day
and the removed ways of setting time shipped in
[PR #170](https://github.com/mzpkdev/pokemon-wayfarer/pull/170) with a
90-minute day (16×), a 10:00 start and the engine's hours; the 2-hour day
(12×), the 8:00 start and [Wayfarer's times of day](#times-of-day) replaced
those afterwards.

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
  Bug-Catching Contest, radio shows) follow it: `GetDayOfWeek`, `GetDay`,
  `GetMonth` and `GetFullYear` read the clock's own date in Wayfarer, so day 1
  is a Saturday for every reader.
- **Local time is the clock.** `gSaveBlock2Ptr->localTimeOffset` is always
  zero in Wayfarer, so `gLocalTime` (the time every reader uses, from
  `RtcCalcLocalTime`) equals the saved clock, and the **day number** is the
  same everywhere: `gLocalTime.days`, `RtcGetLocalDayCount()` and, once the
  daily update has run, `VAR_DAYS`. Day 1 is 1 January 2000.

### Rate

The clock moves **12 in-game seconds for every second of play**
(`WAYFARER_CLOCK_SECONDS_RATIO`, returned by `FakeRtc_GetSecondsRatio`). It
advances once per second of play, in one 12-second step, when the play-time
counter's frame count reaches 60. Five steps make an in-game minute. The
static assert in [fake_rtc.c](../../game/src/fake_rtc.c) checks that a day
lasts 120 minutes of play.

| In-game | Play time |
| --- | --- |
| 1 minute | 5 seconds |
| 1 hour | 5 minutes |
| 1 day | 2 hours |
| 1 week | 14 hours |

The overworld refreshes the time of day (lighting, time-of-day forms) every
`3600 / 12 = 300` frames, once per in-game minute. That interval and the
walkers' busy-frame check are computed from the ratio, so nothing else
changes with it.

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
  the main menu, the intro, and the origin, appearance and challenge pickers of
  a new game; nor while the game is off. (The challenge menu opened later from
  a PC runs with the counter on, like any other menu.)
- Nothing pauses it. The engine's pause flag (`OW_FLAG_PAUSE_TIME`) is 0, so
  `Script_PauseFakeRtc` and its siblings have no effect; Wayfarer adds no pause.
- The counter's display stops at 999:59 and rolls over as before; the clock
  does not depend on it and keeps running.

Play since the last save is lost when the game is turned off, like everything
else since that save; the clock is what the save holds, and Continue resumes
from it.

### New game

A new game sets the clock to **day 1 at 8:00:00**, the start of the day
period (`InGameClock_StartNewGame` in [clock.c](../../game/src/clock.c), from
`NewGameInitData`, with `WAYFARER_CLOCK_START_HOUR` 8): it resets the clock
and advances it 8 hours, zeroes the local offset, then runs
`InitTimeBasedEvents`, which sets `FLAG_SYS_CLOCK_SET`, stamps the berry
update and sets `VAR_DAYS` to 1. Whatever an earlier file left in the clock or
the offset is overwritten. The first map load stamps day 1 for the daily
world slots.

### Times of day

Wayfarer sets its own hours. In
[rtc.h](../../game/include/constants/rtc.h) an `IS_WAYFARER` block defines the
eight `*_HOUR_BEGIN` and `*_HOUR_END` macros ahead of the engine's
`OW_TIMES_OF_DAY` chain, which keeps serving the other builds.
`OW_TIMES_OF_DAY` stays `GEN_LATEST`, so `GenConfigTimeOfDay` folds nothing and
Morning Sun keeps its four-period rule.

| Time of day | Hours | Macros (begin, end) | Play time per day |
| --- | --- | --- | --- |
| Morning | 4:00–8:00 | 4, 8 | 20 minutes |
| Day | 8:00–20:00 | 8, 20 | 1 hour |
| Evening | 20:00–24:00 | 20, **24** | 20 minutes |
| Night | 0:00–4:00 | 0, 4 | 20 minutes |

`EVENING_HOUR_END` is 24, not 0: the lighting blend divides by the period's
length, which would be negative with 0. Every hour belongs to exactly one
time of day, and `GetTimeOfDay` reports it from these macros alone.

A new game starts at the start of the day period, with an hour of daylight
ahead.

#### By day and by night

Systems that only need two halves use:

- **by day**: `TIME_DAY`, 8:00–20:00;
- **by night**: `TIME_EVENING`, `TIME_NIGHT` or `TIME_MORNING`, 20:00–8:00.

Each lasts an hour of play. Nothing reads the halves yet; the S.S. ferries
will be the first.

#### Lighting

The overworld tint (`UpdateTimeOfDay` in
[overworld.c](../../game/src/overworld.c)) keeps its rules and follows the new
macros:

| Hours | Tint |
| --- | --- |
| 4:00–6:00 | blends from night to morning |
| 6:00–8:00 | blends from morning to day |
| 8:00–20:00 | day, no tint |
| 20:00–24:00 | blends from day to evening |
| 0:00–1:00 | blends from evening to night |
| 1:00–4:00 | night |

Morning and evening share one tint. Battle backgrounds show twilight in the
morning, the evening and the first night hour (0:00–1:00), and night after it.

#### Wild Pokémon by time of day

Wild-encounter tables have a day and a night table
([wild level scaling](wild-level-scaling.md)): morning and day use the day
table, evening and night use the night table, so the night table covers
20:00–4:00 (40 minutes of play) and the day table 4:00–20:00 (80 minutes).
The HnS visible overworld Pokémon and `VAR_TIME_BASED_ENCOUNTER` follow the
same rule in Wayfarer, so what the player sees matches what they meet:

- `UpdateTimeOfDay` shows the night overworld Pokémon (`FLAG_DAY_POKEMON`
  set, `FLAG_NIGHT_POKEMON` clear) in the evening and at night, and the day
  ones in the morning and by day. Before this, evening showed the day ones.
- `SetTimeBasedEncounters` sets night (2) in the evening and at night, and
  day (1) otherwise.

The Pokédex area screen already folds evening into night.

#### Everything else keeps its period rules

Readers of `GetTimeOfDay` keep their own rules and only see new hours:
evolutions and forms by time of day (forms keep counting morning and evening
as day), Moonlight, Morning Sun, Synthesis and the Dusk Ball, fog, light
objects, follower chatter, Oak's radio show, and the Alola map scripts that
read `gettimeofday`.

### A new day

A day starts when the clock passes midnight. Each daily system notices it at
its own moment:

- **The daily update** (`DoTimeBasedEvents` in
  [clock.c](../../game/src/clock.c)) compares `gLocalTime.days` with
  `VAR_DAYS`. When the day is later, it clears the daily flags (both the
  shared bank and Hoenn's), advances the Dewford trend, TV shows, weather,
  Pokérus, Mirage Island, Birch, the Frontier maniac and gambler, the Shoal
  Cave items, the lottery number and the days since a form change by the
  number of days passed, refills the apricorn trees, then sets `VAR_DAYS`. It also
  grows berry trees by the in-game minutes passed. It runs on every map load
  (warps and camera transitions), on Continue, and on the field every 512
  frames (`TIME_UPDATE_INTERVAL` in
  [field_tasks.c](../../game/src/field_tasks.c), about 1.7 in-game minutes; the
  other builds keep 8192). So a player standing on a map sees the new day's
  daily flags within about 9 seconds of play.
- **In the Centers `InPokemonCenter` lists** the daily update does not run, as
  before: Emerald's Hoenn Pokémon Centers, the Ever Grande League lobby, the
  Battle Frontier Centers and the link rooms. It runs on the first load or
  field check after the player leaves and counts every day passed in between.
  The other regions' Centers (Johto, Kanto, Sevii, Sinnoh) are ordinary maps
  for it: the field check runs there too. Either way no day is lost.
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
- The household wall clocks only show the time. The native openings still
  stop at the bedroom clock (Brendan's and May's 2F in Littleroot, the player's
  2F in New Bark), but the scene says the clock is ticking and `StartWallClock`
  opens the clock face to look at, not to set, before the scene goes on. The
  "press R to set the clock" line is gone, and R does nothing on the clock face
  (`Task_ViewClock_HandleInput`). The wall clocks on HnS maps (the Johto and
  Kanto Pokémon Centers and a few houses) share Littleroot's clock script and
  read the HnS `FLAG_SET_WALL_CLOCK`; only the Johto origin plays New Bark's scene, so the
  Littleroot and Pallet origins start with that flag set and those clocks
  only show the time.
- No Wayfarer script uses the time-changing script commands (`initclock`,
  `addtime`, `adddays`, `addhours`, `addminutes`, `fwdtime`, `fwdweekday`).
- The overworld debug menu (R + Start, compiled into every build, as before
  this spec) keeps its time tools for development: setting the time of day or
  the weekday moves the clock forward to the next such time. With the clock
  always on they now work in every build. Its Cheat Start doesn't touch the
  clock in Wayfarer.

The day number and `VAR_DAYS` therefore only increase. The daily update and
the league countdown keep their guards against a day that goes backwards.
`gLocalTime.days` is an `s16`, so the day number stays valid for 32,767 days,
about 7.5 years of continuous play.

### Other readers

Every other system reads time through `gLocalTime`, `GetTimeOfDay` or
`GetDayOfWeek`, all from the same clock, so they speed up with it:

| System | Reads | At 12× |
| --- | --- | --- |
| Wild tables, battle backgrounds, fishing, DexNav, evolutions and forms by time of day | time of day | as in the table above |
| Lighting and tint | time of day, refreshed every in-game minute | as in the table above |
| Start menu clock | `gLocalTime`, recalculated whenever the window is drawn | the minutes move while the menu is open |
| Map name popup clock | `gLocalTime`, recalculated | |
| Phone random calls | in-game minutes since the last chance (`UpdateMatchCallMinutesCounter`) | a chance every 180 in-game minutes, about 15 minutes of play (the first comes right after New Game or Continue) |
| Shoal Cave tides | hour of day, from a fixed 24-hour table | each 6-hour tide lasts 30 minutes of play |
| Bug-Catching Contest | weekday | held Tuesday, Thursday and Saturday, each 2 hours of play |
| PokéNews and Game Corner roulette | raw hours (PokéNews' 20:00 cutoff, roulette's hour checks) | the same hours, at 12× |
| Berry trees | in-game minutes | grow the same number of in-game days as before |

Phone rematch readiness counts steps, not time
([phone rematches](phone-rematches.md)).

### Testing hooks

- The E2E day pin (`gDailySlotsDebugDay`, `game.dailySlots.pin`) still
  overrides only the daily slots' day, as before.
- E2E builds export `gE2EInGameClockOffset`, the offset of the clock in
  SaveBlock3, so a journey can read the clock and set the time of day
  (`game.clock`).
- Tests that pin the new values: the in-game clock mechanics test (12×, a
  2-hour day, the 8:00 start, the time of day at each period boundary,
  including 20:00 as evening and 0:00 as night, and a midnight crossing that
  lands on 00:00:02), the wild-encounter time-of-day test (the day table at
  7:00 and 12:00, the night table at 22:00 and 2:00), and the clock and native
  opening journeys, which expect the clock at or after 8:00.

## References

- [Daily world slots](daily-world-slots.md#the-day)
- [Phone rematches](phone-rematches.md)
- [Leagues](leagues.md#invitations)
- [Notable world simulation](notable-world-simulation.md)
