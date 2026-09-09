# Trainer appearance validation

Acceptance is still in progress. The user approved four styles for v1: Gold,
Kris, Brendan, and May, displayed as Style 1 through Style 4. Their saved IDs
remain 1, 2, 5, and 6. IDs 3 and 4 are reserved and rejected by the registry.
Red and Leaf are deferred; their incomplete action art is recorded in the
[historical candidate report](trainer-appearance-art-blocker.md).

The [PRD](../../.product/prds/trainer-appearance-styles.md) and
[specification](../../.product/specs/trainer-appearance-styles.md) originated in
PR #87. The approved four-style scope supersedes their original six-style
roster. The implementation starts from `583308c817`. Keep `Implemented: No`
until the implementation lands with the required evidence.

## Acceptance status

| Area | Completed evidence | Still required |
| --- | --- | --- |
| Picker | Isolated picker run passed in 16.85 seconds; all four native-resolution previews were inspected. Naming lifecycle and reset coverage also passed. | Fast-intro emulator coverage. |
| Initialization and persistence | Final intro E2E suite passed 8/8 in 291.17 seconds, including save/reload and Continue. Mechanics exercise all eight style/origin handoffs, four flash roundtrips away from home, reserved/invalid IDs, gender mismatch, and caller rejection before save clearing. | Fast-intro emulator coverage. |
| Movement and actions | Walking/card/battle/bike journeys passed 12/12; water passed 4/4; underwater passed 4/4; running and reflection passed 4/4. All four running and reflection captures were inspected. | Field-move pose and cleanup coverage. Watering is currently unreachable; VS Seeker is disabled. |
| Local rendering | Walking, Trainer Card and battle-intro captures exist. Mechanics cover local portraits, live-link local controller selection, legacy remote/recorded resolution, contest graphics, and facility head-frame bounds. | Complete Hall of Fame, facility records/playback, other UI, and actual live-link/recorded emulator sessions. |
| Regions and story | Mechanics cover eight style/origin combinations through Hoenn entry and Kanto/Hoenn/Johto warps, plus legacy rival names and graphics. | Complete emulator travel, save/reload away from home, and gender-linked story checks. |
| Build isolation and resources | Final Wayfarer, HNS, FireRed, LeafGreen, and Emerald releases all passed serially; Wayfarer reserve enforcement remains enabled. | No outstanding build failures. |

Compilation, bounds checks, and screenshots from a partial journey do not
establish completion of the remaining acceptance matrix.

## Current checks

- `make -C game -j8 BUILD=wayfarer UNUSED_ERROR=1 DEPRECATED_ERROR=1 release`: passed. Log:
  `/tmp/appearance-four-release-final.log`.
- Wayfarer mechanics selection: 88/88 passed after the movement restoration fix.
  Log: `/tmp/appearance-four-mechanics-final.log`. The long-running existing HM
  acquisition matrix delayed one runner; the run completed successfully.
- Narrow caller-preflight mechanics test: 1/1 passed. Log:
  `/tmp/appearance-four-caller.log`. The test restores the runner callback after
  checking the rejected new-game title callback.
- `python3 game/tools/trainer_appearance/check_manifest.py`: 36 combinations,
  zero failures. `python3 game/tools/trainer_appearance/test_manifest.py`: 6/6
  passed. These checks establish table/frame bounds, not visible action quality.
- Abandoned setup reset: 1/1 passed in 37.12 seconds; the fresh Gold preview was inspected.
- Final intro E2E: 8/8 passed. Log: `/tmp/appearance-eight-origin-e2e.log`.
  Picker and naming lifecycle: 2/2 passed, eight native captures inspected.
- Underwater E2E: 4/4 passed in 12.49 seconds. Log:
  `/tmp/appearance-underwater.log`.
- Combined walking/card/battle and bike suite: 12/12 passed in 76.63 seconds, including actual Fight/Surf input and return
  to the overworld. Log: `/tmp/appearance-four-visual-e2e-real-battle.log`.
  All four native battle-back captures and all four Trainer Cards were inspected. Earlier attempts using
  the debug win helper stalled at the action menu; the actual battle driver
  resolved that test limitation.
- Water-action suite: 4/4 passed in 33.73 seconds after fixing movement-state
  loss on return from the Bag. Sixty captures cover entry, directions, bobbing,
  right-facing fishing, and dismount. All four characters were visually inspected
  surfing up/right, fishing right, and dismounted. Down-facing Lapras obscures
  the trainer; those captures do not establish visible pose correctness.
- Running and Ice Path reflection suite: 4/4 passed in 10.03 seconds. It asserts
  the dash flag, the expected animation for all four directions, movement to a
  different coordinate, and a local reflection sprite. All four running and all
  four reflection captures were inspected. Log:
  `/tmp/appearance-running-final.log`.

The underwater journey starts from a fixture at Underwater Route 124 (10, 5).
It captures all four directions, uses B and the native confirmation to surface,
then A and the native confirmation to dive again. Each style retains its saved
ID and changes movement flags from underwater to surfing and back. The initial
placement is not an observed dive. All 24 direction, surfaced, and returned
captures were inspected; no obvious sprite corruption was observed. Captures
and surfaced state JSON are under
`e2e/artifacts/trainer-appearance/style-N-underwater-*`.

Picker captures are under `e2e/artifacts/trainer-appearance/picker/`. Other
surface captures are under `e2e/artifacts/trainer-appearance/`. These local
artifacts document the runs that produced them and must not be presented as
validation of later untested changes.

## Resource measurements

The passing four-style release used 32,252,320 ROM bytes, 248,517 EWRAM bytes,
and 25,556 IWRAM bytes. The report reconciles its categories to the ROM total
and records `wayfarer_release_limit_enforced: true`.

The normal production limit remains `0x09F80000`, retaining 512 KiB before the
physical 32 MiB limit. This build leaves 1,302,112 bytes before the physical
limit and 777,824 bytes before the production limit. No reserve exemption was
used.

The accepted baseline records 32,865,136 used bytes. The reported total delta is
-612,816 bytes; the graphics category is 2,898,028 bytes, +956 against that
baseline. These are whole-build comparisons, not a claim that appearance work
alone caused every category change. The generated report is
`game/pokewayfarer-release-size.json`. This measurement includes the menu movement restoration fix.

## Standalone release checks

All four targets passed again after the final movement restoration fix.

| Standalone release | ROM bytes used |
| --- | ---: |
| HNS | 30,804,368 |
| FireRed | 28,616,300 |
| LeafGreen | 28,616,704 |
| Emerald | 28,521,460 |

The builds ran serially. Final logs are `/tmp/appearance-four-{hns,firered,leafgreen,emerald}-final.log`.
Existing linker/newlib and LTO warnings remain in the logs.

Other earlier passes: tools/check-tools; Hoenn content audit 10 tests; ROM-report
suite 21 tests; origin static suite 20 tests; E2E harness five files and 22 tests;
E2E typecheck and changed-file lint. Initial six-style Wayfarer builds failed on
missing Red/Leaf action descriptors. That build blocker was superseded by the
approved four-style registry; the rejected art was never accepted as production
assets.

## Current emulator limitations

The SkyEmu HTTP harness exposes no serial transport or peer attachment API.
Two isolated sessions cannot establish a live link battle. Local-link controller
mechanics checks are not actual live-link visual evidence.

VS Seeker charging is disabled in `game/include/config/item.h`; its item uses
`ItemUseOutOfBattle_CannotUse`. Wayfarer's HNS branch of
`game/data/scripts/berry_tree.inc` makes Wailmer Pail use immediately end.
Neither animation is reachable through those items in the current build.
Enabling these gameplay features is outside the appearance change; asset bounds
checks do not replace actual action captures.

## Review

Independent native review found no confirmed defect in the four-style registry,
menu movement restoration, pending name/challenge callbacks, or legacy remote
FRLG paths. A separate E2E/protocol review found the v14 request layout and sparse
style mapping coherent. Ordinary walking and underwater direction captures do
not assert coordinate deltas; their directional evidence relies on visual review.
Water movement tests do assert coordinates and restoration flags.

## Hall of Fame driver attempt

A bounded Style 1 emulator attempt used the normal Hoenn league admission
fixture and Sidney interaction. Calling the debug battle-win helper before the
action menu did not produce a usable victory return. A subsequent attempt
used actual Fight/Surf input, but also returned to the league lobby with
controls locked before reaching Hall of Fame. The latter attempt failed 1/1
in 35.43 seconds; its log is `/tmp/appearance-hof.log`. No Hall of Fame portrait
was captured, and no appearance rendering defect was established by this
failed journey. The unproven test was removed; its diagnostic source is retained
locally at `/tmp/wayfarer-appearance-hof-attempt.e2e.ts`. Hall of Fame visual
acceptance remains pending.
