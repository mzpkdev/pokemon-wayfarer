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
- Choices: `1×`, `2×`, `3×`, and `4×` unless open question 2 drops it (the
  charmap has `×`). The default for a new game is open question 1.
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
- **Extra ticks stop** for the rest of the frame, and the battle runs at 1×,
  while any of these holds. Check before every extra tick, because each can
  start mid-frame:
  - the player is choosing an action, a move, a target or a Pokémon;
  - a palette fade is active or waiting for its VBlank transfer
    (`gPaletteFade.active`, `sPlttBufferTransferPending` in `src/palette.c`);
  - the capture-success animation is playing, or a Pokémon has just been
    caught. Its sparkles toggle each tick and flicker when several ticks
    share one drawn frame.
- **Speed is a ceiling** (only if 4× stays, see open question 2). Skip an extra
  tick when it would not finish before the next VBlank. Measure this with the
  scanline counter, using a tunable threshold. A heavy frame drops ticks,
  never whole frames. The speed never falls below 1×.

### Same outcomes at every speed

A battle's result never depends on its speed. The same seed and the same
inputs give the same moves, damage, critical hits and catches at 1× and 4×.

- Today `VBlankIntr` advances the RNG once per drawn frame
  (`AdvanceRandom` in `src/main.c`, skipped in link, Frontier and recorded
  battles). At 4× a battle draws a quarter of the frames, so it would see
  different random numbers than at 1×.
- Fix: in battle, advance the RNG once per **tick** instead of once per
  drawn frame, under the same exclusions. Waiting at a menu still changes
  later rolls, as it does now.

### What speeds up

Everything driven by battle ticks:

- move, status and stat-change animations (`gAnimScriptCallback` and the
  `delay` waits in `src/battle_anim.c`);
- sprite animations: send-out, faint, switch, the intro and trainer slides;
- battle text printing (`BattlePutTextOnWindow` in `src/battle_message.c`,
  printed by `RunTextPrinters`);
- the scripted `pause` and `waitmessage` waits, when FAST BATTLES is off;
- HP and Exp. bar movement (`MoveBattleBar` in `src/battle_interface.c`).

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
  - SoulGold counts these waits per call, so at speed it stops long sound
    effects early. Counting frames avoids that.
- **Palette fades and the capture-success animation.** They run at 1×, as
  [above](#how-a-faster-battle-runs). A fade can't run faster anyway:
  `UpdatePaletteFade` takes one step per VBlank transfer.
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
  - it returns 1 while the player chooses a move, during a palette fade, and
    during the capture-success animation.
- Same outcome: one scripted wild battle with a fixed seed ends with the same
  damage, critical hits and catch result at 1× and 4×.
- `SetDefaultChallengeSettings` leaves `battleSpeed` at the default.
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
   SoulGold starts new games at 2×.
   *Recommendation:* 2×, following SoulGold. Players who dislike it find the
   option at once, and new players get the feature players asked for.
2. **Top speed.** Stop at 4×, or offer only 1×–3×? SoulGold defines 4× but
   its menu stops at 3×, and it has no ceiling rule.
   *Recommendation:* 3×, without the ceiling rule. It is proven in a shipped
   hack on the same engine, and it drops the unmeasured scanline threshold.
   Add 4× later if players ask.
3. **Hold L for 1×.** SoulGold runs a battle at 1× while L is held, to read a
   message or watch an animation.
   *Recommendation:* yes, unless the L=A button mode is on, where L is A.
4. **FAST BATTLES wording.** Its descriptions are misleading. "Skips all
   delays" promises more than it does now that BATTLE SPEED exists.
   *Recommendation:* rename it SKIP PAUSES and change its On text to "Skips
   the pauses after battle messages."

## References

- Engine paths: `src/main.c` (`AgbMainLoop`, `CallCallbacks`, `VBlankIntr`),
  `src/battle_main.c` (`BattleMainCB1`, `BattleMainCB2`, `VBlankCB_Battle`),
  `src/battle_anim.c`, `src/battle_message.c`, `src/text.c`,
  `src/battle_interface.c`, `src/battle_script_commands.c`, `src/option_menu.c`
- [In-game clock](in-game-clock.md)

### Prior art

[SoulGold](https://github.com/Eemeliri/soulgold) (read at commit `3f4d90aa`)
is built on Heart and Soul like Wayfarer and ships the same tick loop. Its
README credits the battle speed-up to Pokabbie's
[Emerald Rogue](https://github.com/Pokabbie/pokeemerald-rogue).

- Same design: `BattleMainCB2` runs extra ticks, and only VBlank uploads to
  the hardware (`src/battle_main.c`).
- This spec takes from it: the guards on extra ticks
  (`CanRunExtraBattleTick`), the RNG per tick (`AdvanceBattleFrameRng`), and
  hold L for 1× (`Rogue_GetBattleSpeedScale` in `src/battle_controllers.c`).
- Neither repository has a license. Use them as reference only: write
  Wayfarer's own code, or get the authors' permission before porting any.
  Credit Pokabbie in `game/CREDITS.md` either way.
