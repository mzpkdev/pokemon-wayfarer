# SkyEmu tests

This is a package in the repository's pnpm and Turborepo workspace. It does
not run the ROM Makefile or assume a `game/` directory. Give the suite a ROM
and matching symbol file through `SKYEMU_ROM` and `SKYEMU_SYMS`.

## Setup

Install the workspace dependencies:

```sh
pnpm install
```

The `skyemu-static` dependency bundles the patched SkyEmu v5 Linux x64 binary.

The test runner starts SkyEmu through Xvfb, so it does not open a visible
emulator window. On Ubuntu 24.04, install the required runtime packages with:

```sh
sudo apt install libasound2t64 libxcursor1 libxi6 libopengl0 xvfb
```

Other Linux distributions provide equivalent ALSA, Xcursor, Xi, OpenGL, and
Xvfb packages.

Build the playable test ROM from the repository root before running the suite:

```sh
make -C game -j"$(nproc)" e2e
```

This produces `game/pokemon-wayfarer-e2e.gba` with `E2E_TESTING=1`, plus
matching ELF, map, and symbol files. It is a normal playable game ROM. It is
separate from `make -C game check`, which builds and runs the mechanics-test
runner with `TESTING=1`.

## Run tests

```sh
SKYEMU_ROM="$PWD/game/pokemon-wayfarer-e2e.gba" \
SKYEMU_SYMS="$PWD/game/pokemon-wayfarer-e2e.sym" \
pnpm run smoke
```

Run every E2E tier with:

```sh
SKYEMU_ROM="$PWD/game/pokemon-wayfarer-e2e.gba" \
SKYEMU_SYMS="$PWD/game/pokemon-wayfarer-e2e.sym" \
pnpm run e2e
```

The suite boots the ROM through SkyEmu's HTTP server, checks that it reports a
loaded ROM, and advances 60 frames. Its smoke test copies the supplied ROM to a
temporary directory, starts a fresh game, accepts the default character and
challenge settings, and verifies that it reaches the player's bedroom in New
Bark Town. Each test starts SkyEmu on a reserved port and uses its own temporary
ROM copy, so test files can run in parallel.

The ROM and matching symbol paths are always explicit. The tests never build,
scan, or otherwise depend on the game source tree.

## Game session API

`GameSession` uses the `E2E_TESTING=1` ROM hook to arrange gameplay state before a
test starts. `arrange()` creates a clean new game, applies a named checkpoint
and its overrides as one request, then resolves after the normal map loader has
returned control to the player.

```ts
const game = await GameSession.launch()

await game.arrange({
  checkpoint: "elm-lab-before-intro",
  player: {
    facing: "up",
    position: { map: "elm-lab", x: 6, y: 8 },
  },
  story: {
    flags: { hideSilverInNewBark: true },
    vars: {
      newBarkTownLabState: 0,
      newBarkTownState: 2,
    },
  },
  determinism: {
    rngSeed: 1,
    textSpeed: "instant",
  },
})

await game.player.move("up")
await game.dialogue.waitForOpen()
```

The current checkpoints are `bedroom-before-clock`,
`new-bark-after-intro`, and `elm-lab-before-intro`. A test can override the
map, coordinates, facing direction, supported story vars and flags, RNG seed,
and text speed. One request can patch up to eight vars and eight flags.

`game.state.read()` returns the current game phase, map, player position and
facing direction, control lock, script activity, and dialogue state.
`game.story.var()` and `game.story.flag()` read semantic story values through
layout offsets published by the ROM ABI. `game.player` supplies real controller
actions, while `game.wait` and `game.dialogue` synchronize against ROM state
instead of fixed frame delays.

The mailbox, checkpoints, and telemetry only exist in the E2E ROM. Normal and
release ROMs do not compile them.

## Layout

`src/harness/` owns the emulator, ROM, process mechanics, and the public
`GameSession` API. Raw controller and memory operations stay private to the harness.
Tests use `GameSession` directly, while `src/playbooks/` composes it into reusable
player flows such as starting a new game.

Inside `harness/game-session/`, the facade owns launch and cleanup, `protocol.ts`
owns the mailbox wire format, `catalog.ts` owns semantic names, and `runtime.ts`
owns raw emulator access. Each fluent namespace lives in `features/`.

`src/smoke/` contains fast tests that run for every pull request. `src/journeys/`
contains longer player flows. The E2E tier currently covers the opening through
Elm's first dialogue; future journeys can move to a nightly workflow as they
become more expensive. Each test must use its own temporary ROM copy so suites
can run in parallel without sharing a save file.
