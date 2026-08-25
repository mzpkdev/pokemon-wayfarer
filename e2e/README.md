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
SKYEMU_ROM=/absolute/path/to/wayfarer.gba pnpm run test:skyemu
```

The suite boots the ROM through SkyEmu's HTTP server, checks that it reports a
loaded ROM, and advances 60 frames. Its smoke test copies the supplied ROM to a
temporary directory, starts a fresh game, accepts the default character and
challenge settings, and verifies that it reaches the player's bedroom in New
Bark Town. Tests run in one process because one SkyEmu instance owns the
selected port.

The ROM path is always explicit; the tests never build, scan, or otherwise
depend on the game source tree.

## Add a scenario

Add a test file under `src/`, import `startSkyEmu`, and test the documented HTTP
endpoints. Keep scenario data and generated artifacts in this directory so the
harness remains portable.
