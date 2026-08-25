# SkyEmu tests

This is a package in the repository's pnpm and Turborepo workspace. It does
not run the ROM Makefile or assume a `game/` directory. Give the suite a ROM
with `SKYEMU_ROM`.

## Setup

Install the workspace dependencies:

```sh
pnpm install
```

The `skyemu-static` dependency bundles the patched SkyEmu v5 Linux x64 binary.

The test runner starts SkyEmu through Xvfb, so it does not open a visible
emulator window. Install Xvfb on Linux before running the suite.

## Run tests

```sh
SKYEMU_ROM=/absolute/path/to/wayfarer.gba pnpm run test:smoke
```

The suite boots the ROM through SkyEmu's HTTP server, checks that it reports a
loaded ROM, and advances 60 frames. Its smoke test copies the supplied ROM to a
temporary directory, starts a fresh game, accepts the default character and
challenge settings, and verifies that it reaches the player's bedroom in New
Bark Town. Each test starts SkyEmu on a reserved port and uses its own temporary
ROM copy, so test files can run in parallel.

The ROM path is always explicit; the tests never build, scan, or otherwise
depend on the game source tree.

## Layout

`src/harness/` owns the emulator, ROM, and process mechanics. `src/actions/`
contains small reusable inputs and game-state reads. `src/playbooks/` composes
actions into player flows such as starting a new game.

`src/smoke/` contains fast tests that run for every pull request. `src/journey/`
is reserved for longer flows that can run in the nightly workflow. Each test
must use its own temporary ROM copy so suites can run in parallel without
sharing a save file.
