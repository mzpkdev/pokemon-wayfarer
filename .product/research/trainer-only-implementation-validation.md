# Trainer-only implementation validation

Status: In progress. This record does not mark either specification implemented.

Task: `trainer-only-implementation`, based on main
`7525da55faf196a52a1d3efe7160d65b3ef893f0` (merged PR #81).
All changes and builds run in the new task worktree. Game builds run sequentially.

## Product decisions

- Run uses `clamp(floor(50*C/L) + 15*priorFailures, 5, 95)%`. Failed attempts
  consume a turn; there is no guaranteed escape. The existing Less Escapes
  bit-512 refusal uses the same random draw and no player Pokémon stats.
- The explicit feeding table includes standard Cheri through Maranga berries,
  including standard Enigma. It excludes the configurable e-Reader berry.
  Selecting a berry feeds the wild Pokémon directly. Non-berry recovery medicine
  uses normal party targeting; restoring protection ends the encounter.
- Trainer retaliation invokes centralized recovery with no money charge.
  Ordinary supported battle losses retain their existing single charge.
- Exceptional challenge, League, tutorial, chained/partner and unaudited Gym
  losses retain their existing routing. They do not gain ordinary field return.

## Core mechanics

Commands run from the repository root:

| Command | Observed result |
| --- | --- |
| `make -C game BUILD=wayfarer -j6 check TESTS='Trainer*'` | 88 passed, one assumption skip in the existing trainer-scaling rollback test. New trainer-only arithmetic, turn resolution, berry eligibility and entry tests passed. |
| `make -C game BUILD=wayfarer -j6 check TESTS='Capture*'` | 14 passed, including explicit Safari Ball quantization, owned-ball proximity, absent-player modifiers and Quick/Timer completed turns. |
| `make -C game BUILD=hns -j6 check TESTS='Capture*'` | 14 passed after correcting two Wayfarer-only predicate compilation guards. |
| `make -C game BUILD=firered -j6` | Standalone FireRed ROM compiled successfully. |
| `make -C game BUILD=emerald -j6` | Standalone Emerald ROM compiled successfully. |
| `make -C game BUILD=wayfarer -j6 check TESTS='Wayfarer trainer field*'` | One passed: trainer field return requires an explicit redirect and an actual loss. |
| `make -C game BUILD=wayfarer -j6 check TESTS='Wayfarer trainer retaliation*'` | One passed: established Lavaridge recovery adjustment applies in Hoenn and does not leak into Olivine. |
| `make -C game BUILD=wayfarer -j6 check TESTS='Storage compaction*'` | One passed: sparse and empty parties are recounted. |
| `make -C game -j6 e2e` | Playable Wayfarer E2E ROM and matching symbols built. Runtime validation remains in progress. |

The broader `TESTS='Wayfarer*'` run stopped producing results and was interrupted.
It is not recorded as a passing suite. The new hardcore-exclusion and retaliation
cause tests passed in that run; the trainer-loss test passed separately above.

The E2E ABI is version 15. TypeScript, changed-file lint and 12 protocol tests
passed. Emulator launches require isolated temporary
`XDG_DATA_HOME`: the bundled binary crashes if its default cache cannot be written.
The harness now creates and cleans up that directory for each launch.

The expanded core emulator suite passed 23 of 23 cases on ROM SHA-256
`6e86984d30c6445ccb0445b314ac50f59c18c1f151b4253a7214feb4ffe399a7`.
The origin special-recovery suite passed four of four cases on that ROM. Earlier
failures led to a recovery-route fix and corrections to menu/text telemetry and
test input timing. The natural Safari journey passed one of one cases, including
real admission, the closest Go Near message, Run, retirement and a subsequent
ordinary battle. The medicine journey passed five of five on ABI 15 ROM
`dbd2fcff3656841914beb355d0f6b6c8df8b3d8e8fb829bb20f64d27c1c3297f`:
selected-move Ether recovery, cancellation, full-PP no effect, inapplicable
Antidote and empty-party refusal. Ether changed PP from `[3,2]` to `[13,2]`,
consumed one item and committed one turn. No production item change was needed.

The core foundation gate passed before story implementation began. Final log
collection uses consolidated ABI 15 ROM
`5012418fb92921cf3ed730973838bacc2777dbea6133173c15ab8cd6b9732b38`;
its additional fixture change honors requested wild-mon PP consistently with
party/PC PP. Those consolidated run results will be recorded separately.

The action menu was inspected visually: Rock/Bag occupy the top row and
Go Near/Run the bottom, with a trainer sprite and no player Pokémon healthbox.
The retaliation screenshot shows the trainer attack message and lunge. A warning
screenshot taken after the message had cleared is not evidence of text wrapping;
that visual check remains pending.

## Coverage boundaries

The supported implementation target is ordinary single wild encounters, reversible
PC Deposit/Move, ordinary supported defeat and the current scene inventory in the
story specification. Emulator and scene coverage must be recorded below before
claiming those targets complete.

Current main has no personal Blue chapter port. Blue is the existing HNS Viridian
Gym leader and retains Gym defeat routing. The future FRLG/Sevii source inventory
is excluded; this task does not import those maps or implement the proposed Blue
origin/chapter-retirement campaign.

Scripted wild objectives, tutorials, partner/chained fights, League and challenge
exceptions retain their explicit policies. No unsupported loss caller may resume
a victory continuation.
