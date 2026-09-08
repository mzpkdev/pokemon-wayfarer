# Trainer-only emulator coverage audit

The suite retains emulator checks for distinct entry, UI, staging, restoration,
outcome and field-return behavior. Mechanics tests own numeric combinations and
shared predicates; source audits own the complete caller inventory. Variant
coverage does not require every starter × gender × party × entrance combination.

## Cuts and consolidation

| Area | Before | Selected coverage | Reason |
| --- | --- | --- | --- |
| Johto coverage | 160 internal fixtures in 11 tests | 35 fixtures | Keep every authored approach and objective, distribute starter/party variants, and preserve actual outcomes, recovery, occupied-tile reload and exclusions. |
| Ilex/Tower hosts | 6 tests | 2 tests | One native completion per host; repeating the same nonbattle writer with all three unusable-party forms adds no new branch. |
| Hoenn story | 50 tests | 26 tests including gap coverage | Reduce 30 route matrix cells to 10 covering every approach and gender with all starters represented; keep one recovery per route. Remove Rustboro x12 and Victory Road x2 tests already contained in wider entrance tests. Add two restored-actor interactions and extend the existing Route 119 test through forced outcomes. |
| Hoenn objectives | 35 tests | 17 tests | One refusal per objective and one per Rusturf west row. Preserve all five distinct native loss continuations, usable backup staging, real rescue victory and both public exits. |
| Ordinary story | 15 tests | 14 tests | Joey's standalone refusal duplicates the refusal already checked after his real loss. Render all four lines, but repeat/reload only one representative stable assignment. |
| Radio host | One stalled full-fight attempt | Two outcome-boundary tests | Existing forced-outcome harness can exercise the native occupation writer and disguise restoration without maintaining another combat input driver. These are not evidence of actual combat victory/defeat. |

Counts distinguish named tests from fixtures inside loops. Earlier large passing
runs remain historical evidence; they are not the maintained suite's case counts.

## Checks retained deliberately

- Core controller, medicine, storage/capture picker, origin recovery, Safari and
  Rock reaction tests exercise real UI or callbacks that helper tests cannot.
- Mikey's direct retry and Joey's leave-sight/re-enter retry cover different
  recovery behavior. Joey also protects a phone-registration continuation.
- The separate Johto coverage file drops its partial Tower discovery walkthrough;
  the host test already executes that scene through beast release.
- Both Rusturf west coordinate rows remain, alongside direct Grunt interaction.
  This alternate staging path previously exposed a real missed gate.
- Objective losses retain distinct retreat scripts and protected writers. Similar
  dialogue alone does not make their actor restoration equivalent.
- Native encounter-source mechanics remain: synthetic emulator wild fixtures do
  not prove the actual land, outbreak, fishing and field-action readers.

## Meaningful gaps selected

1. One natural ordinary-grass encounter with an empty party closes the composition
   gap between native source generation and actual battle startup.
2. A deterministic Less Escapes veto covers its separate restriction and failed
   turn commitment; the ordinary escape curve and global Run ban do not cover it.
3. Lilycove rival and Mauville Wally restoration check their own object placement
   and native interaction after actual recovery. These stop at the actor's intro;
   no additional full battle is needed to prove restoration.
4. Tower Silver completion after native beast release and Route 119's bike/foot
   retreat plus Fly/Scott victory writer need outcome-boundary checks.
5. Radio native occupation completion must preserve deferred Underground Silver.
   A separate fake-Director defeat must restore the disguise actors before retry.
6. Narrow mechanics assertions cover partner/ghost exclusion predicates without
   multiplying emulator scenarios.

Future personal Blue/FRLG/Sevii ports and deliberately excluded authored
challenge/League consequences remain outside this task's implementation scope.
No broad new challenge or campaign matrix is justified by this audit.

Validation results for the selected suite are recorded in
[the implementation ledger](trainer-only-implementation-validation.md).

## Keeping verification bounded

Run changed journey files through `wa test` and confirm the reported case count.
This workspace's wrapper does not forward Vitest's `-t` or a trailing
`-- --testNamePattern=...` as a reliable name filter; the latter ran the entire
Hoenn file during diagnosis. Do not count that as a focused run. After a passing
run, repeat it only for a relevant code/assertion change or an unresolved failure.
Game builds remain sequential; emulator runs use explicit immutable ROM/symbol
pairs and do not build maps.
