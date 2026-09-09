# Trainer-only implementation validation

Status: Supported core and current story implementation complete and locally
validated. Implementation status excludes future ports, new exceptional policies
and shipping balance approval. Historical runs below retain their original
results; the maintained-suite section records the final selection.

Task: `trainer-only-implementation`, based on main
`7525da55faf196a52a1d3efe7160d65b3ef893f0` (merged PR #81).
All changes and builds run in the new task worktree. Game builds run sequentially.

## Compact ordinary caller records

The reviewed 851-caller manifest and its command fingerprints are unchanged.
Ordinary records now store only caller pointer, stable dialogue key, dialogue
kind and flags; shared policy/retreat defaults are expanded into caller-owned
storage. Active encounter state retains its own copy. Regional scene records and
per-frame registry scope remain unchanged.

Matched normal Wayfarer builds reduced the ordinary table from 30,636 to 6,808
bytes. ROM end moved from `0x09eea108` to `0x09ee44bc`, a net saving of 23,628
bytes (23.1 KiB), including the added lookup code. Persistent active-state storage
adds 36 bytes of EWRAM. These are local normal-build measurements, not a claim
about the exact CI release delta. Evidence: `/tmp/trainer-compact-size.json` and
`/tmp/trainer-compact-build.log`.

Generator validation retains all 851 callers and 787 field-loss redirects;
16 Python tests passed. Seven native story mechanics tests passed, including
all compact records' expanded fields, independent lookup values, unknown-caller
rejection and registry duplicate checks. Logs are
`/tmp/trainer-compact-generator.log`, `/tmp/trainer-compact-python.log` and
`/tmp/trainer-compact-mechanics.log`. Read-only review found no behavior defect.

The unchanged trainer-story emulator suite passed 14/14 on E2E ROM
`3e6b47b155ec8e1dd6f5da385485dff30b9b539483894f3512dd36861305e053`
(`/tmp/trainer-compact-e2e.log`). This includes ordinary refusal variants,
rematches, completed text, real field losses and selected regional story/exception
routing. E2E build log: `/tmp/trainer-compact-e2e-build.log`. No new scenario
matrix or screenshot files were added to the changeset.

## Settled exceptional defeat policy

The user confirmed the existing challenge, League and unaudited authored-battle
exclusions as settled behavior. The PRD and specs now state the current shared
entry/field-loss/storage gate, native challenge recovery, League whiteout ending
the run, and unchanged unaudited scene routing. Configured but inactive Nuzlocke
retains its native activation semantics; hardcore blocks on its option alone.
These are documentation decisions about existing code, not newly enabled challenge
support. No gameplay changed or new emulator coverage is claimed. Broader balance
validation and future port implementation remain separate work.

## Scripted-wild policy clarification

The user confirmed the existing scripted-wild exclusions and no-party guards as
settled behavior. Steven/Kecleon keeps its safe, retryable refusal without a
usable party; native eligible resolution owns Scope/bridge completion. Electrode,
Marowak and Lostelle retain their scripted encounter rules, with unported scenes
still belonging to future ports. The PRD and both specs no longer describe this
as a temporary policy or pending trainer-only conversion. No gameplay changed;
this clarification required a documentation consistency check, not new ROM tests.

## Always-visible Pokémon menu

Wayfarer's normal and debug start menus now always include Pokémon. They no
longer depend on `FLAG_SYS_POKEMON_GET`, so a first capture before receiving a
starter is browsable, and an empty party can open its existing Cancel-only
screen. The menu does not set that flag or advance starter/chapter state.
Non-Wayfarer builds retain their native flag check.

The existing first-capture journey now clears both starter and Pokédex flags,
opens and cancels the empty-party screen, catches a real Pidgey, and browses it
before confirming the next encounter uses normal battles. It checks the starter
flag remains false after each party-menu visit. This focused case passed on
E2E ROM `0a9eede1c27fe540cf4d6853c6186016d40ed84ac2dcac7076f550e0297bc98c`
(`/tmp/trainer-only-party-menu-e2e-final.log`; 1 passed, 22 unrelated cases
skipped). Temporary focus was removed. Root inspected both party screens;
scoped formatting, lint and TypeScript checks passed. The initial driver sent
Cancel before the returned start menu finished opening; waiting for its normal
transition corrected the test without a gameplay change.

E2E and playable Wayfarer builds passed
(`/tmp/trainer-only-party-menu-build.log` and
`/tmp/trainer-only-party-menu-playable-build.log`). The playable ROM SHA-256 is
`12332936f0d8c7007826e982e9c0501d1a0a44c20908590cc90eaeaab8b296b9`.

## R-button ball shortcut

The trainer-only menu now restores and hides the native last-used-ball widget.
It shares the ordinary controller's cycling input handler and emits a distinct
shortcut action. That action rechecks native capture eligibility, removes one
owned ball, and enters the existing trainer-only capture path. The widget's
move-info sprite access now checks its sentinel, since trainer-only encounters
have no move-info widget.

Three focused cases passed on E2E ROM
`e51e59655bd70b561fc03d30ea1215e8f37490d5ef8415595313ebb29f11e654`
(`/tmp/trainer-only-quick-e2e-final.log`): trainer-only R Quick Ball failure
consumes one ball and one completed turn; no-ball R leaves the turn unchanged;
and the ordinary party-battle R shortcut still consumes its ball and returns
after failure. Root visually inspected the widget beside the trainer-only menu.
Cycling and capture-restriction delegation were source-reviewed; these are not
claims of separate emulator cases. The initial test driver waited at battle
intro text; the final driver acknowledges it before checking the action menu.

Scoped format/lint and TypeScript checks passed. E2E and playable Wayfarer builds
passed (`/tmp/trainer-only-quick-build.log` and
`/tmp/trainer-only-quick-playable-build.log`). The playable ROM SHA-256 is
`5caec3419347235ddd35d48bc04fe5ab96478bbeafc6c06308ffcded2b99c57e`.

## Native feeding animation

Committed berry feeding now plays the existing Pokéblock throw unchanged after
Bag returns. The controller waits for the trainer throw, projectile and wild
sway before applying food and resolving the single turn. No new graphics or
berry-specific projectile were added.

The feeding case and four existing Bag-cancellation fixtures passed 5/5 on
E2E ROM `d998b8746cbba57b6536ca3ba6c20db629d0114547960effd201be795f938d27`
(`/tmp/trainer-only-feed-e2e.log`; 18 unrelated cases deliberately skipped).
Temporary focus markers were removed. Feeding retains exactly-one-berry and
one-completed-turn assertions, and now checks that the turn remains pending
through the animation. Root inspected sampled frames showing the trainer arm,
Pokéblock arc and wild motion. Scoped format/lint and TypeScript checks passed.

E2E and playable Wayfarer builds passed (`/tmp/trainer-only-feed-build.log` and
`/tmp/trainer-only-feed-playable-build.log`). The playable ROM SHA-256 is
`53ef300a1f72619288f15b0b351421523116bbaaf569b788bb191541c355f809`.

## Empty-party Pokémon Center fix

The ball-placement effect spawned before checking its remaining count. Zero
became -1, leaving the animation active and reading past the six ball positions.
The shared placement loop now skips spawning when the count is zero and completes
normally. Shared completion also releases an unused glow palette when no ball
sprite existed to free it. Actual party counts and existing Egg rules remain;
fainted members are still healed normally.

The native Cherrygrove nurse regression disables fast healing and confirms the
animated branch before checking completion and subsequent walking. On the prior
`e925cba8…` ROM, the empty-party case timed out after accepting healing
(`/tmp/trainer-only-pokemon-center-old-normal-e925.log`). That exploratory run's
fainted-party failure was a dialogue matcher error, not a healing defect. Earlier
fast-heal and unaccepted-menu probes are not reproduction evidence.

Both final cases passed on E2E ROM
`0f2aa50d6131444f36f0a8bd6dff51cfb27205e947823891fe5c289b09377687`
(`/tmp/trainer-only-pokemon-center-final-0f2a.log`): empty-party completion and
real fainted-member recovery, each followed by field movement. Egg-only and
other map layouts share the reviewed engine guard; they were not separate
emulator cases. Scoped formatting, lint and E2E TypeScript checks passed. E2E and normal Wayfarer builds passed
(`/tmp/trainer-only-center-final-build.log` and
`/tmp/trainer-only-center-playable-build.log`). The updated playable ROM SHA-256
is `514a52d04968b55f2480addb379bc472a76eba509c3629c86213ba15ac43f53f`.

## Playtest presentation polish

Rock now uses the native hit flash and HP-bar controller before committing HP,
then shakes the surviving wild sprite alongside the existing anger marks. Native
controller review confirmed visibility and sprite position are restored before
turn resolution. Successful Run and wild fleeing now play `SE_FLEE`; the normal
wild-flee script also uses sound and text without a sprite exit animation.

The E2E and normal Wayfarer builds passed (`/tmp/trainer-only-polish-build.log`
and `/tmp/trainer-only-polish-playable-build.log`). On E2E ROM
`e925cba8d8ba3d7d1fe9643f9752d2b4944b02c30e8dbb3b4ff9f0141a60ef07`,
the existing wild-flee callback, successful Run and lethal one-HP Rock checks
passed 3/3 (`/tmp/trainer-only-polish-exits.log`; 20 unrelated cases deliberately
skipped). Temporary focus markers were removed. No story matrices were rerun
for this presentation change.

The existing Rock presentation journey passed 1/1 on the same ROM
(`/tmp/trainer-only-rock-reaction-e925.log`). Its retained screenshots show hit
flashing, intermediate HP-bar lengths and wild-sprite movement with anger marks;
these were visually inspected, rather than asserted through whole-screen hashes.
The test asserts turn ownership throughout both presentation intervals. Scoped
formatting, lint and TypeScript checks passed. The updated playable ROM is
`../artifacts/pokemon-wayfarer-trainer-only.gba`, SHA-256
`56eff3466e4ded3c5c8160fd7603db5679d83f92510a2bab5c566741f99d8b42`.

## Maintained suite audit

[The E2E audit](trainer-only-e2e-audit.md) records the removed duplicates,
representative variant selection and the distinct gaps retained for validation.
Earlier larger runs below are historical evidence, not maintained test counts.
The reduced ordinary suite passed 14/14 on `bd22be82b131…`
(`/tmp/trainer-only-story-audit-reduced.log`). Reduced Johto coverage passed 11/11
with 35 internal fixtures on the same ROM
(`/tmp/trainer-only-johto-audit-final.log`).

The natural-grass journey passed 1/1 in 5.93 seconds on `bd22be82b131…`
(`/tmp/trainer-only-natural-grass.log`): ordinary walking in Route 30 generated
Ledyba and initialized the real trainer-only menu with an empty party. Root
inspected `/tmp/trainer-only-natural-grass-menu.png`: trainer and wild sprites,
wild HP only, Rock/Bag above Go Near/Run. No synthetic battle request is used.

Radio's two native continuation probes passed 2/2 on `bd22be82b131…`
(`/tmp/trainer-only-radio-boundary-bd22.log`). They force outcomes only after
entering the actual scripted trainer battle. Archer's native writer grants the
Silver Wing and sets occupation state 10 without retiring Silver; the subsequent
Underground Silver writer retires its own chapter and preserves state 10. The
fake Director probe observes transformation before forcing loss, then verifies
restored disguise actors, unchanged state 6 and safe refusal without another
money change. These tests do not prove actual combat or loss damage; the existing
real-loss journeys provide that evidence.

The final focused mechanics command,
`make -C game BUILD=wayfarer -j6 check TESTS='Trainer-only*'`, passed 13/13
(`/tmp/trainer-only-audit-mechanics.log`). It includes the real ghost/Scope
exclusion, native source readers and the Less Escapes decision. Run's unchanged
single-roll predicate is now a pure helper used by the controller; roll 512
succeeds normally and is vetoed with Less Escapes. Separate resolver assertions
confirm a surviving failed turn advances time and anger. This is not a claim of
challenge-specific controller telemetry. NPC battle followers are disabled in
this configuration; their predicate test is conditional, and partner battle-type
exclusion remains covered by existing loss-policy mechanics.

Current main was rechecked at `1e2c5c3122`: PR #82 contributes the Kanto port
PRD, explicitly unimplemented. It adds no personal Blue runtime chapters. The
task retains its `7525da55fa` base and current HNS Blue Gym coverage.

The maintained Ilex/Tower host suite passed 2/2 on `bd22be82b131…`
(`/tmp/trainer-only-johto-hosts-lifecycle-ack.log`). Ilex executes the native Cut
handoff with an empty party. Tower executes native discovery/beast release with
a fainted member, then consumes a real targeted Revive on that same save. After
returning to Tower Silver, an explicit forced win exercises the native
post-discovery continuation: Silver completes without a second fall, and Ecruteak
state 4 and beast-release flags remain intact.

All four final normal configurations compiled sequentially after the Run helper:
Wayfarer, HNS, FireRed and Emerald. Logs are
`/tmp/trainer-only-audit-playable-build.log`,
`/tmp/trainer-only-audit-hns-build.log`,
`/tmp/trainer-only-audit-firered-build.log` and
`/tmp/trainer-only-audit-emerald-build.log`. The pre-polish playable artifact was
`../artifacts/pokemon-wayfarer-trainer-only.gba`, SHA-256
`158305bef1b1a582b2363a5eb873915c956f56dcee96d1b7912b1ba1c1500e6e`.
The emulator ROMs below precede only that behavior-preserving helper extraction;
its condition and single-roll ownership were reviewed and mechanics-tested.

Hoenn story coverage comprises 26 maintained cases. The final full file run
passed 23 unchanged cases (`/tmp/trainer-only-hoenn-story-e2e-final26-bd22.log`),
then the three changed cases passed in a focused run
(`/tmp/trainer-only-hoenn-story-three-changed-bd22.log`, 3 passed/23 deliberately
skipped). The temporary focus markers were removed; this is combined evidence,
not a claim that the initial full run passed all 26.

Lilycove and Mauville restore their native rival interactions after real Revive
recovery and movement that clears the follower from the interaction tile.
Chapter flags are checked before opening dialogue because the banked flag
observer requires a settled field. The tests stop at native May/Wally text;
they do not claim another complete battle. Initial failures were follower
interaction and test-observer/dialogue timing, not a production restoration bug.

Route 119 extends its existing recovery case with forced loss and win only after
a settled real trainer battle. Loss leaves route/Scott state 0 and the Fly flag
false; real Revive, leave/re-enter and retry then reach the native victory writer,
route/Scott state 1 and `FLAG_RECEIVED_HM_FLY`. The bag observer still reports
Fly quantity 0, so this proves the native award writer and flag, not an observed
HM inventory quantity. Actual damage/ordinary fees remain covered by retained
real-loss cases. No gameplay code changed for these probes.

The reduced Hoenn objective suite passed 17/17 on `bd22be82b131…`
(`/tmp/trainer-only-hoenn-objectives-e2e-final17-bd22.log`), retaining every local
objective, both Rusturf west entrances, five actual loss continuations, native
usable backup staging, real rescue victory and both Chimney exits. Hoenn journey
format/lint/typecheck passed; no temporary focus markers remain.

Final review found no unresolved production defect. Wayfarer story source audit
passed its 851-caller manifest, 16 generator/Hoenn tests and nine Johto contracts
(`/tmp/trainer-only-story-audit-final.log`). Final TypeScript and scoped lint
passed. Logs are preserved under the task's `artifacts/validation/` directory.

## Historical implementation evidence

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
| `make -C game BUILD=wayfarer -j6 e2e` | Playable Wayfarer E2E ROM and matching symbols built. |

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
party/PC PP. Consolidated results: core 23/23, origin recovery 4/4, medicine 5/5 and natural Safari 1/1 passed. Logs are `/tmp/trainer-only-core-e2e-5012418.log`, `/tmp/trainer-only-poison-e2e-5012418.log` and `/tmp/trainer-only-safari-e2e-5012418.log`. The final Safari journey accepts the native probabilistic flee after Go Near instead of requiring survival; the earlier closest-distance screenshot and deterministic mechanics checks cover proximity saturation. A separate settled Revive picker capture also passed its real interaction check.

The action menu was inspected visually: Rock/Bag occupy the top row and
Go Near/Run the bottom, with a trainer sprite and no player Pokémon healthbox.
The retaliation screenshot shows the trainer attack message and lunge. The visible anger warning fits the normal battle text window. The settled Revive
picker shows the actual fainted Rattata at 0/12 HP and normal party targeting.
Evidence: `/tmp/trainer-only-warning-visible.png` and
`/tmp/trainer-only-revive-picker.png`, both inspected visually.

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

## Story integration validation

The current ordinary manifest contains 851 exact callers on 155 maps: 688 base
singles, 99 single rematches, 54 base doubles and 10 double rematches. The 787
single callers authorize field loss return; doubles retain existing loss routing.
Regional registries separately own authored scenes. The source audit rejects
changed command blocks and duplicate ownership, and the runtime registry test
checks the assembled caller addresses. No new caller is enrolled automatically.

`make -C game BUILD=wayfarer -j6 check TESTS='Wayfarer story*'` passed all five
tests: exact caller authorization, stable dialogue, usable-party eligibility,
completed trainer text and collision-safe restoration at transition elevations.
`TESTS='Wayfarer trainer field*'` passed its extended loss/draw/forfeit/abnormal-end
check. Only a win can reach an allowlisted caller's victory continuation; existing
challenge/partner exclusions remain in force. Logs:
`/tmp/trainer-only-story-seventh-build.log` and
`/tmp/trainer-only-story-loss-mechanics.log`.

The current static suites passed: 24 HNS traversal contracts, 9 Johto story
contracts and 13 Hoenn story audits. The 518-map Hoenn content audit passed after
reviewing the changed story scripts and updating its expected fingerprint.
Earlier build failures exposed label casing, conditional special registration,
test include/fixture setup and source-block assertions; they were corrected
before these passing runs.

Story emulator validation is in progress against ROM
`cac7382181712c8c76e1dcd9ebf6d709d74c1062e4e173672a4dfd3e2ed52a44`.
The first Hoenn run passed the Space Center 1F stair refusal but failed the two
Route 110/119 recovery-and-reentry journeys; their fixture and runtime diagnosis
found that the fixture omitted a valid Hoenn starter choice and used unbanked
story-variable aliases. Its state-preservation assertions are therefore not
accepted as evidence, including the stair case. The harness must set and observe
the actual banked Hoenn variables before those journeys can validate the scenes.
The initial ordinary/Johto run also exposed fixture issues and
is not a passing validation result. Review found a missing pre-staging Matt gate
and a saved-position transient-spawn check; both fixes await rebuilt-ROM checks.
These runs do not yet cover all required current story scenes.

The refreshed story ROM is
`62e163300a632d4fb5fd8f4e76ed470713ca380610f653936a748e11ccee966d`.
It includes both review fixes; all five story mechanics tests passed again
(`/tmp/trainer-only-story-spawn-fix-mechanics.log`) and the E2E ROM build passed
(`/tmp/trainer-only-story-spawn-fix-e2e-build.log`). Standalone HNS also compiled
with these story changes (`/tmp/trainer-only-story-hns-build.log`). These builds
do not establish the still-pending scene walkthrough results.

Emulator diagnosis then found ten Johto completed-trainer branches incorrectly
reading `VAR_RESULT` after `checktrainerflag`, which sets the script comparison
result instead. All ten now use `goto_if_defeated`; the 9 Johto contracts and
24 traversal contracts passed after this correction. The corrected scene
walkthroughs remain pending. Standalone FireRed and Emerald builds also passed
(`/tmp/trainer-only-story-firered-build.log` and
`/tmp/trainer-only-story-emerald-build.log`); the ten branch changes are guarded
to Wayfarer.

Preprocessing exposed a separate engine issue: Hoenn eligibility callbacks used
shared HNS constant names, so Route 110 read Goldenrod's state and several Hoenn
completion flags resolved to placeholder zero. The Hoenn registry now uses local
banked source IDs, checked against the generated Emerald source constants. All
six story mechanics tests passed in
`/tmp/trainer-only-story-hoenn-bank-mechanics-final.log`, including independent
Johto/Hoenn variable and flag changes and Hoenn completion retirement. The first
run's only failure was a test assumption that flag zero could be set; the final
test instead checks independence from the valid HNS flag at source ID `0x6F`.

The current story ROM is
`a8661e832466340f8f626d9a6bf23df52c16c35027beb95652ab43dabca1f850`
(`/tmp/trainer-only-story-hoenn-bank-e2e-build.log`). Its E2E-only mailbox validates
banked fixture variables, observes them through `VarGet`, and sets them through
`VarSet` only in settled field state. Read-only observation permits initialized
save-state reads during a battle. The public story fixture API no longer treats Hoenn IDs as offsets into
Johto storage. Scene walkthroughs on this ROM remain in progress.

The journeys use actual Fight/move input
for defeat and check fainted party state, the normal single money charge, no
trainer/reward credit, refusal after loss, recovery and retry. Forced outcome
mailbox calls are not accepted as evidence of those complete loss paths.

Final core regression checks use story ROM
`31a1a40644ef09e91f478ed33cb939b0d53167b386ec6acc0837f15bdc15f6eb`.
All 33 emulator cases passed: the core controller, party-targeted medicine,
ordinary Safari flow and origin-specific recovery
(`/tmp/trainer-only-final-core-e2e-31a1a.log`). The final `Trainer*` mechanics
run passed 88 cases with one existing assumption skip
(`/tmp/trainer-only-final-core-mechanics.log`). Standalone HNS built successfully
with the recovery sight suppression and trainer scan bounds correction
(`/tmp/trainer-only-final-hns-build.log`).

Visual inspection of the settled Rusturf Aqua, Chimney Maxie and Space Center
Steven refusal screenshots confirmed readable text within the dialogue box.
The hostile lines and partner refusal keep their authored character assignments.
The screenshots are `/tmp/trainer-only-objective-Rusturf-Peeko-and-Goods-guard.png`,
`/tmp/trainer-only-objective-Mt-Chimney-Maxie.png` and
`/tmp/trainer-only-objective-Space-Center-Steven-partner-offer.png`.

The Hoenn local-objective suite passed all 25 cases on that ROM
(`/tmp/trainer-only-hoenn-objectives-final-e2e-31a1a.log`). It covers empty,
fainted and Egg-only refusal for Rusturf, Chimney Maxie, Magma Hideout Maxie,
Weather Institute Shelly, Seafloor Archie, the Space Center three-grunt chain
and Steven's partner offer. Each checks pending story state and flags, no battle
and released field control. Actual Rusturf and Magma Hideout defeats retain the
local objective and charge the ordinary loss fee once; repeated interaction
refuses without another charge. A real Rusturf victory clears the rescue through
native completion writers, including Rustboro state 4 and Briney's house state 1.
The test does not invent a Rusturf state increment absent from the native script.
The extended run also passed all 25 cases, walking both Chimney exits outward
and back with Maxie’s objective still pending
(`/tmp/trainer-only-hoenn-objectives-bidirectional-e2e-31a1a.log`).

Final standalone FireRed and Emerald builds also passed with the common trainer
scan bounds correction (`/tmp/trainer-only-final-firered-build.log` and
`/tmp/trainer-only-final-emerald-build.log`). These compilation checks do not
substitute for story walkthrough coverage in Wayfarer.

Alternate-entry review found an additional Rusturf coordinate path at `(9,4)`
and `(9,5)` that moved Grunt/Peeko and wrote tunnel state 3 before the guarded
object dialogue. The earlier 25-case pass covered direct interaction only and
does not establish these approaches. An early gate and dedicated entrance tests
are required before story completion. The same review found no other alternate
caller in the seven objective families; scene-specific actual-loss coverage is
being extended from Rusturf/Magma to Chimney, Shelly and Seafloor.

The Rusturf gate is now validated on ROM
`66eae19593a28dbd2f68a0c35b7772197dd6887d0bd03ffaec204d82449287a6`.
All 35 objective cases passed
(`/tmp/trainer-only-hoenn-objectives-rusturf-final-e2e.log`): both west entrance
rows with empty/fainted/Egg parties preserve state 2 and allow retreat; a usable
party retains the native backup movement and state 3. Actual losses now cover
all five supported objective families in this journey: Rusturf, Chimney Maxie,
Magma Hideout Maxie, Weather Institute Shelly and Seafloor Archie. Each returns
locally, preserves its objective, charges once and refuses a subsequent approach.
The first expanded run's remaining failures were fixture timing before the
Rusturf script started and an unrelated visible Chimney trainer intercepting
arrangement; waiting for native dialogue and using the normal hidden-trainer
flag resolved them. The Rusturf early gate also passes its static assertion.

A final core audit identified two unfinished requirements despite the passing
controller regressions: surviving-Rock reaction presentation and direct native
encounter-source reader coverage. These are being completed; neither spec is
marked implemented by the existing results.

The main story suite passed 13/13 on ROM `31a1a406…`
(`/tmp/trainer-only-story-e2e-31a1a4-final.log`). Actual Mikey/Joey losses return
locally with one fee, no trainer credit and no Joey registration. Post-loss
refusal, real Revive targeting, stable field return and Mikey direct retry pass.
Joey's open Route 30 position also proves walking out of sight and returning
restores an automatic normal challenge. The failed earlier movement assertion
used a dead-end Mikey fixture; no production movement change was needed.
The suite includes Proton's supported loss/retry and Blue's excluded Gym loss
routing without badge credit. Ordinary, Rocket and Gym refusal screenshots were
visually inspected and fit their dialogue windows.

The E2E protocol journey passed 4/4 on the same ROM, and its unit suite passed
14/14 (`/tmp/e2e-protocol-unit-final.log`). TypeScript checking passed
(`/tmp/e2e-typecheck-final.log`). The four ordinary dialogue assignments with
rematch/reload coverage are still being added to the scene walkthroughs.

Native source validation passed two tests with 20 parameter cases
(`/tmp/trainer-only-native-source-mechanics-rerun.log`). The tests execute actual
land, outbreak, fishing, Rock Smash and Sweet Scent readers, generation and
trainer-only preparation with empty/fainted/Egg parties. Seeded comparisons check
neutral lead stats/items and active Repel handling. A test-only hook intercepts
only the final graphics transition; these tests do not grant field-action
permissions or claim to walk through the fishing/field-move menus. The initial
compile needed the missing public Rock Smash reader declaration.

Surviving Rock now plays two native anger-mark effects on the real wild sprite
before the anger message and completed-turn resolution; lethal rocks bypass it.
The updated E2E ROM is
`edbcd8f7956ac6a2955bcbd3571289ad994894ed8a6735dabef2c4a32672b2d0`.
Frames `/tmp/trainer-only-rock-reaction-0.png`, `-2.png`, `-4.png` and `-6.png`
were visually inspected: both marks are anchored to Pidgey, with the trainer
sprite unchanged. The updated core emulator run passed all 24 cases (23 core plus the reaction
journey) in `/tmp/trainer-only-rock-reaction-core-e2e.log`, including lethal Rock
termination and unchanged completed-turn ownership.

The 35-case objective journey also passed on the final reaction ROM
(`/tmp/trainer-only-hoenn-objectives-edbcd8-e2e.log`) with explicit move-menu
cursor selection for actual battles. Final standalone HNS, FireRed and Emerald
builds passed after the reaction and native-source test hook changes:
`/tmp/trainer-only-final-reaction-hns-build.log`,
`/tmp/trainer-only-final-reaction-firered-build.log` and
`/tmp/trainer-only-final-reaction-emerald-build.log`.

Expanded ordinary dialogue walkthroughs passed 14/14 on `edbcd8f7956…`
(`/tmp/trainer-only-story-e2e-rock-ordinary.log`). Route 32 Roland, Henry, Gordon
and Peter render all four frozen dialogue assignments without battle; repeated
interaction preserves each line, and Roland retains it after save/reload.
Selector mechanics separately verify no gameplay RNG use. A real rematch branch
walkthrough remains pending its narrowly scoped E2E fixture.

The final 50-case Hoenn story journey passed on ROM `66eae19593a…`
(`/tmp/trainer-only-hoenn-story-e2e-final50.log`). This includes the expanded
Route 110/119 approach, starter and player-gender matrix, native Go Goggles
handoffs without a usable party, Rustboro's eligible Match Call conversation,
and Lilycove's east sea outlet in both directions before Matt's completion.
The detailed scene inventory is supplied by the journey and its final coverage
summary; this result does not cover future FRLG/Sevii ports.

A normal playable build with the Rusturf guard and selected Rock reaction passed
(`/tmp/trainer-only-final-playable-build.log`). The artifact is
`../artifacts/pokemon-wayfarer-trainer-only.gba`, SHA-256
`d156018284a6112e387e278ee0b19f67c9727a3fa784cca9169e71eb1bb88905`.
The subsequent rematch fixture changes are E2E-only and do not add a gameplay
rematch shortcut.

The final ordinary/Johto base suite passed 15/15 on rematch ROM
`bd22be82b1317a22d1ccded39c89c51fba353cb1ef01afb8ac097b3bfb658b35`
(`/tmp/trainer-only-story-e2e-rematch.log`). The real Joey rematch branch receives
the same frozen refusal without registering Joey. Its E2E fixture validates a
native rematch-table base trainer, marks that base fought, and calls the existing
`UpdateRematchIfDefeated`; no battle outcome is forced. It uses reserved request
bytes without changing ABI 15's 432-byte layout. The caller generator now rejects
base/rematch dialogue or key drift. All four settled ordinary screenshots were
visually inspected and fit two lines. The final protocol journey passed 4/4 and
unit suite 14/14 (`/tmp/hns-e2e-protocol-rematch.log` and
`/tmp/e2e-protocol-unit-rematch.log`).

Native Ilex and Burned Tower host completion passed six cases on `edbcd8f7956…`
(`/tmp/trainer-only-johto-hosts-e2e-final2.log`), covering empty/fainted/Egg parties.
Ilex starts from the completed chase prerequisite and actually executes the Cut
handoff: obtained-item dialogue, received-Cut flag, Farfetch'd state 3, Azalea
state 7 and actor removals. Tower actually falls into the basement and releases
the beasts, setting discovery/release flags and Ecruteak state 4. Both preserve
the corresponding pending Silver completion flag and exact party snapshot.
These are native host completions, not fixture-forward retirement assertions.
Unseeded items are absent from the harness's bag observation list, so the gift
proof uses its native obtained-item dialogue and completion writers rather than
an unsupported zero-fallback bag quantity observation.

The expanded Johto suite passed 11/11 on `31a1a406…`
(`/tmp/trainer-only-johto-coverage-e2e-31a1a4.log`). It covers numbered Silver
approaches/starter variants, deferred recovery and post-occupation return,
saved hidden-actor tiles, named objective refusals and Route 24 with
empty/fainted/Egg parties, the native Tohjo prerequisite refusal, the excluded
Electrode path, and actual Petrel loss without passwords or host completion.
An actual Azalea win from the post-Ilex state-7 fixture spends move PP, retires
Silver's own flag and preserves state 7. A real Proton win executes Kurt's native
return and rescue writers to state 4 while Silver's completion stays false.
The Ilex handoff itself is exercised separately in the six-case host suite.
The earlier level-100 victory fixtures triggered existing obedience behavior at
TR 0; at-cap level-15 Pokémon resolved that test setup without gameplay changes.
Scoped lint, TypeScript and the nine Johto static contracts passed.
