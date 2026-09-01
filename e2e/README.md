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

The HNS-only battle and storage fixtures are deliberately narrow. They support
the HM party-management journeys, not a general battle simulator or a complete
PC model. Party and PC fixtures share species, level, moves, and Egg state.
Fainted state is party-only because boxed Pokémon do not have meaningful current
HP. Tests identify Pokémon by unique species; personality and other identity
fields are not part of this ABI.

```ts
await game.arrange({
  checkpoint: "new-bark-after-intro",
  player: {
    facing: "up",
    position: { map: "cherrygrove-pokemon-center", x: 11, y: 2 },
  },
  party: [{ species: "lapras", moves: ["surf"] }],
  bag: { hms: { surf: 1 }, items: { masterBall: 1 } },
  pc: {
    currentBox: 0,
    observedSlots: [
      { box: 0, slot: 0, mon: null },
      { box: 0, slot: 1, mon: { species: "pidgey", moves: ["fly"] } },
    ],
  },
})
```

Fixtures create synthetic starting state. `arrange()` may create the party and
Bag, initialize and observe up to eight requested PC coordinates, and choose the
current box. Every `pc.observedSlots` entry requires `mon`: a Pokémon initializes
the slot with that fixture, while `null` clears it. The same coordinates remain
in bounded telemetry for `game.state.read()` and `game.storage.slot(box, slot)`.
Boxed fixtures intentionally do not model current HP or fainted state. HMs use
`bag.hms`; `bag.items` is the generic item surface used for the Master Ball and
does not expose duplicate HM aliases.

`battle.startWild()` creates the requested wild opponent and starts the normal
battle state machine from a settled overworld. It resolves at the first real
battle-text input boundary. The playbook then uses controller input for the Bag,
capture messages, catch-swap prompt, and party picker. Capture tests use a Master
Ball because capture probability is outside this capability's scope. V1 supports
ordinary HNS overworld battles only; Safari Zone, Bug Contest, and other special
battle contexts are outside its contract.

Storage journeys arrange the player in Cherrygrove's Pokémon Center and interact
with its real PC script. There is no command that opens storage directly. The
mode selection, deposit, withdrawal, box movement, release, and exit screens run
through their normal game tasks and real controller input. Telemetry reports only
the current box, the requested PC coordinates, and small semantic state-machine
markers. It does not publish all 420 box slots or a general event history.

Field-use assertions also stay on gameplay paths. For example, the recovery
journey leaves the PC, walks to Cherrygrove's shore, and interacts with the water
to prove that the withdrawn Surf user is resolved again. Tests should not treat
fixture state or a memory snapshot as proof that capture, storage, release, or
field use worked.

The command mailbox is versioned as ABI v7. Arrangement and wild-battle commands
share request IDs and result handling, reject commands while a harness-owned game
state machine is active, and validate invalid species, item quantities, boxes,
and slots in the ROM. Protocol changes must increment the ABI and update both the
C static layout assertions and TypeScript protocol tests. Raw invalid-request
construction is available only through the explicitly named protocol-test
support module; it is not part of the public `GameSession` gameplay facade.

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

The mailbox, checkpoints, and telemetry only exist in the HNS E2E ROM. Normal and
release ROMs do not compile them. Other map versions are outside this capability.

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
