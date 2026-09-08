# HNS upstream integration evidence

## Inputs and ancestry

- Wayfarer baseline: `28aaad96a0b8a2b9d790d22d78c384cca05d189d`.
- Previously imported upstream: `93c5eee2abb78f0baaf4050da4a65572bbdcfb66`.
- Imported upstream target: `8cb27a9fb01a0f80063c6285fa5805ae9c594329`.
- Ancestry bridge: `c0f30e4c3b`, whose tree is identical to its first parent and
  whose second parent is the previously imported upstream commit.

The upstream delta is 38 commits across 66 files. A synthetic rehearsal of the
ancestry bridge and subtree merge matched an independent three-way merge of the
game trees in every cleanly merged file, with the same five conflicting paths.
The actual subtree merge likewise touched 66 game files before reconciliation.
Merging the previous upstream revision after the bridge reported already up to date.
Repeating the target import in a fresh uberepo worktree also left HEAD unchanged.
A synthetic later upstream commit adding one file merged cleanly and changed only
that file under `game/`, without altering the working tree during verification.

The old squash workflow produced ten conflicts and changed 574 files during the
assessment. Ordinary subtree merges now use the actual upstream merge base;
historical squash trailers are no longer involved.

## Reconciliation decisions

- Preserve existing HNS/Dojo and Hoenn trainer IDs. Append the 22 upstream
  post-OBC trainers after the combined Wayfarer range; use compact IDs in HNS.
  Store their defeated state in unused HNS trainer flags, keeping it separate
  from Hoenn's flag bank. Classify and regenerate their scaling data.
- Retain Wayfarer League, Dojo, and Red routing. Keep upstream's post-OBC routes
  in non-Wayfarer HNS builds.
- Combine upstream attacks with existing conditional native HM entries. All HM
  entries retain their levels. An explicit upstream-delta fixture updates exact
  learnset expectations without changing historical HM design fixtures. Wayfarer
  excludes the newly added Donphan Play Rough at 52 and Tauros Blaze Kick at 50:
  they would remove Strength from catch windows at levels 52–100 and 55–62,
  respectively. Standalone HNS retains both additions.
- Keep the summary screen's removed Pokédex shortcut and prompt. Let physical L
  reach the EV handler on Skills in L=A mode; the remapped A bit previously
  consumed it. A host-compiled test executes the actual handler to cover this.
- Give the HNS Route 4 Dynamic Punch tutor its own Cerulean message, retaining
  the shared original Mossdeep message for Hoenn. This preserves the original
  Hoenn content fingerprint without resetting the audit expectation.
- Accept upstream's Nuzlocke fix. The removed blanket legendary exception was
  inherited from upstream, not a Wayfarer customization. Roamers receive an
  explicit encounter exception while still respecting the one-type challenge.
- Accept upstream's orphan New Sinjoh encounter-table removal, map/sign updates,
  randomizer and Safari fixes, Tauros form mapping, egg rerolls, and link-card fix.
  Import map binaries directly from upstream; no manual binary edits are used.

## Validation

Final full-suite and required CI status are recorded on
[PR #76](https://github.com/mzpkdev/pokemon-wayfarer/pull/76).

Both baseline and integrated release builds pass with
`UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j8 BUILD=wayfarer release`.
The accepted ROM baseline was not changed.

| ROM measurement | Before | After | Change |
| --- | ---: | ---: | ---: |
| Used bytes | 32,866,544 | 32,878,860 | +12,316 |
| Physical capacity remaining | 687,888 | 675,572 | -12,316 |
| Code | 2,698,312 | 2,698,616 | +304 |
| Trainer data | 368,360 | 376,576 | +8,216 |
| Scripts | 1,961,852 | 1,962,084 | +232 |
| Maps/layouts | 2,082,168 | 2,082,180 | +12 |
| Graphics | 2,904,468 | 2,904,464 | -4 |
| Encounter data | 72,420 | 72,256 | -164 |
| Other data | 11,057,916 | 11,061,636 | +3,720 |
| Audio | 11,721,048 | 11,721,048 | 0 |

The build also enforces the stricter Wayfarer release end address `0x09F80000`;
the integrated end `0x09F5B10C` leaves 151,284 bytes before that limit. Most growth
is in the added trainer parties; the smaller changes correspond to imported code,
scripts, learnsets, graphics, and removal of the orphan encounter table.

| Check | Baseline | Integrated |
| --- | --- | --- |
| Wayfarer release | Pass | Pass |
| Static analysis (`ANALYZE=1`, debug) | Pass | Pass |
| Standalone HNS release | Not needed for comparison | Pass: 31,233,636 bytes |
| E2E ROM build | Pass | Pass |
| Smoke | 3 passed | 3 passed |
| Full E2E coverage | 120 passed, 17 files | 117 passed in full run; remaining 3 passed on targeted startup retry |

Targeted checks pass: trainer classification and ID uniqueness in both builds,
HNS/Wayfarer routing, ten Hoenn content tests with the original fingerprint,
summary input dispatch, native-HM fixtures, and 3,888 regional plus 10,692
directional modeled coverage cells. The new C defeat-state isolation regression
passes in the mechanics runner.

The encounter suite initially passed 57 of 60 tests; the remaining three expected
the pre-update HNS profile counts. Upstream removes exactly one orphan New Sinjoh
profile: update counts from 169 to 168 headers and 271 to 270 source profiles,
assert that the orphan is absent, and rerun those three checks successfully.

Baseline mechanics reports 4,303 passed, 349 known failing, nine assumptions failed,
629 TODO, six expected failing, and zero failures. The first integrated run found
one failed Heavy Ball metadata case. It relied on a zero-odds throw succeeding
under the old division-by-zero behavior. Isolate metadata validation from badge
penalties and add a zero-odds regression for Wobbuffet and Beldum; all eight capture
tests then pass. Neither the upstream odds guard nor the catch-rate clamp is reverted.

The integrated full E2E run passes all 12 League lifecycle/scaling cases and 117
tests overall. Two emulator processes aborted before readiness, skipping three
cases in the boot and Magnet Train files. Both files pass on a targeted retry
outside the sandbox with `LD_LIBRARY_PATH` unset. No gameplay assertion failed;
the initial full-run exit status was nonzero because of those startup errors.
Baseline SkyEmu also crashes inside the sandbox, so emulator tests require the
working host environment. The retry does not establish whether the inherited
library path or transient startup contention caused the two aborts.

Two-emulator link play was not exercised; the imported trainer-card link fix is
covered by compilation and code review, not a direct multiplayer runtime test.

The optional ROM-report unit target has a pre-existing missing fixture
`tools/rom_report/tests/fixtures/at_limit.sym` on baseline main. This does not stop
the release report or its size-limit enforcement; no fixture or test skip is added
as part of the upstream update.

## Landing constraint

At implementation time the GitHub repository allowed squash merging only. Its
`Protect main` ruleset also required linear history and limited PRs to squash merges.
This PR requires a merge commit to preserve the repaired ancestry. Landing requires
enabling repository merge commits, allowing that method in the PR rule, and removing
the linear-history requirement. Keep required checks, PR review/thread rules,
deletion protection, and force-push protection. Do not squash or rebase this
integration. No repository settings are changed by the implementation.
