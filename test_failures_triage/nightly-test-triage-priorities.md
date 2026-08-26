# Runtime test triage priorities

This document orders current investigation of the historical [runtime test failure inventory](./nightly-runtime-test-inventory.md). It does not diagnose every result or authorize production behavior changes. The inventory is a PR #6 snapshot, not a current full-run result. Reclassify a row only after recording current evidence.

## Verified current starting point

Current CI is the `expansion-suite` job in `.github/workflows/ci.yml`:

```sh
TEST=1 UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j"$(nproc)" check
```

Earlier local evidence used the Emerald variant:

```sh
UNUSED_ERROR=1 DEPRECATED_ERROR=1 GAME_VERSION=EMERALD TEST=1 make -j22 check
```

The completed [PR #7 CI run](https://github.com/mzpkdev/pokemon-wayfarer/actions/runs/32975432374) against `fb4ad1f2c5` is the latest full-suite aggregate for this triage. It reported 5,113 total tests: 2,553 `PASS`, 1,884 `FAIL`, 31 `ASSUMPTION_FAIL`, 11 `KNOWN_FAILING`, 628 `TO_DO`, and 6 `EXPECT_FAILING`. The process exited 2 because tests failed. The job completed without hitting the runner timeout. Focused results below may include later worktree changes.

The local `game/build/triage-evidence/full-strict-final.log` is a 4,676-line, 378,565-byte pre-fix artifact. It compiled and started Hydra but stopped before a final summary. Its partial counts remain useful for comparison, but the completed PR #7 run supersedes it as the current aggregate.

## Immediate focused work

1. **Keep the SaveBlock3 test baseline.** `ClearSaveBlocks` zeroes SaveBlock3 before every test. In this project, zero enables the One Type challenge with `TYPE_NONE` rather than disabling it, so species are rejected and sent to the PC. The test runner explicitly selects the disabled sentinel. A strict focused `pokemon.c` run exited 0 with 26 `PASS` and 1 unrelated `KNOWN_FAILING`; all `givemon` paths passed. The focused `pokerus.c` run exited 0 with 19 `PASS`. Neither run emitted a timeout or `INVALID` result.
2. **Keep the Daycare fixture unchanged and defer its regional-form policy discrepancy.** `test/daycare.c:12-15` deliberately stores `gPlayerParty[0]` twice. `StorePokemonInDaycare` clears that party slot and compacts the party, so the second original parent moves into slot 0 before the second store. There is no slot-1 correction to make. The focused `daycare.c` run exited 2 with 4 `PASS` and 1 `FAIL`: regional forms at `test/daycare.c:166`, `EXPECT_EQ(965,52)`. It produced no timeout or `INVALID` result. Source diagnosis places the outcome in production `DetermineEggSpeciesAndParentSlots`, including current-region, Everstone, and regional-form tables. It is a deferred gameplay-policy discrepancy. No production code or test expectation changed because either could alter or hide live breeding behavior.
3. **Keep the committed Poké Ball runner exception under focused validation.** Commit `fb4ad1f2c5` exempts `EFFECT_ITEM_THROW_BALL` from the explicit party-index requirement in `test/test_runner_battle.c`. The exact Light Metal Heavy Ball and Heavy Metal Heavy Ball runs each exited 0 with 1 `PASS`, with no timeout or `INVALID` result. Broader capture and Ball Fetch observations remain incomplete.
4. **The Dazzling Z-status fixture is validated.** The static Baby-Doll Eyes and Fairium Z data are consistent, but a zeroed SaveBlock3 disables `tx_Mode_Fairy_Types` at runtime. The test enables Fairy types locally before creating its battle fixture. Its exact focused run exited 0 with 1 `PASS`, with no timeout or `INVALID` result. This validates the fixture setup, not every Dazzling, Queenly Majesty, or Armor Tail mechanics row.
5. **Keep Fairy mode local to tests that require it.** The exact Focus Punch AI and weakness-berry runs changed from `ASSUMPTION_FAIL` to `PASS` after their fixtures enabled Fairy typing. Pickpocket now reaches the mechanic and fails on an unmatched message instead of its Fairy prerequisite. Keep that mechanics failure open; do not change its expectation without a separate diagnosis.
6. **Remove the intentional recovery marker from the active defect queue.** `Tests resume after CRASH` uses `KNOWN_CRASHING` and deliberately calls a null function in `test/test_test_runner.c`. Its focused run exits 0, showing recovery coverage.
7. **Do not prioritize Heavy Ball mechanics from the historical rows.** The focused Heavy Ball and both literal Heavy Metal and Light Metal title runs pass with the committed Poké Ball runner exception. The inventory rows remain historical until a focused current run records their status.

## Focused evidence and central-fix candidates

These remain hypotheses, not confirmed fixes. Each could explain several rows, but focused evidence must establish a causal link before any shared battle-code change.

| Candidate | Likely reach | First focused check | Guardrail |
| --- | --- | --- | --- |
| SaveBlock3 test baseline | Non-Normal `givemon` setup and dependent Daycare and Pokérus fixtures. | `pokemon.c` exited 0 with 26 `PASS` and 1 unrelated `KNOWN_FAILING`; all `givemon` paths passed. `pokerus.c` exited 0 with 19 `PASS`. | These focused checks had no timeout or `INVALID` result. The baseline remains test-only, not a gameplay default. The Daycare regional-form result is a deferred gameplay-policy discrepancy. |
| Hazard queue and switch-in pipeline | Hazard removal and switch-in ordering failures. | Reproduce Defog, Rapid Spin, and Tidy Up followed by multiple hazards. | This remains a source-reading hypothesis. Confirm queue state and order before changing it. |
| Stat-change pipeline | Contrary, prevention abilities or items, Mirror Armor, Defiant, Competitive, and Sticky Web bookkeeping. | Reproduce Competitive or Defiant after Sticky Web, including Court Change, and inspect the causal state. | This remains a source-reading hypothesis. Do not bulk-change prevention rules from one failure. |
| Damage configuration classification | Some upstream Gen 5+ and Gen 6+ critical-hit and spread-damage expectations. | Run a focused test with test-local configuration overrides, or classify it as configuration divergence. | This remains a hypothesis. Changing defaults affects Wayfarer gameplay and requires a product decision. |

## Tier 1: runner and harness validation

Goal: make every test result interpretable. Start with `ASSUMPTION_FAIL` as a file-level prerequisite blocker, not as an individual assertion failure. The focused `givemon` and Pokérus paths now execute cleanly, but other fixture and helper failures still need evidence before their dependent results count as game-behavior evidence.

Exit criteria: each affected row has either a valid execution result, an intentional/unsupported classification with evidence, or a focused test setup issue. Do not work through ordinary battle failures until their harness dependencies are understood.

## Tier 2: shared behavior clusters

Goal: find one cause that explains multiple failures before inspecting individual assertions. Use one representative test, then run the related group after every confirmed change or classification.

Investigate in this order:

1. Daycare regional forms, because the shared SaveBlock3 baseline, all `givemon` paths, and Pokérus now pass focused checks. The Daycare macro is not the defect. Defer its egg-species policy discrepancy rather than changing live breeding behavior or its expectation.
2. Sleep Clause, because it has many related failures and a small number of `ASSUMPTION_FAIL` rows.
3. Hazards and switch-in effects, including ordering, forced switches, Sticky Web, and Toxic Spikes.

Exit criteria: each cluster has a written classification, a representative focused reproduction, and a list of inventory rows covered by that conclusion.

## Tier 3: high-volume source groups

Goal: reduce the queue through shared battle systems once test execution is trustworthy. Work by source group rather than by the inventory's original order.

Start with the largest groups, such as move effects and abilities. Within a group, prefer a failure that exercises shared machinery, for example damage calculation, turn or event ordering, targets, stat changes, or message handling. A representative result may reveal a configuration difference, a missing upstream behavior, or an actual regression; it is not proof for every row in the group.

Exit criteria: split each source group into smaller evidence-backed themes. Mark confirmed common causes, and leave unrelated failures as separate rows rather than forcing one explanation.

## Tier 4: upstream-feature divergence and known failures

Goal: separate tests for mechanics the fork deliberately does not support from failures that merit a Wayfarer change. All nine historical `KNOWN_FAILING` titles map to current source, but their historical status does not prove the current reason. `KNOWN_FAILING_GEN` does not exist in current source or upstream, so do not plan around that marker. Review feature-heavy groups such as Tera, Dynamax, Z-Moves, Trainer Slides, and generation-specific rules against the project's intended behavior.

Exit criteria: every reviewed row is marked as expected divergence, missing upstream port, retained known failure, or a candidate regression. Keep the supporting source/configuration evidence in the inventory notes.

## Tier 5: isolated one-offs

Goal: finish the remaining independent failures only after the shared and divergence work has reduced the queue. This includes save compatibility, UI text-fit assertions, and single-mechanic tests that do not share a confirmed theme.

Exit criteria: each row has a focused reproduction and one of the same evidence-backed classifications. Promote a row back to Tier 2 or Tier 3 if investigation uncovers a wider cluster.

## Test result markers

Use result markers only after the inventory has an evidence-backed classification.

- Use a placeholder only when a test has not been implemented. It reports `TO_DO` immediately and does not exercise a test body:

  ```c
  TO_DO_BATTLE_TEST("TODO: reason");
  ```

  A plain `// TODO` comment has no effect on the runner.

- For an existing test that should still run but is known to fail, use:

  ```c
  KNOWN_FAILING; // triage ID or evidence-backed reason
  ```

  A failing result is reported as `KNOWN_FAILING` without failing the suite. If it begins passing, the runner reports `KNOWN_FAILING_PASS`; remove the marker then.

Do not use either marker to hide an uninvestigated regression, invalid test setup, or a product decision that has not been made.

## Working rules

- Use a narrow current reproduction before changing a row's classification:

  ```sh
  UNUSED_ERROR=1 DEPRECATED_ERROR=1 GAME_VERSION=EMERALD TEST=1 make -j22 TESTS="<prefix>" check
  ```

- Record the classification and evidence in the inventory's `Triage` and `Notes` columns.
- Re-run the relevant focused group after a test-only or harness change. Save a new full-run inventory only after a run completes with a final status and the group-level triage is stable.
