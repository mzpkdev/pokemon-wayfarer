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
error appears in the current strict build. The completed
[PR #7 CI run](https://github.com/mzpkdev/pokemon-wayfarer/actions/runs/32980328525)
against `5993e0982b` is retained as comparable CI evidence. It reported 5,113 total
tests: 2,556 `PASS`, 1,884
`FAIL`, 28 `ASSUMPTION_FAIL`, 11 `KNOWN_FAILING`, 628 `TO_DO`, and 6
`EXPECT_FAILING`. The process exited 2 because tests failed. The job completed
without hitting the runner timeout. A later attempt did not acquire
a hosted runner, but subsequent PR pushes are acquiring runners normally. That
cancellation is not an active task blocker.

Hydra can now write an uncapped NDJSON ledger with schema
`mgba-rom-test-hydra/v1`. A local current-head `-j22` run recorded all 5,149
terminal results, 26 diagnostics, and a final summary: 2,564 `PASS`, 1,892
`FAIL`, 46 `ASSUMPTION_FAIL`, 12 `KNOWN_FAILING`, 629 `TO_DO`, and 6
`EXPECT_FAILING`. File-level assumptions execute once per Hydra worker, so
those local totals are worker-count-dependent and are not directly comparable
to CI. The ledger is useful as complete per-result evidence, not as a
replacement CI baseline.

The battle runner also reports exact expected and observed message bytes before
an unmatched or forbidden message result. It preserves the existing matcher,
queue advancement, result status, and source line. A full local report with
this instrumentation recorded 2,563 `PASS`, 1,893 `FAIL`, 46
`ASSUMPTION_FAIL`, 12 `KNOWN_FAILING`, 629 `TO_DO`, and 6 `EXPECT_FAILING`.
The only status difference from the preceding local report was one X-item
Friendship parameter that passed twice when rerun alone. It remains
unclassified as run-level nondeterminism or state leakage.

The report contains pending-message context for 1,881 result records. At least
one observed message was retained for 1,762 records; 119 retained none. The
four-entry history overwrote older observations in 1,344 records, so it cannot
prove that an unretained message was absent. Charmap decoding found 268 records
with an expected/observed pair that differed only in canonical species, move,
ability, or item capitalization. HnS commit `73c788a6b1` capitalized those
canonical data names, while older imported message expectations retained title
case.

The first evidence-backed casing batch updates 382 exact assertions across 113 test
files. A complete local rerun recorded 2,732 `PASS`, 1,724 `FAIL`, 46
`ASSUMPTION_FAIL`, 12 `KNOWN_FAILING`, 629 `TO_DO`, and 6 `EXPECT_FAILING`.
Normalized comparison by source file and test title found 169 `FAIL` to `PASS`
transitions and no `PASS` to `FAIL` transitions. Pending-message context fell
from 1,881 to 1,716 records: 1,690 `FAIL`, 23 `PASS`, and 3 `KNOWN_FAILING`.
Some updated tests now fail at a later independent assertion, which is why the
net pass count is lower than the diagnostic-record count.

Reconstructing that ledger found 99 failed result records with a retained
case-only pair, correcting an earlier count of 78. Iterating through newly
exposed assertions updates 144 exact expectations across 53 test files,
including generic stat labels already rendered by the game. Three complete
reruns moved 71 more tests from `FAIL` to `PASS` with no reverse transitions.
That complete run recorded 2,803 `PASS`, 1,653 `FAIL`, 46
`ASSUMPTION_FAIL`, 12 `KNOWN_FAILING`, 629 `TO_DO`, and 6 `EXPECT_FAILING`.
Pending-message context fell to 1,647 records. Its final two retained case-only
pairs were aligned afterward, and both exact focused tests pass.

A separate presentation-only pilot aligns three Galar form scenarios and 11
directly observed Red Card messages. All three Galar targets pass. Nine Red
Card scenarios pass, while the rooted and Suction Cups scenarios advance to
their next independent message assertions. The complete run after the two final
case-only updates and this pilot then recorded 2,817 `PASS`, 1,639 `FAIL`, 46
`ASSUMPTION_FAIL`, 12 `KNOWN_FAILING`, 629 `TO_DO`, and 6 `EXPECT_FAILING`.

Wave 24 recorded all 5,149 results: 3,484 `PASS`, 972 `FAIL`, 46
`ASSUMPTIONS_FAILED`, 12 `KNOWN_FAILING`, 629 `TO_DO`, and 6 `EXPECT_FAILING`.
An earlier scan incorrectly declared the presentation-only lane exhausted
because it considered only the first unmatched message. The runner can retain
later actual messages while the same expectation is pending, so the complete
retained-history scan found another 242 directly evidenced expectations across
127 test files.

Wave 25 applied those 242 expectation-only changes and recorded 3,621 `PASS`
and 835 `FAIL`. There were 138 `FAIL` to `PASS` transitions at edited records;
102 other edited records advanced to later assertions. One unrelated X-item
Friendship result moved from `PASS` to `FAIL`, passed in isolation, and recovered
in Wave 26. The next adversarial scan required a deterministic unmatched-message
failure, a direct one-line source literal, no prior or overwritten actuals, and
a case-only match to the first retained actual on the same queue. Two independent
generators agreed on 55 more expectations. Wave 26 recorded 3,666 `PASS` and 790
`FAIL`, with 45 `FAIL` to `PASS` transitions and no regressions. A final five-row
batch produced Wave 27: 3,671 `PASS`, 785 `FAIL`, and no regressions. All other
status counts remained unchanged throughout.

The three batches changed exactly 302 `MESSAGE` expectations across the same 127
test files and did not change production code or matching behavior. Two
independent Wave 27 scans found zero remaining candidates under the strict gate.
Wave 28 adds two directly observed casing corrections in randomized tests and
makes four sleep tests exercise the configured generation instead of aborting
when `B_SLEEP_TURNS` is older than Gen 5. The sleep bounds remain exact: three
turns for Gen 5 and later, four for Gen 3 and 4, and seven for older generations.
The newly exercised sleep paths add another 15 exact uppercase message
expectations.

Wave 28 recorded 5,107 results: 3,677 `PASS`, 783 `FAIL`, 12 `KNOWN_FAILING`,
629 `TO_DO`, and 6 `EXPECT_FAILING`, with no assumption failures. Compared with
Wave 27, the two randomized tests moved from `FAIL` to `PASS` and all four unique
sleep tests moved from `ASSUMPTIONS_FAILED` to `PASS`. The total fell by 42
because Hydra had emitted the Dark Void and Hypnosis file-level assumption once
per worker, 22 times each. Replacing those 44 duplicate rows with one passing
result per test accounts for the lower total; the 5,099 normalized test
identities are unchanged. No result regressed.

A strict Wave 28 scan found no further presentation-only candidate. None of the
717 remaining deterministic unmatched-message results meets the full evidence
gate: a direct decoded case-only or wrapping-only pair on the same queue, with no
prior or overwritten actual messages. The rest are probability wrappers and
other assertions. They require separate gameplay, ordering, configuration, or
missing-evidence classification.

Exact expectation updates are gameplay-neutral. Reverting the production names
would change visible gameplay, and case-insensitive matching would weaken the
assertions, so neither is part of this triage work. Automatic line wrapping is
accepted only for directly decoded message pairs with otherwise exact content.
Records without direct presentation-only evidence remain unresolved.

Earlier local pre-fix evidence used:

```sh
UNUSED_ERROR=1 DEPRECATED_ERROR=1 GAME_VERSION=EMERALD TEST=1 make -j22 check
```

It produced a bounded partial log at
`game/build/triage-evidence/full-strict-final.log`: 4,676 lines and 378,565
bytes. It has 3,898 result lines: 1,945 `PASS`, 1,416 `FAIL`, 49
`ASSUMPTION_FAIL`, 8 `KNOWN_FAILING`, 5 `EXPECTED_FAIL`, 474 `TO_DO`, and 1
intentional `CRASH`; no `INVALID` result was observed. The run did not
complete, and no final status, complete summary, or exact elapsed time was
captured. Those counts are pre-fix evidence and do not supersede the completed
PR #7 aggregate.

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

The Focus Punch AI, Pickpocket spread-target, and weakness-berry tests also
explicitly require Fairy typing. Each fixture now enables that mode locally.
The exact Focus Punch and weakness-berry runs changed from `ASSUMPTION_FAIL` to
`PASS`. Pickpocket now reaches the mechanic and fails on an unmatched message,
so its remaining failure needs separate diagnosis. No expectation or battle
behavior changed.

The exact Light Metal Heavy Ball and Heavy Metal Heavy Ball runs each exited 0
with 1 `PASS`, with no timeout or `INVALID` result. The Poké Ball runner
exception used by those checks is committed in `fb4ad1f2c5`. Broader capture
and Ball Fetch observations remain outside this presentation-only scope. Both
`CRITICAL_CAPTURE_IF_OWNED` cases completed with `PASS` in the Wave 24 full
run; the older focused timeout artifacts remain historical evidence.

Additional Fairy, modern Sturdy, and modern Sitrus settings are enabled only
inside fixtures that require them. Focused checks confirm that Inverse Battle,
AI Encore, both Whimsicott immunity cases, Aura Break, Filter, type-power
items, and Z-Nature Power now execute with valid setup. Roost, Mimicry, Sturdy,
Sitrus, and related fixtures that still report ordinary battle failures remain
open. Their expectations and production mechanics were not changed.

## Reproduction and follow-up

Use the CI-equivalent command for CI verification:

```sh
TEST=1 UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j"$(nproc)" check
```

For the historical Emerald-focused reproductions, use:

```sh
UNUSED_ERROR=1 DEPRECATED_ERROR=1 GAME_VERSION=EMERALD TEST=1 make -j22 TESTS="<prefix>" check
```

The completed PR #7 CI evidence proves full-suite completion without a runner
timeout. Later pushes are acquiring hosted runners normally. The local NDJSON
report provides an uncapped current result ledger, but it
does not validate the original PR #6 log range, runtime environment, or repeat
count. The inventory rows remain historical until a focused current run
records evidence for an individual row.
