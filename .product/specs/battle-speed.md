# Fast battles, full-speed music

PRD: [Quality of life](../prds/quality-of-life.md)
Implemented: No

## Scope

This spec makes the PRD's [fast battles](../prds/quality-of-life.md#fast-battles-full-speed-music)
precise. It covers the BATTLE SPEED option, what it speeds up, which battles
it applies to, and how it is tested. It covers the Wayfarer build only. The
PRD's other conveniences are not part of it.

## Behavior

### The option

- A new row, **BATTLE SPEED**, on the BATTLE OPTIONS tab of the options menu
  (`src/option_menu.c`). It goes directly after FAST BATTLES.
- Choices: `1×`, `2×`, `3×`, `4×` (the charmap has `×`). The default for a new
  game is `1×`.
- Descriptions:
  - 1×: "Battles run at normal speed."
  - 2×–4×: "Battles run faster. Music and\nsound keep their normal speed."
- Stored as `battleSpeed:2` (0–3 = 1×–4×) in `struct ChallengeSettings`
  (`include/global.h`). It uses the two spare bits after `surfMusic`, so the
  struct stays 32 bytes and `ChallengeSettingsLayoutPinned` (`src/save.c`)
  still holds.
- Wire it in like the other battle rows:
  - save and load in `Task_Save` and `CB2_InitOptionMenu`;
  - default in `SetDefaultChallengeSettings` (`src/new_game.c`);
  - restore in the new-game list in `src/oak_speech_hns.c`, so a change made
    before a new game survives.
- A change applies from the next battle. The options menu cannot be opened
  during a battle.
- **FAST BATTLES** is unchanged. It skips the fixed pauses in `Cmd_pause` and
  `Cmd_waitmessage` (`src/battle_script_commands.c`), and it combines with
  BATTLE SPEED.

### How a faster battle runs

At speed N, each frame of the main battle runs up to N **ticks** instead of 1.

- One tick is one pass of the battle's per-frame work:
  - `BattleMainCB1`: `gBattleMainFunc` and every battler controller;
  - the per-frame part of `BattleMainCB2`: `AnimateSprites`,
    `RunTextPrinters`, `UpdatePaletteFade` and `RunTasks`
    (`src/battle_main.c`).
- `BuildOamBuffer` runs once, after the last tick. `VBlankCB_Battle` and the
  hardware transfers still run once per frame.
- The extra ticks run only while the main callbacks are still `BattleMainCB1`
  and `BattleMainCB2`. When either one changes (the battle ends, or the bag
  or party screen opens), the frame stops ticking.
- **Speed is a ceiling.** Skip an extra tick when it would not finish before
  the next VBlank. Measure this with the scanline counter, using a tunable
  threshold. A heavy frame drops ticks, never whole frames. The speed never
  falls below 1×.

### What speeds up

Everything driven by battle ticks:

- move, status and stat-change animations (`gAnimScriptCallback` and the
  `delay` waits in `src/battle_anim.c`);
- sprite animations: send-out, faint, switch, the intro and trainer slides;
- battle text printing (`BattlePutTextOnWindow` in `src/battle_message.c`,
  printed by `RunTextPrinters`);
- the scripted `pause` and `waitmessage` waits, when FAST BATTLES is off;
- HP and Exp. bar movement (`MoveBattleBar` in `src/battle_interface.c`);
- palette fades inside the battle, including the end-of-battle fade.

### What does not speed up

- **Music.** It runs from the interrupts: `m4aSoundMain` in `VBlankIntr` and
  `m4aSoundVSync` in `VCountIntr` (`src/main.c`). It never depends on battle
  ticks, so tempo and pitch stay normal.
- **Sound effects and cries.** They also play at normal speed and pitch. The
  speed-up never cuts one short:
  - the sound waits in `Cmd_end` and `Cmd_waitsound` (`src/battle_anim.c`, up
    to 90 frames) and in `IsBattleSEPlaying` (`src/battle_gfx_sfx_util.c`, 30
    calls) count frames, not ticks;
  - cry and fanfare waits already wait until the sound ends.
  - So a long cry holds a 4× battle as long as it holds a 1× battle.
- **Play time and the in-game clock.** `PlayTimeCounter_Update` runs once per
  pass of `AgbMainLoop`, not once per tick. A 4× battle uses the same in-game
  time per real second as a 1× battle ([In-game clock](in-game-clock.md)).
- **Input.** New presses (`JOY_NEW`, key repeat) reach the first tick of a
  frame only, and later ticks see no new presses. Held keys stay held. One A
  press is one action at any speed. Holding A or B still speeds up text as
  it does now.
- **The overworld.** The battle transition before a battle, and everything
  after it, runs at normal speed.

### Which battles it applies to

| Battle | Speed |
| --- | --- |
| Wild, trainer, double, partner, Safari, roamer and legendary battles, Trainer Tower, scripted story battles | The option |
| Link battles (`BATTLE_TYPE_LINK`) | Always 1× |
| Recorded battles (`BATTLE_TYPE_RECORDED`, `BATTLE_TYPE_RECORDED_LINK`) | Always 1× |
| Catch tutorials (`BATTLE_TYPE_CATCH_TUTORIAL`: Wally in Petalburg) and `BATTLE_TYPE_POKEDUDE` | Always 1× |
| Mechanics-test build (`TESTING`) | Always 1× |

- **Link battles.** Both consoles exchange battle buffers through the link
  (`src/battle_controllers.c`), and extra ticks on one side would run ahead of
  the other. The engine already overrides player settings in link battles. It
  forces text speed and autoscroll (`src/battle_message.c`), and it ignores
  BATTLE SCENE (`src/battle_main.c`). The option keeps its value, and no
  message is shown.
- **Recorded battles** replay inputs frame by frame, so they stay at 1×.
- **Catch tutorials** are cutscenes the player watches, and their pacing is
  authored. The Wally catch runs at 1×. Wally's real battle later in the game
  uses the option.
- **The mechanics-test build** stays at 1× whatever the option says. The
  battle-speed tests below set the speed explicitly. Headless runs already
  skip animations and pauses (`gTestRunnerHeadless`).
- **The E2E build** (`make e2e`) is a normal playable ROM, and the option works
  there as in a release.

### RAM and save

- No new EWRAM or IWRAM. The setting lives in SaveBlock3's existing bits, and
  the tick counter is a local variable. EWRAM is unchanged in every build. The
  release build is 587 bytes under its ceiling, and the mechanics-test build
  is within 32 bytes of the 256 KB hardware limit.
- `make check` and the release build must still pass
  `game/tools/ram_report/ram_budget.json` with no ceiling raised.
- Prerelease saves need no migration. An old save reads the spare bits as 0,
  which is 1×.

### Testing

**Mechanics tests (`game/test`, run by `make check`):** a new `test/battle_speed.c`.

- Put the tick rule in one function, for example
  `GetBattleTicksThisFrame(void)`, and test it:
  - it returns the option's speed for wild, trainer and double battles;
  - it returns 1 for link, recorded, recorded link, catch tutorial and
    Pokédude battles;
  - it returns 1 in the `TESTING` build unless a test sets the speed.
- `SetDefaultChallengeSettings` leaves `battleSpeed` at 1×.
- `ChallengeSettingsLayoutPinned` still compiles (size 32).

**E2E (`e2e/`):** a new `e2e/src/journeys/wayfarer-battle-speed.e2e.ts`.

- Add a `battleSpeed` field to `arrange` (`e2e/src/harness/game-session/features/arrange.ts`),
  next to `determinism.textSpeed`.
- **Faster.** `game.battle.startWild` with a fixed seed and Tackle. Count the
  frames from choosing Tackle to the action menu coming back, at 1× and 4×.
  4× must take at most half the frames of 1×. The bound is a placeholder;
  tune it once measured.
- **Music unchanged.** Over 300 frames in battle, `gMPlayInfo_BGM.clock`
  advances by the same amount at 1× and 4× (±1).
- **One press, one action.** At 4×, a single A press on FIGHT opens the move
  menu and does not choose a move.
- **Saved.** Set 3×, `saveAndReload`, and read 3× back.
- E2E cannot cover link battles, because it runs one emulator. The tick-rule
  test covers them.

**By hand:** at 4× on hardware or an accurate emulator, play a double battle
with heavy animations (Earthquake, Surf, weather). Check for no lag, no
corrupted sprites and no cut-off cries.

## Open questions

1. **Default speed.** 1× or 2×? This is the PRD's open question 1.
   *Recommendation:* 1×.
2. **Top speed.** Stop at 4×, or offer only 1×–3×?
   *Recommendation:* 4×. The ceiling rule drops ticks in heavy frames, so 4×
   cannot cause lag. Drop it only if playtesting finds 4× unreadable.
3. **FAST BATTLES wording.** Its descriptions are misleading. "Skips all
   delays" promises more than it does now that BATTLE SPEED exists.
   *Recommendation:* rename it SKIP PAUSES and change its On text to "Skips
   the pauses after battle messages."

## References

- Engine paths: `src/main.c` (`AgbMainLoop`, `CallCallbacks`, `VBlankIntr`),
  `src/battle_main.c` (`BattleMainCB1`, `BattleMainCB2`, `VBlankCB_Battle`),
  `src/battle_anim.c`, `src/battle_message.c`, `src/text.c`,
  `src/battle_interface.c`, `src/battle_script_commands.c`, `src/option_menu.c`
- [In-game clock](in-game-clock.md)
