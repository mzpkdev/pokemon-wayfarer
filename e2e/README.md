# SkyEmu tests

This directory is a standalone TypeScript test workspace. It does not run the
ROM Makefile or assume a `game/` directory. Give the suite a ROM with
`SKYEMU_ROM`.

## Setup

Install the Node dependencies, then build the pinned SkyEmu source:

```sh
cd e2e
pnpm install
pnpm run setup:skyemu
```

`setup:skyemu` checks out SkyEmu at a fixed revision under `e2e/.skyemu/` and
builds it with CMake. It leaves the pinned SkyEmu source unmodified.

The SkyEmu build needs CMake, a C/C++ toolchain, SDL2, OpenGL, curl, OpenSSL,
and Xvfb on Linux. If there is no active `DISPLAY`, the test runner starts
SkyEmu through `xvfb-run`.

## Run tests

```sh
cd e2e
SKYEMU_ROM=/absolute/path/to/wayfarer.gba pnpm test
```

The suite boots the ROM through SkyEmu's HTTP server, checks that it reports a
loaded ROM, and advances 60 frames. Tests run in one process because one
SkyEmu instance owns the selected port.

Use `SKYEMU_BIN` to test an already-built emulator instead of
`e2e/.skyemu/build/bin/SkyEmu`. `SKYEMU_DIR` changes the checkout directory,
and `SKYEMU_PORT` chooses a fixed HTTP port. The ROM path is always explicit;
the tests never build, scan, or otherwise depend on the game source tree.

## Add a scenario

Add a `*.e2e.test.ts` file under `src/`, import `startSkyEmu`, and test the
documented HTTP endpoints. Keep scenario data and generated artifacts in this
directory so the harness remains portable.
