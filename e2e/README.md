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
emulator window. Install Xvfb on Linux before running the suite.

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

## Layout

`src/harness/` owns the emulator, ROM, and process mechanics. `src/actions/`
contains small reusable inputs and game-state reads. `src/playbooks/` composes
actions into player flows such as starting a new game.

`src/smoke/` contains fast tests that run for every pull request. `src/journeys/`
contains longer player flows. The E2E tier currently covers the opening through
Elm's first dialogue; future journeys can move to a nightly workflow as they
become more expensive. Each test must use its own temporary ROM copy so suites
can run in parallel without sharing a save file.
