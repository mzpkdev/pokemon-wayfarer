# Nightly test triage baseline

This is the historical PR #6 compiler-blocker note, updated to separate its
claims from current-worktree verification. Historical commit
`c4a1f524ca3eec83bacc55d06f1fa2053a00bb3a` is locally available and its
strict build can be replayed. The raw historical log and original runtime
checkout or environment evidence are unavailable, so the original observed
result cannot be independently confirmed.

## Scope

The current CI workflow is `.github/workflows/ci.yml`. Its `expansion-suite`
job sets `TEST=1`, `UNUSED_ERROR=1`, and `DEPRECATED_ERROR=1`, then runs:

```sh
TEST=1 UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j"$(nproc)" check
```

There is no scheduled nightly workflow. The PR #6 statement that CI used
`.github/workflows/build.yml` is no longer accurate.

## Historical claim

The PR #6 note attributes the following two compile errors to source commit
`c4a1f524ca3eec83bacc55d06f1fa2053a00bb3a`, using the Emerald variant of the
command above. It says that run exited 2 during compilation and never reached
mGBA or Hydra. That is historical context, not a current result.

### Undefined Togepi egg flag

```text
src/braille_puzzles.c:379:17: error: 'FLAG_RECEIVED_TOGEPI_EGG' undeclared (first use in this function)
```

The historical note says this occurred in `CheckTogepi` while building
`build/emerald-test/src/braille_puzzles.o`.

### Unused Frontier healthbox function

```text
src/battle_gfx_sfx_util.c:66:37: error: 'GetSinglesPlayerHealthboxFrontier' defined but not used [-Werror=unused-function]
cc1: all warnings being treated as errors
```

The historical note says this occurred while building the corresponding object
target.

## Current verification

Current strict test objects compile and link, and Hydra starts. Neither the
`CheckTogepi` missing-flag error nor the unused Frontier healthbox-helper
error appears in the current strict build. The current full-run command used
for local evidence was:

```sh
UNUSED_ERROR=1 DEPRECATED_ERROR=1 GAME_VERSION=EMERALD TEST=1 make -j22 check
```

It produced a bounded partial log at
`game/build/triage-evidence/full-strict-final.log`: 4,676 lines and 378,565
bytes. It has 3,898 result lines: 1,945 `PASS`, 1,416 `FAIL`, 49
`ASSUMPTION_FAIL`, 8 `KNOWN_FAILING`, 5 `EXPECTED_FAIL`, 474 `TO_DO`, and 1
intentional `CRASH`; no `INVALID` result was observed. The run did not
complete, and no final status, complete summary, or exact elapsed time was
captured. Those counts are partial evidence, not a replacement inventory.

The intentional `CRASH` is the `Tests resume after CRASH` recovery test in
`test/test_test_runner.c`. It deliberately calls a null function under
`KNOWN_CRASHING` and its focused run exits 0, so it validates runner recovery
rather than representing an active crash defect.

## Current fixture preconditions

`ClearSaveBlocks` clears SaveBlock3 before each test. A zero
`tx_Challenges_OneTypeChallenge` enables the One Type challenge with
`TYPE_NONE`, not the disabled state, so species are rejected and sent to the
PC. The test runner now sets the disabled sentinel after clearing SaveBlock3. This is a
test-only baseline adjustment. A strict focused `pokemon.c` run exited 0 with
26 `PASS` and 1 unrelated `KNOWN_FAILING`; all `givemon` paths passed. The
focused `pokerus.c` run exited 0 with 19 `PASS`. Neither run emitted a timeout
or `INVALID` result.

The Daycare helper intentionally deposits `gPlayerParty[0]` twice. Depositing
the first parent clears its slot and compacts the party, moving the other parent
to slot 0 before the second deposit. Replacing the second slot with `[1]` would
be incorrect. The focused `daycare.c` run exited 2 with 4 `PASS` and 1 `FAIL`.
The remaining regional-form assertion is `test/daycare.c:166`,
`EXPECT_EQ(965,52)`. It had no timeout or `INVALID` result. Source diagnosis
confirms the fixture deposit and compaction are correct; production
`DetermineEggSpeciesAndParentSlots` uses current-region, Everstone, and
regional-form tables.
This is a deferred gameplay-policy discrepancy. No production code or test
expectation changed because either could alter or hide live breeding behavior.

The Dazzling Z-status test has matching static Baby-Doll Eyes and Fairium Z
data, but a cleared SaveBlock3 disables Fairy types at runtime. Its fixture
enables Fairy types locally. The exact focused run exited 0 with 1 `PASS`, with
no timeout or `INVALID` result. This validates the test setup, not every
Dazzling, Queenly Majesty, or Armor Tail mechanics row.

The exact Light Metal Heavy Ball and Heavy Metal Heavy Ball runs each exited 0
with 1 `PASS`, with no timeout or `INVALID` result. Broader capture and Ball
Fetch observations remain incomplete.

## Reproduction and follow-up

Use the CI-equivalent command for CI verification:

```sh
TEST=1 UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j"$(nproc)" check
```

For the historical Emerald-focused reproductions, use:

```sh
UNUSED_ERROR=1 DEPRECATED_ERROR=1 GAME_VERSION=EMERALD TEST=1 make -j22 TESTS="<prefix>" check
```

The current evidence does not prove a full-suite completion. It also cannot
validate the original PR #6 log range, runtime environment, or repeat count.
Consult the inventory and priorities notes before treating a historical row as
current.
