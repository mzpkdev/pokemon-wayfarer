# SkyEmu tests

This is a package in the repository's pnpm and Turborepo workspace. It does
not run the ROM Makefile or assume a `game/` directory. Give the suite a ROM
with `SKYEMU_ROM`.

## Setup

Install the workspace dependencies:

```sh
pnpm install
```

The `skyemu-static` dependency provisions SkyEmu during install. It downloads
the package's patched SkyEmu v5 Linux x64 archive, verifies its SHA-256, and
stores the binary with the package. Subsequent installs reuse it.

The test runner uses Xvfb and starts SkyEmu through `xvfb-run` by default, so
it does not open a visible emulator window. Set `SKYEMU_USE_DISPLAY=1` when you
want to use the current display while debugging.

## Run tests

```sh
SKYEMU_ROM=/absolute/path/to/wayfarer.gba pnpm run test:skyemu
```

The suite boots the ROM through SkyEmu's HTTP server, checks that it reports a
loaded ROM, and advances 60 frames. Tests run in one process because one
SkyEmu instance owns the selected port.

`SKYEMU_BIN` skips the local package binary and uses an already-installed
emulator. Use it on macOS, Windows, or a non-x64 Linux machine.
`SKYEMU_PORT` chooses a fixed HTTP port. The ROM path is always explicit; the
tests never build, scan, or otherwise depend on the game source tree.
`SKYEMU_USE_DISPLAY=1` runs SkyEmu in the current display instead of Xvfb.

## Add a scenario

Add a `*.e2e.test.ts` file under `src/`, import `startSkyEmu`, and test the
documented HTTP endpoints. Keep scenario data and generated artifacts in this
directory so the harness remains portable.
